%{
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "iasm.h"
#ifdef PCNAMES
#include "iasm_tab.h"
#else
#include "iasm.tab.h"
#endif
extern YYSTYPE yylval;
bool comment_at_start = 0;

AFILE *currentfile;
AFILE *nextcurrentfile;
AFILE  filestack[MAXGETFILES];
int    filenum;
int   skipping_level = 0;

int mygets( char *buf, int maxsize );
Symbol *macrolabel(char *s, int l);
Symbol *locallabel(char *text, int l);
Symbol *label(char *text, int l);
TextSeg *macroarg(char *s, int l, bool comment);
Symbol *lastlabel;

#undef YY_INPUT
#define YY_INPUT(buf, result, maxsize ) \
         result = mygets( buf, maxsize )
#undef yywrap
int yywrap(void);

%}

WS        [ \t]
D         [0-9]
O         [0-7]
X         [0-9a-fA-F]
SYMSTART  [\.a-zA-Z_]
SYMCHAR   [a-zA-Z_0-9]
NL        [\n]
MACROARGCHAR [\.a-zA-Z_0-9"|&+*/()%<>^ \$\t-]

%x COMMENT INSTR MACROARGS MACRODEF SKIPPING

%%

^{WS}*"//"    { comment_at_start = 1;
                BEGIN COMMENT;
              }

{WS}*"//"     BEGIN COMMENT;

<COMMENT>. ;

<COMMENT>{NL} { BEGIN 0;
                if( comment_at_start )
                { outlistline();
                  comment_at_start = 0;   /* Go for next line */
                }
                else return NEWLINE;
              }

0[xX]{X}+ { yylval.ival = (int32)strtoul(yytext+2,NULL,16);
            return NUMBER;
          }

0{O}*     { yylval.ival  = (int32)strtoul(yytext,NULL,8);
            return NUMBER;
          }

{D}+      { yylval.ival  = strtol(yytext, NULL, 10);
            return NUMBER;
          }

\".+\"   {  yylval.text = aalloc(yyleng-1);
            memcpy(yylval.text, yytext+1, yyleng-2);
            yylval.text[yyleng-2] = '\0';
            return STRING;
         }

macro    {  BEGIN MACRODEF;
            return MACROSTART;
         }

modnum   { return MODNUM; }
imagesize { return IMAGESIZE; }

^{WS}*{D}+\$@: {
            yylval.symbol = macrolabel(yytext,yyleng);
            return LABEL;
          }

^{WS}*{D}+\$: {
            yylval.symbol = locallabel(yytext,yyleng);
            return LABEL;
          }

^{WS}*{SYMSTART}{SYMCHAR}*(:?):  {
            yylval.symbol = label(yytext,yyleng);
            return LABEL;
          }

{D}+\$@   { yylval.symbol = macrolabel(yytext,yyleng);
            return SYMBOL;
          }

{D}+\$    { yylval.symbol = locallabel(yytext,yyleng);
            return SYMBOL;
          }

"."       return DOT;		/* Must be before {SYMSTART} */

