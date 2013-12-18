/* greed.c v2.0c - Written by Matthew T. Day (mday@ohs.uucp), 07/09/89       *
 * Document and send all source code changes to the above address, I'll make *
 * sure the approved patches are posted.  Please don't redistribute this.    */

/* vi:set ts=8:  set tabspace alignment to 8, usually I use 6 */

/* greed.c v2.1 - modified by David M. Wiesmore (dmw6997@ritvax.bitnet),     *
 * 07/27/89                                                                  *
 * changes -                                                                 *
 *     0. mv[w]printw was changed to a [w]move and then a [w]printw for      *
 *        compatibility                                                      *
 *     1. had to change some definitions for VMS                             *
 *     2. turn on the numeric keypad because VMS curses sets it to           *
 *        applications                                                       *
 *     3. VMS like return not newline                                        *
 *     4. needed to use clearok to refresh properly                          *
 *     5. the 'lockit' idea is not needed, and does not work, in VMS.  open  *
 *        for write locks the file.  if the file is locked try ever second   *
 *        15 times else quit                                                 *
 *     6. 'lockit'  is ignored by VMS                                        *
 *     7. getpwuid does not exist in VMS so I used cterid to get the         *
 *        username                                                           *
 *     8. tgetent does not exist in VMS so I check the terminal type and if  *
 *        it is a vt200 or vt100 then use bold on & off esc sequence         */
static char *version = "Greed v2.1";

#include <curses.h>
#include <signal.h>


#ifdef VAXC

#include <file.h>                        /* 1. different path      */
#include <unixlib>                       /* different name for pwd */
#define random rand                      /* just a different name  */
#define srandom srand                    /* just a different name  */
#define standout() setattr(_REVERSE)     /* just a different name  */
#define standend() clrattr(_REVERSE)     /* just a different name  */
#define SCOREFILE "user11:[dmw6997]greed.hs"  /* change */

#else

#ifdef __HELIOS
#define LOCKPATH "/helios/tmp/Greed.lock"
#else
#define LOCKPATH "/tmp/Greed.lock"	/* lock path for high score file */
#endif
#include <pwd.h>
#ifdef NOTBSD
#include <fcntl.h>
#else
#include <sys/file.h>
#endif

#ifdef NOTBSD
#define crmode cbreak
#define random lrand48			/* use high quality random routines */
#define srandom srand48
#endif

#ifdef __HELIOS
#define random rand
#define srandom srand
#endif

#endif

#define MAXSCORE 10			/* max number of high score entries */
#define FILESIZE (MAXSCORE * sizeof(struct score))	/* total byte size of *
							 * high score file    */
#define rnd(x) (int) ((random() % (x))+1)	/* rnd() returns random num *
						 * between 1 and x          */
#define ME '@'				/* marker of current screen location */

struct score {				/* changing stuff in this struct */
	char user[9];			/* makes old score files incompatible */
	int score;
};
int allmoves = 0, score = 1, grid[22][79], y, x, havebotmsg = 0;
char *cmdname;
extern long random();
void topscores();

/* botmsg() writes "msg" (in normal video) at the middle of the bottom    *
 * line of the screen.  Boolean "backcur" specifies whether to put cursor *
 * back on the grid or leave it on the bottom line (e.g. for questions).  */

void botmsg(msg, backcur)
register char *msg;
register backcur;
{
	standend();
	mvaddstr(23, 40, msg);
	clrtoeol();
	standout();
	if (backcur) move(y, x);
	refresh();
	havebotmsg = 1;
}

/* quit() is ran when the user hits ^C or ^\, it queries the user if he     *
 * really wanted to quit, and if so, checks the high score stuff (with the  *
 * current score) and quits; otherwise, simply returns to the game.         */

quit() {
	int (*osig)() = signal(SIGINT, SIG_IGN);	/* save old signal */
	(void) signal(SIGQUIT, SIG_IGN);

	if (stdscr) {
		botmsg("Really quit? ", 0);
		if (getch() != 'y') {
			move(y, x);
			(void) signal(SIGINT, osig);	/* reset old signal */
			(void) signal(SIGQUIT, osig);
			refresh();
			return(1);
		}
		move(23, 0);
		refresh();
		endwin();
		puts("\n");
		topscores(score);
	}
	exit(0);
}

