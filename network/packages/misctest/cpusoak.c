/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E		--
--           ---------------------------------------------------		--
--																		--
--							CPU Time Soaker								--
--							---------------								--
--                                                                      --
--             Copyright (C) 1993, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- cpusoak.c															--
--                                                                      --
--	Author:  BLV 29/1/93												--
--                                                                      --
------------------------------------------------------------------------*/
static char *rcsid = "$Header: /users/bart/hsrc/network/packages/hwtests1/RCS/hwtest.c,v 1.4 1993/01/15 18:20:00 bart Exp $";

/*{{{  version number and revision history */
#define VersionNumber	"1.00"

/*
** Revision history :
**	1.00	first version of the cpu soak program
*/
/*}}}*/
/*{{{  header files etc. */
#include <helios.h>
#include <sem.h>
#include <syslib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <nonansi.h>
#include <process.h>
#include <posix.h>
/*}}}*/
/*{{{  data structures, statics and manifests */
typedef unsigned int	uint;

		/* Command line arguments.					*/
static	uint	 Days			= -1;
static	uint	 Hours			= -1;
static	uint	 Minutes		= -1;
static	uint	 NumberThreads	= 1;
static	uint	 NumberTasks	= 0;
static	bool	 CdlMode		= FALSE;
static	bool	 IntOnly		= FALSE;
static	bool	 FloatOnly		= FALSE;
static	bool	 MemoryOnly		= FALSE;
static	bool	 SemaphoresOnly	= FALSE;
static	bool	 RandomTests	= FALSE;
static	bool	 DetachMode		= FALSE;
static	bool	 CleanExit		= FALSE;
static	int		 Priority		= 0;

	/* Stacksize for the various threads running.					*/
#define StackSize	3000

	/* To implement the clean exit facility a Finished flag is		*/
	/* used. This flag is set by main() and checked regularly by	*/
	/* the various compute threads. A thread count and a semaphore	*/
	/* are used in the usual manner.								*/
static	bool		Finished		= FALSE;
static	uint		ThreadsStarted	= 0;
static	Semaphore	ThreadsDone;

	/* Control access to the C library.								*/
static	Semaphore	Clib;

/*{{{  initialise statics */
/*
** This routine is called from inside main() to initialise
** the various semaphores etc. which cannot be done automatically.
*/

static void initialise_statics(void)
{
	InitSemaphore(&ThreadsDone, 0);
	InitSemaphore(&Clib, 1);
}
/*}}}*/
/*}}}*/
/*{{{  utilities */
/*
** Error reporting etc.
*/
#if 0
static void Report(char *str, ...)
{ va_list	args;

  Wait(&Clib);
  va_start(args, str);
  fputs("cpusoak: ", stderr);
  vfprintf(stderr, str, args);
  fputs("\n", stderr);
  va_end(args);
  Signal(&Clib);
}
#endif

static void Fatal(char *str, ...)
{ va_list	args;

  Wait(&Clib);
  va_start(args, str);
  fputs("cpusoak: ", stderr);
  vfprintf(stderr, str, args);
  fputs("\n", stderr);
  va_end(args);
  exit(EXIT_FAILURE);
}
/*}}}*/
/*{{{  integer arithmetic */
/*
** Integer arithmetic test. This is based on calculating the sum of
** pseudo-random numbers module 2 ^^ 30.
*/

static void integer_test(void)
{
	uint		seed;
	uint		sum	= 0;
	int			i;

	seed = (uint) &seed;
	if (Priority != 0)
		SetPriority(Priority);
	Delay(10 * OneSec);		/* to allow other threads and tasks to be started*/
		
	while (!Finished)
		for (i = 0; i < 1024; i++)
		{
			sum		= (sum + seed) & 0x3fffffff;
			seed	= (seed * 1103515245) + 12345;
		}

	Signal(&ThreadsDone);
}	
/*}}}*/
/*{{{  floating point arithmetic */
/*
** Floating point test. This code estimates a value for pi by
** integrating over 100 intervals.
*/

#define NumberIntervals		100

