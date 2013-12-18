******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 vicvac.c vicvac.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o vicvac.if vicvac.asm vicvac.tmp 
	.version	40
FP	.set		AR3
	.file	"vicvac.c"

	.sym	_writeVIC,_writeVIC,32,2,0
	.globl	_writeVIC

	.func	1
;>>>> 	void writeVIC( unsigned long add, unsigned long data )
******************************************************
* FUNCTION DEF : _writeVIC
******************************************************
_writeVIC:
	PUSH	FP
	LDI	SP,FP
	.sym	_add,-2,15,9,32
	.sym	_data,-3,15,9,32
	.line	2
	.line	3
;>>>> 		*((unsigned long *)(((0xFFFC0000 | add) >> 2) | 0xB0000000)) = data;
	LDA	*-FP(2),AR0
	OR	@CONST+0,AR0
	LSH	-2,AR0
	OR	@CONST+1,AR0
	LDI	*-FP(3),R0
	STI	R0,*AR0
EPI0_1:
	.line	4
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	4,000000000H,0

	.sym	_readVIC,_readVIC,47,2,0
	.globl	_readVIC

	.func	6
;>>>> 	unsigned long readVIC( unsigned long add )
******************************************************
* FUNCTION DEF : _readVIC
******************************************************
_readVIC:
	PUSH	FP
	LDI	SP,FP
	.sym	_add,-2,15,9,32
	.line	2
	.line	3
;>>>> 		return( *((unsigned long *)(((0xFFFC0000 | add) >> 2) | 0xB0000000)) );
	LDA	*-FP(2),AR0
	OR	@CONST+0,AR0
	LSH	-2,AR0
	OR	@CONST+1,AR0
	LDI	*AR0,R0
EPI0_2:
	.line	4
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	9,000000000H,0

	.sym	_writeVAC,_writeVAC,32,2,0
	.globl	_writeVAC

	.func	11
;>>>> 	void writeVAC( unsigned long add, unsigned long data )
******************************************************
* FUNCTION DEF : _writeVAC
******************************************************
_writeVAC:
	PUSH	FP
	LDI	SP,FP
	.sym	_add,-2,15,9,32
	.sym	_data,-3,15,9,32
	.line	2
	.line	3
;>>>> 		*((unsigned long *)(((0xFFFD0000 | (add << 8)) >> 2) | 0xB0000000)) = (data << 16);
	LDI	*-FP(3),R0
	LSH	16,R0
	LDA	*-FP(2),AR0
	LSH	8,AR0,AR1
	OR	@CONST+2,AR1
	LSH	-2,AR1
	OR	@CONST+1,AR1
	STI	R0,*AR1
EPI0_3:
	.line	4
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	14,000000000H,0

	.sym	_readVAC,_readVAC,47,2,0
	.globl	_readVAC

	.func	16
;>>>> 	unsigned long readVAC( unsigned long add )
******************************************************
* FUNCTION DEF : _readVAC
******************************************************
_readVAC:
	PUSH	FP
	LDI	SP,FP
	.sym	_add,-2,15,9,32
	.line	2
	.line	3
;>>>> 		return( (*((unsigned long *)(( ((0xFFFD0000 | (add << 8)) >> 2) | 0xB0000000))) >> 16) );
	LDA	*-FP(2),AR0
	LSH	8,AR0,AR1
	OR	@CONST+2,AR1
	LSH	-2,AR1
	OR	@CONST+1,AR1
	LSH	-16,*AR1,R0
EPI0_4:
	.line	4
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	19,000000000H,0

	.sym	_readVACEPROM,_readVACEPROM,32,2,0
	.globl	_readVACEPROM

	.func	21
;>>>> 	void readVACEPROM( void )
;>>>> 		unsigned long i;
******************************************************
* FUNCTION DEF : _readVACEPROM
******************************************************
_readVACEPROM:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_i,1,15,1,32
	.line	5
;>>>> 		i = *((unsigned long *)(((unsigned long)0xff000000 >> 2) | 0xb0000000));
	LDA	@CONST+3,AR0
	LDI	*AR0,R0
	STI	R0,*+FP(1)
EPI0_5:
	.line	6
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	26,000000000H,1

	.sym	_SetupUART,_SetupUART,32,2,0
	.globl	_SetupUART

	.func	30
