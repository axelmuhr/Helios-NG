LENGTH	.set	1030

	.text

	ldp	BLOCK_SIZE

TOP	ldi	@DRAM,ar0
	rpts	@BLOCK_SIZE
	sti	ar0,*ar0++

	ldi	@DRAM,ar0
	ldi	@ARRY_ADDR,ar1
	ldi	@BLOCK_SIZE,rc
	ldi	*ar0++,r0
	rpts	@BLOCK_SIZE
	ldi	*ar0++,r0
||	sti	r0,*ar1++
	sti	r0,*ar1++


	ldi	@DRAM,ar0
	addi	1,ar0
	ldi	@ARRY_ADDR,ar1
	ldi	@BLOCK_SIZE,rc
	rptb	CHECK
	ldi	*ar1++,r0
	cmpi	r0,ar0
	bne	OOPS
CHECK	addi	1,ar0

	b	TOP


OOPS	ldi	0beefh,r7
	b	OOPS

BLOCK_SIZE	.word	LENGTH - 2
DRAM		.word	8d000000h
READBACK	.space	LENGTH
ARRY_ADDR	.word	READBACK

	.end
