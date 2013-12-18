/* Include Files */
/* ============= */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/errno.h>
#include <sys/immu.h>
#include <sys/region.h>
#include <sys/proc.h>
#include <sys/sysmacros.h>

#include "hepcdefs.h"

/* Global Constants */
/* ================ */

#define BANNER "\nHEPC2 device driver for UNIX System V.4, version 1.1\n\
Copyright (C) 1992, Perihelion Software Ltd.  All rights reserved.\n"

#define TRUE	1
#define FALSE	0

/*
 * Define number of devices here as 3, corresponding to 3 units at 3 possible
 * base addresses.
 */
#define	NHEPC	3	

/*
 * Define sleep priority as an interruptable by signals, with
 * signals caught.
 */
#define SLEEP_PRI	(PZERO+1|PCATCH)

/* Device status values */

#define STS_AVAILABLE		0
#define STS_OPEN		0x0001
#define STS_DISABLED		0x0002

/* I/o modes */

#define	HEPC_READ		0x0001
#define	HEPC_WRITE		0x0002
#define HEPC_READ_SLEEP		0x0010
#define HEPC_WRITE_SLEEP	0x0020

/* Aborted i/o flags */
 
#define         ABORT_SIGNAL    0x1
#define         ABORT_TIMEOUT   0x2
 
/* reset bits */

#define	TIM40_ASRT_RESET	0x0f
#define	TIM40_RLSE_RESET	0xf0

/* Global Macros */
/* ============= */

/*
 * We encode in the minor device number the following:
 *
 *	bits 0,1	0
 *    
 *	bit 2		Interrupt channel, 0 = 12, 1 = 15
 *
 *	bits 3, 4	Unit number, where 0 uses address 0x150,
 *				           1 uses address 0x200
 *					   2 uses address 0x300
 *					   3 is invalid
 */

/*
 * Macro to extract unit number from device information.
 */
#define HEPCUNIT(dev)	(minor(dev)>>3)
#define HEPC_INT(dev)	((minor(dev)&4)?15:12)

/*
 * Macros to access the device registers in i/o space
 */
#define	READREG(x)	(inb((short)x))
#define WRITEREG(x,y)	(outb((short)x,(u_char)y))
/*
 * Macro to print debug messages if debug flag set
 */
#define debug		if(debug_flag)printf

/* 
 * Macro to delay for a specified number of microseconds
 */
#define DELAY(n)	(void)hepc_delay(n,0);


/* External Routines */
/* ================= */

extern void	outb();
extern u_char	inb();


/* Static functions */
/* ================ */

static void	hepcstrategy();
static void	hepcstart();
static void	hepctimeout();

/* Global Variables */
/* ================ */

/* Possible board addresses */

static struct hepc_reg *board_address[3] = 
{
    (struct hepc_reg *)0x150, 
    (struct hepc_reg *)0x200, 
    (struct hepc_reg *)0x300
};

/* Flags */

static int	debug_flag = 0;	/* debug flag */

/* static buffer headers for physio */

static struct buf       hepc_readbufs[NIMB];
static struct buf       hepc_writebufs[NIMB];

/* Device status information */

static struct hepc_device
{
    int		status;		/* device status */
    int		initialised;	/* device initialised */
    int		iomode;		/* current i/o mode */
    int		read_abort;	/* read abort flags */
    int		write_abort;	/* write abort flags */
    int		timeout;	/* i/o timeout */
    int		int_chan;	/* interrupt channel */
    int		uid;		/* current user id */
    char	intcr;		/* copy of intcr */
    struct buf	*read_bp;	/* current read buf */
    struct buf	*write_bp;	/* current write buf */
    int		read_count;	/* bytes to xfer in read */
    int		write_count;	/* bytes to xfer in write */
    int		read_offset;	/* current offset for read */
    int		write_offset;	/* current offset for write */
} hepcdevice[NHEPC];


/*############################################################################*/
hepcinit( dev )

