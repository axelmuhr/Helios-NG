/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- network.c								--
--                                                                      --
--	A utility to examine the network				--
--                                                                      --
--	Author:  BLV 23/10/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/network.c,v 1.10 1994/03/10 17:13:27 nickc Exp $*/

#include <helios.h>
#include <stdarg.h>
#include <stdio.h>
#include <syslib.h>
#include <gsp.h>
#include <codes.h>
#include <nonansi.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <posix.h>
#include <root.h>
#include <ctype.h>
#include "session.h"
#include "private.h"
#include "rmlib.h"
#include "exports.h"
#include "netutils.h"

#ifndef	eq
#define	eq ==
#define ne !=
#endif

static	void		do_info(   int, char **);
static	void		do_monitor(int, char **);
static	void		do_help(   int, char **);
static	void		do_show(   int, char **);
static	void		do_list(   int, char **);
static	void		do_showall(int, char **);
static	void		do_owners( int, char **);
static	void		do_avail(  int, char **);
static	void		usage(void);
static	int		init_network(RmProcessor processor, ...);
static	int		delete_unwanted(RmProcessor processor, ...);

int main(int argc, char **argv)
{ char	**real_args;

  if (argc <= 1) usage();

  argc	-= 2;
  real_args = &(argv[2]);
  
  if (!strcmp(argv[1], "info"))
   do_info(argc, real_args);
  elif (!strcmp(argv[1], "monitor"))
   do_monitor(argc, real_args);
  elif (!strcmp(argv[1], "show"))
   do_show(argc, real_args);
  elif (!strcmp(argv[1], "showall"))
   do_showall(argc, real_args);
  elif (!strcmp(argv[1], "list"))
   do_list(argc, real_args);
  elif (!strcmp(argv[1], "owners"))
   do_owners(argc, real_args);
  elif (!strncmp(argv[1], "avail", 5))
   do_avail(argc, real_args);
  elif (!strcmp(argv[1], "help"))
   do_help(argc, real_args);
  else
   { fprintf(stderr, "network: unknown option %s\n", argv[1]);
     usage();
   }
  return(EXIT_SUCCESS);
}

static	void	usage(void)
{ fputs("network: usage should be one of the following\n", stderr);
  fputs("       : network list\n", stderr);
  fputs("       : network show [processors]\n", stderr);
  fputs("       : network showall [processors]\n", stderr);
  fputs("       : network owners\n", stderr);
  fputs("       : network info [processors]\n", stderr);
  fputs("       : network monitor interval [processors]\n", stderr);
  fputs("       : network availibility\n", stderr);
  fputs("       : network help\n", stderr);
  exit(EXIT_FAILURE);
}

/**
*** list: a less verbose version of show
**/
static int do_list_aux(RmProcessor, ...);

static	void	do_list(int argc, char **argv)
{ RmNetwork	network;

  if (argc ne 0) usage();
  network = RmGetNetwork();
  if (network eq (RmNetwork) NULL)
   fprintf(stderr,
    "network: failed to get information on current network, RmLib error %s\n",
        	RmMapErrorToString(RmErrno));
  else
   { fputs("Current network : ", stdout);
     (void) RmApplyProcessors(network, &do_list_aux);
     putchar('\n');
   }

  argv = argv;
}

static int do_list_aux(RmProcessor Processor, ...)
{ 
  printf("/%s ", RmGetProcessorId(Processor));
  return(0);
}

/**
*** Show : simply print out the current network, in all its gory detail
**/
static	void	do_show(int argc, char **argv)
{ RmNetwork	network;

  network = RmGetNetwork();
  if (network eq (RmNetwork) NULL)
   fprintf(stderr,
    "network: failed to get information on current network, RmLib error %s\n",
        	RmMapErrorToString(RmErrno));
  elif (argc eq 0)
   PrintNetwork(network);
  else
   { init_PrintNetwork(TRUE);
     for ( ; argc > 0; argc--, argv++)
      { RmProcessor	current = RmLookupProcessor(network, *argv);
        if (current eq (RmProcessor) NULL)
         fprintf(stderr, "network: processor %s is not in the network\n",
		*argv);
        else
         PrintProcessor(current, 0);
      }
     tidy_PrintNetwork();
   }
}

