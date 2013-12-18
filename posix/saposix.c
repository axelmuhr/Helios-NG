/* $Id: saposix.c,v 1.3 1990/11/26 17:59:06 nick Exp $ */
#define in_kernel
#include <root.h>
#include <config.h>
#include <errno.h>
#include <sem.h>
#include <link.h>
#include <syslib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <pwd.h>
#include <signal.h>
#include <termios.h>
#include <posix.h>
#include <dirent.h>
#include <asm.h>

#define LinkVector	((Channel *)MinInt)
#define OutLink(n)	(&LinkVector[(n)])
#define InLink(n)	(&LinkVector[(n)+4])

typedef struct Lstream {
	word		refs;
	word		linkno;
	Semaphore	lock;
} Lstream;

typedef struct fdentry {
	Lstream		*lstream;
} fdentry;

static fdentry *fdvec;
static int fdvecsize;

char *argv[] = { "sa", NULL };

static void lopen(int link, int fd);

extern char 	**_posix_init(void)
{
	Config *config = GetConfig();
	int i;
	
	fdvec = Malloc(sizeof(fdentry)*12);
	memset(fdvec,0,sizeof(fdentry)*12);
	fdvecsize = 12;
		
	for( i = 0; i < 4 ; i++ )
	{
		lopen(i,4+(i*2));
		lopen(i,5+(i*2));
		if( config->LinkConf[i].Flags & Link_Flags_parent )
		{
			dup2(4+(i*2),0);
			dup2(5+(i*2),1);
			dup2(5+(i*2),2);
			dup2(5+(i*2),3);
		}
	}
	
	return argv;
}

static void lopen(int link, int fd)
{
	Lstream *l = New(Lstream);

	l->refs = 1;
	l->linkno = link;
	InitSemaphore(&l->lock,1);
	
	fdvec[fd].lstream = l;
}

extern int dup2(int fd,int fd2)
{
	if( fd < 0 || fd >= fdvecsize || fdvec[fd].lstream == NULL)
	{ errno = EBADF; return -1; }

	if( fdvec[fd2].lstream != NULL ) close(fd2);
	
	fdvec[fd2].lstream = fdvec[fd].lstream;
	fdvec[fd2].lstream->refs++;
	
	return 0;
}

extern int fcntl(int fd, int cmd, ... ) 
{
	return 0; 
}

extern int close(int fd)
{
	Lstream *l = fdvec[fd].lstream;

	if( fd < 0 || fd >= fdvecsize || fdvec[fd].lstream == NULL)
	{ errno = EBADF; return -1; }

	if( --l->refs == 0 ) Free(l);
	
	fdvec[fd].lstream = NULL;
	
	return 0;
}

extern int 	read(int fd, char *buf, unsigned nbyte)
{
	Lstream *l = fdvec[fd].lstream;

	if( fd < 0 || fd >= fdvecsize || l == NULL )
	{ errno = EBADF; return -1; }
	
	Wait(&l->lock);	

	nbyte=1;	
	in_(nbyte,InLink(l->linkno),buf);
		
	Signal(&l->lock);
	
	return nbyte;
}

extern int 	write(int fd, char *buf, int nbyte)
{
	Lstream *l = fdvec[fd].lstream;

	if( fd < 0 || fd >= fdvecsize || l == NULL )
	{ errno = EBADF; return -1; }

	Wait(&l->lock);	

	out_(nbyte,OutLink(l->linkno),buf);

	Signal(&l->lock);
	
	return nbyte;
}

int select(nfds, readfds, writefds, exceptfds, tv)
int nfds, *readfds, *writefds, *exceptfds;
struct timeval *tv;
{ errno = EINVAL; return -1; }

void _posix_exception_handler(void)
{
	stopp_();
}

extern Stream 	*fdstream(int fd)
{
	static Stream s;
	s.Flags = 0;
	return &s;
}

#if 0
extern int 	open(char *name, int oflag, ... ) { errno = EINVAL; return -1; }
extern int 	creat(char *path, mode_t mode) { errno = EINVAL; return -1; }
extern int 	umask(mode_t cmask) { errno = EINVAL; return -1; }
extern int 	link(char *path1, char *path2) { errno = EINVAL; return -1; }
extern int 	mkdir(char *path, mode_t mode) { errno = EINVAL; return -1; }
extern int 	mkfifo(char *path, mode_t mode) { errno = EINVAL; return -1; }
extern int 	unlink(char *path) { errno = EINVAL; return -1; }
extern int 	rmdir(char *path) { errno = EINVAL; return -1; }
extern int 	rename(const char *old, const char *new) { errno = EINVAL; return -1; }
extern int 	access(char *path, int amode) { errno = EINVAL; return -1; }
extern int 	chmod(char *path, mode_t mode) { errno = EINVAL; return -1; }
extern int 	chown(char *path, uid_t owner, uid_t group) { errno = EINVAL; return -1; }
extern int 	pathconf(char *path, int name) { errno = EINVAL; return -1; }
extern int 	fpathconf(int fd, int name) { errno = EINVAL; return -1; }
extern int 	pipe(int fildes[2]) { errno = EINVAL; return -1; }
extern int 	dup(int fd) { errno = EINVAL; return -1; }
extern off_t 	lseek(int fd, off_t offset, int whence) { errno = EINVAL; return -1; }

extern int 	chdir(char *path) { errno = EINVAL; return -1; }
extern char 	*getcwd(char *buf, int size) { errno = EINVAL; *buf = 0; return buf; }

extern int 	getpid(void){ errno = EINVAL; return -1; }
extern int 	getppid(void){ errno = EINVAL; return -1; }
extern uid_t 	getuid(void){ errno = EINVAL; return -1; }
extern uid_t 	geteuid(void){ errno = EINVAL; return -1; }
extern uid_t 	getgid(void){ errno = EINVAL; return -1; }
extern uid_t 	getegid(void){ errno = EINVAL; return -1; }
extern uid_t 	setuid(uid_t uid){ errno = EINVAL; return -1; }
extern uid_t 	setgid(uid_t uid){ errno = EINVAL; return -1; }
extern int 	getgroups(int setsize, uid_t *list){ errno = EINVAL; return -1; }
extern char	*getlogin(void){ errno = EINVAL; return NULL; }
extern char	*cuserid(char *s){ errno = EINVAL; return NULL; }
extern int 	getprgrp(void){ errno = EINVAL; return -1; }
extern int 	setprgrp(void){ errno = EINVAL; return -1; }
extern int 	jcsetpgrp(int pgrp){ errno = EINVAL; return -1; }
extern time_t 	time(time_t *tloc){ errno = EINVAL; return -1; }
extern char 	*getenv(const char *name){ errno = EINVAL; return NULL; }
extern char 	*ctermid(char *s){ errno = EINVAL; return NULL; }
extern char	*ttyname(int fd){ errno = EINVAL; return NULL; }
extern int 	isatty(int fd){ errno = EINVAL; return -1; }
extern int 	sysconf(int name){ errno = EINVAL; return -1; }

extern int 	vfork(void){ errno = EINVAL; return -1; }
extern int 	execl(char *path,...){ errno = EINVAL; return -1; }
extern int 	execv(char *path,char **argv){ errno = EINVAL; return -1; }
extern int 	execle(char *path,...){ errno = EINVAL; return -1; }
extern int 	execlp(char *file,...){ errno = EINVAL; return -1; }
extern int 	execvp(char *file, char **argv){ errno = EINVAL; return -1; }
extern int 	execve(char *name, char **argv, char **envv){ errno = EINVAL; return -1; }
extern void 	_exit(int code) { Exit(code); }
extern int	wait(int *statloc){ errno = EINVAL; return -1; }

/* The following are Helios extensions to map from Posix to Helios objects */


extern Object 	*cdobj(void){ errno = EINVAL; return NULL; }
extern int 	sopen(Stream *s) { errno = EINVAL; return -1; }
extern Environ  *getenviron(void){ errno = EINVAL; return NULL; }

extern int _posixflags(int how, int set ){ errno = EINVAL; return -1; }

extern int stat(char *path, struct stat *buf)  { errno = EINVAL; return -1; }
extern int fstat(int fd, struct stat *buf) { errno = EINVAL; return -1; }

extern DIR *opendir(char *pathname) { errno = EINVAL; return NULL; }
extern struct dirent *readdir(DIR *dir) { errno = EINVAL; return NULL; }
extern void rewinddir(DIR *dir) { errno = EINVAL; }
extern int closedir(DIR *dir) { errno = EINVAL; return -1; }


extern struct passwd *getpwent(void) { errno = EINVAL; return NULL; }
extern struct passwd *getpwuid(uid_t uid) { errno = EINVAL; return NULL; }
extern struct passwd *getpwnam(char *name) { errno = EINVAL; return NULL; }
extern void setpwent(void) { errno = EINVAL; }
extern int endpwent(void) { errno = EINVAL; return -1; }

extern void _ignore_signal_handler(int s) {}
extern void _default_signal_handler(int s) {}
extern void _error_signal_marker(int s) {}

extern int kill(int pid, int sig){ errno = EINVAL; return -1; }
extern int siginitset(sigset_t *set){ errno = EINVAL; return -1; }
extern int sigfillset(sigset_t *set){ errno = EINVAL; return -1; }
extern int sigaddset(sigset_t *set, int sig){ errno = EINVAL; return -1; }
extern int sigdelset(sigset_t *set, int sig){ errno = EINVAL; return -1; }
extern int sigismember(sigset_t *set, int sig){ errno = EINVAL; return -1; }
extern int sigaction(int sig, struct sigaction *act, struct sigaction *oact){ errno = EINVAL; return -1; }
extern int sigprocmask( int how, sigset_t *set, sigset_t *oset){ errno = EINVAL; return -1; }
extern int sigpending(sigset_t *set){ errno = EINVAL; return -1; }
extern int sigsuspend(sigset_t *set){ errno = EINVAL; return -1; }
extern unsigned int alarm(unsigned int sec){ errno = EINVAL; return 0; }
extern int pause(void){ errno = EINVAL; return -1; }
extern unsigned int sleep(unsigned int seconds){ errno = EINVAL; return 0; }
extern void (*signal (int sig, void (*func)(int)))(int){ errno = EINVAL; return NULL; }
extern int raise(int sig){ errno = EINVAL; return -1; }

extern int cf_getospeed(struct termios *t){ errno = EINVAL; return -1; }
extern int cf_setospeed(struct termios *t, int speed){ errno = EINVAL; return -1; }
extern int cf_getispeed(struct termios *t){ errno = EINVAL; return -1; }
extern int cf_setispeed(struct termios *t, int speed){ errno = EINVAL; return -1; }
extern int tcgetattr(int fd, struct termios *t){ errno = EINVAL; return -1; }
extern int tcsetattr(int fd, int actions, struct termios *t){ errno = EINVAL; return -1; }
extern int tcsendbreak(int fd, int duration){ errno = EINVAL; return -1; }
extern int tcdrain(int fd){ errno = EINVAL; return -1; }
extern int tcflush(int fd, int qselect){ errno = EINVAL; return -1; }
extern int tcflow(int fd, int action){ errno = EINVAL; return -1; }
extern int tcgetpgrp(int fd){ errno = EINVAL; return -1; }
extern int tcsetpgrp(int fd, int pid) { errno = EINVAL; return -1; }

extern int uname(struct utsname *name) { errno = EINVAL; return -1; }
extern clock_t times(struct tms *t) { errno = EINVAL; return -1; }
extern int wait2(int *stat_loc, int options) { errno = EINVAL; return -1; }

extern int system(char *s) {return -1;}
extern int atexit(void (*f)(void)) { errno = EINVAL; return -1; }
extern void exit(int n) { _exit(n); }
extern void abort() {_exit(1); }
extern int svopen(Stream *s, int fd) { errno = EINVAL; return -1; }
extern void pipename() {}
extern int utime(char *path, struct utimbuf *times) { errno = EINVAL; return -1; }

#endif

extern void find_file(char *path, char *file ) { strcpy(path,file); }

