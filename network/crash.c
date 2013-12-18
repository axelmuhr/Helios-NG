/**
*** Crash.c.
***
*** BLV, 23.9.91
***
*** The purpose of this program is to crash one or more processors in a
*** more or less random way. The program is supposed to be run on the
*** root processor of the network. The network must not be protected
*** via the nsrc file.
***
*** Options:
***         1) crash by itself crashes a random processor in a random way
***         2) crash /01 /02 etc. crashes those processors specified in a
***            random way
***         3) crash <mode> crashes a random processor in a specific way
***         4) crash /01 <mode> crashes the specified processors in the
***            specified way.
***         5) crash . <mode> crashes the current processor in the specified
***            way. The crash program will invoke a copy of itself remotely
***            to crash a remote processor.
**/

#include <helios.h>
#include <root.h>
#include <syslib.h>
#include <servlib.h>
#include <nonansi.h>
#ifdef __TRAN
# include <asm.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include "rmlib.h"

/**-----------------------------------------------------------------------------
*** Data types.
**/

	/* This data type is used to hold details of the various types	*/
	/* of crash supported by the program on this processor. Both	*/
	/* the name and the function pointer are contained, so that	*/
	/* a particular crash type specified as argument can be		*/
	/* identified and the appropriate function invoked. The boolean	*/
	/* determines whether this function can be selected at random,	*/
	/* since some of the crash modes are likely to happen only as	*/
	/* a result of a hacking attempt.				*/
typedef struct CrashType {
	char		*Name;
	VoidFnPtr	Handler;
	bool		Random;
} CrashType;

	/* This data type is used to hold details of the processor's	*/
	/* memory, both the base address and the size.			*/
typedef struct MemoryDetails {
	word		MemoryBase;
	word		MemorySize;
} MemoryDetails;

/**-----------------------------------------------------------------------------
*** Forward references.
**/

	/* Handlers for the different types of crashes.			*/
static	void	corrupt_bits(void);
static	void	corrupt_words(void);
static	void	buffer_overflow(void);
static	void	stack_overflow(void);
static	void	uninitialised_pointer(void);
static	void	random_ports(void);
static	void	hog_memory(void);
static	void	random_jump(void);
static	void	vecstack_overflow(void);
#ifdef	__TRAN
static	void	out_link(void);
static	void	in_link(void);
#endif

static	void	wipe(void);

	/* Utility routines.						*/
static	void	usage(void);		/* invalid arguments to program	*/
static	void	get_memory_details(MemoryDetails *);
static	void	rexec(Object *processor, char *crash);
static	Object	*select_processor(void);
static	int	select_crash(void);
static	Object	*lookup_processor(char *);

	/* Redefine the random number generators to produce 32-bit	*/
	/* random numbers instead of 16-bit ones.			*/
#define rand()		my_rand()
#define srand(a)	my_srand(a)
static	unsigned int	my_rand(void);
static	void		my_srand(unsigned int);

/**-----------------------------------------------------------------------------
*** Static data.
**/

	/* This table holds the various types of crashes, both names	*/
	/* and handler routines.					*/
	/* BLV - stack overflow and vecstack overflow are unreliable	*/
static CrashType Crashes[] = {	
	{ "bit corruption",		&corrupt_bits,		TRUE	},
	{ "word corruption",		&corrupt_words,		TRUE	},
	{ "buffer overflow",		&buffer_overflow,	TRUE	},
	{ "stack overflow",		&stack_overflow,	FALSE /* TRUE */ 	},
	{ "vector stack overflow",	&vecstack_overflow,	FALSE /* TRUE */	},
	{ "jump to random location",	&random_jump,		TRUE	},
	{ "uninitialised pointer",	&uninitialised_pointer,	FALSE	},
	{ "hog memory",			&hog_memory,		FALSE	},
#ifdef __TRAN
	{ "output on link",		&out_link,		FALSE	},
	{ "input on link",		&in_link,		FALSE	},
#endif
	{ "ports abuse",		&random_ports,		FALSE	},
	{ "wipe",			&wipe,			FALSE	},
	{ NULL, NULL, FALSE }
};


