/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- findsm.c								--
--	Locate the Session Manager in charge of this processor		--
--                                                                      --
--	Author:  BLV 2/9/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/findsm.c,v 1.5 1993/08/11 10:29:51 bart Exp $*/


/**
*** This program attempts to find the correct Session Manager for a given
*** network. This is not necessarily easy.
***
*** 1) determine the network base name. If an argument is given then this
***    is used. Otherwise the base name is extracted from the current
***    processor name. A typical base name would be /Net
*** 2) locate a session manager. If there is no session manager then no
***    amount of searching will find the right one.
*** 3) if the session manager name matches the base name, success
*** 4) start searching through the network for the right processor
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <helios.h>
#include <syslib.h>
#include <gsp.h>
#include <servlib.h>
#include <codes.h>
#include "netutils.h"

#ifndef eq
#define eq ==
#define ne !=
#endif

static void	usage(void);
static void	get_basename(int, char **);
static void	add_processor(Object *);
static void	do_search(void);

static char	BaseName[NameMax + 1];

int main(int argc, char **argv)
{ Object	*sm = Locate(Null(Object), "/sm");
  int		i;
  
  if (sm eq Null(Object))
   { fputs("findsm: failed to find any session managers.\n", stderr);
     exit(EXIT_FAILURE);
   }
   
  get_basename(argc, argv);

  for (i = 1; BaseName[i] ne '\0'; i++)
   if (sm->Name[i] ne BaseName[i])
    goto search_network;
    
  if (sm->Name[i] eq '/')
   { puts(sm->Name);
     return(EXIT_SUCCESS);
   }

search_network:

  { char	procname[IOCDataMax];
    Object	*this_processor;
    MachineName(procname);
    this_processor = Locate(Null(Object), procname);
    if (this_processor eq Null(Object))
     { fprintf(stderr, "findsm: failed to locate own processor %s\n",
     		procname);
       exit(EXIT_FAILURE);
     }
    add_processor(this_processor);
  }

  do_search();
  
  return(EXIT_FAILURE);    
}

static void usage(void)
{ fputs("findsm: usage, findsm <network>\n", stderr);
  exit(EXIT_FAILURE);
}

/**
*** Determine the base name to use for the search for the session manager.
*** 1) if no name is given, determine the current processor name.
***    If this is something like /root then the processor is not in a
***    network so there is no point in searching for a session manager.
***    If it is something like /Net/root then the base name is /Net.
*** 2) either zero or one arguments should be given
*** 3) if a name is given then this should refer to something in the
***    target network. Typically the name would be BartNet or /BartNet
***    to look for a session manager /BartNet/root/sm, but I can be a
***    bit more flexible than this. For example, findsm /helios would find
***    the session manager controlling the network in which the current
***    helios server is located.
**/
static void get_basename(int argc, char **argv)
{
  if (argc eq 1)
   {	/* determine the base name from the current processor name */
     char	processor_name[IOCDataMax];
     char	*temp;
     MachineName(processor_name);
     
     for (temp = &(processor_name[1]); (*temp ne '/') && (*temp ne '\0');
     	  temp++);
     if (*temp eq '\0')
      { fprintf(stderr, "findsm: current processor %s is not part of a network\n",
      		processor_name);
      	exit(EXIT_FAILURE);
      }
     *temp = '\0';
     strcpy(BaseName, processor_name);
   }
  elif ((argc > 2) || (argc < 1))
   usage();
  else
   { Object	*temp;
     char	netname[IOCDataMax];
     char	*str;
     
     if (*(argv[1]) eq '/')
      strcpy(netname, argv[1]);
     else
      { netname[0] = '/';
        strcpy(&(netname[1]), argv[1]);
      }
     temp = Locate(Null(Object), netname);
     if (temp eq Null(Object))
      { fprintf(stderr, "findsm: failed to locate network %s\n", netname);
        exit(EXIT_FAILURE);
      }
     strcpy(netname, temp->Name);
     Close(temp);
     for (str = &(netname[1]); (*str ne '/') && (*str ne '\0'); str++);
     if (*str eq '\0')
      { fprintf(stderr, "findsm: object %s is not part of a network\n",
      		netname);
      	exit(EXIT_FAILURE);
      }
     *str = '\0';
     strcpy(BaseName, netname);
  }
}

/**
*** add a processor to the current table of processors, expanding the
*** table if necessary.
**/
static Object	**ProcessorVec	= Null(Object *);
static int	NextProcessor	= 0;
static int	MaxProcessor	= 0;

static void add_processor(Object *processor)
{ Object	**new_vec;
  int		i;

  for (i = 0; i < NextProcessor; i++)
   if (!strcmp(ProcessorVec[i]->Name, processor->Name))
    { Close(processor); return; }
    
  if (NextProcessor eq MaxProcessor)	/* time to realloc */
   { MaxProcessor = (MaxProcessor eq 0) ? 16 : 2 * MaxProcessor;
     new_vec = (Object **) Malloc(MaxProcessor * sizeof(Object *));
     if (new_vec eq Null(Object *))
      { fputs("findsm: out of memory when searching network\n", stderr);
        exit(EXIT_FAILURE);
      }
     if (NextProcessor > 0)
      memcpy((void *) new_vec, (void *) ProcessorVec,
             NextProcessor * sizeof(Object *));
     Free(ProcessorVec);
     ProcessorVec = new_vec;
   }      
  ProcessorVec[NextProcessor++] = processor;
}

/**
*** do_search().
**/
static void do_search(void)
{ int		proc_count;
  Object	*current_processor;
  Object	*sm;
  Object	*next_processor;
  int		i;
  char		linkbuf[NameMax];
  int		number_links;

  strcpy(linkbuf, "link.");
      
  for (proc_count = 0; proc_count < NextProcessor; proc_count++)
   { current_processor = ProcessorVec[proc_count];
     number_links = Util_CountLinks(current_processor);

	/* Check whether or not the current processor has the right base name */
     for (i = 1; BaseName[i] ne '\0'; i++)
      if (current_processor->Name[i] ne BaseName[i])
       goto links;
     if (current_processor->Name[i] ne '/')
      goto links;

	/* Does the current processor have a network server */
     sm = Locate(current_processor, "sm");
     if (sm ne Null(Object))
      { puts(sm->Name); exit(EXIT_SUCCESS); }
      
links:     
     for (i = 0;  i < number_links; i++)
      { linkbuf[5] = '\0';
        if (i eq 0)
         strcat(linkbuf, "0");
        else
         addint(linkbuf, i);

        next_processor = Locate(current_processor, linkbuf);
        if (next_processor ne Null(Object))
         add_processor(next_processor);
      }
   }
}
 

