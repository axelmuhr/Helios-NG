
/*#############################################################################
 *
 *	Copyright (C) 1989,1990 K-par Systems  Ltd.  All rights reserved
 *
 * Program/Library:	imb Interactive 386/ix B008 driver imb_driver.c
 *
 * Purpose: 		Driver source code
 *
 * Author:		Chris Farey 27-Apr-1989, 15-Oct-1990
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
#include <sys/dma.h>

#include "imbdefs.h"

/* Global Constants */
/* ================ */

#define BANNER "\nK-par \"imb\" device driver for IMS B008,  version 1.1\n\
Copyright (C) 1989, 1990 K-par Systems Ltd.  All rights reserved.\n"

#define TRUE	1
#define FALSE	0

/*
 * Define number of devices here as 3, corresponding to 3 units at 3 possible
 * base addresses.
 */
#define	NIMB	3	

/*
 * Define sleep priority as an interruptable by sugnals, with
 * signals caught.
 */
#define SLEEP_PRI	(PZERO+1|PCATCH)

/* Maximum transfer size for physio */

#define MAX_BLOCKSIZE		4096
/*
 * Define a minimum size for DMA transfers. For small transfers
 * it is more efficient to transfer the data directly, avoiding
 * the overhead of setting up DMA and a context switch.
 */
#define	MIN_DMA_XFER		80	/* min. xfer size using DMA */

/* Device status values */

#define STS_AVAILABLE		0
#define STS_OPEN		0x0001
#define STS_DISABLED		0x0002

/* I/o modes */

#define	IMB_READ	0x0001
#define	IMB_WRITE	0x0002
#define IMB_READ_DMA	0x0004
#define IMB_WRITE_DMA	0x0008
#define IMB_READ_SLEEP	0x0010
#define IMB_WRITE_SLEEP	0x0020
#define IMB_DMA_SLEEP	0x0040

/* Aborted i/o flags */
 
#define         ABORT_SIGNAL    0x1
#define         ABORT_TIMEOUT   0x2
#define         ABORT_ERROR     0x4
 

/* Global Macros */
/* ============= */

/*
 * We encode in the minor device number the following:
 *
 *	bits 0,1	DMA channel, 0 = none
 *				     1 = 0
 *				     2 = 1
 *				     3 = 3
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
#define IMBUNIT(dev)	(minor(dev)>>3)
#define INT_CHAN(dev)	((minor(dev)&4)?5:3)
#define DMA_CHAN(dev)	(dma_channels[(minor(dev)&3)])

/*
 * Macros to access the device registers in i/o space
 */
#define	READREG(x)	(inb((short)x))
#define WRITEREG(x,y)	(outb((short)x,(u_char)y))
/*
 * Macro to determine whether to abort i/o because transputer error
 * flag is set, and user asked to abort if this happened.
 */
#define ABORT_IO(imb,addr_error) \
	(imb->error_abort&&(((int)READREG(addr_error)&1)==error_set))
/*
 * Macro to print debug messages if debug flag set
 */
#define debug		if(debug_flag)printf

/* 
 * Macro to delay for a specified number of microseconds
 */
#define DELAY(n)	(void)imb_delay(n,0);


/* External Routines */
/* ================= */

extern void	outb();
extern u_char	inb();
extern long 	dma_resid();


/* Static functions */
/* ================ */

static void	imbstrategy();
static void	imbstart();
static void	imbtimeout();
static int 	imbinit();

/* Global Variables */
/* ================ */

/* Possible dma channels */

static int	dma_channels[4] = {-1, 0, 1, 3};

/* Possible board addresses */

static struct b008_reg *board_address[3] = 
{
    (struct b008_reg *)0x150, 
    (struct b008_reg *)0x200, 
    (struct b008_reg *)0x300
};

/* Flags */

static int	debug_flag = 0;	/* debug flag */
static int	error_set = 0;	/* error flag set state */

/* static buffer headers for physio */

static struct buf	imb_readbufs[NIMB];	
static struct buf	imb_writebufs[NIMB];


/* Device status information */

static struct imb_device
{
    int		status;		/* device status */
    int		initialised;	/* device initialised */
    int		iomode;		/* current i/o mode */
    int		read_abort;	/* read abort flags */
    int		write_abort;	/* write abort flags */
    int		error_abort;	/* flag to abort i/o on error */
    int		timeout;	/* i/o timeout */
    int		int_chan;	/* interrupt channel */
    int		dma_chan;	/* dma channel */
    int		dma;		/* dma mode */
    int		uid;		/* current user id */
    char	icr;		/* copy of icr */
    struct buf	*read_bp;	/* current read buf */
    struct buf	*write_bp;	/* current write buf */
    int		read_count;	/* bytes to xfer in read */
    int		write_count;	/* bytes to xfer in write */
    int		read_offset;	/* current offset for read */
    int		write_offset;	/* current offset for write */
} imbdevice[NIMB];


