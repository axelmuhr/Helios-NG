/* $Header: /dsl/HeliosRoot/Helios/cmds/public/stevie/RCS/cmdline.c,v 1.1 1993/08/06 15:17:14 nickc Exp tony $
 *
 * Routines to parse and execute "command line" commands, such as searches
 * or colon commands.
 */

#include "stevie.h"

static	char	*altfile = NULL;	/* alternate file */
static	int	altline;		/* line # in alternate file */

static	char	*nowrtmsg = "No write since last change (use ! to override)";
static	char	*nooutfile = "No output file";
static	char	*morefiles = "more files to edit";

extern	char	**files;		/* used for "n" and "rew" */
extern	int	numfiles, curfile;

#define	CMDSZ	100		/* size of the command buffer */

static	void	get_range();
static	LPTR	*get_line();

/*
 * getcmdln() - read a command line from the terminal
 *
 * Reads a command line started by typing '/', '?', '!', or ':'. Returns a
 * pointer to the string that was read. For searches, an optional trailing
 * '/' or '?' is removed.
 */
char *
getcmdln(firstc)
char	firstc;
{
	static	char	buff[CMDSZ];
	register char	*p = buff;
	register int	c;
	register char	*q;

	gotocmd(TRUE, firstc);

	/* collect the command string, handling '\b' and @ */
	do {
		switch (c = vgetc()) {

		default:		/* a normal character */
			outchar(c);
			*p++ = c;
			break;

		case BS:
			if (p > buff) {
				/*
				 * this is gross, but it relies
				 * only on 'gotocmd'
				 */
				p--;
				gotocmd(TRUE, firstc);
				for (q = buff; q < p ;q++)
					outchar(*q);
			} else {
				msg("");
				return NULL;		/* back to cmd mode */
			}
			break;

		case '@':			/* line kill */
			p = buff;
			gotocmd(TRUE, firstc);
			break;

		case ESC:			/* abandon command */
			msg("");
			return  NULL;
			break;

		case NL:			/* done reading the line */
		case CR:
			break;
		}
	} while (c != NL && c != CR);

	*p = '\0';

	if (firstc == '/' || firstc == '?') {	/* did we do a search? */
		/*
		 * Look for a terminating '/' or '?'. This will be the first
		 * one that isn't quoted. Truncate the search string there.
		 */
		for (p = buff; *p ;) {
			if (*p == firstc) {	/* we're done */
				*p = '\0';
				break;
			} else if (*p == '\\')	/* next char quoted */
				p += 2;
			else
				p++;		/* normal char */
		}
	}
	return buff;
}

/*
 * docmdln() - handle a colon command
 *
 * Handles a colon command received interactively by getcmdln() or from
 * the environment variable "EXINIT" (or eventually .virc).
 */
