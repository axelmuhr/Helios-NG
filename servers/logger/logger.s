	align
	module	-1
.ModStart:
	word	#60f160f1
	word	.ModEnd-.ModStart
	blkb	31,"logger.c" byte 0
	word	modnum
	word	1
	word	.MaxData
	init
..0: -- 1 refs
	byte	"logger"
	byte	0
	align
..14: -- 1 refs
.main:
	ajw	-139
	ldl	140
	ldnl	0
	ldnl	modnum
	stl	138
	ldc	..15-2
	ldpi
	stl	137
-- Line 125 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 125 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	stl	136
-- Line 129 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 130 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	1110
	stl	2
	ldc	0
	stl	1
	ldl	140
	ldnl	0
	ldnl	@_MyTask
	ldnl	_MyTask
	ldnl	12
	stl	0
	ldc	0
	ldlp	129
	ldl	140
	call	.InitMCB
-- Line 131 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldlp	129
	ldl	140
	call	.PutMsg
-- Line 135 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	138
	ldnlp	..dataseg+10
	ldl	140
	call	.InitSemaphore
-- Line 136 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	1
	ldl	138
	ldnlp	..dataseg+13
	ldl	140
	call	.InitSemaphore
-- Line 143 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	138
	ldnl	..dataseg+1
	adc	15
	ldc	-16
	and
	ldl	138
	stnl	..dataseg+1
-- Line 145 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	138
	ldnl	..dataseg+1
	ldl	140
	call	.ServMalloc
	ldl	138
	stnl	..dataseg+0
-- Line 146 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	138
	ldnl	..dataseg+0
	eqc	0
	cj	..2
-- Line 148 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	140
	call	.Terminate
..2: -- 2 refs
-- Line 155 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	137
	ldnl	0
	stl	2
	ldl	137
	ldnl	1
	stl	1
	ldc	18
	stl	0
	ldl	138
	ldnl	..dataseg+3
	ldl	138
	ldnlp	..dataseg+16
	ldl	140
	call	.InitNode
-- Line 156 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	138
	ldnlp	..dataseg+16
	adc	92
	ldl	140
	call	.InitList
-- Line 157 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	138
	ldnlp	..dataseg+16
	stnl	17
-- Line 159 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 163 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldlp	8
	ldl	140
	call	.MachineName
-- Line 164 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldlp	8
	ldc	0
	ldl	140
	call	.Locate
	stl	3
-- Line 165 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	eqc	0
	cj	..4
-- Line 167 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	140
	call	.Terminate
..4: -- 2 refs
-- Line 174 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	140
	call	.NewPort
	stl	2
	ldl	2
	ldl	138
	ldnlp	..dataseg+42
	stnl	1
	ldl	2
	stl	4
-- Line 175 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	128
	stl	5
-- Line 176 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	137
	ldnl	2
	stl	6
-- Line 177 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	stl	7
-- Line 179 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldlp	4
	stl	2
	ldc	16
	stl	1
	ldc	112
	stl	0
	ldl	138
	ldnl	..dataseg+3
	ldl	3
	ldl	140
	call	.Create
	stl	136
-- Line 181 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldl	140
	call	.Close
-- Line 184 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	138
	ldnl	..dataseg+7
	eqc	0
	cj	..6
-- Line 185 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	140
	call	.intercept_IOdebugs
..6: -- 2 refs
-- Line 187 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	138
	ldnlp	..dataseg+10
	ldl	140
	call	.Signal
-- Line 189 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	138
	ldnlp	..dataseg+42
	ldl	140
	call	.Dispatch
-- Line 191 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	136
	ldl	140
	call	.Delete
-- Line 192 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	136
	ldl	140
	call	.Close
-- Line 194 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	138
	ldnl	..dataseg+6
	cj	..8
-- Line 195 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	138
	ldnl	..dataseg+6
	ldl	140
	call	.CloseDevice
..8: -- 2 refs
-- Line 197 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	138
	ldnl	..dataseg+9
	cj	..10
-- Line 198 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	138
	ldnl	..dataseg+9
	ldl	140
	call	.FreePort
..10: -- 2 refs
-- Line 200 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	138
	ldnl	..dataseg+7
	eqc	0
	cj	..12
-- Line 201 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	138
	ldnl	..dataseg+8
	mint
	ldnl	1024
	mint
	ldnlp	1024
	add
	stnl	50
..12: -- 2 refs
-- Line 203 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	140
	call	.Exit
	ajw	139
	ret
-- Literals
	align
..15: -- 1 refs
	byte	#c3,#43,#03,#01
	byte	#00,#00,#01,#00
	byte	#07,#09,#11,#21
	align
..17: -- 1 refs
.intercept_IOdebugs:
	ajw	-3
	ldl	4
	ldnl	0
	ldnl	modnum
	stl	2
-- Line 333 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 333 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	mint
	ldnl	1024
	mint
	adc	4096
	add
	stl	1
-- Line 335 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	call	.NewPort
	ldl	2
	stnl	..dataseg+9
-- Line 336 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	1
	ldnl	50
	ldl	2
	stnl	..dataseg+8
-- Line 337 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	2
	ldnl	..dataseg+9
	ldl	1
	stnl	50
-- Line 339 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	stl	0
	ldc	.IOdebug_process-2
	ldpi
	ldc	2000
	ldl	4
	call	.Fork
	ajw	3
	ret
	align
..38: -- 1 refs
.IOdebug_process:
	ajw	-15
	ldc	..39-2
	ldpi
	stl	14
-- Line 347 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 352 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldl	16
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+79
	ldl	16
	call	.strcpy
-- Line 353 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	4
	stl	5
-- Line 355 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	align
..19: -- 7 refs
-- Line 356 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 356 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	16
	ldnl	0
	ldnl	modnum
	ldnl	..dataseg+9
	stl	7
-- Line 357 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	-1
	stl	10
