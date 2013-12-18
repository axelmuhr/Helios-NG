#include "helios.h"
#include <stdlib.h>
#include <stdio.h>
#include <syslib.h>
#include <gsp.h>
#include <codes.h>
#include "../private.h"

/*
 * Program to sync the passed filesystem every x seconds.
 *
 * Usage :	update [-s time] filesystem [filesystem ...]
 *
 * Return	Error code
 *
 * $Id: update.c,v 1.1 1992/09/16 10:01:43 al Exp $
 * $Log: update.c,v $
 * Revision 1.1  1992/09/16  10:01:43  al
 * Initial revision
 *
 */

char *progname;

void usage(void)
{
	fprintf(stderr,"Usage: %s [-s time] filesystem [filesystem ...]\n",
			progname);
	Exit(1);
}

int main ( int argc, char **argv )
{	MCB m;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	int secs = 30;
	char *ch;
	Port reply;
	int i, e;

	/* Get the program name */
	progname = argv[0] + strlen(argv[0]) - 1;
	while ((*progname != '/') && (progname >= argv[0]))
		progname--;
	progname++;
	argc--;	argv++;

	/* Check the 1st argument for new seconds */
	if (!strcmp(*argv,"-s")) {
		argc--;	argv++;
		if (!argc) usage();
	
		secs = 0; ch = *argv;
		while (*ch) {
			secs = (secs*10) + (*ch - '0');
			ch++;
		}

		/* Any errors */
		if (secs < 1) {
			fprintf(stderr,"%s Invalid number of seconds\n",
				progname);
			Exit(1);
		}

		argc--;	argv++;
	}
	
    	/* Check args for plausibility	*/
	if (argc < 1) usage();

	/* Prepare MCB for marshalling */
 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = Data_V;
	m.Timeout = IOCTimeout;

	for (;;) {
	    /* Wait a while */
	    Delay(OneSec * secs);

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

	/* NOT REACHED */
}

