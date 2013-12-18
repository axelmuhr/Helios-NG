/****************************************************************/
/* File: objed.c                                                */
/*                                                              */
/*                                                              */
/* Author: NHG 16-May-88                                        */
/*								*/
/****************************************************************/
static char *SccsId = "%W%	%G% Copyright (C) Perihelion Software Ltd.";

/* do NOT include link.h */
#include <stdio.h>
#include <module.h>

extern FILE *verfd;
extern WORD verbose;

#ifdef ORION
#include <sys/file.h>
#define SEEK_SET L_SET
extern int errno;
#endif

long swap(x)
long x;
#if defined(ORION) || defined(NORCROFT)
{ return (x); }
#else
{
	long r;
	char *a = (char *)&x;
	char *b = (char *)&r;

	b[0] = a[3];
	b[1] = a[2];
	b[2] = a[1];
	b[3] = a[0];

	return r;
}
#endif

void objed(fd,name,stacksize,heapsize)
FILE *fd;
char *name;
long stacksize;
long heapsize;
{
	Program prog;
	ImageHdr hdr;
	int info = verbose;
	int size;
	long progtype;
	char *type;

	fseek(fd,0,SEEK_SET);	/* ensure we are at start of file */
	
	size = fread(&hdr,1,sizeof(ImageHdr),fd);

	if( size != sizeof(ImageHdr) )
		error("Read failure: %d",size);


	if( swap(hdr.Magic) != Image_Magic )
		error("File not object image");

	size = fread(&prog,1,sizeof(Program),fd);

	if( size != sizeof(Program) )
		error("Read failure: %d",size);

	progtype = swap(prog.Module.Type);

	if   ( progtype == T_Program ) type = "Program";
	elif ( progtype == T_Module  ) type = "Module";
	elif ( progtype == T_ResRef  ) type = "ResRef"; 

	report("Type %s Size %ld bytes",type,swap(hdr.Size));

	report(                  "                  Old        New");
	
	if( info ) fprintf(verfd,"Name       %10s",prog.Module.Name);

	if( name != 0 )
	{
		int i;
		for(i=0; i<32; i++ ) prog.Module.Name[i] = 0;
		strncpy(prog.Module.Name,name,31);
		if( info ) fprintf(verfd," %10s",prog.Module.Name);
	}
	if( info ) putc('\n',verfd);

	if( swap(prog.Module.Type) == T_Program )
	{
		if ( info ) fprintf(verfd,"Stacksize      %6ld",swap(prog.Stacksize));
		if( stacksize != -1 )
		{
			prog.Stacksize = swap(stacksize);
			if ( info ) fprintf(verfd,"     %6ld",stacksize);
		}
		if ( info ) fprintf(verfd,"\nHeapsize       %6ld",swap(prog.Heapsize));
		if( heapsize != -1 )
		{
			prog.Heapsize = swap(heapsize);
			if ( info ) fprintf(verfd,"     %6ld",heapsize);
		}
		if ( info ) putc('\n',verfd);
	}

	fseek(fd,0,SEEK_SET);	/* ensure we are at start of file */
	
	size = fwrite(&hdr,1,sizeof(ImageHdr),fd);

	if( size != sizeof(ImageHdr) )
		error("Failed to write back image header: %d %d",size,errno);

	size = fwrite(&prog,1,sizeof(Program),fd);

	if( size != sizeof(Program) )
		error("Failed to write back program header: %d %d",size,errno);

}
