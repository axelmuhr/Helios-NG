/**
*
* Title:  Helios Debugger - C Expression Parser.
*
* Author: Andy England
*
* Date:   September 1988
*
*         (c) Copyright 1988 - 1993, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/expr.c,v 1.7 1993/07/12 15:49:28 nickc Exp $";
#endif

#include "tla.h"

PRIVATE void rdfloat(EVAL *);
PRIVATE void nextch(EVAL *);
PRIVATE void nexttoken(EVAL *);
PRIVATE void checkfor(EVAL *, TOKEN);
PRIVATE BOOL punctuation(EVAL *, TOKEN);
PRIVATE TYPE *rdtypedefname(EVAL *);
PRIVATE TYPE *rdtypespeclist(EVAL *);
PRIVATE TYPE *rddecl(EVAL *, char *, TYPE *);
PRIVATE TYPE *rdtypename(EVAL *);
PRIVATE EXPR *rdexpr(EVAL *, int);
PRIVATE EXPR *rdunary(EVAL *);
PRIVATE EXPR *rdexprlist(EVAL *);
PRIVATE EXPR *rdpostfix(EVAL *);
PRIVATE EXPR *rdprimary(EVAL *);
PRIVATE void synerr(EVAL *, char *, ...);

PUBLIC char *tokennames[99] =
{
  "auto",
  "break",
  "case",
  "char",
  "const",
  "continue",
  "default",
  "do",
  "double",
  "else",
  "enum",
  "extern",
  "float",
  "for",
  "goto",
  "if",
  "int",
  "long",
  "register",
  "return",
  "short",
  "signed",
  "sizeof",
  "static",
  "struct",
  "switch",
  "typedef",
  "union",
  "unsigned",
  "void",
  "volatile",
  "while",
  "<identifier>",
  "<constant>",
  "<string>",
  "[",
  "]",
  "(",
  ")",
  ".",
  "->",
  "++",
  "--",
  "&",
  "*",
  "+",
  "-",
  "~",
  "!",
  "/",
  "%",
  "<<",
  ">>",
  "<",
  ">",
  "<=",
  ">=",
  "==",
  "!=",
  "^",
  "|",
  "&&",
  "||",
  "?",
  "=",
  "*=",
  "/=",
  "%=",
  "+=",
  "-=",
  "<<=",
  ">>=",
  "&=",
  "^=",
  "|=",
  ",",
  "{",
  "}",
  ":",
  ";",
  "...",

  "<subscript>",
  "<call>",
  "<function>",
  "<pointer>",
  "<array>",
  "<unary plus>",
  "<unary minus>",
  "<address>",
  "<indirect>",
  "<post increment>",
  "<post decrement>",
  "<conversion>",
  "<cast>",
  "<error>",
  "<member>",
  "<parameter>",
  "<structure tag>",
  "<eof>"
};

PUBLIC BOOL evalcond(EVAL *eval, char *exprstr, BLOCK *block)
{
  EXPR *expr;

  eval->block = block;
  eval->chptr = exprstr;
  nextch(eval);
  nexttoken(eval);
  expr = rdexpr(eval, UPTOCOMMA);
  genexpr(eval, expr);
  freeexpr(expr);
  return pop(eval);
}

PUBLIC EXPR *parseexpr(EVAL *eval, char *expr, BLOCK *block)
{
  eval->block = block;
  eval->chptr = expr;

  nextch(eval);
  nexttoken(eval);
  
  return rdexpr(eval, UPTOCOMMA);
}

PUBLIC void *evaladdr(EVAL *eval, EXPR *expr)
{
  genaddr(eval, expr);
  return ppop(eval);
}

PUBLIC void putexpr(EXPR *expr, FILE *file)
{
  if (expr == NULL) return;
  switch (expr->generic.op)
  {
    case T_Identifier:
#ifndef OLDCODE
    fprintf(file, "[ <id> ");
    fprintf(file, "(");
    puttype(expr->identifier.type, file);
    fprintf(file, ") ");
#endif
    fprintf(file, "%s ", expr->identifier.entry->name);
#ifndef OLDCODE
    fprintf(file, "] ");
#endif
    break;

    case T_Constant:
#ifndef OLDCODE
    fprintf(file, "[ <const> ");
    fprintf(file, "(");
    puttype(expr->constant.type, file);
    fprintf(file, ") ");
#endif
    if (isintegral(typeofexpr(expr)))
      fprintf(file, "%d ", expr->constant.value.integral);
    else if (isfloat(typeofexpr(expr)))
    {
      if (sizeoftype(typeofexpr(expr)) == 4)
        fprintf(file, "%f ", expr->constant.value.floating4);
      else fprintf(file, "%lf ", expr->constant.value.floating8);
    }
#ifndef OLDCODE
    fprintf(file, "] ");
#endif
    break;

    case T_String:
#ifndef OLDCODE
    fprintf(file, "[ <string> ");
    fprintf(file, "(");
    puttype(expr->string.type, file);
    fprintf(file, ") ");
#endif
    fprintf(file, "\"%s\" ", expr->string.value);
#ifndef OLDCODE
    fprintf(file, "] ");
#endif
    break;

    case T_Cast:
#ifndef OLDCODE
    fprintf(file, "[ <cast> ");
#endif
    fprintf(file, "(");
    puttype(expr->cast.type, file);
    fprintf(file, ") ");
    putexpr(expr->cast.expr, file);
#ifdef OLDCODE
    fprintf(file, "%s ", tokennames[(int)expr->cast.op]);
#else
    fprintf(file, "] ");
#endif
    break;

    default:
#ifndef OLDCODE
    fprintf(file, "[ %s ", tokennames[(int)expr->generic.op]);
    fprintf(file, "(");
    puttype(expr->generic.type, file);
    fprintf(file, ") ");
#endif
    putexpr(expr->generic.expr1, file);
    putexpr(expr->generic.expr2, file);
    putexpr(expr->generic.expr3, file);
#ifdef OLDCODE
    fprintf(file, "%s ", tokennames[(int)expr->generic.op]);
#else
    fprintf(file, "] ");
#endif
    break;
  }
}

PRIVATE void nextch(EVAL *eval)
{
  if ((eval->ch = *eval->chptr++) == '\0') eval->ch = EOF;
}

PRIVATE ULONG getdigit(ULONG c)
{
  return isdigit(c) ? c - '0' :
         islower(c) ? c - 'a' + 10 :
         isupper(c) ? c - 'A' + 10 :
         (ULONG)16;
}
  
PRIVATE void rdnumber(EVAL *eval, ULONG radix)
{
  ULONG number = 0;
  ULONG digit;

  eval->tokenindex = 0;
  forever
  {
    eval->tokenbuffer[eval->tokenindex++] = eval->ch;
    if ((digit = getdigit(eval->ch)) >= radix) break;
    number = number * radix + digit;
    nextch(eval);
  }
  if ((eval->ch == '.' OR eval->ch == 'e' OR eval->ch == 'E') AND
      (radix == OCTAL OR radix == DECIMAL))
  {
    rdfloat(eval);
    return;
  }
  eval->token = T_Constant;
  eval->tokenvalue.expr = mkintconst(eval, (int)number);
}

PRIVATE void rdcomment(EVAL *eval)
{
  forever
  {
    nextch(eval);
    if (eval->ch == '*')
    {
      do nextch(eval); while (eval->ch == '*');
      if (eval->ch == '/')
      {
        nextch(eval);
        return;
      }
    }
    if (eval->ch == EOF)
    {
      synerr(eval, "End of file encountered inside a comment, assumed closed");
      return;
    }
  }
}

PRIVATE void nextstrch(EVAL *eval)
{
  forever
  {
    nextch(eval);
    if (eval->ch == '\n')
    {
      synerr(eval, "Unescaped new line encountered inside a string, ignored");
      continue;
    }
    if (eval->ch == '\\')
    {
      nextch(eval);
      switch (eval->ch)
      {
        case 'a':
        eval->ch = '\a';
        return;

        case 'b':
        eval->ch = '\b';
        return;

        case 'f': 
        eval->ch = '\f';
        return;

        case 'n':
        eval->ch = '\n';
        return;

        case 'r':
        eval->ch = '\r';
        return;

        case 't':
        eval->ch = '\t';
        return;

        case 'v':
        eval->ch = '\v';
        return;

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        {
          ULONG digit;
          ULONG value = 0;
          int count;

          for (count = 0; count < 3; count++)
          {
            if ((digit = getdigit(eval->ch)) >= 8) break;
            value = value * 8 + digit;
            nextch(eval);
          }
          eval->ch = (char)value;
        }
        return;

        case 'x':
        nextch(eval);
        {
          ULONG digit;
          ULONG value = 0;
          int count;

          for (count = 0; count < 3; count++)
          {
            if ((digit = getdigit(eval->ch)) >= 16) break;
            value = value * 16 + digit;
            nextch(eval);
          }
          eval->ch = (char)value;
        }
        return;

        case '\n':
        continue;

        case '\'':
        case '"':
        case '?':
        case '\\':
        return;

        default:
        synerr(eval, "Illegal string escape '\\%c', treated as %c", eval->ch, eval->ch);
        return;
      }
    }
    return;
  }
}

PRIVATE void rdname(EVAL *eval)
{
  TOKEN token;
  int index = 0;

  while (isalnum(eval->ch) OR eval->ch == '_')
  {
    if (index < NAME_MAX) eval->tokenbuffer[index++] = eval->ch;
    nextch(eval);
  }
  eval->tokenbuffer[index] = '\0';
  for (token = T_Auto; token <= T_While; token = (TOKEN)(token + 1))
  {
    if (strequ(eval->tokenbuffer, tokennames[(int)token]))
    {
      eval->token = token;
      return;
    }
  }
  debugf("findvar(%s)", eval->tokenbuffer);
  
  eval->tokenvalue.entry = findvar(eval->debug->table, eval->block, eval->tokenbuffer);
  eval->token = T_Identifier;
}

PRIVATE void rdstring(EVAL *eval)
{
  int index = 0;

  nextstrch(eval);
  until (eval->ch == '"')
  {
    eval->tokenbuffer[index++] = eval->ch;
    nextstrch(eval);
  }
  eval->tokenbuffer[index] = '\0';
  eval->token = T_String;
  eval->tokenvalue.expr = mkstring(eval, eval->tokenbuffer);
  nextch(eval);
}

PRIVATE void rdch(EVAL *eval)
{
  int c = 0;

  nextstrch(eval);
  until (eval->ch == '\'')
  {
    c = (c << 8) + eval->ch;
    nextstrch(eval);
  }
  eval->token = T_Constant;
  eval->tokenvalue.expr = mkintconst(eval, c);
  unless (eval->ch == '\'') synerr(eval, "Quote expected");
  nextch(eval);
}

PRIVATE void rdfloat(EVAL *eval)
{
  double mantissa = 0.0;
  LONG exponent = 0;
  int i = 0;

  forever
  {
    int	digit;
    
    if (i < eval->tokenindex) eval->ch = eval->tokenbuffer[i++];
    else nextch(eval);
    unless (isdigit(eval->ch)) break;
    digit = eval->ch - '0';
    mantissa = mantissa * 10.0 + digit;    
  }
  if (eval->ch == '.')
  {
    forever
    {
      int	digit;
      
      if (i < eval->tokenindex) eval->ch = eval->tokenbuffer[i++];
      else nextch(eval);
      unless (isdigit(eval->ch)) break;
      exponent--;
      digit = eval->ch - '0';
      mantissa = mantissa * 10.0 + digit;
    }
  }
  if (eval->ch == 'E' OR eval->ch == 'e')
  {
    BOOL negative = FALSE;
    ULONG number = 0;

    nextch(eval);
    if (eval->ch == '+' OR eval->ch == '-')
    {
      negative = (eval->ch == '-');
      nextch(eval);
    }
    while (isdigit(eval->ch))
    {
      number = number * 10 + ((ULONG)eval->ch - '0');
      nextch(eval);
    }
    if (negative) exponent -= number;
    else exponent += number;
  }
  if (exponent < 0)
  {
    while (exponent++) mantissa /= 10.0;
  }
  else
  {
    while (exponent--) mantissa *= 10.0;
  }
  eval->token = T_Constant;
  eval->tokenvalue.expr = mkfloatconst(eval, mantissa);
}

/*************************************** 
*
*  CR: parsen des aktuellen Ausdruckes 
*
*
****************************************/

