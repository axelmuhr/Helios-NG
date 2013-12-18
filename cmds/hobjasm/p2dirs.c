/* -> p2dirs/c
 * Title:               Directive handler for pass 2
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
#include "mactypes.h"
#include "nametype.h"
#include "asmvars.h"
#include "p1hand.h"
#include "p2dirs.h"
#include "store.h"
#include "tables.h"
#include "tokens.h"
#include "helios.h"
#include "osdepend.h"
#include "stubs.h"
#include "vars.h"
#include "occur.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*---------------------------------------------------------------------------*/

static char mnoteLine[MaxLineLength + 1] ;

/*---------------------------------------------------------------------------*/

static void CancelListForCond(void)
{
 if (!((1 << ListCondPC) & listStatus))
  CancelLineList() ;
} /* End CancelListForCond */

/*---------------------------------------------------------------------------*/

static void CancelListForSet(void)
{
 if (!((1 << ListSETPC) & listStatus))
  CancelLineList() ;
} /* End CancelListForSet */

/*---------------------------------------------------------------------------*/
/* The returned value indicates error OR handled (i.e. was directive) */
BOOLEAN P2Directive(char *line,BOOLEAN *wasLink,BOOLEAN *passEnded,char **linkPointer)
{
 CARDINAL              lineIndex ;
 CARDINAL              i ;
 CARDINAL              value ;
 Name                  name ;
 Name                  string ;
 Name                  nname ;
 char                  nametext[MaxStringSize] ;
 SymbolPointer         eSymbolPointer;
 SymbolPointer         symbolPointer;
 BOOLEAN               symbolFound ;
 BOOLEAN               ifBool ;
 BOOLEAN               defined ;
 DirectiveNumber       directiveNumber ;
 OperandType           operandType ;
 StructureStackElement s ;
 NamePointer           namePointer ;
 ReadResponse          readResponse ;

 nname.key = nametext ; /* this should be constant */

 lineIndex = 0 ;
 symbolFound = SymbolTest(line,&lineIndex,&name) ;
 if (line[lineIndex] != Space)
  {
   if (symbolFound)
    return(FALSE) ;
   else
    {
     /* May be local label in as style */
     if (isdigit(*line))
      {
       (void)DecimalNumber(line,&lineIndex) ;
       /* No it's not! */
       return(FALSE) ;
      }
     else
      return(FALSE) ;
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
   /* TRY */
   switch (directiveNumber)
    {
     case TIF     : s.type = ConditionalSSET ;
                    s.u.state = listStatus ; /* Stack listing state */
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
                        nextListState |= 1 << ListPC ;
                      }
                    Stack(s) ;
                    CancelListForCond() ;
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
                      listStatus |= 1 << ListPC ;
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
                    lineIndex++ ;
                    StringExpr(line,&lineIndex,&name) ;
                    i = 0 ;
                    memcpy(mnoteLine,name.key,name.length) ;
                    mnoteLine[name.length] = CR ;
                    mnoteLine[name.length + 1] = 0 ;
                    WarningChs(mnoteLine) ;
                    break ;

     case THASH   : value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                    switch (variableCounter.type)
                     {
                      case FixedVCT    : ListWordValue(variableCounter.u.offset) ;
                                         variableCounter.u.offset += value ;
                                         break ;

                      case RelativeVCT : ListWordValue(variableCounter.u.relativeVC.offset) ;
                                         variableCounter.u.relativeVC.offset += value ;
                     } /* switch */
                    break ;

     case TSTAR   : value = NotStringExpr(line,&lineIndex,&i,&operandType,FALSE,&defined) ;
                    if (errorFound)
                     return TRUE ;
                    ListWordValue(value) ;
                    symbolPointer = LookupRef(name,FALSE) ;
                    if (symbolPointer->u.s.sds == DefinedSDS)
                     return TRUE ;
                    if (errorFound)
                     {
                      symbolPointer->u.s.sds = UndefinableSDS ;
                      return TRUE ;
                     }
                    symbolPointer->value.card = value ;
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
                    symbolPointer->length = 0 ;
                    break ;

     case TEQUAL  : do
                     {
                      value = StringOrConstantExpr(line,&lineIndex,FALSE,&operandType) ;
                      if (errorFound)
                       return TRUE ;
                      switch (operandType)
                       {
                        case ConstantOT : if (value >= 0x100)
                                           {
                                            Warning(ImmValRange) ;
                                            return TRUE ;
                                           }
                                          CodeByte(value % 0x100) ;
                                          break ;

                        case StringOT   : namePointer = (NamePointer)value ;
                                          i = 0 ;
                                          while (i < namePointer->length)
                                           CodeByte(namePointer->key[i++]) ;
                       } /* switch */
                      if (line[lineIndex] != Comma)
                       break ;
                      lineIndex++ ;
                     } while (1) ; /* loop */
                    break ;

     case TPERC   : value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                    write_bss(value) ;
                    break ;

     case TDCW    : /* Now handle value to be output */
                    if (programCounter & 1)
                     CodeByte(0) ;
                    do
                     {
                      value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                      if (errorFound)
                       return TRUE ;
                      if (value >= 0x10000)
                       {
                        Warning(ImmValRange) ;
                        return TRUE ;
                       }
                      CodeByte(value % 0x100) ;
                      CodeByte((value / 0x100) % 0x100) ;
                      if (line[lineIndex] != Comma)
                       break ;
                      lineIndex++ ;
                     } while (1) ;
                    break ;

     case TAMP    : /* Now handle value to be output */
                    while ((programCounter % 4) != 0)
                     CodeByte(0) ;
                    do
                     {
                      eSymbolPointer = ExternalExpr(line,&lineIndex,&value) ;
                      if (eSymbolPointer == NULL)
                       {
                        value = ConstantOrAddressExpr(line,&lineIndex,&operandType,FALSE,&defined) ;
                        if (operandType == PCRelOT)
                         {
                          /* This is a problem... in that we are being asked
                           * to place the address of a PC relative symbol into
                           * store. Normally this is a link-time resolved
                           * feature. It produces NON-PIC. The Helios linker
                           * generates PIC only. We want PIC. The best
                           * compromise is to generate the offset from the
                           * store location to the desired address.
                           */

                          /* we do not want non-PIC code, so generate offset */
                          value -= programCounter ;
                         } ; /* if */
                       }
                      else
                       {
                        printf("TAMP: External symbol reference\n") ;
                        value = 0x00000000 ;
                       } ; /* if */
                      if (errorFound)
                       return(TRUE) ;
                      CodeWord(value) ;
                      if (line[lineIndex] != Comma)
                       break ;
                      lineIndex++ ;
                     } while (1) ;              /* loop until no more values */
                    break ;

     case THAT    : value = ConstantOrAddressExpr(line,&lineIndex,&operandType,FALSE,&defined) ;
                    switch (operandType)
                     {
                      case ConstantOT : if (line[lineIndex] == Comma)
                                         {
                                          lineIndex++ ;
                                          i = RegisterExpr(line,&lineIndex) ;
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

                      case PCRelOT    : variableCounter.type = RelativeVCT ;
                                        variableCounter.u.relativeVC.reg = 15 ;
                                        variableCounter.u.relativeVC.offset = value ;
                     } /* switch */
                    break ;

     case TEND    : *passEnded = TRUE ;
                    break ;

     case TLNK    : *wasLink = TRUE ;
                    *linkPointer = line + lineIndex ;
                    break ;

     case TGET    : GetDir(line,&lineIndex) ;
                    break ;

#if 1 /* binary include support */
     case TBGET   : /* Include a binary data file into the image */
                    {
		     CARDINAL  index = 0 ;
		     char     *pointer = (char *)(line + lineIndex) ;
		     char      getname[MaxLineLength + 1] ;
		     FILE     *getStream = NULL ;
		     CARDINAL  bgetsize = 0 ;

		     while ((pointer[index] != Space)&&(pointer[index] != CR))
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
		       int   bcount = 0 ;
		       int   cptr = 0 ;
		       char  buffer[32 * 1024] ;      /* local buffer */
		       int   transfer = (32 * 1024) ; /* transfer amount */

		       /* discover its size */
		       fseek(getStream,0,SEEK_END) ;
		       bgetsize = (CARDINAL)ftell(getStream) ;
		       /* seek back to the start of the file */
		       fseek(getStream,0,SEEK_SET) ;
		       /* Transfer binary data from the file to our object */
		       /* This could be optimised by providing an explicit
			* CodeBlock function.
			*/
		       cptr = bgetsize ;
		       while (cptr > 0)
		        {
			 int loop ;

			 /* check for buffer transfer less than buffer size */
			 if (cptr < transfer)
			  transfer = cptr ;
			 if (fread(buffer,transfer,1,getStream) != 1)
			  {
			   fprintf(stderr,"pass2: TBGET: failed to read bytes from stream\n") ;
			   exit(1) ;
			  }
			 for (loop = 0; (loop < transfer); loop++)
			  {
                           CodeByte(buffer[loop]) ;
			   bcount++ ;
			  }
			 cptr -= transfer ;
		        }

		       /* and release the file */
		       fclose(getStream) ;
		       if (bcount != bgetsize)
		        {
			 fprintf(stderr,"pass2: TBGET: bcount and bgetsize do not match\n") ;
			 exit(1) ;
			}
		      }
                    }
                    break ;
#endif

     case TOPT    : value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                    i = value % 4 ;
                    if (i == 1)
                     nextListState |= 1 << ListPC ;
                    else
                     if (i == 2)
                      nextListState &= ~(1 << ListPC) ;
                    i = value / 4 ;
                    if ((i & 1) && ((1 << ListPC) & listStatus))
                     {
                      ListLine() ;
                      PageThrow() ;
                     }
                    if (i & 2)
                     lineNumber = 0 ;
                    i = i / 4 ;
                    if ((i % 4) == 1)
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
                    i = i / 16 ;
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

     case TRN     :
     case TCN     :
     case TCP     :
     case TFN     :
                    ListRegValue(ConstantExpr(line,&lineIndex,FALSE,&defined,NULL)) ;
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

     case TMACRO  : inMacroDef = TRUE ;
                    do
                     {
                      ListLine() ;
                      ifBool = GetLine(&line) ;
                      if (exception == EndOfInput)
                       break ;
                      InitLineList() ;
                      ListLineNumber() ;
                      ListAddress() ;
                      lineIndex = 0 ;
                      if ((!AllComment(line, &lineIndex)) && (lineIndex != 0) && DirTest(line,&lineIndex,&name) && NameDirective(&directiveNumber,name) && AllComment(line,&lineIndex) && (directiveNumber == TMEND))
                       break ;
                     } while (1) ; /* loop */
                    inMacroDef = FALSE ;
                    break ;

     case TMEXIT  : if (MexitDir() && !exception)
                     return TRUE ;
                    break ;

     case TMEND   : if (MendDir() && !exception)
                     return TRUE ;
                    break ;

     case TGBLA   : ifBool = SymbolTest(line,&lineIndex,&name) ;
                    symbolPointer = LookupGlobalA(name) ;
                    symbolPointer->value.card = 0 ; /* Re-initialise value */
                    CancelListForSet() ;
                    break ;

     case TGBLL   : ifBool = SymbolTest(line,&lineIndex,&name) ;
                    symbolPointer = LookupGlobalL(name) ;
                    symbolPointer->value.bool = FALSE ; /* Re-initialise value */
                    CancelListForSet() ;
                    break ;

     case TGBLS   : ifBool = SymbolTest(line,&lineIndex,&name) ;
                    symbolPointer = LookupGlobalS(name) ;
                    symbolPointer->value.ptr->length = 0 ;
                    CancelListForSet() ;
                    break ;

     case TLCLA   : ifBool = SymbolTest(line,&lineIndex,&name) ;
                    symbolPointer = DefineLocalA(name) ;
                    symbolPointer->value.card = 0 ; /* Re-initialise value */
                    CancelListForSet() ;
                    break ;

     case TLCLL   : ifBool = SymbolTest(line,&lineIndex,&name) ;
                    symbolPointer = DefineLocalL(name) ;
                    symbolPointer->value.bool = FALSE ; /* Re-initialise value */
                    CancelListForSet() ;
                    break ;

     case TLCLS   : ifBool = SymbolTest(line,&lineIndex,&name) ;
                    symbolPointer = DefineLocalS(name) ;
                    symbolPointer->value.ptr->length = 0 ;
                    CancelListForSet() ;
                    break ;

     case TSETA   : value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                    symbolPointer = LookupLocalA(name) ;
                    if (symbolPointer == NULL)
                     symbolPointer = LookupGlobalA(name) ;
                    symbolPointer->value.card = value ;
                    ListWordValue(value) ;
                    CancelListForSet() ;
                    break ;

     case TSETL   : ifBool = LogicalExpr(line,&lineIndex) ;
                    symbolPointer = LookupLocalL(name) ;
                    if (symbolPointer == NULL)
                     symbolPointer = LookupGlobalL(name) ;
                    symbolPointer->value.bool = ifBool ;
                    ListBoolValue(ifBool) ;
                    CancelListForSet() ;
                    break ;

     case TSETS   : StringExpr(line,&lineIndex,&string) ;
                    symbolPointer = LookupLocalS(name) ;
                    if (symbolPointer == NULL)
                     symbolPointer = LookupGlobalS(name) ;
                    ListStringValue(string) ;
                    symbolPointer->value.ptr->length = string.length ;
                    if (string.length > symbolPointer->value.ptr->maxLength)
                     {
                      if (symbolPointer->value.ptr->maxLength > 0)
                       free(symbolPointer->value.ptr->key) ;
                      symbolPointer->value.ptr->key = mymalloc(string.length) ;
                      symbolPointer->value.ptr->maxLength = string.length ;
                     }
                    if (string.length > 0)
                     memcpy(symbolPointer->value.ptr->key,string.key,string.length) ;
                    CancelListForSet() ;
                    break ;

     case TASSERT : if (!(AssertExpr(line,&lineIndex,FALSE,&defined) || errorFound))
                     {
                      WarningReport("Assertion failed") ;
                      exception = FileNotFound ;
                     }
                    break ;

     case TROUT   : while ((programCounter % 4) != 0)
                     CodeByte(0) ;
                    symbolPointer = (symbolFound) ? LookupFixed(name,FALSE) : NULL ;
                    NewRoutine(symbolPointer) ;
                    break ;

     case TALIGN  : value = (AllComment(line,&lineIndex)) ? 4 : ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                    if (line[lineIndex] == Comma)
                     {
                      lineIndex++ ;
                      i = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                     }
                    else
                     i = 0 ;

                    /* Now align */
                    while ((programCounter - i) % value != 0)
                     CodeByte(0) ;
                    break ;

     case TLTORG  : LiteralOrg() ;
                    break ;

     case TIMPORT : (void)SymbolTest(line,&lineIndex,&name) ;
                    if (AllComment(line,&lineIndex))
                     ifBool = FALSE ;	/* no attributes */
                    else
                     if (line[lineIndex] == Comma)
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
                    /* If an "IMPORT"ed symbol is defined internally */
                    if (symbolPointer->u.s.at == NoneAT)
                     {
                      AddSymbolToTable(symbolPointer,nname,TRUE,FALSE) ;
                      symbolPointer->u.s.at = ExportedAT ;
                     }
                    if ((!librarycode) && (!clmake_def))
		     {
		      if (ifBool)
		       AddStub(name,TRUE) ;	/* exception stub */
		      else
                       AddStub(name,FALSE) ;	/* normal stub */
		     }
                    break ;

     case TLABREF : (void)SymbolTest(line,&lineIndex,&name) ;
                    /* We should always IMPORT the "_" preceded name */
                    nametext[0] = 128 ;
                    memcpy(&nametext[1],name.key,name.length) ;
                    nametext[1 + name.length] = '\0' ;
                    nname.length = strlen(nametext) ;
                    symbolPointer = LookupRef(nname,FALSE) ;
                    /* If an "IMPORT"ed symbol is defined internally */
                    if (symbolPointer->u.s.at == NoneAT)
                     {
                      AddSymbolToTable(symbolPointer,nname,TRUE,FALSE) ;
                      symbolPointer->u.s.at = ExportedAT ;
                     }
                    break ;

     case TEXPORT : for (;;)
                     {
                      (void)SymbolTest(line,&lineIndex,&name) ;
                      nametext[0] = 128 ;
                      memcpy(&nametext[1],name.key,name.length) ;
                      nametext[1 + name.length] = '\0' ;
                      nname.length = strlen(nametext) ;
                      symbolPointer = LookupRef(nname,FALSE) ;

                      if ((symbolPointer->u.s.sds != DefinedSDS) && (symbolPointer->u.s.sds != UndefinedSDS))
                       {
                        printf("**DEBUG** failed EXPORT of \"") ;
                        PrintSymbol(symbolPointer->key) ;
                        printf("\"\n") ;
                        Warning(UnDefExp);
                        return TRUE ;
                       }

                      switch (symbolPointer->u.s.at)
                       {
                        case ExportAT      : AddSymbolToTable(symbolPointer,nname,TRUE,TRUE) ;
                                             symbolPointer->u.s.at = ExportedAT ;
                                             break ;

                        case HDataExportAT : AddSymbolToTable(symbolPointer,nname,TRUE,TRUE) ;
                                             symbolPointer->u.s.at = HDataExportedAT ;
                                             break ;

                        case HCodeExportAT : AddSymbolToTable(symbolPointer,nname,TRUE,TRUE) ;
                                             symbolPointer->u.s.at = HCodeExportedAT ;
                                             break ;
                        default            : /* do nothing */
                                             break ;
                       }

                      if (AllComment(line,&lineIndex))
                       break ;
                      while (line[++lineIndex] == Space) ;
                     } ; /* for */
                    break ;

     case TEXTERN : nametext[0] = '|' ;
                    memcpy(&nametext[1],name.key,name.length) ;
                    nametext[1 + name.length] = '|' ;
                    nametext[2 + name.length] = '\0' ;
                    nname.length = strlen(nametext) ;
                    symbolPointer = LookupRef(nname,FALSE) ;
                    if ((symbolPointer->u.s.sds != DefinedSDS) && (symbolPointer->u.s.sds != UndefinedSDS))
                     {
#ifdef DEBUG
                      printf("**DEBUG** failed EXTERN of \"") ;
                      PrintSymbol(symbolPointer->key) ;
                      printf("\"\n") ;
#endif
                      Warning(UnDefExp) ;
                      return(TRUE) ;
                     }

                    if (traceon)
                     {
                      printf("EXTERN: symbol \"") ;
                      PrintSymbol(symbolPointer->key) ;
                      printf("\"\n") ;
                     }

                    AddSymbolToTable(symbolPointer,nname,FALSE,TRUE) ;
                    switch (symbolPointer->u.s.at)
                     {
                      case ExportAT      :
#ifdef DEBUG
                                           printf("**DEBUG** EXTERN EXPORT\n") ;
#endif
                                           AddSymbolToTable(symbolPointer,nname,TRUE,TRUE) ;
                                           symbolPointer->u.s.at = ExportedAT ;
                                           break ;

                      default            : /* do nothing */
#ifdef DEBUG
                                           printf("**DEBUG** EXTERN do nothing\n") ;
#endif
                                           break ;
                     }

                    if (!AllComment(line,&lineIndex))
                     Warning(BadExtern) ;
                    break ;

     case TDCFS   : /* Now handle value to be output */
                    while ((programCounter % 4) != 0)
                     CodeByte(0) ;
                    do
                     {
                      readResponse = hRead(line,&lineIndex,Single,&value,&i) ;
                      CodeWord(value) ;
                      while (line[lineIndex] == Space)
                       lineIndex++ ;
                      if (line[lineIndex] != Comma)
                       break ;
                      lineIndex++ ;
                     } while (1) ;
                    break ;

     case TDCFD   : /* Now handle value to be output */
                    while ((programCounter % 4) != 0)
                     CodeByte(0) ;
                    do
                     {
                      readResponse = hRead(line,&lineIndex,Double,&value,&i) ;
                      while (line[lineIndex] == Space)
                       lineIndex++ ;
                      CodeWord(value) ;
                      CodeWord(i) ;
                      if (line[lineIndex] != Comma)
                       break ;
                      lineIndex++ ;
                     } while (1) ;
                    break ;

     case TMODULE : /* construct a module header */
                    if (traceon)
                     printf("pass2: MODULE\n") ;
                    RelocInit() ;       /* should not be needed */
                    programCounter = 0 ;
                    break ;

     case TDATA   : /* Define a named segment in the data area */
                    /* "        DATA <symbol>,<size>" */
                    (void)SymbolTest(line,&lineIndex,&name) ;

                    nametext[0] = 128 ;
                    memcpy(&nametext[1],name.key,name.length) ;
                    nametext[1 + name.length] = '\0' ;
                    nname.length = strlen(nametext) ;

                    symbolPointer = LookupRef(nname,FALSE) ;
                    if ((symbolPointer->u.s.at == HDataAT) || (symbolPointer->u.s.at == HDataExportAT) || (symbolPointer->u.s.at == HDataExportedAT))
                     {
                      lineIndex++ ;     /* step over the comma */
                      value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                      if (errorFound)
                       return(TRUE) ;

                      if (traceon)
                       {
                        printf("pass2: DATA ") ;
                        PrintSymbol(symbolPointer->key) ;
                        printf(",%d\n",value) ;
                       }

                      /* "DATA <symbol>,<size>"
                       *                HOF_t_data <size> <label>
                       */
                      write_staticarea(HOF_t_data,value,symbolPointer) ;
                     }
                    else
                     {
                      Warning(MulDefSym) ;
                      return(TRUE) ;
                     }
                    break ;

     case TCODE   : /* Define a named segment in the code area */
                    /* "        CODE <symbol>" */
                    (void)SymbolTest(line,&lineIndex,&name) ;

                    nametext[0] = 128 ;
                    memcpy(&nametext[1],name.key,name.length) ;
                    nametext[1 + name.length] = '\0' ;
                    nname.length = strlen(nametext) ;

                    symbolPointer = LookupRef(nname,FALSE) ;
                    if ((symbolPointer->u.s.at == HCodeAT) || (symbolPointer->u.s.at == HCodeExportAT) || (symbolPointer->u.s.at == HCodeExportedAT))
                     {
                      if (traceon)
                       {
                        printf("pass2: CODE ") ;
                        PrintSymbol(symbolPointer->key) ;
                        printf("\n") ;
                       }

                      /* "CODE <label>"
                       *                HOF_t_codetable <symbol>
                       */
                      write_staticarea(HOF_t_codetable,0,symbolPointer) ;
                     }
                    else
                     {
                      Warning(MulDefSym) ;
                      return(TRUE) ;
                     }
                    break ;

     case TCOMMON : /* Define a shared named segment in the data area */
                    /* "        COMMON <symbol>,<size>" */
                    (void)SymbolTest(line,&lineIndex,&name) ;

                    nametext[0] = 128 ;
                    memcpy(&nametext[1],name.key,name.length) ;
                    nametext[1 + name.length] = '\0' ;
                    nname.length = strlen(nametext) ;

                    symbolPointer = LookupRef(nname,FALSE) ;
                    if ((symbolPointer->u.s.at == HDataAT) || (symbolPointer->u.s.at == HDataExportAT) || (symbolPointer->u.s.at == HDataExportedAT))
                     {
                      lineIndex++ ;     /* step over the comma */
                      value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                      if (errorFound)
                       return(TRUE) ;

                      if (traceon)
                       {
                        printf("pass2: COMMON ") ;
                        PrintSymbol(symbolPointer->key) ;
                        printf(",%d\n",value) ;
                       }

                      /* "COMMON <symbol>,<size>"
                       *                HOF_t_common <size> <label>
                       */
                      write_staticarea(HOF_t_common,value,symbolPointer) ;
                     }
                    else
                     {
                      Warning(MulDefSym) ;
                      return(TRUE) ;
                     }
                    break ;

     case TINIT   : /* Create a link in the initialisation chain */
                    while ((programCounter % 4) != 0)   /* word-align data */
                     CodeByte(0) ;
                    if (traceon)
                     printf("pass2: INIT\n") ;
                    write_init() ;
                    break ;

     case TLABEL  : /* Create a "labelref" word patched by the linker */
                    /* "        LABEL <label>" */
                    (void)SymbolTest(line,&lineIndex,&name) ;
                    symbolPointer = LookupRef(name,FALSE) ;
                    if (symbolPointer != NULL)
                     {
                      /* "LABEL <label>"
                       *                32bit HOF_t_labelref <label>
                       */
                      while ((programCounter % 4) != 0) /* word-align data */
                       CodeByte(0) ;

                      if (traceon)
                       {
                        printf("pass2: LABEL ") ;
                        PrintSymbol(symbolPointer->key) ;
                        printf(" (defined)\n") ;
                       }

                      write_labeldir(symbolPointer) ;
                     }
                    else
                     {
                      /* If we are allowing undefined symbols, then generate
                       * a suitable LABEL patch.
                       */
                      if (allowUndefinedSymbols)
                       {
                        while ((programCounter % 4) != 0)
                         CodeByte(0) ;

                        if (traceon)
                         {
                          printf("pass2: LABEL ") ;
                          PrintSymbol(symbolPointer->key) ;
                          printf(" (undefined)\n") ;
                         }

                        write_labeldirname(&name) ;
                       }
                      else
                       {
                        Warning(UnDefSym) ;
                        return(TRUE) ;
                       }
                     }
                    break ;

     case TIMSIZE : /* Create a word containing the complete module size */
                    while ((programCounter % 4) != 0)   /* word-align data */
                     CodeByte(0) ;
                    if (traceon)
                     printf("pass2: IMSIZE\n") ;
                    write_imagesize() ;
                    break ;

     case TMODNUM : /* Create a word containing the linked module number */
                    /* "        MODNUM [<label>]" */
                    /* "MODNUM"
                     *          32bit HOF_t_modnum patch
                     *
                     * "MODNUM <label>"
                     *               HOF_t_patch_m68kshift -2
                     *          32bit HOF_t_datamod patch <label>
                     */
                    while ((programCounter % 4) != 0)   /* word-align data */
                     CodeByte(0) ;

                    if (SymbolTest(line,&lineIndex,&name))
                     {
                      nametext[0] = 128 ;
                      memcpy(&nametext[1],name.key,name.length) ;
                      nametext[1 + name.length] = '\0' ;
                      nname.length = strlen(nametext) ;

                      symbolPointer = LookupRef(nname,FALSE) ;
                      if (symbolPointer != NULL)
                       {
                        if (traceon)
                         {
                          printf("pass2: MODNUM ") ;
                          PrintSymbol(symbolPointer->key) ;
                          printf("\n") ;
                         }
                        write_modnum2(symbolPointer) ;
                       }
                      else
                       {
                        Warning(UnDefSym) ;
                        return(TRUE) ;
                       }
                     }
                    else
                     {
                      if (traceon)
                       printf("pass2: MODNUM\n") ;
                      write_modnum() ;
                     }
                    break ;

     case TMODOFF : /* Create a word containing the linked module offset */
                    /* "        MODOFF [<label>]" */
                    /* "MODOFF"
                     *                HOF_t_patch_m68kshift 2
                     *         32bit HOF_t_modnum patch
                     *
                     * "MODOFF <label>"
                     *          32bit HOF_t_datamod patch <label>
                     */
                    while ((programCounter % 4) != 0)   /* word-align data */
                     CodeByte(0) ;

                    if (SymbolTest(line,&lineIndex,&name))
                     {
                      nametext[0] = 128 ;
                      memcpy(&nametext[1],name.key,name.length) ;
                      nametext[1 + name.length] = '\0' ;
                      nname.length = strlen(nametext) ;

                      symbolPointer = LookupRef(nname,FALSE) ;
                      if (symbolPointer != NULL)
                       {
                        if (traceon)
                         {
                          printf("pass2: MODOFF ") ;
                          PrintSymbol(symbolPointer->key) ;
                          printf("\n") ;
                         }
                        write_modnum1(symbolPointer) ;
                       }
                      else
                       {
                        Warning(UnDefSym) ;
                        return(TRUE) ;
                       }
                     }
                    else
                     {
                      if (traceon)
                       printf("pass2: MODNUM\n") ;
                      write_modnum4() ;
                     }
                    break ;

     case TOFFSET : /* Create a word containing the data offset */
                    /* "        OFFSET <label>" */
                    (void)SymbolTest(line,&lineIndex,&name) ;

                    nametext[0] = 128 ;
                    memcpy(&nametext[1],name.key,name.length) ;
                    nametext[1 + name.length] = '\0' ;
                    nname.length = strlen(nametext) ;

                    symbolPointer = LookupRef(nname,FALSE) ;
                    if (symbolPointer != NULL)
                     {
                      /* "OFFSET <label>"
                       *                32bit HOF_t_dataref <label>
                       */
                      while ((programCounter % 4) != 0) /* word-align data */
                       CodeByte(0) ;

                      if (traceon)
                       {
                        printf("pass2: OFFSET ") ;
                        PrintSymbol(symbolPointer->key) ;
                        printf(" (defined)\n") ;
                       }

                      if ((symbolPointer->u.s.at == HCodeAT) || (symbolPointer->u.s.at == HCodeExportAT) || (symbolPointer->u.s.at == HCodeExportedAT))
                       write_coderef(symbolPointer) ;
                      else
                       write_dataref(symbolPointer) ;
                     }
                    else
                     {
                      /* If we are allowing undefined symbols, then generate
                       * a suitable OFFSET patch.
                       */
                      if (allowUndefinedSymbols)
                       {
                        while ((programCounter % 4) != 0) /* word-align data */
                         CodeByte(0) ;

                        if (traceon)
                         {
                          printf("pass2: OFFSET ") ;
                          PrintSymbol(nname) ;
                          printf(" (undefined)\n") ;
                         }

                        write_datarefname(&nname) ;
                       }
                      else
                       {
                        Warning(UnDefSym) ;
                        return(TRUE) ;
                       }
                     }
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
       }
    }
  }
 return TRUE ;
} /* End P2Directive */

/*---------------------------------------------------------------------------*/
/* EOF p2dirs/c */
