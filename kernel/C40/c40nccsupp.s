	.__stack_overflow_1:
	__stack_overflow_1:
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(__Task_)),
		ldi	*+AR4(0), AR5)
	lsh	-2, AR5
	addi	IR0, AR5
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datasymb(__Task_)),
		ldi	*+AR5(0), AR5)
	ldi	AR5, R0
	ldi	7, R1			
	patchinstr (PATCHC40MASK24ADD,
		shift(-2, labelref(.CallException)),
		br	0)
	.memset:
	memset:
	CMPI	 0,	  R2			
	Beq	 R11				
	LSH	-2,       R0,     AR0	
	ADDI	 IR0,  AR0		
	MB1	 R1,    R1			
	AND3	 3,       R0,     R10	
	BeqD	 set.word_aligned		
	   LDIne	 *AR0,  R6 	
	   MH1	 R1,    R1			
	   CMPI	 2,       R10			
	Beq	 set.byte_selector_2		
	Bgt	 set.byte_selector_3		
	SUBI	 1,       R2			
	Bn	 R11				
	MB1	 R1,    R6			
set.byte_selector_2:
	SUBI	 1,       R2			
	Bn	 set.completed			
	MB2	 R1,    R6			
set.byte_selector_3:
	SUBI	 1,       R2			
	Bn	 set.completed			
	MB3	 R1,    R6			
	STI	 R6,  *AR0++(1)		
set.word_aligned:				
	LSH	-2,       R2,     R6	
	BeqD	 set.almost_finished		
	   SUBI3 1,       R6,    RC		
	   AND	 3,       R2			
	   LDHI	 0xFF00,  R6			
	RPTB	 set.block1			
set.block1:
	STI	 R1,   *AR0++(1)		
set.almost_finished:
	BeqD	 R11				
	   SUBI	 3,       R2			
	   MPYI	 8,       R2			
	   ASH	 R2,    R6			
	LDI	*AR0, R10			
	AND	 R6,   R10			
	BuD	 R11				
	   ANDN	 R6,   R1			
	   OR	 R1,    R10			
	   STI	 R10,   *AR0		
set.completed:
	STI	 R6,  *AR0		
	Bu	 R11				
	.memcpy:
	memcpy:
	CMPI	 0,	  R2			
	Beq	 R11				
	LSH3	-2,       R0,         AR0	
	XOR3	 R0,    R1,         R10	
	AND	 3,       R10			
	BneD	 mcpy.not_aligned		
	   LSH	-2,       R1,         AR1	
	   ADDI	 IR0,  AR0		
	   ADDI	 IR0,  AR1		
	AND	 3,       R1			
	Beq	 mcpy.word_aligned		
	SUBI	 3,       R1			
	ADDI	 R1,    R2			
	SUBI	 1,	  R2			
	Blt	 mcpy.very_small		
	MPYI	 8,       R1			
	LDHI	 0xFF00,  R6			
	ASH	 R1,    R6			
	AND3	 R6,  *AR1++(1), R1	
	ANDN3	 R6,  *AR0,      R10	
	OR	 R1,    R10			
	STI	 R10,   *AR0++(1)		
mcpy.word_aligned:
	LSH3	-2,       R2,         RC	
	Beq	 mcpy.no_more_words		
	LDIne	*AR1++(1),          R1	
	SUBI	 2,       RC		
	Blt	 mcpy.only_one_word		
	RPTB	 mcpy.block1			
mcpy.block1:
	 STI	 R1,   *AR0++(1)		
	 || LDI	*AR1++(1),          R1	
mcpy.only_one_word:
	STI	 R1,   *AR0++(1)		
mcpy.no_more_words:
	AND	 3,       R2			
	Beq	 R11				
	LDHI	 0xFF00,  R6			
	SUBI	 3,       R2			
	MPYI	 8,       R2			
	ASH	 R2,    R6			
	AND	 R6,  *AR0,      R10	
	BuD	 R11				
	   ANDN	 R6,  *AR1,      R1	
	   OR	 R1,    R10			
	   STI	 R10,   *AR0		
