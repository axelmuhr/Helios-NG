/*	This file is for functions dealing with execution of
	commands, command lines, buffers, files and startup files

	written 1986 by Daniel Lawrence				

	$Header: /users/nickc/RTNucleus/cmds/emacs/RCS/exec.c,v 1.2 1994/03/14 16:28:32 nickc Exp $ */

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

#if	MEGAMAX & ST520
overlay	"exec"
#endif

#if	DEBUGM
char outline[NSTRING];		/* global string to hold debug line text */
#endif

/* namedcmd:	execute a named command even if it is not bound */

namedcmd(f, n)

int f, n;	/* command arguments [passed through to command executed] */

{
	register (*kfunc)();	/* ptr to the requexted function to bind to */
	int (*getname())();

	/* prompt the user to type a named command */
	mlwrite(": ");

	/* and now get the function name to execute */
	kfunc = getname();
	if (kfunc == NULL) {
		mlwrite("[No such function]");
		return(FALSE);
	}

	/* and then execute the command */
	return((*kfunc)(f, n));
}

/*	execcmd:	Execute a command line command to be typed in
			by the user					*/

execcmd(f, n)

int f, n;	/* default Flag and Numeric argument */

{
	register int status;		/* status return */
	char cmdstr[NSTRING];		/* string holding command to execute */

	/* get the line wanted */
	if ((status = mlreply(": ", cmdstr, NSTRING)) != TRUE)
		return(status);

	execlevel = 0;
	return(docmd(cmdstr));
	f = f;
	n = n;
}

/*	docmd:	take a passed string as a command line and translate
		it to be executed as a command. This function will be
		used by execute-command-line and by all source and
		startup files. Lastflag/thisflag is also updated.

	format of the command line is:

		{# arg} <command-name> {<argument string(s)>}

	Directives start with a "!" and include:

	!endm		End a macro
	!if (cond)	conditional execution
	!else
	!endif
	!return		Return (terminating current macro)
	!goto <label>	Jump to a label in the current macro

	Line Labels begin with a "*" in column 1, like:

	*LBL01
*/

docmd(cline)

char *cline;	/* command line to execute */

