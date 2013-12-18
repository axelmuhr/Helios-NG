/* -> p2hand/c
 * Title:               The general line processing routine
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "asmvars.h"
#include "asm.h"
#include "occur.h"
#include "osdepend.h"
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
#include "opcode.h"
#include "p1hand.h"
#include "p2dirs.h"
#include "p2hand.h"
#include "tables.h"
#include "tokens.h"
#include "vars.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
/* Bit values */

#define Type1ValueShift  0x200000  /* Bit 21 onwards for four bits */
#define Type1ImmBit      0x2000000 /* bit distinguishing Imm from Reg */
#define Type2RegBit      0x2000000 /* bit distinguishing Imm from Reg */
#define PreBit           0x1000000 /* pre/post bit for type 2/4 opcodes */
#define UpBit            0x800000  /* up/down bit for type 2/4 opcodes */
#define PSRBit           0x400000  /* PSR update bit for group 4 opcodes */
#define WriteBackBit     0x200000  /* writeback bit for data trans opcodes */
#define NOTFPRegisterBit 0x8       /* bit distinguishing imm from reg forms */

#define ANDVal           0x00
#define SUBVal           0x02
#define ADDVal           0x04
#define ADCVal           0x05
#define SBCVal           0x06
#define TSTVal           0x08
#define CMPVal           0x0A
#define CMNVal           0x0B
#define MOVVal           0x0D
#define BICVal           0x0E
#define MVNVal           0x0F
#define ADFVal           0x00
#define SUFVal           0x01
#define MVFVal           0BH
#define MNFVal           0CH

/*---------------------------------------------------------------------------*/
/* Bit positions */

#define DoubleBit        15

/*---------------------------------------------------------------------------*/

#if 0   /* useful function no longer used */
static BOOLEAN ADRRotatable(CARDINAL value)
{
 CARDINAL i ;
 CARDINAL k ;
 for (i = 0; (i <= 1); i++)
  {
   k = 0 ;
   do
    {
     if (value < 0x100)
      return TRUE ;
     if (k >= 0x10)
      break ;
     value = (value / 4) + (value % 4) * 0x40000000 ;
     k++ ;
    } while (1) ; /* loop */
   value = (value ^ -1) + 1 ;
  }
 return FALSE ;
} /* End ADRRotatable */
#endif

/*---------------------------------------------------------------------------*/

static BOOLEAN Rotatable(CARDINAL value,CARDINAL *k,BOOLEAN inv,CARDINAL *immValue1)
{
 CARDINAL i ;

 for (i = 0; i <= 1; i++)
  {
   *k = 0 ;
   do
    {
     if (value < 0x100)
      {
       *immValue1 = value ;
       return TRUE ;
      }
     if (*k >= 0x10)
      break ;
     value = (value % 0x40000000) * 4 + (value / 0x40000000) ;
     /* Rotate left value while incrementing rotator */
     (*k)++ ;
    } while (1) ;
   if (!inv)
    return FALSE ;
   value ^= -1 ;
  }
 return FALSE ;
} /* End Rotatable */

/*---------------------------------------------------------------------------*/

static BOOLEAN RotatedForm(CARDINAL *rotator,CARDINAL *immValue,CARDINAL value,CARDINAL *opcodeValue)
{
 *rotator = 0 ;
 do
  {
   if (*immValue < 0x100)
    break ;
   (*rotator)++ ;
   /* Now shift left immediate value */
   *immValue = (*immValue % 0x40000000) * 4 + (*immValue / 0x40000000) ;
   if (*rotator == 0x10)
    {
     /* Here we have failed, unless it can be converted
      * MOV <-> MVN
      * AND <-> BIC
      * ADC <-> SBC
      * by inverting the second operand, and
      * ADD <-> SUB
      * CMP <-> CMN
      * by negating the second operand.
      */
     *immValue ^= -1 ;
     *rotator = 0 ;
     switch (value)
      {
       case MVNVal : *opcodeValue += (MOVVal - MVNVal) * Type1ValueShift ;
                     break ;

       case MOVVal : *opcodeValue += (MVNVal - MOVVal) * Type1ValueShift ;
                     break ;

       case BICVal : *opcodeValue += (ANDVal - BICVal) * Type1ValueShift ;
                     break ;

       case ANDVal : *opcodeValue += (BICVal - ANDVal) * Type1ValueShift ;
                     break ;

       case ADCVal : *opcodeValue += (SBCVal - ADCVal) * Type1ValueShift ;
                     break ;

       case SBCVal : *opcodeValue += (ADCVal - SBCVal) * Type1ValueShift ;
                     break ;

       case ADDVal : *opcodeValue += (SUBVal - ADDVal) * Type1ValueShift ;
                     (*immValue)++ ;
                     break ;

       case SUBVal : *opcodeValue += (ADDVal - SUBVal) * Type1ValueShift ;
                     (*immValue)++ ;
                     break ;

       case CMPVal : *opcodeValue += (CMNVal - CMPVal) * Type1ValueShift ;
                     (*immValue)++ ;
                     break ;

       case CMNVal : *opcodeValue += (CMPVal - CMNVal) * Type1ValueShift ;
                     (*immValue)++ ;
                     break ;

       default     : Warning(ImmValRange) ;
                     return TRUE ;
      }
     do
      {
       if (*immValue < 0x100)
        break ;
       (*rotator)++ ;
       /* Now shift left immediate value */
       *immValue = (*immValue % 0x40000000) * 4 + (*immValue / 0x40000000) ;
       if (*rotator == 0x10)
        {
         Warning(ImmValRange) ;
         return TRUE ;
        }
      } while (1) ;
    }
  } while (1) ;
 return FALSE ;
} /* End RotatedForm */