/*############################################################################*/
imbinit( dev )

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
    register struct imb_device	*imb;		/* device structure */
    int				unit;		/* unit number */
    unsigned int		channel_select;	/* channel select mask */
    short			addr_isr;	/* address of isr */
    short			addr_osr;	/* address of osr */
    short			addr_analyse;	/* address of analyse */
    short			addr_reset;	/* address of reset */
    short			addr_icr;	/* address of icr */
    short			addr_chansel;	/* address of channel select */


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
	for( unit = 0;  unit < NIMB;  unit++ )
	{
	    imbdevice[unit].status = STS_DISABLED;
	}

	first_time = FALSE;
    }

    /*
     * Initialisation
     */
    unit = IMBUNIT(dev);
    if ( (unit < 0) || (unit > NIMB) )
    {
	return ENXIO;
    }

    reg = board_address[unit];
    imb = &imbdevice[unit];

    debug( "imb%d: init - addr = %x, dma channel = %d, irq = %d\n", 
		unit, reg, DMA_CHAN(dev), INT_CHAN(dev) );

    imb->int_chan = INT_CHAN(dev);
    imb->dma_chan = DMA_CHAN(dev);

    addr_isr 	 = (short)&(reg->isr);
    addr_osr	 = (short)&(reg->osr);
    addr_reset	 = (short)&(reg->reset_error);
    addr_analyse = (short)&(reg->analyse);
    addr_icr 	 = (short)&(reg->icr);
    addr_chansel = (short)&(reg->chan_select);

    debug("imb%d:  init, reg = %x\n", unit, reg );

    /*
     * Initialise device structure
     */
    imb->status = STS_AVAILABLE;
    imb->iomode = 0;
    imb->error_abort = FALSE;
    imb->uid = -1;
    imb->dma = IMB_DMA_OFF;
    /*
     * Set up the dma and interrupt request channels (new b008 only)
     * Also set the address of the dma byte counter.
     */
    channel_select = 0;

    switch( DMA_CHAN(dev) )
    {
      case 0:
	channel_select |= DMA_0;
	break;
      case 1:
	channel_select |= DMA_1;
	break;
      case 3:
	channel_select |= DMA_3;
	break;
      default:
	channel_select |= DMA_OFF;
	break;
    }

    switch( INT_CHAN(dev) )
    {
      case 3:
	channel_select |= IRQ_3;
	break;
      case 5:
	channel_select |= IRQ_5;
	break;
      case 11:
	channel_select |= IRQ_11;
	break;
      case 15:
	channel_select |= IRQ_15;
	break;
    }
    
    WRITEREG( addr_chansel, channel_select ); 
    
    /*
     * Reset the device
     */
    WRITEREG( addr_icr, 0 );
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
	printf( "imb%d: C012 ISR check failed (isr = %x);  disabling device.\n",
		unit, READREG( addr_isr ) );
	imb->status = STS_DISABLED;
	return EIO;
    }
    WRITEREG( addr_osr, INT_C012 );
    DELAY( 1000 );
    if ( (READREG( addr_osr ) & LINK_READY) != LINK_READY )
    {
	printf( "imb%d: C012 OSR check failed( osr = %x);  disabling device.\n",
		unit, READREG( addr_osr ) );
	imb->status = STS_DISABLED;
	return EIO;
    }
    /*
     * Disable interrupts
     */
    WRITEREG( addr_icr, 0 );
    imb->icr = 0;

    return( 0 );
}


/*############################################################################*/
imbopen( dev, flags, otyp )

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

    unit = IMBUNIT( dev );

    /*
     * First time round initialise the device
     */
    if ( !imbdevice[unit].initialised )
    {
	rval = imbinit( dev );
	if ( rval != 0 )
	{
	    u.u_error = rval;
	    return;
	}
	imbdevice[unit].initialised = TRUE;
    }

    /*
     * Check device is usable
     */
    if ( unit >= NIMB || (imbdevice[unit].status & STS_DISABLED) ) 
    {
	u.u_error = ENXIO;
	return;
    }


    /*
     * If already open check for match of uid
     */
    if ( imbdevice[unit].status & STS_OPEN ) 
    {
	if ( u.u_uid == imbdevice[unit].uid ) 
	{
	    debug( "imb%d: Open (2) by process %d\n", unit, u.u_procp->p_pid );
	    return;
	}
	else
	{
	    u.u_error = EBUSY;
	    return;
	}
    }

    debug( "imb%d: Open (1) by process %d\n", unit, u.u_procp->p_pid );
    /*
     * Set status to open, and save user details
     */
    imbdevice[unit].status |= STS_OPEN;
    imbdevice[unit].uid  = u.u_uid;
    /*
     * Initially set DMA off and abort on errors clear
     */
    imbdevice[unit].dma = IMB_DMA_OFF;
    imbdevice[unit].error_abort = FALSE;
    imbdevice[unit].timeout = 0;

    return;
}


