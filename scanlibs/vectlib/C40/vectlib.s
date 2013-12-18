;
; vectlib.a - Assembly language implementation of a simple vector library
;
; Copyright (c) 1992 - 1994 by Perihelion Software Ltd.
;   All Rights Reserved
;
; Author: Nick Clifton
;
; Id: vectlib.a,v 1.6 1994/01/11 16:53:12 nickc Exp 
;
;
        align
        module  -1
.ModStart:
        word    0x60f160f1
		word modsize
	blkb    31, "vectlib"
	byte 0
	word	modnum
	word	1
		word	datasymb(.MaxData)
        init
	word	codesymb(.MaxCodeP)
			align
		__procname_VfAdd:
			byte	"VfAdd", 0
			align
			      word 	0xff000000 | - __procname_VfAdd * 4
			       export	.VfAdd	
	.VfAdd:
	SUBI3	        2,	           R0,	    RC		
	RPTBD	        vfadd_loop					
	   LSH3	       -2,                 R1,	    AR5	
	   LSH3	       -2,                 R2,	    AR0	
	   ADDF3     *++AR5(IR0), *++AR0(IR0), R0	
	 RND	        R0						
	 STF	        R0,             *AR5++(1)			
vfadd_loop:
	 ADDF3        *+AR5(0),      *++AR0(1),      R0	
	BuD	        R11						
	   RND		R0						
	   STF	        R0,             *AR5			
	   NOP								
			align
		__procname_VfSub:
			byte	"VfSub", 0
			align
			      word 	0xff000000 | - __procname_VfSub * 4
			       export	.VfSub	
	.VfSub:
	SUBI3	        2,	            R0,	    RC		
	RPTBD	        vfsub_loop					
	   LSH3	       -2,                  R1,	    AR5	
	   LSH3	       -2,                  R2,	    AR0	
	   SUBF3     *++AR0(IR0), *++AR5(IR0), R0	
	 RND	        R0						
	 STF	        R0,              *AR5++(1)			
vfsub_loop:
	 SUBF3     *++AR0(1),       *+AR5(0),      R0	
	BuD	        R11						
	   RND		R0						
	   STF	        R0,              *AR5			
	   NOP								
			align
		__procname_VfMul:
			byte	"VfMul", 0
			align
			      word 	0xff000000 | - __procname_VfMul * 4
			       export	.VfMul	
	.VfMul:
	SUBI3	        2,	            R0,	    RC		
	RPTBD	        vfmul_loop					
	   LSH3	       -2,                  R1,	    AR5	
	   LSH3	       -2,                  R2,	    AR0	
	   MPYF3     *++AR0(IR0), *++AR5(IR0), R0	
	 RND	        R0						
	 STF	        R0,              *AR5++(1)			
vfmul_loop:
	 MPYF3     *++AR0(1),       *+AR5(0),      R0	
	BuD	        R11						
	   RND		R0						
	   STF	        R0,              *AR5			
	   NOP								
			align
		__procname_VfDiv:
			byte	"VfDiv", 0
			align
			      word 	0xff000000 | - __procname_VfDiv * 4
			       export	.VfDiv	
	.VfDiv:
	SUBI3	   2, 			R0,		RC
	LSH3	  -2,			R1,		AR5
	RPTBD	   vfdiv_loop
	   LSH3	  -2,			R2,		AR0
	   ADDI	   IR0,		AR5
	   LDF	*++AR0(IR0),	R2
	 RCPF	   R2,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R6,	       *AR5,	R1
	 RND	   R1
vfdiv_loop:
	 STF	   R1,	       *AR5++(1)
	 || LDF	*++AR0(1),		R2
	RCPF	   R2,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	BuD	   R11
	   MPYF3   R6,	       *AR5,	R1
	   RND	   R1
	   STF	   R1,	       *AR5++(1)
			align
		__procname_VdAdd:
			byte	"VdAdd", 0
			align
			      word 	0xff000000 | - __procname_VdAdd * 4
			       export	.VdAdd	
	.VdAdd:
	LSH3	     -2, 		R1,	AR5		
	LSH3	     -2,		R2,	AR0		
	SUBI3	      2,		R0,	RC		
	RPTBD          vdadd_loop				
	   LDF	   *++AR5(IR0),	R0			
	   || LDF  *++AR0(IR0),	R1			
	   LDI	      AR5,		AR1
	   LDI	   *++AR5(1),	R0			
	   || LDI  *++AR0(1),	R1			
	 ADDF	      R1,             R0			
	 STF	      R0,            *AR1++(1)		
	 STI	      R0,            *AR1++(1)		
	 LDF	   *++AR5(1),	R0			
	 || LDF    *++AR0(1),	R1			
vdadd_loop:
	 LDI	   *++AR5(1),	R0			
	 || LDI    *++AR0(1),	R1			
	BuD	      R11					
	   ADDF	      R1,		R0			
	   STF	      R0,	       *AR1++(1)		
	   STI	      R0,	       *AR1++(1)		
			align
		__procname_VdSub:
			byte	"VdSub", 0
			align
			      word 	0xff000000 | - __procname_VdSub * 4
			       export	.VdSub	
	.VdSub:
	LSH3	     -2, 		R1,	AR5		
	LSH3	     -2,		R2,	AR0		
	SUBI3	      2,		R0,	RC		
	RPTBD         vdsub_loop				
	   LDF	   *++AR5(IR0),	R0			
	   || LDF  *++AR0(IR0),	R1			
	   LDI	      AR5,		AR1
	   LDI	   *++AR5(1),	R0			
	   || LDI  *++AR0(1),	R1			
	 SUBF	      R1,             R0			
	 STF	      R0,            *AR1++(1)		
	 STI	      R0,            *AR1++(1)		
	 LDF	   *++AR5(1),	R0			
	 || LDF    *++AR0(1),	R1			
vdsub_loop:
	 LDI	   *++AR5(1),	R0			
	 || LDI    *++AR0(1),	R1			
	BuD	      R11					
	   SUBF	      R1,		R0			
	   STF	      R0,	       *AR1++(1)		
	   STI	      R0,	       *AR1++(1)		
			align
		__procname_VdMul:
			byte	"VdMul", 0
			align
			      word 	0xff000000 | - __procname_VdMul * 4
			       export	.VdMul	
	.VdMul:
	LSH3	     -2, 		R1,	AR5		
	LSH3	     -2,		R2,	AR0		
	SUBI3	      2,		R0,	RC		
	RPTBD         vdmul_loop				
	   LDF	   *++AR5(IR0),	R0			
	   || LDF  *++AR0(IR0),	R1			
	   LDI	      AR5,		AR1
	   LDI	   *++AR5(1),	R0			
	   || LDI  *++AR0(1),	R1			
	 MPYF	      R1,             R0			
	 STF	      R0,            *AR1++(1)		
	 STI	      R0,            *AR1++(1)		
	 LDF	   *++AR5(1),	R0			
	 || LDF    *++AR0(1),	R1			
vdmul_loop:
	 LDI	   *++AR5(1),	R0			
	 || LDI    *++AR0(1),	R1			
	BuD	      R11					
	   MPYF	      R1,		R0			
	   STF	      R0,	       *AR1++(1)		
	   STI	      R0,	       *AR1++(1)		
			align
		__procname_VdDiv:
			byte	"VdDiv", 0
			align
			      word 	0xff000000 | - __procname_VdDiv * 4
			       export	.VdDiv	
	.VdDiv:
	SUBI3	      2, 		R0,		RC
	LSH3	     -2,		R1,		AR5
	RPTBD	      vddiv_loop
	   LSH3	     -2,		R2,		AR0
	   LDF	   *++AR0(IR0),	R2
	   || LDF  *++AR5(IR0),	R0
	   LDI	   *++AR0(1),	R2
	   || LDI   *+AR5(1),	R0
	 RCPF	   R2,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   1.0,			R7
	 MPYF	   R6,		R7
	 ADDF	   R7,		R6
	 MPYF	   R6,		R0
	 STF	   R0,	       *AR5++(1)
	 STI	   R0,	       *AR5++(1)
	 LDF	*++AR0(1),		R2
	 || LDF	  *AR5,		R0
vddiv_loop:
	 LDI	*++AR0(1),		R2
	 || LDI	 *+AR5(1),		R0
	RCPF	   R2,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   1.0,			R7
	MPYF	   R6,		R7
	ADDF	   R7,		R6
	BuD	   R11
	   MPYF    R6,	       	R0
	   STF	   R0,	       *AR5++(1)
	   STI	   R0,	       *AR5++(1)
			align
		__procname_VfAddScalar:
			byte	"VfAddScalar", 0
			align
			      word 	0xff000000 | - __procname_VfAddScalar * 4
			       export	.VfAddScalar	
	.VfAddScalar:
	STI	     R0,	 *--AR6(1)
	LDF	    *AR6++(1),     R0
	LDI	     R1,	     R0
	SUBI3	     2,		     R2,	RC
	RPTBD        vfadds_loop
	   LSH3	    -2,		     R3,	AR5
	   ADDF3  *++AR5(IR0), R0,	R1
	   LDI	     AR5,		        AR0
	 RND	     R1
	 STF	     R1,	    *AR0++(1)
