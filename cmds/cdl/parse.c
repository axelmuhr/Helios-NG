/**
*
* Title:  CDL Compiler - Parser.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988 - 1992, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/
/* static char *rcsid = "$Header: /hsrc/cmds/cdl/RCS/parse.c,v 1.4 1993/08/02 12:28:45 nickc Exp $"; */

#include "cdl.h"
#include "cdlobj.h"

#ifdef LINE_MAX
#undef LINE_MAX
#endif

#define LINE_MAX 255

char namebuffer[WORD_MAX + 1];
char tokenbuffer[WORD_MAX + 1];
char linebuffer[LINE_MAX + 1];
int lineindex = -1;
int tokenindex;

CMD *parse(void)
{
  DEBUG("parse()");
  nextch();
  nexttoken();
  while (token == T_COMPONENT) readdeclaration();
  if (token == T_EOF) return NULL;
  return readcmdlist(0);
}

char *readexpr(void)
{
  static char exprbuffer[WORD_MAX + 1];

  exprbuffer[0] = '\0';
  while (token == T_WORD OR token == T_LPAREN OR token == T_RPAREN)
  {
    if (token == T_LPAREN) strcat(exprbuffer, "(");
    else if (token == T_RPAREN) strcat(exprbuffer, ")");
    else strcat(exprbuffer, tokenbuffer);
    nexttoken();
  }
  return exprbuffer;
}

ARGV readsubexprs(TOKEN term)
{
  ARGV subexprs = nullargv();

  do subexprs = addword(subexprs, readexpr()); while (punctuation(T_COMMA));
  unless (punctuation(term)) synerr("']' or '}' expected, inserted");
  return subexprs;
}

void readdeclaration(void)
{
  COMPONENT *component;

  DEBUG("readdeclaration()");
  unless (punctuation(T_COMPONENT)) bug("'component' expected");
  unless (token == T_WORD)
  {
    synerr("Component name expected, component declaration ignored");
    until (token == T_EOF OR token == T_RBRACE) nexttoken();
    ignore(T_RBRACE);
    return;
  }
  component = newtemplate(tokenbuffer);
  nexttoken();
  if (punctuation(T_LBRACKET)) addsubnames(component, readsubexprs(T_RBRACKET));
  unless (punctuation(T_LBRACE)) synerr("'{' expected, inserted");
  until (token == T_EOF OR token == T_RBRACE)
  {
    readattribute(component);
    unless (punctuation(T_SEMICOLON))
    {
      synerr("';' expected");
      until (token == T_EOF OR token == T_SEMICOLON OR token == T_RBRACE)
        nexttoken();
      ignore(T_SEMICOLON);
    }
  }
  unless (punctuation(T_RBRACE)) synerr("'}' expected, inserted");
}

void readattribute(COMPONENT *component)
{
  DEBUG("readattribute()");
  switch (token)
  {
    case T_PTYPE:
    nexttoken();
    component->ptype = readptype();
    break;

    case T_PUID:
    nexttoken();
    addpuid(component, readname());
    break;

    case T_ATTRIB:
    nexttoken();
    readattriblist(component);
    break;

    case T_MEMORY:
    nexttoken();
    component->memory = readnumber();
    break;

    case T_LIFE:
    nexttoken();
    component->life = readlife();
    break;

    case T_TIME:
    nexttoken();
#ifdef NEVER    
    component->time = readnumber();
#endif
    (void) readnumber();
    warning("time not supported in this release");
    break;

    case T_PRIORITY:
    nexttoken();
#ifdef NEVER    
    component->priority = readnumber();
#endif
    (void) readnumber();
    warning("priority not supported in this release");    
    break;

    case T_STREAMS:
    nexttoken();
    readstreamlist(component);
    break;

    case T_CODE:
    nexttoken();
    readpath(component);
    break;

    case T_ARGV:
    nexttoken();
    readargv(component);
    break;

    default:
    synerr("Unknown attribute '%s', ignored", tokenbuffer);
    until (token == T_EOF OR token == T_SEMICOLON OR token == T_RBRACE)
      nexttoken();
    break;
  }
}

