
/*#############################################################################
 *
 *	Copyright (C) 1989,1990 K-par Systems  Ltd.  All rights reserved
 *
 * Program/Library:	bbk Interactive BBK-PC driver bbk_driver.c
 *
 * Purpose: 		Driver source code
 *
 * Author:		Chris Farey 27-Apr-1989, 15-Oct-1990
 *			Modified by Tony Cruickshank 5-July-1993
 *
 *---------------------------------------------------------------------------*/
/* Include Files */
/* ============= */

#define _INKERNEL

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

#include "bbkdefs.h"

/* Global Constants */
/* ================ */

#define BANNER "\nPDS Ltd \"bbk\" device driver for INMOS BBK-PC board,  version 1.1\n\
Copyright (C) Perihelion Software Ltd.  All rights reserved.\n"

#define TRUE	1
#define FALSE	0

/*
 * Define number of devices here as 3, corresponding to 3 units at 3 possible
 * base addresses.
 */
#define	NBBK	3

/*
 * Define sleep priority as an interruptable by sugnals, with
 * signals caught.
 */
#define SLEEP_PRI	(PZERO+1|PCATCH)

/* Maximum transfer size for physio */

#define MAX_BLOCKSIZE		4096

/* Device status values */

#define STS_AVAILABLE		0
#define STS_OPEN		0x0001
#define STS_DISABLED		0x0002

/* I/o modes */

#define	BBK_READ	0x0001
#define	BBK_WRITE	0x0002
#define BBK_READ_SLEEP	0x0010
#define BBK_WRITE_SLEEP	0x0020

/* Aborted i/o flags */
 
#define         ABORT_SIGNAL    0x1
#define         ABORT_TIMEOUT   0x2
#define         ABORT_ERROR     0x4
 

/* Global Macros */
/* ============= */

/*
 * We encode in the minor device number the following:
 *
 *	bits 0,1	Unused - (was used for DMA in kpar driver)
 *			
 *			
 *			
 *    
 *	bit 2		Interrupt channel, 0 = 3, 1 = 5
 *
 *	bits 3, 4	Unit number, where 0 uses address 0x150,
 *				           1 uses address 0x200
 *					   2 uses address 0x300
 *					   3 is invalid
 */

/*
 * Macro to extract unit number from device information.
 */
#define BBKUNIT(dev)	(minor(dev)>>3)
#define INT_CHAN(dev)	((minor(dev)&4)?5:3)

/*
 * Macros to access the device registers in i/o space
 */
#define	READREG(x)	(inb((short)x))
#define WRITEREG(x,y)	(outb((short)x,(u_char)y))
/*
 * Macro to determine whether to abort i/o because transputer error
 * flag is set, and user asked to abort if this happened.
 */
#define ABORT_IO(bbk,addr_error) \
	(bbk->error_abort&&(((int)READREG(addr_error)&1)==error_set))
/*
 * Macro to print debug messages if debug_flag set
 */
#define debug		if(debug_flag)printf
/* 
 * Macro to delay for a specified number of microseconds
 */
#define DELAY(n)	(void)bbk_delay(n,0);


/* External Routines */
/* ================= */

extern void	outb();
extern u_char	inb();


/* Static functions */
/* ================ */

static void	bbkstrategy();
static void	bbkstart();
static void	bbktimeout();
/* static int 	bbkinit(); */

/* Global Variables */
/* ================ */

/* Possible board addresses */

static struct b008_reg *board_address[3] = 
{
    (struct b008_reg *)0x150, 
    (struct b008_reg *)0x200, 
    (struct b008_reg *)0x300
};

/* Flags */

static int	debug_flag = 0;	/* debug_flag */
static int	error_set = 0;	/* error flag set state */

/* static buffer headers for physio */

static struct buf	bbk_readbufs[NBBK];	
static struct buf	bbk_writebufs[NBBK];


/* Device status information */

static struct bbk_device
{
    int		status;		/* device status */
    int		initialised;	/* device initialised */
    int		iomode;		/* current i/o mode */
    int		read_abort;	/* read abort flags */
    int		write_abort;	/* write abort flags */
    int		error_abort;	/* flag to abort i/o on error */
    int		timeout;	/* i/o timeout */
    int		int_chan;	/* interrupt channel */
    int		uid;		/* current user id */
    char	icr;		/* copy of icr - used to keep track of
				   what is going on, but not read/written
				   in the registers - Tony (6/7/93)
				 */
    struct buf	*read_bp;	/* current read buf */
    struct buf	*write_bp;	/* current write buf */
    int		read_count;	/* bytes to xfer in read */
    int		write_count;	/* bytes to xfer in write */
    int		read_offset;	/* current offset for read */
    int		write_offset;	/* current offset for write */
} bbkdevice[NBBK];

#define enable_interrupt(addr)	WRITEREG(addr, (READREG(addr)|2))
#define disable_interrupt(addr)	WRITEREG(addr, (READREG(addr)&(~2)))

#ifdef TDEBUG
/* TDL = Tony Debug Level */
#ifndef TDL
#define TDL -1
#endif

#define check_interrupts_(l,n,reg)	if (l > TDL) check_interrupts (n, reg);
#else
#define check_interrupts_(l,n,reg)
#endif

#ifdef TDEBUG

