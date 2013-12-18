/* $Header: /hsrc/filesys/pfs_v2.1/src/fs/RCS/tserver.c,v 1.1 1992/07/13 16:17:41 craig Exp $ */

/* $Log: tserver.c,v $
 * Revision 1.1  1992/07/13  16:17:41  craig
 * Initial revision
 *
 * Revision 2.1  90/08/31  12:50:11  guenter
 * first multivolume/multipartition PFS with tape
 * 
 * Revision 1.2  90/02/23  11:24:05  chris
 * When tape open fails -> release sempahore
 * 
 * Revision 1.1  90/02/20  11:25:30  chris
 * Initial revision
 * 
 */
 
                                                                                /*
  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                    (c) 1991 by parsytec GmbH, Aachen                    |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |                          Parsytec File System                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  tserver.c								     |
   |                                                                         |
   |	Routines handling Tape Operations.				     |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    1 - C.Selwyn - 12 January 1990 - Basic version                       |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#define UnloadVerbose 0

#include <root.h>

/*FIXME*/
#include <syslib.h>

#define DEBUG 	   0
#define GEPDEBUG   0
#define FLDEBUG    0
#define HDEBUG     1
#define NAMES 	   0
#define IN_NUCLEUS 1

#include "error.h"
#include "fserr.h"
#include "nfs.h"


extern word debug_dev;
extern word debug_bnr;

word debug_dev;
word debug_bnr;

void tdispatch(VD *vol);
static void tworker (MsgBuf *m,DispatchInfo *info, VD *vol);
void test_tape (struct incore_i *tapei);

#define tworkerSS 10000

#define MAX_RETRIES	60	/* time to wait for termination of stream */
				/* workers during an unload or termvol in */
				/* seconds */   


#define MAX_DATA_SIZE 		0x8000	/* maximum message size to client */
#define MAX_BUFFER_SIZE		0x40000 /* maximum size of tape buffer */

static void tdo_open(ServInfo *, VD *);
static void tdo_create(ServInfo *, VD *);
static void tdo_locate(ServInfo *, VD *);
static void tdo_objinfo(ServInfo *, VD *);
static void tdo_serverinfo(ServInfo *, VD *);
static void tdo_delete(ServInfo *, VD *);
static void tdo_rename(ServInfo *, VD *);
static void tdo_link(ServInfo *, VD *);
static void tdo_protect(ServInfo *, VD *);
static void tdo_setdate(ServInfo *, VD *);
static void tdo_refine(ServInfo *, VD *);
static void tdo_closeobj(ServInfo *, VD *);

static int tget_context(ServInfo *, VD *);

static void do_tape_read  (MCB *m,     struct  incore_i *tapei);
static word do_tape_write (MCB *m,     struct  incore_i *tapei);
static void do_tape_seek  (MCB *m,     struct  incore_i *tapei);
static word write_tape    (int  drive,         char     *buf,  word size);


#define imin(a,b) ((a)<(b)? (a):(b))



void 
tdispatch (VD *vol)

/*
*  Receives the request messages to TapeServer at the Server Port,
*  creating 'fworker' processes to perform each request.
*/

