******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 boot_oth.c boot_oth.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o boot_oth.if boot_oth.asm boot_oth.tmp 
	.version	40
FP	.set		AR3
	.file	"boot_oth.c"
	.file	"hydra.h"

	.stag	.fake0,96
	.member	_baud,0,15,8,32
	.member	_parity,32,4,8,32
	.member	_bits,64,4,8,32
	.eos
	.sym	_UART_config,0,8,13,96,.fake0

	.stag	.fake1,512
	.member	_uartA,0,8,8,96,.fake0
	.member	_uartB,96,8,8,96,.fake0
	.member	_dram_size,192,15,8,32
	.member	_cpu_clock,224,15,8,32
	.member	_checksum,256,15,8,32
	.member	_sram1_size,288,15,8,32
	.member	_sram2_size,320,15,8,32
	.member	_sram3_size,352,15,8,32
	.member	_sram4_size,384,15,8,32
	.member	_l_dram_base,416,15,8,32
	.member	_l_jtag_base,448,15,8,32
	.member	_daughter,480,15,8,32
	.eos
	.sym	_hydra_conf,0,8,13,512,.fake1

	.stag	.fake2,128
	.member	_FailAddress,0,31,8,32
	.member	_TestFailed,32,4,8,32
	.member	_Written,64,15,8,32
	.member	_Read,96,15,8,32
	.eos
	.sym	_MemTestStruct,0,8,13,128,.fake2

	.stag	.fake3,64
	.member	_header,0,15,8,32
	.member	_data,32,15,8,32
	.eos
	.sym	_CommMessage,0,8,13,64,.fake3

	.stag	.fake4,1504
	.member	_DSP_ir0,0,15,8,32
	.member	_DSP_ir1,32,15,8,32
	.member	_DSP_ir2,64,15,8,32
	.member	_DSP_ir3,96,15,8,32
	.member	_DSP_ir4,128,15,8,32
	.member	_DSP_ir5,160,15,8,32
	.member	_DSP_ir6,192,15,8,32
	.member	_DSP_ir7,224,15,8,32
	.member	_DSP_ir8,256,15,8,32
	.member	_DSP_ir9,288,15,8,32
	.member	_DSP_ir10,320,15,8,32
	.member	_DSP_ir11,352,15,8,32
	.member	_DSP_fr0,384,15,8,32
	.member	_DSP_fr1,416,15,8,32
	.member	_DSP_fr2,448,15,8,32
	.member	_DSP_fr3,480,15,8,32
	.member	_DSP_fr4,512,15,8,32
	.member	_DSP_fr5,544,15,8,32
	.member	_DSP_fr6,576,15,8,32
	.member	_DSP_fr7,608,15,8,32
	.member	_DSP_fr8,640,15,8,32
	.member	_DSP_fr9,672,15,8,32
	.member	_DSP_fr10,704,15,8,32
	.member	_DSP_fr11,736,15,8,32
	.member	_DSP_ar0,768,15,8,32
	.member	_DSP_ar1,800,15,8,32
	.member	_DSP_ar2,832,15,8,32
	.member	_DSP_ar3,864,15,8,32
	.member	_DSP_ar4,896,15,8,32
	.member	_DSP_ar5,928,15,8,32
	.member	_DSP_ar6,960,15,8,32
	.member	_DSP_ar7,992,15,8,32
	.member	_DSP_DP,1024,15,8,32
	.member	_DSP_IR0,1056,15,8,32
	.member	_DSP_IR1,1088,15,8,32
	.member	_DSP_BK,1120,15,8,32
	.member	_DSP_SP,1152,15,8,32
	.member	_DSP_ST,1184,15,8,32
	.member	_DSP_DIE,1216,15,8,32
	.member	_DSP_IIE,1248,15,8,32
	.member	_DSP_IIF,1280,15,8,32
	.member	_DSP_RS,1312,15,8,32
	.member	_DSP_RE,1344,15,8,32
	.member	_DSP_RC,1376,15,8,32
	.member	_DSP_IVTP,1408,15,8,32
	.member	_DSP_TVTP,1440,15,8,32
	.member	_ret_add,1472,15,8,32
	.eos
	.sym	_reg_set,0,8,13,1504,.fake4
	.globl	_setup
	.globl	_init
	.globl	_bus_error_handler
	.globl	_c_int01
	.globl	_monitor
	.globl	_compare
	.globl	_dump
	.globl	_fill
	.globl	_enter
	.globl	_copy
	.globl	_search
	.globl	_help
	.globl	_ReadEEPROM
	.globl	_WriteEEPROM
	.globl	_c40_printf
	.globl	_putstr
	.globl	_c40_putchar
	.globl	_ftoa
	.globl	_send_host
	.globl	_comm_sen
	.globl	_comm_rec
	.globl	_com_init
	.globl	_o_crdy
	.globl	_i_crdy
	.globl	_xtoa
	.globl	_atox
	.globl	_atod
	.globl	_key_handler
	.globl	_c40_getchar
	.globl	_iswhite
	.globl	_led
	.globl	_configure_uart
	.globl	_get_baud
	.globl	_get_parity
	.globl	_parity_enable
	.globl	_parity_type
	.globl	_get_bits
	.globl	_get_addr
	.globl	_get_clock
	.globl	_get_sram
	.globl	_get_daughter
	.globl	_get_dram
	.globl	_read_config
	.globl	_display_conf
	.globl	_write_config
	.globl	_update_chksum
	.globl	_crcupdate
	.globl	_x_rcvcrc
	.globl	_x_sndcrc
	.globl	_crchware
	.globl	_mk_crctbl
	.globl	_zero_regs
	.globl	_resume_mon
	.globl	_set_brk
	.globl	_del_brk
	.globl	_list_brks
	.globl	_reg_dump
	.globl	_print_reg
	.globl	_step
	.globl	_go
	.globl	_run
	.globl	_writeVIC
	.globl	_readVIC
	.globl	_writeVAC
	.globl	_readVAC
	.globl	_readVACEPROM
	.globl	_test
	.globl	_reset_others
	.globl	_CommTest
	.globl	_BootOthers
	.file	"boot_oth.c"

	.sym	_BootOthers,_BootOthers,36,2,0
	.globl	_BootOthers

	.func	18
