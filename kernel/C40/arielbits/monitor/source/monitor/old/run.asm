	.global		_monitor_set
	.global		_call_set
	.global		_run
	.global		_c_int01

FP	.set	AR3




*****************************************
**                                     **
**   void run( int BREAK_TRAP_NUM );   **
**                                     **
*****************************************
_run:
	PUSH	FP
	LDI	SP,FP

**************************
** Save monitor context **
**************************
	push	ar1
	push	ar0
	ldi	@CONST+0,ar0		;Get pointer to monitor_set structure

	push	r0			;Save lower 32 bits of
	pop	ar1			;extended precision registers
	sti	ar1,*+ar0(0)
	push	r1
	pop	ar1
	sti	ar1,*+ar0(1)
	push	r2
	pop	ar1
	sti	ar1,*+ar0(2)
	push	r3
	pop	ar1
	sti	ar1,*+ar0(3)
	push	r4
	pop	ar1
	sti	ar1,*+ar0(4)
	push	r5
	pop	ar1
	sti	ar1,*+ar0(5)
	push	r6
	pop	ar1
	sti	ar1,*+ar0(6)
	push	r7
	pop	ar1
	sti	ar1,*+ar0(7)
	push	r8
	pop	ar1
	sti	ar1,*+ar0(8)
	push	r9
	pop	ar1
	sti	ar1,*+ar0(9)
	push	r10
	pop	ar1
	sti	ar1,*+ar0(10)
	push	r11
	pop	ar1
	sti	ar1,*+ar0(11)

	pushf	r0			;Save upper 32 bits of
	pop	ar1			;extended precision registers
	sti	ar1,*+ar0(12)
	pushf	r1
	pop	ar1
	sti	ar1,*+ar0(13)
	pushf	r2
	pop	ar1
	sti	ar1,*+ar0(14)
	pushf	r3
	pop	ar1
	sti	ar1,*+ar0(15)
	pushf	r4
	pop	ar1
	sti	ar1,*+ar0(16)
	pushf	r5
	pop	ar1
	sti	ar1,*+ar0(17)
	pushf	r6
	pop	ar1
	sti	ar1,*+ar0(18)
	pushf	r7
	pop	ar1
	sti	ar1,*+ar0(19)
	pushf	r8
	pop	ar1
	sti	ar1,*+ar0(20)
	pushf	r9
	pop	ar1
	sti	ar1,*+ar0(21)
	pushf	r10
	pop	ar1
	sti	ar1,*+ar0(22)
	pushf	r11
	pop	ar1
	sti	ar1,*+ar0(23)

	pop	ar1			;Save ar0
	sti	ar1,*+ar0(24)
	pop	ar1			;Save ar1
	sti	ar1,*+ar0(25)
	sti	ar2,*+ar0(26)		;Save remaining address regesters
	sti	ar3,*+ar0(27)
	sti	ar4,*+ar0(28)
	sti	ar5,*+ar0(29)
	sti	ar6,*+ar0(30)
	sti	ar7,*+ar0(31)

	sti	DP,*+ar0(32)		;Save the rest of the regester set
	sti	IR0,*+ar0(33)
	sti	IR1,*+ar0(34)
	sti	BK,*+ar0(35)
	sti	SP,*+ar0(36)

	sti	ST,*+ar0(37)
	sti	DIE,*+ar0(38)
	sti	IIE,*+ar0(39)
	sti	IIF,*+ar0(40)

	sti	RS,*+ar0(41)
	sti	RE,*+ar0(42)
	sti	RC,*+ar0(43)

	sti	IVTP,*+ar0(44)
	sti	TVTP,*+ar0(45)


******************************************
** Install return address on keyboard   **
** interrupt and breakpoint trap vector **
******************************************
	ldi	@CONST+1,ar0
	ldi	*+ar0(44),IVTP
	ldi	*+ar0(45),TVTP
	ldi	@CONST+2,ar1		;Get Address where user program should
					;return to in this code when stopped.
	ldi	IVTP,ar0		;Install this address on keyboard
	sti	ar1,*+ar0(1)		;interrupt vector.
	ldi     *-FP(2),ir0		;Also install it on the breakpoint
	ldi	TVTP,ar0		;trap vector.
	sti	ar1,*+ar0(ir0)

