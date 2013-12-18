******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 td1cp.c td1cp.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o td1cp.if td1cp.asm td1cp.tmp 
	.version	40
FP	.set		AR3
	.file	"td1cp.c"
	.file	"c:\c40\math.h"
	.globl	_asin
	.globl	_acos
	.globl	_atan
	.globl	_atan2
	.globl	_ceil
	.globl	_cos
	.globl	_cosh
	.globl	_exp
	.globl	_fabs
	.globl	_floor
	.globl	_fmod
	.globl	_frexp
	.globl	_ldexp
	.globl	_log
	.globl	_log10
	.globl	_modf
	.globl	_pow
	.globl	_sin
	.globl	_sinh
	.globl	_sqrt
	.globl	_tan
	.globl	_tanh
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
	.file	"td1cp.c"

	.sect	".cinit"
	.word	IS1,_data
	.word	0
	.word	-1
	.word	-1431655766
	.word	1431655765
IS1	.set	4

	.sym	_data,_data,63,2,128,,4
	.globl	_data
	.bss	_data,4

	.word	1,_out_2
	.word	1

	.sym	_out_2,_out_2,15,2,32
	.globl	_out_2
	.bss	_out_2,1

	.word	IS2,_rout
	.word	43
	.word	99
	.word	35
	.word	3
	.word	3
	.word	35
IS2	.set	6

	.sym	_rout,_rout,63,2,192,,6
	.globl	_rout
	.bss	_rout,6
	.globl	_i_mesg

	.word	IS3,_port
	.word	0
	.word	1
	.word	2
IS3	.set	3

	.sym	_port,_port,52,2,96,,3
	.globl	_port
	.bss	_port,3

	.word	1,_o_err
	.word	0

	.sym	_o_err,_o_err,4,2,32
	.globl	_o_err
	.bss	_o_err,1

	.word	1,_t_err
	.word	0

	.sym	_t_err,_t_err,4,2,32
	.globl	_t_err
	.bss	_t_err,1

	.word	1,_out_port
	.word	3

	.sym	_out_port,_out_port,4,2,32
	.globl	_out_port
	.bss	_out_port,1
	.text

	.sym	_td1cp,_td1cp,32,2,0
	.globl	_td1cp

	.func	19
;>>>> 	void td1cp( int daughter )
******************************************************
* FUNCTION DEF : _td1cp
******************************************************
_td1cp:
	PUSH	FP
	LDI	SP,FP
	ADDI	3,SP
	.sym	_daughter,-2,4,9,32
	.sym	_i,1,4,1,32
	.sym	_j,2,4,1,32
	.sym	_k,3,4,1,32
	.line	2
	.line	3
;>>>> 	  int i,j,k=0;
	STIK	0,*+FP(3)
	.line	5
;>>>> 	  if (daughter)             /* Test All Four Processors */
	LDI	*-FP(2),R0
	BZ	L1
	.line	8
;>>>> 	   for (i=0;i<nu_rout;i++)
	STIK	0,*+FP(1)
	CMPI	6,*+FP(1)
	BGE	L3
L2:
	.line	10
;>>>> 	    if (!fmod(i,2))               /* if it is a forword or backword test ? */
	LDF	2.0,R0
	PUSHF	R0
	FLOAT	*+FP(1),R1
	PUSHF	R1
	CALL	_fmod
	SUBI	2,SP
	CMPF	0,R0
	BNZ	L4
	.line	11
;>>>> 	      out_port=0;
;>>>> 	    else 
	STIK	0,@_out_port
	B	L5
L4:
	.line	13
;>>>> 	      out_port=k;
	LDI	*+FP(3),R0
	STI	R0,@_out_port
L5:
	.line	14
;>>>> 	    for ( j=0;j<nu_patn;j++)
	STIK	0,*+FP(2)
	CMPI	4,*+FP(2)
	BGE	L7
L6:
	.line	16
;>>>> 	        if (!comm_sen(out_port,rout[i]))   /* send the control word out */
	LDA	*+FP(1),IR0
	LDA	@CONST+0,AR0
	LDI	*+AR0(IR0),R0
	PUSH	R0
	LDI	@_out_port,R0
	PUSH	R0
	CALL	_comm_sen
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L8
	.line	18
;>>>> 	          o_err=1;
	STIK	1,@_o_err
	.line	19
;>>>> 	   	  break;
	B	L7
L8:
	.line	21
;>>>> 		if (!comm_sen(out_port,data[j]))   /* send the data out */	
	LDA	*+FP(2),IR1
	LDA	@CONST+1,AR0
	LDI	*+AR0(IR1),R0
	PUSH	R0
	LDI	@_out_port,R0
	PUSH	R0
	CALL	_comm_sen
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L9
	.line	23
;>>>> 		  o_err=1;
	STIK	1,@_o_err
	.line	24
;>>>> 	          break;
;>>>> 	       while (!comm_rec(port[k],&i_mesg))   /* wait for the control word coming back */
	B	L7
L9:
	B	L11
L10:
	.line	28
;>>>> 	          k++;
	ADDI	1,*+FP(3),R0
	STI	R0,*+FP(3)
	.line	29
;>>>> 	          if (k==numb)
	CMPI	3,R0
	BNZ	L11
	.line	30
;>>>> 	          k=0;
	STIK	0,*+FP(3)
L11:
	.line	26
	LDI	@CONST+2,R0
	PUSH	R0
	LDA	*+FP(3),IR0
	LDA	@CONST+3,AR0
	LDI	*+AR0(IR0),R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	2,SP
	CMPI	0,R0
	BZ	L10
