/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1990
*******************************************************************************
******************************************************************************/

#define	MIN_IRQ			2
#define	MAX_IRQ			15
#define	NUM_IRQS		10
#define	D_IRQ_INDEX		1
#define	D_IRQ_VAL		3

#define	MIN_ROMBASE		0xC0000000
#define	MAX_ROMBASE		0xDE000000
#define	NUM_ROMBASES		16
#define	D_ROMBASE_INDEX		13
#define	D_ROMBASE_VAL		0xD8000000

#define	NUM_ROMSIZES		5
#define	D_ROMSIZE_INDEX		0
#define	D_ROMSIZE_VAL		0

#define	MIN_RAMBASE		0xA0000000
#define	MAX_RAMBASE		0xEE000000
#define	NUM_RAMBASES		48
#define	D_RAMBASE_INDEX		24
#define	D_RAMBASE_VAL		0xD0000000

#define	NUM_RAMSIZES		4
#define	D_RAMSIZE_INDEX		0
#define	D_RAMSIZE_VAL		0x2000

#define	NUM_BASEIOS		32
#define	D_BASEIO_INDEX		4
#define	D_BASEIO_VAL		0x280
