#ifndef lint
static  char sccsid[] = "@(#)rstatxdr.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/* 
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

extern int callrpc(...);
extern int xdr_int(...);
extern int xdr_long(...);

#include <rpc/rpc.h>

#ifndef CPUSTATES
#include <sys/dk.h>

/*
 * This is a hack.  I made it 4 to try to get it to work with Suns
 * to no avail.  The important thing is that DK_NDRIVE must agree
 * with whatever rstatd uses when compiled.
 */
#undef DK_NDRIVE
#define DK_NDRIVE 4
#endif

#ifndef DST_NONE
#include <sys/time.h>
#endif

#define RSTATPROG 100001
#define RSTATVERS_ORIG 1
#define RSTATVERS_SWTCH 2
#define RSTATVERS_TIME  3
#define RSTATVERS 3
#define RSTATPROC_STATS 1
#define RSTATPROC_HAVEDISK 2

struct stats {				/* version 1 */
	int cp_time[CPUSTATES];
	int dk_xfer[DK_NDRIVE];
	unsigned v_pgpgin;	/* these are cumulative sum */
	unsigned v_pgpgout;
	unsigned v_pswpin;
	unsigned v_pswpout;
	unsigned v_intr;
	int if_ipackets;
	int if_ierrors;
	int if_opackets;
	int if_oerrors;
	int if_collisions;
};

struct statsswtch {				/* version 2 */
	int cp_time[CPUSTATES];
	int dk_xfer[DK_NDRIVE];
	unsigned v_pgpgin;	/* these are cumulative sum */
	unsigned v_pgpgout;
	unsigned v_pswpin;
	unsigned v_pswpout;
	unsigned v_intr;
	int if_ipackets;
	int if_ierrors;
	int if_opackets;
	int if_oerrors;
	int if_collisions;
	unsigned v_swtch;
	long avenrun[3];
	struct timeval boottime;
};

struct statstime {				/* version 3 */
	int cp_time[CPUSTATES];
	int dk_xfer[DK_NDRIVE];
	unsigned v_pgpgin;	/* these are cumulative sum */
	unsigned v_pgpgout;
	unsigned v_pswpin;
	unsigned v_pswpout;
	unsigned v_intr;
	int if_ipackets;
	int if_ierrors;
	int if_opackets;
	int if_oerrors;
	int if_collisions;
	unsigned v_swtch;
	long avenrun[3];
	struct timeval boottime;
	struct timeval curtime;
};

xdr_timeval(XDR* xdrs, timeval* tvp) {
	if (xdr_long(xdrs, &tvp->tv_sec) == 0)
		return 0;
	if (xdr_long(xdrs, &tvp->tv_usec) == 0)
		return 0;
	return 1;
}

int xdr_stats(XDR* xdrs, stats* statp) {
	int i;
	
	for (i = 0; i < CPUSTATES; i++)
		if (xdr_int(xdrs, &statp->cp_time[i]) == 0)
			return 0;
	for (i = 0; i < DK_NDRIVE; i++)
		if (xdr_int(xdrs, &statp->dk_xfer[i]) == 0)
			return 0;
	if (xdr_int(xdrs, &statp->v_pgpgin) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pgpgout) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pswpin) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pswpout) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_intr) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_ipackets) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_ierrors) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_opackets) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_oerrors) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_collisions) == 0)
		return 0;
	return 1;
}

int xdr_statsswtch(XDR* xdrs, statsswtch* statp) {
	int i;
	
	for (i = 0; i < CPUSTATES; i++)
		if (xdr_int(xdrs, &statp->cp_time[i]) == 0)
			return 0;
	for (i = 0; i < DK_NDRIVE; i++)
		if (xdr_int(xdrs, &statp->dk_xfer[i]) == 0)
			return 0;
	if (xdr_int(xdrs, &statp->v_pgpgin) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pgpgout) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pswpin) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pswpout) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_intr) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_ipackets) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_ierrors) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_opackets) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_oerrors) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_collisions) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_swtch) == 0)
		return 0;
	for (i = 0; i < 3; i++)
		if (xdr_long(xdrs, &statp->avenrun[i]) == 0)
			return 0;
	if (xdr_timeval(xdrs, &statp->boottime) == 0)
		return 0;
	return 1;
}

int xdr_statstime(XDR* xdrs, statstime* statp) {
	int i;
	
	for (i = 0; i < CPUSTATES; i++)
		if (xdr_int(xdrs, &statp->cp_time[i]) == 0)
			return 0;
	for (i = 0; i < DK_NDRIVE; i++)
		if (xdr_int(xdrs, &statp->dk_xfer[i]) == 0)
			return 0;
	if (xdr_int(xdrs, &statp->v_pgpgin) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pgpgout) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pswpin) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_pswpout) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_intr) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_ipackets) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_ierrors) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_opackets) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_oerrors) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->if_collisions) == 0)
		return 0;
	if (xdr_int(xdrs, &statp->v_swtch) == 0)
		return 0;
	for (i = 0; i < 3; i++)
		if (xdr_long(xdrs, &statp->avenrun[i]) == 0)
			return 0;
	if (xdr_timeval(xdrs, &statp->boottime) == 0)
		return 0;
	if (xdr_timeval(xdrs, &statp->curtime) == 0)
		return 0;
	return 1;
}

int rstat(const char* host, statstime* statp) {
	return callrpc(host, RSTATPROG, RSTATVERS, RSTATPROC_STATS,
	    xdr_void, 0, xdr_statstime, statp) != RPC_SUCCESS ? -1 : 0;
}

int havedisk(const char* host) {
	long have;
	
	if (callrpc(host, RSTATPROG, RSTATVERS, RSTATPROC_HAVEDISK,
	    xdr_void, 0, xdr_long,  &have) < 0)
		return -1;
	else
		return have;
}
