******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 monitor.c monitor.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o monitor.if monitor.asm monitor.tmp 
	.version	40
FP	.set		AR3
	.file	"monitor.c"
	.file	"c:\c40\stdlib.h"
	.sym	_size_t,0,14,13,32
	.sym	_wchar_t,0,2,13,32

	.stag	__div_t,64
	.member	_quot,0,4,8,32
	.member	_rem,32,4,8,32
	.eos
	.sym	_div_t,0,8,13,64,__div_t
	.sym	_ldiv_t,0,8,13,64,__div_t
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
	.file	"c:\c40\string.h"
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
	.file	"monitor.c"
	.globl	_call_set
	.globl	_breaks
	.globl	_brk_addrs

	.sym	_monitor,_monitor,32,2,0
	.globl	_monitor

	.func	20
;>>>> 	void monitor( hydra_conf *config )
******************************************************
* FUNCTION DEF : _monitor
******************************************************
_monitor:
	PUSH	FP
	LDI	SP,FP
	ADDI	38,SP
	.sym	_config,-2,24,9,32,.fake1

	.sym	_in_line,STATIC_1,50,3,1600,,50
	.bss	STATIC_1,50

	.sect	".cinit"
	.word	1,STATIC_2
	.word	0

	.sym	_i,STATIC_2,4,3,32
	.bss	STATIC_2,1
	.text
	.sym	_in_char,1,2,1,32
	.sym	_command,2,50,1,320,,10
	.sym	_temp,12,50,1,480,,15
	.sym	_parms,27,63,1,256,,8
	.sym	_j,35,4,1,32
	.sym	_k,36,4,1,32
	.sym	_l,37,4,1,32
	.sym	_ok,38,4,1,32
	.line	2
;>>>> 		static char in_line[LINE_LEN];
;>>>> 		static int i=0;
;>>>> 		char in_char, command[COMM_LEN], temp[TEMP_LEN];
;>>>> 		unsigned long parms[NUM_PARMS];
;>>>> 		int j, k, l, ok;
	.line	10
;>>>> 		c40_printf( "Hydra_Mon => " );
;>>>> 		while( 1 )
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
L1:
	.line	14
;>>>> 			i = 0;
	STIK	0,@STATIC_2
	.line	15
;>>>> 			while( (c40_putchar(in_char = c40_getchar())) != '\n' )
	CALL	_c40_getchar
	STI	R0,*+FP(1)
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	CMPI	10,R0
	BZ	L4
L3:
	.line	17
;>>>> 				if( in_char == '\b' )
	CMPI	8,*+FP(1)
	BNZ	L5
	.line	18
;>>>> 					i--;
;>>>> 				else
	LDI	@STATIC_2,R0
	SUBI	1,R0,R1
	STI	R1,@STATIC_2
	B	L6
L5:
	.line	20
;>>>> 					in_line[i++] = in_char;
	LDA	@STATIC_2,IR0
	ADDI	1,IR0,IR1
	STI	IR1,@STATIC_2
	LDA	@CONST+1,AR0
	SUBI	1,AR0
	LDI	*+FP(1),R0
	STI	R0,*+AR0(IR1)
L6:
	.line	22
;>>>> 				if( i >= LINE_LEN )
	LDI	@STATIC_2,R0
	CMPI	50,R0
	BLT	L7
	.line	24
;>>>> 					c40_printf( "\nLine too long.\n" );
	LDI	@CONST+2,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	1,SP
	.line	25
;>>>> 					i=0;
	STIK	0,@STATIC_2
	.line	26
;>>>> 					break;
	B	L4
L7:
	.line	15
	CALL	_c40_getchar
	STI	R0,*+FP(1)
	PUSH	R0
	CALL	_c40_putchar
	SUBI	1,SP
	CMPI	10,R0
	BNZ	L3
L4:
	.line	29
;>>>> 			in_line[i] = '\0';
	LDA	@CONST+1,AR0
	LDA	@STATIC_2,IR0
	STIK	0,*+AR0(IR0)
	.line	33
;>>>> 			i=0;
	STIK	0,@STATIC_2
	.line	34
;>>>> 			for( j=0 ; iswhite( in_line[j] ) ; j++ ); /* Skip leading white space */
	STIK	0,*+FP(35)
	LDA	*+FP(35),IR1
	LDI	*+AR0(IR1),R0
	PUSH	R0
	CALL	_iswhite
	SUBI	1,SP
	CMPI	0,R0
	BZ	L9