;>>>> 	void SetupUART( void )
******************************************************
* FUNCTION DEF : _SetupUART
******************************************************
_SetupUART:
	PUSH	FP
	LDI	SP,FP
	.line	18
;>>>> 		writeVAC( 0x1d, 0x77d0 );
	LDI	30672,R0
	PUSH	R0
	LDI	29,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	19
;>>>> 		writeVAC( 0x1e, 7 );
	LDI	7,R0
	PUSH	R0
	LDI	30,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	20
;>>>> 		writeVAC( 0x1e, 7 );
	LDI	7,R0
	PUSH	R0
	LDI	30,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	21
;>>>> 		writeVAC( 0x1d, 0x77c0 );
	LDI	30656,R0
	PUSH	R0
	LDI	29,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
EPI0_6:
	.line	24
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	53,000000000H,0

	.sym	_setupVICVAC,_setupVICVAC,32,2,0
	.globl	_setupVICVAC

	.func	56
;>>>> 	void setupVICVAC( void )
******************************************************
* FUNCTION DEF : _setupVICVAC
******************************************************
_setupVICVAC:
	PUSH	FP
	LDI	SP,FP
	ADDI	2,SP
	.sym	_i,1,4,1,32
	.sym	_j,2,4,1,32
	.line	3
;>>>> 		int i=0, j;
	STIK	0,*+FP(1)
	.line	6
;>>>> 		readVACEPROM();	
	CALL	_readVACEPROM
	.line	8
;>>>> 		writeVIC( 0xA7, 0x56 );
	LDI	86,R0
	PUSH	R0
	LDI	167,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	9
;>>>> 		writeVIC( 0xB3, 0x70 );
	LDI	112,R0
	PUSH	R0
	LDI	179,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	10
;>>>> 		writeVIC( 0xB7, 0x0D );
	LDI	13,R0
	PUSH	R0
	LDI	183,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	11
;>>>> 		writeVIC( 0xC3, 0x10 );
	LDI	16,R0
	PUSH	R0
	LDI	195,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	12
;>>>> 		writeVIC( 0xC7, 0xFF );
	LDI	255,R0
	PUSH	R0
	LDI	199,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	13
;>>>> 		writeVIC( 0xCB, 0x10 );
	LDI	16,R0
	PUSH	R0
	LDI	203,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	14
;>>>> 		writeVIC( 0xCF, 0xFF );
	LDI	255,R0
	PUSH	R0
	LDI	207,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	16
;>>>> 		writeVAC( 0x0, 0x00F8 );
	LDI	248,R0
	PUSH	R0
	LDI	0,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	17
;>>>> 		writeVAC( 0x1, 0x0040 );
	LDI	64,R0
	PUSH	R0
	LDI	1,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	18
;>>>> 		writeVAC( 0x2, 0xFF00 );
	LDI	@CONST+4,R0
	PUSH	R0
	LDI	2,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	19
;>>>> 		writeVAC( 0x3, 0x3400 );
	LDI	13312,R0
	PUSH	R0
	LDI	3,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	20
;>>>> 		writeVAC( 0x4, 0xF0F1 );
	LDI	@CONST+5,R0
	PUSH	R0
	LDI	4,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	21
;>>>> 		writeVAC( 0x5, 0x0001 );
	LDI	1,R0
	PUSH	R0
	LDI	5,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	22
;>>>> 		writeVAC( 0x6, 0x0002 );
	LDI	2,R0
	PUSH	R0
	LDI	6,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	23
;>>>> 		writeVAC( 0x7, 0xFDFF );
	LDI	@CONST+6,R0
	PUSH	R0
	LDI	7,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	24
;>>>> 		writeVAC( 0x8, 0x7F00 );
	LDI	32512,R0
	PUSH	R0
	LDI	8,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	25
;>>>> 		writeVAC( 0x9, 0x2C00 );
	LDI	11264,R0
	PUSH	R0
	LDI	9,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	26
;>>>> 		writeVAC( 0xA, 0x4C00 );
	LDI	19456,R0
	PUSH	R0
	LDI	10,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	27
;>>>> 		writeVAC( 0xB, 0x0400 );
	LDI	1024,R0
	PUSH	R0
	LDI	11,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	28
