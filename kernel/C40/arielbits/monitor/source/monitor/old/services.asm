******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 -ic:\c40 services.c C:\TMP\services.if 
;	cg30 -v40 -o -n C:\TMP\services.if C:\TMP\services.asm C:\TMP\services.tmp 
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
	.globl	_compare
;>>>> 	void compare( unsigned long parms[], char out_where )
******************************************************
* FUNCTION DEF : _compare
******************************************************
_compare:
	PUSH	FP
	LDI	SP,FP
;>>>> 		while( parms[2]-- )
	LDI	*-FP(2),AR0
	LDI	*+AR0(2),R0
	SUBI	1,R0
	STI	R0,*+AR0(2)
	ADDI	1,R0
	BZ	L8
L1:
;>>>> 			if( *((unsigned long *)parms[0]++) != *((unsigned long *)parms[1]++) )
	LDI	*-FP(2),AR0
	LDI	*+AR0(1),AR1
	LDI	*AR1++,R0
	STI	AR1,*+AR0(1)
	LDI	*AR0,AR1
	CMPI	R0,*AR1++
	STI	AR1,*AR0
	BZ	L3
;>>>> 				switch( out_where )
;>>>> 					case 't' :
	B	L4
L5:
;>>>> 						c40_printf( "Compare fault at address [%x].\n", --parms[0] );
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	SUBI	1,R0
	STI	R0,*AR0
	PUSH	R0
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 						return;
;>>>> 					default :
	B	EPI0_1
L6:
;>>>> 						send_host( FAILURE );
	LDI	0,R0
	PUSH	R0
	CALL	_send_host
	SUBI	1,SP
;>>>> 						return;
	B	EPI0_1
L4:
	LDI	*-FP(3),R0
	CMPI	116,R0
	BZ	L5
	B	L6
L3:
	LDI	*-FP(2),AR0
	LDI	*+AR0(2),R0
	SUBI	1,R0
	STI	R0,*+AR0(2)
	ADDI	1,R0
	BNZ	L1
;>>>> 	   switch( out_where )
;>>>> 			case 't' :
	B	L8
L9:
;>>>> 				c40_printf( "Compare successfull.\n" );
	LDI	@CONST+1,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 				return;
;>>>> 			default :
	B	EPI0_1
L10:
;>>>> 				send_host( SUCCESS );
	LDI	1,R0
	PUSH	R0
	CALL	_send_host
	SUBI	1,SP
;>>>> 				return;
	B	EPI0_1
L8:
	LDI	*-FP(3),R0
	CMPI	116,R0
	BZ	L9
	B	L10
EPI0_1:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	2,SP
***	B	R1	;BRANCH OCCURS
	.globl	_dump
;>>>> 	void dump( unsigned long parms[], char out_where )
;>>>> 		int i;
******************************************************
* FUNCTION DEF : _dump
******************************************************
_dump:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
;>>>> 		while( parms[1] )
	LDI	*-FP(2),AR0
	LDI	*+AR0(1),R0
	BZ	L13
L12:
;>>>> 			c40_printf( "\n[%x]   ", parms[0] );
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	PUSH	R0
	LDI	@CONST+2,R0
	PUSH	R0
	CALL	_c40_printf
	BD	L16
	SUBI	2,SP
