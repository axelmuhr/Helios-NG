
/*
 * util.c
 *
 *	Generally useful functions.
 *
 */

#include <stdio.h>

#ifdef UNIX

#include "/usr/include/sys/types.h"
#include "/usr/include/sys/stat.h"

#else

#include "/hsrc/include/nonansi.h"

#endif

#include "defs.h"
#include "externs.h"

void capitalise (char *	str)
{
	int	i;

	for (i = 0; str[i] != '\0'; i++)
	{
		if (str[i] >= 'a' && str[i] <= 'z')
		{
			str[i] = str[i] - 'a' + 'A';
		}
	}
}

#ifdef __HELIOS
int getfilesize (char *	file_name)
{
	FILE *	fp;
	int	size;

	if ((fp = fopen (file_name, "r")) == NULL)
	{
		sysbuild_warning ("Failed to open %s", file_name);

		return -1;
	}

	size = (int)GetFileSize (Heliosno (fp));

	fclose (fp);

	size += (size & 3) ? (4 - (size & 3)) : 0;

	return size;
}

#else

int getfilesize (char *	file_name)
{
	struct stat	statbuf;

	int	size;

	if (stat (file_name, &statbuf) != 0)
	{
		sysbuild_warning ("getfilesize () - Failed to stat %s", file_name);

		return 0;
	}

	size = statbuf.st_size;

	/* round to nearest word */
	size += (size & 3) ? (4 - (size & 3)) : 0;

	return size;
}

#endif
