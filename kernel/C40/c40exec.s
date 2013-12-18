/*
 * File:	c40exec.a
 * Subsystem:	C40 Helios executive
 * Author:	P.A.Beskeen
 * Date:	Nov 91
 *
 * Description: `C40 Helios executive support assembler functions.
 *
 *		This source provides general support functions for the
 *		Helios C40 executive.
 *
 * RcsId: Id: c40exec.a,v 1.14 1993/08/10 13:24:41 nickc Exp 
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * RcsLog: Log: c40exec.a,v 
 */
	.GetExecRoot:
	GetExecRoot:
	bud	R11
		ldep		tvtp, R0
	subi	IR0, R0
	lsh	2, R0
	.ResetCPU:
	ResetCPU:
	ldi	0, st	
	ldi	0, iie
	ldi	0, iif
	ldi	0, die
	ldi	0, r0
	ldpe	r0, ivtp
	ldpe	r0, tvtp
	ldhi	0x0010, ar0
	ldi	0, RS	
	sti	RS, *+ar0(0x0020)
	sti	RS, *+ar0(0x0030)
	sti	RS, *+ar0(0x0024)
	sti	RS, *+ar0(0x0034)
	sti	RS, *+ar0(0x0028)
	sti	RS, *+ar0(0x0038)
	sti	RS, *+ar0(0x00a0)
	sti	RS, *+ar0(0x00a3)
	sti	RS, *+ar0(0x00a7)
	sti	RS, *+ar1(0x00a0)
	sti	RS, *+ar1(0x00a3)
	sti	RS, *+ar1(0x00a7)
	sti	RS, *+ar2(0x00a0)
	sti	RS, *+ar2(0x00a3)
	sti	RS, *+ar2(0x00a7)
	sti	RS, *+ar3(0x00a0)
	sti	RS, *+ar3(0x00a3)
	sti	RS, *+ar3(0x00a7)
	sti	RS, *+ar4(0x00a0)
	sti	RS, *+ar4(0x00a3)
	sti	RS, *+ar4(0x00a7)
	sti	RS, *+ar5(0x00a0)
	sti	RS, *+ar5(0x00a3)
	sti	RS, *+ar5(0x00a7)
	sti	RS, *+ar0(0x0040)
	sti	RS, *+ar0(0x0050)
	sti	RS, *+ar0(0x0060)
	sti	RS, *+ar0(0x0070)
	sti	RS, *+ar0(0x0080)
	sti	RS, *+ar0(0x0090)
	ldi	0, ar0
	ldi	*ar0, ar1
	b	ar1	
	.ResetLinkHardware:
	ResetLinkHardware:
	b	R11
	._SetModTab:
	_SetModTab:
	lsh	-2, R0
	addi	IR0, R0
	xor	AR4, R0
	xor	R0, AR4
	bUd	R11	
		xor	AR4, R0
	subi	IR0, R0
	lsh	2, R0
	._GetModTab:
	_GetModTab:
	bud	R11	
		ldi	AR4, R0
	subi	IR0, R0
	lsh	2, R0
	.FastStoreSize:
	FastStoreSize:
	ldi	r0, AR5
	lsh	-2, AR5
	addi	IR0, AR5
	ldhi	0x2f, RS
	or	0xf800, RS	
	sti	RS, *AR5
	ldep	tvtp, AR5
	addi 70, AR5
	ldi	0x400 * 2, R0
	ldi	*+AR5(5), RS
	ldhi	0x30, RE
	cmpi	RS, RE
	beq	R11
	ldi	*+AR5(11), R0
	b	R11
	._ldtimer:
	_ldtimer:
	ldep	tvtp, AR0
	sti	R11, *--AR6
	sti	DP, *--AR6
	LBU3 
			*+AR0(70 + 1),
			R1
	ldhi	(((0x100020) >> 16) & 0xffff), AR5
	or	((0x100020) & 0xffff), AR5
	LDI    *AR5, RS
	LDI   0x80, RE
	ANDN3	RE, RS, RE
	ldi	1000, R0
	STI      RE, *AR5
	patchinstr(PATCHC40MASK24ADD, shift(-2, codestub(.__divide)),
		laj	0)
		mpyi	*+AR5(4), R1
		ldi *+AR0(6), DP
		STI      RS, *AR5
	addi	 DP,    R0
	ldi	*AR6++, DP
	ldi	*AR6++, R11
	bu	R11
	.SetUserStackAndJump:
	SetUserStackAndJump:
	bud	R3
	lsh	-2, R2
	addi	IR0, R2
		ldi	R2, AR6	
	.MP_GetWord:
	MP_GetWord:
	bud	R11
		addi	R0, R1, AR5
		nop
		ldi	*AR5, R0
	.MP_PutWord:
	MP_PutWord:
	bud	R11
		addi	R0, R1, AR5
		nop
		sti	R2, *AR5
	.MP_GetData:
	MP_GetData:
	subi	1, R3, rc		
	rptbd MP_GetDataLoop
		addi	R1, R2, AR0
		lsh	-2, R0, AR1
		addi	IR0, AR1
			ldi	 *AR0++, R2
	MP_GetDataLoop:	sti	R2, *AR1++
	b	R11
	.MP_PutData:
	MP_PutData:
	subi	1, R3, rc		
	rptbd MP_PutDataLoop
		lsh	-2, R2, AR1
		addi	IR0, AR1
		addi	R0, R1, AR0
			ldi	 *AR1++, R2
	MP_PutDataLoop:	sti	R2, *AR0++
	b	R11
	.MP_ReadLock:
	MP_ReadLock:
	bud	R11
		ldi	R0, AR5
		nop	
		ldii	*AR5, R0
	.MP_ReadFPLock:
	MP_ReadFPLock:
	bud	R11
		ldi	R0, AR5
		nop	
		ldfi	*AR5, R0
	.MP_WriteUnlock:
	MP_WriteUnlock:
	bud	R11
		ldi	R0, AR5
		nop	
		stii	R1, *AR5
	.MP_WriteFPUnlock:
	MP_WriteFPUnlock:
	bud	R11
		ldi	R0, AR5
		nop	
		stfi	R1, *AR5
	.MP_Signal:
	MP_Signal:
	ldi	R0, AR5
	bud	R11
		ldii	*AR5, R0
		addi	1, R0
		stii	R0, *AR5
	.MP_BusyWait:
	MP_BusyWait:
	ldi	R0, AR5
	ldi	0, R1
MP_BusyWaitLoop:
	ldii	*AR5, R0
	bzd	MP_BusyWaitLoop
		ldinz	1, R1
		subi	R1, R0
		stii	R0, *AR5
	b	R11
		align
	__procname__linkreg:
		byte	"_linkreg", 0
		align
			word 	0xff000000 | - __procname__linkreg * 4
._linkreg:
	ldi	*+AR6(4), R0
	b	R11
		align
	__procname__fpreg:
		byte	"_fpreg", 0
		align
			word 	0xff000000 | - __procname__fpreg * 4
._fpreg:
	ldi	AR7, R0
	b	R11
		align
	__procname__spreg:
		byte	"_spreg", 0
		align
			word 	0xff000000 | - __procname__spreg * 4
._spreg:
	ldi	AR6, R0
	b	R11
	.JTAGHalt:
	JTAGHalt:
	ldhi	0x2f, R0
delayloop:			/* delay for messages to get out */
	subi	1, R0
	bnz	delayloop
		int	0x66ffffff
	b	R11