PRIVATE void nexttoken(EVAL *eval)
{
  if (eval->backtracked)
  {
    eval->token = eval->nextlexicon.token;
    eval->tokenvalue = eval->nextlexicon.value;
    eval->backtracked = FALSE;
    return;
  }
  forever
  {
    while (isspace(eval->ch)) nextch(eval);
    switch (eval->ch)
    {
      case '0':
      nextch(eval);
      if (eval->ch == 'x' OR eval->ch == 'X')
      {
        nextch(eval);
        rdnumber(eval, HEX);
        return;
      }
      rdnumber(eval, OCTAL);
      return;
  
      case '1':case '2':case '3':case '4':case '5':
      case '6':case '7':case '8':case '9':
      rdnumber(eval, DECIMAL);
      return;
  
      case 'a':case 'b':case 'c':case 'd':case 'e':
      case 'f':case 'g':case 'h':case 'i':case 'j':
      case 'k':case 'l':case 'm':case 'n':case 'o':
      case 'p':case 'q':case 'r':case 's':case 't':
      case 'u':case 'v':case 'w':case 'x':case 'y':
      case 'z':
      case 'A':case 'B':case 'C':case 'D':case 'E':
      case 'F':case 'G':case 'H':case 'I':case 'J':
      case 'K':case 'L':case 'M':case 'N':case 'O':
      case 'P':case 'Q':case 'R':case 'S':case 'T':
      case 'U':case 'V':case 'W':case 'X':case 'Y':
      case 'Z':
      case '_':
      rdname(eval);
      return;
  
      case '{': eval->token = T_LBrace;      break;
      case '}': eval->token = T_RBrace;      break;
      case '[': eval->token = T_LBracket;    break;
      case ']': eval->token = T_RBracket;    break;
      case '(': eval->token = T_LParen;      break;
      case ')': eval->token = T_RParen;      break;
      case '?': eval->token = T_Conditional; break;
      case ':': eval->token = T_Colon;       break;
      case ',': eval->token = T_Comma;       break;
      case ';': eval->token = T_Semicolon;   break; 
      case '~': eval->token = T_BitNot;      break;
 
      case '.':
      eval->token = T_Dot;
      nextch(eval);
      if (isdigit(eval->ch))
      {
        eval->tokenbuffer[0] = '.';
        eval->tokenbuffer[1] = eval->ch;
        eval->tokenindex = 2;
        rdfloat(eval);
        return;
      }
      if (eval->ch == '.')
      {
        nextch(eval);
        if (eval->ch == '.')
        {
          eval->token = T_Ellipsis;
          break;
        }
        /* ACE: help ! */
      }
      return;
 
      case '*':
      nextch(eval);
      if (eval->ch == '=')
      {
        eval->token = T_TimesEq;
        break;
      }
      eval->token = T_Times;
      return;
 
      case '%':
      nextch(eval);
      if (eval->ch == '=')
      {
        eval->token = T_RemainderEq;
        break;
      }
      eval->token = T_Remainder;
      return;
 
      case '^':
      nextch(eval);
      if (eval->ch == '=')
      {
        eval->token = T_BitXOrEq;
        break;
      }
      eval->token = T_BitXOr;
      return;
  
      case '&':
      nextch(eval);
      if (eval->ch == '=')
      {
        eval->token = T_BitAndEq;
        break;
      }
      if (eval->ch == '&')
      {
        eval->token = T_LogAnd;
        break;
      }
      eval->token = T_BitAnd;
      return;
  
      case '=':
      nextch(eval);
      if (eval->ch == '=')
      {
        eval->token = T_EQ;
        break;
      }
      eval->token = T_Assign;
      return;
  
      case '+':
      nextch(eval);
      if (eval->ch == '=')
      {
        eval->token = T_PlusEq;
        break;
      }
      if (eval->ch == '+')
      {
        eval->token = T_PlusPlus;
        break;
      }
      eval->token = T_Plus;
      return;

      case '|':
      nextch(eval);
      if (eval->ch == '=')
      {
        eval->token = T_BitOrEq;
        break;
      }
      if (eval->ch == '|')
      {
        eval->token = T_LogOr;
        break;
      }
      eval->token = T_BitOr;
      return;
  
      case '<':
      nextch(eval);
      if (eval->ch == '<')
      {
        nextch(eval);
        if (eval->ch == '=')
        {
          eval->token = T_LShiftEq;
          break;
        }
        eval->token = T_LShift;
        return;
      }
      if (eval->ch == '=')
      {
        eval->token = T_LE;
        break;
      }
      eval->token = T_LT;
      return;
  
      case '>':
      nextch(eval);
      if (eval->ch == '>')
      {
        nextch(eval);
        if (eval->ch == '=')
        {
          eval->token = T_RShiftEq;
          break;
        }
        eval->token = T_RShift;
        return;
      }
      if (eval->ch == '=')
      {
        eval->token = T_GE;
        break;
      }
      eval->token = T_GT;
      return;
  
      case '!':
      nextch(eval);
      if (eval->ch == '=')
      {
        eval->token = T_NE;
        break;
      }
      eval->token = T_LogNot;
      return;
  
      case '-':
      nextch(eval);
      if (eval->ch == '=')
      {
        eval->token = T_MinusEq;
        nextch(eval);
        return;
      }
      if (eval->ch == '-')
      {
        eval->token = T_MinusMinus;
        break;
      }
      if (eval->ch == '>')
      {
        eval->token = T_Arrow;
        break;
      }
      eval->token = T_Minus;
      return;
  
      case '/':
      nextch(eval);
      if (eval->ch == '=')
      {
        eval->token = T_DivideEq;
        break;
      }
      if (eval->ch == '*')
      {
        rdcomment(eval);
        continue;
      }
      eval->token = T_Divide;
      return;
  
      case '\'':
      rdch(eval);
      return;

      case '"':
      rdstring(eval);
      return;

      case EOF:
      eval->token = T_End;
      return;

		/*  CR: fuer alles unbekannte  */
      default:
      synerr(eval, "Invalid lexical token, ignored");
      nextch(eval);
      continue;
    }
    nextch(eval);
    return;
  }
}

