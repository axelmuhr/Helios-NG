******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 services.c services.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o services.if services.asm services.tmp 
	.version	40
FP	.set		AR3
	.file	"services.c"
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
	.file	"services.c"

	.sym	_compare,_compare,32,2,0
	.globl	_compare

	.func	5
;>>>> 	void compare( unsigned long parms[], char out_where )
******************************************************
* FUNCTION DEF : _compare
******************************************************
_compare:
	PUSH	FP
	LDI	SP,FP
	.sym	_parms,-2,31,9,32
	.sym	_out_where,-3,2,9,32
	.line	2
	.line	3
;>>>> 		if( parms[2] == 0xFFFFFFFF )
	LDA	*-FP(2),AR0
	CMPI	-1,*+AR0(2)
	BZ	EPI0_1
	.line	4
;>>>> 			return;
	.line	6
;>>>> 		while( parms[2]-- )
	SUBI	1,*+AR0(2),R0
	STI	R0,*+AR0(2)
	ADDI	1,R0
	BZ	L3
L2:
	.line	7
;>>>> 			if( *((unsigned long *)parms[0]++) != *((unsigned long *)parms[1]++) )
;>>>> 				switch( out_where )
;>>>> 					case 't' :
	LDA	*-FP(2),AR0
	LDA	*AR0,AR1
	LDI	*AR1++,R0
	STI	AR1,*AR0
	LDA	*+AR0(1),AR1
	CMPI	*AR1++,R0
	STI	AR1,*+AR0(1)
	BZ	L4
	B	L5
L6:
	.line	12
;>>>> 						c40_printf( "Compare fault at address [%x].\n", --parms[0] );
	LDA	*-FP(2),AR0
	SUBI	1,*AR0,R0
	STI	R0,*AR0
	PUSH	R0
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
	.line	13
;>>>> 						return;
;>>>> 					default :
	B	EPI0_1
L7:
	.line	15
;>>>> 						send_host( FAILURE );
	LDI	0,R0
	PUSH	R0
	CALL	_send_host
	SUBI	1,SP
	.line	16
;>>>> 						return;
;>>>> 	   switch( out_where )
;>>>> 			case 't' :
	B	EPI0_1
L5:
	.line	9
	LDI	*-FP(3),R0
	CMPI	116,R0
	BZ	L6
	B	L7
L4:
	.line	6
	LDA	*-FP(2),AR0
	SUBI	1,*+AR0(2),R0
	STI	R0,*+AR0(2)
	ADDI	1,R0
	BNZ	L2
L3:
	B	L9
L10:
	.line	23
;>>>> 				c40_printf( "Compare successfull.\n" );
	LDI	@CONST+1,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	24
;>>>> 				return;
;>>>> 			default :
	B	EPI0_1
L11:
	.line	26
;>>>> 				send_host( SUCCESS );
	LDI	1,R0
	PUSH	R0
	CALL	_send_host
	SUBI	1,SP
	.line	27
;>>>> 				return;
	B	EPI0_1
L9:
	.line	20
	LDI	*-FP(3),R0
	CMPI	116,R0
	BZ	L10
	B	L11
EPI0_1:
	.line	29
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	33,000000000H,0

	.sym	_dump,_dump,32,2,0
	.globl	_dump

	.func	38
;>>>> 	void dump( unsigned long parms[], char out_where )
******************************************************
* FUNCTION DEF : _dump
******************************************************
_dump:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_parms,-2,31,9,32
	.sym	_out_where,-3,2,9,32
	.sym	_i,1,4,1,32
	.line	2
;>>>> 		int i;
	.line	5
;>>>> 		if( parms[1] == 0xFFFFFFFF )
	LDA	*-FP(2),AR0
	CMPI	-1,*+AR0(1)
	BZ	EPI0_2
	.line	6
;>>>> 			return;
	.line	8
;>>>> 		while( parms[1] )
	LDI	*+AR0(1),R0
	BZ	L15
L14:
	.line	10
;>>>> 			c40_printf( "\n[%x]   ", parms[0] );
	LDA	*-FP(2),AR0
	LDI	*AR0,R0
	PUSH	R0
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
	.line	11
;>>>> 			for( i=0 ; i < 4 ; i++ )
;>>>> 	         switch( out_where )
;>>>> 					case 't' :
	STIK	0,*+FP(1)
	CMPI	4,*+FP(1)
	BGE	L17
L16:
	B	L18
L19:
	.line	16
;>>>> 						c40_printf( "%x  ", *((unsigned long *) parms[0]++) );
	LDA	*-FP(2),AR0
	LDA	*AR0,AR1
	LDI	*AR1++,R0
	STI	AR1,*AR0
	PUSH	R0
	LDI	@CONST+3,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
	.line	17
;>>>> 						break;
;>>>> 					default :
	B	L20