vfadds_loop:
	 ADDF3  *++AR5(1),	     R0,	R1
	BuD	     R11
	   RND	     R1
	   STF	     R1,	    *AR0++(1)
	   NOP
			align
		__procname_VfSubScalar:
			byte	"VfSubScalar", 0
			align
			      word 	0xff000000 | - __procname_VfSubScalar * 4
			       export	.VfSubScalar	
	.VfSubScalar:
	STI	     R0,	 *--AR6(1)
	LDF	    *AR6++(1),     R0
	LDI	     R1,	     R0
	SUBI3	     2,		     R2,		RC
	RPTBD        vfsubs_loop
	   LSH3	    -2,		     R3,		AR5
	   SUBF3     R0,	  *++AR5(IR0),	R1
	   LDI	     AR5,				AR0
	 RND	     R1
	 STF	     R1,	    *AR0++(1)
vfsubs_loop:
	 SUBF3     R0,	  *++AR5(1),	   	R1
	BuD	     R11
	   RND	     R1
	   STF	     R1,	    *AR0++(1)
	   NOP
			align
		__procname_VfMulScalar:
			byte	"VfMulScalar", 0
			align
			      word 	0xff000000 | - __procname_VfMulScalar * 4
			       export	.VfMulScalar	
	.VfMulScalar:
	STI	     R0,	 *--AR6(1)
	LDF	    *AR6++(1),     R0
	LDI	     R1,	     R0
	SUBI3	     2,		     R2,		RC
	RPTBD        vfmuls_loop
	   LSH3	    -2,		     R3,		AR5
	   MPYF3     R0,	  *++AR5(IR0),	R1
	   LDI	     AR5,				AR0
	 RND	     R1
	 STF	     R1,	    *AR0++(1)
vfmuls_loop:
	 MPYF3       R0,	  *++AR5(1),	   	R1
	BuD	     R11
	   RND	     R1
	   STF	     R1,	    *AR0++(1)
	   NOP
			align
		__procname_VfDivScalar:
			byte	"VfDivScalar", 0
			align
			      word 	0xff000000 | - __procname_VfDivScalar * 4
			       export	.VfDivScalar	
	.VfDivScalar:
	LSH3	     -2,	    R3,		AR5
	STI	      R0,	*--AR6(1)
	SUBI3	      2,	    R2,		RC
	LDF	     *AR6++(1),   R0
	LDI	      R1,	    R0
	RCPF	      R0,	    R6
	MPYF3	      R0,	    R6,		R7
	SUBRF	      2.0,	    R7
	MPYF	      R7,	    R6
	MPYF3	      R0,	    R6,		R7
	SUBRF         2.0,	    R7
	RPTBD	      vfdivs_loop
	   MPYF	      R7,	    R6
	   MPYF3      R6,	 *++AR5(IR0),	R0
	   LDI	      AR5,	    AR0
	 RND	      R0
vfdivs_loop:
	 STF	      R0,	   *AR5++(1)
	 || MPYF3  *++AR0(1),   R6,		R0
	BuD	      R11
	   RND	      R0
	   STF	      R0,	   *AR5++(1)
	   NOP
			align
		__procname_VfRecScalar:
			byte	"VfRecScalar", 0
			align
			      word 	0xff000000 | - __procname_VfRecScalar * 4
			       export	.VfRecScalar	
	.VfRecScalar:
	STI	   R0,	    *--AR6(1)
	SUBI3	   2, 			R2,		RC
	LDF	  *AR6++(1),		R0
	RPTBD	   vfrecs_loop
	   LSH3	  -2,			R3,		AR5
	   LDI	   R1,		R0
	   LDF	*++AR5(IR0),	R2
	 RCPF	   R2,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R6,		R0,		R1
	 RND	   R1
	 STF	   R1,	       *AR5++(1)
vfrecs_loop:
	 LDF	  *AR5,		R2
	RCPF	   R2,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	BuD	   R11
	   MPYF3   R6,		R0,		R1
	   RND	   R1
	   STF	   R1,	       *AR5++(1)
			align
		__procname_VdAddScalar:
			byte	"VdAddScalar", 0
			align
			      word 	0xff000000 | - __procname_VdAddScalar * 4
			       export	.VdAddScalar	
	.VdAddScalar:
	STI	     R0,	 *--AR6(1)
	LDF	    *AR6++(1),     R0
	LDI	     R1,	     R0
	LSH3	    -2,		     R3,	AR5
	SUBI3	     2,		     R2,	RC
	RPTBD        vdadds_loop
	   LDF    *++AR5(IR0), R1
	   LDI	     AR5,         AR0
	   LDI	  *++AR5(1),	     R1
	 ADDF3       R0,	     R1,	R2
	 STF	     R2,	    *AR0++(1)
	 || LDF   *++AR5(1),	     R1
vdadds_loop:
	 STI	     R2,	    *AR0++(1)
	 || LDI   *++AR5(1),	     R1
	BuD	     R11
	   ADDF3     R0,	     R1,	R2
	   STF	     R2,	    *AR0++(1)
	   STI	     R2,	    *AR0++(1)
			align
		__procname_VdSubScalar:
			byte	"VdSubScalar", 0
			align
			      word 	0xff000000 | - __procname_VdSubScalar * 4
			       export	.VdSubScalar	
	.VdSubScalar:
	STI	     R0,	 *--AR6(1)
	LDF	    *AR6++(1),     R0
	LDI	     R1,	     R0
	LSH3	    -2,		     R3,	AR5
	SUBI3	     2,		     R2,	RC
	RPTBD        vdsubs_loop
	   LDF    *++AR5(IR0), R1
	   LDI	     AR5,         AR0
	   LDI	  *++AR5(1),	     R1
	 SUBF3       R0,	     R1,	R2
	 STF	     R2,	    *AR0++(1)
	 || LDF   *++AR5(1),	     R1
vdsubs_loop:
	 STI	     R2,	    *AR0++(1)
	 || LDI   *++AR5(1),	     R1
	BuD	     R11
	   SUBF3     R0,	     R1,	R2
	   STF	     R2,	    *AR0++(1)
	   STI	     R2,	    *AR0++(1)
			align
		__procname_VdMulScalar:
			byte	"VdMulScalar", 0
			align
			      word 	0xff000000 | - __procname_VdMulScalar * 4
			       export	.VdMulScalar	
	.VdMulScalar:
	STI	     R0,	 *--AR6(1)
	LDF	    *AR6++(1),     R0
	LDI	     R1,	     R0
	LSH3	    -2,		     R3,	AR5
	SUBI3	     2,		     R2,	RC
	RPTBD        vdmuls_loop
	   LDF    *++AR5(IR0), R1
	   LDI	     AR5,         AR0
	   LDI	  *++AR5(1),	     R1
	 MPYF3       R0,	     R1,	R2
	 STF	     R2,	    *AR0++(1)
	 || LDF   *++AR5(1),	     R1
vdmuls_loop:
	 STI	     R2,	    *AR0++(1)
	 || LDI   *++AR5(1),	     R1
	BuD	     R11
	   MPYF3     R0,	     R1,	R2
	   STF	     R2,	    *AR0++(1)
	   STI	     R2,	    *AR0++(1)
			align
		__procname_VdDivScalar:
			byte	"VdDivScalar", 0
			align
			      word 	0xff000000 | - __procname_VdDivScalar * 4
			       export	.VdDivScalar	
	.VdDivScalar:
	LSH3	     -2,	       R3,		AR5
	STI	      R0,	   *--AR6(1)
	SUBI3	      2,	       R2,		RC
	LDF	     *AR6++(1),      R0
	LDI	      R1,	       R0
	RCPF	      R0,	       R6
	MPYF3	      R0,	       R6,		R7
	SUBRF	      2.0,	       R7
	MPYF	      R7,	       R6
	MPYF3	      R0,	       R6,		R7
	SUBRF         2.0,	       R7
	MPYF	      R7,	       R6
	MPYF3	      R0,	       R6,		R7
	SUBRF	      1.0,	       R7
	MPYF	      R6,	       R7
	ADDF	      R7,	       R6
	LDF	   *++AR5(IR0),  R2
	RPTBD	      vddivs_loop
	   LDI	      AR5,	       AR0
	   LDI	   *++AR5(1),       R2
	   MPYF3      R6,	       R2,		R0
	 STF	      R0,	      *AR0++(1)
	 || LDF	   *++AR5(1),       R2
	 STI	      R0,	      *AR0++(1)
	 || LDI	   *++AR5(1),       R2
vddivs_loop:
	 MPYF3	      R2,            R6,		R0
	BuD	      R11
	   STF	      R0,	      *AR0++(1)
	   STI	      R0,	      *AR0++(1)
	   NOP
			align
		__procname_VdRecScalar:
			byte	"VdRecScalar", 0
			align
			      word 	0xff000000 | - __procname_VdRecScalar * 4
			       export	.VdRecScalar	
	.VdRecScalar:
	STI	   R0,	    *--AR6(1)
	SUBI3	   2, 			R2,		RC
	LDF	  *AR6++(1),		R0
	LSH3	  -2,			R3,		AR5
	LDF	*++AR5(IR0),	R2
	RPTBD	   vdrecs_loop
	   LDI	   AR5,		AR0
	   LDI	   R1,		R0
	   LDI	*++AR5(1),		R2
	 RCPF	   R2,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   1.0,			R7
	 MPYF	   R6,		R7
	 ADDF	   R7,		R6
	 MPYF3	   R6,		R0,		R1
	 STF	   R1,	       *AR0++(1)
	 || LDF	*++AR5(1),		R2
