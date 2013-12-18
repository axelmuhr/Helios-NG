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

#include "nidiodefs.h"

/* Global Constants */
/* ================ */

#define BANNER "\nNational Instruments AT-DIO-32F device driver for UNIX System V.4, version 1.0\n\
Copyright (C) 1992, Perihelion Software Ltd.  All rights reserved.\n"

#define TRUE	1
#define FALSE	0

/*
 * Define number of devices here as 3, corresponding to 3 units at 3 possible
 * base addresses.
 */
#define	NNIDIO	1	

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

#define	NIDIO_READ		0x0001
#define	NIDIO_WRITE		0x0002
#define NIDIO_READ_SLEEP	0x0010
#define NIDIO_WRITE_SLEEP	0x0020

/* Aborted i/o flags */
 
#define         ABORT_SIGNAL    0x1
#define         ABORT_TIMEOUT   0x2
 
/* Global Macros */
/* ============= */

/*
 * Macro to extract unit number from device information.
 */
#define NIDIOUNIT(dev)	(minor(dev)>>3)
#define NIDIO_INT(dev)	11

/*
 * Macros to access the device registers in i/o space
 */
#define	READBYTE(x)	(inb((int)x))
#define WRITEBYTE(x,y)	(outb((int)x,(u_char)y))
#define	READWORD(x)	(inw((int)x))
#define WRITEWORD(x,y)	(outw((int)x,(u_short)y))
/*
 * Macro to print debug messages if debug flag set
 */
#define debug		if(debug_flag)printf

/* 
 * Macro to delay for a specified number of microseconds
 */
#define DELAY(n)	(void)nidio_delay(n,0);


/* External Routines */
/* ================= */

extern void	outb();
extern void	outw();
extern u_char	inb();
extern u_short	inw();


/* Static functions */
/* ================ */

static void	nidiostrategy();
static void	nidiostart();
static void	nidiotimeout();

/* Global Variables */
/* ================ */

/* Possible board addresses */

static struct nidio_reg *board_address[1] = 
{
    (struct nidio_reg *)0x240
};

/* Flags */

static int	debug_flag = 0;	/* debug flag */

/* static buffer headers for physio */

static struct buf       nidio_readbufs[NNIDIO];
static struct buf       nidio_writebufs[NNIDIO];

/* Device status information */

static struct nidio_device
{
    int		status;		/* device status */
    int		initialised;	/* device initialised */
    int		iomode;		/* current i/o mode */
    int		read_abort;	/* read abort flags */
    int		write_abort;	/* write abort flags */
    int		timeout;	/* i/o timeout */
    int		int_chan;	/* interrupt channel */
    int		uid;		/* current user id */
    char	cfg1;		/* copy of cfg1 */
    struct buf	*read_bp;	/* current read buf */
    struct buf	*write_bp;	/* current write buf */
    int		read_count;	/* bytes to xfer in read */
    int		write_count;	/* bytes to xfer in write */
    int		read_offset;	/* current offset for read */
    int		write_offset;	/* current offset for write */
} nidiodevice[NNIDIO];


