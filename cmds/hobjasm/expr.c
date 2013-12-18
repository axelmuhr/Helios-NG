/* -> expr/c
 * Title:               Expression parser
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "p1hand.h"
#include "apply.h"
#include "code.h"
#include "constant.h"
#include "errors.h"
#include "exstore.h"
#include "extypes.h"
#include "expr.h"
#include "getline.h"
#include "globvars.h"
#include "llabel.h"
#include "nametype.h"
#include "asmvars.h"
#include "occur.h"
#include "store.h"
#include "tables.h"
#include "tokens.h"
#include "vars.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

static char       exprString[MaxStringSize] ;
static CARDINAL   stackPointer ;
static BOOLEAN    initialised = FALSE ;
static Name       shiftTable[6] ; /* 5 => 0 */
static StringName stringName ;

/*---------------------------------------------------------------------------*/
/* Initialise table of shifters */
static void ShiftInit(void)
{
 CopyName(0,"ASL",shiftTable) ;
 CopyName(1,"LSR",shiftTable) ;
 CopyName(2,"ASR",shiftTable) ;
 CopyName(3,"ROR",shiftTable) ;
 CopyName(4,"RRX",shiftTable) ;
 CopyName(5,"LSL",shiftTable) ;
 initialised = TRUE ;
} /* End ShiftInit */

/*---------------------------------------------------------------------------*/
/*This actually returns a twelve bit value suitable for ORing into the opcode*/
CARDINAL ShiftedRegister(char *line,CARDINAL *index,BOOLEAN allowRegisterShift)
{
 CARDINAL value ;
 CARDINAL shift ;
 CARDINAL shifter ;
 Name     name ;
 BOOLEAN  defined ;

 if (!initialised)
  ShiftInit() ;
 value = RegisterExpr(line,index) ;
 if (errorFound)
  return 0 ;
 if (line[*index] != Comma)
  return value ;

 (*index)++ ;
 /* Now sort out the shifted part */
 while (line[*index] == Space)
  (*index)++ ;
 if (!SymbolTest(line,index,&name))
  {
   Warning(BadShift) ;
   return 0 ;
  }
 if ((name.length != 3) || !NameLookup(shiftTable,name,FALSE,&shifter,6))
  {
   Warning(UnkShift) ;
   return 0 ;
  }
 /* Now we may look for further register or immediate operand */
 if (shifter == 5)
  shifter = 0 ;
 if (shifter == 4)
  return(value + 0x60) ;
 while (line[*index] == Space)
  (*index)++ ;
 if (line[*index] != Hash)
  {
   if (allowRegisterShift)
    return RegisterExpr(line,index)*0x100 + shifter*0x20 + 0x10 + value ;
   Warning(HashMissing) ;
   return 0 ;
  }
 (*index)++ ;
 shift = ConstantExpr(line,index,FALSE,&defined,NULL) ;
 if (errorFound)
  return 0 ;
 switch (shifter)
  {
   case 0 : if (shift <= 31)
             return shift*0x80 + value ;
            break ;
 
   case 1 : if ((shift >= 1) && (shift <= 32))
             return (shift % 32)*0x80 + 0x20 + value ;
            break ;
 
   case 2 : if ((shift >= 1) && (shift <= 32))
             return (shift % 32)*0x80 + 0x40 + value ;
            break ;
 
   case 3 : if ((shift >= 1) && (shift <= 31))
             return shift*0x80 + 0x60 + value ;
            break ;
  }
 Warning(ShiftOpt) ;
 return 0 ;
} /* End ShiftedRegister */

/*---------------------------------------------------------------------------*/
/* Just read and return the value of a register symbol */
CARDINAL AnyRegisterExpr(char *line,CARDINAL *index,BOOLEAN *Int)
{
 Name          name ;
 SymbolPointer symbolPointer ;

 while (line[*index] == Space)
  (*index)++ ;
 if (!SymbolTest(line,index,&name))
  {
   Warning(BadSym) ;
   return 0 ;
  }
 while (line[*index] == Space)
  (*index)++ ;
 symbolPointer = LookupRef(name,FALSE) ;
 if ((symbolPointer == NULL) || (symbolPointer->u.s.sdt != RegisterNameSDT) || ((symbolPointer->u.s.rt != IntRT) && (symbolPointer->u.s.rt != RealRT)))
  {
   Warning(BadReg) ;
   return 0 ;
  }
 *Int = symbolPointer->u.s.rt == IntRT ;
 AddUse(symbolPointer) ;
 return symbolPointer->value.card ;
} /* End AnyRegisterExpr */

