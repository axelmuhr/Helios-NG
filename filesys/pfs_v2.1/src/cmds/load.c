static char rcsid[] = "$Header: /hsrc/filesys/pfs_v2.1/src/cmds/RCS/load.c,v 1.1 1992/07/13 16:18:43 craig Exp $";

/* $Log: load.c,v $
 * Revision 1.1  1992/07/13  16:18:43  craig
 * Initial revision
 *
 * Revision 1.1  90/08/31  08:33:21  guenter
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
** load.c								**
**                                                                      **
**	User entry point to load a volume, i.e. it's medium		**
**									**
**************************************************************************
** HISTORY  :             						**
**-----------                                                           **
** Author   : 14/08/90  G.Lauven					**
*************************************************************************/


#include "fault.h"
#include "misc.h"
#include "nfs.h"


/************************************************************************
 * MAIN ENTRY POINT FOR LOADING A VOLUME
 *
 * - Locates the file server volume with the path, supplied as the first 
 *   argument by the user
 * - Sends a "private" message to the server to load the volume
 *
 * Parameter  : Pathname to the server volume
 *		-f  checker performs full checks (default)
 *		-b  checker performs basic checks
 *		-n  checker is bypassed completely
 *		-l  after a full check 'hanging' links are removed
 *		    (default is not to remove them)
 *		-m  volume is loaded without trying to get a filesystem
 *		    (has to be done before a makefs or format command)
 *		-v  load command waits until load has completed and tells
 *		    the user about the result
 *		Example : load -m -b <pathname>
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
	word make_flag = FALSE;
	word touched = FALSE;
	word verbose = FALSE;
	word checker_mode = FULL;
	word delete_hanging_links = FALSE;
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
			case 'm' : make_flag = TRUE; break;
			case 'f' : checker_mode = FULL;
				   touched = TRUE;
				   break;	
			case 'b' : checker_mode = BASIC;
				   touched = TRUE;
				   break;
			case 'n' : checker_mode = NO;
				   touched = TRUE;
				   break;
			case 'l' : delete_hanging_links = TRUE;
				   break;
			default  : fprintf (stderr,"Usage: %s [-l][-m][-v][-b|-f|-n] <PathToVolume>\n", PrgName);
				   return 1;
			}
			continue;
		}
		volume = arg;
	}
	if( volume == NULL )
	{
		fprintf (stderr,"Usage: %s [-l][-m][-v][-b|-f|-n] <PathToVolume>\n", PrgName);
		return 1;
	}

 /*-----------------  Prepare MCB for marshalling  ---------------------*/
 
	if ( make_flag )
		verbose = FALSE;
	reply = NewPort ();					
					/* Basic initialisation of the	*/
					/* MesssageControlBlock		*/
	InitMCB ( &m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply,
 		   FC_GSP+SS_HardDisk+FG_Private+FO_Load);
 	m.Data    = Data_V;		/* Preparing control and data	*/
	m.Control = Control_V;		/* vector			*/
	MarshalCommon ( &m, Null(Object), volume );          
	MarshalWord ( &m, touched );
	MarshalWord ( &m, checker_mode );
	MarshalWord ( &m, make_flag );
	MarshalWord ( &m, verbose );
	MarshalWord ( &m, delete_hanging_links);
	PutMsg ( &m );	/* Send the message to the server*/

 /*-----------------  wait for reply  ---------------------*/

	InitMCB ( &m, MsgHdr_Flags_preserve, reply, NullPort, 0 );
	m.Timeout = OneSec * 10;
	e = GetMsg ( &m );
	if (e < 0)
	{
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "%s:\tFault 0x%X:\n\t%s\n", PrgName, e, FaultBuff);
		FreePort ( reply );	
	 	return (1);
	}
	elif ( e != FC_GSP+SS_HardDisk+FG_Private+FO_Load )
	{
	 	fprintf (stderr, "%s: Unknown reply.\n", PrgName);
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "Fault (%08x): %s\n", e, FaultBuff);
		FreePort ( reply );	
	 	return (1);
	}
	else {
		fprintf (stderr,"%s:\tVolume %s loading.\n", PrgName, volume);
		if ( !verbose )
		{
			return (0);
		}
	}
	
	fprintf (stderr,"\tWaiting for volume %s to finish loading...\n",volume);
	m.Timeout = OneSec * 60 * 15;
	e = GetMsg (&m);
	if ( e < 0 ) {
	 	fprintf (stderr, "%s:\tCommunication error.\n", PrgName);
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "Fault (%08x): %s\n", e, FaultBuff);
		FreePort ( reply );	
	 	return 1;
	}
	elif ( e != FC_GSP+SS_HardDisk+FG_Private+FO_Load )
	{
	 	fprintf (stderr, "%s: Unknown reply.\n", PrgName);
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "Fault (%08x): %s\n", e, FaultBuff);
		FreePort ( reply );	
	 	return 1;
	}
	else {
		data = (filesys_data *) m.Control;
		switch (data->status) {
		case MAKE_ERR :
			fprintf (stderr,"%s:\tFailed to init filesystem on volume %s.\n", PrgName, volume);
			fprintf (stderr,"\tWatch debug screen for detailed information.\n");
			break;
		case MAKE_OK :
			fprintf (stderr,"%s:\tFilesystem on volume %s successfully set up.\n", PrgName, volume);
			fprintf (stderr,"\t\tNumber of cylindergroups : %5d\n",data->cgs);
			fprintf (stderr,"\t\tBlocks per cylindergroup : %5d\n",data->bpcg);
			break;
		case MAKE_TAPE_ERR : 
			fprintf (stderr,"%s:\tFailed to init raw volume %s.\n",PrgName, volume);
			fprintf (stderr,"\tWatch debug screen for detailed information.\n");
			break;
		case MAKE_TAPE_OK : 
			fprintf (stderr,"%s:\tRaw volume %s successfully loaded.\n", PrgName, volume);
			break;

		default :
			fprintf (stderr,"%s:\tUnknown status information (%d).\n", PrgName, data->status);
		}	
		return (0);
	}
}

/* end of load.c */
