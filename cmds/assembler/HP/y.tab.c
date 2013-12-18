#ifndef lint
char yysccsid[] = "@(#)yaccpar	1.4 (Berkeley) 02/25/90";
#endif
#line 36 "gasm.y"

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






#line 4568 "gasm.y"
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
#line 4674 "gasm.y"
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
#line 4654 "y.tab.c"
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
#define B 303
#define BL 304
#define AND 305
#define EOR 306
#define SUB 307
#define RSB 308
#define ADD 309
#define ADC 310
#define SBC 311
#define RSC 312
#define ORR 313
#define BIC 314
#define TST 315
#define TEQ 316
#define CMP 317
#define CMN 318
#define MOV 319
#define MVN 320
#define MUL 321
#define MLA 322
#define LDR 323
#define STR 324
#define LDM 325
#define STM 326
#define SWI 327
#define SWP 328
#define CDO 329
#define LDC 330
#define STC 331
#define MRC 332
#define MCR 333
#define LEA 334
#define NOP 335
#define MRS 336
#define MSR 337
#define PATCHARMDT 338
#define PATCHARMDP 339
#define PATCHARMJP 340
#define PATCHARMDPLSB 341
#define PATCHARMDPMID 342
#define PATCHARMDPREST 343
#define R0 344
#define R1 345
#define R2 346
#define R3 347
#define R4 348
#define R5 349
#define R6 350
#define R7 351
#define R8 352
#define R9 353
#define R10 354
#define R11 355
#define R12 356
#define R13 357
#define R14 358
#define R15 359
#define CR0 360
#define CR1 361
#define CR2 362
#define CR3 363
#define CR4 364
#define CR5 365
#define CR6 366
#define CR7 367
#define CR8 368
#define CR9 369
#define CR10 370
#define CR11 371
#define CR12 372
#define CR13 373
#define CR14 374
#define CR15 375
#define LSL 376
#define LSR 377
#define ASR 378
#define ROR 379
#define RRX 380
#define CPSR 381
#define SPSR 382
#define CPSR_FLG 383
#define SPSR_FLG 384
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,   44,    0,   46,    0,    0,   43,   45,   45,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   47,   47,   47,   47,   47,   47,   47,   47,   47,
   47,   49,   50,   51,   56,   55,   52,   53,   54,   57,
   58,   59,   60,   61,   62,   63,   64,   65,   66,   67,
   68,   69,   70,   10,   10,   10,   10,   10,   10,   10,
   10,   10,   10,   10,   10,   11,   11,   71,    5,    5,
   72,    7,    7,    6,    6,    6,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    3,    3,    3,    3,    3,    3,    3,    3,    2,
    2,    8,    8,    8,    8,    8,    8,    8,    8,    8,
    8,    8,    8,    8,    8,    8,    8,    9,    9,   12,
   12,   12,   12,   12,   12,   12,   12,   12,   12,   12,
   12,   12,   12,   12,   12,   12,   13,   13,   13,   13,
   13,   13,   13,   13,   13,   13,   13,   13,   13,   13,
   13,   13,   13,   74,   48,   73,   73,   73,   73,   73,
   73,   73,   73,   73,   73,   75,   30,   30,   76,   76,
   76,   85,   86,   87,   87,   14,   14,   15,   15,   15,
   15,   17,   17,   17,   17,   33,   33,   33,   33,   31,
   31,   32,   32,   32,   32,   32,   32,   32,   32,   32,
   32,   77,   77,   88,   89,   18,   18,   28,   28,   29,
   29,   29,   29,   78,   78,   79,   19,   19,   19,   21,
   21,   21,   21,   21,   21,   20,   20,   20,   20,   24,
   24,   24,   16,   16,   16,   23,   23,   34,   34,   80,
   25,   25,   26,   26,   26,   26,   22,   22,   35,   35,
   81,   82,   82,   83,   83,   83,   90,   91,   36,   36,
   92,   37,   37,   38,   38,   40,   40,   40,   42,   42,
   42,   42,   41,   41,   84,   84,   39,   39,    4,    4,
   27,   27,   27,   27,   27,   27,
};
short yylen[] = {                                         2,
    0,    0,    3,    0,    3,    2,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    2,    2,    2,    2,    2,    4,    4,    4,    2,
    2,    4,    4,    2,    2,    2,    2,    1,    2,    8,
    2,    2,    2,    1,    1,    4,    4,    4,    4,    4,
    4,    4,    6,    6,    6,    1,    3,    0,    2,    3,
    0,    2,    3,    1,    1,    1,    3,    3,    3,    3,
    3,    3,    3,    3,    3,    3,    3,    2,    2,    1,
    1,    3,    3,    3,    3,    3,    2,    1,    1,    0,
    1,    3,    3,    3,    3,    3,    3,    3,    3,    3,
    3,    3,    2,    2,    1,    1,    1,    0,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    0,    2,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    2,    1,    1,    1,    1,
    1,    4,    4,    6,    4,    1,    1,    1,    3,    3,
    2,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    4,    4,    1,    1,    1,    1,    1,
    1,    1,    1,    6,    8,    4,    1,    1,    1,    4,
    4,    6,    6,    7,    7,    5,    5,    6,    6,    1,
    2,    2,    0,    2,    1,    0,    1,    1,    1,    6,
    1,    3,    1,    3,    3,    5,    0,    1,    1,    1,
    2,    8,    8,    1,    1,    1,   11,    6,    1,    1,
   11,    1,    1,    0,    2,    1,    1,    1,    4,    4,
    6,    6,    6,    6,    4,    1,    2,    1,    1,    1,
    1,    1,    1,    1,    1,    1,
};
short yydefred[] = {                                      1,
    0,    6,    0,    0,    7,    3,    0,    0,    0,   68,
   68,   68,   71,   71,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   48,    0,    0,    5,    8,
    9,   10,   11,   12,   13,   14,   15,   16,   17,   18,
   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
   29,   30,   31,    0,   51,   52,   53,    0,    0,    0,
    0,    0,    0,    0,   90,   91,    0,    0,    0,    0,
    0,    0,    0,    0,   41,    0,    0,   66,    0,    0,
    0,  117,  115,  116,    0,    0,    0,    0,   47,    0,
    0,  167,  168,  192,  193,  194,  195,  196,  197,  198,
  199,  200,  201,  186,  187,  188,  189,  191,  190,    0,
    0,  238,  239,  249,  250,    0,    0,    0,  259,  260,
  263,  262,    0,  276,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  155,  156,  157,  158,  159,  160,
  161,  162,  163,  164,  165,  169,  170,  171,  202,  203,
  254,  255,  256,    0,   54,   55,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   76,   69,    0,   75,
    0,   98,   99,    0,    0,    0,   89,    0,   88,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   68,
   68,   68,    0,    0,    0,  114,    0,  113,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  281,  282,
  283,  284,  285,  286,    0,    0,    0,  280,  279,  136,
  120,  121,  122,  123,  124,  125,  126,  127,  128,  129,
  130,  131,  132,  133,  134,  135,    0,    0,  251,    0,
    0,    0,    0,  210,  212,  211,  213,    0,  166,    0,
    0,    0,    0,    0,    0,    0,   70,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   97,    0,
    0,    0,    0,    0,   77,    0,    0,    0,    0,    0,
    0,    0,   80,   81,   82,    0,    0,    0,    0,    0,
   67,  102,    0,    0,    0,    0,    0,    0,    0,  105,
  106,  107,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  237,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   92,
    0,    0,   95,   96,    0,    0,    0,    0,    0,    0,
  208,  209,  204,  206,  205,  207,    0,  172,  176,  177,
    0,  175,  173,    0,    0,  216,  219,  218,  217,    0,
  153,  137,  138,  139,  140,  141,  142,  143,  144,  145,
  146,  147,  148,  149,  150,  151,  152,    0,    0,   56,
   57,   58,   59,   60,   61,   62,    0,    0,    0,  154,
    0,    0,    0,    0,  182,  183,  184,  185,  181,    0,
    0,    0,    0,    0,  241,    0,    0,    0,    0,    0,
    0,    0,  214,    0,    0,    0,    0,  179,  180,  174,
    0,    0,    0,    0,    0,    0,  248,  240,    0,    0,
  266,  258,  268,  267,    0,   63,   64,   65,   50,    0,
    0,    0,    0,    0,    0,  230,    0,    0,    0,  221,
    0,    0,    0,  220,    0,    0,  242,    0,    0,    0,
  215,  253,  252,    0,  231,  232,  235,    0,    0,    0,
    0,  227,    0,    0,    0,  226,    0,  245,    0,    0,
    0,    0,    0,    0,    0,  234,  223,  229,    0,  222,
  228,    0,    0,    0,  270,    0,    0,  269,    0,    0,
  225,  224,  246,    0,    0,    0,    0,    0,    0,  257,
  272,  274,  271,  273,  261,  265,
};
short yydgoto[] = {                                       1,
   70,   75,  176,  216,   58,  168,   62,  217,   89,  170,
   79,  446,  378,  348,  349,  468,  469,  345,  356,  357,
  358,  428,  317,  447,  406,  426,  218,  343,  248,  127,
  128,  129,  130,  131,  132,  133,  134,  510,  350,  432,
  433,  434,    6,    3,   29,    4,   30,   31,   32,   33,
   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,
   44,   45,   46,   47,   48,   49,   50,   51,   52,   53,
   59,   63,  135,   54,  136,  137,  138,  139,  140,  141,
  142,  143,  144,  145,  146,  147,  148,  149,  150,  151,
  152,  153,
};
short yysindex[] = {                                      0,
 -202,    0, -216, 1270,    0,    0, -207, -203, -192,    0,
    0,    0,    0,    0,  152,  152,  152,  152,  152, -137,
 -126, -124, -124, -124,  173,    0, -124,   95,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  727,    0,    0,    0,   93,  -24,   93,
   93,   98,  -30,   98,    0,    0,  152,  152,  152,   29,
   42,   69, 1485, 1485,    0,  108,  113,    0,  114,  114,
  114,    0,    0,    0,  173,  173,  173, 3276,    0,  114,
   30,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  433,
  433,    0,    0,    0,    0,   63,  433,   63,    0,    0,
    0,    0,  433,    0,  433, -211,   63,  433,  433,  433,
  433,  433,   63,   63,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  -24,    0,    0,  121,  125,  127,  144,
  162,  163,  164,  176,  181,  182,    0,    0, 3276,    0,
  -30,    0,    0,  -30,  -30,  232,    0,  102,    0,  152,
  152,  152,  152,  152,  152,  152,  152,  152,  152,    0,
    0,    0,  173,  173, -119,    0, 1231,    0,  173,  173,
  173,  173,  173,  173,  173,  173,  173,  173,    0,    0,
    0,    0,    0,    0,  173,  150, 3276,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  180,  188,    0,  192,
  193,  194,  195,    0,    0,    0,    0,  199,    0,  200,
  202,  203,  214,  226,  216,  236,    0,  -32,   19,   35,
   36,   39,   40,  508,  173,  173,  173,  232,    0,  371,
  -30,  -30,  -30,  -30,    0,   85,   85,   54,  172,  -10,
   46,   46,    0,    0,    0,   93,   93,   93, 3276, 3276,
    0,    0,  380,  380,  117,  347,  513,  104,  104,    0,
    0,    0, 3276,  508,  433,  433,  433,   63,  173, -339,
 4768, 4768, 4768, 4768,   24,    0,  256,  397,   63,  260,
  266,  267,  270,  272,  276,  277, 2424, 2765, 3106,    0,
   21,   21,    0,    0,  281,  282,  283,  284,  288, 3276,
    0,    0,    0,    0,    0,    0,  126,    0,    0,    0,
  -36,    0,    0, 4799,  433,    0,    0,    0,    0,  106,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  293,  301,    0,
    0,    0,    0,    0,    0,    0,  508,  508,  508,    0,
  433,  433,  -38,  397,    0,    0,    0,    0,    0, 4768,
 4768,  147,  -35,  433,    0,  252, 1486,  433,  307,  310,
  312,  313,    0,  316,  433,  433,  322,    0,    0,    0,
 4717,  -13, 4717,    3,   65,  231,    0,    0, 4799,  433,
    0,    0,    0,    0,  323,    0,    0,    0,    0,  433,
  333,  287,  397,  433, 4799,    0,  186,  334, 4717,    0,
  186,  289, 4717,    0,  433,  433,    0,  154,   11,  397,
    0,    0,    0,  332,    0,    0,    0,  336,   63,  226,
  186,    0,  290,  226,  186,    0,  343,    0,   63,    5,
   63,    7,  344,  397,  226,    0,    0,    0,  226,    0,
    0,  433,  354,   63,    0,  303,   63,    0,  397,  353,
    0,    0,    0,  226,  226,  226,  226,  353,   63,    0,
    0,    0,    0,    0,    0,    0,
};
short yyrindex[] = {                                      0,
 4896,    0,    0,  976,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, 3113,    0,
    0,    0,    0,    0, 3197,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0, 3279,    0, 3361,
 3451, 3536,    0, 3618,    0,    0,    0,    0,    0,    0,
    0,    0, 3700, 3789,    0,    0,    0,    0, 3875, 3957,
 4039,    0,    0,    0,    0,    0,    0, 4127,    0, 4214,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, 2342,    0,
    0,    0,    0,    0,    0, 2435,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0, 1758,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  355,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0, 2517,    0,    0,
    0,    0,    0,    0,    0, 1066, 1153, 2260, 2002, 1566,
  561,  652,    0,    0,    0, 4296, 4378, 4465, 4553, 4635,
    0,    0,  813,  905, 1658, 1405, 1318,  212,  314,    0,
    0,    0, 1910,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  400, 2097,    0,    0,    0,    0,    0,    0,    0, 2600,
    0,    0,    0,    0,    0,    0, 2682,    0,    0,    0,
 2682,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0, 2775,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, 2858,    0, 2858,  278,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  364,    0,    0,    0,
  326,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, 2858,
 2940,    0,    0, 2858, 2940,    0,  299,    0,    0, 2858,
    0, 2858,    0,    0, 2858,    0,    0,    0, 2858,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, 3022,
    0,    0,    0, 2858, 2858, 2858, 2858, 3022,    0,    0,
    0,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
 1929,    0,  227,    0,   -5,  261,  412,  -25,    0, -263,
   94, -106, -365, -301,    0, -394, -295,    0,    0,    0,
    0,    0,  520, -405,    0, -442,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  -80,  -88,    0,
    0,    0,    0,    0,    0,    0,    0,   45,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,
};
#define YYTABLESIZE 5233
short yytable[] = {                                      88,
  326,  415,  219,  237,  238,   60,   61,  401,  423,  175,
  240,  352,  353,  478,  174,   86,  242,  451,  243,  316,
   85,  250,  251,  252,  253,  254,  189,  239,  417,  241,
  449,  187,  185,  169,  186,  316,  188,  316,  249,  316,
  335,  341,  342,  471,  255,  256,  453,  475,  494,  503,
  497,  400,  416,    2,  481,  400,  473,  424,  215,  196,
  197,  198,  273,  354,  215,  189,  184,  274,   85,   86,
  187,  185,  190,  186,   85,  188,  488,  464,  189,  184,
  491,    5,  189,  187,  185,  191,  186,  187,  188,   55,
  189,  184,  188,   56,  483,  187,  185,  215,  186,  420,
  188,   87,   86,  482,   57,  189,  184,   85,  456,  455,
  187,  185,  192,  186,  355,  188,   80,   81,  500,   76,
   90,  189,  183,  409,  410,  411,  187,  185,  169,  186,
   77,  188,   78,  508,   91,  183,  154,  291,  189,  184,
  208,  171,  275,  187,  185,  206,  186,  183,  188,   87,
  207,  193,  182,  208,  203,   87,  194,  195,  206,  204,
  258,  205,  183,  207,  259,  182,  260,  289,  290,  244,
  245,  246,  247,  293,  294,  295,  296,  297,  298,  299,
  300,  301,  302,  261,  286,  287,  288,  422,   87,  303,
  421,   68,  182,  304,  480,  183,   67,  479,  336,  337,
  338,  262,  263,  264,  344,  347,  351,  347,  189,  184,
  202,  103,   86,  187,  185,  265,  186,   85,  188,  339,
  266,  267,  346,  305,  320,  182,  359,  172,  404,  173,
  379,  306,   82,   83,   84,  307,  308,  309,  310,  327,
  328,  329,  311,  312,  103,  313,  314,  402,  403,  103,
  180,  181,  103,  405,  103,  103,  103,  315,  316,  318,
  155,  156,  157,  158,  159,  160,  161,  162,  163,  164,
  165,  166,  167,  273,  271,  321,  272,   69,  274,  319,
   82,   83,   84,  340,  413,  414,   82,   83,   84,  180,
  181,  322,  323,  418,  347,  324,  325,  425,   87,  360,
  380,  435,  180,  181,  103,  103,  381,  382,  441,  442,
  383,  419,  384,  104,  180,  181,  385,  386,  431,   82,
   83,   84,  458,  459,  390,  391,  392,  393,  197,  180,
  181,  394,  448,  461,  452,  103,  407,  465,  466,  395,
  396,  397,  398,  399,  408,  427,  104,  436,  477,  425,
  437,  104,  438,  439,  104,  457,  104,  104,  104,  440,
  472,  220,  180,  181,  476,  443,  460,  209,  210,  211,
  212,  213,  214,  462,  470,  484,  485,  199,  200,  463,
  486,  474,  489,  208,  203,  425,  492,  499,  206,  204,
  493,  205,  496,  207,  504,  506,  509,  268,  236,   93,
  269,  270,  243,  197,  233,  505,  104,  104,  507,   65,
   66,  330,  273,  271,  257,  272,  208,  274,  233,  196,
  516,  206,  204,  244,  205,   64,  207,  515,    0,   82,
   83,   84,  180,  181,  412,    0,    0,  104,    0,    0,
   93,    0,   93,   93,   93,    0,    0,    0,    0,  221,
  222,  223,  224,  225,  226,  227,  228,  229,  230,  231,
  232,  233,  234,  235,  236,    0,    0,  103,    0,    0,
    0,    0,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,    0,  331,  332,  333,
  334,  395,  396,  397,  398,  399,    0,    0,    0,  103,
    0,    0,    0,    0,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  103,
  103,  103,  103,  103,  103,  103,  103,  103,  103,  208,
    0,    0,    0,    0,  206,  204,    0,  205,    0,  207,
   78,  395,  396,  397,  398,  467,    0,    0,    0,  104,
    0,    0,    0,    0,  104,  104,  104,  104,  104,  104,
  104,  104,  104,  104,  104,  104,  104,  104,  104,  104,
  104,  104,  104,  104,  104,  104,  104,  104,   78,    0,
    0,   78,    0,   78,   78,   78,    0,  199,  200,    0,
    0,  104,    0,    0,    0,    0,  104,  104,  104,  104,
  104,  104,  104,  104,  104,  104,  104,  104,  104,  104,
  104,  104,  104,  104,  104,  104,  104,  104,  104,  104,
  104,  104,  104,  104,  104,  104,  104,  104,  104,  104,
  104,   79,  361,    0,   78,   93,    0,    0,    0,    0,
    0,    0,   93,   93,   93,   93,   93,   93,   93,   93,
   93,   93,   93,   93,   93,   93,   93,   93,   93,   93,
   93,   93,   93,   93,   78,    0,    0,    0,  220,   79,
    0,    0,   79,    0,   79,   79,   79,   93,    0,    0,
    0,    0,   93,   93,   93,   93,   93,   93,   93,   93,
   93,   93,   93,   93,   93,   93,   93,   93,   93,   93,
   93,   93,   93,   93,   93,   93,   93,   93,   93,   93,
   93,   93,   93,   93,   93,   93,   93,    0,    0,    0,
    0,    0,    0,    0,    0,   79,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,
  376,  377,    0,  199,  200,   79,  221,  222,  223,  224,
  225,  226,  227,  228,  229,  230,  231,  232,  233,  234,
  235,  236,  155,  156,  157,  158,  159,  160,  161,  162,
  163,  164,  165,  166,    0,    0,    0,    0,    0,    0,
    0,    0,  111,    0,    0,    0,   78,    0,    0,    0,
    0,   78,   78,   78,   78,   78,   78,   78,   78,   78,
   78,   78,   78,   78,   78,   78,   78,   78,   78,   78,
   78,   78,   78,   78,   78,  111,    0,    0,    0,    0,
  111,    0,    0,  111,    0,    0,  111,    0,   78,    0,
    0,    0,    0,   78,   78,   78,   78,   78,   78,   78,
   78,   78,   78,   78,   78,   78,   78,   78,   78,   78,
   78,   78,   78,   78,   78,   78,   78,   78,   78,   78,
   78,   78,   78,   78,   78,   78,   78,   78,    0,    0,
    0,    0,    0,    0,  112,  111,  111,   79,    0,    0,
    0,    0,   79,   79,   79,   79,   79,   79,   79,   79,
   79,   79,   79,   79,   79,   79,   79,   79,   79,   79,
   79,   79,   79,   79,   79,   79,  111,  112,    0,    0,
    0,  450,  112,  454,    0,  112,    0,    0,  112,   79,
    0,    0,    0,    0,   79,   79,   79,   79,   79,   79,
   79,   79,   79,   79,   79,   79,   79,   79,   79,   79,
   79,   79,   79,   79,   79,   79,   79,   79,   79,   79,
   79,   79,   79,   79,   79,   79,   79,   79,   79,  487,
    0,    0,    0,  490,    0,    0,    0,  112,  112,  495,
    0,  498,    0,    0,  501,    0,    0,    0,  502,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  511,  512,  513,  514,    0,  112,   92,
   93,   94,   95,   96,   97,   98,   99,  100,  101,  102,
  103,  104,  105,  106,  107,  108,  109,  110,  111,  112,
  113,  114,  115,  116,  117,  118,  119,  120,  121,  122,
  123,  124,  125,  126,    0,   86,    0,    0,  111,    0,
    0,    0,    0,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,    0,    0,    0,
    0,    0,    0,   86,    0,    0,   86,    0,    0,   86,
  111,    0,    0,    0,    0,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
  111,  111,  111,  111,  111,  111,  111,  111,  111,  111,
    0,    0,   87,    0,    0,    0,    0,    0,    0,   86,
  112,    0,    0,    0,    0,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,   86,
   87,    0,    0,   87,    0,    0,   87,    0,    0,    0,
    0,    0,  112,    0,    0,    0,    0,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,  112,  112,  112,  112,  112,
  112,  112,    0,    0,    0,    0,   87,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  208,  203,    0,
    0,  292,  206,  204,    0,  205,   87,  207,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,    0,    0,    0,    0,  108,    0,    0,
    0,   86,    0,    0,  202,    0,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
  108,    0,    0,    0,  201,  108,    0,    0,  108,    0,
    0,  108,    0,   86,    0,    0,    0,    0,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,    0,  110,    0,    0,    0,   87,    0,
  108,  108,    0,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,  110,    0,    0,
    0,  108,    0,    0,    0,  110,    0,    0,  110,    0,
   87,    0,    0,    0,    0,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
    0,  199,  200,    0,    0,    0,    0,  110,  110,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  215,  189,  184,    0,    0,  429,  187,  185,  110,  186,
   85,  188,    7,    8,    9,   10,   11,   12,   13,   14,
   15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
   25,   26,   27,   28,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   83,    0,    0,    0,    0,
    0,    0,    0,  108,    0,    0,  430,    0,  183,    0,
  108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
  108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
  108,  108,    0,   83,    0,    0,   83,    0,  182,   83,
    0,   87,    0,    0,    0,  108,    0,    0,    0,    0,
  108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
  108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
  108,  108,  108,  108,  108,  108,  108,  108,  108,  108,
  108,  108,  108,  108,  108,    0,    0,  109,    0,   83,
  110,    0,    0,    0,    0,    0,    0,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,   83,
  109,    0,    0,    0,    0,    0,    0,    0,  109,    0,
    0,  109,  110,    0,    0,    0,    0,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,  110,  110,  110,  110,  110,  110,  110,  110,
  110,  110,   82,   83,   84,  180,  181,    0,    0,    0,
  109,    0,    0,    0,    0,    0,    0,  278,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  109,    0,    0,    0,    0,    0,    0,    0,    0,
  278,    0,    0,    0,    0,    0,    0,    0,  278,    0,
    0,  278,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   83,    0,    0,    0,    0,    0,    0,   83,   83,
   83,   83,   83,   83,   83,   83,   83,   83,   83,   83,
   83,   83,   83,   83,   83,   83,   83,   83,   83,   83,
  278,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   83,    0,    0,    0,    0,   83,   83,
   83,   83,   83,   83,   83,   83,   83,   83,   83,   83,
   83,   83,   83,   83,   83,   83,   83,   83,   83,   83,
   83,   83,   83,   83,   83,   83,   83,   83,   83,   83,
   83,   83,   83,    0,    0,    0,    0,    0,    0,  277,
    0,    0,    0,  109,    0,    0,    0,    0,    0,    0,
  109,  109,  109,  109,  109,  109,  109,  109,  109,  109,
  109,  109,  109,  109,  109,  109,  109,  109,  109,  109,
  109,  109,  277,    0,   71,   72,   73,   74,    0,    0,
  277,    0,    0,  277,    0,  109,    0,    0,    0,    0,
  109,  109,  109,  109,  109,  109,  109,  109,  109,  109,
  109,  109,  109,  109,  109,  109,  109,  109,  109,  109,
  109,  109,  109,  109,  109,  109,  109,  109,  109,  109,
  109,  109,  109,  109,  109,  177,  178,  179,    0,    0,
    0,   85,  277,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  278,    0,    0,    0,    0,    0,    0,
  278,  278,  278,  278,  278,  278,  278,  278,  278,  278,
  278,  278,  278,  278,  278,  278,  278,  278,  278,  278,
  278,  278,   85,    0,    0,   85,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  278,    0,    0,    0,    0,
  278,  278,  278,  278,  278,  278,  278,  278,  278,  278,
  278,  278,  278,  278,  278,  278,  278,  278,  278,  278,
  278,  278,  278,  278,  278,  278,  278,  278,  278,  278,
  278,  278,  278,  278,  278,   85,   94,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  276,  277,
  278,  279,  280,  281,  282,  283,  284,  285,    0,    0,
    0,    0,    0,    0,    0,   85,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   94,    0,   94,
   94,   94,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  277,    0,    0,    0,    0,
    0,    0,  277,  277,  277,  277,  277,  277,  277,  277,
  277,  277,  277,  277,  277,  277,  277,  277,  277,  277,
  277,  277,  277,  277,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  277,    0,    0,
    0,    0,  277,  277,  277,  277,  277,  277,  277,  277,
  277,  277,  277,  277,  277,  277,  277,  277,  277,  277,
  277,  277,  277,  277,  277,  277,  277,  277,  277,  277,
  277,  277,  277,  277,  277,  277,  277,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   85,    0,   84,
    0,    0,    0,    0,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   85,
   84,    0,    0,   84,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,   85,
   85,   85,   85,   85,   85,   85,   85,   85,   85,    0,
    0,   74,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   94,    0,    0,    0,    0,    0,    0,   94,
   94,   94,   94,   94,   94,   94,   94,   94,   94,   94,
   94,   94,   94,   94,   94,   94,   94,   94,   94,   94,
   94,    0,    0,   84,    0,   74,    0,    0,    0,    0,
    0,    0,    0,    0,   94,    0,    0,    0,    0,   94,
   94,   94,   94,   94,   94,   94,   94,   94,   94,   94,
   94,   94,   94,   94,   94,   94,   94,   94,   94,   94,
   94,   94,   94,   94,   94,   94,   94,   94,   94,   94,
   94,   94,   94,   94,   72,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  208,  203,    0,    0,    0,  206,  204,  387,  205,    0,
  207,    0,    0,    0,    0,    0,    0,    0,   72,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   84,   73,  202,    0,    0,
    0,    0,   84,   84,   84,   84,   84,   84,   84,   84,
   84,   84,   84,   84,   84,   84,   84,   84,   84,   84,
   84,   84,   84,   84,    0,    0,    0,  201,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   84,    0,    0,
   73,    0,   84,   84,   84,   84,   84,   84,   84,   84,
   84,   84,   84,   84,   84,   84,   84,   84,   84,   84,
   84,   84,   84,   84,   84,   84,   84,   84,   84,   84,
   84,   84,   84,   84,   84,   84,   84,   74,    0,  275,
    0,    0,    0,    0,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   74,
  275,    0,    0,    0,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,   74,
   74,   74,   74,   74,   74,   74,   74,   74,   74,    0,
    0,  178,    0,    0,  199,  200,    0,    0,    0,    0,
   72,    0,    0,    0,    0,    0,    0,   72,   72,   72,
   72,   72,   72,   72,   72,   72,   72,   72,   72,   72,
   72,   72,   72,   72,   72,   72,   72,   72,   72,    0,
    0,    0,  178,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   72,    0,    0,    0,    0,   72,   72,   72,
   72,   72,   72,   72,   72,   72,   72,   72,   72,   72,
   72,   72,   72,   72,   72,   72,   72,   72,   72,   72,
   72,   72,   72,   72,   72,   72,   72,   72,   72,   72,
   72,   72,   73,    0,  247,    0,    0,    0,    0,   73,
   73,   73,   73,   73,   73,   73,   73,   73,   73,   73,
   73,   73,   73,   73,   73,   73,   73,   73,   73,   73,
   73,  208,  203,    0,    0,    0,  206,  204,  388,  205,
    0,  207,    0,    0,   73,  247,    0,    0,    0,   73,
   73,   73,   73,   73,   73,   73,   73,   73,   73,   73,
   73,   73,   73,   73,   73,   73,   73,   73,   73,   73,
   73,   73,   73,   73,   73,   73,   73,   73,   73,   73,
   73,   73,   73,   73,    0,  275,    0,  236,  202,    0,
    0,    0,  275,  275,  275,  275,  275,  275,  275,  275,
  275,  275,  275,  275,  275,  275,  275,  275,  275,  275,
  275,  275,  275,  275,    0,    0,    0,    0,  201,    0,
    0,    0,    0,    0,    0,    0,    0,  275,  236,    0,
    0,    0,  275,  275,  275,  275,  275,  275,  275,  275,
  275,  275,  275,  275,  275,  275,  275,  275,  275,  275,
  275,  275,  275,  275,  275,  275,  275,  275,  275,  275,
  275,  275,  275,  275,  275,  275,  275,  178,    0,  233,
    0,    0,    0,    0,  178,  178,  178,  178,  178,  178,
  178,  178,  178,  178,  178,  178,  178,  178,  178,  178,
  178,  178,  178,  178,  178,  178,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  178,
  233,    0,    0,    0,  178,  178,  178,  178,  178,  178,
  178,  178,  178,  178,  178,  178,  178,  178,  178,  178,
  178,  178,  178,  178,  178,  178,  178,  178,  178,  178,
  178,  178,  178,  178,  178,  178,  178,  178,  178,    0,
    0,  264,    0,    0,    0,  199,  200,    0,    0,    0,
  247,    0,    0,    0,    0,    0,    0,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,    0,
    0,    0,  264,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  247,    0,    0,    0,    0,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  247,  247,  247,  247,  247,  247,  247,  247,
  247,  247,  100,  236,    0,    0,    0,    0,    0,    0,
  236,  236,  236,  236,  236,  236,  236,  236,  236,  236,
  236,  236,  236,  236,  236,  236,  236,  236,  236,  236,
  236,  236,  208,  203,    0,    0,    0,  206,  204,  389,
  205,    0,  207,    0,    0,  236,    0,    0,    0,    0,
  236,  236,  236,  236,  236,  236,  236,  236,  236,  236,
  236,  236,  236,  236,  236,  236,  236,  236,  236,  236,
  236,  236,  236,  236,  236,  236,  236,  236,  236,  236,
  236,  236,  236,  236,  236,  233,  118,    0,    0,  202,
    0,    0,  233,  233,  233,  233,  233,  233,  233,  233,
  233,  233,  233,  233,  233,  233,  233,  233,  233,  233,
  233,  233,  233,  233,    0,    0,    0,    0,    0,  201,
    0,    0,    0,    0,    0,    0,    0,  233,    0,    0,
    0,    0,  233,  233,  233,  233,  233,  233,  233,  233,
  233,  233,  233,  233,  233,  233,  233,  233,  233,  233,
  233,  233,  233,  233,  233,  233,  233,  233,  233,  233,
  233,  233,  233,  233,  233,  233,  233,  264,   32,    0,
    0,    0,    0,    0,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,    0,    0,    0,    0,
    0,    0,  208,  203,    0,    0,    0,  206,  204,  264,
  205,    0,  207,    0,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,  264,
  264,  264,  264,  264,  264,  264,  264,  264,  264,    0,
   33,    0,    0,    0,    0,    0,  199,  200,  100,  202,
    0,    0,    0,    0,    0,  100,  100,  100,  100,  100,
  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,
  100,  100,  100,  100,  100,  100,  100,    0,    0,  201,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  100,    0,    0,    0,    0,  100,  100,  100,  100,  100,
  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,
  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,
  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,
   34,    0,  118,    0,    0,    0,    0,    0,    0,  118,
  118,  118,  118,  118,  118,  118,  118,  118,  118,  118,
  118,  118,  118,  118,  118,  118,  118,  118,  118,  118,
  118,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  118,    0,    0,    0,    0,  118,
  118,  118,  118,  118,  118,  118,  118,  118,  118,  118,
  118,  118,  118,  118,  118,  118,  118,  118,  118,  118,
  118,  118,  118,  118,  118,  118,  118,  118,  118,  118,
  118,  118,  118,  118,   32,   35,  199,  200,    0,    0,
    0,   32,   32,   32,   32,   32,   32,   32,   32,   32,
   32,   32,   32,   32,   32,   32,   32,   32,   32,   32,
   32,   32,   32,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   32,    0,    0,    0,
    0,   32,   32,   32,   32,   32,   32,   32,   32,   32,
   32,   32,   32,   32,   32,   32,   32,   32,   32,   32,
   32,   32,   32,   32,   32,   32,   32,   32,   32,   32,
   32,   32,   32,   32,   32,   32,   33,   36,    0,    0,
    0,    0,    0,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   33,    0,
    0,    0,    0,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   33,   33,   33,    0,   40,
    0,    0,    0,    0,    0,    0,   34,    0,    0,    0,
    0,    0,    0,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   34,    0,
    0,    0,    0,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,  101,    0,
    0,   35,    0,    0,    0,    0,    0,    0,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   35,    0,    0,    0,    0,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   35,   35,   35,   35,   35,   35,   35,
   35,   35,   35,   36,   44,    0,    0,    0,    0,    0,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   36,    0,    0,    0,    0,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   40,   45,    0,    0,    0,
    0,    0,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   40,    0,    0,
    0,    0,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,   40,    0,   46,    0,
    0,    0,    0,    0,  101,    0,    0,    0,    0,    0,
    0,  101,  101,  101,  101,  101,  101,  101,  101,  101,
  101,  101,  101,  101,  101,  101,  101,  101,  101,  101,
  101,  101,  101,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  101,    0,    0,    0,
    0,  101,  101,  101,  101,  101,  101,  101,  101,  101,
  101,  101,  101,  101,  101,  101,  101,  101,  101,  101,
  101,  101,  101,  101,  101,  101,  101,  101,  101,  101,
  101,  101,  101,  101,  101,  101,  119,    0,    0,    0,
   44,    0,    0,    0,    0,    0,    0,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   44,    0,    0,    0,    0,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   45,   49,    0,    0,    0,    0,    0,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   45,    0,    0,    0,    0,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   46,   37,    0,    0,    0,    0,
    0,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   46,    0,    0,    0,
    0,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,   46,   46,   46,   46,
   46,   46,   46,   46,   46,   46,    0,   38,    0,    0,
    0,    0,  119,    0,    0,    0,    0,    0,    0,  119,
  119,  119,  119,  119,  119,  119,  119,  119,  119,  119,
  119,  119,  119,  119,  119,  119,  119,  119,  119,  119,
  119,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  119,    0,    0,    0,    0,  119,
  119,  119,  119,  119,  119,  119,  119,  119,  119,  119,
  119,  119,  119,  119,  119,  119,  119,  119,  119,  119,
  119,  119,  119,  119,  119,  119,  119,  119,  119,  119,
  119,  119,  119,  119,   39,    0,    0,    0,    0,   49,
    0,    0,    0,    0,    0,    0,   49,   49,   49,   49,
   49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
   49,   49,   49,   49,   49,   49,   49,   49,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   49,    0,    0,    0,    0,   49,   49,   49,   49,
   49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
   49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
   49,   49,   49,   49,   49,   49,   49,   49,   49,   49,
   49,   37,   42,    0,    0,    0,    0,    0,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   37,    0,    0,    0,    0,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,   37,   37,
   37,   37,   37,   38,   43,    0,    0,    0,    0,    0,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   38,    0,    0,    0,    0,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   38,
   38,   38,   38,   38,   38,    0,    0,    0,    0,    0,
   39,    0,    0,    0,    0,    0,    0,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,    0,
    0,  215,    0,    0,    0,    0,   86,    0,    0,  444,
    0,  445,   39,    0,    0,    0,    0,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,  215,    0,    0,    0,    0,   86,   42,    0,
    0,    0,   85,    0,    0,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,    0,   86,    0,
    0,    0,   87,   85,    0,    0,    0,    0,    0,    0,
   42,    0,    0,    0,    0,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   42,   42,   42,   42,   42,   42,   42,   42,   42,   42,
   43,    0,    0,   87,    0,    0,    0,   43,   43,   43,
   43,   43,   43,   43,   43,   43,   43,   43,   43,   43,
   43,   43,   43,   43,   43,   43,   43,   43,   43,    0,
    0,    0,    0,    0,   87,    0,    0,    0,    0,    0,
    0,    0,   43,    0,    0,    0,    0,   43,   43,   43,
   43,   43,   43,   43,   43,   43,   43,   43,   43,   43,
   43,   43,   43,   43,   43,   43,   43,   43,   43,   43,
   43,   43,   43,   43,   43,   43,   43,   43,   43,   43,
   43,   43,  220,   82,   83,   84,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  220,   82,   83,   84,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  220,   82,   83,   84,    0,    0,
  221,  222,  223,  224,  225,  226,  227,  228,  229,  230,
  231,  232,  233,  234,  235,  236,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  221,  222,  223,  224,  225,  226,  227,  228,  229,
  230,  231,  232,  233,  234,  235,  236,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  221,  222,  223,  224,  225,  226,  227,  228,
  229,  230,  231,  232,  233,  234,  235,  236,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    2,    0,    0,    0,    0,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,
};
short yycheck[] = {                                      25,
  264,   40,   91,  110,  111,   11,   12,   44,   44,   40,
  117,  313,  314,  456,   45,   40,  123,  423,  125,   33,
   45,  128,  129,  130,  131,  132,   37,  116,  394,  118,
   44,   42,   43,   59,   45,   33,   47,   33,  127,   33,
  304,  381,  382,  449,  133,  134,   44,  453,   44,  492,
   44,  347,   91,  256,   44,  351,  451,   93,   35,   85,
   86,   87,   42,   40,   35,   37,   38,   47,   45,   40,
   42,   43,   44,   45,   45,   47,  471,  443,   37,   38,
  475,  298,   37,   42,   43,   44,   45,   42,   47,  297,
   37,   38,   47,  297,  460,   42,   43,   35,   45,  401,
   47,  126,   40,   93,  297,   37,   38,   45,   44,   45,
   42,   43,   44,   45,   91,   47,   23,   24,  484,  257,
   27,   37,   94,  387,  388,  389,   42,   43,  154,   45,
  257,   47,  257,  499,   40,   94,   44,  257,   37,   38,
   37,   44,   41,   42,   43,   42,   45,   94,   47,  126,
   47,   44,  124,   37,   38,  126,   44,   44,   42,   43,
   40,   45,   94,   47,   40,  124,   40,  193,  194,  381,
  382,  383,  384,  199,  200,  201,  202,  203,  204,  205,
  206,  207,  208,   40,  190,  191,  192,   41,  126,  215,
   44,   40,  124,   44,   41,   94,   45,   44,  305,  306,
  307,   40,   40,   40,  311,  312,  313,  314,   37,   38,
   94,    0,   40,   42,   43,   40,   45,   45,   47,  308,
   40,   40,  311,   44,  257,  124,  315,  258,  123,  260,
  319,   44,  257,  258,  259,   44,   44,   44,   44,  265,
  266,  267,   44,   44,   33,   44,   44,  354,  355,   38,
  261,  262,   41,  360,   43,   44,   45,   44,   33,   44,
  285,  286,  287,  288,  289,  290,  291,  292,  293,  294,
  295,  296,  297,   42,   43,  257,   45,  126,   47,   44,
  257,  258,  259,  309,  391,  392,  257,  258,  259,  261,
  262,  257,  257,  400,  401,  257,  257,  404,  126,   44,
   41,  408,  261,  262,   93,   94,   41,   41,  415,  416,
   41,  400,   41,    0,  261,  262,   41,   41,  407,  257,
  258,  259,  429,  430,   44,   44,   44,   44,  354,  261,
  262,   44,  421,  440,  423,  124,   44,  444,  445,  376,
  377,  378,  379,  380,   44,   94,   33,   41,  455,  456,
   41,   38,   41,   41,   41,  125,   43,   44,   45,   44,
  449,  256,  261,  262,  453,   44,   44,  338,  339,  340,
  341,  342,  343,   41,   41,   44,   41,  261,  262,   93,
  469,   93,   93,   37,   38,  492,   44,   44,   42,   43,
  479,   45,  481,   47,   41,   93,   44,  171,   44,    0,
  174,  175,  125,  429,   41,  494,   93,   94,  497,  258,
  259,   41,   42,   43,  154,   45,   37,   47,   93,  445,
  509,   42,   43,  125,   45,   14,   47,  508,   -1,  257,
  258,  259,  261,  262,  390,   -1,   -1,  124,   -1,   -1,
   41,   -1,   43,   44,   45,   -1,   -1,   -1,   -1,  344,
  345,  346,  347,  348,  349,  350,  351,  352,  353,  354,
  355,  356,  357,  358,  359,   -1,   -1,  256,   -1,   -1,
   -1,   -1,  261,  262,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,   -1,  271,  272,  273,
  274,  376,  377,  378,  379,  380,   -1,   -1,   -1,  298,
   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,  331,  332,  333,  334,  335,  336,  337,   37,
   -1,   -1,   -1,   -1,   42,   43,   -1,   45,   -1,   47,
    0,  376,  377,  378,  379,  380,   -1,   -1,   -1,  256,
   -1,   -1,   -1,   -1,  261,  262,  263,  264,  265,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  275,  276,
  277,  278,  279,  280,  281,  282,  283,  284,   38,   -1,
   -1,   41,   -1,   43,   44,   45,   -1,  261,  262,   -1,
   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,
  307,  308,  309,  310,  311,  312,  313,  314,  315,  316,
  317,  318,  319,  320,  321,  322,  323,  324,  325,  326,
  327,  328,  329,  330,  331,  332,  333,  334,  335,  336,
  337,    0,  256,   -1,   94,  256,   -1,   -1,   -1,   -1,
   -1,   -1,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,  124,   -1,   -1,   -1,  256,   38,
   -1,   -1,   41,   -1,   43,   44,   45,  298,   -1,   -1,
   -1,   -1,  303,  304,  305,  306,  307,  308,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,
  331,  332,  333,  334,  335,  336,  337,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   94,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  360,  361,  362,  363,
  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,
  374,  375,   -1,  261,  262,  124,  344,  345,  346,  347,
  348,  349,  350,  351,  352,  353,  354,  355,  356,  357,
  358,  359,  285,  286,  287,  288,  289,  290,  291,  292,
  293,  294,  295,  296,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,    0,   -1,   -1,   -1,  256,   -1,   -1,   -1,
   -1,  261,  262,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   33,   -1,   -1,   -1,   -1,
   38,   -1,   -1,   41,   -1,   -1,   44,   -1,  298,   -1,
   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,   -1,   -1,
   -1,   -1,   -1,   -1,    0,   93,   94,  256,   -1,   -1,
   -1,   -1,  261,  262,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,  124,   33,   -1,   -1,
   -1,  422,   38,  424,   -1,   41,   -1,   -1,   44,  298,
   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,  331,  332,  333,  334,  335,  336,  337,  470,
   -1,   -1,   -1,  474,   -1,   -1,   -1,   93,   94,  480,
   -1,  482,   -1,   -1,  485,   -1,   -1,   -1,  489,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  504,  505,  506,  507,   -1,  124,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,   -1,    0,   -1,   -1,  256,   -1,
   -1,   -1,   -1,  261,  262,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,
   -1,   -1,   -1,   38,   -1,   -1,   41,   -1,   -1,   44,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
   -1,   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,   94,
  256,   -1,   -1,   -1,   -1,  261,  262,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,  124,
   38,   -1,   -1,   41,   -1,   -1,   44,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,   -1,   -1,   -1,   -1,   94,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   37,   38,   -1,
   -1,   41,   42,   43,   -1,   45,  124,   47,  303,  304,
  305,  306,  307,  308,  309,  310,  311,  312,  313,  314,
  315,  316,  317,  318,  319,  320,  321,  322,  323,  324,
  325,  326,  327,  328,  329,  330,  331,  332,  333,  334,
  335,  336,  337,   -1,   -1,   -1,   -1,    0,   -1,   -1,
   -1,  256,   -1,   -1,   94,   -1,  261,  262,  263,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,  283,  284,
   33,   -1,   -1,   -1,  124,   38,   -1,   -1,   41,   -1,
   -1,   44,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,
  305,  306,  307,  308,  309,  310,  311,  312,  313,  314,
  315,  316,  317,  318,  319,  320,  321,  322,  323,  324,
  325,  326,  327,  328,  329,  330,  331,  332,  333,  334,
  335,  336,  337,   -1,    0,   -1,   -1,   -1,  256,   -1,
   93,   94,   -1,  261,  262,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   33,   -1,   -1,
   -1,  124,   -1,   -1,   -1,   41,   -1,   -1,   44,   -1,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
   -1,  261,  262,   -1,   -1,   -1,   -1,   93,   94,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   35,   37,   38,   -1,   -1,   40,   42,   43,  124,   45,
   45,   47,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,    0,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  256,   -1,   -1,   91,   -1,   94,   -1,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   -1,   38,   -1,   -1,   41,   -1,  124,   44,
   -1,  126,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,
  303,  304,  305,  306,  307,  308,  309,  310,  311,  312,
  313,  314,  315,  316,  317,  318,  319,  320,  321,  322,
  323,  324,  325,  326,  327,  328,  329,  330,  331,  332,
  333,  334,  335,  336,  337,   -1,   -1,    0,   -1,   94,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,  124,
   33,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,   -1,
   -1,   44,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  257,  258,  259,  261,  262,   -1,   -1,   -1,
   93,   -1,   -1,   -1,   -1,   -1,   -1,    0,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  124,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   33,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,   -1,
   -1,   44,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,  283,  284,
   93,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,
  305,  306,  307,  308,  309,  310,  311,  312,  313,  314,
  315,  316,  317,  318,  319,  320,  321,  322,  323,  324,
  325,  326,  327,  328,  329,  330,  331,  332,  333,  334,
  335,  336,  337,   -1,   -1,   -1,   -1,   -1,   -1,    0,
   -1,   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   33,   -1,   16,   17,   18,   19,   -1,   -1,
   41,   -1,   -1,   44,   -1,  298,   -1,   -1,   -1,   -1,
  303,  304,  305,  306,  307,  308,  309,  310,  311,  312,
  313,  314,  315,  316,  317,  318,  319,  320,  321,  322,
  323,  324,  325,  326,  327,  328,  329,  330,  331,  332,
  333,  334,  335,  336,  337,   67,   68,   69,   -1,   -1,
   -1,    0,   93,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   41,   -1,   -1,   44,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,
  303,  304,  305,  306,  307,  308,  309,  310,  311,  312,
  313,  314,  315,  316,  317,  318,  319,  320,  321,  322,
  323,  324,  325,  326,  327,  328,  329,  330,  331,  332,
  333,  334,  335,  336,  337,   94,    0,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  180,  181,
  182,  183,  184,  185,  186,  187,  188,  189,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  124,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   41,   -1,   43,
   44,   45,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,   -1,
   -1,   -1,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,
   -1,   -1,  303,  304,  305,  306,  307,  308,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,
  331,  332,  333,  334,  335,  336,  337,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,    0,
   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,
   41,   -1,   -1,   44,  303,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,  331,  332,  333,  334,  335,  336,  337,   -1,
   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,  124,   -1,   44,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,    0,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   37,   38,   -1,   -1,   -1,   42,   43,   44,   45,   -1,
   47,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   44,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  256,    0,   94,   -1,   -1,
   -1,   -1,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,   -1,   -1,   -1,  124,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,
   44,   -1,  303,  304,  305,  306,  307,  308,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,
  331,  332,  333,  334,  335,  336,  337,  256,   -1,    0,
   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,
   41,   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,  331,  332,  333,  334,  335,  336,  337,   -1,
   -1,    0,   -1,   -1,  261,  262,   -1,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   41,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  256,   -1,    0,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   37,   38,   -1,   -1,   -1,   42,   43,   44,   45,
   -1,   47,   -1,   -1,  298,   41,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,   -1,  256,   -1,    0,   94,   -1,
   -1,   -1,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,   -1,   -1,   -1,   -1,  124,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   41,   -1,
   -1,   -1,  303,  304,  305,  306,  307,  308,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,
  331,  332,  333,  334,  335,  336,  337,  256,   -1,    0,
   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,
   41,   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,  331,  332,  333,  334,  335,  336,  337,   -1,
   -1,    0,   -1,   -1,   -1,  261,  262,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   41,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,    0,  256,   -1,   -1,   -1,   -1,   -1,   -1,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   37,   38,   -1,   -1,   -1,   42,   43,   44,
   45,   -1,   47,   -1,   -1,  298,   -1,   -1,   -1,   -1,
  303,  304,  305,  306,  307,  308,  309,  310,  311,  312,
  313,  314,  315,  316,  317,  318,  319,  320,  321,  322,
  323,  324,  325,  326,  327,  328,  329,  330,  331,  332,
  333,  334,  335,  336,  337,  256,    0,   -1,   -1,   94,
   -1,   -1,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,  124,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,
   -1,   -1,  303,  304,  305,  306,  307,  308,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,
  331,  332,  333,  334,  335,  336,  337,  256,    0,   -1,
   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,
   -1,   -1,   37,   38,   -1,   -1,   -1,   42,   43,  298,
   45,   -1,   47,   -1,  303,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,  331,  332,  333,  334,  335,  336,  337,   -1,
    0,   -1,   -1,   -1,   -1,   -1,  261,  262,  256,   94,
   -1,   -1,   -1,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   -1,  124,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
    0,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  256,    0,  261,  262,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,  256,    0,   -1,   -1,
   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,
   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,   -1,    0,
   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,
   -1,   -1,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,
   -1,   -1,   -1,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  334,  335,  336,  337,    0,   -1,
   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,  283,  284,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,
  305,  306,  307,  308,  309,  310,  311,  312,  313,  314,
  315,  316,  317,  318,  319,  320,  321,  322,  323,  324,
  325,  326,  327,  328,  329,  330,  331,  332,  333,  334,
  335,  336,  337,  256,    0,   -1,   -1,   -1,   -1,   -1,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,
  303,  304,  305,  306,  307,  308,  309,  310,  311,  312,
  313,  314,  315,  316,  317,  318,  319,  320,  321,  322,
  323,  324,  325,  326,  327,  328,  329,  330,  331,  332,
  333,  334,  335,  336,  337,  256,    0,   -1,   -1,   -1,
   -1,   -1,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,
   -1,   -1,  303,  304,  305,  306,  307,  308,  309,  310,
  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,
  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,
  331,  332,  333,  334,  335,  336,  337,   -1,    0,   -1,
   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,    0,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  256,    0,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,  256,    0,   -1,   -1,   -1,   -1,
   -1,  263,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,
   -1,  303,  304,  305,  306,  307,  308,  309,  310,  311,
  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,
  332,  333,  334,  335,  336,  337,   -1,    0,   -1,   -1,
   -1,   -1,  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,
  334,  335,  336,  337,    0,   -1,   -1,   -1,   -1,  256,
   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  275,  276,
  277,  278,  279,  280,  281,  282,  283,  284,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,
  307,  308,  309,  310,  311,  312,  313,  314,  315,  316,
  317,  318,  319,  320,  321,  322,  323,  324,  325,  326,
  327,  328,  329,  330,  331,  332,  333,  334,  335,  336,
  337,  256,    0,   -1,   -1,   -1,   -1,   -1,  263,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,  283,  284,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,
  305,  306,  307,  308,  309,  310,  311,  312,  313,  314,
  315,  316,  317,  318,  319,  320,  321,  322,  323,  324,
  325,  326,  327,  328,  329,  330,  331,  332,  333,  334,
  335,  336,  337,  256,    0,   -1,   -1,   -1,   -1,   -1,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,
  303,  304,  305,  306,  307,  308,  309,  310,  311,  312,
  313,  314,  315,  316,  317,  318,  319,  320,  321,  322,
  323,  324,  325,  326,  327,  328,  329,  330,  331,  332,
  333,  334,  335,  336,  337,   -1,   -1,   -1,   -1,   -1,
  256,   -1,   -1,   -1,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   35,   -1,   -1,   -1,   -1,   40,   -1,   -1,   43,
   -1,   45,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,   35,   -1,   -1,   -1,   -1,   40,  256,   -1,
   -1,   -1,   45,   -1,   -1,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,   -1,   40,   -1,
   -1,   -1,  126,   45,   -1,   -1,   -1,   -1,   -1,   -1,
  298,   -1,   -1,   -1,   -1,  303,  304,  305,  306,  307,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,
  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,
  328,  329,  330,  331,  332,  333,  334,  335,  336,  337,
  256,   -1,   -1,  126,   -1,   -1,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  276,  277,  278,  279,  280,  281,  282,  283,  284,   -1,
   -1,   -1,   -1,   -1,  126,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,  305,
  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  256,  257,  258,  259,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  256,  257,  258,  259,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  256,  257,  258,  259,   -1,   -1,
  344,  345,  346,  347,  348,  349,  350,  351,  352,  353,
  354,  355,  356,  357,  358,  359,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  344,  345,  346,  347,  348,  349,  350,  351,  352,
  353,  354,  355,  356,  357,  358,  359,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  344,  345,  346,  347,  348,  349,  350,  351,
  352,  353,  354,  355,  356,  357,  358,  359,  263,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,  283,  284,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  298,   -1,   -1,   -1,   -1,  303,  304,
  305,  306,  307,  308,  309,  310,  311,  312,  313,  314,
  315,  316,  317,  318,  319,  320,  321,  322,  323,  324,
  325,  326,  327,  328,  329,  330,  331,  332,  333,  334,
  335,  336,  337,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 384
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'!'",0,"'#'",0,"'%'","'&'",0,"'('","')'","'*'","'+'","','","'-'",0,"'/'",0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'['",0,"']'","'^'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'{'","'|'","'}'","'~'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"NAME","NUMBER","CHARCONST",
"FLOATNUM","LSHIFT","RSHIFT","USERERROR","USERWARNING","USERNOTE","BYTE",
"SHORT","WORD","FLOATY","DOUBLE","BLOCKBYTE","BLOCKSHORT","BLOCKWORD","SPACE",
"ALIGN","DATA","COMMON","CODETABLE","EXPORT","IMPORT","MODULE","INIT","REF",
"PATCHINSTR","MODSIZE","MODNUM","DATASYMB","CODESYMB","DATAMODULE","LABELREF",
"CODESTUB","ADDRSTUB","BYTESWAP","SHIFT","P_ADD","P_OR","STRINGCONST","LABEL",
"PLUSPLUS","MINUSMINUS","INSTRUCTION","UNARY","B","BL","AND","EOR","SUB","RSB",
"ADD","ADC","SBC","RSC","ORR","BIC","TST","TEQ","CMP","CMN","MOV","MVN","MUL",
"MLA","LDR","STR","LDM","STM","SWI","SWP","CDO","LDC","STC","MRC","MCR","LEA",
"NOP","MRS","MSR","PATCHARMDT","PATCHARMDP","PATCHARMJP","PATCHARMDPLSB",
"PATCHARMDPMID","PATCHARMDPREST","R0","R1","R2","R3","R4","R5","R6","R7","R8",
"R9","R10","R11","R12","R13","R14","R15","CR0","CR1","CR2","CR3","CR4","CR5",
"CR6","CR7","CR8","CR9","CR10","CR11","CR12","CR13","CR14","CR15","LSL","LSR",
"ASR","ROR","RRX","CPSR","SPSR","CPSR_FLG","SPSR_FLG",
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
"reg : R12",
"reg : R13",
"reg : R14",
"reg : R15",
"reg : error",
"cp_reg : CR0",
"cp_reg : CR1",
"cp_reg : CR2",
"cp_reg : CR3",
"cp_reg : CR4",
"cp_reg : CR5",
"cp_reg : CR6",
"cp_reg : CR7",
"cp_reg : CR8",
"cp_reg : CR9",
"cp_reg : CR10",
"cp_reg : CR11",
"cp_reg : CR12",
"cp_reg : CR13",
"cp_reg : CR14",
"cp_reg : CR15",
"cp_reg : error",
"$$5 :",
"machine_op : $$5 ARM_op",
"ARM_op : branch",
"ARM_op : dataproc",
"ARM_op : arm6psr",
"ARM_op : multiply",
"ARM_op : datatran",
"ARM_op : ldstmult",
"ARM_op : softintr",
"ARM_op : swap",
"ARM_op : coproc",
"ARM_op : pseudo",
"branch : branch_ops armconstexpr",
"branch_ops : B",
"branch_ops : BL",
"dataproc : move",
"dataproc : comp",
"dataproc : logic",
"move : move_ops reg ',' op2",
"comp : comp_ops reg ',' op2",
"logic : logic_ops reg ',' reg ',' op2",
"logic : logic_ops reg ',' op2",
"op2 : shifttype",
"op2 : armconstexpr",
"shifttype : reg",
"shifttype : reg shiftname reg",
"shifttype : reg shiftname armconstexpr",
"shifttype : reg RRX",
"shiftname : LSL",
"shiftname : LSR",
"shiftname : ASR",
"shiftname : ROR",
"comp_ops : TST",
"comp_ops : TEQ",
"comp_ops : CMP",
"comp_ops : CMN",
"move_ops : MVN",
"move_ops : MOV",
"logic_ops : AND",
"logic_ops : EOR",
"logic_ops : SUB",
"logic_ops : RSB",
"logic_ops : ADD",
"logic_ops : ADC",
"logic_ops : SBC",
"logic_ops : RSC",
"logic_ops : ORR",
"logic_ops : BIC",
"arm6psr : mrs",
"arm6psr : msr",
"mrs : MRS reg ',' cpsr",
"msr : MSR cpsr_flg ',' reg_expr",
"reg_expr : reg",
"reg_expr : armconstexpr",
"cpsr : CPSR",
"cpsr : SPSR",
"cpsr_flg : CPSR",
"cpsr_flg : CPSR_FLG",
"cpsr_flg : SPSR",
"cpsr_flg : SPSR_FLG",
"multiply : MUL reg ',' reg ',' reg",
"multiply : MLA reg ',' reg ',' reg ',' reg",
"datatran : data_ops reg ',' addr",
"addr : armconstexpr",
"addr : preindex",
"addr : postindex",
"preindex : '[' reg ']' optpling",
"preindex : '(' reg ')' optpling",
"preindex : '[' reg ',' armconstexpr ']' optpling",
"preindex : '(' reg ',' armconstexpr ')' optpling",
"preindex : '[' reg ',' indexreg optshift ']' optpling",
"preindex : '(' reg ',' indexreg optshift ')' optpling",
"postindex : '[' reg ']' ',' armconstexpr",
"postindex : '(' reg ')' ',' armconstexpr",
"postindex : '[' reg ']' ',' indexreg optshift",
"postindex : '(' reg ')' ',' indexreg optshift",
"indexreg : reg",
"indexreg : '+' reg",
"indexreg : '-' reg",
"optshift :",
"optshift : shiftname armconstexpr",
"optshift : RRX",
"optpling :",
"optpling : '!'",
"data_ops : LDR",
"data_ops : STR",
"ldstmult : ldstmult_ops reg optpling ',' reglist optclaret",
"reglist : reg",
"reglist : '{' regs '}'",
"regs : reg",
"regs : reg '-' reg",
"regs : reg ',' regs",
"regs : reg '-' reg ',' regs",
"optclaret :",
"optclaret : '^'",
"ldstmult_ops : LDM",
"ldstmult_ops : STM",
"softintr : SWI armconstexpr",
"swap : SWP reg ',' reg ',' '[' reg ']'",
"swap : SWP reg ',' reg ',' '(' reg ')'",
"coproc : cdo",
"coproc : cdt",
"coproc : crt",
"cdo : CDO armconstexpr ',' armconstexpr ',' cp_reg ',' cp_reg ',' cp_reg optaux",
"cdt : cdt_ops armconstexpr ',' cp_reg ',' cp_addr",
"cdt_ops : LDC",
"cdt_ops : STC",
"crt : crt_ops armconstexpr ',' armconstexpr ',' reg ',' cp_reg ',' cp_reg optaux",
"crt_ops : MCR",
"crt_ops : MRC",
"optaux :",
"optaux : ',' armconstexpr",
"cp_addr : armconstexpr",
"cp_addr : cp_preindex",
"cp_addr : cp_postindex",
"cp_preindex : '[' reg ']' optpling",
"cp_preindex : '(' reg ')' optpling",
"cp_preindex : '[' reg ',' armconstexpr ']' optpling",
"cp_preindex : '(' reg ',' armconstexpr ')' optpling",
"cp_postindex : '[' reg ']' ',' armconstexpr optpling",
"cp_postindex : '(' reg ')' ',' armconstexpr optpling",
"pseudo : LEA reg ',' constexpr",
"pseudo : NOP",
"armconstexpr : '#' constexpr",
"armconstexpr : constexpr",
"instrpatch : armconstexpr",
"instrpatch : armpatches",
"armpatches : PATCHARMDT",
"armpatches : PATCHARMDP",
"armpatches : PATCHARMJP",
"armpatches : PATCHARMDPLSB",
"armpatches : PATCHARMDPMID",
"armpatches : PATCHARMDPREST",
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
#line 6376 "gasm.y"
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
#line 6659 "y.tab.c"
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
#line 4870 "gasm.y"
{StartLine = CurLine;}
break;
case 4:
#line 4871 "gasm.y"
{StartLine = CurLine;}
break;
case 6:
#line 4873 "gasm.y"
{
			yyclearin;	/* clear read ahead tokens */
			yyerrok;	/* allow syntax errors to restart */
			ClearInput();	/* clear all input up to next \n */
		}
break;
case 7:
#line 4883 "gasm.y"
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




				Symbol *s = NewSymb(yyvsp[0].str ,  0 , curPC);


				/* add symbol table ref to parse tree */
				/* this will output an object code label */
				ParseTreeItem *pti = NewParseTreeItem(LABEL);

				pti->type.symb = s;










			}
		}