/*---------------------------------------------------------------------------*/
/* Just read and return the value of a register symbol */
CARDINAL RegisterExpr(char *line,CARDINAL *index)
{
 Name          name ;
 SymbolPointer symbolPointer ;

 while (line[*index] == Space)
  (*index)++ ;
 if (!SymbolTest(line,index,&name))
  {
   Warning(BadSym) ;
   return 0 ;
  }
 while (line[*index] == Space)
  (*index)++ ;
 symbolPointer = LookupRef(name,FALSE) ;
 if ((symbolPointer == NULL) || (symbolPointer->u.s.sdt != RegisterNameSDT) || (symbolPointer->u.s.rt != IntRT))
  {
   Warning(BadReg) ;
   return 0 ;
  }
 AddUse(symbolPointer) ;
 return symbolPointer->value.card ;
} /* End RegisterExpr */

/*---------------------------------------------------------------------------*/
/* Just read and return the value of a floating point register symbol */

CARDINAL FPRegisterExpr(char *line,CARDINAL *index)
{
 Name          name ;
 SymbolPointer symbolPointer ;

 while (line[*index] == Space)
  (*index)++ ;
 if (!SymbolTest(line, index, &name))
  {
   Warning(BadSym) ;
   return 0 ;
  }
 while (line[*index] == Space)
  (*index)++ ;
 symbolPointer = LookupRef(name,FALSE) ;
 if ((symbolPointer == NULL) || (symbolPointer->u.s.sdt != RegisterNameSDT) || (symbolPointer->u.s.rt != RealRT))
  {
   Warning(BadReg) ;
   return 0 ;
  }
 if (symbolPointer->value.card >= 16)
  Warning(FPRegRange) ;
 AddUse(symbolPointer) ;
 return symbolPointer->value.card ;
} /* End FPRegisterExpr */

/*---------------------------------------------------------------------------*/
/* Just read and return the value of a coprocessor register symbol */

CARDINAL CPRegisterExpr(char *line,CARDINAL *index)
{
 Name          name ;
 SymbolPointer symbolPointer ;

 while (line[*index] == Space)
  (*index)++ ;
 if (!SymbolTest(line,index,&name))
  {
   Warning(BadSym) ;
   return 0 ;
  }
 while (line[*index] == Space)
  (*index)++ ;
 symbolPointer = LookupRef(name,FALSE) ;
 if ((symbolPointer == NULL) || (symbolPointer->u.s.sdt != RegisterNameSDT) || (symbolPointer->u.s.rt != CopRRT))
  {
   Warning(BadReg) ;
   return 0 ;
  }
 if (symbolPointer->value.card >= 16)
  Warning(CPRegRange) ;
 AddUse(symbolPointer) ;
 return symbolPointer->value.card ;
} /* End CPRegisterExpr */

/*---------------------------------------------------------------------------*/
/* Just read and return the value of a coprocessor name symbol */

CARDINAL CPNameExpr(char *line,CARDINAL *index)
{
 Name          name ;
 SymbolPointer symbolPointer ;

 while (line[*index] == Space)
  (*index)++ ;
 if (!SymbolTest(line,index,&name))
  {
   Warning(BadSym) ;
   return 0 ;
  }
 while (line[*index] == Space)
  (*index)++ ;
 symbolPointer = LookupRef(name,FALSE) ;
 if ((symbolPointer == NULL) || (symbolPointer->u.s.sdt != RegisterNameSDT) || (symbolPointer->u.s.rt != CopNRT))
  {
   Warning(BadReg) ;
   return 0 ;
  }
 if (symbolPointer->value.card >= 16)
  Warning(CPNameRange) ;
 AddUse(symbolPointer) ;
 return symbolPointer->value.card ;
} /* End CPNameExpr */

