/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- rboot.c								--
--                                                                      --
--	Boot a processor.						--
--                                                                      --
--	Author:  BLV 13/8/90						--
--                                                                      --
------------------------------------------------------------------------*/

static char *rcsid = "$Header: /users/bart/hsrc/network/TRAN/RCS/rboot.c,v 1.4 1991/05/18 12:05:19 bart Exp $";

#include <stdio.h>
#include <syslib.h>
#include <task.h>
#include <codes.h>
#include <nonansi.h>
#include <string.h>
#include <root.h>
#include <config.h>
#include <link.h>
#include "private.h"
#include "rmlib.h"
#include "netaux.h"

#ifndef eq
#define eq ==
#define ne !=
#endif

/**
*** This program can be compiled with various options.
*** 1) -DPARSYTEC, assert the Parsytec reset scheme
*** 2) -DHANDBOOT, use a manual bootstrap system rather than the kernel
***    one. Handboot should be the default from now on.
**/

#ifdef PARSYTEC
static char *ProgName = "pa_rboot";
#else
static char *ProgName = "rboot";
#endif

#ifdef HANDBOOT
static int My_BootLink(word link, void *image, Config *config, word confsize);
#define BootLink	My_BootLink
#endif

static void	usage(void);
static void	report(char *);
static Config	*build_config(char *my_name, char *its_name, char *link_modes,
				int memory_limit, int *confsize);
static void	*determine_image(void);
static void	init_link(int);
static void	set_link(int);
static int	transputer_check_link(int);
static Stream	*error;
static int	target_link = -1;
#ifdef PARSYTEC
static void	Parsytec_Reset(int link);
#endif

