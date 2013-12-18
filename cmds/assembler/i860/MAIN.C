#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "iasm.h"

extern int yyparse(void);

int warns;
int errors;
int pass;
int passevalflag;
int debugflags;
uint32 dot;
char *includepaths;

#define MAXFILES 2

void setdebugflags( char *s)
{  char ch;
   while( (ch = *s++)!=0 )
   {
      switch( ch )
      {
#if YYDEBUG != 0
      case 'y':
         yydebug = 1;
         break;
#endif
      case 'm':
         debugflags |= DEBUG_MACRO;
         break;

      default:
         warn("Invalid debug option setting");
         break;
      }
   }
}

int main(int argc, char *argv[])
{  char *files[MAXFILES];
   int nfiles=0;
   bool dosrecs = 0;

   for( argv++; --argc; argv++)
   {  char *s=*argv;
      switch( *s )
      {
      case '-':
      {
         switch(*++s)
         {
         case 'i':
            if( *++s == '\0' )
            {  s = argv[1];
               if( s == NULL || *s == '-' )
               {  parmerror("Include directory path names required");
                  argv--;
                  break;
               }
               argv++,argc--;
            }
            includepaths = s;
            break;

         case 'd':
            setdebugflags(++s);
            break;
         case 'l':
            if( *++s == '\0' )
            {  s = argv[1];
               if( s == NULL || *s == '-' )
               {  parmerror("Listing file name required");
                  argv--;
                  break;
               }
               argv++,argc--;
            }
            setlistfile(s);
            break;
         case 's':
            dosrecs = 1;
            break;
         default:
            warn("Invalid option - Ignored");
            break;
         }
         break;
      }
      default:
         if( nfiles == MAXFILES )
            parmerror("Too many files");
         files[nfiles++] = s;
      }
   }

   currentfile = filestack-1;

   if( nfiles != 2 )
      parmerror("Not enough files\n");

   if( !performget(files[0], 1) )
   {
      fatal("Exiting\n");
   }

   if( dosrecs )
      setsrecfile(files[1]);
   else
      setobjectfile(files[1]);

   initcodebuf();
/*
      Do Pass 1
*/
   pass  = 1;
   passevalflag = sf_eval1;
   syminit();
   lexinit();
   listinglevel = 0;
   pcloc = 0;
   dot   = 0;
   yyparse();

/*
      Do Pass 2
*/
   pass  = 2;
   passevalflag = sf_eval2;
   lexinit();
   syminit();
   performget(files[0], 1 );
   listinglevel = 0;
   pcloc = 0;
   dot   = 0;
   dataloc = 0;
   reinitcodebuf(0);
   yyparse();

   endoutput();
   return 0;
}

void fatal(char *s, ...)
{
   va_list ap;
   va_start(ap,s);
   fprintf(stderr,"Fatal at line %d of %s : ",
                         currentfile->lineno, currentfile->name);
   vfprintf(stderr,s,ap);
   fputc('\n',stderr);
   va_end(ap);
   exit(1);
}

void warn(char *s, ...)
{
   if( pass == 2 )
   {  va_list ap;
      va_list ap1;
      va_start(ap,s);
      va_start(ap1,s);
      fprintf(stderr,"Warning at line %d of %s : ",
                            currentfile->lineno, currentfile->name);

      warns++;
      vfprintf(stderr,s,ap);
      fputc('\n', stderr);
      listmessage("Warning",s,ap1);
      va_end(ap);
   }
}

void error(char *s, ...)
{
   if( pass == 2 )
   {
      va_list ap;
      va_list ap1;
      va_start(ap,s);
      va_start(ap1,s);
      fprintf(stderr,"Error at line %d of %s : ",
                            currentfile->lineno, currentfile->name);
      errors++;
      vfprintf(stderr,s,ap);
      fputc('\n',stderr);
      listmessage("Error",s,ap1);
      va_end(ap);
   }
}

void parmerror(char *s, ...)
{
   va_list ap;
   va_start(ap,s);
   fprintf(stderr,"Error in parameters : ");
   errors++;
   vfprintf(stderr,s,ap);
   fputc('\n',stderr);
   va_end(ap);
   exit(1);
}

Expression *unused_expressions = NULL;
Expression *used_expressions = NULL;

Expression *newexpression(void)
{  Expression *expr;
   if( (expr = unused_expressions) != NULL )
   {
      unused_expressions = expr->exprlist;
   }
   else
   {
      expr = aalloc(sizeof(Expression));
   }
   expr->exprlist = used_expressions;
   used_expressions = expr;
   return expr;
}

void free_expression(Expression *expr)
{  Expression **exprp = &used_expressions;

   while( *exprp != NULL )
   {
      if( *exprp == expr )
      {
         *exprp = expr->exprlist;
         expr->exprlist = unused_expressions;
         unused_expressions = expr;
         return;
      }
      exprp = &((*exprp)->exprlist);
   }
}

void free_expressions(void)
{
   Expression **exprp = &used_expressions;

   while( *exprp != NULL ) exprp = &((*exprp)->exprlist);
   *exprp = unused_expressions;
   unused_expressions = used_expressions;
   used_expressions = NULL;
}

void *aalloc(size_t n)
{
   return malloc(n);
}

void afree(void *p)
{
   free(p);
}
