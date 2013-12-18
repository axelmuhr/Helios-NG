/* fileio.c -- Atari ST file system interface for MG
 *
 * author :  Sandra Loosemore (from an earlier version by dec-rex!conroy)
 * date   :  24 Oct 1987
 * changes:  Marion Hakanson -- Jan 1988
 *
 */
#include	"..\..\def.h"


/* Adjustname regularizes a filename.  This fills in the current drive
 *	and directory onto the front if they are not already provided, and
 *	also lowercases it so a case-sensitive string compare can be used
 *	on filenames.
 * This doesn't do the right things with collapsing "..\" and ".\" -- it
 *      doesn't check for them being embedded in pathnames, only at the
 *	front.  Sigh.
 */

static char adbuffer[128];

char* adjustname (fname)
    char *fname;
{   int i, j, k;
    char pathbuf[128];

    if (fname[1] == ':')  {
        j = 2;
        adbuffer[0] = TOLOWER(fname[0]);
	}
    else  {
        adbuffer[0] = (char)Dgetdrv() + 'a';
        j = 0;
        }
    adbuffer[1] = ':';
    i = 2;
    if (fname[j] != '\\')  {
        Dgetpath(pathbuf, (int)(adbuffer[0] - 'a' + 1));
        for (k=0; pathbuf[k] != '\0'; k++)  {
	    adbuffer[i] = TOLOWER(pathbuf[k]);
	    i++;
	    }
        adbuffer[i] = '\\';
	i++;
        }
    if (fname[j] == '.')  {
        if (fname[j+1] == '.')  {
            i = i - 2;
            while (adbuffer[i] != '\\')  i--;
            j = j + 2;
            }
        else  {
            j = j + 1;
            i = i - 1;
	    }
        }
    while (fname[j] != '\0')  {
        adbuffer[i] = TOLOWER(fname[j]);
        i++;
        j++;
        }
    adbuffer[i] = '\0';
    return(adbuffer);
    }


/* Define some things used later on */

static int handle;
#define RBUFSIZE 512
static char rbuffer[RBUFSIZE];
static int rbufmax, rbufnext, rbufcr;
static int rbufeof;


/* Open and close files.  Store the file handle in "handle".  We'll use 
 *	it later in the routines that read and write to the file.
 */


ffropen (fn)
    char *fn;
{   handle = Fopen(fn, 0);
    if (handle < 0)
        return(FIOFNF);
    else  {
        rbufmax = 0;
        rbufnext = 0;
	rbufcr = FALSE;
	rbufeof = FALSE;
        return(FIOSUC);
	}
    }


ffwopen (fn)
    char *fn;
{   (VOID) Fdelete(fn);
    handle = Fcreate(fn, 0);
    if (handle < 0)  {
	ewprintf("Open of %s failed with error code %d.", fn, handle);
	return (FIOERR);
	}
    else
	return (FIOSUC);
    }
 
ffclose ()
{   (VOID) Fclose(handle);
    return (FIOSUC);
    }


/* Write an entire buffer to the already open file.  The last line of 
 * 	the buffer does not have an implied newline.
 */

static char *crlf = "\r\n";

ffputbuf (buf)
    BUFFER *buf;
{   LINE *line, *last;

    last = buf->b_linep;       /* Header line is empty */
    line = last->l_fp;         /* The first real line */

    while (line->l_fp != last)  {
        if ((Fwrite (handle, (long)line->l_used, line->l_text) < 0) ||
	    (Fwrite (handle, 2l, crlf) < 0))  {
		ewprintf("File write error");
		return (FIOERR);
                }
        line = line->l_fp;
        }
    if (line->l_used > 0)  {
        if (Fwrite (handle, (long)line->l_used, line->l_text) < 0)  {
		ewprintf("File write error");
		return (FIOERR);
                }
        }
    return(FIOSUC);
    }


/* Read a line from the file up to nbuf long.  Set *nbytes to the actual
 *    count read (excluding the end-of-line marker).  Returns:
 *        FIOSUC if all is well.
 *        FIOLONG if no newline was found yet.
 *        FIOEOF on end-of-file.
 *        FIOERR if any random errors happened.
 * The GemDos routine Fread does not break on end-of-line; it will read
 *    however many characters you tell it to.  So we do a little buffering.
 *    Remember that crlf is the line terminator.  Cr's and lf's that appear
 *    by themselves are passed on.
 */


ffgetline (buf, nbuf, nbytes)
    char buf[];
    int nbuf;
    int *nbytes;
{   register int i;
    register char ch;

    for (i=0; i<nbuf; i++)  {
        if (rbufmax == rbufnext)  {
	    if (rbufeof || 
	          ((rbufmax = Fread(handle, (long)RBUFSIZE, rbuffer)) == 0))  {
                (*nbytes) = i;
                return (FIOEOF);
                }
            else if (rbufmax < 0)  {
 	        ewprintf("File read error");
	        return (FIOERR);
                }
	    else  {
		if (rbufmax < RBUFSIZE)  rbufeof = TRUE;
                rbufnext = 0;
		}
            }
        ch = rbuffer[rbufnext];
        rbufnext++;
        if (rbufcr && (ch == '\n'))  {
	    rbufcr = FALSE;
	    (*nbytes) = i-1;
            return (FIOSUC);
            }
        else  {
	    buf[i] = ch;
	    rbufcr = (ch == '\r');
            }
	}
    return (FIOLONG);
    }


#ifndef NO_BACKUP
 
/*
 * Finish this routine when you decide
 * what the right thing to do when renaming a
 * file for backup purposes.
 */

fbackupfile(fname)
char	*fname;
{
	return (TRUE);
}

#endif



#ifndef NO_STARTUP

/* Look for a startup file as MG.INI in the current directory, then for
 *    the file specified by the environment variable MGINIT.
 */

char* startupfile (ignore)
    char *ignore;
{   int handle;
    char *foo;
    extern char *getenv();

    if ((handle = Fopen ("MG.INI", 0)) >= 0)  {
        (VOID) Fclose (handle);
        return("MG.INI");
        }
    else if (((foo = getenv("MGINIT")) != NULL) &&
             ((handle = Fopen (foo, 0)) >= 0))  {
        (VOID) Fclose (handle);
        return(foo);
        }
    else
        return(NULL);
    }

#endif

