	.text

	ldp	ADDR
	ldi	*+ar0(1),r0	;Reset the FIFO
	ldi	@ADDR,ar0
	ldi	@DATA+0,r0
	sti	r0,*+ar0(2)
	ldi	@DATA+1,r0
	sti	r0,*+ar0(1)
	ldi	@DATA+2,r0
	sti	r0,*+ar0(0)

	ldi	@DRAM,ar1
	ldi	@LENGTH,rc
	ldi	@COMP,r1
	rptb	END
HERE	tstb	*+ar0(2),r1
	bz	HERE
	ldi	*+ar0(0),r2
END	sti	r2,*ar1++

STOP	b	STOP
	


ADDR	.word	0c0040020h
DATA	.word	20000h
	.word	100h
	.word	0
COMP	.word	100h
DRAM	.word	08d000000h
LENGTH	.word	100000h - 1

	.end
