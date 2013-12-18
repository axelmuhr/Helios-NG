******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 getchar.c getchar.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o getchar.if getchar.asm getchar.tmp 
	.version	40
FP	.set		AR3
	.file	"getchar.c"

	.sect	".cinit"
	.word	1,_key_ptr
	.word	0

	.sym	_key_ptr,_key_ptr,4,2,32
	.globl	_key_ptr
	.bss	_key_ptr,1
	.globl	_key_mode
	.text

	.sym	_c_int01,_c_int01,32,2,0
	.globl	_c_int01

	.func	10
;>>>> 	void c_int01( void )      /* Keyboard Handler */
;>>>> 		char dummy;
;>>>> 		unsigned long tcr;
******************************************************
* FUNCTION DEF : _c_int01
******************************************************
_c_int01:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
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
	PUSH	R8
	PUSHF	R8
	PUSH	R9
	PUSHF	R9
	PUSH	R10
	PUSHF	R10
	PUSH	R11
	PUSHF	R11
	PUSH	AR0
	PUSH	AR1
	PUSH	AR2
	PUSH	IR0
	PUSH	IR1
	PUSH	BK
	PUSH	RC
	PUSH	RS
	PUSH	RE
	.sym	_dummy,1,2,1,32
	.sym	_tcr,2,15,1,32
	.line	7
;>>>> 		writeVIC( 0x27, 0xa1 );	/* Disable this interrupt */
	LDI	161,R0
	PUSH	R0
	LDI	39,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	9
;>>>> 		if( key_ptr == BUF_SIZE - 1 )
	LDI	@_key_ptr,R0
	CMPI	19,R0
	BNZ	L1
	.line	12
;>>>> 			while( !(readVAC( 0x25 ) & 0x80 ) )
	LDI	37,R1
	PUSH	R1
	CALL	_readVAC
	SUBI	1,SP
	TSTB	128,R0
	BNZ	L3
L2:
	.line	13
;>>>> 				dummy = (char)readVAC( 0x20 );
	LDI	32,R0
	PUSH	R0
	CALL	_readVAC
	SUBI	1,SP
	STI	R0,*+FP(1)
	.line	12
	LDI	37,R1
	PUSH	R1
	CALL	_readVAC
	SUBI	1,SP
	TSTB	128,R0
	BZ	L2
L3:
	.line	16
;>>>> 			dummy = (char)readVAC( 0x20 );
	LDI	32,R0
	PUSH	R0
	CALL	_readVAC
	SUBI	1,SP
	STI	R0,*+FP(1)
	.line	17
;>>>> 			c40_putchar( BEEP );
;>>>> 		else
	LDI	7,R1
	PUSH	R1
	CALL	_c40_putchar
	SUBI	1,SP
	B	L4
L1:
	.line	22
;>>>> 			while( !(readVAC( 0x25 ) & 0x80 ) )
	LDI	37,R1
	PUSH	R1
	CALL	_readVAC
	SUBI	1,SP
	TSTB	128,R0
	BNZ	L6
L5:
	.line	23
;>>>> 				dummy = (char)readVAC( 0x20 );
	LDI	32,R0
	PUSH	R0
	CALL	_readVAC
	SUBI	1,SP
	STI	R0,*+FP(1)
	.line	22
	LDI	37,R1
	PUSH	R1
	CALL	_readVAC
	SUBI	1,SP
	TSTB	128,R0
	BZ	L5
L6:
	.line	26
;>>>> 			key_buf[++key_ptr] = (readVAC( 0x20 ) & 0x0000007F);
	LDI	32,R0
	PUSH	R0
	CALL	_readVAC
	SUBI	1,SP
	AND	07fh,R0
	LDA	@_key_ptr,IR0
	ADDI	1,IR0,IR1
	STI	IR1,@_key_ptr
	LDA	@CONST+0,AR0
	STI	R0,*+AR0(IR1)
L4:
	.line	29
;>>>> 		tcr = readTCR();		/* Save old TCR */
	CALL	_readTCR
	STI	R0,*+FP(2)
	.line	30
;>>>> 		writeTCR( 0x40 );		/* Set LA1 high */
	LDI	64,R1
	PUSH	R1
	CALL	_writeTCR
	SUBI	1,SP
	.line	31
;>>>> 		dummy = *((char *) 0xbfffffc0 );	/* IACK */
	LDA	@CONST+1,AR0
	LDI	*AR0,R0
	STI	R0,*+FP(1)
	.line	32
;>>>> 		writeTCR( tcr );			/* restore old TCR */
	LDI	*+FP(2),R1
	PUSH	R1
	CALL	_writeTCR
	SUBI	1,SP
	.line	34
