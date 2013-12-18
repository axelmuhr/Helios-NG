******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 td2cp.c td2cp.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o td2cp.if td2cp.asm td2cp.tmp 
	.version	40
FP	.set		AR3
	.file	"td2cp.c"
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
	.file	"td2cp.c"
	.globl	_i_mesg
	.globl	_o_mesg

	.sect	".cinit"
	.word	1,_o_err
	.word	0

	.sym	_o_err,_o_err,4,2,32
	.globl	_o_err
	.bss	_o_err,1
	.globl	_out_port
	.text

	.sym	_td2cp,_td2cp,32,2,0
	.globl	_td2cp

	.func	11
;>>>> 	void td2cp(void)
******************************************************
* FUNCTION DEF : _td2cp
******************************************************
_td2cp:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_i,1,4,1,32
	.line	3
;>>>> 	  int i=0;
;>>>> 	  do
	STIK	0,*+FP(1)
L1:
	.line	6
;>>>> 	     while (!comm_rec(i,&i_mesg))    /* waiting for the control word */
	LDI	@CONST+0,R0
	PUSH	R0
	LDI	*+FP(1),R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L3
L2:
	.line	8
;>>>> 	      i++;
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	9
;>>>> 	      if (i==numb)
	CMPI	6,R0
	BNZ	L4
	.line	10
;>>>> 		i=0;
	STIK	0,*+FP(1)
L4:
	.line	6
	LDI	@CONST+0,R0
	PUSH	R0
	LDI	*+FP(1),R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	2,SP
	CMPI	0,R0
	BZ	L2
L3:
	.line	12
;>>>> 	    out_port=i_mesg & 0x7;         /* get the outgoing port number */
	LDI	@_i_mesg,R0
	AND	7,R0,R1
	STI	R1,@_out_port
	.line	13
;>>>> 	    if (out_port==done)           /* if the current test done ? */
	CMPI	7,R1
	BZ	EPI0_1
	.line	14
;>>>> 	      break;
	.line	15
;>>>> 	    o_mesg=i_mesg >> field;       /* strip off the used port number */
	LSH	-3,R0,R2
	STI	R2,@_o_mesg
	.line	16
;>>>> 	    if (!comm_sen(out_port, o_mesg))   /* send the control word */
	PUSH	R2
	PUSH	R1
	CALL	_comm_sen
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L8
	.line	18
;>>>> 	      o_err=1;
	STIK	1,@_o_err
	.line	19
;>>>> 	      break;
	B	EPI0_1
L8:
	.line	21
;>>>> 	    while(!comm_rec(i,&i_mesg));     /* recevie the data */
	LDI	@CONST+0,R0
	PUSH	R0
	LDI	*+FP(1),R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	2,SP
	CMPI	0,R0
	BZ	L8
	.line	22
;>>>> 	    o_mesg=i_mesg;
	LDI	@_i_mesg,R0
	STI	R0,@_o_mesg
	.line	23
;>>>> 	    if (!comm_sen(out_port,o_mesg))      /* send the data out */
	PUSH	R0
	LDI	@_out_port,R1
	PUSH	R1
	CALL	_comm_sen
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L9
	.line	25
;>>>> 	      o_err=1;
	STIK	1,@_o_err
	.line	26
;>>>> 	      break;
	B	EPI0_1
L9:
	.line	28
;>>>> 	  } while (out_port!=done);      /* if the current test done ? */
	LDI	@_out_port,R0
	CMPI	7,R0
	BNZ	L1
EPI0_1:
	.line	29
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	39,000000000H,1

	.sym	_i_mesg,_i_mesg,15,2,32
	.globl	_i_mesg
	.bss	_i_mesg,1

	.sym	_out_port,_out_port,4,2,32
	.globl	_out_port
	.bss	_out_port,1

	.sym	_o_mesg,_o_mesg,15,2,32
	.globl	_o_mesg
	.bss	_o_mesg,1
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,1
	.sect	".cinit"
	.word	1,CONST
	.word 	_i_mesg          ;0
	.end
