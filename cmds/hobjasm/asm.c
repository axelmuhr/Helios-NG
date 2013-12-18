/* -> asm/c
 * Title:               Top level assembly routine
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "asm.h"
#include "code.h"
#include "conds.h"
#include "constant.h"
#include "errors.h"
#include "formatio.h"
#include "getline.h"
#include "globvars.h"
#include "initdir.h"
#include "iface.h"
#include "listing.h"
#include "literals.h"
#include "llabel.h"
#include "mactypes.h"
#include "nametype.h"
#include "asmvars.h"
#include "helios.h"
#include "stubs.h"
#include "opcode.h"
#include "osdepend.h"
#include "p1hand.h"
#include "p2hand.h"
#include "stats.h"
#include "store.h"
#include "tables.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

typedef struct CacheRecord *Cache;

typedef struct CacheRecord {
  Cache next;
  char *name;
  Name  data;
} CacheRecord; /* record */

static BOOLEAN from_stdio = FALSE;

char currentFileName[MaxLineLength+1],
     codeFileName[MaxLineLength+1];
BOOLEAN  codeStreamExists = FALSE,
         inputStreamExists = FALSE;
Cache    cache = NULL;

char       *stringTable;

BOOLEAN inFile;
char   *fileStore;
FILE   *codeStream,
       *inputStream;
BOOLEAN inputStreamExists;

CARDINAL cacheSize = 0;

/*---------------------------------------------------------------------------*/

void FinishFile(void)
{
  if (inputStreamExists) {
    inFile = FALSE;
    inputStreamExists = FALSE;
    fclose(inputStream);
  }; /* if */
} /* End FinishFile */

/*---------------------------------------------------------------------------*/

static void RemoveCache(void)
{
  Cache c = cache,
        d;

  while (c != NULL) {
    free(c->name);
    free(c->data.key);
    d = c->next;
    free(c);
    c = d;
  }; /* while */
  cache = NULL;
  cacheSize = 0;
} /* End RemoveCache */

/*---------------------------------------------------------------------------*/
/* Clear up after an assembly */
void FinishAsm(void)
{
 if (codeStreamExists)
  {
   codeStreamExists = FALSE ;
   fclose(codeStream) ;
   free(stringTable) ;
  }
 FinishFile() ;
 RemoveCache() ;
 PageModeOff() ;
} /* End FinishAsm */

