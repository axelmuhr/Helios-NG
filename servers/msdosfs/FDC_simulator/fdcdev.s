	align
	word #60f360f3,.DevOpen byte "DevOpen",0 align
.DevOpen:
	ajw	-184330
-- Line 365 (fdcdev.c)
-- Line 365 (fdcdev.c)
	ldc	812
	ldl	184331
	call	.Malloc
	stl	5
-- Line 373 (fdcdev.c)
	ldl	5
	eqc	0
	eqc	0
	cj	..3
-- Line 375 (fdcdev.c)
	ldl	184332
	ldl	5
	stnl	0
-- Line 376 (fdcdev.c)
	ldc	.DevOperate-2
	ldpi
	ldl	5
	stnl	1
-- Line 377 (fdcdev.c)
	ldc	.DevClose-2
	ldpi
	ldl	5
	stnl	2
-- Line 378 (fdcdev.c)
	ldc	0
	ldl	5
	stnl	16
-- Line 381 (fdcdev.c)
	ldl	184333
	ldnl	2
	ldl	5
	stnl	15
-- Line 382 (fdcdev.c)
	ldl	184333
	ldnl	1
	ldl	5
	stnl	4
-- Line 386 (fdcdev.c)
	ldc	0
	stl	4
	align
..4: -- 2 refs
	ldc	4
	ldl	4
	gt
	cj	..5
-- Line 387 (fdcdev.c)
-- Line 387 (fdcdev.c)
	ldl	5
	adc	416
	ldl	4
	ldc	92
	prod
	add
	stl	3
-- Line 387 (fdcdev.c)
	ldl	3
	stl	2
-- Line 392 (fdcdev.c)
	ldc	0
	stl	1
	align
..7: -- 2 refs
	ldc	17
	ldl	1
	mint
	xor
	rev
	mint
	xor
	rev
	gt
	cj	..8
-- Line 392 (fdcdev.c)
	ldc	0
	stl	0
	ldl	2
	ldl	0
	rev
	stnl	0
	ldl	2
	adc	4
	stl	2
	ldl	1
	adc	1
	stl	1
	j	..7
	align
..8: -- 1 refs
-- Line 394 (fdcdev.c)
	ldc	0
	ldl	3
	stnl	17
-- Line 395 (fdcdev.c)
	ldc	0
	ldl	3
	stnl	18
-- Line 396 (fdcdev.c)
	ldc	0
	ldl	3
	adc	81
	sb
-- Line 397 (fdcdev.c)
	ldc	0
	ldl	3
	stnl	19
-- Line 398 (fdcdev.c)
	ldc	255
	ldl	3
	adc	80
	sb
	ldl	4
	adc	1
	stl	4
	j	..4
	align
..5: -- 1 refs
-- Line 402 (fdcdev.c)
	ldl	184333
	adc	16
	ldl	184333
	ldnl	4
	add
	stl	6
-- Line 403 (fdcdev.c)
	ldc	0
	stl	4
	align
..10: -- 2 refs
	ldc	4
	ldl	4
	gt
	cj	..11
-- Line 404 (fdcdev.c)
-- Line 405 (fdcdev.c)
	ldl	6
	ldl	4
	ldc	28
	prod
	ldl	5
	adc	104
	add
	ldc	28
	move
-- Line 406 (fdcdev.c)
	ldc	0
	ldl	4
	ldc	92
	prod
	ldl	5
	ldnlp	104
	add
	adc	80
	sb
-- Line 408 (fdcdev.c)
	ldl	6
	ldnl	2
	ldc	2
	and
	cj	..13
-- Line 409 (fdcdev.c)
	ldc	1
	ldl	4
	ldc	92
	prod
	ldl	5
	ldnlp	104
	add
	stnl	19
	align
..13: -- 2 refs
-- Line 410 (fdcdev.c)
	ldl	6
	ldnl	0
	eqc	-1
	eqc	0
	cj	..11
-- Line 411 (fdcdev.c)
	ldl	6
	ldl	6
	ldnl	0
	add
	stl	6
	ldl	4
	adc	1
	stl	4
	j	..10
	align
..11: -- 3 refs
-- Line 413 (fdcdev.c)
	ldl	4
	adc	1
	ldl	5
	stnl	24
-- Line 415 (fdcdev.c)
	ldl	184333
	adc	20
	ldl	184333
	ldnl	5
	add
	stl	8
-- Line 416 (fdcdev.c)
	ldc	0
	stl	7
	align
..17: -- 2 refs
	ldc	10
	ldl	7
	gt
	cj	..18