/*---------------------------------------------------------------------------*/

BOOLEAN LogicalExpr(char *line,CARDINAL *index)
{
 BOOLEAN defined ;
 return AssertExpr(line,index,FALSE,&defined) ;
} /* End LogicalExpr */

/*---------------------------------------------------------------------------*/
/* Evaluate as far as possible an expression */

#define HeapSize 1024 /* Amount of heap allocated for string bashing */

static void Expression(char *line,CARDINAL *index,BOOLEAN allowUnd,BOOLEAN *defined,EHeliosPatch *hDIR)
{
 char         exprHeap[HeapSize] ;
 EStackEntry  eStackEntry ;
 Operator     prevOp ;
 Operator     op ;
 BOOLEAN      tokDefined ;
 CARDINAL     i ;

#ifdef DEBUG
 printf("DEBUG: Expression: \"eStack\" at &%08X\n",(int)eStack) ; 
#endif

 hDIR->type = NoPatch ;

 InitExprStore(exprHeap,HeapSize) ;
 InitTokens() ;
 *defined = TRUE ; /* So far the expression is defined */
 stackPointer = 0 ;

 /* Initialise the stack */
 eStack[0].type = OperatorEST ;
 eStack[0].u.operator = STBot ;
 prevOp = STBot ;
 op = STBot ;
  
 /* The start of the evaluation loop */
 do
  {
   stackPointer++ ;
   Token(line,index,&eStack[stackPointer],&tokDefined,hDIR) ;
   if (errorFound)
    return ;
   if (eStack[stackPointer].type == OperatorEST)
    {
     prevOp = op ;
     /* Better be unary or Bra otherwise we shall moan */
     if (eStack[stackPointer].u.operator == Plus)
      eStack[stackPointer].u.operator = UPlus ;
     else
      if (eStack[stackPointer].u.operator == Minus)
       eStack[stackPointer].u.operator = UMinus ;

     op = eStack[stackPointer].u.operator ;
     if ((op != Bra) && (op < UPlus))
      {
       /* It's a binary operator so it's an error and we give up */
       Warning(UnExpOp) ;
       return ;
      }
    }
   else
    {
     /* Operand: update expression definition status */
     *defined = *defined && tokDefined ;

     if (!(*defined || allowUnd))
      {
       Warning(UnDefSym) ;
       return ;
      }

     /* Now we go look for an operator */
     do
      {
       if (AllComment(line,index) || (line[*index] == ']'))
        {
         prevOp = op ;
         op = STTop ;
        }
       else
        {
         Token(line,index,&eStackEntry,&tokDefined,hDIR) ;
         if (errorFound)
          return ;

         if (eStackEntry.type != OperatorEST)
          {
           Warning(UnExpOpnd) ;
           return ;
          }
         prevOp = op ;
         op = eStackEntry.u.operator ;
         if (op >= UPlus)
          {
           Warning(UnExpUnOp) ;
           return ;
          }
        }

       if (op == STTop)
        {
         while ((prevOp != STBot) && !errorFound) /* Apply previous operator until run out of them */
          Apply(&prevOp,&stackPointer) ;
         if (errorFound)
          return ;

         /* Here we've arrived at the bottom of the stack
          * so check we're not returning a string that's going to go away
          */
         if (eStack[1].u.operand.operandType == StringOT)
          {
           for (i = 1; i <= eStack[1].u.operand.u.string.length; i++)
            exprString[i-1] = eStack[1].u.operand.u.string.key[i-1] ;
           eStack[1].u.operand.u.string.key = exprString ;
          }
         return ;
        }

       /* Now we're sure it's an operator */
       if (op == Ket)
        {
         while ((prevOp != Bra) && (prevOp != STBot) && !errorFound)
          Apply(&prevOp,&stackPointer) ;
         if (errorFound)
          return ;

         if (prevOp == STBot)
           {
            Warning(BraMiss) ;
            return ;
           }
         /* Throw away Bra and Ket and look for an operator */
         ShuffleDown(stackPointer-1,stackPointer,&prevOp) ;
         stackPointer-- ;
         op = prevOp ;
        }
       else
        {
         while ((priorities[prevOp] >= priorities[op]) && (prevOp != Bra) && !errorFound)
          /* Here we attempt to do the previous operator */
          Apply(&prevOp,&stackPointer) ;
         if (errorFound)
          return ;
         /* Just stack the current operator as it's of higher priority */
         stackPointer++ ;
         eStack[stackPointer].type = OperatorEST ;
         eStack[stackPointer].u.operator = op ;
         break ; /* Going to look for an operand now */
        }
      } while (1) ;
    }
  } while (1) ;
} /* End Expression */

