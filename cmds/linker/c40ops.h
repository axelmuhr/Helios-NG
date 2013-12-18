
/* C compiler file c40/ops.h :  Copyright (C) Perihelion Software Ltd., 1991. */
/* version 1 */

#ifndef _c40ops_LOADED
#define _c40ops_LOADED 1

/*
 * XXX -  NC
 *
 * C40 opcodes
 * Version: 1
 */

/* 2-bit binary abbreviations */

#define B_00			0x00L
#define B_01			0x01L
#define B_10			0x02L
#define B_11			0x03L


/* 3-bit binary abbreviations */

#define B_000			0x00L
#define B_001			0x01L
#define B_010			0x02L
#define B_011			0x03L
#define B_100			0x04L
#define B_101			0x05L
#define B_110			0x06L
#define B_111			0x07L


/* 4-bit binary abbreviations */

#define B_0000			0x00L
#define B_0001			0x01L
#define B_0010			0x02L
#define B_0011			0x03L
#define B_0100			0x04L
#define B_0101			0x05L
#define B_0110			0x06L
#define B_0111			0x07L
#define B_1000			0x08L
#define B_1001			0x09L
#define B_1010			0x0aL
#define B_1011			0x0bL
#define B_1100			0x0cL
#define B_1101			0x0dL
#define B_1110			0x0eL
#define B_1111			0x0fL


/* 5-bit binary abbreviations */

#define B_00000			0x00L
#define B_00001			0x01L
#define B_00010			0x02L
#define B_00011			0x03L
#define B_00100			0x04L
#define B_00101			0x05L
#define B_00110			0x06L
#define B_00111			0x07L
#define B_01000			0x08L
#define B_01001			0x09L
#define B_01010			0x0aL
#define B_01011			0x0bL
#define B_01100			0x0cL
#define B_01101			0x0dL
#define B_01110			0x0eL
#define B_01111			0x0fL
#define B_10000			0x10L
#define B_10001			0x11L
#define B_10010			0x12L
#define B_10011			0x13L
#define B_10100			0x14L
#define B_10101			0x15L
#define B_10110			0x16L
#define B_10111			0x17L
#define B_11000			0x18L
#define B_11001			0x19L
#define B_11010			0x1aL
#define B_11011			0x1bL
#define B_11100			0x1cL
#define B_11101			0x1dL
#define B_11110			0x1eL
#define B_11111			0x1fL


/* 6-bit binary abbreviations */

#define B_000000		0x00L
#define B_000001		0x01L
#define B_000010		0x02L
#define B_000011		0x03L
#define B_000100		0x04L
#define B_000101		0x05L
#define B_000110		0x06L
#define B_000111		0x07L
#define B_001000		0x08L
#define B_001001		0x09L
#define B_001010		0x0aL
#define B_001011		0x0bL
#define B_001100		0x0cL
#define B_001101		0x0dL
#define B_001110		0x0eL
#define B_001111		0x0fL
#define B_010000		0x10L
#define B_010001		0x11L
#define B_010010		0x12L
#define B_010011		0x13L
#define B_010100		0x14L
#define B_010101		0x15L
#define B_010110		0x16L
#define B_010111		0x17L
#define B_011000		0x18L
#define B_011001		0x19L
#define B_011010		0x1aL
#define B_011011		0x1bL
#define B_011100		0x1cL
#define B_011101		0x1dL
#define B_011110		0x1eL
#define B_011111		0x1fL
#define B_100000		0x20L
#define B_100001		0x21L
#define B_100010		0x22L
#define B_100011		0x23L
#define B_100100		0x24L
#define B_100101		0x25L
#define B_100110		0x26L
#define B_100111		0x27L
#define B_101000		0x28L
#define B_101001		0x29L
#define B_101010		0x2aL
#define B_101011		0x2bL
#define B_101100		0x2cL
#define B_101101		0x2dL
#define B_101110		0x2eL
#define B_101111		0x2fL
#define B_110000		0x30L
#define B_110001		0x31L
#define B_110010		0x32L
#define B_110011		0x33L
#define B_110100		0x34L
#define B_110101		0x35L
#define B_110110		0x36L
#define B_110111		0x37L
#define B_111000		0x38L
#define B_111001		0x39L
#define B_111010		0x3aL
#define B_111011		0x3bL
#define B_111100		0x3cL
#define B_111101		0x3dL
#define B_111110		0x3eL
#define B_111111		0x3fL

