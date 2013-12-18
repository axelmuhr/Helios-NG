 /************************************************************************
 *  MV for Helios
 *  John Fitch, University of Bath, 1988 May 4
 *  Completely re-written PAB 24/5/88
 *  Helios Rename needs both paths to be relative to the object
 *  move(src,dst) is general purpose rename that can be passed abs path names
 *  and could be reused in other progs.
 *  If Rename fails we have to copy and delete the filename.
 *  If file is a directory, then these operations have to be done recusively.
 *  If file is a symbolic link then we must re-link and delete old ln
 *  This version interprets the -i -f and - options and adds the -m option
 *  -m = move only, do not attempt copy/delete if rename fails
 * Updates:
 *  24/1/89 PAB
 *  check host filesystems names against the ones supplied - server sometimes
 *  translates them allowing files to be copied to themselves etc.
 */
#ifdef __TRAN
static char *rcsid ="$Header: /users/nickc/RTNucleus/cmds/com/RCS/mv.c,v 1.7 1994/03/08 10:23:32 nickc Exp $";
#endif

/*
 * NAME
 *      mv - move or rename files
 * 
 * SYNOPSIS
 *      mv [ -i ] [ -m ] [ -f ] [ - ] file1 file2
 * 
 *      mv [ -i ] [ -m ] [ -f ] [ - ] file ... directory
 * 
 * DESCRIPTION
 *      Mv moves (changes the name of) file1 to file2.
 * 
 *      If file2 already exists,  it  is  removed  before  file1  is
 *      moved. 
 * 
 *      In the second form, one or more files (plain files or direc-
 *      tories)  are  moved  to  the  directory  with their original
 *      file-names.
 * 
 *      Mv refuses to move a file onto itself.
 * 
 *      Options:
 *      -i   stands for interactive mode.  Whenever  a  move  is  to
 *           supercede an existing file, the user is prompted by the
 *           name of the file followed by a  question  mark.  If  he
 *           answers with a line starting with `y', the move contin-
 *           ues. Any other reply prevents the move from occurring.
 *
 *      -m   Move only, do not attempt to copy/delete the file(s) if
 *           Rename fails. (Helios version only - not unix)
 *
 *      -f   stands for force. This option overrides any  mode  res-
 *           trictions or the -i switch (not currently used in
 *           Helios version).
 *
 *      -    means interpret all the following arguments  to  mv  as
 *           file  names.   This  allows  file  names  starting with
 *           minus.
 * 
 * BUGS
 *      If rename of file1 to file2 fails i.e. if they lie on
 *      different file  systems,  mv  must copy  the  file(s)  and
 *      delete the original. Any links that exist are relinked, if this
 *      fails, the link is lost.
 *
 *	-f switch doesn't do anything useful at present.
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <syslib.h>
#include <gsp.h>
#include <string.h>
#include <ctype.h>

#define EOS	'\0'
#define TIMEOUT -1	/* infinity */
#define MAXPATH 200
#define FILEBUFFER 4000	/* size of file buffer for copy */
			/* same as std Message_Limit in HOST.CON */
 
#define USAGE "Usage: mv [-mif] <old> <new>; or mv [-mif] <old1> <old2>... <oldn> <dir>\n"

int	iflag = FALSE;		/* Interactive flag */
int	fflag = FALSE;		/* Force flag */
int	mflag = FALSE;		/* Move only flag */
int	dbug = FALSE;		/* debugging flag */

char	filebuf[FILEBUFFER];	/* copy buffer */
union	AnyInfo
  {
    ObjInfo	Oinfo;			/* obj info buffer */
    Link_Info	Linfo;			/* link info buffer */
    byte	buff[512];		/* max possible size variation */
  } Info;


void usage_exit()
{
	fprintf(stderr, USAGE);
	exit(1);
}

/*
 * Is the object a symbolic link?, if so return the original parent obj
 * that the link points, else NULL if not a link.
 *
 * Note that you must pass both the obj and the path string, if
 * you Locate a link, and then just pass the obj ptr, 
 * we cant then tell if this is a link as Locate locates right thru to the
 * original obj.
 */
Object *
islink(
       Object *	obj,
       char *	path )
{

ObjectInfo(obj, path, (byte *) Info.buff);
if (Info.Oinfo.DirEntry.Type == Type_Link)
	return(Locate(CurrentDir, Info.Linfo.Name));

return(NULL);
}

