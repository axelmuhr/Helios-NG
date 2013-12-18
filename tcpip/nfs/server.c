#include "nfs.h"

#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <nonansi.h>
#include <unistd.h>

Object *RootObj;

char *MyName;
char *HostName;
char *HostRoot;
char *UserName;
char McName[100];

#define STACKSIZE	(5*1024)

nfshandle nfscache[MAXHANDLES];
Semaphore cachelock;			/* lock on handle cache	*/
List cache;				/* cache list		*/

Semaphore nfslock;			/* lock for serializing NFS routines */

CLIENT	*nfsclnt;			/* RPC client handle	*/

nfshandle *roothandle;			/* handle of root of fs	*/

extern nfs_fh nfsroot;			/* root handle from mount */

AUTH *defauth;				/* default authentication	*/

MMsgBuf *NewMMsgBuf()
{
	MMsgBuf *m;

	while( (m = New(MMsgBuf)) == NULL ) Delay(OneSec);

	m->mcb.Control = m->control;
	m->mcb.Data = m->data;
	
	return m;
}

extern int nfs_errno(nfsstat stat)
{
	switch(stat)
	{
	case NFS_OK:		return 0;
	case NFSERR_PERM:	return EPERM;
	case NFSERR_NOENT:	return ENOENT;
	case NFSERR_IO:		return EIO;
	case NFSERR_NXIO:	return ENXIO;
	case NFSERR_ACCES:	return EACCES;
	case NFSERR_EXIST:	return EEXIST;
	case NFSERR_NODEV:	return ENODEV;
	case NFSERR_NOTDIR:	return ENOTDIR;
	case NFSERR_ISDIR:	return EISDIR;
	case NFSERR_FBIG:	return EFBIG;
	case NFSERR_NOSPC:	return ENOSPC;
	case NFSERR_ROFS:	return EROFS;
	case NFSERR_NAMETOOLONG:return ENAMETOOLONG;
	case NFSERR_NOTEMPTY:	return ENOTEMPTY;
	case NFSERR_DQUOT:	return ENOSPC;
	case NFSERR_STALE:	return EIO;
	case NFSERR_WFLUSH:	return EIO;
	default:
	  return 0;
	}
}

int nauth = 0;

AUTH *makeauth(Capability *cap)
{
	word ugid = ((word *)cap)[1];
	if( ugid == -1 )
	{
		((word *)cap)[1] = (word) getuid() | ((word) getgid() << 16);
		return defauth;
	}
	else
	{
		nauth++;
		return authunix_create( McName, (int) ugid & 0xFFFF, ((int) ugid >> 16) & 0xFFFF, 0, 0 );
	}
}

int maxheap = 50000;

void freeauth(AUTH *auth)
{
	if( auth == defauth ) return;
	nauth--;
#if 0
	if( nauth < 0 || nauth > 10 ) IOdebug("nauth %d",nauth);
{
	int new = Malloc(-3);
	if( new > maxheap )
	{
		IOdebug("new heap max %d",new);
		maxheap = new;
	}
}
#endif
	auth_destroy(auth);
}

static int mask2mode(word type, AccMask mask)
{
	int mode = 0;
	
	switch (type)
	{
	case NFSMODE_DIR:
	case NFSMODE_LNK:
		if (mask & AccMask_W) mode |= 2;
		if (mask & AccMask_R) mode |= 5;	/* 1 | 4 */
		break;
	case NFSMODE_REG:
	default:
		if (mask & AccMask_E) mode |= 1;
		if (mask & AccMask_W) mode |= 2;
		if (mask & AccMask_R) mode |= 4;
		break;
	}
	return(mode);
}

bool checkaccess(nfshandle *fh, Capability *cap , AccMask access)
{
	int wantmode = mask2mode((word) fh->Attr.mode & NFSMODE_FMT, access);
	int mode = 0;
	word ugid = ((word *)cap)[1];

	if( fh->Attr.uid ==       (ugid & 0xFFFF)) mode |= (fh->Attr.mode >> 6);
	if( fh->Attr.gid == ((ugid>>16) & 0xFFFF)) mode |= (fh->Attr.mode >> 3);

	mode |= fh->Attr.mode;

/*
IOdebug("checkaccess fh %d %d %x u %d %d %x mode %x",
		fh->Attr.uid,fh->Attr.gid,fh->Attr.mode,
		ugid&0xFFFF,(ugid>>16)&0xFFFF,wantmode,mode);
*/
	/* l.s. 3 bits now encode access rights, check that all the mode */
	/* bits we want are set.					 */

	return (mode & wantmode) == wantmode;
}


void initcache()
{
	int i;
	InitSemaphore(&cachelock,1);
	InitList(&cache);
	for( i = 0; i < MAXHANDLES; i++ )
	{
		AddTail(&cache,&nfscache[i].Node);
	}
}

static word putentry(nfshandle *h)
{
	if( h->Path != NULL )
	{
		IOdebug("%d %x %s",h->InUse,h->Hash,h->Path);
	}
	return 0;
}

static uword hash(char *s)
{
	uword h = 0;
	while( *s ) h += *s++;
	return h;
}

struct gharg
{
	char *name;
	uword hash;
};

static word gethandle(nfshandle *h, struct gharg *arg)
{
	if( (arg->hash == h->Hash) && (strcmp(arg->name,h->Path) == 0) )
	{
		Remove(&h->Node);
		AddHead(&cache,&h->Node);
		return 1;
	}
	return 0;
}

nfshandle *FindHandle(char *name)
{
	struct gharg arg;
	nfshandle *h;

	arg.name = name;
	arg.hash = hash(name);
	
	Wait(&cachelock);
	h = (nfshandle *)SearchList(&cache,gethandle,&arg);
	if( h != NULL && h != roothandle ) h->InUse++;
	Signal(&cachelock);
	return h;
}

nfshandle *NewHandle(char *name, fattr *attr, nfs_fh *handle)
{
	nfshandle *h, *first = NULL;
	int len = strlen(name);

	Wait(&cachelock);
	
	forever
	{
		h = (nfshandle *)RemTail(&cache);
	
		if( h->InUse == 0 ) break;

		AddHead(&cache,&h->Node);

		if( first == NULL ) first = h;
		elif( h == first )
		{
			IOdebug("No more handles!!");
			WalkList(&cache,putentry);
			Signal(&cachelock);
			Delay(OneSec);
			first = NULL;
			Wait(&cachelock);
		}
	}
	
	Free(h->Path);
	
	h->Path = (char *) Malloc( (word)len + 1);
	
	memcpy(h->Path,name,len+1);
	memcpy(&h->Attr,attr,sizeof(fattr));
	memcpy(&h->File,handle,sizeof(nfs_fh));

	h->Hash = hash(h->Path);
	h->InUse = 1;
	
	AddHead(&cache,&h->Node);
	
	Signal(&cachelock);
	
	return h;
}

void FreeHandle(nfshandle *h)
{
	if( h == NULL || h == roothandle ) return;
	Wait(&cachelock);
	h->InUse--;
	Signal(&cachelock);
}

bool ValidateHandle(nfshandle *h, AUTH *auth)
{
	attrstat *res;

	if( h == NULL ) return false;

	Wait(&nfslock);
	nfsclnt->cl_auth = auth;
	res = nfsproc_getattr_2(&h->File, nfsclnt);
	Signal(&nfslock);

	if( res == NULL || res->status != NFS_OK )
	{
		if( DEBUG ) IOdebug("NFS getattr %s: res %x st %d",h->Path,res,res->status);
		h->Hash++;	/* will never match now */		
		return false;
	}
	else
	{
		/* If the attributes have changed in any way, treat this as */
		/* an invalid handle, this will cause a new lookup. This is */
		/* because there is otherwise no way of detecting a file    */
		/* that has been renamed.				    */
		if( memcmp( &h->Attr, &res->attrstat_u.attributes, sizeof(h->Attr) ) != 0)
		{
			h->Hash++;
			return false;
		}
	}
	return true;
}


char *makepath(char *pathname, char *context, char *path)
{
	char *p;
	int len;
	
	strcpy(pathname,"/");
	while( *context == c_dirchar ) context++;
	strcat(pathname,context);
	p = pathname+strlen(pathname);
		
	if( p == pathname+1 ) p = pathname;
	
	for(;;)
	{
		while( *path == c_dirchar ) path++;
		
		len = splitname(p+1, c_dirchar, path );

		if( len == 0 ) break;

		if( p[1] == '.' && p[2] == 0 ) 
		{
			path += len;
			continue;
		}
		
		if( p[1] == '.' && p[2] == '.' && p[3] == 0 )
		{
			while( p != pathname && *p != c_dirchar ) p--;
			*p = 0;
		}
		else *p = c_dirchar, p += len;
		
		path += len;
		
	}
	if( *pathname == '\0' ) strcpy(pathname,"/");
	return pathname;
}

nfshandle *followpath(char *context, char *path, AUTH *auth)
{
	char pathname[512];
	char name[32];
	diropres *res;
	diropargs args;
	fattr attr;
	int len;
	char oldc, *p;
	nfshandle *ch;

	makepath(pathname,context,path);

	if( DEBUG ) IOdebug("NFS followpath %s",pathname);
		
	/* first look for an existing handle */
	p = pathname + strlen(pathname);
	oldc = *p;
	
	for(;;)
	{
		ch = FindHandle(pathname);
		if( !ValidateHandle(ch,auth) ) { FreeHandle(ch); ch = NULL; }
		if( ch == NULL )
		{
			*p-- = oldc;
			while( *p != c_dirchar ) p--;
			if( p == pathname )
			{
				ch = roothandle;
				p = pathname+1;
				break;
			}
			oldc = *p; *p = 0;
		}
		else
		{
			*p = oldc;
			if( oldc != 0 ) p++;
			break;
		}
	}
	
	if( *p == 0 ) return ch;
	
	args.dir = ch->File;
	args.name = name;
		
	for(;;)
	{
		len = splitname(name, '/', p );

		if( len == 0 ) break;

		Wait(&nfslock);
		nfsclnt->cl_auth = auth;
		while((res = nfsproc_lookup_2(&args, nfsclnt)) == NULL)
			clnt_perror(nfsclnt,"lookup");

		if( res->status != NFS_OK )
		{
			if( DEBUG ) IOdebug("NFS lookup %s stat %d",name,res->status);
			ch->Error = res->status;
			Signal(&nfslock);
			FreeHandle(ch);
			return NULL;	
		}
		
		args.dir = res->diropres_u.diropres.file;
		attr = res->diropres_u.diropres.attributes;
			
		Signal(&nfslock);	
		
		p += len;
	}

	FreeHandle(ch);
	return NewHandle(pathname,&attr,&args.dir);
	
}

