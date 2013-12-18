******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 -ic:\c40 boot_oth.c C:\TMP\boot_oth.if 
;	cg30 -v40 -o -n C:\TMP\boot_oth.if C:\TMP\boot_oth.asm C:\TMP\boot_oth.tmp 
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
	.globl	_boot_others
;>>>> 	int boot_others( hydra_conf config )
;>>>> 		unsigned long i, j, k;
;>>>> 		unsigned long *ptr;
;>>>> 		unsigned long sram_sizes[3];
******************************************************
* FUNCTION DEF : _boot_others
******************************************************
_boot_others:
	PUSH	FP
	LDI	SP,FP
	ADDI	7,SP
;>>>> 		sram_sizes[0] = config.sram2_size;
	LDI	*-FP(12),R0
	STI	R0,*+FP(5)
;>>>> 		sram_sizes[1] = config.sram3_size;
	LDI	*-FP(11),R1
	STI	R1,*+FP(6)
;>>>> 		sram_sizes[2] = config.sram4_size;
	LDI	*-FP(10),R2
	STI	R2,*+FP(7)
;>>>> 		for( j=0 ; j < config.daughter?3:1 ; j++ )
	LDI	0,R3
	STI	R3,*+FP(2)
	CMPI	*-FP(2),R3
	LDILO	3,R3
	LDIHS	1,R3
	CMPI	0,R3
	BNZ	L3
	B	EPI0_1
;>>>> 			switch( j )
;>>>> 				case 0:
L4:
;>>>> 					led( 0, ON, config );
	LDI	FP,AR0
	LDI	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*++AR0,R0
	LDI	1,R0
	PUSH	R0
	LDI	0,R1
	PUSH	R1
	CALL	_led
	SUBI	19,SP
;>>>> 					led( 1, OFF, config );
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
;>>>> 					break;
;>>>> 				case 1:
	B	L5
L6:
;>>>> 					led( 0, OFF, config );
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
;>>>> 					led( 1, ON, config );
	LDI	FP,AR0
	LDI	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*++AR0,R0
	LDI	1,R0
	PUSH	R0
	PUSH	R0
	CALL	_led
	SUBI	19,SP
;>>>> 					break;
;>>>> 				case 2:
	B	L5
L7:
;>>>> 					led( 0, ON, config );
	LDI	FP,AR0
	LDI	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*++AR0,R0
	LDI	1,R0
	PUSH	R0
	LDI	0,R1
	PUSH	R1
	CALL	_led
	SUBI	19,SP
;>>>> 					led( 1, ON, config );
	LDI	FP,AR0
	LDI	SP,AR1
	ADDI	17,SP
	LDI	*--AR0(18),R0
	RPTS	16
	STI	R0,*++AR1
    ||	LDI	*++AR0,R0
	LDI	1,R0
	PUSH	R0
	PUSH	R0
	CALL	_led
	SUBI	19,SP
;>>>> 					break;
	B	L5
L3:
	LDI	*+FP(2),R0
	BZ	L4
	CMPI	1,R0
	BZ	L6
	CMPI	2,R0
	BZ	L7
L5:
;>>>> 	      boot_copy( j );
	LDI	*+FP(2),R0
	PUSH	R0
	CALL	_boot_copy
	SUBI	1,SP
;>>>> 			c40_printf( "Testing processor %d ......\n", j );
	LDI	*+FP(2),R0
	PUSH	R0
	LDI	@CONST+0,R1
	PUSH	R1
	CALL	_c40_printf
	BD	L9
	SUBI	2,SP
;>>>> 			for( k=0 ; !(COMM_CONT(j) & IN_READY) && (k<COPY_TIMEOUT) ; k++ );
	LDI	0,R0
	STI	R0,*+FP(3)
***	B	L9	;BRANCH OCCURS
L8:
	LDI	*+FP(3),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
L9:
	LDI	*+FP(2),R1
	LSH	4,R1
	LDI	R1,AR0
	LDI	@CONST+1,IR0
	LDI	*+AR0(IR0),R1
	TSTB	3840,R1
	BNZ	LL5
	CMPI	256,R0
	BLO	L8
LL5:
;>>>> 			if( (IN_FIFO( j )!=IM_UP) || (k==COPY_TIMEOUT) )
	LDI	*+FP(2),R1
	PUSH	R1
	CALL	_in_fifo
	SUBI	1,SP
	CMPI	15,R0
	BNZ	LL6
	LDI	*+FP(3),R0
	CMPI	256,R0
	BNZ	L10
LL6:
;>>>> 				c40_printf( "Synchronization error with processor %d: Aborting.\n", j );
	LDI	*+FP(2),R0
	PUSH	R0
	LDI	@CONST+2,R1
	PUSH	R1
	CALL	_c40_printf
	BD	EPI0_1
	SUBI	2,SP
	NOP
;>>>> 				return( 0 );
	LDI	0,R0
***	B	EPI0_1	;BRANCH OCCURS
L10:
;>>>> 			c40_printf( "Communication test passed.\n" );
	LDI	@CONST+3,R1
	PUSH	R1
	CALL	_c40_printf
	BD	L12
	SUBI	1,SP
;>>>> 			for( k=0 ; !(COMM_CONT(j)&OUT_READY) && (k<COPY_TIMEOUT) ; k++ );
	LDI	0,R0
	STI	R0,*+FP(3)
***	B	L12	;BRANCH OCCURS
L11:
	LDI	*+FP(3),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
L12:
	LDI	*+FP(2),R1
	LSH	4,R1
	LDI	R1,AR0
	LDI	@CONST+1,IR0
	LDI	*+AR0(IR0),R1
	TSTB	240,R1
	BNZ	LL7
	CMPI	256,R0
	BLO	L11
