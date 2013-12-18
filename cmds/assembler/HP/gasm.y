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
 * RcsId: $Id: gasm.ypp,v 1.10 1993/06/22 16:58:53 paul Exp $
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
 * RcsId: $Id: gasm.h,v 1.8 1993/06/22 16:58:53 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */
/* @(#) $Revision: 70.11 $ */
/* $Header: stdsyms.h,v 1.5.61.4 92/06/10 00:57:53 smp Exp $ */





























































































































/* Undefine predefined symbols from cpp which begin with a letter
   (i.e. - Clean up the namespace)
 */












/* buffer size for multi-character output to unbuffered files */


   typedef struct {
	int		 __cnt;
	unsigned char	*__ptr;
	unsigned char	*__base;
	unsigned short	 __flag;
	unsigned char 	 __fileL;		/* low byte of file desc */
	unsigned char 	 __fileH;		/* high byte of file desc */
   } FILE;

   typedef struct {
	int		 __cnt;
	unsigned char	*__ptr;
	unsigned char	*__base;
	unsigned short	 __flag;
	unsigned char 	 __fileL;		/* low byte of file desc */
	unsigned char 	 __fileH;		/* high byte of file desc */
	unsigned char	*__bufendp;	/* end of buffer */
	unsigned char	 __smbuf[ 8 ]; /* small buffer */
   } _FILEX;

/*
 * _IOLBF means that a file's output will be buffered line by line
 * In addition to being flags, _IONBF, _IOLBF and _IOFBF are possible
 * vales for "type" in setvbuf
 */


















     typedef unsigned int size_t;


   typedef long int fpos_t;






     typedef double *__va_list;




























   extern FILE __iob[];


     extern int remove(const char *);
     extern int rename(const char *, const char *);
     extern FILE *tmpfile(void);
     extern char *tmpnam(char *);
     extern int fclose(FILE *);
     extern int fflush(FILE *);
     extern FILE *fopen(const char *, const char *);
     extern FILE *freopen(const char *, const char *, FILE *);
     extern void setbuf(FILE *, char *);
     extern int setvbuf(FILE *, char *, int, size_t);
     extern int fprintf(FILE *, const char *, ...);
     extern int fscanf(FILE *, const char *,...);
     extern int printf(const char *,...);
     extern int scanf(const char *,...);
     extern int sprintf(char *, const char *,...);
     extern int sscanf(const char *, const char *,...);
     extern int vprintf(const char *, __va_list);
     extern int vfprintf(FILE *, const char *, __va_list);
     extern int vsprintf(char *, const char *, __va_list);
     extern int fgetc(FILE *);
     extern char *fgets(char *, int, FILE *);
     extern int fputc(int, FILE *);
     extern int fputs(const char *, FILE *);
     extern int getc(FILE *);
     extern int getchar(void);
     extern char *gets(char *);
     extern int putc(int, FILE *);
     extern int putchar(int);
     extern int puts(const char *);
     extern int ungetc(int, FILE *);
     extern int fgetpos(FILE *, fpos_t *);
     extern int fseek(FILE *, long int, int);
     extern int fsetpos(FILE *, const fpos_t *);
     extern long int ftell(FILE *);
     extern void rewind(FILE *);
     extern void clearerr(FILE *);
     extern int feof(FILE *);
     extern int ferror(FILE *);
     extern void perror(const char *);















































       extern size_t fread(void *, size_t, size_t, FILE *);
       extern size_t fwrite(const void *, size_t, size_t, FILE *);






/*
 * WARNING:  The following function declarations are provided solely
 *  for use in the macros found in this header file.  These are HP-UX
 *  specific routines used internally and direct use of these routines 
 *  is not supported.  These routines may be removed or significantly
 *  changed in future releases of HP-UX.
 */

     extern int __flsbuf(unsigned char, FILE *);
     extern int __filbuf(FILE *);
/* @(#) $Revision: 70.15 $ */















   extern int __nl_char_size;






   typedef struct {
	int quot;	/* quotient */
	int rem;	/* remainder */
   } div_t;
   typedef struct {
	long int quot;	/* quotient */
	long int rem;	/* remainder */
   } ldiv_t;








     typedef unsigned int wchar_t;





       extern double atof(const char *);

     extern int atoi(const char *);
     extern long int atol(const char *);
     extern double strtod(const char *, char **);
     extern long int strtol(const char *, char **, int);
     extern unsigned long int strtoul(const char *, char **, int);
     extern int rand(void);
     extern void srand(unsigned int);
     extern int atexit(void (*) (void));
     extern void exit(int);
     extern char *getenv(const char *);
     extern int system(const char *);

       extern int abs(int);





     extern div_t div(int, int);
     extern ldiv_t ldiv(long int, long int);
     extern long int labs(long int);
     extern int mblen(const char *, size_t);
     extern int mbtowc(wchar_t *, const char *, size_t);
     extern int wctomb(char *, wchar_t);
     extern size_t mbstowcs(wchar_t *, const char *, size_t);
     extern size_t wcstombs(char *, const wchar_t *, size_t);
     extern void free(void *);
     extern void qsort(void *, size_t, size_t, int (*)(const void *, const void *));





































       extern void abort(void);
       extern void *bsearch(const void *, const void *, size_t, size_t, int (*) (const void *, const void *));
       extern void *calloc(size_t, size_t);
       extern void *malloc(size_t);
       extern void *realloc(void *, size_t);