PTYPE readptype(void)
{
  PTYPE ptype = ANY_PROCESSOR;

  if (strequ("ANY", tokenbuffer)) ptype = ANY_PROCESSOR;
  else if (strequ("T212", tokenbuffer)) ptype = T212;
  else if (strequ("T414", tokenbuffer)) ptype = T414;
  else if (strequ("T800", tokenbuffer)) ptype = T800;
  else synerr("Invalid processor type '%s', assumed to be 'ANY'", tokenbuffer);
  nexttoken();
  return ptype;
}

LIFE readlife(void)
{ nexttoken();
  warning("life not supported in this release");
  return(IMMORTAL);
  
#ifdef NEVER	
  LIFE life = MORTAL;

  if (strequ("mortal", tokenbuffer)) life = MORTAL;
  else if (strequ("immortal", tokenbuffer)) life = IMMORTAL;
  else synerr("Invalid life '%s', assumed to be 'mortal'", tokenbuffer);
  nexttoken();
  return life;
#endif  
}

void readattriblist(COMPONENT *component)
{
  DEBUG("readattriblist()");
  do readattrib(component); while (punctuation(T_COMMA));
}

void readattrib(COMPONENT *component)
{
  char *name;
  int count = 1;

  name = readname();
  if (punctuation(T_LBRACKET))
  {
    count = readnumber();
    unless (punctuation(T_RBRACKET)) synerr("']' expected, inserted");
  }
  addattrib(component, name, count);
}

void readstreamlist(COMPONENT *component)
{
  CHANV chanv = newchanv();
  int fd = 0;

  DEBUG("readstreamlist()");
  until (token == T_EOF OR token == T_SEMICOLON)
  {
    unless (punctuation(T_COMMA))
    {
      int mode = readmode();
      char *name = readname();
      ARGV subexprs = NULL;

      if (punctuation(T_LBRACE)) subexprs = readsubexprs(T_RBRACE);
      addchannel(chanv, fd, newchannel(name, subexprs, mode));
      ignore(T_COMMA);
    }
    fd++;
  }
  addchanv(component, chanv);
}

int readmode(void)
{
  int mode;

  switch (token)
  {
    case T_READ:
    mode = O_ReadOnly | O_External;
    break;

    case T_WRITE:
    mode = O_WriteOnly | O_Create | O_External;
    break;

    case T_READWRITE:
    mode = O_WriteOnly | O_External;
    break;

    case T_APPEND:
    mode = O_WriteOnly | O_Append | O_External;
    break;

    case T_READFIFO:
    mode = O_ReadOnly;
    break;

    case T_WRITEFIFO:
    mode = O_WriteOnly;
    break;

    default:
    return O_ReadOnly | O_External;
  }
  nexttoken();
  return mode;
}

void readpath(COMPONENT *component)
{
  DEBUG("readpath()");
  addpath(component, tokenbuffer);
  nexttoken(); 
}

void readargv(COMPONENT *component)
{
  ARGV argv = nullargv();

  DEBUG("readargv()");
  while (token == T_WORD)
  {
    argv = addword(argv, tokenbuffer);
    nexttoken();
  }
  addargv(component, argv);
}

REPLICATOR *readreplicator(void)
{
  BINDV bindv;
  int dim = 0;

  unless (punctuation(T_LBRACKET)) return NULL;
  bindv = newbindv();
  do
  {
    if (dim == BIND_MAX)
    {
      synerr("Too many dimensions in a single replicator, (limit is %d)", BIND_MAX);
      until (token == T_EOF OR token == T_RBRACKET) nexttoken();
      break;
    }
    if (isalpha(tokenbuffer[0]))
    {
      unless (token == T_WORD)
      {
        synerr("Subscript name expected");
        until (token == T_EOF OR token == T_RBRACKET) nexttoken();
        break;
      }
      bindv[dim].name = strdup(tokenbuffer);
      nexttoken();
      punctuation(T_READ);
    }
    bindv[dim++].value = readnumber();
  } while (punctuation(T_COMMA));
  unless (punctuation(T_RBRACKET)) synerr("']' expected, inserted");
  return newreplicator(dim, bindv);
}