/*############################################################################*/
nidioinit( dev )

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

    register struct nidio_reg	*reg;		/* device registers */
    register struct nidio_device *nidio;	/* device structure */
    int				unit;		/* unit number */
    int				addr_cfg1;	/* CFG1 register */
    int				addr_cfg2;	/* CFG2 register */
    int				addr_cfg3;	/* CFG3 register */
    int				addr_cfg4;	/* CFG4 register */
    int				addr_cntrcmd;	/* CNTRCMD register */
    int				addr_dmaclr1;	/* DMACLR1 register */
    int				addr_dmaclr2;	/* DMACLR2 register */
    int				addr_cntintclr;	/* CNTINTCLR register */


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
	for( unit = 0;  unit < NNIDIO;  unit++ )
	{
	    nidiodevice[unit].status = STS_DISABLED;
	}

	first_time = FALSE;
    }

    /*
     * Initialisation
     */
    unit = NIDIOUNIT(dev);
    if ( (unit < 0) || (unit >= NNIDIO) )
    {
	return ENXIO;
    }

    reg = board_address[unit];
    nidio = &nidiodevice[unit];

    addr_cfg1 = (int)&(reg->cfg1);
    addr_cfg2 = (int)&(reg->cfg2);
    addr_cfg3 = (int)&(reg->cfg3);
    addr_cfg4 = (int)&(reg->cfg4);
    addr_cntrcmd = (int)&(reg->cntrcmd);
    addr_dmaclr1 = (int)&(reg->dmaclr1);
    addr_dmaclr2 = (int)&(reg->dmaclr2);
    addr_cntintclr = (int)&(reg->cntintclr);

    debug("nidio%d:  init, reg = %x\n", unit, reg );

    /*
     * Initialise device structure
     */
    nidio->status = STS_AVAILABLE;
    nidio->iomode = 0;
    nidio->uid = -1;
    nidio->int_chan = NIDIO_INT(dev);
    /*
     * Reset the device
     */

    WRITEWORD( addr_cfg1, LRESET1 );
    WRITEWORD( addr_cfg2, LRESET2 );
    WRITEWORD( addr_cfg3, 0 );
    WRITEWORD( addr_cfg1, 0 );
    WRITEWORD( addr_cfg2, 0 );
    WRITEWORD( addr_cfg4, REVC );
    WRITEBYTE( addr_cntrcmd, 0x14 );
    WRITEBYTE( addr_cntrcmd, 0x54 );
    WRITEWORD( addr_dmaclr1, 0 );
    WRITEWORD( addr_dmaclr2, 0 );
    WRITEWORD( addr_cntintclr, 0 );

    /*
     * check the status registers; 
     * input should not be ready, output should be ready
     */
    nidio->cfg1 = 0;

    return( 0 );
}


/*############################################################################*/
nidioopen( dev, flags, otyp )

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
    int			rval;		/* return value */

/*============================================================================*/

    unit = NIDIOUNIT( dev );

    /*
     * First time round initialise the device
     */
    if ( !nidiodevice[unit].initialised )
    {
	rval = nidioinit( dev );
	if ( rval != 0 )
	{
	    u.u_error = rval;
	    return;
	}
	nidiodevice[unit].initialised = TRUE;
    }

    /*
     * Check device is usable
     */
    if ( unit >= NNIDIO || (nidiodevice[unit].status & STS_DISABLED) ) 
    {
	u.u_error = ENXIO;
	return;
    }


    /*
     * If already open check for match of uid
     */
    if ( nidiodevice[unit].status & STS_OPEN ) 
    {
	if ( u.u_uid == nidiodevice[unit].uid ) 
	{
	    debug( "nidio%d: Open (2) by process %d\n", unit, u.u_procp->p_pid );
	    return;
	}
	else
	{
	    u.u_error = EBUSY;
	    return;
	}
    }

    debug( "nidio%d: Open (1) by process %d\n", unit, u.u_procp->p_pid );
    /*
     * Set status to open, and save user details
     */
    nidiodevice[unit].status |= STS_OPEN;
    nidiodevice[unit].uid  = u.u_uid;
    nidiodevice[unit].timeout = 0;

    return;
}


/*############################################################################*/
nidioclose( dev, flags, otyp )

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
    struct nidio_reg	*reg;		/* device structure pointer */
    short		addr_cfg1;	/* address of CFG1 register */
    short		addr_cfg2;	/* address of CFG2 register */
    short		addr_dmaclr1;	/* address of DMACLR1 register */
    short		addr_dmaclr2;	/* address of DMACLR2 register */
    short		addr_cntintclr;	/* address of CNTINTCLR register */

