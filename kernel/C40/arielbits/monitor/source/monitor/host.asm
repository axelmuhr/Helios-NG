******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -q -ic:\c40 host.c host.if 
;	opt30 NOT RUN
;	cg30 -v40 -x -b -o -q host.if host.asm host.tmp 
	.version	40
FP	.set		AR3
	.file	"host.c"
	.file	"hydra.h"

	.stag	.fake0,96
	.member	_baud,0,15,8,32
	.member	_parity,32,4,8,32
	.member	_bits,64,4,8,32
	.eos
	.sym	_UART_config,0,8,13,96,.fake0

	.stag	.fake1,544
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
	.member	_revision,512,2,8,32
	.eos
	.sym	_hydra_conf,0,8,13,544,.fake1

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
	.globl	_BootOthers
	.globl	_comm_sen
	.globl	_comm_rec
	.globl	_com_init
	.globl	_o_crdy
	.globl	_i_crdy
	.globl	_configure
	.globl	_menu
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
	.globl	_WriteEepromWord
	.globl	_crcupdate
	.globl	_x_rcvcrc
	.globl	_x_sndcrc
	.globl	_crchware
	.globl	_mk_crctbl
	.globl	_DMemTest
	.globl	_c40_getchar
	.globl	_c40_putchar
	.globl	_IACK
	.globl	_go
	.globl	_InitHost
	.globl	_ReadHostWord
	.globl	_WriteHostWord
	.globl	_HostIntTrap
	.globl	_LED
	.globl	_zero_regs
	.globl	_SetMICR
	.globl	_monitor
	.globl	_iswhite
	.globl	_isenter
	.globl	_atox
	.globl	_atod
	.globl	_c40_printf
	.globl	_putstr
	.globl	_xtoa
	.globl	_ftoa
	.globl	_reg_dump
	.globl	_print_reg
	.globl	_restore
	.globl	_compare
	.globl	_dump
	.globl	_fill
	.globl	_enter
	.globl	_copy
	.globl	_search
	.globl	_version
	.globl	_help
	.globl	_set_brk
	.globl	_del_brk
	.globl	_list_brks
	.globl	_step
	.globl	_test
	.globl	_reset_others
	.globl	_CommTest
	.globl	_CommFlush
	.globl	_writeVIC
	.globl	_readVIC
	.globl	_writeVAC
	.globl	_readVAC
	.globl	_readVACEPROM
	.globl	_SetupUART
	.globl	_setupVICVAC
	.globl	_ReadEepromWord
	.globl	_SetupVICVACDefault
	.globl	_break_pt
	.globl	_user_int
	.globl	_resume_mon
	.globl	_clr_int
	.globl	_ReadEeprom
	.globl	_WriteEeprom
	.globl	_init
	.globl	_SetIntTable
	.globl	_SetIntVect
	.globl	_SetTrapTable
	.globl	_SetTrapVect
	.globl	_EnableInt
	.globl	_DisableInt
	.globl	_GIEOn
	.globl	_GIEOff
	.globl	_ClearIIOF
	.globl	_MemTest
	.globl	_DualMem
	.globl	_run
	.globl	_RunForHost
	.globl	_readTCR
	.globl	_writeTCR
	.file	"host.h"

	.stag	.fake5,512
	.member	_WhatToDo,0,4,8,32
	.member	_Parameters,32,63,8,448,,14
	.member	_InterruptSemaphore,480,4,8,32
	.eos
	.sym	_HostMessage,0,8,13,512,.fake5

	.stag	.fake6,64
	.member	_IntNum,0,4,8,32
	.member	_IntVector,32,4,8,32
	.eos
	.sym	_HostIntStructure,0,8,13,64,.fake6
	.sym	_HostDataBuff,0,31,13,32
	.file	"host.c"
	.globl	_VMEInt
	.globl	_config
	.globl	__stack
	.globl	_EEPROM_Status

	.sym	_c_int14,_c_int14,32,2,0
	.globl	_c_int14

	.func	13
******************************************************
* FUNCTION DEF : _c_int14
******************************************************
_c_int14:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	PUSH	ST
	PUSH	R0
	PUSHF	R0
	PUSH	R1
	PUSHF	R1
	PUSH	R2
	PUSHF	R2
	PUSH	R3
	PUSHF	R3
	PUSH	R4
	PUSHF	R4
	PUSH	R5
	PUSHF	R5
	PUSH	R6
	PUSHF	R6
	PUSH	R7
	PUSHF	R7
	PUSH	R8
	PUSHF	R8
	PUSH	R9
	PUSHF	R9
	PUSH	R10
	PUSHF	R10
	PUSH	R11
	PUSHF	R11
	PUSH	AR0
	PUSH	AR1
	PUSH	AR2
	PUSH	IR0
	PUSH	IR1
	PUSH	BK
	PUSH	RC
	PUSH	RS
	PUSH	RE
	PUSH	DP
	.sym	_i,1,4,1,32
	.line	5
	LDI	0,R0
	PUSH	R0
	LDI	95,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	7
	LDI	32,R0
	PUSH	R0
	CALL	_IACK
	SUBI	1,SP
	.line	9
	LDI	99,R0
	PUSH	R0
	CALL	_readVIC
	SUBI	1,SP
	STI	R0,*+FP(1)
	CMPI	0,R0
	BZ	L1
	.line	11
	LDI	0,R1
	PUSH	R1
	LDI	99,R2
	PUSH	R2
	CALL	_writeVIC
	SUBI	2,SP
	B	L2
L4:
	.line	16
	LDI	99,R0
	PUSH	R0
	CALL	_readVIC
	SUBI	1,SP
	STI	R0,*+FP(1)
	CMPI	0,R0
	BZ	L4
	.line	17
	LDI	0,R1
	PUSH	R1
	LDI	99,R2
	PUSH	R2
	CALL	_writeVIC
	SUBI	2,SP
	B	L5
L6:
	.line	21
	CALL	_ReadHostWord
	AND	0ffffh,R0
	LDP	_config
	STI	R0,@_config
	.line	22
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_write_config
	SUBI	17,SP
	.line	23
	B	L7
L8:
	.line	25
	CALL	_ReadHostWord
	LDP	_config+1
	STI	R0,@_config+1
	.line	26
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_write_config
	SUBI	17,SP
	.line	27
	B	L7