void check_interrupts (n, reg)
int			n;
struct b008_reg *	reg;
{
	short	addr_isr = (short)&(reg->isr);
	short	addr_osr = (short)&(reg->osr);
	char	isr = READREG (addr_isr);
	char	osr = READREG (addr_osr);
	int	i,j;

	printf ("check_interrupts (%d, 0x%x) ... \n", n, reg);

	printf ("\tinput status register (isr) = 0x%x\n", isr);

	printf ("\toutput status register (osr) = 0x%x\n", osr);
#ifdef NEVER
	for (i = 0, j = 0; i < 9999999; i++)
	{
		j = i - j;
	}
#endif
}	
#endif

/*############################################################################*/
bbkinit( dev )

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

    register struct b008_reg	*reg;		/* device registers */
    register struct bbk_device	*bbk;		/* device structure */
    int				unit;		/* unit number */
    short			addr_isr;	/* address of isr */
    short			addr_osr;	/* address of osr */
    short			addr_analyse;	/* address of analyse */
    short			addr_reset;	/* address of reset */


/*============================================================================*/

    /*
     * For first device print out banner and do a bit
     * of initialisation.
     */
    if ( first_time )
    {
	debug( BANNER );

	/*
	 * Set all devices disabled until we probe for them
	 */
	for( unit = 0;  unit < NBBK;  unit++ )
	{
	    bbkdevice[unit].status = STS_DISABLED;
	}

	first_time = FALSE;
    }

    /*
     * Initialisation
     */
    unit = BBKUNIT(dev);
    if ( (unit < 0) || (unit > NBBK) )
    {
	return ENXIO;
    }

    reg = board_address[unit];
    bbk = &bbkdevice[unit];

    debug ( "bbkinit () - bbk%d: init - addr = %x, irq = %d\n", 
		unit, reg, INT_CHAN(dev) );

    bbk->int_chan = INT_CHAN(dev);

    addr_isr 	 = (short)&(reg->isr);
    addr_osr	 = (short)&(reg->osr);
    addr_reset	 = (short)&(reg->reset_error);
    addr_analyse = (short)&(reg->analyse);

    debug("bbkinit () - bbk%d:  init, reg = %x\n", unit, reg );

    /*
     * Initialise device structure
     */
    bbk->status = STS_AVAILABLE;
    bbk->iomode = 0;
    bbk->error_abort = FALSE;
    bbk->uid = -1;
    /*
     * Reset the device
     */
    disable_interrupt (addr_isr);
    disable_interrupt (addr_osr);
    check_interrupts_(1, 0, reg);
    DELAY( 1000 );
    WRITEREG( addr_reset, 0);
    DELAY( 1000 );
    WRITEREG( addr_analyse, 0 );
    DELAY( 1000 );
    WRITEREG( addr_reset, 1);
    DELAY( 1000 );
    WRITEREG( addr_reset, 0 );
    DELAY( 1000 );

    /*
     * Enable C012 interrupts and check the C012 status registers; 
     * input should not be ready, output should be ready
     */
    WRITEREG( addr_isr, INT_C012 );
    DELAY( 1000 );
    if ( (READREG( addr_isr ) & LINK_READY) != 0 )
    {
	printf( "bbkinit () - bbk%d: C012 ISR check failed (isr = %x);  disabling device.\n",
		unit, READREG( addr_isr ) );
	bbk->status = STS_DISABLED;
	return EIO;
    }
    WRITEREG( addr_osr, INT_C012 );
    DELAY( 1000 );
    if ( (READREG( addr_osr ) & LINK_READY) != LINK_READY )
    {
	printf( "bbkinit () - bbk%d: C012 OSR check failed( osr = %x);  disabling device.\n",
		unit, READREG( addr_osr ) );
	bbk->status = STS_DISABLED;
	return EIO;
    }
    /*
     * Disable interrupts
     */
    disable_interrupt (addr_isr);
    disable_interrupt (addr_osr);
    debug  ("bbkinit () - reg = 0x%x\n", reg);
    check_interrupts_(1, 100, reg);
    check_interrupts_(1, 101, 0x150);

    bbk->icr = 0;

    return( 0 );
}


/*############################################################################*/
bbkopen( dev, flags, otyp )

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

    unit = BBKUNIT( dev );

    /*
     * First time round initialise the device
     */
    if ( !bbkdevice[unit].initialised )
    {
	debug ("bbkopen () - initialising device %d\n", unit);

	rval = bbkinit( dev );
	if ( rval != 0 )
	{
	    u.u_error = rval;
	    return;
	}
	bbkdevice[unit].initialised = TRUE;
    }

    debug ( "bbkopen () - bbk%d: init - addr = %x, irq = %d\n", 
		unit, board_address[unit], INT_CHAN(dev) );

    /*
     * Check device is usable
     */
    if ( unit >= NBBK || (bbkdevice[unit].status & STS_DISABLED) ) 
    {
	u.u_error = ENXIO;
	return;
    }


    /*
     * If already open check for match of uid
     */
    if ( bbkdevice[unit].status & STS_OPEN ) 
    {
	if ( u.u_uid == bbkdevice[unit].uid ) 
	{
	    debug( "bbkopen () - bbk%d: Open (2) by process %d\n", unit, u.u_procp->p_pid );
	    return;
	}
	else
	{
	    u.u_error = EBUSY;
	    return;
	}
    }

    debug ( "bbkopen () - bbk%d: Open (1) by process %d\n", unit, u.u_procp->p_pid );
    /*
     * Set status to open, and save user details
     */
    bbkdevice[unit].status |= STS_OPEN;
    bbkdevice[unit].uid  = u.u_uid;
    /*
     * Initially abort on errors clear
     */
    bbkdevice[unit].error_abort = FALSE;
    bbkdevice[unit].timeout = 0;

    return;
}