-- Line 417 (fdcdev.c)
-- Line 418 (fdcdev.c)
	ldl	8
	ldl	7
	ldc	20
	prod
	ldl	5
	adc	216
	add
	ldc	20
	move
-- Line 419 (fdcdev.c)
	ldl	8
	ldnl	0
	eqc	-1
	eqc	0
	cj	..18
-- Line 420 (fdcdev.c)
	ldl	8
	ldl	8
	ldnl	0
	add
	stl	8
	ldl	7
	adc	1
	stl	7
	j	..17
	align
..18: -- 3 refs
-- Line 422 (fdcdev.c)
	ldl	7
	adc	1
	ldl	5
	stnl	25
-- Line 429 (fdcdev.c)
	ldl	5
	ajw	184330
	ret
..3: -- 2 refs
-- Line 432 (fdcdev.c)
	ldl	5
	ldl	184331
	call	.Free
-- Line 433 (fdcdev.c)
	ldc	0
	ajw	184330
	ret
	align
	word #60f360f3,.DevClose byte "DevClose",0 align
.DevClose:
-- Line 440 (fdcdev.c)
-- Line 441 (fdcdev.c)
	ldc	1
	ldl	2
	stnl	16
-- Line 448 (fdcdev.c)
	ldl	2
	ldl	1
	call	.Free
-- Line 449 (fdcdev.c)
	ldc	0
	ret
	align
	word #60f360f3,.DevOperate byte "DevOperate",0 align
.DevOperate:
	ajw	-8
	ldc	..39-2
	ldpi
	stl	7
-- Line 456 (fdcdev.c)
-- Line 456 (fdcdev.c)
	ldc	0
	stl	1
-- Line 456 (fdcdev.c)
	ldc	3
	ldl	7
	ldl	9
	call	.open
	stl	0
	ldl	0
	ldl	9
	call	.fdstream
	stl	2
-- Line 469 (fdcdev.c)
	ldc	0
	ldl	11
	stnl	10
-- Line 475 (fdcdev.c)
	ldl	11
	ldnl	5
	stl	3
-- Line 476 (fdcdev.c)
	ldc	0
	ldl	3
	gt
	eqc	0
	cj	..25
	ldl	10
	ldnl	25
	ldl	3
	gt
	eqc	0
	cj	..24
..25: -- 3 refs
-- Line 477 (fdcdev.c)
-- Line 478 (fdcdev.c)
	ldl	7
	ldnl	4
	stl	1
-- Line 479 (fdcdev.c)
	j	..28
..24: -- 1 refs
-- Line 482 (fdcdev.c)
	ldl	10
	adc	216
	ldl	3
	ldc	20
	prod
	add
	stl	5
-- Line 483 (fdcdev.c)
	ldl	5
	ldnl	1
	stl	4
-- Line 484 (fdcdev.c)
	ldl	10
	adc	104
	ldl	4
	ldc	28
	prod
	add
	stl	6
-- Line 488 (fdcdev.c)
	ldl	4
	ldl	10
	ldl	9
	call	.RecordCommandTime
-- Line 492 (fdcdev.c)
	ldl	11
	ldnl	2
	ldl	7
	ldnl	5
	and
	stl	0
	ldl	0
	adc	-4112
	cj	..33
	ldl	0
	adc	-4128
	cj	..32
	ldl	0
	adc	-40976
	cj	..31
	j	..30
..33: -- 1 refs
-- Line 495 (fdcdev.c)
	ldl	2
	ldl	11
	ldl	9
	call	.ReadData
	stl	1
-- Line 496 (fdcdev.c)
	j	..28
..32: -- 1 refs
-- Line 499 (fdcdev.c)
	ldl	2
	ldl	11
	ldl	9
	call	.WriteData
	stl	1
-- Line 500 (fdcdev.c)
	j	..28
..31: -- 1 refs
-- Line 503 (fdcdev.c)
	ldl	2
	ldl	11
	ldl	9
	call	.FormatDisc
	stl	1
-- Line 504 (fdcdev.c)
	j	..28
..30: -- 2 refs
-- Line 508 (fdcdev.c)
	ldl	11
	ldnl	2
	stl	0
-- Line 511 (fdcdev.c)
	ldl	7
	ldnl	6
	stl	1
-- Line 512 (fdcdev.c)
..28: -- 7 refs
-- Line 518 (fdcdev.c)
	ldl	1
	ldl	11
	stnl	3
-- Line 520 (fdcdev.c)
	ldl	2
	ldl	9
	call	.close
