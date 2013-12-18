******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 step.c step.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o step.if step.asm step.tmp 
	.version	40
FP	.set		AR3
	.file	"step.c"
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
	.file	"step.c"
	.globl	_breaks
	.globl	_brk_addrs

	.sym	_step,_step,32,2,0
	.globl	_step

	.func	7
;>>>> 	void step( reg_set *call_set )
******************************************************
* FUNCTION DEF : _step
******************************************************
_step:
	PUSH	FP
	LDI	SP,FP
	ADDI	8,SP
	.sym	_call_set,-2,24,9,32,.fake4
	.sym	_break1,1,15,1,32
	.sym	_break2,2,15,1,32
	.sym	_break1adr,3,15,1,32
	.sym	_break2adr,4,15,1,32
	.sym	_instruction,5,15,1,32
	.sym	_reg_mode,6,4,1,32
	.sym	_i,7,4,1,32
	.sym	_is_at_break,8,4,1,32
	.line	2
;>>>> 		unsigned long break1, break2;
	.line	4
;>>>> 		unsigned long break1adr, break2adr=NO_BREAK2;
	LDI	@CONST+0,R0
	STI	R0,*+FP(4)
	.line	5
;>>>> 		unsigned long instruction = *(unsigned long *)call_set->ret_add;
	LDA	*-FP(2),AR0
	LDA	*+AR0(46),AR1
	LDI	*AR1,R1
	STI	R1,*+FP(5)
	.line	6
;>>>> 		int reg_mode=FALSE;
;>>>> 		int i;
	STIK	0,*+FP(6)
	.line	8
;>>>> 		int is_at_break=MAX_BREAKS;
	STIK	8,*+FP(8)
	.line	11
;>>>> 		for( i=0 ; i < MAX_BREAKS ; i++ )
	STIK	0,*+FP(7)
	CMPI	8,*+FP(7)
	BGE	L2
L1:
	.line	12
;>>>> 			if( call_set->ret_add == brk_addrs[i] )
	LDA	*-FP(2),AR0
	LDA	*+FP(7),IR0
	LDA	@CONST+1,AR1
	LDI	*+AR0(46),R0
	CMPI	*+AR1(IR0),R0
	BNZ	L3
	.line	14
;>>>> 				is_at_break = i;
	LDI	*+FP(7),R0
	STI	R0,*+FP(8)
	.line	15
;>>>> 				*(unsigned long *)brk_addrs[i] = breaks[i];
	LDA	@CONST+2,AR2
	LDA	*+AR1(IR0),AR1
	LDI	*+AR2(IR0),R1
	STI	R1,*AR1
	.line	16
;>>>> 				break;
	B	L2
L3:
	.line	11
	ADDI	1,*+FP(7),R0
	STI	R0,*+FP(7)
	CMPI	8,R0
	BLT	L1
L2:
	.line	19
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
	.line	26
;>>>> 			break1adr = call_set->ret_add + 4;
	LDI	*+AR0(46),R0
	ADDI	4,R0
	STI	R0,*+FP(3)
	.line	27
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	28
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	30
;>>>> 			if( instruction & 0x02000000 ) /* Check for PC-Relative branch */
	LDI	@CONST+11,R3
	TSTB	R3,*+FP(5)
	BZ	L5
	.line	32
;>>>> 				break2adr = call_set->ret_add + (int)(instruction & 0xFFFF) + 3;
	LDI	*+FP(5),R9
	AND	0ffffh,R9
	ADDI	*+AR0(46),R9
	ADDI	3,R9
	STI	R9,*+FP(4)
	.line	33
;>>>> 				break2 = *(unsigned long *)break2adr;
	LDA	R9,AR2
	LDI	*AR2,R10
	STI	R10,*+FP(2)
	.line	34
;>>>> 				*(unsigned long *)break2adr = BREAK_TRAP;
;>>>> 			else /* Register mode */
	STI	R2,*AR2
	B	L6
L5:
	.line	38
;>>>> 				reg_mode = TRUE;
	STIK	1,*+FP(6)
L6:
	B	L7
L4:
	.line	41
