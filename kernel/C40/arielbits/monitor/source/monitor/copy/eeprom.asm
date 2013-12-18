FP	.set	AR3
	.globl	_ReadEeprom
;>>>> 	unsigned long read_eeprom( unsigned long addr )
_ReadEeprom:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
**************************************************************************
*       Routine that reads from the EEPROM                               *
**************************************************************************
		push	r1		;Save regesters
		push	r2
		push	r3
		push	r4
		push	r5
		push	r6	
		push	ar0
		push	ar1
		push	ar2


		LDI	@CONST+3,ar1
		ldi	*ar1,r2		;Read its current value
		ldi	@CONST+2,r4	;Used to disable the buffer that ties data low
		or	r4,r2
		sti	r2,*ar1		;Enable is now disabled

		rpts	250		;5 micro sec start condition setup time
		nop

		or	20h,iif		;Make IIOF1 an output
		or	40h,iif		;Output a 1 so that a transiton to
					;low, while SCL is high, will signal
					;a read cycle

		ldi	@CONST,r3	;Used to make the EEPROM clock high
		ldi	@CONST+1,r4	;Used to make the EEPROM clock low
		or	r3,r2
		sti	r2,*ar1		;Clock is now HIGH
		rpts	250		;5 micro sec start condition setup time
		nop

		xor	40h,iif		;Toggle data line to *LOW* to
					;start a read cycle
		rpts	250		;5 micro sec start condition hold time
		nop
		and	r4,r2		;Make clock low
		sti	r2,*ar1		;Clock is now LOW
		
		rpts	250		;5 micro sec hold time
		nop


; Write write header into the chip
		ldi	@CONST + WRITE_HEADER,r0
		call	write_byte

;Write the byte address into the chip
		ldi	*-FP(2),r0
		call	write_byte

;Send another start bit
		or	40h,iif		;Make data bit high
		or	r3,r2		;Make clock high
		sti	r2,*ar1		;Clock is now HIGH
		rpts	250		;5 micro sec start condition setup time
		nop

		xor	40h,iif		;Toggle data line to *LOW* to
					;start a read cycle
		rpts	250		;5 micro sec start condition hold time
		nop
		and	r4,r2		;Make clock low
		sti	r2,*ar1		;Clock is now LOW
		
		rpts	250		;5 micro sec hold time   		nop
                nop

;Repeat the header again
		ldi	@CONST + READ_HEADER,r0
		call	write_byte

;Read data from EEPROM
		ldi	@CONST+7,r0
		and	r0,iif		;Make iiof[1] an input
		ldi	0,r0
		ldi	-6,r5
		ldi	8,ar2
DATA_READ	or	r3,r2		;Make clock high
		sti	r2,*ar1		;Clock is now HIGH
		rpts	250		;5 micro sec clock high time
		nop
		lsh3	r5,iif,r1	;Get next data bit
		ror	r1		;Put data bit in carry flag
		rolc	r0		;Rotate in current data bit from carry
		and	r4,r2		;Make clock low
		sti	r2,*ar1		;Clock is now LOW
		rpts	250		;5 micro sec clock low time
		nop
		subi	1,ar2
		cmpi	0,ar2
		bnz	DATA_READ

		or	r3,r2		;Make clock high
		sti	r2,*ar1		;Clock is now HIGH
		rpts	250		;5 micro sec clock high time
		nop
		and	r4,r2		;Make clock low
		sti	r2,*ar1		;Clock is now LOW
		rpts	250		;5 micro sec clock low time
		nop

		and	0FFh,r0		;Strip off all but last eight bits

; Reverse bits on read value
		ldi	0,r6		; Clear r6
		ldi	7,rc		; 8 Bits
		rptb	BitReverse
		ror	r0		; Rotate it out
BitReverse	rolc	r6		; Rotate it in

		ldi	r6,r0		; Put the bit reversed value in r0

;Send a stop bit
		or	20h,iif		;Make IIOF1 an output
		ldi	@CONST+4,r1
		and	r1,iif		;Set IIOF1 to 0   
		or	r3,r2		;Make clock high
		sti	r2,*ar1		;Clock is now HIGH
		rpts	250
		nop
		or	40h,iif		;Toggle IIOF1 to signal stop

		and	r4,r2		;Make clock low
		sti	r2,*ar1		;Clock is now LOW
		rpts	250		;5 micro sec clock low time
		nop
		or	r3,r2		;Make clock high
		sti	r2,*ar1		;Clock is now HIGH


;Restore regester set and return
		pop	ar2
		pop	ar1
		pop	ar0
		pop	r6
		pop	r5
		pop	r4
		pop	r3
		pop	r2
		pop	r1

	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS

	.globl	_WriteEeprom
;>>>> 	unsigned long write_eeprom( unsigned long addr, int data )
_WriteEeprom:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
**************************************************************************
*       Routine that writes to the EEPROM                                *
**************************************************************************
		push	r1		;Save regesters
		push	r2
		push	r3
		push	r4
		push	r5
		push	ar0
		push	ar1
		push	ar2


		LDI	@CONST+3,ar1
		ldi	*ar1,r2		;Read its current value
		ldi	@CONST+2,r4	;Used to disable the buffer that ties data low
		or	r4,r2
		sti	r2,*ar1		;Enable is now disabled

		rpts	250		;5 micro sec start condition setup time
		nop

		or	20h,iif		;Make IIOF1 an output
		or	40h,iif		;Output a 1 so that a transiton to
					;low, while SCL is high, will signal
					;a read cycle

		ldi	@CONST,r3	;Used to make the EEPROM clock high
		ldi	@CONST+1,r4	;Used to make the EEPROM clock low
		or	r3,r2
		sti	r2,*ar1		;Clock is now HIGH
		rpts	250		;5 micro sec start condition setup time
		nop

		xor	40h,iif		;Toggle data line to *LOW* to
					;start a read cycle
		rpts	250		;5 micro sec start condition hold time
		nop
		and	r4,r2		;Make clock low
		sti	r2,*ar1		;Clock is now LOW
		
		rpts	250		;5 micro sec hold time
		nop


;Write the header into the chip
		ldi	@CONST + WRITE_HEADER,r0
		call	write_byte

;Write the byte address into the chip
		ldi	*-FP(2),r0
		call	write_byte

;Write the data into the chip
		ldi	*-FP(3),r0
		call	write_byte

;Send a stop bit
		ldi	@CONST+4,r1
		and	r1,iif		;Set IIOF1 to 0
		rpts	250
		nop
		or	r3,r2		;Make clock high
		sti	r2,*ar1		;Clock is now HIGH
		rpts	250
		nop
		or	40h,iif		;Toggle IIOF1 to signal stop
		rpts	250
		nop

		rpts	0ffffh		;Kill time while the cycle is 
		nop			;completed internally
		rpts	0ffffh
		nop
		rpts	0ffffh
		nop
		rpts	0ffffh
		nop
		rpts	0ffffh
		nop
		rpts	0ffffh
		nop
		rpts	0ffffh
		nop


		ldi	*-FP(2),r0		;Return data value  ????
;Restore regester set and return
		pop	ar2
		pop	ar1
		pop	ar0
		pop	r5
		pop	r4
		pop	r3
		pop	r2
		pop	r1

EPI0_2:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS



;>>>> Function write_byte
	.globl	write_byte
write_byte:
		or	20h,iif		;Make IIF1 in output
		ldi	8,ar2
W_BYTE_TOP1	and	r4,r2		;Make clock low
		sti	r2,*ar1		;Clock is now LOW
		rpts	250		;5 micro sec clock low time		
		nop
		ldi	1,r1
		and	r0,r1		;Test for 1 or 0 in the LSB
		bz	AND_BYTE
		lsh	6,r1		;Align the current address bit
					;with the flag1 bit of IIOF
		or	r1,iif		;Put next address bit into IIOF
		b	BYTE_NEXT
AND_BYTE	ldi	@CONST+4,r1	;Get and mask
		and	r1,iif
BYTE_NEXT	rpts	250       	;280 nSec data setup delay
		nop
		lsh	-1,r0		;Put next address bit into LSB
		or	r3,r2		;Make clock high
		sti	r2,*ar1		;Clock is now HIGH
		rpts	250		;5 micro sec clock high time
		nop
		subi	1,ar2
		cmpi	0,ar2
		bnz	W_BYTE_TOP1

;Wait for acknowledgement
		ldi	iif,r1		;Check if last bit was a one;
		lsh	-6,r1
		and	1,r1
		bz	LAST_ZERO
		ldi	@CONST+7,r0     ;????
		and	r0,iif		;Make IIOF1 an input
		rpts	250		;250 nSec data setup delay
		nop
LAST_ZERO	and	r4,r2		;Make clock low
		sti	r2,*ar1		;Clock is now LOW
		ldi	@CONST+7,r0
		rpts	250
		nop
		and	r0,iif		;Make IIOF1 an input
		rpts	250		;250 nSec data setup delay
		nop

W_BYTE_WAIT_ACK	ldi	iif,r0
		and	40h,r0		;Check for acknowledgement
		bnz	W_BYTE_WAIT_ACK
                or	r3,r2		;Make clock high
                sti	r2,*ar1         ;Clock is now HIGH
		rpts	250		;5 micro sec clock high time
		nop
		and	r4,r2		;Make clock low
		sti	r2,*ar1		;Clock is now LOW
		rpts	250		;5 micro sec clock low time
		nop
		or	20h,iif		;Make IIF1 in output

		rets




******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,8
	.sect	".cinit"
	.word	8,CONST
	.word	000800000h		; Used to make data clock high
	.word	0ff7fffffh		; Used to make data clock low
	.word	000400000h		; Used to disable data buffer
	.word	0bf7fc008h
	.word	0ffffffbfh		; Used to and data of 1 into iif
	.word	05h			; Write header (bit reversed)
	.word	085h			; Read header (bit reversed)
	.word	0ffffffdfh		; Used to make iiof1 an input

WRITE_HEADER	.set	5
READ_HEADER	.set	6

	.end
