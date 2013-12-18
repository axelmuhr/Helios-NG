/**
*
* Title:  Helios Source Level Debugger - C expression evaluator.
*
* Author: Andy England
*
* Date:   September 1988
*
*         (c) Copyright 1988 - 1993, Perihelion Software Ltd.
*
*         All Rights reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/eval.c,v 1.13 1993/07/06 14:09:24 nickc Exp $";
#endif

#include "tla.h"

/*
-- crf : 17/08/91 - used in checkstack()
*/
extern BOOL debugging ;

#define T_PreInc	T_PlusPlus
#define T_PreDec	T_MinusMinus
#define T_Mul		T_Times
#define T_Div		T_Divide
#define T_Rem		T_Remainder
#define T_Add		T_Plus
#define T_Sub		T_Minus
#define T_Not		T_LogNot
#define T_MulEq		T_TimesEq
#define T_DivEq		T_DivideEq
#define T_RemEq		T_RemainderEq
#define T_AddEq		T_PlusEq
#define T_SubEq		T_MinusEq

PUBLIC void genexpr(EVAL *, EXPR *);
PUBLIC void genvoidexpr(EVAL *, EXPR *);
PUBLIC void genaddr(EVAL *, EXPR *);
PUBLIC void lvalue(EVAL *, ENTRY *);
PUBLIC void rvalue(EVAL *, ENTRY *);
PRIVATE void load(EVAL *, int);
PRIVATE void store(EVAL *, int);
PRIVATE void iinc(EVAL *);
PRIVATE void idec(EVAL *);
PRIVATE void finc(EVAL *);
PRIVATE void fdec(EVAL *);
PRIVATE void iadd(EVAL *);
PRIVATE void isub(EVAL *);
PRIVATE void imul(EVAL *);
PRIVATE void idiv(EVAL *);
PRIVATE void irem(EVAL *);
PRIVATE void fadd(EVAL *);
PRIVATE void fsub(EVAL *);
PRIVATE void fmul(EVAL *);
PRIVATE void fdiv(EVAL *);
PRIVATE void ishl(EVAL *);
PRIVATE void ishr(EVAL *);
PRIVATE void ilt(EVAL *);
PRIVATE void igt(EVAL *);
PRIVATE void ile(EVAL *);
PRIVATE void ige(EVAL *);
PRIVATE void ieq(EVAL *);
PRIVATE void ine(EVAL *);
PRIVATE void flt(EVAL *);
PRIVATE void fgt(EVAL *);
PRIVATE void fle(EVAL *);
PRIVATE void fge(EVAL *);
PRIVATE void feq(EVAL *);
PRIVATE void fne(EVAL *);
PRIVATE void ibnot(EVAL *);
PRIVATE void iand(EVAL *);
PRIVATE void ior(EVAL *);
PRIVATE void ixor(EVAL *);
PRIVATE void inot(EVAL *);
PRIVATE void ineg(EVAL *);
PRIVATE void fneg(EVAL *);
PRIVATE void call(EVAL *, int, int);
PRIVATE void cast(EVAL *, TYPE *, TYPE *);

/**
*
* eval = neweval(debug);
*
* Create a new evaluator.
*
**/
PUBLIC EVAL *neweval(DEBUG *debug)
{
  EVAL *eval = NEW(EVAL);

  eval->debug = debug;
  eval->backtracked = FALSE;
  eval->stackptr = eval->stack + STACK_SIZE;
  return eval;
}

/**
*
* remeval(eval);
*
* Remove an evaluator.
*
**/
PUBLIC void remeval(EVAL *eval)
{
  freemem(eval);
}

/**
*
* checkstack(eval);
*
* Temporary routine to check the consistency of evaluator stack.
*
**/
PUBLIC void checkstack(EVAL *eval)
{
  DISPLAY *display = eval->debug->display;

/*
-- crf : 17/08/91
-- Is there really any point in worrying the user with messages of this type ?
-- I have managed to reduce the occurence of these messages, but they still
-- come up now and again (e.g. if you watchpoint an expression and then
-- print). This problem will have to be re-examined ... in the meantime, I
-- maintain that it is more sensible to enable these messages with the
-- command line debugging option ("-d").
*/
  if (debugging)
  {
    if (eval->stackptr > eval->stack + STACK_SIZE)
      dprintf(display, "\nPopped %d bytes too much.", 
                       eval->stackptr - eval->stack + STACK_SIZE);
    else if (eval->stackptr < eval->stack + STACK_SIZE)
      dprintf(display, "\nPopped %d bytes too little.", 
                       eval->stack + STACK_SIZE - eval->stackptr);
  }
  eval->stackptr = eval->stack + STACK_SIZE;
}

PUBLIC void stackusage(EVAL *eval)
{
  DISPLAY *display = eval->debug->display;

  dprintf(display, "\n%d bytes used.\n", (eval->stack + STACK_SIZE) - eval->stackptr);
}

PRIVATE void push(EVAL *eval, int val)
{
#ifdef __HELIOS /* -- crf : Pardebug - replace "HELIOS" with "__HELIOS" */
  eval->stackptr -= sizeof(int);
  *((int *)eval->stackptr) = val;
#else
  *--((int *)eval->stackptr) = val;
#endif
}