/*---------------------------------------------------------------------------*/
/* Returned value indicates error found */
static BOOLEAN Rotated16Form(CARDINAL *rotator,CARDINAL *rotator1,CARDINAL *immValue1,CARDINAL *immValue,CARDINAL value,CARDINAL *opcodeValue)
{
 CARDINAL i ;

 *rotator = 0 ;
 *rotator1 = 0x0C ;
 do
  {
   if (Rotatable(*immValue / 0x100,&i,FALSE,immValue1))
    {
     *rotator1 = (*rotator1 + i) % 0x10 ;
     return FALSE ;
    }
   (*rotator)++ ;
   (*rotator1)++ ;
   if (*rotator1 == 0x10)
    *rotator1 = 0 ;
   /* Now shift left immediate value */
   *immValue = (*immValue % 0x40000000) * 4 + (*immValue / 0x40000000) ;
   if (*rotator == 0x10)
    {
     /* Here we have failed, unless it can be converted
      * ADD <-> SUB
      * by negating the second operand.
      */
     *immValue ^= -1 ;
     *rotator = 0 ;
     *rotator1 = 0x0C ;
     if (value == ADDVal)
      {
       *opcodeValue += (SUBVal - ADDVal) * Type1ValueShift ;
       (*immValue)++ ;
      }
     else
      {
       Warning(ImmValRange) ;
       return TRUE ;
      }
     do
      {
       if (Rotatable(*immValue / 0x100,&i,FALSE,immValue1))
        {
         *rotator1 = (*rotator1 + i) % 0x10 ;
         return FALSE ;
        }
       (*rotator)++ ;
       (*rotator1)++ ;
       if (*rotator1 == 0x10)
        *rotator1 = 0 ;
       /* Now shift left immediate value */
       *immValue = (*immValue % 0x40000000) * 4 + (*immValue / 0x40000000) ;
       if (*rotator == 0x10)
        {
         Warning(ImmValRange) ;
         return TRUE ;
        }
      } while (1) ;
    }
  } while (1) ;
 return FALSE ;
} /* End Rotated16Form */

