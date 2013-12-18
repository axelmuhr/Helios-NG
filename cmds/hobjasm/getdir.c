/* -> getdir/c
 * Title:               Handle Get and related directives
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "asm.h"
#include "conds.h"
#include "constant.h"
#include "errors.h"
#include "expr.h"
#include "getdir.h"
#include "getline.h"
#include "globvars.h"
#include "listing.h"
#include "mactypes.h"
#include "nametype.h"
#include "store.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*/

static char mnoteLine[MaxLineLength+1];

/*---------------------------------------------------------------------------*/

void GetDir(char *line, CARDINAL *lineIndex)
{
 StructureStackElement s ;

 CheckStack() ;
 if (exception)
  return ;

 s.type = GetSSET ;
 s.u.file.inputMode = fileReadMode ;
 switch (s.u.file.inputMode)
  {
   case WholeFileLoad :
                        s.u.file.u.storePointer = pointerInFile ;
                        s.u.file.fileStart = fileStore ;
                        s.u.file.fileLen = fileSize ;
                        break ;

   case ByteStream    :
                        s.u.file.u.fileOffset = (CARDINAL)ftell(inputStream) ;
                        inputStreamExists = FALSE ;
                        fclose(inputStream) ;

  } ; /* case */

 s.u.file.fileName.length = strlen(currentFileName) ;
 s.u.file.fileName.key = mymalloc(s.u.file.fileName.length + 1) ;
 strcpy(s.u.file.fileName.key,currentFileName) ;
 s.u.file.lineNumber = lineNumber ;
 if (!Stack(s))
  {
   free(s.u.file.fileName.key) ;
   exception = FileNotFound ;
   return ;
  } ; /* if */

 ListLine() ;
 /* Now go into the new files */
 CopyFileName(line + *lineIndex) ;
 while (line[*lineIndex] != CR)
  (*lineIndex)++ ;

#ifdef DEBUG
 printf("DEBUG: GetDir: currentFileName = \"%s\"\n",currentFileName) ;
#endif

 do {} while (!(((pass == 1) && P1File(currentFileName,TRUE)) || ((pass == 2) && P2File(currentFileName,TRUE)))) ;

 /* Now go back to the old file */
 (void)Unstack(&s) ;
 if (s.type != GetSSET)
  AssemblerError("Structure mismatch") ;
 fileReadMode = s.u.file.inputMode ;
 /* Include the terminator in the copy */
 memcpy(currentFileName,s.u.file.fileName.key,s.u.file.fileName.length + 1) ;
 /* And throw away the space */
 free(s.u.file.fileName.key) ;
 /* Avoid stack thinking there's a left over GET */
 s.type = ConditionalSSET ;
 (void)Stack(s) ;
 (void)Unstack(&s) ;
 if (exception)
  return ;
 switch(s.u.file.inputMode)
  {
   case WholeFileLoad :
                        pointerInFile = s.u.file.u.storePointer ;
                        fileStore = s.u.file.fileStart ;
                        fileSize = s.u.file.fileLen ;
                        break ;

   case ByteStream    :
                        inputStream = fopen(currentFileName,"r") ;
                        inputStreamExists = TRUE ;
                        if ((inputStream == NULL) || ferror(inputStream))
                         {
#ifndef Cstyle
                          ErrorFile(FALSE) ;
#endif
                          WarningReport(" not found\\N") ;
                          exception = FileNotFound ;
                          return ;
                         } ;
                        fseek(inputStream,s.u.file.u.fileOffset,SEEK_SET) ;
  } ; /* case */
 lineNumber = s.u.file.lineNumber ;
 inFile = TRUE ;
} /* End GetDir */

/*---------------------------------------------------------------------------*/

