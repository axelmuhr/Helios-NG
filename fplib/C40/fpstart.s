        align
        module  5
.ModStart:
        word    0x60f160f1
		word modsize
	blkb    31, "FpLib"
	byte 0
	word	modnum
	word	1000
		word	datasymb(.MaxData)
        init
	word	codesymb(.MaxCodeP)
FpLib.library:
		global	FpLib.library
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
				codetable _modf
				global _modf
				codetable _ceil
				global _ceil
				codetable _fabs
				global _fabs
				codetable _floor
				global _floor
				codetable _fmod
				global _fmod
				codetable _atan2
				global _atan2
				codetable _cosh
				global _cosh
				codetable _sinh
				global _sinh
				codetable _tanh
				global _tanh
				codetable _acos
				global _acos
				codetable _asin
				global _asin
				codetable _atan
				global _atan
				codetable _cos
				global _cos
				codetable _sin
				global _sin
				codetable _tan
				global _tan
				codetable _exp
				global _exp
				codetable _log
				global _log
				codetable _log10
				global _log10
				codetable _pow
				global _pow
				codetable _sqrt
				global _sqrt
			data __huge_val, 4
			global __huge_val 
			data __huge_val_hi, 4
			global __huge_val_hi 
	ldhi	(((0x7FEFFFFF) >> 16) & 0xffff), R10
	or	((0x7FEFFFFF) & 0xffff), R10
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(__huge_val)),
		ldi	*+AR4(0), AR5)
	lsh	-2, AR5
	addi	IR0, AR5
	patchinstr(PATCHC40MASK16ADD,
		shift(-2, datasymb(__huge_val)),
		addi	0, AR5)
					sti	R10, *AR5
	ldhi	(((0xFFFFFFFF) >> 16) & 0xffff), R10
	or	((0xFFFFFFFF) & 0xffff), R10
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(__huge_val_hi)),
		ldi	*+AR4(0), AR5)
	lsh	-2, AR5
	addi	IR0, AR5
	patchinstr(PATCHC40MASK16ADD,
		shift(-2, datasymb(__huge_val_hi)),
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
						int shift(-2, labelref(.sqrt))
						int shift(-2, labelref(.pow))
						int shift(-2, labelref(.log10))
						int shift(-2, labelref(.log))
						int shift(-2, labelref(.exp))
						int shift(-2, labelref(.tan))
						int shift(-2, labelref(.sin))
						int shift(-2, labelref(.cos))
						int shift(-2, labelref(.atan))
						int shift(-2, labelref(.asin))
						int shift(-2, labelref(.acos))
						int shift(-2, labelref(.tanh))
						int shift(-2, labelref(.sinh))
						int shift(-2, labelref(.cosh))
						int shift(-2, labelref(.atan2))
						int shift(-2, labelref(.fmod))
						int shift(-2, labelref(.floor))
						int shift(-2, labelref(.fabs))
						int shift(-2, labelref(.ceil))
						int shift(-2, labelref(.modf))
				_FuncTableEnd:			
		ref	Kernel.library
		ref	Posix.library
		ref	Util.library
		ref	Clib.library
