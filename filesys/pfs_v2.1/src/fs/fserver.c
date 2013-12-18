/* $Header: /hsrc/filesys/pfs_v2.1/src/fs/RCS/fserver.c,v 1.1 1992/07/13 16:17:41 craig Exp $ */

/* $Log: fserver.c,v $
 * Revision 1.1  1992/07/13  16:17:41  craig
 * Initial revision
 *
 * Revision 2.1  90/08/31  10:54:21  guenter
 * first multivolume/multipartition PFS with tape
 * 
 * Revision 1.7  90/08/08  08:03:17  guenter
 * ParentDir of RootDir is no more RootDir itself
 * 
 * Revision 1.6  90/08/03  13:30:51  guenter
 * multipartition/multivolume
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

                                                                                /*
  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                  (c) 1988-91 by parsytec GmbH, Aachen                   |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |                          Parsytec File System                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  fserver.c							             |
   |                                                                         |
   |    Main function.                                                       |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    5 - O.Imbusch - 22 April     1991 - Error handling centralized       |
   |    4 - O.Imbusch - 26 March     1991 - Optional DevInfo parameter added |
   |    3 - G.Lauven  -  3 August    1990 - Implementation of Multipartition |
   |    2 - H.J.Ermen -  4 January   1989 - Completion of ErrorMsg-codes and |
   |                                        adding of SS_Harddisk-identifiers|
   |                                      - Implementation of do_rename()    |
   |                                      - Implementation of do_link()      |
   |    1 - A.Ishan   - 23 September 1988 - Basic version                    |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#include <root.h>

#define  DEBUG	  0
#define  GEPDEBUG 0
#define  FLDEBUG  0
#define  HDEBUG   1
#include "error.h"

#define  PROCCNT  1
#include "proccnt.h" 

#include "fserr.h"
#include "nfs.h"
#include "partchck.h"

#define	MIN_MEMFREE	30000		/* minimum free space for PFS operation */
#define CHECKER_SIZE	100000		/* memory for checker operation */

#define 	do_syncfsSS		4000
#define 	SayStillAliveSS		4000
#define		fdispatchStackSize	10000
#define		tdispatchStackSize	10000

Semaphore	term_sem;	/* for sync of termination of all volumes */
Semaphore	checker_sem;	/* for serializing checker use */

word		checksum_allowed;	/* global flag for buffer cache	*/
					/* checksum generation enable	*/
word		ReportOpen = FALSE;	/* global flag for open request	*/
					/* report			*/
word		ForkDoSync = TRUE;	/* global flag for Fork(do_syncfs)*/
word		checker_mode;		/* global checker operating mode */
FileSysInfo	*fsi;			/* global filesysteminfo */
Environ		Env;			/* has to be global, so PrintError */
					/* can write to stdio and stderr   */
					
/* ==================================================================== */

/* General procedures */

static void do_syncfs     (void);
static void SayStillAlive (void);

/* ==================================================================== */


int main (void)

/*
 *  Installs the FileServer on a processor with lots of MBytes.
 *  After installation any object of the FileSystem can be accessed 
 *  from any node of the network by the name "/fserver/...".
 */

/*
 *  Meaning of given parameters:
 *
 *
 *	[-c]		: allow buffer cache checksum generation 
 *			  (default is no checksum generation)
 *    
 *      [-o]            : report open request to the server screen
 *
 *      [-s]            : DON'T Fork (do_syncfs)
 *
 *     [ -f		: checker performs full checks (default)  
 *	|-b		: checker performs basic checks
 *	|-n ]		: checker is bypassed completely
 *			  these options determine the general checker mode 
 *			  and can be overridden by the options of the load 
 *			  command; if no checker option is applied to the
 *			  load command the general checker mode is used.
 *   [<Devinfo>]	: path to the devinfo file (default path is
 *			  /helios/etc/devinfo, default dir is /helios/etc)
 *    <Driver>
 */ 
 