mcpy.not_aligned:
	AND3	 3,       R0,         R10	
	AND	 3,       R1,         R6	
	SUBI3	 R6,   R10,         R7	
	MPYI	 8,       R7			
	LDI     *AR1++(1),          RC	
	LDI	 0xFF,    R1			
	MPYI	 8,       R10			
	LSH	 R10,    R1			
	MPYI3	 8,       R6,        RS	
	LDI	 0xFF,    R10			
	LSH	 RS,  R10			
	LDI     *AR0, RS		
mcpy.loop1:
	AND3	 R10,    RC,       R6	
	SUBI	 1,       R2			
	BeqD	 mcpy.finished			
	   LSH	 R7,   R6			
	   ANDN	 R1,    RS		
	   OR	 R6,   RS		
	LSH	 8,       R10			
	Bne	 mcpy.over			
	LDI     *AR1++(1),          RC	
	BuD	 mcpy.loop1			
	   LDI	 0xFF,    R10			
	   XOR	 0xE0,    R7			
mcpy.over:
	   LSH	 8,       R1			
	Bne	 mcpy.loop1			
	LDI	 0xFF,    R1			
	BuD	 mcpy.loop1			
	   STI	 RS, *AR0++(1)		
	   LDI  *AR0, RS		
	   XOR	 0xE0,    R7			
mcpy.finished:
	STI	 RS, *AR0		
	Bu	 R11				
mcpy.very_small:
	LDHI	 0xFF00,  R6			
	CMPI	 R1,	  R2			
	LDIne	-8,	  R10			
	LDIeq	 0,	  R10			
	ASH	 R10,	  R6			
	MPYI	 8,	  R2			
	LSH	 R2,	  R6			
	AND3	 R6,  *AR1++(1), R1	
	BuD	 R11				
	   ANDN3 R6,  *AR0,      R10	
	   OR	 R1,    R10			
	   STI	 R10,   *AR0++(1)		
	._backtrace:
	_backtrace:
	LDI	   R1,	  AR0		
	LDIeq	   AR7,	  AR0		
	LSH	   -2, R0,	  AR5		
	ldhi	   0x2f,	  AR1		
	or	   0xf800,	  AR1		
	cmpi	   AR1,       AR0		
	blo	   b_insane
	LDI	  *AR0,       R0		
	LDI	 *-AR0(1),    AR0		
	cmpi	   AR1,       AR0		
	blo	   b_insane
	ldhi	(((0x60f160f1) >> 16) & 0xffff), RS
	or	((0x60f160f1) & 0xffff), RS
	ldhi	   1,		      AR1		
b_loop1:						
	 LDI	  *AR0--(1),     R1		
         CMPI	   R1,	      RS		
	 Beq	   b_got_module				
	 ASH	  -24,		      R1		
	 CMPI	  -1,		      R1		
	DBne	   AR1,	      b_loop1		
	BneD	   R11					
	   LDIne   0,		      R0	
	   LBU0	*++AR0(1),        R1		
	   LSH	  -2,		      R1		
	SUBI3      2,		      R1,	RC	
	BltD	   b_over				
	   SUBI	   R1,	      AR0		
	   LDI	  *AR0++(1),      R1		
b_copy_name:						
	   ADDI	   IR0,	      AR5		
	RPTB	   b_loop2				
b_loop2:
	 STI	   R1,             *AR5++(1)	
	 || LDI	  *AR0++(1),      R1		
b_over:
	STI	   R1,             *AR5		
	Bu	   R11					
b_got_module:						
	LDI	   8 - 1,	      RC		
	BuD	   b_copy_name				
	  ADDI	   2 + 1,   AR0		
	ldhi	(((0x203a6e69) >> 16) & 0xffff), R1
	or	((0x203a6e69) & 0xffff), R1
b_insane:
	ldi	   0,		  R0		
	b	   R11
	._DataToFuncConvert:
	_DataToFuncConvert:
.C40WordAddress:			
	BuD	 R11			
	   LSH	-2,	  R0		
	   ADDI	 IR0,  R0		
	   NOP				
	._FuncToDataConvert:
	_FuncToDataConvert:
.C40CAddress:				
	SUBI	IR0,	R0
	LDHI	0xC000,	R10
	BuD	R11
	  AND	R0,	R10
	  LDIne	0,	R0
	  LSH	2,	R0
	._SetAddrBase:
	_SetAddrBase:
	ldi	R0, IR0
	b	R11
	._GetAddrBase:
	_GetAddrBase:
	ldi	IR0, R0
	b	R11
