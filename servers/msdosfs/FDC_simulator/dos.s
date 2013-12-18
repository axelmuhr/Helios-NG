		module -1
	.ModStart:
		word	#60f860f8
		word	.ModEnd-.ModStart
		blkb	31,"Disc.Device" byte 0
		word	0
		word	1000
		word	.DevOpen
	byte "%W%	%G% Copyright (C) 1987, Perihelion Software Ltd.",0
		ref	Kernel.library
		ref	SysLib.library
		ref	Util.library
		ref	Posix.library
					.close:
						ldl 1
						ldnl 0
						ldnl @_close
						ldnl _close
						gcall
					.open:
						ldl 1
						ldnl 0
						ldnl @_open
						ldnl _open
						gcall
					.fdstream:
						ldl 1
						ldnl 0
						ldnl @_fdstream
						ldnl _fdstream
						gcall
					.Read:
						ldl 1
						ldnl 0
						ldnl @_Read
						ldnl _Read
						gcall
					.Seek:
						ldl 1
						ldnl 0
						ldnl @_Seek
						ldnl _Seek
						gcall
					.Write:
						ldl 1
						ldnl 0
						ldnl @_Write
						ldnl _Write
						gcall
					._cputime:
						ldl 1
						ldnl 0
						ldnl @__cputime
						ldnl __cputime
						gcall
					.FreeLink:
						ldl 1
						ldnl 0
						ldnl @_FreeLink
						ldnl _FreeLink
						gcall
					.AllocLink:
						ldl 1
						ldnl 0
						ldnl @_AllocLink
						ldnl _AllocLink
						gcall
					.Configure:
						ldl 1
						ldnl 0
						ldnl @_Configure
						ldnl _Configure
						gcall
					.LinkData:
						ldl 1
						ldnl 0
						ldnl @_LinkData
						ldnl _LinkData
						gcall
					.LinkOut:
						ldl 1
						ldnl 0
						ldnl @_LinkOut
						ldnl _LinkOut
						gcall
					.LinkIn:
						ldl 1
						ldnl 0
						ldnl @_LinkIn
						ldnl _LinkIn
						gcall
					.Signal:
						ldl 1
						ldnl 0
						ldnl @_Signal
						ldnl _Signal
						gcall
					.InitSemaphore:
						ldl 1
						ldnl 0
						ldnl @_InitSemaphore
						ldnl _InitSemaphore
						gcall
					.Wait:
						ldl 1
						ldnl 0
						ldnl @_Wait
						ldnl _Wait
						gcall
					.IOdebug:
						ldl 1
						ldnl 0
						ldnl @_IOdebug
						ldnl _IOdebug
						gcall
					.Free:
						ldl 1
						ldnl 0
						ldnl @_Free
						ldnl _Free
						gcall
					.Malloc:
						ldl 1
						ldnl 0
						ldnl @_Malloc
						ldnl _Malloc
						gcall
					.Delay:
						ldl 1
						ldnl 0
						ldnl @_Delay
						ldnl _Delay
						gcall
