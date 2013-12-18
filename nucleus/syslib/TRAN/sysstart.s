        align
        module  2
.ModStart:
        word    #60f160f1
		word    .ModEnd-.ModStart
	        blkb    31, "SysLib"
		byte 0
	word	modnum
	word	1000
		word .MaxData
        init
SysLib.library:
		global	SysLib.library
		align
		init
				ajw -1
				ldl 2 ldnl modnum stl 0
				ldl 3
				eqc 0
				cj ..INIT.0
			data _MyTask 1
			global _MyTask 
				data _Open 1
				global _Open
					ldc .Open-2 ldpi ldl 0 stnl _Open
				data _Locate 1
				global _Locate
					ldc .Locate-2 ldpi ldl 0 stnl _Locate
				data _Create 1
				global _Create
					ldc .Create-2 ldpi ldl 0 stnl _Create
				data _ObjectInfo 1
				global _ObjectInfo
					ldc .ObjectInfo-2 ldpi ldl 0 stnl _ObjectInfo
				data _ServerInfo 1
				global _ServerInfo
					ldc .ServerInfo-2 ldpi ldl 0 stnl _ServerInfo
				data _Link 1
				global _Link
					ldc .Link-2 ldpi ldl 0 stnl _Link
				data _SetDate 1
				global _SetDate
					ldc .SetDate-2 ldpi ldl 0 stnl _SetDate
				data _Protect 1
				global _Protect
					ldc .Protect-2 ldpi ldl 0 stnl _Protect
				data _Delete 1
				global _Delete
					ldc .Delete-2 ldpi ldl 0 stnl _Delete
				data _Rename 1
				global _Rename
					ldc .Rename-2 ldpi ldl 0 stnl _Rename
				data _Refine 1
				global _Refine
					ldc .Refine-2 ldpi ldl 0 stnl _Refine
				data _CopyObject 1
				global _CopyObject
					ldc .CopyObject-2 ldpi ldl 0 stnl _CopyObject
				data _NewObject 1
				global _NewObject
					ldc .NewObject-2 ldpi ldl 0 stnl _NewObject
				data _NewStream 1
				global _NewStream
					ldc .NewStream-2 ldpi ldl 0 stnl _NewStream
				data _ReOpen 1
				global _ReOpen
					ldc .ReOpen-2 ldpi ldl 0 stnl _ReOpen
				data _Read 1
				global _Read
					ldc .Read-2 ldpi ldl 0 stnl _Read
				data _Write 1
				global _Write
					ldc .Write-2 ldpi ldl 0 stnl _Write
				data _Seek 1
				global _Seek
					ldc .Seek-2 ldpi ldl 0 stnl _Seek
				data _GetFileSize 1
				global _GetFileSize
					ldc .GetFileSize-2 ldpi ldl 0 stnl _GetFileSize
				data _SetFileSize 1
				global _SetFileSize
					ldc .SetFileSize-2 ldpi ldl 0 stnl _SetFileSize
				data _Close 1
				global _Close
					ldc .Close-2 ldpi ldl 0 stnl _Close
				data _Load 1
				global _Load
					ldc .Load-2 ldpi ldl 0 stnl _Load
				data _Execute 1
				global _Execute
					ldc .Execute-2 ldpi ldl 0 stnl _Execute
				data _SendEnv 1
				global _SendEnv
					ldc .SendEnv-2 ldpi ldl 0 stnl _SendEnv
				data _GetEnv 1
				global _GetEnv
					ldc .GetEnv-2 ldpi ldl 0 stnl _GetEnv
				data _Malloc 1
				global _Malloc
					ldc .Malloc-2 ldpi ldl 0 stnl _Malloc
				data _Free 1
				global _Free
					ldc .Free-2 ldpi ldl 0 stnl _Free
				data _Result2 1
				global _Result2
					ldc .Result2-2 ldpi ldl 0 stnl _Result2
				data _GetDate 1
				global _GetDate
					ldc .GetDate-2 ldpi ldl 0 stnl _GetDate
				data _Abort 1
				global _Abort
					ldc .Abort-2 ldpi ldl 0 stnl _Abort
				data _Exit 1
				global _Exit
					ldc .Exit-2 ldpi ldl 0 stnl _Exit
				data _TidyUp 1
				global _TidyUp
					ldc .TidyUp-2 ldpi ldl 0 stnl _TidyUp
				data _SendIOC 1
				global _SendIOC
					ldc .SendIOC-2 ldpi ldl 0 stnl _SendIOC
				data _SendMsg 1
				global _SendMsg
					ldc .SendMsg-2 ldpi ldl 0 stnl _SendMsg
				data _XchMsg1 1
				global _XchMsg1
					ldc .XchMsg1-2 ldpi ldl 0 stnl _XchMsg1
				data __SysNewPort 1
				global __SysNewPort
					ldc ._SysNewPort-2 ldpi ldl 0 stnl __SysNewPort
				data __SysFreePort 1
				global __SysFreePort
					ldc ._SysFreePort-2 ldpi ldl 0 stnl __SysFreePort
				data _InitMCB 1
				global _InitMCB
					ldc .InitMCB-2 ldpi ldl 0 stnl _InitMCB
				data _MarshalString 1
				global _MarshalString
					ldc .MarshalString-2 ldpi ldl 0 stnl _MarshalString
				data _MarshalData 1
				global _MarshalData
					ldc .MarshalData-2 ldpi ldl 0 stnl _MarshalData
				data _MarshalWord 1
				global _MarshalWord
					ldc .MarshalWord-2 ldpi ldl 0 stnl _MarshalWord
				data _MarshalOffset 1
				global _MarshalOffset
					ldc .MarshalOffset-2 ldpi ldl 0 stnl _MarshalOffset
				data _MarshalCap 1
				global _MarshalCap
					ldc .MarshalCap-2 ldpi ldl 0 stnl _MarshalCap
				data _MarshalDate 1
				global _MarshalDate
					ldc .MarshalDate-2 ldpi ldl 0 stnl _MarshalDate
				data _MarshalCommon 1
				global _MarshalCommon
					ldc .MarshalCommon-2 ldpi ldl 0 stnl _MarshalCommon
				data _MarshalObject 1
				global _MarshalObject
					ldc .MarshalObject-2 ldpi ldl 0 stnl _MarshalObject
				data _MarshalStream 1
				global _MarshalStream
					ldc .MarshalStream-2 ldpi ldl 0 stnl _MarshalStream
				data _EncodeMatrix 1
				global _EncodeMatrix
					ldc .EncodeMatrix-2 ldpi ldl 0 stnl _EncodeMatrix
				data _getbitchars 1
				global _getbitchars
					ldc .getbitchars-2 ldpi ldl 0 stnl _getbitchars
				data _DecodeMask 1
				global _DecodeMask
					ldc .DecodeMask-2 ldpi ldl 0 stnl _DecodeMask
				data _DecodeMatrix 1
				global _DecodeMatrix
					ldc .DecodeMatrix-2 ldpi ldl 0 stnl _DecodeMatrix
				data _EncodeCapability 1
				global _EncodeCapability
					ldc .EncodeCapability-2 ldpi ldl 0 stnl _EncodeCapability
				data _DecodeCapability 1
				global _DecodeCapability
					ldc .DecodeCapability-2 ldpi ldl 0 stnl _DecodeCapability
				data _splitname 1
				global _splitname
					ldc .splitname-2 ldpi ldl 0 stnl _splitname
				data _GetInfo 1
				global _GetInfo
					ldc .GetInfo-2 ldpi ldl 0 stnl _GetInfo
				data _SetInfo 1
				global _SetInfo
					ldc .SetInfo-2 ldpi ldl 0 stnl _SetInfo
				data _GetAttributes 1
				global _GetAttributes
					ldc .GetAttributes-2 ldpi ldl 0 stnl _GetAttributes
				data _SetAttributes 1
				global _SetAttributes
					ldc .SetAttributes-2 ldpi ldl 0 stnl _SetAttributes
				data _IsAnAttribute 1
				global _IsAnAttribute
					ldc .IsAnAttribute-2 ldpi ldl 0 stnl _IsAnAttribute
				data _AddAttribute 1
				global _AddAttribute
					ldc .AddAttribute-2 ldpi ldl 0 stnl _AddAttribute
				data _RemoveAttribute 1
				global _RemoveAttribute
					ldc .RemoveAttribute-2 ldpi ldl 0 stnl _RemoveAttribute
				data _GetInputSpeed 1
				global _GetInputSpeed
					ldc .GetInputSpeed-2 ldpi ldl 0 stnl _GetInputSpeed
				data _GetOutputSpeed 1
				global _GetOutputSpeed
					ldc .GetOutputSpeed-2 ldpi ldl 0 stnl _GetOutputSpeed
				data _SetInputSpeed 1
				global _SetInputSpeed
					ldc .SetInputSpeed-2 ldpi ldl 0 stnl _SetInputSpeed
				data _SetOutputSpeed 1
				global _SetOutputSpeed
					ldc .SetOutputSpeed-2 ldpi ldl 0 stnl _SetOutputSpeed
				data _MachineName 1
				global _MachineName
					ldc .MachineName-2 ldpi ldl 0 stnl _MachineName
				data _EnableEvents 1
				global _EnableEvents
					ldc .EnableEvents-2 ldpi ldl 0 stnl _EnableEvents
				data _TaskData 1
				global _TaskData
					ldc .TaskData-2 ldpi ldl 0 stnl _TaskData
				data _SetException 1
				global _SetException
					ldc .SetException-2 ldpi ldl 0 stnl _SetException
				data _SendSignal 1
				global _SendSignal
					ldc .SendSignal-2 ldpi ldl 0 stnl _SendSignal
				data _DefaultException 1
				global _DefaultException
					ldc .DefaultException-2 ldpi ldl 0 stnl _DefaultException
				data _Alarm 1
				global _Alarm
					ldc .Alarm-2 ldpi ldl 0 stnl _Alarm
				data _NegAcknowledge 1
				global _NegAcknowledge
					ldc .NegAcknowledge-2 ldpi ldl 0 stnl _NegAcknowledge
				data _Acknowledge 1
				global _Acknowledge
					ldc .Acknowledge-2 ldpi ldl 0 stnl _Acknowledge
				data _InitProgramInfo 1
				global _InitProgramInfo
					ldc .InitProgramInfo-2 ldpi ldl 0 stnl _InitProgramInfo
				data _GetProgramInfo 1
				global _GetProgramInfo
					ldc .GetProgramInfo-2 ldpi ldl 0 stnl _GetProgramInfo
				data _BootLink 1
				global _BootLink
					ldc .BootLink-2 ldpi ldl 0 stnl _BootLink
				data _PseudoStream 1
				global _PseudoStream
					ldc .PseudoStream-2 ldpi ldl 0 stnl _PseudoStream
				data _FreeStop 1
				global _FreeStop
					ldc .FreeStop-2 ldpi ldl 0 stnl _FreeStop
				data _GrabPipe 1
				global _GrabPipe
					ldc .GrabPipe-2 ldpi ldl 0 stnl _GrabPipe
				data _UnGrabPipe 1
				global _UnGrabPipe
					ldc .UnGrabPipe-2 ldpi ldl 0 stnl _UnGrabPipe
				data _OpenDevice 1
				global _OpenDevice
					ldc .OpenDevice-2 ldpi ldl 0 stnl _OpenDevice
				data _CloseDevice 1
				global _CloseDevice
					ldc .CloseDevice-2 ldpi ldl 0 stnl _CloseDevice
				data _SelectStream 1
				global _SelectStream
					ldc .SelectStream-2 ldpi ldl 0 stnl _SelectStream
				data _Socket 1
				global _Socket
					ldc .Socket-2 ldpi ldl 0 stnl _Socket
				data _Bind 1
				global _Bind
					ldc .Bind-2 ldpi ldl 0 stnl _Bind
				data _Listen 1
				global _Listen
					ldc .Listen-2 ldpi ldl 0 stnl _Listen
				data _Accept 1
				global _Accept
					ldc .Accept-2 ldpi ldl 0 stnl _Accept
				data _Connect 1
				global _Connect
					ldc .Connect-2 ldpi ldl 0 stnl _Connect
				data _SendMessage 1
				global _SendMessage
					ldc .SendMessage-2 ldpi ldl 0 stnl _SendMessage
				data _RecvMessage 1
				global _RecvMessage
					ldc .RecvMessage-2 ldpi ldl 0 stnl _RecvMessage
				data _GetSocketInfo 1
				global _GetSocketInfo
					ldc .GetSocketInfo-2 ldpi ldl 0 stnl _GetSocketInfo
				data _SetSocketInfo 1
				global _SetSocketInfo
					ldc .SetSocketInfo-2 ldpi ldl 0 stnl _SetSocketInfo
				data _SetSignalPort 1
				global _SetSignalPort
					ldc .SetSignalPort-2 ldpi ldl 0 stnl _SetSignalPort
				data _DES_KeySchedule 1
				global _DES_KeySchedule
					ldc .DES_KeySchedule-2 ldpi ldl 0 stnl _DES_KeySchedule
				data _DES_Inner 1
				global _DES_Inner
					ldc .DES_Inner-2 ldpi ldl 0 stnl _DES_Inner
				data _DES_ECB 1
				global _DES_ECB
					ldc .DES_ECB-2 ldpi ldl 0 stnl _DES_ECB
				data _DES_CFB 1
				global _DES_CFB
					ldc .DES_CFB-2 ldpi ldl 0 stnl _DES_CFB
				data _CopyStream 1
				global _CopyStream
					ldc .CopyStream-2 ldpi ldl 0 stnl _CopyStream
				data _Revoke 1
				global _Revoke
					ldc .Revoke-2 ldpi ldl 0 stnl _Revoke
				data _MemSize 1
				global _MemSize
					ldc .MemSize-2 ldpi ldl 0 stnl _MemSize
				data _ReLocate 1
				global _ReLocate
					ldc .ReLocate-2 ldpi ldl 0 stnl _ReLocate
				data _PreallocMsgBufs 1
				global _PreallocMsgBufs
					ldc .PreallocMsgBufs-2 ldpi ldl 0 stnl _PreallocMsgBufs
				data _DefaultCapability 1
				global _DefaultCapability
					ldc .DefaultCapability-2 ldpi ldl 0 stnl _DefaultCapability
					ldl 2			
					call	._SysLib_Init	
			..INIT.0:
				ajw 1
				ret
		ref	Kernel.library
		ref	Util.library
					._ldtimer:
							ldl 1
							ldnl 0
							ldnl @__ldtimer
							ldnl __ldtimer
							gcall
					._cputime:
							ldl 1
							ldnl 0
							ldnl @__cputime
							ldnl __cputime
							gcall
					.ExecProcess:
							ldl 1
							ldnl 0
							ldnl @_ExecProcess
							ldnl _ExecProcess
							gcall
					.ZapProcess:
							ldl 1
							ldnl 0
							ldnl @_ZapProcess
							ldnl _ZapProcess
							gcall
					.NewProcess:
							ldl 1
							ldnl 0
							ldnl @_NewProcess
							ldnl _NewProcess
							gcall
					.Fork:
							ldl 1
							ldnl 0
							ldnl @_Fork
							ldnl _Fork
							gcall
					.IOdebug:
							ldl 1
							ldnl 0
							ldnl @_IOdebug
							ldnl _IOdebug
							gcall
					.memset:
							ldl 1
							ldnl 0
							ldnl @_memset
							ldnl _memset
							gcall
					.memcpy:
							ldl 1
							ldnl 0
							ldnl @_memcpy
							ldnl _memcpy
							gcall
					.strcat:
							ldl 1
							ldnl 0
							ldnl @_strcat
							ldnl _strcat
							gcall
					.strcpy:
							ldl 1
							ldnl 0
							ldnl @_strcpy
							ldnl _strcpy
							gcall
					.strlen:
							ldl 1
							ldnl 0
							ldnl @_strlen
							ldnl _strlen
							gcall
					.GetNucleusBase:
							ldl 1
							ldnl 0
							ldnl @_GetNucleusBase
							ldnl _GetNucleusBase
							gcall
					.GetRootBase:
							ldl 1
							ldnl 0
							ldnl @_GetRootBase
							ldnl _GetRootBase
							gcall
					.MultiWait:
							ldl 1
							ldnl 0
							ldnl @_MultiWait
							ldnl _MultiWait
							gcall
					.GetPortInfo:
							ldl 1
							ldnl 0
							ldnl @_GetPortInfo
							ldnl _GetPortInfo
							gcall
					.StopProcess:
							ldl 1
							ldnl 0
							ldnl @_StopProcess
							ldnl _StopProcess
							gcall
					.SignalStop:
							ldl 1
							ldnl 0
							ldnl @_SignalStop
							ldnl _SignalStop
							gcall
					.FreeMemStop:
							ldl 1
							ldnl 0
							ldnl @_FreeMemStop
							ldnl _FreeMemStop
							gcall
					.SearchList:
							ldl 1
							ldnl 0
							ldnl @_SearchList
							ldnl _SearchList
							gcall
					.WalkList:
							ldl 1
							ldnl 0
							ldnl @_WalkList
							ldnl _WalkList
							gcall
					.PutReady:
							ldl 1
							ldnl 0
							ldnl @_PutReady
							ldnl _PutReady
							gcall
					.GetReady:
							ldl 1
							ldnl 0
							ldnl @_GetReady
							ldnl _GetReady
							gcall
					.InPool:
							ldl 1
							ldnl 0
							ldnl @_InPool
							ldnl _InPool
							gcall
					._Trace:
							ldl 1
							ldnl 0
							ldnl @__Trace
							ldnl __Trace
							gcall
					._Mark:
							ldl 1
							ldnl 0
							ldnl @__Mark
							ldnl __Mark
							gcall
					.Delay:
							ldl 1
							ldnl 0
							ldnl @_Delay
							ldnl _Delay
							gcall
					.SendException:
							ldl 1
							ldnl 0
							ldnl @_SendException
							ldnl _SendException
							gcall
					.NewPort:
							ldl 1
							ldnl 0
							ldnl @_NewPort
							ldnl _NewPort
							gcall
					.FreePort:
							ldl 1
							ldnl 0
							ldnl @_FreePort
							ldnl _FreePort
							gcall
					.AbortPort:
							ldl 1
							ldnl 0
							ldnl @_AbortPort
							ldnl _AbortPort
							gcall
					.FreeMem:
							ldl 1
							ldnl 0
							ldnl @_FreeMem
							ldnl _FreeMem
							gcall
					.AllocMem:
							ldl 1
							ldnl 0
							ldnl @_AllocMem
							ldnl _AllocMem
							gcall
					.Remove:
							ldl 1
							ldnl 0
							ldnl @_Remove
							ldnl _Remove
							gcall
					.AddHead:
							ldl 1
							ldnl 0
							ldnl @_AddHead
							ldnl _AddHead
							gcall
					.AddTail:
							ldl 1
							ldnl 0
							ldnl @_AddTail
							ldnl _AddTail
							gcall
					.InitList:
							ldl 1
							ldnl 0
							ldnl @_InitList
							ldnl _InitList
							gcall
					.TestSemaphore:
							ldl 1
							ldnl 0
							ldnl @_TestSemaphore
							ldnl _TestSemaphore
							gcall
					.InitSemaphore:
							ldl 1
							ldnl 0
							ldnl @_InitSemaphore
							ldnl _InitSemaphore
							gcall
					.Signal:
							ldl 1
							ldnl 0
							ldnl @_Signal
							ldnl _Signal
							gcall
					.Wait:
							ldl 1
							ldnl 0
							ldnl @_Wait
							ldnl _Wait
							gcall
					.XchMsg:
							ldl 1
							ldnl 0
							ldnl @_XchMsg
							ldnl _XchMsg
							gcall
					.PutMsg:
							ldl 1
							ldnl 0
							ldnl @_PutMsg
							ldnl _PutMsg
							gcall
					.GetMsg:
							ldl 1
							ldnl 0
							ldnl @_GetMsg
							ldnl _GetMsg
							gcall
