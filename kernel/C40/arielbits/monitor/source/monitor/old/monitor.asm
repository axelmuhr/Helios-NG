******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 -ic:\c40 monitor.c C:\TMP\monitor.if 
;	cg30 -v40 -o -n C:\TMP\monitor.if C:\TMP\monitor.asm C:\TMP\monitor.tmp 
FP	.set	AR3
	.globl	_atof
	.globl	_atoi
	.globl	_atol
	.globl	_strtod
	.globl	_strtol
	.globl	_strtoul
	.globl	_rand
	.globl	_srand
	.globl	_calloc
	.globl	_free
	.globl	_malloc
	.globl	_minit
	.globl	_realloc
	.globl	_abort
	.globl	_exit
	.globl	_atexit
	.globl	_bsearch
	.globl	_qsort
	.globl	_abs
	.globl	_labs
	.globl	_div
	.globl	_ldiv
	.globl	_getenv
	.globl	_memchr
	.globl	_memcmp
	.globl	_memcpy
	.globl	_memmove
	.globl	_memset
	.globl	_strcat
	.globl	_strchr
	.globl	_strcmp
	.globl	_strcpy
	.globl	_strcoll
	.globl	_strcspn
	.globl	_strerror
	.globl	_strlen
	.globl	_strncat
	.globl	_strncmp
	.globl	_strncpy
	.globl	_strpbrk
	.globl	_strrchr
	.globl	_strspn
	.globl	_strstr
	.globl	_strtok
	.globl	_strxfrm
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
	.globl	_call_set
	.globl	_breaks
	.globl	_brk_addrs
	.globl	_monitor
;>>>> 	void monitor( hydra_conf *config )
;>>>> 		static char in_line[LINE_LEN];
;>>>> 		static int i=0;
;>>>> 		char in_char, command[COMM_LEN], temp[TEMP_LEN];
;>>>> 		unsigned long parms[NUM_PARMS];
;>>>> 		int j, k, l, ok;
;>>>> 		while( 1 )
******************************************************
* FUNCTION DEF : _monitor
******************************************************
_monitor:
	PUSH	FP
	LDI	SP,FP
	ADDI	38,SP
	.bss	STATIC_1,50
	.sect	".cinit"
	.word	1,STATIC_2
	.word	0
	.bss	STATIC_2,1
	.text
L1:
;>>>> 			while( (in_char = c40_getchar()) != '\n' )
	CALL	_c40_getchar
	STI	R0,*+FP(1)
	CMPI	10,R0
	BZ	L4
L3:
;>>>> 				if( in_char == '\b' )
	LDI	*+FP(1),R0
	CMPI	8,R0
	BNZ	L5
;>>>> 					i--;
	BD	L6
	LDI	@STATIC_2,R1
	SUBI	1,R1
	STI	R1,@STATIC_2
;>>>> 				else
***	B	L6	;BRANCH OCCURS
L5:
;>>>> 					in_line[i++] = in_char;
	LDI	@STATIC_2,R1
	ADDI	1,R1
	STI	R1,@STATIC_2
	ADDI	@CONST+0,R1
	LDI	R1,AR0
	STI	R0,*-AR0(1)
L6:
;>>>> 				if( i >= LINE_LEN )
	LDI	@STATIC_2,R1
	CMPI	50,R1
	BLT	L7
;>>>> 					c40_printf( "\nLine too long.\n" );
	LDI	@CONST+1,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 					i=0;
	LDI	0,R0
	STI	R0,@STATIC_2
L7:
	CALL	_c40_getchar
	STI	R0,*+FP(1)
	CMPI	10,R0
	BNZ	L3
L4:
;>>>> 			in_line[i] = '\0';
	LDI	@CONST+0,R1
	ADDI	@STATIC_2,R1
	LDI	R1,AR0
	LDI	0,R1
	STI	R1,*AR0
;>>>> 			i=0;
	STI	R1,@STATIC_2
;>>>> 			for( j=0 ; iswhite( in_line[j] ) ; j++ ); /* Skip leading white space */
	STI	R1,*+FP(35)
	ADDI	@CONST+0,R1
	LDI	R1,AR0
	LDI	*AR0,R1
	PUSH	R1
	CALL	_iswhite
	SUBI	1,SP
	CMPI	0,R0
	BZ	L9
