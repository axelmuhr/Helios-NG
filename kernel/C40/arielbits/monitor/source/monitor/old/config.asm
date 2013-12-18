******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 -ic:\c40 config.c C:\TMP\config.if 
;	cg30 -v40 -o -n C:\TMP\config.if C:\TMP\config.asm C:\TMP\config.tmp 
FP	.set	AR3
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
	.globl	_configure
;>>>> 	void configure( hydra_conf *config )
;>>>> 		int i;
;>>>> 		while( 1 )
******************************************************
* FUNCTION DEF : _configure
******************************************************
_configure:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
L1:
;>>>> 			display_conf( *config );
	LDI	*-FP(2),AR0
	LDI	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*AR0++,R0
	CALL	_display_conf
	SUBI	17,SP
;>>>> 			switch( menu() )
;>>>> 				case '0':
	B	L3
L4:
;>>>> 					config->baudA = get_baud();
	CALL	_get_baud
	LDI	*-FP(2),AR0
	STI	R0,*AR0
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case '1':
	B	L1
L6:
;>>>> 					config->baudB = get_baud();
	CALL	_get_baud
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(1)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case '2':
	B	L1
L7:
;>>>> 					config->cpu_clock = get_clock();
	CALL	_get_clock
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(3)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case '3':
	B	L1
L8:
;>>>> 					config->dram_size = get_dram();
	CALL	_get_dram
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(2)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case '4':
	B	L1