;>>>> 		else if( ((instruction & BRD_MASK) == BRD)
;>>>> 				|| ((instruction & LAJ_MASK) == LAJ)
;>>>> 			  )
	LDI	*+FP(5),R0
	AND	@CONST+12,R0
	CMPI	@CONST+13,R0
	BZ	LL4
	LDI	*+FP(5),R0
	AND	@CONST+12,R0
	CMPI	@CONST+14,R0
	BNZ	L8
LL4:
	.line	45
;>>>> 			break1adr = call_set->ret_add + (int)(instruction & 0xFFFFFF) + 3;
	LDI	*+FP(5),R0
	AND	@CONST+15,R0
	ADDI	*+AR0(46),R0
	ADDI	3,R0
	STI	R0,*+FP(3)
	.line	46
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	47
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	B	L7
L8:
	.line	49
;>>>> 		else if( ((instruction & Bcond_MASK) == Bcond)
;>>>> 				|| ((instruction & CALLcond_MASK) == CALLcond)
;>>>> 				|| ((instruction & DBcond_MASK) == DBcond)
;>>>> 				)
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
	BNZ	L9
LL5:
	.line	54
;>>>> 	   	break1adr = call_set->ret_add + 1;
	LDI	*+AR0(46),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	55
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	56
;>>>> 			*(unsigned long *)(call_set->ret_add + 4) = BREAK_TRAP;
	LDA	*+AR0(46),AR2
	LDI	@CONST+10,R2
	STI	R2,*+AR2(4)
	.line	58
;>>>> 			if( instruction & 0x02000000 ) /* Check for PC-Relative branch */
	LDI	@CONST+11,R3
	TSTB	R3,*+FP(5)
	BZ	L10
	.line	60
;>>>> 				break2adr = call_set->ret_add + (int)(instruction & 0xFFFF) + 1;
	LDI	*+FP(5),R9
	AND	0ffffh,R9
	ADDI	*+AR0(46),R9
	ADDI	1,R9
	STI	R9,*+FP(4)
	.line	61
;>>>> 				break2 = *(unsigned long *)break2adr;
	LDA	R9,AR2
	LDI	*AR2,R10
	STI	R10,*+FP(2)
	.line	62
;>>>> 				*(unsigned long *)break2adr = BREAK_TRAP;
;>>>> 			else /* Register mode */
	STI	R2,*AR2
	B	L11
L10:
	.line	66
;>>>> 				reg_mode = TRUE;
	STIK	1,*+FP(6)
L11:
	B	L7
L9:
	.line	69
;>>>> 		else if( ((instruction & CALL_MASK) == CALL)
;>>>> 				|| ((instruction & BR_MASK) == BR)
;>>>> 				)
	LDI	*+FP(5),R0
	AND	@CONST+12,R0
	CMPI	@CONST+21,R0
	BZ	LL6
	LDI	*+FP(5),R0
	AND	@CONST+12,R0
	CMPI	@CONST+22,R0
	BNZ	L12
LL6:
	.line	73
;>>>> 			break1adr = call_set->ret_add + (int)(instruction & 0xFFFFFF) + 1;
	LDI	*+FP(5),R0
	AND	@CONST+15,R0
	ADDI	*+AR0(46),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	74
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	75
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	B	L7
L12:
	.line	77
;>>>> 		else if( ((instruction & RETIcondD_MASK) == RETIcondD) )
	LDI	*+FP(5),R0
	AND	@CONST+23,R0
	CMPI	@CONST+24,R0
	BNZ	L13
	.line	79
;>>>> 	   	break1adr = call_set->ret_add + 4;
	LDI	*+AR0(46),R0
	ADDI	4,R0
	STI	R0,*+FP(3)
	.line	80
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	81
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	83
;>>>> 			break2adr = *(unsigned long *)call_set->DSP_SP;
	LDA	*+AR0(36),AR2
	LDI	*AR2,R3
	STI	R3,*+FP(4)
	.line	84
;>>>> 			break2 = *(unsigned long *)break2adr;
	LDA	R3,AR2
	LDI	*AR2,R9
	STI	R9,*+FP(2)
	.line	85
