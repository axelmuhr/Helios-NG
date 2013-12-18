/* $Header: macros.c,v 1.3 90/11/20 16:41:21 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/drawp/RCS/source/tools/macros.c,v $ */

/*-----------------------------------------------------------------*/
/*                                                 tools/macros.c  */
/*-----------------------------------------------------------------*/

/* This file is a program to expand the macros in an objasm format */
/*   file in order to compensate for the crappy objasm habit of    */
/*   having a maximum length for macros                            */
/* In addition, this file permits macros to be forward-referenced  */
/* The output file is also an objasm-file ...                      */
/* In the output file, the macro definitions will have been        */
/*     removed, and each macro definition will have been replaced  */
/*     by an instance of the macro. Local variable definitions     */
/*     will have been replaced by global variable definitions with */
/*     unique suffixes added to the labels, and the same applies   */
/*     to parameters which are also initialized to the macro       */
/*     parameter value on entry                                    */
/* The program reads the file which is the first parameter and     */
/*     writes the file which is the second parameter. If an error  */
/*     occurs then the output file is deleted.                     */
/* Any diagnostics are written on the                              */
/*     standard error stream.                                      */
/* The program does not (as yet,) process 'GET' and the like, so   */
/*     be careful                                                  */

/*-----------------------------------------------------------------*/
/*                                                  Include files  */
/*-----------------------------------------------------------------*/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/*-----------------------------------------------------------------*/
/*                                                     Structures  */
/*-----------------------------------------------------------------*/

#define True  1
#define False 0
typedef unsigned char Boolean_t;

#define MAXSTRLEN 255
typedef char String_t[MAXSTRLEN+1];

typedef long Position_t;

typedef struct MacroList_s
{  String_t   macroName;
   Position_t macroStart;
   int        startLine;
   struct MacroList_s *nextMacro;
} MacroList_t;

typedef struct LineRecord_s
{  String_t   lineText;
   int        lineNo;
   struct LineRecord_s *calledFrom;
} LineRecord_t;

typedef struct VariableList_s
{  String_t   varName;
   String_t   newName;
   struct VariableList_s *nextVar;
   int        level;
} VariableList_t;

/*-----------------------------------------------------------------*/
/*                                            Utility definitions  */
/*-----------------------------------------------------------------*/

#define isWs(chr)      ((chr)<=32)
#define getMemory(typ) ((typ*)allocWithErr(sizeof(typ)))

/*-----------------------------------------------------------------*/
/*                                        Look-ahead declarations  */
/*-----------------------------------------------------------------*/

int             main            (int,char**);
void            processFile     (FILE*,FILE*);
MacroList_t    *firstPass       (FILE*,FILE*);
void            secondPass      (FILE*,FILE*,MacroList_t*);
void            expandMacro     (FILE*,FILE*,MacroList_t*,LineRecord_t*,
                                 MacroList_t*,int*,int,VariableList_t*);
VariableList_t *readParameters  (FILE*,LineRecord_t*,LineRecord_t*,int*,
                                 int,VariableList_t*);
void            newVariable     (FILE*,VariableList_t**,char,
                                 LineRecord_t*,int*,int);
void            doSubstitutions (LineRecord_t*,VariableList_t*);
void            freeVariables   (int,VariableList_t*);
void            getSymbol       (char*,char*,char**);
Boolean_t       isInstr         (char*,LineRecord_t*);
void            instrCpy        (char*,LineRecord_t*);
char           *getOpand        (LineRecord_t*);
void            outLinePath     (FILE*,LineRecord_t*);
void            getLine         (FILE*,LineRecord_t*);
void            outputLine      (FILE*,LineRecord_t*);
void           *allocWithErr    (int);
void            giveOsReport    (void);
void            fatalError      (void);

/*-----------------------------------------------------------------*/
/*                                                        Globals  */
/*-----------------------------------------------------------------*/

Boolean_t isErr;
char     *outfn;
FILE     *outfd;

int dbg;

/*-----------------------------------------------------------------*/
/*                                                         main()  */
/*-----------------------------------------------------------------*/

