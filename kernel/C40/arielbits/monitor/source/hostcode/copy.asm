	.globl	_CopyForHost

	.text
_CopyForHost:
		ldi	sp,ar0
		ldi	*-ar0(1),ar0		; Get pointer to parameters array
		ldi	*+ar0(0),ar1		; Get source address
		ldi	*+ar0(1),ar2		; Get destination address
		subi	1,*+ar0(2),rc		; Get length.  RPTS does instruction rc+1 times

;		ldi	*ar1++,r0		; Copy loop
		rptb	copy
		ldi	*ar1++,r0
copy		sti	r0,*ar2++
;		sti	r0,*ar2++

		rets

		.end
