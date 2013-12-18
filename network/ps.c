/*------------------------------------------------------------------------
--                                                                      --
--     			H E L I O S   C O M M A N D S			--
--			-----------------------------			--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- ps.c									--
--                                                                      --
--	A new version of the ps command to cope with multi-user		--
--	networks.
--                                                                      --
--	Author:  BLV 6/9/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/ps.c,v 1.10 1993/08/11 10:41:35 bart Exp $*/

#include <stdio.h>
#include <stdlib.h>
#include <syslib.h>
#include <string.h>
#include <gsp.h>
#include <codes.h>
#include "session.h"
#include "rmlib.h"

#define eq ==
#define ne !=

static WORD display_real_processor(Object *processor, int level);
static WORD WalkDir2(Object *obj, WordFnPtr fn, WORD a);

#ifndef SingleProcessor
static WORD display_pseudo_processor(Object *processor, int level);
static WORD WalkDir(Object *obj, WordFnPtr fn, WORD a);

static 		Object	*NetworkRoot;
static		Object	*PseudoRoot;
#endif

#ifdef SingleProcessor

	/* In a single processor system, ps can only be used to examine	*/
	/* everything on the current processor. There is currently no	*/
	/* easy way of distinguishing between the different users'	*/
	/* programs.							*/
int main(int argc, char **argv)
{ static char	namebuf[IOCDataMax];
  Object	*processor;
  
  if (argc ne 1)
   fputs("ps: ignoring arguments\n", stderr);

  MachineName(namebuf);
  processor = Locate(Null(Object), namebuf);
  if (processor eq Null(Object))
   { fputs("ps: error, failed to locate own processor\n", stderr);
     return(EXIT_FAILURE);
   }
   
  display_real_processor(processor, 0);
  argv = argv;
  return(EXIT_SUCCESS);
}

#else	/* multi-processor */

	/* In a multi-processor system things can get more complicated.	*/
	/* ps by itself lists the processors in the user's domain, or	*/
	/* in the current processor in the absence of a tfm.		*/
	/* Alternatively ps can be used to examine a specific processor	*/
	/* or another user's domain.					*/
int main(int argc, char **argv)
{ static char	namebuf[IOCDataMax];
  char	*temp;

	/* Step one, figure out whether or not this processor is part	*/
	/* of a network. MachineName() should be /root or /Net/Root	*/
  MachineName(namebuf);
  for (temp = &(namebuf[1]); (*temp ne '/') && (*temp ne '\0'); temp++);
  if (*temp eq '\0')
   { Object	*processor = Locate(Null(Object), namebuf);

     if (argc ne 1)
      fputs("ps: single-processor system, ignoring arguments.\n", stderr);

     if (processor eq Null(Object))
      { fputs("ps: error, failed to locate own processor\n", stderr);
        return(EXIT_FAILURE);
      }
     display_real_processor(processor, 0); 
     return(EXIT_SUCCESS);
   }

	/* This processor is in a network, so make a note of the network */
	/* root name, e.g. /Net						 */
  *temp = '\0';
  NetworkRoot = Locate(Null(Object), namebuf);
  if (NetworkRoot eq Null(Object))
   { fprintf(stderr, "ps : failed to locate network root %s\n", namebuf);
     return(EXIT_FAILURE);
   }

  if (argc eq 1)	/* ps by itself refers to the user's domain	*/
  			/* extract the TFM and hence the /domain	*/  
  			/* directory, and list that.			*/
   { Object	*tfm = RmGetTfm();
     Object	*domain;
     
     if (tfm eq Null(Object))
      { Object	*processor;
	*temp = '/';
        processor = Locate(Null(Object), namebuf);
        if (processor eq Null(Object))
         { fputs("ps: error, failed to locate own processor\n", stderr);
           return(EXIT_FAILURE);
         }
        display_real_processor(processor, 0);
        return(EXIT_SUCCESS);
      }
      
     domain = Locate(tfm, "../domain");
     if (domain eq Null(Object))
      { fprintf(stderr, "ps: failed to locate own domain, fault 0x%08lx\n",
      		Result2(tfm));
      	return(EXIT_FAILURE);
      }
     Close(tfm);
     PseudoRoot = domain;
     (void) WalkDir(domain, &display_pseudo_processor, 0);
     return(EXIT_SUCCESS);
   }

  if ((argc eq 2) && !strcmp(argv[1], "all"))
   { Object	*ns = Locate(Null(Object), "/ns");
     if (ns eq Null(Object))
      { fputs("ps: failed to locate network server.\n", stderr);
        return(EXIT_FAILURE);
      }
     printf("Network %s\n", NetworkRoot->Name);
     PseudoRoot = ns;
     (void) WalkDir(ns, &display_pseudo_processor, 4);
     return(EXIT_SUCCESS);
   }

  for (argv++, argc--; argc > 0; argc--, argv++)
   { Object	*session;
     Object	*domain;
     char	namebuf[IOCDataMax];
      
     if (**argv eq '/')
      strcpy(namebuf, *argv);
     else
      { namebuf[0] = '/'; strcpy(&(namebuf[1]), *argv); }

     session = Locate(Null(Object), namebuf);
     if (session eq Null(Object))
      { fprintf(stderr, "ps: warning, failed to locate %s\n",
      		namebuf);
      	continue;
      }
     domain = Locate(session, "domain");
     if (domain eq Null(Object))
      { Object	*procman = Locate(session ,"tasks");
        if (procman eq Null(Object))
         { fprintf(stderr, "ps: warning, failed to locate domain %s/domain\n",
      	 	namebuf);
      	   Close(session);
      	   continue;
      	 }
      	Close(procman);
      	display_real_processor(session, 0);
      	Close(session);
      	continue;
      }
     Close(session);
     printf("Session %s :\n", *argv);
     PseudoRoot = domain;
     (void) WalkDir(domain, &display_pseudo_processor, 4);
     Close(domain);
   }   
   
  return(0);
}

