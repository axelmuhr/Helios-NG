	.text

TOP	ldi	@ADDR,ar0
	ldi	@COUNT,rc
	rptb	END
	ldi	*ar0,r0
	addi	1,r0
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

END	sti	r0,*ar0++
	b	TOP

ADDR	.word	08d000000h
COUNT	.word	64

	.end