-- Line 358 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	16
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+79
	ldl	5
	add
	stl	12
-- Line 360 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldlp	6
	ldl	16
	call	.GetMsg
	stl	13
-- Line 361 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	13
	gt
	cj	..21
-- Line 362 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 362 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	13
	ldl	14
	ldnl	2
	and
	stl	13
-- Line 363 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	13
	ldl	14
	ldnl	3
	diff
	cj	..24
	ldl	13
	ldl	14
	ldnl	2
	diff
	eqc	0
	cj	..19
	align
..24: -- 3 refs
-- Line 364 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	15
	ret
	align
..21: -- 2 refs
-- Line 368 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	13
	ldl	14
	ldnl	4
	diff
	eqc	0
	cj	..19
-- Line 371 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	stl	4
	align
..31: -- 2 refs
	ldlp	6
	ldc	0
	stl	3
	ldlp	3
	ldc	2
	move
	ldl	3
	ldl	4
	gt
	cj	..32
-- Line 372 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	5
	ldl	4
	add
	ldl	16
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+79
	bsub
	lb
	eqc	10
	cj	..34
-- Line 373 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 373 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	5
	ldl	4
	add
	adc	1
	ldl	16
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+79
	bsub
	lb
	stl	3
-- Line 374 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	5
	ldl	4
	add
	adc	1
	ldl	16
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+79
	bsub
	sb
-- Line 375 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	16
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+79
	ldl	16
	call	.write_to_log
-- Line 376 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldlp	3
	lb
	ldl	5
	ldl	4
	add
	adc	1
	ldl	16
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+79
	bsub
	sb
-- Line 378 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldlp	6
	ldc	0
	stl	2
	ldlp	2
	ldc	2
	move
	ldl	2
	ldl	4
	adc	1
	sub
	ldc	65535
	and
	stl	2
	ldlp	6
	ldlp	2
	rev
	ldc	2
	move
-- Line 379 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldlp	6
	ldc	0
	stl	1
	ldlp	1
	ldc	2
	move
	ldl	1
	ldc	0
	gt
	cj	..36
-- Line 380 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldlp	6
	ldc	0
	stl	1
	ldlp	1
	ldc	2
	move
	ldl	1
	stl	0
	ldl	16
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+79
	ldl	5
	ldl	4
	add
	adc	1
	add
	ldl	16
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+79
	adc	4
	ldl	16
	call	.memcpy
	align
..36: -- 2 refs
-- Line 381 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	4
	stl	5
-- Line 382 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	-1
	stl	4
	align
..34: -- 2 refs
	ldl	4
	adc	1
	stl	4
	j	..31
	align
..32: -- 1 refs
-- Line 384 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldlp	6
	ldc	0
	stl	3
	ldlp	3
	ldc	2
	move
	ldl	3
	ldl	5
	add
	stl	5
	j	..19
-- Literals
	align
..39: -- 1 refs
	byte	"*** "
	byte	0
	align
	byte	#00,#00,#00,#e0
	byte	#00,#00,#00,#c0
	byte	#22,#22,#22,#22
	align
..63: -- 1 refs
.Logger_Private:
	ajw	-13
	ldl	14
	ldnl	0
	ldnl	modnum
	stl	12
	ldc	..64-2
	ldpi
	stl	11
-- Line 400 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 400 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	15
	ldnl	1
	stl	7
-- Line 400 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnl	5
	stl	8
-- Line 405 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	15
	ldl	14
	call	.GetTargetDir
	stl	10
-- Line 406 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	10
	eqc	0
	cj	..41
-- Line 407 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 407 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	32773
	ldl	7
	ldl	14
	call	.ErrorMsg
-- Line 407 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	13
	ret
..41: -- 1 refs
-- Line 409 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	15
	ldl	14
	call	.GetTargetObj
	stl	9
-- Line 410 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	9
	eqc	0
	cj	..43
-- Line 411 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 411 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	32780
	ldl	7
	ldl	14
	call	.ErrorMsg
-- Line 411 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	13
	ret
..43: -- 1 refs
-- Line 414 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	9
	ldl	12
	ldnlp	..dataseg+16
	diff
	cj	..45
-- Line 415 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 415 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	0
	ldl	7
	ldl	14
	call	.ErrorMsg
-- Line 415 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	13
	ret
..45: -- 1 refs
-- Line 417 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	15
	ldnl	5
	ldl	11
	ldnl	1
	and
	eqc	4208
	eqc	0
	cj	..47
-- Line 418 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 418 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	0
	ldl	7
	ldl	14
	call	.ErrorMsg
-- Line 418 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	13
	ret
..47: -- 1 refs
-- Line 420 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	8
	ldnl	5
	eqc	1
	cj	..49
-- Line 421 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 421 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	12
	ldnl	..dataseg+5
	ldl	12
	stnl	..dataseg+4
-- Line 422 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	7
	stnl	3
-- Line 423 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	7
	ldl	14
	call	.ErrorMsg
-- Line 424 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	13
	ret
..49: -- 1 refs
-- Line 427 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	8
	ldnl	5
	eqc	3
	cj	..51
-- Line 428 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 428 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	12
	stnl	..dataseg+4
-- Line 429 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	7
	stnl	3
-- Line 430 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	7
	ldl	14
	call	.ErrorMsg
-- Line 431 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	13
	ret
..51: -- 1 refs
-- Line 434 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	8
	ldnl	5
	eqc	2
	cj	..53
-- Line 435 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 435 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnlp	2
	stl	5
-- Line 435 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	5
	ldl	14
	call	.strlen
	stl	4
-- Line 435 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnl	5
	ldnl	6
	stl	6
-- Line 435 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnl	6
	ldl	6
	add
	stl	3
-- Line 441 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldnl	0
	stl	0
	ldl	3
	adc	8
	ldl	3
	adc	16
	ldl	14
	call	.NewStream
	stl	2
