/* $Header: /hsrc/cmds/shell/RCS/cdl.h,v 1.4 1993/08/12 15:56:29 nickc Exp $ */

#ifndef HELIOS
#define O_ReadOnly  1
#define o_WriteOnly 2
#define o_ReadWrite 3
#define O_Truncate  0x300
extern char *getenv();
#endif
#define T_READWRITE T_SUBORDINATE

#include "typedef.h"

typedef enum
{
  MORTAL,
  IMMORTAL
} LONGEVITY;

typedef enum
{
  ANY_PROCESSOR,
  T212,
  T414,
  T800
} PROCESSOR;

typedef struct link
{
  struct link *next;
  struct link *prev;
  char *name;
  int flags;
  int count;
  int number;
#ifdef UNIX
  int fds[2];
#endif
} LINK;

typedef struct channel
{
  struct channel *next;
  struct channel *prev;
  LINK *link;
  MODE mode;
  int fd;
} CHANNEL;

typedef struct component
{
  struct component *next;
  struct component *prev;
  char *name;
  ARGV argv;
  int channelcount;
  LIST channellist;
  int auxnumber;
} COMPONENT;
