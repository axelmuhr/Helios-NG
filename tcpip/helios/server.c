#include <helios.h>
#include <syslib.h>
#include <servlib.h>
#include <codes.h>
#include <gsp.h>
#include <module.h>
#include <device.h>
#include <strings.h>
#include <nonansi.h>

#include "param.h"

/* #define __IN_SERVER__ defined in param.h */

#include <sys/socket.h>

#include "mbuf.h"
#include "socketvar.h"
#include "file.h"
#include "netinet/in.h"
#include "proc.h"
#include "user.h"
#include "net/netisr.h"
#include "net/if.h"
#include "netinet/if_ether.h"
#include "local_ioctl.h"



#define Task_Flags_showcalls	1024

#define SHOWCALLS 0

/* my own MsgBuf structure */
typedef struct MMsgBuf {
	MCB		mcb;			/* message control block*/
	word		control[IOCMsgMax];	/* control vector	*/
	byte		data[IOCDataMax]; 	/* data vector		*/
} MMsgBuf;

typedef struct SockEntry
{
	ObjNode		ObjNode;
	Port		Server;
	int		Users;
	int		Protocol;
	bool		Bound;
	
	void		(*SysCall)();
	
	/* Select stuff */
	Port		SelectPort;
	word		SelectMode;
	Semaphore	SelectLock;

	bool		Oob;
		
	/* UNIX structures */
	struct file	File;	
	struct user	u;
	struct proc	proc;	
} SockEntry;

struct sleeper
{
	Node		node;
	caddr_t		chan;
	int		pri;
	word		endtime;
	bool		timeout;
	SockEntry	*sock;
	Semaphore	wait;
};

DirNode		Root;			/* root of server name tree	*/
DirNode	*	TcpNode;		/* sub-root of tcp sockets	*/
DirNode	*	UdpNode;		/* sub-root of udp sockets	*/
DirNode	*	RawNode;		/* sub-root of raw sockets	*/
Object *	RootObj;		/* root name-table object	*/
int		MyAddr;			/* my internet address		*/
int		SubNetMask;		/* my subnet mask		*/
char *		MyName;			/* my machine name		*/
int		SockId = 1;		/* seed for nonce ids		*/
int		NextPid = 1;		/* seed for fake pids		*/


#define DefSockMatrix	DefFileMatrix

#define StackSize	3000

#define TCPDataMax	10000

static void SocketServer( SockEntry * s );

extern void socket( void );
extern void listen( void );
extern void accept( void );
extern void connect( void );
extern void bind( void );
extern void sendmsg( void );
extern void recvmsg( void );
extern void getsockopt( void );
extern void setsockopt( void );
extern void getsockname( void );
extern void getpeername( void );
extern void shutdown( void );

extern struct arpcom ec_softc;
extern struct ifnet loif;
extern List   sleepq;

void ether_addr(u_char *addr, char *str);

static NetDevInfo *getdevinfo(char *name);
extern NetDevInfo *EtherInfo;

extern int MemAlloced;
#ifndef MMalloc
extern void *MMalloc(int size);
extern void FFree(void *v);
#endif

/* Unix compatability */

extern Semaphore kernel;
extern char * procname( VoidFnPtr );

struct file *getf(SockEntry *s)
{
	return &s->File;
}

void tokernel(void)
{
	Wait(&kernel);
}

void fromkernel(void)
{
	while( netisr )
	{
		if( netisr & (1<<NETISR_IP) )
		{ netisr &= ~(1<<NETISR_IP); ipintr(); }
		if( netisr & (1<<NETISR_RAW) ) 
		{ netisr &= ~(1<<NETISR_RAW); rawintr(); }
	}
	Signal(&kernel);
}

int syscall(SockEntry *s, void (*fn)(), ... )
{
	void *ap = ((word *)(&fn))+1;

#if SHOWCALLS
if( MyTask->Flags & Task_Flags_showcalls )
	IOdebug("syscall <%s> %s(%x %x %x)",s->ObjNode.Name,procname(fn),
		((int *)ap)[0],((int *)ap)[1],((int *)ap)[2]);
#endif
	
	s->u.u_ap = ap;

	if ( s->SysCall != NULL ) 
	{
		IOdebug("SysCall %s: call %s already in progress!!",procname(fn),procname(s->SysCall));
		return EALREADY;
	}
	
	s->SysCall = fn;

	tokernel();

	u         = s->u;
	u.u_error = 0;

	if ( setjmp(u.u_qsave) == 0 )
	  {
	    (*fn)();
	  }	
	else
	  {
	    if( u.u_error == 0 ) u.u_error = EINTR;
	  }
	
	s->u           = u;
	s->SysCall     = NULL;
	s->u.u_timeout = 0;
	
	fromkernel();

#if SHOWCALLS
if( MyTask->Flags & Task_Flags_showcalls && s->u.u_error != 0 )
	IOdebug("syscall <%s> %s done error %d",
		s->ObjNode.Name,procname(fn),s->u.u_error);
#endif
	return s->u.u_error;
}

static word do_abort(struct sleeper *s, SockEntry *se)
{
	if( s->sock == se )
	{
#if SHOWCALLS
	  IOdebug("abort syscall %s for %s",procname(se->SysCall),se->ObjNode.Name);
#endif
		s->timeout = TRUE;
		Remove(&s->node);
		Signal(&s->wait);
	}
	return 0;
}

void abortsyscalls(SockEntry *s, bool inkernel)
{
	if( s->SysCall == NULL ) return;
	
#if SHOWCALLS
if( MyTask->Flags & Task_Flags_showcalls )
	IOdebug("abortsyscall <%s> %s",s->ObjNode.Name,procname(s->SysCall));
#endif
	if( !inkernel ) tokernel();
	WalkList(&sleepq,do_abort,s);
	if( !inkernel ) fromkernel();
}

/* sleep/wakeup system */

static int curtime = 1;

void sleep(caddr_t chan, int pri)
{
	struct sleeper s;
	SockEntry *se = (SockEntry *)u.u_procp->p_sock;

	s.chan = chan;
	s.pri = pri;
	s.sock = se;
		
	if( pri < PZERO || u.u_timeout == 0 ) s.endtime = 0;
	else s.endtime = (word)curtime+u.u_timeout;
	
	s.timeout = FALSE;
	InitSemaphore(&s.wait,0);

	AddTail(&sleepq,&s.node);

	se->u = u;
		
	fromkernel();
	Signal(&se->ObjNode.Lock);

	/* IOdebug( "sleep: waiting on %x, endtime = %d, timeout = %d",
		&s.wait, s.endtime, u.u_timeout ); */
	
	Wait(&s.wait);

	/* IOdebug( "sleep: wait over, timeout = %d", s.timeout ); */
	
	Wait(&se->ObjNode.Lock);
	
	tokernel();

	u = se->u;

	/* if we were woken up by a timeout we simulate having been zapped */
	/* by a signal							   */

	if ( s.timeout )
	  {
	    longjmp(u.u_qsave,1);
	  }	
}

