/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- dlink.c								--
--                                                                      --
--	Disable a link, going through the network server.		--
--                                                                      --
--	Author:  BLV 27/10/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/dlink.c,v 1.5 1993/08/11 10:28:47 bart Exp $*/

#include <stdio.h>
#include <nonansi.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "rmlib.h"
#include "netutils.h"

#ifndef eq
#define eq ==
#define ne !=
#endif

static void 		usage(void);

int	main(int argc, char **argv)
{ char		*processor_name;
  int		link;
  RmProcessor	processor;
  RmNetwork	network;
  int		purpose;
  RmProcessor	neighbour;
  int		destlink;
  int		mode = 0;
  int		rc;
  char		*text;
      
  if (argc ne 3) usage();
  processor_name = argv[1];
  link		 = atoi(argv[2]);
  if (link < 0) usage();
  
  network = RmGetNetwork();
  if (network eq (RmNetwork) NULL)
   { fprintf(stderr, "dlink: failed to get network details\n");
     exit(EXIT_FAILURE);
   }

  processor = RmLookupProcessor(network, processor_name);
  if (processor eq (RmProcessor) NULL)
   { fprintf(stderr, "dlink: failed to find processor %s in the network\n",
   		processor_name);
     usage();
   }

  if (link >= RmCountLinks(processor))
   { fprintf(stderr, "dlink: processor %s only has %d link%s\n",
   		processor_name, RmCountLinks(processor),
   		(RmCountLinks(processor) ne 1) ? "s" : " " );
     exit(EXIT_FAILURE);
   }
   
  purpose = RmGetProcessorPurpose(processor) & RmP_Mask;
  if (purpose eq RmP_Native)
   { fputs("dlink: cannot disable links of native processors\n", stderr);     
     exit(EXIT_FAILURE);
   }

	/* BLV routers */
	
  if (purpose eq RmP_IO)
   { neighbour = RmFollowLink(processor, link, &destlink);
     if (neighbour eq (RmProcessor) NULL)
      { fputs("dlink: cannot disable that link of an I/O processor\n",
      		stderr);
      	exit(EXIT_FAILURE);
      }
     else
      { processor = neighbour; link = destlink; }
   }

  rc = RmSetLinkMode(processor, link, RmL_NotConnected);
  if (rc ne RmE_Success)
   fprintf(stderr, "dlink: failed to disable that link, RmLib error %s\n",
   		RmMapErrorToString(rc));
  rc = RmGetLinkMode(processor, link, &mode);
  if (rc ne RmE_Success)
   { fprintf(stderr, "dlink: failed to re-examine that link, RmLib error %s\n",
   		 RmMapErrorToString(rc));
     exit(EXIT_FAILURE);
   }

  switch(mode)
   { case RmL_NotConnected	: text = "not connected"; break;
     case RmL_Dumb		: text = "dumb"; 	  break;
     case RmL_Pending		: text = "pending";	  break;
     case RmL_Intelligent	: text = "running";	  break;
     case RmL_Dead		: text = "dead";	  break;
     default			: text = "unknown";
   }
   
  printf("New link state : %s\n", text);
}

static void usage(void)
{ fputs("dlink: usage,       dlink <processor> link\n", stderr);
  fputs("     : for example, dlink /00 2\n", stderr);
  exit(EXIT_FAILURE);
}