;>>>> 			*(unsigned long *)break2adr = BREAK_TRAP;
	STI	R2,*AR2
	B	L7
L13:
	.line	87
;>>>> 	   else if( ((instruction & RETIcond_MASK) == RETIcond)
;>>>> 				|| ((instruction & RETScond_MASK) == RETScond)
;>>>> 				)
	LDI	*+FP(5),R0
	AND	@CONST+25,R0
	CMPI	@CONST+26,R0
	BZ	LL7
	LDI	*+FP(5),R0
	AND	@CONST+25,R0
	CMPI	@CONST+27,R0
	BNZ	L14
LL7:
	.line	91
;>>>> 			break1adr = call_set->ret_add + 1;
	LDI	*+AR0(46),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	92
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	93
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	95
;>>>> 			break2adr = *(unsigned long *)call_set->DSP_SP;
	LDA	*+AR0(36),AR2
	LDI	*AR2,R3
	STI	R3,*+FP(4)
	.line	96
;>>>> 			break2 = *(unsigned long *)break2adr;
	LDA	R3,AR2
	LDI	*AR2,R9
	STI	R9,*+FP(2)
	.line	97
;>>>> 			*(unsigned long *)break2adr = BREAK_TRAP;
	STI	R2,*AR2
	B	L7
L14:
	.line	99
;>>>> 		else if( ((instruction & RPTBreg_MASK) == RPTBreg)
;>>>> 				|| ((instruction & RPTBDreg_MASK) == RPTBDreg)
;>>>> 				)
;>>>> 			switch( instruction & 0x1F )
;>>>> 				case C40_R0 :
	LDI	*+FP(5),R0
	ANDN	01fh,R0
	CMPI	@CONST+28,R0
	BZ	LL8
	LDI	*+FP(5),R0
	AND	@CONST+12,R0
	CMPI	@CONST+29,R0
	BNZ	L15
LL8:
	B	L16
L17:
	.line	106
;>>>> 					break1adr = call_set->DSP_ir0 + 1;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	107
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	108
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	109
;>>>> 					break;
;>>>> 				case C40_R1 :
	B	L18
L19:
	.line	111
;>>>> 					break1adr = call_set->DSP_ir1 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(1),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	112
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	113
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	114
;>>>> 					break;
;>>>> 				case C40_R2 :
	B	L18
L20:
	.line	116
;>>>> 					break1adr = call_set->DSP_ir2 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(2),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	117
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	118
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	119
;>>>> 					break;
;>>>> 				case C40_R3 :
	B	L18
L21:
	.line	121
;>>>> 					break1adr = call_set->DSP_ir3 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(3),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	122
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	123
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	124
;>>>> 					break;
;>>>> 				case C40_R4 :
	B	L18
L22:
	.line	126
;>>>> 					break1adr = call_set->DSP_ir4 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(4),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	127
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	128
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	129
;>>>> 					break;
;>>>> 				case C40_R5 :
	B	L18
L23:
	.line	131
;>>>> 					break1adr = call_set->DSP_ir5 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(5),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	132
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	133
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	134
;>>>> 					break;
;>>>> 				case C40_R6 :
	B	L18
L24:
	.line	136
;>>>> 					break1adr = call_set->DSP_ir6 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(6),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	137
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	138
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	139
;>>>> 					break;
;>>>> 				case C40_R7 :
	B	L18
L25:
	.line	141
;>>>> 					break1adr = call_set->DSP_ir7 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(7),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	142
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	143
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	144
;>>>> 					break;
;>>>> 				case C40_R8 :
	B	L18
L26:
	.line	146
;>>>> 					break1adr = call_set->DSP_ir8 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(8),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	147
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	148
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	149
;>>>> 					break;
;>>>> 				case C40_R9 :
	B	L18
L27:
	.line	151
;>>>> 					break1adr = call_set->DSP_ir9 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(9),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	152
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	153
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	154
;>>>> 					break;
;>>>> 				case C40_R10 :
	B	L18
L28:
	.line	156