static word do_wakeup(struct sleeper *s, caddr_t chan)
{
	if( s->chan == chan )
	{
		Remove(&s->node);
		Signal(&s->wait);
	}
	return 0;
}

void wakeup(caddr_t chan)
{
	WalkList(&sleepq,do_wakeup,chan);
}

static word do_timeout(struct sleeper *s)
{
	if( (s->endtime > 0) && (s->pri > PZERO) && (s->endtime <= curtime) )
	{
		s->timeout = TRUE;
		Remove(&s->node);
		Signal(&s->wait);
	}
	return 0;	
}

void sleep_timer()
{	
	for(;;)
	{
		Delay(OneSec);
		curtime++;
		tokernel();
		WalkList(&sleepq,do_timeout);
		fromkernel();
	}
}

void psignal( struct proc *p, int sig )
{
	SockEntry *s = (SockEntry *)p->p_sock;
	
	if( sig == SIGURG ) 
	{
		s->Oob = TRUE;
		abortsyscalls(s,TRUE);
	}
}

word findproc(SockEntry *s, int pid) { return pid == s->proc.p_pid; }

struct proc *pfind(int pid)
{
	SockEntry *s;
	
	s = (SockEntry *)SearchList(&TcpNode->Entries,findproc,pid);
	if( !s ) s = (SockEntry *)SearchList(&UdpNode->Entries,findproc,pid);
	if( !s ) s = (SockEntry *)SearchList(&RawNode->Entries,findproc,pid);
	
	if( s ) return &s->proc;
	else return NULL;
}

/* unp_connect2 called from pipe() which we do not use */
int unp_connect2() { return -1; }

MMsgBuf *NewMMsgBuf()
{
	MMsgBuf *m;

	while( (m = New(MMsgBuf)) == NULL ) Delay(OneSec);

	m->mcb.Control = m->control;
	m->mcb.Data = m->data;
	
	return m;
}

SockEntry *NewSocket(DirNode *dir, char *name, int proto)
{
	SockEntry *s = (SockEntry *) MMalloc(sizeof(SockEntry));
	
	if( s == NULL ) return NULL;
	
	memset(s,0,sizeof(SockEntry));
	
	InitNode(&s->ObjNode,name,Type_Socket,0,DefSockMatrix);

	/* init Helios level fields */
	
	s->Protocol = proto;
	s->Users = 1;
	s->Server = NewPort();
	s->Bound = FALSE;
	s->Oob = FALSE;
		
	s->SelectPort = NullPort;
	InitSemaphore(&s->SelectLock,1);
	
	/* the file structure will be filled in by socket or accept */
	
	/* the user structure contains just a few useful things	*/
	
	s->u.u_procp   = (struct proc *)&s->proc;
	s->u.u_timeout = 0;
		
	/* proc structure is vestigial */

	s->proc.p_pid = NextPid++;	
	s->proc.p_wchan = NULL;
	s->proc.p_sock = (caddr_t)s;
	
	Insert(dir,&s->ObjNode,FALSE);
	
	return s;
}

int ufalloc(int arg) { return 0; }

struct file *falloc()
{
	SockEntry *s = (SockEntry *)u.u_procp->p_sock;

	if( s->SysCall == socket )
	{
		/* For socket the structures already exist, just return	*/
		/* a pointer to the file structure.			*/
		u.u_r.r_val1 = 0;
		return &s->File;
	}
	elif( s->SysCall == accept )
	{
		SockEntry *New;
		char newname[NameMax];

		strcpy(newname,s->ObjNode.Name);
		strcat(newname,".");
		addint(newname,SockId++);

		New = NewSocket(s->ObjNode.Parent,newname,s->Protocol);
		
		if( New == NULL )
		{
			u.u_r.r_val1 = 0;
			return NULL;
		}
		else
		{
			u.u_r.r_val1 = (int)New;
			return &New->File;
		}
	}
	u.u_r.r_val1 = 0;
	return NULL;
}

extern void MarshalStruct(MCB *mcb, void *data, word size)
{
	if( data == NULL || size == 0 ) MarshalWord(mcb,-1); 
	else {
		MarshalOffset(mcb);
		MarshalData(mcb,sizeof(word),(byte *)&size);
		MarshalData(mcb,size,(char *) data);
	}
}

void MarshalAddr(MCB *mcb, struct sockaddr_in *addr)
{
	MarshalStruct(mcb,addr,sizeof(struct sockaddr_in));
}

void dgtomsg(MCB *mcb, struct msghdr *msg)
{
	DataGram *dg = (DataGram *)mcb->Control;
	byte *data = mcb->Data;
	
	if( dg->DestAddr != -1 )
	{
		msg->msg_name    = (caddr_t)(data+dg->DestAddr+sizeof(word));
		msg->msg_namelen = *(int *)(data+dg->DestAddr);
	}
	else msg->msg_name = NULL, msg->msg_namelen = 0;
	
	msg->msg_iov->iov_base = data+dg->Data;
	msg->msg_iov->iov_len  = (int)dg->DataSize;
	msg->msg_iovlen        = 1;
	
	if( dg->AccRights != -1 )
	{
		msg->msg_accrights = (caddr_t)(data+dg->AccRights+sizeof(word));
		msg->msg_accrightslen = *(int *)(data+dg->AccRights);
	}
	else msg->msg_accrights = NULL, msg->msg_accrightslen = 0;
}

bool DoConnect(MCB *mcb, SockEntry *s)
{
	ConnectRequest *cr = (ConnectRequest *)mcb->Control;
	struct sockaddr_in *addr = NULL;
	Port reply = mcb->MsgHdr.Reply;
	word e;
	
	if( cr->DestAddr >= 0 ) addr = (struct sockaddr_in *)(mcb->Data+cr->DestAddr+sizeof(word));
	
	e = syscall(s,connect,s,addr,sizeof(struct sockaddr_in));
	
	if( e == 0 && s->Protocol & 0xf == SOCK_STREAM )
	{
		Capability cap;
		char *p;
		
		InitMCB(mcb,0,reply,s->Server,SS_InterNet);
	
		MarshalWord(mcb,s->ObjNode.Type);
		MarshalWord(mcb,s->ObjNode.Flags|Flags_Server|Flags_Selectable|Flags_Closeable);
		NewCap(&cap,&s->ObjNode,AccMask_R|AccMask_W);
		MarshalCap(mcb,&cap);
		MarshalOffset(mcb);
	
		p = mcb->Data;
		*p = 0;
		MachineName(p);
		pathcat(p,Root.Name);
		pathcat(p,TcpNode->Name);
		pathcat(p,s->ObjNode.Name);
		mcb->MsgHdr.DataSize = strlen(p)+1;
		
		PutMsg(mcb);
	}
	else
	{
		InitMCB(mcb,0,reply,NullPort,e?EC_Error|EG_Errno|e:0);
		PutMsg(mcb);
	}
	return FALSE;
}

