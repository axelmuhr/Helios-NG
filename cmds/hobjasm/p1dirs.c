/* -> p1dirs/c
 * Title:               Directive handler for pass 1
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "mactypes.h"
#include "code.h"
#include "conds.h"
#include "constant.h"
#include "errors.h"
#include "expr.h"
#include "extypes.h"
#include "formatio.h"
#include "fpio.h"
#include "getdir.h"
#include "getline.h"
#include "globvars.h"
#include "initdir.h"
#include "listing.h"
#include "literals.h"
#include "llabel.h"
#include "nametype.h"
#include "asmvars.h"
#include "occur.h"
#include "p1hand.h"
#include "p1dirs.h"
#include "store.h"
#include "tables.h"
#include "tokens.h"
#include "vars.h"
#include "stubs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*---------------------------------------------------------------------------*/

static void CancelListForCond(void)
{
 if (!((1 << ListCondPC) & listStatus))
  CancelLineList() ;
} /* EndCancelListForCond */

/*---------------------------------------------------------------------------*/

void CancelListForSet(void)
{
 if (!((1 << ListSETPC) && listStatus))
  CancelLineList() ;
} /* EndCancelListForSet */

/*---------------------------------------------------------------------------*/

static char mnoteLine[MaxLineLength] ;

/*---------------------------------------------------------------------------*/

static BOOLEAN DefineProgramLabel(Name name,SymbolPointer *symbolPointer)
{
 *symbolPointer = LookupFixed(name,FALSE) ;

 if ((*symbolPointer == NULL) || ((*symbolPointer)->u.s.sds != UndefinedSDS))
  {
   Warning(MulDefSym) ;
   return(TRUE) ;
  } ; /* if */

 AddDef(*symbolPointer) ;

 /* Symbol is defined and label type and relocatable */
 (*symbolPointer)->u.s.fst = RelocatableFST ;
 (*symbolPointer)->u.s.sds = DefinedSDS ;
 (*symbolPointer)->u.s.sdt = FixedSDT ;
 (*symbolPointer)->length = 0 ;
 (*symbolPointer)->value.card = programCounter ;
 return(FALSE) ;
} /* EndDefineProgramLabel */

/*---------------------------------------------------------------------------*/

static BOOLEAN DefineModuleLabel(Name name,SymbolPointer *symbolPointer)
{
 char *label = mymalloc(name.length + 2) ;
 Name  nname ;

 *label = 128 ;
 memcpy((label + 1),name.key,name.length) ;
 *(label + 1 + name.length) = '\0' ;
 nname.key = label ;
 nname.length = strlen(label) ;

 *symbolPointer = LookupFixed(nname,FALSE) ;

 if ((*symbolPointer == NULL) || ((*symbolPointer)->u.s.sds != UndefinedSDS))
  {
   free(label) ;
   Warning(MulDefSym) ;
   return(TRUE) ;
  } ; /* if */

 AddDef(*symbolPointer) ;

 /* Symbol is defined and module static data area label type */
 (*symbolPointer)->u.s.fst = ModuleFST ;
 (*symbolPointer)->u.s.sds = DefinedSDS ;
 (*symbolPointer)->u.s.sdt = FixedSDT ;
 (*symbolPointer)->length = 0 ;
 (*symbolPointer)->value.card = 0 ;

 free(label) ;

 return(FALSE) ;
} /* EndDefineModuleLabel */

/*---------------------------------------------------------------------------*/
/* The returned value indicates error OR handled (i.e. was directive) */

BOOLEAN P1Directive(char *line,BOOLEAN *wasLink,BOOLEAN *passEnded,char **linkPointer)
{
 CARDINAL              lineIndex = 0 ;
 CARDINAL              i ;
 CARDINAL              j ;
 CARDINAL              value ;
 Name                  name ;
 Name                  string ;
 Name                  nname ;
 char                  nametext[MaxStringSize] ;
 SymbolPointer         eSymbolPointer ;
 SymbolPointer         symbolPointer ;
 BOOLEAN               symbolFound ;
 BOOLEAN               ifBool ;
 BOOLEAN               defined ;
 DirectiveNumber       directiveNumber ;
 OperandType           operandType ;
 StructureStackElement s ;
 NamePointer           namePointer ;

 nname.key = nametext ; /* this should be constant */

 symbolFound = SymbolTest(line,&lineIndex,&name) ;
 if (line[lineIndex] != Space)
  {
   /* terminating colon allowed in As style, but only for program labels */
   if (symbolFound)
    return FALSE ;
   else
    {
     /* May be local label in as style */
     if (isdigit(*line))
      {
       (void)DecimalNumber(line,&lineIndex) ;
       /* No it's not! */
       return FALSE ;
      }
     else
      return FALSE ;
    }
  }

 while (line[++lineIndex] == Space) ;

 /* Now we should be pointing at the directive name */
 if (OneCharacterDirective(line,&lineIndex,&directiveNumber) || (DirTest(line,&lineIndex,&string) && NameDirective(&directiveNumber,string)))
  {
   /* Here we've spotted a directive */
   if (directiveNumber == -1)
    return FALSE ; /* . is a bad directive in established ObjAsm style */
  }
 else
  return FALSE ;

 if (((rejectedIFs == 0) && (rejectedWHILEs == 0)) || allowDirInCond[directiveNumber])
  {
   if (!DirectiveSyntax(directiveNumber,line[lineIndex],symbolFound))
    return TRUE ;
  }
 else
  return TRUE ; /* Pretend we've handled it if conditionally ignored */

 switch (directiveNumber)
  {
   case TIF     : s.type = ConditionalSSET ;
                  s.u.state = listStatus ; /* Stack listing status */
                  if (!Stack(s))
                   break ;
                  if ((rejectedIFs != 0) || (rejectedWHILEs != 0))
                   {
                    rejectedIFs++ ;
                    while (line[lineIndex] != CR)
                     lineIndex++ ;
                   }
                  else
                   {
                    if (LogicalExpr(line,&lineIndex))
                     includedIFs++ ;
                    else
                     {
                      rejectedIFs = 1 ;
                      if (terseState)
                       nextListState &= ~(1 << ListPC) ;
                     }
                   }
                  CancelListForCond() ;
                  break ;

   case TELSE   : if (!Unstack(&s))
                   break ;
                  if (s.type != ConditionalSSET)
                   {
                    if (s.type != WhileSSET)
                     Stack(s) ;
                    Warning(StructErr) ;
                    exception = StackErr ;
                    break ;
                   }
                  Stack(s) ; /* put it back */
                  if (rejectedIFs == 0)
                   {
                    rejectedIFs = 1 ;
                    includedIFs-- ;
                    if (terseState)
                     nextListState &= ~(1 << ListPC) ;
                   }
                  else
                   if (rejectedIFs == 1)
                    {
                     includedIFs++ ;
                     rejectedIFs = 0 ;
                     if ((1 << ListPC) & s.u.state)
                      nextListState |= (1 << ListPC) ;
                    }
                  CancelListForCond();
                  break ;

   case TFI     : if (!Unstack(&s))
                   break ;
                  if (s.type != ConditionalSSET)
                   {
                    if (s.type != WhileSSET)
                     Stack(s) ;
                    Warning(StructErr) ;
                    exception = StackErr ;
                    break ;
                   }
                  nextListState = s.u.state ;
                  if (((1 << ListPC) & nextListState) && !((1 << ListPC) & listStatus))
                   {
                    listStatus |= (1 << ListPC) ;
                    ListLineNumber() ;
                   }
                  if (rejectedIFs != 0)
                   rejectedIFs-- ;
                  else
                   includedIFs-- ;
                  CancelListForCond() ;
                  break ;

   case TMNOTE  : ListLine() ;
                  value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                  if (errorFound)
                   return TRUE ;
                  if (line[lineIndex] != Comma)
                   {
                    Warning(CommaMissing) ;
                    return TRUE ;
                   }
                  lineIndex++ ;
                  StringExpr(line,&lineIndex,&name) ;
                  memcpy(mnoteLine,name.key,name.length) ;
                  mnoteLine[name.length] = 0 ;
                  if (value != 0)
#ifdef Cstyle
		   NotifyReport(mnoteLine) ;
#else
                   WarningReport(mnoteLine) ;
#endif
                  break ;

   case THASH   : if (symbolFound)
                   {
                    symbolPointer = LookupRef(name,FALSE) ;
                    if (symbolPointer != NULL)
                     {
                      /* Symbol already exists, so better be PCRelative */
                      if (symbolPointer->u.s.sdt != FixedSDT)
                       {
                        Warning(BadSymType) ;
                        return TRUE ;
                       }
                      if (symbolPointer->u.s.sds != UndefinedSDS)
                       {
                        Warning(MulDefSym) ;
                        return TRUE ;
                       }
                     }
                    else
                     symbolPointer = LookupFixed(name,FALSE) ; /* insert the symbol */
                    AddDef(symbolPointer) ;
                    switch (variableCounter.type)
                     {
                      case FixedVCT    : symbolPointer->u.s.fst = AbsoluteFST ;
                                         symbolPointer->value.card = variableCounter.u.offset ;
                                         break ;

                      case RelativeVCT : if (variableCounter.u.relativeVC.reg == 15)
                                          symbolPointer->u.s.fst = RelocatableFST ;
                                         else
                                          {
                                           symbolPointer->u.s.fst = RegisterRelativeFST ;
                                           symbolPointer->u.s.fsr = variableCounter.u.relativeVC.reg ;
                                          }
                                         symbolPointer->value.card = variableCounter.u.relativeVC.offset ;

                     } /* switch */
                    symbolPointer->u.s.sds = DefinedSDS ;
                   }
                  value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                  if (errorFound)
                   return TRUE ;
                  if (symbolFound)
                   symbolPointer->length = value ;
                  switch (variableCounter.type)
                   {
                    case FixedVCT    : ListWordValue(variableCounter.u.offset) ;
                                       variableCounter.u.offset += value ;
                                       break ;

                    case RelativeVCT : ListWordValue(variableCounter.u.relativeVC.offset) ;
                                       variableCounter.u.relativeVC.offset += value ;

                   } /* switch */
                  break ;

   case TSTAR   : value = NotStringExpr(line,&lineIndex,&i,&operandType,TRUE,&defined) ;
                  if (errorFound)
                   return TRUE ;
                  symbolPointer = LookupRef(name,FALSE) ;
                  if (symbolPointer == NULL)
                   symbolPointer = LookupFixed(name,FALSE) ;
                  AddDef(symbolPointer) ;
                  if (symbolPointer->u.s.sds != UndefinedSDS)
                   {
                    Warning(MulDefSym) ;
                    return TRUE ;
                   }
                  if (symbolPointer->u.s.sdt != FixedSDT)
                   {
                    Warning(BadSymType) ;
                    return TRUE ;
                   }
                  if (defined)
                   {
                    switch (operandType)
                     {
                      case ConstantOT : symbolPointer->u.s.fst = AbsoluteFST ;
                                        break ;

                      case PCRelOT    : symbolPointer->u.s.fst = RelocatableFST ;
                                        break ;

                      case RegRelOT   : symbolPointer->u.s.fst = RegisterRelativeFST ;
                                        symbolPointer->u.s.fsr = i ;
                     } /* switch */
                    symbolPointer->u.s.sds = DefinedSDS ;
                    symbolPointer->value.card = value ;
                    ListWordValue(value) ;
                   }
                  else
                   symbolPointer->u.s.sds = PartDefinedSDS ;
                  symbolPointer->length = 0 ;
                  break ;

   case TEQUAL  : if (symbolFound && DefineProgramLabel(name,&symbolPointer))
                   return TRUE ;
                  i = programCounter ;
                  do
                   {
                    value = StringOrConstantExpr(line,&lineIndex,TRUE,&operandType) ;
                    if (errorFound)
                     return TRUE ;
                    switch (operandType)
                     {
                      case ConstantOT : programCounter++ ;
                                        break ;

                      case StringOT   : namePointer = (NamePointer)value ;
                                        programCounter += namePointer->length ;
                     } /* switch */
                    if (line[lineIndex] != Comma)
                     break ;
                    lineIndex++ ;
                   } while (1) ; /* loop */
                  if (symbolFound)
                   symbolPointer->length = programCounter - i ;
                  break ;

   case TPERC   : if (symbolFound && DefineProgramLabel(name,&symbolPointer))
                   return TRUE ;
                  i = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                  programCounter += i ;
                  if (symbolFound)
                   symbolPointer->length = i ;
                  break ;

   case TDCW    : if (programCounter & 1)
                   programCounter++ ;
                  if (symbolFound && DefineProgramLabel(name,&symbolPointer))
                   return TRUE ;
                  i = programCounter ;
                  /* Now handle value to be output */
                  do
                   {
                    value = ConstantExpr(line,&lineIndex,TRUE,&defined,NULL) ;
                    programCounter += 2 ;
                    if (errorFound || (line[lineIndex] != Comma))
                     break ;
                    lineIndex++ ;
                   } while (1) ; /* loop */
                  if (symbolFound)
                   symbolPointer->length = programCounter - i ;
                  break ;

   case TAMP    : while ((programCounter % 4) != 0)
                   programCounter++ ;
                  if (symbolFound && DefineProgramLabel(name,&symbolPointer))
                   return TRUE ;
                  i = programCounter ;
                  /* Now handle value to be output */
                  do
                   {
                    eSymbolPointer = ExternalExpr(line,&lineIndex,&value) ;
                    if (eSymbolPointer == NULL)
                     value = ConstantOrAddressExpr(line,&lineIndex,&operandType,TRUE,&defined) ;
                    programCounter += 4 ;
                    if (errorFound || (line[lineIndex] != Comma))
                     break ;
                    lineIndex++ ;
                   } while (1) ;
                  if (symbolFound)
                   symbolPointer->length = programCounter - i ;
                  break ;

   case THAT    : value = ConstantOrAddressExpr(line,&lineIndex,&operandType,FALSE,&defined) ;
                  if (errorFound)
                   return TRUE ;
                  switch (operandType)
                   {
                    case ConstantOT : if (line[lineIndex] == Comma)
                                       {
                                        lineIndex++ ;
                                        i = RegisterExpr(line,&lineIndex) ;
                                        if (errorFound)
                                         return TRUE ;
                                        variableCounter.type = RelativeVCT ;
                                        variableCounter.u.relativeVC.reg = i ;
                                        variableCounter.u.relativeVC.offset = value ;
                                       }
                                      else
                                       {
                                        variableCounter.type = FixedVCT ;
                                        variableCounter.u.offset = value ;
                                       }
                                      break ;

                    case PCRelOT    :
                                      variableCounter.type = RelativeVCT ;
                                      variableCounter.u.relativeVC.reg = 15 ;
                                      variableCounter.u.relativeVC.offset = value ;
                   } /* switch */
                  break ;

   case TEND    : CheckStack() ;
                  *passEnded = TRUE ;
                  break ;

   case TLNK    : CheckStack() ;
                  *wasLink = TRUE ;
                  *linkPointer = line + lineIndex ;
                  if (!exception)
                   while (!TermCheck(line[lineIndex]))
                    lineIndex++ ;
                  break ;

   case TGET    : GetDir(line,&lineIndex) ;
                  if (!exception)
                   return TRUE ;
                  break ;

#if 1 /* binary include support */
   case TBGET   : /* Include a binary data file into the code */
                  {
		   CARDINAL  index = 0 ;
		   char     *pointer = (char *)(line + lineIndex) ;
		   char      getname[MaxLineLength + 1] ;
		   FILE     *getStream = NULL ;
		   CARDINAL  bgetsize = 0 ;

		   while ((pointer[index] != Space) && (pointer[index] != CR))
		    {
		     getname[index] = pointer[index] ; /* copy the chars */
		     index++ ;
		    }
		   getname[index] = '\0' ; /* terminate the copy */
		   /* check that the file exists */
		   getStream = fopen(getname,"r") ;
		   if ((getStream == NULL) || ferror(getStream))
		    {
		     WarningReport("could not be opened") ;
		     exception = FileNotFound ;
		    }
		   else
		    {
		     /* discover its size */
		     fseek(getStream,0,SEEK_END) ;
		     bgetsize = (CARDINAL)ftell(getStream) ;
		     /* seek back to the start of the file */
		     fseek(getStream,0,SEEK_SET) ;
		     fclose(getStream) ;
		     /* increment the PC by the size of the file */
		     programCounter += bgetsize ;
		    }

		   if (!exception)
		    return(TRUE) ;
                  }
                  break ;
#endif

   case TOPT    : value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                  i = value / 4 ;
                  if ((i & 1) && ((1 << ListPC) & listStatus))
                   {
                    ListLine() ;
                    PageThrow() ;
                   }
                  if ((i / 2) & 1)
                   lineNumber = 0 ;
                  i = i / 4 ;
                  if (i % 4 == 1)
                   nextListState |= 1 << ListSETPC ;
                  else
                   if ((i % 4) == 2)
                    nextListState &= ~(1 << ListSETPC) ;
                  i = i / 4 ;
                  if ((i % 4) == 1)
                   nextListState |= 1 << ListMacExpPC ;
                  else
                   if ((i % 4) == 2)
                    nextListState &= ~(1 << ListMacExpPC) ;
                  i = i / 4 ;
                  if ((i % 4) == 1)
                   nextListState |= 1 << ListMacCallPC ;
                  else
                   if ((i % 4) == 2)
                    nextListState &= ~(1 << ListMacCallPC) ;
                  i = i / 4 ;
                  if ((i % 4) == 1)
                   nextListState |= 1 << ListPC ;
                  else
                   if ((i % 4) == 2)
                    nextListState &= ~(1 << ListPC) ;
                  i = i / 4 ;
                  if ((i % 4) == 1)
                   nextListState |= 1 << ListCondPC ;
                  else
                   if ((i % 4) == 2)
                    nextListState &= ~(1 << ListCondPC) ;
                  i = i / 4 ;
                  if ((i % 4) == 1)
                   nextListState |= 1 << ListMendPC ;
                  else
                   if ((i % 4) == 2)
                    nextListState &= ~(1 << ListMendPC) ;
                  i = i / 4 ;
                  if ((i % 4) == 1)
                   nextListState |= 1 << ListOptsPC ;
                  else
                   if ((i % 4) == 2)
                    nextListState &= ~(1 << ListOptsPC) ;
                  if (!((1 << ListOptsPC) & listStatus))
                   CancelLineList() ;
                  break ;

   case TTTL    : while (line[lineIndex] == Space)
                   lineIndex++ ;
                  value = lineIndex ;
                  while (line[lineIndex] != CR)
                   lineIndex++ ;
                  name.length = lineIndex - value ;
                  name.key = line + value ;
                  SetTitle(name) ;
                  break ;

   case TSUBTTL : while (line[lineIndex] == Space)
                   lineIndex++ ;
                  value = lineIndex ;
                  while (line[lineIndex] != CR)
                   lineIndex++ ;
                  name.length = lineIndex - value ;
                  name.key = line + value ;
                  SetSubtitle(name) ;
                  break ;

   case TRN     : value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                  if (errorFound)
                   return TRUE ;
                  if (value >= 0x10)
                   {
                    Warning(RegRange) ;
                    return TRUE ;
                   }
                  symbolPointer = DefineReg(name) ;
                  if (symbolPointer == NULL)
                   {
                    Warning(RegSymExists) ;
                    return TRUE ;
                   }
                  AddDef(symbolPointer) ;
                  symbolPointer->value.card = value ;
                  ListRegValue(value) ;
                  break ;

   case TFN     : value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                  if (errorFound)
                   return TRUE ;
                  if (value >= 8)
                   {
                    Warning(RegRange) ;
                    return TRUE ;
                   }
                  symbolPointer = DefineFPReg(name) ;
                  if (symbolPointer == NULL)
                   {
                    Warning(RegSymExists) ;
                    return TRUE ;
                   }
                  AddDef(symbolPointer) ;
                  symbolPointer->value.card = value ;
                  ListRegValue(value) ;
                  break ;

   case TCN     : value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                  if (errorFound)
                   return TRUE ;
                  if (value >= 0x10)
                   {
                    Warning(RegRange) ;
                    return TRUE ;
                   }
                  symbolPointer = DefineCoprocReg(name) ;
                  if (symbolPointer == NULL)
                   {
                    Warning(RegSymExists) ;
                    return TRUE ;
                   }
                  AddDef(symbolPointer) ;
                  symbolPointer->value.card = value ;
                  ListRegValue(value) ;
                  break ;

   case TCP     : value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                  if (errorFound)
                   return TRUE ;
                  if (value >= 0x10)
                   {
                    Warning(RegRange) ;
                    return TRUE ;
                   }
                  symbolPointer = DefineCoprocName(name) ;
                  if (symbolPointer == NULL)
                   {
                    Warning(RegSymExists) ;
                    return TRUE ;
                   }
                  AddDef(symbolPointer) ;
                  symbolPointer->value.card = value ;
                  ListRegValue(value) ;
                  break ;

   case TWHILE  : s.type = WhileSSET ;
                  s.u.whileEl.lineNumber = lineNumber - 1 ;
                  s.u.whileEl.pointer = oldPointerInFile ;
                  s.u.whileEl.state = listStatus ;
                  if (!Stack(s))
                   break ;
                  if ((rejectedIFs != 0) || (rejectedWHILEs != 0))
                   {
                    while (line[lineIndex] != CR)
                     lineIndex++ ;
                    rejectedWHILEs++ ;
                   }
                  else
                   {
                    if (LogicalExpr(line,&lineIndex))
                     includedWHILEs++ ;
                    else
                     {
                      rejectedWHILEs = 1 ;
                      if (terseState)
                       nextListState &= ~(1 << ListPC) ;
                     }
                   }
                  CancelListForCond() ;
                  break ;

   case TWEND   : WendDir() ;
                  if (exception)
                   break ;
                  CancelListForCond() ;
                  break ;

   case TMACRO  : if (macroLevel != 0)
                   AssemblerError("MACRO definition attempted within expansion") ;
                  if ((rejectedIFs != 0) || (rejectedWHILEs != 0))
                   IgnoreMacroDefinition() ;
                  else
                   DefineMacro() ;
                  if (!exception)
                   return TRUE ;
                  /* Make sure exceptions handled before returning */
                  break ;

   case TMEXIT  : if (MexitDir() && !exception)
                   return TRUE ;
                  break ;

   case TMEND   : if (MendDir() && !exception)
                   return TRUE ;
                  break ;

   case TGBLA   : if (!SymbolTest(line,&lineIndex,&name))
                   {
                    Warning(BadGlob) ;
                    return TRUE ;
                   }
                  symbolPointer = DefineGlobalA(name) ;
                  if (symbolPointer == NULL)
                   {
                    Warning(GlobExists) ;
                    return TRUE ;
                   }
                  CancelListForSet() ;
                  break ;

   case TGBLL   : if (!SymbolTest(line,&lineIndex,&name))
                   {
                    Warning(BadGlob) ;
                    return TRUE ;
                   }
                  symbolPointer = DefineGlobalL(name) ;
                  if (symbolPointer == NULL)
                   {
                    Warning(GlobExists) ;
                    return TRUE ;
                   }
                  CancelListForSet() ;
                  break ;

   case TGBLS   : if (!SymbolTest(line,&lineIndex,&name))
                   {
                    Warning(BadGlob) ;
                    return TRUE ;
                   }
                  symbolPointer = DefineGlobalS(name) ;
                  if (symbolPointer == NULL)
                   {
                    Warning(GlobExists) ;
                    return TRUE ;
                   }
                  CancelListForSet() ;
                  break ;

   case TLCLA   : if (macroLevel == 0)
                   {
                    Warning(LocNotAllowed) ;
                    return TRUE ;
                   }
                  if (!SymbolTest(line,&lineIndex,&name))
                   {
                    Warning(BadLoc) ;
                    return TRUE ;
                   }
                  symbolPointer = DefineLocalA(name) ;
                  if (symbolPointer == NULL)
                   {
                    Warning(LocExists) ;
                    return TRUE ;
                   }
                  CancelListForSet() ;
                  break ;

   case TLCLL   : if (macroLevel == 0)
                   {
                    Warning(LocNotAllowed) ;
                    return TRUE ;
                   }
                  if (!SymbolTest(line,&lineIndex,&name))
                   {
                    Warning(BadLoc) ;
                    return TRUE ;
                   }
                  symbolPointer = DefineLocalL(name) ;
                  if (symbolPointer == NULL)
                   {
                    Warning(LocExists) ;
                    return TRUE ;
                   }
                  CancelListForSet() ;
                  break ;

   case TLCLS   : if (macroLevel == 0)
                   {
                    Warning(LocNotAllowed) ;
                    return TRUE ;
                   }
                  if (!SymbolTest(line,&lineIndex,&name))
                   {
                    Warning(BadLoc) ;
                    return TRUE ;
                   }
                  symbolPointer = DefineLocalS(name) ;
                  if (symbolPointer == NULL)
                   {
                    Warning(LocExists) ;
                    return TRUE ;
                   }
                  CancelListForSet() ;
                  break ;

   case TSETA   : value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                  if (errorFound)
                   return TRUE ;
                  symbolPointer = LookupLocalA(name) ;
                  if (symbolPointer == NULL)
                   symbolPointer = LookupGlobalA(name) ;
                  if (symbolPointer == NULL)
                   {
                    Warning(WrongSy) ;
                    return TRUE ;
                   } ;
                  symbolPointer->value.card = value ;
                  ListWordValue(value) ;
                  CancelListForSet() ;
                  break ;

   case TSETL   : ifBool = LogicalExpr(line,&lineIndex) ;
                  if (errorFound)
                   return TRUE ;
                  symbolPointer = LookupLocalL(name) ;
                  if (symbolPointer == NULL)
                   symbolPointer = LookupGlobalL(name) ;
                  if (symbolPointer == NULL)
                   {
                    Warning(WrongSy) ;
                    return TRUE ;
                   }
                  symbolPointer->value.bool = ifBool ;
                  ListBoolValue(ifBool) ;
                  CancelListForSet() ;
                  break ;

   case TSETS   : StringExpr(line,&lineIndex,&string) ;
                  if (errorFound)
                   return TRUE ;
                  symbolPointer = LookupLocalS(name) ;
                  if (symbolPointer == NULL)
                   symbolPointer = LookupGlobalS(name) ;
                  if (symbolPointer == NULL)
                   {
                    Warning(WrongSy) ;
                    return TRUE ;
                   }
                  ListStringValue(string) ;
                  symbolPointer->value.ptr->length = string.length ;
                  if (string.length > symbolPointer->value.ptr->maxLength)
                   {
                    if (symbolPointer->value.ptr->maxLength > 0)
                     free(symbolPointer->value.ptr->key) ;
                    symbolPointer->value.ptr->key = mymalloc(string.length) ;
                    symbolPointer->value.ptr->maxLength = symbolPointer->value.ptr->length ;
                   }
                  if (string.length > 0)
                   memcpy(symbolPointer->value.ptr->key,string.key,string.length) ;
                  CancelListForSet() ;
                  break ;

   case TASSERT : if ((!AssertExpr(line,&lineIndex,TRUE,&defined)) && defined)
                   {
                    WarningReport("Assertion failed") ;
                    exception = FileNotFound ;
                   }
                  break ;

   case TROUT   : while ((programCounter % 4) != 0)
                   programCounter++ ;
                  if (symbolFound)
                   {
                    symbolPointer = LookupFixed(name,FALSE) ;
                    if (symbolPointer == NULL)
                     {
                      Warning(MulDefSym) ;
                      return TRUE ;
                     }
                    if (DefineProgramLabel(name,&symbolPointer))
                     return TRUE ;
                   }
                  else
                   symbolPointer = NULL ;
                  NewRoutine(symbolPointer) ;
                  break ;

   case TALIGN  : if (AllComment(line,&lineIndex))
                   value = 4 ;
                  else
                   {
                    value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                    if (errorFound)
                     return TRUE ;
                   }

                  if (line[lineIndex] == Comma)
                   {
                    lineIndex++ ;
                    j = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                    if (errorFound)
                     return TRUE ;
                   }
                  else
                   j = 0 ;

                  i = value ;

                  if (value != 0)
                   while ((value % 2) == 0)
                    value /= 2 ;

                  if (value != 1)
                   {
                    Warning(BadAlign) ;
                    return TRUE ;
                   }
                  /* Now align */
                  programCounter += (j - programCounter) % i ;
                  break ;

   case TLTORG  : LiteralOrg() ; /* Set up the origin */
                  break ;

   case TIMPORT : if (!SymbolTest(line,&lineIndex,&name))
                   Warning(BadImport) ;
                  else
                   {
                    if (AllComment(line,&lineIndex))
                     ifBool = FALSE ;	/* no attributes */
                    else
                     if (line[lineIndex] != Comma)
                      {
                       Warning(CommaMissing) ;
                       return TRUE ;
                      }
                     else
                      {
                       lineIndex++ ;
		       /* Deal with the "EXCEPTION" attribute */
                       if (AllComment(line,&lineIndex) || (memcmp(line+lineIndex,"EXCEPTION",9) != 0) || !AllComment(line+9,&lineIndex))
                        {
                         Warning(BadIMAttr) ;
                         return(TRUE) ;
                        }
                       lineIndex += 9 ;
                       ifBool = TRUE ;
                      } 

                    /* We should always IMPORT the "_" preceded name */
                    nametext[0] = 128 ;
                    memcpy(&nametext[1],name.key,name.length) ;
                    nametext[1 + name.length] = '\0' ;
                    nname.length = strlen(nametext) ;
                    symbolPointer = LookupRef(nname,FALSE) ;
                    if ((symbolPointer != NULL))
                     {
                      if (symbolPointer->u.s.sdt != ExternalSDT)
                       {
                        Warning(ImportExists) ;
                        return TRUE ;
                       }
                     }
                    else
                     symbolPointer = LookupExternal(nname) ;

                    AddDef(symbolPointer) ;
                    if (symbolPointer->aOFData.symbolId == 0x8000)
                     {
                      symbolPointer->value.bool = FALSE ;
                      AddSymbol(symbolPointer) ;
                     }
                    else
                     symbolPointer->value.bool = FALSE ;
                   }

                  /* We should generate a STUB function for each IMPORTed symbol.
                   * These STUB functions are called if a branch is made to an externally
                   * IMPORTed symbol.
                   * To simplify the assembler modification process:
                   *  When working internally we will always use the REAL symbol
                   *  name to reference the code. We will use a "_" prefixed name
                   *  to reference symbols in the module table, or IMPORTed symbols.
                   *  We will use "." to prefix automatically created STUB functions.
                   *  When writing the object file however we should prefix "." to
                   *  all the REAL symbols, and remove the "." from the STUB labels.
                   *  This is done to stay compatible with the rest of the Helios
                   *  build world (basically the C compiler).
                   */
                  if ((!librarycode) && (!clmake_def))
		   {
		    if (ifBool)
		     AddStub(name,TRUE) ;	/* exception stub */
		    else
                     AddStub(name,FALSE) ;	/* normal stub */
		   }
                  break ;

   case TLABREF  : if (!SymbolTest(line,&lineIndex,&name))
                   Warning(BadImport) ;
                  else
                   {
		    /* LABREF has no attributes after the symbol */
		    if (!AllComment(line,&lineIndex))
		     {
                      Warning(BadIMAttr) ;
                      return(TRUE) ;
		     }

                    /* We should always IMPORT the "_" preceded name */
                    nametext[0] = 128 ;
                    memcpy(&nametext[1],name.key,name.length) ;
                    nametext[1 + name.length] = '\0' ;
                    nname.length = strlen(nametext) ;
                    symbolPointer = LookupRef(nname,FALSE) ;
                    if ((symbolPointer != NULL))
                     {
                      if (symbolPointer->u.s.sdt != ExternalSDT)
                       {
                        Warning(ImportExists) ;
                        return TRUE ;
                       }
                     }
                    else
                     symbolPointer = LookupExternal(nname) ;

                    AddDef(symbolPointer) ;
                    if (symbolPointer->aOFData.symbolId == 0x8000)
                     {
                      symbolPointer->value.bool = FALSE ;
                      AddSymbol(symbolPointer) ;
                     }
                    else
                     symbolPointer->value.bool = FALSE ;
                   }
                  break ;

   case TEXPORT : for (;;)
                   {
                    if (!SymbolTest(line,&lineIndex,&name))
                     Warning(BadExport) ;
                    else
                     {
                      nametext[0] = 128 ;
                      memcpy(&nametext[1],name.key,name.length) ;
                      nametext[1 + name.length] = '\0' ;
                      nname.length = strlen(nametext) ;

                      symbolPointer = LookupRef(nname,FALSE) ;
                      if (symbolPointer == NULL)
                       symbolPointer = LookupFixed(nname,FALSE) ;
                      AddUse(symbolPointer) ;
                      if ((symbolPointer->u.s.sdt != FixedSDT) || (symbolPointer->u.s.fst == RegisterRelativeFST))
                       Warning(BadExportType) ;
                      else
                       {
                        if (symbolPointer->aOFData.symbolId == 0x8000)
                         AddSymbol(symbolPointer) ;

                        switch (symbolPointer->u.s.at)
                         {
                          case HDataAT : symbolPointer->u.s.at = HDataExportAT ;
                                         break ;
                          case HCodeAT : symbolPointer->u.s.at = HCodeExportAT ;
                                         break ;
                          default      : symbolPointer->u.s.at = ExportAT ;
                                         break ;
                         }
                       }
                     }
                    if (AllComment(line,&lineIndex))
                     break ;
                    if (line[lineIndex++] != Comma)
                     {
                      Warning(CommaMissing) ;
                      return TRUE ;
                     } ;
                    (void)AllComment(line,&lineIndex) ;
                   } /* for */
                  break ;

   case TEXTERN : if (!AllComment(line,&lineIndex))
                   Warning(BadExtern) ;
                  else
                   {
#ifdef DEBUG
                    printf("**DEBUG** EXTERN \"") ;
                    PrintSymbol(name) ;
                    printf("\"\n") ;
#endif

                    /* Define a pure local version (with "|" bars that will
                     * be removed by the object file generator.
                     */
                    nametext[0] = '|' ;
                    memcpy(&nametext[1],name.key,name.length) ;
                    nametext[1 + name.length] = '|' ;
                    nametext[2 + name.length] = '\0' ;
                    nname.length = strlen(nametext) ;
                    if (DefineProgramLabel(nname,&symbolPointer))
                     return(TRUE) ;

                    symbolPointer = LookupRef(nname,FALSE) ;
                    if (symbolPointer == NULL)
                     symbolPointer = LookupFixed(nname,FALSE) ;

#ifdef DEBUG
                    printf("**DEBUG** EXTERN: symbolPointer = &%08X\n",(int)symbolPointer) ;
#endif /* DEBUG */
                    AddUse(symbolPointer) ;

                    if (symbolPointer->u.s.sdt != FixedSDT)
                     Warning(BadExternType) ;
                    else
                     {
                      if (symbolPointer->aOFData.symbolId == 0x8000)
                       AddSymbol(symbolPointer) ;

#ifdef DEBUG
                      printf("**DEBUG** EXTERN: setting \"ExportAT\"\n") ;
#endif

                      symbolPointer->u.s.at = ExportAT ;
                     }
                   }
                  break ;

   case TDCFS   : while ((programCounter % 4) != 0)
                   programCounter++ ;
                  if (symbolFound && DefineProgramLabel(name,&symbolPointer))
                   return TRUE ;
                  i = programCounter ;
                  /* Now handle value to be output */
                  do
                   {
                    switch (hRead(line,&lineIndex,Single,&value,&value))
                     {
                      case ReadOverflow  : Warning(FPOver) ;
                                           break ;

                      case NoNumberFound : Warning(FPNoNum) ;
                     } /* switch */
                    programCounter += 4 ;
                    while (line[lineIndex] == Space)
                     lineIndex++ ;
                    if (errorFound || (line[lineIndex] != Comma))
                     break ;
                    lineIndex++ ;
                   } while (1) ;
                  if (symbolFound)
                   symbolPointer->length = programCounter - i ;
                  break ;

   case TDCFD   : while ((programCounter % 4) != 0)
                   programCounter++ ;
                  if (symbolFound && DefineProgramLabel(name,&symbolPointer))
                   return TRUE ;
                  i = programCounter ;
                  /* Now handle value to be output */
                  do
                   {
                    switch (hRead(line,&lineIndex,Double,&value,&value))
                     {
                      case ReadOverflow  : Warning(FPOver) ;
                                           break ;

                      case NoNumberFound : Warning(FPNoNum) ;
                     } /* switch */
                    programCounter += 8 ;
                    while (line[lineIndex] == Space)
                     lineIndex++ ;
                    if (errorFound || (line[lineIndex] != Comma))
                     break ;
                    lineIndex++ ;
                   } while (1) ;
                  if (symbolFound)
                   symbolPointer->length = programCounter - i ;
                  break ;

   case TNOFP   : if (hadFP)
                   {
                    Warning(FPTooLate) ;
                    return TRUE ;
                   }
                  allowFP = FALSE ;
                  break ;
    
   case TMODULE : /* construct a module header */
                  /* "        MODULE <modnum>" */
                  heliosHdr.helios_module = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                  if (errorFound)
                   return(TRUE) ;

                  programCounter = 0 ;
                  break ;

   case TLIB    : /* Allow LABELREFs to be generated for undefined symbols */
                  allowUndefinedSymbols = TRUE ;
                  break ;

   case TDATA   : /* Define a named segment in the modules data area */
                  /* "        DATA symbol,size" */
                  if (!SymbolTest(line,&lineIndex,&name))
                   Warning(SKSymMissing) ;
                  else
                   {
                    /* We should define this symbol with type "HDataAT".
                     * Only certain instructions/directives will be able
                     * to use these symbols.
                     */
                    if (DefineModuleLabel(name,&symbolPointer))
                     return(TRUE) ;

                    switch (symbolPointer->u.s.at)
                     {
                      default         : symbolPointer->u.s.at = HDataAT ;
                                        break ;
                      case ExportAT   : symbolPointer->u.s.at = HDataExportAT ;
                                        break ;
                      case ExportedAT : symbolPointer->u.s.at = HDataExportedAT ;
                                        break ;
                     }

                    while (line[lineIndex] == Space)
                     lineIndex++ ;      /* step over spaces */
                    if (line[lineIndex] == Comma)
                     {
                      lineIndex++ ;     /* step over the comma */
                      j = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                      if (errorFound)
                       return(TRUE) ;
                      symbolPointer->value.card = j ; /* size of this entry */
                     }
                    else
                     Warning(SKSizeMissing) ;
                   }
                  break ;

   case TCODE   : /* Define a named segment in the modules code area */
                  /* "        CODE symbol" */
                  if (!SymbolTest(line,&lineIndex,&name))
                   Warning(SKSymMissing) ;

                  /* We should define this symbol with type "HCodeAT".
                   * Only certain instructions/directives will be able
                   * to use these symbols.
                   */
                  if (DefineModuleLabel(name,&symbolPointer))
                   return(TRUE) ;

                  switch (symbolPointer->u.s.at)
                   {
                    default         : symbolPointer->u.s.at = HCodeAT ;
                                      break ;
                    case ExportAT   : symbolPointer->u.s.at = HCodeExportAT ;
                                      break ;
                    case ExportedAT : symbolPointer->u.s.at = HCodeExportedAT ;
                                      break ;
                   }

                  /* function pointers are a fixed size */
                  symbolPointer->value.card = 4 ;
                  break ;

   case TCOMMON : /* Define a shared named segment in the modules data area */
                  /* "        COMMON symbol,size" */
                  if (!SymbolTest(line,&lineIndex,&name))
                   Warning(SKSymMissing) ;
                  else
                   {
                    /* We should define this symbol with type "HDataAT".
                     * Only certain instructions/directives will be able
                     * to use these symbols.
                     */
                    if (DefineModuleLabel(name,&symbolPointer))
                     return(TRUE) ;

                    switch (symbolPointer->u.s.at)
                     {
                      default         : symbolPointer->u.s.at = HDataAT ;
                                        break ;
                      case ExportAT   : symbolPointer->u.s.at = HDataExportAT ;
                                        break ;
                      case ExportedAT : symbolPointer->u.s.at = HDataExportedAT ;
                                        break ;
                     }
                    while (line[lineIndex] == Space)
                     lineIndex++ ;      /* step over spaces */
                    if (line[lineIndex] == Comma)
                     {
                      lineIndex++ ;     /* step over the comma */
                      j = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                      if (errorFound)
                       return(TRUE) ;
                      symbolPointer->value.card = j ; /* size of a entry */
                     }
                    else
                     Warning(SKSizeMissing) ;
                   }
                  break ;

   case TINIT   : /* Create a link in the initialisation chain */
                  /* "        INIT" */
                  while ((programCounter % 4) != 0) /* word-align value */
                   programCounter++ ;
                  if (symbolFound && DefineProgramLabel(name,&symbolPointer))
                   return(TRUE) ;
                  i = programCounter ;
                  /* now handle the value to be output */
                  /* Insert a word that will eventually hold the offset to
                   * the next "INIT" word.
                   */
                  programCounter += 4 ;
                  if (symbolFound)
                   symbolPointer->length = programCounter - i ;
                  break ;

   case TLABEL  : /* Create a "labelref" word patched by the linker */
                  /* "        LABEL <label>" */
                  while ((programCounter % 4) != 0) /* word-align value */
                   programCounter++ ;
                  if (symbolFound && DefineProgramLabel(name,&symbolPointer))
                   return(TRUE) ;
                  i = programCounter ;
                  /* now handle the value to be output */
                  /* Insert a word that will eventually contain a reference
                   * to the specified symbol.
                   */
                  programCounter += 4 ;
                  if (symbolFound)
                   symbolPointer->length = programCounter - i ;
                  /* deal with the <label> argument */
                  if (!SymbolTest(line,&lineIndex,&name))
                   Warning(SKSymMissing) ;
                  break ;

   case TIMSIZE : /* Create a word containing the complete module size */
                  /* "        IMSIZE" */
                  while ((programCounter % 4) != 0) /* word-align value */
                   programCounter++ ;
                  if (symbolFound && DefineProgramLabel(name,&symbolPointer))
                   return(TRUE) ;
                  i = programCounter ;
                  /* now handle the value to be output */
                  /* Insert a word that will eventually contain the size
                   * of the module (excluding header and trailer structures)
                   */
                  programCounter += 4 ;
                  if (symbolFound)
                   symbolPointer->length = programCounter - i ;
                  break ;

   case TMODNUM : /* Create a word containing the linked module number */
                  /* "         MODNUM [<label>]" */
                  while ((programCounter % 4) != 0) /* word-align value */
                   programCounter++ ;
                  if (symbolFound && DefineProgramLabel(name,&symbolPointer))
                   return(TRUE) ;
                  i = programCounter ;
                  /* now handle the value to be output */
                  /* Insert a word that will eventually hold the module
                   * number of this code or that of the specified symbol.
                   */
                  programCounter += 4 ;
                  if (symbolFound)
                   symbolPointer->length = programCounter - i ;

                  /* deal with the optional <label> argument */
                  (void)SymbolTest(line,&lineIndex,&name) ;
                  break ;

   case TMODOFF : /* Create a word containing the linked module offset */
                  /* "        MODOFF [<label>]" */
                  while ((programCounter % 4) != 0) /* word-align value */
                   programCounter++ ;
                  if (symbolFound && DefineProgramLabel(name,&symbolPointer))
                   return(TRUE) ;
                  i = programCounter ;
                  /* now handle the value to be output */
                  /* Insert a word that will eventually hold the module
                   * number of this code or the specified symbol.
                   */
                  programCounter += 4 ;
                  if (symbolFound)
                   symbolPointer->length = programCounter - i ;
                  /* deal with the optional <label> argument */
                  (void)SymbolTest(line,&lineIndex,&name) ;
                  break ;

   case TOFFSET : /* Create a word containing the data offset */
                  /* "        OFFSET <label>" */
                  while ((programCounter % 4) != 0) /* word-align value */
                   programCounter++ ;
                  if (symbolFound && DefineProgramLabel(name,&symbolPointer))
                   return(TRUE) ;
                  i = programCounter ;
                  /* now handle the value to be output */
                  /* Insert a word that will eventually hold the offset of
                   * the specified label within the static data area.
                   */
                  programCounter += 4 ;
                  if (symbolFound)
                   symbolPointer->length = programCounter - i ;
                  /* deal with the <label> argument */
                  if (!SymbolTest(line,&lineIndex,&name))
                   Warning(SKSymMissing) ;
                  break ;
  } /* switch */

 if (exception)
  {
   if ((exception == EndOfInput) || (exception == StackUnderflow))
    exception = FileNotFound ;
   else
    if ((exception == StackOverflow) || (exception == StackErr))
     {
      UnwindToGet() ;
      exception = FileNotFound ;
     } ;
   return TRUE ;
  } ;
 if (!errorFound)
  {
   if (!AllComment(line,&lineIndex))
    Warning(BadEOL) ;
  } ;
 return TRUE ;
} /* End P1Directive */

/*---------------------------------------------------------------------------*/
/*> EOF p1dirs/c <*/
