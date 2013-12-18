/**
*
* Title:  Helios Shell - CDL Interface.
*
* Author: Andy England
*
* Date:   June 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/cdl.c,v 1.4 1993/08/16 15:35:50 nickc Exp $
*
**/
#include <stdio.h>
#define _BSD
#include "typedef.h"
#include "cdl.h"
#include "cdlobj.h"

LIST componentlist;
LIST linklist;
ARGV stringv;
int componentcount;
int linkcount;
int channelcount;
int stringindex;

#ifdef HELIOS
void alloccomponents(CMD *);
void allocchannels(CMD *);
void allocaux(CMD *, CMD *);
void locatecmd(char *, char *);
void putcode(FILE *);
void putheader(FILE *);
void putstringv(FILE *);
void putlink(LINK *, FILE *);
int addstring(char *);
LINK *findlink(char *);
LINK *addlink(char *, int);
void freetaskforce(CMD *);
void freelink(LINK *);
CHANNEL *newchannel(LINK *, MODE, int);
void freechannel(CHANNEL *);
void addchannel(COMPONENT *, LINK *, MODE, int);
void addaux(COMPONENT *, LINK *, LINK *);
int getflags(MODE);
int getmode(MODE);
int getstandard(MODE);
void initdata(void);
void freedata(void);
void putcomponent(COMPONENT *, FILE *);
void putchannels(COMPONENT *, FILE *);
void putchannel(CHANNEL *, FILE *);
COMPONENT *newcomponent(char *, ARGV);
void freecomponent(COMPONENT *);
COMPONENT *addcomponent(char *, ARGV);
#else
void alloccomponents();
void allocchannels();
void allocaux();
void locatecmd();
void putcode();
void putheader();
void putstringv();
void putlink();
int addstring();
LINK *findlink();
LINK *addlink();
void freetaskforce();
void freelink();
CHANNEL *newchannel();
void freechannel();
void addchannel();
void addaux();
int getstandard();
void initdata();
void freedata();
void putcomponent();
void putchannels();
void putchannel();
COMPONENT *newcomponent();
void freecomponent();
COMPONENT *addcomponent();
#endif

#ifndef HELIOS
void WalkList(listptr, function, arg)
LIST *listptr;
int (*function)();
long arg;
{
  NODE *node, *next;

  for (node = listptr->head; node->next; node = next)
  {
    next = node->next;
    (*function)(node, arg);
  }
}
#endif

#ifdef HELIOS
int executetaskforce()
{
  FILE *file;
  static int tfcount = 1;
  char tfname[NUMSTR_MAX + 10];
 int pid;


  sprintf(tfname, "/fifo/tf.%d", tfcount++);
  if ((file = fopen(tfname, "wb")) == NULL)
  {
    syserr(tfname);
    recover();
  }
  putcode(file);
  fclose(file);

  if ((pid = vfork()) == 0)
  {
    char *argv[2];

    argv[0] = tfname;
    argv[1] = NULL;
    execv(argv[0], argv);
    if (errno == ENOENT) error(ERR_NOTFOUND, tfname);
      else syserr(NULL);
    _exit(1);
    }
  if (pid == -1)
  {
    syserr(NULL);
    recover();
  }
  return pid;
}
#else
void openfiddle();
void closefiddle();
void createpipe();
void closepipe();
void executecomponent();
void marklink();
void closelink();
void redirect();

int executetaskforce()
{
#ifdef DEBUGGING
  DEBUG("executetaskforce()");
#endif
  openfiddle();
  WalkList(&linklist, createpipe);
  closefiddle();
  WalkList(&componentlist, executecomponent);
  WalkList(&linklist, closepipe);
  until (wait(0) == -1);
  return OK;
}

void openfiddle()
{
  while (open("/dev/null", 1) < 7);
}

void closefiddle()
{
  int i;

  for (i = 3; i < 8; i++) close(i);
}

void createpipe(link)
LINK *link;
{
#ifdef DEBUGGING
  DEBUG("createpipe()");
#endif
  if (pipe(link->fds) == -1) bug("Unable to create pipe");
}