/* @(#) $Revision: 70.6 $ */






















     extern int memcmp(const void *, const void *, size_t);
     extern char *strncat(char *, const char *, size_t);
     extern int strncmp(const char *, const char *, size_t);
     extern void *memmove(void *, const void *, size_t);
     extern char *strcpy(char *, const char *);
     extern char *strncpy(char *, const char *, size_t);
     extern char *strcat(char *, const char *);
     extern int strcmp(const char *, const char *);
     extern int strcoll(const char *, const char *);
     extern size_t strxfrm(char *, const char *, size_t);
     extern char *strchr(const char *, int);
     extern char *strpbrk(const char *, const char *);
     extern char *strrchr(const char *, int);
     extern char *strstr(const char *, const char *);
     extern char *strtok(char *, const char *);
     extern char *strerror(int);




























       extern void *memcpy(void *, const void *, size_t);
       extern void *memchr(const void *, int, size_t);
       extern void *memset(void *, int, size_t);
       extern size_t strcspn(const char *, const char *);
       extern size_t strspn(const char *, const char *);



          extern size_t strlen(const char *);


/* ********************************************************************
 * Floating point support manifests.
 *
 * Floating point support requires following typedefs (from mip/host.h).
 * These defn's may need alteration for obscure host machines. They are
 * defined at this point as they may be referenced in the processor specific
 * header files.
 */


/* Implementations on these hosts provides fast IEEE FP support. */



typedef long int            int32;
typedef long unsigned int   unsigned32;
typedef short int           int16;
typedef short unsigned int  unsigned16;
typedef signed char         int8;
typedef unsigned char       unsigned8;

typedef int                 bool;



	typedef	double	Dble;























/* ********************************************************************
 * CPU specific header information
 */
/*
 * File:	header_ARM.h
 * Subsystem:	Generic (ARM) Assembler
 * Author:	P.A.Beskeen
 * Date:	Sept '91
 *
 * Description: ARM specific header file for generic Helios assembler
 *
 * RcsId: $Id: hdr_ARM.h,v 1.5 1993/06/22 16:58:53 paul Exp $
 *
 * (C) Copyright 1992 Paul Beskeen
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
/* *** ARM Processor Specific Structures ******************************	*/


/* ********************************************************************	*/
/* Mnemonic Structure							*/
/*									*/
/* The Mnemonic structure is attached to all Symbol structures in the	*/
/* hash table that correspond to CPU specific mneumonic keywords.	*/
/* (type = HT_MNEMONIC)							*/
/*									*/
/* The decoding of Condition codes is done in the ARM lexer, not the	*/
/* parser. This is because they are entered as part of the mnemonic	*/
/* keyword and it is therfore easier to decode them at this point.	*/


typedef struct Mnemonic {
	char		*name;		/* mneumonics text name		*/
	int		token;		/* parser token number		*/
	unsigned	template;	/* binary instruction template	*/
} Mnemonic;


/* ********************************************************************	*/
/* Instruction Structure						*/
/*									*/
/* Instruction is a Target CPU dependent structure that defines the	*/
/* machine opcode to output. It may also contain expressions that need	*/
/* to be evaluated and combined with the instruction before it is	*/
/* This structure is held in the parse tree.				*/


typedef struct Instruction {
	int			opcode;		/* The opcode to output	    */
	struct Expression	*optexpr;	/* Optionally insert this   */
						/* expr into the opcode     */
	int			combine;	/* How to combine the	    */
						/* optexpr with the opcode  */
	/* optexpr 2 is only used for the CDO, LDC/STC and MRC/MCR instr's. */
	/* and is always used to store the coprocessor number (bits 8-11) */
	struct Expression	*optcoprocnum;	/* Optionally insert this   */
						/* expr into the opcode     */
	/* optexpr 3 is only used for the CDO and MRC/MCR instructions */
	/* and is always used to store the optional aux field (bits 5-7) */
	struct Expression	*optcoprocaux;	/* Optionally insert this   */
						/* expr into the opcode     */
} Instruction;


/* ********************************************************************	*/
/* Combine Commands							*/

/* default - no checking */


/* check 24 bit pcrel value, left shift by two and subtract 2 for pipeline */


