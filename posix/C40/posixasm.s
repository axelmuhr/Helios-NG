	align
	init
			CMPI	2, R0
			Beq	_MCodeTableInit
			patchinstr(PATCHC40MASK8ADD,
				shift(1, modnum),
				ldi	*+AR4(0), AR0)
	lsh	-2, AR0
	addi	IR0, AR0
			LDI	R11,	R7
		data __vfork_savearea, 164
			b	R7		
			_MCodeTableInit:
				B	R11
			align
		__procname_vfork:
			byte	"vfork", 0
			align
			      word 	0xff000000 | - __procname_vfork * 4
			       export	.vfork	
	.vfork:
		sti	R11, *--AR6(1)	
	patchinstr(PATCHC40MASK24ADD,
		shift(-2, codestub(.Wait)),
		laj	0)
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(_forklock)),
		ldi	*+AR4(0), R0)
	patchinstr(PATCHC40MASK16ADD,
		datasymb(_forklock),
		addi	0, R0)
			nop
		ldi	*AR6++(1), R11	
		ldi	R11, RS
	patchinstr(PATCHC40MASK24ADD,
		shift(-2, codestub(.SaveCPUState)),
		laj	0)
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(__vfork_savearea)),
		ldi	*+AR4(0), R0)
	patchinstr(PATCHC40MASK16ADD,
		datasymb(__vfork_savearea),
		addi	0, R0)
			nop
		ldi	RS, R11		
		cmpi	0, R0
		patchinstr(PATCHC40MASK16ADD,
			shift(-2, labelref(.vfork_start)),
			beq 0)			
		ldi	R1, R0	
		patchinstr(PATCHC40MASK24ADD,
			shift(-2, labelref(.posix_exit)),
			br	0)
		align
	__procname__exit:
		byte	"_exit", 0
		align
			word 	0xff000000 | - __procname__exit * 4
	._exit:
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(_inchild)),
		ldi	*+AR4(0), AR5)
	lsh	-2, AR5
	addi	IR0, AR5
	patchinstr(PATCHC40MASK16ADD,
		shift(-2, datasymb(_inchild)),
		addi	0, AR5)
		ldi	*AR5, AR5
		patchinstr(PATCHC40MASK16ADD,
			shift(-2, labelref(.posix_exit)),
			beq	0)
		ldi	R0, R1	
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(__vfork_savearea)),
		ldi	*+AR4(0), AR5)
	lsh	-2, AR5
	addi	IR0, AR5
	patchinstr(PATCHC40MASK16ADD,
		shift(-2, datasymb(__vfork_savearea)),
		addi	0, AR5)
		patchinstr(PATCHC40MASK24ADD, shift(-2,
			codestub(.RestoreCPUState)),
			brd 0)
			subi	IR0, AR5, R0
			lsh	2, R0
			sti	R1,	*+AR5(12)
