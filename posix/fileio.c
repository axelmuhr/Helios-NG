/*------------------------------------------------------------------------
--                                                                      --
--                     P O S I X    L I B R A R Y			--
--                     --------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- fileio.c								--
--                                                                      --
--	File I/O for Posix compatability library.			--
--                                                                      --
--	Author:  NHG 8/5/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: fileio.c,v 1.14 1993/07/12 10:35:29 nickc Exp $ */


#include <helios.h>	/* standard header */

#define __in_fileio 1	/* flag that we are in this module */

/*--------------------------------------------------------
-- 		     Include Files			--
--------------------------------------------------------*/

#include <stdarg.h>
#include <syslib.h>
#include <codes.h>
#include <gsp.h>
#include <string.h>

#include <posix.h>

#include "pposix.h"


static fdentry *	fdvec;
static Semaphore	fdlock;
static Object *		CurDir = NULL;
STATIC int		pipecount;
static int		fdvecsize;
static int		_cmask;
static char *		pipename = "/pipe/pipe.";

/* Forward refs... */
static int	   	findfd(  int low );
static fdentry *	checkfd( int fd  );
static void		freefd(  int fd  );

#ifdef NEVER
static Matrix		mode_to_matrix( mode_t mode );
#endif

static int		_close( fdentry * f );
static int		_dup( int fd, int low );
static int		addint( char * s, word i );
static int		posix_error( word e );

/* External routines... */
extern int open(char *name, int oflag, ... )
{
	int fd;
	fdentry *f;
	Stream *s = NULL;
	Pstream *p = NULL;
	word e = 0;

	CHECKSIGS();

	if( (fd = findfd(0)) == -1 ) goto done;

	if( (p = New(Pstream)) == NULL ) { e = EMFILE; goto done; }

	if( oflag & O_EXCL )
	{
		Object *o;

		unless ((o = Locate(CurDir, name)) == NULL)
		{ Close(o); e = EEXIST; goto done; }
	}

	if( oflag & O_APPEND ) oflag |= O_ReadWrite;

	if( (s = Open(CurDir,name,oflag)) == NULL )
	{ e = Result2(CurDir); goto done; }

#ifdef NEVER
	/* Create requires us to set the mode */
	if( oflag & O_CREAT )
	{
		va_list a;
		int mode;
		va_start(a,oflag);
		mode = va_arg(a,int);
		mode &= ~_cmask;
		if( Protect(CurDir,name,mode_to_matrix(mode)) < 0 )
		{
			e = Result2(CurDir);
			goto done;
		}
	}
#endif
	p->type = Type_File;
	p->refs = 1;
	p->stream = s;

	f = checkfd(fd);
	f->pstream = p;
	f->flags |= oflag;

	if( oflag & O_APPEND ) lseek(fd, 0, 2);

done:
	if( e != 0 )
	{
		errno = posix_error(e);
		if( s != NULL ) Close(s);
		if( p != NULL ) Free(p);
		freefd(fd);
		fd = -1;
	}

	CHECKSIGS();
	return fd;
}

extern int creat(char *path, mode_t mode)
{
	return open(path,O_WRONLY|O_CREAT|O_TRUNC,mode);
}


extern int umask(mode_t cmask)
{
	int oldmask = _cmask;
	_cmask = cmask;
	CHECKSIGS();
	return oldmask;
}

extern int link(char *path1, char *path2)
{
	Object *o;
	word e;

	CHECKSIGS();

	o  = Locate(CurDir,path1);

	if( o == NULL ) { errno = posix_error(Result2(CurDir)); return -1; }

	e = Link(CurDir,path2,o);

	if( e < Err_Null ) 
	{
		errno = posix_error(e);
		return -1;
	}

	CHECKSIGS();

	return 0;
}


extern int mkdir(char *path, mode_t mode)
{
	Object *o;

	CHECKSIGS();

	o = Create(CurDir,path,Type_Directory,NULL,0);

	if( o == NULL )
	{
		errno = posix_error(Result2(CurDir));
		return -1;
	}
	mode = mode;

	CHECKSIGS();

	return 0;
}

extern int mkfifo(char *path, mode_t mode)
{
	Object *o = NULL;
	Object *f = Locate(CurDir,"/fifo");

	CHECKSIGS();

	if( f == NULL ) { errno = (int)Result2(CurDir); return -1 ; }

	o = Create(f,path,Type_Fifo,NULL,0);	

	if( o == NULL )
	{
		errno = posix_error(Result2(CurDir)); 
		Close(f);
		return -1; 
	}

	CHECKSIGS();

	mode = mode;
	return 0;
}

extern int unlink(char *path)
{
	word e;

	CHECKSIGS();

	e = Delete(CurDir,path);

	if( e < 0 ) 
	{ errno = posix_error(e); return -1; }

	CHECKSIGS();

	return 0;
}

extern int rmdir(char *path)
{
	return unlink(path);
}

extern int rename(const char *old, const char *New)
{
	word e;

	CHECKSIGS();

	e = Rename(CurDir,(char *)old,(char *)New);

	if( e < 0 )
	{ errno = posix_error(e); return -1; }

	CHECKSIGS();

	return 0;
}

static void makestat(struct stat *buf)
{
	word mode = 0;
	
	/* zero the useless fields */
	buf->st_nlink = buf->st_ino = buf->st_dev = buf->st_gid = 0;
	buf->st_blksize = 4096;
	
	/* and fake up the mode field */
	mode |= (buf->st_matrix)&7;	
	mode |= (buf->st_matrix>>5)&070;
	mode |= (buf->st_matrix>>10)&0700;
	mode |= (buf->st_matrix>>15)&07000;
	
	if ( buf->st_type == Type_Fifo )		  mode |= S_IFIFO;
	elif(buf->st_type == Type_Pipe )		  mode |= S_IFIFO;
	elif((buf->st_type & ~Type_Flags) == Type_Socket) mode |= S_IFSOCK;
	elif(buf->st_type &  Type_Stream)		  mode |= S_IFREG;
	elif(buf->st_type &  Type_Directory)		  mode |= S_IFDIR;

	buf->st_mode = (int)mode;
}

extern int stat(char *path, struct stat *buf)
{
	word e;
	Object *o;
	
	CHECKSIGS();

	/* The Locate() forces resolution of links since ObjectInfo */
	/* does not follow a link if it is the target. Lstat (next) */
	/* applys ObjectInfo directly to the target.		    */
	
	o = Locate(CurDir,path);
	
	if( o != NULL ) 
	{
		e = ObjectInfo(o,NULL,(byte *)buf);
		Close(o);
	}
	else e = Result2(CurDir);

	if( e < 0 ) { errno = posix_error(e); return -1; }
		
	makestat(buf);
	
	CHECKSIGS();

	return 0;
}

extern int lstat(char *path, struct stat *buf)
{
	word e;
	Link_Info *info = (Link_Info *)Malloc(sizeof(Link_Info)+IOCDataMax);
	
	CHECKSIGS();
	
	if( info == NULL ) {errno = ENOMEM; return -1; }

	e = ObjectInfo(CurDir,path,(byte *)info);

	if( e < 0 ) { Free(info); errno = posix_error(e); return -1; }
		
	if( (info->DirEntry.Type & ~Type_Flags) == Type_Link )
	{
		memcpy(buf,info,sizeof(DirEntry));
		buf->st_uid = 0;
		buf->st_size = 0;
		buf->st_ctime = buf->st_atime = buf->st_mtime = 0;
	}
	else memcpy(buf,info,sizeof(ObjInfo));

	Free(info);
	makestat(buf);
	
	CHECKSIGS();

	return 0;
}

extern int fstat(int fd, struct stat *buf)
{
	fdentry *f;

	CHECKSIGS();

	if( (f = checkfd(fd)) == NULL ) return -1;

	if( f->pstream->type == Type_Pipe )
	{
		memset(buf,0,sizeof(struct stat));
		buf->st_size = (int)GetFileSize(f->pstream->stream);
		CHECKSIGS();
		return 0;
	}
	else return stat(f->pstream->stream->Name,buf);
}

extern int access(char *path, int amode)
{
	Object *o = NULL;
	int e = 0;

	CHECKSIGS();

	if( amode <= 0 || amode > 15 ) 
	{ errno = EINVAL; return -1; }

	o = Locate(CurDir,path);

	if( o == NULL )
	{ e = posix_error(Result2(CurDir)); goto done; }

	if( amode == F_OK ) goto done;

	amode &= ~F_OK;

	/* the amode bits are designed to equate with the Helios	*/
	/* AccMask RWE bits.						*/
	if( (amode & o->Access.Access) != amode ) e = EACCES;
done:
	if( o != NULL ) Close(o);
	CHECKSIGS();
	if( e != 0 ) { errno = e; return -1; }
	return 0;
}


extern int chmod(char *path, mode_t mode)
{
	word e = 0;
	Matrix matrix = 0;

	CHECKSIGS();
	
	matrix |=  (word)mode & 7;
	matrix |= ((word)mode & 070)   << 5;
	matrix |= ((word)mode & 0700)  << 10;
	matrix |= ((word)mode & 07000) << 15;

	matrix |= AccMask_D | AccMask_A;

	e = Protect(CurDir,path,matrix);

	if( e < 0 )
	{ errno = posix_error(e); return -1; }

	CHECKSIGS();
	return 0;	
}

extern int chown(char *path, uid_t owner, uid_t group)
{
	path = path;
	owner = owner;
	group = group;
	
	errno = EINVAL;
	CHECKSIGS();
	return 0;
}

extern int utime(char *path, struct utimbuf *times)
{
	DateSet *dates, _dates;
	int res = 0;
	
	CHECKSIGS();
	if( times == NULL )
	{
		dates = &_dates;
		dates->Access = GetDate();
		dates->Modified = GetDate();
	}
	else dates = (DateSet *)times;

	dates->Creation = 0;
	
	if(SetDate(CurDir,path,(DateSet *)times)<Err_Null)
	{ errno = posix_error(Result2(CurDir)); res = -1; goto done; }

done:
	CHECKSIGS();
	return res;
}

extern int pathconf(char *path, int name)
{
	path = path;
	CHECKSIGS();
	
	switch( name )
	{
	case	_PC_LINK_MAX:		return LINK_MAX;
	case	_PC_MAX_CANON:		return MAX_CANON;
	case	_PC_MAX_INPUT:		return MAX_INPUT;
	case	_PC_NAME_MAX:		return NAME_MAX;
	case	_PC_PATH_MAX:		return PATH_MAX;
	case	_PC_PIPE_BUF:		return PIPE_BUF;
	case	_PC_CHOWN_RESTRICTED:	return -1;
	case	_PC_NO_TRUNC:		return -1;
	case	_PC_VDISABLE:		return -1;
	}
	errno = EINVAL;
	return -1;
}

extern int fpathconf(int fd, int name)
{
	fd = fd;
	return pathconf(NULL,name);
}

int chdir(char *path)
{
	Object *NewDir;

	CHECKSIGS();
	if ((NewDir = Locate(CurDir, path)) == (Object *)NULL)
	{
		errno = ENOENT;
		return -1;
	}

	unless (NewDir->Type & Type_Directory)
	{
		errno = ENOTDIR;
		return -1;
	}

	if( CurDir != NULL ) Close(CurDir);
	CurDir = NewDir;
	CHECKSIGS();
	return 0;
}

char *getcwd(char *buf, int size)
{
	int cwdsize = strlen(CurDir->Name);
	
	CHECKSIGS();
	if( buf == NULL || size == 0 )
	{ errno = EINVAL; return NULL; }

	if( cwdsize >= size )
	{ errno = ERANGE; return NULL; }
	
	memcpy(buf,CurDir->Name,cwdsize+1);
	
	return buf;
}

extern int pipe(int fildes[2])
{
	char *name = NULL;
	Object *p;
	int e = 0;
	Stream *s0=NULL,*s1=NULL;
	Pstream *p0=NULL, *p1=NULL;
	int i;
	fdentry *f;


	CHECKSIGS();
	
	fildes[0] = fildes[1] = -1;
	
	p0 = New(Pstream);
	p1 = New(Pstream);

	if ( p0 == NULL || p1 == NULL )
	  {
	    e = ENOMEM;
	    
	    goto done;
	  }

	if ( (fildes[0]=findfd(0)) == -1 )
	  {
	    goto done;
	  }	

	if ( (fildes[1]=findfd(fildes[0]+1)) == -1 )
	  {
	    goto done;
	  }
		
	name = (char *)Malloc(40);

	if ( name == NULL )
	  {
	    e = ENOMEM;
	    goto done;
	  }

	for(  i = 0 ; i < 100 ; i++ )
	{
		pipecount %= 100;
		
		strcpy(name,pipename);
		addint(name,pipecount);

		/* IOdebug( "Pipe: attempting to create pipe %s, count = %d", name, pipecount ); */
		
		p = Create(CurDir,name,Type_Pipe,0,0);

		/* IOdebug( "Pipe: create returned %x", p ); */
		
		/* The rationale here is that while we are succesfully	*/
		/* creating pipes, we increment by 1. When we clash we	*/
		/* try to jump away to an unused region of the number	*/
		/* space.						*/
		/* we also assume that there will be less than 100 pipes*/
		/* in use simultaneously.				*/
		/* Note that 17 and 100 are coprime.			*/

		if ( p != NULL )
		  {
		    pipecount++;

		    break;
		  }

		pipecount += 17;		
	} 

 	s0 = PseudoStream(p,O_ReadOnly);
	s1 = PseudoStream(p,O_WriteOnly);

	Close(p);
	
	if ( s0 == NULL || s1 == NULL )
	  {
	    e = posix_error(Result2(CurDir));
	  
	    goto done;
	  }

	p0->type   = Type_Pipe;
	p0->refs   = 1;
	p0->stream = s0;

	p1->type   = Type_Pipe;
	p1->refs   = 1;
	p1->stream = s1;

	f = checkfd(fildes[0]);
	
	f->pstream = p0;

	f = checkfd(fildes[1]);
	
	f->pstream = p1;
	
done:
	if( name != NULL ) Free(name);
	
	if ( e != 0 )
	{
		if( p0 != NULL ) Free(p0);
		if( p1 != NULL ) Free(p1);
		if( s0 != NULL ) Close(s0);
		if( s1 != NULL ) Close(s1);
		freefd(fildes[0]);
		freefd(fildes[1]);
		errno = e;
		e = -1;
	} else e = 0;
	
	CHECKSIGS();
	
	return e;
}

extern int dup(int fd)
{
	int newfd;
	
	CHECKSIGS();
	if( (newfd = findfd(0)) == -1 ) return -1;
	
	_dup(fd,newfd);

	CHECKSIGS();
	return newfd;
}

extern int dup2(int fd,int fd2)
{
	CHECKSIGS();
	if( fd != fd2 )
	{
		fdentry *f;
		if( (f = checkfd(fd2)) != NULL) _close(f);
		else fd2 = findfd(fd2);
		
		fd2 = _dup(fd,fd2);
	}

	CHECKSIGS();
	return fd2;
}

extern int close(int fd)
{
	int res;
	fdentry *f;
	
	CHECKSIGS();
	if( (f = checkfd(fd)) == NULL ) return -1;

	res = _close(f);

	freefd(fd);
	
	CHECKSIGS();
	return res;
}

extern int
read(
     int 	fd,
     char *	buf,
     unsigned	nbyte )
{
  Stream *	s;
  word		res;
  word		timeout;
  fdentry *	f;

  
  CHECKSIGS();
  
  if ((f = checkfd( fd )) == NULL)
    return -1;
  
  s = f->pstream->stream;
  
  timeout = f->flags & O_NONBLOCK ? 0 : setuptimeout();

  res = Read( s, buf, nbyte, timeout );

  if (res == -1)
    {
      res = errno = 0;
    }  
  elif (res ==  0)
    {
      res   = -1;
      errno = posix_error( Result2( s ) );
    }
  elif (res < -1)
    {
      /*
       * XXX - bug fix added by NC 14/08/92
       *
       * This can happen if 's' is invalid
       */
      
      errno = posix_error( res );
      oserr = (int)res;
      res   = -1;      
    }  
  
  resettimeout();
  
  CHECKSIGS();
  
  return (int)res;

} /* read */


extern int write(int fd, char *buf, int nbyte)
{
	Stream *s;
	word res;
	word timeout;
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	s = f->pstream->stream;
	timeout = f->flags & O_NONBLOCK ? 0 : setuptimeout();
		
	res = Write(s,buf,nbyte,timeout);
	
	if( res < 0 )
	{
		res = -1;
		errno = posix_error(Result2(s));
	}
	resettimeout();	
	CHECKSIGS();
	return (int)res;
}

extern int fcntl(int fd, int cmd, ... )
{
	fdentry *f;
	va_list a;
	va_start(a,cmd);

	CHECKSIGS();
	if((f = checkfd(fd)) == NULL ) 	return -1;
	
	switch( cmd )
	{
	case F_DUPFD:
		return dup2(fd,va_arg(a,int));
	case F_GETFD:
		return (int)(f->flags & FD_GFDMASK);
	case F_SETFD:
		f->flags &= ~FD_SFDMASK;	
		f->flags |= FD_SFDMASK & va_arg(a,word);
		return 0;
	case F_GETFL:
		return (int)(f->flags & FD_GFLMASK);
	case F_SETFL:
		f->flags &= ~FD_SFLMASK;
		f->flags |= FD_SFLMASK & va_arg(a,word);
		return 0;
	case F_GETLK:
	case F_SETLK:
	case F_SETLKW:
	default:
		errno = EINVAL;
		return -1;
	}
}

extern off_t lseek(int fd, off_t offset, int whence)
{
	Stream *s;
	word res;
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL ) return -1;	
	
	s = f->pstream->stream;	

	res = Seek(s,whence,offset);

	if( res == -1 ) errno = posix_error(Result2(s));
	
	CHECKSIGS();
	return (int)res;
}

/* Non-standard extensions */

/* BSD4.3 routine */
extern int getdtablesize(void)
{
	return fdvecsize;
}

extern Stream *fdstream(int fd)
{
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL ) return NULL;	
	
	return f->pstream->stream;
}

extern int fderror(int fd)
{
	fdentry *f;

	CHECKSIGS();
	if((f = checkfd(fd)) == NULL ) return NULL;	
	
	return posix_error(f->pstream->stream->Result2);
}

extern Object *cdobj(void)
{
	CHECKSIGS();
	return CurDir;
}
	
extern int sopen(Stream *s)
{
	int fd;
	Pstream *p = NULL;
	word e = 0;
	fdentry *f;
	
	CHECKSIGS();
	if( (fd = findfd(0)) == -1 ) goto done;

	if( (p = New(Pstream)) == NULL ) { e = EMFILE; goto done; }

	p->type = Type_File;
	p->refs = 1;
	p->stream = s;

	f = checkfd(fd);
	f->pstream = p;
done:
	if( e != 0 ) 
	{
		errno = posix_error(e);
		freefd(fd);
		fd = -1;
	}

	CHECKSIGS();
	return fd;
}

extern int svopen(Stream *s, int fd)
{
	Pstream *p = NULL;
	word e = 0;
	fdentry *f;

	CHECKSIGS();
	if (s != (Stream *)MinInt)
	{
		if( findfd(fd) != fd ) { e = EMFILE; goto done; }
	
		if( (p = New(Pstream)) == NULL ) { e = EMFILE; goto done; }

		p->type = Type_File;
		p->refs = 1;
		p->stream = s;
	
		f = checkfd(fd);
		f->pstream = p;
	}

done:
	if( e != 0 ) 
	{
		errno = posix_error(e);
		freefd(fd);
		fd = -1;
	}

	CHECKSIGS();
	return fd;
}

/* internal support routines... */

static int _dup(int fd, int fd2)
{
	fdentry *f;
	fdentry *f2;
	
	errno = 0;
	
	if( (f = checkfd(fd)) == NULL ) goto done;

	if( (f2 = checkfd(fd2)) == NULL ) goto done;
	
	f2->pstream = f->pstream;

	f->pstream->refs++;

done:
	if( errno ) fd2 = -1;
	return fd2;
}

static int _close(fdentry *f)
{
	Pstream *p;

	p = f->pstream;
	--p->refs;
	
	if( p->refs == 0 )
	{
		Close(p->stream);
		Free(p);
	}
	
	return 0;
}

static int posix_error(word e)
{
	if( e >= 0 ) return (int)e;

	oserr = (int)e;

	if ( (e&SS_Mask) == SS_Loader || (e&SS_Mask) == SS_TFM )
		return ENOEXEC;

	switch( e&EG_Mask )
	{
	case EG_Errno:		return (int)(e & EO_Mask);
	case EG_NoMemory:	return ENOMEM;
	case EG_Create:		return EEXIST;
	case EG_Delete:		return EIO;
	case EG_Protected: 	return EACCES;
	case EG_Timeout:	return EAGAIN;
	case EG_Unknown:	return ENOENT;
	case EG_FnCode:		return EIO;
	case EG_Name:		return ENOENT;
	case EG_Invalid:	return EINVAL;
	case EG_InUse:		return EBUSY;
	case EG_Congested:	return EIO;
	case EG_WrongFn:	return EIO;
	case EG_Exception:
	{
		if( ( e & EE_Mask ) == EE_Abort ) return EINTR;
		elif( (e & EE_Signal) == EE_Signal )
		{
			/* by returning EE_Signal, servers can provoke a signal */
			/* routine						*/
			raise( (int)e & 0xff );
			return EINTR;
		}
		return EIO;
	}
	case EG_Broken:		
		if( (e & EO_Mask) == EO_Pipe ) 
		{
			raise(SIGPIPE);
			return EPIPE;
		}
		return EIO;
	case EG_WrongSize:	return EIO;
	case EG_Parameter:	return EINVAL;
	default:		return EIO;
	}
}

static int findfd(int low)
{
	int fd;
	fdentry *oldfdvec = fdvec;
	fdentry *newfdv;

	Wait( &fdlock );
	
again:
	for( fd = low ; fd < fdvecsize; fd++ )
		if( (fdvec[fd].flags & FD_ALLOC)==0 )  goto gotfd;
	
	/* the fdvec is full, re-allocate it */
	
	newfdv = (fdentry *)Malloc(((word)fdvecsize+fdvecinc)*sizeof(fdentry));
	if( newfdv == NULL ) { errno = EMFILE,fd = -1; goto bad; }

	memset(newfdv,0,(fdvecsize+fdvecinc)*sizeof(fdentry));
	memcpy(newfdv,fdvec,fdvecsize*sizeof(fdentry));
	fdvec = newfdv;
	fdvecsize += fdvecinc;
	Free(oldfdvec);
	goto again;
	
gotfd:
	fdvec[fd].flags = FD_ALLOC;		

bad:
	Signal(&fdlock);

	return fd;
}

static fdentry *checkfd(int fd)
{
	fdentry *f = NULL;

	Wait(&fdlock);
	
	if( fd < 0 || fd >= fdvecsize || (fdvec[fd].flags & FD_ALLOC) == 0)
	{ errno = EBADF; goto done; }
	
	f = &fdvec[fd];
done:
	Signal(&fdlock);

	return f;
	
}

