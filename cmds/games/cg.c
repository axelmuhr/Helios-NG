#define VERSION		13		/* displays as X.X or so */
/*************************************************************************\
*  The all new super dooper good nifty chess guess.			  *
*									  *
*  This game is dedicated to Martin Gardener, who inspired it with	  *
*   his wonderful articles on recreational mathematics.  I hope someone   *
*   someday takes up where he left off.					  *
*									  *
*  Notes:  I can't concieve of a UNIX system where this won't compile     *
*  and run right out of the box, I'm really doing nothing particulary	  *
*  bizzare.  Well, thats not strictly true, you do need to have termcap   *
*  and curses.  So for those that have, enjoy, those that don't, sorry.   *
*									  *
*  Modification History:						  * 
*   started: 9/4/84							  *
*   updated for the net: 4/7/86						  *
*   updated again for the net, 9/19/87					  *
*   updated 9/22/87 for bug fixes, and add version number		  *
*									  *
*  The all important, super vicious copywrong message:			  *
*   Copyright 1984,86 and 87 by Jon Foreman, all rights reserved	  *
*   Permission is granted to distribute this code, or modify it, or use   *
*   portions in other programs, but you must give me credit in either the *
*   program or documentation, and you must not remove the copyright from  *
*   the source text of this program.  You may not sell this program or    *
*   source code for profit.						  *
*    (This is really an leagally backed ego booster for me, really).      *
*									  *
*  It would be really enjoyable to have this become part of the Berkeley  *
*   or Bell unix distributions, hint, hint.				  *
*									  *
\*************************************************************************/

#ifdef __HELIOS
#define srandom srand
#define random rand
#define toupper mytoupper
#define tolower mytolower
#endif

#include <curses.h>
#include <signal.h>

#ifndef LINT
char *copyleft = "Copyright 1987 by Jon Foreman, all rights reserved.";
#endif

#define T		1		/* TRUE */
#define F		0		/* FALSE */

struct bd {
	int  b_attcnt;			/* number of attacks on this cell */
	char b_pcetype;			/* type piece type is on this cell */
	char b_shown;			/* have we shown this cell yet? */
} board[8][8];

int kingmove(), queenmove(), rookmove(), bishmove(), nitemove();

struct pce {
	char *p_name;
	char p_type;
	int  (*p_attfun)();
	int  p_x, p_y;
	char p_guessed;
}  piece[5] = {
	{"King",   'K', kingmove,  0, 0, 0},
	{"Queen",  'Q', queenmove, 0, 0, 0}, /* queenmove is both bish & rook */
	{"Rook",   'R', rookmove,  0, 0, 0},
	{"Bishop", 'B', bishmove,  0, 0, 0},
	{"Knight", 'N', nitemove,  0, 0, 0}
};

int 	bguess = 0,		/* number of guessed pieces */
	tests  = 0,		/* number of attack tests */
	bw     = 1;		/* display black and white squares? */

/*
** If you haven't figured out what main is for yet, then you really
**  will have a great deal of trouble figuring out what the rest of this
**  code does.  One note, though, single letter varibles are almost always
**  loop indexes or other short term temporaries.
*/
main (ac, av)
char **av;
{
	int  endp();
	long time();
	int did = 0;		/* have we seen the board yet? */
	
	srandom (getpid () ^ (int) time (0L)); 	/* I bet this isn't portable */

/*
** Just have to find a way to get command line options in!
*/
	if (ac > 1 && av[1][0] == '-' && av[1][1] == 'n')
		bw = 0;

	initscr ();

	(void) signal (SIGINT, endp);
	(void) signal (SIGHUP, endp);

	nonl ();
	noecho ();
	crmode ();
	leaveok  (stdscr, FALSE);
	scrollok (stdscr, FALSE);

	move (0, 0);
	printw ("Chess guess, version %2d.%1d", VERSION/10, VERSION%10);

	for ( ; ; did = 1) {
		initboard ();
		dispboard (did, bw);
		showpiece ();
		userinterface ();
	}
	/* NOT REACHED */
}

/*
** Display the pieces on the gameboard
*/
showpiece ()
{
	int i;

	for (i = 0; i < 5; i++) {
		move (piece[i].p_y*2+2, piece[i].p_x*4+3+2);
		addch (piece[i].p_guessed ? piece[i].p_type : '*');
		refresh ();
	}
	return;
}

