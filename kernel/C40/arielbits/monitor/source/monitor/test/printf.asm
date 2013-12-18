******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 printf.c printf.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o printf.if printf.asm printf.tmp 
	.version	40
FP	.set		AR3
	.file	"printf.c"
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
	.file	"c:\c40\stdarg.h"
	.sym	_va_list,0,18,13,32
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
	.file	"printf.c"

	.sym	_c40_printf,_c40_printf,32,2,0
	.globl	_c40_printf

	.func	6
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
	.sym	_fmt,-2,18,9,32
	.sym	_ap,1,18,1,32
	.sym	_p,2,18,1,32
	.sym	_sval,3,18,1,32
	.sym	_cval,4,2,1,32
	.sym	_ival,5,5,1,32
	.sym	_fval,6,6,1,32
	.sym	_buf,7,50,1,512,,16
	.line	9
;>>>> 		va_start( ap, fmt );
	LDI	FP,R0
	SUBI	2,R0
	STI	R0,*+FP(1)
	.line	10
;>>>> 		for( p=fmt ; *p ; p++ )
	LDI	*-FP(2),R1
	STI	R1,*+FP(2)
	LDA	R1,AR0
	LDI	*AR0,R2
	BZ	EPI0_1
L1:
	.line	12
;>>>> 			if( *p != '%' )
	LDA	*+FP(2),AR0
	LDI	37,R0
	CMPI	R0,*AR0
	BZ	L3
	.line	14
;>>>> 				c40_putchar( *p );
	LDI	*AR0,R1
	PUSH	R1
	CALL	_c40_putchar
	SUBI	1,SP
	.line	15
;>>>> 				continue;
;>>>> 			switch( *++p )
;>>>> 				case 'd' :
	B	L4
L3:
	B	L5
L6:
	.line	20
;>>>> 					ival = va_arg( ap, int );
	SUBI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	LDA	R0,AR0
	LDI	*AR0,R1
	STI	R1,*+FP(5)
	.line	21
;>>>> 					ltoa( ival, buf, 10 );
	LDI	10,R2
	PUSH	R2
	LDI	FP,R3
	ADDI	7,R3
	PUSH	R3
	PUSH	R1
	CALL	_ltoa
	SUBI	3,SP
	.line	22
;>>>> 					putstr( buf );
	LDI	FP,R0
	ADDI	7,R0
	PUSH	R0
	CALL	_putstr
	SUBI	1,SP
	.line	23
;>>>> 					break;
;>>>> 				case 'f' :
	B	L4
L8:
	.line	25
;>>>> 					fval = va_arg( ap, float );
	SUBI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	LDA	R0,AR0
	LDF	*AR0,R1
	STF	R1,*+FP(6)
	.line	26
;>>>> 					ftoa( fval, buf );
	LDI	FP,R2
	ADDI	7,R2
	PUSH	R2
	PUSHF	R1
	CALL	_ftoa
	SUBI	2,SP
	.line	27
;>>>> 					putstr( buf );
	LDI	FP,R0
	ADDI	7,R0
	PUSH	R0
	CALL	_putstr
	SUBI	1,SP
	.line	28
;>>>> 					break;
;>>>> 				case 'x' :
	B	L4
L9:
	.line	30
;>>>> 					ival = va_arg( ap, int );
	SUBI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	LDA	R0,AR0
	LDI	*AR0,R1
	STI	R1,*+FP(5)
	.line	31
;>>>> 					xtoa( ival, buf );
	LDI	FP,R2
	ADDI	7,R2
	PUSH	R2
	PUSH	R1
	CALL	_xtoa
	SUBI	2,SP
	.line	32
;>>>> 					putstr( buf );
	LDI	FP,R0
	ADDI	7,R0
	PUSH	R0
	CALL	_putstr
	SUBI	1,SP
	.line	33
;>>>> 					break;
;>>>> 				case 'c' :
	B	L4
L10:
	.line	35
;>>>> 					cval = va_arg( ap, char );
	SUBI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	LDA	R0,AR0
	LDI	*AR0,R1
	STI	R1,*+FP(4)
	.line	36
;>>>> 					c40_putchar( cval );
	PUSH	R1
	CALL	_c40_putchar
	SUBI	1,SP
	.line	37
;>>>> 					break;
;>>>> 				case 's' :
	B	L4
L11:
	.line	39
;>>>> 					sval = va_arg( ap, char * );
	SUBI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	LDA	R0,AR0
	LDI	*AR0,R1
	STI	R1,*+FP(3)
	.line	40
;>>>> 					putstr( sval );
	PUSH	R1
	CALL	_putstr
	SUBI	1,SP
	.line	41
