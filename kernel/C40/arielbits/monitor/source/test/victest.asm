		.sect	"EXEC"
START		ldp	CTR_VALS

		ldi	@BCRs,ar0
		ldi	@CTR_VALS,r0
		sti	r0,*ar0
		ldi	@CTR_VALS+1,r0
		sti	r0,*+ar0(4)

		ldi	@G_BUS_ADDR,ar0
		ldi	0,iif
		xor	r0,r0
		xor	r1,r1
LOOP		sti	r0,*ar0
		ldi	*ar0,r1
		b	LOOP


CTR_VALS	.word	01DEA4000H	; Local memory interface control register
		.word	019CC4000H	; Global memory interface control register
BCRs		.word	000100000H
G_BUS_ADDR    	.word	0BFFF0000H
		.end