int main(int argc,char **argv)
{  FILE *infd;

   if(argc!=3)
   {  fprintf(stderr,"Format : macasm <infile> <outfile>\n");
      return 1;
   }
   
   infd = fopen(argv[1],"rb");
   if(infd==NULL)
   {  fprintf(stderr,"macasm : Cannot open input file : %s\n",argv[1]);
      giveOsReport();
      return 1;
   }
   
   outfd = fopen(argv[2],"wb");
   if(outfd==NULL)
   {  fprintf(stderr,"asmmac : Cannot open output file : %s\n",argv[2]);
      giveOsReport();
      return 1;
   }
   
   outfn = argv[2];
   isErr = False;

   fprintf(stderr,"Macro pre-processor for Objasm files\n");   
   processFile(infd,outfd);
   
   fclose(infd);
   fclose(outfd);
   
   if(isErr)
   {  remove(outfn);
      return 1;
   }
   
   return 0;
}

/*-----------------------------------------------------------------*/
/*                                               processFile(...)  */
/*-----------------------------------------------------------------*/

void processFile(FILE *infd,FILE *outfd)
{  MacroList_t *maclist;

   fprintf(stderr,"Pass 1\n");
   maclist = firstPass(infd,outfd);
   if(isErr)
   {  fprintf(stderr,"Preprocessing terminated\n");
      return;
   }
   fprintf(stderr,"Pass 2\n");
   secondPass(infd,outfd,maclist);
}

/*-----------------------------------------------------------------*/
/*                                                firstPass(infd)  */
/*-----------------------------------------------------------------*/

MacroList_t *firstPass(FILE *infd,FILE *outfd)
{  MacroList_t  *maclist;
   LineRecord_t  linerec;
   int           inMacro;

   inMacro            = False;
   linerec.lineNo     = 0;
   linerec.calledFrom = NULL;
   maclist = NULL;
   fseek(infd,0L,SEEK_SET);
   while(!feof(infd)&&!ferror(infd))
   {  getLine(infd,&linerec);
      if(isInstr("MACRO",&linerec))
      {  MacroList_t *macent;
         char        *macptr;
         if(inMacro)
         {  fprintf(stderr,"Line %d",linerec.lineNo);
            fprintf(stderr," : Macro definition within macro \"%s\" ",
                           maclist->macroName);
            fprintf(stderr,"defined on line %d\n",maclist->startLine);
            isErr = True;
         }
         if(!isWs(linerec.lineText[0]))
         {  fprintf(stderr,"Line %d : No label on \'MACRO\' line, please",
                     linerec.lineNo);
            isErr = True;
         }
         macptr = getOpand(&linerec);
         if(*macptr&&*macptr!=';')
         {  fprintf(stderr,"Line %d : ",linerec.lineNo);
            fprintf(stderr,"Garbage after \'MACRO\' directive\n");
         }
         macent = getMemory(MacroList_t);
         macent->nextMacro  = maclist;
         macent->startLine  = linerec.lineNo;
         macent->macroStart = ftell(infd);
         getLine(infd,&linerec);
         instrCpy(macent->macroName,&linerec);
         if(macent->macroName[0]==0)
         {  fprintf(stderr,"Line %d : Bad macro definition line\n",
                    linerec.lineNo);
            isErr = True;
            free(macent);
         }
         else
            maclist = macent;
         inMacro = True;
      }
      else if(isInstr("SHORT_MACRO",&linerec))
      {  int oldLine;
         oldLine = linerec.lineNo;
         strcpy(linerec.lineText," MACRO");
         outputLine(outfd,&linerec);
         do
         {  getLine(infd,&linerec);
            outputLine(outfd,&linerec);
         } while (!feof(infd)&&!ferror(infd)&&!isInstr("MEND",&linerec));
         if(feof(infd))
         {  fprintf(stderr,"Line %d : Macro without MEND\n",oldLine);
            isErr=True;
         }
      }
      else if(isInstr("MEND",&linerec))
      {  if(!inMacro)
         {  fprintf(stderr,"Line %d",linerec.lineNo);
            fprintf(stderr," : MEND without macro\n");
            isErr = True;
         }
         inMacro = False;
      }
   }
   if(inMacro)
   {  fprintf(stderr,"Line %d : ",linerec.lineNo);
      fprintf(stderr,
              "Unexpected EOF: Macro \'%s\' on line %d not terminated\n",
              maclist->macroName,maclist->startLine);
      isErr = True;
   }
   return maclist;
}

/*-----------------------------------------------------------------*/
/*                                                secondPass(...)  */
/*-----------------------------------------------------------------*/

void secondPass(FILE *infd,FILE *outfd,MacroList_t *maclist)
{  MacroList_t  *macent;
   LineRecord_t  linerec;
   int           tag;

   linerec.lineNo     = 0;
   linerec.calledFrom = NULL;
   tag = 0;
   fseek(infd,0L,SEEK_SET);
   while(!feof(infd)&&!ferror(infd))
   {  getLine(infd,&linerec);
      /* Skip macros ... */
      if(isInstr("MACRO",&linerec)||isInstr("SHORT_MACRO",&linerec))
      {  do
         {  getLine(infd,&linerec);
         } while (!feof(infd)&&!ferror(infd)&&!isInstr("MEND",&linerec));
         continue;
      }
      for(macent=maclist;macent;macent=macent->nextMacro)
         if(isInstr(macent->macroName,&linerec)) break;
      if(macent)
         expandMacro(infd,outfd,macent,&linerec,maclist,&tag,1,NULL);
      else
         outputLine(outfd,&linerec);
   }
}
         
/*-----------------------------------------------------------------*/
/*                                               expandMacro(...)  */
/*-----------------------------------------------------------------*/

void expandMacro(FILE *infd,
                 FILE *outfd,
                 MacroList_t  *thismac,
                 LineRecord_t *macline,
                 MacroList_t  *maclist,
                 int *tag,
                 int  level,
                 VariableList_t *vlist)
{  LineRecord_t    linerec;
   Position_t      oldPos;
   MacroList_t    *macent;
   
   linerec.lineNo     = thismac->startLine;
   linerec.calledFrom = macline;

   oldPos = ftell(infd);
   fseek(infd,thismac->macroStart,SEEK_SET);
   
   getLine(infd,&linerec);
   vlist = readParameters(outfd,&linerec,macline,tag,level,vlist);
   
   while(1)
   {  if(feof(infd)||ferror(infd))
      {  outLinePath(stderr,&linerec);
         fprintf(stderr," : Unexpected EOF in macro\n");
         isErr = True;
      }
      getLine(infd,&linerec);
      if(isInstr("LCLA",&linerec))
         newVariable(outfd,&vlist,'A',&linerec,tag,level);
      else if(isInstr("LCLS",&linerec))
         newVariable(outfd,&vlist,'S',&linerec,tag,level);
      else if(isInstr("LCLC",&linerec))
         newVariable(outfd,&vlist,'C',&linerec,tag,level);
      else if(isInstr("MEND",&linerec)) break;
      else 
      {  doSubstitutions(&linerec,vlist);
         for(macent=maclist;macent;macent=macent->nextMacro)
            if(isInstr(macent->macroName,&linerec)) break;
         if(macent)
            expandMacro(infd,outfd,macent,&linerec,maclist,tag,level,vlist);
         else
            outputLine(outfd,&linerec);
      }
   }
   
   fseek(infd,oldPos,SEEK_SET);
   freeVariables(level,vlist);
   
}

/*-----------------------------------------------------------------*/
/*                                            readParameters(...)  */
/*-----------------------------------------------------------------*/