/*---------------------------------------------------------------------------*/

BOOLEAN AssertExpr(char *line,CARDINAL *index,BOOLEAN allowUnd,BOOLEAN *defined)
{
 EHeliosPatch *localhDIR = mymalloc(sizeof(EHeliosPatch)) ;
 Expression(line,index,allowUnd,defined,localhDIR) ;
 if (localhDIR->type != NoPatch)
  Warning(BadOpUse) ;
 free(localhDIR) ;

 if (!errorFound && *defined)
  {
   if (eStack[1].u.operand.operandType == LogicalOT)
    return eStack[1].u.operand.u.bool ;
   Warning(BadExprType) ;
  }
 return FALSE ;
} /* End AssertExpr */

/*---------------------------------------------------------------------------*/

CARDINAL FPConstant(char *line,CARDINAL *index)
{
 char ch ;

 while (line[*index] == Space)
  (*index)++ ;
 ch = line[*index] ;
 (*index)++ ;
 switch (ch)
  {
   case '0' :
              if (line[*index] != Dot)
               return 0 ;
              (*index)++ ;
              if (line[*index] != '5')
               {
                Warning(BadFPCon) ;
                return 0 ;
               }
              (*index)++ ;
              return 6 ;

   case '1' :
              if (line[*index] != '0')
               return 1 ;
              (*index)++ ;
              return 7 ;

   case '2' :
   case '3' :
   case '4' :
   case '5' :
              return ch - '0' ;

   default  :
              (*index)++ ;
              Warning(BadFPCon) ;
              return 0 ;
  }
} /* End FPConstant */

/*---------------------------------------------------------------------------*/

CARDINAL ConstantExpr(char *line,CARDINAL *index,BOOLEAN allowUnd,BOOLEAN *defined,EHeliosPatch *hDIR)
{
 EHeliosPatch *localhDIR = hDIR ;

 /* If a NULL "hDIR" structure pointer is given, then generate a suitable
  * error if any of the Helios patch directives are used.
  */
 if (hDIR == NULL)
  localhDIR = (EHeliosPatch *)mymalloc(sizeof(EHeliosPatch)) ;

 Expression(line,index,allowUnd,defined,localhDIR) ;
 if ((hDIR == NULL) && (localhDIR->type != NoPatch))
  Warning(BadOpInstruction) ;

 if (hDIR == NULL)
  free(localhDIR) ;

 if (errorFound || !*defined)
  return 0 ;

 switch (eStack[1].u.operand.operandType)
  {
   case ConstantOT : return eStack[1].u.operand.u.constant ;

   case StringOT   : if (eStack[1].u.operand.u.string.length == 1)
                      return eStack[1].u.operand.u.string.key[0] ;
  }

 Warning(BadExprType) ;
 return 0 ;
} /* End ConstantExpr */

/*---------------------------------------------------------------------------*/

CARDINAL ConstantOrAddressExpr(char *line,CARDINAL *index,OperandType *type,BOOLEAN allowUnd,BOOLEAN *defined)
{
 EHeliosPatch *localhDIR = mymalloc(sizeof(EHeliosPatch)) ;
 Expression(line,index,allowUnd,defined,localhDIR) ;
 if (localhDIR->type != NoPatch)
  Warning(BadOpUse) ;
 free(localhDIR) ;

 if (!errorFound && *defined)
  {
   *type = eStack[1].u.operand.operandType ;
   switch (*type)
    {
     case ConstantOT :
                       return eStack[1].u.operand.u.constant ;

     case PCRelOT    :
                       return eStack[1].u.operand.u.pLabel ;

     case StringOT   :
                       if (eStack[1].u.operand.u.string.length == 1)
                        {
                         *type = ConstantOT ;
                         return eStack[1].u.operand.u.string.key[0] ;
                        }
    }
   Warning(BadExprType) ;
   return 0 ;
  }
 return 0 ;
} /* End ConstantOrAddressExpr */