/*############################################################################*/
imbclose( dev, flags, otyp )

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
    short		addr_icr;	/* address of icr */

/*============================================================================*/

    /*
     * Extract unit number from device, and check its valid
     */
    unit = IMBUNIT( dev );

    if ( unit < NIMB )
    {
	debug( "imb%d: close\n", unit );
	/*
	 * Clear interrupt control register
	 */
	reg = (struct b008_reg *)board_address[unit];

	addr_icr = (short)&(reg->icr);

	WRITEREG( addr_icr, 0 );
	/*
	 * Clear device details
	 */
	imbdevice[unit].iomode	= 0;
	imbdevice[unit].icr 	= 0;
	imbdevice[unit].status &= ~STS_OPEN;
	imbdevice[unit].uid	= -1;
    }
    return;
}

/*############################################################################*/
imbread( dev )

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
    register struct imb_device	*imb;		/* device structure pointer */
    int 			unit;		/* unit number */
    short			addr_error;	/* address of error reg */

/*============================================================================*/

    /*
     * Extract unit number and check it
     */
    unit = IMBUNIT( dev );

    debug( "imb%d: read\n", unit );

    if ( unit >= NIMB ) 
    {
	u.u_error = ENXIO;
	return;
    }

    /*
     * Initialisation
     */
    imb = &imbdevice[unit];
    reg = board_address[unit];

    addr_error = (short)&(reg->reset_error);
    /*
     * Let physio do the work, unless we need to abort because of an error
     */
    if ( !ABORT_IO( imb, addr_error ) )
    {
	physio( imbstrategy,
		&imb_readbufs[unit],
		dev,
		B_READ );
    }

    /*
     * Output message if i/o aborted
     */
    if ( ABORT_IO( imb, addr_error ) )
    {
	debug( "imb%d:  Error on transputer - read aborted\n", unit );
	u.u_error = EIO;
	return;
    }
    debug( "imb%d:  read done\n", unit );

    return;
}
/*############################################################################*/
imbwrite( dev )


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
    register struct imb_device	*imb;		/* device structure ptr */
    int 			unit;		/* unit number */
    short			addr_error;	/* address of error register */

/*============================================================================*/

    /*
     * Extract and check unit number
     */
    unit = IMBUNIT( dev );

    debug( "imb%d: write\n", unit );

    if ( unit >= NIMB ) 
    {
	u.u_error = ENXIO;
	return;
    }

    /*
     * Initialisation
     */
    imb = &imbdevice[unit];
    reg = board_address[unit];

    addr_error = (short)&(reg->reset_error);

    /*
     * Call physio to do the work, unless abort is set
     */
    if ( !ABORT_IO( imb, addr_error ) )
    {
	physio( imbstrategy,
		&imb_writebufs[unit],
		dev,
		B_WRITE );
    }
    /*
     * Output message if write aborted
     */
    if ( ABORT_IO( imb, addr_error ) )
    {
	debug( "imb%d:  Error on transputer - write aborted\n", unit );

	u.u_error = EIO;
	return;
    }

    debug( "imb%d:  write done\n", unit );

    return;
}

/*############################################################################*/
static void imbstrategy( bp )

register struct buf	*bp;
{

/*----------------------------------------------------------------------------
 * Purpose: High level i/o routine
 *
 * Method:
 *----------------------------------------------------------------------------*/

/* Local Variables */

    register struct b008_reg	*reg;		/* B008 registers */
    register struct imb_device	*imb;		/* device structure */
    register int		unit;		/* unit number */
    int				old_pri;	/* saved priority */
    unsigned int		arg_tmo;	/* timeout argument */
    short			addr_isr;	/* address of isr */
    short			addr_osr;	/* address of osr */
    short			addr_error;	/* address of error */
    short			addr_dma;	/* address of dma */
    short			addr_icr;	/* address of icr */

/*============================================================================*/

    /*
     * Initialisation
     */
    unit = IMBUNIT( bp->b_dev );
    imb = &imbdevice[unit];
    reg = board_address[unit];

    addr_isr	= (short)&(reg->isr);
    addr_osr	= (short)&(reg->osr);
    addr_error	= (short)&(reg->reset_error);
    addr_dma	= (short)&(reg->dma_request);
    addr_icr	= (short)&(reg->icr);

    /*
     * Either do a read or a write
     */
    switch( bp->b_flags & B_READ )
    {
      case B_READ:

	debug( "imb%d:  read %d bytes\n", unit, bp->b_bcount );

	imb->read_count = bp->b_bcount;
	/*
	 * Sleep until link ready, or something else happens
	 */
	imb->read_abort = ABORT_IO( imb, addr_error ) ? ABORT_ERROR : 0;

        while( ( (READREG(addr_isr) & LINK_READY) == 0 ) &&
               (imb->read_abort == 0) )
	{
	    /*
	     * Up priority, enable interrupts and then sleep
	     */
	    old_pri = spl5();

	    imb->icr |= INT_RECEIVE_READY;
	    WRITEREG( addr_icr, imb->icr );
	    /*
	     * Sleep, unless its now ready. We must check it
	     * after re-enabling interrupts, because we may
	     * not get an interrupt if it was ready when we
	     * enabled interrupts.
	     */
	    if ( (READREG(addr_isr) & LINK_READY) == 0 )
	    {
		if ( imb->timeout > 0 )
		{
		    arg_tmo = (unit<<16) | B_READ;
		    timeout( imbtimeout, arg_tmo, imb->timeout );
		}
		imb->iomode |= IMB_READ_SLEEP;

		debug( "imb%d:  read sleep 1\n", unit );
		if ( sleep( (caddr_t)&(imb->read_bp), SLEEP_PRI ) )
				    imb->read_abort |= ABORT_SIGNAL;
		debug( "imb%d:  read wakeup\n", unit );

		imb->iomode &= ~IMB_READ_SLEEP;

		if ( imb->timeout > 0 ) untimeout( imbtimeout, arg_tmo );

		/*
		 * if timed out, but link ready, clear timeout flag
		 */
		if ( (imb->read_abort & ABORT_TIMEOUT) &&
		     ( (READREG(addr_isr) & LINK_READY) != 0 ) )
		{
		    imb->read_abort &= ~ABORT_TIMEOUT;
		}
	    }

	    /*
	     * Clear interrupts and lower priority
	     */
	    imb->icr &= ~INT_RECEIVE_READY;
	    WRITEREG( addr_icr, imb->icr );

	    splx( old_pri );
            /*
             * Re-check error flag
             */
            if ( ABORT_IO( imb, addr_error ) ) imb->read_abort |= ABORT_ERROR;
	}
	/* 
	 * Decide whether to do a DMA transfer.  Do it if DMA has
	 * been requested, the count is big enough and there is not 
	 * already any DMA going on.
	 */
	if ( imb->read_abort != 0 )
	{
	    ;	/* continue */
	}
	else if ( (imb->dma & IMB_DMA_READ ) &&
	     ( bp->b_bcount > MIN_DMA_XFER ) &&
	     ! (imb->iomode & IMB_WRITE_DMA ) &&
	     ( dma_alloc( imb->dma_chan, DMA_NBLOCK ) == 0 ) )
	{
	    /*
	     * Set up DMA
	     */
	    imb->iomode |= IMB_READ_DMA;

	    /*
	     * Initialise the controller and enable dma
	     */
	    dma_param(	imb->dma_chan, 
			DMA_Rdmode, 
			bp->b_un.b_addr, 
			bp->b_bcount-1 );

	    dma_enable( imb->dma_chan );
	    /*
	     * Enable interrupts, start it off and then sleep to completion
	     */
	    old_pri = spl5();
	    imb->icr |= INT_DMA;
	    WRITEREG( addr_icr, imb->icr );

	    WRITEREG( addr_dma, 1 );

	    imb->iomode |= IMB_DMA_SLEEP;
	    debug( "imb%d:  dma read sleep\n", unit );
	    if ( sleep( (caddr_t)&(imb->dma), SLEEP_PRI ) )
	    {
		/*
		 * Woken by signal;  set flag and kill interrupts
		 */
		imb->read_abort |= ABORT_SIGNAL;
		imb->icr &= ~INT_DMA;
		WRITEREG( addr_icr, imb->icr );
	    }
	    debug( "imb%d:  dma read wakeup\n", unit );
	    imb->iomode &= ~IMB_DMA_SLEEP;
	    splx( old_pri );
	    /*
	     * Tidy up after DMA
	     */
	    dma_relse( imb->dma_chan );

	    imb->iomode &= ~IMB_READ_DMA;
	    if( imb->read_abort == 0 ) imb->read_count = 0;
	}
	else
	{
	    /*
	     * Interrupt driven i/o;  set up pointers for first transfer.
	     */
	    debug( "imb%d:  link adaptor i/o - %d\n", unit, imb->read_count );

	    imb->read_bp    = bp;
	    imb->read_offset= 0;

	    imb->iomode |= IMB_READ;
	    /*
	     * Start the transfer, then if there's anything left
	     * loop round sleeping then transferring.
	     */
	    imbstart( imb, reg, B_READ );

	    while( imb->read_count > 0 )
	    {
		/*
		 * Sleep until link ready, or something else happens
		 */
		if ( ABORT_IO( imb, addr_error ) )
                                imb->read_abort |= ABORT_ERROR;
 
                while( ( (READREG(addr_isr) & LINK_READY) == 0 ) &&
                       ( imb->read_abort == 0 ) )
		{
		    /*
		     * Up priority, enable interrupts and then sleep
		     */
		    old_pri = spl5();

		    imb->icr |= INT_RECEIVE_READY;
		    WRITEREG( addr_icr, imb->icr );
		    /*
		     * Sleep, unless its now ready. We must check it
		     * after re-enabling interrupts, because we may
		     * not get an interrupt if it was ready when we
		     * enabled interrupts.
		     */
		    if ( (READREG(addr_isr) & LINK_READY) == 0 )
		    {
			if ( imb->timeout > 0 )
			{
			    arg_tmo = (unit<<16) | B_READ;
			    timeout( imbtimeout, arg_tmo, imb->timeout );
			}
			imb->iomode |= IMB_READ_SLEEP;

			debug( "imb%d:  read sleep 2\n", unit );
			if ( sleep( (caddr_t)&(imb->read_bp), SLEEP_PRI ) )
					    imb->read_abort |= ABORT_SIGNAL;
			debug( "imb%d:  read wakeup\n", unit );

			imb->iomode &= ~IMB_READ_SLEEP;
			if ( imb->timeout > 0 ) 
				untimeout( imbtimeout, arg_tmo );

			/*
			 * if timed out, but link ready, clear timeout flag
			 */
			if ( (imb->read_abort & ABORT_TIMEOUT) &&
			     ( (READREG(addr_isr) & LINK_READY) != 0 ) )
			{
			    imb->read_abort &= ~ABORT_TIMEOUT;
			}
		    }
		    /*
		     * Clear interrupts and lower priority
		     */
		    imb->icr &= ~INT_RECEIVE_READY;
		    WRITEREG( addr_icr, imb->icr );

		    splx( old_pri );
                    /*
                     * Check for abort on error
                     */
                    if ( ABORT_IO( imb, addr_error ) )
                                imb->read_abort |= ABORT_ERROR;
		}
		if ( imb->read_abort != 0 ) break;
		/*
		 * Transfer the data
		 */
		imbstart( imb, reg, B_READ );
	    }
	    imb->iomode &= ~IMB_READ;
	}
        /* 
         * Finish off
         */
        bp->b_resid = imb->read_count;
        if ( imb->read_abort != 0 )
        {  
            debug( "imb%d:  read aborted (%x), resid = %d\n",
                    unit, imb->read_abort, imb->read_count );
 
            if ( imb->read_abort & ABORT_SIGNAL )
            {
                bp->b_flags |= B_ERROR;
                bp->b_error = EINTR;
            }
            else if ( imb->read_abort & ABORT_ERROR )
            {
                bp->b_flags |= B_ERROR;
                bp->b_error = EIO;
            }
	}
	break;

      case B_WRITE:

	debug( "imb%d:  write %d bytes\n", unit, bp->b_bcount );
 
        imb->write_count = bp->b_bcount;
	/*
	 * Sleep until link ready, or something else happens
	 */
        imb->write_abort = ABORT_IO( imb, addr_error ) ? ABORT_ERROR : 0;
          
        while( ( (READREG(addr_osr) & LINK_READY) == 0 ) &&
               (imb->write_abort == 0) )
	{
	    /*
	     * Up priority, enable interrupts and then sleep
	     */
	    old_pri = spl5();

	    imb->icr |= INT_SEND_READY;
	    WRITEREG( addr_icr, imb->icr );
	    /*
	     * Sleep, unless its now ready. We must check it
	     * after re-enabling interrupts, because we may
	     * not get an interrupt if it was ready when we
	     * enabled interrupts.
	     */
	    if ( (READREG(addr_osr) & LINK_READY) == 0 )
	    {
		if ( imb->timeout > 0 )
		{
		    arg_tmo = (unit<<16)|B_WRITE;
		    timeout( imbtimeout, arg_tmo, imb->timeout );
		}
		imb->iomode |= IMB_WRITE_SLEEP;

		debug( "imb%d:  write sleep 1\n", unit );
	       if ( sleep( (caddr_t)&(imb->write_bp), SLEEP_PRI ) )
                                    imb->write_abort |= ABORT_SIGNAL;
		debug( "imb%d:  write wakeup\n", unit );

		imb->iomode &= ~IMB_WRITE_SLEEP;
		if ( imb->timeout > 0 ) untimeout( imbtimeout, arg_tmo );

		/*
		 * if timed out, but link ready, clear timeout flag
		 */
		if ( (imb->write_abort & ABORT_TIMEOUT) &&
		     ( (READREG(addr_osr) & LINK_READY) != 0 ) )
		{
		    imb->write_abort &= ~ABORT_TIMEOUT;
		}
	    }
	    /*
	     * Clear interrupts and lower priority
	     */
	    imb->icr &= ~INT_SEND_READY;
	    WRITEREG( addr_icr, imb->icr );

	    splx( old_pri );
            /*
             * Re-check error flag
             */
            if ( ABORT_IO( imb, addr_error ) ) imb->write_abort |= ABORT_ERROR;
	}
	/* 
	 * Decide whether to do a DMA transfer.  Do it if DMA has
	 * been requested, the count is big enough and there is not 
	 * already any DMA going on.
	 */
        if ( imb->write_abort != 0 )
        { 
            ;           /* no action */
        }
        else if ( (imb->dma & IMB_DMA_WRITE ) &&
	     ( bp->b_bcount > MIN_DMA_XFER ) &&
	     !(imb->iomode & IMB_READ_DMA) &&
	     ( dma_alloc( imb->dma_chan, DMA_NBLOCK ) == 0 ) )
	{
	    /*
	     * Set up DMA
	     */
	    imb->iomode |= IMB_WRITE_DMA;

	    /*
	     * Initialise the controller and enable dma
	     */
	    dma_param(	imb->dma_chan, 
			DMA_Wrmode, 
			bp->b_un.b_addr, 
			bp->b_bcount-1 );

	    dma_enable( imb->dma_chan );
	    /*
	     * Enable interrupts, start it off and then sleep to completion
	     */
	    old_pri = spl5();
	    imb->icr |= INT_DMA;
	    WRITEREG( addr_icr, imb->icr );

	    WRITEREG( addr_dma, 0 );

	    imb->iomode |= IMB_DMA_SLEEP;

	    debug( "imb%d:  dma write sleep\n", unit );
	    if ( sleep( (caddr_t)&(imb->dma), SLEEP_PRI ) )
	    {
		/*
		 * Woken by signal;  set flag and kill interrupts
		 */
		imb->write_abort  = ABORT_SIGNAL;
		imb->icr &= ~INT_DMA;
		WRITEREG( addr_icr, imb->icr );
	    }
	    debug( "imb%d:  dma write wakeup\n", unit );
	    imb->iomode &= ~IMB_DMA_SLEEP;
	    splx( old_pri );

	    /*
	     * Tidy up after DMA
	     */
	    dma_relse( imb->dma_chan );

	    imb->iomode &= ~IMB_WRITE_DMA;
	    if( imb->write_abort == 0 ) imb->write_count = 0;
	}
	else
	{
	    /*
	     * Interrupt driven i/o;  set up pointers for first transfer.
	     */
	    imb->write_bp    = bp;
	    imb->write_offset= 0;

	    debug( "imb%d:  link adaptor i/o\n", unit );

	    imb->iomode |= IMB_WRITE;
	    /*
	     * Start the transfer, then if there's anything left loop
	     * round sleeping and transferring until completion.
	     */
	    imbstart( imb, reg, B_WRITE );

	    while( imb->write_count > 0 )
	    {
		/*
		 * Sleep until link ready, or something else happens
		 */
		if ( ABORT_IO( imb, addr_error ) )
			imb->write_abort |= ABORT_ERROR;

                while( ( (READREG(addr_osr) & LINK_READY) == 0 ) &&
                       (imb->write_abort == 0) )
		{
		    /*
		     * Up priority, enable interrupts and then sleep
		     */
		    old_pri = spl5();

		    imb->icr |= INT_SEND_READY;
		    WRITEREG( addr_icr, imb->icr );
		    /*
		     * Sleep, unless its now ready. We must check it
		     * after re-enabling interrupts, because we may
		     * not get an interrupt if it was ready when we
		     * enabled interrupts.
		     */
		    if ( (READREG(addr_osr) & LINK_READY) == 0 )
		    {
			if ( imb->timeout > 0 )
			{
			    arg_tmo = (unit<<16)|B_WRITE;
			    timeout( imbtimeout, arg_tmo, imb->timeout );
			}
			imb->iomode |= IMB_WRITE_SLEEP;

			debug( "imb%d:  write sleep 2\n", unit );
			if ( sleep( (caddr_t)&(imb->write_bp), SLEEP_PRI ) )
                                        imb->write_abort |= ABORT_SIGNAL;
			debug( "imb%d:  write wakeup\n", unit );

			imb->iomode &= ~IMB_WRITE_SLEEP;

			if ( imb->timeout > 0 ) 
				untimeout( imbtimeout, arg_tmo );

			/*
			 * if timed out, but link ready, clear timeout flag
			 */
			if ( (imb->write_abort & ABORT_TIMEOUT) &&
			     ( (READREG(addr_osr) & LINK_READY) != 0 ) )
			{
			    imb->write_abort &= ~ABORT_TIMEOUT;
			}
		    }
		    /*
		     * Clear interrupts and lower priority
		     */
		    imb->icr &= ~INT_SEND_READY;
		    WRITEREG( addr_icr, imb->icr );

                    if ( ABORT_IO( imb, addr_error ) )
                                imb->write_abort |= ABORT_ERROR;
 
		    splx( old_pri );
		}
		if ( imb->write_abort != 0 ) break;
		/*
		 * Transfer the data
		 */
		imbstart( imb, reg, B_WRITE );
	    }
	    imb->iomode &= ~IMB_WRITE;
	}
        /*
         * Finish off
         */
        bp->b_resid = imb->write_count;
         
        if ( imb->write_abort != 0 )
        {
            debug( "imb%d:  write aborted (%x), resid = %d\n",
                    unit, imb->write_abort, imb->write_count );
 
            if ( imb->write_abort & ABORT_SIGNAL )
            {
                bp->b_flags |= B_ERROR;
                bp->b_error = EINTR;
            }
            else if ( imb->write_abort & ABORT_ERROR )
            {
                bp->b_flags |= B_ERROR;
                bp->b_error = EIO;
            }
        }
	break;
    }
    debug( "imb%d: i/o done, resid = %d\n", unit, bp->b_resid );
 
    iodone( bp );

    return;
}

