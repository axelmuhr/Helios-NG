******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 -ic:\c40 test.c C:\TMP\test.if 
;	cg30 -v40 -o -n C:\TMP\test.if C:\TMP\test.asm C:\TMP\test.tmp 
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
	.globl	_test
;>>>> 	int test( hydra_conf config, char out_where )
;>>>> 		unsigned long *mem_ptr;
;>>>> 		unsigned long i, sram_size;
;>>>> 		int failed;
******************************************************
* FUNCTION DEF : _test
******************************************************
_test:
	PUSH	FP
	LDI	SP,FP
	ADDI	4,SP
;>>>> 		led( 0, OFF, config );
	LDI	FP,AR0
	LDI	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*++AR0,R0
	LDI	0,R0
	PUSH	R0
	PUSH	R0
	CALL	_led
	SUBI	19,SP
;>>>> 		led( 1, OFF, config );
	LDI	FP,AR0
	LDI	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*++AR0,R0
	LDI	0,R0
	PUSH	R0
	LDI	1,R1
	PUSH	R1
	CALL	_led
	SUBI	19,SP
;>>>> 		c40_printf( "Testing processor 1 ....\n" );
	LDI	@CONST+0,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
;>>>> 		for( mem_ptr=(unsigned long *)0x80000000, failed=0 ; (unsigned long)mem_ptr < config.sram1_size+0x80000000 ; mem_ptr++ )
	LDI	@CONST+1,R0
	STI	R0,*+FP(1)
	LDI	0,R1
	STI	R1,*+FP(4)
	LDI	*-FP(13),R3
	SUBI	@CONST+1,R3
	CMPI	R3,R0
	BHS	L2
L1:
;>>>> 			*mem_ptr = ZEROs;
	LDI	*+FP(1),AR0
	LDI	0,R0
	STI	R0,*AR0
;>>>> 			failed = *mem_ptr?failed++:failed;
	LDI	*AR0,R1
	BZ	LL3
	LDI	*+FP(4),R1
	BD	LL4
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
***	B	LL4	;BRANCH OCCURS
LL3:
	LDI	*+FP(4),R1
LL4:
	STI	R1,*+FP(4)
;>>>> 			*mem_ptr = Fs;
	LDI	-1,R2
	STI	R2,*AR0
;>>>> 			failed = *mem_ptr==Fs?failed:failed++;
	CMPI	R2,*AR0
	BZ	LL6
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
LL6:
	STI	R1,*+FP(4)
;>>>> 			*mem_ptr = As;
	LDI	@CONST+2,R3
	STI	R3,*AR0
;>>>> 			failed = *mem_ptr==As?failed:failed++;
	CMPI	R3,*AR0
	BZ	LL8
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
LL8:
	STI	R1,*+FP(4)
;>>>> 			*mem_ptr = FIVEs;
	LDI	@CONST+3,R3
	STI	R3,*AR0
;>>>> 			failed = *mem_ptr==FIVEs?failed:failed++;
	CMPI	R3,*AR0
	BZ	LL10
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
LL10:
	STI	R1,*+FP(4)
;>>>> 			if( failed )
	CMPI	0,R1
	BZ	L3
;>>>> 				c40_printf( "     Memory test FAILURE on processor 1 at address %xh: Aborting.\n", mem_ptr );
	LDI	*+FP(1),R3
	PUSH	R3
	LDI	@CONST+4,R3
	PUSH	R3
	CALL	_c40_printf
	BD	EPI0_1
	SUBI	2,SP
	NOP
;>>>> 				return( 0 );
	LDI	0,R0
***	B	EPI0_1	;BRANCH OCCURS
L3:
	LDI	*+FP(1),R3
	ADDI	1,R3
	STI	R3,*+FP(1)
	LDI	*-FP(13),R0
	SUBI	@CONST+1,R0
	CMPI	R0,R3
	BLO	L1
L2:
;>>>> 		for( mem_ptr=(unsigned long *)config.l_dram_base, failed=0 ; (unsigned long)mem_ptr < config.dram_size+config.l_dram_base ; mem_ptr++ )
	LDI	*-FP(9),R0
	STI	R0,*+FP(1)
	LDI	0,R1
	STI	R1,*+FP(4)
	ADDI	*-FP(16),R0
	CMPI	R0,*+FP(1)
	BHS	L5
L4:
;>>>> 			*mem_ptr = ZEROs;
	LDI	*+FP(1),AR0
	LDI	0,R0
	STI	R0,*AR0
;>>>> 			failed = *mem_ptr?failed++:failed;
	LDI	*AR0,R1
	BZ	LL11
	LDI	*+FP(4),R1
	BD	LL12
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
***	B	LL12	;BRANCH OCCURS
LL11:
	LDI	*+FP(4),R1
LL12:
	STI	R1,*+FP(4)
;>>>> 			*mem_ptr = Fs;
	LDI	-1,R2
	STI	R2,*AR0
;>>>> 			failed = *mem_ptr==Fs?failed:failed++;
	CMPI	R2,*AR0
	BZ	LL14
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
LL14:
	STI	R1,*+FP(4)
;>>>> 			*mem_ptr = As;
	LDI	@CONST+2,R3
	STI	R3,*AR0
;>>>> 			failed = *mem_ptr==As?failed:failed++;
	CMPI	R3,*AR0
	BZ	LL16
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
LL16:
	STI	R1,*+FP(4)
;>>>> 			*mem_ptr = FIVEs;
	LDI	@CONST+3,R3
	STI	R3,*AR0
