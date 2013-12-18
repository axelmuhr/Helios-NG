******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 set_brk.c set_brk.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o set_brk.if set_brk.asm set_brk.tmp 
	.version	40
FP	.set		AR3
	.file	"set_brk.c"
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
	.file	"set_brk.c"
	.globl	_breaks
	.globl	_brk_addrs

	.sym	_set_brk,_set_brk,36,2,0
	.globl	_set_brk

	.func	7
;>>>> 	int set_brk( unsigned long parms[], char put_where )
******************************************************
* FUNCTION DEF : _set_brk
******************************************************
_set_brk:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_parms,-2,31,9,32
	.sym	_put_where,-3,2,9,32
	.sym	_i,1,4,1,32
	.line	2
;>>>> 		int i;
	.line	5
;>>>> 		for( i=0 ; (i < MAX_BREAKS) && breaks[i] ; i++ );
	STIK	0,*+FP(1)
	B	L2
L1:
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
L2:
	CMPI	8,*+FP(1)
	BGE	LL3
	LDA	*+FP(1),IR0
	LDA	@CONST+0,AR0
	LDI	*+AR0(IR0),R0
	BNZ	L1
LL3:
	.line	7
;>>>> 		if( i == MAX_BREAKS )
	CMPI	8,*+FP(1)
	BNZ	L3
	.line	9
;>>>> 			c40_printf( "Exceeded maximum number of breakpoints, no breakpoint set.\n" );
	LDI	@CONST+1,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	10
;>>>> 			return( FAILURE );
	LDI	0,R0
	B	EPI0_1
L3:
	.line	15
;>>>> 		for( i=0 ; i < 3 ; i++ )
	STIK	0,*+FP(1)
	CMPI	3,*+FP(1)
	BGE	L5
L4:
	.line	17
;>>>> 			if( ((*(unsigned long *)(parms[0]-i) & BcondAF_MASK) == BcondAF)
;>>>> 			 || ((*(unsigned long *)(parms[0]-i) & BcondAT_MASK) == BcondAT)
;>>>> 			 || ((*(unsigned long *)(parms[0]-i) & BcondD_MASK) == BcondD)
;>>>> 			 || ((*(unsigned long *)(parms[0]-i) & BRD_MASK) == BRD)
;>>>> 			 || ((*(unsigned long *)(parms[0]-i) & DBcondD_MASK) == DBcondD)
;>>>> 			 || ((*(unsigned long *)(parms[0]-i) & RETIcondD_MASK) == RETIcondD)
;>>>> 			 || ((*(unsigned long *)(parms[0]-i) & RPTBDreg_MASK) == RPTBDreg)
;>>>> 			 || ((*(unsigned long *)(parms[0]-i) & RPTBDim_MASK) == RPTBDim)
;>>>> 			 )
	LDA	*-FP(2),AR0
	LDA	*AR0,AR1
	LDA	*+FP(1),IR1
	LDI	@CONST+2,R0
	AND	R0,*-AR1(IR1),R1
	CMPI	@CONST+3,R1
	BZ	LL4
	LDA	*AR0,AR1
	AND	R0,*-AR1(IR1),R1
	CMPI	@CONST+4,R1
	BZ	LL4
	LDA	*AR0,AR1
	AND	R0,*-AR1(IR1),R1
	CMPI	@CONST+5,R1
	BZ	LL4
	LDA	*AR0,AR1
	LDI	@CONST+6,R1
	AND	R1,*-AR1(IR1),R2
	CMPI	@CONST+7,R2
	BZ	LL4
	LDA	*AR0,AR1
	LDI	@CONST+8,R2
	AND	R2,*-AR1(IR1),R3
	CMPI	@CONST+9,R3
	BZ	LL4
	LDA	*AR0,AR1
	LDI	@CONST+10,R3
	AND	R3,*-AR1(IR1),R9
	CMPI	@CONST+11,R9
	BZ	LL4
	LDA	*AR0,AR1
	AND	R1,*-AR1(IR1),R9
	CMPI	@CONST+12,R9
	BZ	LL4
	LDA	*AR0,AR1
	LDI	31,R9
	ANDN	R9,*-AR1(IR1),R10
	CMPI	@CONST+13,R10
	BNZ	L6
LL4:
	.line	27
;>>>> 				c40_printf( "Can't insert a breakpoint at an address that is\n"
;>>>> 								"one of the three instructions following a delayed\n"
;>>>> 								"program control instruction.\n"
;>>>> 								"No breakpoint set.\n" );
	LDI	@CONST+14,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	1,SP
	.line	31
;>>>> 				return( FAILURE );
	LDI	0,R0
	B	EPI0_1
L6:
	.line	15
	ADDI	1,*+FP(1),R10
	STI	R10,*+FP(1)
	CMPI	3,R10
	BLT	L4
L5:
	.line	35
;>>>> 		brk_addrs[i] = parms[0];
	LDA	*-FP(2),AR0
	LDA	*+FP(1),IR0
	LDA	@CONST+15,AR1
	LDI	*AR0,R0
	STI	R0,*+AR1(IR0)
	.line	36
;>>>> 		breaks[i] = *(unsigned long *)brk_addrs[i];
	LDA	*+AR1(IR0),AR2
	LDA	@CONST+0,AR1
	LDI	*AR2,R0
	STI	R0,*+AR1(IR0)
	.line	37
;>>>> 		*(unsigned long *)brk_addrs[i] = BREAK_TRAP;
	LDA	@CONST+15,AR2
	LDA	*+AR2(IR0),AR2
	LDI	@CONST+16,R0
	STI	R0,*AR2
	.line	39
;>>>> 		return( SUCCESS );
	LDI	1,R0
EPI0_1:
	.line	40
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	46,000000000H,1

	.sym	_del_brk,_del_brk,36,2,0
	.globl	_del_brk

	.func	50
;>>>> 	int del_brk( unsigned long parms[], char put_where )
******************************************************
* FUNCTION DEF : _del_brk
******************************************************
_del_brk:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_parms,-2,31,9,32
	.sym	_put_where,-3,2,9,32
	.sym	_i,1,4,1,32
	.line	2
;>>>> 		int i;
	.line	5
;>>>> 		for( i=0 ; (i < MAX_BREAKS) && (brk_addrs[i] != parms[0]) ; i++ );
	STIK	0,*+FP(1)
	B	L8
L7:
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
L8:
	CMPI	8,*+FP(1)
	BGE	LL7
	LDA	*+FP(1),IR1
	LDA	@CONST+15,AR0
	LDA	*-FP(2),AR1
	CMPI	*AR1,*+AR0(IR1)
	BNZ	L7
LL7:
	.line	7
;>>>> 		if( i == MAX_BREAKS )
	CMPI	8,*+FP(1)
	BNZ	L9
	.line	9
;>>>> 			c40_printf( "Breakpoint not set at address %xh.\n", parms[0] );
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	PUSH	R0
	LDI	@CONST+17,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
	.line	10
;>>>> 			return( FAILURE );
	LDI	0,R0
	B	EPI0_2
L9:
	.line	13
;>>>> 		for( ; i < MAX_BREAKS-1 ; i++ )
	CMPI	7,*+FP(1)
	BGE	L11
L10:
	.line	15
;>>>> 			brk_addrs[i] = brk_addrs[i+1];
	LDA	*+FP(1),IR0
	LDA	@CONST+15,AR0
	ADDI	1,AR0
	LDA	@CONST+15,AR1
	LDI	*+AR0(IR0),R0
	STI	R0,*+AR1(IR0)
	.line	16
;>>>> 			breaks[i] = breaks[i+1];
	LDA	@CONST+0,AR0
	ADDI	1,AR0
	LDA	@CONST+0,AR2
	LDI	*+AR0(IR0),R0
	STI	R0,*+AR2(IR0)
	.line	13
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	7,R0
	BLT	L10
L11:
	.line	19
;>>>> 		brk_addrs[MAX_BREAKS-1] = 0;
	STIK	0,@_brk_addrs+7
	.line	20
;>>>> 		breaks[MAX_BREAKS-1] = 0;
	STIK	0,@_breaks+7
	.line	22
;>>>> 		return( SUCCESS );
	LDI	1,R0
EPI0_2:
	.line	23
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	72,000000000H,1

	.sym	_list_brks,_list_brks,32,2,0
	.globl	_list_brks

	.func	77
;>>>> 	void list_brks( char put_where )
******************************************************
* FUNCTION DEF : _list_brks
******************************************************
_list_brks:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_put_where,-2,2,9,32
	.sym	_i,1,4,1,32
	.line	2
;>>>> 		int i;
	.line	5
;>>>> 		c40_printf( "Break points at addresses:\n" );
	LDI	@CONST+18,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	7
;>>>> 		for( i=0 ; (i < MAX_BREAKS) && brk_addrs[i] ; i++ )
	STIK	0,*+FP(1)
	B	L13
L12:
	.line	8
;>>>> 			c40_printf( "     %xh\n", brk_addrs[i] );
	LDA	*+FP(1),IR1
	LDA	@CONST+15,AR0
	LDI	*+AR0(IR1),R0
	PUSH	R0
	LDI	@CONST+19,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
	.line	7
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
L13:
	CMPI	8,*+FP(1)
	BGE	EPI0_3
	LDA	*+FP(1),IR0
	LDA	@CONST+15,AR0
	LDI	*+AR0(IR0),R0
	BNZ	L12
EPI0_3:
	.line	9
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	85,000000000H,1
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.sect	".const"
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
	.bss	CONST,20
	.sect	".cinit"
	.word	20,CONST
	.word 	_breaks          ;0
	.word 	SL0              ;1
	.word 	-35651584        ;2
	.word 	1755316224       ;3
	.word 	1751121920       ;4
	.word 	1746927616       ;5
	.word 	-16777216        ;6
	.word 	1627389952       ;7
	.word 	-65011712        ;8
	.word 	1814036480       ;9
	.word 	-2097152         ;10
	.word 	2015363072       ;11
	.word 	1694498816       ;12
	.word 	2038431744       ;13
	.word 	SL1              ;14
	.word 	_brk_addrs       ;15
	.word 	1946157567       ;16
	.word 	SL2              ;17
	.word 	SL3              ;18
	.word 	SL4              ;19
	.end
