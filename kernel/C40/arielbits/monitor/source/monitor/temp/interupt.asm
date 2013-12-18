FP	.set	AR3
	.globl	_SetIntVect

_SetIntVect:
	ldep	ivtp,ar0

	ldi	sp,ar1
	ldi	*-ar1(1),ir0	;Get interrupt to set

	ldi	*-ar1(2),r0	;Get pointer to interrupt handler routine
	sti	r0,*+ar0(ir0)

	or	2000H,st	;Set GIE

	rets



	.globl	_SetIntTable

_SetIntTable:
	ldi	sp,ar1

	ldi	*-ar1(1),r0	;Get table address
	ldpe	r0,ivtp

	rets



	.globl	_OrIntMask

_OrIntMask:
	ldi	sp,ar1

	ldi	*-ar1(1),r0	;Get or mask
	or	r0,iie

	rets



	.globl	_AndIntMask

_AndIntMask:
	ldi	sp,ar1

	ldi	*-ar1(1),r0	;Get and mask
	and	r0,iie

	rets


	.end