vdrecs_loop:
	 STI	   R1,	       *AR0++(1)
	 || LDI	*++AR5(1),		R2
	RCPF	   R2,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   1.0,			R7
	MPYF	   R6,		R7
	ADDF	   R7,		R6
	BuD	   R11
	   MPYF3   R6,		R0,		R1
	   STF	   R1,	       *AR0++(1)
	   STI	   R1,	       *AR0++(1)
			align
		__procname_VfCopy:
			byte	"VfCopy", 0
			align
			      word 	0xff000000 | - __procname_VfCopy * 4
			       export	.VfCopy	
	.VfCopy:
	LSH3	     -2,		R1,	AR5
	SUBI3	      2,		R0,	RC
	RPTBD	      vfcopy_loop
	   LSH3	     -2,		R2,	AR0
	   LDF	   *++AR0(IR0),	R0
	   ADDI	      IR0,		AR5
vfcopy_loop:
	 STF	      R0,	       *AR5++(1)
	 || LDF    *++AR0(1),	R0
	STF	      R0,	       *AR5++(1)
	Bu	      R11
			align
		__procname_VdCopy:
			byte	"VdCopy", 0
			align
			      word 	0xff000000 | - __procname_VdCopy * 4
			       export	.VdCopy	
	.VdCopy:
	LSH	      1,		R0
	LSH3	     -2,		R1,	AR5
	SUBI3	      2,		R0,	RC
	RPTBD	      vdcopy_loop
	   LSH3	     -2,		R2,	AR0
	   LDI	   *++AR0(IR0),	R0
	   ADDI	      IR0,		AR5
vdcopy_loop:
	 STI	      R0,	       *AR5++(1)
	 || LDI    *++AR0(1),	R0
	STI	      R0,	       *AR5++(1)
	Bu	      R11
			align
		__procname_VfFill:
			byte	"VfFill", 0
			align
			      word 	0xff000000 | - __procname_VfFill * 4
			       export	.VfFill	
	.VfFill:
	SUBI3	     2,		     R2,	RC
	STI	     R0,	 *--AR6(1)
	LDF	    *AR6++(1),     R0
	LSH3	    -2,		     R3,	AR5
	RPTBD        vffill_loop
	   LDI	     R1,	     R0
	   RND	     R0
	   STF       R0,	  *++AR5(IR0)
vffill_loop:
	 STF	     R0,	  *++AR5(1)
	Bu	     R11
			align
		__procname_VdFill:
			byte	"VdFill", 0
			align
			      word 	0xff000000 | - __procname_VdFill * 4
			       export	.VdFill	
	.VdFill:
	SUBI3	     2,		     R2,	RC
	STI	     R0,	 *--AR6(1)
	LDF	    *AR6++(1),     R0
	LSH3	    -2,		     R3,	AR5
	RPTBD        vdfill_loop
	   LDI	     R1,	     R0
	   STF       R0,	  *++AR5(IR0)
	   STI       R0,	  *++AR5(1)
	 STF	     R0,	  *++AR5(1)
vdfill_loop:
	 STI	     R0,	  *++AR5(1)
	Bu	     R11
			align
		__procname_VfMax:
			byte	"VfMax", 0
			align
			      word 	0xff000000 | - __procname_VfMax * 4
			       export	.VfMax	
	.VfMax:
	SUBI3	   2,			R0,	RC
	LSH3	  -2,			R1,	AR5
	RPTBD	   vfmax_loop
	   LDI	   0,			R0
	   LDI	   0,			R2
	   LDF	*++AR5(IR0),	R1
	 ADDI	   1,			R2
	 CMPF3	   R1,	     *++AR5(1)
	 LDIgt	   R2,		R0
vfmax_loop:
	 LDFgt	  *AR5,		R1
	Bu	   R11 
			align
		__procname_VdMax:
			byte	"VdMax", 0
			align
			      word 	0xff000000 | - __procname_VdMax * 4
			       export	.VdMax	
	.VdMax:
	SUBI3	   2,			R0,	RC
	LSH3	  -2,			R1,	AR5
	LDI	   0,			R0
	RPTBD	   vdmax_loop
	   LDI	   0,			R2
	   LDF	*++AR5(IR0),	R1
	   LDI	*++AR5(1),		R1
	 LDF	*++AR5(1),		R6
	 LDI	*++AR5(1),		R6
	 ADDI	   1,			R2
	 CMPF	   R1,		R6
	 LDFgt	   R6,		R1
vdmax_loop:
	 LDIgt	   R2,		R0
	Bu	   R11 
			align
		__procname_VfMin:
			byte	"VfMin", 0
			align
			      word 	0xff000000 | - __procname_VfMin * 4
			       export	.VfMin	
	.VfMin:
	SUBI3	   2,			R0,	RC
	LSH3	  -2,			R1,	AR5
	RPTBD	   vfmin_loop
	   LDI	   0,			R0
	   LDI	   0,			R2
	   LDF	*++AR5(IR0),	R1
	 ADDI	   1,			R2
	 CMPF3	   R1,             *++AR5(1)
	 LDIlt	   R2,		R0
vfmin_loop:
	 LDFlt	  *AR5,		R1
	Bu	   R11 
			align
		__procname_VdMin:
			byte	"VdMin", 0
			align
			      word 	0xff000000 | - __procname_VdMin * 4
			       export	.VdMin	
	.VdMin:
	SUBI3	   2,			R0,	RC
	LSH3	  -2,			R1,	AR5
	LDI	   0,			R0
	RPTBD	   vdmin_loop
	   LDI	   0,			R2
	   LDF	*++AR5(IR0),	R1
	   LDI	*++AR5(1),		R1
	 LDF	*++AR5(1),		R6
	 LDI	*++AR5(1),		R6
	 ADDI	   1,			R2
	 CMPF	   R1,		R6
	 LDFlt	   R6,		R1
vdmin_loop:
	 LDIlt	   R2,		R0
	Bu	   R11 
			align
		__procname_VfAmax:
			byte	"VfAmax", 0
			align
			      word 	0xff000000 | - __procname_VfAmax * 4
			       export	.VfAmax	
	.VfAmax:
	SUBI3	   2,			R0,	RC
	LSH3	  -2,			R1,	AR5
	RPTBD	   vfamax_loop
	   LDI	   0,			R0
	   LDI	   0,			R2
	   ABSF	*++AR5(IR0),	R1
	 ADDI	   1,			R2
	 ABSF	*++AR5(1),		R6
	 CMPF	   R1,		R6
	 LDFgt	   R6,		R1
vfamax_loop:
	 LDIgt	   R2,		R0
	Bu	   R11 
			align
		__procname_VdAmax:
			byte	"VdAmax", 0
			align
			      word 	0xff000000 | - __procname_VdAmax * 4
			       export	.VdAmax	
	.VdAmax:
	SUBI3	   2,			R0,	RC
	LSH3	  -2,			R1,	AR5
	LDI	   0,			R0
	LDI	   0,			R2
	RPTBD	   vdamax_loop
	   LDF	*++AR5(IR0),	R1
	   LDI	*++AR5(1),		R1
	   ABSF    R1
	 LDF	*++AR5(1),		R6
	 LDI	*++AR5(1),		R6
	 ABSF	   R6
	 ADDI	   1,			R2
	 CMPF	   R1,		R6
	 LDFgt	   R6,		R1
vdamax_loop:
	 LDIgt	   R2,		R0
	Bu	   R11 
			align
		__procname_VfAmin:
			byte	"VfAmin", 0
			align
			      word 	0xff000000 | - __procname_VfAmin * 4
			       export	.VfAmin	
	.VfAmin:
	SUBI3	   2,			R0,	RC
	LSH3	  -2,			R1,	AR5
	RPTBD	   vfamin_loop
	   LDI	   0,			R0
	   LDI	   0,			R2
	   ABSF	*++AR5(IR0),	R1
	 ADDI	   1,			R2
	 ABSF	*++AR5(1),		R6
	 CMPF	   R1,		R6
	 LDFlt	   R6,		R1
vfamin_loop:
	 LDIlt	   R2,		R0
	Bu	   R11 
			align
		__procname_VdAmin:
			byte	"VdAmin", 0
			align
			      word 	0xff000000 | - __procname_VdAmin * 4
			       export	.VdAmin	
	.VdAmin:
	SUBI3	   2,			R0,	RC
	LSH3	  -2,			R1,	AR5
	LDI	   0,			R0
	LDI	   0,			R2
	RPTBD	   vdamin_loop
	   LDF	*++AR5(IR0),	R1
	   LDI	*++AR5(1),		R1
	   ABSF    R1
	 LDF	*++AR5(1),		R6
	 LDI	*++AR5(1),		R6
	 ABSF	   R6
	 ADDI	   1,			R2
	 CMPF	   R1,		R6
	 LDFlt	   R6,		R1
vdamin_loop:
	 LDIlt	   R2,		R0
	Bu	   R11 
			align
		__procname_VfDot:
			byte	"VfDot", 0
			align
			      word 	0xff000000 | - __procname_VfDot * 4
			       export	.VfDot	
	.VfDot:
	SUBI	     2,			R0,			RC
	   LSH3	    -2,			R1,			AR5
	RPTBD        vfdot_loop
	   LSH3	    -2,			R2,			AR0
	   LDF	     0.0,		R2
	   MPYF3  *++AR5(IR0), *++AR0(IR0),	R1