;>>>> 	int BootOthers( hydra_conf config, int booted[] )
******************************************************
* FUNCTION DEF : _BootOthers
******************************************************
_BootOthers:
	PUSH	FP
	LDI	SP,FP
	ADDI	5,SP
	.sym	_config,-17,8,9,512,.fake1
	.sym	_booted,-18,20,9,32
	.sym	_blk_cnt,1,4,1,32
	.sym	_i,2,4,1,32
	.sym	_done,3,4,1,32
	.sym	_eprom_ptr,4,15,1,32
	.sym	_val,5,15,1,32
	.line	2
;>>>> 		int blk_cnt, i, done;
;>>>> 		unsigned long eprom_ptr, val;
	.line	7
;>>>> 		eprom_ptr = (unsigned long)(1); /* Skip boot width */		
	STIK	1,*+FP(4)
	.line	10
;>>>> 		for( i=0 ; i < 2 ; i++ )
	STIK	0,*+FP(2)
	CMPI	2,*+FP(2)
	BGE	L2
L1:
	.line	12
;>>>> 			if( !comm_sen(DSP_2, val=ReadEprom(eprom_ptr++), NumTries) )
	LDI	0,R0
	PUSH	R0
	ADDI	1,*+FP(4),R1
	STI	R1,*+FP(4)
	SUBI	1,R1
	PUSH	R1
	CALL	_ReadEprom
	SUBI	1,SP
	STI	R0,*+FP(5)
	PUSH	R0
	LDI	0,R1
	PUSH	R1
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L3
	.line	13
;>>>> 				return( 2 );
	LDI	2,R0
	B	EPI0_1
L3:
	.line	14