void
docmdln(cmdline)
char	*cmdline;
{
	char	buff[CMDSZ];
	char	cmdbuf[CMDSZ];
	char	argbuf[CMDSZ];
	char	*cmd, *arg;
	register char	*p;
	/*
	 * The next two variables contain the bounds of any range given in a
	 * command. If no range was given, both contain null line pointers.
	 * If only a single line was given, u_pos will contain a null line
	 * pointer.
	 */
	LPTR	l_pos, u_pos;


	/*
	 * Clear the range variables.
	 */
	l_pos.linep = (struct line *) NULL;
	u_pos.linep = (struct line *) NULL;

	if (cmdline == NULL)
		return;

	if (strlen(cmdline) > CMDSZ-2) {
		msg("Error: command line too long");
		return;
	}
	strcpy(buff, cmdline);

	/* skip any initial white space */
	for (cmd = buff; *cmd != NUL && isspace(*cmd) ;cmd++)
		;

	if (*cmd == '%') {		/* change '%' to "1,$" */
		strcpy(cmdbuf, "1,$");	/* kind of gross... */
		strcat(cmdbuf, cmd+1);
		strcpy(cmd, cmdbuf);
	}

	while ((p=strchr(cmd, '%')) != NULL && *(p-1) != '\\') {
					/* change '%' to Filename */
		if (Filename == NULL) {
			emsg("No filename");
			return;
		}
		*p= NUL;
		strcpy (cmdbuf, cmd);
		strcat (cmdbuf, Filename);
		strcat (cmdbuf, p+1);
		strcpy(cmd, cmdbuf);
		msg(cmd);			/*repeat */
	}

	while ((p=strchr(cmd, '#')) != NULL && *(p-1) != '\\') {
					/* change '#' to Altname */
		if (altfile == NULL) {
			emsg("No alternate file");
			return;
		}
		*p= NUL;
		strcpy (cmdbuf, cmd);
		strcat (cmdbuf, altfile);
		strcat (cmdbuf, p+1);
		strcpy(cmd, cmdbuf);
		msg(cmd);			/*repeat */
	}

	/*
	 * Parse a range, if present (and update the cmd pointer).
	 */
	get_range(&cmd, &l_pos, &u_pos);

	if (l_pos.linep != NULL) {
		if (LINEOF(&l_pos) > LINEOF(&u_pos)) {
			emsg("Invalid range");
			return;
		}
	}

	strcpy(cmdbuf, cmd);	/* save the unmodified command */

	/* isolate the command and find any argument */
	for ( p=cmd; *p != NUL && ! isspace(*p); p++ )
		;
	if ( *p == NUL )
		arg = NULL;
	else {
		*p = NUL;
		for (p++; *p != NUL && isspace(*p) ;p++)
			;
		if (*p == NUL)
			arg = NULL;
		else {
			strcpy(argbuf, p);
			arg = argbuf;
		}
	}
	if (strcmp(cmd,"q!") == 0)
		getout();
	if (strcmp(cmd,"q") == 0) {
		if (Changed)
			emsg(nowrtmsg);
		else {
			if ((curfile + 1) < numfiles)
				emsg(morefiles);
			else
				getout();
		}
		return;
	}
	if (strcmp(cmd,"w") == 0) {
		if (arg == NULL) {
			if (Filename != NULL) {
				writeit(Filename, &l_pos, &u_pos);
			} else
				emsg(nooutfile);
		}
		else
			writeit(arg, &l_pos, &u_pos);
		return;
	}
	if (strcmp(cmd,"wq") == 0) {
		if (Filename != NULL) {
			if (writeit(Filename, (LPTR *)NULL, (LPTR *)NULL))
				getout();
		} else
			emsg(nooutfile);
		return;
	}
	if (strcmp(cmd, "x") == 0) {
		doxit();
		return;
	}

	if (strcmp(cmd,"f") == 0 && arg == NULL) {
		fileinfo();
		return;
	}
	if (*cmd == 'n') {
		if ((curfile + 1) < numfiles) {
			/*
			 * stuff ":e[!] FILE\n"
			 */
			stuffin(":e");
			if (cmd[1] == '!')
				stuffin("!");
			stuffin(" ");
			stuffin(files[++curfile]);
			stuffin("\n");
		} else
			emsg("No more files!");
		return;
	}
	if (*cmd == 'N') {
		if (curfile > 0) {
			/*
			 * stuff ":e[!] FILE\n"
			 */
			stuffin(":e");
			if (cmd[1] == '!')
				stuffin("!");
			stuffin(" ");
			stuffin(files[--curfile]);
			stuffin("\n");
		} else
			emsg("No more files!");
		return;
	}
	if (strncmp(cmd, "rew", 3) == 0) {
		if (numfiles <= 1)		/* nothing to rewind */
			return;
		curfile = 0;
		/*
		 * stuff ":e[!] FILE\n"
		 */
		stuffin(":e");
		if (cmd[3] == '!')
			stuffin("!");
		stuffin(" ");
		stuffin(files[0]);
		stuffin("\n");
		return;
	}
	if (strcmp(cmd,"e") == 0 || strcmp(cmd,"e!") == 0) {
		(void) doecmd(arg, cmd[1] == '!');
		return;
	}
	/*
	 * The command ":e#" gets expanded to something like ":efile", so
	 * detect that case here.
	 */
	if (*cmd == 'e' && arg == NULL) {
		if (cmd[1] == '!')
			(void) doecmd(&cmd[2], TRUE);
		else
			(void) doecmd(&cmd[1], FALSE);
		return;
	}
	if (strcmp(cmd,"f") == 0) {
		EnvEval (arg, CMDSZ);	/* expand environment vars */
		Filename = strsave(arg);
		filemess("");
		return;
	}
	if (strcmp(cmd,"r") == 0) {
		if (arg == NULL) {
			badcmd();
			return;
		}
		if (readfile(arg, Curschar, 1)) {
			emsg("Can't open file");
			return;
		}
		updatescreen();
		CHANGED;
		return;
	}
	if (strcmp(cmd,"=") == 0) {
		smsg("%d", cntllines(Filemem, &l_pos));
		return;
	}
	if (strncmp(cmd,"ta", 2) == 0) {
		dotag(arg, cmd[2] == '!');
		return;
	}
	if (strncmp(cmd,"untag", 5) == 0) {
		if (P(P_TG))
			dountag(cmd[5]);
		else
			emsg("Tag stacking not enabled");
		return;
	}
	if (strncmp(cmd,"set", 2) == 0) {
		doset(arg);
		return;
	}
	if (strcmp(cmd,"help") == 0) {
		if (help()) {
			screenclear();
			updatescreen();
		}
		return;
	}
	if (strncmp(cmd, "ve", 2) == 0) {
		extern	char	*Version;

		msg(Version);
		return;
	}
	if (strcmp(cmd, "sh") == 0) {
		doshell(NULL);
		return;
	}
	if (*cmd == '!') {
		doshell(cmdbuf+1);
		return;
	}
	if (strncmp(cmd, "s/", 2) == 0) {
		dosub(&l_pos, &u_pos, cmdbuf+1);
		return;
	}
	if (strncmp(cmd, "g/", 2) == 0) {
		doglob(&l_pos, &u_pos, cmdbuf+1);
		return;
	}
	/*
	 * If we got a line, but no command, then go to the line.
	 */
	if (*cmd == NUL && l_pos.linep != NULL) {
		*Curschar = l_pos;
		return;
	}

	badcmd();
}


