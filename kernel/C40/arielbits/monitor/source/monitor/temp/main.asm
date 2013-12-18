******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 main.c main.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o main.if main.asm main.tmp 
	.version	40
FP	.set		AR3
	.file	"main.c"
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
	.file	"main.c"
	.globl	_config

	.sym	_main,_main,36,2,0
	.globl	_main

	.func	6
;>>>> 	main()
******************************************************
* FUNCTION DEF : _main
******************************************************
_main:
	PUSH	FP
	LDI	SP,FP
	.line	3
;>>>> 		setupVICVAC();
	CALL	_setupVICVAC
	.line	5
;>>>> 		config.uartA.baud = 7;
	STIK	7,@_config
	.line	6
;>>>> 		config.uartB.baud = 7;
	STIK	7,@_config+3
	.line	7
;>>>> 		config.uartA.parity = 1;
	STIK	1,@_config+1
	.line	8
;>>>> 		config.uartB.parity = 1;
	STIK	1,@_config+4
	.line	9
;>>>> 		config.uartA.bits = 7;
	STIK	7,@_config+2
	.line	10
;>>>> 		config.uartB.bits = 7;
	STIK	7,@_config+5
	.line	11
;>>>> 		config.cpu_clock = 40;
	LDI	40,R0
	STI	R0,@_config+7
	.line	12
;>>>> 		config.sram1_size = 64;
	LDI	64,R1
	STI	R1,@_config+9
	.line	13
;>>>> 		config.sram2_size = 64;
	STI	R1,@_config+10
	.line	14
;>>>> 		config.sram3_size = 64;
	STI	R1,@_config+11
	.line	15
;>>>> 		config.sram4_size = 64;
	STI	R1,@_config+12
	.line	16
;>>>> 		config.dram_size = 1;
	STIK	1,@_config+6
	.line	17
;>>>> 		config.daughter = 0;
	STIK	0,@_config+15
	.line	18
;>>>> 		config.l_jtag_base = 0xdeadbeef;
	LDI	@CONST+0,R2
	STI	R2,@_config+14
	.line	19
;>>>> 		config.l_dram_base = 0x8d000000;
	LDI	@CONST+1,R3
	STI	R3,@_config+13
	.line	21
;>>>> 		test( config, 't' );
	LDI	116,R9
	PUSH	R9
	LDA	@CONST+2,AR0
	LDA	SP,AR1
	ADDI	16,SP
	LDI	*AR0++,R3
	RPTS	15
	STI	R3,*++AR1
    ||	LDI	*AR0++,R3
	CALL	_test
	SUBI	17,SP
EPI0_1:
	.line	22
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	27,000000000H,0

	.sym	_config,_config,8,2,512,.fake1
	.globl	_config
	.bss	_config,16
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,3
	.sect	".cinit"
	.word	3,CONST
	.word 	-559038737       ;0
	.word 	-1929379840      ;1
	.word 	_config          ;2
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_setupVICVAC
	.end
