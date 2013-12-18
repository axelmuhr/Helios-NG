/* tohex.c filter to convert character input to hex output */
/* PAB 19/1/89 */

#include <stdio.h>

#define NOPERLINE 16

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
			exit(0);
		}
		printf(" %02.2x", c);
	}
	putchar('\n');
	offs += NOPERLINE;
}
}