static bool DoListen(MCB *mcb, SockEntry *s)
{
	word e;

	e = syscall(s,listen,s,mcb->Control[0]);
	
	ErrorMsg(mcb,e?EC_Error|EG_Errno|e:0);
	
	return FALSE;
}

static bool DoAccept(MCB *mcb, SockEntry *s)
{
	word e;
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	Port reply = mcb->MsgHdr.Reply;
	SockEntry *New;

	s->u.u_timeout = 10;
	
	e = syscall(s,accept,s,&addr,&addrlen);
	
	if( e == EINTR ) { ErrorMsg(mcb,EC_Recover|EG_Timeout|EO_Socket); return FALSE; }
	
	if( e ) goto errormsg;

	New = (SockEntry *)s->u.u_r.r_val1;

	/* fork a New server */
		
	if( !Fork(StackSize,SocketServer,sizeof(New),New) )
	{ e = ENOMEM; goto errormsg; }
	else MemAlloced += StackSize;
	
	/* now reply to the client */

	{
		Capability cap;
		char *p;
		
		InitMCB(mcb,0,reply,New->Server,SS_InterNet);
	
		MarshalWord(mcb,New->ObjNode.Type);
		MarshalWord(mcb,New->ObjNode.Flags|Flags_Server|Flags_Selectable|Flags_Closeable);
		NewCap(&cap,&New->ObjNode,AccMask_R|AccMask_W);
		MarshalCap(mcb,&cap);
		MarshalOffset(mcb);
	
		p = mcb->Data;
		MachineName(p);
		pathcat(p,Root.Name);
		pathcat(p,TcpNode->Name);	/* only TCP call this!! */
		pathcat(p,New->ObjNode.Name);

		mcb->MsgHdr.DataSize = strlen(p)+1;
	
		MarshalAddr(mcb,&addr);

		PutMsg(mcb);
	}
		
	return FALSE;
	
errormsg:	
	InitMCB(mcb,0,reply,NullPort,e<=0?e:EC_Error|EG_Errno|e);
	PutMsg(mcb);

	return FALSE;
}

#define OLDSEL 0

/* @@@ There appears to be a problem here in that under certain conditions */
/* when selwakeup is called, it polls the socket conditions and comes up   */
/* with nothing. This seems to be connected to the closedown of TCP streams*/
/* and manifests itself as a 20 second timeout in termination of programs  */
/* like rsh.								   */
static void sysselect()
{
	struct a {
		int	s;
		int	which;
	} *uap = (struct a *)u.u_ap;
	struct file *fp;
	int which = uap->which;
	int result = 0;
	register struct socket *so;

	fp = (struct file *)getsock(uap->s);
	so = (struct socket *)fp->f_data;
#if OLDSEL
	u.u_error = (*fp->f_ops->fo_select)(fp,which);
#else
	if( (which & O_ReadOnly ) && (*fp->f_ops->fo_select)(fp,FREAD)) result |= O_ReadOnly;
	if( (which & O_WriteOnly ) && (*fp->f_ops->fo_select)(fp,FWRITE)) result |= O_WriteOnly;
	if( (which & O_Exception ) && (*fp->f_ops->fo_select)(fp,0)) result |= O_Exception;
	u.u_error = result;
#endif
}

void selwakeup(struct proc *p, int flags) 
{
	SockEntry *s = (SockEntry *)p->p_sock;
	MCB mcb;

	Wait(&s->SelectLock);

	if( s->SelectPort != NullPort )
	{
		struct file *fp = &s->File;
		int fn      = (int)s->SelectMode;
		int result = 0;

		if( (fn & O_ReadOnly ) && (*fp->f_ops->fo_select)(fp,FREAD)) result |= O_ReadOnly;
		if( (fn & O_WriteOnly ) && (*fp->f_ops->fo_select)(fp,FWRITE)) result |= O_WriteOnly;
		if( (fn & O_Exception ) && (*fp->f_ops->fo_select)(fp,0)) result |= O_Exception;
#if SHOWCALLS
if( MyTask->Flags & Task_Flags_showcalls )
	IOdebug("selwakeup <%s> port %x mode %x result %x",
		s->ObjNode.Name,s->SelectPort,fn,result);
#endif
		if( result )
		{
			InitMCB(&mcb,0,s->SelectPort,NullPort,result);
			PutMsg(&mcb);
			s->SelectPort = NullPort;
		}
	}
	Signal(&s->SelectLock);
}

static bool DoSelect(MCB *mcb, SockEntry *s, int fn)
{
	int result = 0;

	fn &= FF_Mask;
#if OLDSEL
	if( (fn & O_ReadOnly)  && syscall(s,sysselect,s,FREAD) ) result |= O_ReadOnly;
	if( (fn & O_WriteOnly) && syscall(s,sysselect,s,FWRITE) ) result |= O_WriteOnly;	
	if( (fn & O_Exception) && syscall(s,sysselect,s,0)) result |= O_Exception;
#else
	result = syscall(s,sysselect,s,fn);
#endif
#if SHOWCALLS
if( MyTask->Flags & Task_Flags_showcalls )
	IOdebug("select <%s> port %x mode %x result %x",
		s->ObjNode.Name,mcb->MsgHdr.Reply,fn,result);
#endif

	Wait(&s->SelectLock);

	if( result ) ErrorMsg(mcb,result);
	else
	{
		FreePort(s->SelectPort);
		s->SelectPort = mcb->MsgHdr.Reply;
		s->SelectMode = (word)fn & FF_Mask;
	}

	Signal(&s->SelectLock);
	
	return FALSE;
}

