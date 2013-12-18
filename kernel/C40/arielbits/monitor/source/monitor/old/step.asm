******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 -ic:\c40 step.c C:\TMP\step.if 
;	cg30 -v40 -o -n C:\TMP\step.if C:\TMP\step.asm C:\TMP\step.tmp 
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
	.globl	_step
;>>>> 	void step( reg_set *call_set )
;>>>> 		unsigned long break1, break2;
******************************************************
* FUNCTION DEF : _step
******************************************************
_step:
	PUSH	FP
	LDI	SP,FP
	ADDI	8,SP
;>>>> 		unsigned long break1adr, break2adr=NO_BREAK2;
	LDI	@CONST+0,R0
	STI	R0,*+FP(4)
;>>>> 		unsigned long instruction = *(unsigned long *)call_set->ret_add;
	LDI	*-FP(2),AR0
	LDI	*+AR0(46),AR1
	LDI	*AR1,R1
	STI	R1,*+FP(5)
;>>>> 		int reg_mode=FALSE;
;>>>> 		int i;
	LDI	0,R2
	STI	R2,*+FP(6)
;>>>> 		int is_at_break=MAX_BREAKS;
	LDI	8,R3
	STI	R3,*+FP(8)
;>>>> 		for( i=0 ; i < MAX_BREAKS ; i++ )
	STI	R2,*+FP(7)
L1:
;>>>> 			if( call_set->ret_add == brk_addrs[i] )
	LDI	@CONST+1,R0
	ADDI	*+FP(7),R0
	LDI	R0,AR0
	LDI	*-FP(2),AR1
	LDI	*+AR1(46),R0
	CMPI	*AR0,R0
	BNZ	L3
;>>>> 				is_at_break = i;
	LDI	*+FP(7),R0
	STI	R0,*+FP(8)
;>>>> 				*(unsigned long *)brk_addrs[i] = breaks[i];
	ADDI	@CONST+1,R0
	LDI	R0,AR0
	LDI	*AR0,AR0
	LDI	@CONST+2,R0
	ADDI	*+FP(7),R0
	BD	L2
	LDI	R0,AR2
	LDI	*AR2,R0
	STI	R0,*AR0
;>>>> 				break;
***	B	L2	;BRANCH OCCURS
L3:
	LDI	*+FP(7),R0
	ADDI	1,R0
	STI	R0,*+FP(7)
	CMPI	8,R0
	BLT	L1
L2:
;>>>> 		if( ((instruction & BcondAF_MASK) == BcondAF)
;>>>> 		 || ((instruction & BcondAT_MASK) == BcondAT)
;>>>> 		 || ((instruction & BcondD_MASK) == BcondD)
;>>>> 		 || ((instruction & DBcondD_MASK) == DBcondD)
;>>>> 	    || ((instruction & LAJcond_MASK) == LAJcond)
;>>>> 		 )
	LDI	*+FP(5),R0
	AND	@CONST+3,R0
	CMPI	@CONST+4,R0
	BZ	LL3
	LDI	*+FP(5),R0
	AND	@CONST+3,R0
	CMPI	@CONST+5,R0
	BZ	LL3
	LDI	*+FP(5),R0
	AND	@CONST+3,R0
	CMPI	@CONST+6,R0
	BZ	LL3
	LDI	*+FP(5),R0
	AND	@CONST+7,R0
	CMPI	@CONST+8,R0
	BZ	LL3
	LDI	*+FP(5),R0
	AND	@CONST+3,R0
	CMPI	@CONST+9,R0
	BNZ	L4
LL3:
;>>>> 			break1adr = call_set->ret_add + 4;
	LDI	*+AR1(46),R3
	ADDI	4,R3
	STI	R3,*+FP(3)
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDI	R3,AR0
	LDI	*AR0,R0
	STI	R0,*+FP(1)
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR0
;>>>> 			if( instruction & 0x02000000 ) /* Check for PC-Relative branch */
	LDI	*+FP(5),R2
	TSTB	@CONST+11,R2
	BZ	L5
