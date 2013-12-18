static char rcsid[] = "$Header: /hsrc/filesys/fs/RCS/fserver.c,v 1.8 1992/05/18 14:14:41 nickc Exp $";

/* $Log: fserver.c,v $
 * Revision 1.8  1992/05/18  14:14:41  nickc
 * fixed test for in_nucleus to work with C40
 *
 * Revision 1.7  1991/05/08  13:40:03  al
 * Parsytec bug fixes added. (1/5/91)  New revision 1.4
 *
 * Revision 1.6  1991/03/28  11:41:08  nick
 * Version changed to 1.3, also made into a single place for future changes.
 *
 * Revision 1.5  1991/03/26  10:50:55  nick
 * maxnii set for checker, otherwise alloc_mem tries to malloc 0 bytes
 *
 * Revision 1.4  1991/03/22  11:44:08  nick
 * version number changed to 1.3
 *
 * Revision 1.3  1991/03/21  15:09:40  nick
 * Brought up to date. disables buffer checksumming, several other fixes.
 *
 * Revision 1.6  90/08/06  18:01:19  adruni
 * ParentDir of RootDir is no more RootDir itself.
 * 
 * Revision 1.5  90/08/02  11:17:08  adruni
 * Changing ModifyTime in do_rename.
 * 
 * Revision 1.4  90/07/27  18:21:07  adruni
 * Conceptual protection error in do_open fixed.
 * 
 * Revision 1.3  90/06/15  17:00:13  adruni
 * getting rid of -DDEVINFO.
 * devinfo parameter-check added.
 * 
 * Revision 1.2  90/02/01  17:36:40  chris
 * Tape support amongst other things
 * 
 * Revision 1.1  90/01/02  19:03:11  chris
 * Initial revision
 * 
 */

/*************************************************************************
**                                                                      **
**                  H E L I O S   F I L E S E R V E R                   **
**                  ---------------------------------                   **
**                                                                      **
**                  Copyright (C) 1988, Parsytec GmbH                   **
**                         All Rights Reserved.                         **
**                                                                      **
** fserver.c								**
**                                                                      **
**	Routines handling Directory Operations.				**
**									**
**************************************************************************
** HISTORY  :								**
** ----------								**
** Author   :  23/09/88  A.Ishan 					**
** Modified :   4/01/89  H.J.Ermen - Completion of ErrorMsg-codes and   **
**				     adding of SS_Harddisk-identifiers  **
**				   - Implementation of do_rename()	**
**	       			   - Implementation of do_link() 	**
*************************************************************************/

#define DEBUG		0
#define CHECKER 	1
#define TAPE		1
#define GEPDEBUG 	0


#define VERSION		"1.4"

#include <root.h>
#include "nfs.h"

#if CHECKER
#include "check.h"
#endif

NameInfo	FServerInfo =
		{
			NullPort,
			Flags_StripName,
			DefDirMatrix,
			(word *)NULL
		};

FileSysInfo *fsi;
DiscDevInfo *ddi;	
VolumeInfo *vvi;
word checksum_allowed;

#define 	do_syncfsSS	4000

/* ==================================================================== */

/* General procedures */

static void do_syncfs (void);

static void do_open (ServInfo *);
static void do_create (ServInfo *);
static void do_locate (ServInfo *);
static void do_objinfo (ServInfo *);
static void do_serverinfo (ServInfo *);
static void do_delete (ServInfo *);
static void do_rename (ServInfo *);
static void do_link (ServInfo *);
static void do_protect (ServInfo *);
static void do_setdate (ServInfo *);
static void do_refine (ServInfo *);
static void do_closeobj (ServInfo *);

/* ==================================================================== */

static DispatchInfo FServerDInfo = 
		{
			NULL,
			NullPort,
			SS_HardDisk,
			NULL,
			{
				do_open,
				do_create,
				do_locate, 
				do_objinfo, 
				do_serverinfo, 
				do_delete,
				do_rename, 
				do_link, 
				do_protect,
				do_setdate, 
				do_refine,
				do_closeobj 
			}
		};

/* ==================================================================== */

int 
main ( void )

/*
 *  Installs the FileServer on aprocessor with lots of MBytes.
 *  After installation any object of the FileSystem can be accessed 
 *  from any node of the network by the name "/fserver/...".
 */

/*
 *  Meaning of optionally given parameters:
 *
 *
 *  ( nothing )		: Regular boot of the server with BASIC-CHECKS.
 *			  A FULL-CHECK will be performed if any error occures
 *			  during BASIC-CHECKS. 
 *     -m		: Make a new file-system and terminate
 *     -s   		: Generate a new superblock, copy it to the info-block
 *			  of cylinder group 0 and terminate.
 *     -n		: Boot the server without any CHECKS.
 *     -b		: Boot the server with BASIC-CHECKS (same as (nothing))
 *     -f		: Boot the server with FULL-CHECKS
 *
 */ 
 