#endif	/* single/multi processor */

/**
*** This code deals with displaying the tasks within a processor.
*** It involves reading the /tasks directory of that processor,
*** i.e. a simple WalkDir with a display function. The only problem
*** is keeping the display tidy, which can be done using a static.
**/

static int task_count;
static WORD display_task(char *task, WORD spaces)
{ task_count++;
  if (task_count eq 4)
   { printf("\n    ");
     for ( ; spaces > 0; spaces--) putchar(' ');
     task_count = 1; 
   }
  
  printf("%-20s", task);
  return(0);
}

static WORD display_real_processor(Object *processor, int spaces)
{ int		i;
  Object	*procman;

  for (i = 0; i < spaces; i++) putchar(' ');
  printf("Processor %s", objname(processor->Name));
  procman = Locate(processor, "tasks");
  if (procman eq Null(Object)) goto done;
  task_count = 3;
  (void) WalkDir2(procman, &display_task, spaces);
  Close(procman);
  
done:
  putchar('\n');
  return(0);
}

/**
*** This routine copes with pseudo-processors, e.g. /bart/domain/01, which
*** have to be mapped on to the real processors /Net/01 somehow.
**/

#ifndef SingleProcessor

static WORD display_pseudo_processor(Object *pseudo, int spaces)
{ int		i;
  Object	*real_processor;
  ObjInfo	info;
  char		*temp;

  if ((pseudo->Type & Type_Flags) eq Type_Directory)
   { for (i = 0; i < spaces; i++) putchar(' ');
     printf("Subnet %s\n", objname(pseudo->Name));
     return(WalkDir(pseudo, &display_pseudo_processor, (word)spaces + 4));
   }

  if ((i = (int)ObjectInfo(pseudo, Null(char), (BYTE *) &info)) < Err_Null)
   { fprintf(stderr, "ps : failed to examine %s, fault 0x%8x\n",
   		 pseudo->Name, i);
     return(0);
   }

  if ((info.Size & RmS_Running) eq 0)
   return(0);
   
  temp = pseudo->Name + strlen(PseudoRoot->Name) + 1;
  real_processor = Locate(NetworkRoot, temp);
  if (real_processor eq Null(Object))
   fprintf(stderr, "ps : failed to locate %s/%s\n", NetworkRoot->Name, temp);
  else
   { 
     display_real_processor(real_processor, spaces);
     Close(real_processor);
   }
  return(0);
}
#endif