int main(void)
{ Environ	env;
  char		*temp;
  int		link;
  char		*my_name;
  char		*its_name;
  char		*link_modes;
  int		memory_limit;
  int		config_size;
  Config	*config;
  void		*image;
  int		rc;
  int		count;
      
  if (GetEnv(MyTask->Port, &env) < Err_Null)
   { IOdebug("%s: failed to receive environment", ProgName);
     Exit(0x100);
   }

  if ((env.Strv[0] eq Null(Stream)) ||
      (env.Strv[1] eq Null(Stream)) ||
      (env.Strv[2] eq Null(Stream)))
   { IOdebug("%s: failed to get error stream in environment");
     Exit(0x100);
   }

  error = env.Strv[2];

  count = 1;  
  temp = env.Argv[count];
  if (temp eq Null(char)) usage(); 
  if (temp[0] eq '-')
   { unless((temp[1] eq 'l') || (temp[1] eq 'L')) usage();
     unless((temp[2] >= '0') && (temp[2] <= '3')) usage();
     target_link = temp[2] - '0';
     count++;
   }

  temp = env.Argv[count];
  if (temp eq Null(char)) usage();
  
  link = 0;
  for (; ; temp++)
   { if (('0' <= *temp) && (*temp <= '9'))
      link = (10 * link) + (*temp - '0');
     else
      break;
   }
  if (*temp ne '\0') usage();

  { RootStruct	*root = GetRoot();
    LinkInfo	**info = root->Links;
    int		i;
    for (i = 0; i <= link; i++)
     if (*info++ eq Null(LinkInfo))
      usage();
  }      

  count++;
  my_name = env.Argv[count];
  if (my_name eq Null(char)) usage();
  if (my_name[0] ne '/') usage();

  count++; 
  its_name = env.Argv[count];
  if (its_name eq Null(char)) usage();
  if (its_name[0] ne '/') usage();

  count++; 
  link_modes = env.Argv[count];   
  if (link_modes eq Null(char))
   { link_modes = "3333"; goto done_args; }

 	/* BLV - unfortunately this is transputer specific */
  if (strlen(link_modes) ne 4) usage();
   { int i;
     for (i = 0; i < 4; i++)
      { if ((link_modes[i] < '0') || (link_modes[i] > '3'))
         usage();
        if ((link_modes[i] eq '2') && (target_link eq -1))
         target_link = i;
      }
   }

  count++;
  temp		= env.Argv[count];
  memory_limit	= 0;
  if (temp eq Null(char)) goto done_args;
  if ((temp[0] eq '0') && ((temp[1] eq 'x') || (temp[1] eq 'X')))
   { for (temp = &(temp[2]); ; temp++)
      if ((*temp >= '0') && (*temp <= '9'))
       memory_limit = (16 * memory_limit) + (*temp -'0');
      elif ((*temp >= 'a') && (*temp <= 'f'))
       memory_limit = (16 * memory_limit) + (*temp - 'a' + 10);
      elif ((*temp >= 'A') && (*temp <= 'F'))
       memory_limit = (16 * memory_limit) + (*temp - 'A' + 10);
      else break;
   }
  else
   { for ( ; ; temp++)
      if ((*temp >= '0') && (*temp <= '9'))
       memory_limit = (10 * memory_limit) + (*temp - '0');
      else
       break;
   }
  if (*temp ne '\0') usage();
  
done_args:

  if (target_link eq -1) target_link = 0;
  
  config = build_config(my_name, its_name, link_modes, memory_limit, 
  			&config_size);
  image = determine_image();
  config->ImageSize = *((word *) image);
  init_link(link);

#ifdef PARSYTEC
  Parsytec_Reset(link);
#endif

  rc = BootLink(link, image, config, config_size);
  if (rc eq Err_Null)
   { set_link(link);
#if 0
     Delay(100000);   
#else
     Delay(50000);
#endif
     rc = transputer_check_link(link);
     if (rc eq Err_Null) Exit(0x100);
   }   
    
  if ((rc & EG_Mask) ne EG_Timeout)
   { report(": BootLink failed, bad link\n"); Exit(0x100); }
  if ((rc & EO_Mask) eq EO_Message)
   { report(": BootLink failed, timeout sending message to link IOC\n");
     Exit(0x100);
   }
  switch (rc & EO_Mask)
   { case Boot_SoftResetSize	:
     case Boot_SoftResetCode	:
     	report(": BootLink failed to send software reset\n"); break;
     	
     case Boot_BootstrapSize	:
     	report(": BootLink failed to send bootstrap size\n"); break;
     	
     case Boot_BootstrapCode	:
     	report(": BootLink failed to send bootstrap code\n"); break;
     	
     case Boot_Clear1		:
     case Boot_Clear2		:
     case Boot_Clear3		:
     	report(": BootLink failed to send memory clear\n"); break;
     	
     case Boot_ControlByte	:
     	report(": BootLink failed to send control byte\n"); break;
     	
     case Boot_Image		:
     	report(": BootLink failed to send system image\n"); break;
     	
     case Boot_ConfigSize	:
     	report(": BootLink failed to send configuration size\n"); break;
     	
     case Boot_ConfigVector	:
     	report(": BootLink failed to send configuration vector\n"); break;
     	
     default :
     	report(": BootLink failed\n");
   }
   
  Exit(0x100);
}  

/**
*** Display a sensible usage message without using anything fancy.
*** Not as easy as it sounds.
**/
static void usage(void)
{ BYTE	*message;
  int	proglen = strlen(ProgName);
  
  (void) Write(error, ProgName, proglen, -1);
  message = ": usage, ";
  (void) Write(error, message, strlen(message), -1);
  (void) Write(error, ProgName, proglen, -1);
  message = "[-l<link>] <link> <myname> <itsname> [<link modes> <memory limit>]\n";
  (void) Write(error, message, strlen(message), -1);
  (void) Write(error, ProgName, proglen, -1);
  message = ": for example, ";
  (void) Write(error, message, strlen(message), -1);
  (void) Write(error, ProgName, proglen, -1);
  message = " 1 /Cluster/00 /Cluster/01\n";
  (void) Write(error, message, strlen(message), -1);

  Exit(0x100);
}

static void report(char *str)
{ (void) Write(error, ProgName, strlen(ProgName), -1);
  (void) Write(error, str, strlen(str), -1);
}

