#include <stdio.h>
#include <stdlib.h>

typedef enum ETYPE {
E_NULL=0,       E_SYMBOL,    E_PLUSOP,    E_MINUSOP,   E_TIMESOP,
E_DIVIDEOP,     E_IREGISTER, E_FREGISTER, E_CREGISTER, E_REGOFFSET,
E_REGOFFSETINC, E_INSTR,     E_IMMEDIATE, E_NUMBER,    E_UMINUS,
E_LSHIFTOP,     E_RSHIFTOP,  E_ANDOP,     E_XOROP,     E_OROP,
E_MODOP,        E_STRING,    E_GT,        E_LT,        E_GE,
E_LE,           E_EQ,        E_NE,        E_NOT,       E_DOT,
E_MODNUM,       E_AT,        E_IMAGESIZE
} ETYPE;

#define E_OPMASK 0xff

#define E_BINARYOP 0x80000000

#define E_PLUS (E_PLUSOP|E_BINARYOP)
#define E_MINUS (E_MINUSOP|E_BINARYOP)
#define E_TIMES (E_TIMESOP|E_BINARYOP)
#define E_DIVIDE (E_DIVIDEOP|E_BINARYOP)
#define E_LSHIFT (E_LSHIFTOP|E_BINARYOP)
#define E_RSHIFT (E_RSHIFTOP|E_BINARYOP)
#define E_AND (E_ANDOP|E_BINARYOP)
#define E_XOR (E_XOROP|E_BINARYOP)
#define E_OR (E_OROP|E_BINARYOP)
#define E_MOD (E_MODOP|E_BINARYOP)

/* Directives */

#define D_CODE       0x01
#define D_BSS        0x02
#define D_INIT       0x03
#define D_BYTE       0x09
#define D_SHORT      0x0a
#define D_WORD       0x0c
#define D_IMAGESIZE  0x0e
#define D_MODULE     0x20
#define D_GLOBAL     0x22
#define D_LABEL      0x23
#define D_DATA       0x24
#define D_COMMON     0x25
#define D_LABELOFF   0x0f
#define D_DATASYMB   0x10
#define D_DATAMODULE 0x11
#define D_MODNUM     0x12
#define D_PATCH      0x13
#define D_ADD        (D_PATCH+0)
#define D_SHIFT      (D_PATCH+1)

#define EF_BADSYMBOL   1
#define EF_PC_RELATIVE 2
#define EF_DIRECTIVE   4

typedef enum STYPE {
S_NULL=0,    S_INSTR,     S_LABEL,     S_EQU,   S_SET,
S_IREGISTER, S_FREGISTER, S_CREGISTER, S_MACRO, S_DATA
} STYPE;

typedef long int32;
typedef unsigned long uint32;
typedef char bool;
typedef unsigned char uchar;

#define variable 1

typedef struct TextSeg {
   struct TextSeg *cdr;
   char   text[variable];
} TextSeg;

typedef struct Symbol {
   struct Symbol     *cdr;
   STYPE              symtype;
   int                symflags;
   union {
        int32              symval;
        struct InstrInfo  *symins;
        struct TextSeg    *macrotext;
   } symv;
   char               name[variable];
} Symbol;

#define sf_eval1 1
#define sf_eval2 2
#define sf_invisible 4
#define sf_exported 8

typedef struct Expression {
   struct Expression *cdr;
   ETYPE              exprtype;
   struct Expression *exprlist;
   union {
        struct Expression *expr;
        struct Symbol     *symbol;
        int32              value;
        char              *text;
   } e1;
   union {
        struct Expression *expr;
        int32              value;
        int32              insmodifiers;
   } e2;
} Expression;

typedef struct Line {
   Symbol *label;
   Expression *instr;
   Expression *args;
} Line;

typedef struct InstrInfo {
   uint32  base;
   uint32  modifiers;
   int     nargs;
   uint32  argmodes[variable];
} InstrInfo;

typedef union CodeBuf {
       char b1;
       short b2;
       uint32 b4;
} CodeBuf;


#define CODEBUFSIZE 1024
#define CODEBUFSIZEMASK 0x3ff
#define NCODESEGS 1024

typedef struct CodeSeg {
   char    codeflags[CODEBUFSIZE];
   CodeBuf codebuf[CODEBUFSIZE];
} CodeSeg;

/* Modifiers bits */
#define I_CM  0x0001
#define I_BM  0x0002
#define I_SM  0x0004
#define I_LM  0x0008
#define I_DM  0x0010
#define I_QM  0x0020
#define I_SSM 0x0040
#define I_SDM 0x0080
#define I_DSM 0x0100
#define I_DDM 0x0200
#define I_TM  0x0400
#define I_BADM 0x800
#define I_LMASK ((I_BADM<<1)-1) /* Mask of above bits */

#define I_DIM   0x1000
#define I_OLT   0x2000  /* Other length encoding */
#define I_LNEEDED 0x4000
#define I_PSEUDO 0x8000
#define I_FLUSH  0x10000

#define I_X (I_BM|I_SM|I_LM|I_LNEEDED)
#define I_Y (I_LM|I_DM|I_QM|I_LNEEDED)
#define I_Z (I_LM|I_DM|I_LNEEDED)
#define I_P (I_SSM|I_SDM|I_DDM|I_LNEEDED)
#define I_C (I_CM|I_LNEEDED)
#define I_D (I_DM|I_LNEEDED)
#define I_T (I_TM)
#define I_DD (I_DDM|I_LNEEDED)
#define I_W (I_SSM|I_DDM|I_LNEEDED)

/* Pseudo op codes */

