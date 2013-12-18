/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- clink.c								--
--                                                                      --
--	Low-level control over links.					--
--                                                                      --
--	Author:  BLV 13/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/clink.c,v 1.4 1993/08/11 10:27:50 bart Exp $*/

#include <stdio.h>
#include <syslib.h>
#include <task.h>
#include <codes.h>
#include <nonansi.h>
#include <string.h>
#include <root.h>

#ifndef eq
#define eq ==
#define ne !=
#endif

static void usage(Stream *);
static void do_pending(int link);
static void do_enable(int link);
static void do_disable(int link);

int main(void)
{ Environ	env;
  char		*number;
  char		*option;
  int		link;
        
  if (GetEnv(MyTask->Port, &env) < Err_Null)
   { IOdebug("clink: failed to received environment");
     Exit(0x100);
   }

  if ((env.Strv[0] eq Null(Stream)) ||
      (env.Strv[1] eq Null(Stream)) ||
      (env.Strv[2] eq Null(Stream)))
   { IOdebug("clink: failed to get error stream in environment");
     Exit(0x100);
   }

  if ((env.Argv[0] eq Null(char)) ||
      (env.Argv[1] eq Null(char)) ||
      (env.Argv[2] eq Null(char)) ||
      (env.Argv[3] ne Null(char)))
   usage(env.Strv[2]);

  link = 0;
  for  (number = env.Argv[1]; ; number++)
   { if (('0' <= *number) && (*number <= '9'))
      link = (10 * link) + (*number - '0');
     else
      break;
   }
  if (*number ne '\0') usage(env.Strv[2]);

  { RootStruct	*root = GetRoot();
    LinkInfo	**info = root->Links;
    int		i;
    for (i = 0; i <= link; i++)
     if (*info++ eq Null(LinkInfo))
      usage(env.Strv[2]);
  }      
  

  option = env.Argv[2];
  if ((option[0] ne '-') || (option[1] eq '\0') || (option[2] ne '\0'))
   usage(env.Strv[2]);

  switch(option[1])
   { case 'e' : do_enable( link); break;
     case 'p' : do_pending(link); break;
     case 'd' : do_disable(link); break;
     default  : usage(env.Strv[2]);
   }
  Exit(0);
}  

static void usage(Stream *s)
{ static char *message = "clink: usage, clink <0 | 1 | ...> <-e | -d | -p>\n";
  (void) Write(s, (BYTE *) message, strlen(message), -1);
  Exit(0x100);
}

/**
*** Enable a link. This means making sure that the mode is intelligent,
*** that the state is running, and that the link really gets enabled.
**/
static void do_enable(int link)
{ LinkInfo	info;
  
  LinkData(link, &info);
  if ((info.Mode ne Link_Mode_Intelligent) || 
      (info.State ne Link_State_Running))
   { LinkConf conf;
     conf.Mode   = Link_Mode_Intelligent;
     conf.State  = Link_State_Running;
     conf.Flags  = info.Flags;
     conf.Id     = info.Id;
     if (Configure(conf) ne Err_Null) Exit(0x100);
     if (EnableLink(link) ne Err_Null) Exit(0x100);
   }
  return;
}

/**
*** disabling a link. This means making sure that the link is set to mode
*** null (not connected) or mode dumb (leave it alone).
**/
static void do_disable(int link)
{ LinkInfo	info;
  LinkConf	conf;
  
  LinkData(link, &info);
  if ((info.Mode eq Link_Mode_Null) || (info.Mode eq Link_Mode_Dumb))
   return;
   
  conf.Mode   = Link_Mode_Null;
  conf.State  = Link_State_Null;
  conf.Flags  = info.Flags;
  conf.Id     = info.Id;
  if (Configure(conf) ne Err_Null) Exit(0x100);
  return;
}

/**
*** Pending. This has no effect if the link is already intelligent and
*** running. Otherwise the link is set to intelligent, i.e. there is
*** something interesting on the other side, and dead, i.e. wait for
*** the other side to send a message enabling the link.
**/
static void do_pending(int link)
{ LinkInfo	info;
  LinkConf	conf;

  LinkData(link, &info);
  if ((info.Mode eq Link_Mode_Intelligent) &&
      (info.State eq Link_State_Running))
   return;
  
  conf.Mode   = Link_Mode_Intelligent;
  conf.State  = Link_State_Dead;
  conf.Flags  = info.Flags;
  conf.Id     = info.Id;
  if (Configure(conf) ne Err_Null) Exit(0x100);
  return;
}


