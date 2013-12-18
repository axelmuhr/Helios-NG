/*
 * File:	c40intr.a
 * Subsystem:	C40 executive
 * Author:	P.A.Beskeen
 * Date:	Nov 91
 *
 * Description: `C40 Helios executive interrupt related functions.
 *
 *
 * RcsId: Id: c40intr.a,v 1.21 1993/10/04 12:12:37 paul Exp 
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * RcsLog: Log: c40intr.a,v 
 */
	.IntsAreEnabled:
	IntsAreEnabled:
	bud	R11
		ldi	ST, R0
		and	(1 << 13), R0
		lsh	-13, R0		
	.SetIIFBits:
	SetIIFBits:
	bud	R11
		or	R0,IIF
		nop
		nop
	.ClrIIFBits:
	ClrIIFBits:
	bud	R11
		andn	R0,IIF
		nop
		nop
	.WriteTCR:
	WriteTCR:
	ldi	AR0,r10		
	ldhi	0x10,AR0		
	ldi	*AR0,r1		
	ldhi	0x1dea,r2		
	or	0x0050,r2
	sti	r2,*AR0		
	ldhi	0x8000,AR5		
	ldi	iif,r2			
	ldi	2,iif			
	ldi	*AR5,r3		
	sti	r0,*AR5		
	nop				
	nop
	nop
	nop
	ldi	r2,iif			
	sti	r1,*AR0		
	ldi	r10,AR0		
	ldi	r3,r0			
	and	0xFF,r0			
	bu	R11			
	.StartTimeSlicer:
	StartTimeSlicer:
	ldi	R11, AR5			
	laj	4
		nop				
		patchinstr(PATCHC40MASK16ADD,
			shift(-2, labelref(SliceIntrHandler)),
			addi	-2, R11)	
		ldi	R11, RS
	ldi	AR5, R11			
	ldep	ivtp, AR5
	sti	RS, *+AR5(2)	
	ldep	tvtp, AR5
	ldi	*+AR5(70 + 13), RS
	ldhi	0x0010, AR5
	or	0x0020, AR5
	sti	RS, *+AR5(0x8)		
	stik	0, *+AR5(0x4)		
	bUd	R11	
		ldi	((1 << 1)|(1 << 6)|(1 << 7)|(1 << 9)), RS
		sti	RS, *+AR5(0x0)	
		or	1, iie
	export	extern_slice_now
SliceIntrHandler:
	push	ST
	push	IR0
	push	ar0	
	push	ar1
	push	ar2
	ldep	tvtp, ar0
	or	(1 << 15), st		
        ldi     *+ar0(6), ar2
        addi    1000, ar2
        sti     ar2, *+ar0(6)
	ldi	*+ar0(28), IR0
        ldi     *+ar0(10), ar1
        bZ      no_wakeups 	        
	lsh	-2, ar1		 	
	addi	IR0, ar1
        cmpi    *+ar1(3), ar2
        bGE	do_wakeups
no_wakeups:
	ldi	*+ar0(1), ar1
	bZ	IntrRet
	cmpi	*+ar0(2), ar1
	bGT	slice_now
	cmpi	0, *+ar0(9)
	beq	IntrRet
	subi	1, *+ar0(7), ar2	
	sti	ar2, *+ar0(7)
	bLE	check_slice	
IntrRet:
	pop	ar2
	pop	ar1
	pop	ar0
	pop	IR0
	pop	st
	retiU		
do_wakeups:
	push	ar3	
	push	ar4
	push	ar5
	ldi     *+ar1(0), ar4	
	ldi	*+ar1(2), ar3	
	sti     ar4, *+ar0(10)	
	cmpi	*+ar0(2), ar3
	bGEd	not_higherpri
	        addi    12, ar0, ar5
		addi	ar3, ar5
		addi	ar3, ar5	
	sti	ar3, *+ar0(2)
not_higherpri:
	subi	IR0, ar1, ar4
	lsh	2, ar4
	lsh	-2, *+ar5(0), ar3
	addi	IR0, ar3
	cmpi	8, *+ar1(4)
	bEQd	skip_run_status
		sti	ar4, *+ar3(0)	
		sti	ar4, *ar5			
		stik	0, *+ar1(0)	
	stik	2, *+ar1(4)
skip_run_status:
no_more_wakeups:
	bud	no_wakeups
		pop	ar5
		pop	ar4
		pop	ar3
check_slice:
        addi    11, ar0, ar2	
	addi	ar1, ar2
	addi	ar1, ar2			
	ldi	*ar2++, ar1			
	cmpi	0, ar1			
	bNE	do_slice
	bud	IntrRet
		ldi	*+ar0(8), ar1
		nop
		sti	ar1, *+ar0(7) 	
extern_slice_now:
	pop	ar3
	ldep	tvtp, ar0
slice_now:
        addi    12, ar0, ar2	
	addi	ar1, ar2
	addi	ar1, ar2			
