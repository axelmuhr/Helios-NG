/******************************************************************************
**
**  FILE:       heliosio.c
**  VERSION:    1.0
**  PURPOSE:    adaption of MicroEMACS 3.11 to HELIOS
**
**  AUTHOR:     Dieter Stokar (ds)
**  DATE:       Dec. 1991
**  INSTITUTE:  Electronics Lab, ETH, CH-8092 ZUERICH
**  COMPILER:   Helios C V2.01
**  HISTORY:    based on ansi.c and unix.c of the MicroEMACS distribution disk
***
*** BLV - extensive changes to incorporate the folding editor support
******************************************************************************/

#ifdef __HELIOS
#define termdef 1                       /* don't define term external */
#define TIMEOUT         255             /* No character available       */

#include        <stdio.h>
#include        <helios.h>
#ifdef FORWARD
#undef FORWARD
#endif
#include        "estruct.h"
#include        "eproto.h"
#include        "edef.h"
#include        "elang.h"

#define         _BSD

#include        <syslib.h>
#include        <attrib.h>
#include        <nonansi.h>
#include        <time.h>
#include	<gsp.h>
#include        <sys/types.h>                   /* System type definitions      */
#include        <sys/dir.h>                     /* Directory entry definitions  */
#include        <sys/stat.h>                    /* File status definitions      */
#include        <sys/ioctl.h>                   /* I/O control definitions      */

static  Attributes oldConState, newConState;	/* state of IO-server		*/
#if TYPEAH
int     unGotChar=0;
#endif

void PASCAL NEAR ttputs(char *string);

#define NROW    50                      /* max Screen size.             */
#define NCOL    132                     /*   (edit if you want to)      */
#define NPAUSE  100                     /* # times thru update to pause */
#define MARGIN  8                       /* size of minimim margin and   */
#define SCRSIZ  64                      /* scroll size for extended lines */
#define BEL     0x07                    /* BEL character.               */

/* Forward references.          */
extern int PASCAL NEAR heliosmove();
extern int PASCAL NEAR helioseeol();
extern int PASCAL NEAR helioseeop();
extern int PASCAL NEAR heliosbeep();
extern int PASCAL NEAR heliosopen();
extern int PASCAL NEAR heliosrev();
extern int PASCAL NEAR heliosclose();
extern int PASCAL NEAR helioskopen();
extern int PASCAL NEAR helioskclose();
extern int PASCAL NEAR helioscres();
extern int PASCAL NEAR heliosparm();
extern int PASCAL NEAR heliosgetc();

#if     COLOR			/* still hoping that HELIOS will get colors someday...*/
extern int PASCAL NEAR heliosfcol();
extern int PASCAL NEAR heliosbcol();

static int cfcolor = -1;        /* current forground color */
static int cbcolor = -1;        /* current background color */
#endif /* COLOR */

/*
 * Standard terminal interface dispatch table. 
 */
NOSHARE TERM term    = {
        NROW-1,
        24,
        NCOL-1,
        79,
        0, 0,
        MARGIN,
        SCRSIZ,
        NPAUSE,
        heliosopen,
        heliosclose,
        helioskopen,
        helioskclose,
        heliosgetc,
        ttputc,
        ttflush,
        heliosmove,
        helioseeol,
        helioseeop,
        helioseeop,
        heliosbeep,
        heliosrev,
        helioscres
#if     COLOR
        , heliosfcol,
        heliosbcol
#endif /* COLOR */
};

#if     COLOR
PASCAL NEAR heliosfcol(color)             /* set the current output color */

int color;      /* color to set */

{
        if (color == cfcolor)
                return;
        putchar(0x1B); putchar('[');
        heliosparm(color+30);
        putchar('m');
        cfcolor = color;
}

PASCAL NEAR heliosbcol(int color)             /* set the current background color */
{
        if (color == cbcolor)
                return;
        putchar(0x1B); putchar('[');
        heliosparm(color+40);
        putchar('m');
        cbcolor = color;
}
#endif /* COLOR */

PASCAL NEAR heliosmove(int row, int col)
{
        putchar(0x1b); putchar('[');
        heliosparm(row+1);
        putchar(';');
        heliosparm(col+1);
        putchar('H');
}

PASCAL NEAR helioseeol(void)
{
        putchar(0x1B); putchar('[');
        putchar('K');
}

PASCAL NEAR helioseeop(void)
{
#if     COLOR
        heliosfcol(gfcolor);
        heliosbcol(gbcolor);
#endif /* COLOR */
        heliosmove(0,0);
        putchar(0x1B); putchar('[');
        putchar('2');
        putchar('J');
}

PASCAL NEAR heliosrev(int state)              /* change reverse video state */
/* TRUE = reverse, FALSE = normal */
{
#if     COLOR
        int ftmp, btmp;         /* temporaries for colors */
#endif /* COLOR */
        fflush(stdout);         /* because of bug in winsrvr */
        putchar(0x1B); putchar('[');
        putchar(state ? '7': '0');
        putchar('m');
        
#if     COLOR
        if (state == FALSE) {
                ftmp = cfcolor;
                btmp = cbcolor;
                cfcolor = -1;
                cbcolor = -1;
                heliosfcol(ftmp);
                heliosbcol(btmp);
        }
#endif /* COLOR */
}

PASCAL NEAR helioscres(void)  /* change screen resolution */

{
        return(TRUE);
}

PASCAL NEAR spal(char *dummy)           /* change pallette settings */
{
        /* none for now */
	*dummy = *dummy;
}

PASCAL NEAR heliosbeep()
{
        putchar(BEL);
        ttflush();
}

PASCAL NEAR heliosparm(int n)
{
        register int q;

        q = n/10;
        if (q != 0)
	  {
	    register int r = q/10;
	    
	    if (r != 0)
	      {
		putchar((r%10)+'0');
	      }
	    putchar((q%10) + '0');
	  }
	n %= 10;
        putchar( n + '0');
} /* putchar */

PASCAL NEAR heliosopen(void)
{
        Attributes conAttr;
        GetAttributes(Heliosno(stdin),&conAttr);
        term.t_nrow = conAttr.Min-1;
        term.t_ncol = conAttr.Time-1;
        
        strcpy(sres, "NORMAL");
        revexist = TRUE;
        return (0);
}

PASCAL NEAR heliosclose(void)
{
#if     COLOR
        heliosfcol(7);
        heliosbcol(0);
#endif /* COLOR */
        return (0);
} /* heliosclose */

/*---------------------------------------------------------------------------*/
PASCAL NEAR helioskopen(void) /* open the keyboard */
{
    GetAttributes(Heliosno(stdin),&oldConState); /* save to restore in ansikclose() */ 
    newConState = oldConState;
    AddAttribute(&newConState,ConsoleRawInput);
    AddAttribute(&newConState, ConsoleRawOutput);
    RemoveAttribute(&newConState,ConsolePause);
    RemoveAttribute(&newConState,ConsoleIgnoreBreak);
    RemoveAttribute(&newConState,ConsoleBreakInterrupt);
    RemoveAttribute(&newConState,ConsoleEcho);
    SetAttributes(Heliosno(stdin),&newConState);
    return (0);
} /* helioskopen */

/*---------------------------------------------------------------------------*/
PASCAL NEAR helioskclose(void)        /* close the keyboard */
{   SetAttributes(Heliosno(stdin), &oldConState);
    return (0);
} /* helioskclose */

/*---------------------------------------------------------------------------*/
/***
 *  ttputs  -  Send a string to ttputc
 *
 *  Nothing returned
 ***/
void PASCAL NEAR ttputs(char *str)
{
        if (str != NULL) printf("%s",str);
} /* ttputs */


static unsigned char inbuffer[ 10];
static int inpos=0;
static int inlen=0;

/*---------------------------------------------------------------------------*/
void HandleScreenSize(void)
/*
**	when using the winsrvr (Helios IO-server for Windows 3.0) the screen
**	size may change anytime. Here we just check, if the size has changed
**	and try to adapt to it. Works fine for change of width. After a
**	change of hight the command '^x0' may have to be used in order to
**	clean up the screen and emacs-windows within the screen.
*/
{
    int updateScreen = FALSE;
    
    Attributes conAttr;
    GetAttributes(Heliosno(stdin),&conAttr);
    if (term.t_nrow != conAttr.Min-1)
    {							/* hight has changed */
        if (term.t_mrow > conAttr.Min-1) term.t_nrow = conAttr.Min-1;
        else term.t_nrow = term.t_mrow;
        newsize(TRUE, conAttr.Min);
        updateScreen = TRUE;
    }
    
    if (term.t_ncol != conAttr.Time-1)
    {							/* width has changed */
        if (term.t_mcol > conAttr.Time-1) term.t_ncol = conAttr.Time-1;
        else term.t_ncol = term.t_mcol;
        newwidth(TRUE, term.t_ncol);
        updateScreen = TRUE;
    }
    if (updateScreen) upscreen(TRUE, 0);
        
} /* HandleScreenSize */


