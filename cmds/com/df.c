
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/df.c,v 1.5 1992/06/27 11:47:13 paul Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <syslib.h>
#include <servlib.h>

int main(int argc, char **argv)
{ WORD buffer[IOCDataMax/sizeof(word)], result;
  Object *drive;
  int i;

  printf("          Size(Kb)      Used(Kb)     Available(Kb)\n");

  if (argc == 1)
    { if ((result = ServerInfo(cdobj(), (BYTE *) buffer)) < 0)
	{ printf("ServerInfo failed : %lx\n", result);
	  exit(1);
        }    
      printf("      %10ld    %10ld      %10ld\n", buffer[1] / 1024, 
             (buffer[1] - buffer[2]) / 1024, buffer[2] / 1024);
    }

  for (i = 1; i < argc; i++)
    { if ((drive = Locate(cdobj(), argv[i])) == (Object *) NULL)
        { printf("Unable to locate %s.\n", argv[i]);
          continue;
        }
      if ((result = ServerInfo(drive, (BYTE *) buffer)) < 0)
        { printf("ServerInfo failed : %lx\n", result);
          continue;
        }

      printf("      %10ld    %10ld      %10ld\n", buffer[1] / 1024, 
             (buffer[1] - buffer[2]) / 1024, buffer[2] / 1024);
    }
return 0;
}

        
       
 
