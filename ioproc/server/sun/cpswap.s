

	.text
	.proc

	.globl	_cpswap
_cpswap:

	link	a6,#0
	addl	#-12,sp
	moveml	#0x3080,sp@

	movl	a6@(0x8),a5
	movl	a6@(0xc),a4
	movl	a6@(0x10),d7

cpsw1:
	movl	a4@+,d0
	rolw	#8,d0
	swap	d0
	rolw	#8,d0
	movl	d0,a5@+

	subql	#4,d7
	bgt	cpsw1
	
	moveml	a6@(-0xc),#0x3080
	unlk	a6

	rts

