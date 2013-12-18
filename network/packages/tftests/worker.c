/*------------------------------------------------------------------------
--									--
--	   H E L I O S    T A S K F O R C E   T E S T S U I T E		--
--	   ----------------------------------------------------		--
--									--
-- worker.c								--
--		This is the worker program for the test suite.		--
--									--
-- Author: BLV, 16.4.92							--
------------------------------------------------------------------------*/
static char *rcsid = "$Header: /hsrc/network/packages/tftests/RCS/worker.c,v 1.1 1992/04/24 17:26:34 bart Exp $";

/*{{{  Header files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <posix.h>
#include <helios.h>
#include <nonansi.h>
/*}}}*/
/*{{{  forward declarations and statics */
/**
*** For all tests this worker program is given arguments of the form:
***   <component name> <test>_<test_args> <additional args>
*** The following statics hold details of these strings.
**/
static	char	 *ComponentName;
static	char	 *TestName;
static	char	 *TestArgs;
static	char	**ExtraArgv;
static	int	  ExtraArgc;

/**
*** For simplicity a single buffer is used for all communication. This
*** buffer is allocated statically and saves having to declare lots
*** of structures on various stacks. Also, to keep things simple all
*** communication goes via routines my_read() and my_write() which need
*** only a single argument, the file descriptor to use. The structure
*** being communicated includes an integer field, typically used to
*** hold the number of workers in a taskforce for self-sizing taskforces.
**/
#define	BufferSize	32
typedef struct	Communication {
	int	SomeNumber;
	BYTE	Data[BufferSize];
} Communication;

Communication	Buffer;

/**
*** To cope with pipes not connecting etc. a monitor thread is spawned off
*** from inside main(). This compares the time when the last communication
*** was initiated with the current time, and if there has been an excessive
*** delay an error is generated.
**/
static	int	LastCommunicationStart	= -1;
#define MaximumAcceptableDelay		60	/* in seconds */
static	bool	WriteOperation		= FALSE;
static	int	CurrentFd		= -1;
/*}}}*/
/*{{{  Communication utilities */
/**
*** All communication goes via routines my_read() and my_write() which
*** need only a single argument, the file descriptor to use. These routines
*** take care of errors e.g. EBADF if a channel has not been set up.
*** They also perform full reads and full writes, and take care of setting
*** up the timer for the monitor thread. All communication is duplicated
*** to ensure that there is real hand-shaking across the pipes.
**/
static	void	my_read1(int fd)
{ int	read_so_far	= 0;
  int	temp;

  LastCommunicationStart	= time(NULL);
  CurrentFd			= fd;
  WriteOperation		= FALSE;

  while(read_so_far < sizeof(Communication))
   { temp = read(fd, ((BYTE *) &Buffer) + read_so_far, sizeof(Communication) - read_so_far);
     if (temp < 0)
      { fprintf(stderr, "<%s> : communication error reading on %d, errno %d.\n",
		ComponentName, fd, errno);
	exit(EXIT_FAILURE);
      }
     read_so_far += temp;
   }

  LastCommunicationStart	= -1;
}

static	void	my_write1(int fd)
{ int	written_so_far	= 0;
  int	temp;

  LastCommunicationStart	= time(NULL);
  WriteOperation		= TRUE;
  CurrentFd			= fd;

  while(written_so_far < sizeof(Communication))
   { temp = write(fd, ((BYTE *) &Buffer) + written_so_far, sizeof(Communication) - written_so_far);
     if (temp < 0)
      { fprintf(stderr, "<%s> : communication error writing to %d, errno %d.\n",
		ComponentName, fd, errno);
	exit(EXIT_FAILURE);
      }
     written_so_far += temp;
   }

  LastCommunicationStart	= -1;
}

static void my_read(int fd)
{ my_read1(fd); my_read1(fd);
}

static void my_write(int fd)
{ my_write1(fd); my_write1(fd);
}

