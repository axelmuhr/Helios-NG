/*------------------------------------------------------------------------
--									--
--	   H E L I O S    T A S K F O R C E   T E S T S U I T E		--
--	   ----------------------------------------------------		--
--									--
-- tftests.c								--
--		This is the controller program for the test suite.	--
--									--
-- Author: BLV, 16.4.92							--
------------------------------------------------------------------------*/
static char *rcsid = "$Header: /hsrc/network/packages/tftests/RCS/tftests.c,v 1.1 1992/04/24 17:26:34 bart Exp $";

/*{{{  Revision number and history */
static	char	*VersionNumber	= "1.00";
/**
*** Revision history:
***
*** 1.00 : first version, 16.4.92
**/
/*}}}*/
/*{{{  Header files */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <posix.h>
#include <sys/wait.h>
#include <unistd.h>
#include <rmlib.h>
#include <helios.h>
#include <syslib.h>
#include <gsp.h>
#include <codes.h>
#include <root.h>
#include <servlib.h>
/*}}}*/
/*{{{  forward declarations and statics */
	/* Has the -v option been specified ?				*/
static	bool	Verbose		= FALSE;

	/* Number of processors to use for running components.		*/
	/* For Tiny Helios this is set to 4.				*/
	/* Otherwise it is the number of processors that could be	*/
	/* obtained when the program started up.			*/
static	int	NumberProcessors	=	-1;

	/* The network of processors that can be used.			*/
static	RmNetwork	Network		= (RmNetwork)	NULL;

	/* The interval between successive runs of a taskforce.		*/
static	int		RunInterval	= 5 * OneSec;

	/* The name of the current test, used for diagnostics and	*/
	/* when reporting a failure.					*/
static	char		*CurrentTest	= "<Startup>";
/*}}}*/
/*{{{  Cleaning-up, deleting temporary files etc. */
/**
*** The test suite creates several temporary files during its run. These
*** should be deleted at the start and end of the run to avoid confusion.
*** a) ring - this is a compiled CDL binary used to test the signal
***    handling when not running via the CDL compiler.
*** b) environ - this holds environment information when testing the
***    environment inheritance
**/
static void clean_up(void)
{
  if (((remove("ring") != 0) && (errno != ENOENT)) ||
      ((remove("environ") != 0) && (errno != ENOENT)))
   { fputs("tftests: failed to delete temporary files, aborting.\n", stderr);
     exit(EXIT_FAILURE);
   }
}
/*}}}*/
/*{{{  Initialising the domain, pre-allocating processors */
/**
*** The first step in initialising the network is to check that there is
*** a Taskforce Manager associated with this program. The Resource Management
*** library routine RmGetDomain() provides a simple way of doing this.
***
*** Next it is desirable to obtain as many processors as possible. There
*** are three ways of doing this:
*** a) constructing a template for an arbitrary large number of processors,
***    attempt to obtain this template, and set all the obtained processors
***    to permanent to stop the TFM releasing them back to the free pool.
*** b) examining the current network and obtaining all free processors.
***    However, this relies on interacting with the Network Server to
***    determine the current network. In a Tiny Helios system this is not
***    possible.
*** c) running the domain command, e.g. domain get 400. Effectively this
***    does the same as (a) above, but with less code required here.
***    The problem with this is running out of memory to run the domain
***    command or in the TFM or Network Server.
***
*** The Taskforce Manager should now own all the free processors in the
*** network. Another call to RmGetDomain() can be used to get a description
*** of these processors, and it is possible to count them. If there
*** is only one processor in the domain then the system is assumed to
*** run Tiny Helios, and the test suite defaults to four components.
*** If this program is running on the same processor as the TFM and this
*** is a one-megabyte processors then this processor should not be used.
*** If there are more than one but less than four processors
*** in the domain then the test suite will default to four components.
*** Anything less is not really a sensible test of the TFM or the CDL
*** compiler.
**/
/*{{{  initialise_network_aux1() */
	/* Remove inaccessible processors from the global network	*/
	/* before attempting to obtain the network.			*/
