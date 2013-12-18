/**
*
* Title:  CDL Compiler - Expand a replicated structure.
*
* Author: Andy England
*
* Date:   February 1989
*
*         (c) Copyright 1989 - 1992, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/
/* static char *rcsid = "$Header: /hsrc/cmds/cdl/RCS/expand.c,v 1.3 1992/06/11 11:45:16 nickc Exp $"; */

#include "cdl.h"

SIMPLE *expandsimple(SIMPLE *simple);
ARGV expandargv(ARGV);
CHANV expandchanv(CHANV);
CMD *replicatecmd(REPLICATOR *, int);

CMD *expandcmd(CMD *cmd)
{
  if (cmd == NULL) return NULL;
  if (cmd->op == T_SIMPLE) return (CMD *)expandsimple( (SIMPLE *) cmd);
  if (cmd->op == T_REPLICATOR) return replicatecmd((REPLICATOR *)cmd, 0);
  return newcmd(cmd->op, expandcmd(cmd->this), expandcmd(cmd->next));
}

SIMPLE *expandsimple(SIMPLE *simple)
{
  SIMPLE *dup;
  char *expr;
  int subv[BIND_MAX];
  int i = 0;
  ARGV argv;
  CHANV chanv;

  unless (simple->subv == NULL)
  {
    until ((expr = simple->subv[i]) == NULL) subv[i++] = evalexpr(expr);
  }
  while (i < BIND_MAX) subv[i++] = 0;
  argv = expandargv(simple->argv);
  chanv = expandchanv(simple->chanv);
  dup = newsimple(NULL, NULL, NULL);
  dup->aux = expandcmd(simple->aux);
  dup->component = usecomponent(argv, subv, chanv);
  return dup;
}

ARGV expandargv(ARGV argv)
{
  ARGV dupargv = nullargv();
  char *arg;

  until ((arg = *argv++) == NULL)
  {
    if (arg[0] == '%')
    {
      int value = evalexpr(arg + 1);
      char numstr[NUMSTR_MAX + 1];

      sprintf(numstr, "%d", value);
      dupargv = addword(dupargv, numstr);
    }
    else dupargv = addword(dupargv, arg);
  }
  return dupargv;
}

CHANV expandchanv(CHANV chanv)
{
  CHANV dupchanv = newchanv();
  int fd;

  for (fd = 0; chanv->channels[fd] != NULL; fd++)
  {
    CHANNEL *channel;

    unless ((channel = chanv->channels[fd]) == STDCHAN)
    {
      ARGV subexprs;
      char name[PATH_MAX + 1];

      strcpy(name, channel->name);
      unless ((subexprs = channel->subv) == NULL)
      {
      	char *expr;

        until ((expr = *subexprs++) == NULL)
        {
          int value = evalexpr(expr);
          char substr[NUMSTR_MAX + 2];

          sprintf(substr, ".%d", value);
          strcat(name, substr);
        }
      }
      addchannel(dupchanv, fd, newchannel(name, NULL, channel->mode));
    }
  }
  return dupchanv;
}

CMD *replicatecmd(REPLICATOR *rep, int dim)
{
  TOKEN op = (rep->repop == T_INTERLEAVE) ? T_COMMA : rep->repop;
  CMD *cmd = rep->cmd;
  CMD *dup = NULL;		/* ACC 1/11/90 */
  BINDV bindv = rep->bindv;
  int i;

  for (i = 0; i < bindv[dim].value; i++)
  {
    unless (bindv[dim].name == NULL) bindname(bindv[dim].name, i);
    if (rep->dim - dim == 1)
    {
      if (i == 0) dup = expandcmd(cmd);
      else dup = newcmd(op, dup, expandcmd(cmd));
    }
    else
    {
      if (i == 0) dup = replicatecmd(rep, dim + 1);
      else dup = newcmd(op, dup, replicatecmd(rep, dim + 1));
    }
    unless (bindv[dim].name == NULL) unbind();
  }
  return dup;
}
