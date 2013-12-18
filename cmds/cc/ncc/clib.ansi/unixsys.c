
/* unixsys.c:  Copyright (C) Codemist Ltd. */
/* This file is a first stab at the stdio->unix level support routines.    */

/* version 1 */

#include "hostsys.h"                            /* things like _initio() */
#include <stdio.h>                              /* for EOF               */
#include <stdlib.h>                             /* for exit()            */
#include <ctype.h>                              /* for isprint()         */
#include <string.h>                             /* for strlen()          */
#include <time.h>                               /* for clock             */

extern int main(int argc, char **argv);         /* the point of it all   */

/* timing things... */

static clock_t _time0;

static clock_t _clock()
{   struct bbctime bt;
    _bbctime(&bt);
    return bt.l;
}

static void _clock_init()   /* private - for initialisation */
{   _time0 = _clock();
}

/* Exported... */

clock_t clock()
{   return _clock() - _time0;     /* clock runs even if date not set */
}

time_t time(time_t *timer)
/* this version gives the UNIX result of secs since 1-Jan-1970 */
{ time_t result;
  result = -1;
  /* /* more code to insert here */
  if (timer) *timer = result;
  return result;
}

/* system dependent I/O routines ... */

static int _error_recursion;
void _sys_msg(const char *s)
{   /* write out s carefully for intimate system use.                      */
    if ((stderr->_flag & _IOWRITE) && !_error_recursion)
    {   _error_recursion = 1;
        fprintf(stderr, "\n%s\n", s);
        _error_recursion = 0;
    }
    else
    {   _ttywrite((unsigned char *)"\n", 1, 0);
        _ttywrite((unsigned char *)s, strlen(s), 0);
        _ttywrite((unsigned char *)"\n", 1, 0);
    }
}

#define LF '\n'

FILEHANDLE _sys_open(const char *filename, int openmode)
{   /* nasty magic number interface for openmode */
    /* openmode coded as: r=0, w=4, a=8 with options b=1, '+'=2 */
    return NONHANDLE;
}

int remove(const char *pathname)
{
    if (_osfile(6, pathname, 0, 0, 0, 0) == 0) return 1;
    return 0;
}

int rename(const char *old, const char *new)
{
    char s[255];
    if (strlen(old)+strlen(new) >= 255-8) return 1;  /* tough */
    strcpy(s, "rename ");
    strcpy(&s[7], old);
    strcat(&s[7], " ");
    strcat(&s[8], new);
    if (_oscli(s) == 0) return 1;
    return 0;
}

char *getenv(const char *name)
{
/* This is pretty unsatisfactory, but I do not know what more to do!     */
    name = name;  /* stop warning for pointless procedure */
    return NULL;
}

int system(const char *string)
{
    if (string==NULL) return 0;
    else return _oscli(string);
}

static void *top_of_stack;          /* needs rework for stack extension */

static void *_codebase, *_codelimit; /* For profile option */
extern void _count(void), _count1(void);

void _main(int unixargc, char *unixargv)
{   char ch;
    int argc = 0, i = 0, curarg = 0;
    char *stdinfile  = TTYFILENAME,
         *stdoutfile = TTYFILENAME,
         *stderrfile = TTYFILENAME;
    _error_recursion = 0;
    top_of_stack = topofstack;
    _codebase = codebase;
    _codelimit = codelimit;
    _exit_init();           /* must happen before exit() can be called   */
    _signal_init();         /* had better be done pretty early           */
    _clock_init();          /* set Cpu time zero point                   */
    _initalloc(endofloadedfile, baseofstack);   /* as had the allocator  */
/* SIGINT events are not safe until about now.                           */
    _raise_stacked_interrupts();       /* enable SIGINT                  */
    _initio(stdinfile, stdoutfile, stderrfile);
    exit(main(argc, argv));
}

void _mapstore()
{
    fprintf(stderr, "_mapstore() not implemented\n");
}

void _write_profile(char *filename)
{
    fprintf(stderr, "_write_profile() not implemented\n");
}

void _backtrace(int why, int *address, int *fp, int *sp, int *pc)
{
    /* all the messages in the following should go to stderr             */
    if (why==-1)
    {   fprintf(stderr, "\nPostmortem requested\n");
    }
    else if (why==1)
    {   fprintf(stderr, "\nIllegal write to address %p = %d (pc=%p)\n",
               address, (int)address, pc);
    }
    else fprintf(stderr, "\nIllegal read from address %p = %d (pc=%p)\n",
                 address, (int)address, pc);
/* Now unwind the stack -- currently not implemented as hardware specific. */
    fprintf(stderr, "Stack backtrace not yet implemented\n");
    exit(1);
}

/* end of unixsys.c */
