/**
*
* Title:  Helios Debugger - C Semantics / Type Checker
*
* Author: Andy England
*
* Date:   April 1989
*
*         (c) Copyright 1989 - 1993, Perihelion Software Limited.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/sem.c,v 1.7 1993/03/17 17:42:23 nickc Exp $";
#endif

#include "tla.h"

/**
*
* expr = mkexpr?();
*
* Routines which build the expression tree.
*
**/
PRIVATE EXPR *mkexpr1(TOKEN op, TYPE *type, EXPR *expr1)
{
  EXPR *expr = NEW(EXPR);

  debugf("mkexpr1(%s)", tokennames[(int)op]);
  
  expr->generic.op    = op;
  expr->generic.type  = type;
  expr->generic.expr1 = expr1;
  expr->generic.expr2 = NULL;
  expr->generic.expr3 = NULL;

  return expr;
}

PRIVATE EXPR *mkexpr2(TOKEN op, TYPE *type, EXPR *expr1, EXPR *expr2)
{
  EXPR *expr = NEW(EXPR);

  debugf("mkexpr2(%s)", tokennames[(int)op]);
  expr->generic.op = op;
  expr->generic.type = type;
  expr->generic.expr1 = expr1;
  expr->generic.expr2 = expr2;
  expr->generic.expr3 = NULL;
  return expr;
}

PRIVATE EXPR *mkexpr3(TOKEN op, TYPE *type, EXPR *expr1, EXPR *expr2, EXPR *expr3)
{
  EXPR *expr = NEW(EXPR);

  debugf("mkexpr3(%s)", tokennames[(int)op]);
  expr->generic.op = op;
  expr->generic.type = type;
  expr->generic.expr1 = expr1;
  expr->generic.expr2 = expr2;
  expr->generic.expr3 = expr3;
  return expr;
}

/**
*
* type = typeofexpr(expr);
*
* Obtain the type of an expression.
*
**/
PUBLIC TYPE *typeofexpr(EXPR *expr)
{
  debugf("typeofexpr()");
  
  if (expr == NULL)
    {
      debugf("NULL expr in typeofexpr");
      return NULL;
    }
  
  return skipreuse(expr->generic.type);
}

/**
*
* size = sizeoftype(type);
*
* Calculate the size (in bytes) of a particular data type.
*
**/
PUBLIC int sizeoftype(TYPE *type)
{
  int size = 0;

  forever
  {
    type = skiptypedef(type);
    switch (type->generic.id)
    {
      case TI_Pointer:
      return sizeof(void *);

      case TI_Tag:
      if (type->typename.type == NULL)
      {
      	/* ACE: Where to put message ? */
        debugf("sizeof undefined tag");
        return 0;
      }
      type = type->typename.type;
      continue;

      case TI_Struct: /* ACE: do we need to align individual members ? */
      {
        MEMBER *member;

        for (member = (MEMBER *)type->structure.memberlist.head;
             member != NULL; member = (MEMBER *)member->link.next)
        {
          int endofmember = member->offset + sizeoftype(member->type);

          if (endofmember > size) size = endofmember;
        }
      }
      return ALIGN(size);

      case TI_Array:
      return type->array.size == 0 ? sizeoftype(type->array.host) :
             sizeoftype(type->array.host) * type->array.size;

      case TI_Function:
      return sizeof(void *);

      case TI_Enum:
      return sizeof(int);

      case TI_Integral: case TI_Float:
      return type->basetype.size;

      default:
      debugf("Unknown type in sizeoftype");
      return 0;
    }
  }
}

PUBLIC int sizeofexpr(EXPR *expr)
{
  if (expr->generic.op == T_List)
    return sizeofexpr(expr->generic.expr1) + sizeofexpr(expr->generic.expr2);
  return sizeoftype(typeofexpr(expr));
}

PUBLIC int strideoftype(TYPE *type)
{
  return sizeoftype(hosttype(type));
}

PUBLIC int strideofexpr(EXPR *expr)
{
  return strideoftype(typeofexpr(expr));
}

PUBLIC BOOL isunsigned(TYPE *type)
{
  type = skiptypedef(type);
  unless (type->generic.id == TI_Integral) return FALSE;
  return type->basetype.issigned == FALSE;
}

