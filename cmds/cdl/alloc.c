/**
*
* Title:  CDL Compiler - Stream Allocation.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/
static char *rcsid = "$Header: /hsrc/cmds/cdl/RCS/alloc.c,v 1.3 1991/09/12 15:24:47 bart Exp $";

#include "cdl.h"

void resolvechannels(COMPONENT *component)
{
  CHANNEL *channel;
  int fd;

  for (fd = 0; component->chanv->channels[fd] != NULL; fd++)
  {
    unless ((channel = component->chanv->channels[fd]) == STDCHAN)
     channel->fifo = usefifo(channel->name, channel->mode);
  }
}

void allocchannels(cmd)
CMD *cmd;
{
  char *name;

  DEBUG("allocchannels()");
  if (cmd == NULL) return;
  switch (cmd->op)
  {
    case T_COMMA:
    case T_PAR:
    break;

    case T_PIPE:
    name = inventname();
    addstream(cmd->this, 1, newchannel(name, NULL, O_WriteOnly));
    addstream(cmd->next, 0, newchannel(name, NULL, O_ReadOnly));
    break;

    case T_REVPIPE:
    name = inventname();
    addstream(cmd->this, 0, newchannel(name, NULL, O_ReadOnly));
    addstream(cmd->next, 1, newchannel(name, NULL, O_WriteOnly));
    break;

    case T_SUBORDINATE:
    name = inventname();
    addstream(cmd->this, 5, newchannel(name, NULL, O_WriteOnly));
    addstream(cmd->next, 0, newchannel(name, NULL, O_ReadOnly));
    name = inventname();
    addstream(cmd->this, 4, newchannel(name, NULL, O_ReadOnly));
    addstream(cmd->next, 1, newchannel(name, NULL, O_WriteOnly));
    break;

    case T_SEMICOLON:
    case T_AMPERSAND:
    case T_AND:
    case T_OR:
    break;

    case T_LIST:
    allocchannels(cmd->this);
    allocauxlist(cmd, cmd->next, 4);
    return;

    case T_SIMPLE:
    allocauxlist(cmd, ((SIMPLE *)cmd)->aux, 4);
    resolvechannels(((SIMPLE *)cmd)->component);
    return;

    default:
    bug("Unexpected command type in allocchannels");
    return;
  }
  allocchannels(cmd->this);
  allocchannels(cmd->next);
}

void addstream(CMD *cmd, int fd, CHANNEL *channel)
{
  DEBUG("addstream()");
  if (cmd == NULL) return;
  switch (cmd->op)
  {
    case T_SIMPLE:
    addchannel(((SIMPLE *)cmd)->component->chanv, fd, channel);
    break;

    case T_LIST:
    addstream(cmd->this, fd, channel);
    break;

    case T_PAR:
    addstream(cmd->this, fd, channel);
    addstream(cmd->next, fd, channel);
    break;

    case T_PIPE:
    unless (fd == 1) addstream(cmd->this, fd, channel);
    unless (fd == 0) addstream(cmd->next, fd, channel);
    break;

    case T_REVPIPE:
    unless (fd == 0) addstream(cmd->this, fd, channel);
    unless (fd == 1) addstream(cmd->next, fd, channel);
    break;

    case T_SUBORDINATE:
    unless (fd == 4 OR fd == 5) addstream(cmd->this, fd, channel);
    unless (fd == 0 OR fd == 1) addstream(cmd->next, fd, channel);
    break;

    default:
    bug("Unexpected command type in addstream");
    return;
  }
}

int allocauxlist(CMD *cmd, CMD *auxlist, int fd)
{
  DEBUG("allocauxlist()");
  if (auxlist == NULL) return fd;
  if (auxlist->op == T_COMMA)
  {
    fd = allocauxlist(cmd, auxlist->this, fd);
    return allocauxlist(cmd, auxlist->next, fd);
  }
  return allocaux(cmd, auxlist, fd);
}

int allocaux(CMD *cmd, CMD *aux, int fd)
{
  char *name;

  DEBUG("allocaux()");
  switch (aux->op)
  {
    case T_PAR:
    break;

    case T_PIPE:
    name = inventname();
    addstream(cmd, fd + 1, newchannel(name, NULL, O_WriteOnly));
    addstream(aux->next, 0, newchannel(name, NULL, O_ReadOnly));
    break;

    case T_REVPIPE:
    name = inventname();
    addstream(cmd, fd, newchannel(name, NULL, O_ReadOnly));
    addstream(aux->next, 1, newchannel(name, NULL, O_WriteOnly));
    break;

    case T_SUBORDINATE:
    name = inventname();
    addstream(cmd, fd + 1, newchannel(name, NULL, O_WriteOnly));
    addstream(aux->next, 0, newchannel(name, NULL, O_ReadOnly));
    name = inventname();
    addstream(cmd, fd, newchannel(name, NULL, O_ReadOnly));
    addstream(aux->next, 1, newchannel(name, NULL, O_WriteOnly));
    break;

    default:
    bug("Unexpected command type in allocaux");
    return fd;
  }
  allocchannels(aux->next);
  return fd + 2;
}
