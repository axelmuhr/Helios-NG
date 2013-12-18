/**
*
* Title:  CDL Compiler - Code Generation.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, 1989, 1990, 1991, 1992 Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

/* static char *rcsid = "$Header: /hsrc/cmds/cdl/RCS/code.c,v 1.6 1993/04/14 17:19:14 nickc Exp $"; */

#include "cdl.h"
#include "cdlobj.h"
#ifndef __HELIOS
#include <unistd.h>
#endif


void putheader(FILE *file)
{
  CDL_HEADER cdl_header;

  cdl_header.type = expanding ? TYPE_3_OBJ : TYPE_2_OBJ;
  cdl_header.nocomponents = componentcount;
  cdl_header.nocstreams = fifocount;
  cdl_header.noistreams = channelcount;
  cdl_header.noattribs = attribcount;
  channelcount = attribcount = 0;
#ifdef __HELIOS
  cdl_header.currentdir.index = putstring(CurrentDir->Name);
#else
  cdl_header.currentdir.index = putstring(getcwd( NULL, 1024 ));
#endif
  cdl_header.tf_name.index = putstring(filename);
  fwrite(&cdl_header, sizeof(CDL_HEADER), 1, file);
}

int putargv(ARGV argv)
{
  int index = stringindex;
  char *arg;

  until ((arg = *argv++) == NULL) (void)putstring(arg);
  return index;
}

word putcomponent(COMPONENT *component, FILE *file)
{
  CDL_COMPONENT cdl_component;

  cdl_component.name.index = putstring(component->path);
  cdl_component.flags = expanding ? TF_INCLUDED : 0;
  cdl_component.toobj = NULL;
  cdl_component.puid.index = putstring(component->puid);
  cdl_component.p_type = component->ptype;
  cdl_component.noattribs = component->attribcount;
  cdl_component.p_attrib.index = attribcount;
  cdl_component.memory = component->memory;
  cdl_component.longevity = component->life;
  cdl_component.time = component->time;
  cdl_component.priority = component->priority;
  cdl_component.nargs = lenargv(component->argv);
  cdl_component.args.index = putargv(component->argv);
  cdl_component.noistreams = lenargv((ARGV)component->chanv->channels);
  cdl_component.istreams.index = channelcount;
  attribcount += component->attribcount;
  channelcount += lenargv((ARGV)component->chanv->channels);
  fwrite(&cdl_component, sizeof(cdl_component), 1, file);

  return 0;
}

word putobject(COMPONENT *component, FILE *file)
{
  FILE *	object;
  word		size;
  int		c;

  if ((object = fopen(component->path, "rb")) == NULL)
    fatal("Unable to open `%s'", component->path);

#ifdef __HELIOS  
  size = GetFileSize(Heliosno(object));
#else
  fseek( object, 0L, SEEK_END );
  size = ftell( object );
  fseek( object, 0L, SEEK_SET );  
#endif
  
  fwrite(&size, sizeof(int), 1, file);
  until ((c = fgetc(object)) == EOF) fputc(c, file);
  fclose(object);

  return 0;
}

word putattrib(ATTRIB *attrib, FILE *file)
{
  CDL_DEV_ATTR cdl_dev_attr;

  cdl_dev_attr.count = attrib->count;
  cdl_dev_attr.attribute.index = putstring(attrib->name);
  fwrite(&cdl_dev_attr, sizeof(CDL_DEV_ATTR), 1, file);

  return 0;
}

word putattribs(COMPONENT *component, FILE *file)
{
  (void) WalkList(&component->attriblist, putattrib, file);

  return 0;
}

void putchannel(FILE *file, int fd, CHANNEL *channel)
{
  CDL_ISTREAM cdl_istream;

  if (channel == STDCHAN)
  {
    cdl_istream.index = -1;
    cdl_istream.mode = (fd == 0) ? O_ReadOnly : O_WriteOnly;
  }
  else
  {
    cdl_istream.index = channel->fifo->index;
    cdl_istream.mode = channel->mode;
  }
  cdl_istream.standard = fd;
  fwrite(&cdl_istream, sizeof(cdl_istream), 1, file);
}