/*============================================================================*/

    /*
     * Extract unit number from device, and check its valid
     */
    unit = NIDIOUNIT( dev );

    if ( unit < NNIDIO )
    {
	debug( "nidio%d: close\n", unit );
	/*
	 * Disable interrupts 
	 */
	reg = (struct nidio_reg *)board_address[unit];

	addr_cfg1 = (short)&(reg->cfg1);
	addr_cfg2 = (short)&(reg->cfg2);
	addr_dmaclr1 = (short)&(reg->dmaclr1);
	addr_dmaclr2 = (short)&(reg->dmaclr2);
	addr_cntintclr = (short)&(reg->cntintclr);

	WRITEWORD( addr_cfg1, 0 );
	WRITEWORD( addr_cfg2, 0 );
	WRITEWORD( addr_dmaclr1, 0 );
	WRITEWORD( addr_dmaclr2, 0 );
	WRITEWORD( addr_cntintclr, 0 );

	/*
	 * Clear device details
	 */
	nidiodevice[unit].iomode	= 0;
	nidiodevice[unit].cfg1 	= 0;
	nidiodevice[unit].status &= ~STS_OPEN;
	nidiodevice[unit].uid	= -1;
    }
    return;
}

/*############################################################################*/
nidioread( dev )

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

    register struct nidio_reg	*reg;		/* nidio registers */
    register struct nidio_device	*nidio;		/* device structure pointer */
    int 			unit;		/* unit number */

/*============================================================================*/

    /*
     * Extract unit number and check it
     */
    unit = NIDIOUNIT( dev );

    debug( "nidio%d: read\n", unit );

    if ( unit >= NNIDIO ) 
    {
	u.u_error = ENXIO;
	return;
    }

    /*
     * Initialisation
     */
    nidio = &nidiodevice[unit];
    reg = board_address[unit];

    /*
     * Let physio do the work
     */

    physio( nidiostrategy,
		&nidio_readbufs[unit],
		dev,
		B_READ );

    debug( "nidio%d:  read done\n", unit );

    return;
}
/*############################################################################*/
nidiowrite( dev )


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

    register struct nidio_reg	*reg;		/* nidio registers */
    register struct nidio_device	*nidio;		/* device structure ptr */
    int 			unit;		/* unit number */

/*============================================================================*/

    /*
     * Extract and check unit number
     */
    unit = NIDIOUNIT( dev );

    debug( "nidio%d: write\n", unit );

    if ( unit >= NNIDIO ) 
    {
	u.u_error = ENXIO;
	return;
    }

    /*
     * Initialisation
     */
    nidio = &nidiodevice[unit];
    reg = board_address[unit];

    /*
     * Call physio to do the work
     */

    physio( nidiostrategy,
		&nidio_writebufs[unit],
		dev,
		B_WRITE );

    debug( "nidio%d:  write done\n", unit );

    return;
}

/*############################################################################*/
static void nidiostrategy( bp )