void WendDir(void)
{
  StructureStackElement s;
  if (!Unstack(&s)) return;
  if (s.type != WhileSSET) {
    if (s.type != ConditionalSSET) (void) Stack(s);
    Warning(StructErr);
    exception = StackErr;
    return;
  }; /* if */
  if ((rejectedWHILEs == 0) && (rejectedIFs == 0)) {
    lineNumber = s.u.whileEl.lineNumber;
    if ((fileReadMode == WholeFileLoad) || (macroLevel != 0))
      pointerInFile = s.u.whileEl.pointer;
    else
      fseek(inputStream, (int)s.u.whileEl.pointer, SEEK_SET);
    includedWHILEs--;
  } else {
    rejectedWHILEs--;
    nextListState = s.u.whileEl.state;
    if (((1 << ListPC) & nextListState) && !((1 << ListPC) && listStatus)) {
      listStatus |= 1 << ListPC;
      ListLineNumber();
    }; /* if */
  }; /* if */
} /* End WendDir */

/*---------------------------------------------------------------------------*/

void CheckStack(void)
/*Check that END, LNK, GET etc. don't occur inside conditionals*/
{
  if ((rejectedIFs != 0) || (includedIFs != 0) ||
   (rejectedWHILEs != 0) || (includedWHILEs != 0) || (macroLevel != 0)) {
    WarningReport("Unmatched conditional or MACRO") ;
    return;
  }; /* if */
} /* CheckStack */

/*---------------------------------------------------------------------------*/

BOOLEAN MexitDir(void)
{
StructureStackElement s;
  if (macroLevel == 0) {
    Warning(NoMacro);
    return TRUE;
  }; /* if */
  do {
    if (!Unstack(&s)) return TRUE;
    switch (s.type) {
      case ConditionalSSET:
      if (includedIFs != 0) includedIFs--;
      else AssemblerError("missing IF element on structure stack");
      break;

      case WhileSSET:
      if (includedWHILEs != 0) includedWHILEs--;
      else AssemblerError("missing WHILE element on structure stack");
      break;

      case MacroSSET:
      break;

      case GetSSET:
      AssemblerError("unexpected GET on structure stack");

    }; /* case */
    if (s.type == MacroSSET) break; /* Subtle difference in meaning for EXIT */
  } while (1); /* loop */
  lineNumber = s.u.macro.lineNumber;
  nextListState = s.u.macro.state;
  macroLevel--;
  if ((macroLevel != 0) || (s.u.macro.inputMode == WholeFileLoad))
    pointerInFile = s.u.macro.u.storePointer;
  ExitMacro();
  if (!((1 << ListMendPC) & listStatus)) CancelLineList();
  return FALSE;
} /* End MexitDir */

/*---------------------------------------------------------------------------*/

BOOLEAN MendDir(void)
{
  StructureStackElement s;
  if (macroLevel == 0) {
    Warning(NoMacro);
    return TRUE;
  }; /* if */
  if (!Unstack(&s)) return TRUE;
  if (s.type != MacroSSET) {
    Warning(BadMEND);
    while (s.type != MacroSSET) (void) Unstack(&s);
  }; /* if */
  lineNumber = s.u.macro.lineNumber;
  nextListState = s.u.macro.state;
  macroLevel--;
  if ((macroLevel != 0) || (s.u.macro.inputMode == WholeFileLoad))
    pointerInFile = s.u.macro.u.storePointer;
  ExitMacro();
  if (!((1 << ListMendPC) & listStatus)) CancelLineList();
  return FALSE;
} /* End MendDir */

/*---------------------------------------------------------------------------*/

void MnoteDir(char *line, CARDINAL *lineIndex)
{
  CARDINAL i,
           value;
  BOOLEAN  defined;
  Name     name;

  ListLine();
  value = ConstantExpr(line,lineIndex,FALSE,&defined,NULL) ;
  if (errorFound) return;
  if (line[*lineIndex] != Comma) {
    Warning(CommaMissing);
    return;
  }; /* if */
  (*lineIndex)++;
  StringExpr(line, lineIndex, &name);
  i = 0;
  memcpy(mnoteLine, name.key, name.length);
  mnoteLine[name.length] = 0;
#ifdef Cstyle
  if ((value != 0) && (pass == 1)) NotifyReport(mnoteLine) ;
#else
  if ((value != 0) && (pass == 1)) WarningReport(mnoteLine) ;
#endif /* Cstyle */
  if ((value == 0) && (pass == 2)) {
    WarningChs(mnoteLine);
    WarningChs("\\N");
  }; /* if */
} /* End MnoteDir */

/*---------------------------------------------------------------------------*/
/* EOF getdir/c */