/*---------------------------------------------------------------------------*/

CARDINAL NotStringExpr(char *line,CARDINAL *index,CARDINAL *reg,OperandType *type,BOOLEAN allowUnd,BOOLEAN *defined)
{
 CARDINAL i ;
 CARDINAL j ;
 EHeliosPatch *localhDIR = mymalloc(sizeof(EHeliosPatch)) ;

 Expression(line,index,allowUnd,defined,localhDIR) ;
 if (localhDIR->type != NoPatch)
  Warning(BadOpUse) ;
 free(localhDIR) ;

 if (errorFound || !*defined)
  return 0 ;

 *type = eStack[1].u.operand.operandType ;

 switch (*type)
  {
   case ConstantOT : return eStack[1].u.operand.u.constant;

   case PCRelOT    : return eStack[1].u.operand.u.pLabel ;

   case DPRelOT    : return((CARDINAL)eStack[1].u.operand.u.hSymbol) ;

   case RegRelOT   : j = 0;
                     for (i = 0; i <= 15; i++)
                      {
                       j += abs(eStack[1].u.operand.u.regRel.registers[i]) ;

                       /* Could be negative */
                       if (eStack[1].u.operand.u.regRel.registers[i] != 0)
                        *reg = i ;
                      }
                     if (j == 1)
                      return eStack[1].u.operand.u.regRel.offset ;
                     break ;

   case StringOT   : if (eStack[1].u.operand.u.string.length == 1)
                      {
                       *type = ConstantOT ;
                       return eStack[1].u.operand.u.string.key[0] ;
                      }
  }
 Warning(BadExprType) ;
 return 0 ;
} /* End NotStringExpr */

/*---------------------------------------------------------------------------*/

CARDINAL AddressExpr(char *line,CARDINAL *index,BOOLEAN allowUnd,BOOLEAN *defined)
{
 EHeliosPatch *localhDIR = mymalloc(sizeof(EHeliosPatch)) ;

 Expression(line,index,allowUnd,defined,localhDIR) ;
 if (localhDIR->type != NoPatch)
  Warning(BadOpUse) ;
 free(localhDIR) ;

 if (!errorFound && *defined)
  {
   switch (eStack[1].u.operand.operandType)
    {
     case PCRelOT    : /* printf("**DEBUG**: AddressExpr PCRelOT\n") ; */
                       return(eStack[1].u.operand.u.pLabel) ;

     case ConstantOT : /* printf("**DEBUG**: AddressExpr ConstantOT\n") ; */
                       /* constants not allowed */
                       break ;
     case DPRelOT    : /* printf("**DEBUG**: AddressExpr DPRelOT\n") ; */
                       return((CARDINAL)&(eStack[1].u.operand.u.hSymbol)) ;
    }

/* printf("AddressExpr: error\n") ; */
   Warning(BadExprType) ;
   return 0 ;
  }
 return 0 ;
} /* End AddressExpr */

/*---------------------------------------------------------------------------*/

void StringExpr(char *line,CARDINAL *index,Name *string)
{
 BOOLEAN defined ;
 EHeliosPatch *localhDIR = mymalloc(sizeof(EHeliosPatch)) ;

 Expression(line,index,FALSE,&defined,localhDIR) ;
 if (localhDIR->type != NoPatch)
  Warning(BadOpUse) ;
 free(localhDIR) ;

 string->length = 0 ;
 if (!errorFound)
  {
   if (eStack[1].u.operand.operandType != StringOT)
    Warning(BadExprType) ;
   else
    *string = eStack[1].u.operand.u.string ;
  }
} /* End StringExpr */