L21:
	.line	19
;>>>> 						send_host( *((unsigned long *) parms[0]++) );
	LDA	*-FP(2),AR0
	LDA	*AR0,AR1
	LDI	*AR1++,R0
	STI	AR1,*AR0
	PUSH	R0
	CALL	_send_host
	SUBI	1,SP
	B	L20
L18:
	.line	13
	LDI	*-FP(3),R0
	CMPI	116,R0
	BZ	L19
	B	L21
L20:
	.line	21
;>>>> 				if( !(--parms[1]) )
	LDA	*-FP(2),AR0
	SUBI	1,*+AR0(1),R0
	STI	R0,*+AR0(1)
	BZ	L17
	.line	22
;>>>> 					break;
	.line	11
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	4,R0
	BLT	L16
L17:
	.line	8
	LDA	*-FP(2),AR0
	LDI	*+AR0(1),R0
	BNZ	L14
L15:
	.line	25
;>>>> 		c40_printf( "\n\n" );
	LDI	@CONST+4,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
EPI0_2:
	.line	26
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	63,000000000H,1

	.sym	_fill,_fill,32,2,0
	.globl	_fill

	.func	68
;>>>> 	void fill( unsigned long parms[] )
******************************************************
* FUNCTION DEF : _fill
******************************************************
_fill:
	PUSH	FP
	LDI	SP,FP
	.sym	_parms,-2,31,9,32
	.line	2
	.line	3
;>>>> 		if( parms[1] == 0xFFFFFFFF )
	LDA	*-FP(2),AR0
	CMPI	-1,*+AR0(1)
	BZ	EPI0_3
	.line	4
;>>>> 			return;
	.line	6
;>>>> 		while( parms[1]-- )
	SUBI	1,*+AR0(1),R0
	STI	R0,*+AR0(1)
	ADDI	1,R0
	BZ	EPI0_3
L24:
	.line	7
;>>>> 			*((unsigned long *) parms[0]++) = parms[2];
	LDA	*-FP(2),AR0
	LDA	*AR0,AR1
	LDI	*+AR0(2),R0
	STI	R0,*AR1++
	STI	AR1,*AR0
	.line	6
	SUBI	1,*+AR0(1),R0
	STI	R0,*+AR0(1)
	ADDI	1,R0
	BNZ	L24
EPI0_3:
	.line	8
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	75,000000000H,0

	.sym	_enter,_enter,32,2,0
	.globl	_enter

	.func	81
;>>>> 	void enter( unsigned long parms[], char out_where )
******************************************************
* FUNCTION DEF : _enter
******************************************************
_enter:
	PUSH	FP
	LDI	SP,FP
	ADDI	13,SP
	.sym	_parms,-2,31,9,32
	.sym	_out_where,-3,2,9,32
	.sym	_inval,1,50,1,288,,9
	.sym	_i,10,4,1,32
	.sym	_j,11,4,1,32
	.sym	_ok,12,4,1,32
	.sym	_xval,13,15,1,32
	.line	2
;>>>> 		char inval[9];
;>>>> 		int i, j, ok;
;>>>> 		unsigned long xval;
	.line	8
;>>>> 		if( parms[0] == 0xFFFFFFFF )
	LDA	*-FP(2),AR0
	CMPI	-1,*AR0
	BZ	EPI0_4
	.line	9
;>>>> 			return;
;>>>> 		while( 1 )
L27:
	.line	13
;>>>> 			c40_printf( "[%x]  %x : ", parms[0], *((unsigned long *)parms[0]) );
	LDA	*-FP(2),AR0
	LDA	*AR0,AR1
	LDI	*AR1,R0
	PUSH	R0
	LDI	*AR0,R0
	PUSH	R0
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	3,SP
	.line	16
;>>>> 			j=0;
;>>>> 			while( (j < 8) && ((c40_putchar(inval[j]=c40_getchar()))!='\n') )
	STIK	0,*+FP(11)
	B	L30
L29:
	.line	19
;>>>> 				if( inval[j++] == '\b' )
	ADDI	1,*+FP(11),IR0
	STI	IR0,*+FP(11)
	LDI	8,R0
	CMPI	R0,*+FP(IR0)
	BNZ	L30
	.line	20
;>>>> 					if( j == 1 )
	CMPI	1,*+FP(11)
	BNZ	L32
	.line	21
;>>>> 						j -= 1;
;>>>> 					else 
	SUBI	1,*+FP(11),R1
	STI	R1,*+FP(11)
	B	L30
L32:
	.line	23
;>>>> 						j -= 2;
	SUBI	2,*+FP(11),R1
	STI	R1,*+FP(11)
