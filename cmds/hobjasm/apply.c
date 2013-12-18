/* -> apply/c
 * Title:               Operator evaluation
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */

/*---------------------------------------------------------------------------*/

#include "apply.h"
#include "applysub.h"
#include "errors.h"
#include "expr.h"
#include "exstore.h"
#include "extypes.h"

/*---------------------------------------------------------------------------*/
/* "op" is the operator to be applied.
 * "pointer" points to the top of the stack.
 */
void Apply(Operator *op,CARDINAL *pointer)
{
 CARDINAL i ;
 CARDINAL j ;
 CARDINAL op1 ;
 CARDINAL op2 ;
 CARDINAL pointer2 ; /* A second pointer into the stack */
 BOOLEAN  ok ;

 pointer2 = (*op >= UPlus) ? *pointer-1 : *pointer-2 ;

 if (((*op >= UPlus) && ArgSyntaxCheck(eStack[*pointer].u.operand,eStack[*pointer].u.operand,*op)) || ((*op < UPlus) && ArgSyntaxCheck(eStack[pointer2].u.operand,eStack[*pointer].u.operand,*op)))
  {
   /* Here we actually execute the operator */
   if ((eStack[pointer2].u.operand.operandType == UnDefOT) || ((eStack[*pointer].u.operand.operandType == UnDefOT) && (*op < UPlus)))
    {
     Operand *operand = &eStack[pointer2].u.operand ;
     if ((*op >= Less) && (*op <= GreaterEquals))
      {
       operand->u.bool = TRUE ;
       operand->operandType = LogicalOT ;
      }
     else
      operand->operandType = UnDefOT ;
    }
   else
    {
     switch (*op)
      {
       case LAnd          :
       eStack[pointer2].u.operand.u.bool &= eStack[*pointer].u.operand.u.bool ;
       break ;

       case LOr           :
       eStack[pointer2].u.operand.u.bool |= eStack[*pointer].u.operand.u.bool ;
       break ;

       case LEor          :
       eStack[pointer2].u.operand.u.bool ^= eStack[*pointer].u.operand.u.bool ;
       break ;

       case Less          :
       case OpEquals      :
       case LessEquals    :
       case Greater       :
       case NotEquals     :
       case GreaterEquals :
       eStack[pointer2].u.operand.u.bool = Compare(*pointer,pointer2,*op) ;
       eStack[pointer2].u.operand.operandType = LogicalOT ;
       break ;

       case Plus          :
       if (eStack[*pointer].u.operand.operandType == StringOT)
        {
         eStack[*pointer].u.operand.operandType = ConstantOT ;
         eStack[*pointer].u.operand.u.constant = eStack[*pointer].u.operand.u.string.key[0] ;
        } ;
       if (eStack[pointer2].u.operand.operandType == StringOT)
        {
         eStack[pointer2].u.operand.operandType = ConstantOT ;
         eStack[pointer2].u.operand.u.constant = eStack[pointer2].u.operand.u.string.key[0] ;
        } ;
       {
        Operand *operand = &eStack[pointer2].u.operand ;
        switch (eStack[*pointer].u.operand.operandType)
         {
          case ConstantOT :
          switch (operand->operandType)
           {
            case ConstantOT :
            operand->u.constant += eStack[*pointer].u.operand.u.constant ;
            break ;

            case PCRelOT    :
            operand->u.pLabel += eStack[*pointer].u.operand.u.constant ;
            break ;

            case RegRelOT   :
            operand->u.regRel.offset += eStack[*pointer].u.operand.u.constant ;
            break ;
           } ; /* case */
          break ;

          case PCRelOT   :
          switch (operand->operandType)
           {
            case ConstantOT :
            i = operand->u.constant ;
            operand->operandType = PCRelOT ;
            operand->u.pLabel = i + eStack[*pointer].u.operand.u.pLabel ;
            break ;

            case PCRelOT    :
            i = operand->u.pLabel ;
            operand->operandType = RegRelOT ;
            operand->u.regRel.offset = i + eStack[*pointer].u.operand.u.pLabel ;
            for (i = 0; i <= 14; i++)
             operand->u.regRel.registers[i] = 0 ;
            operand->u.regRel.registers[15] = 2 ;
            break ;

            case RegRelOT   :
            operand->u.regRel.offset += eStack[*pointer].u.operand.u.pLabel ;
            operand->u.regRel.registers[15]++ ;
            ok = operand->u.regRel.registers[15] == 0 ;
            /* Testing for case collapse */
            for (i = 0; i <= 14; i++)
             ok &= operand->u.regRel.registers[i] == 0 ;
            if (ok)
             {
              i = operand->u.regRel.offset ;
              operand->operandType = ConstantOT ;
              operand->u.constant = i ;
             } ; /* if */
           } ; /* case */
          break ;

          case RegRelOT  :
          switch (operand->operandType)
           {
            case ConstantOT :
            i = operand->u.constant + eStack[*pointer].u.operand.u.regRel.offset ;
            operand->operandType = RegRelOT ;
            operand->u.regRel.offset = i ;
            for (i = 0; i <= 15; i++)
             operand->u.regRel.registers[i] = eStack[*pointer].u.operand.u.regRel.registers[i] ;
            break ;

            case PCRelOT    :
            i = operand->u.pLabel + eStack[*pointer].u.operand.u.regRel.offset ;
            operand->operandType = RegRelOT ;
            operand->u.regRel.offset = i ;
            for (i = 0; i <= 15; i++)
             operand->u.regRel.registers[i] = eStack[*pointer].u.operand.u.regRel.registers[i] ;
            operand->u.regRel.registers[15] ++ ; /* One more on the PC */
            ok = operand->u.regRel.registers[15] == 0 ;
            /* Testing for case collapse */
            for (i = 0; i <= 14; i++)
             ok &= (operand->u.regRel.registers[i] == 0) ;
            if (ok)
             {
              i = operand->u.regRel.offset ;
              operand->operandType = ConstantOT ;
              operand->u.constant = i ;
             } ; /* if */
            break ;

            case RegRelOT   :
            operand->u.regRel.offset += eStack[*pointer].u.operand.u.regRel.offset ;
            for (i = 0; i <= 15; i++)
             operand->u.regRel.registers[i] += eStack[*pointer].u.operand.u.regRel.registers[i] ;
            ok = (operand->u.regRel.registers[15] == 0) || (operand->u.regRel.registers[15] == 1) ;
            /* Testing for case collapse */
            for (i = 0; i <= 14; i++)
             ok &= (operand->u.regRel.registers[i] == 0) ;
            if (ok)
             {
              i = operand->u.regRel.offset ;
              if (operand->u.regRel.registers[15] == 0)
               {
                operand->operandType = ConstantOT ;
                operand->u.constant = i ;
               }
              else
               {
                operand->operandType = PCRelOT ;
                operand->u.pLabel = i ;
               } ;
             } ; /* if */
           } ; /* case */
          break ;
         } ; /* case */
       } ;
       break;

       case Minus         :
       if (eStack[*pointer].u.operand.operandType == StringOT)
        {
         i = eStack[*pointer].u.operand.u.string.key[0] ;
         eStack[*pointer].u.operand.operandType = ConstantOT ;
         eStack[*pointer].u.operand.u.constant = i ;
        } ;
       if (eStack[pointer2].u.operand.operandType == StringOT)
        {
         i = eStack[pointer2].u.operand.u.string.key[0] ;
         eStack[pointer2].u.operand.operandType = ConstantOT ;
         eStack[pointer2].u.operand.u.constant = i ;
        } ;
       {
        Operand *operand = &eStack[pointer2].u.operand ;
        switch (eStack[*pointer].u.operand.operandType)
         {
          case ConstantOT :
          switch (operand->operandType)
           {
            case ConstantOT :
            operand->u.constant -= eStack[*pointer].u.operand.u.constant ;
            break ;

            case PCRelOT    :
            operand->u.pLabel -= eStack[*pointer].u.operand.u.constant ;
            break ;

            case RegRelOT   :
            operand->u.regRel.offset -= eStack[*pointer].u.operand.u.constant ;
            break ;
           } ; /* case */
          break ;

          case PCRelOT    :
          switch (operand->operandType)
           {
            case ConstantOT :
            i = operand->u.constant ;
            operand->operandType = RegRelOT ;
            operand->u.regRel.offset = i - eStack[*pointer].u.operand.u.pLabel ;
            for (i = 0; i <= 14; i++)
             operand->u.regRel.registers[i] = 0 ;
            operand->u.regRel.registers[15] = -1 ;
            break ;

            case PCRelOT    :
            i = operand->u.pLabel ;
            operand->operandType = ConstantOT ;
            operand->u.constant = i - eStack[*pointer].u.operand.u.pLabel ;
            break ;

            case RegRelOT   :
            operand->u.regRel.offset -= eStack[*pointer].u.operand.u.pLabel ;
            operand->u.regRel.registers[15]-- ;
            ok = operand->u.regRel.registers[15] == 1 ;
            /* Testing for case collapse */
            for (i = 0; i <= 14; i++)
             ok &= (operand->u.regRel.registers[i] == 0) ;
            if (ok)
             {
              i = operand->u.regRel.offset ;
              operand->operandType = PCRelOT ;
              operand->u.pLabel = i ;
             } ; /* if */
            break ;
           } ; /* case */
          break ;

          case RegRelOT   :
          switch (operand->operandType)
           {
            case ConstantOT :
            i = operand->u.constant - eStack[*pointer].u.operand.u.regRel.offset ;
            operand->operandType = RegRelOT ;
            operand->u.regRel.offset = i ;
            for (i = 0; i <= 15; i++)
             operand->u.regRel.registers[i] = - eStack[*pointer].u.operand.u.regRel.registers[i] ;
            ok = operand->u.regRel.registers[15] == 1 ;
            /* Testing for case collapse */
            for (i = 0; i <= 14; i++)
             ok &= (operand->u.regRel.registers[i] == 0) ;
            if (ok)
             {
              i = operand->u.regRel.offset ;
              operand->operandType = PCRelOT ;
              operand->u.pLabel = i ;
             } ; /* if */
            break ;

            case PCRelOT   :
            i = operand->u.pLabel - eStack[*pointer].u.operand.u.regRel.offset ;
            operand->operandType = RegRelOT ;
            operand->u.regRel.offset = i ;
            for (i = 0; i <= 15; i++)
             operand->u.regRel.registers[i] = eStack[*pointer].u.operand.u.regRel.registers[i] ;
            operand->u.regRel.registers[15]++ ; /* One more on the PC */
            break ;

            case RegRelOT   :
            operand->u.regRel.offset -= eStack[*pointer].u.operand.u.regRel.offset ;
            for (i = 0; i <= 15; i++)
             operand->u.regRel.registers[i] -= eStack[*pointer].u.operand.u.regRel.registers[i] ;
            ok = (operand->u.regRel.registers[15] == 0) || (operand->u.regRel.registers[15] == 1) ;
            /* Testing for case collapse */
            for (i = 0; i <= 14; i++)
             ok &= (operand->u.regRel.registers[i] == 0) ;
            if (ok)
             {
              i = operand->u.regRel.offset ;
              if (operand->u.regRel.registers[15] == 0)
               {
                operand->operandType = ConstantOT ;
                operand->u.constant = i ;
               }
              else
               {
                operand->operandType = PCRelOT ;
                operand->u.pLabel = i ;
               } ;
             } ; /* if */
            break ;
           } ; /* case */
         } ; /* case */
       } ; /* with */
       break;

       case Ror :
       op2 = Coerce(*pointer) & 31 ; /* Rotate by more than 32 cycles */
       op1 = Coerce(pointer2) ; /* Coerce string operands */
       if (op2)
        op1 = (op1 >> op2) | (op1 << (32 - op2)) ;
       eStack[pointer2].u.operand.operandType = ConstantOT ;
       eStack[pointer2].u.operand.u.constant = op1 ;
       break ;

       case Rol :
       op2 = Coerce(*pointer) & 31 ; /* Rotate by more than 32 cycles */
       op1 = Coerce(pointer2) ; /* Coerce string operands */
       if (op2)
        op1 = (op1 << op2) | (op1 >> (32 - op2)) ;
       eStack[pointer2].u.operand.operandType = ConstantOT ;
       eStack[pointer2].u.operand.u.constant = op1 ;
       break ;

       case Shr :
       op2 = Coerce(*pointer) ;
       eStack[pointer2].u.operand.u.constant = (op2 >= 0x20) ? 0 : Coerce(pointer2) >> op2 ;
       eStack[pointer2].u.operand.operandType = ConstantOT ;
       break ;

       case Shl :
       op2 = Coerce(*pointer) ;
       eStack[pointer2].u.operand.u.constant = (op2 >= 0x20) ? 0 : Coerce(pointer2) << op2 ;
       eStack[pointer2].u.operand.operandType = ConstantOT ;
       break ;

       case BAnd :
       op1 = Coerce(pointer2) ;
       op2 = Coerce(*pointer) ;
       eStack[pointer2].u.operand.u.constant = op1 & op2 ;
       eStack[pointer2].u.operand.operandType = ConstantOT ;
       break ;

       case BOr :
       op1 = Coerce(pointer2) ;
       op2 = Coerce(*pointer) ;
       eStack[pointer2].u.operand.u.constant = op1 | op2 ;
       eStack[pointer2].u.operand.operandType = ConstantOT ;
       break ;

       case BEor :
       op1 = Coerce(pointer2) ;
       op2 = Coerce(*pointer) ;
       eStack[pointer2].u.operand.u.constant = op1 ^ op2 ;
       eStack[pointer2].u.operand.operandType = ConstantOT ;
       break ;

       case Left :
       op1 = Coerce(*pointer) ;
       if (eStack[pointer2].u.operand.u.string.length < op1)
        {
         Warning(StringShort) ;
         return ;
        } ; /* if */
       eStack[pointer2].u.operand.u.string.length = op1 ;
       break ;

       case Right :
       op1 = Coerce(*pointer) ;
       j = eStack[pointer2].u.operand.u.string.length ;
       if (j < op1)
        {
         Warning(StringShort) ;
         return ;
        } ; /* if */
       for (i = 1; i <= op1; i++)
        eStack[pointer2].u.operand.u.string.key[i-1] = eStack[pointer2].u.operand.u.string.key[i + j - op1 - 1] ;
       eStack[pointer2].u.operand.u.string.length = op1 ;
       break ;

       case Cc :
       CcSub(*pointer,pointer2) ;
       break ;

       case Star :
       op1 = Coerce(pointer2) ;
       op2 = Coerce(*pointer) ;
       eStack[pointer2].u.operand.operandType = ConstantOT ;
       eStack[pointer2].u.operand.u.constant = op1 * op2 ;
       break ;

       case Slash :
       op1 = Coerce(pointer2) ;
       op2 = Coerce(*pointer) ;
       if (op2 == 0)
        {
         Warning(DivZero) ;
         return ;
        } ; /* if */
       eStack[pointer2].u.operand.operandType = ConstantOT ;
       eStack[pointer2].u.operand.u.constant = op1 / op2 ;
       break ;

       case Mod :
       op1 = Coerce(pointer2) ;
       op2 = Coerce(*pointer) ;
       if (op2 == 0)
        {
         Warning(DivZero) ;
         return ;
        } ; /* if */
      eStack[pointer2].u.operand.operandType = ConstantOT ;
      eStack[pointer2].u.operand.u.constant = op1 % op2 ;
      break ;

      case UPlus :
      ShuffleDown(pointer2,*pointer,op) ;
      break ;

      case UMinus :
      {
       Operand *operand = &eStack[*pointer].u.operand ;
       switch (operand->operandType)
        {
         case UnDefOT    :
         case ConstantOT :
         eStack[pointer2].type = OperandEST ;
         eStack[pointer2].u.operand.operandType = operand->operandType ;
         eStack[pointer2].u.operand.u.constant = - operand->u.constant ;
         break ;

         case StringOT   :
         eStack[pointer2].type = OperandEST ;
         eStack[pointer2].u.operand.operandType = ConstantOT ;
         eStack[pointer2].u.operand.u.constant = - operand->u.string.key[0] ;
         break ;

         case PCRelOT    :
         eStack[pointer2].type = OperandEST ;
         eStack[pointer2].u.operand.operandType = RegRelOT ;
         eStack[pointer2].u.operand.u.regRel.offset = - operand->u.pLabel ;
         for (i = 0; i <= 14; i++)
          eStack[pointer2].u.operand.u.regRel.registers[i] = 0 ;
         eStack[pointer2].u.operand.u.regRel.registers[15] = -1 ;
         break ;

         case RegRelOT   :
         eStack[pointer2].type = OperandEST ;
         eStack[pointer2].u.operand.operandType = RegRelOT ;
         for (i = 0; i <= 15; i++)
          eStack[pointer2].u.operand.u.regRel.registers[i] = - operand->u.regRel.registers[i] ;
         eStack[pointer2].u.operand.u.regRel.offset = 0 - operand->u.regRel.offset ;
         /* Here we must check that the result hasn't become simple PCRelOT */
         ok = eStack[pointer2].u.operand.u.regRel.registers[15] == 1 ;
         for (i = 0; i <= 14; i++)
          ok &= (eStack[pointer2].u.operand.u.regRel.registers[i] == 0) ;
         if (ok)
          {
           i = eStack[pointer2].u.operand.u.regRel.offset ;
           eStack[pointer2].u.operand.operandType = PCRelOT ;
           eStack[pointer2].u.operand.u.pLabel = i ;
          } ; /* if */
        } ;
      } ;
      break ;

      case LNot :
      eStack[pointer2].type = OperandEST ;
      eStack[pointer2].u.operand.operandType = LogicalOT ;
      eStack[pointer2].u.operand.u.bool = !eStack[*pointer].u.operand.u.bool ;
      break ;

      case BNot :
      eStack[pointer2].type = OperandEST ;
      eStack[pointer2].u.operand.operandType = ConstantOT ;
      eStack[pointer2].u.operand.u.bool = ~eStack[*pointer].u.operand.u.bool ;
      break ;

      case Len :
      LenSub(*pointer,pointer2) ;
      break ;

      case Chr :
      ChrSub(*pointer,pointer2) ;
      break ;

      case Str :
      StrSub(*pointer,pointer2) ;
      break ;

      case Base :
      eStack[pointer2].type = OperandEST ;
      eStack[pointer2].u.operand.operandType = ConstantOT ;
      switch (eStack[*pointer].u.operand.operandType)
       {
        case RegRelOT :
        j = 16 ;
        for (i = 0; i <= 15; i++)
         {
          if ((j == 16) && (eStack[*pointer].u.operand.u.regRel.registers[i] == 1))
           j = i ;
          else
           if (eStack[*pointer].u.operand.u.regRel.registers[i])
            Warning(BadOpType) ;
         } ; /* for */
        if (j == 16)
         Warning(BadOpType); /* if */
        /* Above may be unnecessary, belt and braces approach */
        eStack[pointer2].u.operand.u.constant = j ;
        break;

        case PCRelOT :
        eStack[pointer2].u.operand.u.constant = 15 ;
       } ; /* case */
      break ;

      case Index :
      eStack[pointer2].type = OperandEST ;
      {
       Operand *operand = &eStack[pointer2].u.operand ;
       operand->operandType = ConstantOT ;
       switch (eStack[*pointer].u.operand.operandType)
        {
         case UnDefOT    :
         case ConstantOT :
         operand->u.constant = eStack[*pointer].u.operand.u.constant ;
         break ;

         case StringOT   :
         operand->u.constant = eStack[pointer2].u.operand.u.string.key[0] ;
         break ;

         case PCRelOT    :
         operand->u.constant = eStack[*pointer].u.operand.u.pLabel ;
         break ;

         case RegRelOT   :
         operand->u.constant = eStack[*pointer].u.operand.u.regRel.offset ;
         break ;
        } ; /* case */
      } ; /* with */
      break ;

      /* These "operators" are used to generate Helios object patches.
       * They should always return a constant value of 0x00000000.
       * NOTE: Only one of these operators is valid per expression.
       */
      case ModOff : /* module table offset */
      eStack[pointer2].type = OperandEST ;
      eStack[pointer2].u.operand.operandType = ConstantOT ;
      eStack[pointer2].u.operand.u.constant = 0x00000000 ;
      break ;

      case Offset : /* offset with module table */
      case LsbOff : /* lo8bits of offset in module table */
      case MidOff : /* middle8bits of offset in module table */
      case MsbOff : /* hi8bits of offset in module table */
      eStack[pointer2].type = OperandEST ;
      eStack[pointer2].u.operand.operandType = ConstantOT ;
      eStack[pointer2].u.operand.u.constant = 0x00000000 ;
      break ;
     } ; /* case */
   } ; /* if */
  *pointer = pointer2 ;
  if (eStack[*pointer-1].type != OperatorEST)
   AssemblerError("Expression stack error") ;
  *op = eStack[*pointer-1].u.operator ;
 }
 else
  Warning(BadOpType) ;
} /* End Apply */

/*---------------------------------------------------------------------------*/
/* End apply/c */
