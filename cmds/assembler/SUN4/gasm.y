/* Warning: You must only edit the unpre-processed .ypp version of this file */
/** STOP STOP STOP STOP STOP STOP STOP STOP STOP STOP STOP STOP STOP STOP ***/
/***************** WARNING: this file has been preprocessed *****************/
/* You must only edit the .ypp version, not this .y file extension version! */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING  */


/*
 * File:	gasm.ypp
 * Author:	P.A.Beskeen
 * Date:	Aug '91
 *
 * Description: Yacc grammer for generic Helios assembler
 *
 * This file is pre-processed by CPP to generate the correct YACC grammer
 * and actions for the target processors assembly language mnemonics.
 *
 * The parser (yyparse()) created by YACC from this file, uses the lexical
 * analyer (yylex() in lex.c) to provide it with a tokenised representation of
 * the input text. The parser checks the syntax of the input files and
 * creates a parse tree from this information. The parse tree in conjunction
 * with the symbol table is then used by later modules to create the object
 * file.
 *
 * RcsId: $Id: gasm.ypp,v 1.11 1993/07/12 16:16:46 nickc Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */

/* ******************************************************************** */
/* C definition section							*/

%{

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




%}


/* ******************************************************************** */
/* YACC declarations section						*/

/* defines types that can be returned from tokens and rules */
%union {
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
}

/* constant expression tokens */
%token <str>	NAME
%token <num>	NUMBER
%token <num>	CHARCONST
%token <flt>	FLOATNUM
%token		LSHIFT RSHIFT

/* message directives */
%token		USERERROR USERWARNING USERNOTE

/* termination directive - never explicitly seen - returns 0 */
/* %token	USEREND = 0 */

/* code area directives */
%token 		BYTE SHORT WORD FLOATY DOUBLE BLOCKBYTE BLOCKSHORT BLOCKWORD SPACE ALIGN

/* module data area directives */ 
%token 		DATA COMMON CODETABLE EXPORT IMPORT

/* object code directives */
%token		MODULE INIT REF PATCHINSTR

/* std patches */
%token		MODSIZE MODNUM DATASYMB CODESYMB DATAMODULE
%token		LABELREF CODESTUB ADDRSTUB BYTESWAP SHIFT P_ADD P_OR

/* string constants */
%token <str>	STRINGCONST
%token <str>	LABEL

/* pre / post increment / decrement operators */
/* usually used in CPU's indirect addressing modes */
%token		PLUSPLUS MINUSMINUS

/* CPU instruction, not actually returned by lexer, but used in parse tree */
/* to denote a machine specific opcode item */
%token <instr>	INSTRUCTION


/* ******************************************************************** */
/* Define what types are returned by internal rules 			*/

%type <num>	imm_constexpr opt_imm_constexpr
%type <flt>	imm_fp_constexpr
%type <expr>	instrpatch

%type <clist>	codeconstlist codeconst
%type <flist>	floatconstlist
%type <expr>	constexpr opt_constexpr
%type <patch>	stdpatch
%type <nlist>	namelist


/* ********************************************************************* */
/* Precidence of operators in decending level of precidence		 */

%left	'|'		/* lowest precidence */
%left	'^'
%left	'&'
%left	LSHIFT RSHIFT
%left	'+' '-'
%left	'*' '/' '%'
%left	UNARY		/* highest precidence */


/* ******************************************************************** */
/* Machine specific mnemonic tokens					*/
/* Machine specific linker instruction patch tokens			*/

/* include target CPU mneumonic and patch token definitions */
/*
 * File:	toks_C40.c
 * Subsystem:	Generic (C40) Assembler
 * Author:	P.A.Beskeen
 * Date:	Aug '91
 *
 * Description: Defines the tokens expected by the TMS320C40 specific parts of
 *		the parser.
 *
 *
 * RcsId: $Id: toks_C40.ypp,v 1.4 1992/11/09 17:28:34 craig Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */


/* ******************************************************************** */
/* 'C40 Parser Specific C Declarations 					*/
%{
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
%}



/* ******************************************************************** */
/* 'C40 Specific Mnemonic Tokens					*/


/* LOAD AND STORES */
/* =============== */

%token	<mnem>	LDA LDE LDEP LDF LDHI LDI LDM LDP LDPE LDPK STF STI STIK

%token	<mnem>	LBb	/* LB0 LB1 LB2 LB3 */

%token	<mnem>	LBUb 	/* LBU0 LBU1 LBU2 LBU3 */

%token	<mnem>	LDFcond
		/* LDFU LDFLO LDFLS LDFHI LDFHS LDFEQ LDFNE LDFL LDFLE LDFGT */
		/* LDFGE LDFEQ LDFNE LDFZ LDFNZ LDFP LDFN LDFNN LDFNV LDFV */
		/* LDFNUF LDFUF LDFNC LDFC LDFNLV LDFLV LDFNLUF LDFLUF LDFZUF */

%token	<mnem>	LDIcond
		/* LDIU LDILO LDILS LDIHI LDIHS LDIEQ LDINE LDIL LDILE LDIGT */
		/* LDIGE LDIEQ LDINE LDIZ LDINZ LDIP LDIN LDINN LDINV LDIV */
		/* LDINUF LDIUF LDINC LDIC LDINLV LDILV LDINLUF LDILUF LDIZUF */

%token	<mnem>	LHw	/* LH0 LH1 */
%token	<mnem>	LHUw	/* LHU0 LHU1 */

%token	<mnem>	LWLct	/* LWL0 LWL1 LWL2 LWL3 */
%token	<mnem>	LWRct	/* LWR0 LWR1 LWR2 LWR3 */


/* DIADIC INSTRUCTIONS */
/* =================== */

%token	<mnem>	ABSF ABSI ADDC ADDF ADDI AND ANDN ASH CMPF CMPI FIX FLOAT
%token	<mnem>	FRIEEE LSH MPYF MPYI MPYSHI MPYUHI NEGB NEGF
%token	<mnem>	NEGI NORM NOT OR RCPF RND ROL ROLC ROR RORC RSQRF SUBB
%token	<mnem>	SUBC SUBF SUBI SUBRB SUBRF SUBRI TOIEEE TSTB XOR

%token	<mnem>	MBct	/* MB0 MB1 MB2 MB3 */

%token	<mnem>	MHct	/* MH0 MH1 MH2 MH3 */


/* TRIADIC INSTRUCTIONS */
/* ==================== */

%token	<mnem>	ADDC3 ADDF3 ADDI3 AND3 ANDN3 ASH3 CMPF3 CMPI3 LSH3 MPYF3 MPYI3
%token	<mnem>	MPYSHI3 MPYUHI3 OR3 SUBB3 SUBF3 SUBI3 TSTB3 XOR3


/* PROGRAM CONTROL */
/* =============== */

%token	<mnem>	BR BRD CALL LAJ RPTB RPTBD RPTS 

%token	<mnem>	Bcond
		/* B BU BLO BLS BHI BHS BEQ BNE BL BLE BGT BGE BZ BNZ BP BN */
		/* BNN BNV BV BNUF BUF BNC BC BNLV BLV BNLUF BLUF BZUF */

%token	<mnem>	BcondAF
		/* BUAF BLOAF BLSAF BHIAF BHSAF BEQAF BNEAF BLAF BLEAF BGTAF */
		/* BGEAF BZAF BNZAF BPAF BNAF BNNAF BNVAF BVAF BNUFAF BUFAF */
		/* BNCAF BCAF BNLVAF BLVAF BNLUFAF BLUFAF BZUFAF */

%token	<mnem>	BcondAT
		/* BUAT BLOAT BLSAT BHIAT BHSAT BEQAT BNEAT BLAT BLEAT BGTAT */
		/* BGEAT BZAT BNZAT BPAT BNAT BNNAT BNVAT BVAT BNUFAT BUFAT */
		/* BNCAT BCAT BNLVAT BLVAT BNLUFAT BLUFAT BZUF */

%token	<mnem>	BcondD
		/* BUD BLOD BLSD BHID BHSD BEQD BNED BLD BLED BGTD BGED BZD */
		/* BNZD BPD BND BNND BNVD BVD BNUFD BUFD BNCD BCD BNLVD BLVD */
		/* BNLUFD BLUFD BZUF */

%token	<mnem>	CALLcond
		/* CALLU CALLLO CALLLS CALLHI CALLHS CALLEQ CALLNE CALLL */
		/* CALLLE CALLGT CALLGE CALLZ CALLNZ CALLP CALLN CALLNN */
		/* CALLNV CALLV CALLNUF CALLUF CALLNC CALLC CALLNLV CALLLV */
		/* CALLNLUF CALLLUF CALLZUF */

%token	<mnem>	DBcond
		/* DBU DBLO DBLS DBHI DBHS DBEQ DBNE DBL DBLE DBGT DBGE */
		/* DBZ DBNZ DBP DBN DBNN DBNV DBV DBNUF DBUF DBNC DBC DBNLV */
		/* DBLV DBNLUF DBLUF DBZUF */

%token	<mnem>	DBcondD
		/* DBUD DBLOD DBLSD DBHID DBHSD DBEQD DBNED DBLD DBLED DBGTD */
		/* DBGED DBZD DBNZD DBPD DBND DBNND DBNVD DBVD DBNUFD DBUFD */
		/* DBNCD DBCD DBNLVD DBLVD DBNLUFD DBLUFD DBZUF */

%token	<mnem>	LAJcond
		/* LAJU LAJLO LAJLS LAJHI LAJHS LAJEQ LAJNE LAJL LAJLE LAJGT */
		/* LAJGE LAJZ LAJNZ LAJP LAJN LAJNN LAJNV LAJV LAJNUF LAJUF */
		/* LAJNC LAJC LAJNLV LAJLV LAJNLUF LAJLUF LAJZUF */

%token	<mnem>	LATcond
		/* LATU LATLO LATLS LATHI LATHS LATEQ LATNE LATL LATLE LATGT */
		/* LATGE LATZ LATNZ LATP LATN LATNN LATNV LATV LATNUF LATUF */
		/* LATNC LATC LATNLV LATLV LATNLUF LATLUF LATZUF */

%token	<mnem>	RETIcond
	 	/* RETIU RETILO RETILS RETIHI RETIHS RETIEQ RETINE RETIL */
		/* RETILE RETIGT RETIGE RETIZ RETINZ RETIP RETIN RETINN */
		/* RETINV RETIV RETINUF RETIUF RETINC RETIC RETINLV RETILV */
		/* RETINLUF RETILUF RETIZUF */

%token	<mnem>	RETIcondD
		/* RETIUD RETILOD RETILSD RETIHID RETIHSD RETIEQD RETINED */
		/* RETILD RETILED RETIGTD RETIGED RETIZD RETINZD RETIPD */
		/* RETIND RETINND RETINVD RETIVD RETINUFD RETIUFD RETINCD */
		/* RETICD RETINLVD RETILVD RETINLUFD RETILUFD RETIZUFD */

%token	<mnem>	RETScond
	 	/* RETSU RETSLO RETSLS RETSHI RETSHS RETSEQ RETSNE RETSL */
		/* RETSLE RETSGT RETSGE RETSZ RETSNZ RETSP RETSN RETSNN */
		/* RETSNV RETSV RETSNUF RETSUF RETSNC RETSC RETSNLV RETSLV */
		/* RETSNLUF RETSLUF RETSZUF */

%token	<mnem>	TRAPcond
		/* TRAPU TRAPLO TRAPLS TRAPHI TRAPHS TRAPEQ TRAPNE TRAPL */
		/* TRAPLE TRAPGT TRAPGE TRAPZ TRAPNZ TRAPP TRAPN TRAPNN */
		/* TRAPNV TRAPV TRAPNUF TRAPUF TRAPNC TRAPC TRAPNLV TRAPLV */
		/* TRAPNLUF TRAPLUF TRAPZUF */


/* INTERLOCK INSTRUCTIONS */
/* ====================== */

%token	<mnem>	LDFI LDII SIGI STFI STII


/* STACK INSTRUCTIONS */
/* ================== */

%token	<mnem>	POPF POP PUSH PUSHF 

/* MISC INSTRUCTIONS */
/* ================= */

%token	<mnem>	IDLE SWI NOP IACK



/* PARALLEL INSTRUCTIONS */
/* ===================== */

/* use standard tokens defined for diadic and triadic instructions */

/*
		ABSF || STF,	ABSI || STI,	ADDF3 || STF,	ADDI3 || STI
		AND3 || STI,	ASH3 || STI,	FIX || STI,	FLOAT || STF
		FRIEEE || STF,	LDF || STF,	LDI || STI,	LSH3 || STI
		MPYF3 || STF,	MPYI3 || STI,	NEGF || STF,	NEGI || STI
		NOT3 || STI,	OR3 || STI,	SUBF3 || STF,	TOIEEE || STF
		SUBI3 || STI,	XOR3 || STI,	LDF || LDF,	LDI || LDI
		MPYF3 || ADDF3,	MPYF3 || SUBF3,	MPYF3 || ADDI3,	MPYF3 || SUBI3
*/


/* ******************************************************************** */
/* C40 Specific Types							*/
/* ******************************************************************** */

%token	<flt>	C40FLOAT


/* ******************************************************************** */
/* C40 specific linker instruction patch tokens				*/
/* ******************************************************************** */

%token	<num>	PATCHC40DATAMODULE1 PATCHC40DATAMODULE2 PATCHC40DATAMODULE3
%token	<num>	PATCHC40DATAMODULE4 PATCHC40DATAMODULE5
%token	<num>	PATCHC40MASK24ADD PATCHC40MASK16ADD PATCHC40MASK8ADD


/* @@@ Maybe add: bitwise OR in to immediate data area (5 bit)  */
/* @@ maybe adjust labelref patches depending on delayed bit -1/-3 + pcrel */


/* ******************************************************************** */
/* C40 Register Tokens							*/
/* ******************************************************************** */

%token	<num>	R0 R1 R2 R3 R4 R5 R6 R7 R8 R9 R10 R11
%token	<num>	AR0 AR1 AR2 AR3 AR4 AR5 AR6 AR7
%token	<num>	DP IR0 IR1 BK SP ST DIE IIE IIF RS RE RC

%token	<num>	IVTP TVTP

%type	<num>	reg Dreg Dreg0_1 Dreg2_3 Dreg0_7 Areg Addr_reg Exp_reg


/* ******************************************************************** */
/* Misc C40 Tokens							*/
/* ******************************************************************** */

%token		BARBAR



/* ******************************************************************** */
/* Internal rule return types						*/
/* ******************************************************************** */

%type	<num>	indirect prefix postfix displacement
%type	<expr>	direct immediate pcrel
%type	<num>	fp_immediate /* 16 bit C40 float representation */
%type	<num>	int_ld_Addr_reg_modes st_addr_modes
%type	<num>	int_unary_op_mode fp_unary_op_mode
%type	<num>	int_diadic_modes fp_diadic_modes float_diadic_modes
%type	<num>	int_triadic_modes fp_triadic_modes
%type	<num>	dia_par_sti_mode tri_par_sti_mode shift_tri_par_sti_mode
%type	<num>	dia_par_stf_mode tri_par_stf_mode shift_tri_par_stf_mode	
%type	<num>	par_mpyi_mode par_mpyf_mode
%type	<num>	rev_par_mpyi_mode rev_par_mpyf_mode
%type	<num>	par_mpyf_op2 par_mpyi_op2

%type	<mnem>	mpyi2_3 mpyf2_3 fp_loads part_loads interlocked_loads genstores
%type	<mnem>	tri_swap_par_sti dia_swap_par_sti shift_swap_par_sti
%type	<mnem>	tri_swap_par_stf dia_swap_par_stf shift_swap_par_stf
%type	<mnem>	bigbranch_ops condbranch fp_unary_only_ops int_diadic_only_ops
%type	<mnem>	int_triadic_only_ops int_both_2_3_ops int_par_st_unary_ops
%type	<mnem>	fp_par_st_unary_ops int_par_st_3_ops shift_par_st_3_ops
%type	<mnem>	int_par_st_2_3_ops shift_par_st_2_3_ops
%type	<mnem>	rotate_ops recip_ops cmpi_ops cmpi3_ops
%type	<mnem>	ret_ops trap_ops rptb_ops

