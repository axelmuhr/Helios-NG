/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1990
*******************************************************************************
******************************************************************************/

/******************************************************************************
 BOARD ID DEFINITIONS

 32 Bits of information are returned by 'GetBoardID ()'.

	The low order 16 bits correspond to the Feature Bits which make
	up a unique ID for a given class of boards.

		e.g. STARLAN MEDIA, INTERFACE_CHIP, MICROCHANNEL

		note: board ID should be ANDed with the STATIC_ID_MASK
		      before comparing to a specific board ID


	The high order 16 bits correspond to the Extra Bits which do not
	change the boards ID.

		e.g. INTERFACE_584_CHIP, 16 BIT SLOT, ALTERNATE IRQ

******************************************************************************/

#define	STARLAN_MEDIA		0x00000001	/* StarLAN */
#define	ETHERNET_MEDIA		0x00000002	/* Ethernet */
#define	TWISTED_PAIR_MEDIA	0x00000003	/* Twisted Pair */
#define	EW_MEDIA		0x00000004	/* Ethernet and Twisted Pair */
#define	MICROCHANNEL		0x00000008	/* MicroChannel Adapter */
#define	INTERFACE_CHIP		0x00000010	/* Soft Config Adapter */
#define	BID_UNUSED_1		0x00000020	/* used to be INTELLIGENT */
#define	BOARD_16BIT		0x00000040	/* 16 bit capability */
#define	RAM_SIZE_UNKNOWN	0x00000000	/* 000 => Unknown RAM Size */
#define	RAM_SIZE_RESERVED_1	0x00010000	/* 001 => Reserved */
#define	RAM_SIZE_8K		0x00020000	/* 010 => 8k RAM */
#define	RAM_SIZE_16K		0x00030000	/* 011 => 16k RAM */
#define	RAM_SIZE_32K		0x00040000	/* 100 => 32k RAM */
#define	RAM_SIZE_64K		0x00050000	/* 101 => 64k RAM */ 
#define	RAM_SIZE_RESERVED_6	0x00060000	/* 110 => Reserved */ 
#define	RAM_SIZE_RESERVED_7	0x00070000	/* 111 => Reserved */ 
/* #define	BID_UNUSED_2		0x00010000 */	/* used to be RAM Size field */
/* #define	BID_UNUSED_3		0x00020000 */	/* used to be RAM Size field */
/* #define	BID_UNUSED_4		0x00040000 */	/* used to be RAM Size field */
#define	SLOT_16BIT		0x00080000	/* 16 bit board - 16 bit slot */
#define	NIC_690_BIT		0x00100000	/* NIC is 690 */
#define	ALTERNATE_IRQ_BIT	0x00200000	/* Alternate IRQ is used */
#define	INTERFACE_5X3_CHIP	0x00000000	/* 0000 = 583 or 593 chips */
#define	INTERFACE_584_CHIP	0x00400000	/* 0001 = 584 chip */
#define	INTERFACE_594_CHIP	0x00800000	/* 0010 = 594 chip */

#define	MEDIA_MASK		0x00000007	/* Isolates Media Type */
#define	RAM_SIZE_MASK		0x00070000	/* Isolates RAM Size */
#define	STATIC_ID_MASK		0x0000FFFF	/* Isolates Board ID */
#define	INTERFACE_CHIP_MASK	0x03C00000	/* Isolates Intfc Chip Type */

/* Word definitions for board types */
#define	WD8003E		ETHERNET_MEDIA
#define	WD8003EBT	WD8003E		/* functionally identical to WD8003E */
#define	WD8003S		STARLAN_MEDIA
#define	WD8003SH	WD8003S		/* functionally identical to WD8003S */
#define	WD8003WT	TWISTED_PAIR_MEDIA
#define	WD8003W		(TWISTED_PAIR_MEDIA | INTERFACE_CHIP)
#define	WD8003EB	(ETHERNET_MEDIA | INTERFACE_CHIP)
#define	WD8003EP	WD8003EB	/* with INTERFACE_584_CHIP */
#define	WD8003EW	(EW_MEDIA | INTERFACE_CHIP)
#define	WD8003ETA	(ETHERNET_MEDIA | MICROCHANNEL)
#define	WD8003STA	(STARLAN_MEDIA | MICROCHANNEL)
#define	WD8003EA	(ETHERNET_MEDIA | MICROCHANNEL | INTERFACE_CHIP)
#define	WD8003EPA	WD8003EA	/* with INTERFACE_594_CHIP */
#define	WD8003SHA	(STARLAN_MEDIA | MICROCHANNEL | INTERFACE_CHIP)
#define	WD8003WA	(TWISTED_PAIR_MEDIA | MICROCHANNEL | INTERFACE_CHIP)
#define	WD8003WPA	WD8003WA	/* with INTERFACE_594_CHIP */
#define	WD8013EBT	(ETHERNET_MEDIA | BOARD_16BIT)
#define	WD8013EB	(ETHERNET_MEDIA | BOARD_16BIT | INTERFACE_CHIP)
#define	WD8013W		(TWISTED_PAIR_MEDIA | BOARD_16BIT | INTERFACE_CHIP)
#define	WD8013EW	(EW_MEDIA | BOARD_16BIT | INTERFACE_CHIP)

/******************************************************************************

Declaration for the Routine Provided in the 'Board ID' Library.

******************************************************************************/
#if M_XENIX || AT286 || AT386 || M_UNIX
unsigned	long	GetBoardID ();
#else
unsigned	long	GetBoardID (unsigned int, int);
#endif

