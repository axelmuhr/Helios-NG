static	char	RCSid[] =
"$Header: /dsl/HeliosRoot/Helios/cmds/public/stevie/RCS/helios.c,v 1.2 1993/03/31 15:51:01 nickc Exp tony $";

/*
 * System-dependent routines for UNIX System V or Berkeley.
 *
 * $Log: helios.c,v $
 * Revision 1.2  1993/03/31  15:51:01  nickc
 * fixed compile time errors
 *
 * Revision 1.1  1993/03/31  15:50:11  nickc
 * Initial revision
 *
 * Revision 1.5  88/10/31  13:11:03  tony
 * Added code (optional) to support the use of termcap.
 * 
 * Revision 1.4  88/10/27  08:16:52  tony
 * Added doshell() to support ":sh" and ":!".
 * 
 * Revision 1.3  88/10/06  10:14:36  tony
 * Added fixname() routine, which does nothing under UNIX.
 * 
 * Revision 1.2  88/06/20  14:51:36  tony
 * Merged in changes for BSD Unix sent in by Michael Lichter.
 * 
 * Revision 1.1  88/03/20  21:11:02  tony
 * Initial revision
 * 
 *
 */
#include <helios.h>
#undef FORWARD

#include "stevie.h"

#include <stdio.h>
#include <posix.h>
#include <nonansi.h>

/*
 * inchar() - get a character from the keyboard
 */

int
inchar( void )
{
	char	c;

	flushbuf();		/* flush any pending output */

	do
	{
		while (read(0, &c, 1) != 1)
			;
	} while (c == NUL);

	if (c == 0x9b)
	{
		switch (c = getchar())
		{
		case 'A': return K_UARROW;
		case 'B': return K_DARROW;
		case 'C': return K_RARROW;
		case 'D': return K_LARROW;
		case '@': return K_INSERT;
		case '?': c = getchar(); return K_HELP;
		case '1': c = getchar(); return K_UNDO;
		case 'H': return K_HOME;
		}
	}
	return c;
}

#define	BSIZE	2048
static	char	outbuf[BSIZE];
static	int	bpos = 0;

void
flushbuf( void )
{
	if (bpos != 0)
		write(1, outbuf, bpos);
	bpos = 0;
}

/*
 * Macro to output a character. Used within this file for speed.
 */
#define	outone(c)	outbuf[bpos++] = c; if (bpos >= BSIZE) flushbuf()

/*
 * Function version for use outside this file.
 */

void
outchar( char c )
{
	outbuf[bpos++] = c;

	if (bpos >= BSIZE)
		flushbuf();
}

void
outstr( char * s )
{
	while (*s) {
		outone(*s++);
	}
}

void
beep( void )
{
	outone('\007');
}

/*
 * remove(file) - remove a file
 */

/* already defined */

/*
 * rename(of, nf) - rename existing file 'of' to 'nf'
 */
/* already defined */

void
delay( void )
{
	Delay( 1 * OneSec );
}

#include <attrib.h>

/*
 * Go into raw mode
 */

void
set_tty( void )
{
  Attributes 	attr;
  int		result;
    
 
  setvbuf( stdin, NULL, _IONBF, 0 );

  if ((result = GetAttributes( Heliosno( stdin ), &attr )) < 0)
   {
     printf( "Failed to get stdin attributes : %x. Exiting.\n", result );
     
     exit( result );
   }

  AddAttribute(    &attr, ConsoleRawInput  );
  RemoveAttribute( &attr, ConsolePause );
  RemoveAttribute( &attr, ConsoleEcho  );
  
  if ((result = SetAttributes( Heliosno( stdin ), &attr )) < 0)
   {
     printf( "Failed to set stdin attributes : %x. Exiting.\n", result );
     
     exit( result );
   }   

  setvbuf( stdout, NULL, _IONBF, 0 );

  if ((result = GetAttributes( Heliosno( stdout ), &attr )) < 0)
   {
     printf( "Failed to get stdout attributes : %x. Exiting.\n", result );
     
     exit( result );
   }

  Columns        = attr.Time;
  P(P_LI) = Rows = attr.Min;

  AddAttribute(    &attr, ConsoleRawOutput );
  
  if ((result = SetAttributes( Heliosno( stdout ), &attr )) < 0)
   {
     printf( "Failed to set stdout attributes : %x. Exiting.\n", result );
     
     exit( result );
   }   
}

/*
 * Restore original terminal modes
 */