%type	<num>	c40patches


/* ******************************************************************** */
/* toks_C40.ypp */




/* ******************************************************************** */
/* Parsing starts at the rule defined here				*/

%start statement


/* ******************************************************************** */
%%		/* YACC Grammer definition section			*/
/* ******************************************************************** */


statement:	/* empty */
		| statement {StartLine = CurLine;} label
		| statement {StartLine = CurLine;} mnemonic
		| statement error
		{
			yyclearin;	/* clear read ahead tokens */
			yyerrok;	/* allow syntax errors to restart */
			ClearInput();	/* clear all input up to next \n */
		}
		;


/* Add label to symbol table and parse tree */
label:		LABEL
		{
			/* if symbol already defined, raise an error */
			if (FindSymb($1)) {
				char	Err[128];

				sprintf(Err, "redefinition of label \"%s\"", $1);
				Error(Err);
				/* and continue parsing... */
			}
			else {
				/* Add label to symbol table */

				/* Naff word address machine */
				Symbol *s = NewSymb($1, 0	, curPC / 4);


				/* add symbol table ref to parse tree */
				/* this will output an object code label */
				ParseTreeItem *pti = NewParseTreeItem(LABEL);

				pti->type.symb = s;


				/* check alignment of label */
				if (curPC % 4 != 0) {
					char Err[128];

					sprintf(Err, "label \"%s\" is not aligned to a  word address", $1);
					Error(Err);
				}

			}
		}
		;


mnemonic:	pseudo_op
		| machine_op
		;


/* Pseudo opcodes supported: */
pseudo_op:
		byte
		| short
		| word
		| blockbyte
		| blockshort
		| blockword
		| double
		| floaty

		| c40float

		| space
		| align
		| data
		| common
		| codetable
		| export
		| import
		| module
		| init
		| ref
		| patchinstr
		| usererror
		| userwarning
		| usernote
		;


/* ******************************************************************** */
/* Code area directives							*/

/* append item to parse tree with a pointer to the code constant list */
/* returned by the 'codeconstlist' rule */
byte:		BYTE codeconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(BYTE);

			/* insert ref to codeconstlist items */
			pti->type.clist = $2;
			curPC += list_size_count * sizeof(char);
		}
		;

short:		SHORT codeconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(SHORT);

			pti->type.clist = $2;
			curPC += list_size_count * (2 * sizeof(char));
		}
		;

word:		WORD codeconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(WORD);

			pti->type.clist = $2;
			curPC += list_size_count * sizeof(int);
		}
		;

floaty:		FLOATY floatconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(FLOATY);

			pti->type.flist = $2;
			curPC += list_size_count * 4; /* 32 bit float */
		}
		;

double:		DOUBLE floatconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(DOUBLE);

			pti->type.flist = $2;
			curPC += list_size_count * 8; /* 64 bit float */
		}
		;


c40float:	C40FLOAT floatconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(C40FLOAT);

			pti->type.flist = $2;
			curPC += list_size_count * 4; /* 32 bit float */
		}
		;



/* add blockbyte to the parse tree as a byte constlist item and a space item */
/* to pad out the remaining space in the block */
blockbyte:	BLOCKBYTE imm_constexpr ',' codeconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(BYTE);
			int remain = $2 - list_size_count;

			pti->type.clist = $4;

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

			curPC += $2 * sizeof(char);
		}
		;

blockshort:	BLOCKSHORT imm_constexpr ',' codeconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(SHORT);
			int remain = $2 - list_size_count;

			pti->type.clist = $4;

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

			curPC += $2 * (2 * sizeof(char));
		}
		;

blockword:	BLOCKWORD imm_constexpr ',' codeconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(WORD);
			int remain = $2 - list_size_count;

			pti->type.clist = $4;

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

			curPC += $2 * sizeof(int);
		}
		;


/* add space directive to the parse tree. The imm_constexpr rule returns an */
/* integer result from the constant expression entered in the source file */
space:		SPACE imm_constexpr
		{
			/* add space directive to parse tree */
			ParseTreeItem *pti = NewParseTreeItem(SPACE);

			pti->type.num = $2;

			curPC += $2;
		}
		;


/* align address, possibly adding space directive to parse tree to pad to */
/* the new aligned address */
align:		ALIGN opt_imm_constexpr
		{
			ParseTreeItem *pti;
			int pad;

			/* if alignment not defined default to 4 */
			pad = (((( curPC ) + (  ($2) ? $2 : 4 ) - 1) & ~((  ($2) ? $2 : 4 ) - 1)) -  curPC ) ;

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
		;


/* ******************************************************************** */
/* Module data area generation directives				*/

/* add static data defn. directive to parse tree */
data:		DATA NAME ',' constexpr
		{
			ParseTreeItem *pti = NewParseTreeItem(DATA);

			pti->type.datacommon.name = $2;
			pti->type.datacommon.expr = $4;


		}
		;

/* add static common defn. directive to parse tree */
common:		COMMON NAME ',' constexpr
		{
			ParseTreeItem *pti = NewParseTreeItem(COMMON);

			pti->type.datacommon.name = $2;
			pti->type.datacommon.expr = $4;


		}
		;

/* add function defn. directive to parse tree */
codetable:	CODETABLE namelist
		{
			ParseTreeItem *pti = NewParseTreeItem(CODETABLE);

			pti->type.nlist = $2;


		}
		;


/* add function defn. directive to parse tree */
export:		EXPORT namelist
		{
			ParseTreeItem *pti = NewParseTreeItem(EXPORT);

			pti->type.nlist = $2;


		}
		;


/* add function label import directive to parse tree */
import:		IMPORT namelist
		{
			/* force a linker global directive to be emited */
			ParseTreeItem *pti = NewParseTreeItem(IMPORT);
			NameList *n = $2;

			pti->type.nlist = $2;

			/* add imported label to the symbol table */
			while ( n != ((void *)  0) ) {
				NewSymb(n->name, 4	, 0);
				n = n->next;
			}

		}
		;


/* ******************************************************************** */
/* Misc object code directives						*/

/* add module start directive to parse tree */
module:		MODULE opt_constexpr
		{
			ParseTreeItem *pti = NewParseTreeItem(MODULE);

			/* if no module number expression, default to -1 */
			pti->type.expr = ($2) ? $2 : NewExprNum(-1);


		}
		;

/* add initialisation chain directive to parse tree */
init:		INIT
		{
			NewParseTreeItem(INIT);

			/* init parse item needs to remember no data */


			curPC += sizeof(int);
		}
		;

/* add library reference to parse tree */
ref:		REF namelist
		{
			ParseTreeItem *pti = NewParseTreeItem(REF);

			pti->type.nlist = $2;


		}
		;

/* add instruction patch to parse tree */
patchinstr:	PATCHINSTR '(' instrpatch ',' stdpatch ',' machine_op ')'
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
				LastParseTreeItem->type.patchinstr.mcpatch = $3;
				LastParseTreeItem->type.patchinstr.patch = $5;

				/* re-insert instruction struct pointer */
				LastParseTreeItem->type.patchinstr.instr =instr;

			}
			else
				Error("patchinstr failed due to instruction syntax error");
		}
		;


/* ******************************************************************** */
/* Message Output							*/

usererror:	USERERROR STRINGCONST
		{
			Error($2);
		}
		;

userwarning:	USERWARNING STRINGCONST
		{
			Warn($2);
		}
		;

usernote:	USERNOTE STRINGCONST
		{
			Note($2);
		}
		;


/* ******************************************************************** */
/* Standard patches 							*/

/* These patches are available in all linkers conforming to GHOF. They	*/
/* return linker calculated data such as the module size and module	*/
/* number. This information can then be placed into the code as		*/
/* contants, or combined with an instruction patch to place the data	*/
/* directly into an instruction.					*/

stdpatch :	MODSIZE
		{
			$$ = NewPatch(MODSIZE);			/* 0x0e */

		}
		| MODNUM
		{
			$$ = NewPatch(MODNUM);			/* 0x012 */

		}
		| DATASYMB '(' NAME ')'
		{
			$$ = NewPatch(DATASYMB);		/* 0x010 */
			$$->type.name = $3;

		}
		| CODESYMB '(' NAME ')'
		{
			$$ = NewPatch(CODESYMB);		/* 0x01d */
			$$->type.name = $3;

		}
		| DATAMODULE '(' NAME ')'
		{
			$$ = NewPatch(DATAMODULE);		/* 0x011 */
			$$->type.name = $3;

		}
		| LABELREF '(' NAME ')'
		{
			/* @@@ give warning if label is not in this module */
			/* @@@ during second pass */
			$$ = NewPatch(LABELREF);		/* 0x01f */
			$$->type.name = $3;


		}

		| CODESTUB '(' NAME ')'
		{
			$$ = NewPatch(CODESTUB);		/* 0x028 */
			$$->type.name = $3;


		}

		| ADDRSTUB '(' NAME ')'
		{
			$$ = NewPatch(ADDRSTUB);		/* 0x029 */
			$$->type.name = $3;


		}

		/* prefix patches (patches that patch patches!) */
		| BYTESWAP '(' stdpatch ')'
		{
			$$ = NewPatch(BYTESWAP);		/* 0x01e */
			$$->type.patch = $3;


		}
		| SHIFT '(' constexpr ',' stdpatch ')'
		{
			$$ = NewPatch(SHIFT);			/* 0x014 */
			$$->type.shift.expr = $3;
			$$->type.shift.patch = $5;


		}
		| P_ADD '(' constexpr ',' stdpatch ')'
		{
			$$ = NewPatch(P_ADD);			/* 0x013 */
			$$->type.shift.expr = $3;
			$$->type.shift.patch = $5;


		}
		| P_OR '(' constexpr ',' stdpatch ')'
		{
			$$ = NewPatch(P_OR);			/* 0x01f */
			$$->type.shift.expr = $3;
			$$->type.shift.patch = $5;


		}
		;


/* ******************************************************************** */
/* Name list processing	(left recursive)				*/

namelist:	NAME
		{
			/* first or only item in list */
			NameList *n = (NameList *)malloc(sizeof(NameList));

			if (n == ((void *)  0) )
				Fatal("Out of memory whilst building name list");

			n->next = ((void *)  0) ;
			n->name = $1;
			$$ = n;
		}
		| namelist ',' NAME
		{
			/* list of names */
			NameList *n = $1;
			NameList *nn = (NameList *)malloc(sizeof(NameList));

			if (nn == ((void *)  0) )
				Fatal("Out of memory whilst building name list");

			/* insert name list item - order is not important */
			/* so dont search to end of list */
			nn->next = n->next;
			n->next = nn;
			nn->name = $3;

			$$ = n;	/* return start of namelist */
		}
		;


/* ******************************************************************** */
/* Code constant list processing					*/

/* codeconstlist chains together codeconst items returned by the	*/
/* codeconst rule (expressions, patches and strings)			*/
/* left recursive */

codeconstlist:	{ list_size_count = 0; }	/* must count as $1 arg! */
		codeconst
		{
			$$ = $2;	/* first or only item in list */
		}
		| codeconstlist ',' codeconst
		{
			ConstList *n = $1;	/* start of list */

			/* search for end of list */
			while (n->next != ((void *)  0) )
				n = n->next;

			n->next = $3;	/* append item to end of list */	

			$$ = $1;	/* returns start of constlist */
		}
		;


floatconstlist:	{ list_size_count = 0; }	/* must count as $1 arg! */
		imm_fp_constexpr
		{
			/* first or only item in list */
			$$ = NewFloatConstItem($2);
			list_size_count++;
		}
		| floatconstlist ',' imm_fp_constexpr
		{
			FloatList *fl = $1;	/* start of list */

			/* search for end of list */
			while (fl->next != ((void *)  0) )
				fl = fl->next;
			/* append item to end of list */
			fl->next = NewFloatConstItem($3);
			list_size_count++;

			$$ = $1;	/* returns start of floatconstlist */
		}
		;


/* codeconst returns items that are then chained into a codeconst list */

codeconst:	constexpr
		{
			/* return an Expression constlist item */
			$$ = NewConstItem(	1 );

			$$->type.expr = $1;
			list_size_count++;
		}
		| stdpatch
		{
			/* return a patch constlist item */
			$$ = NewConstItem(	3 );

			$$->type.patch = $1;
			list_size_count++;
		}
		| STRINGCONST
		{
			/* return a string constlist item */
			$$ = NewConstItem(	4		);

			$$->type.str = $1;
			/* increment the list size count by the number of */
			/* chars in the string - 1 counted by default already */

			list_size_count += strlen($1);

		}
		;


/* ******************************************************************** */
/* Constant expressions							*/

/* Immediately evaluated constant expression (DOESN'T allow symbols)	*/
/* This rule immediately returns the integer result of any valid	*/
/* expression found in the source at this point.			*/


/* ******************************************************************** */
/* Constant expressions							*/

/* Immediately evaluated constant expression (DOESN'T allow symbols)	*/
/* This rule immediately returns the integer result of any valid	*/
/* expression found in the source at this point.			*/

imm_constexpr:	'(' imm_constexpr ')'
		{	$$ = $2; }
		| imm_constexpr '+' imm_constexpr
		{	$$ = $1 + $3; }
		| imm_constexpr '-' imm_constexpr
		{	$$ = $1 - $3; }
		| imm_constexpr '*' imm_constexpr
		{	$$ = $1 * $3; }
		| imm_constexpr '/' imm_constexpr
		{	$$ = $1 / $3; }
		| imm_constexpr '%' imm_constexpr
		{	$$ = $1 % $3; }
		| imm_constexpr '&' imm_constexpr
		{	$$ = $1 & $3; }
		| imm_constexpr '|' imm_constexpr
		{	$$ = $1 | $3; }
		| imm_constexpr '^' imm_constexpr
		{	$$ = $1 ^ $3; }
		| imm_constexpr LSHIFT imm_constexpr
		{	$$ = $1 << $3; }
		| imm_constexpr RSHIFT imm_constexpr
		{	$$ = $1 >> $3; }
		| '~' imm_constexpr %prec UNARY
		{	$$ = ~ ($2); }
		| '-' imm_constexpr %prec UNARY
		{	$$ = - ($2); }

		/* just return value of number */
		| NUMBER

		/* just return value of character constant */
		| CHARCONST

		;


/* Floating point immediate expression.					*/
/* This rule immediately returns the integer result of any valid	*/
/* expression found in the source at this point.			*/

imm_fp_constexpr: '(' imm_fp_constexpr ')'
		{	$$ = $2; }

		| imm_fp_constexpr '+' imm_fp_constexpr
		{	if (!flt_add (&$$, &$1, &$3))
				Warn ("overflow: floating point addition") ; }
		| imm_fp_constexpr '-' imm_fp_constexpr
		{	if (!flt_subtract (&$$, &$1, &$3))
				Warn ("overflow: floating point subtraction") ; }
		| imm_fp_constexpr '*' imm_fp_constexpr
		{	if (!flt_multiply (&$$, &$1, &$3))
				Warn ("overflow: floating point multiplication") ; }
		| imm_fp_constexpr '/' imm_fp_constexpr
		{	if (!flt_divide (&$$, &$1, &$3))
				Warn ("overflow: floating point division") ; }
		| '-' imm_fp_constexpr %prec UNARY
		{	(void) flt_negate (&$$, &$2); }

		/* just return value of number */
		| NUMBER
		{ (void) flt_itod (&$$, $1);}

		/* just return value of number */
		| FLOATNUM
		;


/* optional immediate expression (used by align) */
opt_imm_constexpr:	/* empty */
		{

			$$ = 0;
		}
		| imm_constexpr
		;


/* Delayed evaluation form of constant expression. This allows the	*/
/* inclusion of symbol references in expressions. constexpr builds a	*/
/* binary evaluation tree of Expression items. This describes the	*/
/* operands, operators and order of evaluation (precidence). During the	*/
/* second pass, when all valid labels are guaranteed to have been set,	*/
/* the Eval() fn is used to return an integer result from the tree.	*/

