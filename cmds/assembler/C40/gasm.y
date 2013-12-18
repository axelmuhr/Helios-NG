#line 1 "/pds/nickc/RTNucleus/cmds/assembler/gasm.ypp"
 
#line 1 "/pds/nickc/RTNucleus/cmds/assembler/warn.h"
 
 
 
 
#line 3 "/pds/nickc/RTNucleus/cmds/assembler/gasm.ypp"

 
























 
 

%{

 






 
#line 1 "/pds/nickc/RTNucleus/cmds/assembler/gasm.h"
 



















#line 1 "/hsrc/include/stdio.h"
 
 
 
 




 





typedef char *__va_list[1];        








 
 



extern int errno;		 

typedef struct _fpos_t
{ unsigned long lo;              
} fpos_t;

typedef struct _FILE
{ unsigned char *_ptr;
  int _icnt;       
  int _ocnt;       
  int _flag;









  int :32,:32,:32,:32,:32,:32,:32;

} FILE;


























 




extern FILE _iob[ 16 ];  






extern int remove(char const * filename);
extern int rename(char const * old_name, char const * new_name);
extern FILE *tmpfile(void);
extern char *tmpnam(char *s);

extern int fclose(FILE *stream);
extern int fflush(FILE *stream);
extern FILE *fopen(char const * filename, char const * mode);
extern FILE *freopen(char const * filename, char const * mode, FILE *stream);
extern void setbuf(FILE *stream, char *buf);
extern int setvbuf(FILE *stream, char *buf, int mode,  unsigned int  size);


extern int printf(const char *format, ...);
extern int fprintf(FILE *stream, const char *format, ...);
extern int sprintf(char *s, const char *format, ...);

extern int scanf(const char *format, ...);
extern int fscanf(FILE *stream, const char *format, ...);
extern int sscanf(const char *s, const char *format, ...);
extern int _scanf(const char *format, ...);
extern int _fscanf(FILE *stream, const char *format, ...);
extern int _sscanf(const char *s, const char *format, ...);



extern int vprintf(const char *format, __va_list arg);
extern int vfprintf(FILE *stream, const char *format, __va_list arg);
extern int vsprintf(char *s, const char *format, __va_list arg);
extern int _vfprintf(FILE *stream, const char *format, __va_list arg);








extern int fgetc(FILE *stream);
extern char *fgets(char *s, int n, FILE *stream);
extern int fputc(int c, FILE *stream);
extern int fputs(char const *s, FILE *stream);
extern int _fillbuf(FILE *stream);

extern int (getc)(FILE *stream);

extern int (getchar)(void);
extern char *gets(char *s);
extern int _flushbuf(int ch, FILE *stream);

extern int (putc)(int c, FILE *stream);

extern int (putchar)(int c);
extern int puts(char const *s);
extern int ungetc(int c, FILE *stream);









extern  unsigned int  fread(void *ptr,  unsigned int  size,  unsigned int  nmemb, FILE *stream);
extern  unsigned int  fwrite(void const * ptr,  unsigned int  size,  unsigned int  count, FILE *stream);

extern int fgetpos(FILE *stream, fpos_t *pos);
extern int fseek(FILE *stream, long int offset, int whence);
extern int fsetpos(FILE *stream, const fpos_t *pos);
extern long int ftell(FILE *stream);
extern void rewind(FILE *stream);

extern void clearerr(FILE *stream);


extern int (feof)(FILE *stream);

extern int (ferror)(FILE *stream);





extern void perror(const char *s);

extern int fileno(FILE *);		 
extern FILE *fdopen(int fd, char *mode); 



 
#line 22 "/pds/nickc/RTNucleus/cmds/assembler/gasm.h"
#line 1 "/hsrc/include/stdlib.h"
 
 
 
 




 






   typedef int wchar_t;            







typedef struct div_t  {      int quot, rem; } div_t;	   
typedef struct ldiv_t { long int quot, rem; } ldiv_t;	   


 






extern const double _huge_val;



    






    










    





    





    








double 			atof( const char * nptr );
int 			atoi( const char * nptr );
long int		atol( const char * nptr );

double 			strtod(  const char * nptr, char ** endptr );
long int 		strtol(  const char * nptr, char ** endptr, int base );
unsigned long int	strtoul( const char * nptr, char ** endptr, int base );

int 			rand( void );
void 			srand( unsigned int seed );
extern int		__rand( void );
extern void		__srand( unsigned int seed );

void *			calloc(  unsigned int  nmemb,  unsigned int  size );
void 			free( void * ptr );
void *			malloc(  unsigned int  size );
void *			realloc( void * ptr,  unsigned int  size );

void 			abort( void );
int  			atexit( void (* func )( void ) );
void 			exit( int status );
char *			getenv( const char * name );
int  			system( const char * string );

void *			bsearch( const void * key, const void * base,  unsigned int  nmemb,  unsigned int  size,
				int (* compar )(const void *, const void *) );
void 			qsort( void * base,  unsigned int  nmemb,  unsigned int  size,
			      int (* compar )(const void *, const void *) );


int 			abs( int j );


div_t 			div( int numer, int denom );
long int 		labs( long int j );
ldiv_t 			ldiv( long int numer, long int denom );

 












    













extern int 		mblen( const char * s,  unsigned int  n );

    
















extern int 		mbtowc( wchar_t * pwc, const char * s,  unsigned int  n );

    














extern int 		wctomb( char * s, wchar_t wchar );

 





    















extern  unsigned int  		mbstowcs( wchar_t * pwcs, const char * s,  unsigned int  n );

    















extern  unsigned int  		wcstombs( char * s, const wchar_t * pwcs,  unsigned int  n );



 
#line 23 "/pds/nickc/RTNucleus/cmds/assembler/gasm.h"
#line 1 "/hsrc/include/string.h"
 
 
 
 








void *memcpy(void *s1, const void *s2,  unsigned int  n);
void *memmove(void *s1, const void *s2,  unsigned int  n);
char *strcpy(char *s1, const char *s2);
char *strncpy(char *s1, const char *s2,  unsigned int  n);

char *strcat(char *s1, const char *s2);
char *strncat(char *s1, const char *s2,  unsigned int  n);

int memcmp(const void *s1, const void *s2,  unsigned int  n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2,  unsigned int  n);


void *memchr(const void *s, int c,  unsigned int  n);
char *strchr(const char *s, int c);
 unsigned int  strcspn(const char *s1, const char *s2);
char *strpbrk(const char *s1, const char *s2);
char *strrchr(const char *s, int c);
 unsigned int  strspn(const char *s1, const char *s2);
char *strstr(const char *s1, const char *s2);
char *strtok(char *s1, const char *s2);

void *memset(void *s, int c,  unsigned int  n);
char *strerror(int errnum);
 unsigned int  strlen(const char *s);

int strcoll(const char *s1, const char *s2);   
 unsigned int  strxfrm(char *s1, const char *s2,  unsigned int  n);   



















 
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



 



#line 1 "/pds/nickc/RTNucleus/cmds/assembler/hdr_C40.h"
 






















#line 1 "/pds/nickc/RTNucleus/cmds/assembler/binary.h"
 






















 


 



 





 









 

















 

































 

































































 

































































































































 

































































































































































































































































 

































































































































































































































































































































































































































































































































 



































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































 
#line 25 "/pds/nickc/RTNucleus/cmds/assembler/hdr_C40.h"


 
 


 
 
 



















typedef struct Mnemonic {
	char		*name;		 
	int		token;		 
	unsigned	diadic;		 
	unsigned	triadic;	 
	unsigned	par_st;		 
} Mnemonic;


 
 
 
 
 
 


typedef struct Instruction {
	int			opcode;		 
	struct Expression	*optexpr;	 
						 
	int			combine;	 
						 
	struct Expression	*optexpr2;	 
	int			combine2;	 
} Instruction;


 
 


 

 

 


 


 


 


 


 


 



 

 

 
 
 



 
 



 
 




 

 
 
		 

		 



 

 
 




 

 
 
 

 
 
 


 
 
 


 



 
 

 

































 




 
 
 



















 
 

 

















   










 














 










 
 
 

 
 




































  

					 

					 

					 

					 

					 






















 

					 




 
 
 





















					 

					 


















 
 
 




					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 

					 











 
 




					 

					 

					 

					 

















 
 
 
 
 




 






 











 
 





 
 

 








 


























 




 
 
 






 




 
 

 

 


 





 
 


 


 


 


 




 

 



 


 










 


 



 

 






 



 




 

 


 
 
 
 
 






 










 
 

 


 
 


 


 
 




 

 
extern	Instruction	*CurInstr;


 


 



extern int32 C40_32ToC40_16(int32);




	extern	int32 IEEE_32ToC40_32(int32);


extern	int32 IEEE_64ToC40_32(Dble);
extern	int32 IEEE_64ToC40_16(Dble);














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

extern void		WriteCodeC40Float(Dble d);


 
 




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


 
 
 

 
#line 1 "/pds/nickc/RTNucleus/cmds/assembler/toks_C40.ypp"
 


















 
 
%{
#line 1 "/pds/nickc/RTNucleus/cmds/assembler/ghof.h"
 





















 





























 







 


























 







 

extern int codesize;


 

void FlushCodeBuffer(void);		 
void GHOFEncode(int n);			 
void ObjWriteByte(char b);		 
void ObjWrite(char *buf, int size);	 




#line 24 "/pds/nickc/RTNucleus/cmds/assembler/toks_C40.ypp"

	 
	Instruction	*CurInstr =  0 ;
%}



 
 


 
 

%token	<mnem>	LDA LDE LDEP LDF LDHI LDI LDM LDP LDPE LDPK STF STI STIK

%token	<mnem>	LBb	 

%token	<mnem>	LBUb 	 

%token	<mnem>	LDFcond
		 
		 
		 

%token	<mnem>	LDIcond
		 
		 
		 

%token	<mnem>	LHw	 
%token	<mnem>	LHUw	 

%token	<mnem>	LWLct	 
%token	<mnem>	LWRct	 


 
 

%token	<mnem>	ABSF ABSI ADDC ADDF ADDI AND ANDN ASH CMPF CMPI FIX FLOAT
%token	<mnem>	FRIEEE LSH MPYF MPYI MPYSHI MPYUHI NEGB NEGF
%token	<mnem>	NEGI NORM NOT OR RCPF RND ROL ROLC ROR RORC RSQRF SUBB
%token	<mnem>	SUBC SUBF SUBI SUBRB SUBRF SUBRI TOIEEE TSTB XOR

%token	<mnem>	MBct	 

%token	<mnem>	MHct	 


 
 

%token	<mnem>	ADDC3 ADDF3 ADDI3 AND3 ANDN3 ASH3 CMPF3 CMPI3 LSH3 MPYF3 MPYI3
%token	<mnem>	MPYSHI3 MPYUHI3 OR3 SUBB3 SUBF3 SUBI3 TSTB3 XOR3


 
 

%token	<mnem>	BR BRD CALL LAJ RPTB RPTBD RPTS 

%token	<mnem>	Bcond
		 
		 

%token	<mnem>	BcondAF
		 
		 
		 

%token	<mnem>	BcondAT
		 
		 
		 

%token	<mnem>	BcondD
		 
		 
		 

%token	<mnem>	CALLcond
		 
		 
		 
		 

%token	<mnem>	DBcond
		 
		 
		 

%token	<mnem>	DBcondD
		 
		 
		 

%token	<mnem>	LAJcond
		 
		 
		 

%token	<mnem>	LATcond
		 
		 
		 

%token	<mnem>	RETIcond
	 	 
		 
		 
		 

%token	<mnem>	RETIcondD
		 
		 
		 
		 

%token	<mnem>	RETScond
	 	 
		 
		 
		 

%token	<mnem>	TRAPcond
		 
		 
		 
		 


 
 

%token	<mnem>	LDFI LDII SIGI STFI STII


 
 

%token	<mnem>	POPF POP PUSH PUSHF 

 
 

%token	<mnem>	IDLE SWI NOP IACK



 
 

 

 










 
 
 

%token	<flt>	C40FLOAT


 
 
 

%token	<num>	PATCHC40DATAMODULE1 PATCHC40DATAMODULE2 PATCHC40DATAMODULE3
%token	<num>	PATCHC40DATAMODULE4 PATCHC40DATAMODULE5
%token	<num>	PATCHC40MASK24ADD PATCHC40MASK16ADD PATCHC40MASK8ADD


 
 


 
 
 

%token	<num>	R0 R1 R2 R3 R4 R5 R6 R7 R8 R9 R10 R11
%token	<num>	AR0 AR1 AR2 AR3 AR4 AR5 AR6 AR7
%token	<num>	DP IR0 IR1 BK SP ST DIE IIE IIF RS RE RC

%token	<num>	IVTP TVTP

%type	<num>	reg Dreg Dreg0_1 Dreg2_3 Dreg0_7 Areg Addr_reg Exp_reg


 
 
 

%token		BARBAR



 
 
 

%type	<num>	indirect prefix postfix displacement
%type	<expr>	direct immediate pcrel
%type	<num>	fp_immediate  
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
				 

				 
				Symbol *s = NewSymb($1,  0 , curPC / 4);




				 
				 
				ParseTreeItem *pti = NewParseTreeItem(LABEL);

				pti->type.symb = s;


				 
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


c40float:	C40FLOAT floatconstlist
		{
			ParseTreeItem *pti = NewParseTreeItem(C40FLOAT);

			pti->type.flist = $2;
			curPC += list_size_count * 4;  
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










 
#line 1 "/pds/nickc/RTNucleus/cmds/assembler/rules_C40.ypp"
#line 1 "/hsrc/include/stdio.h"
 
 
 
 
















































































































































































 
#line 2 "/pds/nickc/RTNucleus/cmds/assembler/rules_C40.ypp"
#line 1 "/hsrc/include/stdlib.h"
 
 
 
 



























































































































































































































 
#line 3 "/pds/nickc/RTNucleus/cmds/assembler/rules_C40.ypp"
 

















 













 
 
 


 

		 
reg:		R0 | R1 | R2 | R3 | R4 | R5 | R6 | R7 | R8 | R9 | R10 | R11
		| AR0 | AR1 | AR2 | AR3 | AR4 | AR5 | AR6 | AR7
		| DP | IR0 | IR1 | BK | SP | ST | DIE | IIE | IIF | RS | RE | RC
		| error
		{	Error("Expecting a register");	}
		;

		 
Dreg:		R0 | R1 | R2 | R3 | R4 | R5 | R6 | R7 | R8 | R9 | R10 | R11
		| error
		{	Error("Expecting a register (R0-R11)");	}
		;

		 
Dreg0_1:	R0 | R1
		| error
		{	Error("Expecting register R0 or R1");	}
		;

		 
Dreg2_3:	R2 | R3
		| error
		{	Error("Expecting register R2 or R3");	}
		;

		 
Dreg0_7:	R0 | R1 | R2 | R3 | R4 | R5 | R6 | R7
		| error
		{	Error("Expecting a register R0-R7");	}
		;

		 
Areg:		AR0 | AR1 | AR2 | AR3 | AR4 | AR5 | AR6 | AR7




		;

		 
Addr_reg:	AR0 | AR1 | AR2 | AR3 | AR4 | AR5 | AR6 | AR7 |
		IR0 | IR1 | DP | BK | SP
		| error
		{	Error("Unknown address register");	}
		;

		 
Exp_reg:	IVTP | TVTP
		| error
		{	Error("Unknown expansion file register");	}
		;


 

		 
direct:		'@' constexpr
		{
			if (CurInstr->combine) {
				 
				CurInstr->combine2 =  5 ;
				$$ = CurInstr->optexpr2 = $2;
			}
			else {
				 
				CurInstr->combine =  5 ;
				$$ = CurInstr->optexpr = $2;
			}
		}
		;


 

 
 
 
 

immediate:	constexpr
		{
			if (CurInstr->combine) {
				 
				CurInstr->combine2 =  1 ;
				$$ = CurInstr->optexpr2 = $1;
			}
			else {
				 
				CurInstr->combine =  1 ;
				$$ = CurInstr->optexpr = $1;
			}
		}
		;


fp_immediate:	imm_fp_constexpr
		{
			$$ = (int) IEEE_64ToC40_16($1);
		}
		;


 

 


















		 
indirect:	'*' Areg
			 
		{	$$ =  ((  0x18  ) << 3)  |  (( $2 ) -  0x08 ) ;	}
		| '*' prefix Areg displacement
		{	$$ =  (( $2 | $4 ) << 3)  |  (( $3 ) -  0x08 ) ;	}
		| '*' Areg postfix
		{	$$ =  (( $3 ) << 3)  |  (( $2 ) -  0x08 ) ;		}
		| '*' error
		{
			Error("format of indirection is incorrect");
		}
		;

 
prefix:		 
		'+'
		{	$$ =  0x0 ;		}
		| '-'
		{	$$ =  0x1 ;		}
		| PLUSPLUS
		{	$$ =  0x2 ;		}
		| MINUSMINUS
		{	$$ =  0x3 ;		}
		;

postfix:	 
		PLUSPLUS '(' IR0 ')' bitreverse
		{	$$ =  0x19 ;		}
		| PLUSPLUS displacement
		{	$$ = $2 |  0x4 ;	}
		| MINUSMINUS displacement
		{	$$ = $2 |  0x5 ;	}
		| PLUSPLUS displacement '%'
		{	$$ = $2 |  0x6 ;	}
		| MINUSMINUS displacement '%'
		{	$$ = $2 |  0x7 ;	}
		;

 
 
 
 
 
 

displacement:	 
		{
			 
			 
			if (CurInstr->combine) {
				 
				CurInstr->optexpr2 = NewExprNum(1);
				CurInstr->combine2 =  4 ;
			}
			else {
				 
				CurInstr->optexpr = NewExprNum(1);
				CurInstr->combine =  4 ;
			}
			$$ = 0;  
		}
		| '(' constexpr ')'
		{
			if (CurInstr->combine) {
				CurInstr->optexpr2 = $2;
				CurInstr->combine2 =  4 ;
			}
			else {
				CurInstr->optexpr = $2;
				CurInstr->combine =  4 ;
			}
			$$ = 0;  
		}
		| '(' IR0 ')'
		{	$$ =  ((  0x1  ) << 3) ;	}
		| '(' IR1 ')'
		{	$$ =  ((  0x2  ) << 3) ;	}
		;

		 
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


 

pcrel:		 
		constexpr
		{
			 
			CurInstr->combine =  17 ;
			$$ = CurInstr->optexpr = $1;
		}
		| '@' constexpr
		{
			 
			CurInstr->combine =  17 ;
			$$ = CurInstr->optexpr = $2;
		}
		;



 
 
 

 
 


 

int_ld_Addr_reg_modes:
		reg ',' Addr_reg
		{	$$ =   (  0x0   << 21)   |  (( $3 ) << 16)  |  ( $1 ) ; }
		| indirect ',' Addr_reg
		{	$$ =   (  0x2   << 21)   |  (( $3 ) << 16)  |  (( $1 ) << 8) ; }
		| direct ',' Addr_reg
		{	$$ =   (  0x1   << 21)   |  (( $3 ) << 16) ; }
		| immediate ',' Addr_reg
		{	$$ =   (  0x3   << 21)   |  (( $3 ) << 16) ; }
		| error
		{	Error("illegal addressing mode for an integer load");	}
		;


 
 

st_addr_modes:
		reg ',' direct
		{	$$ =   (  0x1   << 21)   |  (( $1 ) << 16) ; }
		| reg ',' indirect
		{	$$ =   (  0x2   << 21)   |  (( $1 ) << 16)  |  (( $3 ) << 8) ; }
		| reg ',' error
		{
			Error("illegal addressing mode for store instruction");
		}
		;

 
 


 
int_unary_op_mode:
		reg
			 
		{	$$ =   (  0x0   << 21)   |  (( $1 ) << 16)  |  ( $1 ) ; }
		;


 
fp_unary_op_mode:
		Dreg
			 
		{	$$ =   (  0x0   << 21)   |  (( $1 ) << 16)  |  ( $1 ) ; }
		;


 
int_diadic_modes:
		reg ',' reg
		{	$$ =   (  0x0   << 21)   |  (( $3 ) << 16)  |  ( $1 ) ; }
		| direct ',' reg
		{	$$ =   (  0x1   << 21)   |  (( $3 ) << 16) ; }
		| indirect ',' reg
		{	$$ =   (  0x2   << 21)   |  (( $3 ) << 16)  |  (( $1 ) << 8) ; }
		| immediate ',' reg
		{	$$ =   (  0x3   << 21)   |  (( $3 ) << 16) ; }
		;


 
fp_diadic_modes:
		Dreg ',' Dreg
		{	$$ =   (  0x0   << 21)   |  (( $3 ) << 16)  |  ( $1 ) ; }
		| direct ',' Dreg
		{	$$ =   (  0x1   << 21)   |  (( $3 ) << 16) ; }
		| indirect ',' Dreg
		{	$$ =   (  0x2   << 21)   |  (( $3 ) << 16)  |  (( $1 ) << 8) ; }
		| fp_immediate ',' Dreg	 
		{	$$ =   (  0x3   << 21)   | $1 |  (( $3 ) << 16) ; }
		;


 
float_diadic_modes:
		reg ',' Dreg
		{	$$ =   (  0x0   << 21)   |  (( $3 ) << 16)  |  ( $1 ) ; }
		| direct ',' Dreg
		{	$$ =   (  0x1   << 21)   |  (( $3 ) << 16) ; }
		| indirect ',' Dreg
		{	$$ =   (  0x2   << 21)   |  (( $3 ) << 16)  |  (( $1 ) << 8) ; }
		| immediate ',' Dreg	 
		{	$$ =   (  0x3   << 21)   |  (( $3 ) << 16) ; }
		;


 
int_triadic_modes:
		 

		reg ',' reg ',' reg
		{
			$$ =  ( 0  |  ((  0x0  ) << 21) )  |  ( $1 )  |  (( $3 ) << 8)  |  (( $5 ) << 16) ;
		}
		|  reg ',' indirect ',' reg
		{
			$$ =  ( 0  |  ((  0x1  ) << 21) )  |  ( $1 )  |  (( $3 ) << 8)  |  (( $5 ) << 16) ;
			CurInstr->combine =  8 ;
		}

		 

		| immediate ',' reg ',' reg
		{
			$$ =  ( (1 << 28)  |  ((  0x0  ) << 21) )  |  (( $3 ) << 8)  |  (( $5 ) << 16) ;
			CurInstr->combine =  11 ;
		}

		| immediate ',' indirect ',' reg
		{
			$$ =  ( (1 << 28)  |  ((  0x2  ) << 21) )  |  (( $3 ) << 8)  |  (( $5 ) << 16) ;
			CurInstr->combine =  11 ;
			if (! ((( $3 ) &  0xf8 ) ==  0x0 ) ) {
				Error("Only *+ARn format indirections allowed for type 2 triadic instructions");
			}
			CurInstr->combine2 =  10 ;
		}


		 
		 
		 
		 
		 

		| indirect ',' reg ',' reg
		{
			if ( ((( $1 ) &  0xf8 ) ==  0x0 ) ) {
				$$ =  ( (1 << 28)  |  ((  0x1  ) << 21) )  |  ( $1 )  |  (( $3 ) << 8)  |  (( $5 ) << 16) ;
				CurInstr->combine =  9 ;
			}
			else {
				$$ =  ( 0  |  ((  0x2  ) << 21) )  |  ( $1 )  |  (( $3 ) << 8)  |  (( $5 ) << 16) ;
				CurInstr->combine =  7 ;
			}
		}
		| indirect ',' indirect ',' reg
		{
			if ( ((( $1 ) &  0xf8 ) ==  0x0 )  &&  ((( $3 ) &  0xf8 ) ==  0x0 ) ) {
				$$ =  ( (1 << 28)  |  ((  0x3  ) << 21) )  |  ( $1 )  |  (( $3 ) << 8)  |  (( $5 ) << 16) ;
				CurInstr->combine =  9 ;
				CurInstr->combine2 =  10 ;
			}
			else {
				$$ =  ( 0  |  ((  0x3  ) << 21) )  |  ( $1 )  |  (( $3 ) << 8)  |  (( $5 ) << 16) ;
				CurInstr->combine =  7 ;
				CurInstr->combine2 =  8 ;
			}
		}
		;


































 
fp_triadic_modes:	
		 
		Dreg ',' Dreg ',' Dreg
		{
			$$ =  ( 0  |  ((  0x0  ) << 21) )  |  ( $1 )  |  (( $3 ) << 8)  |  (( $5 ) << 16) ;
		}
		|  Dreg ',' indirect ',' Dreg
		{
			$$ =  ( 0  |  ((  0x1  ) << 21) )  |  ( $1 )  |  (( $3 ) << 8)  |  (( $5 ) << 16) ;
			CurInstr->combine =  8 ;
		}

		 
		 
		 


		 
		 
		 
		 
		 

		| indirect ',' Dreg ',' Dreg
		{
			if ( ((( $1 ) &  0xf8 ) ==  0x0 ) ) {
				$$ =  ( (1 << 28)  |  ((  0x1  ) << 21) )  |  ( $1 )  |  (( $3 ) << 8)  |  (( $5 ) << 16) ;
				CurInstr->combine =  9 ;
			}
			else {
				$$ =  ( 0  |  ((  0x2  ) << 21) )  |  ( $1 )  |  (( $3 ) << 8)  |  (( $5 ) << 16) ;
				CurInstr->combine =  7 ;
			}
		}
		| indirect ',' indirect ',' Dreg
		{
			if ( ((( $1 ) &  0xf8 ) ==  0x0 )  &&  ((( $3 ) &  0xf8 ) ==  0x0 ) ) {
				$$ =  ( (1 << 28)  |  ((  0x3  ) << 21) )  |  ( $1 )  |  (( $3 ) << 8)  |  (( $5 ) << 16) ;
				CurInstr->combine =  9 ;
				CurInstr->combine2 =  10 ;
			}
			else {
				$$ =  ( 0  |  ((  0x3  ) << 21) )  |  ( $1 )  |  (( $3 ) << 8)  |  (( $5 ) << 16) ;
				CurInstr->combine =  7 ;
				CurInstr->combine2 =  8 ;
			}
		}
		;


 
 
 
 

tri_par_sti_mode:
		indirect ',' reg ',' reg BARBAR STI Dreg0_7 ',' indirect
		{
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  > 7) { Error("Must be register R0-R7"); } ;
			$$ =  ( $1 )  |  (( $3 ) << 19)  |  (( $5 ) << 22)  |  (( $8 ) << 16)  				|  (( $10 ) << 8) ;

			CurInstr->combine =  13 ;
			CurInstr->combine2 =  14 ;
		}
		| indirect ',' reg BARBAR STI Dreg0_7 ',' indirect
		{
			 
			 

			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;
			$$ =  ( $1 )  |  (( $3 ) << 19)  |  (( $3 ) << 22)  |  (( $6 ) << 16)  				|  (( $8 ) << 8) ;

			CurInstr->combine =  13 ;
			CurInstr->combine2 =  14 ;
		}
		;


 
dia_par_sti_mode:
		indirect ',' reg BARBAR STI Dreg0_7 ',' indirect
		{
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;
			$$ =  ( $1 )  |  (( $3 ) << 22)  |  (( $6 ) << 16)  |  (( $8 ) << 8) ;
			CurInstr->combine =  13 ;
			CurInstr->combine2 =  14 ;
		}
		;


 
 

shift_tri_par_sti_mode:
		reg ',' indirect ',' reg BARBAR STI Dreg0_7 ',' indirect
		{
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  > 7) { Error("Must be register R0-R7"); } ;
			$$ =  (( $1 ) << 19)  |  ( $3 )  |  (( $5 ) << 22)  |  (( $8 ) << 16)  				|  (( $10 ) << 8) ;

			CurInstr->combine =  13 ;
			CurInstr->combine2 =  14 ;
		}
		;


 
 
 
 

tri_par_stf_mode:
		indirect ',' Dreg ',' Dreg BARBAR STF Dreg0_7 ',' indirect
		{
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  > 7) { Error("Must be register R0-R7"); } ;
			$$ =  ( $1 )  |  (( $3 ) << 19)  |  (( $5 ) << 22)  |  (( $8 ) << 16)  				|  (( $10 ) << 8) ;

			CurInstr->combine =  13 ;
			CurInstr->combine2 =  14 ;
		}
		| indirect ',' Dreg BARBAR STF Dreg0_7 ',' indirect
		{
			 
			 

			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;
			$$ =  ( $1 )  |  (( $3 ) << 19)  |  (( $3 ) << 22)  |  (( $6 ) << 16)  				|  (( $8 ) << 8) ;

			CurInstr->combine =  13 ;
			CurInstr->combine2 =  14 ;
		}
		;


 
dia_par_stf_mode:
		indirect ',' Dreg BARBAR STF Dreg0_7 ',' indirect
		{
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;
			$$ =  ( $1 )  |  (( $3 ) << 22)  |  (( $6 ) << 16)  |  (( $8 ) << 8) ;
			CurInstr->combine =  13 ;
			CurInstr->combine2 =  14 ;
		}
		;


 
 

shift_tri_par_stf_mode:
		Dreg ',' indirect ',' Dreg BARBAR STF Dreg0_7 ',' indirect
		{
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  > 7) { Error("Must be register R0-R7"); } ;
			$$ =  (( $1 ) << 19)  |  ( $3 )  |  (( $5 ) << 22)  |  (( $8 ) << 16)  				|  (( $10 ) << 8) ;

			CurInstr->combine =  13 ;
			CurInstr->combine2 =  14 ;
		}
		;


 

 
 
 
 
 
 
 
 


 

par_mpyi_mode:
					indirect ',' indirect ',' reg
		BARBAR	par_mpyi_op2	Dreg0_7 ',' Dreg0_7 ',' Dreg2_3
		{
			 
			 if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			 
			 
			 
			$$ = $7 |  ( ((  0x0  ) << 24) )  				|  (( $3 ) << 8)  |  ( $1 )  |  (( $5 ) << 23)  				|  (( $8 ) << 16)  |  (( $10 ) << 19)  |  (( $12 - 2 ) << 22) ;



			 
			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			indirect ',' reg ',' reg
		BARBAR par_mpyi_op2	indirect ',' Dreg0_7 ',' Dreg2_3
		{
			 
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 |  ( ((  0x3  ) << 24) )  				|  (( $1 ) << 8)  |  (( $3 ) << 19)  |  (( $5 ) << 23)  				|  ( $8 )  |  (( $10 ) << 16)  |  (( $12 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			reg ',' indirect ',' Dreg0_1
		BARBAR par_mpyi_op2	indirect ',' Dreg0_7 ',' Dreg2_3
		{
			 
			 

			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 |  ( ((  0x3  ) << 24) )  				|  (( $3 ) << 8)  |  (( $1 ) << 19)  |  (( $5 ) << 23)  				|  ( $8 )  |  (( $10 ) << 16)  |  (( $12 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			reg ',' reg ',' Dreg0_1
		BARBAR par_mpyi_op2	indirect ',' indirect ',' Dreg2_3
		{
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $3  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 |  ( ((  0x2  ) << 24) )  				|  (( $3 ) << 19)  |  (( $1 ) << 16)  |  (( $5 ) << 23)  				|  ( $8 )  |  (( $10 ) << 8)  |  (( $12 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			indirect ',' reg ',' reg
		BARBAR par_mpyi_op2	Dreg0_7 ',' indirect ',' Dreg2_3
		{
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 |  ( ((  0x1  ) << 24) )  				|  (( $1 ) << 8)  |  (( $3 ) << 19)  |  (( $5 ) << 23)  				|  (( $8 ) << 16)  |  ( $10 )  |  (( $12 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			reg ',' indirect ',' Dreg0_1
		BARBAR par_mpyi_op2	Dreg0_7 ',' indirect ',' Dreg2_3
		{
			 
			 

			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 |  ( ((  0x1  ) << 24) )  				|  (( $3 ) << 8)  |  (( $1 ) << 19)  |  (( $5 ) << 23)  				|  (( $8 ) << 16)  |  ( $10 )  |  (( $12 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}

		 
		 

		|			indirect ',' reg
		BARBAR par_mpyi_op2	indirect ',' Dreg0_7 ',' Dreg2_3
		{
			 if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 |  ( ((  0x3  ) << 24) )  				|  (( $1 ) << 8)  |  (( $3 ) << 19)  |  (( $3 ) << 23)  				|  ( $6 )  |  (( $8 ) << 16)  |  (( $10 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			reg ',' reg
		BARBAR par_mpyi_op2	indirect ',' indirect ',' Dreg2_3
		{
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 |  ( ((  0x2  ) << 24) )  				|  (( $3 ) << 19)  |  (( $1 ) << 16)  |  (( $3 ) << 23)  				|  ( $6 )  |  (( $8 ) << 8)  |  (( $10 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			indirect ',' reg
		BARBAR par_mpyi_op2	Dreg0_7 ',' indirect ',' Dreg2_3
		{
			 if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 |  ( ((  0x1  ) << 24) )  				|  (( $1 ) << 8)  |  (( $3 ) << 19)  |  (( $3 ) << 23)  				|  (( $6 ) << 16)  |  ( $8 )  |  (( $10 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}

		 
		 
		 
		|			indirect ',' reg
		BARBAR par_mpyi_op2	indirect ',' Dreg2_3
		{
			 if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 |  ( ((  0x3  ) << 24) )  				|  (( $1 ) << 8)  |  (( $3 ) << 19)  |  (( $3 ) << 23)  				|  ( $6 )  |  (( $8 ) << 16)  |  (( $8 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}

		 
		 

		|			indirect ',' indirect ',' reg
		BARBAR	par_mpyi_op2	Dreg0_7 ',' Dreg2_3
		{
			 if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 |  ( ((  0x0  ) << 24) )  				|  (( $3 ) << 8)  |  ( $1 )  |  (( $5 ) << 23)  				|  (( $8 ) << 16)  |  (( $10 ) << 19)  |  (( $10 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			indirect ',' reg ',' reg
		BARBAR par_mpyi_op2	indirect ',' Dreg2_3
		{
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 |  ( ((  0x3  ) << 24) )  				|  (( $1 ) << 8)  |  (( $3 ) << 19)  |  (( $5 ) << 23)  				|  ( $8 )  |  (( $10 ) << 16)  |  (( $10 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			reg ',' indirect ',' Dreg0_1
		BARBAR par_mpyi_op2	indirect ',' Dreg2_3
		{
			 
			 

			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 |  ( ((  0x3  ) << 24) )  				|  (( $1 ) << 19)  |  (( $3 ) << 8)  |  (( $5 ) << 23)  				|  ( $8 )  |  (( $10 ) << 16)  |  (( $10 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		;


 
 

par_mpyi_op2:		SUBI
			{	$$ =   (((unsigned) 0x2  << 30) | ((  0x3  ) << 26))  ; }
			| ADDI
			{	$$ =   (((unsigned) 0x2  << 30) | ((  0x2  ) << 26))  ; }
			| SUBI3
			{	$$ =   (((unsigned) 0x2  << 30) | ((  0x3  ) << 26))  ; }
			| ADDI3
			{	$$ =   (((unsigned) 0x2  << 30) | ((  0x2  ) << 26))  ; }
			;


 

par_mpyf_mode:
					indirect ',' indirect ',' Dreg
		BARBAR	par_mpyf_op2	Dreg0_7 ',' Dreg0_7 ',' Dreg2_3
		{
			 
			 if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			 
			 
			 
			$$ = $7 |  ( ((  0x0  ) << 24) )  				|  (( $3 ) << 8)  |  ( $1 )  |  (( $5 ) << 23)  				|  (( $8 ) << 16)  |  (( $10 ) << 19)  |  (( $12 - 2 ) << 22) ;



			 
			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			indirect ',' Dreg ',' Dreg
		BARBAR par_mpyf_op2	indirect ',' Dreg0_7 ',' Dreg2_3
		{
			 
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 |  ( ((  0x3  ) << 24) )  				|  (( $1 ) << 8)  |  (( $3 ) << 19)  |  (( $5 ) << 23)  				|  ( $8 )  |  (( $10 ) << 16)  |  (( $12 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			Dreg ',' indirect ',' Dreg0_1
		BARBAR par_mpyf_op2	indirect ',' Dreg0_7 ',' Dreg2_3
		{
			 
			 

			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 |  ( ((  0x3  ) << 24) )  				|  (( $3 ) << 8)  |  (( $1 ) << 19)  |  (( $5 ) << 23)  				|  ( $8 )  |  (( $10 ) << 16)  |  (( $12 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			Dreg ',' Dreg ',' Dreg0_1
		BARBAR par_mpyf_op2	indirect ',' indirect ',' Dreg2_3
		{
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $3  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 |  ( ((  0x2  ) << 24) )  				|  (( $3 ) << 19)  |  (( $1 ) << 16)  |  (( $5 ) << 23)  				|  ( $8 )  |  (( $10 ) << 8)  |  (( $12 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			indirect ',' Dreg ',' Dreg
		BARBAR par_mpyf_op2	Dreg0_7 ',' indirect ',' Dreg2_3
		{
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 |  ( ((  0x1  ) << 24) )  				|  (( $1 ) << 8)  |  (( $3 ) << 19)  |  (( $5 ) << 23)  				|  (( $8 ) << 16)  |  ( $10 )  |  (( $12 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			Dreg ',' indirect ',' Dreg0_1
		BARBAR par_mpyf_op2	Dreg0_7 ',' indirect ',' Dreg2_3
		{
			 
			 

			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 |  ( ((  0x1  ) << 24) )  				|  (( $3 ) << 8)  |  (( $1 ) << 19)  |  (( $5 ) << 23)  				|  (( $8 ) << 16)  |  ( $10 )  |  (( $12 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}

		 
		 

		|			indirect ',' Dreg
		BARBAR par_mpyf_op2	indirect ',' Dreg0_7 ',' Dreg2_3
		{
			 if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 |  ( ((  0x3  ) << 24) )  				|  (( $1 ) << 8)  |  (( $3 ) << 19)  |  (( $3 ) << 23)  				|  ( $6 )  |  (( $8 ) << 16)  |  (( $10 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			Dreg ',' Dreg
		BARBAR par_mpyf_op2	indirect ',' indirect ',' Dreg2_3
		{
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 |  ( ((  0x2  ) << 24) )  				|  (( $3 ) << 19)  |  (( $1 ) << 16)  |  (( $3 ) << 23)  				|  ( $6 )  |  (( $8 ) << 8)  |  (( $10 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			indirect ',' Dreg
		BARBAR par_mpyf_op2	Dreg0_7 ',' indirect ',' Dreg2_3
		{
			 if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 |  ( ((  0x1  ) << 24) )  				|  (( $1 ) << 8)  |  (( $3 ) << 19)  |  (( $3 ) << 23)  				|  (( $6 ) << 16)  |  ( $8 )  |  (( $10 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}

		 
		 
		 
		|			indirect ',' Dreg
		BARBAR par_mpyf_op2	indirect ',' Dreg2_3
		{
			 if ( $3  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $5 |  ( ((  0x3  ) << 24) )  				|  (( $1 ) << 8)  |  (( $3 ) << 19)  |  (( $3 ) << 23)  				|  ( $6 )  |  (( $8 ) << 16)  |  (( $8 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}

		 
		 

		|			indirect ',' indirect ',' Dreg
		BARBAR	par_mpyf_op2	Dreg0_7 ',' Dreg2_3
		{
			 if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 |  ( ((  0x0  ) << 24) )  				|  (( $3 ) << 8)  |  ( $1 )  |  (( $5 ) << 23)  				|  (( $8 ) << 16)  |  (( $10 ) << 19)  |  (( $10 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			indirect ',' Dreg ',' Dreg
		BARBAR par_mpyf_op2	indirect ',' Dreg2_3
		{
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  > 1) { Error("Must be register R0 or R1"); } ;

			$$ = $7 |  ( ((  0x3  ) << 24) )  				|  (( $1 ) << 8)  |  (( $3 ) << 19)  |  (( $5 ) << 23)  				|  ( $8 )  |  (( $10 ) << 16)  |  (( $10 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			Dreg ',' indirect ',' Dreg0_1
		BARBAR par_mpyf_op2	indirect ',' Dreg2_3
		{
			 
			 

			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;

			$$ = $7 |  ( ((  0x3  ) << 24) )  				|  (( $1 ) << 19)  |  (( $3 ) << 8)  |  (( $5 ) << 23)  				|  ( $8 )  |  (( $10 ) << 16)  |  (( $10 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		;


 
 

par_mpyf_op2:		SUBF
			{	$$ =   (((unsigned) 0x2  << 30) | ((  0x1  ) << 26))  ; }
			| ADDF
			{	$$ =   (((unsigned) 0x2  << 30) | ((  0x0  ) << 26))  ; }
			| SUBF3
			{	$$ =   (((unsigned) 0x2  << 30) | ((  0x1  ) << 26))  ; }
			| ADDF3
			{	$$ =   (((unsigned) 0x2  << 30) | ((  0x0  ) << 26))  ; }
			;


 
 
 

 
 
 

rev_par_mpyi_mode:
					reg ',' reg ',' Dreg2_3
		BARBAR	mpyi2_3		indirect ',' indirect ',' Dreg0_1
		{
			 
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			 
			 
			 
			$$ =  ( ((  0x0  ) << 24) )  				|  (( $10 ) << 8)  |  ( $8 )  |  (( $12 ) << 23)  				|  (( $1 ) << 16)  |  (( $3 ) << 19)  |  (( $5 - 2 ) << 22) ;



			 
			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			indirect ',' reg ',' reg
		BARBAR mpyi2_3		indirect ',' Dreg0_7 ',' Dreg0_1
		{
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x3  ) << 24) )  				|  (( $8 ) << 8)  |  (( $10 ) << 19)  |  (( $12 ) << 23)  				|  ( $1 )  |  (( $3 ) << 16)  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			indirect ',' reg ',' reg
		BARBAR mpyi2_3		Dreg0_7 ',' indirect ',' Dreg0_1
		{
			 
			 
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x3  ) << 24) )  				|  (( $10 ) << 8)  |  (( $8 ) << 19)  |  (( $12 ) << 23)  				|  ( $1 )  |  (( $3 ) << 16)  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			indirect ',' indirect ',' Dreg2_3
		BARBAR mpyi2_3		Dreg0_7 ',' Dreg0_7 ',' Dreg0_1
		{
			 if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x2  ) << 24) )  				|  (( $10 ) << 19)  |  (( $8 ) << 16)  |  (( $12 ) << 23)  				|  ( $1 )  |  (( $3 ) << 8)  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			reg ',' indirect ',' reg
		BARBAR mpyi2_3		indirect ',' Dreg0_7 ',' Dreg0_1
		{
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x1  ) << 24) )  				|  (( $8 ) << 8)  |  (( $10 ) << 19)  |  (( $12 ) << 23)  				|  (( $1 ) << 16)  |  ( $3 )  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			reg ',' indirect ',' reg
		BARBAR mpyi2_3		Dreg0_7 ',' indirect ',' Dreg0_1
		{
			 
			 
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x1  ) << 24) )  				|  (( $10 ) << 8)  |  (( $8 ) << 19)  |  (( $12 ) << 23)  				|  (( $1 ) << 16)  |  ( $3 )  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}

		 
		 

		|			indirect ',' reg ',' reg
		BARBAR mpyi2_3		indirect ',' Dreg0_1
		{
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x3  ) << 24) )  				|  (( $8 ) << 8)  |  (( $10 ) << 19)  |  (( $10 ) << 23)  				|  ( $1 )  |  (( $3 ) << 16)  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			indirect ',' indirect ',' Dreg2_3
		BARBAR mpyi2_3		Dreg0_7 ',' Dreg0_1
		{
			 if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x2  ) << 24) )  				|  (( $10 ) << 19)  |  (( $8 ) << 16)  |  (( $10 ) << 23)  				|  ( $1 )  |  (( $3 ) << 8)  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			reg ',' indirect ',' reg
		BARBAR mpyi2_3		indirect ',' Dreg0_1
		{
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x1  ) << 24) )  				|  (( $8 ) << 8)  |  (( $10 ) << 19)  |  (( $10 ) << 23)  				|  (( $1 ) << 16)  |  ( $3 )  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}

		 
		 
		 
		|			indirect ',' reg
		BARBAR mpyi2_3		indirect ',' Dreg0_1
		{
			 if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x3  ) << 24) )  				|  (( $6 ) << 8)  |  (( $8 ) << 19)  |  (( $8 ) << 23)  				|  ( $1 )  |  (( $3 ) << 16)  |  (( $3 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}

		 
		 

		|			reg ',' Dreg2_3
		BARBAR	mpyi2_3		indirect ',' indirect ',' Dreg0_1
		{
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x0  ) << 24) )  				|  (( $8 ) << 8)  |  ( $6 )  |  (( $10 ) << 23)  				|  (( $1 ) << 16)  |  (( $3 ) << 19)  |  (( $3 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			indirect ',' reg
		BARBAR mpyi2_3		indirect ',' Dreg0_7 ',' Dreg0_1
		{
			 if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x3  ) << 24) )  				|  (( $6 ) << 8)  |  (( $8 ) << 19)  |  (( $10 ) << 23)  				|  ( $1 )  |  (( $3 ) << 16)  |  (( $3 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			indirect ',' reg
		BARBAR mpyi2_3		Dreg0_7 ',' indirect ',' Dreg0_1
		{
			 
			 

			 if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x3  ) << 24) )  				|  (( $6 ) << 19)  |  (( $8 ) << 8)  |  (( $10 ) << 23)  				|  ( $1 )  |  (( $3 ) << 16)  |  (( $3 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		;

mpyi2_3:	MPYI | MPYI3 ;



 
 
 

rev_par_mpyf_mode:
					Dreg ',' Dreg ',' Dreg2_3
		BARBAR	mpyf2_3		indirect ',' indirect ',' Dreg0_1
		{
			 
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			 
			 
			 
			$$ =  ( ((  0x0  ) << 24) )  				|  (( $10 ) << 8)  |  ( $8 )  |  (( $12 ) << 23)  				|  (( $1 ) << 16)  |  (( $3 ) << 19)  |  (( $5 - 2 ) << 22) ;



			 
			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			indirect ',' Dreg ',' Dreg
		BARBAR mpyf2_3		indirect ',' Dreg0_7 ',' Dreg0_1
		{
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x3  ) << 24) )  				|  (( $8 ) << 8)  |  (( $10 ) << 19)  |  (( $12 ) << 23)  				|  ( $1 )  |  (( $3 ) << 16)  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			indirect ',' Dreg ',' Dreg
		BARBAR mpyf2_3		Dreg0_7 ',' indirect ',' Dreg0_1
		{
			 
			 
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x3  ) << 24) )  				|  (( $10 ) << 8)  |  (( $8 ) << 19)  |  (( $12 ) << 23)  				|  ( $1 )  |  (( $3 ) << 16)  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			indirect ',' indirect ',' Dreg2_3
		BARBAR mpyf2_3		Dreg0_7 ',' Dreg0_7 ',' Dreg0_1
		{
			 if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x2  ) << 24) )  				|  (( $10 ) << 19)  |  (( $8 ) << 16)  |  (( $12 ) << 23)  				|  ( $1 )  |  (( $3 ) << 8)  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			Dreg ',' indirect ',' Dreg
		BARBAR mpyf2_3		indirect ',' Dreg0_7 ',' Dreg0_1
		{
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x1  ) << 24) )  				|  (( $8 ) << 8)  |  (( $10 ) << 19)  |  (( $12 ) << 23)  				|  (( $1 ) << 16)  |  ( $3 )  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			Dreg ',' indirect ',' Dreg
		BARBAR mpyf2_3		Dreg0_7 ',' indirect ',' Dreg0_1
		{
			 
			 
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x1  ) << 24) )  				|  (( $10 ) << 8)  |  (( $8 ) << 19)  |  (( $12 ) << 23)  				|  (( $1 ) << 16)  |  ( $3 )  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}

		 
		 

		|			indirect ',' Dreg ',' Dreg
		BARBAR mpyf2_3		indirect ',' Dreg0_1
		{
			 if ( $3  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x3  ) << 24) )  				|  (( $8 ) << 8)  |  (( $10 ) << 19)  |  (( $10 ) << 23)  				|  ( $1 )  |  (( $3 ) << 16)  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			indirect ',' indirect ',' Dreg2_3
		BARBAR mpyf2_3		Dreg0_7 ',' Dreg0_1
		{
			 if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x2  ) << 24) )  				|  (( $10 ) << 19)  |  (( $8 ) << 16)  |  (( $10 ) << 23)  				|  ( $1 )  |  (( $3 ) << 8)  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			Dreg ',' indirect ',' Dreg
		BARBAR mpyf2_3		indirect ',' Dreg0_1
		{
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $5  < 2 ||  $5  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x1  ) << 24) )  				|  (( $8 ) << 8)  |  (( $10 ) << 19)  |  (( $10 ) << 23)  				|  (( $1 ) << 16)  |  ( $3 )  |  (( $5 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}

		 
		 
		 
		|			indirect ',' Dreg
		BARBAR mpyf2_3		indirect ',' Dreg0_1
		{
			 if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x3  ) << 24) )  				|  (( $6 ) << 8)  |  (( $8 ) << 19)  |  (( $8 ) << 23)  				|  ( $1 )  |  (( $3 ) << 16)  |  (( $3 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}

		 
		 

		|			Dreg ',' Dreg2_3
		BARBAR	mpyf2_3		indirect ',' indirect ',' Dreg0_1
		{
			 if ( $1  > 7) { Error("Must be register R0-R7"); } ;  if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x0  ) << 24) )  				|  (( $8 ) << 8)  |  ( $6 )  |  (( $10 ) << 23)  				|  (( $1 ) << 16)  |  (( $3 ) << 19)  |  (( $3 - 2 ) << 22) ;


			CurInstr->combine =  15 ;
			CurInstr->combine2 =  16 ;
		}
		|			indirect ',' Dreg
		BARBAR mpyf2_3		indirect ',' Dreg0_7 ',' Dreg0_1
		{
			 if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x3  ) << 24) )  				|  (( $6 ) << 8)  |  (( $8 ) << 19)  |  (( $10 ) << 23)  				|  ( $1 )  |  (( $3 ) << 16)  |  (( $3 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		|			indirect ',' Dreg
		BARBAR mpyf2_3		Dreg0_7 ',' indirect ',' Dreg0_1
		{
			 
			 

			 if ( $3  < 2 ||  $3  > 3) { Error("Must be Register R2 or R3"); } ;

			$$ =  ( ((  0x3  ) << 24) )  				|  (( $6 ) << 19)  |  (( $8 ) << 8)  |  (( $10 ) << 23)  				|  ( $1 )  |  (( $3 ) << 16)  |  (( $3 - 2 ) << 22) ;


			CurInstr->combine =  16 ;
			CurInstr->combine2 =  15 ;
		}
		;

mpyf2_3:	MPYF | MPYF3 ;




 
 
 

machine_op:	{
			 
			if ((CurInstr = (Instruction *)malloc(sizeof(Instruction))) ==  0 )
				Fatal("Out of Memory for new instruction");

			 
			CurInstr->opcode = 0;
			CurInstr->optexpr =  0 ;
			CurInstr->combine = 0;
			CurInstr->optexpr2 =  0 ;
			CurInstr->combine2 = 0;
		}
		C40_op	 
		{
			 
			ParseTreeItem *pti = NewParseTreeItem(INSTRUCTION);
			pti->type.instr = CurInstr;

			curPC += sizeof(int);
		}
		;


C40_op:		loads | stores | branches | stackops
		| arith_logic_ops | miscops ;



 
 

loads:		load_ints
		| par_load_int
		| load_fp
		| par_load_fp
		| load_parts
		| load_interlocked

		 
		| LDA		int_ld_Addr_reg_modes
		{	CurInstr->opcode = $1->diadic | $2; }

		 
		| LDHI		immediate ',' reg
		{
			CurInstr->opcode = $1->diadic |   (  0x3   << 21)   |  (( $4 ) << 16) ;
			 
			CurInstr->combine =  2 ;
		}

		 
		 
		| LDP		immediate ',' reg
		{
			if ($4 ==  0x10 ) {
				 
				CurInstr->combine =  3 ;
				 
				 
				 
				CurInstr->opcode = $1->diadic;
			}
			else {
				 
				CurInstr->combine =  3 ;
				 
				CurInstr->opcode = $1->triadic |   (  0x3   << 21)   |  (( $4 ) << 16) ;
			}
		}
		| LDP		immediate
		{
				CurInstr->combine =  3 ;
				CurInstr->opcode = $1->diadic;
		}
		| LDP		'@' immediate ',' reg
		{
			if ($5 ==  0x10 ) {
				CurInstr->combine =  3 ;
				CurInstr->opcode = $1->diadic;
			}
			else {
				CurInstr->combine =  3 ;
				CurInstr->opcode = $1->triadic |   (  0x3   << 21)   |  (( $5 ) << 16) ;
			}
		}
		| LDP		'@' immediate
		{
				CurInstr->combine =  3 ;
				CurInstr->opcode = $1->diadic;
		}

		 
		 
		| LDPK		immediate
		{	CurInstr->opcode = $1->diadic; }

		 
		 
		| LDEP		Exp_reg ',' reg
		{	CurInstr->opcode = $1->diadic |  ( $2 )  |  (( $4 ) << 16) ; }
		| LDPE		reg ',' Exp_reg
		{	CurInstr->opcode = $1->diadic |  ( $2 )  |  (( $4 ) << 16) ; }
		;


 
 

load_ints:	LDIcond		int_diadic_modes
		{ CurInstr->opcode = $1->diadic | $2; }
		;


 
 

load_fp:	fp_loads	fp_diadic_modes
		{ CurInstr->opcode = $1->diadic | $2; }
		;

fp_loads:		LDE | LDM | LDFcond ;


 

par_load_int:
		LDI	reg ',' reg
		{
			CurInstr->opcode = $1->diadic 				|   (  0x0   << 21)   |  (( $4 ) << 16)  |  ( $2 ) ;

		}
		| LDI	direct ',' reg
		{
			CurInstr->opcode = $1->diadic |   (  0x1   << 21)   |  (( $4 ) << 16) ;
		}
		| LDI	indirect ',' reg
		{
			CurInstr->opcode = $1->diadic 				|   (  0x2   << 21)   |  (( $4 ) << 16)  |  (( $2 ) << 8) ;

		}
		| LDI	immediate ',' reg
		{
			CurInstr->opcode = $1->diadic |   (  0x3   << 21)   |  (( $4 ) << 16) ;
		}
		| LDI	indirect ',' reg     BARBAR LDI	indirect ',' reg
		{
			 if ( $4  > 7) { Error("Must be register R0-R7"); } ;

			 
			CurInstr->opcode = $1->triadic 			  |  ( $2 )  |  (( $4 ) << 22)  |  (( $7 ) << 8)  |  (( $9 ) << 19) ;


			CurInstr->combine =  13 ;
			CurInstr->combine2 =  14 ;
		}
		| LDI	indirect ',' reg     BARBAR STI	Dreg0_7 ',' indirect
		{
			 if ( $4  > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = $1->par_st 			    |  ( $2 )  |  (( $4 ) << 22)  |  (( $7 ) << 16)  |  (( $9 ) << 8) ;


			CurInstr->combine =  13 ;
			CurInstr->combine2 =  14 ;
		}
		;


 

par_load_fp:
		LDF	Dreg ',' Dreg
		{
			CurInstr->opcode = $1->diadic 			|   (  0x0   << 21)   |  (( $4 ) << 16)  |  ( $2 ) ;

		}
		| LDF	direct ',' Dreg
		{
			CurInstr->opcode = $1->diadic |   (  0x1   << 21)   |  (( $4 ) << 16) ;
		}
		| LDF	indirect ',' Dreg
		{
			CurInstr->opcode = $1->diadic 				|   (  0x2   << 21)   |  (( $4 ) << 16)  |  (( $2 ) << 8) ;

		}
		| LDF	fp_immediate ',' Dreg
		{
			CurInstr->opcode = $1->diadic |   (  0x3   << 21)   | $2 				|  (( $4 ) << 16) ;

		}
		| LDF	indirect ',' Dreg    BARBAR LDF	indirect ',' Dreg
		{
			 if ( $4  > 7) { Error("Must be register R0-R7"); } ;

			 
			CurInstr->opcode = $1->triadic 			  |  ( $2 )  |  (( $4 ) << 22)  |  (( $7 ) << 8)  |  (( $9 ) << 19) ;


			CurInstr->combine =  13 ;
			CurInstr->combine2 =  14 ;
		}
		| LDF	indirect ',' Dreg    BARBAR STF	Dreg0_7 ',' indirect
		{
			 if ( $4  > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = $1->par_st 			   |  ( $2 )  |  (( $4 ) << 22)  |  (( $7 ) << 16)  |  (( $9 ) << 8) ;


			CurInstr->combine =  13 ;
			CurInstr->combine2 =  14 ;
		}
		;


 
 

load_parts:
		part_loads	reg ',' reg
		{
			CurInstr->opcode = $1->diadic 				|   (  0x0   << 21)   |  (( $4 ) << 16)  |  ( $2 ) ;

		}
		| part_loads	direct ',' reg
		{
			CurInstr->opcode = $1->diadic |   (  0x1   << 21)   |  (( $4 ) << 16) ;
		}
		| part_loads	indirect ',' reg
		{
			CurInstr->opcode = $1->diadic 				|   (  0x2   << 21)   |  (( $4 ) << 16)  |  (( $2 ) << 8) ;

		}
		;

part_loads:		LBb | LBUb | LHw | LHUw | LWLct | LWRct | MBct | MHct ;


 
 

load_interlocked:
		interlocked_loads	direct ',' reg
		{
			CurInstr->opcode = $1->diadic |   (  0x1   << 21)   |  (( $4 ) << 16) ;
		}
		| interlocked_loads	indirect ',' reg
		{
			CurInstr->opcode = $1->diadic 				|   (  0x2   << 21)   |  (( $4 ) << 16)  |  (( $2 ) << 8) ;

		}
		;

interlocked_loads:	LDFI | LDII | SIGI ;



 
 

stores:
		 
		genstores		st_addr_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		 
		| STIK			immediate ',' direct
		{
			 
			CurInstr->opcode = $1->diadic |   (  0x0   << 21)  ;

			 
			CurInstr->combine =  6 ;
		}
		| STIK			immediate ',' indirect
		{
			 
			CurInstr->opcode = $1->diadic |   (  0x3   << 21)   |  (( $4 ) << 8) ;
			CurInstr->combine =  6 ;
		}

		| STF			Dreg ',' direct
		{
			CurInstr->opcode = $1->diadic |   (  0x1   << 21)   |  (( $2 ) << 16) ;
		}
		| STF			Dreg ',' indirect
		{
			CurInstr->opcode = $1->diadic |   (  0x2   << 21)   |  (( $2 ) << 16)  				|  (( $4 ) << 8) ;

		}

		| STI			reg ',' direct
		{
			CurInstr->opcode = $1->diadic |   (  0x1   << 21)   |  (( $2 ) << 16) ;
		}
		| STI			reg ',' indirect
		{
			CurInstr->opcode = $1->diadic |   (  0x2   << 21)   |  (( $2 ) << 16)  				|  (( $4 ) << 8) ;

		}


		 

		 
		 
		 
		 

		 
		| STF			Dreg ',' indirect
		BARBAR STF		Dreg0_7 ',' indirect
		{
			 if ( $2  > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = $1->par_st 				|  (( $2 ) << 22)  |  ( $4 )  				|  (( $7 ) << 16)  |  (( $9 ) << 8) ;



			CurInstr->combine =  13 ;
			CurInstr->combine2 =  14 ;
		}


		 
		 
		| STF			Dreg ',' indirect
		BARBAR LDF		indirect ',' Dreg0_7
		{
			 if ( $2  > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = $6->par_st 				|  ( $7 )  |  (( $9 ) << 22)  				|  (( $2 ) << 16)  |  (( $4 ) << 8) ;



			CurInstr->combine =   14 ;
			CurInstr->combine2 =  13 ;
		}

		 

		 
		 
		| STF			Dreg ',' indirect
		BARBAR tri_swap_par_stf	indirect ',' Dreg0_7 ',' Dreg0_7
		{
			 if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				|  ( $7 )  |  (( $9 ) << 19)  |  (( $11 ) << 22)  				|  (( $2 ) << 16)  |  (( $4 ) << 8) ;



			CurInstr->combine =   14 ;
			CurInstr->combine2 =  13 ;
		}
		 
		 
		| STF			Dreg ',' indirect
		BARBAR tri_swap_par_stf	indirect ',' Dreg0_7
		{
			 if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				|  ( $7 )  |  (( $9 ) << 19)  |  (( $9 ) << 22) 

				|  (( $2 ) << 16)  |  (( $4 ) << 8) ;
			CurInstr->combine =  14 ;
			CurInstr->combine2 =  13 ;
		}
		 
		| STF			Dreg ',' indirect
		BARBAR dia_swap_par_stf	indirect ',' Dreg0_7
		{
			 if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				|  ( $7 )  |  (( $9 ) << 22)  				|  (( $2 ) << 16)  |  (( $4 ) << 8) ;


			CurInstr->combine =  14 ;
			CurInstr->combine2 =  13 ;
		}
		 
		| STF			Dreg ',' indirect
		BARBAR shift_swap_par_stf Dreg0_7 ',' indirect ',' Dreg0_7
		{
			 if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				|  (( $7 ) << 19)  |  ( $9 )  |  (( $11 ) << 22)  				|  (( $2 ) << 16)  |  (( $4 ) << 8) ;



			CurInstr->combine =  14 ;
			CurInstr->combine2 =  13 ;
		}


		 

		| STI			reg ',' indirect
		BARBAR STI		Dreg0_7 ',' indirect
		{
			 if ( $2  > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = $1->par_st 				|  (( $2 ) << 22)  |  ( $4 )  				|  (( $7 ) << 16)  |  (( $9 ) << 8) ;



			CurInstr->combine =  13 ;
			CurInstr->combine2 =  14 ;
		}

		 
		 
		| STI			reg ',' indirect
		BARBAR LDI		indirect ',' Dreg0_7
		{
			 if ( $2  > 7) { Error("Must be register R0-R7"); } ;

			CurInstr->opcode = $6->par_st 				|  ( $7 )  |  (( $9 ) << 22)  				|  (( $2 ) << 16)  |  (( $4 ) << 8) ;



			CurInstr->combine =  14 ;
			CurInstr->combine2 =  13 ;
		}

		 

		 

		 
		| STI			reg ',' indirect
		BARBAR tri_swap_par_sti	indirect ',' Dreg0_7 ',' Dreg0_7
		{
			 if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				|  ( $7 )  |  (( $9 ) << 19)  |  (( $11 ) << 22)  				|  (( $2 ) << 16)  |  (( $4 ) << 8) ;



			CurInstr->combine =   14 ;
			CurInstr->combine2 =  13 ;
		}
		 
		 
		| STI			reg ',' indirect
		BARBAR tri_swap_par_sti	indirect ',' Dreg0_7
		{
			 if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				|  ( $7 )  |  (( $9 ) << 19)  |  (( $9 ) << 22) 

				|  (( $2 ) << 16)  |  (( $4 ) << 8) ;
			CurInstr->combine =  14 ;
			CurInstr->combine2 =  13 ;
		}

		 
		| STI			reg ',' indirect
		BARBAR dia_swap_par_sti	indirect ',' Dreg0_7
		{
			 if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				|  ( $7 )  |  (( $9 ) << 22)  				|  (( $2 ) << 16)  |  (( $4 ) << 8) ;


			CurInstr->combine =  14 ;
			CurInstr->combine2 =  13 ;
		}

		 
		| STI			reg ',' indirect
		BARBAR shift_swap_par_sti Dreg0_7 ',' indirect ',' Dreg0_7
		{
			 if ( $2  > 7) { Error("Must be register R0-R7"); } ;
			CurInstr->opcode = $6->par_st 				|  (( $7 ) << 19)  |  ( $9 )  |  (( $11 ) << 22)  				|  (( $2 ) << 16)  |  (( $4 ) << 8) ;



			CurInstr->combine =  14 ;
			CurInstr->combine2 =  13 ;
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

			 
			CurInstr->combine =  18 ;
		}
		;

bigbranch_ops:	BR | BRD | CALL | LAJ
		;


condbranches:	condbranch	pcrel
		{	CurInstr->opcode = $1->diadic |  (1 << 25) ; }
		| condbranch	reg
		{	CurInstr->opcode = $1->diadic |  0  |  ( $2 ) ; }
		;

condbranch:	Bcond | BcondAF | BcondAT | BcondD | CALLcond | LAJcond
		;


decbranches:	DBcond		Areg ',' reg
		{	
			CurInstr->opcode = $1->diadic 				|  0  |  ((( $2 ) - 8) << 22)  |  ( $4 ) ;

		}
		| DBcond	Areg ',' pcrel
		{
			CurInstr->opcode = $1->diadic 				|  (1 << 25)  |  ((( $2 ) - 8) << 22) ;

		}
		| DBcondD	Areg ',' reg
		{
			CurInstr->opcode = $1->diadic 				|  0  |  ((( $2 ) - 8) << 22)  |  ( $4 ) ;

		}
		| DBcondD	Areg ',' pcrel
		{
			CurInstr->opcode = $1->diadic 				|  (1 << 25)  |  ((( $2 ) - 8) << 22) ;

		}
		;


returns:	ret_ops
		{	CurInstr->opcode = $1->diadic; }
		;


ret_ops:	RETIcond | RETIcondD | RETScond ;


traps:		trap_ops immediate
		{
			CurInstr->opcode = $1->diadic;
			CurInstr->combine =  19 ;
		}
		;

trap_ops:	LATcond | TRAPcond ;


repeats:	rptb_ops	pcrel
		{
			CurInstr->opcode = $1->diadic;

			 
			CurInstr->combine =  18 ;
		}
		| rptb_ops	reg
		{
			 
			 
			 
			CurInstr->opcode = $1->triadic |  ( $2 ) ;
		}

		| RPTS		reg
		{	CurInstr->opcode = $1->diadic |   (  0x0   << 21)   |  ( $2 ) ; }
		| RPTS		direct
		{	CurInstr->opcode = $1->diadic |   (  0x1   << 21)  ; }
		| RPTS		immediate
		{
			CurInstr->opcode = $1->diadic |   (  0x3   << 21)  ;
			 
			CurInstr->combine =  2 ;
		}
		| RPTS		indirect
		{	CurInstr->opcode = $1->diadic |   (  0x2   << 21)   |  (( $2 ) << 8) ; }
		;

rptb_ops:	RPTB | RPTBD ;


 
 

stackops:
		 
		PUSH		reg		 
		{	CurInstr->opcode = $1->diadic |  (( $2 ) << 16) ; }
		| PUSHF		Dreg
		{	CurInstr->opcode = $1->diadic |  (( $2 ) << 16) ; }
		| POP		reg		 
		{	CurInstr->opcode = $1->diadic |  (( $2 ) << 16) ; }
		| POPF		Dreg
		{	CurInstr->opcode = $1->diadic |  (( $2 ) << 16) ; }
		;


 
 

miscops:
		IACK		direct
		{	CurInstr->opcode = $1->diadic |   (  0x1   << 21)  ; }
		| IACK		indirect
		{	CurInstr->opcode = $1->diadic |   (  0x2   << 21)   |  (( $2 ) << 8) ; }

		| IDLE
		{	CurInstr->opcode = $1->diadic; }

		| NOP		indirect
		{	CurInstr->opcode = $1->diadic |   (  0x2   << 21)   |  (( $2 ) << 8) ; }
		| NOP
		{	CurInstr->opcode = $1->diadic; }

		| SWI
		{	CurInstr->opcode = $1->diadic; }
		;



 
 
 


 







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



 
 

 




 

int_unary_only:
		NEGB	int_unary_op_mode
		{	CurInstr->opcode = $1->diadic | $2; }
		| NEGB	int_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		;


 

fp_unary_only:
		fp_unary_only_ops	fp_unary_op_mode
		{	CurInstr->opcode = $1->diadic | $2; }
		| fp_unary_only_ops	fp_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		;

fp_unary_only_ops:	NORM | RND ;


 

int_diadic_only:
		int_diadic_only_ops	int_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		;

int_diadic_only_ops:	SUBC | SUBRB | SUBRI ;



 

fp_diadic_only:
		SUBRF	fp_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		;


 

int_triadic_only:
		int_triadic_only_ops	int_triadic_modes
		{
			CurInstr->opcode = $1->triadic | $2;
			if ($1->token == ANDN3 || $1->token == MPYUHI3) {
				 
				if (CurInstr->combine ==  11 )
					CurInstr->combine =  12 ;
				else if (CurInstr->combine2 ==  11 )
					CurInstr->combine2 =  12 ;
			}
		}
		;

int_triadic_only_ops:	ADDC3 | ANDN3 | MPYSHI3 | MPYUHI3 | SUBB3 ;


 

int_both_2_3:
		int_both_2_3_ops	int_diadic_modes
		{
			CurInstr->opcode = $1->diadic | $2;
			if ($1->token == ANDN || $1->token == MPYUHI) {
				 
				if (CurInstr->combine ==  1 )
					CurInstr->combine =  2 ;
			}
		}
		| int_both_2_3_ops	int_triadic_modes
		{
			CurInstr->opcode = $1->triadic | $2;
			if ($1->token == ANDN || $1->token == MPYUHI) {
				 
				if (CurInstr->combine ==  11 )
					CurInstr->combine =  12 ;
				else if (CurInstr->combine2 ==  11 )
					CurInstr->combine2 =  12 ;
			}
		}
		;

int_both_2_3_ops:	ADDC | ANDN | MPYSHI | MPYUHI | SUBB ;


 

int_par_st_unary:
		int_par_st_unary_ops	int_unary_op_mode
		{
			CurInstr->opcode = $1->diadic | $2;
		}
		| int_par_st_unary_ops	int_diadic_modes
		{
			CurInstr->opcode = $1->diadic | $2;
			if ($1->token == NOT && CurInstr->combine ==  1 ) {
				 
				CurInstr->combine =  2 ;
			}
		}
		| int_par_st_unary_ops	dia_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2;	}
		;

int_par_st_unary_ops:	ABSI | NEGI | NOT;


 

fp_par_st_unary:
		fp_par_st_unary_ops	fp_unary_op_mode
		{	CurInstr->opcode = $1->diadic | $2; }
		| fp_par_st_unary_ops	fp_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		| fp_par_st_unary_ops	dia_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;

fp_par_st_unary_ops:	ABSF | NEGF ;

 

float_par_st_unary:
		FLOAT	fp_unary_op_mode
		{	CurInstr->opcode = $1->diadic | $2; }
		| FLOAT	float_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		| FLOAT	dia_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;

 

unsigned_int_par_st_3:
		int_par_st_3_ops	int_triadic_modes
		{
			CurInstr->opcode = $1->triadic | $2;
			 
			if (CurInstr->combine ==  11 )
				CurInstr->combine =  12 ;
			else if (CurInstr->combine2 ==  11 )
				CurInstr->combine2 =  12 ;
		}
		| int_par_st_3_ops	tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;

int_par_st_3_ops:	AND3 | OR3 | XOR3 ;


 

shift_int_par_st_3:

		shift_par_st_3_ops	int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }





		| shift_par_st_3_ops	shift_tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;

shift_par_st_3_ops:	ASH3 | LSH3 ;


 
 

shift_int_par_st_mpy_3:
		SUBI3		int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| SUBI3		shift_tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| SUBI3		rev_par_mpyi_mode
		{	CurInstr->opcode =   (((unsigned) 0x2  << 30) | ((  0x3  ) << 26))   | $2; }
		;


 
 

int_par_st_mpy_3:
		ADDI3		int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| ADDI3		tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| ADDI3		rev_par_mpyi_mode
		{	CurInstr->opcode =   (((unsigned) 0x2  << 30) | ((  0x2  ) << 26))   | $2; }
		;


 
 

shift_fp_par_st_mpy_3:
		SUBF3		fp_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| SUBF3		shift_tri_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| SUBF3 	rev_par_mpyf_mode
		{	CurInstr->opcode =   (((unsigned) 0x2  << 30) | ((  0x1  ) << 26))   | $2; }
		;


 
 

fp_par_st_mpy_3:
		ADDF3		fp_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| ADDF3		tri_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| ADDF3		rev_par_mpyf_mode
		{	CurInstr->opcode =   (((unsigned) 0x2  << 30) | ((  0x0  ) << 26))   | $2; }
		;


 

unsigned_int_par_st_2_3:
		int_par_st_2_3_ops	int_diadic_modes
		{
			CurInstr->opcode = $1->diadic | $2;
			 
			if (CurInstr->combine ==  1 )
				CurInstr->combine =  2 ;
		}
		| int_par_st_2_3_ops	int_triadic_modes
		{
			CurInstr->opcode = $1->triadic | $2;
			 
			if (CurInstr->combine ==  11 )
				CurInstr->combine =  12 ;
			else if (CurInstr->combine2 ==  11 )
				CurInstr->combine2 =  12 ;
		}
		| int_par_st_2_3_ops	tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;

int_par_st_2_3_ops:	AND | OR | XOR ;


 

shift_int_par_st_2_3:
		shift_par_st_2_3_ops	int_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }

		| shift_par_st_2_3_ops	int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }





		| shift_par_st_2_3_ops	shift_tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;

shift_par_st_2_3_ops:	ASH | LSH ;


 
 

shift_int_par_st_mpy_2_3:
		SUBI		int_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		| SUBI		int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| SUBI		shift_tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| SUBI		rev_par_mpyi_mode
		{	CurInstr->opcode =   (((unsigned) 0x2  << 30) | ((  0x3  ) << 26))   | $2; }
		;


 
 
int_par_st_mpy_2_3:
		ADDI		int_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		| ADDI		int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| ADDI		tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| ADDI		rev_par_mpyi_mode
		{	CurInstr->opcode =   (((unsigned) 0x2  << 30) | ((  0x2  ) << 26))   | $2; }
		;


 

shift_fp_par_st_mpy_2_3:
		SUBF		fp_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		| SUBF		fp_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| SUBF		shift_tri_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| SUBF		rev_par_mpyf_mode
		{	CurInstr->opcode =   (((unsigned) 0x2  << 30) | ((  0x1  ) << 26))   | $2; }
		;


 

fp_par_st_mpy_2_3:
		ADDF		fp_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }
		| ADDF		fp_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| ADDF		tri_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| ADDF		rev_par_mpyf_mode
		{	CurInstr->opcode =   (((unsigned) 0x2  << 30) | ((  0x0  ) << 26))   | $2; }
		;



 
 
 


 
 

mpy:	mpyf_3 | mpyf_2_3 | mpyi_3 | mpyi_2_3 ;


 

 
 
 

 
mpyf_3:
		MPYF3		fp_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| MPYF3		tri_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| MPYF3		par_mpyf_mode
		{	CurInstr->opcode = $2; }
		;

 
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


 

 
 

 
mpyi_3:
		MPYI3		int_triadic_modes
		{	CurInstr->opcode = $1->triadic | $2; }
		| MPYI3		tri_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		| MPYI3		par_mpyi_mode
		{	CurInstr->opcode = $2; }
		;

 
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


 
 
 

rotates:	rotate_ops	reg
		{	CurInstr->opcode = $1->diadic |  (( $2 ) << 16) ; }
		;

rotate_ops:	ROL | ROLC | ROR | RORC	;



 
 
 

reciprocals:	recip_ops	Dreg ',' Dreg
		{
			CurInstr->opcode = $1->diadic 				|   (  0x0   << 21)   |  ( $2 )  |  (( $4 ) << 16) ;

		}
		| recip_ops	direct ',' Dreg
		{
			CurInstr->opcode = $1->diadic 				|   (  0x1   << 21)   |  (( $4 ) << 16) ;

		}
		| recip_ops	indirect ',' Dreg
		{
			CurInstr->opcode = $1->diadic 				|   (  0x2   << 21)   |  (( $2 ) << 8)  |  (( $4 ) << 16) ;

		}
		;

recip_ops:	RCPF | RSQRF ;


 
 
 

cmp:	cmpi | cmpf | cmpi3 | cmpf3 ;


 











 

cmpi:
		 
		cmpi_ops	int_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }

		 
		| cmpi_ops	reg ',' indirect
		{
			CurInstr->opcode = $1->triadic 				|  ( 0  |  ((  0x1  ) << 21) )  |  ( $2 )  |  (( $4 ) << 8) ;

			CurInstr->combine =  8 ;
		}

		 
		| cmpi_ops	immediate ',' indirect
		{
			if (! ((( $4 ) &  0xf8 ) ==  0x0 ) ) {
				Error("Only *+ARn format indirections allowed for \"cmpi3 immediate, indirect\"");
			}
			CurInstr->opcode = $1->triadic 				|  ( (1 << 28)  |  ((  0x2  ) << 21) )  |  (( $4 ) << 8) ;

			if ($1->token == TSTB)
				CurInstr->combine =  12 ;
			else
				CurInstr->combine =  11 ;
			CurInstr->combine2 =  10 ;
		}

		 

		 
		 
		 

		| cmpi_ops	indirect ',' indirect
		{
			if ( ((( $2 ) &  0xf8 ) ==  0x0 )  &&  ((( $4 ) &  0xf8 ) ==  0x0 ) ) {
				CurInstr->opcode = $1->triadic 					|  ( (1 << 28)  |  ((  0x3  ) << 21) )  |  ( $2 )  |  (( $4 ) << 8) ;


				CurInstr->combine =  9 ;
				CurInstr->combine2 =  10 ;
			}
			else {
				CurInstr->opcode = $1->triadic 					|  ( 0  |  ((  0x3  ) << 21) )  |  ( $2 )  |  (( $4 ) << 8) ;

				CurInstr->combine =  7 ;
				CurInstr->combine2 =  8 ;
			}
		}
		;

cmpi_ops:	CMPI | TSTB ;


 

cmpf:
		 
		CMPF	fp_diadic_modes
		{	CurInstr->opcode = $1->diadic | $2; }

		 
		| CMPF	Dreg ',' indirect
		{
			CurInstr->opcode = $1->triadic 				|  ( 0  |  ((  0x1  ) << 21) )  |  ( $2 )  |  (( $4 ) << 8) ;

			CurInstr->combine =  8 ;
		}

		 
		 

		 

		 
		 
		 

		| CMPF	indirect ',' indirect
		{
			if ( ((( $2 ) &  0xf8 ) ==  0x0 )  &&  ((( $4 ) &  0xf8 ) ==  0x0 ) ) {
				CurInstr->opcode = $1->triadic 					|  ( (1 << 28)  |  ((  0x3  ) << 21) )  |  ( $2 )  |  (( $4 ) << 8) ;


				CurInstr->combine =  9 ;
				CurInstr->combine2 =  10 ;
			}
			else {
				CurInstr->opcode = $1->triadic 					|  ( 0  |  ((  0x3  ) << 21) )  |  ( $2 )  |  (( $4 ) << 8) ;

				CurInstr->combine =  7 ;
				CurInstr->combine2 =  8 ;
			}
		}
		;


 

cmpi3:
		 
		cmpi3_ops	reg ',' reg
		{
			CurInstr->opcode = $1->triadic |  ( 0  |  ((  0x0  ) << 21) )  				|  ( $2 )  |  (( $4 ) << 8) ;

		}
		| cmpi3_ops	reg ',' indirect
		{
			CurInstr->opcode = $1->triadic |  ( 0  |  ((  0x1  ) << 21) )  				|  ( $2 )  |  (( $4 ) << 8) ;

			CurInstr->combine =  8 ;
		}
		 
		| cmpi3_ops	immediate ',' reg
		{
			CurInstr->opcode = $1->triadic |  ( (1 << 28)  |  ((  0x0  ) << 21) )  |  (( $4 ) << 8) ;
			if ($1->token == TSTB3)
				CurInstr->combine =  12 ;
			else
				CurInstr->combine =  11 ;
		}

		| cmpi3_ops	immediate ',' indirect
		{
			CurInstr->opcode = $1->triadic |  ( (1 << 28)  |  ((  0x2  ) << 21) )  |  (( $4 ) << 8) ;
			if ($1->token == TSTB3)
				CurInstr->combine =  12 ;
			else
				CurInstr->combine =  11 ;

			if (! ((( $4 ) &  0xf8 ) ==  0x0 ) )
				Error("Only *+ARn format indirections allowed for type 2 triadic instructions");

			CurInstr->combine2 =  10 ;
		}

		 

		 
		 
		 
		 

		 
		| cmpi3_ops	indirect ',' reg
		{
			if ( ((( $2 ) &  0xf8 ) ==  0x0 ) ) {
				CurInstr->opcode = $1->triadic 					|  ( (1 << 28)  |  ((  0x1  ) << 21) )  |  ( $2 )  |  (( $4 ) << 8) ;

				CurInstr->combine =  9 ;
			}
			else {
				CurInstr->opcode = $1->triadic 					|  ( 0  |  ((  0x2  ) << 21) )  |  ( $2 )  |  (( $4 ) << 8) ;

				CurInstr->combine =  7 ;
			}
		}
		| cmpi3_ops	indirect ',' indirect
		{
			if ( ((( $2 ) &  0xf8 ) ==  0x0 )  &&  ((( $4 ) &  0xf8 ) ==  0x0 ) ) {
				CurInstr->opcode = $1->triadic 					|  ( (1 << 28)  |  ((  0x3  ) << 21) )  |  ( $2 )  |  (( $4 ) << 8) ;

				CurInstr->combine =  9 ;
				CurInstr->combine2 =  10 ;
			}
			else {
				CurInstr->opcode = $1->triadic 					|  ( 0  |  ((  0x3  ) << 21) )  |  ( $2 )  |  (( $4 ) << 8) ;

				CurInstr->combine =  7 ;
				CurInstr->combine2 =  8 ;
			}
		}
		;

cmpi3_ops:	CMPI3 | TSTB3;


 

cmpf3:
		 
		CMPF3	Dreg ',' Dreg
		{
			CurInstr->opcode = $1->triadic |  ( 0  |  ((  0x0  ) << 21) )  				|  ( $2 )  |  (( $4 ) << 8) ;

		}
		| CMPF3	Dreg ',' indirect
		{
			CurInstr->opcode = $1->triadic |  ( 0  |  ((  0x1  ) << 21) )  				|  ( $2 )  |  (( $4 ) << 8) ;

			CurInstr->combine =  8 ;
		}

		 
		 

		 

		 
		 
		 
		 

		| CMPF3	indirect ',' Dreg
		{
			if ( ((( $2 ) &  0xf8 ) ==  0x0 ) ) {
				CurInstr->opcode = $1->triadic 					|  ( (1 << 28)  |  ((  0x1  ) << 21) )  |  ( $2 )  |  (( $4 ) << 8) ;

				CurInstr->combine =  9 ;
			}
			else {
				CurInstr->opcode = $1->triadic 					|  ( 0  |  ((  0x2  ) << 21) )  |  ( $2 )  |  (( $4 ) << 8) ;

				CurInstr->combine =  7 ;
			}
		}
		| CMPF3	indirect ',' indirect
		{
			if ( ((( $2 ) &  0xf8 ) ==  0x0 )  &&  ((( $4 ) &  0xf8 ) ==  0x0 ) ) {
				CurInstr->opcode = $1->triadic 					|  ( (1 << 28)  |  ((  0x3  ) << 21) )  |  ( $2 )  |  (( $4 ) << 8) ;

				CurInstr->combine =  9 ;
				CurInstr->combine2 =  10 ;
			}
			else {
				CurInstr->opcode = $1->triadic 					|  ( 0  |  ((  0x3  ) << 21) )  |  ( $2 )  |  (( $4 ) << 8) ;

				CurInstr->combine =  7 ;
				CurInstr->combine2 =  8 ;
			}
		}
		;



 
 
 

fix:
		FIX			Dreg
		{
			CurInstr->opcode = $1->diadic |   (  0x0   << 21)   				|  (( $2 ) << 16)  |  ( $2 ) ;

		}
		| FIX			Dreg ',' reg
		{
			CurInstr->opcode = $1->diadic |   (  0x0   << 21)   				|  (( $4 ) << 16)  |  ( $2 ) ;

		}
		| FIX			direct ',' reg
		{
			CurInstr->opcode = $1->diadic |   (  0x1   << 21)   				|  (( $4 ) << 16) ;

		}
		| FIX			indirect ',' reg
		{
			CurInstr->opcode = $1->diadic |   (  0x2   << 21)   				|  (( $4 ) << 16)  |  (( $2 ) << 8) ;

		}
		| FIX			fp_immediate ',' reg
		{
			CurInstr->opcode = $1->diadic |   (  0x3   << 21)   | $2 				|  (( $4 ) << 16) ;

		}

		 
		| FIX			dia_par_sti_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;


 
 
 

toieee:
		TOIEEE			Dreg
		{
			CurInstr->opcode = $1->diadic |   (  0x0   << 21)   				|  (( $2 ) << 16)  |  ( $2 ) ;

		}
		| TOIEEE		Dreg ',' Dreg
		{
			CurInstr->opcode = $1->diadic |   (  0x0   << 21)   				|  (( $4 ) << 16)  |  ( $2 ) ;

		}
		| TOIEEE		direct ',' Dreg
		{
			CurInstr->opcode = $1->diadic |   (  0x1   << 21)   				|  (( $4 ) << 16) ;

		}
		| TOIEEE		indirect ',' Dreg
		{
			CurInstr->opcode = $1->diadic |   (  0x2   << 21)   				|  (( $4 ) << 16)  |  (( $2 ) << 8) ;

		}

		 
		| TOIEEE		dia_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;


 
 
 

frieee:
		FRIEEE			direct ',' Dreg
		{
			CurInstr->opcode = $1->diadic |   (  0x1   << 21)   				|  (( $4 ) << 16) ;

		}
		| FRIEEE		indirect ',' Dreg
		{
			CurInstr->opcode = $1->diadic |   (  0x2   << 21)   				|  (( $4 ) << 16)  |  (( $2 ) << 8) ;

		}

		 
		| FRIEEE		dia_par_stf_mode
		{	CurInstr->opcode = $1->par_st | $2; }
		;



 
 
 
 
 
 
 
 
 
 
 

instrpatch:	constexpr
		| c40patches
		{
			 
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

	 

	pti->logicalPC = curPC / 4;	 



	 
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


 


 
