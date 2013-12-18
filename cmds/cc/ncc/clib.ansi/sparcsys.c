/* sparcsys.c:  Copyright (C) Codemist Ltd         */
/* Very Sparc specific routines.		     */

/* version 1 */

#define __system_io 1
#include "hostsys.h"                            /* things like _initio() */
#include <stdio.h>                              /* for EOF               */
#include <stdlib.h>                             /* for exit()            */
#include <ctype.h>                              /* for isprint()         */
#include <string.h>                             /* for strlen()          */
#include <time.h>                               /* for clock             */
#include "/net/smaug/users/jpff/ncc.include/sys/times.h" /* for times             */

extern int main(int argc, char **argv, char **env);/* the point of it all   */
extern void _ctype_init(void);

volatile int errno;

/* timing things... */
struct timeval {
	long	tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};

struct  rusage {
  struct timeval ru_utime;        /* user time used */
  struct timeval ru_stime;        /* system time used */
  int     ru_maxrss;
  int     ru_ixrss;               /* integral shared memory size */
  int     ru_idrss;               /* integral unshared data size */
  int     ru_isrss;               /* integral unshared stack size */
  int     ru_minflt;              /* page reclaims */
  int     ru_majflt;              /* page faults */
  int     ru_nswap;               /* swaps */
  int     ru_inblock;             /* block input operations */
  int     ru_oublock;             /* block output operations */
  int     ru_msgsnd;              /* messages sent */
  int     ru_msgrcv;              /* messages received */
  int     ru_nsignals;            /* signals received */
  int     ru_nvcsw;               /* voluntary context switches */
  int     ru_nivcsw;              /* involuntary context switches */
};

clock_t clock()
{   struct rusage usage;
    _syscall2(SYS_getrusage, 0, (int) &usage);
    return (clock_t)(1000*usage.ru_utime.tv_sec+(usage.ru_utime.tv_usec/1000));
}

time_t time(time_t *timer)
{
  time_t ans;
  struct timeval {
    long tv_sec;        /* seconds since Jan. 1, 1970 */
    long tv_usec;       /* and microseconds */
  } tv;
  errno = 0;
  if (_syscall2(SYS_gettimeofday, (int)&tv, NULL) == 0)
    ans = (time_t)tv.tv_sec;
  else ans = (time_t)-1;
  if (timer != NULL) *timer = ans;
  return ans;      
}

/* system dependent I/O routines ... */

/* #include <sys/file.h> */
#define	O_RDONLY	000		/* open for reading */
#define	O_WRONLY	001		/* open for writing */
#define	O_RDWR		002		/* open for read & write */
#define	O_APPEND	00010		/* append on each write */
#define	O_CREAT		01000		/* open with file create */
#define	O_TRUNC		02000		/* open with truncation */

FILEHANDLE _sys_open(const char *filename, int openmode)
{   /* nasty magic number interface for openmode */
    static int modtab[6] = { /* r = */  O_RDONLY,
			     /* r+ = */ O_RDWR,
                             /* w = */  O_WRONLY|O_CREAT|O_TRUNC,
			     /* w+ = */ O_RDWR  |O_TRUNC|O_CREAT,
                             /* a = */  O_WRONLY|O_CREAT,
			     /* a+ = */ O_RDWR  |O_CREAT|O_APPEND };
    char *name = (char *)filename;            /* yuk */
    FILEHANDLE fh;
    openmode = modtab[openmode >> 1];         /* forget the 'b'inary bit */
    fh = _syscall3(SYS_open, (int)name, openmode, 0666);
    if (fh<0) return NONHANDLE;            /* not found             */
    return fh;
}

static int _error_recursion;

int _fileno(FILE *strm)
{
  return (int)strm->__file;
}

void _sys_msg(const char *s)
{ 
    if ((stderr->__flag & _IOWRITE) && !_error_recursion)
    {   _error_recursion = 1;
        fprintf(stderr, "\n%s\n", s);
        _error_recursion = 0;
    }
    else
    {   _ttywrite((unsigned char *)"\n", 1, 0);
        _ttywrite((unsigned char *)s, strlen(s), 0);
        _ttywrite((unsigned char *)"\n", 1, 0);
    }
}

int _sys_read_(FILEHANDLE fh, unsigned char *buf, int len, int mode)
{
  int n = _syscall3(SYS_read, (int)fh, (int)buf, (int)len);
  if (n==-1) {			/* error condition */
    n=0;
  }
  n = len - n;			/* number of bytes NOT read */
  if (n!=0) n |= 0x80000000;
  return n;
}

/* #include "/users/jpff/include.ncc/sys/types.h" */
/* #include "/users/jpff/include.ncc/sys/stat.h" */
struct stat {
      short  st_dev; /* device inode resides on */
      unsigned long  st_ino; /* this inode's number */
      unsigned short st_mode;/* protection */
      short  st_nlink;/* number or hard links to the file */
      short  st_uid; /* user-id of owner */
      short  st_gid; /* group-id of owner */
      short  st_rdev;/* the device type, for inode that is device */
      long   st_size;/* total size of file */
      time_t st_atime;/* file last access time */
      int    st_spare1;
      time_t st_mtime;/* file last modify time */
      int    st_spare2;
      time_t st_ctime;/* file last status change time */
      int    st_spare3;
      long   st_blksize;/* optimal blocksize for file system i/o ops */
      long   st_blocks;/* actual number of blocks allocated */
      long   st_spare4[2];
};

int _fh_length(FILEHANDLE fh)
{
  struct stat buf;
  if (_syscall2(SYS_fstat, (int)fh, (int)(&buf))==0) return (int)buf.st_size;
  return -1;
}

#define NUMBER_OF_EXIT_FUNCTIONS  10
int number_of_exit_functions = 0;
void (*_exitvector[NUMBER_OF_EXIT_FUNCTIONS])(void);

char ** _environ;
extern int* _curbrk;
extern int end;

int __init(int argc, char **argv, char **env)	/* Should be __init */
{   int ans;
    _environ = env;
    _error_recursion = 0;
    _curbrk = &end;
    _ctype_init();		/* init to C locale */
    _init_alloc();
    _exit_init();           /* must happen before exit() can be called   */
    _initio(NULL, NULL, NULL);
    ans = main(argc, argv, _environ);
    while (number_of_exit_functions!=0)
      (*_exitvector[--number_of_exit_functions])();
    _terminateio();
    _syscall1(SYS_exit, ans);
}

/* end of sparcsys.c */