break;
case 32:
#line 4962 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(BYTE);

			/* insert ref to codeconstlist items */
			pti->type.clist = yyvsp[0].clist ;
			curPC += list_size_count * sizeof(char);
		}
break;
case 33:
#line 4972 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(SHORT);

			pti->type.clist = yyvsp[0].clist ;
			curPC += list_size_count * (2 * sizeof(char));
		}
break;
case 34:
#line 4981 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(WORD);

			pti->type.clist = yyvsp[0].clist ;
			curPC += list_size_count * sizeof(int);
		}
break;
case 35:
#line 4990 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(FLOATY);

			pti->type.flist = yyvsp[0].flist ;
			curPC += list_size_count * 4; /* 32 bit float */
		}
break;
case 36:
#line 4999 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(DOUBLE);

			pti->type.flist = yyvsp[0].flist ;
			curPC += list_size_count * 8; /* 64 bit float */
		}
break;
case 37:
#line 5022 "gasm.y"
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
case 38:
#line 5044 "gasm.y"
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
case 39:
#line 5066 "gasm.y"
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
case 40:
#line 5091 "gasm.y"
{
			/* add space directive to parse tree */
			ParseTreeItem *pti = NewParseTreeItem(SPACE);

			pti->type.num = yyvsp[0].num ;

			curPC += yyvsp[0].num ;
		}
