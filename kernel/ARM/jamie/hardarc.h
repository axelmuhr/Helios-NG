/*> hardarc.h <*/
/*----------------------------------------------------------------------*/
/* Hardware description of Archimedes A4x0/1				*/
/*									*/
/* Note: Some of these definitions were "borrowed" from the RISCiX	*/
/*	 system header files. This should be sorted out before a	*/
/*	 release is generated.						*/
/*----------------------------------------------------------------------*/
/*-- Memory map --------------------------------------------------------*/

#define PRAM_base	(0x02000000)	/* Physical RAM base address	*/
#define IO_base		(0x03000000)	/* IO controllers		*/
#define IOC_base	(0x03200000)	/* IOC = (ioc_block *)IOC_base	*/
#define VIDC_base	(0x03400000)	/* Video Controller address	*/
#define	MEMC_base	(0x03600000)	/* Memory Controller address	*/
#define CAM_base        (0x03800000)    /* Contents Addressable Memory	*/

#define loROM_base	(0x03400000)	/* Lo-mapped ROM base address	*/
#define hiROM_base	(0x03800000)	/* Hi-mapped ROM base address	*/

#define ROM_base	(hiROM_base)	/* standard ROM mapping		*/

/*----------------------------------------------------------------------*/
/*-- MEMC description --------------------------------------------------*/

/* MEMC DMA Address Generator registers					*/
/* The data and register values are encoded onto the address bus as	*/
/* follows:								*/
/*  address = MEMC_base							*/
/*            | ((reg) << MEMC_DAG_reg_shift)				*/
/*            | (((value) << MEMC_DAG_data_shift) & MEMC_DAG_data_mask)	*/
/*									*/
#define MEMC_vinit	(0)	/* next frame start address		*/
#define MEMC_vstart	(1)	/* video DMA start address		*/
#define MEMC_vend	(2)	/* video DMA end address		*/
#define MEMC_cinit	(3)	/* cursor DMA initialisation address	*/
#define MEMC_sstart	(4)	/* start of next sound buffer		*/
#define MEMC_sendN	(5)	/* end of next sound buffer		*/
#define MEMC_sptr	(6)	/* initialise pointer to start of next	*/

#define MEMC_DAG_reg_shift	(17)		/* register shift	*/
#define MEMC_DAG_data_shift	(2)		/* data shift value	*/
#define MEMC_DAG_data_mask	(0x0001FFFC)	/* data mask value	*/

/* MEMC control register						*/
/* This register controls the DMA enabling, the memory page size and	*/
/* various other features.						*/
#define MEMC_CR_pagesize_mask	(0x3 << 2)	/* page size mask	*/
#define MEMC_CR_pagesize_4KB    (0x0 << 2)	/* 4KB physical pages	*/
#define MEMC_CR_pagesize_8KB    (0x1 << 2)	/* 8KB physical pages	*/
#define MEMC_CR_pagesize_16KB   (0x2 << 2)	/* 16KB physical pages	*/
#define MEMC_CR_pagesize_32KB   (0x3 << 2)	/* 32KB physical pages	*/

#define MEMC_CR_loROM_speed_mask   	(0x3 << 4)
#define MEMC_CR_loROM_speed_450    	(0x0 << 4) /* 450ns default	*/
#define MEMC_CR_loROM_speed_325    	(0x1 << 4) /* 325ns */
#define MEMC_CR_loROM_speed_200		(0x2 << 4) /* 200ns */
#define MEMC_CR_loROM_speed_undef	(0x3 << 4) /* undefined		*/

#define MEMC_CR_hiROM_speed_mask	(0x3 << 6)
#define MEMC_CR_hiROM_speed_450    	(0x0 << 6) /* 450ns default	*/
#define MEMC_CR_hiROM_speed_325    	(0x1 << 6) /* 325ns		*/
#define MEMC_CR_hiROM_speed_200    	(0x2 << 6) /* 200ns		*/
#define MEMC_CR_hiROM_speed_undef   	(0x3 << 6) /* undefined		*/