register struct buf	*bp;
{

/*----------------------------------------------------------------------------
 * Purpose: High level i/o routine
 *
 * Method:
 *----------------------------------------------------------------------------*/

/* Local Variables */

    register struct nidio_reg	*reg;		/* nidio registers */
    register struct nidio_device *nidio;	/* device structure */
    register int		unit;		/* unit number */
    int				old_pri;	/* saved priority */
    unsigned int		arg_tmo;	/* timeout argument */
    int				addr_stat;	/* STAT register */
    int				addr_cfg1;	/* CFG1 register */

/*============================================================================*/

    /*
     * Initialisation
     */
    unit = NIDIOUNIT( bp->b_dev );
    nidio = &nidiodevice[unit];
    reg = board_address[unit];

    addr_stat	= (short)&(reg->STAT);
    addr_cfg1	= (short)&(reg->cfg1);

    /*
     * Either do a read or a write
     */
    switch( bp->b_flags & B_READ )
    {
      case B_READ:

	debug( "nidio%d:  read %d bytes\n", unit, bp->b_bcount );

	nidio->read_count = bp->b_bcount;
	/*
	 * Sleep until link ready, or something else happens
	 */
	nidio->read_abort = 0;

        while( ( (READWORD(addr_stat) & DRDY1) == 0 ) &&
               (nidio->read_abort == 0) )
	{
	    /*
	     * Up priority, enable interrupts and then sleep
	     */
	    old_pri = spl5();

	    nidio->cfg1 |= INTEN1;
	    WRITEBYTE( addr_cfg1, nidio->cfg1 );
	    /*
	     * Sleep, unless its now ready. We must check it
	     * after re-enabling interrupts, because we may
	     * not get an interrupt if it was ready when we
	     * enabled interrupts.
	     */
	    if ( (READWORD(addr_stat) & DRDY1) == 0 )
	    {
		if ( nidio->timeout > 0 )
		{
		    arg_tmo = (unit<<16) | B_READ;
		    timeout( nidiotimeout, arg_tmo, nidio->timeout );
		}
		nidio->iomode |= NIDIO_READ_SLEEP;

		debug( "nidio%d:  read sleep 1\n", unit );
		if ( sleep( (caddr_t)&(nidio->read_bp), SLEEP_PRI ) )
				    nidio->read_abort |= ABORT_SIGNAL;
		debug( "nidio%d:  read wakeup\n", unit );

		nidio->iomode &= ~NIDIO_READ_SLEEP;

		if ( nidio->timeout > 0 ) untimeout( nidiotimeout, arg_tmo );

		/*
		 * if timed out, but link ready, clear timeout flag
		 */
		if ( (nidio->read_abort & ABORT_TIMEOUT) &&
		     ( (READWORD(addr_stat) & DRDY1) != 0 ) )
		{
		    nidio->read_abort &= ~ABORT_TIMEOUT;
		}
	    }

	    /*
	     * Clear interrupts and lower priority
	     */
	    nidio->cfg1 &= ~INTEN1;
	    WRITEBYTE( addr_cfg1, nidio->cfg1 );

	    splx( old_pri );
	}

	if ( nidio->read_abort != 0 )
	{
	    ;	/* continue */
	}
	else
	{
	    /*
	     * Interrupt driven i/o;  set up pointers for first transfer.
	     */
	    debug( "nidio%d:  link adaptor i/o - %d\n", unit, nidio->read_count );

	    nidio->read_bp    = bp;
	    nidio->read_offset= 0;

	    nidio->iomode |= NIDIO_READ;
	    /*
	     * Start the transfer, then if there's anything left
	     * loop round sleeping then transferring.
	     */
	    nidiostart( nidio, reg, B_READ );

	    while( nidio->read_count > 0 )
	    {
		/*
		 * Sleep until link ready, or something else happens
		 */
 
                while( ( (READWORD(addr_stat) & DRDY1) == 0 ) &&
                       ( nidio->read_abort == 0 ) )
		{
		    /*
		     * Up priority, enable interrupts and then sleep
		     */
		    old_pri = spl5();

		    nidio->cfg1 |= INTEN1;
		    WRITEBYTE( addr_cfg1, nidio->cfg1 );
		    /*
		     * Sleep, unless its now ready. We must check it
		     * after re-enabling interrupts, because we may
		     * not get an interrupt if it was ready when we
		     * enabled interrupts.
		     */
		    if ( (READWORD(addr_stat) & DRDY1) == 0 )
		    {
			if ( nidio->timeout > 0 )
			{
			    arg_tmo = (unit<<16) | B_READ;
			    timeout( nidiotimeout, arg_tmo, nidio->timeout );
			}
			nidio->iomode |= NIDIO_READ_SLEEP;

			debug( "nidio%d:  read sleep 2\n", unit );
			if ( sleep( (caddr_t)&(nidio->read_bp), SLEEP_PRI ) )
					    nidio->read_abort |= ABORT_SIGNAL;
			debug( "nidio%d:  read wakeup\n", unit );

			nidio->iomode &= ~NIDIO_READ_SLEEP;
			if ( nidio->timeout > 0 ) 
				untimeout( nidiotimeout, arg_tmo );

			/*
			 * if timed out, but link ready, clear timeout flag
			 */
			if ( (nidio->read_abort & ABORT_TIMEOUT) &&
			     ( (READWORD(addr_stat) & DRDY1) != 0 ) )
			{
			    nidio->read_abort &= ~ABORT_TIMEOUT;
			}
		    }
		    /*
		     * Clear interrupts and lower priority
		     */
		    nidio->cfg1 &= ~INTEN1;
		    WRITEBYTE( addr_cfg1, nidio->cfg1 );

		    splx( old_pri );
		}
		if ( nidio->read_abort != 0 ) break;
		/*
		 * Transfer the data
		 */
		nidiostart( nidio, reg, B_READ );
	    }
	    nidio->iomode &= ~NIDIO_READ;
	}
        /* 
         * Finish off
         */
        bp->b_resid = nidio->read_count;
        if ( nidio->read_abort != 0 )
        {  
            debug( "nidio%d:  read aborted (%x), resid = %d\n",
                    unit, nidio->read_abort, nidio->read_count );
 
            if ( nidio->read_abort & ABORT_SIGNAL )
            {
                bp->b_flags |= B_ERROR;
                bp->b_error = EINTR;
            }
	}
	break;

      case B_WRITE:

	debug( "nidio%d:  write %d bytes\n", unit, bp->b_bcount );
 
        nidio->write_count = bp->b_bcount;
	/*
	 * Sleep until link ready, or something else happens
	 */
        nidio->write_abort = 0;
          
        while( ( (READWORD(addr_stat) & DRDY1) == 0 ) &&
               (nidio->write_abort == 0) )
	{
	    /*
	     * Up priority, enable interrupts and then sleep
	     */
	    old_pri = spl5();

	    nidio->cfg1 |= INTEN1;
	    WRITEBYTE( addr_cfg1, nidio->cfg1 );
	    /*
	     * Sleep, unless its now ready. We must check it
	     * after re-enabling interrupts, because we may
	     * not get an interrupt if it was ready when we
	     * enabled interrupts.
	     */
	    if ( (READWORD(addr_stat) & DRDY1) == 0 )
	    {
		if ( nidio->timeout > 0 )
		{
		    arg_tmo = (unit<<16)|B_WRITE;
		    timeout( nidiotimeout, arg_tmo, nidio->timeout );
		}
		nidio->iomode |= NIDIO_WRITE_SLEEP;

		debug( "nidio%d:  write sleep 1\n", unit );
	       if ( sleep( (caddr_t)&(nidio->write_bp), SLEEP_PRI ) )
                                    nidio->write_abort |= ABORT_SIGNAL;
		debug( "nidio%d:  write wakeup\n", unit );

		nidio->iomode &= ~NIDIO_WRITE_SLEEP;
		if ( nidio->timeout > 0 ) untimeout( nidiotimeout, arg_tmo );

		/*
		 * if timed out, but link ready, clear timeout flag
		 */
		if ( (nidio->write_abort & ABORT_TIMEOUT) &&
		     ( (READWORD(addr_stat) & DRDY1) != 0 ) )
		{
		    nidio->write_abort &= ~ABORT_TIMEOUT;
		}
	    }
	    /*
	     * Clear interrupts and lower priority
	     */
	    nidio->cfg1 &= ~INTEN1;
	    WRITEBYTE( addr_cfg1, nidio->cfg1 );

	    splx( old_pri );
	}

        if ( nidio->write_abort != 0 )
        { 
            ;           /* no action */
        }
	else
	{
	    /*
	     * Interrupt driven i/o;  set up pointers for first transfer.
	     */
	    nidio->write_bp    = bp;
	    nidio->write_offset= 0;

	    debug( "nidio%d:  link adaptor i/o\n", unit );

	    nidio->iomode |= NIDIO_WRITE;
	    /*
	     * Start the transfer, then if there's anything left loop
	     * round sleeping and transferring until completion.
	     */
	    nidiostart( nidio, reg, B_WRITE );

	    while( nidio->write_count > 0 )
	    {
		/*
		 * Sleep until link ready, or something else happens
		 */
                while( ( (READWORD(addr_stat) & DRDY1) == 0 ) &&
                       (nidio->write_abort == 0) )
		{
		    /*
		     * Up priority, enable interrupts and then sleep
		     */
		    old_pri = spl5();

		    nidio->cfg1 |= INTEN1;
		    WRITEBYTE( addr_cfg1, nidio->cfg1 );
		    /*
		     * Sleep, unless its now ready. We must check it
		     * after re-enabling interrupts, because we may
		     * not get an interrupt if it was ready when we
		     * enabled interrupts.
		     */
		    if ( (READWORD(addr_stat) & DRDY1) == 0 )
		    {
			if ( nidio->timeout > 0 )
			{
			    arg_tmo = (unit<<16)|B_WRITE;
			    timeout( nidiotimeout, arg_tmo, nidio->timeout );
			}
			nidio->iomode |= NIDIO_WRITE_SLEEP;

			debug( "nidio%d:  write sleep 2\n", unit );
			if ( sleep( (caddr_t)&(nidio->write_bp), SLEEP_PRI ) )
                                        nidio->write_abort |= ABORT_SIGNAL;
			debug( "nidio%d:  write wakeup\n", unit );

			nidio->iomode &= ~NIDIO_WRITE_SLEEP;

			if ( nidio->timeout > 0 ) 
				untimeout( nidiotimeout, arg_tmo );

			/*
			 * if timed out, but link ready, clear timeout flag
			 */
			if ( (nidio->write_abort & ABORT_TIMEOUT) &&
			     ( (READWORD(addr_stat) & DRDY1) != 0 ) )
			{
			    nidio->write_abort &= ~ABORT_TIMEOUT;
			}
		    }
		    /*
		     * Clear interrupts and lower priority
		     */
		    nidio->cfg1 &= ~INTEN1;
		    WRITEBYTE( addr_cfg1, nidio->cfg1 );

		    splx( old_pri );
		}
		if ( nidio->write_abort != 0 ) break;
		/*
		 * Transfer the data
		 */
		nidiostart( nidio, reg, B_WRITE );
	    }
	    nidio->iomode &= ~NIDIO_WRITE;
	}
        /*
         * Finish off
         */
        bp->b_resid = nidio->write_count;
         
        if ( nidio->write_abort != 0 )
        {
            debug( "nidio%d:  write aborted (%x), resid = %d\n",
                    unit, nidio->write_abort, nidio->write_count );
 
            if ( nidio->write_abort & ABORT_SIGNAL )
            {
                bp->b_flags |= B_ERROR;
                bp->b_error = EINTR;
            }
        }
	break;
    }
    debug( "nidio%d: i/o done, resid = %d\n", unit, bp->b_resid );
 
    iodone( bp );

    return;
}