enum { PS_GET=1, PS_SET,  PS_EQU,   PS_DC,   PS_NOLIST,
       PS_LIST,  PS_EQUR, PS_IF,    PS_IFD,  PS_IFND,
       PS_ELSE,  PS_ENDC, PS_ERROR, PS_DS,   PS_GLOBAL,
       PS_DATA,  PS_INIT, PS_MODULE,PS_XDATA,PS_XLABEL  };

/* Addressing modes */

#define M_IREG        0x00001
#define M_FREG        0x00002
#define M_CONST       0x00004
#define M_AUTOINC     0x00008
#define M_REGOFF      0x00010
#define M_CTRLREG     0x00020

/* The following bits define how a M_CONST (or constant in M_REGOFF)
   is to be formatted in the instruction */
#define M_ITYPE       0x00040  /* Instructions that set the I bit */
#define M_SPLITOFFSET 0x00080
#define M_IMM5        0x00100
#define M_BRX         0x00200

/* And the next ones define which field a register gets put in*/
#define M_S1FLD       0x00400
#define M_S2FLD       0x00800
#define M_DFLD        0x01000

#define M_NOREG       0x02000   /* Reg not allowed as src1 in REGOFF syntax */
#define M_STRING      0x04000
#define M_SHFT0       0x08000
#define M_OPTIONAL    0x10000

#define M_SRC1   (M_IREG|M_CONST|M_S1FLD|M_ITYPE)
#define M_SRC1NI (M_IREG|M_S1FLD)
#define M_SRC1S  (M_IREG|M_CONST|M_IMM5|M_S1FLD)
#define M_SRC2   (M_IREG|M_S2FLD)
#define M_RDEST  (M_IREG|M_DFLD)
#define M_SBROFF (M_CONST|M_SPLITOFFSET|M_BRX)
#define M_LBROFF (M_CONST|M_BRX)

#define M_FSRC1  (M_FREG|M_S1FLD)
#define M_FSRC2  (M_FREG|M_S2FLD)
#define M_FRDEST (M_FREG|M_DFLD)

typedef struct Relocation {
   struct Relocation *cdr;
   Expression        *expr;
   uint32             pcloc;
   int                argno;
   uint32             amode;
} Relocation;

typedef struct Macro {
   TextSeg *line;
   char    *tptr;
   char    *argtext;
   TextSeg *args;
   int      macronum;
   char     label[20];
} Macro;

typedef struct AFILE {
   union {
      struct { FILE *stream;
               char *buf;
             } file;
      struct Macro *macro;
         } io;
   int   lineno;
   uint32 flags;
   char *name;
} AFILE;

/* AFILE->flags bits */
#define ff_nlfound 1
#define ff_macro   2
#define ff_macrolabelvalid 4     /* Only in macro expansion */

/* Debugging flags */
#define DEBUG_MACRO 1

/* Listing related flags */
#define LF_NOCODE 1

#define LO_CODEONLY 1
#define LO_NOTMACRO 2

extern void       *aalloc(size_t n);
extern void        addtosrcline(char *s, int l);
extern void        advancecodebuf(void);
extern void        afree(void *p);
extern int32       arg_boundary( Expression *ins);
extern void        assemble(Line *line);
extern int         checkargs(InstrInfo *ii, Expression *args);
extern Symbol     *checkdatasymb(Expression *e);
extern Symbol     *checklabelsymb(Expression *e);
extern int         checkmodifiers(InstrInfo *ii, Expression *ins);
extern bool        check_pc_boundary(int32 n);
extern void        compile_pseudo(Symbol *label, Expression *ins,
                                                   Expression *args);
extern void        directive( int dir, ...);
extern void        endoutput(void);
extern int32       evaluate(Expression *ins, Expression *expr, int argno);
extern void        free_expressions(void);
extern void        initcodebuf(void);
extern void        syminit(void);
extern void        lexinit(void);
extern void        listmessage(char *ms, char *s, va_list ap);
extern Symbol     *lookup(char *s, int l, bool enter, STYPE type);
extern Expression *newexpression(void);
extern Symbol     *newsymbol(char *s,int l);
extern void        outlistline(void);
extern void        output(int csize,...);
extern void        outputexpr(int size, Expression *expr);
extern int         performget(char *s, bool rootfile);
extern void        reinitcodebuf(uint32 bufstart);
extern void        resolvesym(Symbol *sym, int32 val, STYPE type);
extern void        setlabel(Symbol *lab);
extern void        setlistfile(char *s);
extern bool        setobjectfile(char *s);
extern bool        setsrecfile(char *s);
extern void        startmacro(Symbol *sym, TextSeg *args);
extern void        start_skipping(void);
extern void        stop_skipping(void);

extern void warn(char *s,...);
extern void error(char *s,...);
extern void fatal(char *s,...);
extern void parmerror(char *s, ...);

#define MAXGETFILES 20
#define LINE_BUF_SIZE 100
#define MAXNAME 100  /* Maximum file name length */

typedef struct codeptr {
   uint32 p;
   int    seg;
} codeptr;

extern uint32   bufstart;
extern CodeSeg *code[NCODESEGS];
extern codeptr  codep;
extern int32    codevalsize;
extern AFILE   *currentfile;
extern uint32   dataloc;
extern int      debugflags;
extern bool     doinitcodebuf;
extern int32    evalflags;
extern bool     evaluate_check;
extern int      filenum;
extern AFILE    filestack[MAXGETFILES];
extern char    *includepaths;
extern Symbol  *lastlabel;
extern codeptr  listcodep;
extern FILE    *listingfile;
extern int      listinglevel;
extern int      listing_flags;
extern int      listing_lineno;
extern int      listing_options;
extern char    *listtext;
extern int      module_number;
extern AFILE   *nextcurrentfile;
extern int      pass;
extern int      passevalflag;
extern uint32   pcloc;
extern uint32   dot;
extern int      skipping_level;
extern int      yydebug;