/*---------------------------------------------------------------------------*/

CARDINAL RegisterRelExpr(char *line,CARDINAL *index,CARDINAL *reg,OperandType *type,BOOLEAN allowUnd,BOOLEAN  *defined)
{
 CARDINAL i ;
 CARDINAL j ;
 EHeliosPatch *localhDIR = mymalloc(sizeof(EHeliosPatch)) ;

 Expression(line,index,allowUnd,defined,localhDIR) ;
 if (localhDIR->type != NoPatch)
  Warning(BadOpUse) ;
 free(localhDIR) ;

 if (errorFound || !*defined)
  return 0 ;

 *type = eStack[1].u.operand.operandType ;
 switch (*type)
  {
   case RegRelOT   : /* register relative symbol */
                     j = 0 ;
                     for (i = 0; i <= 15; i++)
                      {
                       j += abs(eStack[1].u.operand.u.regRel.registers[i]) ;
                       /* Could be negative */
                       if (eStack[1].u.operand.u.regRel.registers[i] != 0)
                        *reg = i ;
                      }
                     if (j == 1)
                      return eStack[1].u.operand.u.regRel.offset ;
                     break ;

   case PCRelOT    : *reg = 15 ;
                     return eStack[1].u.operand.u.pLabel ;

   case DPRelOT    : *reg = 9 ; /* data module relative */
                     return((CARDINAL)eStack[1].u.operand.u.hSymbol) ;

   case ConstantOT : /* constants not allowed */
                     break ;
  }
 Warning(BadExprType) ;
 return 0 ;
} /* End RegisterRelExpr */

/*---------------------------------------------------------------------------*/
/* In case of a string the returned CARDINAL is a NamePointer */
CARDINAL StringOrConstantExpr(char *line,CARDINAL *index,BOOLEAN allowUnd,OperandType *type)
{
 BOOLEAN defined ;
 EHeliosPatch *localhDIR = mymalloc(sizeof(EHeliosPatch)) ;

 Expression(line,index,allowUnd,&defined,localhDIR) ;
 if (localhDIR->type != NoPatch)
  Warning(BadOpUse) ;
 free(localhDIR) ;

 if (errorFound || !defined)
  {
   *type = ConstantOT ;
   return 0 ;
  } ; /* if */
 *type = eStack[1].u.operand.operandType ;
 switch (*type)
  {
   case ConstantOT :
                     return eStack[1].u.operand.u.constant ;

   case StringOT   :
                     stringName.length = eStack[1].u.operand.u.string.length ;
                     stringName.key = eStack[1].u.operand.u.string.key ;
                     return (CARDINAL)&stringName ;

   default         :
                     Warning(BadExprType) ;
                     return 0 ;

  }
} /* End StringOrConstantExpr */

/*---------------------------------------------------------------------------*/

/* Used to fake a symbol for other segment local label references */
SymbolPointer ExternalExpr(char *line,CARDINAL *index,CARDINAL *offset)
{
 CARDINAL      oldIndex = *index ;
 Name          name ;
 SymbolPointer symbolPointer ;
 BOOLEAN       defined ;
 Name          nname ;                          /* new external name */
 char          nametext[MaxStringSize] ;        /* string buffer */
 
 nname.key = nametext ;                         /* reference the buffer */

 if (!SymbolTest(line,index,&name))
  return(NULL) ;
 else
  {
   symbolPointer = LookupRef(name,TRUE) ;

   if ((symbolPointer == NULL) || ((symbolPointer->u.s.sdt != FixedSDT) && (symbolPointer->u.s.sdt != ExternalSDT)) || ((symbolPointer->u.s.sdt == FixedSDT) && ((symbolPointer->u.s.sds != DefinedSDS) || (symbolPointer->u.s.fst != RelocatableFST) || (symbolPointer->u.s.fst != ModuleFST))))
    {
#if 1
     nametext[0] = 128 ;
#else
     /* Check for a "_" prefixed version of the symbol */
     nametext[0] = '_' ;
#endif
     memcpy(&nametext[1],name.key,name.length) ;
     nametext[1 + name.length] = '\0' ;
     nname.length = strlen(nametext) ;
     symbolPointer = LookupRef(nname,TRUE) ;
    }

   if ((symbolPointer == NULL) || ((symbolPointer->u.s.sdt != FixedSDT) && (symbolPointer->u.s.sdt != ExternalSDT)) || ((symbolPointer->u.s.sdt == FixedSDT) && ((symbolPointer->u.s.sds != DefinedSDS) || (symbolPointer->u.s.fst != RelocatableFST) || (symbolPointer->u.s.fst != ModuleFST))))
    {
     *index = oldIndex ;        /* do not step over the line */
     return NULL ;
    }

   if (symbolPointer->u.s.sdt == ExternalSDT)
    *offset = 0 ;
   else
    *offset = symbolPointer->value.card ;
   AddUse(symbolPointer) ;
  }

 while (line[*index] == Space)
  (*index)++ ;

 if (line[*index] == PlusSign)
  {
   /* Allow positive offset on externals and out of area symbols */
   *offset += ConstantExpr(line,index,pass == 1,&defined,NULL) ;
  }

 return(symbolPointer) ;
} /* End ExternalExpr */

