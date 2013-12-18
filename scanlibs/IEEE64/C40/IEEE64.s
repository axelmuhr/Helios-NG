        align
        module  -1
.ModStart:
        word    0x60f160f1
		word modsize
	blkb    31, "IEEE64"
	byte 0
	word	modnum
	word	1
		word	datasymb(.MaxData)
        init
	word	codesymb(.MaxCodeP)
IEEEOverflow:
	BuD	  R11				
	   LDI	 -1,	R1			
	   LSH3  -1,	R1,	R0		
	   NOP					
.FTSTD:
	BuD		R11			
	   CMPI	 0,	R0			
	   LDIn	-1,	R0		
	   LDIp	 1,	R0		
._FNEGD:
	CMPI	 0,		R0		
	BuD	 R11				
	   LDHI	 0x8000,	R10		
	   LDIeq 0,		R10		
	   XOR	 R10,		R0		
.FCMPD:
	CMPI	 0,		R0		
	BnnD	 LeftP				
	   LDHI	 0x7FFF,	R10		
	   OR	 0xFFFF,	R10		
	   LDI	-1,		R6		
	XOR	 R10,		R0		
	NOT	 R1				
LeftP:
	CMPI	 0,		R2		
	LDInn	 0,		R10		
	LDInn	 0,		R6		
	XOR	 R10,		R2		
	BuD	 R11				
	   XOR	 R6,		R3		
	   SUBI	 R3,		R1		
	   SUBB	 R2,		R0		
.FDTOU:
	LSH3	 -20,		R0,	R10	
	CMPI	  0x800,	R10		
	BhsD	  R11				
	   LDIhs  0,		R0	
	   SUBI   0x3FF,	R10		
	   LDIn   0,		R0	
	Bn	  R11				
	CMPI	  32,		R10		
	LDInc	 -1,		R10		
	Bnc	  R11				
	LSH	  12,		R0		
	LSH	 -12,		R0		
	LDHI	  0x10,		R6		
	OR	  R6,	R0		
	SUBI	  20,		R10		
	LSH	  R10,		R0		
	BuD	  R11				
	   SUBI   32,		R10		
	   LSH	  R10,		R1		
	   OR	  R1,		R0		
.FDTOI:
	LDI	   R11,	AR0		
	LAJ	  .FDTOU			
	   LDI	   R0,	R2		
	   LDHI    0x8000,	R7		
	   ANDN    R7,	R0		
        CMPI	   0,		R2		
	Bnn	   Positive			
	BuD	   AR0			
	    NEGI   R0				
	    LDIgt  0,		R0		
	    OR	   R7,	R0		
Positive:
	BuD	   AR0			
	    CMPI   0,		R0		
	    LDIlt -1,		R0		
	    ANDN   R7,	R0		
._FUTOD:
	LDI	 R0,		R1		
_FUTOD1:
	Bz	 R11				
	FLOAT	 R0,		R10		
_FUTOD2:
	STF	 R10,		*--AR6(1)	
	LDI	*AR6++(1),	R10		
	LSH	-24,		R10		
        LDI 	 R10,		R7		
        SUBRI    52,		R10		
        CMPI     32,		R10		
        BloD	 SmallShift			
	   LDI	 R7,		R6		
	   ADDI	 0x3fE,		R6		
           LSH   20,		R6		
	LDI	 R1,		R0		
        LDI      0,		R1		
	BuD	 R11				
           SUBI  32,		R10		
	   LSH	 R10,		R0      	
	   ADDI	 R6,		R0      	
SmallShift:
	LSH	 R10,		R0		
	SUBI3	 32,		R10,	R7	
	LSH3	 R7,		R1,	R7	
	BuD	 R11				
	   OR	 R7,		R0		
	   LSH	 R10,		R1      	
	   ADDI	 R6,		R0      	
._FITOD:
	LDI	  R0,		R1		
	Bge	  _FUTOD1			
	LDI	  R11,		RS		
	LAJ	  _FUTOD2			
	   NEGI   R0				
	   LDI	  R0,		R1		
	   FLOAT  R0,		R10		
	BuD	  RS			
	   LDHI   0x8000,	R10		
	   XOR	  R10,	R0			
	   NOP					
