******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 tes_d2cp.c tes_d2cp.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o tes_d2cp.if tes_d2cp.asm tes_d2cp.tmp 
	.version	40
FP	.set		AR3
	.file	"tes_d2cp.c"
	.file	"comm.h"
	.globl	_comm_sen
	.globl	_comm_rec
	.globl	_o_isr
	.globl	_i_isr
	.globl	_td1cp
	.globl	_td2cp
	.globl	_com_init
	.globl	_c_int02
	.globl	_c_int03
	.globl	_c_int04
	.globl	_c_int05
	.globl	_c_int06
	.globl	_c_int07
	.globl	_c_int08
	.globl	_c_int09
	.globl	_c_int10
	.globl	_c_int11
	.globl	_c_int12
	.globl	_c_int13
	.globl	_en_port
	.globl	_dis_port
	.globl	_o_crdy
	.globl	_i_crdy
	.file	"tes_d2cp.c"

	.sym	_main,_main,36,2,0
	.globl	_main

	.func	3
;>>>> 	main()
******************************************************
* FUNCTION DEF : _main
******************************************************
_main:
	PUSH	FP
	LDI	SP,FP
	.line	3
;>>>> 	  td2cp();
	CALL	_td2cp
EPI0_1:
	.line	4
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	6,000000000H,0
	.end
