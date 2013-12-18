BootstrapStart:
	word	32		
	word	0x3e39fff0	
	word	0x3e39fff0	
	word	(BootstrapCopyEnd - BootstrapEntryPoint)
	word	0x2ffc00	
BootstrapEntryPoint:
	lda	ar1, ar6
	or	(1 << 11), st
	laj	_absaddr_BoardIDROM + 1		
		nop				
		ldi	R11, ar5
		addi	*ar5, ar5		
_absaddr_BoardIDROM:
	int	shift(-2, labelref(BoardIDROM))
	ldhi	0x0010, ar0
	ldi	*+ar5(14), r0
	sti	r0, *+ar0(0x0038)
	ldi	*+ar5(15), r0
	lsh	-16, r0
	ldhi	(((1 << 30) | (1 << 31)) >> 16) & 0xffff, r1
	or	r1, r0
	sti	r0, *+ar0(0x0030)
	ldi	*+ar5(13), r0
	sti	r0, *+ar0(0x0028)
	ldi	*+ar5(5), ar1
		ldi	ar1, IR0	
		ldpe	ar1, tvtp
		lda	ar6, ar2
		sti	ar2, *+ar1(4)
	sti	IR0,	*+ar1(28)
	stik	0, *+ar1(29)
	ldi	ar1, SP
	addi	472 / 4, SP
	ldi	SP, ar3
	addi	0x200-1, ar3
	andn	0x200-1, ar3
	ldpe	ar3, ivtp	
	addi	0x40, ar3	
	ldi	ar3, r0
	subi	IR0, r0
	lsh	2, r0
	sti	r0, *+ar1(3)	
	addi	70, ar1, ar4
	ldi	76 / 4, r0
IDROM_Copy:
	ldi	*ar5++, r1
	sti	r1, *ar4++
	subi	1, r0
	bnz	IDROM_Copy
	addi	404 / 4, ar3
	laj	_absaddr_BoardConfig + 1		
		nop				
		ldi	R11, ar5
		addi	*ar5, ar5		
_absaddr_BoardConfig:
	int	shift(-2, labelref(BoardConfig))
	lsh	-2, *+ar5(3), r0
Config_Copy:
	ldi	*ar5++, r1
	sti	r1, *ar3++
	subi	1, r0
	bnz	Config_Copy
		ldi	30 + 40 - 1, AR6
		addi	ar1, AR6
	or	(1 << 15), ST
	ldi	-1, R0			
	ldi	*+ar1(4), R1	
	addi	1, R1, ar2
	ldi	*ar2, ar1	
	lsh	-2, ar1		
	addi	ar2, ar1	
	addi	60 / 4, ar1
	b	ar1		
BoardIDROM:
	word	76 / 4	
	word	0x3100feff	
	word	0		
	word	0x80000000	
	word	-1		
	word	0x00300000	
	word	-1		
	word	0x00100000	
	word	0		
	word	0x00100000	
	word	0		
	word	0800		
	word	0x55552222	
	word	0x203a		
	word	0x80		
	word	0x02810282	
	word	0x1e4a4000	
	word	0x1e4a4000	
	word	1		
BoardIDROMEnd:
	byte	"Bootstrap Config Structure"
	align
BoardConfig:
	word	1024		
	word	1		
	word	0		
	word	(BoardConfigEnd - BoardConfig) * 4	
	word	0		
	word	9		
	word	0		
	word	0x00000001 | 0x00000004	
	word	0		
	word	-1		
	word	-1		
	word	6		
			word	
				( (0) << 0 |
				0     << 8 |
				0    << 16 |
				0       << 24 )
			word	
				( (0) << 0 |
				0     << 8 |
				0    << 16 |
				1       << 24 )
			word	
				( (0) << 0 |
				0     << 8 |
				0    << 16 |
				2       << 24 )
			word	
				( ((0x20|0x10)) << 0 |
				2     << 8 |
				6    << 16 |
				3       << 24 )
			word	
				( (0) << 0 |
				0     << 8 |
				0    << 16 |
				4       << 24 )
			word	
				( (0) << 0 |
				0     << 8 |
				0    << 16 |
				5       << 24 )
MyName:
		align
BoardConfigEnd:
BootstrapCopyEnd:
	word	0		
	word	0		
	word	0		
	word	0x00300000	
BootstrapEnd:
