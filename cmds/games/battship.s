	align
	module	-1
.ModStart:
	word	#60f160f1
	word	.ModEnd-.ModStart
	blkb	31,"battship.c" byte 0
	word	modnum
	word	1
	word	.MaxData
	init
