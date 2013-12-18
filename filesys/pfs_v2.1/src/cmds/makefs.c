static char rcsid[] = "$Header: /hsrc/filesys/pfs_v2.1/src/cmds/RCS/makefs.c,v 1.1 1992/07/13 16:18:43 craig Exp $";

/* $Log: makefs.c,v $
 * Revision 1.1  1992/07/13  16:18:43  craig
 * Initial revision
 *
 * Revision 1.1  90/08/29  13:16:43  guenter
 * Initial revision
 * 
 * 
 */
 
/*************************************************************************
**                                                                      **
**                  H E L I O S   F I L E S E R V E R                   **
**                  ---------------------------------                   **
**                                                                      **
**                  Copyright (C) 1990 Parsytec GmbH                    **
**                         All Rights Reserved.                         **
**                                                                      **
** makefs.c								**
**                                                                      **
**	User entry point to build a filesystem on a structured volume   **
**									**
**************************************************************************
** HISTORY  :             						**
**-----------                                                           **
** Author   : 14/08/90  G.Lauven					**
*************************************************************************/


#include "nfs.h"


/************************************************************************
 * MAIN ENTRY POINT FOR BUILDING A FILESYSTEM 
 *
 * - Locates the file server volume with the path, supplied as the first 
 *   argument by the user
 * - Sends a "private" message to the server to build a filesystem according
 *   to the devinfo file
 *
 * Parameter  : Pathname to the server volume
 *		Example : makefs <pathname>
 * Return     : Error code
 *
 ************************************************************************/

typedef struct filesys_data {
	word		status;
	word		cgs;
	word		bpcg;
} filesys_data;

char *PrgName;
 
int 
main ( int argc, char *argv[] )
{
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;
	filesys_data *data;
	
	PrgName = argv [0];
  
	/* Check args for plausibility	*/
	if ( argc == 1 ) {
	 	fprintf (stderr, "Usage: %s <PathToVolume>\n", PrgName);
	 	return 1;
	}	
	if ( argc > 2 )
 		fprintf (stderr, "%s: Further arguments are ignored.\n", PrgName);
 	

 /*-----------------  Prepare MCB for marshalling  ---------------------*/
 
	reply = NewPort ();					
					/* Basic initialisation of the	*/
					/* MesssageControlBlock		*/
	InitMCB ( &m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply,
 		   FC_GSP+SS_HardDisk+FG_Private+FO_MakeFs);
 	   				/* Preparing control and data	*/
	m.Control = Control_V;			/* vector			*/
	m.Data    = Data_V; 	   
	MarshalCommon ( &m, Null(Object), argv[1] );          
 
	PutMsg ( &m );	/* Send the message to the server*/

	InitMCB ( &m, MsgHdr_Flags_preserve, reply, NullPort, 0 );
	m.Timeout = OneSec*20;
	e = GetMsg ( &m );
	FreePort ( reply );
 
	if ( e < 0 ) {
	 	fprintf (stderr, "%s:\tFailed to reach server (%x).\n", PrgName, e);
	 	return 1;
	}
	elif ( e != FC_GSP+SS_HardDisk+FG_Private+FO_MakeFs ) {
	 	fprintf (stderr, "%s: Unknown reply (%x).\n", PrgName, e);
	 	return 1;
	}
	else {
		data = (filesys_data *) m.Control;
		switch (data->status) {
		case MAKE_ERR : 
			fprintf (stderr,"%s:\tFailed to build filesystem on volume %s.\n", PrgName, argv[1]);
			fprintf (stderr,"\tWatch debug screen for details.\n");
			break;
		case MAKE_OK : 
			fprintf (stderr,"%s:\tFilesystem on volume %s successfully built.\n", PrgName, argv[1]);
			fprintf (stderr,"\t\tNumber of cylindergroups : %5d\n",data->cgs);
			fprintf (stderr,"\t\tBlocks per cylindergroup : %5d\n",data->bpcg);
			break;
		case MAKE_PROTECTED : 
			fprintf (stderr,"%s:\tCould not build filesystem on volume %s because volume is \n", PrgName, argv[1]);
			fprintf (stderr,"\twriteprotected.\n");
			break;
		case MAKE_HFS_ACTIVE : 
			fprintf (stderr,"%s:\tCould not build filesystem on volume %s because volume is active.\n", PrgName, argv[1]);
			fprintf (stderr,"\tUnload volume and perform a load -m %s before makefs.\n",argv[1]);
			break;
		default :
			fprintf (stderr,"%s:\tUnknown status information.\n", PrgName);
		}
	}
}

/* end of makefs.c */