/**----------------------------------------------------------------------------
*** main()
***
*** 1) Argument handling. The arguments are scanned for any strings not
***    beginning with / and not equal to . (current processor). There
***    should be only one such string, giving the crash type. If no
***    crash type is specified then a random one is selected.
*** 2) The argument list is scanned again. If . is encountered then the
***    appropriate crash routine is called locally. If there any arguments
***    beginning with a / character then these are processors to be crashed
***    remotely. If no processor is specified then a random one is chosen
***    and crashed.
**/

int main(int argc, char **argv)
{ int		crash_type = -1;
  int		i, j, len;
  bool		chosen_processor = FALSE;
  Object	*processor;

  srand(time(NULL) + clock());

	/* Scan the arguments looking for a crash type.			*/
  for (i = 1; i < argc; i++)
   { if (!strcmp(argv[i], ".")) continue;
     if (*argv[i] == '/') continue;

	/* Cannot specify two crash types.				*/
     if (crash_type != -1) usage();

     len = strlen(argv[i]);
     for (j = 0; Crashes[j].Name != Null(char); j++)
      if (!strncmp(argv[i], Crashes[j].Name, len))
       { crash_type = j; break; }

     if (crash_type == -1)
      { fprintf(stderr, "crash: unrecognised option/processor %s\n", argv[i]);
        usage();
      }
   }

	/* If no crash type has been specified a random one is picked.	*/
   if (crash_type == -1)
    { crash_type = select_crash();
      printf("crash: selected crash type \"%s\"\n", Crashes[crash_type].Name);
    }

	/* Now  scan the arguments again looking for processors		*/
   for (i = 1; i < argc; i++)
    { if (!strcmp(argv[i], "."))
       { chosen_processor	= TRUE;
	 (*(Crashes[crash_type].Handler))();
       }

      if (*argv[i] != '/') continue;

      processor = lookup_processor(argv[i]);
      if (processor == Null(Object)) continue;
      rexec(processor, Crashes[crash_type].Name);
      chosen_processor	= TRUE;
    }

  unless (chosen_processor)
   { processor = select_processor();
     printf("crash: selected processor \"%s\"\n", processor->Name);
     rexec(processor, Crashes[crash_type].Name);
   }

  return(EXIT_SUCCESS);
}

static void usage(void)
{ int	i;
  fputs("crash: usage, crash [processor | .] [crash type]\n", stderr);
  fputs("crash: crash types include:\n       ", stderr);
  for (i = 0; Crashes[i].Name != NULL; i++)
   fprintf(stderr, "%.3s ", Crashes[i].Name);
  fputc('\n', stderr);
  exit(EXIT_FAILURE);
}

/**----------------------------------------------------------------------------
*** Utility routines
**/

	/* This routine obtains some details about the memory on this	*/
	/* processor using a ServerInfo operation on the current	*/
	/* processor.							*/
static void get_memory_details(MemoryDetails *details)
{ char		procname[IOCDataMax];
  BYTE		stats_buffer[IOCDataMax];
  ProcStats	*stats	= (ProcStats *) stats_buffer;
  Object	*procman;

  MachineName(procname);	/* e.g. /Cluster/01		*/
  strcat(procname, "/tasks");	/* ->   /Cluster/01/tasks	*/

  procman = Locate(Null(Object), procname);
  if (procman == Null(Object))
   { fprintf(stderr, "crash: failed to locate own processor manager %s\n",
		procname);
     exit(EXIT_FAILURE);
   }

  if (ServerInfo(procman, stats_buffer) < Err_Null)
   { fprintf(stderr, "crash: failed to get processor statistics from %s\n",
		procman->Name);
     exit(EXIT_FAILURE);
   }

  Close(procman);

#ifdef __TRAN
  details->MemoryBase	= 0x80000000;
#endif

	/* MemMax does not incorporate the nucleus...Round up to nearest 512K */
  details->MemorySize	 = (stats->MemMax + (256 * 1024)) / (512 * 1024);
  details->MemorySize	*= (512 * 1024);
}

	/* Alternative random number generators, taken from ANSI C	*/
	/* library but without discarding the top 16 bits.		*/
static unsigned int next = 1;

static unsigned int rand()
{ next = (next * 1103515245) + 12345;
  return((next >> 1) & 0x7FFFFFFF);
}

static void srand(unsigned int seed)
{ next = seed;
}

	/* This routine uses the Resource Management library to examine	*/
	/* the current network and selects one of the processors at	*/
	/* random. The processor is mapped to an object structure. An	*/
	/* auxiliary is used to search the network.			*/
