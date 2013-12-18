******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 -ic:\c40 printf.c C:\TMP\printf.if 
;	cg30 -v40 -o -n C:\TMP\printf.if C:\TMP\printf.asm C:\TMP\printf.tmp 
FP	.set	AR3
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
	.globl	_c40_printf
;>>>> 	void c40_printf( char *fmt, ... )
;>>>> 		va_list ap;
;>>>> 		char *p, *sval, cval;
;>>>> 		long ival;
;>>>> 		float fval;
;>>>> 		char buf[16];
******************************************************
* FUNCTION DEF : _c40_printf
******************************************************
_c40_printf:
	PUSH	FP
	LDI	SP,FP
	ADDI	22,SP
;>>>> 		va_start( ap, fmt );
	LDI	FP,R0
	SUBI	2,R0
	STI	R0,*+FP(1)
;>>>> 		for( p=fmt ; *p ; p++ )
	LDI	*-FP(2),R1
	STI	R1,*+FP(2)
	LDI	R1,AR0
	LDI	*AR0,R2
	BZ	EPI0_1
L1:
;>>>> 			if( *p != '%' )
	LDI	*+FP(2),AR0
	LDI	*AR0,R0
	CMPI	37,R0
	BZ	L5
;>>>> 				c40_putchar( *p );
	LDI	*AR0,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
;>>>> 				continue;
	B	L4
;>>>> 			switch( *++p )
;>>>> 				case 'd' :
L6:
;>>>> 					ival = va_arg( ap, int );
	LDI	*+FP(1),R0
	SUBI	1,R0
	STI	R0,*+FP(1)
	LDI	R0,AR0
	LDI	*AR0,R1
	STI	R1,*+FP(5)
;>>>> 					ltoa( ival, buf, 10 );
	LDI	10,R2
	PUSH	R2
	LDI	FP,R3
	ADDI	7,R3
	PUSH	R3
	PUSH	R1
	CALL	_ltoa
	SUBI	3,SP
;>>>> 					putstr( buf );
	LDI	FP,R0
	ADDI	7,R0
	PUSH	R0
	CALL	_putstr
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case 'f' :
	B	L4
L8:
;>>>> 					fval = va_arg( ap, float );
	LDI	*+FP(1),R0
	SUBI	1,R0
	STI	R0,*+FP(1)
	LDI	R0,AR0
	LDF	*AR0,R1
	STF	R1,*+FP(6)
;>>>> 					ftoa( fval, buf );
	LDI	FP,R2
	ADDI	7,R2
	PUSH	R2
	PUSHF	R1
	CALL	_ftoa
	SUBI	2,SP
;>>>> 					putstr( buf );
	LDI	FP,R0
	ADDI	7,R0
	PUSH	R0
	CALL	_putstr
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case 'x' :
	B	L4
L9:
;>>>> 					ival = va_arg( ap, int );
	LDI	*+FP(1),R0
	SUBI	1,R0
	STI	R0,*+FP(1)
	LDI	R0,AR0
	LDI	*AR0,R1
	STI	R1,*+FP(5)
;>>>> 					xtoa( ival, buf );
	LDI	FP,R2
	ADDI	7,R2
	PUSH	R2
	PUSH	R1
	CALL	_xtoa
	SUBI	2,SP
;>>>> 					putstr( buf );
	LDI	FP,R0
	ADDI	7,R0
	PUSH	R0
	CALL	_putstr
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case 'c' :
	B	L4
L10:
;>>>> 					cval = va_arg( ap, char );
	LDI	*+FP(1),R0
	SUBI	1,R0
	STI	R0,*+FP(1)
	LDI	R0,AR0
	LDI	*AR0,R1
	STI	R1,*+FP(4)
;>>>> 					c40_putchar( cval );
	PUSH	R1
	CALL	_c40_putchar
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case 's' :
	B	L4