/*############################################################################*/
static void nidiostart( nidio, reg, rw )

register struct nidio_device	*nidio;		/* device structure */
register struct nidio_reg	*reg;		/* nidio registers */
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
    short		addr_stat;	/* address of isr */
    short		addr_portd;	/* address of osr */

/*============================================================================*/

    switch( rw )
    {
      case B_READ:
	/*
	 * Set up register addresses
	 */
	addr_stat = (short)&(reg->STAT);
	addr_portd = (short)&(reg->portd);
	/*
	 * Get pointer to next location in buffer
	 */
	cptr = ((u_char *)(nidio->read_bp->b_un.b_addr)) + nidio->read_offset;
	/*
	 * Read data while data is to be read, and the link is ready
	 */
	while( (nidio->read_count > 0) && (READWORD( addr_stat ) & DRDY1) )
	{
	    /*
	     * Read register and update counts
	     */
	    *cptr++ = READBYTE( addr_portd );
	    nidio->read_offset++;
	    nidio->read_count--;
	}
	break;

      case B_WRITE:
	/*
	 * Set up register addresses
	 */
	addr_stat = (short)&(reg->STAT);
	addr_portd = (short)&(reg->portd);
	/*
	 * Get pointer to next location in buffer
	 */
	cptr = ((u_char *)(nidio->write_bp->b_un.b_addr)) + nidio->write_offset;
	/*
	 * Write data while data is to be written, and the link is ready
	 */
	while( (nidio->write_count > 0) && (READWORD( addr_stat ) & DRDY1) )
	{
	    /*
	     * Write to register, then wait a bit for next write
	     */
	    WRITEBYTE( addr_portd, *cptr++ );
	    nidio->write_offset++;
	    nidio->write_count--;
	}
	break;
    }
    return;
}