/*---------------------------------------------------------------------------*/
/* Run an assembly */
void Asm(char *fileName)
{
 char firstFileName[MaxLineLength + 1] ;
 Name defaultTitle ;

 char defString[1] ;

 inFile = FALSE ;
 P1InitAreas() ;
 stringOffset = 0 ;
 symbolId = 0 ;
 heliosHdr.helios_module = -1 ;         /* default module number */
 heliosHdr.helios_nsyms = 0 ;           /* no symbols defined yet */
 allowUndefinedSymbols = FALSE ;        /* complain about undefined symbols */
 Init() ;
 codeStreamExists = FALSE ;
 inputStreamExists = FALSE ;

#ifdef DEBUG
 printf("DEBUG: Asm: entry initialisation complete\n") ;
#endif

 /* First check that the file name is given */
 defString[0] = '\0' ;
 defaultTitle.length = 1 ;
 defaultTitle.key = defString ;
 if (!InputFile(firstFileName))
  {
   *firstFileName = 0 ;
   from_stdio = TRUE ;
  } ; /* if */
 strcpy(codeFileName,fileName) ;

#ifdef DEBUG
 printf("DEBUG: Asm: about to perform assembly initialisation\n") ;
#endif

 /* Now initialise the assembly */
 InitSymbolTable() ;

#ifdef DEBUG
 printf("DEBUG: Asm: InitSymbolTable returned\n") ;
#endif

 InitDirectives() ;

#ifdef DEBUG
 printf("DEBUG: Asm: InitDirectives returned\n") ;
#endif

 InitOpcode() ;

#ifdef DEBUG
 printf("DEBUG: Asm: InitOpcode returned\n") ;
#endif

 InitMacroTable() ;

#ifdef DEBUG
 printf("DEBUG: Asm: InitMacroTable returned\n") ;
#endif

 if (printState)
  PageModeOn() ;

 nextListState = (1 << ListCondPC) | (1 << ListSETPC) | (1 << ListMacExpPC) | (1 << ListMacCallPC) ;
 nextListState &= ~(1 << ListPC) ;

 SetTitle(defaultTitle) ;
 SetSubtitle(defaultTitle) ;
 totalErrors = 0 ;
 LiteralAsmStart() ;
 allowFP = TRUE ;
 hadFP = FALSE ;
 keepingAll = TRUE ;

#ifdef DEBUG
 printf("DEBUG: Asm: about to start pass 1\n") ;
#endif

 /* Now pass 1 */
 pass = 1 ;
 InitLocalLabels() ;
 InitStructureStack() ;
 InitStubs() ;
 WarningChs("Pass 1\\N") ;
 strcpy(currentFileName,firstFileName) ;
 programCounter = 0 ;
 InitLineList() ;

 {
  Name          name ;
  SymbolPointer symbolPointer ;

  /* Create the global logical variable "make_def" */
  name.length = strlen("make_def") ;
  name.key = mymalloc(name.length + 1) ;
  memcpy(&(name.key[0]),"make_def\0",name.length + 1) ;
  symbolPointer = DefineGlobalL(name) ;

  /* Initialise depending on the presence of the command line flag */
  if (clmake_def)
   symbolPointer->value.bool = TRUE ;
  else
   symbolPointer->value.bool = FALSE ;

  /* Create the global logical variable "make_SMT" */
  name.length = strlen("make_SMT") ;
  name.key = mymalloc(name.length + 1) ;
  memcpy(&(name.key[0]),"make_SMT\0",name.length + 1) ;
  symbolPointer = DefineGlobalL(name) ;

  /* Initialise depending on the presence of the command line flag */
  if (clmake_SMT)
   symbolPointer->value.bool = TRUE ;
  else
   symbolPointer->value.bool = FALSE ;
 }

 do
  {
   if (PollEscape())
    {
     FinishAsm() ;
     return ;
    } ; /* if */
  } while (!P1File(currentFileName,FALSE)) ;

 if (exception == FileNotFound)
  exception = None ; /* Can ignore it now */
 if (totalErrors > 0)
  {
   FinishAsm() ;
   WarningChs("\\NAssembly terminated\\N") ;
   TellErrors() ;
   return ;
  } ;

 /* Now pass 2 */
 SetTitle(defaultTitle) ;
 SetSubtitle(defaultTitle) ;
 inFile = FALSE ;
 pass = 2 ;

 /* Helios objects have no specific object header */

 InitLocalLabels() ;
 InitStructureStack() ;
 InitStubs() ;
 nextListState = (1 << ListCondPC) | (1 << ListSETPC) | (1 << ListMacExpPC) | (1 << ListMacCallPC) | (1 << ListPC) ;
 WarningChs("Pass 2\\N") ;
 if (PollEscape())
  {
   FinishAsm() ;
   return ;
  } ; /* if */

 programCounter = 0 ;
 P2InitAreas() ;
 strcpy(currentFileName,firstFileName) ;
 codeStream = fopen(codeFileName,"w") ;
 if ((codeStream != NULL) && (!ferror(codeStream)))
  {
   /* allocate the string/symbol table */
   stringTable = mymalloc(stringOffset) ;

   DumpHeader() ;
   CodeInit() ;
   codeStreamExists = TRUE ;
   do
    {
     if (PollEscape())
      {
       FinishAsm() ;
       return ;
      } ; /* if */
    } while (!P2File(currentFileName,FALSE)) ;
   if (exception == FileNotFound)
    exception = None ;
   /* Can ignore it now */
  }
 else
  Warning(CantOpenCode) ;

 PutLine() ;
 if (totalErrors > 0)
  {
   FinishAsm() ;
   WarningChs("\\NAssembly terminated\\N") ;
   TellErrors() ;
   return ;
  } ;
  
 PutImplicitImports() ;
 CodeEnd() ;
 RelocEnd() ;
 DumpAreaDecs() ;
 DumpSymbolTable() ;
 DumpStringTable() ;
 FinishAsm() ;
 WarningChs("\\NAssembly complete\\NNo errors found\\N") ;
} /* End Asm */

/*---------------------------------------------------------------------------*/