/**
*** Error reporting utilities.
**/
static void unknown_test(void)
{ fprintf(stderr, "<%s> : (%s)_(%s), test not recognised.\n", ComponentName,
	TestName, TestArgs);
  exit(EXIT_FAILURE);
}
/*}}}*/
/*{{{  Taskforce Topologies */
/*{{{  uni-directional pipeline */
/**
*** A uni-directional pipeline looks like this:
***    head -> worker0 -> worker1 ... -> tail
*** The head program writes to fd 1, the workers all read from 0 and write to
*** 1, and the tail only reads from 0.
**/
static	void	unipipe(void)
{
  if (!strcmp(TestArgs, "head"))
   my_write(1);
  elif (!strcmp(TestArgs, "worker"))
   { my_read(0);
     my_write(1);
   }
  elif (!strcmp(TestArgs, "tail"))
   my_read(0);
  else
   unknown_test();
}
/*}}}*/
/*{{{  bi-directional pipeline */
/**
*** A bi-directional pipeline looks like this:
***   head <=> worker <=> worker <=> ... <=> tail
***
*** First the head writes to 5, all the workers read from 0 and write to 5,
*** and the tail reads from 0. Then the flow is reversed: the tail writes
*** to 1, all the workers read from 4 and write to 1, and the head reads
*** from 4.
**/
static void bipipe(void)
{
  if (!strcmp(TestArgs, "head"))
   { my_write(5);
     my_read(4);
   }
  elif (!strcmp(TestArgs, "worker"))
   { my_read(0);
     my_write(5);
     my_read(4);
     my_write(1);
   }
  elif (!strcmp(TestArgs, "tail"))
   { my_read(0);
     my_write(1);
   }
  else
   unknown_test();
}
/*}}}*/
/*{{{  uni-directional ring */
/**
*** A uni-directional ring looks like this:
***   master -> worker -> worker
***    ^                    v
***   worker <- worker <- worker
***
*** The master writes to 5, all the workers read from 0 and write to 1, and
*** finally the master reads back from 4.
**/
static void uniring(void)
{
  if (!strcmp(TestArgs, "master"))
   { my_write(5);
     my_read(4);
   }
  elif (!strcmp(TestArgs, "worker"))
   { my_read(0);
     my_write(1);
   }
  else
   unknown_test();
}

/*}}}*/
/*{{{  bi-directional ring */
/**
*** A bi-directional ring looks like this:
***   master <=> worker <=> worker
***    /\                     /\ 
***    ||                     ||
***    \/                     \/ 
***   tail   <=> worker <=> worker
***
*** First data is transmitted clockwise : master 5 -> worker 0 -> worker 5 ...
*** tail 0 -> tail 5 -> master 6. Then the data goes anti-clockwise:
*** master 7 -> tail 4 -> worker 4 -> worker 1 ... -> master 4
**/
static void biring(void)
{
  if (!strcmp(TestArgs, "master"))
   { my_write(5);
     my_read(6);
     my_write(7);
     my_read(4);
   }
  elif (!strcmp(TestArgs, "worker"))
   { my_read(0);
     my_write(5);
     my_read(4);
     my_write(1);
   }
  elif (!strcmp(TestArgs, "tail"))
   { my_read(0);
     my_write(5);
     my_read(4);
     my_write(1);
   }
  else
   unknown_test();
}
/*}}}*/
/*{{{  ring with separate controller, fully connected */
/**
*** A ring with a separate controller which can interact with all the
*** workers directly can be drawn like this:
***    worker  ->   worker   -> worker
***      ^     \      |       /   |
***      |          master        |
***      |     /      |     \     v
***    worker <-    worker  <-  worker
***
*** Alternatively, it can be drawn like this:
***                      /---\
***            /==>  worker   |
***           //       ^      |
***          //        |      V
***   master<=====>  worker   |
***          \\        ^      |
***           \\       |      |
***            \==>  worker   |
***                    ^      |
***                     \____/
***
*** First the master communicates with all the workers: 4 & 5 for worker 0,
*** 6 & 7 for worker 1, and so on. The workers all read from 0 and write to 1.
*** This communication must include the worker number. Next data is
*** circulated around the ring: worker 0 writes to 5 and reads from 4; all
*** other workers read from 4 and write to 5.
**/
static void cring1(void)
{
  if (!strcmp(TestArgs, "master"))
   { int i, number_workers = atoi(ExtraArgv[0]);
     for (i = 0; i < number_workers; i++)
      { Buffer.SomeNumber	= i;
	my_write(5 + (2 * i));
	my_read(4 + (2 * i));
      }
   }
  elif (!strcmp(TestArgs, "worker"))
   { int	worker_number;
     my_read(0);
     worker_number = Buffer.SomeNumber;
     my_write(1);

     if (worker_number == 0)
      { my_write(5);
	my_read(4);
      }
     else
      { my_read(4);
	my_write(5);
      }
   }
  else
   unknown_test();
}
/*}}}*/
/*{{{  ring with separate controller, one connection */
/**
*** In this type of ring the controller component is connected to a
*** specific worker, rather than to all the workers.
***   master <=> worker <=> worker <=> worker
***                /\                    /\ 
***                ||                    ||
***                \/                    \/ 
***             workern <=> worker <=> worker
***
*** The first communication involves the master and the first worker:
*** master 5 -> worker0 0, worker0 1 -> master 4. Next data is transmitted
*** clockwise around the ring: worker0 5 -> worker 0 -> worker 5 ...
*** -> workern 0 -> workern 5 -> worker0 6. Then anti-clockwise:
*** worker0 7 -> workern 4 -> worker 4 -> worker 1 ... -> worker0 4
**/
static void cring2(void)
{
  if (!strcmp(TestArgs, "master"))
   { my_write(5);
     my_read(4);
   }
  elif (!strcmp(TestArgs, "worker0"))
   { my_read(0);
     my_write(1);
     my_write(5);
     my_read(6);
     my_write(7);
     my_read(4);
   }
  elif (!strcmp(TestArgs, "worker"))
   { my_read(0);
     my_write(5);
     my_read(4);
     my_write(1);
   }
  elif (!strcmp(TestArgs, "workern"))
   { my_read(0);
     my_write(5);
     my_read(4);
     my_write(1);
   }
  else
   unknown_test();
}