PRIVATE void ipush(EVAL *eval, int val, int size)
{
  eval->stackptr -= ALIGN(size);
  switch (size)
  {
    case sizeof (char):
    *((unsigned char *)eval->stackptr) = val;
    break;

    case sizeof (short):
    *((unsigned short *)eval->stackptr) = val;
    break;

    case sizeof (int):
    *((int *)eval->stackptr) = val;
    break;
  }
}

PRIVATE void fpush(EVAL *eval, float val)
{
#ifdef __HELIOS /* -- crf : Pardebug - replace "HELIOS" with "__HELIOS" */
  eval->stackptr -= sizeof(float);
  *((float *)eval->stackptr) = val;
#else
  *--((float *)eval->stackptr) = val;
#endif
}

PRIVATE void dpush(EVAL *eval, double val)
{
#ifdef __HELIOS /* -- crf : Pardebug - replace "HELIOS" with "__HELIOS" */
  eval->stackptr -= sizeof(double);
  *((double *)eval->stackptr) = val;
#else
  *--((double *)eval->stackptr) = val;
#endif
}

PRIVATE void ppush(EVAL *eval, void *p)
{
#ifdef __HELIOS /* -- crf : Pardebug - replace "HELIOS" with "__HELIOS" */
  eval->stackptr -= sizeof(void *);
  *((void **)eval->stackptr) = p;
#else
  *--((void **)eval->stackptr) = p;
#endif
}

PRIVATE void mpush(EVAL *eval, void *addr, int size)
{
  eval->stackptr -= ALIGN(size);
  peekmem(eval->debug, eval->stackptr, addr, size);
}

PRIVATE void dupeval(EVAL *eval, int offset, int size)
{
  eval->stackptr -= ALIGN(size);
  memmove(eval->stackptr, eval->stackptr + ALIGN(size) + offset, size);
}

PUBLIC int pop(EVAL *eval)
{
#ifdef __HELIOS /* -- crf : Pardebug - replace "HELIOS" with "__HELIOS" */
  int val;

  val = *((int *)eval->stackptr);
  eval->stackptr += sizeof(int);
  return val;
#else
  return *((int *)eval->stackptr)++;
#endif
}

PUBLIC int ipop(EVAL *eval, int size)
{
  int val = 0;

  switch (size)
  {
  case sizeof (char):
    val = *((unsigned char *)eval->stackptr);
    break;

  case sizeof (short):
    val = *((unsigned short *)eval->stackptr);
    break;

  case sizeof (int):
    val = *((int *)eval->stackptr);
    break;

  default:
    debugf( "ipop: corrupt stack size" );
    break;
  }
  eval->stackptr += ALIGN(size);
  return val;
}

PRIVATE float fpop(EVAL *eval)
{
#ifdef __HELIOS /* -- crf : Pardebug - replace "HELIOS" with "__HELIOS" */
  float val;

  val = *((float *)eval->stackptr);
  eval->stackptr += sizeof(float);
  return val;
#else
  return *((float *)eval->stackptr)++;
#endif
}

PRIVATE double dpop(EVAL *eval)
{
#ifdef __HELIOS /* -- crf : Pardebug - replace "HELIOS" with "__HELIOS" */
  double val;

  val = *((double *)eval->stackptr);
  eval->stackptr += sizeof(double);
  return val;
#else
  return *((double *)eval->stackptr)++;
#endif
}

/* ACE: prefer if this was private */
PUBLIC byte *ppop(EVAL *eval)
{
#ifdef __HELIOS /* -- crf : Pardebug - replace "HELIOS" with "__HELIOS" */
  byte *p;

  p = *((byte **)eval->stackptr);
  eval->stackptr += sizeof(byte *);
  return p;
#else
  return *((void **)eval->stackptr)++;
#endif
}

PRIVATE char *spop(EVAL *eval, int size)
{
  char *s = (char *)newmem(size + 1);
  int i = 0;

  while (i < size)
  {
    s[i] = eval->stackptr[i];
    i++;
  }
  s[i] = '\0';
  eval->stackptr += ALIGN(size);
  return s;
}

PRIVATE unsigned short *
short_pop(
	  EVAL *	eval,
	  int		size )
{
  unsigned short *	s = (unsigned short *)newmem( size * sizeof (short) );
  int			i = 0;

  
  while (i < size)
    {
      s[i] = ((unsigned short *)eval->stackptr)[i];
      i++;
    }

  eval->stackptr += ALIGN(size);
  
  return s;
  
} /* short_pop */


/* ACE: I think this is redundant */
/* 
-- crf : 18/07/91 - "declared but not used ...
PRIVATE void mpop(EVAL *eval, void *addr, int size)
{
  pokemem(eval->debug, eval->stackptr, addr, size);
  eval->stackptr += ALIGN(size);
}
*/

PRIVATE void mstore(EVAL *eval, void *addr, int size)
{
  pokemem(eval->debug, eval->stackptr, addr, size);
}

PRIVATE void drop(EVAL *eval, int size)
{
  eval->stackptr += ALIGN(size);
}