;>>>> 			if( config.daughter )
	LDI	*-FP(2),R0
	BZ	L4
	.line	16
;>>>> 				if( !comm_sen( DSP_3, val, NumTries ) )
	LDI	0,R1
	PUSH	R1
	LDI	*+FP(5),R2
	PUSH	R2
	LDI	1,R3
	PUSH	R3
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L5
	.line	17
;>>>> 					return( 3 ); 
	LDI	3,R0
	B	EPI0_1
L5:
	.line	18
;>>>> 				if( !comm_sen( DSP_4, val, NumTries ) )
	LDI	0,R0
	PUSH	R0
	LDI	*+FP(5),R1
	PUSH	R1
	LDI	2,R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L4
	.line	19
;>>>> 					return( 4 );
	LDI	4,R0
	B	EPI0_1
L4:
	.line	10
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	CMPI	2,R0
	BLT	L1
L2:
	.line	24
;>>>> 		for( done=FALSE ; !done ; )
	STIK	0,*+FP(3)
	LDI	*+FP(3),R0
	BNZ	L8
L7:
	.line	27
;>>>> 			if( !comm_sen(DSP_2, blk_cnt=ReadEprom(eprom_ptr++), NumTries) )
	LDI	0,R0
	PUSH	R0
	ADDI	1,*+FP(4),R1
	STI	R1,*+FP(4)
	SUBI	1,R1
	PUSH	R1
	CALL	_ReadEprom
	SUBI	1,SP
	STI	R0,*+FP(1)
	PUSH	R0
	LDI	0,R1
	PUSH	R1
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L9
	.line	28
;>>>> 				return( 2 );
	LDI	2,R0
	B	EPI0_1
L9:
	.line	29
;>>>> 			if( blk_cnt == 0 )
	LDI	*+FP(1),R0
	BNZ	L10
	.line	31
;>>>> 				done = TRUE;
	STIK	1,*+FP(3)
	.line	32
;>>>> 				continue;
	B	L11
L10:
	.line	36
;>>>> 			if( !comm_sen(DSP_2, val=ReadEprom(eprom_ptr++), NumTries) )
	LDI	0,R1
	PUSH	R1
	ADDI	1,*+FP(4),R2
	STI	R2,*+FP(4)
	SUBI	1,R2
	PUSH	R2
	CALL	_ReadEprom
	SUBI	1,SP
	STI	R0,*+FP(5)
	PUSH	R0
	LDI	0,R1
	PUSH	R1
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L12
	.line	37
;>>>> 				return( 2 );
	LDI	2,R0
	B	EPI0_1
L12:
	.line	38
;>>>> 			if( config.daughter )
	LDI	*-FP(2),R0
	BZ	L13
	.line	40
;>>>> 				if( !comm_sen( DSP_3, val, NumTries ) )
	LDI	0,R1
	PUSH	R1
	LDI	*+FP(5),R2
	PUSH	R2
	LDI	1,R3
	PUSH	R3
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L14
	.line	41
;>>>> 					return( 3 ); 
	LDI	3,R0
	B	EPI0_1
L14:
	.line	42
;>>>> 				if( !comm_sen( DSP_4, val, NumTries ) )
	LDI	0,R0
	PUSH	R0
	LDI	*+FP(5),R1
	PUSH	R1
	LDI	2,R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L13
	.line	43
;>>>> 					return( 4 );
	LDI	4,R0
	B	EPI0_1
L13:
	.line	47
;>>>> 			while( blk_cnt-- )
	SUBI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	ADDI	1,R0
	BZ	L11
L16:
	.line	49
;>>>> 				if( !comm_sen(DSP_2, val=ReadEprom(eprom_ptr++), NumTries) )
	LDI	0,R0
	PUSH	R0
	ADDI	1,*+FP(4),R1
	STI	R1,*+FP(4)
	SUBI	1,R1
	PUSH	R1
	CALL	_ReadEprom
	SUBI	1,SP
	STI	R0,*+FP(5)
	PUSH	R0
	LDI	0,R1
	PUSH	R1
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L18
	.line	50