/* Check immediate can be encoded into 8 bits with a 4*2 bit rotate and */
/* insert into bits 0-7 + 8-11 */


/* Check unsigned shift value < 31/32 (converting shift type if required) */
/* and insert into bits 7-11 */


/* Check immediate <= 8 bits and insert into bits 0-7 */


/* Check if fits into 12 bits unsigned - if positive bit 23 U should be set */
/* insert into bits 0-11 */


/* Check if fits into 8 bits unsigned - if positive bit 23 U should be set */
/* insert into bits 0-7 */


/* Check if fits into 12 bits unsigned and insert into bits 0-11 */
/* If negative, ABS value and reset bit 23 U (sign bit). */
/* If positive bit 23 U should be set. */
/* Sub 8 to adjust for pipeline */


/* Check if fits into 8 bits unsigned and insert into bits 0-7 */
/* If negative, ABS value and reset bit 23 U (sign bit). */
/* If positive bit 23 U should be set. */
/* Sub 8 to adjust for pipeline */


/* Check expression will fit in 24 bits and insert into bits 0-23 */


/* Check expression will fit in 4 bits and insert into bits 20-23 */


/* Check expression will fit in 3 bits and insert into bits 21-23 */


/* After evaluation the constant should have eight subtracted from it */
/* (to take account of the pipeline). Check this value can be encoded */
/* into 8 bits with a 4*2 bit rotate and insert into bits 0-7 + 8-11 */



/* ********************************************************************	*/
/* Register values							*/

/* Standard register file regs: */

















/* coprocessor registers: */


















/* ********************************************************************	*/
/* Condition codes [bits 31 - 28]					*/

/*
 * For all possible conditions, define the binary code to entered into the
 * cond field of the instruction.
 */






















/* ********************************************************************	*/
/* SHIFT TYPES used in data processing and transfer instructions (bits 5/6) */













/* ********************************************************************	*/
/* Common Instruction Definitions					*/






/* index base used in SWP, single, multiple and coproc data transfer instr's */




/* ********************************************************************	*/
/* Individual Instruction Opcode Definitions				*/
/* ********************************************************************	*/


/* ********************************************************************	*/
/* Branch op-codes (bits 27 - 25 => 101)				*/





/* ********************************************************************	*/
/* Data Processing op-codes (bits 27 - 26 => 00)			*/




















/* Flags */


					/* default reg/shift */


/* opcode fields */







/* ********************************************************************	*/
/* New ARM6 PSR TRANSFER op-codes (subset of existing dataproc instr.s)	*/

/* ARM6 ops MSR MRS (use of TST, TEQ, CMP and CMN ops without 'S' bit set) */








/* set PSR current or saved register selection */





/* ********************************************************************	*/
/* MULTIPLY op-codes (bits 27 - 22 => 000000, bits 7 - 4 => 1001)	*/




/* Flags */



/* opcode fields */







/* ********************************************************************	*/
/* Single Data Transfer op-codes (bits 27 - 26 => 01)			*/




/* Flag bits */




/* opcode fields */




/* ********************************************************************	*/
/* Block Data Transfer op-codes (bits 27 - 25 => 100)			*/




/* Flag bits */





/* ********************************************************************	*/
/* SoftWare Interrupt (TRAP) op-codes (bits 27 - 24 => 1111)		*/




/* ********************************************************************	*/
/* Single Data Swap instruction (ARM3) (bits 27 - 23 => 00010)		*/





/* opcode fields */





/* ********************************************************************	*/
/* Common coprocessor instruction manifests				*/





/* ********************************************************************	*/
/* Coprocessor Data Operations (bits 27 - 24 => 1110, bit 4 => 0)	*/



/* opcode fields */







/* ********************************************************************	*/
/* Coprocessor Data Transfers (bits 27 - 25 => 110)			*/




/* Flag bits */



/* opcode fields */





/* ********************************************************************	*/
/* Coprocessor Register Transfers  (bits 27 - 24 => 1110, bit 4 => 1)	*/




/* opcode fields */






/* ********************************************************************	*/
/* LEA Load Effective Address pseudo-opcode				*/
/* Creates either 'sub pc, symb + 8', or 'add pc symb - 8'		*/


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
			int	operator;
			struct Expression *left;/* NULL if operator is unary */
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




/* Floating point functions */
/*
-- IEEE 64 bit integer arithmetic
*/












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
 * File:	toks_ARM.ypp
 * Subsystem:	Generic (ARM) Assembler
 * Author:	P.A.Beskeen
 * Date:	Dec '91 - Aug '92
 *
 * Description: Defines the tokens expected by the ARM specific parts of
 *		the parser.
 *
 *
 * RcsId: $Id: toks_ARM.ypp,v 1.4 1993/06/22 16:58:53 paul Exp $
 *
 * (C) Copyright 1992 Perihelion Software
 * 
 * $RcsLog$
 *
 */



