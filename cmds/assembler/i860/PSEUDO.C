#include "iasm.h"

int module_number = -1;

int argcount(Expression *args)
{
   int i=0;
   while(args!=NULL) i++,args = args->cdr;
   return i;
}

void docond(Expression *ins, InstrInfo *ii, Expression *args)
{
static cond_level = 0;
   switch( ii->base )
   {
   case PS_IF:
   case PS_IFD:
   case PS_IFND:
      if( skipping_level == 0 )
      {  int32 val;

         if( !checkargs(ii, args) ) return;
         if( !checkmodifiers(ii, ins) ) return;

         switch(ii->base)
         {
         case PS_IF:
            val = evaluate(ins,args,0);
            if( evalflags )
               error("Cannot evaluate condition - assuming 0");
            if( !val ) listtext = "False",start_skipping();
            else listtext = "True";
            break;

         case PS_IFD:
         case PS_IFND:
            {  bool d;

               if( args->exprtype != E_SYMBOL )
               {  error("IFD or IFND on non-symbol");
                  return;
               }
               d = (args->e1.symbol->symflags & passevalflag) != 0;
               if(ii->base == PS_IFND) d = !d;
               if( !d ) listtext = "False",start_skipping();
               else listtext = "True";
               break;
            }
         }
      }
      else { listing_flags |= LF_NOCODE;
             start_skipping(); }
      cond_level++;
      break;

   case PS_ELSE:
      if( cond_level == 0 )
      {  error("ELSE without IF");
         break;
      }

      if( skipping_level < 2 )
      {  listing_flags &= ~LF_NOCODE;  /* NOCODE set by skipping so reset it */
         if( skipping_level == 1 ) listtext = "True",stop_skipping();
         else
            if( skipping_level == 0 ) listtext = "False",start_skipping();
      }
      break;

   case PS_ENDC:
      if( cond_level == 0 )
      {  error("ENDC without IF");
         break;
      }
      --cond_level;
      if( skipping_level > 0 ) stop_skipping();
      if( skipping_level == 0 ) listing_flags &= ~LF_NOCODE;
      break;
   }
}

void dodc(Symbol *label, Expression *ins, Expression *args, InstrInfo *ii)
{  int32 l;
   int acnt=1;

   if( label != NULL ) setlabel(label);
   if( args == NULL )
   {  error("No values");
      return;
   }

   l = arg_boundary(ins);

   if( !check_pc_boundary(l) )
      warn("Defining constant on incorrect PC boundary");

   if( !checkmodifiers(ii, ins) ) return;
   if( !checkargs(ii,args) ) return;

   do
   {
      if( args->exprtype == E_STRING )
      {  char *s = args->e1.text;

         if( l != 1 )
         {  error("Can't initialise anything but bytes with a string - arg %d",acnt);
            continue;
         }
         while(*s) output(1,*s++);
      }
      else
      {
         uint32 x;
         if( pass == 2 )
         {  x = evaluate(ins, args, 0);
            if( evalflags & ~EF_DIRECTIVE)
               error("Failed to evaluate expression in arg %d",acnt);
         }
         else x = 0;

         if( evalflags & EF_DIRECTIVE )
            outputexpr(l, args);
         else
         {
            switch(l)
            {
            case 1:  output(1,(char)x); break;
            case 2:  output(2,(short)x); break;
            case 4:  output(4,x); break;
            case 8:
            case 16: warn("%d byte constants not supported yet",l);
            }
         }
      }
   } while( acnt++,(args = args->cdr) != NULL );
}

Symbol *checkdatasymb(Expression *e)
{  ETYPE et = e->exprtype;
   if( et == E_SYMBOL || et == E_AT )
   {  Symbol *sym = e->e1.symbol;
      if( sym->symtype == S_NULL )
      {  sym->symtype = S_DATA;
         return sym;
      }
      else
      {  if( sym->symtype == S_DATA )
            return sym;
         else
            return NULL;
      }
   }
   else return NULL;
}

Symbol *checklabelsymb(Expression *e)
{  ETYPE et = e->exprtype;
   if( et == E_SYMBOL )
   {  Symbol *sym = e->e1.symbol;
      if( sym->symtype == S_NULL )
      {  sym->symtype = S_LABEL;
         return sym;
      }
      else
      {  if( sym->symtype == S_LABEL )
            return sym;
         else
            return NULL;
      }
   }
   else return NULL;
}

