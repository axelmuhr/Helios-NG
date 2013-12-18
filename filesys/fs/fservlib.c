static char rcsid[] = "$Header: /giga/HeliosRoot/Helios/filesys/fs/RCS/fservlib.c,v 1.3 1991/03/21 15:16:18 nick Exp $";

/* $Log: fservlib.c,v $
 * Revision 1.3  1991/03/21  15:16:18  nick
 * New source from Parsytec
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


/*************************************************************************
**                                                                      **
**                  H E L I O S   F I L E S E R V E R                   **
**                  ---------------------------------                   **
**                                                                      **
**                  Copyright (C) 1988, Parsytec GmbH                   **
**                         All Rights Reserved.                         **
**                                                                      **
** fservlib.c								**
**                                                                      **
**	Routines of the FileServer task.		 		**
**                                                                      **
**************************************************************************
** HISTORY   :                                                          **
**------------       							**
** Author    :  10/10/88 : A.Ishan 					**
** Modified  :  19/01/89 : H.J.Ermen  - Marshallling of link-type data  **
**				        in marshal_info()               **
**		20/03/89 : H.J.Ermen  - Investigations to find out, if	**
**					a private message is received. 	**
**					Private server requests implem.:**
**					    FO_Terminate		**
**		22/03/89 :		    FO_Synch			**
**					    FO_Asynch			**
**					    FO_ForceSync		**
**		12/06/89 : NHG		Protection mechanism enabled	**
*************************************************************************/

#include "nfs.h"

#define DEBUG	FALSE
#define NAMES	0
#define MASK	0
#define DSERV	FALSE

#define fworkerStackSize 	10000

word syncop;			/* Flag which determines the operation-	*/
				/* mode (full synchronous or partly syn-*/
			        /* chronous) 				*/
word tidyup;			/* Flag which determines the operation-	*/

Port term_port;			/* Port to signal the termfs-task the	*/
				/* succesful termination of the file-	*/
				/* server.				*/
				
Semaphore sync_sem;

jmp_buf	term_jmp;

/* ==================================================================== */

static void fworker(MsgBuf *m,DispatchInfo *info);
static void Crypt(bool encrypt, Key key, byte *data, word size);

/* ==================================================================== */

void 
fdispatch (DispatchInfo *info)

/*
*  Receives the request messages to FileServer at the Server Port,
*  creating 'fworker' processes to perform each request.
*/

{
	MsgBuf	*m;
	MCB temp_mcb;
	
#if 0
	syncop = FALSE;			/* Initially there are bdwrite- */
					/* operations			*/
#endif
	tidyup = FALSE;			/* Initially there are bdwrite- */

	if ( setjmp(term_jmp) != 0)
	{
#if DEBUG
IOdebug ("fdispatch : Got a request to terminate the file server");
#endif
		/* Close the device being in use.	 */	
		close_dev();
		Free (m);

		/* With returning from this loop, we fi- */
		/* nish completely with the server !	 */
		return;
	}
	forever
	{
		/* create a buffer for messages */
		m = Malloc(sizeof(MsgBuf));
		if( m == Null(MsgBuf) ) 
		{ 
			Delay(OneSec*5); 
			continue; 
		}
		/* initialise message header */
		m->mcb.MsgHdr.Dest	= info->reqport;
		m->mcb.Timeout		= OneSec*30;
		m->mcb.Control		= m->control;
		m->mcb.Data		= m->data;
	lab1:
		/* get a request message */
		while( GetMsg(&(m->mcb)) == EK_Timeout );
#if DEBUG
printf("function	>0x%x<\n",m->mcb.MsgHdr.FnRc);
#endif

 		if( MyTask->Flags & Task_Flags_servlib )
 			IOdebug("HFS: %F %s %s",m->mcb.MsgHdr.FnRc,
 				(m->control[0]==-1)?NULL:&m->data[m->control[0]],
 				(m->control[1]==-1)?NULL:&m->data[m->control[1]]);


/*-----  FO_Synch and FO_Asynch determine the write operation-mode  ----*/

		if ( m->mcb.MsgHdr.FnRc == 
		     FC_GSP+SS_HardDisk+FG_Private+FO_Synch )
		{
IOdebug ("fdispatch : Got a message to turn into synchronous mode");

			syncop = TRUE;			
			Free ( m );
			continue;
		}
		
		if ( m->mcb.MsgHdr.FnRc == 
		     FC_GSP+SS_HardDisk+FG_Private+FO_Asynch )
		{

IOdebug ("fdispatch : Got a message to turn into asynchronous mode");

			syncop = FALSE;
			Free ( m );
			continue;
		}

/*------------- FO_ForceSync generate a sync_fs immediately ------------*/

		if ( m->mcb.MsgHdr.FnRc == 
		     FC_GSP+SS_HardDisk+FG_Private+FO_ForceSync )
		{
#if DEBUG
IOdebug ("fdispatch : Got a message to force a sync-operation immediately");
#endif
			sync_fs ();
				/* Prepare termination message		 */
			InitMCB ( &temp_mcb, MsgHdr_Flags_preserve,
				   m->mcb.MsgHdr.Reply, NullPort, 0 );	
				/* Send the termination message to fstat */
			PutMsg ( &temp_mcb );
			Free ( m );
			continue;
		}

/*-------------  FO_Terminate forces file-server shutdown  --------------*/

		if ( m->mcb.MsgHdr.FnRc ==
		     FC_GSP+SS_HardDisk+FG_Private+FO_Terminate )
		{
#if DEBUG
IOdebug ("fdispatch : Got a request to terminate the file server");
#endif
				/* Save the reply port for further use	 */
			term_port = m->mcb.MsgHdr.Reply;
			
				/* Close the device being in use.	 */	
			term_fs ();
			Free (m);
				/* With returning from this loop, we fi- */
				/* nish completely with the server !	 */
			return;
		}

/*-------------  Now we have handled all private requests  --------------*/

		/* fork a worker process for each request */
		unless( Fork(fworkerStackSize, fworker, 8, m, info) )
		{
			/* send an exception if process couldn't be forked */
			ErrorMsg(&m->mcb,EC_Error+info->subsys+EG_NoMemory);
			/* get a new message */
			goto lab1;
		}

	}
}

/* ==================================================================== */

static void
fworker (MsgBuf *m,DispatchInfo *info)

/*
*  Dynamically created process to deal with FileServer requests.
*/

{
	ServInfo servinfo;
	word fncode = m->mcb.MsgHdr.FnRc;
	IOCCommon *req = (IOCCommon *) (m->mcb.Control);
	
#if DEBUG
printf("	context [%d] >%s<\n",req->Context,
(req->Context>=0)?m->mcb.Data+req->Context:"-1");
printf("	name    [%d] >%s<\n",req->Name,
(req->Name>=0)?m->mcb.Data+req->Name:"-1");
printf("	next    [%d] >%s<\n",req->Next,
(req->Next>=0)?m->mcb.Data+req->Next:"-1");
#endif
	if( setjmp(servinfo.Escape) != 0 )
	{
		Free(m);
		return;
	}

	servinfo.m = &m->mcb;
	MachineName(servinfo.Context);
	pathcat(servinfo.Context,fs_name);
	servinfo.ParentOfRoot = FALSE;
	servinfo.FnCode = fncode;
	m->mcb.MsgHdr.FnRc = info->subsys;	

	if( !get_context (&servinfo) ) 
		ErrorMsg (&m->mcb,0);
	else	
	{
		word fn = fncode & FG_Mask;
		VoidFnPtr f;

#if NAMES
printf("fwCTXT : >%s<\n",servinfo.Context);
printf("fwPATH : >%s<\n",servinfo.Pathname);
printf("fwTRGT : >%s<\n",servinfo.Target);
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
			f = info->fntab[(fn-FG_Open) >> FG_Shift];
			(*f)(&servinfo);
		}
	}
	Free( m );
}

/* ==================================================================== */

word
get_context (ServInfo *servinfo)