dev_t	dev;
{

/*----------------------------------------------------------------------------
 * Purpose:   Initialisation function, called on first open
 *
 * Method:	Reset the device, and check that the isr and osr
 *		behave as expected.  If they don't assume device
 *		is not present.
 *----------------------------------------------------------------------------*/

    static int			first_time = TRUE;

    register struct hepc_reg	*reg;		/* device registers */
    register struct hepc_device	*hepc;		/* device structure */
    int				unit;		/* unit number */
    unsigned int		channel_select;	/* channel select mask */
    short			addr_isr;	/* address of isr */
    short			addr_osr;	/* address of osr */
    short			addr_reset;	/* address of reset */
    short			addr_intcr;	/* address of intcr */


/*============================================================================*/

    /*
     * For first device print out banner and do a bit
     * of initialisation.
     */
    if ( first_time )
    {
	printf( BANNER );

	/*
	 * Set all devices disabled until we probe for them
	 */
	for( unit = 0;  unit < NHEPC;  unit++ )
	{
	    hepcdevice[unit].status = STS_DISABLED;
	}

	first_time = FALSE;
    }

    /*
     * Initialisation
     */
    unit = HEPCUNIT(dev);
    if ( (unit < 0) || (unit >= NHEPC) )
    {
	return ENXIO;
    }

    reg = board_address[unit];
    hepc = &hepcdevice[unit];

    addr_isr 	 = (short)&(reg->isr);
    addr_osr	 = (short)&(reg->osr);
    addr_reset	 = (short)&(reg->reset);
    addr_intcr 	 = (short)&(reg->intcr);

    debug("hepc%d:  init, reg = %x\n", unit, reg );

    /*
     * Initialise device structure
     */
    hepc->status = STS_AVAILABLE;
    hepc->iomode = 0;
    hepc->uid = -1;
    hepc->int_chan = HEPC_INT(dev);
    /*
     * Reset the device
     */
    WRITEREG( addr_intcr, 0 );
    DELAY( 1000 );
    WRITEREG( addr_reset, TIM40_ASRT_RESET);
    DELAY( 1000 );
    WRITEREG( addr_reset, TIM40_RLSE_RESET);
    DELAY( 1000 );

    /*
     * check the status registers; 
     * input should not be ready, output should be ready
     */
    if ( (READREG( addr_isr ) & LINK_READY) != 0 )
    {
	printf( "hepc%d: ISR check failed (isr = %x); disabling device.\n",
		unit, READREG( addr_isr ) );
	hepc->status = STS_DISABLED;
	return EIO;
    }
    if ( (READREG( addr_osr ) & LINK_READY) != LINK_READY )
    {
	printf( "hepc%d: C012 OSR check failed( osr = %x); disabling device.\n",
		unit, READREG( addr_osr ) );
	hepc->status = STS_DISABLED;
	return EIO;
    }
    /*
     * Disable interrupts
     */
    WRITEREG( addr_intcr, 0 );
    hepc->intcr = 0;

    return( 0 );
}


/*############################################################################*/
hepcopen( dev, flags, otyp )

dev_t	dev;		/* device number */
int	flags;		/* read/write flags */
int	otyp;		/* protocol rules (not used) */
{

/* Returned Value:	0 	- success
 *			ENXIO	- device not available
 *			EBUSY	- device in use (non-standard)
 *
 *----------------------------------------------------------------------------
 * Purpose: To open device for data transfers
 *
 * Method:
 *----------------------------------------------------------------------------*/

/* Local Variables */

   

    register int	unit;		/* unit number */
    int			rval;		/* rturn value */

/*============================================================================*/

    unit = HEPCUNIT( dev );

    /*
     * First time round initialise the device
     */
    if ( !hepcdevice[unit].initialised )
    {
	rval = hepcinit( dev );
	if ( rval != 0 )
	{
	    u.u_error = rval;
	    return;
	}
	hepcdevice[unit].initialised = TRUE;
    }

    /*
     * Check device is usable
     */
    if ( unit >= NHEPC || (hepcdevice[unit].status & STS_DISABLED) ) 
    {
	u.u_error = ENXIO;
	return;
    }


    /*
     * If already open check for match of uid
     */
    if ( hepcdevice[unit].status & STS_OPEN ) 
    {
	if ( u.u_uid == hepcdevice[unit].uid ) 
	{
	    debug( "hepc%d: Open (2) by process %d\n", unit, u.u_procp->p_pid );
	    return;
	}
	else
	{
	    u.u_error = EBUSY;
	    return;
	}
    }

    debug( "hepc%d: Open (1) by process %d\n", unit, u.u_procp->p_pid );
    /*
     * Set status to open, and save user details
     */
    hepcdevice[unit].status |= STS_OPEN;
    hepcdevice[unit].uid  = u.u_uid;
    hepcdevice[unit].timeout = 0;

    return;
}


/*############################################################################*/
hepcclose( dev, flags, otyp )

