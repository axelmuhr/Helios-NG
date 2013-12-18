#ifndef lint
char yysccsid[] = "@(#)yaccpar	1.4 (Berkeley) 02/25/90";
#endif
#line 37 "gasm.y"

/* Include Files: */


 
/*
 * File:	gasm.h
 * Subsystem:	Generic Assembler
 * Author:	P.A.Beskeen
 * Date:	Aug '91
 *
 * Description: include file for generic Helios assembler
 *
 * RcsId: $Id: gasm.h,v 1.9 1993/07/12 16:15:56 nickc Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */





/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * User-visible pieces of the ANSI C standard I/O package.
 */




#pragma ident	"@(#)stdio.h	1.23	93/08/06 SMI"	

/*	Copyright (c) 1993, by Sun Microsystems, Inc.	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF SMI	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/




#pragma ident	"@(#)feature_tests.h	1.5	93/04/26 SMI"



/*
 * 	Values of _POSIX_C_SOURCE
 *
 *		undefined	not a POSIX compilation
 *			1	POSIX.1-1990 compilation
 *			2	POSIX.2-1992 compilation
 *		  1993xxL	POSIX.4-1993 compilation
 */











typedef unsigned int	size_t;


typedef long	fpos_t;

















































typedef struct	/* needs to be binary-compatible with old versions */
{

	int		_cnt;	/* number of available characters in buffer */
	unsigned char	*_ptr;	/* next character from/to here in buffer */

	unsigned char	*_base;	/* the buffer */
	unsigned char	_flag;	/* the state of the stream */
	unsigned char	_file;	/* UNIX System file descriptor */
} FILE;


extern FILE		__iob[20	];

extern FILE		*_lastbuf;
extern unsigned char	*_bufendtab[];

extern unsigned char	 _sibuf[], _sobuf[];




extern int	remove(const char *);
extern int	rename(const char *, const char *);
extern FILE	*tmpfile(void);
extern char	*tmpnam(char *);

extern int	fclose(FILE *);
extern int	fflush(FILE *);
extern FILE	*fopen(const char *, const char *);
extern FILE	*freopen(const char *, const char *, FILE *);
extern void	setbuf(FILE *, char *);
extern int	setvbuf(FILE *, char *, int, size_t);
/* PRINTFLIKE2 */
extern int	fprintf(FILE *, const char *, ...);
/* SCANFLIKE2 */
extern int	fscanf(FILE *, const char *, ...);
/* PRINTFLIKE1 */
extern int	printf(const char *, ...);
/* SCANFLIKE1 */
extern int	scanf(const char *, ...);
/* PRINTFLIKE2 */
extern int	sprintf(char *, const char *, ...);
/* SCANFLIKE2 */
extern int	sscanf(const char *, const char *, ...);
extern int	vfprintf(FILE *, const char *, void *);
extern int	vprintf(const char *, void *);
extern int	vsprintf(char *, const char *, void *);
extern int	fgetc(FILE *);
extern char	*fgets(char *, int, FILE *);
extern int	fputc(int, FILE *);
extern int	fputs(const char *, FILE *);
extern int	getc(FILE *);
extern int	getchar(void);
extern char	*gets(char *);
extern int	putc(int, FILE *);
extern int	putchar(int);
extern int	puts(const char *);
extern int	ungetc(int, FILE *);
extern size_t	fread(void *, size_t, size_t, FILE *);
	
#pragma int_to_unsigned fread
extern size_t	fwrite(const void *, size_t, size_t, FILE *);
	
#pragma int_to_unsigned fwrite
extern int	fgetpos(FILE *, fpos_t *);
extern int	fseek(FILE *, long, int);
extern int	fsetpos(FILE *, const fpos_t *);
extern long	ftell(FILE *);
extern void	rewind(FILE *);
extern void	clearerr(FILE *);
extern int	feof(FILE *);
extern int	ferror(FILE *);
extern void	perror(const char *);


extern int	__filbuf(FILE *);
extern int	__flsbuf(int, FILE *);














/*
 * The following are known to POSIX and XOPEN, but not to ANSI-C.
 */


/*
 * The following are known to XOPEN, but not to ANSI-C or POSIX.
 */








/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/




#pragma ident	"@(#)stdlib.h	1.20	93/03/10 SMI"	





typedef	struct {
	int	quot;
	int	rem;
} div_t;

typedef struct {
	long	quot;
	long	rem;
} ldiv_t;







typedef long	uid_t;










typedef long wchar_t;




extern unsigned char	__ctype[];



extern double atof(const char *);
extern int atoi(const char *);
extern long int atol(const char *);
extern double strtod(const char *, char **);
extern long int strtol(const char *, char **, int);
extern unsigned long int strtoul(const char *, char **, int);

extern int rand(void);
extern void srand(unsigned int);

extern void *calloc(size_t, size_t);
extern void free(void *);
extern void *malloc(size_t);
extern void *realloc(void *, size_t);

extern void abort(void);
extern int atexit(void (*)(void));
extern void exit(int);
extern char *getenv(const char *);
extern int system(const char *);

extern void *bsearch(const void *, const void *, size_t, size_t,
	int (*)(const void *, const void *));
extern void qsort(void *, size_t, size_t,
	int (*)(const void *, const void *));

extern int abs(int);
extern div_t div(int, int);
extern long int labs(long);
extern ldiv_t ldiv(long, long);

extern int mbtowc(wchar_t *, const char *, size_t);
extern int mblen(const char *, size_t);
extern int wctomb(char *, wchar_t);

extern size_t mbstowcs(wchar_t *, const char *, size_t);
extern size_t wcstombs(char *, const wchar_t *, size_t);









/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/




#pragma ident	"@(#)string.h	1.13	93/03/15 SMI"	











extern void *memcpy(void *, const void *, size_t);
extern void *memmove(void *, const void *, size_t);
extern char *strcpy(char *, const char *);
extern char *strncpy(char *, const char *, size_t);

extern char *strcat(char *, const char *);
extern char *strncat(char *, const char *, size_t);

extern int memcmp(const void *, const void *, size_t);
extern int strcmp(const char *, const char *);
extern int strcoll(const char *, const char *);
extern int strncmp(const char *, const char *, size_t);
extern size_t strxfrm(char *, const char *, size_t);

extern void *memchr(const void *, int, size_t);
extern char *strchr(const char *, int);
extern size_t strcspn(const char *, const char *);
	
#pragma int_to_unsigned strcspn
extern char *strpbrk(const char *, const char *);
extern char *strrchr(const char *, int);
extern size_t strspn(const char *, const char *);
	
#pragma int_to_unsigned strspn
extern char *strstr(const char *, const char *);
extern char *strtok(char *, const char *);


extern void *memset(void *, int, size_t);
extern char *strerror(int);
extern char *strsignal(int);
extern size_t strlen(const char *);
	
#pragma int_to_unsigned strlen













/* ********************************************************************
 * Floating point support manifests.
 *
 * Floating point support requires following typedefs (from mip/host.h).
 * These defn's may need alteration for obscure host machines. They are
 * defined at this point as they may be referenced in the processor specific
 * header files.
 */



typedef long int            int32;
typedef long unsigned int   unsigned32;
typedef short int           int16;
typedef short unsigned int  unsigned16;
typedef signed char         int8;
typedef unsigned char       unsigned8;

typedef int                 bool;



	/* IEEE 64 bit integer representation (from mip/defs.h)
	 * The following types describe the representation of floating-point
	 * values by the compiler in both binary and source-related forms.
	 * The order of fields in DbleBin is exploited only in object-code
	 * formatters and assembly code generators.
	 */
	typedef struct {

	  int32 msd,lsd;                /* e.g. ARM, 370 (not really IEEE) */

	} DbleBin;

	typedef struct FloatBin {
	  int32 val;
	} FloatBin;

	typedef	DbleBin	Dble ;



/* ********************************************************************
 * CPU specific header information
 */

/*
 * File:	hdr_C40.h
 * Author:	P.A.Beskeen
 * Date:	Sept '91
 *
 * Description: TMS320C40 specific header file for generic Helios assembler
 *
 * RcsId: $Id: hdr_C40.h,v 1.5 1993/06/22 16:58:53 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */







/*
 * File:	binary.h
 * Author:	A. Program (*Author P.A.Beskeen (idea stolen from NickC!))
 * Date:	Originally Sept '91
 *
 * Description: Handy set of binary definitions.
 *
 *		Helps to get around C's lack of binary constants.
 *		This file was automatically generated - so believe it!
 *
 * RcsId: $Id: binhdr.c,v 1.1 1992/03/12 21:16:01 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */






/* Binary Constants: */


/* Bit 0: */



/* Bits 1-0: */





/* Bits 2-0: */









/* Bits 3-0: */

















/* Bits 4-0: */

































/* Bits 5-0: */

































































/* Bits 6-0: */

































































































































/* Bits 7-0: */

































































































































































































































































/* Bits 8-0: */

































































































































































































































































































































































































































































































































/* Bits 10-0: */



































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































/* binary.h */



/* ********************************************************************	*/
/* *** 'C40 Processor Specific Structures *****************************	*/


/* ********************************************************************	*/
/* Mnemonic Structure							*/
/*
 * The Mnemonic structure is attached to all Symbol structures in the hash
 * table that correspond to cpu specific mneumonic keywords
 * (type = HT_MNEMONIC)
 *
 * The decoding of Condition codes is done the C40 lexer, not the parser.
 * This is because they are entered as part of the mnemonic keyword and it is
 * therfore easier to decode them at this point. The condcode item in the
 * Mnemonic structure is used to hold the 'C40 condition field value
 * for any conditional instruction's token.
 *
 * The condcode field is also used to hold the byte/half-word selectors/shifts
 * for the: LBb, LUBb, LHw, LHUw, LWLct, LWRct, MBct and MHct instructions.
 *
 * The condcode field can also be used to store the register selection field
 * for all recognised 'C40 registers: R0-11, AR0-7, DP, IR0-1, BK, SP, ST,
 * DIE, IIE, IIF, RS, RE, RC, IVTP and TVTP.
 *
 */

typedef struct Mnemonic {
	char		*name;		/* mneumonics text name		*/
	int		token;		/* parser token number 		*/
	unsigned	diadic;		/* diadic instruction template	*/
	unsigned	triadic;	/* triadic instruction template	*/
	unsigned	par_st;		/* parallel store template	*/
} Mnemonic;


/* ********************************************************************	*/
/* Instruction Structure						*/
/*									*/
/* Instruction is a Target CPU dependent structure that defines the	*/
/* machine opcode to output. It may also contain expressions that need	*/
/* to be evaluated and combined with the instruction before it is output*/


typedef struct Instruction {
	int			opcode;		/* The opcode to output	    */
	struct Expression	*optexpr;	/* Optionally insert	    */
						/* expr into the opcode	    */
	int			combine;	/* How to combine the       */
						/* expr with the opcode	    */
	struct Expression	*optexpr2;	/* Second src operand	    */
	int			combine2;	/* How to combine 2nd src   */
} Instruction;


/* ********************************************************************	*/
/* Combine Commands							*/


/* *** Diadic Combine Commands ****************************************	*/

/* Most these commands will only use combine, NOT combine2 */

/* Register addressing doesn't need to be checked */


/* Check immediate <= 16 bits and insert int bits 0-15 */


/* Check immediate <= 16 bits and insert int bits 0-15 */


/* Shift immediate >> 16 and insert int bits 0-15 */


/* Check disp <= 8 bits and insert into bits 0-8 */


/* Check direct offset <= 16 bits and insert into bits 0-15 */


/* Check immediate is <= 5 bits and insert into bits 16-20 */



/* *** Triadic Combine Commands ***************************************	*/

/* Notes arg position by: _A1 = bits 0-7, _A2 = bits 8-15 */

/* Triadic type 1 indirect addressing, args 1 and 2 */
/* Check displacement is only 1 or 0. */
/* If displacement is 0, then convert mod to 0b11000 (*+ARn(0) -> *ARn) */



/* Triadic type 2 indirect addressing, args 1 and 2 */
/* Check displacement is <= 5 bits and insert disp. into bits 11-15 or 3-7 */



/* Triadic type 2 immediate addressing, arg 1 only */
/* Check immediate <= eight bits and insert into bits 0-7 of instruction */




/* *** Parallel Store Combine Commands ********************************	*/

/* Check displacement is only 1 or 0 */
/* If displacement is 0, then convert mod to 0b11000 (*+ARn(0) -> *ARn) */
		/* 1st src operand: mod = bits 3-7	*/

		/* Store dest operand: mod = bits 11-15	*/



/* *** Parallel Multiply Add/Sub Combine Commands *********************	*/

/* Check displacement is only 1 or 0 */
/* If displacement is 0, then convert mod to 0b11000 (*+ARn(0) -> *ARn) */




/* *** Branch Combine Commands ****************************************	*/

/* If operation is a delayed branch then subtract 3 from the offset */
/* before inserting it into the instruction, otherwise subtract 1 */
/* These combine commands will only ever occur in the first ->combine */

/* Check PC relative offset < 16 bits and insert into bits 0-15 */
/* Used in conditional branch instructions */
/* Bit 21 specifies if it is a delayed branch */


/* Check PC relative offset < 24 bits and insert into bits 0-23 */
/* Used in UNconditional branch instructions and RPTB(D) */
/* Bit 24 specifies if it is a delayed branch */


/* Check immediate value is <= 9 bits and insert into bits 0-8 */



/* ********************************************************************	*/
/* Register values							*/

/* Standard register file regs: */

































/* Expansion register file regs: */




/* ********************************************************************	*/
/* Macros to check that the correct registers are being used.		*/
/* They raise a parser error if this is not the case.			*/











/* ********************************************************************	*/
/* Condition codes [usually bits 16 - 21]				*/

/*
 * For all possible conditions, define the binary code to entered into the
 * cond field of conditional instructions.
 */














/* #define C_		CC(B_01011)	 */ /* UNSPECIFIED		*/










/* Unspecified condition codes */


/* aliases */










/* ********************************************************************	*/
/* Individual Instruction Opcode Definitions				*/
/* ********************************************************************	*/

/* ********************************************************************	*/
/* Diadic sequential op-codes [bits 23 - 28] (bits 29 - 31 => 000)	*/

/*(B_000 << 29) |*/


































/* #define OP_		DIA(B_100001)	*//* UNSPECIFIED		*/

					/* rotate 1 bit left		*/

					/* rot left, thru carry		*/

					/* rotate 1 bit right		*/

					/* rot right thru carry		*/

					/* repeat single instruction	*/






















/* @@@ NICKC got following two opcodes wrong */

					/* load sixteen bit unsigned imm.*/




/* ********************************************************************	*/
/* Triadic sequential op-codes [bits 23 - 27] (bits 29 - 31 => 001,	*/
/* bit 28 => 0 (type 1), or => 1 (type 2))				*/





















					/* high word of result		*/

					/* high word of result		*/




/* ********************************************************************	*/
/* Parallel store op codes [bits 25 - 29] (bits 30 - 31 => 11)		*/
/* (bits 19 - 21 => 000) 						*/




					/* AND store floating point	*/

					/* AND store integer		*/

					/* AND load  floating point	*/

					/* AND load  integer       	*/

					/* AND store floating point	*/

					/* AND store integer       	*/

					/* AND store floating point	*/

					/* AND store integer       	*/

					/* AND store integer       	*/

					/* AND store integer       	*/

					/* AND store integer       	*/

					/* AND store floating point	*/

					/* AND store floating point	*/

					/* AND store integer       	*/

					/* AND store integer       	*/

					/* AND store floating point	*/

					/* AND store integer       	*/

					/* AND store floating point	*/

					/* AND store integer       	*/

					/* AND store integer       	*/

					/* AND store integer       	*/

					/* AND store floating point	*/

					/* AND store integer       	*/

					/* AND store integer       	*/

					/* AND store floating point	*/

					/* AND store floating point	*/




/* ********************************************************************	*/
/* Parallel Multiply op codes [bits 26 - 29] (bits 30 - 31 => 10)	*/




					/* AND triadic floating point add  */

					/* AND triadic floating point sub  */

					/* AND triadic integer addition    */

					/* AND triadic integer sub	   */




/* ********************************************************************	*/
/* Part word data transfers [bits 25 - 27] (bits 28 - 31 => 1011)	*/
/* Note half word instructions only use bit 23, bit 24 being used to	*/
/* select a particular instruction. Byte instructions use bits 24 and	*/
/* 23 to select a particular byte.					*/




/* Byte loads */






/* Half word loads */







/* ********************************************************************	*/
/* Conditional Load Instructions 					*/





/* ********************************************************************	*/
/* Flow control and miscellaneous opcodes (bits 29 - 31 => 011)		*/

/* Unconditional Branches */








/* Conditional Branches */


























/* Load/Store expansion regs */




/* Repeat Block */
/* Odd in that different addressing modes have completely different opcodes */
/* - so RPTBr is held in triadic position in mnemonic structure */






/* Software Interrupt */




/* ********************************************************************	*/
/* *** Addressing Mode Macros *****************************************	*/

/* *** General addressing mode (ld/st/diadic) macros ******************	*/

/* insert into G addr. mode selection field of instruction */


/* G field address mode selector values */





/* insert destination reg used in ld's, diadic and triadic ops into */
/* instruction template */


/* insert src operand in ld's and diadic ops into instruction template */


/* insert src indirect addressing mod field into instruction template */


/* insert source reg used in store ops into instruction template */


/* insert destination into instruction template */




/* *** Triadic Addressing Macros **************************************	*/

/* Addressing mode subtype selectors */



/* insert triadic addressing mode combination selection field */


/* addressing mode combinations used by triadic operations */










/* insert argument 1 into triadic instruction template */


/* insert argument 2 into triadic instruction template */



/* *** Parallel Store Addressing Macros *******************************	*/

/* insert arguments into parallel store instruction template */






/* insert arguments into parallel load instruction template */



/* insert arguments into parallel st || st instruction template */




/* *** Parallel Multiply Addressing Macros ****************************	*/

/* insert parallel multiply addressing mode combination selection field */


/* addressing mode combinations used by parallel multiply operations	*/
/* I = indirect, R = register, with operand order.			*/
/* The second two operands order has been swapped so that they reflect	*/
/* the ordering of SUB operands. Other operations are commutative, so	*/
/* this is not a problem						*/






/* insert parallel multiply operands ito instruction template */










/* *** Indirect addressing macros *************************************	*/
/* Used in creating the mod and ARn fields in indirect addressing	*/

/* move indirect addr. mod field to make space for address reg field */


/* insert indirect addr. address register field */
/* AR0 = 0 ... AR7 = 7 */


/* move indirect addr. displacement TYPE selector field within modifier field */


/* Checks if mod field is compatible with type 2 triadic addressing */
/* "*+ARn" is only mode allowed */




/* Exported variables */

/* holds current instruction being parsed */
extern	Instruction	*CurInstr;


/* Exported C40 specific functions */


/*
 * Conversion between C40 and IEEE format.
 */

extern int32 C40_32ToC40_16(int32);


	extern	int32 IEEE_32ToC40_32(int32);


extern	int32 IEEE_64ToC40_32(Dble);
extern	int32 IEEE_64ToC40_16(Dble);








/* ********************************************************************
 * Manifests
 */









/* ********************************************************************
 * Hash table size - MUST BE a prime number!
 *
 * Some primes are:
 *	19 31 41 53 61 71 83 97 101 151 199 251 307 401 503
 *	509 557 599 653 701 751 809 853 907 953
 *	1009 1499 1999 2503 3001 4001 4999 6007 7001 8009 9001
 *	10007 15013 30011 40009 50021 60013 65521
 */



/* ********************************************************************	*/
/* Define the different types of data held in the hash table and what	*/
/* should be returned to the parser.					*/







/* ********************************************************************	*/
/* Define what type of data is held in Expression or ConstList		*/
/* structures.								*/






/* ********************************************************************	*/
/* Return amount to add to addr to align it to the byte boundary 	*/
/* specified.								*/




/* ********************************************************************	*/
/* Return value 'n' rotated to the right by 'r' bits.			*/
/* BEWARE will not work for hosts that do arithmetic right shifts!	*/
 



/* ********************************************************************	*/
/* Check that value can fit into the specified number of bits.		*/














/* ********************************************************************	*/
/* Symbol structures held in chained entries from hash table. The hash	*/
/* table is used to hold symbols, pseudo-ops, opcodes and registers.	*/

typedef struct Symbol {
	struct Symbol	*next;	/* pointer to other entries chained at this */
				/* point, NULL at end of chain */
	int	what;		/* HT_PSEUDO/HT_MNEMONIC/HT_TOKENVAL/   */
				/* HT_LABEL/HT_IMPORT */
	int	referenced;	/* has label been referenced by anyone? */
	union {
		int	token;	/* token for HT_PSEUDO - returns token */
		int	value;	/* value of HT_LABEL - returns value */
			/* CPU specific structure - HT_MNEMONIC */
			/* returns token and structure pointer */
		struct Mnemonic *mnemonic;
			/* structure used for registers/patches/eccetera */
			/* Used with HT_TOKENVAL to return both token */
			/* and associated value to the parser. This */
			/* structure is defined by header_CPU.h */
		struct Common *misc;
			/* easy structure pointer assignment */
		void	*any;
	} type;
	char	name[1];	/* text name of symbol */
} Symbol;


/* ********************************************************************	*/
/* Common Structure							*/
/*									*/
/* Used for storing registers, patches and misc items in a symbol	*/
/* structure. Contains string and its associated token and value that	*/
/* are to be passed from lexer to parser.				*/

typedef struct Common {
	char	*name;		/* text string 				*/
	int	token;		/* parser token 			*/
	int	value;		/* integer value of token		*/
} Common;


/* ********************************************************************	*/
/* The ParseTreeItem structure						*/
/*									*/
/* The parse tree constructed in pass 1 by yyparse() is held as a 	*/
/* series of ParseTreeItems that specify what will be output in the	*/
/* object code. These items depending on their 'what' type, chain other */
/* structures that define the object codes exact contents. These other	*/
/* structures being returned by the relevant sections of the parser.	*/

typedef struct ParseTreeItem {
	struct ParseTreeItem	*next;	/* pointer to next parse tree item */

	int		linenum;	/* line num mnemonic started at */

	int		logicalPC;	/* logical PC at start of opcode */

	int		what;		/* a token value describing what type */
					/* of item this is and which of the */
					/* following is held (same token */
					/* values as used by the lex analyser)*/

	union {					/* used for directive: */
		int			num;	/* SPACE */
		struct FloatList	*flist;	/* IEEE32, IEEE64, C40FLOAT */
		struct NameList		*nlist;	/* CODETABLE/EXPORT/REF */
		struct Symbol		*symb;	/* LABEL */
		struct ConstList	*clist;	/* BYTE/WORD/SHORT/LINKERDATA */
		struct Expression	*expr;	/* MODULE */

						/* DATA/COMMON */
		struct {
			char			*name;
			struct Expression	*expr;
		} datacommon;

						/* PATCHINSTR */
		struct {
				/* the actual m/c specific patch */
			struct Expression	*mcpatch;
				/* the instruction to patch */
			struct Instruction	*instr;
				/* the patch to provide data */
				/* to insert into the instruction */
			struct Patch		*patch;
		} patchinstr;

		/* CPU specific structure used to generate full */
		/* instruction in second pass i.e. with expressions */
		/* in displacements evaluated and inserted into instruction. */
		struct Instruction	*instr;	/* INSTRUCTION */
	} type;
} ParseTreeItem;


/* ********************************************************************	*/
/* Parse tree items							*/
/*									*/
/* The following structures are chained off the parse tree items. They	*/
/* describe the various opcodes that have been input and are returned	*/
/* by their associated section of the parser.				*/

/* ConstList is used to hold a variety of code constant types returned	*/
/* from the byte/short/word, blkb/blks/blkw and linkerdata pseudo	*/
/* opcodes.								*/

typedef struct ConstList {
	struct ConstList	*next;		/* NULL term. list of items */
	int			what;		/* E_xxxx (Expr type #defs) */
	union {
		struct Expression	*expr;	/* E_Expr */
		struct Patch		*patch;	/* E_Patch */
		char			*str;	/* E_Str */
	} type;
} ConstList;


/* FloatList is used to hold a list of floating point numbers defined	*/
/* by the ieee32, ieee64 and C40float directives.			*/

typedef struct FloatList {
	struct FloatList *next;		/* NULL term. list of Floats */
	Dble		  value;
} FloatList;


/* NameList is used to hold a list of names defined by the export,	*/
/* codetable and ref directives.					*/

typedef struct NameList {
	struct NameList	*next;		/* NULL term. list of names */
	char		*name;		/* this name */
} NameList;


/* Patch is used to hold any of the standard patch types that may have 	*/
/* been used in a code constant (entered by way of byte / word / blkb /	*/
/* linkerdata / etc), or entered as part of a patchinstr pseudo opcode.	*/

typedef struct Patch {
	int	what;		/* What type of patch decribed: value is the */
				/* same as the lexers patch keyword tokens: */
	union	{
		char		*name;		/* DATASYMB/CODESYMB/  */
						/* DATAMODULE/LABELREF */

		struct Patch	*patch;		/* BYTESWAP */

		struct {			/* SHIFT/ADD/OR */
			struct Expression	*expr;
			struct Patch		*patch;
		} shift;
	} type;
} Patch;


/* **************************************************************************
 * The Expression structure
 *
 * Expression structs are used to hold numeric expressions. They are returned
 * by many parts of the parser whenever an opcode contains a numeric
 * expression as one of its arguments. Expressions are linked together to
 * form a binary evaluation tree that contains the operands, operators and
 * defines their order of evaluation (precidence).
 *
 * e.g. -42 + 3 * (52 - 1 % fred)
 *
 *					+
 *				       / \
 *				     /	   *
 *				   /	  / \
 *				 -	 3   -
 *				/ \	    / \
 *			     NULL  42     52   %
 *					      / \
 *					     1   fred
 *
 *
 * The tree is evaluated by the Eval(Expression *) function which returns the
 * integer result to the caller
 */

typedef struct Expression {
	int		what;			/* E_xxxx #defs */
	union {
		struct {			/* E_Expr */
			int	Operator;
			struct Expression *left;/* NULL if Operator is unary */	
			struct Expression *right;
		} expr;
		int		number;		/* E_Num */
		char		*name;		/* E_Str */
	} type;
} Expression;


/* ********************************************************************	*/
/* Exported variables */

extern int		CurLine;		/* current lexer line */
extern int		StartLine;		/* start line of current stat */
extern int 		curPC;			/* current logical PC */

extern int		errors, warnings;	/* error counters */

extern FILE		*InputFile;		/* lexer input */
extern FILE		*OutputFile;		/* object formatter output */

extern ParseTreeItem	HeaderParseTreeItem;	/* ->next points to first */
						/* item in the parse tree */

extern char		*InputFileName;		/* file names */
extern char		*OutputFileName;

extern int		ModuleHeadTail;		/* module header info */
extern int		ModuleNumber;
extern char		ModuleName[32];
extern int		ModuleVersion;


/* ********************************************************************	*/
/* Exported functions */

/* lexer functions */
extern int		yylex(void);
extern void		InitLex(void);
extern void		ClearInput(void);

/* parser functions */
extern int		yyparse(void);
extern ParseTreeItem	*NewParseTreeItem(int type);
extern ConstList	*NewConstItem(int type);
extern FloatList	*NewFloatConstItem(Dble);
extern Patch		*NewPatchItem(int type);
extern Expression	*NewExpr(Expression *le, int op, Expression *re);
extern Expression 	*NewExprSymbRef(char *name);
extern Expression	*NewExprNum(int num);

/* symbol table functions */
extern unsigned int	Hash(char *name);
extern void		InitSymbTab(void);
extern Symbol		*NewSymb(char *name, int type, int value);
extern void		NewSymbStruct(char *name, int what, void *sstruct);
extern void		NewMnemonic(Mnemonic *mne);
extern Symbol		*FindSymb(char *name);
extern Symbol		*CaseInsensitiveFindSymb(char *name);
extern void		PrintSymbTab(void);

/* evaluation of binary expression trees */
extern int		Eval(struct Expression *expr, int pc);

/* Object code output */
void OutputGHOF(ParseTreeItem *pti);	/* output ghof file from parse tree */

/* Execute processor specific second pass on this instruction */
extern void		Pass2(Instruction *instr, int pc);

/* Pass2 uses these fns to transfer assembled code to the object code */
/* specific formatter module */
extern void		WriteCodeWord(int w);
extern void		WriteCodeShort(int s);
extern void		WriteCodeByte(char b);
extern void		WriteCodeFloat(Dble d);
extern void		WriteCodeDouble(Dble d);

extern void		WriteCodeC40Float(Dble d);


/* Floating point functions */
/*
-- IEEE 64 bit integer arithmetic
*/


extern void fltrep_stod(const char *, DbleBin *) ;
extern bool fltrep_narrow_round(DbleBin *, FloatBin *) ;
extern bool flt_add(DbleBin *, DbleBin *, DbleBin *) ;
extern bool flt_subtract(DbleBin *, DbleBin *, DbleBin *) ;
extern bool flt_multiply(DbleBin *, DbleBin *, DbleBin *) ;
extern bool flt_divide(DbleBin *, DbleBin *, DbleBin *) ;
extern bool flt_negate(DbleBin *, DbleBin *) ;
extern bool flt_itod(DbleBin *, int32) ;


/* Misc functions */
extern void 		Fatal(char *s);
extern void 		Error(char *s);
extern void 		Warn(char *s);
extern void 		Note(char *s);
extern char		*itoabin(char *str, int nbits, int bin);

/* debugging functions */









/* Exported Variables: */

			/* current logical PC value */
int			curPC = 0;

			/* Dummy top item, its next field point to first */
			/* true item */
ParseTreeItem		HeaderParseTreeItem;


int			StartLine = 0;


/* Internal Variables: */

			/* contains the SIZE of the items in a list */
static int		list_size_count = 0;

			/* maintains pointer to last item in parse tree */
static ParseTreeItem	*LastParseTreeItem = &HeaderParseTreeItem;




#line 4712 "gasm.y"
typedef union {
	int			num;
	Dble			flt;
	char			*str;
	struct ConstList	*clist;
	struct FloatList	*flist;
	struct NameList		*nlist;
	struct Patch		*patch;
	struct Instruction	*instr;
	struct Expression	*expr;
	struct Mnemonic		*mnem;
} YYSTYPE;
#line 4817 "gasm.y"
/*
 * File:	ghof.h
 * Subsystem:	Generic Assembler
 * Author:	P.A.Beskeen
 * Date:	Sept '91
 *
 * Description: GHOF (Generic Helios Object Format) directive and patch
 *		definitions.
 *
 * RcsId: $Id: ghof.h,v 1.3 1993/01/29 17:56:29 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */






/* defines the maximum size of an individual CODE directive */





























/* General Patches */







/* Processor Specific Patches */

















/* GHOF encoding definitions */







/* Exported Data: */

extern int codesize;


/* Exported Functions: */

void FlushCodeBuffer(void);		/* flush buffer before GHOFEncode's */
void GHOFEncode(int n);			/* GHOF encode and output a number */
void ObjWriteByte(char b);		/* write a byte to the object file */
void ObjWrite(char *buf, int size);	/* write a block to the object file */






	/* Current instruction we are parsing */
	Instruction	*CurInstr = ((void *)  0) ;
#line 4793 "y.tab.c"
#define NAME 257
#define NUMBER 258
#define CHARCONST 259
#define FLOATNUM 260
#define LSHIFT 261
#define RSHIFT 262
#define USERERROR 263
#define USERWARNING 264
#define USERNOTE 265
#define BYTE 266
#define SHORT 267
#define WORD 268
#define FLOATY 269
#define DOUBLE 270
#define BLOCKBYTE 271
#define BLOCKSHORT 272
#define BLOCKWORD 273
#define SPACE 274
#define ALIGN 275
#define DATA 276
#define COMMON 277
#define CODETABLE 278
#define EXPORT 279
#define IMPORT 280
#define MODULE 281
#define INIT 282
#define REF 283
#define PATCHINSTR 284
#define MODSIZE 285
#define MODNUM 286
#define DATASYMB 287
#define CODESYMB 288
#define DATAMODULE 289
#define LABELREF 290
#define CODESTUB 291
#define ADDRSTUB 292
#define BYTESWAP 293
#define SHIFT 294
#define P_ADD 295
#define P_OR 296
#define STRINGCONST 297
#define LABEL 298
#define PLUSPLUS 299
#define MINUSMINUS 300
#define INSTRUCTION 301
#define UNARY 302
#define LDA 303
#define LDE 304
#define LDEP 305
#define LDF 306
#define LDHI 307
#define LDI 308
#define LDM 309
#define LDP 310
#define LDPE 311
#define LDPK 312
#define STF 313
#define STI 314
#define STIK 315
#define LBb 316
#define LBUb 317
#define LDFcond 318
#define LDIcond 319
#define LHw 320
#define LHUw 321
#define LWLct 322
#define LWRct 323
#define ABSF 324
#define ABSI 325
#define ADDC 326
#define ADDF 327
#define ADDI 328
#define AND 329
#define ANDN 330
#define ASH 331
#define CMPF 332
#define CMPI 333
#define FIX 334
#define FLOAT 335
#define FRIEEE 336
#define LSH 337
#define MPYF 338
#define MPYI 339
#define MPYSHI 340
#define MPYUHI 341
#define NEGB 342
#define NEGF 343
#define NEGI 344
#define NORM 345
#define NOT 346
#define OR 347
#define RCPF 348
#define RND 349
#define ROL 350
#define ROLC 351
#define ROR 352
#define RORC 353
#define RSQRF 354
#define SUBB 355
#define SUBC 356
#define SUBF 357
#define SUBI 358
#define SUBRB 359
#define SUBRF 360
#define SUBRI 361
#define TOIEEE 362
#define TSTB 363
#define XOR 364
#define MBct 365
#define MHct 366
#define ADDC3 367
#define ADDF3 368
#define ADDI3 369
#define AND3 370
#define ANDN3 371
#define ASH3 372
#define CMPF3 373
#define CMPI3 374
#define LSH3 375
#define MPYF3 376
#define MPYI3 377
#define MPYSHI3 378
#define MPYUHI3 379
#define OR3 380
#define SUBB3 381
#define SUBF3 382
#define SUBI3 383
#define TSTB3 384
#define XOR3 385
#define BR 386
#define BRD 387
#define CALL 388
#define LAJ 389
#define RPTB 390
#define RPTBD 391
#define RPTS 392
#define Bcond 393
#define BcondAF 394
#define BcondAT 395
#define BcondD 396
#define CALLcond 397
#define DBcond 398
#define DBcondD 399
#define LAJcond 400
#define LATcond 401
#define RETIcond 402
#define RETIcondD 403
#define RETScond 404
#define TRAPcond 405
#define LDFI 406
#define LDII 407
#define SIGI 408
#define STFI 409
#define STII 410
#define POPF 411
#define POP 412
#define PUSH 413
#define PUSHF 414
#define IDLE 415
#define SWI 416
#define NOP 417
#define IACK 418
#define C40FLOAT 419
#define PATCHC40DATAMODULE1 420
#define PATCHC40DATAMODULE2 421
#define PATCHC40DATAMODULE3 422
#define PATCHC40DATAMODULE4 423
#define PATCHC40DATAMODULE5 424
#define PATCHC40MASK24ADD 425
#define PATCHC40MASK16ADD 426
#define PATCHC40MASK8ADD 427
#define R0 428
#define R1 429
#define R2 430
#define R3 431
#define R4 432
#define R5 433
#define R6 434
#define R7 435
#define R8 436
#define R9 437
#define R10 438
#define R11 439
#define AR0 440
#define AR1 441
#define AR2 442
#define AR3 443
#define AR4 444
#define AR5 445
#define AR6 446
#define AR7 447
#define DP 448
#define IR0 449
#define IR1 450
#define BK 451
#define SP 452
#define ST 453
#define DIE 454
#define IIE 455
#define IIF 456
#define RS 457
#define RE 458
#define RC 459
#define IVTP 460
#define TVTP 461
#define BARBAR 462
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,   82,    0,   84,    0,    0,   81,   83,   83,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   87,   88,   89,   94,   93,   95,   90,   91,
   92,   96,   97,   98,   99,  100,  101,  102,  103,  104,
  105,  106,  107,  108,  109,   10,   10,   10,   10,   10,
   10,   10,   10,   10,   10,   10,   10,   11,   11,  110,
    5,    5,  111,    7,    7,    6,    6,    6,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    3,    3,    3,    3,    3,    3,    3,
    3,    2,    2,    8,    8,    8,    8,    8,    8,    8,
    8,    8,    8,    8,    8,    8,    8,    8,    8,    9,
    9,   12,   12,   12,   12,   12,   12,   12,   12,   12,
   12,   12,   12,   12,   12,   12,   12,   12,   12,   12,
   12,   12,   12,   12,   12,   12,   12,   12,   12,   12,
   12,   12,   12,   12,   13,   13,   13,   13,   13,   13,
   13,   13,   13,   13,   13,   13,   13,   14,   14,   14,
   15,   15,   15,   16,   16,   16,   16,   16,   16,   16,
   16,   16,   17,   17,   17,   17,   17,   17,   17,   17,
   18,   18,   18,   18,   18,   18,   18,   18,   18,   18,
   18,   18,   18,   18,   19,   19,   19,   24,   25,   27,
   20,   20,   20,   20,   21,   21,   21,   21,   22,   22,
   22,   22,   22,   23,   23,   23,   23,  112,   26,   26,
   28,   28,   28,   28,   28,   29,   29,   29,   30,   31,
   32,   32,   32,   32,   33,   33,   33,   33,   34,   34,
   34,   34,   35,   35,   35,   35,   35,   35,   36,   36,
   36,   36,   38,   38,   37,   39,   41,   41,   40,   42,
   43,   43,   43,   43,   43,   43,   43,   43,   43,   43,
   43,   43,   43,   48,   48,   48,   48,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   47,   47,   47,   47,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   49,   49,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   50,   50,  114,   86,  113,  113,  113,  113,
  113,  113,  115,  115,  115,  115,  115,  115,  115,  115,
  115,  115,  115,  115,  115,  115,  115,  121,  123,   51,
   51,   51,  122,  122,  122,  122,  122,  122,  124,  124,
  124,  124,  124,  124,  125,  125,  125,   52,   52,   52,
   52,   52,   52,   52,   52,  126,  126,   53,   53,   53,
  116,  116,  116,  116,  116,  116,  116,  116,  116,  116,
  116,  116,  116,  116,  116,  116,  116,  116,  116,   54,
   54,   55,   55,   55,   55,   55,   55,   55,   55,   55,
   55,   55,   56,   56,   56,   57,   57,   57,   57,   57,
   57,   58,   58,   58,   58,   59,   59,   59,   59,   60,
   60,  117,  117,  117,  117,  117,  117,  127,   61,   61,
   61,   61,  128,  128,   62,   62,   62,   62,   62,   62,
  129,  129,  129,  129,  130,   77,   77,   77,  131,   78,
   78,  132,  132,  132,  132,  132,  132,   79,   79,  118,
  118,  118,  118,  120,  120,  120,  120,  120,  120,  119,
  119,  119,  119,  119,  119,  119,  119,  119,  119,  119,
  119,  119,  119,  119,  119,  119,  119,  119,  119,  119,
  119,  119,  119,  119,  119,  119,  119,  133,  133,  134,
  134,   63,   63,  135,   64,   64,   64,  136,  137,   65,
   65,   65,   65,   65,  138,  138,   66,   66,   66,   66,
   66,  139,  139,  139,   67,   67,   67,  140,  140,  140,
   68,   68,  141,  141,  141,  142,  142,   69,   69,   69,
  144,  144,   70,   70,  152,  152,  152,  150,  150,  150,
  148,  148,  148,  146,  146,  146,  143,  143,  143,   71,
   71,   71,  145,  145,  145,   72,   72,  153,  153,  153,
  153,  151,  151,  151,  151,  149,  149,  149,  149,  147,
  147,  147,  147,  154,  154,  154,  154,  161,  161,  161,
  162,  162,  162,  162,  163,  163,  163,  164,  164,  164,
  164,  156,   73,   73,   73,   73,  155,  155,  155,   74,
   74,  157,  157,  157,  157,  165,  165,  165,  165,   75,
   75,  166,  166,  166,  167,  167,  167,  167,  167,  167,
   76,   76,  168,  168,  168,  168,  160,  160,  160,  160,
  160,  160,  159,  159,  159,  159,  159,  158,  158,  158,
    4,    4,   80,   80,   80,   80,   80,   80,   80,   80,
};
short yylen[] = {                                         2,
    0,    0,    3,    0,    3,    2,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    2,    2,    2,    2,    2,    2,    4,    4,
    4,    2,    2,    4,    4,    2,    2,    2,    2,    1,
    2,    8,    2,    2,    2,    1,    1,    4,    4,    4,
    4,    4,    4,    4,    6,    6,    6,    1,    3,    0,
    2,    3,    0,    2,    3,    1,    1,    1,    3,    3,
    3,    3,    3,    3,    3,    3,    3,    3,    3,    2,
    2,    1,    1,    3,    3,    3,    3,    3,    2,    1,
    1,    0,    1,    3,    3,    3,    3,    3,    3,    3,
    3,    3,    3,    3,    2,    2,    1,    1,    1,    0,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    2,    1,    1,
    2,    4,    3,    2,    1,    1,    1,    1,    5,    2,
    2,    3,    3,    0,    3,    3,    3,    1,    1,    2,
    3,    3,    3,    3,    1,    3,    3,    3,    1,    1,
    3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
    3,    3,    5,    5,    5,    5,    5,    5,    5,    5,
    5,    5,   10,    8,    8,   10,   10,    8,    8,   10,
   12,   12,   12,   12,   12,   12,   10,   10,   10,    8,
   10,   10,   10,    1,    1,    1,    1,   12,   12,   12,
   12,   12,   12,   10,   10,   10,    8,   10,   10,   10,
    1,    1,    1,    1,   12,   12,   12,   12,   12,   12,
   10,   10,   10,    8,   10,   10,   10,    1,    1,   12,
   12,   12,   12,   12,   12,   10,   10,   10,    8,   10,
   10,   10,    1,    1,    0,    2,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    2,    4,
    4,    2,    5,    3,    2,    4,    4,    2,    2,    1,
    1,    1,    4,    4,    4,    4,    9,    9,    4,    4,
    4,    4,    9,    9,    4,    4,    4,    1,    1,    1,
    1,    1,    1,    1,    1,    4,    4,    1,    1,    1,
    2,    4,    4,    4,    4,    4,    4,    9,    9,   11,
    9,    9,   11,    9,    9,   11,    9,    9,   11,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    2,    1,    1,
    1,    1,    2,    2,    1,    1,    1,    1,    1,    1,
    4,    4,    4,    4,    1,    1,    1,    1,    2,    1,
    1,    2,    2,    2,    2,    2,    2,    1,    1,    2,
    2,    2,    2,    2,    2,    1,    2,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    2,    2,    2,
    2,    1,    1,    2,    1,    1,    1,    2,    2,    1,
    1,    1,    1,    1,    2,    2,    1,    1,    1,    1,
    1,    2,    2,    2,    1,    1,    1,    2,    2,    2,
    1,    1,    2,    2,    2,    2,    2,    1,    1,    1,
    2,    2,    1,    1,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    1,
    1,    1,    2,    2,    2,    1,    1,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    1,    1,    1,    1,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    1,    1,    1,    1,    4,    4,    4,    1,
    1,    1,    1,    1,    1,    2,    4,    4,    4,    1,
    1,    2,    4,    4,    4,    4,    4,    4,    4,    4,
    1,    1,    4,    4,    4,    4,    2,    4,    4,    4,
    4,    2,    2,    4,    4,    4,    2,    4,    4,    2,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
};
short yydefred[] = {                                      1,
    0,    6,    0,    0,    7,    3,    0,    0,    0,   70,
   70,   70,   73,   73,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   50,    0,    0,   73,    5,
    8,    9,   10,   11,   12,   13,   14,   15,   16,   17,
   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
   28,   29,   30,   31,   32,    0,   53,   54,   55,    0,
    0,    0,    0,    0,    0,    0,   92,   93,    0,    0,
    0,    0,    0,    0,    0,    0,   43,    0,    0,   68,
    0,    0,    0,  119,  117,  118,    0,    0,    0,    0,
   49,    0,    0,    0,    0,  360,    0,    0,    0,    0,
  361,    0,    0,    0,    0,    0,    0,  378,  379,  362,
    0,  380,  381,  382,  383,  551,  545,  537,    0,    0,
  580,  538,  586,    0,  640,    0,    0,    0,  587,    0,
    0,  539,  540,    0,  552,  546,  522,  547,  581,  630,
  523,  623,  624,  625,  626,  631,  541,  525,    0,    0,
  526,    0,  527,    0,  641,  582,  384,  385,  530,    0,
    0,  558,  531,  563,    0,  651,  564,    0,    0,  532,
  533,  559,  534,    0,    0,  652,  560,  449,  450,  451,
  452,  478,  479,    0,  455,  456,  457,  458,  459,    0,
    0,  460,  470,  466,  467,  468,  471,  388,  389,  390,
  410,  411,    0,    0,    0,    0,  486,  489,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  465,    0,    0,  336,  337,  338,  339,  340,  341,  342,
  343,  344,  345,  346,  347,  348,  442,  443,  444,  445,
  446,  447,  490,  491,  492,  493,  494,  495,  496,  497,
  498,  499,  500,  501,  502,  503,  504,  505,  506,  507,
  508,  509,  510,  511,  512,  513,  514,  515,  516,  517,
  604,  605,  606,  607,  632,  633,  634,  635,    0,   56,
   57,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   78,   71,    0,   77,    0,  100,  101,    0,    0,
    0,   91,    0,   90,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   70,   70,   70,    0,    0,    0,
  116,    0,  115,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  673,  674,  675,  676,  677,  680,  679,
  678,    0,    0,  672,    0,    0,  122,  123,  124,  125,
  126,  127,  128,  129,  130,  131,  132,  133,  134,  135,
  136,  137,  138,  139,  140,  141,  142,  143,  144,  145,
  146,  147,  148,  149,  150,  151,  152,  153,    0,    0,
    0,    0,    0,    0,  349,  207,  205,  206,    0,  167,
  155,  156,  157,  158,  159,  160,  161,  162,  163,  164,
  165,  166,    0,    0,    0,    0,    0,    0,  154,    0,
    0,    0,    0,    0,    0,    0,  355,    0,    0,    0,
    0,    0,    0,    0,  358,    0,    0,    0,    0,  600,
  601,  602,  603,    0,    0,    0,  592,  593,  594,  595,
    0,    0,  642,    0,    0,    0,    0,  662,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  240,    0,    0,    0,  553,  554,  555,    0,
    0,  670,    0,    0,  611,  612,  613,  614,    0,    0,
  618,  619,  620,  621,    0,  518,  519,    0,    0,  596,
  597,  598,  599,    0,    0,  588,  589,  590,  591,    0,
    0,  528,    0,    0,    0,  667,    0,    0,  574,  575,
  576,    0,    0,    0,  568,  569,  570,    0,    0,    0,
    0,  608,  609,  610,    0,    0,  615,  616,  617,    0,
    0,  571,  572,  573,    0,    0,  565,  566,  567,  474,
  477,  475,  476,  183,  184,  185,  186,  187,  188,  189,
  190,    0,    0,  483,  482,  480,  481,  487,  485,  484,
  359,    0,    0,    0,    0,    0,    0,  391,    0,    0,
  448,  454,  453,    0,  520,  521,  524,    0,    0,  529,
    0,    0,  535,  536,    0,  542,  543,  544,    0,  548,
  549,  550,    0,  556,  557,    0,  561,  562,    0,  577,
  578,  579,    0,  583,  584,  585,  622,    0,    0,    0,
    0,    0,    0,  636,    0,    0,    0,  469,  473,  472,
   72,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   99,    0,    0,    0,    0,    0,   79,    0,
    0,    0,    0,    0,    0,    0,   82,   83,   84,    0,
    0,    0,    0,    0,   69,  104,    0,    0,    0,    0,
    0,    0,    0,  107,  108,  109,    0,  214,  217,  218,
  215,  216,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   94,    0,    0,   97,   98,    0,
    0,    0,  213,    0,  204,  191,  192,  193,  194,  195,
  196,  197,  198,  201,  199,  200,  202,  203,  231,  232,
  233,  234,  356,  369,    0,  370,  372,  350,  363,    0,
  364,  366,    0,  351,  357,    0,  394,    0,  396,  393,
  392,  241,  243,  242,  244,    0,    0,    0,    0,    0,
    0,    0,    0,  246,  248,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  245,  643,  247,  644,  658,
    0,  659,  661,  249,    0,  250,  252,    0,  668,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  664,    0,  665,    0,    0,    0,    0,    0,  653,
  654,  655,  656,    0,    0,    0,    0,    0,    0,  461,
  462,  463,  464,  375,  377,  376,  387,  386,  238,  237,
  236,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  627,  629,  628,  637,  639,  638,  645,  646,
  649,  650,  647,  648,   58,   59,   60,   61,   62,   63,
   64,    0,    0,    0,  335,    0,    0,    0,    0,  212,
    0,    0,  353,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  222,    0,  223,    0,    0,    0,    0,
    0,    0,  436,  432,  437,  438,  434,  440,  439,  433,
  435,  441,    0,    0,    0,    0,    0,  423,  412,  414,
  427,  416,  429,  417,  424,  425,  419,  430,  421,  413,
  415,  426,  428,  418,  420,  431,  422,    0,    0,    0,
  259,    0,  333,  334,    0,    0,    0,    0,    0,  262,
    0,  253,    0,  318,  319,    0,    0,    0,    0,    0,
  258,    0,  255,  256,    0,    0,  302,  301,  304,  303,
    0,    0,    0,    0,    0,  260,    0,    0,    0,    0,
  285,  284,  287,  286,    0,    0,    0,    0,    0,  254,
    0,    0,    0,    0,    0,    0,    0,    0,  257,    0,
    0,   65,   66,   67,   52,    0,  227,  225,  226,    0,
  182,  174,  175,  176,  177,  178,  179,  180,  181,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  228,  219,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  373,  374,  367,  368,  399,  398,    0,  402,
    0,  405,  404,    0,  408,    0,    0,    0,    0,    0,
  268,    0,    0,    0,    0,  329,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  264,    0,  314,    0,    0,
    0,    0,    0,  265,  269,    0,    0,    0,    0,    0,
    0,    0,    0,  297,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  280,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  400,  403,  406,  409,    0,  170,
  168,  169,  330,    0,  328,    0,  332,  331,  267,    0,
  326,    0,  327,    0,    0,  315,    0,  313,    0,  317,
  316,  263,    0,  311,    0,  312,    0,  173,  171,  172,
  295,    0,    0,  300,    0,  296,  294,    0,  299,    0,
  298,    0,  278,    0,    0,  283,    0,  279,  277,    0,
  282,    0,  281,    0,  270,  266,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  320,  325,  324,  322,  321,  323,  305,  310,  309,  307,
  306,  308,  291,  293,  290,  292,  289,  288,  274,  276,
  273,  275,  272,  271,
};
short yydgoto[] = {                                       1,
   72,   77,  413,  352,   60,  303,   64,  390,   91,  305,
   81, 1072,  510, 1095,  850, 1140,  562,  819,  399,  853,
  684,  803,  957,  433,  446,  581,  439,  395,  578,  496,
  477,  435,  440,  478,  448,  441,  458,  449,  508,  479,
  442,  502,  494,  488,  450,  443, 1091, 1105, 1079, 1068,
  211,  212,  213,  214, 1058, 1059, 1060, 1033, 1034, 1035,
  215,  216,  217,  218,  219,  220,  221,  222,  223,  224,
  225,  226,  227,  228,  229,  230,  231,  232,  233,  354,
    6,    3,   30,    4,   31,   32,   33,   34,   35,   36,
   37,   38,   39,   40,   41,   42,   43,   44,   45,   46,
   47,   48,   49,   50,   51,   52,   53,   54,   55,   61,
   65, 1192,  234,   56,  235,  236,  237,  238,  239,  240,
  241,  242,  243,  244,  245,  246,  247,  248,  249,  250,
  251,  252,  253,  254,  255,  256,  257,  258,  259,  260,
  261,  262,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,  285,  286,  287,  288,
};
short yysindex[] = {                                      0,
 -239,    0, -261, 5597,    0,    0, -249, -236, -219,    0,
    0,    0,    0,    0,   50,   50,   50,   50,   50, -170,
 -159, -146, -146, -146,  243,    0, -146,  100,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,25579,    0,    0,    0,   92,
25516,   92,   92,   99,    1,   99,    0,    0,   50,   50,
   50,  684,  795, 1166,  836,  836,    0,  121,  129,    0,
  137,  137,  137,    0,    0,    0,  243,  243,  243, 5299,
    0,  137, 3770,   99, 2080,    0, -207,  157,  243, 2500,
    0,   48, 3388,  243, 1001, 3388,  243,    0,    0,    0,
 2500,    0,    0,    0,    0,    0,    0,    0,  157, 2500,
    0,    0,    0,  157,    0,  157, 2920,    5,    0,  157,
 2500,    0,    0, 2500,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  157, 2500,
    0,  157,    0,  -30,    0,    0,    0,    0,    0,  281,
  118,    0,    0,    0,  281,    0,    0,  281,  118,    0,
    0,    0,    0,  281,  118,    0,    0,    0,    0,    0,
    0,    0,    0, 2500,    0,    0,    0,    0,    0,  158,
  158,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0, 1001, 3388, 3388, 1001,    0,    0,  165,    5,
  157,   77,    5, 3388,  153, 1660,  157, 2500,  118, 2500,
 2500,  157,  118,  118, 2500, 2500, 3388,  -30, 2500,  118,
    0,  243, 1660,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,25516,    0,
    0,  171,  180,  184,  187,  197,  198,  211,  217,  232,
  236,    0,    0, 5299,    0,    1,    0,    0,    1,    1,
  601,    0, 3756,    0,   50,   50,   50,   50,   50,   50,
   50,   50,   50,   50,    0,    0,    0,  243,  243,   21,
    0, 4176,    0,  243,  243,  243,  243,  243,  243,  243,
  243,  243,  243,    0,    0,    0,    0,    0,    0,    0,
    0,  252, 5299,    0,    0,  425,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  243, 5299,
  253,  254,  258,  259,    0,    0,    0,    0,  273,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  601,  280,  288,  296,  298,  301,    0,  308,
  312,  323,  324,  243,  328,  329,    0,  340,  344,  346,
  348,  349,  350,  353,    0,  370,  372,  375,  376,    0,
    0,    0,    0,  379,  398,  403,    0,    0,    0,    0,
  417,  436,    0,  439,  440,  450,  451,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  452,    0,  454,  455,  459,    0,    0,    0,  460,
  494,    0,  495,  496,    0,    0,    0,    0,  497,  498,
    0,    0,    0,    0,  348,    0,    0,  501,  534,    0,
    0,    0,    0,  536,  553,    0,    0,    0,    0,  573,
  595,    0,  598,  615,  618,    0,  620,  621,    0,    0,
    0,  622,  623,  624,    0,    0,    0,  626,  632,  638,
  639,    0,    0,    0,  646,  647,    0,    0,    0,  648,
  653,    0,    0,    0,  662,  663,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  664,  679,    0,    0,    0,    0,    0,    0,    0,
    0,  686,  688,  689,  700,  702,  703,    0,  243, 5299,
    0,    0,    0,  573,    0,    0,    0,  704,  705,    0,
  710,  711,    0,    0,  712,    0,    0,    0,  713,    0,
    0,    0,  717,    0,    0,  724,    0,    0,  727,    0,
    0,    0,  728,    0,    0,    0,    0,  733,  737,  738,
  740,  747,  748,    0,  753,  754,  765,    0,    0,    0,
    0,  126,  209,  356,  361,  388,  445, 1398,  243,  243,
  243,  601,    0,  422,    1,    1,    1,    1,    0,  658,
  658,   54,   39,  109,  299,  299,    0,    0,    0,   92,
   92,   92, 5299, 5299,    0,    0,  907,  907,  569,  940,
  125,  311,  311,    0,    0,    0, 1398,    0,    0,    0,
    0,    0, -104,  158, 5299, 1230, 1230, 1230, 1230, 3388,
 1001, 1001, 1001, 1001, 3388, 3388, 3388, 3388, 3388,  771,
 3388, -207,    5,    5,    5, 3388, 3388, 3388, 3388,  652,
  281, 1001, 1001,  741, 2964, 2964,  281,  281, 3388, 3388,
 3388, 3388, 1001, 1001, 1001, 1001, 1001, 1001,  281,  281,
 2964, 2964,  652,  281,  741, 2964, 1001, 1001, 1001, 1001,
 1001,  652,  281,  741, 2964, 2964,  281,  281,  281,  281,
 2964, 2964,  652,  281,  741, 2964, 1660, 1660, 3388, 3388,
 3388, 3388, 3388,  -24, 5299, 2964, 2964, 2964, 2964, 3388,
 1001, 2964, 2964, 2964, 2964, 1001, 1001, 1001, 2964, 2964,
 2964, 2964, 2964, 2964,  655,  721,  729,  779,  780,  781,
  793, 4596, 5016, 5246,    0,  176,  176,    0,    0,  791,
  801,  803,    0,  803,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  382,    0,    0,    0,    0,  383,
    0,    0, 3388,    0,    0,  385,    0,  386,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  807,  397,
  816,  -36,  818,    0,    0,    0,    0,    0,  820,  414,
  838,  -35,  840,  844,  846,    0,    0,    0,    0,    0,
  433,    0,    0,    0,  435,    0,    0,  435,    0,  -34,
  854,  -33,  858,  -22,  859,  -18,  863,  865,  -17,  876,
  -16,    0,  435,    0,  807,  -36,  820,  -35,  844,    0,
    0,    0,    0,  -34,  -33,  -22,  -18,  -17,  -16,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  877,  879,  881,  883,  877,  881,  433,  435,   -2,
  884,   -2,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, 1398, 1398, 1398,    0,   23,  892,   27,  895,    0,
 -255, -205,    0, 5079, 5580, 1704, -293, 1001, -263, 1001,
 1704, 3802, -283, 3388, -148, 3388, 3802, 3388, 3388,  619,
  627, -297, 2124, 2124, -127, 1001, 1001, -136, 4222, 4222,
  535, 3388, 3388, 1001, -293, 1001, 3388, -283, 3388, 3388,
 3388, 3388, 3388,  629, 3388, 3388,  894,  910,  914,  916,
  921,  926, 5257,    0,  927,    0,  165,  482,  165,  482,
  165,  482,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  165,  165,  482,  165,  482,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  165,  165,  482,
    0,  507,    0,    0,  165,  509,  482,   58,  513,    0,
  514,    0,  517,    0,    0,  165,  518,  482,   58,  524,
    0,  526,    0,    0,  482,  482,    0,    0,    0,    0,
  165,    0,    0,    0,  527,    0,  528,   58,  530,  531,
    0,    0,    0,    0,  165,    0,    0,    0,  533,    0,
  537,   58,  538,  539,  540,  542,  543,  544,    0,  545,
  546,    0,    0,    0,    0,  752,    0,    0,    0,  967,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  968,
  969,  970,  971,  972,  975,  988,  989,  991,  994,  996,
 1000, 1004, -293, 1007, -293, 1011, 1014, 1016,  -63, -293,
 -283, 1017, -283, 1019, 1024, 1028,    4, -283, 1029, 1033,
 1048, -297, -297, 1049, 1050,  574, -297, 1051, -136, -136,
 1052, 1056,  578, -136,    6, -293,  104, -283,  788,  790,
    0,    0, 1001,  165, 3388,  165,  482,  165,  482,  482,
  165,  482,  165,  482,  482,  165,  165,  165,   58,  165,
  165,  596,  482,   58,  482,  165,  165,   58,  165,  165,
  596,  482,   58,  482,  165,  165,  165,  165,   58,  165,
  718,   58,  482,  165,  165,   58,  165,  718,   58,  482,
  482,  482,    0,    0,    0,    0,    0,    0, 1061,    0,
 1063,    0,    0, 1065,    0, 1066, 1073, 1076, 1082, 1083,
    0, 1085,    0,    0,    0,    0, 1092, 1094, 1099, 1101,
 1110, 1111, 1124, 1162, 1163,    0, 1168,    0, 1170, 1171,
 1172, 1173, 1174,    0,    0, 1175, 1177, 1179, 1181, 1184,
    0,    0,    0,    0, 1185, 1187, 1189, 1190, 1194, 1196,
 1200, 1201, 1204,    0, 1208, 1209, 1212, 1239, 1241, 1242,
  482,  482,  482,  482,  165, -213,  165,  596, -213, -213,
  165,  165,  596,  596,  165, -213,  165,  596, -213, -213,
  165,  165,  596,  596, -243,  165,  165,  718, -243, -243,
  165,  718,  718, -243,  165,  165,  718, -243, -243,  165,
  718,  718,  165,  165,    0,    0,    0,    0, 1243,    0,
    0,    0,    0, 1245,    0, 1248,    0,    0,    0, 1251,
    0, 1374,    0, 1375, 1376,    0, 1377,    0, 1382,    0,
    0,    0, 1401,    0, 1402,    0, 1403,    0,    0,    0,
    0, 1406, 1407,    0, 1408,    0,    0, 1409,    0, 1418,
    0, 1419,    0, 1422, 1423,    0, 1431,    0,    0, 1432,
    0, 1433,    0, 1438,    0,    0, -213, -213, -213, -213,
 -213, -213, -213, -213, -213, -213, -213, -213, -243, -243,
 -243, -243, -243, -243, -243, -243, -243, -243, -243, -243,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,
};
short yyrindex[] = {                                      0,
 8523,    0,    0,25695,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,21176,    0,
    0,    0,    0,    0,21340,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,21597,
    0,21761,22018,22182,    0,22439,    0,    0,    0,    0,
    0,    0,    0,    0,22603,22860,    0,    0,    0,    0,
23024,23281,23445,    0,    0,    0,    0,    0,    0,23702,
    0,23866,    0,24123,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,14169,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,14426,    0,    0,    0,    0,    0,    0,
14590,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0, 1439,    0,  998,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, 9785,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0, 1443,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,14848,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,15012,    0,    0,    0,    0, 9949,10207,
10371,10629,10793,11051,11215,11473,11637,11895,12059,12317,
12481,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,15270,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,15434,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,15692,
    0,    0,    0,15856,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,16113,    0,    0,    0,    0,    0,    0,    0, 6443,
 6607, 9363, 8195, 7611, 5275, 5439,    0,    0,    0,24287,
24544,24708,24965,25129,    0,    0, 7027, 7191, 9527, 8359,
 7775, 5859, 6023,    0,    0,    0,    0,    0,    0,    0,
    0,    0, 1667,    0,12739,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,16277,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,16535,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0, 8779, 8943,    0,    0,    0,
 1247, 1247,    0, 1915,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,16699,    0,    0,    0,    0,16957,
    0,    0,    0,    0,    0,17121,    0,17379,    0,    0,
    0,    0,    0,    0,    0, 2087, 2335, 2507,17543,    0,
    0,17801,    0,    0,    0, 2755, 2927, 3175,17965,    0,
    0,18223,    0,18387,    0,    0,    0,    0,    0,    0,
18645,    0,    0,    0,18809,    0,    0,19067,    0,17543,
    0,17801,    0,17965,    0,18223,    0,    0,17801,    0,
18223,    0,19231,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,17965,18223,18223,17801,    0,
    0,18223,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0, 3347,    0, 3595,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,19489,    0,    0,19653,    0,
    0,    0,    0,    0,    0,    0,19911,    0,    0,20075,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, 3767, 4015, 4187,    0,    0,    0,    0,19653,20333,
    0,    0,    0,    0,    0, 4435, 4607, 4855,    0,    0,
    0,    0,20075,20497,19489,19653,19911,20075,    0,20075,
19911,    0,    0,    0,    0, 1495,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,20755,    0,
    0,    0,    0,20919,    0,    0,    0,    0,    0,    0,
    0,    0,12903,13161,13325,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
13583,13747,14005,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
  309,    0,  -64,    0,   -5,  829,   91,   96,    0, -615,
  339,   44,  731, -969, -273,24529, -187, -398,  786,  -95,
    0,    0, -643,   82, 5273, -214,   12,    0,    0, 1268,
   18, 5176,  842,    0, 1074,  726, 1269, -117, -122,  -90,
  -23, 1317, 1323, 1325,   29,   65, -698, -849, -394, -764,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  541,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
};
#define YYTABLESIZE 26113
short yytable[] = {                                     392,
  311,  583,  415,  563,  421,   62,   63,  970,  976,  983,
  986,  356, 1388,  493, 1097,  432,    2,  356,  630, 1109,
 1111,  989,  791,  437,  445,  992,  996,  999,  452, 1087,
  455,  474,  480,  389,  484,  490,    5,  482,  432,  389,
  310, 1005, 1360,  526, 1063,  309,  356,   57,  396, 1067,
 1017,  538,  548,  499,  505, 1074,  511, 1018,  514, 1088,
   58,  800,   88,  516,  518,  523,   88,   87,  389,  529,
 1089,   87,  531,  536, 1063,  324,  319,   59,  541,  546,
  322,  320, 1064,  321, 1090,  323,   78,   88,  551,   70,
  324,  319,   87, 1075,   69,  322,  320,   79,  321,  356,
  323,  608, 1019,  616,   66,  605,  487,  612, 1020,  417,
   80,  424, 1064,  568,  569,  511,  573,  575,  356,   94,
   90,  511,  432,  589,  592,  595,  599,  603,  589,  609,
  592,  602,  619,  622,  626,  289,  520,  457,  391,   93,
  389, 1112,  306,  420,  533,  324,  426,  318,   89,  429,
  322,  320,   89,  321,  431,  323,  304,   88,  959,  356,
  960,  343,   87,  444,  328, 1078,  341,  339,  683,  340,
  472,  342,  329,   89,  489,   71,  393,  495,  509,  416,
  330,  422,  331,  332,  333, 1067, 1389, 1390,  353,  527,
 1074, 1101,   88,  504,  801,  802,  310,   87,  356, 1087,
  438,  309, 1065,  549,  522,  438,  356,  456,  475,  481,
  632,  438,  535,  503, 1361, 1362,  579,  647,  545,  633,
  389, 1102,  648,  634,  521,  400,  635,  550, 1075, 1088,
  438,  919, 1103,  438,  585,  515,  636,  637,  544,  600,
 1089,  642, 1266,   89,  643,  644, 1104,  565,  566, 1213,
  638, 1278,  397,  398, 1090,  572,  639,  577,  307,  582,
  308,  431,  588,  591,  495,  552,  588,  606,  591,  613,
  617,  640,  621,  625, 1063,  641,  629,  665,   89,   84,
   85,   86,   88,   84,   85,   86, 1098,   87,  820,  821,
  822,  570,  438,  574,  576,  677,  686,  687,  438,  315,
  316,  688,  689,  438,   84,   85,   86,   67,   68,  620,
  580,  580, 1064, 1131,  315,  316,  690, 1222, 1241,  660,
  661,  662,  356,  691,   73,   74,   75,   76,  580, 1235,
 1236,  692,  419, 1239, 1240,  324, 1007, 1008, 1009,  693,
  322,  694, 1074, 1063,  695,  323, 1363,  343, 1365, 1367,
 1368,  696,  341, 1371, 1373,  697, 1376,  342, 1378, 1380,
 1381,   82,   83, 1384, 1386,   92,  698,  699,   89,  315,
  316,  701,  702,  419,   84,   85,   86,  312,  313,  314,
 1075, 1064,  785,  703,  304,  334,  335,  704, 1207,  705,
 1209,  706,  707,  708, 1214, 1215,  709,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,   84,
   85,   86,  400,  710,  307,  711,  308, 1242,  712,  713,
 1209, 1214,  714,  663,  664,  969,  975,  982,  985,  667,
  668,  669,  670,  671,  672,  673,  674,  675,  676,  988,
  860,  715, 1074,  991,  995,  998,  716, 1441, 1442, 1443,
 1444, 1445, 1446, 1447, 1448, 1449, 1450, 1451, 1452, 1004,
  717,  860,  795,  647,  645,  786,  646,  681,  648,  682,
  860, 1011, 1012, 1228, 1229, 1015, 1012, 1232, 1233,  718,
 1075,  860,  719,  720,  685, 1132, 1133, 1134, 1135, 1136,
 1137, 1138, 1139,  721,  722,  723,  804,  724,  725,   84,
   85,   86,  726,  727,  357,  358,  359,  360,  361,  362,
  363,  364,  365,  366,  367,  368,  369,  370,  371,  372,
  373,  374,  375,  376,  377,  378,  379,  380,  381,  382,
  383,  384,  385,  386,  387,  388,  400,  728,  729,  730,
  731,  732,  911,  913,  733,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  734, 1076,  735,
  796,  797,  798,  799,  401,  402,  403,  404,  405,  406,
  407,  408,  409,  410,  411,  412,  736,  554,  555,  556,
  557,  558,  559,  560,  561,  343,  338,  836,  838,  840,
  341,  339,  787,  340,  851,  342,  737,  788,  861,  863,
  865,  867,  869,  650,  651,  652,  653,  654,  655,  656,
  657,  658,  659,  881,  883,  885,  887,  888,  738,  890,
  863,  739,  647,  645,  789,  646,  851,  648,  861,  863,
  865,  901,  903,  881,  883,  885,  887,  888,  740,  890,
  863,  741,  337,  742,  743,  744,  745,  746,  920,  747,
  923,  925,  923,  925,  765,  748,  925,  931,  925,  931,
  678,  749,  750,  936,  937,  938,  940,  942,  944,  751,
  752,  753, 1062,  356,  324,  945,  754, 1071, 1073,  322,
  320,  790,  321, 1082,  323,  755,  756,  757,  401,  402,
  403,  404,  405,  406,  407,  408,  409,  410,  411,  412,
  324,  319,  758,  679,  680,  322,  320,  325,  321,  759,
  323,  760,  761,  823,  792,  793,  794, 1131,  828,  829,
  830,  831,  832,  762,  834,  763,  764,  766,  767,  842,
  843,  844,  845,  768,  769,  770,  771,  859,  862,  864,
  772,  946,  870,  871,  872,  873, 1216,  773, 1218,  947,
  774,  775, 1223, 1224,  884,  886,  776,  318,  859,  891,
  777,  778,  356,  779,  837,  839,  841,  897,  898,  899,
  780,  781, 1218, 1223,  906,  907,  782,  783,  897,  909,
  910,  912,  914,  915,  916,  917,  918,  317,  784,  922,
  924,  926,  927,  928,  833,  930,  922,  932,  926,  948,
  949,  950,  842,  843,  845,  939,  941,  943,  414,  334,
  335,  324,  319,  951,  955,  428,  322,  320,  326,  321,
  956,  323,  958,  961,  962,  921,  964,  965, 1078,  436,
  966, 1263,  580,  580,  451,  486,  454,  473,  967,  968,
  483,  971, 1101,  972,  554,  555,  556,  557,  558,  559,
  560,  561,  324,  319,  501,  973,  963,  322,  320,  498,
  321,  974,  323,  977,  513,  519, 1213,  978,  318,  979,
  517, 1222, 1102,  532,  980,  528,  981,  984,  530,  542,
 1087,  987,  990, 1103,  540, 1101,  993,  846,  994, 1132,
 1133, 1134, 1135, 1136, 1137, 1138, 1139, 1104,  317,  997,
 1000, 1130, 1001, 1141, 1002, 1143, 1003, 1006, 1014,  318,
 1088, 1016, 1085,  564, 1122, 1102,  567, 1145, 1146, 1086,
 1148, 1089, 1078,  343,  315,  316, 1103,  584,  341,  339,
 1123,  340,  584,  342, 1124, 1090, 1125, 1294,  618,  317,
 1104, 1126, 1150, 1151, 1304,  453, 1127, 1129, 1153, 1154,
 1155,  485, 1158, 1291, 1159, 1160,  343,  338, 1161, 1163,
 1162,  341,  339, 1166,  340, 1167,  342, 1168, 1172, 1173,
  500, 1176, 1177,  512, 1179, 1171,  856,  235, 1180, 1183,
 1184, 1185, 1175, 1186, 1187, 1188, 1189, 1190, 1191, 1178,
 1193, 1194, 1195, 1196, 1197, 1198, 1182, 1077, 1199, 1080,
 1081, 1083, 1084, 1264, 1265, 1134, 1135, 1136, 1137, 1138,
 1139, 1200, 1201, 1110, 1202, 1113, 1114, 1203,  235, 1204,
 1117,  154, 1118, 1205, 1110, 1119, 1081, 1206, 1120, 1121,
 1208, 1013,  571, 1013, 1210,  315,  316, 1211,  586, 1212,
 1217, 1391, 1219,  601, 1394, 1396, 1397, 1220, 1399, 1401,
 1403, 1221, 1225, 1406, 1408, 1409, 1226, 1411, 1413,  401,
  402,  847,  848,  405,  406,  407,  408,  409,  410,  411,
  412, 1227, 1230, 1231, 1234, 1237,  315,  316, 1244, 1238,
 1246, 1222, 1248, 1242, 1311, 1251, 1312, 1253, 1313, 1314,
 1256, 1257, 1258, 1260, 1261, 1262, 1315,  631, 1270, 1316,
 1272, 1273, 1275, 1276, 1277, 1317, 1318, 1282, 1319, 1284,
 1285, 1286, 1287, 1289, 1290, 1320, 1297, 1321, 1299, 1300,
 1302, 1303, 1322, 1307, 1323, 1132, 1133, 1292, 1293, 1136,
 1137, 1138, 1139, 1324, 1325, 1453, 1454, 1455, 1456, 1457,
 1458, 1459, 1460, 1461, 1462, 1463, 1464, 1326,  357,  358,
  857,  858,  361,  362,  363,  364,  365,  366,  367,  368,
  369,  370,  371,  372,  373,  374,  375,  376,  377,  378,
  379,  380,  381,  382,  383,  384,  385,  386,  387,  388,
  334,  335,  324,  319,  492, 1327, 1328,  322,  320,  327,
  321, 1329,  323, 1330, 1331, 1332, 1333, 1334, 1335, 1359,
 1336, 1364, 1337,  507, 1338, 1369, 1370, 1339, 1340, 1375,
 1341, 1377, 1342, 1343,  525, 1382, 1383, 1344, 1245, 1345,
 1392, 1393,  537, 1346, 1347, 1398,  224, 1348,  547, 1404,
 1405, 1349, 1350,  235, 1410, 1351,  400, 1415, 1416,  318,
  235,  235,  235,  235,  235,  235,  235,  235,  235,  235,
  235,  235,  235,  235,  235,  235,  235,  235,  235,  235,
  235,  235, 1352,  224, 1353, 1354, 1417,  224, 1418,  317,
  224, 1419,  590,  594, 1420,  235,  604,  607,  611,  615,
  235,  235,  235,  235,  235,  235,  235,  235,  235,  235,
  235,  235,  235,  235,  235,  235,  235,  235,  235,  235,
  235,  235,  235,  235,  235,  235,  235,  235,  235,  235,
  235,  235,  235,  235,  235,  235,  235,  235,  235,  235,
  235,  235,  235,  235,  235,  235,  235,  235,  235,  235,
  235,  235,  235,  235,  235,  235,  235,  235,  235,  235,
  235,  235,  235,  235,  235,  235,  235,  235,  235,  235,
  235,  235,  235,  235,  235,  235,  235,  235,  235,  235,
  235,  235,  235,  235,  235,  235,  235,  235,  235,  235,
  235,  235,  235,  235,  235,  235,  235,  235,  235,  235,
  235,  235,  235,  235,  235,  235,  235,  235,  235,  235,
  235,  235,  235,  235,  235,  235,  235, 1421, 1422, 1423,
 1424,  824,  825,  826,  827, 1425,  315,  316,  401,  402,
  403,  404,  405,  406,  407,  408,  409,  410,  411,  412,
  849,  852,  854,  855, 1426, 1427, 1428,  866,  868, 1429,
 1430, 1431, 1432,  874,  875,  876,  877,  878,  879,  880,
  882, 1433, 1434,  849,  889, 1435, 1436,  866,  868,  892,
  893,  894,  895,  896, 1437, 1438, 1439,  900,  902,  904,
  905, 1440,  671,  895,  908,  805,  210,  835,  596,  598,
  543,  539,  534,    0,  226, 1010,    0,    0,    0,    0,
    0,  929,  224,    0,    0,    0,  933,  934,  935,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  226,    0,    0,    0,  226,    0,    0,  226,    0,
    0,    0,    0,    0,  224,    0,    0,    0,    0,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  211,    0,    0,  806,
  807,  808,  809,  810,  811,  812,  813,  814,  815,  816,
  817,  818,  290,  291,  292,  293,  294,  295,  296,  297,
  298,  299,  300,  301,    0,    0, 1061,    0, 1066,   88,
 1069, 1070,    0,    0,   87,    0,    0,  211,  224,    0,
  211,    0,    0, 1061, 1096,    0, 1099, 1100,    0,    0,
    0,    0,    0,  579, 1115,    0, 1116,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  226,    0,    0,    0,    0,    0,    0,  226,  226,  226,
  226,  226,  226,  226,  226,  226,  226,  226,  226,  226,
  226,  226,  226,  226,  226,  226,  226,  226,  226,    0,
    0,    0,    0,    0,    0,   89,    0,    0,    0,    0,
    0,    0,  226,    0,    0,    0,    0,  226,  226,  226,
  226,  226,  226,  226,  226,  226,  226,  226,  226,  226,
  226,  226,  226,  226,  226,  226,  226,  226,  226,  226,
  226,  226,  226,  226,  226,  226,  226,  226,  226,  226,
  226,  226,  226,  226,  226,  226,  226,  226,  226,  226,
  226,  226,  226,  226,  226,  226,  226,  226,  226,  226,
  226,  226,  226,  226,  226,  226,  226,  226,  226,  226,
  226,  226,  226,  226,  226,  226,  226,  226,  226,  226,
  226,  226,  226,  226,  226,  226,  226,  226,  226,  226,
  226,  226,  226,  226,  226,  226,  226,  226,  226,  226,
  226,  226,  226,  226,  226,  226,  226,  226,  226,  226,
  226,  226,  226,  226,  226,  226,  226,  226,  226,  226,
  226,  226,  226,  226,  224,  419,   84,   85,   86,    0,
    0,    0,  211, 1243,    0,    0,    0,    0,    0,  211,
  211,  211,  211,  211,  211,  211,  211,  211,  211,  211,
  211,  211,  211,  211,  211,  211,  211,  211,  211,  211,
  211,    0,    0,    0,    0,  224,  226,    0,  224,  846,
    0,    0,    0,    0,  211,    0,    0,    0,    0,  211,
  211,  211,  211,  211,  211,  211,  211,  211,  211,  211,
  211,  211,  211,  211,  211,  211,  211,  211,  211,  211,
  211,  211,  211,  211,  211,  211,  211,  211,  211,  211,
  211,  211,  211,  211,  211,  211,  211,  211,  211,  211,
  211,  211,  211,  211,  211,  211,  211,  211,  211,  211,
  211,  211,  211,  211,  211,  211,  211,  211,  211,  211,
  211,  211,  211,  211,  211,  211,  211,  211,  211,  211,
  211,  211,  211,  211,  211,  211,  211,  211,  211,  211,
  211,  211,  211,  211,  211,  211,  211,  211,  211,  211,
  211,  211,  211,  211,  211,  211,  211,  211,  211,  211,
  211,  211,  211,  211,  211,  211,  211,  211,  211,  211,
  211,  211,  211,  211,  211,  211,  167,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,   88,
    0,  356,    0,    0,   87,    0,    0,  167,  211,    0,
  167,  401,  402,  847,  848,  405,  406,  407,  408,  409,
  410,  411,  412,  389,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  224,    0,    0,    0,    0,    0,    0,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,    0,
    0,    0,    0,    0,    0,   89,    0,    0,    0,    0,
    0,    0,  224,    0,    0,    0,    0,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  224,  224,  224,  224,  224,  224,
  224,  224,  224,  224,  157,  355,   84,   85,   86,    0,
    0,    0,  167,    0,    0,    0,    0,    0,    0,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,    0,    0,    0,    0,  157,  224,    0,  157, 1092,
    0,    0,    0,    0,  167,    0,    0,    0,    0,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  158,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,   88,
    0,  356,    0,    0,   87,    0,    0,  158,  173,    0,
  158, 1093, 1094,  403,  404,  405,  406,  407,  408,  409,
  410,  411,  412,  389,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  157,    0,    0,    0,    0,    0,    0,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,    0,
    0,    0,    0,    0,    0,   89,    0,    0,    0,    0,
    0,    0,  157,    0,    0,    0,    0,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  154,  419,   84,   85,   86,    0,
    0,    0,  158,    0,    0,    0,    0,    0,    0,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,    0,    0,    0,    0,  154,  171,    0,  154,    0,
    0,    0,    0,    0,  158,    0,    0,    0,    0,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  124,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,   88,
    0,  356,    0,    0,   87,    0,    0,  124,  172,    0,
  124,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  389,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  356,    0,    0,    0,    0,
  154,    0,    0,    0,    0,    0,    0,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,    0,
    0,    0,    0,    0,    0,   89,    0,    0,    0,    0,
    0,    0,  154,    0,    0,    0,    0,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  125,  459,   84,   85,   86,    0,
    0,    0,  124,    0,    0,    0,    0,    0,    0,  124,
  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
  124,    0,    0,    0,    0,  125,  173,    0,  125,  419,
    0,    0,    0,    0,  124,    0,    0,    0,    0,  124,
  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,
  124,  124,  124,  124,  124,  124,  220,  460,  461,  462,
  463,  464,  465,  466,  467,  468,  469,  470,  471,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,    0,
    0,    0,    0,    0,    0,    0,    0,  220,  171,    0,
  220,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,    0,    0,    0,    0,    0,    0,    0,
  125,    0,    0,    0,    0,    0,    0,  125,  125,  125,
  125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
  125,  125,  125,  125,  125,  125,  125,  125,  125,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  125,    0,    0,    0,    0,  125,  125,  125,
  125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
  125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
  125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
  125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
  125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
  125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
  125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
  125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
  125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
  125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
  125,  125,  125,  125,  125,  125,  125,  125,  125,  125,
  125,  125,  125,  125,  221,    0,    0,    0,    0,    0,
    0,    0,  220,    0,    0,    0,    0,    0,    0,  220,
  220,  220,  220,  220,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  220,  220,  220,  220,
  220,    0,    0,    0,    0,  221,  172,    0,  221,    0,
    0,    0,    0,  419,  220,    0,    0,    0,    0,  220,
  220,  220,  220,  220,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  167,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  324,  319,    0,    0,  649,  322,  320,    0,
  321,    0,  323,    0,    0,    0,    0,  167,  220,   88,
    0,    0,    0,    0,   87,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,    0,    0,  318,
  221,    0,    0,    0,    0,    0,    0,  221,  221,  221,
  221,  221,  221,  221,  221,  221,  221,  221,  221,  221,
  221,  221,  221,  221,  221,  221,  221,  221,  221,  317,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  221,    0,    0,   89,    0,  221,  221,  221,
  221,  221,  221,  221,  221,  221,  221,  221,  221,  221,
  221,  221,  221,  221,  221,  221,  221,  221,  221,  221,
  221,  221,  221,  221,  221,  221,  221,  221,  221,  221,
  221,  221,  221,  221,  221,  221,  221,  221,  221,  221,
  221,  221,  221,  221,  221,  221,  221,  221,  221,  221,
  221,  221,  221,  221,  221,  221,  221,  221,  221,  221,
  221,  221,  221,  221,  221,  221,  221,  221,  221,  221,
  221,  221,  221,  221,  221,  221,  221,  221,  221,  221,
  221,  221,  221,  221,  221,  221,  221,  221,  221,  221,
  221,  221,  221,  221,  221,  221,  221,  221,  221,  221,
  221,  221,  221,  221,  221,  221,  221,  221,  221,  221,
  221,  221,  221,  221,  155,    0,  315,  316,    0,    0,
    0,    0,  167,    0,    0,    0,   84,   85,   86,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,    0,    0,    0,    0,  155,  221,  856,    0,    0,
    0,    0,    0,    0,  167,    0,    0,    0,    0,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  156,    0,    0,  344,
  345,  346,  347,  348,  349,  350,  351,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  343,  338,    0,    0,  666,  341,  339,    0,
  340,    0,  342,    0,    0,    0,    0,  156,  170,  357,
  358,  857,  858,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,    0,    0,    0,    0,    0,    0,    0,    0,  337,
  155,    0,    0,    0,    0,    0,    0,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  336,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  155,    0,    0,    0,    0,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  154,    0,  334,  335,    0,    0,
    0,    0,  156,    0,    0,    0,    0,    0,    0,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,    0,    0,    0,    0,  154,  168, 1106,    0,    0,
    0,    0,    0,    0,  156,    0,    0,    0,    0,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  122,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  343,  338,    0,    0,    0,  341,  339,  952,
  340,    0,  342,    0,    0,    0,    0,  122,  169, 1107,
 1108,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,    0,    0,    0,    0,    0,    0,    0,    0,  337,
  154,    0,    0,    0,    0,    0,    0,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  336,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  154,    0,    0,    0,    0,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  123,    0,  334,  335,    0,    0,
    0,    0,  122,    0,    0,    0,    0,    0,    0,  122,
  122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
  122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
  122,    0,    0,    0,    0,  123,  170,    0,    0,    0,
    0,    0,    0,    0,  122,    0,    0,    0,    0,  122,
  122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
  122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
  122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
  122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
  122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
  122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
  122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
  122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
  122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
  122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
  122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
  122,  122,  122,  122,  122,  122,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  343,  338,    0,    0,    0,  341,  339,  953,
  340,    0,  342,    0,    0,    0,    0,    0,  168,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  337,
  123,    0,    0,    0,    0,    0,    0,  123,  123,  123,
  123,  123,  123,  123,  123,  123,  123,  123,  123,  123,
  123,  123,  123,  123,  123,  123,  123,  123,  123,  336,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  123,    0,    0,    0,    0,  123,  123,  123,
  123,  123,  123,  123,  123,  123,  123,  123,  123,  123,
  123,  123,  123,  123,  123,  123,  123,  123,  123,  123,
  123,  123,  123,  123,  123,  123,  123,  123,  123,  123,
  123,  123,  123,  123,  123,  123,  123,  123,  123,  123,
  123,  123,  123,  123,  123,  123,  123,  123,  123,  123,
  123,  123,  123,  123,  123,  123,  123,  123,  123,  123,
  123,  123,  123,  123,  123,  123,  123,  123,  123,  123,
  123,  123,  123,  123,  123,  123,  123,  123,  123,  123,
  123,  123,  123,  123,  123,  123,  123,  123,  123,  123,
  123,  123,  123,  123,  123,  123,  123,  123,  123,  123,
  123,  123,  123,  123,  123,  123,  123,  123,  123,  123,
  123,  123,  123,  123,   80,    0,  334,  335,    0,    0,
    0,    0,  343,  338,    0,    0,    0,  341,  339,  954,
  340,    0,  342,  343,  338,  447,    0, 1128,  341,  339,
    0,  340,    0,  342,    0,    0,  491,    0,    0,  497,
    0,    0,   80,    0,    0,   80,  169,   80,   80,   80,
    0,    0,    0,    0,    0,  506,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  343,  338,    0,    0,  337,
  341,  339,    0,  340,    0,  342,    0,    0,    0,    0,
  337,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  394,   80,  336,
    0,  418,  423,    0,  425,    0,  427,    0,    0,  430,
  336,    0,    0,  434, 1021,    0,    0,    0,    0,    0,
    0, 1022,  337,  587,    0,  593,  597,    0,   80,  476,
  610,  614, 1023,    0,  624, 1024,  434,    0,    0,    0,
    0,    0,    0, 1025, 1026,    0, 1027,    0,    0,    0,
    0,    0,  336,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  524,    0, 1028,    0,    0,   81,    0,
 1029,  524,    0,    0,    0,    0, 1030,  524,    0,    0,
    0,    0,    0,    0, 1031,    0,  553,    0,    0,    0,
 1032,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   81,    0,    0,   81,
    0,   81,   81,   81,    0,    0,    0,    0,    0,    0,
  434,  524,    0,  434,    0,  524,  524,    0,    0,    0,
    0,  623,  627,    0,  628,    0,  334,  335,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  334,  335,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   80,    0,   81,    0,    0,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,  334,
  335,    0,   81,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   80,    0,    0,    0,    0,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   80,   80,   80,   80,   80,   80,
   80,   80,   80,   80,   81,    0,  700,    0,    0,   81,
   81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,   81,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   81,    0,    0,    0,
    0,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,   81,   81,   81,   81,   81,   81,   81,   81,
   81,   81,   81,   81,   81,   81,   81,   81,  105,    7,
    8,    9,   10,   11,   12,   13,   14,   15,   16,   17,
   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
   28,    0,    0,    0,    0,    0,    0, 1036,    0,    0,
    0,    0,    0, 1037,    0,    0,  105,    0,    0,  105,
    0,  105,  105,  105, 1038,    0,    0, 1039, 1040,    0,
 1041,    0,    0, 1042,    0,    0, 1043,    0, 1044,    0,
    0,    0,    0, 1045,    0, 1046, 1047,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0, 1048,    0,    0,
    0,    0,    0, 1049,    0,    0,    0,    0, 1050, 1051,
    0, 1052,  105,    0, 1053,    0, 1054,    0,    0, 1055,
    0,    0, 1056,    0, 1057,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  105,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   29,    0,    0,    0,    0,
    0,    0,  106,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  106,    0,    0,  106,    0,  106,  106,  106,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  105,    0,  106,    0,    0,  105,
  105,  105,  105,  105,  105,  105,  105,  105,  105,  105,
  105,  105,  105,  105,  105,  105,  105,  105,  105,  105,
  105,  105,  105,    0,    0,    0,  106,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  105,    0,    0,    0,
    0,  105,  105,  105,  105,  105,  105,  105,  105,  105,
  105,  105,  105,  105,  105,  105,  105,  105,  105,  105,
  105,  105,  105,  105,  105,  105,  105,  105,  105,  105,
  105,  105,  105,  105,  105,  105,  105,  105,  105,  105,
  105,  105,  105,  105,  105,  105,  105,  105,  105,  105,
  105,  105,  105,  105,  105,  105,  105,  105,  105,  105,
  105,  105,  105,  105,  105,  105,  105,  105,  105,  105,
  105,  105,  105,  105,  105,  105,  105,  105,  105,  105,
  105,  105,  105,  105,  105,  105,  105,  105,  105,  105,
  105,  105,  105,  105,  105,  105,  105,  105,  105,  105,
  105,  105,  105,  105,  105,  105,  105,  105,  105,  105,
  105,  105,  105,  105,  105,  105,  105,  105,  106,    0,
    0,    0,    0,  106,  106,  106,  106,  106,  106,  106,
  106,  106,  106,  106,  106,  106,  106,  106,  106,  106,
  106,  106,  106,  106,  106,  106,  106,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  106,    0,    0,    0,    0,  106,  106,  106,  106,  106,
  106,  106,  106,  106,  106,  106,  106,  106,  106,  106,
  106,  106,  106,  106,  106,  106,  106,  106,  106,  106,
  106,  106,  106,  106,  106,  106,  106,  106,  106,  106,
  106,  106,  106,  106,  106,  106,  106,  106,  106,  106,
  106,  106,  106,  106,  106,  106,  106,  106,  106,  106,
  106,  106,  106,  106,  106,  106,  106,  106,  106,  106,
  106,  106,  106,  106,  106,  106,  106,  106,  106,  106,
  106,  106,  106,  106,  106,  106,  106,  106,  106,  106,
  106,  106,  106,  106,  106,  106,  106,  106,  106,  106,
  106,  106,  106,  106,  106,  106,  106,  106,  106,  106,
  106,  106,  106,  106,  106,  106,  106,  106,  106,  106,
  106,  106,   88,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   88,    0,    0,   88,    0,    0,   88,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   88,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   88,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   89,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   89,    0,    0,   89,    0,    0,
   89,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   88,    0,
   89,    0,    0,   88,   88,   88,   88,   88,   88,   88,
   88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
   88,   88,   88,   88,   88,   88,   88,    0,    0,    0,
   89,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   88,    0,    0,    0,    0,   88,   88,   88,   88,   88,
   88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
   88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
   88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
   88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
   88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
   88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
   88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
   88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
   88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
   88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
   88,   88,   88,   88,   88,   88,   88,   88,   88,   88,
   88,   88,   89,    0,    0,    0,    0,   89,   89,   89,
   89,   89,   89,   89,   89,   89,   89,   89,   89,   89,
   89,   89,   89,   89,   89,   89,   89,   89,   89,   89,
   89,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   89,    0,    0,    0,    0,   89,
   89,   89,   89,   89,   89,   89,   89,   89,   89,   89,
   89,   89,   89,   89,   89,   89,   89,   89,   89,   89,
   89,   89,   89,   89,   89,   89,   89,   89,   89,   89,
   89,   89,   89,   89,   89,   89,   89,   89,   89,   89,
   89,   89,   89,   89,   89,   89,   89,   89,   89,   89,
   89,   89,   89,   89,   89,   89,   89,   89,   89,   89,
   89,   89,   89,   89,   89,   89,   89,   89,   89,   89,
   89,   89,   89,   89,   89,   89,   89,   89,   89,   89,
   89,   89,   89,   89,   89,   89,   89,   89,   89,   89,
   89,   89,   89,   89,   89,   89,   89,   89,   89,   89,
   89,   89,   89,   89,   89,   89,   89,   89,   89,   89,
   89,   89,   89,   89,   89,   89,  113,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  113,    0,    0,  113,    0,    0,
  113,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  113,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  113,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  114,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  114,    0,
    0,  114,    0,    0,  114,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  113,    0,  114,    0,    0,  113,  113,  113,
  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
  113,    0,    0,    0,  114,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  113,    0,    0,    0,    0,  113,
  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
  113,  113,  113,  113,  113,  113,  114,    0,    0,    0,
    0,  114,  114,  114,  114,  114,  114,  114,  114,  114,
  114,  114,  114,  114,  114,  114,  114,  114,  114,  114,
  114,  114,  114,  114,  114,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  114,    0,
    0,    0,    0,  114,  114,  114,  114,  114,  114,  114,
  114,  114,  114,  114,  114,  114,  114,  114,  114,  114,
  114,  114,  114,  114,  114,  114,  114,  114,  114,  114,
  114,  114,  114,  114,  114,  114,  114,  114,  114,  114,
  114,  114,  114,  114,  114,  114,  114,  114,  114,  114,
  114,  114,  114,  114,  114,  114,  114,  114,  114,  114,
  114,  114,  114,  114,  114,  114,  114,  114,  114,  114,
  114,  114,  114,  114,  114,  114,  114,  114,  114,  114,
  114,  114,  114,  114,  114,  114,  114,  114,  114,  114,
  114,  114,  114,  114,  114,  114,  114,  114,  114,  114,
  114,  114,  114,  114,  114,  114,  114,  114,  114,  114,
  114,  114,  114,  114,  114,  114,  114,  114,  114,  114,
   85,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   85,    0,
    0,   85,    0,    0,   85,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   85,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   85,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  110,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  110,    0,    0,  110,    0,    0,  110,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   85,    0,  110,    0,
    0,    0,    0,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,    0,    0,    0,  110,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   85,    0,
    0,    0,    0,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
  110,    0,    0,    0,    0,    0,    0,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  110,    0,    0,    0,    0,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,   87,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   87,    0,    0,   87,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   87,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   87,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  112,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  112,
    0,    0,  112,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   87,    0,  112,    0,    0,    0,    0,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,    0,
    0,    0,  112,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   87,    0,    0,    0,    0,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,  112,    0,    0,    0,    0,    0,
    0,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  112,    0,    0,    0,
    0,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,   95,    0,
    0,    0,    0,    0,    0,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   95,
    2,   95,   95,   95,    0,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,   96,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   96,    0,   96,   96,   96,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   95,    0,    0,    0,    0,    0,
    0,   95,   95,   95,   95,   95,   95,   95,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   95,   95,   95,
   95,   95,   95,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   95,    0,    0,    0,
    0,   95,   95,   95,   95,   95,   95,   95,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   95,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   95,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   95,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   95,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   95,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   95,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   95,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   95,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   95,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   95,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   95,   96,    0,
    0,    0,    0,    0,    0,   96,   96,   96,   96,   96,
   96,   96,   96,   96,   96,   96,   96,   96,   96,   96,
   96,   96,   96,   96,   96,   96,   96,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   96,    0,    0,    0,    0,   96,   96,   96,   96,   96,
   96,   96,   96,   96,   96,   96,   96,   96,   96,   96,
   96,   96,   96,   96,   96,   96,   96,   96,   96,   96,
   96,   96,   96,   96,   96,   96,   96,   96,   96,   96,
   96,   96,   96,   96,   96,   96,   96,   96,   96,   96,
   96,   96,   96,   96,   96,   96,   96,   96,   96,   96,
   96,   96,   96,   96,   96,   96,   96,   96,   96,   96,
   96,   96,   96,   96,   96,   96,   96,   96,   96,   96,
   96,   96,   96,   96,   96,   96,   96,   96,   96,   96,
   96,   96,   96,   96,   96,   96,   96,   96,   96,   96,
   96,   96,   96,   96,   96,   96,   96,   96,   96,   96,
   96,   96,   96,   96,   96,   96,   96,   96,   96,   96,
   96,   96,   86,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   86,    0,    0,   86,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   86,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  111,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  111,    0,    0,
  111,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   86,    0,
    0,    0,    0,    0,    0,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,    0,    0,    0,
  111,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   86,    0,    0,    0,    0,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,  111,    0,  209,    0,    0,    0,    0,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  111,  209,    0,    0,  209,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,    0,    0,  167,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  167,
    0,    0,  154,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  209,    0,    0,    0,    0,    0,    0,  209,  209,  209,
  209,  209,  209,  209,  209,  209,  209,  209,  209,  209,
  209,  209,  209,  209,  209,  209,  209,  209,  209,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  209,    0,    0,    0,    0,  209,  209,  209,
  209,  209,  209,  209,  209,  209,  209,  209,  209,  209,
  209,  209,  209,  209,  209,  209,  209,  209,  209,  209,
  209,  209,  209,  209,  209,  209,  209,  209,  209,  209,
  209,  209,  209,  209,  209,  209,  209,  209,  209,  209,
  209,  209,  209,  209,  209,  209,  209,  209,  209,  209,
  209,  209,  209,  209,  209,  209,  209,  209,  209,  209,
  209,  209,  209,  209,  209,  209,  209,  209,  209,  209,
  209,  209,  209,  209,  209,  209,  209,  209,  209,  209,
  209,  209,  209,  209,  209,  209,  209,  209,  209,  209,
  209,  209,  209,  209,  209,  209,  209,  209,  209,  209,
  209,  209,  209,  209,  209,  209,  209,  209,  209,  209,
  209,  209,  209,  209,  167,    0,  155,    0,    0,    0,
    0,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  167,  155,    0,    0,
  122,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,  167,  167,
  167,  167,  167,  167,  167,  167,  167,  167,    0,    0,
  156,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  156,    0,    0,  123,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  155,    0,    0,    0,    0,    0,    0,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  155,    0,    0,    0,    0,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  155,  155,  155,  155,
  155,  155,  155,  155,  155,  155,  156,    0,  157,    0,
    0,    0,    0,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  156,  157,
    0,    0,  124,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,  156,  156,  156,  156,  156,  156,  156,  156,
    0,    0,  158,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  158,    0,    0,  125,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  157,    0,    0,    0,    0,    0,
    0,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  157,    0,    0,    0,
    0,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  157,  157,
  157,  157,  157,  157,  157,  157,  157,  157,  158,    0,
  159,    0,    0,    0,    0,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  158,  159,    0,    0,  126,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,  158,  158,  158,  158,  158,  158,  158,  158,
  158,  158,    0,    0,  160,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  160,    0,    0,  127,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  159,    0,    0,    0,
    0,    0,    0,  159,  159,  159,  159,  159,  159,  159,
  159,  159,  159,  159,  159,  159,  159,  159,  159,  159,
  159,  159,  159,  159,  159,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  159,    0,
    0,    0,    0,  159,  159,  159,  159,  159,  159,  159,
  159,  159,  159,  159,  159,  159,  159,  159,  159,  159,
  159,  159,  159,  159,  159,  159,  159,  159,  159,  159,
  159,  159,  159,  159,  159,  159,  159,  159,  159,  159,
  159,  159,  159,  159,  159,  159,  159,  159,  159,  159,
  159,  159,  159,  159,  159,  159,  159,  159,  159,  159,
  159,  159,  159,  159,  159,  159,  159,  159,  159,  159,
  159,  159,  159,  159,  159,  159,  159,  159,  159,  159,
  159,  159,  159,  159,  159,  159,  159,  159,  159,  159,
  159,  159,  159,  159,  159,  159,  159,  159,  159,  159,
  159,  159,  159,  159,  159,  159,  159,  159,  159,  159,
  159,  159,  159,  159,  159,  159,  159,  159,  159,  159,
  160,    0,  161,    0,    0,    0,    0,  160,  160,  160,
  160,  160,  160,  160,  160,  160,  160,  160,  160,  160,
  160,  160,  160,  160,  160,  160,  160,  160,  160,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  160,  161,    0,    0,  128,  160,  160,  160,
  160,  160,  160,  160,  160,  160,  160,  160,  160,  160,
  160,  160,  160,  160,  160,  160,  160,  160,  160,  160,
  160,  160,  160,  160,  160,  160,  160,  160,  160,  160,
  160,  160,  160,  160,  160,  160,  160,  160,  160,  160,
  160,  160,  160,  160,  160,  160,  160,  160,  160,  160,
  160,  160,  160,  160,  160,  160,  160,  160,  160,  160,
  160,  160,  160,  160,  160,  160,  160,  160,  160,  160,
  160,  160,  160,  160,  160,  160,  160,  160,  160,  160,
  160,  160,  160,  160,  160,  160,  160,  160,  160,  160,
  160,  160,  160,  160,  160,  160,  160,  160,  160,  160,
  160,  160,  160,  160,  160,  160,  160,  160,  160,  160,
  160,  160,  160,  160,    0,    0,  162,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  162,    0,    0,
  129,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  161,    0,
    0,    0,    0,    0,    0,  161,  161,  161,  161,  161,
  161,  161,  161,  161,  161,  161,  161,  161,  161,  161,
  161,  161,  161,  161,  161,  161,  161,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  161,    0,    0,    0,    0,  161,  161,  161,  161,  161,
  161,  161,  161,  161,  161,  161,  161,  161,  161,  161,
  161,  161,  161,  161,  161,  161,  161,  161,  161,  161,
  161,  161,  161,  161,  161,  161,  161,  161,  161,  161,
  161,  161,  161,  161,  161,  161,  161,  161,  161,  161,
  161,  161,  161,  161,  161,  161,  161,  161,  161,  161,
  161,  161,  161,  161,  161,  161,  161,  161,  161,  161,
  161,  161,  161,  161,  161,  161,  161,  161,  161,  161,
  161,  161,  161,  161,  161,  161,  161,  161,  161,  161,
  161,  161,  161,  161,  161,  161,  161,  161,  161,  161,
  161,  161,  161,  161,  161,  161,  161,  161,  161,  161,
  161,  161,  161,  161,  161,  161,  161,  161,  161,  161,
  161,  161,  162,    0,  163,    0,    0,    0,    0,  162,
  162,  162,  162,  162,  162,  162,  162,  162,  162,  162,
  162,  162,  162,  162,  162,  162,  162,  162,  162,  162,
  162,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  162,  163,    0,    0,  130,  162,
  162,  162,  162,  162,  162,  162,  162,  162,  162,  162,
  162,  162,  162,  162,  162,  162,  162,  162,  162,  162,
  162,  162,  162,  162,  162,  162,  162,  162,  162,  162,
  162,  162,  162,  162,  162,  162,  162,  162,  162,  162,
  162,  162,  162,  162,  162,  162,  162,  162,  162,  162,
  162,  162,  162,  162,  162,  162,  162,  162,  162,  162,
  162,  162,  162,  162,  162,  162,  162,  162,  162,  162,
  162,  162,  162,  162,  162,  162,  162,  162,  162,  162,
  162,  162,  162,  162,  162,  162,  162,  162,  162,  162,
  162,  162,  162,  162,  162,  162,  162,  162,  162,  162,
  162,  162,  162,  162,  162,  162,  162,  162,  162,  162,
  162,  162,  162,  162,  162,  162,    0,    0,  164,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  164,
    0,    0,  131,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  163,    0,    0,    0,    0,    0,    0,  163,  163,  163,
  163,  163,  163,  163,  163,  163,  163,  163,  163,  163,
  163,  163,  163,  163,  163,  163,  163,  163,  163,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  163,    0,    0,    0,    0,  163,  163,  163,
  163,  163,  163,  163,  163,  163,  163,  163,  163,  163,
  163,  163,  163,  163,  163,  163,  163,  163,  163,  163,
  163,  163,  163,  163,  163,  163,  163,  163,  163,  163,
  163,  163,  163,  163,  163,  163,  163,  163,  163,  163,
  163,  163,  163,  163,  163,  163,  163,  163,  163,  163,
  163,  163,  163,  163,  163,  163,  163,  163,  163,  163,
  163,  163,  163,  163,  163,  163,  163,  163,  163,  163,
  163,  163,  163,  163,  163,  163,  163,  163,  163,  163,
  163,  163,  163,  163,  163,  163,  163,  163,  163,  163,
  163,  163,  163,  163,  163,  163,  163,  163,  163,  163,
  163,  163,  163,  163,  163,  163,  163,  163,  163,  163,
  163,  163,  163,  163,  164,    0,  165,    0,    0,    0,
    0,  164,  164,  164,  164,  164,  164,  164,  164,  164,
  164,  164,  164,  164,  164,  164,  164,  164,  164,  164,
  164,  164,  164,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  164,  165,    0,    0,
  132,  164,  164,  164,  164,  164,  164,  164,  164,  164,
  164,  164,  164,  164,  164,  164,  164,  164,  164,  164,
  164,  164,  164,  164,  164,  164,  164,  164,  164,  164,
  164,  164,  164,  164,  164,  164,  164,  164,  164,  164,
  164,  164,  164,  164,  164,  164,  164,  164,  164,  164,
  164,  164,  164,  164,  164,  164,  164,  164,  164,  164,
  164,  164,  164,  164,  164,  164,  164,  164,  164,  164,
  164,  164,  164,  164,  164,  164,  164,  164,  164,  164,
  164,  164,  164,  164,  164,  164,  164,  164,  164,  164,
  164,  164,  164,  164,  164,  164,  164,  164,  164,  164,
  164,  164,  164,  164,  164,  164,  164,  164,  164,  164,
  164,  164,  164,  164,  164,  164,  164,  164,    0,    0,
  166,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  166,    0,    0,  133,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  165,    0,    0,    0,    0,    0,    0,  165,
  165,  165,  165,  165,  165,  165,  165,  165,  165,  165,
  165,  165,  165,  165,  165,  165,  165,  165,  165,  165,
  165,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  165,    0,    0,    0,    0,  165,
  165,  165,  165,  165,  165,  165,  165,  165,  165,  165,
  165,  165,  165,  165,  165,  165,  165,  165,  165,  165,
  165,  165,  165,  165,  165,  165,  165,  165,  165,  165,
  165,  165,  165,  165,  165,  165,  165,  165,  165,  165,
  165,  165,  165,  165,  165,  165,  165,  165,  165,  165,
  165,  165,  165,  165,  165,  165,  165,  165,  165,  165,
  165,  165,  165,  165,  165,  165,  165,  165,  165,  165,
  165,  165,  165,  165,  165,  165,  165,  165,  165,  165,
  165,  165,  165,  165,  165,  165,  165,  165,  165,  165,
  165,  165,  165,  165,  165,  165,  165,  165,  165,  165,
  165,  165,  165,  165,  165,  165,  165,  165,  165,  165,
  165,  165,  165,  165,  165,  165,  166,    0,  208,    0,
    0,    0,    0,  166,  166,  166,  166,  166,  166,  166,
  166,  166,  166,  166,  166,  166,  166,  166,  166,  166,
  166,  166,  166,  166,  166,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  166,  208,
    0,    0,  208,  166,  166,  166,  166,  166,  166,  166,
  166,  166,  166,  166,  166,  166,  166,  166,  166,  166,
  166,  166,  166,  166,  166,  166,  166,  166,  166,  166,
  166,  166,  166,  166,  166,  166,  166,  166,  166,  166,
  166,  166,  166,  166,  166,  166,  166,  166,  166,  166,
  166,  166,  166,  166,  166,  166,  166,  166,  166,  166,
  166,  166,  166,  166,  166,  166,  166,  166,  166,  166,
  166,  166,  166,  166,  166,  166,  166,  166,  166,  166,
  166,  166,  166,  166,  166,  166,  166,  166,  166,  166,
  166,  166,  166,  166,  166,  166,  166,  166,  166,  166,
  166,  166,  166,  166,  166,  166,  166,  166,  166,  166,
  166,  166,  166,  166,  166,  166,  166,  166,  166,  166,
    0,    0,  170,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  170,    0,    0,  182,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  208,    0,    0,    0,    0,    0,
    0,  208,  208,  208,  208,  208,  208,  208,  208,  208,
  208,  208,  208,  208,  208,  208,  208,  208,  208,  208,
  208,  208,  208,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  208,    0,    0,    0,
    0,  208,  208,  208,  208,  208,  208,  208,  208,  208,
  208,  208,  208,  208,  208,  208,  208,  208,  208,  208,
  208,  208,  208,  208,  208,  208,  208,  208,  208,  208,
  208,  208,  208,  208,  208,  208,  208,  208,  208,  208,
  208,  208,  208,  208,  208,  208,  208,  208,  208,  208,
  208,  208,  208,  208,  208,  208,  208,  208,  208,  208,
  208,  208,  208,  208,  208,  208,  208,  208,  208,  208,
  208,  208,  208,  208,  208,  208,  208,  208,  208,  208,
  208,  208,  208,  208,  208,  208,  208,  208,  208,  208,
  208,  208,  208,  208,  208,  208,  208,  208,  208,  208,
  208,  208,  208,  208,  208,  208,  208,  208,  208,  208,
  208,  208,  208,  208,  208,  208,  208,  208,  170,    0,
  168,    0,    0,    0,    0,  170,  170,  170,  170,  170,
  170,  170,  170,  170,  170,  170,  170,  170,  170,  170,
  170,  170,  170,  170,  170,  170,  170,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  170,  168,    0,    0,  174,  170,  170,  170,  170,  170,
  170,  170,  170,  170,  170,  170,  170,  170,  170,  170,
  170,  170,  170,  170,  170,  170,  170,  170,  170,  170,
  170,  170,  170,  170,  170,  170,  170,  170,  170,  170,
  170,  170,  170,  170,  170,  170,  170,  170,  170,  170,
  170,  170,  170,  170,  170,  170,  170,  170,  170,  170,
  170,  170,  170,  170,  170,  170,  170,  170,  170,  170,
  170,  170,  170,  170,  170,  170,  170,  170,  170,  170,
  170,  170,  170,  170,  170,  170,  170,  170,  170,  170,
  170,  170,  170,  170,  170,  170,  170,  170,  170,  170,
  170,  170,  170,  170,  170,  170,  170,  170,  170,  170,
  170,  170,  170,  170,  170,  170,  170,  170,  170,  170,
  170,  170,    0,    0,  169,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  169,    0,    0,  175,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  168,    0,    0,    0,
    0,    0,    0,  168,  168,  168,  168,  168,  168,  168,
  168,  168,  168,  168,  168,  168,  168,  168,  168,  168,
  168,  168,  168,  168,  168,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  168,    0,
    0,    0,    0,  168,  168,  168,  168,  168,  168,  168,
  168,  168,  168,  168,  168,  168,  168,  168,  168,  168,
  168,  168,  168,  168,  168,  168,  168,  168,  168,  168,
  168,  168,  168,  168,  168,  168,  168,  168,  168,  168,
  168,  168,  168,  168,  168,  168,  168,  168,  168,  168,
  168,  168,  168,  168,  168,  168,  168,  168,  168,  168,
  168,  168,  168,  168,  168,  168,  168,  168,  168,  168,
  168,  168,  168,  168,  168,  168,  168,  168,  168,  168,
  168,  168,  168,  168,  168,  168,  168,  168,  168,  168,
  168,  168,  168,  168,  168,  168,  168,  168,  168,  168,
  168,  168,  168,  168,  168,  168,  168,  168,  168,  168,
  168,  168,  168,  168,  168,  168,  168,  168,  168,  168,
  169,    0,  173,    0,    0,    0,    0,  169,  169,  169,
  169,  169,  169,  169,  169,  169,  169,  169,  169,  169,
  169,  169,  169,  169,  169,  169,  169,  169,  169,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  169,  173,    0,    0,  182,  169,  169,  169,
  169,  169,  169,  169,  169,  169,  169,  169,  169,  169,
  169,  169,  169,  169,  169,  169,  169,  169,  169,  169,
  169,  169,  169,  169,  169,  169,  169,  169,  169,  169,
  169,  169,  169,  169,  169,  169,  169,  169,  169,  169,
  169,  169,  169,  169,  169,  169,  169,  169,  169,  169,
  169,  169,  169,  169,  169,  169,  169,  169,  169,  169,
  169,  169,  169,  169,  169,  169,  169,  169,  169,  169,
  169,  169,  169,  169,  169,  169,  169,  169,  169,  169,
  169,  169,  169,  169,  169,  169,  169,  169,  169,  169,
  169,  169,  169,  169,  169,  169,  169,  169,  169,  169,
  169,  169,  169,  169,  169,  169,  169,  169,  169,  169,
  169,  169,  169,  169,    0,    0,  171,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  171,    0,    0,
  176,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  173,    0,
    0,    0,    0,    0,    0,  173,  173,  173,  173,  173,
  173,  173,  173,  173,  173,  173,  173,  173,  173,  173,
  173,  173,  173,  173,  173,  173,  173,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  173,    0,    0,    0,    0,  173,  173,  173,  173,  173,
  173,  173,  173,  173,  173,  173,  173,  173,  173,  173,
  173,  173,  173,  173,  173,  173,  173,  173,  173,  173,
  173,  173,  173,  173,  173,  173,  173,  173,  173,  173,
  173,  173,  173,  173,  173,  173,  173,  173,  173,  173,
  173,  173,  173,  173,  173,  173,  173,  173,  173,  173,
  173,  173,  173,  173,  173,  173,  173,  173,  173,  173,
  173,  173,  173,  173,  173,  173,  173,  173,  173,  173,
  173,  173,  173,  173,  173,  173,  173,  173,  173,  173,
  173,  173,  173,  173,  173,  173,  173,  173,  173,  173,
  173,  173,  173,  173,  173,  173,  173,  173,  173,  173,
  173,  173,  173,  173,  173,  173,  173,  173,  173,  173,
  173,  173,  171,    0,  172,    0,    0,    0,    0,  171,
  171,  171,  171,  171,  171,  171,  171,  171,  171,  171,
  171,  171,  171,  171,  171,  171,  171,  171,  171,  171,
  171,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  171,  172,    0,    0,  177,  171,
  171,  171,  171,  171,  171,  171,  171,  171,  171,  171,
  171,  171,  171,  171,  171,  171,  171,  171,  171,  171,
  171,  171,  171,  171,  171,  171,  171,  171,  171,  171,
  171,  171,  171,  171,  171,  171,  171,  171,  171,  171,
  171,  171,  171,  171,  171,  171,  171,  171,  171,  171,
  171,  171,  171,  171,  171,  171,  171,  171,  171,  171,
  171,  171,  171,  171,  171,  171,  171,  171,  171,  171,
  171,  171,  171,  171,  171,  171,  171,  171,  171,  171,
  171,  171,  171,  171,  171,  171,  171,  171,  171,  171,
  171,  171,  171,  171,  171,  171,  171,  171,  171,  171,
  171,  171,  171,  171,  171,  171,  171,  171,  171,  171,
  171,  171,  171,  171,  171,  171,    0,    0,  488,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  488,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  172,    0,    0,    0,    0,    0,    0,  172,  172,  172,
  172,  172,  172,  172,  172,  172,  172,  172,  172,  172,
  172,  172,  172,  172,  172,  172,  172,  172,  172,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  172,    0,    0,    0,    0,  172,  172,  172,
  172,  172,  172,  172,  172,  172,  172,  172,  172,  172,
  172,  172,  172,  172,  172,  172,  172,  172,  172,  172,
  172,  172,  172,  172,  172,  172,  172,  172,  172,  172,
  172,  172,  172,  172,  172,  172,  172,  172,  172,  172,
  172,  172,  172,  172,  172,  172,  172,  172,  172,  172,
  172,  172,  172,  172,  172,  172,  172,  172,  172,  172,
  172,  172,  172,  172,  172,  172,  172,  172,  172,  172,
  172,  172,  172,  172,  172,  172,  172,  172,  172,  172,
  172,  172,  172,  172,  172,  172,  172,  172,  172,  172,
  172,  172,  172,  172,  172,  172,  172,  172,  172,  172,
  172,  172,  172,  172,  172,  172,  172,  172,  172,  172,
  172,  172,  172,  172,  488,   76,    0,    0,    0,    0,
    0,  488,  488,  488,  488,  488,  488,  488,  488,  488,
  488,  488,  488,  488,  488,  488,  488,  488,  488,  488,
  488,  488,  488,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  488,    0,    0,   76,
    0,  488,  488,  488,  488,  488,  488,  488,  488,  488,
  488,  488,  488,  488,  488,  488,  488,  488,  488,  488,
  488,  488,  488,  488,  488,  488,  488,  488,  488,  488,
  488,  488,  488,  488,  488,  488,  488,  488,  488,  488,
  488,  488,  488,  488,  488,  488,  488,  488,  488,  488,
  488,  488,  488,  488,  488,  488,  488,  488,  488,  488,
  488,  488,  488,  488,  488,  488,  488,  488,  488,  488,
  488,  488,  488,  488,  488,  488,  488,  488,  488,  488,
  488,  488,  488,  488,  488,  488,  488,  488,  488,  488,
  488,  488,  488,  488,  488,  488,  488,  488,  488,  488,
  488,  488,  488,  488,  488,  488,  488,  488,  488,  488,
  488,  488,  488,  488,  488,  488,  488,  488,    0,   74,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   74,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   76,    0,    0,    0,    0,    0,    0,   76,   76,
   76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
   76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   76,    0,    0,    0,    0,   76,   76,
   76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
   76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
   76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
   76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
   76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
   76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
   76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
   76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
   76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
   76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
   76,   76,   76,   76,   76,   76,   76,   76,   76,   76,
   76,   76,   76,   76,   76,   74,    0,  352,    0,    0,
    0,    0,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   74,  352,    0,
    0,    0,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,    0,
    0,  657,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  657,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  352,    0,    0,    0,    0,    0,    0,
  352,  352,  352,  352,  352,  352,  352,  352,  352,  352,
  352,  352,  352,  352,  352,  352,  352,  352,  352,  352,
  352,  352,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  352,    0,    0,    0,    0,
  352,  352,  352,  352,  352,  352,  352,  352,  352,  352,
  352,  352,  352,  352,  352,  352,  352,  352,  352,  352,
  352,  352,  352,  352,  352,  352,  352,  352,  352,  352,
  352,  352,  352,  352,  352,  352,  352,  352,  352,  352,
  352,  352,  352,  352,  352,  352,  352,  352,  352,  352,
  352,  352,  352,  352,  352,  352,  352,  352,  352,  352,
  352,  352,  352,  352,  352,  352,  352,  352,  352,  352,
  352,  352,  352,  352,  352,  352,  352,  352,  352,  352,
  352,  352,  352,  352,  352,  352,  352,  352,  352,  352,
  352,  352,  352,  352,  352,  352,  352,  352,  352,  352,
  352,  352,  352,  352,  352,  352,  352,  352,  352,  352,
  352,  352,  352,  352,  352,  352,  352,  657,    0,  239,
    0,    0,    0,    0,  657,  657,  657,  657,  657,  657,
  657,  657,  657,  657,  657,  657,  657,  657,  657,  657,
  657,  657,  657,  657,  657,  657,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  657,
  239,    0,    0,    0,  657,  657,  657,  657,  657,  657,
  657,  657,  657,  657,  657,  657,  657,  657,  657,  657,
  657,  657,  657,  657,  657,  657,  657,  657,  657,  657,
  657,  657,  657,  657,  657,  657,  657,  657,  657,  657,
  657,  657,  657,  657,  657,  657,  657,  657,  657,  657,
  657,  657,  657,  657,  657,  657,  657,  657,  657,  657,
  657,  657,  657,  657,  657,  657,  657,  657,  657,  657,
  657,  657,  657,  657,  657,  657,  657,  657,  657,  657,
  657,  657,  657,  657,  657,  657,  657,  657,  657,  657,
  657,  657,  657,  657,  657,  657,  657,  657,  657,  657,
  657,  657,  657,  657,  657,  657,  657,  657,  657,  657,
  657,  657,  657,  657,  657,  657,  657,  657,  657,  657,
  657,    0,    0,  663,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  663,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  239,    0,    0,    0,    0,
    0,    0,  239,  239,  239,  239,  239,  239,  239,  239,
  239,  239,  239,  239,  239,  239,  239,  239,  239,  239,
  239,  239,  239,  239,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  239,    0,    0,
    0,    0,  239,  239,  239,  239,  239,  239,  239,  239,
  239,  239,  239,  239,  239,  239,  239,  239,  239,  239,
  239,  239,  239,  239,  239,  239,  239,  239,  239,  239,
  239,  239,  239,  239,  239,  239,  239,  239,  239,  239,
  239,  239,  239,  239,  239,  239,  239,  239,  239,  239,
  239,  239,  239,  239,  239,  239,  239,  239,  239,  239,
  239,  239,  239,  239,  239,  239,  239,  239,  239,  239,
  239,  239,  239,  239,  239,  239,  239,  239,  239,  239,
  239,  239,  239,  239,  239,  239,  239,  239,  239,  239,
  239,  239,  239,  239,  239,  239,  239,  239,  239,  239,
  239,  239,  239,  239,  239,  239,  239,  239,  239,  239,
  239,  239,  239,  239,  239,  239,  239,  239,  239,  663,
    0,  229,    0,    0,    0,    0,  663,  663,  663,  663,
  663,  663,  663,  663,  663,  663,  663,  663,  663,  663,
  663,  663,  663,  663,  663,  663,  663,  663,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  663,  229,    0,    0,    0,  663,  663,  663,  663,
  663,  663,  663,  663,  663,  663,  663,  663,  663,  663,
  663,  663,  663,  663,  663,  663,  663,  663,  663,  663,
  663,  663,  663,  663,  663,  663,  663,  663,  663,  663,
  663,  663,  663,  663,  663,  663,  663,  663,  663,  663,
  663,  663,  663,  663,  663,  663,  663,  663,  663,  663,
  663,  663,  663,  663,  663,  663,  663,  663,  663,  663,
  663,  663,  663,  663,  663,  663,  663,  663,  663,  663,
  663,  663,  663,  663,  663,  663,  663,  663,  663,  663,
  663,  663,  663,  663,  663,  663,  663,  663,  663,  663,
  663,  663,  663,  663,  663,  663,  663,  663,  663,  663,
  663,  663,  663,  663,  663,  663,  663,  663,  663,  663,
  663,  663,  663,    0,    0,  240,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  240,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  229,    0,    0,
    0,    0,    0,    0,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  229,  229,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  229,
    0,    0,    0,    0,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  229,  229,  229,  229,  229,  229,
  229,  229,  229,  229,  229,  229,  229,  229,  229,  229,
  229,  240,   75,    0,    0,    0,    0,    0,  240,  240,
  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,
  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  240,    0,    0,   75,    0,  240,  240,
  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,
  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,
  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,
  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,
  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,
  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,
  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,
  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,
  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,
  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,
  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,
  240,  240,  240,  240,  240,    0,  354,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  354,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   75,    0,
    0,    0,    0,    0,    0,   75,   75,   75,   75,   75,
   75,   75,   75,   75,   75,   75,   75,   75,   75,   75,
   75,   75,   75,   75,   75,   75,   75,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   75,    0,    0,    0,    0,   75,   75,   75,   75,   75,
   75,   75,   75,   75,   75,   75,   75,   75,   75,   75,
   75,   75,   75,   75,   75,   75,   75,   75,   75,   75,
   75,   75,   75,   75,   75,   75,   75,   75,   75,   75,
   75,   75,   75,   75,   75,   75,   75,   75,   75,   75,
   75,   75,   75,   75,   75,   75,   75,   75,   75,   75,
   75,   75,   75,   75,   75,   75,   75,   75,   75,   75,
   75,   75,   75,   75,   75,   75,   75,   75,   75,   75,
   75,   75,   75,   75,   75,   75,   75,   75,   75,   75,
   75,   75,   75,   75,   75,   75,   75,   75,   75,   75,
   75,   75,   75,   75,   75,   75,   75,   75,   75,   75,
   75,   75,   75,   75,   75,   75,   75,   75,   75,   75,
   75,   75,  354,    0,  230,    0,    0,    0,    0,  354,
  354,  354,  354,  354,  354,  354,  354,  354,  354,  354,
  354,  354,  354,  354,  354,  354,  354,  354,  354,  354,
  354,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  354,  230,    0,    0,    0,  354,
  354,  354,  354,  354,  354,  354,  354,  354,  354,  354,
  354,  354,  354,  354,  354,  354,  354,  354,  354,  354,
  354,  354,  354,  354,  354,  354,  354,  354,  354,  354,
  354,  354,  354,  354,  354,  354,  354,  354,  354,  354,
  354,  354,  354,  354,  354,  354,  354,  354,  354,  354,
  354,  354,  354,  354,  354,  354,  354,  354,  354,  354,
  354,  354,  354,  354,  354,  354,  354,  354,  354,  354,
  354,  354,  354,  354,  354,  354,  354,  354,  354,  354,
  354,  354,  354,  354,  354,  354,  354,  354,  354,  354,
  354,  354,  354,  354,  354,  354,  354,  354,  354,  354,
  354,  354,  354,  354,  354,  354,  354,  354,  354,  354,
  354,  354,  354,  354,  354,  354,    0,    0,  371,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  371,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  230,    0,    0,    0,    0,    0,    0,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  230,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  230,  230,  230,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  230,    0,    0,    0,    0,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  230,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  230,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  230,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  230,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  230,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  230,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  230,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  230,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  230,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  230,  230,  230,  230,
  230,  230,  230,  230,  230,  230,  230,  230,  230,  230,
  230,  230,  230,  230,  371,    0,  365,    0,    0,    0,
    0,  371,  371,  371,  371,  371,  371,  371,  371,  371,
  371,  371,  371,  371,  371,  371,  371,  371,  371,  371,
  371,  371,  371,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  371,  365,    0,    0,
    0,  371,  371,  371,  371,  371,  371,  371,  371,  371,
  371,  371,  371,  371,  371,  371,  371,  371,  371,  371,
  371,  371,  371,  371,  371,  371,  371,  371,  371,  371,
  371,  371,  371,  371,  371,  371,  371,  371,  371,  371,
  371,  371,  371,  371,  371,  371,  371,  371,  371,  371,
  371,  371,  371,  371,  371,  371,  371,  371,  371,  371,
  371,  371,  371,  371,  371,  371,  371,  371,  371,  371,
  371,  371,  371,  371,  371,  371,  371,  371,  371,  371,
  371,  371,  371,  371,  371,  371,  371,  371,  371,  371,
  371,  371,  371,  371,  371,  371,  371,  371,  371,  371,
  371,  371,  371,  371,  371,  371,  371,  371,  371,  371,
  371,  371,  371,  371,  371,  371,  371,  371,    0,    0,
  395,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  395,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  365,    0,    0,    0,    0,    0,    0,  365,
  365,  365,  365,  365,  365,  365,  365,  365,  365,  365,
  365,  365,  365,  365,  365,  365,  365,  365,  365,  365,
  365,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  365,    0,    0,    0,    0,  365,
  365,  365,  365,  365,  365,  365,  365,  365,  365,  365,
  365,  365,  365,  365,  365,  365,  365,  365,  365,  365,
  365,  365,  365,  365,  365,  365,  365,  365,  365,  365,
  365,  365,  365,  365,  365,  365,  365,  365,  365,  365,
  365,  365,  365,  365,  365,  365,  365,  365,  365,  365,
  365,  365,  365,  365,  365,  365,  365,  365,  365,  365,
  365,  365,  365,  365,  365,  365,  365,  365,  365,  365,
  365,  365,  365,  365,  365,  365,  365,  365,  365,  365,
  365,  365,  365,  365,  365,  365,  365,  365,  365,  365,
  365,  365,  365,  365,  365,  365,  365,  365,  365,  365,
  365,  365,  365,  365,  365,  365,  365,  365,  365,  365,
  365,  365,  365,  365,  365,  365,  395,    0,  397,    0,
    0,    0,    0,  395,  395,  395,  395,  395,  395,  395,
  395,  395,  395,  395,  395,  395,  395,  395,  395,  395,
  395,  395,  395,  395,  395,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  395,  397,
    0,    0,    0,  395,  395,  395,  395,  395,  395,  395,
  395,  395,  395,  395,  395,  395,  395,  395,  395,  395,
  395,  395,  395,  395,  395,  395,  395,  395,  395,  395,
  395,  395,  395,  395,  395,  395,  395,  395,  395,  395,
  395,  395,  395,  395,  395,  395,  395,  395,  395,  395,
  395,  395,  395,  395,  395,  395,  395,  395,  395,  395,
  395,  395,  395,  395,  395,  395,  395,  395,  395,  395,
  395,  395,  395,  395,  395,  395,  395,  395,  395,  395,
  395,  395,  395,  395,  395,  395,  395,  395,  395,  395,
  395,  395,  395,  395,  395,  395,  395,  395,  395,  395,
  395,  395,  395,  395,  395,  395,  395,  395,  395,  395,
  395,  395,  395,  395,  395,  395,  395,  395,  395,  395,
    0,    0,  245,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  245,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  397,    0,    0,    0,    0,    0,
    0,  397,  397,  397,  397,  397,  397,  397,  397,  397,
  397,  397,  397,  397,  397,  397,  397,  397,  397,  397,
  397,  397,  397,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  397,    0,    0,    0,
    0,  397,  397,  397,  397,  397,  397,  397,  397,  397,
  397,  397,  397,  397,  397,  397,  397,  397,  397,  397,
  397,  397,  397,  397,  397,  397,  397,  397,  397,  397,
  397,  397,  397,  397,  397,  397,  397,  397,  397,  397,
  397,  397,  397,  397,  397,  397,  397,  397,  397,  397,
  397,  397,  397,  397,  397,  397,  397,  397,  397,  397,
  397,  397,  397,  397,  397,  397,  397,  397,  397,  397,
  397,  397,  397,  397,  397,  397,  397,  397,  397,  397,
  397,  397,  397,  397,  397,  397,  397,  397,  397,  397,
  397,  397,  397,  397,  397,  397,  397,  397,  397,  397,
  397,  397,  397,  397,  397,  397,  397,  397,  397,  397,
  397,  397,  397,  397,  397,  397,  397,  397,  245,    0,
  247,    0,    0,    0,    0,  245,  245,  245,  245,  245,
  245,  245,  245,  245,  245,  245,  245,  245,  245,  245,
  245,  245,  245,  245,  245,  245,  245,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  245,  247,    0,    0,    0,  245,  245,  245,  245,  245,
  245,  245,  245,  245,  245,  245,  245,  245,  245,  245,
  245,  245,  245,  245,  245,  245,  245,  245,  245,  245,
  245,  245,  245,  245,  245,  245,  245,  245,  245,  245,
  245,  245,  245,  245,  245,  245,  245,  245,  245,  245,
  245,  245,  245,  245,  245,  245,  245,  245,  245,  245,
  245,  245,  245,  245,  245,  245,  245,  245,  245,  245,
  245,  245,  245,  245,  245,  245,  245,  245,  245,  245,
  245,  245,  245,  245,  245,  245,  245,  245,  245,  245,
  245,  245,  245,  245,  245,  245,  245,  245,  245,  245,
  245,  245,  245,  245,  245,  245,  245,  245,  245,  245,
  245,  245,  245,  245,  245,  245,  245,  245,  245,  245,
  245,  245,    0,    0,  241,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  241,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  247,    0,    0,    0,
    0,    0,    0,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  247,    0,
    0,    0,    0,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  241,    0,  243,    0,    0,    0,    0,  241,  241,  241,
  241,  241,  241,  241,  241,  241,  241,  241,  241,  241,
  241,  241,  241,  241,  241,  241,  241,  241,  241,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  241,  243,    0,    0,    0,  241,  241,  241,
  241,  241,  241,  241,  241,  241,  241,  241,  241,  241,
  241,  241,  241,  241,  241,  241,  241,  241,  241,  241,
  241,  241,  241,  241,  241,  241,  241,  241,  241,  241,
  241,  241,  241,  241,  241,  241,  241,  241,  241,  241,
  241,  241,  241,  241,  241,  241,  241,  241,  241,  241,
  241,  241,  241,  241,  241,  241,  241,  241,  241,  241,
  241,  241,  241,  241,  241,  241,  241,  241,  241,  241,
  241,  241,  241,  241,  241,  241,  241,  241,  241,  241,
  241,  241,  241,  241,  241,  241,  241,  241,  241,  241,
  241,  241,  241,  241,  241,  241,  241,  241,  241,  241,
  241,  241,  241,  241,  241,  241,  241,  241,  241,  241,
  241,  241,  241,  241,    0,    0,  244,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  244,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  243,    0,
    0,    0,    0,    0,    0,  243,  243,  243,  243,  243,
  243,  243,  243,  243,  243,  243,  243,  243,  243,  243,
  243,  243,  243,  243,  243,  243,  243,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  243,    0,    0,    0,    0,  243,  243,  243,  243,  243,
  243,  243,  243,  243,  243,  243,  243,  243,  243,  243,
  243,  243,  243,  243,  243,  243,  243,  243,  243,  243,
  243,  243,  243,  243,  243,  243,  243,  243,  243,  243,
  243,  243,  243,  243,  243,  243,  243,  243,  243,  243,
  243,  243,  243,  243,  243,  243,  243,  243,  243,  243,
  243,  243,  243,  243,  243,  243,  243,  243,  243,  243,
  243,  243,  243,  243,  243,  243,  243,  243,  243,  243,
  243,  243,  243,  243,  243,  243,  243,  243,  243,  243,
  243,  243,  243,  243,  243,  243,  243,  243,  243,  243,
  243,  243,  243,  243,  243,  243,  243,  243,  243,  243,
  243,  243,  243,  243,  243,  243,  243,  243,  243,  243,
  243,  243,  244,    0,  660,    0,    0,    0,    0,  244,
  244,  244,  244,  244,  244,  244,  244,  244,  244,  244,
  244,  244,  244,  244,  244,  244,  244,  244,  244,  244,
  244,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  244,  660,    0,    0,    0,  244,
  244,  244,  244,  244,  244,  244,  244,  244,  244,  244,
  244,  244,  244,  244,  244,  244,  244,  244,  244,  244,
  244,  244,  244,  244,  244,  244,  244,  244,  244,  244,
  244,  244,  244,  244,  244,  244,  244,  244,  244,  244,
  244,  244,  244,  244,  244,  244,  244,  244,  244,  244,
  244,  244,  244,  244,  244,  244,  244,  244,  244,  244,
  244,  244,  244,  244,  244,  244,  244,  244,  244,  244,
  244,  244,  244,  244,  244,  244,  244,  244,  244,  244,
  244,  244,  244,  244,  244,  244,  244,  244,  244,  244,
  244,  244,  244,  244,  244,  244,  244,  244,  244,  244,
  244,  244,  244,  244,  244,  244,  244,  244,  244,  244,
  244,  244,  244,  244,  244,  244,    0,    0,  251,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  251,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  660,    0,    0,    0,    0,    0,    0,  660,  660,  660,
  660,  660,  660,  660,  660,  660,  660,  660,  660,  660,
  660,  660,  660,  660,  660,  660,  660,  660,  660,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  660,    0,    0,    0,    0,  660,  660,  660,
  660,  660,  660,  660,  660,  660,  660,  660,  660,  660,
  660,  660,  660,  660,  660,  660,  660,  660,  660,  660,
  660,  660,  660,  660,  660,  660,  660,  660,  660,  660,
  660,  660,  660,  660,  660,  660,  660,  660,  660,  660,
  660,  660,  660,  660,  660,  660,  660,  660,  660,  660,
  660,  660,  660,  660,  660,  660,  660,  660,  660,  660,
  660,  660,  660,  660,  660,  660,  660,  660,  660,  660,
  660,  660,  660,  660,  660,  660,  660,  660,  660,  660,
  660,  660,  660,  660,  660,  660,  660,  660,  660,  660,
  660,  660,  660,  660,  660,  660,  660,  660,  660,  660,
  660,  660,  660,  660,  660,  660,  660,  660,  660,  660,
  660,  660,  660,  660,  251,    0,  669,    0,    0,    0,
    0,  251,  251,  251,  251,  251,  251,  251,  251,  251,
  251,  251,  251,  251,  251,  251,  251,  251,  251,  251,
  251,  251,  251,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  251,  669,    0,    0,
    0,  251,  251,  251,  251,  251,  251,  251,  251,  251,
  251,  251,  251,  251,  251,  251,  251,  251,  251,  251,
  251,  251,  251,  251,  251,  251,  251,  251,  251,  251,
  251,  251,  251,  251,  251,  251,  251,  251,  251,  251,
  251,  251,  251,  251,  251,  251,  251,  251,  251,  251,
  251,  251,  251,  251,  251,  251,  251,  251,  251,  251,
  251,  251,  251,  251,  251,  251,  251,  251,  251,  251,
  251,  251,  251,  251,  251,  251,  251,  251,  251,  251,
  251,  251,  251,  251,  251,  251,  251,  251,  251,  251,
  251,  251,  251,  251,  251,  251,  251,  251,  251,  251,
  251,  251,  251,  251,  251,  251,  251,  251,  251,  251,
  251,  251,  251,  251,  251,  251,  251,  251,    0,    0,
  666,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  666,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  669,    0,    0,    0,    0,    0,    0,  669,
  669,  669,  669,  669,  669,  669,  669,  669,  669,  669,
  669,  669,  669,  669,  669,  669,  669,  669,  669,  669,
  669,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  669,    0,    0,    0,    0,  669,
  669,  669,  669,  669,  669,  669,  669,  669,  669,  669,
  669,  669,  669,  669,  669,  669,  669,  669,  669,  669,
  669,  669,  669,  669,  669,  669,  669,  669,  669,  669,
  669,  669,  669,  669,  669,  669,  669,  669,  669,  669,
  669,  669,  669,  669,  669,  669,  669,  669,  669,  669,
  669,  669,  669,  669,  669,  669,  669,  669,  669,  669,
  669,  669,  669,  669,  669,  669,  669,  669,  669,  669,
  669,  669,  669,  669,  669,  669,  669,  669,  669,  669,
  669,  669,  669,  669,  669,  669,  669,  669,  669,  669,
  669,  669,  669,  669,  669,  669,  669,  669,  669,  669,
  669,  669,  669,  669,  669,  669,  669,  669,  669,  669,
  669,  669,  669,  669,  669,  669,  666,    0,  260,    0,
    0,    0,    0,  666,  666,  666,  666,  666,  666,  666,
  666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
  666,  666,  666,  666,  666,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  666,  260,
    0,    0,    0,  666,  666,  666,  666,  666,  666,  666,
  666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
  666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
  666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
  666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
  666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
  666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
  666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
  666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
  666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
  666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
  666,  666,  666,  666,  666,  666,  666,  666,  666,  666,
    0,    0,  261,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  261,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  260,    0,    0,    0,    0,    0,
    0,  260,  260,  260,  260,  260,  260,  260,  260,  260,
  260,  260,  260,  260,  260,  260,  260,  260,  260,  260,
  260,  260,  260,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  260,    0,    0,    0,
    0,  260,  260,  260,  260,  260,  260,  260,  260,  260,
  260,  260,  260,  260,  260,  260,  260,  260,  260,  260,
  260,  260,  260,  260,  260,  260,  260,  260,  260,  260,
  260,  260,  260,  260,  260,  260,  260,  260,  260,  260,
  260,  260,  260,  260,  260,  260,  260,  260,  260,  260,
  260,  260,  260,  260,  260,  260,  260,  260,  260,  260,
  260,  260,  260,  260,  260,  260,  260,  260,  260,  260,
  260,  260,  260,  260,  260,  260,  260,  260,  260,  260,
  260,  260,  260,  260,  260,  260,  260,  260,  260,  260,
  260,  260,  260,  260,  260,  260,  260,  260,  260,  260,
  260,  260,  260,  260,  260,  260,  260,  260,  260,  260,
  260,  260,  260,  260,  260,  260,  260,  260,  261,    0,
  254,    0,    0,    0,    0,  261,  261,  261,  261,  261,
  261,  261,  261,  261,  261,  261,  261,  261,  261,  261,
  261,  261,  261,  261,  261,  261,  261,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  261,  254,    0,    0,    0,  261,  261,  261,  261,  261,
  261,  261,  261,  261,  261,  261,  261,  261,  261,  261,
  261,  261,  261,  261,  261,  261,  261,  261,  261,  261,
  261,  261,  261,  261,  261,  261,  261,  261,  261,  261,
  261,  261,  261,  261,  261,  261,  261,  261,  261,  261,
  261,  261,  261,  261,  261,  261,  261,  261,  261,  261,
  261,  261,  261,  261,  261,  261,  261,  261,  261,  261,
  261,  261,  261,  261,  261,  261,  261,  261,  261,  261,
  261,  261,  261,  261,  261,  261,  261,  261,  261,  261,
  261,  261,  261,  261,  261,  261,  261,  261,  261,  261,
  261,  261,  261,  261,  261,  261,  261,  261,  261,  261,
  261,  261,  261,  261,  261,  261,  261,  261,  261,  261,
  261,  261,    0,    0,  257,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  257,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  254,    0,    0,    0,
    0,    0,    0,  254,  254,  254,  254,  254,  254,  254,
  254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
  254,  254,  254,  254,  254,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  254,    0,
    0,    0,    0,  254,  254,  254,  254,  254,  254,  254,
  254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
  254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
  254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
  254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
  254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
  254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
  254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
  254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
  254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
  254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
  254,  254,  254,  254,  254,  254,  254,  254,  254,  254,
  257,    0,  262,    0,    0,    0,    0,  257,  257,  257,
  257,  257,  257,  257,  257,  257,  257,  257,  257,  257,
  257,  257,  257,  257,  257,  257,  257,  257,  257,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  257,  262,    0,    0,    0,  257,  257,  257,
  257,  257,  257,  257,  257,  257,  257,  257,  257,  257,
  257,  257,  257,  257,  257,  257,  257,  257,  257,  257,
  257,  257,  257,  257,  257,  257,  257,  257,  257,  257,
  257,  257,  257,  257,  257,  257,  257,  257,  257,  257,
  257,  257,  257,  257,  257,  257,  257,  257,  257,  257,
  257,  257,  257,  257,  257,  257,  257,  257,  257,  257,
  257,  257,  257,  257,  257,  257,  257,  257,  257,  257,
  257,  257,  257,  257,  257,  257,  257,  257,  257,  257,
  257,  257,  257,  257,  257,  257,  257,  257,  257,  257,
  257,  257,  257,  257,  257,  257,  257,  257,  257,  257,
  257,  257,  257,  257,  257,  257,  257,  257,  257,  257,
  257,  257,  257,  257,    0,    0,  258,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  258,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  262,    0,
    0,    0,    0,    0,    0,  262,  262,  262,  262,  262,
  262,  262,  262,  262,  262,  262,  262,  262,  262,  262,
  262,  262,  262,  262,  262,  262,  262,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  262,    0,    0,    0,    0,  262,  262,  262,  262,  262,
  262,  262,  262,  262,  262,  262,  262,  262,  262,  262,
  262,  262,  262,  262,  262,  262,  262,  262,  262,  262,
  262,  262,  262,  262,  262,  262,  262,  262,  262,  262,
  262,  262,  262,  262,  262,  262,  262,  262,  262,  262,
  262,  262,  262,  262,  262,  262,  262,  262,  262,  262,
  262,  262,  262,  262,  262,  262,  262,  262,  262,  262,
  262,  262,  262,  262,  262,  262,  262,  262,  262,  262,
  262,  262,  262,  262,  262,  262,  262,  262,  262,  262,
  262,  262,  262,  262,  262,  262,  262,  262,  262,  262,
  262,  262,  262,  262,  262,  262,  262,  262,  262,  262,
  262,  262,  262,  262,  262,  262,  262,  262,  262,  262,
  262,  262,  258,    0,  401,    0,    0,    0,    0,  258,
  258,  258,  258,  258,  258,  258,  258,  258,  258,  258,
  258,  258,  258,  258,  258,  258,  258,  258,  258,  258,
  258,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  258,  401,    0,    0,    0,  258,
  258,  258,  258,  258,  258,  258,  258,  258,  258,  258,
  258,  258,  258,  258,  258,  258,  258,  258,  258,  258,
  258,  258,  258,  258,  258,  258,  258,  258,  258,  258,
  258,  258,  258,  258,  258,  258,  258,  258,  258,  258,
  258,  258,  258,  258,  258,  258,  258,  258,  258,  258,
  258,  258,  258,  258,  258,  258,  258,  258,  258,  258,
  258,  258,  258,  258,  258,  258,  258,  258,  258,  258,
  258,  258,  258,  258,  258,  258,  258,  258,  258,  258,
  258,  258,  258,  258,  258,  258,  258,  258,  258,  258,
  258,  258,  258,  258,  258,  258,  258,  258,  258,  258,
  258,  258,  258,  258,  258,  258,  258,  258,  258,  258,
  258,  258,  258,  258,  258,  258,    0,    0,  407,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  407,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  401,    0,    0,    0,    0,    0,    0,  401,  401,  401,
  401,  401,  401,  401,  401,  401,  401,  401,  401,  401,
  401,  401,  401,  401,  401,  401,  401,  401,  401,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  401,    0,    0,    0,    0,  401,  401,  401,
  401,  401,  401,  401,  401,  401,  401,  401,  401,  401,
  401,  401,  401,  401,  401,  401,  401,  401,  401,  401,
  401,  401,  401,  401,  401,  401,  401,  401,  401,  401,
  401,  401,  401,  401,  401,  401,  401,  401,  401,  401,
  401,  401,  401,  401,  401,  401,  401,  401,  401,  401,
  401,  401,  401,  401,  401,  401,  401,  401,  401,  401,
  401,  401,  401,  401,  401,  401,  401,  401,  401,  401,
  401,  401,  401,  401,  401,  401,  401,  401,  401,  401,
  401,  401,  401,  401,  401,  401,  401,  401,  401,  401,
  401,  401,  401,  401,  401,  401,  401,  401,  401,  401,
  401,  401,  401,  401,  401,  401,  401,  401,  401,  401,
  401,  401,  401,  401,  407,  102,    0,    0,    0,    0,
    0,  407,  407,  407,  407,  407,  407,  407,  407,  407,
  407,  407,  407,  407,  407,  407,  407,  407,  407,  407,
  407,  407,  407,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  407,    0,    0,    0,
    0,  407,  407,  407,  407,  407,  407,  407,  407,  407,
  407,  407,  407,  407,  407,  407,  407,  407,  407,  407,
  407,  407,  407,  407,  407,  407,  407,  407,  407,  407,
  407,  407,  407,  407,  407,  407,  407,  407,  407,  407,
  407,  407,  407,  407,  407,  407,  407,  407,  407,  407,
  407,  407,  407,  407,  407,  407,  407,  407,  407,  407,
  407,  407,  407,  407,  407,  407,  407,  407,  407,  407,
  407,  407,  407,  407,  407,  407,  407,  407,  407,  407,
  407,  407,  407,  407,  407,  407,  407,  407,  407,  407,
  407,  407,  407,  407,  407,  407,  407,  407,  407,  407,
  407,  407,  407,  407,  407,  407,  407,  407,  407,  407,
  407,  407,  407,  407,  407,  407,  407,  407,    0,  120,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  102,    0,    0,    0,    0,    0,    0,  102,  102,
  102,  102,  102,  102,  102,  102,  102,  102,  102,  102,
  102,  102,  102,  102,  102,  102,  102,  102,  102,  102,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  102,    0,    0,    0,    0,  102,  102,
  102,  102,  102,  102,  102,  102,  102,  102,  102,  102,
  102,  102,  102,  102,  102,  102,  102,  102,  102,  102,
  102,  102,  102,  102,  102,  102,  102,  102,  102,  102,
  102,  102,  102,  102,  102,  102,  102,  102,  102,  102,
  102,  102,  102,  102,  102,  102,  102,  102,  102,  102,
  102,  102,  102,  102,  102,  102,  102,  102,  102,  102,
  102,  102,  102,  102,  102,  102,  102,  102,  102,  102,
  102,  102,  102,  102,  102,  102,  102,  102,  102,  102,
  102,  102,  102,  102,  102,  102,  102,  102,  102,  102,
  102,  102,  102,  102,  102,  102,  102,  102,  102,  102,
  102,  102,  102,  102,  102,  102,  102,  102,  102,  102,
  102,  102,  102,  102,  102,  120,   33,    0,    0,    0,
    0,    0,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  120,    0,    0,
    0,    0,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,    0,
   34,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   33,    0,    0,    0,    0,    0,    0,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   33,    0,    0,    0,    0,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   34,   35,    0,    0,
    0,    0,    0,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   34,    0,
    0,    0,    0,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
    0,   36,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   35,    0,    0,    0,    0,    0,    0,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   35,    0,    0,    0,    0,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   36,   37,    0,
    0,    0,    0,    0,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   36,
    0,    0,    0,    0,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,    0,   42,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   37,    0,    0,    0,    0,    0,
    0,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   37,    0,    0,    0,
    0,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   42,  103,
    0,    0,    0,    0,    0,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   42,    0,    0,    0,    0,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,    0,   46,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  103,    0,    0,    0,    0,
    0,    0,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  103,    0,    0,
    0,    0,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,   46,
   47,    0,    0,    0,    0,    0,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   46,    0,    0,    0,    0,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,    0,   48,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   47,    0,    0,    0,
    0,    0,    0,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   47,    0,
    0,    0,    0,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   48,  121,    0,    0,    0,    0,    0,   48,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   48,    0,    0,    0,    0,   48,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,   48,
   48,   48,   48,   48,   48,   48,   48,   48,   48,   48,
   48,   48,   48,   48,    0,   51,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  121,    0,    0,
    0,    0,    0,    0,  121,  121,  121,  121,  121,  121,
  121,  121,  121,  121,  121,  121,  121,  121,  121,  121,
  121,  121,  121,  121,  121,  121,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  121,
    0,    0,    0,    0,  121,  121,  121,  121,  121,  121,
  121,  121,  121,  121,  121,  121,  121,  121,  121,  121,
  121,  121,  121,  121,  121,  121,  121,  121,  121,  121,
  121,  121,  121,  121,  121,  121,  121,  121,  121,  121,
  121,  121,  121,  121,  121,  121,  121,  121,  121,  121,
  121,  121,  121,  121,  121,  121,  121,  121,  121,  121,
  121,  121,  121,  121,  121,  121,  121,  121,  121,  121,
  121,  121,  121,  121,  121,  121,  121,  121,  121,  121,
  121,  121,  121,  121,  121,  121,  121,  121,  121,  121,
  121,  121,  121,  121,  121,  121,  121,  121,  121,  121,
  121,  121,  121,  121,  121,  121,  121,  121,  121,  121,
  121,  121,  121,  121,  121,  121,  121,  121,  121,  121,
  121,   51,   38,    0,    0,    0,    0,    0,   51,   51,
   51,   51,   51,   51,   51,   51,   51,   51,   51,   51,
   51,   51,   51,   51,   51,   51,   51,   51,   51,   51,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   51,    0,    0,    0,    0,   51,   51,
   51,   51,   51,   51,   51,   51,   51,   51,   51,   51,
   51,   51,   51,   51,   51,   51,   51,   51,   51,   51,
   51,   51,   51,   51,   51,   51,   51,   51,   51,   51,
   51,   51,   51,   51,   51,   51,   51,   51,   51,   51,
   51,   51,   51,   51,   51,   51,   51,   51,   51,   51,
   51,   51,   51,   51,   51,   51,   51,   51,   51,   51,
   51,   51,   51,   51,   51,   51,   51,   51,   51,   51,
   51,   51,   51,   51,   51,   51,   51,   51,   51,   51,
   51,   51,   51,   51,   51,   51,   51,   51,   51,   51,
   51,   51,   51,   51,   51,   51,   51,   51,   51,   51,
   51,   51,   51,   51,   51,   51,   51,   51,   51,   51,
   51,   51,   51,   51,   51,    0,   39,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   38,    0,
    0,    0,    0,    0,    0,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   38,    0,    0,    0,    0,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   39,   40,    0,    0,    0,    0,    0,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   39,    0,    0,    0,    0,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,    0,   41,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   40,
    0,    0,    0,    0,    0,    0,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   40,    0,    0,    0,    0,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   41,   44,    0,    0,    0,    0,    0,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   41,    0,    0,    0,    0,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   41,   41,   41,   41,
   41,   41,   41,   41,   41,   41,   41,    0,   45,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   44,    0,    0,    0,    0,    0,    0,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   44,    0,    0,    0,    0,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   45,    0,    0,    0,    0,    0,
    0,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   45,    0,    0,    0,
    0,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45, 1142,    0,
 1144,    0,    0,    0,    0,   88,    0,    0,    0,    0,
   87,    0,    0, 1147,    0, 1149,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, 1152,    0,
    0,    0,    0,    0,    0, 1156, 1157,    0,    0,    0,
    0,    0,    0,    0,    0,    0, 1164, 1165,    0,    0,
    0,    0,    0, 1169, 1170,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0, 1174,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
 1181,   89,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0, 1247,    0, 1249, 1250,    0,
 1252,    0, 1254, 1255,    0,    0,    0, 1259,    0,    0,
 1267, 1268, 1269, 1271,    0,    0, 1274,    0,    0, 1279,
 1280, 1281, 1283,    0,    0,    0,    0, 1288,    0, 1295,
 1296, 1298,    0,    0, 1301,    0, 1305, 1306, 1308, 1309,
 1310,    0,   84,   85,   86,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  290,  291,  292,  293,  294,  295,  296,  297,  298,  299,
  300,  301,  302,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, 1355,
 1356, 1357, 1358,    0,    0,    0, 1366,    0,    0,    0,
    0, 1372, 1374,    0,    0,    0, 1379,    0,    0,    0,
    0, 1385, 1387,    0,    0,    0, 1395,    0,    0,    0,
 1400, 1402,    0,    0,    0, 1407,    0,    0,    0, 1412,
 1414,   95,   96,   97,   98,   99,  100,  101,  102,  103,
  104,  105,  106,  107,  108,  109,  110,  111,  112,  113,
  114,  115,  116,  117,  118,  119,  120,  121,  122,  123,
  124,  125,  126,  127,  128,  129,  130,  131,  132,  133,
  134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
  144,  145,  146,  147,  148,  149,  150,  151,  152,  153,
  154,  155,  156,  157,  158,  159,  160,  161,  162,  163,
  164,  165,  166,  167,  168,  169,  170,  171,  172,  173,
  174,  175,  176,  177,  178,  179,  180,  181,  182,  183,
  184,  185,  186,  187,  188,  189,  190,  191,  192,  193,
  194,  195,  196,  197,  198,  199,  200,  201,  202,  203,
  204,  205,  206,  207,  208,  209,  210,  335,  335,  335,
  335,  335,  335,  335,  335,  335,  335,  335,  335,  335,
  335,  335,  335,  335,  335,  335,  335,  335,  335,  335,
  335,  335,  335,  335,  335,  335,  335,  335,  335,  335,
  335,  335,  335,  335,  335,  335,  335,  335,  335,  335,
  335,  335,  335,  335,  335,  335,  335,  335,  335,  335,
  335,  335,  335,  335,  335,  335,  335,  335,  335,  335,
  335,  335,  335,  335,  335,  335,  335,  335,  335,  335,
  335,  335,  335,  335,  335,  335,  335,  335,  335,  335,
  335,  335,  335,  335,  335,  335,  335,  335,  335,  335,
  335,  335,  335,  335,  335,  335,  335,  335,  335,  335,
  335,  335,  335,  335,  335,  335,  335,  335,  335,  335,
  335,  335,  335,
};
short yycheck[] = {                                      95,
   65,  216,   98,  191,  100,   11,   12,   44,   44,   44,
   44,   42,  256,  131,  984,  111,  256,   42,  233,  989,
  990,   44,  638,  119,  120,   44,   44,   44,  124,  327,
  126,  127,  128,   64,  130,  131,  298,  128,  134,   64,
   40,   44,  256,  161,  338,   45,   42,  297,  256,  313,
  306,  169,  175,  149,  150,  339,  152,  313,  154,  357,
  297,  677,   40,  154,  160,  161,   40,   45,   64,  165,
  368,   45,  168,  169,  338,   37,   38,  297,  174,  175,
   42,   43,  376,   45,  382,   47,  257,   40,  184,   40,
   37,   38,   45,  377,   45,   42,   43,  257,   45,   42,
   47,  224,  308,  226,   14,  223,  130,  225,  314,   98,
  257,   64,  376,  209,  210,  211,  212,  213,   42,   29,
   25,  217,  218,  219,  220,  221,  222,  223,  224,  225,
  226,  222,  228,  229,  230,   44,  160,  126,   95,   40,
   64,  991,   44,  100,  168,   37,  103,   94,  126,  106,
   42,   43,  126,   45,  111,   47,   61,   40,  802,   42,
  804,   37,   45,  120,   44,  314,   42,   43,  356,   45,
  127,   47,   44,  126,  131,  126,   95,  134,  150,   98,
   44,  100,   87,   88,   89,  313,  430,  431,   93,  161,
  339,  328,   40,  150,  299,  300,   40,   45,   42,  327,
  119,   45,  967,  175,  161,  124,   42,  126,  127,  128,
   40,  130,  169,  149,  428,  429,   64,   42,  175,   40,
   64,  358,   47,   40,  160,  256,   40,  184,  377,  357,
  149,  256,  369,  152,  217,  154,   40,   40,  174,  222,
  368,  306, 1212,  126,  309,  310,  383,  204,  205,  313,
   40, 1221,  460,  461,  382,  212,   40,  214,  258,  216,
  260,  218,  219,  220,  221,  184,  223,  224,  225,  226,
  227,   40,  229,  230,  338,   40,  233,  257,  126,  257,
  258,  259,   40,  257,  258,  259,  985,   45,  687,  688,
  689,  210,  211,  212,  213,   44,   44,   44,  217,  261,
  262,   44,   44,  222,  257,  258,  259,  258,  259,  228,
  215,  216,  376,  256,  261,  262,   44,  314,  313,  325,
  326,  327,   42,   44,   16,   17,   18,   19,  233, 1179,
 1180,   44,  256, 1183, 1184,   37,  952,  953,  954,   44,
   42,   44,  339,  338,   44,   47, 1316,   37, 1318, 1319,
 1320,   44,   42, 1323, 1324,   44, 1326,   47, 1328, 1329,
 1330,   23,   24, 1333, 1334,   27,   44,   44,  126,  261,
  262,   44,   44,  256,  257,  258,  259,   69,   70,   71,
  377,  376,  257,   44,  289,  261,  262,   44, 1153,   44,
 1155,   44,   44,   44, 1159, 1160,   44,  428,  429,  430,
  431,  432,  433,  434,  435,  436,  437,  438,  439,  257,
  258,  259,  256,   44,  258,   44,  260,  314,   44,   44,
 1185, 1186,   44,  328,  329,  462,  462,  462,  462,  334,
  335,  336,  337,  338,  339,  340,  341,  342,  343,  462,
  714,   44,  339,  462,  462,  462,   44, 1417, 1418, 1419,
 1420, 1421, 1422, 1423, 1424, 1425, 1426, 1427, 1428,  462,
   44,  735,   41,   42,   43,  257,   45,   43,   47,   45,
  744,  449,  450, 1172, 1173,  449,  450, 1176, 1177,   44,
  377,  755,   44,   44,  389,  428,  429,  430,  431,  432,
  433,  434,  435,   44,   44,   44,  684,   44,   44,  257,
  258,  259,   44,   44,  428,  429,  430,  431,  432,  433,
  434,  435,  436,  437,  438,  439,  440,  441,  442,  443,
  444,  445,  446,  447,  448,  449,  450,  451,  452,  453,
  454,  455,  456,  457,  458,  459,  256,   44,   44,   44,
   44,   44,  757,  758,   44,  428,  429,  430,  431,  432,
  433,  434,  435,  436,  437,  438,  439,  440,  441,  442,
  443,  444,  445,  446,  447,  448,  449,  450,  451,  452,
  453,  454,  455,  456,  457,  458,  459,   44,  973,   44,
  645,  646,  647,  648,  428,  429,  430,  431,  432,  433,
  434,  435,  436,  437,  438,  439,   44,  440,  441,  442,
  443,  444,  445,  446,  447,   37,   38,  703,  704,  705,
   42,   43,  257,   45,  710,   47,   44,  257,  714,  715,
  716,  717,  718,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  729,  730,  731,  732,  733,   44,  735,
  736,   44,   42,   43,  257,   45,  742,   47,  744,  745,
  746,  747,  748,  749,  750,  751,  752,  753,   44,  755,
  756,   44,   94,   44,   44,   44,   44,   44,  764,   44,
  766,  767,  768,  769,  579,   44,  772,  773,  774,  775,
  256,   44,   44,  779,  780,  781,  782,  783,  784,   44,
   44,   44,  966,   42,   37,   41,   44,  971,  972,   42,
   43,  257,   45,  977,   47,   44,   44,   44,  428,  429,
  430,  431,  432,  433,  434,  435,  436,  437,  438,  439,
   37,   38,   44,  299,  300,   42,   43,   44,   45,   44,
   47,   44,   44,  690,  639,  640,  641,  256,  695,  696,
  697,  698,  699,   44,  701,   44,   44,   44,   44,  706,
  707,  708,  709,   44,   44,   44,   44,  714,  715,  716,
   44,   41,  719,  720,  721,  722, 1161,   44, 1163,   41,
   44,   44, 1167, 1168,  731,  732,   44,   94,  735,  736,
   44,   44,   42,   44,  703,  704,  705,  744,  745,  746,
   44,   44, 1187, 1188,  751,  752,   44,   44,  755,  756,
  757,  758,  759,  760,  761,  762,  763,  124,   44,  766,
  767,  768,  769,  770,   44,  772,  773,  774,  775,   41,
   41,   41,  779,  780,  781,  782,  783,  784,   98,  261,
  262,   37,   38,   41,   44,  105,   42,   43,   44,   45,
   40,   47,   40,  462,  462,  764,  462,  462,  314,  119,
   44,  256,  757,  758,  124,  130,  126,  127,  462,   44,
  130,   44,  328,   44,  440,  441,  442,  443,  444,  445,
  446,  447,   37,   38,  149,  462,  833,   42,   43,  149,
   45,   44,   47,   44,  154,  160,  313,   44,   94,   44,
  160,  314,  358,  168,  462,  165,  462,   44,  168,  174,
  327,   44,   44,  369,  174,  328,   44,  256,   44,  428,
  429,  430,  431,  432,  433,  434,  435,  383,  124,   44,
   44, 1017,   44, 1019,   44, 1021,   44,   44,   37,   94,
  357,   37,  314,  203,   41,  358,  206, 1033, 1034,  313,
 1036,  368,  314,   37,  261,  262,  369,  217,   42,   43,
   41,   45,  222,   47,   41,  382,   41, 1231,  228,  124,
  383,   41, 1058, 1059, 1238,  124,   41,   41,  462, 1065,
  462,  130, 1068,  256,  462,  462,   37,   38,  462,  462,
 1076,   42,   43, 1079,   45,  462,   47,  462,  462,  462,
  149,  462,  462,  152,  462, 1091,  256,    0,  462,  462,
  462,  462, 1098,  462,  462,  462,  462,  462,  257, 1105,
   44,   44,   44,   44,   44,   44, 1112,  974,   44,  976,
  977,  978,  979,  428,  429,  430,  431,  432,  433,  434,
  435,   44,   44,  990,   44,  992,  993,   44,   41,   44,
  997,   44,  999,   44, 1001, 1002, 1003,   44, 1005, 1006,
   44,  956,  211,  958,   44,  261,  262,   44,  217,   44,
   44, 1335,   44,  222, 1338, 1339, 1340,   44, 1342, 1343,
 1344,   44,   44, 1347, 1348, 1349,   44, 1351, 1352,  428,
  429,  430,  431,  432,  433,  434,  435,  436,  437,  438,
  439,   44,   44,   44,   44,   44,  261,  262, 1194,   44,
 1196,  314, 1198,  314,   44, 1201,   44, 1203,   44,   44,
 1206, 1207, 1208, 1209, 1210, 1211,   44,  289, 1214,   44,
 1216, 1217, 1218, 1219, 1220,   44,   44, 1223,   44, 1225,
 1226, 1227, 1228, 1229, 1230,   44, 1232,   44, 1234, 1235,
 1236, 1237,   44, 1239,   44,  428,  429,  430,  431,  432,
  433,  434,  435,   44,   44, 1429, 1430, 1431, 1432, 1433,
 1434, 1435, 1436, 1437, 1438, 1439, 1440,   44,  428,  429,
  430,  431,  432,  433,  434,  435,  436,  437,  438,  439,
  440,  441,  442,  443,  444,  445,  446,  447,  448,  449,
  450,  451,  452,  453,  454,  455,  456,  457,  458,  459,
  261,  262,   37,   38,  131,   44,   44,   42,   43,   44,
   45,   44,   47,   44,   44,   44,   44,   44,   44, 1315,
   44, 1317,   44,  150,   44, 1321, 1322,   44,   44, 1325,
   44, 1327,   44,   44,  161, 1331, 1332,   44, 1195,   44,
 1336, 1337,  169,   44,   44, 1341,    0,   44,  175, 1345,
 1346,   44,   44,  256, 1350,   44,  256, 1353, 1354,   94,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   44,   37,   44,   44,   44,   41,   44,  124,
   44,   44,  219,  220,   44,  298,  223,  224,  225,  226,
  303,  304,  305,  306,  307,  308,  309,  310,  311,  312,
  313,  314,  315,  316,  317,  318,  319,  320,  321,  322,
  323,  324,  325,  326,  327,  328,  329,  330,  331,  332,
  333,  334,  335,  336,  337,  338,  339,  340,  341,  342,
  343,  344,  345,  346,  347,  348,  349,  350,  351,  352,
  353,  354,  355,  356,  357,  358,  359,  360,  361,  362,
  363,  364,  365,  366,  367,  368,  369,  370,  371,  372,
  373,  374,  375,  376,  377,  378,  379,  380,  381,  382,
  383,  384,  385,  386,  387,  388,  389,  390,  391,  392,
  393,  394,  395,  396,  397,  398,  399,  400,  401,  402,
  403,  404,  405,  406,  407,  408,  409,  410,  411,  412,
  413,  414,  415,  416,  417,  418,  419,   44,   44,   44,
   44,  691,  692,  693,  694,   44,  261,  262,  428,  429,
  430,  431,  432,  433,  434,  435,  436,  437,  438,  439,
  710,  711,  712,  713,   44,   44,   44,  717,  718,   44,
   44,   44,   44,  723,  724,  725,  726,  727,  728,  729,
  730,   44,   44,  733,  734,   44,   44,  737,  738,  739,
  740,  741,  742,  743,   44,   44,   44,  747,  748,  749,
  750,   44,   44,  753,  754,  256,   44,  702,  221,  221,
  174,  169,  168,   -1,    0,  955,   -1,   -1,   -1,   -1,
   -1,  771,  256,   -1,   -1,   -1,  776,  777,  778,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   37,   -1,   -1,   -1,   41,   -1,   -1,   44,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,    0,   -1,   -1,  440,
  441,  442,  443,  444,  445,  446,  447,  448,  449,  450,
  451,  452,  285,  286,  287,  288,  289,  290,  291,  292,
  293,  294,  295,  296,   -1,   -1,  966,   -1,  968,   40,
  970,  971,   -1,   -1,   45,   -1,   -1,   41,  462,   -1,
   44,   -1,   -1,  983,  984,   -1,  986,  987,   -1,   -1,
   -1,   -1,   -1,   64,  994,   -1,  996,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,  126,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,    0,  256,  257,  258,  259,   -1,
   -1,   -1,  256, 1193,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   41,  462,   -1,   44,  256,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,    0,  428,  429,  430,
  431,  432,  433,  434,  435,  436,  437,  438,  439,  440,
  441,  442,  443,  444,  445,  446,  447,  448,  449,  450,
  451,  452,  453,  454,  455,  456,  457,  458,  459,   40,
   -1,   42,   -1,   -1,   45,   -1,   -1,   41,  462,   -1,
   44,  428,  429,  430,  431,  432,  433,  434,  435,  436,
  437,  438,  439,   64,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,  126,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,    0,  256,  257,  258,  259,   -1,
   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   41,  462,   -1,   44,  256,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,    0,  428,  429,  430,
  431,  432,  433,  434,  435,  436,  437,  438,  439,  440,
  441,  442,  443,  444,  445,  446,  447,  448,  449,  450,
  451,  452,  453,  454,  455,  456,  457,  458,  459,   40,
   -1,   42,   -1,   -1,   45,   -1,   -1,   41,  462,   -1,
   44,  428,  429,  430,  431,  432,  433,  434,  435,  436,
  437,  438,  439,   64,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,  126,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,    0,  256,  257,  258,  259,   -1,
   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   41,  462,   -1,   44,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,    0,  428,  429,  430,
  431,  432,  433,  434,  435,  436,  437,  438,  439,  440,
  441,  442,  443,  444,  445,  446,  447,  448,  449,  450,
  451,  452,  453,  454,  455,  456,  457,  458,  459,   40,
   -1,   42,   -1,   -1,   45,   -1,   -1,   41,  462,   -1,
   44,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   64,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   42,   -1,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,  126,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,    0,  256,  257,  258,  259,   -1,
   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   41,  462,   -1,   44,  256,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,    0,  428,  429,  430,
  431,  432,  433,  434,  435,  436,  437,  438,  439,  440,
  441,  442,  443,  444,  445,  446,  447,  448,  449,  450,
  451,  452,  453,  454,  455,  456,  457,  458,  459,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,  462,   -1,
   44,  428,  429,  430,  431,  432,  433,  434,  435,  436,
  437,  438,  439,  440,  441,  442,  443,  444,  445,  446,
  447,  448,  449,  450,  451,  452,  453,  454,  455,  456,
  457,  458,  459,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,    0,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   41,  462,   -1,   44,   -1,
   -1,   -1,   -1,  256,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,    0,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   37,   38,   -1,   -1,   41,   42,   43,   -1,
   45,   -1,   47,   -1,   -1,   -1,   -1,   41,  462,   40,
   -1,   -1,   -1,   -1,   45,  428,  429,  430,  431,  432,
  433,  434,  435,  436,  437,  438,  439,  440,  441,  442,
  443,  444,  445,  446,  447,  448,  449,  450,  451,  452,
  453,  454,  455,  456,  457,  458,  459,   -1,   -1,   94,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,  124,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,  126,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,    0,   -1,  261,  262,   -1,   -1,
   -1,   -1,  256,   -1,   -1,   -1,  257,  258,  259,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   41,  462,  256,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,    0,   -1,   -1,  420,
  421,  422,  423,  424,  425,  426,  427,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   37,   38,   -1,   -1,   41,   42,   43,   -1,
   45,   -1,   47,   -1,   -1,   -1,   -1,   41,  462,  428,
  429,  430,  431,  432,  433,  434,  435,  436,  437,  438,
  439,  440,  441,  442,  443,  444,  445,  446,  447,  448,
  449,  450,  451,  452,  453,  454,  455,  456,  457,  458,
  459,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   94,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,  124,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,    0,   -1,  261,  262,   -1,   -1,
   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   41,  462,  256,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,    0,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   37,   38,   -1,   -1,   -1,   42,   43,   44,
   45,   -1,   47,   -1,   -1,   -1,   -1,   41,  462,  428,
  429,  430,  431,  432,  433,  434,  435,  436,  437,  438,
  439,  440,  441,  442,  443,  444,  445,  446,  447,  448,
  449,  450,  451,  452,  453,  454,  455,  456,  457,  458,
  459,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   94,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,  124,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,    0,   -1,  261,  262,   -1,   -1,
   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   41,  462,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   37,   38,   -1,   -1,   -1,   42,   43,   44,
   45,   -1,   47,   -1,   -1,   -1,   -1,   -1,  462,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   94,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,  124,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,    0,   -1,  261,  262,   -1,   -1,
   -1,   -1,   37,   38,   -1,   -1,   -1,   42,   43,   44,
   45,   -1,   47,   37,   38,  120,   -1,   41,   42,   43,
   -1,   45,   -1,   47,   -1,   -1,  131,   -1,   -1,  134,
   -1,   -1,   38,   -1,   -1,   41,  462,   43,   44,   45,
   -1,   -1,   -1,   -1,   -1,  150,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   37,   38,   -1,   -1,   94,
   42,   43,   -1,   45,   -1,   47,   -1,   -1,   -1,   -1,
   94,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   95,   94,  124,
   -1,   99,  100,   -1,  102,   -1,  104,   -1,   -1,  107,
  124,   -1,   -1,  111,  306,   -1,   -1,   -1,   -1,   -1,
   -1,  313,   94,  218,   -1,  220,  221,   -1,  124,  127,
  225,  226,  324,   -1,  229,  327,  134,   -1,   -1,   -1,
   -1,   -1,   -1,  335,  336,   -1,  338,   -1,   -1,   -1,
   -1,   -1,  124,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  161,   -1,  357,   -1,   -1,    0,   -1,
  362,  169,   -1,   -1,   -1,   -1,  368,  175,   -1,   -1,
   -1,   -1,   -1,   -1,  376,   -1,  184,   -1,   -1,   -1,
  382,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   38,   -1,   -1,   41,
   -1,   43,   44,   45,   -1,   -1,   -1,   -1,   -1,   -1,
  218,  219,   -1,  221,   -1,  223,  224,   -1,   -1,   -1,
   -1,  229,  230,   -1,  232,   -1,  261,  262,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  261,  262,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  256,   -1,   94,   -1,   -1,  261,  262,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,  261,
  262,   -1,  124,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,  256,   -1,  424,   -1,   -1,  261,
  262,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,    0,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,  308,   -1,   -1,
   -1,   -1,   -1,  314,   -1,   -1,   38,   -1,   -1,   41,
   -1,   43,   44,   45,  325,   -1,   -1,  328,  329,   -1,
  331,   -1,   -1,  334,   -1,   -1,  337,   -1,  339,   -1,
   -1,   -1,   -1,  344,   -1,  346,  347,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  358,   -1,   -1,
   -1,   -1,   -1,  364,   -1,   -1,   -1,   -1,  369,  370,
   -1,  372,   94,   -1,  375,   -1,  377,   -1,   -1,  380,
   -1,   -1,  383,   -1,  385,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  124,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  419,   -1,   -1,   -1,   -1,
   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   38,   -1,   -1,   41,   -1,   43,   44,   45,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  256,   -1,   94,   -1,   -1,  261,
  262,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,  124,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,  256,   -1,
   -1,   -1,   -1,  261,  262,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   38,   -1,   -1,   41,   -1,   -1,   44,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   94,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  124,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,    0,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   38,   -1,   -1,   41,   -1,   -1,
   44,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,
   94,   -1,   -1,  261,  262,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
  124,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,  256,   -1,   -1,   -1,   -1,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,    0,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   38,   -1,   -1,   41,   -1,   -1,
   44,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   94,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  124,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   38,   -1,
   -1,   41,   -1,   -1,   44,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  256,   -1,   94,   -1,   -1,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,  124,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,  256,   -1,   -1,   -1,
   -1,  261,  262,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,
   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,  338,  339,
  340,  341,  342,  343,  344,  345,  346,  347,  348,  349,
  350,  351,  352,  353,  354,  355,  356,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,  389,
  390,  391,  392,  393,  394,  395,  396,  397,  398,  399,
  400,  401,  402,  403,  404,  405,  406,  407,  408,  409,
  410,  411,  412,  413,  414,  415,  416,  417,  418,  419,
    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   38,   -1,
   -1,   41,   -1,   -1,   44,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   94,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  124,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   38,   -1,   -1,   41,   -1,   -1,   44,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,   94,   -1,
   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,  124,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,
   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,  338,  339,
  340,  341,  342,  343,  344,  345,  346,  347,  348,  349,
  350,  351,  352,  353,  354,  355,  356,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,  389,
  390,  391,  392,  393,  394,  395,  396,  397,  398,  399,
  400,  401,  402,  403,  404,  405,  406,  407,  408,  409,
  410,  411,  412,  413,  414,  415,  416,  417,  418,  419,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,    0,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   41,   -1,   -1,   44,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   94,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  124,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,    0,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,
   -1,   -1,   44,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  256,   -1,   94,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,  124,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,  256,   -1,   -1,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,    0,   -1,
   -1,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,
  298,   43,   44,   45,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   41,   -1,   43,   44,   45,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,  256,   -1,
   -1,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   41,   -1,   -1,   44,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  124,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,    0,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,   -1,   -1,
   44,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,
   -1,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
  124,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,  256,   -1,    0,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   41,   -1,   -1,   44,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,   -1,   -1,    0,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,
   -1,   -1,   44,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,  256,   -1,    0,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   41,   -1,   -1,
   44,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,   -1,   -1,
    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   41,   -1,   -1,   44,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,  256,   -1,    0,   -1,
   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   41,
   -1,   -1,   44,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,  338,  339,
  340,  341,  342,  343,  344,  345,  346,  347,  348,  349,
  350,  351,  352,  353,  354,  355,  356,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,  389,
  390,  391,  392,  393,  394,  395,  396,  397,  398,  399,
  400,  401,  402,  403,  404,  405,  406,  407,  408,  409,
  410,  411,  412,  413,  414,  415,  416,  417,  418,  419,
   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   41,   -1,   -1,   44,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,  256,   -1,
    0,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   41,   -1,   -1,   44,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   41,   -1,   -1,   44,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,
   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,
   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,  338,  339,
  340,  341,  342,  343,  344,  345,  346,  347,  348,  349,
  350,  351,  352,  353,  354,  355,  356,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,  389,
  390,  391,  392,  393,  394,  395,  396,  397,  398,  399,
  400,  401,  402,  403,  404,  405,  406,  407,  408,  409,
  410,  411,  412,  413,  414,  415,  416,  417,  418,  419,
  256,   -1,    0,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   41,   -1,   -1,   44,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,   -1,   -1,    0,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,   -1,   -1,
   44,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,
   -1,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,  256,   -1,    0,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   41,   -1,   -1,   44,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,   -1,   -1,    0,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,
   -1,   -1,   44,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,  256,   -1,    0,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   41,   -1,   -1,
   44,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,   -1,   -1,
    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   41,   -1,   -1,   44,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,  256,   -1,    0,   -1,
   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   41,
   -1,   -1,   44,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,  338,  339,
  340,  341,  342,  343,  344,  345,  346,  347,  348,  349,
  350,  351,  352,  353,  354,  355,  356,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,  389,
  390,  391,  392,  393,  394,  395,  396,  397,  398,  399,
  400,  401,  402,  403,  404,  405,  406,  407,  408,  409,
  410,  411,  412,  413,  414,  415,  416,  417,  418,  419,
   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   41,   -1,   -1,   44,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,  256,   -1,
    0,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   41,   -1,   -1,   44,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   41,   -1,   -1,   44,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,
   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,
   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,  338,  339,
  340,  341,  342,  343,  344,  345,  346,  347,  348,  349,
  350,  351,  352,  353,  354,  355,  356,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,  389,
  390,  391,  392,  393,  394,  395,  396,  397,  398,  399,
  400,  401,  402,  403,  404,  405,  406,  407,  408,  409,
  410,  411,  412,  413,  414,  415,  416,  417,  418,  419,
  256,   -1,    0,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   41,   -1,   -1,   44,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,   -1,   -1,    0,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,   -1,   -1,
   44,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,
   -1,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,  256,   -1,    0,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   41,   -1,   -1,   44,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,   -1,   -1,    0,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,  256,    0,   -1,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   44,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,   -1,    0,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   44,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,  283,  284,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,
  305,  306,  307,  308,  309,  310,  311,  312,  313,  314,
  315,  316,  317,  318,  319,  320,  321,  322,  323,  324,
  325,  326,  327,  328,  329,  330,  331,  332,  333,  334,
  335,  336,  337,  338,  339,  340,  341,  342,  343,  344,
  345,  346,  347,  348,  349,  350,  351,  352,  353,  354,
  355,  356,  357,  358,  359,  360,  361,  362,  363,  364,
  365,  366,  367,  368,  369,  370,  371,  372,  373,  374,
  375,  376,  377,  378,  379,  380,  381,  382,  383,  384,
  385,  386,  387,  388,  389,  390,  391,  392,  393,  394,
  395,  396,  397,  398,  399,  400,  401,  402,  403,  404,
  405,  406,  407,  408,  409,  410,  411,  412,  413,  414,
  415,  416,  417,  418,  419,  256,   -1,    0,   -1,   -1,
   -1,   -1,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   41,   -1,
   -1,   -1,  303,  304,  305,  306,  307,  308,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,
  331,  332,  333,  334,  335,  336,  337,  338,  339,  340,
  341,  342,  343,  344,  345,  346,  347,  348,  349,  350,
  351,  352,  353,  354,  355,  356,  357,  358,  359,  360,
  361,  362,  363,  364,  365,  366,  367,  368,  369,  370,
  371,  372,  373,  374,  375,  376,  377,  378,  379,  380,
  381,  382,  383,  384,  385,  386,  387,  388,  389,  390,
  391,  392,  393,  394,  395,  396,  397,  398,  399,  400,
  401,  402,  403,  404,  405,  406,  407,  408,  409,  410,
  411,  412,  413,  414,  415,  416,  417,  418,  419,   -1,
   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   41,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,
  303,  304,  305,  306,  307,  308,  309,  310,  311,  312,
  313,  314,  315,  316,  317,  318,  319,  320,  321,  322,
  323,  324,  325,  326,  327,  328,  329,  330,  331,  332,
  333,  334,  335,  336,  337,  338,  339,  340,  341,  342,
  343,  344,  345,  346,  347,  348,  349,  350,  351,  352,
  353,  354,  355,  356,  357,  358,  359,  360,  361,  362,
  363,  364,  365,  366,  367,  368,  369,  370,  371,  372,
  373,  374,  375,  376,  377,  378,  379,  380,  381,  382,
  383,  384,  385,  386,  387,  388,  389,  390,  391,  392,
  393,  394,  395,  396,  397,  398,  399,  400,  401,  402,
  403,  404,  405,  406,  407,  408,  409,  410,  411,  412,
  413,  414,  415,  416,  417,  418,  419,  256,   -1,    0,
   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,
   41,   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,  331,  332,  333,  334,  335,  336,  337,  338,
  339,  340,  341,  342,  343,  344,  345,  346,  347,  348,
  349,  350,  351,  352,  353,  354,  355,  356,  357,  358,
  359,  360,  361,  362,  363,  364,  365,  366,  367,  368,
  369,  370,  371,  372,  373,  374,  375,  376,  377,  378,
  379,  380,  381,  382,  383,  384,  385,  386,  387,  388,
  389,  390,  391,  392,  393,  394,  395,  396,  397,  398,
  399,  400,  401,  402,  403,  404,  405,  406,  407,  408,
  409,  410,  411,  412,  413,  414,  415,  416,  417,  418,
  419,   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   41,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,   -1,
   -1,   -1,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,
   -1,   -1,  303,  304,  305,  306,  307,  308,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,
  331,  332,  333,  334,  335,  336,  337,  338,  339,  340,
  341,  342,  343,  344,  345,  346,  347,  348,  349,  350,
  351,  352,  353,  354,  355,  356,  357,  358,  359,  360,
  361,  362,  363,  364,  365,  366,  367,  368,  369,  370,
  371,  372,  373,  374,  375,  376,  377,  378,  379,  380,
  381,  382,  383,  384,  385,  386,  387,  388,  389,  390,
  391,  392,  393,  394,  395,  396,  397,  398,  399,  400,
  401,  402,  403,  404,  405,  406,  407,  408,  409,  410,
  411,  412,  413,  414,  415,  416,  417,  418,  419,  256,
   -1,    0,   -1,   -1,   -1,   -1,  263,  264,  265,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  275,  276,
  277,  278,  279,  280,  281,  282,  283,  284,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  298,   41,   -1,   -1,   -1,  303,  304,  305,  306,
  307,  308,  309,  310,  311,  312,  313,  314,  315,  316,
  317,  318,  319,  320,  321,  322,  323,  324,  325,  326,
  327,  328,  329,  330,  331,  332,  333,  334,  335,  336,
  337,  338,  339,  340,  341,  342,  343,  344,  345,  346,
  347,  348,  349,  350,  351,  352,  353,  354,  355,  356,
  357,  358,  359,  360,  361,  362,  363,  364,  365,  366,
  367,  368,  369,  370,  371,  372,  373,  374,  375,  376,
  377,  378,  379,  380,  381,  382,  383,  384,  385,  386,
  387,  388,  389,  390,  391,  392,  393,  394,  395,  396,
  397,  398,  399,  400,  401,  402,  403,  404,  405,  406,
  407,  408,  409,  410,  411,  412,  413,  414,  415,  416,
  417,  418,  419,   -1,   -1,    0,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   41,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,   -1,
   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,
   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,  331,  332,  333,  334,  335,  336,  337,  338,
  339,  340,  341,  342,  343,  344,  345,  346,  347,  348,
  349,  350,  351,  352,  353,  354,  355,  356,  357,  358,
  359,  360,  361,  362,  363,  364,  365,  366,  367,  368,
  369,  370,  371,  372,  373,  374,  375,  376,  377,  378,
  379,  380,  381,  382,  383,  384,  385,  386,  387,  388,
  389,  390,  391,  392,  393,  394,  395,  396,  397,  398,
  399,  400,  401,  402,  403,  404,  405,  406,  407,  408,
  409,  410,  411,  412,  413,  414,  415,  416,  417,  418,
  419,  256,    0,   -1,   -1,   -1,   -1,   -1,  263,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,  283,  284,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  298,   -1,   -1,   44,   -1,  303,  304,
  305,  306,  307,  308,  309,  310,  311,  312,  313,  314,
  315,  316,  317,  318,  319,  320,  321,  322,  323,  324,
  325,  326,  327,  328,  329,  330,  331,  332,  333,  334,
  335,  336,  337,  338,  339,  340,  341,  342,  343,  344,
  345,  346,  347,  348,  349,  350,  351,  352,  353,  354,
  355,  356,  357,  358,  359,  360,  361,  362,  363,  364,
  365,  366,  367,  368,  369,  370,  371,  372,  373,  374,
  375,  376,  377,  378,  379,  380,  381,  382,  383,  384,
  385,  386,  387,  388,  389,  390,  391,  392,  393,  394,
  395,  396,  397,  398,  399,  400,  401,  402,  403,  404,
  405,  406,  407,  408,  409,  410,  411,  412,  413,  414,
  415,  416,  417,  418,  419,   -1,    0,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,
   -1,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,  256,   -1,    0,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   41,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,   -1,   -1,    0,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,  256,   -1,    0,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   41,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,   -1,   -1,
    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   41,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,  256,   -1,    0,   -1,
   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   41,
   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,  338,  339,
  340,  341,  342,  343,  344,  345,  346,  347,  348,  349,
  350,  351,  352,  353,  354,  355,  356,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,  389,
  390,  391,  392,  393,  394,  395,  396,  397,  398,  399,
  400,  401,  402,  403,  404,  405,  406,  407,  408,  409,
  410,  411,  412,  413,  414,  415,  416,  417,  418,  419,
   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   41,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,  256,   -1,
    0,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   41,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   41,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,
   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,
   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,  338,  339,
  340,  341,  342,  343,  344,  345,  346,  347,  348,  349,
  350,  351,  352,  353,  354,  355,  356,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,  389,
  390,  391,  392,  393,  394,  395,  396,  397,  398,  399,
  400,  401,  402,  403,  404,  405,  406,  407,  408,  409,
  410,  411,  412,  413,  414,  415,  416,  417,  418,  419,
  256,   -1,    0,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   41,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,   -1,   -1,    0,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,
   -1,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,  256,   -1,    0,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   41,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,   -1,   -1,    0,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,  256,   -1,    0,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   41,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,   -1,   -1,
    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   41,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,  256,   -1,    0,   -1,
   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   41,
   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,  338,  339,
  340,  341,  342,  343,  344,  345,  346,  347,  348,  349,
  350,  351,  352,  353,  354,  355,  356,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,  389,
  390,  391,  392,  393,  394,  395,  396,  397,  398,  399,
  400,  401,  402,  403,  404,  405,  406,  407,  408,  409,
  410,  411,  412,  413,  414,  415,  416,  417,  418,  419,
   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   41,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,  256,   -1,
    0,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   41,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   41,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,
   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,
   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,  338,  339,
  340,  341,  342,  343,  344,  345,  346,  347,  348,  349,
  350,  351,  352,  353,  354,  355,  356,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,  389,
  390,  391,  392,  393,  394,  395,  396,  397,  398,  399,
  400,  401,  402,  403,  404,  405,  406,  407,  408,  409,
  410,  411,  412,  413,  414,  415,  416,  417,  418,  419,
  256,   -1,    0,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   41,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,   -1,   -1,    0,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,
   -1,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,  256,   -1,    0,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   41,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,   -1,   -1,    0,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,  256,    0,   -1,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,   -1,    0,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,  283,  284,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,
  305,  306,  307,  308,  309,  310,  311,  312,  313,  314,
  315,  316,  317,  318,  319,  320,  321,  322,  323,  324,
  325,  326,  327,  328,  329,  330,  331,  332,  333,  334,
  335,  336,  337,  338,  339,  340,  341,  342,  343,  344,
  345,  346,  347,  348,  349,  350,  351,  352,  353,  354,
  355,  356,  357,  358,  359,  360,  361,  362,  363,  364,
  365,  366,  367,  368,  369,  370,  371,  372,  373,  374,
  375,  376,  377,  378,  379,  380,  381,  382,  383,  384,
  385,  386,  387,  388,  389,  390,  391,  392,  393,  394,
  395,  396,  397,  398,  399,  400,  401,  402,  403,  404,
  405,  406,  407,  408,  409,  410,  411,  412,  413,  414,
  415,  416,  417,  418,  419,  256,    0,   -1,   -1,   -1,
   -1,   -1,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,
   -1,   -1,  303,  304,  305,  306,  307,  308,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,
  331,  332,  333,  334,  335,  336,  337,  338,  339,  340,
  341,  342,  343,  344,  345,  346,  347,  348,  349,  350,
  351,  352,  353,  354,  355,  356,  357,  358,  359,  360,
  361,  362,  363,  364,  365,  366,  367,  368,  369,  370,
  371,  372,  373,  374,  375,  376,  377,  378,  379,  380,
  381,  382,  383,  384,  385,  386,  387,  388,  389,  390,
  391,  392,  393,  394,  395,  396,  397,  398,  399,  400,
  401,  402,  403,  404,  405,  406,  407,  408,  409,  410,
  411,  412,  413,  414,  415,  416,  417,  418,  419,   -1,
    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,  256,    0,   -1,   -1,
   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,
   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,  338,  339,
  340,  341,  342,  343,  344,  345,  346,  347,  348,  349,
  350,  351,  352,  353,  354,  355,  356,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,  389,
  390,  391,  392,  393,  394,  395,  396,  397,  398,  399,
  400,  401,  402,  403,  404,  405,  406,  407,  408,  409,
  410,  411,  412,  413,  414,  415,  416,  417,  418,  419,
   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,
  303,  304,  305,  306,  307,  308,  309,  310,  311,  312,
  313,  314,  315,  316,  317,  318,  319,  320,  321,  322,
  323,  324,  325,  326,  327,  328,  329,  330,  331,  332,
  333,  334,  335,  336,  337,  338,  339,  340,  341,  342,
  343,  344,  345,  346,  347,  348,  349,  350,  351,  352,
  353,  354,  355,  356,  357,  358,  359,  360,  361,  362,
  363,  364,  365,  366,  367,  368,  369,  370,  371,  372,
  373,  374,  375,  376,  377,  378,  379,  380,  381,  382,
  383,  384,  385,  386,  387,  388,  389,  390,  391,  392,
  393,  394,  395,  396,  397,  398,  399,  400,  401,  402,
  403,  404,  405,  406,  407,  408,  409,  410,  411,  412,
  413,  414,  415,  416,  417,  418,  419,  256,    0,   -1,
   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,
   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,  331,  332,  333,  334,  335,  336,  337,  338,
  339,  340,  341,  342,  343,  344,  345,  346,  347,  348,
  349,  350,  351,  352,  353,  354,  355,  356,  357,  358,
  359,  360,  361,  362,  363,  364,  365,  366,  367,  368,
  369,  370,  371,  372,  373,  374,  375,  376,  377,  378,
  379,  380,  381,  382,  383,  384,  385,  386,  387,  388,
  389,  390,  391,  392,  393,  394,  395,  396,  397,  398,
  399,  400,  401,  402,  403,  404,  405,  406,  407,  408,
  409,  410,  411,  412,  413,  414,  415,  416,  417,  418,
  419,   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419,  256,    0,
   -1,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,   -1,
   -1,   -1,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,
   -1,   -1,  303,  304,  305,  306,  307,  308,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,
  331,  332,  333,  334,  335,  336,  337,  338,  339,  340,
  341,  342,  343,  344,  345,  346,  347,  348,  349,  350,
  351,  352,  353,  354,  355,  356,  357,  358,  359,  360,
  361,  362,  363,  364,  365,  366,  367,  368,  369,  370,
  371,  372,  373,  374,  375,  376,  377,  378,  379,  380,
  381,  382,  383,  384,  385,  386,  387,  388,  389,  390,
  391,  392,  393,  394,  395,  396,  397,  398,  399,  400,
  401,  402,  403,  404,  405,  406,  407,  408,  409,  410,
  411,  412,  413,  414,  415,  416,  417,  418,  419,  256,
    0,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  275,  276,
  277,  278,  279,  280,  281,  282,  283,  284,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,
  307,  308,  309,  310,  311,  312,  313,  314,  315,  316,
  317,  318,  319,  320,  321,  322,  323,  324,  325,  326,
  327,  328,  329,  330,  331,  332,  333,  334,  335,  336,
  337,  338,  339,  340,  341,  342,  343,  344,  345,  346,
  347,  348,  349,  350,  351,  352,  353,  354,  355,  356,
  357,  358,  359,  360,  361,  362,  363,  364,  365,  366,
  367,  368,  369,  370,  371,  372,  373,  374,  375,  376,
  377,  378,  379,  380,  381,  382,  383,  384,  385,  386,
  387,  388,  389,  390,  391,  392,  393,  394,  395,  396,
  397,  398,  399,  400,  401,  402,  403,  404,  405,  406,
  407,  408,  409,  410,  411,  412,  413,  414,  415,  416,
  417,  418,  419,   -1,    0,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,
   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,
   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,  338,  339,
  340,  341,  342,  343,  344,  345,  346,  347,  348,  349,
  350,  351,  352,  353,  354,  355,  356,  357,  358,  359,
  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,
  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,
  380,  381,  382,  383,  384,  385,  386,  387,  388,  389,
  390,  391,  392,  393,  394,  395,  396,  397,  398,  399,
  400,  401,  402,  403,  404,  405,  406,  407,  408,  409,
  410,  411,  412,  413,  414,  415,  416,  417,  418,  419,
  256,    0,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,   -1,    0,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,   -1,
   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,
   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,  331,  332,  333,  334,  335,  336,  337,  338,
  339,  340,  341,  342,  343,  344,  345,  346,  347,  348,
  349,  350,  351,  352,  353,  354,  355,  356,  357,  358,
  359,  360,  361,  362,  363,  364,  365,  366,  367,  368,
  369,  370,  371,  372,  373,  374,  375,  376,  377,  378,
  379,  380,  381,  382,  383,  384,  385,  386,  387,  388,
  389,  390,  391,  392,  393,  394,  395,  396,  397,  398,
  399,  400,  401,  402,  403,  404,  405,  406,  407,  408,
  409,  410,  411,  412,  413,  414,  415,  416,  417,  418,
  419,  256,    0,   -1,   -1,   -1,   -1,   -1,  263,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,  283,  284,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,
  305,  306,  307,  308,  309,  310,  311,  312,  313,  314,
  315,  316,  317,  318,  319,  320,  321,  322,  323,  324,
  325,  326,  327,  328,  329,  330,  331,  332,  333,  334,
  335,  336,  337,  338,  339,  340,  341,  342,  343,  344,
  345,  346,  347,  348,  349,  350,  351,  352,  353,  354,
  355,  356,  357,  358,  359,  360,  361,  362,  363,  364,
  365,  366,  367,  368,  369,  370,  371,  372,  373,  374,
  375,  376,  377,  378,  379,  380,  381,  382,  383,  384,
  385,  386,  387,  388,  389,  390,  391,  392,  393,  394,
  395,  396,  397,  398,  399,  400,  401,  402,  403,  404,
  405,  406,  407,  408,  409,  410,  411,  412,  413,  414,
  415,  416,  417,  418,  419,   -1,    0,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,
   -1,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  338,  339,  340,  341,  342,  343,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,
  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,
  378,  379,  380,  381,  382,  383,  384,  385,  386,  387,
  388,  389,  390,  391,  392,  393,  394,  395,  396,  397,
  398,  399,  400,  401,  402,  403,  404,  405,  406,  407,
  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,
  418,  419,  256,    0,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  338,  339,  340,  341,  342,  343,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,
  384,  385,  386,  387,  388,  389,  390,  391,  392,  393,
  394,  395,  396,  397,  398,  399,  400,  401,  402,  403,
  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,
  414,  415,  416,  417,  418,  419,   -1,    0,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  256,
   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  275,  276,
  277,  278,  279,  280,  281,  282,  283,  284,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,
  307,  308,  309,  310,  311,  312,  313,  314,  315,  316,
  317,  318,  319,  320,  321,  322,  323,  324,  325,  326,
  327,  328,  329,  330,  331,  332,  333,  334,  335,  336,
  337,  338,  339,  340,  341,  342,  343,  344,  345,  346,
  347,  348,  349,  350,  351,  352,  353,  354,  355,  356,
  357,  358,  359,  360,  361,  362,  363,  364,  365,  366,
  367,  368,  369,  370,  371,  372,  373,  374,  375,  376,
  377,  378,  379,  380,  381,  382,  383,  384,  385,  386,
  387,  388,  389,  390,  391,  392,  393,  394,  395,  396,
  397,  398,  399,  400,  401,  402,  403,  404,  405,  406,
  407,  408,  409,  410,  411,  412,  413,  414,  415,  416,
  417,  418,  419,  256,    0,   -1,   -1,   -1,   -1,   -1,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,
  303,  304,  305,  306,  307,  308,  309,  310,  311,  312,
  313,  314,  315,  316,  317,  318,  319,  320,  321,  322,
  323,  324,  325,  326,  327,  328,  329,  330,  331,  332,
  333,  334,  335,  336,  337,  338,  339,  340,  341,  342,
  343,  344,  345,  346,  347,  348,  349,  350,  351,  352,
  353,  354,  355,  356,  357,  358,  359,  360,  361,  362,
  363,  364,  365,  366,  367,  368,  369,  370,  371,  372,
  373,  374,  375,  376,  377,  378,  379,  380,  381,  382,
  383,  384,  385,  386,  387,  388,  389,  390,  391,  392,
  393,  394,  395,  396,  397,  398,  399,  400,  401,  402,
  403,  404,  405,  406,  407,  408,  409,  410,  411,  412,
  413,  414,  415,  416,  417,  418,  419,   -1,    0,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,  419,  256,   -1,   -1,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  419, 1020,   -1,
 1022,   -1,   -1,   -1,   -1,   40,   -1,   -1,   -1,   -1,
   45,   -1,   -1, 1035,   -1, 1037,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 1060,   -1,
   -1,   -1,   -1,   -1,   -1, 1067, 1068,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1, 1078, 1079,   -1,   -1,
   -1,   -1,   -1, 1085, 1086,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1, 1098,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
 1112,  126,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1, 1197,   -1, 1199, 1200,   -1,
 1202,   -1, 1204, 1205,   -1,   -1,   -1, 1209,   -1,   -1,
 1212, 1213, 1214, 1215,   -1,   -1, 1218,   -1,   -1, 1221,
 1222, 1223, 1224,   -1,   -1,   -1,   -1, 1229,   -1, 1231,
 1232, 1233,   -1,   -1, 1236,   -1, 1238, 1239, 1240, 1241,
 1242,   -1,  257,  258,  259,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  285,  286,  287,  288,  289,  290,  291,  292,  293,  294,
  295,  296,  297,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, 1311,
 1312, 1313, 1314,   -1,   -1,   -1, 1318,   -1,   -1,   -1,
   -1, 1323, 1324,   -1,   -1,   -1, 1328,   -1,   -1,   -1,
   -1, 1333, 1334,   -1,   -1,   -1, 1338,   -1,   -1,   -1,
 1342, 1343,   -1,   -1,   -1, 1347,   -1,   -1,   -1, 1351,
 1352,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  338,  339,  340,  341,
  342,  343,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  360,  361,
  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,
  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,
  382,  383,  384,  385,  386,  387,  388,  389,  390,  391,
  392,  393,  394,  395,  396,  397,  398,  399,  400,  401,
  402,  403,  404,  405,  406,  407,  408,  409,  410,  411,
  412,  413,  414,  415,  416,  417,  418,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,
  386,  387,  388,  389,  390,  391,  392,  393,  394,  395,
  396,  397,  398,  399,  400,  401,  402,  403,  404,  405,
  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,
  416,  417,  418,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 462
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,"'%'","'&'",0,"'('","')'","'*'","'+'","','","'-'",0,"'/'",0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,"'@'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'^'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'|'",0,"'~'",0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,"NAME","NUMBER","CHARCONST","FLOATNUM","LSHIFT","RSHIFT",
"USERERROR","USERWARNING","USERNOTE","BYTE","SHORT","WORD","FLOATY","DOUBLE",
"BLOCKBYTE","BLOCKSHORT","BLOCKWORD","SPACE","ALIGN","DATA","COMMON",
"CODETABLE","EXPORT","IMPORT","MODULE","INIT","REF","PATCHINSTR","MODSIZE",
"MODNUM","DATASYMB","CODESYMB","DATAMODULE","LABELREF","CODESTUB","ADDRSTUB",
"BYTESWAP","SHIFT","P_ADD","P_OR","STRINGCONST","LABEL","PLUSPLUS","MINUSMINUS",
"INSTRUCTION","UNARY","LDA","LDE","LDEP","LDF","LDHI","LDI","LDM","LDP","LDPE",
"LDPK","STF","STI","STIK","LBb","LBUb","LDFcond","LDIcond","LHw","LHUw","LWLct",
"LWRct","ABSF","ABSI","ADDC","ADDF","ADDI","AND","ANDN","ASH","CMPF","CMPI",
"FIX","FLOAT","FRIEEE","LSH","MPYF","MPYI","MPYSHI","MPYUHI","NEGB","NEGF",
"NEGI","NORM","NOT","OR","RCPF","RND","ROL","ROLC","ROR","RORC","RSQRF","SUBB",
"SUBC","SUBF","SUBI","SUBRB","SUBRF","SUBRI","TOIEEE","TSTB","XOR","MBct",
"MHct","ADDC3","ADDF3","ADDI3","AND3","ANDN3","ASH3","CMPF3","CMPI3","LSH3",
"MPYF3","MPYI3","MPYSHI3","MPYUHI3","OR3","SUBB3","SUBF3","SUBI3","TSTB3",
"XOR3","BR","BRD","CALL","LAJ","RPTB","RPTBD","RPTS","Bcond","BcondAF",
"BcondAT","BcondD","CALLcond","DBcond","DBcondD","LAJcond","LATcond","RETIcond",
"RETIcondD","RETScond","TRAPcond","LDFI","LDII","SIGI","STFI","STII","POPF",
"POP","PUSH","PUSHF","IDLE","SWI","NOP","IACK","C40FLOAT","PATCHC40DATAMODULE1",
"PATCHC40DATAMODULE2","PATCHC40DATAMODULE3","PATCHC40DATAMODULE4",
"PATCHC40DATAMODULE5","PATCHC40MASK24ADD","PATCHC40MASK16ADD",
"PATCHC40MASK8ADD","R0","R1","R2","R3","R4","R5","R6","R7","R8","R9","R10",
"R11","AR0","AR1","AR2","AR3","AR4","AR5","AR6","AR7","DP","IR0","IR1","BK",
"SP","ST","DIE","IIE","IIF","RS","RE","RC","IVTP","TVTP","BARBAR",
};
char *yyrule[] = {
"$accept : statement",
"statement :",
"$$1 :",
"statement : statement $$1 label",
"$$2 :",
"statement : statement $$2 mnemonic",
"statement : statement error",
"label : LABEL",
"mnemonic : pseudo_op",
"mnemonic : machine_op",
"pseudo_op : byte",
"pseudo_op : short",
"pseudo_op : word",
"pseudo_op : blockbyte",
"pseudo_op : blockshort",
"pseudo_op : blockword",
"pseudo_op : double",
"pseudo_op : floaty",
"pseudo_op : c40float",
"pseudo_op : space",
"pseudo_op : align",
"pseudo_op : data",
"pseudo_op : common",
"pseudo_op : codetable",
"pseudo_op : export",
"pseudo_op : import",
"pseudo_op : module",
"pseudo_op : init",
"pseudo_op : ref",
"pseudo_op : patchinstr",
"pseudo_op : usererror",
"pseudo_op : userwarning",
"pseudo_op : usernote",
"byte : BYTE codeconstlist",
"short : SHORT codeconstlist",
"word : WORD codeconstlist",
"floaty : FLOATY floatconstlist",
"double : DOUBLE floatconstlist",
"c40float : C40FLOAT floatconstlist",
"blockbyte : BLOCKBYTE imm_constexpr ',' codeconstlist",
"blockshort : BLOCKSHORT imm_constexpr ',' codeconstlist",
"blockword : BLOCKWORD imm_constexpr ',' codeconstlist",
"space : SPACE imm_constexpr",
"align : ALIGN opt_imm_constexpr",
"data : DATA NAME ',' constexpr",
"common : COMMON NAME ',' constexpr",
"codetable : CODETABLE namelist",
"export : EXPORT namelist",
"import : IMPORT namelist",
"module : MODULE opt_constexpr",
"init : INIT",
"ref : REF namelist",
"patchinstr : PATCHINSTR '(' instrpatch ',' stdpatch ',' machine_op ')'",
"usererror : USERERROR STRINGCONST",
"userwarning : USERWARNING STRINGCONST",
"usernote : USERNOTE STRINGCONST",
"stdpatch : MODSIZE",
"stdpatch : MODNUM",
"stdpatch : DATASYMB '(' NAME ')'",
"stdpatch : CODESYMB '(' NAME ')'",
"stdpatch : DATAMODULE '(' NAME ')'",
"stdpatch : LABELREF '(' NAME ')'",
"stdpatch : CODESTUB '(' NAME ')'",
"stdpatch : ADDRSTUB '(' NAME ')'",
"stdpatch : BYTESWAP '(' stdpatch ')'",
"stdpatch : SHIFT '(' constexpr ',' stdpatch ')'",
"stdpatch : P_ADD '(' constexpr ',' stdpatch ')'",
"stdpatch : P_OR '(' constexpr ',' stdpatch ')'",
"namelist : NAME",
"namelist : namelist ',' NAME",
"$$3 :",
"codeconstlist : $$3 codeconst",
"codeconstlist : codeconstlist ',' codeconst",
"$$4 :",
"floatconstlist : $$4 imm_fp_constexpr",
"floatconstlist : floatconstlist ',' imm_fp_constexpr",
"codeconst : constexpr",
"codeconst : stdpatch",
"codeconst : STRINGCONST",
"imm_constexpr : '(' imm_constexpr ')'",
"imm_constexpr : imm_constexpr '+' imm_constexpr",
"imm_constexpr : imm_constexpr '-' imm_constexpr",
"imm_constexpr : imm_constexpr '*' imm_constexpr",
"imm_constexpr : imm_constexpr '/' imm_constexpr",
"imm_constexpr : imm_constexpr '%' imm_constexpr",
"imm_constexpr : imm_constexpr '&' imm_constexpr",
"imm_constexpr : imm_constexpr '|' imm_constexpr",
"imm_constexpr : imm_constexpr '^' imm_constexpr",
"imm_constexpr : imm_constexpr LSHIFT imm_constexpr",
"imm_constexpr : imm_constexpr RSHIFT imm_constexpr",
"imm_constexpr : '~' imm_constexpr",
"imm_constexpr : '-' imm_constexpr",
"imm_constexpr : NUMBER",
"imm_constexpr : CHARCONST",
"imm_fp_constexpr : '(' imm_fp_constexpr ')'",
"imm_fp_constexpr : imm_fp_constexpr '+' imm_fp_constexpr",
"imm_fp_constexpr : imm_fp_constexpr '-' imm_fp_constexpr",
"imm_fp_constexpr : imm_fp_constexpr '*' imm_fp_constexpr",
"imm_fp_constexpr : imm_fp_constexpr '/' imm_fp_constexpr",
"imm_fp_constexpr : '-' imm_fp_constexpr",
"imm_fp_constexpr : NUMBER",
"imm_fp_constexpr : FLOATNUM",
"opt_imm_constexpr :",
"opt_imm_constexpr : imm_constexpr",
"constexpr : '(' constexpr ')'",
"constexpr : constexpr '+' constexpr",
"constexpr : constexpr '-' constexpr",
"constexpr : constexpr '*' constexpr",
"constexpr : constexpr '/' constexpr",
"constexpr : constexpr '%' constexpr",
"constexpr : constexpr '&' constexpr",
"constexpr : constexpr '|' constexpr",
"constexpr : constexpr '^' constexpr",
"constexpr : constexpr LSHIFT constexpr",
"constexpr : constexpr RSHIFT constexpr",
"constexpr : '~' constexpr",
"constexpr : '-' constexpr",
"constexpr : NUMBER",
"constexpr : CHARCONST",
"constexpr : NAME",
"opt_constexpr :",
"opt_constexpr : constexpr",
"reg : R0",
"reg : R1",
"reg : R2",
"reg : R3",
"reg : R4",
"reg : R5",
"reg : R6",
"reg : R7",
"reg : R8",
"reg : R9",
"reg : R10",
"reg : R11",
"reg : AR0",
"reg : AR1",
"reg : AR2",
"reg : AR3",
"reg : AR4",
"reg : AR5",
"reg : AR6",
"reg : AR7",
"reg : DP",
"reg : IR0",
"reg : IR1",
"reg : BK",
"reg : SP",
"reg : ST",
"reg : DIE",
"reg : IIE",
"reg : IIF",
"reg : RS",
"reg : RE",
"reg : RC",
"reg : error",
"Dreg : R0",
"Dreg : R1",
"Dreg : R2",
"Dreg : R3",
"Dreg : R4",
"Dreg : R5",
"Dreg : R6",
"Dreg : R7",
"Dreg : R8",
"Dreg : R9",
"Dreg : R10",
"Dreg : R11",
"Dreg : error",
"Dreg0_1 : R0",
"Dreg0_1 : R1",
"Dreg0_1 : error",
"Dreg2_3 : R2",
"Dreg2_3 : R3",
"Dreg2_3 : error",
"Dreg0_7 : R0",
"Dreg0_7 : R1",
"Dreg0_7 : R2",
"Dreg0_7 : R3",
"Dreg0_7 : R4",
"Dreg0_7 : R5",
"Dreg0_7 : R6",
"Dreg0_7 : R7",
"Dreg0_7 : error",
"Areg : AR0",
"Areg : AR1",
"Areg : AR2",
"Areg : AR3",
"Areg : AR4",
"Areg : AR5",
"Areg : AR6",
"Areg : AR7",
"Addr_reg : AR0",
"Addr_reg : AR1",
"Addr_reg : AR2",
"Addr_reg : AR3",
"Addr_reg : AR4",
"Addr_reg : AR5",
"Addr_reg : AR6",
"Addr_reg : AR7",
"Addr_reg : IR0",
"Addr_reg : IR1",
"Addr_reg : DP",
"Addr_reg : BK",
"Addr_reg : SP",
"Addr_reg : error",
"Exp_reg : IVTP",
"Exp_reg : TVTP",
"Exp_reg : error",
"direct : '@' constexpr",
"immediate : constexpr",
"fp_immediate : imm_fp_constexpr",
"indirect : '*' Areg",
"indirect : '*' prefix Areg displacement",
"indirect : '*' Areg postfix",
"indirect : '*' error",
"prefix : '+'",
"prefix : '-'",
"prefix : PLUSPLUS",
"prefix : MINUSMINUS",
"postfix : PLUSPLUS '(' IR0 ')' bitreverse",
"postfix : PLUSPLUS displacement",
"postfix : MINUSMINUS displacement",
"postfix : PLUSPLUS displacement '%'",
"postfix : MINUSMINUS displacement '%'",
"displacement :",
"displacement : '(' constexpr ')'",
"displacement : '(' IR0 ')'",
"displacement : '(' IR1 ')'",
"bitreverse : NAME",
"pcrel : constexpr",
"pcrel : '@' constexpr",
"int_ld_Addr_reg_modes : reg ',' Addr_reg",
"int_ld_Addr_reg_modes : indirect ',' Addr_reg",
"int_ld_Addr_reg_modes : direct ',' Addr_reg",
"int_ld_Addr_reg_modes : immediate ',' Addr_reg",
"int_ld_Addr_reg_modes : error",
"st_addr_modes : reg ',' direct",
"st_addr_modes : reg ',' indirect",
"st_addr_modes : reg ',' error",
"int_unary_op_mode : reg",
"fp_unary_op_mode : Dreg",
"int_diadic_modes : reg ',' reg",
"int_diadic_modes : direct ',' reg",
"int_diadic_modes : indirect ',' reg",
"int_diadic_modes : immediate ',' reg",
"fp_diadic_modes : Dreg ',' Dreg",
"fp_diadic_modes : direct ',' Dreg",
"fp_diadic_modes : indirect ',' Dreg",
"fp_diadic_modes : fp_immediate ',' Dreg",
"float_diadic_modes : reg ',' Dreg",
"float_diadic_modes : direct ',' Dreg",
"float_diadic_modes : indirect ',' Dreg",
"float_diadic_modes : immediate ',' Dreg",
"int_triadic_modes : reg ',' reg ',' reg",
"int_triadic_modes : reg ',' indirect ',' reg",
"int_triadic_modes : immediate ',' reg ',' reg",
"int_triadic_modes : immediate ',' indirect ',' reg",
"int_triadic_modes : indirect ',' reg ',' reg",
"int_triadic_modes : indirect ',' indirect ',' reg",
"fp_triadic_modes : Dreg ',' Dreg ',' Dreg",
"fp_triadic_modes : Dreg ',' indirect ',' Dreg",
"fp_triadic_modes : indirect ',' Dreg ',' Dreg",
"fp_triadic_modes : indirect ',' indirect ',' Dreg",
"tri_par_sti_mode : indirect ',' reg ',' reg BARBAR STI Dreg0_7 ',' indirect",
"tri_par_sti_mode : indirect ',' reg BARBAR STI Dreg0_7 ',' indirect",
"dia_par_sti_mode : indirect ',' reg BARBAR STI Dreg0_7 ',' indirect",
"shift_tri_par_sti_mode : reg ',' indirect ',' reg BARBAR STI Dreg0_7 ',' indirect",
"tri_par_stf_mode : indirect ',' Dreg ',' Dreg BARBAR STF Dreg0_7 ',' indirect",
"tri_par_stf_mode : indirect ',' Dreg BARBAR STF Dreg0_7 ',' indirect",
"dia_par_stf_mode : indirect ',' Dreg BARBAR STF Dreg0_7 ',' indirect",
"shift_tri_par_stf_mode : Dreg ',' indirect ',' Dreg BARBAR STF Dreg0_7 ',' indirect",
"par_mpyi_mode : indirect ',' indirect ',' reg BARBAR par_mpyi_op2 Dreg0_7 ',' Dreg0_7 ',' Dreg2_3",
"par_mpyi_mode : indirect ',' reg ',' reg BARBAR par_mpyi_op2 indirect ',' Dreg0_7 ',' Dreg2_3",
"par_mpyi_mode : reg ',' indirect ',' Dreg0_1 BARBAR par_mpyi_op2 indirect ',' Dreg0_7 ',' Dreg2_3",
"par_mpyi_mode : reg ',' reg ',' Dreg0_1 BARBAR par_mpyi_op2 indirect ',' indirect ',' Dreg2_3",
"par_mpyi_mode : indirect ',' reg ',' reg BARBAR par_mpyi_op2 Dreg0_7 ',' indirect ',' Dreg2_3",
"par_mpyi_mode : reg ',' indirect ',' Dreg0_1 BARBAR par_mpyi_op2 Dreg0_7 ',' indirect ',' Dreg2_3",
"par_mpyi_mode : indirect ',' reg BARBAR par_mpyi_op2 indirect ',' Dreg0_7 ',' Dreg2_3",
"par_mpyi_mode : reg ',' reg BARBAR par_mpyi_op2 indirect ',' indirect ',' Dreg2_3",
"par_mpyi_mode : indirect ',' reg BARBAR par_mpyi_op2 Dreg0_7 ',' indirect ',' Dreg2_3",
"par_mpyi_mode : indirect ',' reg BARBAR par_mpyi_op2 indirect ',' Dreg2_3",
"par_mpyi_mode : indirect ',' indirect ',' reg BARBAR par_mpyi_op2 Dreg0_7 ',' Dreg2_3",
"par_mpyi_mode : indirect ',' reg ',' reg BARBAR par_mpyi_op2 indirect ',' Dreg2_3",
"par_mpyi_mode : reg ',' indirect ',' Dreg0_1 BARBAR par_mpyi_op2 indirect ',' Dreg2_3",
"par_mpyi_op2 : SUBI",
"par_mpyi_op2 : ADDI",
"par_mpyi_op2 : SUBI3",
"par_mpyi_op2 : ADDI3",
"par_mpyf_mode : indirect ',' indirect ',' Dreg BARBAR par_mpyf_op2 Dreg0_7 ',' Dreg0_7 ',' Dreg2_3",
"par_mpyf_mode : indirect ',' Dreg ',' Dreg BARBAR par_mpyf_op2 indirect ',' Dreg0_7 ',' Dreg2_3",
"par_mpyf_mode : Dreg ',' indirect ',' Dreg0_1 BARBAR par_mpyf_op2 indirect ',' Dreg0_7 ',' Dreg2_3",
"par_mpyf_mode : Dreg ',' Dreg ',' Dreg0_1 BARBAR par_mpyf_op2 indirect ',' indirect ',' Dreg2_3",
"par_mpyf_mode : indirect ',' Dreg ',' Dreg BARBAR par_mpyf_op2 Dreg0_7 ',' indirect ',' Dreg2_3",
"par_mpyf_mode : Dreg ',' indirect ',' Dreg0_1 BARBAR par_mpyf_op2 Dreg0_7 ',' indirect ',' Dreg2_3",
"par_mpyf_mode : indirect ',' Dreg BARBAR par_mpyf_op2 indirect ',' Dreg0_7 ',' Dreg2_3",
"par_mpyf_mode : Dreg ',' Dreg BARBAR par_mpyf_op2 indirect ',' indirect ',' Dreg2_3",
"par_mpyf_mode : indirect ',' Dreg BARBAR par_mpyf_op2 Dreg0_7 ',' indirect ',' Dreg2_3",
"par_mpyf_mode : indirect ',' Dreg BARBAR par_mpyf_op2 indirect ',' Dreg2_3",
"par_mpyf_mode : indirect ',' indirect ',' Dreg BARBAR par_mpyf_op2 Dreg0_7 ',' Dreg2_3",
"par_mpyf_mode : indirect ',' Dreg ',' Dreg BARBAR par_mpyf_op2 indirect ',' Dreg2_3",
"par_mpyf_mode : Dreg ',' indirect ',' Dreg0_1 BARBAR par_mpyf_op2 indirect ',' Dreg2_3",
"par_mpyf_op2 : SUBF",
"par_mpyf_op2 : ADDF",
"par_mpyf_op2 : SUBF3",
"par_mpyf_op2 : ADDF3",
"rev_par_mpyi_mode : reg ',' reg ',' Dreg2_3 BARBAR mpyi2_3 indirect ',' indirect ',' Dreg0_1",
"rev_par_mpyi_mode : indirect ',' reg ',' reg BARBAR mpyi2_3 indirect ',' Dreg0_7 ',' Dreg0_1",
"rev_par_mpyi_mode : indirect ',' reg ',' reg BARBAR mpyi2_3 Dreg0_7 ',' indirect ',' Dreg0_1",
"rev_par_mpyi_mode : indirect ',' indirect ',' Dreg2_3 BARBAR mpyi2_3 Dreg0_7 ',' Dreg0_7 ',' Dreg0_1",
"rev_par_mpyi_mode : reg ',' indirect ',' reg BARBAR mpyi2_3 indirect ',' Dreg0_7 ',' Dreg0_1",
"rev_par_mpyi_mode : reg ',' indirect ',' reg BARBAR mpyi2_3 Dreg0_7 ',' indirect ',' Dreg0_1",
"rev_par_mpyi_mode : indirect ',' reg ',' reg BARBAR mpyi2_3 indirect ',' Dreg0_1",
"rev_par_mpyi_mode : indirect ',' indirect ',' Dreg2_3 BARBAR mpyi2_3 Dreg0_7 ',' Dreg0_1",
"rev_par_mpyi_mode : reg ',' indirect ',' reg BARBAR mpyi2_3 indirect ',' Dreg0_1",
"rev_par_mpyi_mode : indirect ',' reg BARBAR mpyi2_3 indirect ',' Dreg0_1",
"rev_par_mpyi_mode : reg ',' Dreg2_3 BARBAR mpyi2_3 indirect ',' indirect ',' Dreg0_1",
"rev_par_mpyi_mode : indirect ',' reg BARBAR mpyi2_3 indirect ',' Dreg0_7 ',' Dreg0_1",
"rev_par_mpyi_mode : indirect ',' reg BARBAR mpyi2_3 Dreg0_7 ',' indirect ',' Dreg0_1",
"mpyi2_3 : MPYI",
"mpyi2_3 : MPYI3",
"rev_par_mpyf_mode : Dreg ',' Dreg ',' Dreg2_3 BARBAR mpyf2_3 indirect ',' indirect ',' Dreg0_1",
"rev_par_mpyf_mode : indirect ',' Dreg ',' Dreg BARBAR mpyf2_3 indirect ',' Dreg0_7 ',' Dreg0_1",
"rev_par_mpyf_mode : indirect ',' Dreg ',' Dreg BARBAR mpyf2_3 Dreg0_7 ',' indirect ',' Dreg0_1",
"rev_par_mpyf_mode : indirect ',' indirect ',' Dreg2_3 BARBAR mpyf2_3 Dreg0_7 ',' Dreg0_7 ',' Dreg0_1",
"rev_par_mpyf_mode : Dreg ',' indirect ',' Dreg BARBAR mpyf2_3 indirect ',' Dreg0_7 ',' Dreg0_1",
"rev_par_mpyf_mode : Dreg ',' indirect ',' Dreg BARBAR mpyf2_3 Dreg0_7 ',' indirect ',' Dreg0_1",
"rev_par_mpyf_mode : indirect ',' Dreg ',' Dreg BARBAR mpyf2_3 indirect ',' Dreg0_1",
"rev_par_mpyf_mode : indirect ',' indirect ',' Dreg2_3 BARBAR mpyf2_3 Dreg0_7 ',' Dreg0_1",
"rev_par_mpyf_mode : Dreg ',' indirect ',' Dreg BARBAR mpyf2_3 indirect ',' Dreg0_1",
"rev_par_mpyf_mode : indirect ',' Dreg BARBAR mpyf2_3 indirect ',' Dreg0_1",
"rev_par_mpyf_mode : Dreg ',' Dreg2_3 BARBAR mpyf2_3 indirect ',' indirect ',' Dreg0_1",
"rev_par_mpyf_mode : indirect ',' Dreg BARBAR mpyf2_3 indirect ',' Dreg0_7 ',' Dreg0_1",
"rev_par_mpyf_mode : indirect ',' Dreg BARBAR mpyf2_3 Dreg0_7 ',' indirect ',' Dreg0_1",
"mpyf2_3 : MPYF",
"mpyf2_3 : MPYF3",
"$$5 :",
"machine_op : $$5 C40_op",
"C40_op : loads",
"C40_op : stores",
"C40_op : branches",
"C40_op : stackops",
"C40_op : arith_logic_ops",
"C40_op : miscops",
"loads : load_ints",
"loads : par_load_int",
"loads : load_fp",
"loads : par_load_fp",
"loads : load_parts",
"loads : load_interlocked",
"loads : LDA int_ld_Addr_reg_modes",
"loads : LDHI immediate ',' reg",
"loads : LDP immediate ',' reg",
"loads : LDP immediate",
"loads : LDP '@' immediate ',' reg",
"loads : LDP '@' immediate",
"loads : LDPK immediate",
"loads : LDEP Exp_reg ',' reg",
"loads : LDPE reg ',' Exp_reg",
"load_ints : LDIcond int_diadic_modes",
"load_fp : fp_loads fp_diadic_modes",
"fp_loads : LDE",
"fp_loads : LDM",
"fp_loads : LDFcond",
"par_load_int : LDI reg ',' reg",
"par_load_int : LDI direct ',' reg",
"par_load_int : LDI indirect ',' reg",
"par_load_int : LDI immediate ',' reg",
"par_load_int : LDI indirect ',' reg BARBAR LDI indirect ',' reg",
"par_load_int : LDI indirect ',' reg BARBAR STI Dreg0_7 ',' indirect",
"par_load_fp : LDF Dreg ',' Dreg",
"par_load_fp : LDF direct ',' Dreg",
"par_load_fp : LDF indirect ',' Dreg",
"par_load_fp : LDF fp_immediate ',' Dreg",
"par_load_fp : LDF indirect ',' Dreg BARBAR LDF indirect ',' Dreg",
"par_load_fp : LDF indirect ',' Dreg BARBAR STF Dreg0_7 ',' indirect",
"load_parts : part_loads reg ',' reg",
"load_parts : part_loads direct ',' reg",
"load_parts : part_loads indirect ',' reg",
"part_loads : LBb",
"part_loads : LBUb",
"part_loads : LHw",
"part_loads : LHUw",
"part_loads : LWLct",
"part_loads : LWRct",
"part_loads : MBct",
"part_loads : MHct",
"load_interlocked : interlocked_loads direct ',' reg",
"load_interlocked : interlocked_loads indirect ',' reg",
"interlocked_loads : LDFI",
"interlocked_loads : LDII",
"interlocked_loads : SIGI",
"stores : genstores st_addr_modes",
"stores : STIK immediate ',' direct",
"stores : STIK immediate ',' indirect",
"stores : STF Dreg ',' direct",
"stores : STF Dreg ',' indirect",
"stores : STI reg ',' direct",
"stores : STI reg ',' indirect",
"stores : STF Dreg ',' indirect BARBAR STF Dreg0_7 ',' indirect",
"stores : STF Dreg ',' indirect BARBAR LDF indirect ',' Dreg0_7",
"stores : STF Dreg ',' indirect BARBAR tri_swap_par_stf indirect ',' Dreg0_7 ',' Dreg0_7",
"stores : STF Dreg ',' indirect BARBAR tri_swap_par_stf indirect ',' Dreg0_7",
"stores : STF Dreg ',' indirect BARBAR dia_swap_par_stf indirect ',' Dreg0_7",
"stores : STF Dreg ',' indirect BARBAR shift_swap_par_stf Dreg0_7 ',' indirect ',' Dreg0_7",
"stores : STI reg ',' indirect BARBAR STI Dreg0_7 ',' indirect",
"stores : STI reg ',' indirect BARBAR LDI indirect ',' Dreg0_7",
"stores : STI reg ',' indirect BARBAR tri_swap_par_sti indirect ',' Dreg0_7 ',' Dreg0_7",
"stores : STI reg ',' indirect BARBAR tri_swap_par_sti indirect ',' Dreg0_7",
"stores : STI reg ',' indirect BARBAR dia_swap_par_sti indirect ',' Dreg0_7",
"stores : STI reg ',' indirect BARBAR shift_swap_par_sti Dreg0_7 ',' indirect ',' Dreg0_7",
"genstores : STFI",
"genstores : STII",
"tri_swap_par_sti : ADDI",
"tri_swap_par_sti : ADDI3",
"tri_swap_par_sti : AND",
"tri_swap_par_sti : AND3",
"tri_swap_par_sti : FIX",
"tri_swap_par_sti : MPYI",
"tri_swap_par_sti : MPYI3",
"tri_swap_par_sti : OR",
"tri_swap_par_sti : OR3",
"tri_swap_par_sti : XOR",
"tri_swap_par_sti : XOR3",
"dia_swap_par_sti : ABSI",
"dia_swap_par_sti : NEGI",
"dia_swap_par_sti : NOT",
"shift_swap_par_sti : ASH3",
"shift_swap_par_sti : ASH",
"shift_swap_par_sti : LSH3",
"shift_swap_par_sti : LSH",
"shift_swap_par_sti : SUBI",
"shift_swap_par_sti : SUBI3",
"tri_swap_par_stf : ADDF",
"tri_swap_par_stf : ADDF3",
"tri_swap_par_stf : MPYF",
"tri_swap_par_stf : MPYF3",
"dia_swap_par_stf : ABSF",
"dia_swap_par_stf : FLOAT",
"dia_swap_par_stf : FRIEEE",
"dia_swap_par_stf : TOIEEE",
"shift_swap_par_stf : SUBF",
"shift_swap_par_stf : SUBF3",
"branches : bigbranches",
"branches : condbranches",
"branches : decbranches",
"branches : returns",
"branches : traps",
"branches : repeats",
"bigbranches : bigbranch_ops pcrel",
"bigbranch_ops : BR",
"bigbranch_ops : BRD",
"bigbranch_ops : CALL",
"bigbranch_ops : LAJ",
"condbranches : condbranch pcrel",
"condbranches : condbranch reg",
"condbranch : Bcond",
"condbranch : BcondAF",
"condbranch : BcondAT",
"condbranch : BcondD",
"condbranch : CALLcond",
"condbranch : LAJcond",
"decbranches : DBcond Areg ',' reg",
"decbranches : DBcond Areg ',' pcrel",
"decbranches : DBcondD Areg ',' reg",
"decbranches : DBcondD Areg ',' pcrel",
"returns : ret_ops",
"ret_ops : RETIcond",
"ret_ops : RETIcondD",
"ret_ops : RETScond",
"traps : trap_ops immediate",
"trap_ops : LATcond",
"trap_ops : TRAPcond",
"repeats : rptb_ops pcrel",
"repeats : rptb_ops reg",
"repeats : RPTS reg",
"repeats : RPTS direct",
"repeats : RPTS immediate",
"repeats : RPTS indirect",
"rptb_ops : RPTB",
"rptb_ops : RPTBD",
"stackops : PUSH reg",
"stackops : PUSHF Dreg",
"stackops : POP reg",
"stackops : POPF Dreg",
"miscops : IACK direct",
"miscops : IACK indirect",
"miscops : IDLE",
"miscops : NOP indirect",
"miscops : NOP",
"miscops : SWI",
"arith_logic_ops : int_unary_only",
"arith_logic_ops : fp_unary_only",
"arith_logic_ops : int_diadic_only",
"arith_logic_ops : fp_diadic_only",
"arith_logic_ops : int_triadic_only",
"arith_logic_ops : int_both_2_3",
"arith_logic_ops : int_par_st_unary",
"arith_logic_ops : fp_par_st_unary",
"arith_logic_ops : float_par_st_unary",
"arith_logic_ops : unsigned_int_par_st_3",
"arith_logic_ops : unsigned_int_par_st_2_3",
"arith_logic_ops : shift_int_par_st_3",
"arith_logic_ops : shift_int_par_st_2_3",
"arith_logic_ops : fp_par_st_mpy_3",
"arith_logic_ops : fp_par_st_mpy_2_3",
"arith_logic_ops : shift_fp_par_st_mpy_3",
"arith_logic_ops : shift_fp_par_st_mpy_2_3",
"arith_logic_ops : int_par_st_mpy_3",
"arith_logic_ops : int_par_st_mpy_2_3",
"arith_logic_ops : shift_int_par_st_mpy_3",
"arith_logic_ops : shift_int_par_st_mpy_2_3",
"arith_logic_ops : mpy",
"arith_logic_ops : reciprocals",
"arith_logic_ops : rotates",
"arith_logic_ops : cmp",
"arith_logic_ops : frieee",
"arith_logic_ops : toieee",
"arith_logic_ops : fix",
"int_unary_only : NEGB int_unary_op_mode",
"int_unary_only : NEGB int_diadic_modes",
"fp_unary_only : fp_unary_only_ops fp_unary_op_mode",
"fp_unary_only : fp_unary_only_ops fp_diadic_modes",
"fp_unary_only_ops : NORM",
"fp_unary_only_ops : RND",
"int_diadic_only : int_diadic_only_ops int_diadic_modes",
"int_diadic_only_ops : SUBC",
"int_diadic_only_ops : SUBRB",
"int_diadic_only_ops : SUBRI",
"fp_diadic_only : SUBRF fp_diadic_modes",
"int_triadic_only : int_triadic_only_ops int_triadic_modes",
"int_triadic_only_ops : ADDC3",
"int_triadic_only_ops : ANDN3",
"int_triadic_only_ops : MPYSHI3",
"int_triadic_only_ops : MPYUHI3",
"int_triadic_only_ops : SUBB3",
"int_both_2_3 : int_both_2_3_ops int_diadic_modes",
"int_both_2_3 : int_both_2_3_ops int_triadic_modes",
"int_both_2_3_ops : ADDC",
"int_both_2_3_ops : ANDN",
"int_both_2_3_ops : MPYSHI",
"int_both_2_3_ops : MPYUHI",
"int_both_2_3_ops : SUBB",
"int_par_st_unary : int_par_st_unary_ops int_unary_op_mode",
"int_par_st_unary : int_par_st_unary_ops int_diadic_modes",
"int_par_st_unary : int_par_st_unary_ops dia_par_sti_mode",
"int_par_st_unary_ops : ABSI",
"int_par_st_unary_ops : NEGI",
"int_par_st_unary_ops : NOT",
"fp_par_st_unary : fp_par_st_unary_ops fp_unary_op_mode",
"fp_par_st_unary : fp_par_st_unary_ops fp_diadic_modes",
"fp_par_st_unary : fp_par_st_unary_ops dia_par_stf_mode",
"fp_par_st_unary_ops : ABSF",
"fp_par_st_unary_ops : NEGF",
"float_par_st_unary : FLOAT fp_unary_op_mode",
"float_par_st_unary : FLOAT float_diadic_modes",
"float_par_st_unary : FLOAT dia_par_stf_mode",
"unsigned_int_par_st_3 : int_par_st_3_ops int_triadic_modes",
"unsigned_int_par_st_3 : int_par_st_3_ops tri_par_sti_mode",
"int_par_st_3_ops : AND3",
"int_par_st_3_ops : OR3",
"int_par_st_3_ops : XOR3",
"shift_int_par_st_3 : shift_par_st_3_ops int_triadic_modes",
"shift_int_par_st_3 : shift_par_st_3_ops shift_tri_par_sti_mode",
"shift_par_st_3_ops : ASH3",
"shift_par_st_3_ops : LSH3",
"shift_int_par_st_mpy_3 : SUBI3 int_triadic_modes",
"shift_int_par_st_mpy_3 : SUBI3 shift_tri_par_sti_mode",
"shift_int_par_st_mpy_3 : SUBI3 rev_par_mpyi_mode",
"int_par_st_mpy_3 : ADDI3 int_triadic_modes",
"int_par_st_mpy_3 : ADDI3 tri_par_sti_mode",
"int_par_st_mpy_3 : ADDI3 rev_par_mpyi_mode",
"shift_fp_par_st_mpy_3 : SUBF3 fp_triadic_modes",
"shift_fp_par_st_mpy_3 : SUBF3 shift_tri_par_stf_mode",
"shift_fp_par_st_mpy_3 : SUBF3 rev_par_mpyf_mode",
"fp_par_st_mpy_3 : ADDF3 fp_triadic_modes",
"fp_par_st_mpy_3 : ADDF3 tri_par_stf_mode",
"fp_par_st_mpy_3 : ADDF3 rev_par_mpyf_mode",
"unsigned_int_par_st_2_3 : int_par_st_2_3_ops int_diadic_modes",
"unsigned_int_par_st_2_3 : int_par_st_2_3_ops int_triadic_modes",
"unsigned_int_par_st_2_3 : int_par_st_2_3_ops tri_par_sti_mode",
"int_par_st_2_3_ops : AND",
"int_par_st_2_3_ops : OR",
"int_par_st_2_3_ops : XOR",
"shift_int_par_st_2_3 : shift_par_st_2_3_ops int_diadic_modes",
"shift_int_par_st_2_3 : shift_par_st_2_3_ops int_triadic_modes",
"shift_int_par_st_2_3 : shift_par_st_2_3_ops shift_tri_par_sti_mode",
"shift_par_st_2_3_ops : ASH",
"shift_par_st_2_3_ops : LSH",
"shift_int_par_st_mpy_2_3 : SUBI int_diadic_modes",
"shift_int_par_st_mpy_2_3 : SUBI int_triadic_modes",
"shift_int_par_st_mpy_2_3 : SUBI shift_tri_par_sti_mode",
"shift_int_par_st_mpy_2_3 : SUBI rev_par_mpyi_mode",
"int_par_st_mpy_2_3 : ADDI int_diadic_modes",
"int_par_st_mpy_2_3 : ADDI int_triadic_modes",
"int_par_st_mpy_2_3 : ADDI tri_par_sti_mode",
"int_par_st_mpy_2_3 : ADDI rev_par_mpyi_mode",
"shift_fp_par_st_mpy_2_3 : SUBF fp_diadic_modes",
"shift_fp_par_st_mpy_2_3 : SUBF fp_triadic_modes",
"shift_fp_par_st_mpy_2_3 : SUBF shift_tri_par_stf_mode",
"shift_fp_par_st_mpy_2_3 : SUBF rev_par_mpyf_mode",
"fp_par_st_mpy_2_3 : ADDF fp_diadic_modes",
"fp_par_st_mpy_2_3 : ADDF fp_triadic_modes",
"fp_par_st_mpy_2_3 : ADDF tri_par_stf_mode",
"fp_par_st_mpy_2_3 : ADDF rev_par_mpyf_mode",
"mpy : mpyf_3",
"mpy : mpyf_2_3",
"mpy : mpyi_3",
"mpy : mpyi_2_3",
"mpyf_3 : MPYF3 fp_triadic_modes",
"mpyf_3 : MPYF3 tri_par_stf_mode",
"mpyf_3 : MPYF3 par_mpyf_mode",
"mpyf_2_3 : MPYF fp_diadic_modes",
"mpyf_2_3 : MPYF fp_triadic_modes",
"mpyf_2_3 : MPYF tri_par_stf_mode",
"mpyf_2_3 : MPYF par_mpyf_mode",
"mpyi_3 : MPYI3 int_triadic_modes",
"mpyi_3 : MPYI3 tri_par_sti_mode",
"mpyi_3 : MPYI3 par_mpyi_mode",
"mpyi_2_3 : MPYI int_diadic_modes",
"mpyi_2_3 : MPYI int_triadic_modes",
"mpyi_2_3 : MPYI tri_par_sti_mode",
"mpyi_2_3 : MPYI par_mpyi_mode",
"rotates : rotate_ops reg",
"rotate_ops : ROL",
"rotate_ops : ROLC",
"rotate_ops : ROR",
"rotate_ops : RORC",
"reciprocals : recip_ops Dreg ',' Dreg",
"reciprocals : recip_ops direct ',' Dreg",
"reciprocals : recip_ops indirect ',' Dreg",
"recip_ops : RCPF",
"recip_ops : RSQRF",
"cmp : cmpi",
"cmp : cmpf",
"cmp : cmpi3",
"cmp : cmpf3",
"cmpi : cmpi_ops int_diadic_modes",
"cmpi : cmpi_ops reg ',' indirect",
"cmpi : cmpi_ops immediate ',' indirect",
"cmpi : cmpi_ops indirect ',' indirect",
"cmpi_ops : CMPI",
"cmpi_ops : TSTB",
"cmpf : CMPF fp_diadic_modes",
"cmpf : CMPF Dreg ',' indirect",
"cmpf : CMPF indirect ',' indirect",
"cmpi3 : cmpi3_ops reg ',' reg",
"cmpi3 : cmpi3_ops reg ',' indirect",
"cmpi3 : cmpi3_ops immediate ',' reg",
"cmpi3 : cmpi3_ops immediate ',' indirect",
"cmpi3 : cmpi3_ops indirect ',' reg",
"cmpi3 : cmpi3_ops indirect ',' indirect",
"cmpi3_ops : CMPI3",
"cmpi3_ops : TSTB3",
"cmpf3 : CMPF3 Dreg ',' Dreg",
"cmpf3 : CMPF3 Dreg ',' indirect",
"cmpf3 : CMPF3 indirect ',' Dreg",
"cmpf3 : CMPF3 indirect ',' indirect",
"fix : FIX Dreg",
"fix : FIX Dreg ',' reg",
"fix : FIX direct ',' reg",
"fix : FIX indirect ',' reg",
"fix : FIX fp_immediate ',' reg",
"fix : FIX dia_par_sti_mode",
"toieee : TOIEEE Dreg",
"toieee : TOIEEE Dreg ',' Dreg",
"toieee : TOIEEE direct ',' Dreg",
"toieee : TOIEEE indirect ',' Dreg",
"toieee : TOIEEE dia_par_stf_mode",
"frieee : FRIEEE direct ',' Dreg",
"frieee : FRIEEE indirect ',' Dreg",
"frieee : FRIEEE dia_par_stf_mode",
"instrpatch : constexpr",
"instrpatch : c40patches",
"c40patches : PATCHC40DATAMODULE1",
"c40patches : PATCHC40DATAMODULE2",
"c40patches : PATCHC40DATAMODULE3",
"c40patches : PATCHC40DATAMODULE4",
"c40patches : PATCHC40DATAMODULE5",
"c40patches : PATCHC40MASK8ADD",
"c40patches : PATCHC40MASK16ADD",
"c40patches : PATCHC40MASK24ADD",
};
#endif
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#ifndef YYSTACKSIZE
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 300
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
#define yystacksize YYSTACKSIZE
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#line 8644 "gasm.y"
		/* Appended C Code section				*/
/* ******************************************************************** */


/****************************************************************/
/* yyerror							*/
/*								*/
/* yyerror() is the predefined YACC error function, it simply	*/
/* prints the error, and returns. The macro YYERROR that it	*/
/* is usually used in conjunction with increments an error	*/
/* count 'yynerr' and attempts to restart parsing at the	*/
/* 'error' rule (placed in our 'statement' rule).		*/
/*								*/
/****************************************************************/

static void yyerror(char *s)
{
	Error(s);
}



/****************************************************************/
/* NewParseTreeItem						*/
/*								*/
/* Create a parse tree item and append it to the parse tree.	*/
/* Set what type of item it is and abort on memory failure.	*/
/*								*/
/* Note that this is the only place that interacts directly	*/
/* with the parse tree, apart from PATCHINSTR where the		*/
/* current tail item is	replaced by another item.		*/
/*								*/
/* Returns pointer to new item.					*/
/*								*/
/****************************************************************/

ParseTreeItem *NewParseTreeItem(int type)
{
	ParseTreeItem *pti = (ParseTreeItem *)malloc(sizeof(ParseTreeItem));

	if (pti == ((void *)  0) )
		Fatal("Out of memory whilst building parse tree");

	pti->what = type;
	pti->next = ((void *)  0) ;

	/* mnemonic's line number for accurate error reports in second pass */
	pti->linenum = StartLine;

	/* remember mnemonic's PC for use in pc relative label computation */

	pti->logicalPC = curPC / 4;	/* word address machine */

	/* add item into parse tree */
	LastParseTreeItem->next = pti;
	LastParseTreeItem = pti;

	return pti;
}


/****************************************************************/
/* NewConstItem							*/
/*								*/
/* Create a new ConstList item and set its type.		*/
/* Abort on any memory failure.					*/
/*								*/
/* Returns pointer to new item.					*/
/*								*/
/****************************************************************/

ConstList *NewConstItem(int type)
{
	ConstList *cl = (ConstList *)malloc(sizeof(ConstList));

	if (cl == ((void *)  0) )
		Fatal("Out of memory whilst building new constant");

	cl->what = type;
	cl->next = ((void *)  0) ;


	return cl;
}


/****************************************************************/
/* NewFloatConstItem						*/
/*								*/
/* Create a new FloatConstList item.				*/
/* Abort on any memory failure.					*/
/*								*/
/* Returns pointer to new item.					*/
/*								*/
/****************************************************************/

FloatList *NewFloatConstItem(Dble d)
{
	FloatList *fl = (FloatList *)malloc(sizeof(FloatList));

	if (fl == ((void *)  0) )
		Fatal("Out of memory whilst building new float constant");

	fl->next = ((void *)  0) ;
	fl->value = d;


	return fl;
}


/****************************************************************/
/* NewPatch							*/
/*								*/
/* Create a new patch item and set its patch type.		*/
/* Abort on any memory failure.					*/
/*								*/
/* Returns pointer to new item.					*/
/*								*/
/****************************************************************/

Patch *NewPatch(int type)
{
	Patch *p = (Patch *)malloc(sizeof(Patch));

	if (p == ((void *)  0) )
		Fatal("Out of memory whilst defining patch");

	p->what = type;


	return p;
}


/****************************************************************/
/* NewExpr							*/
/*								*/
/* Create a new expression item, setting its type, operator and */
/* left and right expressions.					*/
/* Aborts on any memory failure.				*/
/*								*/
/* Returns pointer to new item.					*/
/*								*/
/****************************************************************/

Expression *NewExpr(Expression *le, int op, Expression *re)
{
	Expression *e = (Expression *)malloc(sizeof(Expression));

	if (e == ((void *)  0) )
		Fatal("Out of memory defining expression");

	e->what = 	1 ;

	e->type.expr.Operator = op;
	e->type.expr.left  = le;
	e->type.expr.right = re;


	return e;
}


/****************************************************************/
/* NewExprNum							*/
/*								*/
/* Create a new number containing expression item.		*/
/* Aborts on any memory failure.				*/
/*								*/
/* Returns pointer to new item.					*/
/*								*/
/****************************************************************/

Expression *NewExprNum(int num)
{
	Expression *e = (Expression *)malloc(sizeof(Expression));

	if (e == ((void *)  0) )
		Fatal("Out of memory defining expression");

	e->what = 	2 ;

	e->type.number = num;


	return e;
}


/****************************************************************/
/* NewExprSymbRef						*/
/*								*/
/* Create a new symbol reference containing expression item.	*/
/* Aborts on any memory failure.				*/
/*								*/
/* Returns pointer to new item.					*/
/*								*/
/****************************************************************/

Expression *NewExprSymbRef(char *name)
{
	Expression *e = (Expression *)malloc(sizeof(Expression));

	if (e == ((void *)  0) )
		Fatal("Out of memory defining expression");

	e->what = 	4		;

	e->type.name = name;


	return e;
}


/* *********************************************************************** */


/* end of gasm.ypp */
#line 11818 "y.tab.c"
#define YYABORT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, reading %d (%s)\n", yystate,
                    yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: state %d, shifting to state %d\n",
                    yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: state %d, error recovery shifting\
 to state %d\n", *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: error recovery discarding state %d\n",
                            *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, error recovery discards token %d (%s)\n",
                    yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("yydebug: state %d, reducing by rule %d (%s)\n",
                yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 2:
#line 5174 "gasm.y"
{StartLine = CurLine;}
break;
case 4:
#line 5175 "gasm.y"
{StartLine = CurLine;}
break;
case 6:
#line 5177 "gasm.y"
{
			yyclearin;	/* clear read ahead tokens */
			yyerrok;	/* allow syntax errors to restart */
			ClearInput();	/* clear all input up to next \n */
		}
break;
case 7:
#line 5187 "gasm.y"
{
			/* if symbol already defined, raise an error */
			if (FindSymb(yyvsp[0].str )) {
				char	Err[128];

				sprintf(Err, "redefinition of label \"%s\"", yyvsp[0].str );
				Error(Err);
				/* and continue parsing... */
			}
			else {
				/* Add label to symbol table */

				/* Naff word address machine */
				Symbol *s = NewSymb(yyvsp[0].str , 0	, curPC / 4);


				/* add symbol table ref to parse tree */
				/* this will output an object code label */
				ParseTreeItem *pti = NewParseTreeItem(LABEL);

				pti->type.symb = s;


				/* check alignment of label */
				if (curPC % 4 != 0) {
					char Err[128];

					sprintf(Err, "label \"%s\" is not aligned to a  word address", yyvsp[0].str );
					Error(Err);
				}

			}
		}
break;
case 33:
#line 5264 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(BYTE);

			/* insert ref to codeconstlist items */
			pti->type.clist = yyvsp[0].clist ;
			curPC += list_size_count * sizeof(char);
		}
break;
case 34:
#line 5274 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(SHORT);

			pti->type.clist = yyvsp[0].clist ;
			curPC += list_size_count * (2 * sizeof(char));
		}
break;
case 35:
#line 5283 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(WORD);

			pti->type.clist = yyvsp[0].clist ;
			curPC += list_size_count * sizeof(int);
		}
break;
case 36:
#line 5292 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(FLOATY);

			pti->type.flist = yyvsp[0].flist ;
			curPC += list_size_count * 4; /* 32 bit float */
		}
break;
case 37:
#line 5301 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(DOUBLE);

			pti->type.flist = yyvsp[0].flist ;
			curPC += list_size_count * 8; /* 64 bit float */
		}
break;
case 38:
#line 5311 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(C40FLOAT);

			pti->type.flist = yyvsp[0].flist ;
			curPC += list_size_count * 4; /* 32 bit float */
		}
break;
case 39:
#line 5324 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(BYTE);
			int remain = yyvsp[-2].num  - list_size_count;

			pti->type.clist = yyvsp[0].clist ;

			/* pad remaining block space with zeros by inserting */
			/* a bogus SPACE pseudo op. */
			if (remain < 0) {
				Error("blockbyte: block size exceeded by data supplied");
			}
			else if (remain > 0) {
				pti = NewParseTreeItem(SPACE);
				pti->type.num = remain;
			}
			/* else if (remain==0) no need for padding */

			curPC += yyvsp[-2].num  * sizeof(char);
		}
break;
case 40:
#line 5346 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(SHORT);
			int remain = yyvsp[-2].num  - list_size_count;

			pti->type.clist = yyvsp[0].clist ;

			/* pad remaining block space with zeros by inserting */
			/* a bogus SPACE pseudo op. */
			if (remain < 0) {
				Error("blockshort: block size exceeded by data supplied");
			}
			else if (remain > 0) {
				pti = NewParseTreeItem(SPACE);
				pti->type.num = remain * (2 * sizeof(char));
			}
			/* else if (remain==0) no need for padding */

			curPC += yyvsp[-2].num  * (2 * sizeof(char));
		}
break;
case 41:
#line 5368 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(WORD);
			int remain = yyvsp[-2].num  - list_size_count;

			pti->type.clist = yyvsp[0].clist ;

			/* pad remaining block space with zeros by inserting */
			/* a bogus SPACE pseudo op. */
			if (remain < 0) {
				Error("blockword: block size exceeded by data supplied");
			}
			else if (remain > 0) {
				pti = NewParseTreeItem(SPACE);
				pti->type.num = remain * sizeof(int);
			}
			/* else if (remain==0) no need for padding */

			curPC += yyvsp[-2].num  * sizeof(int);
		}
break;
case 42:
#line 5393 "gasm.y"
{
			/* add space directive to parse tree */
			ParseTreeItem *pti = NewParseTreeItem(SPACE);

			pti->type.num = yyvsp[0].num ;

			curPC += yyvsp[0].num ;
		}
break;
case 43:
#line 5407 "gasm.y"
{
			ParseTreeItem *pti;
			int pad;

			/* if alignment not defined default to 4 */
			pad = (((( curPC ) + (  (yyvsp[0].num ) ? yyvsp[0].num  : 4 ) - 1) & ~((  (yyvsp[0].num ) ? yyvsp[0].num  : 4 ) - 1)) -  curPC ) ;

			if (pad != 0) {
				/* if we need to pad out current address so */
				/* that the next opcode is aligned to the */
				/* specified byte boundary, then add a SPACE */
				/* directive of the relevant size */

				pti = NewParseTreeItem(SPACE);
				pti->type.num = pad;

				curPC += pad;
			}
		}
break;
case 44:
#line 5434 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(DATA);

			pti->type.datacommon.name = yyvsp[-2].str ;
			pti->type.datacommon.expr = yyvsp[0].expr ;


		}
break;
case 45:
#line 5446 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(COMMON);

			pti->type.datacommon.name = yyvsp[-2].str ;
			pti->type.datacommon.expr = yyvsp[0].expr ;


		}
break;
case 46:
#line 5458 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(CODETABLE);

			pti->type.nlist = yyvsp[0].nlist ;


		}
break;
case 47:
#line 5470 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(EXPORT);

			pti->type.nlist = yyvsp[0].nlist ;


		}
break;
case 48:
#line 5482 "gasm.y"
{
			/* force a linker global directive to be emited */
			ParseTreeItem *pti = NewParseTreeItem(IMPORT);
			NameList *n = yyvsp[0].nlist ;

			pti->type.nlist = yyvsp[0].nlist ;

			/* add imported label to the symbol table */
			while ( n != ((void *)  0) ) {
				NewSymb(n->name, 4	, 0);
				n = n->next;
			}

		}
break;
case 49:
#line 5504 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(MODULE);

			/* if no module number expression, default to -1 */
			pti->type.expr = (yyvsp[0].expr ) ? yyvsp[0].expr  : NewExprNum(-1);


		}
break;
case 50:
#line 5516 "gasm.y"
{
			NewParseTreeItem(INIT);

			/* init parse item needs to remember no data */


			curPC += sizeof(int);
		}
break;
case 51:
#line 5528 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(REF);

			pti->type.nlist = yyvsp[0].nlist ;


		}
break;
case 52:
#line 5539 "gasm.y"
{
			Instruction *instr;

			if (LastParseTreeItem->what == INSTRUCTION) {
				/* Change the parse tree item that the */
				/* processor specific parser appends to the */
				/* parse tree into a PATCHINSTR type parse */
				/* tree item */

				/* remember instruction struct pointer */
				instr = LastParseTreeItem->type.instr;

				/* convert parse tree item into a PATCHINSTR */
				LastParseTreeItem->what = PATCHINSTR;
				LastParseTreeItem->type.patchinstr.mcpatch = yyvsp[-5].expr ;
				LastParseTreeItem->type.patchinstr.patch = yyvsp[-3].patch ;

				/* re-insert instruction struct pointer */
				LastParseTreeItem->type.patchinstr.instr =instr;

			}
			else
				Error("patchinstr failed due to instruction syntax error");
		}
break;
case 53:
#line 5570 "gasm.y"
{
			Error(yyvsp[0].str );
		}
break;
case 54:
#line 5576 "gasm.y"
{
			Warn(yyvsp[0].str );
		}
break;
case 55:
#line 5582 "gasm.y"
{
			Note(yyvsp[0].str );
		}
break;
case 56:
#line 5598 "gasm.y"
{
			yyval.patch  = NewPatch(MODSIZE);			/* 0x0e */

		}
break;
case 57:
#line 5603 "gasm.y"
{
			yyval.patch  = NewPatch(MODNUM);			/* 0x012 */

		}
break;
case 58:
#line 5608 "gasm.y"
{
			yyval.patch  = NewPatch(DATASYMB);		/* 0x010 */
			yyval.patch ->type.name = yyvsp[-1].str ;

		}
break;
case 59:
#line 5614 "gasm.y"
{
			yyval.patch  = NewPatch(CODESYMB);		/* 0x01d */
			yyval.patch ->type.name = yyvsp[-1].str ;

		}
break;
case 60:
#line 5620 "gasm.y"
{
			yyval.patch  = NewPatch(DATAMODULE);		/* 0x011 */
			yyval.patch ->type.name = yyvsp[-1].str ;

		}
break;
case 61:
#line 5626 "gasm.y"
{
			/* @@@ give warning if label is not in this module */
			/* @@@ during second pass */
			yyval.patch  = NewPatch(LABELREF);		/* 0x01f */
			yyval.patch ->type.name = yyvsp[-1].str ;


		}
break;
case 62:
#line 5636 "gasm.y"
{
			yyval.patch  = NewPatch(CODESTUB);		/* 0x028 */
			yyval.patch ->type.name = yyvsp[-1].str ;


		}
break;
case 63:
#line 5644 "gasm.y"
{
			yyval.patch  = NewPatch(ADDRSTUB);		/* 0x029 */
			yyval.patch ->type.name = yyvsp[-1].str ;


		}
break;
case 64:
#line 5653 "gasm.y"
{
			yyval.patch  = NewPatch(BYTESWAP);		/* 0x01e */
			yyval.patch ->type.patch = yyvsp[-1].patch ;


		}
break;
case 65:
#line 5660 "gasm.y"
{
			yyval.patch  = NewPatch(SHIFT);			/* 0x014 */
			yyval.patch ->type.shift.expr = yyvsp[-3].expr ;
			yyval.patch ->type.shift.patch = yyvsp[-1].patch ;


		}
break;
case 66:
#line 5668 "gasm.y"
{
			yyval.patch  = NewPatch(P_ADD);			/* 0x013 */
			yyval.patch ->type.shift.expr = yyvsp[-3].expr ;
			yyval.patch ->type.shift.patch = yyvsp[-1].patch ;


		}
break;
case 67:
#line 5676 "gasm.y"
{
			yyval.patch  = NewPatch(P_OR);			/* 0x01f */
			yyval.patch ->type.shift.expr = yyvsp[-3].expr ;
			yyval.patch ->type.shift.patch = yyvsp[-1].patch ;


		}
break;
case 68:
#line 5690 "gasm.y"
{
			/* first or only item in list */
			NameList *n = (NameList *)malloc(sizeof(NameList));

			if (n == ((void *)  0) )
				Fatal("Out of memory whilst building name list");

			n->next = ((void *)  0) ;
			n->name = yyvsp[0].str ;
			yyval.nlist  = n;
		}
break;
case 69:
#line 5702 "gasm.y"
{
			/* list of names */
			NameList *n = yyvsp[-2].nlist ;
			NameList *nn = (NameList *)malloc(sizeof(NameList));

			if (nn == ((void *)  0) )
				Fatal("Out of memory whilst building name list");

			/* insert name list item - order is not important */
			/* so dont search to end of list */
			nn->next = n->next;
			n->next = nn;
			nn->name = yyvsp[0].str ;

			yyval.nlist  = n;	/* return start of namelist */
		}
break;
case 70:
#line 5728 "gasm.y"
{ list_size_count = 0; }
break;
case 71:
#line 5730 "gasm.y"
{
			yyval.clist  = yyvsp[0].clist ;	/* first or only item in list */
		}
break;
case 72:
#line 5734 "gasm.y"
{
			ConstList *n = yyvsp[-2].clist ;	/* start of list */

			/* search for end of list */
			while (n->next != ((void *)  0) )
				n = n->next;

			n->next = yyvsp[0].clist ;	/* append item to end of list */	

			yyval.clist  = yyvsp[-2].clist ;	/* returns start of constlist */
		}
break;
case 73:
#line 5748 "gasm.y"
{ list_size_count = 0; }
break;
case 74:
#line 5750 "gasm.y"
{
			/* first or only item in list */
			yyval.flist  = NewFloatConstItem(yyvsp[0].flt );
			list_size_count++;
		}
break;
case 75:
#line 5756 "gasm.y"
{
			FloatList *fl = yyvsp[-2].flist ;	/* start of list */

			/* search for end of list */
			while (fl->next != ((void *)  0) )
				fl = fl->next;
			/* append item to end of list */
			fl->next = NewFloatConstItem(yyvsp[0].flt );
			list_size_count++;

			yyval.flist  = yyvsp[-2].flist ;	/* returns start of floatconstlist */
		}
break;
case 76:
#line 5774 "gasm.y"
{
			/* return an Expression constlist item */
			yyval.clist  = NewConstItem(	1 );

			yyval.clist ->type.expr = yyvsp[0].expr ;
			list_size_count++;
		}
break;
case 77:
#line 5782 "gasm.y"
{
			/* return a patch constlist item */
			yyval.clist  = NewConstItem(	3 );

			yyval.clist ->type.patch = yyvsp[0].patch ;
			list_size_count++;
		}
break;
case 78:
#line 5790 "gasm.y"
{
			/* return a string constlist item */
			yyval.clist  = NewConstItem(	4		);

			yyval.clist ->type.str = yyvsp[0].str ;
			/* increment the list size count by the number of */
			/* chars in the string - 1 counted by default already */

			list_size_count += strlen(yyvsp[0].str );

		}
break;
case 79:
#line 5820 "gasm.y"
{	yyval.num  = yyvsp[-1].num ; }
break;
case 80:
#line 5822 "gasm.y"
{	yyval.num  = yyvsp[-2].num  + yyvsp[0].num ; }
break;
case 81:
#line 5824 "gasm.y"
{	yyval.num  = yyvsp[-2].num  - yyvsp[0].num ; }
break;
case 82:
#line 5826 "gasm.y"
{	yyval.num  = yyvsp[-2].num  * yyvsp[0].num ; }
break;
case 83:
#line 5828 "gasm.y"
{	yyval.num  = yyvsp[-2].num  / yyvsp[0].num ; }
break;
case 84:
#line 5830 "gasm.y"
{	yyval.num  = yyvsp[-2].num  % yyvsp[0].num ; }
break;
case 85:
#line 5832 "gasm.y"
{	yyval.num  = yyvsp[-2].num  & yyvsp[0].num ; }
break;
case 86:
#line 5834 "gasm.y"
{	yyval.num  = yyvsp[-2].num  | yyvsp[0].num ; }
break;
case 87:
#line 5836 "gasm.y"
{	yyval.num  = yyvsp[-2].num  ^ yyvsp[0].num ; }
break;
case 88:
#line 5838 "gasm.y"
{	yyval.num  = yyvsp[-2].num  << yyvsp[0].num ; }
break;
case 89:
#line 5840 "gasm.y"
{	yyval.num  = yyvsp[-2].num  >> yyvsp[0].num ; }
break;
case 90:
#line 5842 "gasm.y"
{	yyval.num  = ~ (yyvsp[0].num ); }
break;
case 91:
#line 5844 "gasm.y"
{	yyval.num  = - (yyvsp[0].num ); }
break;
case 94:
#line 5860 "gasm.y"
{	yyval.flt  = yyvsp[-1].flt ; }
break;
case 95:
#line 5863 "gasm.y"
{	if (!flt_add (&yyval.flt , &yyvsp[-2].flt , &yyvsp[0].flt ))
				Warn ("overflow: floating point addition") ; }
break;
case 96:
#line 5866 "gasm.y"
{	if (!flt_subtract (&yyval.flt , &yyvsp[-2].flt , &yyvsp[0].flt ))
				Warn ("overflow: floating point subtraction") ; }
break;
case 97:
#line 5869 "gasm.y"
{	if (!flt_multiply (&yyval.flt , &yyvsp[-2].flt , &yyvsp[0].flt ))
				Warn ("overflow: floating point multiplication") ; }
break;
case 98:
#line 5872 "gasm.y"
{	if (!flt_divide (&yyval.flt , &yyvsp[-2].flt , &yyvsp[0].flt ))
				Warn ("overflow: floating point division") ; }
break;
case 99:
#line 5875 "gasm.y"
{	(void) flt_negate (&yyval.flt , &yyvsp[0].flt ); }
break;
case 100:
#line 5879 "gasm.y"
{ (void) flt_itod (&yyval.flt , yyvsp[0].num );}
break;
case 102:
#line 5888 "gasm.y"
{

			yyval.num  = 0;
		}
break;
case 104:
#line 5904 "gasm.y"
{	yyval.expr  = yyvsp[-1].expr ; }
break;
case 105:
#line 5906 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '+', yyvsp[0].expr ); }
break;
case 106:
#line 5908 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '-', yyvsp[0].expr ); }
break;
case 107:
#line 5910 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '*', yyvsp[0].expr ); }
break;
case 108:
#line 5912 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '/', yyvsp[0].expr ); }
break;
case 109:
#line 5914 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '%', yyvsp[0].expr ); }
break;
case 110:
#line 5916 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '&', yyvsp[0].expr ); }
break;
case 111:
#line 5918 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '|', yyvsp[0].expr ); }
break;
case 112:
#line 5920 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '^', yyvsp[0].expr ); }
break;
case 113:
#line 5922 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , LSHIFT, yyvsp[0].expr ); }
break;
case 114:
#line 5924 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , RSHIFT, yyvsp[0].expr ); }
break;
case 115:
#line 5926 "gasm.y"
{	yyval.expr  = NewExpr(((void *)  0) , '~', yyvsp[0].expr ); }
break;
case 116:
#line 5928 "gasm.y"
{	yyval.expr  = NewExpr(((void *)  0) , '-', yyvsp[0].expr ); }
break;
case 117:
#line 5931 "gasm.y"
{	yyval.expr  = NewExprNum(yyvsp[0].num );}
break;
case 118:
#line 5934 "gasm.y"
{	yyval.expr  = NewExprNum(yyvsp[0].num );}
break;
case 119:
#line 5937 "gasm.y"
{	yyval.expr  = NewExprSymbRef(yyvsp[0].str );}
break;
case 120:
#line 5943 "gasm.y"
{
			/* code executed if no optional constexpr in source */


			yyval.expr  = ((void *)  0) ;
		}
break;
case 154:
#line 6002 "gasm.y"
{	Error("Expecting a register");	}
break;
case 167:
#line 6008 "gasm.y"
{	Error("Expecting a register (R0-R11)");	}
break;
case 170:
#line 6014 "gasm.y"
{	Error("Expecting register R0 or R1");	}
break;
case 173:
#line 6020 "gasm.y"
{	Error("Expecting register R2 or R3");	}
break;
case 182:
#line 6026 "gasm.y"
{	Error("Expecting a register R0-R7");	}
break;
case 204:
#line 6038 "gasm.y"
{	Error("Unknown address register");	}
break;
case 207:
#line 6044 "gasm.y"
{	Error("Unknown expansion file register");	}
break;
case 208:
#line 6052 "gasm.y"
{
			if (CurInstr->combine) {
				/* default to diadic direct addr. */
				CurInstr->combine2 = 	5 ;
				yyval.expr  = CurInstr->optexpr2 = yyvsp[0].expr ;
			}
			else {
				/* default to diadic direct addr. */
				CurInstr->combine = 	5 ;
				yyval.expr  = CurInstr->optexpr = yyvsp[0].expr ;
			}
		}
break;
case 209:
#line 6075 "gasm.y"
{
			if (CurInstr->combine) {
				/* default to diadic 16 bit immediate */
				CurInstr->combine2 = 	1 ;
				yyval.expr  = CurInstr->optexpr2 = yyvsp[0].expr ;
			}
			else {
				/* default to diadic 16 bit immediate */
				CurInstr->combine = 	1 ;
				yyval.expr  = CurInstr->optexpr = yyvsp[0].expr ;
			}
		}
break;
case 210:
#line 6091 "gasm.y"
{
			yyval.num  = (int) IEEE_64ToC40_16(yyvsp[0].flt );
		}
break;
case 211:
#line 6121 "gasm.y"
{	yyval.num  = 	(( 0x18  ) << 3)  | (( yyvsp[0].num  ) - 	0x08 ) ;	}
break;
case 212:
#line 6123 "gasm.y"
{	yyval.num  = 	(( yyvsp[-2].num  | yyvsp[0].num  ) << 3)  | (( yyvsp[-1].num  ) - 	0x08 ) ;	}
break;
case 213:
#line 6125 "gasm.y"
{	yyval.num  = 	(( yyvsp[0].num  ) << 3)  | (( yyvsp[-1].num  ) - 	0x08 ) ;		}
break;
case 214:
#line 6127 "gasm.y"
{
			Error("format of indirection is incorrect");
		}
break;
case 215:
#line 6135 "gasm.y"
{	yyval.num  = 0x0 ;		}
break;
case 216:
#line 6137 "gasm.y"
{	yyval.num  = 0x1 ;		}
break;
case 217:
#line 6139 "gasm.y"
{	yyval.num  = 0x2 ;		}
break;
case 218:
#line 6141 "gasm.y"
{	yyval.num  = 0x3 ;		}
break;
case 219:
#line 6146 "gasm.y"
{	yyval.num  = 0x19 ;		}
break;
case 220:
#line 6148 "gasm.y"
{	yyval.num  = yyvsp[0].num  | 0x4 ;	}
break;
case 221:
#line 6150 "gasm.y"
{	yyval.num  = yyvsp[0].num  | 0x5 ;	}
break;
case 222:
#line 6152 "gasm.y"
{	yyval.num  = yyvsp[-1].num  | 0x6 ;	}
break;
case 223:
#line 6154 "gasm.y"
{	yyval.num  = yyvsp[-1].num  | 0x7 ;	}
break;
case 224:
#line 6165 "gasm.y"
{
			/* empty - so assume displacement of 1 */
			/* '(' constexpr ')' equiv. */
			if (CurInstr->combine) {
				/* fake expr of 1 */
				CurInstr->optexpr2 = NewExprNum(1);
				CurInstr->combine2 = 	4 ;
			}
			else {
				/* fake expr of 1 */
				CurInstr->optexpr = NewExprNum(1);
				CurInstr->combine = 	4 ;
			}
			yyval.num  = 0; /* $$ = MOD_DISP(B_00); - defaults to this */
		}
break;
case 225:
#line 6181 "gasm.y"
{
			if (CurInstr->combine) {
				CurInstr->optexpr2 = yyvsp[-1].expr ;
				CurInstr->combine2 = 	4 ;
			}
			else {
				CurInstr->optexpr = yyvsp[-1].expr ;
				CurInstr->combine = 	4 ;
			}
			yyval.num  = 0; /* $$ = MOD_DISP(B_00); - defaults to this */
		}
break;
case 226:
#line 6193 "gasm.y"
{	yyval.num  = (( 0x1  ) << 3) ;	}
break;
case 227:
#line 6195 "gasm.y"
{	yyval.num  = (( 0x2  ) << 3) ;	}
break;
case 228:
#line 6200 "gasm.y"
{
			if (!(strcmp("BIT", yyvsp[0].str ) == 0 || strcmp("bit", yyvsp[0].str ) == 0)) {
				char err[128];
				
				strcpy(err, "'");
				strcat(err, yyvsp[0].str );
				strcat(err, "' is not a valid bitreversal addressing specifier - use 'bit'");
				Error(err);
			}
		}
break;
case 229:
#line 6217 "gasm.y"
{
			/* default to 16 bit cond branch */
			CurInstr->combine = 17	;
			yyval.expr  = CurInstr->optexpr = yyvsp[0].expr ;
		}
break;
case 230:
#line 6223 "gasm.y"
{
			/* default to 16 bit cond. branch */
			CurInstr->combine = 17	;
			yyval.expr  = CurInstr->optexpr = yyvsp[0].expr ;
		}
break;
case 231:
#line 6244 "gasm.y"
{	yyval.num  = 		( 0x0   << 21)					 | (( yyvsp[0].num  ) << 16)		 | ( yyvsp[-2].num  )			; }
break;
case 232:
#line 6246 "gasm.y"
{	yyval.num  = 		( 0x2   << 21)					 | (( yyvsp[0].num  ) << 16)		 | (( yyvsp[-2].num  ) << 8)		; }
break;
case 233:
#line 6248 "gasm.y"
{	yyval.num  = 		( 0x1   << 21)					 | (( yyvsp[0].num  ) << 16)		; }
break;
case 234:
#line 6250 "gasm.y"
{	yyval.num  = 		( 0x3   << 21)					 | (( yyvsp[0].num  ) << 16)		; }
break;
case 235:
#line 6252 "gasm.y"
{	Error("illegal addressing mode for an integer load");	}
break;
case 236:
#line 6261 "gasm.y"
{	yyval.num  = 		( 0x1   << 21)					 | (( yyvsp[-2].num  ) << 16)		; }
break;
case 237:
#line 6263 "gasm.y"
{	yyval.num  = 		( 0x2   << 21)					 | (( yyvsp[-2].num  ) << 16)		 | (( yyvsp[0].num  ) << 8)		; }
break;
case 238:
#line 6265 "gasm.y"
{
			Error("illegal addressing mode for store instruction");
		}
break;
case 239:
#line 6278 "gasm.y"
{	yyval.num  = 		( 0x0   << 21)					 | (( yyvsp[0].num  ) << 16)		 | ( yyvsp[0].num  )			; }
break;
case 240:
#line 6286 "gasm.y"
{	yyval.num  = 		( 0x0   << 21)					 | (( yyvsp[0].num  ) << 16)		 | ( yyvsp[0].num  )			; }
break;
case 241:
#line 6293 "gasm.y"
{	yyval.num  = 		( 0x0   << 21)					 | (( yyvsp[0].num  ) << 16)		 | ( yyvsp[-2].num  )			; }
break;
case 242:
#line 6295 "gasm.y"
{	yyval.num  = 		( 0x1   << 21)					 | (( yyvsp[0].num  ) << 16)		; }
break;
case 243:
#line 6297 "gasm.y"
{	yyval.num  = 		( 0x2   << 21)					 | (( yyvsp[0].num  ) << 16)		 | (( yyvsp[-2].num  ) << 8)		; }
break;
case 244:
#line 6299 "gasm.y"
{	yyval.num  = 		( 0x3   << 21)					 | (( yyvsp[0].num  ) << 16)		; }
break;
case 245:
#line 6306 "gasm.y"
{	yyval.num  = 		( 0x0   << 21)					 | (( yyvsp[0].num  ) << 16)		 | ( yyvsp[-2].num  )			; }
break;
case 246:
#line 6308 "gasm.y"
{	yyval.num  = 		( 0x1   << 21)					 | (( yyvsp[0].num  ) << 16)		; }
break;
case 247:
#line 6310 "gasm.y"
{	yyval.num  = 		( 0x2   << 21)					 | (( yyvsp[0].num  ) << 16)		 | (( yyvsp[-2].num  ) << 8)		; }
break;
case 248:
#line 6312 "gasm.y"
{	yyval.num  = 		( 0x3   << 21)					 | yyvsp[-2].num  | (( yyvsp[0].num  ) << 16)		; }
break;
case 249:
#line 6319 "gasm.y"
{	yyval.num  = 		( 0x0   << 21)					 | (( yyvsp[0].num  ) << 16)		 | ( yyvsp[-2].num  )			; }
break;
case 250:
#line 6321 "gasm.y"
{	yyval.num  = 		( 0x1   << 21)					 | (( yyvsp[0].num  ) << 16)		; }
break;
case 251:
#line 6323 "gasm.y"
{	yyval.num  = 		( 0x2   << 21)					 | (( yyvsp[0].num  ) << 16)		 | (( yyvsp[-2].num  ) << 8)		; }
break;
case 252:
#line 6325 "gasm.y"
{	yyval.num  = 		( 0x3   << 21)					 | (( yyvsp[0].num  ) << 16)		; }
break;
case 253:
#line 6334 "gasm.y"
{
			yyval.num  = (0			 | 	(( 0x0  ) << 21)		)  | ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 16)		;
		}
break;
case 254:
#line 6338 "gasm.y"
{
			yyval.num  = (0			 | 	(( 0x1  ) << 21)		)  | ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 16)		;
			CurInstr->combine = 	8	;
		}
break;
case 255:
#line 6346 "gasm.y"
{
			yyval.num  = ((1 << 28)		 | 	(( 0x0  ) << 21)		)  | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 16)		;
			CurInstr->combine = 	11 ;
		}
break;
case 256:
#line 6352 "gasm.y"
{
			yyval.num  = ((1 << 28)		 | 	(( 0x2  ) << 21)		)  | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 16)		;
			CurInstr->combine = 	11 ;
			if (!((( yyvsp[-2].num  ) & 0xf8 ) == 0x0 ) ) {
				Error("Only *+ARn format indirections allowed for type 2 triadic instructions");
			}
			CurInstr->combine2 = 	10	;
		}
break;
case 257:
#line 6369 "gasm.y"
{
			if (((( yyvsp[-4].num  ) & 0xf8 ) == 0x0 ) ) {
				yyval.num  = ((1 << 28)		 | 	(( 0x1  ) << 21)		)  | ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 16)		;
				CurInstr->combine = 	9	;
			}
			else {
				yyval.num  = (0			 | 	(( 0x2  ) << 21)		)  | ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 16)		;
				CurInstr->combine = 	7	;
			}
		}
break;
case 258:
#line 6380 "gasm.y"
{
			if (((( yyvsp[-4].num  ) & 0xf8 ) == 0x0 )  && ((( yyvsp[-2].num  ) & 0xf8 ) == 0x0 ) ) {
				yyval.num  = ((1 << 28)		 | 	(( 0x3  ) << 21)		)  | ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 16)		;
				CurInstr->combine = 	9	;
				CurInstr->combine2 = 	10	;
			}
			else {
				yyval.num  = (0			 | 	(( 0x3  ) << 21)		)  | ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 16)		;
				CurInstr->combine = 	7	;
				CurInstr->combine2 = 	8	;
			}
		}
break;
case 259:
#line 6401 "gasm.y"
{
			yyval.num  = (0			 | 	(( 0x0  ) << 21)		)  | ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 16)		;
		}
break;
case 260:
#line 6405 "gasm.y"
{
			yyval.num  = (0			 | 	(( 0x1  ) << 21)		)  | ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 16)		;
			CurInstr->combine = 	8	;
		}
break;
case 261:
#line 6422 "gasm.y"
{
			if (((( yyvsp[-4].num  ) & 0xf8 ) == 0x0 ) ) {
				yyval.num  = ((1 << 28)		 | 	(( 0x1  ) << 21)		)  | ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 16)		;
				CurInstr->combine = 	9	;
			}
			else {
				yyval.num  = (0			 | 	(( 0x2  ) << 21)		)  | ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 16)		;
				CurInstr->combine = 	7	;
			}
		}
break;
case 262:
#line 6433 "gasm.y"
{
			if (((( yyvsp[-4].num  ) & 0xf8 ) == 0x0 )  && ((( yyvsp[-2].num  ) & 0xf8 ) == 0x0 ) ) {
				yyval.num  = ((1 << 28)		 | 	(( 0x3  ) << 21)		)  | ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 16)		;
				CurInstr->combine = 	9	;
				CurInstr->combine2 = 	10	;
			}
			else {
				yyval.num  = (0			 | 	(( 0x3  ) << 21)		)  | ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 16)		;
				CurInstr->combine = 	7	;
				CurInstr->combine2 = 	8	;
			}
		}
break;
case 263:
#line 6455 "gasm.y"
{
			if ( yyvsp[-7].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-5].num   > 7) { Error("Must be register R0-R7"); } ;
			yyval.num  = ( yyvsp[-9].num  )		 | (( yyvsp[-7].num  ) << 19)	 | (( yyvsp[-5].num  ) << 22)	 | (( yyvsp[-2].num  ) << 16)	 				| (( yyvsp[0].num  ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
break;
case 264:
#line 6462 "gasm.y"
{
			/* assume src2 and dst1 regs are the same */
			/* Optional syntax #13 */

			if ( yyvsp[-5].num   > 7) { Error("Must be register R0-R7"); } ;
			yyval.num  = ( yyvsp[-7].num  )		 | (( yyvsp[-5].num  ) << 19)	 | (( yyvsp[-5].num  ) << 22)	 | (( yyvsp[-2].num  ) << 16)	 				| (( yyvsp[0].num  ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
break;
case 265:
#line 6477 "gasm.y"
{
			if ( yyvsp[-5].num   > 7) { Error("Must be register R0-R7"); } ;
			yyval.num  = ( yyvsp[-7].num  )		 | (( yyvsp[-5].num  ) << 22)	 | (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
break;
case 266:
#line 6491 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-5].num   > 7) { Error("Must be register R0-R7"); } ;
			yyval.num  = (( yyvsp[-9].num  ) << 19)	 | ( yyvsp[-7].num  )		 | (( yyvsp[-5].num  ) << 22)	 | (( yyvsp[-2].num  ) << 16)	 				| (( yyvsp[0].num  ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
break;
case 267:
#line 6507 "gasm.y"
{
			if ( yyvsp[-7].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-5].num   > 7) { Error("Must be register R0-R7"); } ;
			yyval.num  = ( yyvsp[-9].num  )		 | (( yyvsp[-7].num  ) << 19)	 | (( yyvsp[-5].num  ) << 22)	 | (( yyvsp[-2].num  ) << 16)	 				| (( yyvsp[0].num  ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
break;
case 268:
#line 6514 "gasm.y"
{
			/* assume src2 and dst1 regs are the same */
			/* Optional syntax #13 */

			if ( yyvsp[-5].num   > 7) { Error("Must be register R0-R7"); } ;
			yyval.num  = ( yyvsp[-7].num  )		 | (( yyvsp[-5].num  ) << 19)	 | (( yyvsp[-5].num  ) << 22)	 | (( yyvsp[-2].num  ) << 16)	 				| (( yyvsp[0].num  ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
break;
case 269:
#line 6529 "gasm.y"
{
			if ( yyvsp[-5].num   > 7) { Error("Must be register R0-R7"); } ;
			yyval.num  = ( yyvsp[-7].num  )		 | (( yyvsp[-5].num  ) << 22)	 | (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
break;
case 270:
#line 6543 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-5].num   > 7) { Error("Must be register R0-R7"); } ;
			yyval.num  = (( yyvsp[-9].num  ) << 19)	 | ( yyvsp[-7].num  )		 | (( yyvsp[-5].num  ) << 22)	 | (( yyvsp[-2].num  ) << 16)	 				| (( yyvsp[0].num  ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
break;
case 271:
#line 6569 "gasm.y"
{
			/* check registers are valid */
			if ( yyvsp[-7].num   > 1) { Error("Must be register R0 or R1"); } ;

			/* construct address mode and operand fields to */
			/* insert into instruction template */
			/* subtract 2 from Dreg2_3 to make 1 bit reg selector */
			yyval.num  = yyvsp[-5].num  | 	(	(( 0x0  ) << 24)		)  				| (( yyvsp[-9].num  ) << 8)	 | ( yyvsp[-11].num  )		 | (( yyvsp[-7].num  ) << 23)	 				| (( yyvsp[-4].num  ) << 16)	 | (( yyvsp[-2].num  ) << 19)	 | (( yyvsp[0].num  - 2 ) << 22)	;

			/* Second pass to check that disp.'s are only 1 or 0 */
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 272:
#line 6584 "gasm.y"
{
			/* check registers are valid */
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-11].num  ) << 8)	 | (( yyvsp[-9].num  ) << 19)	 | (( yyvsp[-7].num  ) << 23)	 				| ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 273:
#line 6594 "gasm.y"
{
			/* same as previous, but allowing for commutative mpy */
			/* Optional syntax #14 */

			if ( yyvsp[-11].num   > 7) { Error("Must be register R0-R7"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-9].num  ) << 8)	 | (( yyvsp[-11].num  ) << 19)	 | (( yyvsp[-7].num  ) << 23)	 				| ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 274:
#line 6606 "gasm.y"
{
			if ( yyvsp[-11].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x2  ) << 24)		)  				| (( yyvsp[-9].num  ) << 19)	 | (( yyvsp[-11].num  ) << 16)	 | (( yyvsp[-7].num  ) << 23)	 				| ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 275:
#line 6615 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x1  ) << 24)		)  				| (( yyvsp[-11].num  ) << 8)	 | (( yyvsp[-9].num  ) << 19)	 | (( yyvsp[-7].num  ) << 23)	 				| (( yyvsp[-4].num  ) << 16)	 | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 276:
#line 6624 "gasm.y"
{
			/* same again but allowing for commutative mpy */
			/* Optional syntax #14 */

			if ( yyvsp[-11].num   > 7) { Error("Must be register R0-R7"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x1  ) << 24)		)  				| (( yyvsp[-9].num  ) << 8)	 | (( yyvsp[-11].num  ) << 19)	 | (( yyvsp[-7].num  ) << 23)	 				| (( yyvsp[-4].num  ) << 16)	 | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 277:
#line 6640 "gasm.y"
{
			if ( yyvsp[-7].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-9].num  ) << 8)	 | (( yyvsp[-7].num  ) << 19)	 | (( yyvsp[-7].num  ) << 23)	 				| ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 278:
#line 6649 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x2  ) << 24)		)  				| (( yyvsp[-7].num  ) << 19)	 | (( yyvsp[-9].num  ) << 16)	 | (( yyvsp[-7].num  ) << 23)	 				| ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 279:
#line 6658 "gasm.y"
{
			if ( yyvsp[-7].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x1  ) << 24)		)  				| (( yyvsp[-9].num  ) << 8)	 | (( yyvsp[-7].num  ) << 19)	 | (( yyvsp[-7].num  ) << 23)	 				| (( yyvsp[-4].num  ) << 16)	 | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 280:
#line 6671 "gasm.y"
{
			if ( yyvsp[-5].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-3].num  | 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-7].num  ) << 8)	 | (( yyvsp[-5].num  ) << 19)	 | (( yyvsp[-5].num  ) << 23)	 				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 16)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 281:
#line 6684 "gasm.y"
{
			if ( yyvsp[-5].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-3].num  | 	(	(( 0x0  ) << 24)		)  				| (( yyvsp[-7].num  ) << 8)	 | ( yyvsp[-9].num  )		 | (( yyvsp[-5].num  ) << 23)	 				| (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  ) << 19)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 282:
#line 6693 "gasm.y"
{
			if ( yyvsp[-7].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-5].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-3].num  | 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-9].num  ) << 8)	 | (( yyvsp[-7].num  ) << 19)	 | (( yyvsp[-5].num  ) << 23)	 				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 16)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 283:
#line 6702 "gasm.y"
{
			/* same again but allowing for commutative * */
			/* Optional syntax #14 */

			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ;

			yyval.num  = yyvsp[-3].num  | 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-9].num  ) << 19)	 | (( yyvsp[-7].num  ) << 8)	 | (( yyvsp[-5].num  ) << 23)	 				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 16)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 284:
#line 6719 "gasm.y"
{	yyval.num  = (((unsigned)0x2  << 30) | (( 0x3  ) << 26)) 	; }
break;
case 285:
#line 6721 "gasm.y"
{	yyval.num  = (((unsigned)0x2  << 30) | (( 0x2  ) << 26)) 	; }
break;
case 286:
#line 6723 "gasm.y"
{	yyval.num  = (((unsigned)0x2  << 30) | (( 0x3  ) << 26)) 	; }
break;
case 287:
#line 6725 "gasm.y"
{	yyval.num  = (((unsigned)0x2  << 30) | (( 0x2  ) << 26)) 	; }
break;
case 288:
#line 6734 "gasm.y"
{
			/* check registers are valid */
			if ( yyvsp[-7].num   > 1) { Error("Must be register R0 or R1"); } ;

			/* construct address mode and operand fields to */
			/* insert into instruction template */
			/* subtract 2 from Dreg2_3 to make 1 bit reg selector */
			yyval.num  = yyvsp[-5].num  | 	(	(( 0x0  ) << 24)		)  				| (( yyvsp[-9].num  ) << 8)	 | ( yyvsp[-11].num  )		 | (( yyvsp[-7].num  ) << 23)	 				| (( yyvsp[-4].num  ) << 16)	 | (( yyvsp[-2].num  ) << 19)	 | (( yyvsp[0].num  - 2 ) << 22)	;

			/* Second pass to check that disp.'s are only 1 or 0 */
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 289:
#line 6749 "gasm.y"
{
			/* check registers are valid */
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-11].num  ) << 8)	 | (( yyvsp[-9].num  ) << 19)	 | (( yyvsp[-7].num  ) << 23)	 				| ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 290:
#line 6759 "gasm.y"
{
			/* same as previous, but allowing for commutative mpy */
			/* Optional syntax #14 */

			if ( yyvsp[-11].num   > 7) { Error("Must be register R0-R7"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-9].num  ) << 8)	 | (( yyvsp[-11].num  ) << 19)	 | (( yyvsp[-7].num  ) << 23)	 				| ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 291:
#line 6771 "gasm.y"
{
			if ( yyvsp[-11].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x2  ) << 24)		)  				| (( yyvsp[-9].num  ) << 19)	 | (( yyvsp[-11].num  ) << 16)	 | (( yyvsp[-7].num  ) << 23)	 				| ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 292:
#line 6780 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x1  ) << 24)		)  				| (( yyvsp[-11].num  ) << 8)	 | (( yyvsp[-9].num  ) << 19)	 | (( yyvsp[-7].num  ) << 23)	 				| (( yyvsp[-4].num  ) << 16)	 | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 293:
#line 6789 "gasm.y"
{
			/* same again but allowing for commutative mpy */
			/* Optional syntax #14 */

			if ( yyvsp[-11].num   > 7) { Error("Must be register R0-R7"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x1  ) << 24)		)  				| (( yyvsp[-9].num  ) << 8)	 | (( yyvsp[-11].num  ) << 19)	 | (( yyvsp[-7].num  ) << 23)	 				| (( yyvsp[-4].num  ) << 16)	 | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 294:
#line 6805 "gasm.y"
{
			if ( yyvsp[-7].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-9].num  ) << 8)	 | (( yyvsp[-7].num  ) << 19)	 | (( yyvsp[-7].num  ) << 23)	 				| ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 295:
#line 6814 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x2  ) << 24)		)  				| (( yyvsp[-7].num  ) << 19)	 | (( yyvsp[-9].num  ) << 16)	 | (( yyvsp[-7].num  ) << 23)	 				| ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 296:
#line 6823 "gasm.y"
{
			if ( yyvsp[-7].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-5].num  | 	(	(( 0x1  ) << 24)		)  				| (( yyvsp[-9].num  ) << 8)	 | (( yyvsp[-7].num  ) << 19)	 | (( yyvsp[-7].num  ) << 23)	 				| (( yyvsp[-4].num  ) << 16)	 | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 297:
#line 6836 "gasm.y"
{
			if ( yyvsp[-5].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-3].num  | 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-7].num  ) << 8)	 | (( yyvsp[-5].num  ) << 19)	 | (( yyvsp[-5].num  ) << 23)	 				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 16)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 298:
#line 6849 "gasm.y"
{
			if ( yyvsp[-5].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-3].num  | 	(	(( 0x0  ) << 24)		)  				| (( yyvsp[-7].num  ) << 8)	 | ( yyvsp[-9].num  )		 | (( yyvsp[-5].num  ) << 23)	 				| (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  ) << 19)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 299:
#line 6858 "gasm.y"
{
			if ( yyvsp[-7].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-5].num   > 1) { Error("Must be register R0 or R1"); } ;

			yyval.num  = yyvsp[-3].num  | 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-9].num  ) << 8)	 | (( yyvsp[-7].num  ) << 19)	 | (( yyvsp[-5].num  ) << 23)	 				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 16)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 300:
#line 6867 "gasm.y"
{
			/* same again but allowing for commutative * */
			/* Optional syntax #14 */

			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ;

			yyval.num  = yyvsp[-3].num  | 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-9].num  ) << 19)	 | (( yyvsp[-7].num  ) << 8)	 | (( yyvsp[-5].num  ) << 23)	 				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 16)	 | (( yyvsp[0].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 301:
#line 6884 "gasm.y"
{	yyval.num  = (((unsigned)0x2  << 30) | (( 0x1  ) << 26)) 	; }
break;
case 302:
#line 6886 "gasm.y"
{	yyval.num  = (((unsigned)0x2  << 30) | (( 0x0  ) << 26)) 	; }
break;
case 303:
#line 6888 "gasm.y"
{	yyval.num  = (((unsigned)0x2  << 30) | (( 0x1  ) << 26)) 	; }
break;
case 304:
#line 6890 "gasm.y"
{	yyval.num  = (((unsigned)0x2  << 30) | (( 0x0  ) << 26)) 	; }
break;
case 305:
#line 6905 "gasm.y"
{
			/* check registers are valid */
			if ( yyvsp[-11].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			/* construct address mode and operand fields to */
			/* insert into instruction template */
			/* subtract 2 from Dreg2_3 to make 1 bit reg selector */
			yyval.num  = 	(	(( 0x0  ) << 24)		)  				| (( yyvsp[-2].num  ) << 8)	 | ( yyvsp[-4].num  )		 | (( yyvsp[0].num  ) << 23)	 				| (( yyvsp[-11].num  ) << 16)	 | (( yyvsp[-9].num  ) << 19)	 | (( yyvsp[-7].num  - 2 ) << 22)	;

			/* Second pass to check that disp.'s are only 1 or 0 */
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 306:
#line 6920 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-4].num  ) << 8)	 | (( yyvsp[-2].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-11].num  )		 | (( yyvsp[-9].num  ) << 16)	 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 307:
#line 6929 "gasm.y"
{
			/* same again but allowing for commutative mpy */
			/* Optional syntax #14 */
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[-4].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-11].num  )		 | (( yyvsp[-9].num  ) << 16)	 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 308:
#line 6940 "gasm.y"
{
			if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x2  ) << 24)		)  				| (( yyvsp[-2].num  ) << 19)	 | (( yyvsp[-4].num  ) << 16)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-11].num  )		 | (( yyvsp[-9].num  ) << 8)	 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 309:
#line 6949 "gasm.y"
{
			if ( yyvsp[-11].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x1  ) << 24)		)  				| (( yyvsp[-4].num  ) << 8)	 | (( yyvsp[-2].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| (( yyvsp[-11].num  ) << 16)	 | ( yyvsp[-9].num  )		 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 310:
#line 6958 "gasm.y"
{
			/* same again but allowing for commutative mpy */
			/* Optional syntax #14 */
			if ( yyvsp[-11].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x1  ) << 24)		)  				| (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[-4].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| (( yyvsp[-11].num  ) << 16)	 | ( yyvsp[-9].num  )		 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 311:
#line 6973 "gasm.y"
{
			if ( yyvsp[-7].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-5].num   < 2 ||  yyvsp[-5].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-9].num  )		 | (( yyvsp[-7].num  ) << 16)	 | (( yyvsp[-5].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 312:
#line 6982 "gasm.y"
{
			if ( yyvsp[-5].num   < 2 ||  yyvsp[-5].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x2  ) << 24)		)  				| (( yyvsp[0].num  ) << 19)	 | (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-9].num  )		 | (( yyvsp[-7].num  ) << 8)	 | (( yyvsp[-5].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 313:
#line 6991 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-5].num   < 2 ||  yyvsp[-5].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x1  ) << 24)		)  				| (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| (( yyvsp[-9].num  ) << 16)	 | ( yyvsp[-7].num  )		 | (( yyvsp[-5].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 314:
#line 7004 "gasm.y"
{
			if ( yyvsp[-5].num   < 2 ||  yyvsp[-5].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-7].num  )		 | (( yyvsp[-5].num  ) << 16)	 | (( yyvsp[-5].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 315:
#line 7017 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x0  ) << 24)		)  				| (( yyvsp[-2].num  ) << 8)	 | ( yyvsp[-4].num  )		 | (( yyvsp[0].num  ) << 23)	 				| (( yyvsp[-9].num  ) << 16)	 | (( yyvsp[-7].num  ) << 19)	 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 316:
#line 7026 "gasm.y"
{
			if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-4].num  ) << 8)	 | (( yyvsp[-2].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-9].num  )		 | (( yyvsp[-7].num  ) << 16)	 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 317:
#line 7035 "gasm.y"
{
			/* same again but allowing for commutative * */
			/* Optional syntax #14 */

			if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-4].num  ) << 19)	 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-9].num  )		 | (( yyvsp[-7].num  ) << 16)	 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 320:
#line 7058 "gasm.y"
{
			/* check registers are valid */
			if ( yyvsp[-11].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			/* construct address mode and operand fields to */
			/* insert into instruction template */
			/* subtract 2 from Dreg2_3 to make 1 bit reg selector */
			yyval.num  = 	(	(( 0x0  ) << 24)		)  				| (( yyvsp[-2].num  ) << 8)	 | ( yyvsp[-4].num  )		 | (( yyvsp[0].num  ) << 23)	 				| (( yyvsp[-11].num  ) << 16)	 | (( yyvsp[-9].num  ) << 19)	 | (( yyvsp[-7].num  - 2 ) << 22)	;

			/* Second pass to check that disp.'s are only 1 or 0 */
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 321:
#line 7073 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-4].num  ) << 8)	 | (( yyvsp[-2].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-11].num  )		 | (( yyvsp[-9].num  ) << 16)	 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 322:
#line 7082 "gasm.y"
{
			/* same again but allowing for commutative mpy */
			/* Optional syntax #14 */
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[-4].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-11].num  )		 | (( yyvsp[-9].num  ) << 16)	 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 323:
#line 7093 "gasm.y"
{
			if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x2  ) << 24)		)  				| (( yyvsp[-2].num  ) << 19)	 | (( yyvsp[-4].num  ) << 16)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-11].num  )		 | (( yyvsp[-9].num  ) << 8)	 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 324:
#line 7102 "gasm.y"
{
			if ( yyvsp[-11].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x1  ) << 24)		)  				| (( yyvsp[-4].num  ) << 8)	 | (( yyvsp[-2].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| (( yyvsp[-11].num  ) << 16)	 | ( yyvsp[-9].num  )		 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 325:
#line 7111 "gasm.y"
{
			/* same again but allowing for commutative mpy */
			/* Optional syntax #14 */
			if ( yyvsp[-11].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x1  ) << 24)		)  				| (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[-4].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| (( yyvsp[-11].num  ) << 16)	 | ( yyvsp[-9].num  )		 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 326:
#line 7126 "gasm.y"
{
			if ( yyvsp[-7].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-5].num   < 2 ||  yyvsp[-5].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-9].num  )		 | (( yyvsp[-7].num  ) << 16)	 | (( yyvsp[-5].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 327:
#line 7135 "gasm.y"
{
			if ( yyvsp[-5].num   < 2 ||  yyvsp[-5].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x2  ) << 24)		)  				| (( yyvsp[0].num  ) << 19)	 | (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-9].num  )		 | (( yyvsp[-7].num  ) << 8)	 | (( yyvsp[-5].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 328:
#line 7144 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-5].num   < 2 ||  yyvsp[-5].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x1  ) << 24)		)  				| (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| (( yyvsp[-9].num  ) << 16)	 | ( yyvsp[-7].num  )		 | (( yyvsp[-5].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 329:
#line 7157 "gasm.y"
{
			if ( yyvsp[-5].num   < 2 ||  yyvsp[-5].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-7].num  )		 | (( yyvsp[-5].num  ) << 16)	 | (( yyvsp[-5].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 330:
#line 7170 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ; if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x0  ) << 24)		)  				| (( yyvsp[-2].num  ) << 8)	 | ( yyvsp[-4].num  )		 | (( yyvsp[0].num  ) << 23)	 				| (( yyvsp[-9].num  ) << 16)	 | (( yyvsp[-7].num  ) << 19)	 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
break;
case 331:
#line 7179 "gasm.y"
{
			if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-4].num  ) << 8)	 | (( yyvsp[-2].num  ) << 19)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-9].num  )		 | (( yyvsp[-7].num  ) << 16)	 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 332:
#line 7188 "gasm.y"
{
			/* same again but allowing for commutative * */
			/* Optional syntax #14 */

			if ( yyvsp[-7].num   < 2 ||  yyvsp[-7].num   > 3) { Error("Must be Register R2 or R3"); } ;

			yyval.num  = 	(	(( 0x3  ) << 24)		)  				| (( yyvsp[-4].num  ) << 19)	 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 23)	 				| ( yyvsp[-9].num  )		 | (( yyvsp[-7].num  ) << 16)	 | (( yyvsp[-7].num  - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
break;
case 335:
#line 7209 "gasm.y"
{
			/* create instr. template to be filled in by parser */
			if ((CurInstr = (Instruction *)malloc(sizeof(Instruction))) == ((void *)  0) )
				Fatal("Out of Memory for new instruction");

			/* initialise instruction template */
			CurInstr->opcode = 0;
			CurInstr->optexpr = ((void *)  0) ;
			CurInstr->combine = 0;
			CurInstr->optexpr2 = ((void *)  0) ;
			CurInstr->combine2 = 0;
		}
break;
case 336:
#line 7222 "gasm.y"
{
			/* add new instruction into the parse tree */
			ParseTreeItem *pti = NewParseTreeItem(INSTRUCTION);
			pti->type.instr = CurInstr;

			curPC += sizeof(int);
		}
break;
case 349:
#line 7249 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 350:
#line 7253 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x3   << 21)					 | (( yyvsp[0].num  ) << 16)		;
			/* unsigned immediate */
			CurInstr->combine = 2 ;
		}
break;
case 351:
#line 7262 "gasm.y"
{
			if (yyvsp[0].num  == 	0x10 ) {
				/* Second pass will >> 16 the immediate value */
				CurInstr->combine = 	3 ;
				/* diadic points to a LDPK instruction */
				/* immediate mode and DP reg already set */
				/* in OP_LDPK */
				CurInstr->opcode = yyvsp[-3].mnem ->diadic;
			}
			else {
				/* Second pass will >> 16 the immediate value */
				CurInstr->combine = 	3 ;
				/* triadic points to a LDIU instruction */
				CurInstr->opcode = yyvsp[-3].mnem ->triadic | 		( 0x3   << 21)					 | (( yyvsp[0].num  ) << 16)		;
			}
		}
break;
case 352:
#line 7279 "gasm.y"
{
				CurInstr->combine = 	3 ;
				CurInstr->opcode = yyvsp[-1].mnem ->diadic;
		}
break;
case 353:
#line 7284 "gasm.y"
{
			if (yyvsp[0].num  == 	0x10 ) {
				CurInstr->combine = 	3 ;
				CurInstr->opcode = yyvsp[-4].mnem ->diadic;
			}
			else {
				CurInstr->combine = 	3 ;
				CurInstr->opcode = yyvsp[-4].mnem ->triadic | 		( 0x3   << 21)					 | (( yyvsp[0].num  ) << 16)		;
			}
		}
break;
case 354:
#line 7295 "gasm.y"
{
				CurInstr->combine = 	3 ;
				CurInstr->opcode = yyvsp[-2].mnem ->diadic;
		}
break;
case 355:
#line 7303 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic; }
break;
case 356:
#line 7308 "gasm.y"
{	CurInstr->opcode = yyvsp[-3].mnem ->diadic | ( yyvsp[-2].num  )			 | (( yyvsp[0].num  ) << 16)		; }
break;
case 357:
#line 7310 "gasm.y"
{	CurInstr->opcode = yyvsp[-3].mnem ->diadic | ( yyvsp[-2].num  )			 | (( yyvsp[0].num  ) << 16)		; }
break;
case 358:
#line 7318 "gasm.y"
{ CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 359:
#line 7326 "gasm.y"
{ CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 363:
#line 7336 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic 				| 		( 0x0   << 21)					 | (( yyvsp[0].num  ) << 16)		 | ( yyvsp[-2].num  )			;
		}
break;
case 364:
#line 7340 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x1   << 21)					 | (( yyvsp[0].num  ) << 16)		;
		}
break;
case 365:
#line 7344 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic 				| 		( 0x2   << 21)					 | (( yyvsp[0].num  ) << 16)		 | (( yyvsp[-2].num  ) << 8)		;
		}
break;
case 366:
#line 7348 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x3   << 21)					 | (( yyvsp[0].num  ) << 16)		;
		}
break;
case 367:
#line 7352 "gasm.y"
{
			if ( yyvsp[-5].num   > 7) { Error("Must be register R0-R7"); } ;

			/* triadic position used to hold par_ld opcode */
			CurInstr->opcode = yyvsp[-8].mnem ->triadic 			  | ( yyvsp[-7].num  )		 | (( yyvsp[-5].num  ) << 22)	 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 19)	;

			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
break;
case 368:
#line 7362 "gasm.y"
{
			if ( yyvsp[-5].num   > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = yyvsp[-8].mnem ->par_st 			    | ( yyvsp[-7].num  )		 | (( yyvsp[-5].num  ) << 22)	 | (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  ) << 8)	;

			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
break;
case 369:
#line 7377 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic 			| 		( 0x0   << 21)					 | (( yyvsp[0].num  ) << 16)		 | ( yyvsp[-2].num  )			;
		}
break;
case 370:
#line 7381 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x1   << 21)					 | (( yyvsp[0].num  ) << 16)		;
		}
break;
case 371:
#line 7385 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic 				| 		( 0x2   << 21)					 | (( yyvsp[0].num  ) << 16)		 | (( yyvsp[-2].num  ) << 8)		;
		}
break;
case 372:
#line 7389 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x3   << 21)					 | yyvsp[-2].num  				| (( yyvsp[0].num  ) << 16)		;
		}
break;
case 373:
#line 7393 "gasm.y"
{
			if ( yyvsp[-5].num   > 7) { Error("Must be register R0-R7"); } ;

			/* triadic position used to hold par_ld opcode */
			CurInstr->opcode = yyvsp[-8].mnem ->triadic 			  | ( yyvsp[-7].num  )		 | (( yyvsp[-5].num  ) << 22)	 | (( yyvsp[-2].num  ) << 8)	 | (( yyvsp[0].num  ) << 19)	;

			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
break;
case 374:
#line 7403 "gasm.y"
{
			if ( yyvsp[-5].num   > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = yyvsp[-8].mnem ->par_st 			   | ( yyvsp[-7].num  )		 | (( yyvsp[-5].num  ) << 22)	 | (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  ) << 8)	;

			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
break;
case 375:
#line 7419 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic 				| 		( 0x0   << 21)					 | (( yyvsp[0].num  ) << 16)		 | ( yyvsp[-2].num  )			;
		}
break;
case 376:
#line 7423 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x1   << 21)					 | (( yyvsp[0].num  ) << 16)		;
		}
break;
case 377:
#line 7427 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic 				| 		( 0x2   << 21)					 | (( yyvsp[0].num  ) << 16)		 | (( yyvsp[-2].num  ) << 8)		;
		}
break;
case 386:
#line 7440 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x1   << 21)					 | (( yyvsp[0].num  ) << 16)		;
		}
break;
case 387:
#line 7444 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic 				| 		( 0x2   << 21)					 | (( yyvsp[0].num  ) << 16)		 | (( yyvsp[-2].num  ) << 8)		;
		}
break;
case 391:
#line 7459 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 392:
#line 7462 "gasm.y"
{
			/* STIK uses REG mode for direct mode */
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x0   << 21)					;

			/* In 2nd pass, check immediate is <= 5bits */
			CurInstr->combine = 	6 ;
		}
break;
case 393:
#line 7470 "gasm.y"
{
			/* STIK uses IMMediate mode for indirect mode */
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x3   << 21)					 | (( yyvsp[0].num  ) << 8)		;
			CurInstr->combine = 	6 ;
		}
break;
case 394:
#line 7477 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x1   << 21)					 | (( yyvsp[-2].num  ) << 16)		;
		}
break;
case 395:
#line 7481 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x2   << 21)					 | (( yyvsp[-2].num  ) << 16)		 				| (( yyvsp[0].num  ) << 8)		;
		}
break;
case 396:
#line 7486 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x1   << 21)					 | (( yyvsp[-2].num  ) << 16)		;
		}
break;
case 397:
#line 7490 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x2   << 21)					 | (( yyvsp[-2].num  ) << 16)		 				| (( yyvsp[0].num  ) << 8)		;
		}
break;
case 398:
#line 7505 "gasm.y"
{
			if ( yyvsp[-7].num   > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = yyvsp[-8].mnem ->par_st 				| (( yyvsp[-7].num  ) << 22)	 | ( yyvsp[-5].num  )		 				| (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  ) << 8)	;

			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
break;
case 399:
#line 7519 "gasm.y"
{
			if ( yyvsp[-7].num   > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = yyvsp[-3].mnem ->par_st 				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 22)	 				| (( yyvsp[-7].num  ) << 16)	 | (( yyvsp[-5].num  ) << 8)	;

			CurInstr->combine =  	14 ;
			CurInstr->combine2 = 	13 ;
		}
break;
case 400:
#line 7534 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = yyvsp[-5].mnem ->par_st 				| ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 19)	 | (( yyvsp[0].num  ) << 22)	 				| (( yyvsp[-9].num  ) << 16)	 | (( yyvsp[-7].num  ) << 8)	;

			CurInstr->combine =  	14 ;
			CurInstr->combine2 = 	13 ;
		}
break;
case 401:
#line 7545 "gasm.y"
{
			if ( yyvsp[-7].num   > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = yyvsp[-3].mnem ->par_st 				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 19)	 | (( yyvsp[0].num  ) << 22)	
				| (( yyvsp[-7].num  ) << 16)	 | (( yyvsp[-5].num  ) << 8)	;
			CurInstr->combine = 	14 ;
			CurInstr->combine2 = 	13 ;
		}
break;
case 402:
#line 7555 "gasm.y"
{
			if ( yyvsp[-7].num   > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = yyvsp[-3].mnem ->par_st 				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 22)	 				| (( yyvsp[-7].num  ) << 16)	 | (( yyvsp[-5].num  ) << 8)	;
			CurInstr->combine = 	14 ;
			CurInstr->combine2 = 	13 ;
		}
break;
case 403:
#line 7564 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = yyvsp[-5].mnem ->par_st 				| (( yyvsp[-4].num  ) << 19)	 | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 22)	 				| (( yyvsp[-9].num  ) << 16)	 | (( yyvsp[-7].num  ) << 8)	;

			CurInstr->combine = 	14 ;
			CurInstr->combine2 = 	13 ;
		}
break;
case 404:
#line 7577 "gasm.y"
{
			if ( yyvsp[-7].num   > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = yyvsp[-8].mnem ->par_st 				| (( yyvsp[-7].num  ) << 22)	 | ( yyvsp[-5].num  )		 				| (( yyvsp[-2].num  ) << 16)	 | (( yyvsp[0].num  ) << 8)	;

			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
break;
case 405:
#line 7590 "gasm.y"
{
			if ( yyvsp[-7].num   > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = yyvsp[-3].mnem ->par_st 				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 22)	 				| (( yyvsp[-7].num  ) << 16)	 | (( yyvsp[-5].num  ) << 8)	;

			CurInstr->combine = 	14 ;
			CurInstr->combine2 = 	13 ;
		}
break;
case 406:
#line 7606 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = yyvsp[-5].mnem ->par_st 				| ( yyvsp[-4].num  )		 | (( yyvsp[-2].num  ) << 19)	 | (( yyvsp[0].num  ) << 22)	 				| (( yyvsp[-9].num  ) << 16)	 | (( yyvsp[-7].num  ) << 8)	;

			CurInstr->combine =  	14 ;
			CurInstr->combine2 = 	13 ;
		}
break;
case 407:
#line 7617 "gasm.y"
{
			if ( yyvsp[-7].num   > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = yyvsp[-3].mnem ->par_st 				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 19)	 | (( yyvsp[0].num  ) << 22)	
				| (( yyvsp[-7].num  ) << 16)	 | (( yyvsp[-5].num  ) << 8)	;
			CurInstr->combine = 	14 ;
			CurInstr->combine2 = 	13 ;
		}
break;
case 408:
#line 7628 "gasm.y"
{
			if ( yyvsp[-7].num   > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = yyvsp[-3].mnem ->par_st 				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 22)	 				| (( yyvsp[-7].num  ) << 16)	 | (( yyvsp[-5].num  ) << 8)	;
			CurInstr->combine = 	14 ;
			CurInstr->combine2 = 	13 ;
		}
break;
case 409:
#line 7638 "gasm.y"
{
			if ( yyvsp[-9].num   > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = yyvsp[-5].mnem ->par_st 				| (( yyvsp[-4].num  ) << 19)	 | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 22)	 				| (( yyvsp[-9].num  ) << 16)	 | (( yyvsp[-7].num  ) << 8)	;

			CurInstr->combine = 	14 ;
			CurInstr->combine2 = 	13 ;
		}
break;
case 448:
#line 7680 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->diadic;

			/* specifies 24 bit pcrel value */
			CurInstr->combine = 18 ;
		}
break;
case 453:
#line 7693 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | (1 << 25)	; }
break;
case 454:
#line 7695 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | 	0		 | ( yyvsp[0].num  )			; }
break;
case 461:
#line 7703 "gasm.y"
{	
			CurInstr->opcode = yyvsp[-3].mnem ->diadic 				| 	0		 | ((( yyvsp[-2].num  ) - 8) << 22)	 | ( yyvsp[0].num  )			;
		}
break;
case 462:
#line 7707 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic 				| (1 << 25)	 | ((( yyvsp[-2].num  ) - 8) << 22)	;
		}
break;
case 463:
#line 7711 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic 				| 	0		 | ((( yyvsp[-2].num  ) - 8) << 22)	 | ( yyvsp[0].num  )			;
		}
break;
case 464:
#line 7715 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic 				| (1 << 25)	 | ((( yyvsp[-2].num  ) - 8) << 22)	;
		}
break;
case 465:
#line 7722 "gasm.y"
{	CurInstr->opcode = yyvsp[0].mnem ->diadic; }
break;
case 469:
#line 7730 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->diadic;
			CurInstr->combine = 19 ;
		}
break;
case 472:
#line 7740 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->diadic;

			/* check and insert 24 bit pc rel offset */
			CurInstr->combine = 18 ;
		}
break;
case 473:
#line 7747 "gasm.y"
{
			/* note that "RPTB reg" and "RPTB pcrel" have */
			/* completely different opcodes! The reg version */
			/* is held in the triadic position */
			CurInstr->opcode = yyvsp[-1].mnem ->triadic | ( yyvsp[0].num  )			;
		}
break;
case 474:
#line 7755 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | 		( 0x0   << 21)					 | ( yyvsp[0].num  )			; }
break;
case 475:
#line 7757 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | 		( 0x1   << 21)					; }
break;
case 476:
#line 7759 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->diadic | 		( 0x3   << 21)					;
			/* immediate is unsigned */
			CurInstr->combine = 2 ;
		}
break;
case 477:
#line 7765 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | 		( 0x2   << 21)					 | (( yyvsp[0].num  ) << 8)		; }
break;
case 480:
#line 7777 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | (( yyvsp[0].num  ) << 16)		; }
break;
case 481:
#line 7779 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | (( yyvsp[0].num  ) << 16)		; }
break;
case 482:
#line 7781 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | (( yyvsp[0].num  ) << 16)		; }
break;
case 483:
#line 7783 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | (( yyvsp[0].num  ) << 16)		; }
break;
case 484:
#line 7792 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | 		( 0x1   << 21)					; }
break;
case 485:
#line 7794 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | 		( 0x2   << 21)					 | (( yyvsp[0].num  ) << 8)		; }
break;
case 486:
#line 7797 "gasm.y"
{	CurInstr->opcode = yyvsp[0].mnem ->diadic; }
break;
case 487:
#line 7800 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | 		( 0x2   << 21)					 | (( yyvsp[0].num  ) << 8)		; }
break;
case 488:
#line 7802 "gasm.y"
{	CurInstr->opcode = yyvsp[0].mnem ->diadic; }
break;
case 489:
#line 7805 "gasm.y"
{	CurInstr->opcode = yyvsp[0].mnem ->diadic; }
break;
case 518:
#line 7852 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 519:
#line 7854 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 520:
#line 7862 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 521:
#line 7864 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 524:
#line 7874 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 528:
#line 7885 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 529:
#line 7893 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ;
			if (yyvsp[-1].mnem ->token == ANDN3 || yyvsp[-1].mnem ->token == MPYUHI3) {
				/* convert pass2 immediate tests to unsigned */
				if (CurInstr->combine == 	11 )
					CurInstr->combine = 12 ;
				else if (CurInstr->combine2 == 	11 )
					CurInstr->combine2 = 12 ;
			}
		}
break;
case 535:
#line 7912 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ;
			if (yyvsp[-1].mnem ->token == ANDN || yyvsp[-1].mnem ->token == MPYUHI) {
				/* convert pass2 immediate tests to unsigned */
				if (CurInstr->combine == 	1 )
					CurInstr->combine = 2 ;
			}
		}
break;
case 536:
#line 7921 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ;
			if (yyvsp[-1].mnem ->token == ANDN || yyvsp[-1].mnem ->token == MPYUHI) {
				/* convert pass2 immediate tests to unsigned */
				if (CurInstr->combine == 	11 )
					CurInstr->combine = 12 ;
				else if (CurInstr->combine2 == 	11 )
					CurInstr->combine2 = 12 ;
			}
		}
break;
case 542:
#line 7940 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ;
		}
break;
case 543:
#line 7944 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ;
			if (yyvsp[-1].mnem ->token == NOT && CurInstr->combine == 	1 ) {
				/* convert pass2 immediate tests to unsigned */
				CurInstr->combine = 2 ;
			}
		}
break;
case 544:
#line 7952 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ;	}
break;
case 548:
#line 7962 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 549:
#line 7964 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 550:
#line 7966 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 553:
#line 7975 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 554:
#line 7977 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 555:
#line 7979 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 556:
#line 7986 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ;
			/* convert pass2 immediate tests to unsigned */
			if (CurInstr->combine == 	11 )
				CurInstr->combine = 12 ;
			else if (CurInstr->combine2 == 	11 )
				CurInstr->combine2 = 12 ;
		}
break;
case 557:
#line 7995 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 561:
#line 8006 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ; }
break;
case 562:
#line 8009 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 565:
#line 8020 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ; }
break;
case 566:
#line 8022 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 567:
#line 8024 "gasm.y"
{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x3  ) << 26)) 	 | yyvsp[0].num ; }
break;
case 568:
#line 8033 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ; }
break;
case 569:
#line 8035 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 570:
#line 8037 "gasm.y"
{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x2  ) << 26)) 	 | yyvsp[0].num ; }
break;
case 571:
#line 8046 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ; }
break;
case 572:
#line 8048 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 573:
#line 8050 "gasm.y"
{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x1  ) << 26)) 	 | yyvsp[0].num ; }
break;
case 574:
#line 8059 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ; }
break;
case 575:
#line 8061 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 576:
#line 8063 "gasm.y"
{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x0  ) << 26)) 	 | yyvsp[0].num ; }
break;
case 577:
#line 8071 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ;
			/* convert pass2 immediate tests to unsigned */
			if (CurInstr->combine == 	1 )
				CurInstr->combine = 2 ;
		}
break;
case 578:
#line 8078 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ;
			/* convert pass2 immediate tests to unsigned */
			if (CurInstr->combine == 	11 )
				CurInstr->combine = 12 ;
			else if (CurInstr->combine2 == 	11 )
				CurInstr->combine2 = 12 ;
		}
break;
case 579:
#line 8087 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 583:
#line 8097 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 584:
#line 8100 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ; }
break;
case 585:
#line 8103 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 588:
#line 8114 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 589:
#line 8116 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ; }
break;
case 590:
#line 8118 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 591:
#line 8120 "gasm.y"
{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x3  ) << 26)) 	 | yyvsp[0].num ; }
break;
case 592:
#line 8128 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 593:
#line 8130 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ; }
break;
case 594:
#line 8132 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 595:
#line 8134 "gasm.y"
{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x2  ) << 26)) 	 | yyvsp[0].num ; }
break;
case 596:
#line 8142 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 597:
#line 8144 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ; }
break;
case 598:
#line 8146 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 599:
#line 8148 "gasm.y"
{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x1  ) << 26)) 	 | yyvsp[0].num ; }
break;
case 600:
#line 8156 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 601:
#line 8158 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ; }
break;
case 602:
#line 8160 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 603:
#line 8162 "gasm.y"
{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x0  ) << 26)) 	 | yyvsp[0].num ; }
break;
case 608:
#line 8187 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ; }
break;
case 609:
#line 8189 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 610:
#line 8191 "gasm.y"
{	CurInstr->opcode = yyvsp[0].num ; }
break;
case 611:
#line 8197 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 612:
#line 8199 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ; }
break;
case 613:
#line 8201 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 614:
#line 8203 "gasm.y"
{	CurInstr->opcode = yyvsp[0].num ; }
break;
case 615:
#line 8215 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ; }
break;
case 616:
#line 8217 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 617:
#line 8219 "gasm.y"
{	CurInstr->opcode = yyvsp[0].num ; }
break;
case 618:
#line 8225 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 619:
#line 8227 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->triadic | yyvsp[0].num ; }
break;
case 620:
#line 8229 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 621:
#line 8231 "gasm.y"
{	CurInstr->opcode = yyvsp[0].num ; }
break;
case 622:
#line 8240 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | (( yyvsp[0].num  ) << 16)		; }
break;
case 627:
#line 8252 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic 				| 		( 0x0   << 21)					 | ( yyvsp[-2].num  )			 | (( yyvsp[0].num  ) << 16)		;
		}
break;
case 628:
#line 8256 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic 				| 		( 0x1   << 21)					 | (( yyvsp[0].num  ) << 16)		;
		}
break;
case 629:
#line 8260 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic 				| 		( 0x2   << 21)					 | (( yyvsp[-2].num  ) << 8)		 | (( yyvsp[0].num  ) << 16)		;
		}
break;
case 636:
#line 8292 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 637:
#line 8296 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->triadic 				| (0			 | 	(( 0x1  ) << 21)		)  | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
			CurInstr->combine = 	8	;
		}
break;
case 638:
#line 8303 "gasm.y"
{
			if (!((( yyvsp[0].num  ) & 0xf8 ) == 0x0 ) ) {
				Error("Only *+ARn format indirections allowed for \"cmpi3 immediate, indirect\"");
			}
			CurInstr->opcode = yyvsp[-3].mnem ->triadic 				| ((1 << 28)		 | 	(( 0x2  ) << 21)		)  | (( yyvsp[0].num  ) << 8)	;
			if (yyvsp[-3].mnem ->token == TSTB)
				CurInstr->combine = 12 ;
			else
				CurInstr->combine = 	11 ;
			CurInstr->combine2 = 	10	;
		}
break;
case 639:
#line 8322 "gasm.y"
{
			if (((( yyvsp[-2].num  ) & 0xf8 ) == 0x0 )  && ((( yyvsp[0].num  ) & 0xf8 ) == 0x0 ) ) {
				CurInstr->opcode = yyvsp[-3].mnem ->triadic 					| ((1 << 28)		 | 	(( 0x3  ) << 21)		)  | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;

				CurInstr->combine = 	9	;
				CurInstr->combine2 = 	10	;
			}
			else {
				CurInstr->opcode = yyvsp[-3].mnem ->triadic 					| (0			 | 	(( 0x3  ) << 21)		)  | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
				CurInstr->combine = 	7	;
				CurInstr->combine2 = 	8	;
			}
		}
break;
case 642:
#line 8345 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->diadic | yyvsp[0].num ; }
break;
case 643:
#line 8349 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->triadic 				| (0			 | 	(( 0x1  ) << 21)		)  | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
			CurInstr->combine = 	8	;
		}
break;
case 644:
#line 8364 "gasm.y"
{
			if (((( yyvsp[-2].num  ) & 0xf8 ) == 0x0 )  && ((( yyvsp[0].num  ) & 0xf8 ) == 0x0 ) ) {
				CurInstr->opcode = yyvsp[-3].mnem ->triadic 					| ((1 << 28)		 | 	(( 0x3  ) << 21)		)  | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;

				CurInstr->combine = 	9	;
				CurInstr->combine2 = 	10	;
			}
			else {
				CurInstr->opcode = yyvsp[-3].mnem ->triadic 					| (0			 | 	(( 0x3  ) << 21)		)  | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
				CurInstr->combine = 	7	;
				CurInstr->combine2 = 	8	;
			}
		}
break;
case 645:
#line 8385 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->triadic | (0			 | 	(( 0x0  ) << 21)		)  				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
		}
break;
case 646:
#line 8389 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->triadic | (0			 | 	(( 0x1  ) << 21)		)  				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
			CurInstr->combine = 	8	;
		}
break;
case 647:
#line 8395 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->triadic | ((1 << 28)		 | 	(( 0x0  ) << 21)		)  | (( yyvsp[0].num  ) << 8)	;
			if (yyvsp[-3].mnem ->token == TSTB3)
				CurInstr->combine = 12 ;
			else
				CurInstr->combine = 	11 ;
		}
break;
case 648:
#line 8404 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->triadic | ((1 << 28)		 | 	(( 0x2  ) << 21)		)  | (( yyvsp[0].num  ) << 8)	;
			if (yyvsp[-3].mnem ->token == TSTB3)
				CurInstr->combine = 12 ;
			else
				CurInstr->combine = 	11 ;

			if (!((( yyvsp[0].num  ) & 0xf8 ) == 0x0 ) )
				Error("Only *+ARn format indirections allowed for type 2 triadic instructions");

			CurInstr->combine2 = 	10	;
		}
break;
case 649:
#line 8426 "gasm.y"
{
			if (((( yyvsp[-2].num  ) & 0xf8 ) == 0x0 ) ) {
				CurInstr->opcode = yyvsp[-3].mnem ->triadic 					| ((1 << 28)		 | 	(( 0x1  ) << 21)		)  | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
				CurInstr->combine = 	9	;
			}
			else {
				CurInstr->opcode = yyvsp[-3].mnem ->triadic 					| (0			 | 	(( 0x2  ) << 21)		)  | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
				CurInstr->combine = 	7	;
			}
		}
break;
case 650:
#line 8437 "gasm.y"
{
			if (((( yyvsp[-2].num  ) & 0xf8 ) == 0x0 )  && ((( yyvsp[0].num  ) & 0xf8 ) == 0x0 ) ) {
				CurInstr->opcode = yyvsp[-3].mnem ->triadic 					| ((1 << 28)		 | 	(( 0x3  ) << 21)		)  | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
				CurInstr->combine = 	9	;
				CurInstr->combine2 = 	10	;
			}
			else {
				CurInstr->opcode = yyvsp[-3].mnem ->triadic 					| (0			 | 	(( 0x3  ) << 21)		)  | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
				CurInstr->combine = 	7	;
				CurInstr->combine2 = 	8	;
			}
		}
break;
case 653:
#line 8459 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->triadic | (0			 | 	(( 0x0  ) << 21)		)  				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
		}
break;
case 654:
#line 8463 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->triadic | (0			 | 	(( 0x1  ) << 21)		)  				| ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
			CurInstr->combine = 	8	;
		}
break;
case 655:
#line 8479 "gasm.y"
{
			if (((( yyvsp[-2].num  ) & 0xf8 ) == 0x0 ) ) {
				CurInstr->opcode = yyvsp[-3].mnem ->triadic 					| ((1 << 28)		 | 	(( 0x1  ) << 21)		)  | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
				CurInstr->combine = 	9	;
			}
			else {
				CurInstr->opcode = yyvsp[-3].mnem ->triadic 					| (0			 | 	(( 0x2  ) << 21)		)  | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
				CurInstr->combine = 	7	;
			}
		}
break;
case 656:
#line 8490 "gasm.y"
{
			if (((( yyvsp[-2].num  ) & 0xf8 ) == 0x0 )  && ((( yyvsp[0].num  ) & 0xf8 ) == 0x0 ) ) {
				CurInstr->opcode = yyvsp[-3].mnem ->triadic 					| ((1 << 28)		 | 	(( 0x3  ) << 21)		)  | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
				CurInstr->combine = 	9	;
				CurInstr->combine2 = 	10	;
			}
			else {
				CurInstr->opcode = yyvsp[-3].mnem ->triadic 					| (0			 | 	(( 0x3  ) << 21)		)  | ( yyvsp[-2].num  )		 | (( yyvsp[0].num  ) << 8)	;
				CurInstr->combine = 	7	;
				CurInstr->combine2 = 	8	;
			}
		}
break;
case 657:
#line 8512 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->diadic | 		( 0x0   << 21)					 				| (( yyvsp[0].num  ) << 16)		 | ( yyvsp[0].num  )			;
		}
break;
case 658:
#line 8516 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x0   << 21)					 				| (( yyvsp[0].num  ) << 16)		 | ( yyvsp[-2].num  )			;
		}
break;
case 659:
#line 8520 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x1   << 21)					 				| (( yyvsp[0].num  ) << 16)		;
		}
break;
case 660:
#line 8524 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x2   << 21)					 				| (( yyvsp[0].num  ) << 16)		 | (( yyvsp[-2].num  ) << 8)		;
		}
break;
case 661:
#line 8528 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x3   << 21)					 | yyvsp[-2].num  				| (( yyvsp[0].num  ) << 16)		;
		}
break;
case 662:
#line 8534 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 663:
#line 8544 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->diadic | 		( 0x0   << 21)					 				| (( yyvsp[0].num  ) << 16)		 | ( yyvsp[0].num  )			;
		}
break;
case 664:
#line 8548 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x0   << 21)					 				| (( yyvsp[0].num  ) << 16)		 | ( yyvsp[-2].num  )			;
		}
break;
case 665:
#line 8552 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x1   << 21)					 				| (( yyvsp[0].num  ) << 16)		;
		}
break;
case 666:
#line 8556 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x2   << 21)					 				| (( yyvsp[0].num  ) << 16)		 | (( yyvsp[-2].num  ) << 8)		;
		}
break;
case 667:
#line 8562 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 668:
#line 8572 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x1   << 21)					 				| (( yyvsp[0].num  ) << 16)		;
		}
break;
case 669:
#line 8576 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->diadic | 		( 0x2   << 21)					 				| (( yyvsp[0].num  ) << 16)		 | (( yyvsp[-2].num  ) << 8)		;
		}
break;
case 670:
#line 8582 "gasm.y"
{	CurInstr->opcode = yyvsp[-1].mnem ->par_st | yyvsp[0].num ; }
break;
case 672:
#line 8601 "gasm.y"
{
			/* fake up constexpr item with our patch number */
			yyval.expr  = NewExprNum(yyvsp[0].num );
		}
break;
#line 14892 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#ifdef YYDEBUG
        if (yydebug)
            printf("yydebug: after reduction, shifting from state 0 to\
 state %d\n", YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("yydebug: state %d, reading %d (%s)\n",
                        YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#ifdef YYDEBUG
    if (yydebug)
        printf("yydebug: after reduction, shifting from state %d \
to state %d\n", *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
