	.text
	ldp	DATA
	ldi	@ADDR,ar0

HERE	rpts	@COUNT
	ldi	*ar0,r0

	b	HERE

DATA
ADDR	.word	08d000000h
COUNT	.word	07FFFFFFFh

	.end