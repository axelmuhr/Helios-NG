/*
 * File:	xcb.h
 * Subsystem:	Helios-ARM implementation
 * Author:	P.A.Beskeen
 * Date:	Oct '92
 *
 * Description: Description of the ARM based Acorn Achimedies series
 *		expansion card bus (XCB). This used to be known as the
 *		'podule' interface.
 *
 *
 * RcsId: $Id: xcb.h,v 1.1 1993/08/03 17:11:45 paul Exp $
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 *     All Rights Reserved.
 * 
 */

#ifndef	__xcb_h
#define	__xcb_h


/* Expansion card bus is interfaced through the ARM IOC chip */

#include <ARM/hw_arch.h>


/* Manditory expansion card identification info. */

typedef struct xcb_id
{
    volatile unsigned char
 	id0,		_pad0[3],
	flags,		_pad1[3],
	unused,		_pad2[3],
	product_lo,	_pad3[3],
	product_hi,	_pad4[3],
	company_lo,	_pad5[3],
	company_hi,	_pad6[3],
	country,	_pad7[3];
    union
    {
	volatile char		d_char[4];
	volatile unsigned char	d_uchar[4];
	volatile short		d_short[2];
	volatile unsigned short	d_ushort[2];
	volatile int		d_int;
	volatile unsigned int	d_uint;
    } data[1024-8];
} xcb_id;


/* Expansion card address macros. */

#define XCB_SPEED_SLOW		0
#define XCB_SPEED_MEDIUM	1
#define XCB_SPEED_FAST		2
#define XCB_SPEED_SYNC		3			/* slowest! */

#define XCB_base		0x03240000

/* Address offset for given expansion card slot number. */
#define XCB_SLOT_OFF(slot)	((slot) << 14)

