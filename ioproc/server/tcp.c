/*------------------------------------------------------------------------
--                                                                      --
--         H E L I O S   I N P U T / O U T P U T   S E R V E R          --
--         ---------------------------------------------------          --
--                                                                      --
--         Copyright (C) 1987-91, Perihelion Software Ltd.              --
--                       All Rights Reserved.                           --
--                                                                      --
--  tcp.c                                                               --
--                                                                      --
--          I/O Server Internet Service code                            --
--                                                                      --
--  Author:  MJT July 1991                                              --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: tcp.c,v 1.19 1994/06/29 13:42:25 tony Exp $ */
/* Copyright (C) 1987-91, Perihelion Software Ltd.       		*/

/************************************************************************/
/*									*/
/*  This module contains the code for the Internet server for UNIX	*/
/*  systems which do not have access to transputer ethernet hardware	*/
/*									*/
/*  The code here is a derivation of the Helios tcpip server code and	*/
/*  performs the same functions. All Helios ethernet code will run as	*/
/*  if the Helios internet server is operational. Instead, however, of	*/
/*  using an ethernet device driver under Helios, this code uses the	*/
/*  socket support of the underlying UNIX operating system. All Helios	*/
/*  internet messages are translated into the equivalent UNIX system	*/
/*  calls and the results returned to the client in the normal fashion.	*/
/*  									*/
/*  This server currently supports the following internet GSP requests:	*/
/*									*/
/*  Bind, Listen, Accept, Connect, SendMessage, RecvMessage, GetInfo,	*/
/*  SetInfo, Select, Read, Write, Open, Close, GetSize			*/
/*									*/
/************************************************************************/

#include "helios.h"
#include <stdlib.h>

#if internet_supported

#if (!SM90 && !RS6000 && !HP9000 && !SCOUNIX)
#include <sys/filio.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#endif
#if (SM90)
#include <sys/ptio.h>
#endif

#ifdef SCOUNIX
#include <sys/socket.h>
#endif

#include <sys/uio.h>

#if !SOLARIS
typedef struct sockaddr		sockaddr;
#endif

#define	TCPDEBUG	0

typedef struct SockEntry
{
	ObjNode		ObjNode;	/* Object Node info */
	Port		Server;		/* Stream for socket */
	int		Protocol;	/* Socket protocol and type */
	
	/* Select stuff */
	int		selfn;		/* what to select */
	int		selval;		/* result of select */

	bool		Oob;		/* out-of-band data flag */

	/* time stuff */
	time_t		creation;	/* creation date */
	time_t		access;		/* access date */
		
	/* UNIX stuff */
	int 		fd;		/* socket file descriptor */
	int		connwait;	/* TRUE if waiting for connect */
	int		nonblocking;	/* TRUE if nonblocking socket  */
} SockEntry;

typedef struct locstr {
	int len;			/* length of addr struct */
	struct sockaddr_in dat;		/* struct itself */
	} locstr;

PRIVATE DirHeader Internet_extra;	/* List of sockets currently open */
PRIVATE int MyAddr;			/* My (host's) internet address */
PRIVATE char MyName[NameMax];		/* My (host's) name */

static int SockId = 1;			/* Unique socket number */

/* #if ANSI_prototypes */
SockEntry * fn (Dir_find_socket,   (Conode *));
SockEntry * fn (Dir_find_sockname, (Conode *, char *));

int fn (clearselect, (Conode *, SockEntry *));

void fn (DoShutdown,  (int, int));

short fn (swap_short, (short));

PRIVATE word fn (addint, (char *, word));

SockEntry * fn (NewSocket, (Conode *, char *, int));

void fn (dgtomsg, (MCB *, struct msghdr *));

/* #else */

#if 0
SockEntry *Dir_find_socket();
SockEntry *Dir_find_sockname();

SockEntry *NewSocket();
short swap_short();
void dgtomsg();
PRIVATE word addint();
#endif /* 0 */
/* #endif */	/* ANSI_prototypes */

#if SM90
#define uid_t int
#endif
uid_t realuid;
uid_t effuid;


#define DefSockMatrix	DefFileMatrix
#define TCPDataMax 10000

WORD fn( InternetDoConnect, (Conode *));
WORD fn( InternetDoBind, (Conode *));
int fn (InternetDoListen, (Conode *));
int fn (InternetDoAccept, (Conode *));
int fn (InternetDoSendMessage, (Conode *));
int fn (InternetDoRecvMessage, (Conode *));

/************************************************************************/
/*                                                                      */
/*  Init Server - called at boot time. Set up directory list and my     */
/*                internet address and name.                            */
/*                                                                      */
/************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif
int fn (gethostname, (char *, int));
#ifdef __cplusplus
}
#endif

void Internet_InitServer(myco)
Conode *myco;
{ 
  struct hostent *h;
  myco->extra      = (ptr) &Internet_extra;
  InitList(&Internet_extra.list);
  Internet_extra.entries = 0L;
  gethostname(MyName, NameMax);		/* get unix host name */
  h = gethostbyname(MyName);		/* and local name */
  memcpy (&MyAddr, h->h_addr_list[0], h->h_length);
}

/************************************************************************/
/*                                                                      */
/*  Tidy Server - just free up any server specific things. Leave the    */
/*                streams to sort themselves out.                       */
/*                                                                      */
/************************************************************************/

void Internet_TidyServer(myco)
Conode *myco;
{ FreeList(&Internet_extra.list);
  use(myco)
}

/************************************************************************/
/*                                                                      */
/*  Open - if this request is for the server itself, start a directory  */
/*         stream, else start a socket stream.                          */
/*                                                                      */
/************************************************************************/

void Internet_Open(myco)
Conode *myco;
{ 
  ObjNode *node;

  /* ServerDebug ("Internet_Open (%s)", myco->name); */
  
  if (!strcmp(IOname, myco->name))    /* is it for the server ? */
   NewStream(Type_Directory, Flags_Closeable, (WORD) myco->extra,
             InternetDir_Handlers);

  elif ( (node = (ObjNode *) Dir_find_node(myco) ) eq (ObjNode *) NULL)
   Request_Return(EC_Error + SS_InterNet + EG_Unknown + EO_Socket, 0L, 0L);
  else
     NewStream(Type_Socket, Flags_Closeable|Flags_Selectable, (WORD) 0,
             Internet_Handlers );
}

/************************************************************************/
/*                                                                      */
/*  Initialise a stream. Called after a NewStream request.              */
/*                       Record the port id in the socket structure     */
/*                       associated with this stream. If the co-routine */
/*                       extra field is not NULL, then we must send the */
/*                       reply message by marshalling the extra         */
/*                       structure onto the message created by          */
/*                       NewStream. Otherwise, the message will be sent */
/*                       for us by General_Stream (server.c).           */
/*                                                                      */
/************************************************************************/

WORD Internet_InitStream(myco)
Conode *myco;
{
	SockEntry *d;

	/* ServerDebug( "Internet: InitStream\n" ); */
	
	if((d = Dir_find_sockname(myco, IOname)) != (SockEntry *) NULL)
	{
	    d -> Server = (Port) myco -> id;
	    mcb->MsgHdr.Dest = d->Server;
	    mcb->MsgHdr.DataSize = strlen(mcb->Data) + 1;
	    if(myco -> extra)
	      {
	          WORD hdr = *((WORD *)mcb);
	          WORD offset = hdr & 0xFFFF;
	          offset = (offset+3) & ~3;
	          mcb->Control[open_reply-1] = offset;
	          *((WORD *) mcb) = (hdr & ~0xFFFF) + offset;
    
#if SOLARIS
		  memcpy (&mcb->Data[mcb->MsgHdr.DataSize], myco -> extra, swap (*(myco->extra)));
#else
	          bcopy(myco -> extra, &mcb->Data[mcb->MsgHdr.DataSize],
			        swap(*(myco->extra)));
#endif
    
	          Request_Return(ReplyOK, open_reply,
	              (WORD) mcb->MsgHdr.DataSize + 4L + swap(*(myco->extra)));

			/* Now, don't let server send its message */

	          mcb->MsgHdr.Reply = NullPort;
		  iofree(myco->extra);
		  myco->extra = NULL;
	      }
	}
	return ReplyOK;
}

/************************************************************************/
/*                                                                      */
/*  Stream request - most of the hard work is done via this routine.    */
/*                   The main loop in server.c has been modified to     */
/*                   pass most internet messages to us here.            */
/*                                                                      */
/************************************************************************/