L9:
	.line	29
	CALL	_ReadHostWord
	AND	0ffh,R0
	LDP	_config+2
	STI	R0,@_config+2
	.line	30
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_write_config
	SUBI	17,SP
	.line	31
	B	L7
L10:
	.line	33
	CALL	_ReadHostWord
	AND	0ffffh,R0
	LDP	_config+3
	STI	R0,@_config+3
	.line	34
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_write_config
	SUBI	17,SP
	.line	35
	B	L7
L11:
	.line	37
	CALL	_ReadHostWord
	LDP	_config+4
	STI	R0,@_config+4
	.line	38
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_write_config
	SUBI	17,SP
	.line	39
	B	L7
L12:
	.line	41
	CALL	_ReadHostWord
	AND	0ffh,R0
	LDP	_config+5
	STI	R0,@_config+5
	.line	42
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_write_config
	SUBI	17,SP
	.line	43
	B	L7
L13:
	.line	45
	CALL	_ReadHostWord
	AND	0ffh,R0
	LDP	_config+6
	STI	R0,@_config+6
	.line	46
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_write_config
	SUBI	17,SP
	.line	47
	B	L7
L14:
	.line	49
	CALL	_ReadHostWord
	AND	0ffh,R0
	LDP	_config+7
	STI	R0,@_config+7
	.line	50
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_write_config
	SUBI	17,SP
	.line	51
	B	L7
L15:
	.line	53
	CALL	_ReadHostWord
	AND	0ffffh,R0
	LDP	_config+9
	STI	R0,@_config+9
	.line	54
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_write_config
	SUBI	17,SP
	.line	55
	B	L7
L16:
	.line	57
	CALL	_ReadHostWord
	AND	0ffffh,R0
	LDP	_config+10
	STI	R0,@_config+10
	.line	58
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_write_config
	SUBI	17,SP
	.line	59
	B	L7
L17:
	.line	61
	CALL	_ReadHostWord
	AND	0ffffh,R0
	LDP	_config+11
	STI	R0,@_config+11
	.line	62
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_write_config
	SUBI	17,SP
	.line	63
	B	L7
L18:
	.line	65
	CALL	_ReadHostWord
	AND	0ffffh,R0
	LDP	_config+12
	STI	R0,@_config+12
	.line	66
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_write_config
	SUBI	17,SP
	.line	67
	B	L7
L19:
	.line	69
	CALL	_ReadHostWord
	LDP	_config+13
	STI	R0,@_config+13
	.line	70
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_write_config
	SUBI	17,SP
	.line	71
	B	L7
L20:
	.line	73
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*AR0++,R0
	CALL	_ReadHostWord
	PUSH	R0
	LDI	13,R0
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	74
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*AR0++,R0
	LDI	13,R0
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	18,SP
	PUSH	R0
	LDI	1,R0
	PUSH	R0
	CALL	_writeVAC
	SUBI	2,SP
	.line	75
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*AR0++,R0
	CALL	_write_config
	SUBI	17,SP
	.line	76
	B	L7
L21:
	.line	78
	CALL	_ReadHostWord
	AND	0ffh,R0
	LDP	_config+15
	STI	R0,@_config+15
	.line	79
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_write_config
	SUBI	17,SP
	.line	80
	B	L7
L22:
	.line	82
	LDP	CONST+0
	LDA	@CONST+0,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*AR0++,R0
	CALL	_ReadHostWord
	AND	0ffffh,R0
	PUSH	R0
	LDI	25,R0
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	83
	B	L7
L23:
	.line	85
	LDP	_config+14
	LDI	@_config+14,R0
	ADDI	8,R0
	PUSH	R0
	CALL	_ReadHostWord
	LDP	_config+16
	STI	R0,@_config+16
	PUSH	R0
	LDI	79,R1
	PUSH	R1
	CALL	_WriteEeprom
	SUBI	3,SP
	.line	86
	B	L7
L5:
	.line	18
	LDA	240,IR0
	AND	IR0,*+FP(1),IR1
	LDP	CONST+1
	LDA	@CONST+1,AR0
	SUBI	128,IR1
	CMPI	18,IR1
	LDIHI	19,IR1
	LDA	*+AR0(IR1),AR0
	B	AR0
	.sect	".const"
LL4:
	.word	L6
	.word	L8
	.word	L9
	.word	L10
	.word	L11
	.word	L12
	.word	L13
	.word	L14
	.word	L15
	.word	L16
	.word	L17
	.word	L18
	.word	L19
	.word	L20
	.word	L21
	.word	L22
	.word	L7
	.word	L7
	.word	L23
	.word	L7
	.text
L7:
	.line	88
	B	L24
L26:
	.line	90
	LDI	99,R0
	PUSH	R0
	CALL	_readVIC
	SUBI	1,SP
	STI	R0,*+FP(1)
	CMPI	0,R0
	BZ	L26
	B	L27
L28:
	.line	94
	LDP	_config
	LDI	@_config,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	95
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	96
	B	L29
L30:
	.line	98
	LDP	_config+1
	LDI	@_config+1,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	99
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	100
	B	L29
L31:
	.line	102
	LDP	_config+2
	LDI	@_config+2,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	103
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	104
	B	L29
L32:
	.line	106
	LDP	_config+3
	LDI	@_config+3,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	107
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	108
	B	L29
L33:
	.line	110
	LDP	_config+4
	LDI	@_config+4,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	111
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	112
	B	L29
L34:
	.line	114
	LDP	_config+5
	LDI	@_config+5,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	115
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	116
	B	L29
L35:
	.line	118
	LDP	_config+6
	LDI	@_config+6,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	119
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	120
	B	L29
L36:
	.line	122
	LDP	_config+7
	LDI	@_config+7,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	123
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	124
	B	L29
L37:
	.line	126
	LDP	_config+9
	LDI	@_config+9,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	127
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	128
	B	L29
L38:
	.line	130
	LDP	_config+10
	LDI	@_config+10,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	131
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	132
	B	L29
L39:
	.line	134
	LDP	_config+11
	LDI	@_config+11,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	135
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	136
	B	L29
L40:
	.line	138
	LDP	_config+12
	LDI	@_config+12,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	139
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	140
	B	L29
L41:
	.line	142
	LDP	_config+13
	LDI	@_config+13,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	143
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	144
	B	L29
