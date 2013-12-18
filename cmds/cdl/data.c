/**
*
* Title:  CDL Compiler - Data Structure Support.
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
/* static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/cdl/RCS/data.c,v 1.4 1994/03/13 15:45:42 nickc Exp $"; */

#define xxSHOWMEM

#include "cdl.h"

#define MAX_KEYWORDS 11

KEYWORD keywords[MAX_KEYWORDS] =
{
  "component", T_COMPONENT,
  "processor", T_PTYPE,
  "puid",      T_PUID,
  "attrib",    T_ATTRIB,
  "memory",    T_MEMORY,
  "life",      T_LIFE,
  "time",      T_TIME,
  "priority",  T_PRIORITY,
  "streams",   T_STREAMS,
  "code",      T_CODE,
  "argv",      T_ARGV
};

LIST templatelist;
LIST componentlist;
LIST fifolist;
ARGV stringv;
int componentcount;
int attribcount;
int fifocount;
int channelcount;
int stringindex;

int totalmem = 0;

KEYWORD *findkeyword(char *name)
{
  int i;

  for (i = 0; i < MAX_KEYWORDS; i++)
  {
    if (strequ(name, keywords[i].name)) return &keywords[i];
  }
  return NULL;
}

	/* BLV - added some +1s to the channel vector to ensure that	*/
	/* there is always a NULL channel at the end.			*/
CHANV newchanv(void)
{
  CHANV chanv = newmem(sizeof(CHANLIST));
  int fd;

  chanv->channels = newmem(sizeof(CHANNEL *) * (CHAN_DEFAULT + 1));
  memset(chanv->channels, 0, (CHAN_DEFAULT + 1) * sizeof(CHANNEL *));
  for (fd = 0; fd < 4; fd++) chanv->channels[fd] = STDCHAN;
  chanv->size = CHAN_DEFAULT;
  return chanv;
}

void resizechanv(CHANV chanv,int fd)
{ int newsize = (fd + 16) & 0x7FFFFFF0;	/* round to next 16 */
  CHANNEL **channels = newmem((newsize + 1) * sizeof(CHANNEL *));
  
  memset(channels, 0, (newsize + 1) * sizeof(CHANNEL *));
  memcpy(channels, chanv->channels, chanv->size * sizeof(CHANNEL *));
  freemem(chanv->channels);
  chanv->channels = channels;
  chanv->size = newsize;
}

void freechanv(CHANV chanv)
{
  int fd;

  DEBUG("freechanv()");
  for (fd = 0; chanv->channels[fd] != NULL; fd++)
    unless (chanv->channels[fd] == STDCHAN) freechannel(chanv->channels[fd]);
  freemem(chanv->channels);
  freemem(chanv);
}

BINDV newbindv(void)
{
  BINDV bindv = newmem(sizeof(BINDING) * BIND_MAX);
  int i;

  for (i = 0; i < BIND_MAX; i++)
  {
    bindv[i].name = NULL;
    bindv[i].value = 0;
  }
  return bindv;
}

void freebindv(BINDV bindv)
{
  int i;

  DEBUG("freebindv()");
  for (i = 0; i < BIND_MAX; i++)
    unless (bindv[i].name == NULL) freemem(bindv[i].name);
  freemem(bindv);
}

REPLICATOR *newreplicator(int dim, BINDV bindv)
{
  REPLICATOR *replicator = new(REPLICATOR);
  Memory(replicator);
  
  replicator->op = T_REPLICATOR;
  replicator->repop = 0;
  replicator->cmd = NULL;
  replicator->dim = dim;
  replicator->bindv = bindv;
  return replicator;
}

void freereplicator(REPLICATOR *replicator)
{
  DEBUG("freereplicator()");
  freecmd(replicator->cmd);
  freebindv(replicator->bindv);
  freemem(replicator);
}

CMD *newcmd(TOKEN op, CMD *this, CMD *next)
{
  CMD *cmd = new(CMD);
  Memory(cmd);
  
  cmd->op = op;
  cmd->this = this;
  cmd->next = next;
  return cmd;
}