{
	char **argv;
	Object *nte;
	struct fs_para  mp;
	struct fs f;
	struct info_blk *ibp;
	char *fsname = NULL;	
	char *msg;
			
	int makefs = FALSE;
	int makesb = FALSE;
	int checks = TRUE;
	int full_check = TRUE;
	int chksum = FALSE;
	void *devinfo;
	InfoNode *fsinfo;
	InfoNode *deviceinfo;
     	Environ env;
	word i;
	word (*writemsg)() = Write;
	bool in_nucleus;

#ifdef __TRAN
	in_nucleus = (word)main < (word)GetRoot(); /* better test needed */
#else
	  {
	    word *	base = GetSysBase();

	    
	    in_nucleus = (
#ifdef __C40
			  (base < (word *)C40CAddress( main )) &&
			  ((word *)C40CAddress( main ) < base + *base)
#else
			  (base < (word *)main) &&
			  ((word *)main < base + *base)
#endif
			  )
	  }	
#endif

	if( !in_nucleus )
	{	
		GetEnv(MyTask->Port, &env);
		argv = env.Argv;

		for(argv++; *argv; argv++ )
		{
			char *arg = *argv;
			if( *arg == '-' )
			{
				switch(arg[1])
				{
				case 'm': makefs = TRUE; break;

				case 's': makesb = TRUE; break;
				
				case 'n': checks = FALSE; break;
				
				case 'f': full_check = TRUE; break;
				
				case 'b': full_check = FALSE; break; 

				case 'c': chksum = TRUE; break;
				
				}
				continue;
			}
			fsname = arg;
		}

		if( fsname == NULL )
		{
			msg = "usage: fs [-m|-s|-n|-b|-f][-c] device\n";
			writemsg(env.Strv[1],msg,strlen(msg),-1);
			Exit(0x100);
		}
	}
	else 
	{
		static word IOmsg(Stream *s, char *msg, int size, int timeout);
		fsname = "helios";
		full_check = FALSE;
		writemsg = IOmsg;
		GetRoot()->Flags |= Root_Flags_special;
	}


	msg = "Helios Filesystem  Rel.910501  V" VERSION "   (C) Copyright 1991 Parsytec / Perihelion\r\n\n";
	writemsg(env.Strv[1],msg,strlen(msg),-1);
IOdebug("  -- HFS V" VERSION " :  SERVER started --\n");

	checksum_allowed = chksum;
	
	if((devinfo = load_devinfo()) == NULL )  {
		msg = "HFS : ERROR : Unable to open device-info file !";
		writemsg(env.Strv[1],msg,strlen(msg),-1);
		Exit (1);
	}
	if((fsinfo = find_info(devinfo,Info_FileSys,fsname)) == NULL ) {
		msg = "HFS : ERROR : Cannot find filesystem info !";
		writemsg(env.Strv[1],msg,strlen(msg),-1);
		Exit (1);
	}
	fsi = (FileSysInfo *)RTOA(fsinfo->Info);
	vvi = (VolumeInfo *)RTOA(fsi->Volumes);

	if((deviceinfo = find_info(devinfo,Info_DiscDev,RTOA(fsi->DeviceName)))==NULL) {
		msg = "HFS : ERROR : Cannot find device info !";
		writemsg(env.Strv[1],msg,strlen(msg),-1);
		Exit (1);
	}
	ddi = (DiscDevInfo *)RTOA(deviceinfo->Info);

	
	init_info( fsi, ddi);

	open_dev(ddi);
	
	if( makefs ) 
	{
		if ( setjmp(term_jmp) != 0)
		{
			msg = "HFS : ERROR : Failed to create filesystem!\n";
			writemsg(env.Strv[1],msg,strlen(msg),-1);
			close_dev();
			Exit (1);
		}
		if( make_fs(fsi,ddi, 0) ) {
			msg = "HFS : Filesystem successfully created!\n";
			writemsg(env.Strv[1],msg,strlen(msg),-1);
			Exit (0);
		}
		else  {
			msg = "HFS : ERROR : Failed to create filesystem!\n";
			writemsg(env.Strv[1],msg,strlen(msg),-1);
			Exit (1);
		}
	}
	elif( makesb )
	{
		if ( setjmp(term_jmp) != 0)
		{
			msg = "HFS : ERROR : Failed to create superblock!\n";
			writemsg(env.Strv[1],msg,strlen(msg),-1);
			close_dev();
			Exit (1);
		}
		if( make_super(fsi,ddi,0) ) {
			msg = "HFS : Superblock successfully created!\n";
			writemsg(env.Strv[1],msg,strlen(msg),-1);
			Exit (0);
		}
		else {
			msg = "HFS : ERROR : Failed to create superblock!\n";
			writemsg(env.Strv[1],msg,strlen(msg),-1);
			Exit (1);
		}
	}
	

/*
 *	filesystem checker starts here
 */	

   if (checks) {
	msg = "  Helios Filesystem Checker Rel.910422  V" VERSION "  (C) Copyright 1991 Parsytec\r\n";
	writemsg(env.Strv[1],msg,strlen(msg),-1);
IOdebug("  -- HFS V" VERSION " :  CHECKER started --"); 
	no_corrections = FALSE;
	silent_mode = TRUE;
	
	possdirb = fsi->PossDir;
	possindb = fsi->PossIndir;
	mingood  = fsi->MinGood;
	maxbmerr = fsi->BitMapErrs;

	if ( setjmp(term_jmp) != 0)
	{
		msg = "HFS : ERROR : Device Error! Fileserver during check aborted !";
		writemsg(env.Strv[1],msg,strlen(msg),-1);
		close_dev();
		Exit (1);
	}

	psmal = 1;
	pmedi = 2;
	phuge = 4;
	maxpsz = 16;
	pscnt = 30;
	pmcnt = 1;
	phcnt = 1;
	maxnii = 4;

#if GEPDEBUG
IOdebug(" fserver : going to read info block");
#endif
	ibp = (struct info_blk *) Malloc (BSIZE);
	if ( read_blk ( (void *)ibp, 2) != BSIZE ) {
		write_blk ( (void *)ibp , 2);
		if ( read_blk ( (void *)ibp, 2) != BSIZE ) {
			msg = "HFS : ERROR : Info block on disk irreversibly physically destroyed!";
			writemsg(env.Strv[1],msg,strlen(msg),-1);
			msg = "              Fileserver during check aborted !";
			writemsg(env.Strv[1],msg,strlen(msg),-1);
			Free (ibp);
			close_dev ();
			Exit (1);
		}
/*
 *  further info block reads should be added here to recover lost info block
 */
		msg = "HFS : ERROR : Unable to read info block !";
		writemsg(env.Strv[1],msg,strlen(msg),-1);
		msg = "              Fileserver during check aborted !";
		writemsg(env.Strv[1],msg,strlen(msg),-1);
		Free (ibp);
		close_dev ();
		Exit (1);
	}	
	
	maxbpg = ibp->fs.fs_cgsize;
	maxncg = ibp->fs.fs_ncg;
	cgoffs = ibp->fs.fs_cgoffset;
	Free (ibp);
	
	if ( !alloc_mem() ) {
		msg = "HFS : ERROR : Unable to allocate memory for server!";	
		writemsg(env.Strv[1],msg,strlen(msg),-1);
		msg = "              Fileserver during check aborted !";
		writemsg(env.Strv[1],msg,strlen(msg),-1);
		close_dev ();
		Exit (1);
	}
	

	
	if ( vvi->CgSize != maxbpg || 
	     vvi->CgCount != maxncg ||
	     vvi->CgOffset != cgoffs ) {
		IOdebug(" fserver : The disk parameters kept on disk and in 'devinfo' divert!");     	
		IOdebug("    Parameter     DISK    DEVINFO");
		IOdebug("=====================================");
		IOdebug("     CgSize        %d        %d",maxbpg,vvi->CgSize);
		IOdebug("     CgCount       %d        %d",maxncg,vvi->CgCount);
		IOdebug("     CgOffset      %d        %d\n",cgoffs,vvi->CgOffset);				
		IOdebug(" Change parameters in 'devinfo' or make new superblock !");
		IOdebug(" Fileserver during check terminated!");
		msg = " HFS : ERROR : Fileserver during check terminated !";
		writemsg(env.Strv[1],msg,strlen(msg),-1);
		close_dev ();
		Exit (1);
	}

	init_buffer_cache();
	init_incore_fs();
	
#if GEPDEBUG
IOdebug(" fserver : call fscheck()");
#endif
	
	if ( !fscheck (full_check) ) {
		msg = "HFS : ERROR : Checker returned error, Fileserver aborted!";
		writemsg(env.Strv[1],msg,strlen(msg),-1);
		close_dev ();
		Exit (1);	
	}
IOdebug("  -- HFS V" VERSION " :  CHECKER succesfully finished --\n"); 
   }	
   else {
 	if ( setjmp(term_jmp) != 0)
	{
		msg = "HFS : ERROR : Device Error ! Failed to init filesystem !";
		writemsg(env.Strv[1],msg,strlen(msg),-1);
		close_dev();
		Exit (1);
	}
   }

	
	if( (filedrive = init_fs( fsi, ddi, 0)) == -1 ) /* Assume vol 0 is fs */
		Exit (1);

#if TAPE
	if( (tapedrive = init_fs( fsi, ddi, 1)) != -1 ) /* Assume vol 0 is fs */
		IOdebug("Tape partition is %d",tapedrive);
#endif
	Free(devinfo);

	FServerDInfo.reqport = FServerInfo.Port = NewPort ();

	{
		Object *o;
		char mcname[100];
		/* build machine name */
		MachineName(mcname);
		o = Locate(NULL,mcname);
		/* take an entry in the processors Name Table */ 
		nte = Create(o,fs_name,Type_Name,sizeof(NameInfo),
			(byte *)&FServerInfo);
		/* clear info about the client from Name Server's cache */
		Close(o);
	}

  	/* Reply to procman that we have started */
	if( in_nucleus )
	{
		MCB m;
		word e;
		InitMCB(&m,0,MyTask->Parent,NullPort,0x456);
		e = PutMsg(&m);
	}

	/* init the sync-Semaphore */
	InitSemaphore ( &sync_sem, 1 );
	/* start the background process 'do_syncfs' */
	Fork(do_syncfsSS, do_syncfs, 0);
#if TAPE
	if( tapedrive != -1 )
	{	int xx = (int)Fork(tserverSS, tserver, 0);
		if( xx == 0 )
			IOdebug("Unable to start tape server");
	}
#endif

	/* start 'fdispatch' process */
	fdispatch(&FServerDInfo);
	/* delete the name of the FileServer from the Name Table */
	Delete(nte,NULL);

	/* Send a message to termfs to signal termination of the */
	/* file-server */
	{	MCB m;
		InitMCB ( &m, MsgHdr_Flags_preserve, term_port, NullPort,
			  FC_GSP+SS_HardDisk+FG_Private+FO_Terminate );
		PutMsg ( &m );
	}
IOdebug("  -- HFS V" VERSION " :  SERVER finished --\n");
	Exit(0);
}