{
	register int f;		/* default argument flag */
	register int n;		/* numeric repeat value */
	register int i;
	int (*fnc)();		/* function to execute */
	int status;		/* return status of function */
	int oldcle;		/* old contents of clexec flag */
	int llen;		/* length of cline */
	int force;		/* force TRUE result? */
	char *tmp;		/* tmp pointer into cline */
	struct LINE *lp;	/* a line pointer */
	char *oldestr;		/* original exec string */
	char token[NSTRING];	/* next token off of command line */
	int (*fncmatch())();
#if	DEBUGM
	/* if $debug == TRUE, every line to execute
	   gets echoed and a key needs to be pressed to continue
	   ^G will abort the command */
	register char *sp;	/* pointer into buf to expand %s */

	if (macbug) {
		strcpy(outline, "<<<");
#if	1	/* debug if levels */
		strcat(outline, itoa(execlevel));
		strcat(outline, ":");
#endif
		strcat(outline, cline);
		strcat(outline, ">>>");

		/* change all '%' to ':' so mlwrite won't expect arguments */
		sp = outline;
		while (*sp) {
			if (*sp++ == '%')
				*(sp-1) = ':';
		}

		/* write out the debug line */
		mlwrite(outline);
		update(TRUE);

		/* and get the keystroke */
		if (tgetc() == 7) {
			mlwrite("[Macro aborted]");
			return(FALSE);
		}
	}
#endif
		
	/* dump comments here */
	if (*cline == ';')
		return(TRUE);

	/* eat leading spaces */
	while (*cline == ' ' || *cline == '\t')
		++cline;

	/* check to see if this line turns macro storage off */
	if (cline[0] == '!' && strncmp(&cline[1], "endm", 4) == 0) {
		mstore = FALSE;
		bstore = NULL;
		return(TRUE);
	}

	/* if macro store is on, just salt this away */
	if (mstore) {
		/* allocate the space for the line */
		llen = strlen(cline);
		if ((lp=lalloc(llen)) == NULL) {
			mlwrite("Out of memory while storing macro");
			return (FALSE);
		}

		/* copy the text into the new line */
		for (i=0; i<llen; ++i)
			lputc(lp, i, cline[i]);

		/* attach the line to the end of the buffer */
       		bstore->b_linep->l_bp->l_fp = lp;
		lp->l_bp = bstore->b_linep->l_bp;
		bstore->b_linep->l_bp = lp;
		lp->l_fp = bstore->b_linep;
		return (TRUE);
	}
	
	/* dump labels here */
	if (*cline == '*')
		return(TRUE);

	force = FALSE;
	oldestr = execstr;	/* save last ptr to string to execute */
	execstr = cline;	/* and set this one as current */

	/* process directives */
	if (*cline == '!') {
		/* save directive location and skip it */
		tmp = cline;
		while (*execstr && *execstr != ' ' && *execstr != '\t')
			++execstr;

		if (tmp[1] == 'f' && tmp[2] == 'o') {
			force = TRUE;
			goto do001;

		} else if (tmp[1] == 'i' && tmp[2] == 'f') {

			/* IF directive */
			/* grab the value of the logical exp */
			if (execlevel == 0) {
				if ((status = macarg(token)) != TRUE) {
					execstr = oldestr;
					return(status);
				}
				status = stol(token);
			} else
				status = TRUE;

			if (status) {

				/* IF (TRUE) */
				if (execlevel != 0)
					++execlevel;
			} else {

				/* IF (FALSE) */
				++execlevel;
			}

		} else if (tmp[1] == 'e' && tmp[2] == 'l') {

			/* ELSE directive */
			if (execlevel == 1)
				--execlevel;
			else if (execlevel == 0 )
				++execlevel;

		} else if (tmp[1] == 'e' && tmp[2] == 'n') {

			/* ENDIF directive */
			if (execlevel)
				--execlevel;

		} else if (tmp[1] == 'r' && tmp[2] == 'e') {

			/* RETURN directive */
			execstr = oldestr;
			if (execlevel)
				return(TRUE);
			else
				return(RET);

		} else if (tmp[1] == 'g' && tmp[2] == 'o') {

			/* GOTO directive */
			/* .....only if we are currently executing */
			if (execlevel) {
				execstr = oldestr;
				return(TRUE);
			}

			while (*execstr == ' ' || *execstr == '\t')
				++execstr;
			strncpy(golabel, execstr, NPAT - 1);
			return(GOLINE);

		} else {
			mlwrite("%%Unknown Directive");
			return(FALSE);
		}

		/* restore execstr and exit */
		execstr = oldestr;
		return(TRUE);
	}

do001:	/* if we are scanning and not executing..go back here */
	if (execlevel) {
		execstr = oldestr;
		return(TRUE);
	}

	/* first set up the default command values */
	f = FALSE;
	n = 1;
	lastflag = thisflag;
	thisflag = 0;

	if ((status = macarg(token)) != TRUE) {	/* and grab the first token */
		execstr = oldestr;
		return(status);
	}

	/* process leadin argument */
	if (gettyp(token) != TKCMD) {
		f = TRUE;
		strcpy(token, getval(token));
		n = atoi(token);

		/* and now get the command to execute */
		if ((status = macarg(token)) != TRUE) {
			execstr = oldestr;
			return(status);	
		}	
	}

	/* and match the token to see if it exists */
	if ((fnc = fncmatch(token)) == NULL) {
		mlwrite("[No such Function]");
		execstr = oldestr;
		return(FALSE);
	}
	
	/* save the arguments and go execute the command */
	oldcle = clexec;		/* save old clexec flag */
	clexec = TRUE;			/* in cline execution */
	status = (*fnc)(f, n);		/* call the function */
	cmdstatus = status;		/* save the status */
	if (force)			/* force the status */
		status = TRUE;
	clexec = oldcle;		/* restore clexec flag */
	execstr = oldestr;
	return(status);
}

/* token:	chop a token off a string
		return a pointer past the token
*/

char *token(src, tok)

char *src, *tok;	/* source string, destination token string */

{
	register int quotef;	/* is the current string quoted? */

	/* first scan past any whitespace in the source string */
	while (*src == ' ' || *src == '\t')
		++src;

	/* scan through the source string */
	quotef = FALSE;
	while (*src) {
		/* process special characters */
		if (*src == '~') {
			++src;
			if (*src == 0)
				break;
			switch (*src++) {
				case 'r':	*tok++ = 13; break;
				case 'n':	*tok++ = 10; break;
				case 't':	*tok++ = 9;  break;
				case 'b':	*tok++ = 8;  break;
				case 'f':	*tok++ = 12; break;
				default:	*tok++ = *(src-1);
			}
		} else {
			/* check for the end of the token */
			if (quotef) {
				if (*src == '"')
					break;
			} else {
				if (*src == ' ' || *src == '\t')
					break;
			}

			/* set quote mode if qoute found */
			if (*src == '"')
				quotef = TRUE;

			/* record the character */
			*tok++ = *src++;
		}
	}

	/* terminate the token and exit */
	if (*src)
		++src;
	*tok = 0;
	return(src);
}

