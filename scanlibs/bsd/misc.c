/* $Id: misc.c,v 1.8 1994/03/14 14:35:15 nickc Exp $ */

#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/dir.h>
#include <syslib.h>
#include <gsp.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <utime.h>
#include <nonansi.h>
#include <strings.h>

extern int ffs(int i)
{
	int n = 0;
	if( !i ) return -1;
	i = i & -i;
	/* we could use bitcnt here on T800, but T414 does not have it */
	while( i ) n++,i>>=1;
	return n;
}

extern int initgroups(char *name, int basegid)
{
	/* does nowt for now */
	errno = EPERM;
	return -1;
}

extern int setgroups(int ngroups, int *gidset)
{
	errno = EPERM;
	return -1;
}

extern int seteuid(int euid) { return setuid(euid); }
extern int setruid(int ruid) { return setuid(ruid); }
extern int setegid(int egid) { return setgid(egid); }
extern int setrgid(int rgid) { return setgid(rgid); }

extern int setreuid(int ruid, int euid)
{
	if( ruid != -1 ) setuid(ruid);
	if( euid != -1 ) setuid(euid);
	return 0;
}

extern char *getwd(char *path)
{
	return getcwd(path,PATH_MAX);
}

extern long telldir(DIR *dir)
{
	return dir->dd_pos+dir->dd_loc;
}

extern void seekdir(DIR *dir, long pos)
{
	lseek(dir->dd_fd,pos*sizeof(struct dirent),SEEK_SET);
	dir->dd_pos = pos;
	dir->dd_loc = 0;
	dir->dd_size = 0;
}

extern int readlink(char *path, char *buf, int bufsiz)
{
	Link_Info *info = (Link_Info *)Malloc(sizeof(Link_Info)+IOCDataMax);
	word e;
	int cc = -1;
	
	if( info == NULL ) { errno = ENOMEM; return -1;}
	
	e = ObjectInfo(CurrentDir,path,(byte *)info);
	
	if( e < 0 ) { errno = ENOENT; goto done;}
	
	if( (info->DirEntry.Type & ~Type_Flags) != Type_Link )
	{ errno = EINVAL; goto done; }
	
	cc = 17 + strlen(info->Name) + 1;
	
	if( cc > bufsiz ) { cc=-1,errno=ENOENT; goto done; }
	
	DecodeCapability(buf,&info->Cap);
	strcat(buf,info->Name);
	
done:
	Free(info);
	oserr = (int)e;
	return cc;
}

extern int isascii(int c)
{
	return (0 <= c) && (c < 0200);
}

extern int utimes(char *file, struct timeval *tvp)
{
	struct utimbuf times;
	
	times.ctime = 0;
	if( tvp == NULL )
	{
		times.actime = time(0);
		times.modtime = times.actime;
	}
	else
	{
		times.actime  = (int)tvp[0].tv_sec;
		times.modtime = (int)tvp[1].tv_sec;
	}
	
	return utime(file,&times);
}

extern int fchmod(int fd, mode_t mode)
{
	Stream *s = fdstream(fd);
	
	if( s == NULL ) return -1;
	
	return chmod(s->Name,mode);
}