/* ==================================================================== */

static void 
do_syncfs(void)

/*
*  Calls every 20 seconds the 'sync_fs' routine,
*  in order to write out delayed write data in Buffer Cache.
*/

{
	forever 
	{
		Delay (20*OneSec);
		sync_fs ();
	}	
}

/* ==================================================================== */
/* ==================================================================== */

static void 
do_open(ServInfo *servinfo)

/*
*  Opens a stream to the named object.
*  Returns a pointer to the opened stream structure,
*  and thus provides a Stream Port 
*  for Stream Operations on this object.
*/

{
	MCB *m = servinfo->m;
	MsgBuf *r;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	Port reqport;
	char *data = m->Data;
	char *pathname = servinfo->Pathname, *name;

	struct incore_i *iip, *ip;
	
#if DEBUG
printf("	do_open >%s/%s<\n",pathname,&data[req->Common.Next]);
#endif

	/* get Parent Dir of object */
	iip = get_target_dir (servinfo);	
	/* if no Parent Dir found */
	if (!servinfo->ParentOfRoot && !iip)
	{
		ErrorMsg(m,EO_Directory);
#if DEBUG
printf("	do_open :	No such parent directory >%s< !\n",pathname);
#endif
		return;
	}

	/* get the target object */
	ip = get_target_obj (servinfo,iip);

	/* alloc memory for buffering stream messages */
	r = New(MsgBuf);

	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Message);
#if DEBUG
printf("	do_open :	No memory for message buffering !\n");
#endif
		if (ip)
			iput (ip);
/*$$$
		if (iip != ip)
*/
		if (iip)
			iput (iip);
		return;
	}

	/* if target object doesn't exist and if the open mode is create mode */
	if((iip) && ( !ip && (req->Arg.Mode & O_Create) )) 
	{
		m->MsgHdr.FnRc = SS_HardDisk;

		/* check that we are allowed to create a file here */
		unless( CheckMask(req->Common.Access.Access,AccMask_W) )
		{
			ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
#if DEBUG
printf("	do_open :	No access permission !\n");
#endif
			iput( iip );
			Free(r);
			return;
		}
		if (fsfull)
		{
#if DEBUG
printf("	do_open :	Cannot create >%s< !\n",pathname);
#endif
			ErrorMsg(m,EC_Error|EG_NoMemory);
			iput (iip);
			Free(r);
			return;
		}
		/* create the specified object */
		ip = make_obj (iip, pathname, Type_File, "do_not_iput");

		/* give creators all rights to new object */
		req->Common.Access.Access = AccMask_Full;
	}

	/* if there's still no target object */
	if (!ip)
	{
#if DEBUG
printf("	do_open :	No such file >%s< !\n",pathname);
#endif
		ErrorMsg(m,EC_Error|EG_Create|EO_File);
		if (iip)
			iput (iip);
		Free(r);
		return;
	}

	/* check the access permissions on this object */
/*	
	unless( CheckMask(req->Common.Access.Access,req->Arg.Mode&Flags_Mode) )  
*/
	if( (req->Arg.Mode & O_ReadOnly && ! CheckMask(req->Common.Access.Access, AccMask_R))
	||  (req->Arg.Mode & O_WriteOnly && ! CheckMask(req->Common.Access.Access, AccMask_W))
	||  (req->Arg.Mode & O_Execute && ! CheckMask(req->Common.Access.Access, AccMask_E))
	||  ((ip->ii_i.i_mode == Type_Directory) && ! CheckMask(req->Common.Access.Access, AccMask_R))
	||  ((ip->ii_i.i_mode == Type_File) && 
	     req->Arg.Mode & O_Truncate && ! CheckMask(req->Common.Access.Access, AccMask_W))
	  )
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_File);
#if DEBUG
printf("	do_open :	No access permission !\n");
#endif
		iput (ip);
/*$$$
		if (iip != ip)
*/
		if (iip)
			iput (iip);
		Free(r);
		return;
	}

	/* form a reply message */
	IOCRep1 (r, m, ip, Flags_Server|Flags_Closeable, pathname);

	/* create a new Stream Port */
	reqport = NewPort();
	/* add info about Stream Port to message */
	r->mcb.MsgHdr.Reply = reqport;
	/* send message to client */
	PutMsg(&r->mcb);
	Free(r);

	/* if target object is of type directory
	   then call special routine to handle
	   Stream Operations on directories */
	if (ip->ii_i.i_mode == Type_Directory) 
	{
		/* call the appropriate routine */
		dir_server (ip,iip,m,reqport);
		iput (ip);
/*$$$
		if (iip != ip) 
*/
		if (iip)
			iput (iip);
		/* free Stream Port */
		FreePort(reqport);
		return;
	}
