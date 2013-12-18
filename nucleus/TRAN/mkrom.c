

#include <stdio.h>
#include <module.h>
#include <queue.h>
#include <syslib.h>
#include <nonansi.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define	ADD		1
#define DEL		2
#define	REP		3

typedef struct FileData {
	word		Size;
	char		Name[NameMax];
} FileData;

typedef struct FileInfo {
	Node		Node;
	FileData	FileData;
	bool		InRom;
	void		*Data;
} FileInfo;

Module *romfile;
Module *files;
word romsize;

List filelist;

bool changes = FALSE;

char *openmode = "rb";

long fsize(FILE *f)
{
#if 0
	long size;
	long pos = ftell(f);
	fseek(f,SEEK_END,0);
	size = ftell(f);
	fseek(f,SEEK_SET,pos);
	return size;
#else
	return GetFileSize(Heliosno(f));
#endif
}

void error(char *str,...)
{
	va_list a;
	va_start(a,str);
	fprintf(stderr,"ERROR: ");
	vfprintf(stderr,str,a);
	fprintf(stderr,"\n");
	exit(1);
}

void warn(char *str,...)
{
	va_list a;
	va_start(a,str);
	fprintf(stderr,"WARNING: ");
	vfprintf(stderr,str,a);
	fprintf(stderr,"\n");
}

void loadrom(char *name)
{
	FILE *f = fopen(name,"rb");
	int size;
	ImageHdr hdr;
	FileData *fd;
	
	if( f == NULL ) error("cannot open %s",name);
	
	if( fread(&hdr,1,sizeof(hdr),f) != sizeof(hdr) )
		error("could not read image header");

	size = hdr.Size;
	
	romfile = malloc(size);
	
	if( romfile == NULL ) error("cannot get memory");

	if( fread(romfile,1,size,f) != size )
		error("could not read %d bytes from %s",size,name);
		
	fclose(f);
	
	for(	files = romfile;
		strcmp(files->Name,"Files") != 0;
		files = (Module *)((word)files + files->Size) );
		
	romsize = (word)files-(word)romfile;
	
	InitList(&filelist);
	
	for(	fd = (FileData *)(files+1);
		fd->Size != 0;
		fd = (FileData *)((word)fd + fd->Size + sizeof(FileData)) )
	{
		FileInfo *fi = malloc(sizeof(FileInfo));
		fi->FileData = *fd;
		fi->InRom = TRUE;
		fi->Data = (void *)(fd+1);
		AddTail(&filelist,&fi->Node);
	}
}

word imagesize(FileInfo *f)
{
	return (f->FileData.Size + sizeof(FileData) + 3) & ~3;
}

word writefile(FileInfo *f, FILE *fd)
{
	fwrite(&f->FileData,1,sizeof(FileData),fd);
	
	fwrite(f->Data,1,f->FileData.Size,fd);
	
	return 0;
}

void commitfile(char *name)
{
	FILE *f;
	char backup[256];
	char *p;
	ImageHdr hdr;
	word zero = 0;
	
	strcpy(backup,name);
	
	p = backup + strlen(backup);
	until( *p == '.' || *p == '/' || p == backup ) p--;
	
	if( *p == '.' ) *p = 0;
	strcat(backup,".bak");
	
	rename(name,backup);
	
	f = fopen(name,"wb");
	
	files->Size = WalkList(&filelist,imagesize) + sizeof(Module);

	hdr.Magic = Image_Magic;
	hdr.Flags = 0;
	hdr.Size = romsize + sizeof(zero) + files->Size;
			
	fwrite(&hdr,1,sizeof(hdr),f);
	
	fwrite(romfile,1,romsize,f);
	
	fwrite(files,1,sizeof(Module),f);
	
	WalkList(&filelist,writefile,f);
	
	fwrite(&zero,1,sizeof(zero),f);
	
	fclose(f);
}

int printfile(FileInfo *f)
{
	printf("%6d %s\n",f->FileData.Size,f->FileData.Name);
	return 1;
}

void listfiles()
{
	int nfiles = WalkList(&filelist,printfile);
	printf("Total %d files\n",nfiles);
}

bool findfile(FileInfo *fi, char *name)
{
	return (strcmp(fi->FileData.Name,name) == 0);
}

FileInfo *loadfile(char *fname, char *name)
{
	FILE *f = fopen(fname,openmode);
	FileInfo *fi = NULL;
	int size;
	char *buf;
	
	if( f == NULL ) 
	{
		warn("could not open %s",fname);
		return NULL;
	}
	
	size = fsize(f);

	size = (size + 3) & ~3;
	
	buf = malloc(size);
	
	if( buf == NULL ) error("could not allocate memory");
	
	size = fread(buf,1,size,f);

	until( size % 4 == 0 ) buf[size++] = '\n';
	
	fclose(f);
		
	if( size < 0 ) error("read failure on %s: %d",fname,errno);
	
	fi = malloc(sizeof(FileInfo));
	
	if( fi == NULL ) error("could not allocate memory");
	
	fi->FileData.Size = size;
	strcpy(fi->FileData.Name,name);
	fi->InRom = FALSE;
	fi->Data = buf;
	
	return fi;
}

void delfile(char *name)
{
	FileInfo *fi;

	fi = (FileInfo *)SearchList(&filelist,findfile,name);
	if( fi == NULL ) warn("cannot find %s",name);
	else
	{
		if( !fi->InRom ) free(fi->Data);
		Remove(&fi->Node);
		free(fi);
		changes=TRUE;
	}
}

void dofile(int mode, char *fname)
{
	char *name;
	FileInfo *fi;
	
	name = fname + strlen(fname);
	until( name == fname || *name == '/' ) name--;
	
	if( *name == '/' ) name++;

	switch( mode )
	{
	case REP:		
	case DEL:
		delfile(name);
		if( mode == DEL ) break;
		
	case ADD:
		fi = loadfile(fname,name);
		AddTail(&filelist,&fi->Node);
		changes=TRUE;
		break;				
	}
}

int main(int argc,char **argv)
{
	int mode;
	char *romname;
	
	if( argc <= 2 )
	{
		printf("usage: %s rom -l | {[-adr]|file}\n",argv[0]);
		exit(1);
	}
	
	romname=argv[1];
	
	loadrom(romname);
	
	for( argv+=2; *argv; argv++ )
	{
		char *arg = *argv;
		
		if( *arg == '-' )
		{
			arg++;
			switch( *arg++ )
			{
			case 'a': mode = ADD;		break;
			case 'd': mode = DEL;		break;
			case 'r': mode = REP;		break;
			case 'l': listfiles();		break;
			case 't': openmode = "r";	break;
			}
			continue;
		}
		
		dofile(mode,arg);
		openmode = "rb";
	}
	
	if( changes ) commitfile(romname);
}
