.strlen:
	LSH	-2,      R0,   AR0	
	ADDI	IR0,  AR0		
	AND3	3,       R0,   R1		
	LDIu	0,       R0			
	BeqD	len.word_aligned		
	MPYI	8,       R1			
	LDI	0xFF,    R3			
	LSH	R1,    R3			
	LDI	*AR0++(1), R2		
len.loop1:
	TSTB	R3,    R2			
	Beq	R11				
	ADDI	1,       R0			
	LSH	8,       R3			
	Bne	len.loop1			
len.word_aligned:
	LDHI	0x0101,  R1			
	OR	0x0101,  R1			
	LDHI	0x8080,  R3			
	OR	0x8080,  R3			
	LDI	*AR0++(1), R2		
len.loop2:
	SUBI3	R1,    R2, R10		
	ANDN	R2,    R10			
	AND	R3,    R10			
	BeqD	len.loop2			
	LDIeq	*AR0++(1), R2		
	ADDI	4,       R0			
	NOP					
	LB0	R2,    R1			
	BeqD	R11				
	SUBI	4,       R0			
	LB1	R2,    R1			
	NOP					
	BeqD	R11				
	ADDI	1,       R0			
	LB2	R2,    R1			
	LDIne	2,       R1			
	BuD	R11				
	LDIeq	1,       R1			
	ADDI	R1,    R0			
	NOP					
.strcmp:
	CMPI	R0,   R1			
	BeqD	R11				
	LDIeq	0,      R0			
	LSH	-2,     R0,   AR0		
	ADDI	IR0, AR0			
	AND	3, R0				
	LSH	-2,     R1,   AR1		
	ADDI	IR0, AR1			
	AND	3, R1				
	ADDI3	R0,   R1, R2		
	Beq	cmp.word_aligned		
	CMPI	R0,   R1			
	Bne	cmp.not_even_close		
	LDI	*AR0++(1), R2		
	|| LDI	*AR1++(1), R3		
	LDI	0xFF,   R1	 		
	MPYI	8,      R0			
	LSH	R0,   R1			
	AND3	R2,   R1, R0		
	AND3	R3,   R1, R10		
	SUBI	R10,   R0			
cmp.loop1:
	Bne	R11				
	CMPI	0, R10				
	Beq	R11				
	LSH	8, R1				
	BneD	cmp.loop1			
	AND3	R2,   R1, R0		
	AND3	R3,   R1, R10		
	SUBI	R10,   R0			
cmp.word_aligned:
	LDHI	0x0101, R0			
	OR	0x0101, R0			
	LDHI	0x8080, R1			
	OR	0x8080, R1			
	LDI	*AR0++(1), R2		
	|| LDI	*AR1++(1), R3		
	CMPI	R2,   R3			
cmp.loop2:
	BneD	cmp.mismatch			
	SUBI3	R0,   R2, R10		
	ANDN	R2,   R10			
	AND	R1,   R10			
	BeqD	cmp.loop2			
	LDIeq	*AR0++(1), R2		
	LDIeq	*AR1++(1), R3		
	CMPI	R2,   R3			
	BeqD	R11				
	LB0	R2,   R0			
	LB0	R3,   R1			
	SUBI	R1,   R0			
	BneD	R11				
	LB1	R2,   R6			
	LB1	R3,   R10			
	CMPI	0,      R1			
	Beq	R11				
	SUBI3	R10,   R6, R0		
	BneD	R11				
	LB2	R2,   R1			
	LB2	R3,   R10			
	CMPI	0,      R6			
	Beq	R11				
	SUBI3	R10,   R1, R0		
	BneD	R11				
	LB3	R3,   R2			
	LDIeq	0,      R3			
	LDIne	1,      R3			
	BuD	R11				
	LDI	R3,   R0			
	CMPI	0,      R1			
	LDIeq	0,      R0			
cmp.mismatch:
	LB0	R2,   R0			
	LB0	R3,   R1			
	SUBI	R1,   R0			
	BneD	R11				
	LB1	R2,   R6			
	LB1	R3,   R10			
	CMPI	0,      R1			
	Beq	R11				
	SUBI3	R10,   R6, R0		
	BneD	R11				
	LB2	R2,   R1			
	LB2	R3,   R10			
	CMPI	0,      R6			
	Beq	R11				
	SUBI3	R10,   R1, R0		
	BneD	R11				
	LB3	R2,   R6			
	LB3	R3,   R1			
	SUBI3	R1,   R6, R2		
	BuD	R11				
	CMPI	0,      R10			
	LDIne	R2,   R0		
	NOP					
cmp.not_even_close:
	LDI	*AR0++(1), R2		
	|| LDI	*AR1++(1), R3		
	LDI	0xFF,    R10			
	MPYI	8,       R0			
	LSH	R0,    R10			
	NEGI	R0,    R7			
	LDI	0xFF,    R6			
	MPYI	8,       R1			
	LSH	R1,    R6			
	NEGI	R1,    AR2		
cmp.loop3:
	AND3	R2,    R10, R0		
	LSH	R7,    R0			
	AND3	R3,    R6, R1		
	LSH	AR2, R1			
	SUBI	R1,    R0			
	Bne	R11				
	CMPI	0,       R1			
	Beq	R11				
	LSH	8,       R10			
	BneD	cmp.over1			
	LDIeq	0xFF,    R10			
	LDIeq	 8,      R7			
	ADDI	-8,      R7			
	LDI	*AR0++(1), R2		
cmp.over1:
	LSH	8,       R6			
	BneD	cmp.loop3			
	LDIeq	0xFF,    R6			
	LDIeq	 8,      AR2		
	ADDI	-8,      AR2		
	BuD	cmp.loop3			
	LDI	*AR1++(1), R3		
	NOP					
	NOP					