{
	char 		*fsname      = NULL,
			*DevInfoName = NULL;
	bool		 IsFirstString = TRUE;	 		
			
	void 		*devinfo;
	InfoNode 	*fsinfo;
	InfoNode 	*deviceinfo;
	DiscDevInfo 	*ddi;	
	word		freesize,needed_size;
	word 		curvol,count;
	word		chksum = FALSE;
	char		**argv;

	bool in_nucleus = (word)main < (word)GetRoot(); /* better test needed */
	
        InitSemaphore (&PESem, 1);
        SemSet = TRUE;

	ConstPC ();

	checker_mode = FULL;
	if (!in_nucleus)
	{	
		GetEnv(MyTask->Port, &Env);
		argv = Env.Argv;
		for(argv++; *argv; argv++ )
		{
			char *arg = *argv;
			if( *arg == '-' )
			{
				switch(arg[1])
				{
				case 'c' : chksum = TRUE; 		break;
				case 'f' : checker_mode = FULL; 	break;	
				case 'b' : checker_mode = BASIC; 	break;
				case 'n' : checker_mode = NO; 		break;
				case 'o' : ReportOpen = TRUE; 		break;
				}
				continue;
			}

/******************************************************************************
*
*  Only one argument
*    YES: its the driver
*    NO:  first argument is the DevInfo file,
*         second one the driver
*
******************************************************************************/
			
			if (IsFirstString) 
			{
				IsFirstString = FALSE;
  				fsname = arg;
  			}
  			else
  			{
  				DevInfoName = fsname;
  				fsname = arg;
  			}
		}

/******************************************************************************
*
*  Driver must be given
*
******************************************************************************/

		if( fsname == NULL )
			Fatal (UsageUnknown, FSErr [UsageUnknown]);

	}

	else fsname = "helios";
	
	checksum_allowed = chksum;
	
        Serious (FSErr [Copyright]);

Report (FSErr [Started], _DATE_, _TIME_);

	if ((devinfo = load_devinfo (DevInfoName)) == NULL ) 
		Fatal (DInotLoaded, FSErr [DInotLoaded]);
		
	if ((fsinfo = find_info(devinfo,Info_FileSys,fsname)) == NULL )
		Fatal (FInotFound, FSErr [FInotFound]);

	fsi = (FileSysInfo *)RTOA(fsinfo->Info);

	if ((deviceinfo = find_info(devinfo,Info_DiscDev,RTOA(fsi->DeviceName)))==NULL)
		Fatal (DInotFound, FSErr [DInotFound]);

	ddi = (DiscDevInfo *) (RTOA (deviceinfo->Info));

	if (IllegalPartitioning (ddi))
		Fatal (IllegalPart, FSErr [IllegalPart]);

	if (!open_dev (ddi))
		Fatal (DevNotOpen, FSErr [DevNotOpen]);

	if (!init_info (fsi,ddi))
	{
          close_dev();
          Fatal (DInotInit, FSErr [DInotInit]);
	}

	if (IllegalAllocParts (ddi))  
	{
          close_dev();
	  Fatal (IllegalAlloc, FSErr [IllegalAlloc]);
	}

/*	fsysinfo.BlockSize = fsi->BlockSize;	*/
/*	fsysinfo.CacheSize = fsi->CacheSize;	*/
	maxnii = fsi->MaxInodes;
	psmal = fsi->SmallPkt;
	pmedi = fsi->MediumPkt;
	phuge = fsi->HugePkt;
	maxpsz = fsi->MaxPktSize;
	pscnt = fsi->SmallCount;
	pmcnt = fsi->MediumCount;
	phcnt = fsi->HugeCount;
#ifdef CHECKER
	possindb = fsi->PossIndir;
	possdirb = fsi->PossDir;
	mingood = fsi->MinGood;
	maxbmerr = fsi->BitMapErrs;

	no_corrections = FALSE;
	silent_mode = TRUE;
#endif	
/*
 *   calculate if there is enough free memory for the complete filesystem
 */
 	freesize = memfree();

GEPdebug (" main() : Amount of free memory : %d byte",freesize);

	needed_size = (pscnt * psmal + pmcnt * pmedi + phcnt * phuge) * BSIZE;
	needed_size += volume[0].num_of_vols * 
			(fdispatchStackSize + tdispatchStackSize)/2;
	needed_size += CHECKER_SIZE;
	
	freesize -= needed_size;
	if (freesize < MIN_MEMFREE)
	{
Error (FSErr [NoMem4PFS], memfree(), needed_size + MIN_MEMFREE);
		close_dev();
		Fatal (NotEnoughMem1, FSErr [NotEnoughMem1]);
	}


/*
 *   init buffer cache and inode list
 */
 	if ( !alloc_mem() )
 	{
		close_dev();
		Fatal (NotEnoughMem2, FSErr [NotEnoughMem2]);
 	}

GEPdebug(" main() : Amount of free memory : %d byte",memfree());

	init_buffer_cache();

	init_incore_ilist();

GEPdebug (" main() : Amount of free memory : %d byte",memfree());

	InitSemaphore ( &sync_sem, 1 );		  /* init sync semaphore */


#if RELEASE
	Fork (do_syncfsSS,     do_syncfs,  0);	  /* start the background process 'do_syncfs' */
#else	
	if (ForkDoSync)
	{
		Fork (do_syncfsSS,     do_syncfs,  0);	  /* start the background process 'do_syncfs' */
		FLdebug ("\n\n\n\ndo_syncfs FORKED\n\n\n\n");
	}
	else
	{
		FLdebug ("\n\n\n\ndo_syncfs NOT FORKED\n\n\n\n");
	}
#endif

#if 0
	Fork (SayStillAliveSS, SayStillAlive, 0); /* start the background process 'SayStillAlive' */
#endif
	
	

	InitSemaphore ( &term_sem,1 - (volume->num_of_vols) );
	InitSemaphore ( &checker_sem,1 );

	for (curvol = 0; curvol < volume[0].num_of_vols; curvol++) {	
		if ( volume[curvol].raw ) {

GEPdebug (" fserver : forking tdispatch() for volume %d",curvol);

			if ( Fork(tdispatchStackSize, tdispatch, 4, &volume[curvol]) == 0) {
Error (FSErr [NoMem4Tdisp], curvol);
				for (count = curvol; count < volume[0].num_of_vols; count ++)
					Signal ( &term_sem );
				break;		
			}	
GEPdebug (" main() : Amount of free memory : %d byte",memfree());

		}

		else {
GEPdebug (" fserver : forking fdispatch() for volume %d",curvol);

			if ( Fork(fdispatchStackSize, fdispatch, 4,
						 &volume[curvol] ) == 0)
			{
				Error (FSErr [NoMem4Tdisp], curvol);
				for (count = curvol; count < volume[0].num_of_vols; count ++)
					Signal ( &term_sem );
				break;		
			}	
GEPdebug (" main() : Amount of free memory : %d byte",memfree());

		}
	}

	Wait ( &term_sem );	/* wait for termination of all volumes here */
	
	close_dev();
	Free (devinfo);
	DestPC ();

	Report (FSErr [Finished]);

	Exit (FSOK);		
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

static void SayStillAlive (void)
{
  forever 
  {
    Delay (15 * 60 * OneSec);
    Report (FSErr [StillAlive]);
  }
}

/* ==================================================================== */
/* ==================================================================== */

void do_open (ServInfo *servinfo, 
              VD       *vol)

/*
*  Opens a stream to the named object.
*  Returns a pointer to the opened stream structure,
*  and thus provides a Stream Port 
*  for Stream Operations on this object.
*/

{
	MCB 		*m = servinfo->m;
	MsgBuf 		*r;
	IOCMsg2 	*req = (IOCMsg2 *)(m->Control);
	Port 		reqport;
	char 		*data = m->Data;
	char 		*pathname = servinfo->Pathname;
	struct incore_i *iip, *ip;
#if DEBUG
	char		*name;
#endif	
	
DEBdebug("	do_open >%s/%s<",pathname,&data[req->Common.Next]);

        IncPC (90);

	if (ReportOpen)
		IOdebug ("Supposed to open %s", pathname);

#define RETURN DecPC (90); return

	/* get Parent Dir of object */
	iip = get_target_dir (servinfo,vol);

	/* if no Parent Dir found */

	if (!servinfo->ParentOfRoot && !iip)
	{
		IncPC (40);
		if (vol->sync_allowed)
			ErrorMsg(m,EO_Directory);
		else
			ErrorMsg(m,EO_Medium);

DEBdebug ("	do_open :	No such parent directory >%s< !",pathname);

		Wait (&vol->dircnt_sem);
		DecPC (40); RETURN;
	}

	/* get the target object */
	ip = get_target_obj (servinfo,iip,vol);
	if ( !ip && !vol->sync_allowed )
	{
		ErrorMsg(m,EC_Error+EG_Broken+EO_Medium);
		Wait (&vol->dircnt_sem);
		RETURN;
	}

	/* alloc memory for buffering stream messages */
	r = New(MsgBuf);

	if( r == Null(MsgBuf) )
	{
		IncPC (42);
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Message);

DEBdebug ("	do_open :	No memory for message buffering !");

		if (ip)
			iput (ip);
		if (iip)
			iput (iip);
		Wait (&vol->dircnt_sem);
		DecPC (42); RETURN;
	}

	/* if target object doesn't exist and if the open mode is create mode */
	if((iip) && ( !ip && (req->Arg.Mode & O_Create) ))
	{
		m->MsgHdr.FnRc = SS_HardDisk;

		/* check that we are allowed to create a file here */
		unless( CheckMask(req->Common.Access.Access,AccMask_W) 
				&& !vol->writeprotected )
		{
			IncPC (43);
			if ( vol->writeprotected )
				ErrorMsg(m,EC_Error+EG_Protected+EO_Medium);
			else
				ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);

FLdebug ("No access permission.");

			iput( iip );
			Free(r);
			Wait (&vol->dircnt_sem);
			DecPC (43); RETURN;
		}
		if (vol->vol_full)
		{
			IncPC (44);

DEBdebug ("	do_open :	Cannot create >%s< !",pathname);

			ErrorMsg(m,EC_Error|EG_NoMemory);
			iput (iip);
			Free(r);
			Wait (&vol->dircnt_sem);
			DecPC (44); RETURN;
		}
		/* create the specified object */
		ip = make_obj (iip, pathname, Type_File, "do_not_iput");
		if ( (ip == NULL) && (!vol->sync_allowed))
		{
			IncPC (45);
			ErrorMsg(m,EC_Error|EG_Broken|EO_Medium);
			iput (iip);
			Free(r);
			Wait (&vol->dircnt_sem);
			DecPC (45); RETURN;
		}
		/* give creators all rights to new object */
		req->Common.Access.Access = AccMask_Full;
	}

	/* if there's still no target object */
	if (!ip)
	{
		IncPC (46);

DEBdebug ("	do_open :	No such file >%s< !",pathname);

		ErrorMsg(m,EC_Error|EG_Create|EO_File);
		if (iip)
			iput (iip);
		Free(r);
		Wait (&vol->dircnt_sem);
		DecPC (46); RETURN;
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
		IncPC (47);
		ErrorMsg(m,EC_Error+EG_Protected+EO_File);

FLdebug ("No access permission.");

		iput (ip);
		if (iip)
			iput (iip);
		Free(r);
		Wait (&vol->dircnt_sem);
		DecPC (47); RETURN;
	}

	/* form a reply message */
	IOCRep1 (r, m, ip, Flags_Server|Flags_Closeable, pathname);

	/* create a new Stream Port */
	reqport = DebNewPort();
	/* add info about Stream Port to message */
	r->mcb.MsgHdr.Reply = reqport;
	/* send message to client */
	PutMsg(&r->mcb);
	Free(r);
	
	/* at this point, do_open transforms to a stream-worker */	

	/* decrement the directory worker counter */
	Wait (&vol->dircnt_sem);
	
	/* increment the number of active stream-worker processes */
	Signal (&vol->streamcnt_sem);
	Signal (&vol->streammsg_sem);
	
	/* if target object is of type directory
	   then call special routine to handle
	   Stream Operations on directories */
	if (ip->ii_i.i_mode == Type_Directory) 
	{
		IncPC (49);
		/* call the appropriate routine */
		if (!dir_server (ip,iip,m,reqport))
		{
			IncPC (48);
  			DebFreePort(reqport);
			Wait (&vol->streamcnt_sem);
			Wait (&vol->streammsg_sem);
			DecPC (48); DecPC (49); RETURN;
		}
		if (!iput (ip))
			ErrorMsg(m,EC_Error|EG_Broken|EO_Medium);
		if (iip)
			if (!iput (iip))
				ErrorMsg(m,EC_Error|EG_Broken|EO_Medium);
		/* free Stream Port */
		DebFreePort(reqport);
		Wait (&vol->streamcnt_sem);
		Wait (&vol->streammsg_sem);
		DecPC (49); RETURN;
	}
	if (iip)
		if (!iput (iip))
		{
			IncPC (50);
			ErrorMsg(m,EC_Error|EG_Broken|EO_Medium);
			DebFreePort(reqport);
			Wait (&vol->streamcnt_sem);
			Wait (&vol->streammsg_sem);
			DecPC (50); RETURN;
		}

	if( req->Arg.Mode & O_Truncate )
	{
		/* delete contents if it isn't a directory */
		if ( (ip->ii_i.i_mode == Type_File) && !vol->writeprotected ) {
			if (!setsize_file (ip, 0))
			{
				IncPC (51);
				ErrorMsg(m,EC_Error+EG_Broken+EO_Medium);	
FLdebug ("\n\n\nBINGO\n\n\n");				
				DebFreePort (reqport);
				Wait (&vol->streamcnt_sem);
				Wait (&vol->streammsg_sem);
				DecPC (51); RETURN;
			}
		}
	}

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
	
	/* following lines deal with Stream Operations
	   on normal objects (no directories) */
	forever
	{
		word e;
		m->MsgHdr.Dest = reqport;
		m->Timeout = StreamTimeout;
		m->Data = data;


Hdebug ("<W");
		Wait (&vol->streammsg_sem);
Hdebug (">W");
		
		/* get stream message and check its validity */
Hdebug ("<GM");
		e = GetMsg(m);
Hdebug (">GM");

		if (TestSemaphore (&vol->servreq_sem) < 1)
                {
			while (	(TestSemaphore (&vol->servreq_sem) < 1) &&
				vol->terminate_flag == FALSE)
			{
Hdebug ("<D");
				Delay (OneSec);
Hdebug (">D");
			}
			if ( (TestSemaphore (&vol->servreq_sem) < 1) &&
				vol->terminate_flag == TRUE) {
				m->MsgHdr.FnRc &= ~FG_Mask;
				m->MsgHdr.FnRc |= FG_Close;
				e = Err_Null;

Hdebug (" do_open() : Volume /%s, FG_Close constructed!",vol->vol_name);

			}
			
		}		
Hdebug ("<S");
		Signal (&vol->streammsg_sem);
Hdebug (">S");
		
		if( e == EK_Timeout ) 
		{
Hdebug ("break");
			break;
		}
		if( e < Err_Null ) 
		{
Hdebug ("cont");
			continue;
		}

DEBdebug ("	do_open :	Stream message is 0x%x",m->MsgHdr.FnRc);

 		if( MyTask->Flags & Task_Flags_servlib )
			Report (FSErr [OnlineDebug], m->MsgHdr.FnRc,pathname);

Hdebug (FSErr [OnlineDebug], m->MsgHdr.FnRc,pathname);

		switch( m->MsgHdr.FnRc & FG_Mask )
		{
		case FG_Read:
		{
			if( req->Arg.Mode & O_ReadOnly ) {
				if ( !read_file(m,ip) )
				{
					IncPC (52);
 					if (m->MsgHdr.Reply != NullPort)
 						ErrorMsg(m,Err_Null);
					iput (ip);
					/* give Stream Port free */
					DebFreePort (reqport);

DEBdebug ("	do_open : (FG_Read) closing >%s<",pathname);

					Wait (&vol->streamcnt_sem);
					Wait (&vol->streammsg_sem);
					DecPC (52); RETURN;
				}
 				elif (m->MsgHdr.Reply != NullPort)
 					ErrorMsg(m,Err_Null);
			}
			else
				ErrorMsg(m,EC_Error+EG_Protected+EO_File);
			break;
		}

		case FG_Write:
		{
			if (vol->writeprotected)
				ErrorMsg(m,EC_Error+EG_Protected+EO_Medium);
			elif (!(req->Arg.Mode & O_WriteOnly))
				ErrorMsg(m,EC_Error+EG_Protected+EO_File);
			elif (!write_file(m,ip))
			{
				IncPC (53);
				if (m->MsgHdr.Reply != NullPort)
					ErrorMsg(m,Err_Null);
				iput (ip);
				/* give Stream Port free */
				DebFreePort (reqport);

DEBdebug ("	do_open : (FG_Write) closing >%s<",pathname);

				Wait (&vol->streamcnt_sem);
				Wait (&vol->streammsg_sem);
				DecPC (53); RETURN;
			}
			elif (m->MsgHdr.Reply != NullPort) 
				ErrorMsg(m,Err_Null);

			break;
		}
		
		case FG_Close:
		{
                        IncPC (54);
			if(m->MsgHdr.Reply != NullPort)
				ErrorMsg(m,Err_Null);
			iput (ip);
			/* give Stream Port free */
			DebFreePort (reqport);

DEBdebug ("	do_open : (FG_Close) closing >%s<",pathname);

			Wait (&vol->streamcnt_sem);
			Wait (&vol->streammsg_sem);
			DecPC (54); RETURN;
		}

		case FG_GetSize:
		{
                        IncPC (2);
			if(  req->Arg.Mode & O_ReadOnly 
			  || req->Arg.Mode & O_WriteOnly )
			{
				InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
				MarshalWord(m,ip->ii_i.i_size);
				PutMsg(m);
			}
			else
				ErrorMsg(m,EC_Error+EG_Protected+EO_File);
                        DecPC (2);
			break;
		}

		case FG_Seek:
		{
                        IncPC (3);
			if(  req->Arg.Mode & O_ReadOnly 
			  || req->Arg.Mode & O_WriteOnly )
				seek_file(m,ip);		
			else
				ErrorMsg(m,EC_Error+EG_Protected+EO_File);
                        DecPC (3);
			break;
		}

		case FG_SetSize:
		{
			word newsize = m->Control[0];
			
                        IncPC (4);
			if ( vol->writeprotected )
				ErrorMsg(m,EC_Error+EG_Protected+EO_File);
			elif( ! (req->Arg.Mode & O_WriteOnly) )
				ErrorMsg(m,EC_Error+EG_Protected+EO_File);
			/* only truncation permitted */
			elif( newsize > ip->ii_i.i_size )
				ErrorMsg(m,EC_Error+EG_Parameter+1);
			else 
			{
				if (!setsize_file (ip, newsize))
				{
					IncPC (55);

					ErrorMsg(m,EC_Error+EG_Broken+EO_Medium);
					DebFreePort (reqport);
					Wait (&vol->streamcnt_sem);
					Wait (&vol->streammsg_sem);
                                        DecPC (4);
					DecPC (55); RETURN;	
				}
				InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
				MarshalWord(m,ip->ii_i.i_size);
				PutMsg(m);
			}
                        DecPC (4);
			break;
		}

		default:
		{
                        IncPC (5);
			ErrorMsg(m,EC_Error+EG_FnCode+EO_File);
                        DecPC (5);
			break;
		}

		}
	}
	Wait (&vol->streamcnt_sem);
	Wait (&vol->streammsg_sem);
	DebFreePort (reqport);
        RETURN;
#undef RETURN
}

/* ==================================================================== */

void 
do_create(ServInfo *servinfo, VD *vol)

/*
*  Creates an object of the given type.
*  Returns a pointer to the created object.
*/

{
	MCB 		*m = servinfo->m;
	MsgBuf 		*r;
	IOCCreate 	*req = (IOCCreate *)(m->Control);
	char 		*pathname = servinfo->Pathname;

	struct incore_i *iip, *ip;
#if DEBUG
	char		*data = m->Data;
	
DEBdebug ("	do_create >%s/%s<",pathname,&data[req->Common.Next]);
#endif

	IncPC (91);
	
	/* get Parent Dir of object */
	iip = get_target_dir (servinfo,vol);
	/* if no such Parent Dir */
	if (!iip)
	{
		if (vol->sync_allowed)
			ErrorMsg(m,EO_Directory);
		else
			ErrorMsg(m,EO_Medium);

DEBdebug ("	do_create :	No such parent directory >%s< !",pathname);

		DecPC (91); return;
	}
	/* check if client has permission to write in Parent Dir */
	unless( CheckMask(req->Common.Access.Access,AccMask_W) 
			&& !vol->writeprotected ) 
	{
		if ( vol->writeprotected )
			ErrorMsg(m,EC_Error+EG_Protected+EO_Medium);
		else
			ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);

FLdebug ("No access permission.");

		iput (iip);
		DecPC (91); return;
	}
	/* alloc memory for message buffering */
	r = New(MsgBuf);
	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Message);