/*############################################################################*/
nidioioctl( dev, cmd, data, mode )

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

    register int		unit;	   /* unit number */
    register struct nidio_reg	*reg;	   /* register pointer */
    register struct nidio_device *nidio;   /* device structure pointer */
    int				value;	   /* data value */
    int				addr_stat; /* address of status register */
    int				addr_cfg1;	/* CFG1 register */
    int				addr_cfg2;	/* CFG2 register */
    int				addr_cfg3;	/* CFG3 register */
    int				addr_cfg4;	/* CFG4 register */
    int				addr_cntrcmd;	/* CNTRCMD register */
    int				addr_dmaclr1;	/* DMACLR1 register */
    int				addr_dmaclr2;	/* DMACLR2 register */
    int				addr_cntintclr;	/* CNTINTCLR register */

/*============================================================================*/
    
    /*
     * Initialisation and general checking
     */
    unit = NIDIOUNIT( dev );
    if ( unit >= NNIDIO ) 
    {
	u.u_error = ENXIO;
	return;
    }

    debug( "nidio%d:  ioctl %x, status = %x\n", 
		unit, cmd, nidiodevice[unit].status );

    nidio = &nidiodevice[unit];

    if ( nidio->status & STS_DISABLED )
    {
	u.u_error = ENXIO;
	return;
    }

    /*
     * Get addresses of device registers
     */
    reg = board_address[unit];

    addr_stat	   = (short)&(reg->STAT);
    addr_cfg1 = (int)&(reg->cfg1);
    addr_cfg2 = (int)&(reg->cfg2);
    addr_cfg3 = (int)&(reg->cfg3);
    addr_cfg4 = (int)&(reg->cfg4);
    addr_cntrcmd = (int)&(reg->cntrcmd);
    addr_dmaclr1 = (int)&(reg->dmaclr1);
    addr_dmaclr2 = (int)&(reg->dmaclr2);
    addr_cntintclr = (int)&(reg->cntintclr);
    /*
     * Process command
     */
    switch( cmd )
    {
      case NIDIO_RESET:
	/*
	 * Reset the device;  first check no i/o in progress	
	 */
	if ( nidio->iomode != 0 )
	{
	    u.u_error = EBUSY;
	    return;
	}

        WRITEWORD( addr_cfg1, LRESET1 );
        WRITEWORD( addr_cfg2, LRESET2 );
        WRITEWORD( addr_cfg3, 0 );
        WRITEWORD( addr_cfg1, 0 );
        WRITEWORD( addr_cfg2, 0 );
        WRITEWORD( addr_cfg4, REVC );
        WRITEBYTE( addr_cntrcmd, 0x14 );
        WRITEBYTE( addr_cntrcmd, 0x54 );
        WRITEWORD( addr_dmaclr1, 0 );
        WRITEWORD( addr_dmaclr2, 0 );
        WRITEWORD( addr_cntintclr, 0 );

	nidio->cfg1 = 0;

	break;

      case NIDIO_TIMEOUT:
        /*
         * Set timeout for i/o
         */
        nidio->timeout = (int)data * HZ / 10;
        debug( "nidio%d:  timeout set to %d ticks\n", unit, nidio->timeout );
        break;

      case NIDIO_INPUT_PENDING:
	/*
	 * Check input status register
	 */
	debug( "nidio%d:  ISR at %x contains %x\n", 
		unit, addr_stat, READWORD( addr_stat ) );
	if ( (READWORD( addr_stat ) & DRDY1) == DRDY1 )
	{
	    value = 1;
	}
	else
	{
	    value = 0;
	}
	copyout( (caddr_t)&value, data, sizeof(value) );

	break;

      case NIDIO_OUTPUT_READY:
        /*
         * Check output status register
         */
        debug( "nidio%d:  OSR at %x contains %x\n",
                unit, addr_stat, READWORD( addr_stat ) );
        if ( (READWORD( addr_stat ) & DRDY1) == DRDY1 )
	{
	    value = 1;
	}
	else
	{
	    value = 0;
	}
	copyout( (caddr_t)&value, data, sizeof(value) );

        break;
 
      case NIDIO_DEBUG:
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
	    printf( "nidio%d:  status = %x\n", unit, READWORD(addr_stat) );
	    break;

	  case 3:
	    printf( "nidio%d:  cfg1 = %x, iomode = %x\n", 
		    unit, nidio->cfg1, nidio->iomode );
	    break;
	  default:
	    break;
	}
	break;