L30:
	.line	17
	CMPI	8,*+FP(11)
	BGE	LL6
	CALL	_c40_getchar
	LDA	*+FP(11),IR1
	ADDI	1,IR1
	STI	R0,*+FP(IR1)
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	CMPI	10,R0
	BNZ	L29
LL6:
	.line	26
;>>>> 			if( inval[1] == '\n' )
	CMPI	10,*+FP(2)
	BZ	L28
	.line	28
;>>>> 				break;
	.line	31
;>>>> 			if( j == 8 )
	CMPI	8,*+FP(11)
	BNZ	L35
	.line	32
;>>>> 				c40_printf( "\n" );
	LDI	@CONST+6,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L35:
	.line	34
;>>>> 			inval[j] = '\0';
	LDA	*+FP(11),IR0
	ADDI	1,IR0
	STIK	0,*+FP(IR0)
	.line	36
;>>>> 			*((unsigned int *)parms[0]++) = atox( inval, &ok );
	LDI	FP,R0
	ADDI	12,R0
	PUSH	R0
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	CALL	_atox
	SUBI	2,SP
	LDA	*-FP(2),AR0
	LDA	*AR0,AR1
	STI	R0,*AR1++
	STI	AR1,*AR0
	.line	38
;>>>> 			if( ok == FAILURE )
	LDI	*+FP(12),R0
	BNZ	L36
	.line	40
;>>>> 				c40_printf( "Not a valid number %s.\n", inval );
	LDI	FP,R1
	ADDI	1,R1
	PUSH	R1
	LDI	@CONST+7,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	.line	41
;>>>> 				break;
;>>>> 			switch( out_where )
;>>>> 				case 't' :
	B	L28
L36:
	B	L37
L38:
	.line	48
;>>>> 					break;
;>>>> 				default :
	B	L39
L40:
	.line	50
;>>>> 					send_host( atox( inval, &ok ) );
	LDI	FP,R0
	ADDI	12,R0
	PUSH	R0
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	CALL	_atox
	SUBI	2,SP
	PUSH	R0
	CALL	_send_host
	SUBI	1,SP
	B	L39
L37:
	.line	45
	LDI	*-FP(3),R1
	CMPI	116,R1
	BZ	L38
	B	L40
L39:
	.line	52
;>>>> 			if( !(--parms[1]) )
	LDA	*-FP(2),AR0
	SUBI	1,*+AR0(1),R0
	STI	R0,*+AR0(1)
	BZ	L28
	.line	53
;>>>> 				break;
	.line	54
	B	L27
L28:
	.line	56
;>>>> 		c40_printf( "\n\n" );
	LDI	@CONST+4,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
EPI0_4:
	.line	57
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	15,SP
	B	R1
	.endfunc	137,000000000H,13

	.sym	_copy,_copy,32,2,0
	.globl	_copy

	.func	143
;>>>> 	void copy( unsigned long parms[] )
******************************************************
* FUNCTION DEF : _copy
******************************************************
_copy:
	PUSH	FP
	LDI	SP,FP
	.sym	_parms,-2,31,9,32
	.line	2
	.line	3
;>>>> 		if( parms[2] == 0xFFFFFFFF )
	LDA	*-FP(2),AR0
	CMPI	-1,*+AR0(2)
	BZ	EPI0_5
	.line	4
;>>>> 			return;
	.line	6
;>>>> 		while( parms[2]-- )
	SUBI	1,*+AR0(2),R0
	STI	R0,*+AR0(2)
	ADDI	1,R0
	BZ	EPI0_5
L43:
	.line	7
;>>>> 			*((unsigned long *) parms[1]++) = *((unsigned long *) parms[0]++);
	LDA	*-FP(2),AR0
	LDA	*AR0,AR1
	LDI	*AR1++,R0
	STI	AR1,*AR0
	LDA	*+AR0(1),AR1
	STI	R0,*AR1++
	STI	AR1,*+AR0(1)
	.line	6
	SUBI	1,*+AR0(2),R0
	STI	R0,*+AR0(2)
	ADDI	1,R0
	BNZ	L43
EPI0_5:
	.line	8
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	150,000000000H,0

	.sym	_search,_search,32,2,0
	.globl	_search

	.func	156
;>>>> 	void search( unsigned long parms[], char out_where )
******************************************************
* FUNCTION DEF : _search
******************************************************
_search:
	PUSH	FP
	LDI	SP,FP
	.sym	_parms,-2,31,9,32
	.sym	_out_where,-3,2,9,32
	.line	2
	.line	3
;>>>> 		if( parms[1] == 0xFFFFFFFF )
	LDA	*-FP(2),AR0
	CMPI	-1,*+AR0(1)
	BZ	EPI0_6
	.line	4
;>>>> 			return;
;>>>> 		while( parms[0] != parms[1] )
	B	L47
L46:
	.line	7