void closepipe(link)
LINK *link;
{
#ifdef DEBUGGING
  DEBUG("closepipe()");
#endif
  close(link->fds[READ]);
  close(link->fds[WRITE]);
}

void executecomponent(component)
COMPONENT *component;
{
 #ifdef DEBUGGING
 DEBUG("executecomponent()");
#endif
  if (fork() == 0)
  {
    closefiddle();
    WalkList(&component->channellist, marklink);
    WalkList(&linklist, closelink);
    WalkList(&component->channellist, redirect);
    execv(component->name, component->argv);
    _exit(1);
  }
}

void marklink(channel)
CHANNEL *channel;
{
  unless (channel->link == NULL) channel->link->name = NULL;
}

void closelink(link)
LINK *link;
{
  unless (link->name == NULL)
  {
    close(link->fds[READ]);
    close(link->fds[WRITE]);
  }
}

void redirect(channel)
CHANNEL *channel;
{
  unless (channel->link == NULL)
  {
    int oldfd = channel->nfd;
    int newfd = channel->link->fds[getstandard(channel->mode)];
    int otherend = channel->link->fds[1 - getstandard(channel->mode)];

    close(otherend);
    unless (oldfd == newfd)
    {
      dup2(newfd, oldfd);
      close(newfd);
    }
  }
}
#endif

int runtaskforce(CMD *cmd)
{
  int pid;

#ifdef DEBUGGING
  DEBUG("runtaskforce()");
#endif
  cmd = dupcmd(cmd);
  initdata();
  alloccomponents(cmd);
  allocchannels(cmd);
  pid = executetaskforce();
  freedata();
  freetaskforce(cmd);
  return pid;
}

void allocredirection(
		      COMPONENT *component,
		      IOINFO *ioinfo )
{
  OPENINFO *openinfo;

  if (ioinfo == NULL) return;
  unless ((openinfo = ioinfo->input) == NULL)
  {
    MODE mode = openinfo->op;

    if (mode == T_SHELLREAD)
    {
      FILE *file;
      char *terminator = openinfo->name;
      char line[LINE_MAX + 1];
      int length = strlen(terminator);

      if ((file = fopen(TEMP_FILE, "w")) == NULL)
      {
      	syserr(TEMP_FILE);
      	recover();
      }
      forever
      {
        unless (getline(line, TRUE))
        {
          fclose(file);
          error(ERR_TERMINATOR, terminator);
          recover();
        }
        if (strnequ(terminator, line, length) AND line[length] == '\n') break;
        fputs(line, file);
      }
      fclose(file);
      addchannel(component, addlink(TEMP_FILE, SF_EXTERNAL), T_READ, 0);
    }
    else
    {
      long flags = (mode == T_READ) ? SF_EXTERNAL : 0;

      addchannel(component, addlink(openinfo->name, flags), mode, 0);
    }
  }
  unless ((openinfo = ioinfo->output) == NULL)
  {
    MODE mode = openinfo->op;
    long flags = (mode == T_WRITEFIFO) ? 0 : SF_EXTERNAL;

#ifdef HELIOS
    flags |= O_Create;
#endif
/* ACE: Needs more here */
    addchannel(component, addlink(openinfo->name, flags), T_WRITE, 1);
    if (openinfo->flags & FLAG_STDERR)
      addchannel(component, addlink(openinfo->name, flags), T_WRITE, 2);
  }
}

void alloccomponents(CMD *cmd)
{
  #ifdef DEBUGGING
DEBUG("alloccomponents()");
#endif
  if (cmd == NULL) return;
  if (cmd->op == T_SIMPLE)
  {
    ARGV argv = (ARGV)cmd->This;

    argv = fullsub(argv);
    unless (getvar("echo") == NULL) fputargv(stderr, argv, TRUE);
    cmd->This = (CMD *)addcomponent(argv[0], argv);
    alloccomponents(cmd->next);
    return;
  }
  alloccomponents(cmd->This);
  alloccomponents(cmd->next);
}

