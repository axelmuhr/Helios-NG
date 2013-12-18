		.text

	.globl	_RunForHost

_RunForHost:
		ldi	sp,ar0
		ldi	*-ar0(1),r0

		or	02000h,st	;Enable interrupts
		callu	r0

		rets

	.end