/* ******************************************************************** */
/* ARM Parser Specific C Declarations 					*/
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
	Instruction	*CurInstr =  0 ;
%}



/* ******************************************************************** */
/* ARM specific mnemonic tokens						*/

%token	<mnem>	B BL
%token	<mnem>	AND EOR SUB RSB ADD ADC SBC RSC ORR BIC
%token	<mnem>	TST TEQ CMP CMN
%token	<mnem>	MOV MVN
%token	<mnem>	MUL MLA
%token	<mnem>	LDR STR
%token	<mnem>	LDM STM
%token	<mnem>	SWI
%token	<mnem>	SWP
%token	<mnem>	CDO LDC STC MRC MCR
%token	<mnem>	LEA NOP
%token	<mnem>	MRS MSR


/* ******************************************************************** */
/* ARM specific linker instruction patch tokens				*/

%token	<num>	PATCHARMDT PATCHARMDP PATCHARMJP
%token	<num>	PATCHARMDPLSB PATCHARMDPMID PATCHARMDPREST


/* ******************************************************************** */
/* ARM Register Tokens							*/

%token	<num>	R0 R1 R2 R3 R4 R5 R6 R7 R8 R9 R10 R11 R12 R13 R14 R15


/* ******************************************************************** */
/* ARM Coprocessor Register Tokens							*/

%token	<num>	CR0 CR1 CR2 CR3 CR4 CR5 CR6 CR7 CR8 CR9 CR10 CR11 CR12
%token	<num>	CR13 CR14 CR15


/* ******************************************************************** */
/* Extra ARM Tokens							*/

%token	<num>	LSL LSR ASR ROR

%token	<num>	RRX

%token	<num>	CPSR SPSR CPSR_FLG SPSR_FLG

/* ******************************************************************** */
/* Internal rule return types						*/

%type <num>	reg
%type <num>	cp_reg

%type <num>	op2 shifttype optshift shiftname reg_expr
%type <num>	addr postindex preindex
%type <num>	optclaret optpling indexreg

%type <num>	reglist regs
%type <num>	armpatches
%type <num>	cpsr cpsr_flg

%type <mnem>	branch_ops move_ops logic_ops comp_ops data_ops ldstmult_ops
%type <mnem>	cdt_ops crt_ops

%type <expr>	optaux armconstexpr
%type <num>	cp_addr cp_postindex cp_preindex



/* end of toks_ARM.ypp */


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




				Symbol *s = NewSymb($1,  0 , curPC);


				/* add symbol table ref to parse tree */
				/* this will output an object code label */
				ParseTreeItem *pti = NewParseTreeItem(LABEL);

				pti->type.symb = s;










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
			pad =  ((((curPC) + (($2) ? $2 : 4) - 1) & ~((($2) ? $2 : 4) - 1)) - curPC) ;

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
			while ( n !=  0 ) {
				NewSymb(n->name,  4 , 0);
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
			NameList *n = malloc(sizeof(NameList));

			if (n ==  0 )
				Fatal("Out of memory whilst building name list");

			n->next =  0 ;
			n->name = $1;
			$$ = n;
		}
		| namelist ',' NAME
		{
			/* list of names */
			NameList *n = $1;
			NameList *nn = malloc(sizeof(NameList));

			if (nn ==  0 )
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
			while (n->next !=  0 )
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
			while (fl->next !=  0 )
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
			$$ = NewConstItem( 1 );

			$$->type.expr = $1;
			list_size_count++;
		}
		| stdpatch
		{
			/* return a patch constlist item */
			$$ = NewConstItem( 3 );

			$$->type.patch = $1;
			list_size_count++;
		}
		| STRINGCONST
		{
			/* return a string constlist item */
			$$ = NewConstItem( 4 );

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
		{	$$ = $1 + $3; }
		| imm_fp_constexpr '-' imm_fp_constexpr
		{	$$ = $1 - $3; }
		| imm_fp_constexpr '*' imm_fp_constexpr
		{	$$ = $1 * $3; }
		| imm_fp_constexpr '/' imm_fp_constexpr
		{	$$ = $1 / $3; }
		| '-' imm_fp_constexpr %prec UNARY
		{	$$ = - ($2); }

		/* just return value of number */
		| NUMBER
		{ $$ = (double)$1;}




















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
		{	$$ = NewExpr( 0 , '~', $2); }
		| '-' constexpr %prec UNARY
		{	$$ = NewExpr( 0 , '-', $2); }
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




			$$ =  0 ;
		}
		| constexpr
		;