L9:
;>>>> 					config->l_dram_base = get_addr( "Local DRAM base" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_get_addr
	SUBI	1,SP
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(9)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case '5':
	B	L1
L10:
;>>>> 					config->v_dram_base = get_addr( "VME DRAM base" );
	LDI	@CONST+1,R0
	PUSH	R0
	CALL	_get_addr
	SUBI	1,SP
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(10)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case '6':
	B	L1
L11:
;>>>> 					config->l_jtag_base = get_addr( "Local JTAG base" );
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_get_addr
	SUBI	1,SP
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(11)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case '7':
	B	L1
L12:
;>>>> 					config->v_jtag_base = get_addr( "VME JTAG base" );
	LDI	@CONST+3,R0
	PUSH	R0
	CALL	_get_addr
	SUBI	1,SP
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(12)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case '8':
	B	L1
L13:
;>>>> 					config->l_cont_base = get_addr( "Local bus Hydra control "
;>>>> 															 "register base" );
	LDI	@CONST+4,R0
	PUSH	R0
	CALL	_get_addr
	SUBI	1,SP
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(13)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case '9':
	B	L1
L14:
;>>>> 					config->v_cont_base = get_addr( "VME bus Hydra control"
;>>>> 															 "register base" );
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_get_addr
	SUBI	1,SP
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(14)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 	         case 'A' :
;>>>> 				case 'a' :
	B	L1
L16:
;>>>> 					config->mon_addr = get_addr( "Monitor relocation" );
	LDI	@CONST+6,R0
	PUSH	R0
	CALL	_get_addr
	SUBI	1,SP
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(15)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case 'B' :
;>>>> 				case 'b' :
	B	L1
L18:
;>>>> 					config->daughter = get_daughter();
	CALL	_get_daughter
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(16)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case 'C' :
;>>>> 				case 'c' :
	B	L1
L20:
;>>>> 					config->daughter = get_daughter();
	CALL	_get_daughter
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(16)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case 'D' :
;>>>> 				case 'd' :
	B	L1
L22:
;>>>> 					config->daughter = get_daughter();
	CALL	_get_daughter
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(16)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case 'E' :
;>>>> 				case 'e' :
	B	L1
L24:
;>>>> 					config->daughter = get_daughter();
	CALL	_get_daughter
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(16)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case 'F' :
;>>>> 				case 'f' :
	B	L1
L26:
;>>>> 					config->daughter = get_daughter();
	CALL	_get_daughter
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(16)
;>>>> 					update_chksum( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_update_chksum
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case 'R' :
;>>>> 				case 'r' :
	B	L1
L28:
;>>>> 					write_config( *config );
	LDI	*-FP(2),AR0
	LDI	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*AR0++,R0
	CALL	_write_config
	SUBI	17,SP
;>>>> 					return;
;>>>> 					break;
	B	EPI0_1
L3:
	CALL	_menu
	CMPI	48,R0
	BZ	L4
	CMPI	49,R0
	BZ	L6
	CMPI	50,R0
	BZ	L7
	CMPI	51,R0
	BZ	L8
	CMPI	52,R0
	BZ	L9
	CMPI	53,R0
	BZ	L10
	CMPI	54,R0
	BZ	L11
	CMPI	55,R0
	BZ	L12
	CMPI	56,R0
	BZ	L13
	CMPI	57,R0
	BZ	L14
	CMPI	65,R0
	BZ	L16
	CMPI	66,R0
	BZ	L18
	CMPI	67,R0
	BZ	L20
	CMPI	68,R0
	BZ	L22
	CMPI	69,R0
	BZ	L24
	CMPI	70,R0
	BZ	L26
	CMPI	82,R0
	BZ	L28
	CMPI	97,R0
	BZ	L16
	CMPI	98,R0
	BZ	L18
	CMPI	99,R0
	BZ	L20
	CMPI	100,R0
	BZ	L22
	CMPI	101,R0
	BZ	L24
	CMPI	102,R0
	BZ	L26
	CMPI	114,R0
	BZ	L28
	B	L1
EPI0_1:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
	.globl	_get_addr
;>>>> 	unsigned long get_addr( char *str )
******************************************************
* FUNCTION DEF : _get_addr
******************************************************
_get_addr:
	PUSH	FP
	LDI	SP,FP
	ADDI	13,SP
	.sect	".cinit"
	.word	IS1,STATIC_1
	.word	0
IS1	.set	1
	.bss	STATIC_1,10
	.text
;>>>> 		char inputline[10]={ '\0' };
;>>>> 		int i, ok;
;>>>> 		unsigned long addr;
	LDI	@CONST+7,AR0
	LDI	FP,AR1
	ADDI	1,AR1
	LDI	*AR0++,R0
	RPTS	9
	STI	R0,*AR1++
    ||	LDI	*AR0++,R0
;>>>> 		c40_printf( "\n" );
;>>>> 		while( 1 )
	LDI	@CONST+8,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L29:
;>>>> 			c40_printf( "%s Address => ", str );
	LDI	*-FP(2),R0
	PUSH	R0
	LDI	@CONST+9,R1
	PUSH	R1
	CALL	_c40_printf
	BD	L32
	SUBI	2,SP
;>>>> 			for( i=0 ; (i < 10) && ( (inputline[i]=c40_getchar()) != '\n') ; i++ );
	LDI	0,R0
	STI	R0,*+FP(11)
***	B	L32	;BRANCH OCCURS
L31:
	LDI	*+FP(11),R0
	ADDI	1,R0
	STI	R0,*+FP(11)
L32:
	CMPI	10,R0
	BGE	LL4
	CALL	_c40_getchar
	LDI	*+FP(11),R1
	ADDI	R1,FP,AR0
	STI	R0,*+AR0(1)
	CMPI	10,R0
	BNZ	L31
LL4:
;>>>> 			if( i == 0 )
	LDI	*+FP(11),R0
	BZ	L29
;>>>> 				continue;
;>>>> 			inputline[i] = '\0';
	ADDI	R0,FP,AR0
	LDI	0,R1
	STI	R1,*+AR0(1)
;>>>> 			addr = (inputline[i-1]=='h')||(inputline[i-1]=='H') ?
;>>>> 						 atox( inputline, &ok ) : atod( inputline, &ok );
	ADDI	R0,FP,AR0
	LDI	*AR0,R2
	CMPI	104,R2
	BZ	LL6
	ADDI	R0,FP,AR0
	LDI	*AR0,R2
	CMPI	72,R2
	BNZ	LL5
LL6:
	LDI	FP,R2
	ADDI	12,R2
	PUSH	R2
	SUBI	11,R2
	PUSH	R2
	CALL	_atox
	SUBI	2,SP
	B	LL7
LL5:
	LDI	FP,R2
	ADDI	12,R2
	PUSH	R2
	SUBI	11,R2
	PUSH	R2
	CALL	_atod
	SUBI	2,SP
LL7:
	STI	R0,*+FP(13)
;>>>> 			if( ok == FAILURE )
	LDI	*+FP(12),R1
	BNZ	EPI0_2
;>>>> 				c40_printf( "Not a valid number : %s\n", inputline );
	LDI	FP,R2
	ADDI	1,R2
	PUSH	R2
	LDI	@CONST+10,R3
	PUSH	R3
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 			else
	B	L29
;>>>> 				return( addr );
EPI0_2:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	15,SP
***	B	R1	;BRANCH OCCURS
	.globl	_get_clock
;>>>> 	unsigned long get_clock( void )
******************************************************
* FUNCTION DEF : _get_clock
******************************************************
_get_clock:
;>>>> 		c40_printf( "\n\n" );
;>>>> 		while( 1 )
	LDI	@CONST+11,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L37:
;>>>> 			c40_printf( "1) 33 MHz\n2) 40 MHz\n3) 50 MHz\n" );
	LDI	@CONST+12,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 			c40_printf( "Selection => " );
	LDI	@CONST+13,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 			switch( c40_getchar() )
;>>>> 				case '1' :
	B	L39
L40:
;>>>> 					return( (unsigned long) 33 );
;>>>> 				case '2' :
	LDI	33,R0
	B	EPI0_3
L41:
;>>>> 					return( (unsigned long) 40 );
;>>>> 				case '3' :
	LDI	40,R0
	B	EPI0_3
L42:
;>>>> 					return( (unsigned long) 50 );
;>>>> 				default :
	LDI	50,R0
	B	EPI0_3
L43:
;>>>> 					c40_printf( "\nInvalid selection.\n\n" );
	LDI	@CONST+14,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	B	L37
L39:
	CALL	_c40_getchar
	CMPI	49,R0
	BZ	L40
	CMPI	50,R0
	BZ	L41
	CMPI	51,R0
	BZ	L42
	B	L43
EPI0_3:
	RETS
	.globl	_get_daughter
;>>>> 	unsigned long get_daughter( void )
******************************************************
* FUNCTION DEF : _get_daughter
******************************************************
_get_daughter:
;>>>> 		c40_printf( "\n\nIs a daughter card attached (y/n)?" );
;>>>> 		while( 1 )
	LDI	@CONST+15,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L45:
;>>>> 			c40_printf( "Is daughter card attached (y/n)?" );
	LDI	@CONST+16,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 			switch( c40_getchar() )
;>>>> 				case 'Y' :
;>>>> 				case 'y' :
	B	L47
L49:
;>>>> 					return( 1 );
;>>>> 				case 'N' :
;>>>> 				case 'n' :
	LDI	1,R0
	B	EPI0_4
L51:
;>>>> 					return( 0 );
;>>>> 				default :
	LDI	0,R0
	B	EPI0_4
L52:
;>>>> 					c40_printf( "%c%c", 7, 8 );
	LDI	8,R0
	PUSH	R0
	LDI	7,R1
	PUSH	R1
	LDI	@CONST+17,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	3,SP
;>>>> 					break;
	B	L45
L47:
	CALL	_c40_getchar
	CMPI	78,R0
	BZ	L51
	CMPI	89,R0
	BZ	L49
	CMPI	110,R0
	BZ	L51
	CMPI	121,R0
	BZ	L49
	B	L52
EPI0_4:
	RETS
	.globl	_get_baud
;>>>> 	unsigned long get_baud( void )
;>>>> 		int i;
******************************************************
* FUNCTION DEF : _get_baud
******************************************************
_get_baud:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
;>>>> 		c40_printf( "\n\n" );
;>>>> 		while( 1 )
	LDI	@CONST+11,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L54:
;>>>> 			c40_printf( "Baud Rates :\n" );
	LDI	@CONST+18,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 			c40_printf( "  1) 75\n  2) 150\n  3) 300\n  4) 600\n  5) 1200\n"
;>>>> 							 "  6) 2400\n  7) 4800\n  8) 9600\n" );
	LDI	@CONST+19,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 			c40_printf( "Selection => " );
	LDI	@CONST+13,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 			i = c40_getchar();
	CALL	_c40_getchar
	STI	R0,*+FP(1)
;>>>> 			if( (i >= '1') && ( i <= '8') )
	CMPI	49,R0
	BLT	L56
	CMPI	56,R0
	BGT	L56
;>>>> 				return( (unsigned long) (i - '1') );
;>>>> 			else
	SUBI	49,R0
	B	EPI0_5
L56:
;>>>> 				c40_printf( "\nInvalid selection.\n\n" );
	LDI	@CONST+14,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	1,SP
	B	L54
EPI0_5:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
	.globl	_get_dram
;>>>> 	unsigned int get_dram( void )
******************************************************
* FUNCTION DEF : _get_dram
******************************************************
_get_dram:
;>>>> 		c40_printf( "\n\n" );
;>>>> 		while( 1 )
	LDI	@CONST+11,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L57:
;>>>> 			c40_printf( "1) 4 Mbyte\n2) 16 Mbyte\n3) 64 Mbyte\n" );
	LDI	@CONST+20,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 			c40_printf( "Selection => " );
	LDI	@CONST+13,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 			switch( c40_getchar() )
;>>>> 				case '1' :
	B	L59
L60:
;>>>> 					return( (unsigned long) 4 );
;>>>> 				case '2' :
	LDI	4,R0
	B	EPI0_6
L61:
;>>>> 					return( (unsigned long) 16 );
;>>>> 				case '3' :
	LDI	16,R0
	B	EPI0_6
L62:
;>>>> 					return( (unsigned long) 64 );
;>>>> 				default :
	LDI	64,R0
	B	EPI0_6
L63:
;>>>> 					c40_printf( "\nInvalid selection.\n\n" );
	LDI	@CONST+14,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	B	L57
L59:
	CALL	_c40_getchar
	CMPI	49,R0
	BZ	L60
	CMPI	50,R0
	BZ	L61
	CMPI	51,R0
	BZ	L62
	B	L63
EPI0_6:
	RETS
	.globl	_menu
;>>>> 	unsigned long menu( void )
;>>>> 		int i;
******************************************************
* FUNCTION DEF : _menu
******************************************************
_menu:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
;>>>> 		c40_printf( "\n" );
;>>>> 		while( 1 )
	LDI	@CONST+8,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L65:
;>>>> 			c40_printf(  "0) Serial channel A baud rate\n"
;>>>> 							 "1) Serial channel B baud rate\n"
;>>>> 							 "2) CPU clock rate\n"
;>>>> 							 "3) DRAM size\n"
;>>>> 							 "4) Local bus DRAM base address\n"
;>>>> 							 "5) VME bus DRAM base address\n"
;>>>> 							 "6) Local bus JTAG base address\n"
;>>>> 							 "7) VME bus JTAG base address\n"
;>>>> 							 "8) Local bus Hydra control register address\n"
;>>>> 							 "9) VME bus Hydra control register address\n"
;>>>> 							 "A) Monitor relocation address\n"
;>>>> 							 "B) Daughter card attached\n"
;>>>> 							 "R) Return\n" );
	LDI	@CONST+21,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 			c40_printf( "\nSelection => " );
	LDI	@CONST+22,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 			i = c40_getchar();
	CALL	_c40_getchar
	STI	R0,*+FP(1)
;>>>> 			if( (i=='R') || (i=='r') || isdigit(i) )
	CMPI	82,R0
	BZ	LL14
	CMPI	114,R0
	BZ	LL14
	PUSH	R0
	CALL	_isdigit
	SUBI	1,SP
	CMPI	0,R0
	BZ	L68
LL14:
;>>>> 				return( i );
;>>>> 			else
	LDI	*+FP(1),R0
	B	EPI0_7
;>>>> 				switch( i )
;>>>> 					case 'A':
;>>>> 					case 'a':
L70:
;>>>> 						return( i );
;>>>> 					case 'B':
;>>>> 					case 'b':
	LDI	*+FP(1),R0
	B	EPI0_7
L72:
;>>>> 						return( i );
;>>>> 					default:
	LDI	*+FP(1),R0
	B	EPI0_7
L73:
;>>>> 						c40_printf( "\nInvalid selection.\n\n" );
	LDI	@CONST+14,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	B	L65
L68:
	LDI	*+FP(1),R0
	CMPI	65,R0
	BZ	L70
	CMPI	66,R0
	BZ	L72
	CMPI	97,R0
	BZ	L70
	CMPI	98,R0
	BZ	L72
	B	L73
EPI0_7:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
	.globl	_read_config
;>>>> 	int read_config( hydra_conf *config )
******************************************************
* FUNCTION DEF : _read_config
******************************************************
_read_config:
	PUSH	FP
	LDI	SP,FP
	ADDI	3,SP
;>>>> 		unsigned long chksum=0;
;>>>> 		int i;
;>>>> 		unsigned int temp;
	LDI	0,R0
	STI	R0,*+FP(1)
;>>>> 		config->baudA = read_eeprom( 0 ) & 0xFF;
	PUSH	R0
	CALL	_read_eeprom
	SUBI	1,SP
	AND	0ffh,R0
	LDI	*-FP(2),AR0
	STI	R0,*AR0
;>>>> 		crcupdate( config->baudA & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*AR0,R1
	AND	0ffh,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 		config->baudB = read_eeprom( 1 ) & 0xFF;
	LDI	1,R0
	PUSH	R0
	CALL	_read_eeprom
	SUBI	1,SP
	AND	0ffh,R0
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(1)
;>>>> 		crcupdate( config->baudB & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*+AR0(1),R1
	AND	0ffh,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 		config->dram_size = read_eeprom( 2 ) & 0xFF;
	LDI	2,R0
	PUSH	R0
	CALL	_read_eeprom
	SUBI	1,SP
	AND	0ffh,R0
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(2)
;>>>> 		crcupdate( config->dram_size & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*+AR0(2),R1
	AND	0ffh,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 		config->cpu_clock = read_eeprom( 3 ) & 0xFF;
	LDI	3,R0
	PUSH	R0
	CALL	_read_eeprom
	SUBI	1,SP
	AND	0ffh,R0
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(3)
;>>>> 	   crcupdate( config->cpu_clock & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*+AR0(3),R1
	AND	0ffh,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 		for( i=0, config->l_dram_base=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(9)
L75:
;>>>> 			temp = read_eeprom( 4+i );
	LDI	*+FP(2),R3
	ADDI	4,R3
	PUSH	R3
	CALL	_read_eeprom
	SUBI	1,SP
	STI	R0,*+FP(3)
;>>>> 			crcupdate( temp & 0xFF, &chksum );
	LDI	FP,R1
	ADDI	1,R1
	PUSH	R1
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 			config->l_dram_base |= (unsigned long) temp << i;
	LDI	*+FP(3),R0
	LSH	*+FP(2),R0
	LDI	*-FP(2),AR0
	OR	*+AR0(9),R0
	STI	R0,*+AR0(9)
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L75
;>>>> 		for( i=0, config->v_dram_base=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
	STI	R0,*+AR0(10)
L77:
;>>>> 			temp = read_eeprom( 8+i );
	LDI	*+FP(2),R3
	ADDI	8,R3
	PUSH	R3
	CALL	_read_eeprom
	SUBI	1,SP
	STI	R0,*+FP(3)
;>>>> 	      crcupdate( temp & 0xFF, &chksum );
	LDI	FP,R1
	ADDI	1,R1
	PUSH	R1
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 			config->v_dram_base |= (unsigned long) temp << i;
	LDI	*+FP(3),R0
	LSH	*+FP(2),R0
	LDI	*-FP(2),AR0
	OR	*+AR0(10),R0
	STI	R0,*+AR0(10)
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L77
;>>>> 		for( i=0, config->l_jtag_base=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
	STI	R0,*+AR0(11)
L79:
;>>>> 			temp = read_eeprom( 12+i );
	LDI	*+FP(2),R3
	ADDI	12,R3
	PUSH	R3
	CALL	_read_eeprom
	SUBI	1,SP
	STI	R0,*+FP(3)
;>>>> 	      crcupdate( temp & 0xFF, &chksum );
	LDI	FP,R1
	ADDI	1,R1
	PUSH	R1
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 			config->l_jtag_base |= (unsigned long) temp << i;
	LDI	*+FP(3),R0
	LSH	*+FP(2),R0
	LDI	*-FP(2),AR0
	OR	*+AR0(11),R0
	STI	R0,*+AR0(11)
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L79
;>>>> 		for( i=0, config->v_jtag_base=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
	STI	R0,*+AR0(12)
L81:
;>>>> 			temp = read_eeprom( 16+i );
	LDI	*+FP(2),R3
	ADDI	16,R3
	PUSH	R3
	CALL	_read_eeprom
	SUBI	1,SP
	STI	R0,*+FP(3)
;>>>> 			crcupdate( temp & 0xFF, &chksum );
	LDI	FP,R1
	ADDI	1,R1
	PUSH	R1
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 			config->v_jtag_base |= (unsigned long) temp << i;
	LDI	*+FP(3),R0
	LSH	*+FP(2),R0
	LDI	*-FP(2),AR0
	OR	*+AR0(12),R0
	STI	R0,*+AR0(12)
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L81
;>>>> 		for( i=0, config->l_cont_base=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
	STI	R0,*+AR0(13)
L83:
;>>>> 			temp = read_eeprom( 20+i );
	LDI	*+FP(2),R3
	ADDI	20,R3
	PUSH	R3
	CALL	_read_eeprom
	SUBI	1,SP
	STI	R0,*+FP(3)
;>>>> 	      crcupdate( temp & 0xFF, &chksum );
	LDI	FP,R1
	ADDI	1,R1
	PUSH	R1
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 			config->l_cont_base |= (unsigned long) temp << i;
	LDI	*+FP(3),R0
	LSH	*+FP(2),R0
	LDI	*-FP(2),AR0
	OR	*+AR0(13),R0
	STI	R0,*+AR0(13)
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L83
;>>>> 		for( i=0, config->v_cont_base=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
	STI	R0,*+AR0(14)
L85:
;>>>> 			temp = read_eeprom( 24+i );
	LDI	*+FP(2),R3
	ADDI	24,R3
	PUSH	R3
	CALL	_read_eeprom
	SUBI	1,SP
	STI	R0,*+FP(3)
;>>>> 	      crcupdate( temp & 0xFF, &chksum );
	LDI	FP,R1
	ADDI	1,R1
	PUSH	R1
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 			config->v_cont_base |= (unsigned long) temp << i;
	LDI	*+FP(3),R0
	LSH	*+FP(2),R0
	LDI	*-FP(2),AR0
	OR	*+AR0(14),R0
	STI	R0,*+AR0(14)
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L85
;>>>> 		for( i=0, config->mon_addr=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
	STI	R0,*+AR0(15)
L87:
;>>>> 			temp = read_eeprom( 28+i );
	LDI	*+FP(2),R3
	ADDI	28,R3
	PUSH	R3
	CALL	_read_eeprom
	SUBI	1,SP
	STI	R0,*+FP(3)
;>>>> 	      crcupdate( temp & 0xFF, &chksum );
	LDI	FP,R1
	ADDI	1,R1
	PUSH	R1
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 			config->mon_addr |= (unsigned long) temp << i;
	LDI	*+FP(3),R0
	LSH	*+FP(2),R0
	LDI	*-FP(2),AR0
	OR	*+AR0(15),R0
	STI	R0,*+AR0(15)
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L87
;>>>> 		config->daughter = read_eeprom( 32 ) & 0xFF;
	LDI	32,R1
	PUSH	R1
	CALL	_read_eeprom
	SUBI	1,SP
	AND	0ffh,R0
	LDI	*-FP(2),AR0
	STI	R0,*+AR0(16)
;>>>> 		for( i=0 ; i < 2 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
L89:
;>>>> 			temp = read_eeprom( 33+i );
	LDI	*+FP(2),R3
	ADDI	33,R3
	PUSH	R3
	CALL	_read_eeprom
	SUBI	1,SP
	STI	R0,*+FP(3)
;>>>> 	      crcupdate( temp & 0xFF, &chksum );
	LDI	FP,R1
	ADDI	1,R1
	PUSH	R1
	AND	0ffh,R0
	PUSH	R0
	CALL	_crcupdate
	SUBI	2,SP
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	2,R0
	BLT	L89
;>>>> 		if( chksum )
	LDI	*+FP(1),R1
	BZ	L91
;>>>> 			return( FAILURE );
;>>>> 		else
	LDI	0,R0
	B	EPI0_8
L91:
;>>>> 			return( SUCCESS );
	LDI	1,R0
EPI0_8:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	5,SP
***	B	R1	;BRANCH OCCURS
	.globl	_display_conf
;>>>> 	void display_conf( hydra_conf config )
;>>>> 		int i, tempA, tempB, Dtemp, temparry[4];
******************************************************
* FUNCTION DEF : _display_conf
******************************************************
_display_conf:
	PUSH	FP
	LDI	SP,FP
	ADDI	8,SP
;>>>> 		for( i=0, tempA=75 ; i < config.baudA ; i++, tempA *= 2 );
	LDI	0,R0
	STI	R0,*+FP(1)
	LDI	75,R1
	STI	R1,*+FP(2)
	CMPI	*-FP(18),R0
	BHS	L93
L92:
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	LDI	*+FP(2),R0
	ADDI	R0,R0,R1
	STI	R1,*+FP(2)
	LDI	*+FP(1),R0
	CMPI	*-FP(18),R0
	BLO	L92
L93:
;>>>> 		for( i=0, tempB=75 ; i < config.baudB ; i++, tempB *= 2 );
	LDI	0,R0
	STI	R0,*+FP(1)
	LDI	75,R2
	STI	R2,*+FP(3)
	CMPI	*-FP(17),R0
	BHS	L96
L94:
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	LDI	*+FP(3),R0
	ADDI	R0,R0,R1
	STI	R1,*+FP(3)
	LDI	*+FP(1),R0
	CMPI	*-FP(17),R0
	BLO	L94
;>>>> 		switch( config.cpu_clock )
;>>>> 			case 33 :
	B	L96
L97:
;>>>> 				c40_printf( "UART A baud rate = %d\n", tempA );
	LDI	*+FP(2),R0
	PUSH	R0
	LDI	@CONST+23,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 				c40_printf( "UART B baud rate = %d\n", tempB );
	LDI	*+FP(3),R0
	PUSH	R0
	LDI	@CONST+24,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 				break;
;>>>> 			case 40 :
;>>>> 			case 50 :
	B	L98
L100:
;>>>> 				c40_printf( "UART A baud rate = %d\n", tempA/=2 );
	LDI	*+FP(2),R0
	ASH	-1,R0
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	@CONST+23,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 				c40_printf( "UART B baud rate = %d\n", tempB/=2 );
	LDI	*+FP(3),R0
	ASH	-1,R0
	STI	R0,*+FP(3)
	PUSH	R0
	LDI	@CONST+24,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 				break;
;>>>> 			default :
	B	L98
L101:
;>>>> 				c40_printf( "Invalid value for CPU clock rate.\n" );
	LDI	@CONST+25,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 				break;
	B	L98
L96:
	LDI	*-FP(15),R1
	CMPI	33,R1
	BZ	L97
	CMPI	40,R1
	BZ	L100
	CMPI	50,R1
	BZ	L100
	B	L101
L98:
;>>>> 		c40_printf( "CPU clock rate = %d\n", config.cpu_clock );
	LDI	*-FP(15),R0
	PUSH	R0
	LDI	@CONST+26,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 		temparry[0] = config.sram1_size;
	LDI	*-FP(13),R0
	STI	R0,*+FP(5)
;>>>> 		temparry[1] = config.sram2_size;
	LDI	*-FP(12),R1
	STI	R1,*+FP(6)
;>>>> 		temparry[2] = config.sram3_size;
	LDI	*-FP(11),R2
	STI	R2,*+FP(7)
;>>>> 		temparry[3] = config.sram4_size;
	LDI	*-FP(10),R3
	STI	R3,*+FP(8)
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R3
	STI	R3,*+FP(1)
L102:
;>>>> 			c40_printf( "DSP %d SRAM size = ", i+1 );
	LDI	*+FP(1),R3
	ADDI	1,R3
	PUSH	R3
	LDI	@CONST+27,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 			switch( temparry[i] )
;>>>> 				case 0 :
	B	L104
L105:
;>>>> 					c40_printf( "%d Kbytes\n", 16 );
;>>>> 				case 1 :
	LDI	16,R0
	PUSH	R0
	LDI	@CONST+28,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
L106:
;>>>> 					c40_printf( "%d Kbytes\n", 32 );
;>>>> 				case 2 :
	LDI	32,R0
	PUSH	R0
	LDI	@CONST+28,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
L107:
;>>>> 					c40_printf( "%d Kbytes\n", 64 );
;>>>> 				case 3 :
	LDI	64,R0
	PUSH	R0
	LDI	@CONST+28,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
L108:
;>>>> 					c40_printf( "%d Kbytes\n", 256 );
	LDI	256,R0
	PUSH	R0
	LDI	@CONST+28,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	B	L109
L104:
	ADDI	*+FP(1),FP,AR0
	LDI	*+AR0(5),IR0
	LDI	@CONST+29,AR0
	CMPI	3,IR0
	LDIHI	4,IR0
	LDI	*+AR0(IR0),AR0
	B	AR0
LL19:
	.word	L105
	.word	L106
	.word	L107
	.word	L108
	.word	L109
L109:
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	4,R0
	BLT	L102
;>>>> 		for(i=0, Dtemp=4 ; i < config.dram_size ; i++, Dtemp*=4 );
	LDI	0,R0
	STI	R0,*+FP(1)
	LDI	4,R1
	STI	R1,*+FP(4)
	CMPI	*-FP(16),R0
	BHS	L111
L110:
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	LDI	*+FP(4),R1
	LSH	2,R1
	STI	R1,*+FP(4)
	CMPI	*-FP(16),R0
	BLO	L110
L111:
;>>>> 		c40_printf( "DRAM size = %d Mega bytes\n", Dtemp );
	PUSH	R1
	LDI	@CONST+30,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 		c40_printf( "Local bus DRAM base address = %x\n", config.l_dram_base );
	LDI	*-FP(9),R0
	PUSH	R0
	LDI	@CONST+31,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 		c40_printf( "VME bus DRAM base address = %x\n", config.v_dram_base );
	LDI	*-FP(8),R0
	PUSH	R0
	LDI	@CONST+32,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 		c40_printf( "Local bus JTAG base address = %x\n", config.l_jtag_base );
	LDI	*-FP(7),R0
	PUSH	R0
	LDI	@CONST+33,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 		c40_printf( "VME bus JTAG base address = %x\n", config.v_jtag_base );
	LDI	*-FP(6),R0
	PUSH	R0
	LDI	@CONST+34,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 		c40_printf( "Local bus Hydra control register address = %x\n", config.l_cont_base );
	LDI	*-FP(5),R0
	PUSH	R0
	LDI	@CONST+35,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 		c40_printf( "VME bus Hydra control register address = %x\n", config.v_cont_base );
	LDI	*-FP(4),R0
	PUSH	R0
	LDI	@CONST+36,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 		c40_printf( "Monitor relocation address = %x\n", config.mon_addr );
	LDI	*-FP(3),R0
	PUSH	R0
	LDI	@CONST+37,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
EPI0_9:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	10,SP
***	B	R1	;BRANCH OCCURS
	.globl	_write_config
;>>>> 	void write_config( hydra_conf config )
;>>>> 		int i;
******************************************************
* FUNCTION DEF : _write_config
******************************************************
_write_config:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
;>>>> 		write_eeprom( config.baudA, 0 );
	LDI	0,R0
	PUSH	R0
	LDI	*-FP(18),R1
	PUSH	R1
	CALL	_write_eeprom
	SUBI	2,SP
;>>>> 		write_eeprom( config.baudB, 1 );
	LDI	1,R0
	PUSH	R0
	LDI	*-FP(17),R1
	PUSH	R1
	CALL	_write_eeprom
	SUBI	2,SP
;>>>> 		write_eeprom( config.dram_size, 2 );
	LDI	2,R0
	PUSH	R0
	LDI	*-FP(16),R1
	PUSH	R1
	CALL	_write_eeprom
	SUBI	2,SP
;>>>> 		write_eeprom( config.cpu_clock, 3 );
	LDI	3,R0
	PUSH	R0
	LDI	*-FP(15),R1
	PUSH	R1
	CALL	_write_eeprom
	SUBI	2,SP
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
L112:
;>>>> 			write_eeprom( config.l_dram_base >> i, 4+i );
	LDI	*+FP(1),R3
	ADDI	4,R3
	PUSH	R3
	NEGI	*+FP(1),R0
	LDI	*-FP(9),R1
	LSH	R0,R1,R0
	PUSH	R0
	CALL	_write_eeprom
	SUBI	2,SP
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	4,R0
	BLT	L112
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
L114:
;>>>> 			write_eeprom( config.v_dram_base >> i, 8+i );
	LDI	*+FP(1),R3
	ADDI	8,R3
	PUSH	R3
	NEGI	*+FP(1),R0
	LDI	*-FP(8),R1
	LSH	R0,R1,R0
	PUSH	R0
	CALL	_write_eeprom
	SUBI	2,SP
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	4,R0
	BLT	L114
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
L116:
;>>>> 			write_eeprom( config.l_jtag_base >> i, 12+i );
	LDI	*+FP(1),R3
	ADDI	12,R3
	PUSH	R3
	NEGI	*+FP(1),R0
	LDI	*-FP(7),R1
	LSH	R0,R1,R0
	PUSH	R0
	CALL	_write_eeprom
	SUBI	2,SP
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	4,R0
	BLT	L116
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
L118:
;>>>> 			write_eeprom( config.v_jtag_base >> i, 16+i );
	LDI	*+FP(1),R3
	ADDI	16,R3
	PUSH	R3
	NEGI	*+FP(1),R0
	LDI	*-FP(6),R1
	LSH	R0,R1,R0
	PUSH	R0
	CALL	_write_eeprom
	SUBI	2,SP
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	4,R0
	BLT	L118
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
L120:
;>>>> 			write_eeprom( config.l_cont_base >> i, 20+i );
	LDI	*+FP(1),R3
	ADDI	20,R3
	PUSH	R3
	NEGI	*+FP(1),R0
	LDI	*-FP(5),R1
	LSH	R0,R1,R0
	PUSH	R0
	CALL	_write_eeprom
	SUBI	2,SP
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	4,R0
	BLT	L120
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
L122:
;>>>> 			write_eeprom( config.v_cont_base >> i, 24+i );
	LDI	*+FP(1),R3
	ADDI	24,R3
	PUSH	R3
	NEGI	*+FP(1),R0
	LDI	*-FP(4),R1
	LSH	R0,R1,R0
	PUSH	R0
	CALL	_write_eeprom
	SUBI	2,SP
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	4,R0
	BLT	L122
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
L124:
;>>>> 			write_eeprom( config.mon_addr >> i, 28+i );
	LDI	*+FP(1),R3
	ADDI	28,R3
	PUSH	R3
	NEGI	*+FP(1),R0
	LDI	*-FP(3),R1
	LSH	R0,R1,R0
	PUSH	R0
	CALL	_write_eeprom
	SUBI	2,SP
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	4,R0
	BLT	L124
;>>>> 		write_eeprom( config.daughter, 32 );
	LDI	32,R1
	PUSH	R1
	LDI	*-FP(2),R2
	PUSH	R2
	CALL	_write_eeprom
	SUBI	2,SP
;>>>> 		for( i=0 ; i < 2 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
L126:
;>>>> 			write_eeprom( config.checksum >> i, 33+i );
	LDI	*+FP(1),R3
	ADDI	33,R3
	PUSH	R3
	NEGI	*+FP(1),R0
	LDI	*-FP(14),R1
	LSH	R0,R1,R0
	PUSH	R0
	CALL	_write_eeprom
	SUBI	2,SP
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	2,R0
	BLT	L126
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
	.globl	_update_chksum
;>>>> 	void update_chksum( hydra_conf *config )
******************************************************
* FUNCTION DEF : _update_chksum
******************************************************
_update_chksum:
	PUSH	FP
	LDI	SP,FP
	ADDI	3,SP
;>>>> 		unsigned long chksum=0;
;>>>> 		int i;
;>>>> 		unsigned int temp;
	LDI	0,R0
	STI	R0,*+FP(1)
;>>>> 		crcupdate( config->baudA & 0xFF, &chksum );
	LDI	FP,R1
	ADDI	1,R1
	PUSH	R1
	LDI	*-FP(2),AR0
	LDI	*AR0,R2
	AND	0ffh,R2
	PUSH	R2
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 		crcupdate( config->baudB & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*-FP(2),AR0
	LDI	*+AR0(1),R1
	AND	0ffh,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 		crcupdate( config->dram_size & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*-FP(2),AR0
	LDI	*+AR0(2),R1
	AND	0ffh,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 		crcupdate( config->cpu_clock & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*-FP(2),AR0
	LDI	*+AR0(3),R1
	AND	0ffh,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
L127:
;>>>> 			crcupdate( (config->l_dram_base>>i) & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*-FP(2),AR0
	NEGI	*+FP(2),R1
	LDI	*+AR0(9),R2
	LSH	R1,R2,R1
	AND	0ffh,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L127
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
L129:
;>>>> 			crcupdate( (config->v_dram_base>>i) & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*-FP(2),AR0
	NEGI	*+FP(2),R1
	LDI	*+AR0(10),R2
	LSH	R1,R2,R1
	AND	0ffh,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L129
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
L131:
;>>>> 			crcupdate( (config->l_jtag_base>>i) & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*-FP(2),AR0
	NEGI	*+FP(2),R1
	LDI	*+AR0(11),R2
	LSH	R1,R2,R1
	AND	0ffh,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L131
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
L133:
;>>>> 			crcupdate( (config->v_jtag_base>>i) & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*-FP(2),AR0
	NEGI	*+FP(2),R1
	LDI	*+AR0(12),R2
	LSH	R1,R2,R1
	AND	0ffh,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L133
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
L135:
;>>>> 			crcupdate( (config->l_cont_base>>i) & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*-FP(2),AR0
	NEGI	*+FP(2),R1
	LDI	*+AR0(13),R2
	LSH	R1,R2,R1
	AND	0ffh,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L135
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
L137:
;>>>> 			crcupdate( (config->v_cont_base>>i) & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*-FP(2),AR0
	NEGI	*+FP(2),R1
	LDI	*+AR0(14),R2
	LSH	R1,R2,R1
	AND	0ffh,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L137
;>>>> 		for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(2)
L139:
;>>>> 			crcupdate( (config->mon_addr>>i) & 0xFF, &chksum );
	LDI	FP,R0
	ADDI	1,R0
	PUSH	R0
	LDI	*-FP(2),AR0
	NEGI	*+FP(2),R1
	LDI	*+AR0(15),R2
	LSH	R1,R2,R1
	AND	0ffh,R1
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L139
;>>>> 		crcupdate( config->daughter &= 0xFF, &chksum );
	LDI	FP,R1
	ADDI	1,R1
	PUSH	R1
	LDI	*-FP(2),AR0
	LDI	*+AR0(16),R2
	AND	0ffh,R2
	STI	R2,*+AR0(16)
	PUSH	R2
	CALL	_crcupdate
	SUBI	2,SP
;>>>> 		config->checksum = chksum;
	LDI	*-FP(2),AR0
	LDI	*+FP(1),R0
	STI	R0,*+AR0(4)
EPI0_11:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	5,SP
***	B	R1	;BRANCH OCCURS
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.text
SL0:	.byte	"Local DRAM base",0
SL1:	.byte	"VME DRAM base",0
SL2:	.byte	"Local JTAG base",0
SL3:	.byte	"VME JTAG base",0
SL4:	.byte	"Local bus Hydra control register base",0
SL5:	.byte	"VME bus Hydra controlregister base",0
SL6:	.byte	"Monitor relocation",0
SL7:	.byte	10,0
SL8:	.byte	"%s Address => ",0
SL9:	.byte	"Not a valid number : %s",10,0
SL10:	.byte	10,10,0
SL11:	.byte	"1) 33 MHz",10,"2) 40 MHz",10,"3) 50 MHz",10,0
SL12:	.byte	"Selection => ",0
SL13:	.byte	10,"Invalid selection.",10,10,0
SL14:	.byte	10,10,"Is a daughter card attached (y/n)?",0
SL15:	.byte	"Is daughter card attached (y/n)?",0
SL16:	.byte	"%c%c",0
SL17:	.byte	"Baud Rates :",10,0
SL18:	.byte	"  1) 75",10,"  2) 150",10,"  3) 300",10,"  4) 600",10,"  5)"
	.byte	" 1200",10,"  6) 2400",10,"  7) 4800",10,"  8) 9600",10,0
SL19:	.byte	"1) 4 Mbyte",10,"2) 16 Mbyte",10,"3) 64 Mbyte",10,0
SL20:	.byte	"0) Serial channel A baud rate",10,"1) Serial channel B baud"
	.byte	" rate",10,"2) CPU clock rate",10,"3) DRAM size",10,"4) Loca"
	.byte	"l bus DRAM base address",10,"5) VME bus DRAM base address",10
	.byte	"6) Local bus JTAG base address",10,"7) VME bus JTAG base ad"
	.byte	"dress",10,"8) Local bus Hydra control register address",10,"9"
	.byte	") VME bus Hydra control register address",10,"A) Monitor re"
	.byte	"location address",10,"B) Daughter card attached",10,"R) Ret"
	.byte	"urn",10,0
SL21:	.byte	10,"Selection => ",0
SL22:	.byte	"UART A baud rate = %d",10,0
SL23:	.byte	"UART B baud rate = %d",10,0
SL24:	.byte	"Invalid value for CPU clock rate.",10,0
SL25:	.byte	"CPU clock rate = %d",10,0
SL26:	.byte	"DSP %d SRAM size = ",0
SL27:	.byte	"%d Kbytes",10,0
SL28:	.byte	"DRAM size = %d Mega bytes",10,0
SL29:	.byte	"Local bus DRAM base address = %x",10,0
SL30:	.byte	"VME bus DRAM base address = %x",10,0
SL31:	.byte	"Local bus JTAG base address = %x",10,0
SL32:	.byte	"VME bus JTAG base address = %x",10,0
SL33:	.byte	"Local bus Hydra control register address = %x",10,0
SL34:	.byte	"VME bus Hydra control register address = %x",10,0
SL35:	.byte	"Monitor relocation address = %x",10,0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,38
	.sect	".cinit"
	.word	38,CONST
	.word 	SL0              ;0
	.word 	SL1              ;1
	.word 	SL2              ;2
	.word 	SL3              ;3
	.word 	SL4              ;4
	.word 	SL5              ;5
	.word 	SL6              ;6
	.word 	STATIC_1         ;7
	.word 	SL7              ;8
	.word 	SL8              ;9
	.word 	SL9              ;10
	.word 	SL10             ;11
	.word 	SL11             ;12
	.word 	SL12             ;13
	.word 	SL13             ;14
	.word 	SL14             ;15
	.word 	SL15             ;16
	.word 	SL16             ;17
	.word 	SL17             ;18
	.word 	SL18             ;19
	.word 	SL19             ;20
	.word 	SL20             ;21
	.word 	SL21             ;22
	.word 	SL22             ;23
	.word 	SL23             ;24
	.word 	SL24             ;25
	.word 	SL25             ;26
	.word 	SL26             ;27
	.word 	SL27             ;28
	.word 	LL19             ;29
	.word 	SL28             ;30
	.word 	SL29             ;31
	.word 	SL30             ;32
	.word 	SL31             ;33
	.word 	SL32             ;34
	.word 	SL33             ;35
	.word 	SL34             ;36
	.word 	SL35             ;37
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_crcupdate
	.end
