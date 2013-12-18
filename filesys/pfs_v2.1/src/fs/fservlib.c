/* $Header: /hsrc/filesys/pfs_v2.1/src/fs/RCS/fservlib.c,v 1.1 1992/07/13 16:17:41 craig Exp $ */

/* $Log: fservlib.c,v $
 * Revision 1.1  1992/07/13  16:17:41  craig
 * Initial revision
 *
 * Revision 2.2  90/09/05  07:03:43  guenter
 * format command added, error messages added
 * 
 * Revision 2.1  90/08/31  11:04:34  guenter
 * first multivolume/multipartition PFS with tape
 * 
 * Revision 1.7  90/08/08  08:03:57  guenter
 * ParentDir of RootDir is no more RootDir itself
 * Absolut_access instead of AccMask_Z handling, '..'
 * 
 * Revision 1.6  90/07/24  18:04:20  adruni
 * UpdMask with AccMask_Z for '..'
 * 
 * Revision 1.5  90/05/30  15:39:04  chris
 * Fix application of 1.3
 * and pad out name to NameMax with NULL's
 * 
 * Revision 1.4  90/01/26  13:51:14  chris
 * Release parent directory when handling links
 * 
 * Revision 1.3  90/01/12  19:05:42  chris
 * Fix use of signals in make_obj
 * 
 * Revision 1.2  90/01/03  13:42:14  chris
 * Correct return code for file not found
 * 
 * Revision 1.1  90/01/02  19:02:55  chris
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
   |  fservlib.c							     |
   |                                                                         |
   |	Routines of the FileServer task.		 		     |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |	5 - NHG       - 12 June    1989 - Protection mechanism enabled	     |
   |    4 - H.J.Ermen - 22 March   1989 -     FO_Synch			     |
   |					      FO_Asynch			     |
   |					      FO_ForceSync		     |
   |	3 - H.J.Ermen -	20 March   1989 - Investigations to find out, if     |
   |					  a private message is received.     |
   |					  Private server requests implem.:   |
   |					      FO_Terminate		     |
   |    2 - H.J.Ermen - 19 January 1989 - Marshallling of link-type data     |
   |				          in marshal_info()                  |
   |    1 - A.Ishan   - 10 October 1988 - Basic version                      |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#define DEBUG	   0
#define GEPDEBUG   0
#define FLDEBUG	   0
#define HDEBUG	   1
#define NAMES	   0
#define MASK	   0
#define DSERV	   0
#define MATRIX	   0
#define IN_NUCLEUS 1

#include "error.h"

#define PROCCNT    1
#include "proccnt.h"

#include "misc.h"
#include "nfs.h"

#define fworkerStackSize 	20000
#define KeepOpenStackSize	10000
#define editStackSize 		10000

#define MAX_RETRIES	60	/* time to wait for termination of stream */
				/* workers during an unload or termvol in */
				/* seconds */   


Semaphore sync_sem;

/* ==================================================================== */
 
static void fworker(MsgBuf *m,DispatchInfo *info, VD *vol);
static void Crypt(bool encrypt, Key key, byte *data, word size);

/* ==================================================================== */

#if 0

void KeepOpen (MsgBuf    **MB,
               Semaphore  *KeepOpenSem)
{
  MCB   DummyMsg;
  word  ControlV [IOCMsgMax];
  byte  DataV    [IOCDataMax];
	
  forever
  {
		FLdebug ("KeepOpen: Waiting...");
    DebWait (KeepOpenSem);
		FLdebug ("KeepOpen: Waited...");
    /* Talk to format utility */
    Delay (60 * OneSec);
		FLdebug (">Delay, Preparing");		
		FLdebug ("MB (0x%X)   *MB (0x%X)  (*MB)->mcb.MsgHdr.Reply (0x%X)",
		          MB,         *MB,        (*MB)->mcb.MsgHdr.Reply);
    InitMCB (&DummyMsg, MsgHdr_Flags_preserve, 
             (*MB)->mcb.MsgHdr.Reply, NullPort,
             FC_GSP + SS_HardDisk + FG_Private + FO_Format);
    DummyMsg.Control = ControlV;
    DummyMsg.Data    = DataV;
    MarshalWord (&DummyMsg, FORMAT_KEEP_OPEN);
    MarshalWord (&DummyMsg, 0);
    MarshalWord (&DummyMsg, 0);
    PutMsg (&DummyMsg);
    DebSignal (KeepOpenSem);
FLdebug (">PutMsg Dummy");
  }
}				

#endif
			
void 
fdispatch (VD *vol)

/*
*  Receives the request messages to FileServer at the Server Port,
*  creating 'fworker' processes to perform each request.
*/

