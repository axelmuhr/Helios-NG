******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 setup_sr.c C:\TMP\setup_sr.if
;	cg30 -v40 -o -n C:\TMP\setup_sr.if C:\TMP\setup_sr.asm C:\TMP\setup_sr.tmp
FP	.set	AR3
	.globl	_setup
;>>>> 	void setup( void (*key_handler)(void) )
;>>>> 		void (*i)();
******************************************************
* FUNCTION DEF : _setup
******************************************************
_setup:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP

	LDI	*-FP(2),R0			;Get pointer to keyboard handler routine
	ldi	ivtp,ar0
	sti	r0,*+ar0(1)			;IIOF0 pointer

;>>>> 		return;
EPI0_1:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS
	.end
