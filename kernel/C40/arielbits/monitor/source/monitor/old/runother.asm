		.text

	.globl	_RunOthers

_RunOthers:
		ldi	sp,ar0
		ldi	*-ar0(1),r0

		callu	r0

		rets

	.end
