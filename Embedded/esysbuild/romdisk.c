
/*
 *		ROM DISK FUNCTIONS
 */

/*
 *   If a romdisk directory has been specified, then we create a romdisk
 * from the files found in that directory's tree.  If a romdisk file is
 * specified, we call the romdisk by that name. otherwise we use the
 * default.
 *
 *   Note that, if a romdisk directory is not specified but a romdisk file
 * is, then we tacitly assume that the specified file is a valid romdisk.
 * Also, we add a romdisk file to the nucleus even if a rom disk server is
 * not specified in the (possibly mistaken) belief that the user knows what
 * he is doing.
 */

#ifdef UNIX

#include "/usr/include/stdio.h"
#include "/usr/include/stdlib.h"
#include "/usr/include/string.h"

#include "/usr/include/sys/types.h"

#include "/usr/include/dirent.h"

#include "/usr/include/sys/stat.h"

#include "/hsrc/include/memory.h"
#include "/hsrc/include/nonansi.h"
#include "/hsrc/include/servlib.h"
#include "/hsrc/include/syslib.h"
#include "/hsrc/include/queue.h"
#include "/hsrc/include/gsp.h"
#include "/hsrc/include/module.h"

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory.h>
#include <nonansi.h>
#include <syslib.h>
#include <queue.h>
#include <gsp.h>
#include <servlib.h>

#include <module.h>

#endif	/* UNIX */

#include "defs.h"
#include "externs.h"

int	NodeNumber = 1;

/*
 * To Build a Rom Disk
 *
 *	If writing the rom disk to a file, output an Image Header.
 *
 *	Place a Module Header at the top of the file.
 *	
 *	Calculate the number of entries in the rom disk by starting at
 * the starting directory and finding every file and directory.
 *
 *	Starting at the given directory place the DirNode and ObjNode
 * headers into the romdisk with a pointer to the objects contents in the
 * 'Contents' field.  (The first (root) DirNode will have its name set to
 * the given root name.)
 *
 *	Output the modules.
 *
 */

#define NoWriteMask	(~(AccMask_W * 0x01010101))

#define MyDefDirMatrix	DefDirMatrix
#define MyDefFileMatrix	(DefFileMatrix & NoWriteMask)

#ifndef WORD
typedef long		word;
typedef unsigned char	ubyte;
#endif

int	WriteRomDiskFile;
FILE *	RomDiskFp = NULL;

FILE *	RomInFp = NULL;

word	ObjectOffset = 0;	/* offset to where the file */
				/* starts in the rom disk   */

void output_node (ObjNode 	node)
{
	chars_to_image ((ubyte *)(&node), sizeof (ObjNode));
	
	/* write to rom disk file if reqd */
	if (WriteRomDiskFile)
	{
		fwrite ((void *)(&node), 1, sizeof (ObjNode), RomDiskFp);
	}

	NodeNumber++;
}	

void romfile_patch_space (int	n_bytes)
{
	if (!WriteRomDiskFile)	return;

	while (n_bytes--)
	{
		putc ('\0', RomDiskFp);
	}
}

#ifdef __HELIOS