#ifdef NIDIODEBUG

        case NIDIO_READ_REG:
  	/*
	 * Read register
	 */
	copyin( data, (caddr_t)&value, sizeof(value) );
	value = (int)inw( (int)value );
	copyout( (caddr_t)&value, data, sizeof(value) );
	break;

      case NIDIO_WRITE_REG:
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
	    outw( s1, s2 );
	}
	break;
#endif
      default:

	debug( "nidio%d:  Bad ioctl command 0x%x\n", unit, cmd );
	u.u_error = ENOTTY;
	return;
    }

    return;
}



/*############################################################################*/
nidiointr( irq )

int irq;        /* interrupt request channel */

{

/* Returned Value:	None
 *
 *----------------------------------------------------------------------------
 * Purpose: Handle interrupts from nidio
 *
 * Method:  If something is ready wake up the top half. We could call start
 *	    down here, but for a big transfer that could mean that we stay at
 *   	    interrupt level for rather a long time, so we avoid that.
 *----------------------------------------------------------------------------*/

/* Local Functions Called */

/* Local Variables */

    register struct nidio_reg	*reg;		/* nidio registers */
    register struct nidio_device	*nidio;		/* device structure ptr */
    int				addr_cfg1;	/* address of CFG1 */
    int				addr_stat;	/* address of STAT */
    int				unit;		/* unit number */

/*============================================================================*/

    /* 
     * Initialisation
     */
    debug( "nidiointr irq = %d\n", irq );

    for( unit = 0;  unit < NNIDIO;  unit++ )
    {
	nidio = &nidiodevice[unit];
	if ( !nidio->initialised ) continue;
	if ( nidio->status & STS_DISABLED ) continue;

        /*
         * Check the interrupt channel
         */
        if ( nidio->int_chan == irq )
	{
	    debug( "nidiointr:  unit = %d\n", unit );
	    reg = board_address[unit];

	    addr_stat	= (short)&(reg->STAT);
	    addr_cfg1	= (short)&(reg->cfg1);
	    /*
	     * Kill all interrupts until we're done
	     */
	    WRITEBYTE( addr_cfg1, 0 );
	    /*
	     * Process input ready interrupt;  wake up sleeping process
	     */
	    if ( (nidio->cfg1 & INTEN1) &&
		 ( READWORD( addr_stat ) & DRDY1 ) )
	    {
		debug( "nidiointr:  read interrupt\n" );

		/*
		 * Clear interrupt flag
		 */
		nidio->cfg1 &= ~INTEN1;
		/*
		 * Wake up sleeping process
		 */
		if ( nidio->iomode & NIDIO_READ_SLEEP )
		{	
		    wakeup( (caddr_t)&(nidio->read_bp) );
		}
	    }
	    /*
	     * Process output ready interrupt;  wake up sleeping process
	     */
	    if ( (nidio->cfg1 & INTEN1) &&
		 ( READWORD( addr_stat ) & DRDY1 ) )
	    {
		debug( "nidiointr:  write interrupt\n" );

		/*
		 * Link ready - clear interrupt flag
		 */
		nidio->cfg1 &= ~INTEN1;
		/*
		 * Wake up process
		 */
		if ( nidio->iomode & NIDIO_WRITE_SLEEP )
		{
		    wakeup( (caddr_t)&(nidio->write_bp) );
		}
	    }
	    /*
	     * Re-enable interrupts
	     */
	    WRITEBYTE( addr_cfg1, nidio->cfg1 );

	    break;
	}
    }
    return;
}

/*############################################################################*/static void nidiotimeout( arg )

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
        debug( "nidio%d: read timeout\n", unit );
        nidiodevice[unit].read_abort |= ABORT_TIMEOUT;
        wakeup( (caddr_t)&(nidiodevice[unit].read_bp) );
    }
    else
    {
        debug( "nidio%d: write timeout\n", unit );
        nidiodevice[unit].write_abort |= ABORT_TIMEOUT;
        wakeup( (caddr_t)&(nidiodevice[unit].write_bp) );
    }
 
    return;
}

/*############################################################################*/
static int nidio_delay( n, m )

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
