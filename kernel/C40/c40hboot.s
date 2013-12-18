startboot:
	ldhi	0x10, ar2	
	or	0x40, ar2
	stik	0x1, *+ar2(2)
	ldi	*+ar3(1), r9
	tstb	32, r9
	bnz	DontEnableCache
	or	(1 << 11), st		
DontEnableCache:
	tstb	8, r9
	bnz	DontRaiseConfig
	ldi	(1 << 13) | (1 << 14), r0
	ldi	r0, iif
	ldhi	0x0010, ar1
	or	0x0020, ar1
DontRaiseConfig:
	tstb	8 | 16, r9
	bz	ReadRealIDROM
ReadPseudoIDROM:
	ldi	*+ar3(1), r1		
	subi	2, r1, r0		
	ldhi	0x2f, ar4
	or	0xf800, ar4	
	rpts	r0			
		ldi	*+ar3(1), r1 || sti	r1, *ar4++
	sti	r1, *ar4++		
	b	InitC40
ReadRealIDROM:
	ldhi	0x7000, ar2	
	ldi	*ar1, r0
	stik	(1 << 1), *ar1		
	ldi	2000, r1		
delay0:
	subi	1, r1
	bnz	delay0
	stik	(1 << 1)|(1 << 2), *ar1	
	ldi	2000, r1		
delay1:
	subi	1, r1
	bnz	delay1
	stik	(1 << 1), *ar1		
	ldi	2000, r1		
delay2:
	subi	1, r1
	bnz	delay2
	stik	(1 << 1)|(1 << 2), *ar1		
	and	(1 << 3), r0	
	bz	fourbitread	
onebitread:
getidsize:
	ldi	31, rc		
	rptb	endblock0
	ldi	*ar2++, r2	
	lsh	-1, r2		
endblock0:
	rorc	r3		
	ldi	r3, r0		
	bz	InitC40		
getdest:
	ldi	31, rc		
	rptb	endblock1
	ldi	*ar2++, r2	
	lsh	-1, r2		
endblock1:
	rorc	r3		
	ldi	r3, ar4		
dataloop:
	ldi	31, rc		
	rptb	endblock2
	ldi	*ar2++, r2	
	lsh	-1, r2		
endblock2:
	rorc	r3		
	sti	r3, *ar4++	
	subi	1, r0		
	bnz	dataloop
	b	onebitread
fourbitread:
get4idsize:
	ldi	0, r1		
	ldi	32/4-1, rc	
	rptb	end4block0
	ldi	*ar2++, r2	
	and	0xf, r2		
	lsh	r1, r2
	or	r2, r3		
end4block0:
	addi	4, r1
	ldi	r3, r0		
	bz	InitC40		
get4dest:
	ldi	0, r1		
	ldi	32/4-1, rc	
	rptb	endblock1
	ldi	*ar2++, r2	
	and	0xf, r2
	lsh	r1, r2
	or	r2, r3		
end4block1:
	addi	4, r1
	ldi	r3, ar4		
data4loop:
	ldi	0, r1		
	ldi	32/4-1, rc	
	rptb	endblock2
	ldi	*ar2++, r2	
	and	0xf, r2
	lsh	r1, r2
	or	r2, r3		
end4block2:
	addi	4, r1
	sti	r3, *ar4++	
	subi	1, r0		
	bnz	data4loop
	b	fourbitread
InitC40:
	ldhi	0x0010, ar0
	ldhi	0x2f, ar5
	or	0xf800, ar5	
	ldi	*+ar5(16), r0
	sti	r0, *+ar0(0x0000)
	ldi	*+ar5(17), r0
	sti	r0, *+ar0(0x0004)
	ldi	*+ar5(13), r0
	sti	r0, *+ar0(0x0028)
	ldi	*+ar5(14), r0
	sti	r0, *+ar0(0x0038)
	ldi	*+ar5(15), r0
	lsh	-16, r0
	ldhi	(((1 << 30) | (1 << 31)) >> 16) & 0xffff, r1
	or	r1, r0
	sti	r0, *+ar0(0x0030)
	tstb	8, r9
	bnz	NoTCLK0Reset
	stik	(1 << 1), *ar1		