/*$$$
	if (iip != ip)	
*/
	if (iip)
		iput (iip);

	if( req->Arg.Mode & O_Truncate )
	{
		/* delete contents if it isn't a directory */
		if (ip->ii_i.i_mode == Type_File)
			setsize_file (ip, 0);
	}
	
	/* following lines deal with Stream Operations
	   on normal objects (no directories) */
	forever
	{
		word e;
		m->MsgHdr.Dest = reqport;
		m->Timeout = StreamTimeout;
		m->Data = data;

		/* get stream message and check its validity */
		e = GetMsg(m);
		if( e == EK_Timeout ) 
			break;
		if( e < Err_Null ) 
			continue;

		if (setjmp(close_jmp) != 0)
		{
			if(m->MsgHdr.Reply != NullPort) ErrorMsg(m,Err_Null);
			iput (ip);
			/* give Stream Port free */
			FreePort (reqport);
#if DEBUG
printf("	do_open : closing >%s<\n",pathname);
#endif
			return;
		}

#if 0
printf("	do_open :	Stream message is 0x%x\n",m->MsgHdr.FnRc);
#endif
 		if( MyTask->Flags & Task_Flags_servlib )
 			IOdebug("HFS: %F %s",m->MsgHdr.FnRc,pathname);

		switch( m->MsgHdr.FnRc & FG_Mask )
		{
		case FG_Read:
			if( req->Arg.Mode & O_ReadOnly )
				read_file(m,ip);
			else
				ErrorMsg(m,EC_Error+EG_Protected+EO_File);
			break;

		case FG_Write:
			if( req->Arg.Mode & O_WriteOnly )
				write_file(m,ip);
			else
				ErrorMsg(m,EC_Error+EG_Protected+EO_File);
			break;
		
		case FG_Close:
			if(m->MsgHdr.Reply != NullPort) ErrorMsg(m,Err_Null);
			iput (ip);
			/* give Stream Port free */
			FreePort (reqport);
#if DEBUG
printf("	do_open : closing >%s<\n",pathname);
#endif
			return;

		case FG_GetSize:
			if(  req->Arg.Mode & O_ReadOnly 
			  || req->Arg.Mode & O_WriteOnly )
			{
				InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
				MarshalWord(m,ip->ii_i.i_size);
				PutMsg(m);
			}
			else
				ErrorMsg(m,EC_Error+EG_Protected+EO_File);
			break;

		case FG_Seek:
			if(  req->Arg.Mode & O_ReadOnly 
			  || req->Arg.Mode & O_WriteOnly )
				seek_file(m,ip);		
			else
				ErrorMsg(m,EC_Error+EG_Protected+EO_File);
			break;

		case FG_SetSize:
		{
			word newsize = m->Control[0];
			
			if( ! (req->Arg.Mode & O_WriteOnly) )
				ErrorMsg(m,EC_Error+EG_Protected+EO_File);
			/* only truncation permitted */
			elif( newsize > ip->ii_i.i_size )
				ErrorMsg(m,EC_Error+EG_Parameter+1);
			else 
			{
				setsize_file (ip, newsize);
				InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
				MarshalWord(m,ip->ii_i.i_size);
				PutMsg(m);
			}
			break;
		}

		default:
			ErrorMsg(m,EC_Error+EG_FnCode+EO_File);
			break;
		}
	}
	FreePort (reqport);
}

/* ==================================================================== */

static void 
do_create(ServInfo *servinfo)

/*
*  Creates an object of the given type.
*  Returns a pointer to the created object.
*/

{
	MCB *m = servinfo->m;
	MsgBuf *r;
	IOCCreate *req = (IOCCreate *)(m->Control);
	char *data = m->Data;
	char *pathname = servinfo->Pathname,*name;

	struct incore_i *iip, *ip;
	dir_t idir;
	
#if DEBUG
printf("	do_create >%s/%s<\n",pathname,&data[req->Common.Next]);
#endif

	/* get Parent Dir of object */
	iip = get_target_dir (servinfo);	
	/* if no such Parent Dir */
	if (!iip)
	{
		ErrorMsg(m,EO_Directory);
#if DEBUG
printf("	do_create :	No such parent directory >%s< !\n",pathname);
#endif
		return;
	}
	/* check if client has permission to write in Parent Dir */
	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
#if DEBUG
printf("	do_create :	No access permission !\n");
#endif
		iput (iip);
		return;
	}
	/* alloc memory for message buffering */
	r = New(MsgBuf);
	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Message);
#if DEBUG
printf("	do_create :	No memory for message buffering !\n");
#endif
		iput (iip);
		return;
	}

	ip = get_target_obj (servinfo, iip);

	if (ip)
	{
		m->MsgHdr.FnRc |= EC_Error+EG_Create+EO_File;
#if DEBUG
printf("	do_create :	File already exists >%s< !\n",pathname);
#endif
		iput (ip);
/*$$$
		if (iip != ip)
*/
			iput (iip);
		goto NullFile;
	}
	else
		m->MsgHdr.FnRc = SS_HardDisk;
		
	if (fsfull)
	{
#if DEBUG
printf("	do_create :	Cannot create >%s< !\n",pathname);
#endif
			ErrorMsg(m,EC_Error|EG_NoMemory);
			iput (iip);
			Free(r);
			return;
	}

	if ( req->Type == Type_Directory ) 
	{
		/* create a directory */
		ip = make_obj (iip, pathname, Type_Directory, (string)NULL);
	} 
	else 
	{	
		/* create a normal file */
		ip = make_obj (iip, pathname, Type_File, (string)NULL);
	}
	
	/* if create couldn't succeed */
	if (!ip)
	{
#if DEBUG
printf("	do_create :	Create couldn't succeed !\n");
#endif
NullFile:	ErrorMsg(m,EO_Object);
		Free(r);
		return;
	}

	/* give creators all rights to new object */
	req->Common.Access.Access = AccMask_Full;

	/* form reply message with info about created object */
	IOCRep1 (r, m, ip, 0, pathname);
	/* send message to client */
	PutMsg(&r->mcb);

	iput (ip);
	Free(r);
}

/* ==================================================================== */

static void 
do_locate(ServInfo *servinfo)

/*
*  Tests the existance of the named object.
*  Returns a pointer to the specified object,
*  in order to provide further access.
*/

