/*
 * Revision History:
 *
 * Original source from:
 *  Peter da Silva (ihnp4!shell!neuro1!{hyd-ptd,datafact,baylor}!peter)
 *
 * Changes for padding added by:
 *  Andrew Scott Beals ({ucbvax,decwrl}!amdcad!bandy or bandy@amdcad.amd.com)
 *  20 April 1987
 *
 * Additional changes for padding, fix for computation of tglen,
 * increase max lines, improve termlib handling, add System V #ifdefs.
 *  Bill Randle (billr@tekred.TEK.COM)
 *  21 April 1987
 *
 * Add padding for cl.
 *  Andrew Scott Beals ({ucbvax,decwrl}!amdcad!bandy or bandy@amdcad.amd.com)
 *  22 April 1987
 *
 * Add comments and scroll fixes from Colin Plumb (ccplumb@watmath.UUCP)
 * More terminal handling cleanup and error checking.
 *  Bill Randle (billr@tekred.TEK.COM)
 *  22 April 1987
 */

#include <stdio.h>

#ifdef SYSV
# include <termio.h>
#else
# include <sgtty.h>
#endif

/*		-- Miscellaneous defines --				     */
#define FALSE 0
#define TRUE 1
#define MAXCOL 80
#define MAXLI 36

#define min(a,b)	((a)<(b) ? a : b)

extern char *tgetstr();

int lastx, lasty;
struct _c {
	struct _c *c_next;
	int c_line, c_column;
	char c_mark;
} *clist;

/*		-- Global variables --					     */
char *tent;                                               /* Pointer to tbuf */
#ifdef __HELIOS
char PC;
char *UP, *BC;
short ospeed;
#else
extern char PC;                                             /* Pad character */
extern char *UP, *BC;                         /* Upline, backsapce character */
extern short ospeed;                                /* Terminal output speed */
#endif
int tglen, bclen;

char *cm,                                                   /* Cursor motion */
     *cl,                                                    /* Clear screen */
     *ti,						    /* Init terminal */
     *te;						   /* Reset terminal */
int  li,                                                  /* lines on screen */
     co;                                                    /* columns ditto */
char screen[MAXLI+1][MAXCOL];
char newscreen[MAXLI+1][MAXCOL];

main(ac, av)
int ac;
char **av;
{
	/* set ospeed so padding works correctly */
#ifdef SYSV
	struct termio	p;

	if(ioctl(1, TCGETA, &p) != -1)
		ospeed=p.c_cflag & CBAUD;
#else
	struct sgttyb	p;

	if(ioctl(1, TIOCGETP, &p) != -1)
		ospeed=p.sg_ospeed;
#endif

	srand(getpid());	/* init random number generator */
	tinit(getenv("TERM"));	/* init terminal */
	if(ac > 1)
		/* do all files */
		while(--ac)
			dropf(*++av);
	else
		/* or stdin */
		fdropf(stdin);
	tend();			/* clean up terminal */
}

/* put character c at (x, y) */
at(x, y, c)
int x, y;
char c;
{
#ifdef DEBUG
	_at(x, y);
#else
	/* optimize cursor motion if goal is on *same* line */
	if(y==lasty) {
		if(x!=lastx) {
			if(x<lastx && (lastx-x)*bclen<tglen)
				/* backspace into place if it takes */
				/* less characters than cursor motion */
				while(x<lastx) {
					if (bclen > 1)
						outs(BC);
					else
						putchar(*BC);
					lastx--;
				}
			else if(x>lastx && x-lastx<tglen)
				/* print intervening characters */
				while(x>lastx) {
					putchar(newscreen[lasty][lastx]);
					lastx++;
				}
			else
				_at(x, y);
		}
	} else
		_at(x, y);
#endif
	c &= ~0200;
	putchar(c);
	if(c >= ' ' && c != '\177')
		lastx++;
	if(lastx>=co) {
		lastx -= co;
		lasty++;
	}
}

_at(x, y)
int x, y;
{
	extern void	outc();

	tputs(tgoto(cm, x, y), 1, outc);	 /* handle padding */
	lastx = x;
	lasty = y;
}

void
outc(c)
register c;
{
	putc(c, stdout);
}