vfdot_loop:
	 MPYF3	  *++AR5(1),	     *++AR0(1),		R1
	 || ADDF3    R2,	        R1,			R2
	BuD	     R11
	   ADDF3     R1,		R2,			R0
	   RND	     R0
	   NOP
			align
		__procname_VdDot:
			byte	"VdDot", 0
			align
			      word 	0xff000000 | - __procname_VdDot * 4
			       export	.VdDot	
	.VdDot:
	LSH3	     -2,		R1,	AR5
	LSH3	     -2,		R2,	AR0
	SUBI	      2,		R0,	RC
	RPTBD         vddot_loop
	   LDF	   *++AR5(IR0),	R1
	   || LDF  *++AR0(IR0),	R2
	   LDF	      0.0,		R0
	   LDI	   *++AR5(1),	R1
	   || LDI  *++AR0(1),	R2
	 MPYF	      R1,		R2
	 ADDF	      R2,		R0
	 LDF	   *++AR5(1),	R1
	 || LDF	   *++AR0(1),	R2
vddot_loop:
	 LDI	   *++AR5(1),	R1
	 || LDI	   *++AR0(1),	R2
	BuD	      R11
	   MPYF	      R1,		R2
	   ADDF	      R2,		R0
	   NOP
			align
		__procname_VfSum:
			byte	"VfSum", 0
			align
			      word 	0xff000000 | - __procname_VfSum * 4
			       export	.VfSum	
	.VfSum:
	SUBI3	  2,			R0,	RC
	RPTBD     vfsum_loop
	   LSH3	 -2,			R1,	AR5
	   NOP
	   LDF	*++AR5(IR0),	R0
vfsum_loop:
	 ADDF	*++AR5(1),		R0
	RND	   R0
	Bu	   R11
			align
		__procname_VdSum:
			byte	"VdSum", 0
			align
			      word 	0xff000000 | - __procname_VdSum * 4
			       export	.VdSum	
	.VdSum:
	SUBI3	  2,			R0,	RC
	RPTBD     vdsum_loop
	   LSH3	 -2,			R1,	AR5
	   LDF	*++AR5(IR0),	R0
	   LDI	*++AR5(1),		R0
	 LDF	*++AR5(1),		R1
	 LDI	*++AR5(1),		R1
vdsum_loop:
	 ADDF	   R1,		R0
	Bu	   R11
			align
		__procname_VfProd:
			byte	"VfProd", 0
			align
			      word 	0xff000000 | - __procname_VfProd * 4
			       export	.VfProd	
	.VfProd:
	SUBI3	  2,			R0,	RC
	RPTBD     vfprod_loop
	   LSH3	 -2,			R1,	AR5
	   NOP
	   LDF	*++AR5(IR0),	R0
vfprod_loop:
	 MPYF	*++AR5(1),		R0
	RND	   R0
	Bu	   R11
			align
		__procname_VdProd:
			byte	"VdProd", 0
			align
			      word 	0xff000000 | - __procname_VdProd * 4
			       export	.VdProd	
	.VdProd:
	SUBI3	  2,			R0,	RC
	RPTBD     vdprod_loop
	   LSH3	 -2,			R1,	AR5
	   LDF	*++AR5(IR0),	R0
	   LDI	*++AR5(1),		R0
	 LDF	*++AR5(1),		R1
	 LDI	*++AR5(1),		R1
vdprod_loop:
	 MPYF	   R1,		R0
	Bu	   R11
			align
		__procname_VfMulAdd:
			byte	"VfMulAdd", 0
			align
			      word 	0xff000000 | - __procname_VfMulAdd * 4
			       export	.VfMulAdd	
	.VfMulAdd:
	STI	     R0,	    *--AR6(1)
	LDF	    *AR6++(1),	R0
	LSH3	    -2,			R3,		AR5
	SUBI3	     2,			R2,		RC
	LSH3        -2,		      *+AR6(0),	AR0
	RPTBD	     vfmuladd_loop
	   LDI	     R1,		R0
	   MPYF3  *++AR0(IR0),	R0,		R1
	   ADDF	  *++AR5(IR0),	R1
	 RND	     R1
	 STF	     R1,	       *AR5++(1)
	 || MPYF3 *++AR0(1),	R0,		R1
vfmuladd_loop:
	 ADDF	    *AR5,		R1
	BuD	     R11
	   RND	     R1
	   STF	     R1,	       *AR5
	   NOP
			align
		__procname_VdMulAdd:
			byte	"VdMulAdd", 0
			align
			      word 	0xff000000 | - __procname_VdMulAdd * 4
			       export	.VdMulAdd	
	.VdMulAdd:
	STI	     R0,	    *--AR6(1)
	LDF	    *AR6++(1),	R0
	LSH3	    -2,			R3,		AR5
	SUBI3	     2,			R2,		RC
	LSH3        -2,		      *+AR6(0),	AR0
	LDI	     R1,		R0
	RPTBD	     vdmuladd_loop
	   LDF	  *++AR0(IR0),	R1
	   || LDF *++AR5(IR0),	R2
	   LDI	  *++AR0(1),	R1
	   || LDI  *+AR5(1),		R2
	   MPYF      R0,		R1
	 ADDF	     R2,	        R1
	 STF	     R1,	       *AR5++(1)
	 STI	     R1,	       *AR5++(1)
	 LDF	  *++AR0(1),	R1
	 || LDF     *AR5,		R2
	 LDI	  *++AR0(1),	R1
	 || LDI    *+AR5(1),		R2
vdmuladd_loop:
	 MPYF        R0,		R1
	BuD	     R11
	   ADDF	     R2,		R1
	   STF	     R1,	       *AR5++(1)
	   STI	     R1,	       *AR5
			align
		__procname_VfsAdd:
			byte	"VfsAdd", 0
			align
			      word 	0xff000000 | - __procname_VfsAdd * 4
			       export	.VfsAdd	
	.VfsAdd:
	LSH3     -2,		      R1,	AR5
	LSH3     -2,		      R3,	AR0
	STI	  IR0,         *--AR6(1)
	STI	  IR1,	  *--AR6(1)
	LDF    *++AR5(IR0),     R1
	ADDF   *++AR0(IR0),    R1
	SUBI3     2,		      R0,	RC
	RPTBD	  vfsadd_loop
	   LDI	  R2,		      IR0
	   LDI  *+AR6(2),	      IR1
	   RND    R1
	 STF      R1,		     *AR5++(IR0)
	 LDF     *AR5,	      R1
	 ADDF  *++AR0(IR1),     R1
vfsadd_loop:
	 RND      R1
	BuD	  R11
 	   STF	  R1,		     *AR5
	   LDI	 *AR6++(1),	      IR1
	   LDI	 *AR6++(1),	      IR0
			align
		__procname_VfsSub:
			byte	"VfsSub", 0
			align
			      word 	0xff000000 | - __procname_VfsSub * 4
			       export	.VfsSub	
	.VfsSub:
	LSH3     -2,		      R1,	AR5
	LSH3     -2,		      R3,	AR0
	STI	  IR0,         *--AR6(1)
	STI	  IR1,	  *--AR6(1)
	LDF    *++AR5(IR0),     R1
	SUBF   *++AR0(IR0),    R1
	SUBI3     2,		      R0,	RC
	RPTBD	  vfssub_loop
	   LDI	  R2,		      IR0
	   LDI  *+AR6(2),	      IR1
	   RND    R1
	 STF      R1,		     *AR5++(IR0)
	 LDF     *AR5,	      R1
	 SUBF  *++AR0(IR1),     R1
vfssub_loop:
	 RND      R1
	BuD	  R11
 	   STF	  R1,		     *AR5
	   LDI	 *AR6++(1),	      IR1
	   LDI	 *AR6++(1),	      IR0
			align
		__procname_VfsMul:
			byte	"VfsMul", 0
			align
			      word 	0xff000000 | - __procname_VfsMul * 4
			       export	.VfsMul	
	.VfsMul:
	LSH3     -2,		      R1,	AR5
	LSH3     -2,		      R3,	AR0
	STI	  IR0,         *--AR6(1)
	STI	  IR1,	  *--AR6(1)
	LDF    *++AR5(IR0),     R1
	MPYF   *++AR0(IR0),    R1
	SUBI3     2,		      R0,	RC
	RPTBD	  vfsmul_loop
	   LDI	  R2,		      IR0
	   LDI  *+AR6(2),	      IR1
	   RND    R1
	 STF      R1,		     *AR5++(IR0)
	 LDF     *AR5,	      R1
	 MPYF  *++AR0(IR1),     R1
vfsmul_loop:
	 RND      R1
	BuD	  R11
 	   STF	  R1,		     *AR5
	   LDI	 *AR6++(1),	      IR1
	   LDI	 *AR6++(1),	      IR0
			align
		__procname_VfsDiv:
			byte	"VfsDiv", 0
			align
			      word 	0xff000000 | - __procname_VfsDiv * 4
			       export	.VfsDiv	
	.VfsDiv:
	SUBI3	   2, 			R0,		RC
	LSH3	  -2,			R1,		AR5
	LDI	   IR1,		R10
	LDI	  *AR6,		IR1
	LSH3	  -2,			R3,		AR0
	ADDI	   IR0,		AR5
	RPTBD	   vfsdiv_loop
	   LDF	*++AR0(IR0),	R3
	   LDI	   IR0,		AR1
	   LDI	   R2,		IR0
	 RCPF	   R3,		R6
	 MPYF3	   R3,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R3,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R6,	       *AR5,	R1
	 RND	   R1