dev_t	dev;		/* device information */
int	flags;		/* flags as for open */
int	otyp;		/* protocol details (not used) */
{

/* Returned Value:    Always 0 (success)
 *
 *----------------------------------------------------------------------------
 * Purpose: To close device, and clear device details
 *
 * Method:
 *----------------------------------------------------------------------------*/

/* Local Variables */

    int			unit;		/* unit number */
    struct hepc_reg	*reg;		/* device structure pointer */
    short		addr_intcr;	/* address of intcr */

/*============================================================================*/

    /*
     * Extract unit number from device, and check its valid
     */
    unit = HEPCUNIT( dev );

    if ( unit < NHEPC )
    {
	debug( "hepc%d: close\n", unit );
	/*
	 * Clear interrupt control register
	 */
	reg = (struct hepc_reg *)board_address[unit];

	addr_intcr = (short)&(reg->intcr);

	WRITEREG( addr_intcr, 0 );
	/*
	 * Clear device details
	 */
	hepcdevice[unit].iomode	= 0;
	hepcdevice[unit].intcr 	= 0;
	hepcdevice[unit].status &= ~STS_OPEN;
	hepcdevice[unit].uid	= -1;
    }
    return;
}

/*############################################################################*/
hepcread( dev )

dev_t		dev;	/* device information */
{

/* Returned Value:	0 - success
 *			error code from errno.h if failure
 *
 *----------------------------------------------------------------------------
 * Purpose: Read data from device
 *
 * Method:  Call physio kernel support routine.
 *----------------------------------------------------------------------------*/

/* Local Variables */

    register struct hepc_reg	*reg;		/* hepc registers */
    register struct hepc_device	*hepc;		/* device structure pointer */
    int 			unit;		/* unit number */

/*============================================================================*/

    /*
     * Extract unit number and check it
     */
    unit = HEPCUNIT( dev );

    debug( "hepc%d: read\n", unit );

    if ( unit >= NHEPC ) 
    {
	u.u_error = ENXIO;
	return;
    }

    /*
     * Initialisation
     */
    hepc = &hepcdevice[unit];
    reg = board_address[unit];

    /*
     * Let physio do the work
     */

    physio( hepcstrategy,
		&hepc_readbufs[unit],
		dev,
		B_READ );

    debug( "hepc%d:  read done\n", unit );

    return;
}
/*############################################################################*/
hepcwrite( dev )


dev_t		dev;	/* device information */

{

/* Returned Value:	0 if successful
 *			error code from errno.h otherwise
 *
 *----------------------------------------------------------------------------
 * Purpose: To write to the device
 *
 * Method:  Call physio to do the work.
 *----------------------------------------------------------------------------*/

/* Local Variables */

    register struct hepc_reg	*reg;		/* hepc registers */
    register struct hepc_device	*hepc;		/* device structure ptr */
    int 			unit;		/* unit number */

/*============================================================================*/

    /*
     * Extract and check unit number
     */
    unit = HEPCUNIT( dev );

    debug( "hepc%d: write\n", unit );

    if ( unit >= NHEPC ) 
    {
	u.u_error = ENXIO;
	return;
    }

    /*
     * Initialisation
     */
    hepc = &hepcdevice[unit];
    reg = board_address[unit];

    /*
     * Call physio to do the work
     */

    physio( hepcstrategy,
		&hepc_writebufs[unit],
		dev,
		B_WRITE );

    debug( "hepc%d:  write done\n", unit );

    return;
}

/*############################################################################*/
static void hepcstrategy( bp )

