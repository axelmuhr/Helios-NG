#include <helios.h>
#include <syslib.h>
#include <servlib.h>
#include <codes.h>
#include <gsp.h>

#include "param.h"

#define __IN_SERVER__
#include <sys/socket.h>

#include "mbuf.h"
#include "socketvar.h"
#include "file.h"
#include "netinet/in.h"
#include "proc.h"
#include "user.h"
#include "../net/netisr.h"
#include "../net/if.h"
#include "ioctl.h"

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

DirNode		*TcpNode;		/* sub-root of tcp sockets	*/

DirNode		*UdpNode;		/* sub-root of udp sockets	*/

DirNode		*RawNode;		/* sub-root of raw sockets	*/

Object		*RootObj;		/* root name-table object	*/

int		MyAddr;			/* my internet address		*/

char		*MyName;		/* my machine name		*/

int		SockId = 1;		/* seed for nonce ids		*/

#define DefSockMatrix	DefFileMatrix

#define StackSize	10000

#define TCPDataMax	10000

static void SocketServer(SockEntry *s);

extern void socket();
extern void listen();
extern void accept();
extern void connect();
extern void bind();
extern void sendmsg();
extern void recvmsg();
extern void getsockopt();
extern void setsockopt();
extern void getsockname();
extern void getpeername();
extern void shutdown();
extern unsigned short swap_short(unsigned short a,unsigned long b);

extern int MemAlloced;
#ifndef MMalloc
extern void *MMalloc(int size);
extern void FFree(void *v);
#endif

/* Unix compatability */

extern Semaphore kernel;

procname(void (*fn)())
{
	word *x = (word *)fn;
	while( (*x & T_Mask) != T_Valid ) x--;
	
	switch( *x )
	{
	case T_Proc:
		return ((Proc *)x)->Name;
	case T_Module:
	case T_Program:
		return ((Module *)x)->Name;
	}
}

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
IOdebug("syscall <%s> %s(%x %x %x)",s->ObjNode.Name,procname(fn),
		((int *)ap)[0],((int *)ap)[1],((int *)ap)[2]);
#endif
	s->u.u_ap = ap;
	
	if( s->SysCall != NULL ) 
	{
		IOdebug("SysCall %s: call %s already in progress!!",procname(fn),procname(s->SysCall));
		return EALREADY;
	}
	
	s->SysCall = fn;
	
	tokernel();

	u = s->u;
	u.u_error = 0;

	if( setjmp(u.u_qsave) == 0 ) (*fn)();
	else if( u.u_error == 0 ) u.u_error = EINTR;

	s->u = u;
	s->SysCall = NULL;
	s->u.u_timeout = 0;

	fromkernel();		
#if SHOWCALLS
IOdebug("syscall <%s> %s done error %d",s->ObjNode.Name,procname(fn),s->u.u_error);
#endif
	return s->u.u_error;
}

static int do_abort(struct sleeper *s, SockEntry *se)
{
	if( s->sock == se )
	{
IOdebug("abort syscall %s for %s",procname(se->SysCall),se->ObjNode.Name);
		s->timeout = TRUE;
		Remove(&s->node);
		Signal(&s->wait);
	}
}

void abortsyscalls(SockEntry *s)
{
	if( s->SysCall == NULL ) return;
	
	tokernel();
	WalkList(&sleepq,do_abort,s);
	fromkernel();
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
	else s.endtime = curtime+u.u_timeout;
	
	s.timeout = FALSE;
	InitSemaphore(&s.wait,0);

	AddTail(&sleepq,&s.node);

	se->u = u;
		
	fromkernel();
	Signal(&se->ObjNode.Lock);

	Wait(&s.wait);
	
	Wait(&se->ObjNode.Lock);
	tokernel();

	u = se->u;

	/* if we were woken up by a timeout we simulate having been zapped */
	/* by a signal							   */	
	if( s.timeout ) longjmp(u.u_qsave,1);
}

