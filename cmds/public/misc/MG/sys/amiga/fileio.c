/*
 * Name:	MG 2a401
 *		Commodore Amiga file I/O.
 * Last edit:	05-May-88 swalton@solar.stanford.edu
 * Next-to-Last edit:	16-Dec-87 mic@emx.utexas.edu
 * Created:	23-Jul-86 mic@emx.utexas.edu
 *
 * Read and write ASCII files. All of the low level file I/O
 * knowledge is here.  Uses AmigaDOS standard I/O and does its
 * own dynamic buffering; this seems to save about 2K worth
 * of space in the executable image.
 */

#ifdef		LATTICE
#include	<string.h>
#include	<exec/types.h>
#endif
#include	<exec/memory.h>
#include	<libraries/dos.h>
#include	<libraries/dosextens.h>
#ifdef		USE_ARP
#include	"libraries/arpbase.h"
#else
#define FCHARS	32L
#endif

#undef	TRUE
#undef	FALSE
#include	"def.h"

#define	NIOBUF			4096

extern ULONG			Rename(), UnLock(), Close(), FreeMem();
extern LONG			Write(), Read();
extern UBYTE			*AllocMem();
extern struct FileLock		*Lock();
extern struct FileHandle	*Open();

static struct FileHandle	*ffh = 0;
static UBYTE			*iobuf;
static int			ibufo, niobuf;
static LONG			iostat, access_mode;
#ifdef	MANX
extern char			*strncpy(), *strncat(), *index(), *rindex();
#endif
#ifdef	LATTICE
extern char			*malloc() ;
#define	index	strchr
#define rindex	strrchr
#endif

#define	getch()		(ibufo == niobuf) ? FillBuf() : iobuf[ibufo++]
#define putch(c)	{if (niobuf == NIOBUF) FlushBuf(); iobuf[niobuf++] = c;}

/*
 * Open the Emacs internal file for reading.
 */
ffropen(fn)
char	*fn;
{
	if ((iobuf = AllocMem((ULONG) NIOBUF, 0L)) == NULL)
		return (FIOERR);

	if ((ffh = Open(fn, access_mode = MODE_OLDFILE)) == 0L) {
		FreeMem(iobuf, (ULONG) NIOBUF);
		return (FIOFNF);
	}
	ibufo = niobuf = 0;
	return (FIOSUC);
}

/*
 * Open a file for writing.  Return TRUE if all
 * is well, and FALSE on error (cannot create).
 */

ffwopen(fn)
char	*fn;
{
	if ((iobuf = AllocMem((ULONG) NIOBUF, 0L)) == NULL)
		return (FIOERR);
	if ((ffh = Open(fn, access_mode = MODE_NEWFILE)) == 0L) {
		ewprintf("Cannot open file for writing");
		FreeMem(iobuf, (ULONG) NIOBUF);
		return (FIOERR);
	}
	niobuf = 0;
	iostat = NIOBUF;    /* pretend we wrote out a full buffer last time */
	return (FIOSUC);
}

/*
 * Close a file, flushing the output buffer.  Should look at
 * the status.
 */
ffclose()
{
	if (access_mode == MODE_NEWFILE)
		FlushBuf();
	if (ffh)
		(void) Close(ffh);
	if (iobuf)
		FreeMem(iobuf, (ULONG) NIOBUF);
	return (FIOSUC);
}

/*
 * Write a buffer to the already opened file. bp points to the
 * buffer. Return the status. Check only at the newline and
 * end of buffer.
 */
ffputbuf(bp)
BUFFER *bp;
{
    register char *cp;
    register char *cpend;
    register LINE *lp;
    register LINE *lpend;

    lpend = bp->b_linep;
    lp = lforw(lpend);
    do {
	cp = &ltext(lp)[0];		/* begining of line	*/
	cpend = &cp[llength(lp)];	/* end of line		*/
	while(cp != cpend)
	    putch(*(cp++));		/* putch only evalutes its arg once */
	lp = lforw(lp);
	if(lp == lpend) break;		/* no implied newline on last line */
	putch('\n');
    } while(iostat > 0L);

    if(iostat == -1L) {
	ewprintf("Write I/O error");
	return FIOERR;
    }
    return FIOSUC;
}