char *readname(void)
{
  char *name;

  DEBUG("readname()");
  if (token == T_WORD)
  {
    strcpy(namebuffer, tokenbuffer);
    name = namebuffer;
  }
  else
  {
    synerr("Name expected, assumed NULL");
    name = NULL;
  }
  nexttoken();
  return name;
}

int readnumber(void)
{
  int number = 0;
  int index = 0;
  char c;

  DEBUG("readnumber()");
  if (token == T_WORD)
  {
    while ((c = tokenbuffer[index++]) != '\0' AND isdigit(c))
    {
      number = (number * 10) + (c - '0');
    }
    if (c == '\0') nexttoken();
    return number;
  }
  synerr("Bad number, assumed to be 0");
  return 0;
}

BOOL isconstructor(TOKEN token)
{
  return token == T_PAR OR token == T_PIPE OR token == T_REVPIPE OR
         token == T_SUBORDINATE OR token == T_INTERLEAVE;
}

BOOL isdelimitor(TOKEN token)
{
  return token == T_SEMICOLON OR token == T_AMPERSAND OR
         token == T_AND OR token == T_OR;
}

BOOL isterminator(TOKEN token)
{
  return token == T_SEMICOLON OR token == T_AMPERSAND;
}

BOOL isredirection(TOKEN token)
{
  return token == T_READ OR token == T_WRITE OR token == T_APPEND OR
         token == T_READFIFO OR token == T_WRITEFIFO;
}

int getpri(TOKEN delimitor)
{
  switch (delimitor)
  {
    case T_SEMICOLON:
    return 1;

    case T_AMPERSAND:
    return 1;

    case T_AND:
    return 2;

    case T_OR:
    return 2;

    default:
    bug("Unexpected delimitor");
    return 0;
  }
}

CMD *readcmd(void)
{
  CMD *cmd;

  DEBUG("readcmd()");
  if (punctuation(T_LPAREN))
  {
    cmd = newcmd(T_LIST, readtaskforce(), NULL);
    unless (punctuation(T_RPAREN)) synerr("')' expected, inserted");
  }
  else if ((cmd = readsimplecmd()) == NULL) return NULL;
  cmd->next = readauxlist();
  return cmd;
}

CMD *readcmdlist(int rightpri)
{
  CMD *cmd;

  DEBUG("readcmdlist()");
  cmd = readtaskforce();
  forever
  {
    if (isdelimitor(token))
    {
      TOKEN delimitor = token;
      int leftpri = getpri(delimitor);

      if (leftpri <= rightpri) return cmd;
      nexttoken();
      if (token == T_EOF AND isterminator(delimitor))
        return newcmd(delimitor, cmd, NULL);
      cmd = newcmd(delimitor, cmd, readcmdlist(leftpri));
    }
    else return cmd;
  }
}

#ifdef COMPATIBLE
CMD *buildfarm(REPLICATOR *rep)
{
  SIMPLE *lb;
  char numstr[NUMSTR_MAX + 1];
  ARGV argv = nullargv();
  int count = 1;
  int dim;

  if (rep == Null(REPLICATOR))
   { BINDV bindv = newbindv();
     bindv[0].value = 1;
     rep = newreplicator(1, bindv);
   }
   
  for (dim = 0; dim < rep->dim; dim++) count *= rep->bindv[dim].value;
  argv = addword(argv, "lb");
  (void)sprintf(numstr, "%d", count);
  argv = addword(argv, numstr);
  lb = newsimple(argv, NULL, newchanv());
  lb->aux = (CMD *)rep;
  rep->repop = T_INTERLEAVE;
  rep->cmd = newcmd(T_SUBORDINATE, NULL, readpipeline());
  return (CMD *)lb;
}

