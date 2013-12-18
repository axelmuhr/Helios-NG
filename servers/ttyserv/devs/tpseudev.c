#include "tpseudev.h"

#define FREAD	1
#define FWRITE	2

#define IOCBUF	1

/*--------------  templates for PTY server routines ----------------*/

void	Do_Open (ServInfo *srvinfo);
void	PtyRead (MCB *m, Pty *p);
void	HandleRead (Pty *p, word size, word timeout, Port reply);
void	PtyWrite (MCB *m, Pty *p);
void	PtySelect(MCB *m, Pty *p);
void	PtyGetInfo(MCB *m, Pty *p);
void	PtySetInfo(MCB *m, Pty *p);
bool	SelectResult(Pty *p, int flags);

/*------------  templates for Device Interface routines  -----------*/

void	DevOperate(TermDCB *dcb, TermReq *req);
word	DevClose(TermDCB *dcb);
TermDCB	*DevOpen(MPtr dev, TermDeviceInfo *info);
int	start_pty (TermDCB *dcb, TermDeviceInfo *info);

/*------------  templates for Device handling routines  ------------*/

int	Do_Read(Pty* p, TermReq *req);
int	Do_Write(Pty* p, TermReq *req);
void	SelWakeup(Pty *p);

/*-------------  templates for micellaneous routines  --------------*/

int	ring_init(Ring *r, char *buf, int size);
int	ring_zap(Ring *r, bool lock);
int	ring_space(Ring *r, bool lock);
int	ring_data(Ring *r, bool lock);
int	ring_put(Ring *r, char *buf, int size, bool lock);
int	ring_get(Ring *r, char *buf, int size, bool lock);

/*-------------------  Device Interface routines  ------------------*/


void DevOperate(TermDCB *dcb, TermReq *req)
{
	TermInfoReq *ireq;
	TermInfo *info;
	Pty *p = dcb->Pty;
	
    	if (p->terminating)
    		Wait(&p->Forever);

	ireq = (TermInfoReq *)req;
	info = &ireq->TermInfo;
	
	req->DevReq.Result = 0;

        switch(req->DevReq.Request)
	{
	case FG_Read:
/*		IOdebug("%s Device FG_Read(b %x,s %d,t %d)", dcb->nte->Name 
			req->Buf,req->Size, req->DevReq.Timeout);	*/
		req->Actual = Do_Read(p, req);
		if ( req->DevReq.Action )
		    (*req->DevReq.Action)(dcb,req);
		break;		
		
	case FG_Write:
/*		IOdebug("%s: Device FG_Write(b %x,s %d,t %d)", dcb->nte->Name
			req->Buf,req->Size, req->DevReq.Timeout);	*/
		req->Actual = Do_Write(p, req);
		if ( req->DevReq.Action )
		    (*req->DevReq.Action)(dcb,req);
		break;
		
	case FG_SetInfo:
/*		IOdebug("%s: Device SetInfo called", dcb->nte->Name); */
/*		IOdebug("p->setinfo = %x", p->setinfo); */
		if (p->setinfo)
			req->DevReq.Result = -1;
		else
			p->setinfo = ireq;
/*		IOdebug("p->setinfo = %x", p->setinfo); */
		break;

	case FG_GetInfo:
/*		IOdebug("%s: Device GetInfo called", dcb->nte->Name); */
/*		IOdebug("p->getinfo = %x", p->getinfo); */
		if (p->getinfo)
			req->DevReq.Result = -1;
		else
			p->getinfo = ireq;
/*		IOdebug("p->getinfo = %x", p->getinfo); */
		break;

	default:
		req->DevReq.Result = EC_Error|SS_Device|EG_WrongFn|EO_Unknown;
		break;
	}
	
	return;
}


void FreeAll(TermDCB *dcb)
{
	Free(dcb->PtyRoot);
	Free(dcb->PtyName);
	Free(dcb->PtyInfo);
	Free(dcb->Pty);
	Free(dcb);
}

word DevClose(TermDCB *dcb)
{
	word e;
	char *name = (char *)Malloc((long)strlen(dcb->nte->Name)+1);
	if (name)
            strcpy(name, dcb->nte->Name);

/*	IOdebug("%s: DevClose", name); */
	dcb->Pty->terminating = TRUE;
/*	IOdebug("%s: Deleting NTE", name); */
	while ((e = Delete (dcb->nte, NULL)) < 0)
	    IOdebug("%s: Cannot delete NTE %x", name, e);
/*	IOdebug("%s: Freeing my request port", name); */
	FreePort (dcb->PtyInfo->ReqPort);
/*	IOdebug("%s: Waiting for dispatcher termination", name); */
	Wait(&dcb->DispTerm);
/*	IOdebug("%s: Dispatcher terminated", name); */
	FreeAll(dcb);
/*	IOdebug("%s: DevClose done", name); */
	
	return (0);
}

