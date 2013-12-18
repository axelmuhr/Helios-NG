/*
 * File:	c40slice.a
 * Subsystem:	C40 Helios executive
 * Author:	P.A.Beskeen
 * Date:	Aug 91
 *
 * Description: `C40 Helios executive support assembler functions.
 *
 *		This file is used in conjunction with gslice.c to provide the
 *		timeslicer portion of the Helios C40 executive.
 *
 * RcsId: Id: c40slice.a,v 1.7 1992/11/12 20:54:42 paul Exp 
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * RcsLog: Log: c40slice.a,v 
 */
._SaveCPUState:
	ldi	SP, AR5
	lsh	-2, R0
	addi	IR0, R0
	ldi	R0, SP	
	push	st		
	ldi	1, R0
	push	ar0	push	ar1
	push	ar2	push	ar3
	push	ar4	push	ar5
	push	ar6	push	ar7
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
	push	ir0
	push	ir1
	push	bk
	push	rs
	push	re
	bUd	R11			
		push	rc		
		ldi	AR5, SP
		ldi	0, R0	
	.SaveCPUState:
	SaveCPUState:
	andn	(1 << 13), ST
	ldi	SP, AR5
	lsh	-2, R0
	addi	IR0, R0
	ldi	R0, SP	
	push	st		
	ldi	1, R0
	push	ar0	push	ar1
	push	ar2	push	ar3
	push	ar4	push	ar5
	push	ar6	push	ar7
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
	push	ir0
	push	ir1
	push	bk
	push	rs
	push	re
	push	rc		
	bUd	R11			
		ldi	AR5, SP
	or	(1 << 13), ST
		ldi	0, R0
	.RestoreCPUState:
	RestoreCPUState:
	lsh	-2, R0
	addi	IR0, R0
	andn	(1 << 13), ST
	        nop
        	nop
	        nop
        ldi     SP, AR5
        addi    40, R0, SP
        pop     rc
        pop     re
        pop     rs
        pop     bk
        pop     ir1
        pop     ir0
        pop     dp
        popf    r11     pop     r11
        popf    r10     pop     r10
        popf    r9      pop     r9
        popf    r8      pop     r8
        popf    r7      pop     r7
        popf    r6      pop     r6
        popf    r5      pop     r5
        popf    r4      pop     r4
        popf    r3      pop     r3
        popf    r2      pop     r2
        popf    r1      pop     r1
        popf    r0      pop     r0
        pop     ar7     pop     ar6
        pop     ar4     pop     ar4     
        pop     ar3     pop     ar2     
        pop     ar1     pop     ar0     
        bUd     R11            
                pop     st              
                ldi     AR5, SP   
	or	(1 << 13), ST
.RestoreSlicedState:
	lsh	-2, R0
	addi	IR0, R0
, ar0       
	andn	(1 << 13), ST
		nop
		nop
		nop
	ldi	*+ar0(0), ar1		
	push	ar1
	ldi	*+ar0(1), ar1		
	push	ar1
	ldi	*+ar0(2), ar1	
	push	ar1
	ldi	*+ar0(3), ar1	
	push	ar1
	ldi	SP, ar1
	addi	40, ar0, SP
	pop	rc
	pop	re
	pop	rs
	pop	bk
	pop	ir1
	pop	ir0
	pop	dp
	popf	r11	pop	r11
	popf	r10	pop	r10
	popf	r9	pop	r9
	popf	r8	pop	r8
	popf	r7	pop	r7
	popf	r6	pop	r6
	popf	r5	pop	r5
	popf	r4	pop	r4
	popf	r3	pop	r3
	popf	r2	pop	r2
	popf	r1	pop	r1
	popf	r0	pop	r0
	pop	ar7	pop	ar6
	pop	ar5	pop	ar4
	pop	ar3	pop	ar2
	ldi	ar1, SP	
	pop	ar1
	pop	ar0
	pop	st
	retiU			