-- Line 522 (fdcdev.c)
	ldl	11
	ldnl	4
	stl	0
	ldl	11
	ldl	9
	call	..40
	ajw	8
	ret
-- Literals
	align
..39: -- 1 refs
	byte	"/helios/tmp/dos"
	byte	0
	align
	byte	#05,#00,#ff,#d3
	byte	#f0,#ff,#0f,#00
	byte	#00,#00,#07,#d3
	align
	word #60f360f3,.ReadData byte "ReadData",0 align
.ReadData:
	ajw	-2
-- Line 534 (fdcdev.c)
-- Line 535 (fdcdev.c)
	ldl	4
	ldnl	7
	ldc	512
	prod
	stl	0
	ldc	0
	ldl	5
	ldl	3
	call	.Seek
-- Line 536 (fdcdev.c)
	ldc	-1
	stl	1
	ldl	4
	ldnl	8
	ldc	512
	prod
	stl	0
	ldl	4
	ldnl	9
	ldl	5
	ldl	3
	call	.Read
	ldl	4
	stnl	10
-- Line 537 (fdcdev.c)
	ldl	4
	ldnl	10
	ldl	4
	ldnl	8
	ldc	512
	prod
	diff
	eqc	0
	cj	..43
	ldc	0
	ldc	0
	cj	..42
..43: -- 1 refs
	ldc	-1
	ldc	0
..42: -- 2 refs
	diff
	ldl	4
	stnl	3
-- Line 538 (fdcdev.c)
	ldc	0
	ajw	2
	ret
	align
	word #60f360f3,.WriteData byte "WriteData",0 align
.WriteData:
	ajw	-2
-- Line 550 (fdcdev.c)
-- Line 551 (fdcdev.c)
	ldl	4
	ldnl	7
	ldc	512
	prod
	stl	0
	ldc	0
	ldl	5
	ldl	3
	call	.Seek
-- Line 552 (fdcdev.c)
	ldc	-1
	stl	1
	ldl	4
	ldnl	8
	ldc	512
	prod
	stl	0
	ldl	4
	ldnl	9
	ldl	5
	ldl	3
	call	.Write
	ldl	4
	stnl	10
-- Line 553 (fdcdev.c)
	ldl	4
	ldnl	10
	ldl	4
	ldnl	8
	ldc	512
	prod
	diff
	eqc	0
	cj	..47
	ldc	0
	ldc	0
	cj	..46
..47: -- 1 refs
	ldc	-1
	ldc	0
..46: -- 2 refs
	diff
	ldl	4
	stnl	3
-- Line 554 (fdcdev.c)
	ldc	0
	ajw	2
	ret
	align
	word #60f360f3,.RecordCommandTime byte "RecordCommandTime",0 align
.RecordCommandTime:
-- Line 564 (fdcdev.c)
-- Line 565 (fdcdev.c)
	ldl	1
	call	._cputime
	ldl	3
	ldc	92
	prod
	ldl	2
	ldnlp	104
	add
	stnl	17
	ret
	align
	word #60f360f3,.FormatDisc byte "FormatDisc",0 align
.FormatDisc:
	ajw	-131
-- Line 570 (fdcdev.c)
-- Line 574 (fdcdev.c)
	ldc	0
	ldl	133
	stnl	10
-- Line 576 (fdcdev.c)
	ldc	0
	stl	2
	align
..51: -- 2 refs
	ldc	512
	ldl	2
	gt
	cj	..52
-- Line 577 (fdcdev.c)
	ldc	229
	ldl	2
	ldlp	3
	bsub
	sb
	ldl	2
	adc	1
	stl	2
	j	..51
	align
..52: -- 1 refs
-- Line 579 (fdcdev.c)
	ldc	0
	stl	0
	ldc	0
	ldl	134
	ldl	132
	call	.Seek
-- Line 581 (fdcdev.c)
	ldc	0
	stl	2
	align
..54: -- 2 refs
	ldc	1440
	ldl	2
	gt
	cj	..55
-- Line 582 (fdcdev.c)
	ldc	-1
	stl	1
	ldc	512
	stl	0
	ldlp	3
	ldl	134
	ldl	132
	call	.Write
	ldl	133
	ldnl	10
	add
	ldl	133
	stnl	10
	ldl	2
	adc	1
	stl	2
	j	..54
	align
..55: -- 1 refs
-- Line 583 (fdcdev.c)
	ldc	0
	ldl	133
	stnl	3
-- Line 584 (fdcdev.c)
	ldc	0
	ajw	131
	ret
-- Stubs
	align
..40:
	ldl	4
	gcall
