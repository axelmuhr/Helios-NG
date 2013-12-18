._LinkTx:
        addi    2, R1, AR5         
	ldi	R2, AR0  		 
        subi    2, R0                 
        bn      OneWordTx
        ldi     *AR0++, R1
        rpts R0
                sti     R1, *AR5 || ldi *AR0++, R1
        sti     R1, *AR5
        bu      R11
OneWordTx:
        bud     R11
                ldi     *AR0++, R1
                sti     R1, *AR5
                nop
._LinkRx:
        addi    1, R1, AR5         
	ldi	R2, AR0		
        subi    2, R0                 
        bn      OneWordRx
        ldi     *AR5, R1
        rpts R0
                sti     R1, *AR0++ || ldi *AR5, R1
        sti     R1, *AR0++
        bu      R11
OneWordRx:
        bud     R11
                ldi     *AR5, R1
                sti     R1, *AR0++
                nop
.RxFIFOSpace:
        ldi     R0, AR5
        lsh     -7, *+AR5(0), R1           
        bud     R11
                and     0b011100, R1, R0    
                tstb    0b100000, R1          
                ldinz   32, R0                
.TxFIFOSpace:
        ldi     R0, AR5
        lsh     -3, *+AR5(0), R1
        and     0b011100, R1, R0    
        bud     R11
                subri   32, R0                
                tstb    0b100000, R1          
                ldinz   0, R0                 
.InitLinkDMA:
        ldi     R0, AR0
        stik    0, *+AR0(0)
        stik    0, *+AR0(3)
        stik    0, *+AR0(7)
        stik    1, *+AR0(2)
        stik    1, *+AR0(5)
        ldep    ivtp, R2
        lsh     -4, AR0, AR1
        and     0xf, AR1
        subi    0xa, AR1            
        addi    R2, AR1, AR2  
	ldi	R11, AR5			
	laj	4
		nop				
		patchinstr(PATCHC40MASK16ADD,
			shift(-2, labelref(LinkDMAIntrHandler0)),
			addi	-2, R11)	
		ldi	R11, R3
	ldi	AR5, R11			
        mpyi    6, AR1, R2                
        addi    R2, R3                      
        sti     R3, *+AR2(0x25)
        cmpi    1, AR1
        bhi     hi_die
        bz      chan_zero
        bud     end_die_mask
                andn    0b11110000, die 
                or      0b01010000, die
                nop
chan_zero:
        bud     end_die_mask
                andn    0b1111, die     
                or      0b0101, die
                nop
hi_die: 
        ldi     0b0011111100000000, R2
        subi    2, AR1, R3        
        mpyi    6, R3                 
        lsh     R3, R2              
        andn    R2, die               
        ldi     0b0000100100000000, R2
        lsh     R3, R2
        or      R2, die               
end_die_mask:
        bud     R11
                ldhi    0b0000001000000000, R2
                lsh     AR1, R2   
                or      R2, iie       
._AbortLinkTx:
	lsh	-2, R0, AR0
	addi	IR0, AR0
        lsh     15, R1, R3
        ldi     *+AR0(20), AR1
		cmpi	0, *+AR1(3)
		beq	TxHasCompletedOK
       	cmpi    0, *+AR0(19)
        beq     resetboth
        ldhi    ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b00 & 0b11) << 22) |
                                ((0b01 & 0b11) << 24)) >> 16), R0
        or      ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b00 & 0b11) << 22) |
                                ((0b01 & 0b11) << 24)) & 0xffff), R0
        sti     R0, *+AR1(0)
		nop     
		nop     
		nop     
		nop     
		nop
		nop
		nop
		nop
		nop
HaltedRx:
        ldi     *+AR1(7), R1
        bz      R11            
        ldhi    ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b00 & 0b11) << 22) |
                                ((0b11 & 0b11) << 24)) >> 16), R0
        bud     R11
                or      ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b00 & 0b11) << 22) |
                                ((0b11 & 0b11) << 24)) & 0xffff), R0
                or      R3, R0
                sti     R0, *+AR1(0)
TxHasCompletedOK:
	ldep	tvtp, AR1
        stik	0, *+AR1(2)
        ldi     *+AR0(18), AR5 
        stik    0, *+AR0(18)      
	lsh3	-2,  AR5, AR0		 
	addi	IR0, AR0
        stik    0, *+AR0(0)      
	stik	2, *+AR0(4)
        lsh	-2, *+AR1(12), AR0
	addi	IR0, AR0
	bud	R11
	        sti     AR5, *+AR1(12)
	        sti     AR5, *+AR0(0)
		ldi	0, R0
resetboth:
        bud     R11
                ldhi    ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b00 & 0b11) << 22) |
                                ((0b00 & 0b11) << 24)) >> 16), R0
                or      ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b00 & 0b11) << 22) |
                                ((0b00 & 0b11) << 24)) & 0xffff), R0
                sti     R0, *+AR1(0)
