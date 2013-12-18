/**
*
* Title:  CDLDIS - Disassemble a CDL object file.
*
* Author: Andy England
*
* Date:   19th May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/
static char *rcsid = "$Header: /hsrc/cmds/cdl/RCS/cdldis.c,v 1.2 1993/04/14 16:38:20 nickc Exp $";

#include "cdl.h"
#include <stdio.h>
#include <nonansi.h>
#include "cdlobj.h"

void disasm();
void readfile();
void readheader();
void readcomponents();
void readstreams();
void putarguments();
void putstreams();
char *getstring();
void getbytes();

FILE *inputfile = stdin;
FILE *outputfile = stdout;
char *filename;
char *cmdname;
CDL_HEADER cdl_header;
int filesize;
int fileindex;
char *filetext;
CDL_ISTREAM *streams;
CDL_DEV_ATTR *attribs;
char *strings;
char *stdnames[4] =
{
  "stdin",
  "stdout",
  "stderr",
  "stddbg"
};

int main(argc, argv)
int argc;
char *argv[];
{
  cmdname = *argv++; argc--;
  if (argc > 0)
  {
    if (strequ(*argv, "-o"))
    {
      argv++; argc--;
      if (argc == 0)
      {
        fprintf(stderr, "Missing filename for '-o' option\n");
        exit(1);
      }
      if ((outputfile = fopen(*argv, "w")) == NULL)
      {
        fprintf(stderr, "Unable to open '%s' for output\n", *argv);
        exit(1);
      }
      argv++; argc--;
    }
  }
  if (argc > 0)
  {
    while (argc--)
    {
      filename = *argv++;
      if ((inputfile = fopen(filename, "rb")) == NULL)
      {
        fprintf(stderr, "Unable to open '%s' for input\n", filename);
        exit(1);
      }
      disasm();
    }
  }
  else
  {
    inputfile = stdin;
    disasm();
  }
  return 0;
}

void disasm()
{
  readfile();
  readheader();
  readcomponents();
  readstreams();
}

void readheader()
{
  getbytes(&cdl_header, sizeof(CDL_HEADER));

  unless (cdl_header.type == TYPE_2_OBJ OR cdl_header.type == TYPE_3_OBJ)
  {
    fprintf(stderr, "'%s' is not a CDL object file\n", filename);
    exit(1);
  }
  fprintf(outputfile, "Disassembly of CDL object file '%s':\n\n", filename);

  streams = (CDL_ISTREAM *)(filetext + sizeof(CDL_HEADER) +
            (cdl_header.nocomponents * sizeof(CDL_COMPONENT)) +
            (cdl_header.nocstreams * sizeof(CDL_CSTREAM)));
  attribs = (CDL_DEV_ATTR *)(((char *)streams) +
            (cdl_header.noistreams * sizeof(CDL_ISTREAM)));
  strings = ((char *)attribs) + (cdl_header.noattribs * sizeof(CDL_DEV_ATTR)) +
            sizeof(int);
  fprintf(outputfile, "Current Directory:\t%s\n", getstring(cdl_header.currentdir.index));
  fprintf(outputfile, "Task Force Name:\t%s\n", getstring(cdl_header.tf_name.index));
  fprintf(outputfile, "Number of Components:\t%d\n", cdl_header.nocomponents);
}

void _putattribs(count, index)
int count, index;
{
  CDL_DEV_ATTR *cdl_dev_attr = attribs + index;

  while (count--)
  {
    fprintf(outputfile, "%s", getstring(cdl_dev_attr->attribute.index));
    if (cdl_dev_attr->count == 1) fprintf(outputfile, " ");
    else fprintf(outputfile, "(%d) ", cdl_dev_attr->count);
    cdl_dev_attr++;
  }
  fprintf(outputfile, "\n");
}

void readcomponents()
{
  int i;

  for (i = 0; i < cdl_header.nocomponents; i++)
  {
    CDL_COMPONENT cdl_component;

    getbytes(&cdl_component, sizeof(CDL_COMPONENT));
    fprintf(outputfile, "\n");
    fprintf(outputfile, "Name:\t%s\n", getstring(cdl_component.name.index));
    fprintf(outputfile, "Flags:\t%08x\n", cdl_component.flags);
    fprintf(outputfile, "Type:\t%d\n", cdl_component.p_type);
    fprintf(outputfile, "Attribs:\t");
    _putattribs(cdl_component.noattribs, cdl_component.p_attrib.index);
    fprintf(outputfile, "Memory:\t%d\n", cdl_component.memory);
    fprintf(outputfile, "Life:\t%d\n", cdl_component.longevity);
    fprintf(outputfile, "Time:\t%d\n", cdl_component.time);
    fprintf(outputfile, "Priority:\t%d\n", cdl_component.priority);
    fprintf(outputfile, "argc:\t%d\n", cdl_component.nargs);
    fprintf(outputfile, "argv:\t");
    putarguments(cdl_component.nargs, cdl_component.args.index);
    fprintf(outputfile, "Streams:\n\n");
    putstreams(cdl_component.noistreams, cdl_component.istreams.index);
  }
}

void readstreams()
{
  int i;

  fprintf(outputfile, "\nCommon Streams:\n");
  for (i = 0; i < cdl_header.nocstreams; i++)
  {
    CDL_CSTREAM cdl_cstream;

    getbytes(&cdl_cstream, sizeof(CDL_CSTREAM));
    fprintf(outputfile, "\n");
    fprintf(outputfile, "Index:\t%d\n", i);
    fprintf(outputfile, "Name:\t%s\n", getstring(cdl_cstream.name.index));
    fprintf(outputfile, "Flags:\t%08x\n", cdl_cstream.flags);
    fprintf(outputfile, "Count:\t%d\n", cdl_cstream.count);
  }
}

void putarguments(count, index)
int count, index;
{
  char *string = strings + index;
  int i;

  for (i = 0; i < count; i++)
  {
    fprintf(outputfile, "%s ", string);
    string += strlen(string) + 1;
  }
  fprintf(outputfile, "\n");
}


void putstreams(count, index)
int count, index;
{
  CDL_ISTREAM *cdl_istream = streams + index;

  while (count--)
  {
    if (cdl_istream->index == -1)
      fprintf(outputfile, "Standard stream:\t%s\n", stdnames[cdl_istream->standard]);
    else fprintf(outputfile, "Stream index:\t%d\n", cdl_istream->index);
    fprintf(outputfile, "Mode:\t%08x\n\n", cdl_istream->mode);
    cdl_istream++;
  }
  fprintf(outputfile, "\n");
}

char *getstring(index)
int index;
{
  return strings + index;
}

void getbytes(buffer, size)
char *buffer;
int size;
{
  if (fileindex + size > filesize) 
  {
    fprintf(stderr, "Unexpected end of file\n");
    exit(1);
  }
  while (size--) *buffer++ = filetext[fileindex++];
}

void readfile()
{
  filesize = getsize(inputfile);
  filetext = (char *)malloc(filesize);
  fread(filetext, filesize, 1, inputfile);
  fileindex = 0;
}

int getsize(file)
FILE *file;
{
  return GetFileSize(Heliosno(file));
}