;>>>> 					return( 2 );
	LDI	2,R0
	B	EPI0_1
L18:
	.line	51
;>>>> 				if( config.daughter )
	LDI	*-FP(2),R0
	BZ	L19
	.line	53
;>>>> 					if( !comm_sen( DSP_3, val, NumTries ) )
	LDI	0,R1
	PUSH	R1
	LDI	*+FP(5),R2
	PUSH	R2
	LDI	1,R3
	PUSH	R3
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L20
	.line	54
;>>>> 						return( 3 ); 
	LDI	3,R0
	B	EPI0_1
L20:
	.line	55
;>>>> 					if( !comm_sen( DSP_4, val, NumTries ) )
	LDI	0,R0
	PUSH	R0
	LDI	*+FP(5),R1
	PUSH	R1
	LDI	2,R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L19
	.line	56
;>>>> 						return( 4 );
	LDI	4,R0
	B	EPI0_1
L19:
	.line	47
	SUBI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	ADDI	1,R0
	BNZ	L16
L11:
	.line	24
	LDI	*+FP(3),R0
	BZ	L7
L8:
	.line	62
;>>>> 		for( i=0 ; i < 3 ; i++ )
	STIK	0,*+FP(2)
	CMPI	3,*+FP(2)
	BGE	L23
L22:
	.line	64
;>>>> 			if( !comm_sen(DSP_2, val=ReadEprom(eprom_ptr++), NumTries) )
	LDI	0,R0
	PUSH	R0
	ADDI	1,*+FP(4),R1
	STI	R1,*+FP(4)
	SUBI	1,R1
	PUSH	R1
	CALL	_ReadEprom
	SUBI	1,SP
	STI	R0,*+FP(5)
	PUSH	R0
	LDI	0,R1
	PUSH	R1
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L24
	.line	65
;>>>> 				return( 2 );
	LDI	2,R0
	B	EPI0_1
L24:
	.line	66
;>>>> 			if( config.daughter )
	LDI	*-FP(2),R0
	BZ	L25
	.line	68
;>>>> 				if( !comm_sen( DSP_3, val, NumTries ) )
	LDI	0,R1
	PUSH	R1
	LDI	*+FP(5),R2
	PUSH	R2
	LDI	1,R3
	PUSH	R3
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L26
	.line	69
;>>>> 					return( 3 ); 
	LDI	3,R0
	B	EPI0_1
L26:
	.line	70
;>>>> 				if( !comm_sen( DSP_4, val, NumTries ) )
	LDI	0,R0
	PUSH	R0
	LDI	*+FP(5),R1
	PUSH	R1
	LDI	2,R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L25
	.line	71
;>>>> 					return( 4 );
	LDI	4,R0
	B	EPI0_1
L25:
	.line	62
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	CMPI	3,R0
	BLT	L22
L23:
	.line	76
;>>>> 		if( !comm_sen( DSP_2, 2, NumTries ) )
	LDI	0,R0
	PUSH	R0
	LDI	2,R1
	PUSH	R1
	PUSH	R0
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L28
	.line	77
;>>>> 			return( 2 );
	LDI	2,R0
	B	EPI0_1
L28:
	.line	78
;>>>> 		if( !comm_rec( DSP_2, &val, NumTries ) )
	LDI	0,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	5,R1
	PUSH	R1
	PUSH	R0
	CALL	_comm_rec
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L29
	.line	79
;>>>> 			return( 2 );
	LDI	2,R0
	B	EPI0_1
L29:
	.line	80
;>>>> 		if( val != 2 )
	CMPI	2,*+FP(5)
	BZ	L30
	.line	81
;>>>> 			return( 2 );
	LDI	2,R0
	B	EPI0_1
L30:
	.line	82
;>>>> 		booted[0] = TRUE;
	LDA	*-FP(18),AR0
	STIK	1,*AR0
	.line	84
;>>>> 		if( config.daughter )
	LDI	*-FP(2),R0
	BZ	EPI0_1
	.line	86