/* (a few( 7-bit binary abbreviations */

#define B_0110000		0x30L

/* (some) 8-bit binary abbreviations */

#define B_01100000		0x60L
#define B_01100001		0x61L
#define B_01100010		0x62L
#define B_01100011		0x63L
#define B_01100100		0x64L
#define B_01100101		0x65L
#define B_01100110		0x66L
#define B_01100111		0x67L
#define B_01101000		0x68L
#define B_01101001		0x69L
#define B_01101010		0x6aL
#define B_01101011		0x6bL
#define B_01101100		0x6cL
#define B_01101101		0x6dL
#define B_01101110		0x6eL
#define B_01101111		0x6fL
#define B_01110000		0x70L
#define B_01110001		0x71L
#define B_01110010		0x72L
#define B_01110011		0x73L
#define B_01110100		0x74L
#define B_01110101		0x75L
#define B_01110110		0x76L
#define B_01110111		0x77L
#define B_01111000		0x78L

/* condition_codes [usually bits 16 - 21] */

#define C_U		B_00000		/* unconditional */
#define C_LO		B_00001		/* less than (unsigned) */
#define C_LS		B_00010		/* less than or equal to (unsigned) */
#define C_HI		B_00011		/* greater than (unsigned) */
#define C_HS		B_00100		/* greater than or equal to (unsigned) */
#define C_EQ		B_00101		/* equal to */
#define C_NE		B_00110		/* not equal to */
#define C_LT		B_00111		/* less than (signed) */
#define C_LE		B_01000		/* less than or equal to (signed) */
#define C_GT		B_01001		/* greater than (signed) */
#define C_GE		B_01010		/* greater than or equal to (signed) */
/* #define C_		B_01011		 */
#define C_NV		B_01100		/* no overflow  */
#define C_V		B_01101		/*    overflow  */
#define C_NUF		B_01110		/* no underflow */
#define C_UF		B_01111		/*    underflow */
#define C_NLV		B_10000		/* no latched overflow */
#define C_LV		B_10001		/*    latched overflow */
#define C_NLUF		B_10010		/* no latched floating-point underflow */
#define C_LUF		B_10011		/*    latched floating-point underflow */
#define C_ZUF		B_10100		/* zero or floating-point underflow */
/* #define C_		B_10101		 */
/* #define C_		B_10110		 */
/* #define C_		B_10111		 */
/* #define C_		B_11000		 */
/* #define C_		B_11001		 */
/* #define C_		B_11010		 */
/* #define C_		B_11011		 */
/* #define C_		B_11100		 */
/* #define C_		B_11101		 */
/* #define C_		B_11110		 */
/* #define C_		B_11111		 */

/* aliases */

#define C_Z		C_EQ		/* zero */
#define C_NZ		C_NE		/* not zero */
#define C_P		C_GT		/* positive */
#define C_N		C_L		/* negative */
#define C_NN		C_GE		/* not negative */
#define C_NC		C_HS		/* not carry */
#define C_C		C_LO		/* carry */

/* diadic sequential op-codes [bits 23 - 28] (bits 29 - 31 => 000) */