;>>>> 				break2adr = call_set->ret_add + (int)(instruction & 0xFFFF) + 3;
	AND	0ffffh,R2
	LDI	*+AR1(46),R0
	ADDI	R0,R2
	ADDI	3,R2
	STI	R2,*+FP(4)
;>>>> 				break2 = *(unsigned long *)break2adr;
	LDI	R2,AR2
	BD	L7
	LDI	*AR2,R0
	STI	R0,*+FP(2)
;>>>> 				*(unsigned long *)break2adr = BREAK_TRAP;
	STI	R1,*AR2
;>>>> 			else /* Register mode */
***	B	L7	;BRANCH OCCURS
L5:
;>>>> 				reg_mode = TRUE;
	BD	L7
	LDI	1,R3
	NOP
	STI	R3,*+FP(6)
;>>>> 		else if( ((instruction & BRD_MASK) == BRD)
;>>>> 				|| ((instruction & LAJ_MASK) == LAJ)
;>>>> 			  )
***	B	L7	;BRANCH OCCURS
L4:
	LDI	*+FP(5),R0
	AND	@CONST+12,R0
	CMPI	@CONST+13,R0
	BZ	LL4
	LDI	*+FP(5),R0
	AND	@CONST+12,R0
	CMPI	@CONST+14,R0
	BNZ	L8
LL4:
;>>>> 			break1adr = call_set->ret_add + (int)(instruction & 0xFFFFFF) + 3;
	LDI	*+FP(5),R0
	AND	@CONST+15,R0
	LDI	*+AR1(46),R3
	ADDI	R0,R3
	ADDI	3,R3
	STI	R3,*+FP(3)
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDI	R3,AR0
	LDI	*AR0,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR0
;>>>> 		else if( ((instruction & Bcond_MASK) == Bcond)
;>>>> 				|| ((instruction & CALLcond_MASK) == CALLcond)
;>>>> 				|| ((instruction & DBcond_MASK) == DBcond)
;>>>> 				)
***	B	L7	;BRANCH OCCURS
L8:
	LDI	*+FP(5),R0
	AND	@CONST+16,R0
	CMPI	@CONST+17,R0
	BZ	LL5
	LDI	*+FP(5),R0
	AND	@CONST+16,R0
	CMPI	@CONST+18,R0
	BZ	LL5
	LDI	*+FP(5),R0
	AND	@CONST+19,R0
	CMPI	@CONST+20,R0
	BNZ	L10
LL5:
;>>>> 	   	break1adr = call_set->ret_add + 1;
	LDI	*+AR1(46),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDI	R3,AR0
	LDI	*AR0,R0
	STI	R0,*+FP(1)
;>>>> 			*(unsigned long *)(call_set->ret_add + 4) = BREAK_TRAP;
	LDI	*+AR1(46),AR2
	LDI	@CONST+10,R1
	STI	R1,*+AR2(4)
;>>>> 			if( instruction & 0x02000000 ) /* Check for PC-Relative branch */
	LDI	*+FP(5),R2
	TSTB	@CONST+11,R2
	BZ	L11
;>>>> 				break2adr = call_set->ret_add + (int)(instruction & 0xFFFF) + 1;
	AND	0ffffh,R2
	LDI	*+AR1(46),R0
	ADDI	R0,R2
	ADDI	1,R2
	STI	R2,*+FP(4)
;>>>> 				break2 = *(unsigned long *)break2adr;
	LDI	R2,AR2
	BD	L7
	LDI	*AR2,R0
	STI	R0,*+FP(2)
;>>>> 				*(unsigned long *)break2adr = BREAK_TRAP;
	STI	R1,*AR2
;>>>> 			else /* Register mode */
***	B	L7	;BRANCH OCCURS
L11:
;>>>> 				reg_mode = TRUE;
	BD	L7
	LDI	1,R3
	NOP
	STI	R3,*+FP(6)
