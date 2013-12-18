/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils : numon.c							--
--		Display statistics about a supplied network		--
--									--
--	Author:  BLV 12/4/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /pds/nickc/RTNucleus/network/RCS/numon.c,v 1.7 1993/12/20 12:53:53 nickc Exp $*/

/*{{{  header files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <nonansi.h>
#include <helios.h>
#include <syslib.h>
#include <gsp.h>
#include <codes.h>
#include <attrib.h>
#include <root.h>
#include <posix.h>
#include <sys/wait.h>
#include "rmlib.h"
#include "netutils.h"

#ifndef eq
#define eq ==
#define ne !=
#endif
/*}}}*/
/*{{{  structures */
/**
*** This structure is associated with every processor in the supplied
*** network. It contains information such as whether or not the current
*** processor has been tagged, maximum load and memory stats,
*** and an Object for the processor manager.
**/
typedef struct ProcessorInfo {
	bool	 Use;
	bool	 Tagged;
	int	 MaxLoad;
	int	 MaxMem;
	int	 CurLoad;
	int	 CurMem;
	Object	*Procman;
} ProcessorInfo;
/*}}}*/
/*{{{  statics */
/**
*** Static variables
**/
	/* To avoid unnecessary aggravation when coping with sub-nets	*/
	/* I store all the processors in a suitable array.		*/
static	RmProcessor	*procvec	= (RmProcessor) NULL;

	/* This is the first processor being displayed.			*/
static	int	first_processor		= 0;

	/* This is the last processor being displayed.			*/
static	int	last_processor		= 0;

	/* The "current" processor, this one is high-lighted.		*/
static	int	current_processor	= 0;

	/* Keep track of the total number of processors.		*/
static	int	number_processors	= 0;

	/* The maximum length of a processor name			*/
static	int		max_proc_name		= -1;

	/* Screen sizes.						*/
static	int		screen_width		= -1;
static	int		screen_height		= -1;

	/* Output formatting						*/
static	int		field_width;
static	int		number_to_display;

	/* Are we displaying tagged processors only ?			*/
static	bool		display_tagged	= FALSE;

	/* The delay between monitoring redraws.			*/
static	int		monitor_delay	= 2;	/* in seconds */

	/* Should the whole screen be redrawn ?				*/
static	bool		total_redraw 	= TRUE;
/*}}}*/
/*{{{  screen initialisation */
/**
*** This routine is responsible for setting the screen to raw input mode,
*** for the menu handling.
**/

static void	initialise_screen(void)
{ Attributes	attr;

  unless (isatty(0) && isatty(1))
   { fputs("MonitorNetwork: not running interactively.\n", stderr);
     exit(EXIT_FAILURE);
   }

  if (GetAttributes(fdstream(0), &attr) < Err_Null)
   { fputs("MonitorNetwork: failed to get keyboard details.\n", stderr);
     exit(EXIT_FAILURE);
   }

  AddAttribute(&attr, ConsoleRawInput);
  RemoveAttribute(&attr, ConsoleEcho);
  RemoveAttribute(&attr, ConsoleRawOutput);
  RemoveAttribute(&attr, ConsoleBreakInterrupt);
  if (SetAttributes(fdstream(0), &attr) < Err_Null)
   { fputs("MonitorNetwork: failed to initialise keyboard.\n", stderr);
     exit(EXIT_FAILURE);
   }
}
/*}}}*/
/*{{{  screen manipulation */
static void move_to(int y, int x)
{ printf("\033[%d;%dH", y, x);
}

static void clear_screen()
{ putchar('\f');
}

static void waitfor_user()
{ BYTE	buf[4];

  move_to(screen_height, 1);
  fputs("Press any key to continue.", stdout);
  fflush(stdout);
  while (Read(fdstream(0), buf, 1, -1) < 1);
}
/*}}}*/
/*{{{  screen sizes */
/**
*** This routine is responsible for determining the current screen size.
*** It is invoked every time around the main loop. If the screen size
*** has not changed then this is a no-op. Otherwise the screen is
*** cleared and various formatting parameters are re-calculated.
**/
static	void size_screen(void)
{ Attributes	attr;

  if (GetAttributes(fdstream(1), &attr) < Err_Null)
   { fputs("MonitorNetwork: failed to determine screen size.\n", stderr);
     exit(EXIT_FAILURE);
   }
  if ((attr.Time eq screen_width) && (attr.Min eq screen_height))
   return;

  screen_width	= attr.Time;
  screen_height	= attr.Min;

  if ((screen_height < 10) || (screen_width < 60))
   { fputs("MonitorNetwork: screen too small.\n", stderr);
     exit(EXIT_FAILURE);
   }
	/* Allow one line at the top for the banner, a blank line between */
	/* the processors and the menu, two lines for the menu, and	  */
	/* one line at the bottom for error messages.			  */
  number_to_display = screen_height - 5;

	/* The field width is rather more complicated...		  */
  field_width = (screen_width - (max_proc_name + 19)) / 2;
  if (field_width < 10)
   { fputs("MonitorNetwork: screen too small.\n", stderr);
     exit(EXIT_FAILURE);
   }
  clear_screen();
}
/*}}}*/
/*{{{  network initialisation */
/**
*** This routine is applied to every processor in the network. It
*** performs the following:
***  1) a ProcessorInfo structure is allocated
***  2) if the processor cannot be monitored, e.g. an I/O processor or
***     a native processor, the appropriate flag is set to null.
***  3) the processor is set to un-tagged, and the maximum cpu and memory
***     loads to date are set to 0.
***  4) the Processor Manager for that processor is located.
**/
static int	initialise_processor(RmProcessor processor, ...)
{ ProcessorInfo	*info		= New(ProcessorInfo);
  Object	*real_proc	= Null(Object);
  int		*index;
  va_list	 args;

  va_start(args, processor);
  index = va_arg(args, int *);
  va_end(args);
  procvec[*index] = processor;
  *index += 1;

  if (info eq NULL)
   { fputs("MonitorNetwork: out of memory.\n", stderr);
     exit(EXIT_FAILURE);
   }
  if ((RmGetProcessorPurpose(processor) & RmP_Mask) ne RmP_Helios)
   { info->Use = FALSE; goto done; }

  info->Use	= TRUE;
  info->Tagged	= FALSE;
  info->MaxLoad	= 0;
  info->MaxMem	= 0;
  info->CurLoad	= -1;
  info->MaxLoad = -1;

  real_proc	= RmMapProcessorToObject(processor);
  if (real_proc eq Null(Object))
   { info->Use	= FALSE; goto done; }

  info->Procman	= Locate(real_proc, "tasks");
  if (info->Procman eq Null(Object))
   info->Use = FALSE;

  if ((int) strlen(RmGetProcessorId(processor)) > max_proc_name)
   max_proc_name = strlen(RmGetProcessorId(processor));

done:
  if (real_proc ne Null(Object)) Close(real_proc);
  RmSetProcessorPrivate(processor, (int) info);
  return((info->Use) ? 1 : 0);
}
/*}}}*/
/*{{{  get processor information */
/**
*** This routine is responsible for getting the current performance
*** information about a particular processor.
**/
static	ProcStats *get_info(Object *procman, BYTE *data)
{ 
  if (ServerInfo(procman, data) >= Err_Null)
   return((ProcStats *) data);
  else
   return(NULL);
}
/*}}}*/
/*{{{  display screen */

/**
*** This code is responsible for displaying the main picture.
**/
/*{{{  display banner */
/**
*** The banner should be of the form :
***   Proc    % CPU Load            % Memory Load   Time stamp
*** suitably adjusted to cope with the widths of the various fields...
**/
static void display_banner(void)
{ time_t current_time	= time(NULL);
  char *buf		= ctime(&current_time);

  if (total_redraw)
   { move_to(1,1);
     printf("%*s    %% %-*s          %% %-*s",
		max_proc_name, "Proc",
		field_width - 1, "CPU Load",
		field_width - 1, "Memory");
   }
  move_to(1, screen_width - 7);
  printf("%.8s\n", &(buf[11]));
}
/*}}}*/
/*{{{  is a processor visible */
/**
*** This code determines whether or not a particular processor is
*** currently displayable, by checking the use and tagged fields.
**/
static bool	is_processor_visible(int index)
{ ProcessorInfo	*info	= (ProcessorInfo *) RmGetProcessorPrivate(procvec[index]);
  unless(info->Use) return(FALSE);
  if (display_tagged && !(info->Tagged)) return(FALSE);
  return(TRUE);
}  
/*}}}*/
/*{{{  display a processor */

/**
*** This routine is responsible for displaying the information about
*** a particular processor, or refusing to do that if the processor does
*** not match the current requirements.
**/
static	bool	display_processor(RmProcessor processor)
{ ProcessorInfo	*info	= (ProcessorInfo *) RmGetProcessorPrivate(processor);
  BYTE		 data[IOCDataMax];
  ProcStats	*proc_stats;
  int		 i;
  int		 load, load_percentage;
  int		 memsize, mem, mem_percentage;

  proc_stats = get_info(info->Procman, data);
  if (proc_stats eq NULL) 
   { printf("\033[K\n");
     return(FALSE);
   }

	/* Do the sums. load is some strange number between 0 and 2050. */
	/* The memory size provided by the nucleus does not include	*/
	/* the nucleus code nor, I believe, some kernel static data.	*/
	/* I assume that all processor memory sizes are multiples of a	*/
	/* megabyte...							*/
	/* The nucleus provides the amount of free memory. Again this	*/
	/* has to be scaled according to the field width.		*/  
  load		 = (int) proc_stats->Load;
  if (load > 2050)
   load 	 = 2050;		/* allow for overflow		*/
  load_percentage= (100 * load) / 2050;
  load		 = (load * field_width) / 2050;

  memsize 	 = (int) proc_stats->MemMax;
  memsize 	 = (memsize + ((1024 * 1024) - 1)) & ~((1024 * 1024) - 1);
  mem	 	 = (int)(memsize - proc_stats->MemFree);
  mem_percentage = (100 * mem) / memsize;
  mem		 = (mem * field_width) / memsize;

	/* If load and memory are unchanged, skip the redraw */
  if ((load eq info->CurLoad) && (mem eq info->CurMem))
   { putchar('\n'); return(TRUE); }

  info->CurLoad = load;
  info->CurMem	= mem;

  printf("%s%*s%s%c:",
	(processor eq procvec[current_processor]) ? "\033[7m" : "",
	max_proc_name, RmGetProcessorId(processor),
	(processor eq procvec[current_processor]) ? "\033[0m" : "",
	(info->Tagged) ? '*' : ' ');

  printf("%3d:", load_percentage);
  if (load > info->MaxLoad) info->MaxLoad = load;
  for (i = 0; i < load; i++) putchar('=');
  for (i = load; i < info->MaxLoad; i++) putchar(' ');
  putchar('|');
  for (i = info->MaxLoad + 1; i < (field_width + 1); i++) putchar(' ');

  printf("|%2dMb:", memsize / (1024 * 1024));

  printf("%3d:", mem_percentage);
  if (mem > info->MaxMem) info->MaxMem = mem;
  for (i = 0; i < mem; i++) putchar('=');
  for (i = mem; i < info->MaxMem; i++) putchar(' ');
  putchar('|');
  for (i = info->MaxMem + 1; i < (field_width + 1); i++) putchar(' ');
  fputs("|\n", stdout);
  return(TRUE);  
}

/*}}}*/

/**
*** Displaying a whole screen-full involves moving to the top-left
*** corner of the screen to display the banner. Next the processors to
*** be displayed are determined and held in a table, to allow for
*** re-calculation of the current processor. There are various nasty
*** cases to consider here, including page forwards, page back,
*** switching to tagged mode, ...
**/
static	RmProcessor	screen_table[128];

