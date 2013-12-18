	.text

	ldp	DATA

TOP	ldi	@ADDR,ar0
	ldi	@SIZE,r0
LOOP	sti	ar0,*ar0++
	subi	1,r0
	bnz	LOOP

	b	TOP

DATA
ADDR		.word	08d080000h
SIZE		.word	0FFFFah


	.end