constexpr:	'(' constexpr ')'
		{	$$ = $2; }
		| constexpr '+' constexpr
		{	$$ = NewExpr($1, '+', $3); }
		| constexpr '-' constexpr
		{	$$ = NewExpr($1, '-', $3); }
		| constexpr '*' constexpr
		{	$$ = NewExpr($1, '*', $3); }
		| constexpr '/' constexpr
		{	$$ = NewExpr($1, '/', $3); }
		| constexpr '%' constexpr
		{	$$ = NewExpr($1, '%', $3); }
		| constexpr '&' constexpr
		{	$$ = NewExpr($1, '&', $3); }
		| constexpr '|' constexpr
		{	$$ = NewExpr($1, '|', $3); }
		| constexpr '^' constexpr
		{	$$ = NewExpr($1, '^', $3); }
		| constexpr LSHIFT constexpr
		{	$$ = NewExpr($1, LSHIFT, $3); }
		| constexpr RSHIFT constexpr
		{	$$ = NewExpr($1, RSHIFT, $3); }
		| '~' constexpr %prec UNARY
		{	$$ = NewExpr(((void *)  0) , '~', $2); }
		| '-' constexpr %prec UNARY
		{	$$ = NewExpr(((void *)  0) , '-', $2); }
		| NUMBER
			/* return value of number into evaluation tree */
		{	$$ = NewExprNum($1);}
		| CHARCONST
			/* return integer value of character constant */
		{	$$ = NewExprNum($1);}
		| NAME
			/* return symbol reference */
		{	$$ = NewExprSymbRef($1);}
		;


/* optional expression (used by module) */
opt_constexpr:	/* empty */
		{
			/* code executed if no optional constexpr in source */


			$$ = ((void *)  0) ;
		}
		| constexpr
		;




/* include CPU specific mneumonic and patch rules */


/*
 * File:	rules_C40.ypp
 * Subsystem:	Generic (C40) Assembler
 * Author:	P.A.Beskeen
 * Date:	Sept '91
 *
 * Description: YACC grammer rules and actions to implement the TMS320C40
 *		specific parts of the parser.
 *
 *
 * RcsId: $Id: rules_C40.ypp,v 1.9 1994/01/18 16:21:07 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */

/*
 * The rules define the syntax of acceptable input, enabling the parser to
 * identify individual instructions, operands and addressing modes. If the
 * input does not match these rules, then a syntax error is generated and
 * the parser will attempt to recover and find the next instruction to parse.
 *
 * The actions associated with the rules add Instruction type parse items
 * into the parse tree, incrementing the logical program counter (curPC)
 * as they do so. These items are then used by the second pass CPU
 * specific module to place binary instructions into the object code.
 *
 */


/* ******************************************************************** */
/* 'C40 CPU addressing modes:						*/
/* ******************************************************************** */


/* *** REGISTER ADDRESSING ********************************************	*/

		/* used in diadic instructions */
reg:		R0 | R1 | R2 | R3 | R4 | R5 | R6 | R7 | R8 | R9 | R10 | R11
		| AR0 | AR1 | AR2 | AR3 | AR4 | AR5 | AR6 | AR7
		| DP | IR0 | IR1 | BK | SP | ST | DIE | IIE | IIF | RS | RE | RC
		| error
		{	Error("Expecting a register");	}
		;

		/* used in triadic and fp load instructions */
Dreg:		R0 | R1 | R2 | R3 | R4 | R5 | R6 | R7 | R8 | R9 | R10 | R11
		| error
		{	Error("Expecting a register (R0-R11)");	}
		;

		/* used as first destination register in // instructions */
Dreg0_1:	R0 | R1
		| error
		{	Error("Expecting register R0 or R1");	}
		;

		/* used as second destination register  in // instructions */
Dreg2_3:	R2 | R3
		| error
		{	Error("Expecting register R2 or R3");	}
		;

		/* used for source arguments in // instructions */
Dreg0_7:	R0 | R1 | R2 | R3 | R4 | R5 | R6 | R7
		| error
		{	Error("Expecting a register R0-R7");	}
		;

		/* used in indirect addressing */
Areg:		AR0 | AR1 | AR2 | AR3 | AR4 | AR5 | AR6 | AR7

		;

		/* used in LDA's addressing */
Addr_reg:	AR0 | AR1 | AR2 | AR3 | AR4 | AR5 | AR6 | AR7 |
		IR0 | IR1 | DP | BK | SP
		| error
		{	Error("Unknown address register");	}
		;

		/* used by LDPE and LDEP */
Exp_reg:	IVTP | TVTP
		| error
		{	Error("Unknown expansion file register");	}
		;


/* *** DIRECT ADDRESSING  *********************************************	*/

		/* used in diadic instructions */
direct:		'@' constexpr
		{
			if (CurInstr->combine) {
				/* default to diadic direct addr. */
				CurInstr->combine2 = 	5 ;
				$$ = CurInstr->optexpr2 = $2;
			}
			else {
				/* default to diadic direct addr. */
				CurInstr->combine = 	5 ;
				$$ = CurInstr->optexpr = $2;
			}
		}
		;


/* *** IMMEDIATE ADDRESSING *******************************************	*/

/* Second pass will check that immediate will fit in 8 bits or 16 bits	*/
/* depending on type of instruction.					*/
/* 	Type 2, triadic instructions = 8 bits				*/
/* 	Diadic instructions = 16 bits					*/

immediate:	constexpr
		{
			if (CurInstr->combine) {
				/* default to diadic 16 bit immediate */
				CurInstr->combine2 = 	1 ;
				$$ = CurInstr->optexpr2 = $1;
			}
			else {
				/* default to diadic 16 bit immediate */
				CurInstr->combine = 	1 ;
				$$ = CurInstr->optexpr = $1;
			}
		}
		;


fp_immediate:	imm_fp_constexpr
		{
			$$ = (int) IEEE_64ToC40_16($1);
		}
		;


/* *** INDIRECT ADDRESSING ********************************************	*/

/*
 * Indirect addressing in diadic instructions may use the full range
 * of pre/post inc/decrements and have an indirection of up to eight bits.
 *
 * Indirect addressing for triadic type 2 instructions can only use
 * the forms *ARn | *+ARn(disp), with a displacement that fits into 5 bits.
 *
 * In triadic type 1, and parallel instructions
 * all forms allowed, but displacement is limited to 0 or 1. One is always
 * assumed but zero can be achieved by not using inc/dec/+/- operators.
 *
 * When the code is being output in the second pass, the displacement
 * expressions are evaluated and invalid indirection displacement sizes
 * will raise errors.
 *
 * For an explanation of indirect addressing and the format of its sub fields
 * see page 5.5 - "Types of Addressing" section in the 'C40 Users Guide
 */

		/* Returns concatenated mod and ARn fields */
indirect:	'*' Areg
			/* assumes 0 displacement */
		{	$$ = 	(( 0x18  ) << 3)  | (( $2 ) - 	0x08 ) ;	}
		| '*' prefix Areg displacement
		{	$$ = 	(( $2 | $4 ) << 3)  | (( $3 ) - 	0x08 ) ;	}
		| '*' Areg postfix
		{	$$ = 	(( $3 ) << 3)  | (( $2 ) - 	0x08 ) ;		}
		| '*' error
		{
			Error("format of indirection is incorrect");
		}
		;

/* pre-inc/decrement operators */
prefix:		/* Return operator sub field of mod */
		'+'
		{	$$ = 0x0 ;		}
		| '-'
		{	$$ = 0x1 ;		}
		| PLUSPLUS
		{	$$ = 0x2 ;		}
		| MINUSMINUS
		{	$$ = 0x3 ;		}
		;

postfix:	/* Returns mod = disp selector sub field | operator sub field */
		PLUSPLUS '(' IR0 ')' bitreverse
		{	$$ = 0x19 ;		}
		| PLUSPLUS displacement
		{	$$ = $2 | 0x4 ;	}
		| MINUSMINUS displacement
		{	$$ = $2 | 0x5 ;	}
		| PLUSPLUS displacement '%'
		{	$$ = $2 | 0x6 ;	}
		| MINUSMINUS displacement '%'
		{	$$ = $2 | 0x7 ;	}
		;

/* Diadic = 8 bit displacement						*/
/* Triadic type 2 = 5 bit displacement.					*/
/* Triadic type 1 and parallel instruction  =  assumed			*/
/* 	displacement of	1 or 0 only.					*/
/*									*/
/* These constraints must be check by the second pass.			*/

displacement:	/* empty */
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
			$$ = 0; /* $$ = MOD_DISP(B_00); - defaults to this */
		}
		| '(' constexpr ')'
		{
			if (CurInstr->combine) {
				CurInstr->optexpr2 = $2;
				CurInstr->combine2 = 	4 ;
			}
			else {
				CurInstr->optexpr = $2;
				CurInstr->combine = 	4 ;
			}
			$$ = 0; /* $$ = MOD_DISP(B_00); - defaults to this */
		}
		| '(' IR0 ')'
		{	$$ = (( 0x1  ) << 3) ;	}
		| '(' IR1 ')'
		{	$$ = (( 0x2  ) << 3) ;	}
		;

		/* check for bit reversal addressing specifier character 'B' */
bitreverse:	NAME
		{
			if (!(strcmp("BIT", $1) == 0 || strcmp("bit", $1) == 0)) {
				char err[128];
				
				strcpy(err, "'");
				strcat(err, $1);
				strcat(err, "' is not a valid bitreversal addressing specifier - use 'bit'");
				Error(err);
			}
		}
		;


/* *** PC RELATIVE ADDRESSING  ****************************************	*/

pcrel:		/* used in branches/jumps/calls */
		constexpr
		{
			/* default to 16 bit cond branch */
			CurInstr->combine = 17	;
			$$ = CurInstr->optexpr = $1;
		}
		| '@' constexpr
		{
			/* default to 16 bit cond. branch */
			CurInstr->combine = 17	;
			$$ = CurInstr->optexpr = $2;
		}
		;



/* ******************************************************************** */
/* C40 CPU addressing mode groups:					*/
/* ******************************************************************** */

/* ********************************************************************	*/
/* *** Addressing Modes For Load Instructions *************************	*/


/* *** NORMAL LOADS ***************************************************	*/

int_ld_Addr_reg_modes:
		reg ',' Addr_reg
		{	$$ = 		( 0x0   << 21)					 | (( $3 ) << 16)		 | ( $1 )			; }
		| indirect ',' Addr_reg
		{	$$ = 		( 0x2   << 21)					 | (( $3 ) << 16)		 | (( $1 ) << 8)		; }
		| direct ',' Addr_reg
		{	$$ = 		( 0x1   << 21)					 | (( $3 ) << 16)		; }
		| immediate ',' Addr_reg
		{	$$ = 		( 0x3   << 21)					 | (( $3 ) << 16)		; }
		| error
		{	Error("illegal addressing mode for an integer load");	}
		;


/* ********************************************************************	*/
/* *** Addressing Modes For Store Instructions ************************	*/

st_addr_modes:
		reg ',' direct
		{	$$ = 		( 0x1   << 21)					 | (( $1 ) << 16)		; }
		| reg ',' indirect
		{	$$ = 		( 0x2   << 21)					 | (( $1 ) << 16)		 | (( $3 ) << 8)		; }
		| reg ',' error
		{
			Error("illegal addressing mode for store instruction");
		}
		;

/* ********************************************************************	*/
/* *** Logical and Arithmetic Addressing Mode Groups ******************	*/


/* *** INTEGER UNARY ADDRESSING MODE **********************************	*/
int_unary_op_mode:
		reg
			/* duplicate register for both src and dst */
		{	$$ = 		( 0x0   << 21)					 | (( $1 ) << 16)		 | ( $1 )			; }
		;


/* *** FP UNARY ADDRESSING MODE ***************************************	*/
fp_unary_op_mode:
		Dreg
			/* duplicate register for both src and dst */
		{	$$ = 		( 0x0   << 21)					 | (( $1 ) << 16)		 | ( $1 )			; }
		;


/* *** INTEGER LOAD and DIADIC ADDRESSING MODES ***********************	*/
int_diadic_modes:
		reg ',' reg
		{	$$ = 		( 0x0   << 21)					 | (( $3 ) << 16)		 | ( $1 )			; }
		| direct ',' reg
		{	$$ = 		( 0x1   << 21)					 | (( $3 ) << 16)		; }
		| indirect ',' reg
		{	$$ = 		( 0x2   << 21)					 | (( $3 ) << 16)		 | (( $1 ) << 8)		; }
		| immediate ',' reg
		{	$$ = 		( 0x3   << 21)					 | (( $3 ) << 16)		; }
		;


/* *** FP LOAD and DIADIC ADDRESSING MODES ****************************	*/
fp_diadic_modes:
		Dreg ',' Dreg
		{	$$ = 		( 0x0   << 21)					 | (( $3 ) << 16)		 | ( $1 )			; }
		| direct ',' Dreg
		{	$$ = 		( 0x1   << 21)					 | (( $3 ) << 16)		; }
		| indirect ',' Dreg
		{	$$ = 		( 0x2   << 21)					 | (( $3 ) << 16)		 | (( $1 ) << 8)		; }
		| fp_immediate ',' Dreg	/* @@@ should allow float const exprs */
		{	$$ = 		( 0x3   << 21)					 | $1 | (( $3 ) << 16)		; }
		;


/* *** FLOAT ADDRESSING MODES *****************************************	*/
float_diadic_modes:
		reg ',' Dreg
		{	$$ = 		( 0x0   << 21)					 | (( $3 ) << 16)		 | ( $1 )			; }
		| direct ',' Dreg
		{	$$ = 		( 0x1   << 21)					 | (( $3 ) << 16)		; }
		| indirect ',' Dreg
		{	$$ = 		( 0x2   << 21)					 | (( $3 ) << 16)		 | (( $1 ) << 8)		; }
		| immediate ',' Dreg	/* @@@ should allow float const exprs */
		{	$$ = 		( 0x3   << 21)					 | (( $3 ) << 16)		; }
		;


/* *** INTEGER TRIADIC ADDRESSING MODES *******************************	*/
int_triadic_modes:
		/* Triadic / type 1 ***********************************	*/

		reg ',' reg ',' reg
		{
			$$ = (0			 | 	(( 0x0  ) << 21)		)  | ( $1 )		 | (( $3 ) << 8)	 | (( $5 ) << 16)		;
		}
		|  reg ',' indirect ',' reg
		{
			$$ = (0			 | 	(( 0x1  ) << 21)		)  | ( $1 )		 | (( $3 ) << 8)	 | (( $5 ) << 16)		;
			CurInstr->combine = 	8	;
		}

		/* Triadic / type 2 ***********************************	*/

		| immediate ',' reg ',' reg
		{
			$$ = ((1 << 28)		 | 	(( 0x0  ) << 21)		)  | (( $3 ) << 8)	 | (( $5 ) << 16)		;
			CurInstr->combine = 	11 ;
		}

		| immediate ',' indirect ',' reg
		{
			$$ = ((1 << 28)		 | 	(( 0x2  ) << 21)		)  | (( $3 ) << 8)	 | (( $5 ) << 16)		;
			CurInstr->combine = 	11 ;
			if (!((( $3 ) & 0xf8 ) == 0x0 ) ) {
				Error("Only *+ARn format indirections allowed for type 2 triadic instructions");
			}
			CurInstr->combine2 = 	10	;
		}


		/* Triadic / type 1 or 2 ******************************	*/
		/* Find out which type is being used and generate an	*/
		/* instruction template accordingly.			*/
		/* Second pass will check displacement sizes and insert	*/
		/* displacement into instruction.			*/

		| indirect ',' reg ',' reg
		{
			if (((( $1 ) & 0xf8 ) == 0x0 ) ) {
				$$ = ((1 << 28)		 | 	(( 0x1  ) << 21)		)  | ( $1 )		 | (( $3 ) << 8)	 | (( $5 ) << 16)		;
				CurInstr->combine = 	9	;
			}
			else {
				$$ = (0			 | 	(( 0x2  ) << 21)		)  | ( $1 )		 | (( $3 ) << 8)	 | (( $5 ) << 16)		;
				CurInstr->combine = 	7	;
			}
		}
		| indirect ',' indirect ',' reg
		{
			if (((( $1 ) & 0xf8 ) == 0x0 )  && ((( $3 ) & 0xf8 ) == 0x0 ) ) {
				$$ = ((1 << 28)		 | 	(( 0x3  ) << 21)		)  | ( $1 )		 | (( $3 ) << 8)	 | (( $5 ) << 16)		;
				CurInstr->combine = 	9	;
				CurInstr->combine2 = 	10	;
			}
			else {
				$$ = (0			 | 	(( 0x3  ) << 21)		)  | ( $1 )		 | (( $3 ) << 8)	 | (( $5 ) << 16)		;
				CurInstr->combine = 	7	;
				CurInstr->combine2 = 	8	;
			}
		}
		;




