******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 -ic:\c40 go.c C:\TMP\go.if 
;	cg30 -v40 -o -n C:\TMP\go.if C:\TMP\go.asm C:\TMP\go.tmp 
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
	.globl	_breaks
	.globl	_brk_addrs
	.globl	_go
;>>>> 	void go( reg_set *call_set )
;>>>> 		int i;
******************************************************
* FUNCTION DEF : _go
******************************************************
_go:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
;>>>> 	   int is_at_break=MAX_BREAKS;
	LDI	8,R0
	STI	R0,*+FP(2)
;>>>> 		for( i=0 ; i < MAX_BREAKS ; i++ )
	LDI	0,R1
	STI	R1,*+FP(1)
L1:
;>>>> 			if( call_set->ret_add == brk_addrs[i] )
	LDI	@CONST+0,R0
	ADDI	*+FP(1),R0
	LDI	R0,AR0
	LDI	*-FP(2),AR1
	LDI	*+AR1(46),R0
	CMPI	*AR0,R0
	BNZ	L3
;>>>> 				is_at_break = i;
	LDI	*+FP(1),R0
	STI	R0,*+FP(2)
;>>>> 				*(unsigned long *)brk_addrs[i] = breaks[i];
	ADDI	@CONST+0,R0
	LDI	R0,AR0
	LDI	*AR0,AR0
	LDI	@CONST+1,R0
	ADDI	*+FP(1),R0
	BD	L2
	LDI	R0,AR2
	LDI	*AR2,R0
	STI	R0,*AR0
;>>>> 				break;
***	B	L2	;BRANCH OCCURS
L3:
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	8,R0
	BLT	L1
L2:
;>>>> 		run( BREAK_TRAP_NUM );
	LDI	511,R0
	PUSH	R0
	CALL	_run
	SUBI	1,SP
;>>>> 		if( *(unsigned long *)(call_set->ret_add-1) == BREAK_TRAP )
	LDI	*-FP(2),AR0
	LDI	*+AR0(46),AR1
	LDI	*-AR1(1),R0
	CMPI	@CONST+2,R0
	LDIU	0,R0
	LDIZ	1,R0
	OR	01ffh,R0
	BZ	L4
;>>>> 			call_set->ret_add--;
	LDI	*+AR0(46),R0
	SUBI	1,R0
	STI	R0,*+AR0(46)
L4:
;>>>> 		if( is_at_break != MAX_BREAKS )
	LDI	*+FP(2),R0
	CMPI	8,R0
	BZ	L5
;>>>> 			*(unsigned long *)brk_addrs[is_at_break] = BREAK_TRAP;
	ADDI	@CONST+0,R0
	LDI	R0,AR1
	LDI	*AR1,AR1
	LDI	@CONST+3,R0
	STI	R0,*AR1
L5:
;>>>> 		reg_dump( *call_set, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	SP,AR1
	ADDI	47,SP
	LDI	*AR0++,R1
	RPTS	46
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_reg_dump
	SUBI	48,SP
EPI0_1:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	4,SP
***	B	R1	;BRANCH OCCURS
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,4
	.sect	".cinit"
	.word	4,CONST
	.word 	_brk_addrs       ;0
	.word 	_breaks          ;1
	.word 	1946157056       ;2
	.word 	1946157567       ;3
	.end