;>>>> 					break1adr = call_set->DSP_ir10 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(10),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	157
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	158
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	159
;>>>> 					break;
;>>>> 				case C40_R11 :
	B	L18
L29:
	.line	161
;>>>> 					break1adr = call_set->DSP_ir11 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(11),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	162
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	163
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	164
;>>>> 					break;
;>>>> 				case C40_AR0 :
	B	L18
L30:
	.line	166
;>>>> 					break1adr = call_set->DSP_ar0 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(24),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	167
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	168
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	169
;>>>> 					break;
;>>>> 				case C40_AR1 :
	B	L18
L31:
	.line	171
;>>>> 					break1adr = call_set->DSP_ar1 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(25),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	172
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	173
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	174
;>>>> 					break;
;>>>> 				case C40_AR2 :
	B	L18
L32:
	.line	176
;>>>> 					break1adr = call_set->DSP_ar2 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(26),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	177
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	178
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	179
;>>>> 					break;
;>>>> 				case C40_AR3 :
	B	L18
L33:
	.line	181
;>>>> 					break1adr = call_set->DSP_ar3 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(27),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	182
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	183
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	184
;>>>> 					break;
;>>>> 				case C40_AR4 :
	B	L18
L34:
	.line	186
;>>>> 					break1adr = call_set->DSP_ar4 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(28),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	187
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	188
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	189
;>>>> 					break;
;>>>> 				case C40_AR5 :
	B	L18
L35:
	.line	191
;>>>> 					break1adr = call_set->DSP_ar5 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(29),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	192
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	193
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	194
;>>>> 					break;
;>>>> 				case C40_AR6 :
	B	L18
L36:
	.line	196
;>>>> 					break1adr = call_set->DSP_ar6 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(30),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	197
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	198
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	199
;>>>> 					break;
;>>>> 				case C40_AR7 :
	B	L18
L37:
	.line	201
;>>>> 					break1adr = call_set->DSP_ar7 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(31),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	202
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	203
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	204
;>>>> 					break;
;>>>> 				case C40_DP :
	B	L18
L38:
	.line	206
;>>>> 					break1adr = call_set->DSP_DP + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(32),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	207
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	208
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	209
;>>>> 					break;
;>>>> 				case C40_IR0 :
	B	L18
L39:
	.line	211
;>>>> 					break1adr = call_set->DSP_IR0 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(33),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	212
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	213
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	214
;>>>> 					break;
;>>>> 				case C40_IR1 :
	B	L18
L40:
	.line	216
;>>>> 					break1adr = call_set->DSP_IR1 + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(34),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	217
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	218
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	219
;>>>> 					break;
;>>>> 				case C40_SP :
	B	L18
L41:
	.line	221
;>>>> 					break1adr = call_set->DSP_SP + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(36),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	222
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	223
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	224
;>>>> 					break;
;>>>> 				case C40_ST :
	B	L18
L42:
	.line	226
;>>>> 					break1adr = call_set->DSP_ST + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(37),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	227
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	228
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	229
;>>>> 					break;
;>>>> 				case C40_DIE :
	B	L18
L43:
	.line	231
;>>>> 					break1adr = call_set->DSP_DIE + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(38),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	232
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	233
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	234
;>>>> 					break;
;>>>> 				case C40_IIE :
	B	L18
L44:
	.line	236
;>>>> 					break1adr = call_set->DSP_IIE + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(39),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	237
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	238
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	239
;>>>> 					break;
;>>>> 				case C40_IIF :
	B	L18
L45:
	.line	241
;>>>> 					break1adr = call_set->DSP_IIF + 1;
	LDA	*-FP(2),AR0
	LDI	*+AR0(40),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	242
;>>>> 					break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	243
;>>>> 					*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	244
;>>>> 					break;
	B	L18
L16:
	.line	103
	LDA	*+FP(5),IR1
	AND	01fh,IR1
	LDA	@CONST+30,AR1
	CMPI	31,IR1
	LDIHI	32,IR1
	LDA	*+AR1(IR1),AR1
	B	AR1
	.sect	".const"