L8:
	LDI	*+FP(35),R0
	ADDI	1,R0,R1
	STI	R1,*+FP(35)
	LDA	*+FP(35),IR0
	LDA	@CONST+1,AR0
	LDI	*+AR0(IR0),R0
	PUSH	R0
	CALL	_iswhite
	SUBI	1,SP
	CMPI	0,R0
	BNZ	L8
L9:
	.line	35
;>>>> 			for( l=0 ; (in_line[j] != '\0') 
;>>>> 			        && (!isspace(in_line[j])) 
;>>>> 			        && (l < COMM_LEN) ; j++, l++ )  /* extract command from line */
	STIK	0,*+FP(37)
	B	L11
L10:
	.line	38
;>>>> 				command[l] = (char)tolower( in_line[j] );
	LDA	*+FP(35),IR1
	LDA	@CONST+1,AR0
	LDI	*+AR0(IR1),R0
	PUSH	R0
	CALL	_tolower
	SUBI	1,SP
	LDA	*+FP(37),IR0
	ADDI	2,IR0
	STI	R0,*+FP(IR0)
	.line	37
	LDI	*+FP(35),R0
	ADDI	1,R0,R1
	STI	R1,*+FP(35)
	LDI	*+FP(37),R0
	ADDI	1,R0,R2
	STI	R2,*+FP(37)
L11:
	.line	35
	LDA	*+FP(35),IR1
	LDA	@CONST+1,AR0
	LDI	*+AR0(IR1),R0
	BZ	LL3
	LDI	*+AR0(IR1),R0
	PUSH	R0
	CALL	_isspace
	SUBI	1,SP
	CMPI	0,R0
	BNZ	LL3
	LDI	*+FP(37),R0
	CMPI	10,R0
	BLT	L10
LL3:
	.line	39
;>>>> 			command[l] = '\0';
	LDA	*+FP(37),IR0
	ADDI	2,IR0
	STIK	0,*+FP(IR0)
	.line	41
;>>>> 			for( l=0 ; l < NUM_PARMS ; l++ )  /* 0xFFFFFFFF is end of parms list */
	STIK	0,*+FP(37)
	LDI	*+FP(37),R0
	CMPI	8,R0
	BGE	L13
L12:
	.line	42
;>>>> 				parms[l] = 0xFFFFFFFF;
	LDA	*+FP(37),IR1
	ADDI	27,IR1
	STIK	-1,*+FP(IR1)
	.line	41
	LDI	*+FP(37),R0
	ADDI	1,R0,R1
	STI	R1,*+FP(37)
	CMPI	8,R1
	BLT	L12
L13:
	.line	44
;>>>> 			l=0;
;>>>> 			while( !isenter(in_line[j]) 
;>>>> 			       && (in_line[j] != '\0') 
;>>>> 			       && (l < NUM_PARMS) )   /* break will end this loop */
	STIK	0,*+FP(37)
	B	L15
L14:
	.line	49
;>>>> 				while( iswhite( in_line[j] ) )  /* Find next parameter */
	LDA	*+FP(35),IR0
	LDA	@CONST+1,AR0
	LDI	*+AR0(IR0),R0
	PUSH	R0
	CALL	_iswhite
	SUBI	1,SP
	CMPI	0,R0
	BZ	L17
L16:
	.line	50
;>>>> 					j++;
	LDI	*+FP(35),R0
	ADDI	1,R0,R1
	STI	R1,*+FP(35)
	.line	49
	LDA	*+FP(35),IR1
	LDA	@CONST+1,AR0
	LDI	*+AR0(IR1),R0
	PUSH	R0
	CALL	_iswhite
	SUBI	1,SP
	CMPI	0,R0
	BNZ	L16
L17:
	.line	52
;>>>> 				for( k=0 ; (isalnum(in_line[j])) && (k < TEMP_LEN) ; k++, j++ )
	STIK	0,*+FP(36)
	B	L19
L18:
	.line	53