void freecmd(CMD *cmd)
{
  DEBUG("freecmd()");
  if (cmd == NULL) return;
  if (cmd->op == T_SIMPLE) freesimple((SIMPLE *)cmd);
  else if (cmd->op == T_REPLICATOR) freereplicator((REPLICATOR *)cmd);
  else
  {
    freecmd(cmd->this);
    freecmd(cmd->next);
    freemem(cmd);
  }
}

SIMPLE *newsimple(ARGV argv, ARGV subv, CHANV chanv)
{
  SIMPLE *simple = new(SIMPLE);
  Memory(simple);
  
  simple->op = T_SIMPLE;
  simple->component = NULL;
  simple->aux = NULL;
  simple->argv = argv;
  simple->subv = subv;
  simple->chanv = chanv;
  return simple;
}

void freesimple(SIMPLE *simple)
{
  DEBUG("freesimple()");
  unless (simple->argv == NULL) freeargv(simple->argv);
  unless (simple->subv == NULL) freeargv(simple->subv);
  unless (simple->chanv == NULL) freechanv(simple->chanv);
  freecmd(simple->aux);
  freemem(simple);
}

int getstd(int mode)
{
  if ((mode & O_ReadOnly) == O_ReadOnly) return READ;
  return WRITE;
}

char *inventname(void)
{
  static int namecount;
  static char name[NUMSTR_MAX + 7];

  sprintf(name, "stream%d", namecount++);
  return name;
}

FIFO *findfifo(char *name)
{
  FIFO *fifo;

  DEBUG("findfifo()");
  for (fifo = (FIFO *)fifolist.Head; fifo->next; fifo = fifo->next)
  {
    if (strequ(name, fifo->name)) return fifo;
  }
  return NULL;
}

FIFO *newfifo(char *name)
{
  FIFO *fifo = new(FIFO);
  Memory(fifo);
  
  fifo->name = strdup(name);
  fifo->flags = 0;
  fifo->usage[READ]  = 0;
  fifo->usage[WRITE] = 0;
  fifo->index = fifocount++;
  fifo->next = fifo->prev = (FIFO *)&fifo->next;
  AddTail(&fifolist, (NODE *)fifo);
  return fifo;
}

FIFO *usefifo(char *name, int mode)
{
  FIFO *fifo;

  DEBUG("usefifo()");
  if ((fifo = findfifo(name)) == NULL) fifo = newfifo(name);
  fifo->usage[getstd(mode)]++;
  fifo->flags |= (mode & Mask_Fifo);
  return fifo;
}

word
freefifo(FIFO *fifo)
{
  unless (fifo->name == NULL) freemem(fifo->name);
  freemem(fifo);

  return 0;  
}

word
freeattrib(ATTRIB *attrib)
{
  freemem(attrib->name);
  freemem(attrib);

  return 0;
}

void addattrib(COMPONENT *component, char *name, int count)
{
  ATTRIB *attrib = new(ATTRIB);
  Memory(attrib);
  
  DEBUG("addattrib()");
  attrib->name = strdup(name);
  attrib->count = count;
  component->attribcount++;
  attrib->next = attrib->prev = (ATTRIB *)&attrib->next;
  AddTail(&component->attriblist, (NODE *)attrib);
}

static word
copyattrib(ATTRIB *attrib, COMPONENT *component)
{
  DEBUG("copyattrib()");
  addattrib(component, attrib->name, attrib->count);

  return 0;
}

int putstring(char *text)
{
  int index = stringindex;

  stringv = addword(stringv, text);
  stringindex += strlen(text) + 1;
  return index;
}

CHANNEL *newchannel(char *name, ARGV subv, int mode)
{
  CHANNEL *channel = new(CHANNEL);
  Memory(channel);
  
  DEBUG("newchannel()");
  channel->next = (struct channel *) &channel->next;
  channel->prev = (struct channel *) &channel->next;
  channel->name = strdup(name);
  channel->subv = subv;
  channel->fifo = NULL;
  channel->mode = mode;
  return channel;
}

void freechannel(CHANNEL *channel)
{
  DEBUG("freechannel()");
  freemem(channel->name);
  unless (channel->subv == NULL) freeargv(channel->subv);
  freemem(channel);
}