int find_entries (char *	dir_name)
{
	Object *	o = Locate (CurrentDir, dir_name);
	Stream *	s;

	DirEntry *	entry;
	DirEntry *	cur;

	int	i;
	word	size = 0;
	int	number_of_entries;

	int	n = 0;

	if (o == Null (Object))
	{
		sysbuild_error ("Unknown directory %s", dir_name);

		return 0;
	}

	if ((o -> Type & Type_Flags) == Type_Stream)
	{
		return 0;
	}

	s = Open (o, Null (char), O_ReadOnly);

	if (s == Null (Stream))
	{
		Close (o);

		sysbuild_error ("Failed to open directory %s", dir_name);
	}

	size = GetFileSize (s);

	if (size == 0)
	{
		Close (s);
		Close (o);

		return 0;
	}

	entry = (DirEntry *)(Malloc (size));

	if (entry == Null (DirEntry))
	{
		Close (s);
		Close (o);

		return 0;
	}

	if (Read (s, (byte *)(entry), size, -1) != size)
	{
		sysbuild_error ("Error reading directory %s", dir_name);
	}

	number_of_entries = (int) size / (sizeof (DirEntry));

	n += number_of_entries - 2;	/* take of . and .. */

	for (i = 0, cur = entry; i < number_of_entries; i++, cur++)
	{
		char	this_entry[IOCDataMax];

		strcpy (this_entry, dir_name);
		strcat (this_entry, "/");
		strcat (this_entry, cur -> Name);

		if (strequ_(cur -> Name, ".") || strequ_(cur -> Name, ".."))
		{
			/* ignore myself and parent */
			continue;
		}

		if (cur -> Type == Type_Directory)
		{
			/* found new directory entry, so recurse */
			n += find_entries (this_entry);
		}
	}

	Free (entry);
	Close (s);
	Close (o);

	return n;
}


int output_lower_nodes (char *	dir_name)
{
	Object *	o = Locate (CurrentDir, dir_name);
	Stream *	s;

	DirEntry *	entry;
	DirEntry *	cur;

	ObjNode		objnode;

	word	size;

	int	number_of_entries;

	int	i;

	if (o == Null (Object))
	{
		sysbuild_error ("Unknown directory %s", dir_name);
	}

	if ((o -> Type & Type_Flags) == Type_Stream)
	{
		return SYSBUILD_OK;
	}

	s = Open (o, Null (char), O_ReadOnly);

	if (s == Null (Stream))
	{
		Close (o);

		sysbuild_error ("Failed to open directory %s", dir_name);
	}

	size = GetFileSize (s);

	if (size == 0)
	{
		Close (s);
		Close (o);

		return SYSBUILD_OK;
	}

	entry = (DirEntry *)(Malloc (size));

	if (entry == Null (DirEntry))
	{
		Close (s);
		Close (o);

		return SYSBUILD_FAIL;
	}

	if (Read (s, (byte *)(entry), size, -1) != size)
	{
		sysbuild_error ("Error reading directory %s", dir_name);
	}

	number_of_entries = (int) size / (sizeof (DirEntry));

	for (i = 0, cur = entry; i < number_of_entries; i++, cur++)
	{
		char		this_entry[IOCDataMax];

		strcpy (this_entry, dir_name);
		strcat (this_entry, "/");
		strcat (this_entry, cur -> Name);

		if (strequ_(cur -> Name, ".") || strequ_(cur -> Name, ".."))
		{
			/* ignore myself and parent */
			continue;
		}

		if (cur -> Type == Type_Directory)
		{
			InitNode (&objnode,
				  cur -> Name,
				  (int) swap (Type_Directory),
				  0,
				  swap (MyDefDirMatrix));

			((DirNode *)&objnode) -> Nentries = swap ((word) number_of_entries - 2);

			output_node (objnode);

			output_lower_nodes (this_entry);
		}
		else if (cur -> Type == Type_File)
		{
			InitNode (&objnode,
				  cur -> Name,
				  (int) swap (Type_File),
				  0,
				  swap (MyDefDirMatrix));

			objnode.Size = swap (getfilesize (this_entry));

			/*
			 * Patch the address of code.  The offset from
 			 * the beginning of the romdisk.
			 */

			(&objnode.Size)[1] = swap (ObjectOffset);

			ObjectOffset += objnode.Size;

			output_node (objnode);
		}
		else
		{
			sysbuild_warning ("%s neither file nor directory", 
						this_entry);
		}
	}

	Free (entry);
	Close (s);
	Close (o);

	return SYSBUILD_OK;
}

