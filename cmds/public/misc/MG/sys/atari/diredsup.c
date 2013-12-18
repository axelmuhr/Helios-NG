/* diredsup.c -- Atari ST functions for handling DIR and DIRED features
 *
 * author :  Sandra Loosemore
 * date   :  20 Dec 1987
 *
 */

#include "..\..\def.h"


/* ST-specific code to support the DIR features in dir.c
 */

#ifndef NO_DIR

char *getwd (buffer)
    char *buffer;
{   int drive, i;
    drive = Dgetdrv();
    buffer[0] = (char)drive + 'a';
    buffer[1] = ':';
    Dgetpath (&buffer[2], drive+1);
    for (i=2; buffer[i] != '\0'; i++)
        buffer[i] = TOLOWER(buffer[i]);
    return(buffer);
    }

int chdir (buffer)
    char *buffer;
{   int drive;
    if (buffer[1] == ':')  {
        drive = TOLOWER(buffer[0]) - 'a';
	(VOID) Dsetdrv (drive);
	buffer = buffer + 2;
        }
    return ((int)Dsetpath (buffer));
    }

#endif


/* ST-specific code to support the DIRED features in dired.c.
 */

#ifndef NO_DIRED

#include "..\..\kbd.h"


/* Various file manipulation functions.  */

int rename (fromname, toname)
    char *fromname, *toname;
{   if (Frename(0, fromname, toname) == 0)
        return(0);
    else  {
        ewprintf ("Rename failed.");
        return(-1);
	}
    }

int copy (fromname, toname)
    char *fromname, *toname;
{   int from, to, count;
    char buffer[256];
    if ((from = Fopen (fromname, 0)) < 0)  {
        ewprintf ("Could not open input file %s.", fromname);
        return(-1);
        }
    (VOID) Fdelete (toname);
    if ((to = (Fcreate (toname, 0))) < 0)  {
        ewprintf ("Could not open output file %s.", toname);
        Fclose(from);
	return(-1);
        }
    while ((count = Fread(from, (long)256, buffer)) > 0)
        (VOID) Fwrite (to, (long)count, buffer);
    (VOID) Fclose(from);
    (VOID) Fclose(to);
    return(0);
    }

int unlink (fname)
    char *fname;
{   if (Fdelete(fname) == 0)
        return(0);
    else
        return(-1);
    }

int unlinkdir (fname)
    char *fname;
{   if (Ddelete(fname) == 0)
        return(0);
    else
        return(-1);
    }


/* Create a dired buffer for the given directory name.  */

BUFFER *dired_(dirname)
char *dirname;
{
    register BUFFER *bp;
    BUFFER *findbuffer();

	/* Create the dired buffer */

    if ((dirname = adjustname(dirname)) == NULL) {
	ewprintf("Bad directory name");
	return NULL;
        }
    if ((bp = findbuffer(dirname)) == NULL) {
	ewprintf("Could not create buffer");
	return NULL;
        }
    if (bclear(bp) != TRUE) return FALSE;

	/* Now fill it in.  */

    if (!dirlist (bp, dirname))  {
    	ewprintf("Could not read directory");
        return FALSE;
        }

	/* Clean up and return */

    bp->b_dotp = lforw(bp->b_linep);		/* go to first line */
    (VOID) strncpy(bp->b_fname, dirname, NFILEN);
    if((bp->b_modes[0] = name_mode("dired")) == NULL) {
	bp->b_modes[0] = &map_table[0];
	ewprintf("Could not find mode dired");
	return NULL;
        }
    return bp;
    }


/* Take the name from the line in the buffer and make a filename out of it.
 */

d_makename(lp, fn)
register LINE *lp;
register char *fn;
{
    register char *cp;
    register char ch;
    int i;

    if(llength(lp) < 55) return ABORT;
    (VOID) strcpy (fn, curbp->b_fname);
    cp = fn + strlen(fn);
    *cp++ = '\\';
    i = 2;
    while ((ch = lgetc(lp,i)) != ' ')  {
        *cp++ = ch;
	i++;
	}
    *cp = '\0';
    return lgetc(lp, 52) == 'd';
}


