******************************************************
*    TMS320C40 C COMPILER     Version 4.30
******************************************************
;	ac30 -v40 -ic:\c40 makeeepr.c makeeepr.if 
;	opt30 NOT RUN
;	cg30 -v40 -o -o makeeepr.if makeeepr.asm makeeepr.tmp 
	.version	40
FP	.set		AR3
	.file	"makeeepr.c"

	.sym	_main,_main,36,2,0
	.globl	_main

	.func	1
;>>>> 	main()
******************************************************
* FUNCTION DEF : _main
******************************************************
_main:
	PUSH	FP
	LDI	SP,FP
	.line	4
;>>>> 			yoho();
	CALL	_yoho
EPI0_1:
	.line	5
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	2,SP
	B	R1
	.endfunc	5,000000000H,0

	.sym	_yoho,_yoho,36,2,0
	.globl	_yoho

	.func	9
;>>>> 	yoho()
******************************************************
* FUNCTION DEF : _yoho
******************************************************
_yoho:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_i,1,4,1,32
	.line	3
;>>>> 		int i=0;
	STIK	0,*+FP(1)
	.line	5
;>>>> 		WriteEeprom( i++, 0x09 ); 	/* Address 0x27 */	
	LDI	9,R0
	PUSH	R0
	ADDI	1,*+FP(1),R1
	STI	R1,*+FP(1)
	SUBI	1,R1
	PUSH	R1
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	6
;>>>> 		WriteEeprom( i++, 0x0F );	/* Address 0x57 */
	LDI	15,R0
	PUSH	R0
	ADDI	1,*+FP(1),R1
	STI	R1,*+FP(1)
	SUBI	1,R1
	PUSH	R1
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	7
;>>>> 		WriteEeprom( i++, 0x56 );	/* Address 0xA7 */	
	LDI	86,R0
	PUSH	R0
	ADDI	1,*+FP(1),R1
	STI	R1,*+FP(1)
	SUBI	1,R1
	PUSH	R1
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	8
;>>>> 		WriteEeprom( i++, 0x70 );	/* Address 0xB3 */	
	LDI	112,R0
	PUSH	R0
	ADDI	1,*+FP(1),R1
	STI	R1,*+FP(1)
	SUBI	1,R1
	PUSH	R1
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	9
;>>>> 		WriteEeprom( i++, 0x0D );	/* Address 0xB7 */
	LDI	13,R0
	PUSH	R0
	ADDI	1,*+FP(1),R1
	STI	R1,*+FP(1)
	SUBI	1,R1
	PUSH	R1
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	10
;>>>> 		WriteEeprom( i++, 0x10 );	/* Address 0xC3 */
	LDI	16,R0
	PUSH	R0
	ADDI	1,*+FP(1),R1
	STI	R1,*+FP(1)
	SUBI	1,R1
	PUSH	R1
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	11
;>>>> 		WriteEeprom( i++, 0xFF );	/* Address 0xC7 */
	LDI	255,R0
	PUSH	R0
	ADDI	1,*+FP(1),R1
	STI	R1,*+FP(1)
	SUBI	1,R1
	PUSH	R1
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	12
;>>>> 		WriteEeprom( i++, 0x10 );	/* Address 0xCB */
	LDI	16,R0
	PUSH	R0
	ADDI	1,*+FP(1),R1
	STI	R1,*+FP(1)
	SUBI	1,R1
	PUSH	R1
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	13
;>>>> 		WriteEeprom( i++, 0xFF );	/* Address 0xCF */
	LDI	255,R0
	PUSH	R0
	ADDI	1,*+FP(1),R1
	STI	R1,*+FP(1)
	SUBI	1,R1
	PUSH	R1
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	14
;>>>> 		i++;
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	16
;>>>> 		WriteEepromWord( i, 0x00F8 );	/* Address 0x00 */
	LDI	248,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	17
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	18
;>>>> 		WriteEepromWord( i, 0x0040 );	/* Address 0x01 */
	LDI	64,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	19
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	20
;>>>> 		WriteEepromWord( i, 0xFF00 );	/* Address 0x02 */
	LDI	@CONST+0,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	21
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	22
;>>>> 		WriteEepromWord( i, 0x3400 );	/* Address 0x03 */
	LDI	13312,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	23
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	24
;>>>> 		WriteEepromWord( i, 0xF0F1 );	/* Address 0x04 */
	LDI	@CONST+1,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	25
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	26
;>>>> 		WriteEepromWord( i, 0x0001 );	/* Address 0x05 */
	LDI	1,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	27
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	28
;>>>> 		WriteEepromWord( i, 0x0002 );	/* Address 0x06 */
	LDI	2,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	29
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	30
;>>>> 		WriteEepromWord( i, 0xFDFF );	/* Address 0x07 */
	LDI	@CONST+2,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	31
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	32
;>>>> 		WriteEepromWord( i, 0x7F00 );	/* Address 0x08 */
	LDI	32512,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	33
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	34
;>>>> 		WriteEepromWord( i, 0x2C00 );	/* Address 0x09 */
	LDI	11264,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	35
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	36
;>>>> 		WriteEepromWord( i, 0x4C00 );	/* Address 0x0A */
	LDI	19456,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	37
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	38
;>>>> 		WriteEepromWord( i, 0x0400 );	/* Address 0x0B */
	LDI	1024,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	39
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	40
;>>>> 		WriteEepromWord( i, 0xF810 );	/* Address 0x0C */
	LDI	@CONST+3,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	41
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	42
;>>>> 		WriteEepromWord( i, 0xF810 );	/* Address 0x0D */
	LDI	@CONST+3,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	43
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	44
;>>>> 		WriteEepromWord( i, 0xE010 );	/* Address 0x0E */
	LDI	@CONST+4,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	45
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	46
;>>>> 		WriteEepromWord( i, 0xF810 );	/* Address 0x0F */
	LDI	@CONST+3,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	47
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	48
;>>>> 		WriteEepromWord( i, 0xE014 );	/* Address 0x10 */
	LDI	@CONST+5,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	49
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	50
;>>>> 		WriteEepromWord( i, 0xF810 );	/* Address 0x11 */
	LDI	@CONST+3,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	51
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	52
;>>>> 		WriteEepromWord( i, 0xF810 );	/* Address 0x12 */
	LDI	@CONST+3,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	53
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	54
;>>>> 		WriteEepromWord( i, 0xF810 );	/* Address 0x13 */
	LDI	@CONST+3,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	55
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	56
;>>>> 		WriteEepromWord( i, 0xEBFE );	/* Address 0x14 */
	LDI	@CONST+6,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	57
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	58
;>>>> 		WriteEepromWord( i, 0x0010 );	/* Address 0x16 */
	LDI	16,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	59
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	60
;>>>> 		WriteEepromWord( i, 0x4000 );	/* Address 0x1A */
	LDI	16384,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	61
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	62
;>>>> 		WriteEepromWord( i, 0x1FFF );	/* Address 0x1B */
	LDI	8191,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	63
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	64
;>>>> 		WriteEepromWord( i, 0x4200 );	/* Address 0x1C */
	LDI	16896,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	65
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	66
;>>>> 		WriteEepromWord( i, 0x77C0 );	/* Address 0x1D */
	LDI	30656,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	67
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	68
;>>>> 		WriteEepromWord( i, 0x77C0 );	/* Address 0x1F */
	LDI	30656,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	69
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	70
;>>>> 		WriteEepromWord( i, 0x8000 );	/* Address 0x23 */
	LDI	@CONST+7,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	71
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	72
;>>>> 		WriteEepromWord( i, 0x1 );	/* DRAM size */
	LDI	1,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	73
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	74
;>>>> 		WriteEepromWord( i, 0 );	/* Daughter present */
	LDI	0,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	75
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	76
;>>>> 		WriteEepromWord( i, 16 );	/* SRAM1 size */
	LDI	16,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	77
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	78
;>>>> 		WriteEepromWord( i, 64 );	/* SRAM2 size */
	LDI	64,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	79
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	80
;>>>> 		WriteEepromWord( i, 256 );	/* SRAM3 size */
	LDI	256,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
	.line	81