doxit()
{
	if (Changed) {
		if (Filename != NULL) {
			if (!writeit(Filename, (LPTR *)NULL, (LPTR *)NULL))
				return;
		} else {
			emsg(nooutfile);
			return;
		}
	}
	if ((curfile + 1) < numfiles)
		emsg(morefiles);
	else
		getout();
}

/*
 * get_range - parse a range specifier
 *
 * Ranges are of the form:
 *
 * addr[,addr]
 *
 * where 'addr' is:
 *
 * $  [+- NUM]
 * 'x [+- NUM]	(where x denotes a currently defined mark)
 * .  [+- NUM]
 * NUM
 *
 * The pointer *cp is updated to point to the first character following
 * the range spec. If an initial address is found, but no second, the
 * upper bound is equal to the lower.
 */
static void
get_range(cp, lower, upper)
register char	**cp;
LPTR	*lower, *upper;
{
	register LPTR	*l;
	register char	*p;

	if ((l = get_line(cp)) == NULL)
		return;

	*lower = *l;

	for (p = *cp; *p != NUL && isspace(*p) ;p++)
		;

	*cp = p;

	if (*p != ',') {		/* is there another line spec ? */
		*upper = *lower;
		return;
	}

	*cp = ++p;

	if ((l = get_line(cp)) == NULL) {
		*upper = *lower;
		return;
	}

	*upper = *l;
}