static void float_test(void)
{
	double		total	= 0.0;
	double		pi;
	double		width	= 1.0 / NumberIntervals;
	double		tmp;
	int			i;

	if (Priority != 0)
		SetPriority(Priority);
	Delay(10 * OneSec);		/* let other threads and tasks start up	*/
	
	while (!Finished)
	{
		pi		 = 0.0;
		for (i = 0; i < NumberIntervals; i++)
		{
			tmp	 = (0.5 + (double) i) * width;
			pi	+= width * (4.0 / (1.0 + tmp + tmp));
		}
		total	+= pi;
	}

	Signal(&ThreadsDone);
}

#undef NumberIntervals
/*}}}*/
/*{{{  memory allocation and deallocation */
/*
** Memory allocation test. This continually allocates 32 chunks of memory
** and then frees them again.
*/

#define NumberChunks	32

static	void	memory_test(void)
{
	char	*buffers[NumberChunks];
	int		 i;

	if (Priority != 0)
		SetPriority(Priority);
	Delay(10 * OneSec);		/* let other threads and tasks start up		*/

	while (!Finished)
	{
		for (i = 0; i < NumberChunks; i++)
			buffers[i]	= malloc(i);
		for (i = 0; i < NumberChunks; i++)
			if (buffers[i])
				free(buffers[i]);
	}

	Signal(&ThreadsDone);
}

#undef NumberChunks
/*}}}*/
/*{{{  semaphore stuff */
/*
** Semaphore test. This actually consists of two separate threads, one
** started by the other. The two threads continually Signal() each other
** and then Wait() for a signal. The result should be continuous scheduling
** activity.
*/

static void second_thread(Semaphore *sem1, Semaphore *sem2)
{
	if (Priority != 0)
		SetPriority(Priority);
	Delay(10 * OneSec);
	
	while (!Finished)
	{
		Wait(sem1);
		Signal(sem2);
	}

	Signal(&ThreadsDone);
}

static void semaphore_test(void)
{
	Semaphore	sem1;
	Semaphore	sem2;

	if (Priority != 0)
		SetPriority(Priority);

	InitSemaphore(&sem1, 0);
	InitSemaphore(&sem2, 0);

	unless (Fork(StackSize, &second_thread, 2 * sizeof(Semaphore *), &sem1, &sem2))
		Fatal("semaphore tester, failed to start second thread");

	Delay(10 * OneSec);
	
	while (!Finished)
	{
		Signal(&sem1);
		Wait(&sem2);
	}

	Signal(&sem1);

	Signal(&ThreadsDone);
}
/*}}}*/
/*{{{  start threads and support clean exit */
/*
** This code is responsible for starting up the specified number of threads
** using whatever tests have been specified by the user.
*/

static void	start_threads(void)
{
	int		i;

	srand(time(NULL));
	
	for (i = 0; i < NumberThreads; i++)
	{
		VoidFnPtr	test_fn;

		if (IntOnly)
			test_fn	= &integer_test;
		elif (FloatOnly)
			test_fn = &float_test;
		elif (MemoryOnly)
			test_fn = &memory_test;
		elif (SemaphoresOnly)
			test_fn = &semaphore_test;
		else
		{		/* Select a random test for every thread			*/
			int	x	= rand() % 4;

			switch (x)
			{
			case	0 : test_fn = &integer_test; break;
			case	1 : test_fn = &float_test;	 break;
			case	2 : test_fn	= &memory_test;	 break;
			default	  : test_fn	= &semaphore_test;
			}
		}

		unless (Fork(StackSize, test_fn, 0))
			Fatal("out of memory starting up the various threads.");

		if (test_fn == &semaphore_test)
			ThreadsStarted	+= 2;
		else
			ThreadsStarted++;

	}	/* for the number of threads	*/
}

/*
** Waiting for the various threads to halt involves setting the Finished
** flag and doing the appropriate number of semaphore waits.
*/
static void waitfor_threads(void)
{
	Finished	= TRUE;

	while (ThreadsStarted--)
		Wait(&ThreadsDone);
}
/*}}}*/
/*{{{  start tasks */
/*
** Starting up tasks is done via vfork()/execve() calls. The -cdl option
** is handled by calls to _posixflags(). The tricky bit is constructing
** the argument vector. All the task id's are held in a table.
*/
static	int		*task_table	= NULL;