void init_NameInfo(NameInfo *ni)
{
	ni->Port = NullPort;
	ni->Flags = Flags_StripName;
	ni->Matrix = DefNameMatrix;
	ni->LoadData = NULL;
}	

void init_DispatchInfo(DispatchInfo *di, DirNode *root, char *pn)
{
	di->Root = root;		/* root of Pty 			*/
	di->ReqPort = NullPort;		/* request port, inserted later	*/
	di->SubSys = SS_Device;		/* Subsystem code: Device	*/
	di->ParentName = pn;		/* name of parent directory	*/

	di->PrivateProtocol.Fn = NullFn;/* no Private request handler	*/
	di->PrivateProtocol.StackSize = 4000;

	di->Fntab[0].Fn = Do_Open;	/* Open     : !	Special Handler	*/
	di->Fntab[0].StackSize = 2000;
	di->Fntab[1].Fn = InvalidFn;	/* Create   :	not supported	*/
	di->Fntab[1].StackSize = 2000;
	di->Fntab[2].Fn = DoLocate;	/* Locate   :	Default Handler	*/
	di->Fntab[2].StackSize = 2000;
	di->Fntab[3].Fn = DoObjInfo;	/* ObjInfo  :	Default Handler	*/
	di->Fntab[3].StackSize = 2000;
	di->Fntab[4].Fn = NullFn;	/* ServInfo :	not supported	*/
	di->Fntab[4].StackSize = 2000;
	di->Fntab[5].Fn = InvalidFn;	/* Delete   : 	not supported	*/
	di->Fntab[5].StackSize = 2000;
	di->Fntab[6].Fn = InvalidFn;	/* Rename   :	not supported	*/
	di->Fntab[6].StackSize = 2000;
	di->Fntab[7].Fn = InvalidFn;	/* Link     :	not supported	*/
	di->Fntab[7].StackSize = 2000;
	di->Fntab[8].Fn = InvalidFn;	/* Protect  :	not supported	*/
	di->Fntab[8].StackSize = 2000;
	di->Fntab[9].Fn = InvalidFn;	/* SetDate  :	not supported	*/
	di->Fntab[9].StackSize = 2000;
	di->Fntab[10].Fn = InvalidFn;	/* Refine   :	not supported	*/
	di->Fntab[10].StackSize = 2000;
	di->Fntab[11].Fn = NullFn;	/* CloseObj :	not supported	*/
	di->Fntab[11].StackSize = 2000;
}

void init_Pty(Pty *p)
{
	InitSemaphore(&p->ReadLock, 1);
	InitSemaphore(&p->WriteLock, 1);
	InitSemaphore(&p->SelectLock, 1);
	p->SelectPort = NullPort;
	p->terminating = FALSE;
	InitSemaphore(&p->Forever, 0);
	ring_init(&p->iring, p->ibuf, RINGSIZE);
	ring_init(&p->oring, p->obuf, RINGSIZE);
	/* for now */
	p->Flags = PF_NBIO | PF_PKT;
	p->getinfo = NULL;
	p->setinfo = NULL;
}


TermDCB *DevOpen(MPtr dev, TermDeviceInfo *info)
{
	TermDCB *dcb;
	int e;
		
	dcb = (TermDCB *)Malloc(sizeof(TermDCB));
	if( dcb == NULL )
		return NULL;

	memset( dcb,  0x0, sizeof(TermDCB));

	dcb->PtyRoot = (DirNode *)Malloc(sizeof(DirNode));
	dcb->PtyName = (NameInfo *)Malloc(sizeof(NameInfo));
	dcb->PtyInfo = (DispatchInfo *)Malloc(sizeof(DispatchInfo));
	dcb->Pty     = (Pty *)Malloc(sizeof(Pty));

	if (    (dcb->PtyRoot == NULL)
	     || (dcb->PtyName == NULL)
	     || (dcb->PtyInfo == NULL)
	     || (dcb->Pty     == NULL)
	   )
	   goto bad;

	memset( dcb->PtyRoot, 0x0, sizeof(DirNode));
	memset( dcb->PtyName, 0x0, sizeof(NameInfo));
	memset( dcb->PtyInfo, 0x0, sizeof(DispatchInfo));
	memset( dcb->Pty,     0x0, sizeof(Pty));
	
	dcb->dcb.Device = dev;
	dcb->dcb.Operate = DevOperate;
	dcb->dcb.Close = DevClose;

	MachineName (dcb->McName);		/* Get machine name. 	*/
	InitSemaphore (&dcb->DispTerm, 0);
	
	init_DispatchInfo(dcb->PtyInfo, dcb->PtyRoot, dcb->McName);
	init_NameInfo(dcb->PtyName);
	dcb->Pty->dcb = (DCB *) dcb;
	init_Pty(dcb->Pty);

	if ((e = start_pty(dcb, info)) != 0)
	  {
	    char name[32];
	    char text[24];
	    
	    
	    DeviceName_(name,dev);

	    SetString_( text, 0, '%', 's', ':', ' ' );
	    SetString_( text, 1, 'D', 'e', 'v', 'O' );
	    SetString_( text, 2, 'p', 'e', 'n', ' ' );
	    SetString_( text, 3, 'f', 'a', 'i', 'l' );
	    SetString_( text, 4, 'e', 'd', ' ', '%' );
	    SetString_( text, 5, 'd', '\0', '\0', '\0' );
	    
	    IOdebug("%s: DevOpen failed %d", name, e);
	    
	    goto bad;
	}
/*	IOdebug("%s: DevOpen '%s'", dev->Name, dcb->nte->Name); */
	return (dcb);

bad:
	FreeAll(dcb);
		
	return 0;
}