._FSUBD:
	CMPI	0,	R2		
	Beq	R11			
	LDHI	0x8000,	R10		
	XOR	R10,	R2		
._FADDD:
	ADDI	 R1,		R1		
	ADDC	 R0,		R0		
	ADDC	 0,		R1		
	ADDI	 R3,		R3		
	ADDC	 R2,		R2		
	ADDC	 0,		R3		
	SUBI3	 R3,		R1,	R10	
	SUBB3	 R2,		R0,	R10	
	Bnc	 NoSwap				
	LDI	 R1,		R10		
	LDI	 R3,		R1		
	LDI	 R10,		R3		
	LDI	 R0,		R10		
	LDI	 R2,		R0		
	LDI	 R10,		R2		
NoSwap:
	LSH3	-16,		R2,	R10	
	AND	 0xFFE0,	R10		
	Bz	 ReturnOther			
	XOR3	 R1,		R3,	RC	
	AND	 1,		RC		
	LSH	 1,		RC		
	ANDN	 1,		R3		
	LSH	-1,		R1		
	RORC	 RC				
	LSH	 1,		R1		
	LSH3	-16,		R0,	R7	
	AND	 0xFFE0,	R7		
	LHU1	 R0,		RS	        
	AND	 0x1F,		RS		
	OR	 0x20,		RS		
	MH1	 RS,	R0		
	LHU1	 R2,		RS	        
	AND	 0x1F,		RS		
	OR	 0x20,		RS		
	MH1	 RS,	R2		
	LSH3	-1,		R7,	RE	
	SUBI	 R10,		R7		
        Bz       Aligned			
	LSH	-5,		R7		
	CMPI	 53,		R7		
        Bnc      FADD_Normalised		
        CMPI     32,		R7		
        Bc       ResidualShifts			
	LDI	 R2,		R3	        
	LDI	 0,		R2		
	SUBI	 32,		R7		
        Bz	 Aligned			
ResidualShifts:
	LHU0	 R2,		R10		
	NEGI	 R7				
	LSH	 R7,		R2		
	LSH	 R7,		R3		
	ADDI	 32,		R7		
	LSH	 R7,		R10		
	OR	 R10,		R3		
Aligned:
	TSTB	 1,		RC		
        Bnz      Difference			
	ADDI	 R3,		R1		
	ADDC	 R2,		R0		
	LDHI	 0x40,		RS		
	TSTB	 RS,	R0		
	Bz	 FADD_Round			
	LSH	-1,		R0		
	RORC	 R1				
	ADDI	 0x10,		RE		
	CMPI	 0x7FFF,	RE		
	Bhi	 IEEEOverflow			
FADD_Round:
	ADDI	 1,		R1		
	ADDC	 0,		R0		
	TSTB	 RS,	R0		
	Bz	 FADD_Normalised		
        LSH	-1,		R0	  	
        RORC	 R1				
        ADDI	 0x10,		RE		
FADD_Normalised:
        LSH	-1,		R0		
        RORC	 R1				
        LHU1     R0,		RS		
        AND      0x0f,		RS		
        ADDI     RE,	RS		
	BuD	 R11				
	   MH1   RS,	R0		
           ANDN  1,		RC          
           OR    RC,	R0           	
Difference:
        SUBI     R3,		R1		
        SUBB     R2,		R0		
	Beq	 CheckLSW			
        FLOAT    R0,		R6		
        STF	 R6,	   *--AR6(1)	
	BuD      GotCount 		
           LDI	*AR6++(1),	R6		
           LSH  -24,		R6		
           ADDI  32,		R6		
CheckLSW:					
        CMPI	 0,		R1		
	Bz	 R11				
        LDIn     32,		R6		
        Bn       GotCount			
        FLOAT    R1,		R6		
        STF	 R6,	   *--AR6(1)	
        LDI	*AR6++(1),	R6		
        LSH     -24,		R6		
GotCount:
        SUBRI    53,		R6		
        LSH3     4,		R6,	RS	
        SUBI     RS,	RE		
	BleD	 R11				
           LDIle 0,		R0		
           LDIle 0,		R1		
           CMPI  32,		R6		
        Blo      SmallShift1			
        LDI      R1,		R0		
        LDI      0,		R1		
        SUBI     32,		R6		