/*############################################################################*/
bbkclose( dev, flags, otyp )

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
    struct b008_reg	*reg;		/* device structure pointer */

/*============================================================================*/

    /*
     * Extract unit number from device, and check its valid
     */
    unit = BBKUNIT( dev );

    if ( unit < NBBK )
    {
	debug( "bbkclose () - bbk%d: close\n", unit );
	/*
	 * Clear interrupt control register
	 */
	reg = (struct b008_reg *)board_address[unit];

	/*
	 * Clear device details
	 */
	bbkdevice[unit].iomode	= 0;
	bbkdevice[unit].icr 	= 0;
	bbkdevice[unit].status &= ~STS_OPEN;
	bbkdevice[unit].uid	= -1;
    }
    return;
}

/*############################################################################*/
bbkread( dev )

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

    register struct b008_reg	*reg;		/* B008 reegisters */
    register struct bbk_device	*bbk;		/* device structure pointer */
    int 			unit;		/* unit number */
    short			addr_error;	/* address of error reg */

/*============================================================================*/

    /*
     * Extract unit number and check it
     */
    unit = BBKUNIT( dev );

    debug( "bbkread () - bbk%d: read\n", unit );

    if ( unit >= NBBK ) 
    {
	u.u_error = ENXIO;
	return;
    }

    /*
     * Initialisation
     */
    bbk = &bbkdevice[unit];
    reg = board_address[unit];

    addr_error = (short)&(reg->reset_error);
    /*
     * Let physio do the work, unless we need to abort because of an error
     */
    if ( !ABORT_IO( bbk, addr_error ) )
    {
	physio( bbkstrategy,
		&bbk_readbufs[unit],
		dev,
		B_READ );
    }

    /*
     * Output message if i/o aborted
     */
    if ( ABORT_IO( bbk, addr_error ) )
    {
	debug( "bbkread () - bbk%d:  Error on transputer - read aborted\n", unit );
	u.u_error = EIO;
	return;
    }
    debug( "bbkread () - bbk%d:  read done\n", unit );

    return;
}
/*############################################################################*/
bbkwrite( dev )


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

    register struct b008_reg	*reg;		/* B008 reegisters */
    register struct bbk_device	*bbk;		/* device structure ptr */
    int 			unit;		/* unit number */
    short			addr_error;	/* address of error register */

/*============================================================================*/

    /*
     * Extract and check unit number
     */
    unit = BBKUNIT( dev );

    debug( "write () - bbk%d: write\n", unit );

    if ( unit >= NBBK ) 
    {
	u.u_error = ENXIO;
	return;
    }

    /*
     * Initialisation
     */
    bbk = &bbkdevice[unit];
    reg = board_address[unit];

    addr_error = (short)&(reg->reset_error);

    /*
     * Call physio to do the work, unless abort is set
     */
    if ( !ABORT_IO( bbk, addr_error ) )
    {
	physio( bbkstrategy,
		&bbk_writebufs[unit],
		dev,
		B_WRITE );
    }
    /*
     * Output message if write aborted
     */
    if ( ABORT_IO( bbk, addr_error ) )
    {
	debug ( "write () - bbk%d:  Error on transputer - write aborted\n", unit );

	u.u_error = EIO;
	return;
    }

    debug( "write () - bbk%d:  write done\n", unit );

    return;
}

/*############################################################################*/
static void bbkstrategy( bp )

