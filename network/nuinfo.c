/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils : nuinfo.c							--
--		Display statistics about a supplied network, using	--
--	the more filter.						--
--									--
--	Author:  BLV 12/4/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/nuinfo.c,v 1.3 1993/08/12 11:31:50 nickc Exp $*/

/*{{{  includes */
#include <helios.h>
#include <syslib.h>
#include <attrib.h>
#include <root.h>
#include <rmlib.h>
#include <gsp.h>
#include <stdio.h>
#include <posix.h>
#include <stdlib.h>
#include <string.h>
#include <nonansi.h>
#include <codes.h>
/*}}}*/
/*{{{  statics */
static	int	 screen_width	= 80;
static	int	 screen_height	= 25;
static	FILE	*output_file	= NULL;
static	int	 max_proc_name	= -1;
static	int	 field_width;
/*}}}*/
/*{{{  screen initialisation */
/**
*** Attempt to determine basic screen information such as the number of
*** columns. This routine will default to sensible values on most failures.
**/
static	void	init_screen(void)
{ Attributes	attr;

  unless(isatty(1))
   { fputs("DisplayInfo: stdout is not a tty\n", stderr);
     exit(EXIT_FAILURE);
   }

  if (GetAttributes(fdstream(1), &attr) >= Err_Null)
   { screen_width = attr.Time;
     screen_height = attr.Min;
   }

  field_width  = (screen_width - (max_proc_name + 18)) / 2;
  if (field_width < 10)
   { fputs("DisplayInfo : screen too small.\n", stderr);
     exit(EXIT_FAILURE);
   }
}
/*}}}*/
/*{{{  processor name lengths */
/**
*** For proper output formatting it is necessary to know the maximum
*** length of a processor name. This function is applied to all the
*** processors in the supplied network. If appropriate the global variable
*** max_proc_name is updated.
**/
static	int	determine_maxlen(RmProcessor processor, ...)
{ int	len = strlen(RmGetProcessorId(processor));

  if (((RmGetProcessorPurpose(processor) & RmP_Mask) == RmP_Helios) &&
      (len > max_proc_name))
   max_proc_name = len;
  return(0);
}
/*}}}*/
/*{{{  get info */
/**
*** This routine is responsible for getting performance information
*** about a particular processor.
**/
static ProcStats *get_info(RmProcessor processor, BYTE *data)
{ Object	*real_proc	= Null(Object);
  Object	*procman	= Null(Object);
  ProcStats	*result		= Null(ProcStats);

  real_proc = RmMapProcessorToObject(processor);
  if (real_proc == Null(Object)) goto done;

  procman = Locate(real_proc, "tasks");
  if (procman == Null(Object)) goto done;

  if (ServerInfo(procman, data) >= Err_Null)
   result = (ProcStats *) data;

done:
  if (real_proc != Null(Object)) Close(real_proc);
  if (procman != Null(Object)) Close(procman);
  return(result);
}
/*}}}*/
/*{{{  display_banner */
/**
*** This routine displays a simple banner at the start.
***   Proc:    % CPU Load       % Memory
**/
static void display_banner(void)
{
  fprintf(output_file, "%*s   %% %-*s         %% %-*s\n",
		max_proc_name, "Proc",
		field_width - 1, "CPU Load",
		field_width - 1, "Memory");
}
/*}}}*/
/*{{{  display info */

/**
*** This routine is responsible for displaying the info associated with
*** a processor, in the following format:
*** <name>: xxx:=======               | 8Mb:xxx:                      |
**/
static int	display_info(RmProcessor processor, ...)
{ BYTE		 data[IOCDataMax];
  ProcStats	*proc_stats;
  int		 load;
  int		 memsize;
  int		 mem;
  int		 i;

  if ((RmGetProcessorPurpose(processor) & RmP_Mask) != RmP_Helios)
   return(0);

  proc_stats = get_info(processor, data);
  if (proc_stats == Null(ProcStats))
   return(0);

  fprintf(output_file, "%*s:", max_proc_name, RmGetProcessorId(processor));

	/* determine the processor load. This is effectively a number	*/
	/* between 0 and 2050, although the exact details can vary if	*/
	/* new threads are started or if threads halt.			*/
  load = (int) proc_stats->Load;
  if (load > 2050) load = 2050;		/* allow for overflow		   */
  fprintf(output_file, "%3d:", (100 * load) / 2050);
  load = (load * field_width) / 2050;	/* scale according to screen width */
  for (i = 0; i < load; i++) fputc('=', output_file);
  for (i = load; i < field_width; i++) fputc(' ', output_file);

	/* The memory size provided by the nucleus does not include	*/
	/* the nucleus code nor, I believe, some kernel static data.	*/
	/* I assume that all processor memory sizes are multiples of a	*/
	/* megabyte...							*/
  memsize = (int) proc_stats->MemMax;
  memsize = (memsize + ((1024 * 1024) - 1)) & ~((1024 * 1024) - 1);
  fprintf(output_file, "|%2dMb:", memsize / (1024 * 1024));

	/* The nucleus provides the amount of free memory. Again this	*/
	/* has to be scaled according to the field width.		*/  
  mem = (int)(memsize - proc_stats->MemFree);
  fprintf(output_file, "%3d:", (100 * mem) / memsize);
  mem = (mem * field_width) / memsize;
  for (i = 0; i < mem; i++) fputc('=', output_file);
  for (i = mem; i < field_width; i++) fputc(' ', output_file);
  fputs("|\n", output_file);
  return(0);
}

/*}}}*/
/*{{{  DisplayInfo() - entrypoint */
/**
*** This routine is called from the application with the network details.
*** It must perform the following:
***  1) determine the maximum processor name length, to allow for formatting
***  2) determine screen details and use these to calculate the required
***     widths of the various fields
***  3) open an output filter if possible
***  4) display the network details.
**/
void DisplayInfo(RmNetwork network)
{
  if (network == (RmNetwork) NULL) return;
  (void) RmApplyProcessors(network, &determine_maxlen);
  if (max_proc_name == -1)
   { fputs("DisplayInfo: no Helios processors specified.\n", stderr);
     return;
   }
  if (max_proc_name < 4) max_proc_name = 4;

  init_screen();
  if (RmCountProcessors(network) >= screen_height)
   { output_file = popen("more", "w");
     if (output_file == NULL)
      output_file = stdout;
   }
  else
   output_file = stdout;

  display_banner();
  RmApplyProcessors(network, &display_info);

  if (output_file != stdout)
   pclose(output_file);
}

/*}}}*/