PRIVATE void lookahead(EVAL *eval)
{
  eval->prevlexicon.token = eval->token;
  eval->prevlexicon.value = eval->tokenvalue;
}

PRIVATE void backtrack(EVAL *eval)
{
  eval->backtracked = TRUE;
  eval->nextlexicon.token = eval->token;
  eval->nextlexicon.value = eval->tokenvalue;
  eval->token = eval->prevlexicon.token;
  eval->tokenvalue = eval->prevlexicon.value;
}

PRIVATE void checkfor(EVAL *eval, TOKEN expected)
{
  unless (eval->token == expected)
  {
    synerr(eval, "Missing %s", tokennames[(int)expected]);
    recover(eval);
  }
  nexttoken(eval);
}

PRIVATE BOOL punctuation(EVAL *eval, TOKEN expected)
{
  if (eval->token == expected)
  {
    nexttoken(eval);
    return TRUE;
  }
  return FALSE;
}

PRIVATE TYPE *rdtypedefname(EVAL *eval)
{
  ENTRY *entry;

  if ((entry = findtype(eval->debug->table, eval->tokenbuffer)) == NULL)
    return NULL;
  
  nexttoken(eval);
  return entry->type;
}

PRIVATE TYPE *rdstructdecl(EVAL *eval, char *name, TYPE *host)
{
  TYPE *type = rddecl(eval, name, host);

#ifdef BIT_FIELDS
  if (punctuation(eval, T_Colon)) rdexpr(eval, PASTCOMMA);
#endif
  return type;
}