;>>>> 		else if( ((instruction & CALL_MASK) == CALL)
;>>>> 				|| ((instruction & BR_MASK) == BR)
;>>>> 				)
***	B	L7	;BRANCH OCCURS
L10:
	LDI	*+FP(5),R0
	AND	@CONST+12,R0
	CMPI	@CONST+21,R0
	BZ	LL6
	LDI	*+FP(5),R0
	AND	@CONST+12,R0
	CMPI	@CONST+22,R0
	BNZ	L14
LL6:
;>>>> 			break1adr = call_set->ret_add + (int)(instruction & 0xFFFFFF) + 1;
	LDI	*+FP(5),R0
	AND	@CONST+15,R0
	LDI	*+AR1(46),R3
	ADDI	R0,R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDI	R3,AR0
	LDI	*AR0,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR0
;>>>> 		else if( ((instruction & RETIcondD_MASK) == RETIcondD) )
***	B	L7	;BRANCH OCCURS
L14:
	LDI	*+FP(5),R0
	AND	@CONST+23,R0
	CMPI	@CONST+24,R0
	BNZ	L16
;>>>> 	   	break1adr = call_set->ret_add + 4;
	LDI	*+AR1(46),R3
	ADDI	4,R3
	STI	R3,*+FP(3)
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDI	R3,AR0
	LDI	*AR0,R0
	STI	R0,*+FP(1)
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR0
;>>>> 			break2adr = *(unsigned long *)call_set->DSP_SP;
	LDI	*+AR1(36),AR2
	LDI	*AR2,R2
	STI	R2,*+FP(4)
;>>>> 			break2 = *(unsigned long *)break2adr;
	LDI	R2,AR2
	BD	L7
	LDI	*AR2,R3
	STI	R3,*+FP(2)
;>>>> 			*(unsigned long *)break2adr = BREAK_TRAP;
	STI	R1,*AR2
;>>>> 	   else if( ((instruction & RETIcond_MASK) == RETIcond)
;>>>> 				|| ((instruction & RETScond_MASK) == RETScond)
;>>>> 				)
***	B	L7	;BRANCH OCCURS
L16:
	LDI	*+FP(5),R0
	AND	@CONST+25,R0
	CMPI	@CONST+26,R0
	BZ	LL7
	LDI	*+FP(5),R0
	AND	@CONST+25,R0
	CMPI	@CONST+27,R0
	BNZ	L18
LL7:
;>>>> 			break1adr = call_set->ret_add + 1;
	LDI	*+AR1(46),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDI	R3,AR0
	LDI	*AR0,R0
	STI	R0,*+FP(1)
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR0
;>>>> 			break2adr = *(unsigned long *)call_set->DSP_SP;
	LDI	*+AR1(36),AR2
	LDI	*AR2,R2
	STI	R2,*+FP(4)
;>>>> 			break2 = *(unsigned long *)break2adr;
	LDI	R2,AR2
	BD	L7
	LDI	*AR2,R3
	STI	R3,*+FP(2)
;>>>> 			*(unsigned long *)break2adr = BREAK_TRAP;
	STI	R1,*AR2
;>>>> 		else if( ((instruction & RPTBreg_MASK) == RPTBreg)
;>>>> 				|| ((instruction & RPTBDreg_MASK) == RPTBDreg)
;>>>> 				)
***	B	L7	;BRANCH OCCURS
L18:
	LDI	*+FP(5),R0
	ANDN	01fh,R0
	CMPI	@CONST+28,R0
	BZ	L21
	LDI	*+FP(5),R0
	AND	@CONST+12,R0
	CMPI	@CONST+29,R0
	BZ	L21
	B	L20
;>>>> 			switch( instruction & 0x1F )
;>>>> 				case C40_R0 :
L22:
;>>>> 					break1adr = call_set->DSP_ir0 + 1;
	LDI	*-FP(2),AR0
	LDI	*AR0,R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_R1 :
