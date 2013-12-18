******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 break_pt.c C:\TMP\break_pt.if 
;	cg30 -v40 -o -n C:\TMP\break_pt.if C:\TMP\break_pt.asm C:\TMP\break_pt.tmp 
FP	.set	AR3
	.globl	_setup
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
	.globl	_write_config
	.globl	_update_chksum
	.globl	_zero_regs
	.globl	_monitor_set
	.globl	_call_set
	.globl	_break_pt
;>>>> 	void break_pt( void )   /* Break Point Handler */
******************************************************
* FUNCTION DEF : _break_pt
******************************************************
_break_pt:
	call	clr_int			;Restore ST
;Save program's context
	push	ar1
	push	ar0
	LDI	@CONST+0,ar0		;Get pointer to call_set structure

        sti	st,*+ar0(37)		;Save st
	push	r0			;Save lower 32 bits of
	pop	ar1			;extended precision registers
	sti	ar1,*ar0++
	push	r1
	pop	ar1
	sti	ar1,*ar0++
	push	r2
	pop	ar1
	sti	ar1,*ar0++
	push	r3
	pop	ar1
	sti	ar1,*ar0++
	push	r4
	pop	ar1
	sti	ar1,*ar0++
	push	r5
	pop	ar1
	sti	ar1,*ar0++
	push	r6
	pop	ar1
	sti	ar1,*ar0++
	push	r7
	pop	ar1
	sti	ar1,*ar0++
	push	r8
	pop	ar1
	sti	ar1,*ar0++
	push	r9
	pop	ar1
	sti	ar1,*ar0++
	push	r10
	pop	ar1
	sti	ar1,*ar0++
	push	r11
	pop	ar1
	sti	ar1,*ar0++

	pushf	r0			;Save upper 32 bits of
	pop	ar1			;extended precision registers
	sti	ar1,*ar0++
	pushf	r1
	pop	ar1
	sti	ar1,*ar0++
	pushf	r2
	pop	ar1
	sti	ar1,*ar0++
	pushf	r3
	pop	ar1
	sti	ar1,*ar0++
	pushf	r4
	pop	ar1
	sti	ar1,*ar0++
	pushf	r5
	pop	ar1
	sti	ar1,*ar0++
	pushf	r6
	pop	ar1
	sti	ar1,*ar0++
	pushf	r7
	pop	ar1
	sti	ar1,*ar0++
	pushf	r8
	pop	ar1
	sti	ar1,*ar0++
	pushf	r9
	pop	ar1
	sti	ar1,*ar0++
	pushf	r10
	pop	ar1
	sti	ar1,*ar0++
	pushf	r11
	pop	ar1
	sti	ar1,*ar0++

	pop	ar1			;Save ar0
	sti	ar1,*ar0++
	pop	ar1			;Save ar1
	sti	ar2,*ar0++		;Save remaining address regesters
	sti	ar3,*ar0++
	sti	ar4,*ar0++
	sti	ar5,*ar0++
	sti	ar6,*ar0++
	sti	ar7,*ar0++

        ldi	sp,ar1			;Save the rest of the regester set
	subi	1,ar1			;Loose the interrupt return address
	sti	ar1,*ar0++
	sti	ir0,*ar0++
	sti	ir1,*ar0++
	sti	dp,*ar0++
        addi	1,ar0			;Skip st space
	sti	die,*ar0++
	sti	iie,*ar0++
	sti	iif,*ar0++
	sti	ivtp,*ar0++
	sti	tvtp,*ar0++
	pop	ar1   			;Save the return address
	sti	ar1,*ar0++

;>>>> 	   reg_dump( call_set );
	LDI	@CONST+0,AR0
	LDI	SP,AR1
	ADDI	43,SP
	LDI	*AR0++,R0
	RPTS	42
	STI	R0,*++AR1
    ||	LDI	*AR0++,R0
	CALL	_reg_dump
	SUBI	43,SP
;>>>> 		c40_printf( "Break point reached at address %x.\n", call_set.ret_add );
	LDI	@_call_set+42,R0
	PUSH	R0
	LDI	@CONST+1,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 		resume_mon( monitor_set );
	LDI	@CONST+2,AR0
	LDI	SP,AR1
	ADDI	43,SP
	LDI	*AR0++,R0
	RPTS	42
	STI	R0,*++AR1
    ||	LDI	*AR0++,R0
	CALL	_resume_mon
	SUBI	43,SP