/**
*** Two versions of WalkDir are useful. They differ in the argument passed
*** to the walking function: one gives an Object pointer, the other a
*** pathname.
**/
#ifndef SingleProcessor

static WORD WalkDir(Object *dir, WordFnPtr fn, WORD a)
{ WORD  	sum = 0;
  Stream  	*s;
  WORD		size, i;
  DirEntry	*entry, *cur;
  Object	*item;
  WORD		got;
  
  if ((dir->Type & Type_Flags) eq Type_Stream)
   return(0);
   
  s = Open(dir, Null(char), O_ReadOnly);
  if (s eq Null(Stream))
   { fprintf(stderr, "ps : error, unable to open directory %s\n", dir->Name);
     return(0);
   }

  size = GetFileSize(s);

  if (size eq 0) return(0);
  entry = (DirEntry *) Malloc(size);
  if (entry == Null(DirEntry))
   { fputs("ps : out of memory\n", stderr);
     Close(s); 
     return(0); 
   }
     
  if ((got = Read(s, (BYTE *) entry, size, -1)) ne size)
   {
     if (got > 0 && GetFileSize( s ) == got)
       {
	 /*
	  * XXX - NC - 5/4/93
	  *
	  * The /tasks directory has shrunk between the GetFileSize()
	  * at the start of this function and the Read() above.  This
	  * can happen, albeit very rarly, so do not complain.
	  */

	 size = got;
       }
     else
       {
	 fprintf(stderr, "ps : error reading directory %s (Result2 = %lx)\n",
	     dir->Name, Result2( s ));
	 
	 Close(s);
	 Free(entry);
	 return(0);
       }
   }
  Close(s);
      
  cur = entry;
  for (i = 0; i < size; cur++, i += sizeof(DirEntry) )
   { if ( (!strcmp(cur->Name, ".")) || (!strcmp(cur->Name, "..")) )
      continue;

     item = Locate(dir, cur->Name);
     if (item eq Null(Object))
      fprintf(stderr, "ps : error, failed to locate %s/%s\n", dir->Name,
      		cur->Name);
     else
      sum += (*fn)(item, a);
   }

  Free(entry);
  return(sum);
}
#endif

static WORD WalkDir2(Object *dir, WordFnPtr fn, WORD a)
{ WORD  	sum = 0;
  Stream  	*s;
  WORD		size, i;
  DirEntry	*entry, *cur;
  WORD		got;
  
  if ((dir->Type & Type_Flags) eq Type_Stream)
   return(0);
   
  s = Open(dir, Null(char), O_ReadOnly);
  if (s eq Null(Stream))
   { fprintf(stderr, "ps : error, unable to open directory %s\n", dir->Name);
     return(0);
   }

  size = GetFileSize(s);

  if (size eq 0) return(0);
  entry = (DirEntry *) Malloc(size);
  if (entry == Null(DirEntry))
   { fputs("ps : out of memory\n", stderr);
     Close(s); 
     return(0); 
   }
     
  if ((got = Read(s, (BYTE *) entry, size, -1)) ne size)
   {
     if (got > 0 && got == GetFileSize( s ))
       {
	 /*
	  * XXX - NC - 5/4/93
	  *
	  * The /tasks directory has shrunk between the GetFileSize()
	  * at the start of this function and the Read() above.  This
	  * can happen, albeit very rarly, so do not complain.
	  */

	 size = got;
       }
     else
       {
	 fprintf(stderr, "ps : error reading directory %s (Result2 = %lx)\n",
		 dir->Name, Result2( s ) );
	 Close(s);
	 Free(entry);
	 return(0);
       }
   }
  
  Close(s);
      
  cur = entry;
  for (i = 0; i < size; cur++, i += sizeof(DirEntry) )
   { if ( (!strcmp(cur->Name, ".")) || (!strcmp(cur->Name, "..")) )
      continue;
     sum += (*fn)(cur->Name, a);
   }

  Free(entry);
  return(sum);
}