{
	MsgBuf		*m;
	MCB		temp_mcb,msg;
	Object		*nte;
	Port		term_port;	
	word		FormatSuccess,
			error,
			retries,
			allow_make, 
			checker_local, 
			tell_param,
			delete_hanging_links,
			tell_unload, 
			unload_status;
	load_data	*load_control, *unload_control;
	NameInfo	FServerInfo;
	DispatchInfo 	info;
	word		Control_V[IOCMsgMax];
	byte		Data_V[IOCDataMax];
	word		Err_Control_V[IOCMsgMax];
	byte		Err_Data_V[IOCDataMax];
	char		mcname[PATH_MAX];
	word		flags;
	Capability	cap;
	MsgBuf		*r;
	Semaphore	AvoidPortTimeOut;
		
	FServerInfo.Port 	= info.reqport = DebNewPort ();
	FServerInfo.Flags 	= Flags_StripName;
	FServerInfo.Matrix 	= DefDirMatrix;
	FServerInfo.LoadData 	= (word *) NULL;
	
	info.root		= NULL;
	info.subsys		= SS_HardDisk;
	info.PrivateProtocol	= NULL;
	info.fntab[0]		= do_open;
	info.fntab[1]		= do_create;
	info.fntab[2]		= do_locate;
	info.fntab[3]		= do_objinfo;
	info.fntab[4]		= do_serverinfo;
	info.fntab[5]		= do_delete;
	info.fntab[6]		= do_rename;
	info.fntab[7]		= do_link;
	info.fntab[8]		= do_protect;
	info.fntab[9]		= do_setdate; 
	info.fntab[10]		= do_refine;
	info.fntab[11]		= do_closeobj;
	
/*
	InitSemaphore (&AvoidPortTimeOut, 1 - 1);
	Fork (KeepOpenStackSize, 
	      KeepOpen, 
	      RoundTo (sizeof (&m) + sizeof (&AvoidPortTimeOut), sizeof (word)),
	                       &m,           &AvoidPortTimeOut);
*/

	{
		Object *o;

		MachineName(mcname);	/* build machine name */
		o = Locate(NULL,mcname);
		nte = Create(o,vol->vol_name,Type_Name,sizeof(NameInfo),(byte *)&FServerInfo);	/* take an entry in the processors Name Table */ 
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
	vol->syncwrite = fsi->SyncOp;
	vol->loaded = FALSE;

	error = 0;

	IncPC (10);

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

GEPdebug (" fdispatch : status0 of volume /%s:\n"
	  "     number of volumes: %d\n"
	  "       number of this volume : %d\n"
	  "       number of partitions  : %d\n"
	  "       raw (%d), loaded (%d), loadable (%d)\n"
 	  "       writeprotected (%d), formatted (%d)\n"
	  "       size known (%d), filesystem found (%d), sync allowed (%d)\n"
	  "       found bpcg %d, found cgs %d, used bpcg %d, used cgs %d\n"
	  "       cg offset %d, minfree %d, volume full (%d)",
vol->vol_name,
vol->num_of_vols,
vol->volnum,
vol->num_of_parts,
vol->raw,vol->loaded,vol->loadable,
vol->writeprotected,vol->hard_formatted,
vol->size_known,vol->filesystem_found,vol->sync_allowed,
vol->found_bpcg,vol->found_cgs,vol->bpcg,vol->cgs,
vol->cgoffs,vol->minfree,vol->vol_full);

		/* get a request message */

GEPdebug (" fdispatch : volume /%s waiting for request message ...",vol->vol_name);

		while (GetMsg (&(m->mcb)) == EK_Timeout)
		  ;

FLdebug (" fdispatch : volume /%s received request 0x%08x",vol->vol_name,m->mcb.MsgHdr.FnRc);

 		if( MyTask->Flags & Task_Flags_servlib )
			Report ("PFS: %F %s %s",m->mcb.MsgHdr.FnRc,
 				(m->control[0]==-1)?NULL:&m->data[m->control[0]],
 				(m->control[1]==-1)?NULL:&m->data[m->control[1]]);

		if (TestSemaphore (&vol->servreq_sem) < 1) {
			ErrorMsg (&m->mcb,EC_Error+info.subsys+EG_Protected+EO_Server);

GEPdebug (" fdispatch : volume /%s, central server request port is locked!",vol->vol_name);

			Free (m);
			continue;
		}
		

/*------------- FO_Debug -------------------------------------------*/

		if (m->mcb.MsgHdr.FnRc == 
		     FC_GSP + SS_HardDisk + FG_Private + FO_Debug)
		{
			ReportPC ();
			InitMCB (&temp_mcb, MsgHdr_Flags_preserve,
				   m->mcb.MsgHdr.Reply, NullPort,
		           	   FC_GSP + SS_HardDisk + FG_Private + FO_Debug);
			PutMsg  (&temp_mcb);
			Free    (m);
			continue;
		}

/*-----------------------------------------------------------------*/

/*
 *   handle all requests for which the volume needs not to be loaded
 *	- synch
 *	- asynch
 *	- termfs
 *	- unload
 */

/*-----  FO_Synch and FO_Asynch determine the write operation-mode  ----*/

		if ( m->mcb.MsgHdr.FnRc == 
		     FC_GSP+SS_HardDisk+FG_Private+FO_Synch )
		{

GEPdebug ("fdispatch : Got a message to turn into synchronous mode");

			vol->syncwrite = TRUE;			
			Free ( m );
			continue;
		}
		
		if ( m->mcb.MsgHdr.FnRc == 
		     FC_GSP+SS_HardDisk+FG_Private+FO_Asynch )
		{

GEPdebug ("fdispatch : Got a message to turn into asynchronous mode");

			vol->syncwrite = FALSE;
			Free ( m );
			continue;
		}


/*--------------------  FO_Unload unloads volume  -----------------------*/
/*-------------  FO_Terminate forces file-server shutdown  --------------*/

		if ( (m->mcb.MsgHdr.FnRc == FC_GSP+SS_HardDisk+FG_Private+FO_Unload)
			|| (m->mcb.MsgHdr.FnRc == FC_GSP+SS_HardDisk+FG_Private+FO_Terminate ) )
		{

GEPdebug (" fdispatch : Unload/terminate volume /%s",vol->vol_name);

			DebWait (&vol->servreq_sem);
			term_port = m->mcb.MsgHdr.Reply; 
			unload_control = (load_data *) m->control;
			tell_unload = unload_control->verbose;
			
			InitMCB ( &msg, MsgHdr_Flags_preserve, term_port,
				   NullPort, m->mcb.MsgHdr.FnRc );
			PutMsg ( &msg );
			retries = 0;
			unload_status = MAKE_OK;
			if (vol->loaded) {
				vol->terminate_flag = TRUE;
				while (retries < MAX_RETRIES) {
					if (TestSemaphore(&vol->streammsg_sem) ) {
/*
Report (" fdispatch : Volume /%s has still (%d) active non-interuptable worker!",vol->vol_name,TestSemaphore(&vol->streammsg_sem));
Report ("             Unable to unload or terminate right now!");
*/
						retries++;
					}
					elif (TestSemaphore(&vol->dircnt_sem) ) {
/*
Report (" fdispatch : Volume /%s has still (%d) active directory worker!",vol->vol_name,TestSemaphore(&vol->dircnt_sem));
Report ("             Unable to unload or terminate right now!");
*/
						retries++;
					}
					else {
						break;	
					}
					Delay (OneSec);
				}
				if (retries >= MAX_RETRIES) {
Error ("Volume /%s forced unload with (%d) active stream worker"
       "and (%d) active directory worker.",
       vol->vol_name,
       TestSemaphore(&vol->streammsg_sem),
       TestSemaphore(&vol->dircnt_sem));
					unload_status = MAKE_ERR;
				}
				if ( vol->filesystem_found ) {
					vol->sync_allowed = FALSE;
					clean_cache (vol);
					free_root_inode (vol);
					clean_inodelist (vol);
				}
				unload_volume (vol);
			}
			if ( m->mcb.MsgHdr.FnRc == FC_GSP+SS_HardDisk+
					FG_Private+FO_Terminate ) {
				break;
			}
			else {
				DebSignal (&vol->servreq_sem);
			}
			if (tell_unload) {
				tell_unload = FALSE;
				InitMCB ( &msg, MsgHdr_Flags_preserve, term_port, NullPort,
					  FC_GSP+SS_HardDisk+FG_Private+FO_Unload);
				msg.Control = Control_V;
				msg.Data    = Data_V; 	   
				MarshalWord (&msg,unload_status);
				PutMsg (&msg);
			}
			Free ( m );
			continue;
		}

/*
 *   try to load volume
 */
		if ( m->mcb.MsgHdr.FnRc == FC_GSP+SS_HardDisk+FG_Private+FO_Load )
		{

FLdebug ("Load volume /%s",vol->vol_name);
			
			load_control = (load_data *) m->control;

			if ( load_control->touched )
				checker_local = load_control->checker_info;
			else
				checker_local = checker_mode;
			allow_make = load_control->make_info;
			tell_param = load_control->verbose;
			delete_hanging_links = load_control->delete_hanging_links;
			InitMCB ( &msg, MsgHdr_Flags_preserve, 
				  m->mcb.MsgHdr.Reply, NullPort,
				  FC_GSP+SS_HardDisk+FG_Private+FO_Load );
			msg.Control = Control_V;
			msg.Data    = Data_V; 	   
			PutMsg ( &msg );

			if (!vol->loaded ) { 	/* try to load if not loaded */

FLdebug ("Loading volume /%s",vol->vol_name);

				InitSemaphore (&vol->servreq_sem,1);
				InitSemaphore (&vol->dircnt_sem,0);
				InitSemaphore (&vol->streamcnt_sem,0);
				InitSemaphore (&vol->streammsg_sem,0);
				vol->terminate_flag = FALSE;
	
FLdebug ("< load_volume");
				error = load_volume (vol);
FLdebug ("> load_volume, error (%d)", error);
				if ( error < 0 )
				{
					Error ("Hardware error detected loading volume /%s.",vol->vol_name);
					ErrorMsg(&m->mcb,EC_Error+EG_Unknown+EO_Object);
					break;
				}	
				else if ( error ) {
					Error ("Device error detected loading volume /%s.",vol->vol_name);
					ErrorMsg(&m->mcb,EC_Error+EG_Unknown+EO_Object);
					break;
				}

				if ( !vol->loaded && vol->hard_formatted ) {

FLdebug (" fdispatch : Volume /%s not loadable",vol->vol_name);

					/* send volume not loadable error */
					ErrorMsg(&m->mcb,EC_Error+EG_Unknown+EO_Object);
					Free (m);
					continue;	
				}
			}
			
			if ( allow_make )
			{
				Free (m);
				continue;
			}
		}
		
		if ( !vol->loaded ) {	/* volume must be loaded here */

FLdebug ("Volume /%s not loaded",vol->vol_name);

			/* waitfor /fs in loginrc performs FG_Locate */

			if ( m->mcb.MsgHdr.FnRc == FG_Locate )
			{
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
				
				PutMsg (&r->mcb);
					
				Free (r);
				Free (m);
				continue;	
			}
			/* send volume not loaded error */
			ErrorMsg(&m->mcb,EC_Error+EG_Unknown+EO_Object);
			Free (m);
			continue;	
		}

/*
 *  handle all requests for which the volume has to be loaded but not to
 *  be hard formatted
 *	- format  ( medium must not be writeprotected )
 */

/*-------------  FO_PrivFormat formats volume physically  --------------*/

		if ( m->mcb.MsgHdr.FnRc == 
		     FC_GSP+SS_HardDisk+FG_Private+FO_Format )
		{

FLdebug ("Format volume /%s physically", vol->vol_name);

			InitMCB ( &msg, MsgHdr_Flags_preserve, 
				  m->mcb.MsgHdr.Reply, NullPort,
				  FC_GSP+SS_HardDisk+FG_Private+FO_Format );
			msg.Control = Control_V;
			msg.Data    = Data_V;
			PutMsg ( &msg );
			
			if ( vol->filesystem_found ) {
				MarshalWord (&msg, MAKE_HFS_ACTIVE);
				MarshalWord (&msg, 0);
				MarshalWord (&msg, 0);
				PutMsg ( &msg );
			}				
			else
			{
/*
				DebSignal (&AvoidPortTimeOut);
*/
				
				FormatSuccess = format_vol (vol);
				
/*
FLdebug ("Fdispatch: Waiting...");
				DebWait (&AvoidPortTimeOut);
FLdebug ("Fdispatch: Waited");
*/
				
				if (FormatSuccess)
				{
					MarshalWord (&msg, MAKE_OK);
					MarshalWord (&msg, 0);
					MarshalWord (&msg, 0);
					PutMsg ( &msg );
				}
				else {
					MarshalWord (&msg, MAKE_ERR);
					MarshalWord (&msg, 0);
					MarshalWord (&msg, 0);
					PutMsg ( &msg );
				}
			}
			Free (m);

/*
FLdebug ("formatting without Msg");
if ( !format_vol (vol) )
  FLdebug ("format OK");
else
  FLdebug ("format failed");
*/

			continue;
		}



		if ( !vol->hard_formatted ) {
			/* send a medium not physically formatted error */	
			ErrorMsg(&m->mcb,EC_Error+EG_Invalid+EO_Object);
			Error ("Volume /%s physically not formatted.",vol->vol_name);
			Free (m);
			continue;
		}


/*
 *   handle all requests for which the volume needs to be loaded, physically
 *   formatted, but still no filesystem need to be found
 *	- makefs	( medium must not be writeprotected ) 
 *	- mksuper	
 *	- editvol
 */

/*-------------  FO_Edit can be used to read and write blocks  ---------*/

		if ( m->mcb.MsgHdr.FnRc == 
		     FC_GSP+SS_HardDisk+FG_Private+FO_Edit )
		{
			if ( !vol->size_known) {
				if ( !init_volume_info(vol) ) {
Error ("Volume /%s failed to init size info.",vol->vol_name);
					ErrorMsg(&m->mcb,EC_Error+EG_Invalid+EO_Medium);
					Free (m);
					continue;
				}
			}
			if (!Fork(editStackSize, edit_block, 12, m, vol) ) {
Error ("Volume /%s failed to Fork () edit_block.",vol->vol_name);
				ErrorMsg(&m->mcb,EC_Error+EG_NoMemory+EO_Object);				
				Free (m);
				continue;
			}
			Free (m);
			continue;
		}

/*-------------  FO_MakeFs builds a filesystem on volume  --------------*/

		if ( m->mcb.MsgHdr.FnRc == 
		     FC_GSP+SS_HardDisk+FG_Private+FO_MakeFs )
		{

GEPdebug (" fdispatch : make_fs (volume /%s)",vol->vol_name);

			term_port = m->mcb.MsgHdr.Reply;
			InitMCB ( &msg, MsgHdr_Flags_preserve, term_port, NullPort,
				  FC_GSP+SS_HardDisk+FG_Private+FO_MakeFs);
			msg.Control = Control_V;
			msg.Data    = Data_V; 	   
			if ( !vol->filesystem_found && !vol->writeprotected ) {
				if ( !make_fs (vol) ) {
					MarshalWord (&msg, MAKE_ERR);
					MarshalWord (&msg, 0);
					MarshalWord (&msg, 0);
					PutMsg ( &msg );
					Free ( m );
					continue;
				}
				else {	
					MarshalWord (&msg, MAKE_OK);
					MarshalWord (&msg, vol->cgs);
					MarshalWord (&msg, vol->bpcg);
					PutMsg ( &msg );
				}
			}
			else {
				if ( vol->writeprotected )
					MarshalWord (&msg, MAKE_PROTECTED);
				else
					MarshalWord (&msg, MAKE_HFS_ACTIVE);
				MarshalWord (&msg, vol->cgs);
				MarshalWord (&msg, vol->bpcg);
				PutMsg ( &msg );
				Free ( m );
				continue;
			}	
		}


/*-------------  FO_MakeSuper builds a superblock on volume  --------------*/

		if ( m->mcb.MsgHdr.FnRc == 
		     FC_GSP+SS_HardDisk+FG_Private+FO_MakeSuper )
		{

GEPdebug (" fdispatch : make_super (volume /%s)",vol->vol_name);

			term_port = m->mcb.MsgHdr.Reply;
			InitMCB ( &msg, MsgHdr_Flags_preserve, term_port, NullPort,
				  FC_GSP+SS_HardDisk+FG_Private+FO_MakeSuper);
			msg.Control = Control_V;
			msg.Data    = Data_V; 	   
			if ( !vol->filesystem_found && !vol->writeprotected ) {
				if ( !make_super (vol) ) {
					MarshalWord (&msg, MAKE_ERR);
					MarshalWord (&msg, 0);
					MarshalWord (&msg, 0);
					PutMsg ( &msg );
					Free ( m );
					continue;
				}
				else {	
					MarshalWord (&msg, MAKE_OK);
					MarshalWord (&msg, vol->cgs);
					MarshalWord (&msg, vol->bpcg);
					PutMsg ( &msg );
				}
			}
			else {
				if ( vol->writeprotected )
					MarshalWord (&msg, MAKE_PROTECTED);
				else
					MarshalWord (&msg, MAKE_HFS_ACTIVE);
				MarshalWord (&msg, vol->cgs);
				MarshalWord (&msg, vol->bpcg);
				PutMsg ( &msg );
				Free ( m );
				continue;
			}	
		}


/*
 *   try to find a filesystem on disk equal to found devinfo information 
 */
 
		if ( !vol->filesystem_found ) {

GEPdebug (" fdispatch : init_fs (volume /%s)",vol->vol_name);

			DebWait (&checker_sem);
			if( ! init_fs(checker_local,delete_hanging_links,vol) ) {

GEPdebug (" fdispatch() : Volume /%s failed to init filesystem !",vol->vol_name);

				if ( tell_param && ( m->mcb.MsgHdr.FnRc == FC_GSP+
						SS_HardDisk+FG_Private+FO_Load ) ) {
					tell_param = FALSE;
					MarshalWord ( &msg, MAKE_ERR );
					MarshalWord ( &msg, 0);
					MarshalWord ( &msg, 0);
					PutMsg (&msg);
				}
				/* send no valid filesystem found error */	
				ErrorMsg(&m->mcb,EC_Error+EG_Invalid+EO_Object);
				Free (m);
				DebSignal (&checker_sem);
				continue;
			}
			DebSignal (&checker_sem);
			if ( tell_param && ( m->mcb.MsgHdr.FnRc == FC_GSP+
					SS_HardDisk+FG_Private+FO_Load ) ) {
				tell_param = FALSE;
				MarshalWord ( &msg, MAKE_OK );
				MarshalWord ( &msg, vol->cgs);
				MarshalWord ( &msg, vol->bpcg);
				PutMsg (&msg);
			}
		}

		if ( m->mcb.MsgHdr.FnRc == FC_GSP+SS_HardDisk+FG_Private+FO_MakeFs 
		    || m->mcb.MsgHdr.FnRc == FC_GSP+SS_HardDisk+FG_Private+FO_Load 
		    || m->mcb.MsgHdr.FnRc == FC_GSP+SS_HardDisk+FG_Private+FO_MakeSuper ) {		    
			Free (m);
			continue;    	
		} 
		
		

GEPdebug (" fdispatch : status2 of volume /%s:\n"
	  "     number of volumes: %d\n"
	  "       number of this volume : %d\n"
	  "       number of partitions  : %d\n"
	  "       raw (%d), loaded (%d), loadable (%d)\n"
 	  "       writeprotected (%d), formatted (%d)\n"
	  "       size known (%d), filesystem found (%d), sync allowed (%d)\n"
	  "       found bpcg %d, found cgs %d, used bpcg %d, used cgs %d\n"
	  "       cg offset %d, minfree %d, volume full (%d)",
vol->vol_name,
vol->num_of_vols,
vol->volnum,
vol->num_of_parts,
vol->raw,vol->loaded,vol->loadable,
vol->writeprotected,vol->hard_formatted,
vol->size_known,vol->filesystem_found,vol->sync_allowed,
vol->found_bpcg,vol->found_cgs,vol->bpcg,vol->cgs,
vol->cgoffs,vol->minfree,vol->vol_full);

		
/*
 *   handle all requests for which a proper filesystem has to be found
 */	
 	

/*------------- FO_ForceSync generate a sync_fs immediately ------------*/

		if ( m->mcb.MsgHdr.FnRc == 
		     FC_GSP+SS_HardDisk+FG_Private+FO_ForceSync )
		{

GEPdebug ("fdispatch : Got a message to force a sync-operation immediately");

			sync_vol (vol);
				/* Prepare termination message		 */
			InitMCB ( &temp_mcb, MsgHdr_Flags_preserve,
				   m->mcb.MsgHdr.Reply, NullPort, 0 );	
				/* Send the termination message to fstat */
			PutMsg ( &temp_mcb );
			Free ( m );
			continue;
		}


/*-------------  Now we have handled all private requests  --------------*/


GEPdebug (" fdispatch : volume /%s, forking a worker process",vol->vol_name);

		/* fork a worker process for each request */
		unless( Fork(fworkerStackSize, fworker, 12, m, &info, vol) )
		{
			/* send an exception if process couldn't be forked */
			ErrorMsg(&m->mcb,EC_Error+info.subsys+EG_NoMemory);
			/* get a new message */
			goto lab1;
		}

	}
	
	DecPC (10);
	
GEPdebug (" fdispatch : volume /%s terminated",vol->vol_name);

	if (tell_unload) {	/* if verbose mode tell termination */
		tell_unload = FALSE;
		InitMCB ( &msg, MsgHdr_Flags_preserve, term_port, NullPort,
			  FC_GSP+SS_HardDisk+FG_Private+FO_Unload);
		msg.Control = Control_V;
		msg.Data    = Data_V; 	   
		MarshalWord (&msg,unload_status);
		PutMsg (&msg);
	}

/* OI 910805 */
	DebFreePort (FServerInfo.Port);

	Delete(nte,NULL);	/* delete the name of the FileServer from the Name Table */
	Free(m);

	DebSignal ( &term_sem );		/* signal termination */

}

