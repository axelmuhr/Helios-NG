/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __HELIOS
#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)lpc.c	5.7 (Berkeley) 11/20/88";
#endif /* not lint */
#else
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/tcpip/cmds/lpr/RCS/lpc.c,v 1.3 1994/03/17 17:02:23 nickc Exp $";
#endif
#endif

/*
 * lpc -- line printer control program
 */
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <setjmp.h>
#include <syslog.h>

#include "lpc.h"
#ifdef __HELIOS
#include "bsd.h"
#include "utils.h"
#endif

#ifndef __HELIOS
#define	GRP_OPER	6	/* gid of "operator" */
#endif

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int	fromatty;

char	cmdline[200];
int	margc;
char	*margv[20];
int	top;

#ifndef __HELIOS
int	intr();
struct	cmd *getcmd();
#else
void intr(void) ;
void cmdscanner (int) ;
struct cmd *getcmd (char *) ;
void makeargv(void) ;
#endif

extern void quit (void) ; /* cmds.c */

jmp_buf  toplevel;

int main(int argc, char *argv[])
{
  register struct cmd *c;
  extern char *name;

#ifdef __HELIOS
  {
    struct sigaction act;
    act.sa_handler = f_lock_exit;
    act.sa_mask = 0;
    act.sa_flags = SA_ASYNC;
    (void) sigaction(SIGINT, &act, NULL);
    (void) sigaction(SIGHUP, &act, NULL);
    (void) sigaction(SIGQUIT, &act, NULL);
    (void) sigaction(SIGTERM, &act, NULL);
  }
#endif

  name = argv[0];
  openlog("lpd", 0, LOG_LPR);

  if (--argc > 0) {
    c = getcmd(*++argv);
    if (c == (struct cmd *)-1) {
      printf("?Ambiguous command\n");
      exit(1);
    }
    if (c == 0) {
      printf("?Invalid command\n");
      exit(1);
    }
#ifndef __HELIOS
    if (c->c_priv && getuid() && ingroup(GRP_OPER)==0) {
      printf("?Privileged command\n");
      exit(1);
    }
#else
    debugf ("skipping privilege test") ;
#endif
    (*c->c_handler)(argc, argv);
    exit(0);
  }
  fromatty = isatty(fileno(stdin));
  top = setjmp(toplevel) == 0;
  if (top)
    signal(SIGINT, (void(*)())intr);
  for (;;) {
    cmdscanner(top);
    top = 1;
  }
}

void intr()
{
  if (!fromatty)
    exit(0);
  longjmp(toplevel, 1);
}

/*
 * Command parser.
 */
void cmdscanner (int top)
{
  register struct cmd *c;

  if (!top)
    putchar('\n');
  for (;;) {
    if (fromatty) {
      printf("lpc> ");
      fflush(stdout);
    }
    if (fgets(cmdline, sizeof(cmdline), stdin) == NULL)
      quit();
    if (cmdline[0] == '\0')
      break;
    if (cmdline[0] == '\n')
      continue;
    makeargv();
    c = getcmd(margv[0]);
    if (c == (struct cmd *)-1) {
      printf("?Ambiguous command\n");
      continue;
    }
    if (c == 0) {
      printf("?Invalid command\n");
      continue;
    }
#ifndef __HELIOS
    if (c->c_priv && getuid() && ingroup(GRP_OPER)==0) {
      printf("?Privileged command\n");
      continue;
    }
#else
    debugf ("NOT testing for privileged command") ;
#endif    
    (*c->c_handler)(margc, margv);
  }
  longjmp(toplevel, 0);
}

extern struct cmd cmdtab[];

struct cmd *getcmd (register char *name)
{
  register char *p, *q;
  register struct cmd *c, *found;
  register int nmatches, longest;

  longest = 0;
  nmatches = 0;
  found = 0;
  for (c = cmdtab; (p = c->c_name) != NULL; c++) {
    for (q = name; *q == *p++; q++)
      if (*q == 0)    /* exact match? */
        return(c);
    if (!*q) {      /* the name was a prefix */
      if (q - name > longest) {
        longest = q - name;
        nmatches = 1;
        found = c;
      } else if (q - name == longest)
        nmatches++;
    }
  }
  if (nmatches > 1)
    return((struct cmd *)-1);
  return(found);
}

/*
 * Slice a string up into argc/argv.
 */
void makeargv()
{
  register char *cp;
  register char **argp = margv;

  margc = 0;
  for (cp = cmdline; *cp;) {
    while (isspace(*cp))
      cp++;
    if (*cp == '\0')
      break;
    *argp++ = cp;
    margc += 1;
    while (*cp != '\0' && !isspace(*cp))
      cp++;
    if (*cp == '\0')
      break;
    *cp++ = '\0';
  }
  *argp++ = 0;
}

#define HELPINDENT (sizeof ("directory"))

/*
 * Help command.
 */
void help(int argc, char *argv[])
{
  register struct cmd *c;

  if (argc == 1) {
    register int i, j, w;
    int columns, width = 0, lines;
    extern int NCMDS;

    printf("Commands may be abbreviated.  Commands are:\n\n");
    for (c = cmdtab; c < &cmdtab[NCMDS - 1]; c++) {
      int len = strlen(c->c_name);

      if (len > width)
        width = len;
    }
    width = (width + 8) &~ 7;
    columns = 80 / width;
    if (columns == 0)
      columns = 1;
    lines = (NCMDS + columns - 1) / columns;
    for (i = 0; i < lines; i++) {
      for (j = 0; j < columns; j++) {
#ifndef __HELIOS
        c = cmdtab + j * lines + i;
        printf("%s", c->c_name);
#else
        int offset = j * lines + i;
        c = cmdtab + offset ;
        if (offset < NCMDS - 1)
          printf("%s", c->c_name);
#endif        
        if (c + lines >= &cmdtab[NCMDS]) {
          printf("\n");
          break;
        }
        w = strlen(c->c_name);
        while (w < width) {
          w = (w + 8) &~ 7;
          putchar('\t');
        }
      }
    }
    return;
  }
  while (--argc > 0) {
    register char *arg;
    arg = *++argv;
    c = getcmd(arg);
    if (c == (struct cmd *)-1)
      printf("?Ambiguous help command %s\n", arg);
    else if (c == (struct cmd *)0)
      printf("?Invalid help command %s\n", arg);
    else
      printf("%-*s\t%s\n", HELPINDENT,
        c->c_name, c->c_help);
  }
}

#ifndef __HELIOS
#ifdef USG

#include <pwd.h>
#include <grp.h>

ingroup(gid)
{
  char *name;
  struct passwd *pw, *getpwuid();
  struct group  *gr, *getgrgid();

  pw = getpwuid(geteuid());
  name = pw->pw_name;

  gr = getgrgid(gid);

  if (getegid() == gid)
    return (1);

  while (*(gr->gr_mem) != NULL) {
    if (!strcmp(*(gr->gr_mem), pw->pw_name))
      return (1);
    *(gr->gr_mem)++;
  }

  return 0;
}

#else

#include <sys/param.h>

ingroup(gid)
{
  int groups[NGROUPS];
  register int i;

  if (getgroups(NGROUPS, groups) < 0) {
    perror("getgroups");
    exit(1);
  }
  for (i=0; i<NGROUPS; i++)
    if (gid == groups[i])
      return(1);
  return(0);
}

#endif
#endif