;>>>> 		writeVAC( 0xC, 0xF810 );
	LDI	@CONST+7,R0
	PUSH	R0
	LDI	12,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	29
;>>>> 		writeVAC( 0xD, 0xF810 );
	LDI	@CONST+7,R0
	PUSH	R0
	LDI	13,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	30
;>>>> 		writeVAC( 0xE, 0xE010 );
	LDI	@CONST+8,R0
	PUSH	R0
	LDI	14,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	31
;>>>> 		writeVAC( 0xF, 0xF810 );
	LDI	@CONST+7,R0
	PUSH	R0
	LDI	15,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	32
;>>>> 		writeVAC( 0x10, 0xE014 );
	LDI	@CONST+9,R0
	PUSH	R0
	LDI	16,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	33
;>>>> 		writeVAC( 0x11, 0xF810 );
	LDI	@CONST+7,R0
	PUSH	R0
	LDI	17,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	34
;>>>> 		writeVAC( 0x12, 0xF810 );
	LDI	@CONST+7,R0
	PUSH	R0
	LDI	18,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	35
;>>>> 		writeVAC( 0x13, 0xF810 );
	LDI	@CONST+7,R0
	PUSH	R0
	LDI	19,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	36
;>>>> 		writeVAC( 0x14, 0xEBFE );
	LDI	@CONST+10,R0
	PUSH	R0
	LDI	20,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	37
;>>>> 		writeVAC( 0x1b, 0x1FFF );
	LDI	8191,R0
	PUSH	R0
	LDI	27,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	39
;>>>> 		writeVAC( 0x29, readVAC( 0x29 ) );	/* Initialize VAC with read and write of the VAC id */
	LDI	41,R0
	PUSH	R0
	CALL	_readVAC
	SUBI	1,SP
	PUSH	R0
	LDI	41,R0
	PUSH	R0
	CALL	_writeVAC
	SUBI	2,SP
	.line	42
;>>>> 		writeVIC( 0x27, j=ReadEeprom(i++) );
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	SUBI	1,R0
	PUSH	R0
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	39,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	43
;>>>> 		writeVIC( 0x57, j=ReadEeprom(i++) );
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	SUBI	1,R0
	PUSH	R0
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	87,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	44
;>>>> 		writeVIC( 0xA7, j=ReadEeprom(i++) );
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	SUBI	1,R0
	PUSH	R0
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	167,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	45
;>>>> 		writeVIC( 0xB3, j=ReadEeprom(i++) );
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	SUBI	1,R0
	PUSH	R0
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	179,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	46
;>>>> 		writeVIC( 0xB7, j=ReadEeprom(i++) );
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	SUBI	1,R0
	PUSH	R0
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	183,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	47
;>>>> 		writeVIC( 0xC3, j=ReadEeprom(i++) );
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	SUBI	1,R0
	PUSH	R0
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	195,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	48
;>>>> 		writeVIC( 0xC7, j=ReadEeprom(i++) );
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	SUBI	1,R0
	PUSH	R0
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	199,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	49
;>>>> 		writeVIC( 0xCB, j=ReadEeprom(i++) );
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	SUBI	1,R0
	PUSH	R0
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	203,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	50
;>>>> 		writeVIC( 0xCF, j=ReadEeprom(i++) );
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	SUBI	1,R0
	PUSH	R0
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	207,R1
	PUSH	R1
	CALL	_writeVIC
	SUBI	2,SP
	.line	51
