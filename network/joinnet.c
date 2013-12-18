/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- joinnet.c								--
--		Join the current network into a larger one, by enabling	--
-- a connecting link etc.						--
--                                                                      --
--	Author:  BLV 18/11/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/joinnet.c,v 1.8 1993/08/11 10:30:24 bart Exp $*/

#include <string.h>
#include <stdarg.h>
#include <posix.h>
#include <syslib.h>
#include <codes.h>
#include <nonansi.h>
#include <stdlib.h>
#include "private.h"
#include "rmlib.h"
#include "session.h"
#include "exports.h"
#include "netutils.h"

	/* joinnet cannot work with C40 links, problems with the token	*/
#ifdef __C40
int main(void)
{
	fputs("joinnet: cannot be used in a C40 network.\n", stderr);
	exit(EXIT_FAILURE);
}

#else

#ifndef eq
#define eq ==
#define ne !=
#endif

static	void 		usage(void);
static	void 		enable_the_link(void);
static	void 		find_the_connector(void);
static	void 		find_the_network_server(void);
static	void 		do_the_business(void);

static	Object	*neighbour;
static	int	neighbour_link;
static	Object	*connector;
static	int	connector_link;
static	Object	*network_server;
static	char	text_buffer[1024];
static	char	path[109];

int main(int argc, char **argv)
{ 
  if (argc ne 3) usage();

  if (*argv[1] eq '/')
   neighbour = Locate(Null(Object), argv[1]);
  else
   { path[0] = '/';
     strcpy(&(path[1]), argv[1]);
     neighbour = Locate(Null(Object), path);
   }

  if (neighbour eq Null(Object))
   { fprintf(stderr, "joinnet: failed to locate processor %s\n", argv[1]);
     exit(EXIT_FAILURE);
   }

  neighbour_link = atoi(argv[2]);

  enable_the_link();
  find_the_connector();
  find_the_network_server();
  do_the_business();
  return(0);     
}

static	void usage(void)
{ fputs("joinnet: usage, joinnet <processor> <link>\n", stderr);
  fputs("       : for example, joinnet /02 2\n", stderr);
  exit(EXIT_FAILURE);
}

/**
*** Enable the specified link, if possible.
**/
static	void	enable_the_link(void)
{ RmProcessor	target;
  RmNetwork	net;
  int		mode;
  int		rc;
        
  net = RmGetNetwork();
  if (net eq (RmNetwork) NULL)
   { fputs("joinnet: failed to get current network details.\n", stderr);
     exit(EXIT_FAILURE);
   }

  target = RmLookupProcessor(net, neighbour->Name);
  if (target eq (RmProcessor) NULL)
   { fprintf(stderr, "joinnet: processor %s is not known to the Network Server\n",
   		neighbour->Name);
     exit(EXIT_FAILURE);
   }

  rc = RmSetLinkMode(target, neighbour_link, RmL_Intelligent);
  if (rc ne RmE_Success)
   { fputs("joinnet: failed to enable connecting link.\n", stderr);
     exit(EXIT_FAILURE);
   }  

  Delay(OneSec);
  rc = RmGetLinkMode(target, neighbour_link, &mode);
  if ((rc ne RmE_Success) || (mode ne RmL_Intelligent))
   { fputs("joinnet: failed to enable the connecting link.\n", stderr);
     exit(EXIT_FAILURE);
   }  
}

/**
*** Once the link has been enabled, find the connector. One of its links
*** should come back to processor neighbour.
**/
static	void	find_the_connector(void)
{ char		linkbuf[16];
  int		i;
  Object	*temp;
  int		number_links;

  sprintf(linkbuf, "link.%d", neighbour_link);
  connector = Locate(neighbour, linkbuf);
  if (connector eq Null(Object))
   { fputs("joinnet: failed to find a processor at the other end of the link.\n",
   		stderr);
     exit(EXIT_FAILURE);
   }

  number_links = Util_CountLinks(connector);
  for (i = 0; i < number_links; i++)
   { sprintf(linkbuf, "link.%d", i);
     temp = Locate(connector, linkbuf);
     if (temp ne Null(Object))
      { 
        if (!strcmp(temp->Name, neighbour->Name))
         { connector_link = i;
           Close(temp);
           return;
         }
        Close(temp);
        continue;
      }
   }
  
  fprintf(stderr, "joinnet: failed to find a link on processor %s going back into this network.\n",
  		connector->Name);
  exit(EXIT_FAILURE);
}