;>>>> 					temp[k] = in_line[j];
	LDA	*+FP(35),IR0
	LDA	@CONST+1,AR0
	LDA	*+FP(36),IR1
	ADDI	12,IR1
	LDI	*+AR0(IR0),R0
	STI	R0,*+FP(IR1)
	.line	52
	LDI	*+FP(36),R0
	ADDI	1,R0,R1
	STI	R1,*+FP(36)
	LDI	*+FP(35),R0
	ADDI	1,R0,R2
	STI	R2,*+FP(35)
L19:
	LDA	*+FP(35),IR0
	LDA	@CONST+1,AR0
	LDI	*+AR0(IR0),R0
	PUSH	R0
	CALL	_isalnum
	SUBI	1,SP
	CMPI	0,R0
	BZ	LL4
	LDI	*+FP(36),R0
	CMPI	15,R0
	BLT	L18
LL4:
	.line	54
;>>>> 				temp[k] = '\0';
	LDA	*+FP(36),IR1
	ADDI	12,IR1
	STIK	0,*+FP(IR1)
	.line	56
;>>>> 				if( temp[0] == '\0' )
	LDI	*+FP(12),R0
	BZ	L15
	.line	57
;>>>> 					continue;
	.line	59
;>>>> 				if( (temp[k-1]=='h')||(temp[k-1]=='H') )
	LDA	*+FP(36),IR0
	ADDI	11,IR0
	LDI	104,R0
	CMPI	R0,*+FP(IR0)
	BZ	LL5
	LDA	*+FP(36),IR1
	ADDI	11,IR1
	LDI	72,R1
	CMPI	R1,*+FP(IR1)
	BNZ	L21
LL5:
	.line	61
;>>>> 					temp[k-1] = '\0';
	LDA	*+FP(36),IR0
	ADDI	11,IR0
	STIK	0,*+FP(IR0)
	.line	62
;>>>> 					parms[l++] = atox( temp, &ok );
;>>>> 				else
	LDI	FP,R1
	ADDI	38,R1
	PUSH	R1
	LDI	FP,R1
	ADDI	12,R1
	PUSH	R1
	CALL	_atox
	SUBI	2,SP
	LDA	*+FP(37),IR1
	ADDI	1,IR1,IR0
	STI	IR0,*+FP(37)
	ADDI	26,IR0
	STI	R0,*+FP(IR0)
	B	L22
L21:
	.line	65
;>>>> 					parms[l++] = atod( temp, &ok );
	LDI	FP,R2
	ADDI	38,R2
	PUSH	R2
	LDI	FP,R2
	ADDI	12,R2
	PUSH	R2
	CALL	_atod
	SUBI	2,SP
	LDA	*+FP(37),IR1
	ADDI	1,IR1,IR0
	STI	IR0,*+FP(37)
	ADDI	26,IR0
	STI	R0,*+FP(IR0)
L22:
	.line	67
;>>>> 				if( ok == FAILURE )
	LDI	*+FP(38),R0
	BNZ	L15
	.line	69
;>>>> 					c40_printf( "\nNot a valid number : %s\n", temp );
	LDI	FP,R1
	ADDI	12,R1
	PUSH	R1
	LDI	@CONST+3,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
	.line	70
;>>>> 					i=0;
	STIK	0,@STATIC_2
	.line	71
;>>>> 					command[0] = '\0';
	STIK	0,*+FP(2)
	.line	72
;>>>> 					break;
	B	L24
L15:
	.line	45
	LDA	*+FP(35),IR1
	LDA	@CONST+1,AR0
	LDI	*+AR0(IR1),R0
	PUSH	R0
	CALL	_isenter
	SUBI	1,SP
	CMPI	0,R0
	BNZ	L24
	LDA	*+FP(35),IR0
	LDA	@CONST+1,AR0
	LDI	*+AR0(IR0),R0
	BZ	L24
	LDI	*+FP(37),R0
	CMPI	8,R0
	BLT	L14
L24:
	.line	76
;>>>> 			if( !strcmp(command, "?") )
	LDI	@CONST+4,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L25
	.line	77
;>>>> 				help();
	CALL	_help
	B	L26
L25:
	.line	78
;>>>> 			else if( !strcmp(command, "c") )
	LDI	@CONST+5,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L27
	.line	79