register struct buf	*bp;
{

/*----------------------------------------------------------------------------
 * Purpose: High level i/o routine
 *
 * Method:
 *----------------------------------------------------------------------------*/

/* Local Variables */

    register struct hepc_reg	*reg;		/* hepc registers */
    register struct hepc_device	*hepc;		/* device structure */
    register int		unit;		/* unit number */
    int				old_pri;	/* saved priority */
    unsigned int		arg_tmo;	/* timeout argument */
    short			addr_isr;	/* address of isr */
    short			addr_osr;	/* address of osr */
    short			addr_intcr;	/* address of intcr */

/*============================================================================*/

    /*
     * Initialisation
     */
    unit = HEPCUNIT( bp->b_dev );
    hepc = &hepcdevice[unit];
    reg = board_address[unit];

    addr_isr	= (short)&(reg->isr);
    addr_osr	= (short)&(reg->osr);
    addr_intcr	= (short)&(reg->intcr);

    /*
     * Either do a read or a write
     */
    switch( bp->b_flags & B_READ )
    {
      case B_READ:

	debug( "hepc%d:  read %d bytes\n", unit, bp->b_bcount );

	hepc->read_count = bp->b_bcount;
	/*
	 * Sleep until link ready, or something else happens
	 */
	hepc->read_abort = 0;

        while( ( (READREG(addr_isr) & LINK_READY) == 0 ) &&
               (hepc->read_abort == 0) )
	{
	    /*
	     * Up priority, enable interrupts and then sleep
	     */
	    old_pri = spl5();

	    hepc->intcr |= INT_RECEIVE_READY;
	    WRITEREG( addr_intcr, hepc->intcr );
	    /*
	     * Sleep, unless its now ready. We must check it
	     * after re-enabling interrupts, because we may
	     * not get an interrupt if it was ready when we
	     * enabled interrupts.
	     */
	    if ( (READREG(addr_isr) & LINK_READY) == 0 )
	    {
		if ( hepc->timeout > 0 )
		{
		    arg_tmo = (unit<<16) | B_READ;
		    timeout( hepctimeout, arg_tmo, hepc->timeout );
		}
		hepc->iomode |= HEPC_READ_SLEEP;

		debug( "hepc%d:  read sleep 1\n", unit );
		if ( sleep( (caddr_t)&(hepc->read_bp), SLEEP_PRI ) )
				    hepc->read_abort |= ABORT_SIGNAL;
		debug( "hepc%d:  read wakeup\n", unit );

		hepc->iomode &= ~HEPC_READ_SLEEP;

		if ( hepc->timeout > 0 ) untimeout( hepctimeout, arg_tmo );

		/*
		 * if timed out, but link ready, clear timeout flag
		 */
		if ( (hepc->read_abort & ABORT_TIMEOUT) &&
		     ( (READREG(addr_isr) & LINK_READY) != 0 ) )
		{
		    hepc->read_abort &= ~ABORT_TIMEOUT;
		}
	    }

	    /*
	     * Clear interrupts and lower priority
	     */
	    hepc->intcr &= ~INT_RECEIVE_READY;
	    WRITEREG( addr_intcr, hepc->intcr );

	    splx( old_pri );
	}

	if ( hepc->read_abort != 0 )
	{
	    ;	/* continue */
	}
	else
	{
	    /*
	     * Interrupt driven i/o;  set up pointers for first transfer.
	     */
	    debug( "hepc%d:  link adaptor i/o - %d\n", unit, hepc->read_count );

	    hepc->read_bp    = bp;
	    hepc->read_offset= 0;

	    hepc->iomode |= HEPC_READ;
	    /*
	     * Start the transfer, then if there's anything left
	     * loop round sleeping then transferring.
	     */
	    hepcstart( hepc, reg, B_READ );

	    while( hepc->read_count > 0 )
	    {
		/*
		 * Sleep until link ready, or something else happens
		 */
 
                while( ( (READREG(addr_isr) & LINK_READY) == 0 ) &&
                       ( hepc->read_abort == 0 ) )
		{
		    /*
		     * Up priority, enable interrupts and then sleep
		     */
		    old_pri = spl5();

		    hepc->intcr |= INT_RECEIVE_READY;
		    WRITEREG( addr_intcr, hepc->intcr );
		    /*
		     * Sleep, unless its now ready. We must check it
		     * after re-enabling interrupts, because we may
		     * not get an interrupt if it was ready when we
		     * enabled interrupts.
		     */
		    if ( (READREG(addr_isr) & LINK_READY) == 0 )
		    {
			if ( hepc->timeout > 0 )
			{
			    arg_tmo = (unit<<16) | B_READ;
			    timeout( hepctimeout, arg_tmo, hepc->timeout );
			}
			hepc->iomode |= HEPC_READ_SLEEP;

			debug( "hepc%d:  read sleep 2\n", unit );
			if ( sleep( (caddr_t)&(hepc->read_bp), SLEEP_PRI ) )
					    hepc->read_abort |= ABORT_SIGNAL;
			debug( "hepc%d:  read wakeup\n", unit );

			hepc->iomode &= ~HEPC_READ_SLEEP;
			if ( hepc->timeout > 0 ) 
				untimeout( hepctimeout, arg_tmo );

			/*
			 * if timed out, but link ready, clear timeout flag
			 */
			if ( (hepc->read_abort & ABORT_TIMEOUT) &&
			     ( (READREG(addr_isr) & LINK_READY) != 0 ) )
			{
			    hepc->read_abort &= ~ABORT_TIMEOUT;
			}
		    }
		    /*
		     * Clear interrupts and lower priority
		     */
		    hepc->intcr &= ~INT_RECEIVE_READY;
		    WRITEREG( addr_intcr, hepc->intcr );

		    splx( old_pri );
		}
		if ( hepc->read_abort != 0 ) break;
		/*
		 * Transfer the data
		 */
		hepcstart( hepc, reg, B_READ );
	    }
	    hepc->iomode &= ~HEPC_READ;
	}
        /* 
         * Finish off
         */
        bp->b_resid = hepc->read_count;
        if ( hepc->read_abort != 0 )
        {  
            debug( "hepc%d:  read aborted (%x), resid = %d\n",
                    unit, hepc->read_abort, hepc->read_count );
 
            if ( hepc->read_abort & ABORT_SIGNAL )
            {
                bp->b_flags |= B_ERROR;
                bp->b_error = EINTR;
            }
	}
	break;

      case B_WRITE:

	debug( "hepc%d:  write %d bytes\n", unit, bp->b_bcount );
 
        hepc->write_count = bp->b_bcount;
	/*
	 * Sleep until link ready, or something else happens
	 */
        hepc->write_abort = 0;
          
        while( ( (READREG(addr_osr) & LINK_READY) == 0 ) &&
               (hepc->write_abort == 0) )
	{
	    /*
	     * Up priority, enable interrupts and then sleep
	     */
	    old_pri = spl5();

	    hepc->intcr |= INT_SEND_READY;
	    WRITEREG( addr_intcr, hepc->intcr );
	    /*
	     * Sleep, unless its now ready. We must check it
	     * after re-enabling interrupts, because we may
	     * not get an interrupt if it was ready when we
	     * enabled interrupts.
	     */
	    if ( (READREG(addr_osr) & LINK_READY) == 0 )
	    {
		if ( hepc->timeout > 0 )
		{
		    arg_tmo = (unit<<16)|B_WRITE;
		    timeout( hepctimeout, arg_tmo, hepc->timeout );
		}
		hepc->iomode |= HEPC_WRITE_SLEEP;

		debug( "hepc%d:  write sleep 1\n", unit );
	       if ( sleep( (caddr_t)&(hepc->write_bp), SLEEP_PRI ) )
                                    hepc->write_abort |= ABORT_SIGNAL;
		debug( "hepc%d:  write wakeup\n", unit );

		hepc->iomode &= ~HEPC_WRITE_SLEEP;
		if ( hepc->timeout > 0 ) untimeout( hepctimeout, arg_tmo );

		/*
		 * if timed out, but link ready, clear timeout flag
		 */
		if ( (hepc->write_abort & ABORT_TIMEOUT) &&
		     ( (READREG(addr_osr) & LINK_READY) != 0 ) )
		{
		    hepc->write_abort &= ~ABORT_TIMEOUT;
		}
	    }
	    /*
	     * Clear interrupts and lower priority
	     */
	    hepc->intcr &= ~INT_SEND_READY;
	    WRITEREG( addr_intcr, hepc->intcr );

	    splx( old_pri );
	}

        if ( hepc->write_abort != 0 )
        { 
            ;           /* no action */
        }
	else
	{
	    /*
	     * Interrupt driven i/o;  set up pointers for first transfer.
	     */
	    hepc->write_bp    = bp;
	    hepc->write_offset= 0;

	    debug( "hepc%d:  link adaptor i/o\n", unit );

	    hepc->iomode |= HEPC_WRITE;
	    /*
	     * Start the transfer, then if there's anything left loop
	     * round sleeping and transferring until completion.
	     */
	    hepcstart( hepc, reg, B_WRITE );

	    while( hepc->write_count > 0 )
	    {
		/*
		 * Sleep until link ready, or something else happens
		 */
                while( ( (READREG(addr_osr) & LINK_READY) == 0 ) &&
                       (hepc->write_abort == 0) )
		{
		    /*
		     * Up priority, enable interrupts and then sleep
		     */
		    old_pri = spl5();

		    hepc->intcr |= INT_SEND_READY;
		    WRITEREG( addr_intcr, hepc->intcr );
		    /*
		     * Sleep, unless its now ready. We must check it
		     * after re-enabling interrupts, because we may
		     * not get an interrupt if it was ready when we
		     * enabled interrupts.
		     */
		    if ( (READREG(addr_osr) & LINK_READY) == 0 )
		    {
			if ( hepc->timeout > 0 )
			{
			    arg_tmo = (unit<<16)|B_WRITE;
			    timeout( hepctimeout, arg_tmo, hepc->timeout );
			}
			hepc->iomode |= HEPC_WRITE_SLEEP;

			debug( "hepc%d:  write sleep 2\n", unit );
			if ( sleep( (caddr_t)&(hepc->write_bp), SLEEP_PRI ) )
                                        hepc->write_abort |= ABORT_SIGNAL;
			debug( "hepc%d:  write wakeup\n", unit );

			hepc->iomode &= ~HEPC_WRITE_SLEEP;

			if ( hepc->timeout > 0 ) 
				untimeout( hepctimeout, arg_tmo );

			/*
			 * if timed out, but link ready, clear timeout flag
			 */
			if ( (hepc->write_abort & ABORT_TIMEOUT) &&
			     ( (READREG(addr_osr) & LINK_READY) != 0 ) )
			{
			    hepc->write_abort &= ~ABORT_TIMEOUT;
			}
		    }
		    /*
		     * Clear interrupts and lower priority
		     */
		    hepc->intcr &= ~INT_SEND_READY;
		    WRITEREG( addr_intcr, hepc->intcr );

		    splx( old_pri );
		}
		if ( hepc->write_abort != 0 ) break;
		/*
		 * Transfer the data
		 */
		hepcstart( hepc, reg, B_WRITE );
	    }
	    hepc->iomode &= ~HEPC_WRITE;
	}
        /*
         * Finish off
         */
        bp->b_resid = hepc->write_count;
         
        if ( hepc->write_abort != 0 )
        {
            debug( "hepc%d:  write aborted (%x), resid = %d\n",
                    unit, hepc->write_abort, hepc->write_count );
 
            if ( hepc->write_abort & ABORT_SIGNAL )
            {
                bp->b_flags |= B_ERROR;
                bp->b_error = EINTR;
            }
        }
	break;
    }
    debug( "hepc%d: i/o done, resid = %d\n", unit, bp->b_resid );
 
    iodone( bp );

    return;
}