LL7:
;>>>> 			if( k == COPY_TIMEOUT )
	CMPI	256,R0
	BNZ	L13
;>>>> 				c40_printf( "Communication error with processor %d: Aborting.\n", j+2 );
	LDI	*+FP(2),R3
	ADDI	2,R3
	PUSH	R3
	LDI	@CONST+4,R1
	PUSH	R1
	CALL	_c40_printf
	BD	EPI0_1
	SUBI	2,SP
	NOP
;>>>> 				return( 0 );
	LDI	0,R0
***	B	EPI0_1	;BRANCH OCCURS
L13:
;>>>> 			OUT_FIFO( j, sram_sizes[j] );
	LDI	*+FP(2),R1
	LSH	4,R1
	LDI	R1,AR0
	LDI	*+AR0(IR0),R1
	ANDN	04h,R1
	STI	R1,*+AR0(IR0)
	LDI	*+FP(2),R1
	LSH	4,R1
	LDI	R1,AR0
	LDI	*+FP(2),R1
	ADDI	R1,FP,AR1
	LDI	*+AR1(5),R2
	LDI	@CONST+5,IR1
	BD	L15
	STI	R2,*+AR0(IR1)
;>>>> 			for( k=0 ; !(COMM_CONT(j)&IN_READY) && (k<TEST_TIMEOUT) ; k++ );
	LDI	0,R0
	STI	R0,*+FP(3)
***	B	L15	;BRANCH OCCURS
L14:
	LDI	*+FP(3),R0
	ADDI	1,R0
	STI	R0,*+FP(3)
L15:
	LDI	*+FP(2),R1
	LSH	4,R1
	LDI	R1,AR0
	LDI	@CONST+1,IR0
	LDI	*+AR0(IR0),R1
	TSTB	3840,R1
	BNZ	LL8
	CMPI	@CONST+6,R0
	BLO	L14
LL8:
;>>>> 			if( k == TEST_TIMEOUT )
	CMPI	@CONST+6,R0
	BNZ	L16
;>>>> 				c40_printf( "Communication error with processor %d: Aborting.\n", j+2 );
	LDI	*+FP(2),R3
	ADDI	2,R3
	PUSH	R3
	LDI	@CONST+4,R1
	PUSH	R1
	CALL	_c40_printf
	BD	EPI0_1
	SUBI	2,SP
	NOP
;>>>> 				return( 0 );
	LDI	0,R0
***	B	EPI0_1	;BRANCH OCCURS
L16:
;>>>> 			else if( !IN_FIFO(j) )
	LDI	*+FP(2),R1
	PUSH	R1
	CALL	_in_fifo
	SUBI	1,SP
	CMPI	0,R0
	BNZ	L17
;>>>> 				c40_printf( "     Memory test FAILURE on processor %d at address %xh: Aborting.\n", j+2, IN_FIFO(j) );
	LDI	*+FP(2),R0
	PUSH	R0
	CALL	_in_fifo
	SUBI	1,SP
	PUSH	R0
	LDI	*+FP(2),R3
	ADDI	2,R3
	PUSH	R3
	LDI	@CONST+7,R0
	PUSH	R0
	CALL	_c40_printf
	BD	EPI0_1
	SUBI	3,SP
	NOP
;>>>> 				return( 0 );
;>>>> 			else
	LDI	0,R0
***	B	EPI0_1	;BRANCH OCCURS
L17:
;>>>> 				c40_printf( "     DSP %d memory test PASSED.\n", j+2 );
	LDI	*+FP(2),R3
	ADDI	2,R3
	PUSH	R3
	LDI	@CONST+8,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 			c40_printf( "\n\n" );
	LDI	@CONST+9,R0
	PUSH	R0
	CALL	_c40_printf
	SUBI	1,SP
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	*-FP(2),R0
	LDILO	3,R1
	LDIHS	1,R1
	CMPI	0,R1
	BNZ	L3
EPI0_1:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	9,SP
***	B	R1	;BRANCH OCCURS
	.globl	_in_fifo
;>>>> 	unsigned long in_fifo( unsigned long port )
******************************************************
* FUNCTION DEF : _in_fifo
******************************************************
_in_fifo:
	PUSH	FP
	LDI	SP,FP
;>>>> 		COMM_CONT( port ) |= IN;
	LDI	*-FP(2),R0
	LSH	4,R0
	LDI	R0,AR0
	LDI	@CONST+1,IR0
	LDI	*+AR0(IR0),R0
	OR	04h,R0
	STI	R0,*+AR0(IR0)
;>>>> 		return( (*(unsigned long *)(0x100041+(port*0x10))) );
	LDI	*-FP(2),R0
	LSH	4,R0
	LDI	R0,AR0
	LDI	@CONST+10,IR1
	LDI	*+AR0(IR1),R0
EPI0_2:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	2,SP
***	B	R1	;BRANCH OCCURS
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.text
SL0:	.byte	"Testing processor %d ......",10,0
SL1:	.byte	"Synchronization error with processor %d: Aborting.",10,0
SL2:	.byte	"Communication test passed.",10,0
SL3:	.byte	"Communication error with processor %d: Aborting.",10,0
SL4:	.byte	"     Memory test FAILURE on processor %d at address %xh: Ab"
	.byte	"orting.",10,0
SL5:	.byte	"     DSP %d memory test PASSED.",10,0
SL6:	.byte	10,10,0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,11
	.sect	".cinit"
	.word	11,CONST
	.word 	SL0              ;0
	.word 	1048640          ;1
	.word 	SL1              ;2
	.word 	SL2              ;3
	.word 	SL3              ;4
	.word 	1048642          ;5
	.word 	1048576          ;6
	.word 	SL4              ;7
	.word 	SL5              ;8
	.word 	SL6              ;9
	.word 	1048641          ;10
	.end
