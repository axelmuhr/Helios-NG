BITS 		.set	31
FP		.set	ar3
	.text

	.globl	_MemTest
_MemTest	PUSH	FP
		LDI	SP,FP
	
;Perform bus capacitance test
		xor		r1,r1		; Load zero and then perform the logical not to get
						; 0xFFFFFFFF into the register
		not		r1
		ldi		0,r2
		ldi		*-FP(2),ar0	; Get test start address
		ldi		*-FP(3),ir0	; Get test length
		subi		1,ir0
		ldi		20,rc		; Perform test 20 times
		rptb		SURGETEST
		sti		r1,*ar0		; Store F's at memory base
		sti		r2,*+ar0(ir0)	; Store 0's at the end of the memory
						; (this causes the maximum change in the
						;  address bus)
		ldi		*ar0,r4		; Try to read back the F's
		cmpi		r4,r1		; Check it
SURGETEST	bne		SURGEFAIL


;WRITE WALKING BITS TEST  DATA TO THE TARGET AREA
	ldi		*-FP(2),ar0	; Load test start address
	ldi		*-FP(3),r1	; Load memory under test length
	subi		1,r1		; RC = Itterations - 1
	sti		r1,*-FP(3)	; Store itterations -1 for other test(s)
WRITE	ldi		BITS,rc		; After 31 shifts, the walking 1 needs to be reset
	ldi		1,r0		; Initialize walking bit
	rptb		WWALK
	sti		r0,*ar0++	; Store test data
WWALK	lsh		1,r0	; Shift walking bit
	subi		BITS+1,r1	; Decrement test count
	bgt		WRITE

;READ WALKING BITS TEST DATA FROM TARGET AREA
	ldi		*-FP(2),ar0	; Load test start address
	ldi		*-FP(3),r1	; Load memory under test length
	ldi		0,r3		; Identifies which test is running in the event of a failure
READ	ldi		BITS,rc		; After 31 shifts, the walking 1 needs to be reset
	ldi		1,r0		; Initialize walking bit
	rptb		RWALK
	ldi		*ar0++,r2	; Load test value
	cmpi		r2,r0		; Check if it's correct
	bne		OOPS		; Exit on failure
RWALK	lsh		1,r0	; Shift walking bit
	subi		BITS+1,r1	; Decrement test count
	bgt		READ


;WRITE ADDRESS STORE TEST DATA TO THE TARGET AREA
	ldi		1,r3
	ldi		*-FP(2),ar0	; Load memory under test start address
	ldi		*-FP(3),rc	; Load memory under test length
	rptb		WADDR
	ldi		ar0,r0		; get next data
WADDR	sti		r0,*ar0++	; Store it
;READ ADDRESS STORE TEST DATA FROM THE TARGET AREA
	ldi		*-FP(2),ar0	; Load memory under test start address
	ldi		*-FP(3),rc	; Load memory under test length
	rptb		RADDR
	ldi		*ar0,r0		; Get next test data
	cmpi		r0,ar0		; Check it
	bne		OOPS		; Exit on error
RADDR	addi		1,ar0	; Else increment test address

	ldi		0,r0		; Indicate success
RETURN	LDI		*-FP(1),R1
	BD		R1
	LDI		*FP,FP
	NOP
	SUBI		2,SP	

OOPS	ldi		*-FP(4),ar4
	subi		1,ar0
	sti		ar0,*+ar4(0)	; Store failure address
	sti		r3,*+ar4(1)	; Store the test ID of the test that fialed
	cmpi		0,r3
	beq		ZERO
	sti		ar0,*+ar4(2)	; Store written value
	sti		r0,*+ar4(3)	; Store read value
	b		RETURN
ZERO	sti		r0,*+ar4(2)	; Store written value
	sti		r2,*+ar4(3)	; Store read value
	b		RETURN


SURGEFAIL	ldi		0,r0		; Indicate error with a return value of 0
		ldi		*-FP(5),ar4
		sti		ar0,*+ar3(0)	; Store failure address
		ldi		2,r3		; ID = 2 indicates capacitance test
		sti		r3,*+ar3(1)	; Store the test ID of the test that failed
		sti		r1,*+ar3(2)	; Store the written value
		sti		r4,*+ar3(3)	; Store the read value
		b		RETURN




	.globl	_DualMem
_DualMem	PUSH	FP
		LDI	SP,FP
		push	r4
		push	ar4

;Perform bus capacitance test
		xor		r1,r1		; Load zero and then perform the logical not to get
						; 0xFFFFFFFF into the register
		not		r1
		ldi		0,r2
		ldi		*-FP(2),ar1	; Get first test start address
		ldi		*-FP(3),ar2	; Get second test start address
		ldi		*-FP(4),ir0	; Get test length
		subi		1,ir0
		ldi		20,rc		; Perform test 20 times
		rptb		DSURGETEST
		ldi		ar1,ar0		; Test the first bank
		sti		r1,*ar0		; Store F's at memory base
		sti		r2,*+ar0(ir0)	; Store 0's at the end of the memory
						; (this causes the maximum change in the
						;  address bus)
		ldi		*ar0,r4		; Try to read back the F's
		cmpi		r4,r1		; Check it
		ldi		ar2,ar0		; Test second bank
		sti		r1,*ar0		; Store F's at memory base
		sti		r2,*+ar0(ir0)	; Store 0's at the end of the memory
						; (this causes the maximum change in the
						;  address bus)
		ldi		*ar0,r4		; Try to read back the F's
		cmpi		r4,r1		; Check it