{
	MCB *m = servinfo->m;
	MsgBuf *r;
	char *data = m->Data;
	IOCCommon *req = (IOCCommon *)(m->Control);
	char *pathname = servinfo->Pathname;

	struct incore_i *ip,*iip;
		
#if DEBUG
printf("	do_locate >%s/%s<\n",pathname,&data[req->Next]);
#endif

	/* get Parent Directory */
	iip = get_target_dir (servinfo);	

	/* if Parent Dir doesn't exist */
	if (!servinfo->ParentOfRoot && !iip)
	{
		ErrorMsg(m,EO_Directory);
#if DEBUG
printf("	do_locate :	No such parent directory >%s< !\n",pathname);
#endif
		return;
	}

	/* get target object */
	ip = get_target_obj (servinfo, iip);
/*$$$
	if (iip != ip)
*/
	if (iip)
		iput (iip);
	
	/* if no such target object */
	if (!ip)
	{
		ErrorMsg(m,EO_Object);
#if DEBUG
printf("	do_locate :	No such file >%s< !\n",pathname);
#endif
		return;
	}

	/* The client is only allowed to know about the object 
	   if he has any access at all.	*/
	unless( CheckMask(req->Access.Access,AccMask_Full) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Object);
#if DEBUG
printf("	do_locate :	No access permission !\n");
#endif
		iput (ip);
		return;
	}
	/* alloc memory for message buffering */
	r = New(MsgBuf);
	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Message);
#if DEBUG
printf("	do_locate :	No memory for message buffering !\n");
#endif
		iput (ip);
		return;
	}

	/* form reply message with info of target object */
	IOCRep1 (r, m, ip, 0, pathname);
	/* send message to client */
	PutMsg(&r->mcb);

	iput (ip);
	Free(r);
}

/* ==================================================================== */

static void 
do_objinfo (ServInfo *servinfo)

/*
*  Obtains any extra information on the named object.
*  This includes size, dates, protection, and so forth.
*/

{
	MCB *m = servinfo->m;
	char *data = m->Data;
	IOCMsg1 *req = (IOCMsg1 *)(m->Control);
	char *pathname = servinfo->Pathname;

	struct incore_i *ip,*iip;
		
#if DEBUG
printf("	do_objinfo >%s/%s<\n",pathname,&data[req->Common.Next]);
#endif

	/* get Parent Dir */
	iip = get_target_dir (servinfo);	

	/* if no such Parent Dir */
	if (!servinfo->ParentOfRoot && !iip)
	{
		ErrorMsg(m,EO_Directory);
#if DEBUG
printf("	do_objinfo :	No such parent directory >%s< !\n",pathname);
#endif
		return;
	}

	ip = get_target_obj (servinfo, iip);
/*$$$
	if (iip != ip)
*/
	if (iip)
		iput (iip);

	/* if no such target object */
	if (!ip)
	{
		ErrorMsg(m,EO_Object);
#if DEBUG
printf("	do_objinfo :	No such file >%s< !\n",pathname);
#endif
		return;
	}

	/* only allow if user has any access at all */
	unless( CheckMask(req->Common.Access.Access,AccMask_Full) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Object);
#if DEBUG
printf("	do_objinfo :	No access permission !\n");
#endif
		iput (ip);
		return;
	}
	/* form reply message with extended info about object */
	marshal_info (m, ip);
	/* send message to client */
	PutMsg(m);

	iput (ip);
}

/* ==================================================================== */

static void 
do_serverinfo (ServInfo *servinfo)
{
 MCB *m = servinfo->m;
 IOCMsg1 *req = (IOCMsg1 *) m->Control;
 char *pathname = servinfo->Pathname;
 struct buf *bp;		
 word fssize, fsused, fsfree;
 word d = Flags_Server;
 
#if DEBUG
printf ("	do_serverinfo :	>%s<\n", pathname);
#endif
					 /* only allow if user has any	*/
					 /* access at all 		*/
 unless ( CheckMask (req->Common.Access.Access, AccMask_Full) )
 {
 	ErrorMsg (m, EC_Error+EG_Protected);
 	
#if DEBUG
printf ("	do_serverinfo :	No access permission\n");
#endif
	return;
 }
 
 fssize = BSIZE * incore_fs.fs_dsize;
 fsfree = BSIZE * incore_sum.is_fs.sum.s_nbfree;
 fsused = fssize - fsfree;
 
 InitMCB (m, 0, m->MsgHdr.Reply, NullPort, Err_Null );
 
 MarshalData (m, 4, (byte *) &d);
 					/* The total net size of the fs	*/
 MarshalData ( m, 4, (byte *) &fssize );
  					/* The amount free blocks	*/
 MarshalData ( m, 4, (byte *) &fsfree );
 					/* The amount currently used	*/
 MarshalData ( m, 4, (byte *) &fsused );
 
 PutMsg (m);				
}

/* ==================================================================== */

static void 
do_delete(ServInfo *servinfo)

/*
 *  Delete the named object.
 */

{
 MCB *m = servinfo->m;
 char *data = m->Data;
 IOCCommon *req = (IOCCommon *)(m->Control);
 char *pathname = servinfo->Pathname;
	
 char linkpath[IOCDataMax];
 struct incore_i *ip, *iip;		
 	
 word dlgbnr;

#if DEBUG
printf("	do_delete >%s/%s<\n",pathname,&data[req->Next]);
#endif

 /* get Parent Dir of object */
 iip = get_target_dir (servinfo);
 /* if no such Parent Dir */
 if (!iip)
 {
 	ErrorMsg(m,0);
#if DEBUG
printf("	do_delete :	No such parent directory >%s< !\n",pathname);
#endif
	return;
 }
 /* get the target object */
 ip = get_target_obj (servinfo, iip);
/*
 if (ip == iip)
 {
	ErrorMsg(m,EC_Error+EG_Unknown);
#if DEBUG
printf("	do_delete :	No such file >%s< !\n",pathname);
#endif
	iput (iip);
	iput (ip);
	return;
 }
*/
 /* if no such target object */
if (!ip)
 {
	ErrorMsg(m,0);
#if DEBUG
printf("	do_delete :	No such file >%s< !\n",pathname);
#endif
	iput (iip);
	return;
 }
	
 /* check the delete permissions */
 unless( CheckMask(req->Access.Access,AccMask_D) ) 
 {
	ErrorMsg(m,EC_Error+EG_Protected+EO_File);
#if DEBUG
printf("	do_delete :	No access permission !\n");
#endif
	iput (ip);
	iput (iip);		
	return;
 }

 /* the root-dir should never be deleted ! */
 if ( ip->ii_i.i_mode == Type_Directory &&
      ! strcmp ( strstr(pathname,fs_name), fs_name) )
 {
 	ErrorMsg (m, EC_Error+EG_Delete+EO_Directory);
#if DEBUG
printf ("	do_delete :	Attempt to delete root-directory\n");
#endif
	iput (ip);
	iput (iip);
	return;
 }
 	
 /* directories can be only deleted 
    if they don't have any DirEntries */
 if( ip->ii_i.i_mode == Type_Directory &&
	ip->ii_i.i_spare != 0 )
 {
	ErrorMsg(m,EC_Error+EG_Delete+EO_Directory);
#if DEBUG
printf("	do_delete :	Directory has files !\n");
#endif
	iput (ip);
	iput (iip);
	return;
 }

 if( ip->ii_count > 1 )
 {
	ErrorMsg(m,EC_Error+EG_InUse+EO_File);
#if DEBUG
printf("	do_delete :	Other user working on file !\n");
#endif
	iput (ip);
	iput (iip);
	return;
 }

 /* following lines deal with deleting of normal files */
 Wait (&ip->ii_sem);
 /* give free the data blocks on disc */
 if (ip->ii_i.i_size) 
 {
	dlgbnr = (ip->ii_i.i_size-1)/BSIZE+1;
	fREE (ip, 0, dlgbnr);
 }	
 /* update Incore Inode */
 ip->ii_changed = TRUE;
 /* delete DirEntry of object in the Parent Dir */
 memset (&(ip->ii_i), 0, sizeof(struct inode));
 memset (ip->ii_name, 0, NameMax);
 /* release Incore Inode */
 Signal (&ip->ii_sem);
 iput (ip);
	
 /* delete DirEntry of object in the Parent Dir */
 iip->ii_i.i_spare--;
 iip->ii_changed = TRUE;
 iput(iip); 
 /* send reply message */	
 ErrorMsg(m,Err_Null);
}
/* ==================================================================== */

static void 
do_rename (ServInfo *servinfo)

{
	MCB *m = servinfo->m;
	char *data = m->Data;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	word hdr = *(word *)m;
	AccMask mask = req->Common.Access.Access;
	struct incore_i *iip, *iip2, *ip, *ip2;
	char name[NameMax];
	char name2[NameMax];
	char *pathname = servinfo->Pathname;

	ServInfo servinfo2;

	word dlgbnr;
	
/*debug("DoRename %x %x %s",m,context,pathname);*/
#if DEBUG
printf("	do_rename >%s/%s<\n",pathname,&data[req->Common.Next]);
#endif

	memcpy ((void *)&servinfo2,(void *)servinfo,sizeof(ServInfo));
	
	iip = get_target_dir(servinfo);

	if( !iip )
	{
		ErrorMsg(m,EO_Directory);
#if DEBUG
printf("	do_rename :	No such source parent directory >%s< !\n",pathname);
#endif
		return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
#if DEBUG
printf("	do_rename :	No access permission for source dir !\n");
#endif
		iput (iip);
		return;
	}

	ip = get_target_obj (servinfo, iip);

	if(ip == iip)
	{
		ErrorMsg(m,EC_Error+EG_Unknown);
#if DEBUG
printf("	do_rename :	No such source file >%s< !\n",pathname);
#endif
		iput (iip);
		iput (ip);
		return;
	}

	if(!ip)
	{
		ErrorMsg(m,0);
#if DEBUG
printf("	do_rename :	No such source file >%s< !\n",pathname);
#endif
		iput (iip);
		return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_D) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_File);
#if DEBUG
printf("	do_rename :	No access permission for source file!\n");
#endif
		iput (ip);
		iput (iip);
		return;
	}

	/* the root-dir should never be renamed ! */
	if ( ip->ii_i.i_mode == Type_Directory &&
      		! strcmp ( strstr(pathname,fs_name), fs_name) )
 	{
 		ErrorMsg (m, EC_Error+EG_Delete+EO_Directory);
#if DEBUG
printf ("	do_rename :	Attempt to delete root-directory\n");
#endif
		iput (ip);
		iput (iip);
		return;
	}
 	
 	if( ip->ii_count > 1 )
 	{
		ErrorMsg(m,EC_Error+EG_InUse+EO_File);
#if DEBUG
printf("	do_rename :	Other user working on source file !\n");
#endif
		iput (ip);
		iput (iip);
		return;
	 }

	/* we now have the source object, find the dest directory */
	/* restore state to context dir				  */

	*(word *)m = hdr;
	req->Common.Access.Access = mask;
	req->Common.Next = req->Arg.ToName;

	iip2 = get_target_dir( &servinfo2 );

	if( !iip2 )
	{
		ErrorMsg(m,EO_Directory);
#if DEBUG
printf("	do_rename :	No such dest parent directory >%s< !\n",servinfo2.Pathname);
#endif
		iput (ip);
		iput (iip);
		return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
#if DEBUG
printf("	do_rename :	No access permission for dest dir !\n");
#endif
		iput(ip);
		iput(iip);
		iput(iip2);
		return;
	}
		
	if( splitname(name2, c_dirchar, &data[req->Common.Next]) == 0 )
	{
		ErrorMsg(m,EC_Error+EG_Name);
#if DEBUG
printf("	do_rename :	Mal formed dest name !\n");
#endif
		iput(ip);
		iput(iip);
		iput(iip2);
		return;
	}
		
		
	ip2 = get_target_obj ( &servinfo2, iip2 );
	if( ip2 )
	{
		if (ip2 == iip2)
		{
			ErrorMsg(m,EC_Error+EG_Unknown);
#if DEBUG
printf("	do_rename :	No such dest file !\n");
#endif
			iput (ip);
			iput (iip);
			iput (iip2);
			iput (ip2);
			return;
		}   
		if (ip2->ii_i.i_mode == DIR 
		    && ip2->ii_i.i_spare != 0)
		{
			ErrorMsg(m,EC_Error+EG_Protected+EO_File);
#if DEBUG
printf("	do_rename :	Dest directory has files !\n");
#endif
			iput (ip);
			iput (iip);
			iput (ip2);
			iput (iip2);
			return;
		}   

		unless( CheckMask(req->Common.Access.Access, AccMask_D) ) 
		{
			ErrorMsg(m,EC_Error+EG_Protected+EO_File);
#if DEBUG
printf("	do_rename :	No access permission for dest file !\n");
#endif
			iput (ip);
			iput (iip);
			iput (ip2);
			iput (iip2);
			return;
		}

	 	if( ip2->ii_count > 1 )
 		{
			ErrorMsg(m,EC_Error+EG_InUse+EO_File);
#if DEBUG
printf("	do_rename :	Other user working on destination file !\n");
#endif
			iput (ip);
			iput (iip);
			iput (ip2);
			iput (iip2);
			return;
		 }

		Wait ( &ip2->ii_sem );	/* Get access to the inode	*/
	   	if ( ip2->ii_i.i_size != 0 )	
		{			/* There are blocks to free	*/
			dlgbnr = (ip2->ii_i.i_size - 1) / BSIZE + 1;
			fREE (ip2, 0, dlgbnr);
		}
		
	   	ip2->ii_changed     = TRUE;	/* Update incore inode data	*/
		/* delete DirEntry of object in the Parent Dir */
		memset (&(ip2->ii_i), 0, sizeof(struct inode));
		memset (ip2->ii_name, 0, NameMax);
	   	Signal ( &ip2->ii_sem );	/* Unlock the inode		*/
	   	iput (ip2);
					/* The delete operation itself	*/
		 /* delete DirEntry of object in the Parent Dir */
 		iip2->ii_i.i_spare--;
		iip2->ii_changed = TRUE;
	}
	else
		m->MsgHdr.FnRc = SS_HardDisk;	
#if DEBUG
printf("PATHNAME >%s<\n",pathname);
printf("NEWNAME >%s<\n",servinfo2.Pathname);
#endif
	/* if we get here we have the source and dest directories	*/
	/* the object itself, and the new name in name2.		*/
	/* now do the rename.						*/

	if (iip == iip2)
	{
		Wait(&ip->ii_sem);
		ip->ii_i.i_mtime = GetDate();
		ip->ii_i.i_atime = GetDate();
		ip->ii_changed = TRUE;
		Signal(&ip->ii_sem);
		iput(ip);
		
		make_obj (iip, pathname, -1, name2);
		iput(iip2);
	}
	else
	{
		if (fsfull)
		{
#if DEBUG
printf("	do_rename :	Cannot create >%s< !\n",servinfo2.Pathname);
#endif
			ErrorMsg(m,EC_Error|EG_NoMemory);
			iput (iip);
			iput (ip);
			iput (iip2);
			return;
		}

	 	ip2 = make_obj (iip2, servinfo2.Pathname, DATA, (string)NULL);		
 		if ( !ip2 )
 		{				/* Unable to create an entry	*/
 			ErrorMsg (m, EC_Error+EG_Create);
#if DEBUG
printf ("	do_rename :	Unable to create target-object\n");
#endif
			iput (iip);
			iput (ip);
			return;
 		}
 	
 		Wait ( &ip->ii_sem );		/* Lock the source inode	*/
 		Wait ( &ip2->ii_sem );		/* Get exclusive target access	*/
 						/* Copy disk inode's content	*/
		memcpy ((void *) &ip2->ii_i, (void *) &ip->ii_i, 
			sizeof(struct inode));
 	
		ip2->ii_i.i_mtime = GetDate();
		ip2->ii_i.i_atime = GetDate();
	 	ip2->ii_changed   = TRUE;		/* Inode content really changed */
	 	ip2->ii_count  = ip->ii_count;
	 	strcpy (ip2->ii_name, name2);
 	
	 	Signal ( &ip2->ii_sem );	/* Allow others to access target*/
 		iput (ip2);			/* Target not further used*/
 						/* Now delete the source-inode	*/
 		ip->ii_changed = TRUE;		/* information			*/
		/* delete DirEntry of object in the Parent Dir */
		memset (&(ip->ii_i), 0, sizeof(struct inode));
		memset (ip->ii_name, 0, NameMax);
		Signal ( &ip->ii_sem );
 		iput (ip);
 					/* The delete operation itself	*/
 		 /* delete DirEntry of object in the Parent Dir */
		iip->ii_i.i_spare--;
		iip->ii_changed = TRUE;
		iput(iip); 
	}

	ErrorMsg(m, Err_Null);	
}

/* ==================================================================== */

static void
do_link (ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	IOCMsg3 *req = (IOCMsg3 *)(m->Control);
	byte *data = m->Data;
	char name[NameMax];
	char *pathname = servinfo->Pathname;
	struct incore_i *iip, *ip;
	pkt_t data_pkt;
	struct buf *ldata_ptr;

#if DEBUG
printf("	do_link >%s/%s<\n",pathname,&data[req->Common.Next]);
#endif
	iip = get_target_dir(servinfo);

	if( ! iip )
	{
		ErrorMsg(m,EO_Directory);
#if DEBUG
printf("	do_link :	No such parent directory >%s< !\n",pathname);
#endif
		return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);
#if DEBUG
printf("	do_link :	No access permission !\n");
#endif
		iput (iip);
		return;
	}

	ip = get_target_obj (servinfo, iip);
		
	if( ip )
	{
		ErrorMsg(m,EC_Error+EG_Create);
#if DEBUG
printf("	do_link :	File already exists >%s< !\n",pathname);
#endif
		iput (ip);
/*$$$
		if (iip != ip)	
*/
			iput (iip);
		return;
	}
        else
        	m->MsgHdr.FnRc = SS_HardDisk;
        	
	/* We now know that there is not an existing entry with the	*/
	/* desired name. Install the link.				*/

	if (fsfull)
	{
		ErrorMsg (m, EC_Error+EG_NoMemory+EO_Link);
#if DEBUG
printf("	do_link :	Link alloc couldn't succeed !\n");
#endif
		iput (iip);
		return;
	}
	
	ip = make_obj (iip, pathname, Type_Link, (string)NULL);

	if ( !ip )
	{
		ErrorMsg(m, EC_Error+EG_NoMemory+EO_Link);
#if DEBUG
printf("	do_link :	Link couldn't succeed !\n");
#endif
		return;
	}

	Wait ( &ip->ii_sem );

	if (fsfull)
	{
		ErrorMsg (m, EC_Error+EG_NoMemory+EO_Link);
#if DEBUG
printf("	do_link :	Link alloc couldn't succeed !\n");
#endif
		Signal (&ip->ii_sem);
		iput (ip);
		return;
	}
	data_pkt = alloc (ip, 0, 1);
	if ( !data_pkt.bcnt )
	{
		ErrorMsg (m, EC_Error+EG_NoMemory+EO_Link);
#if DEBUG
printf("	do_link :	Link alloc couldn't succeed !\n");
#endif
		Signal (&ip->ii_sem);
		iput (ip);
		return;
	}

	ip->ii_i.i_size = strlen (&data[req->Name]);
	ip->ii_i.i_blocks = 1;
	ip->ii_changed = TRUE;

	ldata_ptr = getblk (ip->ii_dev, data_pkt.bnr, 1, NOSAVE);
	clr_buf (ldata_ptr);
	strcpy (ldata_ptr->b_un.b_link->name, &data[req->Name]);
	ldata_ptr->b_un.b_link->cap = req->Cap;
	bwrite (ldata_ptr->b_tbp);

	Signal ( &ip->ii_sem );
	iput (ip);

	ErrorMsg (m, Err_Null);
}

/* ==================================================================== */

static void 
do_protect(ServInfo *servinfo)

/*
*  Sets the access matrix of the named object.
*/