;>>>> 		writeVIC( 0x27, 0x01 );		/* Re-enable this interrupt */
	LDI	1,R0
	PUSH	R0
	LDI	39,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
EPI0_1:
	.line	35
	POP	RE
	POP	RS
	POP	RC
	POP	BK
	POP	IR1
	POP	IR0
	POP	AR2
	POP	AR1
	POP	AR0
	POPF	R11
	POP	R11
	POPF	R10
	POP	R10
	POPF	R9
	POP	R9
	POPF	R8
	POP	R8
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
	SUBI	2,SP
	POP	FP
	RETI

	.endfunc	44,00bff07ffH,2

	.sym	_c40_getchar,_c40_getchar,36,2,0
	.globl	_c40_getchar

	.func	48
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
	.sym	_next_ch,1,2,1,32
	.sym	_i,2,4,1,32
L7:
	.line	6
;>>>> 		while( !key_ptr );
	LDI	@_key_ptr,R0
	BZ	L7
	.line	8
;>>>> 		next_ch = key_buf[key_ptr];
	LDA	@CONST+0,AR0
	LDA	@_key_ptr,IR0
	LDI	*+AR0(IR0),R1
	STI	R1,*+FP(1)
	.line	10
;>>>> 		for( i=1 ; i < key_ptr ; i++ )
	STIK	1,*+FP(2)
	CMPI	*+FP(2),R0
	BLE	L9
L8:
	.line	11
;>>>> 			key_buf[i] = key_buf[i+1];
	LDA	*+FP(2),IR1
	LDA	@CONST+0,AR0
	ADDI	1,AR0
	LDA	@CONST+0,AR1
	LDI	*+AR0(IR1),R0
	STI	R0,*+AR1(IR1)
	.line	10
	ADDI	1,*+FP(2),R0
	STI	R0,*+FP(2)
	CMPI	@_key_ptr,R0
	BLT	L8
L9:
	.line	13
;>>>> 		key_ptr--;
	LDI	@_key_ptr,R0
	SUBI	1,R0,R1
	STI	R1,@_key_ptr
	.line	15
;>>>> 		return( next_ch );
	LDI	*+FP(1),R0
EPI0_2:
	.line	16
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	4,SP
	B	R1
	.endfunc	63,000000000H,2

	.sym	_c40_putchar,_c40_putchar,34,2,0
	.globl	_c40_putchar

	.func	67
;>>>> 	char c40_putchar( char ch )
******************************************************
* FUNCTION DEF : _c40_putchar
******************************************************
_c40_putchar:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_ch,-2,2,9,32
	.sym	_i,1,4,1,32
	.line	2
;>>>> 		int i;
	.line	5
;>>>> 		writeVIC( 0x27, 0xa1 );	/* Disable this interrupt */
	LDI	161,R0
	PUSH	R0
	LDI	39,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
L10:
	.line	8
;>>>> 		while( !(readVAC(0x25) & (unsigned long)0x100) );
	LDI	37,R0
	PUSH	R0
	CALL	_readVAC
	SUBI	1,SP
	TSTB	256,R0
	BZ	L10
L11:
	.line	11
;>>>> 		while( !(readVAC(0x25) & (unsigned long)0x100) );
	LDI	37,R0
	PUSH	R0
	CALL	_readVAC
	SUBI	1,SP
	TSTB	256,R0
	BZ	L11
	.line	14
;>>>> 		writeVAC( 0x1E, ch << 8 );
	LDI	*-FP(2),R0
	LSH	8,R0,R1
	PUSH	R1
	LDI	30,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	16
;>>>> 		for( i=0 ; i < 100 ; )
	STIK	0,*+FP(1)
	LDI	100,R0
	CMPI	R0,*+FP(1)
	BGE	L13
L12:
	.line	17
;>>>> 			i++;
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	16
	CMPI	100,R0
	BLT	L12
L13:
	.line	19
;>>>> 		writeVIC( 0x27, 0x01 );		/* Re-enable this interrupt */
	LDI	1,R0
	PUSH	R0
	LDI	39,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	21
;>>>> 		return( ch );
	LDI	*-FP(2),R0
EPI0_3:
	.line	22
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	88,000000000H,1

	.sym	_key_buf,_key_buf,50,3,640,,20
	.bss	_key_buf,20
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,2
	.sect	".cinit"
	.word	2,CONST
	.word 	_key_buf         ;0
	.word 	-1073741888      ;1
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_writeVIC
	.globl	_readVAC
	.globl	_writeTCR
	.globl	_readTCR
	.globl	_writeVAC
	.end