void leftcommunicate(
		     CMD *cmd,
		     LINK *input,
		     LINK *output )
{
  CMD *This = cmd->This;
  CMD *next = cmd->next;

#ifdef DEBUGGING
  DEBUG("leftcommunicate()");
#endif
  switch (cmd->op)
  {
    case T_SIMPLE:
    unless (input == NULL)
      addchannel((COMPONENT *)This, input, T_READFIFO, 0);
    unless (output == NULL)
      addchannel((COMPONENT *)This, output, T_WRITEFIFO, 1);
    return;

    case T_LIST:
    leftcommunicate(This, input, output);
    return;

    case T_SEMICOLON:
    case T_AMPERSAND:
    case T_AND:
    case T_OR:
    leftcommunicate(This, input, output);
    leftcommunicate(next, input, output);
    return;

    case T_COMMA:
    case T_PAR:
    leftcommunicate(This, input, output);
    leftcommunicate(next, input, output);
    return;

    case T_PIPE:
    case T_REVPIPE:
    case T_SUBORDINATE:
    case T_FARM:
    leftcommunicate(next, input, output);
    return;
  }
}

void rightcommunicate(
		      CMD *cmd,
		      LINK *input,
		      LINK *output )
{
  CMD *This = cmd->This;
  CMD *next = cmd->next;

#ifdef DEBUGGING
  DEBUG("rightcommunicate()");
#endif
  switch (cmd->op)
  {
    case T_SIMPLE:
    unless (input == NULL)
      addchannel((COMPONENT *)This, input, T_READFIFO, 0);
    unless (output == NULL)
      addchannel((COMPONENT *)This, output, T_WRITEFIFO, 1);
    return;

    case T_LIST:
    rightcommunicate(This, input, output);
    return;

    case T_SEMICOLON:
    case T_AMPERSAND:
    case T_AND:
    case T_OR:
    rightcommunicate(This, input, output);
    leftcommunicate(next, input, output);
    return;

    case T_COMMA:
    case T_PAR:
    rightcommunicate(This, input, output);
    rightcommunicate(next, input, output);
    return;

    case T_PIPE:
    case T_REVPIPE:
    case T_SUBORDINATE:
    case T_FARM:
    rightcommunicate(This, input, output);
    return;
  }
}

void auxcommunicate(
		    CMD *cmd,
		    LINK *input,
		    LINK *output )
{
  CMD *This = cmd->This;
  CMD *next = cmd->next;

 #ifdef DEBUGGING
 DEBUG("auxcommunicate()");
#endif
  switch (cmd->op)
  {
    case T_SIMPLE:
    addaux((COMPONENT *)This, input, output);
    return;

    case T_LIST:
    auxcommunicate(This, input, output);
    return;

    case T_SEMICOLON:
    case T_AMPERSAND:
    case T_AND:
    case T_OR:
    auxcommunicate(This, input, output);
    auxcommunicate(next, input, output);
    return;

    case T_COMMA:
    case T_PAR:
    auxcommunicate(This, input, output);
    auxcommunicate(next, input, output);
    return;

    case T_PIPE:
    case T_REVPIPE:
    case T_SUBORDINATE:
    case T_FARM:
    auxcommunicate(next, input, output);
    return;
  }
}

