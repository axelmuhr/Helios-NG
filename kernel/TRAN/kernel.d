-- kmodule.p
-- kmodule.p
	global Kernel.library
	data _InitList 1
	global _InitList
	data _PreInsert 1
	global _PreInsert
	data _PostInsert 1
	global _PostInsert
	data _Remove 1
	global _Remove
	data _AddHead 1
	global _AddHead
	data _AddTail 1
	global _AddTail
	data _RemHead 1
	global _RemHead
	data _RemTail 1
	global _RemTail
	data _NewPort 1
	global _NewPort
	data _FreePort 1
	global _FreePort
	data _PutMsg 1
	global _PutMsg
	data _GetMsg 1
	global _GetMsg
	data _PutReady 1
	global _PutReady
	data _GetReady 1
	global _GetReady
	data _AbortPort 1
	global _AbortPort
	data _MultiWait 1
	global _MultiWait
	data _SendException 1
	global _SendException
	data _InitSemaphore 1
	global _InitSemaphore
	data _Wait 1
	global _Wait
	data _Signal 1
	global _Signal
	data _TestSemaphore 1
	global _TestSemaphore
	data _InitPool 1
	global _InitPool
	data _AllocMem 1
	global _AllocMem
	data _FreeMem 1
	global _FreeMem
	data _FreePool 1
	global _FreePool
	data _AllocFast 1
	global _AllocFast
	data _TaskInit 1
	global _TaskInit
	data _KillTask 1
	global _KillTask
	data _CallException 1
	global _CallException
	data __BootLink 1
	global __BootLink
	data _EnableLink 1
	global _EnableLink
	data _AllocLink 1
	global _AllocLink
	data _FreeLink 1
	global _FreeLink
	data _Reconfigure 1
	global _Reconfigure
	data _Terminate 1
	global _Terminate
	data _LinkData 1
	global _LinkData
	data _Delay 1
	global _Delay
	data __Mark 1
	global __Mark
	data __Trace 1
	global __Trace
	data __Halt 1
	global __Halt
	data _WalkList 1
	global _WalkList
	data _SearchList 1
	global _SearchList
	data _InPool 1
	global _InPool
	data _MachineType 1
	global _MachineType
	data _LinkIn 1
	global _LinkIn
	data _LinkOut 1
	global _LinkOut
	data _SetEvent 1
	global _SetEvent
	data _RemEvent 1
	global _RemEvent
	data __Task_ 1
	global __Task_
	data _InitProcess 1
	global _InitProcess
	data _StartProcess 1
	global _StartProcess
	data _StopProcess 1
	global _StopProcess
	data _GetPortInfo 1
	global _GetPortInfo
	data _FreeMemStop 1
	global _FreeMemStop
	data _SignalStop 1
	global _SignalStop
	data _Configure 1
	global _Configure
	data _SoftReset 1
	global _SoftReset
	data _TestWait 1
	global _TestWait
	data _LogToPhysPri 1
	global _LogToPhysPri
	data _PhysToLogPri 1
	global _PhysToLogPri
	data _GetPhysPriRange 1
	global _GetPhysPriRange
	data _GetPriority 1
	global _GetPriority
	data _SetPriority 1
	global _SetPriority
	data _GetROMConfig 1
	global _GetROMConfig
	data _GetROMItem 1
	global _GetROMItem
	data _GetNucleusBase 1
	global _GetNucleusBase
	data _GetRootBase 1
	global _GetRootBase
	data _CallWithModTab 1
	global _CallWithModTab
	data __GetModTab 1
	global __GetModTab
	data _Timer 1
	global _Timer
	data _XchMsg 1
	global _XchMsg
-- kstart.p
-- queue1.p
-- sem1.p
-- port1.p
-- putmsg1.p
-- getmsg1.p
-- kill1.p
-- link1.p
-- linkmsg1.p
-- memory1.p
-- task.p
-- event.p
-- romsupp.p
-- tranasm.p
-- kend.p
	data .MaxData 0
