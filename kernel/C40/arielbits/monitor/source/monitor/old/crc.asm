******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 -ic:\c40 crc.c C:\TMP\crc.if 
;	cg30 -v40 -o -n C:\TMP\crc.if C:\TMP\crc.asm C:\TMP\crc.tmp 
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
	.globl	_crctab
	.globl	_crcupdate
;>>>> 	void crcupdate( unsigned short data, unsigned short *accum )
;>>>> 		unsigned short comb_val;
******************************************************
* FUNCTION DEF : _crcupdate
******************************************************
_crcupdate:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
;>>>> 		comb_val = (*accum >> 8) ^ (data & 0xFF);
	LDI	*-FP(3),AR0
	LDI	*AR0,R0
	LSH	-8,R0
	LDI	*-FP(2),R1
	AND	0ffh,R1
	XOR	R1,R0
	STI	R0,*+FP(1)
;>>>> 		*accum = (*accum << 8) ^ crctab[comb_val];
	LDI	*AR0,R1
	LSH	8,R1
	ADDI	@CONST+0,R0
	LDI	R0,AR1
	XOR	*AR1,R1,R0
	STI	R0,*AR0
EPI0_1:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
	.globl	_x_rcvcrc
;>>>> 	unsigned short x_rcvcrc( char *data, int data_size )
;>>>> 		unsigned short i, accum;
******************************************************
* FUNCTION DEF : _x_rcvcrc
******************************************************
_x_rcvcrc:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
;>>>> 		for( accum=i=0 ; i < data_size+2 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
	STI	R0,*+FP(2)
	LDI	*-FP(3),R3
	ADDI	2,R3
	CMPI	R3,R0
	BHS	L2
L1:
;>>>> 			crcupdate( *data++, &accum );
	LDI	FP,R0
	ADDI	2,R0
	PUSH	R0
	LDI	*-FP(2),AR0
	LDI	*AR0++,R1
	STI	AR0,*-FP(2)
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	LDI	*-FP(3),R3
	ADDI	2,R3
	CMPI	R3,R0
	BLO	L1
L2:
;>>>> 		return( accum );
	LDI	*+FP(2),R0
EPI0_2:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	4,SP
***	B	R1	;BRANCH OCCURS
	.globl	_x_sndcrc
;>>>> 	unsigned short x_sndcrc( char *buff, int data_size )
;>>>> 		unsigned short i, accum;
******************************************************
* FUNCTION DEF : _x_sndcrc
******************************************************
_x_sndcrc:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
;>>>> 		for( accum=i=0 ; i < data_size ; ++i )
	LDI	0,R0
	STI	R0,*+FP(1)
	STI	R0,*+FP(2)
	CMPI	*-FP(3),R0
	BHS	L4
L3:
;>>>> 			crcupdate( *buff++, &accum );
	LDI	FP,R0
	ADDI	2,R0
	PUSH	R0
	LDI	*-FP(2),AR0
	LDI	*AR0++,R1
	STI	AR0,*-FP(2)
	PUSH	R1
	CALL	_crcupdate
	SUBI	2,SP
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	*-FP(3),R0
	BLO	L3
L4:
;>>>> 		return( (accum >> 8) + (accum << 8) );
	LDI	*+FP(2),R1
	LSH	-8,R1
	LDI	*+FP(2),R2
	LSH	8,R2
	ADDI	R2,R1,R0
EPI0_3:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	4,SP
***	B	R1	;BRANCH OCCURS
	.globl	_crchware
;>>>> 	unsigned short crchware( unsigned short data, unsigned short genpoly, unsigned short accum )
;>>>> 		static int i;
******************************************************
* FUNCTION DEF : _crchware
******************************************************
_crchware:
	PUSH	FP
	LDI	SP,FP
	.bss	STATIC_1,1
;>>>> 		data <<= 8;
	LDI	*-FP(2),R0
	LSH	8,R0
	STI	R0,*-FP(2)
;>>>> 		for( i=8 ; i > 0 ; i-- )
	LDI	8,R1
	STI	R1,@STATIC_1
L5:
;>>>> 			if( (data^accum) & 0x8000 )
	LDI	*-FP(2),R0
	XOR	*-FP(4),R0
	TSTB	@CONST+1,R0
	BZ	L7
;>>>> 				accum = (accum << 1) ^ genpoly;
	LDI	*-FP(4),R0
	BD	L8
	LSH	1,R0
	XOR	*-FP(3),R0
	STI	R0,*-FP(4)
;>>>> 			else
***	B	L8	;BRANCH OCCURS
L7:
;>>>> 				accum <<= 1;
	LDI	*-FP(4),R0
	LSH	1,R0
	STI	R0,*-FP(4)
L8:
;>>>> 			data <<= 1;
	LDI	*-FP(2),R1
	LSH	1,R1
	STI	R1,*-FP(2)
	LDI	@STATIC_1,R2
	SUBI	1,R2
	STI	R2,@STATIC_1
	BGT	L5
;>>>> 		return( accum );
EPI0_4:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	2,SP
***	B	R1	;BRANCH OCCURS
	.globl	_mk_crctbl
;>>>> 	void mk_crctbl( unsigned short poly, unsigned short (*crcfn)(unsigned short,unsigned short,unsigned short) )
;>>>> 		int i;
******************************************************
* FUNCTION DEF : _mk_crctbl
******************************************************
_mk_crctbl:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
;>>>> 		for( i=0 ; i < 256 ; i++ )
	LDI	0,R0
	STI	R0,*+FP(1)
L9:
;>>>> 			crctab[i] = (*crcfn)( i, poly, 0 );
	LDI	0,R0
	PUSH	R0
	LDI	*-FP(2),R1
	PUSH	R1
	LDI	*+FP(1),R2
	PUSH	R2
	LDI	*-FP(3),R3
	CALLU	R3
	SUBI	3,SP
	LDI	@CONST+0,R1
	ADDI	*+FP(1),R1
	LDI	R1,AR0
	STI	R0,*AR0
	LDI	*+FP(1),R0
	ADDI	1,R0
	STI	R0,*+FP(1)
	CMPI	256,R0
	BLT	L9
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,2
	.sect	".cinit"
	.word	2,CONST
	.word 	_crctab          ;0
	.word 	32768            ;1
	.end
