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

#ifndef __gasm_h
#define __gasm_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* ********************************************************************
 * Floating point support manifests.
 *
 * Floating point support requires following typedefs (from mip/host.h).
 * These defn's may need alteration for obscure host machines. They are
 * defined at this point as they may be referenced in the processor specific
 * header files.
 */

#ifdef HP	/* Note hosts that have native support for IEEE FP. */
/* Implementations on these hosts provides fast IEEE FP support. */
# define HOST_SUPPORTS_IEEE
#endif

typedef long int            int32;
typedef long unsigned int   unsigned32;
typedef short int           int16;
typedef short unsigned int  unsigned16;
typedef signed char         int8;
typedef unsigned char       unsigned8;
#ifndef __helios_h
typedef int                 bool;
#endif

#ifdef HOST_SUPPORTS_IEEE
	typedef	double	Dble;
#else
	/* IEEE 64 bit integer representation (from mip/defs.h)
	 * The following types describe the representation of floating-point
	 * values by the compiler in both binary and source-related forms.
	 * The order of fields in DbleBin is exploited only in object-code
	 * formatters and assembly code generators.
	 */
	typedef struct {
# ifdef TARGET_HAS_OTHER_IEEE_ORDER
	  int32 lsd,msd;                /* e.g. clipper */
# else
	  int32 msd,lsd;                /* e.g. ARM, 370 (not really IEEE) */
# endif /* TARGET_HAS_OTHER_IEEE_ORDER */
	} DbleBin;

	typedef struct FloatBin {
	  int32 val;
	} FloatBin;

	typedef	DbleBin	Dble ;
#endif /* HOST_SUPPORTS_IEEE */


/* ********************************************************************
 * CPU specific header information
 */

#include CPU_HEADER


/* ********************************************************************
 * Manifests
 */

#ifndef TRUE
# define TRUE 1
# define FALSE 0
#endif

#ifndef NULL
# define NULL 0
#endif


/* ********************************************************************
 * Hash table size - MUST BE a prime number!
 *
 * Some primes are:
 *	19 31 41 53 61 71 83 97 101 151 199 251 307 401 503
 *	509 557 599 653 701 751 809 853 907 953
 *	1009 1499 1999 2503 3001 4001 4999 6007 7001 8009 9001
 *	10007 15013 30011 40009 50021 60013 65521
 */
#define HASHSIZE 1999


/* ********************************************************************	*/
/* Define the different types of data held in the hash table and what	*/
/* should be returned to the parser.					*/
#define HT_LABEL	0	/* return value */
#define HT_PSEUDO	1	/* return token */
#define HT_MNEMONIC	2	/* return token and mnemonic structure ptr */
#define HT_TOKENVAL	3	/* return token and value (regs/patches/etc) */
#define HT_IMPORT	4	/* return error or convert to patch */


/* ********************************************************************	*/
/* Define what type of data is held in Expression or ConstList		*/
/* structures.								*/
#define E_Expr		1
#define E_Num		2
#define E_Patch		3
#define E_Str		4		/* string and label */


/* ********************************************************************	*/
/* Return amount to add to addr to align it to the byte boundary 	*/
/* specified.								*/

#define ALIGNADDR(addr,bytebound) \
		 ((((addr) + (bytebound) - 1) & ~((bytebound) - 1)) - addr)


/* ********************************************************************	*/
/* Return value 'n' rotated to the right by 'r' bits.			*/
/* BEWARE will not work for hosts that do arithmetic right shifts!	*/
 
#define ROTATER(n, r) (((unsigned)(n) << (32 - (r))) | ((unsigned)(n) >> (r)))


/* ********************************************************************	*/
/* Check that value can fit into the specified number of bits.		*/

#define fits_in_5_bits_unsigned( val )	(((val) & ~B_11111) == 0)
#define fits_in_5_bits( val )		(((val) & 0xfffffff0U) == 0 \
					|| ((val) & 0xfffffff0U) == \
					0xfffffff0U)

#define fits_in_8_bits_unsigned( val )	(((val) & 0xffffff00U) == 0 )
#define fits_in_8_bits( val )		(((val) & 0xffffff80U) == 0 \
					|| ((val) & 0xffffff80U) == \
					0xffffff80U)

#define fits_in_16_bits_unsigned( val )	(((val) & 0xffff0000U) == 0 )
#define fits_in_16_bits( val )		(((val) & 0xffff8000U) == 0 \
					|| ((val) & 0xffff8000U) == \
					0xffff8000U)

#define fits_in_24_bits_unsigned( val )	(((val) & 0xff800000U) == 0)
#define fits_in_24_bits( val )		(((val) & 0xff800000U) == 0 \
					|| ((val) & 0xff800000U) == \
					0xff800000U)


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
#ifdef __C40TARGET
extern void		WriteCodeC40Float(Dble d);
#endif

/* Floating point functions */
/*
-- IEEE 64 bit integer arithmetic
*/

#ifndef HOST_SUPPORTS_IEEE	/* Host does NOT support IEEE FP */
extern void fltrep_stod(const char *, DbleBin *) ;
extern bool fltrep_narrow_round(DbleBin *, FloatBin *) ;
extern bool flt_add(DbleBin *, DbleBin *, DbleBin *) ;
extern bool flt_subtract(DbleBin *, DbleBin *, DbleBin *) ;
extern bool flt_multiply(DbleBin *, DbleBin *, DbleBin *) ;
extern bool flt_divide(DbleBin *, DbleBin *, DbleBin *) ;
extern bool flt_negate(DbleBin *, DbleBin *) ;
extern bool flt_itod(DbleBin *, int32) ;
#endif /* HOST_SUPPORTS_IEEE */

/* Misc functions */
extern void 		Fatal(char *s);
extern void 		Error(char *s);
extern void 		Warn(char *s);
extern void 		Note(char *s);
extern char		*itoabin(char *str, int nbits, int bin);

/* debugging functions */
#ifdef DEBUG
extern void 		DebugPrintSymbTab(void);
#endif



#endif /* __gasm_h */

