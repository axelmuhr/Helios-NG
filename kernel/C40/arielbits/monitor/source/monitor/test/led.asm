******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 led.c led.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o led.if led.asm led.tmp 
	.version	40
FP	.set		AR3
	.file	"led.c"

	.sym	_LED,_LED,32,2,0
	.globl	_LED

	.func	8
;>>>> 	void LED( int which, int on_off )
******************************************************
* FUNCTION DEF : _LED
******************************************************
_LED:
	PUSH	FP
	LDI	SP,FP
	.sym	_which,-2,4,9,32
	.sym	_on_off,-3,4,9,32
	.line	2
;>>>> 		switch( which )
;>>>> 			case GREEN:
;>>>> 				switch( on_off )
;>>>> 					case ON:
	B	L1
L2:
	B	L3
L4:
	.line	10
;>>>> 						*((unsigned long *)0xBF7FC008)=*((unsigned long *)0xBFFD8000)|0x80000000;
	LDA	@CONST+0,AR0
	LDI	@CONST+1,R0
	OR	R0,*AR0,R1
	LDA	@CONST+2,AR1
	STI	R1,*AR1
	.line	11
;>>>> 						break;
;>>>> 					case OFF:
	B	L5
L6:
	.line	13
;>>>> 						*((unsigned long *)0xBF7FC008)=*((unsigned long *)0xBFFD8000)&0x7FFFFFFF;
	LDA	@CONST+0,AR0
	LDI	@CONST+3,R0
	AND	R0,*AR0,R1
	LDA	@CONST+2,AR1
	STI	R1,*AR1
	.line	14
;>>>> 						break;
;>>>> 				};
	B	L5
L3:
	.line	7
	LDI	*-FP(3),R0
	BZ	L6
	CMPI	1,R0
	BZ	L4
L5:
	.line	16
;>>>> 				break;
;>>>> 			case RED:
;>>>> 				switch( on_off )
;>>>> 					case ON:
	B	EPI0_1
L8:
	B	L9
L10:
	.line	21
;>>>> 						*((unsigned long *)0xBF7FC008)=*((unsigned long *)0xBFFD8000)|0x00200000;
	LDA	@CONST+0,AR0
	LDI	@CONST+4,R0
	OR	R0,*AR0,R1
	LDA	@CONST+2,AR1
	STI	R1,*AR1
	.line	22
;>>>> 						break;
;>>>> 					case OFF:
	B	L11
L12:
	.line	24
;>>>> 						*((unsigned long *)0xBF7FC008)=*((unsigned long *)0xBFFD8000)&0xFFDFFFFF;
	LDA	@CONST+0,AR0
	LDI	@CONST+5,R0
	AND	R0,*AR0,R1
	LDA	@CONST+2,AR1
	STI	R1,*AR1
	.line	25
;>>>> 						break;
;>>>> 				};
	B	L11
L9:
	.line	18
	LDI	*-FP(3),R0
	BZ	L12
	CMPI	1,R0
	BZ	L10
L11:
	.line	27
;>>>> 				break;
	B	EPI0_1
L1:
	.line	4
	LDI	*-FP(2),R0
	CMPI	1,R0
	BZ	L8
	CMPI	2,R0
	BZ	L2
EPI0_1:
	.line	29
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	36,000000000H,0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,6
	.sect	".cinit"
	.word	6,CONST
	.word 	-1073905664      ;0
	.word 	-2147483648      ;1
	.word 	-1082146808      ;2
	.word 	2147483647       ;3
	.word 	2097152          ;4
	.word 	-2097153         ;5
	.end