PUBLIC BOOL isarray(TYPE *type)
{
  if (skiptypedef(type)->generic.id == TI_Array)
     debugf("TI_Array recognised");
  return skiptypedef(type)->generic.id == TI_Array;
}

PUBLIC BOOL isstructure(TYPE *type)
{
  type = skiptypedef(type);
  return type->generic.id == TI_Struct OR type->generic.id == TI_Tag;
}

PUBLIC BOOL isaggregate(TYPE *type)
{
  type = skiptypedef(type);
  return type->generic.id == TI_Array OR type->generic.id == TI_Struct OR
         type->generic.id == TI_Tag;
}

PUBLIC BOOL ispointer(TYPE *type)
{
  return skiptypedef(type)->generic.id == TI_Pointer;
}

PUBLIC BOOL isfunction(TYPE *type)
{
  return skiptypedef(type)->generic.id == TI_Function;
}

PUBLIC BOOL isintegral(TYPE *type)
{
  return skiptypedef(type)->generic.id == TI_Integral;
}

PUBLIC BOOL isfloat(TYPE *type)
{
  return skiptypedef(type)->generic.id == TI_Float;
}

PUBLIC BOOL isstring(TYPE *type)
{
  type = skiptypedef(type);
  unless (type->generic.id == TI_Pointer OR
          type->generic.id == TI_Array)
    {
      return FALSE;
    }
  type = hosttype(type);
  while (type->generic.id == TI_Typedef)
  {
    ENTRY *entry = GetTypeEntry(type);

    if (strequ("char", entry->name)         ||
	strequ("signed char", entry->name)  ||
	strequ("unsigned char", entry->name) )
      return TRUE;
    
    type = skipreuse(entry->type);
  }
  
  return FALSE;
}

PUBLIC BOOL
isShortArray( TYPE * type )
{
  type = skiptypedef( type );
  
  unless (type->generic.id == TI_Pointer OR
          type->generic.id == TI_Array)
    {
      return FALSE;
    }
  
  type = hosttype( type );
  
  while (type->generic.id == TI_Typedef)
    {
      ENTRY *	entry = GetTypeEntry(type);

      
      if (strequ(          "short", entry->name) ||
	  strequ(   "signed short", entry->name) ||
	  strequ( "unsigned short", entry->name) )
	return TRUE;
      
      type = skipreuse( entry->type );
    }
  
  return FALSE;

} /* isShortArray */


PRIVATE BOOL isscalar(TYPE *type)
{
  type = skiptypedef(type);
  return type->generic.id == TI_Integral OR type->generic.id == TI_Float OR
         type->generic.id == TI_Pointer;
}

PRIVATE BOOL isarith(TYPE *type)
{
  type = skiptypedef(type);
  return type->generic.id == TI_Integral OR type->generic.id == TI_Float;
}

PRIVATE BOOL islvalue(EXPR *expr)
{
  /* ACE: more here */
  switch (expr->generic.op)
  {
    case T_Identifier:
    return !isfunction(typeofexpr(expr));

    case T_String: case T_Subscript: case T_Dot: case T_Indirect:
    return TRUE;

    case T_LParen: case T_Arrow:
    return islvalue(expr->generic.expr1);

    case T_Cast:
    return islvalue(expr->cast.expr);

    default:
    return FALSE;
  }
}

PRIVATE BOOL ismodifiable(EXPR *expr)
{
  TYPE *type = typeofexpr(expr);/* CR: hope it keeps the compiler quite (ueberfluessig)*/
  unless (islvalue(expr)) return FALSE;
  if (isarray(type)) return FALSE; /* ACE: more here CR: type was expr but caused trouble*/
  return TRUE;
}

PRIVATE BOOL isvoid(TYPE *type)
{
  return sizeoftype(type) == 0;
}

PRIVATE BOOL isvoidptr(TYPE *type)
{
  return ispointer(type) AND isvoid(hosttype(type));
}

PRIVATE BOOL isnull(EXPR *expr)
{
  return expr->generic.op == T_Constant AND isintegral(typeofexpr(expr)) AND
         expr->constant.value.integral == 0;
}

