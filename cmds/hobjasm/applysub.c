/* -> applysub/c
 * Title:               Expression evaluation subroutines
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "applysub.h"
#include "errors.h"
#include "exstore.h"
#include "extypes.h"
#include "nametype.h"
#include <string.h>

/*---------------------------------------------------------------------------*/
/* Coerce a one byte string on the stack to a one byte constant if necessary */

CARDINAL Coerce(CARDINAL pointer)
{
  return (eStack[pointer].u.operand.operandType == StringOT) ? eStack[pointer].u.operand.u.string.key[0] : eStack[pointer].u.operand.u.constant;
} /* End Coerce */

/*---------------------------------------------------------------------------*/
/* Returns name1 <= name2 */

BOOLEAN CompareString(Name name1,Name name2,BOOLEAN *equal)
{
 int j ;
 *equal = name1.length == name2.length ;
 if (name1.length == 0)
  return TRUE ;
 if (name2.length == 0)
  return FALSE ;
 j = memcmp(name1.key,name2.key,(name1.length > name2.length) ? name2.length : name1.length) ;
 if (j != 0)
  {
   *equal = FALSE ;
   return j < 0 ;
  }
 return(name1.length <= name2.length) ;
} /* End CompareString */

/*---------------------------------------------------------------------------*/

BOOLEAN Compare(CARDINAL pointer,CARDINAL pointer2,Operator op)
{
 BOOLEAN  equal ;
 CARDINAL i ;
 CARDINAL op1 ;
 CARDINAL op2 ;

 if ((eStack[pointer].u.operand.operandType == StringOT) && (eStack[pointer2].u.operand.operandType == StringOT))
  {
   switch (op)
    {
     case Less          : return(CompareString(eStack[pointer2].u.operand.u.string,eStack[pointer].u.operand.u.string,&equal) && !equal) ;

     case OpEquals      : return(CompareString(eStack[pointer2].u.operand.u.string,eStack[pointer].u.operand.u.string,&equal) && equal) ;

     case LessEquals    : return(CompareString(eStack[pointer2].u.operand.u.string,eStack[pointer].u.operand.u.string,&equal)) ;

     case Greater       : return(CompareString(eStack[pointer].u.operand.u.string,eStack[pointer2].u.operand.u.string,&equal) && !equal) ;

     case NotEquals     : (void)CompareString(eStack[pointer2].u.operand.u.string,eStack[pointer].u.operand.u.string,&equal) ;
                          return(!equal) ;

     case GreaterEquals : return(CompareString(eStack[pointer].u.operand.u.string,eStack[pointer2].u.operand.u.string,&equal)) ;
    } /* switch */
  }
 if ((eStack[pointer].u.operand.operandType == ConstantOT) || (eStack[pointer2].u.operand.operandType == ConstantOT))
  {
   op1 = Coerce(pointer2) ;
   op2 = Coerce(pointer) ;
  }
 else
  if (eStack[pointer].u.operand.operandType == PCRelOT)
   {
    op1 = eStack[pointer2].u.operand.u.pLabel ;
    op2 = eStack[pointer].u.operand.u.pLabel ;
   }
  else
   {
    /* Must be RegRel */
    op1 = eStack[pointer2].u.operand.u.regRel.offset ;
    op2 = eStack[pointer].u.operand.u.regRel.offset ;
    for (i = 0; i <= 15; i++)
     {
      if (eStack[pointer].u.operand.u.regRel.registers[i] != eStack[pointer2].u.operand.u.regRel.registers[i])
       return FALSE ;
     }
   }

 switch (op)
  {
   case Less          : return(op1 < op2) ;
   case OpEquals      : return(op1 == op2) ;
   case LessEquals    : return(op1 <= op2) ;
   case Greater       : return(op1 > op2) ;
   case NotEquals     : return(op1 != op2) ;
   case GreaterEquals : return(op1 >= op2) ;
   default            : return TRUE ;
  } /* switch */
} /* End Compare */

/*---------------------------------------------------------------------------*/

void CcSub(CARDINAL pointer,CARDINAL pointer2)
{
 char     *tempString ;
 CARDINAL  i ;
 CARDINAL  op1 = eStack[pointer].u.operand.u.string.length ;
 CARDINAL  op2 = eStack[pointer2].u.operand.u.string.length ;

 if (op2 + op1 > MaxStringSize)
  {
   Warning(StringOver) ;
   return ;
  }
 ALLOCATE(&tempString,op2 + op1) ;
 if (op2 > 0)
  {
   for (i = 0; i <= op2 - 1; i++)
    tempString[i] = eStack[pointer2].u.operand.u.string.key[i] ;
  }
 if (op1 > 0)
  {
   for (i = 0; i <= op1 - 1; i++)
    tempString[op2 + i] = eStack[pointer].u.operand.u.string.key[i] ;
  }
 eStack[pointer2].u.operand.u.string.length = op1 + op2 ;
 eStack[pointer2].u.operand.u.string.key = tempString ;
} /* End CcSub */

/*---------------------------------------------------------------------------*/

void LenSub(CARDINAL pointer,CARDINAL pointer2)
{
 CARDINAL op1 = eStack[pointer].u.operand.u.string.length ;

 eStack[pointer2].type = OperandEST ;
 eStack[pointer2].u.operand.operandType = ConstantOT ;
 eStack[pointer2].u.operand.u.constant = op1 ;
} /* End LenSub */

/*---------------------------------------------------------------------------*/

void ChrSub(CARDINAL pointer,CARDINAL pointer2)
{
 CARDINAL op1 ;

 op1 = eStack[pointer].u.operand.u.constant % 0x100 ;
 eStack[pointer2].type = OperandEST ;
 eStack[pointer2].u.operand.operandType = StringOT ;
 ALLOCATE(&eStack[pointer2].u.operand.u.string.key,1) ;
 *eStack[pointer2].u.operand.u.string.key = op1 ;
 eStack[pointer2].u.operand.u.string.length = 1 ;
} /* End ChrSub */

/*---------------------------------------------------------------------------*/

void StrSub(CARDINAL pointer,CARDINAL pointer2)
{
 CARDINAL op1 ;
 CARDINAL op2 ;
 int      i ;
 BOOLEAN  boolValue ;

 eStack[pointer2].type = OperandEST ;
 switch (eStack[pointer].u.operand.operandType)
  {
   case ConstantOT : op1 = eStack[pointer].u.operand.u.constant ;
                     eStack[pointer2].u.operand.operandType = StringOT ;
                     ALLOCATE(&eStack[pointer2].u.operand.u.string.key,8) ;
                     eStack[pointer2].u.operand.u.string.length = 8 ;
                     for (i = 7; i >= 0; i--)
                      {
                       op2 = op1 % 16 ;
                       eStack[pointer2].u.operand.u.string.key[i] = (op2 < 10) ? op2 + '0' : op2 - 10 + 'A' ;
                       op1 = op1 >> 4 ;
                      }
                     break ;

   case LogicalOT  : boolValue = eStack[pointer].u.operand.u.bool ;
                     eStack[pointer2].u.operand.operandType = StringOT ;
                     ALLOCATE(&eStack[pointer2].u.operand.u.string.key,1) ;
                     eStack[pointer2].u.operand.u.string.length = 1 ;
                     *eStack[pointer2].u.operand.u.string.key = (boolValue) ? 'T' : 'F' ;
  }
} /* End StrSub */

/*---------------------------------------------------------------------------*/
/* EOF applysub/c */
