		.sect	"EXEC"
START		ldp	CTR_VALS

		;Setup BCR's
		ldi	@BCRs,ar0
		ldi	@CTR_VALS,r0
		sti	r0,*ar0
		ldi	@CTR_VALS+1,r0
		sti	r0,*+ar0(4)

		ldi	iif,r7
		ldi	2,iif		; Set IOF[0] low so that we access to
					; the TCR.
		ldi	@BCRs,ar0	; Remove external wait state dependency
		ldi	*ar0,r0
		or	10h,r0
		sti	r0,*ar0
		ldi	@TCR_LWordSVProgB0,r0
		ldi     @G_BUS_ADDR,ar0
		sti	r0,*ar0		; Save it in the TCR
		ldi	@BCRs,ar0	; Add external wait state dependency
		ldi	*ar0,r0
		ldi	@BCR_MASK,r1
		and	r1,r0
		sti	r0,*ar0
		ldi	r7,iif

		ldi	@EPROM_SPACE,ar0	; Take VAC out of reset
		ash	-2,ar0			; Shift address right by 2 to
		ldi	@EPROM_MASK,r0
		and	r0,ar0
		ldi	*ar0,r0			; align it with VAC address.

HERE		b	HERE


CTR_VALS	.word	1DEA4000H	; Global memory interface control register
		.word	19CC4000H	; Local memory interface control register
BCRs		.word	00100000H
TCR_LWordSVProgB0.word	00000018H	; TCR value for Long Word Supervisor
					; Program BYTE 0 transfers
G_BUS_ADDR	.word	80000000H
BCR_MASK	.word	0FFFFFFEFH
EPROM_SPACE	.word	0FF000000H
EPROM_MASK	.word	0BFFFFFFFH
		.end