{
	MCB *m = servinfo->m;
	char *data = m->Data;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	Matrix newmatrix = req->Arg.Matrix;
	char *pathname = servinfo->Pathname;
	
/* AI */	AccMask mask;
	
	struct incore_i *ip,*iip;
		
#if DEBUG
printf("	do_protect >%s/%s<\n",pathname,&data[req->Common.Next]);
#endif

	/* get Parent Directory of the object */
	iip = get_target_dir (servinfo);	
	/* if there's no such Parent Dir */
	if (!servinfo->ParentOfRoot && !iip)
	{
		ErrorMsg(m,0);
#if DEBUG
printf("	do_protect :	No such parent directory >%s< !\n",pathname);
#endif
		return;
	}

/* AI */	mask = req->Common.Access.Access;

	/* get target object */
	ip = get_target_obj (servinfo, iip);
/*$$$
	if (iip != ip)
*/
	if (iip)
		iput (iip);

	/* if no such target object */
	if (!ip)
	{
		ErrorMsg(m,0);
#if DEBUG
printf("	do_protect :	No such file >%s< !\n",pathname);
#endif
		return;
	}
	/* check alter permissions on the object's Access Matrix */
	unless( CheckMask(req->Common.Access.Access,AccMask_A) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected);
#if DEBUG
printf("	do_protect :	No access permission !\n");
#endif
		iput (ip);
		return;
	}
	/* We are allowed to alter the matrix, ensure that it is resonable, 
	   and that the client retains either Delete or Alter permission. */
	if( (UpdMask(AccMask_Full, newmatrix) & 
		(AccMask_A|AccMask_D)) == 0 )
	{
		ErrorMsg(m,EC_Error+EG_Invalid+EO_Matrix);
#if DEBUG
printf("	do_protect :	Invalid access matrix !\n");
#endif
		iput (ip);
		return;
	}
#if 0
	if( ( ( (ip->ii_i.i_mode != Type_Directory)
	      || (strcmp ( strstr(pathname,fs_name), fs_name ) != 0) 
	      )
	    && (ip->ii_count > 1) 
	    )
	  || ( ( (ip->ii_i.i_mode == Type_Directory)
	       && (strcmp ( strstr(pathname,fs_name), fs_name ) == 0) 
	       )
	     && (ip->ii_count > 2)
	     )
	  )   
 	{
		ErrorMsg(m,EC_Error+EG_InUse);
#if DEBUG
printf("	do_protect :	Other user working on file !\n");
#endif
		iput (ip);
		return;
	}
#endif
	Wait (&ip->ii_sem);
	/* update Incore Inode */
	ip->ii_changed = TRUE;
	ip->ii_i.i_matrix = newmatrix;
	Signal (&ip->ii_sem);
	/* send reply message to client */
	ErrorMsg(m,0);

	iput (ip);
}

/* ==================================================================== */

static void 
do_setdate (ServInfo *servinfo)

/*
*  Sets the date stamps on the named object.
*/

{
	MCB *m = servinfo->m;
	char *data = m->Data;
	IOCMsg4 *req = (IOCMsg4 *)(m->Control);
	DateSet dates = req->Dates;
	word e = Err_Null;
	char *pathname = servinfo->Pathname;
	
	struct incore_i *ip,*iip;

#if DEBUG
printf("	do_setdate >%s/%s<\n",pathname,&data[req->Common.Next]);
#endif

	/* get Parent Dir of the object */
	iip = get_target_dir (servinfo);	
	/* if no such Parent Dir */
	if (!servinfo->ParentOfRoot && !iip)
	{
		ErrorMsg(m,0);
#if DEBUG
printf("	do_setdate :	No such parent directory >%s< !\n",pathname);
#endif
		return;
	}

	/* get the target object */
	ip = get_target_obj (servinfo, iip);
/*
	if (iip != ip)
*/
	if (iip)
		iput (iip);

	/* if no such target object */
	if (!ip)
	{
		ErrorMsg(m,0);
#if DEBUG
printf("	do_setdate :	No such file >%s< !\n",pathname);
#endif
		return;
	}
	/* only allow the user to set the date on an object 
	   if he could write to it. */
	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected);
#if DEBUG
printf("	do_setdate :	No access permission !\n");
#endif
		iput (ip);
		return;
	}

	/* if no new date stamps included in the request */
	if (dates.Access == 0 && dates.Modified == 0)
		e = EC_Error|EG_Parameter|5;
	else 
	{
		Wait (&ip->ii_sem);
		/* set the according date stamps in the Incore Inode */
		if (dates.Access > 0) 
		{
			ip->ii_i.i_atime = dates.Access;
		}
		if (dates.Modified > 0) 
		{
			ip->ii_i.i_mtime = dates.Modified;
		}
		ip->ii_changed = TRUE;
		Signal (&ip->ii_sem);
	}
	/* send reply message to client */
	ErrorMsg(m,e);

	iput (ip);
}

/* ==================================================================== */

static void 
do_refine (ServInfo *servinfo)

/*
*  Refines or restricts the access rights contained in the capability
*  in the named object structure by generating a new capability.
*/

{
	MCB *m = servinfo->m;
	char *data = m->Data;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	Capability cap;
	char *pathname = servinfo->Pathname;
	AccMask newmask = req->Arg.AccMask;
	struct incore_i *ip,*iip;

#if DEBUG
printf("	do_refine >%s/%s<\n",pathname,&data[req->Common.Next]);
#endif

	/* get Parent Dir of object */
	iip = get_target_dir (servinfo);	
	/* if no such Parent Dir */
	if (!servinfo->ParentOfRoot && !iip)
	{
		ErrorMsg(m,0);
#if DEBUG
printf("	do_refine :	No such parent directory >%s< !\n",pathname);
#endif
		return;
	}

	/* get the target object */
	ip = get_target_obj (servinfo, iip);
/*
	if (iip != ip)
*/
	if (iip)
		iput (iip);

	/* if no such target object */
	if (!ip)
	{
		ErrorMsg(m,0);
#if DEBUG
printf("	do_refine :	No such file >%s< !\n",pathname);
#endif
		return;
	}
	/* form reply message */
	InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
	/* build a new capability */
	/* If the client has Alter permission to the object just use 	*/
	/* the new mask as given, otherwise restrict it by his actual	*/
	/* access. He could get the same effect by changing the objects	*/
	/* matrix which is a lot less safe.			     	*/
	
	unless( CheckMask(req->Common.Access.Access,AccMask_A) )
		newmask &= req->Common.Access.Access; 

	new_cap(&cap, ip, newmask );
	
	/* add the new capability to the message */
	MarshalCap(m,&cap);
	/* send reply message to the client */
	PutMsg(m);

	iput (ip);
}
/* ==================================================================== */

static void 
do_closeobj (ServInfo *servinfo)

/*
*  This function will be available, after the Name Server is modified,
*  so that Entries in the Name Table can also be deleted.
*  Thus providing the possibilty to terminate for the FileServer.
*/

{
IOdebug("	do_closeobj :	 This function not yet implemented!");
	NullFn(servinfo);
}

/* ==================================================================== */

static word IOmsg(Stream *s, char *msg, int size, int timeout)
{
	IOputs(msg);
}

/* end of fserver.c */ 