;>>>> 			for( i=0 ; i < 4 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
;>>>> 	         switch( out_where )
;>>>> 					case 't' :
***	B	L16	;BRANCH OCCURS
L17:
;>>>> 						c40_printf( "%x  ", *((unsigned long *) parms[0]++) );
	LDI	*-FP(2),AR0
	LDI	*AR0,AR1
	LDI	*AR1++,R0
	STI	AR1,*AR0
	PUSH	R0
	LDI	@CONST+3,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 						break;
;>>>> 					default :
	B	L18
L19:
;>>>> 						send_host( *((unsigned long *) parms[0]++) );
	LDI	*-FP(2),AR0
	LDI	*AR0,AR1
	LDI	*AR1++,R0
	STI	AR1,*AR0
	PUSH	R0
	CALL	_send_host
	SUBI	1,SP
	B	L18
L16:
	LDI	*-FP(3),R0
	CMPI	116,R0
	BZ	L17
	B	L19
L18:
;>>>> 				if( !(--parms[1]) )
	LDI	*-FP(2),AR0
	LDI	*+AR0(1),R0
	SUBI	1,R0
	STI	R0,*+AR0(1)
	BZ	L15
;>>>> 					break;
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	4,R0
	BLT	L16
L15:
	LDI	*+AR0(1),R0
	BNZ	L12
L13:
;>>>> 		c40_printf( "\n\n" );
	LDI	@CONST+4,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
EPI0_2:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
	.globl	_fill
;>>>> 	void fill( unsigned long parms[] )
******************************************************
* FUNCTION DEF : _fill
******************************************************
_fill:
	PUSH	FP
	LDI	SP,FP
;>>>> 		while( parms[1]-- )
	LDI	*-FP(2),AR0
	LDI	*+AR0(1),R0
	SUBI	1,R0
	STI	R0,*+AR0(1)
	ADDI	1,R0
	BZ	EPI0_3
L21:
;>>>> 			*((unsigned long *) parms[0]++) = parms[2];
	LDI	*-FP(2),AR0
	LDI	*AR0,AR1
	LDI	*+AR0(2),R0
	STI	R0,*AR1++
	STI	AR1,*AR0
	LDI	*+AR0(1),R0
	SUBI	1,R0
	STI	R0,*+AR0(1)
	ADDI	1,R0
	BNZ	L21
EPI0_3:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	2,SP
***	B	R1	;BRANCH OCCURS
	.globl	_enter
;>>>> 	void enter( unsigned long parms[], char out_where )
;>>>> 		char inval[9];
;>>>> 		int i, j, ok;
;>>>> 		unsigned long xval;
;>>>> 		while( 1 )
******************************************************
* FUNCTION DEF : _enter
******************************************************
_enter:
	PUSH	FP
	LDI	SP,FP
	ADDI	13,SP
L23:
;>>>> 			c40_printf( "[%x]  %x : ", parms[0], *((unsigned long *)parms[0]) );
	LDI	*-FP(2),AR0
	LDI	*AR0,AR1
	LDI	*AR1,R0
	PUSH	R0
	LDI	*AR0,R0
	PUSH	R0
	LDI	@CONST+5,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	3,SP
;>>>> 			j=0;
	LDI	0,R0
	STI	R0,*+FP(11)
L25:
;>>>> 			while( (j < 8) && ((inval[j++]=c40_getchar())!='\n') );
	LDI	*+FP(11),R0
	CMPI	8,R0
	BGE	LL6
	CALL	_c40_getchar
	LDI	*+FP(11),R1
	ADDI	1,R1
	STI	R1,*+FP(11)
	ADDI	R1,FP,AR0
	STI	R0,*AR0
	CMPI	10,R0
	BNZ	L25
LL6:
;>>>> 			if( inval[0] == '\n' )
	LDI	*+FP(1),R0
	CMPI	10,R0
	BZ	L24
;>>>> 				break;
;>>>> 			inval[--j] = '\0';
	LDI	*+FP(11),R1
	SUBI	1,R1
	STI	R1,*+FP(11)
	ADDI	R1,FP,AR0
	LDI	0,R2
	STI	R2,*+AR0(1)
;>>>> 			*((unsigned int *)parms[0]++) = atox( inval, &ok );
	LDI	FP,R3
	ADDI	12,R3
	PUSH	R3
	SUBI	11,R3
	PUSH	R3
	CALL	_atox
	SUBI	2,SP
	LDI	*-FP(2),AR0
	LDI	*AR0,AR1
	STI	R0,*AR1++
	STI	AR1,*AR0
;>>>> 			if( ok == FAILURE )
	LDI	*+FP(12),R0
	BNZ	L28
;>>>> 				c40_printf( "Not a valid number %s.\n", inval );
	LDI	FP,R1
	ADDI	1,R1
	PUSH	R1
	LDI	@CONST+6,R2
	PUSH	R2
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 				break;
	B	L24
;>>>> 			switch( out_where )
;>>>> 				case 't' :
L29:
;>>>> 					c40_printf( "%x ", atox( inval, &ok ) );
	LDI	FP,R0
	ADDI	12,R0
	PUSH	R0
	SUBI	11,R0
	PUSH	R0
	CALL	_atox
	SUBI	2,SP
	PUSH	R0
	LDI	@CONST+7,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 					break;
;>>>> 				default :
	B	L30
L31:
;>>>> 					send_host( atox( inval, &ok ) );
	LDI	FP,R0
	ADDI	12,R0
	PUSH	R0
	SUBI	11,R0
	PUSH	R0
	CALL	_atox
	SUBI	2,SP
	PUSH	R0
	CALL	_send_host
	SUBI	1,SP
	B	L30
L28:
	LDI	*-FP(3),R1
	CMPI	116,R1
	BZ	L29
	B	L31
L30:
;>>>> 			if( !(--parms[1]) )
	LDI	*-FP(2),AR0
	LDI	*+AR0(1),R0
	SUBI	1,R0
	STI	R0,*+AR0(1)
	BNZ	L23
;>>>> 				break;
L24:
;>>>> 		c40_printf( "\n\n" );
	LDI	@CONST+4,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
EPI0_4:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	15,SP
***	B	R1	;BRANCH OCCURS
	.globl	_copy
;>>>> 	void copy( unsigned long parms[] )
******************************************************
* FUNCTION DEF : _copy
******************************************************
_copy:
	PUSH	FP
	LDI	SP,FP
;>>>> 		while( parms[2]-- )
	LDI	*-FP(2),AR0
	LDI	*+AR0(2),R0
	SUBI	1,R0
	STI	R0,*+AR0(2)
	ADDI	1,R0
	BZ	EPI0_5
L33:
;>>>> 			*((unsigned long *) parms[1]++) = *((unsigned long *) parms[0]++);
	LDI	*-FP(2),AR0
	LDI	*AR0,AR1
	LDI	*AR1++,R0
	STI	AR1,*AR0
	LDI	*+AR0(1),AR1
	STI	R0,*AR1++
	STI	AR1,*+AR0(1)
	LDI	*+AR0(2),R0
	SUBI	1,R0
	STI	R0,*+AR0(2)
	ADDI	1,R0
	BNZ	L33
EPI0_5:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	2,SP
***	B	R1	;BRANCH OCCURS
	.globl	_search
;>>>> 	void search( unsigned long parms[], char out_where )
;>>>> 		while( parms[0] != parms[1] )
******************************************************
* FUNCTION DEF : _search
******************************************************
_search:
	PUSH	FP
	LDI	SP,FP
;>>>> 			if( *((unsigned long *)parms[0]++) == parms[2] )
	B	L36
L35:
	LDI	*-FP(2),AR0
	LDI	*AR0,AR1
	LDI	*AR1++,R0
	STI	AR1,*AR0
	CMPI	*+AR0(2),R0
	BNZ	L36
;>>>> 				switch( out_where )
;>>>> 					case 't' :
	B	L38
L39:
;>>>> 						c40_printf( "%x found at address %x.\n", parms[2], --parms[0] );
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	SUBI	1,R0
	STI	R0,*AR0
	PUSH	R0
	LDI	*+AR0(2),R0
	PUSH	R0
	LDI	@CONST+8,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	3,SP
;>>>> 						return;
;>>>> 					default :
	B	EPI0_6
L40:
;>>>> 						send_host( --parms[0] );
	LDI	*-FP(2),AR0
	LDI	*AR0,R0
	SUBI	1,R0
	STI	R0,*AR0
	PUSH	R0
	CALL	_send_host
	SUBI	1,SP
	B	L36
L38:
	LDI	*-FP(3),R0
	CMPI	116,R0
	BZ	L39
	B	L40
L36:
	LDI	*-FP(2),AR0
	CMPI	*+AR0(1),*AR0
	BNZ	L35
;>>>> 	   switch( out_where )
;>>>> 			case 't' :
	B	L42
L43:
;>>>> 				c40_printf( "%x not found.\n", parms[2] );
	LDI	*-FP(2),AR0
	LDI	*+AR0(2),R0
	PUSH	R0
	LDI	@CONST+9,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 				return;
;>>>> 			default :
	B	EPI0_6
L44:
;>>>> 				send_host( FAILURE );
	LDI	0,R0
	PUSH	R0
	CALL	_send_host
	SUBI	1,SP
	B	EPI0_6
L42:
	LDI	*-FP(3),R0
	CMPI	116,R0
	BZ	L43
	B	L44
EPI0_6:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	2,SP
***	B	R1	;BRANCH OCCURS
	.globl	_help
;>>>> 	void help()
******************************************************
* FUNCTION DEF : _help
******************************************************
_help:
;>>>> 		c40_printf( "\n\nAriel_Mon Command summary :\n"
;>>>> 						"  Test =>              t\n\n"
;>>>> 						"  Configure =>         cf\n"
;>>>> 						"  Dump =>              d  #(of addresses to dump)\n"
;>>>> 						"  Enter =>             e  address\n"
;>>>> 						"  Fill =>              f  start_addr #(of addresses to fill) value\n"
;>>>> 						"  Copy =>              cp start1 start2 #(of addresses to copy)\n"
;>>>> 						"  Compare =>           c  start1 start2 #(of addresses to compare)\n"
;>>>> 						"  Search =>            s  from_address to_address for_data\n"
;>>>> 						"  Set Breakpoint =>    sbp address\n"
;>>>> 						"  Delete Breakpoint => dbp address\n"
;>>>> 						"  List Breakpoints  => lbp\n"
;>>>> 						"  Single Step =>       st\n"
;>>>> 						"  Go =>                g   (address, blank to continue)\n"
;>>>> 						"  Reg Dump =>          rd\n" );
	LDI	@CONST+10,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
EPI0_7:
	RETS
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.text
SL0:	.byte	"Compare fault at address [%x].",10,0
SL1:	.byte	"Compare successfull.",10,0
SL2:	.byte	10,"[%x]   ",0
SL3:	.byte	"%x  ",0
SL4:	.byte	10,10,0
SL5:	.byte	"[%x]  %x : ",0
SL6:	.byte	"Not a valid number %s.",10,0
SL7:	.byte	"%x ",0
SL8:	.byte	"%x found at address %x.",10,0
SL9:	.byte	"%x not found.",10,0
SL10:	.byte	10,10,"Ariel_Mon Command summary :",10,"  Test =>           "
	.byte	"   t",10,10,"  Configure =>         cf",10,"  Dump =>      "
	.byte	"        d  #(of addresses to dump)",10,"  Enter =>         "
	.byte	"    e  address",10,"  Fill =>              f  start_addr #("
	.byte	"of addresses to fill) value",10,"  Copy =>              cp "
	.byte	"start1 start2 #(of addresses to copy)",10,"  Compare =>    "
	.byte	"       c  start1 start2 #(of addresses to compare)",10,"  S"
	.byte	"earch =>            s  from_address to_address for_data",10
	.byte	"  Set Breakpoint =>    sbp address",10,"  Delete Breakpoint"
	.byte	" => dbp address",10,"  List Breakpoints  => lbp",10,"  Sing"
	.byte	"le Step =>       st",10,"  Go =>                g   (addres"
	.byte	"s, blank to continue)",10,"  Reg Dump =>          rd",10,0
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