;>>>> 		i += 2;
	ADDI	2,*+FP(1),R0
	STI	R0,*+FP(1)
	.line	82
	LDI	64,R1
	PUSH	R1
	PUSH	R0
	CALL	_WriteEepromWord
	SUBI	2,SP
EPI0_2:
	.line	82
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	90,000000000H,1

	.sym	_WriteEepromWord,_WriteEepromWord,32,2,0
	.globl	_WriteEepromWord

	.func	94
;>>>> 	void WriteEepromWord( int WordAddress, unsigned long data )
******************************************************
* FUNCTION DEF : _WriteEepromWord
******************************************************
_WriteEepromWord:
	PUSH	FP
	LDI	SP,FP
	ADDI	1,SP
	.sym	_WordAddress,-2,4,9,32
	.sym	_data,-3,15,9,32
	.sym	_i,1,4,1,32
	.line	2
;>>>> 		int i;
	.line	5
;>>>> 		for( i=0 ; i < 2 ; i++, data >>= 8 )
	STIK	0,*+FP(1)
	CMPI	2,*+FP(1)
	BGE	EPI0_3
L1:
	.line	6
;>>>> 			WriteEeprom( WordAddress++, data & 0xff ); 
	LDI	*-FP(3),R0
	AND	0ffh,R0
	PUSH	R0
	LDI	*-FP(2),R0
	ADDI	1,R0,R1
	STI	R1,*-FP(2)
	SUBI	1,R1
	PUSH	R1
	CALL	_WriteEeprom
	SUBI	2,SP
	.line	5
	ADDI	1,*+FP(1),R0
	STI	R0,*+FP(1)
	LDI	*-FP(3),R1
	LSH	-8,R1,R2
	STI	R2,*-FP(3)
	CMPI	2,R0
	BLT	L1
EPI0_3:
	.line	7
	LDI	*-FP(1),R1
	LDI	*FP,FP
	SUBI	3,SP
	B	R1
	.endfunc	100,000000000H,1
******************************************************
* DEFINE CONSTANTS                                   *
******************************************************
	.bss	CONST,8
	.sect	".cinit"
	.word	8,CONST
	.word 	65280            ;0
	.word 	61681            ;1
	.word 	65023            ;2
	.word 	63504            ;3
	.word 	57360            ;4
	.word 	57364            ;5
	.word 	60414            ;6
	.word 	32768            ;7
******************************************************
* UNDEFINED REFERENCES                               *
******************************************************
	.globl	_WriteEeprom
	.end