static void start_tasks(void)
{
	char		*argv[16];
	int			 arg_count;
	char		 threads_buf[16];
	char		 pri_buf[16];
	char		 days_buf[16];
	char		 hours_buf[16];
	char		 minutes_buf[16];
	char		*code_name;
	int			 i;

	task_table	= malloc(NumberTasks * sizeof(int));
	if (task_table == NULL)
		Fatal("failed to allocate table to hold all the tasks");
		
	arg_count			= 0;
	argv[arg_count++]	= "cpusoak";
	if (NumberThreads > 1)
	{
		sprintf(threads_buf, "-t%d", NumberThreads);
		argv[arg_count++]	= threads_buf;
	}
	if (FloatOnly)
		argv[arg_count++]	= "-f";
	elif (MemoryOnly)
		argv[arg_count++]	= "-m";
	elif (SemaphoresOnly)
		argv[arg_count++]	= "-s";
	elif (RandomTests)
		argv[arg_count++]	= "-r";
	if (CleanExit)
		argv[arg_count++]	= "-x";
	if (Priority != 0)
	{
		sprintf(pri_buf, "-p%d", Priority);
		argv[arg_count++]	= pri_buf;
	}
	if (Days > 0)
	{
		sprintf(days_buf, "%d", Days);
		argv[arg_count++] = days_buf;
	}
	if ((Days > 0) || (Hours > 0))
	{
		sprintf(hours_buf, "%d", Hours);
		argv[arg_count++]	= hours_buf;
	}
	if ((Days > 0) || (Hours > 0) || (Minutes > 0))
	{
		sprintf(minutes_buf, "%d", Minutes);
		argv[arg_count++]	= minutes_buf;
	}
	else
			/* If no duration is specified then the workers have to	*/
			/* run in detach mode and will be signalled explicitly.	*/
		argv[arg_count++]	= "-d";
		
	argv[arg_count]	= NULL;
	
		/* Extract the code name (use the /loader entry)		*/
	code_name	= getenviron()->Objv[OV_Code]->Name;

		/* Decide on the CDL mode								*/
	if (CdlMode)
		_posixflags(PE_BLOCK, PE_RemExecute);
	else
		_posixflags(PE_UNBLOCK, PE_RemExecute);

		/* Now start up the actual tasks.						*/
	for (i = 1; i < NumberTasks; i++)
	{
		task_table[i-1] = vfork();
		if (task_table[i-1] == 0)
			execv(code_name, argv);
	}
}

static void waitfor_tasks(void)
{
	int		rc;
	int		i;

		/* Unless a duration was specified the workers must be terminated	*/
		/* with a signal.													*/
	unless ((Days > 0) || (Hours > 0) || (Minutes > 0))
		for (i = 0; i < NumberTasks; i++)
			kill(task_table[i], SIGINT);
			
		/* BLV - this should detect and cope with signals aborting the wait	*/
	while (NumberTasks--)
		(void) waitpid(task_table[i], &rc, 0);
}
/*}}}*/
/*{{{  main */
/*
** main() is essentially one big parser. The main work is done elsewhere
*/
/*{{{  usage() */
static void usage(void)
{
	fputs("cpusoak: usage, cpusoak [-t no] [-T no] [-cdl] [-i] [-f] [-m] [-s] [-r] [-d] [-x] [-p pri] [time]\n", stderr);
	fputs("       : -t no      use <no> threads per task, default 1\n", stderr);
	fputs("       : -T no      use <no> tasks, default 1\n", stderr);
	fputs("       : -cdl       distribute tasks over the network\n", stderr);
	fputs("       : -i         perform integer tests only (default)\n", stderr);
	fputs("       : -f         perform floating point tests only\n", stderr);
	fputs("       : -m         perform memory allocation tests only\n", stderr);
	fputs("       : -s         use semaphores to switch between threads\n", stderr);
	fputs("       : -r         select from the above tests at random\n", stderr);
    fputs("       : -d         detach mode, run forever\n", stderr);
    fputs("       : -x         exit cleanly, do not rely on system tidy up\n", stderr);
    fputs("       : -p pri     run at the specified priority\n", stderr);
    fputs("       :            -8192 == ServerPri, 0 == StandardPri, 8192 == BackgroundPri\n", stderr);
	fputs("       : [time]     run-time in days, hours and minutes\n\n", stderr);
	fputs("   e.g.: cpusoak -t 20 -T 5 -cdl -p 8192\n", stderr);

	exit(EXIT_FAILURE);
}
/*}}}*/