L8:
	LDI	*+FP(35),R0
	ADDI	1,R0
	STI	R0,*+FP(35)
	ADDI	@CONST+0,R0
	LDI	R0,AR0
	LDI	*AR0,R0
	PUSH	R0
	CALL	_iswhite
	SUBI	1,SP
	CMPI	0,R0
	BNZ	L8
L9:
;>>>> 			for( l=0 ; (!isspace(in_line[j])) && (l < COMM_LEN) ; j++, l++ )  /* extract command from line */
	BD	L11
	LDI	0,R0
	NOP
	STI	R0,*+FP(37)
;>>>> 				command[l] = (char)tolower( in_line[j] );
***	B	L11	;BRANCH OCCURS
L10:
	LDI	@CONST+0,R0
	ADDI	*+FP(35),R0
	LDI	R0,AR0
	LDI	*AR0,R0
	PUSH	R0
	CALL	_tolower
	SUBI	1,SP
	LDI	*+FP(37),R1
	ADDI	R1,FP,AR0
	STI	R0,*+AR0(2)
	LDI	*+FP(35),R0
	ADDI	1,R0
	STI	R0,*+FP(35)
	ADDI	1,R1
	STI	R1,*+FP(37)
L11:
	LDI	@CONST+0,R0
	ADDI	*+FP(35),R0
	LDI	R0,AR0
	LDI	*AR0,R0
	PUSH	R0
	CALL	_isspace
	SUBI	1,SP
	CMPI	0,R0
	BNZ	LL3
	LDI	*+FP(37),R0
	CMPI	10,R0
	BLT	L10
LL3:
;>>>> 			command[l] = '\0';
	LDI	*+FP(37),R0
	ADDI	R0,FP,AR0
	LDI	0,R1
	STI	R1,*+AR0(2)
;>>>> 			for( l=0 ; l < NUM_PARMS ; l++ )  /* 0xFFFFFFFF is end of parms list */
	STI	R1,*+FP(37)
L12:
;>>>> 				parms[l] = 0xFFFFFFFF;
	LDI	*+FP(37),R0
	ADDI	R0,FP,AR0
	LDI	-1,R1
	STI	R1,*+AR0(27)
	ADDI	1,R0
	STI	R0,*+FP(37)
	CMPI	8,R0
	BLT	L12
;>>>> 			l=0;
;>>>> 			while( (in_line[j] != '\0') && (l < NUM_PARMS) )   /* break will end this loop */
	BD	L15
	LDI	0,R0
	NOP
	STI	R0,*+FP(37)
;>>>> 				while( iswhite( in_line[j] ) )  /* Find next parameter */
***	B	L15	;BRANCH OCCURS
L14:
	LDI	@CONST+0,R0
	ADDI	*+FP(35),R0
	LDI	R0,AR0
	LDI	*AR0,R0
	PUSH	R0
	CALL	_iswhite
	SUBI	1,SP
	CMPI	0,R0
	BZ	L17
L16:
;>>>> 					j++;
	LDI	*+FP(35),R0
	ADDI	1,R0
	STI	R0,*+FP(35)
	ADDI	@CONST+0,R0
	LDI	R0,AR0
	LDI	*AR0,R0
	PUSH	R0
	CALL	_iswhite
	SUBI	1,SP
	CMPI	0,R0
	BNZ	L16
L17:
;>>>> 				for( k=0 ; (isalnum(in_line[j])) && (k < TEMP_LEN) ; k++, j++ )
	BD	L19
	LDI	0,R0
	NOP
	STI	R0,*+FP(36)
;>>>> 					temp[k] = in_line[j];
***	B	L19	;BRANCH OCCURS
L18:
	LDI	*+FP(36),R0
	ADDI	R0,FP,AR0
	LDI	@CONST+0,R1
	ADDI	*+FP(35),R1
	LDI	R1,AR1
	LDI	*AR1,R1
	STI	R1,*+AR0(12)
	ADDI	1,R0
	STI	R0,*+FP(36)
	LDI	*+FP(35),R1
	ADDI	1,R1
	STI	R1,*+FP(35)