L42:
	.line	146
	LDI	1,R0
	PUSH	R0
	CALL	_readVAC
	SUBI	1,SP
	LSH	16,R0
	ANDN	0ffffh,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	147
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	148
	B	L29
L43:
	.line	150
	LDP	_config+15
	LDI	@_config+15,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	151
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	152
	B	L29
L44:
	.line	154
	LDI	4,R0
	PUSH	R0
	CALL	_readVAC
	SUBI	1,SP
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	155
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	156
	B	L29
L45:
	.line	158
	LDP	CONST+2
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	159
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	160
	B	L29
L46:
	.line	162
	LDP	CONST+3
	LDI	@CONST+3,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	163
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	164
	B	L29
L47:
	.line	166
	LDP	_config+16
	LDI	@_config+16,R0
	PUSH	R0
	CALL	_WriteHostWord
	SUBI	1,SP
	.line	167
	LDI	1,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	168
	B	L29
L27:
	.line	91
	LDA	*+FP(1),IR0
	LDP	CONST+4
	LDA	@CONST+4,AR0
	SUBI	128,IR0
	CMPI	18,IR0
	LDIHI	19,IR0
	LDA	*+AR0(IR0),AR0
	B	AR0
	.sect	".const"
LL6:
	.word	L28
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
	.word	L41
	.word	L42
	.word	L43
	.word	L44
	.word	L45
	.word	L46
	.word	L47
	.word	L29
	.text
L29:
	.line	170
	B	L24
L48:
	.line	172
	B	EPI0_1
L2:
	.line	13
	LDI	*+FP(1),R0
	CMPI	6,R0
	BZ	L26
	CMPI	7,R0
	BZ	L4
	B	L48
L24:
	.line	175
	LDI	0,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	B	EPI0_1
L1:
	B	L50
L51:
	.line	182
	LDP	_msg
	LDA	@_msg,AR0
	LDI	*+AR0(1),R0
	PUSH	R0
	LDP	CONST+0
	LDA	@CONST+0,AR1
	LDA	SP,AR2
	ADDI	17,SP
	LDI	*AR1++,R0
	RPTS	16
	STI	R0,*++AR2
    ||	LDI	*AR1++,R0
	CALL	_CommFlush
	SUBI	18,SP
	CMPI	0,R0
	BNZ	L52
	.line	184
	LDP	_msg
	LDA	@_msg,AR0
	STIK	0,*AR0
	.line	185
	B	EPI0_1
L52:
	.line	187
	LDP	_msg
	LDA	@_msg,AR0
	LDI	*+AR0(1),R0
	PUSH	R0
	LDP	CONST+0
	LDA	@CONST+0,AR1
	LDA	SP,AR2
	ADDI	17,SP
	LDI	*AR1++,R0
	RPTS	16
	STI	R0,*++AR2
    ||	LDI	*AR1++,R0
	CALL	_reset_others
	SUBI	18,SP
	.line	188
	LDP	_msg
	LDA	@_msg,AR0
	LDI	*+AR0(1),R0
	PUSH	R0
	CALL	_BootOthers
	SUBI	1,SP
	CMPI	0,R0
	BNZ	L54
	.line	189
	LDP	_msg
	LDA	@_msg,AR0
	STIK	1,*AR0
	B	L55
L54:
	.line	192
	LDP	_msg
	LDA	@_msg,AR0
	STIK	0,*AR0
	.line	193
	B	EPI0_1
L55:
	.line	196
	LDI	1000,R0
	PUSH	R0
	LDI	0,R1
	PUSH	R1
	LDI	*+AR0(1),R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	.line	197
	LDI	1000,R0
	PUSH	R0
	LDP	_config+13
	LDI	@_config+13,R1
	PUSH	R1
	LDP	_msg
	LDA	@_msg,AR0
	LDI	*+AR0(1),R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	.line	198
	LDI	1000,R0
	PUSH	R0
	LDP	_config+6
	LDI	@_config+6,R1
	PUSH	R1
	LDP	_msg
	LDA	@_msg,AR0
	LDI	*+AR0(1),R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	.line	199
	B	EPI0_1
L56:
	.line	201
	LDP	_msg
	LDI	@_msg,R0
	ADDI	1,R0
	PUSH	R0
	CALL	_copy
	SUBI	1,SP
	.line	202
	LDP	_msg
	LDA	@_msg,AR0
	STIK	1,*AR0
	.line	203
	B	EPI0_1
L57:
	.line	205
	LDP	_msg
	LDA	@_msg,AR0
	STIK	1,*AR0
	.line	206
	LDI	*+AR0(1),R0
	PUSH	R0
	CALL	_RunForHost
	SUBI	1,SP
	.line	207
	B	EPI0_1
L58:
	.line	209
	LDP	_msg
	LDA	@_msg,AR0
	STIK	1,*AR0
	.line	210
	LDP	CONST+5
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_halt
	SUBI	1,SP
	.line	211
	B	EPI0_1
L59:
	.line	213
	LDP	_msg
	LDA	@_msg,AR0
	LDI	*+AR0(1),R0
	LDP	_VMEInt
	STI	R0,@_VMEInt
	.line	214
	B	EPI0_1
L60:
	.line	216
	LDP	_msg
	LDA	@_msg,AR0
	LDI	*+AR0(1),R0
	LDP	_VMEInt+1
	STI	R0,@_VMEInt+1
	.line	217
	B	EPI0_1
L61:
	.line	219
	B	EPI0_1
L50:
	.line	179
	LDP	_msg
	LDA	@_msg,AR0
	LDI	*AR0,R1
	CMPI	2,R1
	BZ	L51
	CMPI	3,R1
	BZ	L56
	CMPI	4,R1
	BZ	L57
	CMPI	5,R1
	BZ	L58
	CMPI	9,R1
	BZ	L59
	CMPI	16,R1
	BZ	L60
	B	L61
EPI0_1:
	.line	222
	POP	DP
	POP	RE
	POP	RS
	POP	RC
	POP	BK
	POP	IR1
	POP	IR0
	POP	AR2
	POP	AR1
	POP	AR0
	POPF	R11
	POP	R11
	POPF	R10
	POP	R10
	POPF	R9
	POP	R9
	POPF	R8
	POP	R8
	POPF	R7
	POP	R7
	POPF	R6
	POP	R6
	POPF	R5
	POP	R5
	POPF	R4
	POP	R4
	POPF	R3
	POP	R3
	POPF	R2
	POP	R2
	POPF	R1
	POP	R1
	POPF	R0
	POP	R0
	POP	ST
	SUBI	1,SP
	POP	FP
	RETI

	.endfunc	234,01bff07ffH,1

	.sym	_InitHost,_InitHost,36,2,0
	.globl	_InitHost

	.func	239