CMD *readconstruction(CMD *cmd)
{
  REPLICATOR *rep = readreplicator();

  switch (rep->repop = token)
  {
    case T_PAR:
    nexttoken();
    rep->cmd = readinterleave();
    break;

    case T_INTERLEAVE:
    nexttoken();
    return newcmd(T_SUBORDINATE, cmd, buildfarm(rep));

    case T_PIPE:
    nexttoken();
    rep->cmd = readsubordinate();
    break;

    case T_REVPIPE:
    nexttoken();
    rep->cmd = readsubordinate();
    break;

    case T_SUBORDINATE:
    nexttoken();
    rep->cmd = readcmd();
    break;

    default:
    synerr("Constructor expected, farm assumed");
    return newcmd(T_SUBORDINATE, cmd, buildfarm(rep));
  }
  return newcmd(rep->repop, cmd, (CMD *)rep);
}
#endif

CMD *readtaskforce(void)
{
  CMD *cmd;

  DEBUG("readtaskforce()");
  if (punctuation(T_PAR))
  {
    REPLICATOR *rep;
    rep = readreplicator();
    rep->repop = T_PAR;
    rep->cmd = readinterleave();
    return (CMD *)rep;
  }

  cmd = readinterleave();

#ifdef COMPATIBLE
  while (token == T_PAR OR token == T_LBRACKET)
  { 
    if (token == T_LBRACKET)
    { 
      cmd = readconstruction(cmd);
    }
    else
    { 
      nexttoken();
      cmd = newcmd(T_PAR, cmd, readinterleave());
    }
  }
#else
  while (punctuation(T_PAR)) cmd = newcmd(T_PAR, cmd, readinterleave());
#endif
  return cmd;
}

CMD *readinterleave(void)
{ CMD *cmd;

  DEBUG("readinterleave()");
  if (punctuation(T_INTERLEAVE))
   { 
     return buildfarm(readreplicator());
   }
  cmd = readpipeline();
  while (punctuation(T_INTERLEAVE))
   cmd = newcmd( T_SUBORDINATE, cmd,  buildfarm(Null(REPLICATOR)));
   
  return cmd;
}

CMD *readpipeline(void)
{
  CMD *cmd;

  DEBUG("readpipeline()");
  if (punctuation(T_PIPE))
  {
    REPLICATOR *rep = readreplicator();

    rep->repop = T_PIPE;
    rep->cmd = readsubordinate();
    return (CMD *)rep;
  }
  cmd = readsubordinate();
#ifdef COMPATIBLE
  while (token == T_PIPE OR token == T_LBRACKET)
  {
    if (token == T_LBRACKET) cmd = readconstruction(cmd);
    else
    {
      nexttoken();
      cmd = newcmd(T_PIPE, cmd, readsubordinate());
    }
  }
#else
  while (punctuation(T_PIPE)) cmd = newcmd(T_PIPE, cmd, readsubordinate());
#endif
  return cmd;
}

CMD *readsubordinate(void)
{
  CMD *cmd;

  DEBUG("readsubordinate()");
  if (punctuation(T_SUBORDINATE))
  {
    REPLICATOR *rep = readreplicator();

    rep->repop = T_SUBORDINATE;
    rep->cmd = readcmd();
    return (CMD *)rep;
  }
  cmd = readcmd();
#ifdef COMPATIBLE
  while (token == T_SUBORDINATE OR token == T_LBRACKET)
  {
    if (token == T_LBRACKET) cmd = readconstruction(cmd);
    else
    {
      nexttoken();
      cmd = newcmd(T_SUBORDINATE, cmd, readcmd());
    }
  }
#else
  while (punctuation(T_SUBORDINATE)) cmd = newcmd(T_SUBORDINATE, cmd, readcmd());
#endif
  return cmd;
}

CMD *readauxlist(void)
{
  CMD *cmd;

  DEBUG("readauxlist()");
  unless (punctuation(T_LPAREN)) return NULL;
  if (punctuation(T_COMMA))
  {
    REPLICATOR *rep = readreplicator();

    rep->repop = T_COMMA;
    rep->cmd = readaux();
    return (CMD *)rep;
  }
  cmd = readaux();
  while (punctuation(T_COMMA)) cmd = newcmd(T_COMMA, cmd, readaux());
  unless (punctuation(T_RPAREN)) synerr("')' expected, inserted");
  return cmd;
}