/*---------------------------------------------------------------------------*/
/*
 *      Read a keystroke from the terminal.  Interpret escape sequences
 *      that come from function keys, mouse reports, and cursor location
 *      reports (not yet), and return them using Emacs's coding of these events.
 */
PASCAL NEAR heliosgetc(void)
{
    int ch, ch2, ch3, ch4;
    
    HandleScreenSize();
    for(;;)
    {                           /* Until we get a character to return */
        if( inpos < inlen)
        { /* Working out a multi-byte input sequence */
            return( inbuffer[ inpos++]);
        }
        inpos = 0;
        inlen = 0;
        ch = ttgetc();

        if( ch == 0x09B)
        {                               /* CSI, see if sequence follows */
            ch2 = ttgetc_shortwait();
            if ((ch2 & 0xf0) == 0x40)
            {                                   /* most keys on the keypad */
                switch (ch2)
                {
                case 0x48 : return (SPEC | '<');                /* home */
                case 0x41 : return (SPEC | 'P');                /* up */
                case 0x44 : return (SPEC | 'B');                /* left */
                case 0x43 : return (SPEC | 'F');                /* right */
                case 0x42 : return (SPEC | 'N');                /* down */
                case 0x40 : return (SPEC | 'C');                /* ins */
                }
            }
            else
            { 
                ch3=ttgetc_shortwait();
                if (ch3 == 0x7e) 
                {   if (ch2 == '?') return(SPEC | 'H');		 /* help */
                    elif (ch2 == 0x39) return (SPEC | '0');       /* F10 */
                    else             return (SPEC | (ch2+1));    /* Fn */
                }
                else if (ch3 == 0x7a)
                {
                    switch (ch2)
                    {
                    case 0x33: return (SPEC | 'Z');             /* pgup */
                    case 0x32: return (SPEC | '>');             /* end */
                    case 0x34: return (SPEC | 'V');             /* pgdn */
                    }               
                }
                elif ((0x31 <= ch2) && (ch2 <= 0x33))
                {   int key;
                    ch4=ttgetc_shortwait();	/* ~ terminator */
		    if ((ch3 & 0x3F) == 0x39)   /* F10 */
			key = '0';
		    else
			key = ch3 + 1;		/* FN1 to FN 9 */
		    if (ch2 == 0x31)		/* Shifted function key */
			return(SPEC | SHFT | key);
		    elif (ch2 == 0x32)		/* Alt function key */
			return(SPEC | ALTD | key);
		    else
			return(SPEC | SHFT | ALTD | key);
                }
                else
                { /* This isn't an escape sequence, return it unmodified */
                    inbuffer[ inlen++] = ch2;
                    inbuffer[ inlen++] = ch3;
                    return( 0x09B );
                }
            }
        }
        else if (ch == 127) return (SPEC | 'D');                /* del */
	elif (ch & 0x0080)
         return(ALTD|toupper(ch & 0x007F));
        else return (ch);                                       /* any other char */
    }
} /* heliosgetc */

/*--------------------------------------------------------------------*/
PASCAL NEAR ttputc(int ch)
{
    putchar(ch);
}

/*--------------------------------------------------------------------*/
PASCAL NEAR ttflush(void)
{
        fflush(stdout);
}

/*--------------------------------------------------------------------*/
int PASCAL NEAR ttgetc(void)
{
    char cc;
#if TYPEAH    
    if (unGotChar == 0)
    {
#endif    
        Read(Heliosno(stdin),&cc,1,-1);
#if TYPEAH        
    }
    else
    {
        cc = unGotChar;
        unGotChar = 0;
    }
#endif    
    return cc;
} /* ttgetc */

/*--------------------------------------------------------------------*/
int PASCAL NEAR ttgetc_shortwait(void)
{
    char cc;
#if TYPEAH    
    if (unGotChar == 0)
    {
#endif    
    if (!Read(Heliosno(stdin),&cc,1,1)) cc = TIMEOUT;
#if TYPEAH        
    }
    else
    {
        cc = unGotChar;
        unGotChar = 0;
    }
#endif    
    return cc;
}

#if  TYPEAH
/*--------------------------------------------------------------------*/
int PASCAL NEAR typahead(void)
{
    char cc;
    if (!Read(Heliosno(stdin),&cc,1,1)) cc = 0;
    unGotChar = cc;
    return cc;
} /* typahead */
#endif
        