/*############################################################################*/
static void hepcstart( hepc, reg, rw )

register struct hepc_device	*hepc;		/* device structure */
register struct hepc_reg	*reg;		/* hepc registers */
int				rw;		/* read/write flag */
{
/*----------------------------------------------------------------------------
 *
 * Purpose: To perform an i/o transfer
 *
 * Method:  Write to idr/odr as long as there's data and the isr/osr
 *	    says its ready.
 *
 * History:
 *
 *
 *----------------------------------------------------------------------------*/

/* Local Variables */
    register u_char	*cptr;		/* pointer to next char */
    short		addr_isr;	/* address of isr */
    short		addr_osr;	/* address of osr */
    short		addr_idr;	/* address of idr */
    short		addr_odr;	/* address of odr */

/*============================================================================*/

    switch( rw )
    {
      case B_READ:
	/*
	 * Set up register addresses
	 */
	addr_isr = (short)&(reg->isr);
	addr_idr = (short)&(reg->idr);
	/*
	 * Get pointer to next location in buffer
	 */
	cptr = ((u_char *)(hepc->read_bp->b_un.b_addr)) + hepc->read_offset;
	/*
	 * Read data while data is to be read, and the link is ready
	 */
	while( (hepc->read_count > 0) && (READREG( addr_isr ) & LINK_READY) )
	{
	    /*
	     * Read register and update counts
	     */
	    *cptr++ = READREG( addr_idr );
	    hepc->read_offset++;
	    hepc->read_count--;
	}
	break;

      case B_WRITE:
	/*
	 * Set up register addresses
	 */
	addr_osr = (short)&(reg->osr);
	addr_odr = (short)&(reg->odr);
	/*
	 * Get pointer to next location in buffer
	 */
	cptr = ((u_char *)(hepc->write_bp->b_un.b_addr)) + hepc->write_offset;
	/*
	 * Write data while data is to be written, and the link is ready
	 */
	while( (hepc->write_count > 0) && (READREG( addr_osr ) & LINK_READY) )
	{
	    /*
	     * Write to register, then wait a bit for next write
	     */
	    WRITEREG( addr_odr, *cptr++ );
	    hepc->write_offset++;
	    hepc->write_count--;
	}
	break;
    }
    if (READREG( addr_isr ) & LINK_READY))
	debug("HEPC timeout cleared\n");
    return;
}