/*}}}*/
/*{{{  chordal ring, diagonally opposite */
/**
*** One type of chordal ring looks like this:
***           worker
***         /   |   \
***    worker   |   worker
***      |    \ | /   |
***      |    / | \   |
***    worker   |   worker
***         \   |   /
***           worker
***
*** The workers can determine their id using an argument. There is an initial
*** clock-wise communication to determine the number of workers:
***    worker0 7 -> worker 4 -> worker 7 ... -> worker0 4
*** Then anti-clockwise, informing all the workers about the size of the
*** taskforce.
***    worker0 5 -> worker 6 -> worker 5 ... -> worker0 6
*** Finally, across the diagonals. The first n/2 workers write to 9 and
*** read from 8, the other half read from 8 and write to 9.
**/
static void chord1(void)
{ int	worker_number	= atoi(ExtraArgv[0]);
  int	number_workers;

  if (worker_number == 0)
   { Buffer.SomeNumber	= 0;
     my_write(7);
     my_read(4);
     number_workers	= Buffer.SomeNumber + 1;
     Buffer.SomeNumber	= number_workers;
     my_write(5);
     my_read(6);
   }
  else
   { my_read(4);
     Buffer.SomeNumber	= worker_number;
     my_write(7);
     my_read(6);
     number_workers	= Buffer.SomeNumber;
     my_write(5);
   }

  if (worker_number < (number_workers / 2))
   { my_write(9);
     my_read(8);
   }
  else
   { my_read(8);
     my_write(9);
   }
}
/*}}}*/
/*{{{  chordal ring, I do not know how to describe this one */
/**
*** A CDL script for this particular topology was requested by a Canadian
*** university. I do not know what purpose it serves.
***        --->  worker  ->  worker  ->  worker
***       /         |           |           |    \
***   master        |           |           |     |
***       \         V           V           V     |
***        -<-   worker  <-  worker  <-  worker <-
***
*** First there is communication around the ring:
***   master 5 -> worker 0 -> worker 1 ... -> master 4
*** Next there is communication across the chords:
***   worker0 5 -> worker1 4
**/
static void chord2(void)
{
  if (!strcmp(TestArgs, "master"))
   { my_write(5);
     my_read(4);
   }
  elif (!strcmp(TestArgs, "worker0"))
   { my_read(0);
     my_write(1);
     my_write(5);
   }
  elif (!strcmp(TestArgs, "worker1"))
   { my_read(0);
     my_write(1);
     my_read(4);
   }
  else
   unknown_test();
}
/*}}}*/
/*{{{  farm without a load balancer */
/**
*** A farm without a load balancer is easy.
***			worker
***		      /
***		    /
***	     master*-- worker
***		    \
***		     \
***		       worker
***
*** The master communicates with all the workers: 4 and 5 for the first
*** worker, 6 and 7 for the second worker, and so on. All the workers
*** read from 0 and write to 1.
**/
static void farm1(void)
{ int	number_workers, i;

  if (!strcmp(TestArgs, "master"))
   { number_workers = atoi(ExtraArgv[0]);
     for (i = 0; i < number_workers; i++)
      { my_write(5 + (2 * i));
        my_read(4 + (2 * i));
      }
   }
  elif (!strcmp(TestArgs, "worker"))
   { my_read(0);
     my_write(1);
   }
  else
   unknown_test();
}
/*}}}*/
/*{{{  farm with load balancer */
/**
*** A farm with a load balancer is almost as easy.
***			worker
***		      /
***		    /
***    master - lb *-- worker
***		    \
***		     \
***		       worker
***
*** First the master interacts with the load balancer:
***   master 5 -> lb 0 -> lb 1 -> master 4
*** Then the load balancer interacts with all the workers: 4 and 5 for the
*** first worker, 6 and 7 for the second worker, and so on. All the workers
*** read from 0 and write to 1.
**/
static void farm2(void)
{ int	number_workers, i;

  if (!strcmp(TestArgs, "master"))
   { my_write(5);
     my_read(4);
   }
  elif (!strcmp(TestArgs, "lb"))
   { number_workers = atoi(ExtraArgv[0]);
     my_read(0);
     my_write(1);
     for (i = 0; i < number_workers; i++)
      { my_write(5 + (2 * i));
        my_read(4 + (2 * i));
      }
   }
  elif (!strcmp(TestArgs, "worker"))
   { my_read(0);
     my_write(1);
   }
  else
   unknown_test();
}
/*}}}*/
/*{{{  binary tree */
/**
*** A binary tree looks like this:
***                 root
***        -------/    \--------
***       /                     \         
***     node                   node
***    /    \                 /    \
***  leaf   leaf           leaf    leaf
***
*** Communication goes left->right. The root writes to its left child using
*** 5 and reads back from 4. Then the root root writes to the right child
*** using 7 and reads back from 6. All the nodes start by reading from
*** 0 for the message coming, directly or indirectly, from the root.
*** The nodes then interact with left and right children just like the root,
*** and write back to 1 to send data back, directly or indirectly, to the
*** root. Finally all the leafs read from 0 and write to 1.
**/
static void bintree(void)
{
  if (!strcmp(TestArgs, "root"))
   { my_write(5);
     my_read(4);
     my_write(7);
     my_read(6);
   }
  elif (!strcmp(TestArgs, "node"))
   { my_read(0);
     my_write(5);
     my_read(4);
     my_write(7);
     my_read(6);
     my_write(1);
   }
  elif (!strcmp(TestArgs, "leaf"))
   { my_read(0);
     my_write(1);
   }
  else
   unknown_test();
}
/*}}}*/
/*{{{  ternary tree */
/**
*** Ternary trees involve three brances per node rather than 2.
***			root
***		       / |  \
***        /----------/  |   \------------\
***      node           node              node
***     /  | \         / |  \            / |  \
***   lf  lf  lf     lf  lf  lf        lf  lf  lf
***
*** In communication terms this is almost identical to the binary tree
*** described above, the only difference being the numbers of descendants.
**/
static void terntree(void)
{
  if (!strcmp(TestArgs, "root"))
   { my_write(5);
     my_read(4);
     my_write(7);
     my_read(6);
     my_write(9);
     my_read(8);
   }
  elif (!strcmp(TestArgs, "node"))
   { my_read(0);
     my_write(5);
     my_read(4);
     my_write(7);
     my_read(6);
     my_write(9);
     my_read(8);
     my_write(1);
   }
  elif (!strcmp(TestArgs, "leaf"))
   { my_read(0);
     my_write(1);
   }
  else
   unknown_test();
}
/*}}}*/
/*{{{  two-dimensional toroidal grid */
/**
*** A two-dimensional grid looks like this:
***    worker00  <=>  worker01  <=>  worker02
***       /\             /\             /\ 
***       ||             ||             || 
***       \/             \/             \/ 
***    worker10  <=>  worker11  <=>  worker12
***
*** There are four phases: left -> right, right -> left, top -> bottom,
*** bottom -> top. In all cases the left-most or top-most workers
*** initiate the communication.
**/
static void twodgrid(void)
{ int	column	= atoi(ExtraArgv[1]);
  int	row	= atoi(ExtraArgv[0]);

  if (column == 0)
   { my_write(5);	/*    x -> */
     my_read(4);	/* -> x    */
     my_write(7);	/* <- x	   */ 
     my_read(6);	/*    x <- */
   }
  else
   { my_read(4);
     my_write(5);
     my_read(6);
     my_write(7);
   }

  if (row == 0)
   { my_write(9);
     my_read(8);
     my_write(11);
     my_read(10);
   }
  else
   { my_read(8);
     my_write(9);
     my_read(10);
     my_write(11);
   }
}
/*}}}*/
/*{{{  hypercubes */
/**
*** This test is unusual in that it involves parallel communication.
*** All the workers know their number and the size of the hypercube.
*** Then they all communicate via "link" 0: depending on the relative
*** worker numbers they write/read or read/write. Next they communicate
*** via "link" 1, etc.
**/
static void hypercube(void)
{ int	worker_number	= atoi(ExtraArgv[0]);
  int	number_workers	= atoi(ExtraArgv[1]);
  int	i;

  for (i = 0; (1 << i) < number_workers; i++)
   { int neighbour = worker_number ^ (1 << i);
     if (neighbour > worker_number)
      { my_read(4 + (2 * i));
        my_write(5 + (2 * i));
      }
     else
      { my_write(5 + (2 * i));
        my_read(4 + (2 * i));
      }
   }
}
/*}}}*/
/*{{{  fully connected */
/**
*** In a fully connected taskforce, the workers are handled in order.
*** worker 0 writes to and reads from all the other workers. Worker 1
*** writes to and reads from all higher workers. And so on.
**/
static void full(void)
{ int	worker_number	= atoi(ExtraArgv[0]);
  int	number_workers	= atoi(ExtraArgv[1]);
  int	i;

  for (i = 0; i < worker_number; i++)
   { my_read(4 + (2 * i));
     my_write(5 + (2 * i));
   }

  for (i = (worker_number + 1); i < number_workers; i++)
   { my_write(5 + (2 * i));
     my_read(4 + (2 * i));
   }
}
/*}}}*/
/*}}}*/
/*{{{  Inspecting the result of a clean-up */

