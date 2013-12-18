FP                .SET    AR3
		  .globl  _i_crdy
		  .bss    CONST, 1
		  .sect   ".cinit"
		  .word   1, CONST
		  .word   00100040h

		  .text

_i_crdy:
		  PUSH   FP
		  LDI    SP, FP

		  LDI    @CONST, R0
		  LDA    5, IR0
		  LDI    4, R1
		  LSH    R1, *-FP(IR0), AR0
		  ADDI   R0, AR0            ; the addr. of CPCRx

		  LDI    01E00h, R0          ; the CPCRx mask pattern
		  AND    *AR0,R0,R1         ; get the INPUT LEVEL
		  CMPI   0, R1
		  BNE    L1                 ; ICRDY=1
		  LDI    0, R0              ; input FIFO is empty


L1:		  LDI    *-FP(1), R1
		  BD     R1
		  LDA    *FP, FP
		  NOP
		  SUBI   2, SP

		  .END

