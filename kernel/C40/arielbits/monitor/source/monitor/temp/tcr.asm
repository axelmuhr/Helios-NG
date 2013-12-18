	.global		_readTCR
	.global		_writeTCR

FP	.set	ar3
_readTCR:
	push	dp
	ldp	DATA

	ldi	@DATA+0,ar0
	ldi	@DATA+1,ar1
	ldi	@DATA+2,r2
	ldi	@DATA+3,r3
	ldi	iif,r1			; Set the bit 1 of iif
	ldi	02,iif

	sti	r2,*ar1
	ldi	*ar0,r0			; Read the TCR
	sti	r3,*ar1

	ldi	r1, iif

	pop	dp

	rets


_writeTCR:
	push	dp
	ldp	DATA

	ldi	@DATA+0,ar0
	ldi	@DATA+1,ar1
	ldi	@DATA+2,r2
	ldi	@DATA+3,r3
	ldi	iif,r1			; Set the bit 1 of iif
	ldi	02,iif

	ldi	sp,ar2
	ldi	*-ar2(2),r0		; Get value to be written
	sti	r2,*ar1			; Set global bus wait states to internal only
	sti	r0,*ar0			; Write TCR
	nop
	nop
	nop
	nop
	sti	r3,*ar1			; Restore global bus wait states to external only

	ldi	r1,iif			; Restore iif

	pop	dp
	rets


	.bss	DATA,4
	.sect	".cinit"
	.word	4,DATA
	.word	80000000h		; 0
	.word	100000h		; 1
	.word	1dea4050h		; 2
	.word	1dea4000h		; 3

	.end