static int initialise_network_aux1(RmProcessor processor, ...)
{ 
  if ((RmGetProcessorPurpose(processor) != (RmP_Helios + RmP_User)) ||
      ((RmGetProcessorOwner(processor) != RmO_FreePool) && (RmGetProcessorSession(processor) != RmGetSession())))
   { RmRemoveProcessor(processor);
     RmFreeProcessor(processor);
   }
  return(0);
}
/*}}}*/
/*{{{  initialise_network_aux2() */
	/* Make a processor allocation permanent, to keep it in the 	*/
	/* current domain.						*/
static int initialise_network_aux2(RmProcessor processor, ...)
{
  unless(RmIsProcessorPermanent(processor))
   RmSetProcessorPermanent(processor);
  return(0);
}
/*}}}*/
/*{{{  initialise_network_aux3() */
	/* If the current processor is running the TFM and does not	*/
	/* have a lot of memory, it should be booked at this point.	*/
	/* This stops the CDL compiler being run on this processor,	*/
	/* and hence reduces the probability of failure due to lack	*/
	/* of memory.							*/
static int initialise_network_aux3(RmProcessor processor, ...)
{ va_list	args;
  char		*procname;

  va_start(args, processor);
  procname = va_arg(args, char *);
  va_end(args);

  if (!strcmp(procname, RmGetProcessorId(processor)))
   { RmProcessor	obtained = RmObtainProcessor(processor);
     if (obtained != (RmProcessor) NULL)
      { RmSetProcessorBooked(processor);
	RmReleaseProcessor(processor);
	RmFreeProcessor(processor);
      }
     return(1);
   }
  return(0);
}
/*}}}*/

static	void initialise_network(void)
{ RmNetwork	global;
  RmNetwork	obtained;
  RmNetwork	domain;
  Object	*tfm;
  bool		small_processor	= FALSE;

  if (Verbose) puts("Initialising the domain");

  domain	= RmGetDomain();
  if (domain == (RmNetwork) NULL)
   { fputs("tftests: failed to access a Taskforce Manager for this session.\n", stderr);
     exit(EXIT_FAILURE);
   }

  tfm = Locate(Null(Object), "/loader/tfm");
  if (tfm != Null(Object))
   { Object	*tasks	= Locate(Null(Object), "/tasks");
     BYTE	 data[IOCDataMax];
     ProcStats	*stats = (ProcStats *) data;
     if (tasks != Null(Object))
      if (ServerInfo(tasks, data) >= Err_Null)
       if (stats->MemMax <= (1024 * 1024))
        { _posixflags(PE_BLOCK, PE_RemExecute);	/* equivalent to SET CDL */
	  small_processor = TRUE;
	  MachineName(data);
	  (void) RmSearchProcessors(domain, &initialise_network_aux3, objname(data));
        }
     if (tasks != Null(Object)) Close(tasks);
     Close(tfm);
   }

  RmFreeNetwork(domain);

  global	= RmGetNetwork();
  if (global != (RmNetwork) NULL)
   { (void) RmApplyProcessors(global, &initialise_network_aux1);
     obtained = RmObtainNetwork(global, FALSE, NULL);
     if (obtained != (RmNetwork) NULL)
      { (void) RmApplyProcessors(obtained, &initialise_network_aux2);
        RmReleaseNetwork(obtained);
        RmFreeNetwork(obtained);
      }
     RmFreeNetwork(global);
   }

  Network		= RmGetDomain();
  NumberProcessors	= RmCountProcessors(Network);
  if (small_processor)
   if (--NumberProcessors < 0)
    { fprintf(stderr, "tftests: network too small to run this test suite.\n");
      exit(EXIT_FAILURE);
    }

  if (Verbose) printf("Running test suite on %d processors.\n", NumberProcessors);
  if (NumberProcessors < 4) NumberProcessors = 4;
  if (Verbose) printf("Using a maximum of %d components.\n", NumberProcessors);
}
/*}}}*/
/*{{{  Emptying the domain */
/**
*** After the initial run of taskforces it is desirable to empty the domain,
*** forcing the TFM to obtain processors from the free pool and to
*** release processors back to the free pool at every stage. Similarly
*** it forces the Network Server to clean out processors at every stage,
*** hence resident libraries have to be re-loaded etc.
**/
/*{{{  empty_domain_aux1() */
	/* Clear the permanent flag on all processors in the current domain */