/*
 * Find common path of src and dst
 * returns object to common path and adjusts src and dst to exclude
 * the common path. If error returns NULL.
 * e.g. passed /helios/john/tim and /helios/john/fred
 * returns obj * to /helios/john and src = tim, dst = fred
 * DANGER: routine cannot cope with src or dst = '/'.
 */
Object *Find_Common(
		    char **	src,
		    char **	dst )
{
char	*oldname, *newname;
char	*startold, relpath[MAXPATH];

startold = oldname = *src;	/* get ptrs to actual str ptrs */
newname = *dst;

/* find common relative path */
/* first find point where names differ */
while (*++oldname == *++newname && *oldname != '\0') ;/*null stat*/

/* fix problems where a pathname includes another */
/* i.e. /test is incl. in /test2 partway */
if (*oldname == '/' && *newname != '\0') {
	/* dir completely incorporated into other */
	oldname--;	/* get past /test/ -> /test2 */
	newname--;
	}
while (*oldname != '/')	{
	/* dir partly incorporated into other */
	--newname;	/* get past /abc123 -> /abc987 */
	--oldname;
	}

memcpy(relpath, startold, (size_t) (oldname - startold));
relpath[oldname-startold] = '\0';

oldname++; /* inc past leading '/' */

if (*newname != '\0') /* if not zero length i.e. not in same dir as oldname */
	newname++; /* inc past '/' */

*src = oldname;	/* pass new path positions back */
*dst = newname;

/* return obj for common parent to both paths */
return(Locate(CurrentDir, relpath));
}


/*
 * Check that it is safe to recursively move src dir to dst dir
 * i.e. not safe to mv /helios/john to /helios/john/harry !
 * Returns TRUE if recursion is unsafe
 */
int
Check_Recursion(
		char *	src,
		char *	dst )
{
int	l;

if ((l = strlen(src)) <= strlen(dst)
	&& strncmp(src, dst, l) == 0
	&& (dst[l] == '/' || dst[l] == '\0'))
	return (TRUE); /* Not safe to recurse */
/* last test is to make sure src dir is not partial name of target i.e. fred in freddy */

return(FALSE);
}

/*
 * Delete objects file, if file is directory, delete its contents recursively
 */
int
DeepDelete(Object *Target )
{
Stream	*inf;
int	i, len;
word	size,dirsize;
DirEntry *Dirbuf, *MDirbuf;		/* directory ops buffer */

if (Target == NULL)
	return(FALSE);

if (Target->Type & Type_Directory)
	{
	inf = Open(Target,NULL,O_ReadOnly);
	dirsize = GetFileSize(inf);
	len     = (int) dirsize/sizeof(DirEntry);
	Dirbuf  = (DirEntry *) Malloc(dirsize);
	MDirbuf = Dirbuf;
	if(Dirbuf == Null(DirEntry))
	{
		fprintf(stderr,"mv: Out of memory\n");
		exit(1);
	}
	size = Read(inf, (byte *)Dirbuf, dirsize, TIMEOUT);
	if(size != dirsize)
	{
		fprintf(stderr,
			"mv: Read error on directory %s : %lx\n", Target->Name, Result2(inf));
		exit (1);
	}
	
	for(i = 0; i < len; i++,Dirbuf++)
		{
		if (Dirbuf->Name[0]=='.')
			{
			if (Dirbuf->Name[1]=='\0')
				continue;
			elif ((Dirbuf->Name[1] == '.')
			       && (Dirbuf->Name[2] == '\0'))
				continue;
			}
		/* recursive entry */
		if (DeepDelete(Locate(Target, Dirbuf->Name)) == FALSE)
			return(FALSE);
		}
	Close(inf);
	Free(MDirbuf);
	/* fall thru and delete actual directory */
	}

if (dbug) fprintf(stderr,"Delete %s\n",Target->Name);

if (Delete(Target,NULL) == 0);
	{
	Close(Target);	/* close these objects as recursion may be deep */
	return(TRUE);
	}
Close(Target);
return(FALSE);
}

/*
 * Extracted from helios cp command and then munged extensively!
 *
 * Copy srcpath object into dstdir object with the name dst.
 */