/*---------------------------------------------------------------------------*/
/* Shuffle down the stack one element, so that the element at pointerLo
 * is removed. op is the new previous operator.
 */
void ShuffleDown(CARDINAL pointerLo,CARDINAL pointerHi,Operator *op)
{
 *op = eStack[pointerLo-1].u.operator ;
 while (pointerLo < pointerHi)
  {
   switch (eStack[pointerLo+1].type)
    {
     case OperatorEST : eStack[pointerLo].type = OperatorEST ;
                        eStack[pointerLo].u.operator = eStack[pointerLo+1].u.operator ;
                        break ;

     case OperandEST  : eStack[pointerLo].type = OperandEST ;
                        eStack[pointerLo].u.operand.operandType = eStack[pointerLo+1].u.operand.operandType ;
                        switch (eStack[pointerLo].u.operand.operandType)
                         {
                          case ConstantOT :
                          case UnDefOT    : eStack[pointerLo].u.operand.u.constant = eStack[pointerLo+1].u.operand.u.constant ;
                                            break ;

                          case StringOT   : eStack[pointerLo].u.operand.u.string = eStack[pointerLo+1].u.operand.u.string ;
                                            break ;

                          case PCRelOT    : eStack[pointerLo].u.operand.u.pLabel = eStack[pointerLo+1].u.operand.u.pLabel ;
                                            break ;

                          case RegRelOT   : eStack[pointerLo].u.operand.u.regRel = eStack[pointerLo+1].u.operand.u.regRel ;
                                            break ;

                          case LogicalOT  : eStack[pointerLo].u.operand.u.bool = eStack[pointerLo+1].u.operand.u.bool ;
                         }
    }
   pointerLo++ ;
  }
} /* End ShuffleDown */