int output_nodes (char *	root_name,
		  char *	dir_name)
{
	Object *	o = Locate (CurrentDir, dir_name);
	Stream *	s;

	DirEntry *	entry;

	ObjNode		objnode;

	word	size;

	memset (&objnode, 0, sizeof (ObjNode));

	sysbuild_debug ("output_nodes (%s, %s)\n", root_name, dir_name);

	if (o == Null (Object))
	{
		sysbuild_error ("Unknown directory %s", dir_name);

		return SYSBUILD_FAIL;
	}

	if ((o -> Type & Type_Flags) == Type_Stream)
	{
		return SYSBUILD_OK;
	}

	s = Open (o, Null (char), O_ReadOnly);

	if (s == Null (Stream))
	{
		Close (o);

		sysbuild_error ("Failed to open directory %s", dir_name);
	}

	size = GetFileSize (s);

	if (size == 0)
	{
		Close (s);
		Close (o);

		return SYSBUILD_OK;
	}

	entry = (DirEntry *)(Malloc (size));

	if (entry == Null (DirEntry))
	{
		Close (s);
		Close (o);

		return SYSBUILD_FAIL;
	}

	if (Read (s, (byte *)(entry), size, -1) != size)
	{
		sysbuild_error ("Error reading directory %s", dir_name);
	}

	InitNode (&objnode,
		  root_name,
		  (int) swap (Type_Directory),
		  0,
		  swap (MyDefDirMatrix));

	((DirNode *)(&objnode)) -> Nentries = swap ((size / sizeof (DirEntry)) - 2);

	output_node (objnode);

	Free (entry);
	Close (s);
	Close (o);

	return (output_lower_nodes (dir_name));
}

int output_file (char *	file_name)
{
	word	size;
	int	extra;

	if ((RomInFp = fopen (file_name, "rb")) == NULL)
	{
		sysbuild_error ("Failed to open %s", file_name);

		return SYSBUILD_FAIL;
	}

	size = GetFileSize (Heliosno (RomInFp));

	extra = (size & 3) ? 4 - ((int)size & 3) : 0;

	sysbuild_info ("\t\t[file: %s; size: 0x%08lx]", file_name, size);

	/* write out the file to the image */
	file_to_image (RomInFp, (int) size, file_name);

	if (WriteRomDiskFile)
	{
		/* write out file to separate rom disk file */
		fseek (RomInFp, 0, SEEK_SET);

		while (size--)
		{
			putc (getc (RomInFp), RomDiskFp);
		}
	}

	align_image ();
	romfile_patch_space (extra);

	fclose (RomInFp);  RomInFp = NULL;

	return SYSBUILD_OK;
}


int output_objects (char *	dir_name)
{
	Object *	o = Locate (CurrentDir, dir_name);
	Stream *	s;

	DirEntry *	entry;
	DirEntry *	cur;

	int	i;
	int	size = 0;
	int	number_of_entries;

	if (o == Null (Object))
	{
		sysbuild_error ("Unknown directory %s", dir_name);
	}

	if ((o -> Type & Type_Flags) == Type_Stream)
	{
		return SYSBUILD_OK;
	}

	s = Open (o, Null (char), O_ReadOnly);

	if (s == Null (Stream))
	{
		Close (o);

		sysbuild_error ("Failed to open directory %s", dir_name);
	}

	size = (int) GetFileSize (s);

	if (size == 0)
	{
		return SYSBUILD_OK;
	}

	entry = (DirEntry *)(Malloc (size));

	if (entry == Null (DirEntry))
	{
		Close (s);
		Close (o);

		return SYSBUILD_FAIL;
	}

	if (Read (s, (byte *)(entry), size, -1) != size)
	{
		sysbuild_error ("Error reading directory %s", dir_name);
	}

	number_of_entries = size / (sizeof (DirEntry));

	for (i = 0, cur = entry; i < number_of_entries; i++, cur++)
	{
		char	this_entry[IOCDataMax];

		strcpy (this_entry, dir_name);
		strcat (this_entry, "/");
		strcat (this_entry, cur -> Name);

		if (strequ_(cur -> Name, ".") || strequ_(cur -> Name, ".."))
		{
			/* ignore myself and parent */
			continue;
		}

		if (cur -> Type == Type_Directory)
		{
			/* found new directory entry, so recurse */
			if ((output_objects (this_entry)) == SYSBUILD_FAIL)
			{
				return SYSBUILD_FAIL;
			}
		}
		else if (cur -> Type == Type_File)
		{
			output_file (this_entry);
		}
		else
		{
			sysbuild_warning ("%s neither file nor directory",
						this_entry);
		}
	}

	Free (entry);
	Close (s);
	Close (o);

	return SYSBUILD_OK;
}

#else	/* In Unix World */

int getdirsize (char *	dir_name)
{
	DIR *	dirp;

	struct dirent *	dir_entry;

	int	entries = 0;

	if ((dirp = opendir (dir_name)) == NULL)
	{
		sysbuild_warning ("Unable to open directory %s", dir_name);

		return -1;
	}
	
	while ((dir_entry = readdir (dirp)) != NULL)
	{
		if (  strequ_(dir_entry -> d_name, ".")
		   || strequ_(dir_entry -> d_name, ".."))
		{
			continue;
		}

		entries++;
	}

	return entries;
}

int find_entries (char *	dir_name)
{
	struct stat	statbuf;
	int		size;

	char *	this_entry;
	char *	tailp;
	char	pathname[IOCDataMax];
	int	pos;

	sysbuild_debug ("find_entries (%s)", dir_name);

	if (stat (dir_name, &statbuf) != 0)
	{
		sysbuild_error ("find_entries () - Failed to stat directory %s", dir_name);

		return 0;
	}

	size = statbuf.st_size;

	pos = strlen (dir_name) - 1;	 /* points to last char in string */
	while (pos != 0 && dir_name[pos] != '/')
	{
		pos--;
	}
	if (dir_name[pos] == '/')	pos++;

	this_entry = &(dir_name[pos]);

	sysbuild_debug ("find_entries (%s) - this_entry = %s", dir_name, this_entry);

	strcpy (pathname, dir_name);
	strcat (pathname, "/");

	tailp = &(pathname[strlen(pathname)]);

	if (size == 0)
	{
		return 0;
	}

	if (S_ISREG (statbuf.st_mode))
	{
		sysbuild_debug ("%s is a regular file", dir_name);
		return 1;
	}
	else if (S_ISDIR (statbuf.st_mode))
	{
		DIR *	dirp;

		struct dirent *	dir_entry;

		int	n = 0;

		sysbuild_debug ("find_entries () - %s is a directory", dir_name);

		if ((dirp = opendir (dir_name)) == NULL)
		{
			sysbuild_error ("Failed to open %s", dir_name);

			return 0;
		}

		while ((dir_entry = readdir (dirp)) != NULL)
		{
			if (  strequ_(dir_entry -> d_name, ".")
			   || strequ_(dir_entry -> d_name, ".."))
			{
				/* ignore myself and parent */
				continue;
			}

			strcpy (tailp, dir_entry -> d_name);

			sysbuild_debug ("find_entries () - pathname = %s", pathname);

			n += find_entries (pathname);
		}

		closedir (dirp);

		return n;
	}
	else
	{
		sysbuild_error ("%s neither directory or regular file", dir_name);

		return 0;
	}
}

