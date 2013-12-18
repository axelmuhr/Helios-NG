        align
        module  3
.ModStart:
        word    0x60f160f1
		word modsize
	blkb    31, "ServLib"
	byte 0
	word	modnum
	word	1000
		word	datasymb(.MaxData)
        init
	word	codesymb(.MaxCodeP)
ServLib.library:
		global	ServLib.library
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
				codetable _InitNode
				global _InitNode
				codetable _Dispatch
				global _Dispatch
				codetable _GetContext
				global _GetContext
				codetable _GetTarget
				global _GetTarget
				codetable _GetTargetObj
				global _GetTargetObj
				codetable _GetTargetDir
				global _GetTargetDir
				codetable _HandleLink
				global _HandleLink
				codetable _GetName
				global _GetName
				codetable _pathcat
				global _pathcat
				codetable _objname
				global _objname
				codetable _addint
				global _addint
				codetable _Lookup
				global _Lookup
				codetable _Insert
				global _Insert
				codetable _Unlink
				global _Unlink
				codetable _FormOpenReply
				global _FormOpenReply
				codetable _DirServer
				global _DirServer
				codetable _MarshalInfo
				global _MarshalInfo
				codetable _DoLocate
				global _DoLocate
				codetable _DoRename
				global _DoRename
				codetable _DoLink
				global _DoLink
				codetable _DoProtect
				global _DoProtect
				codetable _DoObjInfo
				global _DoObjInfo
				codetable _DoSetDate
				global _DoSetDate
				codetable _DoRefine
				global _DoRefine
				codetable _InvalidFn
				global _InvalidFn
				codetable _NullFn
				global _NullFn
				codetable _ErrorMsg
				global _ErrorMsg
				codetable _UpdMask
				global _UpdMask
				codetable _CheckMask
				global _CheckMask
				codetable _NewCap
				global _NewCap
				codetable _GetAccess
				global _GetAccess
				codetable _Crypt
				global _Crypt
				codetable _NewKey
				global _NewKey
				codetable _AdjustBuffers
				global _AdjustBuffers
				codetable _DoRead
				global _DoRead
				codetable _DoWrite
				global _DoWrite
				codetable _GetReadBuffer
				global _GetReadBuffer
				codetable _GetWriteBuffer
				global _GetWriteBuffer
				codetable _ServMalloc
				global _ServMalloc
			data _SafetySize, 4
			global _SafetySize 
				codetable _DoRevoke
				global _DoRevoke
	ldhi	(((5120) >> 16) & 0xffff), R10
	or	((5120) & 0xffff), R10
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(_SafetySize)),
		ldi	*+AR4(0), AR5)
	lsh	-2, AR5
	addi	IR0, AR5
	patchinstr(PATCHC40MASK16ADD,
		shift(-2, datasymb(_SafetySize)),
		addi	0, AR5)
					sti	R10, *AR5
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
						int shift(-2, labelref(.DoRevoke))
						int shift(-2, labelref(.ServMalloc))
						int shift(-2, labelref(.GetWriteBuffer))
						int shift(-2, labelref(.GetReadBuffer))
						int shift(-2, labelref(.DoWrite))
						int shift(-2, labelref(.DoRead))
						int shift(-2, labelref(.AdjustBuffers))
						int shift(-2, labelref(.NewKey))
						int shift(-2, labelref(.Crypt))
						int shift(-2, labelref(.GetAccess))
						int shift(-2, labelref(.NewCap))
						int shift(-2, labelref(.CheckMask))
						int shift(-2, labelref(.UpdMask))
						int shift(-2, labelref(.ErrorMsg))
						int shift(-2, labelref(.NullFn))
						int shift(-2, labelref(.InvalidFn))
						int shift(-2, labelref(.DoRefine))
						int shift(-2, labelref(.DoSetDate))
						int shift(-2, labelref(.DoObjInfo))
						int shift(-2, labelref(.DoProtect))
						int shift(-2, labelref(.DoLink))
						int shift(-2, labelref(.DoRename))
						int shift(-2, labelref(.DoLocate))
						int shift(-2, labelref(.MarshalInfo))
						int shift(-2, labelref(.DirServer))
						int shift(-2, labelref(.FormOpenReply))
						int shift(-2, labelref(.Unlink))
						int shift(-2, labelref(.Insert))
						int shift(-2, labelref(.Lookup))
						int shift(-2, labelref(.addint))
						int shift(-2, labelref(.objname))
						int shift(-2, labelref(.pathcat))
						int shift(-2, labelref(.GetName))
						int shift(-2, labelref(.HandleLink))
						int shift(-2, labelref(.GetTargetDir))
						int shift(-2, labelref(.GetTargetObj))
						int shift(-2, labelref(.GetTarget))
						int shift(-2, labelref(.GetContext))
						int shift(-2, labelref(.Dispatch))
						int shift(-2, labelref(.InitNode))
				_FuncTableEnd:			
		ref	Kernel.library
		ref	Util.library
		ref	SysLib.library
