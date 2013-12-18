#include <stdio.h>
main()
{
	FILE *f,*f1;
	int i;
	char c;

	if((f = fopen("/dos/file1", "w+")) == NULL)
		{
		printf("couldn't open file1\n");
		exit(-1);
		}
	if((f1 = fopen("/dos/file2", "w+")) == NULL)
		{
		printf("couldn't open file2\n");
		exit(-1);
		}

	for(i = 0 ; i < 26 ; i++)
		{
		fseek(f, (long) i, SEEK_SET);
		fputc('a'+i, f);
		fseek(f, (long) (26-i-1), SEEK_SET);
		fputc('A'+i, f);
		fseek(f1, (long) (26-i-1), SEEK_SET);
		fputc('a'+i, f1);
		fseek(f1, (long) i, SEEK_SET);
		fputc('A'+i, f1);
		}

	putchar('\n');
}
		