/*---------------------------------------------------------------------------*/
/* The returned value indicates if the pass has finished. */
BOOLEAN P2LineHandler(char **linkPointer,BOOLEAN *wasLink)
{
 Name           name ;
 Name           lname ;
 SymbolPointer  symbolPointer;
 char          *line ;
 CARDINAL       lineIndex = 0 ;
 CARDINAL       after_symbol ;
 CARDINAL       pastOpcode ;
 CARDINAL       opcodeValue ;
 CARDINAL       cop ;
 CARDINAL       destReg ;
 CARDINAL       sourceReg1 ;
 CARDINAL       sourceReg2 ;
 CARDINAL       immValue ;
 CARDINAL       rotator ;
 CARDINAL       immValue1 ;
 CARDINAL       rotator1 ;
 CARDINAL       reg1 ;
 CARDINAL       reg2 ;
 CARDINAL       value1 ;
 CARDINAL       value2 ;
 CARDINAL       value ;
 OpcodeType     opcodeType ;
 BOOLEAN        passEnded = FALSE ;
 BOOLEAN        opcodeFound = FALSE ;
 BOOLEAN        symbol_found = FALSE ;
 BOOLEAN        defined ;
 int            blockDataBits ;
 OperandType    type = UnDefOT ;
 Size           fPSize ;
 ReadResponse   readResponse ;
 EHeliosPatch   hDIR ;

 *wasLink = FALSE ;
 errorFound = FALSE ;
 hDIR.type = NoPatch ;

 if (GetLine(&line))
  {
   line = SubstituteString(line) ;
   currentLinePointer = line ;
  } ; /* if */

 if (exception == EndOfInput)
  {
   exception = FileNotFound ;
   /* As allows EOF as end of assembly without error */
   return TRUE ;
  } ;

 ListLineNumber() ;
 ListAddress() ;

 /* Now check for all comment */
 if (AllComment(line,&lineIndex))
  return FALSE ;
 else /* sort out lines starting with # */

 lineIndex = 0 ;

 /* pass first item on line */
 if (SymbolTest(line,&lineIndex,&lname))
  {
   symbol_found = TRUE ;
   after_symbol = lineIndex ;

   if (line[lineIndex] == Colon)
    lineIndex++;

   if (!TermCheck(line[lineIndex]))
    {
     symbol_found = FALSE ;
     while (!TermCheck(line[lineIndex]))
      lineIndex++;
    } ; /* if */
  }
 else
  {
   while (!TermCheck(line[lineIndex]))
    lineIndex++ ;
  } ; /* if */

 if (!AllComment(line,&lineIndex))
  {
   (void)DirTest(line,&lineIndex,&name) ;
   opcodeFound = TRUE ;
   pastOpcode = lineIndex ;
   if (errorFound || P2Directive(line,wasLink,&passEnded,linkPointer))
    return(passEnded || exception) ;
  } ; /* if */

 /* Now handle WHILE and conditional stuff */
 if ((rejectedIFs > 0) || (rejectedWHILEs > 0))
  return FALSE ; /* Line conditionally ignored */

 /* Now search for a valid opcode or macro */
 if (opcodeFound)
  {
   if (!Opcode(name,&opcodeType,&opcodeValue))
    {
     ExpandMacro(line,name) ;
     if (exception)
      {
       if (exception == StackOverflow)
        {
         UnwindToGet() ;
         exception = FileNotFound ;
        } ;
       return TRUE ;
      } ;
     return FALSE ;
    } ;
  } ;

 /* Now sort out local or ordinary labels */
 if (*line != Space)
  {
   if (symbol_found)
    {
     lineIndex = after_symbol ;
     if (line[lineIndex] == Colon)
      lineIndex++ ;
     symbolPointer = LookupFixed(lname,FALSE) ;

     if (keepingAll && ((symbolPointer->u.s.at == NoneAT) || (symbolPointer->u.s.at == ExportAT) || (symbolPointer->u.s.at == HDataExportAT) || (symbolPointer->u.s.at == HCodeExportAT)))
      {
       AddSymbolToTable(symbolPointer,lname,FALSE,TRUE) ;
       if (symbolPointer->u.s.at == NoneAT)
        symbolPointer->u.s.at = KeptAT ;
      } ; /* if */

     /* define "EXPORT"ed symbols as GLOBAL as soon as they are defined */
     if ((symbolPointer->u.s.at == ExportAT) || (symbolPointer->u.s.at == HDataExportAT) || (symbolPointer->u.s.at == HCodeExportAT))
      {
       AddSymbolToTable(symbolPointer,lname,TRUE,TRUE) ;

       switch (symbolPointer->u.s.at)
        {
         case HDataExportAT : symbolPointer->u.s.at = HDataExportedAT ;
                              break ;
         case HCodeExportAT : symbolPointer->u.s.at = HCodeExportedAT ;
                              break ;
         default            : symbolPointer->u.s.at = ExportedAT ;
                              break ;
        }
      }
    }
   else
    if (isdigit(*line))
     {
      lineIndex = 0 ;
      LabelDef(line,&lineIndex) ;
     } ;
  } ;

 /* now deal with opcodes */
 if (opcodeFound)
  {
   lineIndex = pastOpcode ;
   while (line[lineIndex] == Space)
    lineIndex++ ;

   /* Now align on word boundary */
   while ((programCounter % 4) != 0)
    CodeByte(0) ;

   switch (opcodeType)
    {
     case DataProcessing : destReg = RegisterExpr(line,&lineIndex) ;
                           lineIndex++ ; /* Step past comma */
                           /* Now handle extra register unless MOV or MVN or test.
                            * First get the opcode name bits
                            */
                           value = (opcodeValue / Type1ValueShift) % 16 ;
                           if ((value != MOVVal) && (value != MVNVal) && ((value > CMNVal) || (value < TSTVal)))
                            {
                             /* Here we need another register */
                             sourceReg1 = RegisterExpr(line,&lineIndex) ; /* Read the destination */
                             lineIndex++ ; /* Step past comma */
                            }
                           else
                            if ((value == MOVVal) || (value == MVNVal))
                             sourceReg1 = 0 ;
                            else
                             {
                              sourceReg1 = destReg ;
                              destReg = 0 ;
                             }

                           /* Now look for immediate or register form */
                           while (line[lineIndex] == Space)
                            lineIndex++ ;

                           if (line[lineIndex] == Hash)
                            {
                             lineIndex++ ;
                             /* Now look for immediate form */
                             immValue = ConstantExpr(line,&lineIndex,FALSE,&defined,&hDIR) ;
                             if (errorFound)
                              return FALSE ;

                             /* Now see if we have a rotator as well */
                             if (line[lineIndex] == Comma)
                              {
                               /* Look for a rotator */
                               lineIndex++ ;
                               rotator = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                               /* Now check that it is valid */
                               if (immValue >= 0x100)
                                {
                                 Warning(ImmValRange) ;
                                 return FALSE ;
                                }
                               if ((rotator & 1) || ((rotator / 2) >= 0x10))
                                {
                                 Warning(BadRot) ;
                                 return FALSE ;
                                }
                               rotator = rotator / 2 ;
                              }
                             else
                              {
                               /* Here we try to determine if the rotated form can be produced */
                               if (immValue < 0x100)
                                rotator = 0 ;
                               else
                                if (RotatedForm(&rotator,&immValue,value,&opcodeValue))
                                 return FALSE ;
                              }

                             /* Now update opcodeValue */
                             opcodeValue += rotator*0x100 + immValue + Type1ImmBit ;
                            }
                           else
                            {
                             /* Now look for register form */
                             opcodeValue += ShiftedRegister(line,&lineIndex,TRUE) ;
                            }
                           if (errorFound)
                            return FALSE ;

                           if (hDIR.type != NoPatch)
                            {
                             int ptype = HOF_t_patch_armdp ;    /* by default */

                             switch (hDIR.type)
                              {
                               case ModOffPatch   : ptype = HOF_t_patch_armdp ;
                                                    break ;
                               case OffsetPatch   : ptype = HOF_t_patch_armdp ;
                                                    break ;
                               case LsbOffPatch   : ptype = HOF_t_patch_armdplsb ;
                                                    break ;
                               case MidOffPatch   : ptype = HOF_t_patch_armdpmid ;
                                                    break ;
                               case MsbOffPatch   : ptype = HOF_t_patch_armdprest ;
                                                    break ;
                               case ModOffWOPatch : ptype = HOF_t_patch_armdp ;
                                                    break ;
                              }
                             write_patch(ptype,(opcodeValue + destReg * 0x1000 + sourceReg1 * 0x10000)) ;
                             if (hDIR.type == ModOffWOPatch)
                              write_modnum5() ;
                             else
                              {
                               if (hDIR.type == ModOffPatch)
                                write_label(HOF_t_datamod,hDIR.symbol->key.key,hDIR.symbol->key.length) ;
                               else
                                {
                                 if ((hDIR.symbol->u.s.at == HCodeAT) || (hDIR.symbol->u.s.at == HCodeExportAT) || (hDIR.symbol->u.s.at == HCodeExportedAT))
                                  write_label(HOF_t_codesymbol,hDIR.symbol->key.key,hDIR.symbol->key.length) ;
                                 else
                                  write_label(HOF_t_dataref,hDIR.symbol->key.key,hDIR.symbol->key.length) ;
                                }
                              }
                            }
                           else
                            CodeWord(opcodeValue + destReg * 0x1000 + sourceReg1 * 0x10000) ;
                           break ;

     case DataTransfer   : destReg = RegisterExpr(line,&lineIndex) ; /* Destination/source */
                           lineIndex++ ; /* Step past comma */
                           while (line[lineIndex] == Space)
                            lineIndex++ ;       /* step over spaces */

                           if (line[lineIndex] == SquareBra)
                            {
                             lineIndex++ ;
                             sourceReg1 = RegisterExpr(line,&lineIndex) ;
                             /* Here we split according to pre/post indexing */
                             if (line[lineIndex] == SquareKet)
                              {
                               /* This is the post-indexed form */
                               lineIndex++ ;
                               while (line[lineIndex] == Space)
                                lineIndex++;

                               if (line[lineIndex] != Comma)
                                {
                                 opcodeValue += UpBit ;
                                 /* Allow optional shriek */
                                 if (line[lineIndex] == Shriek)
                                  lineIndex++ ;
                                 immValue = 0 ;
                                }
                               else
                                {
                                 /* Definitely post-indexed */
                                 lineIndex++ ;
                                 while (line[lineIndex] == Space)
                                  lineIndex++ ;
                                 if (line[lineIndex] == Hash)
                                  {
                                   lineIndex++ ;
                                   immValue = ConstantExpr(line,&lineIndex,FALSE,&defined,&hDIR) ;
                                   if (errorFound)
                                    return FALSE ;
                                   if (abs((int)immValue) >= 0x1000)
                                    {
                                     Warning(DatOff) ;
                                     return FALSE ;
                                    }
                                   if (immValue < 0x1000)
                                    opcodeValue += UpBit ;
                                   immValue = abs((int)immValue) ;
                                  }
                                 else
                                  {
                                   opcodeValue += Type2RegBit ;
                                   /* Here we must allow an optional + or - */
                                   if (line[lineIndex] == PlusSign)
                                    {
                                     lineIndex++ ;
                                     opcodeValue += UpBit ;
                                    }
                                   else
                                    if (line[lineIndex] == MinusSign)
                                     lineIndex++ ;
                                    else
                                     opcodeValue += UpBit ;
                                   immValue = ShiftedRegister(line,&lineIndex,FALSE) ;
                                  }
                                }
                              }
                             else
                              {
                               /* This is definitely pre-indexed */
                               opcodeValue += PreBit ;
                               lineIndex++ ;
                               while (line[lineIndex] == Space)
                                lineIndex++ ;
                               if (line[lineIndex] == Hash)
                                {
                                 lineIndex++ ;
                                 immValue = ConstantExpr(line,&lineIndex,FALSE,&defined,&hDIR) ;
                                 if (errorFound)
                                  return FALSE ;
                                 if (abs((int)immValue) >= 0x1000)
                                  {
                                   Warning(DatOff) ;
                                   return FALSE ;
                                  }
                                 if (immValue < 0x1000)
                                  opcodeValue += UpBit ;
                                 immValue = abs((int)immValue) ;
                                }
                               else
                                {
                                 opcodeValue += Type2RegBit ;
                                 /* Here we must allow an optional + or - */
                                 if (line[lineIndex] == PlusSign)
                                  {
                                   lineIndex++ ;
                                   opcodeValue += UpBit ;
                                  }
                                 else
                                  if (line[lineIndex] == MinusSign)
                                   lineIndex++ ;
                                  else
                                   opcodeValue += UpBit ;
                                 immValue = ShiftedRegister(line,&lineIndex,FALSE) ;
                                }
                               lineIndex++ ;
                               while (line[lineIndex] == Space)
                                lineIndex++ ;
                               if (line[lineIndex] == Shriek)
                                {
                                 lineIndex++ ;
                                 opcodeValue += WriteBackBit ;
                                }
                              }
                            }
                           else
                            {
                             /* Here we look for one of the following:
                              *   a PC relative expression
                              *   a register relative expression
                              *   a literal
                              *   a module data area relative expression
                              */
                             if (line[lineIndex] == Equals)
                              {
                               lineIndex++ ;
                               symbolPointer = ExternalExpr(line,&lineIndex,&value) ;
                               sourceReg1 = 15 ;
                               if (symbolPointer == NULL)
                                {
                                 value = ConstantOrAddressExpr(line,&lineIndex,&type,FALSE,&defined) ;
                                 if (errorFound)
                                  return FALSE ;

                                 if (type == PCRelOT)
                                  immValue = AddAddressLiteral(TRUE,value) ;
                                 else
                                  if (Rotatable(value,&immValue,TRUE,&immValue1))
                                   {
                                    opcodeValue = (opcodeValue & 0xF0000000) + destReg * 0x1000 + Type1ImmBit + MOVCode ;
                                    immValue = value ;
                                    value = MOVVal ;
                                    defined = RotatedForm(&rotator,&immValue,value,&opcodeValue) ;
                                    /* Throw away result here */
                                    CodeWord(opcodeValue + rotator * 0x100 + immValue) ;
                                    return FALSE ;
                                   }
                                  else
                                   immValue = AddLiteral(TRUE,value) ;
                                }
                               else
                                immValue = AddExternalLiteral(symbolPointer,value) ;
                              }
                             else
                              {
                               immValue = RegisterRelExpr(line,&lineIndex,&sourceReg1,&type,FALSE,&defined) ;
                               if (errorFound)
                                return FALSE ;

                               if (type == DPRelOT)
                                {
                                 SymbolPointer hlabel = (SymbolPointer)immValue ;
                                 /* "LDR <r>,[dp,#modnum_offset_of_label]" */
                                 write_patch(HOF_t_patch_armdt,LDRfromdp(destReg)) ;
                                 write_label(HOF_t_datamod,hlabel->key.key,hlabel->key.length) ;

                                 /* "[LDR[B]|STR[B]] <r>,[<r>,#offset]" */
                                 /* horrible piece of constant use */
                                 switch (opcodeValue & 0x0FFFFFFF)
                                  {
                                   case 0x04100000 : /* LDR<cc>  reg,symbol */
                                                     write_patch(HOF_t_patch_armdt,LDRfromoffset(destReg)) ;
                                                     break ;
                                   case 0x04500000 : /* LDR<cc>B reg,symbol */
                                                     write_patch(HOF_t_patch_armdt,LDRBfromoffset(destReg)) ;
                                                     break ;
                                   case 0x04000000 : /* STR<cc>  reg,symbol */
                                                     write_patch(HOF_t_patch_armdt,STRtooffset(destReg)) ;
                                                     break ;
                                   case 0x04400000 : /* STR<cc>B reg,symbol */
                                                     write_patch(HOF_t_patch_armdt,STRBtooffset(destReg)) ;
                                                     break ;
                                   default         : /* BAD NEWS */
                                                     printf("LDR/STR: Unknown opcode value &%08X\n",opcodeValue) ;
                                                     exit(1) ;
                                                     break ;
                                  }

                                 if ((hlabel->u.s.at == HCodeAT) || (hlabel->u.s.at == HCodeExportAT) || (hlabel->u.s.at == HCodeExportedAT))
                                  write_label(HOF_t_codesymbol,hlabel->key.key,hlabel->key.length) ;
                                 else
                                  write_label(HOF_t_dataref,hlabel->key.key,hlabel->key.length) ;
                                 break ;        /* exit "switch" */
                                }
                              }

                             opcodeValue += PreBit ;
                             if (sourceReg1 == 15)
                              immValue -= programCounter + 8 ;

                             if (abs((int)immValue) >= 0x1000)
                              {
                               Warning(DatOff) ;
                               return FALSE ;
                              }

                             if (immValue < 0x1000)
                              opcodeValue += UpBit ;

                             immValue = abs((int)immValue) ;

                             /* deal with write-back request */
                             if (line[lineIndex] == Shriek)
                              {
                               lineIndex++ ;
                               opcodeValue += WriteBackBit ;
                              }
                            }

                           if (hDIR.type != NoPatch)
                            {
                             int ptype = HOF_t_patch_armdt ;    /* by default */
                             switch (hDIR.type)
                              {
                               case ModOffPatch   : ptype = HOF_t_patch_armdt ;
                                                    break ;
                               case OffsetPatch   : ptype = HOF_t_patch_armdt ;
                                                    break ;
                               case LsbOffPatch   :
                               case MidOffPatch   :
                               case MsbOffPatch   : Warning(BadPatchUse) ;
                                                    break ;
                               case ModOffWOPatch : ptype = HOF_t_patch_armdt ;
                                                    break ;
                              }
                             write_patch(ptype,(opcodeValue + immValue + sourceReg1 * 0x10000 + destReg * 0x1000)) ;
                             if (hDIR.type == ModOffWOPatch)
                              write_modnum5() ;
                             else
                              {
                               if (hDIR.type == ModOffPatch)
                                write_label(HOF_t_datamod,hDIR.symbol->key.key,hDIR.symbol->key.length) ;
                               else
                                {
                                 if ((hDIR.symbol->u.s.at == HCodeAT) || (hDIR.symbol->u.s.at == HCodeExportAT) || (hDIR.symbol->u.s.at == HCodeExportedAT))
                                  write_label(HOF_t_codesymbol,hDIR.symbol->key.key,hDIR.symbol->key.length) ;
                                 else
                                  write_label(HOF_t_dataref,hDIR.symbol->key.key,hDIR.symbol->key.length) ;
                                }
                              }
                            }
                           else
                            CodeWord(opcodeValue + immValue + sourceReg1 * 0x10000 + destReg * 0x1000) ;
                           break ;

     case SWI            : value = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                           if (value >= 0x1000000)
                            {
                             Warning(ImmValRange) ;
                             return FALSE ;
                            }
                           CodeWord(opcodeValue + value) ;
                           break ;

     case BlockData      : blockDataBits = 0 ;
                           sourceReg1 = RegisterExpr(line,&lineIndex) ;
                           if (line[lineIndex] == Shriek)
                            {
                             lineIndex++ ;
                             while (line[lineIndex] == Space)
                              lineIndex++ ;
                             opcodeValue += WriteBackBit ;
                            }
                           lineIndex++ ;
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           lineIndex++ ; /* Past { */
                           do
                            {
                             reg1 = RegisterExpr(line,&lineIndex) ;
                             if (line[lineIndex] == Dash)
                              {
                               lineIndex++ ;
                               reg2 = RegisterExpr(line,&lineIndex) ;
                               if (reg2 < reg1)
                                {
                                 Warning(BadRegRange) ;
                                 return FALSE ;
                                }
                              }
                             else
                              reg2 = reg1 ;
                             while (reg1 <= reg2)
                              {
                               if ((1 << reg1) & blockDataBits)
                                {
                                 Warning(BadRegInBD) ;
                                 return FALSE ;
                                }
                               blockDataBits |= 1 << reg1 ;
                               reg1++ ;
                              } /* while */
                             if (line[lineIndex] != Comma)
                              break ;
                             lineIndex++ ;
                            } while (1) ;
                           lineIndex++ ; /* Past } */
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           if (line[lineIndex] == Hat)
                            {
                             lineIndex++ ;
                             opcodeValue += PSRBit ;
                            }
                           CodeWord(opcodeValue + sourceReg1 * 0x10000 + blockDataBits) ;
                           break ;

     case Branch         : /* Even Helios patched/modified branch instructions are a word long */
                           symbolPointer = ExternalExpr(line,&lineIndex,&value) ;
                           if (symbolPointer == NULL)
                            {
                             /* If undefined symbol, then we should generate a LABELREF to it */
                             SymbolScan(line,lineIndex,&name) ; /* get possible symbol */

                             if (traceon)
                              {
                               printf("pass2: Branch: undefined symbol \"") ;
                               PrintSymbol(name) ;
                               printf("\"\n") ;
                              }

                             value = AddressExpr(line,&lineIndex,TRUE,&defined) ;
                             if (errorFound)
                              return(FALSE) ;

                             if (!defined && allowUndefinedSymbols)
                              {
                               if (traceon)
                                {
                                 printf("Branch: generating LABELREF to \"") ;
                                 PrintSymbol(name) ;
                                 printf("\"\n") ;
                                }
                               write_patch(HOF_t_patch_armjp,opcodeValue) ;
                               /* assume we are trying to reference the STUB */
                               write_dotlabel(HOF_t_labelref,name.key,name.length) ;
                              }
                             else
                              {
                               if (!defined)
                                {
                                 Warning(UnDefSym) ;
                                 return(FALSE) ;
                                }

                              value -= programCounter + 8 ;

                              if (abs((int)value) > 0x4000000)
                               {
                                Warning(BrOff);
                                return FALSE;
                               }
                              CodeWord(opcodeValue + ((value % 0x4000000 + 3) >> 2)) ;
                             }
                            }
                           else
                            {
                              {
                               Name dummyname ;
                               char dummytext[MaxStringSize] ;

                               dummytext[0] = '.' ;     /* we are looking for the stub */
                               memcpy(&dummytext[1],&(symbolPointer->key.key[1]),(symbolPointer->key.length - 1)) ;
                               dummytext[symbolPointer->key.length] = '\0' ;
                               dummyname.key = dummytext ;
                               dummyname.length = strlen(dummytext) ;

                               symbolPointer = LookupRef(dummyname,TRUE) ;
                               if (symbolPointer == NULL)
                                {
                                 if (allowUndefinedSymbols)
                                  {
                                   if (traceon)
                                    {
                                     printf("Branch: generating LABELREF to \"") ;
                                     PrintSymbol(dummyname) ;
                                     printf("\"\n") ;
                                    }
                                   write_patch(HOF_t_patch_armjp,opcodeValue) ;
                                   write_purelabel(HOF_t_labelref,dummyname.key,dummyname.length) ;
                                   return(FALSE) ;
                                  }
                                 else
                                  {
                                   Warning(NoStubFN) ;
                                   return(FALSE) ;
                                  }
                                }

                               value = symbolPointer->value.card - (programCounter + 8) ;

                               CodeWord(opcodeValue + ((value >> 2) & 0x00FFFFFF)) ;
                               break ;
                              }

                             /* We should never get to this point when producing Helios code */
                             printf("**FATAL** branch: should never reach this point when creating Helios code\n") ;
                             return(TRUE) ; /* terminate the assembly */
                             CodeWord(opcodeValue + ((value >> 2) & 0x00ffffff)) ;
                            }
                           break ;

     case Adr            : destReg = RegisterExpr(line,&lineIndex) ;

                           /* notify the user if we are unhappy about his register choice */
                           if (destReg == 9)
                            Notification(BadADRL) ;

                           lineIndex++ ;
                           immValue = NotStringExpr(line,&lineIndex,&sourceReg1,&type,FALSE,&defined) ;
                           if (errorFound)
                            return FALSE ;

                           switch (type)
                            {
                             case ConstantOT : opcodeValue += MOVCode ;
                                               value = MOVVal ;
                                               break ;

                             case PCRelOT    : immValue -= programCounter + 8 ;
                                               opcodeValue += ADDCode ;
                                               value = ADDVal ;
                                               break ;

                             case DPRelOT    : {
                                                SymbolPointer hlabel = (SymbolPointer)immValue ;
                                                /* "LDR <r>,[dp,#modnum_offset_of_label]" */
                                                write_patch(HOF_t_patch_armdt,LDRfromdp(destReg)) ;
                                                write_label(HOF_t_datamod,hlabel->key.key,hlabel->key.length) ;
                                                /* "ADD <r>,<r>,#offset" */
                                                write_patch(HOF_t_patch_armdp,ADDwithoffset(destReg)) ;
                                                if ((hlabel->u.s.at == HCodeAT) || (hlabel->u.s.at == HCodeExportAT) || (hlabel->u.s.at == HCodeExportedAT))
                                                 write_label(HOF_t_codesymbol,hlabel->key.key,hlabel->key.length) ;
                                                else
                                                 write_label(HOF_t_dataref,hlabel->key.key,hlabel->key.length) ;
                                               }
                                               break ;

                             case RegRelOT   : opcodeValue += ADDCode + sourceReg1 * 0x10000 - 0xF0000 ;
                                               value = ADDVal ;
                            }

                           if (type != DPRelOT)
                            {
                             if (RotatedForm(&rotator,&immValue,value,&opcodeValue))
                              return FALSE ;

                             CodeWord(opcodeValue + (rotator*0x100) + immValue + (destReg*0x1000)) ;
                            }
                           break ;

     case ADRL           : destReg = RegisterExpr(line,&lineIndex) ;
                           if (destReg == 9)
                            Notification(BadADRL) ;
                           lineIndex++ ;
                           immValue = NotStringExpr(line,&lineIndex,&sourceReg1,&type,FALSE,&defined) ;
                           if (errorFound)
                            return FALSE ;

                           switch (type)
                            {
                             case ConstantOT : Warning(ImmValRange) ;
                                               return(FALSE) ;
                                               break ;

                             case PCRelOT    : immValue -= programCounter + 8 ;
                                               opcodeValue += ADDCode ;
                                               value = ADDVal ;
                                               sourceReg1 = 0x0F ;
                                               break ;

                             case DPRelOT    : {
                                                SymbolPointer hlabel = (SymbolPointer)immValue ;
                                                /* "LDR <r>,[dp,#modnum_offset_of_label]" */
                                                write_patch(HOF_t_patch_armdt,LDRfromdp(destReg)) ;
                                                write_label(HOF_t_datamod,hlabel->key.key,hlabel->key.length) ;

                                                /* "ADD <r>,<r>,#(:LSB: offset)" */
                                                write_patch(HOF_t_patch_armdplsb,ADDwithoffset(destReg)) ;
                                                if ((hlabel->u.s.at == HCodeAT) || (hlabel->u.s.at == HCodeExportAT) || (hlabel->u.s.at == HCodeExportedAT))
                                                 write_label(HOF_t_codesymbol,hlabel->key.key,hlabel->key.length) ;
                                                else
                                                 write_label(HOF_t_dataref,hlabel->key.key,hlabel->key.length) ;

                                                /* "ADD <r>,<r>,#(:MSB: offset)" */
                                                write_patch(HOF_t_patch_armdprest,ADDwithoffset(destReg)) ;
                                                if ((hlabel->u.s.at == HCodeAT) || (hlabel->u.s.at == HCodeExportAT) || (hlabel->u.s.at == HCodeExportedAT))
                                                 write_label(HOF_t_codesymbol,hlabel->key.key,hlabel->key.length) ;
                                                else
                                                 write_label(HOF_t_dataref,hlabel->key.key,hlabel->key.length) ;
                                               }
                                               break ;

                             case RegRelOT   : opcodeValue += ADDCode + sourceReg1 * 0x10000 - 0xF0000 ;
                                               value = ADDVal ;
                            } /* switch */

                           if (type != DPRelOT)
                            {
                             if (Rotated16Form(&rotator,&rotator1,&immValue1,&immValue,value,&opcodeValue))
                              return FALSE ;
                             CodeWord(opcodeValue + rotator*0x100 + immValue % 0x100 + destReg*0x1000) ;
                             CodeWord(opcodeValue + rotator1*0x100 + immValue1 + destReg*0x11000 - sourceReg1*0x10000) ;
                            }
                           break ;

     case SWP            : destReg = RegisterExpr(line,&lineIndex) ; /* Destination */
                           lineIndex++ ;
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           sourceReg1 = RegisterExpr(line,&lineIndex) ; /* Source */
                           lineIndex++ ; /* Step past comma */
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           lineIndex++ ; /* Step past square bra */
                           sourceReg2 = RegisterExpr(line,&lineIndex) ;
                           lineIndex++ ; /* Step past square ket */
                           CodeWord(opcodeValue + sourceReg1 + sourceReg2 * 0x10000 + destReg * 0x1000) ;
                           break ;

     case FPDataTransfer : destReg = FPRegisterExpr(line,&lineIndex) ; /* Destination/source */
                           lineIndex++ ;
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           if (line[lineIndex] == SquareBra)
                            {
                             lineIndex++ ;
                             sourceReg1 = RegisterExpr(line,&lineIndex) ;
                             /* Here we split according to pre/post indexing */
                             if (line[lineIndex] == SquareKet)
                              {
                               /* This is the post-indexed form */
                               lineIndex++ ;
                               while (line[lineIndex] == Space)
                                lineIndex++ ;
                               if (line[lineIndex] != Comma)
                                {
                                 opcodeValue += UpBit ;
                                 /* Allow optional shriek */
                                 if (line[lineIndex] == Shriek)
                                  lineIndex++ ;
                                 immValue = 0 ;
                                }
                               else
                                {
                                 /* Definitely post-indexed */
                                 lineIndex++ ;
                                 while (line[lineIndex] == Space)
                                  lineIndex++ ;
                                 lineIndex++ ;
                                 immValue = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                                 if (errorFound)
                                  return FALSE ;
                                 if ((abs((int)immValue) >= 0x400) || (immValue % 4 != 0))
                                  {
                                   Warning(DatOff) ;
                                   return FALSE ;
                                  }
                                 if (immValue < 0x400)
                                  opcodeValue += UpBit ;
                                 immValue = abs((int)immValue) / 4 ;
                                }
                               opcodeValue += WriteBackBit ; /* We must write back */
                              }
                             else
                              {
                               /* This is definitely pre-indexed */
                               opcodeValue += PreBit ;
                               lineIndex++ ; /* Past comma */
                               while (line[lineIndex] == Space)
                                lineIndex++ ;
                               lineIndex++ ; /* Past hash */
                               immValue = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                               if (errorFound)
                                return FALSE ;
                               if ((abs((int)immValue) >= 0x400) || (immValue % 4 != 0))
                                {
                                 Warning(DatOff) ;
                                 return FALSE ;
                                }
                               if (immValue < 0x400)
                                opcodeValue += UpBit ;
                               immValue = abs((int)immValue) / 4 ;
                               lineIndex++ ; /* Past ket */
                               while (line[lineIndex] == Space)
                                lineIndex++ ;
                               if (line[lineIndex] == Shriek)
                                {
                                 lineIndex++ ;
                                 opcodeValue += WriteBackBit ;
                                }
                              }
                            }
                           else
                            {
                             /* Here we look for a PC or register relative expression
                              * or a literal.
                              */
                             if (line[lineIndex] == Equals)
                              {
                               lineIndex++ ;
                               if ((1 << DoubleBit) & opcodeValue)
                                fPSize = Double ;
                               else
                                fPSize = Single ;
                               readResponse = hRead(line,&lineIndex,fPSize,&value1,&value2) ;
                               switch (fPSize)
                                {
                                 case Single : immValue = AddFPLiteralSingle(value1) ;
                                               break ;

                                 case Double : immValue = AddFPLiteralDouble(value1,value2) ;
                                }
                               sourceReg1 = 15 ;
                              }
                             else
                              {
                               immValue = RegisterRelExpr(line,&lineIndex,&sourceReg1,&type,FALSE,&defined) ;
                               if (errorFound)
                                return FALSE ;
                              }
                             opcodeValue += PreBit ;
                             if (sourceReg1 == 15)
                              immValue -= programCounter + 8 ;
                             if ((abs((int)immValue) >= 0x400) || (immValue % 4 != 0))
                              {
                               Warning(DatOff) ;
                               return FALSE ;
                              }
                             if (immValue < 0x400)
                              opcodeValue += UpBit ;
                             immValue = abs((int)immValue) / 4 ;
                             if (line[lineIndex] == Shriek)
                              {
                               lineIndex++ ;
                               opcodeValue += WriteBackBit ;
                              }
                            }
                           CodeWord(opcodeValue + immValue + sourceReg1 * 0x10000 + destReg * 0x1000) ;
                           break ;

     case FPDyadic       : destReg = FPRegisterExpr(line,&lineIndex) ; /* Read the destination */
                           lineIndex++ ; /* Step past comma */
                           sourceReg1 = FPRegisterExpr(line,&lineIndex) ; /* Read first source */
                           lineIndex++ ; /* Step past comma */
                           /* Now look for immediate or register form */
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           if (line[lineIndex] == Hash)
                            {
                             lineIndex++ ;
                             /* Now look for immediate form */
                             immValue = FPConstant(line,&lineIndex) ;
                             /* Now update opcodeValue */
                             opcodeValue += immValue + NOTFPRegisterBit ;
                            }
                           else
                            opcodeValue += FPRegisterExpr(line,&lineIndex) ;
                           CodeWord(opcodeValue + destReg * 0x1000 + sourceReg1 * 0x10000) ;
                           break ;

     case FPMonadic      :
     case FPCompare      :
                           destReg = FPRegisterExpr(line, &lineIndex) ;
                           lineIndex++ ;
                           /* Now look for immediate or register form */
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           if (line[lineIndex] == Hash)
                            {
                             lineIndex++ ;
                             /* Now look for immediate form */
                             immValue = FPConstant(line,&lineIndex) ;
                             opcodeValue += immValue + NOTFPRegisterBit ;
                            }
                           else
                            {
                             /* Now look for register form */
                             opcodeValue += FPRegisterExpr(line,&lineIndex) ;
                            }
                           if (opcodeType == FPCompare)
                            CodeWord(opcodeValue + destReg * 0x10000) ;
                           else
                            CodeWord(opcodeValue + destReg * 0x1000) ;
                           break ;

     case FPFloat        : opcodeValue += FPRegisterExpr(line,&lineIndex) * 0x10000 ;
                           lineIndex++ ;
                           opcodeValue += RegisterExpr(line,&lineIndex) * 0x1000 ;
                           CodeWord(opcodeValue) ;
                           break ;

     case FPFix          : opcodeValue += RegisterExpr(line,&lineIndex) * 0x1000 ;
                           lineIndex++ ;
                           opcodeValue += FPRegisterExpr(line,&lineIndex) ;
                           CodeWord(opcodeValue) ;
                           break ;

     case FPStatus       : CodeWord(opcodeValue + RegisterExpr(line,&lineIndex) * 0x1000) ;
                           break ;

     case CPDT           : cop = CPNameExpr(line,&lineIndex) ;
                           lineIndex++ ;
                           destReg = CPRegisterExpr(line,&lineIndex) ; /* Destination/source */
                           lineIndex++ ;
                           while (line[lineIndex] == Space)
                            lineIndex++ ;
                           if (line[lineIndex] == SquareBra)
                            {
                             lineIndex++ ;
                             sourceReg1 = RegisterExpr(line,&lineIndex) ;
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
                                 opcodeValue += UpBit ;
                                 /* Allow optional shriek */
                                 if (line[lineIndex] == Shriek)
                                  lineIndex++ ;
                                 immValue = 0 ;
                                }
                               else
                                {
                                 /* Definitely post-indexed */
                                 lineIndex++ ;
                                 while (line[lineIndex] == Space)
                                  lineIndex++ ;
                                 lineIndex++ ;
                                 immValue = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                                 if (errorFound)
                                  return FALSE ;
                                 if ((abs((int)immValue) >= 0x400) || (immValue % 4 != 0))
                                  {
                                   Warning(DatOff) ;
                                   return FALSE ;
                                  }
                                 if (immValue < 0x400)
                                  opcodeValue += UpBit ;
                                 immValue = (abs((int)immValue)) / 4 ;
                                }
                               opcodeValue += WriteBackBit ; /* We must write back */
                              }
                             else
                              {
                               /* This is definitely pre-indexed */
                               opcodeValue += PreBit ;
                               lineIndex++ ;
                               while (line[lineIndex] == Space)
                                lineIndex++ ;
                               lineIndex++ ; /* Past hash */
                               immValue = ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) ;
                               if (errorFound)
                                return FALSE ;
                               if ((abs((int)immValue) >= 0x400) || (immValue % 4 != 0))
                                {
                                 Warning(DatOff) ;
                                 return FALSE ;
                                }
                               if (immValue < 0x400)
                                opcodeValue += UpBit ;
                               immValue = abs((int)immValue) / 4 ;
                               lineIndex++ ;
                               while (line[lineIndex] == Space)
                                lineIndex++ ;
                               if (line[lineIndex] == Shriek)
                                {
                                 lineIndex++ ;
                                 opcodeValue += WriteBackBit ;
                                }
                              }
                            }
                           else
                            {
                             immValue = RegisterRelExpr(line,&lineIndex,&sourceReg1,&type,FALSE,&defined) ;
                             if (errorFound)
                              return FALSE ;
                             opcodeValue += PreBit ;
                             if (sourceReg1 == 15)
                              immValue -= programCounter + 8 ;
                             if ((abs((int)immValue) >= 0x400) || (immValue % 4 != 0))
                              {
                               Warning(DatOff) ;
                               return FALSE ;
                              }
                             if (immValue < 0x400)
                              opcodeValue += UpBit ;
                             immValue = abs((int)immValue) / 4 ;
                             if (line[lineIndex] == Shriek)
                              {
                               lineIndex++ ;
                               opcodeValue += WriteBackBit ;
                              }
                            }
                           CodeWord(opcodeValue + immValue + sourceReg1 * 0x10000 + destReg * 0x1000 + cop * 0x100) ;
                           break ;

     case CPDO           : cop = CPNameExpr(line,&lineIndex) ;
                           lineIndex++ ;
                           opcodeValue += ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) * 0x100000 ;
                           lineIndex++ ;
                           destReg = CPRegisterExpr(line,&lineIndex) ;
                           lineIndex++ ;
                           sourceReg1 = CPRegisterExpr(line,&lineIndex) ;
                           lineIndex++ ;
                           sourceReg2 = CPRegisterExpr(line,&lineIndex) ;
                           if (line[lineIndex] == Comma)
                            {
                             lineIndex++ ;
                             opcodeValue += ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) * 0x20 ;
                            }
                           CodeWord(opcodeValue + sourceReg1 * 0x10000 + sourceReg2 + destReg * 0x1000 + cop * 0x100) ;
                           break ;

     case CPRT           : cop = CPNameExpr(line,&lineIndex) ;
                           lineIndex++ ;
                           opcodeValue += ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) * 0x200000 ;
                           lineIndex++ ;
                           destReg = RegisterExpr(line,&lineIndex) ;
                           lineIndex++ ;
                           sourceReg1 = CPRegisterExpr(line,&lineIndex) ;
                           lineIndex++ ;
                           sourceReg2 = CPRegisterExpr(line,&lineIndex) ;
                           if (line[lineIndex] == Comma)
                            {
                             lineIndex++ ;
                             opcodeValue += ConstantExpr(line,&lineIndex,FALSE,&defined,NULL) * 0x20 ;
                            }
                           CodeWord(opcodeValue + sourceReg1 * 0x10000 + sourceReg2 + destReg * 0x1000 + cop * 0x100) ;
                           break ;

     case MUL            : destReg = RegisterExpr(line,&lineIndex) ; /* Read the destination */
                           lineIndex++ ; /* Step past */
                           sourceReg1 = RegisterExpr(line,&lineIndex) ; /* Read the first source */
                           lineIndex++ ; /* Step past */
                           sourceReg2 = RegisterExpr(line,&lineIndex) ; /* Read the second source */
                           CodeWord(opcodeValue + destReg * 0x10000 + sourceReg1 + sourceReg2 * 0x100) ;
                           break ;

     case MLA            : destReg = RegisterExpr(line,&lineIndex) ; /* Read the destination */
                           lineIndex++ ;
                           sourceReg1 = RegisterExpr(line,&lineIndex) ; /* Read the first source */
                           lineIndex++ ;
                           sourceReg2 = RegisterExpr(line,&lineIndex) ; /* Read the second source */
                           lineIndex++ ;
                           value = RegisterExpr(line,&lineIndex) ;
                           CodeWord(opcodeValue + destReg * 0x10000 + sourceReg1 + sourceReg2 * 0x100 + value * 0x1000) ;
    }
  }
 return FALSE ;
} /* End P2LineHandler */

/*---------------------------------------------------------------------------*/
/* EOF p2hand/c */