void allocchannels(CMD *cmd)
{
  CMD *This = cmd->This;
  CMD *next = cmd->next;
  LINK *link1, *link2;

#ifdef DEBUGGING
  DEBUG("allocchannels(op = %d)", cmd->op);
#endif
  switch (cmd->op)
  {
    case T_COMMA:
    case T_PAR:
    break;

    case T_PIPE:
    unless (This == NULL)
    {
      link1 = addlink(NULL, 0);
      leftcommunicate(This, NULL, link1);
      rightcommunicate(next, link1, NULL);
    }
    break;

    case T_REVPIPE:
    unless (cmd == NULL)
    {
      link1 = addlink(NULL, 0);
      leftcommunicate(This, link1, NULL);
      rightcommunicate(next, NULL, link1);
    }
    break;

    case T_SUBORDINATE:
    unless (cmd == NULL)
    {
      link1 = addlink(NULL, 0);
      link2 = addlink(NULL, 0);
      auxcommunicate(This, link2, link1);
      rightcommunicate(next, link1, link2);
    }
    break;

    case T_FARM:
    unless (cmd == NULL)
    {
      link1 = addlink(NULL, 0);
      link2 = addlink(NULL, 0);
      auxcommunicate(This, link2, link1);
      rightcommunicate(next, link1, link2);
    }
    return;

    case T_SEMICOLON:
    case T_AMPERSAND:
    if (next == NULL) return;
    case T_AND:
    case T_OR:
    break;

    case T_SIMPLE:
    allocredirection((COMPONENT *)This, cmd->ioinfo);
    case T_LIST:
    unless (next == NULL) allocaux(cmd, next);
    return;

    default:
    bug("Unexpected command type");
    return;
  }
  unless (This == NULL) allocchannels(This);
  allocchannels(next);
}

void allocaux(
	      CMD *cmd,
	      CMD *aux )
{
  CMD *This = aux->This;
  CMD *next = aux->next;
  LINK *link1, *link2;

#ifdef DEBUGGING
  DEBUG("allocaux(op = %d)", aux->op);
#endif
  switch (aux->op)
  {
    case T_COMMA:
    case T_PAR:
    allocaux(cmd, This);
    allocaux(cmd, next);
    return;

    case T_PIPE:
    link1 = addlink(NULL, 0);
    auxcommunicate(cmd, NULL, link1);
    rightcommunicate(next, link1, NULL);
    break;

    case T_REVPIPE:
    link1 = addlink(NULL, 0);
    auxcommunicate(cmd, link1, NULL);
    rightcommunicate(next, NULL, link1);
    break;

    case T_SUBORDINATE:
    link1 = addlink(NULL, 0);
    link2 = addlink(NULL, 0);
    auxcommunicate(cmd, link2, link1);
    rightcommunicate(next, link1, link2);
    break;

    case T_FARM:
    bug("FARM constructor currently not supported");
    return;

    default:
    bug("Unexpected command type");
  }
  allocchannels(next);
}

void locatecmd(
	       char *filename,
	       char *name )
{
  ARGV pathargv;
  char *path;

  unless ((pathargv = findvar("path")) == NULL)
  {
    until ((path = *pathargv++) == NULL)
    {
      if (lookforcmd(path, name))
      {
#ifdef HELIOS
      	Object *object;

        formfilename(filename, path, name);
      	unless ((object = Locate(CurrentDir, filename)) == NULL)
      	{
       	  Close(object);
      	  return;
        }
#else
        formfilename(filename, path, name);
        return;
#endif
      }
    }
  }
  error(ERR_NOTFOUND, name);
  recover();
}

void putcode(FILE *file)
{
#ifdef DEBUGGING
  DEBUG("putcode()");
#endif
  stringv = nullargv();
  stringindex = 0;
  putheader(file);
  WalkList(&componentlist, (WordFnPtr)putcomponent, file);
  WalkList(&linklist, (WordFnPtr) putlink, file);
  WalkList(&componentlist,(WordFnPtr)putchannels, file);
  putstringv(file);
  freeargv(stringv);
}

void putheader(FILE *file)
{
  CDL_HEADER cdl_header;

#ifdef DEBUGGING
  DEBUG("putheader()");
#endif
  cdl_header.type = TYPE_2_OBJ;
  cdl_header.nocomponents = componentcount;
  cdl_header.nocstreams = linkcount;
  cdl_header.noistreams = channelcount;
  channelcount = 0;
  cdl_header.noattribs = 0;
#ifdef HELIOS
  cdl_header.currentdir.index = addstring(CurrentDir->Name);
#else
  {
    char path[1025];

    getwd(path);
    cdl_header.currentdir.index = addstring(path);
  }
#endif
  cdl_header.tf_name.index = addstring("taskforce");
  fwrite(&cdl_header, sizeof(CDL_HEADER), 1, file);
}