******************************************************
* FUNCTION DEF : _InitHost
******************************************************
_InitHost:
	PUSH	FP
	LDI	SP,FP
	.sym	_config,-18,8,9,544,.fake1
	.line	5
	LDI	*-FP(12),R0
	LSH	20,R0
	ADDI	*-FP(5),R0
	SUBI	16,R0
	LDP	_msg
	STI	R0,@_msg
	.line	8
	LDA	R0,AR0
	STIK	0,*+AR0(15)
	.line	11
	LDI	0,R1
	PUSH	R1
	LDI	99,R2
	PUSH	R2
	CALL	_writeVIC
	SUBI	2,SP
	.line	12
	LDI	0,R0
	PUSH	R0
	LDI	103,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	13
	LDI	0,R0
	PUSH	R0
	LDI	107,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	14
	LDI	0,R0
	PUSH	R0
	LDI	111,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	15
	LDI	0,R0
	PUSH	R0
	LDI	115,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
EPI0_2:
	.line	17
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	255,000000000H,0

	.sym	_ReadHostWord,_ReadHostWord,47,2,0
	.globl	_ReadHostWord

	.func	259
******************************************************
* FUNCTION DEF : _ReadHostWord
******************************************************
_ReadHostWord:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
	.sym	_val,1,15,1,32
	.sym	_i,2,15,1,32
	.line	5
	STIK	0,*+FP(2)
	STIK	0,*+FP(1)
	CMPI	4,*+FP(2)
	BHS	L63
L64:
	.line	7
	LDI	99,R0
	PUSH	R0
	CALL	_readVIC
	SUBI	1,SP
	CMPI	0,R0
	BZ	L64
	.line	9
	LDI	103,R0
	PUSH	R0
	CALL	_readVIC
	SUBI	1,SP
	OR	*+FP(1),R0
	STI	R0,*+FP(1)
	.line	11
	LDI	0,R1
	PUSH	R1
	LDI	99,R2
	PUSH	R2
	CALL	_writeVIC
	SUBI	2,SP
	.line	5
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLO	L64
L63:
	.line	14
	LDI	*+FP(1),R0
EPI0_3:
	.line	15
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	4,SP
	B	R1
	.endfunc	273,000000000H,2

	.sym	_WriteHostWord,_WriteHostWord,32,2,0
	.globl	_WriteHostWord

	.func	277
******************************************************
* FUNCTION DEF : _WriteHostWord
******************************************************
_WriteHostWord:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_val,-2,15,9,32
	.sym	_i,1,15,1,32
	.line	2
	.line	5
	STIK	0,*+FP(1)
	CMPI	4,*+FP(1)
	BHS	EPI0_4
L65:
	.line	7
	MPYI	-8,*+FP(1),R0
	LDI	*-FP(2),R1
	LSH	R0,R1,R0
	AND	0ffh,R0
	PUSH	R0
	LDI	103,R0
	PUSH	R0
	CALL	_writeVIC
	SUBI	2,SP
	.line	9
	LDI	0,R0
	PUSH	R0
	LDI	99,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
L67:
	.line	11
	LDI	99,R0
	PUSH	R0
	CALL	_readVIC
	SUBI	1,SP
	CMPI	0,R0
	BZ	L67
	.line	5
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	4,R0
	BLO	L65
EPI0_4:
	.line	13
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	289,000000000H,1

	.sym	_writeVIC,_writeVIC,32,2,0
	.globl	_writeVIC

	.func	292
******************************************************
* FUNCTION DEF : _writeVIC
******************************************************
_writeVIC:
	PUSH	FP
	LDI	SP,FP
	.sym	_add,-2,15,9,32
	.sym	_data,-3,15,9,32
	.line	2
	.line	3
	LDA	*-FP(2),AR0
	LDP	CONST+6
	OR	@CONST+6,AR0
	LSH	-2,AR0
	LDP	CONST+7
	OR	@CONST+7,AR0
	LDI	*-FP(3),R0
	STI	R0,*AR0
EPI0_5:
	.line	4
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	295,000000000H,0

	.sym	_readVIC,_readVIC,47,2,0
	.globl	_readVIC

	.func	297
******************************************************
* FUNCTION DEF : _readVIC
******************************************************
_readVIC:
	PUSH	FP
	LDI	SP,FP
	.sym	_add,-2,15,9,32
	.line	2
	.line	3
	LDA	*-FP(2),AR0
	LDP	CONST+6
	OR	@CONST+6,AR0
	LSH	-2,AR0
	LDP	CONST+7
	OR	@CONST+7,AR0
	LDI	255,R0
	AND	*AR0,R0
EPI0_6:
	.line	4
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	300,000000000H,0

	.sym	_IACK,_IACK,32,2,0
	.globl	_IACK

	.func	304
******************************************************
* FUNCTION DEF : _IACK
******************************************************
_IACK:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
	.sym	_ipl,-2,15,9,32
	.sym	_tcr,1,15,1,32
	.sym	_dummy,2,2,1,32
	.line	2
	.line	6
	CALL	_readTCR
	STI	R0,*+FP(1)
	.line	7
	LDI	*-FP(2),R1
	PUSH	R1
	CALL	_writeTCR
	SUBI	1,SP
	.line	8
	LDP	CONST+8
	LDA	@CONST+8,AR0
	LDI	*AR0,R0
	STI	R0,*+FP(2)
	.line	9
	LDI	*+FP(1),R1
	PUSH	R1
	CALL	_writeTCR
	SUBI	1,SP
EPI0_7:
	.line	10
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	4,SP
	B	R1
	.endfunc	313,000000000H,2

	.sym	_write_config,_write_config,36,2,0
	.globl	_write_config

	.func	319
******************************************************
* FUNCTION DEF : _write_config
******************************************************
_write_config:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_config,-18,8,9,544,.fake1
	.sym	_val,1,15,1,32
	.line	6
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*++AR0,R0
	LDI	61,R0
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	18,SP
	STI	R0,*+FP(1)
	.line	7
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R1
	BNZ	L68
	.line	9
	LDI	0,R0
	B	EPI0_8