/**
*** figure out the system image to use. This involves checking the size
*** of the current nucleus and of the file /helios/lib/nucleus.
*** If they are the same, use the current nucleus. Otherwise load the
*** file off disk and use that.
**/
static void	*determine_image(void)
{ Object	*nuc = Locate(Null(Object), "/helios/lib/nucleus");
  ObjInfo	info;
  WORD		*image = GetSysBase();
  Stream	*s;
  void		*buffer;
      
  if (nuc eq Null(Object))
   return((void *) image);
  if (ObjectInfo(nuc, Null(char), (BYTE *) &info) < Err_Null)
   return((void *) image);
  if (info.Size eq *image)
   return((void *) image);
  buffer = Malloc(info.Size);
  if (buffer eq Null(BYTE))
   { report(": not enough memory to load nucleus off disk.\n");
     Exit(0x100);
   }
  s = Open(nuc, Null(char), O_ReadOnly);
  if (s eq Null(Stream))
   { report(": failed to open nucleus file /helios/lib/nucleus\n");
     Exit(0x100);
   }
  if (Read(s, buffer, info.Size, -1) ne info.Size)
   { report(": failed to read all of nucleus file /helios/lib/nucleus\n");
     Exit(0x100);
   }
  Close(s);
  return((void *) buffer);  
}

/**
*** Build a configuration vector in memory. This involves allocating a
*** vector big enough for the main vector plus the two names (including
*** terminators), and for the hell of it allow for more than four links.
*** Most of the configuration vector can be filled in with default
*** information. The ImageSize field is patched later on. 
**/
static Config	*build_config(char *my_name, char *its_name, char *link_modes,
				int memory_limit, int *confsize)
{ int		size = sizeof(Config) + strlen(my_name) + strlen(its_name) + 2;
  Config	*result;
  int		i;
  BYTE		*temp;
  
  if (strlen(link_modes) > 4)
   size += (4 * (strlen(link_modes) - 4));
  result = (Config *) Malloc(size);  
  if (result eq Null(Config))
   { report(": not enough memory to build configuration vector.\n");
     Exit(0x100);
   }
  result->PortTabSize		= 32;
  result->Incarnation		= 1;
  result->LoadBase		= GetSysBase();
  result->ImageSize		= 0;
  result->Date			= GetDate();
  result->FirstProg		= IVecProcMan; 
  result->MemSize		= memory_limit;
  result->Flags			= 0;
  result->NLinks		= strlen(link_modes);
  for (i = 0; link_modes[i] ne '\0'; i++)
   { result->LinkConf[i].Flags = 0;
     switch(link_modes[i])
      { case '0' : result->LinkConf[i].Mode	= Link_Mode_Null;
      		   result->LinkConf[i].State	= Link_State_Null;
      		   break;
        case '1' : result->LinkConf[i].Mode	= Link_Mode_Dumb;
        	   result->LinkConf[i].State	= Link_State_Dumb;
        	   break;
        case '2' : result->LinkConf[i].Mode	= Link_Mode_Intelligent;
        	   result->LinkConf[i].State	= Link_State_Running;
        	   break;
        case '3' : result->LinkConf[i].Mode	= Link_Mode_Intelligent;
        	   result->LinkConf[i].State	= Link_State_Dead;
        	   break;
      }
     result->LinkConf[i].Id = i;
   }

  result->LinkConf[target_link].Flags = Link_Flags_debug + Link_Flags_parent;
  result->LinkConf[target_link].State = Link_State_Running;

	/* Take care of the two RPTRs */
  temp = (BYTE *) result;
  temp = &(temp[sizeof(Config)]);
  result->MyName = temp - (BYTE *) &(result->MyName);
  strcpy(temp, its_name);
  temp += (strlen(its_name) + 1);
  result->ParentName = temp - (BYTE *) &(result->ParentName);
  strcpy(temp, my_name);
  
 	/* and return the results */
  *confsize = size;
  return(result);
}

/**
*** If I am about to attempt a bootstrap down a link, that link had better
*** be in a sensible state. The correct mode depends on whether or not 
*** hand bootstrap is used.
**/
static void init_link(int link)
{ LinkInfo info;
  LinkConf conf;
  
  if (LinkData(link, &info) ne Err_Null)
   { report(": failed to determine current link status.\n");
     Exit(0x100);
   }

#ifdef HANDBOOT   
  if (info.Mode eq Link_Mode_Dumb) return;
  conf.Mode 	= Link_Mode_Dumb;
  conf.State	= Link_State_Dumb;
#else
  if (info.Mode eq Link_Mode_Intelligent) return;
  conf.Mode	= Link_Mode_Intelligent;
  conf.State	= Link_State_Crashed;
#endif
  
  conf.Id	= info.Id;
  conf.Flags	= info.Flags;
  (void) Configure(conf);
}