#define OP_ABSF		B_000000	/* absolute value of floating point */
#define OP_ABSI		B_000001	/* absolute value of integer */
#define OP_ADDC		B_000010	/* add integer with carry */
#define OP_ADDF		B_000011	/* add foating-point */
#define OP_ADDI		B_000100	/* add integer, no carry */
#define OP_AND		B_000101	/* bitwise AND */
#define OP_ANDN		B_000110	/* bitwise (NOT src) AND dst */
#define OP_ASH		B_000111	/* arithmetic shift */
#define OP_CMPF		B_001000	/* compare floating point */
#define OP_CMPI		B_001001	/* compare integer */
#define OP_FIX		B_001010	/* floating point to integer conversion */
#define OP_FLOAT	B_001011	/* integer to floating point conversion */
#define OP_IDLE		B_001100	/* idle until interrupt */
#define OP_LDE		B_001101	/* load floating point exponent */
#define OP_LDF		B_001110	/* load floating point */
#define OP_LDFI		B_001111	/* load floating point, interlocked */
#define OP_LDI		B_010000	/* load integer */
#define OP_LDII		B_010001	/* load integer, interlocked */
#define OP_LDM		B_010010	/* load floating point mantissa */
#define OP_LSH		B_010011	/* logical shift */
#define OP_MPYF		B_010100	/* multiply, floating point */
#define OP_MPYI		B_010101	/* multiply, integer */
#define OP_NEGB		B_010110	/* negate integer with borrow */
#define OP_NEGF		B_010111	/* negate floating point */
#define OP_NEGI		B_011000	/* negate intger, no borrow */
#define OP_NOP		B_011001	/* nop */
#define OP_NORM		B_011010	/* normalise */
#define OP_NOT		B_011011	/* bitwise NOT */
#define OP_POP		B_011100	/* pull intger off stack */
#define OP_POPF		B_011101	/* pull floating point off stack */
#define OP_PUSH		B_011110	/* push intger onto stack */
#define OP_PUSHF	B_011111	/* push floating point onto stack */
#define OP_OR		B_100000	/* bitwise OR */
/* #define OP_		B_100001	*//*  */
#define OP_RND		B_100010	/* round, floating point */
#define OP_ROL		B_100011	/* rotate left */
#define OP_ROLC		B_100100	/* rotate left, through carry */
#define OP_ROR		B_100101	/* rotate right */
#define OP_RORC		B_100110	/* rotate right through carry */
#define OP_RPTS		B_100111	/* repeat single instruction */
#define OP_STF		B_101000	/* store floating point */
#define OP_STFI		B_101001	/* store floating point, interlocked */
#define OP_STI		B_101010	/* store integer */
#define OP_STIK		B_101010	/* store integer (constant) */
#define OP_STII		B_101011	/* store integer, interlocked */
#define OP_SIGI		B_101100	/* signal, interlocked */
#define OP_SUBB		B_101101	/* subtract integer with borrow */
#define OP_SUBC		B_101110	/* subtract integer, maybe */
#define OP_SUBF		B_101111	/* subtract floating point */
#define OP_SUBI		B_110000	/* subtract integer, no borrow */
#define OP_SUBRB	B_110001	/* subtract integer with borrow, reversed */
#define OP_SUBRF	B_110010	/* subtract floating point, reversed */
#define OP_SUBRI	B_110011	/* subtract integer, no borrow, reversed */
#define OP_TSTB		B_110100	/* test bit fields */
#define OP_XOR		B_110101	/* bitwise exclusive or */
#define OP_IACK		B_110110	/* interrupt acknowledge */
#define OP_TOIEEE	B_110111	/* convert internal floating point to IEEE format */
#define OP_FRIEEE	B_111000	/* convert IEEE to internal floating point format */
#define OP_RSQRF	B_111001	/* reciprocal square root floating point */
#define OP_RCPF		B_111010	/* reciprocal floating point */
#define OP_MPYSHI	B_111011	/* multiply   signed integer, high word of result */
#define OP_MPYUHI	B_111100	/* multiply unsigned integer, high word of result */
#define OP_LDA		B_111101	/* load address register */
#define OP_LDPK		B_111110	/* load DP register */
#define OP_LDHI		B_111111	/* load top 16 bits (immediate) */


/* triadic sequential op-codes [bits 23 - 27] (bits 29 - 31 => 001, bit 28 => 0 (type 1) or => 1 (type 2)) */