/* *** FP TRIADIC ADDRESSING MODES ************************************	*/
fp_triadic_modes:	
		/* Triadic / type 1 ***********************************	*/
		Dreg ',' Dreg ',' Dreg
		{
			$$ = (0			 | 	(( 0x0  ) << 21)		)  | ( $1 )		 | (( $3 ) << 8)	 | (( $5 ) << 16)		;
		}
		|  Dreg ',' indirect ',' Dreg
		{
			$$ = (0			 | 	(( 0x1  ) << 21)		)  | ( $1 )		 | (( $3 ) << 8)	 | (( $5 ) << 16)		;
			CurInstr->combine = 	8	;
		}

		/* Triadic / type 2 ***********************************	*/
		/* No type 2 immediate addressing modes allowed for 	*/
		/* fp operations.					*/


		/* Triadic / type 1 or 2 ******************************	*/
		/* Find out which type is being used and generate an	*/
		/* instruction template accordingly.			*/
		/* Second pass will check displacement sizes and insert	*/
		/* displacement into instruction.			*/

		| indirect ',' Dreg ',' Dreg
		{
			if (((( $1 ) & 0xf8 ) == 0x0 ) ) {
				$$ = ((1 << 28)		 | 	(( 0x1  ) << 21)		)  | ( $1 )		 | (( $3 ) << 8)	 | (( $5 ) << 16)		;
				CurInstr->combine = 	9	;
			}
			else {
				$$ = (0			 | 	(( 0x2  ) << 21)		)  | ( $1 )		 | (( $3 ) << 8)	 | (( $5 ) << 16)		;
				CurInstr->combine = 	7	;
			}
		}
		| indirect ',' indirect ',' Dreg
		{
			if (((( $1 ) & 0xf8 ) == 0x0 )  && ((( $3 ) & 0xf8 ) == 0x0 ) ) {
				$$ = ((1 << 28)		 | 	(( 0x3  ) << 21)		)  | ( $1 )		 | (( $3 ) << 8)	 | (( $5 ) << 16)		;
				CurInstr->combine = 	9	;
				CurInstr->combine2 = 	10	;
			}
			else {
				$$ = (0			 | 	(( 0x3  ) << 21)		)  | ( $1 )		 | (( $3 ) << 8)	 | (( $5 ) << 16)		;
				CurInstr->combine = 	7	;
				CurInstr->combine2 = 	8	;
			}
		}
		;


/* *** INTEGER TRIADIC PARALLEL STORE ADDRESSING MODE *****************	*/
/* We have to check the regs are Dreg0-7 by hand as YACC only has a one	*/
/* token look ahead and this causes conflicts with other rules		*/
/* (CHK_Dreg0_7).							*/