register struct buf	*bp;
{

/*----------------------------------------------------------------------------
 * Purpose: High level i/o routine
 *
 * Method:
 *----------------------------------------------------------------------------*/

/* Local Variables */

    register struct b008_reg	*reg;		/* B008 registers */
    register struct bbk_device	*bbk;		/* device structure */
    register int		unit;		/* unit number */
    int				old_pri;	/* saved priority */
    unsigned int		arg_tmo;	/* timeout argument */
    short			addr_isr;	/* address of isr */
    short			addr_osr;	/* address of osr */
    short			addr_error;	/* address of error */

    short			addr_idr;	/* address of idr */
    short			addr_odr;	/* address of odr */

/*============================================================================*/

    /*
     * Initialisation
     */
    unit = BBKUNIT( bp->b_dev );
    bbk = &bbkdevice[unit];
    reg = board_address[unit];

    addr_isr	= (short)&(reg->isr);
    addr_osr	= (short)&(reg->osr);
    addr_error	= (short)&(reg->reset_error);

    addr_idr	= (short)&(reg->idr);
    addr_odr	= (short)&(reg->odr);

    /*
     * Either do a read or a write
     */
    switch( bp->b_flags & B_READ )
    {
      case B_READ:

	debug( "bbkstrategy () - bbk%d:  read %d bytes\n", unit, bp->b_bcount );

	bbk->read_count = bp->b_bcount;
	/*
	 * Sleep until link ready, or something else happens
	 */
	bbk->read_abort = ABORT_IO( bbk, addr_error ) ? ABORT_ERROR : 0;

        while( ( (READREG(addr_isr) & LINK_READY) == 0 ) &&
               (bbk->read_abort == 0) )
	{
	    /*
	     * Up priority, enable interrupts and then sleep
	     */
	    old_pri = spl5();

	    bbk->icr |= INT_RECEIVE_READY;
	    enable_interrupt (addr_isr);
	    check_interrupts_(0, 2, reg);

	    /*
	     * Sleep, unless its now ready. We must check it
	     * after re-enabling interrupts, because we may
	     * not get an interrupt if it was ready when we
	     * enabled interrupts.
	     */
	    if ( (READREG(addr_isr) & LINK_READY) == 0 )
	    {
		if ( bbk->timeout > 0 )
		{
		    arg_tmo = (unit<<16) | B_READ;
		    timeout( bbktimeout, arg_tmo, bbk->timeout );
		}
		bbk->iomode |= BBK_READ_SLEEP;

		debug( "bbkstrategy () - bbk%d:  read sleep 1\n", unit );
		if ( sleep( (caddr_t)&(bbk->read_bp), SLEEP_PRI ) )
				    bbk->read_abort |= ABORT_SIGNAL;
		debug( "bbkstrategy () - bbk%d:  read wakeup\n", unit );

		bbk->iomode &= ~BBK_READ_SLEEP;

		if ( bbk->timeout > 0 ) untimeout( bbktimeout, arg_tmo );

		/*
		 * if timed out, but link ready, clear timeout flag
		 */
		if ( (bbk->read_abort & ABORT_TIMEOUT) &&
		     ( (READREG(addr_isr) & LINK_READY) != 0 ) )
		{
		    bbk->read_abort &= ~ABORT_TIMEOUT;
		}
	    }

	    /*
	     * Clear interrupts and lower priority
	     */
	    bbk->icr &= ~INT_RECEIVE_READY;
	    disable_interrupt (addr_isr);
	    check_interrupts_(1, 3, reg);

	    splx( old_pri );
            /*
             * Re-check error flag
             */
            if ( ABORT_IO( bbk, addr_error ) ) bbk->read_abort |= ABORT_ERROR;
	}

	check_interrupts_(1, 770, reg);
	if ( bbk->read_abort != 0 )
	{
	    ;	/* continue */
	}
	else
	{
	    /*
	     * Interrupt driven i/o;  set up pointers for first transfer.
	     */
	    debug( "bbkstrategy () - bbk%d:  link adaptor i/o - %d\n", unit, bbk->read_count );
	    check_interrupts_(1, 771, reg);

	    bbk->read_bp    = bp;
	    bbk->read_offset= 0;

	    bbk->iomode |= BBK_READ;
	    /*
	     * Start the transfer, then if there's anything left
	     * loop round sleeping then transferring.
	     */
	    bbkstart( bbk, reg, B_READ );

	    while( bbk->read_count > 0 )
	    {
		/*
		 * Sleep until link ready, or something else happens
		 */
		if ( ABORT_IO( bbk, addr_error ) )
                                bbk->read_abort |= ABORT_ERROR;
 
                while( ( (READREG(addr_isr) & LINK_READY) == 0 ) &&
                       ( bbk->read_abort == 0 ) )
		{
		    /*
		     * Up priority, enable interrupts and then sleep
		     */
		    old_pri = spl5();

		    bbk->icr |= INT_RECEIVE_READY;
		    enable_interrupt (addr_isr);
		    check_interrupts_(1, 4, reg);
		    /*
		     * Sleep, unless its now ready. We must check it
		     * after re-enabling interrupts, because we may
		     * not get an interrupt if it was ready when we
		     * enabled interrupts.
		     */
		    if ( (READREG(addr_isr) & LINK_READY) == 0 )
		    {
			if ( bbk->timeout > 0 )
			{
			    arg_tmo = (unit<<16) | B_READ;
			    timeout( bbktimeout, arg_tmo, bbk->timeout );
			}
			bbk->iomode |= BBK_READ_SLEEP;
			debug( "bbkstrategy () - bbk%d:  read sleep 2\n", unit );

			if ( sleep( (caddr_t)&(bbk->read_bp), SLEEP_PRI ) )
			{
				/* print out device register ... */
				printf ("(0x%x) isr = 0x%x\n", addr_isr, READREG (addr_isr));
				printf ("(0x%x) osr = 0x%x\n", addr_osr, READREG (addr_osr));
				printf ("(0x%x) idr = 0x%x\n", addr_idr, READREG (addr_idr));
				printf ("(0x%x) odr = 0x%x\n", addr_odr, READREG (addr_odr));
 			    	bbk->read_abort |= ABORT_SIGNAL;
			}

			debug( "bbkstrategy () - bbk%d:  read wakeup\n", unit );

			bbk->iomode &= ~BBK_READ_SLEEP;
			if ( bbk->timeout > 0 ) 
				untimeout( bbktimeout, arg_tmo );

			/*
			 * if timed out, but link ready, clear timeout flag
			 */
			if ( (bbk->read_abort & ABORT_TIMEOUT) &&
			     ( (READREG(addr_isr) & LINK_READY) != 0 ) )
			{
			    bbk->read_abort &= ~ABORT_TIMEOUT;
			}
		    }
		    /*
		     * Clear interrupts and lower priority
		     */
		    bbk->icr &= ~INT_RECEIVE_READY;
		    disable_interrupt (addr_isr);
		    check_interrupts_(1, 5, reg);

		    splx( old_pri );
                    /*
                     * Check for abort on error
                     */
                    if ( ABORT_IO( bbk, addr_error ) )
                                bbk->read_abort |= ABORT_ERROR;
		}
		if ( bbk->read_abort != 0 ) break;
		/*
		 * Transfer the data
		 */
		bbkstart( bbk, reg, B_READ );
	    }
	    bbk->iomode &= ~BBK_READ;
	}
        /* 
         * Finish off
         */
        bp->b_resid = bbk->read_count;
        if ( bbk->read_abort != 0 )
        {  
            debug( "bbkstrategy () - bbk%d:  read aborted (%x), resid = %d\n",
                    unit, bbk->read_abort, bbk->read_count );
 
            if ( bbk->read_abort & ABORT_SIGNAL )
            {
                bp->b_flags |= B_ERROR;
                bp->b_error = EINTR;
            }
            else if ( bbk->read_abort & ABORT_ERROR )
            {
                bp->b_flags |= B_ERROR;
                bp->b_error = EIO;
            }
	}
	break;

      case B_WRITE:

	debug( "bbkstrategy () [B_WRITE] - bbk%d:  write %d bytes\n", unit, bp->b_bcount );
 
        bbk->write_count = bp->b_bcount;
	/*
	 * Sleep until link ready, or something else happens
	 */
        bbk->write_abort = ABORT_IO( bbk, addr_error ) ? ABORT_ERROR : 0;
          
        while( ( (READREG(addr_osr) & LINK_READY) == 0 ) &&
               (bbk->write_abort == 0) )
	{
	    /*
	     * Up priority, enable interrupts and then sleep
	     */
	    old_pri = spl5();

	    bbk->icr |= INT_SEND_READY;
	    enable_interrupt (addr_osr);
	    check_interrupts_(1, 6, reg);

	    /*
	     * Sleep, unless its now ready. We must check it
	     * after re-enabling interrupts, because we may
	     * not get an interrupt if it was ready when we
	     * enabled interrupts.
	     */
	    if ( (READREG(addr_osr) & LINK_READY) == 0 )
	    {
		if ( bbk->timeout > 0 )
		{
		    arg_tmo = (unit<<16)|B_WRITE;
		    timeout( bbktimeout, arg_tmo, bbk->timeout );
		}
		bbk->iomode |= BBK_WRITE_SLEEP;

		debug( "bbkstrategy () [B_WRITE] - bbk%d:  write sleep 1\n", unit );
	       if ( sleep( (caddr_t)&(bbk->write_bp), SLEEP_PRI ) )
                                    bbk->write_abort |= ABORT_SIGNAL;
		debug( "bbkstrategy () [B_WRITE] - bbk%d:  write wakeup\n", unit );
		bbk->iomode &= ~BBK_WRITE_SLEEP;
		if ( bbk->timeout > 0 ) untimeout( bbktimeout, arg_tmo );

		/*
		 * if timed out, but link ready, clear timeout flag
		 */
		if ( (bbk->write_abort & ABORT_TIMEOUT) &&
		     ( (READREG(addr_osr) & LINK_READY) != 0 ) )
		{
		    bbk->write_abort &= ~ABORT_TIMEOUT;
		}
	    }
	    /*
	     * Clear interrupts and lower priority
	     */
	    bbk->icr &= ~INT_SEND_READY;
	    disable_interrupt (addr_osr);
	    check_interrupts_(1, 7, reg);

	    splx( old_pri );
            /*
             * Re-check error flag
             */
            if ( ABORT_IO( bbk, addr_error ) ) bbk->write_abort |= ABORT_ERROR;
	}

        if ( bbk->write_abort != 0 )
        { 
            ;           /* no action */
        }
	else
	{
	    /*
	     * Interrupt driven i/o;  set up pointers for first transfer.
	     */
	    bbk->write_bp    = bp;
	    bbk->write_offset= 0;

	    debug( "bbkstrategy () [B_WRITE] - bbk%d:  link adaptor i/o\n", unit );

	    bbk->iomode |= BBK_WRITE;
	    /*
	     * Start the transfer, then if there's anything left loop
	     * round sleeping and transferring until completion.
	     */
	    bbkstart( bbk, reg, B_WRITE );

	    while( bbk->write_count > 0 )
	    {
		/*
		 * Sleep until link ready, or something else happens
		 */
		if ( ABORT_IO( bbk, addr_error ) )
			bbk->write_abort |= ABORT_ERROR;

                while( ( (READREG(addr_osr) & LINK_READY) == 0 ) &&
                       (bbk->write_abort == 0) )
		{
		    /*
		     * Up priority, enable interrupts and then sleep
		     */
		    old_pri = spl5();

		    bbk->icr |= INT_SEND_READY;
		    check_interrupts_(1, 80, reg);
		    check_interrupts_(1, 81, reg);
		    enable_interrupt (addr_osr);
		    check_interrupts_(1, 82, reg);

		    /*
		     * Sleep, unless its now ready. We must check it
		     * after re-enabling interrupts, because we may
		     * not get an interrupt if it was ready when we
		     * enabled interrupts.
		     */
		    if ( (READREG(addr_osr) & LINK_READY) == 0 )
		    {
			if ( bbk->timeout > 0 )
			{
			    arg_tmo = (unit<<16)|B_WRITE;
			    timeout( bbktimeout, arg_tmo, bbk->timeout );
			}
			bbk->iomode |= BBK_WRITE_SLEEP;

			debug( "bbkstrategy () [B_WRITE] - bbk%d:  write sleep 2\n", unit );
			if ( sleep( (caddr_t)&(bbk->write_bp), SLEEP_PRI ) )
			{
				/* print out device register ... */
				printf ("(0x%x) isr = 0x%x\n", addr_isr, READREG (addr_isr));
				printf ("(0x%x) osr = 0x%x\n", addr_osr, READREG (addr_osr));
				printf ("(0x%x) idr = 0x%x\n", addr_idr, READREG (addr_idr));
				printf ("(0x%x) odr = 0x%x\n", addr_odr, READREG (addr_odr));
                                bbk->write_abort |= ABORT_SIGNAL;
			}
			debug( "bbkstrategy () [B_WRITE] - bbk%d:  write wakeup\n", unit );

			bbk->iomode &= ~BBK_WRITE_SLEEP;

			if ( bbk->timeout > 0 ) 
				untimeout( bbktimeout, arg_tmo );

			/*
			 * if timed out, but link ready, clear timeout flag
			 */
			if ( (bbk->write_abort & ABORT_TIMEOUT) &&
			     ( (READREG(addr_osr) & LINK_READY) != 0 ) )
			{
			    bbk->write_abort &= ~ABORT_TIMEOUT;
			}
		    }
		    /*
		     * Clear interrupts and lower priority
		     */
		    bbk->icr &= ~INT_SEND_READY;
		    disable_interrupt (addr_osr);
		    check_interrupts_(1, 9, reg);

                    if ( ABORT_IO( bbk, addr_error ) )
                                bbk->write_abort |= ABORT_ERROR;
 
		    splx( old_pri );
		}
		if ( bbk->write_abort != 0 ) break;
		/*
		 * Transfer the data
		 */
		bbkstart( bbk, reg, B_WRITE );
	    }
	    bbk->iomode &= ~BBK_WRITE;
	}
        /*
         * Finish off
         */
        bp->b_resid = bbk->write_count;
         
        if ( bbk->write_abort != 0 )
        {
            debug ( "bbkstrategy () [B_WRITE] - bbk%d:  write aborted (%x), resid = %d\n",
                    unit, bbk->write_abort, bbk->write_count );
 
            if ( bbk->write_abort & ABORT_SIGNAL )
            {
                bp->b_flags |= B_ERROR;
                bp->b_error = EINTR;
            }
            else if ( bbk->write_abort & ABORT_ERROR )
            {
                bp->b_flags |= B_ERROR;
                bp->b_error = EIO;
            }
        }
	break;
    }
    debug( "bbkstrategy () - bbk%d: i/o done, resid = %d\n", unit, bp->b_resid );
 
    iodone( bp );

    return;
}

