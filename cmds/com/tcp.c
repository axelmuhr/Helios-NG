/*
 * this is a quick hack at a text copy program
 * necessary because cp does a binary copy which does
 * not work properly when copying text files across
 * different filing systems (eg ATARI to RAM disk).
 */

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/tcp.c,v 1.3 1992/08/12 13:55:22 craig Exp $";

#include <stdio.h>
#include <stdarg.h>
#include <syslib.h>
#include <string.h>
#include <stdlib.h>
#include <gsp.h>

static char *progname;


static void 
fail( char *s, ... )
/*
 * abort with the provided message
 */
{
	va_list ap;

	va_start(ap,s);
	vfprintf(stderr, s, ap);
	va_end(ap);

	exit(1);

} /* fail */


static void 
usage( void )
/*
 * explain how to use the program
 */
{
	fail("Args: %s src dst, or %s src [src] dstdir\n",
		progname, progname);

} /* usage */


static bool
isdir( char *path )
/*
 * returns true if the file pointed to by 'path' is a directory
 */
{
	Object 		*o = Locate(CurrentDir, path);

	return (o == Null(Object) ? FALSE : (o->Type == Type_Directory));

} /* isdir */


#define BUFFERSIZE	10240

static buffer[BUFFERSIZE];

static void 
filecopy( char *src, char *dst )
/*
 * copy the file pointed to by 'src' into 'dst' (overwriting 'dst')
 */
{
	FILE 	*in, *out;
	int	read, written;
	

	if (isdir(src))
	{
		fail("Cannot copy from directory\n");
	}

	if ((in = fopen(src,"r")) == Null(FILE))
	{
		fail("File %s not found\n", src);
	}

	if ((out = fopen(dst,"w")) == Null(FILE))
	{
		fail("Failed to open %s for writing\n", dst);
	}

	do
	{
		if ((read = fread(buffer, 1, BUFFERSIZE, in)) < 1)
		{
			break;
		}

		if ((written = fwrite(buffer, 1, read, out)) < 1)
		{
			fail("Write failed\n");
		}

	}
	while (read >= BUFFERSIZE);

	fclose(in);
	fclose(out);

} /* filecopy */


#define NAMESIZE 1024

static void 
dircopy( int number, char **names, char *dir )
/*
 * copy the 'number' files contained in the array 'names'
 * into the directory 'dir'
 */
{
	int 	i,
		namep;
	char 	dst[NAMESIZE],
		*src,
		*base;


	if (strlen(dir) >= NAMESIZE - 1)
	{
		fail("INCREASE NAMESIZE!\n");
	}

	/* calculate root directory name */
	
	strcpy(dst, dir);
	strcat(dst, "/");
	namep = strlen(dst);

	for (i = 0 ; i < number ; i++)
	{
		src = names[i];
		
		if (namep + strlen(src) >= NAMESIZE)
		{
			fail("INCREASE NAMESIZE\n");
		}

		if ((base = strrchr( src, '/')) == Null(char))
		{
			base = src;
		}
/*
-- crf: 12/08/92 - Bug 889
-- Problem when src is an absolute pathname.
-- e.g. tcp /c/users/craig/cshrc /a
-- At this point
--	- dst will point to  "/a/"
--	- base will point to "/cshrc"
-- When the absolute destination path is constructed (using strcpy() below),
-- dst will point to "/a//cshrc".  This causes fopen() in filecopy() to
-- fail. Therefore, necessary to increment base in this case.
*/
		else
		{
			base ++ ;
		}

		strcpy(dst + namep, base);

		filecopy(src, dst);
	}

} /* dircopy */


int
main(int argc, char **argv)
{
	progname = argv[0];

	if (argc < 3) usage();
 
	if (isdir(argv[argc - 1]))
	{
		dircopy(argc - 2, argv + 1, argv[argc - 1]);
	}
	else
	{
		if (argc > 3) usage();

		filecopy(argv[1], argv[2]);
	}

	return (0);
} /* main */