static LPTR *
get_line(cp)
char	**cp;
{
	static	LPTR	pos;
	LPTR	*lp;
	register char	*p, c;
	register int	lnum;

	pos.index = 0;		/* shouldn't matter... check back later */

	p = *cp;
	/*
	 * Determine the basic form, if present.
	 */
	switch (c = *p++) {

	case '$':
		pos.linep = Fileend->linep->prev;
		break;

	case '.':
		pos.linep = Curschar->linep;
		break;

	case '\'':
		if ((lp = getmark(*p++)) == NULL) {
			emsg("Unknown mark");
			return (LPTR *) NULL;
		}
		pos = *lp;
		break;

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		for (lnum = c - '0'; isdigit(*p) ;p++)
			lnum = (lnum * 10) + (*p - '0');

		pos = *gotoline(lnum);
		break;

	default:
		return (LPTR *) NULL;
	}

	while (*p != NUL && isspace(*p))
		p++;

	if (*p == '-' || *p == '+') {
		bool_t	neg = (*p++ == '-');

		for (lnum = 0; isdigit(*p) ;p++)
			lnum = (lnum * 10) + (*p - '0');

		if (neg)
			lnum = -lnum;

		pos = *gotoline( cntllines(Filemem, &pos) + lnum );
	}

	*cp = p;
	return &pos;
}

void
badcmd()
{
	emsg("Unrecognized command");
}

bool_t
doecmd(arg, force)
char	*arg;
bool_t	force;
{
	int	line = 1;		/* line # to go to in new file */

	if (!force && Changed) {
		emsg(nowrtmsg);
		if (altfile)
			free(altfile);
		altfile = strsave(arg);
		return FALSE;
	}
	if (arg != NULL) {
		/*
		 * First detect a ":e" on the current file. This is mainly
		 * for ":ta" commands where the destination is within the
		 * current file.
		 */
		if (Filename != NULL && strcmp(arg, Filename) == 0) {
			if (!Changed || (Changed && !force))
				return TRUE;
		}
		if (altfile) {
			if (strcmp (arg, altfile) == 0)
				line = altline;
			free(altfile);
		}
		altfile = Filename;
		altline = cntllines(Filemem, Curschar);
		Filename = strsave(arg);
	}
	if (Filename == NULL) {
		emsg("No filename");
		return FALSE;
	}

	/* clear mem and read file */
	freeall();
	filealloc();
	UNCHANGED;

	if (readfile(Filename, Filemem, 0))
		filemess("[New File]");

	*Topchar = *Curschar;
	if (line != 1) {
		stuffnum(line);
		stuffin("G");
	}
	do_mlines();
	setpcmark();
	updatescreen();
	return TRUE;
}

void
gotocmd(clr, firstc)
bool_t  clr;
char	firstc;
{
	windgoto(Rows-1,0);
	if (clr)
		CLEOL;		/* clear the bottom line */
	if (firstc)
		outchar(firstc);
}

/*
 * msg(s) - displays the string 's' on the status line
 */
void
msg(s)
char	*s;
{
	gotocmd(TRUE, 0);
	outstr(s);
	flushbuf();
}

/*VARARGS1*/
void
smsg(s, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16)
char	*s;
int	a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16;
{
	char	sbuf[80];

	sprintf(sbuf, s,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16);
	msg(sbuf);
}

/*
 * emsg() - display an error message
 *
 * Rings the bell, if appropriate, and calls message() to do the real work
 */
void
emsg(s)
char	*s;
{
	if (P(P_EB))
		beep();
	msg(s);
}

void
wait_return()
{
	register char	c;

	if (got_int)
		outstr("Interrupt: ");

	outstr("Press RETURN to continue");
	do {
		c = vgetc();
	} while (c != CR && c != NL && c != ' ' && c != ':');

	if (c == ':') {
		outchar(NL);
		docmdln(getcmdln(c));
	} else
		screenclear();

	updatescreen();
}
