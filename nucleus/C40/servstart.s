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
				patchinstr(PATCHC40MASK8ADD,
					shift(1, modnum),
					ldi	*+AR4(0), AR0)
	lsh	-2, AR0
				patchinstr(PATCHC40MASK8ADD,
					shift(1, modnum),
					ldi	*+AR4(1), AR1)
				codetable _InitNode
				global _InitNode
					cmpi	2, R0
					bne	no_codeinit_InitNode
	ldi	R11, AR5			
	laj	_absaddr_.InitNode + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.InitNode:
	int	shift(-2, labelref(.InitNode))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_InitNode)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_InitNode:
				codetable _Dispatch
				global _Dispatch
					cmpi	2, R0
					bne	no_codeinit_Dispatch
	ldi	R11, AR5			
	laj	_absaddr_.Dispatch + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.Dispatch:
	int	shift(-2, labelref(.Dispatch))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_Dispatch)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_Dispatch:
				codetable _GetContext
				global _GetContext
					cmpi	2, R0
					bne	no_codeinit_GetContext
	ldi	R11, AR5			
	laj	_absaddr_.GetContext + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.GetContext:
	int	shift(-2, labelref(.GetContext))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_GetContext)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_GetContext:
				codetable _GetTarget
				global _GetTarget
					cmpi	2, R0
					bne	no_codeinit_GetTarget
	ldi	R11, AR5			
	laj	_absaddr_.GetTarget + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.GetTarget:
	int	shift(-2, labelref(.GetTarget))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_GetTarget)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_GetTarget:
				codetable _GetTargetObj
				global _GetTargetObj
					cmpi	2, R0
					bne	no_codeinit_GetTargetObj
	ldi	R11, AR5			
	laj	_absaddr_.GetTargetObj + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.GetTargetObj:
	int	shift(-2, labelref(.GetTargetObj))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_GetTargetObj)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_GetTargetObj:
				codetable _GetTargetDir
				global _GetTargetDir
					cmpi	2, R0
					bne	no_codeinit_GetTargetDir
	ldi	R11, AR5			
	laj	_absaddr_.GetTargetDir + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.GetTargetDir:
	int	shift(-2, labelref(.GetTargetDir))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_GetTargetDir)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_GetTargetDir:
				codetable _HandleLink
				global _HandleLink
					cmpi	2, R0
					bne	no_codeinit_HandleLink
	ldi	R11, AR5			
	laj	_absaddr_.HandleLink + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.HandleLink:
	int	shift(-2, labelref(.HandleLink))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_HandleLink)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_HandleLink:
				codetable _GetName
				global _GetName
					cmpi	2, R0
					bne	no_codeinit_GetName
	ldi	R11, AR5			
	laj	_absaddr_.GetName + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.GetName:
	int	shift(-2, labelref(.GetName))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_GetName)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_GetName:
				codetable _pathcat
				global _pathcat
					cmpi	2, R0
					bne	no_codeinit_pathcat
	ldi	R11, AR5			
	laj	_absaddr_.pathcat + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.pathcat:
	int	shift(-2, labelref(.pathcat))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_pathcat)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_pathcat:
				codetable _objname
				global _objname
					cmpi	2, R0
					bne	no_codeinit_objname
	ldi	R11, AR5			
	laj	_absaddr_.objname + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.objname:
	int	shift(-2, labelref(.objname))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_objname)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_objname:
				codetable _addint
				global _addint
					cmpi	2, R0
					bne	no_codeinit_addint
	ldi	R11, AR5			
	laj	_absaddr_.addint + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.addint:
	int	shift(-2, labelref(.addint))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_addint)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_addint:
				codetable _Lookup
				global _Lookup
					cmpi	2, R0
					bne	no_codeinit_Lookup
	ldi	R11, AR5			
	laj	_absaddr_.Lookup + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.Lookup:
	int	shift(-2, labelref(.Lookup))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_Lookup)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_Lookup:
				codetable _Insert
				global _Insert
					cmpi	2, R0
					bne	no_codeinit_Insert
	ldi	R11, AR5			
	laj	_absaddr_.Insert + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.Insert:
	int	shift(-2, labelref(.Insert))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_Insert)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_Insert:
				codetable _Unlink
				global _Unlink
					cmpi	2, R0
					bne	no_codeinit_Unlink
	ldi	R11, AR5			
	laj	_absaddr_.Unlink + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.Unlink:
	int	shift(-2, labelref(.Unlink))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_Unlink)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_Unlink:
				codetable _FormOpenReply
				global _FormOpenReply
					cmpi	2, R0
					bne	no_codeinit_FormOpenReply
	ldi	R11, AR5			
	laj	_absaddr_.FormOpenReply + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.FormOpenReply:
	int	shift(-2, labelref(.FormOpenReply))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_FormOpenReply)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_FormOpenReply:
				codetable _DirServer
				global _DirServer
					cmpi	2, R0
					bne	no_codeinit_DirServer
	ldi	R11, AR5			
	laj	_absaddr_.DirServer + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.DirServer:
	int	shift(-2, labelref(.DirServer))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_DirServer)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_DirServer:
				codetable _MarshalInfo
				global _MarshalInfo
					cmpi	2, R0
					bne	no_codeinit_MarshalInfo
	ldi	R11, AR5			
	laj	_absaddr_.MarshalInfo + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.MarshalInfo:
	int	shift(-2, labelref(.MarshalInfo))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_MarshalInfo)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_MarshalInfo:
				codetable _DoLocate
				global _DoLocate
					cmpi	2, R0
					bne	no_codeinit_DoLocate
	ldi	R11, AR5			
	laj	_absaddr_.DoLocate + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.DoLocate:
	int	shift(-2, labelref(.DoLocate))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_DoLocate)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_DoLocate:
				codetable _DoRename
				global _DoRename
					cmpi	2, R0
					bne	no_codeinit_DoRename
	ldi	R11, AR5			
	laj	_absaddr_.DoRename + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.DoRename:
	int	shift(-2, labelref(.DoRename))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_DoRename)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_DoRename:
				codetable _DoLink
				global _DoLink
					cmpi	2, R0
					bne	no_codeinit_DoLink
	ldi	R11, AR5			
	laj	_absaddr_.DoLink + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.DoLink:
	int	shift(-2, labelref(.DoLink))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_DoLink)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_DoLink:
				codetable _DoProtect
				global _DoProtect
					cmpi	2, R0
					bne	no_codeinit_DoProtect
	ldi	R11, AR5			
	laj	_absaddr_.DoProtect + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.DoProtect:
	int	shift(-2, labelref(.DoProtect))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_DoProtect)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_DoProtect:
				codetable _DoObjInfo
				global _DoObjInfo
					cmpi	2, R0
					bne	no_codeinit_DoObjInfo
	ldi	R11, AR5			
	laj	_absaddr_.DoObjInfo + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.DoObjInfo:
	int	shift(-2, labelref(.DoObjInfo))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_DoObjInfo)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_DoObjInfo:
				codetable _DoSetDate
				global _DoSetDate
					cmpi	2, R0
					bne	no_codeinit_DoSetDate
	ldi	R11, AR5			
	laj	_absaddr_.DoSetDate + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.DoSetDate:
	int	shift(-2, labelref(.DoSetDate))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_DoSetDate)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_DoSetDate:
				codetable _DoRefine
				global _DoRefine
					cmpi	2, R0
					bne	no_codeinit_DoRefine
	ldi	R11, AR5			
	laj	_absaddr_.DoRefine + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.DoRefine:
	int	shift(-2, labelref(.DoRefine))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_DoRefine)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_DoRefine:
				codetable _InvalidFn
				global _InvalidFn
					cmpi	2, R0
					bne	no_codeinit_InvalidFn
	ldi	R11, AR5			
	laj	_absaddr_.InvalidFn + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.InvalidFn:
	int	shift(-2, labelref(.InvalidFn))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_InvalidFn)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_InvalidFn:
				codetable _NullFn
				global _NullFn
					cmpi	2, R0
					bne	no_codeinit_NullFn
	ldi	R11, AR5			
	laj	_absaddr_.NullFn + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.NullFn:
	int	shift(-2, labelref(.NullFn))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_NullFn)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_NullFn:
				codetable _ErrorMsg
				global _ErrorMsg
					cmpi	2, R0
					bne	no_codeinit_ErrorMsg
	ldi	R11, AR5			
	laj	_absaddr_.ErrorMsg + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.ErrorMsg:
	int	shift(-2, labelref(.ErrorMsg))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_ErrorMsg)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_ErrorMsg:
				codetable _UpdMask
				global _UpdMask
					cmpi	2, R0
					bne	no_codeinit_UpdMask
	ldi	R11, AR5			
	laj	_absaddr_.UpdMask + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.UpdMask:
	int	shift(-2, labelref(.UpdMask))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_UpdMask)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_UpdMask:
				codetable _CheckMask
				global _CheckMask
					cmpi	2, R0
					bne	no_codeinit_CheckMask
	ldi	R11, AR5			
	laj	_absaddr_.CheckMask + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.CheckMask:
	int	shift(-2, labelref(.CheckMask))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_CheckMask)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_CheckMask:
				codetable _NewCap
				global _NewCap
					cmpi	2, R0
					bne	no_codeinit_NewCap
	ldi	R11, AR5			
	laj	_absaddr_.NewCap + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.NewCap:
	int	shift(-2, labelref(.NewCap))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_NewCap)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_NewCap:
				codetable _GetAccess
				global _GetAccess
					cmpi	2, R0
					bne	no_codeinit_GetAccess
	ldi	R11, AR5			
	laj	_absaddr_.GetAccess + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.GetAccess:
	int	shift(-2, labelref(.GetAccess))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_GetAccess)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_GetAccess:
				codetable _Crypt
				global _Crypt
					cmpi	2, R0
					bne	no_codeinit_Crypt
	ldi	R11, AR5			
	laj	_absaddr_.Crypt + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.Crypt:
	int	shift(-2, labelref(.Crypt))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_Crypt)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_Crypt:
				codetable _NewKey
				global _NewKey
					cmpi	2, R0
					bne	no_codeinit_NewKey
	ldi	R11, AR5			
	laj	_absaddr_.NewKey + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.NewKey:
	int	shift(-2, labelref(.NewKey))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_NewKey)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_NewKey:
				codetable _AdjustBuffers
				global _AdjustBuffers
					cmpi	2, R0
					bne	no_codeinit_AdjustBuffers
	ldi	R11, AR5			
	laj	_absaddr_.AdjustBuffers + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.AdjustBuffers:
	int	shift(-2, labelref(.AdjustBuffers))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_AdjustBuffers)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_AdjustBuffers:
				codetable _DoRead
				global _DoRead
					cmpi	2, R0
					bne	no_codeinit_DoRead
	ldi	R11, AR5			
	laj	_absaddr_.DoRead + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.DoRead:
	int	shift(-2, labelref(.DoRead))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_DoRead)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_DoRead:
				codetable _DoWrite
				global _DoWrite
					cmpi	2, R0
					bne	no_codeinit_DoWrite
	ldi	R11, AR5			
	laj	_absaddr_.DoWrite + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.DoWrite:
	int	shift(-2, labelref(.DoWrite))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_DoWrite)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_DoWrite:
				codetable _GetReadBuffer
				global _GetReadBuffer
					cmpi	2, R0
					bne	no_codeinit_GetReadBuffer
	ldi	R11, AR5			
	laj	_absaddr_.GetReadBuffer + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.GetReadBuffer:
	int	shift(-2, labelref(.GetReadBuffer))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_GetReadBuffer)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_GetReadBuffer:
				codetable _GetWriteBuffer
				global _GetWriteBuffer
					cmpi	2, R0
					bne	no_codeinit_GetWriteBuffer
	ldi	R11, AR5			
	laj	_absaddr_.GetWriteBuffer + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.GetWriteBuffer:
	int	shift(-2, labelref(.GetWriteBuffer))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_GetWriteBuffer)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_GetWriteBuffer:
				codetable _ServMalloc
				global _ServMalloc
					cmpi	2, R0
					bne	no_codeinit_ServMalloc
	ldi	R11, AR5			
	laj	_absaddr_.ServMalloc + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.ServMalloc:
	int	shift(-2, labelref(.ServMalloc))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_ServMalloc)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_ServMalloc:
			data _SafetySize, 4
			global _SafetySize 
				codetable _DoRevoke
				global _DoRevoke
					cmpi	2, R0
					bne	no_codeinit_DoRevoke
	ldi	R11, AR5			
	laj	_absaddr_.DoRevoke + 1		
		nop				
		ldi	R11, AR2
		addi	*AR2, AR2		
