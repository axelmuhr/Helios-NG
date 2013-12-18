static char rcsid[] = "$Header: /hsrc/filesys/pfs_v2.1/src/cmds/RCS/termvol.c,v 1.1 1992/07/13 16:18:43 craig Exp $";

/* $Log: termvol.c,v $
 * Revision 1.1  1992/07/13  16:18:43  craig
 * Initial revision
 *
 * Revision 1.2  90/08/31  08:33:33  guenter
 * minor bug fixed
 * 
 * Revision 1.1  90/08/29  13:16:14  guenter
 * Initial revision
 * 
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
** termvol.c								**
**                                                                      **
**	User entry point to terminate the file server			**
**									**
**************************************************************************
** HISTORY  :             						**
**-----------                                                           **
** Author   : 01/04/89  H.J.Ermen					**
*************************************************************************/


#include "fault.h"
#include "misc.h"
#include "nfs.h"

char *PrgName;

/************************************************************************
 * MAIN ENTRY POINT FOR FILE SERVER TERMINATION
 *
 * - Locates the file server with the path, supplied as the first 
 *   argument by the user
 * - Sends a "private" message to the server to signal a terminate request
 *
 * Parameter  : Pathname to the server
 *		-v  unload command waits until unload has completed and tells
 *		    the user about the result
 *		Example : termvol -v <pathname>
 * Return     : Error code
 *
 ************************************************************************/

typedef struct filesys_data {
	word		status;
} filesys_data;
 
int 
main ( int argc, char *argv[] )
{
	MCB m;
	word e;
	word verbose = FALSE;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;
	char *volume = NULL;	  
	filesys_data *data;
	char FaultBuff [K];

        PrgName = argv [0];
  
  	for(argv++; *argv; argv++ )
	{
		char *arg = *argv;
		if( *arg == '-' )
		{
			switch(arg[1])
			{
			case 'v' : verbose = TRUE; break;

			default  : fprintf (stderr,"Usage: %s [-v] <PathToVolume>\n", PrgName);
				   return 1;
			}
			continue;
		}
		volume = arg;
	}
	if( volume == NULL )
	{
		fprintf (stderr,"Usage: %s [-v] <PathToVolume>\n", PrgName);
		return 1;
	}

 /*-----------------  Prepare MCB for marshalling  ---------------------*/
 
	reply = NewPort ();					
					/* Basic initialisation of the	*/
					/* MesssageControlBlock		*/
	InitMCB ( &m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply,
 		   FC_GSP+SS_HardDisk+FG_Private+FO_Terminate);
 	   				/* Preparing control and data	*/
	m.Control = Control_V;		/* vector			*/
	m.Data    = Data_V; 	   
	MarshalCommon ( &m, Null(Object), volume );
	MarshalWord ( &m, 0 );
	MarshalWord ( &m, 0 );
	MarshalWord ( &m, 0 );
	MarshalWord ( &m, verbose );
	MarshalWord ( &m, 0 );
 
	PutMsg ( &m );			/* Send the message to the server*/
 	/* Expect termination signal from the file-server	 ...	*/

	InitMCB ( &m, MsgHdr_Flags_preserve, reply, NullPort, 0 );
	m.Timeout = OneSec * 10;
	e = GetMsg ( &m );

	if ( e < 0 ) {
 		fprintf (stderr, "%s: Failed to reach server.\n", PrgName);
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "%s:\tFault 0x%X:\n\t%s\n", PrgName, e, FaultBuff);
		FreePort ( reply );
	 	return 1;
	}
	elif ( e != FC_GSP+SS_HardDisk+FG_Private+FO_Terminate )
	{
  		fprintf (stderr,"%s: Unknown reply.\n", PrgName);
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "%s:\tFault 0x%X:\n\t%s\n", PrgName, e, FaultBuff);
		FreePort ( reply );
		return 1;
	}
	else {
		fprintf (stderr,"%s: Volume %s terminating.\n", PrgName, volume);
		if (!verbose)
		{
			FreePort ( reply );
			return (0);
		}
	}

	fprintf (stderr,"\tWaiting for volume %s to terminate...\n",volume);
	m.Timeout = OneSec * 60 * 2;	/* wait for two minutes */
	e = GetMsg (&m);
	FreePort ( reply );	
	if ( e < 0 ) {
	 	fprintf (stderr, "%s: Communication error.\n", PrgName);
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "%s:\tFault 0x%X:\n\t%s\n", PrgName, e, FaultBuff);
	 	return 1;
	}
	elif ( e != FC_GSP+SS_HardDisk+FG_Private+FO_Unload )
	{
	 	fprintf (stderr, "%s: Unknown reply.\n", PrgName);
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "%s:\tFault 0x%X:\n\t%s\n", PrgName, e, FaultBuff);
	 	return 1;
	}
	else {
		data = (filesys_data *) m.Control;
		switch (data->status) {
		case MAKE_ERR :
			fprintf (stderr,"%s: Forced termination of volume %s.\n", PrgName, volume);
			break;
		case MAKE_OK :
			fprintf (stderr,"%s: Volume %s terminated.\n", PrgName, volume);
			break;

		default :
			fprintf (stderr,"%s: Unknown status information (%d).\n", PrgName, data->status);
		}	
		return (0);
	}
}

/* end of termvol.c */