**************************
** Restore user context **
**************************
	ldi	*+ar0(0),ar1		;Restore lower 32 bits of
	push	ar1			;extended precision registers
	pop	r0
	ldi	*+ar0(1),ar1
	push	ar1
	pop	r1
	ldi	*+ar0(2),ar1
	push	ar1
	pop	r2
	ldi	*+ar0(3),ar1
	push	ar1
	pop	r3
	ldi	*+ar0(4),ar1
	push	ar1
	pop	r4
	ldi	*+ar0(5),ar1
	push	ar1
	pop	r5
	ldi	*+ar0(6),ar1
	push	ar1
	pop	r6
	ldi	*+ar0(7),ar1
	push	ar1
	pop	r7
	ldi	*+ar0(8),ar1
	push	ar1
	pop	r8
	ldi	*+ar0(9),ar1
	push	ar1
	pop	r9
	ldi	*+ar0(10),ar1
	push	ar1
	pop	r10
	ldi	*+ar0(11),ar1
	push	ar1
	pop	r11

	ldi	*+ar0(12),ar1           ;Restore upper 32 bits of
	push	ar1			;extended precision registers
	popf	r0
	ldi	*+ar0(13),ar1
	push	ar1
	popf	r1
	ldi	*+ar0(14),ar1
	push	ar1
	popf	r2
	ldi	*+ar0(15),ar1
	push	ar1
	popf	r3
	ldi	*+ar0(16),ar1
	push	ar1
	popf	r4
	ldi	*+ar0(17),ar1
	push	ar1
	popf	r5
	ldi	*+ar0(18),ar1
	push	ar1
	popf	r6
	ldi	*+ar0(19),ar1
	push	ar1
	popf	r7
	ldi	*+ar0(20),ar1
	push	ar1
	popf	r8
	ldi	*+ar0(21),ar1
	push	ar1
	popf	r9
	ldi	*+ar0(22),ar1
	push	ar1
	popf	r10
	ldi	*+ar0(23),ar1
	push	ar1
	popf	r11

	ldi	*+ar0(26),ar2		;Restore auxiliary registers
	ldi	*+ar0(27),ar3		;Note: The jump from index 23 to
	ldi	*+ar0(28),ar4		;is due to the fact that AR0 and
	ldi	*+ar0(29),ar5		;AR1 will be restored later.
	ldi	*+ar0(30),ar6
	ldi	*+ar0(31),ar7

	ldi	*+ar0(32),DP
	ldi	*+ar0(33),IR0
	ldi	*+ar0(34),IR1
	ldi	*+ar0(35),BK
	ldi	*+ar0(36),SP

	ldi	*+ar0(38),DIE
	ldi	*+ar0(39),IIE
	ldi	*+ar0(40),IIF

	ldi	*+ar0(41),RS
	ldi	*+ar0(42),RE
	ldi	*+ar0(43),RC

	ldi	*+ar0(46),ar1		;Get resume address

	bd	ar1     		;Resume user program !!!!
	ldi	*+ar0(25),ar1		;Restore AR1, AR0 and ST while branch
        ldi	*+ar0(37),ST		;propagates through pipeline.
	ldiU	*+ar0(24),ar0		;Conditional load is neccessary to
					;insure that ST isn't clobbered
;>>>>>>    bd     ar1  OCCURS

;####################################
;###                              ###
;###  User program is called here ###
;###                              ###
;####################################

RETURN_HERE
	call	INT_FIX			;This point will be reached via
					;either an interrupt or a trap, as
					;such, a RETI instruction is required
					;fix the interrupt flags.  This call
					;does this.
*******************************
** Save user program context **
*******************************
	push	ar1
	push	ar0
	ldi	@CONST+1,ar0		;Get pointer to call_set structure

	push	r0			;Save lower 32 bits of
	pop	ar1			;extended precision registers
	sti	ar1,*+ar0(0)
	push	r1
	pop	ar1
	sti	ar1,*+ar0(1)
	push	r2
	pop	ar1
	sti	ar1,*+ar0(2)
	push	r3
	pop	ar1
	sti	ar1,*+ar0(3)
	push	r4
	pop	ar1
	sti	ar1,*+ar0(4)
	push	r5
	pop	ar1
	sti	ar1,*+ar0(5)
	push	r6
	pop	ar1
	sti	ar1,*+ar0(6)
	push	r7
	pop	ar1
	sti	ar1,*+ar0(7)
	push	r8
	pop	ar1
	sti	ar1,*+ar0(8)
	push	r9
	pop	ar1
	sti	ar1,*+ar0(9)
	push	r10
	pop	ar1
	sti	ar1,*+ar0(10)
	push	r11
	pop	ar1
	sti	ar1,*+ar0(11)

	pushf	r0			;Save upper 32 bits of
	pop	ar1			;extended precision registers
	sti	ar1,*+ar0(12)
	pushf	r1
	pop	ar1
	sti	ar1,*+ar0(13)
	pushf	r2
	pop	ar1
	sti	ar1,*+ar0(14)
	pushf	r3
	pop	ar1
	sti	ar1,*+ar0(15)
	pushf	r4
	pop	ar1
	sti	ar1,*+ar0(16)
	pushf	r5
	pop	ar1
	sti	ar1,*+ar0(17)
	pushf	r6
	pop	ar1
	sti	ar1,*+ar0(18)
	pushf	r7
	pop	ar1
	sti	ar1,*+ar0(19)
	pushf	r8
	pop	ar1
	sti	ar1,*+ar0(20)
	pushf	r9
	pop	ar1
	sti	ar1,*+ar0(21)
	pushf	r10
	pop	ar1
	sti	ar1,*+ar0(22)
	pushf	r11
	pop	ar1
	sti	ar1,*+ar0(23)

	pop	ar1			;Save ar0
	sti	ar1,*+ar0(24)
	pop	ar1			;Save ar1
	sti	ar1,*+ar0(25)
	sti	ar2,*+ar0(26)		;Save remaining address regesters
	sti	ar3,*+ar0(27)
	sti	ar4,*+ar0(28)
	sti	ar5,*+ar0(29)
	sti	ar6,*+ar0(30)
	sti	ar7,*+ar0(31)

	sti	DP,*+ar0(32)		;Save the rest of the regester set
	sti	IR0,*+ar0(33)
	sti	IR1,*+ar0(34)
	sti	BK,*+ar0(35)
	subi	1,SP
	sti	SP,*+ar0(36)

	sti	ST,*+ar0(37)
	sti	DIE,*+ar0(38)
	sti	IIE,*+ar0(39)
	sti	IIF,*+ar0(40)

	sti	RS,*+ar0(41)
	sti	RE,*+ar0(42)
	sti	RC,*+ar0(43)

	sti	IVTP,*+ar0(44)
	sti	TVTP,*+ar0(45)

	ldi	SP,ar1				;Save return address
	ldi	*+ar1(1),ar1  		
	sti	ar1,*+ar0(46)