;>>>> 				compare( parms, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	27,R1
	PUSH	R1
	CALL	_compare
	SUBI	2,SP
	B	L26
L27:
	.line	80
;>>>> 			else if( !strcmp(command, "d") )
	LDI	@CONST+6,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L28
	.line	81
;>>>> 				dump( parms, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	27,R1
	PUSH	R1
	CALL	_dump
	SUBI	2,SP
	B	L26
L28:
	.line	82
;>>>> 			else if( !strcmp(command, "f") )
	LDI	@CONST+7,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L29
	.line	83
;>>>> 				fill( parms );
	LDI	FP,R0
	ADDI	27,R0
	PUSH	R0
	CALL	_fill
	SUBI	1,SP
	B	L26
L29:
	.line	84
;>>>> 			else if( !strcmp(command, "e") )
	LDI	@CONST+8,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L30
	.line	85
;>>>> 				enter( parms, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	27,R1
	PUSH	R1
	CALL	_enter
	SUBI	2,SP
	B	L26
L30:
	.line	86
;>>>> 			else if( !strcmp(command, "cp") )
	LDI	@CONST+9,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L31
	.line	87
;>>>> 				copy( parms );
	LDI	FP,R0
	ADDI	27,R0
	PUSH	R0
	CALL	_copy
	SUBI	1,SP
	B	L26
L31:
	.line	88
;>>>> 			else if( !strcmp(command, "s") )
	LDI	@CONST+10,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L32
	.line	89
;>>>> 				search( parms, 't' );
	LDI	116,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	27,R1
	PUSH	R1
	CALL	_search
	SUBI	2,SP
	B	L26
L32:
	.line	90
;>>>> 			else if( !strcmp(command, "cf") )
	LDI	@CONST+11,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L33
	.line	91
;>>>> 				configure( config );
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_configure
	SUBI	1,SP
	B	L26
L33:
	.line	92
;>>>> 			else if( !strcmp(command, "t") )
	LDI	@CONST+12,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L34
	.line	93
;>>>> 				test( *config, 't' );
	LDI	116,R0
	PUSH	R0
	LDA	*-FP(2),AR0
	LDA	SP,AR1
	ADDI	16,SP
	LDI	*AR0++,R1
	RPTS	15
	STI	R1,*++AR1
    ||	LDI	*AR0++,R1
	CALL	_test
	SUBI	17,SP
	B	L26
L34:
	.line	111
;>>>> 			else if( !strcmp( command, "" ) );
	LDI	@CONST+13,R0
	PUSH	R0
	LDI	FP,R1
	ADDI	2,R1
	PUSH	R1
	CALL	_strcmp
	SUBI	2,SP
	CMPI	0,R0
	BZ	L26
	.line	112
;>>>> 			else c40_printf( "\nUnknown command : %s\n", command );
	LDI	FP,R0
	ADDI	2,R0
	PUSH	R0
	LDI	@CONST+14,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
L26:
	.line	114
;>>>> 			c40_printf( "Hydra_Mon => " );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	.line	116
	B	L1
	.endfunc	136,000000000H,38

	.sym	_iswhite,_iswhite,36,2,0
	.globl	_iswhite

	.func	142
;>>>> 	int iswhite( char chr )
******************************************************
* FUNCTION DEF : _iswhite
******************************************************
_iswhite:
	PUSH	FP
	LDI	SP,FP
	.sym	_chr,-2,2,9,32
	.line	5
	.line	6
;>>>> 		if( chr == ' ' )
	LDI	*-FP(2),R0
	CMPI	32,R0
	BNZ	L36
	.line	7
;>>>> 			return( 1 );
	LDI	1,R0
	B	EPI0_2
L36:
	.line	8
;>>>> 		else if(chr == '\t')
	CMPI	9,R0
	BNZ	L37
	.line	9
;>>>> 				  return( 1 );
	LDI	1,R0
	B	EPI0_2
L37:
	.line	10
;>>>> 		else return( 0 );
	LDI	0,R0
EPI0_2:
	.line	11
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	152,000000000H,0

	.sym	_isenter,_isenter,36,2,0
	.globl	_isenter

	.func	155
;>>>> 	int isenter( char chr )
******************************************************
* FUNCTION DEF : _isenter
******************************************************
_isenter:
	PUSH	FP
	LDI	SP,FP
	.sym	_chr,-2,2,9,32
	.line	5
	.line	6
;>>>> 		if( chr == 0x0A )
	LDI	*-FP(2),R0
	CMPI	10,R0
	BNZ	L38
	.line	7
;>>>> 			return( 1 );
	LDI	1,R0
	B	EPI0_3
L38:
	.line	8
;>>>> 		else if(chr == 0x0D)
	CMPI	13,R0
	BNZ	L39
	.line	9
;>>>> 			return( 1 );
	LDI	1,R0
	B	EPI0_3
L39:
	.line	10
;>>>> 		else return( 0 );
	LDI	0,R0
EPI0_3:
	.line	11
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	165,000000000H,0

	.sym	_atox,_atox,47,2,0
	.globl	_atox

	.func	169
;>>>> 	unsigned long atox( char *str, int *ok )
******************************************************
* FUNCTION DEF : _atox
******************************************************
_atox:
	PUSH	FP
	LDI	SP,FP
	ADDI	4,SP
	.sym	_str,-2,18,9,32
	.sym	_ok,-3,20,9,32
	.sym	_j,1,15,1,32
	.sym	_xnum,2,15,1,32
	.sym	_num,3,15,1,32
	.sym	_i,4,4,1,32
	.line	5
;>>>> 		unsigned long j, xnum;
;>>>> 		unsigned long num;
;>>>> 		int i;
	.line	10
;>>>> 		*ok = SUCCESS;
	LDA	*-FP(3),AR0
	STIK	1,*AR0
	.line	12
;>>>> 		for( i=0 ; str[i] != '\0' ; i++ );  /* Find end of string */
	STIK	0,*+FP(4)
	LDA	*-FP(2),AR1
	LDA	*+FP(4),IR1
	LDI	*+AR1(IR1),R0
	BZ	L41
L40:
	ADDI	1,*+FP(4),R0
	STI	R0,*+FP(4)
	LDA	*-FP(2),AR0
	LDA	*+FP(4),IR0
	LDI	*+AR0(IR0),R1
	BNZ	L40
L41:
	.line	14
;>>>> 		i -= 1;
	SUBI	1,*+FP(4),R0
	STI	R0,*+FP(4)
	.line	16
;>>>> 		for( j=0, num=0 ; i >= 0 ; i--, j++ )
	STIK	0,*+FP(1)
	STIK	0,*+FP(3)
	BLT	L43
L42:
	.line	18
;>>>> 			if( !isxdigit( str[i] ) )
	LDA	*-FP(2),AR0
	LDA	*+FP(4),IR1
	LDI	*+AR0(IR1),R0
	PUSH	R0
	CALL	_isxdigit
	SUBI	1,SP
	CMPI	0,R0
	BNZ	L44
	.line	20
;>>>> 				*ok = FAILURE;
	LDA	*-FP(3),AR0
	STIK	0,*AR0
	.line	21
;>>>> 				return( 0 );
	LDI	0,R0
	B	EPI0_4
L44:
	.line	24
;>>>> 			xnum = isdigit(str[i]) ? str[i]-'0' : toupper(str[i])-'A'+10;
	LDA	*-FP(2),AR0
	LDA	*+FP(4),IR0
	LDI	*+AR0(IR0),R0
	PUSH	R0
	CALL	_isdigit
	SUBI	1,SP
	CMPI	0,R0
	BZ	LL11
	LDA	*-FP(2),AR0
	LDA	*+FP(4),IR1
	LDI	*+AR0(IR1),R0
	SUBI	48,R0
	B	LL12
LL11:
	LDA	*-FP(2),AR0
	LDA	*+FP(4),IR0
	LDI	*+AR0(IR0),R0
	PUSH	R0
	CALL	_toupper
	SUBI	1,SP
	SUBI	55,R0
LL12:
	STI	R0,*+FP(2)
	.line	25
;>>>> 			num += xnum * (unsigned long)((unsigned long)0x1 << (j*4));
	LSH	2,*+FP(1),R1
	LDI	1,R2
	LSH	R1,R2,R1
	MPYI	R0,R1
	ADDI	*+FP(3),R1
	STI	R1,*+FP(3)
	.line	16
	SUBRI	*+FP(4),R2
	STI	R2,*+FP(4)
	ADDI	1,*+FP(1),R3
	STI	R3,*+FP(1)
	CMPI	0,R2
	BGE	L42
L43:
	.line	29
;>>>> 		return( num );
	LDI	*+FP(3),R0
EPI0_4:
	.line	30
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	6,SP
	B	R1
	.endfunc	198,000000000H,4

	.sym	_atod,_atod,47,2,0
	.globl	_atod

	.func	203
;>>>> 	unsigned long atod( char *str, int *ok )
******************************************************
* FUNCTION DEF : _atod
******************************************************
_atod:
	PUSH	FP
	LDI	SP,FP
	ADDI	4,SP
	.sym	_str,-2,18,9,32
	.sym	_ok,-3,20,9,32
	.sym	_j,1,15,1,32
	.sym	_dnum,2,15,1,32
	.sym	_num,3,15,1,32
	.sym	_i,4,4,1,32
	.line	5
;>>>> 		unsigned long j, dnum;
;>>>> 		unsigned long num;
;>>>> 		int i;
	.line	10
;>>>> 		*ok = SUCCESS;
	LDA	*-FP(3),AR0
	STIK	1,*AR0
	.line	12
;>>>> 		for( i=0 ; str[i] != '\0' ; i++ );  /* Find end of string */
	STIK	0,*+FP(4)
	LDA	*-FP(2),AR1
	LDA	*+FP(4),IR1
	LDI	*+AR1(IR1),R0
	BZ	L46
L45:
	ADDI	1,*+FP(4),R0
	STI	R0,*+FP(4)
	LDA	*-FP(2),AR0
	LDA	*+FP(4),IR0
	LDI	*+AR0(IR0),R1
	BNZ	L45
L46:
	.line	14
;>>>> 		i -= 1;
	SUBI	1,*+FP(4),R0
	STI	R0,*+FP(4)
	.line	16
;>>>> 		for( j=0, num=0 ; i >= 0 ; i--, j++ )
	STIK	0,*+FP(1)
	STIK	0,*+FP(3)
	BLT	L48
L47:
	.line	18
;>>>> 			if( !isdigit( str[i] ) )
	LDA	*-FP(2),AR0
	LDA	*+FP(4),IR1
	LDI	*+AR0(IR1),R0
	PUSH	R0
	CALL	_isdigit
	SUBI	1,SP
	CMPI	0,R0
	BNZ	L49
	.line	20
;>>>> 				*ok = FAILURE;
	LDA	*-FP(3),AR0
	STIK	0,*AR0
	.line	21
;>>>> 				return( 0 );
	LDI	0,R0
	B	EPI0_5
L49:
	.line	24
;>>>> 			dnum = str[i]-'0';
	LDA	*-FP(2),AR0
	LDA	*+FP(4),IR0
	LDI	*+AR0(IR0),R0
	SUBI	48,R0
	STI	R0,*+FP(2)
	.line	25
;>>>> 			num += dnum * (unsigned long)pow( 10.0, (double)j );
	FLOAT	*+FP(1),R1
	LDFLT	@CONST+15,R2
	LDFGE	0,R2
	ADDF	R2,R1
	PUSHF	R1
	LDF	1.0e1,R1
	PUSHF	R1
	CALL	_pow
	SUBI	2,SP
	FIX	R0
	MPYI	*+FP(2),R0
	ADDI	*+FP(3),R0
	STI	R0,*+FP(3)
	.line	16
	SUBI	1,*+FP(4),R1
	STI	R1,*+FP(4)
	ADDI	1,*+FP(1),R2
	STI	R2,*+FP(1)
	CMPI	0,R1
	BGE	L47
L48:
	.line	28
;>>>> 		return( num );
	LDI	*+FP(3),R0
EPI0_5:
	.line	29
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	6,SP
	B	R1
	.endfunc	231,000000000H,4
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.sect	".const"
SL0:	.byte	"Hydra_Mon => ",0
SL1:	.byte	10,"Line too long.",10,0
SL2:	.byte	10,"Not a valid number : %s",10,0
SL3:	.byte	"?",0
SL4:	.byte	"c",0
SL5:	.byte	"d",0
SL6:	.byte	"f",0
SL7:	.byte	"e",0
SL8:	.byte	"cp",0
SL9:	.byte	"s",0
SL10:	.byte	"cf",0
SL11:	.byte	"t",0
SL12:	.byte	0
SL13:	.byte	10,"Unknown command : %s",10,0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,16
	.sect	".cinit"
	.word	16,CONST
	.word 	SL0              ;0
	.word 	STATIC_1         ;1
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
	.float	4294967296.0     ;15
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_configure
	.end