/* include CPU specific mneumonic and patch rules */
/*
 * File:	rules_ARM.c
 * Subsystem:	Generic (ARM) Assembler
 * Author:	P.A.Beskeen
 * Date:	Dec '91 - Aug '92
 *
 * Description: YACC grammer rules and actions to implement the ARM
 *		specific parts of the parser.
 *
 *
 * RcsId: $Id: rules_ARM.ypp,v 1.5 1993/06/22 16:58:53 paul Exp $
 *
 * (C) Copyright 1992 Perihelion Software
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


/* *** REGISTER ADDRESSING ********************************************	*/

reg:		R0 | R1 | R2 | R3 | R4 | R5 | R6 | R7 |
		R8 | R9 | R10 | R11 | R12 | R13 | R14 | R15
		| error
		{	Error("Expecting an ARM register");	}
		;

cp_reg:		CR0 | CR1 | CR2 | CR3 | CR4 | CR5 | CR6 | CR7 |
		CR8 | CR9 | CR10 | CR11 | CR12 | CR13 | CR14 | CR15
		| error
		{	Error("Expecting a coprocessor register");	}
		;



/* ******************************************************************** */
/* ARM specific mnemonic grammers and actions				*/
/* ******************************************************************** */
/* Parsed opcodes should increment the logical program counter (curPC)	*/
/* and add themselves to the parse tree.				*/
/* ******************************************************************** */


machine_op:	{
			/* create instr. template to be filled in by parser */
			if ((CurInstr = malloc(sizeof(Instruction))) == 0)
				Fatal("Out of Memory for new instruction");

			/* initialise instruction template */
			CurInstr->opcode = 0;
			CurInstr->optexpr =  0 ;
			CurInstr->combine = 0;
			CurInstr->optcoprocnum =  0 ;
			CurInstr->optcoprocaux =  0 ;
		}
		ARM_op	/* match 'ARM mneumonics */
		{
			/* add new instruction into the parse tree */
			ParseTreeItem *pti = NewParseTreeItem(INSTRUCTION);
			pti->type.instr = CurInstr;

			curPC += sizeof(int);
		}
		;

ARM_op:
	branch
	| dataproc
	| arm6psr
	| multiply
	| datatran
	| ldstmult
	| softintr
	| swap
	| coproc
	| pseudo
	;



/* *** BRANCH INSTRUCTIONS ********************************************	*/

branch:		branch_ops armconstexpr
		{
			CurInstr->opcode = $1->template;
			CurInstr->optexpr = $2;

			/* specifies eval and check of 24 bit pcrel value */
			CurInstr->combine =  1 ;
		}
		;

branch_ops:	B | BL ;


/* *** DATA PROCESSING INSTRUCTIONS ***********************************	*/

dataproc:	move | comp | logic ;

move:		move_ops reg ',' op2
		{
			CurInstr->opcode = $1->template |  (($2) << 12)  | $4;
		}
		;

comp:		comp_ops reg ',' op2
		{
			CurInstr->opcode = $1->template |  (($2) << 16)  | $4;
		}
		;

logic:		logic_ops reg ',' reg ',' op2
		{
			CurInstr->opcode = $1->template | 				 (($2) << 12)  |  (($4) << 16)  | $6;

		}
		| logic_ops reg ',' op2		/* shorthand src=dst */
		{
			CurInstr->opcode = $1->template | 				 (($2) << 12)  |  (($2) << 16)  | $4;

		}
		;

op2:		shifttype
		{	$$ = $1; }
		| armconstexpr
		{
			/* after evaluation the value must be formed into an */
			/* eight bit value + a rotation 4*2 rotation */
			CurInstr->optexpr = $1;
			CurInstr->combine =  2 ;
			/* set immediate mode in instruction */
			$$ =  (1 << 25) ;
		}
		;

shifttype:	reg
		{	$$ = $1; }
		| reg shiftname reg
		{	$$ =  (1 << 4)  | $1 | $2 |  (($3) << 8) ; }
		| reg shiftname armconstexpr
		{
			$$ = $1 | $2;
			CurInstr->optexpr = $3;
			CurInstr->combine =  3 ;
		}
		| reg RRX
		{	$$ = $1 | $2; }
		;

shiftname:	LSL | LSR | ASR | ROR ;

comp_ops:	TST | TEQ | CMP | CMN ;

move_ops:	MVN | MOV ;

logic_ops:	AND | EOR | SUB | RSB | ADD | ADC | SBC | RSC | ORR | BIC ;


/* *** ARM6 PSR INSTRUCTIONS ******************************************	*/

arm6psr:	mrs | msr ;

mrs:		MRS	reg ',' cpsr
		{
/*fprintf(stderr, "mrs opcode = %x, dstreg %x, cpsr %x\n", $1, $2, $4);*/
			CurInstr->opcode = $1->template |  (($2) << 12)  | $4;
		}
		;

