/***************************************************************************
 ***************************************************************************
 **	SPLIT COMMAND for HELIOS                                          **
 **	John Fitch - University of Bath  				  **
 **	2 March 1988	                                                  **
 ***************************************************************************
 ***************************************************************************/

/* Revision history:
 * Written from scratch and 
 * Helios-ized by John Fitch 26 April 1988
 */
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/textutil/RCS/split.c,v 1.4 1994/05/12 13:39:53 nickc Exp $";
#endif
#include <stdlib.h>
#include <stdio.h>
#include <syslib.h>
#include <string.h>

int split_size = 1000;


void give_help()
{
  printf("Syntax: split [-n] file [name]\n");
}

FILE * open_file(
		 char *name,
		 int number )
{
  char newname[256];
  int i;
  FILE *ans;

#ifdef DEBUG
  fprintf(stderr,"Open called with %s %d\n",name,number);
#endif
  strcpy(newname,name);
  i = strlen(name);
  newname[i++] = ('a'+(number/26));
  newname[i++] = ('a'+(number%26));
  newname[i] = '\0';
#ifdef DEBUG
  fprintf(stderr,"Opening file %s\n",newname);
#endif
  ans = fopen(newname,"w");
  if (ans == NULL) {
    fprintf(stderr, "split: Cannot open output file\n");
    exit(1);
  }
  return(ans);
}

int main(
	 int argc,
	 char **argv )
{
  FILE *inf;
  FILE *outf;
  char *split_name;
  int file_number;
  int ch;
  int line_number = 0;

#ifdef DEBUG
  fprintf(stderr,"argc=%d\n",argc);
#endif
  if (argc>4) {
    give_help();
    exit(1);
  }
  argv++;
  if ((*argv)[0] == '-' && (*argv)[1] != '\0') {
				/* Set new split rate */
    sscanf(*argv,"-%d",&split_size);
#ifdef DEBUG
    fprintf(stderr,"Splitsize reset to %d\n",split_size);
#endif
    argv++;
    argc--;
  }
  if (argc != 2 && argc != 3) {
    give_help();
    exit(1);
  }
  if ((*argv)[0] == '-' && (*argv)[1] == '\0') {
				/* Take current input */
    inf = stdin;
  }
  else {
    inf = fopen(*argv,"r");
    if (inf == NULL) {
      fprintf(stderr, "split: Cannot open input\n");
      exit(1);
    }
  }
  if (argc == 3) {
				/* set split name */
    split_name = *(++argv);
  }
  else split_name = "x";
#ifdef DEBUG
    fprintf(stderr,"Split name reset to %s\n",split_name);
#endif

  file_number = 0;
  outf = open_file(split_name,file_number);
  while ((ch = getc(inf)) != EOF) {
    putc(ch,outf);
    if (ch == '\n') {
      line_number++;
      if (line_number == split_size) {
	fclose(outf);
	ch = getc(inf);
	if (ch != EOF) outf=open_file(split_name,++file_number);
	ungetc(ch,inf);
	line_number = 0;
      }
    }
  }
  fclose(outf);
  exit(0);
}