L68:
	B	L69
L70:
	.line	14
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	15
	.line	16
	B	L71
L72:
	.line	18
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	19
	OR	0400h,R1
	STI	R1,*+FP(1)
	.line	20
	B	L71
L73:
	.line	22
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	23
	OR	0800h,R1
	STI	R1,*+FP(1)
	.line	24
	B	L71
L74:
	.line	26
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	27
	OR	0c00h,R1
	STI	R1,*+FP(1)
	.line	28
	B	L71
L75:
	.line	30
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	31
	OR	01000h,R1
	STI	R1,*+FP(1)
	.line	32
	B	L71
L76:
	.line	34
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	35
	OR	01400h,R1
	STI	R1,*+FP(1)
	.line	36
	B	L71
L77:
	.line	38
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	39
	OR	01800h,R1
	STI	R1,*+FP(1)
	.line	40
	B	L71
L78:
	.line	42
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	43
	OR	01c00h,R1
	STI	R1,*+FP(1)
	.line	44
	B	L71
L69:
	.line	11
	LDI	*-FP(18),R2
	CMPI	150,R2
	BZ	L70
	CMPI	300,R2
	BZ	L72
	CMPI	600,R2
	BZ	L73
	CMPI	1200,R2
	BZ	L74
	CMPI	2400,R2
	BZ	L75
	CMPI	4800,R2
	BZ	L76
	CMPI	9600,R2
	BZ	L77
	CMPI	19200,R2
	BZ	L78
L71:
	.line	47
	LDI	*-FP(17),R0
	BLE	L79
	.line	48
	TSTB	1,R0
	BZ	L80
	.line	50
	LDP	CONST+10
	LDI	@CONST+10,R1
	OR	R1,*+FP(1),R2
	STI	R2,*+FP(1)
	.line	51
	AND	0bfffh,R2
	STI	R2,*+FP(1)
	B	L81
L80:
	.line	55
	LDP	CONST+11
	LDI	@CONST+11,R1
	OR	R1,*+FP(1),R2
	STI	R2,*+FP(1)
L81:
	B	L82
L79:
	.line	58
	TSTB	1,R0
	BZ	L83
	.line	60
	LDI	16383,R1
	AND	R1,*+FP(1),R2
	STI	R2,*+FP(1)
	B	L82
L83:
	.line	64
	LDI	16384,R1
	OR	R1,*+FP(1),R2
	STI	R2,*+FP(1)
	.line	65
	AND	07fffh,R2
	STI	R2,*+FP(1)
L82:
	.line	67
	LDI	*-FP(16),R1
	CMPI	7,R1
	BNZ	L84
	.line	68
	AND	0dfffh,R2
	STI	R2,*+FP(1)
	B	L85
L84:
	.line	70
	OR	02000h,R2
	STI	R2,*+FP(1)
L85:
	.line	71
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R3
	RPTS	16
	STI	R3,*++AR1
    ||	LDI	*++AR0,R3
	PUSH	R2
	LDI	61,R3
	PUSH	R3
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	72
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BNZ	L86
	.line	74
	LDI	0,R0
	B	EPI0_8
L86:
	.line	77
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*++AR0,R1
	LDI	63,R1
	PUSH	R1
	CALL	_ReadEepromWord
	SUBI	18,SP
	STI	R0,*+FP(1)
	.line	78
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R1
	BNZ	L87
	.line	80
	LDI	0,R0
	B	EPI0_8
L87:
	B	L88
L89:
	.line	85
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	86
	.line	87
	B	L90
L91:
	.line	89
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	90
	OR	0400h,R1
	STI	R1,*+FP(1)
	.line	91
	B	L90
L92:
	.line	93
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	94
	OR	0800h,R1
	STI	R1,*+FP(1)
	.line	95
	B	L90
L93:
	.line	97
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	98
	OR	0c00h,R1
	STI	R1,*+FP(1)
	.line	99
	B	L90
L94:
	.line	101
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	102
	OR	01000h,R1
	STI	R1,*+FP(1)
	.line	103
	B	L90
L95:
	.line	105
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	106
	OR	01400h,R1
	STI	R1,*+FP(1)
	.line	107
	B	L90
L96:
	.line	109
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	110
	OR	01800h,R1
	STI	R1,*+FP(1)
	.line	111
	B	L90
L97:
	.line	113
	LDP	CONST+9
	LDI	@CONST+9,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	114
	OR	01c00h,R1
	STI	R1,*+FP(1)
	.line	115
	B	L90
L88:
	.line	82
	LDI	*-FP(15),R2
	CMPI	150,R2
	BZ	L89
	CMPI	300,R2
	BZ	L91
	CMPI	600,R2
	BZ	L92
	CMPI	1200,R2
	BZ	L93
	CMPI	2400,R2
	BZ	L94
	CMPI	4800,R2
	BZ	L95
	CMPI	9600,R2
	BZ	L96
	CMPI	19200,R2
	BZ	L97
L90:
	.line	118
	LDI	*-FP(14),R0
	BLE	L98
	.line	119
	TSTB	1,R0
	BZ	L99
	.line	121
	LDP	CONST+10
	LDI	@CONST+10,R1
	OR	R1,*+FP(1),R2
	STI	R2,*+FP(1)
	.line	122
	AND	0bfffh,R2
	STI	R2,*+FP(1)
	B	L100
L99:
	.line	126
	LDP	CONST+11
	LDI	@CONST+11,R1
	OR	R1,*+FP(1),R2
	STI	R2,*+FP(1)
L100:
	B	L101
L98:
	.line	129
	TSTB	1,R0
	BZ	L102
	.line	131
	LDI	16383,R1
	AND	R1,*+FP(1),R2
	STI	R2,*+FP(1)
	B	L101
L102:
	.line	135
	LDI	16384,R1
	OR	R1,*+FP(1),R2
	STI	R2,*+FP(1)
	.line	136
	AND	07fffh,R2
	STI	R2,*+FP(1)
L101:
	.line	138
	LDI	*-FP(13),R1
	CMPI	7,R1
	BNZ	L103
	.line	139
	AND	0dfffh,R2
	STI	R2,*+FP(1)
	B	L104
L103:
	.line	141
	OR	02000h,R2
	STI	R2,*+FP(1)