DEBdebug ("	do_create :	No memory for message buffering !");

		iput (iip);
		DecPC (91); return;
	}

	ip = get_target_obj (servinfo, iip, vol);

	if (ip)
	{
		m->MsgHdr.FnRc |= EC_Error+EG_Create+EO_File;

DEBdebug ("	do_create :	File already exists >%s< !",pathname);

		iput (ip);
		iput (iip);
		goto NullFile;
	}
	else {
		m->MsgHdr.FnRc = SS_HardDisk;
		if (!vol->sync_allowed) {
			Free (r);
			DecPC (91); return;
		}
	}
		
	if (vol->vol_full)
	{

DEBdebug ("	do_create :	Cannot create >%s< !",pathname);

			ErrorMsg(m,EC_Error|EG_NoMemory);
			iput (iip);
			Free(r);
			DecPC (91); return;
	}

	if ( req->Type == Type_Directory ) 
	{
		/* create a directory */
		ip = make_obj (iip, pathname, Type_Directory, (string)NULL);
		if ( !ip ) {
			ErrorMsg(m,EC_Error|EG_Broken|EO_Medium);
			iput (iip);
			Free(r);
			DecPC (91); return;	
		}
	} 
	else 
	{	
		/* create a normal file */
		ip = make_obj (iip, pathname, Type_File, (string)NULL);
		if ( !ip ) {
			ErrorMsg(m,EC_Error|EG_Broken|EO_Medium);
			iput (iip);
			Free(r);
			DecPC (91); return;	
		}
	}
	
	/* if create couldn't succeed */
	if (!ip)
	{

DEBdebug ("	do_create :	Create couldn't succeed !");

NullFile:	ErrorMsg(m,EO_Object);
		Free(r);
		DecPC (91); return;
	}

	/* give creators all rights to new object */
	req->Common.Access.Access = AccMask_Full;

	/* form reply message with info about created object */
	IOCRep1 (r, m, ip, 0, pathname);
	/* send message to client */
	PutMsg(&r->mcb);

	iput (ip);
	Free(r);
	DecPC (91);
}

/* ==================================================================== */

void 
do_locate(ServInfo *servinfo, VD *vol)

/*
*  Tests the existance of the named object.
*  Returns a pointer to the specified object,
*  in order to provide further access.
*/

{
	MCB 		*m = servinfo->m;
	MsgBuf 		*r;
	IOCCommon 	*req = (IOCCommon *)(m->Control);
	char 		*pathname = servinfo->Pathname;

	struct incore_i *ip,*iip;
#if DEBUG
	char		*data = m->Data;

DEBdebug ("	do_locate >%s/%s<",pathname,&data[req->Next]);
#endif	

	IncPC (92);

	/* get Parent Directory */
	iip = get_target_dir (servinfo,vol);	

	/* if Parent Dir doesn't exist */

	if (!servinfo->ParentOfRoot && !iip)
	{
		if (vol->sync_allowed)
		{
FLdebug ("EO_Directory");			
			ErrorMsg(m,EO_Directory);
		}
		else
		{
FLdebug ("EO_Medium");			
			ErrorMsg(m,EO_Medium);
		}

DEBdebug ("	do_locate :	No such parent directory >%s< !",pathname);

		DecPC (92); return;
	}

	/* get target object */
	ip = get_target_obj (servinfo, iip, vol);

	if (iip)
		if (!iput (iip))
		{
FLdebug ("EO_Medium");			
			ErrorMsg(m,EO_Medium);
			DecPC (92); return;
		}
	
	/* if no such target object */
	if (!ip)
	{
		if (vol->sync_allowed)
		{
FLdebug ("EO_Object");			
			ErrorMsg(m,EO_Object);
		}
		else
		{
FLdebug ("EO_Medium");			
			ErrorMsg(m,EO_Medium);
		}

FLdebug ("	do_locate :	No such file >%s< !",pathname);

		DecPC (92); return;
	}

	/* The client is only allowed to know about the object 
	   if he has any access at all.	*/
	   
#if 1 /*EXABYTE-TEST*/
	unless( CheckMask(req->Access.Access,AccMask_Full) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Object);

FLdebug ("No access permission.");

		iput (ip);
		DecPC (92); return;
	}
#endif
	/* alloc memory for message buffering */
	r = New(MsgBuf);
	if( r == Null(MsgBuf) )
	{
FLdebug ("EC_Error+EG_NoMemory+EO_Message");
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Message);

DEBdebug ("	do_locate :	No memory for message buffering !");

		iput (ip);
		DecPC (92); return;
	}

	/* form reply message with info of target object */
	IOCRep1 (r, m, ip, 0, pathname);
	/* send message to client */
	PutMsg(&r->mcb);

	iput (ip);
	Free(r);
	DecPC (92);
}

/* ==================================================================== */

void 
do_objinfo (ServInfo *servinfo, VD *vol)

/*
*  Obtains any extra information on the named object.
*  This includes size, dates, protection, and so forth.
*/

{
	MCB 		*m = servinfo->m;
	IOCMsg1 	*req = (IOCMsg1 *)(m->Control);
	struct incore_i *ip,*iip;

#if DEBUG
	char 		*data = m->Data;
	char 		*pathname = servinfo->Pathname;

DEBdebug ("	do_objinfo >%s/%s<",pathname,&data[req->Common.Next]);
#endif	

	IncPC (93);
		
	/* get Parent Dir */
	iip = get_target_dir (servinfo,vol);

	/* if no such Parent Dir */

	if (!servinfo->ParentOfRoot && !iip)
	{
		if (vol->sync_allowed)
		{
			ErrorMsg(m,EO_Directory);
		}
		else
		{
			ErrorMsg(m,EO_Medium);
		}

DEBdebug ("	do_objinfo :	No such parent directory >%s< !",pathname);

		DecPC (93); return;
	}

	ip = get_target_obj (servinfo, iip, vol);

	if (iip)
		if (!iput (iip)) {
			ErrorMsg(m,EO_Medium);
			DecPC (93); return;	
		}

	/* if no such target object */
	if (!ip)
	{
		if (vol->sync_allowed)
			ErrorMsg(m,EO_Object);
		else
			ErrorMsg(m,EO_Medium);

DEBdebug ("	do_objinfo :	No such file >%s< !",pathname);

		DecPC (93); return;
	}

#if 1 /*EXABYTE-TEST*/
	/* only allow if user has any access at all */
	unless( CheckMask(req->Common.Access.Access,AccMask_Full) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Object);

FLdebug ("No access permission.");

		iput (ip);
		DecPC (93); return;
	}
#endif	
	/* form reply message with extended info about object */
	if (!marshal_info (m, ip)) {
		ErrorMsg(m,EC_Error+EG_Broken+EO_Medium);
		DecPC (93); return;
	}
	/* send message to client */
	PutMsg(m);

	iput (ip);
	DecPC (93);
}

/* ==================================================================== */

void 
do_serverinfo (ServInfo *servinfo, VD *vol)
{
 	MCB 		*m = servinfo->m;
 	IOCMsg1 	*req = (IOCMsg1 *) m->Control;
 	word 		fssize, fsused, fsfree;
	word 		d = Flags_Server;
#if DEBUG
        char 		*pathname = servinfo->Pathname;
#endif

DEBdebug ("	do_serverinfo :	>%s<", pathname);

	IncPC (94);

					 /* only allow if user has any	*/
					 /* access at all 		*/
	unless ( CheckMask (req->Common.Access.Access, AccMask_Full) )
 	{
 		ErrorMsg (m, EC_Error+EG_Protected);
 	
FLdebug ("No access permission.");

		DecPC (94); return;
 	}
 
	fssize = BSIZE * vol->incore_fs.fs_dsize;
	fsfree = BSIZE * vol->incore_sum.is_fs.sum.s_nbfree;
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
	DecPC (94);
}

/* ==================================================================== */

void 
do_delete(ServInfo *servinfo, VD *vol)

/*
 *  Delete the named object.
 */

