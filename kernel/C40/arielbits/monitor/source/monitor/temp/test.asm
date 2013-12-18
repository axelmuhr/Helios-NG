******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 test.c test.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o test.if test.asm test.tmp 
	.version	40
FP	.set		AR3
	.file	"test.c"
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
	.file	"c:\c40\math.h"
	.globl	_asin
	.globl	_acos
	.globl	_atan
	.globl	_atan2
	.globl	_ceil
	.globl	_cos
	.globl	_cosh
	.globl	_exp
	.globl	_fabs
	.globl	_floor
	.globl	_fmod
	.globl	_frexp
	.globl	_ldexp
	.globl	_log
	.globl	_log10
	.globl	_modf
	.globl	_pow
	.globl	_sin
	.globl	_sinh
	.globl	_sqrt
	.globl	_tan
	.globl	_tanh
	.file	"test.c"

	.sect	".cinit"
	.word	IS1,_booted
	.word	0
	.word	0
	.word	0
IS1	.set	3

	.sym	_booted,_booted,52,2,96,,3
	.globl	_booted
	.bss	_booted,3
	.text

	.sym	_test,_test,36,2,0
	.globl	_test

	.func	13
;>>>> 	int test( hydra_conf config, char out_where )
******************************************************
* FUNCTION DEF : _test
******************************************************
_test:
	PUSH	FP
	LDI	SP,FP
	ADDI	11,SP
	.sym	_config,-17,8,9,512,.fake1
	.sym	_out_where,-18,2,9,32
	.sym	_mem_ptr,1,31,1,32
	.sym	_i,2,15,1,32
	.sym	_j,3,15,1,32
	.sym	_k,4,15,1,32
	.sym	_sram_size,5,15,1,32
	.sym	_fail_addr,6,15,1,32
	.sym	_failed,7,15,1,32
	.sym	_MemResults,8,8,1,128,.fake2
	.line	2
;>>>> 		unsigned long *mem_ptr;
;>>>> 		unsigned long i, j, k, sram_size;
	.line	5
;>>>> 		unsigned long fail_addr, failed=FALSE;
;>>>> 		MemTestStruct MemResults;
	STIK	0,*+FP(7)
	.line	9