int copyfile(
	     Object *	srcpath,
	     Object *	dstdir,
	     char *	dst )
{
Stream	*inf, *outf;
char	dstname[MAXPATH];
Object	*origin;
int	n, i, len;
word	size,dirsize;
DirEntry *Dirbuf, *MDirbuf;		/* directory ops buffer */

 
if (srcpath->Type & Type_Directory)
	{
	/* Must work recusively; first create destination directory */
	/* and then work through directory */
	/* mimic unix mv - mv into directory as long as its not empty */

	if (dbug) fprintf(stderr,"Create %s/%s directory\n",dstdir->Name,dst);

	unless (Create(dstdir, dst, Type_Directory,0,0))
		{
		fprintf(stderr,"mv: Cannot create %s directory : %lx\n",
							dst, Result2(dstdir));
		exit(1);
		}
	dstdir = Locate(dstdir, dst);
	inf = Open(srcpath,NULL,O_ReadOnly);
	dirsize = GetFileSize(inf);
	len = (int)dirsize/sizeof(DirEntry);
		
	Dirbuf = (DirEntry *) Malloc(dirsize);
	MDirbuf = Dirbuf;
	if(Dirbuf == Null(DirEntry))
	{
		fprintf(stderr,"mv: Out of memory\n");
		exit(1);
	}
	size = Read(inf, (byte *)Dirbuf, dirsize, TIMEOUT);
	if(size != dirsize)
	{
		fprintf(stderr,
			"mv: Read failure on directory %s : %lx\n", srcpath->Name, Result2(inf));
		exit (1);
	}
	
	for(i = 0; i < len; i++,Dirbuf++)
		{
		if (Dirbuf->Name[0]=='.')
			{
			if (Dirbuf->Name[1]=='\0')
				continue;
			elif ((Dirbuf->Name[1] == '.')
			       && (Dirbuf->Name[2] == '\0'))
				continue;
			}
		if ((origin = islink(srcpath, Dirbuf->Name)) != NULL)
			{
			strcpy(dstname, dstdir->Name);
			strcat(dstname, "/");
			strcat(dstname, Dirbuf->Name);
			if (dbug) fprintf(stderr,  "Relink %s/%s (%s), -> %s\n",srcpath->Name, Dirbuf->Name, origin->Name, dstname);
			if (Link(CurrentDir, dstname, origin) != 0)
				{
				fprintf(stderr,"mv: Warning %s/%s was not relinked\n",srcpath->Name,Dirbuf->Name);
				return(TRUE); /* ignore any failures on relinking */
				}
			Delete(srcpath,Dirbuf->Name);
			continue;
			}
		/* recursive entry */
		copyfile(Locate(srcpath, Dirbuf->Name), dstdir, Dirbuf->Name);
		}
	Close(dstdir);	/* close locates as this is a recusive fn */
	Close(inf);
	Free(MDirbuf);
	return (TRUE);
	}

if (dbug) fprintf(stderr,"Copyfile %s\nto %s/%s\n",srcpath->Name,dstdir->Name,dst);

if ((inf = Open(srcpath, NULL, O_ReadOnly)) == NULL)
	{
	fprintf(stderr,
	  "mv: Cannot open %s : %lx\n", srcpath->Name, Result2(srcpath));
	exit(1);
	}

if ((outf = Open(dstdir, dst, O_Create|O_WriteOnly)) == NULL)
	{
	fprintf(stderr,
	  "mv: Error trying to create %s : %lx \n", dst, Result2(dstdir));
	exit(1);
	}

forever
	{
	n = (int) Read(inf, filebuf, FILEBUFFER, TIMEOUT);
	if (n == -1) /* eof */
		{
		Close(inf);
		Close(outf);
		return (TRUE);
		}
	if (n <= 0)
		{
		/* n==0 means timeout with no chars read */
		fprintf(stderr,"mv: Error whilst reading %s : %lx\n",
					srcpath->Name, Result2((Object *)inf));
		Close(inf);
		Close(outf);
		return (FALSE);
		}
	elif (Write(outf, filebuf, n, TIMEOUT) != n)
		{
		fprintf(stderr,"mv: Error whilst writing %s : %lx\n",
      					srcpath->Name, Result2((Object *)inf));
		Close(inf);
		Close(outf);
		return (FALSE);
		}
	}
}

/* Due to the spec. of the Helios Rename routine, you have to find
the first part of the two filenames that are common and then locate this and use
it as your common relative object to the remaining portions of the filenames.
(couldnt find a way to get name of server responsible for src file)
You cannot locate the actual target filename, as it probably doesnt exist
yet, therfore find its parent directory, locate this and then add the final part
of the name back on for the actual rename */