/*--------------------------------------------------------------------*/
/** Get time of day **/
char * timeset(void)
{
#if 0
        long int buf; /* Should be time_t */
#else
	time_t buf;
#endif
        char * sp, * cp;

        char * ctime();

        /* Get system time */
        time(&buf);

        /* Pass system time to converter */
        sp = ctime(&buf);

        /* Eat newline character */
        for (cp = sp; *cp; cp++)
                if (*cp == '\n') {
                        *cp = '\0';
                        break;
                }
        return(sp);
}


/*--------------------------------------------------------------------*/
/** Get next filename from pattern **/

extern int MatchingType;	/* CMP_DIRECTORY or CMP_FILENAME */

static Stream *current_directory = NULL;
static char path[NFILEN];

char *getnfile(void)
{ DirEntry	entry;

	/* At this point current_directory should be an open stream	*/
	/* to the directory being searched. MatchingType can be one of	*/
	/* CMP_DIRECTORY or CMP_FILENAME, depending on what is being	*/
	/* searched for. The reply should be held in the static		*/
	/* path.							*/


	while (Read(current_directory, (BYTE *) &entry, sizeof(DirEntry), -1) == sizeof(DirEntry))
	 { if (!strcmp(entry.Name, ".") || !strcmp(entry.Name, ".."))
	    continue;
	   if ((MatchingType == CMP_DIRECTORY) && ((entry.Type & Type_Flags) != Type_Directory))
	    continue;
	   if (((entry.Type & Type_Flags) != Type_Stream) &&
	       ((entry.Type & Type_Flags) != Type_Directory))
	    continue;

	   strcpy(path, entry.Name);
	   if ((entry.Type & Type_Flags) == Type_Directory)
	    strcat(path, "/");
	   return(path);
	 }
		/* Reset for the next operation	*/
	MatchingType = CMP_FILENAME;
	Close(current_directory);
	current_directory = NULL;
	return(NULL);
}


/*--------------------------------------------------------------------*/
/** Get first filename from pattern **/
/**
*** fspec can currently be absolute or relative. If relative it is
*** updated to absolute... Then it will always be possible to find the
*** matching directory open it.
**/

char *getffile(char *fspec)
{ char	*tmp;

	if (*fspec != '/')
	{ Object *directory;
	  tmp = fspec + strlen(fspec);
	  while ((tmp >= fspec) && (*tmp != '/')) tmp--;
	  if (tmp < fspec)  /* No slash anywhere */
	   directory = Locate(cdobj(), NULL);
	  else
	   { *tmp = '\0';
	     directory = Locate(cdobj(), fspec);
	     *tmp = '/';
	   }
	  if (directory == NULL)
	   return(NULL);
	  if ((directory->Type & Type_Flags) != Type_Directory)
	   { Close(directory); return(NULL); }
	  strcpy(path, directory->Name);
	  strcat(path, "/");
	  strcat(path, ++tmp);
	  strcpy(fspec, path);
	  Close(directory);
	}

	/* At this point fspec contains an absolute pathname which may	*/
	/* or may not be valid. It is necessary to find the directory	*/
	/* by tracking back looking for a slash. This directory can	*/
	/* be opened. getnfile() is responsible for reading it.		*/

	if (current_directory != NULL) Close(current_directory);
	tmp = fspec + strlen(fspec);
	while (*tmp != '/') tmp--;
	*tmp = '\0';
	current_directory = Open(cdobj(), fspec, O_ReadOnly);
	*tmp = '/';
	if (current_directory == NULL)
	 return(NULL);

	/* Update the fspec, to cope with .. etc. */
	strcpy(path, current_directory->Name);
	strcat(path, tmp);
	strcpy(fspec, path);
	if ((current_directory->Type & Type_Flags) != Type_Directory)
	 { Close(current_directory); current_directory = NULL; return(NULL); }

	return(getnfile());	 	
}

/*--------------------------------------------------------------------*/
/** Create subshell **/
int spawncli(int f, int n)
{ int stat;

  /* Don't allow this command if restricted */
  if (restflag)
   return(resterr());

        /* Close down */
  heliosmove(term.t_nrow, 0);
  ttflush();
  helioskclose(); 

  if (vfork() == 0)
   { int fd, maxfd = getdtablesize();
     for (fd = 4; fd < maxfd; fd++)
      close(fd);
     execl("/helios/bin/shell", "shell", NULL);
     _exit(20);
   }
  wait(&stat);

        /* Restart system */
  sgarbf = TRUE;
  if (helioskopen())
   { puts("** Error reopening terminal device **");
     exit(1);
   }

  return 0;
  
  f = n;
}