static int	select_processor_aux(RmProcessor, ...);

static Object	*select_processor(void)
{ RmNetwork	network			= RmGetNetwork();
  int		network_size		= RmCountProcessors(network);
  RmProcessor	choice			= (RmProcessor) NULL;
  Object	*chosen_processor	= Null(Object);
  int		i;
  
	/* loop a few times trying to choose a suitable processor	*/
  for (i = 0; (choice == (RmProcessor) NULL) && (i < 32); i++)
   choice = (RmProcessor) RmSearchNetwork(network, &select_processor_aux, network_size);

  if (choice == (RmProcessor) NULL)
   { fputs( "crash: failed to find a suitable processor in the network.\n", stderr);
     exit(EXIT_FAILURE);
   }
  chosen_processor = RmMapProcessorToObject(choice);

  if (chosen_processor == Null(Object))
   { fprintf(stderr, "crash: failed to map processor %s to an Object\n",
		RmGetProcessorId(choice));
     exit(EXIT_FAILURE);
   }

	/* BLV - the Object returned by RmMapProcessorToObject is	*/
	/* a read-only one at present.					*/
  { Object	*temp = Locate(Null(Object), chosen_processor->Name);
    Close(chosen_processor);
    chosen_processor = temp;
  }

  RmFreeNetwork(network);
  return(chosen_processor);
}

static int select_processor_aux(RmProcessor processor, ...)
{ va_list	args;
  int		network_size;

  va_start(args, processor);
  network_size = va_arg(args, int);
  va_end(args);

	/* Limit crashes to ordinary worker processors.			*/
  if (RmGetProcessorPurpose(processor) != (RmP_Normal + RmP_User))
   return(0);

	/* There is a finite chance that this processor will be		*/
	/* selected. The factor of 2 should ensure that processors	*/
	/* later on in the network have a reasonable chance.		*/
  if ((rand() % (2 * network_size)) != 0)
   return(0);

  return((int) processor);
}

	/* This routine examines the crash table and selects a suitable	*/
	/* one.	The size of the table is determined, and random entries	*/
	/* are selected until a suitable one is found. Some entries	*/
	/* cannot be selected at random.				*/
static int	select_crash(void)
{ int		table_size;
  int		i;

  for (table_size = 0; Crashes[table_size].Name != Null(char); table_size++);

  do
   i = rand() % table_size;
  until (Crashes[i].Random);

  return(i);
}

static	Object	*lookup_processor(char *name)
{ Object	*result = Locate(Null(Object), name);
  Object	*procman;

  if (result == Null(Object))
   { fprintf(stderr, "crash: failed to locate processor %s\n", name);
     exit(EXIT_FAILURE);
   }

  procman = Locate(result, "tasks");
  if (procman == Null(Object))
   { fprintf(stderr, "crash: %s does not appear to be a processor.\n", name);
     exit(EXIT_FAILURE);
   }

  Close(procman);
  return(result);
}

	/* The rexec() routine. This is given an Object pointer for the	*/
	/* target processor and a string representing the crash. The	*/
	/* processor manager is located, the task is executed, and a	*/
	/* suitable environment is created and sent off.		*/