EPI0_1:
	RETS
	.globl	_user_int
;>>>> 	void user_int( void )   /* Keyboard handler for debug mode */
******************************************************
* FUNCTION DEF : _user_int
******************************************************
_user_int:
	call	clr_int			;Restore ST
;Save program's context
	push	ar1
	push	ar0
	LDI	@CONST+0,ar0		;Get pointer to call_set structure

	sti	st,*+ar0(37)		;Save st
	push	r0			;Save lower 32 bits of
	pop	ar1			;extended precision registers
	sti	ar1,*ar0++
	push	r1
	pop	ar1
	sti	ar1,*ar0++
	push	r2
	pop	ar1
	sti	ar1,*ar0++
	push	r3
	pop	ar1
	sti	ar1,*ar0++
	push	r4
	pop	ar1
	sti	ar1,*ar0++
	push	r5
	pop	ar1
	sti	ar1,*ar0++
	push	r6
	pop	ar1
	sti	ar1,*ar0++
	push	r7
	pop	ar1
	sti	ar1,*ar0++
	push	r8
	pop	ar1
	sti	ar1,*ar0++
	push	r9
	pop	ar1
	sti	ar1,*ar0++
	push	r10
	pop	ar1
	sti	ar1,*ar0++
	push	r11
	pop	ar1
	sti	ar1,*ar0++

	pushf	r0			;Save upper 32 bits of
	pop	ar1			;extended precision registers					;
	sti	ar1,*ar0++
	pushf	r1
	pop	ar1
	sti	ar1,*ar0++
	pushf	r2
	pop	ar1
	sti	ar1,*ar0++
	pushf	r3
	pop	ar1
	sti	ar1,*ar0++
	pushf	r4
	pop	ar1
	sti	ar1,*ar0++
	pushf	r5
	pop	ar1
	sti	ar1,*ar0++
	pushf	r6
	pop	ar1
	sti	ar1,*ar0++
	pushf	r7
	pop	ar1
	sti	ar1,*ar0++
	pushf	r8
	pop	ar1
	sti	ar1,*ar0++
	pushf	r9
	pop	ar1
	sti	ar1,*ar0++
	pushf	r10
	pop	ar1
	sti	ar1,*ar0++
	pushf	r11
	pop	ar1
	sti	ar1,*ar0++

	pop	ar1			;Save ar0
	sti	ar1,*ar0++
	pop	ar1			;Save ar1
	sti	ar2,*ar0++		;Save remaining address regesters
	sti	ar3,*ar0++
	sti	ar4,*ar0++
	sti	ar5,*ar0++
	sti	ar6,*ar0++
	sti	ar7,*ar0++

        ldi	sp,ar1			;Save the rest of the regester set
	subi	1,ar1			;Loose the interrupt return address
	sti	ar1,*ar0++
	sti	ir0,*ar0++
	sti	ir1,*ar0++
	sti	dp,*ar0++
        addi	1,ar0			;Skip st space
	sti	die,*ar0++
	sti	iie,*ar0++
	sti	iif,*ar0++
	sti	ivtp,*ar0++
	sti	tvtp,*ar0++
	pop	ar1   			;Save the return address
	sti	ar1,*ar0++

;>>>> 		reg_dump( call_set );
	LDI	@CONST+0,AR0
	LDI	SP,AR1
	ADDI	43,SP
	LDI	*AR0++,R0
	RPTS	42
	STI	R0,*++AR1
    ||	LDI	*AR0++,R0
	CALL	_reg_dump
	SUBI	43,SP
;>>>> 		c40_printf( "User interrupt at address %x.\n", call_set.ret_add );
	LDI	@_call_set+42,R0
	PUSH	R0
	LDI	@CONST+3,R1
	PUSH	R1
	CALL	_c40_printf
	SUBI	2,SP
