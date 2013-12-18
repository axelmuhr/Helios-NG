	align
	module	-1
.ModStart:
	word	#60f160f1
	word	.ModEnd-.ModStart
	blkb	31,"test.c" byte 0
	word	modnum
	word	1
	word	.MaxData
	init
	align
	word #60f360f3,.f byte "f",0 align
.f:
-- Line 3 (/users/nickc/tmp/test.c)
	ret
-- Stubs
-- Data Initialization
	data	..dataseg 0
	global	_x
	data	_x 1
	global	_y
	data	_y 1
	global	_f
	data	_f 1
	align
	init
	ajw	-2
	ldl	3
	ldnl	0
	ldnl	modnum
	stl	1
	ldl	1
	ldnlp	..dataseg
	stl	0
	ldl	4
	cj	..2
	ldl	3
	ldnl	0
	ldnl	@_x
	ldnlp	_x
	ldl	0
	stnl	1
	j	..3
..2: -- 1 refs
	ldc	.f-2
	ldpi
	ldl	0
	stnl	2
..3: -- 1 refs
	ajw	2
	ret
	data	.MaxData 0
	align
.ModEnd:
