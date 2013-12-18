/*------------------------------------------------------------------------
--                                                                      --
--     			H E L I O S   C O M M A N D S			--
--			-----------------------------			--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- kill.c								--
--                                                                      --
--	A kill command that works correctly in a Helios multi-user	--
--	protected environment, as well as in a single-user system	--
--                                                                      --
--	Author:  BLV 9/1/91						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/kill.c,v 1.11 1994/03/10 17:13:06 nickc Exp $*/

/*{{{  header files etc. */
#include <stdio.h>
#include <queue.h>
#include <syslib.h>
#include <stdlib.h>
#include <nonansi.h>
#include <posix.h>
#include <string.h>
#include <servlib.h>
#include <signal.h>
#include <fault.h>
#include <ctype.h>
#include <stdarg.h>
#include "rmlib.h"
#include "session.h"
#include "netutils.h"

#define eq ==
#define ne !=
/*}}}*/
/*{{{  statics and forward declarations */
typedef struct {
	Node		 Node;
	Object		*Parent;	/* Directory,  or object if name is null */
	char		 Name[NameMax];
} CandidateNode;
static	List		CandidatesList;

static	WORD 		WalkDir(Object *dir, WordFnPtr fn, char *name);
static	WORD 		match_name(Object *dir, char *direntry_name, char *name);
static	Object		*get_proper_access(Object *program);
static	void		usage(void);
static	void		show_signals(void);
static	int		find_program(char *name);
static	void		clean_up(void);
static	int		determine_signal(char *);
static	bool		send_signal(int);

#ifndef SingleProcessor
static	int		check_processor(RmProcessor, ...);
#endif
/*}}}*/
/*{{{  main() */

int main(int argc, char **argv)
{ int	signo = SIGTERM;
  int	i	= 1;
  int	rc	= EXIT_SUCCESS;
  bool	all	= FALSE;
  int	count;

  if (argc eq 1) usage();

  if (argc eq 2)
   if ((!strcmp(argv[1], "-l")) || (!strcmp(argv[1], "-L")))
    show_signals();

  for ( ; i < argc; i++)
   { 
     if (argv[i][0] eq '-')
      { if (!strcmp(argv[i], "-a"))
	 all = TRUE;
	else
 	 signo = determine_signal(&(argv[i][1]));
        continue;
      }

     if ((argv[i][0] ne '/') && (strchr(argv[i], '/') ne NULL))
      { fprintf(stderr, "kill: illegal task name %s\n", stderr);
	rc = EXIT_FAILURE;
 	continue;
      }
         
     count = find_program(argv[i]);
     if (count eq 0)
      { fprintf(stderr, "kill: cannot find program %s\n", argv[i]);
        rc = EXIT_FAILURE;
      }
     elif ((count > 1) && !all)
      { CandidateNode	*node1	= Head_(CandidateNode, CandidatesList);
	CandidateNode	*node2	= Next_(CandidateNode, node1);
	fprintf(stderr, "kill: ambigious name %s\n", argv[i]);
	fprintf(stderr, "    : it matches %s/%s and %s/%s\n",
		node1->Parent->Name, node1->Name, node2->Parent->Name, node2->Name);
	rc = EXIT_FAILURE;
      }
     else
      unless(send_signal(signo)) 
       rc = EXIT_FAILURE;
     clean_up();
   }
  return(rc);
}

static	void usage(void)
{ fputs("kill: usage, kill [-SIG] [-a] program\n", stderr);
  fputs("    :        kill -l\n", stderr);
  exit(EXIT_FAILURE);
}
/*}}}*/
/*{{{  determining the signal to be sent */

/**
*** A table of the known signals. This is used when examining the argument
*** specifying the signal number, and for the -l option.
**/
typedef struct signal_definition {
	char	*name;
	int	number;
} signal_definition;

static	signal_definition known_signals[] =
{	{ "ZERO",	 0	},
	{ "ABRT",	 1	},
	{ "FPE",	 2	},
	{ "ILL",	 3  	},
	{ "INT",	 4	},
	{ "SEGV",	 5	},
	{ "TERM",	 6	},
	{ "STAK",	 7	},
	{ "ALRM",	 8	},
	{ "HUP",	 9	},
	{ "PIPE",	10	},
	{ "QUIT",	11	},
	{ "TRAP",	12	},
	{ "USR1",	13	},
	{ "USR2",	14	},
	{ "CHLD",	15	},
	{ "URG",	16	},
	{ "CONT",	17	},
	{ "STOP",	18	},
	{ "TSTP",	19	},
	{ "TTIN",	20	},
	{ "TTOU",	21	},
	{ "WINCH",	22	},
	{ "SIB",	23	},
	{ "KILL",	31	},
	{ Null(char),	-1	}
};

/**
*** This routine shows the signals known to the kill command. To produce
*** a reasonable format it assumes 80 columns and four characters per
*** signal.
**/
static	void	show_signals(void)
{ int	i, j;

  for (i = 0, j = 0; known_signals[i].name ne Null(char); i++)
   { fputs(known_signals[i].name, stderr);
     fputc(' ', stderr);
     if (++j eq 16)
      { fputc('\n', stderr);
        j = 0;
      }
   }
  unless (j eq 0) fputc('\n', stderr);
  exit(EXIT_SUCCESS);
}

/**
*** Work out the signal from the given argument. This signal may be:
***     1) a number between 0 and 31
***	2) a string corresponding to one of the known signals
**/
static	int	determine_signal(char *str)
{ int	result = 0;
  int	i;
  
  if (isdigit(*str))
   { char	*temp = str;
     while (isdigit(*temp))
      { result = (10 * result) + (*temp - '0');
        temp++;
      }
     if (*temp eq '\0')
      { if ((result < 0) || (result > 31))
         { fprintf(stderr,
         "kill: illegal signal value %d, signals should be between 0 and 31.\n",
       		result);
       	   exit(EXIT_FAILURE);
       	 }
      	return(result);
      }
   }

	/* The string is not a simple number */
  for (i = 0; known_signals[i].name ne Null(char); i++)
   if (!mystrcmp(known_signals[i].name, str))
    return(known_signals[i].number);

  fprintf(stderr, "kill: unknown signal %s\n", str);
  exit(EXIT_FAILURE);
  return(SIGZERO);    
}
/*}}}*/
/*{{{  cleaning up between arguments */
static void clean_up(void)
{ CandidateNode	*node, *next;

  for (node = Head_(CandidateNode, CandidatesList);
	!EndOfList_(node);
	node = next)
   { next = Next_(CandidateNode, node);
     Remove(&(node->Node));
     Close(node->Parent);
     Free(node);
   }
}
/*}}}*/
/*{{{  find the programs */

/**
*** Finding a program involves checking lots of possibilities.
***
*** The steps used to determine which program to kill are as follows:
***  1) if the program name is absolute, just locate it. However,
***     in a protected multi-user environment that may not suffice...
***  2) if there is no TFM, just check the current processor
***  3) if there is a TFM and the CDL flag is set, check the tfm directory
***  4) if CDL is not set, search the domain for a suitable program.
***  5) give up
***
*** All candidates found are put into a list. At the end of the search the
*** number of candidates is counted and suitable action is taken.
**/

static	int find_program(char *name)
{ Object	*tfm;
  CandidateNode	*node;

  InitList(&CandidatesList);

  if (name[0] eq '/')
   { int	access;
     Object	*current_candidate;

     current_candidate = Locate(Null(Object), name);
     if (current_candidate eq Null(Object))
      { fprintf(stderr, "kill: failed to locate program %s\n", name);
        return(0);
      }
     access = current_candidate->Access.Access;
     if ((access & (AccMask_R + AccMask_W)) ne (AccMask_R + AccMask_W))
      current_candidate = get_proper_access(current_candidate);

     if (current_candidate eq Null(Object))
      { fprintf(stderr, "kill: insufficient access to kill %s\n", name);
        return(0);
      }
	/* Add just this program to the list of candidates */
     node = New(CandidateNode);
     if (node eq NULL)
      { fputs("kill: out of memory\n", stderr);
        exit(EXIT_FAILURE);
      }
     node->Parent	= current_candidate;
     node->Name[0]	= '\0';
#ifdef SYSDEB
     node->Node.Next = node->Node.Prev = &node->Node;
#endif
     AddTail(&CandidatesList, &(node->Node));
     return(1);
   }

#ifdef SingleProcessor
  tfm = Null(Object);
#else
  tfm = RmGetTfm();
  if (tfm eq Null(Object))
#endif
   { Object	*procman = Locate(Null(Object), "/tasks");
     int	count;
     
     if (procman eq Null(Object))
      { fputs("kill: failed to locate own processor manager.\n", stderr);
        return(0);
      }
     count = (int) WalkDir(procman, (WordFnPtr) &match_name, name);
     Close(procman);
     return(count);
   }

#ifndef SingleProcessor
  if (getenv("CDL") ne Null(char))
   { int count = (int) WalkDir(tfm, (WordFnPtr) &match_name, name);
     if (count > 0)
      return(count);
   }

  { RmNetwork	domain = RmGetDomain();
    RmNetwork	obtained;
    int		count;

    if (domain eq (RmNetwork) NULL)
     { fputs("kill: failed to locate own domain of processors.\n", stderr);
       exit(EXIT_FAILURE);
     }
    obtained = RmObtainNetwork(domain, FALSE, NULL);
    RmFreeNetwork(domain);
    if (obtained eq NULL)
     { fputs("kill: failed to access own domain of processors.\n", stderr);
       exit(EXIT_FAILURE);
     }
    count = RmApplyProcessors(obtained, &check_processor, name);
    RmReleaseNetwork(obtained);
    RmFreeNetwork(obtained);
    return(count);
  }  
#endif
}

/*}}}*/
/*{{{  match_name() */

/**
*** This routine is used to match the name passed as an argument and the
*** name of a directory entry. The algorithm used is as follows:
*** 1) if the name matches exactly, fine.
*** 2) if the name matches up to the . in the directory entry and is then
***    terminated, fine.
*** 3) otherwise this is not the right program.
***
*** For example, it is possible to use
***    kill -9 ls
*** or
***    kill -9 ls.27
***
*** If a match is found then it is added to the list of the existing matches.
**/

static WORD match_name(Object *dir, char *direntry_name, char *name)
{ char			*temp1 = direntry_name;
  char			*temp2 = name;
  CandidateNode		*candidate_entry;

  until ((*temp1 ne *temp2) || (*temp2 eq '\0'))
   { temp1++; temp2++; }
   
  if (*temp2 ne '\0') return(0);	/* failed before end of name */
  if ((*temp1 ne '\0') && (*temp1 ne '.')) return(0);

  candidate_entry	= New(CandidateNode);
  if (candidate_entry eq NULL)
   { fputs("kill: out of memory.\n", stderr);  
     exit(EXIT_FAILURE);
   }
  candidate_entry->Parent	= NewObject(dir->Name, &(dir->Access));
  if (candidate_entry->Parent eq NULL)
   { fputs("kill: out of memory.\n", stderr);
     exit(EXIT_FAILURE);
   }
  strcpy(candidate_entry->Name, direntry_name);
#ifdef SYSDEB
  candidate_entry->Node.Next = candidate_entry->Node.Prev = &candidate_entry->Node;
#endif
  AddTail(&CandidatesList, &(candidate_entry->Node));
  return(1);
}

/*}}}*/
/*{{{  search the domain */

/**
*** This is where the real fun starts. Somewhere inside the user's domain
*** there may be a program matching the argument. It is necessary to search
*** the domain processor by processor. These processors have been obtained so
*** there is no problem gaining access.
**/
#ifndef SingleProcessor

static int check_processor(RmProcessor processor, ...)
{ va_list	args;
  char		*name;
  Object	*real_processor;
  Object	*procman;
  int		count;

  va_start(args, processor);
  name = va_arg(args, char *);
  va_end(args);

	/* Check that this processor is running Helios  */
  if ((RmGetProcessorPurpose(processor) & RmP_Mask) ne RmP_Helios)
   return(0);

	/* Switch from RmLib world to Helios world	*/  
  real_processor = RmMapProcessorToObject(processor);
  if (real_processor eq Null(Object))
   { fprintf(stderr, "kill: failed to map processor %s\n",
   	RmGetProcessorId(processor));
     return(0);
   }

  procman = Locate(real_processor, "tasks");
  if (procman eq Null(Object))
   { fprintf(stderr, "kill: failed to locate %s/tasks\n", real_processor->Name);
     Close(real_processor);
     return(0);
   }
  Close(real_processor);

  count = (int) WalkDir(procman, &match_name, name);
  Close(procman);
  return(count);
}
#endif
/*}}}*/
/*{{{  get access to a processor */
/**
*** Given an absolute pathname as argument in a protected multi-user
*** environment, simply locating the program is not sufficient to
*** send a signal to it: the capability will be inadequate. Hence it
*** is necessary to do some more work to get the capability.
***
*** 1) If this session does not involve a TFM, tough! There is no way of
***    getting the right access.
*** 2) If the argument matches with the tfm name, i.e. the user is
***    trying to kill off a task or taskforce controlled by the TFM,
***    then life is fairly easy.
*** 3) Otherwise the argument should correspond to a program inside a
***    processor, i.e. it should be of the form /xx/tasks/yy. The work
***    involved is as follows:
***     a) extract the string xx, get the current domain, and look up
***        the processor.
***     b) locate the processor manager and check the access.
***     c) if insufficient, obtain the processor and try again.
***     d) locate the program itself.
**/

#ifdef SingleProcessor

static	Object *get_proper_access(Object *program)
{ return(program);
}

#else

static	Object *get_proper_access(Object *program)
{ Object	*tfm = RmGetTfm();
  int		len;
  Object	*result;
  char		buf[IOCDataMax];
  char		*ptr;
  RmNetwork	domain;
  RmProcessor	processor;
  RmProcessor	obtained_processor;
  Object	*real_processor;
          
  if (tfm eq Null(Object))
   { Close(program); return(Null(Object)); }

  len = strlen(tfm->Name);
  if (!strncmp(tfm->Name, program->Name, len))
   if (program->Name[len] eq '/')
    { result = Locate(tfm, &(program->Name[len + 1]));
      if (result eq Null(Object))
       fprintf(stderr, "kill: failed to re-locate taskforce %s/%s\n",
        		tfm->Name, &(program->Name[len + 1]));
      Close(program);
      return(result);
    }
        
  strcpy(buf, program->Name);
  ptr = strstr(buf, "/tasks/");
  if (ptr eq Null(char))
   { fprintf(stderr, "kill: %s does not appear to be a program.\n",
   		program->Name);
     Close(program);
     return(Null(Object));
   }
  *ptr++ = '\0';	/* terminate processor name, and point to tasks/xx.29 */

  Close(program);
  domain = RmGetDomain();
  if (domain eq (RmNetwork) NULL)
   { fputs("kill: failed to get current domain details.\n", stderr);
     return(Null(Object));
   }
  processor = RmLookupProcessor(domain, buf);
  if (processor eq (RmProcessor) NULL)
   { fprintf(stderr, "kill: processor %s is not in your own domain.\n",
   		buf);
     RmFreeNetwork(domain);
     return(Null(Object));
   }
	/* It is a safe bet that the processor is protected and has to	*/
	/* be obtained, or this routine would not have been called.	*/
  obtained_processor = RmObtainProcessor(processor);
  if (obtained_processor eq (RmProcessor) NULL)
   { fprintf(stderr, "kill: failed to obtain access to processor %s.\n",
   		buf);
     return(Null(Object));
   }

  RmFreeNetwork(domain);	/* No longer needed */

  real_processor = RmMapProcessorToObject(obtained_processor);
  if (real_processor eq Null(Object))
   { fprintf(stderr, "kill: failed to re-map obtained processor %s.\n",
   		buf);
     return(Null(Object));
   }
  RmReleaseProcessor(obtained_processor);
  RmFreeProcessor(obtained_processor);

	/* ptr still points to "tasks/xx.29" */
  result = Locate(real_processor, ptr);  
  if (result eq Null(Object))
   fprintf(stderr, "kill: failed to re-locate %s/%s\n",
   	real_processor->Name, ptr);

  Close(real_processor);
  return(result);
}  
#endif
/*}}}*/
/*{{{  WalkDir() */

/**
*** Standard WalkDir() routine.
**/
static WORD WalkDir(Object *dir, WordFnPtr fn, char *name)
{ WORD  	sum = 0;
  Stream  	*s;
  WORD		size, i;
  DirEntry	*entry, *cur;
   
  s = Open(dir, Null(char), O_ReadOnly);
  if (s eq Null(Stream))
   { fprintf(stderr, "kill : error, unable to open directory %s\n", dir->Name);
     return(0);
   }

  size = GetFileSize(s);

  if (size eq 0) return(0);
  entry = (DirEntry *) Malloc(size);
  if (entry == Null(DirEntry))
   { fputs("kill : out of memory\n", stderr);
     Close(s); 
     exit(EXIT_FAILURE); 
   }
     
  if (Read(s, (BYTE *) entry, size, -1) ne size)
   { fprintf(stderr, "kill : error reading directory %s\n", dir->Name);
     Close(s); Free(entry);
     return(0);
   }
  Close(s);
      
  cur = entry;
  for (i = 0; i < size; cur++, i += sizeof(DirEntry) )
   { if ( (!strcmp(cur->Name, ".")) || (!strcmp(cur->Name, "..")) )
      continue;
     sum += (*fn)(dir, cur->Name, name);
   }

  Free(entry);
  return(sum);
}
/*}}}*/
/*{{{  send a signal */

/**
*** Sending a signal to a program under Helios is relatively easy, albeit
*** rather different from the Unix way.
**/
static	bool	send_signal(int sig)
{ CandidateNode	*node;
  bool		 result = TRUE;

  for ( node = Head_(CandidateNode, CandidatesList);
	!EndOfList_(node);
	node = Next_(CandidateNode, node))
   { Stream	*s;
     WORD		rc;
     if (node->Name[0] eq '\0')
      s = Open(node->Parent, NULL, O_ReadWrite);
     else
      s = Open(node->Parent, node->Name, O_ReadWrite);
     if (s eq Null(Stream))
      { fprintf(stderr, "kill: failed to access program %s/%s, fault 0x%08lx\n",
   		node->Parent->Name, node->Name, Result2(node->Parent));
	result = FALSE;
       	continue;
      }
     rc = SendSignal(s, sig);
     Close(s);
     if (rc < Err_Null)
      { fprintf(stderr, "kill: failed to send signal to program %s/%s, fault 0x%08lx\n",
   		node->Parent, node->Name, rc);
	result = FALSE;
        continue;
      }
   }
  return(result);
}

/*}}}*/
