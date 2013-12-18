******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 test_com.c test_com.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o test_com.if test_com.asm test_com.tmp 
	.version	40
FP	.set		AR3
	.file	"test_com.c"
	.file	"comm.h"
	.globl	_comm_sen
	.globl	_comm_rec
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
	.file	"test_com.c"

	.sect	".cinit"
	.word	1,_o_err
	.word	0

	.sym	_o_err,_o_err,4,2,32
	.globl	_o_err
	.bss	_o_err,1

	.word	1,_i_err
	.word	0

	.sym	_i_err,_i_err,4,2,32
	.globl	_i_err
	.bss	_i_err,1
	.globl	_i_mesg
	.globl	_k
	.text

	.sym	_main,_main,36,2,0
	.globl	_main

	.func	9
;>>>> 	main()
******************************************************
* FUNCTION DEF : _main
******************************************************
_main:
	PUSH	FP
	LDI	SP,FP
	.line	5
;>>>> 	    com_init();
	CALL	_com_init
	.line	7
;>>>> 	    for (k=0; k<count1; k++)
	STIK	0,@_k
	LDI	@_k,R0
	CMPI	2,R0
	BGE	L2
L1:
	.line	9
;>>>> 	       if (!comm_sen(2,15))
	LDI	15,R0
	PUSH	R0
	LDI	2,R1
	PUSH	R1
	CALL	_comm_sen
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L3
	.line	10
;>>>> 	          o_err=1;
	STIK	1,@_o_err
L3:
	.line	7
	LDI	@_k,R0
	ADDI	1,R0,R1
	STI	R1,@_k
	CMPI	2,R1
	BLT	L1
L2:
	.line	13
;>>>> 	     for (k=0; k<count2; k++)
	STIK	0,@_k
	LDI	@_k,R0
	CMPI	4,R0
	BGE	EPI0_1
L4:
	.line	15
;>>>> 	        if (!comm_rec(5, &i_mesg)) 
	LDI	@CONST+0,R0
	PUSH	R0
	LDI	5,R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L6
	.line	16
;>>>> 	    	  i_err=1; 
	STIK	1,@_i_err
L6:
	.line	13
	LDI	@_k,R0
	ADDI	1,R0,R1
	STI	R1,@_k
	CMPI	4,R1
	BLT	L4
EPI0_1:
	.line	18
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	26,000000000H,0

	.sym	_i_mesg,_i_mesg,15,2,32
	.globl	_i_mesg
	.bss	_i_mesg,1

	.sym	_k,_k,4,2,32
	.globl	_k
	.bss	_k,1
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,1
	.sect	".cinit"
	.word	1,CONST
	.word 	_i_mesg          ;0
	.end