PRIVATE TYPE *rdstructspec(EVAL *eval)
{
  TYPE *structure = newstruct();
  BOOL isstruct = (eval->token == T_Struct);
  int offset = 0;

  nexttoken(eval);
  if (eval->token == T_Identifier)
  {
#ifdef OLDCODE
    /* ACE: Declaring the tag is a waste of time, the tag is only local */
    declaretag(eval->debug->table, eval->block, eval->tokenbuffer, type);
#endif
    nexttoken(eval);
  }
  checkfor(eval, T_LBrace);
  until (eval->token == T_RBrace) /* ACE: using until is not a good idea */
  {
    TYPE *host;

    if ((host = rdtypespeclist(eval)) == NULL)
    {
      synerr(eval, "Missing type specifier");
      recover(eval);
    }
    do
    {
      char name[NameMax + 1];
      TYPE *type = rdstructdecl(eval, name, host);

      (void)declaremember(eval->debug->table, structure, name, type, offset);
      if (isstruct) offset += sizeoftype(type);
    } while (punctuation(eval, T_Comma));
    checkfor(eval, T_Semicolon);
  }
  checkfor(eval, T_RBrace);
  return structure;
}

/*
-- crf : 18/08/91 - eval declared, not used ...
*/
#ifdef OLDCODE
PRIVATE TYPE *rdenumspec(EVAL *eval)
#endif
PRIVATE TYPE *rdenumspec(void)
{
  TYPE *type = newenumeration();

#ifdef NEWCODE
  if (token == T_Identifier)
  {
#ifdef OLDCODE
    declaretag(eval->debug->table, eval->block, eval->tokenbuffer, type);
#endif
    nexttoken(eval);
  }
  checkfor(eval, T_LBrace);
  do
  {
    checkfor(eval, T_Identifier);
    checkfor(eval, T_Equals);
    rdexpr(eval, PASTCOMMA)
  } while (punctuation(eval, T_Comma));
  checkfor(eval, T_RBrace);
#endif
  return type;
}

