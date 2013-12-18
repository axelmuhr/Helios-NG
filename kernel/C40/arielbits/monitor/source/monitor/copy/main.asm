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
	.globl	_monitor_set
	.globl	_call_set
	.globl	_breaks
	.globl	_brk_addrs
	.globl	_key_ptr
	.globl	_crctab

	.sym	_main,_main,36,2,0
	.globl	_main

	.func	15
;>>>> 	main( void )
;>>>> 		int i, j;
******************************************************
* FUNCTION DEF : _main
******************************************************
_main:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
	.sym	_i,1,4,1,32
	.sym	_j,2,4,1,32
	.line	6
;>>>> 		setupVICVAC();
	CALL	_setupVICVAC
	.line	8
;>>>> 		for( j=0 ; j < 2 ; j++ )
	STIK	0,*+FP(2)
	CMPI	2,*+FP(2)
	BGE	L2
L1:
	.line	10
;>>>> 			LED( GREEN, ON );
	LDI	1,R0
	PUSH	R0
	LDI	2,R1
	PUSH	R1
	CALL	_LED
	SUBI	2,SP
	.line	11
;>>>> 			LED( RED, OFF );
	LDI	0,R0
	PUSH	R0
	LDI	1,R1
	PUSH	R1
	CALL	_LED
	SUBI	2,SP
	.line	12
;>>>> 			for( i=0 ; i < 0x80000 ; i++ );		
	STIK	0,*+FP(1)
	LDI	@CONST+0,R0
	CMPI	R0,*+FP(1)
	BGE	L4
L3:
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	@CONST+0,R0
	BLT	L3
L4:
	.line	14
;>>>> 			LED( GREEN, OFF );
	LDI	0,R0
	PUSH	R0
	LDI	2,R1
	PUSH	R1
	CALL	_LED
	SUBI	2,SP
	.line	15
;>>>> 			LED( RED, ON );
	LDI	1,R0
	PUSH	R0
	PUSH	R0
	CALL	_LED
	SUBI	2,SP
	.line	16
;>>>> 			for( i=0 ; i < 0x80000 ; i++ );
	STIK	0,*+FP(1)
	LDI	@CONST+0,R0
	CMPI	R0,*+FP(1)
	BGE	L6
L5:
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	@CONST+0,R0
	BLT	L5
L6:
	.line	8
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	CMPI	2,R0
	BLT	L1
L2:
	.line	19
;>>>> 		LED( GREEN, OFF );
	LDI	0,R0
	PUSH	R0
	LDI	2,R1
	PUSH	R1
	CALL	_LED
	SUBI	2,SP
	.line	20
;>>>> 		LED( RED, OFF );
	LDI	0,R0
	PUSH	R0
	LDI	1,R1
	PUSH	R1
	CALL	_LED
	SUBI	2,SP
	.line	23
;>>>> 		SetIntTable( 0x2ff800 );
	LDI	@CONST+1,R0
	PUSH	R0
	CALL	_SetIntTable
	SUBI	1,SP
	.line	24
;>>>> 		SetIntVect( 1, c_int01 );	
	LDI	@CONST+2,R0
	PUSH	R0
	LDI	1,R1
	PUSH	R1
	CALL	_SetIntVect
	SUBI	2,SP
	.line	26
;>>>> 		mk_crctbl( CRC16, crchware );
	LDI	@CONST+3,R0
	PUSH	R0
	LDI	4129,R1
	PUSH	R1
	CALL	_mk_crctbl
	SUBI	2,SP
	.line	28
;>>>> 		for( i=0 ; i < 5 ; i++ )
	STIK	0,*+FP(1)
	CMPI	5,*+FP(1)
	BGE	L8
L7:
	.line	30
;>>>> 			c40_printf( "%c\n", BEEP );
	LDI	7,R0
	PUSH	R0
	LDI	@CONST+4,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	.line	31
;>>>> 			for( j=0 ; j < (int)1e4 ; j++ );
	STIK	0,*+FP(2)
	LDI	10000,R0
	CMPI	*+FP(2),R0
	BLE	L10