/*}}}*/
/*{{{  Miscellaneous tests */
/*{{{  fully-mapped taskforce */

/*}}}*/
/*{{{  partially-mapped taskforce */

/*}}}*/
/*{{{  code specification and argv[0] */

/*}}}*/
/*{{{  argument processing */

/*}}}*/
/*{{{  inheritance of environment strings and standard streams */

/*}}}*/
/*{{{  termination */

/*}}}*/
/*{{{  signal handling */

/*}}}*/

/*{{{  Miscellaneous tests */

/*}}}*/

/*}}}*/
/*{{{  Test monitor thread */
/**
*** This thread runs continuously, attempting to detect deadlocks in the
*** communication etc. It wakes up every ten seconds and checks the
*** time that the last communication was started. If this exceeds some
*** limit then an error is generated and the program abort.
**/
static	void	monitor_thread(void)
{
  forever
   { Delay(10 * OneSec);
     if ((LastCommunicationStart != -1) && 
	 ((LastCommunicationStart + MaximumAcceptableDelay) < time(NULL)))
      { fprintf(stderr, "<%s> : test <%s>, communication timeout %sing on fd %d.\n",
		ComponentName, TestName, (WriteOperation) ? "writ" : "read",
		CurrentFd);
        exit(EXIT_FAILURE);
      }
   }
}
/*}}}*/
/*{{{  main() */
/**
*** main() is responsible for parsing the arguments and invoking the
*** appropriate handler routines. The first argument defines the test
*** currently being executed.
**/