/**
*** Showall : similar to show but also prints out the hardware details
**/
static	void	do_showall(int argc, char **argv)
{ RmNetwork	network;

  network = RmGetNetworkAndHardware();
  if (network eq (RmNetwork) NULL)
   fprintf(stderr,
    "network: failed to get information on current network, RmLib error %s\n",
        	RmMapErrorToString(RmErrno));
  elif (argc eq 0)
   PrintNetwork(network);
  else
   { init_PrintNetwork(TRUE);
     for ( ; argc > 0; argc--, argv++)
      { RmProcessor	current = RmLookupProcessor(network, *argv);
        if (current eq (RmProcessor) NULL)
         fprintf(stderr, "network: processor %s is not in the network\n",
		*argv);
        else
         PrintProcessor(current, 0);
      }
     tidy_PrintNetwork();
   }
}

static	void	do_info(int argc, char **argv)
{ RmNetwork	network;
  
  network = RmGetNetwork();
  if (network eq (RmNetwork) NULL)
   { fprintf(stderr,
      "network: failed to get information on current network, RmLib error %s\n",
    		RmMapErrorToString(RmErrno));
     exit(EXIT_FAILURE);
   }
  if (argc ne 0)
   { (void) RmApplyProcessors(network, &init_network);
      for ( ; argc > 0; argc--, argv++)
       { RmProcessor	current = RmLookupProcessor(network, *argv);
         if (current eq (RmProcessor) NULL)
           fprintf(stderr, "network: processor %s is not in the network\n",
       		*argv);
         else
          RmSetProcessorPrivate(current, 1);
       }
      (void) RmApplyProcessors(network, &delete_unwanted);
   }

  DisplayInfo(network);

  RmFreeNetwork(network);
}

static	void	do_monitor(int argc, char **argv)
{ int		delay = 2;
  RmNetwork	network;

  if ((argc ne 0) && (**argv ne '/'))
   { delay = atoi(*argv);
     argc--; argv++;
     if (delay <= 0) delay = 2;
   }

  network = RmGetNetwork();
  if (network eq (RmNetwork) NULL)
   { fprintf(stderr,
      "network: failed to get information on current network, RmLib error %s\n",
    		RmMapErrorToString(RmErrno));
     exit(EXIT_FAILURE);
   }

  if (argc ne 0)
   { (void) RmApplyProcessors(network, &init_network);
      for ( ; argc > 0; argc--, argv++)
       { RmProcessor	current = RmLookupProcessor(network, *argv);
         if (current eq (RmProcessor) NULL)
           fprintf(stderr, "network: processor %s is not in the network\n",
       		*argv);
         else
          RmSetProcessorPrivate(current, 1);
       }
      (void) RmApplyProcessors(network, &delete_unwanted);
   }

  MonitorNetwork(network, delay);   
}

/**
*** do_owners
**/
typedef	struct	OwnerDetails {
	Node		Node;
	int		Owner;
	int		Count;
} OwnerDetails;

static	List	OwnerList;
static	int	NumberProcessors;
static	int	IOCount;
static	int	RouterCount;
static	int	NetworkWalk(RmProcessor Processor, ...);
static	WORD	ShowOwners(Node *node, WORD arg);
static	WORD	MatchOwner(Node *node, WORD arg);

static void do_owners(int argc, char **argv)
{ RmNetwork	Network;

  argc = argc; argv = argv;
  
  InitList(&(OwnerList));  
  IOCount = RouterCount = 0;
  
	/* Get details of the current network into local memory */
  Network = RmGetNetwork();
  if (Network == (RmNetwork) NULL)
   { fputs("network: failed to get network details\n", stderr);
     return;
   }
  NumberProcessors = RmCountProcessors(Network);
  
	/* Walk down the current network examining every processor	*/
	/* Build the ownership list as the program goes along.		*/
  (void) RmApplyProcessors(Network, &NetworkWalk);

	/* Output the results by walking down the owner list.		*/
  (void) WalkList(&OwnerList, &ShowOwners);
  if (IOCount > 0)
   printf("%-10s : %3d processor%s, %2d%% of the network\n",
  	"I/O", IOCount,
  	(IOCount > 1) ? "s" : " ", (100 * IOCount) / NumberProcessors);

  if (RouterCount > 0)
   printf("%-10s : %3d processor%s, %2d%% of the network\n",
  	"Router", RouterCount,
  	(RouterCount > 1) ? "s" : " ", (100 * RouterCount) / NumberProcessors);

	/* Free the network and the owner list */
  RmFreeNetwork(Network);
  until(EmptyList_(OwnerList)) Free(RemHead(&OwnerList));
}

	/* This routine is called for every processor in the network.	*/