static bool DoSendMessage(MCB *mcb, SockEntry *s)
{
	struct msghdr msg;
	struct iovec iov;
	DataGram *dg = (DataGram *)mcb->Control;
	Port dataport = NewPort();
	byte *buf = (char * ) MMalloc(dg->DataSize+mcb->MsgHdr.DataSize);
	byte *data = mcb->Data;
	word e;

	if( s->Oob )
	{
		ErrorMsg(mcb,EC_Error|EG_Exception|EE_Signal|SIGURG);
		s->Oob = FALSE;
		goto done;
	}
		
	if( buf == NULL ) 
	{
		ErrorMsg(mcb,EC_Error|EG_NoMemory|EO_Message);
		goto done;
	}

	mcb->MsgHdr.Flags = 0;
	mcb->MsgHdr.Dest = mcb->MsgHdr.Reply;
	mcb->MsgHdr.Reply = dataport;
	mcb->MsgHdr.FnRc = Err_Null;
	
	MarshalWord(mcb,-1);		/* no need to pass addr back */

	e = PutMsg(mcb);
	
	if( e < 0 ) goto done;
	
	/* now prepare for data to arrive */
	
	mcb->MsgHdr.Dest = dataport;
	mcb->Data = buf;
	
	e = GetMsg(mcb);

	if( e < 0 ) goto done;

	msg.msg_iov = &iov;
	dgtomsg(mcb,&msg);

	s->u.u_timeout = dg->Timeout == -1 ? 0 : (int)(dg->Timeout / OneSec);

	e = syscall(s,sendmsg,s,&msg,dg->Flags);

	if( mcb->MsgHdr.Reply != NullPort )
	{
		InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,e?EC_Error|EG_Errno|e:0);
		PutMsg(mcb);
	}

done:
	FreePort(dataport);
	FFree(buf);
	mcb->Data = data;

	return FALSE;
}

static bool DoRecvMessage(MCB *mcb, SockEntry *s)
{
	DataGram *dg = (DataGram *)mcb->Control;
	word e;
	byte *buf = NULL;
	byte *data = mcb->Data;
	struct sockaddr_in addr;
	struct msghdr msg;
	struct iovec iov;
	int timeout = (int)dg->Timeout;

	/* IOdebug( "TCPIP: DoRecvMessage: called for %d", dg->DataSize ); */
	
	if( s->Oob && (dg->Flags & MSG_OOB) == 0 )
	{
		ErrorMsg(mcb,EC_Error|EG_Exception|EE_Signal|SIGURG);
		s->Oob = FALSE;

		return FALSE;
	}
		
	buf = (char * ) MMalloc(dg->DataSize+sizeof(addr)+sizeof(word));
	
	iov.iov_base = buf;
	iov.iov_len  = (int)dg->DataSize;
	msg.msg_name = (caddr_t)&addr;
	msg.msg_namelen = sizeof(addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_accrights = NULL;	/* ignore accrights for now */
	msg.msg_accrightslen = 0;

	if( timeout == -1 ) timeout = 0;
	else timeout /= (int)OneSec;
	
	s->u.u_timeout = timeout;

	/* IOdebug( "TCPIP: DoRecvMessage: receiving, timeout = %d", timeout ); */
	
	e = syscall(s,recvmsg,s,&msg,dg->Flags);

	/* IOdebug( "TCPIP: DoRecvMessage: syscall returned, e = %x", e ); */
	
	if( e == EINTR )
	{
		if( s->Oob ) e = EC_Error|EG_Exception|EE_Signal|SIGURG;
		elif( timeout ) e |= EC_Error|EG_Errno;
		else e = EC_Recover|EG_Timeout|EO_Socket;
		s->Oob = FALSE;
		ErrorMsg(mcb,e); 
		goto done; 
	}
	
	if( e ) 
	{ ErrorMsg(mcb,EC_Error|EG_Errno|e); goto done; }

	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,0);
	mcb->Data = buf;
	mcb->MsgHdr.DataSize = s->u.u_r.r_val1;
	MarshalWord(mcb,0);			/* no flags set		*/
	MarshalWord(mcb,s->u.u_r.r_val1);	/* actual data size	*/
	MarshalWord(mcb,0);			/* no timeout		*/
	MarshalWord(mcb,-1);			/* no acc rights 	*/
	MarshalWord(mcb,-1);			/* no dest (its me!)	*/
	MarshalAddr(mcb,&addr);			/* set source		*/
	MarshalWord(mcb,0);			/* data is at start	*/
	
	e = PutMsg(mcb);			/* return message	*/

	mcb->Data = data;	
done:
	FFree(buf);

	return FALSE;
}

static void ioctl()
{
	struct a {
		int fd;
		int cmd;
		caddr_t data;
	} *uap = (struct a *)u.u_ap;
	struct file *fp;
	
	fp = (struct file *)getsock(uap->fd);
	
	u.u_error = (*fp->f_ops->fo_ioctl)(fp,uap->cmd,uap->data);
}

static bool DoSetInfo(MCB *mcb, SockEntry *s, int fn)
{
	word e;
	SocketInfoReq *si = (SocketInfoReq *)mcb->Control;
	caddr_t val = NULL;
	word valsize = 0;

/*
-- crf: 17/03/93 - Invalid request ?
*/
	if ((mcb->MsgHdr.ContSize * sizeof(word)) != sizeof(SocketInfoReq))
	{
		ErrorMsg(mcb,EC_Error|SS_InterNet|EG_WrongFn|EO_Socket);
		return(FALSE);
	}

	if( si->Optval != -1 )
	{
		val = mcb->Data+si->Optval+sizeof(word);
		valsize = *(word *)(mcb->Data+si->Optval);
	}

	if( si->Level == SOL_IOCTL ) 
	{
		/* if IOCTL is set owner, use my local pid in place of arg */
		if( si->Option == SIOCSPGRP ) *(int *)val = s->proc.p_pid;
		e = syscall(s,ioctl,s,si->Option,val);
	}
	else e = syscall(s,setsockopt,s,si->Level,si->Option,val,valsize);
	
	if( e ) ErrorMsg(mcb,EC_Error|EG_Errno|e);
	else
	{
		InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,0);
		mcb->MsgHdr.ContSize += 2;
		PutMsg(mcb);
	}
	
	return FALSE;
}

static bool DoGetInfo(MCB *mcb, SockEntry *s, int fn)
{
	word e = 0;
	SocketInfoReq *si = (SocketInfoReq *)mcb->Control;
	int level = (int)si->Level;
	int name  = (int)si->Option;
	byte *val = mcb->Data + sizeof(word);
	int valsize = IOCDataMax - sizeof(word);
	
	if( level == SOL_SYSTEM )
	{
		switch( name )
		{
		case SO_HOSTID:
			*(word *)val = MyAddr;
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
			e = syscall(s,getpeername,s,val,&valsize);
			goto done;
			
		case SO_SOCKNAME:
			e = syscall(s,getsockname,s,val,&valsize);
			goto done;
		}
	}
	
	/* To allow us to support ioctls which transfer data in both	*/
	/* directions, we allow GetInfo to supply some data in the	*/
	/* buffer.							*/
	if( si->Level == SOL_IOCTL ) 
	{
		e = syscall(s,ioctl,s,si->Option,val);
		valsize = (int)((si->Option>>16) & 0xff);
	}
	else e = syscall(s,getsockopt,s,si->Level,si->Option,val,&valsize);
	
done:
	if( e ) ErrorMsg(mcb,e<0?e:EC_Error|EG_Errno|e);
	else
	{
		InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,SS_InterNet);
		MarshalWord(mcb,level);
		MarshalWord(mcb,name);
		MarshalOffset(mcb);
		mcb->MsgHdr.DataSize = valsize + sizeof(word);
		*(word *)mcb->Data = valsize;
		PutMsg(mcb);
	}
	
	return FALSE;
}