L104:
	.line	142
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R3
	RPTS	16
	STI	R3,*++AR1
    ||	LDI	*++AR0,R3
	PUSH	R2
	LDI	63,R3
	PUSH	R3
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	143
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BNZ	L105
	.line	145
	LDI	0,R0
	B	EPI0_8
L105:
	.line	148
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*++AR0,R1
	LDI	*-FP(12),R1
	PUSH	R1
	LDI	67,R2
	PUSH	R2
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	149
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BNZ	L106
	.line	151
	LDI	0,R0
	B	EPI0_8
L106:
	.line	153
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*++AR0,R1
	LDI	*-FP(3),R1
	PUSH	R1
	LDI	69,R2
	PUSH	R2
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	154
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BNZ	L107
	.line	156
	LDI	0,R0
	B	EPI0_8
L107:
	.line	158
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*++AR0,R1
	LDI	*-FP(9),R1
	PUSH	R1
	LDI	71,R2
	PUSH	R2
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	159
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BNZ	L108
	.line	161
	LDI	0,R0
	B	EPI0_8
L108:
	.line	163
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*++AR0,R1
	LDI	*-FP(8),R1
	PUSH	R1
	LDI	73,R2
	PUSH	R2
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	164
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BNZ	L109
	.line	166
	LDI	0,R0
	B	EPI0_8
L109:
	.line	168
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*++AR0,R1
	LDI	*-FP(7),R1
	PUSH	R1
	LDI	75,R2
	PUSH	R2
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	169
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BNZ	L110
	.line	171
	LDI	0,R0
	B	EPI0_8
L110:
	.line	173
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*++AR0,R1
	LDI	*-FP(6),R1
	PUSH	R1
	LDI	77,R2
	PUSH	R2
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	174
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BNZ	L111
	.line	176
	LDI	0,R0
	B	EPI0_8
L111:
	B	L112
L113:
	.line	182
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*++AR0,R0
	LDI	16896,R0
	PUSH	R0
	LDI	59,R1
	PUSH	R1
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	183
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BNZ	L114
	.line	185
	LDI	0,R0
	B	EPI0_8
L114:
	.line	187
	B	L115
L116:
	.line	189
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*++AR0,R0
	LDI	20992,R0
	PUSH	R0
	LDI	59,R1
	PUSH	R1
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	190
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BNZ	L117
	.line	192
	LDI	0,R0
	B	EPI0_8
L117:
	.line	194
	B	L115
L118:
	.line	196
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*++AR0,R0
	LDI	16896,R0
	PUSH	R0
	LDI	59,R1
	PUSH	R1
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	197
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BNZ	L119
	.line	199
	LDI	0,R0
	B	EPI0_8
L119:
	.line	201
	B	L115
L112:
	.line	179
	LDI	*-FP(11),R1
	CMPI	40,R1
	BZ	L113
	CMPI	50,R1
	BZ	L116
	B	L118
L115:
	.line	205
	LDI	*-FP(5),R1
	LDP	CONST+12
	AND	@CONST+12,R1
	LSH	-14,R1
	STI	R1,*+FP(1)
	.line	206
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R2
	RPTS	16
	STI	R2,*++AR1
    ||	LDI	*++AR0,R2
	PUSH	R1
	LDI	17,R2
	PUSH	R2
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	207
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BNZ	L120
	.line	209
	LDI	0,R0
	B	EPI0_8
L120:
	.line	212
	LDI	*-FP(4),R1
	LDP	CONST+12
	AND	@CONST+12,R1
	LSH	-14,R1
	STI	R1,*+FP(1)
	.line	213
	LDA	FP,AR0
	LDA	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R2
	RPTS	16
	STI	R2,*++AR1
    ||	LDI	*++AR0,R2
	PUSH	R1
	LDI	25,R2
	PUSH	R2
	CALL	_WriteEepromWord
	SUBI	19,SP
	.line	214
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BNZ	L121
	.line	216
	LDI	0,R0
	B	EPI0_8
L121:
	.line	219
	LDI	*-FP(4),R1
	ADDI	8,R1
	PUSH	R1
	LDI	*-FP(2),R1
	PUSH	R1
	LDI	79,R2
	PUSH	R2
	CALL	_WriteEeprom
	SUBI	3,SP
	.line	220
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BNZ	EPI0_8
	.line	222
	LDI	0,R0
EPI0_8:
	.line	224
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	542,000000000H,1

	.sym	_WriteEepromWord,_WriteEepromWord,32,2,0
	.globl	_WriteEepromWord

	.func	547
******************************************************
* FUNCTION DEF : _WriteEepromWord
******************************************************
_WriteEepromWord:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_WordAddress,-2,4,9,32
	.sym	_data,-3,15,9,32
	.sym	_config,-20,8,9,544,.fake1
	.sym	_i,1,4,1,32
	.line	2
	.line	5
	STIK	0,*+FP(1)
	CMPI	2,*+FP(1)
	BGE	EPI0_9
L123:
	.line	7
	LDI	*-FP(6),R0
	ADDI	8,R0
	PUSH	R0
	LDI	*-FP(3),R0
	AND	0ffh,R0
	PUSH	R0
	LDI	*-FP(2),R0
	ADDI	1,R0,R1
	STI	R1,*-FP(2)
	SUBI	1,R1
	PUSH	R1
	CALL	_WriteEeprom
	SUBI	3,SP
	.line	8
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R0
	BZ	EPI0_9
	.line	10
	.line	5
	ADDI	1,*+FP(1),R1
	STI	R1,*+FP(1)
	LDI	*-FP(3),R2
	LSH	-8,R2,R3
	STI	R3,*-FP(3)
	CMPI	2,R1
	BLT	L123
EPI0_9:
	.line	13
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	559,000000000H,1

	.sym	_ReadEepromWord,_ReadEepromWord,47,2,0
	.globl	_ReadEepromWord

	.func	564
******************************************************
* FUNCTION DEF : _ReadEepromWord
******************************************************
_ReadEepromWord:
	PUSH	FP
	LDI	SP,FP
	ADDI	3,SP
	.sym	_WordAddress,-2,4,9,32
	.sym	_config,-19,8,9,544,.fake1
	.sym	_word,1,15,1,32
	.sym	_i,2,4,1,32
	.sym	_j,3,4,1,32
	.line	2
	.line	6
	STIK	0,*+FP(2)
	STIK	0,*+FP(1)
	CMPI	2,*+FP(2)
	BGE	L127