/*--------------------------------------------------------------------*/
/** Spawn a command **/
int spawn(int f, int n)
{
        char line[NLINE];
        int s;

        /* Don't allow this command if restricted */
        if (restflag)
                return(resterr());

        /* Get command line */
        s = mlreply("!", line, NLINE);
        if (!s)
                return(s);

        /* Perform the command */
        s = callout(line);

        /* if we are interactive, pause here */
        if (clexec == FALSE) {
                mlwrite("[End]");
                ttflush();
                ttgetc();
        }
        return(s);
}

/*--------------------------------------------------------------------*/
/** Execute program **/
/* BLV - optimise ! */
int execprg(int f, int n)
{
        /* Same as spawn */
        return(spawn(f, n));
}

/*--------------------------------------------------------------------*/
/** Callout to system to perform command **/
/* BLV - fiddle with attributes */
int callout(char *cmd)
{
        int status;

        /* Close down */
        heliosmove(term.t_nrow, 0);
        ttflush();
        helioskclose(); 

        /* Do command */
        status = system(cmd) == 0;

        /* Restart system */
        sgarbf = TRUE;
        if (helioskopen()) {
                puts("** Error reopening terminal device **");
                exit(1);
        }

        /* Success */
        return(status);
}

/*--------------------------------------------------------------------*/
/** Pipe output of program to buffer **/
int pipecmd(int f, int n)
{
        char line[NLINE];
        int s;
        BUFFER * bp;
        WINDOW * wp;
        static char filnam[] = "command";

        /* Don't allow this command if restricted */
        if (restflag)
                return(resterr());

        /* Get pipe-in command */
        s = mlreply("@", line, NLINE);
        if (!s)
                return(s);

        /* Get rid of the command output buffer if it exists */
        bp = bfind(filnam, FALSE, 0);
        if (bp) {
                /* Try to make sure we are off screen */
                wp = wheadp;
                while (wp) {
                        if (wp->w_bufp == bp) {
                                onlywind(FALSE, 1);
                                break;
                        }
                        wp = wp->w_wndp;
                }
                if (!zotbuf(bp))
                        return(0);
        }

        /* Add output specification */
        strcat(line, ">");
        strcat(line, filnam);

        /* Do command */
        s = callout(line);
        if (!s)
                return(s);

        /* Split the current window to make room for the command output */
        if (!splitwind(FALSE, 1))
                return(0);

        /* ...and read the stuff in */
        if (!getfile(filnam, FALSE))
                return(0);

        /* Make this window in VIEW mode, update all mode lines */
        curwp->w_bufp->b_mode |= MDVIEW;
        wp = wheadp;
        while (wp) {
                wp->w_flag |= WFMODE;
                wp = wp->w_wndp;
        }

        /* ...and get rid of the temporary file */
        unlink(filnam);
        return(1);
}

/*--------------------------------------------------------------------*/
/** Filter buffer through command **/
int filter(
int f,                                  /* Flags                        */
int n )                                  /* Argument count               */
{
        char line[NLINE], tmpnam[NFILEN];
        int s;
        BUFFER * bp;
        static char bname1[] = "fltinp";
        static char filnam1[] = "fltinp";
        static char filnam2[] = "fltout";

        /* Don't allow this command if restricted */
        if (restflag)
                return(resterr());

        /* Don't allow filtering of VIEW mode buffer */
        if (curbp->b_mode & MDVIEW)
                return(rdonly());

        /* Get the filter name and its args */
        s = mlreply("#", line, NLINE);
        if (!s)
                return(s);

        /* Setup the proper file names */
        bp = curbp;
        strcpy(tmpnam, bp->b_fname);    /* Save the original name */
        strcpy(bp->b_fname, bname1);    /* Set it to our new one */

        /* Write it out, checking for errors */
        if (!writeout(filnam1, "w")) {
                mlwrite("[Cannot write filter file]");
                strcpy(bp->b_fname, tmpnam);
                return(0);
        }

        /* Setup input and output */
        strcat(line," <fltinp >fltout");

        /* Perform command */
        s = callout(line);

        /* If successful, read in file */
        if (s) {
                s = readin(filnam2, FALSE);
                if (s)
                        /* Mark buffer as changed */
                        bp->b_flag |= BFCHG;
        }
                        

        /* Reset file name */
        strcpy(bp->b_fname, tmpnam);

        /* and get rid of the temporary file */
        unlink(filnam1);
        unlink(filnam2);

        /* Show status */
        if (!s)
                mlwrite("[Execution failed]");
        return(s);
}


#endif