void move(
	  char * Src,
	  char * Dst )
{
Object	*SrcObj, *DstObj, *RelObj;
char	newpath[MAXPATH] ,oldpath[MAXPATH];
char	*newfile, *oldname, *newname = "", *tmp;
Object	*origin;
int	invalid = FALSE;

/* if target exists and we are in interactive mode */
if ((DstObj = Locate(CurrentDir, Dst)) != NULL && iflag && !fflag)
	{
	/* Ask the user for permission to delete */
	fprintf(stderr,"Remove %s? ",Dst);
	fflush(stderr);
	if (tolower(getchar()) != 'y')
	       	{
		while (getchar() != '\n') ;
		fprintf(stderr,"File not overwritten\n");
		return;
		}
	while (getchar() != '\n') ;
	}

/* if the src obj cannot be found or it is the current directory */
if ((SrcObj = Locate(CurrentDir,Src)) == NULL || strcmp(SrcObj->Name, CurrentDir->Name) == 0)
	{
	fprintf(stderr,"mv: %s : Invalid source name\n", Src);
#if 0
	exit(1);
#else
	return;
#endif
	}

/* get parent directory part of dst - cannot locate file that doesn't exist */
/* newfile points to final part of target name i.e. the new file/dir to create */
if ((newfile = strrchr(Dst, '/')) == NULL) 
	{
	newfile = Dst;
	Dst = NULL;				/* dst dir must be current ?! */
	}
else
	{
	*newfile++ = '\0';			/* separate dir from target name + del trailing '/' */

	if (*Dst == '\0')
		strcpy(Dst,"/");		/* special case - if root need trailing '/' ! */
	}

/* check that filenames are correct i.e. that server has not changed them */
/* behind our backs, and that src and dst are not now the same */
/* as these checks are only valid if we are in the same directory */
/* set a flag to test when we know if the directory is the same */
/* Ok its a hack ! */
if (DstObj != NULL)
	{
		/* check for valid dst name */
	if ((newname = strrchr(DstObj->Name, '/')) == NULL)
		invalid = TRUE;
	else
		{
		/* compare real obj filename and text name */
		if (strcmp(++newname,newfile) != 0)
			invalid = TRUE;
		if (dbug) fprintf(stderr,"compare obj %s, txt %s\n", newname, newfile);
		}
	}

/* check for valid src name */
if((oldname = strrchr(SrcObj->Name, '/')) == NULL)
	invalid = TRUE;
else
	/* compare real obj name and text name */
	{
	if ((tmp = strrchr(Src,'/')) == NULL)
		tmp = Src;
	else
		tmp++;
	if (strcmp(++oldname, tmp) != 0)
		invalid = TRUE;
	if (dbug) fprintf(stderr,"compare src obj %s, txt %s\n", oldname, tmp);
	}

#if 0
if (strcmp(oldname, newname) == 0)
	invalid = TRUE;
#endif


/* if destination directory doesnt exist, or its not a directory - err */
if ((DstObj = Locate(CurrentDir, Dst)) == NULL || !(DstObj->Type & Type_Directory))
	{
	fprintf(stderr,"mv: Invalid destination\n");
	exit(1);
	}

/* if src is a link, we must relink and delete it rather than rename */
/* this is because rename requires a common path which has to be found */
/* by locates - locates locate thru to the parent of the link */
/* and we want to rename the link not its parent */
if ((origin = islink(CurrentDir, Src)) != NULL)
	{
	strcpy(newpath, DstObj->Name);
	strcat(newpath, "/");
	strcat(newpath, newfile);
	if (dbug) fprintf(stderr,"Link %s(%s) -> %s\n",Src,origin->Name, newpath);
	Link(CurrentDir, newpath, origin);
	Delete(CurrentDir, Src);
	return;
	}

strcpy(oldpath, SrcObj->Name);			/* get full pathnames */
strcpy(newpath, DstObj->Name);

oldname = oldpath;
newname = newpath;

/* if following two tests are not valid, then the following Find_Common must */
/* be changed as it cannot cope with just '/' as src or dst */ 
if (strcmp(oldname, "/") == 0) /* if src is just root */
	{
	fprintf(stderr,"mv: Cannot move root directory!\n");
	exit(1);
	}
if (strcmp(newname, "/") == 0) /* if dst is just root */
	{
	fprintf(stderr,"mv: Cannot move to root directory\n");
	exit(1);
	}

/* find portion of path common to old and newname */
if ((RelObj = Find_Common(&oldname, &newname)) == NULL)
	{
	fprintf(stderr,"mv: Internal error finding common path\n");
	exit(1);
	}

if (*newname != '\0') /* if not zero length i.e. not in same dir as oldname */
	strcat(newname,"/"); /* separate filenames */
else
	if (invalid)
		{
		fprintf(stderr,"mv: filename not valid\n");
     		exit(1);
		}

strcat(newname,newfile); /* add target filename onto the end of the dest dir */

if (strcmp(oldname,newname) == 0) /* if filenames are the same */
	{
	fprintf(stderr,"mv: Cannot rename a file to itself\n");
	exit(1);
	}

/* check that rename is safe */
if (SrcObj->Type & Type_Directory && DstObj->Type & Type_Directory
   && Check_Recursion(SrcObj->Name, DstObj->Name))
	{
	if (dbug) fprintf(stderr,"Src dir = %s, target dir = %s\n",
			  SrcObj->Name, DstObj->Name);
	fprintf(stderr,	"mv: Cannot move directory into itself\n");
	exit(1);
	}

if(dbug) fprintf(stderr,
  "Rename relObj = %s, oldname = %s, newname = %s\n",RelObj->Name, oldname,newname);

if (Rename(RelObj,oldname,newname) != 0)
	{
	if (mflag)
		{
		fprintf(stderr,
		  "mv: Rename on %s failed, object not moved\n", SrcObj->Name);
		exit(1);
		}

	if (dbug) fprintf(stderr,"Trying to copy file - Rename failed\n");

	/* moving directories? */
	if (SrcObj->Type & Type_Directory && DstObj->Type & Type_Directory)
		{
		if (copyfile(SrcObj,DstObj,newfile) == TRUE)
			{
			if (dbug) fprintf(stderr,"mv: Directory copy successfull\n");
			if (DeepDelete(SrcObj) == TRUE)
				{
				if (dbug) fprintf(stderr,"mv: DeepDelete successful\n");
				return;
				}
			fprintf(stderr,"mv: Directory copied, but couldn't delete original\n");
			return;
			}
		fprintf(stderr,"mv: Directory not moved, attempted copy failed also\n");
		}
	elif (copyfile(SrcObj,DstObj,newfile) == TRUE)
		{
		if (dbug) fprintf(stderr,"mv: copy successfull\n");
		if (dbug) fprintf(stderr,"Delete %s\n",SrcObj->Name);
		if (Delete(SrcObj,NULL) == 0)
			{
			if (dbug) fprintf(stderr,"mv: delete successful\n");
			return;
			}
		fprintf(stderr,"mv: File copied, but couldn't delete original\n");
		return;
		}
	fprintf(stderr,"mv: File not moved, attempted copy failed also\n");
	}
}

