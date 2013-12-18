/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/07/14 14:14:31 $
 * Revising $Author: nickc $
 */

/* 
 * C compiler support file StripHdrs.c
 * Copyright (C) Codemist Ltd, 1987.
 * Copyright (C) Acorn Computers Ltd., 1988
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#define  MAXPATHS        10
#define  MAXEXCEPTIONS   10
#define  MAXFILENAMESZ  256
#define  MAXLINELEN     256
static char *headers =   "";
static char *viafile = 0;

static lastchar = 0;
static npaths = 0;
static int nerrors = 0;
static char *paths[MAXPATHS];
static char file_name[MAXFILENAMESZ];

static void outch(ch, fh)
int ch;
FILE *fh;
{
    if (lastchar==ch) return;
    if (lastchar=='\n')
    {   /*putc('\n', fh);*/
        lastchar = ' ';
    }
    else lastchar = 0;
    switch (ch)
    {
case '\\':
case '\'':
case '\"':
        fprintf(fh, "%c", ch);
        return;
case '\n':
        fprintf(fh, "\n");
        lastchar = '\n';
        return;
case ' ':
        lastchar = ' ';
default:
        putc(ch, fh);
        return;
    }
}

/* The following are states in a machine that detects & deletes comments */
#define NORMAL   0x0
#define STRING   0x1
#define CHAR     0x2
#define STRBACK  0x3
#define CHARBACK 0x4
#define SLASH    0x5
#define COMMENT  0x6
#define STAR     0x7

static FILE *path_open(name, mode)
char *name, *mode;
{
  FILE *f;
  int j;

  for (j = 0;  j < npaths;  ++j) {
    strcpy(file_name, paths[j]);
    strcat(file_name, name);
    f = fopen(file_name, mode);
    if (f != NULL) return f;
  }
  return NULL;
}

static void copy_header(fh, name)
FILE *fh;
char *name;
{
    FILE *fh1;
    int ch, state = NORMAL;
    char year[32];
    time_t now;
    char ansi_name[256];

    fh1 = path_open(name, "r");
    if (fh1==NULL) {
      fprintf(stderr, "Unable to read input file %s\n", name);
      ++nerrors;
      return;
    }
    strcpy(ansi_name,headers);
    strcat(ansi_name, name);

    fprintf(stderr, "StripHdrs: Copying %s as %s\n", file_name, ansi_name);

    now = time(NULL);
    strftime(year, sizeof(year), "%Y", localtime(&now));
    fprintf(fh, "\
/* %s.h\n\
 * Copyright (C) Acorn Computers Ltd., %s\n\
 * Copyright (C) Codemist Ltd., %s\n\
 */\n",
        name, year, year);  

    lastchar = '\n';

    while ((ch=getc(fh1))!=EOF)
    {
        switch (state)
        {
    case NORMAL:
            switch (ch)
            {
        case '\'':  outch(ch, fh);  state = CHAR;   break;
        case '\"':  outch(ch, fh);  state = STRING; break;
        case '/':                   state = SLASH;  break;
        default:    outch(ch, fh);                  break;
            }
            break;
    case STRING:
            switch (ch)
            {
        case '\"':  outch(ch, fh);  state = NORMAL; break;
        case '\\':  outch(ch, fh);  state = STRBACK;break;
        default:    outch(ch, fh);                  break;
            }
            break;
    case CHAR:
            switch (ch)
            {
        case '\'':  outch(ch, fh);  state = NORMAL; break;
        case '\\':  outch(ch, fh);  state = CHARBACK;break;
        default:    outch(ch, fh);                  break;
            }
            break;
    case STRBACK:
            outch(ch, fh);
            state = STRING;
            break;
    case CHARBACK:
            outch(ch, fh);
            state = CHAR;
            break;
    case SLASH:
            switch (ch)
            {
        case '*':                   state = COMMENT;break;
        case '\'':  outch('/', fh);
                    outch(ch, fh);  state = CHAR;   break;
        case '\"':  outch('/', fh);
                    outch(ch, fh);  state = STRING; break;
        default:    outch('/', fh);
                    outch(ch, fh);  state = NORMAL; break;
            }
            break;
    case COMMENT:
            if (ch == '*') state = STAR;
            break;
    case STAR:
            switch (ch)
            {
        case '/':                   state = NORMAL; break;
        case '*':                                   break;
        default:                    state = COMMENT;break;
            }
            break;
    default:
            fprintf(stderr, "\nBad state %d\n", state);
            exit(1);
        }
    }
    if (state!=NORMAL || lastchar!='\n') {
        fprintf(stderr, "\nUnexpected end of file in %s?\n", file_name);
        ++nerrors;
    }
    fclose(fh1);
}

int main(argc, argv)
int argc;
char *argv[];
{
    int j;
    char *arg;
    FILE *fh, *vf;
    char stripped_file[256];
    char hdrfile[256];

    nerrors = npaths = 0;
    for (j = 1;  j < argc;  ++j) {
      arg = argv[j];
      if (arg[0] == '-') {
        switch (arg[1]) {
case 'h':
case 'H':
          break;
case 'i':
case 'I': if (npaths >= MAXPATHS) {
            fprintf(stderr,
              "StripHdrs: Too many paths - only %u allowed\n", MAXPATHS);
            exit(1);
          } else {
            arg += 2;
            if (*arg == 0) arg = argv[++j];
            paths[npaths++] = arg;
          }
          break;
case 'o':
case 'O': arg += 2;
          if (*arg == 0) arg = argv[++j];
          headers = arg;
          break;
case 'f':
case 'F': arg += 2;
          if (*arg == 0) arg = argv[++j];
          viafile = arg;
default:
          break;
        }
      }
    }

    fprintf(stderr, "StripHdrs: Creating %s...\n", headers);

    if (nerrors == 0)
    {   for (j = 1;  j < argc;  ++j) {
        arg = argv[j];
        strcpy(stripped_file,headers);
        strcat(stripped_file,arg);
        if (arg[0] != '-')
        {
          fh = fopen(stripped_file, "w");
          copy_header(fh, arg);
          fclose(fh);
        }
      }

      if (viafile != 0)
      {
        vf = fopen (viafile, "r");
        while ((fgets(hdrfile, 256, vf))!=0)
        {
           hdrfile[strlen(hdrfile)-1] = '\0';
           strcpy(stripped_file,headers);
           strcat(stripped_file,hdrfile);
           fh = fopen(stripped_file, "w");
           copy_header(fh, hdrfile);
           fclose(fh);
        }
        fclose(vf);
      }
    }

    if (nerrors == 0)
      fprintf(stderr, "StripHdrs: Finished\n");
    else
    {
      fclose(fh);
      fprintf(stderr,"\nStripHdrs: %s is junk because of errors\n\n",headers);
      exit(1);
    }
    return 0;
}
