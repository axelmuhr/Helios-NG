/**
*
* Title:  CDL Compiler - Administration.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988 - 1992, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/
#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/cdl/RCS/cdl.c,v 1.4 1993/04/14 17:19:04 nickc Exp $";
#endif

#include "cdl.h"
#include <stdarg.h>

#ifndef __HELIOS
#include <sys/stat.h>
#endif


BOOL compiling = FALSE;
BOOL debugging = FALSE;
BOOL expanding = FALSE;
BOOL listing = FALSE;
BOOL locating = TRUE;
BOOL memorycheck = FALSE;
BOOL parseonly = FALSE;
int ch;
TOKEN token;
int errorcount;
int linenumber;
char *filename = "<stdin>";
char *cmdname;
FILE *inputfile = stdin;
static char *outputfile = Null(char);
FILE *listingfile = stderr;
char **arguments;
int argcount;
CMD *globcmd = NULL;

extern void freedata( void );
extern void errorline( void );

int main(int argc, char *argv[])
{ int	exit_status = 0;
  initialise(argc, argv);
  exit_status = cdl();
  tidyup();
  return ((exit_status == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
}

int cdl()
{
  if ((globcmd = parse()) == NULL)
   return(EXIT_FAILURE);
  else
  {
    globcmd = buildtaskforce(globcmd);
    if (errorcount != 0)
     return(EXIT_FAILURE);
    else
    {
      if (listing) putlisting(globcmd, listingfile);
      unless (parseonly)
      {
        if (compiling)
         { FILE *output;
           if (outputfile == Null(char))
#ifdef __HELIOS
            output = freopen(Heliosno(stdout)->Name, "wb",stdout) ;
#else
	   output = stdout;
#endif
           else
            { output = fopen(outputfile, "wb");
              if (output == Null(FILE))
               { fprintf(stderr, "Unable to open %s for output.\n", outputfile);
                 exit(1);
               }
            }
           return(putcode(output));
         }
        else return(exectaskforce());
      }
      return EXIT_SUCCESS;
    }
  }
}

void usage()
{
  fprintf(stderr, "usage: cdl [-i -l <listing> -n -o <object>] [<source>] [<arguments>]\n");
}

void initialise(int argc, char *argv[])
{
  errorcount = 0;
  linenumber = 1;
  cmdname = *argv++; argc--;
  while (argc > 0 AND **argv == '-')
  {
    char c;
    char *name;
    char *arg = *argv++; argc--;

    arg++;
    until ((c = *arg++) == '\0')
    {
      switch (c)
      {
        case 'c':
        compiling = TRUE;
        continue;

        case 'd':
        debugging = TRUE;
        continue;

        case 'e':
	expanding = TRUE;
        continue;

        case 'f':
        locating = FALSE;
        continue;

        case 'i':
        listing = TRUE;
        continue;

        case 'l':
        listing = TRUE;
        if (*arg == '\0')
        {
          if (argc == 0)
          {
            fprintf(stderr, "Missing filename for '-l' option.\n");
            exit(1);
          }
          name = *argv++; argc--;
        }
        else name = arg;
        if ((listingfile = fopen(name, "w")) == NULL)
        {
          fprintf(stderr, "Unable to open '%s' for output.\n", name);
          exit(1);
        }
        break;

        case 'm':
        memorycheck = TRUE;
        continue;

        case 'n':
        parseonly = TRUE;
        continue;

        case 'o':
        compiling = TRUE;
        if (*arg == '\0')
        {
          if (argc == 0)
          {
            fprintf(stderr, "Missing filename for '-o' option.\n");
            exit(1);
          }
          outputfile = *argv++; argc--;
        }
        else outputfile = arg;

        break;

        default:
        fprintf(stderr, "Invalid option '-%c'.\n", c);
        usage();
        exit(1);
        continue;
      }
      break;
    }
  }
  if (argc > 0)
  {
    filename = *argv;
    if ((inputfile = fopen(filename, "r")) == NULL)
    {
      fprintf(stderr, "Unable to open '%s' for input.\n", filename);
      exit(1);
    }
  }
  arguments = argv;
  argcount = argc - 1;
  initdata();
  if (compiling || parseonly) signon();
  if (expanding)
   { fprintf(stderr, "Warning : -e option is not supported in this release.\n");
     expanding = FALSE;
   }
}

void tidyup(void)
{ 
  DEBUG("tidyup()");
  freecmd(globcmd);
  freedata();
  fclose(inputfile);
  if (compiling || parseonly) signoff();
}

void fatal(char *format, ...)
{
  va_list args;

  errorcount++;
  fprintf(stderr, "%s: ", filename);
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, ".\n");
  tidyup();
  exit(1);
}

void error(char *format, ...)
{
  va_list args;

  errorcount++;
  fprintf(stderr, "%s, Error: ", filename);
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, ".\n");
}

void synerr(char *format, ...)
{
  va_list args;

  errorcount++;
  errorline();
  fprintf(stderr, "%s, line %d, Error: ", filename, linenumber);
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, ".\n");
}

void warning(char *format, ...)
{
  va_list args;

  errorline();
  fprintf(stderr, "%s, line %d, Warning : ", filename, linenumber);
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, ".\n");
}

void bug(char *message)
{
  fprintf(stderr, "COMPILER ERROR: %s.\n", message);
  tidyup();
  exit(1);
}

void report(char *format, ...)
{
  va_list args;

  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, ".\n");
  fflush(stderr);
}

void signon(void)
{
  fprintf(stderr, "Helios CDL compiler  Version 2.04\n");
  fprintf(stderr, "(c) Copyright 1988-1992, Perihelion Software Ltd.\n");
  fprintf(stderr, "All Rights Reserved.\n");
}

void signoff(void)
{
  if (errorcount == 0)
    fprintf(stderr, "%s: Compilation successful\n", filename);
  else
  {
    fprintf(stderr, "%s: Compilation failed, ", filename);
    if (errorcount == 1) fprintf(stderr, "1 error.\n");
    else fprintf(stderr, "%d errors.\n", errorcount);
  }
}

char *locatecmd(char *name)
{
  static char path[PATH_MAX + 1];
#ifdef __HELIOS
  Object *object;
#else
  struct stat stat_buf;
#endif

  if (isabspath(name) || (name[0] == '.'))
  {
#ifdef __HELIOS
    unless ((object = Locate(CurrentDir, name)) == NULL)
      {
	strcpy(path, object->Name);
	Close(object);
	return path;
      }
#else
    unless (stat(name, &stat_buf) == 0)
      {
	strcpy( path, name );
	return path;
      }
#endif
  }
  else
  {
    char *pathenv;

    unless ((pathenv = getenv("PATH")) == NULL)
    {
      until (*pathenv == '\0')
      {
        int index = 0;

        until (*pathenv == ':' OR *pathenv == '\0') path[index++] = *pathenv++;
        path[index++] = '/';
        strcpy(path + index, name);
#ifdef __HELIOS
        unless ((object = Locate(CurrentDir, path)) == NULL)
        {
          strcpy(path, object->Name);
          Close(object);
          return path;
        }
#else
        unless (stat(path, &stat_buf) == 0)
	  {
	    return path;
	  }
#endif
        if (*pathenv == '\0') break;
        pathenv++;
      }
    }
  }
  return NULL;
}