L9:
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	CMPI	10000,R0
	BLT	L9
L10:
	.line	28
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	5,R0
	BLT	L7
L8:
	.line	34
;>>>> 		key_ptr = 0;
	STIK	0,@_key_ptr
	.line	36
;>>>> 		zero_regs( &monitor_set );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_zero_regs
	SUBI	1,SP
	.line	37
;>>>> 		zero_regs( &call_set );
	LDI	@CONST+6,R0
	PUSH	R0
	CALL	_zero_regs
	SUBI	1,SP
	.line	39
;>>>> 		for( i=0 ; i < MAX_BREAKS ; i++ ) /* Reset all breakpoints */
	STIK	0,*+FP(1)
	CMPI	8,*+FP(1)
	BGE	L12
L11:
	.line	41
;>>>> 			breaks[i] = 0;
	LDA	*+FP(1),IR0
	LDA	@CONST+7,AR0
	STIK	0,*+AR0(IR0)
	.line	42
;>>>> 			brk_addrs[i] = 0;
	LDA	@CONST+8,AR1
	STIK	0,*+AR1(IR0)
	.line	39
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	8,R0
	BLT	L11
L12:
	.line	67
;>>>> 		init();    /* Turns Cache on */
	CALL	_init
	.line	69
;>>>> 		config.uartA.baud = 7;
	STIK	7,@_config
	.line	70
;>>>> 		config.uartB.baud = 7;
	STIK	7,@_config+3
	.line	71
;>>>> 		config.uartA.parity = 1;
	STIK	1,@_config+1
	.line	72
;>>>> 		config.uartB.parity = 1;
	STIK	1,@_config+4
	.line	73
;>>>> 		config.uartA.bits = 7;
	STIK	7,@_config+2
	.line	74
;>>>> 		config.uartB.bits = 7;
	STIK	7,@_config+5
	.line	75
;>>>> 		config.cpu_clock = 40;
	LDI	40,R0
	STI	R0,@_config+7
	.line	76
;>>>> 		config.sram1_size = 64;
	LDI	64,R1
	STI	R1,@_config+9
	.line	77
;>>>> 		config.sram2_size = 64;
	STI	R1,@_config+10
	.line	78
;>>>> 		config.sram3_size = 64;
	STI	R1,@_config+11
	.line	79
;>>>> 		config.sram4_size = 64;
	STI	R1,@_config+12
	.line	80
;>>>> 		config.dram_size = 1;
	STIK	1,@_config+6
	.line	81
;>>>> 		config.daughter = 0;
	STIK	0,@_config+15
	.line	82
;>>>> 		config.l_jtag_base = 0xdeadbeef;
	LDI	@CONST+9,R2
	STI	R2,@_config+14
	.line	83
;>>>> 		config.l_dram_base = 0x8d000000;
	LDI	@CONST+10,R3
	STI	R3,@_config+13
	.line	85
;>>>> 		read_config( &config ); 
	LDI	@CONST+11,R9
	PUSH	R9
	CALL	_read_config
	SUBI	1,SP
	.line	87
;>>>> 		monitor( &config );
	LDI	@CONST+11,R0
	PUSH	R0
	CALL	_monitor
	SUBI	1,SP
EPI0_1:
	.line	88
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	4,SP
	B	R1
	.endfunc	102,000000000H,2

	.sym	_zero_regs,_zero_regs,32,2,0
	.globl	_zero_regs

	.func	108
;>>>> 	void zero_regs( reg_set *regs )
******************************************************
* FUNCTION DEF : _zero_regs
******************************************************
_zero_regs:
	PUSH	FP
	LDI	SP,FP
	.sym	_regs,-2,24,9,32,.fake4
	.line	2
	.line	3
;>>>> 		regs->DSP_ir0 = 0;
	LDA	*-FP(2),AR0
	STIK	0,*AR0
	.line	4
;>>>> 		regs->DSP_ir1 = 0;
	STIK	0,*+AR0(1)
	.line	5