/*
** Initialize the play board.  First clear all the cells, then place the
**  pieces, and then count attacks.  Nothing to it.
*/
initboard ()
{
	int i, j;
	
	bguess = tests = 0;

	for (i = 0; i < 8; i++) 
		for (j = 0; j < 8; j++)
		
			board[i][j].b_pcetype = board[i][j].b_attcnt = board[i][j].b_shown = 0;


/*
** I suppose it is possible to get an infinite loop here, but there are
**  after all 64 squares on the board, and I'm only using 5.  I suspect
**  that rnd() will eventually return 5 different pairs of x,y's.
*/
	for (i = 0; i < 5; i++)
		for ( ;; )
			if (board[piece[i].p_x = rnd (8)][piece[i].p_y = rnd (8)].b_pcetype == 0) {
				board[piece[i].p_x][piece[i].p_y].b_pcetype = piece[i].p_type;
				break;
			}

	for (i = 0; i < 5; i++) {
		piece[i].p_guessed = 0;
		piece[i].p_attfun (&piece[i]);
	}
	return;
}

/*
** Come here whenever we want to stop playing, and unsetup all the stuff
**  that got set up.
*/
endp ()
{
	(void) signal (SIGINT, SIG_IGN);
	(void) signal (SIGHUP, SIG_IGN);
	mvcur (0, COLS-1, LINES-1, 0);	/* cleveland */
	endwin ();
	exit (0);
}

/*
** Display the playfield, and stuff like that.
*/
dispboard (dispfix, bw)
int dispfix;				/* just fixing the display (=1) */
int bw;					/* want black & white squares (-1) */
{
	int i, j, phase;
	
	if (!dispfix)
		mvaddstr (1, 40, "There are one each of these:");

	for (phase = i = 0; i <= 8; i++) {
		mvaddstr (i*2+1, 3, "+---+---+---+---+---+---+---+---+");
		if (i != 8) {
			move (i*2+2, 0);
			printw ("%2d", 8 - i);
			move (i*2+2, 3);

			for (j = 0; j <= 8; j++, phase = !phase) {
				addch ('|');
				if (j != 8) {
					addch ((bw && phase) ? '.' : ' ');
					if (board[j][i].b_shown)
						addch (board[j][i].b_pcetype ? board[j][i].b_pcetype : (board[j][i].b_attcnt + '0'));
					else if (board[j][i].b_pcetype != 0)
						addch ('*');
					else
						addch ((bw && phase) ? '.' : ' ');
					addch ((bw && phase) ? '.' : ' ');
				}
			}
			refresh ();
		}
	}

	if (dispfix)		/* only fixing display board */
		return;

	mvaddstr (i*2, 5, "a   b   c   d   e   f   g   h");
	refresh ();
	for (i = 0; i < 5; i++) {
		move (i+2, 40);
		printw ("%c = %s\n", piece[i].p_type, piece[i].p_name);
		refresh ();
	}
	mvaddstr ( 8, 40, "Commands:");
	mvaddstr ( 9, 40, "'Q' or <BREAK> is Quit");
	mvaddstr (10, 40, "'G' is Guess");
	mvaddstr (11, 40, "' ' is ask for attacks");
	mvaddstr (12, 40, "'h' or '^B' moves back");
	mvaddstr (13, 40, "'j' or '^N' moves down");
	mvaddstr (14, 40, "'k' or '^P' moves up");
	mvaddstr (15, 40, "'l' or '^F' moves forward");
	mvaddstr (16, 40, "'r' or '^L' redraws screen");
	mvaddstr (17, 40, "'t' to toggle black & white squares");
	mvaddstr (18, 40, "'?' to get help");
	refresh ();
}

/*
** Kings can move one square in any direction.
*/
kingmove (p)
struct pce *p;
{
	int i, j;
	
	for (i = -1; i <= 1; i++)
		for (j = -1; j <= 1; j++) {
			if (!onboard (p->p_x + i, p->p_y + j) || (!i && !j))
				continue;
			board[p->p_x + i][p->p_y + j].b_attcnt ++ ;
		}
	return;
}

/*
** Knights can move over things, so just check to see if there is
**  anything at where we end up.  Problem: figure out exactly what
**  the for(;;) loops do.
*/
nitemove (p)
struct pce *p;
{
	int i, j;

	for (i = -2; i <= 2; i += (!++i) ? 1 : 0)
		for (j = 2; j >= -2; j += (!--j) ? -1 : 0) {
			if (abs (j) == abs (i))
				continue;
			if (!onboard (p->p_x + i, p->p_y + j))
				continue;
			board[p->p_x + i][p->p_y + j].b_attcnt ++ ;
		}
	return;
}