vfsdiv_loop:
	 STF	   R1,	       *AR5++(IR0)
	 || LDF	*++AR0(IR1),	R3
	RCPF	   R3,		R6
	MPYF3	   R3,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	MPYF3	   R3,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	MPYF3      R6,	       *AR5,		R1
	RND	   R1
	BuD	   R11
	   STF	   R1,	       *AR5
	   LDI	   R10,	        IR1
	   LDI	   AR1,		IR0
			align
		__procname_VdsAdd:
			byte	"VdsAdd", 0
			align
			      word 	0xff000000 | - __procname_VdsAdd * 4
			       export	.VdsAdd	
	.VdsAdd:
	LSH3       -2,		      R1,		AR5
	LSH3       -2,		      R3,		AR0
	STI	    IR0,       *--AR6(1)
	STI	    IR1,	  *--AR6(1)
	LDF      *++AR5(IR0),   R1
	|| LDF   *++AR0(IR0),  R6
	LDI       *+AR5(1),	      R1
	|| LDI    *+AR0(1),	      R6
	SUBI3       2,		      R0,		RC
	RPTBD	    vdsadd_loop
	   ADDF     R6,	      R1
	   LSH3	    1,		      R2,		IR0
	   LSH3     1,		    *+AR6(2),		IR1
	 STI        R1,	    *+AR5(1)
	 STF        R1,	     *AR5++(IR0)
	 LDF       *AR5,	      R1
	 || LDF  *++AR0(IR1),   R6
	 LDI      *+AR5(1),	      R1
	 || LDI   *+AR0(1),       R6
vdsadd_loop:
	 ADDF	    R6,	      R1
 	STI	    R1,	    *+AR5(1)
	BuD	    R11
 	   STF	    R1,	     *AR5
	   LDI	   *AR6++(1),	      IR1
	   LDI	   *AR6++(1),	      IR0
			align
		__procname_VdsSub:
			byte	"VdsSub", 0
			align
			      word 	0xff000000 | - __procname_VdsSub * 4
			       export	.VdsSub	
	.VdsSub:
	LSH3       -2,		      R1,		AR5
	LSH3       -2,		      R3,		AR0
	STI	    IR0,       *--AR6(1)
	STI	    IR1,	  *--AR6(1)
	LDF      *++AR5(IR0),   R1
	|| LDF   *++AR0(IR0),  R6
	LDI       *+AR5(1),	      R1
	|| LDI    *+AR0(1),	      R6
	SUBI3       2,		      R0,		RC
	RPTBD	    vdssub_loop
	   SUBF     R6,	      R1
	   LSH3	    1,		      R2,		IR0
	   LSH3     1,		    *+AR6(2),		IR1
	 STI        R1,	    *+AR5(1)
	 STF        R1,	     *AR5++(IR0)
	 LDF       *AR5,	      R1
	 || LDF  *++AR0(IR1),   R6
	 LDI      *+AR5(1),	      R1
	 || LDI   *+AR0(1),       R6
vdssub_loop:
	 SUBF	    R6,	      R1
 	STI	    R1,	    *+AR5(1)
	BuD	    R11
 	   STF	    R1,	     *AR5
	   LDI	   *AR6++(1),	      IR1
	   LDI	   *AR6++(1),	      IR0
			align
		__procname_VdsMul:
			byte	"VdsMul", 0
			align
			      word 	0xff000000 | - __procname_VdsMul * 4
			       export	.VdsMul	
	.VdsMul:
	LSH3       -2,		      R1,		AR5
	LSH3       -2,		      R3,		AR0
	STI	    IR0,       *--AR6(1)
	STI	    IR1,	  *--AR6(1)
	LDF      *++AR5(IR0),   R1
	|| LDF   *++AR0(IR0),  R6
	LDI       *+AR5(1),	      R1
	|| LDI    *+AR0(1),	      R6
	SUBI3       2,		      R0,		RC
	RPTBD	    vdsmul_loop
	   MPYF     R6,	      R1
	   LSH3	    1,		      R2,		IR0
	   LSH3     1,		    *+AR6(2),		IR1
	 STI        R1,	    *+AR5(1)
	 STF        R1,	     *AR5++(IR0)
	 LDF       *AR5,	      R1
	 || LDF  *++AR0(IR1),   R6
	 LDI      *+AR5(1),	      R1
	 || LDI   *+AR0(1),       R6
vdsmul_loop:
	 MPYF	    R6,	      R1
 	STI	    R1,	    *+AR5(1)
	BuD	    R11
 	   STF	    R1,	     *AR5
	   LDI	   *AR6++(1),	      IR1
	   LDI	   *AR6++(1),	      IR0
			align
		__procname_VdsDiv:
			byte	"VdsDiv", 0
			align
			      word 	0xff000000 | - __procname_VdsDiv * 4
			       export	.VdsDiv	
	.VdsDiv:
	SUBI3	   2, 			R0,		RC
	LSH3	  -2,			R1,		AR5
	LDI	   IR1,		R10
	LSH3	   1,		      *+AR6(0),	IR1
	LSH3	  -2,			R3,		AR0
	ADDI	   IR0,		AR5
	LDF	*++AR0(IR0),	R3
	RPTBD	   vdsdiv_loop
	   LDI	   IR0,		AR1
	   LDI	 *+AR0(1),		R3
	   LSH3	   1,			R2,		IR0
	 RCPF	   R3,		R6
	 MPYF3	   R3,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R3,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R3,		R6,		R7
	 SUBRF	   1.0,			R7
	 MPYF	   R6,		R7
	 ADDF	   R7,		R6
	 LDF	  *AR5,		R1
	 LDI	 *+AR5(1),		R1
	 MPYF	   R6,	       	R1
	 STI	   R1,	      *+AR5(1)
	 STF	   R1,	       *AR5++(IR0)
	 || LDF	*++AR0(IR1),	R3
vdsdiv_loop:
	 LDI	 *+AR0(1),		R3
	RCPF	   R3,		R6
	MPYF3	   R3,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	MPYF3	   R3,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	MPYF3	   R3,		R6,		R7
	SUBRF	   1.0,			R7
	MPYF	   R6,		R7
	ADDF	   R7,		R6
	LDF	  *AR5,		R1
	LDI	 *+AR5(1),		R1
	MPYF       R6,	       	R1
	STF	   R1,	       *AR5
	BuD	   R11
	   STI	   R1,	      *+AR5(1)
	   LDI	   R10,	        IR1
	   LDI	   AR1,		IR0
			align
		__procname_VfsAddScalar:
			byte	"VfsAddScalar", 0
			align
			      word 	0xff000000 | - __procname_VfsAddScalar * 4
			       export	.VfsAddScalar	
	.VfsAddScalar:
	STI	     R0,	    *--AR6(1)
	LSH3	    -2,			R3,		AR5
	LDF	    *AR6++(1),	R0
	LDI	     R1,		R0
	SUBI3	     2,			R2,		RC
	RPTBD	     vfsadds_loop
	   STI	     IR1,	    *--AR6(1)
	   ADDF3  *++AR5(IR0),	R0,		R3
	   LDI	   *+AR6(1),		IR1
	 RND	     R3
	 STF	     R3,	       *AR5
vfsadds_loop:
	 ADDF3    *++AR5(IR1),     R0,		R3
	BuD	     R11
	   RND	     R3
           STF	     R3,	       *AR5
	   LDI	    *AR6++(1),	IR1
			align
		__procname_VfsSubScalar:
			byte	"VfsSubScalar", 0
			align
			      word 	0xff000000 | - __procname_VfsSubScalar * 4
			       export	.VfsSubScalar	
	.VfsSubScalar:
	STI	     R0,	    *--AR6(1)
	LSH3	    -2,			R3,		AR5
	LDF	    *AR6++(1),	R0
	LDI	     R1,		R0
	SUBI3	     2,			R2,		RC
	RPTBD	     vfssubs_loop
	   STI	     IR1,	    *--AR6(1)
	   SUBF3     R0,	     *++AR5(IR0),	R3
	   LDI	   *+AR6(1),		IR1
	 RND	     R3
	 STF	     R3,	       *AR5
vfssubs_loop:
	 SUBF3       R0,	     *++AR5(IR1),  R3
	BuD	     R11
	   RND	     R3
           STF	     R3,	       *AR5
	   LDI	    *AR6++(1),	IR1
			align
		__procname_VfsMulScalar:
			byte	"VfsMulScalar", 0
			align
			      word 	0xff000000 | - __procname_VfsMulScalar * 4
			       export	.VfsMulScalar	
	.VfsMulScalar:
	STI	     R0,	    *--AR6(1)
	LSH3	    -2,			R3,		AR5
	LDF	    *AR6++(1),	R0
	LDI	     R1,		R0
	SUBI3	     2,			R2,		RC
	RPTBD	     vfsmuls_loop
	   STI	     IR1,	    *--AR6(1)
	   MPYF3  *++AR5(IR0),	R0,		R3
	   LDI	   *+AR6(1),		IR1
	 RND	     R3
	 STF	     R3,	       *AR5