/* initialize terminal dependent variables */
tinit(name)
char *name;
{
	static char junkbuf[1024], *junkptr;
	char tbuf[1024];
	int  intr();

	junkptr = junkbuf;

	tgetent(tbuf, name);

	if (!tgetflag("bs"))		/* is backspace not used? */
		BC = tgetstr("bc",&junkptr);	/* find out what is */
	else
		BC = "\b";		/* make a backspace handy */
	bclen = strlen(BC);		/* for optimization stuff */
	if (tgetstr("pc", &junkptr) != NULL)
		PC = *junkptr;  /* set pad character */
	else
		PC = '\0';
	UP = tgetstr("up", &junkptr);
	cm = tgetstr("cm", &junkptr);
	if (cm == NULL) {
		printf("Can't rot on dumb terminals.\n");
		exit(1);
	}
	cl = tgetstr("cl", &junkptr);
	ti = tgetstr("ti", &junkptr);
	te = tgetstr("te", &junkptr);
	li = min(tgetnum("li"), MAXLI);
	if (li == -1)
		li = 24;
	/* the original code had special case code for last line and */
	/* last column.  Unfortunately, it didn't always work on all */
	/* terminals so we take the easy way out and don't use the   */
	/* bottom line of the screen				     */
	li--;	/* prevent bottom screen line from scrolling */
	co = min(tgetnum("co"), MAXCOL);
	if (co == -1)
		co = 80;
	tglen = strlen(tgoto(cm, co-1, li-1)); /* for optimization stuff */
	if (ti != NULL)
		outs(ti);
}

/* cleanup terminal after use */
tend()
{
	if (te != NULL)
		outs(te);
	_at(0, li);
	putchar('\n');
	fflush(stdout);
}

/* read in a new screen */
readscreen(fp)
FILE *fp;
{
	int line, column, p;
	char tmp[256];

	for(line=0; line<li; line++)
		for(column=0; column<co; column++)
			newscreen[line][column] = screen[line][column] = ' ';
	for(column=0; column<co; column++)
		newscreen[li][column] = screen[li][column] = '*';
	line=0;
	while(line<li) {
		if(!fgets(tmp, 256, fp))
			return;

		for(column=0, p=0; tmp[p]; p++) {
			tmp[p] &= ~0200;
			if(tmp[p] < ' ' || tmp[p] == 127)
				switch(tmp[p]) {
					case '\t':
						while(++column % 8)
							continue;
						break;
					case '\n':
						column = 0;
						line++;
						break;
					default:
						newscreen[line][column] = '^';
						column++;
						if(column>=co) {
							column -= co;
							line++;
						}
						newscreen[line][column] =
							(tmp[p]+'@') & 127;
						column++;
						break;
				}
			else {
				newscreen[line][column] = tmp[p];
				column++;
			}
			if(column >= co) {
				column -= co;
				line++;
			}
			if(line >= li)
				break;
		}
	}
	for(column=0; column<co; column++)
		newscreen[line][column] = screen[li][column] = '*';
}

drawscreen()
{
	extern void	outc();

	lastx = lasty = 0;
	if (cl != NULL)
		tputs(cl, li, outc);	/* for really slow terminals */
	update();
}

/* copy newscreen[][] to physical screen and screen[][] */
update()
{
	int l, c;

	for(l=0; l<li; l++)
		for(c=0; c<co; c++)
			/* copy any changes */
			if(screen[l][c] != newscreen[l][c]) {
				/* are they *really* different? */
				if((screen[l][c] & ~0200) !=
				   (newscreen[l][c] & ~0200))
					at(c, l, newscreen[l][c]);
				screen[l][c] = newscreen[l][c];
			}
}

/* add char at (column, line) to clist if feasable */
drop(line, column)
int line, column;
{
	struct _c *hold;

	if(line<0 || line>=li || column<0 || column>=co || /* off screen */
	   screen[line][column]==' ' || /* empty */
	   screen[line][column] & 0200) /* already in list */
		return;
	if(screen[line+1][column]!=' ' &&
	   (column==co-1 ||screen[line+1][column+1]!=' ') &&
	   (column==0 ||screen[line+1][column-1]!=' ')) /* can't be dropped */
		return;

	hold = (struct _c *) malloc(sizeof(struct _c));
	hold -> c_next = clist;
	hold -> c_column = column;
	hold -> c_line = line;
	hold -> c_mark = 0;
	screen[line][column] |= 0200;
	clist = hold;
}

