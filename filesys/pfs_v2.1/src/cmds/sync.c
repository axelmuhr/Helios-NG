static char rcsid[] = "$Header: /hsrc/filesys/pfs_v2.1/src/cmds/RCS/sync.c,v 1.1 1992/07/13 16:18:43 craig Exp $";

/* $Log: sync.c,v $
 * Revision 1.1  1992/07/13  16:18:43  craig
 * Initial revision
 *
 * Revision 1.1  90/01/09  13:32:48  chris
 * Initial revision
 * 
 */
/*************************************************************************
**                                                                      **
**                  H E L I O S   F I L E S E R V E R                   **
**                  ---------------------------------                   **
**                                                                      **
**                Copyright (C) 1988,1989 Parsytec GmbH                 **
**                         All Rights Reserved.                         **
**                                                                      **
** sync.c								**
**                                                                      **
**	User entry point to generate an "extra" sync_fs()-call.		**
**									**
**************************************************************************
** HISTORY  :             						**
**-----------                                                           **
** Author   : 06/04/89  H.J.Ermen					**
*************************************************************************/

#include <helios.h>
#include <syslib.h>
#include <codes.h>
#include <message.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>

#include "fservlib.h"
#include "buf.h"
#include "inode.h"
#include "nfs.h"

char *PrgName;

/************************************************************************
 * MAIN ENTRY POINT TO CHANGE THE OPERATION MODE OF A FILE-SERVER
 *
 * - Locates the file server with the path, supplied as the first
 *   argument by the user
 * - Sends a "private" message to the server to signal a file system
 *   synchronisation request.
 *
 * Parameter  : Example:
 *		sync <pathname to fileserver>
 * Return     : Error code
 *
 ************************************************************************/
int
main ( int argc, char *argv[] )
{
 MCB m;
 word e;
 Port reply;
 char *path_to;
 word Control_V[IOCMsgMax];
 byte Data_V[IOCDataMax];

 PrgName = argv [0];

    					/* Check args for plausibility	*/
 if ( argc == (1 + 0))
 {
 	if ( ( path_to = getenv ("FILESERVER") ) == NULL )
 	{	path_to = cdobj()->Name;
/* 		fprintf (stderr, "Usage: %s <PathToVolume>\n", PrgName);
 		return 1; */
	}
 }
 else
 	path_to = argv[1];

 if ( argc > 2 )
 	fprintf (stderr, "%s: Further arguments are ignored.\n", PrgName);


 /*-----------------  Prepare MCB for marshalling  ---------------------*/

 reply = NewPort ();			/* Get a port for the reply -	*/
 					/* message			*/
					/* Basic initialisation of the	*/
					/* MesssageControlBlock		*/
 InitMCB ( &m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply,
   	   FC_GSP+SS_HardDisk+FG_Private+FO_ForceSync);

 	   	  			/* Preparing control and data	*/
 m.Control = Control_V;			/* vector			*/
 m.Data    = Data_V;
 MarshalCommon ( &m, Null(Object), path_to );

 PutMsg ( &m );				/* Send the message to the server*/

 					/* Expect the server's reply	*/
 InitMCB ( &m, MsgHdr_Flags_preserve, reply, NullPort, 0 );
 m.Timeout = MaxInt;			/* Infinite wait		*/
 e = GetMsg ( &m );
 if( e != 0 )
      fprintf(stderr,"%s: failed - code %08lx\n", PrgName, e);

 FreePort ( reply );			/* The port is not further used	*/

 return 0;				/* Normal termination 		*/
}

/* end of sync.c */