#if 0
static Buffer *SupplyBuffer(word pos, Buffer *b) { return b; }
#endif
  
static void
sysreadwrite()
{
	struct a {
		int	s;
		int	rw;
		struct uio *uio;
	} *uap = (struct a *)u.u_ap;
	struct file *fp;
	int len = uap->uio->uio_resid;
	
	fp = (struct file *)getsock(uap->s);
	
	u.u_error = (*fp->f_ops->fo_rw)(fp,uap->rw,uap->uio);
	
	u.u_r.r_val1 = len - uap->uio->uio_resid;
}

static bool Do_Read(MCB *mcb, SockEntry *s)
{
	ReadWrite *rw = (ReadWrite *)mcb->Control;
	word size = rw->Size;
	byte *buf;
	byte *data = mcb->Data;
	word e;
	struct uio uio;
	struct iovec iov;

	/* IOdebug( "TCPIP: Do_Read: called" ); */
	
	if( s->Oob )
	{
		ErrorMsg(mcb,EC_Error|EG_Exception|EE_Signal|SIGURG);
		s->Oob = FALSE;
		return FALSE;
	}
		
	if( size > TCPDataMax ) size = TCPDataMax;
		
	buf = (char * ) MMalloc(size);

	if( buf == NULL )
	{
		ErrorMsg(mcb,EC_Error|EG_NoMemory|EO_Socket);
		return FALSE;
	}

	iov.iov_base   = buf;
	iov.iov_len    = (int)size;
	uio.uio_iov    = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_segflg = 0;
	uio.uio_offset = 0;
	uio.uio_resid  = (int)size;
	
	s->u.u_timeout = (int)(rw->Timeout/OneSec);
	
	/* IOdebug( "TCPIP: Do_Read: syscall" ); */
	
	e = syscall(s,sysreadwrite,s,UIO_READ,&uio);

	/* IOdebug( "TCPIP: Do_Read: return = %x", e ); */
	
	if( e == EINTR ) 
	{
		if( s->Oob ) ErrorMsg(mcb,EC_Error|EG_Exception|EE_Signal|SIGURG);
		else ErrorMsg(mcb,EC_Recover|EG_Timeout|EO_Socket);
		s->Oob = FALSE;
		goto done;
	}
	
	if( e == 0 && s->u.u_r.r_val1 == 0 )
	{
		InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,ReadRc_EOF);
		PutMsg(mcb);
		goto done;
	}
	
	if( e != 0 ) { ErrorMsg(mcb,EC_Error|EG_Errno|e); goto done;  }

	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,ReadRc_EOD);
	mcb->MsgHdr.DataSize = 	s->u.u_r.r_val1;
	mcb->Data = buf;		
	e = PutMsg(mcb);

done:
	FFree(buf);
	mcb->Data = data;
	
	/* IOdebug( "TCPIP: Do_Read: finished" ); */
	
	return FALSE;
}

static bool Do_Write(MCB *mcb, SockEntry *s)
{
	ReadWrite *rw = (ReadWrite *)mcb->Control;
	word size = rw->Size;
	struct uio uio;
	struct iovec iov;
	byte *buf, *bufp;
	byte *data = mcb->Data;
	word e;
	word confrc = WriteRc_Done;
	word idata = mcb->MsgHdr.DataSize;
	Port dataport = NullPort;
	Port replyport = mcb->MsgHdr.Reply;

#if 0			
	if( s->Oob )
	{
		ErrorMsg(mcb,EC_Error|EG_Exception|EE_Signal|SIGURG);
		s->Oob = FALSE;
		return FALSE;
	}
#endif		
	if( size > TCPDataMax ) size = TCPDataMax;

	bufp = buf = (char *) MMalloc(size);
	
	if( buf == NULL )
	{
		ErrorMsg(mcb,EC_Error|EG_NoMemory|EO_Socket);
		return FALSE;
	}

	/* handle any immediate data			*/
	if( idata )
	{
		memcpy(buf,data,(int)idata);
		if( idata == size ) goto dowrite;
		bufp += idata;	
	}

	dataport = NewPort();

	InitMCB(mcb,MsgHdr_Flags_preserve,replyport,dataport,WriteRc_Sizes);
	
	MarshalWord(mcb,size);
	MarshalWord(mcb,size);
	MarshalWord(mcb,size);

	e = PutMsg(mcb);

	mcb->MsgHdr.Dest = dataport;
	mcb->Data = bufp;
	if( e >= 0 ) e = GetMsg(mcb);

	if( mcb->MsgHdr.DataSize != size-idata )
	{
		e = EC_Error;
		goto confirm;
	}
	
dowrite:
	iov.iov_base   = buf;
	iov.iov_len    = (int)size;
	uio.uio_iov    = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_segflg = 0;
	uio.uio_offset = 0;
	uio.uio_resid  = (int)size;

	e = syscall(s,sysreadwrite,s,UIO_WRITE,&uio);

	if( e == EWOULDBLOCK && s->u.u_r.r_val1 == 0 )
		confrc = EC_Error|SS_InterNet|EG_Errno|e;

confirm:
	InitMCB(mcb,0,replyport,NullPort,confrc);
	
	MarshalWord(mcb,e<0?e:size);
	MarshalWord(mcb,s->u.u_r.r_val1);
	
	e = PutMsg(mcb);

	FFree(buf);	
	FreePort(dataport);		
	mcb->Data = data;
	return FALSE;
	
}

static bool DoGetSize(MCB *mcb, SockEntry *s)
{
	int size;
	word e;
	
	e = (word)syscall(s,ioctl,s,FIONREAD,&size);

	if( e != 0 ) { ErrorMsg(mcb,EC_Error|EG_Errno|e); return FALSE;  }
	
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,0);
	MarshalWord(mcb,size);
	PutMsg(mcb);
		
	return FALSE;
}