break;
case 41:
#line 5105 "gasm.y"
{
			ParseTreeItem *pti;
			int pad;

			/* if alignment not defined default to 4 */
			pad =  ((((curPC) + ((yyvsp[0].num ) ? yyvsp[0].num  : 4) - 1) & ~(((yyvsp[0].num ) ? yyvsp[0].num  : 4) - 1)) - curPC) ;

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
case 42:
#line 5132 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(DATA);

			pti->type.datacommon.name = yyvsp[-2].str ;
			pti->type.datacommon.expr = yyvsp[0].expr ;




		}
break;
case 43:
#line 5146 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(COMMON);

			pti->type.datacommon.name = yyvsp[-2].str ;
			pti->type.datacommon.expr = yyvsp[0].expr ;




		}
break;
case 44:
#line 5160 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(CODETABLE);

			pti->type.nlist = yyvsp[0].nlist ;




		}
break;
case 45:
#line 5174 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(EXPORT);

			pti->type.nlist = yyvsp[0].nlist ;




		}
break;
case 46:
#line 5188 "gasm.y"
{
			/* force a linker global directive to be emited */
			ParseTreeItem *pti = NewParseTreeItem(IMPORT);
			NameList *n = yyvsp[0].nlist ;

			pti->type.nlist = yyvsp[0].nlist ;

			/* add imported label to the symbol table */
			while ( n !=  0 ) {
				NewSymb(n->name,  4 , 0);
				n = n->next;
			}



		}