-- Line 442 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	2
	eqc	0
	cj	..55
-- Line 443 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 443 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	9
	ldl	7
	ldl	14
	call	.ErrorMsg
-- Line 443 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	13
	ret
..55: -- 1 refs
-- Line 445 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldnl	1
	ldl	2
	stnl	10
-- Line 446 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	-1
	stl	1
	ldl	4
	stl	0
	ldl	5
	ldl	2
	ldl	14
	call	.Write
	ldl	4
	diff
	cj	..57
-- Line 447 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 447 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	9
	ldl	7
	ldl	14
	call	.ErrorMsg
-- Line 448 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	2
	ldl	14
	call	.Close
-- Line 449 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	13
	ret
..57: -- 1 refs
-- Line 452 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	12
	ldnl	..dataseg+4
	ldl	12
	ldnl	..dataseg+5
	diff
	cj	..59
-- Line 452 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	12
	ldnl	..dataseg+4
	ldl	14
	call	.Close
..59: -- 2 refs
-- Line 453 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	2
	ldl	12
	stnl	..dataseg+4
-- Line 454 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	7
	stnl	3
-- Line 455 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	7
	ldl	14
	call	.ErrorMsg
-- Line 456 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	13
	ret
..53: -- 1 refs
-- Line 459 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	8
	ldnl	5
	eqc	4
	cj	..61
-- Line 460 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 460 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	7
	stnl	3
-- Line 461 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	7
	ldl	14
	call	.ErrorMsg
-- Line 462 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	10
	ldl	12
	ldnlp	..dataseg+42
	ldnl	1
	ldl	14
	call	.AbortPort
..61: -- 2 refs
-- Line 465 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	0
	ldl	7
	ldl	14
	call	.ErrorMsg
	ajw	13
	ret
-- Literals
	align
..64: -- 1 refs
	byte	#06,#80,#0c,#c0
	byte	#f0,#ff,#0f,#00
	byte	"Logger: output redirected\n"
	byte	0
	align
	byte	#07,#80,#11,#c0
	byte	#00,#00,#00,#e0
	align
..74: -- 1 refs
.Logger_ServerInfo:
	ajw	-13
	ldl	14
	ldnl	0
	ldnl	modnum
	stl	12
	ldc	..75-2
	ldpi
	stl	11
-- Line 470 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 470 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	15
	ldnl	1
	stl	3
-- Line 470 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldnl	5
	stl	9
-- Line 476 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	15
	ldl	14
	call	.GetTargetDir
	stl	10
-- Line 477 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	10
	eqc	0
	cj	..66
-- Line 478 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 478 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	32773
	ldl	3
	ldl	14
	call	.ErrorMsg
-- Line 478 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	13
	ret
..66: -- 1 refs
-- Line 480 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	15
	ldl	14
	call	.GetTargetObj
	stl	8
-- Line 481 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	8
	eqc	0
	cj	..68
-- Line 482 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 482 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	32780
	ldl	3
	ldl	14
	call	.ErrorMsg
-- Line 482 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	13
	ret
..68: -- 1 refs
-- Line 484 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	1
	ldl	9
	ldnlp	3
	lb
	ldl	14
	call	.CheckMask
	eqc	0
	cj	..70
-- Line 485 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 485 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	0
	ldl	3
	ldl	14
	call	.ErrorMsg
-- Line 485 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	13
	ret
..70: -- 1 refs
-- Line 487 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	8
	ldl	12
	ldnlp	..dataseg+16
	diff
	cj	..72
-- Line 488 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 488 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	1
	ldl	3
	ldl	14
	call	.ErrorMsg
-- Line 488 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	13
	ret
..72: -- 1 refs
-- Line 490 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	12
	ldnlp	..dataseg+10
	ldl	14
	call	.Wait
-- Line 491 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	2
	stl	4
-- Line 492 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	12
	ldnl	..dataseg+1
	stl	5
-- Line 493 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	12
	ldnl	..dataseg+1
	ldl	12
	ldnl	..dataseg+2
	sub
	stl	6
-- Line 494 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	12
	ldnl	..dataseg+2
	stl	7
-- Line 495 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	12
	ldnlp	..dataseg+10
	ldl	14
	call	.Signal
-- Line 497 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	stl	2
	ldc	0
	stl	1
	ldl	3
	ldnl	2
	stl	0
	ldc	0
	ldl	3
	ldl	14
	call	.InitMCB
-- Line 498 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldlp	4
	ldl	3
	stnl	6
-- Line 499 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	16
	stl	2
	ldl	3
	ldlp	2
	rev
	ldc	2
	move
-- Line 500 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldl	14
	call	.PutMsg
	ajw	13
	ret
-- Literals
	align
..75: -- 1 refs
	byte	#0c,#80,#04,#c0
	byte	#06,#80,#0c,#c0
	byte	#00,#00,#00,#40
	align
..85: -- 1 refs
.Logger_Delete:
	ajw	-5
	ldc	..86-2
	ldpi
	stl	4
-- Line 504 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 504 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnl	1
	stl	0
-- Line 504 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	0
	ldnl	5
	stl	2
-- Line 509 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldl	6
	call	.GetTargetDir
	stl	3
-- Line 510 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	eqc	0
	cj	..77
-- Line 511 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 511 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	32773
	ldl	0
	ldl	6
	call	.ErrorMsg
-- Line 511 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	5
	ret
..77: -- 1 refs
-- Line 513 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldl	6
	call	.GetTargetObj
	stl	1
-- Line 514 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	1
	eqc	0
	cj	..79
-- Line 515 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 515 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	32780
	ldl	0
	ldl	6
	call	.ErrorMsg
-- Line 515 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	5
	ret
..79: -- 1 refs
-- Line 517 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	64
	ldl	2
	ldnlp	3
	lb
	ldl	6
	call	.CheckMask
	eqc	0
	cj	..81
-- Line 518 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 518 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	ldnl	0
	ldl	0
	ldl	6
	call	.ErrorMsg