tri_par_sti_mode:
		indirect ',' reg ',' reg BARBAR STI Dreg0_7 ',' indirect
		{
			if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  > 7) { Error("Must be register R0-R7"); } ;
			$$ = ( $1 )		 | (( $3 ) << 19)	 | (( $5 ) << 22)	 | (( $8 ) << 16)	 				| (( $10 ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
		| indirect ',' reg BARBAR STI Dreg0_7 ',' indirect
		{
			/* assume src2 and dst1 regs are the same */
			/* Optional syntax #13 */

			if ( $3  > 7) { Error("Must be register R0-R7"); } ;
			$$ = ( $1 )		 | (( $3 ) << 19)	 | (( $3 ) << 22)	 | (( $6 ) << 16)	 				| (( $8 ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
		;


/* *** INTEGER DIADIC PARALLEL STORE ADDRESSING MODE ******************	*/
dia_par_sti_mode:
		indirect ',' reg BARBAR STI Dreg0_7 ',' indirect
		{
			if ( $3  > 7) { Error("Must be register R0-R7"); } ;
			$$ = ( $1 )		 | (( $3 ) << 22)	 | (( $6 ) << 16)	 | (( $8 ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
		;


/* *** SHIFTS/SUB TRIADIC PARALLEL STORE ADDRESSING MODE **************	*/
/* Note that shifts/sub par stores have swapped order of src1 and src2	*/

shift_tri_par_sti_mode:
		reg ',' indirect ',' reg BARBAR STI Dreg0_7 ',' indirect
		{
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $5  > 7) { Error("Must be register R0-R7"); } ;
			$$ = (( $1 ) << 19)	 | ( $3 )		 | (( $5 ) << 22)	 | (( $8 ) << 16)	 				| (( $10 ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
		;


/* *** FP PARALLEL STORE ADDRESSING MODE ******************************	*/
/* We have to check the regs are Dreg0-7 by hand as YACC only has a one	*/
/* token look ahead and this causes conflicts with other rules		*/
/* (CHK_Dreg0_7).							*/

tri_par_stf_mode:
		indirect ',' Dreg ',' Dreg BARBAR STF Dreg0_7 ',' indirect
		{
			if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  > 7) { Error("Must be register R0-R7"); } ;
			$$ = ( $1 )		 | (( $3 ) << 19)	 | (( $5 ) << 22)	 | (( $8 ) << 16)	 				| (( $10 ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
		| indirect ',' Dreg BARBAR STF Dreg0_7 ',' indirect
		{
			/* assume src2 and dst1 regs are the same */
			/* Optional syntax #13 */

			if ( $3  > 7) { Error("Must be register R0-R7"); } ;
			$$ = ( $1 )		 | (( $3 ) << 19)	 | (( $3 ) << 22)	 | (( $6 ) << 16)	 				| (( $8 ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
		;


/* *** FP DIADIC PARALLEL STORE ADDRESSING MODE ***********************	*/
dia_par_stf_mode:
		indirect ',' Dreg BARBAR STF Dreg0_7 ',' indirect
		{
			if ( $3  > 7) { Error("Must be register R0-R7"); } ;
			$$ = ( $1 )		 | (( $3 ) << 22)	 | (( $6 ) << 16)	 | (( $8 ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
		;


/* *** SHIFTS/SUB FP PARALLEL STORE ADDRESSING MODE ***************************	*/
/* Swapped order of operands to normal instructions */

shift_tri_par_stf_mode:
		Dreg ',' indirect ',' Dreg BARBAR STF Dreg0_7 ',' indirect
		{
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $5  > 7) { Error("Must be register R0-R7"); } ;
			$$ = (( $1 ) << 19)	 | ( $3 )		 | (( $5 ) << 22)	 | (( $8 ) << 16)	 				| (( $10 ) << 8)	;
			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
		;


/* *** PARALLEL MPY's and ADD/SUB's ***********************************	*/

/* Construct opcode templates for MPYI/F parallel opcodes. The only 	*/
/* differences between par_mpyi/f are the source registers that are	*/
/* checked. @@@ could probably simplify the parser by inserting		*/
/* a flag that is read by the register check routines that specifies if */
/* the operation is fp or integer? We have to check some of the src	*/
/* regs (Dreg0-7) and dest regs (R0-1, R2-3) by hand as YACC only has a */
/* one token look ahead and this causes conflicts with other rules	*/
/* (CHK_Dreg0_7 / CHK_Dreg0_1 / CHK_Dreg2_3)				*/


/* MPYF || SUB/ADD ****************************************************	*/

par_mpyi_mode:
					indirect ',' indirect ',' reg
		BARBAR	par_mpyi_op2	Dreg0_7 ',' Dreg0_7 ',' Dreg2_3
		{
			/* check registers are valid */
			if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			/* construct address mode and operand fields to */
			/* insert into instruction template */
			/* subtract 2 from Dreg2_3 to make 1 bit reg selector */
			$$ = $7 | 	(	(( 0x0  ) << 24)		)  				| (( $3 ) << 8)	 | ( $1 )		 | (( $5 ) << 23)	 				| (( $8 ) << 16)	 | (( $10 ) << 19)	 | (( $12 - 2 ) << 22)	;

			/* Second pass to check that disp.'s are only 1 or 0 */
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			indirect ',' reg ',' reg
		BARBAR par_mpyi_op2	indirect ',' Dreg0_7 ',' Dreg2_3
		{
			/* check registers are valid */
			if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 | 	(	(( 0x3  ) << 24)		)  				| (( $1 ) << 8)	 | (( $3 ) << 19)	 | (( $5 ) << 23)	 				| ( $8 )		 | (( $10 ) << 16)	 | (( $12 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			reg ',' indirect ',' Dreg0_1
		BARBAR par_mpyi_op2	indirect ',' Dreg0_7 ',' Dreg2_3
		{
			/* same as previous, but allowing for commutative mpy */
			/* Optional syntax #14 */

			if ( $1  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 | 	(	(( 0x3  ) << 24)		)  				| (( $3 ) << 8)	 | (( $1 ) << 19)	 | (( $5 ) << 23)	 				| ( $8 )		 | (( $10 ) << 16)	 | (( $12 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			reg ',' reg ',' Dreg0_1
		BARBAR par_mpyi_op2	indirect ',' indirect ',' Dreg2_3
		{
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $3  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 | 	(	(( 0x2  ) << 24)		)  				| (( $3 ) << 19)	 | (( $1 ) << 16)	 | (( $5 ) << 23)	 				| ( $8 )		 | (( $10 ) << 8)	 | (( $12 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			indirect ',' reg ',' reg
		BARBAR par_mpyi_op2	Dreg0_7 ',' indirect ',' Dreg2_3
		{
			if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 | 	(	(( 0x1  ) << 24)		)  				| (( $1 ) << 8)	 | (( $3 ) << 19)	 | (( $5 ) << 23)	 				| (( $8 ) << 16)	 | ( $10 )		 | (( $12 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			reg ',' indirect ',' Dreg0_1
		BARBAR par_mpyi_op2	Dreg0_7 ',' indirect ',' Dreg2_3
		{
			/* same again but allowing for commutative mpy */
			/* Optional syntax #14 */

			if ( $1  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 | 	(	(( 0x1  ) << 24)		)  				| (( $3 ) << 8)	 | (( $1 ) << 19)	 | (( $5 ) << 23)	 				| (( $8 ) << 16)	 | ( $10 )		 | (( $12 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}

		/* repeated again, but defaulting MPY dest reg to src2 reg */
		/* optional syntax #13 */

		|			indirect ',' reg
		BARBAR par_mpyi_op2	indirect ',' Dreg0_7 ',' Dreg2_3
		{
			if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 | 	(	(( 0x3  ) << 24)		)  				| (( $1 ) << 8)	 | (( $3 ) << 19)	 | (( $3 ) << 23)	 				| ( $6 )		 | (( $8 ) << 16)	 | (( $10 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			reg ',' reg
		BARBAR par_mpyi_op2	indirect ',' indirect ',' Dreg2_3
		{
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 | 	(	(( 0x2  ) << 24)		)  				| (( $3 ) << 19)	 | (( $1 ) << 16)	 | (( $3 ) << 23)	 				| ( $6 )		 | (( $8 ) << 8)	 | (( $10 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			indirect ',' reg
		BARBAR par_mpyi_op2	Dreg0_7 ',' indirect ',' Dreg2_3
		{
			if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 | 	(	(( 0x1  ) << 24)		)  				| (( $1 ) << 8)	 | (( $3 ) << 19)	 | (( $3 ) << 23)	 				| (( $6 ) << 16)	 | ( $8 )		 | (( $10 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}

		/* same again, but defaulting MPY dest reg to src2 reg */
		/* AND in 2nd operation too */
		/* optional syntax #13 */
		|			indirect ',' reg
		BARBAR par_mpyi_op2	indirect ',' Dreg2_3
		{
			if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 | 	(	(( 0x3  ) << 24)		)  				| (( $1 ) << 8)	 | (( $3 ) << 19)	 | (( $3 ) << 23)	 				| ( $6 )		 | (( $8 ) << 16)	 | (( $8 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}

		/* Finally just defaulting 2nd operation dest reg to src2 reg */
		/* optional syntax #13 */

		|			indirect ',' indirect ',' reg
		BARBAR	par_mpyi_op2	Dreg0_7 ',' Dreg2_3
		{
			if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 | 	(	(( 0x0  ) << 24)		)  				| (( $3 ) << 8)	 | ( $1 )		 | (( $5 ) << 23)	 				| (( $8 ) << 16)	 | (( $10 ) << 19)	 | (( $10 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			indirect ',' reg ',' reg
		BARBAR par_mpyi_op2	indirect ',' Dreg2_3
		{
			if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 | 	(	(( 0x3  ) << 24)		)  				| (( $1 ) << 8)	 | (( $3 ) << 19)	 | (( $5 ) << 23)	 				| ( $8 )		 | (( $10 ) << 16)	 | (( $10 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			reg ',' indirect ',' Dreg0_1
		BARBAR par_mpyi_op2	indirect ',' Dreg2_3
		{
			/* same again but allowing for commutative * */
			/* Optional syntax #14 */

			if ( $1  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 | 	(	(( 0x3  ) << 24)		)  				| (( $1 ) << 19)	 | (( $3 ) << 8)	 | (( $5 ) << 23)	 				| ( $8 )		 | (( $10 ) << 16)	 | (( $10 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		;


/* As par_mpyi_mode is only called by MPYI3 ||, we can set the opcodes with */
/* confidence. */

par_mpyi_op2:		SUBI
			{	$$ = (((unsigned)0x2  << 30) | (( 0x3  ) << 26)) 	; }
			| ADDI
			{	$$ = (((unsigned)0x2  << 30) | (( 0x2  ) << 26)) 	; }
			| SUBI3
			{	$$ = (((unsigned)0x2  << 30) | (( 0x3  ) << 26)) 	; }
			| ADDI3
			{	$$ = (((unsigned)0x2  << 30) | (( 0x2  ) << 26)) 	; }
			;


/* MPYF || SUB/ADD ****************************************************	*/

par_mpyf_mode:
					indirect ',' indirect ',' Dreg
		BARBAR	par_mpyf_op2	Dreg0_7 ',' Dreg0_7 ',' Dreg2_3
		{
			/* check registers are valid */
			if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			/* construct address mode and operand fields to */
			/* insert into instruction template */
			/* subtract 2 from Dreg2_3 to make 1 bit reg selector */
			$$ = $7 | 	(	(( 0x0  ) << 24)		)  				| (( $3 ) << 8)	 | ( $1 )		 | (( $5 ) << 23)	 				| (( $8 ) << 16)	 | (( $10 ) << 19)	 | (( $12 - 2 ) << 22)	;

			/* Second pass to check that disp.'s are only 1 or 0 */
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			indirect ',' Dreg ',' Dreg
		BARBAR par_mpyf_op2	indirect ',' Dreg0_7 ',' Dreg2_3
		{
			/* check registers are valid */
			if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 | 	(	(( 0x3  ) << 24)		)  				| (( $1 ) << 8)	 | (( $3 ) << 19)	 | (( $5 ) << 23)	 				| ( $8 )		 | (( $10 ) << 16)	 | (( $12 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			Dreg ',' indirect ',' Dreg0_1
		BARBAR par_mpyf_op2	indirect ',' Dreg0_7 ',' Dreg2_3
		{
			/* same as previous, but allowing for commutative mpy */
			/* Optional syntax #14 */

			if ( $1  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 | 	(	(( 0x3  ) << 24)		)  				| (( $3 ) << 8)	 | (( $1 ) << 19)	 | (( $5 ) << 23)	 				| ( $8 )		 | (( $10 ) << 16)	 | (( $12 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			Dreg ',' Dreg ',' Dreg0_1
		BARBAR par_mpyf_op2	indirect ',' indirect ',' Dreg2_3
		{
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $3  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 | 	(	(( 0x2  ) << 24)		)  				| (( $3 ) << 19)	 | (( $1 ) << 16)	 | (( $5 ) << 23)	 				| ( $8 )		 | (( $10 ) << 8)	 | (( $12 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			indirect ',' Dreg ',' Dreg
		BARBAR par_mpyf_op2	Dreg0_7 ',' indirect ',' Dreg2_3
		{
			if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 | 	(	(( 0x1  ) << 24)		)  				| (( $1 ) << 8)	 | (( $3 ) << 19)	 | (( $5 ) << 23)	 				| (( $8 ) << 16)	 | ( $10 )		 | (( $12 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			Dreg ',' indirect ',' Dreg0_1
		BARBAR par_mpyf_op2	Dreg0_7 ',' indirect ',' Dreg2_3
		{
			/* same again but allowing for commutative mpy */
			/* Optional syntax #14 */

			if ( $1  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 | 	(	(( 0x1  ) << 24)		)  				| (( $3 ) << 8)	 | (( $1 ) << 19)	 | (( $5 ) << 23)	 				| (( $8 ) << 16)	 | ( $10 )		 | (( $12 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}

		/* repeated again, but defaulting MPY dest reg to src2 reg */
		/* optional syntax #13 */

		|			indirect ',' Dreg
		BARBAR par_mpyf_op2	indirect ',' Dreg0_7 ',' Dreg2_3
		{
			if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 | 	(	(( 0x3  ) << 24)		)  				| (( $1 ) << 8)	 | (( $3 ) << 19)	 | (( $3 ) << 23)	 				| ( $6 )		 | (( $8 ) << 16)	 | (( $10 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			Dreg ',' Dreg
		BARBAR par_mpyf_op2	indirect ',' indirect ',' Dreg2_3
		{
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 | 	(	(( 0x2  ) << 24)		)  				| (( $3 ) << 19)	 | (( $1 ) << 16)	 | (( $3 ) << 23)	 				| ( $6 )		 | (( $8 ) << 8)	 | (( $10 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			indirect ',' Dreg
		BARBAR par_mpyf_op2	Dreg0_7 ',' indirect ',' Dreg2_3
		{
			if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 | 	(	(( 0x1  ) << 24)		)  				| (( $1 ) << 8)	 | (( $3 ) << 19)	 | (( $3 ) << 23)	 				| (( $6 ) << 16)	 | ( $8 )		 | (( $10 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}

		/* same again, but defaulting MPY dest reg to src2 reg */
		/* AND in 2nd operation too */
		/* optional syntax #13 */
		|			indirect ',' Dreg
		BARBAR par_mpyf_op2	indirect ',' Dreg2_3
		{
			if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 | 	(	(( 0x3  ) << 24)		)  				| (( $1 ) << 8)	 | (( $3 ) << 19)	 | (( $3 ) << 23)	 				| ( $6 )		 | (( $8 ) << 16)	 | (( $8 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}

		/* Finally just defaulting 2nd operation dest reg to src2 reg */
		/* optional syntax #13 */

		|			indirect ',' indirect ',' Dreg
		BARBAR	par_mpyf_op2	Dreg0_7 ',' Dreg2_3
		{
			if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 | 	(	(( 0x0  ) << 24)		)  				| (( $3 ) << 8)	 | ( $1 )		 | (( $5 ) << 23)	 				| (( $8 ) << 16)	 | (( $10 ) << 19)	 | (( $10 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			indirect ',' Dreg ',' Dreg
		BARBAR par_mpyf_op2	indirect ',' Dreg2_3
		{
			if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 | 	(	(( 0x3  ) << 24)		)  				| (( $1 ) << 8)	 | (( $3 ) << 19)	 | (( $5 ) << 23)	 				| ( $8 )		 | (( $10 ) << 16)	 | (( $10 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			Dreg ',' indirect ',' Dreg0_1
		BARBAR par_mpyf_op2	indirect ',' Dreg2_3
		{
			/* same again but allowing for commutative * */
			/* Optional syntax #14 */

			if ( $1  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 | 	(	(( 0x3  ) << 24)		)  				| (( $1 ) << 19)	 | (( $3 ) << 8)	 | (( $5 ) << 23)	 				| ( $8 )		 | (( $10 ) << 16)	 | (( $10 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		;


/* As par_mpyf_mode is only called by MPYF3 ||, we can set the opcodes with */
/* confidence. */

par_mpyf_op2:		SUBF
			{	$$ = (((unsigned)0x2  << 30) | (( 0x1  ) << 26)) 	; }
			| ADDF
			{	$$ = (((unsigned)0x2  << 30) | (( 0x0  ) << 26)) 	; }
			| SUBF3
			{	$$ = (((unsigned)0x2  << 30) | (( 0x1  ) << 26)) 	; }
			| ADDF3
			{	$$ = (((unsigned)0x2  << 30) | (( 0x0  ) << 26)) 	; }
			;


/* *** REVERSE PARALLEL MPYI MODE *************************************	*/
/* Allows order of parallel operations in instruction to be swapped	*/
/* syntactically (but not semantically): SUBI || MPYI, ADDI || MPYI	*/

/* We have to check the source regs are Dreg0-7 and dest regs R0-1,	*/
/* R2-3 by hand as YACC only has a one token look ahead and this causes */
/* conflicts with other rules (CHK_Dreg0_7 / CHK_Dreg0_1 / CHK_Dreg2_3).*/

rev_par_mpyi_mode:
					reg ',' reg ',' Dreg2_3
		BARBAR	mpyi2_3		indirect ',' indirect ',' Dreg0_1
		{
			/* check registers are valid */
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			/* construct address mode and operand fields to */
			/* insert into instruction template */
			/* subtract 2 from Dreg2_3 to make 1 bit reg selector */
			$$ = 	(	(( 0x0  ) << 24)		)  				| (( $10 ) << 8)	 | ( $8 )		 | (( $12 ) << 23)	 				| (( $1 ) << 16)	 | (( $3 ) << 19)	 | (( $5 - 2 ) << 22)	;

			/* Second pass to check that disp.'s are only 1 or 0 */
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			indirect ',' reg ',' reg
		BARBAR mpyi2_3		indirect ',' Dreg0_7 ',' Dreg0_1
		{
			if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x3  ) << 24)		)  				| (( $8 ) << 8)	 | (( $10 ) << 19)	 | (( $12 ) << 23)	 				| ( $1 )		 | (( $3 ) << 16)	 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			indirect ',' reg ',' reg
		BARBAR mpyi2_3		Dreg0_7 ',' indirect ',' Dreg0_1
		{
			/* same again but allowing for commutative mpy */
			/* Optional syntax #14 */
			if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x3  ) << 24)		)  				| (( $10 ) << 8)	 | (( $8 ) << 19)	 | (( $12 ) << 23)	 				| ( $1 )		 | (( $3 ) << 16)	 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			indirect ',' indirect ',' Dreg2_3
		BARBAR mpyi2_3		Dreg0_7 ',' Dreg0_7 ',' Dreg0_1
		{
			if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x2  ) << 24)		)  				| (( $10 ) << 19)	 | (( $8 ) << 16)	 | (( $12 ) << 23)	 				| ( $1 )		 | (( $3 ) << 8)	 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			reg ',' indirect ',' reg
		BARBAR mpyi2_3		indirect ',' Dreg0_7 ',' Dreg0_1
		{
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x1  ) << 24)		)  				| (( $8 ) << 8)	 | (( $10 ) << 19)	 | (( $12 ) << 23)	 				| (( $1 ) << 16)	 | ( $3 )		 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			reg ',' indirect ',' reg
		BARBAR mpyi2_3		Dreg0_7 ',' indirect ',' Dreg0_1
		{
			/* same again but allowing for commutative mpy */
			/* Optional syntax #14 */
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x1  ) << 24)		)  				| (( $10 ) << 8)	 | (( $8 ) << 19)	 | (( $12 ) << 23)	 				| (( $1 ) << 16)	 | ( $3 )		 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}

		/* repeated again, but defaulting MPY dest reg to src2 reg */
		/* optional syntax #13 */

		|			indirect ',' reg ',' reg
		BARBAR mpyi2_3		indirect ',' Dreg0_1
		{
			if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x3  ) << 24)		)  				| (( $8 ) << 8)	 | (( $10 ) << 19)	 | (( $10 ) << 23)	 				| ( $1 )		 | (( $3 ) << 16)	 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			indirect ',' indirect ',' Dreg2_3
		BARBAR mpyi2_3		Dreg0_7 ',' Dreg0_1
		{
			if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x2  ) << 24)		)  				| (( $10 ) << 19)	 | (( $8 ) << 16)	 | (( $10 ) << 23)	 				| ( $1 )		 | (( $3 ) << 8)	 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			reg ',' indirect ',' reg
		BARBAR mpyi2_3		indirect ',' Dreg0_1
		{
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x1  ) << 24)		)  				| (( $8 ) << 8)	 | (( $10 ) << 19)	 | (( $10 ) << 23)	 				| (( $1 ) << 16)	 | ( $3 )		 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}

		/* same again, but defaulting MPY dest reg to src2 reg */
		/* AND in 2nd operation too */
		/* optional syntax #13 */
		|			indirect ',' reg
		BARBAR mpyi2_3		indirect ',' Dreg0_1
		{
			if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x3  ) << 24)		)  				| (( $6 ) << 8)	 | (( $8 ) << 19)	 | (( $8 ) << 23)	 				| ( $1 )		 | (( $3 ) << 16)	 | (( $3 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}

		/* Finally just defaulting 2nd operation dest reg to src2 reg */
		/* optional syntax #13 */

		|			reg ',' Dreg2_3
		BARBAR	mpyi2_3		indirect ',' indirect ',' Dreg0_1
		{
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x0  ) << 24)		)  				| (( $8 ) << 8)	 | ( $6 )		 | (( $10 ) << 23)	 				| (( $1 ) << 16)	 | (( $3 ) << 19)	 | (( $3 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			indirect ',' reg
		BARBAR mpyi2_3		indirect ',' Dreg0_7 ',' Dreg0_1
		{
			if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x3  ) << 24)		)  				| (( $6 ) << 8)	 | (( $8 ) << 19)	 | (( $10 ) << 23)	 				| ( $1 )		 | (( $3 ) << 16)	 | (( $3 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			indirect ',' reg
		BARBAR mpyi2_3		Dreg0_7 ',' indirect ',' Dreg0_1
		{
			/* same again but allowing for commutative * */
			/* Optional syntax #14 */

			if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x3  ) << 24)		)  				| (( $6 ) << 19)	 | (( $8 ) << 8)	 | (( $10 ) << 23)	 				| ( $1 )		 | (( $3 ) << 16)	 | (( $3 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		;

mpyi2_3:	MPYI | MPYI3 ;



/* *** REVERSE PARALLEL MPYF MODE *************************************	*/
/* Allows order of parallel operations in instruction to be swapped	*/
/* syntactically (but not semantically): SUBF || MPYF, ADDF || MPYF	*/

rev_par_mpyf_mode:
					Dreg ',' Dreg ',' Dreg2_3
		BARBAR	mpyf2_3		indirect ',' indirect ',' Dreg0_1
		{
			/* check registers are valid */
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			/* construct address mode and operand fields to */
			/* insert into instruction template */
			/* subtract 2 from Dreg2_3 to make 1 bit reg selector */
			$$ = 	(	(( 0x0  ) << 24)		)  				| (( $10 ) << 8)	 | ( $8 )		 | (( $12 ) << 23)	 				| (( $1 ) << 16)	 | (( $3 ) << 19)	 | (( $5 - 2 ) << 22)	;

			/* Second pass to check that disp.'s are only 1 or 0 */
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			indirect ',' Dreg ',' Dreg
		BARBAR mpyf2_3		indirect ',' Dreg0_7 ',' Dreg0_1
		{
			if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x3  ) << 24)		)  				| (( $8 ) << 8)	 | (( $10 ) << 19)	 | (( $12 ) << 23)	 				| ( $1 )		 | (( $3 ) << 16)	 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			indirect ',' Dreg ',' Dreg
		BARBAR mpyf2_3		Dreg0_7 ',' indirect ',' Dreg0_1
		{
			/* same again but allowing for commutative mpy */
			/* Optional syntax #14 */
			if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x3  ) << 24)		)  				| (( $10 ) << 8)	 | (( $8 ) << 19)	 | (( $12 ) << 23)	 				| ( $1 )		 | (( $3 ) << 16)	 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			indirect ',' indirect ',' Dreg2_3
		BARBAR mpyf2_3		Dreg0_7 ',' Dreg0_7 ',' Dreg0_1
		{
			if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x2  ) << 24)		)  				| (( $10 ) << 19)	 | (( $8 ) << 16)	 | (( $12 ) << 23)	 				| ( $1 )		 | (( $3 ) << 8)	 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			Dreg ',' indirect ',' Dreg
		BARBAR mpyf2_3		indirect ',' Dreg0_7 ',' Dreg0_1
		{
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x1  ) << 24)		)  				| (( $8 ) << 8)	 | (( $10 ) << 19)	 | (( $12 ) << 23)	 				| (( $1 ) << 16)	 | ( $3 )		 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			Dreg ',' indirect ',' Dreg
		BARBAR mpyf2_3		Dreg0_7 ',' indirect ',' Dreg0_1
		{
			/* same again but allowing for commutative mpy */
			/* Optional syntax #14 */
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x1  ) << 24)		)  				| (( $10 ) << 8)	 | (( $8 ) << 19)	 | (( $12 ) << 23)	 				| (( $1 ) << 16)	 | ( $3 )		 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}

		/* repeated again, but defaulting MPY dest reg to src2 reg */
		/* optional syntax #13 */

		|			indirect ',' Dreg ',' Dreg
		BARBAR mpyf2_3		indirect ',' Dreg0_1
		{
			if ( $3  > 7) { Error("Must be register R0-R7"); } ; if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x3  ) << 24)		)  				| (( $8 ) << 8)	 | (( $10 ) << 19)	 | (( $10 ) << 23)	 				| ( $1 )		 | (( $3 ) << 16)	 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			indirect ',' indirect ',' Dreg2_3
		BARBAR mpyf2_3		Dreg0_7 ',' Dreg0_1
		{
			if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x2  ) << 24)		)  				| (( $10 ) << 19)	 | (( $8 ) << 16)	 | (( $10 ) << 23)	 				| ( $1 )		 | (( $3 ) << 8)	 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			Dreg ',' indirect ',' Dreg
		BARBAR mpyf2_3		indirect ',' Dreg0_1
		{
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x1  ) << 24)		)  				| (( $8 ) << 8)	 | (( $10 ) << 19)	 | (( $10 ) << 23)	 				| (( $1 ) << 16)	 | ( $3 )		 | (( $5 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}

		/* same again, but defaulting MPY dest reg to src2 reg */
		/* AND in 2nd operation too */
		/* optional syntax #13 */
		|			indirect ',' Dreg
		BARBAR mpyf2_3		indirect ',' Dreg0_1
		{
			if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x3  ) << 24)		)  				| (( $6 ) << 8)	 | (( $8 ) << 19)	 | (( $8 ) << 23)	 				| ( $1 )		 | (( $3 ) << 16)	 | (( $3 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}

		/* Finally just defaulting 2nd operation dest reg to src2 reg */
		/* optional syntax #13 */

		|			Dreg ',' Dreg2_3
		BARBAR	mpyf2_3		indirect ',' indirect ',' Dreg0_1
		{
			if ( $1  > 7) { Error("Must be register R0-R7"); } ; if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x0  ) << 24)		)  				| (( $8 ) << 8)	 | ( $6 )		 | (( $10 ) << 23)	 				| (( $1 ) << 16)	 | (( $3 ) << 19)	 | (( $3 - 2 ) << 22)	;
			CurInstr->combine = 	15	;
			CurInstr->combine2 = 	16	;
		}
		|			indirect ',' Dreg
		BARBAR mpyf2_3		indirect ',' Dreg0_7 ',' Dreg0_1
		{
			if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x3  ) << 24)		)  				| (( $6 ) << 8)	 | (( $8 ) << 19)	 | (( $10 ) << 23)	 				| ( $1 )		 | (( $3 ) << 16)	 | (( $3 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		|			indirect ',' Dreg
		BARBAR mpyf2_3		Dreg0_7 ',' indirect ',' Dreg0_1
		{
			/* same again but allowing for commutative * */
			/* Optional syntax #14 */

			if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ = 	(	(( 0x3  ) << 24)		)  				| (( $6 ) << 19)	 | (( $8 ) << 8)	 | (( $10 ) << 23)	 				| ( $1 )		 | (( $3 ) << 16)	 | (( $3 - 2 ) << 22)	;
			CurInstr->combine = 	16	;
			CurInstr->combine2 = 	15	;
		}
		;

mpyf2_3:	MPYF | MPYF3 ;




/* ******************************************************************** */
/* C40 CPU mnemonic grammer:						*/
/* ******************************************************************** */

machine_op:	{
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
		C40_op	/* match 'C40 mneumonics */
		{
			/* add new instruction into the parse tree */
			ParseTreeItem *pti = NewParseTreeItem(INSTRUCTION);
			pti->type.instr = CurInstr;

			curPC += sizeof(int);
		}
		;


C40_op:		loads | stores | branches | stackops
		| arith_logic_ops | miscops ;



/* ********************************************************************	*/
/* *** LOADS **********************************************************	*/

loads:		load_ints
		| par_load_int
		| load_fp
		| par_load_fp
		| load_parts
		| load_interlocked

		/* load address register (read phase of pipeline)	*/
		| LDA		int_ld_Addr_reg_modes
		{	CurInstr->opcode = $1->diadic | $2; }

		/* load imm. value into top 16bits of register		*/
		| LDHI		immediate ',' reg
		{
			CurInstr->opcode = $1->diadic | 		( 0x3   << 21)					 | (( $4 ) << 16)		;
			/* unsigned immediate */
			CurInstr->combine = 2 ;
		}

		/* pseudo-op places 16 MSB's of imm. via LDPK or LDIU	*/
		/* into DP. '@' = optional syntax #10			*/
		| LDP		immediate ',' reg
		{
			if ($4 == 	0x10 ) {
				/* Second pass will >> 16 the immediate value */
				CurInstr->combine = 	3 ;
				/* diadic points to a LDPK instruction */
				/* immediate mode and DP reg already set */
				/* in OP_LDPK */
				CurInstr->opcode = $1->diadic;
			}
			else {
				/* Second pass will >> 16 the immediate value */
				CurInstr->combine = 	3 ;
				/* triadic points to a LDIU instruction */
				CurInstr->opcode = $1->triadic | 		( 0x3   << 21)					 | (( $4 ) << 16)		;
			}
		}
		| LDP		immediate
		{
				CurInstr->combine = 	3 ;
				CurInstr->opcode = $1->diadic;
		}
		| LDP		'@' immediate ',' reg
		{
			if ($5 == 	0x10 ) {
				CurInstr->combine = 	3 ;
				CurInstr->opcode = $1->diadic;
			}
			else {
				CurInstr->combine = 	3 ;
				CurInstr->opcode = $1->triadic | 		( 0x3   << 21)					 | (( $5 ) << 16)		;
			}
		}
		| LDP		'@' immediate
		{
				CurInstr->combine = 	3 ;
				CurInstr->opcode = $1->diadic;
		}

		/* place 16bit immediate into DP - straight, no messin' */
		/* immediate mode and DP reg already set in OP_LDPK */
		| LDPK		immediate
		{	CurInstr->opcode = $1->diadic; }

		/* load and store the contents of the expansion reg	*/
		/* file regs						*/
		| LDEP		Exp_reg ',' reg
		{	CurInstr->opcode = $1->diadic | ( $2 )			 | (( $4 ) << 16)		; }
		| LDPE		reg ',' Exp_reg
		{	CurInstr->opcode = $1->diadic | ( $2 )			 | (( $4 ) << 16)		; }
		;


/* *** INTEGER LOADS **************************************************	*/
/* All general addressing modes allowed.				*/

load_ints:	LDIcond		int_diadic_modes
		{ CurInstr->opcode = $1->diadic | $2; }
		;


/* *** FP LOADS *******************************************************	*/
/* All addr. modes, but can only use data regs.				*/

load_fp:	fp_loads	fp_diadic_modes
		{ CurInstr->opcode = $1->diadic | $2; }
		;

fp_loads:		LDE | LDM | LDFcond ;


/* *** PARALLEL INT LOADS AND LOAD/STORES *****************************	*/

par_load_int:
		LDI	reg ',' reg
		{
			CurInstr->opcode = $1->diadic 				| 		( 0x0   << 21)					 | (( $4 ) << 16)		 | ( $2 )			;
		}
		| LDI	direct ',' reg
		{
			CurInstr->opcode = $1->diadic | 		( 0x1   << 21)					 | (( $4 ) << 16)		;
		}
		| LDI	indirect ',' reg
		{
			CurInstr->opcode = $1->diadic 				| 		( 0x2   << 21)					 | (( $4 ) << 16)		 | (( $2 ) << 8)		;
		}
		| LDI	immediate ',' reg
		{
			CurInstr->opcode = $1->diadic | 		( 0x3   << 21)					 | (( $4 ) << 16)		;
		}
		| LDI	indirect ',' reg     BARBAR LDI	indirect ',' reg
		{
			if ( $4  > 7) { Error("Must be register R0-R7"); } ;

			/* triadic position used to hold par_ld opcode */
			CurInstr->opcode = $1->triadic 			  | ( $2 )		 | (( $4 ) << 22)	 | (( $7 ) << 8)	 | (( $9 ) << 19)	;

			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
		| LDI	indirect ',' reg     BARBAR STI	Dreg0_7 ',' indirect
		{
			if ( $4  > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = $1->par_st 			    | ( $2 )		 | (( $4 ) << 22)	 | (( $7 ) << 16)	 | (( $9 ) << 8)	;

			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
		;


/* *** Parallel FP loads and load/stores ******************************	*/

par_load_fp:
		LDF	Dreg ',' Dreg
		{
			CurInstr->opcode = $1->diadic 			| 		( 0x0   << 21)					 | (( $4 ) << 16)		 | ( $2 )			;
		}
		| LDF	direct ',' Dreg
		{
			CurInstr->opcode = $1->diadic | 		( 0x1   << 21)					 | (( $4 ) << 16)		;
		}
		| LDF	indirect ',' Dreg
		{
			CurInstr->opcode = $1->diadic 				| 		( 0x2   << 21)					 | (( $4 ) << 16)		 | (( $2 ) << 8)		;
		}
		| LDF	fp_immediate ',' Dreg
		{
			CurInstr->opcode = $1->diadic | 		( 0x3   << 21)					 | $2 				| (( $4 ) << 16)		;
		}
		| LDF	indirect ',' Dreg    BARBAR LDF	indirect ',' Dreg
		{
			if ( $4  > 7) { Error("Must be register R0-R7"); } ;

			/* triadic position used to hold par_ld opcode */
			CurInstr->opcode = $1->triadic 			  | ( $2 )		 | (( $4 ) << 22)	 | (( $7 ) << 8)	 | (( $9 ) << 19)	;

			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
		| LDF	indirect ',' Dreg    BARBAR STF	Dreg0_7 ',' indirect
		{
			if ( $4  > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = $1->par_st 			   | ( $2 )		 | (( $4 ) << 22)	 | (( $7 ) << 16)	 | (( $9 ) << 8)	;

			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}
		;


/* *** BYTE AND PART WORD LOADS ***************************************	*/
/* Only reg, direct and indirect addr. modes allowed.			*/

load_parts:
		part_loads	reg ',' reg
		{
			CurInstr->opcode = $1->diadic 				| 		( 0x0   << 21)					 | (( $4 ) << 16)		 | ( $2 )			;
		}
		| part_loads	direct ',' reg
		{
			CurInstr->opcode = $1->diadic | 		( 0x1   << 21)					 | (( $4 ) << 16)		;
		}
		| part_loads	indirect ',' reg
		{
			CurInstr->opcode = $1->diadic 				| 		( 0x2   << 21)					 | (( $4 ) << 16)		 | (( $2 ) << 8)		;
		}
		;

part_loads:		LBb | LBUb | LHw | LHUw | LWLct | LWRct | MBct | MHct ;


/* *** INTERLOCKED LOADS **********************************************	*/
/* Only direct and indirect addr. modes are allowed.			*/

load_interlocked:
		interlocked_loads	direct ',' reg
		{
			CurInstr->opcode = $1->diadic | 		( 0x1   << 21)					 | (( $4 ) << 16)		;
		}
		| interlocked_loads	indirect ',' reg
		{
			CurInstr->opcode = $1->diadic 				| 		( 0x2   << 21)					 | (( $4 ) << 16)		 | (( $2 ) << 8)		;
		}
		;

interlocked_loads:	LDFI | LDII | SIGI ;



/* ********************************************************************	*/
/* *** STORES *********************************************************	*/

stores:
		/* general stores *************************************	*/
		genstores		st_addr_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		/* five bit immediate */
		| STIK			immediate ',' direct
		{
			/* STIK uses REG mode for direct mode */
			CurInstr->opcode = $1->diadic | 		( 0x0   << 21)					;

			/* In 2nd pass, check immediate is <= 5bits */
			CurInstr->combine = 	6 ;
		}
		| STIK			immediate ',' indirect
		{
			/* STIK uses IMMediate mode for indirect mode */
			CurInstr->opcode = $1->diadic | 		( 0x3   << 21)					 | (( $4 ) << 8)		;
			CurInstr->combine = 	6 ;
		}

		| STF			Dreg ',' direct
		{
			CurInstr->opcode = $1->diadic | 		( 0x1   << 21)					 | (( $2 ) << 16)		;
		}
		| STF			Dreg ',' indirect
		{
			CurInstr->opcode = $1->diadic | 		( 0x2   << 21)					 | (( $2 ) << 16)		 				| (( $4 ) << 8)		;
		}

		| STI			reg ',' direct
		{
			CurInstr->opcode = $1->diadic | 		( 0x1   << 21)					 | (( $2 ) << 16)		;
		}
		| STI			reg ',' indirect
		{
			CurInstr->opcode = $1->diadic | 		( 0x2   << 21)					 | (( $2 ) << 16)		 				| (( $4 ) << 8)		;
		}


		/* Parallel Stores ************************************	*/

		/* We have to check the regs are Dreg0-7 by hand as	*/
		/* YACC only has a one token look ahead and this causes */
		/* a shift reduce conflict with normal STF/STI		*/
		/* operations (CHK_Dreg0_7).				*/

		/* STF || STF *****************************************	*/
		| STF			Dreg ',' indirect
		BARBAR STF		Dreg0_7 ',' indirect
		{
			if ( $2  > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = $1->par_st 				| (( $2 ) << 22)	 | ( $4 )		 				| (( $7 ) << 16)	 | (( $9 ) << 8)	;

			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}


		/* STF || LDF *****************************************	*/
		/* optional syntax #11 - swapped order of LDF||STF */
		| STF			Dreg ',' indirect
		BARBAR LDF		indirect ',' Dreg0_7
		{
			if ( $2  > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = $6->par_st 				| ( $7 )		 | (( $9 ) << 22)	 				| (( $2 ) << 16)	 | (( $4 ) << 8)	;

			CurInstr->combine =  	14 ;
			CurInstr->combine2 = 	13 ;
		}

		/* OP||STF -> STF||OP *********************************	*/

		/* Optional syntax #11 - swapped order of OP||STF	*/
		/* triadic parallel stores */
		| STF			Dreg ',' indirect
		BARBAR tri_swap_par_stf	indirect ',' Dreg0_7 ',' Dreg0_7
		{
			if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				| ( $7 )		 | (( $9 ) << 19)	 | (( $11 ) << 22)	 				| (( $2 ) << 16)	 | (( $4 ) << 8)	;

			CurInstr->combine =  	14 ;
			CurInstr->combine2 = 	13 ;
		}
		/* and with dst register same as 2nd source reg */
		/* Optional syntax #13 */
		| STF			Dreg ',' indirect
		BARBAR tri_swap_par_stf	indirect ',' Dreg0_7
		{
			if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				| ( $7 )		 | (( $9 ) << 19)	 | (( $9 ) << 22)	
				| (( $2 ) << 16)	 | (( $4 ) << 8)	;
			CurInstr->combine = 	14 ;
			CurInstr->combine2 = 	13 ;
		}
		/* diadic parallel stores */
		| STF			Dreg ',' indirect
		BARBAR dia_swap_par_stf	indirect ',' Dreg0_7
		{
			if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				| ( $7 )		 | (( $9 ) << 22)	 				| (( $2 ) << 16)	 | (( $4 ) << 8)	;
			CurInstr->combine = 	14 ;
			CurInstr->combine2 = 	13 ;
		}
		/* swapped operand shift instructions */
		| STF			Dreg ',' indirect
		BARBAR shift_swap_par_stf Dreg0_7 ',' indirect ',' Dreg0_7
		{
			if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				| (( $7 ) << 19)	 | ( $9 )		 | (( $11 ) << 22)	 				| (( $2 ) << 16)	 | (( $4 ) << 8)	;

			CurInstr->combine = 	14 ;
			CurInstr->combine2 = 	13 ;
		}


		/* STI || STI *****************************************	*/

		| STI			reg ',' indirect
		BARBAR STI		Dreg0_7 ',' indirect
		{
			if ( $2  > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = $1->par_st 				| (( $2 ) << 22)	 | ( $4 )		 				| (( $7 ) << 16)	 | (( $9 ) << 8)	;

			CurInstr->combine = 	13 ;
			CurInstr->combine2 = 	14 ;
		}

		/* STI || LDI *****************************************	*/
		/* optional syntax #11 - swapped order of LDF||STF */
		| STI			reg ',' indirect
		BARBAR LDI		indirect ',' Dreg0_7
		{
			if ( $2  > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = $6->par_st 				| ( $7 )		 | (( $9 ) << 22)	 				| (( $2 ) << 16)	 | (( $4 ) << 8)	;

			CurInstr->combine = 	14 ;
			CurInstr->combine2 = 	13 ;
		}

		/* OP||STI -> STI||OP *********************************	*/

		/* Optional syntax #11 - swapped order of OP||STI */

		/* triadic parallel stores */
		| STI			reg ',' indirect
		BARBAR tri_swap_par_sti	indirect ',' Dreg0_7 ',' Dreg0_7
		{
			if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				| ( $7 )		 | (( $9 ) << 19)	 | (( $11 ) << 22)	 				| (( $2 ) << 16)	 | (( $4 ) << 8)	;

			CurInstr->combine =  	14 ;
			CurInstr->combine2 = 	13 ;
		}
		/* and with dst register same as 2nd source reg */
		/* Optional syntax #13 */
		| STI			reg ',' indirect
		BARBAR tri_swap_par_sti	indirect ',' Dreg0_7
		{
			if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				| ( $7 )		 | (( $9 ) << 19)	 | (( $9 ) << 22)	
				| (( $2 ) << 16)	 | (( $4 ) << 8)	;
			CurInstr->combine = 	14 ;
			CurInstr->combine2 = 	13 ;
		}

		/* diadic parallel stores */
		| STI			reg ',' indirect
		BARBAR dia_swap_par_sti	indirect ',' Dreg0_7
		{
			if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				| ( $7 )		 | (( $9 ) << 22)	 				| (( $2 ) << 16)	 | (( $4 ) << 8)	;
			CurInstr->combine = 	14 ;
			CurInstr->combine2 = 	13 ;
		}

		/* swapped operand shift instructions */
		| STI			reg ',' indirect
		BARBAR shift_swap_par_sti Dreg0_7 ',' indirect ',' Dreg0_7
		{
			if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				| (( $7 ) << 19)	 | ( $9 )		 | (( $11 ) << 22)	 				| (( $2 ) << 16)	 | (( $4 ) << 8)	;

			CurInstr->combine = 	14 ;
			CurInstr->combine2 = 	13 ;
		}
		;


genstores:		STFI | STII ;


tri_swap_par_sti:	ADDI | ADDI3 | AND | AND3 | FIX |
			MPYI | MPYI3 | OR | OR3 | XOR | XOR3 ;

dia_swap_par_sti:	ABSI | NEGI | NOT ;

shift_swap_par_sti:	ASH3 | ASH |  LSH3 | LSH | SUBI | SUBI3 ;


tri_swap_par_stf:	ADDF | ADDF3 | MPYF | MPYF3 ;

dia_swap_par_stf:	ABSF | FLOAT | FRIEEE | TOIEEE ;

shift_swap_par_stf:	SUBF | SUBF3 ;



/* ********************************************************************	*/
/* *** BRANCHES *******************************************************	*/

branches:	bigbranches
		| condbranches
		| decbranches
		| returns
		| traps
		| repeats
		;


bigbranches:	bigbranch_ops pcrel
		{
			CurInstr->opcode = $1->diadic;

			/* specifies 24 bit pcrel value */
			CurInstr->combine = 18 ;
		}
		;

bigbranch_ops:	BR | BRD | CALL | LAJ
		;


condbranches:	condbranch	pcrel
		{	CurInstr->opcode = $1->diadic | (1 << 25)	; }
		| condbranch	reg
		{	CurInstr->opcode = $1->diadic | 	0		 | ( $2 )			; }
		;

condbranch:	Bcond | BcondAF | BcondAT | BcondD | CALLcond | LAJcond
		;


decbranches:	DBcond		Areg ',' reg
		{	
			CurInstr->opcode = $1->diadic 				| 	0		 | ((( $2 ) - 8) << 22)	 | ( $4 )			;
		}
		| DBcond	Areg ',' pcrel
		{
			CurInstr->opcode = $1->diadic 				| (1 << 25)	 | ((( $2 ) - 8) << 22)	;
		}
		| DBcondD	Areg ',' reg
		{
			CurInstr->opcode = $1->diadic 				| 	0		 | ((( $2 ) - 8) << 22)	 | ( $4 )			;
		}
		| DBcondD	Areg ',' pcrel
		{
			CurInstr->opcode = $1->diadic 				| (1 << 25)	 | ((( $2 ) - 8) << 22)	;
		}
		;


returns:	ret_ops
		{	CurInstr->opcode = $1->diadic; }
		;


ret_ops:	RETIcond | RETIcondD | RETScond ;


traps:		trap_ops immediate
		{
			CurInstr->opcode = $1->diadic;
			CurInstr->combine = 19 ;
		}
		;

trap_ops:	LATcond | TRAPcond ;


repeats:	rptb_ops	pcrel
		{
			CurInstr->opcode = $1->diadic;

			/* check and insert 24 bit pc rel offset */
			CurInstr->combine = 18 ;
		}
		| rptb_ops	reg
		{
			/* note that "RPTB reg" and "RPTB pcrel" have */
			/* completely different opcodes! The reg version */
			/* is held in the triadic position */
			CurInstr->opcode = $1->triadic | ( $2 )			;
		}

		| RPTS		reg
		{	CurInstr->opcode = $1->diadic | 		( 0x0   << 21)					 | ( $2 )			; }
		| RPTS		direct
		{	CurInstr->opcode = $1->diadic | 		( 0x1   << 21)					; }
		| RPTS		immediate
		{
			CurInstr->opcode = $1->diadic | 		( 0x3   << 21)					;
			/* immediate is unsigned */
			CurInstr->combine = 2 ;
		}
		| RPTS		indirect
		{	CurInstr->opcode = $1->diadic | 		( 0x2   << 21)					 | (( $2 ) << 8)		; }
		;

rptb_ops:	RPTB | RPTBD ;


/* ********************************************************************	*/
/* *** STACK OPERATIONS ***********************************************	*/

stackops:
		/* stack operations (rising stack) */
		PUSH		reg		/* *SP++ */
		{	CurInstr->opcode = $1->diadic | (( $2 ) << 16)		; }
		| PUSHF		Dreg
		{	CurInstr->opcode = $1->diadic | (( $2 ) << 16)		; }
		| POP		reg		/* *--SP */
		{	CurInstr->opcode = $1->diadic | (( $2 ) << 16)		; }
		| POPF		Dreg
		{	CurInstr->opcode = $1->diadic | (( $2 ) << 16)		; }
		;


/* ********************************************************************	*/
/* *** MISC OPERATIONS ************************************************	*/

miscops:
		IACK		direct
		{	CurInstr->opcode = $1->diadic | 		( 0x1   << 21)					; }
		| IACK		indirect
		{	CurInstr->opcode = $1->diadic | 		( 0x2   << 21)					 | (( $2 ) << 8)		; }

		| IDLE
		{	CurInstr->opcode = $1->diadic; }

		| NOP		indirect
		{	CurInstr->opcode = $1->diadic | 		( 0x2   << 21)					 | (( $2 ) << 8)		; }
		| NOP
		{	CurInstr->opcode = $1->diadic; }

		| SWI
		{	CurInstr->opcode = $1->diadic; }
		;



/* ********************************************************************	*/
/* *** Arithmetic and Logical Operations ******************************	*/
/* ********************************************************************	*/


/*
 * The following classification may seem a little long-winded, but due to
 * YACC's one token look ahead and the required compatibility with Texas's
 * optional syntax (triadic opcodes dont need the trailing 3, unary 2
 * opcodes can leave out the second destination register, etc) they have had
 * to be compartmentalised in this fashion.
 */

arith_logic_ops:
		int_unary_only | fp_unary_only
		| int_diadic_only | fp_diadic_only
		| int_triadic_only | int_both_2_3
		| int_par_st_unary | fp_par_st_unary | float_par_st_unary
		| unsigned_int_par_st_3 | unsigned_int_par_st_2_3
		| shift_int_par_st_3 | shift_int_par_st_2_3
		| fp_par_st_mpy_3 | fp_par_st_mpy_2_3
		| shift_fp_par_st_mpy_3 | shift_fp_par_st_mpy_2_3
		| int_par_st_mpy_3 | int_par_st_mpy_2_3
		| shift_int_par_st_mpy_3 | shift_int_par_st_mpy_2_3
		| mpy | reciprocals | rotates
		| cmp | frieee | toieee | fix
		;



/* ********************************************************************	*/
/* *** OPERATION AND ADDRESSING MODE COMBINATIONS *********************	*/

/* 
 * For each group of instruction types, define what the instructions are and
 * what addressing modes can be legally combined with them.
 */

/* *** INTEGER UNARY DIADIC OPERATIONS ONLY ***************************	*/

int_unary_only:
		NEGB	int_unary_op_mode
		{	CurInstr->opcode = $1->diadic | $2; }
		| NEGB	int_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		;


/* *** FP UNARY DIADIC OPERATIONS ONLY ********************************	*/

fp_unary_only:
		fp_unary_only_ops	fp_unary_op_mode
		{	CurInstr->opcode = $1->diadic | $2; }
		| fp_unary_only_ops	fp_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		;

fp_unary_only_ops:	NORM | RND ;


/* *** INTEGER DIADIC OPERATIONS ONLY *********************************	*/

int_diadic_only:
		int_diadic_only_ops	int_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		;

int_diadic_only_ops:	SUBC | SUBRB | SUBRI ;



/* *** FP DIADIC OPERATIONS ONLY **************************************	*/

fp_diadic_only:
		SUBRF	fp_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		;


/* *** INTEGER TRIADIC OPERATIONS ONLY ********************************	*/

int_triadic_only:
		int_triadic_only_ops	int_triadic_modes
		{
			CurInstr->opcode = $1->triadic | $2;
			if ($1->token == ANDN3 || $1->token == MPYUHI3) {
				/* convert pass2 immediate tests to unsigned */
				if (CurInstr->combine == 	11 )
					CurInstr->combine = 12 ;
				else if (CurInstr->combine2 == 	11 )
					CurInstr->combine2 = 12 ;
			}
		}
		;

int_triadic_only_ops:	ADDC3 | ANDN3 | MPYSHI3 | MPYUHI3 | SUBB3 ;


/* *** BOTH DIADIC AND TRIADIC INTEGER OPERATIONS *********************	*/

int_both_2_3:
		int_both_2_3_ops	int_diadic_modes
		{
			CurInstr->opcode = $1->diadic | $2;
			if ($1->token == ANDN || $1->token == MPYUHI) {
				/* convert pass2 immediate tests to unsigned */
				if (CurInstr->combine == 	1 )
					CurInstr->combine = 2 ;
			}
		}
		| int_both_2_3_ops	int_triadic_modes
		{
			CurInstr->opcode = $1->triadic | $2;
			if ($1->token == ANDN || $1->token == MPYUHI) {
				/* convert pass2 immediate tests to unsigned */
				if (CurInstr->combine == 	11 )
					CurInstr->combine = 12 ;
				else if (CurInstr->combine2 == 	11 )
					CurInstr->combine2 = 12 ;
			}
		}
		;

int_both_2_3_ops:	ADDC | ANDN | MPYSHI | MPYUHI | SUBB ;


/* *** BOTH UNARY DIADIC INTEGER AND PARALLEL STORE OPERATIONS ********	*/

int_par_st_unary:
		int_par_st_unary_ops	int_unary_op_mode
		{
			CurInstr->opcode = $1->diadic | $2;
		}
		| int_par_st_unary_ops	int_diadic_modes
		{
			CurInstr->opcode = $1->diadic | $2;
			if ($1->token == NOT && CurInstr->combine == 	1 ) {
				/* convert pass2 immediate tests to unsigned */
				CurInstr->combine = 2 ;
			}
		}
		| int_par_st_unary_ops	dia_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2;	}
		;

int_par_st_unary_ops:	ABSI | NEGI | NOT;


/* *** BOTH UNARY DIADIC FP AND PARALLEL STORE OPERATIONS *************	*/

fp_par_st_unary:
		fp_par_st_unary_ops	fp_unary_op_mode
		{	CurInstr->opcode = $1->diadic | $2; }
		| fp_par_st_unary_ops	fp_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		| fp_par_st_unary_ops	dia_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;

fp_par_st_unary_ops:	ABSF | NEGF ;

/* *** FLOAT UNARY DIADIC FP AND PARALLEL STORE OPERATIONS ************	*/

float_par_st_unary:
		FLOAT	fp_unary_op_mode
		{	CurInstr->opcode = $1->diadic | $2; }
		| FLOAT	float_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		| FLOAT	dia_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;

/* *** UNSIGNED TRIADIC, INTEGER AND PARALLEL STORE OPERATIONS ********	*/

unsigned_int_par_st_3:
		int_par_st_3_ops	int_triadic_modes
		{
			CurInstr->opcode = $1->triadic | $2;
			/* convert pass2 immediate tests to unsigned */
			if (CurInstr->combine == 	11 )
				CurInstr->combine = 12 ;
			else if (CurInstr->combine2 == 	11 )
				CurInstr->combine2 = 12 ;
		}
		| int_par_st_3_ops	tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;

int_par_st_3_ops:	AND3 | OR3 | XOR3 ;


/* *** TRIADIC SHIFT AND PARALLEL STORE OPERATIONS ********************	*/

shift_int_par_st_3:

		shift_par_st_3_ops	int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }

		| shift_par_st_3_ops	shift_tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;

shift_par_st_3_ops:	ASH3 | LSH3 ;


/* *** TRIADIC INTEGER AND PARALLEL STORE/MPY OPERATIONS **************	*/
/* With swapped order of subi operands */

shift_int_par_st_mpy_3:
		SUBI3		int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| SUBI3		shift_tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| SUBI3		rev_par_mpyi_mode
		{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x3  ) << 26)) 	 | $2; }
		;


/* *** TRIADIC INTEGER AND PARALLEL STORE/MPY OPERATIONS **************	*/
/* Normal order of operands */

int_par_st_mpy_3:
		ADDI3		int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| ADDI3		tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| ADDI3		rev_par_mpyi_mode
		{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x2  ) << 26)) 	 | $2; }
		;


/* *** BOTH TRIADIC FP AND PARALLEL STORE OPERATIONS ******************	*/
/* Swapped order of operands for sub */

shift_fp_par_st_mpy_3:
		SUBF3		fp_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| SUBF3		shift_tri_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| SUBF3 	rev_par_mpyf_mode
		{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x1  ) << 26)) 	 | $2; }
		;


/* *** BOTH TRIADIC FP AND PARALLEL STORE OPERATIONS ******************	*/
/* Normal order of operands */

fp_par_st_mpy_3:
		ADDF3		fp_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| ADDF3		tri_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| ADDF3		rev_par_mpyf_mode
		{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x0  ) << 26)) 	 | $2; }
		;


/* *** UNSIGNED DIADIC, TRIADIC INTEGER AND PARALLEL STORE OPERATIONS *	*/

unsigned_int_par_st_2_3:
		int_par_st_2_3_ops	int_diadic_modes
		{
			CurInstr->opcode = $1->diadic | $2;
			/* convert pass2 immediate tests to unsigned */
			if (CurInstr->combine == 	1 )
				CurInstr->combine = 2 ;
		}
		| int_par_st_2_3_ops	int_triadic_modes
		{
			CurInstr->opcode = $1->triadic | $2;
			/* convert pass2 immediate tests to unsigned */
			if (CurInstr->combine == 	11 )
				CurInstr->combine = 12 ;
			else if (CurInstr->combine2 == 	11 )
				CurInstr->combine2 = 12 ;
		}
		| int_par_st_2_3_ops	tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;

int_par_st_2_3_ops:	AND | OR | XOR ;


/* *** DIADIC, TRIADIC SHIFT AND PARALLEL STORE OPERATIONS ************	*/

shift_int_par_st_2_3:
		shift_par_st_2_3_ops	int_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }

		| shift_par_st_2_3_ops	int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }

		| shift_par_st_2_3_ops	shift_tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;

shift_par_st_2_3_ops:	ASH | LSH ;


/* *** DIADIC, TRIADIC INTEGER AND PARALLEL STORE/MPY OPERATIONS ******	*/
/* With swapped order of operands for sub || sti */

shift_int_par_st_mpy_2_3:
		SUBI		int_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		| SUBI		int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| SUBI		shift_tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| SUBI		rev_par_mpyi_mode
		{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x3  ) << 26)) 	 | $2; }
		;


/* *** DIADIC, TRIADIC INTEGER AND PARALLEL STORE/MPY OPERATIONS ******	*/
/* normal order of operands */
int_par_st_mpy_2_3:
		ADDI		int_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		| ADDI		int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| ADDI		tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| ADDI		rev_par_mpyi_mode
		{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x2  ) << 26)) 	 | $2; }
		;


/* *** DIADIC, TRIADIC FP AND PARALLEL STORE/MPY OPERATIONS ***********	*/

shift_fp_par_st_mpy_2_3:
		SUBF		fp_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		| SUBF		fp_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| SUBF		shift_tri_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| SUBF		rev_par_mpyf_mode
		{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x1  ) << 26)) 	 | $2; }
		;


/* *** DIADIC, TRIADIC FP AND PARALLEL STORE/MPY OPERATIONS ***********	*/

fp_par_st_mpy_2_3:
		ADDF		fp_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		| ADDF		fp_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| ADDF		tri_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| ADDF		rev_par_mpyf_mode
		{	CurInstr->opcode = (((unsigned)0x2  << 30) | (( 0x0  ) << 26)) 	 | $2; }
		;



/* ********************************************************************	*/
/* *** Non Standard Format Arithmetic and Logical Operations **********	*/
/* ********************************************************************	*/


/* ********************************************************************	*/
/* *** MULTIPLY *******************************************************	*/

mpy:	mpyf_3 | mpyf_2_3 | mpyi_3 | mpyi_2_3 ;


/* *** FP MULTIPLY ****************************************************	*/

/* Floating point multiply can be either diadic or triadic formats	*/
/* Both of which can also be used in a parallel format:			*/
/* ||STI, ||ADDF(3) and ||SUBI(3).					*/

/* Triadic or parallel operations MPYF3.				*/
mpyf_3:
		MPYF3		fp_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| MPYF3		tri_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| MPYF3		par_mpyf_mode
		{	CurInstr->opcode = $2; }
		;

/* Diadic or triadic or parallel operations MPYF.			*/
mpyf_2_3:
		MPYF		fp_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		| MPYF		fp_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| MPYF		tri_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| MPYF		par_mpyf_mode
		{	CurInstr->opcode = $2; }
		;


/* *** INTEGER MULTIPLY ***********************************************	*/

/* Integer multiply can be either diadic or triadic,			*/
/* both can also be used in a parallel format.				*/

/* Triadic or parallel operations MPYI3.				*/
mpyi_3:
		MPYI3		int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| MPYI3		tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| MPYI3		par_mpyi_mode
		{	CurInstr->opcode = $2; }
		;

/* Diadic or triadic or parallel operations MPYI.			*/
mpyi_2_3:
		MPYI		int_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		| MPYI		int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| MPYI		tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| MPYI		par_mpyi_mode
		{	CurInstr->opcode = $2; }
		;


/* ********************************************************************	*/
/* *** ROTATES ********************************************************	*/
/* Only allow unary register addressing for a 1 bit shift.		*/

rotates:	rotate_ops	reg
		{	CurInstr->opcode = $1->diadic | (( $2 ) << 16)		; }
		;

rotate_ops:	ROL | ROLC | ROR | RORC	;



/* ********************************************************************	*/
/* *** RECIPROCALS ****************************************************	*/
/* Odd in that they cannot use immediate addressing mode.		*/

reciprocals:	recip_ops	Dreg ',' Dreg
		{
			CurInstr->opcode = $1->diadic 				| 		( 0x0   << 21)					 | ( $2 )			 | (( $4 ) << 16)		;
		}
		| recip_ops	direct ',' Dreg
		{
			CurInstr->opcode = $1->diadic 				| 		( 0x1   << 21)					 | (( $4 ) << 16)		;
		}
		| recip_ops	indirect ',' Dreg
		{
			CurInstr->opcode = $1->diadic 				| 		( 0x2   << 21)					 | (( $2 ) << 8)		 | (( $4 ) << 16)		;
		}
		;

recip_ops:	RCPF | RSQRF ;


/* ********************************************************************	*/
/* *** COMPARES *******************************************************	*/
/* Odd in that their 3 operand versions only take 2 operands!		*/

cmp:	cmpi | cmpf | cmpi3 | cmpf3 ;


/*
 * YUK - the triadic cmp's can be confused with the diadic ones
 * as many look the same without the usual triadic trailing destination
 * register. The following addr. modes can only be accessed from diadic
 * addressing instuctions:
 * 	cmp	reg, reg
 *	cmp 	indr, reg
 *	cmp	imm. reg
 *
 * Only allow these instructions if cmpf/i3 is stated explicitly.
 */

/* *** CMPI ***********************************************************	*/

cmpi:
		/* Diadic versions ************************************	*/
		cmpi_ops	int_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }

		/* Triadic / type 1 ***********************************	*/
		| cmpi_ops	reg ',' indirect
		{
			CurInstr->opcode = $1->triadic 				| (0			 | 	(( 0x1  ) << 21)		)  | ( $2 )		 | (( $4 ) << 8)	;
			CurInstr->combine = 	8	;
		}

		/* Triadic / type 2 ***********************************	*/
		| cmpi_ops	immediate ',' indirect
		{
			if (!((( $4 ) & 0xf8 ) == 0x0 ) ) {
				Error("Only *+ARn format indirections allowed for \"cmpi3 immediate, indirect\"");
			}
			CurInstr->opcode = $1->triadic 				| ((1 << 28)		 | 	(( 0x2  ) << 21)		)  | (( $4 ) << 8)	;
			if ($1->token == TSTB)
				CurInstr->combine = 12 ;
			else
				CurInstr->combine = 	11 ;
			CurInstr->combine2 = 	10	;
		}

		/* Triadic / type 1 or 2 ******************************	*/

		/* is 1/0 or 5 bit and if the indirect mod field is of	*/
		/* the right type, generate type 1 or type 2		*/
		/* instruction.						*/

		| cmpi_ops	indirect ',' indirect
		{
			if (((( $2 ) & 0xf8 ) == 0x0 )  && ((( $4 ) & 0xf8 ) == 0x0 ) ) {
				CurInstr->opcode = $1->triadic 					| ((1 << 28)		 | 	(( 0x3  ) << 21)		)  | ( $2 )		 | (( $4 ) << 8)	;

				CurInstr->combine = 	9	;
				CurInstr->combine2 = 	10	;
			}
			else {
				CurInstr->opcode = $1->triadic 					| (0			 | 	(( 0x3  ) << 21)		)  | ( $2 )		 | (( $4 ) << 8)	;
				CurInstr->combine = 	7	;
				CurInstr->combine2 = 	8	;
			}
		}
		;

cmpi_ops:	CMPI | TSTB ;


/* *** CMPF ***********************************************************	*/

cmpf:
		/* Diadic versions ************************************	*/
		CMPF	fp_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }

		/* Triadic / type 1 ***********************************	*/
		| CMPF	Dreg ',' indirect
		{
			CurInstr->opcode = $1->triadic 				| (0			 | 	(( 0x1  ) << 21)		)  | ( $2 )		 | (( $4 ) << 8)	;
			CurInstr->combine = 	8	;
		}

		/* Triadic / type 2 ***********************************	*/
		/* no immediates allowed for floating point */

		/* Triadic / type 1 or 2 ******************************	*/

		/* is 1/0 or 5 bit and if the indirect mod field is of	*/
		/* the right type, generate type 1 or type 2		*/
		/* instruction.						*/

		| CMPF	indirect ',' indirect
		{
			if (((( $2 ) & 0xf8 ) == 0x0 )  && ((( $4 ) & 0xf8 ) == 0x0 ) ) {
				CurInstr->opcode = $1->triadic 					| ((1 << 28)		 | 	(( 0x3  ) << 21)		)  | ( $2 )		 | (( $4 ) << 8)	;

				CurInstr->combine = 	9	;
				CurInstr->combine2 = 	10	;
			}
			else {
				CurInstr->opcode = $1->triadic 					| (0			 | 	(( 0x3  ) << 21)		)  | ( $2 )		 | (( $4 ) << 8)	;
				CurInstr->combine = 	7	;
				CurInstr->combine2 = 	8	;
			}
		}
		;


/* *** CMPI3 **********************************************************	*/

cmpi3:
		/* Triadic / type 1 ***********************************	*/
		cmpi3_ops	reg ',' reg
		{
			CurInstr->opcode = $1->triadic | (0			 | 	(( 0x0  ) << 21)		)  				| ( $2 )		 | (( $4 ) << 8)	;
		}
		| cmpi3_ops	reg ',' indirect
		{
			CurInstr->opcode = $1->triadic | (0			 | 	(( 0x1  ) << 21)		)  				| ( $2 )		 | (( $4 ) << 8)	;
			CurInstr->combine = 	8	;
		}
		/* Triadic / type 2 ***********************************	*/
		| cmpi3_ops	immediate ',' reg
		{
			CurInstr->opcode = $1->triadic | ((1 << 28)		 | 	(( 0x0  ) << 21)		)  | (( $4 ) << 8)	;
			if ($1->token == TSTB3)
				CurInstr->combine = 12 ;
			else
				CurInstr->combine = 	11 ;
		}

		| cmpi3_ops	immediate ',' indirect
		{
			CurInstr->opcode = $1->triadic | ((1 << 28)		 | 	(( 0x2  ) << 21)		)  | (( $4 ) << 8)	;
			if ($1->token == TSTB3)
				CurInstr->combine = 12 ;
			else
				CurInstr->combine = 	11 ;

			if (!((( $4 ) & 0xf8 ) == 0x0 ) )
				Error("Only *+ARn format indirections allowed for type 2 triadic instructions");

			CurInstr->combine2 = 	10	;
		}

		/* Triadic / type 1 or 2 ******************************	*/

		/* @@@ in second pass depending on wether displacement	*/
		/* is 1/0 or 5 bit and if the indirect mod field is of	*/
		/* the right type, generate type 1 or type 2		*/
		/* instruction.						*/

		/* Following mode can only be accessed via diadic mode.	*/
		| cmpi3_ops	indirect ',' reg
		{
			if (((( $2 ) & 0xf8 ) == 0x0 ) ) {
				CurInstr->opcode = $1->triadic 					| ((1 << 28)		 | 	(( 0x1  ) << 21)		)  | ( $2 )		 | (( $4 ) << 8)	;
				CurInstr->combine = 	9	;
			}
			else {
				CurInstr->opcode = $1->triadic 					| (0			 | 	(( 0x2  ) << 21)		)  | ( $2 )		 | (( $4 ) << 8)	;
				CurInstr->combine = 	7	;
			}
		}
		| cmpi3_ops	indirect ',' indirect
		{
			if (((( $2 ) & 0xf8 ) == 0x0 )  && ((( $4 ) & 0xf8 ) == 0x0 ) ) {
				CurInstr->opcode = $1->triadic 					| ((1 << 28)		 | 	(( 0x3  ) << 21)		)  | ( $2 )		 | (( $4 ) << 8)	;
				CurInstr->combine = 	9	;
				CurInstr->combine2 = 	10	;
			}
			else {
				CurInstr->opcode = $1->triadic 					| (0			 | 	(( 0x3  ) << 21)		)  | ( $2 )		 | (( $4 ) << 8)	;
				CurInstr->combine = 	7	;
				CurInstr->combine2 = 	8	;
			}
		}
		;

cmpi3_ops:	CMPI3 | TSTB3;


/* *** CMPF3 **********************************************************	*/

cmpf3:
		/* Triadic / type 1 ***********************************	*/
		CMPF3	Dreg ',' Dreg
		{
			CurInstr->opcode = $1->triadic | (0			 | 	(( 0x0  ) << 21)		)  				| ( $2 )		 | (( $4 ) << 8)	;
		}
		| CMPF3	Dreg ',' indirect
		{
			CurInstr->opcode = $1->triadic | (0			 | 	(( 0x1  ) << 21)		)  				| ( $2 )		 | (( $4 ) << 8)	;
			CurInstr->combine = 	8	;
		}

		/* Triadic / type 2 ***********************************	*/
		/* no immediates allowed for floating point */

		/* Triadic / type 1 or 2 ******************************	*/

		/* @@@ in second pass depending on wether displacement	*/
		/* is 1/0 or 5 bit and if the indirect mod field is of	*/
		/* the right type, generate type 1 or type 2		*/
		/* instruction.						*/

		| CMPF3	indirect ',' Dreg
		{
			if (((( $2 ) & 0xf8 ) == 0x0 ) ) {
				CurInstr->opcode = $1->triadic 					| ((1 << 28)		 | 	(( 0x1  ) << 21)		)  | ( $2 )		 | (( $4 ) << 8)	;
				CurInstr->combine = 	9	;
			}
			else {
				CurInstr->opcode = $1->triadic 					| (0			 | 	(( 0x2  ) << 21)		)  | ( $2 )		 | (( $4 ) << 8)	;
				CurInstr->combine = 	7	;
			}
		}
		| CMPF3	indirect ',' indirect
		{
			if (((( $2 ) & 0xf8 ) == 0x0 )  && ((( $4 ) & 0xf8 ) == 0x0 ) ) {
				CurInstr->opcode = $1->triadic 					| ((1 << 28)		 | 	(( 0x3  ) << 21)		)  | ( $2 )		 | (( $4 ) << 8)	;
				CurInstr->combine = 	9	;
				CurInstr->combine2 = 	10	;
			}
			else {
				CurInstr->opcode = $1->triadic 					| (0			 | 	(( 0x3  ) << 21)		)  | ( $2 )		 | (( $4 ) << 8)	;
				CurInstr->combine = 	7	;
				CurInstr->combine2 = 	8	;
			}
		}
		;



/* ********************************************************************	*/
/* *** FIX - FP to INT ************************************************	*/
/* Odd in that it takes FP src and produces int result.			*/

fix:
		FIX			Dreg
		{
			CurInstr->opcode = $1->diadic | 		( 0x0   << 21)					 				| (( $2 ) << 16)		 | ( $2 )			;
		}
		| FIX			Dreg ',' reg
		{
			CurInstr->opcode = $1->diadic | 		( 0x0   << 21)					 				| (( $4 ) << 16)		 | ( $2 )			;
		}
		| FIX			direct ',' reg
		{
			CurInstr->opcode = $1->diadic | 		( 0x1   << 21)					 				| (( $4 ) << 16)		;
		}
		| FIX			indirect ',' reg
		{
			CurInstr->opcode = $1->diadic | 		( 0x2   << 21)					 				| (( $4 ) << 16)		 | (( $2 ) << 8)		;
		}
		| FIX			fp_immediate ',' reg
		{
			CurInstr->opcode = $1->diadic | 		( 0x3   << 21)					 | $2 				| (( $4 ) << 16)		;
		}

		/* Parallel store *************************************	*/
		| FIX			dia_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;


/* ********************************************************************	*/
/* *** TOIEEE - FP to IEEEE *******************************************	*/
/* Odd in that it doesn't allow immediate addressing mode.		*/

toieee:
		TOIEEE			Dreg
		{
			CurInstr->opcode = $1->diadic | 		( 0x0   << 21)					 				| (( $2 ) << 16)		 | ( $2 )			;
		}
		| TOIEEE		Dreg ',' Dreg
		{
			CurInstr->opcode = $1->diadic | 		( 0x0   << 21)					 				| (( $4 ) << 16)		 | ( $2 )			;
		}
		| TOIEEE		direct ',' Dreg
		{
			CurInstr->opcode = $1->diadic | 		( 0x1   << 21)					 				| (( $4 ) << 16)		;
		}
		| TOIEEE		indirect ',' Dreg
		{
			CurInstr->opcode = $1->diadic | 		( 0x2   << 21)					 				| (( $4 ) << 16)		 | (( $2 ) << 8)		;
		}

		/* Parallel store *************************************	*/
		| TOIEEE		dia_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;


/* ********************************************************************	*/
/* *** FRIEEE - IEEEE to FP *******************************************	*/
/* Odd in that it doesn't allow register OR immediate addressing modes. */

frieee:
		FRIEEE			direct ',' Dreg
		{
			CurInstr->opcode = $1->diadic | 		( 0x1   << 21)					 				| (( $4 ) << 16)		;
		}
		| FRIEEE		indirect ',' Dreg
		{
			CurInstr->opcode = $1->diadic | 		( 0x2   << 21)					 				| (( $4 ) << 16)		 | (( $2 ) << 8)		;
		}

		/* Parallel store *************************************	*/
		| FRIEEE		dia_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;



/* ******************************************************************** */
/* C40 CPU specific linker instruction patches				*/
/* ******************************************************************** */
/* These patches are implemented by the target CPU's linker.		*/
/* They enable instructions to be patched with data that is only	*/
/* available at link time, such as the module number, or offsets into	*/
/* the module table for static data. The patch usually masks in the	*/
/* information into the immediate data area of specific instructions.	*/
/* The instrpatch rule fakes up a Expression structure to allow normal	*/
/* expressions to be used as patch number arguments as well.		*/
/* ******************************************************************** */

instrpatch:	constexpr
		| c40patches
		{
			/* fake up constexpr item with our patch number */
			$$ = NewExprNum($1);
		}
		;

c40patches:
		PATCHC40DATAMODULE1
		| PATCHC40DATAMODULE2
		| PATCHC40DATAMODULE3
		| PATCHC40DATAMODULE4
		| PATCHC40DATAMODULE5
		| PATCHC40MASK8ADD
		| PATCHC40MASK16ADD
		| PATCHC40MASK24ADD
		;



/* end of rules_C40.ypp */


/* CPU_YYRULES: */
/* ******************************************************************** */
/* Machine specific mnemonic grammers and actions			*/
/* ******************************************************************** */
/* Parsed opcodes should increment the logical program counter (curPC)	*/
/* and add themselves to the parse tree.				*/
/* ******************************************************************** */
/* Target CPU specific linker instruction patches			*/
/* ******************************************************************** */
/* These patches are implemented by the target CPU's linker.		*/
/* They enable instructions to be patched with data that is only	*/
/* available at link time, such as the module number, or offsets into	*/
/* the module table for static data. The patch usually masks in the	*/
/* information into the immediate data area of specific instructions.	*/
/* The instrpatch rule fakes up a Expression structure to allow normal	*/
/* expressions to be used as patch number arguments as well.		*/
/* ******************************************************************** */



/* ******************************************************************** */
%%		/* Appended C Code section				*/
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