;>>>> 					break;
;>>>> 				case '\\' :
;>>>> 					switch( *++p )
;>>>> 						case 'n' :
	B	L4
L12:
	B	L13
L14:
	.line	46
;>>>> 							c40_putchar( (int) 10 );
	LDI	10,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	.line	47
;>>>> 							c40_putchar( (int) 13 );
	LDI	13,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	.line	48
;>>>> 							break;
;>>>> 						case '\\' :
	B	L15
L16:
	.line	50
;>>>> 							c40_putchar( '\\' );
	LDI	92,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	.line	51
;>>>> 							break;
;>>>> 						case 't' :
	B	L15
L17:
	.line	53
;>>>> 							c40_putchar( (int) 9 );
	LDI	9,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	.line	54
;>>>> 							break;
;>>>> 						case 'b' :
	B	L15
L18:
	.line	56
;>>>> 							c40_putchar( (int) 8 );
	LDI	8,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	.line	57
;>>>> 							break;
;>>>> 						case '"' :
	B	L15
L19:
	.line	59
;>>>> 							c40_putchar( '"' );
	LDI	34,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	.line	60
;>>>> 							break;
;>>>> 						case 'a' :
	B	L15
L20:
	.line	62
;>>>> 							c40_putchar( (int) 7 );
	LDI	7,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	.line	63
;>>>> 							break;
;>>>> 						default :
	B	L15
L21:
	.line	65
;>>>> 							c40_putchar( ' ' );
	LDI	32,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	B	L15
L13:
	.line	43
	LDA	*+FP(2),AR0
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
L15:
	.line	67
;>>>> 					break;
;>>>> 				default :
	B	L4
L22:
	.line	69
;>>>> 					c40_putchar( *p );
	LDA	*+FP(2),AR0
	LDI	*AR0,R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	.line	70
;>>>> 					break;
;>>>> 		va_end( ap );
	B	L4
L5:
	.line	17
	LDI	*++AR0,R1
	STI	AR0,*+FP(2)
	CMPI	92,R1
	BZ	L12
	CMPI	99,R1
	BZ	L10
	CMPI	100,R1
	BZ	L6
	CMPI	102,R1
	BZ	L8
	CMPI	115,R1
	BZ	L11
	CMPI	120,R1
	BZ	L9
	B	L22
L4:
	.line	10
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	LDA	R0,AR0
	LDI	*AR0,R1
	BNZ	L1
EPI0_1:
	.line	74
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	24,SP
	B	R1
	.endfunc	79,000000000H,22

	.sym	_putstr,_putstr,32,2,0
	.globl	_putstr

	.func	83
;>>>> 	void putstr( char *buf )
******************************************************
* FUNCTION DEF : _putstr
******************************************************
_putstr:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_buf,-2,18,9,32
	.sym	_i,1,4,1,32
	.line	2
;>>>> 		int i;
	.line	5
;>>>> 		for( i=0 ; buf[i] != '\0' ; i++ )
	STIK	0,*+FP(1)
	LDA	*-FP(2),AR0
	LDA	*+FP(1),IR0
	LDI	*+AR0(IR0),R0
	BZ	EPI0_2
L23:
	.line	6
;>>>> 			c40_putchar( buf[i] );
	LDA	*-FP(2),AR0
	LDA	*+FP(1),IR1
	LDI	*+AR0(IR1),R0
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	.line	5
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	LDA	*-FP(2),AR0
	LDA	*+FP(1),IR0
	LDI	*+AR0(IR0),R1
	BNZ	L23
EPI0_2:
	.line	7
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	89,000000000H,1

	.sym	_xtoa,_xtoa,32,2,0
	.globl	_xtoa

	.func	94
;>>>> 	void xtoa( unsigned long hexval, char *buf )
******************************************************
* FUNCTION DEF : _xtoa
******************************************************
_xtoa:
	PUSH	FP
	LDI	SP,FP
	ADDI	3,SP
	.sym	_hexval,-2,15,9,32
	.sym	_buf,-3,18,9,32
	.sym	_mask,1,15,1,32
	.sym	_i,2,15,1,32
	.sym	_temp,3,15,1,32
	.line	2
	.line	3
;>>>> 		unsigned long mask=0x0F0000000, i;
;>>>> 		unsigned long temp;
	LDI	@CONST+0,R0
	STI	R0,*+FP(1)
	.line	6
;>>>> 		for( i=0 ; i < 8 ; i++, mask >>= 4 )
	STIK	0,*+FP(2)
	CMPI	8,*+FP(2)
	BHS	L26
L25:
	.line	8