/**
*** After a bootstrap, ensure that the link is in a sensible state
**/
static void set_link(int link)
{ LinkInfo	info;
  LinkConf	conf;

  if (LinkData(link, &info) ne Err_Null)
   { report(": failed to determine new link status.\n");
     Exit(0x100);
   }
   
  if ((info.Mode eq Link_Mode_Intelligent) &&
      (info.State eq Link_State_Running))
   return;
  conf.Mode 	= Link_Mode_Intelligent;
  conf.State	= Link_State_Dead;
  conf.Id	= info.Id;
  conf.Flags	= info.Flags;
  Configure(conf);
}

/**
*** After a suitable delay, controlled in do_TransputerBoot() above, the
*** network agent checks the booting link. If the remote processor has come
*** up then it will have enabled the link by now, and it can be used by the
*** networking software.
**/
static	int	transputer_check_link(int link)
{ LinkInfo	info;
  int		rc;
    
  if ((rc = LinkData(link, &info)) ne Err_Null) return(rc);
  if ((info.Mode ne Link_Mode_Intelligent) ||
      (info.State ne Link_State_Running))
   return(EC_Error + SS_NetServ + EG_Boot + EO_Processor);

#if 0
  { char		buf[16];
    Object	*temp;
    strcpy(buf, "/link.0/tasks");
    buf[6] = link + '0';
    temp = Locate(Null(Object), buf);
    if (temp eq Null(Object))
     return(EC_Error + SS_NetServ + EG_Boot + EO_ProcMan);
  }
#endif

  return(Err_Null);
}

#ifdef PARSYTEC
#define Reset_Address	0x000000C0

static void Parsytec_Reset(int link)
{ uword *reg = (uword *) Reset_Address;
  *reg = 0;
  *reg = 1;
  *reg = 2;
  *reg = 3;
  *reg = 1 << link;
  Delay(10000);	/* 10 Msec */
  *reg = 0;
}

#endif

#ifdef HANDBOOT
/**
*** Manual bootstrap. This involves the following stages.
*** 1) send in nboot.i, held in a slot in the system image
*** 2) send in a control byte to nboot.i to read in the system image
*** 3) send in the whole system image
*** 4) send in the configuration vector
**/
static int My_BootLink(word link, void *image, Config *config, word confsize)
{ UBYTE	temp[4];
  void  *nboot;
  word  nboot_size;
  word	image_size;
  int	rc;
    
  { word *sysimage	= (word *) image;
    int  offset    	= sysimage[IVecBootStrap];
    nboot		= (BYTE *) &(sysimage[IVecBootStrap]) + offset;
    nboot_size		= sysimage[IVecProcMan] - sysimage[IVecBootStrap] + 4;
  }

  temp[0] = (UBYTE) nboot_size;
  if ((rc = LinkOut(1, link, temp, 2 * OneSec)) ne Err_Null)
   return(Boot_BootstrapSize | EG_Timeout);

  if (LinkOut(nboot_size, link, nboot, 2 *OneSec) ne Err_Null)
   return(Boot_BootstrapCode | EG_Timeout);
  Delay(1000);		/* One millisecond for nboot.i to settle down */

  temp[0] = 4;		/* bootstrap command */
  if (LinkOut(1, link, temp, 2 * OneSec) ne Err_Null)
   return(Boot_ControlByte | EG_Timeout);
  
  image_size = *((word *) image);
  if (LinkOut(image_size, link, image, 2 * OneSec) ne Err_Null)
   return(Boot_Image | EG_Timeout);
   
  if (LinkOut(sizeof(WORD), link, (BYTE *) &confsize, 2 * OneSec) ne Err_Null)
   return(Boot_ConfigSize | EG_Timeout);

  if (LinkOut(confsize, link, (BYTE *) config, 2 * OneSec) ne Err_Null)
   return(Boot_ConfigVector | EG_Timeout);

  return(Err_Null);
}
#endif