#define OP_ADDC3	B_00000		/* add integer with carry */
#define OP_ADDF3	B_00001		/* add floating point */
#define OP_ADDI3	B_00010		/* add integer, no carry */
#define OP_AND3		B_00011		/* bitwise AND */
#define OP_ANDN3	B_00100		/* (NOT src) AND dst */
#define OP_ASH3		B_00101		/* arithmetic shift */
#define OP_CMPF3	B_00110		/* floating point compare */
#define OP_CMPI3	B_00111		/* integer compare */
#define OP_LSH3		B_01000		/* logical shift */
#define OP_MPYF3	B_01001		/* floating point multiply */
#define OP_MPYI3	B_01010		/* integer multiply */
#define OP_OR3		B_01011		/* bitwise OR */
#define OP_SUBB3	B_01100		/* integer subtract with borrow */
#define OP_SUBF3	B_01101		/* floating point subtact */
#define OP_SUBI3	B_01110		/* integer subtract, no borrow */
#define OP_TSTB3	B_01111		/* test bit fields */
#define OP_XOR3		B_10000		/* bitwise XOR */
#define OP_MPYSHI3	B_10001		/*   signed integer multiply, high word of result */
#define OP_MPYUHI3	B_10010		/* unsigned integer multiply, high word of result */
/* #define OP_		B_10011		*//* */
/* #define OP_		B_10100		*//* */
/* #define OP_		B_10101		*//* */
/* #define OP_		B_10110		*//* */
/* #define OP_		B_10111		*//* */
/* #define OP_		B_11000		*//* */
/* #define OP_		B_11001		*//* */
/* #define OP_		B_11010		*//* */
/* #define OP_		B_11011		*//* */
/* #define OP_		B_11100		*//* */
/* #define OP_		B_11101		*//* */
/* #define OP_		B_11110		*//* */
/* #define OP_		B_11111		*//* */


/* parallel op codes [bits 25 - 29] (bits 30 - 31 => 11, bits 19 - 21 => 000) (first style) */

#define OP_STF_STF	B_00000		/* store floating point              AND store floating point */
#define OP_STI_STI	B_00001		/* store integer                     AND store integer        */
#define OP_LDF_LDF	B_00010		/* load floating point               AND load  floating point */
#define OP_LDI_LDI	B_00011		/* load integer                      AND load  integer        */
#define OP_ABSF_STF	B_00100		/* absolute floating point           AND store floating point */
#define OP_ABSI_STI	B_00101		/* absolute integer                  AND store integer        */
#define OP_ADDF3_STF	B_00110		/* triadic add floating point        AND store floating point */
#define OP_ADDI3_STI	B_00111		/* triadic add integer               AND store integer        */
#define OP_AND3_STI	B_01000		/* triadic bitwise and               AND store integer        */
#define OP_ASH3_STI	B_01001		/* triadic arithmetic shift          AND store integer        */
#define OP_FIX_STI	B_01010		/* convert floating point to integer AND store integer        */
#define OP_FLOAT_STF	B_01011		/* convert integer to floating point AND store floating point */
#define OP_LDF_STF	B_01100		/* load floating point               AND store floating point */
#define OP_LDI_STI	B_01101		/* load integer                      AND store integer        */
#define OP_LSH3_STI	B_01110		/* triadic logical shift             AND store integer        */
#define OP_MPYF3_STF	B_01111		/* triadic floating point multiply   AND store floating point */
#define OP_MPYI3_STI	B_10000		/* triadic integer multiply          AND store integer        */
#define OP_NEGF_STF	B_10001		/* negate floating point             AND store floating point */
#define OP_NEGI_STI	B_10010		/* negate integer                    AND store integer        */
#define OP_NOT_STI	B_10011		/* bitwise NOT                       AND store integer        */
#define OP_OR3_STI	B_10100		/* traidic bitwise OR                AND store integer        */
#define OP_SUBF3_STF	B_10101		/* triadic floating point subtract   AND store floating point */
#define OP_SUBI3_STI	B_10110		/* triadic integer subtract          AND store integer        */
#define OP_XOR3_STI	B_10111		/* triadic bitwise XOR               AND store integer        */
#define OP_TOIEEE_STF	B_11000		/* convert to IEEE format            AND store floating point */
#define OP_FRIEEE_STF	B_11001		/* convert from IEEE format          AND store floating point */
/* #define OP_		B_11010		*//* */
/* #define OP_		B_11011		*//* */
/* #define OP_		B_11100		*//* */
/* #define OP_		B_11101		*//* */
/* #define OP_		B_11110		*//* */
/* #define OP_		B_11111		*//* */


/* parallel op codes [bits 26 - 27] (bits 28 - 31 => 1000) (second style) */

#define OP_MPYF3_ADDF3	B_00		/* triadic floating point multiply   AND triadic floating point addition */
#define OP_MPYF3_SUBF3	B_01		/* triadic floating point multiply   AND triadic floating point subtraction */
#define OP_MPYI3_ADDI3	B_10		/* triadic integer multiply          AND triadic integer addition */
#define OP_MPYI3_SUBI3	B_11		/* triadic integer multiply          AND triadic integer subtract */