/*############################################################################*/
hepcioctl( dev, cmd, data, mode )

dev_t	dev;
int	cmd;
caddr_t	data;
int	mode;
{

/* Returned Value:	0 - success
 *			non-zero error code from errno.h if failure
 *
 *----------------------------------------------------------------------------
 * Purpose: To perform device specific functions
 *
 * Method:
 *----------------------------------------------------------------------------*/

/* Local Variables */

    register int		unit;	/* unit number */
    register struct hepc_reg	*reg;	/* register pointer */
    register struct hepc_device	*hepc;	/* device structure pointer */
    int				value;	/* data value */
    short		addr_isr;	/* address of isr */
    short		addr_osr;	/* address of osr */
    short		addr_reset;	/* address of reset */
    short		addr_intcr;	/* address of intcr */

/*============================================================================*/
    
    /*
     * Initialisation and general checking
     */
    unit = HEPCUNIT( dev );
    if ( unit >= NHEPC ) 
    {
	u.u_error = ENXIO;
	return;
    }

    debug( "hepc%d:  ioctl %x, status = %x\n", 
		unit, cmd, hepcdevice[unit].status );

    hepc = &hepcdevice[unit];

    if ( hepc->status & STS_DISABLED )
    {
	u.u_error = ENXIO;
	return;
    }

    /*
     * Get addresses of device registers
     */
    reg = board_address[unit];

    addr_isr	   = (short)&(reg->isr);
    addr_osr	   = (short)&(reg->osr);
    addr_reset     = (short)&(reg->reset);
    addr_intcr	   = (short)&(reg->intcr);
    /*
     * Process command
     */
    switch( cmd )
    {
      case HEPC_RESET:
	/*
	 * Reset the device;  first check no i/o in progress	
	 */
	if ( hepc->iomode != 0 )
	{
	    u.u_error = EBUSY;
	    return;
	}

	WRITEREG( addr_reset, TIM40_ASRT_RESET);
	DELAY( 1000 );
	WRITEREG( addr_reset, TIM40_RLSE_RESET);
	DELAY( 1000 );
	/*
	 * Disable interrupts
	 */
	WRITEREG( addr_intcr, 0 );
	hepc->intcr = 0;

	break;

      case HEPC_TIMEOUT:
        /*
         * Set timeout for i/o
         */
        hepc->timeout = (int)data * HZ / 10;
        debug( "hepc%d:  timeout set to %d ticks\n", unit, hepc->timeout );
        break;

      case HEPC_INPUT_PENDING:
	/*
	 * Check input status register
	 */
	debug( "hepc%d:  ISR at %x contains %x\n", 
		unit, addr_isr, READREG( addr_isr ) );
	if ( (READREG( addr_isr ) & LINK_READY) == LINK_READY )
	{
	    value = 1;
	}
	else
	{
	    value = 0;
	}
	copyout( (caddr_t)&value, data, sizeof(value) );

	break;

      case HEPC_OUTPUT_READY:
        /*
         * Check output status register
         */
        debug( "hepc%d:  OSR at %x contains %x\n",
                unit, addr_osr, READREG( addr_osr ) );
        if ( (READREG( addr_osr ) & LINK_READY) == LINK_READY )
	{
	    value = 1;
	}
	else
	{
	    value = 0;
	}
	copyout( (caddr_t)&value, data, sizeof(value) );

        break;
 
      case HEPC_DEBUG:
	/*
	 * Turn on debugging, or output debugging information
	 */
	switch ( (int)data )
	{
	  case 0:
	  case 1:
	    debug_flag = (int)data;
	    break;

	  case 2:
	    printf( "hepc%d:  isr = %x, osr = %x, err = %x\n", 
		    unit, READREG(addr_isr), READREG(addr_osr), 
		    READREG(addr_reset) );
	    break;

	  case 3:
	    printf( "hepc%d:  intcr = %x, iomode = %x\n", 
		    unit, hepc->intcr, hepc->iomode );
	    break;
	  default:
	    break;
	}
	break;

#ifdef HEPCDEBUG

        case HEPC_READ_REG:
  	/*
	 * Read register
	 */
	copyin( data, (caddr_t)&value, sizeof(value) );
	value = (int)inb( (short)value );
	copyout( (caddr_t)&value, data, sizeof(value) );
	break;

      case HEPC_WRITE_REG:
	/*
	 * Write to register
	 */
	{
	    unsigned short s1, *s_ptr;
	    u_char	s2;

	    copyin( data, (caddr_t)&value, sizeof(value) );
	    s_ptr = (unsigned short *) &value;
	    s1 = *s_ptr++;
	    s2 = *(u_char *)s_ptr;
	    outb( s1, s2 );
	}
	break;
#endif
      default:

	debug( "hepc%d:  Bad ioctl command 0x%x\n", unit, cmd );
	u.u_error = ENOTTY;
	return;
    }

    return;
}