#define MEMC_CR_RAM_refresh_mask	(0x3 << 8)
#define MEMC_CR_RAM_refresh_off1   	(0x0 << 8) /* no refresh	*/
#define MEMC_CR_RAM_refresh_VFLY   	(0x1 << 8) /* video flyback	*/
#define MEMC_CR_RAM_refresh_off2   	(0x2 << 8) /* no refresh	*/
#define MEMC_CR_RAM_refresh_cont   	(0x3 << 8) /* continuous	*/

#define MEMC_CR_video_DMA_enable	(0x1 << 10)
#define MEMC_CR_sound_DMA_enable	(0x1 << 11)
#define MEMC_CR_OSmode_enable		(0x1 << 12)

/************************************************************************/
/* At the moment this header does not describe the page mapping system	*/
/************************************************************************/

/*----------------------------------------------------------------------*/
/*-- VIDC description --------------------------------------------------*/

#define VIDC_lcol0   (0 + (0 * 4))          /* display logical colours 0     */
#define VIDC_lcol1   (0 + (1 * 4))          /* display logical colours 1     */
#define VIDC_lcol2   (0 + (2 * 4))          /* display logical colours 2     */
#define VIDC_lcol3   (0 + (3 * 4))          /* display logical colours 3     */
#define VIDC_lcol4   (0 + (4 * 4))          /* display logical colours 4     */
#define VIDC_lcol5   (0 + (5 * 4))          /* display logical colours 5     */
#define VIDC_lcol6   (0 + (6 * 4))          /* display logical colours 6     */
#define VIDC_lcol7   (0 + (7 * 4))          /* display logical colours 7     */
#define VIDC_lcol8   (0 + (8 * 4))          /* display logical colours 8     */
#define VIDC_lcol9   (0 + (9 * 4))          /* display logical colours 9     */
#define VIDC_lcol10  (0 + (10 * 4))         /* display logical colours 10    */
#define VIDC_lcol11  (0 + (11 * 4))         /* display logical colours 11    */
#define VIDC_lcol12  (0 + (12 * 4))         /* display logical colours 12    */
#define VIDC_lcol13  (0 + (13 * 4))         /* display logical colours 13    */
#define VIDC_lcol14  (0 + (14 * 4))         /* display logical colours 14    */
#define VIDC_lcol15  (0 + (15 * 4))         /* display logical colours 15    */
#define VIDC_bcr     (0x40)                 /* border colour                 */
#define VIDC_clcr1   (0x44 + (1 - 1) * 4)   /* cursor logical colours 1      */
#define VIDC_clcr2   (0x44 + (2 - 1) * 4)   /* cursor logical colours 2      */
#define VIDC_clcr3   (0x44 + (3 - 1) * 4)   /* cursor logical colours 3      */
                                            /* 50-5C RESERVED - DO NOT TOUCH */
#define VIDC_sir7    (0x60 + ((7+1)&7)*4)   /* stereo sound positions 7      */
#define VIDC_sir0    (0x60 + ((0+1)&7)*4)   /* stereo sound positions 0      */
#define VIDC_sir1    (0x60 + ((1+1)&7)*4)   /* stereo sound positions 1      */
#define VIDC_sir2    (0x60 + ((2+1)&7)*4)   /* stereo sound positions 2      */
#define VIDC_sir3    (0x60 + ((3+1)&7)*4)   /* stereo sound positions 3      */
#define VIDC_sir4    (0x60 + ((4+1)&7)*4)   /* stereo sound positions 4      */
#define VIDC_sir5    (0x60 + ((5+1)&7)*4)   /* stereo sound positions 5      */
#define VIDC_sir6    (0x60 + ((6+1)&7)*4)   /* stereo sound positions 6      */
#define VIDC_hcr     (0x80)                 /* horizontal cycle              */
#define VIDC_hswr    (0x84)                 /* horizontal sync width         */
#define VIDC_hbsr    (0x88)                 /* horizontal border start       */
#define VIDC_hdsr    (0x8C)                 /* horizontal display start      */
#define VIDC_hder    (0x90)                 /* horizontal display end        */
#define VIDC_hber    (0x94)                 /* horizontal border end         */
#define VIDC_hcsr    (0x98)                 /* horizontal cursor start       */
#define VIDC_hir     (0x9C)                 /* horizontal interlace          */
#define VIDC_vcr     (0xA0)                 /* vertical cycle                */
#define VIDC_vswr    (0xA4)                 /* vertical sync width           */
#define VIDC_vbsr    (0xA8)                 /* vertical border start         */
#define VIDC_vdsr    (0xAC)                 /* vertical display start        */
#define VIDC_vder    (0xB0)                 /* vertical display end          */
#define VIDC_vber    (0xB4)                 /* vertical border end           */
#define VIDC_vcsr    (0xB8)                 /* vertical cursor start         */
#define VIDC_vcer    (0xBC)                 /* vertical cursor end           */
#define VIDC_sfr     (0xC0)                 /* sound frequency               */
                                            /* C4-DC RESERVED - DO NOT TOUCH */