-- Line 518 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	5
	ret
..81: -- 1 refs
-- Line 520 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	1
	ldl	6
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+16
	diff
	cj	..83
-- Line 521 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 521 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	ldnl	1
	ldl	0
	ldl	6
	call	.ErrorMsg
-- Line 521 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	5
	ret
..83: -- 1 refs
-- Line 523 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	6
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+10
	ldl	6
	call	.Wait
-- Line 524 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	6
	ldnl	0
	ldnl	modnum
	stnl	..dataseg+2
-- Line 525 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	6
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+10
	ldl	6
	call	.Signal
-- Line 527 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	0
	stnl	3
-- Line 528 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	0
	ldl	6
	call	.ErrorMsg
	ajw	5
	ret
-- Literals
	align
..86: -- 1 refs
	byte	#0c,#80,#04,#c0
	byte	#06,#80,#0c,#c0
	align
..130: -- 1 refs
.Logger_Open:
	ajw	-12
	ldc	..131-2
	ldpi
	stl	11
-- Line 537 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 537 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldnl	1
	stl	3
-- Line 537 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldnl	5
	stl	8
-- Line 537 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldnl	6
	stl	6
-- Line 537 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	adc	32
	stl	9
-- Line 546 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldl	13
	call	.GetTargetDir
	stl	10
-- Line 547 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	10
	eqc	0
	cj	..88
-- Line 548 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 548 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	32773
	ldl	3
	ldl	13
	call	.ErrorMsg
-- Line 548 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	12
	ret
..88: -- 1 refs
-- Line 550 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldl	13
	call	.GetTargetObj
	stl	4
-- Line 551 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	eqc	0
	cj	..90
-- Line 552 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 552 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	32780
	ldl	3
	ldl	13
	call	.ErrorMsg
-- Line 552 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	12
	ret
..90: -- 1 refs
-- Line 554 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	8
	ldnl	5
	ldc	15
	and
	ldl	8
	ldnlp	3
	lb
	ldl	13
	call	.CheckMask
	eqc	0
	cj	..92
-- Line 555 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 555 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	0
	ldl	3
	ldl	13
	call	.ErrorMsg
-- Line 555 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	12
	ret
..92: -- 1 refs
-- Line 557 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	ldl	13
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+16
	diff
	cj	..94
-- Line 558 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 558 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	1
	ldl	3
	ldl	13
	call	.ErrorMsg
-- Line 558 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	12
	ret
..94: -- 1 refs
-- Line 560 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	612
	ldl	13
	call	.ServMalloc
	stl	7
-- Line 561 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	eqc	0
	cj	..96
-- Line 562 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 562 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	2
	ldl	3
	ldl	13
	call	.ErrorMsg
-- Line 562 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	12
	ret
..96: -- 1 refs
-- Line 564 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	9
	stl	2
	ldl	11
	ldnl	3
	stl	1
	ldl	4
	stl	0
	ldl	3
	ldl	7
	ldl	13
	call	.FormOpenReply
-- Line 565 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	13
	call	.NewPort
	stl	5
	ldl	5
	ldl	7
	stnl	4
-- Line 566 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	adc	8
	ldl	13
	call	.PutMsg
-- Line 567 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldl	13
	call	.Free
-- Line 569 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	ldnl	21
	adc	1
	ldl	4
	stnl	21
-- Line 570 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldnl	4
	cj	..100
-- Line 570 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 570 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldnl	3
	adc	52
	ldl	13
	call	.Signal
-- Line 570 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	14
	stnl	4
	align
..100: -- 13 refs
-- Line 572 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 574 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	5
	ldl	3
	stnl	1
-- Line 575 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	4
	ldl	3
	stnl	4
-- Line 576 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	6
	ldl	3
	stnl	6
-- Line 578 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldl	13
	call	.GetMsg
	stl	2
-- Line 579 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	5
	ldl	3
	stnl	3
-- Line 581 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	2
	gt
	cj	..102
-- Line 582 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 582 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	2
	ldl	11
	ldnl	6
	diff
	eqc	0
	eqc	0
	cj	..101
-- Line 583 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	2
	ldl	11
	ldnl	7
	and
	stl	2
-- Line 584 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	2
	ldl	11
	ldnl	8
	diff
	cj	..101
	ldl	2
	ldl	11
	ldnl	7
	diff
	eqc	0
	cj	..100
	j	..101
	align
..102: -- 2 refs
-- Line 590 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	2
	ldl	11
	ldnl	9
	and
	cj	..112
-- Line 591 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 591 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	10
	ldl	3
	ldl	13
	call	.ErrorMsg
-- Line 591 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	j	..100
	align
..112: -- 1 refs
-- Line 595 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	2
	ldl	11
	ldnl	11
	and
	stl	1
	ldl	1
	ldc	4144
	gt
	cj	..121
	ldl	1
	adc	-4176
	cj	..117
	ldl	1
	adc	-4192
	cj	..116
	j	..115
	align
..121: -- 1 refs
	diff
	ldl	1
	adc	-4112
	cj	..120
	ldl	1
	adc	-4128
	cj	..119
	ldl	1
	adc	-4144
	cj	..118
	j	..115
	align
..120: -- 1 refs
-- Line 596 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldl	13
	call	.Logger_Read
-- Line 596 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	j	..100
	align
..119: -- 1 refs
-- Line 597 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldl	13
	call	.Logger_Write
-- Line 597 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	j	..100
	align
..117: -- 1 refs
-- Line 599 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldnl	2
	cj	..128
-- Line 600 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 600 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	3
	stnl	3
-- Line 601 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	3
	ldl	13
	call	.ErrorMsg
	align
..128: -- 2 refs
-- Line 603 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	5
	ldl	13
	call	.FreePort
-- Line 604 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	ldnl	21
	adc	-1
	ldl	4
	stnl	21