/* UNIX WORLD */
int output_lower_nodes (char *	dir_name,
			int	is_root)	/* Is this directory the root ? */
{
	struct stat	statbuf;
	int		size;

	char *	this_entry;
	char *	tailp;
	char	pathname[IOCDataMax];
	int	pos;

	ObjNode		objnode;

	memset ((void *)(&objnode), 0, sizeof (ObjNode));

	sysbuild_debug( "@output_lower_nodes (%s)", dir_name);

	if (stat (dir_name, &statbuf) != 0)
	{
		sysbuild_error ("output_lower_nodes () - Failed to stat directory %s", dir_name);

		return 0;
	}

	size = statbuf.st_size;

	if (size == 0)
	{
		return SYSBUILD_OK;
	}

	size += (size & 3) ? (4 - (size & 3)) : 0;

	pos = strlen (dir_name) - 1;	 /* points to last char in string */
	while (pos != 0 && dir_name[pos] != '/')
	{
		pos--;
	}
	if (dir_name[pos] == '/')	pos++;

	this_entry = &(dir_name[pos]);

	sysbuild_debug ("output_lower_nodes (%s) - this_entry = %s", dir_name, this_entry);

	strcpy (pathname, dir_name);
	strcat (pathname, "/");

	tailp = &(pathname[strlen(pathname)]);

	if (S_ISREG (statbuf.st_mode))
	{
		/* regular file */

		strcpy (objnode.Name, this_entry);
		objnode.Type = swap (Type_File);
		objnode.Flags = 0;
		objnode.Matrix = swap (MyDefDirMatrix);

		objnode.Size = swap (size);

		(&objnode.Size)[1] = swap (ObjectOffset);

		ObjectOffset += size;

		output_node (objnode);

		return SYSBUILD_OK;
	}
	else if (S_ISDIR (statbuf.st_mode))
	{
		DIR *	dirp;

		struct dirent *	dir_entry;

		int	n = 0;

		if (!is_root)
		{
			/*
			 * The root directory has already been done by
			 * output_nodes () (with its name changed.
			 */
			strcpy (objnode.Name, this_entry);
			objnode.Type = swap (Type_Directory);
			objnode.Flags = 0;
			objnode.Matrix = swap (MyDefDirMatrix);

			((DirNode *)(&objnode)) -> Nentries = swap (getdirsize (dir_name));

			if (swap (objnode.Size) == -1)
			{
				return 0;
			}

			output_node (objnode);
		}

		if ((dirp = opendir (dir_name)) == NULL)
		{
			sysbuild_error ("Failed to open %s", dir_name);

			return SYSBUILD_FAIL;
		}

		while ((dir_entry = readdir (dirp)) != NULL)
		{
			if (  strequ_(dir_entry -> d_name, ".")
			   || strequ_(dir_entry -> d_name, ".."))
			{
				/* ignore myself and parent */
				continue;
			}

			strcpy (tailp, dir_entry -> d_name);
			if (output_lower_nodes (pathname, FALSE) == SYSBUILD_FAIL)
			{
				return SYSBUILD_FAIL;
			}
		}

		closedir (dirp);

		return n;
	}
	else
	{
		sysbuild_error ("%s neither directory or regular file", dir_name);

		return SYSBUILD_FAIL;
	}
}

/* UNIX WORLD */
int output_nodes (char *	root_name,
		  char *	dir_name)
{
	struct stat	statbuf;

	ObjNode 	objnode;

	sysbuild_debug ("@output_nodes (%s, %s)", root_name, dir_name);

	memset ((void *)(&objnode), 0, sizeof (ObjNode));

	if (stat (dir_name, &statbuf) != 0)
	{
		sysbuild_error ("output_nodes () - Failed to stat directory %s", dir_name);

		return SYSBUILD_FAIL;
	}

	if (!S_ISDIR (statbuf.st_mode))
	{
		sysbuild_error ("%s not a directory", dir_name);
	
		return SYSBUILD_FAIL;
	}

	strcpy (objnode.Name, root_name);
	objnode.Type = swap (Type_Directory);
	objnode.Flags = 0;
	objnode.Matrix = swap (MyDefDirMatrix);

	((DirNode *)(&objnode)) -> Nentries = swap (getdirsize (dir_name));

	if (swap (objnode.Size) == -1)
	{
		return SYSBUILD_FAIL;
	}

	output_node (objnode);

	return (output_lower_nodes (dir_name, TRUE));
}