L126:
	.line	8
	LDI	*-FP(5),R0
	ADDI	8,R0
	PUSH	R0
	LDI	*-FP(2),R0
	ADDI	1,R0,R1
	STI	R1,*-FP(2)
	SUBI	1,R1
	PUSH	R1
	CALL	_ReadEeprom
	SUBI	2,SP
	STI	R0,*+FP(3)
	LSH	3,*+FP(2),R1
	LSH	R1,R0,R1
	OR	*+FP(1),R1
	STI	R1,*+FP(1)
	.line	9
	LDP	_EEPROM_Status
	LDI	@_EEPROM_Status,R2
	BNZ	L128
	.line	11
	LDI	0,R0
	B	EPI0_10
L128:
	.line	6
	ADDI	1,*+FP(2),R3
	STI	R3,*+FP(2)
	CMPI	2,R3
	BLT	L126
L127:
	.line	15
	LDI	*+FP(1),R0
EPI0_10:
	.line	16
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	5,SP
	B	R1
	.endfunc	579,000000000H,3

	.sym	_reset_others,_reset_others,32,2,0
	.globl	_reset_others

	.func	585
******************************************************
* FUNCTION DEF : _reset_others
******************************************************
_reset_others:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_config,-18,8,9,544,.fake1
	.sym	_Which,-19,4,9,32
	.sym	_i,1,15,1,32
	.line	2
	.line	6
	LDP	CONST+13
	LDI	@CONST+13,R0
	LSH	*-FP(19),R0
	LDA	*-FP(4),AR0
	LDI	*+AR0(8),R1
	ANDN	R0,R1,R0
	STI	R0,*+AR0(8)
	.line	9
	STIK	0,*+FP(1)
	LDI	100,R0
	CMPI	R0,*+FP(1)
	BHS	L130
L129:
	.line	10
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	9
	CMPI	100,R0
	BLO	L129
L130:
	.line	14
	LDI	*-FP(19),R0
	MPYI	3,R0,R1
	LDP	CONST+14
	LDI	@CONST+14,R2
	LSH	R1,R2,R1
	LDA	*-FP(4),AR0
	OR	*+AR0(8),R1
	STI	R1,*+AR0(8)
	.line	17
	STIK	0,*+FP(1)
	LDI	100,R1
	CMPI	R1,*+FP(1)
	BHS	L132
L131:
	.line	18
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	17
	CMPI	100,R0
	BLO	L131
L132:
	.line	22
	LDP	CONST+13
	LDI	@CONST+13,R0
	LSH	*-FP(19),R0
	LDA	*-FP(4),AR0
	OR	*+AR0(8),R0
	STI	R0,*+AR0(8)
	.line	25
	STIK	0,*+FP(1)
	LDI	100,R0
	CMPI	R0,*+FP(1)
	BHS	EPI0_11
L133:
	.line	26
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	25
	CMPI	100,R0
	BLO	L133
EPI0_11:
	.line	27
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	611,000000000H,1

	.sym	_CommFlush,_CommFlush,36,2,0
	.globl	_CommFlush

	.func	617
******************************************************
* FUNCTION DEF : _CommFlush
******************************************************
_CommFlush:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
	.sym	_config,-18,8,9,544,.fake1
	.sym	_Which,-19,4,9,32
	.sym	_i,1,4,1,32
	.sym	_TimeOut,2,4,1,32
	.line	2
	.line	14
	LDA	*-FP(19),AR0
	LSH	4,AR0,AR1
	LDP	CONST+15
	LDA	@CONST+15,IR1
	STIK	8,*+AR1(IR1)
	.line	16
	LSH	4,AR0,AR1
	LDI	7680,R0
	TSTB	R0,*+AR1(IR1)
	BZ	L136
L135:
	.line	17
	LDA	*-FP(19),AR0
	LSH	4,AR0,AR1
	LDP	CONST+16
	LDA	@CONST+16,IR0
	LDI	*+AR1(IR0),R0
	STI	R0,*+FP(1)
	.line	16
	LSH	4,AR0,AR1
	LDP	CONST+15
	LDA	@CONST+15,IR1
	LDI	*+AR1(IR1),R1
	TSTB	7680,R1
	BNZ	L135
L136:
	.line	26
	LDP	CONST+13
	LDI	@CONST+13,R0
	LSH	*-FP(19),R0
	LDA	*-FP(4),AR1
	TSTB	R0,*+AR1(8)
	BZ	L137
	.line	29
	LSH	4,AR0,AR2
	LDI	256,R0
	TSTB	R0,*+AR2(IR1)
	BNZ	L138
	.line	30
	LSH	4,AR0,AR2
	LDP	CONST+17
	LDA	@CONST+17,IR0
	STIK	0,*+AR2(IR0)
L138:
	.line	32
	STIK	0,*+FP(2)
	B	L140
L139:
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
L140:
	LDI	1000,R0
	CMPI	*+FP(2),R0
	BLE	LL19
	LDA	*-FP(19),AR0
	LSH	4,AR0,AR1
	LDP	CONST+15
	LDA	@CONST+15,IR1
	LDI	*+AR1(IR1),R1
	TSTB	4,R1
	BNZ	L139
LL19:
	.line	33
	CMPI	*+FP(2),R0
	BNZ	L137
	.line	34
	LDI	0,R0
	B	EPI0_12
L137:
	.line	39
	LDP	CONST+13
	LDI	@CONST+13,R0
	LSH	*-FP(19),R0
	LDA	*-FP(4),AR0
	LDI	*+AR0(8),R1
	ANDN	R0,R1,R0
	STI	R0,*+AR0(8)
	.line	43
	LDI	*-FP(19),R0
	MPYI	3,R0,R1
	LDI	3585,R2
	LSH	R1,R2,R1
	OR	*+AR0(8),R1
	STI	R1,*+AR0(8)
	.line	46
	LDP	CONST+18
	LDI	@CONST+18,R1
	LSH	R0,R1,R3
	OR	*+AR0(8),R3
	STI	R3,*+AR0(8)
	.line	49
	STIK	0,*+FP(1)
	LDI	100,R3
	CMPI	R3,*+FP(1)
	BGE	L143