CMD *readaux(void)
{
  DEBUG("readaux()");
  if (punctuation(T_PIPE)) return newcmd(T_PIPE, NULL, readtaskforce());
  if (punctuation(T_REVPIPE)) return newcmd(T_REVPIPE, NULL, readtaskforce());
  if (punctuation(T_SUBORDINATE)) return newcmd(T_SUBORDINATE, NULL, readtaskforce());
  synerr("Constructor expected, auxiliary ignored");
  until (token == T_EOF OR token == T_COMMA OR token == T_RPAREN) nexttoken();
  return NULL;
}

void readredirection(CHANV chanv)
{
  int mode = readmode();

  if (token == T_WORD)
  {
    char *name = readname();
    ARGV subexprs = NULL;

    if (punctuation(T_LBRACE)) subexprs = readsubexprs(T_RBRACE);
    addchannel(chanv, getstd(mode), newchannel(name, subexprs, mode));
  }
  else synerr("Missing name for redirect, ignored");
}

CMD *readsimplecmd(void)
{
  ARGV argv;
  ARGV subv = NULL;
  CHANV chanv;

  DEBUG("readsimplecmd()");
  unless (token == T_WORD)
  {
    synerr("Command expected");
    return NULL;
  }
  argv = addword(nullargv(), tokenbuffer);
  nexttoken();
  if (punctuation(T_LBRACE)) subv = readsubexprs(T_RBRACE);
  chanv = newchanv();
  while (token == T_WORD OR isredirection(token))
  {
    if (token == T_WORD)
    {
      argv = addword(argv, tokenbuffer);
      nexttoken();
    }
    else readredirection(chanv);
  }
  return (CMD *)newsimple(argv, subv, chanv);
}

BOOL punctuation(TOKEN expected)
{
  if (token == expected)
  {
    nexttoken();
    return TRUE;
  }
  return FALSE;
}

void ignore(TOKEN expected)
{
  if (token == expected) nexttoken();
}

BOOL ismetach(int c)
{
  return c == ';' OR c == '|' OR c == '&' OR
         c == '(' OR c == ')' OR c == '[' OR c == ']' OR
         c == '<' OR c == '>' OR c == '{' OR c == '}' OR
         c == ',' OR c == '^';
}

char *getarguments(char *start, char *buffer)
{
  char *arg;
  int count = 1;
  int index = 0;
  int length = strlen(start);

  until ((arg = arguments[count++]) == NULL)
  {
    if (strnequ(start, arg, length))
    {
      strcpy(buffer + index, arg);
      index += strlen(arg);
      buffer[index++] = ' ';
    }
  }
  buffer[index] = '\0';
  return buffer;
}

static char macrobuffer[1025];

