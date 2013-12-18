******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 config.c config.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o config.if config.asm config.tmp 
	.version	40
FP	.set		AR3
	.file	"config.c"
	.file	"c:\c40\ctype.h"
	.globl	__ctypes_
	.globl	_isalnum
	.globl	_isalpha
	.globl	_iscntrl
	.globl	_isdigit
	.globl	_isgraph
	.globl	_islower
	.globl	_isprint
	.globl	_ispunct
	.globl	_isspace
	.globl	_isupper
	.globl	_isxdigit
	.globl	_isascii
	.globl	_toupper
	.globl	_tolower
	.globl	_toascii
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
	.file	"config.c"

	.sym	_configure,_configure,32,2,0
	.globl	_configure

	.func	4
;>>>> 	void configure( hydra_conf *config )
******************************************************
* FUNCTION DEF : _configure
******************************************************
_configure:
	PUSH	FP
	LDI	SP,FP
	ADDI	3,SP
	.sym	_config,-2,24,9,32,.fake1
	.sym	_i,1,15,1,32
	.sym	_temp_addr,2,15,1,32
	.sym	_flag,3,15,1,32
	.line	2
;>>>> 		unsigned long i, temp_addr, flag;
;>>>> 		while( 1 )
L1:
	.line	8
;>>>> 			display_conf( *config );
;>>>> 			switch( menu() )
;>>>> 				case '1':
	LDA	*-FP(2),AR0
	LDA	SP,AR1
	ADDI	16,SP
	LDI	*AR0++,R0
	RPTS	15
	STI	R0,*++AR1
    ||	LDI	*AR0++,R0
	CALL	_display_conf
	SUBI	16,SP
	B	L3
L4:
	.line	13
;>>>> 					configure_uart( config, 'a' );
	LDI	97,R0
	PUSH	R0
	LDI	*-FP(2),R1
	PUSH	R1
	CALL	_configure_uart
	SUBI	2,SP
	.line	14
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
	.line	15
;>>>> 					break;
;>>>> 				case '2':
	B	L5
L6:
	.line	17
;>>>> 					configure_uart( config, 'b' );
	LDI	98,R0
	PUSH	R0
	LDI	*-FP(2),R1
	PUSH	R1
	CALL	_configure_uart
	SUBI	2,SP
	.line	18
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
	.line	19
;>>>> 					break;
;>>>> 				case '3':
	B	L5
L7:
	.line	21
;>>>> 					config->cpu_clock = get_clock();
	CALL	_get_clock
	LDA	*-FP(2),AR0
	STI	R0,*+AR0(7)
	.line	22
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
	.line	23
;>>>> 					break;
;>>>> 				case '4':
	B	L5
L8:
	.line	25