#define VIDC_cr      (0xE0)                 /* control register              */
                                            /* E4-FC RESERVED - DO NOT TOUCH */

/* values for the Stereo Image position registers */
#define SIR_left_100	(1)
#define SIR_left_83	(2)
#define SIR_left_67	(3)
#define SIR_centre	(4)
#define SIR_right_67	(5)
#define SIR_right_83	(6)
#define SIR_tight_100	(7)

#define VIDC_reg_shift		(24)	/* register position in address      */
					/* colour data value in bits 0..12   */
                                	/* stereo image data in bits 0..2    */
#define VIDC_display_shift	(14)	/* display data value in bits 14..23 */
#define VIDC_hcsr_shift		(13)	/* normal HCSR data in bits 13..23   */
#define VIDC_hires_hcsr_shift	(11)	/* hires HCSR data in bits 11.23     */
					/* sound freq. data in bits 0..7     */

/* VIDC CR (Control Register) has following format:	*/
/* bit    7: composite sync bit      0 => off, 1 => on	*/
#define VIDC_control_csync_shift	(7)
#define VIDC_control_csync_off		(0)
#define VIDC_control_csync_on		(1)
/* bit    6: interlaced sync bit     0 => off, 1 => on	*/
#define VIDC_control_isync_shift	(6)
#define VIDC_control_isync_off		(0)
#define VIDC_control_isync_on		(1)
/* bits 4-5: DMA request timing: (required setting determined by RAM speed/video data rate etc) */
#define VIDC_control_dmareq_shift	(4)
#define VIDC_control_dmareq_04		(0)	/* end of words 0 and 4 */
#define VIDC_control_dmareq_15		(1)	/* end of words 1 and 5 */
#define VIDC_control_dmareq_26		(2)	/* end of words 2 and 6 */
#define VIDC_control_dmareq_37		(3)	/* end of words 3 and 7 */
/* bits 2-3: Bits per Pixel: */
#define VIDC_control_bpp_shift		(2)
#define VIDC_control_bpp_1		(0)	/* 1bit/pixel  */
#define VIDC_control_bpp_2		(1)	/* 2bits/pixel */
#define VIDC_control_bpp_4		(2)	/* 4bits/pixel */
#define VIDC_control_bpp_8		(3)	/* 8bits/pixel */
/* bits 0-1: Pixel Rate: */
#define VIDC_control_pixelrate_shift	(0)
#define VIDC_control_pixelrate_8	(0)	/* 8MHz  */
#define VIDC_control_pixelrate_12	(1)	/* 12MHz */
#define VIDC_control_pixelrate_16	(2)	/* 16MHz */
#define VIDC_control_pixelrate_24	(3)	/* 24MHz */

/*----------------------------------------------------------------------*/
/*-- IOC description ---------------------------------------------------*/

typedef struct {
                ubyte status ;
			ubyte pad0a ;
			ubyte pad0b ;
			ubyte pad0c ;
		ubyte request ;
			ubyte pad1a ;
			ubyte pad1b ;
			ubyte pad1c ;
		ubyte mask ;
			ubyte pad2a ;
			ubyte pad2b ;
			ubyte pad2c ;
			ubyte pad3a ;
			ubyte pad3b ;
			ubyte pad3c ;
			ubyte pad3d ;
               } intr_block ;