;>>>> 		regs->DSP_ir2 = 0;
	STIK	0,*+AR0(2)
	.line	6
;>>>> 		regs->DSP_ir3 = 0;
	STIK	0,*+AR0(3)
	.line	7
;>>>> 		regs->DSP_ir4 = 0;
	STIK	0,*+AR0(4)
	.line	8
;>>>> 		regs->DSP_ir5 = 0;
	STIK	0,*+AR0(5)
	.line	9
;>>>> 		regs->DSP_ir6 = 0;
	STIK	0,*+AR0(6)
	.line	10
;>>>> 		regs->DSP_ir7 = 0;
	STIK	0,*+AR0(7)
	.line	11
;>>>> 		regs->DSP_ir8 = 0;
	STIK	0,*+AR0(8)
	.line	12
;>>>> 		regs->DSP_ir9 = 0;
	STIK	0,*+AR0(9)
	.line	13
;>>>> 		regs->DSP_ir10 = 0;
	STIK	0,*+AR0(10)
	.line	14
;>>>> 		regs->DSP_ir11 = 0;
	STIK	0,*+AR0(11)
	.line	16
;>>>> 		regs->DSP_fr0 = 0x7FFF0000;
	LDI	@CONST+12,R0
	STI	R0,*+AR0(12)
	.line	17
;>>>> 		regs->DSP_fr1 = 0x7FFF0000;
	STI	R0,*+AR0(13)
	.line	18
;>>>> 		regs->DSP_fr2 = 0x7FFF0000;
	STI	R0,*+AR0(14)
	.line	19
;>>>> 		regs->DSP_fr3 = 0x7FFF0000;
	STI	R0,*+AR0(15)
	.line	20
;>>>> 		regs->DSP_fr4 = 0x7FFF0000;
	STI	R0,*+AR0(16)
	.line	21
;>>>> 		regs->DSP_fr5 = 0x7FFF0000;
	STI	R0,*+AR0(17)
	.line	22
;>>>> 		regs->DSP_fr6 = 0x7FFF0000;
	STI	R0,*+AR0(18)
	.line	23
;>>>> 		regs->DSP_fr7 = 0x7FFF0000;
	STI	R0,*+AR0(19)
	.line	24
;>>>> 		regs->DSP_fr8 = 0x7FFF0000;
	STI	R0,*+AR0(20)
	.line	25
;>>>> 		regs->DSP_fr9 = 0x7FFF0000;
	STI	R0,*+AR0(21)
	.line	26
;>>>> 		regs->DSP_fr10 = 0x7FFF0000;
	STI	R0,*+AR0(22)
	.line	27
;>>>> 		regs->DSP_fr11 = 0x7FFF0000;
	STI	R0,*+AR0(23)
	.line	29
;>>>> 		regs->DSP_ar0 = 0;
	STIK	0,*+AR0(24)
	.line	30
;>>>> 		regs->DSP_ar1 = 0;
	STIK	0,*+AR0(25)
	.line	31
;>>>> 		regs->DSP_ar2 = 0;
	STIK	0,*+AR0(26)
	.line	32
;>>>> 		regs->DSP_ar3 = 0;
	STIK	0,*+AR0(27)
	.line	33
;>>>> 		regs->DSP_ar4 = 0;
	STIK	0,*+AR0(28)
	.line	34
;>>>> 		regs->DSP_ar5 = 0;
	STIK	0,*+AR0(29)
	.line	35
;>>>> 		regs->DSP_ar6 = 0;
	STIK	0,*+AR0(30)
	.line	36
;>>>> 		regs->DSP_ar7 = 0;
	STIK	0,*+AR0(31)
	.line	38
;>>>> 		regs->DSP_DP = 0;
	STIK	0,*+AR0(32)
	.line	39
;>>>> 		regs->DSP_IR0 = 0;
	STIK	0,*+AR0(33)
	.line	40
;>>>> 		regs->DSP_IR1 = 0;
	STIK	0,*+AR0(34)
	.line	41
;>>>> 		regs->DSP_BK = 0;
	STIK	0,*+AR0(35)
	.line	42
