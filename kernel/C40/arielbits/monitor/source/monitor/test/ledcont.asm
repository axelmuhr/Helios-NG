	.global	_LEDon
	.global	_LEDoff


	.text

_LEDoff:
	lda	@CONST+0,ar0
	ldi	sp,ar1
	ldi	*-ar1(1),r0
	and	*ar0,r0
	sti	r0,*ar0
	rets

_LEDon:
	lda	@CONST+0,ar0
	ldi	sp,ar1
	ldi	*-ar1(1),r0
	or	*ar0,r0
	sti	r0,*ar0
	rets

	
	.bss	CONST,1
	.sect	".cinit"
	.word	1,CONST
	.word	0BFFD8000H

	.end