void Internet_PrivateStream(myco)
Conode *myco;
{
	WORD fncode = mcb -> MsgHdr.FnRc & FG_Mask;
	
	/* ServerDebug ("Internet_PrivateStream (%s) - fncode = 0x%lx", 
			myco->name, fncode); */

	switch( fncode )
	{
		case FG_GetAttr:      Internet_GetInfo(myco);		break; 
		case FG_SetAttr:      Internet_SetInfo(myco);		break; 
		case FG_Bind:	      InternetDoBind(myco);		break; 
		case FG_Connect:      InternetDoConnect(myco);		break; 
		case FG_Listen:	      InternetDoListen(myco);		break;
		case FG_Accept:	      InternetDoAccept(myco);		break;
		case FG_SendMessage:  InternetDoSendMessage(myco);	break;
		case FG_RecvMessage:  InternetDoRecvMessage(myco);	break;
		case FG_Read:         Internet_Read(myco);		break;
		case FG_Write:        Internet_Write(myco);		break;
		case FG_Close:        Internet_Close(myco);		break;
		case FG_Select:       Internet_Select(myco);		break;
		case FG_GetSize:      Internet_GetSize(myco);		break;
		case FG_SetSize:      Internet_SetSize(myco);		break;

		default:

		     /* ServerDebug ("Internet_PrivateStream () - AAARRRGGGHHH!!!"); */
		     Request_Return(SS_InterNet|EC_Error|EG_WrongFn|EO_Socket,
				0L, 0L);
	}

	return ;
}

/************************************************************************/
/*                                                                      */
/*  Close - close a socket and its associated stream. Not quite that    */
/*          simple, actually. Firstly, clear any outstanding select     */
/*          operation on this stream. Then if only part of the stream   */
/*          is to be closed, call the shutdown system call to do the    */
/*          work. Otherwise, if all users of theis stream have closed   */
/*          it, remove from directory, close socket and kill stream.    */
/*                                                                      */
/************************************************************************/

void Internet_Close(myco)
Conode *myco;
{
	SockEntry *d;

	/* ServerDebug ("Internet_Close (%s)", myco->name); */

	mcb->MsgHdr.Dest = NullPort;

	if((d = Dir_find_socket(myco)) == (SockEntry *) NULL)
	{
		Request_Return(SS_InterNet|EC_Error|EG_Unknown|EO_Socket,0L,0L);
		return ;
	}

	clearselect(myco, d);		/* clear up any outstanding selects */

	if(mcb->MsgHdr.FnRc & 0xf)		/* close one direction only */
		DoShutdown(d->fd, mcb->MsgHdr.FnRc);
	else
	{
		d->ObjNode.account--;		/* one less user */

		if(d->ObjNode.account == 0)		/* all done */
			{
			close(d->fd);		/* close socket */
			listRemove((Node *)d);		/* remove name */
			iofree(d);
			if(Internet_extra.entries)
				Internet_extra.entries--;
			}

		if(mcb->MsgHdr.Reply)		/* if client wants a reply */
			Request_Return(ReplyOK, 0L, 0L);

		if(d->ObjNode.account == 0)
			Seppuku();		/* die now */
	}

	return ;
}

/************************************************************************/
/*                                                                      */
/*  shutdown one or more parts of full-duplex socket                    */
/*                                                                      */
/************************************************************************/

void DoShutdown(sfd, fncode)
int sfd, fncode;
{
	word how = 0;
	
	if(fncode & O_WriteOnly)
		{
		if(fncode & O_ReadOnly)
			how = 2;
		else
			how = 1;
		}

	if(shutdown(sfd,how) == -1)
		{
		convert_errno();
		Request_Return(SS_InterNet|EC_Error|EG_Errno|errno, 0L, 0L);
		}
	else
		Request_Return(ReplyOK, 0L, 0L);
}

/************************************************************************/
/*                                                                      */
/*   Connect - initiate a connection on a socket. The initial connect   */
/*             may well error until the connection is actually made.    */
/*             In this case, return a recoverable error to the client   */
/*             and let it retry. Mark the socket as awaiting connection */
/*             Once connected return appropriate message to client.     */
/*                                                                      */
/************************************************************************/

WORD InternetDoConnect(myco)
Conode *myco;
{
	ConnectRequest *cr = (ConnectRequest *)mcb->Control;
	struct sockaddr_in *addr = NULL;
	Port reply = mcb->MsgHdr.Reply;
	Port dest = mcb->MsgHdr.Dest;
	word e;
	SockEntry *d;
	int csize = mcb->MsgHdr.ContSize;
	int dsize = mcb->MsgHdr.DataSize;
	
	/* ServerDebug ("InternetDoConnect (%s)", myco->name); */

	
	if( cr->DestAddr >= 0 )
		addr = (struct sockaddr_in *)(mcb->Data+cr->DestAddr+4);

	if(addr)
	{
		addr->sin_family = swap_short(addr->sin_family);
#if TCPDEBUG
		ServerDebug("Connect %d %x %x %x", sizeof(struct sockaddr_in),
		(short)(addr->sin_family), (ushort)addr->sin_port,
		addr->sin_addr.s_addr);
#endif
	}
	
	if((d = Dir_find_socket(myco)) == (SockEntry *) NULL)
		e = SS_InterNet|EC_Error|EG_Unknown|EO_Socket;
	else
	{
		d->connwait = TRUE;		/* waiting for connect */
		e = connect(d->fd, (sockaddr *)addr,sizeof(struct sockaddr_in));
	}
#if (TR5 || i486V4)
	if ( (e == -1) && (errno == EAGAIN) ) errno = EISCONN;
#endif
                                 /* I might have to poll later */
        AddTail(Remove(&(myco->node)), PollingCo);

	while( e != 0 && d->connwait && 
		(errno == EINPROGRESS || errno == EALREADY)) /* wait for it */
	{
		/* going to wait for connect - save mcb data */

		char *buf;

		if((buf = (char *) malloc(csize+dsize)) == NULL)
		   {
		   Request_Return(SS_InterNet|EC_Error|EG_NoMemory|EO_Socket,
				0L, 0L);
		   return TRUE;
		   }

#if SOLARIS
		memcpy (buf, mcb->Control, csize * 4);
		memcpy (&buf[csize*4], mcb->Data, dsize);
#else
		bcopy(mcb->Control, buf, csize * 4);
		bcopy(mcb->Data, &buf[csize*4], dsize);
#endif
        	myco->type = CoReady;

		Suspend();

		mcb->MsgHdr.Reply = reply;
		mcb->MsgHdr.Dest = dest;

#if SOLARIS
		memcpy (mcb->Control, buf, csize * 4);
		memcpy (mcb->Data, &buf[csize*4], dsize);
#else
		bcopy(buf, mcb->Control, csize * 4);
		bcopy(&buf[csize*4], mcb->Data, dsize);
#endif
		iofree(buf);

		if(myco->type == CoSuicide)
		  {
		  mcb->MsgHdr.FnRc = FG_Close;
		  Internet_Close(myco);
		  return TRUE;
		  }
		elif (myco->type == CoTimeout)
		   {
		   Request_Return(SS_InterNet|EC_Recover|EG_Timeout|EO_Socket,
				0L, 0L);
		   return TRUE;
		   }

		e = connect(d->fd, (sockaddr *)addr,sizeof(struct sockaddr_in));
	}
#if (TR5 || i486V4)
	if ( (e == -1) && (errno == EAGAIN) ) errno = EISCONN;
#endif
                                 /* make sure I get back into waiting list */
        PostInsert(Remove(&(myco->node)), Heliosnode);
	
	if( e != 0 && errno == EISCONN && d->connwait)	/* finally OK */
		e = 0;					/* clear error */

#if (TR5 || i486V4)
	if( e == 0 && ((d->Protocol & 0xf) == SOCK_DGRAM ))
	    /* Unix 5 4.0 : DGRAM  = 0 STREAM = 1 */
	    /* Unix BSD   : DGRAM  = 1 STREAM = 0 */
#else
	if( e == 0 && ((d->Protocol & 0xf) == SOCK_STREAM ))
#endif
	{
		WORD temp;
		
		d->connwait = FALSE;

		temp = FormOpenReply(Type_Socket,
		     Flags_Server|Flags_Closeable|Flags_Selectable, -1L, -1L);

#if 0 /* @@@ sort out later */
		Capability cap;
		NewCap(&cap,&d->ObjNode,AccMask_R|AccMask_W);
		MarshalCap(mcb,&cap);
		MarshalOffset(mcb);
#endif
		mcb->MsgHdr.Dest = d->Server;
		Request_Return(ReplyOK, open_reply, temp);
	}
	else
	{
		if(e)
		   {
		   convert_errno();
		   Request_Return(SS_InterNet|EC_Error|EG_Errno|errno, 0L, 0L);
		   }
		else
		   Request_Return(ReplyOK, 0L, 0L);
	}
	return TRUE;
}

/************************************************************************/
/*                                                                      */
/*  Listen for connections on a socket. The parameter supplied in the   */
/*  control vector defines the maximum length to which the pending      */
/*  connections queue may grow.                                         */
/*                                                                      */
/************************************************************************/

InternetDoListen(myco)
Conode *myco;
{
	SockEntry *d;

	/* ServerDebug ("InternetDoListen (%s)", myco->name); */

	mcb->MsgHdr.Dest = NullPort;

	if((d = Dir_find_socket(myco)) == (SockEntry *) NULL)
		Request_Return(SS_InterNet|EC_Error|EG_Unknown|EO_Socket,0L,0L);
	else
		{
		if(listen(d->fd, mcb->Control[0]) == -1)
			{
			convert_errno();
			Request_Return(SS_InterNet|EC_Error|EG_Errno|errno,
				0L, 0L);
			}
		else
			Request_Return(ReplyOK, 0L, 0L);
		}

	return FALSE;
}

/************************************************************************/
/*                                                                      */
/*  Accept a connection on a socket. If operation would block, then     */
/*  return recoverable error. Otherwise, create a new socket structure, */
/*  initialise it (remembering to set the new socket to non-blocking -  */
/*  we can't block in the I/O server), reply to client and then create  */
/*  a new stream to handle this socket.                                 */
/*                                                                      */
/************************************************************************/

InternetDoAccept(myco)
Conode *myco;
{
	word e;
	locstr *l;
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	SockEntry *d;
	char newname[NameMax];
        char pathname[NameMax];
	int setval = 1;
	
	/* ServerDebug ("InternetDoAccept (%s)", myco->name); */

	strcpy(pathname, "internet");

	if((d = Dir_find_socket(myco)) == (SockEntry *) NULL)
	{
		Request_Return(SS_InterNet|EC_Error|EG_Unknown|EO_Socket,0L,0L);
		return FALSE;
	}

	if(( l = (locstr *) malloc(sizeof(locstr))) == NULL)
	{
		Request_Return(SS_InterNet|EC_Error|EG_NoMemory|EO_Server,
				0L,0L);
		return FALSE;
	}

	e = accept(d->fd,(sockaddr *)&addr,&addrlen);
#if (TR5 || i486V4)
	if ( (e == -1) && (errno == EAGAIN) ) errno = EWOULDBLOCK;
#endif
	
	if( e == -1 )
	   {
	   if(errno == EWOULDBLOCK)
		{
		if (d->nonblocking)
			/* 37 == EWOULDBLOCK under Helios	*/
			Request_Return(SS_InterNet|EC_Error|EG_Errno|37,
				0L, 0L);
		else
		       Request_Return(SS_InterNet|EC_Recover|EG_Timeout|EO_Socket,
				0L, 0L);
		}
	   else
	       {
	       convert_errno();
	       Request_Return(EC_Error|EG_Errno|SS_InterNet|errno, 0L, 0L);
	       }

	   return FALSE;
	   }

	/* accept OK - make new socket structure */
	
	strcpy(newname, d->ObjNode.direntry.Name);	/* form name */
	strcat(newname, ".");
	addint(newname, SockId++);

	d = NewSocket(myco,newname,d->Protocol);	/* get structure */

	if(d == NULL)
		goto killsocket;

	d->fd = e;				/* record fd */

	ioctl(e, FIONBIO, &setval);		/* set non-blocking */
						/* now reply to the client */
	l->len = swap(addrlen);

	addr.sin_family = swap_short(addr.sin_family);

#if SOLARIS
	memcpy (&l->dat, &addr, addrlen);
#else
	bcopy(&addr, &l->dat, addrlen);
#endif

	strcpy(IOname, pathname);	/* saved name */
	pathcat(IOname, newname);

     	NewStream(Type_Socket, Flags_Closeable|Flags_Selectable, (WORD) l,
             Internet_Handlers );
		
	return FALSE;
	
killsocket:
	close(d->fd);
	listRemove((Node *)d);
	iofree(d);
	if(Internet_extra.entries)
		Internet_extra.entries--;
	Request_Return(SS_InterNet|EC_Error|EG_NoMemory|EO_Socket, 0L, 0L);
	return FALSE;
}

/************************************************************************/
/*                                                                      */
/*  Bind - bind a name to a socket. Tricky things to take care of here. */
/*         Firstly, see if we have already bound to this socket. If so, */
/*         see if we can bind again. Otherwise, we are binding to a new */
/*         socket. Create a new (unique) name, a new socket structure   */
/*         and create a new socket in the given domain. If we have an   */
/*         address, bind this to socket.                                */
/*                                                                      */
/************************************************************************/

WORD InternetDoBind(myco)
Conode *myco;
{
	SockEntry *d;
	IOCBind *bindreq = (IOCBind *)mcb->Control;
	char *dirname;
	struct sockaddr_in *addr;
	char sockname[NameMax];
	int setval = 1;
	int errsent = 0;

	word e;

	unless(convert_name())
	{
	  ServerDebug ("InternetDoBind () - convert_name () failed"); 

		Request_Return(EC_Error|SS_InterNet|EG_Name|EO_Message,0L,0L);
		return -1;
	}

	/* This is possibly a bind for a socket which we have only */
	/* created previously. Let TCPIP sort out whether we can   */
	/* do this.						   */

	if((d = Dir_find_sockname(myco, IOname)) != (SockEntry *) NULL)
	{
	    if(!strcmp(d->ObjNode.direntry.Name, "internet"))
		goto isroot;			/* OK carry on */

		if( bindreq->Addr == -1 )
		{
		    Request_Return(SS_InterNet|EC_Error|EG_WrongFn|EO_Socket,
					0L, 0L);
		    goto bad;
		}

		addr = (struct sockaddr_in *)&mcb->Data[bindreq->Addr+4];
#if TCPDEBUG
		ServerDebug("InternetDoBind () - Bind %x %x %x %x", addr->sin_family,addr->sin_port,
			addr->sin_addr.s_addr, addr->sin_zero);
#endif

		setsu(TRUE);
	        e = bind(d->fd, (sockaddr *)addr, sizeof(struct sockaddr_in));
		setsu(FALSE);

	        if( e )
	             {
			convert_errno();
		        Request_Return(EC_Error|EG_Errno|SS_InterNet|errno, 0L, 0L);
		        goto bad;
	             }
	        goto sendreply;
	}

isroot:
	switch( bindreq->Protocol & 0xf )
	{
#if (TR5 || i486V4)		/* No compability between BSD and 4.0 */
		case 3: 		dirname = "raw"; break;
		case 1:		 	dirname = "tcp"; break;
		case 2: 
#else
		case SOCK_RAW: 		dirname = "raw"; break;
		case SOCK_STREAM: 	dirname = "tcp"; break;
		case SOCK_DGRAM: 
#endif
		default:	 	dirname = "udp"; break;
	}
	
	sockname[0] = 0;		/* form new name */
	strcpy(sockname,dirname);
	strcat(sockname, ".");

	if( bindreq->Addr == -1 )
		addr = NULL;
	else
		addr = (struct sockaddr_in *)&mcb->Data[bindreq->Addr+4];
	
	addint(sockname,SockId++);

	if( addr && addr->sin_port != 0 ) 
	{
		strcat(sockname,".");
		addint(sockname,ntohs(addr->sin_port));
	}

	/* ServerDebug ("InternetDoBind () - sockname = %s", sockname); */

	if((d = Dir_find_sockname(myco, sockname)) == (SockEntry *) NULL)
	{
		word e = 0;

		/* ServerDebug ("InternetDoBind () - checking access rights..."); */

		/* @@@@ check access rights */
		
		d = NewSocket(myco,sockname,bindreq->Protocol);

		if( d == NULL ) 
		{
			goto killsocket;
		}

		setsu(TRUE);
#if (TR5 || i486V4)
		{ int prot = 0;
		  if ( (d->Protocol&0x0f) == 1) prot = SOCK_STREAM;
		  if ( (d->Protocol&0x0f) == 2) prot = SOCK_DGRAM;
		  if ( (d->Protocol&0x0f) == 3) prot = SOCK_RAW;
		  if ( (d->Protocol&0x0f) == 4) prot = SOCK_RDM;
		  if ( (d->Protocol&0x0f) == 5) prot = SOCK_SEQPACKET;

		  e = socket(AF_INET, prot,(d->Protocol>>8));
                }
#else
		e = socket(AF_INET, d->Protocol&0xf,(d->Protocol>>8));
#endif
		setsu(FALSE);

		if( e == -1 )
		  {
		   convert_errno();
		   Request_Return(SS_InterNet|EC_Error|EG_Errno|errno, 0L, 0L);
		   errsent = TRUE;

		   goto killsocket;
		  }

		d->fd = e;	/* save socket fd for later */
		ioctl(d->fd, FIONBIO, &setval);		/* set non-blocking */
		if( addr ) 
		{
			setsu(TRUE);
#if (TR5 || i486V4)
			addr->sin_family = swap_short(addr->sin_family);
#endif

			/* ServerDebug ("InternetDoBind () - attempting bind () on <%d, %d, <%d>, %s>", 
				addr->sin_family, addr->sin_port, 
				(addr->sin_addr).s_addr, addr->sin_zero); */

			e = bind(d->fd, (sockaddr *)addr,sizeof(struct sockaddr_in));
			setsu(FALSE);

			if( e ) 
			{
			    /* ServerDebug ("InternetDoBind () - errno = %d (0x%lx)", errno, errno); */

			    convert_errno();
			    Request_Return(SS_InterNet|EC_Error|EG_Errno|errno,
					   0L, 0L);
			    errsent = TRUE;

			    goto killsocket;
			}
		}

		/* If no name was supplied, change name now to port	*/
		/* number chosen by protocol.				*/

		if( !addr || addr->sin_port==0  )
		{
			struct sockaddr_in a;
			int alen = sizeof(a);

			e = getsockname(d->fd,(sockaddr *)(&a),&alen);

			if( e == 0 && a.sin_port != 0 )
			{
				strcat(d->ObjNode.direntry.Name,".");
				addint(d->ObjNode.direntry.Name,
						ntohs(a.sin_port));
			}
		}
		
	}
	else
				/* connect user to existing socket */
		d->ObjNode.account++;

	pathcat(IOname, d->ObjNode.direntry.Name);
sendreply:

     	NewStream(Type_Socket, Flags_Closeable|Flags_Selectable, (WORD) 0,
             Internet_Handlers );

	return TRUE;
	
killsocket:
	/* ServerDebug ("InternetDoBind () - killing socket"); */

	close(d->fd);
	listRemove((Node *)d);
	if(Internet_extra.entries)
		Internet_extra.entries--;
	if( ! errsent)
	Request_Return(SS_InterNet|EC_Error|EG_NoMemory|EO_Socket, 0L, 0L);
bad:

	return FALSE;
}

/************************************************************************/
/*                                                                      */
/*  NewSocket - create and initialise a new socket structure.           */
/*                                                                      */
/************************************************************************/

SockEntry *NewSocket(myco, name, proto)
Conode *myco;
char *name;
int proto;
{
	SockEntry *s = (SockEntry *) malloc(sizeof(SockEntry));
	ObjNode *o = (ObjNode *) &s->ObjNode;
	
	if( s == NULL ) return NULL;
	
	memset(s,0,sizeof(SockEntry));
	
	strncpy(o->direntry.Name, name, NameMax-1);	/* new name */
	o->direntry.Name[31] = 0;
	o->direntry.Type = Type_Socket;
	o->direntry.Flags = 0;
	o->direntry.Matrix = DefSockMatrix;
	o->account = 1;
	o->size = 0;

	s->Protocol = proto;			/* protocol/type */
	s->Server = (Port) myco->id;		/* stream id */
	s->Oob = FALSE;

	s->creation = get_unix_time();			/* creation time */
	s->access = s->creation;			/* last access time */

	s->fd = -1;				/* no socket yet */
	s->connwait = FALSE;			/* not connect waiting */
	s->nonblocking = FALSE;			/* blocking */
		
	AddTail((Node *)(&(s->ObjNode)), (List *)(&(Internet_extra)));
	Internet_extra.entries++;
	
	return s;
}

/************************************************************************/
/*                                                                      */
/*  Find a socket in the directory list using the stream id field       */
/*                                                                      */
/************************************************************************/

SockEntry *Dir_find_socket(myco)
Conode *myco;
{ List                  *list = (List *) &(Internet_extra);
  register SockEntry *node;

  for (node = (SockEntry *) list->head;
       node->ObjNode.node.next ne (Node *) NULL;
       node = (SockEntry *) node->ObjNode.node.next)
   if (node->Server == myco->id)
	{
	node->access = get_unix_time();
	return(node);
	}

   return((SockEntry *) NULL);
}

/************************************************************************/
/*                                                                      */
/*  Find a socket in the direcotry list using the socket name           */
/*                                                                      */
/************************************************************************/

SockEntry *Dir_find_sockname(myco, name)
Conode *myco;
char *name;
{ List                  *list = (List *) &(Internet_extra);
  register SockEntry *node;

  for ( ; (*name ne '/') && (*name ne '\0'); name++);
  if (*name eq '\0')
    return((SockEntry *) NULL);

  name++;
  for (node = (SockEntry *) list->head;
       node->ObjNode.node.next ne (Node *) NULL;
       node = (SockEntry *) node->ObjNode.node.next)
   if (!strcmp(name, &(node->ObjNode.direntry.Name[0])))
	{
	node->access = get_unix_time();
	return(node);
	}

   return((SockEntry *) NULL);
}

/************************************************************************/
/*                                                                      */
/*  swap_short - byte swap a short int.                                 */
/*                                                                      */
/************************************************************************/

#if swapping_needed

short swap_short(a)
short a;
{ register word b = 0;
  int i;

  if (a eq 0) return(0);

  for (i=0; i<2; i++)
    { b <<= 8; b |= (a & 0xFF); a >>= 8; }

  return(b);
}

#else

short swap_short(a)
short a;
{
	return(a);
}

#endif

/************************************************************************/
/*                                                                      */
/*  SetInfo - perform ioctl on socket or setsockopt. If ioctl is to set */
/*            or reset Non Blocking mode, record details in socket      */
/*            structure.                                                */
/*                                                                      */
/************************************************************************/

void Internet_SetInfo(myco)
Conode *myco;
{
	word e;
	SocketInfoReq *si = (SocketInfoReq *)mcb->Control;
	caddr_t val = NULL;
	word valsize = 0;
	SockEntry *d;

	/* ServerDebug ("Internet_SetInfo (%s)", myco->name); */
	
	if((d = Dir_find_socket(myco)) == (SockEntry *) NULL)
	{
		e = SS_InterNet|EC_Error|EG_Unknown|EO_Socket;
		goto done;
	}

	if( si->Optval != -1 )		/* data in data vector */
	{
		val = mcb->Data+si->Optval+4;
		valsize = *(WORD *)(mcb->Data+si->Optval);
	}

	if( si->Level == SOL_IOCTL ) 
	{
		/* if IOCTL is set owner, use my local pid in place of arg */
		if( si->Option == SIOCSPGRP ) *(int *)val = getpid();

		e = ioctl(d->fd,si->Option,val);

		if(e != -1 && si->Option == FIONBIO) /* no error - set block */
			{
			if(*val != 0)
				d->nonblocking = TRUE;
			else
				d->nonblocking = FALSE;
			}
	}
	else
	   e = setsockopt(d->fd,si->Level,si->Option,val,swap(valsize));
	
done:
	if( e )
	   {
	   convert_errno();
	   Request_Return(SS_InterNet|EC_Error|EG_Errno|errno, 0L, 0L);
	   }
	else
	{
		mcb->MsgHdr.Dest = NullPort;
		mcb->MsgHdr.ContSize += 2;
		Request_Return(ReplyOK, mcb->MsgHdr.ContSize, 0);
	}
	
	return ;
}

/************************************************************************/
/*                                                                      */
/*  GetInfo - perform info operations gethostid, gethostname,           */
/*            getpeername, getsockname, getsockopt and ioctls           */
/*                                                                      */
/************************************************************************/

void Internet_GetInfo(myco)
Conode *myco;
{
	word e = 0;
	SocketInfoReq *si = (SocketInfoReq *)mcb->Control;
	int level = si->Level;
	int name = si->Option;
	byte *val = mcb->Data + sizeof(word);
	int valsize = IOCDataMax - sizeof(word);
	SockEntry *d;

	/* ServerDebug ("Internet_GetInfo (%s), level= %x", myco->name, level); */

	if((d = Dir_find_socket(myco)) == (SockEntry *) NULL)
	{
		e = SS_InterNet|EC_Error|EG_Unknown|EO_Socket;
		goto done;
	}

	if( level == SOL_SYSTEM )
	{
		switch( name )
		{
		case SO_HOSTID:
			*(word *)val = swap(MyAddr);
			valsize = sizeof(MyAddr);
			goto done;
			
		case SO_HOSTNAME:
			strcpy(val,MyName);
			valsize = strlen(MyName)+1;
			goto done;
		}
	}
	elif( level == SOL_SOCKET )
	{
		switch( name )
		{
		case SO_PEERNAME:
			e = getpeername(d->fd,(sockaddr *)val,&valsize);
			goto done;
			
		case SO_SOCKNAME:
			e = getsockname(d->fd,(sockaddr *)val,&valsize);
			goto done;
		}
	}
	
	/* To allow us to support ioctls which transfer data in both	*/
	/* directions, we allow GetInfo to supply some data in the	*/
	/* buffer.							*/

	if( si->Level == SOL_IOCTL ) 
	{
		valsize = (si->Option>>16) & 0xff;
#if i486V4
		if(si->Option == SIOCATMARK)	/* strangeness of Sys V.4 */
			*val = 1;
		else
#endif
		  {
		    e = ioctl(d->fd,si->Option,val);

		    /*
		     * XXX - NC - 5/10/93
		     *
		     * The result of some ioctl()s must be swapped.
		     * Unfortunately since there is no central
		     * documentation on ioctls(), we must guess at which
		     * ones will be needed....
		     */
		    
		    if (valsize)
		      switch (si->Option)
			{
#ifdef FIOGETOWN
			case FIOGETOWN:
#endif
			case FIONREAD:
			  *(int *)val = swap(*(int *)val);
			  break;
			  
			default:
			  ServerDebug( "ioctl %x returns an (unprocessed) value",
				      si->Option );
			  break;
			}
		  }
	}
	else
	 	e = getsockopt(d->fd,si->Level,si->Option,val,&valsize);

done:
	if(!e)
	{
	    mcb->Control[0] = level;
	    mcb->Control[1] = name;
	    mcb->MsgHdr.DataSize = 0;
	    {
		WORD hdr = *((WORD *)mcb);
		WORD offset = hdr & 0xFFFF;
		offset = (offset+3) & ~3;
		mcb->Control[2] = offset;
		*((WORD *) mcb) = (hdr & ~0xFFFF) + offset;
	    }
	    *(WORD *)mcb->Data = swap(valsize);
	    mcb->MsgHdr.Flags = 0;
	    mcb->MsgHdr.Dest = NullPort;
	    mcb->MsgHdr.DataSize = valsize+4;
	    mcb->MsgHdr.ContSize = 3;
	    Request_Return(ReplyOK, 3L, (long) (valsize+4));
	}
	else
	    Request_Return(SS_InterNet|EC_Error|EO_Socket|e, 0L, 0L);
	
	return ;
}

/************************************************************************/
/*                                                                      */
/*  Write - firstly clear outstanding write select on this stream. Then */
/*          usual Helios write protocol. Suspend coroutine between msgs */
/*                                                                      */
/************************************************************************/

void Internet_Write(myco)
Conode *myco;
{
	ReadWrite *rw = (ReadWrite *)mcb->Control;
	word size;
	word savesize;
	word saveact;
	word e;
	word idata;
	Port replyport;
	Port destport;
	word timeout = ((rw->Timeout == -1) ? 0 : rw->Timeout / time_unit);
	SockEntry *d;
	char *buf, *buf1, *savebuf1;

	/* ServerDebug ("Internet_Write (%s)", myco->name); */

	if((d = Dir_find_socket(myco)) == (SockEntry *) NULL)
	{
		Request_Return(SS_InterNet|EC_Error|EG_Unknown|EO_Socket,0L,0L);
		return ;
	}

	e = d->selfn;
	d->selfn = O_WriteOnly;		/* only interested in write */
	clearselect(myco, d);		/* abort outstanding select */
	d->selfn = e & ~O_WriteOnly;

try_again:
	rw = (ReadWrite *)mcb->Control;
	size = rw->Size;
	idata = mcb->MsgHdr.DataSize;
	replyport = mcb->MsgHdr.Reply;
	destport = mcb->MsgHdr.Dest;
	buf = NULL;
	buf1 = NULL;

	if(size == 0)
	{
		mcb->Control[0] = 0;
		Request_Return(ReplyOK, 1, 0);
		return ;
	}

	if(size > maxdata)
		 size = maxdata;

	if(idata)	/* handle immediate data */
	{
	   if(idata == size)
		 goto dowrite;		/* got it all */

	   if((buf = (char *)malloc(IOCDataMax)) == NULL)	/* holding buffer */
	   {
	      Request_Return(SS_InterNet|EC_Error|EG_NoMemory|EO_Socket,0L,0L);
	      return ;
	   }

	   memcpy(buf,mcb->Data,idata);	/* save immediate for later */
	}
	
	mcb->Control[0] = size;
	mcb->Control[1] = size;
	mcb->Control[2] = size;

	mcb->MsgHdr.Dest = NullPort;

	mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;

	Request_Return(WriteRc_Sizes, 3L, 0L);

	myco->timelimit = Now+20;	/* next message must arrive soon */

	Suspend();			/* wait for next message */

	mcb->MsgHdr.Reply = replyport;
	mcb->MsgHdr.Dest = NullPort;
	if(myco->type == CoSuicide)
		{
		Request_Return(SS_InterNet|EC_Error|EG_Broken|EO_Stream,0L,0L);
		if(buf)
		  iofree(buf);
		Seppuku();
		return;
		}
	elif (myco->type == CoTimeout)
		{
		saveact = savesize = 0;
		goto done;
		}
	elif (getfnrc(mcb) == FG_Write)		/* new write message, restart */
		{
		if(buf)
		  iofree(buf);
		goto try_again;
		}

	if(mcb->MsgHdr.DataSize != size-idata)
		{
		Request_Return(SS_InterNet|EC_Error|EG_WrongSize|EO_Stream,
					0L,0L);
		goto finish;
		}

	if(idata && idata != size)		/* regroup data */
	   {
	   int errsave;

	   buf1 = (char *) malloc(size);
	   if(buf1 == NULL)
	      {
	      Request_Return(SS_InterNet|EC_Error|EG_NoMemory|EO_Socket,0L,0L);
	      iofree(buf);
	      return ;
	      }
	  
#if SOLARIS
	   memcpy (buf1, buf, idata);
	   memcpy (&buf1[idata], mcb->Data, size - idata);
#else
	   bcopy(buf, buf1, idata);
	   bcopy(mcb->Data, &buf1[idata], size-idata);
#endif
	   iofree(buf);
	   buf = NULL;
	   }

dowrite:
	if(buf1 == NULL)
		buf1 = mcb->Data;

	savebuf1 = buf1;	/* save changing variables for later */
	savesize = size;
	saveact = size;

	destport = mcb->MsgHdr.Dest;
	replyport = mcb->MsgHdr.Reply;

	AddTail(Remove(&(myco->node)), PollingCo);	/* may have to retry */

	myco->timelimit = Now + timeout;

	while(size > 0)			/* if blocking, retry until write 
						succeeds or times-out */
		{
		e = write(d->fd, buf1, size);
#if (TR5 || i486V4)
	if ( (e == -1) && (errno == EAGAIN) ) errno = EWOULDBLOCK;
#endif

		if(e == size)		/* write OK - continue */
			break;

		if( e == -1 && errno != EWOULDBLOCK ) /* some error - report */
			{
			convert_errno();
			Request_Return(SS_InterNet|EC_Error|EG_Errno|errno,
								0L,0L);
			goto finish;
			}
		if(e == -1 || e != size)	/* blocking error */
						/* or incomplete write */
			{
			if(d->nonblocking)	/* don't block */
			  {
			  if(e == -1)		/* return block error */
		   	    {
		   	    convert_errno();
		   	    Request_Return(SS_InterNet|EC_Error|EG_Errno|errno,
								0L,0L);
		   	    goto finish;
		   	    }
			  saveact = e;		/* or partial write result */
			  goto done;
			  }

			else			/* block */
				{
				if(e != -1)	/* no bytes written */
					{
					size -= e;	/* partial write */
					buf1 += e;
					}
				}

			myco->type = CoReady;

			Suspend();			/* wait for a while */

			mcb->MsgHdr.Reply = replyport;
			mcb->MsgHdr.Dest = destport;

			if(myco->type == CoSuicide)
			  {
			  if(savebuf1 != mcb->Data)
				iofree(savebuf1);
			  mcb->MsgHdr.FnRc = FG_Close;
			  Internet_Close(myco);
			  return;
			  }

				/* if timer expired, error else retry write */

			if(myco->type == CoTimeout || myco->timelimit < Now)
				{
				Request_Return(SS_InterNet|EC_Recover|EG_Timeout
						|EO_Socket, 0L, 0L);
				goto finish;
				}
			}
		}

done:
	mcb->Control[0] = savesize;
	mcb->Control[1] = saveact;
	Request_Return(WriteRc_Done, 2L, 0L);

finish:
	PostInsert(Remove(&(myco->node)), Heliosnode);
	myco->type = CoReady;
	if(buf)
	  iofree(buf);
	if(savebuf1 != mcb->Data)
	  iofree(savebuf1);
	return ;
	
}

/************************************************************************/
/*                                                                      */
/*  Read - clear outstanding read select then read data from socket.    */
/*         Take care of out-of-band data and tiemouts.                  */
/*                                                                      */
/************************************************************************/

void Internet_Read(myco)
Conode *myco;
{
	SockEntry *d;
	ReadWrite *rw = (ReadWrite *)mcb->Control;
	word size = rw->Size;
	word timeout = ((rw->Timeout == -1) ? 0 : rw->Timeout / time_unit);
	byte *buf;
	byte *data = mcb->Data;
	word e;
	Port reply = mcb->MsgHdr.Reply;
	Port dest = mcb->MsgHdr.Dest;

	/* ServerDebug ("Internet_Read (%s)", myco->name); */

	if((d = Dir_find_socket(myco)) == (SockEntry *) NULL)
	{
		Request_Return(SS_InterNet|EC_Error|EG_Unknown|EO_Socket,0L,0L);
		goto done;
	}

	e = d->selfn;
	d->selfn = O_ReadOnly;		/* only interested in write */
	clearselect(myco, d);		/* abort outstanding select */
	d->selfn = e & ~O_ReadOnly;

	if( d->Oob )
	{
	     Request_Return(SS_InterNet|EC_Error|EG_Exception|EE_Signal|SIGURG,
				0L, 0L);
	     d->Oob = FALSE;
	     return ;
	}

	if( size > TCPDataMax ) size = TCPDataMax;
		
	buf = (char *)malloc(size);

	if( buf == NULL )
	{
	     Request_Return(SS_InterNet|EC_Error|EG_NoMemory|EO_Socket,0L,0L);
	     return ;
	}
                                 /* I might have to poll later */
        AddTail(Remove(&(myco->node)), PollingCo);

        myco->timelimit = Now + timeout;
retry:
	e = read(d->fd, buf, size);
#if (TR5 || i486V4)
	if ( (e == -1) && (errno == EAGAIN) ) errno = EWOULDBLOCK;
#endif

	if( e == -1 && (errno == EINTR))		/* exception */
	{
	  if( d->Oob )
	    Request_Return(SS_InterNet|EC_Error|EG_Exception|EE_Signal|SIGURG,
				0L, 0L);
	  else
	    Request_Return(SS_InterNet|EC_Recover|EG_Timeout|EO_Socket, 0L, 0L);

	  d->Oob = FALSE;

	  goto done;
	}
	
	mcb->MsgHdr.Flags = 0;
	mcb->MsgHdr.Reply = reply;
	mcb->MsgHdr.Dest = dest;

	if( size && e == 0)		/* end of file */
	{
		Request_Return(ReadRc_EOF, 0L, 0L);
		goto done;
	}
	
	if( e == -1 && errno != EWOULDBLOCK)
		 {
		 convert_errno();
		 Request_Return(SS_InterNet|EC_Error|EG_Errno|errno,0L,0L);
		 goto done; 
		 }

	if(e == -1)		/* zero bytes to return */
	     {
	     if(d->nonblocking)
		{
		convert_errno();
		Request_Return(SS_InterNet|EC_Error|EG_Errno|errno,0L,0L);
		}
	     else
		{
        	myco->type = CoReady;

		Suspend();

		mcb->MsgHdr.Reply = reply;
		mcb->MsgHdr.Dest = dest;

		if(myco->type == CoSuicide)
		  {
		  iofree(buf);
		  mcb->MsgHdr.FnRc = FG_Close;
		  Internet_Close(myco);
		  return;
		  }
		elif (myco->type == CoTimeout || myco->timelimit < Now)
		   Request_Return(SS_InterNet|EC_Recover|EG_Timeout|EO_Socket,
				0L, 0L);
		else
			goto retry;
		}
	     }
	else
	     {
	     mcb->Data = buf;
	     Request_Return(ReadRc_EOD, 0L, e);
	     mcb->Data = data;
	     }

done:
                                 /* make sure I get back into waiting list */
        PostInsert(Remove(&(myco->node)), Heliosnode);
	iofree(buf);
	return ;
}

/************************************************************************/
/*                                                                      */
/*  SendMessage - send a message taking care of out-of-band data,       */
/*                timeouts, etc                                         */
/*                                                                      */
/************************************************************************/

InternetDoSendMessage(myco)
Conode *myco;
{
	struct msghdr msg;
	struct iovec iov;
	DataGram *dg = (DataGram *)mcb->Control;
	Port myport = myco -> id;
	Port replyport = mcb->MsgHdr.Reply;
	word e;
	SockEntry *d;

	/* ServerDebug ("InternetDoSendMessage (%s)", myco->name); */

	if((d = Dir_find_socket(myco)) == (SockEntry *) NULL)
	{
		Request_Return(SS_InterNet|EC_Error|EG_Unknown|EO_Socket,0L,0L);
		goto done;
	}
	if( d->Oob )
	{
	     Request_Return(SS_InterNet|EC_Error|EG_Exception|EE_Signal|SIGURG,
				0L, 0L);
	     d->Oob = FALSE;
	     goto done;
	}
		
	if( (word)((dg->DataSize)+(mcb->MsgHdr.DataSize)) > maxdata)
	{
	    Request_Return(SS_InterNet|EC_Error|EG_WrongSize|EO_Message,0L,0L);
	    goto done;
	}

	mcb->MsgHdr.Flags = 0;
	mcb->MsgHdr.Dest = myport;
	mcb->Control[mcb->MsgHdr.ContSize++] = -1;  /* don't pass addr back */

	Request_Return(Err_Null, mcb->MsgHdr.ContSize, mcb->MsgHdr.DataSize);

	myco->timelimit = Now+20;	/* next message must arrive soon */

	Suspend();

	mcb->MsgHdr.Dest = NullPort;
	if(myco->type == CoSuicide)
		{
		Request_Return(SS_InterNet|EC_Error|EG_Broken|EO_Stream,0L,0L);
		Seppuku();
		return FALSE;
		}
	elif (myco->type == CoTimeout)
		{
		Request_Return(SS_InterNet|EC_Recover|EG_Timeout|EO_Stream,
					0L,0L);
		goto done;
		}
	
	msg.msg_iov = &iov;
	dgtomsg(mcb,&msg);

	dg = (DataGram *)mcb->Control;

#ifdef USE_sendto
	e = sendto (d->fd, (msg.msg_iov)->iov_base, (msg.msg_iov)->iov_len, 
			dg->Flags, msg.msg_name, msg.msg_namelen);
#else
	e = sendmsg(d->fd,&msg,dg->Flags);
#endif

	if( e == -1 )
	 {
	 if(errno == EWOULDBLOCK)
	   {
	   if(d->nonblocking)
		{
		   convert_errno();
		   Request_Return(SS_InterNet|EC_Error|EG_Errno|errno,0L,0L);
		}
	   else
	   Request_Return(SS_InterNet|EC_Recover|EG_Timeout|EO_Socket, 0L, 0L);
	   }
	 else
	   {
	   convert_errno();
	   Request_Return(EC_Error|EG_Errno|SS_InterNet|errno, 0L, 0L);
	   }
	 }

	elif( mcb->MsgHdr.Reply )
		Request_Return(ReplyOK, 0L, 0L);

done:

	return FALSE;
}

/************************************************************************/
/*                                                                      */
/*  RecvMessage - receive a message, suspending coroutine if needed     */
/*                                                                      */
/************************************************************************/

