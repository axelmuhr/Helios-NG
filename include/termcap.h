/* 
 * Prototype list for functions defined in TERMACP(3X) under Helios
 * (C) Perihelion 1991
 */

#ifndef NULL
#define NULL 0
#endif

int tgetent(char *bp, char *name);
int tgetnum(char *id);
int tgetflag(char *id);
char *tgetstr(char *id, char **area);
char *tgoto(char *cm, int destcol, int destline);
void tputs(char *cp, int affcnt, int (*outc)(char c));