VariableList_t *readParameters(FILE           *outfd,
                               LineRecord_t   *defline,
                               LineRecord_t   *invline,
                               int            *tag,
                               int             level,
                               VariableList_t *vlist)
{  LineRecord_t    outline;
   char            *srcptr,*desptr,*defptr,*varptr,*laptr;
   int             bracketLvl,quotes,notEnoughParams;
   VariableList_t *newVar;
   String_t        tmpstr,vartmp;
   
   vlist = NULL;
   /* Now to read the macro parameters in <defline> and output */
   /*   string definitions and assignements for each parameter */
  
   /* First prepare outline: */
   outline.lineNo     = invline->lineNo;
   outline.calledFrom = invline->calledFrom;
   if(defline->lineText[0]!='$')
   {  if(!isWs(defline->lineText[0]))
      {  outLinePath(stderr,defline);
         fprintf(stderr," : Bad label on macro header line\n");
         isErr = True;
      }
   }
   else
   {  newVar = getMemory(VariableList_t);
      newVar->level = level;
      newVar->nextVar = vlist; vlist = newVar;
      getSymbol(newVar->varName,defline->lineText+1,NULL);
      sprintf(newVar->newName,"%s_%d",newVar->varName,(*tag)++);
      sprintf(outline.lineText," GBLS %s",newVar->newName);
      outputLine(outfd,&outline);
      desptr=tmpstr;
      for(srcptr=invline->lineText;!isWs(*srcptr);srcptr++)
      {  if(*srcptr=='\"') *desptr++='\"';
         *desptr++=*srcptr;
      } *desptr = 0;
      sprintf(outline.lineText,"%s SETS \"%s\"",newVar->newName,tmpstr);
      outputLine(outfd,&outline);
   }
  
   defptr = getOpand(defline);
   srcptr = getOpand(invline);
   notEnoughParams = False;
   
   while(*defptr=='$')
   {  defptr++;
      newVar = getMemory(VariableList_t);
      newVar->level = level;
      newVar->nextVar = vlist; vlist = newVar;
      getSymbol(newVar->varName,defptr,&defptr);
      while(*defptr&&isWs(*defptr)) defptr++;
      if(*defptr==',') for(defptr++;*defptr&&isWs(*defptr);defptr++);
      sprintf(newVar->newName,"%s_%d",newVar->varName,(*tag)++);
      sprintf(outline.lineText," GBLS %s",newVar->newName);
      outputLine(outfd,&outline);
      bracketLvl=0; quotes = False;
      if(*srcptr==0||*srcptr==';') notEnoughParams=True;
      while(*srcptr&&isWs(*srcptr)) srcptr++;
      for(desptr=tmpstr;*srcptr;srcptr++)
      {  for(laptr=srcptr;*laptr&&isWs(*laptr);laptr++);
         if(*laptr==0) break;
         if(!quotes)
         {  if(bracketLvl==0&&*laptr==',') break;
            if(*laptr==';') break;
         }
         if(*srcptr=='\"') quotes=!quotes, *desptr++ = '\"';
         if(*srcptr=='(') bracketLvl++;
         if(*srcptr==')') bracketLvl--;
         if((*srcptr==0x0A||*srcptr==0x0D)&&!quotes) continue;
         if(bracketLvl<0) bracketLvl=0;
         if(*srcptr=='$'&&isalpha(srcptr[1]))
         {  getSymbol(vartmp,srcptr+1,&srcptr);
            *desptr++ = '\"'; *desptr++ = ':' ;
            *desptr++ = 'C' ; *desptr++ = 'C' ;
            *desptr++ = ':' ;
            for(varptr=vartmp;*varptr;varptr++) *desptr++=*varptr;
            *desptr++ = ':' ; *desptr++ = 'C' ; 
            *desptr++ = 'C' ; *desptr++ = ':' ;
            *desptr++ = '\"';
            srcptr--;
         }
         else
           *desptr++=*srcptr;
      } *desptr = 0;
      while(*srcptr&&isWs(*srcptr)) ++srcptr;
      if(*srcptr==0&&quotes)
      {  outLinePath(stderr,invline);
         fprintf(stderr," : Missing quote\n");
         isErr = True;
      }
      if(*srcptr==',') srcptr++;
      sprintf(outline.lineText,"%s SETS \"%s\"",newVar->newName,tmpstr);
      outputLine(outfd,&outline);
   }
  
   if(notEnoughParams)
   {  outLinePath(stderr,invline);
      fprintf(stderr," : Not enough parameters being passed to macro\n");
      isErr = True;
   }

   while(*srcptr&&isWs(*srcptr)) srcptr++;
   if(*srcptr&&*srcptr!=';')
   {  outLinePath(stderr,invline);
      fprintf(stderr," : Too many parameters being passed to macro\n");
      isErr = True;
   }
   return vlist;
}

/*-----------------------------------------------------------------*/
/*                                               newVariable(...)  */
/*-----------------------------------------------------------------*/

void newVariable(FILE            *outfd, 
                 VariableList_t **vlist,
                 char             type,
                 LineRecord_t    *linerec,
                 int             *tag,
                 int              level)
{  char           *srcptr,*desptr;
   LineRecord_t    outline;
   String_t        lblfld;
   VariableList_t *newVar;
   
   outline.lineNo     = linerec->lineNo;
   outline.calledFrom = linerec->calledFrom;

   desptr = lblfld;
   for(srcptr=linerec->lineText;!isWs(*srcptr);srcptr++) *desptr++=*srcptr;
   *desptr = 0;
   
   newVar = getMemory(VariableList_t);
   
   newVar->nextVar = *vlist;
   newVar->level   =  level;
   *vlist = newVar;
   
   getSymbol(newVar->varName,getOpand(linerec),NULL);
   sprintf(newVar->newName,"%s_%d",newVar->varName,(*tag)++);
   
   sprintf(outline.lineText,"%s GBL%c %s",lblfld,type,newVar->newName);
   outputLine(outfd,&outline);
}