{
	MsgBuf		*m;
	MCB		msg;
	Object		*nte;
	word		error = FALSE;
	NameInfo	TServerInfo;
	DispatchInfo 	info;
	word		old_flags;
	word		Control_V[IOCMsgMax];
	byte		Data_V[IOCDataMax];
	word		Err_Control_V[IOCMsgMax];
	byte		Err_Data_V[IOCDataMax];
	word		tell_param = FALSE;
	word		retries;
	load_data	*load_control;

#if UnloadVerbose /* OI 11 Dec 91 */
	load_data	*unload_control;
	word		tell_unload,
			unload_status;
	Port		term_port;	
#endif

/*
#undef  UnloadVerbose
#define UnloadVerbose 0
*/

	char		mcname[PATH_MAX];
	word		flags;
	Capability	cap;
	MsgBuf		*r;

	vol->tape = Malloc( sizeof (TapeDescriptor) );
	if ( vol->tape == NULL )
	{
		Error (FSErr [AllocTDFailed]);
		Signal ( &term_sem );		/* signal termination */
		return;
	} 	
	
	TServerInfo.Port 	= info.reqport = DebNewPort ();
	TServerInfo.Flags 	= Flags_StripName;
	TServerInfo.Matrix 	= DefFileMatrix;
	TServerInfo.LoadData 	= (word *) NULL;
	
	info.root		= NULL;
	info.subsys		= SS_HardDisk;
	info.PrivateProtocol	= NULL;
	info.fntab[0]		= tdo_open;
	info.fntab[1]		= tdo_create;
	info.fntab[2]		= tdo_locate;
	info.fntab[3]		= tdo_objinfo;
	info.fntab[4]		= tdo_serverinfo;
	info.fntab[5]		= tdo_delete;
	info.fntab[6]		= tdo_rename;
	info.fntab[7]		= tdo_link;
	info.fntab[8]		= tdo_protect;
	info.fntab[9]		= tdo_setdate; 
	info.fntab[10]		= tdo_refine;
	info.fntab[11]		= tdo_closeobj;
	info.fntab[12]		= NULL;
	info.fntab[13]		= NULL;
	info.fntab[14]		= NULL;
	info.fntab[15]		= NULL;

	{
		Object *o;

		MachineName(mcname);	/* build machine name */
		o = Locate(NULL,mcname);
		nte = Create(o,vol->vol_name,Type_Name,sizeof(NameInfo),(byte *)&TServerInfo);	/* take an entry in the processors Name Table */ 
		Close(o);		/* clear info about the client from Name Server's cache */
		pathcat (mcname,vol->vol_name);
	}
	InitMCB (&vol->unload_mcb,MsgHdr_Flags_preserve,info.reqport,NullPort,
				FC_GSP+SS_HardDisk+FG_Private+FO_Unload);
	vol->unload_mcb.Control = Err_Control_V;
	vol->unload_mcb.Data    = Err_Data_V; 	   

	InitSemaphore (&vol->servreq_sem,1);
	InitSemaphore (&vol->dircnt_sem,0);
	InitSemaphore (&vol->streamcnt_sem,0);
	InitSemaphore (&vol->streammsg_sem,0);
	vol->terminate_flag = FALSE;
	vol->loaded = FALSE;

	error = 0;
	while (!error)
	{
		/* create a buffer for messages */
		m = Malloc(sizeof(MsgBuf));
		if( m == Null(MsgBuf) ) 
		{ 
			Delay(OneSec*5); 
			continue; 
		}
		/* initialise message header */
		m->mcb.MsgHdr.Dest	= info.reqport;
		m->mcb.Timeout		= OneSec*30;
		m->mcb.Control		= m->control;
		m->mcb.Data		= m->data;
	lab1:
		/* get a request message */

GEPdebug ("volume /%s waiting for message",vol->vol_name);

		while( GetMsg(&(m->mcb)) == EK_Timeout )
		{
FLdebug ("volume /%s: TimeOut", vol->vol_name);
		}

GEPdebug ("function >0x%x<",m->mcb.MsgHdr.FnRc);


 		if( MyTask->Flags & Task_Flags_servlib )
			Report (FSErr [OnlineDebug1],
			        m->mcb.MsgHdr.FnRc,
 				(m->control[0]==-1)?NULL:&m->data[m->control[0]],
 				(m->control[1]==-1)?NULL:&m->data[m->control[1]]);

		if (TestSemaphore (&vol->servreq_sem) < 1) {
			ErrorMsg (&m->mcb,EC_Error+info.subsys+EG_Protected+EO_Server);

GEPdebug ("volume /%s, central server request port is locked!",vol->vol_name);

			Free (m);
			continue;
		}

/*-------------  FO_Terminate forces tape-server shutdown  --------------*/
/*--------------------  FO_Unload unloads volume  -----------------------*/

		if ( (m->mcb.MsgHdr.FnRc == FC_GSP+SS_HardDisk+FG_Private+FO_Terminate) 
			|| (m->mcb.MsgHdr.FnRc == FC_GSP+SS_HardDisk+FG_Private+FO_Unload ) )
		{
GEPdebug ("Unload/terminate volume /%s",vol->vol_name);

#if UnloadVerbose  /* OI 11 Dec 91 */

/*			Wait (&vol->servreq_sem);*/
			term_port 	= m->mcb.MsgHdr.Reply; 
			unload_control	= (load_data *) m->control;
			tell_unload	= unload_control->verbose;
			unload_status	= MAKE_OK;
			
			InitMCB ( &msg, MsgHdr_Flags_preserve, term_port,
				   NullPort, m->mcb.MsgHdr.FnRc );
#else			

			InitMCB ( &msg, MsgHdr_Flags_preserve, 
				  m->mcb.MsgHdr.Reply, NullPort,
				  m->mcb.MsgHdr.FnRc );
#endif			

			PutMsg ( &msg );
			retries = 0;
			if (vol->loaded) {
				Wait (&vol->servreq_sem);
				vol->terminate_flag = TRUE;
				while (retries < MAX_RETRIES) {
					if (TestSemaphore(&vol->streammsg_sem) ) {
/*
Error (FSErr [StillNIWorker], vol->vol_name, TestSemaphore (&vol->streammsg_sem));
*/
						retries++;
					}

					elif (TestSemaphore(&vol->dircnt_sem) ) {
/*
Error (FSErr [StillDirWorker], vol->vol_name, TestSemaphore(&vol->dircnt_sem));
*/
						retries++;
					}
					else {
						break;	
					}
					Delay (OneSec * 10);
				}

				if (retries >= MAX_RETRIES)
				{
Error (FSErr [UnloadForced], vol->vol_name, TestSemaphore(&vol->streammsg_sem), TestSemaphore (&vol->dircnt_sem));

#if UnloadVerbose /* OI 11 Dec 91 */				
				unload_status = MAKE_ERR;
#endif				
				}

				unload_volume (vol);
				Signal (&vol->servreq_sem);
			}
			if ( m->mcb.MsgHdr.FnRc == FC_GSP+SS_HardDisk+
					FG_Private+FO_Terminate ) {
				break;
			}


#if UnloadVerbose /* OI 11 Dec 91 */
			if (tell_unload) {
				tell_unload = FALSE;
				InitMCB ( &msg, MsgHdr_Flags_preserve, term_port, NullPort,
					  FC_GSP+SS_HardDisk+FG_Private+FO_Unload);
				msg.Control = Control_V;
				msg.Data    = Data_V; 	   
				MarshalWord (&msg,unload_status);
				PutMsg (&msg);
			}
#endif


			Free ( m );
			continue;
		}

/*
 *   try to load volume
 */

		if ( m->mcb.MsgHdr.FnRc == FC_GSP+SS_HardDisk+FG_Private+FO_Load ) {
			load_control = (load_data *) m->control;
			tell_param = load_control->verbose;
			
			InitMCB ( &msg, MsgHdr_Flags_preserve, 
				  m->mcb.MsgHdr.Reply, NullPort,
				  FC_GSP+SS_HardDisk+FG_Private+FO_Load );
			msg.Control = Control_V;
			msg.Data    = Data_V; 	   
			PutMsg ( &msg );
		
			if ( !vol->loaded ) { 	/* try to load if not loaded */

GEPdebug ("loading volume /%s",vol->vol_name);

				InitSemaphore (&vol->servreq_sem,1);
				InitSemaphore (&vol->dircnt_sem,0);
				InitSemaphore (&vol->streamcnt_sem,0);
				InitSemaphore (&vol->streammsg_sem,0);
				vol->terminate_flag = FALSE;

				error = load_volume (vol);
				if (error < 0)
				{
					Error (FSErr [HWLoad], vol->vol_name);
					ErrorMsg(&m->mcb,EC_Error+EG_Unknown+EO_Object);
					break;
				}	
				else if ( error )
				{
					Error (FSErr [DevLoad], vol->vol_name);
					ErrorMsg(&m->mcb,EC_Error+EG_Unknown+EO_Object);
					break;
				}
				vol->tape->tapebuf = NULL;
				vol->tape->tapebufsize = 0;
				vol->tape->tape_position = 0;
				memset(&vol->tape->tapeinode,0,sizeof(struct incore_i));
				InitSemaphore(&vol->tape->tapeinode.ii_sem,1);
				vol->tape->tapeinode.ii_dev = vol->volnum;
Hdebug ("tdisp: Count 0;");					
				vol->tape->tapeinode.ii_count = 0;
				vol->tape->tapeinode.ii_i.i_mode = DATA;
				vol->tape->tapeinode.ii_i.i_size = 0;
				vol->tape->tapeinode.ii_i.i_atime = GetDate();
				vol->tape->tapeinode.ii_i.i_mtime = GetDate();
				vol->tape->tapeinode.ii_i.i_ctime = GetDate();
				vol->tape->tapeinode.ii_i.i_cryptkey = NewKey();
				vol->tape->tapeinode.ii_i.i_matrix = DefFileMatrix;
				strcpy(vol->tape->tapeinode.ii_name, vol->vol_name);
			}
		}
		
		if ( !vol->loaded ) {
			if ( m->mcb.MsgHdr.FnRc == FG_Locate ) {
				r = New(MsgBuf);
				if ( r == Null(MsgBuf) ) {
					ErrorMsg(&m->mcb,EC_Error+EG_NoMemory+EO_Message);
					Free (m);
					continue;	
				}
				flags = 0;
				if( m->mcb.MsgHdr.Reply & Port_Flags_Remote ) 
					flags |= Flags_Remote;

				*((int *)&(r->mcb)) = 0;
				r->mcb.MsgHdr.Dest = m->mcb.MsgHdr.Reply;
				r->mcb.MsgHdr.Reply = NullPort;
				r->mcb.MsgHdr.FnRc = Err_Null;
	
				r->mcb.Timeout = IOCTimeout;
				r->mcb.Control = r->control;
				r->mcb.Data    = r->data;

				MarshalWord(&r->mcb,Type_Directory);
				MarshalWord(&r->mcb,flags);
				cap.Access = 0;
				cap.Valid[0] = 0;
				cap.Valid[1] = 0;
				cap.Valid[2] = 0;
				cap.Valid[3] = 0;
				cap.Valid[4] = 0;
				cap.Valid[5] = 0;
				cap.Valid[6] = 0;
				
				MarshalCap(&r->mcb,&cap);
				MarshalString(&r->mcb,mcname);

GEPdebug ("volume /%s, pathname %s",vol->vol_name,mcname);

				
				PutMsg (&r->mcb);
					
				Free (r);
				Free (m);
				continue;	
			}
			/* send volume not loadable error */

GEPdebug ("volume /%s not loaded",vol->vol_name);

			ErrorMsg(&m->mcb,EC_Error+EG_Unknown+EO_Object);
			Free (m);
			continue;	
		}


		if ( !vol->size_known ) {

GEPdebug ("init volume info of volume /%s",vol->vol_name);

			if ( !init_volume_info(vol) ) {
				if ( tell_param && ( m->mcb.MsgHdr.FnRc == FC_GSP+
						SS_HardDisk+FG_Private+FO_Load ) ) {
					tell_param = FALSE;
					MarshalWord ( &msg, MAKE_TAPE_ERR );
					MarshalWord ( &msg, 0);
					MarshalWord ( &msg, 0);
					PutMsg (&msg);
				}
				Error (FSErr [InitVIFailed]);
				ErrorMsg(&m->mcb,EC_Error+EG_Invalid+EO_Object);
				break;
			}
			else
				vol->size_known = TRUE;
			if ( tell_param && ( m->mcb.MsgHdr.FnRc == FC_GSP+
					SS_HardDisk+FG_Private+FO_Load ) ) {
				tell_param = FALSE;
				MarshalWord ( &msg, MAKE_TAPE_OK );
				MarshalWord ( &msg, vol->cgs);
				MarshalWord ( &msg, vol->bpcg);
				PutMsg (&msg);
			}
		}

		if ( m->mcb.MsgHdr.FnRc == 
		     FC_GSP+SS_HardDisk+FG_Private+FO_Load )
		{
			Free ( m );
			continue;
		}


GEPdebug ("status of volume /%s:\n"
          "     number of volumes    : %d\n"
          "     number of this volume: %d\n"
          "     number of partitions : %d\n"
          "     raw (%d), loaded (%d), loadable (%d)\n"
          "     writeprotected (%d), formatted (%d)\n"
          "     size known (%d), filesystem found (%d), sync allowed (%d)\n"
          "     found bpcg %d, found cgs %d, used bpcg %d, used cgs %d\n"
          "     cg offset %d, minfree %d, volume full (%d)\n",
vol->vol_name,
vol->num_of_vols,
vol->volnum,
vol->num_of_parts,
vol->raw,
vol->loaded,
vol->loadable,
vol->writeprotected,
vol->hard_formatted,
vol->size_known,
vol->filesystem_found,
vol->sync_allowed,
vol->found_bpcg,
vol->found_cgs,
vol->bpcg,
vol->cgs,
vol->cgoffs,
vol->minfree,
vol->vol_full);
		
		/* fork a worker process for each request */
		unless( Fork(tworkerSS, tworker, 12, m, &info, vol) )
		{
			Error (FSErr [TWNotForked]);
			/* send an exception if process couldn't be forked */
			ErrorMsg(&m->mcb,EC_Error+EG_NoMemory);
			/* get a new message */
			goto lab1;
		}
		
	}


GEPdebug ("volume %d terminated",vol->volnum);



#if UnloadVerbose  /* OI 11 Dec 91 */
	if (tell_unload) {	/* if verbose mode tell termination */
		tell_unload = FALSE;
		InitMCB ( &msg, MsgHdr_Flags_preserve, term_port, NullPort,
			  FC_GSP+SS_HardDisk+FG_Private+FO_Unload);
		msg.Control = Control_V;
		msg.Data    = Data_V; 	   
		MarshalWord (&msg,unload_status);
		PutMsg (&msg);
	}
#endif	



	MyTask->Flags = old_flags;

	Delete(nte,NULL);	/* delete the name of the FileServer from the Name Table */
	Free(m);

	Signal ( &term_sem );		/* signal termination */
	
}

