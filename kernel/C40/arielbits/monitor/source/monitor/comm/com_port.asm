******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 com_port.c com_port.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o com_port.if com_port.asm com_port.tmp 
	.version	40
FP	.set		AR3
	.file	"com_port.c"
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
	.file	"com_port.c"

	.sect	".cinit"
	.word	IS1,_o_fifo
	.word	1048642
	.word	1048658
	.word	1048674
	.word	1048690
	.word	1048706
	.word	1048722
IS1	.set	6

	.sym	_o_fifo,_o_fifo,52,2,192,,6
	.globl	_o_fifo
	.bss	_o_fifo,6

	.word	IS2,_i_fifo
	.word	1048641
	.word	1048657
	.word	1048673
	.word	1048689
	.word	1048705
	.word	1048721
IS2	.set	6

	.sym	_i_fifo,_i_fifo,52,2,192,,6
	.globl	_i_fifo
	.bss	_i_fifo,6
	.text

	.sym	_comm_sen,_comm_sen,36,2,0
	.globl	_comm_sen

	.func	193
;>>>> 	  int comm_sen(int no, unsigned long omessage)
******************************************************
* FUNCTION DEF : _comm_sen
******************************************************
_comm_sen:
	PUSH	FP
	LDI	SP,FP
	.sym	_no,-2,4,9,32
	.sym	_omessage,-3,15,9,32
	.line	2
	.line	3
;>>>> 	     if (o_crdy(no))
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_o_crdy
	SUBI	1,SP
	CMPI	0,R0
	BZ	L1
	.line	5
;>>>> 	        *((unsigned long *)o_fifo[no])=omessage;
	LDA	*-FP(2),IR0
	LDA	@CONST+0,AR0
	LDA	*+AR0(IR0),AR1
	LDI	*-FP(3),R0
	STI	R0,*AR1
	.line	6
;>>>> 	        return  1;
;>>>> 	     else
	LDI	1,R0
	B	EPI0_1
L1:
	.line	10
;>>>> 	        return  0;
	LDI	0,R0
EPI0_1:
	.line	11
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	203,000000000H,0

	.sym	_comm_rec,_comm_rec,36,2,0
	.globl	_comm_rec

	.func	205
;>>>> 	   int comm_rec(int no, unsigned long *imessage)
******************************************************
* FUNCTION DEF : _comm_rec
******************************************************
_comm_rec:
	PUSH	FP
	LDI	SP,FP
	.sym	_no,-2,4,9,32
	.sym	_imessage,-3,31,9,32
	.line	2
	.line	3
;>>>> 	      if (i_crdy(no))
	LDI	*-FP(2),R0
	PUSH	R0
	CALL	_i_crdy
	SUBI	1,SP
	CMPI	0,R0
	BZ	L2
	.line	5
;>>>> 	          *imessage=*((unsigned long *)i_fifo[no]);
	LDA	*-FP(2),IR1
	LDA	@CONST+1,AR0
	LDA	*+AR0(IR1),AR1
	LDA	*-FP(3),AR2
	LDI	*AR1,R0
	STI	R0,*AR2
	.line	6
;>>>> 	          return  1;
;>>>> 	       else
	LDI	1,R0
	B	EPI0_2
L2:
	.line	9
;>>>> 	           return  0;
	LDI	0,R0
EPI0_2:
	.line	10
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	214,000000000H,0

	.sym	_com_init,_com_init,32,2,0
	.globl	_com_init

	.func	216
;>>>> 	    void com_init(void)
******************************************************
* FUNCTION DEF : _com_init
******************************************************
_com_init:
	PUSH	FP
	LDI	SP,FP
EPI0_3:
	.line	3
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	218,000000000H,0
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,2
	.sect	".cinit"
	.word	2,CONST
	.word 	_o_fifo          ;0
	.word 	_i_fifo          ;1
	.end
