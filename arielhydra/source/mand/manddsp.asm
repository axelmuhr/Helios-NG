******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -q manddsp.c D:manddsp.if 
;	opt30 NOT RUN
;	cg30 -v40 -q D:manddsp.if manddsp.asm D:manddsp.tmp 
	.version	40
FP	.set		AR3

	.sect	".cinit"
	.word	1,_start_flag
	.word	0
	.globl	_start_flag
	.bss	_start_flag,1

	.word	1,_line_done
	.word	0
	.globl	_line_done
	.bss	_line_done,1
	.globl	_width
	.globl	_height
	.globl	_maxit
	.globl	_intpri
	.globl	_intvec

	.word	1,_x_start
	.float	3.14159265359
	.globl	_x_start
	.bss	_x_start,1

	.word	1,_x_step
	.float	-3.14159265359
	.globl	_x_step
	.bss	_x_step,1
	.globl	_y_start
	.globl	_y_step
	.globl	_max_val
	.globl	_y_coord
	.globl	_buf1
	.globl	_buf2
	.globl	_ycoord1
	.globl	_ycoord2

	.word	1,_VIC_virsr
	.word	-1073807328
	.globl	_VIC_virsr
	.bss	_VIC_virsr,1
	.text
	.globl	_main
******************************************************
* FUNCTION DEF : _main
******************************************************
_main:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	CALL	_GIEOn
L3:
	LDI	@_start_flag,R0
	BZ	L3
	LDI	@CONST+0,R0
	PUSH	R0
	LDI	0,R1
	PUSH	R1
	CALL	_process_line
	SUBI	2,SP
	LDI	@_y_coord,R0
	SUBI	1,R0
	STI	R0,@_y_coord
	ADDI	1,R0
	STI	R0,@_ycoord1
	STIK	1,@_line_done
	LDA	@_VIC_virsr,AR0
	LDA	@_intpri,IR0
	LDI	@_intvec,R1
	STI	R1,*+AR0(IR0)
	LDI	1,R1
	LSH	@_intpri,R1
	ADDI	1,R1
	STI	R1,*AR0
	LDI	@_height,R1
	SUBI	1,R1
	STI	R1,@_height
	STIK	0,*+FP(1)
	LDI	@_height,R1
	BZ	L5
L4:
	LDI	@CONST+1,R0
	PUSH	R0
	ADDI	1,*+FP(1),R1
	STI	R1,*+FP(1)
	SUBI	1,R1
	PUSH	R1
	CALL	_process_line
	SUBI	2,SP
	LDI	@_y_coord,R0
	SUBI	1,R0
	STI	R0,@_y_coord
	ADDI	1,R0
	STI	R0,@_ycoord2
L6:
	LDI	@_line_done,R0
	BNZ	L6
	STIK	2,@_line_done
	LDA	@_VIC_virsr,AR0
	LDA	@_intpri,IR1
	LDI	@_intvec,R0
	STI	R0,*+AR0(IR1)
	LDI	1,R0
	LSH	@_intpri,R0
	ADDI	1,R0
	STI	R0,*AR0
	LDI	@_height,R0
	SUBI	1,R0
	STI	R0,@_height
	LDI	@_height,R0
	BZ	L5
	LDI	@CONST+0,R0
	PUSH	R0
	ADDI	1,*+FP(1),R1
	STI	R1,*+FP(1)
	SUBI	1,R1
	PUSH	R1
	CALL	_process_line
	SUBI	2,SP
	LDI	@_y_coord,R0
	SUBI	1,R0
	STI	R0,@_y_coord
	ADDI	1,R0
	STI	R0,@_ycoord1
L8:
	LDI	@_line_done,R0
	BNZ	L8
	STIK	1,@_line_done
	LDA	@_VIC_virsr,AR0
	LDA	@_intpri,IR0
	LDI	@_intvec,R0
	STI	R0,*+AR0(IR0)
	LDI	1,R0
	LSH	@_intpri,R0
	ADDI	1,R0
	STI	R0,*AR0
	LDI	@_height,R0
	SUBI	1,R0
	STI	R0,@_height
	LDI	@_height,R0
	BNZ	L4
L5:
	STIK	0,@_start_flag
	B	L3
	.globl	_process_line
******************************************************
* FUNCTION DEF : _process_line
******************************************************
_process_line:
	PUSH	FP
	LDI	SP,FP
	ADDI	9,SP
	LDF	@_y_start,R0
	STF	R0,*+FP(4)
	STIK	0,*+FP(1)
	LDI	@_width,R1
	CMPI	R1,*+FP(1)
	BHS	L10
L9:
	FLOAT	*+FP(1),R0
	MPYF	@_x_step,R0
	ADDF	@_x_start,R0
	STF	R0,*+FP(3)
	STF	R0,*+FP(5)
	LDF	*+FP(4),R1
	STF	R1,*+FP(6)
	LDI	@_maxit,R2
	STI	R2,*+FP(2)
	BLE	L12
L11:
	MPYF	*+FP(5),*+FP(5),R0
	STF	R0,*+FP(7)
	MPYF	*+FP(6),*+FP(6),R1
	STF	R1,*+FP(8)
	ADDF	R1,R0,R2
	STF	R2,*+FP(9)
	CMPF	4.0,R2
	BGT	L12
	LDF	2.0,R3
	MPYF	R3,*+FP(5),R9
	MPYF	*+FP(6),R9
	ADDF	*+FP(4),R9
	STF	R9,*+FP(6)
	SUBF	R1,R0,R10
	ADDF	*+FP(3),R10
	STF	R10,*+FP(5)
	SUBI	1,*+FP(2),R11
	STI	R11,*+FP(2)
	BGT	L11
L12:
	LDI	125,R0
	MPYI	*+FP(2),R0
	LDI	@_maxit,R1
	CALL	DIV_U
	LDA	*-FP(3),AR0
	ADDI	1,R0
	STI	R0,*AR0++
	STI	AR0,*-FP(3)
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	CMPI	@_width,R0
	BLO	L9
L10:
	LDF	@_y_start,R0
	ADDF	@_y_step,R0
	STF	R0,@_y_start
EPI0_2:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	11,SP
***	B	R1	;BRANCH OCCURS
	.globl	_y_start
	.bss	_y_start,1
	.globl	_intpri
	.bss	_intpri,1
	.globl	_y_coord
	.bss	_y_coord,1
	.globl	_y_step
	.bss	_y_step,1
	.globl	_ycoord1
	.bss	_ycoord1,1
	.globl	_ycoord2
	.bss	_ycoord2,1
	.globl	_height
	.bss	_height,1
	.globl	_intvec
	.bss	_intvec,1
	.globl	_width
	.bss	_width,1
	.globl	_max_val
	.bss	_max_val,1
	.globl	_buf1
	.bss	_buf1,1200
	.globl	_buf2
	.bss	_buf2,1200
	.globl	_maxit
	.bss	_maxit,1
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,2
	.sect	".cinit"
	.word	2,CONST
	.word 	_buf1            ;0
	.word 	_buf2            ;1
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_GIEOn
	.globl	DIV_U
	.end