static void rexec(Object *processor, char *crash_name)
{ Environ	*my_env		= getenviron();
  Object	*loader_entry;
  Object	*procman;
  Object	*executing;
  Stream	*program;
  Environ	env;
  Object	*objv[OV_End + 1];
  char		*argv[4];

  if (processor == Null(Object))
   { fputs("crash: rexec called with invalid processor\n", stderr);
     exit(EXIT_FAILURE);
   }
  procman = Locate(processor, "tasks");
  if (procman == Null(Object))
   { fprintf(stderr, "crash: failed to locate %s/tasks\n", processor->Name);
     exit(EXIT_FAILURE);
   }
  loader_entry = my_env->Objv[OV_Code];
  if (loader_entry == Null(Object))
   if ((loader_entry = my_env->Objv[OV_Source]) == Null(Object))
    { fputs("crash: failed to find code for this program.\n", stderr);
      exit(EXIT_FAILURE);
    }
  executing = Execute(procman, loader_entry);
  if (executing == Null(Object))
   { fprintf(stderr, "crash: failed to execute program in %s, fault 0x%x\n",
		procman->Name, Result2(procman));
     exit(EXIT_FAILURE);
   }
  program = Open(executing, Null(char), O_ReadWrite);
  if (program == Null(Stream))
   { fprintf(stderr, "crash: failed to open running program %s, fault 0x%x\n",
		executing->Name, Result2(executing));
     (void) Delete(executing, Null(char));
     (void) Delete(executing, Null(char));
     (void) Delete(executing, Null(char));
     exit(EXIT_FAILURE);
   }

  env.Strv		= my_env->Strv;
  env.Envv		= my_env->Envv;
  argv[0]		= "crash";
  argv[1]		= ".";
  argv[2]		= crash_name;
  argv[3]		= Null(char);
  env.Argv		= argv;
  objv[OV_Cdir]		= my_env->Objv[OV_Cdir];
  objv[OV_Task]		= executing;
  objv[OV_Code]		= (Object *) MinInt;
  objv[OV_Source]	= loader_entry;
  objv[OV_Parent]	= my_env->Objv[OV_Task];
  objv[OV_Home]		= my_env->Objv[OV_Home];
  objv[OV_Console]	= my_env->Objv[OV_Console];
  objv[OV_CServer]	= my_env->Objv[OV_CServer];
  objv[OV_Session]	= my_env->Objv[OV_Session];
  objv[OV_TFM]		= my_env->Objv[OV_TFM];
  objv[OV_TForce]	= (Object *) MinInt;
  objv[OV_End]		= Null(Object);
  env.Objv		= objv;

  (void) SendEnv(program->Server, &env);
  Close(program);
  Close(executing);
  Close(procman);
}

/**----------------------------------------------------------------------------
*** The individual crash routines.
**/

	/* corrupt_bits(). This routine corrupts random bits of memory	*/
	/* fairly random intervals of between 1/100 and 1/10 second.	*/
	/* Corruption of individual bits is typically caused by a	*/
	/* hardware failure.						*/
static void corrupt_bits(void)
{ MemoryDetails	details;
  int		delay;
  word		*addr;
  word		bit;

  get_memory_details(&details);
  delay = (rand() % ((9 * OneSec) / 100)) + (OneSec / 100);

  forever
   {
	/* random word address within the permitted range.		*/
     addr = (word *) ((details.MemoryBase + (rand() % details.MemorySize)) & ~0x03);

	/* random bit position within the word.				*/
     bit  = rand() % 32;

	/* set/clear the appropriate bit.				*/
     *addr ^= (0x01 << bit);

     Delay(delay);
   }

	/* This routine continues running until the processor crashes.	*/
}

	/*--------------------------------------------------------------*/
	/* This routine is very similar to corrupt_bits() but zaps a	*/
	/* whole word at a time. Nine times out of ten this word is 0,	*/
	/* the other time it is a completely random number. This	*/
	/* happens mainly when C programmers are careless with their	*/
	/* pointers.							*/
static void corrupt_words(void)
{ MemoryDetails	details;
  int		delay;
  word		*addr;

  get_memory_details(&details);
  delay = (rand() % ((9 * OneSec) / 100)) + (OneSec / 100);

  forever
   { addr = (word *) ((details.MemoryBase + (rand() % details.MemorySize)) & ~0x03);

     if ((rand() % 10) == 0)
      *addr = rand();
     else
      *addr = 0;

     Delay(delay);
   }
}

	/*--------------------------------------------------------------*/
	/* This routine tests the overflowing of dynamically-allocated	*/
	/* buffers, resulting in corruption of the heap. It allocates	*/
	/* chunks of memory of up to 64K and clears a buffer of this	*/
	/* size plus up to another 64K.	This buffer is then freed.	*/
	/* Frequent causes include allocating byte buffers rather than	*/
	/* word buffers, and copying duff strings at the end of a data	*/
	/* structure.							*/
