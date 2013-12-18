%{
#include "iasm.h"
Line nextline;         /* The result ! */
Expression *binexpression(ETYPE t, Expression *e1, Expression *e2);
Expression *NullExpression(void);
TextSeg *NullTextSeg(void);
void yyerror(char *s);
%}

%union {
      Expression  *instr;
      Symbol      *symbol;
      Expression  *expression;
      int32        ival;
      char        *text;
      TextSeg     *textseg;
}

%token <textseg>   MACROLINE MACROARG
%token <symbol>    LABEL SYMBOL MACRO IREGISTER FREGISTER CREGISTER AT
%token <instr>     INSTRUCTION
%token <ival>      NUMBER
%token <text>      STRING
%token             COMMA RPAREN LPAREN AUTOINC HASH NEWLINE MODNUM
%token             AUTOINC HASH GET MACROEND MACROSTART IMAGESIZE
%token             PLUS MINUS TIMES DIVIDE LSHIFT RSHIFT AND XOR OR
%token             EQ NE GT LT GE LE NOT DOT
%token             JUNK

%type <expression> arg args iexpression regorexpr fregister iregister
%type <expression> optplusexpr
%type <ival> optautoinc
%type <textseg> macrolines macroargs macroarg

%left OR
%left XOR
%left AND
%left EQ NE
%left GT LT GE LE
%left LSHIFT RSHIFT
%left PLUS MINUS
%left TIMES DIVIDE MOD 
%left NOT

%start lines

%%

lines : line
      ;
      | lines line
      ;

line :  optlabel restofline
      {  dot = pcloc; }

optlabel : LABEL
      { nextline.label = $1; }
      |
      { nextline.label = NULL; }

restofline : optins NEWLINE
      { assemble(&nextline); }
      | MACROSTART NEWLINE { outlistline(); } macrolines MACROEND NEWLINE
      {  if( nextline.label == NULL )
            error("Macro definition needs macro name");
         else
            if (pass==1) resolvesym(nextline.label,(int32)$4, S_MACRO);
         outlistline();
      }
      | MACRO macroargs NEWLINE
      {  if( nextline.label ) setlabel(nextline.label);
         startmacro($1,$2);
         outlistline();
      }

macrolines : MACROLINE NEWLINE
            {  $$ = $1;
               $$->cdr = NULL;
               outlistline();
            }
            | macrolines MACROLINE NEWLINE
            {  TextSeg **ml = &($1->cdr);
               while( *ml != NULL ) ml = &((*ml)->cdr);
               *ml = $2;
               $2->cdr = NULL;
               outlistline();
            }
            |
            {
               $$ = NULL;
            }

macroargs : macroarg
            {  $$ = $1;
            }
           | macroargs COMMA  macroarg
            {  TextSeg **ma;
               if( $1 == NULL ) $1 = NullTextSeg();
               if( $3 == NULL ) $3 = NullTextSeg();
               ma = &($1->cdr);
               while( *ma != NULL ) ma = &((*ma)->cdr);
               *ma = $3;
               $3->cdr = NULL;
               $$ = $1;
            }

macroarg : MACROARG
          { $$ = $1; }
          |
          { $$ = NULL; }

optins : INSTRUCTION optargs
      { nextline.instr = $1; }
      |
      { nextline.instr = NULL; }

optargs : args
      { nextline.args = $1; }

args :  arg
      {  $$ = $1; }

      | args COMMA arg
      {  Expression *e;

         if( $1 == NULL ) $1 = NullExpression();
         if( $3 == NULL ) $3 = NullExpression();
         e = $1;
         while( e->cdr != NULL ) e = e->cdr;
         e->cdr = $3;
         $$ = $1;
      }

arg : regorexpr
     { $$ = $1; }

     | fregister
      {  $$ = $1;
      }

     | CREGISTER
      {
         $$ = newexpression();
         $$->exprtype = E_CREGISTER;
         $$->e1.symbol = $1;
         $$->cdr = NULL;
      }

     | regorexpr LPAREN iregister RPAREN optautoinc
      {  Expression *e1;
         e1 = $$ = newexpression();
         e1->exprtype   = ($5)? E_REGOFFSETINC: E_REGOFFSET;
         e1->e1.expr    = $1;
         e1->cdr        = $3;
      }
      | STRING
      {  $$ = newexpression();
         $$->exprtype = E_STRING;
         $$->e1.text  = $1;
         $$->cdr      = NULL;
      }
      | IMAGESIZE
      {  $$ = newexpression();
         $$->exprtype = E_IMAGESIZE;
         $$->cdr      = NULL;
      }
      |
      {  $$ = NULL; }

optautoinc : AUTOINC
      { $$ = 1; }
      |
      { $$ = 0; }

regorexpr :  iregister
      {  $$ = $1;
      }
     | iexpression
      { $$ = $1;
        $$->cdr = NULL;
      }

fregister : FREGISTER optplusexpr
      {  $$ = newexpression();
         $$->exprtype = E_FREGISTER;
         $$->e1.symbol = $1;
         $$->e2.expr   = $2;
         $$->cdr = NULL;
      }

