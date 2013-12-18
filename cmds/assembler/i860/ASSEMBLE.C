#include <stdio.h>
#include "iasm.h"

uint32 pcloc=0;
uint32 dataloc=0;
Relocation *relocations = NULL;

uint32 exprmask( Expression *arg )
{  uint32 emask;

   if( arg->exprtype & E_BINARYOP ) return M_CONST;

   switch( arg->exprtype & E_OPMASK)
   {
   case E_SYMBOL:
   case E_INSTR: case E_DOT:
   case E_NUMBER:   case E_UMINUS:
   case E_GE:  case E_LE: case E_GT:
   case E_LT:  case E_EQ: case E_NE:
   case E_NOT: case E_MODNUM: case E_AT: case E_IMAGESIZE:
           emask |= M_CONST; break;
   case E_IREGISTER:    emask = M_IREG; break;
   case E_FREGISTER:    emask = M_FREG; break;
   case E_CREGISTER:    emask = M_CTRLREG; break;
   case E_REGOFFSET:    emask = M_REGOFF; break;
   case E_REGOFFSETINC: emask = M_AUTOINC; break;
   case E_STRING:       emask = M_STRING; break;
   case E_NULL:         emask = M_OPTIONAL; break;
   default: error("Can't determine expression result type"
                        " - Default to constant");
            emask = M_CONST; break;
   }
   return emask;
}

int checkargs(InstrInfo *ii, Expression *args)
{  int i;
   int32 maxargno = ii->nargs;
   bool inci;

   if( maxargno == -1 )
   { maxargno = 1;
     inci = 0;
   }
   else inci = 1;

   for( i=0; args!=NULL && i < maxargno ;args=args->cdr)
   {
      if( !(exprmask(args) & ii->argmodes[i]) )
      {  error("Illegal argument type in argument no. %d",i+1);
         return 0;
      }
      if( inci ) i++;
   }

/* The conditions "i<maxargno" and "args!=NULL" are mutually exclusive */

   if( inci && (i < maxargno || args!=NULL) )
   {
/* Check for the rest of the args being optional */
      if( i < maxargno )
      {  for(; i < maxargno; i++)
            if( !(ii->argmodes[i] & M_OPTIONAL) ) break;
         if( i == maxargno ) return 1;
      }
      error("Incorrect No. of args for this instruction");
      return 0;
   }
   return 1;
}

uint32 modifiers(Expression *ins, InstrInfo *ii)
{  bool otherltype = (ii->modifiers & I_OLT)!=0;
   uint32 modval = 0;

   switch( ins->e2.insmodifiers & I_LMASK)
   {
   case I_CM:
   case I_BM:   modval = 0x00000000; break;
   case I_SM:   modval = 0x10000000; break;
   case I_LM:   modval = otherltype? 2: 0x10000001; break;
   case I_DM:   modval = 0x00000000; break;
   case I_QM:   modval = 0x00000004; break;
   case I_SSM:  modval = 0x00000000; break;
   case I_SDM:  modval = 0x00000080; break;
   case I_DSM:  modval = 0x00000100; break;
   case I_DDM:  modval = 0x00000180; break;
   case I_TM:   modval = 0x04000000; break;
   case I_BADM: warn("Bad suffix to instruction - ignored"); modval = 0; break;
   }
   if( ins->e2.insmodifiers & I_DIM )
   {  if( !check_pc_boundary(8) )
         warn("Attempting to enter DIM mode off 8 byte boundary");
      modval |= 0x200;
   }
   return modval;
}

bool check_pc_boundary(int32 n)
{
   if( (pcloc & -pcloc) >= n || (pcloc == 0) ) return 1;
   return 0;
}

int32 arg_boundary( Expression *ins )
{
   switch( ins->e2.insmodifiers & I_LMASK)
   {
   case I_BM:
      return 1;
   case I_SM:
      return 2;
   case I_LM:
      return 4;
   case I_DM:
      return 8;
   case I_QM:
      return 16;
   default:
      if( ins->e1.symbol->symv.symins->modifiers & I_FLUSH ) return 16;
      warn("boundary indeterminable");
      return 0x1000000;
   }
}

void relocate(Expression *expr, uint32 pcloc, int argno, uint32 amode )
{
   Relocation *reloc = (Relocation *)aalloc(sizeof(Relocation));
   reloc->cdr = relocations;
   relocations = reloc;
   reloc->expr = expr;
   reloc->pcloc = pcloc;
   reloc->argno = argno;
   reloc->amode = amode;
}