break;
case 47:
#line 5212 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(MODULE);

			/* if no module number expression, default to -1 */
			pti->type.expr = (yyvsp[0].expr ) ? yyvsp[0].expr  : NewExprNum(-1);




		}
break;
case 48:
#line 5226 "gasm.y"
{
			NewParseTreeItem(INIT);

			/* init parse item needs to remember no data */




			curPC += sizeof(int);
		}
break;
case 49:
#line 5240 "gasm.y"
{
			ParseTreeItem *pti = NewParseTreeItem(REF);

			pti->type.nlist = yyvsp[0].nlist ;




		}
break;
case 50:
#line 5253 "gasm.y"
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
case 51:
#line 5286 "gasm.y"
{
			Error(yyvsp[0].str );
		}
break;
case 52:
#line 5292 "gasm.y"
{
			Warn(yyvsp[0].str );
		}
break;
case 53:
#line 5298 "gasm.y"
{
			Note(yyvsp[0].str );
		}
break;
case 54:
#line 5314 "gasm.y"
{
			yyval.patch  = NewPatch(MODSIZE);			/* 0x0e */



		}
break;
case 55:
#line 5321 "gasm.y"
{
			yyval.patch  = NewPatch(MODNUM);			/* 0x012 */



		}
break;
case 56:
#line 5328 "gasm.y"
{
			yyval.patch  = NewPatch(DATASYMB);		/* 0x010 */
			yyval.patch ->type.name = yyvsp[-1].str ;



		}