int output_file (char *	file_name)
{
	struct stat	statbuf;

	int	size;
	int	extra;

	if (stat (file_name, &statbuf) != 0)
	{
		sysbuild_error ("output_file () - Failed to stat %s", file_name);

		return SYSBUILD_FAIL;
	}

	if ((RomInFp = fopen (file_name, "rb+")) == NULL)
	{
		sysbuild_error ("Failed to open %s", file_name);

		return SYSBUILD_FAIL;
	}

	size = statbuf.st_size;

	extra = (size & 3) ? (4 - (size & 3)) : 0;

	sysbuild_info ("\t\t[file: %s; size: 0x%08lx]", file_name, size);

	file_to_image (RomInFp, size, file_name);

	if (WriteRomDiskFile)
	{
		/* write out file to separate rom disk file */
		fseek (RomInFp, 0, SEEK_SET);

		while (size--)
		{
			putc (getc (RomInFp), RomDiskFp);
		}
	}

	align_image ();
	romfile_patch_space (extra);

	fclose (RomInFp); RomInFp = NULL;

	return SYSBUILD_OK;
}

int output_objects (char *	dir_name)
{
	struct stat	statbuf;
	int		size;

	char *	this_entry;
	char *	tailp;
	char	pathname[IOCDataMax];
	int	pos;

	if (stat (dir_name, &statbuf) != 0)
	{
		sysbuild_error ("output_objects () - Failed to stat directory %s", dir_name);

		return 0;
	}

	size = statbuf.st_size;

	if (size == 0)
	{
		return SYSBUILD_OK;
	}

	size += (size & 3) ? (4 - (size & 3)) : 0;

	pos = strlen (dir_name) - 1;	 /* points to last char in string */
	while (pos != 0 && dir_name[pos] != '/')
	{
		pos--;
	}
	if (dir_name[pos] == '/')	pos++;

	this_entry = &(dir_name[pos]);

	sysbuild_debug ("output_lower_nodes (%s) - this_entry = %s", dir_name, this_entry);

	strcpy (pathname, dir_name);
	strcat (pathname, "/");

	tailp = &(pathname[strlen(pathname)]);

	if (S_ISREG (statbuf.st_mode))
	{
		/* regular file */

		output_file (dir_name);

		return SYSBUILD_OK;
	}
	else if (S_ISDIR (statbuf.st_mode))
	{
		DIR *	dirp;

		struct dirent *	dir_entry;

		if ((dirp = opendir (dir_name)) == NULL)
		{
			sysbuild_error ("Failed to open %s", dir_name);

			return 0;
		}

		while ((dir_entry = readdir (dirp)) != NULL)
		{
			if (  strequ_(dir_entry -> d_name, ".")
			   || strequ_(dir_entry -> d_name, ".."))
			{
				/* ignore myself and parent */
				continue;
			}

			strcpy (tailp, dir_entry -> d_name);
			if (output_objects (pathname) == SYSBUILD_FAIL)
			{
				return SYSBUILD_FAIL;
			}
		}

		closedir (dirp);

		return SYSBUILD_OK;
	}
	else
	{
		sysbuild_error ("%s neither directory or regular file", dir_name);

		return SYSBUILD_FAIL;
	}
}

#endif


