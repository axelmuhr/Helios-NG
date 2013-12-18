PRAM_base	*	(&02000000)	

IO_base	*	(&03000000)	

IOC_base	*	(&03200000)	

VIDC_base	*	(&03400000)	

MEMC_base	*	(&03600000)	

CAM_base	*	(&03800000)    

loROM_base	*	(&03400000)	

hiROM_base	*	(&03800000)	

ROM_base	*	(hiROM_base)	

MEMC_vinit	*	(0)	

MEMC_vstart	*	(1)	

MEMC_vend	*	(2)	

MEMC_cinit	*	(3)	

MEMC_sstart	*	(4)	

MEMC_sendN	*	(5)	

MEMC_sptr	*	(6)	

MEMC_DAG_reg_shift	*	(17)		

MEMC_DAG_data_shift	*	(2)		

MEMC_DAG_data_mask	*	(&0001FFFC)	

MEMC_CR_pagesize_mask	*	(&00000003 :SHL: 2)	

MEMC_CR_pagesize_4KB	*	(&00000000 :SHL: 2)	

MEMC_CR_pagesize_8KB	*	(&00000001 :SHL: 2)	

MEMC_CR_pagesize_16KB	*	(&00000002 :SHL: 2)	

MEMC_CR_pagesize_32KB	*	(&00000003 :SHL: 2)	

MEMC_CR_loROM_speed_mask	*	(&00000003 :SHL: 4)

MEMC_CR_loROM_speed_450	*	(&00000000 :SHL: 4) 

MEMC_CR_loROM_speed_325	*	(&00000001 :SHL: 4) 

MEMC_CR_loROM_speed_200	*	(&00000002 :SHL: 4) 

MEMC_CR_loROM_speed_undef	*	(&00000003 :SHL: 4) 

MEMC_CR_hiROM_speed_mask	*	(&00000003 :SHL: 6)

MEMC_CR_hiROM_speed_450	*	(&00000000 :SHL: 6) 

MEMC_CR_hiROM_speed_325	*	(&00000001 :SHL: 6) 

MEMC_CR_hiROM_speed_200	*	(&00000002 :SHL: 6) 

MEMC_CR_hiROM_speed_undef	*	(&00000003 :SHL: 6) 

MEMC_CR_RAM_refresh_mask	*	(&00000003 :SHL: 8)

MEMC_CR_RAM_refresh_off1	*	(&00000000 :SHL: 8) 

MEMC_CR_RAM_refresh_VFLY	*	(&00000001 :SHL: 8) 

MEMC_CR_RAM_refresh_off2	*	(&00000002 :SHL: 8) 

MEMC_CR_RAM_refresh_cont	*	(&00000003 :SHL: 8) 

MEMC_CR_video_DMA_enable	*	(&00000001 :SHL: 10)

MEMC_CR_sound_DMA_enable	*	(&00000001 :SHL: 11)

MEMC_CR_OSmode_enable	*	(&00000001 :SHL: 12)

VIDC_lcol0	*	(0 + (0 * 4))          

VIDC_lcol1	*	(0 + (1 * 4))          

VIDC_lcol2	*	(0 + (2 * 4))          

VIDC_lcol3	*	(0 + (3 * 4))          

VIDC_lcol4	*	(0 + (4 * 4))          

VIDC_lcol5	*	(0 + (5 * 4))          

VIDC_lcol6	*	(0 + (6 * 4))          

VIDC_lcol7	*	(0 + (7 * 4))          

VIDC_lcol8	*	(0 + (8 * 4))          

VIDC_lcol9	*	(0 + (9 * 4))          

VIDC_lcol10	*	(0 + (10 * 4))         

VIDC_lcol11	*	(0 + (11 * 4))         

VIDC_lcol12	*	(0 + (12 * 4))         

VIDC_lcol13	*	(0 + (13 * 4))         

VIDC_lcol14	*	(0 + (14 * 4))         

VIDC_lcol15	*	(0 + (15 * 4))         

VIDC_bcr	*	(&00000040)                 

VIDC_clcr1	*	(&00000044 + (1 - 1) * 4)   

VIDC_clcr2	*	(&00000044 + (2 - 1) * 4)   

VIDC_clcr3	*	(&00000044 + (3 - 1) * 4)   

VIDC_sir7	*	(&00000060 + ((7+1):AND:7)*4)   

VIDC_sir0	*	(&00000060 + ((0+1):AND:7)*4)   

VIDC_sir1	*	(&00000060 + ((1+1):AND:7)*4)   

VIDC_sir2	*	(&00000060 + ((2+1):AND:7)*4)   

VIDC_sir3	*	(&00000060 + ((3+1):AND:7)*4)   

VIDC_sir4	*	(&00000060 + ((4+1):AND:7)*4)   

