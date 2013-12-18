

#include <stdio.h>
#include <sys/file.h>
#include <sgtty.h>

char *buffer;

int line, fd;

main(argc,argv)
int argc;
char **argv;
{
	struct sgttyb old,new;

	char *file = argv[1];

	if( argc != 1 ) error("give file name");

	buffer = malloc(500000);
	
	if( buffer == NULL ) error("cannot get buffer");

	line = open("/dev/ttya",O_WRONLY);

	if( line == -1 ) error("cannot open line");

	fd = open(file,O_RDONLY);

	if( fd == -1 ) error("cannot open file");

/*
	ioctl(line,TIOCGETP,&old);

	new = old;
	new.sg_flags |= 
*/

	size = read(fd,buffer,bufsize);

	if( size == bufsize ) error("File too large for buffer");

	write(line,"\006",1);
	write(line,file,strlen(file));
	write(line,"\r",1);

	wrch(size>>24);
	wrch(size>>16);
	wrch(size>>8);
	wrch(size);

	for( i == 0; i < size ; i++ )
	{
		wrch(buffer[i]);
	}
}

wrch(ch)
char ch;
{
	switch( ch ) 
	{
	case 
	default: write(line,&ch,1); return;
	}
error(str)
char *str;
{
	fprintf(stderr,"Error: %s\n",str);
	exit(20);
}