/*############################################################################*/
hepcintr( irq )

int irq;        /* interrupt request channel */

{

/* Returned Value:	None
 *
 *----------------------------------------------------------------------------
 * Purpose: Handle interrupts from hepc
 *
 * Method:  If something is ready wake up the top half. We could call start
 *	    down here, but for a big transfer that could mean that we stay at
 *   	    interrupt level for rather a long time, so we avoid that.
 *----------------------------------------------------------------------------*/

/* Local Functions Called */

/* Local Variables */

    register struct hepc_reg	*reg;		/* hepc registers */
    register struct hepc_device	*hepc;		/* device structure ptr */
    short			addr_isr;	/* address of isr */
    short			addr_osr;	/* address of osr */
    short			addr_intcr;	/* address of intcr */
    short			addr_intsr;	/* address of intsr */
    int				unit;		/* unit number */

/*============================================================================*/

    /* 
     * Initialisation
     */
    debug( "hepcintr irq = %d\n", irq );

    for( unit = 0;  unit < NHEPC;  unit++ )
    {
	hepc = &hepcdevice[unit];
	if ( !hepc->initialised ) continue;
	if ( hepc->status & STS_DISABLED ) continue;

        /*
         * Check the interrupt channel
         */
        if ( hepc->int_chan == irq )
	{
	    debug( "hepcintr:  unit = %d\n", unit );
	    reg = board_address[unit];

	    addr_intsr	= (short)&(reg->intsr);
	    addr_intcr	= (short)&(reg->intcr);
	    addr_isr	= (short)&(reg->isr);
	    addr_osr	= (short)&(reg->osr);
	    /*
	     * Kill all interrupts until we're done
	     */
	    WRITEREG( addr_intcr, 0 );
	    /*
	     * Process input ready interrupt;  wake up sleeping process
	     */
	    if ( (hepc->intcr & INT_RECEIVE_READY) &&
		 ( READREG( addr_isr ) & LINK_READY ) )
	    {
		debug( "hepcintr:  read interrupt\n" );

		/*
		 * Clear interrupt flag
		 */
		hepc->intcr &= ~INT_RECEIVE_READY;
		/*
		 * Wake up sleeping process
		 */
		if ( hepc->iomode & HEPC_READ_SLEEP )
		{	
		    wakeup( (caddr_t)&(hepc->read_bp) );
		}
	    }
	    /*
	     * Process output ready interrupt;  wake up sleeping process
	     */
	    if ( (hepc->intcr & INT_SEND_READY) &&
		 ( READREG( addr_osr ) & LINK_READY ) )
	    {
		debug( "hepcintr:  write interrupt\n" );

		/*
		 * Link ready - clear interrupt flag
		 */
		hepc->intcr &= ~INT_SEND_READY;
		/*
		 * Wake up process
		 */
		if ( hepc->iomode & HEPC_WRITE_SLEEP )
		{
		    wakeup( (caddr_t)&(hepc->write_bp) );
		}
	    }
	    /*
	     * Re-enable interrupts
	     */
	    WRITEREG( addr_intcr, hepc->intcr );

	    break;
	}
    }
    return;
}

