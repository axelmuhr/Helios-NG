        align
        module  4
.ModStart:
        word    0x60f160f1
		word modsize
	blkb    31, "Util"
	byte 0
	word	modnum
	word	1000
		word	datasymb(.MaxData)
        init
	word	codesymb(.MaxCodeP)
Util.library:
		global	Util.library
		align
		init
				ajw -1
				ldl 2 ldnl modnum stl 0
				ldl 3
				eqc 0
				cj ..INIT.0
				codetable _NewProcess
				global _NewProcess
					ldc .NewProcess-2 ldpi ldl 0 stnl _NewProcess
				codetable _RunProcess
				global _RunProcess
					ldc .RunProcess-2 ldpi ldl 0 stnl _RunProcess
				codetable _ZapProcess
				global _ZapProcess
					ldc .ZapProcess-2 ldpi ldl 0 stnl _ZapProcess
				codetable _setjmp
				global _setjmp
					ldc .setjmp-2 ldpi ldl 0 stnl _setjmp
				codetable _longjmp
				global _longjmp
					ldc .longjmp-2 ldpi ldl 0 stnl _longjmp
				codetable _IOdebug
				global _IOdebug
					ldc .IOdebug-2 ldpi ldl 0 stnl _IOdebug
				codetable _IOputc
				global _IOputc
					ldc .IOputc-2 ldpi ldl 0 stnl _IOputc
				codetable _Fork
				global _Fork
					ldc .Fork-2 ldpi ldl 0 stnl _Fork
				codetable _strlen
				global _strlen
					ldc .strlen-2 ldpi ldl 0 stnl _strlen
				codetable _strcpy
				global _strcpy
					ldc .strcpy-2 ldpi ldl 0 stnl _strcpy
				codetable _strncpy
				global _strncpy
					ldc .strncpy-2 ldpi ldl 0 stnl _strncpy
				codetable _strcat
				global _strcat
					ldc .strcat-2 ldpi ldl 0 stnl _strcat
				codetable _strncat
				global _strncat
					ldc .strncat-2 ldpi ldl 0 stnl _strncat
				codetable _strcmp
				global _strcmp
					ldc .strcmp-2 ldpi ldl 0 stnl _strcmp
				codetable _strncmp
				global _strncmp
					ldc .strncmp-2 ldpi ldl 0 stnl _strncmp
				codetable _memset
				global _memset
					ldc .memset-2 ldpi ldl 0 stnl _memset
				codetable _memcpy
				global _memcpy
					ldc .memcpy-2 ldpi ldl 0 stnl _memcpy
				codetable _IOputs
				global _IOputs
					ldc .IOputs-2 ldpi ldl 0 stnl _IOputs
				codetable _ExecProcess
				global _ExecProcess
					ldc .ExecProcess-2 ldpi ldl 0 stnl _ExecProcess
				codetable _procname
				global _procname
					ldc .procname-2 ldpi ldl 0 stnl _procname
			..INIT.0:
				ajw 1
				ret
		ref	Kernel.library
		ref	SysLib.library
					.InitMCB:
							ExternBigBranch InitMCB
					.FreeStop:
							ExternBigBranch FreeStop
					.Free:
							ExternBigBranch Free
					.Malloc:
							ExternBigBranch Malloc
					.LogToPhysPri:
							ExternBigBranch LogToPhysPri
					.GetPriority:
							ExternBigBranch GetPriority
					.GetRootBase:
							ExternBigBranch GetRootBase
					._Halt:
							ExternBigBranch _Halt
					.Signal:
							ExternBigBranch Signal
					.Wait:
							ExternBigBranch Wait
					.StopProcess:
							ExternBigBranch StopProcess
					.StartProcess:
							ExternBigBranch StartProcess
					.InitProcess:
							ExternBigBranch InitProcess
					._Trace:
							ExternBigBranch _Trace
					.PutMsg:
							ExternBigBranch PutMsg
					.Delay:
							ExternBigBranch Delay
