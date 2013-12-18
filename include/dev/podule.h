/* $Header: /dsl/HeliosARM/nick/RCS/podule.h,v 1.1 1991/03/03 22:15:35 paul Exp $ */
/* $Source: /dsl/HeliosARM/nick/RCS/podule.h,v $ */
/************************************************************************/ 
/* podule.h - Access to podules from ARM Helios				*/
/*									*/
/* Copyright (C) 1989 Active Book Company Ltd., Cambridge, England	*/
/*									*/
/* Author: Brian Knight, 17th November 1989				*/
/************************************************************************/

/*
 * $Log: podule.h,v $
 * Revision 1.1  1991/03/03  22:15:35  paul
 * Initial revision
 *
 */

#ifndef  _PODULE_H_
#define  _PODULE_H_

/* Structure which describes memory-mapped podule id registers */
/* Note the use of volatile!				       */

typedef struct PoduleId
{
  volatile unsigned int  irq:1, not_present:1, fiq:1, id_low:5, :0;
  volatile unsigned char flags,	     _pad1[3];
  volatile unsigned char unused,     _pad2[3];
  volatile unsigned char product_lo, _pad3[3];
  volatile unsigned char product_hi, _pad4[3];
  volatile unsigned char company_lo, _pad5[3];
  volatile unsigned char company_hi, _pad6[3];
  volatile unsigned char country,    _pad7[3];
  union
  {
    volatile char	    d_char[4];
    volatile unsigned char  d_uchar[4];
    volatile short	    d_short[2];
    volatile unsigned short d_ushort[2];
    volatile int	    d_int;
    volatile unsigned int   d_uint;
  } data[1024-8];
} PoduleId, *RefPoduleId;

/* bits in c_id0 field: */
#define PODULE_ID0_IRQ		(1 << 0)  /* set if IRQ outstanding */
#define PODULE_ID0_NOT_PRESENT	(1 << 1)  /* pull-up gives 1 if slot empty */
#define PODULE_ID0_FIQ		(1 << 2)  /* set if FIQ outstanding */
#define PODULE_ID0_EXTRA	(0xF8)	  /* remaining 5 bits should be 0 */


/* Podule source country codes */

#define PODULE_COUNTRY_UK	0
#define PODULE_COUNTRY_ITALY	4
#define PODULE_COUNTRY_SPAIN	5
#define PODULE_COUNTRY_FRANCE	6
#define PODULE_COUNTRY_GERMANY	7
#define PODULE_COUNTRY_PORTUGAL	8
#define PODULE_COUNTRY_GREECE	10
#define PODULE_COUNTRY_SWEDEN	11
#define PODULE_COUNTRY_FINLAND	12
#define PODULE_COUNTRY_DENMARK	14
#define PODULE_COUNTRY_NORWAY	15
#define PODULE_COUNTRY_ICELAND	16
#define PODULE_COUNTRY_CANADA	17
#define PODULE_COUNTRY_TURKEY	20

/* Podule manufacturing company codes */

#define PODULE_COMPANY_ACORNUK			0
#define PODULE_COMPANY_ACORNUSA			1
#define PODULE_COMPANY_OLIVETTI			2
#define PODULE_COMPANY_WATFORD			3
#define PODULE_COMPANY_COMPUTERCONCEPTS		4
#define PODULE_COMPANY_INTELIGENTINTERFACES	5
#define PODULE_COMPANY_CAMAN			6
#define PODULE_COMPANY_ARMADILLO		7
#define PODULE_COMPANY_SOFTOPTION		8
#define PODULE_COMPANY_WILDVISION		9
#define PODULE_COMPANY_ANGLOCOMPUTERS		10
#define PODULE_COMPANY_RESOURCE			11
#define PODULE_COMPANY_GNOME			14

/* Podule product type codes */

#define PODULE_PRODUCT_HOSTTUBE			0 /* To a BBC */
#define PODULE_PRODUCT_PARASITETUBE		1 /* To a second processor */
#define PODULE_PRODUCT_SCSI			2
#define PODULE_PRODUCT_ETHERNET			3
#define PODULE_PRODUCT_IBMDISC			4
#define PODULE_PRODUCT_RAMROM			5
#define PODULE_PRODUCT_BBCIO			6
#define PODULE_PRODUCT_MODEM			7
#define PODULE_PRODUCT_TELETEXT			8
#define PODULE_PRODUCT_CDROM			9
#define PODULE_PRODUCT_IEEE488			10
#define PODULE_PRODUCT_WINCHESTER		11
#define PODULE_PRODUCT_ESDI			12
#define PODULE_PRODUCT_SMD			13
#define PODULE_PRODUCT_LASERPRINTER		14
#define PODULE_PRODUCT_SCANNER			15
#define PODULE_PRODUCT_FASTRING			16
#define PODULE_PRODUCT_VMEBUS			17
#define PODULE_PRODUCT_PROMPROGRAMMER		18
#define PODULE_PRODUCT_MIDI			19
#define PODULE_PRODUCT_MONOVPU			20
#define PODULE_PRODUCT_FRAMEGRABBER		21
#define PODULE_PRODUCT_SOUNDSAMPLER		22
#define PODULE_PRODUCT_VIDEODIGITISER		23
#define PODULE_PRODUCT_GENLOCK			24
#define PODULE_PRODUCT_CODECSAMPLER		25
#define PODULE_PRODUCT_IMAGEANALYSER		26
#define PODULE_PRODUCT_ANALOGUEINPUT		27
#define PODULE_PRODUCT_CDSOUNDSAMPLER		28
#define PODULE_PRODUCT_6MIPSSIGPROC		29
#define PODULE_PRODUCT_12MIPSSIGPROC		30
#define PODULE_PRODUCT_33MIPSSIGPROC		31
#define PODULE_PRODUCT_TOUCHSCREEN		32
#define PODULE_PRODUCT_TRANSPUTERLINK		33
#define PODULE_PRODUCT_TRANSPUTERLINKADAPTER	36 /* Gnome version */
#define PODULE_PRODUCT_TRAMBOARD		75 /* Gnome TRAM podule */
#define PODULE_PRODUCT_TRANSPUTER		76 /* Gnome transputer board */


#define PODULE_SLOT_OFF(slot)	((slot) << 18)
#define PODULE_ADDRESS(slot) 	(0x00C00000 + PODULE_SLOT_OFF(slot))


/* PODULE manager interface details */

#define PODULE_SLOTS	4


/*
 * The PODULEInfo structure contains card-identification information, as 
 * stored in ARM memory.  Each expansion card must also contain these
 * codes, in a standard format and location, so that the PODULE manager 
 * can read the card ID and hence match cards to drivers.
 */
typedef struct PODULE_info
{
    unsigned short product, company;
    unsigned char  country;
} PODULEInfo, *PODULEInfoRef;

#ifdef NOT_YET
typedef struct PODULE_io_mem
{
    struct PODULE_io_mem  *next;			/* in access-def list */
    int flags;					/* b/w, r/w, speed, multi etc */
    unsigned short offset, len;			/* area loc/len in words  */
} XCIOMem, *XCIOMemRef;
/*
 * Flags bits: 
 *	0..1:	I/O cycle speed use PODULE_SPEED_xxx 
 *	2:	memc-space address (speed ignored)
 *	3:	readable
 *	4	writable
 *	5:	byte-wide (0 => halfword-wide) access
 *	6:	can use ldm/stm access as appropriate in this area
 *	7:	is multi-mapped fifo register (requires bit 6 also)
 */
