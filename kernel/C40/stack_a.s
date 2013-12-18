	.__stack_free:
	__stack_free:
   SUBI3     IR0,	     AR6,	R10 
   LDI	   *+AR6( 3 ), IR1	     
   LDI	   *+AR6( 2  ), R11	     
   LDI 	   *+AR6( 4 ), AR6	     
   STI       R11,	 *--AR6(1)        
patchinstr( PATCHC40MASK24ADD, shift( -2, codestub( .StackFree ) ),
   LAJ       0 )
      STI    R0, *--AR6(1)	        
      LSH3   2,	             R10,    R0	
      NOP					
   LDI	    *AR6++(1),     R0		
   LDI	    *AR6++(1),     R11		
   Bu	 R11
	.__stack_overflow:
	__stack_overflow:
   STI	  DP,		*--AR6(1)
   STI	  BK,		*--AR6(1)
   STI	  R8,		*--AR6(1)
   STI	  R9,		*--AR6(1)
   STI	  R4,	*--AR6(1)	
   STI	  R5,	*--AR6(1)	
   STI	  R0,		*--AR6(1)
   STI	  R1,		*--AR6(1)
   STI	  R2,		*--AR6(1)
   STI	  R3,		*--AR6(1)
   STI	  R11,   	*--AR6(1)
   STI	  RS, 	*--AR6(1)
   STI	  RC,	*--AR6(1)
   LDI	  RE,	   R0
patchinstr( PATCHC40MASK24ADD, shift( -2, CODESTUB( .StackMalloc ) ),
   LAJ	  0 )
      NOP
      NOP
      NOP
   ADDI	  .__stack_free,   R11
   LDI	  R11,		   AR0
   LSH3	 -2,		   R0,	AR5
   ADDI	  IR0,	   AR5
   LDI	 *AR6++(1),	   RC
   LDI	 *AR6++(1),	   RS
   LDI	 *AR6++(1),	   R11
   LDI	 *AR6++(1),	   R3
   LDI	 *AR6++(1),	   R2
   LDI	 *AR6++(1),	   R1
   LDI	 *AR6++(1),	   R0
   LDI	 *AR6++(1),	   R5
   LDI	 *AR6++(1),	   R4
   LDI	 *AR6++(1),	   R9
   LDI	 *AR6++(1),	   R8
   LDI	 *AR6++(1),	   BK
   LDI	 *AR6++(1),	   DP
   ADDI3  RS,	   AR6,    R10	
   STI	  R10,		 *+AR5(4)	
   STI	  IR1,	 *+AR5(3)	
   STI	  RC,	 *+AR5(2)
   SUBI3  RS,          AR5,      AR6   
   LDI	*+AR5(5),  RE		
   SUBI3  RE,	   AR5,	IR1	
   ADDI	  384 >> 2,     IR1		
   BuD	  R11
      LDI AR0,	   R11
      NOP
      NOP
	._stack_size:
	_stack_size:
	SUBI3	IR1,	AR6,	R0
	Bu	R11
	.Accelerate:
	Accelerate:
	LSH3	-2,		R0,		AR5		
	ADDI	IR0,		AR5				
	LDI	*+AR5( 0 ),	AR0		
	LDI	*+AR5( 1 ),	R0		
	SUBI	6,	R0				
	ADDI3	R0,		AR0,	AR5		
	STI	R0,		*+AR5( 5 )		
	STI	R11,		*+AR5( 2 )		
	STI	AR6,		*+AR5( 4 )		
	STI	IR1,		*+AR5( 3 )		
	STIK	0,		*+AR5( 1 )		
	STIK	0,		*+AR5( 0 )		
	LDI	R1,		R10				
	STI	DP,		*--AR5(1)			
	LSH3	-2,		R2,		DP		
	Beq	ac_done						
	LDI	R3,		R0				
	SUBI	1,		DP				
	Beq	ac_done						
	LDI	*AR6++(1),	R1				
	SUBI	1,		DP				
	Beq	ac_done						
	LDI	*AR6++(1),	R2				
	SUBI	1,		DP				
	Beq	ac_done						
	LDI	*AR6++(1),	R3				
	SUBI	1,		DP				
	Beq	ac_done						
	SUBI3	1, 		DP,		RC		
	ADDI	DP,		AR6
	RPTB	ac_loop1					
	 LDI	*--AR6(1),	R7			
ac_loop1:
	 STI	R7,		*--AR5(1)			
ac_done:
	LDI	AR0,	IR1				
	ADDI	384 >> 2,	IR1				
	LDI	AR5,		AR6				
	LAJu	R10						
	   NOP
	   NOP
	   NOP
	ADDI      DP,		        AR6			
	LDI      *AR6++( 1 ),		DP			
	LDI	*+AR6( 2 ),	R11			
	LDI	*+AR6( 3 ),	IR1			
	LDI	*+AR6( 4 ),	AR6			
	Bu	R11