{
 MCB 		*m = servinfo->m;
 IOCCommon 	*req = (IOCCommon *)(m->Control);
 char 		*pathname = servinfo->Pathname;
 struct incore_i *ip, *iip;		
 word		dlgbnr;
#if DEBUG
 char 		*data = m->Data;

DEBdebug ("	do_delete >%s/%s<",pathname,&data[req->Next]);
#endif		

	IncPC (95);


 /* get Parent Dir of object */
 iip = get_target_dir (servinfo,vol);
 /* if no such Parent Dir */
 if (!iip)
 {
 	if (vol->sync_allowed)
 		ErrorMsg(m,0);
 	else
 		ErrorMsg(m,EO_Medium);

DEBdebug (" do_delete : No such parent directory >%s< !",pathname);

	DecPC (95); return;
 }
 /* get the target object */
 ip = get_target_obj (servinfo, iip, vol);
/*
 if (ip == iip)
 {
	ErrorMsg(m,EC_Error+EG_Unknown);

DEBdebug ("	do_delete :	No such file >%s< !",pathname);

	iput (iip);
	iput (ip);
	DecPC (95); return;
 }
*/ 
 /* if no such target object */
if (!ip)
 {
 	if (vol->sync_allowed) 
 		ErrorMsg(m,0);
 	else
 		ErrorMsg(m,EO_Medium);

DEBdebug ("	do_delete :	No such file >%s< !",pathname);

	iput (iip);
	DecPC (95); return;
 }
	
 /* check the delete permissions */
 unless( CheckMask(req->Access.Access,AccMask_D) && !vol->writeprotected ) 
 {
	if ( vol->writeprotected )
		ErrorMsg(m,EC_Error+EG_Protected+EO_Medium);
	else
		ErrorMsg(m,EC_Error+EG_Protected+EO_File);

FLdebug ("No access permission.");

	iput (ip);
	iput (iip);		
	DecPC (95); return;
 }

 /* the root-dir should never be deleted ! */
 if ( ip->ii_i.i_mode == Type_Directory &&
      ! strcmp ( strstr(pathname,vol->vol_name), vol->vol_name) )
 {
 	ErrorMsg (m, EC_Error+EG_Delete+EO_Directory);

DEBdebug ("	do_delete :	Attempt to delete root-directory");

        iput (ip);
	iput (iip);
	DecPC (95); return;
 }
 	
 /* directories can be only deleted 
    if they don't have any DirEntries */
 if( ip->ii_i.i_mode == Type_Directory &&
	ip->ii_i.i_spare != 0 )
 {
	ErrorMsg(m,EC_Error+EG_Delete+EO_Directory);

DEBdebug ("	do_delete :	Directory has files !");

	iput (ip);
	iput (iip);
	DecPC (95); return;
 }

 if( ip->ii_count > 1 )
 {
	ErrorMsg(m,EC_Error+EG_InUse+EO_File);

DEBdebug ("	do_delete :	Other user working on file !");

	iput (ip);
	iput (iip);
	DecPC (95); return;
 }

 /* following lines deal with deleting of normal files */
 Wait (&ip->ii_sem);
 /* give free the data blocks on disc */
 if (ip->ii_i.i_size) 
 {
	dlgbnr = (ip->ii_i.i_size-1)/BSIZE+1;
	if (!fREE (ip, 0, dlgbnr)) {
		PutMsg(&vol->unload_mcb);
		ErrorMsg(m,EC_Error+EG_Broken+EO_Medium);	
		DecPC (95); return;
	}
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
	DecPC (95);
}
/* ==================================================================== */

void 
do_rename (ServInfo *servinfo, VD *vol)

{
	MCB 		*m = servinfo->m;
	char 		*data = m->Data;
	IOCMsg2 	*req = (IOCMsg2 *)(m->Control);
	word 		hdr = *(word *)m;
	AccMask 	mask = req->Common.Access.Access;
	struct incore_i *iip, *iip2, *ip, *ip2;
	char 		name2[NameMax];
	char 		*pathname = servinfo->Pathname;
	ServInfo 	servinfo2;
	word		dlgbnr;
#if DEBUG
	char 		name[NameMax];
#endif	

/*debug("DoRename %x %x %s",m,context,pathname);*/

DEBdebug ("	do_rename >%s/%s<",pathname,&data[req->Common.Next]);

	IncPC (96);

	memcpy ((void *)&servinfo2,(void *)servinfo,sizeof(ServInfo));
	
	iip = get_target_dir(servinfo,vol);

	if( !iip )
	{
		if (vol->sync_allowed)
			ErrorMsg(m,EO_Directory);
		else
			ErrorMsg(m,EO_Medium);

DEBdebug ("	do_rename :	No such source parent directory >%s< !",pathname);

		DecPC (96); return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_W) 
			&& !vol->writeprotected ) 
	{
		if ( vol->writeprotected )
			ErrorMsg(m,EC_Error+EG_Protected+EO_Medium);
		else
			ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);

FLdebug ("No access permission for source dir.");

		iput (iip);
		DecPC (96); return;
	}

	ip = get_target_obj (servinfo, iip, vol);

	if(ip == iip)
	{
		ErrorMsg(m,EC_Error+EG_Unknown);

DEBdebug ("	do_rename :	No such source file >%s< !",pathname);

		iput (iip);
		iput (ip);
		DecPC (96); return;
	}

	if(!ip)
	{
		if (vol->sync_allowed)
			ErrorMsg(m,0);
		else
			ErrorMsg(m,EO_Medium);

DEBdebug ("	do_rename :	No such source file >%s< !",pathname);

		iput (iip);
		DecPC (96); return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_D) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_File);

FLdebug ("No access permission for source file.");

		iput (ip);
		iput (iip);
		DecPC (96); return;
	}

	/* the root-dir should never be renamed ! */
	if ( ip->ii_i.i_mode == Type_Directory &&
      		! strcmp ( strstr(pathname,vol->vol_name), vol->vol_name) )
 	{
 		ErrorMsg (m, EC_Error+EG_Delete+EO_Directory);

DEBdebug ("	do_rename :	Attempt to delete root-directory");

		iput (ip);
		iput (iip);
		DecPC (96); return;
	}
 	
 	if( ip->ii_count > 1 )
 	{
		ErrorMsg(m,EC_Error+EG_InUse+EO_File);

DEBdebug ("	do_rename :	Other user working on source file !");

		iput (ip);
		iput (iip);
		DecPC (96); return;
	 }

	/* we now have the source object, find the dest directory */
	/* restore state to context dir				  */

	*(word *)m = hdr;
	req->Common.Access.Access = mask;
	req->Common.Next = req->Arg.ToName;

	iip2 = get_target_dir( &servinfo2, vol );

	if( !iip2 )
	{
		if (vol->sync_allowed)
			ErrorMsg(m,EO_Directory);
		else
			ErrorMsg(m,EO_Medium);

DEBdebug ("	do_rename :	No such dest parent directory >%s< !",servinfo2.Pathname);

		iput (ip);
		iput (iip);
		DecPC (96); return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_W) ) 
	{
		ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);

FLdebug ("No access permission for dest dir.");

		iput(ip);
		iput(iip);
		iput(iip2);
		DecPC (96); return;
	}
		
	if( splitname(name2, c_dirchar, &data[req->Common.Next]) == 0 )
	{
		ErrorMsg(m,EC_Error+EG_Name);

DEBdebug ("	do_rename :	Mal formed dest name !");

		iput(ip);
		iput(iip);
		iput(iip2);
		DecPC (96); return;
	}
		
		
	ip2 = get_target_obj ( &servinfo2, iip2, vol);
	if( ip2 )
	{
		if (ip2 == iip2)
		{
			ErrorMsg(m,EC_Error+EG_Unknown);

DEBdebug ("	do_rename :	No such dest file !");

			iput (ip);
			iput (iip);
			iput (iip2);
			iput (ip2);
			DecPC (96); return;
		}   
		if (ip2->ii_i.i_mode == DIR 
		    && ip2->ii_i.i_spare != 0)
		{
			ErrorMsg(m,EC_Error+EG_Protected+EO_File);

DEBdebug ("	do_rename :	Dest directory has files !");

			iput (ip);
			iput (iip);
			iput (ip2);
			iput (iip2);
			DecPC (96); return;
		}   

		unless( CheckMask(req->Common.Access.Access, AccMask_D) ) 
		{
			ErrorMsg(m,EC_Error+EG_Protected+EO_File);

FLdebug ("No access permission for dest file.");

			iput (ip);
			iput (iip);
			iput (ip2);
			iput (iip2);
			DecPC (96); return;
		}

	 	if( ip2->ii_count > 1 )
 		{
			ErrorMsg(m,EC_Error+EG_InUse+EO_File);

DEBdebug ("	do_rename :	Other user working on destination file !");

			iput (ip);
			iput (iip);
			iput (ip2);
			iput (iip2);
			DecPC (96); return;
		 }

		Wait ( &ip2->ii_sem );	/* Get access to the inode	*/
	   	if ( ip2->ii_i.i_size != 0 )	
		{			/* There are blocks to free	*/
			dlgbnr = (ip2->ii_i.i_size - 1) / BSIZE + 1;
			if (!fREE (ip2, 0, dlgbnr)) {
				PutMsg(&vol->unload_mcb);
				ErrorMsg(m,EC_Fatal+EG_Broken+EO_Medium);	
				DecPC (96); return;
			}
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
	else {
		m->MsgHdr.FnRc = SS_HardDisk;	
		if (!vol->sync_allowed) {
			ErrorMsg(m,EC_Error|EG_Broken|EO_Medium);
			DecPC (96); return;
		}
	}

DEBdebug ("PATHNAME >%s<",pathname);
DEBdebug ("NEWNAME >%s<",servinfo2.Pathname);

	/* if we get here we have the source and dest directories	*/
	/* the object itself, and the new name in name2.		*/
	/* now do the rename.						*/

	if (iip == iip2)
	{
		Wait (&ip->ii_sem);
		ip->ii_i.i_mtime = GetDate();
		ip->ii_changed = TRUE;
		Signal (&ip->ii_sem);
		iput(ip);
		
		if ( !make_obj (iip, pathname, -1, name2) ) {
			if (!vol->sync_allowed) {
				ErrorMsg(m,EC_Error|EG_Broken|EO_Medium);
				iput(iip2);
				DecPC (96); return;	
			}
		}
		iput(iip2);
	}
	else
	{
		if (vol->vol_full)
		{

DEBdebug ("	do_rename :	Cannot create >%s< !",servinfo2.Pathname);

			ErrorMsg(m,EC_Error|EG_NoMemory);
			iput (iip);
			iput (ip);
			iput (iip2);
			DecPC (96); return;
		}

	 	ip2 = make_obj (iip2, servinfo2.Pathname, DATA, (string)NULL);
	 	if ( !ip2 )
 		{				/* Unable to create an entry	*/
 			ErrorMsg (m, EC_Error+EG_Create);

DEBdebug ("	do_rename :	Unable to create target-object");

			iput (iip);
			iput (ip);
			DecPC (96); return;
 		}
 	
 		Wait ( &ip->ii_sem );		/* Lock the source inode	*/
 		Wait ( &ip2->ii_sem );		/* Get exclusive target access	*/
 						/* Copy disk inode's content	*/
		memcpy ((void *) &ip2->ii_i, (void *) &ip->ii_i, 
			sizeof(struct inode));
 	
		ip2->ii_i.i_mtime = GetDate();
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
	DecPC (96);
}

/* ==================================================================== */

void
do_link (ServInfo *servinfo, VD *vol)
{
	MCB 		*m = servinfo->m;
	IOCMsg3 	*req = (IOCMsg3 *)(m->Control);
	byte 		*data = m->Data;
	char 		*pathname = servinfo->Pathname;
	struct incore_i *iip, *ip;
	pkt_t 		data_pkt;
	struct buf 	*ldata_ptr;
#if DEBUG
	char 		name[NameMax]; 
#endif	


DEBdebug ("	do_link >%s/%s<",pathname,&data[req->Common.Next]);

	IncPC (97);

	iip = get_target_dir(servinfo,vol);

	if( ! iip )
	{
		if (vol->sync_allowed) 	
			ErrorMsg(m,EO_Directory);
		else
			ErrorMsg(m,EO_Medium);

DEBdebug ("	do_link :	No such parent directory >%s< !",pathname);

		DecPC (97); return;
	}

	unless( CheckMask(req->Common.Access.Access,AccMask_W) 
			&& !vol->writeprotected ) 
	{
		if ( vol->writeprotected )
			ErrorMsg(m,EC_Error+EG_Protected+EO_Medium);
		else
			ErrorMsg(m,EC_Error+EG_Protected+EO_Directory);

FLdebug ("No access permission.");

		iput (iip);
		DecPC (97); return;
	}

	ip = get_target_obj (servinfo, iip, vol);
		
	if( ip )
	{
		ErrorMsg(m,EC_Error+EG_Create);

DEBdebug ("	do_link :	File already exists >%s< !",pathname);

		iput (ip);
		iput (iip);
		DecPC (97); return;
	}
        else {
        	m->MsgHdr.FnRc = SS_HardDisk;
        	if (!vol->sync_allowed) {
			ErrorMsg(m,EC_Error+EG_Broken+EO_Medium);
        		DecPC (97); return;	
        	}
        }
        	
	/* We now know that there is not an existing entry with the	*/
	/* desired name. Install the link.				*/

	if (vol->vol_full)
	{
		ErrorMsg (m, EC_Error+EG_NoMemory+EO_Link);

DEBdebug ("	do_link :	Link alloc couldn't succeed !");

		iput (iip);
		DecPC (97); return;
	}
	
	ip = make_obj (iip, pathname, Type_Link, (string)NULL);

	if ( !ip )
	{
		ErrorMsg(m, EC_Error+EG_NoMemory+EO_Link);

DEBdebug ("	do_link :	Link couldn't succeed !");

		DecPC (97); return;
	}

	Wait ( &ip->ii_sem );

	if (vol->vol_full)
	{
		ErrorMsg (m, EC_Error+EG_NoMemory+EO_Link);

DEBdebug ("	do_link :	Link alloc couldn't succeed !");

		Signal (&ip->ii_sem);
		iput (ip);
		DecPC (97); return;
	}
	data_pkt = alloc (ip, 0, 1);
	if ( !data_pkt.bcnt )
	{
		ErrorMsg (m, EC_Error+EG_NoMemory+EO_Link);

DEBdebug ("	do_link :	Link alloc couldn't succeed !");

		Signal (&ip->ii_sem);
		iput (ip);
		DecPC (97); return;
	}

	ip->ii_i.i_size = strlen (&data[req->Name]);
	ip->ii_i.i_blocks = 1;
	ip->ii_changed = TRUE;

	ldata_ptr = getblk (ip->ii_dev, data_pkt.bnr, 1, NOSAVE);
	if (!ldata_ptr) {
Error (FSErr [GetLDBfailed], vol->vol_name);
		Signal ( &ip->ii_sem );
		ErrorMsg (m, EC_Error+EG_Broken+EO_Medium);
		iput (ip);
		DecPC (97); return;
	}
	clr_buf (ldata_ptr);
	strcpy (ldata_ptr->b_un.b_link->name, &data[req->Name]);
	ldata_ptr->b_un.b_link->cap = req->Cap;
	if (!bwrite (ldata_ptr->b_tbp)) {
Report (FSErr [WriteLDBfailed], vol->vol_name);
		Signal ( &ip->ii_sem );
		PutMsg(&vol->unload_mcb);
		ErrorMsg (m, EC_Error+EG_Broken+EO_Medium);
		iput (ip);
		DecPC (97); return;
	}

	Signal ( &ip->ii_sem );
	iput (ip);

	ErrorMsg (m, Err_Null);
	IncPC (97);
}

/* ==================================================================== */

void 
do_protect(ServInfo *servinfo, VD *vol)

/*
*  Sets the access matrix of the named object.
*/

{
	MCB 		*m = servinfo->m;
	IOCMsg2 	*req = (IOCMsg2 *)(m->Control);
	Matrix 		newmatrix = req->Arg.Matrix;
	AccMask 	mask;					/* AI */
	struct incore_i *ip,*iip;
#if FLDEBUG
	char 		*data = m->Data;
	char 		*pathname = servinfo->Pathname;

DEBdebug ("	do_protect >%s/%s<",pathname,&data[req->Common.Next]);
#endif

#if FLDEBUG
FLdebug ("return without protecting <%s/s>",pathname,&data[req->Common.Next]);
return;
#endif

	IncPC (98);

	/* get Parent Directory of the object */
	iip = get_target_dir (servinfo,vol);	

	/* if there's no such Parent Dir */

	if (!servinfo->ParentOfRoot && !iip)
	{
		if (vol->sync_allowed)
			ErrorMsg(m,0);
		else
			ErrorMsg(m,EO_Medium);

DEBdebug ("	do_protect :	No such parent directory >%s< !",pathname);

		DecPC (98); return;
	}

/* AI */	mask = req->Common.Access.Access;

	/* get target object */
	ip = get_target_obj (servinfo, iip, vol);

	if (iip)
		if (!iput (iip)) {
			ErrorMsg(m,EO_Medium);
			DecPC (98); return;	
		}

	/* if no such target object */
	if (!ip)
	{
		if (vol->sync_allowed)
			ErrorMsg(m,0);
		else
			ErrorMsg(m,EO_Medium);

DEBdebug ("	do_protect :	No such file >%s< !",pathname);

		DecPC (98); return;
	}
	

#if 1 /*EXABYTE-TEST*/
	/* check alter permissions on the object's Access Matrix */
	unless( CheckMask(req->Common.Access.Access,AccMask_A) 
			&& !vol->writeprotected ) 
	{
		if ( vol->writeprotected )
			ErrorMsg(m,EC_Error+EG_Protected+EO_Medium);
		else
			ErrorMsg(m,EC_Error+EG_Protected+EO_Object);

FLdebug ("No access permission.");

		iput (ip);
		DecPC (98); return;
	}
	
#endif

	/* We are allowed to alter the matrix, ensure that it is resonable, 
	   and that the client retains either Delete or Alter permission. */
	if( (UpdMask(AccMask_Full, newmatrix) & 
		(AccMask_A|AccMask_D)) == 0 )
	{
		ErrorMsg(m,EC_Error+EG_Invalid+EO_Matrix);

FLdebug ("Invalid access matrix.");

		iput (ip);
		DecPC (98); return;
	}
#if 0
	if( ( ( (ip->ii_i.i_mode != Type_Directory)
	      || (strcmp ( strstr(pathname,vol->vol_name), vol->vol_name ) != 0) 
	      )
	    && (ip->ii_count > 1) 
	    )
	  || ( ( (ip->ii_i.i_mode == Type_Directory)
	       && (strcmp ( strstr(pathname,vol->vol_name), vol->vol_name ) == 0) 
	       )
	     && (ip->ii_count > 2)
	     )
	  )   
 	{
		ErrorMsg(m,EC_Error+EG_InUse);

DEBdebug ("	do_protect :	Other user working on file !");

		iput (ip);
		DecPC (98); return;
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
	IncPC (98);
}

/* ==================================================================== */

void 
do_setdate (ServInfo *servinfo, VD *vol)

/*
*  Sets the date stamps on the named object.
*/

{
	MCB 		*m = servinfo->m;
	IOCMsg4 	*req = (IOCMsg4 *)(m->Control);
	DateSet 	dates = req->Dates;
	word 		e = Err_Null;
	struct incore_i *ip,*iip;
#if DEBUG	
	char 		*data = m->Data;
	char 		*pathname = servinfo->Pathname;

DEBdebug ("	do_setdate >%s/%s<",pathname,&data[req->Common.Next]);
#endif

	IncPC (99);

	/* get Parent Dir of the object */
	iip = get_target_dir (servinfo,vol);	

	/* if no such Parent Dir */

	if (!servinfo->ParentOfRoot && !iip)
	{
		if (vol->sync_allowed)
			ErrorMsg(m,0);
		else
			ErrorMsg(m,EO_Medium);

DEBdebug ("	do_setdate :	No such parent directory >%s< !",pathname);

		DecPC (99); return;
	}

	/* get the target object */
	ip = get_target_obj (servinfo, iip, vol);

	if (iip)
		if (!iput (iip)) {
			ErrorMsg (m,EO_Medium);
			DecPC (99); return;	
		}

	/* if no such target object */
	if (!ip)
	{
		if (vol->sync_allowed)
			ErrorMsg(m,0);
		else
			ErrorMsg(m,EO_Medium);

DEBdebug ("	do_setdate :	No such file >%s< !",pathname);

		DecPC (99); return;
	}
	/* only allow the user to set the date on an object 
	   if he could write to it. */
#if 1 /*EXABYTE-TEST*/	   
	unless( CheckMask(req->Common.Access.Access,AccMask_W) 
			&& !vol->writeprotected ) 
	{
		if ( vol->writeprotected )
			ErrorMsg(m,EC_Error+EG_Protected+EO_Medium);
		else
			ErrorMsg(m,EC_Error+EG_Protected+EO_Object);

FLdebug ("No access permission.");

		iput (ip);
		DecPC (99); return;
	}

#endif

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
	DecPC (99);
}

/* ==================================================================== */

void 
do_refine (ServInfo *servinfo, VD *vol)

/*
*  Refines or restricts the access rights contained in the capability
*  in the named object structure by generating a new capability.
*/

{
	MCB 		*m = servinfo->m;
	IOCMsg2 	*req = (IOCMsg2 *)(m->Control);
	Capability 	cap;
	AccMask		newmask = req->Arg.AccMask;
	struct incore_i *ip,*iip;
#if DEBUG	
	char 		*data = m->Data;
	char 		*pathname = servinfo->Pathname;

DEBdebug ("	do_refine >%s/%s<",pathname,&data[req->Common.Next]);
#endif

	/* get Parent Dir of object */
	iip = get_target_dir (servinfo,vol);	

	/* if no such Parent Dir */

	if (!servinfo->ParentOfRoot && !iip)
	{
		if (vol->sync_allowed)
			ErrorMsg(m,0);
		else
			ErrorMsg(m,EO_Medium);

DEBdebug ("	do_refine :	No such parent directory >%s< !",pathname);

		return;
	}

	/* get the target object */
	ip = get_target_obj (servinfo, iip, vol);

	if (iip)
		if (!iput (iip)) {
			ErrorMsg(m,EO_Medium);
			return;	
		}

	/* if no such target object */
	if (!ip)
	{
		if (vol->sync_allowed)
			ErrorMsg(m,0);
		else
			ErrorMsg(m,EO_Medium);

DEBdebug ("	do_refine :	No such file >%s< !" ,pathname);

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

void 
do_closeobj (ServInfo *servinfo, VD *vol)

/*
*  This function will be available, after the Name Server is modified,
*  so that Entries in the Name Table can also be deleted.
*  Thus providing the possibilty to terminate for the FileServer.
*/

{
Report (FSErr [FutureVersion], vol->vol_name);
	NullFn(servinfo);
}

/* ==================================================================== */

/* end of fserver.c */ 
