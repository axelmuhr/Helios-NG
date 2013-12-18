
#include <stdio.h>
#include <syslib.h>
#include <stdlib.h>
#include <string.h>
#include <posix.h>
#include <gsp.h>
#include <unistd.h>

char lname[100];
char path[100];
int  load_immediately = 0;

void usage(void)
{
	fprintf(stderr, "usage: cache [-m mc] [-l] cmd...\n");
	exit(1);
}

int main(int argc, char **argv)
{
	Object *loader, *prog;
	char *cmd;
	char *mc = NULL;
	
	if (argc < 2) usage();
	argv++;

	while (**argv == '-')
	 { if ((*argv)[1] == 'm')
	    {
		mc = (*argv) + 2;
		if (*mc == '\0') mc = *(++argv);
		argv++;	
	    }
	   else if ((*argv)[1] == 'l')
	    { load_immediately = 1; argv++; }
           else
            usage();
         }
	if (*argv == NULL) usage();

	strcpy(lname, "/");
	if (mc != NULL)
	{
		strcat(lname, mc);
		strcat(lname, "/");
	}
	strcat(lname, "loader");

	if ((loader = Locate(NULL, lname)) == NULL) 
	{
		fprintf(stderr, "Cannot locate loader %s\n", lname);
		exit(20);
	}
			
	while ((cmd = *argv++) != NULL)
	{
		find_file(path, cmd);

		  /* Get to final part of name */
		{ char *tmp = &(cmd[strlen(cmd)]);
		  while ((*tmp != '/') && (tmp >= cmd)) tmp--;
		  cmd = ++tmp;
		}

		if ((prog = Locate(CurrentDir, path)) == NULL) 
		 { Object *HeliosBin = Locate(Null(Object), "/helios/bin");
		   if (HeliosBin != Null(Object))
		    prog = Locate(HeliosBin, cmd);
		   Close(HeliosBin);
		 }
		if (prog == Null(Object))
		{
			fprintf(stderr, "Cannot locate %s\n", cmd);
			continue;
		}

		if (prog->Type & Type_Directory)
		{
			fprintf(stderr, "Cannot cache directory %s\n", cmd);
			continue;
		}
		
		if (Link(loader, cmd, prog) < 0 )
		{
			fprintf(stderr, "Cannot cache %s\n", cmd);
			continue;
		}

		if (load_immediately)
		 { Stream	*stream = Open(loader, cmd, O_ReadOnly);
		   if (stream != Null(Stream)) Close(stream);
		 }
	}
	return 0;
}