void
reset_tty( void )
{
  Attributes	attr;
  int		result;
  
 
  if ((result = GetAttributes( Heliosno( stdin ), &attr )) < 0)
   {
     printf( "Failed to get stdin attributes : %x. Exiting.\n", result );
     
     exit( result );
   }

  RemoveAttribute(    &attr, ConsoleRawInput  );
  AddAttribute( &attr, ConsolePause );
  AddAttribute( &attr, ConsoleEcho  );
  
  if ((result = SetAttributes( Heliosno( stdin ), &attr )) < 0)
   {
     printf( "Failed to reset stdin attributes : %x. Exiting.\n", result );
     
     exit( result );
   }

  if ((result = GetAttributes( Heliosno( stdout ), &attr )) < 0)
   {
     printf( "Failed to get stdout attributes : %x. Exiting.\n", result );
     
     exit( result );
   }

  RemoveAttribute(    &attr, ConsoleRawOutput );
  
  if ((result = SetAttributes( Heliosno( stdout ), &attr )) < 0)
   {
     printf( "Failed to set stdout attributes : %x. Exiting.\n", result );
     
     exit( result );
   }
}

void
windinit( void )
{
#ifdef	TERMCAP
	if (t_init() != 1)
	{
		fprintf(stderr, "unknown terminal type\n");
		exit(1);
	}
#else
	Columns = 80;
	P(P_LI) = Rows = 24;
#endif

	set_tty();
}

void
windexit( int r )
{
	reset_tty();
	exit(r);
}

#undef  outone
#define	outone(c)	outbuf[bpos++] = c; if (bpos >= BSIZE) flushbuf()

void
windgoto(r, c)
register int	r, c;
{
#ifdef	TERMCAP
	char	*tgoto();
#else
	r += 1;
	c += 1;
#endif

	/*
	 * Check for overflow once, to save time.
	 */
	if (bpos + 8 >= BSIZE)
		flushbuf();

#ifdef	TERMCAP
	outstr(tgoto(T_CM, c, r));
#else
	outbuf[bpos++] = '\033';
	outbuf[bpos++] = '[';
	if (r >= 10)
		outbuf[bpos++] = r/10 + '0';
	outbuf[bpos++] = r%10 + '0';
	outbuf[bpos++] = ';';
	if (c >= 10)
		outbuf[bpos++] = c/10 + '0';
	outbuf[bpos++] = c%10 + '0';
	outbuf[bpos++] = 'H';
#endif
}

FILE *
fopenb(fname, mode)
char	*fname;
char	*mode;
{
	return fopen(fname, mode);
}

char *
fixname(s)
char	*s;
{
	return s;
}

/*
 * doshell() - run a command or an interactive shell
 */
void
doshell(cmd)
char	*cmd;
{
	char	*cp, *getenv();
	char	cline[128];

	outstr("\r\n");
	flushbuf();

	if (cmd == NULL) {
		if ((cmd = getenv("SHELL")) == NULL)
			cmd = "/bin/sh";
		sprintf(cline, "%s -i", cmd);
		cmd = cline;
	}

	reset_tty();
	system(cmd);
	set_tty();

	wait_return();
}
/*
 *	FILL IT IN, FOR YOUR SYSTEM, AND SHARE IT!
 *
 *	The next couple of functions do system-specific stuff.
 *	They currently do nothing; I'm not familiar enough with
 *	system-specific programming on this system.
 *	If you fill it in for your system, please post the results
 *	and share with the rest of us.
 */


setcolor (c)
/*
 * Set the color to c, using the local system convention for numbering
 * colors or video attributes.
 *
 * If you implement this, remember to note the original color in
 * windinit(), before you do any setcolor() commands, and
 * do a setcolor() back to the original as part of windexit().
 */
  int c;
{
	/* Dummy routine, just return 0 */
	return (0);
}


setrows (r)
/*
 * Set the number of lines to r, if possible.  Otherwise
 * "do the right thing".  Return the number of lines actually set.
 *
 * If you implement this, remember to note the original number of rows
 * in windinit(), before you do any setrows() commands, and
 * do a setrows() back to the original as part of windexit().
 */
  int r;
{
	/* Since we do nothing, just return the current number of lines */
	return ( P(P_LI) );
}


vbeep ()
/*
 * Do a "visual bell".  This generally consists of flashing the screen
 * once in inverse video.
 */
{
	int	color, revco;

	color = P( P_CO );		/* get current color */
	revco = reverse_color (color);	/* system-specific */
	setcolor (revco);
	flushbuf ();
	pause ();
	setcolor (color);
	windgoto (Cursrow, Curscol);
	flushbuf ();
}

reverse_color (co)
/*
 * Returns the inverse video attribute or color of co.
 * The existing code below is VERY simple-minded.
 * Replace it with proper code for your system.
 */
 int co;
{
	if (co)		return (0);
	else		return (1);
}


/********** End of do-it-yourself kit **********************/