/* drop everything in the clist */
drops()
{
	int line, column;
	struct _c *hold;

	for(hold = clist; hold; hold=hold->c_next) {
		line = hold->c_line;
		column = hold->c_column;
		/* add adjacent characters to clist */
		drop(line+1, column);
		drop(line, column+1);
		drop(line-1, column);
		drop(line, column-1);
		/* drop straight down if possible */
		if(newscreen[line+1][column]==' ') {
			newscreen[line+1][column] = screen[line][column];
			newscreen[line][column] = ' ';
			line++;
		}
		/* otherwise try and drop to the sides.  Randomly pick
		/* which side to try first. */
		else if(rand()&01000) {
			if(column>0 && newscreen[line][column-1] == ' ' &&
			    newscreen[line+1][column-1]==' ') {
				newscreen[line][column-1] =
					screen[line][column];
				newscreen[line][column] = ' ';
				column--;
			}
			else if(column<co-1 &&
				newscreen[line][column+1] == ' ' &&
				newscreen[line+1][column+1]==' ') {
					newscreen[line][column+1] =
						screen[line][column];
					newscreen[line][column] = ' ';
					column++;
			}
			else {
				/* forget it */
				screen[line][column] &= ~0200;
				newscreen[line][column] &= ~0200;
				hold -> c_mark = 1;
			}
		} else {
			if(column<co-1 && newscreen[line][column+1] == ' ' &&
			    newscreen[line+1][column+1]==' ') {
				newscreen[line][column+1] =
					screen[line][column];
				newscreen[line][column] = ' ';
				column++;
			}
			else if(column>0 && newscreen[line][column-1] == ' ' &&
			    newscreen[line+1][column-1]==' ') {
				newscreen[line][column-1] =
					screen[line][column];
				newscreen[line][column] = ' ';
				column--;
			}
			else {
				/* forget it */
				newscreen[line][column] &= ~0200;
				screen[line][column] &= ~0200;
				hold -> c_mark = 1;
			}
		}
		/* update list entry */
		hold -> c_column = column;
		hold -> c_line = line;
	}

	/* delete all list entries marked for deletion */
	/* do all at head of list */
	while(clist && clist->c_mark) {
		struct _c *p = clist;
		clist = clist -> c_next;
		free(p);
	}
	/* ...and all in body */
	hold = clist;
	while(hold && hold->c_next)
		if(hold->c_next->c_mark) {
			struct _c *p = hold->c_next;
			hold->c_next = p->c_next;
			free(p);
		} else
			hold=hold->c_next;
}

droplet(line, column)
int line, column;
{
	int ret;
	while(column>=0 && screen[line][column]!=' ')
		column--;
	column++;
	while(column<co && screen[line][column]!=' ')
		drop(line, column++);
	ret = clist != 0;
	while(clist) {
		drops();
		update();
	}
	return ret;
}

dropscreen()
{
	int column, line;
	int rubbish = 0, count = 0;

	do {
		int start, limit, incr;
		count++;
		rubbish = 0;
		if(count&1) { start=li-2; limit=0; incr = -1; }
		else { start=0; limit=li-2; incr=1; }
		for(line=start; line!=limit && !rubbish; line+=incr) {
			if(line&1)
				for(column=0; column<co && !rubbish; column++)
					rubbish += droplet(line, column);
			else
				for(column=co-1; column>=0 && !rubbish; column--)
					rubbish += droplet(line, column);
		}
	} while(rubbish);
}

dropf(file)
char *file;
{
	FILE *fp;

	if((fp = fopen(file, "r")) == NULL) {
		perror(file);
		return;
	}
	fdropf(fp);
}

fdropf(fp)
FILE *fp;
{
	int i;

	while(!feof(fp)) {
		readscreen(fp);
		drawscreen();
		for(i=0; i<20; i++)
			droplet((rand()>>4) % li, (rand()>>4) % co);
		dropscreen();
	}
}

outs(s)
char *s;
{
	fputs(s, stdout);
}
