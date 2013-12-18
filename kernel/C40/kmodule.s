        align
        module  1
.ModStart:
        word    0x60f160f1
		word modsize
	blkb    31, "Kernel"
	byte 0
	word	modnum
	word	2000
		word	datasymb(.MaxData)
        init
	word	codesymb(.MaxCodeP)
Kernel.library:
		global	Kernel.library
					patchinstr (PATCHC40MASK24ADD,
						   shift(-2, labelref(.Kstart)),
						   br	0)
			byte "The Helios Parallel Operating System "
			byte "Copyright (C) 1987-1993, Perihelion Software Ltd."
			align
		align
		init
				CMPI	2, R0
				Beq	_CodeTableInit
				patchinstr(PATCHC40MASK8ADD,
					shift(1, modnum),
					ldi	*+AR4(0), AR0)
	lsh	-2, AR0
	addi	IR0, AR0
				LDI	R11,	R7
				codetable _InitList
				global _InitList
				codetable _PreInsert
				global _PreInsert
				codetable _PostInsert
				global _PostInsert
				codetable _Remove
				global _Remove
				codetable _AddHead
				global _AddHead
				codetable _AddTail
				global _AddTail
				codetable _RemHead
				global _RemHead
				codetable _RemTail
				global _RemTail
					codetable _NewPort
					global _NewPort
					codetable _FreePort
					global _FreePort
					codetable _PutMsg
					global _PutMsg
					codetable _GetMsg
					global _GetMsg
					codetable _PutReady
					global _PutReady
					codetable _GetReady
					global _GetReady
					codetable _AbortPort
					global _AbortPort
					codetable _MultiWait
					global _MultiWait
					codetable _SendException
					global _SendException
					codetable _InitSemaphore
					global _InitSemaphore
					codetable _Wait
					global _Wait
					codetable _Signal
					global _Signal
					codetable _TestSemaphore
					global _TestSemaphore
					codetable _InitPool
					global _InitPool
					codetable _AllocMem
					global _AllocMem
					codetable _FreeMem
					global _FreeMem
					codetable _FreePool
					global _FreePool
					codetable _AllocFast
					global _AllocFast
					codetable _TaskInit
					global _TaskInit
					codetable _KillTask
					global _KillTask
					codetable _CallException
					global _CallException
					codetable _EnableLink
					global _EnableLink
					codetable _AllocLink
					global _AllocLink
					codetable _FreeLink
					global _FreeLink
					codetable _Reconfigure
					global _Reconfigure
					codetable _Terminate
					global _Terminate
					codetable _LinkData
					global _LinkData
					codetable _Delay
					global _Delay
				codetable __Mark
				global __Mark
				codetable __Trace
				global __Trace
					codetable __Halt
					global __Halt
				codetable _WalkList
				global _WalkList
				codetable _SearchList
				global _SearchList
					codetable _InPool
					global _InPool
				codetable _MachineType
				global _MachineType
					codetable _LinkIn
					global _LinkIn
					codetable _LinkOut
					global _LinkOut
					codetable _SetEvent
					global _SetEvent
					codetable _RemEvent
					global _RemEvent
			data __Task_, 4
			global __Task_ 
					codetable _InitProcess
					global _InitProcess
					codetable _StartProcess
					global _StartProcess
					codetable _StopProcess
					global _StopProcess
					codetable _GetPortInfo
					global _GetPortInfo
					codetable _FreeMemStop
					global _FreeMemStop
					codetable _SignalStop
					global _SignalStop
					codetable _Configure
					global _Configure
					codetable _TestWait
					global _TestWait
				codetable _LogToPhysPri
				global _LogToPhysPri
				codetable _PhysToLogPri
				global _PhysToLogPri
				codetable _GetPhysPriRange
				global _GetPhysPriRange
				codetable _GetPriority
				global _GetPriority
					codetable _SetPriority
					global _SetPriority
				codetable _GetNucleusBase
				global _GetNucleusBase
				codetable _GetRootBase
				global _GetRootBase
				codetable _CallWithModTab
				global _CallWithModTab
				codetable __GetModTab
				global __GetModTab
					codetable _Timer
					global _Timer
					codetable _XchMsg
					global _XchMsg
				codetable _SaveCPUState
				global _SaveCPUState
				codetable _RestoreCPUState
				global _RestoreCPUState
					codetable _AvoidEvents
					global _AvoidEvents
					codetable _HardenedWait
					global _HardenedWait
					codetable _HardenedSignal
					global _HardenedSignal
					codetable _System
					global _System
					codetable __cputime
					global __cputime
					codetable __ldtimer
					global __ldtimer
					codetable _StatMem
					global _StatMem
					codetable _LowAllocMem
					global _LowAllocMem
					codetable _TimedWait
					global _TimedWait
					codetable _SliceState
					global _SliceState
					codetable _SliceQuantum
					global _SliceQuantum
				codetable __linkreg
				global __linkreg
				codetable __fpreg
				global __fpreg
				codetable __spreg
				global __spreg
				codetable ___divtest
				global ___divtest
				codetable ___divide
				global ___divide
				codetable ___udivide
				global ___udivide
				codetable ___remainder
				global ___remainder
				codetable ___uremainder
				global ___uremainder
				codetable ___stack_overflow
				global ___stack_overflow
				codetable _GetExecRoot
				global _GetExecRoot
				codetable _AllocSpecial
				global _AllocSpecial
				codetable _memcpy
				global _memcpy
				codetable _memset
				global _memset
				codetable _C40CAddress
				global _C40CAddress
				codetable _C40WordAddress
				global _C40WordAddress
				codetable _JTAGHalt
				global _JTAGHalt
				codetable __udiv10
				global __udiv10
				codetable __sdiv10
				global __sdiv10
				codetable __backtrace
				global __backtrace
				codetable __DataToFuncConvert
				global __DataToFuncConvert
				codetable __FuncToDataConvert
				global __FuncToDataConvert
				codetable _GetIDROM
				global _GetIDROM
				codetable __stack_size
				global __stack_size
				codetable _Accelerate
				global _Accelerate
				codetable _MP_GetWord
				global _MP_GetWord
				codetable _MP_PutWord
				global _MP_PutWord
				codetable _MP_GetData
				global _MP_GetData
				codetable _MP_PutData
				global _MP_PutData
				codetable _MP_ReadLock
				global _MP_ReadLock
				codetable _MP_ReadFPLock
				global _MP_ReadFPLock
				codetable _MP_WriteUnlock
				global _MP_WriteUnlock
				codetable _MP_WriteFPUnlock
				global _MP_WriteFPUnlock
				codetable _MP_Signal
				global _MP_Signal
				codetable _MP_BusyWait
				global _MP_BusyWait
				codetable _MP_LinkIn
				global _MP_LinkIn
				codetable _MP_LinkOut
				global _MP_LinkOut
				codetable _GetHWConfig
				global _GetHWConfig
				codetable _ReleaseStack
				global _ReleaseStack
				b	R7		
				_CodeTableInit:
				patchinstr(PATCHC40MASK8ADD,
					shift(1, modnum),
					ldi	*+AR4(1), AR0)
	ldi	R11, AR5			
	laj	4
		nop				
		patchinstr(PATCHC40MASK16ADD,
			shift(-2, labelref(_FuncTableEnd)),
			addi	-2, R11)	
		ldi	R11, AR1
	ldi	AR5, R11			
				B	_Loop1Start		
				_Loop1:				
				ADDI	AR1, RS		
				STI	RS,	*AR0++(1)	
				_Loop1Start:			
				LDI *--AR1, RS	
				Bne	_Loop1	    		
				B	R11			
				_FuncTable:			
					int 0			
						int shift(-2, labelref(.ReleaseStack))
						int shift(-2, labelref(.GetHWConfig))
						int shift(-2, labelref(.MP_LinkOut))
						int shift(-2, labelref(.MP_LinkIn))
						int shift(-2, labelref(.MP_BusyWait))
						int shift(-2, labelref(.MP_Signal))
						int shift(-2, labelref(.MP_WriteFPUnlock))
						int shift(-2, labelref(.MP_WriteUnlock))
						int shift(-2, labelref(.MP_ReadFPLock))
						int shift(-2, labelref(.MP_ReadLock))
						int shift(-2, labelref(.MP_PutData))
						int shift(-2, labelref(.MP_GetData))
						int shift(-2, labelref(.MP_PutWord))
						int shift(-2, labelref(.MP_GetWord))
						int shift(-2, labelref(.Accelerate))
						int shift(-2, labelref(._stack_size))
						int shift(-2, labelref(.GetIDROM))
						int shift(-2, labelref(._FuncToDataConvert))
						int shift(-2, labelref(._DataToFuncConvert))
						int shift(-2, labelref(._backtrace))
						int shift(-2, labelref(._sdiv10))
						int shift(-2, labelref(._udiv10))
						int shift(-2, labelref(.JTAGHalt))
						int shift(-2, labelref(.C40WordAddress))
						int shift(-2, labelref(.C40CAddress))
						int shift(-2, labelref(.memset))
						int shift(-2, labelref(.memcpy))
						int shift(-2, labelref(.AllocSpecial))
						int shift(-2, labelref(.GetExecRoot))
						int shift(-2, labelref(.__stack_overflow))
						int shift(-2, labelref(.__uremainder))
						int shift(-2, labelref(.__remainder))
						int shift(-2, labelref(.__udivide))
						int shift(-2, labelref(.__divide))
						int shift(-2, labelref(.__divtest))
						int shift(-2, labelref(._spreg))
						int shift(-2, labelref(._fpreg))
						int shift(-2, labelref(._linkreg))
						int shift(-2, labelref(.SliceQuantum))
						int shift(-2, labelref(.SliceState))
						int shift(-2, labelref(.TimedWait))
						int shift(-2, labelref(.LowAllocMem))
						int shift(-2, labelref(.StatMem))
						int shift(-2, labelref(._ldtimer))
						int shift(-2, labelref(._cputime))
						int shift(-2, labelref(.System))
						int shift(-2, labelref(.HardenedSignal))
						int shift(-2, labelref(.HardenedWait))
						int shift(-2, labelref(.AvoidEvents))
						int shift(-2, labelref(.RestoreCPUState))
						int shift(-2, labelref(.SaveCPUState))
						int shift(-2, labelref(.XchMsg))
						int shift(-2, labelref(.Timer))
						int shift(-2, labelref(._GetModTab))
						int shift(-2, labelref(.CallWithModTab))
						int shift(-2, labelref(.GetRootBase))
						int shift(-2, labelref(.GetNucleusBase))
						int shift(-2, labelref(.SetPriority))
						int shift(-2, labelref(.GetPriority))
						int shift(-2, labelref(.GetPhysPriRange))
						int shift(-2, labelref(.PhysToLogPri))
						int shift(-2, labelref(.LogToPhysPri))
						int shift(-2, labelref(.TestWait))
						int shift(-2, labelref(.Configure))
						int shift(-2, labelref(.SignalStop))
						int shift(-2, labelref(.FreeMemStop))
						int shift(-2, labelref(.GetPortInfo))
						int shift(-2, labelref(.StopProcess))
						int shift(-2, labelref(.StartProcess))
						int shift(-2, labelref(.InitProcess))
						int shift(-2, labelref(.RemEvent))
						int shift(-2, labelref(.SetEvent))
						int shift(-2, labelref(.LinkOut))
						int shift(-2, labelref(.LinkIn))
						int shift(-2, labelref(.MachineType))
						int shift(-2, labelref(.InPool))
						int shift(-2, labelref(.SearchList))
						int shift(-2, labelref(.WalkList))
						int shift(-2, labelref(._Halt))
						int shift(-2, labelref(._Trace))
						int shift(-2, labelref(._Mark))
						int shift(-2, labelref(.Delay))
						int shift(-2, labelref(.LinkData))
						int shift(-2, labelref(.Terminate))
						int shift(-2, labelref(.Reconfigure))
						int shift(-2, labelref(.FreeLink))
						int shift(-2, labelref(.AllocLink))
						int shift(-2, labelref(.EnableLink))
						int shift(-2, labelref(.CallException))
						int shift(-2, labelref(.KillTask))
						int shift(-2, labelref(.TaskInit))
						int shift(-2, labelref(.AllocFast))
						int shift(-2, labelref(.FreePool))
						int shift(-2, labelref(.FreeMem))
						int shift(-2, labelref(.AllocMem))
						int shift(-2, labelref(.InitPool))
						int shift(-2, labelref(.TestSemaphore))
						int shift(-2, labelref(.Signal))
						int shift(-2, labelref(.Wait))
						int shift(-2, labelref(.InitSemaphore))
						int shift(-2, labelref(.SendException))
						int shift(-2, labelref(.MultiWait))
						int shift(-2, labelref(.AbortPort))
						int shift(-2, labelref(.GetReady))
						int shift(-2, labelref(.PutReady))
						int shift(-2, labelref(.GetMsg))
						int shift(-2, labelref(.PutMsg))
						int shift(-2, labelref(.FreePort))
						int shift(-2, labelref(.NewPort))
						int shift(-2, labelref(.RemTail))
						int shift(-2, labelref(.RemHead))
						int shift(-2, labelref(.AddTail))
						int shift(-2, labelref(.AddHead))
						int shift(-2, labelref(.Remove))
						int shift(-2, labelref(.PostInsert))
						int shift(-2, labelref(.PreInsert))
						int shift(-2, labelref(.InitList))
				_FuncTableEnd:			
