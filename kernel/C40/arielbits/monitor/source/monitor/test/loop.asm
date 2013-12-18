	.text

	ldp	@DATA
	
HERE	ldi	@ADDR,ar0

	
	rpts	@LENGTH
	ldi	*ar0++,r0
	b	HERE

DATA
ADDR	.word	08d000000H
LENGTH	.word	0FFFFFh

	.end