static int empty_domain_aux1(RmProcessor processor, ...)
{ if (RmIsProcessorPermanent(processor))
   RmSetProcessorTemporary(processor);
  return(0);
}
/*}}}*/

static void empty_domain(void)
{ RmNetwork	domain		= RmGetDomain();
  RmNetwork	obtained 	= RmObtainNetwork(domain, FALSE, NULL);

  (void) RmApplyProcessors(obtained, &empty_domain_aux1);
  RmReleaseNetwork(obtained);
  RmFreeNetwork(obtained);
  RmFreeNetwork(domain);
}
/*}}}*/
/*{{{  Taskforce Topologies */
/*{{{  start up the CDL compiler */
/**
*** Most taskforces are executed by starting the CDL compiler and
*** piping a CDL script into it. When the whole script has been written
*** out the pipe is closed, the CDL compiler does its stuff and executes
*** the taskforce, and the call to pclose() will return when the
*** taskforce has finished and the CDL compiler has exited.
***
*** Failure to start a CDL compiler is a serious error for now.
**/
static FILE *start_cdl(void)
{ FILE	*result = popen("/helios/bin/cdl", "w");

  if (Verbose) printf("Running test \"%s\".\n", CurrentTest);

  if (result == NULL)
   { fprintf(stderr, "tftests: \"%s\", failed to start CDL compiler.\n", CurrentTest);
     exit(EXIT_FAILURE);
   }
  return(result);
}

static void run_taskforce(FILE *x)
{ int status;

  if (Verbose) printf("Waiting for test \"%s\" to terminate.\n", CurrentTest);
  status = pclose(x);
  if (Verbose) printf("Test \"%s\" has terminated with %d.\n", CurrentTest, status);
  if (status != 0)
   { fprintf(stderr, "tftests: %s, CDL compiler returned an error %x.\n", CurrentTest, status);
     exit(EXIT_FAILURE);
   }
}
/*}}}*/