static void display_screen(void)
{ int	number_displayed 	= 0;
  int	temp_proc;
  int	i;

  memset(screen_table, 0, sizeof(screen_table));

  display_banner();

	/* Step 1 : determine the first visible processor.		*/
	/*          If first_processor is already visible this is a	*/
	/*	    no-op, otherwise the code searches forwards and	*/
	/*	    then backwards.					*/
  while ((first_processor < number_processors) &&
	 (!is_processor_visible(first_processor)))
   first_processor++;

  while ((first_processor >= 0) &&
	 (!is_processor_visible(first_processor)))
   first_processor--;

	/* If we failed to find any visible processors, abort here	*/
  if (first_processor < 0)
   { first_processor = last_processor = current_processor = -1;
     for (i = 0; i < number_to_display; i++)
      printf("\033[K\n");      
     return;
   }
	/* Step 2: determine the processors that will actually be	*/
	/*         displayed, updating last_processor in the process.	*/
  temp_proc	= first_processor;
  while ((number_displayed < number_to_display) &&
	 (temp_proc < number_processors))
   { if (is_processor_visible(temp_proc))
      { last_processor			 = temp_proc;
	screen_table[number_displayed++] = procvec[temp_proc];
      }
     temp_proc++;
   }

	/* At this point we have the following	:			*/
	/* 1) first_processor is the start of the display		*/
	/* 2) last_processor is the end of the display			*/
	/* 3) current_processor is indeterminate.			*/

	/* If current_processor is nowhere on the screen, switch back	*/
	/* to first_processor.						*/
  if ((current_processor < first_processor) || (current_processor > last_processor))
   current_processor = first_processor;

	/* If current_processor is withing the required range but not	*/
	/* visible, search forwards, then backwards for a visible	*/
	/* processor.							*/
  else
   { while (!is_processor_visible(current_processor) && (current_processor <= last_processor))
      current_processor++;
     while (!is_processor_visible(current_processor) && (current_processor >= first_processor))
     current_processor--;
   }

	/* current_processor is now guaranteed to be visible.		*/
	/* Display all the selected processors.				*/
  for (i = 0; i < number_displayed; i++)
   display_processor(screen_table[i]);

	/* clear the remaining lines.					*/
  if (total_redraw)
   { for (i = number_displayed; i < number_to_display; i++)
      printf("\033[K\n");
   }
}

/*}}}*/
/*{{{  menu */
/**
*** menu handling. There are separate routines to cover the various
*** menu options, some of which can get quite complicated.
**/
/*{{{  next page */
/**
*** Switching to the next page. This involves updating first_processor
*** appropriately, skipping past number_to_display visible processors.
*** However, to avoid having a nearly empty screen at the end I want
*** to walk back from the end of the network. 
**/
static void next_page(void)
{ int	skipped;
  int	tail;

  for ( skipped = 0;
	(skipped < number_to_display) && (first_processor < number_processors);
	first_processor++)
   if (is_processor_visible(first_processor))
    skipped++;

  for (tail = number_processors - 1, skipped = 1;
	(skipped < number_to_display) && (tail >= 0);
	tail--)
   if (is_processor_visible(tail))
    skipped++;

  if (tail < 0) tail = 0;
  if (first_processor > tail) first_processor = tail;
}
/*}}}*/
/*{{{  previous page */
/**
*** Previous page - this involves skipping back number_to_display visible
*** processors.
**/
static void previous_page(void)
{ int	skipped = 0;

  while ((first_processor > 0) && (skipped < number_to_display))
   { first_processor--;
     if (is_processor_visible(first_processor))
      skipped++;
   }
}
/*}}}*/
/*{{{  first processor */
/**
*** Going back to the first page always involves setting first_processor
*** to 0. display_screen() will sort out the rest.
**/
static void first_page(void)
{ 
  first_processor = 0;
}

/*}}}*/
/*{{{  last processor */
/**
*** Going to the last processor involves searching from the end for
*** a visible processor which becomes the current one, and then searching
*** back further to find enough visible ones to fill the screen.
**/
static void	last_page(void)
{ int	skipped = 0;

  first_processor = number_processors - 1;
  while ((first_processor > 0) && !(is_processor_visible))
   first_processor--;
  current_processor = first_processor;

  while ((first_processor > 0) && (skipped < number_to_display))
   { if (is_processor_visible(first_processor))
      skipped++;
     first_processor--;
   }
}
/*}}}*/
/*{{{  cursor keys */
/**
*** Cursor key handling. The 0x9B CSI has been detected. One additional
*** character should be read in, to identify the cursor key.
*** If it is not a recognised cursor key another read is used to get rid of
*** any rubbish characters left over in the input buffer. current_processor
*** is incremented/decremented as appropriate. If possible, the screen
*** is updated here rather than forcing a complete and expensive redraw.
*** If a page switch is required then a complete redraw is better.
*** Note that display_screen() will sort out any problems such as
*** going past the last processor.
**/
static bool cursor_key(void)
{ BYTE	buf[16];
  int	old_current	= current_processor;
  int	i;

  if (Read(fdstream(0), buf, 1, 2 * OneSec) ne 1)
   /*	What's up doc ?  */
   return(FALSE);

  switch(buf[0])
   { case	'B' : 	/* down-arrow */
	if (current_processor eq -1) return(TRUE);
	while ((++current_processor < number_processors) &&
	       !is_processor_visible(current_processor));
	if (current_processor >= number_processors)
	 current_processor = last_processor;
        if (current_processor > last_processor) 
         next_page();
	else
	 goto redraw;
        return(FALSE);

     case	'A' :	/* up-arrow */
	if (current_processor eq -1) return(TRUE);
	while ((--current_processor >= 0) &&
		!is_processor_visible(current_processor));
        if (current_processor < first_processor)
	 previous_page();
	else
	 goto redraw;
	return(FALSE);;

     default	    : 
	(void) Read(fdstream(0), buf, 16, OneSec);
	return(FALSE);
   }

redraw:
  for (i = 0; i < number_to_display; i++)
   if (screen_table[i] == procvec[old_current])
    { move_to(i + 2, 0);
      printf("%*s", max_proc_name, RmGetProcessorId(procvec[old_current]));
      break;
    }
  for (i = 0; i < number_to_display; i++)
   if (screen_table[i] == procvec[current_processor])
    { move_to(i + 2, 0);
      printf("\033[7m%*s\033[0m", max_proc_name, RmGetProcessorId(procvec[current_processor]));
      break;
    }
  return(TRUE);

}
/*}}}*/
/*{{{  tag */
static void	tag_current(void)
{ ProcessorInfo	*info;

  if (current_processor eq -1) return;
  info = (ProcessorInfo *) RmGetProcessorPrivate(procvec[current_processor]);
  info->Tagged = TRUE;
}
/*}}}*/
/*{{{  untag */
static void untag_current(void)
{ ProcessorInfo	*info;
  if (current_processor eq -1) return;
  info = (ProcessorInfo *) RmGetProcessorPrivate(procvec[current_processor]);
  info->Tagged = FALSE;
}
/*}}}*/
/*{{{  display tagged */
/**
*** Toggling the display_tagged flag. It seems like a good idea to
*** save the old state of first_processor etc. when switching to
*** the tagged processors.
**/
static int	save_first, save_current, save_last;

static void display_tagged_processors(void)
{
  if (!display_tagged)
   { save_first		= first_processor;
     save_current	= current_processor;
     save_last		= last_processor;
     first_processor = 0;
   }
  else
   { first_processor	= save_first;
     current_processor	= save_current;
     last_processor	= save_last;
   }
  display_tagged  = !display_tagged;
}
/*}}}*/
/*{{{  examine */
/**
*** Examining the current processor. This is a combination of
*** network show, ps, and loaded.
**/

/*{{{  WalkDir() */
static WORD WalkDir2(Object *dir, WordFnPtr fn)
{ WORD  	sum = 0;
  Stream  	*s;
  WORD		size, i;
  DirEntry	*entry, *cur;
  
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
  if (entry eq Null(DirEntry))
   { fputs("ps : out of memory\n", stderr);
     Close(s); 
     return(0); 
   }
     
  if (Read(s, (BYTE *) entry, size, -1) ne size)
   { fprintf(stderr, "ps : error reading directory %s\n", dir->Name);
     Close(s); Free(entry);
     return(0);
   }
  Close(s);
      
  cur = entry;
  for (i = 0; i < size; cur++, i += sizeof(DirEntry) )
   { if ( (!strcmp(cur->Name, ".")) || (!strcmp(cur->Name, "..")) )
      continue;
     sum += (*fn)(cur->Name);
   }

  Free(entry);
  return(sum);
}
/*}}}*/
/*{{{  ps */
/**
*** This code deals with displaying the tasks within a processor.
*** It involves reading the /tasks directory of that processor,
*** i.e. a simple WalkDir with a display function. The only problem
*** is keeping the display tidy, which can be done using a static.
**/

static int task_count;
static WORD display_task(char *task)
{ task_count++;
  if (task_count eq 5)
   { printf("\n    ");
     task_count = 1; 
   }
  printf("%-16s", task);
  return(0);
}

static WORD display_procman(Object *processor)
{ Object	*procman;

  procman = Locate(processor, "tasks");
  if (procman eq Null(Object)) goto done;
  task_count = 4;
  (void) WalkDir2(procman, &display_task);
  Close(procman);
  
done:
  putchar('\n');
  return(0);
}

/*}}}*/
/*{{{  loaded */
/**
*** This code deals with displaying the loader within a processor.
*** It involves reading the /loader directory of that processor,
*** i.e. a simple WalkDir with a display function. The only problem
*** is keeping the display tidy, which can be done using a static.
**/

static int loader_count;
static WORD display_code(char *loader)
{ loader_count++;
  if (loader_count eq 5)
   { printf("\n    ");
     loader_count = 1; 
   }
  printf("%-16s", loader);
  return(0);
}

static WORD display_loader(Object *processor)
{ Object	*loader;

  loader = Locate(processor, "loader");
  if (loader eq Null(Object)) goto done;
  loader_count = 4;
  (void) WalkDir2(loader, &display_code);
  Close(loader);
  
done:
  putchar('\n');
  return(0);
}
/*}}}*/

static void examine_current(void)
{ Object	*real_processor;

  if (current_processor eq -1) return;
  putchar('\n');

  init_PrintNetwork(FALSE);
  PrintProcessor(procvec[current_processor], 0);
  tidy_PrintNetwork();

  real_processor = RmMapProcessorToObject(procvec[current_processor]);
  if (real_processor ne Null(Object))
   { printf("\nRunning Tasks:");
     display_procman(real_processor);
     printf("\nLoaded Code:");
     display_loader(real_processor);
     Close(real_processor);
   }

  waitfor_user();
  screen_height = screen_width = -1;
}
/*}}}*/
/*{{{  map */
/**
*** map in the foreground. This command is used to run map on the current
*** processor using the current window.
BLV signals ?
**/
static void do_map(void)
{ int	pid, rc;

  if (current_processor eq -1) return;

  if ((pid = vfork()) eq 0)
   { char	*argv[4];
     argv[0]	= "remote";
     argv[1]	= (char *) RmGetProcessorId(procvec[current_processor]);
     argv[2]	=  "/helios/bin/map";
     argv[3]	= NULL;
     execv("/helios/bin/remote", argv);
     _exit(1);
   }
  else
   waitpid(pid, &rc, 0);

  initialise_screen();	/* reset attributes etc. */
  if (rc ne 0) waitfor_user();
  screen_width = screen_height = -1;
}
/*}}}*/
/*{{{  run map */
/**
*** map in the background. This command is used to run map on the current
*** processor using another way. For simplicity this goes via the run
*** command. N.B. The order is significant, to prevent the TFM from
*** releasing the processor too early.
**/
static void run_map(void)
{ int	pid, rc;

  if (current_processor eq -1) return;

  if ((pid = vfork()) eq 0)
   { char	*argv[6];
     argv[0]	= "run";
     argv[1]	= "remote";
     argv[2]	= "-d";
     argv[3]	= (char *) RmGetProcessorId(procvec[current_processor]);
     argv[4]	=  "/helios/bin/map";
     argv[5]	= NULL;
     execv("/helios/bin/run", argv);
     _exit(1);
   }
  else
   waitpid(pid, &rc, 0);

  initialise_screen();	/* reset attributes etc. */
  if (rc ne 0) waitfor_user();
  screen_width = screen_height = -1;
}
/*}}}*/
/*{{{  shell escape */
/**
*** Escaping from a shell simply means exec'ing the shell. When the shell
*** returns the screen can be redrawn immediately, there is no need to
*** prompt the user.
**/
static void shell_escape(void)
{ int	pid, rc;

  if ((pid = vfork()) eq 0)
   { execl("/helios/bin/shell", "shell",  NULL);
     _exit(0);
   }
  else
   waitpid(pid, &rc, 0);
  initialise_screen();	/* reset attributes etc. */
  screen_width = screen_height = -1;
}
/*}}}*/
/*{{{  help */
/**
*** display help information
**/
static char	*text1 = "\
Network Monitor - help information\n\n\
The main display is dedicated to some or all of the processors that\n\
are being monitored. Each line contains the following:\n\
  Name   : one processor, the current one, should be highlighted\n\
  Tag    : if there is an * character after the name then this processor\n\
           has been tagged.\n\
  Load   : the current CPU load on that processor is shown as a percentage\n\
           and graphically. The maximum load detected so far will be\n\
           indicated as well.\n\
  Memory : the total amount of memory on that processor, and the proportion\n\
           that is currently used.\n\n\
";

