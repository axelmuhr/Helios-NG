	.globl	_halt

_halt:
	ldi	sp,ar1
	ldi	*-ar1(1),sp
	ldp	DeadLoop
	ldi	@DeadLoop,ar0
	push	ar0
	reti

HERE	b	HERE

DeadLoop	.word	HERE

	.end