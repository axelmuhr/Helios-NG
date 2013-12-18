/**
*
* Title:  which
*
* Author: Andy England
*
* Date:   29th April 1988
*
**/

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/which.c,v 1.4 1993/07/12 11:48:25 nickc Exp $";

#include <helios.h>
#include <syslib.h>
#include <gsp.h>
#include <stdio.h>
#include <string.h>
#include <posix.h>
#include <limits.h>

typedef int BOOL;

#define AND &&
#define OR  ||

int which(char *basename);
void formfilename(char *name, char *path, char *basename);
BOOL findfile(char *name);

int main(int argc, char **argv)
{
  int error = 0;

  while (--argc > 0) error += which(*++argv);
  return error;
}

static char name[PATH_MAX + 1];
static char path[PATH_MAX + 1];

int which(char *basename)
{
  char *value, *pathenv;

  unless ((pathenv = value = getenv("PATH")) == NULL)
  {
    int c;

    forever
    {
      int index = 0;

      until ((c = *value++) == ':' OR c == '\0') path[index++] = c;

      path[index] = '\0';
      formfilename(name, path, basename);

      if (findfile(name))
      {
        return 0;
      }

      unless (c == ':') break;
    }
    printf("no %s in ", basename);
    until ((c = *pathenv++) == '\0')
    {
      if (c == ':') putchar(' ');
      else putchar(c);
    }
    putchar('\n');
  }
  return 1;
}

void formfilename(char *name, char *path, char *basename)
{
  if (path[0] == '\0' OR basename[0] == '/') strcpy(name, basename);
  else
  {
    strcpy(name, path);
    strcat(name, "/");
    strcat(name, basename);
  }
}

BOOL findfile(char *name)
{
  Object *cmd;

  if ((cmd = Locate(CurrentDir, name)) == NULL)
    return FALSE;
  Close(cmd);
  if ( cmd->Type & Type_Directory )
    printf("%s is a directory\n", name);
  else
    printf("%s\n",name);
  return TRUE;
}