VIDC_sir5	*	(&00000060 + ((5+1):AND:7)*4)   

VIDC_sir6	*	(&00000060 + ((6+1):AND:7)*4)   

VIDC_hcr	*	(&00000080)                 

VIDC_hswr	*	(&00000084)                 

VIDC_hbsr	*	(&00000088)                 

VIDC_hdsr	*	(&0000008C)                 

VIDC_hder	*	(&00000090)                 

VIDC_hber	*	(&00000094)                 

VIDC_hcsr	*	(&00000098)                 

VIDC_hir	*	(&0000009C)                 

VIDC_vcr	*	(&000000A0)                 

VIDC_vswr	*	(&000000A4)                 

VIDC_vbsr	*	(&000000A8)                 

VIDC_vdsr	*	(&000000AC)                 

VIDC_vder	*	(&000000B0)                 

VIDC_vber	*	(&000000B4)                 

VIDC_vcsr	*	(&000000B8)                 

VIDC_vcer	*	(&000000BC)                 

VIDC_sfr	*	(&000000C0)                 

VIDC_cr	*	(&000000E0)                 

SIR_left_100	*	(1)

SIR_left_83	*	(2)

SIR_left_67	*	(3)

SIR_centre	*	(4)

SIR_right_67	*	(5)

SIR_right_83	*	(6)

SIR_tight_100	*	(7)

VIDC_reg_shift	*	(24)	

VIDC_display_shift	*	(14)	

VIDC_hcsr_shift	*	(13)	

VIDC_hires_hcsr_shift	*	(11)	

VIDC_control_csync_shift	*	(7)

VIDC_control_csync_off	*	(0)

VIDC_control_csync_on	*	(1)

VIDC_control_isync_shift	*	(6)

VIDC_control_isync_off	*	(0)

VIDC_control_isync_on	*	(1)

VIDC_control_dmareq_shift	*	(4)

VIDC_control_dmareq_04	*	(0)	

VIDC_control_dmareq_15	*	(1)	

VIDC_control_dmareq_26	*	(2)	

VIDC_control_dmareq_37	*	(3)	

VIDC_control_bpp_shift	*	(2)

VIDC_control_bpp_1	*	(0)	

VIDC_control_bpp_2	*	(1)	

VIDC_control_bpp_4	*	(2)	

VIDC_control_bpp_8	*	(3)	

VIDC_control_pixelrate_shift	*	(0)

VIDC_control_pixelrate_8	*	(0)	

VIDC_control_pixelrate_12	*	(1)	

VIDC_control_pixelrate_16	*	(2)	

VIDC_control_pixelrate_24	*	(3)	

	^	0
intr_block_status	#	1
intr_block_pad0a	#	1
intr_block_pad0b	#	1
intr_block_pad0c	#	1
intr_block_request	#	1
intr_block_pad1a	#	1
intr_block_pad1b	#	1
intr_block_pad1c	#	1
intr_block_mask	#	1
intr_block_pad2a	#	1
intr_block_pad2b	#	1
intr_block_pad2c	#	1
intr_block_pad3a	#	1
intr_block_pad3b	#	1
intr_block_pad3c	#	1
intr_block_pad3d	#	1
	[	(({VAR} :AND: 3) <> 0)
	#	(4 - ({VAR} :AND: 3))	; word align next entry
	]
intr_block_sizeof	#	0

	^	0
timer_block_count_lo	#	1
timer_block_pad0a	#	1
timer_block_pad0b	#	1
timer_block_pad0c	#	1
timer_block_count_hi	#	1
timer_block_pad1a	#	1
timer_block_pad1b	#	1
timer_block_pad1c	#	1
timer_block_go_cmd	#	1
timer_block_pad2a	#	1
timer_block_pad2b	#	1
timer_block_pad2c	#	1
timer_block_latch_cmd	#	1
timer_block_pad3a	#	1
timer_block_pad3b	#	1
timer_block_pad3c	#	1
	[	(({VAR} :AND: 3) <> 0)
	#	(4 - ({VAR} :AND: 3))	; word align next entry
	]
timer_block_sizeof	#	0

	^	0
ioc_block_control	#	1
ioc_block_pad0a	#	1
ioc_block_pad0b	#	1
ioc_block_pad0c	#	1
ioc_block_kart_data	#	1
ioc_block_pad1a	#	1
ioc_block_pad1b	#	1
ioc_block_pad1c	#	1
	[	(({VAR} :AND: 3) <> 0)
	#	(4 - ({VAR} :AND: 3))	; word align next entry
	]
