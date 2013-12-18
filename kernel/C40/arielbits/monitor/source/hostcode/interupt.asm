FP	.set	AR3
	.globl	_SetIntVect

_SetIntVect:
	ldep	ivtp,ar0

	ldi	sp,ar1
	ldi	*-ar1(1),ir0	;Get interrupt to set

	ldi	*-ar1(2),r0	;Get pointer to interrupt handler routine
	sti	r0,*+ar0(ir0)

	rets



	.globl	_SetIntTable

_SetIntTable:
	ldi	sp,ar1

	ldi	*-ar1(1),r0	;Get table address
	ldpe	r0,ivtp

	rets




	.globl	_SetTrapTable

_SetTrapTable:
	ldi	sp,ar1

	ldi	*-ar1(1),r0	;Get table address
	ldpe	r0,tvtp

	rets




	.globl	_SetTrapVect

_SetTrapVect:
	ldep	tvtp,ar0

	ldi	sp,ar1
	ldi	*-ar1(1),ir0	;Get interrupt to set

	ldi	*-ar1(2),r0	;Get pointer to interrupt handler routine
	sti	r0,*+ar0(ir0)

	rets



	.globl	_EnableInt

_EnableInt:
		ldi	sp,ar1

		ldi	*-ar1(1),r0	; Get offset
	
		cmpi	02h,r0		; Check for TINT0
		beq	ETINT0

		cmpi	02bh,r0	; Check for TINT1
		beq	ETINT1

		cmpi	06h,r0		; Check for 3 <= Offset  <= 6
		bgt	EGREAT6
		cmpi	03h,r0
		blt	ERESERVED
		b	EIIOF

EGREAT6	cmpi	024h,r0	; Check for 0xd <= Offset <= 0x24
		bgt	EGREAT24
		cmpi	0dh,r0
		blt	ERESERVED
		b	ECOMM
		
EGREAT24	cmpi	02ah,r0	; Check for 0x25 <= Offset <= 0x2a
		bgt	ERESERVED
					; DMA interrupt enable
		ldi	1,r1		; Get 0x02000000 into r1
		lsh	25,r1		; Put the 1 into the 25th bit position
		subi	025h,r0	; Convert Offset into shift count
		lsh	r0,r1
		or	r1,iie
		b	ERETURN

ERESERVED	ldi	0,r0		; Here are the wo ways out: Successfull, Unsuccessfull.
		rets

ERETURN	ldi	1,r0
		rets

ETINT0		or	1,iie		; Timers' interrupt enable
		b	ERETURN

ETINT1		ldi	1,r1
		lsh	31,r1	
		or	r1,iie
		b	ERETURN

					; IIOF interrupt enable
EIIOF		subi	3,r0		; Convert the offset into a shift count
		lsh	2,r0		; r0 * 4
					; To see why these two instructions are needed, see 
					; Figure 3-6, page 3-12
		ldi	9,r1
		lsh	r0,r1
		or	r1,iif
		b	ERETURN

					; Comm port interrupt enable
ECOMM	subi	0dh,r0		; Convert the offset into a shift count
		addi	1,r0
		ldi	1,r1
		lsh	r0,r1
		or	r1,iie
		b	ERETURN






	.globl	_DisableInt

_DisableInt:
		ldi	sp,ar1

		ldi	*-ar1(1),r0	; Get offset
	
		cmpi	02h,r0		; Check for TINT0
		beq	DTINT0

		cmpi	02bh,r0	; Check for TINT1
		beq	DTINT1

		cmpi	06h,r0		; Check for 3 <= Offset  <= 6
		bgt	DGREAT6
		cmpi	03h,r0
		blt	DRESERVED
		b	DIIOF

DGREAT6	cmpi	024h,r0	; Check for 0xd <= Offset <= 0x24
		bgt	DGREAT24
		cmpi	0dh,r0
		blt	DRESERVED
		b	DCOMM
		
DGREAT24	cmpi	02ah,r0	; Check for 0x25 <= Offset <= 0x2a
		bgt	DRESERVED
					; DMA interrupt disable
		ldi	1,r1		; Get 0x02000000 into r1
		lsh	24,r1
		subi	025h,r0	; Convert Offset into shift count
		lsh	r0,r1
		not	r1
		and	r1,iie
		b	DRETURN

DRESERVED	ldi	0,r0		; Here are the wo ways out: Successfull, Unsuccessfull.
		rets

DRETURN	ldi	1,r0
		rets

DTINT0	ldi	1,r1
		not	r1
		and	r1,iie		; Timers' interrupt disable
		b	DRETURN

DTINT1	ldi	1,r1
		lsh	31,r1
		not	r1	
		and	r1,iie
		b	DRETURN

					; IIOF interrupt disable
DIIOF		subi	3,r0		; Convert the offset into a shift count
		lsh	2,r0		; r0 * 4
					; To see why these two instructions are needed, see 
					; Figure 3-6, page 3-12
		ldi	9,r1
		lsh	r0,r1
		not	r1
		and	r1,iif
		b	DRETURN

					; Comm port interrupt disable
DCOMM	subi	0dh,r0		; Convert the offset into a shift count
		addi	1,r0
		ldi	1,r1
		lsh	r0,r1
		not	r1
		and	r1,iie
		b	DRETURN



	.globl	_GIEOn

_GIEOn:
	or	02000h,st

	rets



	.globl	_GIEOff

_GIEOff:
	ldi	02000h,r0
	not	r0

	and	r0,st

	rets



	.globl	_ClearIIOF

_ClearIIOF:
	ldi	0,r0
	or	0BBBBh,r0

	and	r0,iif

	rets


	.globl	_HostInt
_HostInt:
	trap	7h

	rets

	.end