;>>>> 					get_sram( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_get_sram
	SUBI	1,SP
	.line	26
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
	.line	27
;>>>> 					break;
;>>>> 				case '5':
	B	L5
L9:
	.line	29
;>>>> 					config->dram_size = get_dram();
	CALL	_get_dram
	LDA	*-FP(2),AR0
	STI	R0,*+AR0(6)
	.line	30
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
	.line	31
;>>>> 					break;
;>>>> 				case '6':
	B	L5
L10:
	.line	33
;>>>> 					for( flag=FALSE ; !flag ; )
	STIK	0,*+FP(3)
	LDI	*+FP(3),R0
	BNZ	L12
L11:
	.line	35
;>>>> 						temp_addr = get_addr( "DRAM base" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_get_addr
	SUBI	1,SP
	STI	R0,*+FP(2)
	.line	36
;>>>> 						if( !temp_addr )
	CMPI	0,R0
	BNZ	L13
	.line	37
;>>>> 							flag = TRUE;
	STIK	1,*+FP(3)
	B	L14
L13:
	.line	38
;>>>> 						else if( (temp_addr>>16) <= 0x8000 )
	LSH	-16,R0,R1
	CMPI	@CONST+1,R1
	BHI	L15
	.line	39
;>>>> 							c40_printf( "Not a valid base address for DRAM\n" );
;>>>> 						else
	LDI	@CONST+2,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	1,SP
	B	L14
L15:
	.line	42
;>>>> 							config->l_dram_base = temp_addr;
	LDA	*-FP(2),AR0
	STI	R0,*+AR0(13)
	.line	43
;>>>> 							flag = TRUE;
	STIK	1,*+FP(3)
L14:
	.line	33
	LDI	*+FP(3),R0
	BZ	L11
L12:
	.line	46
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R1
	PUSH	R1
	CALL	_update_chksum
	SUBI	1,SP
	.line	47
;>>>> 					break;
;>>>> 				case '7':
	B	L5
L16:
	.line	49
;>>>> 					temp_addr = get_addr( "JTAG / Hydra control register base" );
	LDI	@CONST+3,R0
	PUSH	R0
	CALL	_get_addr
	SUBI	1,SP
	STI	R0,*+FP(2)
	.line	50
;>>>> 					config->l_jtag_base = temp_addr ? temp_addr : config->l_jtag_base;
	CMPI	0,R0
	LDINZ	R0,R1
	LDA	*-FP(2),AR0
	LDIZ	*+AR0(14),R1
	STI	R1,*+AR0(14)
	.line	51
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R1
	PUSH	R1
	CALL	_update_chksum
	SUBI	1,SP
	.line	52
;>>>> 					break;
;>>>> 				case '8' :
	B	L5
L17:
	.line	54
;>>>> 					config->daughter = get_daughter();
	CALL	_get_daughter
	LDA	*-FP(2),AR0
	STI	R0,*+AR0(15)
	.line	55
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
	.line	56
;>>>> 					break;
;>>>> 				case 'R' :
;>>>> 				case 'r' :
	B	L5
L19:
	.line	59
;>>>> 					write_config( *config ); 
	LDA	*-FP(2),AR0
	LDA	SP,AR1
	ADDI	16,SP
	LDI	*AR0++,R0
	RPTS	15
	STI	R0,*++AR1
    ||	LDI	*AR0++,R0
	CALL	_write_config
	SUBI	16,SP
	.line	60
;>>>> 					c40_printf( "\n\n" );
	LDI	@CONST+4,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	61
;>>>> 					return;
;>>>> 					break;
	B	EPI0_1
L3:
	.line	10
	CALL	_menu
	CMPI	49,R0
	BZ	L4
	CMPI	50,R0
	BZ	L6
	CMPI	51,R0
	BZ	L7
	CMPI	52,R0
	BZ	L8
	CMPI	53,R0
	BZ	L9
	CMPI	54,R0
	BZ	L10
	CMPI	55,R0
	BZ	L16
	CMPI	56,R0
	BZ	L17
	CMPI	82,R0
	BZ	L19
	CMPI	114,R0
	BZ	L19
L5:
	.line	64
	B	L1
EPI0_1:
	.line	65
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	5,SP
	B	R1
	.endfunc	68,000000000H,3

	.sym	_menu,_menu,47,2,0
	.globl	_menu

	.func	72
;>>>> 	unsigned long menu( void )
;>>>> 		int i;
******************************************************
* FUNCTION DEF : _menu
******************************************************
_menu:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_i,1,4,1,32
	.line	5
;>>>> 		c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	6
;>>>> 		c40_printf(  "1) RS-232 channel A configuration\n"
;>>>> 		             "2) RS-232 channel B configuration\n"
;>>>> 		             "3) CPU clock rate\n"
;>>>> 		             "4) SRAM size(s)\n"
;>>>> 		             "5) DRAM size\n"
;>>>> 		             "6) DRAM base address\n"
;>>>> 		             "7) JTAG / Hydra control register address\n"
;>>>> 		             "8) Daughter card attached\n"
;>>>> 		             "R) Return\n" );
	LDI	@CONST+6,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	16
;>>>> 		c40_printf( "Selection => " );
;>>>> 		while( 1 )
	LDI	@CONST+7,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L20:
	.line	19
;>>>> 			i = c40_getchar();
	CALL	_c40_getchar
	STI	R0,*+FP(1)
	.line	20
;>>>> 			if( (i=='R') || (i=='r') )
	CMPI	82,R0
	BZ	LL4
	CMPI	114,R0
	BNZ	L22
LL4:
	.line	21
;>>>> 				return( i );
	B	EPI0_2
L22:
	.line	22
;>>>> 			else if( isdigit(i) )
	PUSH	R0
	CALL	_isdigit
	SUBI	1,SP
	CMPI	0,R0
	BZ	L23
	.line	24
;>>>> 				if( (i >= '1') && (i <= '8') )
	LDI	49,R0
	CMPI	R0,*+FP(1)
	BLT	L24
	LDI	56,R1
	CMPI	R1,*+FP(1)
	BGT	L24
	.line	25
;>>>> 					return( i );
;>>>> 				else
	LDI	*+FP(1),R0
	B	EPI0_2
L24:
	.line	27
;>>>> 					c40_printf( "%c", 7 );
;>>>> 			else
	LDI	7,R1
	PUSH	R1
	LDI	@CONST+8,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	2,SP
	B	L25
L23:
	.line	31
;>>>> 				c40_printf( "%c", 7 );
	LDI	7,R0
	PUSH	R0
	LDI	@CONST+8,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
L25:
	.line	32
	B	L20
EPI0_2:
	.line	33
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	104,000000000H,1

	.sym	_configure_uart,_configure_uart,32,2,0
	.globl	_configure_uart

	.func	108
;>>>> 	void configure_uart( hydra_conf *config, char which )
******************************************************
* FUNCTION DEF : _configure_uart
******************************************************
_configure_uart:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
	.sym	_config,-2,24,9,32,.fake1
	.sym	_which,-3,2,9,32
	.sym	_val,1,15,1,32
	.sym	_inchar,2,2,1,32
	.line	2
;>>>> 		unsigned long val;
;>>>> 		char inchar;
	.line	7
;>>>> 		c40_printf( "\n\n1) Baud rate\n2) Parity\n3) Bits/character\nR) Return\n\nSelection => " );
;>>>> 		while( 1 )
	LDI	@CONST+9,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L26:
	.line	11
;>>>> 			inchar = c40_getchar();
	CALL	_c40_getchar
	STI	R0,*+FP(2)
	.line	12
;>>>> 			if( inchar == '1' ) 
;>>>> 				switch( which )
;>>>> 					case 'a':
	CMPI	49,R0
	BNZ	L28
	B	L29
L30:
	.line	17
;>>>> 						config->uartA.baud = (val=get_baud())?val:config->uartA.baud;
	CALL	_get_baud
	STI	R0,*+FP(1)
	CMPI	0,R0
	LDINZ	R0,R1
	LDA	*-FP(2),AR0
	LDIZ	*AR0,R1
	STI	R1,*AR0
	.line	18
;>>>> 						break;
;>>>> 					case 'b':
	B	L31
L32:
	.line	20
;>>>> 						config->uartB.baud = (val=get_baud())?val:config->uartB.baud;
	CALL	_get_baud
	STI	R0,*+FP(1)
	CMPI	0,R0
	LDINZ	R0,R1
	LDA	*-FP(2),AR0
	LDIZ	*+AR0(3),R1
	STI	R1,*+AR0(3)
	.line	21
;>>>> 						break;
	B	L31
L29:
	.line	14
	LDI	*-FP(3),R1
	CMPI	97,R1
	BZ	L30
	CMPI	98,R1
	BZ	L32
L31:
	.line	23
;>>>> 				c40_printf( "\n\n1) Baud rate\n2) Parity\n3) Bits/character\nR) Return\n\nSelection => " );
	LDI	@CONST+9,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	B	L33
L28:
	.line	25
;>>>> 			else if( inchar == '2' )
	CMPI	50,R0
	BNZ	L34
	.line	27
;>>>> 				get_parity( config, which );
	LDI	*-FP(3),R1
	PUSH	R1
	LDI	*-FP(2),R2
	PUSH	R2
	CALL	_get_parity
	SUBI	2,SP
	.line	28
;>>>> 				c40_printf( "\n\n1) Baud rate\n2) Parity\n3) Bits/character\nR) Return\n\nSelection => " );
	LDI	@CONST+9,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	B	L33
L34:
	.line	30
;>>>> 			else if( inchar == '3' )
	CMPI	51,R0
	BNZ	L35
	.line	32
;>>>> 				get_bits( config, which );
	LDI	*-FP(3),R1
	PUSH	R1
	LDI	*-FP(2),R2
	PUSH	R2
	CALL	_get_bits
	SUBI	2,SP
	.line	33
;>>>> 				c40_printf( "\n\n1) Baud rate\n2) Parity\n3) Bits/character\nR) Return\n\nSelection => " );
	LDI	@CONST+9,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	B	L33
L35:
	.line	35
;>>>> 			else if( (inchar == 'r') || (inchar == 'R') )
	CMPI	114,R0
	BZ	LL8
	CMPI	82,R0
	BNZ	L36
LL8:
	.line	36
;>>>> 				return;
;>>>> 			else
	B	EPI0_3
L36:
	.line	38
;>>>> 				c40_printf( "%c", 7 );
	LDI	7,R1
	PUSH	R1
	LDI	@CONST+8,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	2,SP
L33:
	.line	39
	B	L26
EPI0_3:
	.line	40
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	4,SP
	B	R1
	.endfunc	147,000000000H,2

	.sym	_get_baud,_get_baud,47,2,0
	.globl	_get_baud

	.func	151
;>>>> 	unsigned long get_baud( void )
;>>>> 		char inchar;
******************************************************
* FUNCTION DEF : _get_baud
******************************************************
_get_baud:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_inchar,1,2,1,32
	.line	5
;>>>> 		c40_printf( "\n\nBaud Rates :\n" );
	LDI	@CONST+10,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	6
;>>>> 		c40_printf( "  1) 150\n  2) 300\n  3) 600\n  4) 1200\n  5) 2400\n"
;>>>> 		            "  6) 4800\n  7) 9600\n  8) 19200\n  R) Return" );
	LDI	@CONST+11,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	8
;>>>> 		c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	9
;>>>> 		c40_printf( "Selection => " );
;>>>> 		while( 1 )
	LDI	@CONST+7,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L37:
	.line	13
;>>>> 			inchar = c40_getchar();
	CALL	_c40_getchar
	STI	R0,*+FP(1)
	.line	14
;>>>> 			if( (inchar >= '1') && ( inchar <= '7') )
;>>>> 				switch( inchar )
;>>>> 					case '1':
	CMPI	49,R0
	BLT	L39
	CMPI	55,R0
	BGT	L39
	B	L40
L41:
	.line	18
;>>>> 						return( 150 );
;>>>> 					case '2':
	LDI	150,R0
	B	EPI0_4
L42:
	.line	20
;>>>> 						return( 300 );
;>>>> 					case '3':
	LDI	300,R0
	B	EPI0_4
L43:
	.line	22
;>>>> 						return( 600 );
;>>>> 					case '4':
	LDI	600,R0
	B	EPI0_4
L44:
	.line	24
;>>>> 						return( 1200 );
;>>>> 					case '5':
	LDI	1200,R0
	B	EPI0_4
L45:
	.line	26
;>>>> 						return( 2400 );
;>>>> 					case '6':
	LDI	2400,R0
	B	EPI0_4
L46:
	.line	28
;>>>> 						return( 4800 );
;>>>> 					case '7':
	LDI	4800,R0
	B	EPI0_4
L47:
	.line	30
;>>>> 						return( 9600 );
;>>>> 					case '8':
	LDI	9600,R0
	B	EPI0_4
L48:
	.line	32
;>>>> 						return( 19200 );
	LDI	19200,R0
	B	EPI0_4
L40:
	.line	15
	LDA	*+FP(1),IR0
	LDA	@CONST+12,AR0
	SUBI	49,IR0
	CMPI	7,IR0
	LDIHI	8,IR0
	LDA	*+AR0(IR0),AR0
	B	AR0
	.sect	".const"
LL12:
	.word	L41
	.word	L42
	.word	L43
	.word	L44
	.word	L45
	.word	L46
	.word	L47
	.word	L48
	.word	L49
	.text
L49:
	B	L50
L39:
	.line	34
;>>>> 			else if( (inchar == 'r') || (inchar == 'R') )
	CMPI	114,R0
	BZ	LL13
	CMPI	82,R0
	BNZ	L51
LL13:
	.line	35
;>>>> 				return( 0 );
;>>>> 			else
	LDI	0,R0
	B	EPI0_4
L51:
	.line	37
;>>>> 				c40_printf( "%c", 7 );
	LDI	7,R1
	PUSH	R1
	LDI	@CONST+8,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	2,SP
L50:
	.line	38
	B	L37
EPI0_4:
	.line	39
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	189,000000000H,1

	.sym	_get_parity,_get_parity,32,2,0
	.globl	_get_parity

	.func	195
;>>>> 	void get_parity( hydra_conf *config, char which )
******************************************************
* FUNCTION DEF : _get_parity
******************************************************
_get_parity:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_config,-2,24,9,32,.fake1
	.sym	_which,-3,2,9,32
	.sym	_inchar,1,2,1,32
	.line	2
;>>>> 		char inchar;
	.line	5
;>>>> 		c40_printf( "\n\n1) Parity enabled/disabled\n2) Even/odd parity\nR) Return\n\nSelection => " );
;>>>> 		while( 1 )
	LDI	@CONST+13,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L52:
	.line	9
;>>>> 			inchar = c40_getchar();
	CALL	_c40_getchar
	STI	R0,*+FP(1)
	.line	10
;>>>> 			if( inchar == '1' ) 
;>>>> 				switch( which )
;>>>> 					case 'a':
	CMPI	49,R0
	BNZ	L54
	B	L55
L56:
	.line	15
;>>>> 						config->uartA.parity = parity_enable()?abs(config->uartA.parity):-abs(config->uartA.parity);
	CALL	_parity_enable
	CMPI	0,R0
	BZ	LL16
	LDA	*-FP(2),AR0
	ABSI	*+AR0(1),R0
	B	LL17
LL16:
	LDA	*-FP(2),AR0
	ABSI	*+AR0(1),R0
	NEGI	R0
LL17:
	STI	R0,*+AR0(1)
	.line	16
;>>>> 						break;
;>>>> 					case 'b':
	B	L57
L58:
	.line	18
;>>>> 						config->uartB.parity = parity_enable()?abs(config->uartB.parity):-abs(config->uartA.parity);
	CALL	_parity_enable
	CMPI	0,R0
	BZ	LL18
	LDA	*-FP(2),AR0
	ABSI	*+AR0(4),R0
	B	LL19
LL18:
	LDA	*-FP(2),AR0
	ABSI	*+AR0(1),R0
	NEGI	R0
LL19:
	STI	R0,*+AR0(4)
	.line	19
;>>>> 						break;
	B	L57
L55:
	.line	12
	LDI	*-FP(3),R1
	CMPI	97,R1
	BZ	L56
	CMPI	98,R1
	BZ	L58
L57:
	.line	21
;>>>> 				c40_printf( "\n\n1) Parity enabled/disabled\n2) Even/odd parity\nR) Return\n\nSelection => " );
	LDI	@CONST+13,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	B	L59
L54:
	.line	23
;>>>> 			else if( inchar == '2' )
;>>>> 				switch( which )
;>>>> 					case 'a':
	CMPI	50,R0
	BNZ	L60
	B	L61
L62:
	.line	28
;>>>> 						if( config->uartA.parity > 0 )
	LDA	*-FP(2),AR0
	LDI	*+AR0(1),R0
	BLE	L63
	.line	29
;>>>> 							config->uartA.parity = parity_type();
;>>>> 						else
	CALL	_parity_type
	LDA	*-FP(2),AR0
	STI	R0,*+AR0(1)
	B	L64
L63:
	.line	31
;>>>> 							config->uartA.parity = -parity_type();
	CALL	_parity_type
	NEGI	R0
	LDA	*-FP(2),AR0
	STI	R0,*+AR0(1)
L64:
	.line	32
;>>>> 						break;
;>>>> 					case 'b':
	B	L65
L66:
	.line	34
;>>>> 						if( config->uartB.parity > 0 )
	LDA	*-FP(2),AR0
	LDI	*+AR0(4),R0
	BLE	L67
	.line	35
;>>>> 							config->uartB.parity = parity_type();
;>>>> 						else
	CALL	_parity_type
	LDA	*-FP(2),AR0
	STI	R0,*+AR0(4)
	B	L68
L67:
	.line	37
;>>>> 							config->uartB.parity = -parity_type();
	CALL	_parity_type
	NEGI	R0
	LDA	*-FP(2),AR0
	STI	R0,*+AR0(4)
L68:
	.line	38
;>>>> 						break;
	B	L65
L61:
	.line	25
	LDI	*-FP(3),R1
	CMPI	97,R1
	BZ	L62
	CMPI	98,R1
	BZ	L66
L65:
	.line	40
;>>>> 				c40_printf( "\n\n1) Parity enabled/disabled\n2) Even/odd parity\nR) Return\n\nSelection => " );
	LDI	@CONST+13,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	B	L59
L60:
	.line	42
;>>>> 			else if( (inchar == 'r') || (inchar == 'R') )
	CMPI	114,R0
	BZ	LL22
	CMPI	82,R0
	BNZ	L69
LL22:
	.line	43
;>>>> 				return;
;>>>> 			else
	B	EPI0_5
L69:
	.line	45
;>>>> 				c40_printf( "%c", 7 );
	LDI	7,R1
	PUSH	R1
	LDI	@CONST+8,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	2,SP
L59:
	.line	46
	B	L52
EPI0_5:
	.line	47
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	241,000000000H,1

	.sym	_parity_enable,_parity_enable,47,2,0
	.globl	_parity_enable

	.func	246
;>>>> 	unsigned long parity_enable( void )
******************************************************
* FUNCTION DEF : _parity_enable
******************************************************
_parity_enable:
	PUSH	FP
	LDI	SP,FP
	.line	3
;>>>> 		c40_printf( "\n\nEnable parity (y/n)?" );
;>>>> 		while( 1 )
;>>>> 			switch( c40_getchar() )
;>>>> 				case 'Y' :
;>>>> 				case 'y' :
	LDI	@CONST+14,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L70:
	B	L72
L74:
	.line	10
;>>>> 					c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	11
;>>>> 					return( 1 );
;>>>> 				case 'N' :
;>>>> 				case 'n' :
	LDI	1,R0
	B	EPI0_6
L76:
	.line	14
;>>>> 					c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	15
;>>>> 					return( 0 );
;>>>> 				default :
	LDI	0,R0
	B	EPI0_6
L77:
	.line	17
;>>>> 					c40_printf( "%c", 7 );
	LDI	7,R0
	PUSH	R0
	LDI	@CONST+8,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	.line	18
;>>>> 					break;
	B	L78
L72:
	.line	6
	CALL	_c40_getchar
	CMPI	78,R0
	BZ	L76
	CMPI	89,R0
	BZ	L74
	CMPI	110,R0
	BZ	L76
	CMPI	121,R0
	BZ	L74
	B	L77
L78:
	.line	20
	B	L70
EPI0_6:
	.line	21
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	266,000000000H,0

	.sym	_parity_type,_parity_type,47,2,0
	.globl	_parity_type

	.func	271
;>>>> 	unsigned long parity_type( void )
******************************************************
* FUNCTION DEF : _parity_type
******************************************************
_parity_type:
	PUSH	FP
	LDI	SP,FP
	.line	3
;>>>> 		c40_printf( "\n\nEven or odd parity (E/O)?" );
;>>>> 		while( 1 )
;>>>> 			switch( c40_getchar() )
;>>>> 				case 'E' :
;>>>> 				case 'e' :
	LDI	@CONST+15,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L79:
	B	L81
L83:
	.line	10
;>>>> 					c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	11
;>>>> 					return( 2 );
;>>>> 				case 'O' :
;>>>> 				case 'o' :
	LDI	2,R0
	B	EPI0_7
L85:
	.line	14
;>>>> 					c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	15
;>>>> 					return( 1 );
;>>>> 				default :
	LDI	1,R0
	B	EPI0_7
L86:
	.line	17
;>>>> 					c40_printf( "%c", 7 );
	LDI	7,R0
	PUSH	R0
	LDI	@CONST+8,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	.line	18
;>>>> 					break;
	B	L87
L81:
	.line	6
	CALL	_c40_getchar
	CMPI	69,R0
	BZ	L83
	CMPI	79,R0
	BZ	L85
	CMPI	101,R0
	BZ	L83
	CMPI	111,R0
	BZ	L85
	B	L86
L87:
	.line	20
	B	L79
EPI0_7:
	.line	21
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	291,000000000H,0

	.sym	_get_bits,_get_bits,32,2,0
	.globl	_get_bits

	.func	299
;>>>> 	void get_bits( hydra_conf *config, char which )
******************************************************
* FUNCTION DEF : _get_bits
******************************************************
_get_bits:
	PUSH	FP
	LDI	SP,FP
	.sym	_config,-2,24,9,32,.fake1
	.sym	_which,-3,2,9,32
	.line	2
	.line	3
;>>>> 		c40_printf( "\n\nBits per character (7/8)?" );
;>>>> 		while( 1 )
;>>>> 			switch( c40_getchar() )
;>>>> 				case '7' :
;>>>> 					switch( which )
;>>>> 						case 'a':
	LDI	@CONST+16,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L88:
	B	L90
L91:
	B	L92
L93:
	.line	12
;>>>> 							config->uartA.bits = 7;
	LDA	*-FP(2),AR0
	STIK	7,*+AR0(2)
	.line	13
;>>>> 							return;
;>>>> 						case 'b':
	B	EPI0_8
L94:
	.line	15
;>>>> 							config->uartB.bits = 7;
	LDA	*-FP(2),AR0
	STIK	7,*+AR0(5)
	.line	16
;>>>> 							return;
;>>>> 				case '8' :
;>>>> 					switch( which )
;>>>> 						case 'a':
	B	EPI0_8
L92:
	.line	9
	LDI	*-FP(3),R0
	CMPI	97,R0
	BZ	L93
	CMPI	98,R0
	BZ	L94
L96:
	B	L97
L98:
	.line	22
;>>>> 							config->uartA.bits = 8;
	LDA	*-FP(2),AR0
	STIK	8,*+AR0(2)
	.line	23
;>>>> 							return;
;>>>> 						case 'b':
	B	EPI0_8
L99:
	.line	25
;>>>> 							config->uartB.bits = 8;
	LDA	*-FP(2),AR0
	STIK	8,*+AR0(5)
	.line	26
;>>>> 							return;
;>>>> 				default :
	B	EPI0_8
L97:
	.line	19
	LDI	*-FP(3),R0
	CMPI	97,R0
	BZ	L98
	CMPI	98,R0
	BZ	L99
L101:
	.line	29
;>>>> 					c40_printf( "%c", 7 );
	LDI	7,R0
	PUSH	R0
	LDI	@CONST+8,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	.line	30
;>>>> 					break;
	B	L102
L90:
	.line	6
	CALL	_c40_getchar
	CMPI	55,R0
	BZ	L91
	CMPI	56,R0
	BZ	L96
	B	L101
L102:
	.line	32
	B	L88
EPI0_8:
	.line	33
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	331,000000000H,0

	.sym	_get_addr,_get_addr,47,2,0
	.globl	_get_addr

	.func	337
;>>>> 	unsigned long get_addr( char *str )
******************************************************
* FUNCTION DEF : _get_addr
******************************************************
_get_addr:
	PUSH	FP
	LDI	SP,FP
	ADDI	13,SP
	.sym	_str,-2,18,9,32

	.sect	".cinit"
	.word	IS1,STATIC_1
	.word	0
IS1	.set	1

	.sym	.init0,STATIC_1,50,3,288,,9
	.bss	STATIC_1,9
	.text
	.sym	_in_char,1,2,1,32
	.sym	_in_line,2,50,1,288,,9
	.sym	_i,11,4,1,32
	.sym	_ok,12,4,1,32
	.sym	_addr,13,15,1,32
	.line	2
	.line	3
;>>>> 		char in_char, in_line[9]={ '\0' };
	LDA	@CONST+17,AR0
	LDA	FP,AR1
	ADDI	2,AR1
	LDI	*AR0++,R0
	RPTS	8
	STI	R0,*AR1++
    ||	LDI	*AR0++,R0
	.line	4
;>>>> 		int i, ok=FAILURE;
;>>>> 		unsigned long addr;
	STIK	0,*+FP(12)
	.line	7
;>>>> 		while( !ok )
	LDI	*+FP(12),R0
	BNZ	EPI0_9
L103:
	.line	9
;>>>> 			c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	10
;>>>> 			c40_printf( "%s Address => ", str );
	LDI	*-FP(2),R0
	PUSH	R0
	LDI	@CONST+18,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	.line	12
;>>>> 			ok = SUCCESS;
	STIK	1,*+FP(12)
	.line	13
;>>>> 			i=0;
	STIK	0,*+FP(11)
	.line	14
;>>>> 			while( (c40_putchar((in_char = c40_getchar()))) != '\n' )
	CALL	_c40_getchar
	STI	R0,*+FP(1)
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	CMPI	10,R0
	BZ	L106
L105:
	.line	16
;>>>> 				if( in_char == '\b' )
	CMPI	8,*+FP(1)
	BNZ	L107
	.line	18
;>>>> 					if( i )
	LDI	*+FP(11),R0
	BZ	L108
	.line	19
;>>>> 						i--;
	SUBI	1,R0,R1
	STI	R1,*+FP(11)
L108:
	B	L109
L107:
	.line	21
;>>>> 				else if( in_char == 0xd ) /* 0xd is a carriage return */
	CMPI	13,*+FP(1)
	BZ	L111
	.line	22
;>>>> 					continue;
;>>>> 				else
	.line	24
;>>>> 					in_line[i++] = in_char;
	ADDI	1,*+FP(11),IR1
	STI	IR1,*+FP(11)
	ADDI	1,IR1
	LDI	*+FP(1),R0
	STI	R0,*+FP(IR1)
L109:
	.line	26
;>>>> 				if( i > 8 )
	CMPI	8,*+FP(11)
	BLE	L111
	.line	28
;>>>> 					c40_printf( "\nLine too long.\n" );
	LDI	@CONST+19,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	29
;>>>> 					ok = FAILURE;
	STIK	0,*+FP(12)
	.line	30
;>>>> 					break;
	B	L106
L111:
	.line	14
	CALL	_c40_getchar
	STI	R0,*+FP(1)
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	CMPI	10,R0
	BNZ	L105
L106:
	.line	33
;>>>> 			in_line[i] = '\0';
	LDA	*+FP(11),IR0
	ADDI	2,IR0
	STIK	0,*+FP(IR0)
	.line	35
;>>>> 			if( !ok )
	LDI	*+FP(12),R0
	BZ	L114
	.line	36
;>>>> 				continue;
	.line	38
;>>>> 			if( i == 0 )
	LDI	*+FP(11),R1
	BNZ	L115
	.line	39
;>>>> 				return( 0 );	
	LDI	0,R0
	B	EPI0_9
L115:
	.line	41
;>>>> 			ok = SUCCESS;
	STIK	1,*+FP(12)
	.line	42
;>>>> 			addr = atox( in_line, &ok );
	LDI	FP,R0
	ADDI	12,R0
	PUSH	R0
	LDI	FP,R0
	ADDI	2,R0
	PUSH	R0
	CALL	_atox
	SUBI	2,SP
	STI	R0,*+FP(13)
	.line	43
;>>>> 			if( ok == FAILURE )
	LDI	*+FP(12),R1
	BNZ	L116
	.line	44
;>>>> 				c40_printf( "Not a valid number : %s\n", in_line );
;>>>> 			else
	LDI	FP,R2
	ADDI	2,R2
	PUSH	R2
	LDI	@CONST+20,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	2,SP
	B	L114
L116:
	.line	46
;>>>> 				return( addr );
	B	EPI0_9
L114:
	.line	7
	LDI	*+FP(12),R0
	BZ	L103
EPI0_9:
	.line	48
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	15,SP
	B	R1
	.endfunc	384,000000000H,13

	.sym	_get_clock,_get_clock,47,2,0
	.globl	_get_clock

	.func	389
;>>>> 	unsigned long get_clock( void )
******************************************************
* FUNCTION DEF : _get_clock
******************************************************
_get_clock:
	PUSH	FP
	LDI	SP,FP
	.line	3
;>>>> 		c40_printf( "\n\n" );
	LDI	@CONST+4,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	4
;>>>> 		c40_printf( "1) 40 MHz\n2) 50 MHz\n" );
	LDI	@CONST+21,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	5
;>>>> 		c40_printf( "Selection => " );
;>>>> 		while( 1 )
;>>>> 			switch( c40_getchar() )
;>>>> 				case '1' :
	LDI	@CONST+7,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L118:
	B	L120
L121:
	.line	12
;>>>> 					c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	13
;>>>> 					return( (unsigned long) 40 );
;>>>> 				case '2' :
	LDI	40,R0
	B	EPI0_10
L122:
	.line	15
;>>>> 					c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	16
;>>>> 					return( (unsigned long) 50 );
;>>>> 				default :
	LDI	50,R0
	B	EPI0_10
L123:
	.line	18
;>>>> 					c40_printf( "%c", 7 );
	LDI	7,R0
	PUSH	R0
	LDI	@CONST+8,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	B	L124
L120:
	.line	9
	CALL	_c40_getchar
	CMPI	49,R0
	BZ	L121
	CMPI	50,R0
	BZ	L122
	B	L123
L124:
	.line	20
	B	L118
EPI0_10:
	.line	21
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	409,000000000H,0

	.sym	_get_sram,_get_sram,32,2,0
	.globl	_get_sram

	.func	415
;>>>> 	void get_sram( hydra_conf *config )
******************************************************
* FUNCTION DEF : _get_sram
******************************************************
_get_sram:
	PUSH	FP
	LDI	SP,FP
	ADDI	3,SP
	.sym	_config,-2,24,9,32,.fake1
	.sym	_flag,1,4,1,32
	.sym	_inchar,2,2,1,32
	.sym	_ptr,3,31,1,32
	.line	2
	.line	3
;>>>> 		int flag=FALSE;
;>>>> 		char inchar;
;>>>> 		unsigned long *ptr;
	STIK	0,*+FP(1)
	.line	7
;>>>> 		c40_printf( "\nFor which processor (1..4) " );
	LDI	@CONST+22,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	8
;>>>> 		while( !flag )
	LDI	*+FP(1),R0
	BNZ	L126
L125:
	.line	10
;>>>> 			inchar = c40_getchar();
	CALL	_c40_getchar
	STI	R0,*+FP(2)
	.line	11
;>>>> 			if( (inchar >= '1') && (inchar <= '4') )
;>>>> 				switch( inchar )
;>>>> 					case '1':
	CMPI	49,R0
	BLT	L127
	CMPI	52,R0
	BGT	L127
	B	L128
L129:
	.line	16
;>>>> 						ptr = &config->sram1_size;
	LDI	*-FP(2),R0
	ADDI	9,R0
	STI	R0,*+FP(3)
	.line	17
;>>>> 						break;
;>>>> 					case '2':
	B	L130
L131:
	.line	19
;>>>> 						ptr = &config->sram2_size;
	LDI	*-FP(2),R0
	ADDI	10,R0
	STI	R0,*+FP(3)
	.line	20
;>>>> 						break;
;>>>> 					case '3':
	B	L130
L132:
	.line	22
;>>>> 						ptr = &config->sram3_size;
	LDI	*-FP(2),R0
	ADDI	11,R0
	STI	R0,*+FP(3)
	.line	23
;>>>> 						break;
;>>>> 					case '4':
	B	L130
L133:
	.line	25
;>>>> 						ptr = &config->sram4_size;
	LDI	*-FP(2),R0
	ADDI	12,R0
	STI	R0,*+FP(3)
	.line	26
;>>>> 						break;
	B	L130
L128:
	.line	13
	LDA	*+FP(2),IR1
	LDA	@CONST+23,AR0
	SUBI	49,IR1
	CMPI	3,IR1
	LDIHI	4,IR1
	LDA	*+AR0(IR1),AR0
	B	AR0
	.sect	".const"
LL31:
	.word	L129
	.word	L131
	.word	L132
	.word	L133
	.word	L130
	.text
L130:
	.line	28
;>>>> 				flag = TRUE;
;>>>> 			else
	STIK	1,*+FP(1)
	B	L134
L127:
	.line	31
;>>>> 				c40_printf( "%c", 7 );
	LDI	7,R1
	PUSH	R1
	LDI	@CONST+8,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	2,SP
L134:
	.line	8
	LDI	*+FP(1),R0
	BZ	L125
L126:
	.line	33
;>>>> 		c40_printf( "\n" );
	LDI	@CONST+5,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	1,SP
	.line	35
;>>>> 		c40_printf( "\n\n" );
	LDI	@CONST+4,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	36
;>>>> 		c40_printf( "1) 16 Kwords\n2) 64 Kwords\n3) 256 Kwords\n" );
	LDI	@CONST+24,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	37
;>>>> 		c40_printf( "Selection => " );
;>>>> 		while( 1 )
;>>>> 			switch( c40_getchar() )
;>>>> 				case '1' :
	LDI	@CONST+7,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L135:
	B	L137
L138:
	.line	44
;>>>> 					c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	45
;>>>> 					*ptr = 16;
	LDA	*+FP(3),AR0
	LDI	16,R0
	STI	R0,*AR0
	.line	46
;>>>> 					return;
;>>>> 				case '2' :
	B	EPI0_11
L139:
	.line	48
;>>>> 					c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	49
;>>>> 					*ptr = 64;
	LDA	*+FP(3),AR0
	LDI	64,R0
	STI	R0,*AR0
	.line	50
;>>>> 					return;
;>>>> 				case '3' :
	B	EPI0_11
L140:
	.line	52
;>>>> 					c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	53
;>>>> 					*ptr = 256;
	LDA	*+FP(3),AR0
	LDI	256,R0
	STI	R0,*AR0
	.line	54
;>>>> 					return;
;>>>> 				default :
	B	EPI0_11
L141:
	.line	56
;>>>> 					c40_printf( "%c", 7 );
	LDI	7,R0
	PUSH	R0
	LDI	@CONST+8,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	B	L142
L137:
	.line	41
	CALL	_c40_getchar
	CMPI	49,R0
	BZ	L138
	CMPI	50,R0
	BZ	L139
	CMPI	51,R0
	BZ	L140
	B	L141
L142:
	.line	58
	B	L135
EPI0_11:
	.line	59
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	5,SP
	B	R1
	.endfunc	473,000000000H,3

	.sym	_get_daughter,_get_daughter,47,2,0
	.globl	_get_daughter

	.func	477
;>>>> 	unsigned long get_daughter( void )
******************************************************
* FUNCTION DEF : _get_daughter
******************************************************
_get_daughter:
	PUSH	FP
	LDI	SP,FP
	.line	3
;>>>> 		c40_printf( "\n\nIs a daughter card attached (y/n)?" );
;>>>> 		while( 1 )
;>>>> 			switch( c40_getchar() )
;>>>> 				case 'Y' :
;>>>> 				case 'y' :
	LDI	@CONST+25,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L143:
	B	L145
L147:
	.line	10
;>>>> 					c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	11
;>>>> 					return( 1 );
;>>>> 				case 'N' :
;>>>> 				case 'n' :
	LDI	1,R0
	B	EPI0_12
L149:
	.line	14
;>>>> 					c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	15
;>>>> 					return( 0 );
;>>>> 				default :
	LDI	0,R0
	B	EPI0_12
L150:
	.line	17
;>>>> 					c40_printf( "%c", 7 );
	LDI	7,R0
	PUSH	R0
	LDI	@CONST+8,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	.line	18
;>>>> 					break;
	B	L151
L145:
	.line	6
	CALL	_c40_getchar
	CMPI	78,R0
	BZ	L149
	CMPI	89,R0
	BZ	L147
	CMPI	110,R0
	BZ	L149
	CMPI	121,R0
	BZ	L147
	B	L150
L151:
	.line	20
	B	L143
EPI0_12:
	.line	21
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	497,000000000H,0

	.sym	_get_dram,_get_dram,46,2,0
	.globl	_get_dram

	.func	501
;>>>> 	unsigned int get_dram( void )
******************************************************
* FUNCTION DEF : _get_dram
******************************************************
_get_dram:
	PUSH	FP
	LDI	SP,FP
	.line	3
;>>>> 		c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	4
;>>>> 		c40_printf( "1) 1 MWords\n2) 4 MWords\n3) 16 MWords\n\n" );
	LDI	@CONST+26,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	5
;>>>> 		c40_printf( "Selection => " );
;>>>> 		while( 1 )
;>>>> 			switch( c40_getchar() )
;>>>> 				case '1' :
	LDI	@CONST+7,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L152:
	B	L154
L155:
	.line	12
;>>>> 					c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	13
;>>>> 					return( (unsigned long) 1 );
;>>>> 					break;
;>>>> 				case '2' :
	LDI	1,R0
	B	EPI0_13
L157:
	.line	16
;>>>> 					c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	17
;>>>> 					return( (unsigned long) 4 );
;>>>> 					break;
;>>>> 				case '3' :
	LDI	4,R0
	B	EPI0_13
L158:
	.line	20
;>>>> 					c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	21
;>>>> 					return( (unsigned long) 16 );
;>>>> 					break;
;>>>> 				default :
	LDI	16,R0
	B	EPI0_13
L159:
	.line	24
;>>>> 					c40_printf( "%c", 7 );
	LDI	7,R0
	PUSH	R0
	LDI	@CONST+8,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	.line	25
;>>>> 					break;
	B	L156
L154:
	.line	9
	CALL	_c40_getchar
	CMPI	49,R0
	BZ	L155
	CMPI	50,R0
	BZ	L157
	CMPI	51,R0
	BZ	L158
	B	L159
L156:
	.line	27
	B	L152
EPI0_13:
	.line	28
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	528,000000000H,0

	.sym	_read_config,_read_config,36,2,0
	.globl	_read_config

	.func	532
;>>>> 	int read_config( hydra_conf *config )
******************************************************
* FUNCTION DEF : _read_config
******************************************************
_read_config:
	PUSH	FP
	LDI	SP,FP
	ADDI	3,SP
	.sym	_config,-2,24,9,32,.fake1
	.sym	_chksum,1,15,1,32
	.sym	_val,2,15,1,32
	.sym	_temp,3,14,1,32
	.line	2
	.line	3
;>>>> 		unsigned long chksum=0, val;
;>>>> 		unsigned int temp;
	STIK	0,*+FP(1)
	.line	7
;>>>> 		val = ReadEepromWord( 60 );
;>>>> 		switch( (val&0x1C00) >> 10 )
;>>>> 			case 0:
	LDI	60,R0
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	B	L160
L161:
	.line	11
;>>>> 				config->uartA.baud = 150;
	LDA	*-FP(2),AR0
	LDI	150,R0
	STI	R0,*AR0
	.line	12
;>>>> 				break;
;>>>> 			case 1:
	B	L162
L163:
	.line	14
;>>>> 				config->uartA.baud = 300;
	LDA	*-FP(2),AR0
	LDI	300,R0
	STI	R0,*AR0
	.line	15
;>>>> 				break;
;>>>> 			case 2:
	B	L162
L164:
	.line	17
;>>>> 				config->uartA.baud = 600;
	LDA	*-FP(2),AR0
	LDI	600,R0
	STI	R0,*AR0
	.line	18
;>>>> 				break;
;>>>> 			case 3:
	B	L162
L165:
	.line	20
;>>>> 				config->uartA.baud = 1200;
	LDA	*-FP(2),AR0
	LDI	1200,R0
	STI	R0,*AR0
	.line	21
;>>>> 				break;
;>>>> 			case 4:
	B	L162
L166:
	.line	23
;>>>> 				config->uartA.baud = 2400;
	LDA	*-FP(2),AR0
	LDI	2400,R0
	STI	R0,*AR0
	.line	24
;>>>> 				break;
;>>>> 			case 5:
	B	L162
L167:
	.line	26
;>>>> 				config->uartA.baud = 4800;
	LDA	*-FP(2),AR0
	LDI	4800,R0
	STI	R0,*AR0
	.line	27
;>>>> 				break;
;>>>> 			case 6:
	B	L162
L168:
	.line	29
;>>>> 				config->uartA.baud = 9600;
	LDA	*-FP(2),AR0
	LDI	9600,R0
	STI	R0,*AR0
	.line	30
;>>>> 				break;
;>>>> 			case 7:
	B	L162
L169:
	.line	32
;>>>> 				config->uartA.baud = 19200;
	LDA	*-FP(2),AR0
	LDI	19200,R0
	STI	R0,*AR0
	.line	33
;>>>> 				break;
	B	L162
L160:
	.line	8
	AND	01c00h,R0
	LSH	-10,R0,IR0
	LDA	@CONST+27,AR0
	CMPI	7,IR0
	LDIHI	8,IR0
	LDA	*+AR0(IR0),AR0
	B	AR0
	.sect	".const"
LL37:
	.word	L161
	.word	L163
	.word	L164
	.word	L165
	.word	L166
	.word	L167
	.word	L168
	.word	L169
	.word	L162
	.text
L162:
	.line	36
;>>>> 		if( val & 0x8000 )
	LDI	@CONST+1,R0
	TSTB	R0,*+FP(2)
	BZ	L170
	.line	37
;>>>> 			if( val & 0x4000 )
	LDI	16384,R1
	TSTB	R1,*+FP(2)
	BZ	L171
	.line	38
;>>>> 				config->uartA.parity = 2;
;>>>> 			else
	LDA	*-FP(2),AR0
	STIK	2,*+AR0(1)
	B	L172
L171:
	.line	40
;>>>> 				config->uartA.parity = 1;
;>>>> 		else
	LDA	*-FP(2),AR0
	STIK	1,*+AR0(1)
L172:
	B	L173
L170:
	.line	42
;>>>> 			if( val & 0x4000 )
	LDI	16384,R1
	TSTB	R1,*+FP(2)
	BZ	L174
	.line	43
;>>>> 				config->uartA.parity = -2;
;>>>> 			else
	LDA	*-FP(2),AR0
	STIK	-2,*+AR0(1)
	B	L173
L174:
	.line	45
;>>>> 				config->uartA.parity = -1;
	LDA	*-FP(2),AR0
	STIK	-1,*+AR0(1)
L173:
	.line	47
;>>>> 		if( val & 0x2000 )
	LDI	8192,R2
	TSTB	R2,*+FP(2)
	BZ	L175
	.line	48
;>>>> 			config->uartA.bits = 8;
;>>>> 		else
	STIK	8,*+AR0(2)
	B	L176
L175:
	.line	50
;>>>> 			config->uartA.bits = 7;
	STIK	7,*+AR0(2)
L176:
	.line	53
;>>>> 		val = ReadEepromWord( 62 );
;>>>> 		switch( (val&0x1C00) >> 10 )
;>>>> 			case 0:
	LDI	62,R3
	PUSH	R3
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	B	L177
L178:
	.line	57
;>>>> 				config->uartB.baud = 150;
	LDA	*-FP(2),AR0
	LDI	150,R0
	STI	R0,*+AR0(3)
	.line	58
;>>>> 				break;
;>>>> 			case 1:
	B	L179
L180:
	.line	60
;>>>> 				config->uartB.baud = 300;
	LDA	*-FP(2),AR0
	LDI	300,R0
	STI	R0,*+AR0(3)
	.line	61
;>>>> 				break;
;>>>> 			case 2:
	B	L179
L181:
	.line	63
;>>>> 				config->uartB.baud = 600;
	LDA	*-FP(2),AR0
	LDI	600,R0
	STI	R0,*+AR0(3)
	.line	64
;>>>> 				break;
;>>>> 			case 3:
	B	L179
L182:
	.line	66
;>>>> 				config->uartB.baud = 1200;
	LDA	*-FP(2),AR0
	LDI	1200,R0
	STI	R0,*+AR0(3)
	.line	67
;>>>> 				break;
;>>>> 			case 4:
	B	L179
L183:
	.line	69
;>>>> 				config->uartB.baud = 2400;
	LDA	*-FP(2),AR0
	LDI	2400,R0
	STI	R0,*+AR0(3)
	.line	70
;>>>> 				break;
;>>>> 			case 5:
	B	L179
L184:
	.line	72
;>>>> 				config->uartB.baud = 4800;
	LDA	*-FP(2),AR0
	LDI	4800,R0
	STI	R0,*+AR0(3)
	.line	73
;>>>> 				break;
;>>>> 			case 6:
	B	L179
L185:
	.line	75
;>>>> 				config->uartB.baud = 9600;
	LDA	*-FP(2),AR0
	LDI	9600,R0
	STI	R0,*+AR0(3)
	.line	76
;>>>> 				break;
;>>>> 			case 7:
	B	L179
L186:
	.line	78
;>>>> 				config->uartB.baud = 19200;
	LDA	*-FP(2),AR0
	LDI	19200,R0
	STI	R0,*+AR0(3)
	.line	79
;>>>> 				break;
	B	L179
L177:
	.line	54
	AND	01c00h,R0
	LSH	-10,R0,IR1
	LDA	@CONST+28,AR0
	CMPI	7,IR1
	LDIHI	8,IR1
	LDA	*+AR0(IR1),AR0
	B	AR0
	.sect	".const"
LL39:
	.word	L178
	.word	L180
	.word	L181
	.word	L182
	.word	L183
	.word	L184
	.word	L185
	.word	L186
	.word	L179
	.text
L179:
	.line	82
;>>>> 		if( val & 0x8000 )
	LDI	@CONST+1,R0
	TSTB	R0,*+FP(2)
	BZ	L187
	.line	83
;>>>> 			if( val & 0x4000 )
	LDI	16384,R1
	TSTB	R1,*+FP(2)
	BZ	L188
	.line	84
;>>>> 				config->uartB.parity = 2;
;>>>> 			else
	LDA	*-FP(2),AR0
	STIK	2,*+AR0(4)
	B	L189
L188:
	.line	86
;>>>> 				config->uartB.parity = 1;
;>>>> 		else
	LDA	*-FP(2),AR0
	STIK	1,*+AR0(4)
L189:
	B	L190
L187:
	.line	88
;>>>> 			if( val & 0x4000 )
	LDI	16384,R1
	TSTB	R1,*+FP(2)
	BZ	L191
	.line	89
;>>>> 				config->uartB.parity = -2;
;>>>> 			else
	LDA	*-FP(2),AR0
	STIK	-2,*+AR0(4)
	B	L190
L191:
	.line	91
;>>>> 				config->uartB.parity = -1;
	LDA	*-FP(2),AR0
	STIK	-1,*+AR0(4)
L190:
	.line	93
;>>>> 		if( val & 0x2000 )
	LDI	8192,R2
	TSTB	R2,*+FP(2)
	BZ	L192
	.line	94
;>>>> 			config->uartB.bits = 8;
;>>>> 		else
	STIK	8,*+AR0(5)
	B	L193
L192:
	.line	96
;>>>> 			config->uartB.bits = 7;
	STIK	7,*+AR0(5)
L193:
	.line	99
;>>>> 		config->dram_size = ReadEepromWord( 66 );
	LDI	66,R3
	PUSH	R3
	CALL	_ReadEepromWord
	SUBI	1,SP
	LDA	*-FP(2),AR0
	STI	R0,*+AR0(6)
	.line	100
;>>>> 		config->daughter = ReadEepromWord( 68 );
	LDI	68,R0
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	LDA	*-FP(2),AR0
	STI	R0,*+AR0(15)
	.line	101
;>>>> 		config->sram1_size = ReadEepromWord( 70 );
	LDI	70,R0
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	LDA	*-FP(2),AR0
	STI	R0,*+AR0(9)
	.line	102
;>>>> 		config->sram2_size = ReadEepromWord( 72 );
	LDI	72,R0
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	LDA	*-FP(2),AR0
	STI	R0,*+AR0(10)
	.line	103
;>>>> 		config->sram3_size = ReadEepromWord( 74 );
	LDI	74,R0
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	LDA	*-FP(2),AR0
	STI	R0,*+AR0(11)
	.line	104
;>>>> 		config->sram4_size = ReadEepromWord( 76 );
;>>>> 		switch( ReadEepromWord(58) >> 8 )
;>>>> 			case 66:
	LDI	76,R0
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	LDA	*-FP(2),AR0
	STI	R0,*+AR0(12)
	B	L194
L195:
	.line	109
;>>>> 				config->cpu_clock = 40;
	LDA	*-FP(2),AR0
	LDI	40,R0
	STI	R0,*+AR0(7)
	.line	110
;>>>> 				break;
;>>>> 			case 82:
	B	L196
L197:
	.line	112
;>>>> 				config->cpu_clock = 50;
	LDA	*-FP(2),AR0
	LDI	50,R0
	STI	R0,*+AR0(7)
	.line	113
;>>>> 				break;
;>>>> 			default:
	B	L196
L198:
	.line	115
;>>>> 				config->cpu_clock = 40;
	LDA	*-FP(2),AR0
	LDI	40,R0
	STI	R0,*+AR0(7)
	.line	116
;>>>> 				break;
	B	L196
L194:
	.line	106
	LDI	58,R0
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	ASH	-8,R0
	CMPI	66,R0
	BZ	L195
	CMPI	82,R0
	BZ	L197
	B	L198
L196:
	.line	119
;>>>> 		val = ReadEepromWord( 16 );
	LDI	16,R0
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	.line	120
;>>>> 		config->l_dram_base = (val << 14) | 0x80000000;
	LSH	14,R0,R1
	OR	@CONST+29,R1
	LDA	*-FP(2),AR0
	STI	R1,*+AR0(13)
	.line	122
;>>>> 		val = ReadEepromWord( 24 );
	LDI	24,R1
	PUSH	R1
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	.line	123
;>>>> 		config->l_jtag_base = (val << 14) | 0x80000000;
	LSH	14,R0,R1
	OR	@CONST+29,R1
	LDA	*-FP(2),AR0
	STI	R1,*+AR0(14)
EPI0_14:
	.line	124
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	5,SP
	B	R1
	.endfunc	655,000000000H,3

	.sym	_display_conf,_display_conf,32,2,0
	.globl	_display_conf

	.func	661
;>>>> 	void display_conf( hydra_conf config )
;>>>> 		int i, j, tempA, tempB, Dtemp, temparry[4];
;>>>> 		char	*ptrA, *ptrB;
******************************************************
* FUNCTION DEF : _display_conf
******************************************************
_display_conf:
	PUSH	FP
	LDI	SP,FP
	ADDI	11,SP
	.sym	_config,-17,8,9,512,.fake1
	.sym	_i,1,4,1,32
	.sym	_j,2,4,1,32
	.sym	_tempA,3,4,1,32
	.sym	_tempB,4,4,1,32
	.sym	_Dtemp,5,4,1,32
	.sym	_temparry,6,52,1,128,,4
	.sym	_ptrA,10,18,1,32
	.sym	_ptrB,11,18,1,32
	.line	7
;>>>> 		c40_printf( "\n\n" );
	LDI	@CONST+4,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	9
;>>>> 		c40_printf( "                           Channel A              Channel B\n" );
	LDI	@CONST+30,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	10
;>>>> 		c40_printf( "Baud rate                     %d                   %d\n", config.uartA.baud, config.uartB.baud );
	LDI	*-FP(14),R0
	PUSH	R0
	LDI	*-FP(17),R1
	PUSH	R1
	LDI	@CONST+31,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	3,SP
	.line	12
;>>>> 		i = config.uartA.parity;
	LDI	*-FP(16),R0
	STI	R0,*+FP(1)
	.line	13
;>>>> 		if( i > 0 )
	BLE	L199
	.line	14
;>>>> 			ptrA = " En";
;>>>> 		else
	LDI	@CONST+32,R1
	STI	R1,*+FP(10)
	B	L200
L199:
	.line	16
;>>>> 			ptrA = "Dis";
	LDI	@CONST+33,R1
	STI	R1,*+FP(10)
L200:
	.line	18
;>>>> 		i = config.uartB.parity;
	LDI	*-FP(13),R2
	STI	R2,*+FP(1)
	.line	19
;>>>> 		if( i > 0 )
	BLE	L201
	.line	20
;>>>> 			ptrB = " En";
;>>>> 		else
	LDI	@CONST+32,R3
	STI	R3,*+FP(11)
	B	L202
L201:
	.line	22
;>>>> 			ptrB = "Dis";
	LDI	@CONST+33,R3
	STI	R3,*+FP(11)
L202:
	.line	24
;>>>> 		c40_printf( "Parity                    %sabled/%s           %sabled/%s\n"
;>>>> 		          , ptrA, abs(config.uartA.parity)==1?"Odd":"Even"
;>>>> 		          , ptrB, abs(config.uartB.parity)==1?"Odd":"Even" );
	ABSI	R2,R9
	CMPI	1,R9
	LDIZ	@CONST+34,R9
	LDINZ	@CONST+35,R9
	PUSH	R9
	PUSH	R3
	ABSI	R0,R9
	CMPI	1,R9
	LDIZ	@CONST+34,R9
	LDINZ	@CONST+35,R9
	PUSH	R9
	PUSH	R1
	LDI	@CONST+36,R9
	PUSH	R9
	CALL	_c40_printf
	SUBI	5,SP
	.line	27
;>>>> 		c40_printf( "Bits per character             %d                      %d\n"
;>>>> 		          , config.uartA.bits, config.uartB.bits );
	LDI	*-FP(12),R0
	PUSH	R0
	LDI	*-FP(15),R1
	PUSH	R1
	LDI	@CONST+37,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	3,SP
	.line	30
;>>>> 		c40_printf( "CPU clock rate = %d MHz.\n", config.cpu_clock );
	LDI	*-FP(10),R0
	PUSH	R0
	LDI	@CONST+38,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	.line	32
;>>>> 		temparry[0] = config.sram1_size;
	LDI	*-FP(8),R0
	STI	R0,*+FP(6)
	.line	33
;>>>> 		temparry[1] = config.sram2_size;
	LDI	*-FP(7),R1
	STI	R1,*+FP(7)
	.line	34
;>>>> 		temparry[2] = config.sram3_size;
	LDI	*-FP(6),R2
	STI	R2,*+FP(8)
	.line	35
;>>>> 		temparry[3] = config.sram4_size;
	LDI	*-FP(5),R3
	STI	R3,*+FP(9)
	.line	36
;>>>> 		if( config.daughter )
	LDI	*-FP(2),R9
	BZ	L203
	.line	37
;>>>> 			j = 4;
;>>>> 		else
	STIK	4,*+FP(2)
	B	L204
L203:
	.line	39
;>>>> 			j = 2;
	STIK	2,*+FP(2)
L204:
	.line	40
;>>>> 		for( i=0 ; i < j ; i++ )
;>>>> 			switch( temparry[i] )
;>>>> 				case 16 :
;>>>> 				case 64 :
;>>>> 				case 256 :
	STIK	0,*+FP(1)
	CMPI	*+FP(2),*+FP(1)
	BGE	L206
L205:
	B	L207
L210:
	.line	47
;>>>> 					c40_printf( "DSP %d SRAM size = %d KWords.", i+1, temparry[i] );
	LDA	*+FP(1),IR0
	ADDI	6,IR0
	LDI	*+FP(IR0),R0
	PUSH	R0
	LDI	*+FP(1),R0
	ADDI	1,R0
	PUSH	R0
	LDI	@CONST+39,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	3,SP
	.line	48
;>>>> 					if( !(i % 2) )
	TSTB	1,*+FP(1)
	BNZ	L211
	.line	49
;>>>> 						c40_printf( "  " );
;>>>> 					else
	LDI	@CONST+40,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	B	L212
L211:
	.line	51
;>>>> 						c40_printf( "\n" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L212:
	.line	52
;>>>> 					break;
;>>>> 				default:
	B	L213
L214:
	.line	54
;>>>> 					c40_printf( "Invalid DSP %d SRAM size.\n", i+1 );
	LDI	*+FP(1),R0
	ADDI	1,R0
	PUSH	R0
	LDI	@CONST+41,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
	.line	55
;>>>> 					break;
;>>>> 		switch( config.dram_size )
;>>>> 			case 1 :
;>>>> 			case 4 :
;>>>> 			case 16 :
	B	L213
L207:
	.line	42
	LDA	*+FP(1),IR1
	ADDI	6,IR1
	LDI	*+FP(IR1),R0
	CMPI	16,R0
	BZ	L210
	CMPI	64,R0
	BZ	L210
	CMPI	256,R0
	BZ	L210
	B	L214
L213:
	.line	40
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	*+FP(2),R0
	BLT	L205
L206:
	B	L215
L218:
	.line	64
;>>>> 				c40_printf( "DRAM size = %d MWords.\n", config.dram_size );
	LDI	*-FP(11),R0
	PUSH	R0
	LDI	@CONST+42,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	.line	65
;>>>> 				break;
;>>>> 			default:
	B	L219
L220:
	.line	67
;>>>> 				c40_printf( "Invalid DRAM size.\n" );
	LDI	@CONST+43,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	68
;>>>> 				break;
	B	L219
L215:
	.line	59
	LDI	*-FP(11),R0
	CMPI	1,R0
	BZ	L218
	CMPI	4,R0
	BZ	L218
	CMPI	16,R0
	BZ	L218
	B	L220
L219:
	.line	71
;>>>> 		c40_printf( "DRAM base address = %xH\n", config.l_dram_base );
	LDI	*-FP(4),R0
	PUSH	R0
	LDI	@CONST+44,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	.line	73
;>>>> 		c40_printf( "JTAG / Hydra control register address = %xH\n", config.l_jtag_base );
	LDI	*-FP(3),R0
	PUSH	R0
	LDI	@CONST+45,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	.line	75
;>>>> 		c40_printf( "Daughter card %spresent\n", config.daughter?"":"not " );
	LDI	*-FP(2),R0
	LDINZ	@CONST+46,R1
	LDIZ	@CONST+47,R1
	PUSH	R1
	LDI	@CONST+48,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
EPI0_15:
	.line	76
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	13,SP
	B	R1
	.endfunc	736,000000000H,11

	.sym	_write_config,_write_config,32,2,0
	.globl	_write_config

	.func	743
;>>>> 	void write_config( hydra_conf config )
;>>>> 		unsigned long val;
******************************************************
* FUNCTION DEF : _write_config
******************************************************
_write_config:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_config,-17,8,9,512,.fake1
	.sym	_val,1,15,1,32
	.line	6
;>>>> 		val = ReadEepromWord( 60 );
;>>>> 		switch( config.uartA.baud )
;>>>> 			case 150:
	LDI	60,R0
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(1)
	B	L221
L222:
	.line	10
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	11
;>>>> 				val |= 0x0000;
	.line	12
;>>>> 				break;
;>>>> 			case 300:
	B	L223
L224:
	.line	14
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	15
;>>>> 				val |= 0x0400;
	OR	0400h,R1
	STI	R1,*+FP(1)
	.line	16
;>>>> 				break;
;>>>> 			case 600:
	B	L223
L225:
	.line	18
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	19
;>>>> 				val |= 0x0800;
	OR	0800h,R1
	STI	R1,*+FP(1)
	.line	20
;>>>> 				break;
;>>>> 			case 1200:
	B	L223
L226:
	.line	22
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	23
;>>>> 				val |= 0x0C00;
	OR	0c00h,R1
	STI	R1,*+FP(1)
	.line	24
;>>>> 				break;
;>>>> 			case 2400:
	B	L223
L227:
	.line	26
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	27
;>>>> 				val |= 0x1000;
	OR	01000h,R1
	STI	R1,*+FP(1)
	.line	28
;>>>> 				break;
;>>>> 			case 4800:
	B	L223
L228:
	.line	30
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	31
;>>>> 				val |= 0x1400;
	OR	01400h,R1
	STI	R1,*+FP(1)
	.line	32
;>>>> 				break;
;>>>> 			case 9600:
	B	L223
L229:
	.line	34
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	35
;>>>> 				val |= 0x1800;
	OR	01800h,R1
	STI	R1,*+FP(1)
	.line	36
;>>>> 				break;
;>>>> 			case 19200:
	B	L223
L230:
	.line	38
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	39
;>>>> 				val |= 0x1c00;
	OR	01c00h,R1
	STI	R1,*+FP(1)
	.line	40
;>>>> 				break;
	B	L223
L221:
	.line	7
	LDI	*-FP(17),R1
	CMPI	150,R1
	BZ	L222
	CMPI	300,R1
	BZ	L224
	CMPI	600,R1
	BZ	L225
	CMPI	1200,R1
	BZ	L226
	CMPI	2400,R1
	BZ	L227
	CMPI	4800,R1
	BZ	L228
	CMPI	9600,R1
	BZ	L229
	CMPI	19200,R1
	BZ	L230
L223:
	.line	43
;>>>> 		if( config.uartA.parity > 0 )
	LDI	*-FP(16),R0
	BLE	L231
	.line	44
;>>>> 			if( config.uartA.parity % 2 )
	TSTB	1,R0
	BZ	L232
	.line	46
;>>>> 				val |= 0x8000;
	LDI	@CONST+1,R1
	OR	R1,*+FP(1),R2
	STI	R2,*+FP(1)
	.line	47
;>>>> 				val &= 0xBFFF;
;>>>> 			else
	AND	0bfffh,R2
	STI	R2,*+FP(1)
	B	L233
L232:
	.line	51
;>>>> 				val |= 0xC000;
;>>>> 		else
	LDI	@CONST+50,R1
	OR	R1,*+FP(1),R2
	STI	R2,*+FP(1)
L233:
	B	L234
L231:
	.line	54
;>>>> 			if( config.uartA.parity % 2 )
	TSTB	1,R0
	BZ	L235
	.line	56
;>>>> 				val &= 0x3FFF;
;>>>> 			else
	LDI	16383,R1
	AND	R1,*+FP(1),R2
	STI	R2,*+FP(1)
	B	L234
L235:
	.line	60
;>>>> 				val |= 0x4000;
	LDI	16384,R1
	OR	R1,*+FP(1),R2
	STI	R2,*+FP(1)
	.line	61
;>>>> 				val &= 0x7FFF;
	AND	07fffh,R2
	STI	R2,*+FP(1)
L234:
	.line	63
;>>>> 		if( config.uartA.bits == 7 )
	LDI	*-FP(15),R1
	CMPI	7,R1
	BNZ	L236
	.line	64
;>>>> 			val &= 0xDFFF;
;>>>> 		else
	AND	0dfffh,R2
	STI	R2,*+FP(1)
	B	L237
L236:
	.line	66
;>>>> 			val |= 0x2000;
	OR	02000h,R2
	STI	R2,*+FP(1)
L237:
	.line	67
;>>>> 		WriteEepromWord( 60, val );
	PUSH	R2
	LDI	60,R3
	PUSH	R3
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	70
;>>>> 		val = ReadEepromWord( 62 );
;>>>> 		switch( config.uartB.baud )
;>>>> 			case 150:
	LDI	62,R0
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(1)
	B	L238
L239:
	.line	74
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	75
;>>>> 				val |= 0x0000;
	.line	76
;>>>> 				break;
;>>>> 			case 300:
	B	L240
L241:
	.line	78
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	79
;>>>> 				val |= 0x0400;
	OR	0400h,R1
	STI	R1,*+FP(1)
	.line	80
;>>>> 				break;
;>>>> 			case 600:
	B	L240
L242:
	.line	82
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	83
;>>>> 				val |= 0x0800;
	OR	0800h,R1
	STI	R1,*+FP(1)
	.line	84
;>>>> 				break;
;>>>> 			case 1200:
	B	L240
L243:
	.line	86
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	87
;>>>> 				val |= 0x0C00;
	OR	0c00h,R1
	STI	R1,*+FP(1)
	.line	88
;>>>> 				break;
;>>>> 			case 2400:
	B	L240
L244:
	.line	90
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	91
;>>>> 				val |= 0x1000;
	OR	01000h,R1
	STI	R1,*+FP(1)
	.line	92
;>>>> 				break;
;>>>> 			case 4800:
	B	L240
L245:
	.line	94
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	95
;>>>> 				val |= 0x1400;
	OR	01400h,R1
	STI	R1,*+FP(1)
	.line	96
;>>>> 				break;
;>>>> 			case 9600:
	B	L240
L246:
	.line	98
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	99
;>>>> 				val |= 0x1800;
	OR	01800h,R1
	STI	R1,*+FP(1)
	.line	100
;>>>> 				break;
;>>>> 			case 19200:
	B	L240
L247:
	.line	102
;>>>> 				val &= 0xE3FF;
	LDI	@CONST+49,R0
	AND	R0,*+FP(1),R1
	STI	R1,*+FP(1)
	.line	103
;>>>> 				val |= 0x1c00;
	OR	01c00h,R1
	STI	R1,*+FP(1)
	.line	104
;>>>> 				break;
	B	L240
L238:
	.line	71
	LDI	*-FP(14),R1
	CMPI	150,R1
	BZ	L239
	CMPI	300,R1
	BZ	L241
	CMPI	600,R1
	BZ	L242
	CMPI	1200,R1
	BZ	L243
	CMPI	2400,R1
	BZ	L244
	CMPI	4800,R1
	BZ	L245
	CMPI	9600,R1
	BZ	L246
	CMPI	19200,R1
	BZ	L247
L240:
	.line	107
;>>>> 		if( config.uartB.parity > 0 )
	LDI	*-FP(13),R0
	BLE	L248
	.line	108
;>>>> 			if( config.uartB.parity % 2 )
	TSTB	1,R0
	BZ	L249
	.line	110
;>>>> 				val |= 0x8000;
	LDI	@CONST+1,R1
	OR	R1,*+FP(1),R2
	STI	R2,*+FP(1)
	.line	111
;>>>> 				val &= 0xBFFF;
;>>>> 			else
	AND	0bfffh,R2
	STI	R2,*+FP(1)
	B	L250
L249:
	.line	115
;>>>> 				val |= 0xC000;
;>>>> 		else
	LDI	@CONST+50,R1
	OR	R1,*+FP(1),R2
	STI	R2,*+FP(1)
L250:
	B	L251
L248:
	.line	118
;>>>> 			if( config.uartB.parity % 2 )
	TSTB	1,R0
	BZ	L252
	.line	120
;>>>> 				val &= 0x3FFF;
;>>>> 			else
	LDI	16383,R1
	AND	R1,*+FP(1),R2
	STI	R2,*+FP(1)
	B	L251
L252:
	.line	124
;>>>> 				val |= 0x4000;
	LDI	16384,R1
	OR	R1,*+FP(1),R2
	STI	R2,*+FP(1)
	.line	125
;>>>> 				val &= 0x7FFF;
	AND	07fffh,R2
	STI	R2,*+FP(1)
L251:
	.line	127
;>>>> 		if( config.uartB.bits == 7 )
	LDI	*-FP(12),R1
	CMPI	7,R1
	BNZ	L253
	.line	128
;>>>> 			val &= 0xDFFF;
;>>>> 		else
	AND	0dfffh,R2
	STI	R2,*+FP(1)
	B	L254
L253:
	.line	130
;>>>> 			val |= 0x2000;
	OR	02000h,R2
	STI	R2,*+FP(1)
L254:
	.line	131
;>>>> 		WriteEepromWord( 62, val );
	PUSH	R2
	LDI	62,R3
	PUSH	R3
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	134
;>>>> 		WriteEepromWord( 66, config.dram_size );
	LDI	*-FP(11),R0
	PUSH	R0
	LDI	66,R1
	PUSH	R1
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	135
;>>>> 		WriteEepromWord( 68, config.daughter );
	LDI	*-FP(2),R0
	PUSH	R0
	LDI	68,R1
	PUSH	R1
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	136
;>>>> 		WriteEepromWord( 70, config.sram1_size );
	LDI	*-FP(8),R0
	PUSH	R0
	LDI	70,R1
	PUSH	R1
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	137
;>>>> 		WriteEepromWord( 72, config.sram2_size );
	LDI	*-FP(7),R0
	PUSH	R0
	LDI	72,R1
	PUSH	R1
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	138
;>>>> 		WriteEepromWord( 74, config.sram3_size );
	LDI	*-FP(6),R0
	PUSH	R0
	LDI	74,R1
	PUSH	R1
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	139
;>>>> 		WriteEepromWord( 76, config.sram4_size );
;>>>> 		switch( config.cpu_clock )
;>>>> 			case 40:
	LDI	*-FP(5),R0
	PUSH	R0
	LDI	76,R1
	PUSH	R1
	CALL	_WriteEepromWord
	SUBI	2,SP
	B	L255
L256:
	.line	144
;>>>> 				WriteEepromWord( 58, 66 << 8 );
	LDI	16896,R0
	PUSH	R0
	LDI	58,R1
	PUSH	R1
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	145
;>>>> 				break;
;>>>> 			case 50:
	B	L257
L258:
	.line	147
;>>>> 				WriteEepromWord( 58, 82 << 8 );
	LDI	20992,R0
	PUSH	R0
	LDI	58,R1
	PUSH	R1
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	148
;>>>> 				break;
;>>>> 			default:
	B	L257
L259:
	.line	150
;>>>> 				WriteEepromWord( 58, 66 << 8 );
	LDI	16896,R0
	PUSH	R0
	LDI	58,R1
	PUSH	R1
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	151
;>>>> 				break;
	B	L257
L255:
	.line	141
	LDI	*-FP(10),R0
	CMPI	40,R0
	BZ	L256
	CMPI	50,R0
	BZ	L258
	B	L259
L257:
	.line	155
;>>>> 		val = (config.l_dram_base & 0x7FFFFFFF) >> 14;
	LDI	*-FP(4),R0
	AND	@CONST+51,R0
	LSH	-14,R0
	STI	R0,*+FP(1)
	.line	156
;>>>> 		WriteEepromWord( 16, val );
	PUSH	R0
	LDI	16,R1
	PUSH	R1
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	158
;>>>> 		val = (config.l_jtag_base & 0x7FFFFFFF) >> 14;
	LDI	*-FP(3),R0
	AND	@CONST+51,R0
	LSH	-14,R0
	STI	R0,*+FP(1)
	.line	159
;>>>> 		WriteEepromWord( 24, val );
	PUSH	R0
	LDI	24,R1
	PUSH	R1
	CALL	_WriteEepromWord
	SUBI	2,SP
EPI0_16:
	.line	160
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	902,000000000H,1

	.sym	_update_chksum,_update_chksum,32,2,0
	.globl	_update_chksum

	.func	910
;>>>> 	void update_chksum( hydra_conf *config )
******************************************************
* FUNCTION DEF : _update_chksum
******************************************************
_update_chksum:
	PUSH	FP
	LDI	SP,FP
	ADDI	3,SP
	.sym	_config,-2,24,9,32,.fake1
	.sym	_chksum,1,15,1,32
	.sym	_i,2,4,1,32
	.sym	_temp,3,14,1,32
	.line	2
	.line	3
;>>>> 		unsigned long chksum=0;
;>>>> 		int i;
;>>>> 		unsigned int temp;
	STIK	0,*+FP(1)
	.line	8
;>>>> 		crcupdate( config->uartA.baud & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDA	*-FP(2),AR0
	LDI	255,R0
	AND	R0,*AR0,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
	.line	10
;>>>> 		crcupdate( config->uartB.baud & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDA	*-FP(2),AR0
	LDI	*+AR0(3),R0
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
	.line	12
;>>>> 		for( i=0 ; i < 4 ; i++ )
	STIK	0,*+FP(2)
	CMPI	4,*+FP(2)
	BGE	L261
L260:
	.line	13
;>>>> 			crcupdate( (config->uartA.parity>>i) & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDA	*-FP(2),AR0
	NEGI	*+FP(2),R0
	ASH	R0,*+AR0(1),R0
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
	.line	12
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L260
L261:
	.line	15
;>>>> 		for( i=0 ; i < 4 ; i++ )
	STIK	0,*+FP(2)
	CMPI	4,*+FP(2)
	BGE	L263
L262:
	.line	16
;>>>> 			crcupdate( (config->uartB.parity>>i) & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDA	*-FP(2),AR0
	NEGI	*+FP(2),R0
	LDI	*+AR0(4),R1
	ASH	R0,R1,R0
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
	.line	15
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L262
L263:
	.line	18
;>>>> 		crcupdate( config->uartA.bits & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDA	*-FP(2),AR0
	LDI	*+AR0(2),R0
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
	.line	20
;>>>> 		crcupdate( config->uartB.bits & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDA	*-FP(2),AR0
	LDI	*+AR0(5),R0
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
	.line	22
;>>>> 		crcupdate( config->dram_size & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDA	*-FP(2),AR0
	LDI	*+AR0(6),R0
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
	.line	24
;>>>> 		crcupdate( config->cpu_clock & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDA	*-FP(2),AR0
	LDI	*+AR0(7),R0
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
	.line	26
;>>>> 		for( i=0 ; i < 4 ; i++ )
	STIK	0,*+FP(2)
	CMPI	4,*+FP(2)
	BGE	L265
L264:
	.line	27
;>>>> 			crcupdate( (config->l_dram_base>>i) & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDA	*-FP(2),AR0
	NEGI	*+FP(2),R0
	LDI	*+AR0(13),R1
	LSH	R0,R1,R0
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
	.line	26
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L264
L265:
	.line	29
;>>>> 		for( i=0 ; i < 4 ; i++ )
	STIK	0,*+FP(2)
	CMPI	4,*+FP(2)
	BGE	L267
L266:
	.line	30
;>>>> 			crcupdate( (config->l_jtag_base>>i) & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDA	*-FP(2),AR0
	NEGI	*+FP(2),R0
	LDI	*+AR0(14),R1
	LSH	R0,R1,R0
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
	.line	29
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L266
L267:
	.line	32
;>>>> 		crcupdate( config->daughter &= 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDA	*-FP(2),AR0
	LDI	*+AR0(15),R0
	AND	0ffh,R0
	STI	R0,*+AR0(15)
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
	.line	34
;>>>> 		config->checksum = chksum;
	LDA	*-FP(2),AR0
	LDI	*+FP(1),R0
	STI	R0,*+AR0(8)
EPI0_17:
	.line	35
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	5,SP
	B	R1
	.endfunc	944,000000000H,3

	.sym	_WriteEepromWord,_WriteEepromWord,32,2,0
	.globl	_WriteEepromWord

	.func	947
;>>>> 	void WriteEepromWord( int WordAddress, unsigned long data )
******************************************************
* FUNCTION DEF : _WriteEepromWord
******************************************************
_WriteEepromWord:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_WordAddress,-2,4,9,32
	.sym	_data,-3,15,9,32
	.sym	_i,1,4,1,32
	.line	2
;>>>> 		int i;
	.line	5
;>>>> 		for( i=0 ; i < 2 ; i++, data >>= 8 )
	STIK	0,*+FP(1)
	CMPI	2,*+FP(1)
	BGE	EPI0_18
L268:
	.line	6
;>>>> 			WriteEeprom( WordAddress++, data & 0xff ); 
	LDI	*-FP(3),R0
	AND	0ffh,R0
	PUSH	R0
	LDI	*-FP(2),R0
	ADDI	1,R0,R1
	STI	R1,*-FP(2)
	SUBI	1,R1
	PUSH	R1
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	5
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	LDI	*-FP(3),R1
	LSH	-8,R1,R2
	STI	R2,*-FP(3)
	CMPI	2,R0
	BLT	L268
EPI0_18:
	.line	7
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	953,000000000H,1
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.sect	".const"
SL0:	.byte	"DRAM base",0
SL1:	.byte	"Not a valid base address for DRAM",10,0
SL2:	.byte	"JTAG / Hydra control register base",0
SL3:	.byte	10,10,0
SL4:	.byte	10,0
SL5:	.byte	"1) RS-232 channel A configuration",10,"2) RS-232 channel B "
	.byte	"configuration",10,"3) CPU clock rate",10,"4) SRAM size(s)",10
	.byte	"5) DRAM size",10,"6) DRAM base address",10,"7) JTAG / Hydra"
	.byte	" control register address",10,"8) Daughter card attached",10
	.byte	"R) Return",10,0
SL6:	.byte	"Selection => ",0
SL7:	.byte	"%c",0
SL8:	.byte	10,10,"1) Baud rate",10,"2) Parity",10,"3) Bits/character",10
	.byte	"R) Return",10,10,"Selection => ",0
SL9:	.byte	10,10,"Baud Rates :",10,0
SL10:	.byte	"  1) 150",10,"  2) 300",10,"  3) 600",10,"  4) 1200",10,"  "
	.byte	"5) 2400",10,"  6) 4800",10,"  7) 9600",10,"  8) 19200",10," "
	.byte	" R) Return",0
SL11:	.byte	10,10,"1) Parity enabled/disabled",10,"2) Even/odd parity",10
	.byte	"R) Return",10,10,"Selection => ",0
SL12:	.byte	10,10,"Enable parity (y/n)?",0
SL13:	.byte	10,10,"Even or odd parity (E/O)?",0
SL14:	.byte	10,10,"Bits per character (7/8)?",0
SL15:	.byte	"%s Address => ",0
SL16:	.byte	10,"Line too long.",10,0
SL17:	.byte	"Not a valid number : %s",10,0
SL18:	.byte	"1) 40 MHz",10,"2) 50 MHz",10,0
SL19:	.byte	10,"For which processor (1..4) ",0
SL20:	.byte	"1) 16 Kwords",10,"2) 64 Kwords",10,"3) 256 Kwords",10,0
SL21:	.byte	10,10,"Is a daughter card attached (y/n)?",0
SL22:	.byte	"1) 1 MWords",10,"2) 4 MWords",10,"3) 16 MWords",10,10,0
SL23:	.byte	"                           Channel A              Channel B"
	.byte	10,0
SL24:	.byte	"Baud rate                     %d                   %d",10,0
SL25:	.byte	" En",0
SL26:	.byte	"Dis",0
SL27:	.byte	"Odd",0
SL28:	.byte	"Even",0
SL29:	.byte	"Parity                    %sabled/%s           %sabled/%s",10
	.byte	0
SL30:	.byte	"Bits per character             %d                      %d",10
	.byte	0
SL31:	.byte	"CPU clock rate = %d MHz.",10,0
SL32:	.byte	"DSP %d SRAM size = %d KWords.",0
SL33:	.byte	"  ",0
SL34:	.byte	"Invalid DSP %d SRAM size.",10,0
SL35:	.byte	"DRAM size = %d MWords.",10,0
SL36:	.byte	"Invalid DRAM size.",10,0
SL37:	.byte	"DRAM base address = %xH",10,0
SL38:	.byte	"JTAG / Hydra control register address = %xH",10,0
SL39:	.byte	0
SL40:	.byte	"not ",0
SL41:	.byte	"Daughter card %spresent",10,0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,52
	.sect	".cinit"
	.word	52,CONST
	.word 	SL0              ;0
	.word 	32768            ;1
	.word 	SL1              ;2
	.word 	SL2              ;3
	.word 	SL3              ;4
	.word 	SL4              ;5
	.word 	SL5              ;6
	.word 	SL6              ;7
	.word 	SL7              ;8
	.word 	SL8              ;9
	.word 	SL9              ;10
	.word 	SL10             ;11
	.word 	LL12             ;12
	.word 	SL11             ;13
	.word 	SL12             ;14
	.word 	SL13             ;15
	.word 	SL14             ;16
	.word 	STATIC_1         ;17
	.word 	SL15             ;18
	.word 	SL16             ;19
	.word 	SL17             ;20
	.word 	SL18             ;21
	.word 	SL19             ;22
	.word 	LL31             ;23
	.word 	SL20             ;24
	.word 	SL21             ;25
	.word 	SL22             ;26
	.word 	LL37             ;27
	.word 	LL39             ;28
	.word 	080000000h       ;29
	.word 	SL23             ;30
	.word 	SL24             ;31
	.word 	SL25             ;32
	.word 	SL26             ;33
	.word 	SL27             ;34
	.word 	SL28             ;35
	.word 	SL29             ;36
	.word 	SL30             ;37
	.word 	SL31             ;38
	.word 	SL32             ;39
	.word 	SL33             ;40
	.word 	SL34             ;41
	.word 	SL35             ;42
	.word 	SL36             ;43
	.word 	SL37             ;44
	.word 	SL38             ;45
	.word 	SL39             ;46
	.word 	SL40             ;47
	.word 	SL41             ;48
	.word 	58367            ;49
	.word 	49152            ;50
	.word 	07fffffffh       ;51
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_ReadEepromWord
	.globl	_WriteEeprom
	.end
