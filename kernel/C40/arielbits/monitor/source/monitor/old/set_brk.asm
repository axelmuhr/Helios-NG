******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 -ic:\c40 set_brk.c C:\TMP\set_brk.if 
;	cg30 -v40 -o -n C:\TMP\set_brk.if C:\TMP\set_brk.asm C:\TMP\set_brk.tmp 
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
	.globl	_set_brk
;>>>> 	int set_brk( unsigned long parms[], char put_where )
;>>>> 		int i;
******************************************************
* FUNCTION DEF : _set_brk
******************************************************
_set_brk:
	PUSH	FP
	LDI	SP,FP
	BD	L2
	ADDI	1,SP
;>>>> 		for( i=0 ; (i < MAX_BREAKS) && breaks[i] ; i++ );
	LDI	0,R0
	STI	R0,*+FP(1)
***	B	L2	;BRANCH OCCURS
L1:
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
L2:
	CMPI	8,R0
	BGE	LL3
	ADDI	@CONST+0,R0
	LDI	R0,AR0
	LDI	*AR0,R0
	BNZ	L1
LL3:
;>>>> 		if( i == MAX_BREAKS )
	LDI	*+FP(1),R0
	CMPI	8,R0
	BNZ	L3
;>>>> 			c40_printf( "Exceeded maximum number of breakpoints, no breakpoint set.\n" );
	LDI	@CONST+1,R1
	PUSH	R1
	CALL	_c40_printf
	BD	EPI0_1
	SUBI	1,SP
	NOP
;>>>> 			return( FAILURE );
	LDI	0,R0