._AbortLinkRx:
	lsh	-2, R0, AR0
	addi	IR0, AR0
        lsh     15, R1,  R3
        ldi     *+AR0(20), AR1
        cmpi    0, *+AR0(18)
        beq     resetboth
        ldhi    ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b01 & 0b11) << 22) |
                                ((0b00 & 0b11) << 24)) >> 16), R0
        or      ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b01 & 0b11) << 22) |
                                ((0b00 & 0b11) << 24)) & 0xffff), R0
        sti     R0, *+AR1(0)
		nop     
		nop     
		nop     
		nop     
		nop
		nop
		nop
		nop
		nop
HaltedTx:
        ldi     *+AR1(3), R1
        bz      R11            
        ldhi    ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b11 & 0b11) << 22) |
                                ((0b00 & 0b11) << 24)) >> 16), R0
        bud     R11
                or      ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b11 & 0b11) << 22) |
                                ((0b00 & 0b11) << 24)) & 0xffff), R0
                or      R3, R0
                sti     R0, *+AR1(0)
.__LinkTx:
	lsh	-2, R0, AR0
	addi	IR0, AR0
        ldi     *+AR0(20), AR1
        lsh     15, R1
        sti     R3, *+AR1(1)
        cmpi    0, *+AR1(7)      
        bne     CarefulTxStart                  
        stik    0, *+AR1(0)
SimpleTxStart:
        ldhi    ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b11 & 0b11) << 22) |
                                ((0b00 & 0b11) << 24)) >> 16), R0
        or      ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b11 & 0b11) << 22) |
                                ((0b00 & 0b11) << 24)) & 0xffff), R0
        sti     R2, *+AR1(3)
        bud     R11
                or      R1, R0
	andn	(1 << 13), ST
                sti     R0, *+AR1(0)
CarefulTxStart:
        ldhi    ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b00 & 0b11) << 22) |
                                ((0b01 & 0b11) << 24)) >> 16), R0
        or      ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b00 & 0b11) << 22) |
                                ((0b01 & 0b11) << 24)) & 0xffff), R0
        sti     R0, *+AR1(0)
        sti     R0, *+AR1(0)
                nop     
                nop     
                nop     
                nop     
                nop	
                nop	
                nop
                nop
                nop
haltedRx:
        sti     R2, *+AR1(3)
        cmpi    0, *+AR1(7)
        beq     SimpleTxStart   
        ldhi    ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b11 & 0b11) << 22) |
                                ((0b11 & 0b11) << 24)) >> 16), R0
	or      ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b11 & 0b11) << 22) |
                                ((0b11 & 0b11) << 24)) & 0xffff), R0
        bud     R11
                or      R1, R0
	andn	(1 << 13), ST
                sti     R0, *+AR1(0)
.__LinkRx:
	lsh	-2, R0, AR0
	addi	IR0, AR0
        ldi     *+AR0(20), AR1
        lsh     15, R1
        sti     R3, *+AR1(4)
        cmpi    0, *+AR1(3)         
        bne     CarefulRxStart                  
        stik    0, *+AR1(0)
SimpleRxStart:
        ldhi    ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b00 & 0b11) << 22) |
                                ((0b11 & 0b11) << 24)) >> 16), R0
	or      ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b00 & 0b11) << 22) |
                                ((0b11 & 0b11) << 24)) & 0xffff), R0
        sti     R2, *+AR1(7)
        bud     R11
                or      R1, R0
	andn	(1 << 13), ST
                sti     R0, *+AR1(0)
CarefulRxStart:
        ldhi    ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b01 & 0b11) << 22) |
                                ((0b00 & 0b11) << 24)) >> 16), R0
        or      ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b01 & 0b11) << 22) |
                                ((0b00 & 0b11) << 24)) & 0xffff), R0
        sti     R0, *+AR1(0)
                nop     
                nop     
                nop     
                nop     
                nop
                nop
                nop
                nop
                nop
haltedTx:
        sti     R2, *+AR1(7)
        cmpi    0, *+AR1(3)
        beq     SimpleRxStart   
        ldhi    ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b11 & 0b11) << 22) |
                                ((0b11 & 0b11) << 24)) >> 16), R0
	or      ((((0b01 & 0b11) | (0b11 << 6) |
                                (1 << 14) |
                                ((0b01 & 0b11) << 2) | (1 << 18) |
                                ((0b01 & 0b11) << 4) | (1 << 19)) | ((0b11 & 0b11) << 22) |
                                ((0b11 & 0b11) << 24)) & 0xffff), R0
        bud     R11
                or      R1, R0
	andn	(1 << 13), ST
                sti     R0, *+AR1(0)
	import	extern_slice_now
LinkDMAIntrHandler0:
        push    ST                              
	push	IR0				
        bud     GenEDMAHandler
                push    ar0                     
                ldhi    0x10, ar0               
                or      0x00a0, ar0       
LinkDMAIntrHandler1:
        push    ST
	push	IR0
        bud     GenEDMAHandler
                push    ar0
                ldhi    0x10, ar0
                or      0x00b0, ar0
