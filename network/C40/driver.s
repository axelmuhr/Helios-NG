		module -1
		.ModStart:
		word	0x60f860f8
		word	modsize
		blkb	31,"Network.Device" byte 0
		word	0
		word	1000
		word	labelref(.DevOpen)
		ref	Kernel.library
		ref	SysLib.library
		ref	Util.library
		ref	Posix.library
		ref	RmLib.library
