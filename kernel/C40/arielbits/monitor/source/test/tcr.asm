		.sect	"EXEC"
START		ldp	CTR_VALS

		ldi	@BCRs,ar0
		ldi	@CTR_VALS,r0
		sti	r0,*ar0
		ldi	@CTR_VALS+1,r0
		sti	r0,*+ar0(4)

		ldi	@G_BUS_ADDR,ar0
		ldi	2,iif
		xor	r0,r0
		xor	r2,r2
LOOP		sti	r0,*ar0

		ldi	100,r7
WAIT		subi	1,r7
		bnz	WAIT

		ldi	*ar0,r1
		cmpi	r1,r0
		beq	NO_ERROR
		addi	1,r2

NO_ERROR	addi	1,r0
		b	LOOP


CTR_VALS	.word	1DEA4010H	; Local memory interface control register
		.word	19CC4000H	; Global memory interface control register
BCRs		.word	00100000H
G_BUS_ADDR	.word	80000000H
		.end

