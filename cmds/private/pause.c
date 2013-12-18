/**
*
* Title:  Pause
*
* Author: Andy England
*
**/
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/private/RCS/pause.c,v 1.3 1994/03/08 12:15:10 nickc Exp $";
#endif

#include <stdio.h>

int
main(argc, argv)
int argc;
char *argv[];
{
#ifndef unix
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
#endif
  if (argc > 1) while (--argc) printf("%s ", *++argv);
  else printf("Hit any key to continue:");
  (void) getchar();
  putchar('\n');
  return 0;
}