PRIVATE EXPR *coerceunary(EXPR *expr)
{
  TYPE *type = typeofexpr(expr);

  if (type == NULL) return NULL;
  
  if (isarray(type))
  {
#ifdef OLDCODE
    return mkexpr1(T_Cast, newpointer(reusetype(hosttype(type))), expr);
#else /* ACE: Type will not be freed */
    return mkexpr1(T_Address, newpointer(reusetype(hosttype(type))), expr);
#endif
  }
  if (isfunction(type))
  {
#ifdef OLDCODE
    return mkexpr1(T_Cast, newpointer(reusetype(type)), expr);
#else /* ACE: Type will not be freed */
    return mkexpr1(T_Address, newpointer(reusetype(type)), expr);
#endif
  }
  return expr;
}

PRIVATE TYPE *arithconversions(EVAL *eval, TYPE *type1, TYPE *type2)
{
  int sizeoflong = sizeoftype(findtype(eval->debug->table, "long")->type);
  int sizeofint = sizeoftype(findtype(eval->debug->table, "int")->type);
  int size1 = sizeoftype(type1);
  int size2 = sizeoftype(type2);

  if (isfloat(type1) AND isfloat(type2))
    return (size1 == size2) ? NULL: (size1 > size2) ? type1 : type2;
  if (isfloat(type1)) return type1;
  if (isfloat(type2)) return type2;
  if (size1 == sizeoflong AND size2 == sizeoflong)
  {
    if (isunsigned(type1) AND isunsigned(type2)) return NULL;
    if (isunsigned(type1)) return type1;
    if (isunsigned(type2)) return type2;
    return NULL;
  }
  if (size1 == sizeoflong) return type1;
  if (size2 == sizeoflong) return type2;
  if (size1 == sizeofint AND size2 == sizeofint)
  {
    if (isunsigned(type1) AND isunsigned(type2)) return NULL;
    if (isunsigned(type1)) return type1;
    if (isunsigned(type2)) return type2;
    return NULL;
  }
  /* ACE: Integral promotions */
  type1 = newtypedef(findtype(eval->debug->table, "int"));
  return type1;
}

PRIVATE BOOL equivtypes(TYPE *type1, TYPE *type2)
{
  type1 = skipreuse(type1);
  type2 = skipreuse(type2);
  unless (type1->generic.id == type2->generic.id) return FALSE;
  switch (type1->generic.id)
  {
    case TI_Tag:
    case TI_Typedef: /* ACE: this is not enough, matches all integral types */
    return type1 == type2;

    case TI_Pointer:
    return equivtypes(type1->pointer.host, type2->pointer.host);

    case TI_Array:
    unless (type1->array.size == 0 OR type2->array.size == 0 OR
            type1->array.size == type2->array.size) return FALSE;
    return equivtypes(type1->array.host, type2->array.host);
    
    case TI_Struct: /* ACE: I think it will suffice if the pointers are identical */
    return TRUE;

    case TI_Function:
    return TRUE;

    default:
    debugf("Unknown type in equivtypes");
    return FALSE;
  }
}

PUBLIC void recover(EVAL *eval)
{
  /* ACE: Must free all memory use by evaluator in here */
  cmdjmp(eval->debug, ErrorLevel);
}

/**
*
* semerr(eval, format, ...);
*
* Display an evaluator error and recover.
*
**/
PUBLIC void semerr(EVAL *eval, char *format, ...)
{
  DISPLAY *display = eval->debug->display;
  bool test = FALSE;
  va_list args;

  va_start(args, format);
  if (TestSemaphore(&display->lock) > 0)/* CR: to avoid a crash */
  {
    dlock(display);
    if (TestSemaphore(&display->lock) > 0)
      IOdebug("TLA: Not locked (sem err, lock = %d)", TestSemaphore( &display->lock ));
    test = TRUE;
  }
  dcursor(display, display->height - 1, 0);
  deol(display);
  vfprintf(display->fileout, format, args);
  if (test) dunlock(display);
  va_end(args);
  recover(eval);
}

PRIVATE void typeerr(EVAL *eval, TOKEN op)
{
  semerr(eval, "Illegal types for %s", tokennames[(int)op]);
}

PUBLIC EXPR *mkidentifier(ENTRY *entry)
{
  EXPR *expr = NEW(EXPR);

  debugf("mkidentifier(%s)", entry->name);
  
  expr->identifier.token = T_Identifier;
  expr->identifier.type  = entry->type;
  expr->identifier.entry = entry;

  return expr;
}