void putcomponent(
		  COMPONENT *component,
		  FILE *file )
{
  CDL_COMPONENT cdl_component;

#ifdef DEBUGGING
  DEBUG("putcomponent()");
#endif
  cdl_component.name.index = addstring(component->name);
  cdl_component.flags = 0;
  cdl_component.toobj = NULL;
  cdl_component.puid.index = addstring("");
  cdl_component.p_type = ANY_PROCESSOR;
  cdl_component.noattribs = 0;
  cdl_component.p_attrib.index = 0;
  cdl_component.memory = 0;
  cdl_component.longevity = IMMORTAL;
  cdl_component.time = 44236800;
  cdl_component.priority = 1;
  cdl_component.nargs = lenargv(component->argv);
  cdl_component.args.index = addargv(component->argv);
  cdl_component.noistreams = component->channelcount;
  cdl_component.istreams.index = channelcount;
  channelcount += component->channelcount;
  fwrite(&cdl_component, sizeof(cdl_component), 1, file);
}

void putchannels(
		 COMPONENT *component,
		 FILE *file )
{
  WalkList(&component->channellist, (WordFnPtr)putchannel, file);
}

void putchannel(
		CHANNEL *channel,
		FILE *file )
{
  CDL_ISTREAM cdl_istream;

  if (channel->link == NULL) cdl_istream.index = -1;
  else cdl_istream.index = channel->link->number;
  cdl_istream.mode = getmode(channel->mode);
  cdl_istream.standard = channel->fd;
  fwrite(&cdl_istream, sizeof(cdl_istream), 1, file);
}


void putlink(
	     LINK *link,
	     FILE *file )
{
  CDL_CSTREAM cdl_cstream;

  cdl_cstream.name.index = addstring(link->name);
  cdl_cstream.flags = link->flags;
  cdl_cstream.count = link->count;
  fwrite(&cdl_cstream, sizeof(cdl_cstream), 1, file);
}