msr:		MSR	cpsr_flg ',' reg_expr
		{
/*fprintf(stderr, "msr opcode = %x, cpsr %x, reg_expr %x\n", $1, $2, $4);*/
			if (($2 &  (1 << 16) ) && ($4 &  (1 << 25) )) {
				/* Cannot use immediate mode with full psr */
				Error("MSR instruction can only set psr flags with immediate operand");
			}
			CurInstr->opcode = $1->template | $2 | $4;
		}
		;

reg_expr:	reg
		{	$$ = $1; }
		| armconstexpr
		{
			/* after evaluation the value must be formed into an */
			/* eight bit value + a rotation 4*2 rotation */
			CurInstr->optexpr = $1;
			CurInstr->combine =  2 ;
			/* set immediate mode in instruction */
			$$ =  (1 << 25) ;
		}
		;

cpsr:		CPSR | SPSR ;

cpsr_flg:	CPSR | CPSR_FLG | SPSR | SPSR_FLG ;


/* *** MULTIPLY INSTRUCTIONS ******************************************	*/

multiply:	MUL	reg ',' reg ',' reg
		{
			CurInstr->opcode = $1->template | 				 (($2) << 16)  | 				 ($4)  | 				 (($6) << 8) ;




			if ($2 == $4)
				Error("Destination and first source operand "
				"must not be the same");

			if ($2 ==  0xF )
				Warn("Destination of R15 will have no "
				"effect, other than altering the PSR flags");

		}
		| MLA	reg ',' reg ',' reg ',' reg
		{
			CurInstr->opcode = $1->template | 				 (($2) << 16)  | 				 ($4)  | 				 (($6) << 8)  | 				 (($8) << 12) ;





			if ($2 == $4)
				Error("Destination and first source operand "
				"must not be the same");

			if ($2 ==  0xF )
				Warn("Destination of R15 will have no "
				"effect, other than altering the PSR flags");
		}
		;



/* *** DATA TRANSFER INSTRUCTIONS *************************************	*/

datatran:	data_ops reg ',' addr
		{	CurInstr->opcode = $1->template |  (($2) << 12)  | $4; }
		;

addr:		armconstexpr	/* twelve bit */
		{
			CurInstr->optexpr = $1;
			CurInstr->combine =  5 ;
			/* preindex address relative to the PC */
			$$ =  ((0xF) << 16)  |  (1 << 24) ;
		}
		| preindex
		| postindex
		;

preindex:	'[' reg ']' optpling
		{
			/* as the offset is zero, make it a positive offset */
			/* (INDEXUP) so we don't see '-0' in disassemblies */
			$$ =  (1 << 24)  |  (($2) << 16)  |  (1 << 23)  | $4;
		}
		| '(' reg ')' optpling
		{
			$$ =  (1 << 24)  |  (($2) << 16)  |  (1 << 23)  | $4;
		}
		| '[' reg ',' armconstexpr ']' optpling
		{
			CurInstr->optexpr = $4;
			CurInstr->combine =  5 ;
			$$ =  (1 << 24)  |  (($2) << 16)  | $6;
		}
		| '(' reg ',' armconstexpr ')' optpling
		{
			CurInstr->optexpr = $4;
			CurInstr->combine =  5 ;
			$$ =  (1 << 24)  |  (($2) << 16)  | $6;
		}
		| '[' reg ',' indexreg optshift ']' optpling
		{
			$$ =  (1 << 24)  |  (1 << 25)  |  (($2) << 16)  |
			     $4 | $5 | $7;
		}
		| '(' reg ',' indexreg optshift ')' optpling
		{
			$$ =  (1 << 24)  |  (1 << 25)  |  (($2) << 16)  |
			     $4 | $5 | $7;
		}
		;

postindex:	'[' reg ']' ',' armconstexpr
		{
			CurInstr->optexpr = $5;
			CurInstr->combine =  5 ;
			$$ =  (($2) << 16) ;
		}
		| '(' reg ')' ',' armconstexpr
		{
			CurInstr->optexpr = $5;
			CurInstr->combine =  5 ;
			$$ =  (($2) << 16) ;
		}
		| '[' reg ']' ',' indexreg optshift
		{	$$ =  (1 << 25)  |  (($2) << 16)  | $5 | $6; }
		| '(' reg ')' ',' indexreg optshift
		{	$$ =  (1 << 25)  |  (($2) << 16)  | $5 | $6; }
		;

indexreg:	reg
		{	$$ =  (1 << 23)  | $1; }	/* default to add offset */
		| '+' reg
		{	$$ =  (1 << 23)  | $2; }
		| '-' reg
		{	$$ = $2; }
		;

		/* similar to shifttype, but without the reg shift */
optshift:	/* empty */
		{	$$ = 0; }
		| shiftname armconstexpr
		{
			CurInstr->optexpr = $2;
			CurInstr->combine =  3 ;
			$$ = $1;
		}
		| RRX
		;