***	B	EPI0_1	;BRANCH OCCURS
L3:
;>>>> 		for( i=0 ; i < 3 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
L4:
;>>>> 			if( ((*(unsigned long *)(parms[0]-i) & BcondAF_MASK) == BcondAF)
;>>>> 			 || ((*(unsigned long *)(parms[0]-i) & BcondAT_MASK) == BcondAT)
;>>>> 			 || ((*(unsigned long *)(parms[0]-i) & BcondD_MASK) == BcondD)
;>>>> 			 || ((*(unsigned long *)(parms[0]-i) & BRD_MASK) == BRD)
;>>>> 			 || ((*(unsigned long *)(parms[0]-i) & DBcondD_MASK) == DBcondD)
;>>>> 			 || ((*(unsigned long *)(parms[0]-i) & RETIcondD_MASK) == RETIcondD)
;>>>> 			 || ((*(unsigned long *)(parms[0]-i) & RPTBDreg_MASK) == RPTBDreg)
;>>>> 			 || ((*(unsigned long *)(parms[0]-i) & RPTBDim_MASK) == RPTBDim)
;>>>> 			 )
	LDI	*-FP(2),AR0
	SUBI	*+FP(1),*AR0,AR1
	LDI	*AR1,R0
	AND	@CONST+2,R0
	CMPI	@CONST+3,R0
	BZ	LL4
	SUBI	*+FP(1),*AR0,AR1
	LDI	*AR1,R0
	AND	@CONST+2,R0
	CMPI	@CONST+4,R0
	BZ	LL4
	SUBI	*+FP(1),*AR0,AR1
	LDI	*AR1,R0
	AND	@CONST+2,R0
	CMPI	@CONST+5,R0
	BZ	LL4
	SUBI	*+FP(1),*AR0,AR1
	LDI	*AR1,R0
	AND	@CONST+6,R0
	CMPI	@CONST+7,R0
	BZ	LL4
	SUBI	*+FP(1),*AR0,AR1
	LDI	*AR1,R0
	AND	@CONST+8,R0
	CMPI	@CONST+9,R0
	BZ	LL4
	SUBI	*+FP(1),*AR0,AR1
	LDI	*AR1,R0
	AND	@CONST+10,R0
	CMPI	@CONST+11,R0
	BZ	LL4
	SUBI	*+FP(1),*AR0,AR1
	LDI	*AR1,R0
	AND	@CONST+6,R0
	CMPI	@CONST+12,R0
	BZ	LL4
	SUBI	*+FP(1),*AR0,AR1
	LDI	*AR1,R0
	ANDN	01fh,R0
	CMPI	@CONST+13,R0
	BNZ	L6
LL4:
;>>>> 				c40_printf( "Can't insert a breakpoint at an address that is\n"
;>>>> 								"one of the three instructions following a delayed\n"
;>>>> 								"program control instruction.\n"
;>>>> 								"No breakpoint set.\n" );
	LDI	@CONST+14,R0
	PUSH	R0
	CALL	_c40_printf
	BD	EPI0_1
	SUBI	1,SP
	NOP
;>>>> 				return( FAILURE );
	LDI	0,R0
***	B	EPI0_1	;BRANCH OCCURS
L6:
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	3,R0
	BLT	L4
;>>>> 		brk_addrs[i] = parms[0];
	ADDI	@CONST+15,R0
	LDI	R0,AR1
	LDI	*AR0,R0
	STI	R0,*AR1
;>>>> 		breaks[i] = *(unsigned long *)brk_addrs[i];
	LDI	@CONST+0,R0
	ADDI	*+FP(1),R0
	LDI	R0,AR1
	LDI	@CONST+15,R0
	ADDI	*+FP(1),R0
	LDI	R0,AR2
	LDI	*AR2,AR2
	LDI	*AR2,R0
	STI	R0,*AR1
;>>>> 		*(unsigned long *)brk_addrs[i] = BREAK_TRAP;
	LDI	@CONST+15,R0
	ADDI	*+FP(1),R0
	LDI	R0,AR1
	LDI	*AR1,AR1
	LDI	@CONST+16,R0
	STI	R0,*AR1
;>>>> 		return( SUCCESS );
	LDI	1,R0
EPI0_1:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
	.globl	_del_brk
;>>>> 	int del_brk( unsigned long parms[], char put_where )
;>>>> 		int i;
******************************************************
* FUNCTION DEF : _del_brk
******************************************************
_del_brk:
	PUSH	FP
	LDI	SP,FP
	BD	L8
	ADDI	1,SP
;>>>> 		for( i=0 ; (i < MAX_BREAKS) && (brk_addrs[i] != parms[0]) ; i++ );
	LDI	0,R0
	STI	R0,*+FP(1)
***	B	L8	;BRANCH OCCURS
L7:
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
L8:
	CMPI	8,R0
	BGE	LL7
	ADDI	@CONST+15,R0
	LDI	R0,AR0
	LDI	*-FP(2),AR1
	CMPI	*AR1,*AR0
	BNZ	L7
LL7:
;>>>> 		if( i == MAX_BREAKS )
	LDI	*+FP(1),R0
	CMPI	8,R0
	BNZ	L9
;>>>> 			c40_printf( "Breakpoint not set at address %xh.\n", parms[0] );
	LDI	*-FP(2),AR0
	LDI	*AR0,R1
	PUSH	R1
	LDI	@CONST+17,R1
	PUSH	R1
	CALL	_c40_printf
	BD	EPI0_2
	SUBI	2,SP
	NOP
;>>>> 			return( FAILURE );
	LDI	0,R0
***	B	EPI0_2	;BRANCH OCCURS
L9:
;>>>> 		for( ; i < MAX_BREAKS-1 ; i++ )
	CMPI	7,R0
	BGE	L11
L10:
;>>>> 			brk_addrs[i] = brk_addrs[i+1];
	LDI	@CONST+15,R0
	ADDI	*+FP(1),R0
	LDI	R0,AR0
	LDI	@CONST+18,R3
	ADDI	*+FP(1),R3
	LDI	R3,AR1
	LDI	*AR1,R0
	STI	R0,*AR0
;>>>> 			breaks[i] = breaks[i+1];
	LDI	@CONST+0,R0
	ADDI	*+FP(1),R0
	LDI	R0,AR0
	LDI	@CONST+19,R3
	ADDI	*+FP(1),R3
	LDI	R3,AR1
	LDI	*AR1,R0
	STI	R0,*AR0
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	7,R0
	BLT	L10
L11:
;>>>> 		brk_addrs[MAX_BREAKS-1] = 0;
	LDI	0,R1
	STI	R1,@_brk_addrs+7
;>>>> 		breaks[MAX_BREAKS-1] = 0;
	STI	R1,@_breaks+7
;>>>> 		return( SUCCESS );
	LDI	1,R0
EPI0_2:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
	.globl	_list_brks
;>>>> 	void list_brks( char put_where )
;>>>> 		int i;
******************************************************
* FUNCTION DEF : _list_brks
******************************************************
_list_brks:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
;>>>> 		c40_printf( "Break points at addresses:\n" );
	LDI	@CONST+20,R0
	PUSH	R0
	CALL	_c40_printf
	BD	L13
	SUBI	1,SP
;>>>> 		for( i=0 ; (i < MAX_BREAKS) && brk_addrs[i] ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
;>>>> 			c40_printf( "     %xh\n", brk_addrs[i] );
***	B	L13	;BRANCH OCCURS
L12:
	LDI	@CONST+15,R0
	ADDI	*+FP(1),R0
	LDI	R0,AR0
	LDI	*AR0,R0
	PUSH	R0
	LDI	@CONST+21,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
L13:
	CMPI	8,R0
	BGE	EPI0_3
	ADDI	@CONST+15,R0
	LDI	R0,AR0
	LDI	*AR0,R0
	BNZ	L12
EPI0_3:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.text
SL0:	.byte	"Exceeded maximum number of breakpoints, no breakpoint set."
	.byte	10,0
SL1:	.byte	"Can't insert a breakpoint at an address that is",10,"one of"
	.byte	" the three instructions following a delayed",10,"program co"
	.byte	"ntrol instruction.",10,"No breakpoint set.",10,0
SL2:	.byte	"Breakpoint not set at address %xh.",10,0
SL3:	.byte	"Break points at addresses:",10,0
SL4:	.byte	"     %xh",10,0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,22
	.sect	".cinit"
	.word	22,CONST
	.word 	_breaks          ;0
	.word 	SL0              ;1
	.word 	0fde00000h       ;2
	.word 	1755316224       ;3
	.word 	1751121920       ;4
	.word 	1746927616       ;5
	.word 	0ff000000h       ;6
	.word 	1627389952       ;7
	.word 	0fc200000h       ;8
	.word 	1814036480       ;9
	.word 	0ffe00000h       ;10
	.word 	2015363072       ;11
	.word 	1694498816       ;12
	.word 	2038431744       ;13
	.word 	SL1              ;14
	.word 	_brk_addrs       ;15
	.word 	1946157567       ;16
	.word 	SL2              ;17
	.word 	_brk_addrs+1     ;18
	.word 	_breaks+1        ;19
	.word 	SL3              ;20
	.word 	SL4              ;21
	.end