macarg(tok)	/* get a macro line argument */

char *tok;	/* buffer to place argument */

{
	int savcle;	/* buffer to store original clexec */
	int status;

	savcle = clexec;	/* save execution mode */
	clexec = TRUE;		/* get the argument */
	status = nextarg("", tok, NSTRING, ctoec('\n'));
	clexec = savcle;	/* restore execution mode */
	return(status);
}

/*	nextarg:	get the next argument	*/

nextarg(prompt, buffer, size, terminator)

char *prompt;		/* prompt to use if we must be interactive */
char *buffer;		/* buffer to put token into */
char *size;		/* size of the buffer */
int terminator;		/* terminating char to be used on interactive fetch */

{
	/* if we are interactive, go get it! */
	if (clexec == FALSE)
		return(getstring(prompt, buffer, size, terminator));

	/* grab token and advance past */
	execstr = token(execstr, buffer);

	/* evaluate it */
	strcpy(buffer, getval(buffer));
	return(TRUE);
}

/*	storemac:	Set up a macro buffer and flag to store all
			executed command lines there			*/

storemac(f, n)

int f;		/* default flag */
int n;		/* macro number to use */

{
	register struct BUFFER *bp;	/* pointer to macro buffer */
	char bname[NBUFN];		/* name of buffer to use */

	/* must have a numeric argument to this function */
	if (f == FALSE) {
		mlwrite("No macro specified");
		return(FALSE);
	}

	/* range check the macro number */
	if (n < 1 || n > 40) {
		mlwrite("Macro number out of range");
		return(FALSE);
	}

	/* construct the macro buffer name */
	strcpy(bname, "[Macro xx]");
	bname[7] = '0' + (n / 10);
	bname[8] = '0' + (n % 10);

	/* set up the new macro buffer */
	if ((bp = bfind(bname, TRUE, BFINVS)) == NULL) {
		mlwrite("Can not create macro");
		return(FALSE);
	}

	/* and make sure it is empty */
	bclear(bp);

	/* and set the macro store pointers to it */
	mstore = TRUE;
	bstore = bp;
	return(TRUE);
}

#if	PROC
/*	storeproc:	Set up a procedure buffer and flag to store all
			executed command lines there			*/

storeproc(f, n)

int f;		/* default flag */
int n;		/* macro number to use */

{
	register struct BUFFER *bp;	/* pointer to macro buffer */
	register int status;		/* return status */
	char bname[NBUFN];		/* name of buffer to use */

	/* a numeric argument means its a numbered macro */
	if (f == TRUE)
		return(storemac(f, n));

	/* get the name of the procedure */
        if ((status = mlreply("Procedure name: ", &bname[1], NBUFN-2)) != TRUE)
                return(status);

	/* construct the macro buffer name */
	bname[0] = '[';
	strcat(bname, "]");

	/* set up the new macro buffer */
	if ((bp = bfind(bname, TRUE, BFINVS)) == NULL) {
		mlwrite("Can not create macro");
		return(FALSE);
	}

	/* and make sure it is empty */
	bclear(bp);

	/* and set the macro store pointers to it */
	mstore = TRUE;
	bstore = bp;
	return(TRUE);
}

/*	execproc:	Execute a procedure				*/

execproc(f, n)

int f, n;	/* default flag and numeric arg */

{
        register BUFFER *bp;		/* ptr to buffer to execute */
        register int status;		/* status return */
        char bufn[NBUFN+2];		/* name of buffer to execute */

	/* find out what buffer the user wants to execute */
        if ((status = mlreply("Execute procedure: ", &bufn[1], NBUFN)) != TRUE)
                return(status);

	/* construct the buffer name */
	bufn[0] = '[';
	strcat(bufn, "]");

	/* find the pointer to that buffer */
        if ((bp=bfind(bufn, FALSE, 0)) == NULL) {
		mlwrite("No such procedure");
                return(FALSE);
        }

	/* and now execute it as asked */
	while (n-- > 0)
		if ((status = dobuf(bp)) != TRUE)
			return(status);
	return(TRUE);
	f = f;
}
#endif

/*	execbuf:	Execute the contents of a buffer of commands	*/

execbuf(f, n)

int f, n;	/* default flag and numeric arg */