L142:
	.line	50
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	49
	CMPI	100,R0
	BLT	L142
L143:
	.line	53
	LDP	CONST+13
	LDI	@CONST+13,R0
	LSH	*-FP(19),R0
	LDA	*-FP(4),AR0
	OR	*+AR0(8),R0
	STI	R0,*+AR0(8)
	.line	56
	STIK	0,*+FP(1)
	LDI	100,R0
	CMPI	R0,*+FP(1)
	BGE	L145
L144:
	.line	57
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	56
	CMPI	100,R0
	BLT	L144
L145:
	.line	60
	LDP	CONST+18
	LDI	@CONST+18,R0
	LSH	*-FP(19),R0
	LDA	*-FP(4),AR0
	LDI	*+AR0(8),R1
	ANDN	R0,R1,R0
	STI	R0,*+AR0(8)
	.line	63
	LDA	*-FP(19),AR1
	LSH	4,AR1,AR2
	LDP	CONST+15
	LDA	@CONST+15,IR0
	LDI	*+AR2(IR0),R0
	TSTB	256,R0
	BNZ	L146
	.line	64
	LSH	4,AR1,AR2
	LDP	CONST+17
	LDA	@CONST+17,IR1
	STIK	0,*+AR2(IR1)
L146:
	.line	67
	STIK	0,*+FP(2)
	B	L148
L147:
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
L148:
	LDI	1000,R0
	CMPI	*+FP(2),R0
	BLE	LL20
	LDA	*-FP(19),AR0
	LSH	4,AR0,AR1
	LDP	CONST+15
	LDA	@CONST+15,IR0
	LDI	*+AR1(IR0),R1
	TSTB	4,R1
	BNZ	L147
LL20:
	.line	68
	CMPI	*+FP(2),R0
	BNZ	L149
	.line	69
	LDI	0,R0
	B	EPI0_12
L149:
	.line	73
	LDP	CONST+13
	LDI	@CONST+13,R1
	LSH	*-FP(19),R1
	LDA	*-FP(4),AR0
	LDI	*+AR0(8),R2
	ANDN	R1,R2,R1
	STI	R1,*+AR0(8)
	.line	76
	LDA	*-FP(19),AR1
	LSH	4,AR1,AR2
	LDP	CONST+15
	LDA	@CONST+15,IR1
	STIK	0,*+AR2(IR1)
	.line	78
	LDI	1,R0
EPI0_12:
	.line	79
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	4,SP
	B	R1
	.endfunc	695,000000000H,2

	.sym	_copy,_copy,32,2,0
	.globl	_copy

	.func	701
******************************************************
* FUNCTION DEF : _copy
******************************************************
_copy:
	PUSH	FP
	LDI	SP,FP
	PUSH	R4
	PUSH	AR4
	PUSH	AR5
	.sym	_parms,-2,31,9,32
	.sym	_source,12,31,4,32
	.sym	_destination,13,31,4,32
	.sym	_length,4,15,4,32
	.line	2
	.line	5
	LDA	*-FP(2),AR0
	LDA	*AR0,AR4
	.line	6
	LDA	*+AR0(1),AR5
	.line	7
	LDI	*+AR0(2),R4
	.line	9
	CMPI	-1,R4
	BZ	LL23
	CMPI	0,R4
	BNZ	L150
LL23:
	.line	10
	B	EPI0_13
L150:
	.line	12
	LDI	R4,R0
	SUBI	1,R4
	CMPI	0,R0
	BZ	EPI0_13
L151:
	.line	13
	LDI	*AR4++,R0
	STI	R0,*AR5++
	.line	12
	LDI	R4,R0
	SUBI	1,R4
	CMPI	0,R0
	BNZ	L151
EPI0_13:
	.line	14
	LDI	*-FP(1),R1
	LDI	*FP,FP
	POP	AR5
	POP	AR4
	POP	R4
	SUBI	2,SP
	B	R1
	.endfunc	714,000003010H,0

	.sym	_HostIntTrap,_HostIntTrap,32,2,0
	.globl	_HostIntTrap

	.func	718
******************************************************
* FUNCTION DEF : _HostIntTrap
******************************************************
_HostIntTrap:
	PUSH	FP
	LDI	SP,FP
	.line	3
	LDP	_msg
	LDI	@_msg,R0
	ADDI	15,R0
	PUSH	R0
	CALL	_wait
	SUBI	1,SP
L153:
	.line	5
	LDI	131,R0
	PUSH	R0
	CALL	_readVIC
	SUBI	1,SP
	LDI	1,R1
	LDP	_VMEInt
	LSH	@_VMEInt,R1
	TSTB	R1,R0
	BNZ	L153
	.line	7
	LDP	_VMEInt+1
	LDI	@_VMEInt+1,R0
	PUSH	R0
	LDP	_VMEInt
	LDI	@_VMEInt,R1
	SUBI	1,R1
	LSH	2,R1
	ADDI	135,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	9
	LDI	1,R0
	LDP	_VMEInt
	LSH	@_VMEInt,R0
	OR	1,R0
	PUSH	R0
	LDI	131,R0
	PUSH	R0
	CALL	_writeVIC
	SUBI	2,SP
	.line	11
	LDP	_msg
	LDI	@_msg,R0
	ADDI	15,R0
	PUSH	R0
	CALL	_signal
	SUBI	1,SP
EPI0_14:
	.line	12
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	729,000000000H,0

	.sym	_VMEInt,_VMEInt,8,2,64,.fake6
	.globl	_VMEInt
	.bss	_VMEInt,2

	.sym	_msg,_msg,24,3,32,.fake5
	.bss	_msg,1
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.sect	".const"
CONST:
	.word 	_config          ;0
	.word 	LL4              ;1
	.word 	1447244848       ;2
	.word 	540028465        ;3
	.word 	LL6              ;4
	.word 	__stack          ;5
	.word 	0fffc0000h       ;6
	.word 	0b0000000h       ;7
	.word 	-1073741887      ;8
	.word 	58367            ;9
	.word 	32768            ;10
	.word 	49152            ;11
	.word 	07fffffffh       ;12
	.word 	268435456        ;13
	.word 	265729           ;14
	.word 	1048640          ;15
	.word 	1048641          ;16
	.word 	1048642          ;17
	.word 	262144           ;18
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_halt
	.globl	_wait
	.globl	_signal
	.end