int checkmodifiers(InstrInfo *ii, Expression *ins)
{
   if( ins->e2.insmodifiers & ~(ii->modifiers) )
   {
      error("Illegal modifier set in instruction");
      return 0;
   }

   if( ii->modifiers & I_LNEEDED )
   {
      if( (ins->e2.insmodifiers & I_LMASK) == 0 )
      {  error("Length modifier required");
         return 0;
      }
      else
         return 1;
   }
   return 1;
}

void setlabel(Symbol *lab)
{
   if( pass == 2 )
   {  if( lab->symflags & sf_eval2 )
      {
         error("Label %s already set",lab->name);
         return;
      }
   }
   resolvesym(lab, pcloc, S_LABEL);
   if( !(lab->symflags & sf_invisible) )
      lastlabel = lab;
   if( lab->symflags & sf_exported )
      directive(D_LABEL, lab->name);
}

void gen_directive( int n, int32 val, Expression *arg)
{
   int ldir = n == 1? D_BYTE:
              n == 2? D_SHORT:
              n == 4? D_WORD: -1;

   if( ldir == -1 )
   {  error("Bad length for directive generation");
      return;
   }

   switch( arg->exprtype )
   {
   case E_REGOFFSET:
   case E_REGOFFSETINC:
      {  Expression *expr1 = arg->e1.expr;
         Symbol *sym = expr1->e1.symbol;
         int rtype = expr1->exprtype == E_AT ? D_DATAMODULE: D_DATASYMB;

         directive(ldir, D_ADD, val, rtype, expr1->e1.symbol->name);
         break;
      }

   case E_MODNUM:
      directive(ldir, D_ADD, val, D_MODNUM );
      break;

   case E_IMAGESIZE:
      directive(ldir, D_ADD, val, D_IMAGESIZE );
      break;

   case E_AT:
      {  Symbol *sym = checkdatasymb(arg);
         if( !sym )
         {  error("Attempt to apply @ to non-data symbol");
            output(4,0);   /* Keep PC in step */
         }
         else
            directive(ldir, D_ADD, val, D_DATAMODULE, sym->name);
      }
      break;

   case E_SYMBOL:
      {  Symbol *sym = arg->e1.symbol;
         switch( sym->symtype )
         {
         case S_DATA:
            directive(ldir, D_ADD, val, D_DATASYMB, sym->name);
            break;
         case S_LABEL:
            directive(ldir, D_ADD, val, D_SHIFT, -2, D_LABELOFF, sym->name);
            break;
         default:
            output(4,0);   /* Keep PC in step */
            error("Cannot handle relocation of symbol type %d",
                          sym->symtype);
            break;
         }
      }
      break;

   default:
      {
         output(4,0);   /* Keep PC in step */
         error("Cannot handle relocation of expression type %d",
                       arg->exprtype&E_OPMASK);
         break;
      }
   }
}

int compile_instruction(Expression *instr, Expression *args)
{  InstrInfo *ii = instr->e1.symbol->symv.symins;
   uint32 insval  = ii->base;
   int i;
   Expression *argdirect = NULL;

   if( pass == 1 )
   {  output(4,0);
      return 0;
   }

   if( !check_pc_boundary(4) )
       error("Assembling instruction off 4 byte boundary");

   if( !(checkmodifiers(ii, instr) && checkargs(ii,args)) )
   {  output(4,0);
      return 0;
   }

   insval |= modifiers(instr, ii);

   for( i=0; args; i++,args=args->cdr)
   {
      insval |= evaluate(instr, args, i);
      if( evalflags )
      {  if( evalflags & EF_DIRECTIVE )
            argdirect = args;
         else
            error("Unable to evaluate argument %d",i+1);
      }
   }
   if( argdirect == NULL )
      output(4,insval);
   else
      gen_directive( 4, insval, argdirect );
   return 1;
}

void assemble(Line *line)
{  Symbol *lab = line->label;
   Expression *instr = line->instr;

   if( instr != NULL )
   {
      switch( instr->exprtype )
      {
      case E_INSTR:
         {  InstrInfo *ii = instr->e1.symbol->symv.symins;

            if( ii->modifiers & I_PSEUDO )
               compile_pseudo(lab, instr, line->args );
            else
            {
               if( lab != NULL ) setlabel(lab);
               compile_instruction(instr, line->args );
            }
            break;
         }
      default:
         fatal("This can't happen");
      }
   }
   else
      if( lab != NULL ) setlabel(lab);
   outlistline();
/* Save space if buffer is empty */
   if( pcloc == bufstart ) reinitcodebuf(bufstart);
   free_expressions();
}