(@|(d\.))?{SYMSTART}{SYMCHAR}* {
            bool dim;
            Symbol *sym = NULL;
            bool at;

/* '@' and 'd.' are mutually exclusive */

            if( yytext[0] == '@' )
               at = 1, yytext++,yyleng--;
            else
               at = 0;

            if( yytext[0] == 'd' && yytext[1]== '.' )
            {  sym = lookup(yytext+2, yyleng-2, 0, S_INSTR );
               if( sym ) dim = 1;
            }
            else
               dim=0;

            if( sym == 0 )
               sym = lookup(yytext,yyleng,1,S_NULL);

            switch( sym->symtype )
            {
            case S_INSTR:
              {
                yylval.instr = newexpression();
                yylval.instr->exprtype = E_INSTR;
                yylval.instr->e1.symbol = sym;
                yylval.instr->e2.insmodifiers = (dim)? I_DIM:0;
                dim = 0;
                BEGIN INSTR;
                break;
              }
            case S_IREGISTER:
              {
                yylval.symbol = sym;
                if( at ) return JUNK;
                return IREGISTER;
              }
            case S_FREGISTER:
              {
                yylval.symbol = sym;
                if( at ) return JUNK;
                return FREGISTER;
              }
            case S_CREGISTER:
              {
                yylval.symbol = sym;
                if( at ) return JUNK;
                return CREGISTER;
              }
            case S_MACRO:
              {
                yylval.symbol = sym;
                if( at ) return JUNK;
                BEGIN MACROARGS;
                return MACRO;
              }
            case S_EQU: case S_SET:
            case S_LABEL: case S_NULL:
            case S_DATA:
              {
                yylval.symbol = sym;
                if( at )
                {  if( sym->symtype == S_NULL )
                   {  sym->symtype = S_DATA;
                      return AT;
                   }
                   else
                   {  if( sym->symtype == S_DATA ) return AT;
                      else
                         return JUNK;
                   }
                }
                return SYMBOL;
              }

            }
          }

<INSTR>\.c  { char *sname = yylval.instr->e1.symbol->name;
              char *newsym = NULL;
              BEGIN 0;
/* This is a horrid kludge ! */
              if( sname[0] == 'l' && sname[1] == 'd' )
                  newsym = "ld.c";
              if( sname[0] == 's' && sname[1] == 't' )
                  newsym = "st.c";
              if( newsym )
                  yylval.instr->e1.symbol = lookup(newsym,4,0,S_INSTR);
              yylval.instr->e2.insmodifiers = I_CM; return INSTRUCTION;
            }
<INSTR>\.b  BEGIN 0;yylval.instr->e2.insmodifiers |= I_BM; return INSTRUCTION;
<INSTR>\.s  BEGIN 0;yylval.instr->e2.insmodifiers |= I_SM; return INSTRUCTION;
<INSTR>\.l  BEGIN 0;yylval.instr->e2.insmodifiers |= I_LM; return INSTRUCTION;
<INSTR>\.d  BEGIN 0;yylval.instr->e2.insmodifiers |= I_DM; return INSTRUCTION;
<INSTR>\.q  BEGIN 0;yylval.instr->e2.insmodifiers |= I_QM; return INSTRUCTION;
<INSTR>\.ss BEGIN 0;yylval.instr->e2.insmodifiers |= I_SSM; return INSTRUCTION;
<INSTR>\.sd BEGIN 0;yylval.instr->e2.insmodifiers |= I_SDM; return INSTRUCTION;
<INSTR>\.dd BEGIN 0;yylval.instr->e2.insmodifiers |= I_DDM; return INSTRUCTION;
<INSTR>\.ds BEGIN 0;yylval.instr->e2.insmodifiers |= I_DSM; return INSTRUCTION;
<INSTR>\.t  BEGIN 0;yylval.instr->e2.insmodifiers |= I_TM;return INSTRUCTION;
<INSTR>({WS}|{NL}) BEGIN 0; yyless(yyleng-1); return INSTRUCTION;
<INSTR>.    BEGIN 0;yylval.instr->e2.insmodifiers |= I_BADM; return INSTRUCTION;

<MACROARGS>{MACROARGCHAR}+"//".* {
            yylval.textseg = macroarg(yytext,yyleng,1);
            return MACROARG;
         }
<MACROARGS>{MACROARGCHAR}+ {
            yylval.textseg = macroarg(yytext,yyleng,0);
            return MACROARG;
         }

<MACROARGS>,     return COMMA;
<MACROARGS>.     return JUNK;
<MACROARGS>\n  { BEGIN 0;
                 return NEWLINE;
               }

<MACRODEF>endm BEGIN 0; return MACROEND; 
<MACRODEF>.*   { yylval.textseg = aalloc(sizeof(TextSeg)+yyleng);
                 memcpy(yylval.textseg->text, yytext, yyleng);
                 yylval.textseg->text[yyleng] = '\0';
                 yylval.textseg->cdr = NULL;
                 return MACROLINE;
               }

