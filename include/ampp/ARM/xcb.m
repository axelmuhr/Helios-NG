--  File:	xcb.m
--  Subsystem:	Helios-ARM implementation
--  Author:	P.A.Beskeen
--  Date:	Oct '92
-- 
--  Description: Description of the ARM based Acorn Achimedies series
-- 		expansion card bus (XCB). This used to be known as the
-- 		'podule' interface.
-- 
-- 
--  RcsId: $Id: xcb.m,v 1.1 1993/08/05 13:10:57 paul Exp $
-- 
--  (C) Copyright 1992 Perihelion Software Ltd.
--      All Rights Reserved.


_report ['include xcb.m]
_def 'ARM/xcb.m_flag 1


-- Acorn expansion bus is interfaced through the ARM IOC chip.

include ARM/hw_arch.m


-- Manditory expansion card identification info.

struct xcb_id [
 	byte	id0		byte _pad0.1 byte _pad0.2 byte _pad0.3
	byte	flags		byte _pad1.1 byte _pad1.2 byte _pad1.3
	byte	unused		byte _pad2.1 byte _pad2.2 byte _pad2.3
	byte	product_lo	byte _pad3.1 byte _pad3.2 byte _pad3.3
	byte	product_hi	byte _pad4.1 byte _pad4.2 byte _pad4.3
	byte	company_lo	byte _pad5.1 byte _pad5.2 byte _pad5.3
	byte	company_hi	byte _pad6.1 byte _pad6.2 byte _pad6.3
	byte	country		byte _pad7.1 byte _pad7.2 byte _pad7.3
	vec	1016 data
]


-- Expansion card address macros.

_def 'XCB_SPEED_SLOW	0
_def 'XCB_SPEED_MEDIUM	1
_def 'XCB_SPEED_FAST	2
_def 'XCB_SPEED_SYNC	3			-- slowest!

_def 'XCB_base		0x03240000


-- Address offset for given expansion card slot number

_defq 'XCB_SLOT_OFF[slot] [
	((slot) << 14)
]


-- Return address of XCB memory for given card number and access speed

_defq 'XCB_ADDRESS[speed slot] [
	(XCB_base + (_eval [XCB_SPEED_$speed] << 19) + XCB_SLOT_OFF slot )
]


-- Return address of xcb_id structure for given card number.

_defq 'XCB_IDx[speed slot] [
	XCB_ADDRESS speed slot
]


-- Return address of xcb_id structure for given card number (at std speed).

_defq 'XCB_ID[slot] [
	XCB_IDx SYNC slot
]


-- Load address into register argument

_defq 'LD_XCB_ADDRESS[speed slot reg] [
	mov	reg, XCB_base
	add	reg, ((_eval [XCB_SPEED_$speed] << 19) + XCB_SLOT_OFF slot )
]


-- Load address into register argument, slot argument passed as a register

_defq 'LD_XCB_ADDRESS2[speed slot reg] [
	mov	reg, XCB_base
	add	reg, _eval [XCB_SPEED_$speed] << 19
	add	reg, slot LSL 14
]


-- Load address of xcb_id structure for given card number into reg

_defq 'LD_XCB_IDx[speed slot reg] [
	LD_XCB_ADDRESS speed slot reg
]


-- Load address of xcb_id structure for given card (at std speed) into reg.

_defq 'LD_XCB_ID[slot reg] [
	LD_XCB_IDx SYNC slot reg
]


-- Some Archimedies hardware has a expansion card interrupt source register
-- (R140, R260, R540, ...). @@@ How do we detect if a machine has this facility?
-- This defines which cards are generating IRQ interrupts, freeing the
-- kernel from examining each card to see if it has raised an interrupt.
--
-- The low 4 bits of the req and mask registers correspond to slots 0-3.
-- The high 4 bits are undefined.

struct xcb_irqregs [
    byte req	byte _pad1.1 byte _pad1.2 byte _pad1.3
    byte mask	byte _pad2.1 byte _pad2.2 byte _pad2.3
]

_def 'XCB_IRQREGS_REGS	0x03360000


-- xcb_id.id0 bits.

_def 'XCB_ID0_IRQ		[(1 << 0)]   -- set if IRQ outstanding
_def 'XCB_ID0_NOT_PRESENT	[(1 << 1)]   -- pull-up gives 1 if slot empty
_def 'XCB_ID0_FIQ		[(1 << 2)]   -- set if FIQ outstanding
_def 'XCB_ID0_EXTRA		0xF8	     -- remaining 5 bits (normally 0)
_def 'XCB_ID0_SHORT_ID_MASK	[(0xF << 3)] -- short ID bits (normally 0)
_def 'XCB_ID0_SHORT_ID__SHIFT	3	     -- shift for same
_def 'XCB_ID0_NOT_CONFORMANT	[(1 << 7)]   -- doesn't conform to Acorn spec.


-- Expansion card country numbers.