/* out() is run when the signal SIGTERM is sent, it corrects the terminal *
 * state (if necessary) and exits.                                        */

void out() {
	if (stdscr) endwin();
	exit(0);
}

/* usage() prints out the proper command line usage for Greed and exits. */

void usage() {
	fprintf(stderr, "Usage: %s [-p] [-s]\n", cmdname);
	exit(1);
}

/* showscore() prints the score and the percentage of the screen eaten at the *
 * beginning of the bottom line of the screen, moves the cursor back on the   *
 * grid, and refreshes the screen.                                            */

void showscore() {
	standend();
	move(23, 7);
	printw("%d  %.2f%%", score, (float) score / 17.32);
	move(y, x);
	standout();
	refresh();
}

void showmoves();

main(argc, argv)
int argc;
char *argv[];
{
	int zero = 0;
	register val = 1;
	extern long time();

	cmdname = argv[0];			/* save the command name */
	if (argc == 2) {			/* process the command line */
		if (strlen(argv[1]) != 2 || argv[1][0] != '-') usage();
		switch(argv[1][1]) {
		case 'p':
			allmoves = 1;
			break;
		case 's':
			topscores(0);
			exit(0);
		default:
			usage();
		}
	} else if (argc > 2) usage();		/* can't have > 2 arguments */

	(void) signal(SIGINT, quit);		/* catch off the signals */
	(void) signal(SIGQUIT, quit);
	(void) signal(SIGTERM, out);

	initscr();				/* set up the terminal modes */
	crmode();
	noecho();
	refresh();				/* clears the screen */

#ifdef VAXC
        smg$create_virtual_keyboard (stdkb->_id);  /* 2. used to reset the */
        smg$set_keypad_mode (stdkb->_id,&zero);    /*    keypad to numeric */
#endif

	srandom(time(0) ^ getpid() << 16);	/* initialize the random seed *
						 * with a unique number       */

	for (y=0; y < 22; y++)			/* fill the grid array and */
		for (x=0; x < 79; x++)		/* print numbers out */
			mvaddch(y, x, (grid[y][x] = rnd(9)) + '0');

	mvaddstr(23, 0, "Score: ");		/* initialize bottom line */
	move(23, 40);
	printw("%s - Hit '?' for help.", version);
	standout();
	y = rnd(22)-1; x = rnd(79)-1;		/* random initial location */
	mvaddch(y, x, ME);
	grid[y][x] = 0;				/* eat initial square */

	if (allmoves) showmoves(1);
	showscore();

	while ((val = tunnel(getch())) > 0);	/* main loop, gives tunnel() *
						 * user command              */

	if (!val) {				/* if didn't quit by 'q' cmd */
		int x = 0;
		botmsg("Hit <Enter>", 0);	/* then let user examine     */
#ifdef VAXC
                while (getch() != '\r');  /* 3. VMS likes return not newline */
#else
		while ((x != '\n') && (x != '\r')) x = getch();	/* final screen              */
#endif
	}

	move(23, 0);
	refresh();
	endwin();
	puts("\n");				/* writes two newlines */
	topscores(score);
	exit(0);
}

/* tunnel() does the main game work.  Returns 1 if everything's okay, 0 if *
 * user "died", and -1 if user specified and confirmed 'q' (fast quit).    */

