/* Parse command line arguments for bison,
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

static char *rcsid = "$Header: /dsl/HeliosRoot/Helios/cmds/gnu/bison/RCS/getargs.c,v 1.1 1990/08/28 11:24:51 james Exp $";

int verboseflag;
int definesflag;
extern int fixed_outfiles;/* JF */


getargs(argc, argv)
int argc;
char *argv[];
{
  register int i;
  register char *cp;
  register int duplicates;

  verboseflag = 0;
  definesflag = 0;
  duplicates = 0;

  i = 1;
  while (i < argc && *argv[i] == '-')
    {
      cp = argv[i] + 1;
      while (*cp)
	{
	  switch (*cp)
	    {
	    case 'y':/* JF this case */
	    case 'Y':
	      if(fixed_outfiles)
	        duplicates = 1;
	      else
		fixed_outfiles = 1;
	      break;

	    case 'v':
	    case 'V':
	      if (verboseflag)
		duplicates = 1;
	      else
		verboseflag = 1;
	      break;

	    case 'd':
	    case 'D':
	      if (definesflag)
		duplicates = 1;
	      else
		definesflag = 1;
	      break;

	    default:
	      fatals("illegal option:  %s", argv[i]);
	    }
	  cp++;
	}
      i++;
    }

  if (duplicates)
    fprintf(stderr, "warning: repeated arguments ignored");

  if (i == argc)
    fatal("grammar file not specified");
  else
    infile = argv[i];

  if (i < argc - 1)
    fprintf(stderr, "warning: extra arguments ignored");
}
