static char rcsid[] = "$Header: /usr/perihelion/Helios/filesys/cmds/RCS/fsync.c,v 1.1 90/10/05 16:40:34 nick Exp $";

/* $Log:	fsync.c,v $
 * Revision 1.1  90/10/05  16:40:34  nick
 * Initial revision
 * 
 * Revision 1.1  90/01/09  13:32:53  chris
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
** fsync.c								**
**                                                                      **
**	User entry point to change the operation mode of the server	**
**									**
**************************************************************************
** HISTORY  :             						**
**-----------                                                           **
** Author   : 22/03/89  H.J.Ermen					**
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

/************************************************************************
 * MAIN ENTRY POINT TO CHANGE THE OPERATION MODE OF A FILE-SERVER
 *
 * - Locates the file server with the path, supplied as the first 
 *   argument by the user
 * - Sends a "private" message to the server to signal a file system
 *   sync/async - request.
 *
 * Parameter  : Example:
 *		fsync <pathname to fileserver> <option>
 * Return     : Error code
 *
 ************************************************************************/
int 
main ( int argc, char *argv[] )
{
 MCB m;
 word e;
 char *path_to;
 word Control_V[IOCMsgMax];
 byte Data_V[IOCDataMax];
 
    					/* Check args for plausibility	*/
 if ( argc == 1 )
 {
 	fprintf (stderr, "Usage : fsync <pathname to fileserver> <option [-sa]>\n");
 	return 1;
 }	
 
 if ( argc > 3 )
 	fprintf (stderr, "	fsync : 	Further arguments are ignored !\n");
 
 if ( argc == 2 )
 {
 	if ( ( path_to = getenv ("FILESERVER") ) == NULL )
 	{
	 	fprintf (stderr, "Usage : fsync <pathname to fileserver> <option [-sa]>\n");
 		return 1;
 	}
 }
 else
 	path_to = argv[1];	

 /*-----------------  Prepare MCB for marshalling  ---------------------*/
 					
					/* Basic initialisztion of the	*/
					/* MesssageControlBlock		*/
 switch ( toupper ( (argv[1]==path_to) ? (argv[2][1]):(argv[1][1]) ) )
 {
	case 'S' :
		 InitMCB ( &m, MsgHdr_Flags_preserve, MyTask->IOCPort, NullPort,
 	   	 FC_GSP+SS_HardDisk+FG_Private+FO_Synch);
		 break;
	case 'A' :
		 InitMCB ( &m, MsgHdr_Flags_preserve, MyTask->IOCPort, NullPort,
 	   	 FC_GSP+SS_HardDisk+FG_Private+FO_Asynch);
		 break;
	default  :
		 fprintf (stderr, "ERROR : Illegal option used\n");
		 return 1;
 }
 	   	  			/* Preparing control and data	*/
 m.Control = Control_V;			/* vector			*/
 m.Data    = Data_V; 	   
 MarshalCommon ( &m, Null(Object), path_to );          
 
 e = PutMsg ( &m );			/* Send the message to the server*/
 return 0;				/* Normal termination 		*/
}

/* end of fsync.c */