void DoDispatch(TermDCB *dcb)
{
	Dispatch(dcb->PtyInfo);
	if (dcb->Pty->terminating == FALSE)
		raise(SIGTERM);
	Signal(&dcb->DispTerm);
}

int start_pty (TermDCB *dcb, TermDeviceInfo *info)
{
  word 		e;
  Object *	o;			/* machine root object		*/
  Object *	n;
  LinkNode *	Parent;		/* Node for parent link		*/
  char *	name = info->NTE_Name;
  Stream *	reply= info->write;
  DirNode *	root = dcb->PtyRoot;
  Pty	*	p    = dcb->Pty;
  char		text[ 10 ];
  
  
  o = Locate (NULL, dcb->McName);
  
  if ((n = Locate(o, name)) != NULL)
    return(-1);
  
  InitNode ((ObjNode *)root, name, Type_Directory, 0, DefDirMatrix);
  InitList(&root->Entries);
  
  /* Install my request port.	*/
  
  dcb->PtyInfo->ReqPort = dcb->PtyName->Port = NewPort();
  
  if (dcb->PtyName->Port == NullPort)
    return(-2);
  
  Parent = (LinkNode *) Malloc (sizeof (LinkNode) + (long)strlen (dcb->McName));
  
  if (Parent == NULL)
    return(-3);
  
  SetString_( text, 0, '.', '.', '\0', '\0' );	/* ".." */
  
  InitNode (&Parent->ObjNode, text, Type_Link, 0, DefDirMatrix);
  
  Parent->Cap = o->Access;
  
  InitList(&Parent->ObjNode.Contents);
  
  strcpy (Parent->Link, dcb->McName);
  
  root->Parent = (DirNode *)Parent;
  
  /* initialise Pty's ObjNode	*/
  
  SetString_( text, 0, 'e', 'n', 't', 'r' );
  SetString_( text, 1, 'y', '\0', '\0', '\0' );	/* "entry" */
  
  InitNode (&p->ObjNode, text, Type_File, 0, DefFileMatrix);
  InitList(&p->ObjNode.Contents);
  
  /* now insert the 'entry' object in the directory */
  
  Insert (root, &p->ObjNode, FALSE);
  
  dcb->nte = Create (o, name, Type_Name, sizeof(NameInfo), (byte *)dcb->PtyName);
  
  Close (o);
  
  if (dcb->nte == NULL)
    {
      FreePort(dcb->PtyName->Port);     	
      return (-4);
    }
  
  e = Write( reply, dcb->nte->Name, strlen(dcb->nte->Name), IdleTimeout);
  
  if (e != strlen(dcb->nte->Name))
    {
      FreePort(dcb->PtyInfo->ReqPort);     	
      Delete (dcb->nte, NULL);
      return(-5);
    }
  
  /* This was the intialisation,	*/
  /* now comes the work.		*/
  
  e = Fork( 4000, DoDispatch, 4, dcb);
  
  if (e) 
    return (0);    	/* success	*/
  
  FreePort(dcb->PtyInfo->ReqPort);     	
  Delete (dcb->nte, NULL);
  Close(dcb->nte);
  
  return(-6);
  
} /* start_pty */

/*-----------------  Device handling routines  ---------------------*/


int Do_Read(Pty *p, TermReq *req)
{
	int timeout = (int)(req->DevReq.Timeout);
	int evil    = timeout == -1;
	int tstep   = (int)(OneSec / 200);
	int actual  = 0;
		
	unless (req->Size > 0)
		return 0;
		
	SelWakeup(p);
	while (	
		( ring_data(&p->iring, TRUE) == 0 ) 
		&&
		( evil || (timeout >= 0) )
	      )
	{
		if (p->terminating)
			Wait(&p->Forever);
		Delay(tstep);
		timeout -= tstep;
	}
	actual = ring_get(&p->iring, (char *)req->Buf, (int)req->Size, TRUE);
	return actual;
}

int Do_Write(Pty *p, TermReq *req)
{
	int size    = (int)(req->Size);
	int actual  = 0;
	int timeout = (int)(req->DevReq.Timeout);
	int evil    = timeout == -1;
	int tstep   = (int)(OneSec / 200);
	
	unless (size > 0)
		return 0;
		
	while (	
		(actual < size) 
		&&
		( evil || (timeout > 0) )
	      )
	{
		if (p->terminating)
			Wait(&p->Forever);
		actual += ring_put(&p->oring, ((char *)(req->Buf)) + actual,
				    size - actual, TRUE);
		SelWakeup(p);
		timeout -= tstep;
		Delay(tstep);
	}
	return (actual);	
}


/*--------------------  PTY server routines  -----------------------*/

void
Do_Open (ServInfo *srvinfo)
{
    char	*pathname = srvinfo->Pathname;
    MCB		*m = srvinfo->m;
    IOCMsg2	*req	= (IOCMsg2 *) (m->Control);
    char	*data= m->Data;
    MsgBuf	*r;
    DirNode	*d;
    ObjNode	*o;
    Port	reqport;
    word	e;
    Pty		*p;
    
/*    IOdebug ("OPEN (%s) started.", pathname); */

    d = (DirNode *) GetTargetDir (srvinfo);
    if (d == NULL) {
	ErrorMsg (m, Err_Null);
	return;
    }

    reqport = NewPort ();		/* Get a new port for requests.	*/
    if (reqport == NullPort) {
	ErrorMsg (m, EC_Warn + EG_NoMemory + EO_Server);
	return;
    }

    r = New (MsgBuf);			/* Get a Buffer for the reply.	*/
    if (r == NULL) {
	ErrorMsg (m, EC_Warn + EG_NoMemory + EO_Server);
	FreePort (reqport);
	return;
    }

    o = GetTargetObj (srvinfo);

    if (o == NULL)			/* we are not called on our	*/
    {					/* root or 'entry' object.	*/
	if (req->Arg.Mode & O_Create)
	    ErrorMsg (m, EC_Error + EG_Create + EO_File);
	else
	    ErrorMsg (m, EC_Error + EG_Unknown + EO_Object);
	FreePort (reqport);
	Free (r);
	return;
    }

    /* Check the access rights...	*/
    unless (CheckMask (req->Common.Access.Access, (int)req->Arg.Mode & Flags_Mode))
    {
	ErrorMsg (m, EC_Error + EG_Protected + EO_File);
	FreePort (reqport);
	Free (r);
	return;
    }

    if (o->Type == Type_Directory)	/* The object is the root.	*/
    {					/* Let this be done by the	*/
				    	/* directory handler.		*/
        FormOpenReply (r, m, o, Flags_Server|Flags_Closeable, pathname);
	r->mcb.MsgHdr.Reply = reqport;
	PutMsg (&r->mcb);		/* send an open reply to	*/
	Free (r);			/* the client.			*/
	DirServer (srvinfo, m, reqport);
	FreePort (reqport);
	return;
    }

    /* now we are in 'entry' */
    /* get the Pty structure */
    p = (Pty *) o;

    if (o->Account)
    {
	ErrorMsg (m, EC_Error + EG_InUse + EO_File);
	FreePort (reqport);
	Free (r);
	return;
    }

    FormOpenReply (r, m, o, Flags_Server|Flags_Closeable|Flags_Selectable,
    	pathname);
    r->mcb.MsgHdr.Reply = reqport;
    PutMsg (&r->mcb);			/* send an open reply to	*/
    Free (r);				/* the client.			*/

/*    IOdebug ("%s opened.", pathname);  */

    o->Account++;			/* Increment the usage counter. */

    UnLockTarget (srvinfo);

    forever				/* Loop for the Stream GSP	*/
    {
	m->MsgHdr.Dest = reqport;
	m->Timeout = IOCTimeout; /* StreamTimeout; */
	m->Data = data;

	e = GetMsg (m);			/* Get the next request.	*/
	if (e == EK_Timeout) break;	/* If Timeout, then terminate,	*/
	if (e < Err_Null) continue;	/* else ignore the message.	*/	
	
	Wait (&o->Lock);		/* Lock the object and call the	*/
					/* known handling functions.	*/

	if (p->terminating)
		Wait(&p->Forever);

	switch (m->MsgHdr.FnRc & FG_Mask)
	{
	case FG_Read:
	    PtyRead (m, p);
	    break;

	case FG_Write:
	    PtyWrite (m, p);
	    break;

	case FG_SetInfo:
	    PtySetInfo (m, p);
	    break;

	case FG_GetInfo:
	    PtyGetInfo (m, p);
	    break;

	case FG_Select:
	    PtySelect (m, p);
	    break;

	case FG_Close:
	    if (m->MsgHdr.Reply != NullPort) 
	    	ErrorMsg (m, Err_Null);
	    FreePort (reqport);
	    o->Account--;		/* Remove the usage mark	*/
	    
	    unless (o->Account) {
	    	Signal (&o->Lock);	/* and release the object	*/
	    	if (p->terminating == FALSE)
		    raise (SIGTERM);	/* and signal for termination	*/
	    }
	    else
		Signal (&o->Lock);	/* and release the object	*/
	    return;

	default:
/*	    IOdebug ("%s: WrongFn", pathname, m->MsgHdr.FnRc & FG_Mask); */
	    ErrorMsg (m, EC_Error + EG_WrongFn + EO_File);
	    break;
	}
	Signal (&o->Lock);		/* Release the object for the	*/
					/* next request.		*/
    }

/*    IOdebug ("%s: Stream timed out !!!", pathname); */

					/* The stream has timed out :	*/
    Wait (&o->Lock);			/* Lock the object,		*/
    FreePort (reqport);			/* free the Stream Port,	*/
    o->Account--;			/* remove the usage mark	*/
    Signal (&o->Lock);			/* and release the object.	*/
}



void
PtyRead (MCB *m, Pty *p)
{
    ReadWrite	*rw	= (ReadWrite *) m->Control;
    word	timeout	= rw->Timeout;
    word	size	= rw->Size;

/*    IOdebug ("%s: Read request %d bytes, port %x, timeout %d.",
	p->ObjNode.Name, size, m->MsgHdr.Reply, timeout);	*/

    if (size < 0)			/* Check the read size :	*/
    {					/* no negative sizes allowed !	*/
	ErrorMsg (m, EC_Error | EG_Parameter | 2);
	return;
    }

#if IOCBUF
    if (size > IOCDataMax)		/* limit the read size : max.	*/
	size = IOCDataMax;		/* size is IOCDataMax.		*/
#else
    if (size > RINGSIZE)		/* limit the read size : max.	*/
	size = RINGSIZE;		/* size is RINGSIZE.		*/
#endif

    timeout = (timeout < 0 || timeout > IdleTimeout) ? IdleTimeout : timeout;

#if 0
    if (TestSemaphore (&p->ReadLock) < 1)	/* check active Read */
    {
	ErrorMsg (m, SS_Device + EC_Recover + EG_InUse + EO_Port);
	return;
    }
#endif

    /* this Wait will NOT wait, because the ObjNode is locked !	*/
    Wait (&p->ReadLock);	

    unless (Fork (2000, HandleRead, 16, p, size, timeout, m->MsgHdr.Reply))
    {
	ErrorMsg (m, SS_Device + EC_Warn + EG_NoMemory + EO_Server);
	Signal (&p->ReadLock);
	return;
    }
}

void
HandleRead (Pty *p, word size, word timeout, Port reply)
{
    MCB		m;
    word	rc = 0;
    word	control[1];
#if IOCBUF
    char	data[IOCDataMax + 1];
#else
    char	*data;
#endif
    char	*buffer;
    int		result = 0;
    int		got;
    int		tstep  = (int)(OneSec / 200);
    bool	packet = ( (p->Flags & PF_PKT) == PF_PKT);
    bool	nbio = ( (p->Flags & PF_NBIO) == PF_NBIO);
    
#if IOCBUF == 0
    data = Malloc(RINGSIZE + 1);
    if (data == NULL) {
	rc = EC_Warn + EG_NoMemory + EO_Server;
	goto repl
    }
#endif

    if (packet)
    {
	/* reserve one byte for packet flag */
    	*data = '\0';
	size--;
    	buffer = data+1;
    }
    else
    	buffer = data;
    
    while (result < size)
    {
#if 0
	Wait(&p->oring.lock);
	ring_debug(&p->oring, "PTY -> req", size - result, FALSE);
	got = ring_get(&p->oring, buffer + result, size - result, FALSE);
	ring_debug(&p->oring, "PTY -> got", got, FALSE);
	Signal(&p->oring.lock);
#else
	got = ring_get(&p->oring, buffer + result, (int)(size - result), TRUE);
#endif
    	result += got;
	if ( (got == 0) && nbio )
	    break;			/* nonblocking IO */
		
	if (p->terminating)
	    Wait(&p->Forever);
		
    	if (got == 0) {
	    Delay (tstep);		/* sleep a little bit,		*/
	    if ((timeout -= tstep) < 0)	/* then check for timeout :	*/
		break;			/* reached, so break		*/
	    continue;			/* else try reading once more	*/
	}
    }
    if ((result > 0) || nbio)
	rc = ReadRc_EOD;
    else
	rc = SS_Device + EC_Recover + EG_Timeout + EO_Stream;
    if (packet)
	result++;

#if IOCBUF == 0
repl:
#endif
    
    InitMCB (&m, 0, reply, NullPort, rc);	/* build reply message.	*/
    m.MsgHdr.DataSize = result;
    m.Control = control;
    m.Data = (byte *) data;
    PutMsg (&m);			/* Send the result back.	*/

#if IOCBUF == 0
    Free(data);
#endif

    Signal (&p->ReadLock);		/* unlock read semaphore.	*/
    
/*    IOdebug ("%s : HandleRead ready, size = %d, rc = %08x.",
	p->ObjNode.Name, result, rc);	*/
}


