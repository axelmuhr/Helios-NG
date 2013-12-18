/* -> p1hand/c
 * Title:               The general line processing routine
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "code.h"
#include "conds.h"
#include "constant.h"
#include "errors.h"
#include "expr.h"
#include "extypes.h"
#include "formatio.h"
#include "fpio.h"
#include "getline.h"
#include "globvars.h"
#include "listing.h"
#include "literals.h"
#include "llabel.h"
#include "mactypes.h"
#include "nametype.h"
#include "asmvars.h"
#include "occur.h"
#include "opcode.h"
#include "p1dirs.h"
#include "p1hand.h"
#include "tables.h"
#include "tokens.h"
#include "vars.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
/* Bit values */

#define Type1ValueShift  0x200000  /* Bit 21 onwards for four bits */
#define Type1ImmBit      0x2000000 /* The bit distinguishing Imm from Reg */
#define Type2RegBit      0x2000000 /* The bit distinguishing Imm from Reg */
#define PreBit           0x1000000 /* The pre/post bit for type 2/4 opcodes */
#define UpBit            0x800000  /* The up/down bit for type 2/4 opcodes */
#define PSRBit           0x400000  /* PSR update bit for group 4 opcodes */
#define WriteBackBit     0x200000  /* Writeback bit for data trans opcodes */
#define ANDVal           0x00
#define TSTVal           0x08
#define CMNVal           0x0B
#define MOVVal           0x0D
#define BICVal           0x0E
#define MVNVal           0x0F

/*---------------------------------------------------------------------------*/
/* Bit positions */

#define TransBit         21
#define LoadBit          20
#define CLnBit           22
#define DoubleBit        15

/*---------------------------------------------------------------------------*/

BOOLEAN Rotatable(CARDINAL value)
{
 CARDINAL i ;
 CARDINAL j ;
 CARDINAL k ;

 for (i = 0; (i <= 1); i++)
  {
   j = value ;
   k = 0 ;
   do 
    {
     if (value < 0x100)
      return TRUE ;
     if (k >= 0x10)
      break ;
     value = (value / 4) + (value % 4) * 0x40000000 ;
     k++ ;
    } while (1) ;
   value ^= -1 ;
  } /* for */
 return FALSE ;
} /* End Rotatable */

/*---------------------------------------------------------------------------*/
/* The returned value indicates if (the pass has finished */

BOOLEAN P1LineHandler(char **linkPointer,BOOLEAN *wasLink)
{
 Name           name ;  /* Used for the opcode */
 Name           lname ; /* Used for the label */
 SymbolPointer  symbolPointer ;
 char          *line ;
 CARDINAL       lineIndex = 0 ;
 CARDINAL       after_symbol ;
 CARDINAL       pastOpcode ;
 CARDINAL       opcodeValue ;
 CARDINAL       value1 ;
 CARDINAL       value2 ;
 CARDINAL       value ;
 OpcodeType     opcodeType ;
 BOOLEAN        passEnded = FALSE ;
 BOOLEAN        opcodeFound = FALSE ;
 BOOLEAN        symbol_found = FALSE ;
 BOOLEAN        defined ;
 OperandType    type = UnDefOT ;
 Size           fPSize ;
 EHeliosPatch   hDIR ;

 *wasLink = FALSE ;
 errorFound = FALSE ;

 if (GetLine(&line))
  {
   line = SubstituteString(line) ;
   currentLinePointer = line ;
  }
 if (exception == EndOfInput)
  {
   exception = FileNotFound ;
   return TRUE;
  }

 /* Now check for all comment */
 ListLineNumber() ;
 ListAddress() ;
 if (AllComment(line,&lineIndex))
  {
   return FALSE ;
  }

 lineIndex = 0 ;
 /* pass first item on line */
 if (SymbolTest(line,&lineIndex,&lname))
  {
   symbol_found = TRUE ;
   after_symbol = lineIndex ;

   if (!TermCheck(line[lineIndex]))
    {
     symbol_found = FALSE ;
     while (!TermCheck(line[lineIndex]))
      lineIndex++ ;
    }
  }
 else
  {
   while (!TermCheck(line[lineIndex]))
    lineIndex++ ;
  }

 if (!AllComment(line,&lineIndex))
  {
   (void)DirTest(line,&lineIndex,&name) ;
   opcodeFound = TRUE ;
   pastOpcode = lineIndex ;
   if (errorFound || P1Directive(line,wasLink,&passEnded,linkPointer))
    return(passEnded || exception) ;
  }

 /* Now handle WHILE and conditional stuff */
 if ((rejectedIFs > 0) || (rejectedWHILEs > 0))
  return FALSE ; /* line conditionally ignored */

 /* Now search for a valid opcode or macro */
 if (opcodeFound)
  {
   if (Opcode(name,&opcodeType,&opcodeValue))
    programCounter = (programCounter + 3) & ~3 ;        /* word-align PC */
   else
    {
     ExpandMacro(line,name) ;
     if (exception)
      {
       if (exception == StackOverflow)
        {
         UnwindToGet() ;
         exception = FileNotFound ;
        }
       return TRUE ;
      }
     return FALSE ;
    }
  }

 /* Now sort out local or ordinary labels */
 if (*line != Space)
  {
   if (symbol_found)
    {
     lineIndex = after_symbol ;
     symbolPointer = LookupFixed(lname,FALSE) ;
     if (symbolPointer == NULL)
      {
       Warning(MulDefSym) ;
       return FALSE ;
      }
     AddDef(symbolPointer) ;
     if ((opcodeFound && (line[lineIndex] != Space)) || !termin[line[lineIndex]])
      {
       Warning(SynAfterLab) ;
       return FALSE ;
      }

     /* Here we check for double definition etc. */
     if (symbolPointer->u.s.sds != UndefinedSDS)
      {
       Warning(MulDefSym) ;
       return FALSE ;
      }
     
     /* Symbol is defined and label type and relocatable */
     symbolPointer->u.s.sds = DefinedSDS ;
     symbolPointer->u.s.fst = RelocatableFST ;
     symbolPointer->u.s.sdt = FixedSDT ;
     symbolPointer->length = (opcodeFound) ? 4 : 0 ;
     symbolPointer->value.card = programCounter ;
     if (keepingAll && (symbolPointer->aOFData.symbolId == 0x8000))
      AddSymbol(symbolPointer) ;
    }
   else
    if (isdigit(*line))
     {
      lineIndex = 0 ;
      LabelDef(line,&lineIndex) ;
      if (errorFound)
       return FALSE ;
     }
    else
     { 
      Warning(InvLineStart) ;
      return FALSE ;
     }
  }

 if (opcodeFound)
  {
   if (!area_is_code)
    {
     Warning(CodeInDataArea) ;
     return FALSE ;
    }
   lineIndex = pastOpcode ;

   switch (opcodeType)
    {
     case DataProcessing : /* read the destination */
                           value = RegisterExpr(line,&lineIndex) ;
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ; /* Step past comma */
                           /* Now handle extra register unless MOV or MVN or test
                            * First get the opcode name bits
                            */
                           value = (opcodeValue / Type1ValueShift) % 16 ;
                           if ((value != MOVVal) && (value != MVNVal) && ((value > CMNVal) || (value < TSTVal)))
                            {
                             /* Here we need another register */
                             value = RegisterExpr(line,&lineIndex) ; /* Read the destination */
                             if (errorFound)
                              return FALSE ;
                             if (line[lineIndex] != Comma)
                              {
                               Warning(CommaMissing) ;
                               return FALSE ;
                              }
                             lineIndex++ ; /* Step past comma */
                            }
                           /* Now look for immediate or register form */
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           if (line[lineIndex] == Hash)
                            {
                             lineIndex++ ;
                             /* Now look for immediate form */
                             value = ConstantExpr(line,&lineIndex,TRUE,&defined,&hDIR) ;
                             if (errorFound)
                              return FALSE ;
                             /* Now see if (we have a rotator as well */
                             if (line[lineIndex] == Comma)
                              {
                               /* Look for a rotator */
                               lineIndex++ ;
                               value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                              }
                            }
                           else
                            {
                             /* Now look for register form */
                             value = ShiftedRegister(line, &lineIndex, TRUE);
                            }
                           break ;

     case DataTransfer   : value = RegisterExpr(line,&lineIndex) ; /* Destination/source */
                           if (errorFound)
                            return FALSE ;

                           if (value == 9)
                            Notification(BadLDR) ; /* warn user about possible corruption of dp */

                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ; /* Step past */
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           if (line[lineIndex] == SquareBra)
                            {
                             lineIndex++ ;
                             value = RegisterExpr(line,&lineIndex) ;
                             if (errorFound)
                              return FALSE ;

                             /* Here we split according to pre/post indexing */
                             if (line[lineIndex] == SquareKet)
                              {
                               /* This is the post-indexed form */
                               lineIndex++ ;
                               while (line[lineIndex] == Space)
                                lineIndex++ ;
                               if (line[lineIndex] != Comma)
                                {
                                 /* Allow optional shriek */
                                 if (line[lineIndex] == Shriek)
                                  lineIndex++ ;
                                }
                               else
                                {
                                 lineIndex++ ;
                                 while (line[lineIndex] == Space)
                                  lineIndex++ ;
                                 if (line[lineIndex] == Hash)
                                  {
                                   lineIndex++ ;
                                   value = ConstantExpr(line,&lineIndex,TRUE,&defined,&hDIR) ;
                                  }
                                 else
                                  {
                                   /* Here we must allow an optional + or - */
                                   if ((line[lineIndex] == PlusSign) || (line[lineIndex] == MinusSign))
                                    lineIndex++ ;
                                   value = ShiftedRegister(line,&lineIndex,FALSE) ;
                                  }
                                }
                              }
                             else
                              if (line[lineIndex] != Comma)
                               {
                                Warning(CommaMissing) ;
                                return FALSE ;
                               }
                              else
                               {
                                /* This is the pre-index case, check translate bit not set in opcode */
                                if ((1 << TransBit) & opcodeValue)
                                 {
                                  Warning(BadTrans) ;
                                  return FALSE ;
                                 } ; /* if */
                                lineIndex++ ;
                                while (line[lineIndex] == Space)
                                 lineIndex++ ;
                                if (line[lineIndex] == Hash)
                                 {
                                  lineIndex++ ;
                                  value = ConstantExpr(line,&lineIndex,TRUE,&defined,&hDIR) ;
                                 }
                                else
                                 {
                                  /* Here we must allow an optional + or - */
                                  if ((line[lineIndex] == PlusSign) || (line[lineIndex] == MinusSign))
                                   lineIndex++ ;
                                  value = ShiftedRegister(line,&lineIndex,FALSE) ;
                                 }
                                if (errorFound)
                                 return FALSE ;
                                if (line[lineIndex] != SquareKet)
                                 {
                                  Warning(MissSqKet) ;
                                  return FALSE ;
                                 }
                                lineIndex++ ;
                                while (line[lineIndex] == Space)
                                 lineIndex++ ;
                                if (line[lineIndex] == Shriek)
                                 lineIndex++ ;
                               }
                            }
                           else
                            {
                             /* look for a PC or register relative expression, or a literal */
                             if ((1 << TransBit) & opcodeValue)
                              {
                               Warning(BadTrans) ;
                               return FALSE ;
                              }

                             if ((line[lineIndex] == Equals) && ((1 << LoadBit) & opcodeValue))
                              {
                               lineIndex++ ;
                               symbolPointer = ExternalExpr(line,&lineIndex,&value) ;
                               if (symbolPointer == NULL)
                                {
                                 value = ConstantOrAddressExpr(line,&lineIndex,&type,TRUE,&defined);
                                 if (errorFound)
                                  return FALSE ;
                                 if (defined && (type == PCRelOT))
                                  value = AddAddressLiteral(TRUE,value) ;
                                 else
                                  if (!(defined && Rotatable(value)))
                                   value = AddLiteral(defined,value) ;
                                }
                               else
                                value = AddExternalLiteral(symbolPointer,value) ;
                              }
                             else
                              {
                               value = RegisterRelExpr(line,&lineIndex,&value,&type,TRUE,&defined) ;
                               if (type == DPRelOT)
                                programCounter += 4 ; /* 2 instructions generated */
                               if (line[lineIndex] == Shriek)
                                lineIndex++ ;
                              }
                            }
                           break ;

     case SWI            : value = ConstantExpr(line,&lineIndex,TRUE,&defined,NULL) ;
                           break ;

     case BlockData      : value = RegisterExpr(line,&lineIndex) ;
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] == Shriek)
                            {
                             lineIndex++ ;
                             while (line[lineIndex] == Space)
                              lineIndex++ ;
                            }
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           if (line[lineIndex] != CurlyBra)
                            {
                             Warning(BraMiss) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           do
                            {
                             value = RegisterExpr(line,&lineIndex) ;
                             if (errorFound)
                              return FALSE ;
                             if (line[lineIndex] == Dash)
                              {
                               lineIndex++ ;
                               value = RegisterExpr(line,&lineIndex) ;
                               if (errorFound)
                                return FALSE ;
                              }
                             if (line[lineIndex] != Comma)
                              break ;
                             lineIndex++ ;
                            } while (1) ; /* loop */
                           if (line[lineIndex] != CurlyKet)
                            {
                             Warning(KetMiss) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           if (line[lineIndex] == Hat)
                            lineIndex++ ;
                           break ;

     case Branch         : symbolPointer = ExternalExpr(line,&lineIndex,&value) ;
                           if (symbolPointer == NULL)
                            value = AddressExpr(line,&lineIndex,TRUE,&defined) ;
                           break ;

     case Adr            :
     case ADRL           :
                           value = RegisterExpr(line,&lineIndex) ;
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;

                           if (value == 9)
                            Notification(BadADRL) ;

                           if ((value == 15) && (opcodeType == ADRL))
                            {
                             Warning(BadADRL) ;
                             return FALSE ;
                            }

                           value = NotStringExpr(line,&lineIndex,&value,&type,TRUE,&defined) ;

                           if (opcodeType == ADRL)
                            programCounter += 4 ; /* Double word opcode */

                           if (type == DPRelOT)
                            {
                             /* If the referenced symbol is in the static data area then we
                              * must generate different code to the normal (local) ADR[L].
                              * If the symbol is undefined we should generate the normal
                              * ADR[L] code and complain if it turns out to be a static area symbol.
                              */

                             /* two(ADR) or three(ADRL) word sequence under Helios */
                             programCounter += 4 ;
                            }
                           break ;

     case SWP            : value = RegisterExpr(line,&lineIndex) ; /* Destination */
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = RegisterExpr(line,&lineIndex) ; /* Source */
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           if (line[lineIndex] != SquareBra)
                            {
                             Warning(MissBra) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = RegisterExpr(line,&lineIndex) ;
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != SquareKet)
                            {
                             Warning(MissSqKet) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           break ;

     case FPDataTransfer : value = FPRegisterExpr(line,&lineIndex) ; /* Destination/source */
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           if (line[lineIndex] == SquareBra)
                            {
                             lineIndex++ ;
                             value = RegisterExpr(line,&lineIndex) ;
                             if (errorFound)
                              return FALSE ;
                             /* Here we split according to pre/post indexing */
                             if (line[lineIndex] == SquareKet)
                              {
                               /* This is the post-indexed form */
                               lineIndex++ ;
                               while (line[lineIndex] == Space)
                                lineIndex++ ;
                               if (line[lineIndex] != Comma)
                                {
                                 /* Allow optional shriek */
                                 if (line[lineIndex] == Shriek)
                                  lineIndex++ ;
                                }
                               else
                                {
                                 lineIndex++ ;
                                 while (line[lineIndex] == Space)
                                  lineIndex++ ;
                                 if (line[lineIndex] == Hash)
                                  {
                                   lineIndex++ ;
                                   value = ConstantExpr(line,&lineIndex,TRUE,&defined,NULL) ;
                                  }
                                 else
                                  Warning(HashMissing) ;
                                }
                              }
                             else
                              if (line[lineIndex] != Comma)
                               Warning(CommaMissing) ;
                              else
                               {
                                /* This is the pre-index case */
                                lineIndex++ ;
                                while (line[lineIndex] == Space)
                                 lineIndex++ ;
                                if (line[lineIndex] == Hash)
                                 {
                                  lineIndex++ ;
                                  value = ConstantExpr(line,&lineIndex,TRUE,&defined,NULL) ;
                                 }
                                else
                                 Warning(HashMissing) ;
                                if (errorFound)
                                 return FALSE ;
                                if (line[lineIndex] != SquareKet)
                                 {
                                  Warning(MissSqKet) ;
                                  return FALSE ;
                                 }
                                lineIndex++ ;
                                while (line[lineIndex] == Space)
                                 lineIndex++ ;
                                if (line[lineIndex] == Shriek)
                                 lineIndex++ ;
                               }
                            }
                           else
                            {
                             /* Here we look for a PC or register relative expression,
                              * or a literal
                              */
                             if ((line[lineIndex] == Equals) && ((1 << LoadBit) & opcodeValue) && !((1 << CLnBit) & opcodeValue))
                              {
                               /* Don't allow literals for extended or packed formats */
                               lineIndex++ ;
                               fPSize = ((1 << DoubleBit) & opcodeValue) ? Double : Single ;
                               switch (hRead(line,&lineIndex,fPSize,&value1,&value2))
                                {
                                 case ReadOverflow  : Warning(FPOver) ;
                                                      return FALSE ;
                                                      break ;

                                 case NoNumberFound : Warning(FPNoNum) ;
                                                      return FALSE ;
                                } /* switch */
                               switch (fPSize)
                                {
                                 case Single : value = AddFPLiteralSingle(value1) ;
                                               break ;

                                 case Double : value = AddFPLiteralDouble(value1,value2) ;
                                } /* switch */
                              }
                             else
                              {
                               value = RegisterRelExpr(line,&lineIndex,&value,&type,TRUE,&defined) ;
                               if (line[lineIndex] == Shriek)
                                lineIndex++ ;
                              }
                            }
                           break ;

     case FPDyadic       : value = FPRegisterExpr(line,&lineIndex) ; /* Read the destination */
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ; /* Step past */
                           value = FPRegisterExpr(line,&lineIndex) ; /* Read first source */
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           /* Now look for immediate or register form */
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           if (line[lineIndex] == Hash)
                            {
                             lineIndex++ ;
                             value = FPConstant(line,&lineIndex) ;
                            } else
                           /* Now look for register form */
                           value = FPRegisterExpr(line, &lineIndex) ;
                           break ;

     case FPMonadic      :
     case FPCompare      :
                           value = FPRegisterExpr(line,&lineIndex) ; /* Read the destination */
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           /* Now look for immediate or register form */
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           if (line[lineIndex] == Hash)
                            {
                             lineIndex++ ;
                             /* Now look for immediate form */
                             value = FPConstant(line,&lineIndex) ;
                            }
                           else /* now look for register form */
                            value = FPRegisterExpr(line,&lineIndex) ;
                           break ;

     case FPFloat        : value = FPRegisterExpr(line,&lineIndex) ; /* Read the destination */
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = RegisterExpr(line,&lineIndex) ;
                           break ;

     case FPFix          : value = RegisterExpr(line,&lineIndex) ; /* Read the destination */
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = FPRegisterExpr(line,&lineIndex) ;
                           break ;

     case FPStatus       : value = RegisterExpr(line,&lineIndex) ;
                           break ;

     case CPDT           : value = CPNameExpr(line,&lineIndex) ;
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = CPRegisterExpr(line,&lineIndex) ; /* Destination/source */
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           if (line[lineIndex] == SquareBra)
                            {
                             lineIndex++ ;
                             value = RegisterExpr(line,&lineIndex) ;
                             if (errorFound)
                              return FALSE ;
                             /* Here we split according to pre/post indexing */
                             if (line[lineIndex] == SquareKet)
                              {
                               /* This is the post-indexed form */
                               lineIndex++ ;
                               while (line[lineIndex] == Space)
                                lineIndex++ ;
                               if (line[lineIndex] != Comma)
                                {
                                 /* Allow optional shriek */
                                 if (line[lineIndex] == Shriek)
                                  lineIndex++ ;
                                }
                               else
                                {
                                 lineIndex++ ;
                                 while (line[lineIndex] == Space)
                                  lineIndex++ ;
                                 if (line[lineIndex] == Hash)
                                  {
                                   lineIndex++ ;
                                   value = ConstantExpr(line,&lineIndex,TRUE,&defined,NULL) ;
                                  }
                                 else
                                  Warning(HashMissing) ;
                                }
                              }
                             else
                              if (line[lineIndex] != Comma)
                               {
                                Warning(CommaMissing) ;
                                return FALSE ;
                               }
                              else
                               {
                                /* This is the pre-index case */
                                lineIndex++ ;
                                while (line[lineIndex] == Space)
                                 lineIndex++ ;
                                if (line[lineIndex] == Hash)
                                 {
                                  lineIndex++ ;
                                  value = ConstantExpr(line,&lineIndex,TRUE,&defined,NULL) ;
                                 }
                                else
                                 Warning(HashMissing) ;
                                if (errorFound)
                                 return FALSE ;
                                if (line[lineIndex] != SquareKet)
                                 {
                                  Warning(MissSqKet) ;
                                  return FALSE ;
                                 }
                                lineIndex++ ;
                                while (line[lineIndex] == Space)
                                 lineIndex++ ;
                                if (line[lineIndex] == Shriek)
                                 lineIndex++ ;
                               }
                            }
                           else
                            {
                             /* Here we look for a PC or register relative expression */
                             value = RegisterRelExpr(line,&lineIndex,&value,&type,TRUE,&defined) ;
                             if (line[lineIndex] == Shriek)
                              lineIndex++ ;
                            }
                           break ;

     case CPDO           : value = CPNameExpr(line,&lineIndex) ;
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                           if (errorFound)
                            return FALSE ;
                           if (value >= 0x10)
                            {
                             Warning(CPOpRange) ;
                             return FALSE ;
                            }
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = CPRegisterExpr(line,&lineIndex) ;
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = CPRegisterExpr(line,&lineIndex) ;
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = CPRegisterExpr(line,&lineIndex) ;
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] == Comma)
                            {
                             lineIndex++ ;
                             value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                             if (value >= 8)
                              Warning(CPOpRange) ;
                            }
                           break ;

     case CPRT           : value = CPNameExpr(line,&lineIndex) ;
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                           if (errorFound)
                            return FALSE ;
                           if (value >= 8)
                            {
                             Warning(CPOpRange) ;
                             return FALSE ;
                            }
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = RegisterExpr(line,&lineIndex) ;
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = CPRegisterExpr(line,&lineIndex) ;
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = CPRegisterExpr(line,&lineIndex) ;
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] == Comma)
                            {
                             lineIndex++ ;
                             value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                             if (value >= 8)
                              Warning(CPOpRange) ;
                            }
                           break ;

     case MUL            : value = RegisterExpr(line,&lineIndex) ; /* Read the destination */
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value1 = RegisterExpr(line,&lineIndex) ; /* Read the first source */
                           if (errorFound)
                            return FALSE ;
                           if (value1 == value)
                            WarningChs("Multiply destination equals first source\\N") ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = RegisterExpr(line,&lineIndex) ; /* Read the second source */
                           break ;

     case MLA            : value = RegisterExpr(line,&lineIndex) ; /* Read the destination */
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value1 = RegisterExpr(line,&lineIndex) ; /* Read the first source */
                           if (errorFound)
                            return FALSE ;
                           if (value1 == value)
                            WarningChs("Multiply destination equals first source\\N") ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = RegisterExpr(line,&lineIndex) ; /* Read the second source */
                           if (errorFound)
                            return FALSE ;
                           if (line[lineIndex] != Comma)
                            {
                             Warning(CommaMissing) ;
                             return FALSE ;
                            }
                           lineIndex++ ;
                           value = RegisterExpr(line,&lineIndex) ; /* Read the addend */
                           break ;
    } /* switch */
   programCounter += 4 ; /* Another 4 bytes produced */
  }

 if (!errorFound)
  {
   if (!AllComment(line,&lineIndex))
    Warning(BadEOL) ;
  }
 return FALSE ;
} /* End P1LineHandler */

/*---------------------------------------------------------------------------*/
/* Check for line all comment */

BOOLEAN AllComment(char *line,CARDINAL *index)
{
 char ch ;
 while (line[*index] == Space)
  (*index)++ ;
 ch = line[*index] ;
 if (ch == CommentSymbol)
  return(TRUE) ;
 return(ch == CR) ;
} /* End AllComment */

/*---------------------------------------------------------------------------*/

BOOLEAN TermCheck(char ch)
{
 return((ch == Space) || (ch == CommentSymbol) || (ch == CR)) ;
} /* End TermCheck */

/*---------------------------------------------------------------------------*/
/* EOF p1hand/c */
