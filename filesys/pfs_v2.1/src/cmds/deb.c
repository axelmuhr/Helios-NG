static char rcsid[] = "$Header";

/* $Log: deb.c,v $
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
** deb.c								**
**                                                                      **
**	User entry point to deb a volume, i.e. it's medium		**
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

	if (argc != (1 + 1))
	{
	 	fprintf (stderr, "Usage: %s <PathToVolume>\n", PrgName);
	 	return (1);
	}	
	if ( argc > 2 )
 		fprintf (stderr, "%s: Further arguments are ignored.\n", PrgName);
 	

 /*-----------------  Prepare MCB for marshalling  ---------------------*/
 
	reply = NewPort ();					
					/* Basic initialisation of the	*/
					/* MesssageControlBlock		*/
	InitMCB ( &m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply,
 		   FC_GSP+SS_HardDisk+FG_Private+FO_Debug);
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
	 	return (1);
	}
	elif ( e != FC_GSP+SS_HardDisk+FG_Private+FO_Debug)
	{
	 	fprintf (stderr, "deb : Unknown reply (%x).\n", e);
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "Fault (%08x): %s\n", e, FaultBuff);
	 	return (1);
	}


	m.Timeout = Infinite;
	e = GetMsg ( &m );
	FreePort ( reply );	
	if ( e < 0 ) {
	 	fprintf (stderr, "deb : communication error (%x).\n", e);
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "Fault (%08x): %s\n", e, FaultBuff);
	 	return 1;
	}
	elif ( e != FC_GSP+SS_HardDisk+FG_Private+FO_Format)
	{
	 	fprintf (stderr, "%s: Unknown reply (%08lx).\n", PrgName, e);
	 	Fault (e, FaultBuff, K);
		fprintf (stderr, "Fault (%08x): %s\n", e, FaultBuff);
	 	return 1;
	}
	return (0);

}

/* end of deb.c */
