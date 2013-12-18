/**
*
* Title:  tee
*
* Author: Andy England
*
* Date:   29th April 1988
*
**/

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/tee.c,v 1.4 1993/07/12 11:31:52 nickc Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define unless(c) if(!(c))
#define until(c)  while(!(c))
#define strequ(s,t) (strcmp(s,t) == 0)

typedef enum
{
  FALSE,
  TRUE
} BOOLEAN;

#define MAX_FILES 20

FILE *Files[MAX_FILES];
int FileCount = 0;

void Tidyup()
{
  int Count;

  for (Count = 0; Count < MAX_FILES; Count++)
  {
    unless (Files[Count] == NULL) fclose(Files[Count]);
  }
}

void Initialise(
int argc,
char *argv[] )
{
  char *Command = *argv++;
  BOOLEAN Append = FALSE;
  int Count;

  for (Count = 0; Count < MAX_FILES; Count++) Files[Count] = NULL;
  if(argc > 1)
     if (strequ(*argv, "-a"))
     {
       Append = TRUE;
       argv++; argc--;
     }
  while (--argc > 0)
  {
    char *FileName = *argv++;

    if (FileCount == MAX_FILES)
    {
      fprintf(stderr, "%s: Too many files\n", Command);
      Tidyup();
      exit(1);
    }
    if ((Files[FileCount++] = fopen(FileName, (Append ? "a" : "w"))) == NULL)
    {
      fprintf(stderr, "%s: Can't open %s\n", Command, FileName);
      Tidyup();
      exit(1);
    }
  }
}

void Tee()
{
  int Count;
  int c;

  until ((c = fgetc(stdin)) == EOF)
  {
    fputc(c, stdout);
    for (Count = 0; Count < FileCount; Count++) fputc(c, Files[Count]);
  }
}

int
main(
int argc,
char *argv[] )
{
  Initialise(argc, argv);
  Tee();
  Tidyup();
  return 0;
}

