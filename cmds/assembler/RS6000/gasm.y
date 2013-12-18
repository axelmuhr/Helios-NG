#line 1 "/pds/nickc/RTNucleus/cmds/assembler/gasm.ypp"
 
#line 1 "/pds/nickc/RTNucleus/cmds/assembler/warn.h"
 
 
 
 
#line 3 "/pds/nickc/RTNucleus/cmds/assembler/gasm.ypp"

 
























 
 

%{

 






 
#line 1 "/pds/nickc/RTNucleus/cmds/assembler/gasm.h"
 



















#line 1 "stdio.h"
___toplevel

















typedef unsigned int size_t; 



typedef char *__va_list[1]; 





typedef struct __fpos_t_struct
{ unsigned long __lo; 
} fpos_t;
 

typedef struct __FILE_struct FILE;
 







 


 



 

 






 

extern FILE __stdin, __stdout, __stderr;


 

 

 

extern int remove(const char * );
 
extern int rename(const char * , const char * );
 
extern FILE *tmpfile(void);
 
extern char *tmpnam(char * );
 

extern int fclose(FILE * );
 
extern int fflush(FILE * );
 
extern FILE *fopen(const char * , const char * );
 
extern FILE *freopen(const char * , const char * ,
 FILE * );
 
extern void setbuf(FILE * , char * );
 
extern int setvbuf(FILE * , char * ,
 int , size_t );
 


extern int fprintf(FILE * , const char * , ...);
 
extern int printf(const char * , ...);
 
extern int sprintf(char * , const char * , ...);
 

extern int fscanf(FILE * , const char * , ...);
 
extern int scanf(const char * , ...);
 
extern int sscanf(const char * , const char * , ...);
 

extern int vprintf(const char * , __va_list );
 
extern int vfprintf(FILE * ,
 const char * , __va_list );
 
extern int vsprintf(char * , const char * , __va_list );
 

extern int fgetc(FILE * );
 
extern char *fgets(char * , int , FILE * );
 
extern int fputc(int , FILE * );
 
extern int fputs(const char * , FILE * );
 
extern int getc(FILE * );
 

extern int (getchar)(void);
 
extern char *gets(char * );
 
extern int putc(int , FILE * );
 

extern int (putchar)(int );
 
extern int puts(const char * );
 
extern int ungetc(int , FILE * );
 

extern size_t fread(void * ,
 size_t , size_t , FILE * );
 
extern size_t fwrite(const void * ,
 size_t , size_t , FILE * );
 

extern int fgetpos(FILE * , fpos_t * );
 
extern int fseek(FILE * , long int , int );
 
extern int fsetpos(FILE * , const fpos_t * );
 
extern long int ftell(FILE * );
 
extern void rewind(FILE * );
 

extern void clearerr(FILE * );
 

extern int feof(FILE * );
 
extern int ferror(FILE * );
 
extern void perror(const char * );
 




#line 22 "/pds/nickc/RTNucleus/cmds/assembler/gasm.h"
#line 1 "stdlib.h"
___toplevel


















 typedef int wchar_t; 







typedef struct div_t { int quot, rem; } div_t;
 
typedef struct ldiv_t { long int quot, rem; } ldiv_t;
 








 


 

 

 

extern double atof(const char * );
 
extern int atoi(const char * );
 
extern long int atol(const char * );
 

extern double strtod(const char * , char ** );
 
extern long int strtol(const char * , char **, int );
 
extern unsigned long int strtoul(const char * ,
 char ** , int );
 

extern int rand(void);
 
extern void srand(unsigned int );
 
extern int _ANSI_rand(void);
 
extern void _ANSI_srand(unsigned int );
 

extern void *calloc(size_t , size_t );
 
extern void free(void * );
 
extern void *malloc(size_t );
 
extern void *realloc(void * , size_t );
 

extern void abort(void);
 
extern int atexit(void (* )(void));
 
extern void exit(int );
 

extern char *getenv(const char * );
 
extern int system(const char * );
 

extern void *bsearch(const void *key, const void * ,
 size_t , size_t ,
 int (* )(const void *, const void *));
 
extern void qsort(void * , size_t , size_t ,
 int (* )(const void *, const void *));
 

extern int abs(int );
 
extern div_t div(int , int );
 
extern long int labs(long int );
 
extern ldiv_t ldiv(long int , long int );
 


extern int mblen(const char * , size_t );
 
extern int mbtowc(wchar_t * , const char * , size_t );
 
extern int wctomb(char * , wchar_t );
 


extern size_t mbstowcs(wchar_t * , const char * , size_t );
 
extern size_t wcstombs(char * , const wchar_t * , size_t );
 




#line 23 "/pds/nickc/RTNucleus/cmds/assembler/gasm.h"
#line 1 "string.h"
___toplevel





















extern void *memcpy(void * , const void * , size_t );
 
extern void *memmove(void * , const void * , size_t );
 
extern char *strcpy(char * , const char * );
 
extern char *strncpy(char * , const char * , size_t );
 

extern char *strcat(char * , const char * );
 
extern char *strncat(char * , const char * , size_t );
 



extern int memcmp(const void * , const void * , size_t );
 
extern int strcmp(const char * , const char * );
 
extern int strncmp(const char * , const char * , size_t );
 
extern int strcoll(const char * , const char * );
 

extern size_t strxfrm(char * , const char * , size_t );
 


extern void *memchr(const void * , int , size_t );
 
extern char *strchr(const char * , int );
 
extern size_t strcspn(const char * , const char * );
 
extern char *strpbrk(const char * , const char * );
 
extern char *strrchr(const char * , int );
 
extern size_t strspn(const char * , const char * );
 
extern char *strstr(const char * , const char * );
 
extern char *strtok(char * , const char * );
 

extern void *memset(void * , int , size_t );
 
extern char *strerror(int );
 
extern size_t strlen(const char * );
 




#line 24 "/pds/nickc/RTNucleus/cmds/assembler/gasm.h"


 













typedef long int            int32;
typedef long unsigned int   unsigned32;
typedef short int           int16;
typedef short unsigned int  unsigned16;
typedef signed char         int8;
typedef unsigned char       unsigned8;

typedef int                 bool;





	 





	typedef struct {



	  int32 msd,lsd;                 

	} DbleBin;

	typedef struct FloatBin {
	  int32 val;
	} FloatBin;

	typedef	DbleBin	Dble ;



 



#line 1 "/pds/nickc/RTNucleus/cmds/assembler/hdr_ARM.h"
 
























#line 1 "/pds/nickc/RTNucleus/cmds/assembler/binary.h"
 






















 


 



 





 









 

















 

































 

































































 

































































































































 

































































































































































































































































 

































































































































































































































































































































































































































































































































 



































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































 
#line 27 "/pds/nickc/RTNucleus/cmds/assembler/hdr_ARM.h"


 
 


 
 
 
 
 
 
 
 
 
 


typedef struct Mnemonic {
	char		*name;		 
	int		token;		 
	unsigned	Template;	 
} Mnemonic;


 
 
 
 
 
 
 


typedef struct Instruction {
	int			opcode;		 
	struct Expression	*optexpr;	 
						 
	int			combine;	 
						 
	 
	 
	struct Expression	*optcoprocnum;	 
						 
	 
	 
	struct Expression	*optcoprocaux;	 
						 
} Instruction;


 
 

 


 


 
 


 
 


 


 
 


 
 


 
 
 
 


 
 
 
 


 


 


 


 
 
 



 
 

 

















 


















 
 

 

























 
 













 
 






 




 
 
 


 
 





 
 




















 


					 


 







 
 

 








 





 
 




 



 







 
 




 




 




 
 




 





 
 




 
 





 





 
 





 
 



 







 
 




 



 





 
 




 






 
 
 





#line 80 "/pds/nickc/RTNucleus/cmds/assembler/gasm.h"


 













 











 
 
 







 
 
 






 
 
 





 
 
 
 



 
 






















 
 
 

typedef struct Symbol {
	struct Symbol	*next;	 
				 
	int	what;		 
				 
	int	referenced;	 
	union {
		int	token;	 
		int	value;	 
			 
			 
		struct Mnemonic *mnemonic;
			 
			 
			 
			 
		struct Common *misc;
			 
		void	*any;
	} type;
	char	name[1];	 
} Symbol;


 
 
 
 
 
 

typedef struct Common {
	char	*name;		 
	int	token;		 
	int	value;		 
} Common;


 
 
 
 
 
 
 
 

typedef struct ParseTreeItem {
	struct ParseTreeItem	*next;	 

	int		linenum;	 

	int		logicalPC;	 

	int		what;		 
					 
					 
					 

	union {					 
		int			num;	 
		struct FloatList	*flist;	 
		struct NameList		*nlist;	 
		struct Symbol		*symb;	 
		struct ConstList	*clist;	 
		struct Expression	*expr;	 

						 
		struct {
			char			*name;
			struct Expression	*expr;
		} datacommon;

						 
		struct {
				 
			struct Expression	*mcpatch;
				 
			struct Instruction	*instr;
				 
				 
			struct Patch		*patch;
		} patchinstr;

		 
		 
		 
		struct Instruction	*instr;	 
	} type;
} ParseTreeItem;


 
 
 
 
 
 

 
 
 

typedef struct ConstList {
	struct ConstList	*next;		 
	int			what;		 
	union {
		struct Expression	*expr;	 
		struct Patch		*patch;	 
		char			*str;	 
	} type;
} ConstList;


 
 

typedef struct FloatList {
	struct FloatList *next;		 
	Dble		  value;
} FloatList;


 
 

typedef struct NameList {
	struct NameList	*next;		 
	char		*name;		 
} NameList;


 
 
 

typedef struct Patch {
	int	what;		 
				 
	union	{
		char		*name;		 
						 

		struct Patch	*patch;		 

		struct {			 
			struct Expression	*expr;
			struct Patch		*patch;
		} shift;
	} type;
} Patch;


 

























typedef struct Expression {
	int		what;			 
	union {
		struct {			 
			int	Operator;
			struct Expression *left; 	
			struct Expression *right;
		} expr;
		int		number;		 
		char		*name;		 
	} type;
} Expression;


 
 

extern int		CurLine;		 
extern int		StartLine;		 
extern int 		curPC;			 

extern int		errors, warnings;	 

extern FILE		*InputFile;		 
extern FILE		*OutputFile;		 

extern ParseTreeItem	HeaderParseTreeItem;	 
						 

extern char		*InputFileName;		 
extern char		*OutputFileName;

extern int		ModuleHeadTail;		 
extern int		ModuleNumber;
extern char		ModuleName[32];
extern int		ModuleVersion;


 
 

 
extern int		yylex(void);
extern void		InitLex(void);
extern void		ClearInput(void);

 
extern int		yyparse(void);
extern ParseTreeItem	*NewParseTreeItem(int type);
extern ConstList	*NewConstItem(int type);
extern FloatList	*NewFloatConstItem(Dble);
extern Patch		*NewPatchItem(int type);
extern Expression	*NewExpr(Expression *le, int op, Expression *re);
extern Expression 	*NewExprSymbRef(char *name);
extern Expression	*NewExprNum(int num);

 
extern unsigned int	Hash(char *name);
extern void		InitSymbTab(void);
extern Symbol		*NewSymb(char *name, int type, int value);
extern void		NewSymbStruct(char *name, int what, void *sstruct);
extern void		NewMnemonic(Mnemonic *mne);
extern Symbol		*FindSymb(char *name);
extern Symbol		*CaseInsensitiveFindSymb(char *name);
extern void		PrintSymbTab(void);

 
extern int		Eval(struct Expression *expr, int pc);

 
void OutputGHOF(ParseTreeItem *pti);	 

 
extern void		Pass2(Instruction *instr, int pc);

 
 
extern void		WriteCodeWord(int w);
extern void		WriteCodeShort(int s);
extern void		WriteCodeByte(char b);
extern void		WriteCodeFloat(Dble d);
extern void		WriteCodeDouble(Dble d);




 
 




extern void fltrep_stod(const char *, DbleBin *) ;
extern bool fltrep_narrow_round(DbleBin *, FloatBin *) ;
extern bool flt_add(DbleBin *, DbleBin *, DbleBin *) ;
extern bool flt_subtract(DbleBin *, DbleBin *, DbleBin *) ;
extern bool flt_multiply(DbleBin *, DbleBin *, DbleBin *) ;
extern bool flt_divide(DbleBin *, DbleBin *, DbleBin *) ;
extern bool flt_negate(DbleBin *, DbleBin *) ;
extern bool flt_itod(DbleBin *, int32) ;


 
extern void 		Fatal(char *s);
extern void 		Error(char *s);
extern void 		Warn(char *s);
extern void 		Note(char *s);
extern char		*itoabin(char *str, int nbits, int bin);

 








#line 43 "/pds/nickc/RTNucleus/cmds/assembler/gasm.ypp"


 

			 
int			curPC = 0;

			 
			 
ParseTreeItem		HeaderParseTreeItem;


int			StartLine = 0;


 

			 
static int		list_size_count = 0;

			 
static ParseTreeItem	*LastParseTreeItem = &HeaderParseTreeItem;






%}


 
 

 
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

 
%token <str>	NAME
%token <num>	NUMBER
%token <num>	CHARCONST
%token <flt>	FLOATNUM
%token		LSHIFT RSHIFT

 
%token		USERERROR USERWARNING USERNOTE

 
 

 
%token 		BYTE SHORT WORD FLOATY DOUBLE BLOCKBYTE BLOCKSHORT BLOCKWORD SPACE ALIGN

  
%token 		DATA COMMON CODETABLE EXPORT IMPORT

 
%token		MODULE INIT REF PATCHINSTR

 
%token		MODSIZE MODNUM DATASYMB CODESYMB DATAMODULE
%token		LABELREF CODESTUB ADDRSTUB BYTESWAP SHIFT P_ADD P_OR

 
%token <str>	STRINGCONST
%token <str>	LABEL

 
 
%token		PLUSPLUS MINUSMINUS

 
 
%token <instr>	INSTRUCTION


 
 

%type <num>	imm_constexpr opt_imm_constexpr
%type <flt>	imm_fp_constexpr
%type <expr>	instrpatch

%type <clist>	codeconstlist codeconst
%type <flist>	floatconstlist
%type <expr>	constexpr opt_constexpr
%type <patch>	stdpatch
%type <nlist>	namelist


 
 

%left	'|'		 
%left	'^'
%left	'&'
%left	LSHIFT RSHIFT
%left	'+' '-'
%left	'*' '/' '%'
%left	UNARY		 


 
 
 

 
#line 1 "/pds/nickc/RTNucleus/cmds/assembler/toks_ARM.ypp"
 



















 
 
%{
#line 1 "/pds/nickc/RTNucleus/cmds/assembler/ghof.h"
 





















 





























 







 


























 







 

extern int codesize;


 

void FlushCodeBuffer(void);		 
void GHOFEncode(int n);			 
void ObjWriteByte(char b);		 
void ObjWrite(char *buf, int size);	 




#line 25 "/pds/nickc/RTNucleus/cmds/assembler/toks_ARM.ypp"

	 
	Instruction	*CurInstr =  0 ;
%}



 
 

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


 
 

%token	<num>	PATCHARMDT PATCHARMDP PATCHARMJP
%token	<num>	PATCHARMDPLSB PATCHARMDPMID PATCHARMDPREST


 
 

%token	<num>	R0 R1 R2 R3 R4 R5 R6 R7 R8 R9 R10 R11 R12 R13 R14 R15


 
 

%token	<num>	CR0 CR1 CR2 CR3 CR4 CR5 CR6 CR7 CR8 CR9 CR10 CR11 CR12
%token	<num>	CR13 CR14 CR15


 
 

%token	<num>	LSL LSR ASR ROR

%token	<num>	RRX

%token	<num>	CPSR SPSR CPSR_FLG SPSR_FLG

 
 

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



 
#line 162 "/pds/nickc/RTNucleus/cmds/assembler/gasm.ypp"


 
 

%start statement


 
%%		 
 


statement:	 
		| statement {StartLine = CurLine;} label
		| statement {StartLine = CurLine;} mnemonic
		| statement error
		{
			yyclearin;	 
			yyerrok;	 
			ClearInput();	 
		}
		;


 
label:		LABEL
		{
			 
			if (FindSymb($1)) {
				char	Err[128];

				sprintf(Err, "redefinition of label \"%s\"", $1);
				Error(Err);
				 
			}
			else {
				 




				Symbol *s = NewSymb($1,  0 , curPC);


				 
				 
				ParseTreeItem *pti = NewParseTreeItem(LABEL);

				pti->type.symb = s;










			}
		}
		;


mnemonic:	pseudo_op
		| machine_op
		;


 
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


 
 

 
 
byte:		BYTE codeconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(BYTE);

			 
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
			curPC += list_size_count * 4;  
		}
		;

double:		DOUBLE floatconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(DOUBLE);

			pti->type.flist = $2;
			curPC += list_size_count * 8;  
		}
		;













 
 
blockbyte:	BLOCKBYTE imm_constexpr ',' codeconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(BYTE);
			int remain = $2 - list_size_count;

			pti->type.clist = $4;

			 
			 
			if (remain < 0) {
				Error("blockbyte: block size exceeded by data supplied");
			}
			else if (remain > 0) {
				pti = NewParseTreeItem(SPACE);
				pti->type.num = remain;
			}
			 

			curPC += $2 * sizeof(char);
		}
		;

blockshort:	BLOCKSHORT imm_constexpr ',' codeconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(SHORT);
			int remain = $2 - list_size_count;

			pti->type.clist = $4;

			 
			 
			if (remain < 0) {
				Error("blockshort: block size exceeded by data supplied");
			}
			else if (remain > 0) {
				pti = NewParseTreeItem(SPACE);
				pti->type.num = remain * (2 * sizeof(char));
			}
			 

			curPC += $2 * (2 * sizeof(char));
		}
		;

blockword:	BLOCKWORD imm_constexpr ',' codeconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(WORD);
			int remain = $2 - list_size_count;

			pti->type.clist = $4;

			 
			 
			if (remain < 0) {
				Error("blockword: block size exceeded by data supplied");
			}
			else if (remain > 0) {
				pti = NewParseTreeItem(SPACE);
				pti->type.num = remain * sizeof(int);
			}
			 

			curPC += $2 * sizeof(int);
		}
		;


 
 
space:		SPACE imm_constexpr
		{
			 
			ParseTreeItem *pti = NewParseTreeItem(SPACE);

			pti->type.num = $2;

			curPC += $2;
		}
		;


 
 
align:		ALIGN opt_imm_constexpr
		{
			ParseTreeItem *pti;
			int pad;

			 
			pad =  (((( curPC ) + ( ($2) ? $2 : 4 ) - 1) & ~(( ($2) ? $2 : 4 ) - 1)) -  curPC ) ;

			if (pad != 0) {
				 
				 
				 
				 

				pti = NewParseTreeItem(SPACE);
				pti->type.num = pad;

				curPC += pad;
			}
		}
		;


 
 

 
data:		DATA NAME ',' constexpr
		{
			ParseTreeItem *pti = NewParseTreeItem(DATA);

			pti->type.datacommon.name = $2;
			pti->type.datacommon.expr = $4;




		}
		;

 
common:		COMMON NAME ',' constexpr
		{
			ParseTreeItem *pti = NewParseTreeItem(COMMON);

			pti->type.datacommon.name = $2;
			pti->type.datacommon.expr = $4;




		}
		;

 
codetable:	CODETABLE namelist
		{
			ParseTreeItem *pti = NewParseTreeItem(CODETABLE);

			pti->type.nlist = $2;




		}
		;


 
export:		EXPORT namelist
		{
			ParseTreeItem *pti = NewParseTreeItem(EXPORT);

			pti->type.nlist = $2;




		}
		;


 
import:		IMPORT namelist
		{
			 
			ParseTreeItem *pti = NewParseTreeItem(IMPORT);
			NameList *n = $2;

			pti->type.nlist = $2;

			 
			while ( n !=  0 ) {
				NewSymb(n->name,  4 , 0);
				n = n->next;
			}



		}
		;


 
 

 
module:		MODULE opt_constexpr
		{
			ParseTreeItem *pti = NewParseTreeItem(MODULE);

			 
			pti->type.expr = ($2) ? $2 : NewExprNum(-1);




		}
		;

 
init:		INIT
		{
			NewParseTreeItem(INIT);

			 




			curPC += sizeof(int);
		}
		;

 
ref:		REF namelist
		{
			ParseTreeItem *pti = NewParseTreeItem(REF);

			pti->type.nlist = $2;




		}
		;

 
patchinstr:	PATCHINSTR '(' instrpatch ',' stdpatch ',' machine_op ')'
		{
			Instruction *instr;

			if (LastParseTreeItem->what == INSTRUCTION) {
				 
				 
				 
				 

				 
				instr = LastParseTreeItem->type.instr;

				 
				LastParseTreeItem->what = PATCHINSTR;
				LastParseTreeItem->type.patchinstr.mcpatch = $3;
				LastParseTreeItem->type.patchinstr.patch = $5;

				 
				LastParseTreeItem->type.patchinstr.instr =instr;



			}
			else
				Error("patchinstr failed due to instruction syntax error");
		}
		;


 
 

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


 
 

 
 
 
 
 

stdpatch :	MODSIZE
		{
			$$ = NewPatch(MODSIZE);			 



		}
		| MODNUM
		{
			$$ = NewPatch(MODNUM);			 



		}
		| DATASYMB '(' NAME ')'
		{
			$$ = NewPatch(DATASYMB);		 
			$$->type.name = $3;



		}
		| CODESYMB '(' NAME ')'
		{
			$$ = NewPatch(CODESYMB);		 
			$$->type.name = $3;



		}
		| DATAMODULE '(' NAME ')'
		{
			$$ = NewPatch(DATAMODULE);		 
			$$->type.name = $3;



		}
		| LABELREF '(' NAME ')'
		{
			 
			 
			$$ = NewPatch(LABELREF);		 
			$$->type.name = $3;




		}

		| CODESTUB '(' NAME ')'
		{
			$$ = NewPatch(CODESTUB);		 
			$$->type.name = $3;




		}

		| ADDRSTUB '(' NAME ')'
		{
			$$ = NewPatch(ADDRSTUB);		 
			$$->type.name = $3;




		}

		 
		| BYTESWAP '(' stdpatch ')'
		{
			$$ = NewPatch(BYTESWAP);		 
			$$->type.patch = $3;




		}
		| SHIFT '(' constexpr ',' stdpatch ')'
		{
			$$ = NewPatch(SHIFT);			 
			$$->type.shift.expr = $3;
			$$->type.shift.patch = $5;




		}
		| P_ADD '(' constexpr ',' stdpatch ')'
		{
			$$ = NewPatch(P_ADD);			 
			$$->type.shift.expr = $3;
			$$->type.shift.patch = $5;




		}
		| P_OR '(' constexpr ',' stdpatch ')'
		{
			$$ = NewPatch(P_OR);			 
			$$->type.shift.expr = $3;
			$$->type.shift.patch = $5;




		}
		;


 
 

namelist:	NAME
		{
			 
			NameList *n = (NameList *)malloc(sizeof(NameList));

			if (n ==  0 )
				Fatal("Out of memory whilst building name list");

			n->next =  0 ;
			n->name = $1;
			$$ = n;
		}
		| namelist ',' NAME
		{
			 
			NameList *n = $1;
			NameList *nn = (NameList *)malloc(sizeof(NameList));

			if (nn ==  0 )
				Fatal("Out of memory whilst building name list");

			 
			 
			nn->next = n->next;
			n->next = nn;
			nn->name = $3;

			$$ = n;	 
		}
		;


 
 

 
 
 

codeconstlist:	{ list_size_count = 0; }	 
		codeconst
		{
			$$ = $2;	 
		}
		| codeconstlist ',' codeconst
		{
			ConstList *n = $1;	 

			 
			while (n->next !=  0 )
				n = n->next;

			n->next = $3;	 	

			$$ = $1;	 
		}
		;


floatconstlist:	{ list_size_count = 0; }	 
		imm_fp_constexpr
		{
			 
			$$ = NewFloatConstItem($2);
			list_size_count++;
		}
		| floatconstlist ',' imm_fp_constexpr
		{
			FloatList *fl = $1;	 

			 
			while (fl->next !=  0 )
				fl = fl->next;
			 
			fl->next = NewFloatConstItem($3);
			list_size_count++;

			$$ = $1;	 
		}
		;


 

codeconst:	constexpr
		{
			 
			$$ = NewConstItem( 1 );

			$$->type.expr = $1;
			list_size_count++;
		}
		| stdpatch
		{
			 
			$$ = NewConstItem( 3 );

			$$->type.patch = $1;
			list_size_count++;
		}
		| STRINGCONST
		{
			 
			$$ = NewConstItem( 4 );

			$$->type.str = $1;
			 
			 





			list_size_count += strlen($1);

		}
		;


 
 

 
 
 


 
 

 
 
 

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

		 
		| NUMBER

		 
		| CHARCONST

		;


 
 
 

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

		 
		| NUMBER
		{ (void) flt_itod (&$$, $1);}

		 
		| FLOATNUM
		;


 
opt_imm_constexpr:	 
		{



			$$ = 0;
		}
		| imm_constexpr
		;


 
 
 
 
 
 

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
			 
		{	$$ = NewExprNum($1);}
		| CHARCONST
			 
		{	$$ = NewExprNum($1);}
		| NAME
			 
		{	$$ = NewExprSymbRef($1);}
		;


 
opt_constexpr:	 
		{
			 




			$$ =  0 ;
		}
		| constexpr
		;










 
#line 1 "/pds/nickc/RTNucleus/cmds/assembler/rules_ARM.ypp"
 

















 













 

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



 
 
 
 
 
 


machine_op:	{
			 
			if ((CurInstr = (Instruction *)malloc(sizeof(Instruction))) == 0)
				Fatal("Out of Memory for new instruction");

			 
			CurInstr->opcode = 0;
			CurInstr->optexpr =  0 ;
			CurInstr->combine = 0;
			CurInstr->optcoprocnum =  0 ;
			CurInstr->optcoprocaux =  0 ;
		}
		ARM_op	 
		{
			 
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



 

branch:		branch_ops armconstexpr
		{
			CurInstr->opcode = $1->Template;
			CurInstr->optexpr = $2;

			 
			CurInstr->combine =  1 ;
		}
		;

branch_ops:	B | BL ;


 

dataproc:	move | comp | logic ;

move:		move_ops reg ',' op2
		{
			CurInstr->opcode = $1->Template |  (( $2 ) << 12)  | $4;
		}
		;

comp:		comp_ops reg ',' op2
		{
			CurInstr->opcode = $1->Template |  (( $2 ) << 16)  | $4;
		}
		;

logic:		logic_ops reg ',' reg ',' op2
		{
			CurInstr->opcode = $1->Template | 				 (( $2 ) << 12)  |  (( $4 ) << 16)  | $6;

		}
		| logic_ops reg ',' op2		 
		{
			CurInstr->opcode = $1->Template | 				 (( $2 ) << 12)  |  (( $2 ) << 16)  | $4;

		}
		;

op2:		shifttype
		{	$$ = $1; }
		| armconstexpr
		{
			 
			 
			CurInstr->optexpr = $1;
			CurInstr->combine =  2 ;
			 
			$$ =  (1 << 25) ;
		}
		;

shifttype:	reg
		{	$$ = $1; }
		| reg shiftname reg
		{	$$ =  (1 << 4)  | $1 | $2 |  (( $3 ) << 8) ; }
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


 

arm6psr:	mrs | msr ;

mrs:		MRS	reg ',' cpsr
		{
 
			CurInstr->opcode = $1->Template |  (( $2 ) << 12)  | $4;
		}
		;

msr:		MSR	cpsr_flg ',' reg_expr
		{
 
			if (($2 &  (1 << 16) ) && ($4 &  (1 << 25) )) {
				 
				Error("MSR instruction can only set psr flags with immediate operand");
			}
			CurInstr->opcode = $1->Template | $2 | $4;
		}
		;

reg_expr:	reg
		{	$$ = $1; }
		| armconstexpr 
		{
			 
			 
			CurInstr->optexpr = $1;
			CurInstr->combine =  2 ;
			 
			$$ =  (1 << 25) ;
		}
		;

cpsr:		CPSR | SPSR ;

cpsr_flg:	CPSR | CPSR_FLG | SPSR | SPSR_FLG ;


 

multiply:	MUL	reg ',' reg ',' reg
		{
			CurInstr->opcode = $1->Template | 				 (( $2 ) << 16)  | 				 ( $4 )  | 				 (( $6 ) << 8) ;




			if ($2 == $4)
				Error("Destination and first source operand "
				"must not be the same");

			if ($2 ==  0xF )
				Warn("Destination of R15 will have no "
				"effect, other than altering the PSR flags");

		}
		| MLA	reg ',' reg ',' reg ',' reg
		{
			CurInstr->opcode = $1->Template | 				 (( $2 ) << 16)  | 				 ( $4 )  | 				 (( $6 ) << 8)  | 				 (( $8 ) << 12) ;





			if ($2 == $4)
				Error("Destination and first source operand "
				"must not be the same");

			if ($2 ==  0xF )
				Warn("Destination of R15 will have no "
				"effect, other than altering the PSR flags");
		}
		;



 

datatran:	data_ops reg ',' addr
		{	CurInstr->opcode = $1->Template |  (( $2 ) << 12)  | $4; }
		;

addr:		armconstexpr	 
		{
			CurInstr->optexpr = $1;
			CurInstr->combine =  5 ;
			 
			$$ =  ((  0xF  ) << 16)  |  (1 << 24) ;
		}
		| preindex
		| postindex
		;

preindex:	'[' reg ']' optpling
		{
			 
			 
			$$ =  (1 << 24)  |  (( $2 ) << 16)  |  (1 << 23)  | $4;
		}
		| '(' reg ')' optpling
		{
			$$ =  (1 << 24)  |  (( $2 ) << 16)  |  (1 << 23)  | $4;
		}
		| '[' reg ',' armconstexpr ']' optpling
		{
			CurInstr->optexpr = $4;
			CurInstr->combine =  5 ;
			$$ =  (1 << 24)  |  (( $2 ) << 16)  | $6;
		}
		| '(' reg ',' armconstexpr ')' optpling
		{
			CurInstr->optexpr = $4;
			CurInstr->combine =  5 ;
			$$ =  (1 << 24)  |  (( $2 ) << 16)  | $6;
		}
		| '[' reg ',' indexreg optshift ']' optpling
		{
			$$ =  (1 << 24)  |  (1 << 25)  |  (( $2 ) << 16)  |
			     $4 | $5 | $7;
		}
		| '(' reg ',' indexreg optshift ')' optpling
		{
			$$ =  (1 << 24)  |  (1 << 25)  |  (( $2 ) << 16)  |
			     $4 | $5 | $7;
		}
		;

postindex:	'[' reg ']' ',' armconstexpr
		{
			CurInstr->optexpr = $5;
			CurInstr->combine =  5 ;
			$$ =  (( $2 ) << 16) ;
		}
		| '(' reg ')' ',' armconstexpr
		{
			CurInstr->optexpr = $5;
			CurInstr->combine =  5 ;
			$$ =  (( $2 ) << 16) ;
		}
		| '[' reg ']' ',' indexreg optshift
		{	$$ =  (1 << 25)  |  (( $2 ) << 16)  | $5 | $6; }
		| '(' reg ')' ',' indexreg optshift
		{	$$ =  (1 << 25)  |  (( $2 ) << 16)  | $5 | $6; }
		;

indexreg:	reg
		{	$$ =  (1 << 23)  | $1; }	 
		| '+' reg
		{	$$ =  (1 << 23)  | $2; }
		| '-' reg
		{	$$ = $2; }
		;

		 
optshift:	 
		{	$$ = 0; }
		| shiftname armconstexpr
		{
			CurInstr->optexpr = $2;
			CurInstr->combine =  3 ;
			$$ = $1;
		}
		| RRX
		;

 
optpling:	 
		{	$$ = 0; }
		| '!'
		{	$$ =  (1 << 21) ; }
		;

data_ops:	LDR | STR ;



 

ldstmult:	ldstmult_ops	reg optpling ',' reglist optclaret
		{	CurInstr->opcode = $1->Template |  (( $2 ) << 16)  				| $3 | $5 | $6;}

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

optclaret:	 
		{	$$ = 0; }
		| '^'
		{	$$ =  (1 << 22) ; }
		;

ldstmult_ops:	LDM
		{
			$$ = $1;
			if ($1->Template ==  0 )
				Error("LDM mnemonic must include a mode");
		}
		| STM
		{
			$$ = $1;

			if ($1->Template ==  0 )
				Error("STM mnemonic must include a mode");
		}
		;



 

softintr:	SWI	armconstexpr
		{
			CurInstr->opcode = $1->Template;
			CurInstr->optexpr = $2;
			CurInstr->combine =  9 ;
		}
		;



 

swap:		SWP reg ',' reg ',' '[' reg ']'
		{
			CurInstr->opcode = $1->Template |  (( $2 ) << 12)  |
				 ( $4 )  |  (( $7 ) << 16)  ;
		}
		| SWP reg ',' reg ',' '(' reg ')'
		{
			CurInstr->opcode = $1->Template |  (( $2 ) << 12)  |
				 ( $4 )  |  (( $7 ) << 16)  ;
		}
		;



 

		 
coproc:		cdo | cdt | crt ;


cdo:		CDO armconstexpr ',' armconstexpr ',' cp_reg ',' cp_reg ',' cp_reg optaux
		{
			CurInstr->opcode = $1->Template |  (( $6 ) << 12)  |
				 (( $8 ) << 16)  |  ( $10 ) ;

			CurInstr->optexpr = $4;  
			CurInstr->combine =  10 ;

			CurInstr->optcoprocnum = $2;  
			CurInstr->optcoprocaux = $11;  
		}
		;

cdt:		cdt_ops armconstexpr ',' cp_reg ',' cp_addr
		{
			CurInstr->opcode = $1->Template |  (( $4 ) << 12)  | $6;
			 
			CurInstr->optcoprocnum = $2;  
		}
		;


cdt_ops:	LDC | STC ;


crt:		crt_ops armconstexpr ',' armconstexpr ',' reg ',' cp_reg ',' cp_reg optaux
		{
			CurInstr->opcode = $1->Template |  (( $6 ) << 12)  |
				 (( $8 ) << 16)  |  ( $10 ) ;

			CurInstr->optexpr = $4;  
			CurInstr->combine =  11 ;

			CurInstr->optcoprocnum = $2;  
			CurInstr->optcoprocaux = $11;  
		}


crt_ops:	MCR | MRC ;


optaux:		 
		{	$$ =  0 ; }
		| ',' armconstexpr
		{	$$ = $2; }
		;


 
cp_addr:	armconstexpr	 
		{
			CurInstr->optexpr = $1;
			CurInstr->combine =  6 ;
			 
			$$ =  ((  0xF  ) << 16)  |  (1 << 24) ;
		}
		| cp_preindex
		| cp_postindex
		;

cp_preindex:	'[' reg ']' optpling
		{
			 
			 
			$$ =  (1 << 24)  |  (( $2 ) << 16)  |  (1 << 23)  | $4;
		}
		| '(' reg ')' optpling
		{
			$$ =  (1 << 24)  |  (( $2 ) << 16)  |  (1 << 23)  | $4;
		}
		| '[' reg ',' armconstexpr ']' optpling
		{
			CurInstr->optexpr = $4;
			CurInstr->combine =  6 ;
			$$ =  (1 << 24)  |  (( $2 ) << 16)  | $6;
		}
		| '(' reg ',' armconstexpr ')' optpling
		{
			CurInstr->optexpr = $4;
			CurInstr->combine =  6 ;
			$$ =  (1 << 24)  |  (( $2 ) << 16)  | $6;
		}
		;

cp_postindex:	'[' reg ']' ',' armconstexpr optpling
		{
			CurInstr->optexpr = $5;
			CurInstr->combine =  6 ;
			$$ =  (( $2 ) << 16)  | $6;
		}
		| '(' reg ')' ',' armconstexpr optpling
		{
			CurInstr->optexpr = $5;
			CurInstr->combine =  6 ;
			$$ =  (( $2 ) << 16)  | $6;
		}
		;


 

pseudo:		LEA	reg ',' constexpr
		{
			 
			 
			 
			 

			 
			 

			 
			CurInstr->opcode = $1->Template |  (( $2 ) << 12)  					|  ((  0xF  ) << 16)  |  (1 << 25) ;


			 
			 
			 
			 

			CurInstr->optexpr = $4;
			CurInstr->combine =  12 ;
		}
		| NOP
		{
			 
			CurInstr->opcode = $1->Template;
		}
		;



 
 
armconstexpr:	'#' constexpr
		{	$$ = $2; }
		| constexpr		 
		;


 
 
 
 
 
 
 
 
 
 
 


instrpatch:
		armconstexpr
		| armpatches
			 
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



 
#line 1032 "/pds/nickc/RTNucleus/cmds/assembler/gasm.ypp"

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 



 
%%		 
 


 
 
 
 
 
 
 
 
 
 

static void yyerror(char *s)
{
	Error(s);
}



 
 
 
 
 
 
 
 
 
 
 
 
 

ParseTreeItem *NewParseTreeItem(int type)
{
	ParseTreeItem *pti = (ParseTreeItem *)malloc(sizeof(ParseTreeItem));

	if (pti ==  0 )
		Fatal("Out of memory whilst building parse tree");

	pti->what = type;
	pti->next =  0 ;

	 
	pti->linenum = StartLine;

	 



	pti->logicalPC = curPC;		 

	 
	LastParseTreeItem->next = pti;
	LastParseTreeItem = pti;

	return pti;
}


 
 
 
 
 
 
 
 
 

ConstList *NewConstItem(int type)
{
	ConstList *cl = (ConstList *)malloc(sizeof(ConstList));

	if (cl ==  0 )
		Fatal("Out of memory whilst building new constant");

	cl->what = type;
	cl->next =  0 ;




	return cl;
}


 
 
 
 
 
 
 
 
 

FloatList *NewFloatConstItem(Dble d)
{
	FloatList *fl = (FloatList *)malloc(sizeof(FloatList));

	if (fl ==  0 )
		Fatal("Out of memory whilst building new float constant");

	fl->next =  0 ;
	fl->value = d;




	return fl;
}


 
 
 
 
 
 
 
 
 

Patch *NewPatch(int type)
{
	Patch *p = (Patch *)malloc(sizeof(Patch));

	if (p ==  0 )
		Fatal("Out of memory whilst defining patch");

	p->what = type;




	return p;
}


 
 
 
 
 
 
 
 
 
 

Expression *NewExpr(Expression *le, int op, Expression *re)
{
	Expression *e = (Expression *)malloc(sizeof(Expression));

	if (e ==  0 )
		Fatal("Out of memory defining expression");

	e->what =  1 ;

	e->type.expr.Operator = op;
	e->type.expr.left  = le;
	e->type.expr.right = re;




	return e;
}


 
 
 
 
 
 
 
 
 

Expression *NewExprNum(int num)
{
	Expression *e = (Expression *)malloc(sizeof(Expression));

	if (e ==  0 )
		Fatal("Out of memory defining expression");

	e->what =  2 ;

	e->type.number = num;




	return e;
}


 
 
 
 
 
 
 
 
 

Expression *NewExprSymbRef(char *name)
{
	Expression *e = (Expression *)malloc(sizeof(Expression));

	if (e ==  0 )
		Fatal("Out of memory defining expression");

	e->what =  4 ;

	e->type.name = name;




	return e;
}


 


 