L19:
	LDI	@CONST+0,R1
	ADDI	*+FP(35),R1
	LDI	R1,AR0
	LDI	*AR0,R1
	PUSH	R1
	CALL	_isalnum
	SUBI	1,SP
	CMPI	0,R0
	BZ	LL4
	LDI	*+FP(36),R0
	CMPI	15,R0
	BLT	L18
LL4:
;>>>> 				temp[k] = '\0';
	LDI	*+FP(36),R0
	ADDI	R0,FP,AR0
	LDI	0,R1
	STI	R1,*+AR0(12)
;>>>> 	         parms[l++] = (temp[k-1]=='h')||(temp[k-1]=='H') ?
;>>>> 										atox( temp, &ok ) : atod( temp, &ok );
	ADDI	R0,FP,AR0
	LDI	*+AR0(11),R2
	CMPI	104,R2
	BZ	LL6
	ADDI	R0,FP,AR0
	LDI	*+AR0(11),R2
	CMPI	72,R2
	BNZ	LL5
LL6:
	LDI	FP,R2
	ADDI	38,R2
	PUSH	R2
	SUBI	26,R2
	PUSH	R2
	CALL	_atox
	SUBI	2,SP
	B	LL7
LL5:
	LDI	FP,R2
	ADDI	38,R2
	PUSH	R2
	SUBI	26,R2
	PUSH	R2
	CALL	_atod
	SUBI	2,SP
LL7:
	LDI	*+FP(37),R1
	ADDI	1,R1
	STI	R1,*+FP(37)
	ADDI	R1,FP,AR0
	STI	R0,*+AR0(26)
;>>>> 				if( ok == FAILURE )
	LDI	*+FP(38),R0
	BNZ	L15
;>>>> 					c40_printf( "\nNot a valid number : %s\n", temp );
	LDI	FP,R2
	ADDI	12,R2
	PUSH	R2
	LDI	@CONST+2,R3
	PUSH	R3
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 					i=0;
	BD	L21
	LDI	0,R0
	STI	R0,@STATIC_2
;>>>> 					command[0] = '\0';
	STI	R0,*+FP(2)
;>>>> 					break;
***	B	L21	;BRANCH OCCURS
L15:
	LDI	@CONST+0,R0
	ADDI	*+FP(35),R0
	LDI	R0,AR0
	LDI	*AR0,R0
	BZ	L21
	LDI	*+FP(37),R0
	CMPI	8,R0
	BLT	L14
L21:
;>>>> 			if( !strcmp(command, "?") )
	LDI	@CONST+3,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L22
;>>>> 				help();
	CALL	_help
;>>>> 			else if( !strcmp(command, "c") )
	B	L23
L22:
	LDI	@CONST+4,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L24