tunnel(cmd)
register cmd;
{
	register dy, dx, distance;
	void help();

	switch (cmd) {				/* process user command */
	case 'h': case '4':
		dy = 0; dx = -1;
		break;
	case 'j': case '2':
		dy = 1; dx = 0;
		break;
	case 'k': case '8':
		dy = -1; dx = 0;
		break;
	case 'l': case '6':
		dy = 0; dx = 1;
		break;
	case 'b': case '1':
		dy = 1; dx = -1;
		break;
	case 'n': case '3':
		dy = dx = 1;
		break;
	case 'y': case '7':
		dy = dx = -1;
		break;
	case 'u': case '9':
		dy = -1; dx = 1;
		break;
	case 'q':
		return(quit());
	case '?':
		help();
		return(1);
	case 12:				/* Control-L (redraw) */
#ifdef VAXC
                clearok(curscr, TRUE);  /* 4. need to tell VMS to clear first */
#endif
		wrefresh(curscr);		/* falls through to return */
	default:
		return(1);
	}
	distance = grid[y+dy][x+dx];		/* get attempted distance */

	{
		register j = y, i = x, d = distance;

		do {				/* process move for validity */
			j += dy;
			i += dx;
			if (j >= 0 && i >= 0 && j < 22 && i < 79 && grid[j][i])
				continue;	/* if off the screen */
			else if (!othermove(dy, dx)) {	/* no other good move */
				j -= dy;
				i -= dx;
				mvaddch(y, x, ' ');
				while (y != j || x != i) {
					y += dy;	/* so we manually */
					x += dx;	/* print chosen path */
					score++;
					mvaddch(y, x, ' ');
				}
				mvaddch(y, x, '*');	/* with a '*' */
				showscore();		/* print final score */
				return(0);
			} else {		/* otherwise prevent bad move */
				botmsg("Bad move.", 1);
				return(1);
			}
		} while (--d);
	}

	if (allmoves) showmoves(0);		/* remove possible moves */

	if (havebotmsg) {			/* if old bottom msg exists */
		standend();			/* put standard message back */
		move(23, 40);
		printw("%s - Hit '?' for help.", version);
		standout();
		havebotmsg = 0;
	}

	mvaddch(y, x, ' ');			/* erase old ME */
	do {					/* print good path */
		y += dy;
		x += dx;
		score++;
		grid[y][x] = 0;
		mvaddch(y, x, ' ');
	} while (--distance);
	mvaddch(y, x, ME);			/* put new ME */
	if (allmoves) showmoves(1);		/* put new possible moves */
	showscore();				/* does refresh() finally */
	return(1);
}

/* othermove() checks area for an existing possible move.  bady and badx are  *
 * direction variables that tell othermove() they are already no good, and to *
 * not process them.  I don't know if this is efficient, but it works!        */

othermove(bady, badx)
register bady, badx;
{
	register dy = -1, dx;

	for (; dy <= 1; dy++)
		for (dx = -1; dx <= 1; dx++)
			if ((!dy && !dx) || (dy == bady && dx == badx))
					/* don't do 0,0 or bad coordinates */
				continue;
			else {
				register j=y, i=x, d=grid[y+dy][x+dx];

				if (!d) continue;
				do {		/* "walk" the path, checking */
					j += dy;
					i += dx;
					if (j < 0 || i < 0 || j >= 22 ||
					    i >= 79 || !grid[j][i]) break;
				} while (--d);
				if (!d) return(1);	/* if "d" got to 0, *
							 * move was okay.   */
			}
	return(0);			/* no good moves were found */
}

/* showmoves() is nearly identical to othermove(), but it highlights possible */
/* moves instead.  "on" tells showmoves() whether to add or remove moves.     */

void showmoves(on)
register on;
{
	register dy = -1, dx;

	for (; dy <= 1; dy++)
		for (dx = -1; dx <= 1; dx++) {
			register j=y, i=x, d=grid[y+dy][x+dx];

			if (!d) continue;
			do {
				j += dy;
				i += dx;
				if (j < 0 || i < 0 || j >= 22
				    || i >= 79 || !grid[j][i]) break;
			} while (--d);
			if (!d) {
				register j=y, i=x, d=grid[y+dy][x+dx];

				/* The next section chooses inverse-video    *
				 * or not, and then "walks" chosen valid     *
				 * move, reprinting characters with new mode */

				if (on) standout();
				else standend();
				do {
					j += dy;
					i += dx;
					mvaddch(j, i, grid[j][i]);
				} while (--d);
				if (!on) standout();
			}
		}
}