void nexttoken(void)
{
  int index = 0;
  KEYWORD *keyword;

  DEBUG("nexttoken()");
  forever
  {
    tokenindex = lineindex;
    switch (ch)
    {
      case '\n':
      linenumber++;
      case ' ':
      case '\t':
      nextch();
      continue;

      case '#':
      do nextch(); until (ch == '\n' OR ch == EOF);
      continue;

      case '$':
      nextch();
      {
        char *macrotext;
        char name[256];
        int number = 0;
        int index = 0;
        BOOL inbraces = FALSE;

        if (ch == '{')
        {
          nextch();
          inbraces = TRUE;
        }
        switch (ch)
        {
          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
          do
          {
            number = (number * 10) + (ch - '0');
            nextch();
          } while (isdigit(ch));
          if (number <= argcount) macrotext = arguments[number];
          else macrotext = NULL;
          break;

          case '#':
          nextch();
          (void)sprintf(macrobuffer, "%d", argcount);
          macrotext = macrobuffer;
          break;

          case '^':
          nextch();
          until (isspace(ch))
          {
            name[index++] = ch;
            nextch();
          }
          name[index] = '\0';
          macrotext = getarguments(name, macrobuffer);
          break;

          case '*':
          nextch();
          macrotext = getarguments("", macrobuffer);
          break;

          default:
          while (isalpha(ch))
          {
            name[index++] = ch;
            nextch();
          }
          name[index] = '\0';
          macrotext = getenv(name);
          break;
        }
        if (inbraces)
        {
          if (ch == '}') nextch();
          else fprintf(stderr, "} expected, inserted\n");
        }
        newmacro(macrotext);
      }
      continue;

      case ';':
      nextch();
      token = T_SEMICOLON;
      return;

      case '&':
      nextch();
      if (ch == '&')
      {
        nextch();
        token = T_AND;
        return;
      }
      token = T_AMPERSAND;
      return;

      case '|':
      nextch();
      switch (ch)
      {
        case '|':
        nextch();
        if (ch == '|')
        {
          nextch();
          token = T_INTERLEAVE;
          return;
        }
        token = T_OR;
        return;

        case '<':
        nextch();
        token = T_REVPIPE;
        return;

        case '>':
        nextch();
        token = T_PIPE;
        return;
      }
      token = T_PIPE;
      return;

      case '^':
      nextch();
      if (ch == '^')
      {
        nextch();
        token = T_PAR;
        return;
      }
      if (isspace(ch) OR ismetach(ch)) 
      {
        token = T_PAR;
        return;
      }
      tokenbuffer[index++] = '^';
      continue;

      case ',':
      nextch();
      token = T_COMMA;
      return;

      case '{':
      nextch();
      token = T_LBRACE;
      return;

      case '}':
      nextch();
      token = T_RBRACE;
      return;

      case '(':
      nextch();
      token = T_LPAREN;
      return;

      case ')':
      nextch();
      token = T_RPAREN;
      return;

      case '[':
      nextch();
      token = T_LBRACKET;
      return;

      case ']':
      nextch();
      token = T_RBRACKET;
      return;

      case '<':
      nextch();
      switch (ch)
      {
        case '>':
        nextch();
        token = T_SUBORDINATE;
        return;

        case '|':
        nextch();
        token = T_READFIFO;
        return;
      }
      token = T_READ;
      return;

      case '>':
      nextch();
      switch (ch)
      {
        case '>':
        nextch();
        token = T_APPEND;
        return;

        case '|':
        nextch();
        token = T_WRITEFIFO;
        return;
      }
      token = T_WRITE;
      return;

      default:
      until (isspace(ch) OR ismetach(ch))
      {
        if (ch == '\\') nextch();
        tokenbuffer[index++] = ch;
        nextch();
      }
      tokenbuffer[index] = '\0';
      if ((keyword = findkeyword(tokenbuffer)) == NULL) token = T_WORD;
      else token = keyword->token;
      return;

      case EOF:
      token = T_EOF;
      return;
    }
  }
}

char *macro = NULL;
int savech;

void nextch(void)
{ 
back:
  if (macro == NULL)
   {
    if (lineindex == -1 OR (ch = linebuffer[++lineindex]) == '\0')
     { 
       if (fgets(linebuffer, LINE_MAX, inputfile) == NULL)
        {
  	  lineindex = -1;
          ch = EOF;
        }
       else ch = linebuffer[lineindex = 0];
     }
   }
  else
   readmacroch();

  if (ch == '\r') goto back;
}

void newmacro(char *text)
{
  if (text == NULL) return;
  macro = text;
  savech = ch;
  nextch();
}

void readmacroch(void)
{
  unless ((ch = *macro++) == '\0') return;
  macro = NULL;
  ch = savech;
}

PUBLIC void errorline()
{
  unless (lineindex == -1)
  {
    int i;

    fprintf(stderr, "%4d: %s", linenumber, linebuffer);
    fprintf(stderr, "      ");
    for (i = 0; i < tokenindex; i++) fputc(' ', stderr);
    fputc('^', stderr);
    fputc('\n', stderr);
  }
}