static bool DoShutdown(MCB *mcb, SockEntry *s, int fn)
{
	word e;
	word how = 0;
	
	if( fn & O_ReadOnly ) how |= FREAD;
	if( fn & O_WriteOnly ) how |= FWRITE;
	
	e = syscall(s,shutdown,s,how);
	
	ErrorMsg(mcb,e?EC_Error|EG_Errno|e:0);
	
	return FALSE;
}

void sysclose()
{
	struct a {
		int	s;
	}*uap = (struct a *)u.u_ap;
	struct file *fp = (struct file *)getsock(uap->s);
	
	if( fp == NULL ) return;

	u.u_error = soclose((struct socket *)fp->f_data);
}

static void DoClose(SockEntry *s)
{
  syscall( s, sysclose, s );
  
  Unlink(&s->ObjNode,FALSE);

  FreePort(s->Server);

  FFree(s);

  return;
}

static void SocketServer(SockEntry *s)
{
	MMsgBuf *	m = NULL;
	MCB *		mcb;
	bool		bufkept = TRUE;

	forever
	{
		word e;
		

		if( bufkept ) m = NewMMsgBuf(), bufkept = FALSE;

		mcb = &m->mcb;
		mcb->MsgHdr.Dest = s->Server;
		mcb->Timeout = OneSec*5;

		e = GetMsg(mcb);

		if( e < Err_Null ) continue;

		mcb->MsgHdr.FnRc = SS_InterNet;

		Wait(&s->ObjNode.Lock);

		switch( e & FG_Mask )
		{
		case FG_Connect:	bufkept = DoConnect(mcb,s);	break; 
		case FG_Listen:		bufkept = DoListen(mcb,s);	break;
		case FG_Accept:		bufkept = DoAccept(mcb,s);	break;
		case FG_Select:		bufkept = DoSelect(mcb,s,(int)e);	break;
		case FG_SendMessage:	bufkept = DoSendMessage(mcb,s);	break;
		case FG_RecvMessage:	bufkept = DoRecvMessage(mcb,s);	break;
		case FG_GetInfo:	bufkept = DoGetInfo(mcb,s,(int)e);	break;
		case FG_SetInfo:	bufkept = DoSetInfo(mcb,s,(int)e);	break;
		case FG_Read:		bufkept = Do_Read(mcb,s);	break;
		case FG_Write:		bufkept = Do_Write(mcb,s);	break;
		case FG_GetSize:	bufkept = DoGetSize(mcb,s);	break;
		
		case FG_Close:
			if( e & FF_Mask ) bufkept = DoShutdown(mcb,s,(int)e);
			else
			{
				abortsyscalls(s,FALSE);
				s->Users--;
				if( s->Users == 0 ) goto done;
			}
			break;


		default:
			ErrorMsg(mcb,EC_Error|EG_WrongFn|EO_Socket);
			break;
		}

		Signal(&s->ObjNode.Lock);
	}

done:

	DoClose(s);
	if( !bufkept ) FFree(m);
	MemAlloced -= StackSize;
}

static word DoBind(ServInfo *servinfo) 
{
	SockEntry *s;
	SockEntry *d;
	MCB *m = servinfo->m;
	IOCBind *bindreq = (IOCBind *)m->Control;
	MsgBuf *r;
	char *dirname;
	char *pathname = servinfo->Pathname;
	struct sockaddr_in *addr;
	char sockname[NameMax];
	
	
	if( (r = New(MsgBuf)) == NULL )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		
		return TRUE;		
	}
	
	s = (SockEntry *)GetTarget(servinfo);

	if( s != (SockEntry *)&Root )
	{
		word e;

		/* This is possibly a bind for a socket which we have only */
		/* created previously. Let TCPIP sort out whether we can   */
		/* do this.						   */
		
		if( bindreq->Addr == -1 )
		{
			ErrorMsg(m,EC_Error|EG_WrongFn|EO_Socket);
			goto bad;
		}
		d = s;
		addr = (struct sockaddr_in *)&m->Data[bindreq->Addr+4];
		e = syscall(d,bind,d,addr,sizeof(struct sockaddr_in));

		if( e )
		{
			ErrorMsg(m,EC_Error|EG_Errno|e);
			goto bad;
		}
		d->Bound = TRUE;		
		goto sendreply;
	}

	switch( bindreq->Protocol & 0xf )
	{
	case SOCK_RAW: 		dirname = "raw"; break;
	case SOCK_STREAM: 	dirname = "tcp"; break;
	default:
	case SOCK_DGRAM: 	dirname = "udp"; break;
	}
	
	pathcat(pathname,dirname);
	
	d = (SockEntry *)Lookup((DirNode *)s,dirname,TRUE);
	
	Wait(&d->ObjNode.Lock);
	
	Signal(&s->ObjNode.Lock);
	
	servinfo->Target = (ObjNode *)(s = d);

	if( bindreq->Addr == -1 ) addr = NULL;
	else addr = (struct sockaddr_in *)&m->Data[bindreq->Addr+4];
	
	sockname[0] = 0;
#if 1
	strcpy(sockname,"socket.");
	addint(sockname,SockId++);
	if( addr && addr->sin_port != 0 ) 
	{
		strcat(sockname,".");
		addint(sockname,ntohs(addr->sin_port));
	}
#else
	if( addr && addr->sin_port != 0 ) addint(sockname,ntohs(addr->sin_port));
	else 
	{
		/* if no bind addr given, make up a name */
		strcpy(sockname,"anon.");
		addint(sockname,SockId++);
	}
#endif
	
	UnLockTarget(servinfo);

	d = (SockEntry *)Lookup((DirNode *)s,sockname,FALSE);

	if( d == NULL )
	{
		word e;

		/* @@@@ check access rights */
		
		d = NewSocket((DirNode *)s,sockname,(int)bindreq->Protocol);

		if( d == NULL ) goto nomem;

		e = syscall( d, socket, AF_INET, d->Protocol  & 0xf, (d->Protocol >> 8));

		if ( e )
		  {
		    ErrorMsg( m, EC_Error | EG_Errno | e);
		    goto killsocket;
		  }

		if( addr ) 
		  {
			e = syscall(d,bind,d,addr,sizeof(struct sockaddr_in));
		    
			if( e ) 
			{
				ErrorMsg(m,EC_Error|EG_Errno|e);
				goto killsocket;
			}
		}

		/* If no name was supplied, change name now to port	*/
		/* number chosen by protocol.				*/
		if( !addr || addr->sin_port==0  )
		{
			struct sockaddr_in a;
			int alen = sizeof(a);
			
			e = syscall(d,getsockname,d,&a,&alen);

			if( e == 0 && a.sin_port != 0 )
			{
#if 1
				strcat(d->ObjNode.Name,".");
				addint(d->ObjNode.Name,ntohs(a.sin_port));
#else
				sockname[0] = 0;
				addint(sockname,ntohs(a.sin_port));
				strcpy(d->ObjNode.Name,sockname);
#endif
			}
		}
		
		if( !Fork(StackSize,SocketServer,sizeof(d),d) )
		{
		nomem:
			ErrorMsg(m,EC_Error|EG_NoMemory|EO_Socket);
			goto killsocket;
		}
		else MemAlloced += StackSize;
	}
	else
	{
		/* else connect user to existing socket */
		d->Users++;
	}

	pathcat(pathname,d->ObjNode.Name);