;>>>> 			temp = hexval & mask;
	LDI	*-FP(2),R0
	AND	*+FP(1),R0,R1
	STI	R1,*+FP(3)
	.line	9
;>>>> 			temp >>= (7-i)*4;
	LDI	*+FP(2),R2
	SUBI	7,R2
	LSH	2,R2
	LSH	R2,R1,R2
	STI	R2,*+FP(3)
	.line	10
;>>>> 			buf[i] = (temp < 10) ? 48+temp : 55+temp;
	CMPI	10,R2
	BHS	LL5
	ADDI	48,R2
	B	LL6
LL5:
	ADDI	55,R2
LL6:
	LDA	*-FP(3),AR0
	LDA	*+FP(2),IR1
	STI	R2,*+AR0(IR1)
	.line	6
	ADDI	1,*+FP(2),R1
	STI	R1,*+FP(2)
	LSH	-4,*+FP(1),R2
	STI	R2,*+FP(1)
	CMPI	8,R1
	BLO	L25
L26:
	.line	12
;>>>> 		buf[8] = '\0';
	LDA	*-FP(3),AR0
	STIK	0,*+AR0(8)
EPI0_3:
	.line	13
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	5,SP
	B	R1
	.endfunc	106,000000000H,3

	.sym	_ftoa,_ftoa,32,2,0
	.globl	_ftoa

	.func	112
;>>>> 	void ftoa( float fval, char *buf )
******************************************************
* FUNCTION DEF : _ftoa
******************************************************
_ftoa:
	PUSH	FP
	LDI	SP,FP
	ADDI	5,SP
	.sym	_fval,-2,6,9,32
	.sym	_buf,-3,18,9,32
	.sym	_index,1,4,1,32
	.sym	_count,2,4,1,32
	.sym	_exponent,3,4,1,32
	.sym	_temp1,4,7,1,32
	.sym	_temp2,5,7,1,32
	.line	2
	.line	3
;>>>> 		int index=0, count, exponent;
;>>>> 		double temp1, temp2;
	STIK	0,*+FP(1)
	.line	6
;>>>> 		if( fval < 0 )
	LDF	*-FP(2),R0
	BGE	L27
	.line	8
;>>>> 			buf[index++] = '-';
	LDA	*-FP(3),AR0
	ADDI	1,*+FP(1),IR0
	STI	IR0,*+FP(1)
	SUBI	1,AR0
	LDI	45,R1
	STI	R1,*+AR0(IR0)
	.line	9
;>>>> 			fval = -fval;
	NEGF	R0,R2
	STF	R2,*-FP(2)
L27:
	.line	12
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
	FIX	R0,R1
	NEGF	R0
	FIX	R0
	NEGI	R0
	LDILE	R0,R1
	STI	R1,*+FP(3)
	.line	14
;>>>> 		fval /= pow( (double)10.0, (double) exponent );
	FLOAT	R1,R0
	PUSHF	R0
	LDF	1.0e1,R0
	PUSHF	R0
	CALL	_pow
	SUBI	2,SP
	CALL	INV_F
	RND	R0
	MPYF	*-FP(2),R0
	STF	R0,*-FP(2)
	.line	17
;>>>> 		if( (fval > -1.0) && (fval < 1.0) && (fval != 0.0) )
	CMPF	-1.0,R0
	BLE	L28
	CMPF	1.0,R0
	BGE	L28
	CMPF	0,R0
	BZ	L28
	.line	19
;>>>> 			fval *= 10;
	MPYF	1.0e1,R0
	STF	R0,*-FP(2)
	.line	20
;>>>> 			exponent--;
	SUBI	1,*+FP(3),R1
	STI	R1,*+FP(3)
	.line	21
;>>>> 	      buf[index++] = '0' + (int)fval;
	FIX	R0,R2
	NEGF	R0,R3
	FIX	R3
	NEGI	R3
	LDILE	R3,R2
	LDA	*-FP(3),AR0
	ADDI	1,*+FP(1),IR1
	STI	IR1,*+FP(1)
	SUBI	1,AR0
	ADDI	48,R2
	STI	R2,*+AR0(IR1)
	.line	22
;>>>> 			fval -= (int)fval;
	FIX	R0,R2
	NEGF	R0,R3
	FIX	R3
	NEGI	R3
	LDILE	R3,R2
	FLOAT	R2
	SUBF	R2,R0,R2
	STF	R2,*-FP(2)
	.line	23
;>>>> 			buf[index++] = '.';
;>>>> 		else
	LDA	*-FP(3),AR0
	ADDI	1,*+FP(1),IR0
	STI	IR0,*+FP(1)
	SUBI	1,AR0
	LDI	46,R0
	STI	R0,*+AR0(IR0)
	B	L29