static void
tworker (MsgBuf *m,DispatchInfo *info, VD *vol)

/*
*  Dynamically created process to deal with FileServer requests.
*/

{
	ServInfo servinfo;
	word fncode = m->mcb.MsgHdr.FnRc;

#if DEBUG
	IOCCommon *req = (IOCCommon *) (m->mcb.Control);

DEBdebug ("	context [%d] >%s<\n"
          "	name    [%d] >%s<\n"
          "	next    [%d] >%s<\n",
          req->Context,
          (req->Context>=0)?m->mcb.Data+req->Context:"-1",
          req->Name,
          (req->Name>=0)?m->mcb.Data+req->Name:"-1",
          req->Next,
          (req->Next>=0)?m->mcb.Data+req->Next:"-1");
#endif

	if( setjmp(servinfo.Escape) != 0 )
	{
		Free(m);
		return;
	}

	servinfo.m         = &m->mcb;
	MachineName          (servinfo.Context);
	pathcat              (servinfo.Context, vol->vol_name);
	servinfo.FnCode    = fncode;
	m->mcb.MsgHdr.FnRc = info->subsys;	

	if( !tget_context (&servinfo,vol) ) 
		ErrorMsg (&m->mcb,0);
	else	
	{
		word fn = fncode & FG_Mask;
   		VoidFnPtr f;

#if NAMES
Report (FSErr [OnlineDebug2],servinfo.Context);
Report (FSErr [OnlineDebug3],servinfo.Pathname);
Report (FSErr [OnlineDebug4],servinfo.Target);
#endif

		/* if request message is not valid */
		if( fn < FG_Open || fn > FG_CloseObj )
		{
			WordFnPtr f = info->PrivateProtocol;
			/* send exception message */
			if( (f==NULL) || (!f(&servinfo)) ) 
			{
				Error (FSErr [UnknownFC]);
				m->mcb.MsgHdr.FnRc = Err_Null;
				ErrorMsg(&m->mcb,EC_Error+info->subsys+EG_FnCode );
			}
		}
		/* else jump to the routine which deals this request */
		else 
		{
			Signal (&vol->dircnt_sem);	
						
FLdebug ("Calling function #0x%x", fn);			
			f = info->fntab[(fn-FG_Open) >> FG_Shift];
			(*f)(&servinfo,vol);
FLdebug ("Done");			

			if ( servinfo.FnCode != FG_Open )
				Wait (&vol->dircnt_sem);
		}
	}
	Free( m );
}

static int tget_context(servinfo, vol)
ServInfo *servinfo;
VD *vol;
{
	MCB         *m          = servinfo->m;
	IOCCommon   *req        = (IOCCommon *)(m->Control);
	int          context    = req->Context;
	int          next       = req->Next;
	int          name       = req->Name;
	char        *pathname   = servinfo->Pathname;
	
/* OI 12 Feb 1992: like get_context (fservlib.c), initialise pathname */
	strcpy(pathname,servinfo->Context);	

	if( context == -1 || (name >0 && next >= name) )
	{
		req->Access.Access = UpdMask(req->Access.Access, 
						vol->tape->tapeinode.ii_i.i_matrix);
		return TRUE;
	}
	else
	{	if( !GetAccess(&req->Access,
				(Key)vol->tape->tapeinode.ii_i.i_cryptkey) )
		{
			m->MsgHdr.FnRc |= EC_Error+EG_Invalid+EO_Capability;
			*(servinfo->Context) = '\0';
			return FALSE;
		}
		if( req->Access.Access == 0 )
		{
			m->MsgHdr.FnRc |= EC_Error+EG_Protected;
			*(servinfo->Context) = '\0';
			return FALSE;
		}
		strcpy(pathname,servinfo->Context);
		return TRUE;
	}
}

static void tdo_open(servinfo, vol)
ServInfo *servinfo;
VD *vol;
{	
	MCB 	*m        = servinfo->m;
	MsgBuf 	*r;
	Port 	 reqport;
	char 	*data     = m->Data;
	char 	*pathname = servinfo->Pathname;
	bool	 CloseReqWhileWrite;
	
FLdebug ("si->Pn (%s)  pn", servinfo->Pathname, pathname);
	
FLdebug ("Wait tdo_open");
	Wait(&vol->tape->tapeinode.ii_sem);
FLdebug ("Waited tdo_open");

	if( vol->tape->tapeinode.ii_count )
	{
Hdebug ("tape is already blocked!");

		ErrorMsg(m,EC_Error+EG_InUse+EO_Stream);
		Signal(&vol->tape->tapeinode.ii_sem);
		Wait (&vol->dircnt_sem);
		return;
	}

Hdebug ("tdo_op (start): Count 1;");					
	vol->tape->tapeinode.ii_count = 1;
	Signal(&vol->tape->tapeinode.ii_sem);

	r = New(MsgBuf);
	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Message);
		Wait (&vol->dircnt_sem);
		return;
	}

FLdebug ("<IOCRep1, pathname (%s)", pathname);
	IOCRep1(r,m,&vol->tape->tapeinode,Flags_Server+Flags_Closeable,pathname);
	reqport = DebNewPort();
	r->mcb.MsgHdr.Reply = reqport;
	PutMsg(&r->mcb);
FLdebug (">PutMsg", pathname);
	Free(r);

	Wait (&vol->dircnt_sem);
	Signal (&vol->streamcnt_sem);
	Signal (&vol->streammsg_sem);
	
	forever
	{	
		word e;
 	
		m->MsgHdr.Dest = reqport;
		m->Timeout = StreamTimeout;
		m->Data    = data;

/*FLebug ("Waiting (forever?)");*/
		Wait (&vol->streammsg_sem);
/*FLdebug ("Waited  (!forever)");*/
		
		/* get stream message and check its validity */

FLdebug ("Volume /%s, waiting 4 message", vol->vol_name);
		e = GetMsg(m);

FLdebug ("Msg: 0x%x", e);

		if (TestSemaphore (&vol->servreq_sem) < 1) {
			while (	(TestSemaphore (&vol->servreq_sem) < 1) &&
				vol->terminate_flag == FALSE)
				Delay (OneSec);
			if ( (TestSemaphore (&vol->servreq_sem) < 1) &&
				vol->terminate_flag == TRUE) 
			{
				m->MsgHdr.FnRc &= ~FG_Mask;
				m->MsgHdr.FnRc |= FG_Close;
				e = Err_Null;

GEPdebug ("Volume /%s, FG_Close constructed!",vol->vol_name);

			}
			
		}		
		Signal (&vol->streammsg_sem);
		
		if( MyTask->Flags & Task_Flags_servlib )
		{
			if( e < Err_Null )
				Error (FSErr [StreamGetMsg], e);
			else
		 		if( MyTask->Flags & Task_Flags_servlib )
					Report (FSErr [OnlineDebug5], m->MsgHdr.FnRc);
		}
		if( e == EK_Timeout ) break;
		if( e < Err_Null ) continue;
		
		do
		{
			CloseReqWhileWrite = FALSE;
			switch(m->MsgHdr.FnRc & FG_Mask )
			{
				case FG_Read:
				{
GEPdebug ("FG_Read request");
					do_tape_read(m,&vol->tape->tapeinode);
				}
				break;
		
				case FG_Write:
				{
#if 0
int DELAYTIME = OneSec;	
/* FIXME */
Hdebug ("FG_Write request");
Hdebug ("Delay (%d)", DELAYTIME);
Delay (DELAYTIME);
#endif

GEPdebug ("FG_Write request");

					if (vol->writeprotected)
					{
						ErrorMsg(m,EC_Error+EG_Protected+EO_File);
						break;
					}
					if (CloseReqWhileWrite = (do_tape_write (m,&vol->tape->tapeinode) == FG_Close))
					{
						Report (FSErr [CloseWhileWrite], vol->vol_name);
						m->MsgHdr.FnRc &= ~FG_Mask;
						m->MsgHdr.FnRc |= FG_Close;
					}
				}
				break;
		
				case FG_Close:
				{
GEPdebug ("FG_Close request");
					Report (FSErr [RewindTape], vol->vol_name);
					if( m->MsgHdr.Reply != NullPort )
					{
						ErrorMsg(m,Err_Null);
					}
					
					do_dev_seek(FG_Seek,vol->volnum,-1, 0, 0);

					vol->tape->tape_position=0;
					if( vol->tape->tapebuf )
					{
						Free(vol->tape->tapebuf);
					}
					vol->tape->tapebuf = NULL;
					vol->tape->tapebufsize = 0;
Hdebug ("FG_Close: Count 0;");					
					vol->tape->tapeinode.ii_count = 0;
					DebFreePort(reqport);
					Wait (&vol->streamcnt_sem);
					Wait (&vol->streammsg_sem);
					Report (FSErr [CloseTape], vol->vol_name);
					return;
				}
				break;
		
				case FG_GetSize:
				{
GEPdebug ("FG_GetSize request");
					InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
/*Hdebug ("GetSize: 10KB");*/
					MarshalWord(m,0);
FLdebugMCB(m);					
					PutMsg(m);
				}
				break;
		
 				case FG_SetSize:
				{
GEPdebug ("FG_SetSize request");
					InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
					MarshalWord(m,0);
					PutMsg(m);
				}
				break;
		
				case FG_Seek:
				{
GEPdebug ("FG_Seek request");
					do_tape_seek(m,&vol->tape->tapeinode);
				}
				break;
		
				default:
				{
FLdebug ("ErrorMsg (%x)", EC_Error+EG_FnCode+EO_File);
					ErrorMsg(m,EC_Error+EG_FnCode+EO_File);
				}
				break;
			}
		}
		while (CloseReqWhileWrite);
FLdebug (">while");		
	}
	if (vol->tape->tapebuf)
	{
		Free(vol->tape->tapebuf);
		FLdebug (">Free");		
	}
	vol->tape->tapebuf = NULL;
	vol->tape->tapebufsize = 0;