static void freefd(int fd)
{
	Wait(&fdlock);

	if( fd < 0 || fd >= fdvecsize ) return;
	
	if( fdvec[fd].flags & FD_ALLOC ) fdvec[fd].flags = 0;
	
	Signal(&fdlock);
}

#ifdef NEVER
static Matrix mode_to_matrix(mode_t mode)
{
	Matrix matrix = 0;

	matrix |=  (word)mode & 7;
	matrix |= ((word)mode & 070)   << 5;
	matrix |= ((word)mode & 0700)  << 10;
	matrix |= ((word)mode & 07000) << 15;

	matrix |= 0x000040c0;			/* add da:d:: */

	return matrix;
}
#endif

static int addint(char *s, word i)
{	
  int len;
  if( i == 0 ) return strlen(s);
  len = addint(s,i/10);
  s[len] = (char)((i%10) + '0');
  s[len+1] = '\0';
  return len+1;
}

/* fdvec manipulation */

static Stream **marshalfdv(void)
{
	int i;
	int hwm = 0;
	Stream **strv = (Stream **)Malloc(sizeof(Stream *)*((word)fdvecsize+1));

	if( strv == NULL ) return NULL;
	
	Wait( &fdlock );
	for( i = 0 ; i < fdvecsize; i++ )
	{
		if( fdvec[i].flags & FD_ALLOC )
		{
			if( (fdvec[i].flags & FD_CLOEXEC) == 0 )
			{
				strv[i] = fdvec[i].pstream->stream;
				hwm = i;
				continue;
			}
		}
		strv[i] = (Stream *)MinInt;
	}

	Signal(&fdlock);
	
	strv[hwm+1] = NULL;
	
	return strv;
}

/* save fdvec and increment refs on open files */
static fdentry *savefdv(void)
{
	int i;
	fdentry *sfdvec = (fdentry *)Malloc(((word)fdvecsize+3)*sizeof(fdentry));
	if( sfdvec == NULL ) return NULL;

	Wait( &fdlock );
	for( i = 0 ; i < fdvecsize ; i++ )
	{
		if( fdvec[i].flags & FD_ALLOC ) fdvec[i].pstream->refs++;
		sfdvec[i] = fdvec[i];
	}	
	sfdvec[i].flags = -1;
	Signal( &fdlock );
	return sfdvec;
} 

static void restorefdv(fdentry *sfdvec)
{
	int i;

	Wait( &fdlock );
	for( i = 0 ; i < fdvecsize && (sfdvec[i].flags != -1); i++ )
	{
		fdentry *f = &fdvec[i];
		if( f->flags & FD_ALLOC ) _close(f);
		fdvec[i] = sfdvec[i];
	}
	Signal( &fdlock );

	Free(sfdvec);
}

static void abortfdv(void)
{
	int i;

	Wait( &fdlock );
	for( i = 0 ; i < fdvecsize ; i++ )
	{
		fdentry *f = &fdvec[i];
		if( f->flags & FD_ALLOC ) Abort(f->pstream->stream);
	}
	Signal( &fdlock );
}


static void init_fileio(Environ *env)
{
	Stream **s;
	int i;

	pipecount = 1;
		
	InitSemaphore(&fdlock,1);

	fdvec = NULL;
	fdvecsize = 0;	/* notes that we need to allocate a new fdvec */
	
	CurDir = env->Objv[0];

	for( i = 0, s = env->Strv; *s != NULL ; s++ ) {
		int j;

		if (fdvec == NULL)	/* dont indirect thru NULL pointer */
			j = i;
		else
			for( j = 0; j != i; j++ )
				if (fdvec[j].pstream != NULL)	/* dont indirect thru NULL pointer */
					if( fdvec[j].pstream->stream == *s )
						break;

		if( j == i )
			svopen(*s, i++);
		else
			dup2(j,i++);
	}	
}


/* -- End of fileio.c */