/*-----------------------------------------------------------------*/
/*                                           doSubstitutions(...)  */
/*-----------------------------------------------------------------*/

void doSubstitutions(LineRecord_t *linerec,VariableList_t *vlist)
{  String_t        tmpstr,varname;
   char           *srcptr,*desptr,*srcvar,*desvar;
   VariableList_t *varsrch;
   Boolean_t       quotes,vquotes,inSym;

   strcpy(tmpstr,linerec->lineText);
   srcptr = tmpstr;
   desptr = linerec->lineText;

   inSym = quotes = vquotes = False;
   while(*srcptr)
   {  if(inSym||!isalpha(*srcptr))
      {  if(isalpha(*srcptr)) inSym=True;
         if(!isalpha(*srcptr)&&!isdigit(*srcptr)&&*srcptr!='_') 
            inSym = False;
         if(!quotes)
         {  if(*srcptr=='|') vquotes = !vquotes;  }
         else if(!vquotes)
         {  if(*srcptr=='\"') quotes = !quotes;  }
         *desptr++=*srcptr++;
         continue;
      }
      desvar = varname;
      while(*srcptr)
      {  if(isalpha(*srcptr)||isdigit(*srcptr)||*srcptr=='_')
            *desvar++ = *srcptr++;
         else break;
      }
      *desvar = 0;
      for(varsrch=vlist;varsrch;varsrch=varsrch->nextVar)
      {  if(!strcmp(varsrch->varName,varname))
         {  for(srcvar = varsrch->newName; *srcvar; srcvar++)
               *desptr++=*srcvar;
            break;
         }
      }
      if(varsrch==NULL)
      {  for(srcvar = varname; *srcvar; srcvar++)
            *desptr++=*srcvar;
      }
      inSym = False;
   }
   *desptr = 0;
}

/*-----------------------------------------------------------------*/
/*                                             freeVariables(...)  */
/*-----------------------------------------------------------------*/

void freeVariables(int level,VariableList_t *vlist)
{  VariableList_t *nxt;

   for(;vlist&&vlist->level==level;vlist=nxt)
   {  nxt = vlist->nextVar;
      free(vlist);
   }
}

/*-----------------------------------------------------------------*/
/*                                                 getSymbol(...)  */
/*-----------------------------------------------------------------*/

void getSymbol(char *des,char *src,char **end)
{  while(isalpha(*src)||isdigit(*src)||*src=='_')
   {  *des++ = *src++;  }
   *des=0;
   if(end) *end = src;
}

/*-----------------------------------------------------------------*/
/*                                                   isInstr(...)  */
/*-----------------------------------------------------------------*/

Boolean_t isInstr(char *inst,LineRecord_t *linerec)
{  char *iptr,*lptr;

   lptr = linerec->lineText;
   if(*lptr==';') return False;
   while(*lptr&&!isWs(*lptr)) lptr++;
   while(*lptr&& isWs(*lptr)) lptr++;
   if(*lptr==';') return False;
   iptr = inst;
   while(*iptr&&*lptr&&*iptr==*lptr) iptr++,lptr++;
   if(*iptr) return False;
   if(!isWs(*lptr)) return False;
   return True;
}

/*-----------------------------------------------------------------*/
/*                                                  instrCpy(...)  */
/*-----------------------------------------------------------------*/

void instrCpy(char *des,LineRecord_t *linerec)
{  char *iptr,*lptr;

   lptr = linerec->lineText;
   if(*lptr==';') { *des=0; return; }
   while(*lptr&&!isWs(*lptr)) lptr++;
   while(*lptr&& isWs(*lptr)) lptr++;
   if(*lptr==';') { *des=0; return; }
   iptr = des;
   while(!isWs(*lptr)) *iptr++=*lptr++;
   *iptr=0;
}

/*-----------------------------------------------------------------*/
/*                                                  getOpand(...)  */
/*-----------------------------------------------------------------*/

char *getOpand(LineRecord_t *linerec)
{  char *lptr;
   lptr = linerec->lineText;
   if(*lptr==';') return lptr;
   while(*lptr&&!isWs(*lptr)) lptr++;
   while(*lptr&& isWs(*lptr)) lptr++;
   if(*lptr==';') return lptr;
   while(*lptr&&!isWs(*lptr)) lptr++;
   while(*lptr&& isWs(*lptr)) lptr++;
   return lptr;
}

/*-----------------------------------------------------------------*/
/*                                               outLinePath(...)  */
/*-----------------------------------------------------------------*/