/* ==================================================================== */

static void
fworker (MsgBuf *m,DispatchInfo *info,VD *vol)

/*
*  Dynamically created process to deal with FileServer requests.
*/

{
	ServInfo 	servinfo;
	word		value;
	word 		fncode = m->mcb.MsgHdr.FnRc;

#if DEBUG
	IOCCommon 	*req = (IOCCommon *) (m->mcb.Control);

DEBdebug ("	context [%d] >%s<",req->Context, (req->Context>=0)?m->mcb.Data+req->Context:"-1");
DEBdebug ("	name    [%d] >%s<",req->Name, (req->Name>=0)?m->mcb.Data+req->Name:"-1");
DEBdebug ("	next    [%d] >%s<",req->Next, (req->Next>=0)?m->mcb.Data+req->Next:"-1");
#endif

	switch ( value = setjmp(servinfo.Escape) )
	{
		case 0  : break;
		
		case 2  : DebWait (&vol->dircnt_sem);
			  Free (m);
			  return;

		default : Free(m);
			  return;
	}

	servinfo.m            = &m->mcb;
	MachineName             (servinfo.Context);
	pathcat	                (servinfo.Context, vol->vol_name);
	servinfo.FnCode       = fncode;
	servinfo.ParentOfRoot = FALSE;
	m->mcb.MsgHdr.FnRc    = info->subsys;	

#if NAMES
DEBdebug ("fwCTXT : >%s<\n"
	  "fwPATH : >%s<\n"
	  "fwTRGT : >%s<",
	  servinfo.Context,
	  servinfo.Pathname,
	  servinfo.Target);
#endif

	if( !get_context (&servinfo,vol) ) 
		ErrorMsg (&m->mcb,0);
	else	
	{
		word fn = fncode & FG_Mask;
		VoidFnPtr f;

#if NAMES
DEBdebug ("fwCTXT : >%s<\n"
	  "fwPATH : >%s<\n"
	  "fwTRGT : >%s<",
	  servinfo.Context,
	  servinfo.Pathname,
	  servinfo.Target);
#endif

		/* if request message is not valid */
		if( fn < FG_Open || fn > FG_CloseObj )
		{
			WordFnPtr f = info->PrivateProtocol;
			/* send exception message */
			if( (f==NULL) || (!f(&servinfo)) ) 
			{
				m->mcb.MsgHdr.FnRc = Err_Null;
				ErrorMsg(&m->mcb,EC_Error+info->subsys+EG_FnCode );
			}
		}
		/* else jump to the routine which deals this request */
		else 
		{
			DebSignal (&vol->dircnt_sem);	
		
			f = info->fntab[(fn-FG_Open) >> FG_Shift];
			IncPC (11);
			IncPC (12 + ((fn-FG_Open) >> FG_Shift));
			(*f)(&servinfo,vol);
			DecPC (12 + ((fn-FG_Open) >> FG_Shift));
			DecPC (11);
			
			if ( servinfo.FnCode != FG_Open )
			{
				IncPC (73);
				DebWait (&vol->dircnt_sem);
				DecPC (73);
			}
		}
	}
	Free( m );
}

/* ==================================================================== */

word
get_context (ServInfo *servinfo, VD *vol)

{
	MCB 	  *m 	    = servinfo->m;
	IOCCommon *req 	    = (IOCCommon *)(m->Control);
	int 	   context  = req->Context;
	int 	   name     = req->Name;
	int 	   next     = req->Next;
	byte 	  *data     = m->Data;
	char 	  *pathname = servinfo->Pathname;

		
/*debug("GetContext %x %x",m->MsgHdr.FnRc,servinfo->FnCode);*/
	/* check that this is a GSP message */
	if ( m->MsgHdr.FnRc & FC_Mask != FC_GSP )
	{
		m->MsgHdr.FnRc |= EC_Error+EG_FnCode;
		*(servinfo->Context) = '\0';
		return FALSE;
	}

	/* initialise pathname */
	strcpy(pathname,servinfo->Context);	


	/* If there is no context string or the pathname has already    */
	/* been entered the capability is not valid but the access 	*/
	/* mask is.							*/
	/* This means that the name string is an absolute path name  	*/
	/* which need not be followed further here. 			*/
	/* Simply modify the access mask by the matrix of the root 	*/
	/* directory.							*/
	if( context == -1 || (name > 0 && next >= name) )
	{
		struct buf bp;
		struct incore_i *ip;
		
		bp.b_bnr = 1;
		bp.b_dev = vol->volnum;
		ip = iget(&bp,0,1,0);
#if MASK
Report ("	get_context:	UpdMask(0x%x, 0x%x, %s);",
	req->Access.Access, ip->ii_i.i_matrix, ip->ii_name);
#endif
		req->Access.Access = UpdMask(req->Access.Access,ip->ii_i.i_matrix);
		if (!iput(ip))
			return FALSE;
		return TRUE;
	}

	/* Otherwise simply follow the remains of the context name	*/
	/* through the directories to the end.				*/

	{
		struct incore_i *ip;
		
		/* build the absolute pathname of the required object in the FileSystem */
		if ( name==-1 || (next<name && next!=-1 && data[next]!='\0') )
			pathcat(servinfo->Context,data+next);

		ip = namei (servinfo->Context,vol);
		if( !ip ) 
		{
			if (vol->sync_allowed) {
				m->MsgHdr.FnRc |= EC_Error+EG_Name;
				*(servinfo->Context) = '\0';
				return FALSE;
			}
			m->MsgHdr.FnRc |= EC_Error+EG_Broken+EO_Medium;
			*(servinfo->Context) = '\0';
			return FALSE;
		}

		if( !GetAccess(&req->Access,(Key)ip->ii_i.i_cryptkey) )
		{
			m->MsgHdr.FnRc |= EC_Error+EG_Invalid+EO_Capability;
			*(servinfo->Context) = '\0';
			iput (ip);
			return FALSE;
		}
		if (!iput (ip)) {
			m->MsgHdr.FnRc |= EC_Error+EG_Broken+EO_Medium;
			*(servinfo->Context) = '\0';
			return FALSE;
		}
		
		unless( req->Access.Access != 0 )
		{
			m->MsgHdr.FnRc |= EC_Error+EG_Protected;
			*(servinfo->Context) = '\0';
			return FALSE;
		}
		
		strcpy(pathname,servinfo->Context);

		req->Next = name;
		return TRUE;
	}

	m->MsgHdr.FnRc |= EC_Fatal;
	*(servinfo->Context) = '\0';
	return FALSE;

}

/* ==================================================================== */

struct incore_i *
get_parent (servinfo,pathname,vol)

ServInfo *servinfo;
string pathname;
VD *vol;

/*
*  Returns a pointer to the Incore Inode of the Parent Directory
*  of the object specified by the absolute FileSystem-pathname.
*/

{
	size_t len;
	char subdir[IOCDataMax];
	struct incore_i *iip;

FLdebug("get_parent");


	/* calc the length of the Parent Directories pathname */
	len = strrchr (pathname,'/') - pathname;
	/* build the pathname for the Parent Dir */
	strncpy (subdir, pathname, len);
	subdir[len] = '\0';
	/* the Parent Dir of the Root Dir is Root Dir itself */
	if ( ! strstr(subdir,vol->vol_name) ) 
	{
/*
		iip = iget ((struct buf *)NULL, 0, 1, 0);
		return (iip);
*/
		servinfo->ParentOfRoot = TRUE;
		return ((struct incore_i *)NULL);
	}
	/* get the pointer to the inode of the Parent Dir */
	iip = namei (subdir,vol);
	return (iip);
}

/* ==================================================================== */

struct incore_i *
get_child (name, iip)

string name;
struct incore_i *iip;

/*
*  Searches for the specified object in the pointed Parent Dir.
*  Returns a pointer to the Incore Inode of the object, if it's found.
*/ 

{
	dir_t idir;
	struct incore_i *ip;
	int len = strlen(name);

FLdebug ("get_child (%s)", name);

	if (len == 0)
	{
FLdebug ("NULL (wg. len== 0)");
		return ((struct incore_i *)NULL);
	}
	DebWait (&iip->ii_sem);
	/* search in the Parent Dir */
	idir = searchi (iip, name, len);
	if (!idir.bp)
	{
	 	DebSignal (&iip->ii_sem);
FLdebug("NULL");
		return ((struct incore_i *)NULL);
	}
	/* make a incore copy of the inode of the object */
	ip = iget (idir.bp, idir.ofs, 
		   iip->ii_dirbnr, iip->ii_dirofs);
#if DSERV
Report ("	get_child :		brelse (0x%p, TAIL);",idir.bp->b_tbp);
#endif
	/* release Parent Dir */
	brelse (idir.bp->b_tbp, TAIL) ;
	DebSignal (&iip->ii_sem);
FLdebug("ip");
	return (ip);	 
}

/*--------------------------------------------------------
-- GetTargetDir						--
--							--
-- Returns the directory containing the 		--
-- target object of the supplied request		--
-- with the access permissions checked and validated.	--
-- If NULL is returned the global variable ServErr is	--
-- set to the appropriate error code.			--
-- The request must have been passed through GetContext --
-- first.						--
-- This is used in request which affect an object's	--
-- directory, e.g. create, delete, rename.		--
--							--
--------------------------------------------------------*/

struct incore_i *get_target_dir(ServInfo *servinfo, VD *vol)
{
	MCB *m = servinfo->m;
	struct incore_i *ip;
	byte *data = m->Data;
	IOCCommon *req = (IOCCommon *)(m->Control);
	int next = req->Next;
	char *pathname = servinfo->Pathname;
	
/*debug("GetTargetDir %x %x %s %s",m,servinfo->Context,pathname,&data[next]);*/

#if NAMES
Report ("gtdCTXT : >%s<",servinfo->Context);
Report ("gtdPATH : >%s<",servinfo->Pathname);
Report ("gtdTRGT : >%s<",servinfo->Target);
#endif
	/* if context is object, step back to parent */
	if( next == -1 || data[next] == '\0' )
	{
		ip = get_parent(servinfo,servinfo->Context,vol); 
		if( !ip ) {
			if (vol->sync_allowed) {
				m->MsgHdr.FnRc |= EC_Error+EG_Name;
				strcpy (servinfo->Target, "\0"); 
			}
			else {
				m->MsgHdr.FnRc |= EC_Error+EG_Broken+EO_Medium;
				strcpy (servinfo->Target, "\0"); 
			}
		}
		else
		{
			strcpy (servinfo->Target, ip->ii_name); 
		}
		return ip;
	}

	/* Otherwise we must follow the path through the directory	*/
	/* structure, checking access permissions on the way.		*/
	{
		int len;
		char name[NameMax];
		AccMask mask = req->Access.Access;
		struct incore_i *iip;
		extern /*inode.c*/ word iput (struct incore_i *);

		iip = namei(servinfo->Context,vol); 
		if( !iip ) {
			if (vol->sync_allowed) {
				m->MsgHdr.FnRc |= EC_Error+EG_Name;
				strcpy (servinfo->Target, "\0"); 
				return iip;
			}
			m->MsgHdr.FnRc |= EC_Error+EG_Broken+EO_Medium;
			strcpy (servinfo->Target, "\0"); 
			return iip;
		}
		ip = iip;
		
		forever
		{
			while ( data[next] == c_dirchar )
				next++;
				
			len = splitname( name, c_dirchar, &data[next]);

			if( data[next+len] == '\0' ) 
			{
				break;
			}

			/* special case . and .. */
			if( name[0] == '.' && name[1] == '\0' ) 
			{
				next += len;
				continue;
			}
			elif( name[0] == '.' && name[1] == '.' 
				&& name[2] == '\0') 
			{
				int l = strlen(pathname);

				ip = get_parent(servinfo,pathname,vol);
				if( !ip ) {
					if (vol->sync_allowed) {
						m->MsgHdr.FnRc |= EC_Error+EG_Name;
						strcpy (servinfo->Target, "\0"); 
						if (!iput (iip)) {
							ip = (struct incore_i *)NULL;
							m->MsgHdr.FnRc |= EC_Error+EG_Broken+EO_Medium;
						}
						return (ip);
					}
					m->MsgHdr.FnRc |= EC_Error+EG_Broken+EO_Medium;
					strcpy (servinfo->Target, "\0"); 
					iput (iip);
					return (ip);
				}
/* GEP 18/02/91				if (!iput (iip)) {
					m->MsgHdr.FnRc |= EC_Error+EG_Broken+EO_Medium;
					strcpy (servinfo->Target, "\0"); 
					return NULL;	
				}
				iip = ip;
*/	
				while( pathname[l--] != c_dirchar );
				if (l<0)
				{
					pathname[0] = '/';
					pathname[1] = '\0';
				}
				else
					pathname[l+1] = '\0';
				if (!iput (iip)) {
					m->MsgHdr.FnRc |= EC_Error+EG_Broken+EO_Medium;
					strcpy (servinfo->Target, "\0"); 
					return NULL;	
				}
				mask = UpdMask(mask,ip->ii_i.i_matrix);

				unless( CheckMask(mask,AccMask_R) )
				{
					m->MsgHdr.FnRc |= EC_Error+EG_Protected;
					strcpy (servinfo->Target, "\0"); 
					return NULL;
				}
				next += len;
				iip = ip;
				continue;
			}
			else 	
			{
				pathcat(pathname, name );
				ip = get_child(name, iip);
				if (!ip && !vol->sync_allowed) {
					m->MsgHdr.FnRc |= EC_Error+EG_Broken;
					strcpy (servinfo->Target, "\0"); 
					return ip;
				}
					
			}

			if (iip != ip)
				if (!iput (iip)) {
					m->MsgHdr.FnRc |= EC_Error+EG_Broken+EO_Medium;
					strcpy (servinfo->Target, "\0"); 
					return NULL;	
				}
			
			if( !ip ) {
				m->MsgHdr.FnRc |= EC_Error+EG_Name;
				strcpy (servinfo->Target, "\0"); 
				return ip;
			}

#if MASK
Report ("	get_dir:	UpdMask(0x%x, 0x%x, %s);",
	mask, ip->ii_i.i_matrix, ip->ii_name);
#endif
			mask = UpdMask(mask,ip->ii_i.i_matrix);
			
			if( ip->ii_i.i_mode == Type_Link )
			{
				req->Access.Access = mask;
				req->Next = next+len;
				handle_link (ip, servinfo);
				return NULL;
			}
			unless( ip->ii_i.i_mode & Type_Directory )
			{
				m->MsgHdr.FnRc |= EC_Error+EG_Name;
				strcpy (servinfo->Target, "\0"); 
				return NULL;
			}
			
			unless( CheckMask(mask,AccMask_R) )
			{
				m->MsgHdr.FnRc |= EC_Error+EG_Protected;
				strcpy (servinfo->Target, "\0"); 
				return NULL;
			}
			next += len;
			iip = ip;
		}
		req->Access.Access = mask;
		req->Next = next;
		strcpy (servinfo->Target, ip->ii_name); 
		return ip;
	}
}

/*--------------------------------------------------------
-- GetTargetObj						--
--							--
-- Returns the target object of the supplied request    --
-- with the access permissions checked and validated.	--
-- The request must have been passed through GetContext --
-- and GetTargetDir first.				--
--							--
--------------------------------------------------------*/

struct incore_i *get_target_obj(ServInfo *servinfo, struct incore_i *iip, VD *vol)
{
	struct incore_i *ip;
	MCB *m = servinfo->m;
	IOCCommon *req = (IOCCommon *)(m->Control);
	byte *data = m->Data;
	word next = req->Next;
	word len;
	char name[NameMax];
	char *pathname = servinfo->Pathname;

/*debug("GetTargetObj %x %s",next,pathname);*/
#if NAMES
Report ("gtoCTXT : >%s<",servinfo->Context);
Report ("gtoPATH : >%s<",servinfo->Pathname);
Report ("gtoTRGT : >%s<",servinfo->Target);
#endif
	if (!iip)
	{
		struct buf bp;
		
		bp.b_bnr = 1;
		bp.b_dev = vol->volnum;
		ip = iget (&bp, 0, 1, 0);
FLdebug ("return");		
		return (ip);
	}

	if( next == -1 || data[next] == '\0' )
	{
		extern /*"inode.c"*/ struct incore_i *namei (string,VD *);
		
		ip = namei(servinfo->Context,vol);
		if (!ip)
		{
			if (vol->sync_allowed)
				m->MsgHdr.FnRc |= EC_Error+EG_Name;
			else
				m->MsgHdr.FnRc |= EC_Error+EG_Broken;
			strcpy (servinfo->Target, "\0"); 
		}
		else
		{
			strcpy (servinfo->Target, ip->ii_name); 
		}
FLdebug ("return");		
		return ip;
	}

	if( (len = splitname(name, c_dirchar, &data[next] )) == 0 )
	{
		m->MsgHdr.FnRc |= EC_Error+EG_Name;
		ip = (struct incore_i *)NULL;
		strcpy (servinfo->Target, "\0"); 
FLdebug ("return");		
		return ip;
	}

	/* special case . and .. */
	if( name[0] == '.' && name[1] == '\0' ) 
	{
		req->Next = next+len;
		DebWait (&ihash_sem);
		iip->ii_count++;
		DebSignal (&ihash_sem);
FLdebug ("return");		
		return iip;
	}
	elif( name[0] == '.' && name[1] == '.' 
		&& name[2] == '\0') 
	{
		int l = strlen(pathname);

		ip = get_parent(servinfo,pathname,vol);
		if( !ip ) {
			if (vol->sync_allowed) {
				m->MsgHdr.FnRc |= EC_Error+EG_Name;
				strcpy (servinfo->Target, "\0"); 
FLdebug ("return");		
				return (ip);
			}
			m->MsgHdr.FnRc |= EC_Error+EG_Broken+EO_Medium;
			strcpy (servinfo->Target, "\0"); 
FLdebug ("return");		
			return (ip);
		}
		while( pathname[l--] != c_dirchar );
		if (l<0)
		{
			pathname[0] = '/';
			pathname[1] = '\0';
		}
		else
			pathname[l+1] = '\0';
	}
	else 	
	{
		pathcat(pathname, name );
		ip = get_child(name, iip);
		if ( !ip && !vol->sync_allowed ) {
			m->MsgHdr.FnRc |= EC_Error+EG_Broken+EO_Medium;
			strcpy (servinfo->Target, "\0"); 
FLdebug ("return");		
			return ip;	
		}
	}

	if( !ip ) 
	{
		m->MsgHdr.FnRc |= EC_Error+EG_Unknown+EO_Object;
		strcpy (servinfo->Target, "\0"); 
FLdebug ("return");		
		return ip;
	}

#if MASK
Report ("	get_obj:	UpdMask(0x%x, 0x%x, %s);",
	req->Access.Access, ip->ii_i.i_matrix, ip->ii_name);
#endif
	req->Access.Access = UpdMask(req->Access.Access,ip->ii_i.i_matrix);

	if( ip->ii_i.i_mode == Type_Link )
	{
		switch( servinfo->FnCode & FG_Mask )
		{
		default:
			req->Next = next+len;
			if (!iput(iip)) {	/* Free parent directory */
				m->MsgHdr.FnRc |= EC_Error+EG_Broken+EO_Medium;
				strcpy (servinfo->Target, "\0"); 
FLdebug ("return");		
				return NULL;
			}
			handle_link (ip, servinfo);
FLdebug ("return");		
			return ip = (struct incore_i *)NULL;
	
		/* These should be applied to the link, not to the	*/
		/* linked object.					*/
		case FG_Delete:
		case FG_Protect:
		case FG_Rename:
		case FG_ObjectInfo:
			break;
		}
		
	}

	req->Next = next+len;
	
	strcpy (servinfo->Target, ip->ii_name); 
FLdebug ("return");		
	return ip;
}

/* ==================================================================== */

/*--------------------------------------------------------
-- handle_link						--
--							--
-- Deal with a symbolic link in the directory path.	--
-- This routine does not return, but longjumps out to 	--
-- the root of the worker process.			--
-- Entered with Target locked.				--
--							--
--------------------------------------------------------*/

void handle_link (struct incore_i *ip, ServInfo *servinfo)
{
	MCB *mcb = servinfo->m;
	MsgBuf *m = New(MsgBuf);
	IOCCommon *req = (IOCCommon *)mcb->Control;
	word next = req->Next;
	byte *data = mcb->Data;

	struct buf *ldata_ptr;

	if( m == Null(MsgBuf) )
	{
		ErrorMsg(mcb,EC_Error|EG_NoMemory|EO_Message);
		iput (ip);
		return;
	}

	m->mcb.Control	= m->control;
	m->mcb.Data	= m->data;

	InitMCB(&m->mcb,mcb->MsgHdr.Flags,NullPort,
			mcb->MsgHdr.Reply,servinfo->FnCode);

	ldata_ptr = bread (ip->ii_dev, ip->ii_i.i_db[0], 1, SAVEA);
	if (ldata_ptr == NULL ) {
Error ("Volume /%s failed to read linked block.",volume[ip->ii_dev].vol_name);
		PutMsg(&volume[ip->ii_dev].unload_mcb);
		ErrorMsg(mcb,EC_Error|EG_Broken|EO_Medium);
		longjmp (servinfo->Escape,2);		
	}
	MarshalString(&m->mcb, ldata_ptr->b_un.b_link->name);

	if( data[next]!= '0' ) 
		MarshalString(&m->mcb,&data[next]);
	else 
		MarshalWord(&m->mcb, -1);

	MarshalWord(&m->mcb, 1);
	MarshalCap(&m->mcb, &ldata_ptr->b_un.b_link->cap);

	brelse ( ldata_ptr->b_tbp, TAIL );
	if (!iput ( ip )) {
		ErrorMsg(mcb,EC_Error|EG_Broken|EO_Medium);
		longjmp (servinfo->Escape,2);		
	}

	/* copy across any more parameters in the control vector */

	while( mcb->MsgHdr.ContSize > m->mcb.MsgHdr.ContSize )
	{
		word i = m->mcb.MsgHdr.ContSize;
		MarshalWord(&m->mcb,mcb->Control[i]);
	}
	/* and in the data vector */
	while( data[next++] != '\0' );

	if( next < mcb->MsgHdr.DataSize )
		MarshalData(&m->mcb,mcb->MsgHdr.DataSize-next,&data[next]);

	SendIOC(&m->mcb);

	Free(m);

GEPdebug (" handle_link() : longjmp(servinfo->Escape,2);");


	longjmp(servinfo->Escape,2);
}

/* ================================================================= */

struct incore_i *
make_obj (iip, pathname, mode, newname)

struct incore_i *iip;
string pathname, newname;
word mode;

/*
*  Depending on the make mode this routine creates
*  and renames objects in the pointed Parent Dir of the FileSystem.
*  Returns a pointer to the Incore Inode of the created or renamed object.
*/

{
	Matrix matrix;
	Date date;
	
	string name;
	dir_t idir;
	struct incore_i *ip;
	pkt_t ipkt;
	daddr_t ilgbnr;
	VD *vol;	

	vol = &volume[iip->ii_dev];
	DebWait (&iip->ii_sem);
	/* extract the name of the object */
	name = strrchr (pathname,'/');
	name++;
	/* if rename mode */
	if (mode<0) 
	{
		/* find the object in its Parent Dir */
		idir = searchi (iip, name, strlen (name));
		if (!idir.bp)
			goto finish;

		/* change the name of the DirEntry to the given one */
		strncpy ((idir.bp->b_un.b_dir+idir.ofs)->de_name,newname,
					NameMax);
	}
	/* if create mode */
	elif (mode>0)
	{
		/* search for an empty DirEntry */
		idir = searchi (iip, (string)NULL, 0);
		/* if an empty DirEntry is not found */
		if (!idir.bp) 
		{
			if (!vol->sync_allowed)
				;
			/* report exception if all direct blocks
			   of the Parent Dir already allocated and
			   there is no free DirEntry */
			elif (iip->ii_i.i_db[MAXDIR-1]) 
			{
Error ("Volume /%s; The parent-directory is full.",vol->vol_name);
				idir.bp = (struct buf *)NULL;
			} 
			else 
			{
				for (ilgbnr=0; iip->ii_i.i_db[ilgbnr]; ilgbnr++)
					;

DEBdebug ("	make_obj : 	ipkt = alloc (0x%p, %d, 1);",iip,ilgbnr);

				/* alloc a new block for Parent Dir */
				ipkt = alloc (iip, ilgbnr, 1);
				if (!ipkt.bcnt)
					idir.bp = (struct buf *)NULL;
				else 
				{

DEBdebug ("	make_obj : 	idir.bp = getblk (%d, %d, 1, NOSAVE);",
iip->ii_dev,ipkt.bnr);

					/* get a free buffer for the new block */
					idir.bp = getblk (iip->ii_dev,ipkt.bnr,
							  1,NOSAVE);
					/* clear its contents */
					if (!idir.bp) {
Report ("Volume /%s failed to get block.",vol->vol_name);
						goto finish;	
					}
					clr_buf (idir.bp);
				}
			}
			/* if no block could be alloced cause FileSystem is full */
			if (!idir.bp) 
				goto finish;

			/* update size of Parent Dir */
			iip->ii_i.i_size += sizeof (struct dir_elem);
		}
		/* create the DirEntry */
		strcpy ((idir.bp->b_un.b_dir+idir.ofs)->de_name,name);
		(idir.bp->b_un.b_dir+idir.ofs)->de_inode.i_mode = mode;
		matrix = (mode == DIR)  ? DefDirMatrix  : 
			 (mode == LINK) ? DefLinkMatrix : DefFileMatrix;
		(idir.bp->b_un.b_dir+idir.ofs)->de_inode.i_matrix = matrix;
		(idir.bp->b_un.b_dir+idir.ofs)->de_inode.i_cryptkey = NewKey();
		date = GetDate();
		(idir.bp->b_un.b_dir+idir.ofs)->de_inode.i_ctime = date;
		(idir.bp->b_un.b_dir+idir.ofs)->de_inode.i_atime = date;
		(idir.bp->b_un.b_dir+idir.ofs)->de_inode.i_mtime = date;	
		
		/* update number of DirEntries */
		iip->ii_i.i_spare++;
	}	
	/* make an incore copy of the inode if not delete mode */
	if (mode>0)
		ip = iget (idir.bp, idir.ofs, iip->ii_dirbnr, iip->ii_dirofs);
	else
		ip = (struct incore_i *)NULL;

DEBdebug ("	make_obj :	bwrite (0x%p);", idir.bp->b_tbp);

	/* write out Parent Dir */
	if (!bwrite(idir.bp->b_tbp)) {
Error ("Volume /%s failed to write parent directory.",vol->vol_name);
		DebSignal (&iip->ii_sem);
		PutMsg(&vol->unload_mcb);
		return ((struct incore_i *)NULL);
	}
	/* mark its Incore Inode as modified and release */
	iip->ii_changed = TRUE;
	DebSignal (&iip->ii_sem);
	if (mode!=Type_File || newname==(string)NULL)	
		if (!iput (iip))
			goto finish;
	return (ip);

finish:	
	DebSignal (&iip->ii_sem);
	if (mode!=Type_File || newname==(string)NULL)	
		iput (iip);
	return ((struct incore_i *)NULL);
}

/* ==================================================================== */

word
dir_server (struct incore_i *ip, struct incore_i *iip, MCB *m, Port reqport)

/*
*  Performs Stream Operations on Directory objects,
*  allowing merely Read and GetSize requests.
*  Forms a message including the DirEntries.
*  Return : FALSE if any I/O-error occurred, TRUE if not.
*/
 
{
	VD *vol = &volume[ip->ii_dev];
	word error;
	
	forever 
	{
		word e;
		m->MsgHdr.Dest = reqport;
		m->Timeout = StreamTimeout;

		DebWait (&vol->streammsg_sem);
		
		/* get stream message and check its validity */
		e = GetMsg(m);

		if (TestSemaphore (&vol->servreq_sem) < 1) {
			while (	(TestSemaphore (&vol->servreq_sem) < 1) &&
				vol->terminate_flag == FALSE)
				Delay (OneSec);
			if ( (TestSemaphore (&vol->servreq_sem) < 1) &&
				vol->terminate_flag == TRUE) {
				m->MsgHdr.FnRc &= ~FG_Mask;
				m->MsgHdr.FnRc |= FG_Close;
				e = Err_Null;

GEPdebug (" dir_server() : Volume /%s, FG_Close constructed!",vol->vol_name);

			}
			
		}		

		DebSignal (&vol->streammsg_sem);

		if( e == EK_Timeout ) break;
		if( e < Err_Null ) continue;

DEBdebug ("stream		>0x%x<",m->MsgHdr.FnRc);

		switch( m->MsgHdr.FnRc & FG_Mask ) 
		{
		case FG_Read:
		{
			ReadWrite *r = (ReadWrite *)(m->Control);
			word pos = r->Pos;
			word size = r->Size;
			word dirsize;
			word dpos = sizeof(DirEntry) * 2;
			word tfr = 0;
			word seq = 0;
			Port reply = m->MsgHdr.Reply;
			
			word req_size, i, k;
			daddr_t bnr;
			struct buf *bp;
			struct packet *tbp;
			
			DebWait ( &ip->ii_sem );
			/* calc size of directory */
			dirsize = (ip->ii_i.i_spare+2) * sizeof(DirEntry);
			/* check the given directory position */

			if( pos == dirsize )
			{
				DebSignal ( &ip->ii_sem);
				InitMCB(m,0,reply,NullPort,ReadRc_EOF|seq);
				PutMsg(m);
				break;
			}

			if( pos % sizeof(DirEntry) != 0 ||
			    pos < 0 || pos >= dirsize )
			{
				DebSignal ( &ip->ii_sem);
				ErrorMsg(m,EC_Error+EG_Parameter+1);
				break;
			}

			/* reduce the requested size if necessary */
			if( pos + size > dirsize ) 
				size = dirsize - pos;

			/* initialise reply message */
			InitMCB(m,MsgHdr_Flags_preserve,
				reply,NullPort,ReadRc_More|seq);
			/* special handling of "." */
			if (pos==0 && size>=sizeof(DirEntry)) 
			{ 
				marshal_ientry (m, ip, ".");
				tfr += sizeof (DirEntry);		
				size -= sizeof (DirEntry);		
				pos += sizeof (DirEntry);
			}
			/* special handling of ".." */
			if (pos<=sizeof(DirEntry)&&size>=sizeof(DirEntry)) 
			{ 
				marshal_ientry (m, iip, "..");
				tfr += sizeof (DirEntry);		
				size -= sizeof (DirEntry);		
				pos += sizeof (DirEntry);
			}
			/* find out number of directory blocks */
			req_size = ip->ii_i.i_blocks;
			
			for (i=0; (i<req_size && size>=sizeof(DirEntry));i++) 
			{
				/* adjust disc address of block */
				bnr = bmap (ip, i*BSIZE, &error);
				if ( error ) {
					ErrorMsg(m,EC_Error+EG_Broken+EO_Medium);
					return (FALSE);
				}
				/* read the packet of Dir Blocks */
				bp = bread(ip->ii_dev,bnr,1,SAVEA);
				if (bp == NULL ) {
Error ("Volume /%s failed to read directory block.",volume[ip->ii_dev].vol_name);
					PutMsg(&vol->unload_mcb);
					ErrorMsg(m,EC_Error+EG_Broken+EO_Medium);
					return (FALSE);
				}

				tbp = bp->b_tbp;
			
/* +2 Tabs */		
		/* inner loop for each Dir Block */
		for (k=0; (k<MAXDPB && size>=sizeof(DirEntry)); k++) 
		{
			/* skip the free DirEntries */
			if((bp->b_un.b_dir+k)->de_inode.i_mode != FREE) 
			{
				/* if required DirEntry add to reply message */
				if( dpos == pos ) 
				{
					marshal_dentry(m,bp->b_un.b_dir+k,vol);
					tfr += sizeof (DirEntry);	
					size -= sizeof (DirEntry);	
					pos += sizeof (DirEntry);	
					dpos += sizeof (DirEntry);	
				}
				/* else merely update position in directory */
				else 
				{
					dpos += sizeof(DirEntry);
					continue;
				}
			}
			/* if all requested DirEntries gathered
			   OR maximal transfer size achieved */
			if( size < sizeof(DirEntry) ||
			    IOCDataMax - tfr < sizeof(DirEntry) ) 
			{
				word e;
				/* mark last message */
				if( size < sizeof(DirEntry) ) 
				{
					m->MsgHdr.Flags = 0;
					if( dpos == dirsize )
					m->MsgHdr.FnRc = ReadRc_EOF|seq;
					else
						m->MsgHdr.FnRc = ReadRc_EOD|seq;
				}
				/* send the message */
				e = PutMsg(m);
				InitMCB(m,MsgHdr_Flags_preserve,
					reply,NullPort,ReadRc_More|seq);
				/* reset data-size to be transferred */
				tfr = 0;
				/* incr sequence-nr of consecutive messages */
				seq += ReadRc_SeqInc;
				m->MsgHdr.FnRc = ReadRc_More|seq;
			}
		}
/* -2 Tabs */		
				/* release packet */
				brelse (tbp, TAIL);
			}
			
			DebSignal ( &ip->ii_sem );
			/* if there's more data to be transferred */
			if( tfr > 0 )
			{
				/* form the last message and send */
				m->MsgHdr.Flags = 0;
				m->MsgHdr.FnRc = ReadRc_EOD|seq;
				e = PutMsg(m);
			}

			break;
		}
		
		case FG_Close:
		{
/*
Hdebug ("dir_server (%s)", vol->vol_name);
*/
			ErrorMsg(m,Err_Null);
			return (TRUE);
		}

		case FG_GetSize:
		{
			/* initialise reply message */
			InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);

			/* add info about the size of directory */
			MarshalWord(m,(ip->ii_i.i_spare+2) * sizeof(DirEntry));

			/* send the message */
#if DSERV
Report ("	dir_server :	GetSize returning %d",m->Control[0]);
#endif
			PutMsg(m);
			break;
		}

		/* other Stream Operations not permitted on Directories */
		case FG_Seek:
		case FG_Write:
		case FG_SetSize:
		{
			ErrorMsg(m,EC_Error+EG_WrongFn+EO_Directory);
			break;
		}	

		default:
			ErrorMsg(m,EC_Error+EG_FnCode+EO_Directory);
			break;
		}
	}
	return (TRUE);
}

/* ==================================================================== */
/* ==================================================================== */

void IOCRep1 (MsgBuf *r, MCB *m, struct incore_i *ip, 
	      word flags, char *pathname)

/*
*  Build a reply message for Open, Create and Locate.
*/

{
	IOCCommon *req = (IOCCommon *)(m->Control);
	Capability cap;


DEBdebug ("	pathname	>%s<",pathname);


	if( m->MsgHdr.Reply & Port_Flags_Remote ) flags |= Flags_Remote;

	*((int *)&(r->mcb)) = 0;
	r->mcb.MsgHdr.Dest = m->MsgHdr.Reply;
	r->mcb.MsgHdr.Reply = NullPort;
	r->mcb.MsgHdr.FnRc = Err_Null;
	
	r->mcb.Timeout = IOCTimeout;
	r->mcb.Control = r->control;
	r->mcb.Data    = r->data;

	MarshalWord(&r->mcb,ip->ii_i.i_mode);
	MarshalWord(&r->mcb,flags);
	new_cap(&cap, ip, req->Access.Access);
	MarshalCap(&r->mcb,&cap);
	MarshalString(&r->mcb,pathname);
}

/* ==================================================================== */

void 
new_cap(Capability *cap, struct incore_i *ip, AccMask mask)

/*
*  Create a new capability for the object with the given
*  access mask.
*/

{
	int i;
	Key key = (Key)ip->ii_i.i_cryptkey;
	byte check = (key>>16)&0xff;


DEBdebug ("	new_cap		>0x%x<", mask);

	cap->Access = mask;

	for( i=0; i<7 ; cap->Valid[i++] = check );
	cap->Valid[0] = mask;
	cap->Valid[3] = mask;

	Crypt(1, key, (byte *)&cap->Valid, 7);

}

/* ==================================================================== */

#define PROTECT_MASK	0xC2C2C2C2

void 
marshal_ientry( MCB *m, struct incore_i *ip, char *name)

/*
*  Form a reply message according to informations of the Incore Inode.
*/

{
	word 	flags = 0;
	Matrix	result_matrix;
	
	if (ip)
		MarshalData(m,4,(byte *)&ip->ii_i.i_mode);
	else
		MarshalData(m,4,(byte *)&flags);
	MarshalData(m,4,(byte *)&flags);
	if (ip) {
		if (volume[ip->ii_dev].writeprotected)
			result_matrix = ip->ii_i.i_matrix & ~PROTECT_MASK;
		else
			result_matrix = ip->ii_i.i_matrix;
#if MATRIX
Report (" marshal_ientry() : Volume /%s marshalling matrix 0x%x",volume[ip->ii_dev].vol_name,result_matrix);
#endif
		MarshalData(m,4,(byte *)&result_matrix);
	}
	else
		MarshalData(m,4,(byte *)&flags);
	MarshalData(m,32,name);
}

/* ==================================================================== */

void 
marshal_dentry( MCB *m, struct dir_elem *dp, VD *vol)

/*
*  Form a reply message according to informations of the DirEntry.
*/

{
	word 	flags = 0;
	Matrix	result_matrix;
	
	if (vol->writeprotected)
		result_matrix = dp->de_inode.i_matrix & ~PROTECT_MASK;
	else
		result_matrix = dp->de_inode.i_matrix;
	MarshalData(m,4,(byte *)&dp->de_inode.i_mode);
	MarshalData(m,4,(byte *)&flags);
#if MATRIX
Report (" marshal_dentry() : Volume /%s marshalling matrix 0x%x",vol->vol_name,result_matrix);
#endif
	MarshalData(m,4,(byte *)&result_matrix);
	MarshalData(m,32,dp->de_name);
}

/* ==================================================================== */


/*
 *  Initialise and form a reply message for ObjInfo,
 *  with expanded informations of the Incore Inode.
 *  Return : FALSE if any I/O-error occurred, TRUE if not
 *
 *   19/01/89 : - Addition of marshalling link-entries
 *   15/02/89 : - Managing capabilities in a proper manner
 *
 */

word 
marshal_info (MCB *m, struct incore_i *ip)
{
 word size;
 struct buf *ldata_ptr;

 InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
 marshal_ientry(m,ip,ip->ii_name);
	
 if ( ip->ii_i.i_mode == LINK )
 {
  	ldata_ptr = bread (ip->ii_dev, ip->ii_i.i_db[0], 1, SAVEA);
	if (ldata_ptr == NULL ) {
Error ("Volume /%s failed to read data.",volume[ip->ii_dev].vol_name);
		PutMsg(&volume[ip->ii_dev].unload_mcb);
		return (FALSE);
	}
	MarshalData (m,sizeof(Capability),(byte *)&ldata_ptr->b_un.b_link->cap);
	MarshalData (m,strlen(ldata_ptr->b_un.b_link->name),
		     ldata_ptr->b_un.b_link->name);
	brelse (ldata_ptr->b_tbp, TAIL);
 }
 else
 {
 	size = ip->ii_i.i_size;
 	MarshalData(m,4,(byte *)&ip->ii_i.i_accnt);
 	if( ip->ii_i.i_mode & Type_Directory )
		size = (ip->ii_i.i_spare+2) * sizeof(DirEntry);
 	MarshalData(m,4,(byte *)&size);
 	MarshalData(m,4,(byte *)&ip->ii_i.i_ctime);
 	MarshalData(m,4,(byte *)&ip->ii_i.i_atime);
 	MarshalData(m,4,(byte *)&ip->ii_i.i_mtime);
 }
 return (TRUE);	
}

/* ==================================================================== */
/* ==================================================================== */

void pathcat(string s1, string s2)

{
	if ( *s2 == '\0') return;
	while( *s1 ) s1++;
	if( *(s1-1) != c_dirchar ) *s1++ = c_dirchar;
	while( (*s1++ = *s2++) != 0 );
}

/*--------------------------------------------------------
-- InvalidFn						--
-- NullFn						--
--							--
-- Server function table default entries which either	--
-- complain or quietly reply respectively.		--
--							--
--------------------------------------------------------*/

void InvalidFn(ServInfo *servinfo)

{
	ErrorMsg(servinfo->m,EC_Error+EG_WrongFn);
}

void NullFn(ServInfo *servinfo)

{
	ErrorMsg(servinfo->m,Err_Null);
}

/*--------------------------------------------------------
-- ErrorMsg						--
--							--
-- Return the given error to the sender of the message.	--
--							--
--------------------------------------------------------*/

void ErrorMsg(MCB *mcb, word err)

{
Hdebug ("Code %x, %x, Called by %s%s", 
         err, err | mcb->MsgHdr.FnRc, CalledBy (mcb), 
         (mcb->MsgHdr.Reply == NullPort) ? ",but NULLPORT" : "");

	err |= mcb->MsgHdr.FnRc;

Hdebug ("mcb (%x) mcb->MsgHdr->Rep (%x) Code (%x)",mcb,mcb->MsgHdr.Reply,err);

	if( mcb->MsgHdr.Reply == NullPort ) 
		return;

	*((int *)mcb) = 0;	/* no shorts at present */
	mcb->MsgHdr.Dest = mcb->MsgHdr.Reply;
	mcb->MsgHdr.Reply = NullPort;
	mcb->MsgHdr.FnRc = err;
	PutMsg(mcb);
}

/*--------------------------------------------------------
-- UpdMask						--
--							--
-- Generate a new mask from the old, modified by the	--
-- matrix.						--
--							--
--------------------------------------------------------*/

AccMask UpdMask(AccMask mask, Matrix matrix)

{
	AccMask res = 0;

FLdebug ("UpdMask(0x%x, 0x%x)",mask,matrix);

	if( mask & AccMask_V ) res |= matrix & 0xff;
	if( mask & AccMask_X ) res |= (matrix>>8) & 0xff;
	if( mask & AccMask_Y ) res |= (matrix>>16) & 0xff;
	if( mask & AccMask_Z ) res |= (matrix>>24) & 0xff;

FLdebug ("new Mask (0x%x)",res);

	return res;
	
}

/*--------------------------------------------------------
-- CheckMask						--
--							--
-- Check that the given mask allows the given access	--
-- 							--
--							--
--------------------------------------------------------*/

int CheckMask(AccMask mask,AccMask access)

{

FLdebug("CheckMask(%x %x)",mask,access);

	if( (mask & access) == 0 ) return 0;
	return 1;
}

/*--------------------------------------------------------
-- GetAccess						--
--							--
-- Update the capability Access field with the access	--
-- rights actually allowed by the capability.		--
--							--
--------------------------------------------------------*/

word GetAccess(Capability *cap, Key key)

{
	AccMask mask;
	int i;
	byte check = (key>>16)&0xff;
	
	/* decrypt the capability */
	Crypt(0, key, (byte *)&(cap->Valid), 7);

	for( i = 0; i < 7 ; i++ )
	{
		if( i == 0 || i == 3 ) continue;
		if(cap->Valid[i] != check) break;
	}
	
	if( i != 7 ) return false;

	mask = cap->Valid[0];
	if( mask != cap->Valid[3] ) return false;

	cap->Access &= mask;

	/* re-encrypt it for protection */
	Crypt(1, key, (byte *)&(cap->Valid), 7);

	return true;
}

/*--------------------------------------------------------
-- crypt						--
--							--
-- Encryption/decryption routine. Intended for 		--
-- capabilities, but may be used for anything.		--
--							--
--------------------------------------------------------*/

static void Crypt(bool encrypt, Key key, byte *data, word size)
{
	word c;
	word salt = size;

	while( size-- )
	{
		c = *data;

		/* we are using a 29 bit key, if it overflows, feed bit	*/
		/* back in at the bottom.				*/
		key &= 0x1fffffff;
		if( key & 0x10000000 ) key ^= 0x0040a001;

		/* encrypt the character */
		c = (key & 0xff) - c;

		if( ++salt >= 20857 ) salt = 0;

		/* the new key is made dependant on the last cleartext char */
		/* this is *data on encryption and c on decryption.	    */
		if( encrypt ) key = key + key + *data + salt;
		else key = key + key + (c&0xff) + salt;
		
		*data++ = c;
	}
}

Key NewKey()
{
	/* make a key out of the date plus the current processor uptime */
	return GetDate()+_cputime();
}

/*--------------------------------------------------------
-- InitNode						--
--							--
-- Initialise a node					--
--							--
--------------------------------------------------------*/

void InitNode(ObjNode *o, char *name, int type, int flags, Matrix matrix)
{
	Date date = GetDate();
FLdebug("InitNode %x %s %x %x %x",o,name,type,flags,matrix);
	strcpy(o->Name,name);
	o->Type = type;
	o->Flags = flags;
	o->Matrix = matrix;
	InitSemaphore(&o->Lock,1);
	o->Key = NewKey();
	o->Dates.Creation = date;
	o->Dates.Access   = date;
	o->Dates.Modified = date;
	o->Account = 0;
	o->Size = 0;
}

/*--------------------------------------------------------
-- GetName						--
--							--
-- Extracts the final part of the object named in the	--
-- MCB. Updates pathname accordingly.			--
--							--
--------------------------------------------------------*/

bool GetName(MCB *m, string name, string pathname)
{
	IOCCommon *req = (IOCCommon *)(m->Control);
	byte *data = m->Data;
	int next = req->Next;

/*debug("GetName %s %s",&data[next],pathname);	*/
	if( splitname(name, c_dirchar, &data[next] ) == 0 )
	{
		m->MsgHdr.FnRc |= EC_Error+EG_Name;
		return false;
	}

	pathcat(pathname, name );
	return true;
}

/* ==================================================================== */


/* Routines copied from C library */

#define _chararg int               /* arg spec for char when ANSI says int */

char *strstr(const char *a, const char *b)
                              /* find first occurrence of b in a, or NULL */
{   int i;
    for (;;)
    {   for (i=0;; i++)
        {   if (b[i] == 0) return (char *)a;
            if (a[i] != b[i]) break;
        }
        if (*a++ == 0) return (char *) NULL;
    }
}

char *strchr(const char *s, _chararg ch)
                                        /* find first instance of ch in s */
{
    for (;; s++)
    {   if (*s == (char)ch) return (char *)s;
        if (*s == 0) return (char *)NULL;
    }
}

char *strrchr(const char *s, _chararg ch)  /* find last instance of ch in s */
{   const char *p = s;
    while (*p++ != 0);
    do { if (*--p == (char)ch) return (char *)p; } while (p!=s);
    return (char *)NULL;
}


#pragma -s1

void _stack_error(Proc *p)
{
  /* do NOT use Error instead of IOdebug */
	IOdebug ("File Server stack overflow in %s at %x.",p->Name,&p);
}


/* ================================================================= */
/* ==================================================================== */

/* end of fservlib.c */ 