***	B	L7	;BRANCH OCCURS
L24:
;>>>> 					break1adr = call_set->DSP_ir1 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(1),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_R2 :
***	B	L7	;BRANCH OCCURS
L25:
;>>>> 					break1adr = call_set->DSP_ir2 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(2),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_R3 :
***	B	L7	;BRANCH OCCURS
L26:
;>>>> 					break1adr = call_set->DSP_ir3 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(3),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_R4 :
***	B	L7	;BRANCH OCCURS
L27:
;>>>> 					break1adr = call_set->DSP_ir4 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(4),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_R5 :
***	B	L7	;BRANCH OCCURS
L28:
;>>>> 					break1adr = call_set->DSP_ir5 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(5),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_R6 :
***	B	L7	;BRANCH OCCURS
L29:
;>>>> 					break1adr = call_set->DSP_ir6 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(6),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_R7 :
***	B	L7	;BRANCH OCCURS
L30:
;>>>> 					break1adr = call_set->DSP_ir7 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(7),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_R8 :
***	B	L7	;BRANCH OCCURS
L31:
;>>>> 					break1adr = call_set->DSP_ir8 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(8),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_R9 :
***	B	L7	;BRANCH OCCURS
L32:
;>>>> 					break1adr = call_set->DSP_ir9 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(9),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_R10 :
***	B	L7	;BRANCH OCCURS
L33:
;>>>> 					break1adr = call_set->DSP_ir10 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(10),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_R11 :
***	B	L7	;BRANCH OCCURS
L34:
;>>>> 					break1adr = call_set->DSP_ir11 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(11),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_AR0 :
***	B	L7	;BRANCH OCCURS
L35:
;>>>> 					break1adr = call_set->DSP_ar0 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(24),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_AR1 :
***	B	L7	;BRANCH OCCURS
L36:
;>>>> 					break1adr = call_set->DSP_ar1 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(25),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_AR2 :
***	B	L7	;BRANCH OCCURS
L37:
;>>>> 					break1adr = call_set->DSP_ar2 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(26),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_AR3 :
***	B	L7	;BRANCH OCCURS
L38:
;>>>> 					break1adr = call_set->DSP_ar3 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(27),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_AR4 :
***	B	L7	;BRANCH OCCURS
L39:
;>>>> 					break1adr = call_set->DSP_ar4 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(28),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_AR5 :
***	B	L7	;BRANCH OCCURS
L40:
;>>>> 					break1adr = call_set->DSP_ar5 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(29),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_AR6 :
***	B	L7	;BRANCH OCCURS
L41:
;>>>> 					break1adr = call_set->DSP_ar6 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(30),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_AR7 :
***	B	L7	;BRANCH OCCURS
L42:
;>>>> 					break1adr = call_set->DSP_ar7 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(31),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_DP :
***	B	L7	;BRANCH OCCURS
L43:
;>>>> 					break1adr = call_set->DSP_DP + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(32),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_IR0 :
***	B	L7	;BRANCH OCCURS
L44:
;>>>> 					break1adr = call_set->DSP_IR0 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(33),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_IR1 :
***	B	L7	;BRANCH OCCURS
L45:
;>>>> 					break1adr = call_set->DSP_IR1 + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(34),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_SP :
***	B	L7	;BRANCH OCCURS
L46:
;>>>> 					break1adr = call_set->DSP_SP + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(36),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_ST :
***	B	L7	;BRANCH OCCURS
L47:
;>>>> 					break1adr = call_set->DSP_ST + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(37),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_DIE :
***	B	L7	;BRANCH OCCURS
L48:
;>>>> 					break1adr = call_set->DSP_DIE + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(38),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_IIE :
***	B	L7	;BRANCH OCCURS
L49:
;>>>> 					break1adr = call_set->DSP_IIE + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(39),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
;>>>> 				case C40_IIF :
***	B	L7	;BRANCH OCCURS
L50:
;>>>> 					break1adr = call_set->DSP_IIF + 1;
	LDI	*-FP(2),AR0
	LDI	*+AR0(40),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDI	R3,AR1
	LDI	*AR1,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR1
;>>>> 					break;
***	B	L7	;BRANCH OCCURS
L21:
	LDI	*+FP(5),R0
	AND	01fh,R0
	LDI	R0,IR0
	LDI	@CONST+30,AR0
	CMPI	31,IR0
	LDIHI	32,IR0
	LDI	*+AR0(IR0),AR0
	B	AR0