static	int	NetworkWalk(RmProcessor Processor, ...)
{ int		Owner;
  OwnerDetails	*details;
  int		purpose = RmGetProcessorPurpose(Processor) & RmP_Mask;

  if (purpose eq RmP_IO)
   { IOCount++; return(0); }
  elif (purpose eq RmP_Router)
   { RouterCount++; return(0); }
   
	/* Get the current processor owner, and see if this owner	*/
	/* is already known.						*/
  Owner		= RmGetProcessorOwner(Processor);
  details	= (OwnerDetails *) SearchList(&OwnerList, &MatchOwner, Owner);

	/* If the user is already known then the search will have found	*/
	/* an OwnerDetails structure that can be updated. Otherwise	*/
	/* a new structure must be allocated and initialised.		*/
  if (details != (OwnerDetails *) NULL)
   details->Count++;
  else
   { details	= (OwnerDetails *) malloc(sizeof(OwnerDetails));
     if (details == (OwnerDetails *) NULL)
      { fputs("network : out of memory building owner list\n", stderr);
        return(0);
      }
     details->Owner	= Owner;
     details->Count	= 1;
#ifdef SYSDEB
     details->Node.Next = details->Node.Prev = &details->Node;
#endif
     AddTail(&(OwnerList), &(details->Node));
   }

	/* for this program the return value is ignored anyway.	*/
  return(0);
}

	/* Match a processor's owner with an entry in the current	*/
	/* list of owners.						*/
static	WORD	MatchOwner(Node *node, WORD arg)
{ OwnerDetails	*details	= (OwnerDetails *) node;

  if (details->Owner == arg)
   return(1);
  else
   return(0);
}

	/* Print out the results of the search.				*/
static	WORD	ShowOwners(Node *node, WORD arg)
{ OwnerDetails	*details = (OwnerDetails *) node;

  printf("%-10s : %3d processor%s, %2d%% of the network\n",
  	RmWhoIs(details->Owner), details->Count, 
	(details->Count > 1) ? "s" : " ",
  	(details->Count * 100) / NumberProcessors);
  return(0);
  arg = arg;
}


/**
*** do_help(). Print out lots and lots and lots.
**/

static char *text1 = "\
Network is a command to allow users to examine the whole network of\n\
processors, and see what is happening. There are various options.\n\n\
";
static char *text2 = "\
nework list : this simply lists the processors in the current network.\n\
Typical output might be something like:\n\
  Current network : /01 /02 /03 /06\n\n\
";
static char *text3 = "\
network show : this is used to give more information about the processors,\n\
including processor type, the amount of memory, and details of connections\n\
to other processors. By default all processors will be listed, but it is\n\
possible to specify particular processors if desired, for example:\n\
    network show /01 /02 /03\n\n\
";
static char *text4 = "\
network showall : this gives similar information to the show option,\n\
but also gives details of the hardware facilities available in the\n\
network.\n\n\
";
static char *text5 = "\
network info : this is used to get information about the current loading\n\
of the processors in the network. The output produced is something like:\n\
   Processor 02 : load 1045, memory free 42%\n\
By default the command gives details about all the processors in the\n\
network, but it is possible to restrict the processors by listing\n\
the ones desired, for example:\n\
    network info /02 /03\n\n\
";
static char *text6 = "\
network monitor : this is like the info option, but runs continuously\n\
until the user quits. A menu is displayed. It is possible to\n\
specify a different interval if desired:\n\
  network monitor 20\n\
Also, the processors affected may be restricted as per the info command.\n\
  network monitor 20 /02 /03\n\n\
";
static char *text7 = "\
network owners : this shows the various people logged in to the network\n\
and how many processors have been allocated to each user.\n\n\
";

static char *text8 = "\
network availability : this shows the numbers of processors currently free\n\
or being cleaned out, and lists them.\n\n\
";

static char *text9 = "\
network help : the help option gives this information.\n\n\
";