static	void buffer_overflow(void)
{ int	delay;
  byte	*buffer;
  word	size;
  word	overflow = 1;

  delay = (rand() % ((9 * OneSec) / 10)) + (OneSec / 10);

  forever
   { size	= rand() % (64 * 1024);
     buffer	= malloc(size);
     if (buffer == NULL) continue;

	/* Calculate, 1 2 4 8 16 ... bytes overflow.			*/
     overflow = 1 << (rand() % 16);

     memset(buffer, 0, size + overflow);
     free(buffer);

     Delay(delay);
   }
}

	/*--------------------------------------------------------------*/
	/* stack overflow - stack checking is disabled by a pragma.	*/
	/* Then the routine simply calls itself, with a local		*/
	/* variable on the stack plus a call to a system routine.	*/
	/* This happens if the programmer is careless enough to disable	*/
	/* stack checking for a slight improvement in performance,	*/
	/* without determining the exact stack requirements in advance.	*/
#pragma	-s1

static void stack_overflow(void)
{ time_t	junk;

  (void) time(&junk);
  stack_overflow();
  (void) time(&junk);
}

#pragma	-s0

	/*--------------------------------------------------------------*/
	/* Uninitialised pointer - this is slightly tricky. Two		*/
	/* auxiliary routines are defined, one of which puts random	*/
	/* numbers on the stack while the other zaps an uninitialised	*/
	/* pointer on the stack. Typically uninitialised pointers are	*/
	/* either global variables or part of a data structure.	The end	*/
	/* result is basically equivalent to word corruption.		*/
static	int	uninitialised_pointer_aux1(void);
static	void	uninitialised_pointer_aux2(void);

static void uninitialised_pointer(void)
{ int	delay;

  delay = (rand() % ((9 * OneSec) / 10)) + (OneSec / 10);

  forever 
   { (void) uninitialised_pointer_aux1();
     uninitialised_pointer_aux2();
     Delay(delay);
   }
}

static	int	uninitialised_pointer_aux1(void)
{ MemoryDetails	details;
  int		a, b, c, d, e;

  get_memory_details(&details);
  a = details.MemoryBase + rand();
  b = details.MemoryBase + rand();
  c = details.MemoryBase + rand();
  d = details.MemoryBase + rand();
  e = details.MemoryBase + rand();

  return(a + b + c + d + e);
}

static void	uninitialised_pointer_aux2(void)
{ word	*pointer;
  *pointer = rand();
}

	/*--------------------------------------------------------------*/
	/* This routine jumps to a random location in memory. Typically	*/
	/* this might happen if the return address on the stack has	*/
	/* been zapped.							*/

static	void	random_jump(void)
{ MemoryDetails	details;
  union { 
	VoidFnPtr	x;
	int		y;
  } addr;
  
  get_memory_details(&details);

  addr.y = ((details.MemoryBase + (rand() % details.MemorySize)) & ~0x03);
  (*(addr.x))();
}

	/*--------------------------------------------------------------*/
	/* A vector stack overflow is similar to a buffer overflow, but	*/
	/* the buffer is on the stack or vector stack rather than	*/
	/* being malloc'ed. Errors occur for exactly the same reasons.	*/
	/* A typical overflow results in corrupting the stack and in	*/
	/* particular the return address. Since the overflow may not	*/
	/* be detected until the routine returns, an auxiliary is	*/
	/* required.							*/
static	void	vecstack_overflow_aux1(void);

static	void	vecstack_overflow(void)
{ int		delay;

  forever
   { vecstack_overflow_aux1();
     delay = (rand() % ((9 * OneSec) / 10)) + (OneSec / 10);
   }
}

static	void	vecstack_overflow_aux1(void)
{ char	buffer[48];
  int	size;
  int	i;

  size = 0x20 << (rand() % 16);

	/* Do not use memset() here, since memset() appears to fail	*/
	/* cleanly most of the time when its local variables get	*/
	/* corrupted.							*/
  /*memset(&(buffer[0]), 0, size);*/
  for (i = 0; i < size; i++) buffer[i] = rand();
}

	/*--------------------------------------------------------------*/
	/* This routine is slightly unusual in that it does nothing	*/
	/* actually illegal. Instead it continually grabs all memory	*/
	/* that is available, leaving nothing for the system. In	*/
	/* theory the system should be able to cope with this albeit	*/
	/* with a reduced performance....				*/