-- Line 605 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	12
	ret
	align
..116: -- 1 refs
-- Line 607 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldl	13
	call	.Logger_Seek
-- Line 607 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	j	..100
	align
..118: -- 1 refs
-- Line 609 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldl	13
	call	.Logger_GetSize
-- Line 609 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	j	..100
	align
..115: -- 4 refs
-- Line 611 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	10
	ldl	3
	ldl	13
	call	.ErrorMsg
-- Line 612 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	j	..100
	align
..101: -- 6 refs
-- Line 616 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	ldnl	21
	adc	-1
	ldl	4
	stnl	21
-- Line 617 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	5
	ldl	13
	call	.FreePort
	ajw	12
	ret
-- Literals
	align
..131: -- 1 refs
	byte	#0c,#80,#04,#c0
	byte	#06,#80,#0c,#c0
	byte	#01,#80,#01,#c0
	byte	#00,#00,#01,#20
	byte	#00,#d2,#49,#6b
	byte	#00,#00,#00,#1b
	byte	#01,#80,#05,#81
	byte	#00,#00,#00,#e0
	byte	#00,#00,#00,#c0
	byte	#00,#00,#00,#60
	byte	#07,#80,#0c,#c0
	byte	#f0,#ff,#0f,#00
	align
..147: -- 1 refs
.Logger_Seek:
	ajw	-7
	ldl	8
	ldnl	0
	ldnl	modnum
	stl	6
	ldc	..148-2
	ldpi
	stl	5
-- Line 621 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 621 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	9
	ldnl	5
	stl	3
-- Line 624 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	6
	ldnlp	..dataseg+10
	ldl	8
	call	.Wait
-- Line 626 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldnl	1
	eqc	0
	cj	..133
-- Line 627 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldnl	2
	stl	4
	j	..135
..133: -- 1 refs
-- Line 628 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldnl	1
	eqc	1
	cj	..136
-- Line 629 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldnl	0
	ldl	3
	ldnl	2
	add
	stl	4
	j	..135
..136: -- 1 refs
-- Line 630 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldnl	1
	eqc	2
	cj	..139
-- Line 631 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	6
	ldnl	..dataseg+2
	stl	4
	j	..135
..139: -- 1 refs
-- Line 633 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 633 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	5
	ldnl	0
	ldl	9
	ldl	8
	call	.ErrorMsg
-- Line 633 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	j	..142
..135: -- 5 refs
-- Line 635 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	4
	gt
	eqc	0
	cj	..144
	ldl	4
	ldl	6
	ldnl	..dataseg+2
	gt
	cj	..143
..144: -- 3 refs
-- Line 636 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 636 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	5
	ldnl	1
	ldl	9
	ldl	8
	call	.ErrorMsg
-- Line 636 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	j	..142
..143: -- 1 refs
-- Line 638 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	stl	2
	ldc	0
	stl	1
	ldl	9
	ldnl	2
	stl	0
	ldc	0
	ldl	9
	ldl	8
	call	.InitMCB
-- Line 639 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	ldl	9
	ldl	8
	call	.MarshalWord
-- Line 640 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	9
	ldl	8
	call	.PutMsg
-- Line 642 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
..142: -- 3 refs
-- Line 643 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	6
	ldnlp	..dataseg+10
	ldl	8
	call	.Signal
	ajw	7
	ret
-- Literals
	align
..148: -- 1 refs
	byte	#01,#00,#ff,#c0
	byte	#0c,#80,#0f,#c0
	align
..150: -- 1 refs
.Logger_GetSize:
	ajw	-3
-- Line 647 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 647 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+10
	ldl	4
	call	.Wait
-- Line 648 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	stl	2
	ldc	0
	stl	1
	ldl	5
	ldnl	2
	stl	0
	ldc	0
	ldl	5
	ldl	4
	call	.InitMCB
-- Line 649 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	ldnl	0
	ldnl	modnum
	ldnl	..dataseg+2
	ldl	5
	ldl	4
	call	.MarshalWord
-- Line 650 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	5
	ldl	4
	call	.PutMsg
-- Line 651 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	ldnl	0
	ldnl	modnum
	ldnlp	..dataseg+10
	ldl	4
	call	.Signal
	ajw	3
	ret
	align
..172: -- 1 refs
.Logger_Read:
	ajw	-10
	ldl	11
	ldnl	0
	ldnl	modnum
	stl	9
	ldc	..173-2
	ldpi
	stl	8
-- Line 655 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 655 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	12
	ldnl	5
	stl	6
-- Line 655 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	12
	ldnl	2
	stl	7
-- Line 659 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	9
	ldnlp	..dataseg+10
	ldl	11
	call	.Wait
-- Line 660 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	9
	ldnl	..dataseg+2
	ldl	6
	ldnl	0
	gt
	eqc	0
	cj	..152
-- Line 661 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 661 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	2
	stl	2
	ldc	0
	stl	1
	ldl	7
	stl	0
	ldc	0
	ldl	12
	ldl	11
	call	.InitMCB
-- Line 662 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	12
	ldl	11
	call	.PutMsg
-- Line 663 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	j	..154
..152: -- 1 refs
-- Line 666 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	6
	ldnl	0
	gt
	cj	..155
-- Line 667 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 667 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	8
	ldnl	0
	ldl	12
	ldl	11
	call	.ErrorMsg
-- Line 667 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	j	..154
..155: -- 1 refs
-- Line 669 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	6
	ldnl	1
	gt
	cj	..157
-- Line 670 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 670 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	8
	ldnl	1
	ldl	12
	ldl	11
	call	.ErrorMsg
-- Line 670 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	j	..154
..157: -- 1 refs
-- Line 672 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	6
	ldnl	0
	ldl	6
	ldnl	1
	add
	ldl	9
	ldnl	..dataseg+2
	gt
	cj	..159
