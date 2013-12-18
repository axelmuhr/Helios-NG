static char rcsid[] = "$Header: /usr/perihelion/Helios/filesys/cmds/RCS/termfs.c,v 1.1 90/10/05 16:40:40 nick Exp $";

/* $Log:	termfs.c,v $
 * Revision 1.1  90/10/05  16:40:40  nick
 * Initial revision
 * 
 * Revision 1.1  90/01/09  13:32:56  chris
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
** termfs.c								**
**                                                                      **
**	User entry point to terminate the file server			**
**									**
**************************************************************************
** HISTORY  :             						**
**-----------                                                           **
** Author   : 01/04/89  H.J.Ermen					**
*************************************************************************/


#include "nfs.h"


/************************************************************************
 * MAIN ENTRY POINT FOR FILE SERVER TERMINATION
 *
 * - Locates the file server with the path, supplied as the first 
 *   argument by the user
 * - Sends a "private" message to the server to signal a terminate request
 *
 * Parameter  : Pathname to the server
 *		Example : termfs <pathname>
 * Return     : Error code
 *
 ************************************************************************/
int 
main ( int argc, char *argv[] )
{
 MCB m;
 word e;
 word Control_V[IOCMsgMax];
 byte Data_V[IOCDataMax];
 Port reply;
  
    					/* Check args for plausibility	*/
 if ( argc == 1 )
 {
 	fprintf (stderr, "Usage : %s <pathname to fileserver>\n",
 		 argv[0] );
 	return 1;
 }	
 if ( argc > 2 )
 	fprintf (stderr, "%s : Further arguments are ignored !\n",
 		 argv[0] );
 	

 /*-----------------  Prepare MCB for marshalling  ---------------------*/
 
 reply = NewPort ();					
					/* Basic initialisation of the	*/
					/* MesssageControlBlock		*/
 InitMCB ( &m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply,
 	   FC_GSP+SS_HardDisk+FG_Private+FO_Terminate);
 	   				/* Preparing control and data	*/
 m.Control = Control_V;			/* vector			*/
 m.Data    = Data_V; 	   
 MarshalCommon ( &m, Null(Object), argv[1] );          
 
 e = PutMsg ( &m );			/* Send the message to the server*/
 if ( e != Err_Null )
 {
 	fprintf (stderr, "%s : Can't send message to server :%x\n",
 		 argv[0], e);
 	return 1;
 }
 					/* Expect termination signal	*/
 					/* from the file-server	 ...	*/
 InitMCB ( &m, MsgHdr_Flags_preserve, reply, NullPort, 0 );
 m.Timeout = MaxInt;
 e = GetMsg ( &m );
 FreePort ( reply );
 
 if ( m.MsgHdr.FnRc == FC_GSP+SS_HardDisk+FG_Private+FO_Terminate )
 {
	return 0;
 }
 else
 {
 	fprintf (stderr,"%s: Failed to terminate Helios filing system !!!\n",
 		 argv[0] ); 
	return 1;
 }
}

/* end of termfs.c */