int main(int argc, char **argv)
{
	int		i;

	initialise_statics();

	for (i = 1; i < argc; i++)
		if (argv[i][0] == '-')
			switch(argv[i][1])
		  	{
  			case 't' :
				if (argv[i][2] != '\0')		/* -t5 */
					NumberThreads = atoi(&(argv[i][2]));
  				else						/* -t 5	*/
  				{
  					if (++i == argc) usage();
  					NumberThreads = atoi(argv[i]);
	  			}
	
  				if (NumberThreads == 0) usage();
  				break;

	  		case 'T' :
	  			if (argv[i][2] != '\0')		/* -T5	*/
	  				NumberTasks = atoi(&(argv[i][2]));
	  			else
	  			{							/* -T 5	*/
	  				if (++i == argc) usage();
	  				NumberTasks = atoi(argv[i]);
				}

				if (NumberTasks == 0) usage();
				break;

			case 'c'	:
				CdlMode = TRUE;
				break;

			case 'i' :
				if (FloatOnly || MemoryOnly || SemaphoresOnly || RandomTests)
					usage();
				IntOnly	= TRUE;
				break;

			case 'f' :
				if (IntOnly || MemoryOnly || SemaphoresOnly || RandomTests)
					usage();
				FloatOnly = TRUE;
				break;

			case 'm' :
				if (IntOnly || FloatOnly || SemaphoresOnly || RandomTests)
					usage();
				MemoryOnly = TRUE;
				break;

			case 's' :
				if (IntOnly || FloatOnly || MemoryOnly || RandomTests)
					usage();
				SemaphoresOnly = TRUE;
				break;

			case 'r' :
				if (IntOnly || FloatOnly || MemoryOnly || SemaphoresOnly)
					usage();
				RandomTests = TRUE;
				break;

			case 'd' :
				DetachMode	= TRUE;
				break;

			case 'x' :
				CleanExit	= TRUE;
				break;

			case 'p' :
				{
					char	*tmp;
					
					if ((argv[i][2] != '\0') &&		/* -p8192	*/
						 isdigit(argv[i][2]))
						tmp = &(argv[i][2]);
					else
					{							/* -p 8192	*/
						if (++i == argc) usage();
						tmp = argv[i];
					}

					unless(isdigit(*tmp)) usage();
					Priority = atoi(tmp);
					break;
				}

			default:
				usage();
			}
		else	/* not a flag -xxx, should be a time field	*/
		{
			if	 (Days == -1)		Days	= atoi(argv[i]);
			elif (Hours == -1)		Hours	= atoi(argv[i]);
			elif (Minutes == -1)	Minutes	= atoi(argv[i]);
			else usage();
		}

		/* All argument parsed, now sort out the duration if specified	*/
	while (Minutes == -1) { Minutes = Hours; Hours = Days; Days = 0; }

		/* If a duration is specified then the Detach option cannot be used	*/
	if (((Minutes > 0) || (Hours > 0) || (Days > 0)) && DetachMode)
	{
		fputs("cpusoak: cannot combine -d option with a duration.\n", stderr);
		usage();
	}
	
		/* If no test is specified default to integer-only				*/
	unless (IntOnly || FloatOnly || MemoryOnly || SemaphoresOnly || RandomTests)
		IntOnly	= TRUE;
		
		/* Start up all the tasks if appropriate						*/
	if (NumberTasks > 1) start_tasks();

		/* Start up all the threads, there will always be one.			*/
	start_threads();

		/* Now wait for the termination condition						*/
	if (DetachMode)
		forever		/* This program should not terminate.				*/
			Delay(60 * OneSec);
	elif ((Minutes > 0)	|| (Hours > 0) || (Days > 0))
	{
					/* Run for the specified amount of time.			*/
		uint minutes = Minutes + 60 * (Hours + (24 * Days));
		while (minutes--)
			Delay(60 * OneSec);
	}
	else
	{
					/* Prompt the user and wait for a key.				*/
		puts("cpusoak running - hit return to exit.");
		fflush(stdout);
		(void) getchar();
	}

		/* The program should now exit. The correct behaviour depends	*/
		/* on the CleanExit flag.										*/
	if (CleanExit)
		waitfor_threads();

	if (NumberTasks > 1)
		waitfor_tasks();

	return(EXIT_SUCCESS);
}
/*}}}*/


