/*
 * Helios terminal I/O.
 * The functions in this file negotiate with the operating system for
 * characters, and write characters in a barely buffered fashion on the display.
 * All operating systems.
 */

#include <nonansi.h>
#include <attrib.h>
Attributes ostate;		/* saved attributes		*/
Attributes nstate;		/* values for editor mode	*/

#include "def.h"
#include "kbd.h"

#define CSI  0x9B
#define BELL 0x07

int nrow;			/* Terminal size, rows.		*/
int ncol;			/* Terminal size, columns.	*/

int tceeol = 2;
int tcinsl;
int tcdell;

extern int ttrow;
extern int ttcol;
extern int tttop;
extern int ttbot;
extern int tthue;

ttopen()
{
  setvbuf(stdin, NULL, _IONBF, 0);
  GetAttributes(Heliosno(stdin), &ostate);
  nstate = ostate;
  AddAttribute(&nstate,ConsoleRawInput);
  AddAttribute(&nstate, ConsoleRawOutput);
  RemoveAttribute(&nstate, ConsolePause);
  RemoveAttribute(&nstate, ConsoleIgnoreBreak);
  RemoveAttribute(&nstate, ConsoleBreakInterrupt);
  RemoveAttribute(&nstate, ConsoleEcho);
  ttraw();
  nrow = ostate.Time;
  ncol = ostate.Min;
  if (nrow > NROW) nrow = NROW;
  if (ncol > NCOL) ncol = NCOL;
  tcinsl = tcdell = (nrow * 2) + (ncol / 10);

  /* on all screens we are not sure of the initial position
     of the cursor					*/
  ttrow = 999;
  ttcol = 999;
}

ttraw()
{
  SetAttributes(Heliosno(stdin), &nstate);
}

ttclose()
{
  ttcooked();
}

ttcooked()
{
  SetAttributes(Heliosno(stdin), &ostate);
}

ttputc(c)
{
  fputc(c, stdout);
}

ttflush()
{
  fflush(stdout);
}

ttgetc()
{
  return(255 & fgetc(stdin));
}

typeahead()
{
  return FALSE;
}

panic(s)
char *s;
{
  fputs("panic: ", stderr);
  fputs(s, stderr);
  fputc('\n', stderr);
  fflush(stderr);
  exit(1);
}

#ifndef NO_DPROMPT
#include <sys/time.h>
/*
 * A program to return TRUE if we wait for 2 seconds without anything
 * happening, else return FALSE.  Cribbed from mod.sources xmodem.
 */
int ttwait() {
	int readfd;
	struct timeval tmout;

	tmout.tv_sec = 2;
	tmout.tv_usec = 0;

	readfd = 1;

	if ((select(1, &readfd, (int *)0, (int *)0, &tmout)) == 0)
		return(TRUE);
	return(FALSE);
}
#endif

ttparm(n)
{
  int q, r;

  q = n / 10;
  if (q != 0)
  {
    r = q / 10;
    if (r != 0) ttputc((r % 10) + '0');
    ttputc((q % 10) + '0');
  }
  ttputc((n % 10) + '0');
}

ttmove(row, col)
{
  if (ttrow != row || ttcol != col)
  {
    if (row > nrow) row = nrow;
    if (col > ncol) col = ncol;
    ttputc(CSI);
    ttparm(row + 1);
    ttputc(';');
    ttparm(col + 1);
    ttputc('H');
    ttflush();
    ttrow = row;
    ttcol = col;
  }
}

tteeol()
{
  ttputc(CSI);
  ttputc('K');
}

tteeop()
{
  ttputc(CSI);
  ttputc('J');
}

ttbeep()
{
  ttputc(BELL);
  ttflush();
}

ttinsl(row, bot, nchunk)
{
  int i;

IOdebug("ttinsl(%d, %d, %d)", row, bot, nchunk);
  if (row == bot)
  {
    ttmove(row, 0);
    tteeol();
    return;
  }
  ttmove(1 + bot - nchunk, 0);
  for (i = 0; i < nchunk; i++)
  {
    ttputc(CSI);
    ttputc('M');
  }
  ttmove(row, 0);
  for (i = 0; i < nchunk; i++)
  {
    ttputc(CSI);
    ttputc('L');
  }
  ttrow = row;
  ttcol = 0;
}

ttdell(row, bot, nchunk)
{
  int i;

IOdebug("ttdell(%d, %d, %d)", row, bot, nchunk);
  if (row == bot)
  {
    ttmove(row, 0);
    tteeol();
    return;
  }
  ttmove(row, 0);
  for (i = 0; i < nchunk; i++)
  {
    ttputc(CSI);
    ttputc('M');
  }
  ttmove(1 + bot - nchunk, 0);
  for (i = 0; i < nchunk; i++)
  {
    ttputc(CSI);
    ttputc('L');
  }
  ttrow = 1 + bot - nchunk;
  ttcol = 0;
}

ttcolor(color)
int color;
{
  if (color != tthue)
  {
    ttflush();
    ttputc(CSI);
    /* Normal video.	*/
    if (color == CTEXT) ttputc('0');
    /* Reverse video.	*/
    else /* if (color == CMODE) */ ttputc('7');
    ttputc('m');
    tthue = color;
  }
}

#ifdef	FKEYS
char	*keystrings[] = {
	"Up",		"Down",		"Left",		"Right",
	"Shift-Up",	"Shift-Down",	"Shift-Left",	"Shift-Right",
	"Help",		"The menu",	"The resize gadget", "The mouse",
	"F1",		"F2",		"F3",		"F4",
	"F5",		"F6",		"F7",		"F8",
	"F9",		"F10",		"Shift-F1",	"Shift-F2",
	"Shift-F3",	"Shift-F4",	"Shift-F5",	"Shift-F6",
	"Shift-F7",	"Shift-F8",	"Shift-F9",	"Shift-F10",
};
#endif

ttykeymapinit()
{
#ifdef	FKEYS
  KCHAR		c;
  register KEYMAP **mapp = &map_table[0].p_map;
#endif
  static KCHAR	esc_bs[] = { CCHR('['), CCHR('H') };
  static KCHAR	esc_del[] = { CCHR('['), CCHR('?') };
	
#define	BINDC(k,s) (c = k, bindkey(mapp, s, &c, 1))
#define	BINDM(m,s) bindkey(mapp, s, m, (sizeof(m)/sizeof(KCHAR)))
	
/* Swap the backspace and del keys, at least in normal usage.
 * This loses the help feature of CTRL-H, but we rebind
 * CTRL-_ to do the same thing. Under FKEYS, the Help key
 * calls describe-key-briefly.
 */
  BINDC(CCHR('_'),	"help-help");	/* CTRL-Backspace */
  BINDM(esc_bs,		"backward-kill-word");
  BINDC(CCHR('H'),	"delete-backward-char");

  BINDC(CCHR('?'),	"delete-char");
  BINDM(esc_del,		"kill-word");
}

getkbd()
{
  int c;
#ifdef	FKEYS
  int n;
#endif
loop:
  if ((c = ttgetc()) == CSI)
  {
    c = ttgetc();
#ifdef	FKEYS
    if (c == '?')
    {			/* HELP key		*/
      ttgetc();		/* discard '~'		*/
      return (KHELP);
    }
    /* Arrow keys */
    if (c == 'A') return (KUP);
    if (c == 'B') return (KDOWN);
    if (c == 'C') return (KRIGHT);
    if (c == 'D') return (KLEFT);
    if (c == 'T') return (KSUP);
    if (c == 'S') return (KSDOWN);

    /* Shifted left, right arrow */
    if (c == ' ')
    {
      c = ttgetc();
      if (c == 'A' || c == '@') return ((c == 'A') ? (KSLEFT) : (KSRIGHT));
      goto loop;		/* try again, sucker */
    }

    /* Function keys	*/
    if (c >= '0' && c <= '9')
    {
      n = 0;
      do
      {
        n = 10 * n + c - '0';
        c = ttgetc();
      } while (c >= '0' && c <= '9');
      if (c == '~' && n < 20) return (n < 9) ? (KF1 + n) : (KSF1 + (n - 10));
      else goto loop;	/* Try again */
    }
#endif
    goto loop;		/* Try again */
  }
  return (c);
}

