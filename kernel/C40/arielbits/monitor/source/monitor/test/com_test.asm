	.text

	ldp	DATA

	ldi	@COMM_CONT,ar0
	ldi	@MASK,r1
	ldi	0,r0

TOP	ldi	*ar0,r2
	and	r1,r2
	bz	TOP

	ldi	*+ar0(1),r0
HERE	b	HERE

DATA
COMM_CONT	.word	000100070h
MASK		.word	000001E00h

	.end