nfshandle *gettarget(MCB *mcb, AUTH *auth)
{
	IOCCommon *req = (IOCCommon *)mcb->Control;
	int context = (int) req->Context;
	int name    = (int) req->Name;
	int next    = (int) req->Next;
	char *cpath = mcb->Data+context;
	char *tpath = mcb->Data+name;
	nfshandle *th;
		
	if( DEBUG ) IOdebug("NFS gettarget [%d,%d,%d] %s+%s",context,name,next,cpath,tpath);
	
	/* if next is at the end of context string, step on to name */
	if( next < name && mcb->Data[next] == 0 ) next = name;
	
	/* if there is no context string, or we have already used 	*/
	/* some of the name, search from the root. Otherwise	  	*/
	/* get hold of the context object.			  	*/
	/* This also implies that the capability is absent or 		*/
	/* useless, so reset its mask to Full so that any capability	*/
	/* we generate for this object reflects its access rights in	*/
	/* the server.							*/
	if( context == -1 || (name > 0 && next >= name) )
	{
		next -= name;
		cpath = "/";
		req->Access.Access = AccMask_Full;
	}
	else
	{
		cpath += next;
		next = 0;
	}
	
	if( name == -1 || tpath[next] == 0 ) tpath = "";
	else tpath += next;
	
	th = followpath(cpath,tpath,auth);

	if( th == NULL ) mcb->MsgHdr.FnRc = EC_Error|SS_HardDisk|EG_Errno|ENOENT;

	return th;	
}

static AccMask mode2mask(word type, unsigned int mode)
{
	AccMask mask = 0;
	switch( type )
	{
	case Type_Directory:
	case Type_Link:
		if( mode & 1 ) mask |= AccMask_W;
		if( mode & 2 ) mask |= AccMask_W;		
		if( mode & 4 ) mask |= AccMask_R;
		break;
		
	default:
	case Type_File:
		if( mode & 1 ) mask |= AccMask_E;
		if( mode & 2 ) mask |= AccMask_W;
		if( mode & 4 ) mask |= AccMask_R;
		break;
	}
	return mask;
}

static Matrix mode2matrix(word type, uword mode)
{
	Matrix matrix = 0;
	matrix |= (word) mode2mask( type,  (unsigned) mode       & 07 ) << 24;
	matrix |= (word) mode2mask( type, ((unsigned) mode >> 3) & 07 ) << 16;
	matrix |= (word) mode2mask( type, ((unsigned) mode >> 3) & 07 ) <<  8;
	matrix |= (word) mode2mask( type, ((unsigned) mode >> 6) & 07 ) <<  0;

	matrix |= AccMask_A;
	matrix |= AccMask_D*0x00010101;

	switch( type )
	{
	case Type_Directory:
	case Type_Link:
		matrix |= AccMask_V;
		matrix |= (AccMask_X|AccMask_Y)*0x00010100;
		matrix |= AccMask_Z*0x01000000;
		break;
	}

	return matrix;
}



int matrix2mode(word type, Matrix matrix)
{
	int mode = 0;

	mode |= mask2mode( type, ((unsigned) matrix >> 24) & 0377 );
/*	mode |= mask2mode( type, ((unsigned) matrix >> 16) & 0377 ) << 3; */
	mode |= mask2mode( type, ((unsigned) matrix >> 8)  & 0377 ) << 3;
	mode |= mask2mode( type, ((unsigned) matrix >> 0)  & 0377 ) << 6;
	
	return (mode);
}


void form_open_reply(MCB *mcb, nfshandle *fh, word flags)
{
	char pathname[512];
	word type;
	Capability cap;
	Matrix matrix;
	AccMask mask = 0;
	int ugid;
	
	cap = ((IOCCommon *)mcb->Control)->Access;
	ugid = ((int *)&cap)[1];
		
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,0);

	/* @@@ get some flags from attr, also set up capability */

	switch( fh->Attr.type )
	{
	case NFDIR:	type = Type_Directory;	break;
	case NFLNK:	type = Type_Link;	break;
	default:
	case NFREG:	type = Type_File;	break;
	}

	matrix = mode2matrix(type, fh->Attr.mode);
	
	if( (ugid&0xFFFF) == fh->Attr.uid ) mask |= (int) matrix & AccMatrix_V;
	if( ((ugid>>16)&0xFFFF) == fh->Attr.gid ) mask |= ((int) matrix & AccMatrix_X ) >> 8;
	
	/* we always get Z (= public) access */
	mask |= ((int) matrix & AccMatrix_Z)>>24;

	cap.Access &= mask;
	
	MarshalWord(mcb,type);
	MarshalWord(mcb,flags);
	MarshalCap(mcb,&cap);
	MachineName(pathname);
	pathcat(pathname,MyName);
	pathcat(pathname,fh->Path+1);
	MarshalString(mcb,pathname);
}


void marshal_info(MCB *mcb, nfshandle *fh, AUTH *auth)
{
	word type = 0;
	word flags = 0;
	word matrix;
	char *name = objname(fh->Path);
		
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,SS_HardDisk);
		
	/* @@@ init matrix and flags from attributes... */
	
	switch( fh->Attr.type )
	{
	case NFDIR:	type = Type_Directory; 	break;
	case NFLNK:	type = Type_Link;	break;
	default:
	case NFREG:	type = Type_File; 	break;
	}

	matrix = mode2matrix(type,fh->Attr.mode);
	
	MarshalData(mcb,4,(byte *)&type);
	MarshalData(mcb,4,(byte *)&flags);
	MarshalData(mcb,4,(byte *)&matrix);
	MarshalData(mcb,32,name);
	
	if( type == Type_Link )
	{
		Capability cap;
		readlinkres *res;
		char *s;

		cap = ((IOCCommon *)mcb->Control)->Access;
		
		Wait(&nfslock);
		nfsclnt->cl_auth = auth;
		while((res = nfsproc_readlink_2(&fh->File, nfsclnt)) == NULL )
			clnt_perror(nfsclnt, "readlink");
		
		s = res->readlinkres_u.data;	
		MarshalData(mcb,sizeof(Capability),(byte *)&cap);
		MarshalData(mcb,(long) strlen(s)+1,s);
		Signal(&nfslock);

	}
	else
	{
		word size = fh->Attr.size;
		word acct = fh->Attr.uid;
		MarshalData(mcb,4,(byte *)&acct);
		MarshalData(mcb,4,(byte *)&size);
		MarshalData(mcb,4,(byte *)&fh->Attr.ctime.seconds);		
		MarshalData(mcb,4,(byte *)&fh->Attr.atime.seconds);		
		MarshalData(mcb,4,(byte *)&fh->Attr.mtime.seconds);

	}

}

nfshandle *create_obj(MCB *mcb, word type, AUTH *auth)
{
	IOCCommon *req = (IOCCommon *)mcb->Control;
	int context = (int) req->Context;
	int name = (int) req->Name;
	int next = (int) req->Next;
	char *cpath = mcb->Data+context;
	char *tpath = mcb->Data+name;
	nfshandle *th;
	char *tpe = tpath+strlen(tpath);
	char *oname = tpath;
	int ugid = ((int *)&req->Access)[1];

	
	if( DEBUG ) IOdebug("NFS create [%d,%d,%d] %s+%s",context,name,next,cpath,tpath);
		
	/* scan backwards from end of name to create */
	while( *tpe != '/' && tpe != tpath ) tpe--;
	
	/* remove object name from request */
	if( tpe != tpath ) *tpe = 0,oname=tpe+1;
	else req->Name = -1;

	/* if next is past current end of name, set to end */
	if ( req->Name == -1 ) { if ( next > name ) req->Next = (word) name - 1; }
	elif( next > tpe-mcb->Data ) req->Next = tpe-mcb->Data;
		
	th = gettarget(mcb,auth);
	
	if( th )
	{
		struct createargs args;
		struct diropres *res;
		char pathname[512];
		diropres *(*nfs_create)();
		
		if( type == Type_File ) nfs_create = nfsproc_create_2;
		else nfs_create = nfsproc_mkdir_2;

		if( DEBUG ) IOdebug("NFS create %s+%s",th->Path,oname);
				
		args.where.dir = th->File;
		args.where.name = oname;
		
		args.attributes.mode = type==Type_File?0100664:0040755;
		args.attributes.uid = ugid & 0xFFFF;
		args.attributes.gid = (ugid>>16) & 0xFFFF;
		args.attributes.size = 0;
		args.attributes.atime.seconds = -1;
		args.attributes.atime.useconds = -1;
		args.attributes.mtime.seconds = -1;
		args.attributes.mtime.useconds = -1;
		
		Wait(&nfslock);
		nfsclnt->cl_auth = auth;
		while((res = nfs_create(&args, nfsclnt)) == NULL)
			clnt_perror(nfsclnt,"create");

		if( res->status != NFS_OK )
		{
			if( DEBUG ) IOdebug("NFS create %s/%s stat %d",th->Path,oname,res->status);
			th->Error = res->status;
			mcb->MsgHdr.FnRc = EC_Error|SS_HardDisk|EG_Errno|nfs_errno(res->status);
			Signal(&nfslock);
			FreeHandle(th);
			return NULL;	
		}
		
		makepath(pathname,th->Path,oname);
		
		FreeHandle(th);
		
		th = NewHandle(pathname,&res->diropres_u.diropres.attributes,
					&res->diropres_u.diropres.file);

		Signal(&nfslock);
	}

	if( tpe != tpath ) *tpe = '/';
	else req->Name = name;

	return th;
}

void do_open(MMsgBuf *m)
{
	MCB *mcb = &m->mcb;
	IOCMsg2 *req = (IOCMsg2 *)mcb->Control;
	int mode = (int) req->Arg.Mode;
	nfshandle *fh;
	bool created = FALSE;
	AUTH *auth = makeauth(&req->Common.Access);

	fh = gettarget(mcb,auth);	

	if( fh == NULL && (mode & O_Create) ) 
		created = TRUE, fh = create_obj(mcb,Type_File,auth);
 
	if( fh == NULL )
	{
		ErrorMsg(mcb,mcb->MsgHdr.FnRc);
		goto done;
	}

	if( !checkaccess(fh,&req->Common.Access,mode) )
	{
		ErrorMsg(mcb,EC_Error|EG_Protected|EO_Object);
		goto done;
	}
	
	/* if we have not created the file, and the truncate bit is 	*/
	/* set, do a truncate here					*/
	if( !created && (mode & O_Truncate) )
	{
		struct sattrargs args;
		struct attrstat *res;
		
		args.file = fh->File;
		args.attributes.mode = -1;
		args.attributes.uid = -1;
		args.attributes.gid = -1;
		args.attributes.size = 0;
		args.attributes.atime.seconds = -1;
		args.attributes.atime.useconds = -1;
		args.attributes.mtime.seconds = -1;
		args.attributes.mtime.useconds = -1;
		
		Wait(&nfslock);
		nfsclnt->cl_auth = auth;
		while((res = nfsproc_setattr_2(&args, nfsclnt)) == NULL)
			clnt_perror(nfsclnt,"truncate");


		if( res->status != NFS_OK )
		{
			if( DEBUG ) IOdebug("NFS truncate %s stat %d",fh->Path,res->status);
			ErrorMsg(mcb,EC_Error|SS_HardDisk|EG_Errno|nfs_errno(res->status));
			Signal(&nfslock);
			goto done;
		}

		fh->Attr = res->attrstat_u.attributes;		
		
		Signal(&nfslock);
	}
	
	form_open_reply(mcb,fh,Flags_Closeable|Flags_NoIData);
	mcb->MsgHdr.Reply = NewPort();
	PutMsg(mcb);
	
	if(fh->Attr.type == NFDIR ) dir_server(mcb,fh,auth);
	else file_server(mcb,fh,auth);

done:
	FreeHandle(fh);
	Free(m);
	freeauth(auth);
}

void do_create(MMsgBuf *m)
{
	MCB *mcb = &m->mcb;
	IOCCreate *req = (IOCCreate *)mcb->Control;
	nfshandle *fh;
	AUTH *auth = makeauth(&req->Common.Access);
	
	fh = create_obj(mcb,req->Type,auth);
	
	if( fh == NULL )
	{
		ErrorMsg(mcb,mcb->MsgHdr.FnRc);
		goto done;
	}
	
	form_open_reply(mcb,fh,0);
	PutMsg(mcb);

	FreeHandle(fh);
done:
	freeauth(auth);
	Free(m);		
}

void do_locate(MMsgBuf *m)
{
	MCB *mcb = &m->mcb;
	IOCMsg1 *req = (IOCMsg1 *)mcb->Control;
	nfshandle *fh;
	AUTH *auth = makeauth(&req->Common.Access);

	mcb->MsgHdr.FnRc = SS_HardDisk;
	
	fh = gettarget(mcb,auth);

	if( fh == NULL )
	{
		ErrorMsg(mcb,mcb->MsgHdr.FnRc);
		goto done;
	}
	
	form_open_reply(mcb,fh,0);
		
	PutMsg(mcb);
		
done:
	FreeHandle(fh);
	Free(m);
	freeauth(auth);
}

void
do_objinfo(MMsgBuf *m)
{
	MCB *mcb = &m->mcb;
	IOCMsg1 *req = (IOCMsg1 *)mcb->Control;
	nfshandle *fh;
	AUTH *auth = makeauth(&req->Common.Access);
	
	mcb->MsgHdr.FnRc = SS_HardDisk;
	
	fh = gettarget(mcb,auth);

	if( fh == NULL )
	{
		ErrorMsg(mcb,mcb->MsgHdr.FnRc);
		goto done;
	}

	marshal_info(mcb,fh,auth);
		
	PutMsg(mcb);
		
done:
	FreeHandle(fh);
	Free(m);
	freeauth(auth);
}

void
do_servinfo(MMsgBuf *m)
{
	MCB *mcb = &m->mcb;
	statfsres *res;
	statfsokres *info;
	FSInfo fsi;
	AUTH *auth = makeauth(&((IOCCommon *)mcb->Control)->Access);
	
	Wait(&nfslock);
	nfsclnt->cl_auth = auth;
	while((res = nfsproc_statfs_2(&roothandle->File, nfsclnt)) == NULL )
		clnt_perror(nfsclnt,"statfs");
	Signal(&nfslock);

	freeauth(auth);
	
	if( res->status != NFS_OK )
	{
		ErrorMsg(mcb,EC_Error|SS_HardDisk|EG_Errno|nfs_errno(res->status));
		goto done;
	}

	info = &res->statfsres_u.reply;

	fsi.Flags = Flags_Server;
	fsi.Size  = info->blocks * (long) info->bsize;
	fsi.Avail = info->bavail * (long) info->bsize;
	fsi.Used  = fsi.Size-fsi.Avail;
	
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,0);
	MarshalData(mcb,sizeof(fsi),(byte *)&fsi);

	PutMsg(mcb);
done:
	Free(m);
	
	if( DEBUG )
	{
		Wait(&cachelock);
		WalkList(&cache,putentry);
		Signal(&cachelock);
	}

}

void
do_delete(MMsgBuf *m)
{
	MCB *mcb = &m->mcb;
	nfshandle *fh, *pfh;
	diropargs args;
	nfsstat *res;
	char name[NameMax];
	nfsstat *(*nfs_remove)() = nfsproc_remove_2;
	AUTH *auth = makeauth(&((IOCCommon *)mcb->Control)->Access);
	
	mcb->MsgHdr.FnRc = SS_HardDisk;
	
	fh = gettarget(mcb,auth);

	if( fh == NULL )
	{
		ErrorMsg(mcb,mcb->MsgHdr.FnRc);
		goto done;
	}

	if( fh->Attr.type == NFDIR ) nfs_remove = nfsproc_rmdir_2;

	pfh = followpath(fh->Path,"..",auth);
		
	if( pfh == NULL )
	{
		FreeHandle(fh);
		ErrorMsg(mcb,mcb->MsgHdr.FnRc);
		goto done;
	}

	strcpy(name,objname(fh->Path));

	FreeHandle(fh);

	args.dir = pfh->File;
	args.name = name;

	Wait(&nfslock);
	nfsclnt->cl_auth = auth;
	while((res = nfs_remove(&args, nfsclnt)) == NULL)
			clnt_perror(nfsclnt,"remove");

	if( *res != NFS_OK )
	{
		if( DEBUG ) IOdebug("NFS remove %s/%s stat %d",pfh->Path,name,*res);
		pfh->Error = *res;
		ErrorMsg(mcb,EC_Error|SS_HardDisk|EG_Errno|nfs_errno(*res));
		Signal(&nfslock);
		FreeHandle(pfh);
		goto done;
	}

	Signal(&nfslock);
	
	FreeHandle(pfh);

	ErrorMsg(mcb,0);	
done:
	Free(m);
	freeauth(auth);

}

void
do_rename(MMsgBuf *m)
{
	MCB *mcb = &m->mcb;
	IOCMsg2 *req = (IOCMsg2 *)mcb->Control;
	nfshandle *fh = NULL, *pfh = NULL;
	nfshandle *pth = NULL;
	renameargs args;
	nfsstat *res;
	char *tpath = mcb->Data + req->Arg.ToName;
	char *tpe = tpath+strlen(tpath);
	char fromname[NameMax];
	char toname[NameMax];
	word error = Err_Null;
	AUTH *auth = makeauth(&((IOCCommon *)mcb->Control)->Access);
	
	mcb->MsgHdr.FnRc = SS_HardDisk;
	
	fh = gettarget(mcb,auth);

	if( fh == NULL )
	{
		error = mcb->MsgHdr.FnRc;
		goto done;
	}

	/* get source object's parent	*/
	
	pfh = followpath(fh->Path,"..",auth);
		
	if( pfh == NULL )
	{
		error = mcb->MsgHdr.FnRc;
		goto done;
	}

	strcpy(fromname,objname(fh->Path));

	args.from.dir = pfh->File;
	args.from.name = fromname;

	/* now the destination		*/

	/* scan backwards from end of name to create */
	while( *tpe != '/' && tpe != tpath ) tpe--;
	
	/* remove object name from path */
	if( tpe != tpath ) strcpy(toname,tpe+1);
	else strcpy(toname,tpe);
	*tpe = 0;

	if( req->Common.Next > req->Common.Name )
		req->Common.Next = req->Arg.ToName;

	req->Common.Name = req->Arg.ToName;

	pth = gettarget(mcb,auth);

	if( pth == NULL )
	{
		error = mcb->MsgHdr.FnRc;
		goto done;
	}

	args.to.dir = pth->File;
	args.to.name = toname;

	if( DEBUG ) IOdebug("NFS rename %s/%s to %s/%s",pfh->Path,fromname,pth->Path,toname);
	
	Wait(&nfslock);
	nfsclnt->cl_auth = auth;
	while((res = nfsproc_rename_2(&args, nfsclnt)) == NULL)
			clnt_perror(nfsclnt,"rename");

	if( *res != NFS_OK )
	{
		if( DEBUG ) IOdebug("NFS rename %s/%s stat %d",pfh->Path,fromname,*res);
		pfh->Error = *res;
		error = EC_Error|SS_HardDisk|EG_Errno|nfs_errno(*res);
	}

	Signal(&nfslock);
	
done:
	if(  fh != NULL ) FreeHandle( fh);
	if( pfh != NULL ) FreeHandle(pfh);
	if( pth != NULL ) FreeHandle(pth);

	ErrorMsg(mcb,error);	
	
	Free(m);

	freeauth(auth);
}


/* In theory we should create a symbolic link, however, getting it	*/
/* wrong crashes the SUN, so I cannot debug this...We make hard links	*/
/* instead...								*/
void
do_link(MMsgBuf *m)
{
	MCB *mcb = &m->mcb;
	IOCMsg3 *req = (IOCMsg3 *)mcb->Control;
	nfshandle *pth = NULL;
	nfshandle *fh = NULL;
	linkargs args;
	nfsstat *res;
	char name[NameMax];
	char *tpath = mcb->Data + req->Common.Name;
	char *tpe = tpath+strlen(tpath);
	char *fromp;
	word error = 0;
	AUTH *auth = makeauth(&((IOCCommon *)mcb->Control)->Access);
	
	mcb->MsgHdr.FnRc = SS_HardDisk;

	/* scan backwards from end of name to create */
	while( *tpe != '/' && tpe != tpath ) tpe--;
	
	/* remove object name from path */
	if( tpe != tpath ) strcpy(name,tpe+1);
	else strcpy(name,tpe);
	*tpe = 0;

	if( req->Common.Next > (req->Common.Name + (tpe-tpath)) )
		req->Common.Next = req->Common.Name + (tpe-tpath);
		

	pth = gettarget(mcb,auth);

	if( pth == NULL )
	{
		error = mcb->MsgHdr.FnRc;
		goto done;
	}

	fromp = &mcb->Data[req->Name];

	{
		char mcname[100];
		MachineName(mcname);
		strcat(mcname,"/");
		strcat(mcname,MyName);
		if( strncmp(mcname,fromp,strlen(mcname) ) != 0 )
		{
			error = EC_Error|EG_Invalid|EO_Name;
			goto done;
		}
		fromp += strlen(mcname) + 1;
	}
	

	fh = followpath(fromp,"",auth);

	if( fh == NULL )
	{
		error = mcb->MsgHdr.FnRc;
		goto done;
	}

	if( DEBUG ) IOdebug("NFS link from %s to %s/%s",fh->Path,pth->Path,name);
	
	args.from = fh->File;
	args.to.dir = pth->File;
	args.to.name = name;

	Wait(&nfslock);
	nfsclnt->cl_auth = auth;
	while((res = nfsproc_link_2(&args, nfsclnt)) == NULL)
			clnt_perror(nfsclnt,"symlink");

	if( *res != NFS_OK )
	{
		if( DEBUG ) IOdebug("NFS link %s/%s stat %d",pth->Path,name,*res);
		pth->Error = *res;
		error = EC_Error|SS_HardDisk|EG_Errno|nfs_errno(*res);
	}

	Signal(&nfslock);

done:	
	if( pth != NULL ) FreeHandle(pth);
	if(  fh != NULL ) FreeHandle( fh);

	ErrorMsg(mcb,error);	

	Free(m);

	freeauth(auth);
}

void
do_protect(MMsgBuf *m)
{
	MCB *mcb = &m->mcb;
	IOCMsg2 *req = (IOCMsg2 *)mcb->Control;
	nfshandle *fh = NULL;
	sattrargs args;
	attrstat *res;
	word error = Err_Null;
	int mode;
	AUTH *auth = makeauth(&((IOCCommon *)mcb->Control)->Access);
	
	mcb->MsgHdr.FnRc = SS_HardDisk;
	
	fh = gettarget(mcb,auth);

	if( fh == NULL )
	{
		error = mcb->MsgHdr.FnRc;
		goto done;
	}

	args.file = fh->File;

	mode = fh->Attr.mode;

	mode = (mode & 0xFFFFF000) | matrix2mode( (word) mode & NFSMODE_FMT, req->Arg.Matrix);

	if( DEBUG ) IOdebug("NFS protect %s modes old %x new %x",fh->Path,fh->Attr.mode,mode);	

	args.attributes.mode = mode;
	args.attributes.uid = -1;
	args.attributes.gid = -1;
	args.attributes.size = -1;
	args.attributes.atime.seconds = -1;
	args.attributes.atime.useconds = -1;
	args.attributes.mtime.seconds = -1;
	args.attributes.mtime.useconds = -1;
		
	Wait(&nfslock);
	nfsclnt->cl_auth = auth;
	while((res = nfsproc_setattr_2(&args, nfsclnt)) == NULL)
			clnt_perror(nfsclnt,"protect");

	if( res->status != NFS_OK )
	{
		if( DEBUG ) IOdebug("NFS protect %s stat %d",fh->Path,res->status);
		fh->Error = res->status;
		error = EC_Error|SS_HardDisk|EG_Errno|nfs_errno(res->status);
	}
	else fh->Attr = res->attrstat_u.attributes;

	Signal(&nfslock);
	
done:
	if( fh != NULL ) FreeHandle(fh);

	ErrorMsg(mcb,error);	

	Free(m);

	freeauth(auth);
}

void
do_setdate(MMsgBuf *m)
{
	MCB *mcb = &m->mcb;
	IOCMsg4 *req = (IOCMsg4 *)mcb->Control;
	nfshandle *fh = NULL;
	sattrargs args;
	attrstat *res;
	word error = Err_Null;

	AUTH *auth = makeauth(&((IOCCommon *)mcb->Control)->Access);
	
	mcb->MsgHdr.FnRc = SS_HardDisk;
	
	fh = gettarget(mcb,auth);

	if( fh == NULL )
	{
		error = mcb->MsgHdr.FnRc;
		goto done;
	}

	args.file = fh->File;

	if( DEBUG ) IOdebug("NFS setdate %s",fh->Path);

	args.attributes.mode = -1;
	args.attributes.uid = -1;
	args.attributes.gid = -1;
	args.attributes.size = -1;
	args.attributes.atime.seconds = req->Dates.Access==0?-1:req->Dates.Access;
	args.attributes.atime.useconds = 0;
	args.attributes.mtime.seconds = req->Dates.Modified==0?-1:req->Dates.Modified;
	args.attributes.mtime.useconds = 0;
		
	Wait(&nfslock);
	nfsclnt->cl_auth = auth;
	while((res = nfsproc_setattr_2(&args, nfsclnt)) == NULL)
			clnt_perror(nfsclnt,"setdate");

	if( res->status != NFS_OK )
	{
		if( DEBUG ) IOdebug("NFS setdate %s stat %d",fh->Path,res->status);
		fh->Error = res->status;
		error = EC_Error|SS_HardDisk|EG_Errno|nfs_errno(res->status);
	}
	else fh->Attr = res->attrstat_u.attributes;

	Signal(&nfslock);
	
done:
	if( fh != NULL ) FreeHandle(fh);

	ErrorMsg(mcb,error);	

	Free(m);

	freeauth(auth);

}

void
do_refine(MMsgBuf *m)
{
	MCB *mcb = &m->mcb;
	IOCMsg2 *req = (IOCMsg2 *)mcb->Control;
	nfshandle *fh = NULL;
	word error = Err_Null;
	Capability cap = req->Common.Access;
	AccMask mask =  req->Arg.AccMask;
	AUTH *auth = makeauth(&((IOCCommon *)mcb->Control)->Access);
	
	mcb->MsgHdr.FnRc = SS_HardDisk;
	
	fh = gettarget(mcb,auth);

	if( fh == NULL )
	{
		error = mcb->MsgHdr.FnRc;
		ErrorMsg(mcb,error);	
		goto done;
	}

	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,Err_Null);

	unless ( cap.Access & AccMask_A ) mask &= cap.Access;

	cap.Access = mask;

	MarshalCap(mcb,&cap);

	PutMsg(mcb);
	
done:
	if( fh != NULL ) FreeHandle(fh);

	Free(m);

	freeauth(auth);	
}

void
do_closeobj(MMsgBuf *m)
{
	ErrorMsg(&m->mcb,EC_Error|SS_HardDisk|EG_WrongFn|EO_File);
	Free(m);
}

void
do_revoke(MMsgBuf *m)
{
	ErrorMsg(&m->mcb,EC_Error|SS_HardDisk|EG_WrongFn|EO_File);
	Free(m);
}

int init_nfs()
{
	attrstat *attr;	
#if 0

	nfsclnt = clnt_create(HostName, NFS_PROGRAM, NFS_VERSION, "udp");
#else
	static struct timeval TIMEOUT = { 5, 0 };
	int bufsiz = 9*1024;
	int sock;
	struct hostent *h;
	struct sockaddr_in sin;

	h = gethostbyname(HostName);
	
	sin.sin_family = h->h_addrtype;
	sin.sin_port = 0;
	bzero(sin.sin_zero, sizeof(sin.sin_zero));
	bcopy(h->h_addr, (char*)&sin.sin_addr, h->h_length);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&bufsiz, sizeof(int)))
	  perror( "NFS: set send buffer size" );
	
	if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&bufsiz, sizeof(int)))
	  perror( "NFS: set receive buffer size" );
	
	nfsclnt = clntudp_create(&sin, NFS_PROGRAM, NFS_VERSION, TIMEOUT, &sock);
#endif
	if( nfsclnt == NULL )
	{
		clnt_pcreateerror(HostName);
		return 0;
	}

	gethostname(McName,sizeof(McName));

	defauth = authunix_create_default();

	nfsclnt->cl_auth = defauth;
	attr = nfsproc_getattr_2(&nfsroot, nfsclnt);
	
	if( attr->status != NFS_OK )
	{
		clnt_perror(nfsclnt,"getattr");
		return 0;
	}

	InitSemaphore(&nfslock,1);


	initcache();
		
	roothandle = NewHandle("/",&attr->attrstat_u.attributes,&nfsroot);
	
	roothandle->InUse++;
	
	return 1;
}

void dispatch(Port reqport)
{
	MMsgBuf *m = NULL;
	
	forever
	{
		word e;
		void (*fn)();
		
		if( m == NULL ) m = NewMMsgBuf();
		
		m->mcb.MsgHdr.Dest = reqport;
		m->mcb.Timeout = OneSec * 10;
		
		e = GetMsg(&m->mcb);

		if( (e&EG_Mask) == EG_Invalid ) break;

		if( e > 0 )
		{
			if( DEBUG ) IOdebug("NFS dispatch %F",e);
			switch( e & FG_Mask )
			{
			case FG_Open:		fn = do_open;	  break;
			case FG_Create:		fn = do_create;	  break;
			case FG_Locate:		fn = do_locate;	  break;
			case FG_ObjectInfo:	fn = do_objinfo;  break;
			case FG_ServerInfo:	fn = do_servinfo; break;
			case FG_Delete:		fn = do_delete;	  break;
			case FG_Rename:		fn = do_rename;	  break;
			case FG_Link:		fn = do_link;	  break;
			case FG_Protect:	fn = do_protect;  break;
			case FG_SetDate:	fn = do_setdate;  break;
			case FG_Refine:		fn = do_refine;	  break;
			case FG_CloseObj:	fn = do_closeobj; break;
			case FG_Revoke:		fn = do_revoke;	  break;
			default:
				InitMCB(&m->mcb,0,m->mcb.MsgHdr.Reply,NullPort,EC_Error|SS_HardDisk|EG_WrongFn);
				PutMsg(&m->mcb);
				continue;
			}
			
			if( !Fork(STACKSIZE,fn,sizeof(m),m) )
			{
				InitMCB(&m->mcb,0,m->mcb.MsgHdr.Reply,NullPort,EC_Error|SS_HardDisk|EG_NoMemory);
				PutMsg(&m->mcb);
				continue;
			}
			m = NULL;
		}
	}
}

void AddName(char *name, Port reqport)
{
	Object *o;
	char mcname[100];
	NameInfo info;
	
	MachineName(mcname);
	o = Locate(NULL,mcname);
	
	info.Port = reqport;
	info.Flags = Flags_StripName;
	info.Matrix = DefNameMatrix;
	info.LoadData = NULL;
	
	RootObj = Create(o, name, Type_Name, sizeof(NameInfo), (byte *)&info);

	Close(o);
	
	if( RootObj == NULL )
	{
		printf("NFS %s failed to create name server entry",name);
		Exit(1);
	}
}

Port reqport;

void quit(int sig)
{
	if( DEBUG ) IOdebug("NFS %s quitting",MyName);
	FreePort(reqport);
	sig=sig;
}

void CleanUp(void)
{
	if( DEBUG ) IOdebug("NFS %s CleanUp",MyName);
	DisMount(HostName,HostRoot);
	Delete(RootObj, NULL );
	exit(0);
}

extern int maxread;
extern int maxwrite;

int main(int argc, char **argv)
{
	int err = 0;
	struct sigaction act;

	if( argc < 4 )
	{
		printf("usage: nfs [-u user] [-s RW] <fsname> <host> <root>\r\n");
		exit(1);
	}

	if( strncmp("-u",argv[1],2) == 0 )
	{
		char *arg = argv[1];
		argv++;
		if( arg[2] != 0 ) UserName = arg+2;
		else UserName = argv[1], argv++;
	}
	
	if( strncmp("-s",argv[1],2) == 0 )
	{
		char *arg = argv[1];
		argv++;
		if( arg[2] != 0 ) arg = arg+2;
		else arg = argv[1], argv++;

		if( '1' <= arg[0] && arg[0] <= '8' )
			maxread = 1024 * (arg[0] - '0');

		if( '1' <= arg[1] && arg[1] <= '8' )
			maxwrite = 1024 * (arg[1] - '0');
	}

	MyName = argv[1];
	HostName = argv[2];
	HostRoot = argv[3];

	while( MyName[0] == '/' ) MyName++;

	/* If no UserName is provided, we use whatever the current uid	*/
	/* gid are set to. If a user name is provided, it must either	*/
	/* match the current uid/gid pair, or the current uid must be 0.*/
	
	if( UserName != NULL )
	{
		struct passwd *pw;
		pw = getpwnam(UserName);

		if( pw )
		{
			uid_t uid = getuid();
			gid_t gid = getgid();

			if
			(!(
				uid == 0  ||
				uid == -1 ||
				(uid == pw->pw_uid && gid == pw->pw_gid)
			))
			{
				printf("NFS %s: sorry, you cannot pretend to be %s\n",MyName,UserName);
				exit(1);
			}
			
			setuid(pw->pw_uid);
			setgid(pw->pw_gid);
		}
		else
		{
			printf("NFS %s: sorry, unknown user %s\n",MyName,UserName);
			exit(1);
		}
	}

	err = Mount(HostName,HostRoot);

	if(err != 0)
	{
		printf("NFS %s: failed to mount %s:%s error %d\n",MyName,HostName,HostRoot,err);
		exit(1);
	}

	reqport = NewPort();
	
	AddName(MyName,reqport);	

	init_nfs();
	
	act.sa_handler = quit;
	act.sa_mask = 0;
	act.sa_flags = SA_ASYNC;
	
	sigaction(SIGINT,&act,NULL);
	sigaction(SIGTERM,&act,NULL);
	sigaction(SIGQUIT,&act,NULL);

	printf("NFS Version 1.4 /%s -> %s:%s mounted\n",MyName,HostName,HostRoot);
	
	dispatch(reqport);
	
	CleanUp();
}

