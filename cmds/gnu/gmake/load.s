	align
	module	-1
.ModStart:
	word	#60f160f1
	word	.ModEnd-.ModStart
	blkb	31,"load.c" byte 0
	word	modnum
	word	1
	word	.MaxData
	init
..0: -- 1 refs
	byte	"$Header: /usr/perihelion/Helios/cmds/gnu/gmake/make-3.57/RCS/load.c,v 1.1 90/08/28 14:38:09 james Exp $"
	byte	0
	align
..2: -- 1 refs
	word #60f360f3,.wait_to_start_job byte "wait_to_start_job",0 align
.wait_to_start_job:
	ldl	1
	ldnl	1
	ldlp	-64
	gt
	cj	..3
	ldc	..2-2
	ldpi
	ldl	1
	call	._stack_error
..3: -- 1 refs
-- Line 267 (../load.c)
-- Line 268 (../load.c)
	ret
-- Stubs
	align
._stack_error:
	ldl	1
	ldnl	0
	ldnl	@__stack_error
	ldnl	__stack_error
	gcall
-- Data Initialization
	data	..dataseg 1
	global	_wait_to_start_job
	data	_wait_to_start_job 1
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
	cj	..5
	j	..6
..5: -- 1 refs
	ldc	.wait_to_start_job-2
	ldpi
	ldl	0
	stnl	1
	ldc	..0-2
	ldpi
	ldl	0
	stnl	0
..6: -- 1 refs
	ajw	2
	ret
	data	.MaxData 0
	align
.ModEnd:
