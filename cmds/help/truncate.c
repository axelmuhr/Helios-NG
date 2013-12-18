#include <stdio.h>

#ifdef __STDC__
extern unsigned char *stem(unsigned char *p);
#else
extern unsigned char *stem();
#endif

int main(argc, argv)
int argc; char **argv;
{
	while(--argc > 0)
		printf("%s\n", stem((unsigned char *)*++argv));
}
