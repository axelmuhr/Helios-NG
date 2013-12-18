/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 19 August 1992                                               */
/* File: hl.c                                                         */
/*                                                                    */
/* 
 * This creates a hard link to a target object on a UFS file server.
 */
/* $Id: hl.c,v 1.1 1992/09/16 10:01:43 al Exp $ */
/* $Log: hl.c,v $
 * Revision 1.1  1992/09/16  10:01:43  al
 * Initial revision
 * */

#include <helios.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslib.h>
#include <strings.h>

char *progname;
int link(char *target, char *linkname);

void usage()
{
	fprintf(stderr,"usage: %s filename [linkname]",progname);
	Exit(1);
}

/* Return the unit name within a path */
char *unit(char *name)
{	char *last;
	
	if ((last = strrchr(name,'/')) == NULL) 
		last = name;
	else	last++;
	return(last);
}

int main(int argc, char *argv[])
{
	progname = unit(argv[0]);

	if (argc == 2) {
		if (link(argv[1],unit(argv[1]))) {
			(void)fprintf(stderr,"%s: %s: %s\n", 
					progname, argv[1], strerror(errno));
			exit(1);
		}
	} else if (argc == 3) {
		if (link(argv[1],argv[2])) {
			(void)fprintf(stderr,"%s: %s %s: %s\n", 
					progname, argv[1], argv[2], strerror(errno));
			exit(1);
		}
	} else usage();

	return(0);
}