typedef struct {
                ubyte count_lo ; /* (R) = latch_lo (W) */
			ubyte pad0a ;
			ubyte pad0b ;
			ubyte pad0c ;
		ubyte count_hi ; /* (R) = latch_hi (W) */
			ubyte pad1a ;
			ubyte pad1b ;
			ubyte pad1c ;
		ubyte go_cmd ; /* (W) */
			ubyte pad2a ;
			ubyte pad2b ;
			ubyte pad2c ;
		ubyte latch_cmd ; /* (W) */
			ubyte pad3a ;
			ubyte pad3b ;
			ubyte pad3c ;
               } timer_block ;

/* #define latch_lo count_lo */
/* #define latch_hi count_hi */

typedef struct {
                ubyte       control ;		/* 00..03 read/write (CAREFUL!) */
				ubyte pad0a ;
				ubyte pad0b ;
				ubyte pad0c ;
		ubyte       kart_data ;		/* 04..07 read:RX, write:TX */
				ubyte pad1a ;
				ubyte pad1b ;
				ubyte pad1c ;
		word        pad2 ;              /* 08..0B */
		word        pad3 ;              /* 0C..0F */
		intr_block  irq_A ;             /* 10..1F */
		intr_block  irq_B ;             /* 20..2F */
		intr_block  fiq ;               /* 30..3F */
		timer_block timer_0 ;           /* 40..4F */
		timer_block timer_1 ;           /* 50..5F */
		timer_block timer_baud ;        /* 60..6F */
		timer_block timer_kart ;        /* 70..7F */
               } ioc_block ;

/* #define irq_clear	request	*/		/* "irq_A.request" : write only */

/* Control port register bits */
#define IOC_CON_I2C_DATA     (1<<0)	/* I2C Bus data  line - R/W */
#define IOC_CON_I2C_CLOCK    (1<<1)	/* I2C Bus clock line - R/W */
/*
 * The sound mute line is defined on all machines, but only performs
 * its expected function on the A500 and A440.  On A680 it must be 
 * left as a 1.
 */
#define IOC_CON_SOUND_MUTE   (1<<5)	/* Sound Mute (A500/A440) - R/W */
/*
 * The set of bits which must be 1 when writing to the control port,
 * on any machine.
 */
#define IOC_CON_WRITE_SET   (1<<7 | 1<<6 | 1<<4 | 1<<3 | 1<<2)


/* The following bits are defined for A4x0 machines */
#define IOC_CON4_DISC_READY  (1<<2)	/* floppy drive ready - RO */
#define IOC_CON4_AUX_C4	     (1<<4)	/* aux I/O line (unused) R/W */

/* The following 3 bits are present only on A500's */
#define IOC_CON5_RTC_MINUTES (1<<2)	/* Real Time Clock minutes - RO */
#define IOC_CON5_RTC_SECONDS (1<<3)	/* Real Time Clock seconds - RO */
#define IOC_CON5_DISC_CHANGE (1<<4)	/* Floppy disc changed - RO */

/* The following bit is defined on A680. */
#define IOC_CON6_DISC_CHANGE (1<<2)	/* floppy disc changed - RO */

/* Interrupt control register bits */

/* IRQ A block - mostly latched events, cleared via IRQ clear reg */
#define  IRQA_PBSY   (1 << 0)   /* Printer BuSY */
#define  IRQA_RII    (1 << 1)   /* Serial line RInging Indication */
#define  IRQA_PACK   (1 << 2)   /* Printer ACKnowledge event */
#define  IRQA_VFLY   (1 << 3)   /* Vertical FLYback event */
#define  IRQA_POR    (1 << 4)   /* Power On Reset */
#define  IRQA_TM0    (1 << 5)   /* Timer 0 expiry */
#define  IRQA_TM1    (1 << 6)   /* Timer 1 expiry */
#define  IRQA_FORCE  (1 << 7)   /* Force IRQ int bit (permanently 1) */

