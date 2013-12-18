#include <stdlib.h>
#include <stdio.h>
#include <syslib.h>
#include <gsp.h>
#include <codes.h>

#include "param.h"
#include "malloc.h"
#include "../private.h"

/*
 * Program to print the file system memory usage.
 *
 * Usage :	ufsmem pathname
 *
 * Return	Error code
 *
 * $Id: ufsmem.c,v 1.1 1992/09/16 10:01:43 al Exp $
 * $Log: ufsmem.c,v $
 * Revision 1.1  1992/09/16  10:01:43  al
 * Initial revision
 *
 */

char *progname;

void usage(void)
{
	fprintf(stderr,"Usage: %s path\n",progname);
	Exit(1);
}

/*
 * Display Memory Statistics
 */
void showmemdata(char *str, struct kmemstats *ksp)
{
	printf("%s   %8d   %8d   %8d   %8d\n",
	str,ksp->ks_memuse,ksp->ks_maxused,ksp->ks_calls,ksp->ks_inuse);
}

int main ( int argc, char **argv )
{
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	IOCGetmemReply *rep = (IOCGetmemReply *)&Control_V;
	struct kmemstats kmemstats[M_LAST];
	Port reply;

	progname = argv[0] + strlen(argv[0]) - 1;
	while ((*progname != '/') && (progname >= argv[0]))
		progname--;
	progname++;
	argc--;	argv++;

    	/* Check args for plausibility	*/
	if (argc != 1) usage();

	/* Prepare MCB for marshalling */
 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = (byte *)&kmemstats; 	   
	m.Timeout = IOCTimeout;

	InitMCB (&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
		FC_GSP | SS_HardDisk | FG_UfsGetmem);
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
 
 	if (e < 0) {
		fprintf(stderr,"%s: %s %s.  Fault %x\n",
 			progname, "Failed to get memory statistics from",
			argv[0], e); 
		Exit(1);
	}

	/* Print the Usage */
	printf("HELIOS FILE SYSTEM USAGE");
#if 0
	printf("Cache Hits\t\%d\tCache Misses\t%d\t\tHit ratio\t%3d%%%\n",
		rep->cache_hits,rep->cache_misses,
		(rep->cache_hits * 100) / 
			(rep->cache_hits + rep->cache_misses));
	printf("Disk Reads\t%d\tDisk Writes\t%d\t\tTotal access\t%d\n",
		rep->hd_reads,rep->hd_writes,
		rep->hd_reads + rep->hd_writes);
	printf("Active Clients\t%d\tOutstanding disk requests\t%d\n\n\n",
		rep->clients_active,rep->hd_numbufs);

#endif /* 0 */
	/* Print the memory usage */
	printf("UNIX DYNAMIC MEMORY USAGE\n");
	printf("Type       In Use     Maximum    Total Calls   Current Calls\n");

	showmemdata("M_FREE    ",&kmemstats[M_FREE]);
	showmemdata("M_MBUF    ",&kmemstats[M_MBUF]);
	showmemdata("M_DEVBUF  ",&kmemstats[M_DEVBUF]);
	showmemdata("M_SOCKET  ",&kmemstats[M_SOCKET]);
	showmemdata("M_PCB     ",&kmemstats[M_PCB]);
	showmemdata("M_RTABLE  ",&kmemstats[M_RTABLE]);
	showmemdata("M_HTABLE  ",&kmemstats[M_HTABLE]);
	showmemdata("M_FTABLE  ",&kmemstats[M_FTABLE]);
	showmemdata("M_ZOMBIE  ",&kmemstats[M_ZOMBIE]);
	showmemdata("M_IFADDR  ",&kmemstats[M_IFADDR]);
	showmemdata("M_SOOPTS  ",&kmemstats[M_SOOPTS]);
	showmemdata("M_SONAME  ",&kmemstats[M_SONAME]);
	showmemdata("M_NAMEI   ",&kmemstats[M_NAMEI]);
	showmemdata("M_GPROF   ",&kmemstats[M_GPROF]);
	showmemdata("M_IOCTLOPS",&kmemstats[M_IOCTLOPS]);
	showmemdata("M_SUPERBLK",&kmemstats[M_SUPERBLK]);
	showmemdata("M_CRED    ",&kmemstats[M_CRED]);
	showmemdata("M_PGRP    ",&kmemstats[M_PGRP]);
	showmemdata("M_SESSION ",&kmemstats[M_SESSION]);
	showmemdata("M_IOV     ",&kmemstats[M_IOV]);
	showmemdata("M_MOUNT   ",&kmemstats[M_MOUNT]);
	showmemdata("M_FHANDLE ",&kmemstats[M_FHANDLE]);
	showmemdata("M_NFSREQ  ",&kmemstats[M_NFSREQ]);
	showmemdata("M_NFSMNT  ",&kmemstats[M_NFSMNT]);
	showmemdata("M_VNODE   ",&kmemstats[M_VNODE]);
	showmemdata("M_CACHE   ",&kmemstats[M_CACHE]);
	showmemdata("M_DQUOT   ",&kmemstats[M_DQUOT]);
	showmemdata("M_UFSMNT  ",&kmemstats[M_UFSMNT]);
	showmemdata("M_MAPMEM  ",&kmemstats[M_MAPMEM]);
	showmemdata("M_SHM     ",&kmemstats[M_SHM]);
	showmemdata("M_CLIENT  ",&kmemstats[M_CLIENT]);
	showmemdata("M_MCB     ",&kmemstats[M_MCB]);
	showmemdata("M_TEMP    ",&kmemstats[M_TEMP]);

	Exit(0);
}

