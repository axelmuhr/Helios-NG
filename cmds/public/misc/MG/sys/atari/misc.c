/* misc.c -- Miscellaneous ST-specific code for MG.
 *
 * author :  Sandra Loosemore
 * date   :  24 Oct 1987
 * changes:  Marion Hakanson -- Jan 1988
 *
 */


#include	"..\..\def.h"

#ifdef MWC
static char *
mwc_cpr="Portions of this program, copyright 1984, Mark Williams Co.";
#endif /* MWC */


/* Exit as quickly as possible.  */

VOID panic (s)
    char* s;
{   Cconws (s);
    Pterm (-1);
    }


#ifdef MWC
extern char **environ;
#else
typedef struct
	{
	char	*p_lowtpa;
	char	*p_hitpa;
	char	*p_tbase;
	long	p_tlen;
	char	*p_dbase;
	long	p_dlen;
	char	*p_bbase;
	long	p_blen;
	char	*p_dta;
	char	*p_parent;
	char	*p_reserved;
	char	*p_env;
	long	p_undefined[20];
	char	p_cmdlin[128];
	}
	BASEPAGE;

extern BASEPAGE *_base;
#endif /* MWC */


/*
 * The environment code here is borrowed from Dale Schumacher.
 */

static char *findenv(var)
	register char *var;
/*
 *	INTERNAL FUNCTION.  This functions attempts to locate <var> in
 *	the environment.  If <var> is not found, NULL is returned.  If
 *	the environment is NULL, NULL is returned.  If <var> is found,
 *	a pointer to the beginning of <var> in the environment is returned.
 *	BOTH MS-DOS AND TOS ENVIRONMENT FORMATS ARE ACCOMODATED.
 */
	{
	register char *p;
	register int len;

#ifdef MWC
	if((p = *environ) == NULL)
#else
	if((p = _base->p_env) == NULL)
#endif /* MWC */
		return(NULL);
	len = strlen(var);
	while(*p)
		{
		if(!strncmp(p, var, len) && (p[len] == '='))
			return(p);
		while(*p++)		/* move to next arg */
			;
		}
	return(NULL);
	}


char *getenv(var)
	register char *var;
/*
 *	Search for <var> in the environment.  If <var> is found, a pointer
 *	to it's value is returned.  NULL is returned if <var> is not found.
 *	WARNING:  The returned pointer points into the environment and
 *	must not be modified!
 */
	{
	register char *p, *q;
	register int len;

	len = strlen(var);
	if(p = findenv(var))
		{
		p += (len + 1);
		if(*p == '\0')		/* TOS env format or empty value */
			{
			q = p+1;
			if(*q == '\0')		/* empty value + end of env */
				return(p);
			while(*q && (*q != '='))
				++q;
			if(*q)			/* empty value */
				return(p);
			else			/* TOS env format */
				return(p+1);
			}
		}
	return(p);
	}


/* Spawn a command.  You can run anything from this; it prompts you for
 *	the command to run.
 * I check for the two most common problems with Pexec:  not finding the
 *    file, and not having enough memory.  Otherwise, I assume bad status
 *    codes were the fault of the program, not Pexec, and that the program
 *    would have told the user what is wrong.
 */

spawncli(f, n)
{   register int s;
    char fname[NFILEN];
    char tail[NFILEN];
    char *shell = getenv("SHELL");
    extern VOID splitcmd();
   
    if (shell && *shell) {
        (VOID) strcpy(fname, shell);
        goto tryit;
	}
    else
        shell = NULL;
	
askfor:
    if ((s = ereply("Run program: ", fname, NFILEN)) != TRUE)
        return (s);
tryit:
    ttcolor (CTEXT);
    ttmove (nrow-1, 0);
    tteeol ();
    splitcmd (fname, tail);
    ttclose ();
    s = Pexec(0, fname, tail, 0L);
    ttopen ();
    if (s == -33) {
        if (shell) {
	    shell = NULL;
            ewprintf ("Could not find shell.");
            sleep(1);
            goto askfor;
	    }
        ewprintf ("Could not find file.");
        return (FALSE);
	}
    else if (s == -39) {
        if (shell) {
	    shell = NULL;
            ewprintf ("Not enough memory to run shell!");
            sleep(1);
            goto askfor;
	    }
        ewprintf ("Not enough memory to run this program!");
        return (FALSE);
	}
    else {
        sgarbf = TRUE;          /* Force repaint */
        Cconws ("Hit any key to return to MG:");
        (VOID) Crawcin ();
        return (TRUE);
        }
}


/* Pexec wants the command tail to have a byte count as the first
 *    character.  This function separates the program name from the command
 *    tail and adds the byte count.
 */

static VOID splitcmd (cmd, tail)
    char* cmd;
    char* tail;
{   int i, j;
    i = 0;
    j = 0;
    while (cmd[i] != '\0') {
        if (j != 0)  { 
	    tail[j] = cmd[i]; 
	    j++;
            }
        else if (cmd[i] == ' ')  {
            cmd[i] = '\0';
            tail[1] = ' ';
            j = 2;
	    }
        i++;
        }
    if (j == 0)  {
        tail[0] = (char) 1;
        tail[1] = ' ';
        tail[2] = '\0';
        }
    else {
        tail[j+1] = '\0';
        tail[0] = (char) (j-1);
        }
    }




/*
 * copy 'count' bytes from 'src' to 'dst'. (optimized)
 */
VOID bcopy(src, dst, count)
register char *src, *dst;
register count;
{
    register c1;

    /*
    ** The magic number 8 comes from:
    **   1   max. bytes before a word boundary
    **   4   min. bytes in a block of long-words
    **   3   max. bytes after the last long-word in the block
    */
    if (count >= 8) {
        /*
        ** Advance dst pointer to word boundary.
        */
        if (1 & (short)dst) {
	        *dst++ = *src++;
                count--;
	}
        /*
        ** If src pointer is (also) aligned, use long-word copy.
        */
        if ((1 & (short)src) == 0) {
            c1 = ((unsigned)count >> 2);
            while (--c1 >= 0)
                *((long *)dst)++ = *((long *)src)++;
            count &= 3;		/* There are 0,1,2, or 3 bytes left */
        }
    }
    /*
    ** Copy whatever is left.
    */
    while (--count >= 0)
	*dst++ = *src++;
}

/* Busy waiting to fake out sleep.  Thanks to Rich Sansom for the routine
 *    to poll the system clock.
 */

static long *hz_200 = (long *)0x000004ba;	/* system 200 hz clock		*/

#ifdef NO_DPROMPT
static long read200hz()
#else
long read200hz()
#endif /* NO_DPROMPT */
{   return(*hz_200);
    }


/* Sleep (busily) for n seconds */

sleep (n)
    int n;
{   register long waitfor = (n * 200) + Supexec(read200hz);

    while (Supexec(read200hz) < waitfor);
    }