/* IRQ B block */
#define  IRQB_XFIQ   (1 << 0)   /* XCB FIQ(!) bit - mask OFF */
#define  IRQB_SND    (1 << 1)   /* Sound buffer switch event */
#define  IRQB_SLCI   (1 << 2)   /* Serial Line Controller Int */
#define  IRQB_WINC   (1 << 3)   /* Winchester Controller int */
#define  IRQB_WIND   (1 << 4)   /* Winchester Data int */
#define  IRQB_WINCD  (1 << 3)	/* Combined Controller/Data bit (Archimedes) */
#define  IRQB_FDDC   (1 << 4)	/* Floppy Disc Disc Changed (Archimedes) */
#define	 IRQB_FDINT  (1 << 4)   /* Floppy disc intr (A680) */
#define  IRQB_XCB    (1 << 5)   /* Expansion card common IRQ */
#define  IRQB_KSTX   (1 << 6)   /* KART transmitter done */
#define  IRQB_KSRX   (1 << 7)   /* KART receiver data ready */

/* FIQ block */
#define  FIQ_FDDR    (1 << 0)   /* Floppy Disc Data Request */
#define  FIQ_FDIR    (1 << 1)   /* Floppy Disc Interrupt Request */
#define  FIQ_ECONET  (1 << 2)   /* ECOnet interrupt */
/* 3..5 not used */
#define  FIQ_XCB     (1 << 6)   /* XCB card FIQ */
#define  FIQ_FORCE   (1 << 7)   /* Force FIQ int (permanently 1) */

/* Delay required after SRX set in IRQ B status before read KART data reg,
 * with KART serial line on maximum speed.
 */
#define  IOC_KART_BUG_DELAY  16        /* in microseconds: 1/2 bit time */


/*----------------------------------------------------------------------*/
/*-- Podule interface --------------------------------------------------*/

typedef enum {
              irq = 0,		/* 1bit  */
	      not_present = 1,	/* 1bit  */
	      fiq = 2,		/* 1bit  */
	      id_low = 3        /* 5bits */
             } idflagbits ;
#define idlow_mask	(0xF8)	/* id_low above */

typedef struct {
                ubyte b ; ubyte pad[3] ;
               } poddata ;

typedef struct {
                word    idflags ;
		ubyte   flags ; ubyte pad0[3] ;
		ubyte   unused ; ubyte pad1[3] ;
		ubyte   product_lo ; ubyte pad2[3] ;
		ubyte   product_hi ; ubyte pad3[3] ;
		ubyte   company_lo ; ubyte pad4[3] ;
		ubyte   company_hi ; ubyte pad5[3] ;
		ubyte   country ; ubyte pad6[3] ;
		poddata data[1016] ; /* 1024 - 8 */
	       } PoduleID ;

/* Podule Country codes */
#define POD_COUNTRY_UK	0
#define POD_COUNTRY_ITALY	4
#define POD_COUNTRY_SPAIN	5
#define POD_COUNTRY_FRANCE	6
#define POD_COUNTRY_GERMANY	7
#define POD_COUNTRY_PORTUGAL	8
#define POD_COUNTRY_GREECE	10
#define POD_COUNTRY_SWEDEN	11
#define POD_COUNTRY_FINLAND	12
#define POD_COUNTRY_DENMARK	14
#define POD_COUNTRY_NORWAY	15
#define POD_COUNTRY_ICELAND	16
#define POD_COUNTRY_CANADA	17
#define POD_COUNTRY_TURKEY	20

/* Podule Manufacturing Company codes */

#define POD_COMPANY_ACORNUK			0
#define POD_COMPANY_ACORNUSA			1
#define POD_COMPANY_OLIVETTI			2
#define POD_COMPANY_WATFORD			3
#define POD_COMPANY_COMPUTERCONCEPTS		4
#define POD_COMPANY_INTELIGENTINTERFACES	5
#define POD_COMPANY_CAMAN			6
#define POD_COMPANY_ARMADILLO			7
#define POD_COMPANY_SOFTOPTION			8
#define POD_COMPANY_WILDVISION			9
#define POD_COMPANY_ANGLOCOMPUTERS		10
#define POD_COMPANY_RESOURCE			11