break;
case 57:
#line 5336 "gasm.y"
{
			yyval.patch  = NewPatch(CODESYMB);		/* 0x01d */
			yyval.patch ->type.name = yyvsp[-1].str ;



		}
break;
case 58:
#line 5344 "gasm.y"
{
			yyval.patch  = NewPatch(DATAMODULE);		/* 0x011 */
			yyval.patch ->type.name = yyvsp[-1].str ;



		}
break;
case 59:
#line 5352 "gasm.y"
{
			/* @@@ give warning if label is not in this module */
			/* @@@ during second pass */
			yyval.patch  = NewPatch(LABELREF);		/* 0x01f */
			yyval.patch ->type.name = yyvsp[-1].str ;




		}
break;
case 60:
#line 5364 "gasm.y"
{
			yyval.patch  = NewPatch(CODESTUB);		/* 0x028 */
			yyval.patch ->type.name = yyvsp[-1].str ;




		}
break;
case 61:
#line 5374 "gasm.y"
{
			yyval.patch  = NewPatch(ADDRSTUB);		/* 0x029 */
			yyval.patch ->type.name = yyvsp[-1].str ;




		}
break;
case 62:
#line 5385 "gasm.y"
{
			yyval.patch  = NewPatch(BYTESWAP);		/* 0x01e */
			yyval.patch ->type.patch = yyvsp[-1].patch ;




		}
