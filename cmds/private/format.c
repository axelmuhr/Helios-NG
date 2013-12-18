/*------------------------------------------------------------------------
--									--
-- FORMAT.C								--
--									--
-- Author : BLV 27/5/88							--     
--									--
------------------------------------------------------------------------*/
/* SccsId: 1.2 19/7/88	 Copyright (C) 1988, Perihelion Software Ltd.	*/

static char *rcsid = "$Header: /hsrc/cmds/private/RCS/format.c,v 1.2 1990/08/23 10:15:40 james Exp $";

#include <syslib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <nonansi.h>

#define eq ==
#define ne !=

static void usage(char *);
static void format(char *, int, int, char *);

int main(int argc, char **argv)
{ char label[80];
  char *name = NULL, *current_arg;
  int  i, sides = 2, tracks = 80, temp;
  label[0] = '\0';

  for (i=1; i < argc; i++)
    { current_arg = argv[i];
      if (*current_arg eq '-')
       { if (*(++current_arg) eq 'n')
           strcpy(label, ++current_arg);
         else
         if (*current_arg eq 's')
           { temp = atoi(++current_arg);
             if (temp eq 0)
               { if (!strcmp(current_arg, "single"))
                   sides = 1;
                 else
                 if (!strcmp(current_arg, "double"))
                   sides = 2;
                 else
                  usage("Invalid argument near -s");
               }
             else
             if (temp < 1 || temp > 2)
               usage("Invalid argument near -s");
             else
               sides = temp;
           }
         else
         if (*current_arg eq 't')
           { temp = atoi(++current_arg);
             if (temp ne 40 && temp ne 80)
              usage("Invalid argument near -t");
             else
              tracks = temp;
           }
         else
          { sprintf(label, "Unknown option %s", argv[i]);
            usage(label);
          }
       }
     else
      if (name ne NULL)
        usage("Duplicate name");  /* a name has been given already */
      else
        name = argv[i];
    }

   if (name eq NULL)
     usage("No drive specified");

   format(name, sides, tracks, label);

   return 0;
}

static void usage(char *message)
{ printf("%s : usage FORMAT name [-t40/80] [-s1/2] [-nlabel]\n", message);
  exit(1);
}

/**
*** To format a floppy, use 16 Create requests. The object and name specify the
*** drive to be formatted. The size and data give the label for the floppy.
*** The type differs for each Create request : it consists of a magic number in
*** the top three bytes, followed by a nibble indicating the sides and tracks,
*** followed by a nibble indicating the number of the format.
**/

#define format_mask    0xFFFFFF00
#define format_magic	0x659A2B00
#define format_single   0x00000080
#define format_double   0x00000000
#define format_40       0x00000040
#define format_80       0x00000000

static void format(char *name, int sides, int tracks, char *label)
{ int i;
  int type = format_magic + ((sides eq 1) ? format_single : format_double) +
             ((tracks eq 40) ? format_40 : format_80);
  Object *temp;

  temp = Locate(CurrentDir, name);
  if (temp ne Null(Object))
    Close(temp);

  for (i = 0; i < 16; i++)
   { if ((temp = Create(CurrentDir, name, type + i, strlen(label) + 1, label)) 
          eq Null(Object))
        { printf("Format failed : %x\n", Result2(CurrentDir));
          exit(1);
        }
      Close(temp);
      putchar('.'); fflush(stdout);
   }   
  printf(" Format done.\n");
}