-- Line 673 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	9
	ldnl	..dataseg+2
	ldl	6
	ldnl	0
	sub
	ldl	6
	stnl	1
..159: -- 2 refs
-- Line 677 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	stl	5
	align
..161: -- 2 refs
	ldl	6
	ldnl	1
	ldl	5
	gt
	cj	..154
-- Line 678 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 678 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	stl	3
-- Line 681 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	5
	adc	16384
	ldl	6
	ldnl	1
	gt
	cj	..164
-- Line 682 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 682 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	6
	ldnl	1
	ldl	5
	sub
	stl	4
-- Line 682 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	1
	stl	3
	align
..164: -- 2 refs
-- Line 684 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	cj	..167
	ldc	1
	ldc	0
	cj	..166
	align
..167: -- 1 refs
	ldc	0
	ldc	0
	align
..166: -- 2 refs
	diff
	stl	2
	ldc	0
	stl	1
	ldl	7
	stl	0
	ldl	3
	cj	..170
	ldc	0
	ldc	0
	cj	..169
	align
..170: -- 1 refs
	ldc	64
	ldc	0
	align
..169: -- 2 refs
	diff
	ldl	12
	ldl	11
	call	.InitMCB
-- Line 686 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	ldc	65535
	and
	stl	2
	ldl	12
	ldlp	2
	rev
	ldc	2
	move
-- Line 687 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	9
	ldnl	..dataseg+0
	ldl	6
	ldnl	0
	ldl	5
	add
	add
	ldl	12
	stnl	6
-- Line 688 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	12
	ldl	11
	call	.PutMsg
	ldl	5
	adc	16384
	stl	5
	j	..161
..154: -- 5 refs
-- Line 693 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	9
	ldnlp	..dataseg+10
	ldl	11
	call	.Signal
	ajw	10
	ret
-- Literals
	align
..173: -- 1 refs
	byte	#01,#00,#ff,#c0
	byte	#02,#00,#ff,#c0
	align
..191: -- 1 refs
.Logger_Write:
	ajw	-12
	ldc	..192-2
	ldpi
	stl	11
-- Line 697 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 697 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldnl	5
	stl	10
-- Line 697 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	stl	9
-- Line 697 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldnl	2
	stl	8
-- Line 697 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldnl	1
	stl	6
-- Line 706 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	10
	ldnl	1
	stl	5
-- Line 709 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldc	0
	stl	2
	ldlp	2
	ldc	2
	move
	ldl	2
	cj	..175
-- Line 710 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldnl	6
	stl	7
	j	..177
..175: -- 1 refs
-- Line 712 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 712 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	10
	ldnl	1
	adc	1
	ldl	13
	call	.ServMalloc
	stl	7
	ldl	7
	eqc	0
	cj	..178
-- Line 713 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 713 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	11
	ldnl	0
	ldl	14
	ldl	13
	call	.ErrorMsg
-- Line 713 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ajw	12
	ret
..178: -- 1 refs
-- Line 714 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	1
	stl	9
-- Line 717 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	1
	stl	2
	ldc	0
	stl	1
	ldl	8
	stl	0
	ldc	64
	ldl	14
	ldl	13
	call	.InitMCB
-- Line 718 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	5
	ldc	16384
	gt
	cj	..181
	ldc	16384
	ldc	0
	cj	..180
..181: -- 1 refs
	ldl	5
	ldc	0
..180: -- 2 refs
	diff
	ldl	14
	ldl	13
	call	.MarshalWord
-- Line 719 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	16384
	ldl	14
	ldl	13
	call	.MarshalWord
-- Line 720 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldl	13
	call	.PutMsg
-- Line 722 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	stl	4
-- Line 722 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	stl	3
-- Line 723 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	align
..183: -- 2 refs
	ldl	5
	ldl	3
	gt
	cj	..177
-- Line 724 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 724 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	6
	ldl	14
	stnl	1
-- Line 725 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	ldl	14
	stnl	6
-- Line 726 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldl	13
	call	.GetMsg
	ldc	0
	rev
	gt
	eqc	0
	cj	..188
-- Line 728 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldc	0
	stl	2
	ldlp	2
	ldc	2
	move
	ldl	2
	ldl	3
	add
	stl	3
-- Line 729 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldc	0
	stl	2
	ldlp	2
	ldc	2
	move
	ldl	2
	ldl	4
	add
	stl	4
	j	..183
..177: -- 3 refs
-- Line 733 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	stl	2
	ldc	0
	stl	1
	ldl	8
	stl	0
	ldc	0
	ldl	14
	ldl	13
	call	.InitMCB
-- Line 734 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	5
	ldl	14
	ldl	13
	call	.MarshalWord
-- Line 735 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	14
	ldl	13
	call	.PutMsg
-- Line 739 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	5
	ldl	7
	bsub
	sb
-- Line 740 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldl	13
	call	.write_to_log
-- Line 742 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
..188: -- 3 refs
-- Line 743 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	9
	cj	..189
-- Line 743 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldl	13
	call	.Free
..189: -- 2 refs
	ajw	12
	ret
-- Literals
	align
..192: -- 1 refs
	byte	#01,#80,#01,#c0
	align
..209: -- 1 refs
.write_to_log:
	ajw	-8
	ldl	9
	ldnl	0
	ldnl	modnum
	stl	7
	ldc	..210-2
	ldpi
	stl	6
-- Line 762 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 762 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	10
	ldl	9
	call	.strlen
	stl	4
-- Line 762 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	10
	stl	5
-- Line 765 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnlp	..dataseg+10
	ldl	9
	call	.Wait
-- Line 768 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnl	..dataseg+1
	ldl	4
	gt
	eqc	0
	cj	..194
-- Line 769 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 769 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	10
	ldl	4
	ldl	7
	ldnl	..dataseg+1
	sub
	add
	stl	5
-- Line 770 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnl	..dataseg+1
	stl	4
-- Line 771 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	7
	stnl	..dataseg+2
