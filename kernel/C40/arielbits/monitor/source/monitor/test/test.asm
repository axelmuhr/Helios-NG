	.text

	ldi	@G_SRAM,ar0
	ldi	@G_SIZE,rc
	rptb	COPY
	sti	ar0,*ar0
COPY	addi	1,ar0

	ldi	@L_SRAM,ar0
	ldi	@G_SRAM,ar1
	ldi	@L_SIZE,r0
	subi	50,r0
	addi	50,ar1
	addi	50,ar0
TOP	ldi	*ar1,r1
	sti	r1,*ar0
	cmpi	ar1,r1
	bne	SHIT
	addi	1,ar0
	addi	1,ar1
	subi	1,r1
	bnz	TOP

SHIT	b	SHIT


G_SRAM	.word	08d000000h
G_SIZE	.word	04000h
L_SRAM	.word	040000000h
L_SIZE		.word	04000h

	.end
		
	