vfsmuls_loop:
	 MPYF3    *++AR5(IR1),     R0,		R3
	BuD	     R11
	   RND	     R3
           STF	     R3,	       *AR5
	   LDI	    *AR6++(1),	IR1
			align
		__procname_VfsDivScalar:
			byte	"VfsDivScalar", 0
			align
			      word 	0xff000000 | - __procname_VfsDivScalar * 4
			       export	.VfsDivScalar	
	.VfsDivScalar:
	LSH3	     -2,	       R3,		AR5
	STI	      R0,	   *--AR6(1)
	SUBI3	      2,	       R2,		RC
	LDF	     *AR6++(1),      R0
	LDI	      R1,	       R0
	LDI	      IR1,	       R10
	LDI	     *AR6,	       IR1
	RCPF	      R0,	       R6
	MPYF3	      R0,	       R6,		R7
	SUBRF	      2.0,	       R7
	MPYF	      R7,	       R6
	MPYF3	      R0,	       R6,		R7
	SUBRF         2.0,	       R7
	RPTBD	      vfsdivs_loop
	   MPYF	      R7,	       R6
	   MPYF3      R6,	    *++AR5(IR0),	R0
	   LDI	      AR5,	       AR0
	 RND	      R0
vfsdivs_loop:
	 STF	      R0,	      *AR5++(IR1)
	 || MPYF3  *++AR0(IR1),  R6,		R0
	BuD	      R11
	   RND	      R0
	   STF	      R0,	      *AR5
	   LDI	      R10,	       IR1
			align
		__procname_VfsRecScalar:
			byte	"VfsRecScalar", 0
			align
			      word 	0xff000000 | - __procname_VfsRecScalar * 4
			       export	.VfsRecScalar	
	.VfsRecScalar:
	STI	   R0,	    *--AR6(1)
	SUBI3	   2, 			R2,		RC
	LDF	  *AR6++(1),		R0
	LDI	   IR1,		R10
	LDI	  *AR6,		IR1
	RPTBD	   vfsrecs_loop
	   LSH3	  -2,			R3,		AR5
	   LDI	   R1,		R0
	   LDF	*++AR5(IR0),	R2
	 RCPF	   R2,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R6,		R0,		R1
	 RND	   R1
	 STF	   R1,	       *AR5++(IR1)
vfsrecs_loop:
	 LDF	  *AR5,		R2
	RCPF	   R2,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	LDI	   R10,		IR1
	BuD	   R11
	   MPYF3   R6,		R0,		R1
	   RND	   R1
	   STF	   R1,	       *AR5
			align
		__procname_VdsAddScalar:
			byte	"VdsAddScalar", 0
			align
			      word 	0xff000000 | - __procname_VdsAddScalar * 4
			       export	.VdsAddScalar	
	.VdsAddScalar:
	STI	     R0,	    *--AR6(1)
	LSH3	    -2,			R3,		AR5
	LDF	    *AR6++(1),	R0
	LDI	     R1,		R0
	LDF       *++AR5(IR0),	R3
	STI	     IR1,	    *--AR6(1)
	LDI        *+AR5(1),		R3
	SUBI3	     2,			R2,		RC
	RPTBD	     vdsadds_loop
	   LDI	     AR5,		AR0
	   LSH3	     1,		      *+AR6(1),	IR1
	   ADDF	     R0,		R3
	 STI	     R3,	      *+AR0(1)
	 STF	     R3,	       *AR0++(IR1)
	 || LDF   *++AR5(IR1),     R3
	 LDI       *+AR5(1),         R3
vdsadds_loop:
	 ADDF	     R0,		R3
	BuD	     R11
           STF	     R3,	       *AR0
           STI	     R3,	      *+AR0(1)
	   LDI	    *AR6++(1),	IR1
			align
		__procname_VdsSubScalar:
			byte	"VdsSubScalar", 0
			align
			      word 	0xff000000 | - __procname_VdsSubScalar * 4
			       export	.VdsSubScalar	
	.VdsSubScalar:
	STI	     R0,	    *--AR6(1)
	LSH3	    -2,			R3,		AR5
	LDF	    *AR6++(1),	R0
	LDI	     R1,		R0
	LDF       *++AR5(IR0),	R3
	STI	     IR1,	    *--AR6(1)
	LDI        *+AR5(1),		R3
	SUBI3	     2,			R2,		RC
	RPTBD	     vdssubs_loop
	   LDI	     AR5,		AR0
	   LSH3	     1,		      *+AR6(1),	IR1
	   SUBF	     R0,		R3
	 STI	     R3,	      *+AR0(1)
	 STF	     R3,	       *AR0++(IR1)
	 || LDF   *++AR5(IR1),     R3
	 LDI       *+AR5(1),         R3
vdssubs_loop:
	 SUBF	     R0,		R3
	BuD	     R11
           STF	     R3,	       *AR0
           STI	     R3,	      *+AR0(1)
	   LDI	    *AR6++(1),	IR1
			align
		__procname_VdsMulScalar:
			byte	"VdsMulScalar", 0
			align
			      word 	0xff000000 | - __procname_VdsMulScalar * 4
			       export	.VdsMulScalar	
	.VdsMulScalar:
	STI	     R0,	    *--AR6(1)
	LSH3	    -2,			R3,		AR5
	LDF	    *AR6++(1),	R0
	LDI	     R1,		R0
	LDF       *++AR5(IR0),	R3
	STI	     IR1,	    *--AR6(1)
	LDI        *+AR5(1),		R3
	SUBI3	     2,			R2,		RC
	RPTBD	     vdsmuls_loop
	   LDI	     AR5,		AR0
	   LSH3	     1,		      *+AR6(1),	IR1
	   MPYF	     R0,		R3
	 STI	     R3,	      *+AR0(1)
	 STF	     R3,	       *AR0++(IR1)
	 || LDF   *++AR5(IR1),     R3
	 LDI       *+AR5(1),         R3
vdsmuls_loop:
	 MPYF	     R0,		R3
	BuD	     R11
           STF	     R3,	       *AR0
           STI	     R3,	      *+AR0(1)
	   LDI	    *AR6++(1),	IR1
			align
		__procname_VdsDivScalar:
			byte	"VdsDivScalar", 0
			align
			      word 	0xff000000 | - __procname_VdsDivScalar * 4
			       export	.VdsDivScalar	
	.VdsDivScalar:
	LSH3	     -2,	       R3,		AR5
	STI	      R0,	   *--AR6(1)
	SUBI3	      2,	       R2,		RC
	LDF	     *AR6++(1),      R0
	LDI	      R1,	       R0
	LDI	      IR1,	       R10
	LSH3	      1,	     *+AR6(0),	IR1
	RCPF	      R0,	       R6
	MPYF3	      R0,	       R6,		R7
	SUBRF	      2.0,	       R7
	MPYF	      R7,	       R6
	MPYF3	      R0,	       R6,		R7
	SUBRF         2.0,	       R7
	MPYF	      R7,	       R6
	MPYF3	      R0,	       R6,		R7
	SUBRF	      1.0,	       R7
	MPYF	      R6,	       R7
	ADDF	      R7,	       R6
	LDF	   *++AR5(IR0),  R0
	RPTBD	      vdsdivs_loop
	   LDI	    *+AR5(1),       R0
	   MPYF       R6,	       R0
	   LDI	      AR5,	       AR0
	 STI	      R0,	     *+AR5(1)
	 STF	      R0,	      *AR5++(IR1)
	 || LDF	   *++AR0(IR1),  R1
	 LDI	    *+AR0(1),      R1
vdsdivs_loop:
	 MPYF3	      R6,	       R1,		R0
	BuD	      R11
	   STF	      R0,	      *AR5
	   STI	      R0,	     *+AR5(1)
	   LDI	      R10,	       IR1
			align
		__procname_VdsRecScalar:
			byte	"VdsRecScalar", 0
			align
			      word 	0xff000000 | - __procname_VdsRecScalar * 4
			       export	.VdsRecScalar	
	.VdsRecScalar:
	STI	   R0,	    *--AR6(1)
	SUBI3	   2, 			R2,		RC
	LDF	  *AR6++(1),		R0
	LDI	   IR1,		R10
	LSH3	   1,		      *+AR6(0),	IR1
	LSH3	  -2,			R3,		AR5
	RPTBD	   vdsrecs_loop
	   LDI	   R1,		R0
	   LDF	*++AR5(IR0),	R2
	   LDI	   AR5,		AR0
	 RCPF	   R2,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   2.0,			R7
	 MPYF	   R7,		R6
	 MPYF3	   R2,		R6,		R7
	 SUBRF	   1.0,			R7
	 MPYF	   R6,		R7
	 ADDF	   R7,		R6
	 MPYF3	   R6,		R0,		R1
	 STI	   R1,	      *+AR5(1)
	 STF	   R1,	       *AR5++(IR1)
	 || LDF	*++AR0(IR1),	R2