void PtyWrite (MCB *m, Pty *p)
{
    ReadWrite	*rw	= (ReadWrite *) m->Control;
    Port	dest	= m->MsgHdr.Dest;
    Port	reply	= m->MsgHdr.Reply;
    word	dsize	= m->MsgHdr.DataSize;
    word	timeout	= rw->Timeout;
    word	size	= rw->Size;
    char	*data	= m->Data;
    word	tlimit;
    word	tstep	= OneSec / 200;
    word	result	= 0;	/* number of bytes received total	   */
    int		got;		/* number of bytes received for each chunk */
    word	error	= Err_Null;
    bool	nbio	= ((p->Flags & PF_NBIO) == PF_NBIO);
    bool	stopped	= ((p->Flags & PF_STOPPED) == PF_STOPPED);
    bool	nostop	= ((p->Flags & PF_NOSTOP) == PF_NOSTOP);

/*    IOdebug ("%s : Write request %d bytes, port %x, timeout %d.",
	p->ObjNode.Name, size, reply, timeout);		*/

    if (size <= 0)			/* nothing to be written :	*/
    {					/* send Done message.		*/
	InitMCB (m, 0, reply, NullPort, WriteRc_Done);
	MarshalWord (m, 0);
	PutMsg (m);
	return;
    }
					/* calculate private timeout	*/
    tlimit = (timeout < 0 || timeout > 20 * OneSec) ? 20 * OneSec : timeout;

    /* wait until write is enabled */
    while ( stopped && !nostop )	
    {
	if (p->terminating)
	    Wait(&p->Forever);

	if (nbio)
	    /* nonblocking IO */
	    goto done;

	Delay (tstep);			/* sleep 1/20 sec		*/

	if ((tlimit -= tstep) <= 0)	/* timelimit reached ?		*/
	{				/* yes, abort transfer		*/
	    ErrorMsg (m, SS_Device + EC_Recover + EG_Timeout + EO_Stream);
	    return;
	}
    }

#if 0
    if (TestSemaphore (&p->WriteLock) < 1)	  /* check active Write */
    {
	ErrorMsg (m, SS_Device + EC_Recover + EG_InUse + EO_Port);
	return;
    }
#endif

    /* this Wait will NOT wait, because the ObjNode is locked! */
    Wait (&p->WriteLock);

    /* some data came with the request: put it into the input ring */
    if (dsize > size)	/* check data size. ( must	*/
	dsize = size;	/* not be more than size ).	*/

    got  = 0;
    while (got < dsize)
    {	
#if 0
	int cnt;
	Wait(&p->iring.lock);
	ring_debug(&p->iring, "PTY <- req", dsize - got, FALSE);
	cnt = ring_put(&p->iring, data + got, dsize - got, FALSE);
	ring_debug(&p->iring, "PTY <- put", cnt, FALSE);
	Signal(&p->iring.lock);
#else
    	int cnt = ring_put(&p->iring, data + got, (int)(dsize - got), TRUE);
#endif
    	got += cnt;
    	result += cnt;

	if ( (cnt == 0) && nbio )
	    /* nonblocking IO */
	    goto done;
	    
	if (p->terminating)
	    Wait(&p->Forever);
		
    	unless (cnt) {
	    Delay (tstep);		/* sleep a little bit,		*/
	    if ((timeout -= tstep) < 0)	/* then check for timeout :	*/
		goto done;		/* reached, so break		*/
	    continue;			/* else try writing once more	*/
	}
    }
    if (result >= size)			/* that was all the work...	*/
	goto done;
	
    InitMCB (m, MsgHdr_Flags_preserve, reply, NullPort, WriteRc_Sizes);
#if IOCBUF
    MarshalWord (m, IOCDataMax);	/* More data expected, tell the	*/
    MarshalWord (m, IOCDataMax);	/* Client about our buffer.	*/
#else
    MarshalWord (m, RINGSIZE);		/* More data expected, tell the	*/
    MarshalWord (m, RINGSIZE);		/* Client about our buffer.	*/
#endif

    if ((error = PutMsg (m)) < Err_Null)	/* send Buffer sizes	*/
    {
/*	IOdebug ("Write request error %x on sending sizes.", error); */
	Signal (&p->WriteLock);
	return;
    }

    m->MsgHdr.Dest = dest;		/* now the real work starts...	*/
    m->Timeout = WriteTimeout;

    while (result < size)
    {
	if ((error = GetMsg (m)) < Err_Null)	/* get next data block	*/
	{
/*	    IOdebug ("%s : Write request received error %x", 
		p->ObjNode.Name, error); */
	    Signal (&p->WriteLock);
	    return;			/* on error, abort transfer.	*/
	}
	dsize = m->MsgHdr.DataSize;
	data = m->Data;
	got  = 0;

	if ((result + dsize) > size) 	/* result + dsize must not be 	*/
		dsize = size - result;	/* more than size.		*/

	while (got < dsize)
	{	
#if 0
	    int cnt;
	    Wait(&p->iring.lock);
	    ring_debug(&p->iring, "PTY <- req", dsize - got, FALSE);
	    cnt = ring_put(&p->iring, data + got, dsize - got, FALSE);
	    ring_debug(&p->iring, "PTY <- put", cnt, FALSE);
	    Signal(&p->iring.lock);
#else
	    int cnt = ring_put(&p->iring, data + got, (int)(dsize - got), TRUE);
#endif
	    got += cnt;
	    result += cnt;

	    if ( (cnt == 0) && nbio )
		/* nonblocking IO */
		/* when I get here, result is > 0 */
		goto done;
		
	    if (p->terminating)
		Wait(&p->Forever);
		
	    unless (cnt) {
		Delay (tstep);			/* sleep a little bit,		*/
		if ((timeout -= tstep) < 0)	/* then check for timeout :	*/
		    goto done;			/* reached, so break		*/
		continue;			/* else try writing once more	*/
	    }
	}
    }

done:

/* When I get here, an error occured or all data is in the buffer	*/
/* and the Client waits for a Reply and I can release the Server port.	*/

    InitMCB (m, 0, reply, NullPort, error < 0 ? error : WriteRc_Done);
    MarshalWord (m, result);
    PutMsg (m);

    Signal (&p->WriteLock);		/* unlock Write semaphore.	*/

}