static	void	do_help(int argc, char **argv)
{ FILE	*output = popen("/helios/bin/more", "w");

  argc = argc; argv = argv;
  
  if (output eq (FILE *) NULL)
   { fputs("network: failed to access /helios/bin/more\n", stderr);
     exit(EXIT_FAILURE);
   }
  fputs(text1, output);
  fputs(text2, output);
  fputs(text3, output);
  fputs(text4, output);
  fputs(text5, output);
  fputs(text6, output);
  fputs(text7, output);
  fputs(text8, output);
  fputs(text9, output);
  pclose(output);  
}

/**
*** Show the availability of processors. This involves two phases.
*** In the first phase the numbers of processors in the free pool or
*** owned by the cleaners are counted up. In the second phase the
*** appropriate processors are printed. A junk variable is used to get
*** the commas right.
**/
static int do_avail_aux1(RmProcessor, ...);
static int do_avail_aux2(RmProcessor, ...);

static void do_avail(int argc, char **argv)
{ RmNetwork	network;
  int		free_count;
  int		cleaner_count;
  int		graveyard_count;
  int		junk;

  if (argc ne 0) usage();
  argv = argv;

  network = RmGetNetwork();
  if (network eq (RmNetwork) NULL)
   { fprintf(stderr, "network: failed to get information on current network, RmLib error %s\n",
		RmMapErrorToString(RmErrno));
     return;
   }

  free_count	  = RmApplyProcessors(network, &do_avail_aux1, RmO_FreePool);
  cleaner_count	  = RmApplyProcessors(network, &do_avail_aux1, RmO_Cleaners);
  graveyard_count = RmApplyProcessors(network, &do_avail_aux1, RmO_Graveyard);
  printf("network  : %d processors free, %d being cleaned, %d dead.\n",
		free_count, cleaner_count, graveyard_count);
  if (free_count > 0)
   { junk = 0;
     fputs("Free     : ", stdout);
     (void) RmApplyProcessors(network, &do_avail_aux2, RmO_FreePool, &junk);
     putchar('\n');
   }
  if (cleaner_count > 0)
   { junk = 0;
     fputs("Cleaners : ", stdout);
     (void) RmApplyProcessors(network, &do_avail_aux2, RmO_Cleaners, &junk);
     putchar('\n');
   }
}

static int do_avail_aux1(RmProcessor processor, ...)
{ va_list	args;
  int		owner;
  int		purpose;

  va_start(args, processor);
  owner = va_arg(args, int);
  va_end(args);

  if (RmGetProcessorOwner(processor) ne owner)
   return(0);

  purpose = RmGetProcessorPurpose(processor) & RmP_Mask;
  if ((purpose eq RmP_IO) || (purpose eq RmP_Router))
   return(0);

  return(1);
}

static int do_avail_aux2(RmProcessor processor, ...)
{ va_list	args;
  int		owner;
  int		*junk;
  int		purpose;

  va_start(args, processor);
  owner = va_arg(args, int);
  junk  = va_arg(args, int *);
  va_end(args);

  if (RmGetProcessorOwner(processor) ne owner)
   return(0);

  purpose = RmGetProcessorPurpose(processor) & RmP_Mask;
  if ((purpose eq RmP_IO) || (purpose eq RmP_Router))
   return(0);

  if (*junk ne 0) fputs(", ", stdout);
  fputs(RmGetProcessorId(processor), stdout);
  *junk += 1;
  return(1);
}

/**
*** init_domain(), this is used to set a processor's private field to 0,
*** to mark it as unused.
**/
static int init_network(RmProcessor Processor, ...)
{ 
  RmSetProcessorPrivate(Processor, 0);
  return(0);
}

/**
*** complement the above, get rid of any unwanted processors
**/
static int delete_unwanted(RmProcessor Processor, ...)
{ int	rc;

  if (RmGetProcessorPrivate(Processor) ne 0) return(0);
  
  if (RmRemoveProcessor(Processor) eq (RmProcessor) NULL)
   { fputs("network: internal error manipulating network (remove)\n", stderr);
     exit(EXIT_FAILURE);
   }
  if ((rc = RmFreeProcessor(Processor)) ne RmE_Success)
   { fprintf(stderr, "network: internal error manipulating network (delete %x)\n",
   		rc);
     exit(EXIT_FAILURE);
   }

  return(0);
}
   