static char	*text2 = "\
If there are too many processors in the network to display on one screen\n\
then it is possible to move from one screen to the next or to the previous\n\
ones. In addition it is possible to return to the first processor or to\n\
to move directly to the last processor.\n\n\
";

static char	*text3 = "\
It may be desirable to select a small number of processors while the\n\
monitor program is running. This can be achieved by tagging the\n\
required processors and then choosing the Display-tagged option.\n\
If a processor is no longer of interest it can be untagged.\n\
To get back to normal mode, choose the Display-tagged option again.\n\n\
";

static char	*text4 = "\
The cursor-up and cursor-down keys can be used to change the current\n\
processor. Certain operations affect the current processor only:\n\
  Examine : this provides information about the current processor\n\
            such as ownership, link connections, running programs,\n\
            and loaded code.\n\
  Map     : this executes the Helios map program on that processor.\n\
  Run map : this creates another window and runs map in that window\n\
            in the background.\n\n\
";

static char	*text5 ="\
To run another shell, use the ! command.\n\n\
The interval between monitor updates can be decreased using the + key,\n\
and increased using -.\n\n\
? gives this help information.\n\n\
Q should be used to exit the network monitor.\n\n\
";
 

static void display_help()
{ FILE	*output;

  output = popen("/helios/bin/more", "w");
  if (output == NULL)
   output = stdout;
  fputs(text1, output);
  fputs(text2, output);
  fputs(text3, output);
  fputs(text4, output);
  fputs(text5, output);
  if (output != stdout)
   pclose(output);

  initialise_screen();
  waitfor_user();
  screen_height = screen_width = -1;
}
/*}}}*/