vdsrecs_loop:
	 LDI	 *+AR0(1),		R2
	RCPF	   R2,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   2.0,			R7
	MPYF	   R7,		R6
	MPYF3	   R2,		R6,		R7
	SUBRF	   1.0,			R7
	MPYF	   R6,		R7
	ADDF	   R7,		R6
	LDI	   R10,		IR1
	BuD	   R11
	   MPYF3   R6,		R0,		R1
	   STF	   R1,	       *AR5
	   STI	   R1,	      *+AR5(1)
			align
		__procname_VfsCopy:
			byte	"VfsCopy", 0
			align
			      word 	0xff000000 | - __procname_VfsCopy * 4
			       export	.VfsCopy	
	.VfsCopy:
	LSH3	  -2,			R1,		AR5
	LSH3	  -2,			R3,		AR0
	ADDI	   IR0,		AR5
	LDF	*++AR0(IR0),	R1
	STI	   IR0,	    *--AR6(1)
	SUBI3	   2,		    	R0,		RC
	RPTBD	   vfscopy_loop
	   STI	   IR1,	    *--AR6(1)
	   LDI	   R2,	    	IR0
	   LDI	 *+AR6(2),		IR1
vfscopy_loop:
	 LDF	*++AR0(IR1),	R1
	 || STF	   R1,	       *AR5++(IR0)
	BuD	   R11
           STF	   R1,	       *AR5
	   LDI	  *AR6++(1),	        IR1
	   LDI	  *AR6++(1),		IR0
			align
		__procname_VdsCopy:
			byte	"VdsCopy", 0
			align
			      word 	0xff000000 | - __procname_VdsCopy * 4
			       export	.VdsCopy	
	.VdsCopy:
	LSH3	  -2,			R1,		AR5
	LSH3	  -2,			R3,		AR0
	LDI	*++AR0(IR0),	R1
	STI	   R1,	     *++AR5(IR0)
	|| LDI	 *+AR0(1),		R3
	STI	   IR0,	    *--AR6(1)
	SUBI3	   2,		    	R0,		RC
	RPTBD	   vdscopy_loop
	   STI	   IR1,	    *--AR6(1)
	   LSH3	   1,		        R2,	    	IR0
	   LSH3	   1,		      *+AR6(2),	IR1
	 STI	   R3,	      *+AR5(1)
	 || LDI	*++AR0(IR1),      R1
vdscopy_loop:
	 STI	   R1,	     *++AR5(IR0)
	 || LDI	 *+AR0(1),		R3
	BuD	   R11
           STI	   R3,	      *+AR5(1)
	   LDI	  *AR6++(1),	        IR1
	   LDI	  *AR6++(1),		IR0
			align
		__procname_VfsFill:
			byte	"VfsFill", 0
			align
			      word 	0xff000000 | - __procname_VfsFill * 4
			       export	.VfsFill	
	.VfsFill:
	STI	  R0,	    *--AR6(1)
	LDF	 *AR6++(1),	R0
	LDI	  R1,		R0
	RND	  R0
	LSH3	 -2,		R3,	AR5
	SUBI3	  2,		R2,	RC
	RPTBD	  vfsfill_loop
	   STF	  R0,      *++AR5(IR0)
	   STI	  IR1,    *--AR6(1)
	   LDI	*+AR6(1),	IR1
vfsfill_loop:
	 STF	  R0,	     *++AR5(IR1)
	LDI	 *AR6++(1),	IR1
	Bu	  R11
			align
		__procname_VdsFill:
			byte	"VdsFill", 0
			align
			      word 	0xff000000 | - __procname_VdsFill * 4
			       export	.VdsFill	
	.VdsFill:
	STI	  R0,	    *--AR6(1)
	LDF	 *AR6++(1),	R0
	LSH3	 -2,		R3,	AR5
	LDI	  R1,		R0
	SUBI3	  2,		R2,	RC
	STF	  R0,      *++AR5(IR0)
	RPTBD	  vdsfill_loop
	   STI	  R0,       *+AR5(1)
	   STI	  IR1,    *--AR6(1)
	   LSH3	  1,          *+AR6(1),	IR1
	 STF	  R0,	     *++AR5(IR1)
vdsfill_loop:
	 STI	  R0,	      *+AR5(1)
	LDI	 *AR6++(1),	IR1
	Bu	  R11
			align
		__procname_VfsMax:
			byte	"VfsMax", 0
			align
			      word 	0xff000000 | - __procname_VfsMax * 4
			       export	.VfsMax	
	.VfsMax:
	LSH3	  -2,			R1,		AR5
	LDI	   IR1,	        R10
	LDI	   R2,		IR1
	SUBI3	   2,			R0,		RC
	RPTBD	   vfsmax_loop
	   LDF	*++AR5(IR0),	R2
	   LDI	   0,			R1
	   LDI	   0,			R0
	 LDF	*++AR5(IR1),	R3
	 ADDI	   1,			R1
	 CMPF	   R2,		R3
	 LDIgt	   R1,		R0
vfsmax_loop:
	 LDFgt	   R3,		R2
	LDI	   R10,		IR1
	Bu	   R11
			align
		__procname_VdsMax:
			byte	"VdsMax", 0
			align
			      word 	0xff000000 | - __procname_VdsMax * 4
			       export	.VdsMax	
	.VdsMax:
	LSH3	  -2,			R1,		AR5
	LDI	   IR1,	        R10
	LSH3	   1,		        R2,		IR1
	SUBI3	   2,			R0,		RC
	LDF	*++AR5(IR0),	R2
	RPTBD	   vdsmax_loop
	   LDI	   0,			R1
	   LDI	 *+AR5(1),		R2
	   LDI	   0,			R0
	 LDF	*++AR5(IR1),	R3
	 LDI	 *+AR5(1),		R3
	 ADDI	   1,			R1
	 CMPF	   R2,		R3
	 LDIgt	   R1,		R0
vdsmax_loop:
	 LDFgt	   R3,		R2
	LDI	   R10,		IR1
	Bu	   R11
			align
		__procname_VfsMin:
			byte	"VfsMin", 0
			align
			      word 	0xff000000 | - __procname_VfsMin * 4
			       export	.VfsMin	
	.VfsMin:
	LSH3	  -2,			R1,		AR5
	LDI	   IR1,	        R10
	LDI	   R2,		IR1
	SUBI3	   2,			R0,		RC
	RPTBD	   vfsmin_loop
	   LDF	*++AR5(IR0),	R2
	   LDI	   0,			R1
	   LDI	   0,			R0
	 LDF	*++AR5(IR1),	R3
	 ADDI	   1,			R1
	 CMPF	   R2,		R3
	 LDIlt	   R1,		R0
vfsmin_loop:
	 LDFlt	   R3,		R2
	LDI	   R10,		IR1
	Bu	   R11
			align
		__procname_VdsMin:
			byte	"VdsMin", 0
			align
			      word 	0xff000000 | - __procname_VdsMin * 4
			       export	.VdsMin	
	.VdsMin:
	LSH3	  -2,			R1,		AR5
	LDI	   IR1,		R10
	LSH3	   1,		        R2,		IR1
	SUBI3	   2,			R0,		RC
	LDF	*++AR5(IR0),	R2
	RPTBD	   vdsmin_loop
	   LDI	   0,			R1
	   LDI	 *+AR5(1),		R2
	   LDI	   0,			R0
	 LDF	*++AR5(IR1),	R3
	 LDI	 *+AR5(1),		R3
	 ADDI	   1,			R1
	 CMPF	   R2,		R3
	 LDIlt	   R1,		R0
vdsmin_loop:
	 LDFlt	   R3,		R2
	LDI	   R10,		IR1
	Bu	   R11
			align
		__procname_VfsAmax:
			byte	"VfsAmax", 0
			align
			      word 	0xff000000 | - __procname_VfsAmax * 4
			       export	.VfsAmax	
	.VfsAmax:
	LSH3	  -2,			R1,		AR5
	LDI	   IR1,	        R10
	LDI	   R2,		IR1
	SUBI3	   2,			R0,		RC
	RPTBD	   vfsamax_loop
	   ABSF	*++AR5(IR0),	R2	   
	   LDI	   0,			R1
	   LDI	   0,			R0
	 ABSF	*++AR5(IR1),	R3
	 ADDI	   1,			R1
	 CMPF	   R2,		R3
	 LDIgt	   R1,		R0
vfsamax_loop:
	 LDFgt	   R3,		R2
	LDI	   R10,		IR1
	Bu	   R11
			align
		__procname_VdsAmax:
			byte	"VdsAmax", 0
			align
			      word 	0xff000000 | - __procname_VdsAmax * 4
			       export	.VdsAmax	
	.VdsAmax:
	LSH3	  -2,			R1,		AR5
	LDI	   IR1,	        R10
	LSH3	   1,		        R2,		IR1
	SUBI3	   2,			R0,		RC
	LDF	*++AR5(IR0),	R2
	LDI	   0,			R1
	RPTBD	   vdsamax_loop
	   LDI	   0,			R0
	   LDI	 *+AR5(1),		R2
	   ABSF	   R2
	 LDF	*++AR5(IR1),	R3
	 LDI	 *+AR5(1),		R3
	 ABSF	   R3
	 ADDI	   1,			R1
	 CMPF	   R2,		R3
	 LDIgt	   R1,		R0
vdsamax_loop:
	 LDFgt	   R3,		R2
	LDI	   R10,		IR1
	Bu	   R11
			align
		__procname_VfsAmin:
			byte	"VfsAmin", 0
			align
			      word 	0xff000000 | - __procname_VfsAmin * 4
			       export	.VfsAmin	
	.VfsAmin:
	LSH3	  -2,			R1,		AR5
	LDI	   IR1,	        R10
	LDI	   R2,		IR1
	SUBI3	   2,			R0,		RC
	RPTBD	   vfsamin_loop
	   ABSF	*++AR5(IR0),	R2	   
	   LDI	   0,			R1
	   LDI	   0,			R0
	 ABSF	*++AR5(IR1),	R3
	 ADDI	   1,			R1
	 CMPF	   R2,		R3
	 LDIlt	   R1,		R0