/* Return address of XCB memory for given card number and access speed. */
#define XCB_ADDRESS(speed, slot) \
		(XCB_base + (XCB_SPEED_##speed << 19) + XCB_SLOT_OFF(slot))

/* Return address of xcb_id structure for given card number. */
#define XCB_IDx(speed, slot) \
		((xcb_id *)(XCB_ADDRESS (speed, slot)))

/* Return address of xcb_id structure for given card number (at std speed). */
#define XCB_ID(slot) \
		XCB_IDx(SYNC, slot)



/*
 * Some Archimedies hardware has a expansion card interrupt source register
 * (R140, R260, R540, ...). @@@ How do we detect if a machine has this facility?
 * This defines which cards are generating IRQ interrupts, freeing the
 * kernel from examining each card to see if it has raised an interrupt.
 *
 * The low 4 bits of the req and mask registers correspond to slots 0-3.
 * The high 4 bits are undefined.
 */

typedef struct xcb_irqregs
{
    volatile unsigned char req, _pad1[3], mask, _pad2[3];
} xcb_irqregs;

#define XCB_IRQREGS_REGS	((xcb_irqregs *)0x03360000)


/* xcb_id.id0 bits. */

#define XCB_ID0_IRQ		(1 << 0)   /* set if IRQ outstanding */
#define XCB_ID0_NOT_PRESENT	(1 << 1)   /* pull-up gives 1 if slot empty */
#define XCB_ID0_FIQ		(1 << 2)   /* set if FIQ outstanding */
#define XCB_ID0_EXTRA		0xF8	   /* remaining 5 bits (normally 0) */
#define XCB_ID0_SHORT_ID_MASK	(0xF << 3) /* short ID bits (normally 0) */
#define XCB_ID0_SHORT_ID__SHIFT	3	   /* shift for same */
#define XCB_ID0_NOT_CONFORMANT	(1 << 7)   /* doesn't conform to Acorn spec. */


/* Expansion card country numbers. */

#define XCB_COUNTRY_UK		0
#define XCB_COUNTRY_ITALY	4
#define XCB_COUNTRY_SPAIN	5
#define XCB_COUNTRY_FRANCE	6
#define XCB_COUNTRY_GERMANY	7
#define XCB_COUNTRY_PORTUGAL	8
#define XCB_COUNTRY_GREECE	10
#define XCB_COUNTRY_SWEDEN	11
#define XCB_COUNTRY_FINLAND	12
#define XCB_COUNTRY_DENMARK	14
#define XCB_COUNTRY_NORWAY	15
#define XCB_COUNTRY_ICELAND	16
#define XCB_COUNTRY_CANADA	17
#define XCB_COUNTRY_TURKEY	20
#define XCB_COUNTRY_IRELAND	22
#define XCB_COUNTRY_HONGKONG	23


/* Expansion card manufacturing company numbers. */

#define XCB_COMPANY_ACORNUK			0
#define XCB_COMPANY_ACORNUSA			1
#define XCB_COMPANY_OLIVETTI			2
#define XCB_COMPANY_WATFORD			3
#define XCB_COMPANY_COMPUTERCONCEPTS		4
#define XCB_COMPANY_INTELIGENTINTERFACES	5
#define XCB_COMPANY_CAMAN			6
#define XCB_COMPANY_ARMADILLO			7
#define XCB_COMPANY_SOFTOPTION			8
#define XCB_COMPANY_WILDVISION			9
#define XCB_COMPANY_ANGLOCOMPUTERS		10
#define XCB_COMPANY_RESOURCE			11
#define XCB_COMPANY_ALLIEDINTERACTIVE		12
#define XCB_COMPANY_MSBURYCONSULTANTS		13
#define XCB_COMPANY_GNOME			14
#define XCB_COMPANY_AANDGELECTRONICS		15
#define XCB_COMPANY_SPACETECH			16
#define XCB_COMPANY_ATOMWIDELTD			17
#define XCB_COMPANY_SYNTEC			18
#define XCB_COMPANY_ELECTROMUSICRESEARCH	19
#define XCB_COMPANY_MILLIPEDE			20
#define XCB_COMPANY_VIDEOELECTRONICSLTD		21
#define XCB_COMPANY_BRAINSOFT			22
#define XCB_COMPANY_ASP				23
#define XCB_COMPANY_LENDACDATASYSTEMS		24
#define XCB_COMPANY_CAMBRIDGEMICROSYSTEMS	25
#define XCB_COMPANY_JOHNBALANCECOMPUTING	26
#define XCB_COMPANY_SIPLANELECTRONICSRESEARCH	27
#define XCB_COMPANY_SCIENCEFRONTIERS		28
#define XCB_COMPANY_PINEAPPLESOFTWARE		29
#define XCB_COMPANY_TECHNOMATIC			30
#define XCB_COMPANY_IRLAMENTERPRISE		31
#define XCB_COMPANY_NEXUSELECTRONICS		32
#define XCB_COMPANY_OAKCOMPUTERS		33
#define XCB_COMPANY_HUGHSYMONS			34
#define XCB_COMPANY_BEEBUG			35


/* Expansion card product numbers.*/

#define XCB_PRODUCT_HOSTTUBE			0 /* To a BBC */
#define XCB_PRODUCT_PARASITETUBE		1 /* To a second processor */
#define XCB_PRODUCT_SCSI			2
#define XCB_PRODUCT_ETHERNET			3
#define XCB_PRODUCT_IBMDISC			4
#define XCB_PRODUCT_RAMROM			5
#define XCB_PRODUCT_BBCIO			6
#define XCB_PRODUCT_MODEM			7
#define XCB_PRODUCT_TELETEXT			8
#define XCB_PRODUCT_CDROM			9
#define XCB_PRODUCT_IEEE488			10
#define XCB_PRODUCT_WINCHESTER			11
#define XCB_PRODUCT_ESDI			12
#define XCB_PRODUCT_SMD				13
#define XCB_PRODUCT_LASERPRINTER		14
#define XCB_PRODUCT_SCANNER			15
#define XCB_PRODUCT_FASTRING			16
#define XCB_PRODUCT_FASTRING2			17
#define XCB_PRODUCT_PROMPROGRAMMER		18
#define XCB_PRODUCT_MIDI			19
#define XCB_PRODUCT_MONOVPU			20
#define XCB_PRODUCT_FRAMEGRABBER		21
#define XCB_PRODUCT_SOUNDSAMPLER		22
#define XCB_PRODUCT_VIDEODIGITISER		23
#define XCB_PRODUCT_GENLOCK			24
#define XCB_PRODUCT_CODECSAMPLER		25
#define XCB_PRODUCT_IMAGEANALYSER		26
#define XCB_PRODUCT_ANALOGUEINPUT		27
#define XCB_PRODUCT_CDSOUNDSAMPLER		28
#define XCB_PRODUCT_6MIPSSIGPROC		29
#define XCB_PRODUCT_12MIPSSIGPROC		30
#define XCB_PRODUCT_33MIPSSIGPROC		31
#define XCB_PRODUCT_TOUCHSCREEN			32
#define XCB_PRODUCT_TRANSPUTERLINK		33
#define XCB_PRODUCT_INTERACTIVEVIDEO		34
#define XCB_PRODUCT_LASERSCANNER		35
#define XCB_PRODUCT_TRANSPUTERLINKADAPTER	36 /* Gnome version */
#define XCB_PRODUCT_VMEBUS			37
#define XCB_PRODUCT_TAPESTREAMER		38
#define XCB_PRODUCT_LASERTEST			39
#define XCB_PRODUCT_COLOURDIGITISER		40
#define XCB_PRODUCT_WEATHERSATELLITE		41
#define XCB_PRODUCT_AUTOCUE			42
#define XCB_PRODUCT_PARALLELIO16BIT		43
#define XCB_PRODUCT_12BITATOD			44
#define XCB_PRODUCT_SERIALPORTRS423		45
#define XCB_PRODUCT_MINI			46
#define XCB_PRODUCT_FRAMEGRABBER2		47
#define XCB_PRODUCT_INTERACTIVEVIDEO2		48
#define XCB_PRODUCT_WILDATOD			49
#define XCB_PRODUCT_WILDDTOA			50
#define XCB_PRODUCT_EMRMIDI4			51
#define XCB_PRODUCT_FLOATINGPOINTCP		52
#define XCB_PRODUCT_PRISMA3			53
#define XCB_PRODUCT_ARVIS			54
#define XCB_PRODUCT_4BY4MIDI			55
#define XCB_PRODUCT_BISERIALPARALLEL		56
#define XCB_PRODUCT_CHROMA300GENLOCK		57
#define XCB_PRODUCT_CHROMA400GENLOCK		58
#define XCB_PRODUCT_COLOURCONVERTER		59
#define XCB_PRODUCT_8BITSAMPLER			60
#define XCB_PRODUCT_PLUTOINTERFACE		61
#define XCB_PRODUCT_LOGICANALYSER		62
#define XCB_PRODUCT_USERPORTANDMIDI		63
#define XCB_PRODUCT_JBCOMPUTINGSCSI		64
#define XCB_PRODUCT_SIPLANADCANDDAC		65
#define XCB_PRODUCT_DUALUSERPORT		66
#define XCB_PRODUCT_EMRSAMPLER8			67
#define XCB_PRODUCT_EMRSMTP			68
#define XCB_PRODUCT_EMRMIDI2			69
#define XCB_PRODUCT_PINEAPPLEDIGITISER		70
#define XCB_PRODUCT_VIDEOFRAMECAPTURE		71
#define XCB_PRODUCT_MONOOVERLAYFRSTORE		72
#define XCB_PRODUCT_MARKETBUFFER		73
#define XCB_PRODUCT_PAGESTORE			74
#define XCB_PRODUCT_TRAMMMOTHERBOARD		75
#define XCB_PRODUCT_TRANSPUTER			76
#define XCB_PRODUCT_OPTICALSCANNER		77
#define XCB_PRODUCT_DIGITISINGTABLET		78
#define XCB_PRODUCT_200DPISCANNER		79
#define XCB_PRODUCT_DIGITALIO			80
#define XCB_PRODUCT_PRESENTERGENLOCK		81
#define XCB_PRODUCT_COLOURFRAMEGRABBER		82
#define XCB_PRODUCT_CHROMA200GENLOCK		83
#define XCB_PRODUCT_WVSOUNDSAMPLER		84
#define XCB_PRODUCT_SMTPEINTERFACE		85
#define XCB_PRODUCT_8BITATOD			86
#define XCB_PRODUCT_MFMHDCONTROLLER		87
#define XCB_PRODUCT_OAKSCSI			88
#define XCB_PRODUCT_QUADSERIAL			89
#define XCB_PRODUCT_PALPROGRAMMER		90
#define XCB_PRODUCT_I2CBUS			91
#define XCB_PRODUCT_BEEBUGSCANNER		92
#define XCB_PRODUCT_ETHERNET2			97


/* No acorn hardware currently has more than four slots. */

#define XCB_MAX_SLOTS	4


/*
 * If two Acorn ethernet cards used thick Ethernet, then all the available
 * 12V current would be used up. @@@ If this condition is detected, then a
 * warning should be issued.
 */

#define XCB_MAX_ETHERNET_CARDS	2


#endif/*__xcb_h*/

/* end of xcb.h */