LL10:
	.word	L22
	.word	L24
	.word	L25
	.word	L26
	.word	L27
	.word	L28
	.word	L29
	.word	L30
	.word	L35
	.word	L36
	.word	L37
	.word	L38
	.word	L39
	.word	L40
	.word	L41
	.word	L42
	.word	L43
	.word	L44
	.word	L45
	.word	L7
	.word	L46
	.word	L47
	.word	L48
	.word	L49
	.word	L50
	.word	L7
	.word	L7
	.word	L7
	.word	L31
	.word	L32
	.word	L33
	.word	L34
	.word	L7
;>>>> 		else if( ((instruction & RPTBim_MASK) == RPTBim) )
L20:
	LDI	*+FP(5),R0
	AND	@CONST+12,R0
	CMPI	@CONST+31,R0
	BNZ	L52
;>>>> 			break1adr = call_set->ret_add + (instruction&0x00FFFFFF) + 2;
	LDI	*+FP(5),R0
	AND	@CONST+15,R0
	LDI	*+AR1(46),R3
	ADDI	R0,R3
	ADDI	2,R3
	STI	R3,*+FP(3)
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDI	R3,AR0
	LDI	*AR0,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR0
;>>>> 		else if( ((instruction & RPTS_MASK) == RPTS) )
***	B	L7	;BRANCH OCCURS
L52:
	LDI	*+FP(5),R0
	AND	@CONST+32,R0
	CMPI	@CONST+33,R0
	BNZ	L54
;>>>> 			break1adr = call_set->ret_add + 2;
	LDI	*+AR1(46),R3
	ADDI	2,R3
	STI	R3,*+FP(3)
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDI	R3,AR0
	LDI	*AR0,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR0
;>>>> 		else if( ((instruction & RPTBDim_MASK) == RPTBDim) )
***	B	L7	;BRANCH OCCURS
L54:
	LDI	*+FP(5),R0
	ANDN	01fh,R0
	CMPI	@CONST+34,R0
	BNZ	L56
;>>>> 			break1adr = call_set->ret_add + (instruction&0x00FFFFFF) + 4;
	LDI	*+FP(5),R0
	AND	@CONST+15,R0
	LDI	*+AR1(46),R3
	ADDI	R0,R3
	ADDI	4,R3
	STI	R3,*+FP(3)
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDI	R3,AR0
	LDI	*AR0,R0
	BD	L7
	STI	R0,*+FP(1)
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR0
;>>>> 		else if( (instruction & TRAPcond_MASK) == TRAPcond )
***	B	L7	;BRANCH OCCURS
L56:
	LDI	*+FP(5),R0
	AND	@CONST+35,R0
	CMPI	@CONST+36,R0
	BNZ	L58
;>>>> 			break1adr = call_set->ret_add + 1;
	LDI	*+AR1(46),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDI	R3,AR0
	LDI	*AR0,R0
	STI	R0,*+FP(1)
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR0
;>>>> 			break2adr = *(unsigned long *)(call_set->DSP_TVTP + (unsigned long)(instruction & 0xFF));
	LDI	*+FP(5),R2
	AND	0ffh,R2
	ADDI	*+AR1(45),R2
	LDI	R2,AR2
	LDI	*AR2,R2
	STI	R2,*+FP(4)
;>>>> 			break2 = *(unsigned long *)break2adr;
	LDI	R2,AR2
	BD	L7
	LDI	*AR2,R3
	STI	R3,*+FP(2)
;>>>> 			*(unsigned long *)break2adr = BREAK_TRAP;
	STI	R1,*AR2
;>>>> 		else if( (instruction & LATcond_MASK) == LATcond )
***	B	L7	;BRANCH OCCURS
L58:
	LDI	*+FP(5),R0
	AND	@CONST+23,R0
	CMPI	@CONST+37,R0
	BNZ	L60
;>>>> 			break1adr = call_set->ret_add + 4;
	LDI	*+AR1(46),R3
	ADDI	4,R3
	STI	R3,*+FP(3)
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDI	R3,AR0
	LDI	*AR0,R0
	STI	R0,*+FP(1)
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR0
;>>>> 			break2adr = *(unsigned long *)(call_set->DSP_TVTP + (unsigned long)(instruction & 0xFF));
	LDI	*+FP(5),R2
	AND	0ffh,R2
	ADDI	*+AR1(45),R2
	LDI	R2,AR2
	LDI	*AR2,R2
	STI	R2,*+FP(4)
;>>>> 			break2 = *(unsigned long *)break2adr;
	LDI	R2,AR2
	BD	L7
	LDI	*AR2,R3
	STI	R3,*+FP(2)
;>>>> 			*(unsigned long *)break2adr = BREAK_TRAP;
	STI	R1,*AR2
;>>>> 		else
***	B	L7	;BRANCH OCCURS
L60:
;>>>> 			break1adr = call_set->ret_add + 1;
	LDI	*+AR1(46),R3
	ADDI	1,R3
	STI	R3,*+FP(3)
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDI	R3,AR0
	LDI	*AR0,R0
	STI	R0,*+FP(1)
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R1
	STI	R1,*AR0
L7:
;>>>> 		if( reg_mode )
	LDI	*+FP(6),R0
	BZ	L62
;>>>> 			switch( instruction & 0xFF )
;>>>> 				case C40_R0 :
	B	L63
L64:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_R1 :
***	B	L62	;BRANCH OCCURS
L66:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_R2 :
***	B	L62	;BRANCH OCCURS
L67:
;>>>> 					break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_R3 :
***	B	L62	;BRANCH OCCURS
L68:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_R4 :
***	B	L62	;BRANCH OCCURS
L69:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_R5 :
***	B	L62	;BRANCH OCCURS
L70:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_R6 :
***	B	L62	;BRANCH OCCURS
L71:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_R7 :
***	B	L62	;BRANCH OCCURS
L72:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_R8 :
***	B	L62	;BRANCH OCCURS
L73:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_R9 :
***	B	L62	;BRANCH OCCURS
L74:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_R10 :
***	B	L62	;BRANCH OCCURS
L75:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_R11 :
***	B	L62	;BRANCH OCCURS
L76:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_AR0 :
***	B	L62	;BRANCH OCCURS
L77:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_AR1 :
***	B	L62	;BRANCH OCCURS
L78:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_AR2 :
***	B	L62	;BRANCH OCCURS
L79:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_AR3 :
***	B	L62	;BRANCH OCCURS
L80:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_AR4 :
***	B	L62	;BRANCH OCCURS
L81:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_AR5 :
***	B	L62	;BRANCH OCCURS
L82:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_AR6 :
***	B	L62	;BRANCH OCCURS
L83:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_AR7 :
***	B	L62	;BRANCH OCCURS
L84:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_DP :
***	B	L62	;BRANCH OCCURS
L85:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_IR0 :
***	B	L62	;BRANCH OCCURS
L86:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_IR1 :
***	B	L62	;BRANCH OCCURS
L87:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_SP :
***	B	L62	;BRANCH OCCURS
L88:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_ST :
***	B	L62	;BRANCH OCCURS
L89:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_DIE :
***	B	L62	;BRANCH OCCURS
L90:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_IIE :
***	B	L62	;BRANCH OCCURS
L91:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
;>>>> 				case C40_IIF :
***	B	L62	;BRANCH OCCURS
L92:
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDI	R0,AR1
	LDI	*AR1,R1
	BD	L62
	STI	R1,*+FP(2)
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
;>>>> 					break;
***	B	L62	;BRANCH OCCURS
L63:
	LDI	*+FP(5),R1
	AND	0ffh,R1
	LDI	R1,IR0
	LDI	@CONST+38,AR0
	CMPI	31,IR0
	LDIHI	32,IR0
	LDI	*+AR0(IR0),AR0
	B	AR0