break;
case 63:
#line 5394 "gasm.y"
{
			yyval.patch  = NewPatch(SHIFT);			/* 0x014 */
			yyval.patch ->type.shift.expr = yyvsp[-3].expr ;
			yyval.patch ->type.shift.patch = yyvsp[-1].patch ;




		}
break;
case 64:
#line 5404 "gasm.y"
{
			yyval.patch  = NewPatch(P_ADD);			/* 0x013 */
			yyval.patch ->type.shift.expr = yyvsp[-3].expr ;
			yyval.patch ->type.shift.patch = yyvsp[-1].patch ;




		}
break;
case 65:
#line 5414 "gasm.y"
{
			yyval.patch  = NewPatch(P_OR);			/* 0x01f */
			yyval.patch ->type.shift.expr = yyvsp[-3].expr ;
			yyval.patch ->type.shift.patch = yyvsp[-1].patch ;




		}
break;
case 66:
#line 5430 "gasm.y"
{
			/* first or only item in list */
			NameList *n = malloc(sizeof(NameList));

			if (n ==  0 )
				Fatal("Out of memory whilst building name list");

			n->next =  0 ;
			n->name = yyvsp[0].str ;
			yyval.nlist  = n;
		}
break;
case 67:
#line 5442 "gasm.y"
{
			/* list of names */
			NameList *n = yyvsp[-2].nlist ;
			NameList *nn = malloc(sizeof(NameList));

			if (nn ==  0 )
				Fatal("Out of memory whilst building name list");

			/* insert name list item - order is not important */
			/* so dont search to end of list */
			nn->next = n->next;
			n->next = nn;
			nn->name = yyvsp[0].str ;

			yyval.nlist  = n;	/* return start of namelist */
		}
break;
case 68:
#line 5468 "gasm.y"
{ list_size_count = 0; }
break;
case 69:
#line 5470 "gasm.y"
{
			yyval.clist  = yyvsp[0].clist ;	/* first or only item in list */
		}
