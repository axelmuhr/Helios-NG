******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 dmemtest.c dmemtest.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o dmemtest.if dmemtest.asm dmemtest.tmp 
	.version	40
FP	.set		AR3
	.file	"dmemtest.c"

	.sym	_DMemTest,_DMemTest,47,2,0
	.globl	_DMemTest

	.func	1
;>>>> 	unsigned long DMemTest( unsigned long *base1, unsigned long *base2, unsigned long length1, unsigned long length2 )
******************************************************
* FUNCTION DEF : _DMemTest
******************************************************
_DMemTest:
	PUSH	FP
	LDI	SP,FP
	ADDI	5,SP
	.sym	_base1,-2,31,9,32
	.sym	_base2,-3,31,9,32
	.sym	_length1,-4,15,9,32
	.sym	_length2,-5,15,9,32
	.sym	_i,1,15,1,32
	.sym	_maxlength,2,15,1,32
	.sym	_minlength,3,15,1,32
	.sym	_times,4,15,1,32
	.sym	_fail_addr,5,15,1,32
	.line	2
;>>>> 		unsigned long i, maxlength, minlength, times, fail_addr;
	.line	5
;>>>> 		maxlength = length1>length2?length1:length2;
	LDI	*-FP(4),R0
	CMPI	*-FP(5),R0
	LDIHI	R0,R1
	LDILS	*-FP(5),R1
	STI	R1,*+FP(2)
	.line	6
;>>>> 		minlength = length1<length2?length1:length2;
	CMPI	*-FP(5),R0
	LDILO	R0,R2
	LDIHS	*-FP(5),R2
	STI	R2,*+FP(3)
	.line	7
;>>>> 		times = maxlength / minlength;
	LDI	*+FP(2),R0
	LDI	*+FP(3),R1
	CALL	DIV_U
	STI	R0,*+FP(4)
	.line	9
;>>>> 		for( i=0 ; i < times ; i++ )
	STIK	0,*+FP(1)
	CMPI	R0,*+FP(1)
	BHS	L2
L1:
	.line	10
;>>>> 			if( fail_addr = DualMem( base1+(i*minlength), base2+(i*minlength), minlength ))
	LDI	*+FP(3),R0
	PUSH	R0
	MPYI	R0,*+FP(1),R1
	ADDI	*-FP(3),R1
	PUSH	R1
	MPYI	R0,*+FP(1),R1
	ADDI	*-FP(2),R1
	PUSH	R1
	CALL	_DualMem
	SUBI	3,SP
	STI	R0,*+FP(5)
	CMPI	0,R0
	BNZ	EPI0_1
	.line	11
;>>>> 				return( fail_addr );
	.line	9
	ADDI	1,*+FP(1),R1
	STI	R1,*+FP(1)
	CMPI	*+FP(4),R1
	BLO	L1
L2:
	.line	13
;>>>> 		if( fail_addr = DualMem( base1+(times*minlength), base2+(times*minlength),  maxlength%minlength ) )
	LDI	*+FP(2),R0
	LDI	*+FP(3),R1
	CALL	MOD_U
	PUSH	R0
	MPYI	*+FP(3),*+FP(4),R0
	ADDI	*-FP(3),R0
	PUSH	R0
	MPYI	*+FP(3),*+FP(4),R0
	ADDI	*-FP(2),R0
	PUSH	R0
	CALL	_DualMem
	SUBI	3,SP
	STI	R0,*+FP(5)
	CMPI	0,R0
	.line	14
;>>>> 			return( fail_addr );
EPI0_1:
	.line	15
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	7,SP
	B	R1
	.endfunc	15,000000000H,5
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	DIV_U
	.globl	_DualMem
	.globl	MOD_U
	.end