_def 'XCB_COUNTRY_UK		0
_def 'XCB_COUNTRY_ITALY		4
_def 'XCB_COUNTRY_SPAIN		5
_def 'XCB_COUNTRY_FRANCE	6
_def 'XCB_COUNTRY_GERMANY	7
_def 'XCB_COUNTRY_PORTUGAL	8
_def 'XCB_COUNTRY_GREECE	10
_def 'XCB_COUNTRY_SWEDEN	11
_def 'XCB_COUNTRY_FINLAND	12
_def 'XCB_COUNTRY_DENMARK	14
_def 'XCB_COUNTRY_NORWAY	15
_def 'XCB_COUNTRY_ICELAND	16
_def 'XCB_COUNTRY_CANADA	17
_def 'XCB_COUNTRY_TURKEY	20
_def 'XCB_COUNTRY_IRELAND	22
_def 'XCB_COUNTRY_HONGKONG	23


-- Expansion card manufacturing company numbers.

_def 'XCB_COMPANY_ACORNUK			0
_def 'XCB_COMPANY_ACORNUSA			1
_def 'XCB_COMPANY_OLIVETTI			2
_def 'XCB_COMPANY_WATFORD			3
_def 'XCB_COMPANY_COMPUTERCONCEPTS		4
_def 'XCB_COMPANY_INTELIGENTINTERFACES		5
_def 'XCB_COMPANY_CAMAN				6
_def 'XCB_COMPANY_ARMADILLO			7
_def 'XCB_COMPANY_SOFTOPTION			8
_def 'XCB_COMPANY_WILDVISION			9
_def 'XCB_COMPANY_ANGLOCOMPUTERS		10
_def 'XCB_COMPANY_RESOURCE			11
_def 'XCB_COMPANY_ALLIEDINTERACTIVE		12
_def 'XCB_COMPANY_MSBURYCONSULTANTS		13
_def 'XCB_COMPANY_GNOME				14
_def 'XCB_COMPANY_AANDGELECTRONICS		15
_def 'XCB_COMPANY_SPACETECH			16
_def 'XCB_COMPANY_ATOMWIDELTD			17
_def 'XCB_COMPANY_SYNTEC			18
_def 'XCB_COMPANY_ELECTROMUSICRESEARCH		19
_def 'XCB_COMPANY_MILLIPEDE			20
_def 'XCB_COMPANY_VIDEOELECTRONICSLTD		21
_def 'XCB_COMPANY_BRAINSOFT			22
_def 'XCB_COMPANY_ASP				23
_def 'XCB_COMPANY_LENDACDATASYSTEMS		24
_def 'XCB_COMPANY_CAMBRIDGEMICROSYSTEMS		25
_def 'XCB_COMPANY_JOHNBALANCECOMPUTING		26
_def 'XCB_COMPANY_SIPLANELECTRONICSRESEARCH	27
_def 'XCB_COMPANY_SCIENCEFRONTIERS		28
_def 'XCB_COMPANY_PINEAPPLESOFTWARE		29
_def 'XCB_COMPANY_TECHNOMATIC			30
_def 'XCB_COMPANY_IRLAMENTERPRISE		31
_def 'XCB_COMPANY_NEXUSELECTRONICS		32
_def 'XCB_COMPANY_OAKCOMPUTERS			33
_def 'XCB_COMPANY_HUGHSYMONS			34
_def 'XCB_COMPANY_BEEBUG			35


-- Expansion card product numbers.