L28:
	.line	27
;>>>> 			buf[index++] = '0' + (int)fval;
	FIX	*-FP(2),R1
	NEGF	*-FP(2),R2
	FIX	R2
	NEGI	R2
	LDILE	R2,R1
	LDA	*-FP(3),AR0
	ADDI	1,*+FP(1),IR1
	STI	IR1,*+FP(1)
	SUBI	1,AR0
	ADDI	48,R1
	STI	R1,*+AR0(IR1)
	.line	28
;>>>> 			fval -= (int)fval;
	FIX	*-FP(2),R1
	NEGF	*-FP(2),R2
	FIX	R2
	NEGI	R2
	LDILE	R2,R1
	FLOAT	R1
	SUBRF	*-FP(2),R1
	STF	R1,*-FP(2)
	.line	29
;>>>> 			buf[index++] = '.';
	LDA	*-FP(3),AR0
	ADDI	1,*+FP(1),IR0
	STI	IR0,*+FP(1)
	SUBI	1,AR0
	LDI	46,R0
	STI	R0,*+AR0(IR0)
L29:
	.line	31
;>>>> 		for( count=0 ; count < 4 ; count++ )
	STIK	0,*+FP(2)
	CMPI	4,*+FP(2)
	BGE	L31
L30:
	.line	33
;>>>> 			fval *= 10;
	LDF	*-FP(2),R0
	MPYF	1.0e1,R0
	STF	R0,*-FP(2)
	.line	34
;>>>> 			buf[index++] = '0' + (int)fval;
	FIX	R0,R1
	NEGF	R0,R2
	FIX	R2
	NEGI	R2
	LDILE	R2,R1
	LDA	*-FP(3),AR0
	ADDI	1,*+FP(1),IR1
	STI	IR1,*+FP(1)
	SUBI	1,AR0
	ADDI	48,R1
	STI	R1,*+AR0(IR1)
	.line	35
;>>>> 	      fval -= (int)fval;
	FIX	R0,R1
	NEGF	R0,R2
	FIX	R2
	NEGI	R2
	LDILE	R2,R1
	FLOAT	R1
	SUBF	R1,R0,R1
	STF	R1,*-FP(2)
	.line	31
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	CMPI	4,R0
	BLT	L30
L31:
	.line	38
;>>>> 		if( exponent )
	LDI	*+FP(3),R0
	BZ	L32
	.line	40
;>>>> 			buf[index++] = 'x';
	LDA	*-FP(3),AR0
	ADDI	1,*+FP(1),IR0
	STI	IR0,*+FP(1)
	SUBI	1,AR0
	LDI	120,R1
	STI	R1,*+AR0(IR0)
	.line	41
;>>>> 			buf[index++] = '1';
	LDA	*-FP(3),AR0
	ADDI	1,*+FP(1),IR1
	STI	IR1,*+FP(1)
	SUBI	1,AR0
	LDI	49,R2
	STI	R2,*+AR0(IR1)
	.line	42
;>>>> 			buf[index++] = '0';
	LDA	*-FP(3),AR0
	ADDI	1,*+FP(1),IR0
	STI	IR0,*+FP(1)
	SUBI	1,AR0
	LDI	48,R3
	STI	R3,*+AR0(IR0)
	.line	43
;>>>> 			buf[index++] = 'e';
	LDA	*-FP(3),AR0
	ADDI	1,*+FP(1),IR1
	STI	IR1,*+FP(1)
	SUBI	1,AR0
	LDI	101,R9
	STI	R9,*+AR0(IR1)
	.line	44
;>>>> 			ltoa( exponent, buf+index );
;>>>> 		else
	LDI	*-FP(3),R10
	ADDI	*+FP(1),R10,R11
	PUSH	R11
	PUSH	R0
	CALL	_ltoa
	SUBI	2,SP
	B	EPI0_4
L32:
	.line	48
;>>>> 			buf[index] = '\0';
	LDA	*-FP(3),AR0
	LDA	*+FP(1),IR0
	STIK	0,*+AR0(IR0)
EPI0_4:
	.line	50
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	7,SP
	B	R1
	.endfunc	161,000000000H,5

	.sym	_send_host,_send_host,32,2,0
	.globl	_send_host

	.func	165
;>>>> 	void send_host( unsigned long data )
******************************************************
* FUNCTION DEF : _send_host
******************************************************
_send_host:
	PUSH	FP
	LDI	SP,FP
	.sym	_data,-2,15,9,32
	.line	2
EPI0_5:
	.line	3
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	167,000000000H,0
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
	.globl	INV_F
	.end