SmallShift1:
        LSH      R6,		R0      	
        SUBI3    32,		R6,	RS 	
	BuD      FADD_Round			
           LSH3	 RS,	R1,	RS 	
           OR    RS,	R0      	
           LSH   R6,		R1      	
ReturnOther:
	BuD	 R11				
           LSH3 -1,		R1,	R3	
           RORC  R0				
           RORC  R1				
._FMULD:
	XOR3	 R0,  	R2,	RS	
	LSH	-31,		RS		
	LSH	 31,		RS		
	LSH3	-20,		R0,	R10	
	AND	 0x7FF,		R10		
	BzD	 R11				
	   LDIz	 0,		R0		
	   LDIz	 0,		R1		
	   LSH3	-20,		R2,	R6	
	AND	 0x7FF,		R6		
	BzD	 R11				
	   LDIz	 0,		R0		
	   LDIz	 0,		R1		
	   ADDI	 R10,		R6		
	SUBI	 0x3ff,		R6		
	BlsD	 R11				
	   LDIls 0,		R0		
	   LDIls 0,		R1		
	   NOP					
	Bn	 IEEEOverflow			
	LSH	 12,		R0		
	OR	 1,		ST		
	RORC	 R0				
	LSH	-3,		R0		
	LWR3	 R1,		R0		
	LSH	 8,		R1		
	LSH	 12,		R2		
	OR	 1,		ST		
	RORC	 R2				
	LSH	-3,		R2		
	LWR3	 R3,		R2		
	LSH	 8,		R3		
	MPYUHI3	 R2,		R1,	AR5	
	MPYUHI3	 R0,		R3,	R7	
	MPYUHI3	 R2,		R0,	RC	
	MPYI	 R2,		R0		
	ADDI	 AR5,	R7		
	ADDC	 0,		RC		
	ADDI	 R0,		R7		
	ADDC	 0,		RC		
	ADDI3	 1,		R7,	R1	
	ADDC3	 0,		RC,	R0	
	LBU3	 R0,		R7		
	TSTB	 2,		R7		
	Bz	 NoCarry			
	ADDI	 1,		R6		
	LSH	-1,		R0		
	RORC	 R1				
NoCarry:
	LSH	-4,		R1		
	LSH	 32 - 4,	R0,	RE	
	OR	 RE,	R1		
	LSH	-4,		R0		
	LHU1	 R0,		RE		
	TSTB	 0x10,		RE		
	Bnz	 FMULD_Normalised		
	LSH	 1,		R1		
	ROLC	 R0				
	SUBI	 1,		R6		
	BlsD	 R11				
	   LDIls 0,		R0		
	   LDIls 0,		R1		
FMULD_Normalised:
	   CMPI	 0x7FF,		R6		
	Bhi	 IEEEOverflow			
	LSH      12,		R0		
	LSH	 -12,		R0		
	BuD	 R11				
	   LSH	 20,		R6		
	   OR	 R6,		R0		
	   OR	 RS,	R0		
._FDIVD:
	LSH3	-16,		R2,	R6	
	AND	 0x7FF0,	R6		
	Bz	 DivideByZero			
	LSH3	-16,		R0,	AR1	
	AND	 0x7FF0,	AR1		
	Bz	 R11				
	SUBI	 R6,		AR1		
	ADDI	 0x3FF0,	AR1		
	CMPI	 0x7FFF,	AR1		
	Bhi	 IEEEOverflow			
	XOR3	 R0,		R2,	AR5	
	LSH	-31,		AR5		
	LDHI	 0x0F,		R6		
	OR	 0xFFFF,	R6		
	AND	 R6,		R0		
	AND	 R6,		R2		
	ADDI	 1,		R6		
	LDI	 22 - 1,	RC		
	RPTBD	 EndLoop1			
	   OR	 R6,		R0		
	   OR	 R6,		R2		
	   LDI	-1,		R7		
	 SUBI3	 R3,		R1,	R10	
	 SUBB3	 R2,		R0,	R6	
	 LDInc	 R10,		R1		
	 LDInc	 R6,		R0		
	 ADDC	 R7,		R7		
	 ADDI	 R1,		R1		