PRIVATE TYPE *rdtypespeclist(EVAL *eval)
{
  TYPE *type = NULL;

  forever
  {
    switch (eval->token)
    {
      case T_Void:
      case T_Char:
      case T_Short:
      case T_Int:
      case T_Long:
      case T_Float:
      case T_Double:
      case T_Signed:
      case T_Unsigned:
      case T_Const:
      case T_Volatile:
      type = newtypedef(findtype(eval->debug->table, tokennames[(int)eval->token]));
      nexttoken(eval);
      return type;

      case T_Struct:
      case T_Union:
      return rdstructspec(eval);

      case T_Enum:
/*
-- crf : 18/08/91 - eval declared, not used ...
*/
#ifdef OLDCODE
      return rdenumspec(eval);
#endif
      return rdenumspec();

      case T_Identifier:
      if ((type = rdtypedefname(eval)) == NULL) return NULL;
      continue;

      default:
      return type;
    }
  }
}

PRIVATE TYPE *constructtype(TYPE *type, TYPE *host)
{
  TYPE *tail;

  if (type == NULL) return host;
  tail = type;
  until (tail->generic.host == NULL) tail = tail->generic.host;
  tail->generic.host = host;
  return type;
}

PRIVATE TYPE *rddirectdecl(EVAL *eval, char *name, TYPE *host)
{
  TYPE *type = NULL;
  int size;

  switch (eval->token)
  {
    case T_LParen:
    if (name == NULL)
    {
      lookahead(eval);
      nexttoken(eval);
      if (eval->token == T_Times OR eval->token == T_LParen OR
          eval->token == T_LBracket)
      {
        type = rddecl(eval, NULL, NULL);
        checkfor(eval, T_RParen);
      }
      else backtrack(eval);
      break;
    }
    nexttoken(eval);
    type = rddecl(eval, name, NULL);
    checkfor(eval, T_RParen);
    break;

    case T_Identifier:
    unless (name == NULL)
    {
      strcpy(name, eval->tokenbuffer);
      nexttoken(eval);
    }
    break;

    default:
    unless (name == NULL)
      synerr(eval, "Unexpected token %s in declarator", tokennames[(int)eval->token]);
    break;
  }
  forever
  {
    switch (eval->token)
    {
      case T_LBracket:
      nexttoken(eval);
#ifdef OLDCODE
      (void)rdexpr(eval, PASTCOMMA);
#else
      if (eval->token == T_Constant)
      {
        size = (int)eval->tokenvalue.number;
        nexttoken(eval);
      }
      else
      {
        (void)rdexpr(eval, PASTCOMMA);
        size = 0;
      }
#endif
      checkfor(eval, T_RBracket);
      type = constructtype(type, newarray(NULL, size, 0));
      continue;

      case T_LParen:
      nexttoken(eval);
#ifdef OLDCODE
      rdparamlist(eval);
#endif
      checkfor(eval, T_RParen);
      type = constructtype(type, newfunction(NULL));
      continue;

      default:
      return constructtype(type, host);
    }
  }
}