LL12:
	.word	L64
	.word	L66
	.word	L67
	.word	L68
	.word	L69
	.word	L70
	.word	L71
	.word	L72
	.word	L77
	.word	L78
	.word	L79
	.word	L80
	.word	L81
	.word	L82
	.word	L83
	.word	L84
	.word	L85
	.word	L86
	.word	L87
	.word	L62
	.word	L88
	.word	L89
	.word	L90
	.word	L91
	.word	L92
	.word	L62
	.word	L62
	.word	L62
	.word	L73
	.word	L74
	.word	L75
	.word	L76
	.word	L62
L62:
;>>>> 		run( BREAK_TRAP_NUM );
	LDI	511,R0
	PUSH	R0
	CALL	_run
	SUBI	1,SP
;>>>> 	   if( *(unsigned long *)(call_set->ret_add-1) == BREAK_TRAP )
	LDI	*-FP(2),AR0
	LDI	*+AR0(46),AR1
	LDI	*-AR1(1),R0
	CMPI	@CONST+36,R0
	LDIU	0,R0
	LDIZ	1,R0
	OR	01ffh,R0
	BZ	L93
;>>>> 			call_set->ret_add--;
	LDI	*+AR0(46),R0
	SUBI	1,R0
	STI	R0,*+AR0(46)
L93:
;>>>> 		*(unsigned long *)break1adr = break1;
	LDI	*+FP(3),AR1
	LDI	*+FP(1),R0
	STI	R0,*AR1
;>>>> 		if( break2adr != NO_BREAK2 )
	LDI	*+FP(4),R1
	CMPI	@CONST+0,R1
	BZ	L94
;>>>> 			*(unsigned long *)break2adr = break2;
	LDI	R1,AR2
	LDI	*+FP(2),R2
	STI	R2,*AR2
L94:
;>>>> 		if( is_at_break != MAX_BREAKS )
	LDI	*+FP(8),R2
	CMPI	8,R2
	BZ	L95
;>>>> 			*(unsigned long *)brk_addrs[is_at_break] = BREAK_TRAP;
	ADDI	@CONST+1,R2
	LDI	R2,AR2
	LDI	*AR2,AR2
	LDI	@CONST+10,R2
	STI	R2,*AR2
L95:
;>>>> 		reg_dump( *call_set, 't' );
	LDI	116,R2
	PUSH	R2
	LDI	SP,AR2
	ADDI	47,SP
	LDI	*AR0++,R3
	RPTS	46
	STI	R3,*++AR2
    ||	LDI	*AR0++,R3
	CALL	_reg_dump
	SUBI	48,SP
EPI0_1:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	10,SP
***	B	R1	;BRANCH OCCURS
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,39
	.sect	".cinit"
	.word	39,CONST
	.word 	1048832          ;0
	.word 	_brk_addrs       ;1
	.word 	_breaks          ;2
	.word 	0fde00000h       ;3
	.word 	1755316224       ;4
	.word 	1751121920       ;5
	.word 	1746927616       ;6
	.word 	0fc200000h       ;7
	.word 	1814036480       ;8
	.word 	1881145344       ;9
	.word 	1946157567       ;10
	.word 	33554432         ;11
	.word 	0ff000000h       ;12
	.word 	1627389952       ;13
	.word 	1660944384       ;14
	.word 	0ffffffh         ;15
	.word 	0fd000000h       ;16
	.word 	1744830464       ;17
	.word 	1879048192       ;18
	.word 	0fc000000h       ;19
	.word 	1811939328       ;20
	.word 	1644167168       ;21
	.word 	1610612736       ;22
	.word 	0ffe00000h       ;23
	.word 	2015363072       ;24
	.word 	0ffe0ffffh       ;25
	.word 	2013265920       ;26
	.word 	2021654528       ;27
	.word 	2030043136       ;28
	.word 	1694498816       ;29
	.word 	LL10             ;30
	.word 	1677721600       ;31
	.word 	0ff9f0000h       ;32
	.word 	328925184        ;33
	.word 	2038431744       ;34
	.word 	0ffe0fe00h       ;35
	.word 	1946157056       ;36
	.word 	1954545664       ;37
	.word 	LL12             ;38
	.end