/*############################################################################*/static void hepctimeout( arg )

int     arg;
{

/*----------------------------------------------------------------------------
 * Purpose: To handle i/o timeouts
 *
 * Method:
 *----------------------------------------------------------------------------*/

    /* Local variables */

    int         unit;

/*============================================================================*/

    /*
     * Extract unit number from argument
     */
    unit = arg >> 16;
    /*
     * Set read or write abort flag, and wake sleeping process
     */
    if ( arg & B_READ )
    {  
        debug( "hepc%d: read timeout\n", unit );
        hepcdevice[unit].read_abort |= ABORT_TIMEOUT;
        wakeup( (caddr_t)&(hepcdevice[unit].read_bp) );
    }
    else
    {
        debug( "hepc%d: write timeout\n", unit );
        hepcdevice[unit].write_abort |= ABORT_TIMEOUT;
        wakeup( (caddr_t)&(hepcdevice[unit].write_bp) );
    }
 
    return;
}

/*############################################################################*/
static int hepc_delay( n, m )

register int n;
register int m;

{

/*----------------------------------------------------------------------------
 * Purpose: To delay for n microseconds;  m is always 0
 *
 * Method:
 *----------------------------------------------------------------------------*/
/*============================================================================*/

    n <<= 3;

    while(--n > m );
    return( n );
}
