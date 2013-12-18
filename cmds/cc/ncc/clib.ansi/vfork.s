	.globl	__vfork
	.globl	__wait
__vfork:
	popw    r15, r3
	loadi   $66, r0
	calls   0
	cmpq    $0, r2
	bceq    __cerror
	cmpq    $0, r1
	bceq    (r3)
	loadq   $0, r0
	b       (r3)
	storw   r1, 0
	notq    $0, r0
	b       (r3)

__wait: movw    r0, r5
	loadq	$0, r2
	movw    r0, r1
	loadq	$0, r3
	loadi   $84, r0
	calls   0
	cmpq    $0, r2
	bceq    __cerror
	cmpq    $0, r5
	bceq    L1
	storw   r1, (r5)
L1:	ret     r15