InternetDoRecvMessage(myco)
Conode *myco;
{
	DataGram *dg = (DataGram *)mcb->Control;
	word e;
	byte *buf = NULL;
	byte *data = mcb->Data;
	static struct sockaddr_in addr;
	struct msghdr msg;
	struct iovec iov;
	int timeout = ((dg->Timeout == -1)? 0 : dg->Timeout / time_unit);
	int reply = mcb->MsgHdr.Reply;
	int flags = dg->Flags;
	SockEntry *d;
#ifdef USE_recvfrom
	int	fromlen;
#endif

	/* ServerDebug ("InternetDoRecvMessage (%s)", myco->name); */

	if((d = Dir_find_socket(myco)) == (SockEntry *) NULL)
	{
		Request_Return(SS_InterNet|EC_Error|EG_Unknown|EO_Socket,
				0L, 0L);
		return FALSE;
	}

	if( d->Oob && (dg->Flags & MSG_OOB) == 0 )
	{
	     Request_Return(SS_InterNet|EC_Error|EG_Exception|EE_Signal|SIGURG,
				0L, 0L);
	     d->Oob = FALSE;
	     return FALSE;
	}
		
	buf = (char *)malloc((dg->DataSize)+sizeof(addr)+4);

	if( buf == NULL )
	{
	    Request_Return(SS_InterNet|EC_Error|EG_NoMemory|EO_Server,
				0L, 0L);
	    return FALSE;
	}

	iov.iov_base = buf;
	iov.iov_len = dg->DataSize;
	msg.msg_name = (caddr_t)&addr;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_accrights = NULL;	/* ignore accrights for now */
	msg.msg_accrightslen = 0;
                                 /* I might have to poll later */
        AddTail(Remove(&(myco->node)), PollingCo);
        myco->timelimit = timeout + Now;
	
retry:
	msg.msg_namelen = sizeof(addr);	/* set after retry cos kernel */
					/* buggers it if recv fails   */

#ifdef USE_recvfrom
	fromlen = sizeof (addr);

	e = recvfrom(d->fd, buf, dg->DataSize, flags,
			(struct sockaddr_in *)&(addr), &fromlen);
#else
	e = recvmsg(d->fd,&msg,flags);
#endif

#if (TR5 || i486V4)
	if ( (e == -1) && (errno == EAGAIN) ) errno = EWOULDBLOCK;
#endif

	if( e == -1 && errno == EINTR )
	{
	    /* ServerDebug ("InternetDoRecvMesg () - e == -1 && errno == EINTR"); */

	    if( d->Oob )
	      Request_Return(SS_InterNet|EC_Error|EG_Exception|EE_Signal|SIGURG,
				0L, 0L);
	    else
	      Request_Return(SS_InterNet|EC_Recover|EG_Timeout|EO_Socket,
				0L, 0L);

	    d->Oob = FALSE;
	    goto done;
	}
	
	if( e == -1) 
	{
		if(errno == EWOULDBLOCK)
		{
		   /* ServerDebug ("InternetDoRecvMesg () - e == -1 && errno == EWOULDBLOCK"); */

		   if(d->nonblocking)
		     {
		     convert_errno();
		     Request_Return(SS_InterNet|EC_Error|EG_Errno|errno,0L,0L);
		     goto done;
		     }
		   else
			{
			myco->type = CoReady;

			Suspend();

			mcb->MsgHdr.Dest = NullPort;
			mcb->MsgHdr.Reply = reply;
		   	if(myco->type == CoSuicide)
		   	 {
		   	 iofree(buf);
			 mcb->MsgHdr.FnRc = FG_Close;
			 Internet_Close(myco);
			 goto done;
		   	 }
		   	elif (myco->type == CoTimeout || myco->timelimit < Now)
			 {
			 errno = EINTR;
			 convert_errno();
			 Request_Return(SS_InterNet|EC_Error|EG_Errno|errno,
				0L, 0L);
			 goto done;
			 }
			goto retry;
			}
		}
		else	
		  {
			/* ServerDebug ("InternetDoRecvMesg () - e == -1 && errno = 0x%lx", errno); */
		  	convert_errno();
		 	 Request_Return(SS_InterNet|EC_Error|EG_Errno|errno,
					0L, 0L);
			  goto done;
		  }
	}

	mcb->Data = buf;
	mcb->MsgHdr.DataSize = e;
	mcb->Control[0] = 0;		/* No flags set */
	mcb->Control[1] = e;		/* data size */
	mcb->Control[2] = 0;		/* no timeout */
	mcb->Control[3] = -1;		/* no acc rights */
	mcb->Control[4] = -1;		/* no dest */
	{
		WORD hdr = *((WORD *)mcb);
		WORD offset = hdr & 0xFFFF;
		offset = (offset+3) & ~3;
		mcb->Control[5] = offset;	/* set source */
		*((WORD *) mcb) = (hdr & ~0xFFFF) + offset;
	}
	*(WORD *)&mcb->Data[mcb->MsgHdr.DataSize]
			= swap(sizeof(struct sockaddr_in));

#if SOLARIS
	memcpy (&mcb->Data[mcb->MsgHdr.DataSize+4], &addr, sizeof (struct sockaddr_in));
#else
	bcopy(&addr, &mcb->Data[mcb->MsgHdr.DataSize+4],
			sizeof(struct sockaddr_in));
#endif

	mcb->MsgHdr.DataSize += (sizeof(struct sockaddr_in) + 4);
	mcb->Control[6] = 0;		/* data is at start */
	
	Request_Return(ReplyOK, 7, mcb->MsgHdr.DataSize);
	
done:
                                 /* make sure I get back into waiting list */
        PostInsert(Remove(&(myco->node)), Heliosnode);
	iofree(buf);
	mcb->Data = data;	

	return FALSE;
}

/************************************************************************/
/*                                                                      */
/*  dgtomsg - convert data in data vector to msg structure.             */
/*                                                                      */
/************************************************************************/

void dgtomsg(mcb,msg)
MCB *mcb;
struct msghdr *msg;
{
	DataGram *dg = (DataGram *)mcb->Control;
	byte *data = mcb->Data;

#if TCPDEBUG
	ServerDebug("dgtomsg : DestAddr %x Data %x DataSize %x",
		     dg->DestAddr, dg->Data, dg->DataSize);
#endif
	
	if( dg->DestAddr != -1 )
	{
	   msg->msg_name = (caddr_t)(data+dg->DestAddr+4);
	   ((struct sockaddr_in *) (msg->msg_name))->sin_family =
              swap_short(((struct sockaddr_in *) (msg->msg_name))->sin_family);
	   msg->msg_namelen = swap(*(word *)(data+dg->DestAddr));
	}
	else msg->msg_name = NULL, msg->msg_namelen = 0;
	
#if TCPDEBUG
	ServerDebug("data %x dg->Data %x", data,dg->Data);
	ServerDebug("data+dg->Data %x, DataSize %x",
		     data+dg->Data, dg->DataSize);
#endif

	msg->msg_iov->iov_base = data+dg->Data;
	msg->msg_iov->iov_len = dg->DataSize;
	msg->msg_iovlen = 1;
	
	if( dg->AccRights != -1 )
	{
		msg->msg_accrights = (caddr_t)(data+dg->AccRights+4);
		msg->msg_accrightslen = swap(*(word *)(data+dg->AccRights));
	}
	else msg->msg_accrights = NULL, msg->msg_accrightslen = 0;
}

/************************************************************************/
/*                                                                      */
/*   Select - use the I/O server's Multiwait to wait for a select       */
/*            operation on a socket. The AddMultiwait and Multiwait     */
/*            routines have been modified to allow the result of the    */
/*            select to be placed directly in the appropriate socket    */
/*            structure.                                                */
/*                                                                      */
/************************************************************************/

void Internet_Select(myco)
Conode *myco;
{
	int result = 0;
	int fnmode = (mcb->MsgHdr.FnRc & 0xf);	/* select function */
	SockEntry *d;
	int e = 0;
	Port replyport = mcb->MsgHdr.Reply;

	/* ServerDebug ("Internet_Select (%s)", myco->name); */
	
	if((d = Dir_find_socket(myco)) == (SockEntry *) NULL)
	{
		Request_Return(SS_InterNet|EC_Error|EG_Unknown|EO_Socket,0L,0L);
		return ;
	}

	if(d -> selfn & fnmode)		/* outstanding select */
		clearselect(myco, d);

	d -> selfn |= fnmode;		/* save for later */
	d -> selval = 0;

	if( fnmode & O_ReadOnly )
		AddMultiwait(Multi_SocketInput, &myco->type, d->fd, (long)(&d->selval));
	if( fnmode & O_WriteOnly )
		AddMultiwait(Multi_SocketOutput, &myco->type, d->fd, (long)(&d->selval));
	if( fnmode & O_Exception )
		AddMultiwait(Multi_SocketExcp, &myco->type,  d->fd, (long)(&d->selval));

	myco -> type = 0;
                                  /* suspend myself until selected */
        AddTail(Remove(&(myco->node)), SelectCo);

        Suspend();
                                 /* make sure I get back into waiting list */
        PostInsert(Remove(&(myco->node)), Heliosnode);

   	if(myco->type == CoSuicide)
	   	{
		mcb->MsgHdr.Reply = replyport;
		mcb->MsgHdr.Dest = NullPort;
	   	Request_Return(SS_InterNet|EC_Error|EG_Broken|EO_Stream,0L, 0L);
	   	Seppuku();
		return;
	   	}

	result = clearselect(myco, d);

	if(myco -> type == CoAbortSelect)
		return;
		
	mcb->MsgHdr.Reply = replyport;
	mcb->MsgHdr.Dest = NullPort;

	Request_Return(result, 0L, 0L);
}

/************************************************************************/
/*                                                                      */
/*  clearselect - remove select operations. Called after select, close  */
/*                and possibly read and write.                          */
/*                                                                      */
/************************************************************************/

