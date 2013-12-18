FP                .SET    AR3

		  .globl  _o_crdy
		  .bss    CONST, 1
		  .sect   ".cinit"
		  .word   1, CONST
		  .word   00100040h

		  .text

_o_crdy:
		  PUSH   FP
		  LDI    SP, FP

		  LDI    @CONST, R0
		  LDA    5, IR0
		  LDI    4, R1
		  LSH    R1, *-FP(IR0), AR0
		  ADDI   R0, AR0            ; the addr. of CPCRx

		  LDI    01E0h, R0          ; the CPCRx mask pattern
		  AND    *AR0,R0,R1         ; get the OUTPUT LEVEL
		  CMPI   R0, R1
		  BNE    L1                 ; OCRDY=1
		  LDI    0, R0              ; output FIFO is full


L1:		  LDI    *-FP(1), R1
		  BD     R1
		  LDA    *FP, FP
		  NOP
		  SUBI   2, SP

		  .END