;>>>> 		c40_printf( "\n\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	11
;>>>> 		LED( RED, OFF );
	LDI	0,R0
	PUSH	R0
	LDI	1,R1
	PUSH	R1
	CALL	_LED
	SUBI	2,SP
	.line	12
;>>>> 		LED( GREEN, OFF );
	LDI	0,R0
	PUSH	R0
	LDI	2,R1
	PUSH	R1
	CALL	_LED
	SUBI	2,SP
	.line	15
;>>>> 		c40_printf( "Testing DRAM .... " );
	LDI	@CONST+1,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	16
;>>>> 		if( MemTest( 0x8d000000, config.dram_size*0x100000, &MemResults ) )
	LDI	FP,R0
	ADDI	8,R0
	PUSH	R0
	LDI	*-FP(11),R0
	LSH	20,R0
	PUSH	R0
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_MemTest
	SUBI	3,SP
	CMPI	0,R0
	BZ	L1
	.line	18
;>>>> 			c40_printf( "Failed at address %x during the %s test.\n", MemResults.FailAddress, MemResults.TestFailed?"Write Address":"Walking Bits" );
	LDI	*+FP(9),R0
	LDINZ	@CONST+3,R1
	LDIZ	@CONST+4,R1
	PUSH	R1
	LDI	*+FP(8),R1
	PUSH	R1
	LDI	@CONST+5,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	3,SP
	.line	19
;>>>> 			c40_printf( "Written value  = %x, Read value = %x\n", MemResults.Written, MemResults.Read );
	LDI	*+FP(11),R0
	PUSH	R0
	LDI	*+FP(10),R1
	PUSH	R1
	LDI	@CONST+6,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	3,SP
	.line	20
;>>>> 			LED( RED, ON );
	LDI	1,R0
	PUSH	R0
	PUSH	R0
	CALL	_LED
	SUBI	2,SP
	.line	21
;>>>> 			return( 0 );
;>>>> 		else
	LDI	0,R0
	B	EPI0_1
L1:
	.line	24
;>>>> 			c40_printf( "Passed\n" );
	LDI	@CONST+7,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	26
;>>>> 		c40_printf( "Testing processor 1 ....\n" );
	LDI	@CONST+8,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	28
;>>>> 		c40_printf( "   Global SRAM ... " );
	LDI	@CONST+9,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	29
;>>>> 		if( MemTest( 0xc0000000, config.sram1_size*1024, &MemResults ) )
	LDI	FP,R0
	ADDI	8,R0
	PUSH	R0
	LDI	*-FP(8),R0
	LSH	10,R0,R1
	PUSH	R1
	LDI	@CONST+10,R1
	PUSH	R1
	CALL	_MemTest
	SUBI	3,SP
	CMPI	0,R0
	BZ	L2
	.line	31
;>>>> 			c40_printf( "Failed at address %x during the %s test.\n", MemResults.FailAddress, MemResults.TestFailed?"Write Address":"Walking Bits" );
	LDI	*+FP(9),R0
	LDINZ	@CONST+3,R1
	LDIZ	@CONST+4,R1
	PUSH	R1
	LDI	*+FP(8),R1
	PUSH	R1
	LDI	@CONST+5,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	3,SP
	.line	32
;>>>> 			c40_printf( "Written value  = %x, Read value = %x\n", MemResults.Written, MemResults.Read );
	LDI	*+FP(11),R0
	PUSH	R0
	LDI	*+FP(10),R1
	PUSH	R1
	LDI	@CONST+6,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	3,SP
	.line	33
;>>>> 			LED( RED, ON );
	LDI	1,R0
	PUSH	R0
	PUSH	R0
	CALL	_LED
	SUBI	2,SP
	.line	34
;>>>> 			failed = TRUE;
;>>>> 		else
	STIK	1,*+FP(7)
	B	L3
L2:
	.line	37
;>>>> 			c40_printf( "Passed\n" );
	LDI	@CONST+7,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L3:
	.line	39
;>>>> 	return( 1 );
;>>>> 		CommFlush( config );
;>>>> 		if( !reset_others() )
;>>>> 			c40_printf( "Error Writing Hydra control register.\n" );
;>>>> 			LED( RED, ON );
;>>>> 			failed = TRUE;
	LDI	1,R0
	B	EPI0_1
	.line	53
;>>>> 		c40_printf( "Booting other processor%s ... ", config.daughter?"s":"" );
	.line	54
;>>>> 		if( i = BootOthers( config, booted ) )
	.line	56
;>>>> 			c40_printf( "Failed booting processor %d\n", i );
	.line	57
;>>>> 			LED( RED, ON );
	.line	58
;>>>> 			failed = TRUE;
	.line	59
;>>>> 			return(0);
;>>>> 		else
	.line	62
;>>>> 			c40_printf( "Successful\n" );	
	.line	64
;>>>> 		if( config.daughter )
	.line	65
;>>>> 			k = 3;
;>>>> 		else
	.line	67
;>>>> 			k = 1;
	.line	69
;>>>> 		for( i=0 ; i < k ; i++ )
;>>>> 			switch( i )
;>>>> 				case 0 :
L8:
	B	L10
L11:
	.line	74
;>>>> 					comm_sen( 0, config.sram2_size, NumTries );
	LDI	0,R0
	PUSH	R0
	LDI	*-FP(7),R1
	PUSH	R1
	PUSH	R0
	CALL	_comm_sen
	SUBI	3,SP
	.line	75
;>>>> 					break;
;>>>> 				case 1 :
	B	L12
L13:
	.line	77
;>>>> 					comm_sen( 1, config.sram3_size, NumTries );
	LDI	0,R0
	PUSH	R0
	LDI	*-FP(6),R1
	PUSH	R1
	LDI	1,R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	.line	78
;>>>> 					break;
;>>>> 				case 2 :
	B	L12
L14:
	.line	80
;>>>> 					comm_sen( 2, config.sram4_size, NumTries );
	LDI	0,R0
	PUSH	R0
	LDI	*-FP(5),R1
	PUSH	R1
	LDI	2,R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	.line	81
;>>>> 					break;
	B	L12
L10:
	.line	71
	LDI	*+FP(2),R0
	BZ	L11
	CMPI	1,R0
	BZ	L13
	CMPI	2,R0
	BZ	L14
L12:
	.line	83
;>>>> 			comm_sen( i, config.dram_size, NumTries );
	LDI	0,R0
	PUSH	R0
	LDI	*-FP(11),R1
	PUSH	R1
	LDI	*+FP(2),R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	.line	84
;>>>> 			comm_sen( i, config.l_dram_base, NumTries );
	LDI	0,R0
	PUSH	R0
	LDI	*-FP(4),R1
	PUSH	R1
	LDI	*+FP(2),R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	.line	86
;>>>> 			for( j=0 ; j < 2 ; j++ )
	STIK	0,*+FP(3)
	CMPI	2,*+FP(3)
	BHS	L16
L15:
	.line	88
;>>>> 				c40_printf( "Testing %s SRAM on DSP %d ... ", j?"local":"global", 2+i );
	LDI	*+FP(2),R0
	ADDI	2,R0
	PUSH	R0
	LDI	*+FP(3),R0
	LDINZ	@CONST+17,R1
	LDIZ	@CONST+18,R1
	PUSH	R1
	LDI	@CONST+19,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	3,SP
	.line	89
;>>>> 				comm_rec( i, &fail_addr, (int)1e6 );
	LDI	@CONST+20,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	6,R1
	PUSH	R1
	LDI	*+FP(2),R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	3,SP
	.line	90
;>>>> 				if( fail_addr )
	LDI	*+FP(6),R0
	BZ	L17
	.line	92
;>>>> 					failed = TRUE;
	STIK	1,*+FP(7)
	.line	93
;>>>> 					c40_printf( "Failed\n     Failed at address %x.\n", fail_addr );
;>>>> 				else
	PUSH	R0
	LDI	@CONST+21,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	B	L18
L17:
	.line	96
;>>>> 					c40_printf( "Passed\n" );
	LDI	@CONST+7,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	1,SP
L18:
	.line	86
	ADDI	1,*+FP(3),R0
	STI	R0,*+FP(3)
	CMPI	2,R0
	BLO	L15
L16:
	.line	69
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	CMPI	*+FP(4),R0
	BLO	L8
	.line	101
;>>>> 		c40_printf( "\n\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	103
;>>>> 		if( failed )
	LDI	*+FP(7),R0
	BZ	L19
	.line	106
;>>>> 			LED( GREEN, OFF );
	LDI	0,R1
	PUSH	R1
	LDI	2,R2
	PUSH	R2
	CALL	_LED
	SUBI	2,SP
	.line	107
;>>>> 			LED( RED, ON );		
	LDI	1,R0
	PUSH	R0
	PUSH	R0
	CALL	_LED
	SUBI	2,SP
	.line	108
;>>>> 			return( 0 );
;>>>> 		else
	LDI	0,R0
	B	EPI0_1
L19:
	.line	113
;>>>> 			LED( GREEN, ON );
	LDI	1,R1
	PUSH	R1
	LDI	2,R2
	PUSH	R2
	CALL	_LED
	SUBI	2,SP
	.line	114
;>>>> 			LED( RED, OFF );
	LDI	0,R0
	PUSH	R0
	LDI	1,R1
	PUSH	R1
	CALL	_LED
	SUBI	2,SP
	.line	115
;>>>> 			return( 1 );
	LDI	1,R0
EPI0_1:
	.line	117
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	13,SP
	B	R1
	.endfunc	129,000000000H,11

	.sym	_reset_others,_reset_others,36,2,0
	.globl	_reset_others

	.func	133
;>>>> 	int reset_others( void )
;>>>> 		unsigned long leds, i;
******************************************************
* FUNCTION DEF : _reset_others
******************************************************
_reset_others:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
	.sym	_leds,1,15,1,32
	.sym	_i,2,15,1,32
	.line	5
;>>>> 		leds = (*(unsigned long *)0xbf7fc008) & 0x80200000;
	LDA	@CONST+22,AR0
	LDI	@CONST+23,R0
	AND	R0,*AR0,R1
	STI	R1,*+FP(1)
	.line	8
;>>>> 		*(unsigned long *)(0xbf7fc008) = 0x005FFE49 | leds;
	OR	@CONST+24,R1
	STI	R1,*AR0
	.line	9
;>>>> 		if( *(unsigned long *)(0xbf7fc008) != (0x005FFE49|leds))
	LDI	@CONST+24,R1
	OR	R1,*+FP(1),R2
	CMPI	R2,*AR0
	BZ	L20
	.line	10
;>>>> 			return( 0 );
	LDI	0,R0
	B	EPI0_2
L20:
	.line	12
;>>>> 		for( i=0 ; i < 1000 ; )
	STIK	0,*+FP(2)
	LDI	1000,R2
	CMPI	*+FP(2),R2
	BLS	L22
L21:
	.line	13
;>>>> 			i++;
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	.line	12
	CMPI	1000,R0
	BLO	L21
L22:
	.line	16
;>>>> 		*(unsigned long *)(0xbf7fc008) = 0x705FFE49 | leds;
	LDI	@CONST+25,R0
	OR	R0,*+FP(1),R1
	LDA	@CONST+22,AR0
	STI	R1,*AR0
	.line	17
;>>>> 		if( *(unsigned long *)(0xbf7fc008) != (0x705FFE49|leds))
	OR	R0,*+FP(1),R1
	CMPI	R1,*AR0
	BZ	L23
	.line	18
;>>>> 			return( 0 );
	LDI	0,R0
	B	EPI0_2
L23:
	.line	20
;>>>> 		return( 1 );	
	LDI	1,R0
EPI0_2:
	.line	21
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	4,SP
	B	R1
	.endfunc	153,000000000H,2

	.sym	_CommTest,_CommTest,36,2,0
	.globl	_CommTest

	.func	159
;>>>> 	int CommTest( hydra_conf config )
******************************************************
* FUNCTION DEF : _CommTest
******************************************************
_CommTest:
	PUSH	FP
	LDI	SP,FP
	ADDI	19,SP
	.sym	_config,-17,8,9,512,.fake1

	.sect	".cinit"
	.word	IS2,STATIC_1
	.word	0
	.word	-1
	.word	-1431655766
	.word	1431655765
IS2	.set	4

	.sym	.init0,STATIC_1,63,3,128,,4
	.bss	STATIC_1,4

	.word	IS3,STATIC_2
	.word	43
	.word	99
	.word	35
	.word	3
	.word	3
	.word	35
IS3	.set	6

	.sym	.init1,STATIC_2,63,3,192,,6
	.bss	STATIC_2,6

	.word	IS4,STATIC_3
	.word	0
	.word	1
	.word	2
IS4	.set	3

	.sym	.init2,STATIC_3,52,3,96,,3
	.bss	STATIC_3,3
	.text
	.sym	_data,1,63,1,128,,4
	.sym	_out_2,5,15,1,32
	.sym	_rout,6,63,1,192,,6
	.sym	_i_mesg,12,15,1,32
	.sym	_port,13,52,1,96,,3
	.sym	_out_port,16,4,1,32
	.sym	_i,17,4,1,32
	.sym	_j,18,4,1,32
	.sym	_k,19,4,1,32
	.line	3
;>>>> 		unsigned long data[nu_patn]={0x0,0xFFFFFFFF,0xAAAAAAAA,0x55555555};   /* data to be sent */
	LDI	@STATIC_1+0,R0
	STI	R0,*+FP(1)
	LDI	@STATIC_1+1,R0
	STI	R0,*+FP(2)
	LDI	@STATIC_1+2,R0
	STI	R0,*+FP(3)
	LDI	@STATIC_1+3,R0
	STI	R0,*+FP(4)
	.line	5
;>>>> 		unsigned long  out_2=3;         /* the route ID when without the daughter card attached */
	STIK	3,*+FP(5)
	.line	6
;>>>> 		unsigned long   rout[nu_rout]={0053,0143,043,003,003,043};   /* route IDs in octal */
;>>>> 		unsigned long   i_mesg;         /* the received message */
	LDI	@STATIC_2+0,R0
	STI	R0,*+FP(6)
	LDI	@STATIC_2+1,R0
	STI	R0,*+FP(7)
	LDI	@STATIC_2+2,R0
	STI	R0,*+FP(8)
	LDI	@STATIC_2+3,R0
	STI	R0,*+FP(9)
	LDI	@STATIC_2+4,R0
	STI	R0,*+FP(10)
	LDI	@STATIC_2+5,R0
	STI	R0,*+FP(11)
	.line	9
;>>>> 		int  port[numb]={0,1,2};        /* the connected port numbers in DSP1 */
	LDI	@STATIC_3+0,R0
	STI	R0,*+FP(13)
	LDI	@STATIC_3+1,R0
	STI	R0,*+FP(14)
	LDI	@STATIC_3+2,R0
	STI	R0,*+FP(15)
	.line	10
;>>>> 		int  out_port=0;
	STIK	0,*+FP(16)
	.line	12
;>>>> 		int i,j,k=0;
	STIK	0,*+FP(19)
	.line	14
;>>>> 		if (config.daughter)             /* Test All Four Processors */
	LDI	*-FP(2),R0
	BZ	L24
	.line	17
;>>>> 		 	for (i=0;i<nu_rout;i++)
	STIK	0,*+FP(17)
	CMPI	6,*+FP(17)
	BGE	L26
L25:
	.line	19
;>>>> 				if (!fmod(i,2))               /* if it is a forword or backword test ? */
	LDF	2.0,R0
	PUSHF	R0
	FLOAT	*+FP(17),R1
	PUSHF	R1
	CALL	_fmod
	SUBI	2,SP
	CMPF	0,R0
	BNZ	L27
	.line	20
;>>>> 					out_port=0;
;>>>> 				else 
	STIK	0,*+FP(16)
	B	L28
L27:
	.line	22
;>>>> 					out_port=k;
	LDI	*+FP(19),R0
	STI	R0,*+FP(16)
L28:
	.line	23
;>>>> 				for ( j=0;j<nu_patn;j++)
	STIK	0,*+FP(18)
	CMPI	4,*+FP(18)
	BGE	L30
L29:
	.line	25
;>>>> 					if (!comm_sen(out_port,rout[i], NumTries))   /* send the control word out */
	LDI	0,R0
	PUSH	R0
	LDA	*+FP(17),IR0
	ADDI	6,IR0
	LDI	*+FP(IR0),R1
	PUSH	R1
	LDI	*+FP(16),R1
	PUSH	R1
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L31
	.line	26
;>>>> 						return(0);
;>>>> 					while (!comm_rec(port[k],&i_mesg, NumTries))   /* wait for the control word coming back */
	LDI	0,R0
	B	EPI0_3
L31:
	B	L33
L32:
	.line	29
;>>>> 						k++;
	ADDI	1,*+FP(19),R0
	STI	R0,*+FP(19)
	.line	30
;>>>> 						if (k==numb)
	CMPI	3,R0
	BNZ	L33
	.line	31
;>>>> 							k=0;
	STIK	0,*+FP(19)
L33:
	.line	27
	LDI	0,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	12,R1
	PUSH	R1
	LDA	*+FP(19),IR1
	ADDI	13,IR1
	LDI	*+FP(IR1),R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	3,SP
	CMPI	0,R0
	BZ	L32
	.line	33
;>>>> 	  				if (!comm_sen(out_port,data[j], NumTries))   /* send the data out */	
	LDI	0,R0
	PUSH	R0
	LDA	*+FP(18),IR0
	ADDI	1,IR0
	LDI	*+FP(IR0),R1
	PUSH	R1
	LDI	*+FP(16),R1
	PUSH	R1
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L36
	.line	34
;>>>> 						return(0);
	LDI	0,R0
	B	EPI0_3
L36:
	.line	36
;>>>> 					while (!comm_rec(port[k],&i_mesg, NumTries));   /* dump the control word and get the data coming back */
	LDI	0,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	12,R1
	PUSH	R1
	LDA	*+FP(19),IR1
	ADDI	13,IR1
	LDI	*+FP(IR1),R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	3,SP
	CMPI	0,R0
	BZ	L36
	.line	38
;>>>> 					if (i_mesg!=data[j])       /* is there any error in the received data */
	LDA	*+FP(18),IR0
	ADDI	1,IR0
	LDI	*+FP(IR0),R0
	CMPI	*+FP(12),R0
	BZ	L37
	.line	39
;>>>> 						return(0);
	LDI	0,R0
	B	EPI0_3
L37:
	.line	23
	ADDI	1,*+FP(18),R0
	STI	R0,*+FP(18)
	CMPI	4,R0
	BLT	L29
L30:
	.line	17
	ADDI	1,*+FP(17),R0
	STI	R0,*+FP(17)
	CMPI	6,R0
	BLT	L25
L26:
	.line	43
;>>>> 			for (i=0;i<numb;i++)
	STIK	0,*+FP(17)
	CMPI	3,*+FP(17)
	BGE	L39
L38:
	.line	44
;>>>> 				if (!comm_sen(i,done, NumTries))      /* send the "done" flag to all other DSPs */
	LDI	0,R0
	PUSH	R0
	LDI	7,R1
	PUSH	R1
	LDI	*+FP(17),R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L40
	.line	45
;>>>> 					return(0);
	LDI	0,R0
	B	EPI0_3
L40:
	.line	43
	ADDI	1,*+FP(17),R0
	STI	R0,*+FP(17)
	CMPI	3,R0
	BLT	L38
L39:
	.line	47
;>>>> 			return(1);  
;>>>> 		else            /* Only test DSP 2 */
	LDI	1,R0
	B	EPI0_3
L24:
	.line	51
;>>>> 			for ( j=0;j<nu_patn;j++)
	STIK	0,*+FP(18)
	CMPI	4,*+FP(18)
	BGE	L42
L41:
	.line	53
;>>>> 				if (!comm_sen(out_port,out_2, NumTries))   /* send the control word out */
	LDI	0,R0
	PUSH	R0
	LDI	*+FP(5),R1
	PUSH	R1
	LDI	*+FP(16),R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L43
	.line	54
;>>>> 					return( 0 );
	LDI	0,R0
	B	EPI0_3
L43:
	.line	56
;>>>> 		                if( !comm_rec(out_port,&i_mesg, NumTries) )
	LDI	0,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	12,R1
	PUSH	R1
	LDI	*+FP(16),R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L44
	.line	57
;>>>> 					return( 0 );   /* Get the control word coming back */
	LDI	0,R0
	B	EPI0_3
L44:
	.line	59
;>>>> 				if( !comm_sen(out_port,data[j], NumTries))   /* send the data out */
	LDI	0,R0
	PUSH	R0
	LDA	*+FP(18),IR1
	ADDI	1,IR1
	LDI	*+FP(IR1),R1
	PUSH	R1
	LDI	*+FP(16),R1
	PUSH	R1
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L45
	.line	60
;>>>> 					return( 0 );
	LDI	0,R0
	B	EPI0_3
L45:
	.line	62
;>>>> 				if( !comm_rec(out_port,&i_mesg, NumTries) )
	LDI	0,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	12,R1
	PUSH	R1
	LDI	*+FP(16),R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L46
	.line	63
;>>>> 					return( 0 );   /* dump the control word and get the data coming back */
	LDI	0,R0
	B	EPI0_3
L46:
	.line	65
;>>>> 				if (i_mesg!=data[j])       /* is there any error in the received data */
	LDA	*+FP(18),IR0
	ADDI	1,IR0
	LDI	*+FP(IR0),R0
	CMPI	*+FP(12),R0
	BZ	L47
	.line	66
;>>>> 					return( 0 );
	LDI	0,R0
	B	EPI0_3
L47:
	.line	51
	ADDI	1,*+FP(18),R0
	STI	R0,*+FP(18)
	CMPI	4,R0
	BLT	L41
L42:
	.line	68
;>>>> 			if (!comm_sen(out_port,done, NumTries))      /* send the "done" flag to DSP2 */
	LDI	0,R0
	PUSH	R0
	LDI	7,R1
	PUSH	R1
	LDI	*+FP(16),R2
	PUSH	R2
	CALL	_comm_sen
	SUBI	3,SP
	CMPI	0,R0
	BNZ	L48
	.line	69
;>>>> 				return( 0 );
	LDI	0,R0
	B	EPI0_3
L48:
	.line	71
;>>>> 			return( 1 );
	LDI	1,R0
EPI0_3:
	.line	73
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	21,SP
	B	R1
	.endfunc	231,000000000H,19

	.sym	_CommFlush,_CommFlush,32,2,0
	.globl	_CommFlush

	.func	236
;>>>> 	void CommFlush( hydra_conf config )
;>>>> 		unsigned long ContRegVal, i, val;
******************************************************
* FUNCTION DEF : _CommFlush
******************************************************
_CommFlush:
	PUSH	FP
	LDI	SP,FP
	ADDI	3,SP
	.sym	_config,-17,8,9,512,.fake1
	.sym	_ContRegVal,1,15,1,32
	.sym	_i,2,15,1,32
	.sym	_val,3,15,1,32
	.line	5
;>>>> 		ContRegVal = (*(unsigned long *)(config.l_jtag_base + 8));
	LDA	*-FP(3),AR0
	LDI	*+AR0(8),R0
	STI	R0,*+FP(1)
	.line	9
;>>>> 		if( config.daughter )
	LDI	*-FP(2),R1
	BZ	L49
	.line	10
;>>>> 			i = 3;
;>>>> 		else
	STIK	3,*+FP(2)
	B	L50
L49:
	.line	12
;>>>> 			i = 1;
	STIK	1,*+FP(2)
L50:
	.line	13
;>>>> 		while( i --)
	SUBI	1,*+FP(2),R2
	STI	R2,*+FP(2)
	ADDI	1,R2
	BZ	EPI0_4
L51:
	.line	15
;>>>> 			if( !booted[i] )
	LDA	*+FP(2),IR1
	LDA	@CONST+14,AR0
	LDI	*+AR0(IR1),R0
	BZ	L54
	.line	16
;>>>> 				continue;
	.line	19
;>>>> 			*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal | (0x00040000<<i);
	LDI	@CONST+26,R0
	LSH	*+FP(2),R0,R1
	OR	*+FP(1),R1
	LDA	*-FP(3),AR1
	STI	R1,*+AR1(8)
	.line	22
;>>>> 			*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal & ~(0x00040000<<i);
	LSH	*+FP(2),R0,R1
	ANDN	R1,*+FP(1),R1
	STI	R1,*+AR1(8)
	.line	25
;>>>> 			*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal | (0x00040000<<i);
	LSH	*+FP(2),R0,R1
	OR	*+FP(1),R1
	STI	R1,*+AR1(8)
	.line	28
;>>>> 			while( *(unsigned long *)(0x100040+(i*0x10)) & 0x00001E00 )
	LSH	4,*+FP(2),AR2
	LDA	@CONST+27,IR0
	LDI	*+AR2(IR0),R1
	TSTB	7680,R1
	BZ	L56
L55:
	.line	29
;>>>> 				val = *(unsigned long *)(0x100041+(i*0x10));
	LSH	4,*+FP(2),AR0
	LDA	@CONST+28,IR1
	LDI	*+AR0(IR1),R0
	STI	R0,*+FP(3)
	.line	28
	LSH	4,*+FP(2),AR0
	LDA	@CONST+27,IR0
	LDI	*+AR0(IR0),R1
	TSTB	7680,R1
	BNZ	L55
L56:
	.line	32
;>>>> 			*(unsigned long *)(0x100042+(i*0x10)) = 0;
	LSH	4,*+FP(2),AR0
	LDA	@CONST+29,IR1
	STIK	0,*+AR0(IR1)
L57:
	.line	35
;>>>> 			while( *(unsigned long *)(0x100040+(i*0x10)) & 0x000001E0 );
	LSH	4,*+FP(2),AR0
	LDA	@CONST+27,IR0
	LDI	*+AR0(IR0),R0
	TSTB	480,R0
	BNZ	L57
	.line	38
;>>>> 			*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal & ~(0x00040000<<i);
	LDI	@CONST+26,R0
	LSH	*+FP(2),R0,R1
	ANDN	R1,*+FP(1),R1
	LDA	*-FP(3),AR0
	STI	R1,*+AR0(8)
	.line	41
;>>>> 			*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal | (0x00040000<<i);
	LSH	*+FP(2),R0,R1
	OR	*+FP(1),R1
	STI	R1,*+AR0(8)
L54:
	.line	13
	SUBI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	ADDI	1,R0
	BNZ	L51
EPI0_4:
	.line	45
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	5,SP
	B	R1
	.endfunc	280,000000000H,3
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.sect	".const"
SL0:	.byte	10,10,0
SL1:	.byte	"Testing DRAM .... ",0
SL2:	.byte	"Write Address",0
SL3:	.byte	"Walking Bits",0
SL4:	.byte	"Failed at address %x during the %s test.",10,0
SL5:	.byte	"Written value  = %x, Read value = %x",10,0
SL6:	.byte	"Passed",10,0
SL7:	.byte	"Testing processor 1 ....",10,0
SL8:	.byte	"   Global SRAM ... ",0
SL9:	.byte	"s",0
SL10:	.byte	0
SL11:	.byte	"Booting other processor%s ... ",0
SL12:	.byte	"Failed booting processor %d",10,0
SL13:	.byte	"Successful",10,0
SL14:	.byte	"local",0
SL15:	.byte	"global",0
SL16:	.byte	"Testing %s SRAM on DSP %d ... ",0
SL17:	.byte	"Failed",10,"     Failed at address %x.",10,0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,30
	.sect	".cinit"
	.word	30,CONST
	.word 	SL0              ;0
	.word 	SL1              ;1
	.word 	-1929379840      ;2
	.word 	SL2              ;3
	.word 	SL3              ;4
	.word 	SL4              ;5
	.word 	SL5              ;6
	.word 	SL6              ;7
	.word 	SL7              ;8
	.word 	SL8              ;9
	.word 	-1073741824      ;10
	.word 	SL9              ;11
	.word 	SL10             ;12
	.word 	SL11             ;13
	.word 	_booted          ;14
	.word 	SL12             ;15
	.word 	SL13             ;16
	.word 	SL14             ;17
	.word 	SL15             ;18
	.word 	SL16             ;19
	.word 	1000000          ;20
	.word 	SL17             ;21
	.word 	-1082146808      ;22
	.word 	-2145386496      ;23
	.word 	05ffe49h         ;24
	.word 	1885339209       ;25
	.word 	262144           ;26
	.word 	1048640          ;27
	.word 	1048641          ;28
	.word 	1048642          ;29
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_LED
	.globl	_MemTest
	.end
