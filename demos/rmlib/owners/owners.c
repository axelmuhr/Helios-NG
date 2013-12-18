/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- owners.c								--
--                                                                      --
--	Public command to display some usage statistics about the	--
--	current network.						--
--                                                                      --
--	Author:  BLV 10/9/90						--
--                                                                      --
------------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <queue.h>
#include <rmlib.h>

typedef	struct	OwnerDetails {
	Node		Node;
	int		Owner;
	int		Count;
} OwnerDetails;

static	List	OwnerList;
static	int	NumberProcessors;
static	int	NetworkWalk(RmProcessor Processor, ...);
static	WORD	ShowOwners(Node *node, WORD arg);
static	WORD	MatchOwner(Node *node, WORD arg);

int main(int argc, char **argv)
{ RmNetwork	Network;

  argv = argv;  
  if (argc != 1)
   { fprintf(stderr, "owners: usage, owners\n");
     exit(EXIT_FAILURE);
   }

  InitList(&(OwnerList));  

	/* Get details of the current network into local memory */
  Network = RmGetNetwork();
  if (Network == (RmNetwork) NULL)
   { fprintf(stderr, "owners: failed to get network details.\n");
     fprintf(stderr, "      : %s\n", RmMapErrorToString(RmErrno));
     exit(EXIT_FAILURE);
   }
  NumberProcessors = RmCountProcessors(Network);
  
	/* Walk down the current network examining every processor	*/
	/* Build the ownership list as the program goes along.		*/
  (void) RmApplyProcessors(Network, &NetworkWalk);

	/* Output the results by walking down the owner list.		*/
  (void) WalkList(&OwnerList, &ShowOwners);
  return(EXIT_SUCCESS);
}

	/* This routine is called for every processor in the network.	*/
static	int	NetworkWalk(RmProcessor Processor, ...)
{ int		Owner;
  OwnerDetails	*details;
  
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
      { fprintf(stderr, "owners: out of memory building owner list\n");
        exit(EXIT_FAILURE);
      }
     details->Owner	= Owner;
     details->Count	= 1;
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

  printf("%-10s : %3d processors, %2d%% of the network\n",
  	RmWhoIs(details->Owner), details->Count, 
  	(details->Count * 100) / NumberProcessors);
  return(0);
  arg = arg;
}