word putchannels(COMPONENT *component, FILE *file)
{
  int fd;

  for (fd = 0; component->chanv->channels[fd] != NULL; fd++)
    putchannel(file, fd, component->chanv->channels[fd]);

  return 0;
}

word putfifo(FIFO *fifo, FILE *file)
{
  CDL_CSTREAM cdl_cstream;

  cdl_cstream.name.index = putstring(fifo->name);
  cdl_cstream.flags = fifo->flags;
  cdl_cstream.count = (WORD)fifo->usage[READ] + (WORD)fifo->usage[WRITE];
  fwrite(&cdl_cstream, sizeof(cdl_cstream), 1, file);

  return 0;
}

void putstringv(FILE *file)
{
  ARGV argv = stringv;
  char *text;

  fwrite(&stringindex, sizeof(int), 1, file);
  until ((text = *argv++) == NULL)
  {
    fprintf(file, "%s", text);
    fputc('\0', file);
  }
}

void putlisting(CMD *cmd, FILE *file)
{
  fprintf(file, "\ncommand:\n\n");
  listcmd(cmd, file);
  fprintf(file, "\n\nlist of components:\n\n");
  (void) WalkList(&componentlist, listcomponent, file);
}

void listdelimitor(TOKEN op, FILE *file)
{
  switch (op)
  {
    case T_SEMICOLON:
    fprintf(file, "; ");
    return;

    case T_AMPERSAND:
    fprintf(file, "& ");
    return;

    case T_AND:
    fprintf(file, "&& ");
    return;

    case T_OR:
    fprintf(file, "|| ");
    return;
  }
}

void listreplicator(REPLICATOR *rep, FILE *file)
{
  BINDV bindv = rep->bindv;
  int i;

  listop(rep->repop, file);
  fprintf(file, "[");
  for (i = 0; i < rep->dim; i++)
  {
    unless (i == 0) fprintf(file, ",");
    fprintf(file, "%d", bindv[i].value);
    unless (bindv[i].name == NULL) fprintf(file, ":%s", bindv[i].name);
  }
  fprintf(file, "] ");
  listcmd(rep->cmd, file);
}

void listsimple(SIMPLE *simple, FILE *file)
{
  if (simple->component == NULL) listargv(simple->argv, file);
  else listargv(simple->component->argv, file);
  listaux(simple->aux, file);
}

void listcmd(CMD *cmd, FILE *file)
{
  DEBUG("listcmd()");
  if (cmd == NULL) return;
  switch (cmd->op)
  {
    case T_REPLICATOR:
    listreplicator((REPLICATOR *)cmd, file);
    return;
   
    case T_PAR:
    case T_PIPE:
    case T_REVPIPE:
    case T_SUBORDINATE:
    case T_INTERLEAVE:
    listcmd(cmd->this, file);
    listop(cmd->op, file);
    listcmd(cmd->next, file);
    return;

    case T_SEMICOLON:
    case T_AMPERSAND:
    case T_AND:
    case T_OR:
    listcmd(cmd->this, file);
    listdelimitor(cmd->op, file);
    listcmd(cmd->next, file);
    return;

    case T_LIST:
    fprintf(file, "( ");
    listcmd(cmd->this, file);
    fprintf(file, ") ");
    listaux(cmd->next, file);
    return;

    case T_SIMPLE:
    listsimple((SIMPLE *)cmd, file);
    return;

    case T_COMMA:
    listcmd(cmd->this, file);
    fprintf(file, ", ");
    listcmd(cmd->next, file);
    return;

    default:
    bug("Unknown command type");
    return;
  }
}

void listaux(CMD *aux, FILE *file)
{
  unless (aux == NULL)
  {
    fprintf(file, "( ");
    listcmd(aux, file);
    fprintf(file, ") ");
  }
}