/**
*** Given a processor in the remote network, find the appropriate network
*** server.
**/
static	void	find_the_network_server(void)
{ Object		**proc_vec;
  int			max_processors;
  char			*temp;
  int			len = 2;
  int			next_proc = 0;
  char			linkbuf[16];
  int			cur_proc;
  char			name[IOCDataMax];

  network_server = Null(Object);
  strcpy(name, connector->Name);
        
 	/* Ignore processors that do not have the right base name	*/
  for (temp = &(name[1]); *temp ne '/'; temp++) len++;
  
  proc_vec = (Object **) Malloc(16 * sizeof(Object *));
  if (proc_vec eq Null(Object *))
   { fputs("joinnet: out of memory locating Network Server\n", stderr);
     exit(EXIT_FAILURE);
   }
   
  max_processors = 16;
  proc_vec[next_proc++] = connector;
  strcpy(linkbuf, "link.");
  
  for (cur_proc = 0; cur_proc < next_proc; cur_proc++)
   { Object	*current = proc_vec[cur_proc];
     Object	*ns = Locate(current, "ns");
     Object	*next;
     int	i;
     int	number_links;

     if (ns ne Null(Object))
      { network_server = ns;
        break;
      }

     number_links = Util_CountLinks(current);
     for (i = 0; i < number_links; i++)
      { 
	if ((current eq connector) && (i eq connector_link))
         continue;

	linkbuf[5] = '\0';
        if (i eq 0)
         strcat(linkbuf, "0");
        else
         addint(linkbuf, i);
        next = Locate(current, linkbuf);
        if (next ne Null(Object))
         { int	j;

		/* Check we are still in the right network */
	   if (strncmp(next->Name, name, len)) 
	    goto skip;
	    
	   	/* Check that this processor has not been found already */
           for (j = 0; j < next_proc; j++)
            if (!strcmp(proc_vec[j]->Name, next->Name))
             goto skip;
             
           if (next_proc eq max_processors)
            { Object **temp = Malloc(2 * max_processors * sizeof(Object *));
              if (temp eq Null(Object *))
               { fputs("joinnet: out of memory locating Network Server.\n", stderr);
                 exit(EXIT_FAILURE);
               }
              memcpy(temp, proc_vec, max_processors * sizeof(Object *));
              Free(proc_vec);
              proc_vec = temp;
              max_processors *= 2;
            }
           proc_vec[next_proc++] = next;
         }
skip:
	continue;
      }
   }

  for (cur_proc = 0; cur_proc < next_proc; cur_proc++)
   Close(proc_vec[cur_proc]);
  Free(proc_vec);
  if (network_server eq Null(Object))
   { fputs("joinnet: failed to locate a Network Server in the remote network.\n",
   		stderr);
     exit(EXIT_FAILURE);
   }

	/* /Cluster/00/ns -> /Cluster/00/.socket	*/
  strcpy(path, network_server->Name);
  temp = objname(path);
  strcpy(temp, ".socket");
}

/**
*** Do the business. This involves contacting the local network server and
*** asking it to do lots of nasties.
**/
static	void	do_the_business(void)
{ int		rc;
  RmRequest	request;
  RmReply	reply;
  int		size;
  
  Clear(request); Clear(reply);
  request.FnRc		= RmC_JoinNetwork;
  request.Arg1		= connector_link;
  request.Arg2		= neighbour_link;
  request.VariableData	= text_buffer;

  strcpy(text_buffer, path);
  size = strlen(path) + 1;
  strcpy(&(text_buffer[size]), connector->Name);
  size += strlen(connector->Name) + 1;
  strcpy(&(text_buffer[size]), neighbour->Name);
  size += strlen(neighbour->Name) + 1;

  request.VariableSize	= size;

  rc = RmXch(&RmNetworkServer, &request, &reply);
  if (rc ne RmE_Success)
   { fprintf(stderr, "joinnet: failed to join networks, RmLib error %s.\n",
   	RmMapErrorToString(rc));
     exit(EXIT_FAILURE);
   }
}

#endif
