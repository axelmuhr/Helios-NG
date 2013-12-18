	.text

	ldi	@DRAM_ADDR,ar0
HERE	ldi	*ar0,r0
	sti	r1,*ar0
	b	HERE

DRAM_ADDR	.word	8d000000h

	.end