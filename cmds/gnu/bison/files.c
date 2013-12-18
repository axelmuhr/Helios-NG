/* Open and close files for bison,
   Copyright (C) 1984, 1986 Bob Corbett and Free Software Foundation, Inc.

BISON is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY.  No author or distributor accepts responsibility to anyone
for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.
Refer to the BISON General Public License for full details.

Everyone is granted permission to copy, modify and redistribute BISON,
but only under the conditions described in the BISON General Public
License.  A copy of this license is supposed to have been given to you
along with BISON so you can know your rights and responsibilities.  It
should be in a file named COPYING.  Among other things, the copyright
notice and this notice must be preserved on all copies.

 In other words, you are welcome to use, share and improve this program.
 You are forbidden to forbid anyone else to use, share and improve
 what you give them.   Help stamp out software-hoarding!  */

#include <stdio.h>
#include "files.h"
#include "new.h"
#include "gram.h"

static char *rcsid = "$Header: /dsl/HeliosRoot/Helios/cmds/gnu/bison/RCS/files.c,v 1.1 1990/08/28 11:23:18 james Exp $";

FILE *finput = NULL;
FILE *foutput = NULL;
FILE *fdefines = NULL;
FILE *ftable = NULL;
FILE *fattrs = NULL;
FILE *fguard = NULL;
FILE *faction = NULL;
FILE *fparser = NULL;
/* FILE *fparser1 = NULL; JF we don't need this anymore */

char *infile;
char *outfile;
char *defsfile;
char *tabfile;
char *attrsfile;
char *guardfile;
char *actfile;
char *tmpattrsfile;
char *tmptabfile;
/* JF nobody really uses these */
/* char *pfile;
char *pfile1; */

char   *mktemp();   /* So the compiler won't complain */
FILE   *tryopen();   /* This might be a good idea */

extern int verboseflag;
extern int definesflag;
int fixed_outfiles = 0;



char*
stringappend(string1, end1, string2)
char *string1;
int end1;
char *string2;
{
  register char *ostring;
  register char *cp, *cp1;
  register int i;

  cp = string2;  i = 0;
  while (*cp++) i++;

  ostring = NEW2(i+end1+1, char);

  cp = ostring;
  cp1 = string1;
  for (i = 0; i < end1; i++)
    *cp++ = *cp1++;

  cp1 = string2;
  while (*cp++ = *cp1++) ;

  return ostring;
}


/* JF this has been hacked to death.  Nowaday it sets up the file names for
   the output files, and opens the tmp files and the parser */
openfiles()
{
  char *name_base;
  register int i;
  register char *cp;

  name_base = fixed_outfiles ? "y.y" : infile;

  cp = name_base;
  i = 0;
  while (*cp++) i++;

  cp -= 2;

  if (*cp-- == 'y' && *cp == '.')
    i -= 2;


  finput = tryopen(infile, "r");

  fparser = tryopen(PFILE1, "r");

  if (verboseflag)
    {
      outfile = stringappend(name_base, i, ".output");
      foutput = tryopen(outfile, "w");
    }

  if (definesflag)
    {
      defsfile = stringappend(name_base, i, "-tab.h");
      fdefines = tryopen(defsfile, "w");
    }
#ifdef __STDC__
  faction = tmpfile();
  fattrs  = tmpfile();
  ftable  = tmpfile();
#else
  actfile = mktemp(stringappend("", 0, "/tmp/b.act.XXXXXX"));
  faction = tryopen(actfile, "w+");
  unlink(actfile);

  tmpattrsfile = mktemp(stringappend("", 0, "/tmp/b.attrs.XXXXXX"));
  fattrs = tryopen(tmpattrsfile,"w+");
  unlink(tmpattrsfile);

  tmptabfile = mktemp(stringappend("", 0, "/tmp/b.tab.XXXXXX"));
  ftable = tryopen(tmptabfile, "w+");
  unlink(tmptabfile);
#endif
   /* These are opened by open_extra_files, if at all */
  tabfile = stringappend(name_base, i, "-tab.c");
  attrsfile = stringappend(name_base, i, "-st.h");
  guardfile = stringappend(name_base, i, "-gu.c");

}



/* open the output files needed only for the semantic parser.
This is done when %semantic_parser is seen in the declarations section.  */
open_extra_files()
{
  FILE *ftmp;
  int c;
      /* JF change open parser file */
  fclose(fparser);
  fparser= tryopen(PFILE,"r");

      /* JF change from inline attrs file to separate one */
  ftmp = tryopen(attrsfile, "w");
  rewind(fattrs);
  while((c=getc(fattrs))!=EOF)   /* Thank god for buffering */
    putc(c,ftmp);
  fclose(fattrs);
  fattrs=ftmp;

  fguard = tryopen(guardfile, "w");

}

   /* JF to make file opening easier.  This func tries to open file
      NAME with mode MODE, and prints an error message if it fails. */
FILE *
tryopen(name, mode)
char *name;
char *mode;
{
  FILE   *ptr;

  ptr = fopen(name, mode);
  if (ptr == NULL)
    {
      fprintf(stderr, "bison: ");
      perror(name);
      done(2);
    }
  return ptr;
}

done(k)
int k;
{
  if (faction)
    fclose(faction);

  if (fattrs)
    fclose(fattrs);

  if (fguard)
    fclose(fguard);

  if (finput)
    fclose(finput);

  if (fparser)
    fclose(fparser);

/* JF we don't need this anymore
  if (fparser1)
    fclose(fparser1); */

  if (foutput)
    fclose(foutput);

   /* JF write out the output file */
  if (k == 0 && ftable)
    {
      FILE *ftmp;
      register int c;

      ftmp=tryopen(tabfile, "w");
      rewind(ftable);
      while((c=getc(ftable)) != EOF)
        putc(c,ftmp);
      fclose(ftmp);
      fclose(ftable);
    }

  exit(k);
}