/*
 * Read a line from a file, and store the bytes
 * in the supplied buffer. Stop on end of file or end of
 * line.  When FIOEOF is returned, there is a valid line
 * of data without the normally implied \n.
 */
ffgetline(buf, nbuf, nbytes)
register char	*buf;
register int	nbuf;
register int	*nbytes;
{
	register int	c;
	register int	i;

	i = 0;
	while((c = getch())!=EOF && c!='\n') {
		buf[i++] = c;
		if (i >= nbuf) return FIOLONG;
	}
	if (c == EOF  && (iostat == -1L)) {
		ewprintf("File read error");
		return FIOERR;
	}
	*nbytes = i;
	return c==EOF ? FIOEOF : FIOSUC;
}

#ifndef	NO_BACKUP
/*
 * Rename the current file into a backup copy,
 * possibly after deleting the original file.
 */
fbackupfile(fname)
char	*fname;
{
	struct FileLock *twiddle, *lock;
	char buffer[NFILEN];

	(void) strncpy(buffer,fname,NFILEN - 1);
	(void) strcat(buffer,"~");

	lock = Lock(fname,(ULONG)EXCLUSIVE_LOCK);/* does file exist?	*/
	if (!lock)
		return (FALSE);			/* nope, return error	*/

	twiddle = Lock(buffer,(ULONG)EXCLUSIVE_LOCK);
	if (twiddle) {				/* delete old backup	*/
		UnLock(twiddle);		/* let it go		*/
		if (!DeleteFile(buffer)) {
			UnLock(lock);
			return (FALSE);
		}
		twiddle = NULL;
	}
	/* rename file to backup name (after unlocking the file)
	 */
	UnLock(lock);
	return (int) Rename(fname,buffer);
}
#endif	NO_BACKUP

#ifndef	NO_STARTUP
/*
 * Return name of user's startup file.  On Amiga, make it
 * .mg in the current directory, then s:.mg
 */

static char startname[] = ".mg";
static char altstartname[] = "s:.mg";

char *startupfile()
{
	struct FileLock *lock;

	if (lock = Lock(startname,(ULONG)SHARED_LOCK)) {
		UnLock(lock);
		return(startname);
	}
	if (lock = Lock(altstartname,(ULONG)SHARED_LOCK)) { /* alternate */
		UnLock(lock);
		return (altstartname);
	}
	return (NULL);
}
#endif	NO_STARTUP

/*
 * The string "fn" is a file name. Perform any required name adjustments,
 * including conversion to a fully qualified path if NO_DIR isn't defined.
 */

#define MAX_ELEMS	  8		/* Maximum number of elements	*/
extern char MyDirName[];

char *adjustname(fn)
register char	*fn;
{
#ifndef NO_DIR
	static char fnb[MAX_ELEMS*FCHARS + 1];
	struct FileLock *lock;
	long PathName();
	void TackOn();
	char *dup, *p;

	if (!index(fn, ':')) {			/* no device		*/
		strcpy(fnb, MyDirName);
		TackOn(fnb, fn);
		if (!index(fn, '/'))		/* completely bare name */
			return fnb;
	} else
		strcpy(fnb, fn);
	/*
	 * Else fn has some path components in it.  We try to PathName
	 * the whole thing first, but since the file specified by fn
	 * may not exist, we PathName the leading part and TackOn the
	 * trailing part if it doesn't.
	 */
	if (lock = Lock(fnb, SHARED_LOCK)) {
		if (PathName(lock, fnb, (long) MAX_ELEMS) !=0) {
			UnLock(lock);
			return fnb;
		}
		ewprintf("adjustname: PathName() failed!");
		UnLock(lock);
		return fn;
	}
	if (!(p = rindex(fnb, '/')))
		p = index(fnb, ':');
	p++;
	strcpy((dup = malloc(strlen(p) + 1)), p);
	*p = '\0';
	if (lock = Lock(fnb, SHARED_LOCK)) {
		if (PathName(lock, fnb, (long) MAX_ELEMS) != 0) {
			UnLock(lock);
			TackOn(fnb, dup);
			free(dup);
			return fnb;
		}
		ewprintf("adjustname: PathName() failed!");
		UnLock(lock);
	}
	free(dup);
#endif
	return fn;				/* if all else fails	*/
}

