******************************************************
*    TMS320C30 C COMPILER     Version 4.00
******************************************************
;	ac30 -v40 -ic:\c40 getchar.c C:\TMP\getchar.if 
;	cg30 -v40 -o -n C:\TMP\getchar.if C:\TMP\getchar.asm C:\TMP\getchar.tmp 
FP	.set	AR3
	.sect	".cinit"
	.word	1,_key_ptr
	.word	0
	.globl	_key_ptr
	.bss	_key_ptr,1
	.globl	_key_mode
	.text
	.globl	_c_int01
;>>>> 	void c_int01( void )      /* Keyboard Handler */
;>>>> 		char dummy;
******************************************************
* FUNCTION DEF : _c_int01
******************************************************
_c_int01:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	PUSH	ST
	PUSH	R0
	PUSHF	R0
	PUSH	R1
	PUSHF	R1
	PUSH	R2
	PUSHF	R2
	PUSH	R3
	PUSHF	R3
	PUSH	R4
	PUSHF	R4
	PUSH	R5
	PUSHF	R5
	PUSH	R6
	PUSHF	R6
	PUSH	R7
	PUSHF	R7
	PUSH	AR0
	PUSH	AR1
	PUSH	AR2
	PUSH	IR0
	PUSH	IR1
	PUSH	BK
	PUSH	RC
	PUSH	RS
	PUSH	RE
	PUSH	DP
;>>>> 		if( key_ptr == BUF_SIZE - 1 )
	LDI	@_key_ptr,R0
	CMPI	19,R0
	BNZ	L1
;>>>> 			dummy = (char)((*((unsigned long *)0xFFFD2000) >> 16) & 0x0000007F);
	LDI	@CONST+0,AR0
	LDI	*AR0,R1
	LSH	-16,R1
	AND	07fh,R1
	STI	R1,*+FP(1)
;>>>> 			c40_putchar( BEEP );
	LDI	7,R2
	PUSH	R2
	CALL	_c40_putchar
	SUBI	1,SP
;>>>> 		else
	B	EPI0_1
L1:
;>>>> 			key_buf[++key_ptr] = (*((unsigned long *)0xFFFD2000) >> 16) & 0x000000FF;
	ADDI	1,R0
	STI	R0,@_key_ptr
	ADDI	@CONST+1,R0
	LDI	R0,AR0
	LDI	@CONST+0,AR1
	LDI	*AR1,R0
	LSH	-16,R0
	AND	0ffh,R0
	STI	R0,*AR0
EPI0_1:
	POP	DP
	POP	RE
	POP	RS
	POP	RC
	POP	BK
	POP	IR1
	POP	IR0
	POP	AR2
	POP	AR1
	POP	AR0
	POPF	R7
	POP	R7
	POPF	R6
	POP	R6
	POPF	R5
	POP	R5
	POPF	R4
	POP	R4
	POPF	R3
	POP	R3
	POPF	R2
	POP	R2
	POPF	R1
	POP	R1
	POPF	R0
	POP	R0
	POP	ST
	SUBI	1,SP
	POP	FP
	RETI
	.globl	_c40_getchar
;>>>> 	int c40_getchar( void )
;>>>> 		char next_ch;
;>>>> 		int i;
******************************************************
* FUNCTION DEF : _c40_getchar
******************************************************
_c40_getchar:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
L3:
;>>>> 		while( !key_ptr );
	LDI	@_key_ptr,R0
	BZ	L3
;>>>> 		next_ch = key_buf[key_ptr];
	ADDI	@CONST+1,R0
	LDI	R0,AR0
	LDI	*AR0,R0
	STI	R0,*+FP(1)
;>>>> 		for( i=1 ; i < key_ptr ; i++ )
	LDI	1,R1
	STI	R1,*+FP(2)
	CMPI	@_key_ptr,R1
	BGE	L5
L4:
;>>>> 			key_buf[i] = key_buf[i+1];
	LDI	@CONST+1,R0
	ADDI	*+FP(2),R0
	LDI	R0,AR0
	LDI	@CONST+2,R3
	ADDI	*+FP(2),R3
	LDI	R3,AR1
	LDI	*AR1,R0
	STI	R0,*AR0
	LDI	*+FP(2),R0
	ADDI	1,R0
	STI	R0,*+FP(2)
	CMPI	@_key_ptr,R0
	BLT	L4
L5:
;>>>> 		key_ptr--;
	LDI	@_key_ptr,R0
	SUBI	1,R0
	STI	R0,@_key_ptr
;>>>> 		c40_putchar( next_ch );
	LDI	*+FP(1),R1
	PUSH	R1
	CALL	_c40_putchar
	SUBI	1,SP
;>>>> 		return( next_ch );
	LDI	*+FP(1),R0
EPI0_2:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	4,SP
***	B	R1	;BRANCH OCCURS
	.globl	_c40_putchar
;>>>> 	int c40_putchar( char ch )
******************************************************
* FUNCTION DEF : _c40_putchar
******************************************************
_c40_putchar:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
;>>>> 		unsigned long *out_reg=(unsigned long *)0xFFFFE00,
	LDI	@CONST+3,R0
	STI	R0,*+FP(1)
;>>>> 						  *int_status_reg=(unsigned long *)0xFFFD2500;
	LDI	@CONST+4,R1
	STI	R1,*+FP(2)
L6:
;>>>> 		while( !(*int_status_reg & (unsigned long)0x01000000) );
	LDI	*+FP(2),AR0
	LDI	*AR0,R0
	TSTB	@CONST+5,R0
	BZ	L6
;>>>> 		*(out_reg) = ch << 24;
	LDI	*-FP(2),R0
	LSH	24,R0
	LDI	*+FP(1),AR1
	STI	R0,*AR1
;>>>> 		return( ch );
	LDI	*-FP(2),R0
EPI0_3:
	LDI	*-FP(1),R1
	BD	R1
	LDI	*FP,FP
	NOP
	SUBI	4,SP
***	B	R1	;BRANCH OCCURS
	.bss	_key_buf,20
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,6
	.sect	".cinit"
	.word	6,CONST
	.word 	-188416          ;0
	.word 	_key_buf         ;1
	.word 	_key_buf+1       ;2
	.word 	268434944        ;3
	.word 	-187136          ;4
	.word 	16777216         ;5
	.end
