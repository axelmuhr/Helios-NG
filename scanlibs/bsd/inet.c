/* $Id: inet.c,v 1.2 1993/04/20 09:27:41 nickc Exp $ */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

extern int inet_netof(struct in_addr in)
{
	int a = (int)ntohl(in.s_addr);
	if( IN_CLASSA(a) ) return (a&IN_CLASSA_NET)>>IN_CLASSA_NSHIFT;
	if( IN_CLASSB(a) ) return (a&IN_CLASSB_NET)>>IN_CLASSB_NSHIFT;
	if( IN_CLASSC(a) ) return (a&IN_CLASSC_NET)>>IN_CLASSC_NSHIFT;
	return -1;
}

extern int inet_lnaof(struct in_addr in)
{
	int a = (int)ntohl(in.s_addr);
	if( IN_CLASSA(a) ) return (a&IN_CLASSA_HOST);
	if( IN_CLASSB(a) ) return (a&IN_CLASSB_HOST);
	if( IN_CLASSC(a) ) return (a&IN_CLASSC_HOST);
	return -1;
}

extern struct in_addr inet_makeaddr(int net, int lna)
{
	long a;
	struct in_addr in;
	if( net <= 0x7f ) a = ((long)net<<IN_CLASSA_NSHIFT) | lna;
	else if( net <= 0x3fff ) a = 0x80000000 | ((long)net<<IN_CLASSB_NSHIFT) | lna;
	else a = 0xc0000000 | ((long)net<<IN_CLASSC_NSHIFT) | lna;
	in.s_addr = htonl(a);
	return in;
}