/* doputc() simply prints out a character to stdout, used by tputs() */

void doputc(c)
register char c;
{
	(void) fputc(c, stdout);
}

/* topscores() processes it's argument with the high score file, makes any *
 * updates to the file, and outputs the list to the screen.  If "newscore" *
 * is 0, the score file is printed to the screen (i.e. "greed -s")         */

void topscores(newscore)
register int newscore;
{
	register fd, count = 1;
	static char termbuf[BUFSIZ];
	char *tptr = (char *) malloc(16), *boldon, *boldoff, user[8];
	struct score *toplist = (struct score *) malloc(FILESIZE);
	register struct score *ptrtmp, *eof = &toplist[MAXSCORE], *new = NULL;
	extern char *getenv(), *tgetstr();
	extern struct passwd *getpwuid();
	void lockit();

	(void) signal(SIGINT, SIG_IGN);		/* Catch all signals, so high */
	(void) signal(SIGQUIT, SIG_IGN);	/* score file doesn't get     */
	(void) signal(SIGTERM, SIG_IGN);	/* messed up with a kill.     */
	(void) signal(SIGHUP, SIG_IGN);

		/* following open() creates the file if it doesn't exist */
		/* already, using secure mode */
	if ((fd = open(SCOREFILE, O_RDWR | O_CREAT, 0600)) == -1) {
#ifdef VAXC
                register tries = 1; /* 5. VMS open locks the file when opened */
                                    /* for write so instead of waiting for    */
                                    /* the lock file we wait for the score    */
                                    /* file to unlock                         */
                sleep(1);
                while ((fd = open(SCOREFILE, O_RDWR | O_CREAT, 0600)) == -1) {
                        printf("Waiting for scorefile access... %d/15\n", tries);
                        if (tries++ >= 15) {
                                fprintf(stderr, "%s: %s: Can't open.\n",
                                        cmdname, SCOREFILE);
                                exit(1);
                        }
                        sleep(1);
                }
#else
		fprintf(stderr, "%s: %s: Cannot open.\n", cmdname, SCOREFILE);
		exit(1);
#endif
	}

#ifndef VAXC
	/* 6a. not used in VMS */
	lockit(1);			/* lock score file */
#endif
	for (ptrtmp=toplist; ptrtmp < eof; ptrtmp++) ptrtmp->score = 0;
					/* initialize scores to 0 */
	read(fd, toplist, FILESIZE);	/* read whole score file in at once */
	if (newscore) {			/* if possible high score */
		for (ptrtmp=toplist; ptrtmp < eof; ptrtmp++)
					/* find new location for score */
			if (newscore > ptrtmp->score) break;
		if (ptrtmp < eof) {	/* if it's a new high score */
			new = ptrtmp;	/* put "new" at new location */
			ptrtmp = eof-1;	/* start at end of list */
			while (ptrtmp > new) {	/* shift list one down */
				*ptrtmp = *(ptrtmp-1);
				ptrtmp--;
			}

			new->score = newscore;	/* fill "new" with the info */
#ifdef VAXC
                        cuserid(user); /* 7. getpwuid does not exist in VMS */
                        strncpy(new->user, user, 8);
#else
			strncpy(new->user, getpwuid(getuid())->pw_name, 8);
#endif
			(void) lseek(fd, 0, 0);	/* seek back to top of file */
			write(fd, toplist, FILESIZE);	/* write it all out */
		}
	}

	close(fd);
#ifndef VAXC
	lockit(0);			/* unlock score file */
#endif

	if (toplist->score) puts("Rank  Score  Name     Percentage");
	else puts("No high scores.");	/* perhaps "greed -s" was run before *
					 * any greed had been played? */
#ifdef VAXC
        boldon = boldoff = 0; /* 8a. tgetent does not exist in VMS */
        if (new && ((strncmp(getenv("TERM"), "vt200", 5) == 0)
                ||  (strncmp(getenv("TERM"), "vt100", 5) == 0)))
                boldon = boldoff = 1;
#else
	if (new && tgetent(termbuf, getenv("TERM")) > 0) {
		boldon = tgetstr("so", &tptr);		/* grab off inverse */
		boldoff = tgetstr("se", &tptr);		/* video codes */
		if (!boldon || !boldoff) boldon = boldoff = 0;
						/* if only got one of the *
						 * codes, use neither     */
	}
#endif

	/* print out list to screen, highlighting new score, if any */
	for (ptrtmp=toplist; ptrtmp < eof && ptrtmp->score; ptrtmp++, count++) {
#ifdef VAXC
                if (ptrtmp == new && boldon) printf("[1m"); /* 8b. boldon */
#else
		if (ptrtmp == new && boldon) tputs(boldon, 1, doputc);
#endif
		printf("%-5d %-6d %-8s %.2f%%", count, ptrtmp->score,
			ptrtmp->user, (float) ptrtmp->score / 17.32);
		fflush(stdout);
#ifdef VAXC
                if (ptrtmp == new && boldoff) printf("[22m"); /* 8b. boldoff */
#else
		if (ptrtmp == new && boldoff) tputs(boldoff, 1, doputc);
#endif
		putchar('\n');
	}
}

