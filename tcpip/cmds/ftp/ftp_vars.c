#include <stdarg.h>
#include <stdio.h>
#define EXTERN
#include "ftp_var.h"

void dummy()
{
	difftime();	/* force loading of FpClib */
}

#if 0
char *getpass(char *prompt)
{
	char pw[100];
	printf(prompt); fflush(stdout);
	scanf("%s",pw);
	return pw;
}
#endif

void _doprnt(char *fmt, int *args, FILE *f)
{
	va_list a;
	
	*a = (char *)args;
	
	vfprintf(f,fmt,a);
}

#if 0
int strcasecmp(char *a, char *b)
{
    for (;;)
    {   char c1 = *a++,c2 = *b++;
    	c1 = islower(c1)?toupper(c1):c1;
    	c2 = islower(c2)?toupper(c2):c2;
        if (c1 != c2) return c1 - c2;
        if (c1 == 0) return 0;     /* no need to check c2 */
    }
}

int strncasecmp(char *a, char *b, int n)
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
#endif