;>>>> 		resume_mon( monitor_set );
	LDI	@CONST+2,AR0
	LDI	SP,AR1
	ADDI	43,SP
	LDI	*AR0++,R0
	RPTS	42
	STI	R0,*++AR1
    ||	LDI	*AR0++,R0
	CALL	_resume_mon
	SUBI	43,SP
EPI0_2:
	RETS
;>>>> 	void resume_mon( reg_set monitor_set )
******************************************************
* FUNCTION DEF : _resume_mon
******************************************************
_resume_mon:

;Restore monitor context
	ldi	@CONST+2,ar0		;Get pointer to monitor_set structure

	ldi	*+ar0(36),sp		;Get monitor stack

	ldi	*ar0++,ar1		;Restore extended precision registers
	push	ar1
	pop	r0
        ldi	*ar0++,ar1
	push	ar1
	popf	r0
        ldi	*ar0++,ar1
	push	ar1
	pop	r1
        ldi	*ar0++,ar1
	push	ar1
	popf	r1
        ldi	*ar0++,ar1
	push	ar1
	pop	r2
        ldi	*ar0++,ar1
	push	ar1
	popf	r2
        ldi	*ar0++,ar1
	push	ar1
	pop	r3
        ldi	*ar0++,ar1
	push	ar1
	popf	r3
        ldi	*ar0++,ar1
	push	ar1
	pop	r4
        ldi	*ar0++,ar1
	push	ar1
	popf	r4
        ldi	*ar0++,ar1
	push	ar1
	pop	r5
        ldi	*ar0++,ar1
	push	ar1
	popf	r5
        ldi	*ar0++,ar1
	push	ar1
	pop	r6
        ldi	*ar0++,ar1
	push	ar1
	popf	r6
        ldi	*ar0++,ar1
	push	ar1
	pop	r7
        ldi	*ar0++,ar1
	push	ar1
	popf	r7
        ldi	*ar0++,ar1
	push	ar1
	pop	r8
        ldi	*ar0++,ar1
	push	ar1
	popf	r8
        ldi	*ar0++,ar1
	push	ar1
	pop	r9
        ldi	*ar0++,ar1
	push	ar1
	popf	r9
        ldi	*ar0++,ar1
	push	ar1
	pop	r10
        ldi	*ar0++,ar1
	push	ar1
	popf	r10
        ldi	*ar0++,ar1
	push	ar1
	pop	r11
        ldi	*ar0++,ar1
	push	ar1
	popf	r11

	addi	2,ar0			;Restore auxiliary registers
	ldi	*ar0++,ar2
	ldi	*ar0++,ar3
	ldi	*ar0++,ar4
	ldi	*ar0++,ar5
	ldi	*ar0++,ar6
	ldi	*ar0++,ar7
	ldi	*ar0++,sp
	ldi	*ar0++,ir0
	ldi	*ar0++,ir1
	ldi	*ar0++,dp
	ldi	*ar0++,st
	ldi	*ar0++,die
	ldi	*ar0++,iie
	ldi	*ar0++,iif
	ldi	*ar0++,ivtp
	ldi	*ar0++,tvtp
	push	ar2			;Restore pointer to key handler
	ldi	ivtp,ar2
	ldi	*+ar0(1),ar1   		
	sti	ar1,*+ar2(1)
	pop	ar2
	ldi	*ar0++,ar1		;Get return address
        ldi	@CONST+2,ar0		;Get pointer to monitor_set structure
	bd	ar1
	ldi	*+ar0(25),ar1		;Restore ar1, ar0 and st while branch
        ldi	*+ar0(37),st		;propagates through pipeline
	ldinuf	*+ar0(24),ar0		;Conditional load insures that st is
					;unaffected.  nuf is allways true
					;after ldi.
;****   bd     ar1  OCCURS
******************************************************
* FUNCTION DEF : clr_int
******************************************************
clr_int:
	reti
******************************************************
* DEFINE STRINGS                                     *
******************************************************
	.text
SL0:	.byte	"Break point reached at address %x.",10,0
SL1:	.byte	"User interrupt at address %x.",10,0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,4
	.sect	".cinit"
	.word	4,CONST
	.word 	_call_set        ;0
	.word 	SL0              ;1
	.word 	_monitor_set     ;2
	.word 	SL1              ;3
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_reg_dump
	.end