L11:
;>>>> 					sval = va_arg( ap, char * );
	LDI	*+FP(1),R0
	SUBI	1,R0
	STI	R0,*+FP(1)
	LDI	R0,AR0
	LDI	*AR0,R1
	STI	R1,*+FP(3)
;>>>> 					putstr( sval );
	PUSH	R1
	CALL	_putstr
	SUBI	1,SP
;>>>> 					break;
;>>>> 				case '\\' :
	B	L4
;>>>> 					switch( *++p )
;>>>> 						case 'n' :
L14:
;>>>> 							c40_putchar( (int) 12 );
	LDI	12,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
;>>>> 							c40_putchar( (int) 13 );
	LDI	13,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
;>>>> 							break;
;>>>> 						case '\\' :
	B	L4
L16:
;>>>> 							c40_putchar( '\\' );
	LDI	92,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
;>>>> 							break;
;>>>> 						case 't' :
	B	L4
L17:
;>>>> 							c40_putchar( (int) 9 );
	LDI	9,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
;>>>> 							break;
;>>>> 						case 'b' :
	B	L4
L18:
;>>>> 							c40_putchar( (int) 8 );
	LDI	8,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
;>>>> 							break;
;>>>> 						case '"' :
	B	L4
L19:
;>>>> 							c40_putchar( '"' );
	LDI	34,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
;>>>> 							break;
;>>>> 						case 'a' :
	B	L4
L20:
;>>>> 							c40_putchar( (int) 7 );
	LDI	7,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
;>>>> 							break;
;>>>> 						default :
	B	L4
L21:
;>>>> 							c40_putchar( ' ' );
	LDI	32,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	B	L4
L13:
	LDI	*+FP(2),AR0
	LDI	*++AR0,R0
	STI	AR0,*+FP(2)
	CMPI	34,R0
	BZ	L19
	CMPI	92,R0
	BZ	L16
	CMPI	97,R0
	BZ	L20
	CMPI	98,R0
	BZ	L18
	CMPI	110,R0
	BZ	L14
	CMPI	116,R0
	BZ	L17
	B	L21
;>>>> 					break;
;>>>> 				default :
L22:
;>>>> 					c40_putchar( *p );
	LDI	*+FP(2),AR0
	LDI	*AR0,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
;>>>> 					break;
;>>>> 		va_end( ap );
	B	L4
L5:
	LDI	*++AR0,R0
	STI	AR0,*+FP(2)
	CMPI	92,R0
	BZ	L13
	CMPI	99,R0
	BZ	L10
	CMPI	100,R0
	BZ	L6
	CMPI	102,R0
	BZ	L8
	CMPI	115,R0
	BZ	L11
	CMPI	120,R0
	BZ	L9
	B	L22
L4:
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	LDI	R0,AR0
	LDI	*AR0,R1
	BNZ	L1
EPI0_1:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	24,SP
***	B	R1	;BRANCH OCCURS
	.globl	_putstr
