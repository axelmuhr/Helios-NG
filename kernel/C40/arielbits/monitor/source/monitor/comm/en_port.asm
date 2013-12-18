FP       .SET   AR3
	.globl  _en_port
	.bss    CONST,1
	.sect   ".cinit"
	.word   1,CONST
	.word   00000004h            ; mask pattern

	.text

_en_port:
	 PUSH  FP
	 LDA   SP, FP

	ldi	*-FP(3),r0
	 LDI   @CONST, R1
	 LSH   R0, R1

	 LDI   2, R2
	 LDA   2, IR0
	 LSH   R2, *-FP(IR0), R0         ; find the shift count
	 LSH   R0, R1
	 OR    R1, IIE                ; enable the interrupts

	 LDI   *-FP(1), R1            ; returning
	 BD    R1
	 LDA   *FP, FP
	 NOP
	 SUBI  2, SP

	 .END