vfsamin_loop:
	 LDFlt	   R3,		R2
	LDI	   R10,		IR1
	Bu	   R11
			align
		__procname_VdsAmin:
			byte	"VdsAmin", 0
			align
			      word 	0xff000000 | - __procname_VdsAmin * 4
			       export	.VdsAmin	
	.VdsAmin:
	LSH3	  -2,			R1,		AR5
	LDI	   IR1,	        R10
	LSH3	   1,		        R2,		IR1
	SUBI3	   2,			R0,		RC
	LDF	*++AR5(IR0),	R2
	LDI	   0,			R1
	RPTBD	   vdsamin_loop
	   LDI	   0,			R0
	   LDI	 *+AR5(1),		R2
	   ABSF	   R2
	 LDF	*++AR5(IR1),	R3
	 LDI	 *+AR5(1),		R3
	 ABSF	   R3
	 ADDI	   1,			R1
	 CMPF	   R2,		R3
	 LDIlt	   R1,		R0
vdsamin_loop:
	 LDFlt	   R3,		R2
	LDI	   R10,		IR1
	Bu	   R11
			align
		__procname_VfsDot:
			byte	"VfsDot", 0
			align
			      word 	0xff000000 | - __procname_VfsDot * 4
			       export	.VfsDot	
	.VfsDot:
	LSH3	     -2,		  R1,			AR5
	LSH3	     -2,		  R3,			AR0
	MPYF3	   *++AR5(IR0),  *++AR0(IR0),	R1
	STI	      IR1,	      *--AR6(1)
	SUBI3	      2,		  R0,			RC	
	LDF	      0.0,		  R3
	RPTBD	      vfsdot_loop
	   STI	      IR0,	      *--AR6(1)
	   LDI	      R2,		  IR0
	   LDI	    *+AR6(2),		  IR1
vfsdot_loop:
	 MPYF3	   *++AR5(IR0),  *++AR0(IR1),	R1
	 || ADDF3     R1,		  R3,			R3
	ADDF3	      R1,		  R3,			R0
	BuD	      R11
	   RND	      R0
	   LDI	     *AR6++(1),	  IR0
	   LDI	     *AR6++(1),	  IR1
			align
		__procname_VdsDot:
			byte	"VdsDot", 0
			align
			      word 	0xff000000 | - __procname_VdsDot * 4
			       export	.VdsDot	
	.VdsDot:
	LSH3	     -2,		  R1,			AR5
	LSH3	     -2,		  R3,			AR0
	LDF	   *++AR5(IR0),	  R1
	|| LDF	   *++AR0(IR0),	  R3
	SUBI3	      2,		  R0,			RC	
	STI	      IR1,	      *--AR6(1)
	LDI	    *+AR5(1),	  R1
	|| LDI	    *+AR0(1),	  R3
	MPYF3	      R3,		  R1,			R0
	RPTBD	      vdsdot_loop
	   STI	      IR0,	      *--AR6(1)
	   LSH3	      1,	          R2,			IR0
	   LSH3	      1,	        *+AR6(2),		IR1         
	 LDF	   *++AR5(IR0),	  R1
	 || LDF	   *++AR0(IR1),	  R3
	 LDI	    *+AR5(1),	  R1
	 || LDI	    *+AR0(1),	  R3
	 MPYF	      R3,		  R1
vdsdot_loop:
	 ADDF	      R1,		  R0
	BuD	      R11
	   LDI	     *AR6++(1),	  IR0
	   NOP
	   LDI	     *AR6++(1),	  IR1
			align
		__procname_VfsSum:
			byte	"VfsSum", 0
			align
			      word 	0xff000000 | - __procname_VfsSum * 4
			       export	.VfsSum	
	.VfsSum:
	LSH3	  -2,		   R1,	AR5
	SUBI3	   2,		   R0,	RC
	RPTBD	   vfssum_loop
	   LDF	*++AR5(IR0), R0
	   LDI	   IR1,	   R1
	   LDI	   R2,	   IR1
vfssum_loop:
	 ADDF	*++AR5(IR1),  R0
	BuD	   R11
	   RND	   R0
	   LDI	   R1,	   IR1
	   NOP
			align
		__procname_VdsSum:
			byte	"VdsSum", 0
			align
			      word 	0xff000000 | - __procname_VdsSum * 4
			       export	.VdsSum	
	.VdsSum:
	LSH3	  -2,		   R1,	AR5
	SUBI3	   2,		   R0,	RC
	LDF	*++AR5(IR0), R0
	RPTBD	   vdssum_loop
	   LDI	 *+AR5(1),      R0
	   LDI	   IR1,	   R1
	   LSH3	   1,		   R2,	IR1
	 LDF	*++AR5(IR1),  R2
	 LDI	 *+AR5(1),      R2
vdssum_loop:
	 ADDF	   R2,	   R0
	LDI	   R1,	   IR1
	Bu	   R11
			align
		__procname_VfsProd:
			byte	"VfsProd", 0
			align
			      word 	0xff000000 | - __procname_VfsProd * 4
			       export	.VfsProd	
	.VfsProd:
	LSH3	  -2,		   R1,	AR5
	SUBI3	   2,		   R0,	RC
	RPTBD	   vfsprod_loop
	   LDF	*++AR5(IR0), R0
	   LDI	   IR1,	   R1
	   LDI	   R2,	   IR1
vfsprod_loop:
	 MPYF	*++AR5(IR1),  R0
	BuD	   R11
	   RND	   R0
	   LDI	   R1,	   IR1
	   NOP
			align
		__procname_VdsProd:
			byte	"VdsProd", 0
			align
			      word 	0xff000000 | - __procname_VdsProd * 4
			       export	.VdsProd	
	.VdsProd:
	LSH3	  -2,		   R1,	AR5
	SUBI3	   2,		   R0,	RC
	LDF	*++AR5(IR0), R0
	RPTBD	   vdsprod_loop
	   LDI	 *+AR5(1),      R0
	   LDI	   IR1,	   R1
	   LSH3	   1,		   R2,	IR1
	 LDF	*++AR5(IR1),  R2
	 LDI	 *+AR5(1),      R2
vdsprod_loop:
	 MPYF	   R2,	   R0
	LDI	   R1,	   IR1
	Bu	   R11
			align
		__procname_VfsMulAdd:
			byte	"VfsMulAdd", 0
			align
			      word 	0xff000000 | - __procname_VfsMulAdd * 4
			       export	.VfsMulAdd	
	.VfsMulAdd:
	STI	     R0,	    *--AR6(1)
	LDF	    *AR6++(1),	R0
	LSH3	    -2,			R3,		AR5
	SUBI3	     2,			R2,		RC
	LSH3        -2,		      *+AR6(1),	AR0
	LDI	     R1,		R0
	LDI	     IR1,		R10
	MPYF3     *++AR0(IR0),	R0,		R1
	ADDF	  *++AR5(IR0),	R1
	RPTBD	     vfsmuladd_loop
	   LDI	   *+AR6(2),		IR1
	   LDI	     IR0,		AR1
	   LDI	    *AR6,		IR0
	 RND	     R1
	 STF	     R1,	       *AR5++(IR0)
	 || MPYF3 *++AR0(IR1),	R0,		R1
vfsmuladd_loop:
	 ADDF	    *AR5,		R1
        LDI	     AR1,		IR0
	BuD	     R11
	   RND	     R1
	   STF	     R1,	       *AR5
	   LDI	     R10,	        IR1
			align
		__procname_VdsMulAdd:
			byte	"VdsMulAdd", 0
			align
			      word 	0xff000000 | - __procname_VdsMulAdd * 4
			       export	.VdsMulAdd	
	.VdsMulAdd:
	STI	     R0,	    *--AR6(1)
	LDF	    *AR6++(1),	R0
	LSH3	    -2,			R3,		AR5
	SUBI3	     2,			R2,		RC
	LSH3        -2,		      *+AR6(1),	AR0
	LDI	     R1,		R0
	LDF	  *++AR0(IR0),	R1
	|| LDF    *++AR5(IR0),	R2
	LDI	     IR0,		AR1
	LDI	     IR1,		R10
	LSH3	     1,		      *+AR6(0),	IR0
	RPTBD	     vdsmuladd_loop
	   LSH3	     1,		      *+AR6(2),	IR1
	   LDI	   *+AR0(1),	R1
	   || LDI  *+AR5(1),		R2
	   MPYF      R0,		R1
	 ADDF	     R2,	        R1
	 STI	     R1,	      *+AR5(1)
	 STF	     R1,	       *AR5++(IR0)
	 LDF	  *++AR0(IR1),	R1
	 || LDF     *AR5,		R2
	 LDI	   *+AR0(1),	R1
	 || LDI    *+AR5(1),		R2
vdsmuladd_loop:
	 MPYF        R0,		R1
	ADDF	     R2,		R1
	STI	     R1,	      *+AR5(1)
	BuD	     R11
	   STF	     R1,	       *AR5
	   LDI	     AR1,	        IR0
	   LDI	     R10,		IR1
init
	Bu	R11
		data .MaxData, 0
		codetable .MaxCodeP
	align		
.ModEnd:
		end
