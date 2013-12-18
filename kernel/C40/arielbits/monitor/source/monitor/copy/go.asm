******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 go.c go.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o go.if go.asm go.tmp 
	.version	40
FP	.set		AR3
	.file	"go.c"
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
	.file	"go.c"
	.globl	_breaks
	.globl	_brk_addrs

	.sym	_go,_go,32,2,0
	.globl	_go

	.func	6
;>>>> 	void go( reg_set *call_set )
******************************************************
* FUNCTION DEF : _go
******************************************************
_go:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
	.sym	_call_set,-2,24,9,32,.fake4
	.sym	_i,1,4,1,32
	.sym	_is_at_break,2,4,1,32
	.line	2
;>>>> 		int i;
	.line	4
;>>>> 	   int is_at_break=MAX_BREAKS;
	STIK	8,*+FP(2)
	.line	7
;>>>> 		for( i=0 ; i < MAX_BREAKS ; i++ )
	STIK	0,*+FP(1)
	CMPI	8,*+FP(1)
	BGE	L2
L1:
	.line	8
;>>>> 			if( call_set->ret_add == brk_addrs[i] )
	LDA	*-FP(2),AR0
	LDA	*+FP(1),IR0
	LDA	@CONST+0,AR1
	LDI	*+AR0(46),R0
	CMPI	*+AR1(IR0),R0
	BNZ	L3
	.line	10
;>>>> 				is_at_break = i;
	LDI	*+FP(1),R0
	STI	R0,*+FP(2)
	.line	11
;>>>> 				*(unsigned long *)brk_addrs[i] = breaks[i];
	LDA	@CONST+1,AR2
	LDA	*+AR1(IR0),AR1
	LDI	*+AR2(IR0),R1
	STI	R1,*AR1
	.line	12
;>>>> 				break;
	B	L2
L3:
	.line	7
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	8,R0
	BLT	L1
L2:
	.line	15
;>>>> 		run( BREAK_TRAP_NUM );
	LDI	511,R0
	PUSH	R0
	CALL	_run
	SUBI	1,SP
	.line	17
;>>>> 		if( *(unsigned long *)(call_set->ret_add-1) == BREAK_TRAP )
	LDA	*-FP(2),AR0
	LDA	*+AR0(46),AR1
	LDI	@CONST+2,R0
	CMPI	R0,*-AR1(1)
	LDIU	0,R1
	LDIZ	1,R1
	OR	01ffh,R1
	BZ	L4
	.line	18
;>>>> 			call_set->ret_add--;
	LDI	*+AR0(46),R1
	SUBI	1,R1
	STI	R1,*+AR0(46)
L4:
	.line	20
;>>>> 		if( is_at_break != MAX_BREAKS )
	CMPI	8,*+FP(2)
	BZ	L5
	.line	21
;>>>> 			*(unsigned long *)brk_addrs[is_at_break] = BREAK_TRAP;
	LDA	*+FP(2),IR1
	LDA	@CONST+0,AR1
	LDA	*+AR1(IR1),AR2
	LDI	@CONST+3,R1
	STI	R1,*AR2
L5:
	.line	23
;>>>> 		reg_dump( *call_set, 't' );
	LDI	116,R1
	PUSH	R1
	LDA	SP,AR1
	ADDI	47,SP
	LDI	*AR0++,R2
	RPTS	46
	STI	R2,*++AR1
    ||	LDI	*AR0++,R2
	CALL	_reg_dump
	SUBI	48,SP
EPI0_1:
	.line	24
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	4,SP
	B	R1
	.endfunc	29,000000000H,2
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
