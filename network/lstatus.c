/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- lstatus.c								--
--                                                                      --
--	Display the current state of a link				--
--                                                                      --
--	Author:  BLV 27/10/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/lstatus.c,v 1.7 1993/08/11 10:31:33 bart Exp $*/

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
   { fprintf(stderr, "lstatus: failed to get network details\n");
     exit(EXIT_FAILURE);
   }

  processor = RmLookupProcessor(network, processor_name);
  if (processor eq (RmProcessor) NULL)
   { fprintf(stderr, "lstatus: failed to find processor %s in the network\n",
   		processor_name);
     usage();
   }

  if (link >= RmCountLinks(processor))
   { fprintf(stderr, "lstatus: processor %s only has %d link%s\n",
   		processor_name, RmCountLinks(processor),
   		(RmCountLinks(processor) ne 1) ? "s" : " " );
     exit(EXIT_FAILURE);
   }
   
  purpose = RmGetProcessorPurpose(processor) & RmP_Mask;
  if (purpose eq RmP_Native)
   { fputs("lstatus: cannot examine links of native processors\n", stderr);     
     exit(EXIT_FAILURE);
   }

	/* BLV - routers */
	
  if (purpose eq RmP_IO)
   { neighbour = RmFollowLink(processor, link, &destlink);
     if (neighbour eq (RmProcessor) NULL)
      { fputs("lstatus: cannot examine that link of an I/O processor\n",
      		stderr);
      	exit(EXIT_FAILURE);
      }
     else
      { processor = neighbour; link = destlink; }
   }

  rc = RmGetLinkMode(processor, link, &mode);
  if (rc ne RmE_Success)
   { fprintf(stderr, "lstatus: failed to examine that link, RmLib error %s\n",
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
   
  printf("Link state %s\n", text);
  return(EXIT_SUCCESS);
}

static void usage(void)
{ fputs("lstatus: usage,       lstatus <processor> link\n", stderr);
  fputs("       : for example, lstatus /00 2\n", stderr);
  exit(EXIT_FAILURE);
}