;>>>> 		regs->DSP_SP = 0;
	STIK	0,*+AR0(36)
	.line	44
;>>>> 		regs->DSP_ST = 0;
	STIK	0,*+AR0(37)
	.line	45
;>>>> 		regs->DSP_DIE = 0;
	STIK	0,*+AR0(38)
	.line	46
;>>>> 		regs->DSP_IIE = 0;
	STIK	0,*+AR0(39)
	.line	47
;>>>> 		regs->DSP_IIF = 0;
	STIK	0,*+AR0(40)
	.line	49
;>>>> 		regs->DSP_RS = 0;
	STIK	0,*+AR0(41)
	.line	50
;>>>> 		regs->DSP_RE = 0;
	STIK	0,*+AR0(42)
	.line	51
;>>>> 		regs->DSP_RC = 0;
	STIK	0,*+AR0(43)
	.line	53
;>>>> 		regs->DSP_IVTP = 0;
	STIK	0,*+AR0(44)
	.line	54
;>>>> 		regs->DSP_TVTP = 0;
	STIK	0,*+AR0(45)
	.line	56
;>>>> 		regs->ret_add = 0;
	STIK	0,*+AR0(46)
EPI0_2:
	.line	57
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	164,000000000H,0

	.sym	_SetBCR,_SetBCR,32,2,0
	.globl	_SetBCR

	.func	168
;>>>> 	void SetBCR( unsigned long global, unsigned long local )
******************************************************
* FUNCTION DEF : _SetBCR
******************************************************
_SetBCR:
	PUSH	FP
	LDI	SP,FP
	.sym	_global,-2,15,9,32
	.sym	_local,-3,15,9,32
	.line	2
	.line	3
;>>>> 		*(unsigned long *)(0x100000) = global;
	LDA	@CONST+13,AR0
	LDI	*-FP(2),R0
	STI	R0,*AR0
	.line	4
;>>>> 		*(unsigned long *)(0x100004) = local;
	LDA	@CONST+14,AR1
	LDI	*-FP(3),R1
	STI	R1,*AR1
EPI0_3:
	.line	5
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	172,000000000H,0

	.sym	_bus_error_handler,_bus_error_handler,32,2,0
	.globl	_bus_error_handler

	.func	177
;>>>> 	void bus_error_handler( void )
******************************************************
* FUNCTION DEF : _bus_error_handler
******************************************************
_bus_error_handler:
	PUSH	FP
	LDI	SP,FP
EPI0_4:
	.line	3
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	179,000000000H,0

	.sym	_config,_config,8,2,512,.fake1
	.globl	_config
	.bss	_config,16

	.sym	_crctab,_crctab,61,2,8192,,256
	.globl	_crctab
	.bss	_crctab,256

	.sym	_breaks,_breaks,63,2,256,,8
	.globl	_breaks
	.bss	_breaks,8

	.sym	_call_set,_call_set,8,2,1504,.fake4
	.globl	_call_set
	.bss	_call_set,47

	.sym	_brk_addrs,_brk_addrs,63,2,256,,8
	.globl	_brk_addrs
	.bss	_brk_addrs,8

	.sym	_monitor_set,_monitor_set,8,2,1504,.fake4
	.globl	_monitor_set
	.bss	_monitor_set,47
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.sect	".const"
SL0:	.byte	"%c",10,0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,15
	.sect	".cinit"
	.word	15,CONST
	.word 	524288           ;0
	.word 	3143680          ;1
	.word 	_c_int01         ;2
	.word 	_crchware        ;3
	.word 	SL0              ;4
	.word 	_monitor_set     ;5
	.word 	_call_set        ;6
	.word 	_breaks          ;7
	.word 	_brk_addrs       ;8
	.word 	-559038737       ;9
	.word 	-1929379840      ;10
	.word 	_config          ;11
	.word 	2147418112       ;12
	.word 	1048576          ;13
	.word 	1048580          ;14
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_setupVICVAC
	.globl	_LED
	.globl	_SetIntTable
	.globl	_SetIntVect
	.end