*****************************
** Restore monitor context **
*****************************
	ldi	@CONST+0,ar0		;Get pointer to monitor_set structure

	ldi	*+ar0(0),ar1		;Restore extended precision registers
	push	ar1
	pop	r0
	ldi	*+ar0(1),ar1
	push	ar1
	pop	r1
	ldi	*+ar0(2),ar1
	push	ar1
	pop	r2
	ldi	*+ar0(3),ar1
	push	ar1
	pop	r3
	ldi	*+ar0(4),ar1
	push	ar1
	pop	r4
	ldi	*+ar0(5),ar1
	push	ar1
	pop	r5
	ldi	*+ar0(6),ar1
	push	ar1
	pop	r6
	ldi	*+ar0(7),ar1
	push	ar1
	pop	r7
	ldi	*+ar0(8),ar1
	push	ar1
	pop	r8
	ldi	*+ar0(9),ar1
	push	ar1
	pop	r9
	ldi	*+ar0(10),ar1
	push	ar1
	pop	r10
	ldi	*+ar0(11),ar1
	push	ar1
	pop	r11

	ldi	*+ar0(12),ar1
	push	ar1
	popf	r0
	ldi	*+ar0(13),ar1
	push	ar1
	popf	r1
	ldi	*+ar0(14),ar1
	push	ar1
	popf	r2
	ldi	*+ar0(15),ar1
	push	ar1
	popf	r3
	ldi	*+ar0(16),ar1
	push	ar1
	popf	r4
	ldi	*+ar0(17),ar1
	push	ar1
	popf	r5
	ldi	*+ar0(18),ar1
	push	ar1
	popf	r6
	ldi	*+ar0(19),ar1
	push	ar1
	popf	r7
	ldi	*+ar0(20),ar1
	push	ar1
	popf	r8
	ldi	*+ar0(21),ar1
	push	ar1
	popf	r9
	ldi	*+ar0(22),ar1
	push	ar1
	popf	r10
	ldi	*+ar0(23),ar1
	push	ar1
	popf	r11

	ldi	*+ar0(26),ar2		;Restore auxiliary registers
	ldi	*+ar0(27),ar3
	ldi	*+ar0(28),ar4
	ldi	*+ar0(29),ar5
	ldi	*+ar0(30),ar6
	ldi	*+ar0(31),ar7

	ldi	*+ar0(32),DP
	ldi	*+ar0(33),IR0
	ldi	*+ar0(34),IR1
	ldi	*+ar0(35),BK
	ldi	*+ar0(36),SP

	ldi	*+ar0(37),ST
	ldi	*+ar0(38),DIE
	ldi	*+ar0(39),IIE
	ldi	*+ar0(40),IIF

	ldi	*+ar0(41),RS
	ldi	*+ar0(42),RE
	ldi	*+ar0(43),RC

	ldi	*+ar0(44),IVTP
	ldi	*+ar0(45),TVTP

	ldi	ivtp,ar0		;Restore pointer to key handler
	ldi	@CONST+3,ar1
	sti	ar1,*+ar0(1)

	ldi	*+ar0(46),ar1		;Get return address
	bd	ar1
	ldi	*+ar0(25),ar1		;Restore ar1, ar0 and st while branch
	ldi	*+ar0(37),ST		;propagates through pipeline
	ldiU	*+ar0(24),ar0		;Conditional load insures that st is
					;unaffected.  nuf is allways true
					;after ldi.
;****   bd     ar1  OCCURS


	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	2,SP
***	B	R1	;BRANCH OCCURS




**********************************************
** This routine is used to recover from     **
** a breakpoint or keyboard user interrupt. **
** See code above for more details.         **
**********************************************
INT_FIX
	reti
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,4
	.sect	".cinit"
	.word	4,CONST
	.word 	_monitor_set     ;0
	.word 	_call_set        ;1
	.word	RETURN_HERE      ;2
	.word	_c_int01         ;3
	.end