NoTCLK0Reset:
	ldi    *++ar5(3),R0
	ldi    4,R7                 
nextstrobe:
	LDI    *ar5,AR1             
	LDI    *+ar5(1),R10         
	CMPI   1,R7                 
	BNE    notlast              
	LDI    *-ar5(3),R10         
notlast:
	LDI    *+ar5(4),R0          
	Bne    dunstrobe            
	LDI    -1,r0                
	CMPI   r0,AR1               
	Beq    dunstrobe            
	LDI    0x1000,R1            
	LDI    0xc40,R2             
	STI    R2,*AR1              
	STI    R1,*+AR1(1)
	LDA    AR1,AR2              
	CMPI   R2,*AR1
        Bne    gotall
lookmore:
	ADDI   R1,AR2               
	LDI    AR2,R3               
	ADDI   R2,R3                
	STI    R3,*AR2              
	CMPI   R2,*AR1              
	Bne    gotall               
	CMPI   AR1,R10              
	Beq    gotall               
	LDI    *AR2,R6              
	CMPI   R3,R6                
	Beq    lookmore             
gotall:
	SUBI3  AR1,AR2,R0           
	sti    R0,*+ar5(4)          
dunstrobe:
	LDI    *++ar5(1),R0         
	SUBI   1,R7                 
	Bne    nextstrobe           
	ldhi	0x2f, ar5
	or	0xf800, ar5	
	tstb	1, r9
	bnz	UseLStrobe1
	ldi	*+ar5(5), ar1
	addi	*+ar5(9), ar1, ar6
	cmpi	-1, ar1
	bne	ValidLStrobe
UseLStrobe1:
	ldi	*+ar5(6), ar1
	addi	*+ar5(10), ar1, ar6
ValidLStrobe:
	ldpe	ar1, tvtp	
	ldi	ar1, IR0	
	ldhi	0x4000, ar4
	cmpi	ar4, ar6	
	ldile	0, IR0	
	sti	IR0,	*+ar1(28)
	sti	r9, *+ar1(29)
	ldi	ar1, SP
	addi	472 / 4, SP
	ldi	SP, ar2
	addi	0x200-1, ar2
	andn	0x200-1, ar2
	ldpe	ar2, ivtp	
	addi	0x40, ar2	
TestGlobalS0:
	tstb	2, r9
	bz	TestGlobalS1
	ldi	*+ar5(3), ar4
	b	UseHighNuc
TestGlobalS1:
	tstb	4, r9
	bz	LocalBusNucleus
	ldi	*+ar5(4), ar4
UseHighNuc:
	ldi	ar2, r0
	subi	IR0, r0
	lsh	2, r0
	sti	r0, *+ar1(3)	
	ldi	ar4, ar2			
	b	SetNucAddress
LocalBusNucleus:
	stik	0,  *+ar1(3)
SetNucAddress:
	sti	ar2, *+ar1(4)
	ldi	*+ar3(1), r0		
	ldi	ar2, ar4		
	sti	r0, *ar4++		
	lsh	-2, r0			
	subi	3, r0			
	ldi	*+ar3(1), r1		
	rpts	r0			
		ldi	*+ar3(1), r1 || sti	r1, *ar4++
	sti	r1, *ar4++		
	addi	70, ar1, ar4
	ldi	*ar5, r0
IDROM_save:
	ldi	*ar5++, r1
	sti	r1, *ar4++
	subi	1, r0
	bnz	IDROM_save
	ldi	*+ar1(3), r0
	cmpi	0, r0
	bnz	CallKStart
	ldi	ar2, r0
	subi	IR0, r0
	lsh	2, r0
	addi	*ar2, r0			
	sti	r0, *+ar1(3)	
CallKStart:
	ldhi	0x30, AR6	
	subi	1, AR6
	or	(1 << 15), ST
	ldi	ar3, R0			
	ldi	ar2, R1			
	addi	1, ar2
	ldi	*ar2, ar1	
	lsh	-2, ar1		
	addi	ar2, ar1	
	addi	60 / 4, ar1
	tstb	8, r9
	bnz	NoDropConfig
	ldi	(1 << 13), ar2
	ldi	ar2, iif
NoDropConfig:
	b	ar1