{
	MCB *m = servinfo->m;
	IOCCommon *req = (IOCCommon *)(m->Control);
	int context = req->Context;
	int name = req->Name;
	int next = req->Next;
	byte *data = m->Data;
	char *pathname = servinfo->Pathname;

	char smatrix[40];
	char smask[40];
	char *s;
	
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
		struct incore_i *ip = iget(NULL,0,1,0);
#if MASK
s = DecodeMask(smask,req->Access.Access,DirChars);
*s = 0;
DecodeMatrix(smatrix,ip->ii_i.i_matrix,ip->ii_i.i_mode);
printf("get_ctx: UpdMask(%s, %s, %s);\n",
	smask, smatrix, ip->ii_name);
#endif
		req->Access.Access = UpdMask(req->Access.Access,ip->ii_i.i_matrix);
		iput(ip);
		return TRUE;
	}

	/* Otherwise simply follow the remains of the context name	*/
	/* through the directories to the end.				*/

	{
		int len;
		
		struct incore_i *ip;
		
		/* build the absolute pathname of the required object in the FileSystem */
		if ( name==-1 || (next<name && next!=-1 && data[next]!='\0') )
			pathcat(servinfo->Context,data+next);

		ip = namei (servinfo->Context);
		if( !ip ) 
		{
			m->MsgHdr.FnRc |= EC_Error+EG_Name;
			*(servinfo->Context) = '\0';
			return FALSE;
		}

		if( !GetAccess(&req->Access,(Key)ip->ii_i.i_cryptkey) )
		{
			m->MsgHdr.FnRc |= EC_Error+EG_Invalid+EO_Capability;
			*(servinfo->Context) = '\0';
/* AI 900927 */		iput (ip);
			return FALSE;
		}
		iput (ip);
		
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
get_parent (ServInfo *servinfo, string pathname)


/*
*  Returns a pointer to the Incore Inode of the Parent Directory
*  of the object specified by the absolute FileSystem-pathname.
*/

{
	size_t len;
	char subdir[IOCDataMax];
	struct incore_i *iip;

	/* calc the length of the Parent Directories pathname */
	len = strrchr (pathname,'/') - pathname;
	/* build the pathname for the Parent Dir */
	strncpy (subdir, pathname, len);
	subdir[len] = '\0';
	/* the Parent Dir of the Root Dir is Root Dir itself */
	if ( ! strstr(subdir,fs_name) ) 
	{
/*
		iip = iget ((struct buf *)NULL, 0, 1, 0);
		return (iip);
*/
		servinfo->ParentOfRoot = TRUE;
		return ((struct incore_i *)NULL);
	}
	/* get the pointer to the inode of the Parent Dir */
	iip = namei (subdir);
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

	if (len == 0) return NULL;
	
	/* search in the Parent Dir */
	Wait(&iip->ii_sem);
	idir = searchi (iip, name, len);
	if (!idir.bp)
	{ 
	 	Signal (&iip->ii_sem);
		return ((struct incore_i *)NULL);
	}
	/* make a incore copy of the inode of the object */
	ip = iget (idir.bp, idir.ofs, 
		   iip->ii_dirbnr, iip->ii_dirofs);
#if DSERV
printf("	get_child :		brelse (0x%p, TAIL);\n",idir.bp->b_tbp);	
#endif
	/* release Parent Dir */
	brelse (idir.bp->b_tbp, TAIL) ;
	Signal (&iip->ii_sem);
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

struct incore_i *get_target_dir(ServInfo *servinfo)
{
	MCB *m = servinfo->m;
	struct incore_i *ip;
	byte *data = m->Data;
	IOCCommon *req = (IOCCommon *)(m->Control);
	int next = req->Next;
	char *pathname = servinfo->Pathname;

/*debug("GetTargetDir %x %x %s %s",m,servinfo->Context,pathname,&data[next]);*/

#if NAMES
printf("gtdCTXT : >%s<\n",servinfo->Context);
printf("gtdPATH : >%s<\n",servinfo->Pathname);
printf("gtdTRGT : >%s<\n",servinfo->Target);
#endif
	/* if context is object, step back to parent */
	if( next == -1 || data[next] == '\0' )
	{
		ip = get_parent(servinfo, servinfo->Context); 
		if( !ip ) {
			m->MsgHdr.FnRc |= EC_Error+EG_Name;
			strcpy (servinfo->Target, "\0"); 
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
		extern /*inode.c*/ void iput (struct incore_i *);

		char smatrix[40];
		char smask[40];
		char *s;
	
		iip = namei(servinfo->Context); 
		if( !iip ) {
			m->MsgHdr.FnRc |= EC_Error+EG_Name;
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

				ip = get_parent(servinfo, pathname);
				if( !ip ) {
					m->MsgHdr.FnRc |= EC_Error+EG_Name;
					strcpy (servinfo->Target, "\0"); 
/* AI 900921 */				iput (iip);
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
				iput (iip);
/* AI 900921 */			mask = UpdMask(mask,ip->ii_i.i_matrix);
/* AI 900921
				if( ip->ii_i.i_mode == Type_Link ) goto dolink;
*/
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
			}

			if (iip != ip) 
				iput (iip);
			
			if( !ip ) {
				m->MsgHdr.FnRc |= EC_Error+EG_Name;
				strcpy (servinfo->Target, "\0"); 
				return ip;
			}

#if MASK
s = DecodeMask(smask,mask,DirChars);
*s = 0;
DecodeMatrix(smatrix,ip->ii_i.i_matrix,ip->ii_i.i_mode);
printf("get_dir: UpdMask(%s, %s, %s);\n",
	smask, smatrix, ip->ii_name);
#endif
			mask = UpdMask(mask,ip->ii_i.i_matrix);
			
			if( ip->ii_i.i_mode == Type_Link )
			{
/* AI 900921
			dolink:
*/
				req->Access.Access = mask;
				req->Next = next+len;
/* AI 900921
				iput (iip);
*/
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

struct incore_i *get_target_obj(ServInfo *servinfo, struct incore_i *iip)
{
	struct incore_i *ip;
	MCB *m = servinfo->m;
	IOCCommon *req = (IOCCommon *)(m->Control);
	byte *data = m->Data;
	word next = req->Next;
	word len;
	char name[NameMax];
	char *pathname = servinfo->Pathname;

	char smatrix[40];
	char smask[40];
	char *s;
	
/*debug("GetTargetObj %x %s",next,pathname);*/
#if NAMES
IOdebug("gtoCTXT : >%s<",servinfo->Context);
IOdebug("gtoPATH : >%s<",servinfo->Pathname);
IOdebug("gtoTRGT : >%s<",servinfo->Target);
#endif
	if (!iip)
	{
		ip = iget ((struct buf *)NULL, 0, 1, 0);
		return (ip);
	}

	if( next == -1 || data[next] == '\0' )
	{
		extern /*"inode.c"*/ struct incore_i *namei (string);
		
		ip = namei(servinfo->Context);
		if (!ip)
		{
			m->MsgHdr.FnRc |= EC_Error+EG_Name;
			strcpy (servinfo->Target, "\0"); 
		}
		else
		{
			strcpy (servinfo->Target, ip->ii_name); 
		}
		return ip;
	}

	if( (len = splitname(name, c_dirchar, &data[next] )) == 0 )
	{
		m->MsgHdr.FnRc |= EC_Error+EG_Name;
		ip = (struct incore_i *)NULL;
		strcpy (servinfo->Target, "\0"); 
		return ip;
	}

	/* special case . and .. */
	if( name[0] == '.' && name[1] == '\0' ) 
	{
		req->Next = next+len;
		Wait (&ihash_sem);
		iip->ii_count++;
		Signal (&ihash_sem);
		return iip;
	}
	elif( name[0] == '.' && name[1] == '.' 
		&& name[2] == '\0') 
	{
		int l = strlen(pathname);

		ip = get_parent(servinfo, pathname);
		if( !ip ) {
			m->MsgHdr.FnRc |= EC_Error+EG_Name;
			strcpy (servinfo->Target, "\0"); 
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
	}

	if( !ip ) 
	{
		m->MsgHdr.FnRc |= EC_Error+EG_Unknown+EO_Object;
		strcpy (servinfo->Target, "\0"); 
		return ip;
	}

#if MASK
s = DecodeMask(smask,req->Access.Access,DirChars);
*s = 0;
DecodeMatrix(smatrix,ip->ii_i.i_matrix,ip->ii_i.i_mode);
printf("get_obj: UpdMask(%s, %s, %s);\n",
	smask, smatrix, ip->ii_name);
#endif
	req->Access.Access = UpdMask(req->Access.Access,ip->ii_i.i_matrix);

	if( ip->ii_i.i_mode == Type_Link )
	{
		switch( servinfo->FnCode & FG_Mask )
		{
		default:
			req->Next = next+len;
			iput (iip);
			handle_link (ip, servinfo);
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
	MarshalString(&m->mcb, ldata_ptr->b_un.b_link->name);

	if( data[next]!= '0' ) 
		MarshalString(&m->mcb,&data[next]);
	else 
		MarshalWord(&m->mcb, -1);

	MarshalWord(&m->mcb, 1);
	MarshalCap(&m->mcb, &ldata_ptr->b_un.b_link->cap);

	brelse ( ldata_ptr->b_tbp, TAIL );
	iput ( ip );

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
	
	longjmp(servinfo->Escape,1);
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

	Wait (&iip->ii_sem);
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
			/* report exception if all direct blocks
			   of the Parent Dir already allocated and
			   there is no free DirEntry */
			if (iip->ii_i.i_db[MAXDIR-1]) 
			{

IOdebug("	make_obj :	ERROR : The parent-directory is full !!!");
				idir.bp = (struct buf *)NULL;
			} 
			else 
			{
				for (ilgbnr=0; iip->ii_i.i_db[ilgbnr]; ilgbnr++)
					;
#if DEBUG
printf("	make_obj : 	ipkt = alloc (0x%p, %d, 1);\n",iip,ilgbnr);
#endif
				/* alloc a new block for Parent Dir */
				ipkt = alloc (iip, ilgbnr, 1);
				if (!ipkt.bcnt)
					idir.bp = (struct buf *)NULL;
				else 
				{
#if DEBUG
printf("	make_obj : 	idir.bp = getblk (%d, %d, 1, NOSAVE);\n",
iip->ii_dev,ipkt.bnr);
#endif
					/* get a free buffer for the new block */
					idir.bp = getblk (iip->ii_dev,ipkt.bnr,
							  1,NOSAVE);
					/* clear its contents */
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
#if DEBUG
printf("	make_obj :	bwrite (0x%p);\n", idir.bp->b_tbp);	
#endif
	/* write out Parent Dir */
	bwrite(idir.bp->b_tbp);
	/* mark its Incore Inode as modified and release */
	iip->ii_changed = TRUE;
	Signal (&iip->ii_sem);
	if (mode!=Type_File || newname==(string)NULL)	
		iput (iip);

	return (ip);

finish:	
	Signal (&iip->ii_sem);
	if (mode!=Type_File || newname==(string)NULL)	
		iput (iip);
	return ((struct incore_i *)NULL);
}

/* ==================================================================== */

void 
dir_server (struct incore_i *ip, struct incore_i *iip, MCB *m, Port reqport)

/*
*  Performs Stream Operations on Directory objects,
*  allowing merely Read and GetSize requests.
*  Forms a message including the DirEntries.
*/
 
{
	forever 
	{
		word e;
		m->MsgHdr.Dest = reqport;
		m->Timeout = StreamTimeout;

		/* get Stream request and check its validity */
		e = GetMsg(m);
		if( e == EK_Timeout ) break;
		if( e < Err_Null ) continue;
#if DEBUG
printf("stream		>0x%x<\n",m->MsgHdr.FnRc);
#endif
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
			
			word req_size, bcnt, i, j, k;
			daddr_t bnr;
			struct buf *bp;
			struct packet *tbp;
			
			Wait( &ip->ii_sem );
			/* calc size of directory */
			dirsize = (ip->ii_i.i_spare+2) * sizeof(DirEntry);
			/* check the given directory position */
			if( pos == dirsize )
			{
				Signal( &ip->ii_sem);
				InitMCB(m,0,reply,NullPort,ReadRc_EOF|seq);
				PutMsg(m);
				break;
			}

			if( pos % sizeof(DirEntry) != 0 ||
			    pos < 0 || pos >= dirsize )
			{
				Signal( &ip->ii_sem);
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
				bnr = bmap (ip, i*BSIZE);

				/* read the packet of Dir Blocks */
				bp = bread(ip->ii_dev,bnr,1,SAVEA);
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
					marshal_dentry(m,bp->b_un.b_dir+k);
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
finish:
			Signal( &ip->ii_sem );
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
			ErrorMsg(m,Err_Null);
			return;

		case FG_GetSize:
		{
			/* initialise reply message */
			InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
			/* add info about the size of directory */
			MarshalWord(m,(ip->ii_i.i_spare+2) * sizeof(DirEntry));
			/* send the message */
#if DSERV
printf("	dir_server :	GetSize returning %d\n",m->Control[0]);
#endif
			PutMsg(m);
			break;
		}

		/* other Stream Operations not permitted on Directories */
		case FG_Seek:
		case FG_Write:
		case FG_SetSize:
			ErrorMsg(m,EC_Error+EG_WrongFn+EO_Directory);
		default:
			ErrorMsg(m,EC_Error+EG_FnCode+EO_Directory);
			break;
		}
	}
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

#if DEBUG
printf("	pathname	>%s<\n",pathname);
#endif

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

#if DEBUG
printf("	new_cap		>0x%x<\n", mask);
#endif
	cap->Access = mask;

	for( i=0; i<7 ; cap->Valid[i++] = check );
	cap->Valid[0] = mask;
	cap->Valid[3] = mask;

	Crypt(1, key, (byte *)&cap->Valid, 7);

}

/* ==================================================================== */

void 
marshal_ientry( MCB *m, struct incore_i *ip, char *name)

/*
*  Form a reply message according to informations of the Incore Inode.
*/

{
	word flags = 0;

	if (ip)
		MarshalData(m,4,(byte *)&ip->ii_i.i_mode);
	else
		MarshalData(m,4,(byte *)&flags);
	MarshalData(m,4,(byte *)&flags);
	if (ip)
		MarshalData(m,4,(byte *)&ip->ii_i.i_matrix);
	else
		MarshalData(m,4,(byte *)&flags);
	MarshalData(m,32,name);
}

/* ==================================================================== */

void 
marshal_dentry( MCB *m, struct dir_elem *dp)

/*
*  Form a reply message according to informations of the DirEntry.
*/

{
	word flags = 0;

	MarshalData(m,4,(byte *)&dp->de_inode.i_mode);
	MarshalData(m,4,(byte *)&flags);
	MarshalData(m,4,(byte *)&dp->de_inode.i_matrix);
	MarshalData(m,32,dp->de_name);
}

/* ==================================================================== */


/*
 *  Initialise and form a reply message for ObjInfo,
 *  with expanded informations of the Incore Inode.
 *
 *   19/01/89 : - Addition of marshalling link-entries
 *   15/02/89 : - Managing capabilities in a proper manner
 *
 */

void 
marshal_info (MCB *m, struct incore_i *ip)
{
 word size;
 struct buf *ldata_ptr;

 InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
 marshal_ientry(m,ip,ip->ii_name);
	
 if ( ip->ii_i.i_mode == LINK )
 {
  	ldata_ptr = bread (ip->ii_dev, ip->ii_i.i_db[0], 1, SAVEA);
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
	err |= mcb->MsgHdr.FnRc;
	
	if( mcb->MsgHdr.Reply == NullPort ) return;

/*debug("ErrorMsg %x %x %x",mcb,mcb->MsgHdr.Reply,err);*/

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
#if DEBUG
printf("UpdMask(0x%x, 0x%x)",mask,matrix);
#endif
	if( mask & AccMask_V ) res |= matrix & 0xff;
	if( mask & AccMask_X ) res |= (matrix>>8) & 0xff;
	if( mask & AccMask_Y ) res |= (matrix>>16) & 0xff;
	if( mask & AccMask_Z ) res |= (matrix>>24) & 0xff;
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
/*debug("CheckMask(%x %x)",mask,access);*/
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
/*debug("InitNode %x %s %x %x %x",o,name,type,flags,matrix);*/
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
	IOdebug("File Server stack overflow in %s at %x",p->Name,&p);
}

/* ================================================================= */

/* end of fservlib.c */ 