static void menu(void)
{ static char	*inverse = "\033[7m";
  static char	*normal	 = "\033[0m";
  static int	 state	 = 0;
  static char	*rotate	 = "|/-\\";
  BYTE	 buf[4];

back:
  if (total_redraw)
   { move_to(screen_height - 2, 0);
     printf("%sF%sirst, %sL%sast, %sN%sext, %sP%srev, %sT%sag, %sU%sntag, %sD%sisplay tagged,\n",
	inverse, normal, inverse, normal, inverse, normal,
	inverse, normal, inverse, normal,
	inverse, normal, inverse, normal);
     printf("%sE%sxamine, %sM%sap, %sR%sun map, %s!%s, %s+%s, %s-%s, %s?%s, %sQ%suit.\n",
	inverse, normal, inverse, normal, inverse, normal,
	inverse, normal, inverse, normal, inverse, normal,
	inverse, normal, inverse, normal);
   }
  else
   move_to(screen_height, 0);

  putchar(rotate[state]); putchar('\b');
  state = (state + 1) % 4;
  fflush(stdout);

  total_redraw	= FALSE;	/* set only if the user presses a key	*/
    
  if (Read(fdstream(0), buf, 1, monitor_delay * OneSec) < 1) return;

  /*{{{  invalidate screen */
  total_redraw = TRUE;
  { int	i;
    for (i = 0; i < number_processors; i++)
     { ProcessorInfo	*info = (ProcessorInfo *) RmGetProcessorPrivate(procvec[i]);
       info->CurLoad = info->CurMem = -1;
     }
   }
  /*}}}*/
  /*{{{  big switch statement */
  switch(buf[0])
     { case 'q'	:
       case 'Q'	: 
       case 0x03: exit(EXIT_SUCCESS);
  
       case ' '	:
       case '\n'	:
       case '\r'	: break;		/* no-op */
  
       case 0x0C	: screen_width = screen_height = -1;	/* ctrl-L redraw */
  		  break;
  
       case '+'	:			/* reduce delay		*/
  		  if ((monitor_delay > 2) && (monitor_delay < 5))
  		   monitor_delay -= 1;
  	          elif ((monitor_delay >= 5) && (monitor_delay < 10))
  		   monitor_delay -= 2;
  		  elif (monitor_delay > 10)
  		   monitor_delay -= (monitor_delay / 4);
  		  break;
       case '-'	:			/* increase monitor_delay	*/
  		  if (monitor_delay < 5)
  		   monitor_delay += 1;
  		  elif (monitor_delay < 10)
  		   monitor_delay += 2;
  		  else
  		   monitor_delay += (monitor_delay / 4);
  		  break;
  
       case 'n'	:
       case 'N'	: next_page(); current_processor = -1; break;
       case 'p'	:
       case 'P'	: previous_page(); current_processor = -1; break;
       case 'f'	:
       case 'F'	: first_page(); current_processor = -1; break;
       case 'l'	:
       case 'L'	: last_page(); break;
       case 't'	:
       case 'T'	: tag_current(); break;
       case 'u'	:
       case 'U'	: untag_current(); break;
       case 'd'	:
       case 'D'	: display_tagged_processors(); break;
  
       case 0x09B :
       case 0xFFFFFF9B :
  			/* Cursor keys may or may not involve a complete redraw */
  		  if (cursor_key())
  		   goto back;
  		  else
     		   return;
  
       case '!'	: shell_escape(); break;
       
       case 'm' :
       case 'M' : do_map(); break;
  
       case 'r' :
       case 'R' : run_map(); break;
  
       case 'e' :
       case 'E' :
       case 'x' :
       case 'X' : examine_current(); break;
  
       case '?' :
       case 'h' :
       case 'H' : display_help(); break;
  
     }
  /*}}}*/
 
}

/*}}}*/
/*{{{  MonitorNetwork() - entrypoint */

/**
*** This routine is the entry point for this library facility. It never
*** returns. When the user chooses to exit the whole program is aborted.
***  1) initialise the supplied network, associating a ProcessorInfo
***	structure with every processor in the network and determining
***	the maximum length of a processor name.
***  2) initialise the screen, setting the keyboard into raw input mode
***  3) forever,
***	a) allow for a screen resize
***	b) display the current screen
***	c) prompt the user for some input
**/
void MonitorNetwork(RmNetwork network, int delay)
{ int	count;
  int	index;

  { static char	buf[500];
    setvbuf(stdout, buf, _IOFBF, 500);
  }
  number_processors = RmCountProcessors(network);
  if (number_processors eq 0)
   { fputs("MonitorNetwork: the supplied the network is empty.\n", stderr);
     exit(EXIT_FAILURE);
   }

  procvec = (RmProcessor *)Malloc((word) RmCountProcessors(network) * sizeof(RmProcessor));
  if (procvec eq NULL)
   { fputs("MonitorNetwork: out of memory.\n", stderr);
     exit(EXIT_FAILURE);
   }
  index	= 0;
  count = RmApplyProcessors(network, &initialise_processor, &index);
  if (count <= 0)
   { fputs("MonitorNetwork: no processors to monitor.\n", stderr);
     exit(EXIT_FAILURE);
   }
  if (max_proc_name < 4) max_proc_name = 4;

	/* Impose a minimum monitor_delay between redraws, to allow the	*/
	/* keyboard a sensible timeout.				*/
  if (delay < 2)
   monitor_delay = 2;
  else
   monitor_delay = delay;

  initialise_screen();

  forever
   { size_screen();
     display_screen();
     menu();
   }
}

/*}}}*/