#ifndef VAXC
/* lockit() creates a file with mode 0 to serve as a lock file.  The creat() *
 * call will fail if the file exists already, since it was made with mode 0. *
 * lockit() will wait approx. 15 seconds for the lock file, and then         *
 * override it (shouldn't happen, but might).  "on" says whether to turn     *
 * locking on or not.                                                        */

void lockit(on)
register on;
{
	register fd, x = 1;

	if (on) {
		while ((fd = creat(LOCKPATH, 0)) == -1) {
			printf("Waiting for scorefile access... %d/15\n", x);
			if (x++ >= 15) {
				puts("Overriding stale lock...");
				if (unlink(LOCKPATH) == -1) {
					fprintf(stderr,
						"%s: %s: Can't unlink lock.\n",
						cmdname, LOCKPATH);
					exit(1);
				}
			}
			sleep(1);
		}
		close(fd);
	} else unlink(LOCKPATH);
}
#endif

#define msg(row, msg) mvwaddstr(helpwin, row, 2, msg);

/* help() simply creates a new window over stdscr, and writes the help info *
 * inside it.  Uses macro msg() to save space.                              */

void help() {
	WINDOW *helpwin = newwin(18, 65, 1, 7);

	(void) wclear(helpwin);
	box(helpwin, '|', '-');		/* print box around info */
					/* put '+' at corners, looks better */
	(void) waddch(helpwin, '+'); mvwaddch(helpwin, 0, 64, '+');
	mvwaddch(helpwin, 17, 0, '+'); mvwaddch(helpwin, 17, 64, '+');

	wmove(helpwin, 1, 2);
	wprintw(helpwin,
		"Welcome to %s, by Matthew T. Day (mday@ohs.uucp).", version);
	msg(3, "The object of Greed is to erase as much of the screen as");
	msg(4, "possible by moving around in a grid of numbers.  To move");
	msg(5, "your cursor, simply use the 'hjklyubn' keys or your number");
	wmove(helpwin, 6, 2);
	wprintw(helpwin,
		"keypad.  Your location is signified by the '%c' symbol.", ME);
	msg(7, "When you move in a direction, you erase N number of grid");
	msg(8, "squares in that direction, N being the first number in that");
	msg(9, "direction.  Your score reflects the total number of squares");
	msg(10, "eaten.  Greed will not let you make a move that would have");
	msg(11, "placed you off the grid or over a previously eaten square");
	msg(12, "unless no valid moves exist, in which case your game ends.");
	msg(13, "Other Greed commands are 'Ctrl-L' to redraw the screen and");
	msg(14, "'q' to quit.  Command line options to Greed are '-s' to output");
	msg(15, "the high score file and '-p' to highlight the possible moves");
	msg(16, "when playing.");

	(void) wmove(helpwin, 17, 64);
	wrefresh(helpwin);
	(void) wgetch(helpwin);
	delwin(helpwin);
	touchwin(stdscr);
	refresh();
}