/*
** Rooks can only move horizontally or vertically, and cannot move through
**  other pieces to get there
*/
rookmove (p)
struct pce *p;
{
	/*
	** radiate outward in the correct directions until we hit
	**  something.  Like the end of the board, or another piece.
	*/
	register int q;

	for (q = p->p_x - 1; onboard (q, p->p_y); --q)
		board[q][p->p_y].b_attcnt++;
	for (q = p->p_x + 1; onboard (q, p->p_y); ++q)
		board[q][p->p_y].b_attcnt++;
	for (q = p->p_y - 1; onboard (p->p_x, q); --q)
		board[p->p_x][q].b_attcnt++;
	for (q = p->p_y + 1; onboard (p->p_x, q); ++q)
		board[p->p_x][q].b_attcnt++;
	return;
}

/*
** Bishops move diagonally
*/
bishmove (p)
struct pce *p;
{
	/*
	** radiate in the proper directions until you hit something.
	*/
	int i;
	int done, a, b, c, d;

	for (a = b = c = d = T, done = 0, i = 1; !done; done = !(a|b|c|d), i++) {
		if (a)
			if (a = onboard (p->p_x + i, p->p_y + i))
				board[p->p_x + i][p->p_y + i].b_attcnt ++ ;
		if (b)
			if (b = onboard (p->p_x + i, p->p_y - i))
				board[p->p_x + i][p->p_y - i].b_attcnt ++ ;
		if (c)
			if (c = onboard (p->p_x - i, p->p_y + i))
				board[p->p_x - i][p->p_y + i].b_attcnt ++ ;
		if (d)
			if (d = onboard (p->p_x - i, p->p_y - i))
				board[p->p_x - i][p->p_y - i].b_attcnt ++ ;
	}		
	return;
}
/*
** Queens are just rooks | bishops
*/
queenmove (p)
struct pce *p;
{
	bishmove (p);
	rookmove (p);
	return;
}

/*
** Make sure that we're still on the board someplace.
*/
onboard (x, y)
register int x, y;
{
	if (x >= 8 || y >= 8 || x < 0 || y < 0 || board[x][y].b_pcetype)
		return (F);
	return (T);
}

/*
** Simplified integer random number generator, taylor for this application
*/
rnd (n)
int n;
{
	long random();

	return ((unsigned) random() % (unsigned) n);
}

/*
** MacIntosh inspired keyboard interface..., no wait, that a lie.  Actually,
**  this one if fairly intuitional (what with the help on the screen and all)
**  and I suppose if I had a Sun I could even implement mouse functions.
**  But I don't, so I didn't.  Just remember, all good programs have a 
**  well designed userinterface().
*/
userinterface ()
{
	char c;
	int x, y, ox, oy, gsd;
	
	x = y = gsd = 0;
	move (y*2+2, x*4+3+2);
	refresh ();
	
	for (;;) {
		c = tolower (getch ());
		getyx (stdscr, oy, ox);
		move (22, 0);
		clrtoeol ();
		move (oy, ox);
		refresh ();
		switch (c) {
			case 'h': case '\002':
				if (x > 0)
					--x;
				break;

			case 'j': case '\016':
				if (y++ > 6)
					--y;
				break;

			case 'k': case '\020':
				if (y > 0)
					--y;
				break;

			case 'l': case '\006':
				if (x++ > 6)
					--x;
 				break;

			case ' ':
				if (board[x][y].b_shown)
					break;

				if (board[x][y].b_pcetype == 0) {
					printw ("%1d", board[x][y].b_attcnt);
					board[x][y].b_shown = 1;
					tests++;
				}
				break;

			case 'g': case 'G':
				if (board[x][y].b_shown)
					break;

				if (board[x][y].b_pcetype == 0)
					break;
				getyx (stdscr, oy, ox);
				mvaddstr (22, 3, "Piece? ");
#ifdef __HELIOS
				wclrtoeol (stdscr);
#else
				clrtoeol (stdscr);
#endif
				refresh ();
				echo ();
				c = getch ();
				refresh ();
				move (oy, ox);
				noecho ();
				if (toupper (c) == board[x][y].b_pcetype) {
					int q;

					addch (board[x][y].b_pcetype);
					board[x][y].b_shown = 1;
					refresh ();
					for (q = gsd = 0; q < 5; q++) {
						if (board[x][y].b_pcetype == piece[q].p_type) {
							gsd++;
							piece[q].p_guessed = 1;
						} else if (piece[q].p_guessed) {
							gsd++;
						}
					}
					if (gsd == 5) {
						win ();
						return;
					}
				} else {
					putchar ('\007');
					bguess++;
				}
				break;

			case 'q':
				endp ();

			case 'r': case '\014':
				wrefresh (curscr);
				break;

			case 't':
				dispboard (F, bw = !bw);
				refresh ();
				break;

			case '?':
				help();
				break;

			default:		/* reminder */
				break;
		}
		move (y*2+2, x*4+3+2);
		refresh ();
	}
}

/*
** I have troubles finding toupper on every machine I use, so I wrote my
**  own.  I know, I know, "ctype.h" right.  Wro.  This is slower, but
**  at least I know everyone will have it.
*/
toupper (c)
char c;
{
	return ((c >= 'a' && c <= 'z') ? c -= '\040' : c);
}

/*
** See comment for toupper (above).  These routines also make more sense
**  in that we don't have to find out if islower() or isupper is true
**  or not first.
*/
tolower (c)
char c;
{
	return ((c >= 'A' && c <= 'Z') ? c += '\040' : c);
}

win ()
/*
** Colors, someone won one!  Hurray!.
*/
{
	char *buf[200];
	
	sprintf (buf, "YOU WIN!  statistics: %d tests, %d bad guesses\n", tests, bguess);
	mvaddstr (0, 0, buf);
	mvaddstr (22, 3, "Go again? ");
	refresh ();
	if (getch () == 'n')
		endp ();
	move (0,  0);
	clrtoeol ();
	move (22, 0);
	clrtoeol ();
	return;
}

/*
** Generate a help screen, and if nothing else, this is a good
**  example of how to create overlapping windows (like in rogue
**  and so forth
*/
#define HWXMAX		50
#define HWYMAX		18

help ()
{
	WINDOW *hwin;
	int curx, cury;

	getyx (stdscr, cury, curx);
	hwin = newwin (HWYMAX, HWXMAX, 0, 0);
	mybox (hwin, HWYMAX, HWXMAX);

	mvwaddstr (hwin,2,2,"   Chess guess is a game that was described in");
	mvwaddstr (hwin,3,2,"one of Martin Gardeners famous recreational");
	mvwaddstr (hwin,4,2,"mathematics articles for \"Scientific American\"");
	mvwaddstr (hwin,5,2,"magazine.");
	mvwaddstr (hwin,7,2,"   You are given a chess board with 5 pieces");
	mvwaddstr (hwin,8,2,"displayed.  The pieces are disguised, and your");
	mvwaddstr (hwin,9,2,"mission (should you decide to accept it) is to");
	mvwaddstr (hwin,10,2,"guess which pieces are which.  You can ask for");
	mvwaddstr (hwin,11,2,"help in the form of asking how many pieces can");
	mvwaddstr (hwin,12,2,"attack various squares on the board.  The");
	mvwaddstr (hwin,13,2,"challenge is to pick squares that maximize");
	mvwaddstr (hwin,14,2,"your chance of guessing the correct pieces.");
	mvwaddstr (hwin,16,2," (when done reading, press space) ");
	wrefresh (hwin);
	getch ();

	delwin (hwin);
	move (cury, curx);
	touchwin (stdscr);
	refresh ();
	return;
}

/*
** This is my box routine, here because the standard box() didn't really
**  do what I wanted.  Note, it clear the window as it goes along.
*/
mybox (hwin, ymax, xmax)
WINDOW *hwin;
{
	int y, x;

	for (y = 0; y < ymax; y++) {
		for (x = 0; x < xmax; x++)
			if (x == 0 && y == 0)
				mvwaddch (hwin, y, x, '/');
			else if (x == 0 && y == ymax-1)
				mvwaddch (hwin, y, x, '\\');
			else if (x == xmax-1 && y == 0)
				mvwaddch (hwin, y, x, '\\');
			else if (x == xmax-1 && y == ymax-1)
				mvwaddch (hwin, y, x, '/');
			else if (x > xmax-1 || y > ymax-1)
				mvwaddch (hwin, y, x, ' ');
			else if (y == 0)
				mvwaddch (hwin, y, x, '=');
			else if (y == ymax-1)
				mvwaddch (hwin, y, x, '-');
			else if (x == 0 || x == xmax-1)
				mvwaddch (hwin, y, x, '|');
	}
	wmove (hwin, 0, 2);
	waddstr (hwin, " * Help * ");
	return;
}
