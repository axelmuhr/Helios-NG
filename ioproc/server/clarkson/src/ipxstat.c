/*
 * IPXSTAT - display statistics from the IPXPKT driver
 * Compile with: tcc -ml -eipxstat ipxstat.c
 * Usage: ipxstat <vector_num>
 *
 */

#include <stdio.h>
#include <dos.h>

#ifndef VERSION
#define VERSION	2
#endif

struct rte {
#if VERSION > 1
	unsigned char rt_ether[6];
#endif
	unsigned char rt_net[4];
	unsigned char rt_node[6];
	unsigned char rt_gate[6];
	unsigned char rt_x_type;
	unsigned int rt_use;
};

struct ipxstat {
	unsigned long queue_full;
	unsigned long route_drop;
	unsigned long scache_miss;
	unsigned long rcache_miss;
	unsigned long route_loops;
	unsigned long route_lookups;
};

static int version = VERSION;

atoh(s)
char *s;
/* from KA9Q, netuser.c */
{
	int ret = 0;
	char c;
	while((c = *s++) != '\0'){
		c &= 0x7f;
		if(c == 'x')
			continue;	/* Ignore 'x', e.g., '0x' prefixes */
		if(c >= '0' && c <= '9')
			ret = ret*16 + (c - '0');
		else if(c >= 'a' && c <= 'f')
			ret = ret*16 + (10 + c - 'a');
		else if(c >= 'A' && c <= 'F')
			ret = ret*16 + (10 + c - 'A');
		else
			break;
	}
	return ret;
}

main(argc, argv)
int argc;
char *argv[];
{
	int i, int_no, rt_count;
	char far *(far *interrupts)[];
	struct rte far *rte, far *rte_end;
	struct ipxstat far *ipxstat;

	interrupts = (char far *(far *)[]) 0;

	if (argc < 2) {
		printf("Give int no\n");
		exit(1);
	}
	if (strncmp("0x", argv[1], 2) == 0)
		int_no = atoh(argv[1]+2);
	else
		int_no = atoi(argv[1]);
	if (strcmp((*interrupts)[int_no] + 3, "PKT DRVR")) {
		printf("No signature at %#x\n", int_no);
		exit(1);
	}

	for (i=0; i<10000; i++)
		if (strcmp((*interrupts)[int_no] + i, "RTE_TABL") == 0) break;

	printf("RTE found at %4x:%4x\n", FP_SEG((*interrupts)[int_no]+i),
					 FP_OFF((*interrupts)[int_no]+i));

	i += sizeof("RTE_TABL");
	rt_count = * ((*interrupts)[int_no] + i);
	printf("Max route entries: %d\n", rt_count);
	rte = (struct rte far *) ((*interrupts)[int_no] + i + 1);
	rte_end = (struct rte far *) MK_FP(FP_SEG(rte), *(int far *)(rte + rt_count));

	printf("RTE end at %4x:%4x\n", FP_SEG(rte_end), FP_OFF(rte_end));

#if VERSION < 2
	printf("Net          Node               Gate               Type Age\n");
#else
	printf("Ether              Net          Node               Gate               Type Age\n");
#endif

	for (; rte < rte_end; rte++)

#if VERSION < 2
		printf("%02x:%02x:%02x:%02x  %02x:%02x:%02x:%02x:%02x:%02x  %02x:%02x:%02x:%02x:%02x:%02x  %2x  %u\n",
		rte->rt_net[0], rte->rt_net[1], rte->rt_net[2], rte->rt_net[3],
		rte->rt_node[0], rte->rt_node[1], rte->rt_node[2],
		rte->rt_node[3], rte->rt_node[4], rte->rt_node[5],
		rte->rt_gate[0], rte->rt_gate[1], rte->rt_gate[2],
		rte->rt_gate[3], rte->rt_gate[4], rte->rt_gate[5],
		rte->rt_x_type, rte->rt_use);
#else
		printf("%02x:%02x:%02x:%02x:%02x:%02x  %02x:%02x:%02x:%02x  %02x:%02x:%02x:%02x:%02x:%02x  %02x:%02x:%02x:%02x:%02x:%02x  %2x  %u\n",
		rte->rt_ether[0], rte->rt_ether[1], rte->rt_ether[2],
		rte->rt_ether[3], rte->rt_ether[4], rte->rt_ether[5],
		rte->rt_net[0], rte->rt_net[1], rte->rt_net[2], rte->rt_net[3],
		rte->rt_node[0], rte->rt_node[1], rte->rt_node[2],
		rte->rt_node[3], rte->rt_node[4], rte->rt_node[5],
		rte->rt_gate[0], rte->rt_gate[1], rte->rt_gate[2],
		rte->rt_gate[3], rte->rt_gate[4], rte->rt_gate[5],
		rte->rt_x_type, rte->rt_use);
#endif

/* next the stats */
	for (i=0; i<10000; i++)
		if (strcmp((*interrupts)[int_no] + i, "IPXSTAT") == 0) break;
	if (i>=10000) printf("STATS not found\n"), exit(1);

	printf("STATS found at %04x:%04x\n", FP_SEG((*interrupts)[int_no]+i),
					 FP_OFF((*interrupts)[int_no]+i));

	i += sizeof("IPXSTAT");
	ipxstat = (struct ipxstat *) ((*interrupts)[int_no] + i);
	printf("IPX stats.\n");
	printf("queue full: %lu\n", ipxstat->queue_full);
	printf("route drops: %lu\n", ipxstat->route_drop);
	printf("send cache misses: %lu\n", ipxstat->scache_miss);
	printf("receive cache misses: %lu\n", ipxstat->rcache_miss);
	printf("total route lookups: %lu, loops: %lu, (%f loops/lookup)\n",
		ipxstat->route_lookups, ipxstat->route_loops,
		(float) (ipxstat->route_loops) / (float) (ipxstat->route_lookups?ipxstat->route_lookups:1));

}