/*---------------------------------------------------------------------------*/
/* Returns true if the arguments are ok */
BOOLEAN ArgSyntaxCheck(Operand arg1,Operand arg2,Operator op)
{
 switch (op)
  {
   default    :
   case STBot :
   case STTop :
   case Bra   :
   case Ket   :
                return FALSE ;

   case LAnd  :
   case LOr   :
   case LEor  :
                return (arg1.operandType == LogicalOT) && (arg2.operandType == LogicalOT) ;

   case Less          :
   case OpEquals      :
   case LessEquals    :
   case Greater       :
   case NotEquals     :
   case GreaterEquals :
                        return (arg1.operandType != LogicalOT) && (arg2.operandType != LogicalOT) && (((arg1.operandType == StringOT) && (arg2.operandType == StringOT)) || (arg1.operandType == arg2.operandType) || (arg1.operandType == UnDefOT) || (arg2.operandType == UnDefOT) || ((((arg2.operandType == StringOT) && (arg2.u.string.length == 1)) || (arg2.operandType == ConstantOT)) && ((arg1.operandType == StringOT) && (arg1.u.string.length == 1)) || (arg1.operandType == ConstantOT))) ;


   case Plus         :
   case Minus        :
                       return ((arg1.operandType != LogicalOT) && ((arg1.operandType != StringOT) || (arg1.u.string.length == 1))) && ((arg2.operandType != LogicalOT) && ((arg2.operandType != StringOT) || (arg2.u.string.length == 1))) ;

   case Ror          :
   case Rol          :
   case Shr          :
   case Shl          :
   case BAnd         :
   case BOr          :
   case BEor         :
   case Star         :
   case Slash        :
   case Mod          :
                       return ((arg1.operandType == ConstantOT) || ((arg1.operandType == StringOT) && (arg1.u.string.length == 1)) || (arg1.operandType == UnDefOT)) && ((arg2.operandType == ConstantOT) || ((arg2.operandType == StringOT) && (arg2.u.string.length == 1)) || (arg2.operandType == UnDefOT)) ;

   case Left         :
   case Right        :
                       return (arg1.operandType == StringOT) && (arg2.operandType == ConstantOT) ;

   case Cc           :
                       return (arg1.operandType == StringOT) && (arg2.operandType == StringOT) ;


   case UPlus        :
   case UMinus       :
   case Index        :
                       return (arg1.operandType != LogicalOT) && ((arg1.operandType != StringOT) || (arg1.u.string.length == 1)) ;

   case LNot         :
                       return arg1.operandType == LogicalOT ;

   case BNot         :
                       return (arg1.operandType == ConstantOT) || (arg1.operandType == UnDefOT) || ((arg1.operandType == StringOT) && (arg1.u.string.length == 1)) ;

   case Len          : return arg1.operandType == StringOT ;

   case Chr          : return arg1.operandType == ConstantOT ;

   case Str          : return (arg1.operandType == ConstantOT) || (arg1.operandType == LogicalOT) ;

   case Base         : return (arg1.operandType == RegRelOT) || (arg1.operandType == PCRelOT) ;

   case ModOff       :
   case Offset       :
   case LsbOff       :
   case MidOff       :
   case MsbOff       :
                       return ((arg1.operandType == DPRelOT) || (arg1.operandType == UnDefOT)) ;
  } ; /* case */
} /* End ArgSyntaxCheck */

/*---------------------------------------------------------------------------*/

void Init_Expression(void)
{
 /* Initialise the array of operator precedences */
 priorities[STBot] = 0 ;
 priorities[STTop] = 0 ;
 priorities[Bra] = 8 ;
 priorities[Ket] = 0 ;
 priorities[LAnd] = 1 ;
 priorities[LOr] = 1 ;
 priorities[LEor] = 1 ;
 priorities[Less] = 2 ;
 priorities[OpEquals] = 2 ;
 priorities[LessEquals] = 2 ;
 priorities[Greater] = 2 ;
 priorities[NotEquals] = 2 ;
 priorities[GreaterEquals] = 2 ;
 priorities[Plus] = 3 ;
 priorities[Minus] = 3 ;
 priorities[Ror] = 4 ;
 priorities[Rol] = 4 ;
 priorities[Shr] = 4 ;
 priorities[Shl] = 4 ;
 priorities[BAnd] = 3 ;
 priorities[BOr] = 3 ;
 priorities[BEor] = 3 ;
 priorities[Left] = 5 ;
 priorities[Right] = 5 ;
 priorities[Cc] = 5 ;
 priorities[Star] = 6 ;
 priorities[Slash] = 6 ;
 priorities[Mod] = 6 ;
 priorities[UPlus] = 7 ;
 priorities[UMinus] = 7 ;
 priorities[LNot] = 7 ;
 priorities[BNot] = 7 ;
 priorities[Len] = 7 ;
 priorities[Chr] = 7 ;
 priorities[Str] = 7 ;
 priorities[Base] = 7 ;
 priorities[Index] = 7 ;
 priorities[ModOff] = 7 ;
 priorities[Offset] = 7 ;
 priorities[LsbOff] = 7 ;
 priorities[MidOff] = 7 ;
 priorities[MsbOff] = 7 ;
}

/*---------------------------------------------------------------------------*/
/* EOF expr/c */