..194: -- 2 refs
-- Line 779 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	ldl	7
	ldnl	..dataseg+2
	add
	ldl	7
	ldnl	..dataseg+1
	gt
	cj	..196
-- Line 780 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 780 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	stl	1
-- Line 780 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnl	..dataseg+1
	ldc	16
	div
	stl	2
-- Line 780 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnl	..dataseg+2
	stl	3
-- Line 784 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	align
..198: -- 2 refs
	ldl	4
	ldl	7
	ldnl	..dataseg+2
	add
	ldl	7
	ldnl	..dataseg+1
	gt
	cj	..199
-- Line 785 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 785 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	1
	ldl	2
	add
	stl	1
-- Line 786 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnl	..dataseg+2
	ldl	2
	sub
	ldl	7
	stnl	..dataseg+2
	j	..198
	align
..199: -- 1 refs
-- Line 788 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	7
	ldnl	..dataseg+2
	gt
	cj	..201
-- Line 788 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldc	0
	ldl	7
	stnl	..dataseg+2
..201: -- 2 refs
-- Line 789 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldl	1
	gt
	cj	..196
-- Line 790 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	3
	ldl	1
	sub
	stl	0
	ldl	7
	ldnl	..dataseg+0
	ldl	1
	add
	ldl	7
	ldnl	..dataseg+0
	ldl	9
	call	.memcpy
..196: -- 4 refs
-- Line 794 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	4
	stl	0
	ldl	5
	ldl	7
	ldnl	..dataseg+2
	ldl	7
	ldnl	..dataseg+0
	add
	ldl	9
	call	.memcpy
-- Line 795 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnl	..dataseg+2
	ldl	4
	add
	ldl	7
	stnl	..dataseg+2
-- Line 798 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnlp	..dataseg+10
	ldl	9
	call	.Signal
-- Line 801 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnl	..dataseg+6
	cj	..205
-- Line 802 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 802 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnlp	..dataseg+13
	ldl	9
	call	.Wait
-- Line 803 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnl	..dataseg+6
	ldnl	1
	stl	3
	ldl	10
	ldl	7
	ldnl	..dataseg+6
	ldl	9
	call	..211
-- Line 804 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnlp	..dataseg+13
	ldl	9
	call	.Signal
..205: -- 2 refs
-- Line 808 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	7
	ldnl	..dataseg+4
	cj	..207
-- Line 809 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
-- Line 809 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	10
	ldl	9
	call	.strlen
	stl	4
-- Line 810 (/giga/HeliosRoot/Helios/servers/logger/logger.c)
	ldl	6
	ldnl	0
	stl	1
	ldl	4
	stl	0
	ldl	10
	ldl	7
	ldnl	..dataseg+4
	ldl	9
	call	.Write
..207: -- 2 refs
	ajw	8
	ret
-- Literals
	align
..210: -- 1 refs
	byte	#40,#42,#0f,#00
-- Stubs
	align
.InvalidFn:
	ldl	1
	ldnl	0
	ldnl	@_InvalidFn
	ldnl	_InvalidFn
	gcall
	align
.DoLocate:
	ldl	1
	ldnl	0
	ldnl	@_DoLocate
	ldnl	_DoLocate
	gcall
	align
.DoObjInfo:
	ldl	1
	ldnl	0
	ldnl	@_DoObjInfo
	ldnl	_DoObjInfo
	gcall
	align
.NullFn:
	ldl	1
	ldnl	0
	ldnl	@_NullFn
	ldnl	_NullFn
	gcall
	align
.InitMCB:
	ldl	1
	ldnl	0
	ldnl	@_InitMCB
	ldnl	_InitMCB
	gcall
	align
.PutMsg:
	ldl	1
	ldnl	0
	ldnl	@_PutMsg
	ldnl	_PutMsg
	gcall
	align
.InitSemaphore:
	ldl	1
	ldnl	0
	ldnl	@_InitSemaphore
	ldnl	_InitSemaphore
	gcall
	align
.ServMalloc:
	ldl	1
	ldnl	0
	ldnl	@_ServMalloc
	ldnl	_ServMalloc
	gcall
	align
.Terminate:
	ldl	1
	ldnl	0
	ldnl	@_Terminate
	ldnl	_Terminate
	gcall
	align
.InitNode:
	ldl	1
	ldnl	0
	ldnl	@_InitNode
	ldnl	_InitNode
	gcall
	align
.InitList:
	ldl	1
	ldnl	0
	ldnl	@_InitList
	ldnl	_InitList
	gcall
	align
.MachineName:
	ldl	1
	ldnl	0
	ldnl	@_MachineName
	ldnl	_MachineName
	gcall
	align
.Locate:
	ldl	1
	ldnl	0
	ldnl	@_Locate
	ldnl	_Locate
	gcall
	align
.NewPort:
	ldl	1
	ldnl	0
	ldnl	@_NewPort
	ldnl	_NewPort
	gcall
	align
.Create:
	ldl	1
	ldnl	0
	ldnl	@_Create
	ldnl	_Create
	gcall
	align
.Close:
	ldl	1
	ldnl	0
	ldnl	@_Close
	ldnl	_Close
	gcall
	align
.Signal:
	ldl	1
	ldnl	0
	ldnl	@_Signal
	ldnl	_Signal
	gcall
	align
.Dispatch:
	ldl	1
	ldnl	0
	ldnl	@_Dispatch
	ldnl	_Dispatch
	gcall
	align
.Delete:
	ldl	1
	ldnl	0
	ldnl	@_Delete
	ldnl	_Delete
	gcall
	align
.CloseDevice:
	ldl	1
	ldnl	0
	ldnl	@_CloseDevice
	ldnl	_CloseDevice
	gcall
	align
.FreePort:
	ldl	1
	ldnl	0
	ldnl	@_FreePort
	ldnl	_FreePort
	gcall
	align