LL10:
	.word	L17
	.word	L19
	.word	L20
	.word	L21
	.word	L22
	.word	L23
	.word	L24
	.word	L25
	.word	L30
	.word	L31
	.word	L32
	.word	L33
	.word	L34
	.word	L35
	.word	L36
	.word	L37
	.word	L38
	.word	L39
	.word	L40
	.word	L18
	.word	L41
	.word	L42
	.word	L43
	.word	L44
	.word	L45
	.word	L18
	.word	L18
	.word	L18
	.word	L26
	.word	L27
	.word	L28
	.word	L29
	.word	L18
	.text
L18:
	B	L7
L15:
	.line	247
;>>>> 		else if( ((instruction & RPTBim_MASK) == RPTBim) )
	LDI	*+FP(5),R0
	AND	@CONST+12,R0
	CMPI	@CONST+31,R0
	BNZ	L46
	.line	249
;>>>> 			break1adr = call_set->ret_add + (instruction&0x00FFFFFF) + 2;
	LDI	*+FP(5),R0
	AND	@CONST+15,R0
	ADDI	*+AR0(46),R0
	ADDI	2,R0
	STI	R0,*+FP(3)
	.line	250
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	251
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	B	L7
L46:
	.line	253
;>>>> 		else if( ((instruction & RPTS_MASK) == RPTS) )
	LDI	*+FP(5),R0
	AND	@CONST+32,R0
	CMPI	@CONST+33,R0
	BNZ	L47
	.line	255
;>>>> 			break1adr = call_set->ret_add + 2;
	LDI	*+AR0(46),R0
	ADDI	2,R0
	STI	R0,*+FP(3)
	.line	256
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	257
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	B	L7
L47:
	.line	259
;>>>> 		else if( ((instruction & RPTBDim_MASK) == RPTBDim) )
	LDI	*+FP(5),R0
	ANDN	01fh,R0
	CMPI	@CONST+34,R0
	BNZ	L48
	.line	261
;>>>> 			break1adr = call_set->ret_add + (instruction&0x00FFFFFF) + 4;
	LDI	*+FP(5),R0
	AND	@CONST+15,R0
	ADDI	*+AR0(46),R0
	ADDI	4,R0
	STI	R0,*+FP(3)
	.line	262
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	263
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	B	L7
L48:
	.line	265
;>>>> 		else if( (instruction & TRAPcond_MASK) == TRAPcond )
	LDI	*+FP(5),R0
	AND	@CONST+35,R0
	CMPI	@CONST+36,R0
	BNZ	L49
	.line	267
;>>>> 			break1adr = call_set->ret_add + 1;
	LDI	*+AR0(46),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	268
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	269
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	271
;>>>> 			break2adr = *(unsigned long *)(call_set->DSP_TVTP + (unsigned long)(instruction & 0xFF));
	LDA	*+AR0(45),AR2
	LDA	*+FP(5),IR0
	AND	0ffh,IR0
	LDI	*+AR2(IR0),R3
	STI	R3,*+FP(4)
	.line	272
;>>>> 			break2 = *(unsigned long *)break2adr;
	LDA	R3,AR2
	LDI	*AR2,R9
	STI	R9,*+FP(2)
	.line	273
;>>>> 			*(unsigned long *)break2adr = BREAK_TRAP;
	STI	R2,*AR2
	B	L7
L49:
	.line	275
;>>>> 		else if( (instruction & LATcond_MASK) == LATcond )
	LDI	*+FP(5),R0
	AND	@CONST+23,R0
	CMPI	@CONST+37,R0
	BNZ	L50
	.line	277
;>>>> 			break1adr = call_set->ret_add + 4;
	LDI	*+AR0(46),R0
	ADDI	4,R0
	STI	R0,*+FP(3)
	.line	278
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	279
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	281
;>>>> 			break2adr = *(unsigned long *)(call_set->DSP_TVTP + (unsigned long)(instruction & 0xFF));
	LDA	*+AR0(45),AR2
	LDA	*+FP(5),IR1
	AND	0ffh,IR1
	LDI	*+AR2(IR1),R3
	STI	R3,*+FP(4)
	.line	282