/*############################################################################*/
static void imbstart( imb, reg, rw )

register struct imb_device	*imb;		/* device structure */
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
	cptr = ((u_char *)(imb->read_bp->b_un.b_addr)) + imb->read_offset;
	/*
	 * Read data while data is to be read, and the link is ready
	 */
	while( (imb->read_count > 0) && (READREG( addr_isr ) & LINK_READY) )
	{
	    /*
	     * Read register and update counts
	     */
	    *cptr++ = READREG( addr_idr );
	    imb->read_offset++;
	    imb->read_count--;
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
	cptr = ((u_char *)(imb->write_bp->b_un.b_addr)) + imb->write_offset;
	/*
	 * Write data while data is to be written, and the link is ready
	 */
	while( (imb->write_count > 0) && (READREG( addr_osr ) & LINK_READY) )
	{
	    /*
	     * Write to register, then wait a bit for next write
	     */
	    WRITEREG( addr_odr, *cptr++ );
	    imb->write_offset++;
	    imb->write_count--;
	}
	break;
    }
    return;
}

/*############################################################################*/
imbioctl( dev, cmd, data, mode )

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
    register struct imb_device	*imb;	/* device structure pointer */
    int				value;	/* data value */
    short		addr_isr;	/* address of isr */
    short		addr_osr;	/* address of osr */
    short		addr_reset_err;	/* address of reset/error */
    short		addr_analyse;	/* address of analyse */
    short		addr_icr;	/* address of icr */

/*============================================================================*/
    
    /*
     * Initialisation and general checking
     */
    unit = IMBUNIT( dev );
    if ( unit >= NIMB ) 
    {
	u.u_error = ENXIO;
	return;
    }

    debug( "imb%d:  ioctl %x, status = %x\n", 
		unit, cmd, imbdevice[unit].status );

    imb = &imbdevice[unit];

    if ( imb->status & STS_DISABLED )
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
    addr_icr	   = (short)&(reg->icr);
    /*
     * Process command
     */
    switch( cmd )
    {
      case IMB_RESET:
	/*
	 * Reset the device;  first check no i/o in progress	
	 */
	if ( imb->iomode != 0 )
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
	WRITEREG( addr_icr, 0 );
	imb->icr = 0;

	WRITEREG( addr_isr, INT_C012 );
	WRITEREG( addr_osr, INT_C012 );

	break;

      case IMB_ANALYSE:
	/*
	 * Analyse the device; first check no i/o in progress	
	 */
	if ( imb->iomode != 0 )
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
	WRITEREG( addr_icr, 0 );
	imb->icr = 0;

	WRITEREG( addr_isr, INT_C012 );
	WRITEREG( addr_osr, INT_C012 );
	
	break;

      case IMB_ENABLE_ERRORS:
	/*
	 * Set abort on error status flag, and enable error interrupts
	 */
	imb->error_abort = TRUE;

	imb->icr |= INT_ERROR;
	WRITEREG( addr_icr, imb->icr );
	break;

      case IMB_DISABLE_ERRORS:
	/*
	 * Clear abort on error flag, and disable error interrupts
	 */
	imb->icr &= ~INT_ERROR;
	WRITEREG( addr_icr, imb->icr );

	imb->error_abort = FALSE;
	break;

      case IMB_TIMEOUT:
        /*
         * Set timeout for i/o
         */
        imb->timeout = (int)data * HZ / 10;
        debug( "imb%d:  timeout set to %d ticks\n", unit, imb->timeout );
        break;

      case IMB_SET_ERROR:
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

      case IMB_ERROR:
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

      case IMB_INPUT_PENDING:
	/*
	 * Check input status register
	 */
	debug( "imb%d:  ISR at %x contains %x\n", 
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

      case IMB_OUTPUT_READY:
        /*
         * Check output status register
         */
        debug( "imb%d:  OSR at %x contains %x\n",
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
 
      case IMB_DMA:
	/*
	 * Set DMA flag, after checking for invalid data
	 */
	if ( (int)data & ~IMB_DMA_READWRITE ) 
	{
	   u.u_error = EINVAL;
	   return;
	}

	imb->dma = (int)data;

	break;

      case IMB_DEBUG:
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
	    printf( "imb%d:  isr = %x, osr = %x, err = %x\n", 
		    unit, READREG(addr_isr), READREG(addr_osr), 
		    READREG(addr_reset_err) );
	    break;

	  case 3:
	    printf( "imb%d:  icr = %x, iomode = %x\n", 
		    unit, imb->icr, imb->iomode );
	    break;
	  default:
	    break;
	}
	break;

#ifdef IMBDEBUG

        case IMB_READ_REG:
  	/*
	 * Read register
	 */
	copyin( data, (caddr_t)&value, sizeof(value) );
	value = (int)inb( (short)value );
	copyout( (caddr_t)&value, data, sizeof(value) );
	break;

      case IMB_WRITE_REG:
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

	debug( "imb%d:  Bad ioctl command 0x%x\n", unit, cmd );
	u.u_error = ENOTTY;
	return;
    }

    return;
}



