/*------------------------------------------------------------------------
--                                                                      --
--      H E L I O S   P A R A L L E L   P R O G R A M M I N G		--
--	-----------------------------------------------------		--
--									--
--	  F A R M   C O M P U T A T I O N   L I B R A R Y		--
--	  -----------------------------------------------		--
--									--
--		Copyright (C) 1992, Perihelion Software Ltd.		--
--                        All Rights Reserved.                          --
--                                                                      --
-- farmtest.c								--
--		Part of the farm library test harness, together with	--
--	testaux.c							--
--                                                                      --
--	Author:  BLV 28/10/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: farmtest.c,v 1.1 1992/10/30 19:00:08 bart Exp $ */

/*{{{  description */
/*
** This test program is responsible for checking the behaviour of the
** farm library. It works by invoking another program, testaux, using
** suitable arguments. The program can be run interactively using a
** simple menu, or it can just go through all the tests one by one.
*/
/*}}}*/
/*{{{  version number and history */
static char *VersionNumber	= "1.00";
/*
** Revision history :
**
**	1.00,	first version
*/
/*}}}*/
/*{{{  header files etc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <helios.h>
#include <syslib.h>
#include <attrib.h>
#include <nonansi.h>
#include <codes.h>
#include <rmlib.h>
/*}}}*/
/*{{{  the table of tests */
/*
** All tests are defined by a simple name, which also happens to be the
** argument passed to testaux. The various tests are held in a simple
** NULL-terminated table.
*/
static char *Tests[] = {
#if 0
	"basic operation 1",
	"basic operation 2",
#endif
	"flood network",
	"flood domain",
	"flood fixed",
	"flood fixed single",
	"flood fixed many",
	"flood processors",
	"FmCountProcessors",
	"FmNumberWorkers",
	"FmOverloadController",
	"overload single",
	"FmWorkerNumber",
	"FmInWorker",
	"FmFastCode",
	"FmFastStack",
	"info 1",
	"info 2",
	"FmRand",
	"FmSeed",
	"FmWorkerExit",
	"FmWorkerInitialise",
	"FmControllerExit",
	"FmControllerInitialise",
#if 0
	"fault 1",
	"fault 2",
	"FmIsRunning",
	"FmRunningWorkers",
#endif
	NULL
};
/*}}}*/
/*{{{  run a test */
static void run_test(int number)
{ int	 pid;
  char	*argv[3];
  int	 res;

  printf("Running test %s\n\n", Tests[number]);
  argv[0]	= "testaux";
  argv[1]	= Tests[number];
  argv[2]	= NULL;

  pid = vfork();
  if (pid == 0)
   { execv(argv[0], argv);
     _exit(1);
   }

  (void) waitpid(pid, &res, 0);
  if (res != 0)
   printf("Warning, test returned exit code 0x%08x\n", res); 
  printf("\nTest %s done\n", Tests[number]);
}
/*}}}*/
/*{{{  menu */
/*{{{  screen manipulation */
/**
*** This contains the usual routines to initialise the screen and move
*** the cursor.
**/
static	int	ScreenHeight	= 25;
static	int	ScreenWidth	= 80;

static void	initialise_screen(void)
{ Attributes	attr;

  unless (isatty(0) && isatty(1))
   { fputs("farmtest: not running interactively.\n", stderr);
     exit(EXIT_FAILURE);
   }

  if (GetAttributes(fdstream(0), &attr) < Err_Null)
   { fputs("farmtest: failed to get keyboard details.\n", stderr);
     exit(EXIT_FAILURE);
   }

  AddAttribute(&attr, ConsoleRawInput);
  RemoveAttribute(&attr, ConsoleEcho);
  RemoveAttribute(&attr, ConsoleRawOutput);
  if (SetAttributes(fdstream(0), &attr) < Err_Null)
   { fputs("farmtest: failed to initialise keyboard.\n", stderr);
     exit(EXIT_FAILURE);
   }
  ScreenHeight	= attr.Min;
  ScreenWidth	= attr.Time;

  setvbuf(stdin, NULL, _IONBF, 0);
}

static void move_to(int y, int x)
{ printf("\033[%d;%dH", y, x);
}

static void clear_screen()
{ putchar('\f');
}

static void waitfor_user()
{ int	x;

  move_to(ScreenHeight, 1);
  fputs("\033[KPress any key to continue.", stdout);
  fflush(stdout);
  x = getchar();
  move_to(ScreenHeight, 1);
  fputs("\033[K", stdout);
  fflush(stdout);
}
/*}}}*/
/*{{{  statics */
/*
** These statics are used to manage the menu.
*/
static	int	First;		/* index of first function	*/
static	int	Current;	/* ditto for current		*/
static	int	NumberFuncs;	/* total number of tests	*/