;>>>> 			break2 = *(unsigned long *)break2adr;
	LDA	R3,AR2
	LDI	*AR2,R9
	STI	R9,*+FP(2)
	.line	283
;>>>> 			*(unsigned long *)break2adr = BREAK_TRAP;
;>>>> 		else
	STI	R2,*AR2
	B	L7
L50:
	.line	287
;>>>> 			break1adr = call_set->ret_add + 1;
	LDI	*+AR0(46),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
	.line	288
;>>>> 			break1 = *(unsigned long *)break1adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(1)
	.line	289
;>>>> 			*(unsigned long *)break1adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
L7:
	.line	292
;>>>> 		if( reg_mode )
;>>>> 			switch( instruction & 0xFF )
;>>>> 				case C40_R0 :
	LDI	*+FP(6),R0
	BZ	L51
	B	L52
L53:
	.line	297
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	298
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	299
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	300
;>>>> 					break;
;>>>> 				case C40_R1 :
	B	L51
L55:
	.line	302
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	303
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	304
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	305
;>>>> 					break;
;>>>> 				case C40_R2 :
	B	L51
L56:
	.line	307
;>>>> 					break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	308
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	309
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	310
;>>>> 					break;
;>>>> 				case C40_R3 :
	B	L51
L57:
	.line	312
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	313
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	314
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	315
;>>>> 					break;
;>>>> 				case C40_R4 :
	B	L51
L58:
	.line	317
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	318
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	319
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	320
;>>>> 					break;
;>>>> 				case C40_R5 :
	B	L51
L59:
	.line	322
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	323
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	324
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	325
;>>>> 					break;
;>>>> 				case C40_R6 :
	B	L51
L60:
	.line	327
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	328
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	329
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	330
;>>>> 					break;
;>>>> 				case C40_R7 :
	B	L51
L61:
	.line	332
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	333
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	334
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	335
;>>>> 					break;
;>>>> 				case C40_R8 :
	B	L51
L62:
	.line	337
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	338
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	339
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	340
;>>>> 					break;
;>>>> 				case C40_R9 :
	B	L51
L63:
	.line	342
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	343
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	344
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	345
;>>>> 					break;
;>>>> 				case C40_R10 :
	B	L51
L64:
	.line	347
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	348
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	349
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	350
;>>>> 					break;
;>>>> 				case C40_R11 :
	B	L51
L65:
	.line	352
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	353
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	354
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	355
;>>>> 					break;
;>>>> 				case C40_AR0 :
	B	L51
L66:
	.line	357
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	358
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	359
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	360
;>>>> 					break;
;>>>> 				case C40_AR1 :
	B	L51
L67:
	.line	362
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	363
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	364
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	365
;>>>> 					break;
;>>>> 				case C40_AR2 :
	B	L51
L68:
	.line	367
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	368
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	369
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	370
;>>>> 					break;
;>>>> 				case C40_AR3 :
	B	L51
L69:
	.line	372
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	373
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	374
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	375
;>>>> 					break;
;>>>> 				case C40_AR4 :
	B	L51
L70:
	.line	377
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	378
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	379
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	380
;>>>> 					break;
;>>>> 				case C40_AR5 :
	B	L51
L71:
	.line	382
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	383
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	384
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	385
;>>>> 					break;
;>>>> 				case C40_AR6 :
	B	L51
L72:
	.line	387
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	388
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	389
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	390
;>>>> 					break;
;>>>> 				case C40_AR7 :
	B	L51
L73:
	.line	392
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	393
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	394
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	395
;>>>> 					break;
;>>>> 				case C40_DP :
	B	L51
L74:
	.line	397
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	398
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	399
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	400
;>>>> 					break;
;>>>> 				case C40_IR0 :
	B	L51
L75:
	.line	402
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	403
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	404
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	405
;>>>> 					break;
;>>>> 				case C40_IR1 :
	B	L51
L76:
	.line	407
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	408
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	409
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	410
;>>>> 					break;
;>>>> 				case C40_SP :
	B	L51
L77:
	.line	412
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	413
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	414
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	415
;>>>> 					break;
;>>>> 				case C40_ST :
	B	L51
L78:
	.line	417
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	418
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	419
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	420
;>>>> 					break;
;>>>> 				case C40_DIE :
	B	L51
L79:
	.line	422
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	423
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	424
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	425
;>>>> 					break;
;>>>> 				case C40_IIE :
	B	L51
L80:
	.line	427
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	428
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	429
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	430
;>>>> 					break;
;>>>> 				case C40_IIF :
	B	L51
L81:
	.line	432
;>>>> 	         	break2adr = call_set->DSP_ir0;
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(4)
	.line	433
;>>>> 					break2 = *(unsigned long *)break2adr;
	LDA	R0,AR1
	LDI	*AR1,R1
	STI	R1,*+FP(2)
	.line	434
;>>>> 					*(unsigned long *)break2adr = BREAK_TRAP;
	LDI	@CONST+10,R2
	STI	R2,*AR1
	.line	435
;>>>> 					break;
	B	L51
L52:
	.line	294
	LDA	*+FP(5),IR0
	AND	0ffh,IR0
	LDA	@CONST+38,AR0
	CMPI	31,IR0
	LDIHI	32,IR0
	LDA	*+AR0(IR0),AR0
	B	AR0
	.sect	".const"
LL12:
	.word	L53
	.word	L55
	.word	L56
	.word	L57
	.word	L58
	.word	L59
	.word	L60
	.word	L61
	.word	L66
	.word	L67
	.word	L68
	.word	L69
	.word	L70
	.word	L71
	.word	L72
	.word	L73
	.word	L74
	.word	L75
	.word	L76
	.word	L51
	.word	L77
	.word	L78
	.word	L79
	.word	L80
	.word	L81
	.word	L51
	.word	L51
	.word	L51
	.word	L62
	.word	L63
	.word	L64
	.word	L65
	.word	L51
	.text
L51:
	.line	439
;>>>> 		run( BREAK_TRAP_NUM );
	LDI	511,R0
	PUSH	R0
	CALL	_run
	SUBI	1,SP
	.line	441
;>>>> 	   if( *(unsigned long *)(call_set->ret_add-1) == BREAK_TRAP )
	LDA	*-FP(2),AR0
	LDA	*+AR0(46),AR1
	LDI	@CONST+36,R0
	CMPI	R0,*-AR1(1)
	LDIU	0,R1
	LDIZ	1,R1
	OR	01ffh,R1
	BZ	L82
	.line	442
;>>>> 			call_set->ret_add--;
	LDI	*+AR0(46),R1
	SUBI	1,R1
	STI	R1,*+AR0(46)
L82:
	.line	444
;>>>> 		*(unsigned long *)break1adr = break1;
	LDA	*+FP(3),AR1
	LDI	*+FP(1),R1
	STI	R1,*AR1
	.line	445
;>>>> 		if( break2adr != NO_BREAK2 )
	LDI	@CONST+0,R2
	CMPI	*+FP(4),R2
	BZ	L83
	.line	446
;>>>> 			*(unsigned long *)break2adr = break2;
	LDA	*+FP(4),AR2
	LDI	*+FP(2),R3
	STI	R3,*AR2
L83:
	.line	448
;>>>> 		if( is_at_break != MAX_BREAKS )
	CMPI	8,*+FP(8)
	BZ	L84
	.line	449
;>>>> 			*(unsigned long *)brk_addrs[is_at_break] = BREAK_TRAP;
	LDA	*+FP(8),IR1
	LDA	@CONST+1,AR2
	LDA	*+AR2(IR1),AR2
	LDI	@CONST+10,R3
	STI	R3,*AR2
L84:
	.line	451
;>>>> 		reg_dump( *call_set, 't' );
	LDI	116,R3
	PUSH	R3
	LDA	SP,AR2
	ADDI	47,SP
	LDI	*AR0++,R3
	RPTS	46
	STI	R3,*++AR2
    ||	LDI	*AR0++,R3
	CALL	_reg_dump
	SUBI	48,SP
EPI0_1:
	.line	452
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	10,SP
	B	R1
	.endfunc	458,000000000H,8
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
