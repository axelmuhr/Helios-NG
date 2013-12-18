	align
	module	-1
.ModStart:
	word	#60f160f1
	word	.ModEnd-.ModStart
	blkb	31,"dbdump.c" byte 0
	word	modnum
	word	1
	word	.MaxData
	init
..0: -- 1 refs
	byte	"$Id: dbdump.c,v 1.2 1994/05/12 13:46:06 nickc Exp $"
	byte	0
