
#include <stdio.h>

#define bufsize 30000
char buf[bufsize];

main(argc,argv)
int argc;
char **argv;
{
	argv++;


	while( *argv != NULL )
	{
		printf("%s...",*argv);
		decr(*argv++);
		printf("done\n");
	}
}

decr(name)
char *name;
{
	FILE *fd = fopen(name,"rb");
	int c;
	int crseen;
	int bufmax;
	int i = 0;

	if( fd == NULL )
	{
		printf("failed ot open %s\n",name);
		return;
	}

	bufmax = fread(buf,1,bufsize,fd);

	fclose(fd);

	fd = fopen(name,"w");

	for( i = 0; i < bufmax ; i++ )
	{
		c = buf[i];
		
		switch( c )
		{
		default:
			putc(c,fd); break;

		case 0x0d:
			crseen = 1; break;

		case 0x0a:
/*			putc(0x0d,fd); */
			putc(0x0a,fd);
			break;	
		}
	}	

	fclose(fd);
}