#define XCIO_SPEED__MASK	(3 << 0)	/*  */
#endif /* NOT_YET */

    
/*
 * Expansion Card type record.  This is the means whereby an PODULE
 * device driver is linked with the PODULE manager, in a fashion
 * analogous to the normal cdevsw and bdevsw tables (note that most
 * expansion card drivers will also have entries in one or both of
 * those tables).  The file "PODULEconf.c" in the kernel build directory
 * contains a table, PODULE_type[], of expansion card type structures,
 * one for each supported card type; the count of entries is set in
 * the external int nPODULE_type.  Adding an PODULE device driver requires
 * the inclusion in PODULE_type of an entry for the card type.  At boot
 * time, the expansion card bus is scanned for expansion cards; the ID
 * of each installed card is determined by reading the first few bytes
 * of PODULE ID space for the slot the card resides in.  Only those cards
 * with IDs matching entries in PODULE_type will be considered valid as
 * far as RISC iX is concerned.  For all valid cards, a sequence of
 * initialisation is then performed, as follows:
 * 
 *    {cpu priority level still at MAX_SPL}
 *    for each valid card		{round 1}
 * 	 if (card_type->init_high != NULL)
 * 	     (*card_type->init_high) (card_slot);
 *
 *    {cpu priority reduced to 0, but PODULE interrupts disabled}
 *    for each valid card		{round 2}
 * 	 if (card_type->init_low != NULL)
 * 	     (*card_type->init_low) (card_slot, 0);
 *    enable interrupts from expansion card bus
 *    for each valid card		{round 3}
 * 	 if (card_type->init_low != NULL)
 * 	     (*card_type->init_low) (card_slot, 1);
 * 
 * The first round of calls is designed to allow a driver to bring its
 * hardware to a stable, non-interrupting, initialised state, before the
 * processor interrupt priority is dropped from its initial maximum
 * and interrupts are allowed in.  The "init_high" function must not
 * reduce the processor priority.
 *
 * The second round of calls, via the "init_low" field, permits
 * further initialisation if required, with general interrupts
 * enabled, but no interrupts from expansion cards (marked by the
 * second parameter being 0).  The standard Ethernet card uses this
 * call to perform a test routine during which each configured card is
 * checked for correct functionality: if the card is OK then it is
 * added to the driver's tables and an interrupt handler is declared
 * for it using the function decl_PODULE_interrupt().
 *
 * During either of the first two initialisation calls to a driver,
 * the driver may call the function decl_PODULE_interrupt(), to declare
 * to the interrupt manager the existence of an interrupt handler for
 * IRQs from a specific card.  At most one call may be made, for each
 * slot in which a valid card has been found.  In the case that the
 * hardware configuration does not include a full-spec backplane, as
 * indicated by (PODULE_regs == (PODULERegsRef)0), any call to this function
 * (and to the function decl_PODULE_irq_sense, for non-standard card
 * interrupt indication) MUST happen at this time: an attempt to call
 * it later will cause a panic.
 *
 * The third and last round of initialisation calls uses the same
 * entry point as the second round, but with a second parameter of 1,
 * marking the fact that by this point interrupts from expansion cards
 * have been enabled, and all declared PODULE interrupt handlers can
 * potentially be called.  Again the processor priority level is 0.
 * The Ethernet driver uses this call to perform final setup for each
 * configured working Ethernet card, during which time the Ethernet
 * interrupt handler is functioning normally.
 *
 * During the second and third calls, the driver may raise and lower
 * priority as required, using the normal splnnn calls, but should
 * return with the SPL restored to 0.
 *
 * The finally PODULE driver entry point is used when the system reboot
 * or halt mechanism is invoked (e.g. via the "reboot" or "halt"
 * programs); the function specified in the shutdown field will be
 * called, as follows:
 *
 *    {cpu priority at maximum}
 *    foreach valid card:
 * 	 if (card_type->shutdown != NULL)
 * 	     (*card_type->shutdown) (card_slot);
 *
 * This function should arrange to close off any activity on the
 * device and bring the card to a stable quiescent state such that
 * when the kernel passes control to RISC OS, the card (a) will not
 * generate any interrupts, and (b) is in a condition equivalent to
 * the hardware reset state, such that the ID bytes used to identify
 * the card are readable at the standard address.
 *
 * In all cases, the called function is passed the physical slot
 * number of the expansion card as its first only parameter; from this
 * the device address may be derived.  Multiple instances of the same
 * card type on the bus will cause multiple calls on the function, one
 * for each slot in which a card of the given type is found.
 */