.Exit:
	ldl	1
	ldnl	0
	ldnl	@_Exit
	ldnl	_Exit
	gcall
	align
.Fork:
	ldl	1
	ldnl	0
	ldnl	@_Fork
	ldnl	_Fork
	gcall
	align
.strcpy:
	ldl	1
	ldnl	0
	ldnl	@_strcpy
	ldnl	_strcpy
	gcall
	align
.GetMsg:
	ldl	1
	ldnl	0
	ldnl	@_GetMsg
	ldnl	_GetMsg
	gcall
	align
.memcpy:
	ldl	1
	ldnl	0
	ldnl	@_memcpy
	ldnl	_memcpy
	gcall
	align
.GetTargetDir:
	ldl	1
	ldnl	0
	ldnl	@_GetTargetDir
	ldnl	_GetTargetDir
	gcall
	align
.ErrorMsg:
	ldl	1
	ldnl	0
	ldnl	@_ErrorMsg
	ldnl	_ErrorMsg
	gcall
	align
.GetTargetObj:
	ldl	1
	ldnl	0
	ldnl	@_GetTargetObj
	ldnl	_GetTargetObj
	gcall
	align
.strlen:
	ldl	1
	ldnl	0
	ldnl	@_strlen
	ldnl	_strlen
	gcall
	align
.NewStream:
	ldl	1
	ldnl	0
	ldnl	@_NewStream
	ldnl	_NewStream
	gcall
	align
.Write:
	ldl	1
	ldnl	0
	ldnl	@_Write
	ldnl	_Write
	gcall
	align
.AbortPort:
	ldl	1
	ldnl	0
	ldnl	@_AbortPort
	ldnl	_AbortPort
	gcall
	align
.CheckMask:
	ldl	1
	ldnl	0
	ldnl	@_CheckMask
	ldnl	_CheckMask
	gcall
	align
.Wait:
	ldl	1
	ldnl	0
	ldnl	@_Wait
	ldnl	_Wait
	gcall
	align
.FormOpenReply:
	ldl	1
	ldnl	0
	ldnl	@_FormOpenReply
	ldnl	_FormOpenReply
	gcall
	align
.Free:
	ldl	1
	ldnl	0
	ldnl	@_Free
	ldnl	_Free
	gcall
	align
.MarshalWord:
	ldl	1
	ldnl	0
	ldnl	@_MarshalWord
	ldnl	_MarshalWord
	gcall
	align
..211:
	ldl	7
	gcall
-- Data Initialization
..212: -- 1 refs

	byte	0,0,0,0,0,40
	data	..dataseg 78
	global	_main
	data	_main 131
..213: -- 1 refs

	byte	27,0,0,0,0,0,0,0,0,208,7,0,0,0,0,0,0,160,15,0,0,0,0,0,0,208,7,0,0,0,0,0,0,208,7,0,0,0,0,0,0,208,7,0,0,0,0,0,0,208,7,0,0,0,0,0,0,208,7,0,0,0,0,0,0,208,7,0,0,0,0,0,0,208,7,0,0,0,0,0,0,208,7,0,0,0,0,0,0,208,7,0,0,0,0,0,0,208,7,0,0,0,0,0,0,208,7,0,0,0,0,0,0,208,7,0,0,0,0,0,0,208,7,0,0,0,0,0,0,208,7
	align
	init
	ajw	-2
	ldl	3
	ldnl	0
	ldnl	modnum
	stl	1
	ldl	1
	ldnlp	..dataseg
	stl	0
	ldl	4
	cj	..215
	ldl	3
	ldnl	@_InvalidFn
	ldnl	_InvalidFn
	ldl	0
	stnl	76
	ldl	3
	ldnl	@_InvalidFn
	ldnl	_InvalidFn
	ldl	0
	stnl	74
	ldl	3
	ldnl	@_InvalidFn
	ldnl	_InvalidFn
	ldl	0
	stnl	72
	ldl	3
	ldnl	@_NullFn
	ldnl	_NullFn
	ldl	0
	stnl	70
	ldl	3
	ldnl	@_InvalidFn
	ldnl	_InvalidFn
	ldl	0
	stnl	68
	ldl	3
	ldnl	@_InvalidFn
	ldnl	_InvalidFn
	ldl	0
	stnl	66
	ldl	3
	ldnl	@_InvalidFn
	ldnl	_InvalidFn
	ldl	0
	stnl	64
	ldl	3
	ldnl	@_InvalidFn
	ldnl	_InvalidFn
	ldl	0
	stnl	62
	ldl	3
	ldnl	@_InvalidFn
	ldnl	_InvalidFn
	ldl	0
	stnl	60
	ldl	3
	ldnl	@_DoObjInfo
	ldnl	_DoObjInfo
	ldl	0
	stnl	54
	ldl	3
	ldnl	@_DoLocate
	ldnl	_DoLocate
	ldl	0
	stnl	52
	ldl	3
	ldnl	@_InvalidFn
	ldnl	_InvalidFn
	ldl	0
	stnl	50
	j	..216
..215: -- 1 refs
	ldc	..213-2
	ldpi
	ldl	0
	adc	179
	ldc	131
	move
	ldc	.main-2
	ldpi
	ldl	0
	stnl	78
	ldc	.Logger_Delete-2
	ldpi
	ldl	0
	stnl	58
	ldc	.Logger_ServerInfo-2
	ldpi
	ldl	0
	stnl	56
	ldc	.Logger_Open-2
	ldpi
	ldl	0
	stnl	48
	ldc	.Logger_Private-2
	ldpi
	ldl	0
	stnl	46
	ldc	..212-2
	ldpi
	ldl	0
	ldc	6
	move
	ldl	0
	ldnlp	16
	ldl	0
	stnl	42
	ldc	..0-2
	ldpi
	ldl	0
	stnl	3
..216: -- 1 refs
	ajw	2
	ret
	data	.MaxData 0
	align
.ModEnd:
