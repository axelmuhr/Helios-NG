#include "helios.h"
#include <stdlib.h>
#include <stdio.h>
#include <syslib.h>
#include <gsp.h>
#include <codes.h>
#include "../private.h"

/*
 * Program to sync the passed filesystem immediately
 *
 * Usage :	ufssync filesystem [filesystem ...]
 *
 * Return	Error code
 *
 * $Id: ufssync.c,v 1.1 1992/10/19 11:36:08 al Exp $
 * $Log: ufssync.c,v $
 * Revision 1.1  1992/10/19  11:36:08  al
 * Initial revision
 *
 *
 */

char *progname;

void usage(void)
{
	fprintf(stderr,"Usage: %s filesystem [filesystem ...]\n",
			progname);
	Exit(1);
}

int main ( int argc, char **argv )
{	MCB m;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;
	int i, e;

	/* Get the program name */
	progname = argv[0] + strlen(argv[0]) - 1;
	while ((*progname != '/') && (progname >= argv[0]))
		progname--;
	progname++;
	argc--;	argv++;

    	/* Check args for plausibility	*/
	if (argc < 1) usage();

	/* Prepare MCB for marshalling */
 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = Data_V;
	m.Timeout = IOCTimeout;

	/* Now sync everyone */
	for (i=0; i<argc; i++) {
		InitMCB (&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
			FC_GSP | SS_HardDisk | FG_UfsSync);
		MarshalCommon ( &m, Null(Object), argv[i] );          
 
		/* Send the message to the server*/
		e = PutMsg(&m);
		if (e != Err_Null) {
			fprintf(stderr, "%s : Can't sync server %s : Fault %x\n",
	 			progname, argv[i], e);
		} else {
			/* Get reply */
			InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
			m.Timeout = IOCTimeout;
			e = GetMsg(&m);
		 	if (e < 0) {
				fprintf(stderr,"%s: Sync on server %s failed.  Fault %x\n",
 					progname, argv[i], e);
			}
		}
	}
}

