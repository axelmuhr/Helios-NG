static char rcsid[] = "$Header";

/* $Log: format.c,v $
 * Revision 1.1  1992/07/13  16:18:43  craig
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
** format.c								**
**                                                                      **
**	User entry point to format a volume, i.e. it's medium		**
**									**
**************************************************************************
** HISTORY  :             						**
**-----------                                                           **
** Author   : 03/09/90  G.Lauven					**
*************************************************************************/


#include <fault.h>
#include <stdio.h>
#include "misc.h"
#include "nfs.h"

#define Infinite (-1)


/************************************************************************
 * MAIN ENTRY POINT FOR FORMATTING A VOLUME
 *
 * - Locates the file server volume with the path, supplied as the first 
 *   argument by the user
 * - Sends a message to the server to format the volume
 *
 * Parameter  : Pathname to the server volume
 *		Example : format <pathname>
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
	char FaultBuff [K];
	  
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
 		   FC_GSP+SS_HardDisk+FG_Private+FO_Format);
 	   				/* Preparing control and data	*/
	m.Control = Control_V;		/* vector			*/
	m.Data    = Data_V; 	   
	MarshalCommon ( &m, Null(Object), argv[1] );          
 
	PutMsg ( &m );	/* Send the message to the server*/

	InitMCB ( &m, MsgHdr_Flags_preserve, reply, NullPort, 0 );
	m.Timeout = OneSec * 10;
	e = GetMsg ( &m );
	if ( e < 0 )
	{
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "%s: %s\n", PrgName, FaultBuff);
		FreePort ( reply );	
	 	return (1);
	}
	elif ( e != FC_GSP+SS_HardDisk+FG_Private+FO_Format )
	{
	 	fprintf (stderr, "%s: Unknown reply.\n", PrgName);
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "Fault (%08x): %s\n", e, FaultBuff);
		FreePort ( reply );	
	 	return (1);
	}
	else {
		fprintf (stderr,"%s:\tVolume %s being formatted. Depending on medium this may\n",PrgName,argv[1]);
		fprintf (stderr,"\tlast up to 45 minutes.\n");
	}

	m.Timeout = Infinite;

#if 0
	do
	{
IOdebug ("format: waiting 4 a message");	  
	  e = GetMsg (&m);
IOdebug ("format: received a message, e (%x)", e);	  
	  if (e == (FC_GSP + SS_HardDisk + FG_Private + FO_Format))
	  {
	    data = (filesys_data *) m.Control;
	    if (data->status == FORMAT_KEEP_OPEN)
	      IOdebug ("%s: Keeping port open", PrgName);
	  }
	}
	while (   (e == (FC_GSP + SS_HardDisk + FG_Private + FO_Format)) 
	       && (data->status == FORMAT_KEEP_OPEN));
	
#else
	  e = GetMsg (&m);
#endif

	FreePort ( reply );	
	if ( e < 0 ) {
	 	fprintf (stderr, "%s: Communication error.\n", PrgName);
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "Fault (%08x): %s\n", e, FaultBuff);
	 	return 1;
	}
	elif ( e != FC_GSP+SS_HardDisk+FG_Private+FO_Format)
	{
	 	fprintf (stderr, "%s: Unknown reply.\n", PrgName);
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "Fault (%08x): %s\n", e, FaultBuff);
	 	return 1;
	}
	else {
		data = (filesys_data *) m.Control;
		switch (data->status) {
		case MAKE_ERR :
			fprintf (stderr,"%s:\tFailed to format volume %s.\n", PrgName, argv[1]);
			fprintf (stderr,"\tWatch debug screen for detailed information.\n");
			break;
		case MAKE_OK :
			fprintf (stderr,"%s:\tVolume %s successfully formatted.\n", PrgName, argv[1]);
			break;
		case MAKE_HFS_ACTIVE :
			fprintf (stderr,"%s:\tCould not format volume %s because volume is active.\n", PrgName, argv[1]);
			fprintf (stderr,"\tUnload volume and perform a load -m %s before formatting.\n",argv[1]);
			break;
		default :
			fprintf (stderr,"%s: Unknown status information.\n", PrgName);
		}	
		return (0);
	}
}

/* end of format.c */