PUBLIC EXPR *mkintconst(EVAL *eval, int value)
{
  EXPR *expr = NEW(EXPR);

  debugf("mkintconst(%d)", value);
  
  expr->constant.token = T_Constant;
  expr->constant.type  = newtypedef(findtype(eval->debug->table, "int"));
  expr->constant.value.integral = value;
  return expr;
}

PUBLIC EXPR *mkfloatconst(EVAL *eval, double value)
{
  EXPR *expr = NEW(EXPR);

  debugf("mkfloatconst(%lf)", value);
  expr->constant.token           = T_Constant;
  expr->constant.type            = newtypedef(findtype(eval->debug->table, "float"));
  expr->constant.value.floating4 = (float)value;
  return expr;
}

PUBLIC EXPR *mkstring(EVAL *eval, char *value)
{
  EXPR *expr = NEW(EXPR);
  TYPE *type;

  debugf("mkstring(%s)", value);
  expr->string.token = T_String;
  type = newtypedef(findtype(eval->debug->table, "char"));
  expr->string.type = newarray(type, 0, 0);
  expr->string.value = strdup(value);
  return expr;
}

PUBLIC EXPR *mkcond(EVAL *eval, EXPR *expr1, EXPR *expr2, EXPR *expr3)
{
  TYPE *type  = NULL;
  TYPE *type1 = typeofexpr(expr1);
  TYPE *type2 = typeofexpr(expr2);
  TYPE *type3 = typeofexpr(expr3);

  debugf("mkcond()");
  if (isscalar(type1))
  {
    if (isarith(type2) AND isarith(type3))
    {
      if ((type = arithconversions(eval, type2, type3)) == NULL) type = type2;
      else
      {
      	unless (type == type2) expr2 = mkcast(eval, reusetype(type), expr2);
      	unless (type == type3) expr3 = mkcast(eval, reusetype(type), expr3);
      }
    }
#ifdef NEWCODE
    else if (samestruct(type1, type2) OR samepointer(type1, type2) OR
             (isvoid(type1) AND isvoid(type2)) type = type1
#endif
    else if (ispointer(type2) AND isnull(expr3)) type = type2;
    else if (isnull(expr2) AND ispointer(type3)) type = type3;
    else if (!isvoidptr(type2) AND isvoidptr(type3)) type = type2;
    else if (isvoidptr(type2) AND !isvoidptr(type3)) type = type3;
    else semerr(eval, "Illegal types for conditional");
    return mkexpr3(T_Conditional, type, expr1, expr2, expr3);
  }
  semerr(eval, "First operand of conditional must be a scalar");

    return NULL;
    
}

PUBLIC EXPR *mkcast(EVAL *eval, TYPE *type, EXPR *expr)
{
  TYPE *oldtype = typeofexpr(expr);

  debugf("mkcast()");
  if (isvoid(type) AND isvoid(oldtype))
    semerr(eval, "Cannot cast void to void");
  else
  {
#ifdef ANSI
    unless (isscalar(type) AND isscalar(oldtype)) semerr(eval, "Illegal cast");
#else
    unless ((isscalar(type) AND isscalar(oldtype)) OR
            sizeoftype(type) == sizeoftype(oldtype))
      semerr(eval, "Illegal cast");
#endif
  }
  return mkexpr1(T_Cast, type, expr);
}

#ifdef OLDCODE
PUBLIC EXPR *mkexprlist(EVAL *eval, EXPR *expr1, EXPR *expr2)
#endif
/*
-- crf : 18/08/91 - "eval" declared, not used ...
*/
PUBLIC EXPR *mkexprlist(EXPR *expr1, EXPR *expr2)
{
  debugf("mkexprlist()");
  return mkexpr2(T_List, NULL, coerceunary(expr1), coerceunary(expr2));
}

PUBLIC EXPR *mkfieldref(EVAL *eval, TOKEN op, EXPR *expr, char *name)
{
  debugf("mkfieldref(%s)", tokennames[(int)op]);
  if (op == T_Dot)
  {
    TYPE *type = typeofexpr(expr = coerceunary(expr));

    if (isstructure(type))
    {
      ENTRY *member; /* CR: was MEMBER but caused trouble */
      
#ifdef OLDCODE
      if ((member = findmember(skiptypedef(type), name)) == NULL)
#else
      type = skiptypedef(type);
      if (type->generic.id == TI_Tag)
      {
        if (type->typename.type == NULL)
          semerr(eval, "structure tag %s undefined", GetTypeEntry(type)->name);
        type = type->typename.type;
      }
      if ((member = findmember(type, name)) == NULL)
#endif
      {
        freemem(name);
        semerr(eval, "%s is not a member", name);
      }
      freemem(name);
      return mkexpr2(T_Dot, member->type, expr, mkidentifier(member));
    }
#ifdef NEWCODE
#ifndef ANSI
    if (functiontype(type))
    {
    }
#endif
#endif
    typeerr(eval, op);
  }
  /* ACE: should check whether it is a pointer to a struct */
  return mkfieldref(eval, T_Dot, mkunary(eval, T_Indirect, expr), name);
}

/**
*
* expr = mkbinary(eval, op, expr1, expr2);
*
* Build a binary expression node.
*
**/
PUBLIC EXPR *mkbinary(EVAL *eval, TOKEN op, EXPR *expr1, EXPR *expr2)
{
  TYPE *type1, *type2, *type;

  debugf("mkbinary(%s)", tokennames[(int)op]);
  type1 = typeofexpr(expr1 = coerceunary(expr1));
  type2 = typeofexpr(expr2 = coerceunary(expr2));
  type  = NULL;
  
  switch (op)
  {
    case T_Comma:
    return mkexpr2(T_Comma, type, expr1, expr2);

    case T_Subscript:
    unless ((ispointer(type1) AND isintegral(type2)) OR
        (isintegral(type1) AND ispointer(type2))) typeerr(eval, op);
    return mkunary(eval, T_Indirect, mkbinary(eval, T_Plus, expr1, expr2));

    case T_Call: /* ACE: more here */
    unless (ispointer(type1) AND isfunction(type = hosttype(type1))) typeerr(eval, op);
    return mkexpr2(T_Call, hosttype(type), expr1, expr2);

    case T_Plus:
    if (ispointer(type1) AND isintegral(type2))
    {
      debugf("pointer + integral");
      return mkexpr2(T_Plus, type1, expr1,
        mkbinary(eval, T_Times, expr2, mkintconst(eval, strideoftype(type1))));
    }
    if (isintegral(type1) AND ispointer(type2))
    {
      return mkexpr2(T_Plus, type2, expr2,
        mkbinary(eval, T_Times, expr1, mkintconst(eval, strideoftype(type2))));
    }
    unless (isarith(type1) AND isarith(type2)) typeerr(eval, T_Plus);
    break;

    case T_Minus:
    if (ispointer(type1) AND ispointer(type2))
    {
      if (equivtypes(type1, type2))
      {
        type = newtypedef(findtype(eval->debug->table, "int"));
        return mkbinary(eval, T_Divide, mkexpr2(T_Minus, type1, expr1, expr2),
                                   mkintconst(eval, strideoftype(type1)));
      }
      semerr(eval, "different pointer types to %s", tokennames[(int)op]);
    }
    if (ispointer(type1) AND isintegral(type2))
    {
      return mkexpr2(T_Minus, type1, expr1,
        mkbinary(eval, T_Times, expr2, mkintconst(eval, strideoftype(type1))));
    }
    if (isintegral(type1) AND ispointer(type2))
    {
      return mkexpr2(T_Minus, type2, expr2,
        mkbinary(eval, T_Times, expr1, mkintconst(eval, strideoftype(type2))));
    }
    unless (isarith(type1) AND isarith(type2)) typeerr(eval, T_Minus);
    break;

    case T_Times: case T_Divide:
    if (isarith(type1) AND isarith(type2)) break;
    typeerr(eval, op);

    case T_EQ: case T_NE:
    if (isarith(type1) AND isarith(type2)) break;
    if (ispointer(type1) AND ispointer(type2)) type = type1;
    else if (!isvoidptr(type1) AND isvoidptr(type2)) type = type1;
    else if (isvoidptr(type1) AND !isvoidptr(type2)) type = type2;
    else if (ispointer(type1) AND isnull(expr2)) type = type1;
    else if (isnull(expr1) AND ispointer(type2)) type = type2;
    else typeerr(eval, op);
    return mkexpr2(op, type, expr1, expr2);

    case T_LT: case T_GT: case T_LE: case T_GE:
    if (isarith(type1) AND isarith(type2)) break;
    if (ispointer(type1) AND ispointer(type2)) return mkexpr2(op, type1, expr1, expr2);
    typeerr(eval, op);

    case T_Remainder: case T_LShift: case T_RShift:
    case T_BitAnd: case T_BitXOr: case T_BitOr:
    unless (isintegral(type1) AND isintegral(type2)) typeerr(eval, op);
    break;

    case T_LogAnd: case T_LogOr: /* ACE: What about pointers ? */
    unless (isscalar(type1) AND isscalar(type2)) typeerr(eval, op);
    break;

    case T_Assign: case T_TimesEq: case T_DivideEq:
    case T_RemainderEq: case T_PlusEq: case T_MinusEq:
    case T_LShiftEq: case T_RShiftEq: case T_BitAndEq:
    case T_BitXOrEq: case T_BitOrEq:
    unless (ismodifiable(expr1)) typeerr(eval, op);
    /* ACE: more here */
    return mkexpr2(op, type1, expr1, expr2);
  }
  if ((type = arithconversions(eval, type1, type2)) == NULL) type = type1;
  else
  {
    unless (type == type1) expr1 = mkcast(eval, reusetype(type), expr1);
    unless (type == type2) expr2 = mkcast(eval, reusetype(type), expr2);
  }
  return mkexpr2(op, type, expr1, expr2);
}

/**
*
* expr = mkunary(eval, op, expr);
*
* Build a unary expression node.
*
**/
PUBLIC EXPR *mkunary(EVAL *eval, TOKEN op, EXPR *expr)
{
  TYPE *type = typeofexpr((op == T_Address) ? expr : (expr = coerceunary(expr)));


  if (expr == NULL)
    return NULL;
  
  debugf("mkunary(%s)", tokennames[(int)op]);
  switch (op)
  {
    case T_PostInc:
    case T_PostDec:
    case T_PlusPlus:
    case T_MinusMinus: /* ACE: more here */
    unless (isscalar(type) AND ismodifiable(expr)) typeerr(eval, op);
    break;

    case T_Address:
    unless (isfunction(type) OR islvalue(expr)) typeerr(eval, op);
    /* ACE: Free type */
    type = newpointer(reusetype(typeofexpr(expr)));
    return mkexpr1(op, type, expr);

    case T_Indirect:
    unless (ispointer(type))
      typeerr(eval, op);
    type = hosttype(type);
    return mkexpr1(op, type, expr);

    case T_UPlus:
    unless (isscalar(type)) typeerr(eval, op);
    return expr;

    case T_UMinus:
    unless (isarith(type)) typeerr(eval, op);
    break;
    
    case T_BitNot:
    unless (isintegral(type)) typeerr(eval, op);
    break;

    case T_LogNot:
    unless (isscalar(type)) typeerr(eval, op);
    break;
  }
  return mkexpr1(op, type, expr);
}

/**
*
* freeexpr(expr);
*
* Free an expression.
*
**/
PUBLIC void freeexpr(EXPR *expr)
{
  if (expr == NULL) return;
  switch (expr->generic.op)
  {
    case T_Identifier:
    case T_Constant:
    break;

    case T_String:
    freemem(expr->string.value);
    break;

    case T_Cast:
    freetype(expr->cast.type);
    freeexpr(expr->cast.expr);
    break;

    default:
    freeexpr(expr->generic.expr1);
    freeexpr(expr->generic.expr2);
    freeexpr(expr->generic.expr3);
    break;
  }
  freemem(expr);
}

/**
*
* tagerr(eval, format, ...);
*
* Display an evaluator error in a structure.
* CR: this is a special funktion for structure not found
**/
PUBLIC void tagerr(EVAL *eval, char *format, ...)
{
  DISPLAY *display = eval->debug->display;
  bool test = FALSE;
  va_list args;

  va_start(args, format);
  if (TestSemaphore(&display->lock) > 0)/* CR: to avoid a crash */
  {
    dlock(display);
    if (TestSemaphore(&display->lock) > 0)
      IOdebug("TLA: Not locked, tag err, lock = %d", TestSemaphore( &display->lock ));
    test = TRUE;
  }
  /* dcursor(display, display->height - 1, 0); CR: causes a new line */
  /* deol(display); CR: there is naut to delete */
  vfprintf(display->fileout, format, args);
  if (test) dunlock(display);
  va_end(args);
  /* recover(eval); CR: Im not sure, but could cause a fault*/
}

