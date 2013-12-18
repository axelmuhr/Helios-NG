#include "iasm.h"

int32 evalflags;

int32 evaluate_integer(Expression *expr)
{  int32 pcr1;
   int32 v1,v2;

   if( expr->exprtype & E_BINARYOP )
   {
      v1 = evaluate_integer(expr->e1.expr);
      pcr1 = evalflags & EF_PC_RELATIVE;
      v2 = evaluate_integer(expr->e2.expr);
   }

   switch(expr->exprtype & E_OPMASK)
   {
   case E_AT:
      {  Symbol *sym = checkdatasymb(expr);
         if( !sym )
         {
            error("Attempt to take @ of non-data symbol");
            return 0;
         }
         else
         {
/* Not-evaluable at the moment but may be coerced to a directive later */
            evalflags |= EF_BADSYMBOL;
            return 0;
         }
      }

   case E_MODNUM:
      if( module_number != -1 ) return module_number;
      evalflags |= EF_DIRECTIVE;
      return 0;

   case E_IMAGESIZE:
      evalflags |= EF_DIRECTIVE;
      return 0;

   case E_SYMBOL:
      {  Symbol *s = expr->e1.symbol;
         switch( s->symtype )
         {

         case S_DATA:
            if( !(s->symflags & sf_eval1) )
            {  evalflags |= EF_DIRECTIVE;
               return 0;
            }
            else break;	/* Pick up symval slot */

         case S_LABEL:
            if( s->symflags & sf_eval1 )
            {  evalflags |= EF_PC_RELATIVE; break; }
            else
            {  evalflags |= EF_DIRECTIVE;
               return 0;
            }
         case S_SET: case S_EQU:
            break;
         default:
            evalflags |= EF_BADSYMBOL;
            break;
         }
         return s->symv.symval;
      }

   case E_DOT:
      {  evalflags |= EF_PC_RELATIVE;
         return dot;
      }

   case E_PLUSOP:
         if( pcr1 && (evalflags & EF_PC_RELATIVE) )
         {  error("Can't add 2 PC-relative symbols");
            evalflags &= ~EF_PC_RELATIVE;
            return 0;
         }
         return v1+v2;

   case E_MINUSOP:
         evalflags ^= pcr1;
         return v1-v2;

   case E_TIMESOP:
         if( (evalflags & EF_PC_RELATIVE) || pcr1 )
         {  error("Can't multiply PC-relative symbol");
            return 0;
         }
         return v1 * v2;

   case E_DIVIDEOP:
         if( (evalflags & EF_PC_RELATIVE) || pcr1 )
         {  error("Can't divide PC-relative symbol");
            return 0;
         }
         return v1 / v2;

   case E_MODOP:
         if( (evalflags & EF_PC_RELATIVE) || pcr1 )
         {  error("Can't take modulus of PC-relative symbol");
            return 0;
         }
         return v1 % v2;

   case E_LSHIFTOP:
         if( (evalflags & EF_PC_RELATIVE) || pcr1 )
         {  error("Can't shift PC-relative symbol");
            return 0;
         }
         return v1 << v2;

   case E_RSHIFTOP:
         if( (evalflags & EF_PC_RELATIVE) || pcr1 )
         {  error("Can't shift PC-relative symbol");
            return 0;
         }
         return v1 >> v2;

   case E_ANDOP:
         if( (evalflags & EF_PC_RELATIVE) || pcr1 )
         {  error("Can't AND PC-relative symbol");
            return 0;
         }
         return v1 & v2;

   case E_XOROP:
         if( (evalflags & EF_PC_RELATIVE) || pcr1 )
         {  error("Can't XOR PC-relative symbol");
            return 0;
         }
         return v1 ^ v2;

   case E_OROP:
         if( (evalflags & EF_PC_RELATIVE) || pcr1 )
         {  error("Can't OR PC-relative symbol");
            return 0;
         }
         return v1 | v2;

   case E_GT: case E_LT: case E_GE:
   case E_LE: case E_EQ: case E_NE:
         {  ETYPE oldtype = expr->exprtype;
            int32 val;
            expr->exprtype = E_MINUS;
            val = evaluate_integer(expr);
            if( evalflags )
            {  error("Can't evaluate expression - assuming 0");
               val = 0;
            }
            else
               switch( expr->exprtype = oldtype )
               {
               case E_GT: val = val>0; break;
               case E_LT: val = val<0; break;
               case E_GE: val = val>=0; break;
               case E_LE: val = val<=0; break;
               case E_EQ: val = val==0; break;
               case E_NE: val = val!=0; break;
               }
            return val;
         }

   case E_NUMBER:
         evalflags &= ~EF_PC_RELATIVE;
         return expr->e1.value;

   case E_UMINUS:
         v1 = -evaluate_integer(expr->e1.expr);
         if( evalflags & EF_PC_RELATIVE )
         {
            error("Can't negate PC-relative symbol");
            return 0;
         }
         return v1;

   case E_NOT:
         v1 = ~evaluate_integer(expr->e1.expr);
         if( evalflags & EF_PC_RELATIVE )
         {
            error("Can't invert PC-relative symbol");
            return 0;
         }
         return v1;

   default:
      error("Invalid evaluation operator");
      return 0;
   }
}