int
main(int argc, char **argv)
{
int	i;
char	*ap, *lastpart;
Object	*DstDir;
char	newname[MAXPATH];

while (argc > 1 && *(ap = argv[1]) == '-')
	{
	if (*++ap==EOS)
		break;	/* Deal with - option */
	while (*ap != EOS)
		{
		switch (*ap++)
			{
			case 'i':
				iflag++;
				break;
	      
			case 'f':
				fflag++;
				break;

			case 'm':
				mflag++;
				break;

			case '!':
				dbug++;
				break;

			default:
				fprintf(stderr,
				  "mv: Warning, bad option '%c'\n", ap[-1]);
				break;
			}
		}
	argc--;
	argv++;
	}

if (argc < 3)
	usage_exit();

/* must have at least two name args */
DstDir = Locate(CurrentDir,argv[argc-1]);

/* If last arg is a directory then move the files into it */
if (DstDir != NULL && DstDir->Type & Type_Directory)
	{
	for (i = 1; i < argc-1; i++)
		{
		strcpy(newname,DstDir->Name);
		strcat(newname,"/");
		/* append name to end of dir */
		if ((lastpart = strrchr(argv[i], '/')) == NULL)
			strcat(newname, argv[i]);
		else
			strcat(newname, ++lastpart);
		/* rename file */
		move(argv[i],newname);
		}
	}
else
	/* else must be mv file1 -> file2 format */
	{
	if (argc != 3) /* do we have just two filenames? */
		usage_exit();

	move(argv[1], argv[2]);
	}

exit(0);
}