/*{{{  uni-directional pipeline */
/**
*** A uni-directional pipeline looks like this:
***    head -> worker0 -> worker1 ... -> tail
*** The CDL script for this is:
***   head [n] | worker | tail
**/
static void uni_pipeline(void)
{ FILE	*cdl	= start_cdl();

  fprintf(cdl,
    "worker unipipe_head [%d] | worker unipipe_worker | worker unipipe_tail\n",
		NumberProcessors - 2);

  run_taskforce(cdl);  
}
/*}}}*/
/*{{{  bi-directional pipeline */
/**
*** A bi-directional pipeline looks like this:
***   head <=> worker <=> worker <=> ... <=> tail
*** The CDL script is:
***   head [n] <> worker <> tail
**/
static void bi_pipeline(void)
{ FILE	*cdl	= start_cdl();

  fprintf(cdl,
   "worker bipipe_head [%d] <> worker bipipe_worker <> worker bipipe_tail\n",
    NumberProcessors -3);

  run_taskforce(cdl);
}
/*}}}*/
/*{{{  uni-directional ring */
/**
*** A uni-directional ring looks like this:
***   master -> worker -> worker
***    ^                    v
***   worker <- worker <- worker
***
*** The CDL script is:
***   master <> (| [n] worker)
**/
static void uni_ring(void)
{ FILE	*cdl	= start_cdl();

  fprintf(cdl,
   "worker uniring_master <> (| [%d] worker uniring_worker)\n",
   NumberProcessors - 1);

  run_taskforce(cdl);
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
*** The CDL script for this is as follows:
***  component master { code worker; streams ,,,,,, <|pipe1, >|pipe2; }
***  component tail   { code worker; streams ,,,,<| pipe2, >| pipe1; }
***
***  master [n] <> worker <> tail
***
*** i.e. the ring is implemented as a bidirectional pipeline with the
*** end of the ring handled by explicit streams.
**/
static void bi_ring(void)
{ FILE	*cdl	= start_cdl();
  fprintf(cdl,
   "component master { code worker; streams ,,,,,, <|pipe1, >| pipe2; }\n");
  fprintf(cdl,
   "component tail   { code worker; streams ,,,,<| pipe2, >| pipe1; }\n\n");
  fprintf(cdl,
   "master biring_master [%d] <> worker biring_worker <> tail biring_tail\n",
    NumberProcessors - 2);
  run_taskforce(cdl);
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
*** The simplest CDL script for this treats it as a non-loadbalanced farm,
*** with stream declarations used for the workers.
***   component worker[i] { code worker;
***                         streams ,,,, <| pipe{i}, >| pipe{(i+1) % n; }
***   component master    { code worker; }
***
***   master n (, [i < n] <> worker{i} )
**/
static void controller_ring1(void)
{ FILE	*cdl = start_cdl();

  fprintf(cdl, "component worker[i] { code worker; \n");
  fprintf(cdl, "     streams ,,,, <| pipe{i}, >| pipe{(i+1) %% %d}; }\n",
	NumberProcessors - 1);
  fprintf(cdl, "component master { code worker; }\n\n");
  fprintf(cdl, 
   "master cring1_master %d (, [i < %d] <> worker{i} cring1_worker)\n",
	NumberProcessors - 1, NumberProcessors - 1);

  run_taskforce(cdl);
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
***              worker <=> worker <=> worker
***
*** The CDL script for this again uses the streams to make the final ring
*** connection. Note that the stream allocation for worker0 is irregular:
*** 6 and 7 are used to communicate with the previous worker rather than
*** 0 and 1, because those are already used for the master.
***
***   component worker0 { code worker; streams ,,,,,,<| pipe1, >| pipe2; }
***   component workern { code worker; streams ,,,, <| pipe2, >| pipe1; }
***
***   worker <> worker0 [n] <> worker <> workern
**/
static void controller_ring2(void)
{ FILE	*cdl;

  cdl = start_cdl();

  fprintf(cdl,
    "component worker0 { code worker; streams ,,,,,, <| pipe1, >| pipe2; }\n");
  fprintf(cdl,
    "component workern { code worker; streams ,,,, <| pipe2, >| pipe1; }\n\n");

  fprintf(cdl, "worker cring2_master <> worker0 cring2_worker0 ");
  fprintf(cdl, "[%d] <> worker cring2_worker <> workern cring2_workern\n",
		NumberProcessors - 3);

  run_taskforce(cdl);
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
*** In other words, all the workers are connected in a ring, plus every
*** worker can interact with the one exactly opposite. Unfortunately
*** the CDL compiler cannot do division at present. Hence the size
*** argument specified should be half the ring size.
***
***   component worker[i] { code worker;
***      streams { ,,,,
***		<| clockwise{i}, >| antiwise{i},
***		<| antiwise{(i + 1) % (2 * n)}, >| clockwise{(i + 1) % (2 * n)},
***		<| across{i}, >| across{(i + n) % (2 * n)} ; }
***
***   ^^ [i < (2 * n)] worker{i}
**/
static void chordal_ring1(void)
{ FILE	*cdl	= start_cdl();
  int	magic	= NumberProcessors / 2;

  fprintf(cdl, "component worker[i] { code worker;\n");
  fprintf(cdl, "	streams ,,,,\n");
  fprintf(cdl, "	<| clockwise{i}, >| antiwise{i},\n");
  fprintf(cdl, "	<| antiwise{(i + 1) %% (2 * %d)}, >| clockwise{(i + 1) %% (2 * %d)},\n", magic, magic);
  fprintf(cdl, "	<| across{i}, >| across{(i + %d) %% (2 * %d)} ; }\n\n", magic, magic);
/* BLV - CDL cannot cope with expressions in a replicator at present */
/*  fprintf(cdl, "^^ [i < 2 * %d] worker{i} chord1_ %%i\n", magic);*/
  fprintf(cdl, "^^ [i < %d] worker{i} chord1_ %%i\n", 2 * magic);

  run_taskforce(cdl);
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
*** The CDL script for this is :
***   component worker0[i] { code worker; streams ,,,,, >| pipe{i}; }
***   component worker1[j] { code worker; streams ,,,,<| pipe{n - (j + 1)}; }
***
***   master <> ( (| [i < n] worker0{i}) | (| [j < n] worker1{j}) )
***
*** Again the size argument passed should be half the real size.
**/
static void chordal_ring2(void)
{ FILE	*cdl	= start_cdl();
  int	magic	= (NumberProcessors - 1) / 2;	/* allow for master */

  fprintf(cdl, "component worker0[i] { code worker; streams ,,,,, >| pipe{i} ; }\n");
  fprintf(cdl, "component worker1[j] { code worker; streams ,,,, <| pipe{%d - (j + 1)}; }\n\n", magic);
  fprintf(cdl, "component master { code worker; }\n");
  fprintf(cdl, "master chord2_master <> ( ");
  fprintf(cdl, "(| [i < %d] worker0{i} chord2_worker0 %%i) | ", magic);
  fprintf(cdl, "(| [j < %d] worker1{j} chord2_worker1 %%j) )\n", magic);

  run_taskforce(cdl);
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
*** The CDL script is:
***  master n (, [n] <> worker)
**/
static void farm1(void)
{ FILE	*cdl	= start_cdl();

  fprintf(cdl, "component master { code worker; }\n");
  fprintf(cdl, "master farm1_master %d (, [%d] <> worker farm1_worker)\n", 
	NumberProcessors - 1, NumberProcessors - 1);

  run_taskforce(cdl);
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
*** CDL has two syntaxes for this:
***   master [n] ||| worker
***   master <> lb n (, [n] <> worker)
***
*** The second syntax is used because it allows me to overwrite the code
*** for the lb component, hence I do not need to worry about interacting
*** with a program that is not part of this testsuite.
**/
static void farm2(void)
{ FILE	*cdl	= start_cdl();

  fprintf(cdl, "component master { code worker; }\n");
  fprintf(cdl, "component lb	 { code worker; }\n\n");
  fprintf(cdl, "master farm2_master <> lb farm2_lb %d (, [%d] <> worker farm2_worker)\n",
		NumberProcessors - 2, NumberProcessors - 2);

  run_taskforce(cdl);
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
*** For my purposes the size of a binary tree is defined in terms of
*** the number of lead nodes. A CDL script for this topology is as follows:
***   component root    { streams ,,,,, >| down{0}, , >| down{1}; }
***   component node[i] { streams <| down{i},,,, >| down{(2 * i) + 2)},, >| down{(2 * i) + 3}; }
***   component leaf[j] { streams <| down{j + (n - 2)},,,; }
***
***   root ^^ (^^ [i < (n - 2)] node{i}) ^^ (^^ [j < n] leaf{j})
***
*** The size of the tree has to be calculated carefully. Streams back can be
*** added trivially if desired - add a matching up to every down pipe, using
*** exactly the same maths.
**/
static void binary_tree(void)
{ int	tree_size;
  FILE	*cdl	= start_cdl();

  for (tree_size = 1; ((4 * tree_size) - 1) <= NumberProcessors; tree_size *= 2);

  fprintf(cdl, "component root    { code worker; streams ,,,,<|up{0},>|down{0},<| up{1},>|down{1}; }\n");
  fprintf(cdl, "component node[i] { code worker; streams <| down{i},>|up{i},,,<| up{(2 * i) + 2},>| down{(2 * i) + 2}, <| up{(2 * i) + 3}, >| down{(2 * i) + 3}; }\n");
  fprintf(cdl, "component leaf[j] { code worker; streams <| down{j + (%d - 2)},>| up{j + (%d - 2)},,; }\n\n", tree_size, tree_size);
/* BLV - CDL cannot cope with expressions in replicators at present */
/*  fprintf(cdl, "root bintree_root [i < (%d - 2)] ^^ node{i} bintree_node [j < %d] ^^ leaf{j} bintree_leaf\n",
	tree_size, tree_size);*/
  fprintf(cdl, "root bintree_root ^^ ");
  if ((tree_size - 2) > 0)
   fprintf(cdl, "(^^ [i < %d] node{i} bintree_node) ^^ ", tree_size - 2);
  fprintf(cdl, "(^^ [j < %d] leaf{j} bintree_leaf)\n", tree_size);

  run_taskforce(cdl);
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
*** There is a problem at the leaf nodes: the number of higher-level nodes,
*** and hence the pipe names, is related to half the number of leaf nodes
*** rather than to the actual number of leaf nodes. In the absence of
*** division I insist on two arguments, the number of leaf nodes and
*** the number of higher-level nodes.
***
*** A suitable CDL script would be:
***  component root    { ,,,, <| up{0}, >| down{0}, <| up{1}, >| down{1}, <| up{2}, >| down{2}; }
***  component node[i] { <| down{i}, >| up{i},,,
***			 <| up{(3 * i) + 3}, >| down{(3 * i) + 3},
***			 <| up{(3 * i) + 4}, >| down{(3 * i) + 4},
***			 <| up{(3 * i) + 5}, >| down{(3 * i) + 5} ; }
*** component leaf[j]  { <| down{j + x}, >| up{j + x}; }
***
*** root ^^ (^^ [i < x] node{i}) ^^ (^^ [j < n] leaf{j})
***
*** where x == (j / 2) - 1
**/
static void ternary_tree(void)
{ FILE	*cdl	= start_cdl();
  int	number_leafs;

  for (number_leafs = 1;
       ((3 * number_leafs) + number_leafs + (number_leafs / 2)) <= NumberProcessors;
       number_leafs *= 3);

  fprintf(cdl, "component root    { code worker; streams ,,,,<|up{0},>|down{0},<|up{1},>|down{1},<|up{2},>|down{2}; }\n");
  fprintf(cdl, "component node[i] { code worker; streams <|down{i},>|up{i},,,\n");
  fprintf(cdl, "		<| up{(3 * i) + 3}, >| down{(3 * i) + 3},\n");
  fprintf(cdl, "		<| up{(3 * i) + 4}, >| down{(3 * i) + 4},\n");
  fprintf(cdl, "		<| up{(3 * i) + 5}, >| down{(3 * i) + 5}; }\n");
  fprintf(cdl, "component leaf[j] { code worker; streams <| down{j + %d}, >| up{j + %d},,,; }\n\n",
		(number_leafs / 2) - 1, (number_leafs / 2) - 1);
  fprintf(cdl, "root terntree_root ^^");
  if (((number_leafs / 2) - 1) > 0)
   fprintf(cdl, "(^^ [i < %d] node{i} terntree_node) ^^ ", (number_leafs / 2) - 1);
  fprintf(cdl, "(^^ [j < %d] leaf{j} terntree_leaf)\n", number_leafs);
  
  run_taskforce(cdl);
}
/*}}}*/
/*{{{  two-dimensional toroidal grids */
/**
*** A two-dimensional grid looks like this:
***    worker00  <=>  worker01  <=>  worker02
***       /\             /\             /\ 
***       ||             ||             || 
***       \/             \/             \/ 
***    worker10  <=>  worker11  <=>  worker12
*** with the addition of wrap-around on all the edges. This test runs
*** through grids of various sizes, e.g. 2x16, 3x10, 4x8, ... The CDL
*** script for this looks like this:
***   component worker[i,j] { streams ,,,,
***		<| ltor{i,j},		  >| ltor{i, (j + 1) % $2},
***		<| rtol{i, (j + 1) % $2}, >| rtol{i, j},
***		<| utod{i,j},		  >| utod{(i + 1) % $1, j},
***		<| dtou{(i+1) % $1, j},	  >| dtou{i,j}; }
***  ^^ [i < $1, j < $2] worker{i,j} %i %j
**/
static void grids(void)
{ int	rows, cols;
  FILE	*cdl;
  char	buf[32];

  for (rows = 2, cols = NumberProcessors / 2;
       rows < cols; 
       rows++, cols = NumberProcessors / rows)
   { cdl	= start_cdl();
     if (Verbose) printf("Running a %dx%d grid\n", rows, cols);
     sprintf(buf, "%dx%d grid", rows, cols);
     CurrentTest = buf;
     fprintf(cdl, "component worker[i,j] { streams , , , ,\n");
     fprintf(cdl, "\t\t<| ltor{i,j}, >| ltor{i, (j + 1) %% %d},\n", cols);
     fprintf(cdl, "\t\t<| rtol{i, (j + 1) %% %d}, >| rtol{i, j},\n", cols);
     fprintf(cdl, "\t\t<| utod{i,j}, >| utod{(i + 1) %% %d, j},\n", rows);
     fprintf(cdl, "\t\t<| dtou{(i + 1) %% %d, j}, >| dtou{i,j}; }\n\n", rows);
     fprintf(cdl, "^^ [i < %d, j < %d] worker{i,j} 2dgrid_ %%i %%j\n", 
		rows, cols);
     run_taskforce(cdl);
     Delay(RunInterval);
   }
}
/*}}}*/
/*{{{  hypercubes */
/**
*** I am not about to attempt to draw a hypercube using block characters.
*** The degree of a hypercube has to be a power of 2. To determine the
*** connections, proceed as follows;
***  for (0 <= i < NumberProcessors)
***   for (0 <= j < Degree of hypercube)
***    task i, channel j <-> task i ^ (2 ^ j), channel j
***
*** For example, task 0 channel 0 goes to task 1, channel 0, and
*** task 31 channel 3 goes to task 23 channel 3 (0x1F ^ 0x08).
***
*** A program is needed to generate the appropriate component declarations
*** which take care of the various stream connections. With a guaranteed
*** minimum of four processors a sensible hypercube size is always possible.
**/
static void hypercubes(void)
{ int	hypercube_size; 
  int	channel;
  int	i;
  FILE	*cdl;

  for (hypercube_size = 4; 
       (2 * hypercube_size) <= NumberProcessors; 
       hypercube_size *= 2);

  if (Verbose) printf("Hypercube size is %d\n", hypercube_size);

  cdl = start_cdl();
  for (i = 0; i < hypercube_size; i++)
   { fprintf(cdl, "component worker%d { code worker; streams ,,,,\n", i);
     for (channel = 1; channel < hypercube_size; channel *= 2)
      { int neighbour = i ^ channel;
        fprintf(cdl, "\t\t<| pipe%d_%d, >| pipe%d_%d,\n", neighbour, i, i, neighbour);
      }
     fprintf(cdl, "\t\t;}\n");
   }
  for (i = 0; i < (hypercube_size - 1); i++)
   fprintf(cdl, "worker%d hypercube_ %d %d ^^\n", i, i, hypercube_size);
  fprintf(cdl, "worker%d hypercube_ %d %d\n", i, i, hypercube_size);
  run_taskforce(cdl);
}
/*}}}*/
/*{{{  fully connected */
/**
*** A fully connected taskforce is similarly impossible to draw. The CDL
*** script has to be generated by a program.
**/
static void fully_connected(void)
{ int	i, j;
  FILE	*cdl;

  cdl = start_cdl();

  for (i = 0; i < NumberProcessors; i++)
   { fprintf(cdl, "component worker%d { code worker; streams , , , , \n", i);
     for (j = 0; j < NumberProcessors; j++)
      if (i == j)
       fprintf(cdl, "\t\t, , \n");
      else
       fprintf(cdl, "\t\t <| pipe%d_%d, >| pipe%d_%d, \n", j, i, i, j);
     fprintf(cdl, "; } \n\n");
   }

  for (i = 0; i < (NumberProcessors - 1); i++)
   fprintf(cdl, "worker%d full_ %d %d ^^\n", i, i, NumberProcessors);
  fprintf(cdl, "worker%d full_ %d %d\n", i, i, NumberProcessors);

  run_taskforce(cdl);
}
/*}}}*/

/*{{{  Execute the various topologies */
static void run_topologies(void)
{
  CurrentTest = "uni-directional pipeline";
  uni_pipeline();
  Delay(RunInterval);

  CurrentTest = "bi-directional pipeline";
  bi_pipeline();
  Delay(RunInterval);

  CurrentTest = "uni-directional ring";
  uni_ring();
  Delay(RunInterval);

  CurrentTest = "bi-directional ring";
  bi_ring();
  Delay(RunInterval);

  CurrentTest = "ring with fully-connected controller";
  controller_ring1();
  Delay(RunInterval);

  CurrentTest = "ring with a singly-connected controller";
  controller_ring2();
  Delay(RunInterval);

  CurrentTest = "simple chordal ring";
  chordal_ring1();
  Delay(RunInterval);

  CurrentTest = "more complex chordal ring";
  chordal_ring2();
  Delay(RunInterval);

  CurrentTest = "farm without a load balancer";
  farm1();
  Delay(RunInterval);

  CurrentTest = "farm with a load balancer";
  farm2();
  Delay(RunInterval);

  CurrentTest = "binary tree";
  binary_tree();
  Delay(RunInterval);

  CurrentTest = "ternary tree";
  ternary_tree();
  Delay(RunInterval);

  CurrentTest = "various 2-dimensional toroidal grids";
  grids();
  Delay(RunInterval);

  CurrentTest = "hypercube";
  hypercubes();
  Delay(RunInterval);

  CurrentTest = "fully-connected taskforce";
  fully_connected();
  Delay(RunInterval);

  if (Verbose) puts("Finished with the standard topologies.");
}
/*}}}*/

/*}}}*/
/*{{{  Inspecting the result of a clean-up */
static void check_cleaners(void)
{
}
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
static void miscellaneous_tests(void)
{
}
/*}}}*/

/*}}}*/
/*{{{  main() */
int main(int argc, char **argv)
{
  if (argc > 1)
   { if ((argc > 2) || strcmp(argv[1], "-v"))
      { fputs("tftests: usage, tftests [-v]\n", stderr);
        exit(EXIT_FAILURE);
      }
     else
      Verbose = TRUE;
   }

  if (Verbose)
   { time_t	now	= time(NULL);
     printf("Taskforce Test Suite : version %s\n", VersionNumber);
     printf("Starting on %s\n", ctime(&now));
   }

  clean_up();
  initialise_network();
  RunInterval	= 5 * OneSec;
  if (Verbose) puts("Running various taskforce topologies in a pre-allocated domain.");
  run_topologies();
  empty_domain();
  RunInterval	= (10 * OneSec) + (NumberProcessors * (OneSec / 10));
  Delay(RunInterval);
  if (Verbose) puts("Running various taskforce topologies without pre-allocation.");
  run_topologies();
  if (Verbose) puts("Checking the operation of the cleaners.");
  check_cleaners();
  if (Verbose) puts("Performing miscellaneous tests.");
  miscellaneous_tests();
  clean_up();

  if (Verbose)
   { time_t	now	= time(NULL);
     printf("Testing finished on %s\n", ctime(&now));
   }
  return(EXIT_SUCCESS);  
}
/*}}}*/