;>>>> 		i++;
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	53
;>>>> 		writeVAC( 0x0, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	0,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	54
;>>>> 		i += 2;	
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	55
;>>>> 		writeVAC( 0x1, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	1,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	56
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	57
;>>>> 		writeVAC( 0x2, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	2,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	58
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	59
;>>>> 		writeVAC( 0x3, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	3,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	60
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	61
;>>>> 		writeVAC( 0x4, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	4,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	62
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	63
;>>>> 		writeVAC( 0x5, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	5,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	64
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	65
;>>>> 		writeVAC( 0x6, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	6,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	66
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	67
;>>>> 		writeVAC( 0x7, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	7,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	68
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	69
;>>>> 		writeVAC( 0x8, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	8,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	70
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	71
;>>>> 		writeVAC( 0x9, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	9,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	72
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	73
;>>>> 		writeVAC( 0xA, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	10,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	74
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	75
;>>>> 		writeVAC( 0xB, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	11,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	76
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	77
;>>>> 		writeVAC( 0xC, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	12,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	78
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	79
;>>>> 		writeVAC( 0xD, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	13,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	80
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	81
;>>>> 		writeVAC( 0xE, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	14,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	82
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	83
;>>>> 		writeVAC( 0xF, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	15,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	84
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	85
;>>>> 		writeVAC( 0x10, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	16,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	86
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	87
;>>>> 		writeVAC( 0x11, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	17,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	88
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	89
;>>>> 		writeVAC( 0x12, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	18,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	90
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	91
;>>>> 		writeVAC( 0x13, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	19,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	92
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	93
;>>>> 		writeVAC( 0x14, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	20,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	94
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	95
;>>>> 		writeVAC( 0x16, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	22,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	96
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	97
;>>>> 		writeVAC( 0x1a, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	26,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	98
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	99
;>>>> 		writeVAC( 0x1b, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	27,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	100
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	101
;>>>> 		writeVAC( 0x1c, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	28,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	102
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	103
;>>>> 		writeVAC( 0x1d, j=ReadEepromWord(i) );	
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	29,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	104
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	105
;>>>> 		writeVAC( 0x23, j=ReadEepromWord(i) );
	PUSH	R0
	CALL	_ReadEepromWord
	SUBI	1,SP
	STI	R0,*+FP(2)
	PUSH	R0
	LDI	35,R1
	PUSH	R1
	CALL	_writeVAC
	SUBI	2,SP
	.line	106
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	108
;>>>> 		writeVAC( 0x29, readVAC( 0x29 ) );	/* Initialize VAC with read and write of the VAC id */
	LDI	41,R1
	PUSH	R1
	CALL	_readVAC
	SUBI	1,SP
	PUSH	R0
	LDI	41,R0
	PUSH	R0
	CALL	_writeVAC
	SUBI	2,SP
EPI0_7:
	.line	110
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	4,SP
	B	R1
	.endfunc	165,000000000H,2

	.sym	_ReadEepromWord,_ReadEepromWord,47,2,0
	.globl	_ReadEepromWord

	.func	170
;>>>> 	unsigned long ReadEepromWord( int WordAddress )
******************************************************
* FUNCTION DEF : _ReadEepromWord
******************************************************
_ReadEepromWord:
	PUSH	FP
	LDI	SP,FP
	ADDI	3,SP
	.sym	_WordAddress,-2,4,9,32
	.sym	_word,1,15,1,32
	.sym	_i,2,4,1,32
	.sym	_j,3,4,1,32
	.line	2
;>>>> 		unsigned long word;
;>>>> 		int i, j;
	.line	6
;>>>> 		for( i=0, word=0L ; i < 2 ; i++ )
	STIK	0,*+FP(2)
	STIK	0,*+FP(1)
	CMPI	2,*+FP(2)
	BGE	L2
L1:
	.line	7
;>>>> 			word |= ((j=ReadEeprom( WordAddress++ )) << (i*8));
	LDI	*-FP(2),R0
	ADDI	1,R0,R1
	STI	R1,*-FP(2)
	SUBI	1,R1
	PUSH	R1
	CALL	_ReadEeprom
	SUBI	1,SP
	STI	R0,*+FP(3)
	LSH	3,*+FP(2),R1
	LSH	R1,R0,R1
	OR	*+FP(1),R1
	STI	R1,*+FP(1)
	.line	6
	ADDI	1,*+FP(2),R2
	STI	R2,*+FP(2)
	CMPI	2,R2
	BLT	L1
L2:
	.line	9
;>>>> 		return( word );
	LDI	*+FP(1),R0
EPI0_8:
	.line	10
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	5,SP
	B	R1
	.endfunc	179,000000000H,3
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,11
	.sect	".cinit"
	.word	11,CONST
	.word 	0fffc0000h       ;0
	.word 	0b0000000h       ;1
	.word 	0fffd0000h       ;2
	.word 	-1077936128      ;3
	.word 	65280            ;4
	.word 	61681            ;5
	.word 	65023            ;6
	.word 	63504            ;7
	.word 	57360            ;8
	.word 	57364            ;9
	.word 	60414            ;10
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_ReadEeprom
	.end