void outLinePath(FILE *outfd,LineRecord_t *linerec)
{  LineRecord_t *recptr;

   fprintf(outfd,"Line ");
   for(recptr=linerec;recptr;recptr=recptr->calledFrom)
      fprintf(outfd,"%d%s",recptr->lineNo,recptr->calledFrom?"\\":"");
}

/*-----------------------------------------------------------------*/
/*                                                   getLine(...)  */
/*-----------------------------------------------------------------*/

void getLine(FILE *infd,LineRecord_t *linerec)
{  char *cmp;

   cmp = fgets(linerec->lineText,MAXSTRLEN,infd);
   if(cmp==NULL) linerec->lineText[0] = 0;
   linerec->lineNo++;

}

/*-----------------------------------------------------------------*/
/*                                                outputLine(...)  */
/*-----------------------------------------------------------------*/

void outputLine(FILE *outfd,LineRecord_t *linerec)
{  char      *srcptr,*desptr;
   String_t   outln;
   Boolean_t  wasWs,quotes,empty;

   empty  = True;
   srcptr = linerec->lineText;
   desptr = outln;
   if(!isWs(*srcptr)&&*srcptr!=';')
   {  empty = False;
      do *desptr++=*srcptr++; while(!isWs(*srcptr)&&*srcptr!=';');
      *desptr++=' ';
   }
   while(*srcptr&&isWs(*srcptr)) srcptr++;
   *desptr++=' ';
   if(!isWs(*srcptr)&&*srcptr!=';')
   {  empty = False;
      do *desptr++=*srcptr++; while(!isWs(*srcptr)&&*srcptr!=';');
   }
   *desptr++=' ';
   while(*srcptr&&isWs(*srcptr)) srcptr++;
   while((desptr-outln)<8) *desptr++=' ';
   if(!isWs(*srcptr)&&*srcptr!=';')
   {  empty  = False; wasWs  = True; quotes = False;
      do
      {  if(!quotes&&isWs(*srcptr))
         {  if(!wasWs) *desptr++=' ';  }
         else
            *desptr++=*srcptr;
         wasWs = (isWs(*srcptr));
         if(*srcptr=='\"') quotes = !quotes;
         if(*++srcptr==';'&&!quotes) break;
      }  while(*srcptr);
   }
   if(!empty)
   {  while((desptr-outln)<28) *desptr++=' ';
      *desptr = '\0';
      fprintf(outfd,"%s ; ",outln);
      outLinePath(outfd,linerec);
      fputc('\n',outfd);
   }
}

/*-----------------------------------------------------------------*/
/*                                              allocWithErr(...)  */
/*-----------------------------------------------------------------*/

void *allocWithErr(int size)
{  void *mem;
   mem = calloc(1,size);
   if(mem==NULL)
   {  fprintf(stderr,"Out of memory\n");
      fatalError();
   }
   return mem;
}

/*----------------------------------------------------------------------*/
/*                                                        GiveOSReport  */
/*----------------------------------------------------------------------*/

void giveOsReport()
{ 
   switch(errno)
   {  case ENOTDIR:
         fprintf(stderr,"A component of the pathname is not a directory\n");
         break;
      case EINVAL:
         fprintf(stderr,"Bad argument (e.g. wrong type of file)\n");
         break;
      case ENAMETOOLONG:
         fprintf(stderr,"Name is too long\n");
         break;
      case ENOENT:
         fprintf(stderr,"File does not exist\n");
         break;
      case EACCES:
         fprintf(stderr,"Permission denied\n");
         break;
      case ELOOP:
         fprintf(stderr,"Passes through too many symbolic links\n");
         break;
      case EIO:
         fprintf(stderr,"An I/O error occurred during the operation\n");
         break;
      case EFAULT:
         fprintf(stderr,"Memory addressing problem, or general fault\n");
         break;
      case EBADF:
         fprintf(stderr,"Bad file descriptor (bug in code.)\n");
         break;
      case EMFILE:
         fprintf(stderr,"Too many open files\n");
         break;
      case 0:
         fprintf(stderr,"Error code is 0: no apparent error\n");
         break;
      default:
         fprintf(stderr,"Unrecognized error condition errno = %d\n",errno);
         break;
   }
}

/*-----------------------------------------------------------------*/
/*                                                   fatalError()  */
/*-----------------------------------------------------------------*/

void fatalError()
{  fclose(outfd);
   remove(outfn);
   exit(1);
}