EndLoop1:
	 ADDC	 R0,		R0		
	LDI	 32 - 1,	RC		
	NOT	 R7				
	RPTBD	 EndLoop2			
	   LSH	 31,		AR5		
	   LHU1	 R7,		AR2		
	   LDI	-1,		AR0		
	 SUBI	 R3,		R1,	R10	
	 SUBB	 R2,		R0,	R6	
	 LDInc	 R10,		R1		
	 LDInc	 R6,		R0		
	 ADDC	 AR0,	AR0		
	 ADDI	 R1,		R1		
EndLoop2:
	 ADDC	 R0,		R0		
	NOT	 AR0			
	TSTB	 0x20,		AR2		
	Bnz	 FDIV_Round			
	SUBI	 0x10,		AR1		
	BleD	 R11				
	   LDIle 0,		R0		
	   LDIle 0,		R1		
	   ADDI	 AR0,	AR0		
	ADDC	 R7,		R7		
FDIV_Round:
	ADDI3	 1,		AR0, R1	
	ADDC3	 0,		R7,   R0	
	Bnc	 FDIV_Normalised		
	RORC	 R0				
	RORC	 R1				
FDIV_Normalised:
	LSH	-1,		R0		
	RORC	 R1				
	LHU1	 R0,		R10		
	AND	 0x0F,		R10		
	BuD	 R11				
	   OR	 AR1,	R10		
	   MH1	 R10,		R0		
	   OR	 AR5,	R0		
DivideByZero:
patchinstr( PATCHC40MASK24ADD, shift( -2, CODESTUB ( .raise ) ),
	LAJ	 0 )				
	   LDI	 2,		R0		
	   NOP					
	   NOP					
._FSTOD:
	CMPI	 0,		R0		  
	BzD	 R11				  
	   LDI	 0,		R1		  
	   LSH3	-23,		R0,	R6	  
	   AND	 0xFF,		R6		  
	ADDI	 0x3FF - 0x7F,	R6		  
	LSH	 32 - 12 + 1,	R6		  
	LSH	 1,		R0		  
	RORC	 R6				  
	LSH3	 32 - 4,	R0,	R1	  
	BuD	 R11				  
	   LSH	 8,		R0		  
 	   LSH	-12,		R0		  
	   OR	 R6,		R0		  
.FDTOS:
	LSH3	-16,		R0,	R2	
	AND	 0x7FF0,	R2		
	SUBI	 0x3FF0 - 0x7F0,R2		
	BleD	 R11				
	   LDIle 0,		R0		
	   CMPI	 0,		R0		
	   LDInn 0,		RS		
	LDIn	 1,		RS		
	CMPI	 0xFF0,		R2		
	Bgt	 FDTOS_Overflow			
	LDHI	 0x1000,	R3		
	ADDI	 R3,		R1		
	BncD	 NoAdjustment			
	   LSH	 19,		R2		
	   LSH	 12,		R0		
 	   LSH	-12,		R0		
	ADDI	 1,		R0		
	LDHI	 0x10,		R3		
	TSTB	 R3,		R0		
	Bz	 NoAdjustment			
	LSH	-1,		R0		
	RORC	 R1				
	LDHI	 0x80,		R3		
	ADDI	 R3,		R2		
	Bn	 FDTOS_Overflow			
NoAdjustment:
	LSH3	 3 - 32,	R1,	R3	
	LSH	 3,		R0		
	OR	 R3,		R0		
	BuD	 R11				
	   OR	 R2,		R0		
	   ROR	 RS				
	   OR	 RS,	R0		
FDTOS_Overflow:
	BuD	 R11				
	   LDI	-1,		R0		
	   SUBI	 1,		RS		
	   LSH	 RS,	R0		
.FNEGD:
	LDI	 R11,	   AR0		
	LAJ	._FNEGD				
	   LSH3	-2,	   R0,   AR5	
	   LDI	 R1,	   R0			
	   LDI	 R2,	   R1			
	BuD	 AR0			
	   STI	 R0,	*++AR5(IR0)	
	   STI	 R1,	 *+AR5(1)		
	   NOP					