_def 'XCB_PRODUCT_HOSTTUBE		0 	-- To a BBC
_def 'XCB_PRODUCT_PARASITETUBE		1	-- To a second processor
_def 'XCB_PRODUCT_SCSI			2
_def 'XCB_PRODUCT_ETHERNET		3
_def 'XCB_PRODUCT_IBMDISC		4
_def 'XCB_PRODUCT_RAMROM		5
_def 'XCB_PRODUCT_BBCIO			6
_def 'XCB_PRODUCT_MODEM			7
_def 'XCB_PRODUCT_TELETEXT		8
_def 'XCB_PRODUCT_CDROM			9
_def 'XCB_PRODUCT_IEEE488		10
_def 'XCB_PRODUCT_WINCHESTER		11
_def 'XCB_PRODUCT_ESDI			12
_def 'XCB_PRODUCT_SMD			13
_def 'XCB_PRODUCT_LASERPRINTER		14
_def 'XCB_PRODUCT_SCANNER		15
_def 'XCB_PRODUCT_FASTRING		16
_def 'XCB_PRODUCT_FASTRING2		17
_def 'XCB_PRODUCT_PROMPROGRAMMER	18
_def 'XCB_PRODUCT_MIDI			19
_def 'XCB_PRODUCT_MONOVPU		20
_def 'XCB_PRODUCT_FRAMEGRABBER		21
_def 'XCB_PRODUCT_SOUNDSAMPLER		22
_def 'XCB_PRODUCT_VIDEODIGITISER	23
_def 'XCB_PRODUCT_GENLOCK		24
_def 'XCB_PRODUCT_CODECSAMPLER		25
_def 'XCB_PRODUCT_IMAGEANALYSER		26
_def 'XCB_PRODUCT_ANALOGUEINPUT		27
_def 'XCB_PRODUCT_CDSOUNDSAMPLER	28
_def 'XCB_PRODUCT_6MIPSSIGPROC		29
_def 'XCB_PRODUCT_12MIPSSIGPROC		30
_def 'XCB_PRODUCT_33MIPSSIGPROC		31
_def 'XCB_PRODUCT_TOUCHSCREEN		32
_def 'XCB_PRODUCT_TRANSPUTERLINK	33
_def 'XCB_PRODUCT_INTERACTIVEVIDEO	34
_def 'XCB_PRODUCT_LASERSCANNER		35
_def 'XCB_PRODUCT_TRANSPUTERLINKADAPTER	36	-- Gnome version
_def 'XCB_PRODUCT_VMEBUS		37
_def 'XCB_PRODUCT_TAPESTREAMER		38
_def 'XCB_PRODUCT_LASERTEST		39
_def 'XCB_PRODUCT_COLOURDIGITISER	40
_def 'XCB_PRODUCT_WEATHERSATELLITE	41
_def 'XCB_PRODUCT_AUTOCUE		42
_def 'XCB_PRODUCT_PARALLELIO16BIT	43
_def 'XCB_PRODUCT_12BITATOD		44
_def 'XCB_PRODUCT_SERIALPORTRS423	45
_def 'XCB_PRODUCT_MINI			46
_def 'XCB_PRODUCT_FRAMEGRABBER2		47
_def 'XCB_PRODUCT_INTERACTIVEVIDEO2	48
_def 'XCB_PRODUCT_WILDATOD		49
_def 'XCB_PRODUCT_WILDDTOA		50
_def 'XCB_PRODUCT_EMRMIDI4		51
_def 'XCB_PRODUCT_FLOATINGPOINTCP	52
_def 'XCB_PRODUCT_PRISMA3		53
_def 'XCB_PRODUCT_ARVIS			54
_def 'XCB_PRODUCT_4BY4MIDI		55
_def 'XCB_PRODUCT_BISERIALPARALLEL	56
_def 'XCB_PRODUCT_CHROMA300GENLOCK	57
_def 'XCB_PRODUCT_CHROMA400GENLOCK	58
_def 'XCB_PRODUCT_COLOURCONVERTER	59
_def 'XCB_PRODUCT_8BITSAMPLER		60
_def 'XCB_PRODUCT_PLUTOINTERFACE	61
_def 'XCB_PRODUCT_LOGICANALYSER		62
_def 'XCB_PRODUCT_USERPORTANDMIDI	63
_def 'XCB_PRODUCT_JBCOMPUTINGSCSI	64
_def 'XCB_PRODUCT_SIPLANADCANDDAC	65
_def 'XCB_PRODUCT_DUALUSERPORT		66
_def 'XCB_PRODUCT_EMRSAMPLER8		67
_def 'XCB_PRODUCT_EMRSMTP		68
_def 'XCB_PRODUCT_EMRMIDI2		69
_def 'XCB_PRODUCT_PINEAPPLEDIGITISER	70
_def 'XCB_PRODUCT_VIDEOFRAMECAPTURE	71
_def 'XCB_PRODUCT_MONOOVERLAYFRSTORE	72
_def 'XCB_PRODUCT_MARKETBUFFER		73
_def 'XCB_PRODUCT_PAGESTORE		74
_def 'XCB_PRODUCT_TRAMMMOTHERBOARD	75
_def 'XCB_PRODUCT_TRANSPUTER		76
_def 'XCB_PRODUCT_OPTICALSCANNER	77
_def 'XCB_PRODUCT_DIGITISINGTABLET	78
_def 'XCB_PRODUCT_200DPISCANNER		79
_def 'XCB_PRODUCT_DIGITALIO		80
_def 'XCB_PRODUCT_PRESENTERGENLOCK	81
_def 'XCB_PRODUCT_COLOURFRAMEGRABBER	82
_def 'XCB_PRODUCT_CHROMA200GENLOCK	83
_def 'XCB_PRODUCT_WVSOUNDSAMPLER	84
_def 'XCB_PRODUCT_SMTPEINTERFACE	85
_def 'XCB_PRODUCT_8BITATOD		86
_def 'XCB_PRODUCT_MFMHDCONTROLLER	87
_def 'XCB_PRODUCT_OAKSCSI		88
_def 'XCB_PRODUCT_QUADSERIAL		89
_def 'XCB_PRODUCT_PALPROGRAMMER		90
_def 'XCB_PRODUCT_I2CBUS		91
_def 'XCB_PRODUCT_BEEBUGSCANNER		92
_def 'XCB_PRODUCT_ETHERNET2		97


-- No acorn hardware currently has more than four slots.

_def 'XCB_MAX_SLOTS		4


-- If two Acorn ethernet cards used thick Ethernet, then all the available
-- 12V current would be used up. @@@ If this condition is detected, then a
-- warning should be issued.

_def 'XCB_MAX_ETHERNET_CARDS	2



-- end of xcb.m