/*############################################################################*/
imbintr( irq )

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
    register struct imb_device	*imb;		/* device structure ptr */
    short			addr_isr;	/* address of isr */
    short			addr_osr;	/* address of osr */
    short			addr_icr;	/* address of icr */
    short			addr_error;	/* address of error */
    int				unit;		/* unit number */

/*============================================================================*/

    /* 
     * Initialisation
     */
    debug( "imbintr irq = %d\n", irq );

    for( unit = 0;  unit < NIMB;  unit++ )
    {
	imb = &imbdevice[unit];
	if ( !imb->initialised ) continue;
	if ( imb->status & STS_DISABLED ) continue;

        /*
         * Check the interrupt channel
         */
        if ( imb->int_chan == irq )
	{
	    debug( "imbintr:  unit = %d\n", unit );
	    reg = board_address[unit];

	    addr_icr	= (short)&(reg->icr);
	    addr_isr	= (short)&(reg->isr);
	    addr_osr	= (short)&(reg->osr);
	    addr_error	= (short)&(reg->reset_error);
	    /*
	     * Kill all interrupts until we're done
	     */
	    WRITEREG( addr_icr, 0 );
	    /*
	     * Process error interrupt
	     */
	    if ( (imb->icr & INT_ERROR ) && 
		 ((READREG( addr_error ) & 1) == error_set ) )
	    {
		/*
		 * Disable error intrerrupts and wake any sleeping processes
		 */
		debug( "imbintr:  error interrupt\n" );

		imb->icr &= ~INT_ERROR;

		wakeup( (caddr_t)&(imb->read_bp) );
		wakeup( (caddr_t)&(imb->write_bp) );
		wakeup( (caddr_t)&(imb->dma) );
	    }
	    /*
	     * Process input ready interrupt;  wake up sleeping process
	     */
	    if ( (imb->icr & INT_RECEIVE_READY) &&
		 ( READREG( addr_isr ) & LINK_READY ) )
	    {
		debug( "imbintr:  read interrupt\n" );

		/*
		 * Clear interrupt flag
		 */
		imb->icr &= ~INT_RECEIVE_READY;
		/*
		 * Wake up sleeping process
		 */
		if ( imb->iomode & IMB_READ_SLEEP )
		{	
		    wakeup( (caddr_t)&(imb->read_bp) );
		}
	    }
	    /*
	     * Process output ready interrupt;  wake up sleeping process
	     */
	    if ( (imb->icr & INT_SEND_READY) &&
		 ( READREG( addr_osr ) & LINK_READY ) )
	    {
		debug( "imbintr:  write interrupt\n" );

		/*
		 * Link ready - clear interrupt flag
		 */
		imb->icr &= ~INT_SEND_READY;
		/*
		 * Wake up process
		 */
		if ( imb->iomode & IMB_WRITE_SLEEP )
		{
		    wakeup( (caddr_t)&(imb->write_bp) );
		}
	    }
	    /*
	     * Check for DMA done - Ha!, easier said than done.
	     * The B008 won't tell you when dma is done, you have
	     * to read the residual count
	     */
	    if ( imb->iomode & IMB_DMA_SLEEP )
	    {
		if ( dma_resid( imb->dma_chan ) == 0 )
                {
                    /*
                     * DMA done
                     */
		    wakeup( (caddr_t)&(imb->dma) );
		    imb->icr &= ~INT_DMA;
		}
	    }
	    /*
	     * Re-enable interrupts
	     */
	    WRITEREG( addr_icr, imb->icr );

	    break;
	}
    }
    return;
}

/*############################################################################*/static void imbtimeout( arg )

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
        debug( "imb%d: read timeout\n", unit );
        imbdevice[unit].read_abort |= ABORT_TIMEOUT;
        wakeup( (caddr_t)&(imbdevice[unit].read_bp) );
    }
    else
    {
        debug( "imb%d: write timeout\n", unit );
        imbdevice[unit].write_abort |= ABORT_TIMEOUT;
        wakeup( (caddr_t)&(imbdevice[unit].write_bp) );
    }
 
    return;
}

/*############################################################################*/
static int imb_delay( n, m )

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