sendreply:
	FormOpenReply(r,m,&d->ObjNode, Flags_Closeable|Flags_Selectable, pathname);
	r->mcb.MsgHdr.Reply = d->Server;
	PutMsg(&r->mcb);
bad:
	FFree(r);
	
	return TRUE;
	
killsocket:
	FFree(r);
	
	DoClose(d);

	return 0;
}

static void DoOpen(ServInfo *servinfo)
{
	SockEntry *s;
	MCB *m = servinfo->m;
	char *pathname = servinfo->Pathname;
	MsgBuf *r;
	Port reqport;

	s = (SockEntry *)GetTarget(servinfo);

	if( s == NULL )
	{
		ErrorMsg(m,EC_Error|EG_WrongFn|EO_Socket);
		return;
	}

	r = New(MsgBuf);
	if( r == NULL )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory);
		return;		
	}
	
	/* @@@@ check access rights !! */
	
	if( s->ObjNode.Type == Type_Directory )
	{
		reqport = NewPort();
		FormOpenReply(r,m,&s->ObjNode, Flags_Closeable, pathname);
		r->mcb.MsgHdr.Reply = reqport;
		PutMsg(&r->mcb);
		FFree(r);
		DirServer(servinfo, m, reqport);
		FreePort(reqport);
#ifdef MBUF_INFO
mbuf_info();
#endif /* MBUF_INFO */
	}
	else
	{
		reqport = s->Server;
		FormOpenReply(r,m,&s->ObjNode, Flags_Closeable|Flags_Selectable, pathname);
		r->mcb.MsgHdr.Reply = reqport;
		PutMsg(&r->mcb);
		FFree(r);
		s->Users++;
	}
}

static word so_private(ServInfo *servinfo)
{
	MCB *mcb = servinfo->m;

	if( (servinfo->FnCode & FG_Mask) == FG_Bind ) return DoBind(servinfo);
	else ErrorMsg(mcb,EC_Error|SS_InterNet|EG_WrongFn|EO_Socket);
	
	return FALSE;
}

static DispatchInfo dinfo = 
{
	NULL,
	NullPort,
	SS_InterNet,
	NULL,
	{ (VoidFnPtr)so_private,10000 },
	{
		{ DoOpen,	3000 },
		{ InvalidFn,	3000 },
		{ DoLocate,	3000 },
		{ DoObjInfo,	3000 },
		{ InvalidFn,	3000 },
		{ InvalidFn,	3000 },
		{ InvalidFn,	3000 },
		{ DoLink,	3000 },
		{ DoProtect,	3000 },
		{ DoSetDate,	3000 },
		{ DoRefine,	3000 },
		{ NullFn,	3000 },
		{ DoRevoke,	3000 },
		{ InvalidFn,	3000 },
		{ InvalidFn,	3000 }
	}
};


void InitDirTree()
{	
	InitNode((ObjNode *)&Root,"internet", Type_Directory, 0, DefRootMatrix );
	InitList(&Root.Entries);
	Root.Nentries = 0;
	
	TcpNode = (DirNode *)MMalloc(sizeof(DirNode));
	InitNode((ObjNode *)TcpNode,"tcp", Type_Directory, 0, DefDirMatrix );
	InitList(&TcpNode->Entries);
	TcpNode->Nentries = 0;
	Insert(&Root, (ObjNode *)TcpNode, FALSE );

	UdpNode = (DirNode *)MMalloc(sizeof(DirNode));
	InitNode((ObjNode *)UdpNode,"udp", Type_Directory, 0, DefDirMatrix );
	InitList(&UdpNode->Entries);
	UdpNode->Nentries = 0;
	Insert(&Root, (ObjNode *)UdpNode, FALSE );

	RawNode = (DirNode *)MMalloc(sizeof(DirNode));
	InitNode((ObjNode *)RawNode,"raw", Type_Directory, 0, DefDirMatrix );
	InitList(&RawNode->Entries);
	RawNode->Nentries = 0;
	Insert(&Root, (ObjNode *)RawNode, FALSE );
}

void AddName()
{
	Object *o;
	char mcname[100];
	NameInfo info;
	
	MachineName(mcname);
	o = Locate(NULL,mcname);
	
	info.Port = dinfo.ReqPort;
	info.Flags = Flags_StripName;
	info.Matrix = DefNameMatrix;
	info.LoadData = NULL;
	
	RootObj = Create(o, "internet", Type_Name, sizeof(NameInfo), (byte *)&info);

	Close(o);
	
	if( RootObj == NULL )
	{
		IOdebug( "tcpip: failed to create name server entry" );
		Exit(1);
	}
}

void CleanUp(void)
{
	if (RootObj != (Object *) NULL)
		Delete(RootObj, NULL );
}

/*
-- crf: 17/08/92 - Bugs 956, 999, 1002
-- Attempt to exit cleanly (after UNIX panic)
*/
void clean_exit ()
{
	CleanUp() ;
	Exit(1) ;
}

Environ env;

static struct{
	NetDevInfo	ndi;
	char		dev[32];
} DfltNetDevInfo =
{
	{ 24, 0, 0, 0, 0,0,0,0,0,0,0,0 },
	"ether.d"
};

static void pstr(char *msg)
{
	Write(env.Strv[1],msg,strlen(msg),-1);
}

