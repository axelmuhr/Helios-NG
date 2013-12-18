/* $Id: string.c,v 1.5 1992/06/16 15:28:14 craig Exp $ */
#include <string.h>
#include <ctype.h>

#if 0 /* BUG 842 - bcopy() does not handle overlapping memory blocks */
extern void   bcopy(char *a,char *b,int c) { memcpy(b,a,c); }
#else
extern void   bcopy(char *a,char *b,int c) { memmove(b,a,c); }
#endif /* BUG 842 */
extern void   bzero(char *a,int c) { memset(a,0,c); }
extern int    bcmp(char *a,char *b,int c) { return memcmp(a,b,c); }
extern char * index(char *a,char b) { return strchr(a,b); }
extern char * rindex(char *a,char b) { return strrchr(a,b); }


extern int strcasecmp(char *a, char *b)
{
    for (;;)
    {   char c1 = *a++,c2 = *b++;
    	c1 = islower(c1)?toupper(c1):c1;
    	c2 = islower(c2)?toupper(c2):c2;
        if (c1 != c2) return c1 - c2;
        if (c1 == 0) return 0;     /* no need to check c2 */
    }
}

extern int strncasecmp(char *a, char *b, int n)
{
    while( n-- > 0 )
    {   char c1 = *a++,c2 = *b++;
    	c1 = islower(c1)?toupper(c1):c1;
    	c2 = islower(c2)?toupper(c2):c2;
        if (c1 != c2) return c1 - c2;
        if (c1 == 0) return 0;     /* no need to check c2 */
    }
    return 0;	
}

