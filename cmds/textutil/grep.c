/* grep - search for a pattern 		Author: Martin C. Atkins */
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/textutil/RCS/grep.c,v 1.4 1994/05/12 13:40:05 nickc Exp $";
#endif
/*
 *	Search files for a regular expression
 *
 *<-xtx-*>cc -o grep grep.c -lregexp
 */

/*
 *	This program was written by:
 *		Martin C. Atkins,
 *		University of York,
 *		Heslington,
 *		York. Y01 5DD
 *		England
 *	and is released into the public domain, on the condition
 *	that this comment is always included without alteration.
 *
 *	The program was modified by Andy Tanenbaum.
 */

/* Modified CHRG : 3:3:88 : Perihelion Software				     */
/* PAB 1/8/88 */

#include "regexp.h"
#include <string.h>
#include <helios.h>
#include <stdio.h>
#include <stdlib.h>

void pline(char *, int, char *);
int getline(char *, int);
void done(int);

#define MAXLINE (1024)

int status = 1;
char *progname;
int pmflag = 1;		/* print lines which match */
int pnmflag = 0;	/* print lines which don't match */
int nflag = 0;		/* number the lines printed */
int args;
FILE *instream = stdin;


void regerror(char *s )
{
  fprintf(stderr,"grep: %s\n", s);
}


/*
 *	This routine actually matches the file
 */
void
match(
      char *name,
      regexp *exp )
{
  char buf[MAXLINE];
  int lineno = 0;
  char nochars;

  while( (nochars = getline(&buf[0],MAXLINE)) != 0) {
	char *cr = strchr(&buf[0],'\n');
	lineno++;

	if(cr == 0) {
		fprintf(stderr,"Line too long in ");
		fprintf(stderr,name == 0 ? "stdin\n":"%s\n",name);
	} else
		*cr = '\0';

	if(regexec(exp,&buf[0], TRUE)) { /* TRUE believed to enable beginnning of line searches */
		if(pmflag)
			pline(name,lineno,buf);
		if(status != 2)
			status = 0;
	} 
	else if(pnmflag)
		pline(name,lineno,buf);
  }
}

void
pline(
      char *name,
      int lineno,
      char buf[] )
{
  if(name && args > 3) printf("%s:",name);
  if(nflag) printf("%d:", lineno);
  printf("%s\n",buf);
}


int
getline(
	char *buf,
	int size )
{
  char *initbuf = buf, c;

  while (1) {
	c = (char)getc(instream);
  	*(buf++) = c;
	if (c == (char)EOF)  		return(0);

  	if (buf - initbuf == size - 1) 	return((int)(buf - initbuf));

  	if (c == '\n')  		return((int)(buf - initbuf));

  }
}

void
done(int n)
{
  fflush(stdout);
  exit(n);
}


void Usage(void)
{
  fprintf(stderr,"Usage: grep [-v] [-n] [-s] [-e] < expression > [<file> ...]\n");
  done(2);
}

int
main(
     int argc,
     char *argv[] )
{
  regexp *exp;
  char **argp = &argv[1];

  /* setvbuf(stdin,(char *)NULL,_IONBF,0);    unbuffered input stream 	*/
  /* setvbuf(stdout,(char *)NULL,_IONBF,0);  unbuffered output stream 	*/

  args = argc;
  progname = argv[0];
  while(*argp != 0 && argp[0][0] == '-') {
	args--;			/* flags don't count */
	switch(argp[0][1]) {
	case 'v':
		pmflag = 0;
		pnmflag = 1;
		break;
	case 'n':
		nflag++;
		break;
	case 's':
		pmflag = pnmflag = 0;
		break;
	case 'e':
		argp++;
		goto out;
	default:
		Usage();
	}
	argp++;
  }
out:
  if(*argp == 0) Usage();

  if((exp = regcomp(*argp++)) == NULL) {
  	fprintf(stderr,"grep: regcomp failed\n");
	done(2);
  }
  if(*argp == 0) {
	instream = stdin;
	match((char *)0,exp);
  }
  else
	while(*argp != (char *)NULL) {

		if(strcmp(*argp,"-") == 0) {
			instream = stdin;
			match("-",exp);
		}
		else {
			if( (instream = fopen(*argp, "r")) == (FILE *)NULL) {
				fprintf(stderr,"Can't open ");
				fprintf(stderr,*argp);
				fprintf(stderr,"\n");
				status = 2;
			} else {
				match(*argp,exp);
				fclose(instream);
			}
		}
		argp++;
	}
  done(status);
}

