
/* grep - search for a pattern 		Author: Martin C. Atkins */
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/textutil/RCS/egrep.c,v 1.4 1994/03/08 11:51:20 nickc Exp $";
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

/* paulh 15/8/90 modified to perform egrep task for V1.2 */

#include "regexp.h"
#include <string.h>
#include <helios.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

void match(char *, regexp *);
void pline(char *, int, char *);
int getline(char *, int);
void done(int);
void stolower ( char* );

#define MAXLINE (1024)

int status = 1;
char *progname;
int pmflag = 1;		/* print lines which match */
int pnmflag = 0;	/* print lines which don't match */
int nflag = 0;		/* number the lines printed */
int count_flag = 0;	/* number of matching lines to be printed, only */
int no_case_flag = 0;	/* relinquish case sensitivity on pattern matching */
int name_flag = 0;	/* list only the filename when a match occours */
int multiple_files = 0; /* Indicates that file name should be listed on output */

int args;
FILE *instream = stdin;

/*
 *	This routine actually matches the file
 */
void
match(
      char *name,
      regexp *exp )
{
  char buf[MAXLINE];
  char tmp_buf[MAXLINE];	/* Working copy of buffer which is not seen by user */
  int lineno = 0;
  int count_number = 0, matched = 0;
  char nochars;

  while( (nochars = getline(&buf[0],MAXLINE)) != 0) {
	char *cr = strchr(&buf[0],'\n');
	lineno++;

	if(cr == 0) {
		fprintf(stderr,"Line too long in ");
		fprintf(stderr,name == 0 ? "stdin\n":"%s\n",name);
	} else
		*cr = '\0';

	strcpy ( tmp_buf , buf );
	if ( no_case_flag )
		stolower ( tmp_buf );
	
	if( regexec(exp,&tmp_buf[0], TRUE) ) /* Bool. flag thought to enable ^ searches */ {
		if(pmflag)
			pline(name,lineno,buf);
		if(status != 2)
			status = 0;
		if (count_flag)
			count_number++;
		matched = 1;
	} 
	else if(pnmflag)
		pline(name,lineno,buf);
  }
if (count_flag)
	fprintf ( stdout , "%s : %d\n" , name , count_number );
if (name_flag && matched) {
	fprintf ( stdout , "%s\n" , name );
}
}

void regerror(char *s)
{
	fprintf(stderr,"%s: %s\n" , progname , s);
}

void
pline(
      char *name,
      int lineno,
      char buf[] )
{
  if(name && multiple_files) printf("%s:",name);
  if(nflag) printf(" %d:", lineno);
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

void stolower ( register char* str )
{
	
	while ( *str ) {
		*str = tolower ( *str );
		str++;
	}
}

void
done(int n )
{
  fflush(stdout);
  exit(n);
}

void Usage(void)
{
  fprintf(stderr,"Usage: %s [-cilnv] [-e] <pattern> [<file> ...]\n" , progname );
  done(2);
}

int
main(
     int argc,
     char *argv[] )
{
  regexp *exp;
  char **argp = &argv[1];
  char *index = (char*) malloc (255 * sizeof(char));
  
  /* setvbuf(stdin,(char *)NULL,_IONBF,0);    unbuffered  input stream 	*/
  /* setvbuf(stdout,(char *)NULL,_IONBF,0);   unbuffered output stream 	*/

  args = argc;
  progname = argv[0];
  while(*argp != 0 && argp[0][0] == '-') {
	args--;			/* flags don't count */
	strcpy (index , *argp);
	if ( strcmp ( index , "-e" ) == NULL ) {
		args--;
		argp++;
		goto out;
	}
	index++;
	while (*index != 0) {
		switch(*index) {
		case 'v':
			pmflag = 0;
			pnmflag = 1;
			break;
		case 'n':
			nflag = 1;
			break;
		case 'c':
			count_flag = 1;
			pmflag = 0;
			pnmflag = 0;
			break;
		case 'i':
			no_case_flag = 1;
			break;
		case 'l' :
			name_flag = 1;
			pmflag = 0;
			pnmflag = 0;
			break;
		default:
			Usage();
		}
		index++;
	}
	argp++;
  }
out:
  if(*argp == 0) Usage();
  args--;
  
  if ( no_case_flag ) {
  	stolower ( *argp );
  }

  if((exp = regcomp(*argp++)) == NULL) {
  	fprintf(stderr,"%s: regcomp failed\n" , progname);
  	done(2);
  }
  args--;
  
 if ( args > 1 ) {
 	multiple_files = 1;
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