/* non-word data transfers [bits 24 - 27] (bits 28 - 31 => 1011) */
/* note some instructions only use bits 25 - 27, with bit 24 being part of another field */

#define FUNC_LOAD	B_1011		/* pseudo op used as top nibble of instruction */

#define OP_LB		B_0000		/* load   (signed) byte */
/* #define 		B_0001		*//* ditto */
#define OP_LBU		B_0010		/* load (unsigned) byte */
/* #define		B_0011		*//* ditto */
#define OP_LWL		B_0100		/* load word, left shifted */
/* #define 		B_0101		*//* ditto */
#define OP_LWR		B_0110		/* load word, right shifted */
/* #define 		B_0111		*//* ditto */
#define OP_MB		B_1000		/* merge byte, left shifted */
/* #define 		B_1001		*//* ditto */
#define OP_LH		B_1010		/* load half word sign extended */
#define OP_LHU		B_1011		/* load half word, unsigned */
#define OP_MH		B_1100		/* merge half word, left shifted */
/* #define 		B_1101		*//* unused */
/* #define 		B_1110		*//* unused */
/* #define 		B_1111		*//* unused */


/* flow control and miscellaneous op codes (bits 30 - 31 => 01) */

/* these are currently un-defined ... */


#define OP_LDIc		B_0101		/* conditionally load integer */
#define OP_BR		B_01100000	/* branch unconditionally */
#define OP_BRD		B_01100001	/* unconditional branch, delayed */
#define OP_CALL		B_01100010	/* unconditional call to subroutine */
#define OP_LAJ		B_01100011	/* unconditional link to subroutine */
#define OP_RPTB		B_01100100	/* repeat block (immeadiate), PC relative */
#define OP_BRcr		B_01101000	/* branch conditionally, register relative */
#define OP_BRcrD	B_01101000	/* branch conditionally delayed, register relative */
#define OP_BRc		B_01101010	/* branch conditionally, PC relative */
#define OP_BRcD		B_01101010	/* branch conditionally, delayed, PC relative */
#define OP_CALLcr	B_01110000	/*   conditional call to subroutine, register relative */
#define OP_LAJcr	B_01110000	/*   conditional link and jump, register relative */
#define OP_CALLc	B_01110010	/*   conditional call to subroutine, PC relative */
#define OP_LAJc		B_01110010	/*   conditional link and jump, PC relative */
#define OP_RETSc	B_01111000	/*   conditional return from subroutine */

#ifdef NEVER
#define OP_LDFc		B_00		/* conditionally load floating point */
#define OP_RPTBD	B_100101	/* repeat block (immeadiate), delayed */
#define OP_SWI		B_100110	/* software interrupt */
#define OP_B		B_1010x0000	/*   conditional branch */
#define OP_BAF		B_1010x0101	/*   conditional branch, annul execute phase if FALSE */
#define OP_BAT		B_1010x0011	/*   conditional branch, annul execute phase if TRUE  */
#define OP_DB		B_1011xxxx0	/*   conditional decrement and branch */
#define OP_DBD		B_1011xxxx1	/*   conditional decrement and branch, delayed */
#define OP_TRAP		B_110100000	/*   conditional trap */
#define OP_LAT		B_110100100	/*   conditional link and trap */
#define OP_LDEP		B_110110000	/* load register from expansion register */
#define OP_LDPE		B_110110100	/* load expansion register from register */
#define OP_RETI		B_111000000	/*   conditional return from interrupt/trap */
#define OP_RETD		B_111000001	/*   conditional return from interrupt/trap, delayed */
#define OP_RPTB		B_1110010	/* repeat block (register)   */
#define OP_RPTBD	B_1110011	/* repeat block (register),   delayed */
#endif /* NEVER */

/*
 * addressing modes
 *
 * NB/ these values have been carefully chosen to
 * match the G field of C40 op codes, SO DO NOT ALTER
 */

#define ADDR_MODE_REGISTER	0
#define ADDR_MODE_DIRECT	1
#define ADDR_MODE_INDIRECT	2
#define ADDR_MODE_IMMEDIATE	3
#define ADDR_MODE_PCREL		4

#endif /* c40ops_LOADED */

/* end of c40/ops.h */