void PtyGetInfo (MCB *m, Pty *p)
{
    Port	reply	= m->MsgHdr.Reply;

    Attributes  *a;
    
    if ((p->getinfo == 0) || (p->getinfo->DevReq.Action == 0))
      {
	/* IOdebug("PtyGetInfo STATE!"); */
	ErrorMsg (m, SS_Device + EC_Recover + EG_State);
	return;
    }

    (*p->getinfo->DevReq.Action)(p->dcb, p->getinfo);
    a = &p->getinfo->TermInfo.Attr;
   
    InitMCB (m, 0, reply, NullPort, Err_Null);
    MarshalData (m, sizeof (Attributes), (byte *) a);
    PutMsg (m);
}



void PtySetInfo (MCB *m, Pty *p)
{
    Port	reply	= m->MsgHdr.Reply;
    Attributes  *a;
#if 0
    int dsize = m->MsgHdr.DataSize;
    if (dsize != sizeof(Attributes))
	IOdebug("Setinfo datasize: get %d expected %d",
		 dsize, sizeof(Attributes));
#endif
    
    if ((p->setinfo == 0) || (p->setinfo->DevReq.Action == 0))
      {
	/* IOdebug("PtySetInfo STATE!"); */
	ErrorMsg (m, SS_Device + EC_Recover + EG_State);
	return;
    }

    a = &p->setinfo->TermInfo.Attr;
    memcpy (a, m->Data, sizeof (Attributes));
    (*p->setinfo->DevReq.Action)(p->dcb, p->setinfo);

    InitMCB (m, 0, reply, NullPort, Err_Null);
    PutMsg (m);

}

void SelWakeup(Pty *p) 
{
	MCB m;

	Wait(&p->SelectLock);
	if( p->SelectPort != NullPort )
	{
		int fn     = (int)(p->SelectMode);
		int result = 0;

		if( (fn & O_WriteOnly) && SelectResult(p, FWRITE) )
			result |= O_WriteOnly;	
		if( (fn & O_ReadOnly) && SelectResult(p, FREAD) )
			result |= O_ReadOnly;
		if( (fn & O_Exception) && SelectResult(p, 0) )
			result |= O_Exception;
			
#if 0
	IOdebug ("PTY: SelectWakeup (%s,%s,%s) = (%s,%s,%s)", 
		(fn & O_ReadOnly) ? "read" : "", 
		(fn & O_WriteOnly) ? "write" : "", 
		(fn & O_Exception) ? "except" : "",
		(result & O_ReadOnly) ? "read" : "", 
		(result & O_WriteOnly) ? "write" : "", 
		(result & O_Exception) ? "except" : ""); 
#endif
		if (result)
		{
			InitMCB(&m, 0, p->SelectPort, NullPort, result);
			PutMsg(&m);
			p->SelectPort = NullPort;
		}
	}
	Signal(&p->SelectLock);
	return;
}