<MACRODEF>\n  return NEWLINE;

<SKIPPING>"//".+ ;
<SKIPPING>\n   { listing_flags |= LF_NOCODE;
                 return NEWLINE;
               }
<SKIPPING>endc|if|ifd|ifnd|else {
                listing_flags |= LF_NOCODE;
                yylval.instr = newexpression();
                yylval.instr->exprtype = E_INSTR;
                yylval.instr->e1.symbol = lookup(yytext,yyleng,0,S_INSTR);
                yylval.instr->e2.insmodifiers = 0;
                return INSTRUCTION;
               }
<SKIPPING>.    ;

{WS}+ ;

,         return COMMA;
\(        return LPAREN;
\)        return RPAREN;
#         return HASH;
"++"      return AUTOINC;
"+"       return PLUS;
"-"       return MINUS;
"*"       return TIMES;
"/"       return DIVIDE;
"%"       return MOD;
"<<"      return LSHIFT;
">>"      return RSHIFT;
"&"       return AND;
"^"       return XOR;
"|"       return OR;
">"       return GT;
"<"       return LT;
">="      return GE;
"<="      return LE;
"=="      return EQ;
"!="      return NE;
"~"       return NOT;

\n        return NEWLINE;

.         return JUNK;

%%

int yywrap(void)
{
   if( currentfile->flags & ff_macro )
      afree(currentfile->io.macro);
   else
   {
      fclose(currentfile->io.file.stream);
      afree(currentfile->io.file.buf);
   }
   currentfile--, filenum--;
   if( filenum == 0 )
   {  yy_init = 1;      /* Re-initialise yylex on next token request */
      return 1;
   }
   else
      return 0;
}

int mygets( char *buf, int maxsize)
{
   int r;

   if( nextcurrentfile != NULL )
   {  currentfile = nextcurrentfile;
      nextcurrentfile = NULL;
   }

   if( currentfile->flags & ff_macro )
      r = arg_subs(buf, currentfile->io.macro, maxsize );
   else
   {
     if( fgets(buf, maxsize, currentfile->io.file.stream) != 0 )
     {
	r = strlen(buf);
     }
     else
	r = 0;
   }

   if( r == 0 ) return 0;

   if( currentfile->flags & ff_nlfound )
   {  currentfile->lineno++;
      currentfile->flags &= ~ff_nlfound;
   }

   if( buf[r-1] == '\n' )
      currentfile->flags |= ff_nlfound;
   addtosrcline(buf,r);
   return r;
}

int arg_subs( char *buf, Macro *macro, int maxsize )
{  char *mtext;
   int r=0;
   char ch;
   bool argcont;     /* argument continuation flag */

   if( macro->argtext )
   {  mtext = macro->argtext;
      macro->argtext = NULL;
      argcont = 1;
   }
   else
   {
      mtext = macro->tptr;
      argcont = 0;
   }

   if( debugflags & DEBUG_MACRO )
   {  int i=1;
      TextSeg *arg = macro->args;
      for( ; arg; arg = arg->cdr,i++ )
         fprintf(stderr,"Macro arg %d = \"%s\"\n",i,arg->text);
   }

   if( mtext == NULL ) return 0;

   while( (ch = *mtext) != '\0' && r < maxsize-1 )
   {
      if( ch == '$' && !argcont )
      {  char *substr = NULL;

         switch ( *(++mtext) )
         {
         case '$':
            *buf++ = '$';
            mtext++, r++;
            break;

         case '@':
            *buf++ = '$';
            r++;        /* Don't use @ yet - it may not fit */
            break;

         default:
            {  char *np;
               int argno,i;
               TextSeg *arg;
   
               arg = macro->args;
               i = argno = strtol(mtext, &np, 10);
      
               mtext = np;    /* Should check valid number */
      
               while(--i && (arg != NULL) )
                  arg = arg->cdr;
      
               if( arg == NULL )
                  error("Can't find arg %d", argno);
               else
                  substr = arg->text;

               break;
            }
         }
         if( substr )
         {  int copysize;
            int substrl = strlen(substr);

            if( substrl >= maxsize-r )
            {
/* The next line will exit the loop                      */
               copysize       = maxsize-r-1;
               macro->argtext = substr + copysize;
            }
            else
               copysize = substrl;

            memcpy(buf, substr, copysize);
            buf += copysize;
            r += copysize;
         }
      }
      else
         *buf++ = *mtext++, r++;
   }

   if( *mtext != '\0' )
   {
      if( argcont )
         macro->argtext = mtext;    /* Rest of argument string */
      else
         macro->tptr = mtext;       /* Rest of macro string */
   }
   else
   {
      if( !argcont )
      {  if( macro->argtext == 0 )
         {
            *buf = '\n'; r++;
            if( (macro->line = macro->line->cdr) != NULL )
               macro->tptr = macro->line->text;    /* Next line */
            else
               macro->tptr = NULL;                 /* No next line */
         }
         else
            macro->tptr = mtext; /* Save current position in macro text */
      }
   }
   return r;
}

