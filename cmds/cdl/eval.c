/**
*
* Title:  CDL Compiler - Evaluate Subscipt Expressions.
*
* Author: Andy England
*
* Date:   January 1989
*
*         (c) Copyright 1989 - 1992, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/
/* static char *rcsid = "$Header: /hsrc/cmds/cdl/RCS/eval.c,v 1.2 1992/06/11 11:41:15 nickc Exp $"; */

#include <setjmp.h>
#include "cdl.h"

#define NAME_MAX 20
#define STACK_MAX 100
#define BINDSTACK_MAX 100

PRIVATE evalstack[STACK_MAX];
PRIVATE int stackdepth = 0;

PRIVATE BINDING bindstack[BINDSTACK_MAX];
PRIVATE int binddepth = 0;

PRIVATE jmp_buf home;

PRIVATE char *evalbinary(char *, int);
PRIVATE char *evalunary(char *);

PRIVATE int popval(void)
{
  if (stackdepth == 0) bug("Evaluation stack empty");
  return evalstack[--stackdepth];
}

PRIVATE void pushval(int value)
{
  if (stackdepth == STACK_MAX)
  {
    error("Evaluation stack full, (expression to complicated)");
    longjmp(home, 1);
  }
  evalstack[stackdepth++] = value;
}

PUBLIC void bindname(char *name, int value)
{
  if (binddepth == BINDSTACK_MAX)
  {
    error("Binding stack full");
    longjmp(home, 1);
  }
  bindstack[binddepth].name = name;
  bindstack[binddepth++].value = value;
}

PUBLIC void unbind(void)
{
  if (binddepth == 0) bug("Binding stack empty");
  --binddepth;
}

PRIVATE BINDING *findbinding(char *name)
{
  int i;

  for (i = binddepth - 1; i >= 0; i--)
  {
    if (strequ(bindstack[i].name, name)) return &bindstack[i];
  }
  return NULL;
}

PUBLIC int bindnames(ARGV subnames, int *subvals)
{
  int i = 0;

  unless (subnames == NULL)
  {
    char *subname;

    until ((subname = subnames[i]) == NULL) bindname(subname, subvals[i++]);
  }
  return i;
}

PUBLIC void unbindnames(int n)
{
  while (n-- > 0) unbind();
}

PUBLIC int evalexpr(char *expr)
{
  if (setjmp(home) == 0)
  {
    stackdepth = 0;
    (void)evalbinary(expr, 0);
    return popval();
  }
  return 0;
}

PRIVATE char *evalbinary(char *expr, int pri)
{
  char c;
  int value;

  expr = evalunary(expr);
  until ((c = *expr) == '\0')
  {
    switch (c)
    {
      case '+':
      if (pri >= 1) return expr;
      expr = evalbinary(expr + 1, 1);
      pushval(popval() + popval());
      break;

      case '*':
      if (pri >= 2) return expr;
      expr = evalbinary(expr + 1, 2);
      pushval(popval() * popval());
      break;

      case '-':
      if (pri >= 1) return expr;
      expr = evalbinary(expr + 1, 1);
      value = popval();
      pushval(popval() - value);
      break;

      case '%':
      if (pri >= 2) return expr;
      expr = evalbinary(expr + 1, 2);
      {
        int val2 = popval();
        int val1 = popval();

        value = val1 % val2;
        if (val1 < 0) value += val2;
      }
      pushval(value);
      break;

      default:
      return expr;
    }
  }
  return expr;
}

PRIVATE char *evalunary(char *expr)
{
  int c;

  switch (c = *expr)
  {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    {
      int value = 0;

      while (isdigit(c))
      {
        value = value * 10 + (c - '0');
        c = *++expr;
      }
      pushval(value);
    }
    break;

    case '+':
    expr = evalbinary(expr + 1, 3);
    break;
    
    case '-':
    expr = evalbinary(expr + 1, 3);
    pushval(-popval());
    break;

    case '(':
    expr = evalbinary(expr + 1, 0);
    unless (*expr++ == ')')
    {
      error("')' expected");
      longjmp(home, 1);
    }
    break;

    default:
    if (isalpha(c))
    {
      BINDING *binding;
      char name[NAME_MAX + 1];
      int i = 0;

      while (isalnum(c))
      {
      	if (i == NAME_MAX)
      	{
      	  error("Subscript name too long, (limit is %d characters)", NAME_MAX);
      	  longjmp(home, 1);
      	}
      	name[i++] = c;
      	c = *++expr;
      }
      name[i] = '\0';
      if ((binding = findbinding(name)) == NULL)
      {
      	error("Subscript name `%s' not defined, assumed to be 0", name);
        pushval(0);
      }
      else pushval(binding->value);
    }
    else
    {
      error("Unexpected character `%c' in expression", c);
      longjmp(home, 1);
    }
    break;
  }
  return expr;
}
