******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 setup_sr.c C:\TMP\setup_sr.if
;	cg30 -v40 -o -n C:\TMP\setup_sr.if C:\TMP\setup_sr.asm C:\TMP\setup_sr.tmp
FP	.set	AR3
	.globl	_SetKeyHandler
;>>>> 	void setup( void (*key_handler)(void) )
;>>>> 		void (*i)();
******************************************************
* FUNCTION DEF : _setup
******************************************************
_SetKeyHandler:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP

	ldi	@IRAM0,r1
	ldpe	r1,ivtp

	LDI	*-FP(2),R0	;Get pointer to keyboard handler routine
	ldi	r1,ar0
	sti	r0,*+ar0(1)

	or	2000H,st	;Set GIE

;>>>> 		return;
EPI0_1:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	3,SP
***	B	R1	;BRANCH OCCURS

	.bss	IRAM0,1
	.sect	".cinit"
	.word	1,IRAM0
	.word	2ff800h

	.end
