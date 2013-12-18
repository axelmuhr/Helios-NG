******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 setuptes.c setuptes.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o setuptes.if setuptes.asm setuptes.tmp 
	.version	40
FP	.set		AR3
	.file	"setuptes.c"

	.sym	_main,_main,36,2,0
	.globl	_main

	.func	1
;>>>> 	main()
******************************************************
* FUNCTION DEF : _main
******************************************************
_main:
	PUSH	FP
	LDI	SP,FP
	.line	4
;>>>> 		setupVICVAC();
	CALL	_setupVICVAC
EPI0_1:
	.line	5
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	5,000000000H,0
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_setupVICVAC
	.end