PUBLIC void genexpr(EVAL *eval, EXPR *expr)
{
#ifdef THIS_IS_HIGHLY_STUPID_AND_CRASHES_HELIOS
  debugf("function name in genexpr = %s",expr->identifier.entry->name);
#endif
  
  switch (expr->generic.op)
  {
    case T_Identifier:
    rvalue(eval, expr->identifier.entry);
    break;

    case T_Constant:
    /* ACE: should store pointer to value in constant node */
    {
      TYPE *type = typeofexpr(expr);

      
      if (isintegral(type))
	{
	  push(eval, expr->constant.value.integral);
	}      
      else if (sizeoftype(type) == sizeof(float))
	{
	  fpush(eval, expr->constant.value.floating4);
	}      
      else
	{
	  dpush(eval, expr->constant.value.floating8);
	}      
    }
    break;

    case T_String:
    /* ACE: do we need to load the whole string onto the stack ? */
    ppush(eval, expr->string.value);
    break;

    case T_Subscript:
    break;

    case T_Call:
    genexpr(eval, expr->generic.expr2);
    genexpr(eval, expr->generic.expr1);
    call(eval, sizeofexpr(expr->generic.expr2), sizeofexpr(expr));
    break;

    case T_Dot:
    case T_Arrow:
    genaddr(eval, expr);
    load(eval, sizeofexpr(expr->generic.expr2));
    break;

    case T_PostInc:
    genexpr(eval, expr->generic.expr1);
    dupeval(eval, 0, sizeofexpr(expr->generic.expr1));
    if (isfloat(expr->generic.type)) finc(eval);
    else iinc(eval);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    drop(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_PostDec:
    genexpr(eval, expr->generic.expr1);
    dupeval(eval, 0, sizeofexpr(expr->generic.expr1));
    if (isfloat(expr->generic.type)) fdec(eval);
    else idec(eval);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    drop(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_PreInc:
    genexpr(eval, expr->generic.expr1);
    if (isfloat(expr->generic.type)) finc(eval);
    else iinc(eval);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_PreDec:
    genexpr(eval, expr->generic.expr1);
    if (isfloat(expr->generic.type)) fdec(eval);
    else idec(eval);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_Address:
    genaddr(eval, expr->generic.expr1);
    break;

    case T_Indirect:
    genexpr(eval, expr->generic.expr1);
    load(eval, strideofexpr(expr->generic.expr1));
    break;

    case T_UPlus:
    genexpr(eval, expr->generic.expr1);
    break;

    case T_UMinus:
    genexpr(eval, expr->generic.expr1);
    if (isfloat(expr->generic.type)) fneg(eval);
    else ineg(eval);
    break;

    case T_BitNot:
    genexpr(eval, expr->generic.expr1);
    ibnot(eval);
    break;

    case T_Not:
    genexpr(eval, expr->generic.expr1);
    inot(eval);
    break;

    case T_Cast:
    genexpr(eval, expr->cast.expr);
    cast(eval, expr->cast.type, typeofexpr(expr->cast.expr));
    break;

    case T_Mul:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    if (isfloat(expr->generic.type)) fmul(eval);
    else imul(eval);
    break;

    case T_Div:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    if (isfloat(expr->generic.type)) fdiv(eval);
    else idiv(eval);
    break;

    case T_Rem:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    irem(eval);
    break;

    case T_Add:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    if (isfloat(expr->generic.type)) fadd(eval);
    else iadd(eval);
    break;

    case T_Sub:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    if (isfloat(expr->generic.type)) fsub(eval);
    else isub(eval);
    break;

    case T_LShift:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    ishl(eval);
    break;

    case T_RShift:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    ishr(eval);
    break;

    case T_LT:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    if (isfloat(expr->generic.type)) flt(eval);
    else ilt(eval);
    break;

    case T_GT:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    if (isfloat(expr->generic.type)) fgt(eval);
    else igt(eval);
    break;

    case T_LE:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    if (isfloat(expr->generic.type)) fle(eval);
    else ile(eval);
    break;

    case T_GE:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    if (isfloat(expr->generic.type)) fge(eval);
    else ige(eval);
    break;

    case T_EQ:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    if (isfloat(expr->generic.type)) feq(eval);
    else ieq(eval);
    
    break;

    case T_NE:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    if (isfloat(expr->generic.type)) fne(eval);
    else ine(eval);
    break;

    case T_BitAnd:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    iand(eval);
    break;

    case T_BitXOr:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    ixor(eval);
    break;

    case T_BitOr:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    ior(eval);
    break;

    case T_LogAnd:
    push(eval, (genexpr(eval, expr->generic.expr1), pop(eval)) &&
               (genexpr(eval, expr->generic.expr2), pop(eval)));
    break;

    case T_LogOr:
    push(eval, (genexpr(eval, expr->generic.expr1), pop(eval)) ||
               (genexpr(eval, expr->generic.expr2), pop(eval)));
    break;

    case T_Conditional:
    push(eval, (genexpr(eval, expr->generic.expr1), pop(eval)) ?
               (genexpr(eval, expr->generic.expr2), pop(eval)) :
               (genexpr(eval, expr->generic.expr3), pop(eval)));
    break;

    case T_Assign:
    genexpr(eval, expr->generic.expr2);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_MulEq:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    if (isfloat(expr->generic.type)) fmul(eval);
    else imul(eval);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_DivEq:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    if (isfloat(expr->generic.type)) fdiv(eval);
    else idiv(eval);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_RemEq:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    irem(eval);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_AddEq:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    if (isfloat(expr->generic.type)) fadd(eval);
    else iadd(eval);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_SubEq:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    if (isfloat(expr->generic.type)) fsub(eval);
    else isub(eval);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_LShiftEq:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    ishl(eval);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_RShiftEq:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    ishr(eval);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_BitAndEq:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    iand(eval);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_BitXOrEq:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    ixor(eval);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_BitOrEq:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    ior(eval);
    genaddr(eval, expr->generic.expr1);
    store(eval, sizeofexpr(expr->generic.expr1));
    break;

    case T_Comma:
    genvoidexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    break;

    case T_List:
    genexpr(eval, expr->generic.expr1);
    genexpr(eval, expr->generic.expr2);
    break;

    default:
    IOdebug( "TLA: Bad operator %s in genexpr", tokennames[(int)expr->generic.op]);
    break;
  }
}

PUBLIC void genvoidexpr(EVAL *eval, EXPR *expr)
{
  genexpr(eval, expr);
  drop(eval, sizeofexpr(expr));
}

PUBLIC void genaddr(EVAL *eval, EXPR *expr)
{
  switch (expr->generic.op)
  {
    case T_Identifier:
    lvalue(eval, expr->identifier.entry);
    break;

/*
-- crf : 24/07/91 - Bug 702
-- Problem : watchpointing expressions (e.g. "i * 2") does not work. This is
-- because genaddr() does not cater for operators (gives IOdebug message "Bad 
-- operator <op> in genaddr").
-- Solution : if expression contains an operator (I have catered for those
-- that are listed directly below) evaluate the expression ("genexpr()")
-- and then call "genaddr()" with expr1.
*/
    case T_Cast:
    case T_Mul:
    case T_Div:
    case T_Rem:
    case T_Add:
    case T_Sub:
    case T_LShift:
    case T_RShift:
    genexpr (eval, expr) ;
    genaddr (eval, expr->generic.expr1) ;
    break;

    case T_Indirect:
    genexpr(eval, expr->generic.expr1);
    break;

    case T_Dot:
    {
      int offset;

      genaddr(eval, expr->generic.expr1);
      genaddr(eval, expr->generic.expr2);
      offset = pop(eval);
      ppush(eval, ppop(eval) + offset);
    }
    break;

    case T_Arrow:
    {
      int offset;

      genexpr(eval, expr->generic.expr1);
      genaddr(eval, expr->generic.expr2);
      offset = pop(eval);
      ppush(eval, ppop(eval) + offset);
    }
    break;

    default:
    IOdebug( "TLA: Bad operator %s in genaddr", tokennames[(int)expr->generic.op]);
    break;
  }
}

/**
*
* genparam(eval, param, frame);
*
* Mimics a call to rvalue() for a parameter down the stack frame.
*
**/
PUBLIC void genparam(EVAL *eval, PARAM *param, int frame)
{
#ifdef __C40
  ppush(eval, locateframe(eval->debug, eval->debug->thread->id, frame, param->offset));
#else
  ppush(eval, locatestack(eval->debug, eval->debug->thread->id, frame, param->offset));
#endif
  mpush(eval, ppop(eval), sizeoftype(param->type));	
}

PUBLIC void lvalue(EVAL *eval, ENTRY *entry)
{
  switch (entry->Class)
  {
    case C_Extern:
    case C_Static:
    if (entry == NULL || entry->block == NULL)
      return;
    
    ppush(eval, locatedata(eval->debug, entry->block->module->modnum, entry->offset));
    if (isfunction(entry->type)) mpush(eval, ppop(eval), sizeof(void *));
    return;

    case C_Auto: /* ACE: get frame from somewhere */
    ppush(eval, locatestack(eval->debug, eval->debug->thread->id, 0, entry->offset));

#ifdef __TRAN
    /* ACE: large structures stored on vector stack */
    if (sizeoftype(entry->type) > 8) mpush(eval, ppop(eval), sizeof(void *));
#endif
    
    return;

    case C_Param:
#ifdef __C40
    ppush(eval, locateframe(eval->debug, eval->debug->thread->id, 0, entry->offset));
#else
    ppush(eval, locatestack(eval->debug, eval->debug->thread->id, 0, entry->offset));
#endif
#ifdef OLDCODE
/* ACE: Is this the right place to do this ?
        No it's not - this is a feature of C not our implementation  */
    if (isarray(entry->type)) mpush(eval, ppop(eval), sizeof(void *));
#endif
    return;

    case C_Member:
    push(eval, entry->offset);
    return;

#ifdef __C40
    case C_Register:
    ppush( eval, locateregister( eval->debug, eval->debug->thread->id, 0, entry->offset ) );
    return;
#endif
    
    default:
    IOdebug( "TLA: Unknown class %d in lvalue", entry->Class);
    ppush(eval, NULL);
    return;
  }
}

PUBLIC void rvalue(EVAL *eval, ENTRY *entry)
{
#ifdef NEWCODE
  switch (entry->Class)
  {
  }
#else
  lvalue(eval, entry);
  mpush(eval, ppop(eval), sizeoftype(entry->type));
#endif
}

PRIVATE void load(EVAL *eval, int size)
{
  mpush(eval, ppop(eval), size);
}

PRIVATE void store(EVAL *eval, int size)
{
  mstore(eval, ppop(eval), size);
}

PRIVATE void call(EVAL *eval, int sizeofparams, int sizeofreturn)
{
  void *addr = ppop(eval);

  syscall(eval->debug, addr, sizeofparams, eval->stackptr,
          sizeofreturn, eval->stackptr + sizeofparams - sizeofreturn);
  eval->stackptr += sizeofparams - sizeofreturn;
}

PRIVATE void cast(EVAL *eval, TYPE *newtype, TYPE *oldtype)
{
  int oldsize = sizeoftype(oldtype);
  int newsize = sizeoftype(newtype);

  if (newsize == 0) drop(eval, oldsize);
  else if (isintegral(newtype))
  {
    if (isintegral(oldtype))
      ipush(eval, ipop(eval, oldsize), newsize);
    else if (isfloat(oldtype))
    {
      if (oldsize == 8) push(eval, (int)dpop(eval));
      else push(eval, (int)fpop(eval));
    }
  }
  else if (isfloat(newtype))
  {
    if (newsize == 8)
    {
      if (isintegral(oldtype)) dpush(eval, (double)pop(eval));
      else if (isfloat(oldtype) AND oldsize == 4)
        dpush(eval, (double)fpop(eval));
    }
    else if (isintegral(oldtype)) fpush(eval, (float)pop(eval));
    else if (isfloat(oldtype) AND oldsize == 8) fpush(eval, (float)dpop(eval));
  }
}

PRIVATE void ineg(EVAL *eval)
{
  push(eval, -pop(eval));
}

PRIVATE void fneg(EVAL *eval)
{
  fpush(eval, -fpop(eval));
}

PRIVATE void ishl(EVAL *eval)
{
  int val = pop(eval);

  push(eval, pop(eval) << val);
}

PRIVATE void ishr(EVAL *eval)
{
  int val = pop(eval);

  push(eval, pop(eval) >> val);
}

PRIVATE void ilt(EVAL *eval)
{
  int val = pop(eval);

  push(eval, pop(eval) < val);
}

PRIVATE void igt(EVAL *eval)
{
  int val = pop(eval);

  push(eval, pop(eval) > val);
}

PRIVATE void ile(EVAL *eval)
{
  int val = pop(eval);

  push(eval, pop(eval) <= val);
}

PRIVATE void ige(EVAL *eval)
{
  int val = pop(eval);

  push(eval, pop(eval) >= val);
}

PRIVATE void ieq(EVAL *eval)
{
  push(eval, pop(eval) == pop(eval));
}

PRIVATE void ine(EVAL *eval)
{
  push(eval, pop(eval) != pop(eval));
}

PRIVATE void flt(EVAL *eval)
{
  float val = fpop(eval);

  push(eval, fpop(eval) < val);
}

PRIVATE void fgt(EVAL *eval)
{
  float val = fpop(eval);

  push(eval, fpop(eval) > val);
}

PRIVATE void fle(EVAL *eval)
{
  float val = fpop(eval);

  push(eval, fpop(eval) <= val);
}

PRIVATE void fge(EVAL *eval)
{
  float val = fpop(eval);

  push(eval, fpop(eval) >= val);
}

PRIVATE void feq(EVAL *eval)
{
  push(eval, fpop(eval) == fpop(eval));
}

PRIVATE void fne(EVAL *eval)
{
  push(eval, fpop(eval) != fpop(eval));
}

PRIVATE void ibnot(EVAL *eval)
{
  push(eval, ~pop(eval));
}

PRIVATE void iand(EVAL *eval)
{
  push(eval, pop(eval) & pop(eval));
}

PRIVATE void ixor(EVAL *eval)
{
  push(eval, pop(eval) ^ pop(eval));
}

PRIVATE void ior(EVAL *eval)
{
  push(eval, pop(eval) | pop(eval));
}

PRIVATE void inot(EVAL *eval)
{
  push(eval, !pop(eval));
}

PRIVATE void iinc(EVAL *eval)
{
  push(eval, pop(eval) + 1);
}

PRIVATE void idec(EVAL *eval)
{
  push(eval, pop(eval) - 1);
}

PRIVATE void finc(EVAL *eval)
{
  fpush(eval, fpop(eval) + 1.0);
}

PRIVATE void fdec(EVAL *eval)
{
  fpush(eval, fpop(eval) - 1.0);
}

PRIVATE void iadd(EVAL *eval)
{
  push(eval, pop(eval) + pop(eval));
}

PRIVATE void fadd(EVAL *eval)
{
  fpush(eval, fpop(eval) + fpop(eval));
}


/* 
-- crf : 18/07/91 - "declared but not used ...
PRIVATE void dadd(EVAL *eval)
{
  dpush(eval, dpop(eval) + dpop(eval));
}
*/

PRIVATE void isub(EVAL *eval)
{
  int val = pop(eval);

  push(eval, pop(eval) - val);
}

PRIVATE void fsub(EVAL *eval)
{
  float val = fpop(eval);

  fpush(eval, fpop(eval) - val);
}


/* 
-- crf : 18/07/91 - "declared but not used ...
PRIVATE void dsub(EVAL *eval)
{
  double val = dpop(eval);

  dpush(eval, dpop(eval) - val);
}
*/

PRIVATE void imul(EVAL *eval)
{
  push(eval, pop(eval) * pop(eval));
}

PRIVATE void fmul(EVAL *eval)
{
  fpush(eval, fpop(eval) * fpop(eval));
}

/* 
-- crf : 18/07/91 - "declared but not used ...
PRIVATE void dmul(EVAL *eval)
{
  dpush(eval, dpop(eval) * dpop(eval));
}
*/

PRIVATE void idiv(EVAL *eval)
{
  int val = pop(eval);

  push(eval, pop(eval) / val);
}

PRIVATE void fdiv(EVAL *eval)
{
  float val = fpop(eval);

  fpush(eval, fpop(eval) / val);
}

/* 
-- crf : 18/07/91 - "declared but not used ...
PRIVATE void ddiv(EVAL *eval)
{
  double val = dpop(eval);

  dpush(eval, dpop(eval) / val);
}
*/

PRIVATE void irem(EVAL *eval)
{
  int val = pop(eval);

  push(eval, pop(eval) % val);
}

#ifdef OLDCODE
PRIVATE void putlvalue(DISPLAY *display, TYPE *type, int offset)
{
  type = skiptypedef(type);
  if (isstructure(type))
  {
    MEMBER *member;

    unless ((member = whichmember(type, offset)) == NULL)
    {
      lvalue(member);
      offset -= pop(eval);
      dprintf(display, ".%s", member->name);
      putlvalue(display, member->type, offset);
    }
    return;
  }
  if (isarray(type))
  {
    dprintf(display, "[%d]", offset / sizeoftype(type->array.host));
    putlvalue(display, type->array.host, offset % sizeoftype(type->array.host));
    return;
  }
  unless (offset == 0) dprintf(display, " + %d", offset);
}
#endif

PRIVATE void putcharconst(DISPLAY *display, int c)
{
  if (isprint(c))
  {
    if (c == '\'' OR c == '\"' OR c == '\\') dputc(display, '\\');
    dputc(display, c);
  }
  else
  {
    dputc(display, '\\');
    switch (c)
    {
      case '\a':
      dputc(display, 'a');
      break;

      case '\b':
      dputc(display, 'b');
      break;

      case '\f':
      dputc(display, 'f');
      break;

      case '\n':
      dputc(display, 'n');
      break;

      case '\r':
      dputc(display, 'r');
      break;

      case '\t':
      dputc(display, 't');
      break;

      case '\v':
      dputc(display, 'v');
      break;

      default:
      dprintf(display, "%o", c);
      break;
    }
  }
}

PRIVATE void putstr(DEBUG *debug, char *s)
{
  DISPLAY *display = debug->display;
  char buffer[81];
  int i = 0;
  int c;

  peekmem(debug, buffer, s, 80);
  buffer[80] = '\0';
  dputc(display, '\"');
  until ((c = buffer[i++]) == '\0') putcharconst(display, c);
  dputc(display, '\"');
}

PRIVATE BOOL isvalidptr(void *p)
{
#ifdef __TRAN
  return (((word)p & 0x80000000) == 0) ? FALSE : TRUE;
#else
  return (((word)p) > 0x100);  
#endif
}

/**
*
* putvalue(debug, type, format, indent, chase);
*
* Display a objects value.
*
**/
PUBLIC void putvalue(DEBUG *debug, TYPE *type, FORMAT format, int indent, int chase)
{
  DISPLAY *display = debug->display;
  EVAL *eval = debug->eval;
  BOOL char_type = FALSE ; /* -- crf : Bug 707, 709 - see below */

  forever
  {
    type = skipreuse(type);
    
    switch (type->generic.id)
    {
      case TI_Tag:
      if (type->typename.type == NULL)
      {
        tagerr(eval, "Undefined structure tag %s", GetTypeEntry(type)->name);
	return;
      }
      type = type->typename.type;
      continue;
      
      case TI_Typedef:
      if (strequ(GetTypeEntry(type)->name, "char"))
      {
/*
-- crf : 23/07/91 - Bug 707
-- Problem : printing character variables, or casting to character, gives the
-- message "Popped 2004 bytes too much". This is because the value is 
-- "ipopped" here (i.e. case TI_Typedef) and below (case TI_Integral).
-- crf : 24/07/91 - Bug 709
-- Problem : actual display of character values (or casts) is strange - e.g.
-- "print (char) i" where i=2 gives "'\2'00000002" (i.e. octal + hex value).
-- This happens because the value is displayed here, and then the type is
-- changed to TI_Integral (via "skipreuse()"). On the next iteration, the
-- program goes into TI_Integral, and redisplays the value (case Default).
-- Solution (to both bugs) : set a flag indicating that a character value
-- (or cast) has been found, and let the program go into TI_Integral and
-- find "case Default". Then check if the type of the value is character, and 
-- display it accordingly.

-- crf : 24/07/91 - Bug 485 
-- (this bug was given to me as 377 ... it is really 485)
-- I didn't mean to, but the above changes have fixed Bug 485 (this concerned
-- getting incorrect values when printing out an MCB structure). The reason
-- for the error was that which concerns Bug 707 - i.e. duplicate popping.
*/
        char_type = TRUE ;

#ifdef OLDCODE
        dputc(display, '\'');
        putcharconst(display, ipop(eval, sizeoftype(type)));
        dputc(display, '\'');
#endif
      }
      type = skiptypedef(type);
      continue;

      case TI_Integral:
      unless (sizeoftype(type) == 0)
      {
        int val = ipop(eval, sizeoftype(type));

        switch (format)
        {
          case Ascii:
#ifdef OLDCODE
          dprintf(display, "%c", val);
#endif
/* 
-- crf : 24/07/91 - more meaningful to use "putcharconst"
*/
          dputc(display, '\'');
          putcharconst(display, val) ;
          dputc(display, '\'');
          break;

          case Binary:
          {
            int i;

            for (i = 31; i >= 0; i--)
              dprintf(display, "%d", (val >> i) & 0x1);
          }
          break;

          case Decimal:
          dprintf(display, "%d", val);
          break;

          case Error:
          {
            char msg[128];

            Fault(val, msg, 128);
            dprintf(display, "%08x: \"%s\"", val, msg);
          }
          break;

          case Float:
          dprintf(display, "%f", val);
          break;

          case Hexadecimal:
          dprintf(display, "0x%08x", val);
          break;

          case Octal:
          dprintf(display, "%#o", val);
          break;

          case Unsigned:
          dprintf(display, "%u", val);
          break;

          case Default:
          {
/*
-- crf : Bug 707, 709 - see above
*/
            if (char_type)
            {
              dputc(display, '\'');
              putcharconst(display, val) ;
              dputc(display, '\'');
              char_type = FALSE ;
            }
            else
            {
              if (isunsigned(type)) dprintf(display, "0x%08x", val);
              else dprintf(display, "%d", val);
            }
            break;
          }
        }
      }
      return;

      case TI_Float:
/* 
-- crf : 17/07/91 - Bug 529
-- Regarding the print format of floating point variables (e.g. 
-- "print -h var_name"). The best way to handle this is open to question - 
-- what do you really hope to see when you format a floating point value as a
-- character (say). The way I've handled it is as follows :
-- Float - use "%f" or "%lf" (as before)
-- Ascii, Decimal, Error, Unsigned - cast to integer
-- Binary, Octal, Hexadecimal - display the contents of the memory location
-- in the specified format
*/
      {
        int no_of_words = (sizeoftype (type)) / sizeof (WORD) ;
        UWORD *addr ;
        int i, int_rep ;
        float flt_res ;
        double dbl_res ;
        if (sizeoftype(type) == sizeof (float))
        {
          flt_res = fpop (eval) ;
          addr = (UWORD *) &flt_res ;
          int_rep = (int) flt_res ;
          /* dprintf(display, "%f", fpop(eval));*/
        }
        else
        {
          dbl_res = dpop (eval) ;
          addr = (UWORD *) &dbl_res ;
          int_rep = (int) dbl_res ;
          /* dprintf(display, "%lf", dpop(eval));*/
        }

        switch (format)
        {
          case Ascii :
            dputc(display, '\'');
            putcharconst(display, int_rep) ;
            dputc(display, '\'');
            break;

          case Binary :
            for (i = 0 ; i < no_of_words ; i ++)
            {
              int j ;
              if (i > 0) dprintf (display, " ") ;
              for (j = 31 ; j >= 0 ; j --)
                dprintf (display, "%d", (*addr >> j) & 0x1) ;
              addr ++ ;
            }
            break ;

          case Decimal :
            dprintf(display, "%d",  int_rep) ;  
            break ;

          case Error:
            {
              char msg[128];
              Fault(int_rep, msg, 128);
              dprintf(display, "%08x: \"%s\"", int_rep, msg);
            }
            break;

          case Hexadecimal :
            dprintf (display, "0x") ;
            for (i = 0 ; i < no_of_words ; i ++)
            {
              if (i > 0) dprintf (display, " ") ;
              dprintf (display, "%08x", *(addr ++)) ;
            }
            break ;

          case Octal :
            for (i = 0 ; i < no_of_words ; i ++)
            {
              if (i > 0) dprintf (display, " ") ;
              dprintf (display, "%#o", *(addr ++)) ;
            }
            break ;

          case Unsigned :
            dprintf(display, "%u",  int_rep) ;  
            break ;
 
          case Float :

          case Default :
            if (sizeoftype (type) == sizeof (float))
              dprintf(display, "%f", flt_res) ;
            else
              dprintf(display, "%lf", dbl_res) ;
            break ;
        }  
      return;
      }

      case TI_Function:
      return;

      case TI_Array:
      if (isstring(type) || format == STring)
      {
        char *s = spop(eval, type->array.size);
        
#ifdef OLDCODE
        putstr(debug, s);
#endif
/*
-- crf : 18/08/91 - Bug 697
-- Problem : printing strings (i.e. arrays of type char) does not work.
-- The problem is with regard to putstr() and the associated call to
-- peekmem(). This is fine for pointers, but not for arrays. 
-- Solution : very simple. Just require to display "s".
*/
        int i = 0 ;
        deol (display) ;
	switch (format)
	  {
	  case Decimal:
	    dprintf( display, "{ %d", *s );
	    for (i = 1; i < type->array.size; i++)
	      {
		dprintf( display, ", %d", s[i]);
	      }
	    dprintf( display, " }" );
	    
	    break;
	    
	  case Hexadecimal:
	    dprintf( display, "{ %#02x", *s );
	    for (i = 1; i < type->array.size; i++)
	      {
		dprintf( display, ", %#02x", s[i]);
	      }
	    dputc(display, '}');
	    break;
	    
	  case Octal:
	    dprintf( display, "{ %#03o", *s );
	    for (i = 1; i < type->array.size; i++)
	      {
		dprintf( display, ", %#03o,", s[i]);
	      }
	    dputc(display, '}');
	    break;
	    
	  case Unsigned:
	    dprintf( display, "{ %03d", *s );
	    for (i = 1; i < type->array.size; i++)
	      {
		dprintf( display, ", %03d,", s[i]);
	      }
	    dputc(display, '}');
	    break;
	    
	  case STring:
	  default:
	    dputc(display, '\"');
	    until (s [i] == '\0')
	      putcharconst (display, s [i++]) ;
	    dputc(display, '\"');
	    break;
	  }

        freemem(s);
      }
      else if (isShortArray( type ))
	{
	  unsigned short * s = short_pop( eval, type->array.size );
	  int		   i;


	  deol( display );

	  switch (format)
	    {
	    default:
	    case Decimal:
	      dprintf( display, "{ %05d", *(signed short *)s );
	      for (i = 1; i < type->array.size; i++)
		dprintf( display, ", %05d", ((signed short *)s)[i] );
	      break;
	      
	    case Hexadecimal:
	      dprintf( display, "{ %#04x", *s );
	      for (i = 1; i < type->array.size; i++)
		dprintf( display, ", %#04x", s[i] );
	      break;
	      
	    case Octal:
	      dprintf( display, "{ %#06o", *s );
	      for (i = 1; i < type->array.size; i++)
		dprintf( display, ", %#06o", s[i] );
	      break;
	      
	    case Unsigned:
	      dprintf( display, "{ %05u", *s );
	      for (i = 1; i < type->array.size; i++)
		dprintf( display, ", %05u", s[i] );
	      break;
	    }
	  
	  dprintf( display, " }" );
	  
	  freemem( s );
	  
	}
      else
      {
        int index = 0;
        int size = type->array.size;
        int i ;

        if (size == 0) size = 1;

        dprintf(display, "{");
/*
-- crf : 11/08/91 - neaten up display
-- e.g. use of "\n\r" (refer TI_Struct), deol, indent ...
*/
        deol (display) ;
        if (isaggregate(type->array.host)) 
        {
          dprintf(display, "\n\r");
          for (i = 0; i <= indent; i++) dprintf(display, "  ");
        }
        while (index++ < size)
        {
          putvalue(debug, type->array.host, format, indent, chase);
          unless (index == size)
          {
            if (isaggregate(type->array.host)) 
            {
              dprintf(display, ",\n\r");
              for (i = 0; i <= indent; i++) dprintf(display, "  ");
            }
            else dprintf(display, ", ");
          }
        }
        dprintf(display, "}");
      }
      return;

      case TI_Pointer:
      {
        void *p = ppop(eval);

        if (p == NULL) dprintf(display, "NULL");
        else unless (isvalidptr(p)) dprintf(display, "(ptr)%p (INVALID)", p);
        else
        {
/* 
-- crf : 18/07/91 - "declared but not used ...
          ENTRY *entry;
*/
          if (isstring(type) || format == STring) putstr(debug, (char *)p);
          else if (chase > 0)
          {
            chase--;
            type = hosttype(type);
            ppush(eval, p);
            load(eval, sizeoftype(type));
            continue;
          }
          else dprintf(display, "(ptr)%p", p);
        }
#ifdef NEWCODE
        unless ((entry = whichentry(eval->debug, p, entry->name)) == NULL)
        {
          int offset;

          genaddr(eval, entry);
          offset = p - ppop(eval);
          dprintf(display, " (&%s", entry->name);
          putlvalue(display, entry->type, offset);
          dprintf(display, ")");
        }
#endif
      }
      return;

      case TI_Struct:
      {
        MEMBER *member;
/* 
-- crf : 23/07/91 - Bug 543 (a)
-- Problem : watchpointing structures gives a staggered display - e.g.
-- 0) a = {
--           b = 1, etc
-- Solution : I am not sure why, but it is necessary to replace "\n" with
-- "\n\r" ("\n" by itself works fine for printing structures, but not for
-- watchpointing them (!))
-- I have also used "deol(...)" to neaten up the watchpoint display.
*/
        dprintf(display, "{");
        deol (display) ;
        dprintf(display, "\n\r");
        for (member = (MEMBER *)type->structure.memberlist.head; member != NULL;
             member = (MEMBER *)member->link.next)
        {
          int i; /* CR: hier ist irgendwo ein Fehler bei chase > Tiefe */

          for (i = 0; i <= indent; i++) dprintf(display, "  ");
          dprintf(display, "%s = ", member->name);
          dupeval(eval, member->offset, sizeoftype(member->type));
          putvalue(debug, member->type, format, indent + 1, chase);
          unless (member->link.next == NULL)
          {
/* crf : Bug 543 (a) - see above */
            dprintf(display, ",");
            deol (display) ;
            dprintf(display, "\n\r");
          }
        }
        dprintf(display, "}");
        drop(eval, sizeoftype(type));
      }
      return;

      case TI_Enum:
      {
        ENTRY *enumconst; /* CR: war mal ENUMCONST  */
        int val = pop(eval);

        if ((enumconst = findenumconst(type, val)) == NULL)
          dprintf(display, "%d", val);
        else
          dprintf(display, "%s", enumconst->name);
      }
      return;

      default:
      IOdebug( "TLA: Unknown type in putvalue %s", tokennames[(int)type->generic.id]);
      return;
    }
  }
}
