/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   I N S T A L L A T I O N   U T I L I T Y      --
--           -----------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- hfree.c								--
--                                                                      --
--	Author:  MJT 27/11/91						--
--                                                                      --
------------------------------------------------------------------------*/

static char *rcsid = "$Header: /hsrc/text/RCS/hfree.c,v 1.1 1991/11/28 13:18:45 martyn Exp $";

/**
*** This program is run from the Helios installation script, under MSDOS
*** It determines the amount of free disk space on the specified drive.
*** In addition, the style of the directory supplied is indicated by the
*** exit code e.g. 
***
***	c:		exit 1
***	.		exit 1
***	c:\fred 	exit 0
***	\fred 		exit 0
***	c:fred 		exit 0
***	fred 		exit 0
***
*** usage:
***
***	hfree k_space_needed [directory]
***
*** exit 2 if not enough space
***
**/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define EXIT_DIRECTORY	0
#define EXIT_DRIVE	1
#define EXIT_NOSPACE	2

main(argc, argv)
int argc;
char **argv;
{
	struct diskfree_t dfree;
	long space, hfree;
	unsigned sect, clust, bytes;
	unsigned drivenum;
	char drive;
	int exit_status = EXIT_NOSPACE;

	if(argc < 2)
		exit(exit_status);	/* bad call */

	space = atol(argv[1]);		/* space needed */

	if(space <= 0)
		exit(exit_status);	/* bad size */

	if(argc == 3)			/* directory given */
		{
		exit_status = EXIT_DRIVE;

		if(strlen(argv[2]) == 1)	/* single char or '.' */
			{
			drive = '.';
			if(argv[2][0] != '.')	/* single char */
				exit_status = EXIT_DIRECTORY;
			}
		else if(argv[2][1] == ':')	/* drive specified */
			{
			drive = toupper(argv[2][0]);
			if(argv[2][2])		/* drive + name */
				exit_status = EXIT_DIRECTORY;
			}
		else
			{
			drive = '.';
			exit_status = EXIT_DIRECTORY;
			}
		}
	else				/* no directory - use current */
		{
		drive = '.';
		exit_status = EXIT_DRIVE;
		}

	if(drive == '.')
		{
		drivenum = 0;
		_dos_getdrive((unsigned *)&drive);	/* get current drive */
		drive += 'A' - 1;
		}
	else
		drivenum = drive - 'A' + 1;

	if(drivenum < 0)
		exit(EXIT_NOSPACE);		/* bad drive */

	_dos_getdiskfree(drivenum, &dfree);

	sect = dfree.sectors_per_cluster;
	clust = dfree.avail_clusters;
	bytes = dfree.bytes_per_sector;

#ifdef DEBUG
	printf("Drive %c (%d) - %d %d %d\n", drive, drivenum, clust,	sect, bytes);
#endif

	hfree = (long) ((long)clust * (long)sect	* (long)bytes);

	printf("Drive %c - %ld bytes free\n", drive,	hfree);

	hfree /= (long)1024;

	if(hfree < space)
		exit(EXIT_NOSPACE);	/* not enough space */

	exit(exit_status);		/* space OK */
}