/*############################################################################*/
static void bbkstart( bbk, reg, rw )

register struct bbk_device	*bbk;		/* device structure */
register struct b008_reg	*reg;		/* B008 registers */
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
	cptr = ((u_char *)(bbk->read_bp->b_un.b_addr)) + bbk->read_offset;
	/*
	 * Read data while data is to be read, and the link is ready
	 */
	debug ("bbkstart () - reading %d bytes\n", bbk->read_count);
	while( (bbk->read_count > 0) && (READREG( addr_isr ) & LINK_READY) )
	{
	    /*
	     * Read register and update counts
	     */
	    *cptr++ = READREG( addr_idr );
/*
	    debug ("bbkstart () - read 0x%x\n", *(cptr - 1));
*/
	    bbk->read_offset++;
	    bbk->read_count--;
	}
	debug ("bbkstart () - still need to read %d bytes\n", bbk->read_count);
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
	cptr = ((u_char *)(bbk->write_bp->b_un.b_addr)) + bbk->write_offset;
	/*
	 * Write data while data is to be written, and the link is ready
	 */
	debug ("bbkstart () - writing %d bytes\n", bbk->write_count);
	while( (bbk->write_count > 0) && (READREG( addr_osr ) & LINK_READY) )
	{
	    /*
	     * Write to register, then wait a bit for next write
	     */
	    WRITEREG( addr_odr, *cptr++ );
/*
	    debug ("bbkstart () - write 0x%x\n", *(cptr - 1));
*/
	    bbk->write_offset++;
	    bbk->write_count--;
	}
	debug ("bbkstart () - still need to write %d bytes\n", bbk->write_count);
	break;
    }
    return;
}