;>>>> 				compare( parms, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	27,R1
	PUSH	R1
	CALL	_compare
	SUBI	2,SP
;>>>> 			else if( !strcmp(command, "d") )
	B	L23
L24:
	LDI	@CONST+5,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L26
;>>>> 				dump( parms, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	27,R1
	PUSH	R1
	CALL	_dump
	SUBI	2,SP
;>>>> 			else if( !strcmp(command, "f") )
	B	L23
L26:
	LDI	@CONST+6,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L28
;>>>> 				fill( parms );
	LDI	FP,R0
	ADDI	27,R0
	PUSH	R0
	CALL	_fill
	SUBI	1,SP
;>>>> 			else if( !strcmp(command, "e") )
	B	L23
L28:
	LDI	@CONST+7,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L30
;>>>> 				enter( parms, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	27,R1
	PUSH	R1
	CALL	_enter
	SUBI	2,SP
;>>>> 			else if( !strcmp(command, "cp") )
	B	L23
L30:
	LDI	@CONST+8,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L32
;>>>> 				copy( parms );
	LDI	FP,R0
	ADDI	27,R0
	PUSH	R0
	CALL	_copy
	SUBI	1,SP
;>>>> 			else if( !strcmp(command, "s") )
	B	L23
L32:
	LDI	@CONST+9,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L34
;>>>> 				search( parms, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	27,R1
	PUSH	R1
	CALL	_search
	SUBI	2,SP
;>>>> 			else if( !strcmp(command, "cf") )
	B	L23
L34:
	LDI	@CONST+10,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L36
;>>>> 				configure( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_configure
	SUBI	1,SP
;>>>> 			else if( !strcmp(command, "t") )
	B	L23
L36:
	LDI	@CONST+11,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L38
;>>>> 				test( *config, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	*-FP(2),AR0
	LDI	SP,AR1
	ADDI	17,SP
	LDI	*AR0++,R1
	RPTS	16
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_test
	SUBI	18,SP
;>>>> 			else if( !strcmp(command, "sbp") )
	B	L23
L38:
	LDI	@CONST+12,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L40
;>>>> 				set_brk( parms, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	27,R1
	PUSH	R1
	CALL	_set_brk
	SUBI	2,SP
;>>>> 			else if( !strcmp(command, "dbp") )
	B	L23
L40:
	LDI	@CONST+13,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L42
;>>>> 				del_brk( parms, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	27,R1
	PUSH	R1
	CALL	_del_brk
	SUBI	2,SP
;>>>> 			else if( !strcmp(command, "lbp") )
	B	L23
L42:
	LDI	@CONST+14,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L44
;>>>> 				list_brks( 't' );
	LDI	116,R0
	PUSH	R0
	CALL	_list_brks
	SUBI	1,SP
;>>>> 			else if( !strcmp(command, "rd") )
	B	L23
L44:
	LDI	@CONST+15,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L46
;>>>> 				reg_dump( call_set, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	@CONST+16,AR0
	LDI	SP,AR1
	ADDI	47,SP
	LDI	*AR0++,R1
	RPTS	46
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_reg_dump
	SUBI	48,SP
;>>>> 			else if( !strcmp(command, "st") )
	B	L23
L46:
	LDI	@CONST+17,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L48
;>>>> 				step( &call_set );
	LDI	@CONST+16,R0
	PUSH	R0
	CALL	_step
	SUBI	1,SP
;>>>> 			else if( !strcmp(command, "g") )
	B	L23
L48:
	LDI	@CONST+18,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L50
;>>>> 	      	if( parms[0] )
	LDI	*+FP(27),R0
	BZ	L51
;>>>> 					call_set.ret_add = parms[0];
	STI	R0,@_call_set+46
L51:
;>>>> 				go( &call_set );
	LDI	@CONST+16,R1
	PUSH	R1
	CALL	_go
	SUBI	1,SP
;>>>> 			else if( !strcmp( command, "" ) );
	B	L23
L50:
	LDI	@CONST+19,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BZ	L23
;>>>> 			else c40_printf( "\nUnknown command : %s\n", command );
	LDI	FP,R0
	ADDI	2,R0
	PUSH	R0
	LDI	@CONST+20,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
L23:
;>>>> 			c40_printf( "Ariel_Mon => " );
	LDI	@CONST+21,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	B	L1
	.globl	_iswhite
;>>>> 	int iswhite( char chr )
******************************************************
* FUNCTION DEF : _iswhite
******************************************************
_iswhite:
	PUSH	FP
	LDI	SP,FP
;>>>> 		if( chr == ' ' )
	LDI	*-FP(2),R0
	CMPI	32,R0
	BNZ	L55
;>>>> 			return( 1 );
	LDI	1,R0
	B	EPI0_2
L55:
;>>>> 		else if(chr == '\t')
	CMPI	9,R0
	BNZ	L56
;>>>> 				  return( 1 );
	LDI	1,R0
	B	EPI0_2
L56:
;>>>> 		else return( 0 );
	LDI	0,R0
EPI0_2:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	2,SP
***	B	R1	;BRANCH OCCURS
	.globl	_atox
;>>>> 	unsigned long atox( char *str, int *ok )
;>>>> 		unsigned long j, xnum;
;>>>> 		unsigned long num;
;>>>> 		int i;
******************************************************
* FUNCTION DEF : _atox
******************************************************
_atox:
	PUSH	FP
	LDI	SP,FP
	ADDI	4,SP
;>>>> 	   *ok = SUCCESS;
	LDI	*-FP(3),AR0
	LDI	1,R0
	STI	R0,*AR0
;>>>> 		for( i=0 ; str[i] != '\0' ; i++ );  /* Find end of string */
	LDI	0,R1
	STI	R1,*+FP(4)
	ADDI	*-FP(2),R1
	LDI	R1,AR1
	LDI	*AR1,R1
	BZ	L58
L57:
	LDI	*+FP(4),R0
	ADDI	1,R0
	STI	R0,*+FP(4)
	ADDI	*-FP(2),R0
	LDI	R0,AR0
	LDI	*AR0,R0
	BNZ	L57
L58:
;>>>> 		i -= 2;
	LDI	*+FP(4),R0
	SUBI	2,R0
	STI	R0,*+FP(4)
;>>>> 		for( j=0, num=0 ; i >= 0 ; i--, j++ )
	LDI	0,R1
	STI	R1,*+FP(1)
	STI	R1,*+FP(3)
	CMPI	0,R0
	BLT	L60
L59:
;>>>> 			if( !isxdigit( str[i] ) )
	LDI	*-FP(2),R0
	ADDI	*+FP(4),R0
	LDI	R0,AR0
	LDI	*AR0,R0
	PUSH	R0
	CALL	_isxdigit
	SUBI	1,SP
	CMPI	0,R0
	BNZ	L61
;>>>> 				*ok = FAILURE;
	BD	EPI0_3
	LDI	*-FP(3),AR0
	LDI	0,R0
	STI	R0,*AR0
;>>>> 				return( 0 );
***	B	EPI0_3	;BRANCH OCCURS
L61:
;>>>> 			xnum = isdigit(str[i]) ? str[i]-'0' : toupper(str[i])-'A'+10;
	LDI	*-FP(2),R0
	ADDI	*+FP(4),R0
	LDI	R0,AR0
	LDI	*AR0,R0
	PUSH	R0
	CALL	_isdigit
	SUBI	1,SP
	CMPI	0,R0
	BZ	LL12
	LDI	*-FP(2),R0
	ADDI	*+FP(4),R0
	BD	LL13
	LDI	R0,AR0
	LDI	*AR0,R3
	SUBI	48,R3
***	B	LL13	;BRANCH OCCURS
LL12:
	LDI	*-FP(2),R0
	ADDI	*+FP(4),R0
	LDI	R0,AR0
	LDI	*AR0,R0
	PUSH	R0
	CALL	_toupper
	SUBI	1,SP
	SUBI	55,R0
	LDI	R0,R3
LL13:
	STI	R3,*+FP(2)
;>>>> 			num += xnum * (unsigned long)pow( 16, j );
	FLOAT	*+FP(1),R0
	LDFLT	@CONST+22,R1
	LDFGE	0,R1
	ADDF	R1,R0
	PUSHF	R0
	LDF	1.6e1,R0
	PUSHF	R0
	CALL	_pow
	SUBI	2,SP
	FIX	R0,R1
	LDI	*+FP(2),R0
	CALL	MPY_I
	ADDI	*+FP(3),R0
	STI	R0,*+FP(3)
	LDI	*+FP(4),R1
	SUBI	1,R1
	STI	R1,*+FP(4)
	LDI	*+FP(1),R2
	ADDI	1,R2
	STI	R2,*+FP(1)
	CMPI	0,R1
	BGE	L59
L60:
;>>>> 		return( num );
	LDI	*+FP(3),R0
EPI0_3:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	6,SP
***	B	R1	;BRANCH OCCURS
	.globl	_atod
;>>>> 	unsigned long atod( char *str, int *ok )
;>>>> 		unsigned long j, dnum;
;>>>> 		unsigned long num;
;>>>> 		int i;
******************************************************
* FUNCTION DEF : _atod
******************************************************
_atod:
	PUSH	FP
	LDI	SP,FP
	ADDI	4,SP
;>>>> 		*ok = SUCCESS;
	LDI	*-FP(3),AR0
	LDI	1,R0
	STI	R0,*AR0
;>>>> 		for( i=0 ; str[i] != '\0' ; i++ );  /* Find end of string */
	LDI	0,R1
	STI	R1,*+FP(4)
	ADDI	*-FP(2),R1
	LDI	R1,AR1
	LDI	*AR1,R1
	BZ	L63
L62:
	LDI	*+FP(4),R0
	ADDI	1,R0
	STI	R0,*+FP(4)
	ADDI	*-FP(2),R0
	LDI	R0,AR0
	LDI	*AR0,R0
	BNZ	L62
L63:
;>>>> 		i -= 1;
	LDI	*+FP(4),R0
	SUBI	1,R0
	STI	R0,*+FP(4)
;>>>> 		for( j=0, num=0 ; i >= 0 ; i--, j++ )
	LDI	0,R1
	STI	R1,*+FP(1)
	STI	R1,*+FP(3)
	CMPI	0,R0
	BLT	L65
L64:
;>>>> 			if( !isdigit( str[i] ) )
	LDI	*-FP(2),R0
	ADDI	*+FP(4),R0
	LDI	R0,AR0
	LDI	*AR0,R0
	PUSH	R0
	CALL	_isdigit
	SUBI	1,SP
	CMPI	0,R0
	BNZ	L66
;>>>> 				*ok = FAILURE;
	BD	EPI0_4
	LDI	*-FP(3),AR0
	LDI	0,R0
	STI	R0,*AR0
;>>>> 				return( 0 );
***	B	EPI0_4	;BRANCH OCCURS
L66:
;>>>> 			dnum = str[i]-'0';
	LDI	*-FP(2),R0
	ADDI	*+FP(4),R0
	LDI	R0,AR0
	LDI	*AR0,R3
	SUBI	48,R3
	STI	R3,*+FP(2)
;>>>> 			num += dnum * (unsigned long)pow( 10.0, (double)j );
	FLOAT	*+FP(1),R0
	LDFLT	@CONST+22,R1
	LDFGE	0,R1
	ADDF	R1,R0
	PUSHF	R0
	LDF	1.0e1,R0
	PUSHF	R0
	CALL	_pow
	SUBI	2,SP
	FIX	R0,R1
	LDI	*+FP(2),R0
	CALL	MPY_I
	ADDI	*+FP(3),R0
	STI	R0,*+FP(3)
	LDI	*+FP(4),R1
	SUBI	1,R1
	STI	R1,*+FP(4)
	LDI	*+FP(1),R2
	ADDI	1,R2
	STI	R2,*+FP(1)
	CMPI	0,R1
	BGE	L64
L65:
;>>>> 		return( num );
	LDI	*+FP(3),R0
EPI0_4:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	6,SP
***	B	R1	;BRANCH OCCURS
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.text
SL0:	.byte	10,"Line too long.",10,0
SL1:	.byte	10,"Not a valid number : %s",10,0
SL2:	.byte	"?",0
SL3:	.byte	"c",0
SL4:	.byte	"d",0
SL5:	.byte	"f",0
SL6:	.byte	"e",0
SL7:	.byte	"cp",0
SL8:	.byte	"s",0
SL9:	.byte	"cf",0
SL10:	.byte	"t",0
SL11:	.byte	"sbp",0
SL12:	.byte	"dbp",0
SL13:	.byte	"lbp",0
SL14:	.byte	"rd",0
SL15:	.byte	"st",0
SL16:	.byte	"g",0
SL17:	.byte	0
SL18:	.byte	10,"Unknown command : %s",10,0
SL19:	.byte	"Ariel_Mon => ",0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,23
	.sect	".cinit"
	.word	23,CONST
	.word 	STATIC_1         ;0
	.word 	SL0              ;1
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
	.word 	SL11             ;12
	.word 	SL12             ;13
	.word 	SL13             ;14
	.word 	SL14             ;15
	.word 	_call_set        ;16
	.word 	SL15             ;17
	.word 	SL16             ;18
	.word 	SL17             ;19
	.word 	SL18             ;20
	.word 	SL19             ;21
	.float	4294967296.0     ;22
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	MPY_I
	.end