;>>>> 			failed = *mem_ptr==FIVEs?failed:failed++;
	CMPI	R3,*AR0
	BZ	LL18
	ADDI	1,R1
	STI	R1,*+FP(4)
	SUBI	1,R1
LL18:
	STI	R1,*+FP(4)
;>>>> 			if( failed )
	CMPI	0,R1
	BZ	L6
;>>>> 				c40_printf( "     Memory test FAILURE on processor 1 at address %xh: Aborting.\n", mem_ptr );
	LDI	*+FP(1),R3
	PUSH	R3
	LDI	@CONST+4,R3
	PUSH	R3
	CALL	_c40_printf
	BD	EPI0_1
	SUBI	2,SP
	NOP
;>>>> 	      	return( 0 );
	LDI	0,R0
***	B	EPI0_1	;BRANCH OCCURS
L6:
	LDI	*+FP(1),R3
	ADDI	1,R3
	STI	R3,*+FP(1)
	LDI	*-FP(16),R3
	ADDI	*-FP(9),R3
	CMPI	R3,*+FP(1)
	BLO	L4
L5:
;>>>> 		reset_others( config );
	LDI	FP,AR0
	LDI	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*++AR0,R0
	CALL	_reset_others
	SUBI	17,SP
;>>>> 		boot_others( config );
	LDI	FP,AR0
	LDI	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*++AR0,R0
	CALL	_boot_others
	SUBI	17,SP
;>>>> 		return( 1 );
	LDI	1,R0
EPI0_1:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	6,SP
***	B	R1	;BRANCH OCCURS
	.globl	_reset_others
;>>>> 	void reset_others( hydra_conf config )
;>>>> 		unsigned long ctrl_val;
******************************************************
* FUNCTION DEF : _reset_others
******************************************************
_reset_others:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
;>>>> 		ctrl_val = *(unsigned long *)config.l_cont_base;
	LDI	*-FP(5),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(1)
;>>>> 		ctrl_val &= 0x8FFFFFFF;
	AND	@CONST+5,R0
	STI1	R0,*+FP(1)
    ||	STI2	R0,*AR0
;>>>> 		*(unsigned long *)config.l_cont_base = ctrl_val;
;>>>> 		ctrl_val |= 0x70000000;
	OR	@CONST+6,R0
	STI1	R0,*+FP(1)
    ||	STI2	R0,*AR0
;>>>> 		*(unsigned long *)config.l_cont_base = ctrl_val;
EPI0_2:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
	.globl	_led
;>>>> 	void led( int which, int on_off, hydra_conf config )
;>>>> 		unsigned long control_reg;
******************************************************
* FUNCTION DEF : _led
******************************************************
_led:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
;>>>> 		control_reg = *(unsigned long *)config.l_cont_base;
	BD	L7
	LDI	*-FP(7),AR0
	LDI	*AR0,R0
	STI	R0,*+FP(1)
;>>>> 		switch( which )
;>>>> 			case 0:
***	B	L7	;BRANCH OCCURS
;>>>> 				switch( on_off )
;>>>> 					case OFF:
L10:
;>>>> 						control_reg &= 0xFFEFFFFF;
	BD	L13
	LDI	*+FP(1),R0
	AND	@CONST+7,R0
	STI	R0,*+FP(1)
;>>>> 						break;
;>>>> 					case ON:
***	B	L13	;BRANCH OCCURS
L12:
;>>>> 						control_reg |= 0x00100000;
	BD	L13
	LDI	*+FP(1),R0
	OR	@CONST+8,R0
	STI	R0,*+FP(1)
;>>>> 						break;
***	B	L13	;BRANCH OCCURS
L9:
	LDI	*-FP(3),R0
	BZ	L10
	CMPI	1,R0
	BZ	L12
;>>>> 				break;
;>>>> 			case 1:
	B	L13
;>>>> 				switch( on_off )
;>>>> 					case OFF:
L16:
;>>>> 						control_reg &= 0xFFDFFFFF;
	BD	L13
	LDI	*+FP(1),R0
	AND	@CONST+9,R0
	STI	R0,*+FP(1)
;>>>> 						break;
;>>>> 					case ON:
***	B	L13	;BRANCH OCCURS
L18:
;>>>> 						control_reg |= 0x00200000;
	BD	L13
	LDI	*+FP(1),R0
	OR	@CONST+10,R0
	STI	R0,*+FP(1)
;>>>> 						break;
***	B	L13	;BRANCH OCCURS
L15:
	LDI	*-FP(3),R0
	BZ	L16
	CMPI	1,R0
	BZ	L18
;>>>> 				break;
	B	L13
L7:
	LDI	*-FP(2),R1
	BZ	L9
	CMPI	1,R1
	BZ	L15
L13:
;>>>> 		*(unsigned long *)config.l_cont_base = control_reg;
	LDI	*-FP(7),AR0
	LDI	*+FP(1),R0
	STI	R0,*AR0
EPI0_3:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.text
SL0:	.byte	"Testing processor 1 ....",10,0
SL1:	.byte	"     Memory test FAILURE on processor 1 at address %xh: Abo"
	.byte	"rting.",10,0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,11
	.sect	".cinit"
	.word	11,CONST
	.word 	SL0              ;0
	.word 	-2147483648      ;1
	.word 	-1431655766      ;2
	.word 	1431655765       ;3
	.word 	SL1              ;4
	.word 	08fffffffh       ;5
	.word 	070000000h       ;6
	.word 	0ffefffffh       ;7
	.word 	0100000h         ;8
	.word 	0ffdfffffh       ;9
	.word 	0200000h         ;10
	.end