/* optpling used by ARM data transfer and coprocessor data transfer instr. */
optpling:	/* empty */
		{	$$ = 0; }
		| '!'
		{	$$ =  (1 << 21) ; }
		;

data_ops:	LDR | STR ;



/* *** LOAD STORE MULTIPLE REGISTER INSTRUCTIONS **********************	*/

ldstmult:	ldstmult_ops	reg optpling ',' reglist optclaret
		{	CurInstr->opcode = $1->template |  (($2) << 16)  				| $3 | $5 | $6;}

		;

reglist:	reg
		{	$$ = 1 << $1; }
		| '{' regs '}'
		{	$$ = $2; }
		;

regs:		reg
		{	$$ = 1 << $1; }
		| reg '-' reg
		{
			int i;

			$$ = 0;

			if ($1 < $3) {
				for (i = $1; i <= $3; i++)
					$$ |= 1 << i;
			} else {
				for (i = $3; i <= $1; i++)
					$$ |= 1 << i;
			}
		}
		| reg ',' regs
		{	$$ = 1 << $1 | $3; }
		| reg '-' reg ',' regs
		{
			int i;

			$$ = 0;

			if ($1 < $3) {
				for (i = $1; i <= $3; i++)
					$$ |= 1 << i;
			} else {
				for (i = $3; i <= $1; i++)
					$$ |= 1 << i;
			}

			$$ |= $5;
		}
		;

optclaret:	/* empty */
		{	$$ = 0; }
		| '^'
		{	$$ =  (1 << 22) ; }
		;

ldstmult_ops:	LDM
		{
			$$ = $1;
			if ($1->template ==  0 )
				Error("LDM mnemonic must include a mode");
		}
		| STM
		{
			$$ = $1;

			if ($1->template ==  0 )
				Error("STM mnemonic must include a mode");
		}
		;



/* *** SOFTWARE INTERRUPT INSTRUCTION *********************************	*/

softintr:	SWI	armconstexpr
		{
			CurInstr->opcode = $1->template;
			CurInstr->optexpr = $2;
			CurInstr->combine =  9 ;
		}
		;



/* *** SWAP INSTRUCTION ***********************************************	*/

swap:		SWP reg ',' reg ',' '[' reg ']'
		{
			CurInstr->opcode = $1->template |  (($2) << 12)  |
				 ($4)  |  (($7) << 16)  ;
		}
		| SWP reg ',' reg ',' '(' reg ')'
		{
			CurInstr->opcode = $1->template |  (($2) << 12)  |
				 ($4)  |  (($7) << 16)  ;
		}
		;



/* *** COPROCESSOR INSTRUCTIONS ***************************************	*/

		/* data operations, data transfers and register transfers */
coproc:		cdo | cdt | crt ;


cdo:		CDO armconstexpr ',' armconstexpr ',' cp_reg ',' cp_reg ',' cp_reg optaux
		{
			CurInstr->opcode = $1->template |  (($6) << 12)  |
				 (($8) << 16)  |  ($10) ;

			CurInstr->optexpr = $4; /* CP Opc */
			CurInstr->combine =  10 ;

			CurInstr->optcoprocnum = $2; /* CP# */
			CurInstr->optcoprocaux = $11; /* AUX expr */
		}
		;

cdt:		cdt_ops armconstexpr ',' cp_reg ',' cp_addr
		{
			CurInstr->opcode = $1->template |  (($4) << 12)  | $6;
			/* CurInstr->optexpr set by cp_addr rule */
			CurInstr->optcoprocnum = $2; /* CP# */
		}
		;


cdt_ops:	LDC | STC ;


crt:		crt_ops armconstexpr ',' armconstexpr ',' reg ',' cp_reg ',' cp_reg optaux
		{
			CurInstr->opcode = $1->template |  (($6) << 12)  |
				 (($8) << 16)  |  ($10) ;

			CurInstr->optexpr = $4; /* CP Opc */
			CurInstr->combine =  11 ;

			CurInstr->optcoprocnum = $2; /* CP# */
			CurInstr->optcoprocaux = $11; /* AUX expr */
		}


crt_ops:	MCR | MRC ;


optaux:		/* empty */
		{	$$ =  0 ; }
		| ',' armconstexpr
		{	$$ = $2; }
		;


/* resticted form of indirect addressing - only 8 bit offset and no shifts */
cp_addr:	armconstexpr	/* 8 bit */
		{
			CurInstr->optexpr = $1;
			CurInstr->combine =  6 ;
			/* preindex address relative to the PC */
			$$ =  ((0xF) << 16)  |  (1 << 24) ;
		}
		| cp_preindex
		| cp_postindex
		;