LinkDMAIntrHandler2:
        push    ST
	push	IR0
        bud     GenEDMAHandler
                push    ar0
                ldhi    0x10, ar0
                or      0x00c0, ar0
LinkDMAIntrHandler3:
        push    ST
	push	IR0
        bud     GenEDMAHandler
                push    ar0
                ldhi    0x10, ar0
                or      0x00d0,ar0
LinkDMAIntrHandler4:
        push    ST
	push	IR0
        bud     GenEDMAHandler
                push    ar0
                ldhi    0x10, ar0
                or      0x00e0, ar0
LinkDMAIntrHandler5:
        push    ST
	push	IR0
        push    ar0
        ldhi    0x10, ar0
        or      0x00f0, ar0
GenEDMAHandler:
        push    ar1                     
        push    ar2
	push	ar3
        or      (1 << 15), st         
        ldi     *ar0, ar1               
        lsh     -15, ar1, ar0           
        lsh     -20, ar1                
	ldep	tvtp, ar3
        and     0b11, ar1               
        bz      interrupt_return        
	ldi	*+ar3(28), IR0
        stik	0, *+ar3(2)
	lsh	-2, *+ar3(3), ar2
	addi	IR0, ar2
        cmpi    0b11, ar1
        beq     TxANDRxInterrupt
	lsh	-2, *+ar2(4), ar2	
	addi	IR0, ar2
        and     0b111, ar0              	
        tstb    0b10, ar1                       
        bnzd    RxInterrupt
                addi    ar0, ar2                
		lsh	-2, *+ar2(0), ar0	
		addi	IR0, ar0
TxInterrupt:
	cmpi	0, *+ar0(19)
	beq	NoLostRxInterrupt
	ldi	*+ar0(20), ar2
	cmpi	0, *+ar2(7)
	beq	TxAndRxInterrupt2
NoLostRxInterrupt:
        ldi     *+ar0(18), ar2       
        stik    0, *+ar0(18)         
	lsh	-2,  ar2, ar0			
        bZ      interrupt_return                
	addi	IR0, ar0
        stik    0, *+ar0(0)        
	stik	2, *+ar0(4)
        lsh	-2, *+ar3(12), ar0
	addi	IR0, ar0
        sti     ar2, *+ar3(12)
        sti     ar2, *+ar0(0)
	ldi	*+ar3(1), ar1
	bnz	extern_slice_now
interrupt_return:
	pop	ar3                     
        pop     ar2
        pop     ar1
        pop     ar0
	pop	IR0
        pop     st
        retiU                           
RxInterrupt:
	cmpi	0, *+ar0(18)
	beq	NoLostTxInterrupt
	ldi	*+ar0(20), ar2
	cmpi	0, *+ar2(3)
	beq	TxAndRxInterrupt2
NoLostTxInterrupt:
        ldi     *+ar0(19), ar2       
        stik    0, *+ar0(19)         
        lsh	-2, ar2, ar0			
        bZ      interrupt_return                
	addi	IR0, ar0
        stik    0, *+ar0(0)        
	stik	2, *+ar0(4)
        lsh	-2, *+ar3(12), ar0
	addi	IR0, ar0
        sti     ar2, *+ar3(12)
        sti     ar2, *+ar0(0)
	ldi	*+ar3(1), ar1
	bnz	extern_slice_now
	pop	ar3	                
        pop     ar2
        pop     ar1
        pop     ar0
	pop	IR0
        pop     st
        retiU                           
TxANDRxInterrupt:
        lsh	-2, *+ar2(4), ar2	
	addi	IR0, ar2
        and     0b111, ar0                      
        addi    ar0, ar2                        
        lsh	-2, *+ar2(0), ar0		
	addi	IR0, ar0			
TxAndRxInterrupt2:
        ldi     *+ar0(19), ar2       
        stik    0, *+ar0(19)         
        ldi     *+ar0(18), ar1       
        stik    0, *+ar0(18)         
HandleRxSide:
	lsh	-2, ar2, ar0			
        bZ      HandleTxSide                    
	addi	IR0, ar0
        stik    0, *+ar0(0)        
	stik	2, *+ar0(4)
        lsh	-2, *+ar3(12), ar0
	addi	IR0, ar0
        sti     ar2, *+ar3(12)
        sti     ar2, *+ar0(0)
HandleTxSide:
        lsh	-2, ar1, ar0                    
        bZ      TxAndRxReturn                   
	addi	IR0, ar0
        stik    0, *+ar0(0)        
	stik	2, *+ar0(4)
        lsh	-2, *+ar3(12), ar0
	addi	IR0, ar0
        sti     ar1, *+ar3(12)
        sti     ar1, *+ar0(0)
	ldi	*+ar3(1), ar1
	bnz	extern_slice_now
TxAndRxReturn:
        pop     ar3                     
        pop     ar2
        pop     ar1
        pop     ar0
	pop	IR0
        pop     st
        retiU                           
