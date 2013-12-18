#include <stdlib.h>
#include <stdio.h>
#include <syslib.h>
#include <gsp.h>
#include <codes.h>

#include "types.h"
#define MAXPATHLEN 1024
#define UFSOPENFILE
#include "../private.h"

struct UfsOpenFile **getopenfiles(char *dir, int *max);

/*
 * Program to signal a UFS file server to terminate.
 *
 * Usage :	termufs [-now|hard|soft] pathname
 *
 * where	now	will shut down file server, not accepting
 *			any new requests, but wait for all clients to
 *			terminate.  That is, existing open files must
 *			be closed by the client.  New files cannot be
 *			opened.
 *		hard	will force all open files to close immediately
 *			and terminate the file server.
 *		soft	will shut down the file server if it does not receive
 *			any new requests (excluding existing open file
 *			requests) within 20 seconds.  That is, if the
 *			indirect request dispatcher does not receive a
 *			a request within 20 seconds, every 20 seconds,
 *			a shutdown with the now option will be initiated.
 *
 *		die	Undocumented shutdown as quick as possible.
 *
 * Return	Error code
 *
 * $Id: termufs.c,v 1.1 1992/09/16 10:01:43 al Exp $
 * $Log: termufs.c,v $
 * Revision 1.1  1992/09/16  10:01:43  al
 * Initial revision
 *
 */

char *progname;

void usage(void)
{
	fprintf(stderr,"Usage: %s [-soft|now|hard] path\n",progname);
	Exit(1);
}

int main ( int argc, char **argv )
{
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;
	int severity = UfsTermSoft;
	struct UfsOpenFile **ofs;
	struct UfsOpenFile *of;
	int max, index;

	progname = argv[0] + strlen(argv[0]) - 1;
	while ((*progname != '/') && (progname >= argv[0]))
		progname--;
	progname++;
	argc--;	argv++;

	/* Parse the arguments */
	while ((**argv == '-') && (argc)) {
		if (!strcmp(*argv,"-now"))
			severity = UfsTermNow;
		elif (!strcmp(*argv,"-soft"))
			severity = UfsTermSoft;
		elif (!strcmp(*argv,"-hard"))
			severity = UfsTermHard;
		elif (!strcmp(*argv,"-die"))
			severity = UfsTermDie;
		else usage();
		argc--;	argv++;
	}
	
    	/* Check args for plausibility	*/
	if (argc != 1) usage();

	/* Any open files ? */
	if (severity != UfsTermHard) {
	    /* Get the open files */
	    if ((ofs = getopenfiles(argv[0],&max)) != NULL) {
		fprintf(stderr,"%s Warning:  These files are still open\n");
		fprintf(stderr,"PID       UID   Filename\n");
		for (index=0; ofs[index]; index++) {
			of = ofs[index];
			fprintf(stderr,"%3d  %8d   %s\n",
				of->pid,of->uid,of->name);
			Free(of);
		}
		Free(ofs);

		/* Confirm request ! */
		fprintf(stderr,"CONTINUE SHUTDOWN (Y/N) ? ");
		fflush(stderr);
		if (tolower(getchar()) != 'y') {
			while (getchar() != '\n');
			fprintf(stderr,"SHUTDOWN ABORTED\n");
			Exit(0);
		}
		while (getchar() != '\n');
	    }
	}
	
	/* Prepare MCB for marshalling */
 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = Data_V; 	   
	m.Timeout = IOCTimeout;

	InitMCB (&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
		FC_GSP | SS_HardDisk | FG_Terminate | severity);
	MarshalCommon ( &m, Null(Object), argv[0] );          
 
	/* Send the message to the server*/
	e = PutMsg(&m);
	if (e != Err_Null) {
		fprintf(stderr, "%s : Can't send message to server %s : Fault %x\n",
 			progname, argv[0], e);
		FreePort(reply);
		Exit(1);
	}
 	
	/* Expect termination signal response from the ufs file-server. */
	InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
	m.Timeout = IOCTimeout;
	e = GetMsg(&m);
	FreePort(reply);
 
	if ((e & EG_Mask) == EG_Unknown) {
 		fprintf(stderr,"%s: Failed to shutdown %s.  Not found\n",
 			progname, argv[0]); 
		Exit(1);
	} else if (e < 0) {
 		fprintf(stderr,"%s: Failed to shutdown %s.  Fault %x\n",
 			progname, argv[0], e); 
		Exit(1);
	}
	Exit(0);
}