static int macrolabelnum;

Symbol *macrolabel(char *s, int l)
{  Symbol *r;
   char labname[50];

   if( !(currentfile->flags & ff_macro) )
   {  error("$@ not in macro");
      return 0;
   }

   l -= (s[l-1 ] == ':' )? 2: 1;

   while( isspace(*s) ) s++,l--;

   if( !(currentfile->flags & ff_macrolabelvalid) )
   {
      sprintf(currentfile->io.macro->label,"%d", macrolabelnum++);
      currentfile->flags |= ff_macrolabelvalid;
   }

   memcpy(labname,s,l); labname[l] = '\0';
   strcat(labname+l,currentfile->name);
   strcat(labname+l,"$");
   strcat(labname+l,currentfile->io.macro->label);

   r = lookup(labname,strlen(labname),1,S_LABEL);
   r->symflags |= sf_invisible;
   return r;
}

Symbol *locallabel(char *text, int l)
{
   char localname[20];
   Symbol *sym;
   int l1 = sprintf(localname,"%s$",lastlabel->name);

   if( text[l-1] == ':' ) l--;

   memcpy(localname+l1,text,l);
   localname[l1+l] = '\0';
   sym = lookup(localname,l1+l,1,S_LABEL);
   sym->symflags |= sf_invisible;
   return sym;
}

void start_skipping(void)
{
   skipping_level++;
   BEGIN SKIPPING;
}

void stop_skipping(void)
{
   if( --skipping_level == 0 ) BEGIN 0;
}

void lexinit(void)
{
   lastlabel = lookup("$$$",3,1,S_LABEL);
   resolvesym(lastlabel,0,S_LABEL);
   lastlabel->symflags |= sf_invisible;
   macrolabelnum = 0;
   yy_init = 1;
}

Symbol *label(char *s, int l)
{  Symbol *r;
   bool exported = 0;

   if( s[l-1] == ':' ) l--;
   if( s[l-1] == ':' ) l--,exported=1;

   while( (*s == '\t') || (*s == ' ') ) s++,l--;
   r = lookup(s,l,1,S_NULL);  /* Do not set to label - it might be resolved */
                              /* to something else later */
   if( exported ) r->symflags |= sf_exported;
   return r;
}

TextSeg *macroarg(char *s, int l, bool comment)
{  TextSeg *r;

   if( comment )
   {  char *es = strstr(s,"//");
      l = es-s;
   }

   while( isspace(*s) && l > 0) s++,l--;
   while( isspace(s[l-1]) && l > 0) l--;

   r = aalloc(sizeof(TextSeg)+l);
   memcpy(r->text, s, l);
   r->text[l] = '\0';
   r->cdr = NULL;
   return r;
}