break;
case 70:
#line 5474 "gasm.y"
{
			ConstList *n = yyvsp[-2].clist ;	/* start of list */

			/* search for end of list */
			while (n->next !=  0 )
				n = n->next;

			n->next = yyvsp[0].clist ;	/* append item to end of list */

			yyval.clist  = yyvsp[-2].clist ;	/* returns start of constlist */
		}
break;
case 71:
#line 5488 "gasm.y"
{ list_size_count = 0; }
break;
case 72:
#line 5490 "gasm.y"
{
			/* first or only item in list */
			yyval.flist  = NewFloatConstItem(yyvsp[0].flt );
			list_size_count++;
		}
break;
case 73:
#line 5496 "gasm.y"
{
			FloatList *fl = yyvsp[-2].flist ;	/* start of list */

			/* search for end of list */
			while (fl->next !=  0 )
				fl = fl->next;
			/* append item to end of list */
			fl->next = NewFloatConstItem(yyvsp[0].flt );
			list_size_count++;

			yyval.flist  = yyvsp[-2].flist ;	/* returns start of floatconstlist */
		}
break;
case 74:
#line 5514 "gasm.y"
{
			/* return an Expression constlist item */
			yyval.clist  = NewConstItem( 1 );

			yyval.clist ->type.expr = yyvsp[0].expr ;
			list_size_count++;
		}
break;
case 75:
#line 5522 "gasm.y"
{
			/* return a patch constlist item */
			yyval.clist  = NewConstItem( 3 );

			yyval.clist ->type.patch = yyvsp[0].patch ;
			list_size_count++;
		}
break;
case 76:
#line 5530 "gasm.y"
{
			/* return a string constlist item */
			yyval.clist  = NewConstItem( 4 );

			yyval.clist ->type.str = yyvsp[0].str ;
			/* increment the list size count by the number of */
			/* chars in the string - 1 counted by default already */





			list_size_count += strlen(yyvsp[0].str );

		}
break;
case 77:
#line 5564 "gasm.y"
{	yyval.num  = yyvsp[-1].num ; }
break;
case 78:
#line 5566 "gasm.y"
{	yyval.num  = yyvsp[-2].num  + yyvsp[0].num ; }
break;
case 79:
#line 5568 "gasm.y"
{	yyval.num  = yyvsp[-2].num  - yyvsp[0].num ; }
break;
case 80:
#line 5570 "gasm.y"
{	yyval.num  = yyvsp[-2].num  * yyvsp[0].num ; }
break;
case 81:
#line 5572 "gasm.y"
{	yyval.num  = yyvsp[-2].num  / yyvsp[0].num ; }
break;
case 82:
#line 5574 "gasm.y"
{	yyval.num  = yyvsp[-2].num  % yyvsp[0].num ; }
break;
case 83:
#line 5576 "gasm.y"
{	yyval.num  = yyvsp[-2].num  & yyvsp[0].num ; }
break;
case 84:
#line 5578 "gasm.y"
{	yyval.num  = yyvsp[-2].num  | yyvsp[0].num ; }
break;
case 85:
#line 5580 "gasm.y"
{	yyval.num  = yyvsp[-2].num  ^ yyvsp[0].num ; }
break;
case 86:
#line 5582 "gasm.y"
{	yyval.num  = yyvsp[-2].num  << yyvsp[0].num ; }
break;
case 87:
#line 5584 "gasm.y"
{	yyval.num  = yyvsp[-2].num  >> yyvsp[0].num ; }
break;
case 88:
#line 5586 "gasm.y"
{	yyval.num  = ~ (yyvsp[0].num ); }
break;
case 89:
#line 5588 "gasm.y"
{	yyval.num  = - (yyvsp[0].num ); }
break;
case 92:
#line 5604 "gasm.y"
{	yyval.flt  = yyvsp[-1].flt ; }
break;
case 93:
#line 5607 "gasm.y"
{	yyval.flt  = yyvsp[-2].flt  + yyvsp[0].flt ; }
break;
case 94:
#line 5609 "gasm.y"
{	yyval.flt  = yyvsp[-2].flt  - yyvsp[0].flt ; }
break;
case 95:
#line 5611 "gasm.y"
{	yyval.flt  = yyvsp[-2].flt  * yyvsp[0].flt ; }
break;
case 96:
#line 5613 "gasm.y"
{	yyval.flt  = yyvsp[-2].flt  / yyvsp[0].flt ; }
break;
case 97:
#line 5615 "gasm.y"
{	yyval.flt  = - (yyvsp[0].flt ); }
break;
case 98:
#line 5619 "gasm.y"
{ yyval.flt  = (double)yyvsp[0].num ;}
break;
case 100:
#line 5647 "gasm.y"
{



			yyval.num  = 0;
		}
break;
case 102:
#line 5665 "gasm.y"
{	yyval.expr  = yyvsp[-1].expr ; }
break;
case 103:
#line 5667 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '+', yyvsp[0].expr ); }
break;
case 104:
#line 5669 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '-', yyvsp[0].expr ); }
break;
case 105:
#line 5671 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '*', yyvsp[0].expr ); }
break;
case 106:
#line 5673 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '/', yyvsp[0].expr ); }
break;
case 107:
#line 5675 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '%', yyvsp[0].expr ); }
break;
case 108:
#line 5677 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '&', yyvsp[0].expr ); }
break;
case 109:
#line 5679 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '|', yyvsp[0].expr ); }
break;
case 110:
#line 5681 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , '^', yyvsp[0].expr ); }
break;
case 111:
#line 5683 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , LSHIFT, yyvsp[0].expr ); }
break;
case 112:
#line 5685 "gasm.y"
{	yyval.expr  = NewExpr(yyvsp[-2].expr , RSHIFT, yyvsp[0].expr ); }
break;
case 113:
#line 5687 "gasm.y"
{	yyval.expr  = NewExpr( 0 , '~', yyvsp[0].expr ); }
break;
case 114:
#line 5689 "gasm.y"
{	yyval.expr  = NewExpr( 0 , '-', yyvsp[0].expr ); }
break;
case 115:
#line 5692 "gasm.y"
{	yyval.expr  = NewExprNum(yyvsp[0].num );}
break;
case 116:
#line 5695 "gasm.y"
{	yyval.expr  = NewExprNum(yyvsp[0].num );}
break;
case 117:
#line 5698 "gasm.y"
{	yyval.expr  = NewExprSymbRef(yyvsp[0].str );}
break;
case 118:
#line 5704 "gasm.y"
{
			/* code executed if no optional constexpr in source */




			yyval.expr  =  0 ;
		}
break;
case 136:
#line 5762 "gasm.y"
{	Error("Expecting an ARM register");	}
break;
case 153:
#line 5768 "gasm.y"
{	Error("Expecting a coprocessor register");	}
break;
case 154:
#line 5781 "gasm.y"
{
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
break;
case 155:
#line 5794 "gasm.y"
{
			/* add new instruction into the parse tree */
			ParseTreeItem *pti = NewParseTreeItem(INSTRUCTION);
			pti->type.instr = CurInstr;

			curPC += sizeof(int);
		}
break;
case 166:
#line 5821 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->template;
			CurInstr->optexpr = yyvsp[0].expr ;

			/* specifies eval and check of 24 bit pcrel value */
			CurInstr->combine =  1 ;
		}
break;
case 172:
#line 5838 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->template |  ((yyvsp[-2].num ) << 12)  | yyvsp[0].num ;
		}
break;
case 173:
#line 5844 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->template |  ((yyvsp[-2].num ) << 16)  | yyvsp[0].num ;
		}
break;
case 174:
#line 5850 "gasm.y"
{
			CurInstr->opcode = yyvsp[-5].mnem ->template | 				 ((yyvsp[-4].num ) << 12)  |  ((yyvsp[-2].num ) << 16)  | yyvsp[0].num ;

		}
break;
case 175:
#line 5855 "gasm.y"
{
			CurInstr->opcode = yyvsp[-3].mnem ->template | 				 ((yyvsp[-2].num ) << 12)  |  ((yyvsp[-2].num ) << 16)  | yyvsp[0].num ;

		}
break;
case 176:
#line 5862 "gasm.y"
{	yyval.num  = yyvsp[0].num ; }
break;
case 177:
#line 5864 "gasm.y"
{
			/* after evaluation the value must be formed into an */
			/* eight bit value + a rotation 4*2 rotation */
			CurInstr->optexpr = yyvsp[0].expr ;
			CurInstr->combine =  2 ;
			/* set immediate mode in instruction */
			yyval.num  =  (1 << 25) ;
		}
break;
case 178:
#line 5875 "gasm.y"
{	yyval.num  = yyvsp[0].num ; }
break;
case 179:
#line 5877 "gasm.y"
{	yyval.num  =  (1 << 4)  | yyvsp[-2].num  | yyvsp[-1].num  |  ((yyvsp[0].num ) << 8) ; }
break;
case 180:
#line 5879 "gasm.y"
{
			yyval.num  = yyvsp[-2].num  | yyvsp[-1].num ;
			CurInstr->optexpr = yyvsp[0].expr ;
			CurInstr->combine =  3 ;
		}
break;
case 181:
#line 5885 "gasm.y"
{	yyval.num  = yyvsp[-1].num  | yyvsp[0].num ; }
break;
case 204:
#line 5902 "gasm.y"
{
/*fprintf(stderr, "mrs opcode = %x, dstreg %x, cpsr %x\n", $1, $2, $4);*/
			CurInstr->opcode = yyvsp[-3].mnem ->template |  ((yyvsp[-2].num ) << 12)  | yyvsp[0].num ;
		}
break;
case 205:
#line 5909 "gasm.y"
{
/*fprintf(stderr, "msr opcode = %x, cpsr %x, reg_expr %x\n", $1, $2, $4);*/
			if ((yyvsp[-2].num  &  (1 << 16) ) && (yyvsp[0].num  &  (1 << 25) )) {
				/* Cannot use immediate mode with full psr */
				Error("MSR instruction can only set psr flags with immediate operand");
			}
			CurInstr->opcode = yyvsp[-3].mnem ->template | yyvsp[-2].num  | yyvsp[0].num ;
		}
break;
case 206:
#line 5920 "gasm.y"
{	yyval.num  = yyvsp[0].num ; }
break;
case 207:
#line 5922 "gasm.y"
{
			/* after evaluation the value must be formed into an */
			/* eight bit value + a rotation 4*2 rotation */
			CurInstr->optexpr = yyvsp[0].expr ;
			CurInstr->combine =  2 ;
			/* set immediate mode in instruction */
			yyval.num  =  (1 << 25) ;
		}
break;
case 214:
#line 5940 "gasm.y"
{
			CurInstr->opcode = yyvsp[-5].mnem ->template | 				 ((yyvsp[-4].num ) << 16)  | 				 (yyvsp[-2].num )  | 				 ((yyvsp[0].num ) << 8) ;




			if (yyvsp[-4].num  == yyvsp[-2].num )
				Error("Destination and first source operand "
				"must not be the same");

			if (yyvsp[-4].num  ==  0xF )
				Warn("Destination of R15 will have no "
				"effect, other than altering the PSR flags");

		}
break;
case 215:
#line 5956 "gasm.y"
{
			CurInstr->opcode = yyvsp[-7].mnem ->template | 				 ((yyvsp[-6].num ) << 16)  | 				 (yyvsp[-4].num )  | 				 ((yyvsp[-2].num ) << 8)  | 				 ((yyvsp[0].num ) << 12) ;





			if (yyvsp[-6].num  == yyvsp[-4].num )
				Error("Destination and first source operand "
				"must not be the same");

			if (yyvsp[-6].num  ==  0xF )
				Warn("Destination of R15 will have no "
				"effect, other than altering the PSR flags");
		}
