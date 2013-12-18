		module -1
		.ModStart:
		word	0x60f860f8
		word	modsize
		blkb	31,"Disc.Device" byte 0
		word	0
		word	1000
		word	labelref(.DevOpen)
		ref	Kernel.library
		ref	SysLib.library
		ref	Util.library
					.Seek:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Seek)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Seek)),
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
					.Create:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Create)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Create)),
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
					.strcat:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_strcat)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_strcat)),
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
					.TestSemaphore:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_TestSemaphore)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_TestSemaphore)),
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
					.GetFileSize:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_GetFileSize)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_GetFileSize)),
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
					.Result2:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Result2)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Result2)),
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
					.Write:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Write)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Write)),
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
					.Read:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Read)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Read)),
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
					.Close:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Close)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Close)),
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
					.Locate:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Locate)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Locate)),
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
					.Open:
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_Open)),
		ldi	AR4, AR5)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_Open)),
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