void compile_pseudo(Symbol *label, Expression *ins, Expression *args)
{
   InstrInfo *ii = ins->e1.symbol->symv.symins;
   switch( ii->base )
   {
   case PS_XDATA:
   case PS_XLABEL:
      {  int acnt = 1;
         char *s;
         Symbol *(*fn)(Expression *arg);
         
         if(ii->base == PS_XDATA )
         {  s = "xdata";
            fn = checkdatasymb;
         }
         else
         {  s = "xlabel";
            fn = checklabelsymb;
         }
         if( label )
         {  error("%s directive should not have a label",s);
            break;
         }
         if( !checkmodifiers(ii, ins) ) return;
         if( !checkargs(ii,args) ) return;
         do { Symbol *sym;
            if( (sym = (*fn)(args)) == NULL)
            {  error("Symbol %d in %s already declared of different type",
                                                            acnt,s);
               break;
            }
         } while( acnt++,(args = args->cdr) != NULL );
         break;
      }
   case PS_DATA:
      {  int32 size;
         int argsize;

         if( !checkmodifiers(ii, ins) ) return;
         if( !checkargs(ii,args) ) return;

         argsize = arg_boundary(ins);

         if( label == NULL )
         {  error("Data statement needs label");
            break;
         }

         size = evaluate(ins,args,0);
         if( evalflags )
         {  error("Failed to evaluate size of data symbol");
            return;
         }
         resolvesym(label, dataloc, S_DATA);
         directive( D_DATA, size*argsize, label->name);
         dataloc += size*argsize;
         break;
      }

   case PS_GLOBAL:
      {  Symbol *sym;

         if( !checkmodifiers(ii, ins) ) return;
         if( !checkargs(ii,args) ) return;

         if( (sym = checkdatasymb(args)) == NULL)
         {
            error("Global definition requires simple data symbol");
            return;
         }

         directive(D_GLOBAL,sym->name);
         break;
      }
   case PS_DS:
      {  int32 val,l;
         if( label ) setlabel(label);
         l = arg_boundary(ins);
     
         if( !check_pc_boundary(l) )
            warn("Defining space on incorrect PC boundary");

         if( !checkmodifiers(ii, ins) ) return;
         if( !checkargs(ii,args) ) return;

         val = evaluate( ins,args, 0 );
         if( pass == 2 && evalflags )
         {  error("Failed to evaluate space size");
            return;
         }

         while(val--) output(l,0);

         break;
      }

   case PS_ERROR:
         if( !checkmodifiers(ii, ins) ) return;
         if( !checkargs(ii,args) ) return;
         error( args->e1.text );
         return;

   case PS_IF:
   case PS_IFD:
   case PS_IFND:
   case PS_ELSE:
   case PS_ENDC:
         docond(ins, ii, args);
         break;

   case PS_EQUR:
      {  Symbol *t = args->e1.symbol;

         if( label == 0 )
         {  error("EQUR needs register to be assigned");
            return;
         }
         if( !checkargs(ii, args) ) return;
         if( !checkmodifiers(ii, ins) ) return;
         resolvesym(label,evaluate(ins,args,0),t->symtype);
      }
      break;
   case PS_GET:
      if( label != 0 ) setlabel(label);
      if( !checkargs(ii, args) ) return;
      if( !checkmodifiers(ii, ins) ) return;
      performget(args->e1.text,0);
      break;
   case PS_SET:
   case PS_EQU:
      {  int32 val;
static char vbuf[9];
         if( label == 0 )
         {
            error("EQU/SET needs symbol to set");
            return;
         }
         if( !checkargs(ii, args) ) return;
         if( !checkmodifiers(ii, ins) ) return;

         if( !(   (label->symtype == S_NULL ) ||
                ( (label->symtype == S_SET  ) && (ii->base == PS_SET ) )
              ) )
         {
            if( (label->symflags & passevalflag) )
            {
               error("Illegal attempt to reassign symbol");
               return;
            }
         }

         val = evaluate(ins, args, 0);
         if( pass == 2 && evalflags )
         {  error("Failed to evaluate expression");
            return;
         }
         if( listingfile )
         {  sprintf(vbuf,"%08lx",val);
            listtext = vbuf;
         }
         resolvesym(label, val, ii->base==PS_SET? S_SET: S_EQU);
         return;
      }

   case PS_NOLIST:
      if( !checkargs(ii, args) ) return;
      if( !checkmodifiers(ii, ins) ) return;
      listinglevel--;
      break;

   case PS_LIST:
      if( !checkargs(ii, args) ) return;
      if( !checkmodifiers(ii, ins) ) return;

      if( args )
      {  int32 val = evaluate(ins,args,0);
         if( pass == 2 && evalflags )
         {  error("Failed to evaluate expression");
            return;
         }
         listing_options = val;
      }
      else
         listinglevel++;
      break;

   case PS_DC:
      dodc(label, ins, args, ii);
      break;
   case PS_INIT:
      if( !checkargs(ii, args) ) return;
      if( !checkmodifiers(ii, ins) ) return;
      directive(D_INIT);
      pcloc += 4;
      advancecodebuf();
      break;

   case PS_MODULE:
   {  int32 val; 
      if( !checkargs(ii, args) ) return;
      if( !checkmodifiers(ii, ins) ) return;

      val = evaluate(ins,args,0);
      if( pass == 2 && evalflags )
      {
         error("failed to evaluate module number");
         return;
      }
      directive(D_MODULE,val);
      module_number = val;
      break;
   }
   default:
      fatal("This can't happen either\n");
   }
}
