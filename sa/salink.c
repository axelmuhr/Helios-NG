
#include <stdarg.h>
#include <stdio.h>
#include <posix.h>
#include <fcntl.h>
#include <module.h>
#include <string.h>
#include <stdlib.h>

#define LINKMAX 20

char *pad = "/helios/lib/nboot.i";
char *linkv[LINKMAX];

static int error(char *f,...)
{
	va_list a;
	
	va_start(a,f);
	
	vfprintf(stderr,f,a);
	
	putc( '\n', stderr );
	
	exit(1);
}

#define MEMSIZE 400000l
#define swap(x) (x)

static void sysbuild(char *dest,int argc, char **argv)
{
	int i;
	WORD p;
	WORD *iwd;
	WORD sz;
	FILE *infd, *outfd;
	byte *image, *iptr;
	word isize;
	word ptabsize;

#if 1
/*printf("sysbuild ");*/
/*for(i = 0; argv[i] ; i++ ) / *printf("%s ",argv[i])*/;
printf("\n");
#endif

	argc -= 1;
	infd = outfd = 0;	
	image = 0;

	ptabsize = argc;

	image = malloc(MEMSIZE);

	if( image == 0 ) error("Cannot allocate memory");

	iptr = image + (8 + ptabsize*4);
	iwd  = (WORD *)image;
	isize = iptr - image;

	p = 0;

	while( p++ != argc  )
	{
		WORD hdr[3];
		WORD fsize;
		int s;
		WORD mn;
		char *nptr = argv[p];

		infd = fopen(nptr,"rb");

		if( infd == NULL ) 
			error("Cannot open %s for input",nptr);

		if((s=fread(hdr,1,12,infd)) != 12 )
			error("Cannot read image file header: %s %ld",nptr,s);

		/* Extract final part of name */
		nptr = nptr + strlen(nptr);
		while (*nptr != '/' && *nptr != '\\' && nptr >= argv[p]) nptr--;
		nptr++;

		mn = swap(hdr[0]);

		if(mn != Image_Magic )
			error("Bad magic number in file %s",argv[p]);
		
		fsize = swap(hdr[2]);

		if (isize+fsize > MEMSIZE ) error("Run out of memory\n");

		if((sz=fread(iptr,1,(int)fsize,infd)) != fsize)
			error("Bad header in file %s %ld %ld",argv[p],sz,fsize);

		printf("%20s: offset %8lx size %8lx\n", nptr, isize, fsize );

		/* align to next word boundary */
		fsize = (fsize+3) & (~3);

		isize += fsize;

		{
			WORD ip = iptr - image - 4*p;
			iptr += fsize;
			iwd[p] = swap(ip);
		}

		fclose(infd); infd = 0;
	}

	iwd[0] = swap(isize);
	iwd[p] = 0L;

	printf("System size = %lx\n",isize);

	outfd = fopen(dest,"wb");
	if( outfd == NULL ) 
		error("Cannot open %s for output",dest);
	if (fwrite(image,1,isize,outfd) != isize) printf("Write failed\n");

	fclose(outfd); outfd = 0;

}

void usage()
{
	error("usage: salink [-t4|8][-o dest] program");
}

int main(int argc, char **argv)
{
	char *dest = NULL;
	char *src = NULL;
	int fd;
	ImageHdr hdr;
	byte *prog;
	Module *m;
	int i;
	int linkc = 0;
	bool t4 = true;
	
	if( argc < 3 ) usage();

	for( argv++; *argv; argv++ )
	{
#define _arg_ if(*arg==0) arg = *++argv;
		char *arg = *argv;
		if( *arg == '-' ) 
		{
			arg++;
			switch( *arg++ )
			{
			case 't':
				if( *arg == '8' ) t4 = false;
				continue;
		
			case 'o':
				_arg_;
				dest = arg;
				continue;
			}
		}
		else src = arg;
	}

	if( dest == NULL || src == NULL ) usage();
	
	printf("Helios StandAlone System Builder V1.0 01/8/89\n");
	printf("(C) 1989 Copyright Perihelion Software Ltd.");
	printf(" All Rights Reserved.\n");

	/* first read the source file in */
	
	fd = open(src,O_RDONLY);

	if( read(fd,(byte *)&hdr,sizeof(ImageHdr)) != sizeof(ImageHdr))
		error("cannot read image header");
		
	prog = malloc(hdr.Size);
	
	if( prog == NULL ) error("cannot allocate memory");
	
	if( read(fd,prog,hdr.Size) != hdr.Size )
			error("cannot read program");
			

	close(fd);
	
	for( i = 0; i < LINKMAX; i++ ) linkv[i] = pad;
			
	m = (Module *)prog;
	
	while( m->Type != 0 )
	{
		if( m->Type == T_ResRef )
		{
			int id = m->Id;
			char *name = m->Name;
			char *pre = "/helios/lib/";
			char *s = malloc(strlen(m->Name)+strlen(pre)+1);
			switch( id )
			{
			case 1:	name = "kernel.sa";	break;
			case 2:	name = "syslib.sa";	break;
			case 4: pre = "/loader/";	break;
			case 5: name = t4?"fplib.t4":"fplib.t8"; break;
			case 6:	name = "posix.sa";	break;
			}
			strcpy(s,pre);
			strcat(s,name);
			linkv[id] = s;
			if( id > linkc ) linkc = id;
		}
		m = (Module *)((byte *)m+m->Size);
	}

	if( linkc == 0 )
	{
		linkv[1] = "/helios/lib/sainit";
		linkv[2] = src;
		linkv[3] = pad;
		linkv[4] = NULL;
		linkc = 4;
	}
	else
	{
		linkv[++linkc] = src;	
		linkv[++linkc] = NULL;
	}
	
	sysbuild(dest,linkc,linkv);
	
	return 0;
}