static int do_wakeup(struct sleeper *s, caddr_t chan)
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

static int do_timeout(struct sleeper *s)
{
	if( (s->endtime > 0) && (s->pri > PZERO) && (s->endtime <= curtime) )
	{
		s->timeout = TRUE;
		Remove(&s->node);
		Signal(&s->wait);
	}
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
	SockEntry *s = MMalloc(sizeof(SockEntry));
	
	if( s == NULL ) return NULL;
	
	memset(s,0,sizeof(SockEntry));
	
	InitNode(&s->ObjNode,name,Type_Socket,0,DefSockMatrix);

	/* init Helios level fields */
	
	s->Protocol = proto;
	s->Users = 1;
	s->Server = NewPort();
	s->Bound = FALSE;
	
	s->SelectPort = NullPort;
			
	/* the file structure will be filled in by socket or accept */
	
	/* the user structure contains just a few useful things	*/
	
	s->u.u_procp = (caddr_t)&s->proc;
	s->u.u_timeout = 0;
		
	/* proc structure is vestigial */
	
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
		SockEntry *new;
		char newname[NameMax];

		strcpy(newname,s->ObjNode.Name);
		strcat(newname,".");
		addint(newname,SockId++);

		new = NewSocket(s->ObjNode.Parent,newname,s->Protocol);
		
		if( new == NULL )
		{
			u.u_r.r_val1 = 0;
			return NULL;
		}
		else
		{
			u.u_r.r_val1 = (int)new;
			return &new->File;
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
		MarshalData(mcb,size,data);
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
		msg->msg_name = (caddr_t)(data+dg->DestAddr+sizeof(word));
		msg->msg_namelen = *(word *)(data+dg->DestAddr);
	}
	else msg->msg_name = NULL, msg->msg_namelen = 0;
	
	msg->msg_iov->iov_base = data+dg->Data;
	msg->msg_iov->iov_len = dg->DataSize;
	msg->msg_iovlen = 1;
	
	if( dg->AccRights != -1 )
	{
		msg->msg_accrights = (caddr_t)(data+dg->AccRights+sizeof(word));
		msg->msg_accrightslen = *(word *)(data+dg->AccRights);
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
	SockEntry *new;

	s->u.u_timeout = 10;
	e = syscall(s,accept,s,&addr,&addrlen);
	
	if( e == EINTR ) { ErrorMsg(mcb,EC_Recover|EG_Timeout|EO_Socket); return FALSE; }
	
	if( e ) goto errormsg;

	new = (SockEntry *)s->u.u_r.r_val1;

	/* fork a new server */
		
	if( !Fork(StackSize,SocketServer,sizeof(new),new) )
	{ e = ENOMEM; goto errormsg; }
	else MemAlloced += StackSize;
	
	/* now reply to the client */

	{
		Capability cap;
		char *p;
		
		InitMCB(mcb,0,reply,new->Server,SS_InterNet);
	
		MarshalWord(mcb,new->ObjNode.Type);
		MarshalWord(mcb,new->ObjNode.Flags|Flags_Server|Flags_Selectable|Flags_Closeable);
		NewCap(&cap,&new->ObjNode,AccMask_R|AccMask_W);
		MarshalCap(mcb,&cap);
		MarshalOffset(mcb);
	
		p = mcb->Data;
		MachineName(p);
		pathcat(p,Root.Name);
		pathcat(p,TcpNode->Name);	/* only TCP call this!! */
		pathcat(p,new->ObjNode.Name);

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

static void sysselect()
{
	struct a {
		int	s;
		int	which;
	} *uap = (struct a *)u.u_ap;
	struct file *fp;
	int ok;
	
	fp = getsock(uap->s);

	u.u_error = (*fp->f_ops->fo_select)(fp,uap->which);
}

void selwakeup(struct proc *p, int flags) 
{
	SockEntry *s = (SockEntry *)p->p_sock;
	MCB mcb;

	/* Not locking the socket here is dodgy, but there is a		*/
	/* potential deadlock if we do. The worst that can happen is 	*/
	/* for this routine to zap a port just installed by DoSelect	*/
	/* hence the do loop in that routine.				*/
	if( s->SelectPort != NullPort )
	{
		struct file *fp = &s->File;
		int fn = s->SelectMode;
		int result = 0;

		if( (fn & O_ReadOnly ) && (*fp->f_ops->fo_select)(fp,FREAD)) result |= O_ReadOnly;
		if( (fn & O_WriteOnly ) && (*fp->f_ops->fo_select)(fp,FWRITE)) result |= O_WriteOnly;
		if( (fn & O_Exception ) && (*fp->f_ops->fo_select)(fp,0)) result |= O_Exception;
		InitMCB(&mcb,0,s->SelectPort,NullPort,result);
		PutMsg(&mcb);
		s->SelectPort = NullPort;
	}
}

static bool DoSelect(MCB *mcb, SockEntry *s, int fn)
{
	int result = 0;

	fn &= FF_Mask;

	if( (fn & O_ReadOnly)  && syscall(s,sysselect,s,FREAD) ) result |= O_ReadOnly;
	if( (fn & O_WriteOnly) && syscall(s,sysselect,s,FWRITE) ) result |= O_WriteOnly;	
	if( (fn & O_Exception) && syscall(s,sysselect,s,0)) result |= O_Exception;
	if( result ) ErrorMsg(mcb,result);
	else
	{
		do 
		{ 
			FreePort(s->SelectPort);
			s->SelectPort = mcb->MsgHdr.Reply;
		} while( s->SelectPort == NullPort );
		s->SelectMode = fn & FF_Mask;
	}
	
	return FALSE;
}

static bool DoSendMessage(MCB *mcb, SockEntry *s)
{
	struct msghdr msg;
	struct iovec iov;
	DataGram *dg = (DataGram *)mcb->Control;
	Port dataport = NewPort();
	byte *buf = MMalloc(dg->DataSize+mcb->MsgHdr.DataSize);
	byte *data = mcb->Data;
	word e;

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

	s->u.u_timeout = dg->Timeout==-1?0:dg->Timeout/OneSec;
	
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
	int timeout = dg->Timeout;

	buf = MMalloc(dg->DataSize+sizeof(addr)+sizeof(word));
	
	iov.iov_base = buf;
	iov.iov_len = dg->DataSize;
	msg.msg_name = (caddr_t)&addr;
	msg.msg_namelen = sizeof(addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_accrights = NULL;	/* ignore accrights for now */
	msg.msg_accrightslen = 0;
	
	if( timeout == -1 ) timeout = 0;
	else timeout /= OneSec;
	
	s->u.u_timeout = timeout;
	e = syscall(s,recvmsg,s,&msg,dg->Flags);

	if( e == EINTR )
	{
		if( timeout ) e |= EC_Error|EG_Errno;
		else e = EC_Recover|EG_Timeout|EO_Socket;
		
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
	
	fp = getsock(uap->fd);
	
	u.u_error = (*fp->f_ops->fo_ioctl)(fp,uap->cmd,uap->data);
}

static bool DoSetInfo(MCB *mcb, SockEntry *s, int fn)
{
	word e;
	SocketInfoReq *si = (SocketInfoReq *)mcb->Control;
	caddr_t val = NULL;
	word valsize = 0;

	if( si->Optval != -1 )
	{
		val = mcb->Data+si->Optval+sizeof(word);
		valsize = *(word *)(mcb->Data+si->Optval);
	}

	if( si->Level == SOL_IOCTL ) e = syscall(s,ioctl,s,si->Option,val);
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
	int level = si->Level;
	int name = si->Option;
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
		valsize = (si->Option>>16) & 0xff;
	}
	else e = syscall(s,getsockopt,s,si->Level,si->Option,val,valsize);
	
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

static Buffer *SupplyBuffer(word pos, Buffer *b) { return b; }

static sysreadwrite()
{
	struct a {
		int	s;
		int	rw;
		struct uio *uio;
	} *uap = (struct a *)u.u_ap;
	struct file *fp;
	int len = uap->uio->uio_resid;
	
	fp = getsock(uap->s);
	
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
	
	if( size > TCPDataMax ) size = TCPDataMax;
		
	buf = MMalloc(size);

	if( buf == NULL )
	{
		ErrorMsg(mcb,EC_Error|EG_NoMemory|EO_Socket);
		return FALSE;
	}

	iov.iov_base = buf;
	iov.iov_len = size;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_segflg = 0;
	uio.uio_offset = 0;
	uio.uio_resid = size;
	
	s->u.u_timeout = rw->Timeout/OneSec;
	e = syscall(s,sysreadwrite,s,UIO_READ,&uio);

	if( e == EINTR ) { ErrorMsg(mcb,EC_Recover|EG_Timeout|EO_Socket); goto done; }
	
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
	return FALSE;
}

static bool Do_Write(MCB *mcb, SockEntry *s)
{
	ReadWrite *rw = (ReadWrite *)mcb->Control;
	word size = rw->Size;
	struct uio uio;
	struct iovec iov;
	Buffer b;
	byte *buf;
	word e;
			
	buf = MMalloc(size);
	
	if( buf == NULL )
	{
		ErrorMsg(mcb,EC_Error|EG_NoMemory|EO_Socket);
		return FALSE;
	}

	b.Pos = rw->Pos;
	b.Size = 0;
	b.Max = size;
	b.Data = buf;
	
	DoWrite(mcb, SupplyBuffer, &b);

	if( b.Size == 0 ) goto done;

	iov.iov_base = buf;
	iov.iov_len = b.Size;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_segflg = 0;
	uio.uio_offset = 0;
	uio.uio_resid = b.Size;
		
	s->u.u_timeout = rw->Timeout/OneSec;
	e = syscall(s,sysreadwrite,s,UIO_WRITE,&uio);

done:
	FFree(buf);			
	return FALSE;
	
}

static bool DoGetSize(MCB *mcb, SockEntry *s)
{
	int size;
	int e;
	
	e = syscall(s,ioctl,s,FIONREAD,&size);

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
	struct file *fp = getsock(uap->s);
	if( fp == 0 ) return;
	u.u_error = soclose((struct socket *)fp->f_data);
}

static void DoClose(SockEntry *s)
{	
	syscall(s,sysclose,s);
	Unlink(&s->ObjNode,FALSE);
	FreePort(s->Server);
	FFree(s);
}

static void SocketServer(SockEntry *s)
{
	MMsgBuf *m;
	MCB *mcb;
	bool bufkept = TRUE;

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
		case FG_Select:		bufkept = DoSelect(mcb,s,e);	break;
		case FG_SendMessage:	bufkept = DoSendMessage(mcb,s);	break;
		case FG_RecvMessage:	bufkept = DoRecvMessage(mcb,s);	break;
		case FG_GetInfo:	bufkept = DoGetInfo(mcb,s,e);	break;
		case FG_SetInfo:	bufkept = DoSetInfo(mcb,s,e);	break;
		case FG_Read:		bufkept = Do_Read(mcb,s);	break;
		case FG_Write:		bufkept = Do_Write(mcb,s);	break;
		case FG_GetSize:	bufkept = DoGetSize(mcb,s);	break;
		
		case FG_Close:
			if( e & FF_Mask ) bufkept = DoShutdown(mcb,s,e);
			else
			{
				abortsyscalls(s);
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
	MemAlloced -= StackSize;
}

static word DoBind(ServInfo *servinfo) 
{
	SockEntry *s;
	SockEntry *d;
	MCB *m = servinfo->m;
	IOCBind *bindreq = (IOCBind *)m->Control;
	MsgBuf *r;
	char *name, *dirname;
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
		addint(sockname,swap_short(addr->sin_port,0));
	}
#else
	if( addr && addr->sin_port != 0 ) addint(sockname,swap_short(addr->sin_port,0));
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
		
		d = NewSocket(s,sockname,bindreq->Protocol);

		if( d == NULL ) goto nomem;

		e = syscall(d,socket,AF_INET,d->Protocol&0xf,(d->Protocol>>8));

		if( e )	{ ErrorMsg(m,EC_Error|EG_Errno|e); goto killsocket;}

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
				addint(d->ObjNode.Name,swap_short(a.sin_port,0));
#else
				sockname[0] = 0;
				addint(sockname,swap_short(a.sin_port,0));
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
		{ DoOpen,	1200 },
		{ InvalidFn,	1200 },
		{ DoLocate,	1200 },
		{ DoObjInfo,	1200 },
		{ InvalidFn,	1200 },
		{ InvalidFn,	1200 },
		{ InvalidFn,	1200 },
		{ DoLink,	1200 },
		{ DoProtect,	1200 },
		{ DoSetDate,	1200 },
		{ DoRefine,	1200 },
		{ NullFn,	1200 },
		{ DoRevoke,	1200 },
		{ InvalidFn,	1200 },
		{ InvalidFn,	1200 }
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
		IOdebug("failed to create name server entry");
		Exit(1);
	}
}

void CleanUp(void)
{
	Delete(RootObj, NULL );
}

int main(void)
{
	struct ifreq ifr;
	struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;
	Environ env;
	int argc;
	char **argv;

	ipprintfs  = 0;
	tcpconsdebug = 1;
	tcpprintfs = 1;
	
	GetEnv(MyTask->Port,&env);

	argv = env.Argv;
	for( argc = 0; argv[argc]; argc++);

	if( argc != 3 )
	{
		char *msg = "usage: tcpip <hostname> <hostaddr>\r\n";
		Write(env.Strv[1],msg,strlen(msg),-1);
		Exit(1<<8);
	}
	
	MyName = argv[1];
	MyAddr = inet_addr(argv[2]);

#if 0
	MyName = "nick";
	MyAddr = 0x69000059;
#endif	

	init_unix();


	Fork(StackSize,sleep_timer);

	mbinit();
	
	loattach();

	etattach();
	
	domaininit();

	raw_init();
		
	/* the following is equivalent to a sethostid() call 	*/
	/* fix this to get name from hosts database		*/
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = MyAddr;
	in_control(NULL, SIOCSIFADDR, &ifr, &ec_softc);
	
	dinfo.ReqPort = NewPort();	
	dinfo.Root = &Root;

	InitDirTree();

	AddName();	

	Dispatch(&dinfo);

	CleanUp();
	
	return 0;	
}


/* The following come from the C and Posix libraries */

/* NOTE: swap_short & swap_long rely on the calling conventions of 	*/
/* Tranny C, specifically that there will be an unused word after the	*/
/* argument in a single-argument function.				*/
extern unsigned long swap_long(unsigned long a,unsigned long b)
{	
	((char *)&b)[0] = ((char *)&a)[3];
	((char *)&b)[1] = ((char *)&a)[2];
	((char *)&b)[2] = ((char *)&a)[1];
	((char *)&b)[3] = ((char *)&a)[0];
	return b;
}

extern unsigned short swap_short(unsigned short a,unsigned long b)
{	
	b = 0;
	((char *)&b)[0] = ((char *)&a)[1];
	((char *)&b)[1] = ((char *)&a)[0];
	return b;
}

int memcmp(const void *a, const void *b, int n)
{   const unsigned char *ac = a, *bc = b;
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
		else break;
		nptr++;
	}
	*endptr = nptr;
	return val;
}

#pragma -s1

void _stack_error(Proc *p)
{
	IOdebug("TCP/IP stack overflow in %s at %x",p->Name,&p);
}