/* Podule product type codes */
#define POD_PRODUCT_HOSTTUBE			0 /* To a BBC */
#define POD_PRODUCT_PARASITETUBE		1 /* To a second processor */
#define POD_PRODUCT_SCSI			2
#define POD_PRODUCT_ETHERNET			3
#define POD_PRODUCT_IBMDISC			4
#define POD_PRODUCT_RAMROM			5
#define POD_PRODUCT_BBCIO			6
#define POD_PRODUCT_MODEM			7
#define POD_PRODUCT_TELETEXT			8
#define POD_PRODUCT_CDROM			9
#define POD_PRODUCT_IEEE488			10
#define POD_PRODUCT_WINCHESTER			11
#define POD_PRODUCT_ESDI			12
#define POD_PRODUCT_SMD				13
#define POD_PRODUCT_LASERPRINTER		14
#define POD_PRODUCT_SCANNER			15
#define POD_PRODUCT_FASTRING			16
#define POD_PRODUCT_VMEBUS			17
#define POD_PRODUCT_PROMPROGRAMMER		18
#define POD_PRODUCT_MIDI			19
#define POD_PRODUCT_MONOVPU			20
#define POD_PRODUCT_FRAMEGRABBER		21
#define POD_PRODUCT_SOUNDSAMPLER		22
#define POD_PRODUCT_VIDEODIGITISER		23
#define POD_PRODUCT_GENLOCK			24
#define POD_PRODUCT_CODECSAMPLER		25
#define POD_PRODUCT_IMAGEANALYSER		26
#define POD_PRODUCT_ANALOGUEINPUT		27
#define POD_PRODUCT_CDSOUNDSAMPLER		28
#define POD_PRODUCT_6MIPSSIGPROC		29
#define POD_PRODUCT_12MIPSSIGPROC		30
#define POD_PRODUCT_33MIPSSIGPROC		31
#define POD_PRODUCT_TOUCHSCREEN			32
#define POD_PRODUCT_TRANSPUTERLINK		33

/* Hardware addressing (via IOC) */
#define PODULE_SPEED_SLOW	0
#define PODULE_SPEED_MEDIUM	1
#define PODULE_SPEED_FAST	2
#define PODULE_SPEED_SYNC	3		/* slowest of all...(!) */

/* Podule manager interface details */
#define PODULE_SLOTS	4

/*----------------------------------------------------------------------*/
/*-- Standard Archimedes CMOS memory allocations -----------------------*/

#define CMOS_RAM_SIZE		(256-16)

#define CMOS_RAM_CLOCKINFO	0x80
#define CMOS_RAM_YEAR		0x80
#define CMOS_RAM_CENTURY	0x81
#define CMOS_RAM_MONTH		0x82
#define CMOS_RAM_LEAPFLAG	0x83

#define CMOS_RAM_VDU_INFO	0x85
#define		 VDU_CSYNC_FLAG		(1 << 0)
#define		 VDU_TYPE__OFFSET	2	/* bits 2,3 in byte */
#define		 VDU_TYPE__BITS		4
#define		 VDU_TYPE__COUNT	(1 << VDU_TYPE__BITS)
#define		 VDU_TYPE__MASK		((1 << VDU_TYPE__BITS) - 1)
#define		 VDU_TYPE_LOWRES	0	/* PAL low res TV fmt only */
#define		 VDU_TYPE_MULTISCAN	1	/* low & medium res */
#define		 VDU_TYPE_HIGHRES	2	/* high res (mono) only */
#define		 VDU_TYPE_VGA		3	/* 640x480 fixed med res */

#define CMOS_UNIX_AREA_BASE	0x50
#define CMOS_UNIX_AREA_SIZE	0x20

#define CMOS_RAM_BASE_VTE	CMOS_UNIX_AREA_BASE + 0
#define CMOS_RAM_VTE_BYTES	5		/* bytes/emulator */
#define CMOS_RAM_SIZE_VTE	(2+(1+4)*CMOS_RAM_VTE_BYTES) /* console+4*vterm */
#define CMOS_RAM_BASE_ECONET	0x6E
#define CMOS_RAM_SIZE_ECONET	2
#define	CMOS_RAM_ECONET_SMALL_BUFFERS	0x6E
#define	CMOS_RAM_ECONET_LARGE_BUFFERS	0x6F

#define CMOS_CHECKSUM           0xEF /* Simple additive checksum of all the bytes 00..EE */

#define CMOS_BOOT_FLAGS		0xE0 /* Bootstrap flags */
#define CMOS_BOOT_AUTOBOOT	0x80 /* Autoboot bit in bootstrap flags */

/*----------------------------------------------------------------------*/
/*> EOF hardarc.h <*/