void addchannel(CHANV chanv, int fd, CHANNEL *channel)
{
  if (fd >= chanv->size) resizechanv(chanv, fd);

  unless (chanv->channels[fd] == NULL OR chanv->channels[fd] == STDCHAN)
  { 
    if (getstd(channel->mode) == READ) error("Ambigous input redirect");
    else error("Ambigous output redirect");
    freechannel(channel);
    return;
  }
  chanv->channels[fd] = channel;
  while (--fd >= 0) if (chanv->channels[fd] == NULL) chanv->channels[fd] = STDCHAN;
}

void mergechanv(CHANV chanv, CHANV xchanv)
{
  int fd;

  for (fd = 0; xchanv->channels[fd] != NULL; fd++)
  {
    CHANNEL *channel;

    unless ((channel = xchanv->channels[fd]) == STDCHAN)
    {
      addchannel(chanv, fd, newchannel(channel->name, NULL, channel->mode));
    }
  }
  freechanv(xchanv);
}

void addchanv(COMPONENT *component, CHANV chanv)
{
  freechanv(component->chanv);
  component->chanv = chanv;
}

void addargv(COMPONENT *component, ARGV argv)
{
  freeargv(component->argv);
  component->argv = argv;
}

void initdata()
{
  DEBUG("initdata()");
  InitList(&templatelist);
  InitList(&componentlist);
  InitList(&fifolist);
  fifocount = 0;
  stringindex = 0;
  stringv = nullargv();
}

void freedata()
{
  DEBUG("freedata()");
  WalkList(&templatelist, freecomponent);
  WalkList(&componentlist, freecomponent);
  WalkList(&fifolist, freefifo);
  freeargv(stringv);
  if (memorycheck AND totalmem != 0) 
    fprintf(stderr, "Memory used = %d.\n", totalmem);
}

COMPONENT *newcomponent(char *name)
{
  COMPONENT *component = new(COMPONENT);
  Memory(component);
  
  DEBUG("newcomponent()");
  component->next = (COMPONENT *) &component->next;
  component->prev = (COMPONENT *) &component->next;
  component->name = strdup(name);
  component->path = strdup(name);
  component->argv = nullargv();
  component->chanv = newchanv();
  component->ptype = ANY_PROCESSOR;
  component->puid = strdup("");
  component->memory = 0;
  component->life = IMMORTAL;
  component->time = 44236800;
  component->priority = 1;
  component->attribcount = 0;
  InitList(&component->attriblist);
  component->subv = NULL;
  return component;
}

void addsubnames(COMPONENT *component, ARGV subv)
{
  component->subv = subv;
}

word freecomponent(COMPONENT *component)
{
  DEBUG("freecomponent()");
  freemem(component->name);
  freemem(component->path);
  freeargv(component->argv);
  freechanv(component->chanv);
  freemem(component->puid);
  WalkList(&component->attriblist, freeattrib);
  unless (component->subv == NULL) freeargv(component->subv);
  freemem(component);

  return 0;
}

COMPONENT *copycomponent(COMPONENT *component, int *subv, CHANV chanv)
{
  COMPONENT *dup = newcomponent(component->name);
  int n;

  DEBUG("copycomponent()");
  addpath(dup, component->path);
  n = bindnames(component->subv, subv);
  unless (component->argv[0] == NULL) addargv(dup, expandargv(component->argv));
  mergechanv(chanv, expandchanv(component->chanv));
  addchanv(dup, chanv);
  unbindnames(n);
  dup->ptype = component->ptype;
  addpuid(dup, component->puid);
  dup->memory = component->memory;
  dup->life = component->life;
  dup->time = component->time;
  dup->priority = component->priority;
  WalkList(&component->attriblist, copyattrib, dup);
  return dup;
}

COMPONENT *findtemplate(char *name)
{
  COMPONENT *component;

  DEBUG("findtemplate()");
  for (component = (COMPONENT *)templatelist.Head; component->next;
       component = component->next)
  {
    if (strequ(component->name, name)) return component;
  }
  return NULL;
}

