/* tostr.c filter to convert binary input to readable ascii output */
/* PAB 19/1/89 */

#include <stdio.h>
#include <ctype.h>

#define NOPERLINE 70

main(argc,argv)
int argc;
char **argv;
{
int c,i,offs = 0;

for(;;)
	{
		printf("%04x: ",offs);
		for (i=0; i < NOPERLINE ; i++)
		{
			if ((c=getchar()) == EOF)
			{	
				putchar('\n');
				exit(0)	;
			}
			if (!isprint(c))
				if (c == '\n')
					c = ':';
				else if (c == 0)
					c = '.';
				else
					c = '?';
			putchar(c);
		}
		putchar('\n');
		offs += NOPERLINE;
	}	
}