static BOOLEAN StoreAvailable(char **p,CARDINAL request)
{
 /* Limit the user to "maxCache" bytes of cached source file.
  * Note: we currently preserve the old cache entries, and just stop any
  *       new cache insertions.
  */
 if ((cacheSize + request) <= maxCache)
  {
   if ((*p = (char *)malloc(request)) == NULL)
    return(FALSE) ;                     /* no heap available for the cache */
   cacheSize += request ;
   return(TRUE) ;
  } ;
 return(FALSE) ;
} /* End StoreAvailable */

/*---------------------------------------------------------------------------*/

static BOOLEAN FileInCache(char *file, Cache *c)
/*See if file in cache*/
{
  CARDINAL i;
  BOOLEAN  found;

  *c = cache;
  while (*c != NULL) {
    i = 0;
    found = TRUE;
    while ((file[i] > Space) && (file[i] < Del) && ((*c)->name[i] > Space)
      && ((*c)->name[i] < Del)) {
      if (file[i] == (*c)->name[i]) i++;
      else {
        found = FALSE;
        break;
      }; /* if */
    }; /* while */
    if (found && ((file[i] <= Space) || (file[i] >= Del)) &&
      (((*c)->name[i] <= Space) || ((*c)->name[i] >= Del))) return TRUE;
    *c = (*c)->next;
  }; /* while */
  return FALSE;
} /* End FileInCache */

/*---------------------------------------------------------------------------*/

/* stream is an override, when set true it forces bytestream mode */
static BOOLEAN LoadFile(char *currentFileName/*, BOOLEAN stream*/)
{
  CARDINAL localSize;
  Cache    c;
  char    *s;

#ifdef DEBUG
 printf("DEBUG: LoadFile: currentFileName = \"%s\"\n",currentFileName) ;
#endif

  inputStreamExists = FALSE;
  fileReadMode = WholeFileLoad;
  /* Here we handle input from stdin */
  if (from_stdio)
   {
    typedef struct buffer *Buff ;
    typedef struct buffer {
                           Buff next;
                           char data[1025];
                          } ;
    Buff buffer = mymalloc(sizeof(struct buffer)), temp = buffer ;
    int  i = 0, j = 0 ;
    while (!(ferror(stdin) || feof(stdin)))
     {
      temp->data[i++] = getc(stdin);
      if (i == 1024)
       {
        temp->next = mymalloc(sizeof(struct buffer));
        temp->data[1024] = 0;
        i = 0;
        j++;
        temp = temp->next;
       }; /* if */
     }; /* while */
    temp->data[i] = 0;
    /* Now we've read it all, add it to the cache */
    localSize = 1024*j+i;
    c = mymalloc(sizeof(*c));
    c->data.key = mymalloc(localSize);
    c->data.length = localSize;
    c->name = mymalloc(1);
    *c->name = 0;
    c->next = cache;
    cache = c;
    /* cache entry set up, now add the data */
    s = c->data.key;
    temp = buffer;
    for (i = 0; i <= j; i++) {
      strcpy(s, temp->data);
      s += 1024;
      temp = temp->next;
      free(buffer);
      buffer = temp;
    }; /* for */
    from_stdio = FALSE;
   }; /* if */

  if (caching && FileInCache(currentFileName,&c))
   {
    fileStore = c->data.key;
    fileSize = c->data.length;
    pointerInFile = fileStore;
    inFile = TRUE;
    lineNumber = 0;
    return TRUE;
   } ; /* if */

  inputStream = fopen(currentFileName,"r") ;
  if ((inputStream == NULL) || ferror(inputStream))
   {
    char tmpname[MaxLineLength + 1] ;
    /* Check and see if the file exists on the filepath */
    if (HdrPathname(tmpname))
     {
      sprintf(tmpname + strlen(tmpname),"/%s",currentFileName) ;
      strcpy(currentFileName,tmpname) ;
      inputStream = fopen(currentFileName,"r") ;
      if ((inputStream == NULL) || ferror(inputStream))
       {
        inFile = FALSE ;
#ifndef Cstyle
        ErrorFile(FALSE) ;
#endif /* Cstyle */
        WarningReport("could not be opened") ;
        exception = FileNotFound ;
        return FALSE ;
       }
     }
    else
     {
      inFile = FALSE ;
#ifndef Cstyle
      ErrorFile(FALSE) ;
#endif /* Cstyle */
      WarningReport("could not be opened") ;
      exception = FileNotFound ;
      return FALSE ;
     }
   }  

  inputStreamExists = TRUE;
  /*Find out the file length*/
  fseek(inputStream, 0, SEEK_END);
  localSize = (CARDINAL)ftell(inputStream);
  /*Go to the start of the file*/
  fseek(inputStream, 0, SEEK_SET);
  /*stream OR*//*Experimental mod to cache GET files!*/
  if (!(caching && StoreAvailable(&s,localSize)))
   fileReadMode = ByteStream ;
  else
   {
    /* Add file to cache */
    c = mymalloc(sizeof(*c)) ;
    c->data.key = s ;
    c->data.length = localSize ;
    c->name = mymalloc(strlen(currentFileName) + 1) ;
    strcpy(c->name,currentFileName) ;
    c->next = cache ;
    cache = c ;
    if (fread(s,1,localSize,inputStream) != localSize)
     {
      inputStreamExists = FALSE ;
      fclose(inputStream) ;
#ifndef Cstyle
      ErrorFile(FALSE) ;
#endif
      WarningReport("could not all be loaded") ;
      exception = FileNotFound ;
      return FALSE ;
     } ;
    /* Close the file */
    inputStreamExists = FALSE ;
    fclose(inputStream) ;
    fileStore = s ;
    fileSize = localSize ;
    pointerInFile = fileStore ;
#ifdef DEBUG
 printf("DEBUG: LoadFile: fileStore &%08X, fileSize &%08X\n",(int)fileStore,(int)fileSize) ;
#endif
   } ;


  /* Where the file starts */
  inFile = TRUE;
  lineNumber = 0;
  return TRUE;
} /* End LoadFile */