_absaddr_.DoRevoke:
	int	shift(-2, labelref(.DoRevoke))
	ldi	AR5, R11			
						ldi	AR1, AR5
						patchinstr(PATCHC40MASK16ADD,
							shift(-2, codesymb(_DoRevoke)),
							addi	0, AR5)
						sti	AR2, *AR5
				no_codeinit_DoRevoke:
					cmpi	0, R0
					bne	no_wordinit_SafetySize
	ldhi	(((5120) >> 16) & 0xffff), R10
	or	((5120) & 0xffff), R10
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(_SafetySize)),
		ldi	*+AR4(0), AR5)
	lsh	-2, AR5
	patchinstr(PATCHC40MASK16ADD,
		shift(-2, datasymb(_SafetySize)),
		addi	0, AR5)
					sti	R10, *AR5
				no_wordinit_SafetySize:
				b	R11		
		ref	Kernel.library
		ref	Util.library
		ref	SysLib.library
					.memset:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_memset)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_memset)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.memcpy:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_memcpy)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_memcpy)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.IOdebug:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_IOdebug)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_IOdebug)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.Fork:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Fork)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Fork)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					._cputime:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(__cputime)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(__cputime)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.longjmp:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_longjmp)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_longjmp)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.setjmp:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_setjmp)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_setjmp)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.strncpy:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_strncpy)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_strncpy)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.strcpy:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_strcpy)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_strcpy)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.strcmp:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_strcmp)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_strcmp)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.strlen:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_strlen)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_strlen)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.DES_Inner:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_DES_Inner)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_DES_Inner)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.DES_KeySchedule:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_DES_KeySchedule)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_DES_KeySchedule)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.DES_CFB:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_DES_CFB)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_DES_CFB)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.splitname:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_splitname)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_splitname)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.GetDate:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_GetDate)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_GetDate)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.MarshalData:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_MarshalData)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_MarshalData)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.MarshalCap:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_MarshalCap)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_MarshalCap)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.MarshalWord:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_MarshalWord)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_MarshalWord)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.MarshalString:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_MarshalString)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_MarshalString)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.InitMCB:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_InitMCB)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_InitMCB)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.SendIOC:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_SendIOC)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_SendIOC)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.Free:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Free)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Free)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.Malloc:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Malloc)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Malloc)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.MachineName:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_MachineName)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_MachineName)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					._FuncToDataConvert:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(__FuncToDataConvert)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(__FuncToDataConvert)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					._linkreg:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(__linkreg)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(__linkreg)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.JTAGHalt:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_JTAGHalt)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_JTAGHalt)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.__stack_overflow:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(___stack_overflow)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(___stack_overflow)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.__uremainder:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(___uremainder)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(___uremainder)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.__remainder:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(___remainder)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(___remainder)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.__udivide:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(___udivide)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(___udivide)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.__divide:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(___divide)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(___divide)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.__divtest:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(___divtest)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(___divtest)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.SearchList:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_SearchList)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_SearchList)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					._Trace:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(__Trace)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(__Trace)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.Delay:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Delay)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Delay)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.Remove:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Remove)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Remove)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.RemTail:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_RemTail)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_RemTail)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.AddTail:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_AddTail)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_AddTail)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.InitList:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_InitList)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_InitList)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.InitSemaphore:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_InitSemaphore)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_InitSemaphore)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.Signal:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Signal)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Signal)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.Wait:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Wait)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Wait)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.PutMsg:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_PutMsg)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_PutMsg)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
					.GetMsg:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_GetMsg)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_GetMsg)),
		addi	1, AR5)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*AR5, AR5)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, AR5)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	AR5)
	b	AR5
