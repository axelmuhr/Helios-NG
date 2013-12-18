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
				CMPI	2, R0
				Beq	_CodeTableInit
				patchinstr(PATCHC40MASK8ADD,
					shift(1, modnum),
					ldi	*+AR4(0), AR0)
	lsh	-2, AR0
	addi	IR0, AR0
				LDI	R11,	R7
				codetable _NewProcess
				global _NewProcess
				codetable _RunProcess
				global _RunProcess
				codetable _ZapProcess
				global _ZapProcess
				codetable _setjmp
				global _setjmp
				codetable _longjmp
				global _longjmp
				codetable _IOdebug
				global _IOdebug
				codetable _IOputc
				global _IOputc
				codetable _Fork
				global _Fork
				codetable _strlen
				global _strlen
				codetable _strcpy
				global _strcpy
				codetable _strncpy
				global _strncpy
				codetable _strcat
				global _strcat
				codetable _strncat
				global _strncat
				codetable _strcmp
				global _strcmp
				codetable _strncmp
				global _strncmp
				codetable _IOputs
				global _IOputs
				codetable _AccelerateCode
				global _AccelerateCode
				codetable _ExecProcess
				global _ExecProcess
				codetable _back_trace
				global _back_trace
				codetable __wr1chk
				global __wr1chk
				codetable __wr2chk
				global __wr2chk
				codetable __wr4chk
				global __wr4chk
				codetable __rd1chk
				global __rd1chk
				codetable __rd2chk
				global __rd2chk
				codetable __rd4chk
				global __rd4chk
				codetable _procname
				global _procname
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
						int shift(-2, labelref(.procname))
						int shift(-2, labelref(._rd4chk))
						int shift(-2, labelref(._rd2chk))
						int shift(-2, labelref(._rd1chk))
						int shift(-2, labelref(._wr4chk))
						int shift(-2, labelref(._wr2chk))
						int shift(-2, labelref(._wr1chk))
						int shift(-2, labelref(.back_trace))
						int shift(-2, labelref(.ExecProcess))
						int shift(-2, labelref(.AccelerateCode))
						int shift(-2, labelref(.IOputs))
						int shift(-2, labelref(.strncmp))
						int shift(-2, labelref(.strcmp))
						int shift(-2, labelref(.strncat))
						int shift(-2, labelref(.strcat))
						int shift(-2, labelref(.strncpy))
						int shift(-2, labelref(.strcpy))
						int shift(-2, labelref(.strlen))
						int shift(-2, labelref(.Fork))
						int shift(-2, labelref(.IOputc))
						int shift(-2, labelref(.IOdebug))
						int shift(-2, labelref(.longjmp))
						int shift(-2, labelref(.setjmp))
						int shift(-2, labelref(.ZapProcess))
						int shift(-2, labelref(.RunProcess))
						int shift(-2, labelref(.NewProcess))
				_FuncTableEnd:			
		ref	Kernel.library
		ref	SysLib.library