typedef struct {
	char		*Name;
	VoidFnPtr	Handler;
} TestSpecification;

TestSpecification Tests[] = {
	{ "unipipe",	unipipe 	},
	{ "bipipe",	bipipe		},
	{ "uniring",	uniring		},
	{ "biring",	biring		},
	{ "cring1",	cring1		},
	{ "cring2",	cring2		},
	{ "chord1",	chord1		},
	{ "chord2",	chord2		},
	{ "farm1",	farm1		},
	{ "farm2",	farm2		},
	{ "bintree",	bintree		},
	{ "terntree",	terntree	},
	{ "2dgrid",	twodgrid	},
	{ "hypercube",	hypercube	},
	{ "full",	full		},
	{ NULL,		NULL		}
};

int main(int argc, char **argv)
{ char	*underscore;
  int	 i;

  unless(Fork(2000, &monitor_thread, 0))
   { fprintf(stderr, "<%s> : failed to spawn monitor thread.\n", argv[0]);
     exit(EXIT_FAILURE);
   }

  if (argc < 2)
   { fprintf(stderr, "<%s> : not enough arguments.\n", argv[0]);
     exit(EXIT_FAILURE);
   }

  ComponentName = argv[0];
  TestName	= argv[1];
  ExtraArgv	= &(argv[2]);
  ExtraArgc	= argc - 2;
  underscore	= strchr(argv[1], '_');
  if (underscore == NULL)
   { fprintf(stderr, "<%s> : argv[1] (%s) is not a test specification.\n",
		ComponentName, argv[1]);
     exit(EXIT_FAILURE);
   }
  *underscore	= '\0';
  TestArgs	= ++underscore;

  for (i = 0; Tests[i].Name != NULL; i++)
   if (!strcmp(TestName, Tests[i].Name))
    { (*Tests[i].Handler)();
      return(EXIT_SUCCESS);
    }

  fprintf(stderr, "<%s> : test (%s) not recognised.\n", ComponentName,
	TestName);
  return(EXIT_FAILURE);
}

/*}}}*/

