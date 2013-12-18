		module -1
		.ModStart:
		word	0x60f860f8
		word	modsize
		blkb	31,"SeeqEther.device" byte 0
		word	0
		word	1000
		word	labelref(.DevOpen)
		ref	Kernel.library
		ref	SysLib.library
