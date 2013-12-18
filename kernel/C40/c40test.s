	word	0x1e4a4000		
	word	0x1e4a4000		
	word	(CodeEnd - CodeStart)	
	word	0x300000		
CodeStart:
start:
	b start
	ldhi	0x0030, ar1
	or	0x00000040, ar1
	stik	0x1, *ar1++
	stik	0x2, *ar1++
	stik	0x4, *ar1++
	stik	0x8, *ar1++
fubar:
	b fubar
CodeEnd:
	word 0		
	word 0x2ffc00	
	word 0x2ffe00	
	word 0x300000	