ioc_block_pad2	#	4
ioc_block_pad3	#	4
ioc_block_irq_A	#	intr_block_sizeof
ioc_block_irq_B	#	intr_block_sizeof
ioc_block_fiq	#	intr_block_sizeof
ioc_block_timer_0	#	timer_block_sizeof
ioc_block_timer_1	#	timer_block_sizeof
ioc_block_timer_baud	#	timer_block_sizeof
ioc_block_timer_kart	#	timer_block_sizeof
	[	(({VAR} :AND: 3) <> 0)
	#	(4 - ({VAR} :AND: 3))	; word align next entry
	]
ioc_block_sizeof	#	0

IOC_CON_I2C_DATA	*	(1:SHL:0)	

IOC_CON_I2C_CLOCK	*	(1:SHL:1)	

IOC_CON_SOUND_MUTE	*	(1:SHL:5)	

IOC_CON_WRITE_SET	*	(1:SHL:7 :OR: 1:SHL:6 :OR: 1:SHL:4 :OR: 1:SHL:3 :OR: 1:SHL:2)

IOC_CON4_DISC_READY	*	(1:SHL:2)	

IOC_CON4_AUX_C4	*	(1:SHL:4)	

IOC_CON5_RTC_MINUTES	*	(1:SHL:2)	

IOC_CON5_RTC_SECONDS	*	(1:SHL:3)	

IOC_CON5_DISC_CHANGE	*	(1:SHL:4)	

IOC_CON6_DISC_CHANGE	*	(1:SHL:2)	

IRQA_PBSY	*	(1 :SHL: 0)   

IRQA_RII	*	(1 :SHL: 1)   

IRQA_PACK	*	(1 :SHL: 2)   

IRQA_VFLY	*	(1 :SHL: 3)   

IRQA_POR	*	(1 :SHL: 4)   

IRQA_TM0	*	(1 :SHL: 5)   

IRQA_TM1	*	(1 :SHL: 6)   

IRQA_FORCE	*	(1 :SHL: 7)   

IRQB_XFIQ	*	(1 :SHL: 0)   

IRQB_SND	*	(1 :SHL: 1)   

IRQB_SLCI	*	(1 :SHL: 2)   

IRQB_WINC	*	(1 :SHL: 3)   

IRQB_WIND	*	(1 :SHL: 4)   

IRQB_WINCD	*	(1 :SHL: 3)	

IRQB_FDDC	*	(1 :SHL: 4)	

IRQB_FDINT	*	(1 :SHL: 4)   

IRQB_XCB	*	(1 :SHL: 5)   

IRQB_KSTX	*	(1 :SHL: 6)   

IRQB_KSRX	*	(1 :SHL: 7)   

FIQ_FDDR	*	(1 :SHL: 0)   

FIQ_FDIR	*	(1 :SHL: 1)   

FIQ_ECONET	*	(1 :SHL: 2)   

FIQ_XCB	*	(1 :SHL: 6)   

FIQ_FORCE	*	(1 :SHL: 7)   

IOC_KART_BUG_DELAY	*	16        

irq	*	0
not_present	*	1
fiq	*	2
id_low	*	3

idlow_mask	*	(&000000F8)	

	^	0
poddata_b	#	1
poddata_pad	#	(3 * 1)
	[	(({VAR} :AND: 3) <> 0)
	#	(4 - ({VAR} :AND: 3))	; word align next entry
	]
poddata_sizeof	#	0

	^	0
PoduleID_idflags	#	4
PoduleID_flags	#	1
PoduleID_pad0	#	(3 * 1)
PoduleID_unused	#	1
PoduleID_pad1	#	(3 * 1)
PoduleID_product_lo	#	1
PoduleID_pad2	#	(3 * 1)
PoduleID_product_hi	#	1
PoduleID_pad3	#	(3 * 1)
PoduleID_company_lo	#	1
PoduleID_pad4	#	(3 * 1)
PoduleID_company_hi	#	1
PoduleID_pad5	#	(3 * 1)
PoduleID_country	#	1
PoduleID_pad6	#	(3 * 1)
	[	(({VAR} :AND: 3) <> 0)
	#	(4 - ({VAR} :AND: 3))	; word align next entry
	]
PoduleID_data	#	(1016 * poddata_sizeof)
	[	(({VAR} :AND: 3) <> 0)
	#	(4 - ({VAR} :AND: 3))	; word align next entry
	]
PoduleID_sizeof	#	0

POD_COUNTRY_UK	*	0

POD_COUNTRY_ITALY	*	4

POD_COUNTRY_SPAIN	*	5

POD_COUNTRY_FRANCE	*	6

POD_COUNTRY_GERMANY	*	7

POD_COUNTRY_PORTUGAL	*	8

POD_COUNTRY_GREECE	*	10

POD_COUNTRY_SWEDEN	*	11

POD_COUNTRY_FINLAND	*	12

POD_COUNTRY_DENMARK	*	14

POD_COUNTRY_NORWAY	*	15

POD_COUNTRY_ICELAND	*	16

POD_COUNTRY_CANADA	*	17

POD_COUNTRY_TURKEY	*	20

POD_COMPANY_ACORNUK	*	0

POD_COMPANY_ACORNUSA	*	1

POD_COMPANY_OLIVETTI	*	2

POD_COMPANY_WATFORD	*	3

POD_COMPANY_COMPUTERCONCEPTS	*	4

POD_COMPANY_INTELIGENTINTERFACES	*	5

POD_COMPANY_CAMAN	*	6

POD_COMPANY_ARMADILLO	*	7

POD_COMPANY_SOFTOPTION	*	8

POD_COMPANY_WILDVISION	*	9

POD_COMPANY_ANGLOCOMPUTERS	*	10

POD_COMPANY_RESOURCE	*	11

POD_PRODUCT_HOSTTUBE	*	0 

POD_PRODUCT_PARASITETUBE	*	1 

POD_PRODUCT_SCSI	*	2

POD_PRODUCT_ETHERNET	*	3

POD_PRODUCT_IBMDISC	*	4

POD_PRODUCT_RAMROM	*	5

POD_PRODUCT_BBCIO	*	6

POD_PRODUCT_MODEM	*	7

POD_PRODUCT_TELETEXT	*	8

POD_PRODUCT_CDROM	*	9

POD_PRODUCT_IEEE488	*	10

POD_PRODUCT_WINCHESTER	*	11

POD_PRODUCT_ESDI	*	12

POD_PRODUCT_SMD	*	13

POD_PRODUCT_LASERPRINTER	*	14

POD_PRODUCT_SCANNER	*	15

POD_PRODUCT_FASTRING	*	16

POD_PRODUCT_VMEBUS	*	17

POD_PRODUCT_PROMPROGRAMMER	*	18

POD_PRODUCT_MIDI	*	19

POD_PRODUCT_MONOVPU	*	20

POD_PRODUCT_FRAMEGRABBER	*	21

POD_PRODUCT_SOUNDSAMPLER	*	22

POD_PRODUCT_VIDEODIGITISER	*	23

POD_PRODUCT_GENLOCK	*	24

POD_PRODUCT_CODECSAMPLER	*	25

POD_PRODUCT_IMAGEANALYSER	*	26

POD_PRODUCT_ANALOGUEINPUT	*	27

POD_PRODUCT_CDSOUNDSAMPLER	*	28

POD_PRODUCT_6MIPSSIGPROC	*	29

POD_PRODUCT_12MIPSSIGPROC	*	30

POD_PRODUCT_33MIPSSIGPROC	*	31

POD_PRODUCT_TOUCHSCREEN	*	32

POD_PRODUCT_TRANSPUTERLINK	*	33

PODULE_SPEED_SLOW	*	0

PODULE_SPEED_MEDIUM	*	1

PODULE_SPEED_FAST	*	2

PODULE_SPEED_SYNC	*	3		

PODULE_SLOTS	*	4

CMOS_RAM_SIZE	*	(256-16)

CMOS_RAM_CLOCKINFO	*	&00000080

CMOS_RAM_YEAR	*	&00000080

CMOS_RAM_CENTURY	*	&00000081

CMOS_RAM_MONTH	*	&00000082

CMOS_RAM_LEAPFLAG	*	&00000083

CMOS_RAM_VDU_INFO	*	&00000085

VDU_CSYNC_FLAG	*	(1 :SHL: 0)

VDU_TYPE__OFFSET	*	2	

VDU_TYPE__BITS	*	4

VDU_TYPE__COUNT	*	(1 :SHL: VDU_TYPE__BITS)

VDU_TYPE__MASK	*	((1 :SHL: VDU_TYPE__BITS) - 1)

VDU_TYPE_LOWRES	*	0	

VDU_TYPE_MULTISCAN	*	1	

VDU_TYPE_HIGHRES	*	2	

VDU_TYPE_VGA	*	3	

CMOS_UNIX_AREA_BASE	*	&00000050

CMOS_UNIX_AREA_SIZE	*	&00000020

CMOS_RAM_BASE_VTE	*	CMOS_UNIX_AREA_BASE + 0

CMOS_RAM_VTE_BYTES	*	5		

CMOS_RAM_SIZE_VTE	*	(2+(1+4)*CMOS_RAM_VTE_BYTES) 

CMOS_RAM_BASE_ECONET	*	&0000006E

CMOS_RAM_SIZE_ECONET	*	2

CMOS_RAM_ECONET_SMALL_BUFFERS	*	&0000006E

CMOS_RAM_ECONET_LARGE_BUFFERS	*	&0000006F

CMOS_CHECKSUM	*	&000000EF 

CMOS_BOOT_FLAGS	*	&000000E0 

CMOS_BOOT_AUTOBOOT	*	&00000080 

	END
