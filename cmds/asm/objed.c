/****************************************************************/
/* File: objed.c                                                */
/*                                                              */
/*                                                              */
/* Author: NHG 16-May-88                                        */
/*								*/
/****************************************************************/
/* $Id: objed.c,v 1.12 1994/08/09 16:43:25 al Exp $ */

/* do NOT include asm.h */

#if defined(SUN4) || defined(RS6000) || defined (__DOS386)
#include <stdio.h>
#include "ttypes.h"
#ifndef __helios_h
#define __helios_h 1
#endif
#include <module.h>

#ifndef __DOS386
#include <sys/file.h>
#ifndef SEEK_SET
#define SEEK_SET L_SET
#endif
#endif /* __DOS386 */

extern int errno;
#else
#include <stdio.h>
#include <module.h>
#endif

extern FILE *verfd;
extern WORD verbose;

long swap(x)
long x;

#if defined(HOSTISBIGENDIAN)
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
#else
{ return (x); }
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

	if      ( progtype == T_Program ) type = "Program";
	else if ( progtype == T_Module  ) type = "Module";
	else if ( progtype == T_ResRef  ) type = "ResRef"; 
	else if ( progtype == T_Device  ) type = "Device"; 
	else type = "Unknown";

report("Image:           Type   %16s Size           %8ld",type,swap(hdr.Size));

report("Header Fields:                       Old                     New");
	
	if( info ) 
report("                 Name %18s      %18s",prog.Module.Name,name?name:"");

	if( name != 0 )
	{
		int i;
		for(i=0; i<32; i++ ) prog.Module.Name[i] = 0;
		strncpy(prog.Module.Name,name,31);
	}

	if( swap(prog.Module.Type) == T_Program )
	{
		if ( info ) fprintf(verfd,"                 Stacksize      %8ld",swap(prog.Stacksize));
		if( stacksize != -1 )
		{
			prog.Stacksize = swap(stacksize);
			if ( info ) fprintf(verfd,"                %8ld",stacksize);
		}
		if ( info ) fprintf(verfd,"\n                 Heapsize       %8ld",swap(prog.Heapsize));
		if( heapsize != -1 )
		{
			prog.Heapsize = swap(heapsize);
			if ( info ) fprintf(verfd,"                %8ld",heapsize);
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