void listop(TOKEN op, FILE *file)
{
  switch (op)
  {
    case T_PAR:
    fprintf(file, "^^ ");
    return;

    case T_PIPE:
    fprintf(file, "| ");
    return;

    case T_REVPIPE:
    fprintf(file, "|< ");
    return;

    case T_SUBORDINATE:
    fprintf(file, "<> ");
    return;

    case T_INTERLEAVE:
    fprintf(file, "||| ");
    return;

    case T_NULL:
    return;

    default:
    bug("Unknown constructor");
    return;
  }
}

word listcomponent(COMPONENT *component, FILE *file)
{
  int fd;
  
  fprintf(file, "name:\t%s\n", component->name);
  fprintf(file, "code:\t%s\n", component->path);
  listptype(component->ptype, file);
  fprintf(file, "puid:\t%s\n", component->puid);
  fprintf(file, "attribs:\t");
  
  (void) WalkList(&component->attriblist, listattrib, file);
  
  fprintf(file, "\n");
  fprintf(file, "memory:\t%ld\n", component->memory);
  
#ifdef NEVER
  listlife(component->life, file);
  fprintf(file, "time:\t%d\n", component->time);
  fprintf(file, "priority:\t%d\n", component->priority);
#endif
  
  fprintf(file, "streams:\t");
  
  for (fd = 0; component->chanv->channels[fd] != NULL; fd++)
   { if (fd != 0) fprintf(file, ", ");
     listchannel(file, fd, component->chanv->channels[fd]);
   }
  fprintf(file, "\n");
  fprintf(file, "arguments:\t");
  listargv(component->argv, file);
  fprintf(file, "\n\n");

  return 0;
}

void listargv(ARGV argv, FILE *file)
{
  char *arg;

  until ((arg = *argv++) == NULL) fprintf(file, "%s ", arg);
}

word listattrib(ATTRIB *attrib, FILE *file)
{
  fprintf(file, "%s[%d] ", attrib->name, attrib->count);

  return 0;  
}

void listptype(PTYPE ptype, FILE *file)
{
  static char *ptypename[4] =
  {
    "ANY",
    "T212",
    "T414",
    "T800"
  };
  fprintf(file, "ptype:\t%s\n", ptypename[(int)ptype]);
}

#ifdef NEVER
void listlife(LIFE life, FILE *file)
{
  static char *lifename[2] =
  {
    "mortal",
    "immortal"
  };
  fprintf(file, "life:\t%s\n", lifename[(int)life]);
}
#endif

void listchannel(FILE *file, int fd, CHANNEL *channel)
{
  static char *stdname[4] =
  {
    "stdin",
    "stdout",
    "stderr",
#ifdef NEVER    
    "stddbg"
#else
    "unused"
#endif        
  };

  if (channel == STDCHAN)
  {
    if (fd < 4) fprintf(file, " %s(%d) ", stdname[fd], fd);
  }
  else
  {
    FIFO *fifo = channel->fifo;
    int mode = channel->mode;

    if ((mode & O_ReadOnly) == O_ReadOnly) fputc('<', file);
    if ((mode & O_WriteOnly) == O_WriteOnly) fputc('>', file);
    if ((mode & O_Append) == O_Append) fputc('>', file);
    unless ((fifo->flags & SF_EXTERNAL) == SF_EXTERNAL) fputc('|', file);
    fprintf(file, " %s(%d)", fifo->name, fd);
  }
}

int putcode(FILE *file)
{
  putheader(file);
  (void) WalkList(&componentlist, putcomponent, file);
  (void) WalkList(&fifolist, putfifo, file);
  (void) WalkList(&componentlist, putchannels, file);
  (void) WalkList(&componentlist, putattribs, file);
  putstringv(file);
  if (expanding)
    (void) WalkList(&componentlist, putobject, file);
  return(EXIT_SUCCESS);
}