int addstring(char *text)
{
  int index = stringindex;

#ifdef DEBUGGING
  DEBUG("addstring()");
#endif
  stringv = addword(stringv, text);
  stringindex += strlen(text) + 1;
  return index;
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

int addargv(ARGV argv)
{
  char *arg;
  int index = stringindex;

  until ((arg = *argv++) == NULL) ignore addstring(arg);
  return index;
}

void freetaskforce(CMD *cmd)
{
 #ifdef DEBUGGING
 DEBUG("freetaskforce()");
#endif
  if (cmd == NULL) return;
  unless (cmd->op == T_SIMPLE) freetaskforce(cmd->This);
  freetaskforce(cmd->next);
  freeioinfo(cmd->ioinfo);
  freememory((int *)cmd);
}

LINK *findlink(char *name)
{
  LINK *link;

#ifdef DEBUGGING
  DEBUG("findlink()");
#endif
  for (link = (LINK *)linklist.Head; link->next; link = link->next)
  {
    if (strequ(name, link->name)) return link;
  }
  return NULL;
}

LINK *addlink(
	      char *name,
	      int flags )
{
  LINK *link;

#ifdef DEBUGGING
  DEBUG("addlink()");
#endif
  if (name == NULL OR (link = findlink(name)) == NULL)
  {
    link = new(LINK);
    if (name == NULL)
    {
      char buffer[20];

      ignore sprintf(buffer, "stream%d", linkcount);
      name = buffer;
    }
    link->name = strdup(name);
    link->flags = flags;
    link->count = 0;
    link->number = linkcount++;
    AddTail(&linklist, (NODE *)link);
  }
  link->count++;
  return link;
}

void freelink(LINK *link)
{
#ifdef DEBUGGING
  DEBUG("freelink()");
#endif
  freememory((int *)link->name);
  freememory((int *)link);
}

CHANNEL *newchannel(
		    LINK *link,
		    MODE mode,
		    int fd )
{
  CHANNEL *channel = new(CHANNEL);

#ifdef DEBUGGING
  DEBUG("newchannel()");
#endif
  channel->next = NULL;
  channel->prev = NULL;
  channel->link = link;
  channel->mode = mode;
  channel->fd = fd;
  channelcount++;
  return channel;
}

void freechannel(CHANNEL *channel)
{
#ifdef DEBUGGING
  DEBUG("freechannel()");
#endif
  freememory((int *)channel);
}

void addchannel(
		COMPONENT *component,
		LINK *link,
		MODE mode,
		int fd )
{
  if (fd < component->channelcount)
  {
    CHANNEL *channel;
    int count = 0;

    for (channel = (CHANNEL *)component->channellist.Head; channel->next;
         channel = channel->next)
    {
      if (count++ == fd)
      {
        unless (channel->link == NULL)
        {
          if (getstandard(mode) == 0) error(ERR_INPUT, NULL);
          else error(ERR_OUTPUT, NULL);
          recover();
        }
        channel->link = link;
        channel->mode = mode;
        channel->fd = fd;
        return;
      }
    }
    bug("Did not find stream");
  }
  while (component->channelcount < fd)
  {
    AddTail(&component->channellist, (NODE *)newchannel(NULL, T_WRITE, 3));
    component->channelcount++;
  }
  AddTail(&component->channellist, (NODE *)newchannel(link, mode, fd));
  component->channelcount++;
}

void addaux(
	    COMPONENT *component,
	    LINK *input,
	    LINK *output )
{
  int number = component->auxnumber;

#ifdef DEBUGGING
  DEBUG("addaux()");
#endif
  component->auxnumber += 2;
  unless (input == NULL) addchannel(component, input, T_READFIFO, number);
  unless (output == NULL) addchannel(component, output, T_WRITEFIFO, number + 1);
}
  
void initdata()
{
  InitList(&componentlist);
  InitList(&linklist);
  componentcount = 0;
  linkcount = 0;
  channelcount = 0;
}

void freedata()
{
  WalkList(&componentlist, (WordFnPtr)freecomponent);
  WalkList(&linklist, (WordFnPtr)freelink);
}

COMPONENT *newcomponent(
			char *name,
			ARGV argv )
{
  COMPONENT *component = new(COMPONENT);

 #ifdef DEBUGGING
 DEBUG("newcomponent()");
#endif
  component->next = NULL;
  component->prev = NULL;
  component->name = strdup(name);
  component->argv = argv;
  component->channelcount = 0;
  InitList(&component->channellist);
  component->auxnumber = 4;
  addchannel(component, NULL, T_READ, 0);
  addchannel(component, NULL, T_WRITE, 1);
  addchannel(component, NULL, T_WRITE, 2);
  addchannel(component, NULL, T_WRITE, 3);
  componentcount++;
  return component;
}

void freecomponent(COMPONENT *component)
{
 #ifdef DEBUGGING
 DEBUG("freecomponent()");
#endif
  freememory((int *)component->name);
  freeargv(component->argv);
  WalkList(&component->channellist, (WordFnPtr)freechannel);
  freememory((int *)component);
}

COMPONENT *addcomponent(
			char *name,
			ARGV argv )
{
  COMPONENT *component;
  char path[PATH_MAX + 1];
  char interp[PATH_MAX + 1];

#ifdef DEBUGGING
  DEBUG("addcomponent()");
#endif
  locatecmd(path, name);
  if (getinterpreter(interp, path))
  {
    locatecmd(path, interp);
    argv = prefixword(argv, interp);
  }
  component = newcomponent(path, argv);
  AddTail(&componentlist, (NODE *)component);
  return component;
}

int getmode(MODE mode)
{
  switch (mode)
  {
    case T_READ:
    case T_READFIFO:
    return O_ReadOnly;

    case T_WRITE:
    case T_WRITEFIFO:
    return O_WriteOnly;

    case T_READWRITE:
    return O_ReadWrite;

    case T_APPEND:
    return O_WriteOnly | O_Append;

    default:
    bug("Unknown mode");
    return 0;
  }
}

int getstandard(MODE mode) 
{
  if (mode == T_READ OR mode == T_READFIFO) return READ;
  return WRITE;
}