DSURGETEST	bne		DSURGEFAIL


;WRITE WALKING BITS TEST  DATA TO THE TARGET AREA
	ldi		0,r4		; Indicate which test is running
	ldi		*-FP(2),ar0	; Get first test start address
	ldi		*-FP(3),ar1	; Get second test start address
	ldi		*-FP(4),r1	; Get test length
	subi		1,r1		; RC = Itterations - 1
	sti		r1,*-FP(4)	; Store it for latter tests
DWRITE	ldi		BITS,rc		; Walking bit needs to be initialized every 31 shifts
	ldi		1,r0		; Initialize walking bit
	rptb		DWWALK
	sti		r0,*ar0++	; Store test value into both banks
||	sti		r0,*ar1++
DWWALK	lsh		1,r0	; Shift walking bit
	subi		BITS+1,r1	; Subtract 32 from the test counter
	bgt		DWRITE

;READ WALKING BITS TEST DATA FROM TARGET AREA
	ldi		*-FP(2),ar0	; Load first test start address
	ldi		*-FP(3),ar1	; Load second test start address
	ldi		*-FP(4),r1	; Load test length
DREAD	ldi		BITS,rc		; Walking bit needs to be initialized exery 31 shifts
	ldi		1,r0		; Initialize walking bit
	rptb		DRWALK
	ldi		*ar0++,r2	; Load both test data
||	ldi		*ar1++,r3
	cmpi		r2,r0		; Check the first value
	bne		DOOPS0	; Exit on error
	cmpi		r3,r0		; Check the second value
	bne		DOOPS1	; Exit on error
DRWALK	lsh		1,r0	; Shift the walking bit
	subi		BITS+1,r1	; Subtract 32 from the test counter
	bgt		DREAD


;WRITE ADDRESS STORE TEST DATA TO THE TARGET AREA
	ldi		1,r4		; Indicate which test is running
	ldi		*-FP(2),ar0	; Load the first test strart address
	ldi		*-FP(3),ar1	; Load the second test start address
	ldi		*-FP(4),rc	; Load the test length
	rptb		DWADDR
	ldi		ar0,r0		; Copy first test address to store as test data
	ldi		ar1,r1		; Copy second test address to store as test data
DWADDR	sti		r0,*ar0++	; Store both test data
||	sti		r1,*ar1++
;READ ADDRESS STORE TEST DATA FROM THE TARGET AREA
	ldi		*-FP(2),ar0	; Load first start address
	ldi		*-FP(3),ar1	; Load second start address
	ldi		*-FP(4),rc	; Load test length
	rptb		DRADDR
	ldi		*ar0,r0		; Load test data
||	ldi		*ar1,r1
	cmpi		r0,ar0		; Check first value
	bne		DOOPS0	; Exit on error
	cmpi		r1,ar1		; Check second value
	bne		DOOPS1	; Exit on error
	addi		1,ar1		; Increment first test address
DRADDR	addi		1,ar0	; Increment second test address

	ldi		1,r0		; Indicate success with a return value of 1
	pop		ar4
	pop		r4
DRETURN	LDI		*-FP(1),R1
	BD		R1
	LDI		*FP,FP
	nop
	SUBI		2,SP	

DOOPS0	ldi	0,r0		; Indicate error with a return value of 0
		ldi	*-FP(5),ar4
		sti	ar0,*+ar3(0)	; Store failure address
		sti	r4,*+ar3(1)	; Store the test ID of the test that failed
		cmpi	0,r3		; Which test failed ?
		bne	DZERO0
		sti	r0,*+ar3(2)	; Store the written value
		sti	r2,*+ar3(3)	; Store the read value
		b	DRETURN
DZERO0	sti	ar0,*+ar3(2)	; Store the written value
		sti	r0,*+ar3(3)	; Store the read value
		b		DRETURN

DOOPS1	ldi	*-FP(5),ar4
		sti	ar1,*+ar4(0)	; Store failure address
		sti	r4,*+ar4(1)	; Store the test ID of the test that failed
		cmpi	0,r4		; Which test failed ?
		bne	DZERO1
		sti	r0,*+ar4(2)	; Store the written value
		sti	r3,*+ar4(3)	; Store the read value
		b	DRETURN
DZERO1	sti	ar1,*+ar4(2)	; Store the written value
		sti	r1,*+ar4(3)	; Store the read value
		b		DRETURN

DSURGEFAIL	ldi	0,r0		; Indicate error with a return value of 0
		ldi	*-FP(5),ar4
		sti	ar0,*+ar3(0)	; Store failure address
		ldi	2,r3		; ID = 2 indicates capacitance test
		sti	r3,*+ar3(1)	; Store the test ID of the test that failed
		sti	r1,*+ar3(2)	; Store the written value
		sti	r4,*+ar3(3)	; Store the read value
		b	DRETURN

	.end