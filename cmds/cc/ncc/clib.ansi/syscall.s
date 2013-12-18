	.globl	__syscall0
	.globl	__syscall1
	.globl	__syscall2
	.globl	__syscall3
	.globl	___ctype
	.globl	__sbrk
	.globl	__curbrk
	.globl	_errno
	.globl	__cerror
__syscall1:
	movw	r1, r2
__syscall0:
	movw	r0, r1
	loadq	$0, r0
	calls	0
	cmpq	$0, r2
	bceq	__cerror
	ret    	r15
__syscall2:
__syscall3:
	pushw   r6, r15
	pushw   r7, r15
	movw	r1, r2
	movw	r0, r1
	loadw   0xc(r15), r3
	loadw   0x10(r15), r4
	loadw   0x14(r15), r5
	loadw   0x18(r15), r6
	loadw   0x1c(r15), r7
	loadq   $0, r0
	calls   0
	popw	r15, r7
	popw	r15, r6
	cmpq	$0, r2
	bceq	__cerror
	ret    	r15
__cerror:
	storw   r1, _errno
	notq    $0, r0
	ret     r15


	.data
	.long	0
___ctype:
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
