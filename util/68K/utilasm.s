	.setjmp:
	setjmp:
		patchinstr(PATCHC40MASK24ADD,
			shift(-2, codestub(.SaveCPUState)),
			br	0)
	.longjmp:
	longjmp:
		ldi	R0, AR5
	lsh	-2, AR5
	addi	IR0, AR5
		sti	R1, *+AR5(10)
		patchinstr(PATCHC40MASK24ADD,
			shift(-2,codestub(.RestoreCPUState)),
			br	0)
		align
	__procname_._ProcExit:
		byte	"._ProcExit", 0
		align
			word 	0xff000000 | - __procname_._ProcExit * 4
	._ProcExit:
		ldi	DP, R0
		patchinstr(PATCHC40MASK24ADD,
			shift(-2, labelref(.__ProcExit)),
			br	0)
	._grab_module:
	_grab_module:
	LDHI	0x60f160f1 >> 16,		R10		
	OR	0x60f160f1 & 0xffff,	R10		
	LDI	R0,			AR5		
gm_loop1:
	 CMPI	*--AR5(1),		R10		
	Bne	gm_loop1				
	LDI	AR5,			R0	
	LSH3	-2,	R1,		AR0		
	ADDI	IR0,			AR0		
	LDI	(60 / 4) - 1,RC		
	RPTB	gm_loop2				
	 LDI	*AR5++(1),		R1		
gm_loop2:
	 STI	R1,			*AR0++(1)	
	Bu	R11					
	._copy_module:
	_copy_module:
	LDI	R0,		AR5				
	LDI	R1,		AR0				
	LDI	*+AR5( 1 ),	RC		
	LSH	-2,		RC				
	SUBI	1,		RC				
	RPTB	cm_loop1					
	 LDI	*AR5++(1),	R0				
cm_loop1:			
	 STI	R0,		*AR0++(1)			
	Bu	R11						
	._init_module:
	_init_module:
	LDI	R11,		AR3				
	LDI	R0,		AR1				
	LDI	R1,		R0				
	LDI	*++AR1( 13 ),	R1		
	Beq	R11						
im_loop:
	LSH	-2,		R1				
	ADDI	R1,		AR1				
	ADDI3	1,		AR1,	AR5		
	LAJu	AR5						
	   NOP
	   NOP
	   NOP
	LDI	*AR1,	R1				
	Bne	im_loop						
	Bu	AR3						