PRIVATE TYPE *rdpointer(EVAL *eval, TYPE *host)
{
  TYPE *type = NULL;

  while (punctuation(eval, T_Times))
    type = constructtype(type, newpointer(NULL));
  return constructtype(type, host);
}

PRIVATE TYPE *rddecl(EVAL *eval, char *name, TYPE *host)
{
  host = rdpointer(eval, host);
  return rddirectdecl(eval, name, host);
}

PRIVATE TYPE *rdtypename(EVAL *eval)
{
  TYPE *host;

  if ((host = rdtypespeclist(eval)) == NULL) return NULL;
  return rddecl(eval, NULL, host);
}

PRIVATE char *rdidentifier(EVAL *eval)
{
  char *name;

  unless (eval->token == T_Identifier)
  {
    synerr(eval, "Missing <identifier>");
    recover(eval);
  }
  name = strdup(eval->tokenbuffer);
  nexttoken(eval);
  return name;
}

PRIVATE int leftpri(TOKEN op)
{
  switch (op)
  {
    case T_Comma:
    return 2;

    case T_Assign: case T_TimesEq: case T_DivideEq:
    case T_RemainderEq: case T_PlusEq: case T_MinusEq:
    case T_LShiftEq: case T_RShiftEq: case T_BitAndEq:
    case T_BitXOrEq: case T_BitOrEq:
    return 4;

    case T_Conditional:
    return 6;

    case T_LogOr:
    return 8;

    case T_LogAnd:
    return 10;

    case T_BitOr:
    return 12;

    case T_BitXOr:
    return 14;

    case T_BitAnd:
    return 16;

    case T_EQ: case T_NE:
    return 18;

    case T_LT: case T_GT: case T_LE: case T_GE:
    return 20;

    case T_LShift: case T_RShift:
    return 22;

    case T_Plus: case T_Minus:
    return 24;

    case T_Times: case T_Divide: case T_Remainder:
    return 26;

    default:
    return 0;
  }
}

