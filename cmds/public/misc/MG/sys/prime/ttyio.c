/*
 *     sys>prime>ttyio.c by Robert A. Larson
 *
 * The functions in this file
 * negotiate with the operating system for
 * keyboard characters, and write characters to
 * the display in a barely buffered fashion.
 */
#include       "def.h"

#define	       NOBUF   512	       /* Output buffer size.	       */

char   obuf[NOBUF];		       /* Output buffer.	       */
short  nobuf;			       /* characters in obuf	       */
int    nrow;			       /* Terminal size, rows.	       */
int    ncol;			       /* Terminal size, columns.      */
short  ospeed;			       /* Terminal speed, for termlib.l */

#ifndef PRE21
/* use undocumented calls to set terminal modes */
#define NSETTINGS 6
fortran void as$lin(), as$get(), as$set();
static short myline = 0;
static short reset[NSETTINGS][2];
#else
/* do the best we can without undocumented calls */
fortran short duplx$();
fortran void break$();
short duplx;
#endif

/*
 * This function gets called once, to set up
 * the terminal channel.
 */

ttopen()
{
#ifndef PRE21
    short code, vers, len, bad;
    static short settings[NSETTINGS][2] = {
	{1, 0},			       /* no echo */
	{2, 0},			       /* don't echo line feed for cr */
	{3, 0},			       /* disable ^S/^Q processing */
	{11, 0},		       /* parity: none */
	{12, 3},		       /* character length: 8 */
	{15, 1},		       /* tran protocol */
    };
    short plist[38][2];
    short *p;
    register int i;

    if(myline==0) {
	as$lin(myline, code);
	if(code!=0) panic("Can't get terminal line");
	/* do io to get terminal type before setting modes */
	(void) gettermtype();
    }
    p = &plist[0][0];
    as$get(myline, vers, p, len, code);
    if(code!=0) panic("Can't determine current settings");
    ospeed = plist[9-1][1];
    for(i=0; i < NSETTINGS; i++) {
	reset[i][0] = settings[i][0];
	reset[i][1] = plist[settings[i][0]-1][1];
    }
    p = &settings[0][0];
    as$set(myline, (short)0, p, (short)NSETTINGS, code, bad);
    if(code!=0) panic("Can't set terminal modes");
#else
    (void) gettermtype();
    duplx = duplx$((short)-1);
    (void) duplx$((unsigned short)0140000);
    ospeed = 4; /* assume 9600 for debugging */
#endif
    nobuf = 0;
}

/*
 * This function gets called just
 * before we go back home to the shell. Put all of
 * the terminal parameters back.
 */
ttclose()
{
#ifndef PRE21
    short code, bad;
    short *p;
#endif

    ttflush();
#ifndef PRE21
    p = &reset[0][0];
    as$set(myline, (short)0, p, (short)NSETTINGS, code, bad);
    if(code!=0) {
	printf("Can't reset terminal modes: code: %d bad: %d\n",
	    code, bad);
	exit(1);
    }
#else
    (void) duplx$(duplx);
#endif
}

/*
 * Write character to the display.
 * Characters are buffered up, to make things
 * a little bit more efficient.
 */
ttputc(c)
{
       if (nobuf >= NOBUF)
	       ttflush();
       obuf[nobuf++] = c;
}

/*
 * Flush output.
 */
ttflush()
{
       fortran void tnoua();

       if (nobuf != 0) {
	       tnoua((char [])obuf, nobuf);
	       nobuf = 0;
       }
}

/*
 * Read character from terminal.
 * Parity bit is stripped, to be normal
 */
ttgetc()
{
    fortran void t1in();
    short c;

    t1in(c);
#ifdef DO_METAKEY
    if(c&0200) c |= METABIT;
#endif
    c |= 0200;
#ifdef PRE21
    if(c=='\n') return '\r';	   /* fix repping done by primos */
#ifdef DO_METAKEY
    if(c==('\n'|METABIT)) return '\r'|METABIT;
#endif
#endif
    return c;
}

int typeahead()
{
    fortran short tty$in();

    return tty$in() < 0;
}

panic(s) char *s; {
  fortran void tnoua(), tnou();

  ttclose();
  tnoua((char [])"Panic: ", (short)7);
  tnou((char [])s, (short)strlen(s));
  exit(1);
}

#ifndef NO_DPROMPT
ttwait() {
  register short i = 20;
  fortran short tty$in();
  fortran void sleep$();

  while(i--) {
    if(tty$in()<0) return FALSE;
    sleep$((long)100);
  }
  return tty$in() >= 0;
}
#endif

char *gettermtype()
{
    register char *cp;
    char *gvget();
    static char termtype[64] = "";

    if(termtype[0]!='\0') return termtype;
    cp = gvget(".TERMINAL_TYPE$");     /* get terminal type */
    if(cp==NULL) cp = gvget(".TERM");
    if(cp != NULL) {
	strncpy(termtype, cp, 64);
	return termtype;
    }
    fputs("Terminal type? ", stdout);
    fgets(termtype, 64, stdin);
    putchar('\n');
    termtype[strlen(termtype)-1] = '\0';	/* chop off \n */
    return termtype;
}
