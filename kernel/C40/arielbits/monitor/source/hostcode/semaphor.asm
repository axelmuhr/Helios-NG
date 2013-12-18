	.globl	_wait

_wait:
	ldi	sp,ar0		; Get pointer to semaphore
	ldi	*-ar0(1),ar0

	ldi	0,r0	
wait:	ldii	*ar0,r1		; Read semaphore's current value
	bzd	wait		; If semaphore is 0, go to wait and try again
	ldinz	1,r0		; If semaphore is not 0, decrement it
	subi	r0,r1
	stii	r1,*ar0		; Update semaphore

	rets


	.globl	_signal

_signal:
	ldi	sp,ar0		; Get pointer to semaphore
	ldi	*-ar0(1),ar0

	ldii	*ar0,r1
	addi	1,r0
	stii	r0,*ar0		; semaphore = semaphore + 1

	rets
	.end