/*############################################################################*/
bbkioctl( dev, cmd, data, mode )

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
    register struct b008_reg	*reg;	/* register pointer */
    register struct bbk_device	*bbk;	/* device structure pointer */
    int				value;	/* data value */
    short		addr_isr;	/* address of isr */
    short		addr_osr;	/* address of osr */
    short		addr_reset_err;	/* address of reset/error */
    short		addr_analyse;	/* address of analyse */

/*============================================================================*/
    
    /*
     * Initialisation and general checking
     */
    unit = BBKUNIT( dev );
    if ( unit >= NBBK ) 
    {
	u.u_error = ENXIO;
	return;
    }

    debug( "ioctl () - bbk%d:  ioctl %x, status = %x\n", 
		unit, cmd, bbkdevice[unit].status );

    bbk = &bbkdevice[unit];

    if ( bbk->status & STS_DISABLED )
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
    addr_reset_err = (short)&(reg->reset_error);
    addr_analyse   = (short)&(reg->analyse);
    /*
     * Process command
     */
    switch( cmd )
    {
      case BBK_RESET:
	debug ("ioctl () - BBK_RESET (0x%lx)\n", cmd);
	/*
	 * Reset the device;  first check no i/o in progress	
	 */
	if ( bbk->iomode != 0 )
	{
	    u.u_error = EBUSY;
	    return;
	}

	WRITEREG( addr_analyse, 0 );
	DELAY(1000)
	WRITEREG( addr_reset_err, 1 );
	DELAY(1000)
	WRITEREG( addr_reset_err, 0 );
	/*
	 * Disable VME interrupts, enable C012 interrupts
	 */
	bbk->icr = 0;
/*
	WRITEREG( addr_isr, INT_C012 );
	WRITEREG( addr_osr, INT_C012 );
*/
	disable_interrupt (addr_isr);
	disable_interrupt (addr_osr);
	check_interrupts_(1, 2222, reg);

	break;

      case BBK_ANALYSE:
	debug ("ioctl () - BBK_ANALYSE (0x%lx)\n", cmd);
	/*
	 * Analyse the device; first check no i/o in progress	
	 */
	if ( bbk->iomode != 0 )
	{
	    u.u_error = EBUSY;
	    return;
	}

	WRITEREG( addr_analyse, 1 );
	DELAY(5000)		/* must be > 3 ms */
	WRITEREG( addr_reset_err, 1 );
	DELAY(1000)
	WRITEREG( addr_reset_err, 0 );
	DELAY(1000)
	WRITEREG( addr_analyse, 0 );
	/*
	 * Disable VME interrupts, enable C012 interrupts
	 */
	bbk->icr = 0;

	WRITEREG( addr_isr, INT_C012 );
	WRITEREG( addr_osr, INT_C012 );

	check_interrupts_(1, 11, reg);
	
	break;

      case BBK_ENABLE_ERRORS:
	debug ("ioctl () - BBK_ENABLE_ERRORS (0x%lx)\n", cmd);
	/*
	 * Set abort on error status flag, and enable error interrupts
	 */
	bbk->error_abort = TRUE;

	bbk->icr |= INT_ERROR;
	check_interrupts_(1, 12, reg);

	break;

      case BBK_DISABLE_ERRORS:
	debug ("ioctl () - BBK_DISABLE_ERRORS (0x%lx)\n", cmd);
	/*
	 * Clear abort on error flag, and disable error interrupts
	 */
	bbk->icr &= ~INT_ERROR;
	disable_interrupt (addr_isr);
	disable_interrupt (addr_osr);
	check_interrupts_(1, 13, reg);


	bbk->error_abort = FALSE;
	break;

      case BBK_TIMEOUT:
	debug ("ioctl () - BBK_TIMEOUT (0x%lx)\n", cmd);
        /*
         * Set timeout for i/o
         */
        bbk->timeout = (int)data * HZ / 10;
        debug( "ioctl () - bbk%d:  timeout set to %d ticks\n", unit, bbk->timeout );
        break;

      case BBK_SET_ERROR:
	debug ("ioctl () - BBK_SET_ERROR (0x%lx)\n", cmd);
	/*
	 * Set error flag set bit setting.  Inmos seem unclear about whether
	 * all boards are the same as far as setting the error bit, and
	 * different versions of the B008 manual say different things,
	 * so put in an ioctl to allow the user to define which way round
	 * it is, in case of problems.
	 */
	value = (int)data;
	if ( value < 0 || value > 1 ) 
	{
	    u.u_error = EINVAL;
	    return;
	}

	error_set = value;
	break;

      case BBK_ERROR:
	debug ("ioctl () - BBK_ERROR (0x%lx)\n", cmd);
	/*
	 * Check error register
	 */
	if ( ((int)READREG(addr_reset_err) & 1) == error_set )
	{
	    value = 1;
	}
	else
	{
	    value = 0;
	}
	copyout( (caddr_t)&value, data, sizeof(value) );

	break;

      case BBK_INPUT_PENDING:
	debug ("ioctl () - BBK_INPUT_PENDING (0x%lx)\n", cmd);
	/*
	 * Check input status register
	 */
	debug( "ioctl () - bbk%d:  ISR at %x contains %x\n", 
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

      case BBK_OUTPUT_READY:
	debug ("ioctl () - BBK_OUTPUT_READY (0x%lx)\n", cmd);
        /*
         * Check output status register
         */
        debug( "ioctl () - bbk%d:  OSR at %x contains %x\n",
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

      case BBK_DEBUG:
	debug ("ioctl () - BBK_DEBUG (0x%lx)\n", cmd);
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
	    debug( "ioctl () - bbk%d:  isr = %x, osr = %x, err = %x\n", 
		    unit, READREG(addr_isr), READREG(addr_osr), 
		    READREG(addr_reset_err) );
	    break;

	  case 3:
	    debug( "ioctl () - bbk%d:  icr = %x, iomode = %x\n", 
		    unit, bbk->icr, bbk->iomode );
	    break;
	  default:
	    break;
	}
	break;

#ifdef BBKDEBUG

        case BBK_READ_REG:
  	/*
	 * Read register
	 */
	copyin( data, (caddr_t)&value, sizeof(value) );
	value = (int)inb( (short)value );
	copyout( (caddr_t)&value, data, sizeof(value) );
	break;

      case BBK_WRITE_REG:
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

	debug( "ioctl () - bbk%d:  Bad ioctl command 0x%x\n", unit, cmd );
	u.u_error = ENOTTY;
	return;
    }

    check_interrupts_(1, 3333, reg);

    return;
}