;>>>> 	void putstr( char *buf )
;>>>> 		int i;
******************************************************
* FUNCTION DEF : _putstr
******************************************************
_putstr:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
;>>>> 		for( i=0 ; buf[i] != '\0' ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
	ADDI	*-FP(2),R0
	LDI	R0,AR0
	LDI	*AR0,R0
	BZ	EPI0_2
L23:
;>>>> 			c40_putchar( buf[i] );
	LDI	*-FP(2),R0
	ADDI	*+FP(1),R0
	LDI	R0,AR0
	LDI	*AR0,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	ADDI	*-FP(2),R0
	LDI	R0,AR0
	LDI	*AR0,R0
	BNZ	L23
EPI0_2:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
	.globl	_xtoa
;>>>> 	void xtoa( unsigned long hexval, char *buf )
******************************************************
* FUNCTION DEF : _xtoa
******************************************************
_xtoa:
	PUSH	FP
	LDI	SP,FP
	ADDI	3,SP
;>>>> 		unsigned long mask=0x0F0000000, i;
;>>>> 		unsigned long temp;
	LDI	@CONST+0,R0
	STI	R0,*+FP(1)
;>>>> 		for( i=0 ; i < 8 ; i++, mask >>= 4 )
	LDI	0,R1
	STI	R1,*+FP(2)
L25:
;>>>> 			temp = hexval & mask;
	LDI	*-FP(2),R0
	AND	*+FP(1),R0
	STI	R0,*+FP(3)
;>>>> 			temp >>= (7-i)*4;
	LDI	*+FP(2),R3
	SUBI	7,R3
	LSH	2,R3
	LSH	R3,R0,R1
	STI	R1,*+FP(3)
;>>>> 			buf[i] = (temp < 10) ? 48+temp : 55+temp;
	LDI	*-FP(3),R0
	ADDI	*+FP(2),R0
	LDI	R0,AR0
	CMPI	10,R1
	BHS	LL5
	ADDI	48,R1
	B	LL6
LL5:
	ADDI	55,R1
LL6:
	STI	R1,*AR0
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	LDI	*+FP(1),R1
	LSH	-4,R1
	STI	R1,*+FP(1)
	CMPI	8,R0
	BLO	L25
;>>>> 		buf[8] = '\0';
	LDI	*-FP(3),AR0
	LDI	0,R2
	STI	R2,*+AR0(8)
EPI0_3:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	5,SP
***	B	R1	;BRANCH OCCURS
	.globl	_ftoa
;>>>> 	void ftoa( float fval, char *buf )
******************************************************
* FUNCTION DEF : _ftoa
******************************************************
_ftoa:
	PUSH	FP
	LDI	SP,FP
	ADDI	5,SP
;>>>> 		int index=0, count, exponent;
;>>>> 		double temp1, temp2;
	LDI	0,R0
	STI	R0,*+FP(1)
;>>>> 		if( fval < 0 )
	LDF	*-FP(2),R1
	BGE	L26
;>>>> 			buf[index++] = '-';
	ADDI	1,R0
	STI	R0,*+FP(1)
	ADDI	*-FP(3),R0
	LDI	R0,AR0
	LDI	45,R0
	STI	R0,*-AR0(1)
;>>>> 			fval = -fval;
	NEGF	R1,R2
	STF	R2,*-FP(2)
L26:
;>>>> 		exponent = fval!=0.0?log10( fval ):0;
	LDF	*-FP(2),R0
	BZ	LL9
	PUSHF	R0
	CALL	_log10
	SUBI	1,SP
	B	LL10
LL9:
	LDF	0.0,R0
LL10:
	FIX	R0
	STI	R0,*+FP(3)
;>>>> 		fval /= pow( (double)10.0, (double) exponent );
	FLOAT	R0,R1
	PUSHF	R1
	LDF	1.0e1,R1
	PUSHF	R1
	CALL	_pow
	SUBI	2,SP
	LDF	R0,R1
	LDF	*-FP(2),R0
	CALL	DIV_F
	STF	R0,*-FP(2)
;>>>> 		if( (fval > -1.0) && (fval < 1.0) && (fval != 0.0) )
	CMPF	-1.0,R0
	BLE	L27
	CMPF	1.0,R0
	BGE	L27
	CMPF	0,R0
	BZ	L27
;>>>> 			fval *= 10;
	MPYF	1.0e1,R0
	STF	R0,*-FP(2)
;>>>> 			exponent--;
	LDI	*+FP(3),R1
	SUBI	1,R1
	STI	R1,*+FP(3)
;>>>> 	      buf[index++] = '0' + (int)fval;
	LDI	*+FP(1),R2
	ADDI	1,R2
	STI	R2,*+FP(1)
	ADDI	*-FP(3),R2
	LDI	R2,AR0
	FIX	R0,R2
	ADDI	48,R2
	STI	R2,*-AR0(1)
;>>>> 			fval -= (int)fval;
	FIX	R0,R2
	FLOAT	R2
	SUBF	R2,R0,R3
	STF	R3,*-FP(2)
;>>>> 			buf[index++] = '.';
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	ADDI	*-FP(3),R0
	BD	L28
	LDI	R0,AR0
	LDI	46,R0
	STI	R0,*-AR0(1)
;>>>> 		else
***	B	L28	;BRANCH OCCURS
L27:
;>>>> 			buf[index++] = '0' + (int)fval;
	LDI	*+FP(1),R1
	ADDI	1,R1
	STI	R1,*+FP(1)
	ADDI	*-FP(3),R1
	LDI	R1,AR0
	FIX	R0,R1
	ADDI	48,R1
	STI	R1,*-AR0(1)
;>>>> 			fval -= (int)fval;
	FIX	R0,R1
	FLOAT	R1
	SUBF	R1,R0,R3
	STF	R3,*-FP(2)
;>>>> 			buf[index++] = '.';
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	ADDI	*-FP(3),R0
	LDI	R0,AR0
	LDI	46,R0
	STI	R0,*-AR0(1)
L28:
;>>>> 		for( count=0 ; count < 4 ; count++ )
	LDI	0,R1
	STI	R1,*+FP(2)
L29:
;>>>> 			fval *= 10;
	LDF	*-FP(2),R0
	MPYF	1.0e1,R0
	STF	R0,*-FP(2)
;>>>> 			buf[index++] = '0' + (int)fval;
	LDI	*+FP(1),R1
	ADDI	1,R1
	STI	R1,*+FP(1)
	ADDI	*-FP(3),R1
	LDI	R1,AR0
	FIX	R0,R1
	ADDI	48,R1
	STI	R1,*-AR0(1)
;>>>> 	      fval -= (int)fval;
	FIX	R0,R1
	FLOAT	R1
	SUBF	R1,R0,R3
	STF	R3,*-FP(2)
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L29
;>>>> 		if( exponent )
	LDI	*+FP(3),R1
	BZ	L31
;>>>> 			buf[index++] = 'x';
	LDI	*+FP(1),R2
	ADDI	1,R2
	STI	R2,*+FP(1)
	ADDI	*-FP(3),R2
	LDI	R2,AR0
	LDI	120,R2
	STI	R2,*-AR0(1)
;>>>> 			buf[index++] = '1';
	LDI	*+FP(1),R3
	ADDI	1,R3
	STI	R3,*+FP(1)
	ADDI	*-FP(3),R3
	LDI	R3,AR0
	LDI	49,R3
	STI	R3,*-AR0(1)
;>>>> 			buf[index++] = '0';
	LDI	*+FP(1),R3
	ADDI	1,R3
	STI	R3,*+FP(1)
	ADDI	*-FP(3),R3
	LDI	R3,AR0
	LDI	48,R3
	STI	R3,*-AR0(1)
;>>>> 			buf[index++] = 'e';
	LDI	*+FP(1),R3
	ADDI	1,R3
	STI	R3,*+FP(1)
	ADDI	*-FP(3),R3
	LDI	R3,AR0
	LDI	101,R3
	STI	R3,*-AR0(1)
;>>>> 			ltoa( exponent, buf+index );
	LDI	*-FP(3),R3
	ADDI	*+FP(1),R3
	PUSH	R3
	PUSH	R1
	CALL	_ltoa
	SUBI	2,SP
;>>>> 		else
	B	EPI0_4
L31:
;>>>> 			buf[index] = '\0';
	LDI	*-FP(3),R2
	ADDI	*+FP(1),R2
	LDI	R2,AR0
	LDI	0,R2
	STI	R2,*AR0
EPI0_4:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	7,SP
***	B	R1	;BRANCH OCCURS
	.globl	_send_host
;>>>> 	void send_host( unsigned long data )
******************************************************
* FUNCTION DEF : _send_host
******************************************************
_send_host:
	PUSH	FP
	LDI	SP,FP
EPI0_5:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	2,SP
***	B	R1	;BRANCH OCCURS
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,1
	.sect	".cinit"
	.word	1,CONST
	.word 	-268435456       ;0
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_ltoa
	.globl	DIV_F
	.end