cp_preindex:	'[' reg ']' optpling
		{
			/* as the offset is zero, make it a positive offset */
			/* (INDEXUP) so we don't see '-0' in disassemblies */
			$$ =  (1 << 24)  |  (($2) << 16)  |  (1 << 23)  | $4;
		}
		| '(' reg ')' optpling
		{
			$$ =  (1 << 24)  |  (($2) << 16)  |  (1 << 23)  | $4;
		}
		| '[' reg ',' armconstexpr ']' optpling
		{
			CurInstr->optexpr = $4;
			CurInstr->combine =  6 ;
			$$ =  (1 << 24)  |  (($2) << 16)  | $6;
		}
		| '(' reg ',' armconstexpr ')' optpling
		{
			CurInstr->optexpr = $4;
			CurInstr->combine =  6 ;
			$$ =  (1 << 24)  |  (($2) << 16)  | $6;
		}
		;

cp_postindex:	'[' reg ']' ',' armconstexpr optpling
		{
			CurInstr->optexpr = $5;
			CurInstr->combine =  6 ;
			$$ =  (($2) << 16)  | $6;
		}
		| '(' reg ')' ',' armconstexpr optpling
		{
			CurInstr->optexpr = $5;
			CurInstr->combine =  6 ;
			$$ =  (($2) << 16)  | $6;
		}
		;


/* *** PSEUDO OPCODES *************************************************	*/

pseudo:		LEA	reg ',' constexpr
		{
			/* LEA is used to load the absolute address of a */
			/* label at run time. This version is currently */
			/* limited to the addressability of one data */
			/* processing add or sub. */

			/* note constexpr, not armconstexpr as we don't allow */
			/* non pc relative constants in LEA's */

			/* set PC as src and immediate mode in instruction */
			CurInstr->opcode = $1->template |  (($2) << 12)  					|  ((0xF) << 16)  |  (1 << 25) ;


			/* After evaluation the constant should have eight */
			/* subtracted from it (to take account of the */
			/* pipeline). This value must then be formed into */
			/* an eight bit value + a rotation 4*2 rotation */

			CurInstr->optexpr = $4;
			CurInstr->combine =  12 ;
		}
		| NOP
		{
			/* NOP is simply a mov r0, r0 */
			CurInstr->opcode = $1->template;
		}
		;



/* For compatibility with old ARM assemblers, we allow an optional syntax */
/* where any numeric constant can be prefixed by a '#' */
armconstexpr:	'#' constexpr
		{	$$ = $2; }
		| constexpr		/* defaults to $$ = $1 */
		;


/* ******************************************************************** */
/* ARM specific linker instruction patches				*/
/* ******************************************************************** */
/* These patches are implemented by the target CPU's linker.		*/
/* They enable instructions to be patched with data that is only	*/
/* available at link time, such as the module number, or offsets into	*/
/* the module table for static data. The patch usually masks in the	*/
/* information into the immediate data area of specific instructions.	*/
/* The instrpatch rule fakes up a Expression struture to allow normal	*/
/* expressions to be used as patch number arguments as well.		*/
/* ******************************************************************** */


instrpatch:
		armconstexpr
		| armpatches
			/* fake up constexpr item with our patch number */
		{	$$ = NewExprNum($1);}
		;


armpatches:
		PATCHARMDT
		| PATCHARMDP
		| PATCHARMJP
		| PATCHARMDPLSB
		| PATCHARMDPMID
		| PATCHARMDPREST
		;



/* end of rules_arm.ypp */

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
	ParseTreeItem *pti = malloc(sizeof(ParseTreeItem));

	if (pti ==  0 )
		Fatal("Out of memory whilst building parse tree");

	pti->what = type;
	pti->next =  0 ;

	/* mnemonic's line number for accurate error reports in second pass */
	pti->linenum = StartLine;

	/* remember mnemonic's PC for use in pc relative label computation */



	pti->logicalPC = curPC;		/* luverly BYTE address machines */

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
	ConstList *cl = malloc(sizeof(ConstList));

	if (cl ==  0 )
		Fatal("Out of memory whilst building new constant");

	cl->what = type;
	cl->next =  0 ;




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
	FloatList *fl = malloc(sizeof(FloatList));

	if (fl ==  0 )
		Fatal("Out of memory whilst building new float constant");

	fl->next =  0 ;
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
	Patch *p = malloc(sizeof(Patch));

	if (p ==  0 )
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
	Expression *e = malloc(sizeof(Expression));

	if (e ==  0 )
		Fatal("Out of memory defining expression");

	e->what =  1 ;

	e->type.expr.operator = op;
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
	Expression *e = malloc(sizeof(Expression));

	if (e ==  0 )
		Fatal("Out of memory defining expression");

	e->what =  2 ;

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
	Expression *e = malloc(sizeof(Expression));

	if (e ==  0 )
		Fatal("Out of memory defining expression");

	e->what =  4 ;

	e->type.name = name;




	return e;
}


/* *********************************************************************** */


/* end of gasm.ypp */