/*
 * Functions to read/write into the I/O buffer
 */

int FlushBuf()
{
	if (niobuf > 0) {
		iostat = Write(ffh, iobuf, (ULONG) niobuf);
		niobuf = 0;
	}
}

/*
 * Fill up the input buffer and return the first character in it.
 */
int FillBuf()
{
	if ((iostat = Read(ffh, iobuf, (ULONG) NIOBUF)) <= 0L)
		return (EOF);
	ibufo = 0;
	niobuf = (int) iostat;
	return (int) (iobuf[ibufo++]);
}

#ifndef NO_DIRED

#include "kbd.h"

copy(frname, toname)
char *frname, *toname;
{
#ifdef	MANX
	return fexecl("copy", "copy", frname, toname, (char *) 0);
#endif
#ifdef	LATTICE
	int	error ;
	if (error = forkl("copy", "copy", frname, toname, (char *) 0, (char *) 0, 2))
		return error ;
	return (int) wait(2) ;
#endif
}

BUFFER *dired_(dirname)
char *dirname;
{
    register BUFFER *bp;
    char line[256];
    BUFFER *findbuffer();
    char *tmpname, *mktemp();
    int i;
    VOID lfree();

    if((dirname = adjustname(dirname)) == NULL) {
	ewprintf("Bad directory name");
	return NULL;
    }
    if(!isdirectory(dirname)) {
	ewprintf("Not a directory: %s", dirname);
	return NULL;
    }
    if((bp = findbuffer(dirname)) == NULL) {
	ewprintf("Could not create buffer");
	return NULL;
    }
    bclear(bp);				/* clear out leftover garbage	*/
    (void) strcpy(line, "list >");
    (void) strncat(line, tmpname = mktemp("ram:mgXXX.XXX"), sizeof(line));
    (void) strncat(line, " \"", sizeof(line));
    (void) strncat(line, dirname, sizeof(line));
    (void) strncat(line, "\"", sizeof(line));
    Execute(line, 0L, 0L);
    if (ffropen(tmpname) != FIOSUC) {
 	ewprintf("Can't open temporary dir file");
 	return NULL;
    }
    if (ffgetline(line, sizeof(line), &i) != FIOSUC ||
	strncmp(line, "Directory", 9) != 0) {
	ffclose();
	DeleteFile(tmpname);
	ewprintf("No such directory: `%s'", dirname);
    	return NULL;
    }
    line[0] = line[1] = ' ';
    while (ffgetline(&line[2], sizeof(line)-3, &i) == FIOSUC) {
	line[i+2] = '\0';
	(VOID) addline(bp, line);
    }
    ffclose();
    DeleteFile(tmpname);
    bp->b_dotp = lforw(bp->b_linep);		/* go to first line */
    (VOID) strncpy(bp->b_fname, dirname, NFILEN);
    if((bp->b_modes[0] = name_mode("dired")) == NULL) {
	bp->b_modes[0] = &map_table[0];
	ewprintf("Could not find mode dired");
	return NULL;
    }
    bp->b_nmodes = 0;
    return bp;
}

#ifdef	LATTICE
char *
mktemp(pattern)
char *pattern;
{
/* quick hack mktemp for this purpose only */
	register char		*name, *printo ;
	register unsigned short	counter = 0 ;

	if ((name = malloc(strlen(pattern) + 5)) == NULL)
		panic("Manx sucks rocks!") ;
	(VOID) strcpy(name, pattern) ;
	printo = name + strlen(name) ;
	do
		(void) sprintf(printo, "%d", counter += 1) ;
		while (counter > 0 && access(name, 0) == 0) ;
	if (counter == 0) panic("Manx _really_ sucks!") ;
	return name ;
}
#endif

#define LIST_LINE_LENGTH 58			/* Size of line from List */

d_makename(lp, fn)
register LINE *lp;
register char *fn;
{
    register char *cp;
    int n = 2;

    if(llength(lp) < LIST_LINE_LENGTH) return ABORT;
    if(lgetc(lp, 2) == ':') return ABORT;	/* FileNote line	*/
    (VOID) strcpy(fn, curbp->b_fname);
    cp = fn + strlen(fn);
    if ((cp[-1] != ':') && (cp[-1] != '/'))	/* append '/' if needed	*/
	*cp++ = '/';
    while (lgetc(lp, n) != ' ') {
	*cp++ = lgetc(lp, n);
	n++;
    }
    *cp = '\0';
    return strncmp(&lp->l_text[31], "Dir", 3) == 0;
}

static isdirectory(name)
char *name;
{
    struct FileLock *lock;
    struct FileInfoBlock *fib;
    int result;

    if ((lock = Lock(name, ACCESS_READ)) == NULL)
	return FALSE;
    if ((fib = (struct FileInfoBlock *)
	   AllocMem((long)sizeof(struct FileInfoBlock),MEMF_PUBLIC))==NULL) {
	UnLock(lock);
	return FALSE;
    }
    result = (fib->fib_DirEntryType > 0L) ? TRUE : FALSE;
    FreeMem(fib,(long)sizeof(struct FileInfoBlock));
    UnLock(lock);
    return result;
}

#endif

#ifndef USE_ARP

/*
 * Here are work-alikes for the few ARP commands now used by the
 * Amiga version of mg.  These may go away if we require ARP in future.
 */

Strcmp(s1, s2)
register char *s1, *s2;
{
	while (tolower(*s1) == tolower(*s2)) {
		if (*s1 == '\0')
			return 0;
		s1++; s2++;
	}
	return (tolower(*s1) < tolower(*s2) ? -1 : 1);
}

/*
 * This PathName function shamelessly stolen from the Matt Dillon Shell.
 * It is a slight mod of that program's get_pwd routine, from comm1.c.
 */
long
PathName(flock, pwdstr, nentries)
struct Lock *flock;
long nentries;
char *pwdstr;
{

   char *ptr;
   char *name;
   int err=0;

   struct FileLock *lock, *newlock, *ParentDir(), *DupLock();
   long Examine();
   struct FileInfoBlock *fib;
   int i, len, n;

   lock = DupLock(flock);
   n = nentries * FCHARS + 1;
         
   fib = (struct FileInfoBlock *)AllocMem((long)sizeof(struct FileInfoBlock),
   					  MEMF_PUBLIC);
   pwdstr[i = n-1] = '\0';

   while (lock) {
      newlock = ParentDir(lock);
      if (!Examine(lock, fib)) ++err;
      name = fib->fib_FileName;
      if (*name == '\0')	    /* HACK TO FIX RAM: DISK BUG */
	 name = "RAM";
      len = strlen(name);
      if (newlock) {
	 if (i == n-1) {
	    i -= len;
	    movmem(name, pwdstr + i, len);
	 } else {
	    i -= len + 1;
	    movmem(name, pwdstr + i, len);
	    pwdstr[i+len] = '/';
	 }
      } else {
	 i -= len + 1;
	 movmem(name, pwdstr + i, len);
	 pwdstr[i+len] = ':';
      }
      UnLock(lock);
      lock = newlock;
   }
   FreeMem(fib, (long)sizeof(struct FileInfoBlock));
   movmem(pwdstr + i, pwdstr, n - i);
   if (err) return(0L);
   return((long) n - i - 1);
}

void TackOn(path, file)
char *path, *file;
{
	if (*file != '\0') {
		if (path[strlen(path)-1] != ':')
			strcat(path, "/");
		strcat(path, file);
	}
}
#endif
