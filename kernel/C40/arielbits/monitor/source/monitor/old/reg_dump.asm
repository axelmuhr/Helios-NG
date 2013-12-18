******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 -ic:\c40 reg_dump.c C:\TMP\reg_dump.if 
;	cg30 -v40 -o -n C:\TMP\reg_dump.if C:\TMP\reg_dump.asm C:\TMP\reg_dump.tmp 
FP	.set	AR3
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
	.globl	_test
	.globl	_help
	.globl	_read_eeprom
	.globl	_write_eeprom
	.globl	_c40_printf
	.globl	_putstr
	.globl	_c40_putchar
	.globl	_ftoa
	.globl	_send_host
	.globl	_xtoa
	.globl	_atox
	.globl	_atod
	.globl	_key_handler
	.globl	_c40_getchar
	.globl	_iswhite
	.globl	_reset_others
	.globl	_boot_others
	.globl	_boot_copy
	.globl	_led
	.globl	_configure
	.globl	_display_conf
	.globl	_get_addr
	.globl	_get_daughter
	.globl	_get_clock
	.globl	_get_baud
	.globl	_get_dram
	.globl	_menu
	.globl	_read_config
	.globl	_write_config
	.globl	_update_chksum
	.globl	_x_rcvcrc
	.globl	_x_sndcrc
	.globl	_mk_crctbl
	.globl	_crchware
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
	.globl	_monitor_set
	.globl	_reg_dump
;>>>> 	void reg_dump( reg_set monitor_set, char put_where )
******************************************************
* FUNCTION DEF : _reg_dump
******************************************************
_reg_dump:
	PUSH	FP
	LDI	SP,FP
;>>>> 		c40_printf( "\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		c40_printf( "R0 => " );
	LDI	@CONST+1,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		print_reg( monitor_set.DSP_ir0, monitor_set.DSP_fr0, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(36),R1
	PUSH	R1
	LDI	*-FP(48),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
;>>>> 		c40_printf( "     " );
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		c40_printf( "R1 => " );
	LDI	@CONST+3,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		print_reg( monitor_set.DSP_ir1, monitor_set.DSP_fr1, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(35),R1
	PUSH	R1
	LDI	*-FP(47),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
;>>>> 	   c40_printf( "\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		c40_printf( "R2 => " );
	LDI	@CONST+4,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		print_reg( monitor_set.DSP_ir2, monitor_set.DSP_fr2, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(34),R1
	PUSH	R1
	LDI	*-FP(46),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
;>>>> 	   c40_printf( "     " );
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		c40_printf( "R3 => " );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		print_reg( monitor_set.DSP_ir3, monitor_set.DSP_fr3, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(33),R1
	PUSH	R1
	LDI	*-FP(45),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
;>>>> 	   c40_printf( "\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		c40_printf( "R4 => " );
	LDI	@CONST+6,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		print_reg( monitor_set.DSP_ir4, monitor_set.DSP_fr4, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(32),R1
	PUSH	R1
	LDI	*-FP(44),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
;>>>> 	   c40_printf( "     " );
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		c40_printf( "R5 => " );
	LDI	@CONST+7,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		print_reg( monitor_set.DSP_ir5, monitor_set.DSP_fr5, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(31),R1
	PUSH	R1
	LDI	*-FP(43),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
;>>>> 	   c40_printf( "\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		c40_printf( "R6 => " );
	LDI	@CONST+8,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		print_reg( monitor_set.DSP_ir6, monitor_set.DSP_fr6, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(30),R1
	PUSH	R1
	LDI	*-FP(42),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
;>>>> 	   c40_printf( "     " );
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		c40_printf( "R7 => " );
	LDI	@CONST+9,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		print_reg( monitor_set.DSP_ir7, monitor_set.DSP_fr7, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(29),R1
	PUSH	R1
	LDI	*-FP(41),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
;>>>> 	   c40_printf( "\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		c40_printf( "R8 => " );
	LDI	@CONST+10,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		print_reg( monitor_set.DSP_ir8, monitor_set.DSP_fr8, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(28),R1
	PUSH	R1
	LDI	*-FP(40),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
;>>>> 	   c40_printf( "     " );
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		c40_printf( "R9 => " );
	LDI	@CONST+11,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		print_reg( monitor_set.DSP_ir9, monitor_set.DSP_fr9, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(27),R1
	PUSH	R1
	LDI	*-FP(39),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
;>>>> 	   c40_printf( "\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		c40_printf( "R10 => " );
	LDI	@CONST+12,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		print_reg( monitor_set.DSP_ir10, monitor_set. DSP_fr10, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(26),R1
	PUSH	R1
	LDI	*-FP(38),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
;>>>> 		c40_printf( "    " );
	LDI	@CONST+13,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		c40_printf( "R11 => " );
	LDI	@CONST+14,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		print_reg( monitor_set.DSP_ir11,  monitor_set.DSP_fr11, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(25),R1
	PUSH	R1
	LDI	*-FP(37),R2
	PUSH	R2
	CALL	_print_reg
	SUBI	3,SP
;>>>> 		c40_printf( "\n\n" );
	LDI	@CONST+15,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
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
	LDI	@CONST+16,R3
	PUSH	R3
	CALL	_c40_printf
	SUBI	5,SP
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
	LDI	@CONST+17,R3
	PUSH	R3
	CALL	_c40_printf
	SUBI	5,SP
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
	LDI	@CONST+18,R3
	PUSH	R3
	CALL	_c40_printf
	SUBI	5,SP
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
	LDI	@CONST+19,R3
	PUSH	R3
	CALL	_c40_printf
	SUBI	5,SP
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
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	2,SP
***	B	R1	;BRANCH OCCURS
	.globl	_print_reg
;>>>> 	void print_reg( unsigned long lower, unsigned long upper, char put_where )
;>>>> 		char buf[9];
******************************************************
* FUNCTION DEF : _print_reg
******************************************************
_print_reg:
	PUSH	FP
	LDI	SP,FP
	ADDI	9,SP
;>>>> 		c40_printf( "0x" );
	LDI	@CONST+21,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 	   xtoa( upper, buf );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*-FP(3),R1
	PUSH	R1
	CALL	_xtoa
	SUBI	2,SP
;>>>> 		buf[2] = '\0';
	LDI	0,R0
	STI	R0,*+FP(3)
;>>>> 		putstr( buf );
	LDI	FP,R1
	ADDI	1,R1
	PUSH	R1
	CALL	_putstr
	SUBI	1,SP
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
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	11,SP
***	B	R1	;BRANCH OCCURS
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.text
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
