/*************************************************************************
**                                                                      **
**                  H E L I O S   F I L E S E R V E R                   **
**                  ---------------------------------                   **
**                                                                      **
**                  Copyright (C) 1991 Parsytec GmbH                    **
**                         All Rights Reserved.                         **
**                                                                      **
** mksuper.c								**
**                                                                      **
**	User entry point to build a superblock on a structured volume   **
**									**
**************************************************************************
** HISTORY  :             						**
**-----------                                                           **
** Author   : 14/08/90  G.Lauven					**
*************************************************************************/


#include "nfs.h"


/************************************************************************
 * MAIN ENTRY POINT FOR BUILDING A SUPERBLOCK 
 *
 * - Locates the file server volume with the path, supplied as the first 
 *   argument by the user
 * - Sends a "private" message to the server to build a superblock according
 *   to the devinfo file
 *
 * Parameter  : Pathname to the server volume
 *		Example : mksuper <pathname>
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
 		   FC_GSP+SS_HardDisk+FG_Private+FO_MakeSuper);
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
	 	fprintf (stderr, "%s: Failed to reach server (%x).\n", PrgName, e);
	 	return 1;
	}
	elif ( e != FC_GSP+SS_HardDisk+FG_Private+FO_MakeSuper ) {
	 	fprintf (stderr, "%s: Unknown reply (%x).\n", PrgName, e);
	 	return 1;
	}
	else {
		data = (filesys_data *) m.Control;
		switch (data->status) {
		case MAKE_ERR : 
			fprintf (stderr,"%s:\tFailed to build superblock on volume %s.\n", PrgName, argv[1]);
			fprintf (stderr,"\tWatch debug screen for details.\n");
			break;
		case MAKE_OK : 
			fprintf (stderr,"%s:\tSuperblock on volume %s successfully built.\n", PrgName, argv[1]);
			fprintf (stderr,"\t\tNumber of cylindergroups : %5d\n",data->cgs);
			fprintf (stderr,"\t\tBlocks per cylindergroup : %5d\n",data->bpcg);
			break;
		case MAKE_PROTECTED : 
			fprintf (stderr,"%s:\tCould not build superblock on volume %s because volume is \n", PrgName, argv[1]);
			fprintf (stderr,"\twriteprotected.\n");
			break;
		case MAKE_HFS_ACTIVE : 
			fprintf (stderr,"%s:\tCould not build superblock on volume %s because volume is active.\n", PrgName, argv[1]);
			fprintf (stderr,"\tUnload volume and perform a load -m %s before %s.\n",argv[1], PrgName);
			break;
		default :
			fprintf (stderr,"%s:\tUnknown status information.\n", PrgName);
		}
	}
}

/* end of makefs.c */