PRIVATE int rightpri(TOKEN op)
{
  switch (op)
  {
    case T_Comma:
    return 3;

    case T_Assign: case T_TimesEq: case T_DivideEq:
    case T_RemainderEq: case T_PlusEq: case T_MinusEq:
    case T_LShiftEq: case T_RShiftEq: case T_BitAndEq:
    case T_BitXOrEq: case T_BitOrEq:
    return 4;

    case T_Conditional:
    return 6;

    case T_LogOr:
    return 9;

    case T_LogAnd:
    return 11;

    case T_BitOr:
    return 13;

    case T_BitXOr:
    return 15;

    case T_BitAnd:
    return 17;

    case T_EQ: case T_NE:
    return 19;

    case T_LT: case T_GT: case T_LE: case T_GE:
    return 21;

    case T_LShift: case T_RShift:
    return 22;

    case T_Plus: case T_Minus:
    return 25;

    case T_Times: case T_Divide: case T_Remainder:
    return 27;

    default:
    return 0;
  }
}

PRIVATE EXPR *rdexpr(EVAL *eval, int pri)
{
  EXPR *expr;
  

  debugf("rdexpr (EVAL %x, int %d)",eval,pri);
  debugf("stackptr = %x",eval->stackptr);

  expr = rdunary(eval);
  
  while (leftpri(eval->token) >= pri AND expr != NULL) /* CR: make sure there is a valid expr */
  {
    TOKEN op = eval->token;

    nexttoken(eval);
    if (op == T_Conditional)
    {
      EXPR *oper = rdexpr(eval, PASTCOMMA);
      debugf ("conditional in rdexpr");

      checkfor(eval, T_Colon);
      expr = mkcond(eval, expr, oper, rdexpr(eval, rightpri(op)));
    }
    else expr = mkbinary(eval, op, expr, rdexpr(eval, rightpri(op)));
  }
  return expr;
}

PRIVATE TOKEN unaryversion(TOKEN op)
{
  switch (op)
  {
    case T_BitAnd:
    return T_Address;

    case T_Times:
    return T_Indirect;

    case T_Plus:
    return T_UPlus;

    case T_Minus:
    return T_UMinus;

    default:
    return op;
  }
}

PRIVATE EXPR *rdunary(EVAL *eval)
{
  TYPE *type;
  TOKEN op = eval->token;

  switch (op)
  {
    case T_PlusPlus:
    case T_MinusMinus:
    case T_BitAnd:
    case T_Times:
    case T_Plus:
    case T_Minus:
    case T_BitNot:
    case T_LogNot:
    nexttoken(eval);
    return mkunary(eval, unaryversion(op), rdunary(eval));

    case T_Sizeof:
    nexttoken(eval);
    if (punctuation(eval, T_LParen))
    {
      type = rdtypename(eval);
      checkfor(eval, T_RParen);
      return mkintconst(eval, sizeoftype(type));
    }
    return mkintconst(eval, sizeofexpr(rdunary(eval)));

    default:
    
    return rdpostfix(eval);
  }
}