int buildromdisk (char *	root_name,
		  char *	dir_name,
		  char *	romdisk_name)
{
	ImageHdr	ihdr;
/*	struct Module	modhdr; */
	ubyte *		modhdr;
	int		modhdrsize;

	Module *	modp;

	int	entries;

	ubyte * module_start;

	sysbuild_debug ("buildromdisk (%s, %s, %s)\n", root_name, dir_name, romdisk_name);

	if (WriteRomDiskFile)
	{
		if ((RomDiskFp = fopen (romdisk_name, "wb+")) == NULL)
		{
			sysbuild_warning ("Failed to open rom disk file %s\n", romdisk_name);

			WriteRomDiskFile = FALSE;
		}
	}

	/*
	 * The size of the Module Header structure is different for different
	 * processors.  60 for C40/ARM's, 56 for TRAN's.
	 */
	modhdrsize = sizeof (struct Module);
#ifdef __SMT
	/* compiled to run on ARM's/C40's, only need to change size if for TRAN */
	if (strequ_(ConfigData.processor, "TRAN"))
	{
		modhdrsize -= 4;
	}
#else
	/* compiled to run on TRAN's, need to change size for ARM's and C40's */
	if (strequ_(ConfigData.processor, "ARM") || strequ_(ConfigData.processor, "C40"))
	{
		modhdrsize += 4;
	}
#endif

	modhdr = (ubyte *)(malloc (modhdrsize));
	memset ((void *)modhdr, 0, modhdrsize);

	 /*  Find number of entries */
	if ((entries = find_entries (dir_name)) == 0)
	{
		return SYSBUILD_FAIL;
	}

	entries++;	/* add one for the root node */

	sysbuild_debug ("Found %d entries", entries);

	/* Output patch space for ImageHdr ... */
	romfile_patch_space (sizeof (ImageHdr));

	/* Output patch space for Module Header */
	module_start = ImagePtr;
	patch_space (modhdrsize);
	romfile_patch_space (modhdrsize);

	/*
	 * Calculate where the first object in the romdisk will go.
	 * Address is calculated with respect to the rom disk, not
	 * the image file.
	 */
	ObjectOffset = ((word) entries * sizeof (ObjNode))
			+ modhdrsize;

	output_nodes (root_name, dir_name);

	output_objects (dir_name);

	align_image ();

	/*
	 * Patch the size part of ImageHdr and Module Header.
	 */

	if (WriteRomDiskFile)
	{
		ihdr.Magic = 0;
		ihdr.Flags = 0;
		ihdr.Size = swap (ObjectOffset);

		fseek (RomDiskFp, 0, SEEK_SET);

		fwrite ((void *)(&ihdr), 1, sizeof (ImageHdr), RomDiskFp);
	}

	modp = (Module *)(modhdr);

	modp -> Type  		= swap (T_RomDisk);
	modp -> Size  		= swap (ObjectOffset - sizeof (struct Module));
	strcpy (modp -> Name,	  ConfigData.romdisk_file);
	modp -> Id   		= swap (ModuleNumber);
	modp -> Version		= swap (0x201);
	modp -> MaxData		= 0;
	modp -> Init		= 0;

	do_patch (module_start, modhdr, modhdrsize);

	if (WriteRomDiskFile)
	{
		fwrite ((void *)(modhdr), 1, modhdrsize, RomDiskFp);

		fclose (RomDiskFp);
		RomDiskFp = NULL;
	}

	return SYSBUILD_OK;
}

void add_romdisk ()
{
	ubyte *	mod_start = ImagePtr;	/* start of rom disk */

	int	mod_offset = ImageSize;	/* for info only */

	if (ConfigData.romdisk_dir[0] == '\0')
	{
		sysbuild_debug ("add_romdisk () - romdisk_dir not specified");

		if (ConfigData.romdisk_file[0] != '\0')
		{
			/* Rom disk file already made */
			add_module (ConfigData.romdisk_file);
		}
		return;
	}

	if (ConfigData.romdisk_write == 'Y')
	{
		WriteRomDiskFile = 1;

		sysbuild_info ("%s - ", ConfigData.romdisk_file);
	}
	else
	{
		WriteRomDiskFile = 0;

		sysbuild_info ("[internal romdisk] - ", ConfigData.romdisk_file);
	}

	buildromdisk (ConfigData.romdisk_root,
		      ConfigData.romdisk_dir,
		      ConfigData.romdisk_file);

	patch_vector (mod_start);

	sysbuild_info ("\t\toffset: 0x%08lx;\tsize: 0x%08lx",
				mod_offset, ObjectOffset);

	return;
}

#ifdef SUN4
void ___typeof( void ) {}
void ___assert( void ) {}
#endif