/*---------------------------------------------------------------------------*/

void CopyFileName(char *pointer)
/* Get the new file name to be linked to and put it in currentFileName */
{
  CARDINAL index;
  index = 0;
  while ((pointer[index] != Space) && (pointer[index] != CR)) {
    currentFileName[index] = pointer[index];
    index++;
  };
  currentFileName[index] = '\0' ;
} /* End CopyFileName */

/*---------------------------------------------------------------------------*/

/* DO the first pass, returning TRUE if END directive encountered
 * Stream is used to override file load format when get directives in use
 */
BOOLEAN P1File(char *currentFileName, BOOLEAN stream)
{
  char    *linkPointer;
  BOOLEAN wasLink,
          ended;

#ifdef DEBUG
 printf("DEBUG: P1File: currentFileName \"%s\"\n",currentFileName) ;
#endif

  if (LoadFile(currentFileName/*, stream*/)) {
    if (!stream) LiteralFileStart();
    macroLevel = 0;
    do {
      ended = P1LineHandler(&linkPointer, &wasLink);
      if (exception) break;
      ListLine();
    } while (!(ended || wasLink));
  };
  FinishFile();
  if (exception) return TRUE;
  if (wasLink) CopyFileName(linkPointer);
  if (!stream)
   {
    LiteralFileEnd() ;
    StubFileEnd() ;
   }
  return ended;
} /* End P1File */

/*---------------------------------------------------------------------------*/

BOOLEAN P2File(char *currentFileName, BOOLEAN stream)
/*Do the second pass, returning TRUE if END directive encountered*/
{
  char    *linkPointer;
  BOOLEAN wasLink,
          ended;
  if (LoadFile(currentFileName/*, stream*/)) {
    if (!stream) LiteralFileStart();
    macroLevel = 0;
    do {
      ended = P2LineHandler(&linkPointer, &wasLink);
      if (exception) break;
      ListLine();
    } while (!(ended || wasLink));
  };
  FinishFile();
  if (exception) return TRUE;
  if (wasLink) CopyFileName(linkPointer);
  if (!stream)
   {
    LiteralFileEnd() ;
    StubFileEnd() ;
   }
  return ended;
} /* End P2File */

/*---------------------------------------------------------------------------*/

void Init_Asm(void)
{
 maxCache = MaxCache ;
}

/*---------------------------------------------------------------------------*/
/* EOF asm/c */