PRIVATE EXPR *rdexprlist(EVAL *eval)
{
  EXPR *expr = rdexpr(eval, UPTOCOMMA);

  while (punctuation(eval, T_Comma))
#ifdef OLDCODE
    expr = mkexprlist(eval, expr, rdexpr(eval, UPTOCOMMA));
#endif
/*
-- crf : 18/08/91 - "eval" declared, not used
*/
    expr = mkexprlist(expr, rdexpr(eval, UPTOCOMMA));

  return expr;
}

PRIVATE EXPR *rdpostfix(EVAL *eval)
{
  EXPR *	expr;

  
  expr = rdprimary( eval );

  if (expr != NULL)  /* CR: in case there is no valid expr */
    {
      forever
	{
	  switch (eval->token)
	    {
	    case T_LBracket:
	      nexttoken(eval);
	      expr = mkbinary(eval, T_Subscript, expr, rdexpr(eval, PASTCOMMA));
	      checkfor(eval, T_RBracket);
	      continue;
	      
	    case T_LParen:
	      nexttoken(eval);
	      expr = mkbinary(eval, T_Call, expr, rdexprlist(eval));
	      checkfor(eval,T_RParen);
	      continue;
	      
	    case T_Dot:
	      nexttoken(eval);
	      expr = mkfieldref(eval, T_Dot, expr, rdidentifier(eval));
	      continue;
	      
	    case T_Arrow:
	      nexttoken(eval);
	      expr = mkfieldref(eval, T_Arrow, expr, rdidentifier(eval));
	      continue;
	      
	    case T_PlusPlus:
	      nexttoken(eval);
	      expr = mkunary(eval, T_PostInc, expr);
	      continue;
	      
	    case T_MinusMinus:
	      nexttoken(eval);
	      expr = mkunary(eval, T_PostDec, expr);
	      continue;
	      
	    default:
	      return expr;
	    }
	}
    }
  
  return expr;
}

PRIVATE EXPR *rdprimary(EVAL *eval)
{
  EXPR *	expr;
  TYPE *	type;

  
  switch (eval->token)
    {
    case T_Identifier:
      
      debugf("Identifier");
      if (eval->tokenvalue.entry == NULL)
	{
	  synerr(eval, "Undefined variable %s", eval->tokenbuffer);
	  /* recover(eval); CR: causes a fault */
	  expr = NULL ;
	}
#ifdef PARSYTEC
      if (expr != NULL)   /* CR: if no fault'*/
#endif
	/*
	  -- crf : 18/08/91 (Sunday 02h05 if you're interested)
	  -- Not happy with this. Using the "-d" option, I am unable to print
	  -- or watchpoint expressions ... unless I uncomment the DISPLAY
	  -- declaration. Yes, I am confused. I particulary don't like testing
	  -- for (expr != NULL) at this point. Try using "else" instead.
	  */
      else
	{
	  expr = mkidentifier(eval->tokenvalue.entry);
	  nexttoken(eval);
	}
      
      return expr;
      
    case T_Constant:
    case T_String:
      expr = eval->tokenvalue.expr;

      nexttoken( eval );

      return expr;
      
    case T_LParen:
      nexttoken(eval);
      if ((type = rdtypename(eval)) == NULL)
	{
	  expr = rdexpr(eval, PASTCOMMA);
	  checkfor(eval, T_RParen);
	  return expr;
	}
      checkfor(eval, T_RParen);
      return mkcast(eval, type, rdunary(eval));
      
    default:
#ifdef OLDCODE
      synerr(eval, "Missing primary");
      recover(eval);
#else
      return NULL;
#endif
    }
}

/* ACE: Tidy up */
/**
*
* synerr(eval, format, ...);
*
* Display a syntax error.
*
**/
PRIVATE void synerr(EVAL *eval, char *format, ...)
{
  DISPLAY *display = eval->debug->display;
  bool test = FALSE;
    
  va_list args;
  va_start(args, format);
    
  if (TestSemaphore(&display->lock) > 0)
  {
    dlock(display); /* CR: ensure display has to be locked */
    test = TRUE;
  }
  /* dcursor(display, display->height - 1, 0); CR: why ? */
  deol(display);
  vfprintf(display->fileout, format, args);
  if (test)
  {
    dunlock(display);/* CR: only free display if locked before */
    test = FALSE;
  }
  va_end(args);
}