Hdebug ("tdo_op (<ret): Count 0;");					
	vol->tape->tapeinode.ii_count = 0;
	DebFreePort(reqport);
FLdebug ("<Wait1");		
	Wait (&vol->streamcnt_sem);
FLdebug ("<Wait2");		
	Wait (&vol->streammsg_sem);
FLdebug ("<ret");		
	return;
}
	
static void tdo_create(servinfo, vol)
ServInfo *servinfo;
VD *vol;
{	MsgBuf 		*r;
	MCB 		*m = servinfo->m;
	char 		*pathname = servinfo->Pathname;

	r = New(MsgBuf);
	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Message);
		return;
	}
	IOCRep1(r,m,&vol->tape->tapeinode,Flags_Server,pathname);
	PutMsg(&r->mcb);
	Free(r);
}

static void tdo_locate(servinfo, vol)
ServInfo *servinfo;
VD *vol;
{
	MsgBuf 		*r;
	MCB 		*m = servinfo->m;
	char		*pathname = servinfo->Pathname;
	
	r = New(MsgBuf);
	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Message);
		return;
	}
	strcpy(pathname,servinfo->Context);

	IOCRep1(r,m,&vol->tape->tapeinode,Flags_Server,pathname);
	PutMsg(&r->mcb);
	Free(r);
}

static void tdo_objinfo(servinfo, vol)
ServInfo *servinfo;
VD *vol;
{	
	MCB 		*m = servinfo->m;
	
	marshal_info(m,&vol->tape->tapeinode);
	PutMsg(m);
}

static void tdo_serverinfo(servinfo, vol)
ServInfo *servinfo;
VD *vol;
{	
	word 		d = Flags_Server;
	word 		z = 0;
	MCB 		*m = servinfo->m;
	
	InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
	MarshalData(m,4,(byte *)&d);
	MarshalData(m,4,(byte *)&z);
	MarshalData(m,4,(byte *)&z);
	MarshalData(m,4,(byte *)&z);
	PutMsg(m);
	
	vol = vol;              /* just to please the compiler */
}

static void tdo_delete (servinfo, vol)
ServInfo *servinfo;
VD *vol;
{	
	MCB *m = servinfo->m;

	ErrorMsg(m,EC_Error+EG_Delete+EO_File);

	vol = vol;              /* just to please the compiler */
}

static void tdo_rename(servinfo, vol)
ServInfo *servinfo;
VD *vol;
{
	MCB *m = servinfo->m;

	ErrorMsg(m,EC_Error+EG_WrongFn+EO_File);

	vol = vol;              /* just to please the compiler */
}

static void tdo_link(servinfo, vol)
ServInfo *servinfo;
VD *vol;
{
	MCB *m = servinfo->m;

	ErrorMsg(m,EC_Error+EG_WrongFn+EO_File);

	vol = vol;              /* just to please the compiler */
}

static void tdo_protect(servinfo, vol)
ServInfo *servinfo;
VD *vol;
{	
	MCB *m = servinfo->m;

	ErrorMsg(m,EC_Error+EG_WrongFn+EO_File);

	vol = vol;              /* just to please the compiler */
}

static void tdo_setdate(servinfo, vol)
ServInfo *servinfo;
VD *vol;
{	
	MCB *m = servinfo->m;

	ErrorMsg(m,EC_Error+EG_WrongFn+EO_File);

	vol = vol;              /* just to please the compiler */
}

static void tdo_refine(servinfo, vol)
ServInfo *servinfo;
VD *vol;
{	
	MCB *m = servinfo->m;

	ErrorMsg(m,EC_Error+EG_WrongFn+EO_File);

	vol = vol;              /* just to please the compiler */
}

static void tdo_closeobj(servinfo, vol)
ServInfo *servinfo;
VD *vol;
{	
	Error (FSErr [NotImpl], vol->vol_name);
	NullFn(servinfo);

	vol = vol;              /* just to please the compiler */
}

/************************************************
*						*
*	Real Tape access starts here		*
*						*
************************************************/

int ensure_tape_buffer(word reqsize, VD *vol)
{
	
	if ( reqsize > MAX_BUFFER_SIZE ) 
		return 0;		
	if ( vol->tape->tapebufsize >= reqsize )
		return 1;
	if ( vol->tape->tapebuf )
		Free (vol->tape->tapebuf);

	vol->tape->tapebuf = Malloc(reqsize);
	if ( vol->tape->tapebuf )
	{

GEPdebug ("buffer of %d bytes allocated",reqsize);

		vol->tape->tapebufsize  = reqsize;
		memset (vol->tape->tapebuf,0,reqsize);		
		return 1;
	}
	else
		return 0;
}


static word read_tape(int drive, char *buf, word size)
{	
	word r;
	word err;

Hdebug ("<req");
	r = do_dev_rdwr(FG_Read,
			volume[drive].logic_partition[0].partnum,
			OneSec * 10,
			size,
			volume[drive].tape->tape_position,
			(char *)buf,
			&err);
Hdebug (">req");
	if( MyTask->Flags & Task_Flags_servlib )
		Report (FSErr [OnlineDebug6], r*addr_to_block (BSIZE), drive);

Hdebug (FSErr [OnlineDebug6], r*addr_to_block (BSIZE), drive);
Hdebug ("Err (%x)", err);

	volume[drive].tape->tape_position += r;
	if (err)
		return(-1);
	else
		return (r);
}


static void do_tape_read(MCB *m, struct incore_i *tapei)
{	
	ReadWrite *req = (ReadWrite *)m->Control;
	Port reply = m->MsgHdr.Reply;
	word reqsize = req->Size;
	word reqpos = req->Pos;
	word dataread;
	word sent = 0;
	word seq = 0;
	word thissend;
	word firstsend = 1;
	MCB	     ErrorMCB;

	VD *vol = &volume[tapei->ii_dev];
	
	if( MyTask->Flags & Task_Flags_servlib )
		Report (FSErr [OnlineDebug7], reqsize);

	m->MsgHdr.FnRc = 0;

        InitMCB (&ErrorMCB, 
                m->MsgHdr.Flags, 
                m->MsgHdr.Dest, 
                m->MsgHdr.Reply, 
                m->MsgHdr.FnRc);

	if ( reqsize % addr_to_block(BSIZE) )
	{
FLdebug ("BadPar2, reqsize (%x), addr_to_block(%x) = %x", reqsize, BSIZE, addr_to_block (BSIZE));
		Error (FSErr [ReqMultAddr], addr_to_block (BSIZE));
		ErrorMsg (&ErrorMCB, EC_Error|EG_Parameter|2);
		return;
	}
	if ( reqpos % addr_to_block(BSIZE) )
	{
Hdebug ("BadPar1, reqpos (%x), addr_to_block(%x) = %x", reqpos, BSIZE, addr_to_block (BSIZE));
		Error (FSErr [PosMultAddr], addr_to_block (BSIZE));
		ErrorMsg(&ErrorMCB, EC_Error|EG_Parameter|1);
		return;
	}
	
/* Ensure that I can fit all of the request into my current buffer */
	if( !ensure_tape_buffer(reqsize,vol) )
	{
		ErrorMsg(m,EC_Error|EG_NoMemory|EO_Message);
		return;
	}
Hdebug ("Buffer ok");		

	reqpos = reqpos / addr_to_block(BSIZE);
	reqsize = reqsize / addr_to_block(BSIZE);

	if ( reqpos != vol->tape->tape_position )
	{

GEPdebug ("position error! req pos. %d, act pos. %d",reqpos,vol->tape->tape_position);

FLdebug ("BadPar");		
		ErrorMsg(&ErrorMCB, EC_Error|EG_Parameter|1);
		return;
	}

	InitMCB(m,MsgHdr_Flags_preserve,reply,NullPort,ReadRc_More);
	
Hdebug ("<r");
	dataread = read_tape(tapei->ii_dev, vol->tape->tapebuf, reqsize);
Hdebug (">r (%d)", dataread);
	if (dataread < 0)
	{
		ErrorMsg(&ErrorMCB, EC_Error+EG_Broken+EO_Medium);
		return;	
	}
	dataread *= addr_to_block(BSIZE);	
	
	while( (sent < dataread) || firstsend )
	{	
		word e;

		firstsend = 0;
		thissend = imin(dataread-sent,MAX_DATA_SIZE);
		m->Data = vol->tape->tapebuf+sent;
		m->MsgHdr.DataSize = thissend;

		if( (thissend + sent) == dataread )
		{	
			m->MsgHdr.Flags = 0;	/* No preserve */
			m->MsgHdr.FnRc  = ReadRc_EOD+seq;
		}
		else
			m->MsgHdr.FnRc  = ReadRc_More+seq;

		if( MyTask->Flags & Task_Flags_servlib )
			Report (FSErr [OnlineDebug8], thissend, m->MsgHdr.FnRc);

GEPdebug ("send %d bytes to client",thissend);

		e = PutMsg(m);
FLdebug (">send: e (%x)", e);		
		seq  += ReadRc_SeqInc;
		sent += thissend;
	}
}


static word write_tape(int drive, char *buf, word size)
{	
	word r;
	word err;

	if( MyTask->Flags & Task_Flags_servlib )
		Report (FSErr [OnlineDebug9], r*addr_to_block (BSIZE), drive);

/* FIXME */
FLdebug ("<do_dev_rdwr");		
	r = do_dev_rdwr(FG_Write,
			volume[drive].logic_partition[0].partnum,
			OneSec * 10,
			size,
			volume[drive].tape->tape_position,
			(char *)buf,
			&err);

/* FIXME */
FLdebug (">do_dev_rdwr, r (%d), err (%x)", r, err);
			
	if( MyTask->Flags & Task_Flags_servlib )
		Report (FSErr [OnlineDebug10], r*addr_to_block (BSIZE), drive);

/*FIXME*/
FLdebug (FSErr [OnlineDebug10], r*addr_to_block (BSIZE), drive);

	volume[drive].tape->tape_position += r;			
	if (err) 
	{
/* FIXME */
FLdebug ("return (-1)");
		return (-1);	
	}
	else
	{
/* FIXME */
FLdebug ("return r (%x)", r);
		return (r);
	}

}


static word do_tape_write(MCB *m, struct incore_i *tapei)
{	
	ReadWrite   *req        = (ReadWrite *)m->Control;
	Port 	     request	= m->MsgHdr.Dest;
	Port 	     reply      = m->MsgHdr.Reply;
	word 	     reqsize    = req->Size;
	word         reqpos     = req->Pos;
	word 	     written    = 0;
	word 	     msgdata    = m->MsgHdr.DataSize;
	MCB	     ErrorMCB;
	VD 	    *vol        = &volume[tapei->ii_dev];	

/*FIXME*/
FLdebug ("m (%x) m->MsgHdr->Rep (%x)  reply (%x)",m,m->MsgHdr.Reply, reply);

	m->MsgHdr.FnRc = 0;

        InitMCB (&ErrorMCB, 
                m->MsgHdr.Flags, 
                m->MsgHdr.Dest, 
                m->MsgHdr.Reply, 
                m->MsgHdr.FnRc);

	if( MyTask->Flags & Task_Flags_servlib )
	{
		Report (FSErr [OnlineDebug11], req->Pos, req->Size);
	}

	if ( reqsize % addr_to_block(BSIZE) )
	{
		Error (FSErr [ReqMultAddr], addr_to_block(BSIZE));
Hdebug ("BadPar2");
		ErrorMsg(&ErrorMCB, EC_Error|EG_Parameter|2);
		return (Err_Null);
	}
	if ( reqpos % addr_to_block(BSIZE) )
	{
		Error (FSErr [PosMultAddr], addr_to_block(BSIZE));
Hdebug ("BadPar 1a: reqpos (%x) addr_to_block (%x) = %x  r % a2b (%x)", 
			    reqpos, 	       BSIZE, addr_to_block (BSIZE), reqpos % addr_to_block (BSIZE));
		ErrorMsg(&ErrorMCB, EC_Error|EG_Parameter|1);
		return (Err_Null);
	}
	
	if( !ensure_tape_buffer(reqsize,vol) )
	{
		Error (FSErr [BuffAllocFailed]);
		ErrorMsg(&ErrorMCB, EC_Error|EG_NoMemory|EO_Message);
		return (Err_Null);
	}

GEPdebug ("buffer ok");

	reqpos = reqpos / addr_to_block(BSIZE);
	reqsize = reqsize / addr_to_block(BSIZE);

	if( reqpos != vol->tape->tape_position )
	{
		if( MyTask->Flags & Task_Flags_servlib )
		{
			Report (FSErr [OnlineDebug12], vol->tape->tape_position);
		}
Hdebug ("BadPar 1b, reqpos (%x) reqsize (%x)", reqpos, reqsize);
		ErrorMsg (&ErrorMCB, EC_Error|EG_Parameter|1);
		return (Err_Null);
	}


GEPdebug ("tape position ok");


	InitMCB(m,MsgHdr_Flags_preserve,reply,NullPort,WriteRc_Sizes);
/*FIXME*/
FLdebug ("m (%x) m->MsgHdr->Rep (%x)  reply (%x)",m,m->MsgHdr.Reply, reply);

	if( msgdata == 0 )
	{
		word e;
		word gotdata = 0;
		word trnsfr_size = addr_to_block(BSIZE);

		MarshalWord(m,imin(trnsfr_size,reqsize*addr_to_block(BSIZE)));
		MarshalWord(m,trnsfr_size);
		e = PutMsg(m);
		
		m->MsgHdr.Dest = request;
		m->Timeout = WriteTimeout;

GEPdebug ("getting data");

		while( gotdata < (reqsize*addr_to_block(BSIZE)) )
		{
			m->Data = vol->tape->tapebuf+gotdata;
			e = GetMsg(m);
/*
			if (e != Err_Null)
			{			
FLdebug ("Error while getting data (0x%x)", e);
				ErrorMsg (&ErrorMCB, EC_Error|EG_Broken|EO_Medium);
				return (e);
			}
*/			
/*
			if (e == FG_Close)
			{			
FLdebug ("Received FG_Close while getting data.");
				ErrorMsg (&ErrorMCB, EC_Error|EG_Broken|EO_Medium);
				return (e);
			}
*/
			gotdata += m->MsgHdr.DataSize;
		}
/*FIXME*/		
FLdebug ("<w1");
		written = write_tape(tapei->ii_dev,vol->tape->tapebuf,reqsize);
/*FIXME*/		
FLdebug (">w1 (%d)", written);

		if (written < 0) {
			ErrorMsg (&ErrorMCB, EC_Error|EG_Broken|EO_Medium);
/*FIXME*/		
FLdebug ("Returning %d", Err_Null);			
			return (Err_Null);
		}
	}
	else {	/* No copies here ! */

/*FIXME*/		
FLdebug ("<w2");

		written = write_tape(tapei->ii_dev,m->Data,reqsize);
/*FIXME*/		
FLdebug (">w2 (%d)", written);

		if (written < 0) {
			ErrorMsg (&ErrorMCB, EC_Error|EG_Broken|EO_Medium);
/*FIXME*/		
FLdebug ("Returning %d", Err_Null);			
			return (Err_Null);
		}
	}

	if( MyTask->Flags & Task_Flags_servlib )
		Report (FSErr [OnlineDebug13], written * addr_to_block (BSIZE));

	InitMCB(m,0,reply,NullPort,WriteRc_Done);
	MarshalWord(m,written * addr_to_block(BSIZE));
	PutMsg(m);
	return (Err_Null);
}


static void do_tape_seek(MCB *m, struct incore_i *tapei)
{	
	SeekRequest *req = (SeekRequest *)m->Control;
	word reqpos = req->NewPos;
	word newpos;
	Port reply = m->MsgHdr.Reply;
	word err=Err_Null;

	if( MyTask->Flags & Task_Flags_servlib )
		Report (FSErr [OnlineDebug14], req->Mode, req->NewPos);

	m->MsgHdr.FnRc = 0;
	
	if ( reqpos % addr_to_block(BSIZE) )
	{
		Error (FSErr [PosMultAddr], addr_to_block (BSIZE));
Hdebug ("BadPar1");		
		ErrorMsg (m,EC_Error|EG_Parameter|1);
		return;
	}
	
	reqpos = reqpos / addr_to_block(BSIZE);
	
	newpos = do_dev_seek(FG_Seek,
			     volume[tapei->ii_dev].logic_partition[0].partnum,
			     -1,
			     0,
			     reqpos);
	if( MyTask->Flags & Task_Flags_servlib )
		Report (FSErr [OnlineDebug15], newpos);
	if( newpos < 0 ) err = newpos,newpos = -1;
	
	InitMCB(m,0,reply,NullPort,err);
	MarshalWord(m,newpos * addr_to_block(BSIZE));
	PutMsg(m);
}