static  char	*Normal	 = "\033[0m";	/* normal video		*/
static	char	*Inverse = "\033[7m";	/* inverse video	*/
/*}}}*/
/*{{{  first */
static void first_test(void)
{ Current = First = 0;
}
/*}}}*/
/*{{{  last */
static void last_test(void)
{ 
  Current = NumberFuncs - 1;
  while ((First + 30) < NumberFuncs)
   First += 30;
}
/*}}}*/
/*{{{  next */
static void next_page(void)
{ if ((First + 30) < NumberFuncs)
   { First += 30; Current = First; }
}
/*}}}*/
/*{{{  prev */
static void previous_page(void)
{ if (First > 0)
   { First -= 30; Current = First; }
}
/*}}}*/
/*{{{  up */
static void up_line(void)
{ if ((Current == First) || (Current == (First + 15)))
   { if ((Current + 14) < NumberFuncs)
      Current += 14;
     else
      Current = NumberFuncs - 1;
   }
  else
   Current--;
}
/*}}}*/
/*{{{  down */
static void down_line(void)
{ if ((Current == (First + 14)) || (Current == (First + 29)))
   Current -= 14;
  else
   { if (Current == (NumberFuncs - 1))
      { if (Current < (First + 15))
	 Current = First;
	else
	 Current = First + 15;
      }
     else
      Current++;
   }
}
/*}}}*/
/*{{{  left */
static void left(void)
{ 
  if (Current >= (First + 15))	/* in second column	*/
   Current -= 15;
  elif ((Current + 15) < NumberFuncs)
   Current += 15;
}
/*}}}*/
/*{{{  right */
static void right(void)
{
  if (Current >= (First + 15))	/* in first column	*/
   Current -= 15;
  elif ((Current + 15) < NumberFuncs)
   Current += 15;
}
/*}}}*/
/*{{{  CSI handling */
static bool cursor_key(void)
{ char	buf[16];

  if (Read(fdstream(0), buf, 1, OneSec) < 1)
   return(TRUE);	/* What's up Doc ?	*/

  if ((buf[0] >= 'A') && (buf[0] <= 'D'))	/* cursor key */
   { switch (buf[0])
      { case 'A'	: up_line(); break;
	case 'B'	: down_line(); break;
	case 'C'	: right(); break;
	case 'D'	: left(); break;
      }
     return(TRUE);	/* no redrawing needed	*/
   }

  if (buf[0] == 'H')	/* HOME	*/
   { first_test(); return(FALSE); }

  if ((buf[0] >= '2') && (buf[0] <= '4'))	/* potentially END, PgUp, PgDn */
   { if (Read(fdstream(0), &(buf[1]), 1, OneSec) < 1)
      return(TRUE);		/* What's up Doc ?	*/

     if (buf[1] == '~')		/* Sorry, function key	*/
      return(TRUE);

     if (buf[1] == 'z')		/* Bingo		*/
      { switch(buf[0])
	 { case '2'	: last_test(); break;		/* END	*/
	   case '3'	: previous_page(); break;	/* PgUp	*/
	   case '4'	: next_page(); break;		/* PgDn	*/
	 }
	return(FALSE);
      }
   }

	/* The start of a CSI has been detected but it is not a	*/
	/* recognised one. Discard the rest of it.		*/
  (void) Read(fdstream(0), buf, 16, OneSec);
  return(TRUE);
}
/*}}}*/
/*{{{  run current test */
static void do_test(void)
{ clear_screen();
  run_test(Current);
  waitfor_user();
}
/*}}}*/
/*{{{  shell escape */
static void shell_escape(void)
{ int	pid, rc;

  printf("\n"); fflush(stdout);

  if ((pid = vfork()) == 0)
   { execl("/helios/bin/shell", "shell", NULL);
     _exit(0);
   }
  else
   waitpid(pid, &rc, 0);
  initialise_screen();
}
/*}}}*/

static void menu(void)
{ int	i;
  int	input;

  First = Current = 0;
  for (NumberFuncs = 0; Tests[NumberFuncs] != NULL; NumberFuncs++);

  initialise_screen();
  if ((ScreenHeight < 20) || (ScreenWidth < 60))
   { fprintf(stderr, "farmtest: screen too small.\n");
     exit(EXIT_FAILURE);
   }

  forever
   { clear_screen();
     printf("\t\tFarm Library Test Suite Version %s", VersionNumber);
     
     for (i = 0; i < 15; i++)
      { if ((First + i) == NumberFuncs) break;
	move_to(3 + i, 1);
	fputs(Tests[First+i], stdout);
      }
     for (i = 15; i < 30; i++)
      { if ((First + i) == NumberFuncs) break;
	move_to(3 + i - 15, ScreenWidth / 2);
	fputs(Tests[First+i], stdout);
      }
back:
     if (Current >= (First + 15))
      move_to(3 + (Current - First) - 15, ScreenWidth / 2);
     else
      move_to(3 + (Current - First), 1);
     printf("%s%s%s", Inverse, Tests[Current], Normal);

     move_to(ScreenHeight - 1, 1);
     printf("%sF%sirst, %sL%sast, %sN%sext, %sP%srev, %sQ%suit, %s!%s shell\n",
		Inverse, Normal,Inverse, Normal, Inverse, Normal,
		Inverse, Normal,Inverse, Normal, Inverse, Normal);
     printf("Use cursor keys to select, return to run test.");
     fflush(stdout);

     input = getchar();
     if (Current >= (First + 15))
      move_to(3 + (Current - First) - 15, ScreenWidth / 2);
     else
      move_to(3 + (Current - First), 1);
     fputs(Tests[Current], stdout);
     move_to(ScreenHeight, ScreenWidth);
     fflush(stdout);

     switch(input)
      { case 'q'	:
	case 'Q'	:
	case 0x03	:	/* ctrl-C	*/
	case 0x04	: 	/* ctrl-D	*/
	case 0x07	:	/* ctrl-G	*/
			  fputs("\n", stdout);
			  return;

	case 0x0C	: continue;	/* ctrl-L, redraw	*/

	case 'f'	:
	case 'F'	: first_test(); continue;

	case 'l'	:
	case 'L'	: last_test(); continue;

	case 0x16	:	/* ctrl-V	*/
	case 'n'	:
	case 'N'	: next_page(); continue;

	case 0x1A	:	/* ctrl-Z	*/
	case 'p'	:
	case 'P'	: previous_page(); continue;

	case 0x0E	: down_line();	/* ctrl-N	*/
			  break;

	case 0x10	: up_line();	/* ctrl-P	*/
			  break;

	case 0x09	:			/* tab		*/
	case 0x06	: right();		/* ctrl-F	*/
			  break;

	case 0x02	: left();		/* ctrl-B	*/
			  break;

	case 0x09B	:
	case 0xFFFFFF9B	: if (cursor_key())
				break;
			  else
				continue;

	case '!'	: shell_escape(); continue;

	case 0x0A	:
	case 0x0D	: do_test(); continue;

	default		: break;
      }

	/* By default no redrawing is done. The various cases in the	*/
	/* switch statement should use continue to perform a redraw.	*/
     goto back;
     input = input;
   }
}
/*}}}*/
/*{{{  main */
/*{{{  usage() */
static void usage(void)
{ fputs("farmtest: usage, farmtest [-m]\n", stderr);
  exit(EXIT_FAILURE);
}
/*}}}*/
/*{{{  check the world */
static int	NumberProcessors	= 0;

static int network_filter(RmProcessor processor, ...)
{ int	purpose	= RmGetProcessorPurpose(processor);
  int	state	= RmGetProcessorState(processor);
  int	owner	= RmGetProcessorOwner(processor);
  bool	ok	= FALSE;

  if (purpose & RmP_System) goto done;
  purpose &= RmP_Mask;
  if (purpose != RmP_Helios) goto done;

  unless(state & RmS_Running) goto done;

  if (owner != RmO_FreePool)
   { if (owner != RmWhoAmI()) goto done;
     if (RmGetProcessorSession(processor) != RmGetSession()) goto done;
   }

	/* BLV - check processor type				*/

  ok = TRUE;

done:
  if (!ok)
   RmFreeProcessor(RmRemoveProcessor(processor));
  return(0);
}

static void check_world(void)
{ RmNetwork	network		= RmGetNetwork();
  RmNetwork	domain;

	/* 1) check that there is a network.				*/
  if (network == (RmNetwork) NULL)
   { fputs("farmtest: failed to get details of the current network.\n", stderr);
     exit(EXIT_FAILURE);
   }

	/* 2) check that the network has sufficient free and suitable	*/
	/*    processors to run this test harness. There is little	*/
	/*    point in trying to flood the network if there is only	*/
	/*    one processor.						*/
  (void) RmApplyProcessors(network, &network_filter);
  NumberProcessors = RmCountProcessors(network);
  if (NumberProcessors < 4)
   { fputs("farmtest: error, this network is too small to run the test harness sensibly.\n", stderr);
     exit(EXIT_FAILURE);
   }  

  RmFreeNetwork(network);

	/* 3) Check that there is a TFM associated with this session	*/
  domain	= RmGetDomain();
  if (domain == NULL)
   { fputs("farmtest: failed to get domain details from the TFM.\n", stderr);
     exit(EXIT_FAILURE);
   }
  RmFreeNetwork(domain);
}
/*}}}*/
int main(int argc, char **argv)
{ int	i;

  if (argc > 2) usage();
  if ((argc == 2) && strcmp(argv[1], "-m")) usage();

  check_world();

  if (argc == 2)
   menu();
  else
   { printf("Farm Library test program version %s\n", VersionNumber);
     for (i = 0; Tests[i] != NULL; i++)
      { run_test(i);
			/* sleep a bit to allow for cleaning	*/
	Delay((3 * OneSec) + (NumberProcessors * (OneSec / 6)));
      }
   }
  return(0);
}
/*}}}*/