/*############################################################################*/
bbkintr( irq )

int irq;        /* interrupt request channel */

{

/* Returned Value:	None
 *
 *----------------------------------------------------------------------------
 * Purpose: Handle interrupts from B008
 *
 * Method:  If something is ready wake up the top half. We could call start
 *	    down here, but for a big transfer that could mean that we stay at
 *   	    interrupt level for rather a long time, so we avoid that.
 *----------------------------------------------------------------------------*/

/* Local Functions Called */

/* Local Variables */

    register struct b008_reg	*reg;		/* B008 registers */
    register struct bbk_device	*bbk;		/* device structure ptr */
    short			addr_isr;	/* address of isr */
    short			addr_osr;	/* address of osr */
    short			addr_error;	/* address of error */
    int				unit;		/* unit number */

    int		set_read_interrupt, set_write_interrupt;

/*============================================================================*/

    /* 
     * Initialisation
     */
    debug( "bbkintr () -  irq = %d\n", irq );

    for( unit = 0;  unit < NBBK;  unit++ )
    {
	bbk = &bbkdevice[unit];
	if ( !bbk->initialised ) continue;
	if ( bbk->status & STS_DISABLED ) continue;

        /*
         * Check the interrupt channel
         */
        if ( bbk->int_chan == irq )
	{
	    debug( "bbkintr:  unit = %d\n", unit ); 
	    reg = board_address[unit];

	    addr_isr	= (short)&(reg->isr);
	    addr_osr	= (short)&(reg->osr);
	    addr_error	= (short)&(reg->reset_error);

	    set_read_interrupt = READREG (addr_isr) & 2;
	    set_write_interrupt = READREG (addr_osr) & 2;

	    debug ("set_read_interrupt = 0x%x, set_write_interrupt = 0x%x\n",
			set_read_interrupt, set_write_interrupt);

	    /*
	     * Kill all interrupts until we're done
	     */
	    check_interrupts_(1, 140, reg);
	    disable_interrupt (addr_isr);
	    check_interrupts_(1, 141, reg);
	    disable_interrupt (addr_osr);
	    check_interrupts_(1, 142, reg);

	    /*
	     * Process error interrupt
	     */
	    if ( (bbk->icr & INT_ERROR ) && 
		 ((READREG( addr_error ) & 1) == error_set ) )
	    {
		/*
		 * Disable error interrupts and wake any sleeping processes
		 */
		debug ( "bbkintr () -  error interrupt\n" );

		bbk->icr &= ~INT_ERROR;

		wakeup( (caddr_t)&(bbk->read_bp) );
		wakeup( (caddr_t)&(bbk->write_bp) );
	    }
	    /*
	     * Process input ready interrupt;  wake up sleeping process
	     */
	    if ( (bbk->icr & INT_RECEIVE_READY) &&
		 ( READREG( addr_isr ) & LINK_READY ) )
	    {
		debug( "bbkintr () -  read interrupt\n" );

		/*
		 * Clear interrupt flag
		 */
		bbk->icr &= ~INT_RECEIVE_READY;
		set_read_interrupt = 0;

		/*
		 * Wake up sleeping process
		 */
		if ( bbk->iomode & BBK_READ_SLEEP )
		{	
		    wakeup( (caddr_t)&(bbk->read_bp) );
		}
	    }
	    /*
	     * Process output ready interrupt;  wake up sleeping process
	     */
	    if ( (bbk->icr & INT_SEND_READY) &&
		 ( READREG( addr_osr ) & LINK_READY ) )
	    {
		debug( "bbkintr () -  write interrupt\n" );

		/*
		 * Link ready - clear interrupt flag
		 */
		bbk->icr &= ~INT_SEND_READY;
		set_write_interrupt = 0;

		/*
		 * Wake up process
		 */
		if ( bbk->iomode & BBK_WRITE_SLEEP )
		{
		    wakeup( (caddr_t)&(bbk->write_bp) );
		}
	    }
	    /*
	     * Re-enable interrupts
	     */
	    check_interrupts_(1, 150, reg);
	    if (set_read_interrupt)
	    {
		debug ("didn't do read - re-enabling interrupt\n");
		enable_interrupt (addr_isr);
	    }
	    if (set_write_interrupt)
	    {
		debug ("didn't do write - re-enabling interrupt\n");
		enable_interrupt (addr_osr);
	    }
	    check_interrupts_(1, 151, reg);

	    break;
	}
    }
    return;
}

/*############################################################################*/static void bbktimeout( arg )

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
        debug( "bbktimeout () - bbk%d: read timeout\n", unit );
        bbkdevice[unit].read_abort |= ABORT_TIMEOUT;
        wakeup( (caddr_t)&(bbkdevice[unit].read_bp) );
    }
    else
    {
        debug( "bbktimeout () - bbk%d: write timeout\n", unit );
        bbkdevice[unit].write_abort |= ABORT_TIMEOUT;
        wakeup( (caddr_t)&(bbkdevice[unit].write_bp) );
    }
 
    return;
}

/*############################################################################*/
static int bbk_delay( n, m )

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
