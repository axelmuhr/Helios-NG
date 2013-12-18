        align
        module  2
.ModStart:
        word    0x60f160f1
		word modsize
	blkb    31, "SysLib"
	byte 0
	word	modnum
	word	1000
		word	datasymb(.MaxData)
        init
	word	codesymb(.MaxCodeP)
SysLib.library:
		global	SysLib.library
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
			data _MyTask, 4
			global _MyTask 
				codetable _Open
				global _Open
				codetable _Locate
				global _Locate
				codetable _Create
				global _Create
				codetable _ObjectInfo
				global _ObjectInfo
				codetable _ServerInfo
				global _ServerInfo
				codetable _Link
				global _Link
				codetable _SetDate
				global _SetDate
				codetable _Protect
				global _Protect
				codetable _Delete
				global _Delete
				codetable _Rename
				global _Rename
				codetable _Refine
				global _Refine
				codetable _CopyObject
				global _CopyObject
				codetable _NewObject
				global _NewObject
				codetable _NewStream
				global _NewStream
				codetable _ReOpen
				global _ReOpen
				codetable _Read
				global _Read
				codetable _Write
				global _Write
				codetable _Seek
				global _Seek
				codetable _GetFileSize
				global _GetFileSize
				codetable _SetFileSize
				global _SetFileSize
				codetable _Close
				global _Close
				codetable _Load
				global _Load
				codetable _Execute
				global _Execute
				codetable _SendEnv
				global _SendEnv
				codetable _GetEnv
				global _GetEnv
				codetable _Malloc
				global _Malloc
				codetable _Free
				global _Free
				codetable _Result2
				global _Result2
				codetable _GetDate
				global _GetDate
				codetable _Abort
				global _Abort
				codetable _Exit
				global _Exit
				codetable _TidyUp
				global _TidyUp
				codetable _SendIOC
				global _SendIOC
				codetable _SendMsg
				global _SendMsg
				codetable _XchMsg1
				global _XchMsg1
				codetable _InitMCB
				global _InitMCB
				codetable _MarshalString
				global _MarshalString
				codetable _MarshalData
				global _MarshalData
				codetable _MarshalWord
				global _MarshalWord
				codetable _MarshalOffset
				global _MarshalOffset
				codetable _MarshalCap
				global _MarshalCap
				codetable _MarshalDate
				global _MarshalDate
				codetable _MarshalCommon
				global _MarshalCommon
				codetable _MarshalObject
				global _MarshalObject
				codetable _MarshalStream
				global _MarshalStream
				codetable _EncodeMatrix
				global _EncodeMatrix
				codetable _getbitchars
				global _getbitchars
				codetable _DecodeMask
				global _DecodeMask
				codetable _DecodeMatrix
				global _DecodeMatrix
				codetable _EncodeCapability
				global _EncodeCapability
				codetable _DecodeCapability
				global _DecodeCapability
				codetable _splitname
				global _splitname
				codetable _GetInfo
				global _GetInfo
				codetable _SetInfo
				global _SetInfo
				codetable _GetAttributes
				global _GetAttributes
				codetable _SetAttributes
				global _SetAttributes
				codetable _IsAnAttribute
				global _IsAnAttribute
				codetable _AddAttribute
				global _AddAttribute
				codetable _RemoveAttribute
				global _RemoveAttribute
				codetable _GetInputSpeed
				global _GetInputSpeed
				codetable _GetOutputSpeed
				global _GetOutputSpeed
				codetable _SetInputSpeed
				global _SetInputSpeed
				codetable _SetOutputSpeed
				global _SetOutputSpeed
				codetable _MachineName
				global _MachineName
				codetable _EnableEvents
				global _EnableEvents
				codetable _TaskData
				global _TaskData
				codetable _SetException
				global _SetException
				codetable _SendSignal
				global _SendSignal
				codetable _DefaultException
				global _DefaultException
				codetable _Alarm
				global _Alarm
				codetable _NegAcknowledge
				global _NegAcknowledge
				codetable _Acknowledge
				global _Acknowledge
				codetable _InitProgramInfo
				global _InitProgramInfo
				codetable _GetProgramInfo
				global _GetProgramInfo
				codetable _PseudoStream
				global _PseudoStream
				codetable _FreeStop
				global _FreeStop
				codetable _OpenDevice
				global _OpenDevice
				codetable _CloseDevice
				global _CloseDevice
				codetable _SelectStream
				global _SelectStream
				codetable _Socket
				global _Socket
				codetable _Bind
				global _Bind
				codetable _Listen
				global _Listen
				codetable _Accept
				global _Accept
				codetable _Connect
				global _Connect
				codetable _SendMessage
				global _SendMessage
				codetable _RecvMessage
				global _RecvMessage
				codetable _GetSocketInfo
				global _GetSocketInfo
				codetable _SetSocketInfo
				global _SetSocketInfo
				codetable _SetSignalPort
				global _SetSignalPort
				codetable _DES_KeySchedule
				global _DES_KeySchedule
				codetable _DES_Inner
				global _DES_Inner
				codetable _DES_ECB
				global _DES_ECB
				codetable _DES_CFB
				global _DES_CFB
				codetable _CopyStream
				global _CopyStream
				codetable _Revoke
				global _Revoke
				codetable _MemSize
				global _MemSize
				codetable _ReLocate
				global _ReLocate
				codetable _PreallocMsgBufs
				global _PreallocMsgBufs
				codetable _DefaultCapability
				global _DefaultCapability
					patchinstr(PATCHC40MASK24ADD,
						shift(-2, labelref(._SysLib_Init)),
						br	0)
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
						int shift(-2, labelref(.DefaultCapability))
						int shift(-2, labelref(.PreallocMsgBufs))
						int shift(-2, labelref(.ReLocate))
						int shift(-2, labelref(.MemSize))
						int shift(-2, labelref(.Revoke))
						int shift(-2, labelref(.CopyStream))
						int shift(-2, labelref(.DES_CFB))
						int shift(-2, labelref(.DES_ECB))
						int shift(-2, labelref(.DES_Inner))
						int shift(-2, labelref(.DES_KeySchedule))
						int shift(-2, labelref(.SetSignalPort))
						int shift(-2, labelref(.SetSocketInfo))
						int shift(-2, labelref(.GetSocketInfo))
						int shift(-2, labelref(.RecvMessage))
						int shift(-2, labelref(.SendMessage))
						int shift(-2, labelref(.Connect))
						int shift(-2, labelref(.Accept))
						int shift(-2, labelref(.Listen))
						int shift(-2, labelref(.Bind))
						int shift(-2, labelref(.Socket))
						int shift(-2, labelref(.SelectStream))
						int shift(-2, labelref(.CloseDevice))
						int shift(-2, labelref(.OpenDevice))
						int shift(-2, labelref(.FreeStop))
						int shift(-2, labelref(.PseudoStream))
						int shift(-2, labelref(.GetProgramInfo))
						int shift(-2, labelref(.InitProgramInfo))
						int shift(-2, labelref(.Acknowledge))
						int shift(-2, labelref(.NegAcknowledge))
						int shift(-2, labelref(.Alarm))
						int shift(-2, labelref(.DefaultException))
						int shift(-2, labelref(.SendSignal))
						int shift(-2, labelref(.SetException))
						int shift(-2, labelref(.TaskData))
						int shift(-2, labelref(.EnableEvents))
						int shift(-2, labelref(.MachineName))
						int shift(-2, labelref(.SetOutputSpeed))
						int shift(-2, labelref(.SetInputSpeed))
						int shift(-2, labelref(.GetOutputSpeed))
						int shift(-2, labelref(.GetInputSpeed))
						int shift(-2, labelref(.RemoveAttribute))
						int shift(-2, labelref(.AddAttribute))
						int shift(-2, labelref(.IsAnAttribute))
						int shift(-2, labelref(.SetAttributes))
						int shift(-2, labelref(.GetAttributes))
						int shift(-2, labelref(.SetInfo))
						int shift(-2, labelref(.GetInfo))
						int shift(-2, labelref(.splitname))
						int shift(-2, labelref(.DecodeCapability))
						int shift(-2, labelref(.EncodeCapability))
						int shift(-2, labelref(.DecodeMatrix))
						int shift(-2, labelref(.DecodeMask))
						int shift(-2, labelref(.getbitchars))
						int shift(-2, labelref(.EncodeMatrix))
						int shift(-2, labelref(.MarshalStream))
						int shift(-2, labelref(.MarshalObject))
						int shift(-2, labelref(.MarshalCommon))
						int shift(-2, labelref(.MarshalDate))
						int shift(-2, labelref(.MarshalCap))
						int shift(-2, labelref(.MarshalOffset))
						int shift(-2, labelref(.MarshalWord))
						int shift(-2, labelref(.MarshalData))
						int shift(-2, labelref(.MarshalString))
						int shift(-2, labelref(.InitMCB))
						int shift(-2, labelref(.XchMsg1))
						int shift(-2, labelref(.SendMsg))
						int shift(-2, labelref(.SendIOC))
						int shift(-2, labelref(.TidyUp))
						int shift(-2, labelref(.Exit))
						int shift(-2, labelref(.Abort))
						int shift(-2, labelref(.GetDate))
						int shift(-2, labelref(.Result2))
						int shift(-2, labelref(.Free))
						int shift(-2, labelref(.Malloc))
						int shift(-2, labelref(.GetEnv))
						int shift(-2, labelref(.SendEnv))
						int shift(-2, labelref(.Execute))
						int shift(-2, labelref(.Load))
						int shift(-2, labelref(.Close))
						int shift(-2, labelref(.SetFileSize))
						int shift(-2, labelref(.GetFileSize))
						int shift(-2, labelref(.Seek))
						int shift(-2, labelref(.Write))
						int shift(-2, labelref(.Read))
						int shift(-2, labelref(.ReOpen))
						int shift(-2, labelref(.NewStream))
						int shift(-2, labelref(.NewObject))
						int shift(-2, labelref(.CopyObject))
						int shift(-2, labelref(.Refine))
						int shift(-2, labelref(.Rename))
						int shift(-2, labelref(.Delete))
						int shift(-2, labelref(.Protect))
						int shift(-2, labelref(.SetDate))
						int shift(-2, labelref(.Link))
						int shift(-2, labelref(.ServerInfo))
						int shift(-2, labelref(.ObjectInfo))
						int shift(-2, labelref(.Create))
						int shift(-2, labelref(.Locate))
						int shift(-2, labelref(.Open))
				_FuncTableEnd:			
		ref	Kernel.library
		ref	Util.library
