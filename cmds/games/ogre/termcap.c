#include <sgtty.h>


/*
    Interface to termcap library.
*/

char    *BC, *UP;
char    *eeolseq, *cmseq;
char    *clearseq;
short   ospeed;
int     putchar();

tc_setup() {

    static  char    bp[1024];
    static  char    buffer[1024];
    char    *area = buffer;
    char    *getenv();
    char    *tgetstr();
    char    *name;
    int     retcode;

    name = getenv("TERM");

    retcode = tgetent(bp, name);

    switch(retcode) {

        case -1:
            printf("can't open termcap file.\n");
            exit(1);
            break;

        case 0:
            printf("No termcap entry for %s.\n", name);
            exit(1);
            break;

    }

    eeolseq = tgetstr("ce", &area);
    cmseq   = tgetstr("cm", &area);
    clearseq = tgetstr("cl", &area);
    BC   = tgetstr("bc", &area);
    UP   = tgetstr("up", &area);

}

eeol() {

    tputs(eeolseq, 0, putchar);

}

clear_screen() {

    tputs(clearseq, 0, putchar);

}

movecur(row, col)
int row, col;
{
    char *tgoto() ;

    tputs(tgoto(cmseq, col, row), 0, putchar);

}

#ifndef __HELIOS

struct sgttyb old_term;
struct sgttyb new_term;

/*
    Set terminal to CBREAK and NOECHO.
*/
set_term() {
    static int  first = 1;

    if(first) {
        gtty(0, &old_term);
        gtty(0, &new_term);

        new_term.sg_flags &= ~(ECHO | XTABS); /* | CRMOD); */
        new_term.sg_flags |= CBREAK;
	
	ospeed = new_term.sg_ospeed;

        first = 0;
    }

    stty(0, &new_term);

}

/*
    Reset the terminal to normal mode.
*/
reset_term() {

    stty(0, &old_term);

}

#else

#include <stdio.h>
#include <attrib.h>
#include <syslib.h>
#include <nonansi.h>

Attributes old_term;
Attributes new_term;

set_term() {
  static int first = 1;

  if (first)
   { setvbuf(stdin, NULL, _IONBF, 0);
     GetAttributes(Heliosno(stdin), &old_term);
     GetAttributes(Heliosno(stdin), &new_term);
     AddAttribute(&new_term, ConsoleRawInput);
     RemoveAttribute(&new_term, ConsoleEcho);
     first = 0;
   }

  SetAttributes(Heliosno(stdin), &new_term);
}

reset_term() {
  SetAttributes(Heliosno(stdin), &old_term);
}

#endif