break;
case 216:
#line 5978 "gasm.y"
{	CurInstr->opcode = yyvsp[-3].mnem ->template |  ((yyvsp[-2].num ) << 12)  | yyvsp[0].num ; }
break;
case 217:
#line 5982 "gasm.y"
{
			CurInstr->optexpr = yyvsp[0].expr ;
			CurInstr->combine =  5 ;
			/* preindex address relative to the PC */
			yyval.num  =  ((0xF) << 16)  |  (1 << 24) ;
		}
break;
case 220:
#line 5993 "gasm.y"
{
			/* as the offset is zero, make it a positive offset */
			/* (INDEXUP) so we don't see '-0' in disassemblies */
			yyval.num  =  (1 << 24)  |  ((yyvsp[-2].num ) << 16)  |  (1 << 23)  | yyvsp[0].num ;
		}
break;
case 221:
#line 5999 "gasm.y"
{
			yyval.num  =  (1 << 24)  |  ((yyvsp[-2].num ) << 16)  |  (1 << 23)  | yyvsp[0].num ;
		}
break;
case 222:
#line 6003 "gasm.y"
{
			CurInstr->optexpr = yyvsp[-2].expr ;
			CurInstr->combine =  5 ;
			yyval.num  =  (1 << 24)  |  ((yyvsp[-4].num ) << 16)  | yyvsp[0].num ;
		}
break;
case 223:
#line 6009 "gasm.y"
{
			CurInstr->optexpr = yyvsp[-2].expr ;
			CurInstr->combine =  5 ;
			yyval.num  =  (1 << 24)  |  ((yyvsp[-4].num ) << 16)  | yyvsp[0].num ;
		}
break;
case 224:
#line 6015 "gasm.y"
{
			yyval.num  =  (1 << 24)  |  (1 << 25)  |  ((yyvsp[-5].num ) << 16)  |
			     yyvsp[-3].num  | yyvsp[-2].num  | yyvsp[0].num ;
		}
break;
case 225:
#line 6020 "gasm.y"
{
			yyval.num  =  (1 << 24)  |  (1 << 25)  |  ((yyvsp[-5].num ) << 16)  |
			     yyvsp[-3].num  | yyvsp[-2].num  | yyvsp[0].num ;
		}
break;
case 226:
#line 6027 "gasm.y"
{
			CurInstr->optexpr = yyvsp[0].expr ;
			CurInstr->combine =  5 ;
			yyval.num  =  ((yyvsp[-3].num ) << 16) ;
		}
break;
case 227:
#line 6033 "gasm.y"
{
			CurInstr->optexpr = yyvsp[0].expr ;
			CurInstr->combine =  5 ;
			yyval.num  =  ((yyvsp[-3].num ) << 16) ;
		}
break;
case 228:
#line 6039 "gasm.y"
{	yyval.num  =  (1 << 25)  |  ((yyvsp[-4].num ) << 16)  | yyvsp[-1].num  | yyvsp[0].num ; }
break;
case 229:
#line 6041 "gasm.y"
{	yyval.num  =  (1 << 25)  |  ((yyvsp[-4].num ) << 16)  | yyvsp[-1].num  | yyvsp[0].num ; }
break;
case 230:
#line 6045 "gasm.y"
{	yyval.num  =  (1 << 23)  | yyvsp[0].num ; }
break;
case 231:
#line 6047 "gasm.y"
{	yyval.num  =  (1 << 23)  | yyvsp[0].num ; }
break;
case 232:
#line 6049 "gasm.y"
{	yyval.num  = yyvsp[0].num ; }
break;
case 233:
#line 6054 "gasm.y"
{	yyval.num  = 0; }
break;
case 234:
#line 6056 "gasm.y"
{
			CurInstr->optexpr = yyvsp[0].expr ;
			CurInstr->combine =  3 ;
			yyval.num  = yyvsp[-1].num ;
		}
break;
case 236:
#line 6066 "gasm.y"
{	yyval.num  = 0; }
break;
case 237:
#line 6068 "gasm.y"
{	yyval.num  =  (1 << 21) ; }
break;
case 240:
#line 6078 "gasm.y"
{	CurInstr->opcode = yyvsp[-5].mnem ->template |  ((yyvsp[-4].num ) << 16)  				| yyvsp[-3].num  | yyvsp[-1].num  | yyvsp[0].num ;}
break;
case 241:
#line 6083 "gasm.y"
{	yyval.num  = 1 << yyvsp[0].num ; }
break;
case 242:
#line 6085 "gasm.y"
{	yyval.num  = yyvsp[-1].num ; }
break;
case 243:
#line 6089 "gasm.y"
{	yyval.num  = 1 << yyvsp[0].num ; }
break;
case 244:
#line 6091 "gasm.y"
{
			int i;

			yyval.num  = 0;

			if (yyvsp[-2].num  < yyvsp[0].num ) {
				for (i = yyvsp[-2].num ; i <= yyvsp[0].num ; i++)
					yyval.num  |= 1 << i;
			} else {
				for (i = yyvsp[0].num ; i <= yyvsp[-2].num ; i++)
					yyval.num  |= 1 << i;
			}
		}
break;
case 245:
#line 6105 "gasm.y"
{	yyval.num  = 1 << yyvsp[-2].num  | yyvsp[0].num ; }
break;
case 246:
#line 6107 "gasm.y"
{
			int i;

			yyval.num  = 0;

			if (yyvsp[-4].num  < yyvsp[-2].num ) {
				for (i = yyvsp[-4].num ; i <= yyvsp[-2].num ; i++)
					yyval.num  |= 1 << i;
			} else {
				for (i = yyvsp[-2].num ; i <= yyvsp[-4].num ; i++)
					yyval.num  |= 1 << i;
			}

			yyval.num  |= yyvsp[0].num ;
		}
break;
case 247:
#line 6125 "gasm.y"
{	yyval.num  = 0; }
break;
case 248:
#line 6127 "gasm.y"
{	yyval.num  =  (1 << 22) ; }
break;
case 249:
#line 6131 "gasm.y"
{
			yyval.mnem  = yyvsp[0].mnem ;
			if (yyvsp[0].mnem ->template ==  0 )
				Error("LDM mnemonic must include a mode");
		}
break;
case 250:
#line 6137 "gasm.y"
{
			yyval.mnem  = yyvsp[0].mnem ;

			if (yyvsp[0].mnem ->template ==  0 )
				Error("STM mnemonic must include a mode");
		}
break;
case 251:
#line 6150 "gasm.y"
{
			CurInstr->opcode = yyvsp[-1].mnem ->template;
			CurInstr->optexpr = yyvsp[0].expr ;
			CurInstr->combine =  9 ;
		}
break;
case 252:
#line 6162 "gasm.y"
{
			CurInstr->opcode = yyvsp[-7].mnem ->template |  ((yyvsp[-6].num ) << 12)  |
				 (yyvsp[-4].num )  |  ((yyvsp[-1].num ) << 16)  ;
		}
break;
case 253:
#line 6167 "gasm.y"
{
			CurInstr->opcode = yyvsp[-7].mnem ->template |  ((yyvsp[-6].num ) << 12)  |
				 (yyvsp[-4].num )  |  ((yyvsp[-1].num ) << 16)  ;
		}
break;
case 257:
#line 6182 "gasm.y"
{
			CurInstr->opcode = yyvsp[-10].mnem ->template |  ((yyvsp[-5].num ) << 12)  |
				 ((yyvsp[-3].num ) << 16)  |  (yyvsp[-1].num ) ;

			CurInstr->optexpr = yyvsp[-7].expr ; /* CP Opc */
			CurInstr->combine =  10 ;

			CurInstr->optcoprocnum = yyvsp[-9].expr ; /* CP# */
			CurInstr->optcoprocaux = yyvsp[0].expr ; /* AUX expr */
		}
break;
case 258:
#line 6195 "gasm.y"
{
			CurInstr->opcode = yyvsp[-5].mnem ->template |  ((yyvsp[-2].num ) << 12)  | yyvsp[0].num ;
			/* CurInstr->optexpr set by cp_addr rule */
			CurInstr->optcoprocnum = yyvsp[-4].expr ; /* CP# */
		}
break;
case 261:
#line 6207 "gasm.y"
{
			CurInstr->opcode = yyvsp[-10].mnem ->template |  ((yyvsp[-5].num ) << 12)  |
				 ((yyvsp[-3].num ) << 16)  |  (yyvsp[-1].num ) ;

			CurInstr->optexpr = yyvsp[-7].expr ; /* CP Opc */
			CurInstr->combine =  11 ;

			CurInstr->optcoprocnum = yyvsp[-9].expr ; /* CP# */
			CurInstr->optcoprocaux = yyvsp[0].expr ; /* AUX expr */
		}
break;
case 264:
#line 6223 "gasm.y"
{	yyval.expr  =  0 ; }
break;
case 265:
#line 6225 "gasm.y"
{	yyval.expr  = yyvsp[0].expr ; }
break;
case 266:
#line 6231 "gasm.y"
{
			CurInstr->optexpr = yyvsp[0].expr ;
			CurInstr->combine =  6 ;
			/* preindex address relative to the PC */
			yyval.num  =  ((0xF) << 16)  |  (1 << 24) ;
		}
break;
case 269:
#line 6242 "gasm.y"
{
			/* as the offset is zero, make it a positive offset */
			/* (INDEXUP) so we don't see '-0' in disassemblies */
			yyval.num  =  (1 << 24)  |  ((yyvsp[-2].num ) << 16)  |  (1 << 23)  | yyvsp[0].num ;
		}
break;
case 270:
#line 6248 "gasm.y"
{
			yyval.num  =  (1 << 24)  |  ((yyvsp[-2].num ) << 16)  |  (1 << 23)  | yyvsp[0].num ;
		}
break;
case 271:
#line 6252 "gasm.y"
{
			CurInstr->optexpr = yyvsp[-2].expr ;
			CurInstr->combine =  6 ;
			yyval.num  =  (1 << 24)  |  ((yyvsp[-4].num ) << 16)  | yyvsp[0].num ;
		}
break;
case 272:
#line 6258 "gasm.y"
{
			CurInstr->optexpr = yyvsp[-2].expr ;
			CurInstr->combine =  6 ;
			yyval.num  =  (1 << 24)  |  ((yyvsp[-4].num ) << 16)  | yyvsp[0].num ;
		}
break;
case 273:
#line 6266 "gasm.y"
{
			CurInstr->optexpr = yyvsp[-1].expr ;
			CurInstr->combine =  6 ;
			yyval.num  =  ((yyvsp[-4].num ) << 16)  | yyvsp[0].num ;
		}
break;
case 274:
#line 6272 "gasm.y"
{
			CurInstr->optexpr = yyvsp[-1].expr ;
			CurInstr->combine =  6 ;
			yyval.num  =  ((yyvsp[-4].num ) << 16)  | yyvsp[0].num ;
		}
break;
case 275:
#line 6283 "gasm.y"
{
			/* LEA is used to load the absolute address of a */
			/* label at run time. This version is currently */
			/* limited to the addressability of one data */
			/* processing add or sub. */

			/* note constexpr, not armconstexpr as we don't allow */
			/* non pc relative constants in LEA's */

			/* set PC as src and immediate mode in instruction */
			CurInstr->opcode = yyvsp[-3].mnem ->template |  ((yyvsp[-2].num ) << 12)  					|  ((0xF) << 16)  |  (1 << 25) ;


			/* After evaluation the constant should have eight */
			/* subtracted from it (to take account of the */
			/* pipeline). This value must then be formed into */
			/* an eight bit value + a rotation 4*2 rotation */

			CurInstr->optexpr = yyvsp[0].expr ;
			CurInstr->combine =  12 ;
		}
break;
case 276:
#line 6305 "gasm.y"
{
			/* NOP is simply a mov r0, r0 */
			CurInstr->opcode = yyvsp[0].mnem ->template;
		}
break;
case 277:
#line 6316 "gasm.y"
{	yyval.expr  = yyvsp[0].expr ; }
break;
case 280:
#line 6338 "gasm.y"
{	yyval.expr  = NewExprNum(yyvsp[0].num );}
break;
#line 8091 "y.tab.c"
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