L13:
	.line	32
;>>>> 	        while (!comm_rec(port[k],&i_mesg));   /* dump the control word and get the data coming back */
	LDI	@CONST+2,R0
	PUSH	R0
	LDA	*+FP(3),IR1
	LDA	@CONST+3,AR0
	LDI	*+AR0(IR1),R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	2,SP
	CMPI	0,R0
	BZ	L13
	.line	34
;>>>> 	        if (i_mesg!=data[j])       /* is there any error in the received data */
	LDA	*+FP(2),IR0
	LDA	@CONST+1,AR0
	LDI	@_i_mesg,R0
	CMPI	*+AR0(IR0),R0
	BZ	L14
	.line	36
;>>>> 	          t_err=1;
	STIK	1,@_t_err
	.line	37
;>>>> 	          break;
	B	L7
L14:
	.line	14
	ADDI	1,*+FP(2),R1
	STI	R1,*+FP(2)
	CMPI	4,R1
	BLT	L6
L7:
	.line	40
;>>>> 	      if( o_err==1||t_err==1)    /* if error happened, break out */ 
	LDI	@_o_err,R0
	CMPI	1,R0
	BZ	LL3
	LDI	@_t_err,R1
	CMPI	1,R1
	BNZ	L15
LL3:
	.line	41
;>>>> 		break;
	B	L3
L15:
	.line	8
	ADDI	1,*+FP(1),R2
	STI	R2,*+FP(1)
	CMPI	6,R2
	BLT	L2
L3:
	.line	44
;>>>> 	    for (i=0;i<numb;i++)
	STIK	0,*+FP(1)
	CMPI	3,*+FP(1)
	BGE	L17
L16:
	.line	45
;>>>> 	      if (!comm_sen(i,done))      /* send the "done" flag to all other DSPs */
	LDI	7,R0
	PUSH	R0
	LDI	*+FP(1),R1
	PUSH	R1
	CALL	_comm_sen
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L18
	.line	47
;>>>> 	        o_err=1;
	STIK	1,@_o_err
	.line	48
;>>>> 	        break;
;>>>> 	 else            /* Only test DSP 2 */
	B	L17
L18:
	.line	44
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	3,R0
	BLT	L16
L17:
	B	EPI0_1
L1:
	.line	54
;>>>> 	     for ( j=0;j<nu_patn;j++)
	STIK	0,*+FP(2)
	CMPI	4,*+FP(2)
	BGE	L21
L20:
	.line	56
;>>>> 	        if (!comm_sen(out_port,out_2))   /* send the control word out */
	LDI	@_out_2,R0
	PUSH	R0
	LDI	@_out_port,R1
	PUSH	R1
	CALL	_comm_sen
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L22
	.line	58
;>>>> 	          o_err=1;
	STIK	1,@_o_err
	.line	59
;>>>> 	   	  break;
	B	L21
L22:
	.line	61
;>>>> 		if (!comm_sen(out_port,data[j]))   /* send the data out */	
	LDA	*+FP(2),IR1
	LDA	@CONST+1,AR0
	LDI	*+AR0(IR1),R0
	PUSH	R0
	LDI	@_out_port,R0
	PUSH	R0
	CALL	_comm_sen
	SUBI	2,SP
	CMPI	0,R0
	BNZ	L24
	.line	63
;>>>> 		  o_err=1;
	STIK	1,@_o_err
	.line	64
;>>>> 		  break;
	B	L21
L24:
	.line	66
;>>>> 	       while (!comm_rec(out_port,&i_mesg));   /* wait for the control word coming back */
	LDI	@CONST+2,R0
	PUSH	R0
	LDI	@_out_port,R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	2,SP
	CMPI	0,R0
	BZ	L24
L25:
	.line	67
;>>>> 	       while (!comm_rec(out_port,&i_mesg));   /* dump the control word and get the data coming back */
	LDI	@CONST+2,R0
	PUSH	R0
	LDI	@_out_port,R1
	PUSH	R1
	CALL	_comm_rec
	SUBI	2,SP
	CMPI	0,R0
	BZ	L25
	.line	69
;>>>> 	       if (i_mesg!=data[j])       /* is there any error in the received data */
	LDA	*+FP(2),IR0
	LDA	@CONST+1,AR0
	LDI	@_i_mesg,R0
	CMPI	*+AR0(IR0),R0
	BZ	L26
	.line	71
;>>>> 	          t_err=1;
	STIK	1,@_t_err
	.line	72
;>>>> 	          break;
	B	L21
L26:
	.line	54
	ADDI	1,*+FP(2),R1
	STI	R1,*+FP(2)
	CMPI	4,R1
	BLT	L20
L21:
	.line	75
;>>>> 	      if (!comm_sen(out_port,done))      /* send the "done" flag to DSP2 */
	LDI	7,R0
	PUSH	R0
	LDI	@_out_port,R1
	PUSH	R1
	CALL	_comm_sen
	SUBI	2,SP
	CMPI	0,R0
	BNZ	EPI0_1
	.line	76
;>>>> 	        o_err=1;
	STIK	1,@_o_err
EPI0_1:
	.line	78
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	5,SP
	B	R1
	.endfunc	96,000000000H,3

	.sym	_i_mesg,_i_mesg,15,2,32
	.globl	_i_mesg
	.bss	_i_mesg,1
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,4
	.sect	".cinit"
	.word	4,CONST
	.word 	_rout            ;0
	.word 	_data            ;1
	.word 	_i_mesg          ;2
	.word 	_port            ;3
	.end