int main(void)
{
	struct ifreq ifr;
	struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;
	int argc;
	char **argv;
	char *netdev = "ether";
#ifdef NOT_USED_SO_WHY_ARE_THEY_HERE
	ipprintfs  = 0;
	tcpconsdebug = 1;
	tcpprintfs = 1;
#endif

	GetEnv(MyTask->Port,&env);
	
/*
-- crf: 13/08/92
-- Currently, if there is already a tcpip server running, attempting to
-- install tcpip on the *same* processor will fail. However, it is possible
-- to load tcpip on a different processor. This, as I have discovered to my
-- cost, can cause a great deal of confusion.
*/
	if (Locate ((Object *) NULL, "/internet") != (Object *) NULL)
	{
		IOdebug ("tcpip: /internet already exists") ;
		Exit (1) ;
	}

	argv = env.Argv;
	
	for( argc = 0; argv[argc]; argc++);

	if( argc < 3 )
	{
pstr("usage: tcpip <hostname> <hostaddr> [-s <subnetmask>] [-e <devicename>]\r\n");
pstr("       <hostname>     name of this machine\r\n");
pstr("       <hostaddr>     address in dot format e.g. 12.34.56.78\r\n");
pstr("       <subnetmask>   subnet mask in dot format\r\n");
pstr("       <devicename>   name of device in /helios/etc/devinfo file\n");
		Exit(1<<8);
	}
	
	MyName = argv[1];
	MyAddr = inet_addr(argv[2]);
	SubNetMask = 0;
			
	argv = argv+3;
	
	while( *argv )
	{
		char *arg = *argv++;
		if( *arg == '-' )
		{
			arg++;
			switch( *arg++ )
			{
			case 's':
				if( *arg==0 ) arg = *argv++;
				SubNetMask = inet_addr(arg);
				break;

			case 'e':
				if( *arg==0 ) arg = *argv++;
				netdev = arg;
				break;

			default:
				pstr("unknown option: ");
				pstr(argv[-1]);
				break;
			}
		}
		else
		{
			pstr("unexpected argument: ");
			pstr(arg);
			pstr("\r\n");
		}		
	}

	if ((EtherInfo = getdevinfo(netdev)) == NULL )
	{
		EtherInfo = &DfltNetDevInfo.ndi;
	}

	init_unix();

	Fork( StackSize, sleep_timer, 0);

	mbinit();
	
	loattach();

	etattach();
	
	domaininit();

	raw_init();
		
	if( SubNetMask ) 
	{
		addr->sin_addr.s_addr = SubNetMask;
		in_control(NULL, SIOCSIFNETMASK, &ifr, &ec_softc);
	}
		
	/* the following is equivalent to a sethostid() call 	*/
	/* fix this to get name from hosts database		*/
	
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = MyAddr;
	
	in_control(NULL, SIOCSIFADDR, &ifr, &ec_softc);
	
	addr->sin_addr.s_addr = inet_addr("127.0.0.1");
	in_control(NULL, SIOCSIFADDR, &ifr, &loif);
	
	dinfo.ReqPort = NewPort();	
	dinfo.Root = &Root;

	InitDirTree();

	AddName();	

	Dispatch(&dinfo);

	CleanUp();
	
	return 0;	
}


/* The following come from the C and Posix libraries */

extern u_long swap_long(u_long a)
{	
	unsigned long b;
	((char *)&b)[0] = ((char *)&a)[3];
	((char *)&b)[1] = ((char *)&a)[2];
	((char *)&b)[2] = ((char *)&a)[1];
	((char *)&b)[3] = ((char *)&a)[0];
	return b;
}

extern u_short swap_short(u_short a)
{	
	unsigned long b = 0;

	((char *)&b)[0] = ((char *)&a)[1];
	((char *)&b)[1] = ((char *)&a)[0];
	return (u_short)b;
}


int memcmp(const void *a, const void *b, size_t n)
{   const unsigned char * ac = (const unsigned char *) a;
    const unsigned char * bc = (const unsigned char *) b;
    if ((((int)ac | (int)bc) & 3) == 0)
    {   while (n >= 4 && *(int *)ac == *(int *)bc)
            ac += 4, bc += 4, n -= 4;
    }
    while (n-- > 0)
    {   unsigned char c1,c2;   /* unsigned cmp seems more intuitive */
        if ((c1 = *ac++) != (c2 = *bc++)) return c1 - c2;
    }
    return 0;
}


long int strtol(const char *nptr, char **endptr, int base)
{
	long int val = 0;	
	for(;;)
	{
		char c = *nptr;
		if( '0' <= c && c <= '9' ) val = val*base + c - '0';
		else if( base == 16 )
		{
			if( 'A' <= c && c <= 'F' ) val = val*base + c - 'A' + 10;
			elif( 'a' <= c && c <= 'f' ) val = val*base + c - 'a' + 10;			
			else break;
		}
		else break;
		nptr++;
	}
	*endptr = (char *)nptr;
	return val;
}

#if 0
void ether_addr(u_char *addr, char *str)
{
    int    i;
    long   x;

    for (i = 0; i < 6; i++)
    {
        if ((x = strtol(str, &str, 10)) > 255) return;
        addr[i] = x;
        if ((i < 5) && (*str++ != '.')) return;
    }
}
#endif

static void *load_devinfo(void)
{
	Stream *s = NULL;
	Object *o;
	void *devinfo = NULL;
	int size;
	ImageHdr hdr;

	o = Locate(NULL,"/rom/devinfo");

	if( o == NULL ) o = Locate(NULL,"/loader/DevInfo");

	if( o == NULL ) o = Locate(NULL,"/helios/etc/devinfo");

	if( o == NULL )
	  {
	    return NULL;
	  }

	s = Open(o,NULL,O_ReadOnly);

	if( s == NULL )
	  { 
		Close(o);
		return NULL;
	}

	if(Read(s,(byte *)&hdr,sizeof(hdr),-1) != sizeof(hdr) )
	  {	    
	    goto done;
	  }	
		
	if ( hdr.Magic != Image_Magic )
	  {
	    goto done;
	  }	
	
	size = (int)hdr.Size;
	
	devinfo = Malloc(size);
	
	if( devinfo == NULL )
	  {
	    goto done;
	  }
		
	if (Read( s, (char *) devinfo, size, -1) != size ) 
	{ 
		Free(devinfo);
		devinfo = NULL;
	}
	
done:
	Close(s);
	Close(o);

	return devinfo;
}

static InfoNode *find_info(void *devinfo, word type, char *name)
{
	InfoNode *info = (InfoNode *)((Module *)devinfo + 1);

	forever
	{
		if( strcmp(name,RTOA(info->Name))==0 &&
		    info->Type == type ) return info;
		    
		if( info->Next == 0 ) break;
		info = (InfoNode *)RTOA(info->Next);
	}
	return NULL;
}

static NetDevInfo *getdevinfo(char *name)
{
	void *devinfo = load_devinfo();
	InfoNode *i;
	
	if ( devinfo == NULL )
	  {
	    IOdebug( "TCP/IP: corrupt or missing devinfo file - using default driver: ether.d" );
	    
	    return NULL;
	  }
		
	i = find_info(devinfo,Info_Net,name);
	
	if( i == NULL ) return NULL;
	
	return (NetDevInfo *)(RTOA(i->Info));
}

#ifdef __TRAN
#pragma -s1

void _stack_error(Proc *p)
{
	IOdebug("TCP/IP stack overflow in %s at %x",p->Name,&p);
}
#endif
