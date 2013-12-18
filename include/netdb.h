/* netdb.h : network database info.					*/
/* %W% %G% (C) Copyright 1990, Perihelion Software			*/
/* $Id: netdb.h,v 1.4 91/02/27 14:11:18 nick Exp $ */

#ifndef _NETDB_H_
#define _NETDB_H_

#define MAXHOSTNAMELEN	32

struct	hostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* address */
};
#define h_addr h_addr_list[0]

struct	netent {
	char	*n_name;	/* official name of net */
	char	**n_aliases;	/* alias list */
	int	n_addrtype;	/* net address type */
	int	n_net;		/* network # */
};

struct	servent {
	char	*s_name;	/* official service name */
	char	**s_aliases;	/* alias list */
	int	s_port;		/* port # */
	char	*s_proto;	/* protocol to use */
};

struct	protoent {
	char	*p_name;	/* official protocol name */
	char	**p_aliases;	/* alias list */
	int	p_proto;	/* protocol # */
};

/* Perihelion Health Warning: perusal of the following code can	*/
/* seriously damage your health.				*/

struct NETDBINFO
{
	struct
	{
		struct hostent	h;
		struct netent	n;
		struct servent	s;
		struct protoent	p;
		int i[8];
	} u;
};

#ifndef TEST
extern struct NETDBINFO *dbinfo;
#endif

extern int opendb(char *name, int mode);
extern int closedb(int mode);
extern int scandb(char *format, ... );

extern int errno;

/* Host Database access macros */

#define HOSTDB	"/helios/etc/hosts"

#define sethostent(stayopen) (opendb("/etc/hosts",stayopen?1:0))
#define endhostent() (closedb(0))

#define gethostent() 							\
	((errno=opendb(HOSTDB,2))!=0?0:					\
	((errno=scandb("%a %s %ls",					\
	&dbinfo->u.i[2],&dbinfo->u.h.h_name,&dbinfo->u.h.h_aliases)),	\
	(dbinfo->u.h.h_addrtype=AF_INET),				\
	(dbinfo->u.h.h_length=sizeof(int)),				\
	(dbinfo->u.i[0]=(int)&(dbinfo->u.i[2])),			\
	(dbinfo->u.h.h_addr_list=(char **)&(dbinfo->u.i[0])),		\
	(dbinfo->u.i[1]=0),						\
	errno?0:&dbinfo->u.h))
			
#define gethostbyname(name)						\
	((errno=opendb(HOSTDB,0))!=0?0:					\
	(dbinfo->u.h.h_name=name,(errno=scandb("%a #s %ls",		\
	&dbinfo->u.i[2],&dbinfo->u.h.h_name,&dbinfo->u.h.h_aliases)),	\
	(dbinfo->u.h.h_addrtype=AF_INET),				\
	(dbinfo->u.h.h_length=sizeof(int)),				\
	(dbinfo->u.i[0]=(int)&(dbinfo->u.i[2])),			\
	(dbinfo->u.h.h_addr_list=(char **)&(dbinfo->u.i[0])),		\
	(dbinfo->u.i[1]=0),						\
	closedb(1),							\
	errno?0:&dbinfo->u.h))

#define gethostbyaddr(addr,len,type)					\
	(((errno=opendb(HOSTDB,0))!=0)?0:				\
	((void)(errno=scandb("!a %s %ls",				\
	*(int *)addr,&dbinfo->u.h.h_name,&dbinfo->u.h.h_aliases)),	\
	(dbinfo->u.h.h_addrtype=AF_INET),				\
	(dbinfo->u.h.h_length=sizeof(int)),				\
	(dbinfo->u.i[2]=*(int *)addr),					\
	(dbinfo->u.i[1]=0),						\
	(dbinfo->u.i[0]=(int)&(dbinfo->u.i[2])),			\
	(dbinfo->u.h.h_addr_list=(char **)&(dbinfo->u.i[0])),		\
	closedb(1),							\
	errno?0:&dbinfo->u.h))

#define herror(str) perror(str)

#define h_errno errno

/* Networks database access macros */

#define NETSDB	"/helios/etc/networks"

#define setnetent(stayopen) (opendb(NETSDB,stayopen?1:0))
#define endnetent() (closedb(0))

#define getnetent() 							\
	((errno=opendb(NETSDB,2))!=0?0:					\
	((errno=scandb("%s %a %ls",					\
	&dbinfo->u.n.n_name,&dbinfo->u.n.n_net,&dbinfo->u.n.n_aliases)),\
	dbinfo->u.n.n_addrtype=AF_INET,					\
	errno?0:&dbinfo->u.n))
			
#define getnetbyname(name)						\
	((errno=opendb(NETSDB,0))!=0?0:					\
	(dbinfo->u.n.n_name=name,(errno=scandb("#s %a %ls",		\
	&dbinfo->u.n.n_name,&dbinfo->u.n.n_net,&dbinfo->u.n.n_aliases)),\
	dbinfo->u.n.n_addrtype=AF_INET,					\
	closedb(1),							\
	errno?0:&dbinfo->u.n))

#define getnetbyaddr(addr,type)						\
	((errno=opendb(NETSDB,0))!=0?0:					\
	((errno=scandb("%s !a %ls",					\
	&dbinfo->u.n.n_name,addr,&dbinfo->u.n.n_aliases)),		\
	dbinfo->u.n.n_net=addr,						\
	dbinfo->u.n.n_addrtype=AF_INET,					\
	closedb(1),							\
	errno?0:&dbinfo->u.n))

/* Protocol Database access macros */

#define PROTODB	"/helios/etc/protocols"

#define setprotoent(stayopen) (opendb(PROTODB,stayopen?1:0))
#define endprotoent() (closedb(0))

#define getprotoent() 							\
	((errno=opendb(PROTODB,2))!=0?0:				\
	((errno=scandb("%s %d %ls",					\
	&dbinfo->u.p.p_name,&dbinfo->u.p.p_proto,&dbinfo->u.p.p_aliases)),\
	errno?0:&dbinfo->u.p))
			
#define getprotobyname(name)						\
	((errno=opendb(PROTODB,0))!=0?0:				\
	(dbinfo->u.p.p_name=name,(errno=scandb("#s %d %ls",		\
	&dbinfo->u.p.p_name,&dbinfo->u.p.p_proto,&dbinfo->u.p.p_aliases)),\
	closedb(1),							\
	errno?0:&dbinfo->u.p))

#define getprotobynumber(proto)						\
	((errno=opendb(PROTODB,0))!=0?0:				\
	((errno=scandb("%s !d %ls",					\
	&dbinfo->u.p.p_name,proto,&dbinfo->u.p.p_aliases)),		\
	dbinfo->u.p.p_proto=proto,					\
	closedb(1),							\
	errno?0:&dbinfo->u.p))


/* Services Database access macros */

#define SERVDB	"/helios/etc/services"

#define setservent(stayopen) (opendb(SERVDB,stayopen?1:0))
#define endservent() (closedb(0))

#define getservent() 							\
	((errno=opendb(SERVDB,2))!=0?0:					\
	((errno=scandb("%s %d/%s %ls",					\
	&dbinfo->u.s.s_name,&dbinfo->u.s.s_port,			\
	&dbinfo->u.s.s_proto,&dbinfo->u.s.s_aliases)),			\
	(dbinfo->u.s.s_port=htons(dbinfo->u.s.s_port)),			\
	errno?0:&dbinfo->u.s))

#define getservbyname(name,proto)					\
	(((errno=opendb(SERVDB,0))!=0)?0:				\
	((dbinfo->u.s.s_name=name),					\
	(((proto!=0)?(dbinfo->u.s.s_proto=proto,			\
		(errno=scandb("#s %d/!s %ls",&dbinfo->u.s.s_name,	\
			&dbinfo->u.s.s_port,&dbinfo->u.s.s_proto,	\
			&dbinfo->u.s.s_aliases))):			\
		(errno=scandb("#s %d/%s %ls",&dbinfo->u.s.s_name,	\
			&dbinfo->u.s.s_port,				\
			&dbinfo->u.s.s_proto,&dbinfo->u.s.s_aliases))),	\
	(dbinfo->u.s.s_port=htons(dbinfo->u.s.s_port)),			\
	closedb(1),							\
	(errno?0:&dbinfo->u.s))))

#define getservbyport(port,proto)					\
	(((errno=opendb(SERVDB,0))!=0)?0:				\
	(((proto!=0)?(dbinfo->u.s.s_proto=proto,			\
		(errno=scandb("%s !d/!s %ls",&dbinfo->u.s.s_name,	\
		port,&dbinfo->u.s.s_proto,&dbinfo->u.s.s_aliases))):	\
	(errno=scandb("%s !d/%s %ls",&dbinfo->u.s.s_name,port,		\
	 &dbinfo->u.s.s_proto,&dbinfo->u.s.s_aliases))),		\
	(dbinfo->u.s.s_port=htons(port)),				\
	closedb(1),							\
	(errno?0:&dbinfo->u.s)))
	
#endif