{
        register BUFFER *bp;		/* ptr to buffer to execute */
        register int status;		/* status return */
        char bufn[NBUFN];		/* name of buffer to execute */

	/* find out what buffer the user wants to execute */
        if ((status = mlreply("Execute buffer: ", bufn, NBUFN)) != TRUE)
                return(status);

	/* find the pointer to that buffer */
        if ((bp=bfind(bufn, FALSE, 0)) == NULL) {
		mlwrite("No such buffer");
                return(FALSE);
        }

	/* and now execute it as asked */
	while (n-- > 0)
		if ((status = dobuf(bp)) != TRUE)
			return(status);
	return(TRUE);
	f = f;
}

/*	dobuf:	execute the contents of the buffer pointed to
		by the passed BP				*/

dobuf(bp)

BUFFER *bp;	/* buffer to execute */

{
        register int status;		/* status return */
	register LINE *lp;		/* pointer to line to execute */
	register LINE *hlp;		/* pointer to line header */
	register LINE *glp;		/* line to goto */
	register int linlen;		/* length of line to execute */
	register WINDOW *wp;		/* ptr to windows to scan */
	char *eline;			/* text of line to execute */

	/* clear IF level flags */
	execlevel = 0;

	/* starting at the beginning of the buffer */
	hlp = bp->b_linep;
	lp = hlp->l_fp;
	while (lp != hlp) {
		/* allocate eline and copy macro line to it */
		linlen = lp->l_used;
		if ((eline = (char *)malloc(linlen+1)) == NULL) {
			mlwrite("%%Out of Memory during macro execution");
			return(FALSE);
		}
		strncpy(eline, lp->l_text, linlen);
		eline[linlen] = 0;	/* make sure it ends */

		/* trim leading whitespace */
		while (eline[0] == ' ' || eline[0] == '\t')
			strcpy(eline, &eline[1]);

		/* if it is not a comment, execute it */
		if (eline[0] != 0 && eline[0] != ';') {
			status = docmd(eline);

			/* if it is a !GOTO directive, deal with it */
			if (status == GOLINE) {
				linlen = strlen(golabel);
				glp = hlp->l_fp;
				while (glp != hlp) {
					if (*glp->l_text == '*' &&
					    (strncmp(&glp->l_text[1], golabel,
					            linlen) == 0)) {
						lp = glp;
						status = TRUE;
					}
				glp = glp->l_fp;
				}
			}

			if (status == GOLINE) {
				mlwrite("%%No such label");
				return(FALSE);
			}

			/* if it is a !RETURN directive...do so */
			if (status == RET) {
				free(eline);
				break;
			}

			/* check for a command error */
			if (status != TRUE) {
				/* look if buffer is showing */
				wp = wheadp;
				while (wp != NULL) {
					if (wp->w_bufp == bp) {
						/* and point it */
						wp->w_dotp = lp;
						wp->w_doto = 0;
						wp->w_flag |= WFHARD;
					}
					wp = wp->w_wndp;
				}
				/* in any case set the buffer . */
				bp->b_dotp = lp;
				bp->b_doto = 0;
				free(eline);
				execlevel = 0;
				return(status);
			}
		}

		/* on to the next line */
		free(eline);
		lp = lp->l_fp;
	}

	/* exit the current function */
	execlevel = 0;
        return(TRUE);
}

execfile(f, n)	/* execute a series of commands in a file
*/

int f, n;	/* default flag and numeric arg to pass on to file */

{
	register int status;	/* return status of name query */
	char fname[NSTRING];	/* name of file to execute */

	if ((status = mlreply("File to execute: ", fname, NSTRING -1)) != TRUE)
		return(status);

	/* otherwise, execute it */
	while (n-- > 0)
		if ((status=dofile(fname)) != TRUE)
			return(status);

	return(TRUE);
	f = f;
}

/*	dofile:	yank a file into a buffer and execute it
		if there are no errors, delete the buffer on exit */

dofile(fname)

char *fname;	/* file name to execute */

{
	register BUFFER *bp;	/* buffer to place file to exeute */
	register BUFFER *cb;	/* temp to hold current buf while we read */
	register int status;	/* results of various calls */
	char bname[NBUFN];	/* name of buffer */

	makename(bname, fname);		/* derive the name of the buffer */
	if ((bp = bfind(bname, TRUE, 0)) == NULL) /* get the needed buffer */
		return(FALSE);

	bp->b_mode = MDVIEW;	/* mark the buffer as read only */
	cb = curbp;		/* save the old buffer */
	curbp = bp;		/* make this one current */
	/* and try to read in the file to execute */
	if ((status = readin(fname, FALSE)) != TRUE) {
		curbp = cb;	/* restore the current buffer */
		return(status);
	}

	/* go execute it! */
	curbp = cb;		/* restore the current buffer */
	if ((status = dobuf(bp)) != TRUE)
		return(status);

	/* if not displayed, remove the now unneeded buffer and exit */
	if (bp->b_nwnd == 0)
		zotbuf(bp);
	return(TRUE);
}

/*	cbuf:	Execute the contents of a numbered buffer	*/

cbuf(f, n, bufnum)

int f, n;	/* default flag and numeric arg */
int bufnum;	/* number of buffer to execute */

{
        register BUFFER *bp;		/* ptr to buffer to execute */
        register int status;		/* status return */
	static char bufname[] = "[Macro xx]";

	/* make the buffer name */
	bufname[7] = '0' + (bufnum / 10);
	bufname[8] = '0' + (bufnum % 10);

	/* find the pointer to that buffer */
        if ((bp=bfind(bufname, FALSE, 0)) == NULL) {
        	mlwrite("Macro not defined");
                return(FALSE);
        }

	/* and now execute it as asked */
	while (n-- > 0)
		if ((status = dobuf(bp)) != TRUE)
			return(status);
	return(TRUE);
	f = f;
}

cbuf1(f, n)
int f ,n;
{
	cbuf(f, n, 1);
}

cbuf2(f, n)
int f,n;
{
	cbuf(f, n, 2);
}

cbuf3(f, n)
int f,n;
{
	cbuf(f, n, 3);
}

cbuf4(f, n)
int f,n;
{
	cbuf(f, n, 4);
}

cbuf5(f, n)
int f,n;
{
	cbuf(f, n, 5);
}

cbuf6(f, n)
int f,n;
{
	cbuf(f, n, 6);
}

cbuf7(f, n)
int f,n;
{
	cbuf(f, n, 7);
}

cbuf8(f, n)
int f,n;
{
	cbuf(f, n, 8);
}

cbuf9(f, n)
int f,n;
{
	cbuf(f, n, 9);
}

cbuf10(f, n)
int f,n;
{
	cbuf(f, n, 10);
}

cbuf11(f, n)
int f,n;
{
	cbuf(f, n, 11);
}

cbuf12(f, n)
int f,n;
{
	cbuf(f, n, 12);
}

cbuf13(f, n)
int f,n;
{
	cbuf(f, n, 13);
}

cbuf14(f, n)
int f,n;
{
	cbuf(f, n, 14);
}

cbuf15(f, n)
int f,n;
{
	cbuf(f, n, 15);
}

cbuf16(f, n)
int f,n;
{
	cbuf(f, n, 16);
}

cbuf17(f, n)
int f,n;
{
	cbuf(f, n, 17);
}

cbuf18(f, n)
int f,n;
{
	cbuf(f, n, 18);
}

cbuf19(f, n)
int f,n;
{
	cbuf(f, n, 19);
}

cbuf20(f, n)
int f,n;
{
	cbuf(f, n, 20);
}

cbuf21(f, n)
int f,n;
{
	cbuf(f, n, 21);
}

cbuf22(f, n)
int f,n;
{
	cbuf(f, n, 22);
}

cbuf23(f, n)
int f,n;
{
	cbuf(f, n, 23);
}

cbuf24(f, n)
int f,n;
{
	cbuf(f, n, 24);
}

cbuf25(f, n)
int f,n;
{
	cbuf(f, n, 25);
}

cbuf26(f, n)
int f,n;
{
	cbuf(f, n, 26);
}

cbuf27(f, n)
int f,n;
{
	cbuf(f, n, 27);
}

cbuf28(f, n)
int f,n;
{
	cbuf(f, n, 28);
}

cbuf29(f, n)
int f,n;
{
	cbuf(f, n, 29);
}

cbuf30(f, n)
int f,n;
{
	cbuf(f, n, 30);
}

cbuf31(f, n)
int f,n;
{
	cbuf(f, n, 31);
}

cbuf32(f, n)
int f,n;
{
	cbuf(f, n, 32);
}

cbuf33(f, n)
int f,n;
{
	cbuf(f, n, 33);
}

cbuf34(f, n)
int f,n;
{
	cbuf(f, n, 34);
}

cbuf35(f, n)
int f,n;
{
	cbuf(f, n, 35);
}

cbuf36(f, n)
int f,n;
{
	cbuf(f, n, 36);
}

cbuf37(f, n)
int f,n;
{
	cbuf(f, n, 37);
}

cbuf38(f, n)
int f,n;
{
	cbuf(f, n, 38);
}

cbuf39(f, n)
int f,n;
{
	cbuf(f, n, 39);
}

cbuf40(f, n)
int f,n;
{
	cbuf(f, n, 40);
}