/* Here is all the messy code for getting a directory listing.
 *    It is printed out like
 *
 * 0         1         2         3         4         5
 * 01234567890123456789012345678901234567890123456789012345
 *   name----------><-----size  dd-mmm-yyyy  hh:mm:ss  drw
 *
 */

typedef struct dta {
    char junk[21];
    char attrib;
    unsigned int timestamp;
    unsigned int datestamp;
    long filesize;
    char name[14];
    } DTA;

static char* months[] = {
    "???", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", 
    "Aug", "Sep", "Oct", "Nov", "Dec"};


/* Print a number into the character buffer, bumping up the pointer */

static char* printit (str, number, width, fill, zero)
    char* str;
    long number;
    int width;
    char fill;
{   int i;
    if (number == 0)   {
        for (i=1; i<width; i++)
	    *str++ = fill;
        if (width == 0)
            ;
        else if (zero) 
	    *str++ = '0';
        else
	    *str++ = fill;
        return (str);
        }
    else  {
        str = printit (str, (number/10), width-1, fill, FALSE);
        *str++ = (char) ((number%10) + (int) '0');
        return (str);
        }
    }


/* Print a null-terminated string into the buffer */

static char* copyit (to, from, width, fill)
    char *from, *to;
    int width;
    char fill;
{   int i;
    char ch;
    i = 0;
    while (*from != '\0')  {
        ch = *from++;
        *to++ = TOLOWER(ch);
        i++;
	}
    while (i < width)  {
        *to++ = fill;
        i++;
        }
    return (to);
    }


static char* getdate (datestamp, buffer)
    unsigned int datestamp;
    char* buffer;
{   int date, month, year;

    date = datestamp & 31;
    month = (datestamp >> 5) & 15;
    year = (datestamp >> 9) + 1980;
    *buffer++ = ' ';
    *buffer++ = ' ';
    buffer = printit (buffer, (long) date, 2, '0', TRUE);
    *buffer++ = '-';
    buffer = copyit (buffer, months[month], 3, ' ');
    *buffer++ = '-';
    buffer = printit (buffer, (long) year, 4, ' ', TRUE);
    return (buffer);
    }


static char* gettime (timestamp, buffer)
    unsigned int timestamp;
    char* buffer;
{   int second, minute, hour;

    second = (timestamp & 31) * 2;
    minute = (timestamp >> 5) & 63;
    hour = (timestamp >> 11);
    *buffer++ = ' ';
    *buffer++ = ' ';
    buffer = printit (buffer, (long) hour, 2, '0', TRUE);
    *buffer++ = ':';
    buffer = printit (buffer, (long) minute, 2, '0', TRUE);
    *buffer++ = ':';
    buffer = printit (buffer, (long) second, 2, '0', TRUE);
    return (buffer);
    }


static dirlist (bp, dirname)
    BUFFER *bp;
    char *dirname;
{   
    char fname[NFILEN], buf[NFILEN];
    int status;
    DTA my_dta, *old_dta;
    char *bufptr;

    (VOID) strcpy (fname, dirname);
    (VOID) strcat (fname, "\\*.*");
    old_dta = (DTA *) Fgetdta ();
    Fsetdta (&my_dta);
    status = Fsfirst (fname, 0x11);
    while (status == 0)  {
        bufptr = buf;
	*bufptr++ = ' ';
	*bufptr++ = ' ';
        bufptr = copyit (bufptr, my_dta.name, 15, ' ');
        bufptr = printit (bufptr, my_dta.filesize, 10, ' ', TRUE);
        bufptr = getdate (my_dta.datestamp, bufptr);
        bufptr = gettime (my_dta.timestamp, bufptr);
	*bufptr++ = ' ';
	*bufptr++ = ' ';
        *bufptr++ = (my_dta.attrib) ? 'd' : '-';  /* directory */
	*bufptr++ = 'r';
        *bufptr++ = (my_dta.attrib) ? '-' : 'w';  /* read-only */
        *bufptr = '\0';
        if (addline(bp, buf) == FALSE)  {
            Fsetdta (old_dta);
            return (FALSE);
            }
        status = Fsnext ();
        }
    Fsetdta (old_dta);
    return (TRUE);
    }

#endif