typedef struct PODULE_type
{
    PODULEInfo id;					/* ROM ID values */
    int flags;					/* for various purposes */
#ifdef NOTYET
    XCIOMemRef io_mem;				/* for user access */
#endif /* NOTYET */
    void (*init_high) (/*int slot*/);		/* init function 1, max prio */
    void (*init_low) (/*int slot*/);		/* init function 2, prio 0 */
    void (*shutdown) (/*int slot*/);		/* reboot shutdown function */
} PODULEType, *PODULETypeRef;


#ifdef KERNEL
extern PODULEType PODULE_type[];
extern int nPODULE_type;
#endif



#ifdef KERNEL
/*
 * decl_PODULE_interrupt
 *
 * The parameters to decl_PODULE_interrupt have the following meanings:
 *
 * slot:	0..(PODULE_SLOTS-1) - the card's physical slot number.
 * handler:	the address of a statically allocated interrupt handler
 *		record.  The ih_fn field should be set to the address of
 *		the interrupt handling function, and the ih_farg field 
 *		should contain a value to be passed to this function on
 *		interrupt; the ih_mask and ih_next fields must be left 
 *		entirely alone by the driver.
 * priority:	the interrupt level at which IRQs from the expansion card
 *		should be serviced (i.e. a PRIO_xxxx value).  Normally,
 *		interrupts from the slot will actually arrive at this spl;
 *		however on those few systems configured without a proper
 *		backplane but with multiple expansion cards running at 
 *		different priorities, the lowest specified priority will
 *		be used for the single hardware interrupt level, but the
 *		handler function will be called with the spl level set to
 *		the requested level.
 *
 * This requests that whenever an expansion card in the specified slot
 * generates an IRQ, handler->ih_fn will be called, with argument
 * handler->ih_farg, with spl set to priority.
 */

extern void decl_PODULE_interrupt (/* int slot, 
				   struct int_hndlr *handler,
				   int priority */);
#endif /* KERNEL */

/*
 * Structure describing the way in which an interrupt from a non-
 * standard expansion card may be detected by the PODULE manager.  On
 * systems with a proper backplane this mechanism is not used: the
 * backplane registers give indication of individual PODULE interrupts.
 * On systems (regretably there still are a number) without a proper
 * backplane, we need to cope with PODULE devices where the "standard"
 * IRQ flag is not used (ID byte 0, bit 0, positive logic).  Such
 * devices include the winchester expansion card (which has a separate
 * register) and early (pre-issue 1) ethernet cards, where the ID byte
 * must be read at FAST speed as opposed to SYNC.  This mechanism
 * assumes that an interrupting card may be identified by reading a
 * flag word from a (slot-adjusted) address and testing it against a
 * mask, the result indicating IRQ presence (!=0) or absence (==0).
 */
typedef struct PODULE_irq_sense
{
    int *flag;		/* Note: slot address bits set by PODULE manager */
    int mask;
} PODULEIRQSense, *PODULEIRQSenseRef;

#ifdef KERNEL
/*
 * decl_PODULE_irq_sense need be called only for non-standard cards
 * in systems where the full-spec backplane hardware is not fitted.
 * This can be tested by checking the contents of the external
 * variable PODULE_regs (see PODULEregs.h).  See the comment above about
 * the meaning of the PODULEIRQSense structure.  Note that the slot-
 * specific part of the device address (bits 14,15) is controlled
 * by the PODULE manager, and should be left 0 in the flag field of
 * the structure passed in.
 */
extern void decl_PODULE_irq_sense (/*int slot, PODULEIRQSenseRef sense*/);
#endif /* KERNEL */


#endif /* _PODULE_H_ */

/* End of podule.h */