void PtySelect(MCB *m, Pty *p)
{
	int result = 0;
	word fn = m->MsgHdr.FnRc & FF_Mask;
	m->MsgHdr.FnRc = 0;

	Wait(&p->SelectLock);
	if( (fn & O_ReadOnly) && SelectResult(p, FREAD) )
		result |= O_ReadOnly;
	if( (fn & O_WriteOnly) && SelectResult(p, FWRITE) )
		result |= O_WriteOnly;	
	if( (fn & O_Exception) && SelectResult(p, 0) )
		result |= O_Exception;

#if 0
	IOdebug ("PTY: SelectRequest (%s,%s,%s) = (%s,%s,%s)", 
		(fn & O_ReadOnly) ? "read" : "", 
		(fn & O_WriteOnly) ? "write" : "", 
		(fn & O_Exception) ? "except" : "",
		(result & O_ReadOnly) ? "read" : "", 
		(result & O_WriteOnly) ? "write" : "", 
		(result & O_Exception) ? "except" : ""); 
#endif
	
	if( result )
	{
	    ErrorMsg(m, result);
	    p->SelectPort = NullPort;
	}
	else
	{
#if 0
	IOdebug ("PTY: SelectRequest going to sleep");
#endif
	    FreePort(p->SelectPort);
	    p->SelectMode = fn & FF_Mask;
	    p->SelectPort = m->MsgHdr.Reply;
	}
	Signal(&p->SelectLock);
	return;
}

bool SelectResult(Pty *p, int flag)
{
	switch (flag) {
	    case FREAD:
		return (ring_data(&p->oring, TRUE) > 0);

	    case FWRITE:
		{
		    bool stopped = (p->Flags & PF_STOPPED) == PF_STOPPED;
		    bool nostop  = (p->Flags & PF_NOSTOP) == PF_NOSTOP;
		    return ( 
		    	     (ring_space(&p->iring, TRUE) > 0)
			     && 
			     ( !stopped || nostop )
			   );
		}
			   
	    case 0: /* Exception */
	        /* only for TIOCPKT_FLUSHWRITE */
		return 0;

	    default:
	    	return 0;
	}	
}

/*--------------------  miscellaneous routines  ---------------------*/


int ring_init(Ring *r, char *buf, int size)
{
	InitSemaphore(&r->lock, 1);
	r->buf = buf;
	r->size = size;
	r->fp = 0;
	r->ep = 0;
	r->empty = TRUE;
#if 0
	ring_debug(r, "ring_init", 0, TRUE); 
#endif
	return 0;
}

int ring_zap(Ring *r, bool lock)
{
	bool empty;
	
	if (lock) Wait(&r->lock);
#if 0
	ring_debug(r, "ring_zap start", 0, TRUE); 
#endif
	empty = r->empty;
	r->fp = 0;
	r->ep = 0;
	r->empty = TRUE;
#if 0
	ring_debug(r, "ring_zap done", !empty, TRUE); 
#endif

	if (lock) Signal(&r->lock);
	return !empty;
}

int ring_data(Ring *r, bool lock)
{
	int s;
	
	if (lock) Wait(&r->lock);
	
	if ((s = r->ep - r->fp) == 0) 
		s = r->empty ? 0 : r->size;
	if (s < 0)
 		s += r->size;

	if (lock) Signal(&r->lock);	
	return s;
}

int ring_space(Ring *r, bool lock)
{
	int d;
	
	if (lock) Wait(&r->lock);
	d = r->size - ring_data(r, FALSE);
	if (lock) Signal(&r->lock);

	return d;
}

int ring_put(Ring *r, char *buf, int size, bool lock)
{
	int l;
	
	if (lock) Wait(&r->lock);
#if 0
	ring_debug(r, "ring_put start", 0, FALSE); 
#endif
	size = min(size, ring_space(r, FALSE));
	if (size) {
		l = min(size, r->size - r->ep);
		memcpy(&r->buf[r->ep], buf, l);
		if (l < size)
			memcpy(r->buf, &buf[l], size-l);
		if ( (r->ep += size) >= r->size)
			r->ep -= r->size;
		r->empty = FALSE;
	}
#if 0
	ring_debug(r, "ring_put done", size, FALSE); 
#endif
	if (lock) Signal(&r->lock);

	return size;
}


int ring_get(Ring *r, char *buf, int size, bool lock)
{
	int l;
	
	if (lock) Wait(&r->lock);
#if 0
	ring_debug(r, "ring_get start", 0, FALSE); 
#endif
	size = min(size, ring_data(r, FALSE));
	if (size) {
		l = min(size, r->size - r->fp);
		memcpy(buf, &r->buf[r->fp], l);
		if (l < size)
			memcpy(&buf[l], r->buf, size-l);
		if ( (r->fp += size) >= r->size)
			r->fp -= r->size;
		r->empty = r->fp == r->ep;
	}
#if 0
	ring_debug(r, "ring_get done", size, FALSE); 
#endif
	if (lock) Signal(&r->lock);

	return size;
}

#if 0
int ring_debug(Ring *r, char *msg, int msgi, bool lock)
{
	if (lock) Wait(&r->lock);

	IOdebug("%s(%d): %x(%d), %d(%d)%d",
		msg, msgi, r->buf, r->size, 
		r->fp, ring_data(r, FALSE), r->ep );

	if (lock) Signal(&r->lock);
}
#endif