;>>>> 			if( *((unsigned long *)parms[0]++) == parms[2] )
;>>>> 				switch( out_where )
;>>>> 					case 't' :
	LDA	*-FP(2),AR0
	LDA	*AR0,AR1
	LDI	*+AR0(2),R0
	CMPI	R0,*AR1++
	STI	AR1,*AR0
	BNZ	L47
	B	L49
L50:
	.line	12
;>>>> 						c40_printf( "%x found at address %x.\n", parms[2], --parms[0] );
	LDA	*-FP(2),AR0
	SUBI	1,*AR0,R0
	STI	R0,*AR0
	PUSH	R0
	LDI	*+AR0(2),R0
	PUSH	R0
	LDI	@CONST+8,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	3,SP
	.line	13
;>>>> 						return;
;>>>> 					default :
	B	EPI0_6
L51:
	.line	15
;>>>> 						send_host( --parms[0] );
;>>>> 	   switch( out_where )
;>>>> 			case 't' :
	LDA	*-FP(2),AR0
	SUBI	1,*AR0,R0
	STI	R0,*AR0
	PUSH	R0
	CALL	_send_host
	SUBI	1,SP
	B	L47
L49:
	.line	9
	LDI	*-FP(3),R0
	CMPI	116,R0
	BZ	L50
	B	L51
L47:
	.line	6
	LDA	*-FP(2),AR0
	CMPI	*+AR0(1),*AR0
	BNZ	L46
	B	L53
L54:
	.line	22
;>>>> 				c40_printf( "%x not found.\n", parms[2] );
	LDA	*-FP(2),AR0
	LDI	*+AR0(2),R0
	PUSH	R0
	LDI	@CONST+9,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
	.line	23
;>>>> 				return;
;>>>> 			default :
	B	EPI0_6
L55:
	.line	25
;>>>> 				send_host( FAILURE );
	LDI	0,R0
	PUSH	R0
	CALL	_send_host
	SUBI	1,SP
	B	EPI0_6
L53:
	.line	19
	LDI	*-FP(3),R0
	CMPI	116,R0
	BZ	L54
	B	L55
EPI0_6:
	.line	28
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	183,000000000H,0

	.sym	_help,_help,32,2,0
	.globl	_help

	.func	190
;>>>> 	void help()
******************************************************
* FUNCTION DEF : _help
******************************************************
_help:
	PUSH	FP
	LDI	SP,FP
	.line	3
;>>>> 		c40_printf( "\n\nHydra_Mon Command summary :\n"
;>>>> 						"  Test =>              t\n"
;>>>> 						"  Configure =>         cf\n"
;>>>> 						"  Dump =>              d  start_addr #(of addresses to dump)\n"
;>>>> 						"  Enter =>             e  address\n"
;>>>> 						"  Fill =>              f  start_addr #(of addresses to fill) value\n"
;>>>> 						"  Copy =>              cp start1 start2 #(of addresses to copy)\n"
;>>>> 						"  Compare =>           c  start1 start2 #(of addresses to compare)\n"
;>>>> 						"  Search =>            s  from_address to_address for_data\n" );
	LDI	@CONST+10,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
EPI0_7:
	.line	19
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	208,000000000H,0
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.sect	".const"
SL0:	.byte	"Compare fault at address [%x].",10,0
SL1:	.byte	"Compare successfull.",10,0
SL2:	.byte	10,"[%x]   ",0
SL3:	.byte	"%x  ",0
SL4:	.byte	10,10,0
SL5:	.byte	"[%x]  %x : ",0
SL6:	.byte	10,0
SL7:	.byte	"Not a valid number %s.",10,0
SL8:	.byte	"%x found at address %x.",10,0
SL9:	.byte	"%x not found.",10,0
SL10:	.byte	10,10,"Hydra_Mon Command summary :",10,"  Test =>           "
	.byte	"   t",10,"  Configure =>         cf",10,"  Dump =>         "
	.byte	"     d  start_addr #(of addresses to dump)",10,"  Enter => "
	.byte	"            e  address",10,"  Fill =>              f  start"
	.byte	"_addr #(of addresses to fill) value",10,"  Copy =>         "
	.byte	"     cp start1 start2 #(of addresses to copy)",10,"  Compar"
	.byte	"e =>           c  start1 start2 #(of addresses to compare)"
	.byte	10,"  Search =>            s  from_address to_address for_da"
	.byte	"ta",10,0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,11
	.sect	".cinit"
	.word	11,CONST
	.word 	SL0              ;0
	.word 	SL1              ;1
	.word 	SL2              ;2
	.word 	SL3              ;3
	.word 	SL4              ;4
	.word 	SL5              ;5
	.word 	SL6              ;6
	.word 	SL7              ;7
	.word 	SL8              ;8
	.word 	SL9              ;9
	.word 	SL10             ;10
	.end
