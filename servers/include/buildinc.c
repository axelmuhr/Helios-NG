/*------------------------------------------------------------------------
--                                                                      --
--			H E L I O S   S E R V E R S			--
--			---------------------------			--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--  buildinc.c								--
--                                                                      --
--	Program to rebuild the include disk when required.		--
--                                                                      --
--	Author:  BLV 21.3.91						--
--                                                                      --
------------------------------------------------------------------------*/

/* $Header: /hsrc/servers/include/RCS/buildinc.c,v 1.3 1992/09/04 10:49:47 nickc Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nonansi.h>
#include <syslib.h>
#include <queue.h>
#include <gsp.h>
#include <servlib.h>
#include "include.h"

static	WORD WalkDir(char *name, WordFnPtr fn);
static	WORD add_entry(char *dir_name, DirEntry *dir_entry);
static	void copy_file(FileEntry *entry, char *buffer, int size);
static 	WORD write_entry(FileEntry *entry, FILE *output);

static	List IncludeList;

/**
*** Build an include disk.
*** 1) walk down all the entries in /helios/include and add them to a list
*** 2) open the output file
*** 3) write the number of entries in the list
*** 4) walk down the list and add the entries to the file
**/

int main(void)
{ WORD	number_entries;
  FILE	*output;
  
  InitList(&IncludeList);
  number_entries = WalkDir("/helios/include", &add_entry);

  output = fopen(IncludeDisk, "wb");
  if (output == Null(FILE))
   { fprintf(stderr, "buildinc : failed to open output file %s\n", IncludeDisk);
     exit(EXIT_FAILURE);
   }

  if (fwrite((void *) &number_entries, sizeof(WORD), 1, output) < 1)
   { fprintf(stderr, "buildinc : failed to write number of entries to %s\n",
   	IncludeDisk);
     exit(EXIT_FAILURE);
   }

  if (SearchList(&IncludeList, &write_entry, output) != 0)
   { fprintf(stderr, "buildinc : failed to write all data to %s\n",
   		IncludeDisk);
     exit(EXIT_FAILURE);
   }

  fclose(output);
  return(EXIT_SUCCESS);
}

/**
*** Write a single entry to the output file
**/
static WORD write_entry(FileEntry *entry, FILE *f)
{ int	size = sizeof(FileEntry) + (int)entry->ObjNode.Size;
  size	= (size + 3) & ~3;
  if (fwrite((void *) entry, 1, size, f) < 1)
   return(1);
  else
   return(0);	
}

/**
*** This routine is called from WalkDir(). 
*** 1) check that this direntry is a file rather than a subdirectory
*** 2) open the file and see how big it is. C library I/O is used to
***    avoid carriage-return/linefeed problems.
*** 3) allocate a buffer for the file and another to hold the file entry.
*** 4) read in all of the file
*** 5) copy the data to the file entry, stripping out comments
*** 6) add the new entry to the list.
**/
static WORD add_entry(char *dir_name, DirEntry *dir_entry)
{ char		name[IOCDataMax];
  FILE		*file;
  char		*buffer;
  FileEntry	*file_entry;
  int		size;

	/* For now ignore sub-directories */  
  if (dir_entry->Type != Type_File) return(0);

  strcpy(name, dir_name);
  strcat(name, "/");
  strcat(name, dir_entry->Name);
  file = fopen(name, "r");
  if (file == Null(FILE))
   { fprintf(stderr, "buildinc : failed to open %s\n", name);
     exit(EXIT_FAILURE);
   }
   
  size = (int)GetFileSize(Heliosno(file));
  if (size < 0)
   { fprintf(stderr, "buildinc : fault 0x%08x determining size of %s\n",
   		size, name);    
     exit(EXIT_FAILURE);
   }
   
  buffer = (char *) Malloc(size);
  if (buffer == Null(char))  
   { fprintf(stderr, "buildinc : not enough memory for %s\n", name);
     exit(EXIT_FAILURE);
   }
  file_entry = (FileEntry *) Malloc(sizeof(FileEntry) + (word)size);
  if (file_entry == Null(FileEntry))
   { fprintf(stderr, "buildinc : not enough memory for %s\n", name);
     exit(EXIT_FAILURE);
   }

  InitNode(&(file_entry->ObjNode), dir_entry->Name, Type_File, 0, DefFileMatrix);
  if ((size = fread((void *) buffer, 1, size,  file)) < 0)
   { fprintf(stderr, "buildinc : error reading file %s\n", name);
     exit(EXIT_FAILURE);
   }

  fclose(file);
  copy_file(file_entry, buffer, size);
  Free(buffer);
  AddTail(&IncludeList, &(file_entry->ObjNode.Node));
  return(1);
}

/**
*** Copy a single header file currently held in memory to another buffer,
*** stripping out comments.
**/
static void copy_file(FileEntry *entry, char *buffer, int size)
{ int	dest_index	= 0;
  int	source_index	= 0;
  char	*dest_buffer	= (char *) &(entry->Data[0]);

  for ( ; source_index < size; )
   { if ((buffer[source_index] != '/') || (buffer[source_index+1] != '*'))
      { dest_buffer[dest_index++] = buffer[source_index++];
        continue;
      }
     for (source_index += 2; source_index < size; source_index++)
      if ((buffer[source_index] == '*') && (buffer[source_index+1] == '/'))
       { source_index += 2; break; }
   }
  entry->ObjNode.Size = dest_index;
}

/**
*** Usual WalkDir() routine.
**/
static WORD WalkDir(char *name, WordFnPtr fn)
{ Object *o = Locate(CurrentDir, name);
  WORD   sum = 0;
  int	 i;
  
  if (o == Null(Object))
   { fprintf(stderr, "Error : unknown directory %s\n", name);
     return(0);
   }
   
  if ((o->Type & Type_Flags) == Type_Stream)
   return(0);
   
  { Stream *s = Open(o, Null(char), O_ReadOnly);
    WORD size;
    DirEntry *entry, *cur;
    
    if (s == Null(Stream))
     { fprintf(stderr, "Error : unable to open directory %s\n", name);
       Close(o);
       return(0);
     }

    size = GetFileSize(s);
    if (size == 0) return(0);
    entry = (DirEntry *) Malloc(size);
    if (entry == Null(DirEntry))
     { Close(s); Close(o); return(0); }
     
    if (Read(s, (BYTE *) entry, size, -1) != size)
     { fprintf(stderr, "Error reading directory %s\n", name);
       Close(s); Close(o); Free(entry);
       return(0);
     }
    
    cur = entry;
    for (i = 0; i < size; cur++, i += sizeof(DirEntry) )
     { if ( (!strcmp(cur->Name, ".")) || (!strcmp(cur->Name, "..")) )
        continue;
       sum += (*fn)(s->Name, cur);
     }

    Free(entry);
    Close(s);
  }  
    
  Close(o);
  return(sum);
}

