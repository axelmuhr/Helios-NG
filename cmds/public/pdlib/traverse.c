#ifndef lint
static char *RCSid = "$Header: traverse.c,v 1.1 87/08/21 16:34:28 rnovak Exp $";
#endif

/*
 *------------------------------------------------------------------
 *
 * $Source: /u3/syseng/rnovak/src/lib/RCS/traverse.c,v $
 * $Revision: 1.1 $
 * $Date: 87/08/21 16:34:28 $
 * $State: Exp $
 * $Author: rnovak $
 * $Locker:  $
 *
 *------------------------------------------------------------------
 * $Log:	traverse.c,v $
 * Revision 1.1  87/08/21  16:34:28  rnovak
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

/*LINTLIBRARY*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>

#ifdef MAXNAMLEN

#define	namedir(entry) (entry->d_name)
#define	MAXNAME 256

#else

#define	DIR	FILE
#define	MAXNAME (DIRSIZ+2)
#define	opendir(path) fopen (path, "r")
#define closedir(dirp) fclose (dirp)
struct direct *
readdir (dirp)
DIR 	*dirp;
	{
	static	struct	direct	entry;
	if (dirp == NULL) return (NULL);
	for (;;)
		{
		if (fread (&entry, sizeof (struct direct), 1, dirp) == 0) return (NULL);
		if (entry.d_ino) return (&entry);
		}
	}
char	*strncpy ();
char *
namedir (entry)
struct	direct	*entry;
	{
	static	char	name[MAXNAME];
	return (strncpy (name, entry->d_name, DIRSIZ));
	}

#endif

#include <sys/stat.h>
#define	isdir(path) (stat(path, &buf) ? 0 : (buf.st_mode&S_IFMT)==S_IFDIR)

traverse (path, func)
char	*path;
int 	(*func) ();
	{
	DIR 	*dirp;
	struct	direct	*entry;
	struct	stat	buf;
	int 	filetype = isdir (path) ? 'd' : 'f';
	(*func) (path, filetype, 0);
	if (filetype == 'd')
		{
		if (chdir (path) == 0)
			{
			if (dirp = opendir ("."))
				{
				while (entry = readdir (dirp))
					{
					char	name[MAXNAME];
					(void) strcpy (name, namedir (entry));
					if (strcmp(name, ".") && strcmp(name, ".."))
						traverse (name, func);
					}
				(void) closedir(dirp);
				}
			(void) chdir ("..");
			}
		}
	(*func) (path, filetype, 1);
	}

#ifdef STANDALONE

static	Indent = 0;
tryverse (file, type, pos)
char	*file;
	{
	int 	in;
	if (pos == 0)
		{
		for (in = 0; in < Indent; in++) putchar ('\t');
		if (type == 'd')
			{
			printf ("%s/\n", file);
			Indent++;
			}
		else puts (file);
		}
	else if (type == 'd') Indent--;
	}

main (argc, argv) char **argv;
	{
	int 	tryverse ();
	char	*root = argc > 1 ? argv[1] : ".";
	traverse (root, tryverse);
	}
#endif