int32 evaluate(Expression *ins, Expression *expr, int argno )
{  InstrInfo *ii = ins->e1.symbol->symv.symins;

   evalflags = 0;

   switch( expr->exprtype )
   {

   case E_FREGISTER:
   case E_IREGISTER:
      {  int sh = ii->argmodes[argno]&M_S1FLD? 11:
                  ii->argmodes[argno]&M_S2FLD? 21:
                  ii->argmodes[argno]&M_DFLD ? 16:
                  ii->argmodes[argno]&M_SHFT0? 0 : -1;
         int reg = expr->e1.symbol->symv.symval;
         if( sh == -1 )
            error("Don't know where to put this register");
         if( expr->e2.expr != NULL )
         {
            reg += evaluate_integer(expr->e2.expr);
            if( evalflags )
               error("Unable to evaluate constant register offset - Assume 0");
            if( evalflags & EF_PC_RELATIVE )
            {  error("Can't handle PC relative offset from register!");
               return 0;
            }
         }
         return (reg&0x1f) << sh;
      }

   case E_CREGISTER:
         return (expr->e1.symbol->symv.symval) << 21;

   case E_REGOFFSETINC:
   case E_REGOFFSET:
      {  int32 value;
         bool is_register =  expr->e1.expr->exprtype == E_IREGISTER;
         if( is_register && (ii->argmodes[argno] & M_NOREG) )
         {  error("Register illegal here");
            return 0;
         }
         value = evaluate(ins, expr->e1.expr, argno);
         if( evalflags & EF_PC_RELATIVE )
         {
            error("Can't handle PC-relative value here");
            value = 0;
         }
/* If unresolved data symbol then convert to data symbol */
/* for resolving at link time                            */
         if( evalflags & EF_BADSYMBOL )
         {  Symbol *sym = checkdatasymb(expr->e1.expr);
            if( sym )
            {  evalflags ^= EF_BADSYMBOL;
               evalflags |= EF_DIRECTIVE;
            }
         }
         if( !is_register )
         {  if( arg_boundary( ins ) > (value & -value) && value != 0)
               warn("Offset is on invalid boundary");

            if( ii->argmodes[argno] & M_ITYPE )
               value |= 0x04000000;
         }
         return value + (expr->exprtype==E_REGOFFSET ? 0: 1);
      }

   case E_INSTR:
         warn("Can't handle instruction constants yet!");
         return 0;

   default:
         {  int32 v = evaluate_integer(expr);
            int32 amode = ii->argmodes[argno];

/* If there was an unresolved symbol then */
/* convert it to a link time reference to */
/* a label or symbol according to context */

            if( evalflags & EF_BADSYMBOL )
            {  Symbol *sym;
               if( amode & M_BRX )
                  sym = checklabelsymb(expr);
               else
                  sym = checkdatasymb(expr);
               if( sym )
               {
                  evalflags ^= EF_BADSYMBOL;
                  evalflags |= EF_DIRECTIVE;
               }
            }
/* Checks BADSYMBOL or DIRECTIVE */
            if( evalflags & ~EF_PC_RELATIVE )
               return 0;

            if( amode & M_SPLITOFFSET )
            {  if( (amode & M_BRX) &&
                   (evalflags & EF_PC_RELATIVE)    )
               {  evalflags &= ~EF_PC_RELATIVE;
                  v = (v-(pcloc+4))>>2;
               }
               return ((v & 0xf800) << 5) + (v & 0x07ff);
            }
            if( amode & M_BRX )
            {  if( evalflags & EF_PC_RELATIVE )
               {  evalflags &= ~EF_PC_RELATIVE;
                  v = (v-(pcloc+4))>>2;
               }
               return v & 0x3ffffff;
            }
            if( evalflags & EF_PC_RELATIVE )
            {  v -= pcloc;
               evalflags &= ~EF_PC_RELATIVE;
#ifdef never
               error("Can't handle PC-relative value here");
               return 0;
#endif
            }
            if( amode & M_ITYPE )
              return (v & 0xffff) | 0x04000000;
            if( amode & M_IMM5 )
              return ((v & 0x1f) << 11 ) | 0x04000000;
            if( amode & M_CONST )
              return v;
            error("Dunno how to use this constant");
            return 0;
         }
   }
}