static void hog_memory(void)
{ int	size;
  int	delay;

  delay = (rand() % ((9 * OneSec) / 10)) + (OneSec / 10);

  forever
   { size = 1 << (rand() % 16);
     if( malloc(size) == NULL)
      Delay(delay);
   }
}

	/*--------------------------------------------------------------*/
	/* In theory a clever hacker can figure out how Helios message	*/
	/* ports are encoded, which ports are remote ones, and send	*/
	/* 64K messages to all remote ones. This has the effect of	*/
	/* crashing remote processors rather than the local one, and	*/
	/* may well crash the root processor in the system. Such errors	*/
	/* are most unlikely to happen by accident and hence this	*/
	/* routine is only invoked if specifically requested. An	*/
	/* auxiliary is used to actually send the message. Helios 2.0	*/
	/* will protect itself from these errors.			*/

static void send_message(Port port);

static void random_ports(void)
{ RootStruct	*root			= GetRoot();
  PortInfo	**table_of_tables	= (PortInfo **) root->PortTable;
  PortInfo	*port_table;
  word		table_size		= root->PTSize / 4 - 1;
  int		i, j;
  Port		port;
  int		delay;

  delay = (rand() % ((9 * OneSec) / 10)) + (OneSec / 10);

  forever
   { for (j = 0; j < table_size; j++)
      { port_table = table_of_tables[j];
        if (port_table == NULL) continue;
        if (port_table == (PortInfo *) MinInt) continue;

        for (i = 0; i < 64; i++)
	 { if ((port_table[i].Type != T_Trail) && (port_table[i].Type != T_Surrogate))
	    continue;
	   port  = 0xE0000000;
	   port += (port_table[i].Cycle << 16);
	   port += (j << 8);
	   port += i;
	   send_message(port);
	 }	/* end of this particular port table	*/
      }		/* For each port table			*/

     Delay(delay);
   }	/* forever...					*/
}

static	void send_message(Port port)
{ MCB		m;
  MemoryDetails	details;
  void		*addr;

  get_memory_details(&details);
  addr = (void *) ((details.MemoryBase + (rand() % details.MemorySize)) & ~0x03);

  InitMCB(&m, 0, port, NullPort, Err_Null);
  m.MsgHdr.DataSize	= rand() % (64 * 1024);
  m.MsgHdr.ContSize	= rand() % 256;
  m.Data		= (BYTE *) addr;
  m.Control		= (WORD *) addr;

  PutMsg(&m);
}
  
#ifdef __TRAN
	/*--------------------------------------------------------------*/
	/* Under Helios the links are usually reserved for use by the	*/
	/* system. However the instructions to access the links are 	*/
	/* not protected so in theory application programmers could	*/
	/* produce code that accesses the links directly in parallel	*/
	/* with the system. This is only likely to happen as a result	*/
	/* of hacking activity. Problems include links being marked as	*/
	/* faulty when both processors are running happily, messages	*/
	/* being intercepted and discarded, and so on. These errors	*/
	/* are unlikely to happen by accident and hence these crashes	*/
	/* will not be chosen at random. To avoid blocking the main	*/
	/* thread other threads are spawned to actually interact with	*/
	/* the links.							*/
static void do_out(int);
static void do_in(int);

static void out_link(void)
{ int		delay;
  int		link;

  delay = (rand() % ((9 * OneSec) / 10)) + (OneSec / 10);

  forever
   { link	= rand() % 4;
     (void) Fork(1000, &do_out, sizeof(link), link);
     Delay(delay);
   }
}

static void in_link(void)
{ int		delay;
  int		link;

  delay = (rand() % ((9 * OneSec) / 10)) + (OneSec / 10);

  forever
   { link	= rand() % 4;
     (void) Fork(1000, &do_in, sizeof(link), link);
     Delay(delay);
   }
}

static void do_out(int link)
{ MemoryDetails	details;
  int		addr;
  int		size;
  unsigned int	channel = 0x80000000 + (4 * link);

  get_memory_details(&details);
  addr = (details.MemoryBase + (rand() % details.MemorySize) & ~0x03);
  size = rand() % (64 * 1024);

  out_(size, channel, addr);
}

static void do_in(int link)
{ MemoryDetails	details;
  int		addr;
  int		size;
  unsigned int	channel = 0x80000010 + (4 * link);

  get_memory_details(&details);
  addr = (details.MemoryBase + (rand() % details.MemorySize) & ~0x03);
  size = rand() % (64 * 1024);

  in_(size, channel, addr);
}


#endif

static void wipe(void)
{ int	*x = (int *) 0x80001000;

  memset(x, 0, *x);
}