int clearselect(myco, d)
Conode *myco;
SockEntry *d;
{
	int result;
	if(d->selfn & O_ReadOnly)
	{
#if SOLARIS
		/* The last 0 argument is a dummy value to keep the C++ compiler happy */
		ClearMultiwait(Multi_SocketInput, d->fd, 0);
#else
		ClearMultiwait(Multi_SocketInput, d->fd);
#endif
	}
	if(d->selfn & O_WriteOnly)
	{
#if SOLARIS
		/* The last 0 argument is a dummy value to keep the C++ compiler happy */
		ClearMultiwait(Multi_SocketOutput,d->fd, 0);
#else
		ClearMultiwait(Multi_SocketOutput,d->fd);
#endif
	}
	if(d->selfn & O_Exception)
	{
#if SOLARIS
		/* The last 0 argument is a dummy value to keep the C++ compiler happy */
		ClearMultiwait(Multi_SocketExcp,  d->fd, 0);
#else
		ClearMultiwait(Multi_SocketExcp,  d->fd);
#endif
	}
	result = d -> selval;
	d -> selval = 0;
	d -> selfn  = 0;

	return result;
}

/************************************************************************/
/*                                                                      */
/*  (S)GetSize - do a (s)getsize operation on socket                    */
/*                                                                      */
/************************************************************************/

void Internet_SetSize(myco)
Conode *myco;
{
	int size = mcb->Control[0];
	int e;
	SockEntry *d;

	/* ServerDebug ("Internet_SetSize (%s)", myco->name); */

	if((d = Dir_find_socket(myco)) == (SockEntry *) NULL)
	{
		Request_Return(SS_InterNet|EC_Error|EG_Unknown|EO_Socket,0L,0L);
		return ;
	}
	
	e = setsockopt(d->fd,SOL_SOCKET,SO_SNDBUF,(char *)(&size),sizeof(int));

	if( e == -1 )
	{
		convert_errno();
		Request_Return(SS_InterNet|EC_Error|EG_Errno|errno, 0L, 0L);
		return ;
	}
	
	mcb->MsgHdr.Dest = NullPort;
	Request_Return(ReplyOK, 0L, 0L);
		
	return ;
}

void Internet_GetSize(myco)
Conode *myco;
{
	int size;
	int e;
	SockEntry *d;

	/* ServerDebug ("Internet_GetSize (%s)", myco->name); */
	
	if((d = Dir_find_socket(myco)) == (SockEntry *) NULL)
	{
		Request_Return(SS_InterNet|EC_Error|EG_Unknown|EO_Socket,0L,0L);
		return ;
	}
	
	e = ioctl(d->fd,FIONREAD,&size);

	if( e == -1 )
	{
		convert_errno();
		Request_Return(SS_InterNet|EC_Error|EG_Errno|errno, 0L, 0L);
		return ;
	}
	
	mcb->Control[0] = size;
	Request_Return(ReplyOK, 1L, 0L);
		
	return ;
}

/************************************************************************/
/*                                                                      */
/*  ObjectInfo - return info for socket or server                       */
/*                                                                      */
/************************************************************************/

void Internet_ObjectInfo(myco)
Conode *myco;
{ SockEntry *node;
                       /* put info in data vector straightaway, nothing in */
                        /* there that we need any more */
  register ObjInfo *Heliosinfo = (ObjInfo *) mcb->Data;

             /* The ObjectInfo information is in the Data vector so it does */
             /* not get swapped automatically by the message passing        */
             /* routines. All integers must be swapped explicitly.          */

  Heliosinfo->DirEntry.Matrix = swap(DefSockMatrix);
  Heliosinfo->Size            =
  Heliosinfo->DirEntry.Flags  =
  Heliosinfo->Account         = swap(0L);
  Heliosinfo->Creation        =
  Heliosinfo->Access          =
  Heliosinfo->Modified        = swap(Startup_Time);
             
  if (!strcmp(IOname, myco->name))    /* is it for the server ? */
   { Heliosinfo->DirEntry.Type   = swap(Type_Directory);
     Heliosinfo->DirEntry.Matrix = swap(DefDirMatrix);
     strcpy(Heliosinfo->DirEntry.Name, myco->name);
     Request_Return(ReplyOK, 0L, (word) sizeof(ObjInfo));
   }
  elif ( (node = Dir_find_sockname(myco, IOname) ) eq (SockEntry *) NULL)
   Request_Return(EC_Error + SS_InterNet + EG_Unknown + EO_Socket, 0L, 0L);
  else
   { Heliosinfo->DirEntry.Type   = swap(Type_Socket);
     strcpy(Heliosinfo->DirEntry.Name, node->ObjNode.direntry.Name);
     Heliosinfo->Size            = swap(node->ObjNode.size);
     Heliosinfo->Account         = swap(node->ObjNode.account);
     Heliosinfo->Access          = swap(node->access);
     Heliosinfo->Modified        =
     Heliosinfo->Creation        = swap(node->creation);
     Request_Return(ReplyOK, 0L, (word) sizeof(ObjInfo));
   }
}

/************************************************************************/
/*                                                                      */
/*  oobdata - handle out of band data. Called when SIGURG signal is     */
/*                  received.                                           */
/*                                                                      */
/************************************************************************/

void oobdata()
{
	SockEntry *node;
	List *list = (List *) &(Internet_extra);
	fd_set exfds;
	struct timeval tim;

	memset(&tim, 0, sizeof(tim));	/* just poll */

  	for (node = (SockEntry *) list->head;
       		node->ObjNode.node.next ne (Node *) NULL;
       		node = (SockEntry *) node->ObjNode.node.next)
	{
	   FD_ZERO(&exfds);
	   FD_SET(node->fd, &exfds);	/* try this one */
#if 0
	   if(select(1, NULL, NULL, &exfds, tim) > 0)	/* it was this one */
#else
	   if (select(node->fd + 1, NULL, NULL, &exfds, &tim) > 0)
#endif
		node->Oob = TRUE;
	   else
#if (TR5 || i486V4)
                {
		node->Oob = FALSE;
		FD_ZERO(&exfds);
	        }
#endif
		node->Oob = FALSE;
	}
}

/************************************************************************/
/*                                                                      */
/*  addint - add a number to a string                                   */
/*                                                                      */
/************************************************************************/

PRIVATE word addint(s, i)
char *s;
word i;
{	
	int len;

	if( i == 0 ) return strlen(s);

	len = addint(s,i/10);
  
	s[len] = (i%10) + '0';
  
	s[len+1] = '\0';

	return len+1;
}

/************************************************************************/
/*                                                                      */
/*  setsu - set effective uid of process                                */
/*                                                                      */
/************************************************************************/

setsu(onoff)
int onoff;
{
	if(onoff == -1)	/* first call */
	{
		realuid = getuid();
		effuid = geteuid();
#if SM90 || SCOUNIX
		setuid(effuid);
#endif
		onoff = 0;
	}

	return(seteuid(onoff ? effuid : realuid));
}
#endif /* internet_supported */

/************************************************************************/
/*                                                                      */
/*  Convert_errno - routine to convert UNIX style error number to the   */
/*                  corresponding Helios error number.                  */
/*                                                                      */
/************************************************************************/

int helios_errs[] = {
	0, 
	E2BIG, EACCES, EAGAIN, EBADF, EBUSY, ECHILD, EDEADLK, EDOM, EEXIST,
	EFAULT, EFBIG, EINTR, EINVAL, EIO, EISDIR, EMFILE, EMLINK, ENAMETOOLONG,
	ENFILE, ENODEV, ENOENT, ENOEXEC, ENOLCK, ENOMEM, ENOSPC, ENOTDIR,
	ENOTEMPTY, ENOTTY, ENXIO, EPERM, EPIPE, ERANGE, EROFS, ESPIPE, ESRCH,
	EXDEV, EWOULDBLOCK, EINPROGRESS, EALREADY, ENOTSOCK, EDESTADDRREQ,
	EMSGSIZE, EPROTOTYPE, ENOPROTOOPT, EPROTONOSUPPORT, ESOCKTNOSUPPORT,
	EOPNOTSUPP, EPFNOSUPPORT, EAFNOSUPPORT, EADDRINUSE, EADDRNOTAVAIL,
	ENETDOWN, ENETUNREACH, ENETRESET, ECONNABORTED, ECONNRESET, ENOBUFS,
	EISCONN, ENOTCONN, ESHUTDOWN, ETIMEDOUT, ECONNREFUSED, EHOSTDOWN,
	EHOSTUNREACH, -1
	};

void convert_errno()		/* yuck - I hate it, I hate it !! */
{
	/* convert UNIX error numbers to Helios error numbers */

	int i;

	for(i = 0 ; helios_errs[i] != -1 ; i++)
		if(helios_errs[i] == errno)
			{ errno = i; break; }

}
