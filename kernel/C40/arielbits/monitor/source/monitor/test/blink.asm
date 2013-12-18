******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 blink.c blink.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o blink.if blink.asm blink.tmp 
	.version	40
FP	.set		AR3
	.file	"blink.c"

	.sym	_main,_main,36,2,0
	.globl	_main

	.func	8
;>>>> 	main()
;>>>> 		int i;
;>>>> 		while( 1 )
******************************************************
* FUNCTION DEF : _main
******************************************************
_main:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_i,1,4,1,32
L1:
	.line	7
;>>>> 			LED( RED, ON );
	LDI	1,R0
	PUSH	R0
	PUSH	R0
	CALL	_LED
	SUBI	2,SP
	.line	8
;>>>> 			LED( GREEN, OFF );
	LDI	0,R0
	PUSH	R0
	LDI	2,R1
	PUSH	R1
	CALL	_LED
	SUBI	2,SP
	.line	10
;>>>> 			for( i=0 ; i < 0x80000 ; i++ );
	STIK	0,*+FP(1)
	LDI	@CONST+0,R0
	CMPI	R0,*+FP(1)
	BGE	L4
L3:
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	@CONST+0,R0
	BLT	L3
L4:
	.line	12
;>>>> 			LED( RED, OFF );
	LDI	0,R0
	PUSH	R0
	LDI	1,R1
	PUSH	R1
	CALL	_LED
	SUBI	2,SP
	.line	13
;>>>> 			LED( GREEN, ON );
	LDI	1,R0
	PUSH	R0
	LDI	2,R1
	PUSH	R1
	CALL	_LED
	SUBI	2,SP
	.line	15
;>>>> 			for( i=0 ; i < 0x80000 ; i++ );
	STIK	0,*+FP(1)
	LDI	@CONST+0,R0
	CMPI	R0,*+FP(1)
	BGE	L6
L5:
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	@CONST+0,R0
	BLT	L5
L6:
	.line	17
	B	L1
	.endfunc	25,000000000H,1
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,1
	.sect	".cinit"
	.word	1,CONST
	.word 	524288           ;0
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_LED
	.end
