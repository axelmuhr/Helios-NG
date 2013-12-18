/*
 * A safer saner malloc, for careless programmers
 * $Revision: 1.2 $
 */

#include <stdio.h>
#include <curses.h>
#include <stdlib.h>

#ifdef SYSV3
extern void free();
extern void exit();
#endif

extern void deraw( void );

void
fatal(str)
char *str;
{
    deraw();
    (void) fprintf(stderr,"%s\n", str);
    exit(1);
}

char *
xmalloc(n)
unsigned n;
{
register char *ptr;

if ((ptr = (char *)malloc(n + sizeof(double))) == NULL)
    fatal("xmalloc: no memory");
*((int *) ptr) = 12345;		/* magic number */
return(ptr + sizeof(double));
}

void
xfree(p)
char *p;
{
if (p == NULL)
    fatal("xfree: NULL");
p -= sizeof(double);
if (*((int *) p) != 12345)
    fatal("xfree: storage not malloc'ed");
free(p);
}