;>>>> 			if( !comm_sen( DSP_3, 3, NumTries ) )
	LDI	0,R1
	PUSH	R1
	LDI	3,R2
	PUSH	R2
	LDI	1,R3
	PUSH	R3
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L32
	.line	87
;>>>> 				return( 3 ); 
	LDI	3,R0
	B	EPI0_1
L32:
	.line	88
;>>>> 			if( !comm_rec( DSP_3, &val, NumTries ) )
	LDI	0,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	5,R1
	PUSH	R1
	LDI	1,R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L33
	.line	89
;>>>> 				return( 3 );
	LDI	3,R0
	B	EPI0_1
L33:
	.line	90
;>>>> 			if( val != 3 )
	CMPI	3,*+FP(5)
	BZ	L34
	.line	91
;>>>> 				return( 3 );
	LDI	3,R0
	B	EPI0_1
L34:
	.line	92
;>>>> 			booted[1] = TRUE;
	LDA	*-FP(18),AR0
	STIK	1,*+AR0(1)
	.line	94
;>>>> 			if( !comm_sen( DSP_4, 4, NumTries ) )
	LDI	0,R0
	PUSH	R0
	LDI	4,R1
	PUSH	R1
	LDI	2,R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L35
	.line	95
;>>>> 				return( 4 );
	LDI	4,R0
	B	EPI0_1
L35:
	.line	96
;>>>> 			if( !comm_rec( DSP_4, &val, NumTries ) )
	LDI	0,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	5,R1
	PUSH	R1
	LDI	2,R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L36
	.line	97
;>>>> 				return( 4 );
	LDI	4,R0
	B	EPI0_1
L36:
	.line	98
;>>>> 			if( val != 4 )
	CMPI	4,*+FP(5)
	BZ	L37
	.line	99
;>>>> 				return( 4 );
	LDI	4,R0
	B	EPI0_1
L37:
	.line	100
;>>>> 			booted[2] = TRUE;
	LDA	*-FP(18),AR0
	STIK	1,*+AR0(2)
EPI0_1:
	.line	102
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	7,SP
	B	R1
	.endfunc	119,000000000H,5

	.sym	_ReadEprom,_ReadEprom,47,3,0

	.func	122
;>>>> 	static unsigned long ReadEprom( unsigned long WordAddr )
******************************************************
* FUNCTION DEF : _ReadEprom
******************************************************
_ReadEprom:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
	.sym	_WordAddr,-2,15,9,32
	.sym	_data,1,15,1,32
	.sym	_i,2,15,1,32
	.line	2
;>>>> 		unsigned long data, i;
	.line	5
;>>>> 		for( i=0, data=0, WordAddr*=4 ; i < 4 ; i++, WordAddr++ )
	STIK	0,*+FP(2)
	STIK	0,*+FP(1)
	LDI	*-FP(2),R0
	LSH	2,R0,R1
	STI	R1,*-FP(2)
	CMPI	4,*+FP(2)
	BHS	L39
L38:
	.line	6
;>>>> 			data |= (*(unsigned long *)(EPROM_ADDR + WordAddr ) & 0xFF)
;>>>> 			        << (i * 8);
	LDA	*-FP(2),AR0
	LDA	@CONST+0,IR0
	LDI	*+AR0(IR0),R0
	AND	0ffh,R0
	LSH	3,*+FP(2),R1
	LSH	R1,R0
	OR	*+FP(1),R0
	STI	R0,*+FP(1)
	.line	5
	ADDI	1,*+FP(2),R1
	STI	R1,*+FP(2)
	LDI	*-FP(2),R2
	ADDI	1,R2,R3
	STI	R3,*-FP(2)
	CMPI	4,R1
	BLO	L38
L39:
	.line	9
;>>>> 		return( data );
	LDI	*+FP(1),R0
EPI0_2:
	.line	10
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	4,SP
	B	R1
	.endfunc	131,000000000H,2
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,1
	.sect	".cinit"
	.word	1,CONST
	.word 	3198976          ;0
	.end
