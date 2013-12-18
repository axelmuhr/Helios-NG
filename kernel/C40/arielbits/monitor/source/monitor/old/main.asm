******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 -ic:\c40 main.c C:\TMP\main.if 
;	cg30 -v40 -o -n C:\TMP\main.if C:\TMP\main.asm C:\TMP\main.tmp 
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
	.globl	_config
	.globl	_monitor_set
	.globl	_call_set
	.globl	_breaks
	.globl	_brk_addrs
	.globl	_key_ptr
	.globl	_crctab
	.globl	_main
;>>>> 	main( void )
;>>>> 		int i, j;
******************************************************
* FUNCTION DEF : _main
******************************************************
_main:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
;>>>> 		mk_crctbl( CRC16, crchware );
	LDI	@CONST+0,R0
	PUSH	R0
	LDI	4129,R1
	PUSH	R1
	CALL	_mk_crctbl
	SUBI	2,SP
;>>>> 		for( i=0 ; i < 5 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
L1:
;>>>> 			c40_printf( "%c\n", BEEP );
	LDI	7,R0
	PUSH	R0
	LDI	@CONST+1,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 			for( j=0 ; j < 1e6 ; j++ );
	LDI	0,R0
	STI	R0,*+FP(2)
	FLOAT	R0,R1
	CMPF	@CONST+2,R1
	BGE	L4
L3:
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	FLOAT	R0,R1
	CMPF	@CONST+2,R1
	BLT	L3
L4:
	LDI	*+FP(1),R1
	ADDI	1,R1
	STI	R1,*+FP(1)
	CMPI	5,R1
	BLT	L1
;>>>> 		init( &bus_error_handler );    /* Initialize HYDRA */
	LDI	@CONST+3,R2
	PUSH	R2
	CALL	_init
	SUBI	1,SP
;>>>> 		key_ptr = 0;
	LDI	0,R0
	STI	R0,@_key_ptr
;>>>> 		zero_regs( &monitor_set );
	LDI	@CONST+4,R1
	PUSH	R1
	CALL	_zero_regs
	SUBI	1,SP
;>>>> 		zero_regs( &call_set );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_zero_regs
	SUBI	1,SP
;>>>> 		for( i=0 ; i < MAX_BREAKS ; i++ ) /* Reset all breakpoints */
	LDI	0,R0
	STI	R0,*+FP(1)
L5:
;>>>> 			breaks[i] = 0;
	LDI	@CONST+6,R0
	ADDI	*+FP(1),R0
	LDI	R0,AR0
	LDI	0,R0
	STI	R0,*AR0
;>>>> 			brk_addrs[i] = 0;
	LDI	@CONST+7,R1
	ADDI	*+FP(1),R1
	LDI	R1,AR0
	STI	R0,*AR0
	LDI	*+FP(1),R1
	ADDI	1,R1
	STI	R1,*+FP(1)
	CMPI	8,R1
	BLT	L5
;>>>> 		setup( &c_int01 );
	LDI	@CONST+8,R2
	PUSH	R2
	CALL	_setup
	SUBI	1,SP
;>>>> 		if( read_config( &config ) )
	LDI	@CONST+9,R0
	PUSH	R0
	CALL	_read_config
	SUBI	1,SP
	CMPI	0,R0
	BZ	L7
;>>>> 			test( config, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	@CONST+9,AR0
	LDI	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_test
	SUBI	18,SP
;>>>> 		else
	B	L8
L7:
;>>>> 			c40_printf( "*************************\n"
;>>>> 							"******** WARNING ********\n"
;>>>> 							"*************************\n"
;>>>> 							"\n"
;>>>> 							"There is an EEPROM checksum error.\n"
;>>>> 							"Fix the incorrect feild with the configure service.\n"
;>>>> 							"Improper operation may result if this is not done.\n"
;>>>> 							"\n"
;>>>> 							"Not performing power on testing.\n"
;>>>> 							"\n"
;>>>> 							"Hit any key to continue.\n" );
	LDI	@CONST+10,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 			c40_getchar();
	CALL	_c40_getchar
L8:
;>>>> 		monitor( &config );
	LDI	@CONST+9,R0
	PUSH	R0
	CALL	_monitor
	SUBI	1,SP
EPI0_1:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	4,SP
***	B	R1	;BRANCH OCCURS
	.globl	_zero_regs
;>>>> 	void zero_regs( reg_set *regs )
******************************************************
* FUNCTION DEF : _zero_regs
******************************************************
_zero_regs:
	PUSH	FP
	LDI	SP,FP
;>>>> 		regs->DSP_ir0 = 0;
	LDI	*-FP(2),AR0
	LDI	0,R0
	STI1	R0,*AR0
    ||	STI2	R0,*+AR0(1)
;>>>> 		regs->DSP_ir1 = 0;
;>>>> 		regs->DSP_ir2 = 0;
	STI	R0,*+AR0(2)
;>>>> 		regs->DSP_ir3 = 0;
	STI	R0,*+AR0(3)
;>>>> 		regs->DSP_ir4 = 0;
	STI	R0,*+AR0(4)
;>>>> 		regs->DSP_ir5 = 0;
	STI	R0,*+AR0(5)
;>>>> 		regs->DSP_ir6 = 0;
	STI	R0,*+AR0(6)
;>>>> 		regs->DSP_ir7 = 0;
	STI	R0,*+AR0(7)
;>>>> 		regs->DSP_ir8 = 0;
	STI	R0,*+AR0(8)
;>>>> 		regs->DSP_ir9 = 0;
	STI	R0,*+AR0(9)
;>>>> 		regs->DSP_ir10 = 0;
	STI	R0,*+AR0(10)
;>>>> 		regs->DSP_ir11 = 0;
	STI	R0,*+AR0(11)
;>>>> 		regs->DSP_fr0 = 0x7FFF0000;
	LDI	@CONST+11,R1
	STI	R1,*+AR0(12)
;>>>> 		regs->DSP_fr1 = 0x7FFF0000;
	STI	R1,*+AR0(13)
;>>>> 		regs->DSP_fr2 = 0x7FFF0000;
	STI	R1,*+AR0(14)
;>>>> 		regs->DSP_fr3 = 0x7FFF0000;
	STI	R1,*+AR0(15)
;>>>> 		regs->DSP_fr4 = 0x7FFF0000;
	STI	R1,*+AR0(16)
;>>>> 		regs->DSP_fr5 = 0x7FFF0000;
	STI	R1,*+AR0(17)
;>>>> 		regs->DSP_fr6 = 0x7FFF0000;
	STI	R1,*+AR0(18)
;>>>> 		regs->DSP_fr7 = 0x7FFF0000;
	STI	R1,*+AR0(19)
;>>>> 		regs->DSP_fr8 = 0x7FFF0000;
	STI	R1,*+AR0(20)
;>>>> 		regs->DSP_fr9 = 0x7FFF0000;
	STI	R1,*+AR0(21)
;>>>> 		regs->DSP_fr10 = 0x7FFF0000;
	STI	R1,*+AR0(22)
;>>>> 		regs->DSP_fr11 = 0x7FFF0000;
	STI	R1,*+AR0(23)
;>>>> 		regs->DSP_ar0 = 0;
	STI	R0,*+AR0(24)
;>>>> 		regs->DSP_ar1 = 0;
	STI	R0,*+AR0(25)
;>>>> 		regs->DSP_ar2 = 0;
	STI	R0,*+AR0(26)
;>>>> 		regs->DSP_ar3 = 0;
	STI	R0,*+AR0(27)
;>>>> 		regs->DSP_ar4 = 0;
	STI	R0,*+AR0(28)
;>>>> 		regs->DSP_ar5 = 0;
	STI	R0,*+AR0(29)
;>>>> 		regs->DSP_ar6 = 0;
	STI	R0,*+AR0(30)
;>>>> 		regs->DSP_ar7 = 0;
	STI	R0,*+AR0(31)
;>>>> 		regs->DSP_DP = 0;
	STI	R0,*+AR0(32)
;>>>> 		regs->DSP_IR0 = 0;
	STI	R0,*+AR0(33)
;>>>> 		regs->DSP_IR1 = 0;
	STI	R0,*+AR0(34)
;>>>> 		regs->DSP_BK = 0;
	STI	R0,*+AR0(35)
;>>>> 		regs->DSP_SP = 0;
	STI	R0,*+AR0(36)
;>>>> 		regs->DSP_ST = 0;
	STI	R0,*+AR0(37)
;>>>> 		regs->DSP_DIE = 0;
	STI	R0,*+AR0(38)
;>>>> 		regs->DSP_IIE = 0;
	STI	R0,*+AR0(39)
;>>>> 		regs->DSP_IIF = 0;
	STI	R0,*+AR0(40)
;>>>> 		regs->DSP_RS = 0;
	STI	R0,*+AR0(41)
;>>>> 		regs->DSP_RE = 0;
	STI	R0,*+AR0(42)
;>>>> 		regs->DSP_RC = 0;
	STI	R0,*+AR0(43)
;>>>> 		regs->DSP_IVTP = 0;
	STI	R0,*+AR0(44)
;>>>> 		regs->DSP_TVTP = 0;
	STI	R0,*+AR0(45)
;>>>> 		regs->ret_add = 0;
	STI	R0,*+AR0(46)
EPI0_2:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	2,SP
***	B	R1	;BRANCH OCCURS
	.globl	_bus_error_handler
;>>>> 	void bus_error_handler( void )
******************************************************
* FUNCTION DEF : _bus_error_handler
******************************************************
_bus_error_handler:
EPI0_3:
	RETS
	.globl	_config
	.bss	_config,17
	.globl	_crctab
	.bss	_crctab,256
	.globl	_breaks
	.bss	_breaks,8
	.globl	_call_set
	.bss	_call_set,47
	.globl	_brk_addrs
	.bss	_brk_addrs,8
	.globl	_monitor_set
	.bss	_monitor_set,47
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.text
SL0:	.byte	"%c",10,0
SL1:	.byte	"*************************",10,"******** WARNING ********",10
	.byte	"*************************",10,10,"There is an EEPROM checks"
	.byte	"um error.",10,"Fix the incorrect feild with the configure s"
	.byte	"ervice.",10,"Improper operation may result if this is not d"
	.byte	"one.",10,10,"Not performing power on testing.",10,10,"Hit a"
	.byte	"ny key to continue.",10,0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,12
	.sect	".cinit"
	.word	12,CONST
	.word 	_crchware        ;0
	.word 	SL0              ;1
	.float	1.0e6            ;2
	.word 	_bus_error_handler;3
	.word 	_monitor_set     ;4
	.word 	_call_set        ;5
	.word 	_breaks          ;6
	.word 	_brk_addrs       ;7
	.word 	_c_int01         ;8
	.word 	_config          ;9
	.word 	SL1              ;10
	.word 	2147418112       ;11
	.end
