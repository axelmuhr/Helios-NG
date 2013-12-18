******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 reg_dump.c reg_dump.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o reg_dump.if reg_dump.asm reg_dump.tmp 
	.version	40
FP	.set		AR3
	.file	"reg_dump.c"
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
	.file	"reg_dump.c"
	.globl	_monitor_set

	.sym	_reg_dump,_reg_dump,32,2,0
	.globl	_reg_dump

	.func	6
;>>>> 	void reg_dump( reg_set monitor_set, char put_where )
******************************************************
* FUNCTION DEF : _reg_dump
******************************************************
_reg_dump:
	PUSH	FP
	LDI	SP,FP
	.sym	_monitor_set,-48,8,9,1504,.fake4
	.sym	_put_where,-49,2,9,32
	.line	2
	.line	3
;>>>> 		c40_printf( "\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	4
;>>>> 		c40_printf( "R0 => " );
	LDI	@CONST+1,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	5
;>>>> 		print_reg( monitor_set.DSP_ir0, monitor_set.DSP_fr0, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(36),R1
	PUSH	R1
	LDI	*-FP(48),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
	.line	6
;>>>> 		c40_printf( "     " );
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	7
;>>>> 		c40_printf( "R1 => " );
	LDI	@CONST+3,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	8
;>>>> 		print_reg( monitor_set.DSP_ir1, monitor_set.DSP_fr1, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(35),R1
	PUSH	R1
	LDI	*-FP(47),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
	.line	9
;>>>> 	   c40_printf( "\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	10
;>>>> 		c40_printf( "R2 => " );
	LDI	@CONST+4,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	11
;>>>> 		print_reg( monitor_set.DSP_ir2, monitor_set.DSP_fr2, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(34),R1
	PUSH	R1
	LDI	*-FP(46),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
	.line	12
;>>>> 	   c40_printf( "     " );
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	13
;>>>> 		c40_printf( "R3 => " );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	14
;>>>> 		print_reg( monitor_set.DSP_ir3, monitor_set.DSP_fr3, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(33),R1
	PUSH	R1
	LDI	*-FP(45),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
	.line	15
;>>>> 	   c40_printf( "\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	16
;>>>> 		c40_printf( "R4 => " );
	LDI	@CONST+6,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	17
;>>>> 		print_reg( monitor_set.DSP_ir4, monitor_set.DSP_fr4, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(32),R1
	PUSH	R1
	LDI	*-FP(44),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
	.line	18
;>>>> 	   c40_printf( "     " );
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	19
;>>>> 		c40_printf( "R5 => " );
	LDI	@CONST+7,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	20
;>>>> 		print_reg( monitor_set.DSP_ir5, monitor_set.DSP_fr5, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(31),R1
	PUSH	R1
	LDI	*-FP(43),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
	.line	21
;>>>> 	   c40_printf( "\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	22
;>>>> 		c40_printf( "R6 => " );
	LDI	@CONST+8,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	23
;>>>> 		print_reg( monitor_set.DSP_ir6, monitor_set.DSP_fr6, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(30),R1
	PUSH	R1
	LDI	*-FP(42),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
	.line	24
;>>>> 	   c40_printf( "     " );
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	25
;>>>> 		c40_printf( "R7 => " );
	LDI	@CONST+9,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	26
;>>>> 		print_reg( monitor_set.DSP_ir7, monitor_set.DSP_fr7, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(29),R1
	PUSH	R1
	LDI	*-FP(41),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
	.line	27
;>>>> 	   c40_printf( "\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	28
;>>>> 		c40_printf( "R8 => " );
	LDI	@CONST+10,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	29
;>>>> 		print_reg( monitor_set.DSP_ir8, monitor_set.DSP_fr8, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(28),R1
	PUSH	R1
	LDI	*-FP(40),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
	.line	30
;>>>> 	   c40_printf( "     " );
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	31
;>>>> 		c40_printf( "R9 => " );
	LDI	@CONST+11,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	32
;>>>> 		print_reg( monitor_set.DSP_ir9, monitor_set.DSP_fr9, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(27),R1
	PUSH	R1
	LDI	*-FP(39),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
	.line	33
;>>>> 	   c40_printf( "\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	34
;>>>> 		c40_printf( "R10 => " );
	LDI	@CONST+12,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	35
;>>>> 		print_reg( monitor_set.DSP_ir10, monitor_set. DSP_fr10, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(26),R1
	PUSH	R1
	LDI	*-FP(38),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
	.line	36
;>>>> 		c40_printf( "    " );
	LDI	@CONST+13,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	37
;>>>> 		c40_printf( "R11 => " );
	LDI	@CONST+14,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	38
;>>>> 		print_reg( monitor_set.DSP_ir11,  monitor_set.DSP_fr11, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(25),R1
	PUSH	R1
	LDI	*-FP(37),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
	.line	39
;>>>> 		c40_printf( "\n\n" );
	LDI	@CONST+15,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	41
;>>>> 		c40_printf( "ar0 = %x     ar1 = %x     ar2 = %x     ar3 = %x\n",
;>>>> 				  monitor_set.DSP_ar0, monitor_set.DSP_ar1,
;>>>> 				  monitor_set.DSP_ar2, monitor_set.DSP_ar3 );
	LDI	*-FP(21),R0
	PUSH	R0
	LDI	*-FP(22),R1
	PUSH	R1
	LDI	*-FP(23),R2
	PUSH	R2
	LDI	*-FP(24),R3
	PUSH	R3
	LDI	@CONST+16,R9
	PUSH	R9
	CALL	_c40_printf
	SUBI	5,SP
	.line	44
;>>>> 		c40_printf( "ar4 = %x     ar5 = %x     ar6 = %x     ar7 = %x\n",
;>>>> 				  monitor_set.DSP_ar4, monitor_set.DSP_ar5,
;>>>> 				  monitor_set.DSP_ar6, monitor_set.DSP_ar7 );
	LDI	*-FP(17),R0
	PUSH	R0
	LDI	*-FP(18),R1
	PUSH	R1
	LDI	*-FP(19),R2
	PUSH	R2
	LDI	*-FP(20),R3
	PUSH	R3
	LDI	@CONST+17,R9
	PUSH	R9
	CALL	_c40_printf
	SUBI	5,SP
	.line	47
;>>>> 		c40_printf( "\nDP = %x      IR0 = %x     IR1 = %x     SP = %x\n",
;>>>> 				  monitor_set.DSP_DP, monitor_set.DSP_IR0,
;>>>> 				  monitor_set.DSP_IR1, monitor_set.DSP_SP );
	LDI	*-FP(12),R0
	PUSH	R0
	LDI	*-FP(14),R1
	PUSH	R1
	LDI	*-FP(15),R2
	PUSH	R2
	LDI	*-FP(16),R3
	PUSH	R3
	LDI	@CONST+18,R9
	PUSH	R9
	CALL	_c40_printf
	SUBI	5,SP
	.line	50
;>>>> 		c40_printf( "ST = %x      IIE = %x     IIF = %x     DIE = %x\n",
;>>>> 				  monitor_set.DSP_ST, monitor_set.DSP_IIE,
;>>>> 				  monitor_set.DSP_IIF, monitor_set.DSP_DIE );
	LDI	*-FP(10),R0
	PUSH	R0
	LDI	*-FP(8),R1
	PUSH	R1
	LDI	*-FP(9),R2
	PUSH	R2
	LDI	*-FP(11),R3
	PUSH	R3
	LDI	@CONST+19,R9
	PUSH	R9
	CALL	_c40_printf
	SUBI	5,SP
	.line	53
;>>>> 		c40_printf( "IVTP = %x   TVTP = %x\n\n",
;>>>> 				  monitor_set.DSP_IVTP, monitor_set.DSP_TVTP );
	LDI	*-FP(3),R0
	PUSH	R0
	LDI	*-FP(4),R1
	PUSH	R1
	LDI	@CONST+20,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	3,SP
EPI0_1:
	.line	55
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	60,000000000H,0

	.sym	_print_reg,_print_reg,32,2,0
	.globl	_print_reg

	.func	65
;>>>> 	void print_reg( unsigned long lower, unsigned long upper, char put_where )
******************************************************
* FUNCTION DEF : _print_reg
******************************************************
_print_reg:
	PUSH	FP
	LDI	SP,FP
	ADDI	9,SP
	.sym	_lower,-2,15,9,32
	.sym	_upper,-3,15,9,32
	.sym	_put_where,-4,2,9,32
	.sym	_buf,1,50,1,288,,9
	.line	2
;>>>> 		char buf[9];
	.line	5
;>>>> 		c40_printf( "0x" );
	LDI	@CONST+21,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	6
;>>>> 	   xtoa( upper, buf );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*-FP(3),R0
	PUSH	R0
	CALL	_xtoa
	SUBI	2,SP
	.line	7
;>>>> 		buf[2] = '\0';
	STIK	0,*+FP(3)
	.line	8
;>>>> 		putstr( buf );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	CALL	_putstr
	SUBI	1,SP
	.line	9
;>>>> 		c40_printf( " %x = %f", lower, upper );
	LDI	*-FP(3),R0
	PUSH	R0
	LDI	*-FP(2),R1
	PUSH	R1
	LDI	@CONST+22,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	3,SP
EPI0_2:
	.line	10
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	11,SP
	B	R1
	.endfunc	74,000000000H,9
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.sect	".const"
SL0:	.byte	10,0
SL1:	.byte	"R0 => ",0
SL2:	.byte	"     ",0
SL3:	.byte	"R1 => ",0
SL4:	.byte	"R2 => ",0
SL5:	.byte	"R3 => ",0
SL6:	.byte	"R4 => ",0
SL7:	.byte	"R5 => ",0
SL8:	.byte	"R6 => ",0
SL9:	.byte	"R7 => ",0
SL10:	.byte	"R8 => ",0
SL11:	.byte	"R9 => ",0
SL12:	.byte	"R10 => ",0
SL13:	.byte	"    ",0
SL14:	.byte	"R11 => ",0
SL15:	.byte	10,10,0
SL16:	.byte	"ar0 = %x     ar1 = %x     ar2 = %x     ar3 = %x",10,0
SL17:	.byte	"ar4 = %x     ar5 = %x     ar6 = %x     ar7 = %x",10,0
SL18:	.byte	10,"DP = %x      IR0 = %x     IR1 = %x     SP = %x",10,0
SL19:	.byte	"ST = %x      IIE = %x     IIF = %x     DIE = %x",10,0
SL20:	.byte	"IVTP = %x   TVTP = %x",10,10,0
SL21:	.byte	"0x",0
SL22:	.byte	" %x = %f",0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,23
	.sect	".cinit"
	.word	23,CONST
	.word 	SL0              ;0
	.word 	SL1              ;1
	.word 	SL2              ;2
	.word 	SL3              ;3
	.word 	SL4              ;4
	.word 	SL5              ;5
	.word 	SL6              ;6
	.word 	SL7              ;7
	.word 	SL8              ;8
	.word 	SL9              ;9
	.word 	SL10             ;10
	.word 	SL11             ;11
	.word 	SL12             ;12
	.word 	SL13             ;13
	.word 	SL14             ;14
	.word 	SL15             ;15
	.word 	SL16             ;16
	.word 	SL17             ;17
	.word 	SL18             ;18
	.word 	SL19             ;19
	.word 	SL20             ;20
	.word 	SL21             ;21
	.word 	SL22             ;22
	.end