.FUTOD:
	LDI	 R11,	   AR0		
	LAJ	._FUTOD				
	   LSH3	-2,	   R0,   AR5	
	   LDI	 R1,	   R0			
	   NOP					
	BuD	 AR0			
	   STI	 R0,	*++AR5(IR0)	
	   STI	 R1,	 *+AR5(1)		
	   NOP					
.FITOD:
	LDI	 R11,	   AR0		
	LAJ	._FITOD				
	   LSH3	-2,	   R0,   AR5	
	   LDI	 R1,	   R0			
	   NOP					
	BuD	 AR0			
	   STI	 R0,	*++AR5(IR0)	
	   STI	 R1,	 *+AR5(1)		
	   NOP	 				
.FSUBD:
	LDI	 R11,	   AR0		
	LSH3	-2,	   R0,   AR5	
	LDI	 R1,	   R0			
	LAJ	._FSUBD				
	   LDI	 R2,	   R1			
	   LDI	 R3,	   R2			
	   LDI	*AR6,    R3			
	BuD	 AR0			
	   STI	 R0,	*++AR5(IR0)	
	   STI	 R1,	 *+AR5(1)		
	   NOP					
.FADDD:
	LDI	 R11,	   AR0		
	LSH3	-2,	   R0,   AR5	
	LDI	 R1,	   R0			
	LAJ	._FADDD				
	   LDI	 R2,	   R1			
	   LDI	 R3,	   R2			
	   LDI	*AR6,	   R3			
	BuD	 AR0			
	   STI	 R0,	*++AR5(IR0)	
	   STI	 R1,	 *+AR5(1)		
	   NOP					
.FMULD:
	LDI	 R11,	   AR0		
	LSH3	-2,	   R0,   AR1	
	LDI	 R1,	   R0			
	LAJ	._FMULD				
	   LDI	 R2,	   R1			
	   LDI	 R3,	   R2			
	   LDI	*AR6,	   R3			
	BuD	 AR0			
	   STI	 R0,	*++AR1(IR0)	
	   STI	 R1,	 *+AR1(1)		
	   NOP					
.FDIVD:
	STI	  R0,	   *--AR6(1)		
	STI	  R11,	   *--AR6(1)		
	LDI	  R1,		R0			
	LAJ	  ._FDIVD				
	   LDI	  R2,		R1			
	   LDI	  R3,		R2			
	   LDI	*+AR6(2),	R3			
	LDI	 *AR6++(1),   R11			
	LSH3	 -2,	      *+AR6(0),   AR5	
	BuD	  R11					
	   STI	  R0,	     *++AR5(IR0)		
	   STI	  R1,	      *+AR5(1)		
	   ADDI	  1,	        AR6			
.FSTOD:
	LDI	 R11,     AR0		
	LAJ	 ._FSTOD			
	   LSH3	-2,	   R0,  AR5	
	   LDI	 R1,	   R0			
	   NOP					
	BuD	 AR0			
	   STI	 R0,	*++AR5(IR0)	
	   STI	 R1,	 *+AR5(1)		
	   NOP					
.FFTOS:
 	ASH3	 -24,	        R0,	R10	
	CMPI	 -127,		R10		
	BleD	  R11				
	   LDIle  0,		R0	
	   LDHI	  0xFF80,	R6		
	   CMPI	  127,		R10		
	BeqD	  R11				
	   LDIeq  R6,	R0	
	   LDHI	  0x0080,	R7		
	   TSTB	  R7,	R0		
	BeqD	  FFTOS_positive		
	   ADDI	  0x7F,		R10		
	   LDI	  0,		R7		
	   ANDN	  R6,	R0		
	LDIeq	  1,		R7		
	LDIeq	  0,		R0		
	ADDI	  R7,	R10		
	NOT	  R0	 			
	ADDI	  1,		R0		
	OR	  0x100,	R10		
FFTOS_positive:
	BuD	  R11				
	   LSH	  23,		R10		
	   ANDN	  R6,	R0	
	   OR	  R10,		R0	