do_slice:
	push	ar3	
	push	ar4
	lsh	-2, *+ar0(0), ar1
	addi	IR0, ar1
	subi	IR0, ar1, ar4
	lsh	2, ar4
	lsh	-2, *+ar2(0), ar3
	addi	IR0, ar3
	sti	ar4, *+ar3(0)	
	sti	ar4, *ar2			
	stik	0, *+ar1(0)	
	subi	*+ar1(8), *+ar0(6), ar3
	addi	*+ar1(7), ar3
	sti	ar3, *+ar1(7)
	stik	1, *+ar1(4)
	addi	11 + 0, ar1, ar4
	subi	5 + 2, SP, ar3
	ldi	*ar3++, ar2
	sti	ar2, *ar4++		
	ldi	*ar3++, ar2
	sti	ar2, *ar4++		
	ldi	*ar3++, ar2		
	addi	11 + 10 - 1, ar1, SP
	push	r0	pushf	r0
	push	r1	pushf	r1
	push	r2	pushf	r2
	push	r3	pushf	r3
	push	r4	pushf	r4
	push	r5	pushf	r5
	push	r6	pushf	r6
	push	r7	pushf	r7
	push	r8	pushf	r8
	push	r9	pushf	r9
	push	r10	pushf	r10
	push	r11	pushf	r11
	push	dp
	push	ar2		
	push	ir1
	push	bk
	push	rs
	push	re
	push	rc
	subi	1 + 3, ar3, SP
	ldi	*ar3++, r0		
	rpts	3			
		ldi	*ar3++, r0 ||	
		sti	r0, *ar4++	
	sti	r0, *ar4++		
	sti	ar5, *ar4++		
	sti	ar6, *ar4++		
	sti	ar7, *ar4++		
	patchinstr(PATCHC40MASK24ADD, shift(-2, labelref(.Dispatch)),
		bud	0)
		ldi	30 + 40 - 1, AR6
		addi	ar0, AR6
		ldi	0, R0	
	.IdleUntilInterrupt:
	IdleUntilInterrupt:
	idle			
	andn	(1 << 13), ST
	b	R11
	.InitEventHandler:
	InitEventHandler:
	ldi	R11, AR5			
	laj	4
		nop				
		patchinstr(PATCHC40MASK16ADD,
			shift(-2, labelref(IntrHandler0)),
			addi	-2, R11)	
		ldi	R11, R0
	ldi	AR5, R11			
        ldep    ivtp, AR0
        sti     R0, *+AR0(1)		
	addi	5, R0				
        sti     R0, *+AR0(3)	
	addi	5, R0				
        sti     R0, *+AR0(4)	
	addi	5, R0				
        sti     R0, *+AR0(5)	
	addi	5, R0				
        sti     R0, *+AR0(6)	
	addi	5, R0				
        sti     R0, *+AR0(0x2b)	
	b	R11
IntrHandler0:
        push    ST                     
        bud     GenHandler
		push	IR0
		push	AR0		
		ldi	0, AR0
IntrHandler1:
        push    ST                     
        bud     GenHandler
		push	IR0
		push	AR0		
		ldi	1, AR0
IntrHandler2:
        push    ST                     
        bud     GenHandler
		push	IR0
		push	AR0		
		ldi	2, AR0
IntrHandler3:
        push    ST                     
        bud     GenHandler
		push	IR0
		push	AR0		
		ldi	3, AR0
IntrHandler4:
        push    ST                     
        bud     GenHandler
		push	IR0
		push	AR0		
		ldi	4, AR0
IntrHandler5:
        push    ST                     
        bud     GenHandler
		push	IR0
		push	AR0		
		ldi	5, AR0
GenHandler:
	push	AR1			
	push	AR2			
	push	AR3
	push	R0
	push	R1
	push	R2
	push	R3
	push	R6		
	push	R7
	push	R10
	push	R11
	push	RS
	push	RE
	push	RC
	push	AR4
	push	AR5
	push	IR1
	push	AR6
	push	AR7
	ldi	AR0, R0		
	or	(1 << 15), st		
	ldep	tvtp, AR5
	ldi	*+AR5(28), IR0
	addi	472 / 4, AR5, AR6
	addi	0x200-1, AR6
	patchinstr(PATCHC40MASK24ADD, shift(-2, labelref(.RootEventHandler)),
		laj	0)
		andn	0x200-1, AR6
		ldi	0, IR1
		ldi	0, AR7
GenIntrReturn:
	pop	AR7
	pop	AR6
	pop	IR1
	pop	AR5
	pop	AR4
	pop	RC
	pop	RE
	pop	RS
	pop	R11
	pop	R10
	pop	R7
	pop	R6		
	pop	R3
	pop	R2
	pop	R1
        pop     R0
	pop	AR3
	ldep	tvtp, ar0
	ldi	*+ar0(1), ar1
	cmpi	*+ar0(2), ar1
	bgt	slice_now
	pop	AR2			
	pop	AR1			
	pop	AR0			
	pop	IR0			
        pop     ST
        retiU                           
	.DefineExecErrorHandler:
	DefineExecErrorHandler:
	b	R11
