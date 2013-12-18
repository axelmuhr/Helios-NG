******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 eetest.c eetest.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o eetest.if eetest.asm eetest.tmp 
	.version	40
FP	.set		AR3
	.file	"eetest.c"
	.file	"hydra.h"

	.stag	.fake0,544
	.member	_baudA,0,15,8,32
	.member	_baudB,32,15,8,32
	.member	_dram_size,64,15,8,32
	.member	_cpu_clock,96,15,8,32
	.member	_checksum,128,15,8,32
	.member	_sram1_size,160,15,8,32
	.member	_sram2_size,192,15,8,32
	.member	_sram3_size,224,15,8,32
	.member	_sram4_size,256,15,8,32
	.member	_l_dram_base,288,15,8,32
	.member	_v_dram_base,320,15,8,32
	.member	_l_jtag_base,352,15,8,32
	.member	_v_jtag_base,384,15,8,32
	.member	_l_cont_base,416,15,8,32
	.member	_v_cont_base,448,15,8,32
	.member	_mon_addr,480,15,8,32
	.member	_daughter,512,15,8,32
	.eos
	.sym	_hydra_conf,0,8,13,544,.fake0

	.stag	.fake1,1504
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
	.sym	_reg_set,0,8,13,1504,.fake1
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
	.file	"eetest.c"

	.sym	_main,_main,36,2,0
	.globl	_main

	.func	5
;>>>> 	main()
;>>>> 		char i;
******************************************************
* FUNCTION DEF : _main
******************************************************
_main:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_i,1,2,1,32
	.line	6
;>>>> 		WriteEeprom( 0xf, 1 );
	LDI	1,R0
	PUSH	R0
	LDI	15,R1
	PUSH	R1
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	8
;>>>> 		i = ReadEeprom( 1 );
	LDI	1,R0
	PUSH	R0
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(1)
	.line	11
;>>>> 		WriteEeprom( 0xf, 2 );
	LDI	2,R1
	PUSH	R1
	LDI	15,R2
	PUSH	R2
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	13
;>>>> 		i = ReadEeprom( 2 );
	LDI	2,R0
	PUSH	R0
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(1)
	.line	16
;>>>> 		WriteEeprom( 0x55, 3 );
	LDI	3,R1
	PUSH	R1
	LDI	85,R2
	PUSH	R2
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	18
;>>>> 		i = ReadEeprom( 3 );
	LDI	3,R0
	PUSH	R0
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(1)
	.line	21
;>>>> 		WriteEeprom( 0xaa, 0xff );
	LDI	255,R1
	PUSH	R1
	LDI	170,R2
	PUSH	R2
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	23
;>>>> 		i = ReadEeprom( 0xff );
	LDI	255,R0
	PUSH	R0
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(1)
	.line	26
;>>>> 		WriteEeprom( 120, 10 );
	LDI	10,R1
	PUSH	R1
	LDI	120,R2
	PUSH	R2
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	28
;>>>> 		i = ReadEeprom( 10 );
	LDI	10,R0
	PUSH	R0
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(1)
EPI0_1:
	.line	29
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	33,000000000H,1
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_WriteEeprom
	.globl	_ReadEeprom
	.end
