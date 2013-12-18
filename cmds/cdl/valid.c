/**
*
* Title:  CDL Compiler - Validatate Fifo Usage.
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
/* static char *rcsid = "$Header: /hsrc/cmds/cdl/RCS/valid.c,v 1.2 1992/06/11 11:41:24 nickc Exp $"; */

#include "cdl.h"
#include "cdlobj.h"

word countobjects(COMPONENT *component)
{
  channelcount += lenargv((ARGV)component->chanv->channels);
  attribcount += component->attribcount;
  componentcount++;

  return 0;
}

word checkfifo(FIFO *fifo)
{
  if ((fifo->flags & SF_EXTERNAL) == SF_EXTERNAL) return 0;
  if (fifo->usage[READ] == 1 AND fifo->usage[WRITE] == 1) return 0;
  if (fifo->usage[READ] == 0)
    error("stream '%s' has no reader", fifo->name);
  if (fifo->usage[READ] > 1)
    error("stream '%s' has more than one reader", fifo->name);
  if (fifo->usage[WRITE] == 0)
    error("stream '%s' has no writer", fifo->name);
  if (fifo->usage[WRITE] > 1)
    error("stream '%s' has more than one writer", fifo->name);

  return 0;
}

CMD *buildtaskforce(CMD *cmd)
{
  CMD *tf = expandcmd(cmd);

  freecmd(cmd);
  allocchannels(tf);
  componentcount = channelcount = attribcount = 0;
  (void) WalkList(&componentlist, countobjects);
  WalkList(&fifolist, checkfifo);
  return tf;
}