._FFTOD:
	ASH3	-24,	R0,	R10		
	CMPI	-128,	R10			
	BneD	 FFTOD_not_zero			
	   LDI	 0,	R7			
	   ADDI	 0x3FF,	R10			
	   CMPI	 0,	R1			
	LDIeq    0,	R0			
	Beq	 R11				
FFTOD_not_zero:
	LDHI	 0x8000,	R6		
	LSH	 1,	R1			
	LDIc	 R6,	R7			
	Bnc	 FFTOD_positive			
	NOT	 R1				
	ADDI	 2,	R1			
	ADDC	 0,	R10			
FFTOD_positive:
	LSH	 20,	R10			
	LSH3	-12,	R1,	R0		
	LSH	 20,	R1			
	BuD	 R11				
	   OR	 R10,	R0			
	   ANDN	 R6,	R0			
	   OR	 R7,	R0			
.FFTOD:
	LDI	 R11,     AR0		
	LAJ	 ._FFTOD			
	   LSH3	-2,	   R0,  AR5	
	   LDI	 R1,	   R0			
	   LDI	 R2,	   R1			
	BuD	 AR0			
	   STI	 R0,	*++AR5(IR0)	
	   STI	 R1,	 *+AR5(1)		
	   NOP					
.FSTOF:
	BuD	   R11				
	   STI	   R0,	*--AR6(1)	
	   FRIEEE *AR6++(1),	     R0	
	   NOP					
.FDTOF:
	ASH3	 -20,	R0,	R6		
	AND3	  0x08,	ST,	R10		
	AND	  0x7FF,	R6		
	LSH	 -20,		R1		
	LSH3	  12,	R0,	R7		
	OR	  R1,		R7		
	SUBI	  0x3FF,	R6		
	CMPI	  127,		R6		
	Bgt	  FDTOF_Overflow		
	CMPI	 -128,		R6		
	Blt	  FDTOF_Underflow2		
	CMPI	  0,		R10		
	Beq	  FDTOF_Return			
	NEGI	  R7,	R7		
	LDIeq	  1,		R1		
	LDIne	  0,		R1		
	SUBI	  R1,		R6		
FDTOF_Return:
	LSH	  24,		R6		
	LSH	  20,		R10		
	LSH3	 -9,	R7,	R0		
	OR	  R10,		R0		
	OR	  R6,	R0		
	STI	  R0,	   *--AR6(1)	
	BuD	  R11				
	   LDF	 *AR6++(1),	R0		
	   LSH	 -1,		R7		
	   OR	  R7,	R0		
FDTOF_Overflow:
	LDI	  0x7F,		R6		
	CMPI	  0,		R10		
	BRD	  FDTOF_Return			
	   LDIeq -1,		R7		
	   LDIne  0,		R7		
	   LSH	 -1,		R7		
FDTOF_Underflow1:
	BRD	  FDTOF_Return			
	   LDI	  0,		R10		
	   LDI	  0,		R7		
	   LDI	  0x80,		R6		
FDTOF_Underflow2:
	CMPI	 -1023,		R6		
	Beq	  FDTOF_Underflow1		
	CMPI	  0,		R10		
	LDIeq	  0x81,		R6		
	LDIne	  0x80,		R6		
	BRD	  FDTOF_Return			
	   LDIeq  0,		R7		
	   LDIne -1,		R7		
	   LSH	 -1,		R7		
	init
	Bu	R11
	export	.FTSTD
	export	._FNEGD
	export	.FCMPD
	export	.FDTOU
	export	.FDTOI
	export	._FUTOD
	export	._FITOD
	export	._FSUBD
	export	._FADDD
	export	._FMULD
	export	._FDIVD
	export	._FSTOD
	export	.FDTOS
	export	.FNEGD
	export	.FUTOD
	export	.FITOD
	export	.FSUBD
	export	.FADDD
	export	.FMULD
	export	.FDIVD
	export	.FSTOD
	export	.FFTOS
	export	._FFTOD
	export	.FFTOD
	export	.FSTOF
	export	.FDTOF
		data .MaxData, 0
		codetable .MaxCodeP
	align		
.ModEnd:
		end
