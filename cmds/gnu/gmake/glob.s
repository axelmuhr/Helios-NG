	align
	module	-1
.ModStart:
	word	#60f160f1
	word	.ModEnd-.ModStart
	blkb	31,"glob.c" byte 0
	word	modnum
	word	1
	word	.MaxData
	init
..0: -- 1 refs
	byte	"$Header: /usr/perihelion/Helios/cmds/gnu/gmake/make-3.57/RCS/glob.c,v 1.1 90/08/28 14:35:51 james Exp $"
	byte	0
	align
..18: -- 1 refs
	word #60f360f3,.glob_pattern_p byte "glob_pattern_p",0 align
.glob_pattern_p:
	ldl	1
	ldnl	1
	ldlp	-67
	gt
	cj	..19
	ldc	..18-2
	ldpi
	ldl	1
	call	._stack_error
..19: -- 1 refs
	ajw	-3
-- Line 83 (glob.c)
-- Line 83 (glob.c)
	ldl	5
	stl	1
-- Line 87 (glob.c)
	align
..2: -- 8 refs
	ldl	1
	lb
	ldl	1
	adc	1
	stl	1
	stl	2
	ldlp	2
	lb
	cj	..3
-- Line 88 (glob.c)
	ldlp	2
	lb
	stl	0
	ldl	0
	ldc	91
	gt
	cj	..10
	ldl	0
	adc	-92
	cj	..6
	j	..2
	align
..10: -- 1 refs
	diff
	ldl	0
	adc	-42
	cj	..9
	ldl	0
	adc	-63
	cj	..9
	ldl	0
	adc	-91
	eqc	0
	cj	..2
	align
..9: -- 5 refs
-- Line 93 (glob.c)
	ldc	1
	ajw	3
	ret
	align
..6: -- 1 refs
-- Line 96 (glob.c)
	ldl	1
	lb
	ldl	1
	adc	1
	stl	1
	eqc	0
	cj	..2
-- Line 97 (glob.c)
	ldc	0
	ajw	3
	ret
	align
..3: -- 1 refs
-- Line 100 (glob.c)
	ldc	0
	ajw	3
	ret