iregister : IREGISTER optplusexpr
      {  $$ = newexpression();
         $$->exprtype = E_IREGISTER;
         $$->e1.symbol = $1;
         $$->e2.expr   = $2;
         $$->cdr = NULL;
      }

optplusexpr : PLUS iexpression
      {  $$ = $2;
      }
       |
      {  $$ = NULL; }

iexpression :  iexpression PLUS iexpression
             {  $$ = binexpression(E_PLUS,$1,$3);
             }

             | iexpression MINUS iexpression
             {  $$ = binexpression(E_MINUS,$1,$3);
             }

             | iexpression TIMES iexpression
             {  $$ = binexpression(E_TIMES,$1,$3);
             }

             | iexpression DIVIDE iexpression
             {  $$ = binexpression(E_DIVIDE,$1,$3);
             }

             | iexpression MOD iexpression
             {  $$ = binexpression(E_MOD, $1, $3);
             }

             | iexpression LSHIFT iexpression
             {  $$ = binexpression(E_LSHIFT, $1, $3);
             }

             | iexpression RSHIFT iexpression
             {  $$ = binexpression(E_RSHIFT,$1,$3);
             }

             | iexpression AND iexpression
             {  $$ = binexpression(E_AND,$1,$3);
             }

             | iexpression XOR iexpression
             {  $$ = binexpression(E_XOR,$1,$3);
             }

             | iexpression OR iexpression
             {  $$ = binexpression(E_OR,$1,$3);
             }

             | iexpression EQ iexpression
             {  $$ = binexpression(E_EQ,$1,$3);
             }
             | iexpression NE iexpression
             {  $$ = binexpression(E_NE,$1,$3);
             }
             | iexpression GT iexpression
             {  $$ = binexpression(E_GT,$1,$3);
             }
             | iexpression LT iexpression
             {  $$ = binexpression(E_LT,$1,$3);
             }
             | iexpression LE iexpression
             {  $$ = binexpression(E_LE,$1,$3);
             }
             | iexpression GE iexpression
             {  $$ = binexpression(E_GE,$1,$3);
             }

             | MINUS iexpression %prec TIMES
             {  $$ = binexpression(E_UMINUS,$2,NULL);
             }

             | NOT iexpression
             {  $$ = binexpression(E_NOT,$2,NULL);
             }

             | SYMBOL
             {  $$ = newexpression();
                $$->exprtype = E_SYMBOL;
                $$->e1.symbol    = $1;
             }
             | MODNUM
             {  $$ = newexpression();
                $$->exprtype = E_MODNUM;
             }
             | AT
             {  $$ = newexpression();
                $$->exprtype = E_AT;
                $$->e1.symbol    = $1;
             }
             | DOT
             {  $$ = newexpression();
                $$->exprtype = E_DOT;
             }
             | NUMBER
             {  $$ = newexpression();
                $$->exprtype = E_NUMBER;
                $$->e1.value    = $1;
             }
             | LPAREN iexpression RPAREN
             {  $$ = $2; }


%%
void yyerror(char *s)
{
   warn("Parse error at line %d: %s",currentfile->lineno,s);
}

Expression *binexpression(ETYPE t, Expression *e1, Expression *e2)
{
   Expression *r = newexpression();
   r->exprtype   = t;
   r->e1.expr    = e1;
   r->e2.expr    = e2;
   return r;
}

int performget(char *s, bool rootfile)
{
   FILE *f;
   char fullname[MAXNAME];
   char *ip = includepaths;

   if( ip && !rootfile)
   {
      while(1)
      {  char *fp = fullname;
         char ch;
         while( (ch = *ip++) != '\0')
         {
            if( ch == ',' ) break;
            *fp++ = ch;
         }
         strcpy(fp,s);
         if( (f = fopen(fullname,"r")) != NULL ) break;
         if( ch == '\0' ) break;
      }
   }
   else
      f = fopen(s, "r");

   if( f != NULL )
   {  AFILE *nf = nextcurrentfile = currentfile+1;
      filenum++;
      nf->io.file.stream = f;
      nf->io.file.buf    = aalloc(LINE_BUF_SIZE);
      setvbuf( f, nf->io.file.buf, _IOLBF, LINE_BUF_SIZE );
      nf->lineno = 1;
      nf->flags  = 0;
      nf->name   = s;
      return 1;
   }
   else
   {
      error("File \"%s\" not found\n",s);
      return 0;
   }
}

void startmacro(Symbol *s, TextSeg *args)
{
   AFILE *nf = nextcurrentfile = currentfile+1;
   Macro *m;
   filenum++;
   nf->io.macro = m = aalloc(sizeof(Macro));
   m->line      = s->symv.macrotext;
   m->tptr      = m->line->text;
   m->argtext   = NULL;
   m->args      = args;

   nf->lineno = 1;
   nf->flags  = ff_macro;
   nf->name   = s->name;
}

TextSeg *NullTextSeg(void)
{  TextSeg *r = (TextSeg *)aalloc(sizeof(TextSeg));
   r->cdr = NULL;
   r->text[0] = '\0';
   return r;
}

Expression *NullExpression(void)
{  Expression *r = newexpression();
   r->exprtype = E_NULL;
   r->cdr = NULL;
   return r;
}