COMPONENT *newtemplate(char *name)
{
  COMPONENT *component;

  DEBUG("newtemplate()");
  unless ((component = findtemplate(name)) == NULL)
  {
    error("Redeclaration of component `%s'", name);
    Remove((Node *)component);
    freecomponent(component);
  }
  component = newcomponent(name);
  AddTail(&templatelist, (NODE *)component);
  return component;
}

COMPONENT *usecomponent(ARGV argv, int *subv, CHANV chanv)
{
  COMPONENT *component;

  DEBUG("usecomponent()");
  if ((component = findtemplate(argv[0])) == NULL)
  {
    component = newcomponent(argv[0]);
    addchanv(component, chanv);
  }
  else component = copycomponent(component, subv, chanv);
  AddTail(&componentlist, (NODE *)component);
  addargv(component, argv);
  if (locating)
  {
    char *path;

    if ((path = locatecmd(component->path)) == NULL)
      fatal("Command '%s' not found", component->path);
    addpath(component, path);
  }
  return component;
}

void addpuid(COMPONENT *component, char *name)
{
  freemem(component->puid);
  component->puid = strdup(name);
}

void addpath(COMPONENT *component, char *path)
{
  freemem(component->path);
  component->path = strdup(path);
}

ARGV nullargv(void)
{
  ARGV argv = (ARGV)newmem(sizeof(char *) * ARGV_MAX);

  argv[0] = 0;
  return argv;
}

ARGV addword(ARGV argv, char *arg)
{
  int length = lenargv(argv);
  int i;

  if (((length + 1) % ARGV_MAX) == 0)
  {
    ARGV newargv = (ARGV)newmem(sizeof(char *) * (length + 1 + ARGV_MAX));

    for (i = 0; i < length; i++) newargv[i] = argv[i];
    freemem(argv);
    argv = newargv;
  }
  argv[length++] = strdup(arg);
  argv[length] = 0;
  return argv;
}

ARGV dupargv(ARGV argv)
{
  ARGV dup = nullargv();
  char *arg;

  until ((arg = *argv++) == NULL) dup = addword(dup, arg);
  return dup;
}

int lenargv(ARGV argv)
{
  int length = 0;

  until (argv[length] == NULL) length++;
  return length;
}

void freeargv(ARGV argv)
{
  DEBUG("freeargv()");
  unless (argv == NULL)
  {
    char *arg;
    int i = 0;

    until ((arg = argv[i++]) == NULL) freemem(arg);
    freemem(argv);
  }
}

char *strdup(char *s)
{ int  len = strlen(s) + 1;
  char *d;
  
  d = (char *)newmem(len);
  strcpy(d, s);
  return d;
}

void *newmem(int size)
{
  int *mem = (int *)malloc(size + sizeof(int));
  
  if (mem == NULL) fatal("Out of memory");
  mem[0] = size;
  totalmem += size;
  return mem + 1;
}

void myfree(int *);

void freemem(void *ptr)
{
  int *	mem = ptr;
  
  mem--;
  totalmem -= mem[0];

  free(mem);
}

#ifdef NEVER
typedef struct Memb {
	word		Size;
	struct Memb	*next;
} Memb;

#define CHECKTAG 0xaabbccdd

void myfree(int *x)
{ Memb *m = ((Memb *) x) - 1;
  word msize = (m->Size < 0) ? -m->Size : m->Size;
  word size  = (msize & ~1);
  word wsize  = size / 4;
  word *wm   = (word *) m;
  
  if ( (msize & 1) != 0)
   { IOdebug("myfree, double free attempt on %x", m); Delay(2 * 60 * OneSec); IOdebug("continuing"); }
   
  if (wm[wsize-1] != (word) m)
   { IOdebug("myfree, end tags not set on %x", m); Delay(2 * 60 * OneSec); IOdebug("continuing"); }

  if (wm[wsize-2] != (word) m)
   { IOdebug("myfree, 2 end tags not set on %x", m); Delay(2 * 60 * OneSec); IOdebug("continuing"); }
  free(x);
} 
#endif
