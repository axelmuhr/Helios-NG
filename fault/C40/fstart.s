        align
        module  8
.ModStart:
        word    0x60f160f1
		word modsize
	blkb    31, "Fault"
	byte 0
	word	modnum
	word	1000
		word	datasymb(.MaxData)
        init
	word	codesymb(.MaxCodeP)
Fault.library:
		global	Fault.library
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
				codetable _fdbopen
				global _fdbopen
				codetable _fdbclose
				global _fdbclose
				codetable _fdbrewind
				global _fdbrewind
				codetable _fdbfind
				global _fdbfind
				codetable _Fault
				global _Fault
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
						int shift(-2, labelref(.Fault))
						int shift(-2, labelref(.fdbfind))
						int shift(-2, labelref(.fdbrewind))
						int shift(-2, labelref(.fdbclose))
						int shift(-2, labelref(.fdbopen))
				_FuncTableEnd:			
		ref	Kernel.library
		ref	SysLib.library
		ref	ServLib.library
		ref	Util.library
