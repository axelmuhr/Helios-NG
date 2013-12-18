/*------------------------------------------------------------------------
--                                                                      --
--         H E L I O S   I N P U T / O U T P U T   S E R V E R          --
--         ---------------------------------------------------          --
--                                                                      --
--         Copyright (C) 1987, Perihelion Software Ltd.                 --
--                       All Rights Reserved.                           --
--                                                                      --
--  server.c                                                            --
--                                                                      --
--          The main module of the I/O server.                          --
--                                                                      --
--  Author:  BLV 8/10/87                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: server.c,v 1.26 1994/07/06 10:44:59 mgun Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.       			*/

#define Server_Module

#include <stdlib.h>

/**
***    This is the main module of the Helios input/output Server. It contains
*** the start-up code, the main loops, and many utilities.
***
***    Please note the following conventions. Server with a capital S refers
*** to the entire system, i.e. to all the devices together, whereas server
*** with a little s refers to the particular server for a device, e.g.
*** the console server or a server for one of the drives. Processor usually
*** refers to the target processor serviced by the I/O Server, which may
*** be a transputer or it may be some other chip running Helios.
**/

/**
*** Header files :
***               sccs.h declares static strings containing the current
***               version number, its date, and a copyright message. This
***               is displayed when the Server starts up.
***
***               helios.h is the main header file. It is a catenation of
***               all the header files, and should be kept in Ram disk if
***               possible. Using a single header file greatly reduces the
***               time needed for compilation. For details of which header
***               files form part of helios.h, see the makefile.
**/

#include "helios.h"
#include "sccs.h"

/**
*** Function declarations
***                      The following lines declare the types of some of the
*** functions used in this module. PRIVATE is an alias for static, i.e. not
*** accessible from outside this module. Functions which may be accessed from
*** other modules are declared in the header file fundefs.h . Also, a large
*** number of public variables are declared in the header file server.h
***/

PRIVATE word fn( Init,               (void));
PRIVATE int  fn( Server,             (void));
PRIVATE int  fn( MainLoop,           (void));
PRIVATE void fn( TidyUp,             (void));
PRIVATE void fn( General_Server,     (Conode *));
PRIVATE void fn( add_config,         (char *, List *));
PRIVATE int  fn( read_config,        (char *));
PRIVATE void fn( tidy_config,        (void));

/*------------------------------------------------------------------------
--
-- int main(argc, argv)
--
------------------------------------------------------------------------*/

/**
***    This is the program start-up. It does lots of initialisation, and
***  starts up either the Server or the debugger.
***
***    On the ST I need to worry about the stack size for the main coroutine,
***  which has to be set in a very strange way under Mark Williams C.
***
***    If the server wants to do its own memory management then this must be
***  initialised as early as possible. The header file defines.h determines
***  whether or not the server uses the standard C library's memory routines,
***  and there are some memory management routines declared at the end of
***  fundefs.h
***
***    I initialise two strings, system_image which contains the name of
***  the system image to be booted into the Processor, and a temporary
***  string, and I set up a jump buffer. This setjmp() allows all the other
***  initialisation routines to exit the program safely at any time : exit()
***  is not safe because I may have zapped some interrupt vectors. A set of
***  flags is maintained to check which bits of the initialisation have
***  been done, so that those and only those can be undone.
***
***  If the implementation of the Server supports Gem, and Gem is currently
***  loaded, then there is a good chance that the argument vector is junk
***  as I discovered to my great surprise. Hence I need a special test to
***  avoid analysing the command line.
***
***    Otherwise I analyse the command line used to activate the server. There
***  are a number of command line options:
***      -Cfilename : this specifies a configuration file other than the
***                   default HOST.CON in the current directory.
***      -D         : enter system in debugger mode.
***      -{flags}   : set the specified debugging flags. The header file
***                   debugopt.h contains details of the debugging flags
***                   currently supported.
***      +<option>  : add a configuration option
***      filename   : this specifies that some system image other than the
***                   one specified in HOST.CON is to be booted into the
***                   Processor.
***
***     Once the command line has been analysed I know the name of the
***  configuration file and I can read it in. This is done by routine
***  read_config(), in this module. The configuration file usually specifies
***  a system image, but this may have been overruled by the command line.
***  Some system image must have been specified either in the configuration
***  file or on the command line.
***
***    With the information from the configuration file it is now possible
***  to initialise all parts of the I/O Server. InitMultiwait() is a
***  machine-specific function, used in a multi-tasking system to avoid
***  busy-waiting. init_logger() in module files.c sets up the error logging
***  system. initialise_devices() is in module devices.c, and sets up whatever
***  devices are supported by the Server on the current hardware.
***  init_boot() in module tload.c sets up the bootstrap mechanism, and
***  on certain hardware it checks that there really is a Processor to boot
***  Helios into. init_main_message() is also in module tload.c, and
***  sets up a single large buffer for all message transactions between the
***  I/O Server and the target Processor. Finally, 
***  init_debug() is in module debug.c, and initialises some bits and pieces
***  of the debugging system.
***
***     By now I have done all my once-off initialisation calls, and I can
***  activate either the Server or the debugger depending on whether or not
***  the user specified debugging mode. It is possible to reboot the Server
***  and to switch between the Server and the debugger many times, so the
***  activation is done in an infinite loop. The string ANSIclear
***  clears the screen, and this string is output if the Server is entered.
***  If the debugger is entered there may be some useful information still
***  on the screen so I do not want to clear it. It is important to reset
***  all the devices by calling restart_devices() in module devices.c, e.g.
***  to flush out characters still in input buffers. It is also important to
***  output a copyright message, for obvious reasons, and on a multi-tasking
***  system it may be necessary to tidy up the MultiWait support.
***
***    Entering the Server involves booting up the root Processor and some
***  strange message interaction, see module tload.c for details of
***  boot_processor() and server_helios(). When the Processor has been
***  booted its kernel and system tasks will be spending some time
***  initialising themselves, so I can use some time initialising myself
***  for Server mode. Server() can be found further down in this module.
***
***    Entering the debugger involves making sure that all output goes to
***  the screen rather than to a logfile, which would make debugging
***  difficult. The routine debug() in module debug.c takes care of the
***  rest.
***
***    The main loop is exited only when the user explicitly wants to exit.
***  All the bits that have been initialised are undone.
***
***    It is desirable for the server to exit tidily with a decent error code,
***  slightly tricky given all the longjmp'ing going on. However, exit always
***  involves going via the bit at the end of main() (unless memory
***  initialisation fails). The I/O Server should not normally call exit()
***  or _exit(), except as a last resort.
**/

#if ST
word _stksize = 20000L;       /* should be plenty */
#endif

PRIVATE int return_code;
PRIVATE char *configname = "host.con";

#if ANSI_prototypes
#  if MSWINDOWS
int server_main(int argc, char *argv[])
#  else
int main (int argc, char *argv[])
#  endif

#else

#  if MSWINDOWS
int server_main(argc, argv)
#  else
int main(argc, argv)
#  endif
int argc;
char *argv[];
#endif

{ int i, j, k;
#define done_config      0x001
#define done_boot        0x002
#define done_devices     0x004
#define done_logger      0x008
#define done_message     0x010
#define done_debugger    0x020
#define done_memory      0x040
#define done_multiwait   0x080
  int what_done = 0;
  char tempname[80], *curr_arg;
  PRIVATE char ANSIclear[] = { 0x0C, '\0' };

#if (PC)
	/* To get around protection problems with MS-Windows it is	*/
	/* necessary to copy stacks during a coroutine switch !!!	*/
	/* Whenever a coroutine is activated its stack is copied into	*/
	/* this buffer, and copied out again when the coroutine		*/
	/* suspends. The root coroutine is a special case.		*/
	/* This patch is also necessary for Microsoft C 7.0		*/
	/*								*/
	/* A similar hack is needed for interrupt routines, to cope	*/
	/* with the C compiler continually assuming that ds == ss in	*/
	/* spite of all the compiler flags.				*/
  char		buf[4196];		/* 4K + a bit to spare		*/
  extern	char	*coroutine_buf;	/* In the assembler file	*/
  char		buf2[1024];		/* Plenty for interrupts	*/
  extern	char	*interrupt_buf;
  coroutine_buf = buf;
  interrupt_buf	= &(buf2[1000]);
#endif

  time_unit = OneSec / CLK_TCK;		/* do init here for i486 */

#if internet_supported
	setsu(-1);		/* if we are su setuid, turn off for now */
#endif

#if (PC && !MSWINDOWS)
     /* These are set up by the library but never used. */
     /* Closing them gives me some more streams.        */
  fclose(stdaux); fclose(stdprn);
#endif

#if use_own_memory_management
    initialise_memory();
    what_done |= done_memory;
#endif

#if (MAC)
	init_mac();
#endif /* MAC */

  system_image[0] = '\0';
  tempname[0] = '\0';

#if SOLARIS
  exit_jmpbuf = (jmp_buf *)(malloc (256));

  if ((return_code = setjmp (*exit_jmpbuf)) ne 0)
    {
      ServerDebug ("Ending after setjmp ()");
      goto endpoint;
    }

#else
  if ((return_code = setjmp(exit_jmpbuf)) ne 0) goto endpoint;
#endif

#if gem_supported
#if PC
  { bool i;
    Gem_Testfun(&i);
    if (i) goto skipargs;
  }
#endif
#endif
                            /* Examine the command line arguments.*/
  for (i=1; i<argc; i++)
    { curr_arg = argv[i];
      if (curr_arg[0] eq '+')
       { add_config(&(curr_arg[1]), (List *) NULL);
         continue;
       }
      if (curr_arg[0] eq '-')
       if ((curr_arg[1] eq 'C') || (curr_arg[1] eq 'c'))
        { if (curr_arg[2] ne '\0')
           configname = &(curr_arg[2]);
          elif (++i < argc)
           configname = argv[i];
        }
#if debugger_incorporated
       elif ((curr_arg[1] eq 'D') || (curr_arg[1] eq 'd'))
           DebugMode = 1;
#endif
       elif ((curr_arg[1] eq 'A') || (curr_arg[1] eq 'a'))
        debugflags = All_Debug_Flags;
       elif ((curr_arg[1] eq 'E') || (curr_arg[1] eq 'e'))
        EnableThatLink = 1;
       else
           { for (j=1; curr_arg[j] ne '\0'; j++)
              for (k=0; options_list[k].flagchar ne '\0'; k++)
                { 
                  if (((int) curr_arg[j]) eq options_list[k].flagchar)
                    debugflags |= options_list[k].flag; 
                }
           }
      else
       strcpy(tempname, curr_arg);   /* argument specifies system image */
    }

#if PC
skipargs:
#endif

  if (!read_config(configname))
    {
      ServerDebug ("failed read_config (%s) call", configname);
      goto endpoint;
    }
  what_done |= done_config;

#if multi_tasking
  InitMultiwait();
  what_done |= done_multiwait;
#endif

  init_logger();
  what_done |= done_logger;

/**
*** N.B : after initialise_devices output should go via ServerDebug(),
*** not via printf() or fprintf().
**/
  initialise_files();
  initialise_devices();
  what_done |= done_devices;
  output(ANSIclear);
  output("Helios "); output(SccsId2);
#ifdef SMALL
  output(" reduced");
#endif
  output(" I/O Server"); output(SccsId1); output(SccsId5);

  if (!init_boot())
   { output("Helios "); output(SccsId2);
#ifdef SMALL
     output(" reduced");
#endif
     output(" I/O Server"); output(SccsId1); output(SccsId5);
     output("\r\nFailed to locate processor.\r\nExiting.\r\n\n");

     goto endpoint;
   }
  what_done |= done_boot;

  init_main_message();         /* in module tload.c */
  what_done |= done_message;

#if debugger_incorporated
  init_debug();             /* This should be done once only.     */
  what_done |= done_debugger;
#endif

  if (strlen(tempname) ne 0)     /* command line argument takes preference */
    strcpy(system_image, tempname);

  if ((get_config("no_image") == NULL) &&
      ((get_config("just_attach") == NULL)||(get_config("enable_link") == NULL))
      && (strlen(system_image) eq 0))
    {
      output("No image file specified\n");
      goto endpoint;
    }

  forever
   { 
#if multi_tasking
     RestartMultiwait();
#endif

     restart_devices();

#if debugger_incorporated
     if (!DebugMode)
#endif
        { output(ANSIclear);
          output("Helios "); output(SccsId2); output(" I/O Server");
          output(SccsId1); output(SccsId5);
        }
#if debugger_incorporated
     else
        { output("Helios "); output(SccsId2); output(" Transputer Debugger");
          output(SccsId1); output(SccsId5);
        }

     if (DebugMode)
       { int old_log = log_dest;
         int result;
         debugflags &= ~Keyboard_Flag;
         log_dest    = Log_to_screen;
         result      = debug();
         log_dest    = old_log;
         if (result eq 0) break; 
       }
     else
#endif
      { if ((log_dest eq Log_to_file) || (log_dest eq Log_to_both))
         {
#if SOLARIS
	   char 	sol_buffer[30];
	   strcpy (sol_buffer, "\r\n***\r\n*** Reboot\r\n***\r\n\n");
	   write_to_log (sol_buffer);
#else
	   strcpy(misc_buffer1, "\r\n***\r\n*** Reboot\r\n***\r\n\n");
           write_to_log(misc_buffer1);
#endif
         }
        
        /* make the -e option do what is says it does in bart's manual */

        if (EnableThatLink)
	{
          if (EnableThatLink++ eq 2)
	  {
            EnableThatLink = 0;
	    init_boot();
          }
        }

        boot_processor(serverboot);

#if debugger_incorporated
        if (DebugMode)
	  {
	    continue;
	  }
#endif
        if (Special_Reboot)
        {
          Special_Reboot = false ;
          continue;
        }
        if (Server() eq 0) break;
      }
   }

endpoint:

  if (debugflags & Quit_Flag) printf("Restoring devices.\r\n");
  if (what_done & done_devices) restore_devices();


  if (debugflags & Quit_Flag) printf("Freeing main message buffer.\r\n");

  if (what_done & done_message) free_main_message();   /* see tload.c */  

#if debugger_incorporated
  if (debugflags & Quit_Flag) printf("Tidying debugger.\r\n");
  if (what_done & done_debugger) tidy_debug();
#endif

  if (debugflags & Quit_Flag) printf("Tidying bootstrap.\r\n");
  if (what_done & done_boot) tidy_boot();

#if (UNIX || PC || HELIOS)
  { extern void fn( tidy_link, (void));
    if (debugflags & Quit_Flag) printf("Restoring link interface.\r\n");
    tidy_link();
  }
#endif

  if (debugflags & Quit_Flag) printf("Terminating logger.\r\n");
  if (what_done & done_logger) tidy_logger();

#if multi_tasking
  if (debugflags & Quit_Flag) printf("Tidying MultiWait.\r\n");
  if (what_done & done_multiwait) TidyMultiwait();
#endif

  if (debugflags & Quit_Flag) printf("Freeing configuration information.\r\n");
  if (what_done & done_config) tidy_config();

  if (debugflags & Quit_Flag) printf("Server exiting.\r\n");
  return(return_code);
}

/*------------------------------------------------------------------------
--
-- The system configuration file.
--
------------------------------------------------------------------------*/

/**
***   The Server runs on a variety of different hardware configurations,
***  on different hosts and on different Processor boxes for a given host.
***  The Server can adapt to this, provided it knows exactly what the
***  hardware is. This is the purpose of the configuration file.
***
***   A configuration file consists of a series of strings like :
***      box = ATW
***  These strings resemble C environment strings. The following bits of code
***  are responsible for reading in the configuration and giving details of
***  the configuration to other parts of the system.
***
***   The configuration information is stored in a linked list, so I declare
***  a suitable data type and list header. read_config() is called by main()
***  and reads in the configuration file, one line at a time. I assume that
***  each entry in the configuration file is on a separate line. Storing the
***  information in the linked list is trivial. For details of the linked
***  lists see module cofuns.c .
***
***   Once the information has all been read in, I analyse a few bits of it.
***  Some items are essential in all configuration files, so I test for these,
***  and some other items are almost certainly present.
***
***   tidy_config() is called when the system exits, and frees all the memory
***  taken up by the configuration linked list. This can be done conveniently
***  using WalkList().
***
***   When some bit of the system wants to know something about the
***  configuration it can call get_config() with a suitable string.
***  Get_config() wanders down the linked list until it finds an entry
***  which matches the search string, and returns the appropriate part of
***  this entry. Hence it operates in a similar way to the C routine
***  getenv(). There is a related routine get_int_config() which takes a
***  configuration entry and converts it to a word, useful for numeric
***  arguments. Get_int_config() returns Invalid_config to indicate failure,
***  and can deal with hexadecimal and octal data.
***
***  In addition, it is possible to specify extra options on the command
***  line using a +<xyz> argument, and it is possible to reread the
***  configuration file at any time.
**/ 

PRIVATE List config_hdr;/* This is the list used to hold the configuration. */
PRIVATE List base_config;  /* command line config entries */
typedef struct { Node node;
                 char name[1];         /* space for byte '\0' */
} config;
PRIVATE int config_initialised = 0;
PRIVATE int base_config_initialised = 0;

/**
*** This routine adds a configuration option to a list. It may be called
*** when processing the command line arguments, in which case the
*** list argument will be a null pointer, or it may be called when
*** reading in a configuration file to add an option to the main configuration
*** list.
**/

#if ANSI_prototypes
PRIVATE void add_config (char * option, List * list)
#else
PRIVATE void add_config(option, list)
char *option;
List *list;
#endif
{ config *newconfig;
  if (list eq (List *) NULL)
   { unless(base_config_initialised) InitList(&base_config);
     base_config_initialised = 1;
     list = &base_config;
   }

  if (*option eq '\0') return;
  newconfig = (config *) malloc(sizeof(config) + strlen(option));
  if (newconfig eq (config *) NULL)
   { printf("Not enough memory to initialise server.\n");
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
   }
  strcpy(&(newconfig->name[0]), option);

  AddTail((Node *) newconfig, list);
}

/**
*** The next three routines deal with reading or rereading the configuration
*** file. The first step is to copy all the command line options into
*** the new configuration using a WalkList. Then the configuration file is
*** read in, and all the entries are added. The compulsory entries are
*** checked. Finally any entries relative to the Helios directory, e.g
*** bootfile = ~lib/nboot.i, are expanded.
**/

#if ANSI_prototypes
PRIVATE void copy_config (config * node)
#else
PRIVATE void copy_config(node)
config *node;
#endif
{ add_config(&(node->name[0]), &config_hdr);
}

#if ANSI_prototypes
PRIVATE void test_relative (config * node)
#else
PRIVATE void test_relative(node)
config *node;
#endif
{ register char *p = &(node->name[0]);
  config        *newnode;
  char          *buff = (char *) &(misc_buffer1[0]);

  for ( ; (*p ne '=') && (*p ne '\0'); p++);
  if (*p eq '\0')
   return;
  else
   p++;

  while (isspace(*p)) p++;
  if (*p ne '~') return;

  *p = '\0'; p++;
  strcpy(buff, &(node->name[0]));  /* bootfile = */
  strcat(buff, Heliosdir);         /* c:\helios  */
  strcat(buff, p);                 /* \lib\nboot.i */
  newnode = (config *) malloc(sizeof(config) + strlen(buff));
  if (newnode eq (config *) NULL)
   { printf("Insufficient memory on host machine for configuration.\r\n");
     exit(0); 
   }
  strcpy(newnode->name, buff);
  AddHead((Node *) newnode, &config_hdr);
  iofree(Remove((Node *) node));
}

#if ANSI_prototypes
PRIVATE int read_config (char * config_name)
#else
PRIVATE int read_config(config_name)
char *config_name;
#endif
{ FILE   *conf;
  register char   *buff = (char *) &(misc_buffer1[0]);
  register int    length;
  char   *temp;
  int    error=0;

  InitList(&config_hdr);
  unless(base_config_initialised) InitList(&base_config);

  WalkList(&base_config, (VoidNodeFnPtr)func(copy_config));

  config_initialised++;                /* There may be memory to be freed */

  conf = fopen(config_name, "r");      /* open the configuration file */
  if (conf eq NULL)
    {
      printf("Unable to open configuration file %s\r\n", config_name);
      exit(1);
    }

                                      /* and read it in, one line at a time */
  while(fgets(buff, 255, conf) ne NULL)
     {		/* Ignore leading white space */
       while (isspace(*buff)) buff++;
		/* Find and remove any comment parts of the line 	*/
       for (length = 0; buff[length] ne '\0'; length++)
        if (buff[length] eq '#')
         { buff[length] = '\0'; break; }
       if (length eq 0)
	 continue;
       length--;
                        /* fgets() leaves a newline character in the buffer */
       if ((buff[length] eq '\n') || (buff[length] eq '\r'))
         buff[length] = ' ';
       while ((isspace(buff[length])) && (length >= 0) )
        length--;
       buff[++length] = '\0';     
       add_config(buff, &config_hdr);
       buff = &(misc_buffer1[0]);
     }

  fclose(conf);

  temp = get_config("HOST");           /* There must be an entry for HOST */
  if (temp eq NULL)
    { printf("Error in configuration file, missing entry for HOST\r\n");
      error = 1;
    }

  temp = get_config("BOX");                         /* And an entry for BOX */
  if (temp eq NULL)
    { printf("Error in configuration file : missing entry for BOX.\r\n");
      error = 1;
    }

  maxdata = get_int_config("MESSAGE_LIMIT");
  if (maxdata eq Invalid_config)
    maxdata = 2000L;     /* a sensible default ? */
  elif (maxdata < 1000L)
    maxdata = 1000L;
  elif (maxdata > 65535L)        /* enforce an upper limit */
   maxdata = 65535L;

                               /* Find out where the Helios directory is */
  Heliosdir = get_config("Helios_directory");
  if (Heliosdir eq NULL)       /* the default varies from machine to machine */
#if (ST || PC)
      Heliosdir = "c:\\helios";
#endif
#if MAC
      Heliosdir = "helios";
#endif
#if AMIGA 
      Heliosdir = "helios:";
#endif
#if (UNIX)
      Heliosdir = "/usr/helios";
#endif
#if HELIOS
      Heliosdir = "/helios";
#endif
  if (error)
    { printf("\nPlease edit the configuration file %s.\r\n", config_name);
      return(0); 
    }

  WalkList(&config_hdr, (VoidNodeFnPtr)func(test_relative));

  temp = get_config("SYSTEM_IMAGE");
  if (temp ne NULL) strcpy(system_image, temp);

  return(1);
}

PRIVATE void reread_config()
{ List oldlist;

  oldlist.head = config_hdr.head; oldlist.tail = config_hdr.tail;
  oldlist.earth = (Node *) NULL;
  oldlist.head->prev = (Node *) &oldlist;
  oldlist.tail->next = (Node *) &(oldlist.earth);

  if (read_config(configname))
   { 
     ServerDebug("Reread configuration file"); 
     FreeList(&oldlist);
   }
  else
   { ServerDebug("Failed to reread configuration file");
     config_hdr.head = oldlist.head; config_hdr.tail = oldlist.tail;
     config_hdr.head->prev = (Node *) &config_hdr;
     config_hdr.tail->next = (Node *) &(config_hdr.earth);
   }
}

PRIVATE void tidy_config()
{ if (config_initialised) FreeList(&config_hdr);
  if (base_config_initialised) FreeList(&base_config);
}

        /* The following code allows the server to examine the   */
        /* configuration file. For example, to get the entry for */
        /* HOST, use a call get_config("host"). If there is      */
        /* an entry HOST=XXX get_config() returns a pointer to   */
        /* XXX. The function is fairly easy to implement using a */
        /* list Wander.                                          */
        /* If no entry is found the routine returns NULL.        */
#if ANSI_prototypes
PRIVATE char * test_name (config * node, char * name)
#else
PRIVATE char *test_name(node, name)
config *node;
char   *name;
#endif
{ register char *config_name = &(node->name[0]);

                                  /* compare name and config_name */
  while ((*name ne '\0') && (*config_name ne '\0'))
   { if ( ToUpper(*name) ne ToUpper(*config_name) )
       return(false);
     name++; config_name++;
   }
                    /* succeeded ? */
  while (isspace(*config_name)) config_name++;

  if ((*name eq '\0') && (*config_name eq '\0')) return(config_name);

  if ((*name eq '\0') && (*config_name++ eq '='))
    {   while (isspace(*config_name)) config_name++;
        return(config_name);
    }
  else
    return(false);
}

#if ANSI_prototypes
char * get_config (char * str)
#else
char *get_config(str)
char *str;
#endif
{ word result;

  result = Wander(&config_hdr, (WordNodeFnPtr) func(test_name), str);
  if (result eq false)
    return(NULL);
  else
    return((char *) result);
}

#if ANSI_prototypes
word get_int_config (char * str)
#else
word get_int_config(str)
char *str;
#endif
{ register char *result = get_config(str);
  word base = 10L, value = 0L, sign = 1L;
  word temp;

  if (result eq NULL)
    return(Invalid_config);

  while (isspace(*result)) result++;
  if (*result eq '-') { sign = -1L; result++; }
  if (*result eq '0')
    { result++;
      if (*result eq 'x' || *result eq 'X')
        { result++; base = 16L; }
      else
       base = 8L;
    }
  if (*result eq '-') { sign = -1L; result++; }


#if (UNIX || PC)
  while (isxdigit(*result))    /* Use this routine if it is available */
#else
  while ( isdigit(*result) || ('a' <= *result && *result <= 'f') ||
          ('A' <= *result && *result <= 'F'))
#endif
   { 
     switch (*result)
      { case '0' : case '1' : case '2' : case '3' : case '4' :
        case '5' : case '6' : case '7' : case '8' : case '9' :
                      temp = (word) (*result - '0'); break;

        case 'a' : case 'b' : case 'c' : case 'd' : case 'e' :
        case 'f' :    temp = 10L + (word) (*result - 'a'); break;

        case 'A' : case 'B' : case 'C' : case 'D' : case 'E' :
        case 'F' :    temp = 10L + (word) (*result - 'A'); break;
      }

     if (temp >= base) break;
     value = (base * value) + temp;
     result++;
   }

 return(value * sign);
}

/*------------------------------------------------------------------------
--
--  Server - main routine.
--
------------------------------------------------------------------------*/

/**
***   Routine Server() is called by main() when the user is in Server mode
***  and when the Processor has just been booted up. It returns zero if
***  the system should exit, or non-zero if the system should reboot or 
***  enter debugging mode.
**/

PRIVATE int Server()
{ int i=0;                /* If I fail to initialise I do not try to reboot */

  if (Init())             /* Init() sets up lists and basic coroutines */
    i = MainLoop();
  else
    ServerDebug("Server unable to initialise.");

  TidyUp();               /* clean up the Server system. */

  return(i);
}


/*------------------------------------------------------------------------
--
--  Initialise the I/O system
--
------------------------------------------------------------------------*/

/**
*** This routine is responsible for initialising the Server once the
*** Processor has been bootstrapped.
***
*** First I call InitColib() to set up the basic coroutine library.
***
*** Next I work out the current system time as a Unix time stamp, i.e.
*** seconds since 1970, using one of the machine-dependant routines.
***
*** I cannot escape any longer - time to explain the coroutine mechanism
*** used in the Server....
***
*** At any time, there will be a number of different devices available such
*** as clock, console, disk drives etc. There will also be a large number of
*** streams open. Each server and each stream has a Helios message port,
*** and programs running on the Processor send requests to these ports. 
*** All the servers and all the streams act independently of each other.
***
*** Now, programming each server and each stream so that they do not
*** interfere with each other, e.g. zapping each other's variables, is not
*** trivial. The solution is coroutines : each server and each stream runs
*** as a separate coroutine in the system, and hence it has its own stack
*** and its own local variables which are safe from the rest of the system.
*** I do not care how sceptical you are - they work for me and they have
*** saved me a lot of trouble.
***
*** The next problem is how to keep track of all these servers and streams.
*** Each coroutine (i.e. server or stream) can be in one of four states :
*** a) running, which means that it has got the CPU to itself, it can do
***    what it want, and there will not be another message coming in at the
***    wrong moment. Coroutines only stop running when they explicitly want
***    to.
*** b) waiting, which means that the coroutine has nothing to do until
***    another message arrives from the Processor.
*** c) polling, which means that the coroutine is handling some existing
***    request from the Processor but has to wait for an event to happen
***    on the host machine, e.g. for the user to press a key. In this state
***    the coroutine cannot accept new messages from the Processor, but
***    the Processor may be trying to send messages to other coroutines and
***    there is no reason why these messages cannot be handled. The polling
***    coroutine has to suspend itself regularly, the Server's main loop will
***    wake it up again, and the coroutine will check whether its event has
***    occurred yet and if not suspend itself again.
*** d) selecting, which means that a Select request has been sent to the
***    stream coroutine. If the select can be satisfied a message is sent
***    to the transputer, and the coroutine moves back to WaitingCo. This
***    is true at any time, e.g. when the user hits a key. Alternatively,
***    Helios can abort the Select simply by sending another request to
***    the coroutine.
*** So, I have three linked lists WaitingCo, PollingCo, and SelectCo,
*** and a coroutine is either running or it is held in one of these
*** three linked lists. If it is running it can move itself freely
*** between the two lists, but when it suspends itself it must be in
*** one of the three lists.
***
***  A problem with coroutines is that they share all global values, and in
*** particular the message buffer used to communicate with the Processor is
*** global. It is guaranteed that only one coroutine runs at the same time,
*** and that coroutines keep running until they do an explicit Suspend()
*** (barring interrupts ofcourse). Hence coroutines can access global data
*** safely whilst they are running, but before they do a Suspend() they must
*** save global data they want to preserve on the stack or in private bits of
*** the heap. This includes things like reply ports. Also, the main message
*** buffer cannot be used to hold data collected by polling.
***
*** The main message buffer is allocated by a single call to 
*** init_main_message(),
*** and all modules have access to a pointer mcb. Having large message
*** buffers, the upper limit being 64K, tends to reduce the time taken to
*** load large files at the cost of reducing the responsiveness of keyboard,
*** mouse etc. 
***
*** Back to the code. I create the three lists by calls to MakeHeader() in
*** module cofuns.c, where all the list and coroutine functions are kept,
*** Then I create a server coroutine for each device supported by the Server
*** and for each drive connected to the machine. See header files server.h
*** and fundefs.h for the declarations of all these strange arrays of
*** functions etc. Each coroutine starts running in routine General_Server()
*** later in this module, and General_Server() implements a general purpose
*** Helios server. When the coroutine gets say an Open request
*** General_Server() calls the appropriate functions held in its array of
*** request handlers, e.g. Console_Open(). All the coroutines have a unique
*** integer identifier, which is also the coroutine's Helios message port,
*** and they all have a name to facilitate handling distributes searches
*** described later. Finally all the servers are started so that they can
*** initialise themselves.
***
*** The first device is the IOprocessor, and it is important that this is the
*** first coroutine in the linked list. For details of this server, see module
*** devices.c
***
*** There is a special server called /helios, which is a pseudo-drive. This
*** allows the Processor to access files /helios/lib/ram etc. without
*** having to know anything about the local filing system. The directory
*** which corresponds to drive helios, e.g. c:\helios, is part of the
*** configuration file but I provide a default.
***
*** The extra field in each coroutine node specifies the type of the device.
*** This information is used by the IOPROC device when it starts up to
*** work out its linked list of server directory entries.
***
*** The optimal order for the coroutines in the linked list WaitingCo is as
*** follows : ioproc server, helios server, active streams, other servers.
*** This order is important, because each time the Server gets a request from
*** the Processor it has to search through the list to find the coroutine
*** whose id matches the destination port. Obviously when a file is being read
*** its stream coroutine is likely to get more requests than say the mouse
*** server. To keep the coroutines in the right order, stream coroutines are
*** postinserted after the Helios node, and I keep a pointer Heliosnode which
*** is initialised by Helios_InitServer() in module files.c
**/

PRIVATE word StreamTimeout;

PRIVATE word Init()
{ int i;
  register Conode *tempco;
#if internet_supported
  char *internet = get_config("internet");
#endif

  StreamTimeout = get_int_config("StreamTimeout");
  if (StreamTimeout eq Invalid_config)
   StreamTimeout = DefaultStreamTimeout;
  StreamTimeout  = divlong( StreamTimeout, time_unit);

  Startup_Time = get_unix_time();
  initial_stamp = clock();
  Device_count = 0;

  unless(InitColib())           /* start up the coroutine library */
     return(false);

  WaitingCo = MakeHeader();    /* set up the list structure */
  PollingCo = MakeHeader();
  SelectCo  = MakeHeader();
  if((WaitingCo eq NULL) || (PollingCo eq NULL) || (SelectCo eq NULL))
      return(false);
             
                     /* create a server coroutine for each device */
  for (i=0; devices[i].name ne NULL; i++) 
    {  
#if internet_supported
       if(internet == NULL && !strcmp(devices[i].name, "internet"))
	   continue;
#endif

       if (devices[i].handlers[Testfun_off] ne (VoidConFnPtr)Nullfn)
        { bool x;
	  VoidWordFnPtr test_fn = (VoidWordFnPtr)((devices[i].handlers[Testfun_off]));
	  (*test_fn)(&x);
          unless(x) continue;
        }
       tempco = NewCo(General_Server);
       unless(tempco) 
        return(false);
       Device_count += 1;
       AddTail(&(tempco->node), WaitingCo); /* Add new server to tail of list */
       tempco->id         = CoCount++;
       tempco->timelimit  = MAXINT;       /* servers never time out */
       strcpy(tempco->name, devices[i].name);
       tempco->handlers   = devices[i].handlers;
       tempco->extra      = (ptr) devices[i].type;
       tempco->flags      = 0L;
    }

#if drives_are_special
                /* And another server coroutine for each drive           */
#if (PC || ST)   
  { word   drivemap, mask, floppies;
                /* get_drives() tells me what drives are connected, and  */
                /* afterwards floppies will tell me which of these are   */
                /* floppies */
                /* This area needs a lot of rewriting on other machines. */
                /* Possibly make up a linked list of mounted drives each */
                /* node having the name ?                                */
    drivemap = get_drives(&floppies);

    mask     = 0x00000001L;
    for (i=0; i < 32; i++)
     { if (drivemap & mask)
         { 
           tempco            = NewCo(General_Server);
           unless(tempco)
             return(false);
           Device_count     += 1;
           AddTail(&(tempco->node), WaitingCo);
           tempco->id        = CoCount++;
           tempco->timelimit = MAXINT;          /* servers never time out */
           tempco->name[0]   = (char) ( (int) 'a' + i);
           tempco->name[1]   = '\0';
           tempco->handlers  = Drive_Handlers;
           tempco->extra     = (ptr) Type_Directory;
           if (floppies & mask)
            tempco->flags    = CoFlag_Floppy;
           else
            tempco->flags    = 0L;
         }
       mask <<= 1;
     }
  }
#endif   /* MSdos_compatible */


#if AMIGA
                /* And another server coroutine for each drive           */
  {
    extern word create_drive_list(); /* returns number of drives found */
    extern char * get_drive_name(); /* get next drive name from list */
    extern void free_drive_list();
    word nr_of_drives;
    char * tempname;
    
    nr_of_drives = create_drive_list();
    for(i=0;i<nr_of_drives;i++) {
       tempco            = NewCo(General_Server);
       unless(tempco) {
          free_drive_list();
          return(false);
       }
       tempname = get_drive_name();
       Device_count     += 1;
       AddTail((Node *)tempco, WaitingCo);
       tempco->id        = CoCount++;
       tempco->timelimit = MAXINT;          /* servers never time out */
       strcpy(tempco->name,tempname);
       tempco->handlers  = Drive_Handlers;
       tempco->extra     = (ptr) Type_Directory;
       tempco->flags     =  0L;
     }
  }

#endif /* AMIGA */

#endif   /* drives_are_special */

#ifdef SYNERGY
#include "synergy\\synergy.h"
#endif

#if AMIGA
#include "x.h"
#endif

   for (tempco = (Conode *) WaitingCo->head;
        tempco->node.next ne NULL;
        tempco = (Conode *) tempco->node.next)
    { Debug(Init_Flag, ("Initialising device %s.", &(tempco->name[0])) );
      StartCo(tempco); 
    }

   Debug(Init_Flag, ("Server successfully initialised."));

   return(true);
}


/*------------------------------------------------------------------------
--
-- Tidy up the Server
--
------------------------------------------------------------------------*/

/**
*** When the user wishes to reboot the server or enter the debugger, it
*** is important that I do not leave tens of coroutines each with large
*** unfreed stacks. The following little routine should persuade all
*** the coroutines to tidy up and kill themselves, by setting the
*** CoSuicide flag in the coroutine node. Each coroutine has to be programmed
*** fairly carefully so that whenever it suspends itself it checks this
*** suicide bit afterwards, and takes appropriate action.
***
*** Having killed off all the server and stream coroutines, I can release the
*** entire coroutine library and the large shared message buffer.
**/

PRIVATE void fn( TerminateCo, (Conode *));
PRIVATE int  TerminateState;

PRIVATE void TidyUp()
{ 
  Debug(Quit_Flag, ("Shutting down streams and servers."));

  for (TerminateState = 1; TerminateState <= 4; TerminateState++)
   { WalkList(WaitingCo, (VoidNodeFnPtr)func(TerminateCo));
     WalkList(PollingCo, (VoidNodeFnPtr)func(TerminateCo));
     WalkList(SelectCo,  (VoidNodeFnPtr)func(TerminateCo));
   }

  iofree(WaitingCo);   /* and free all memory that has been allocated */
  iofree(PollingCo);
  iofree(SelectCo);

#if multi_tasking
#if SOLARIS
  /* Last 0 argument is a dummy value to kepp the C++ compiler happy */
  ClearMultiwait(Multi_LinkMessage, current_link, 0);
#else
  ClearMultiwait(Multi_LinkMessage, current_link);
#endif

#endif

  TidyColib();

  Debug(Quit_Flag, ("Streams and servers shut down."));
}
                      /* try to persuade a coroutine to die off */
#if ANSI_prototypes
PRIVATE void TerminateCo (Conode * conode)
#else
PRIVATE void TerminateCo(conode)
Conode *conode;
#endif
{
	switch(TerminateState)
	{
		case 1 :	/* Streams -> suicidal		*/
				/* Also Write auxiliaries	*/
			if ((conode->name[0] == 0x7f) || (conode->name[0] == '\0'))
				conode->type = CoSuicide;
			break;
		case 2 :	/* Restart streams	*/
			if ((conode->name[0] == 0x7f) || (conode->name[0] == '\0'))
				StartCo(conode);
			break;
		case 3:		/* Servers -> suicidal */
			conode->type = CoSuicide;
			break;
		default :
			StartCo(conode);
	}
	
}

/*------------------------------------------------------------------------
-- The main loop for the Server. 
------------------------------------------------------------------------*/

/**
*** This is the main loop of the Server. It is an infinite loop, which exits
*** only when the Processor sends a special message or when the user
*** wants to exit, reboot, or enter debugging mode. It returns 0 if the user
*** wants to exit or upon receipt of a quit message from the Processor,
*** non-zero if the system should continue to run.
***
*** First the system clock is updated. An approximation to the current time
*** is needed quite frequently, and it might be expensive to call the
*** appropriate clock routines each time. Hence I use a single variable Now
*** to hold a value roughly corresponding to the current time.
***
*** Next I check various debugflags and reboot flags and so on.
*** 
*** At this point I reactivate all the coroutines which are waiting for some
*** event to happen on the host side of the world, e.g. for a key to be
*** pressed. WalkList does the trick nicely. I must also check all coroutines
*** for timeouts.
***
*** Poll_the_devices() in module devices.c does exactly that : it performs
*** whatever checks of the systems are desirable, checks which may be
*** inappropriate for a server or stream to perform. For example, console
*** coroutines should not access the keyboard directly because there may
*** be multiple streams trying to do that, so poll_the_devices() does this
*** and stores the data somewhere where it can be accessed safely by the
*** coroutines.
***
*** This has finished all the work I MUST do each time around the loop.
*** Now the question is : is the Processor trying to send me
*** some more work to do ? If not, I can just go around my main loop again,
*** updating the time stamp, reactivating polling coroutines, and so on.
*** However, if there is a message I had better get hold of it and process it.
*** And this documentation is becoming rather silly.
***
*** If the message debugging flags is set, I should print out details of the
*** message for the user to admire. Then I check for a couple of special
*** messages. FG_Terminate is a quit request, a message from Helios to tell
*** the Server to exit. FG_Reboot forces a complete reboot.
*** The message may be an error message,
*** indicating that e.g. a message to a Processor at the other end of a long
*** pipeline was lost due to congestion. I have no idea what to do with such
*** messages, so I ignore them.
***
*** If a message has got this far, it must be a message for an existing
*** server or stream. This means it has been sent to a message port, which
*** is really a coroutine identifier. Therefore I check all my coroutines
*** to see which one is meant to be activated. It may be a message for a
*** coroutine which is waiting for one, in which case there is no problem and
*** I activate the coroutine. It may be a message for a coroutine that is still
*** handling an old request, in which case I cannot accept the new message.
*** The coroutine may be currently handling a Select request, in which case
*** the Select must be aborted first.
*** It may be a message for a coroutine that has already died, e.g. because
*** of a five-minute timeout, and this can be detected using the coroutine
*** counter. Finally it may be a message for a completely wrong port.
*** Whatever the case, I send back an appropriate error code. Routine
*** Request_Return() in module tload.c takes care of sending messages
*** back to the Processor.
**/

PRIVATE   void fn( list_debugging_options, (void));
PRIVATE   void fn( timeout_check, (Conode *));
PRIVATE   void fn( polling_check, (Conode *));
PRIVATE   Conode *fn(CheckPkt, (Conode *, word));

#ifdef DEMONSTRATION
PRIVATE INT warning_not_sent = 1;
#endif

PRIVATE int MainLoop()
{ Conode    *result;
  word      tempport;

  for (;;)          /* infinite loop to fetch requests */
   { 
     Now = clock();

#ifdef DEMONSTRATION
{
  time_t time_now;
  time_now = get_unix_time();
  if ((time_now > Startup_Time + 570) && warning_not_sent)
    { ServerDebug(
            "*** WARNING : demonstration Helios is terminating in 30 seconds.");
      warning_not_sent = 0; 
    }
  if (time_now > Startup_Time + 600)    
    {
      ServerDebug("*** Demonstration Helios is now terminating.");
      Special_Exit = true;
    }
}
#endif

     if (debugflags)
     {
     if (debugflags & Reconfigure_Flag)
      { reread_config();
        debugflags &= ~Reconfigure_Flag;
      }

     if (debugflags & ListAll_Flag)
      { list_debugging_options();
        debugflags &= ~ListAll_Flag;
      }

     if (debugflags & Nopop_Flag)
      { Server_windows_nopop = !Server_windows_nopop;
        unless(Server_windows_nopop)
         ServerDebug("Server_windows_nopop disabled.");
        debugflags &= ~Nopop_Flag;
      }
 
     if ( debugflags & Log_Flag)
      { debugflags &= ~Log_Flag;
        if (log_dest eq Log_to_screen)
         { ServerDebug("Switching logging to file only.");
           log_dest = Log_to_file;
         }
        elif (log_dest eq Log_to_file)
         { log_dest = Log_to_both;
           ServerDebug("Switching logging to file and screen.");
         }
        else
         { log_dest = Log_to_screen;
           ServerDebug("Switching logging to screen only.");
         }
      }

     if ( debugflags & Memory_Flag)
      { int    total, streams;
        register Conode *mynode;
#if 0
  	extern void rs232_check (void);
	rs232_check ();
#endif
#if use_own_memory_management
        memory_map();
#endif
        total = 0; streams = 0;
        ServerDebug("Open streams are : %q");

        for (mynode = (Conode *) WaitingCo->head;
             mynode->node.next ne (Node *) NULL;
             mynode = (Conode *) mynode->node.next)
         { total++;
           if (mynode->name[0] eq 0x7F)
            { if (streams++) ServerDebug(", %q");
              ServerDebug("%s%q", &(mynode->name[1]));
            }
         }

        for (mynode = (Conode *) PollingCo->head;
             mynode->node.next ne (Node *) NULL;
             mynode = (Conode *) mynode->node.next)
         { total++;
           if (mynode->name[0] eq 0x7F)
            { if (streams++) ServerDebug(", %q");
              ServerDebug("%s%q",&(mynode->name[1]));
            }
         }

        ServerDebug(
       "\r\nThere are %d devices, %d open streams, and %d other coroutines.",
                Device_count, streams, total - (Device_count + streams) );
        debugflags &= ~Memory_Flag;
      }
     }

     if (Special_Exit)                  /* check for exit key pressed */
     {
        return(0);
     }

     if (Special_Status)                /* or for status request */
      { ServerDebug("Server alive and well. Logging goes to %q");
        if (log_dest eq Log_to_screen)
         ServerDebug("screen");
        elif (log_dest eq Log_to_file)
         ServerDebug("file");
        elif (log_dest eq Log_to_both)
         ServerDebug("screen and file");
        else
         { log_dest = Log_to_screen; ServerDebug("screen"); }
        Special_Status = false;
      }
     if (Special_Reboot)               /* or for reboot condition */
      { Special_Reboot = false;
        output("\014");                /* Clear server window */
        return(1);
      }
#if debugger_incorporated
     if (DebugMode) return(1);
#endif

                                    /* wake up any coroutines that may */
     WalkList(PollingCo, (VoidNodeFnPtr)func(polling_check)); /* be waiting */
     WalkList(SelectCo,  (VoidNodeFnPtr)func(polling_check));

                                    /* allow stream coroutines to die if */
                                    /* no message has arrived in time    */
     WalkList(WaitingCo, (VoidNodeFnPtr)func(timeout_check));

     WalkList(PollingCo, (VoidNodeFnPtr)func(timeout_check));

     WalkList(SelectCo,  (VoidNodeFnPtr)func(timeout_check));

#if multi_tasking

      /* MultiWait() is allowed to suspend the I/O Server for a while,
         typically 1/2 second, if no event is outstanding. It should not
         suspend the I/O Server indefinitely because that fouls up the
         timeout handling.
      */

    if (!Multiwait()) continue;

#endif /* multi_tasking */

     poll_the_devices();           /* in devices.c */

#if multi_tasking
          /* MultiWait() fills this in if there is outstanding link I/O */
     if (link_table[current_link].ready ne CoReady) 
      continue;
     link_table[current_link].ready = 0L;
#endif

     unless(Request_Stat())
       continue;

     tempport = (mcb->MsgHdr).Dest;
     (mcb->MsgHdr).Dest = NullPort;

     if (debugflags & Message_Flag) {
       ServerDebug("Request: fn %lx, for %lx from %lx, csize %d, dsize %ld",
                  (mcb->MsgHdr).FnRc, tempport, (mcb->MsgHdr).Reply,
                  (mcb->MsgHdr).ContSize, (word) (mcb->MsgHdr).DataSize ); 
     }
     if (((mcb->MsgHdr).FnRc & FG_Mask) eq FG_Terminate)
     {
      return(0); 
     }

     if (((mcb->MsgHdr).FnRc & FG_Mask) eq FG_Reboot)
      { output("\014");
        return(1);
      }

                      /* I want to ignore error messages for the time being */
     if (((mcb->MsgHdr).FnRc & 0x80000000L) ne 0L)
       continue;

                      /* BLV - patch to cope with very slow I/O Servers,    */
                      /* for example ones using the hydra interface. The    */
                      /* root processor may start sending the search message*/
                      /* before the handshake has finished.                 */
     if ((mcb->MsgHdr.FnRc eq 0x60002010) && (tempport eq 0L))
      tempport = 1L;
      
                                                /* who is the request for ? */

        /* A coroutine in WaitingCo can usually receive a message immediately */
     result = (Conode *)Wander(WaitingCo, (WordNodeFnPtr) func(CheckPkt), tempport);
     if (result)
      {
        StartCo(result);
        continue;
      }

        /* A coroutine in PollingCo cannot receive another request, but if */
        /* that request happens to be a Close the last transaction must    */
        /* be aborted and replaced by the Close. */
      result = (Conode *)Wander(PollingCo,(WordNodeFnPtr) func(CheckPkt), tempport);
      if (result)
       { if ((mcb->MsgHdr.FnRc & FG_Mask) eq FG_Close)
         result->type = CoSuicide; 
         Request_Return(EC_Recover + SS_IOProc + EG_InUse + EO_Port,
                          0L, 0L);
         continue;
       }

      result = (Conode *)Wander(SelectCo, (WordNodeFnPtr) func(CheckPkt), tempport);
      if (result)
      { 
         result->type = CoAbortSelect;
         StartCo(result);   /* to abort the Select */
         
         /* One must hope that the coroutine has not died */
         /* I hope that I am correct in thinking it won't */
          
         result->type = 0;
         StartCo(result);   /* to handle the message */

         continue;
       }
                        /* coroutine does not exist */
      Request_Return(EC_Warn + SS_IOProc + EG_Unknown + EO_Port, 0L, 0L);
  }    /* forever */
}

#if ANSI_prototypes
PRIVATE void timeout_check (Conode * conode)
#else
PRIVATE void timeout_check(conode)
Conode *conode;
#endif
{ if (conode->timelimit < Now)
    { conode->type = CoTimeout;
      Debug(Timeout_Flag, ("Timeout in stream %s", &(conode->name[1])) );
      StartCo(conode);
    }
}

#if ANSI_prototypes
PRIVATE void polling_check (Conode * conode)
#else
PRIVATE void polling_check(conode)
Conode *conode;
#endif
{ if ((conode->type eq CoReady) || (conode->type eq CoSuicide))
    {
      StartCo(conode);
    }
}

             /* used to see whether a message is for an existing coroutine */
#if ANSI_prototypes
PRIVATE Conode * CheckPkt (Conode * conode, word id)
#else
PRIVATE Conode *CheckPkt(conode, id)
Conode *conode;
word id;
#endif
{ if (conode->id eq id)
    return(conode);
  else
    return((Conode *) false);
}

#if ANSI_prototypes
PRIVATE void list_debugging_options (void)
#else
PRIVATE void list_debugging_options()
#endif
{ int i;
  ServerDebug("Available debugging options are : a = all");
  for (i = 0; options_list[i].flagchar ne '\0'; i++)
   ServerDebug( (i % 4) eq 3 ? " %c : %16s" : "%c : %14s%q",
                  options_list[i].flagchar, options_list[i].name);
  ServerDebug("");
}

/*------------------------------------------------------------------------
--
--  A general purpose server for handling GSP requests.
--
------------------------------------------------------------------------*/

/**
*** The following code implements a general-purpose server suitable for all
*** devices and drives. This function always runs as a separate coroutine,
*** with its own stack, etc. When the Processor sends a message to this
*** server, the message is received from the link by the main Server coroutine
*** in routine main_loop(), and the server coroutine is reactivated.
***
*** The coroutine is started up by NewCo() in module cofuns.c when the
*** coroutine is created. Myco is the coroutine list node containing all
*** the required information, including a table of handler routines,
*** and I can call the specific initialisation routine for this server.
*** After this the server is ready to receive messages from the Processor,
*** so the coroutine suspends itself to wait for one.
***
*** When the server is woken up again, either the system is supposed to die
*** or a message has been received. It is impossible for servers to time out,
*** so I do not need to check for that. If the server got a message, it may
*** be a private protocol message, or a General Server Protocol or GSP message,
*** which can be checked by looking at some of the bits in the function code,
*** and the GSP function code has to be in the right range for a server, to
*** make sure that the server does not try to handle stream requests.
*** GSP requests which are outside the normal range are treated as private
*** protocol requests.
***
*** All GSP requests to a server involve a name of some sort, and it is
*** necessary to convert the Helios naming scheme to a local name. Ofcourse the
*** name supplied may be complete junk, so I have to allow for failure here.
***
*** Having passed all these hurdles, I can now call the handling routine
*** appropriate for the request, by indirecting through the table of handlers.
**/

#if ANSI_prototypes
PRIVATE void General_Server (Conode * myco)
#else
PRIVATE void General_Server (myco)
Conode *myco;
#endif
{ char   namebuf[32];
  int    handler_off;

  strcpy(namebuf, "no_");       /* if there is a line no_xxx in host.con */
  strcat(namebuf, myco->name);  /* do not create the server */
  if (get_config(namebuf) ne NULL)
   { Device_count--;
     Seppuku();
   }

  (*(myco->handlers)[InitServer_off])(myco);

  forever
   { Suspend();                      /* wait for the next request        */
     if (myco->type eq CoSuicide)    /* which may be an order to die        */
       { (*(myco->handlers)[TidyServer_off])(myco);
         Seppuku();
       }
                             /* The message may be part of a private protocol */
     if (((mcb->MsgHdr).FnRc & FC_Mask) ne FC_GSP)
       { (*((myco->handlers)[Private_off]))(myco);
         continue;
       }
                             /* Check the range of the GSP request */ 
     unless((FG_Open <= getfnrc(mcb)) && (getfnrc(mcb) <= FG_CloseObj))
      { (*((myco->handlers)[Private_off]))(myco);
        continue;
      }

                                       /* and protect us from Nick Garnett */
     mcb->Data[mcb->MsgHdr.DataSize] = '\0';

                                           /* work out the local name IOname  */
     unless(convert_name())
      { Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
        continue;
      } 
/**
*** At this stage the function code is known to be a GSP request between 0x10
*** for Open and 0xC0 for CloseObj, so I mask off the useless bits, and use the
*** remainder as an array index.
**/
    handler_off = ((int) (mcb->MsgHdr.FnRc & 0x000000F0L)) >> 4;
    (*(myco->handlers)[Open_off - 1 + handler_off])(myco);
   }
}

/*------------------------------------------------------------------------
--
--  A similar routine for handling streams.
--
------------------------------------------------------------------------*/

/**
*** The code below is very similar to the above General_Server(), but it
*** is for handling stream coroutines rather than servers. Whenever a
*** server receives a valid Open request, e.g. to open a file, it creates
*** a new coroutine for the corresponding Helios stream, which has its own
*** message port/coroutine identifier.
***
*** There are some differences. The main difference is that opening a stream
*** is not always possible, e.g. the server may have received a request to
*** open a non-existant file in read-only mode. Routine General_Stream() is
*** responsible for this, because it knows all about its coroutine nodes etc.
*** unlike the server. This works by calling an init_stream handler which
*** forms part of the usual array of functions. Ofcourse the program on the
*** Processor which attempted to Open the stream needs to be informed, so
*** it is the responsibility of General_Stream() to reply to the Open
*** request and it has to know the address of the Open message. Most of
*** the reply's data structure has been set up already by the server, on 
*** the assumption that the request was valid.
***
*** After initialisation the coroutine is ready to handle GSP stream
*** requests, so there is the usual Suspend() to suspend the coroutine.
*** Streams can time out, so I have to check the timeout flag as well
*** as the suicide flag. Also, tidying up the world is rather more
*** complicated for a stream than for a server as there are files to be
*** closed etc.
**/

#if ANSI_prototypes
PRIVATE void General_Stream (Conode * myco)
#else
PRIVATE void General_Stream(myco)
Conode *myco;
#endif
{ 
  word   temp;
  int    handler_off;

                              /* start up the stream using the Init routine */
  temp = ( (*( (WordConFnPtr)myco->handlers[InitStream_off])) (myco) );
  if (temp)
    { Request_Return(temp, 0L, 0L);     /* The Open failed for some reason */
      Seppuku();
    }
                             /* now the stream is ready for stream requests */
                             /* so I reply to the Open request              */
  Request_Return(ReplyOK, open_reply,
                 (word) strlen(mcb->Data) + 1L);

  forever
   { myco->timelimit   = Now + StreamTimeout;
     myco->type        = 0L; 
     Suspend();                         /* wait for the next request          */
     if (myco->type eq CoSuicide)       /* which may be an order to die       */
       { 
                                /* do a tidy up     */
         (*myco->handlers[TidyStream_off])(myco);
         Seppuku();
       }
     elif (myco->type eq CoTimeout)     /* or a timeout may have occurred     */
       { 
         if ((*((WordConFnPtr)myco->handlers[TidyStream_off]))(myco))
             continue;             /* Do not die off for some reason or other */
                                   /* e.g. because there is an event handler  */
         Seppuku();
       }
                             /* The message may be part of a private protocol */
#if internet_supported
     if (((mcb->MsgHdr).FnRc & FC_Mask) ne FC_GSP || 
            (FG_Listen <= getfnrc(mcb)) && (getfnrc(mcb) <= FG_RecvMessage))
#else
     if (((mcb->MsgHdr).FnRc & FC_Mask) ne FC_GSP)
#endif
       { (*((myco->handlers)[StreamPrivate_off]))(myco);
         continue;
       }

     unless((FG_Read <= getfnrc(mcb)) && (getfnrc(mcb) <= FG_Select))
      { Request_Return(EC_Error + SS_IOProc + EG_FnCode + EO_Message, 0L, 0L);
        continue;
      }

/**
*** At this stage the function code is known to be a GSP request between 0x1010
*** for Read and 0x10B0 for NegAck, so I mask off the useless bits, and use the
*** remainder as an array index.
**/
    handler_off = ((int) (mcb->MsgHdr.FnRc & 0x000000F0L)) >> 4;
    (*(myco->handlers)[Read_off - 1 + handler_off])(myco);
   }
}


/*------------------------------------------------------------------------
--------------------------------------------------------------------------
--
-- The IO processor device
--
--------------------------------------------------------------------------
------------------------------------------------------------------------*/

/**
*** The IOprocessor is a node in the Helios network. This means that it must
*** have a name, e.g. /IO. All devices are servers inside this network node.
*** Hence the network name of the console device starts off as /IO/console.
***
*** It is possible to perform a number of operations on the entire network node,
*** i.e. on the device IOproc. For example, you should be able to list the
*** node to find out all the servers supported by that node, and you can
*** rename the node when the network changes. Note that it is possible to
*** access a network node in two different ways: via its network name, or
*** via the links. Thus listing /IO and listing /00/Link.0 should have the
*** same effect. This complicates matters slightly for the Server, because
*** if you access /00/Link.0 there are no bits of the name left when the
*** message gets to me, whereas if you access /IO the name I get is IO. This
*** means I have to do a number of special checks.
***
*** The network node may receive distributed search requests for servers that
*** the rest of the network does not know about yet. These are sent only to
*** IOproc, not to any servers inside the network node. However, it is possible
*** for Helios to send server requests to IO when they are meant for some other
*** server inside IO, on the assumption that IOproc will forward them to the
*** appropriate server. 
**/
PRIVATE void fn( Forward, (void));
PRIVATE char network_name[IOCDataMax];

/**
*** Forward_handler() is a default handler for many IOproc requests. The request
*** is inappropriate if sent directly to IOproc, but it may be fine for another
*** server. Hence I need to check that it is for another server and forward it.
***
*** The routine always executes in the IOproc coroutine, so myco->name holds the
*** current local name.
**/

#if ANSI_prototypes
void Forward_handler (Conode * myco)
#else
void Forward_handler(myco)
Conode *myco;
#endif
{
                 /* Messages arriving at the IOproc device can come either by */
                 /* accessing the device or by accessing a link. If the latter*/
                 /* the local name is nothing. This has to be treated as a    */
                 /* special case.                                             */
  if (strcmp(IOname, myco->name) && IOname[0] ne '\0' )
    Forward();
  else
    Request_Return(EC_Error + SS_IOProc + EG_WrongFn + EO_Server, 0L, 0L);

use(myco)
}

/**
*** If a message arrives at the IOproc device that is intended for another
*** device it should be forwarded to that device. This only affects server
*** requests so the local name is something like IO/d/helios/bin/shell.
*** I need to extract the appropriate server, i.e. the server for drive D,
*** and work out which is the corresponding coroutine. This is handled by
*** do_search() which actually forms part of the distributed search mechanism.
*** Forwarding a message involves reactivating the server coroutine, since the
*** single global message buffer is shared. The coroutine will start
*** up inside General_Server() because it is waiting for a message. It will
*** perform a new name conversion automatically which will cause IOname to
*** become d/helios/bin/shell, and everything is fine. When the other
*** coroutine suspends itself this one is reactived, and the routine returns
*** into General_Server() where the coroutine suspends itself, reactivating
*** the main coroutine which goes around MainLoop again.
***
*** NOTE : it may be desirable to extend this, so that internal Servers can
*** forward messages e.g. to allow console/../helios/x as a valid name, or
*** even to forward messages to other bits of the network.
**/

PRIVATE word fn( do_search, (Conode *, char *));
PRIVATE void Forward()
{ register char *server_name, *server_end;
  Conode *tempco;

  for (server_name = IOname; *server_name ne '/'; server_name++);
  server_name++;

  for (server_end = server_name; *server_end ne '/' && *server_end ne '\0';
        server_end++ );

  *server_end = '\0';

  if ((tempco = (Conode *) Wander(WaitingCo, (WordNodeFnPtr)func(do_search), server_name) )
      ne (Conode *) false)
   if (tempco->id ne 1L)                   /* Cannot forward to IOPROC device */
    { 
      StartCo(tempco);
      return;
    }

   Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Message, 0L, 0L);
}

/**
*** The main thing that can be done with the IOproc device is listing it to
*** find out what servers are supported by the Server. This involves Open
*** requests and Reads of DirEntry. The servers supported are generally fixed,
*** so I can set up my directory structure when the IOproc device is given a
*** chance to initialise. This directory structure is the same as that used
*** for real directories in module files.c, which means that the code for
*** reading the IOproc directory is shared with the code for the reading of
*** file directories. All that is handled by the declarations in fundefs.h
***
*** Working out what servers exist is very easy, because the list WaitingCo
*** contains all the server coroutines and they are named in the Conode
*** structure. The IOproc device is the first to initialise itself, and via
*** some hacking in Init() above the extra field in each conode contains the
*** type of the device, i.e. file, directory or special.
***
*** InitServer() is also responsible for setting up the initial network name,
*** which is always /IO. The IOproc device may receive Rename requests from
*** the network server, which will affect this network name.
***
*** It is essential that the IOproc device has a constant message port even
*** across reboots, and this port is 1L. The configuration info sent in module
*** tload.c includes this port id.
**/

DirHeader IO_extra;

#if ANSI_prototypes
void IOPROC_InitServer (Conode * myco)
#else
void IOPROC_InitServer(myco)
Conode *myco;
#endif
{ Conode *curnode  = (Conode *) myco->node.next;
  char *proc_name  = get_config("io_processor");
  Heliosnode = (Node*)myco;
  if (proc_name ne (char *) NULL)         /* change the ioprocessors name */
   { char *ptr1, *ptr2;
     for (ptr1 = ptr2 = &(proc_name[1]); *ptr1 ne '\0'; ptr1++)
      if (*ptr1 eq '/')
       ptr2 = ++ptr1;
     strcpy(myco->name, ptr2);
     strcpy(network_name, proc_name);
   }
  else
   strcpy(network_name, slashDefaultServerName);  /* Initial network name */
 
  myco->id         = 1L;
  myco->extra      = (ptr) &IO_extra;
  InitList(&IO_extra.list);
  IO_extra.entries = 0L;

  for ( ; curnode->node.next ne (Node *) NULL;
          curnode = (Conode *) curnode->node.next)
   { ObjNode *newnode = (ObjNode *) malloc(sizeof(ObjNode));
     word type = (word) curnode->extra;

     if (newnode eq (ObjNode *) NULL)
       { ServerDebug("Insufficient memory to initialise IOProc device.");
/*         longjmp(exit_jmpbuf, 1); */
	 longjmp_exit;
       }

     memset((char *) newnode, 0, sizeof(ObjNode));
     newnode->direntry.Type     = swap(type);
     newnode->direntry.Flags    = swap(0L);
     newnode->direntry.Matrix   = swap( (type eq Type_Directory) ? 
                                       DefDirMatrix : DefFileMatrix  );
     strcpy(newnode->direntry.Name, curnode->name);

     AddTail(&(newnode->node), &IO_extra.list);
     IO_extra.entries++;
   }
}

#if ANSI_prototypes
void IOPROC_TidyServer(Conode * myco)
#else
void IOPROC_TidyServer(myco)
Conode *myco;
#endif
{ FreeList(&IO_extra.list);
  use(myco)
}

/*------------------------------------------------------------------------
--
--  code to handle distributed search requests.
--
------------------------------------------------------------------------*/

/**
*** All distributed search requests are sent to the IOproc device, using a
*** private protocol. Distributed search is the only private protocol
*** understood by the IOproc device, so that makes life slightly easier.
*** Essentially, when I get a search request the data vector contains a
*** C string identifying the server Helios is trying to get hold off.
*** Distributed searches are always for servers.
***
*** Needless to say there are a couple of complications. First of all, the
*** string may be rather complicated and I need to extract the bit I am
*** interested in. Unfortunately nobody has told me exactly what the naming
*** scheme is, so I have done some guess work on what I have to do. Secondly,
*** there is a debugging option to handle searches, so I may have to display
*** a message. Note that I only need to check WaitingCo, and not PollingCo,
*** because servers will never suspend themselves in a polling loop...
*** SelectCo is also out, because only streams can end up in SelectCo and
*** distributed searches are always for servers.
***
*** As for what I return to the processor, if I know about the name I send
*** back a message port for the server so that the server can get messages
*** from the processor, and this port is the unique coroutine identifier.
*** Otherwise I return a suitable error code. I am also meant to return a
*** full network name for the server.
**/

PRIVATE void fn( handle_search, (void));

#if ANSI_prototypes
void IOPROC_Private (Conode * myco)
#else
void IOPROC_Private(myco)
Conode *myco;
#endif
{ if (getfnrc(mcb) eq FG_Search)      /* check for distributed search */
    handle_search();          /* request */
  else
    Request_Return(EC_Error + SS_IOProc + EG_FnCode + EO_Message, 0L, 0L);

use(myco)
}

#if ANSI_prototypes
PRIVATE word do_search (Conode * conode, char * str)
#else
PRIVATE word do_search(conode, str)      /* function passed to list walking */
Conode *conode;
char   *str;
#endif
{ 
  if (!strcmp(conode->name, str))
    return((word) conode);
  else
    return(false);
}

/**
*** the data field of the MCB contains a string such as /net/IO/console.
*** I need to extract the last bit, and compare it with known servers, by
*** walking down my list of coroutines and comparing the names. If the
*** server is known, I have to check that the subnet address matches the
*** current name of the IO processor. For example, search strings might
*** be /console, /IO/console, /net/IO/console. If the current processor
*** name is /cluster/IO/console the first two strings would match, but
*** not the last.
**/
                   
PRIVATE void handle_search()
{ Conode *tempco;
  char *ptr1=mcb->Data, *ptr2;
  Conode *ioproc_co = (Conode *) NULL;

  for ( ptr2 = mcb->Data; *ptr1 ne '\0'; ptr1++)
   if (*ptr1 eq '/')
    ptr2 = ptr1 + 1;

  tempco = (Conode *) Wander(WaitingCo, (WordNodeFnPtr)func(do_search), ptr2);
  if (!tempco)
    { Debug(Search_Flag, ("Distributed search for %s : unknown server", mcb->Data));
      Request_Return(EC_Error + SS_IOProc + EG_Unknown + EO_Server, 0L, 0L);
      return;
    }

      /* the server is known, check the subnet address                */
      /* network_name is e.g. /Cluster/IO                             */
      /* I put a null at the start of the supplied                    */
      /* string, and start comparing that and network_name from the   */
      /* back. If I reach my null in the supplied string before I     */
      /* exhaust network_name, the network address is OK.             */
      /* there is a nasty to deal with /IO itself                     */
  mcb->Data[0] = '\0';

  if (tempco->id eq 1L)    /* suppose the IO processor is called /pc */
   { Conode *junkco;       /* and there is a request for /pc/pc      */
     char   junk = tempco->name[0];  /* I must pick up the 2nd one   */

     tempco->name[0] = '\0';
     junkco = (Conode *) Wander(WaitingCo, (WordNodeFnPtr)func(do_search), ptr2);
     tempco->name[0] = junk;
     if (junkco)                    /* there is a lower-level server */
      {                             /* with the right name */
        ioproc_co = tempco;
        tempco    = junkco;
      }
   }

retry:

  if (tempco->id eq 1L)    /* for /IO, move on a bit */
    for (; *ptr2 ne '\0'; ptr2++);

  ptr1 = ptr2 - 1;  /* this is the network bit of the supplied string */
  if (*ptr1 eq '/') ptr1--;

  for (ptr2 = network_name; *ptr2 ne '\0'; ptr2++);
  ptr2--;           /* This is now the last char in the network name */

  for ( ; (*ptr1 ne '\0') && (ptr2 >= network_name); ptr1--, ptr2--)
   if (*ptr1 ne *ptr2)
    goto wrong_subnet;

  if (*ptr1 ne '\0')  /* supplied name exceeds network address       */
                      /* e.g. /net/cluster/IO instead of /cluster/IO */
   goto wrong_subnet;

  mcb->Data[0] = '/';
  Debug(Search_Flag, ("Distributed search for %s : found, server is %lx", mcb->Data, (word) tempco->id));

  mcb->MsgHdr.Dest = tempco->id;
  strcpy(mcb->Data, network_name);
  if (tempco->id ne 1L)                  /* Send back the full network name */
   pathcat(mcb->Data, tempco->name);     /* special case for /IO */
  Request_Return(ReplyOK, 0L, (word) (strlen(mcb->Data) + 1) );
  return;

wrong_subnet:
  if (ioproc_co ne (Conode *) NULL)
   { tempco = ioproc_co;
     ioproc_co = (Conode *) NULL;
     goto retry;
   }

  mcb->Data[0] = '/';
  Debug(Search_Flag, ("Distributed search for %s : wrong network address", mcb->Data));
  Request_Return(EC_Error + SS_IOProc + EG_Unknown + EO_Route, 0L, 0L);
  return;
}

/*------------------------------------------------------------------------------
--
-- Other handlers for messages sent to the IOproc device
--
------------------------------------------------------------------------------*/

/**
*** This code is for both Locate and Create. If the message is for the IOproc
*** device I return the appropriate reply, otherwise I forward the message
*** to the appropriate server.
**/

#if ANSI_prototypes
void IOPROC_Locate (Conode * myco)
#else
void IOPROC_Locate(myco)
Conode *myco;
#endif
{ word temp;

  if (strcmp(IOname, myco->name) && IOname[0] ne '\0')
    Forward();
  else
   { IOname[0] = '\0';
     temp = FormOpenReply(Type_Directory, 0L, -1L, -1L);
     Request_Return(ReplyOK, open_reply, temp);
   }

 use(myco)
}

/**
*** Open requests for devices other than IOproc are forwarded, as usual, and
*** opening the IOproc device itself involves creating a new Stream coroutine
*** to handle Directory reads. Most of the IOPROC_handlers are the same as
*** the corresponding Dir_handlers. The extra field in the new stream's conode
*** structure is initialised to the directory list structure, so there is no
*** need for a separate InitStream.
**/

#if ANSI_prototypes
void IOPROC_Open (Conode * myco)
#else
void IOPROC_Open(myco)
Conode *myco;
#endif
{ if (strcmp(IOname, myco->name) && IOname[0] ne '\0')
    Forward();
  else
   { if ( ((mcb->Control)[OpenMode_off] & 0x0F) eq O_WriteOnly)
      { Request_Return(EC_Error + SS_IOProc + EG_WrongFn + EO_Server, 0L, 0L);
        return;
      }

     NewStream(Type_Directory, Flags_Closeable, (word) &IO_extra,
               IOPROC_Handlers);
   } 
}

/**
*** ObjectInfo's intended for sub-servers are forwarded, and an ObjectInfo
*** on the IOproc device is easy to support. This means that e.g. the server
*** for drive d has to be able to handle ObjectInfo requests on drive d,
*** but it is more likely to succeed at that than the IOproc device.
**/

#if ANSI_prototypes
void IOPROC_ObjectInfo (Conode * myco)
#else
void IOPROC_ObjectInfo(myco)
Conode *myco;
#endif
{ register ObjInfo *Heliosinfo = (ObjInfo *) mcb->Data;

  if (strcmp(IOname, myco->name) && IOname[0] ne '\0')
   { Forward();
     return;
   }

             /* The ObjectInfo information is in the Data vector so it does */
             /* not get swapped automatically by the message passing        */
             /* routines. All integers must be swapped explicitly. On the   */
             /* PC swap() is defined to be a no-op.                         */
  Heliosinfo->DirEntry.Type   = swap(Type_Directory);
  Heliosinfo->DirEntry.Flags  = swap(0L);
  Heliosinfo->DirEntry.Matrix = swap(DefDirMatrix);
  Heliosinfo->Size            = swap(0L);
  Heliosinfo->Account         = swap(0L);
  Heliosinfo->Creation        = swap(Startup_Time);
  Heliosinfo->Access          = swap(Startup_Time);
  Heliosinfo->Modified        = swap(Startup_Time);
  strcpy(Heliosinfo->DirEntry.Name, DefaultServerName);

  Request_Return(ReplyOK, 0L, (word) sizeof(ObjInfo));
  use(myco)
}

/**
*** IOPROC_Rename() is one of the most important operations on the IOPROC
*** device because that is how the network server renames my node in the
*** network. There are two stages. First, my local name i.e. IO may have
*** changed. Secondly my global network name may have changed.
***
**/

#if ANSI_prototypes
void IOPROC_Rename (Conode * myco)
#else
void IOPROC_Rename(myco)
Conode *myco;
#endif
{
#if SOLARIS
  char *newname = misc_buffer1;
#else
  char *newname = (char *) &(misc_buffer1[0]);
#endif
  char *data = mcb->Data;
  int  context = (int) mcb->Control[Context_off];
  int  dest    = (int) mcb->Control[RenameToname_off];

  if (strcmp(IOname, myco->name) && IOname[0] ne '\0')
   {
     Forward();

     return;
   }

  if (context ne -1 && data[dest] ne '/')
   { 
     strcpy(newname, &(data[context]));
     pathcat(newname, &(data[dest]));
   }
  else
    {
      strcpy(newname, &(data[dest]));
    }

  unless(flatten(newname))
    { Request_Return(EC_Error + SS_IOProc + EG_Name + EO_Server, 0L, 0L);

      return;
    }

  strcpy(network_name, newname);

  data = newname + strlen(newname) - 1;
 
  while (*data ne '/') data--;

  strcpy(myco->name, ++data);

  Request_Return(ReplyOK, 0L, 0L);

use(myco)
}

/**
*** I cannot use Dir_Close() here because that frees all the space taken up
*** by the linked list used to hold the directories.
**/

#if ANSI_prototypes
void IOPROC_Close (Conode * myco)
#else
void IOPROC_Close(myco)
Conode *myco;
#endif
{ Request_Return(ReplyOK, 0L, 0L);
  Seppuku();
  use(myco)
}

/*----------------------------------------------------------------------
--
-- Some general purpose handlers
--
----------------------------------------------------------------------*/

/**
*** The following routines are used as default request handlers with
*** particular servers, and are declared in header file fundefs.h as
*** the handler for that request for the server in question. For example,
*** the Rename request is illegal for the console server so the handler
*** Console_Rename, which is stored in the array of handlers for the
*** Console device in the Console server coroutine node, is hash-defined
*** to InvalidFn.
**/ 
                /* for requests that are not defined on a particular object */
#if ANSI_prototypes
void Invalidfn_handler(Conode *myco)
#else
void Invalidfn_handler(myco)
Conode *myco;
#endif
{ Request_Return(EC_Error + SS_IOProc + EG_WrongFn + EO_Server, 0L, 0L);
  use(myco)
}

                      /* for requests to devices that can be ignored */
#if ANSI_prototypes
void Dummy_handler(Conode *myco)
#else
void Dummy_handler(myco)
Conode *myco;
#endif
{ Request_Return(ReplyOK, 0L, 0L);
  use(myco)
}

word Ignore()                /* dummy initialisation routine */
{ return(0L);
}

#ifdef __cplusplus
void IgnoreVoid(Conode * c)
{ return;
  use(c);
}
#else
void IgnoreVoid()
{ return;
}
#endif

                               /* for create and locate requests on devices */
#if ANSI_prototypes
void Create_handler(Conode * myco)
#else
void Create_handler(myco)
Conode *myco;
#endif
{ word temp;

  temp = FormOpenReply(Type_File, 0L, -1L, -1L);
  Request_Return(ReplyOK, open_reply, temp);
  use(myco)
}

              /* ObjectInfo is straightforward for all devices except Clock */
              /* so the devices can share this ObjectInfo handler.          */
#if ANSI_prototypes
void Device_ObjectInfo_handler(Conode * myco)
#else
void Device_ObjectInfo_handler(myco)
Conode *myco;
#endif
{                       /* put info in data vector straightaway, nothing in */
                        /* there that we need any more */
  register ObjInfo *Heliosinfo = (ObjInfo *) mcb->Data;

             /* The ObjectInfo information is in the Data vector so it does */
             /* not get swapped automatically by the message passing        */
             /* routines. All integers must be swapped explicitly. On the   */
             /* PC swap() is defined to be a no-op.                         */
  Heliosinfo->DirEntry.Type   = swap(Type_File);
  Heliosinfo->DirEntry.Flags  = swap(0L);
  Heliosinfo->DirEntry.Matrix = swap(DefFileMatrix);
  Heliosinfo->Size            = swap(0L);
  Heliosinfo->Account         = swap(0L);
  Heliosinfo->Creation        = swap(Startup_Time);
  Heliosinfo->Access          = swap(Startup_Time);
  Heliosinfo->Modified        = swap(Startup_Time);
  strcpy(Heliosinfo->DirEntry.Name, IOname);

  Request_Return(ReplyOK, 0L, (word) sizeof(ObjInfo));
  use(myco)
}

void GetDefaultAttr(myco)
Conode *myco;
{ InitAttributes((Attributes *) mcb->Data);
  Request_Return(ReplyOK, 0L, (word) sizeof(Attributes));
  use(myco)
}

void Device_GetSize(myco)
Conode *myco;
{ (mcb->Control)[Reply1_off] = MAXINT;
  Request_Return(ReplyOK, 1L, 0L);
  use(myco)
}

void NullFn(myco)
Conode *myco;
{ Request_Return(ReplyOK, 0L, 0L);
  use(myco)
}

void Select_handler(myco)
Conode *myco;
{
  AddTail(Remove(&(myco->node)), SelectCo);
  Suspend();
  if ((myco->type eq CoSuicide) || (myco->type eq CoTimeout))
   Seppuku();
  AddTail(Remove(&(myco->node)), WaitingCo);
}

/**
*** Refine returns a new capability, which is a duff one but has the
*** access mask set correctly.
**/
void Refine_handler(myco)
Conode *myco;
{ Capability *cap = (Capability *) mcb->Data;
  word       access = mcb->Control[RefineAccessMask_off];

  cap->Access = (AccMask) access;

  Request_Return(ReplyOK, 0L, (word) sizeof(Capability));
  use(myco)
}

/**
*** Protect returns nothing
**/
void Protect_handler(myco)
Conode *myco;
{ Request_Return(ReplyOK, 0L, 0L);
  use(myco)
}

/**
*** These routines deal with servers that are directories rather than
*** files.
**/

ObjNode *Dir_find_node(myco)
Conode *myco;
{ List                  *list = (List *) myco->extra;
  register ObjNode *node;
  register char         *name = &(IOname[0]);

  for ( ; (*name ne '/') && (*name ne '\0'); name++);
  if (*name eq '\0')
    return((ObjNode *) NULL);

  *name = '\0';           /* check the server name */
  if (strcmp(IOname, myco->name))
    return((ObjNode *) NULL);

  *name++ = '/';          /* server fine, restore IOname */
  for (node = (ObjNode *) list->head;
       node->node.next ne (Node *) NULL;
       node = (ObjNode *) node->node.next)
   if (!strcmp(name, &(node->direntry.Name[0])))
     return(node);

   return((ObjNode *) NULL);
}

void Dir_TidyServer(myco)
Conode *myco;
{ FreeList((List *) myco->extra);
}

             /* Locate may be either for the server or for a device */
void Dir_Locate(myco)
Conode *myco;
{ word         temp;
  ObjNode      *node;
            
  if (!strcmp(IOname, myco->name))    /* is it for the server ? */
   { temp = FormOpenReply(Type_Directory, 0L, -1L, -1L); 
     Request_Return(ReplyOK, open_reply, temp);
   }
  elif ( (node = Dir_find_node(myco) ) eq (ObjNode *) NULL)
   Request_Return(EC_Error + SS_IOProc + EG_Unknown + EO_File, 0L, 0L);
  else
   { temp = FormOpenReply(Type_File, 0L, -1L, -1L);
     Request_Return(ReplyOK, open_reply, temp);
   }
}

void Dir_ObjInfo(myco)
Conode *myco;
{ ObjNode *node;
                       /* put info in data vector straightaway, nothing in */
                        /* there that we need any more */
  register ObjInfo *Heliosinfo = (ObjInfo *) mcb->Data;

             /* The ObjectInfo information is in the Data vector so it does */
             /* not get swapped automatically by the message passing        */
             /* routines. All integers must be swapped explicitly. On the   */
             /* PC swap() is defined to be a no-op.                         */
  Heliosinfo->DirEntry.Flags  = swap(0L);
  Heliosinfo->DirEntry.Matrix = swap(DefFileMatrix);
  Heliosinfo->Size            = swap(0L);
  Heliosinfo->Account         = swap(0L);
  Heliosinfo->Creation        = swap(Startup_Time);
  Heliosinfo->Access          = swap(Startup_Time);
  Heliosinfo->Modified        = swap(Startup_Time);
             
  if (!strcmp(IOname, myco->name))    /* is it for the server ? */
   { Heliosinfo->DirEntry.Type   = swap(Type_Directory);
     Heliosinfo->DirEntry.Matrix = swap(DefDirMatrix);
     strcpy(Heliosinfo->DirEntry.Name, myco->name);
     Request_Return(ReplyOK, 0L, (word) sizeof(ObjInfo));
   }
  elif ( (node = Dir_find_node(myco) ) eq (ObjNode *) NULL)
   Request_Return(EC_Error + SS_IOProc + EG_Unknown + EO_File, 0L, 0L);
  else
   { Heliosinfo->DirEntry.Type   = swap(Type_File);
     strcpy(Heliosinfo->DirEntry.Name, node->direntry.Name);
     Heliosinfo->Size            = swap(node->size);
     Heliosinfo->Account         = swap(node->account);
     Request_Return(ReplyOK, 0L, (word) sizeof(ObjInfo));
   }
}

/**
*** Here are some routines to store numbers of ObjNodes in larger
*** buffers. This is particularly useful when reading large directories,
*** 200 entries or more, where malloc'ing 200 small lumps of memory is
*** inefficient. Also, when using own memory management this may cause
*** the small heap to run out of space.
**/
PRIVATE List nodes_list;
PRIVATE int  nodes_list_initialised = 0;
#define MaxNodesPerBuffer   10
#define node_magic_size 0x654321fe

typedef struct nodes_buffer {
        Node         node;
        int          free;
        ObjNode      nodes[MaxNodesPerBuffer];
} nodes_buffer;

PRIVATE word find_free(buf)
nodes_buffer *buf;
{ int i;
  if (buf->free eq 0)
   return(NULL);
  buf->free--;
  for (i = 0; i < MaxNodesPerBuffer; i++)
   if (buf->nodes[i].size eq node_magic_size)
    { buf->nodes[i].size = 0;
      return((word) &(buf->nodes[i]));
    }
  buf->free++;
  return(NULL);
}

ObjNode *NewObjNode()
{ ObjNode *result; 
  if (!nodes_list_initialised)
   { InitList(&nodes_list); nodes_list_initialised++; }

  result = (ObjNode *) Wander(&nodes_list, (WordNodeFnPtr)func(find_free));
  if (result ne (ObjNode *) NULL)
   return(result);

  { nodes_buffer *newbuf = (nodes_buffer *) malloc(sizeof(nodes_buffer));
    int i;
    if (newbuf eq (nodes_buffer *) NULL)
     return((ObjNode *) NULL);
    for (i = 1; i < MaxNodesPerBuffer; i++)
     newbuf->nodes[i].size = node_magic_size;
    newbuf->nodes[0].size = 0L;
    newbuf->free = MaxNodesPerBuffer - 1;
    AddTail(&(newbuf->node), &nodes_list);
    return(&(newbuf->nodes[0]));
  }
}

PRIVATE word find_node_to_free(buf, node)
nodes_buffer *buf;
ObjNode      *node;
{ int i;

  for (i = 0; i < MaxNodesPerBuffer; i++)
   if (&(buf->nodes[i]) eq node)
    { buf->nodes[i].size = node_magic_size;
      buf->free++;
      if (buf->free eq MaxNodesPerBuffer)
       free(Remove(&(buf->node)));
      return(1);
    }
  return(0);
}

void FreeObjNode(node)
ObjNode *node;
{ if (Wander(&nodes_list, (WordNodeFnPtr)func(find_node_to_free), node) eq 0)
   ServerDebug("Internal error : supposed to free unknown ObjNode.");
}

/*----------------------------------------------------------------------------
--
--  Name conversion
--
----------------------------------------------------------------------------*/

/**
*** Name conversion is non-trivial. The technical manual describes how the
*** Helios name is packed inside my message structure, and the code below
*** extracts the local name, i.e. the bit relevant to me, from all the junk.
*** It starts by extracting the necessary bits from the control vector and
*** giving a debugging message.
***
*** The first bit of code goes through the names, starting at pointer rest which
*** may be in either the context or the name fields, and puts all the data into
*** the array IOname. I try to update the next field in the message in case the
*** message has to be passed from one server to another, i.e. if another name
*** conversion may be required. This bit of code is rather dubious. It may be
*** necessary to extract bits of the name from the name field as well, in case
*** the rest pointer was still somewhere inside context. Having got hold of
*** the whole name I put in a terminator.
***
*** Sadly IOname may still contain elements . and .. which have to be filtered
*** out. Hence I must go through my entire local name again looking for these
*** special cases, and this is done by routine flatten() which is also called
*** by Rename handlers. Only now am I finished, and I can put in another
*** terminator just in case and produce another debugging option. It is
*** possible for the name conversion to fail if there is an attempt to
*** backtrack to a server outside the Server, as I refuse to forward messages
*** to other bits of the network.
**/

word convert_name()
{ byte *data     = mcb->Data;
  word *control  = mcb->Control;         /* the control vector contains the */
                                         /* offsets needed for the conversion */
  int  context   = (int) control[Context_off];
  int  name      = (int) control[Pathname_off];
  register int  	next = (int) control[Nextname_off];
  register char *	dest = IOname;

  if ((context eq -1) && (name eq -1))
   { Debug(Name_Flag, ("System library error : NULL context and NULL name"));
     return(false);
   }

  if (name > IOCDataMax)
    {
      Debug (Name_Flag, ("System library error: name offset %d too large", name));

      return (false);
    }

   Debug(Name_Flag, ("Context %s, name %s, rest %s", (context ne -1) ? &(data[context]) : "NULL", (name ne -1) ? &(data[name]) : "NULL", (next ne -1) ? &(data[next]) : "NULL" ) );

  for ( ; data[next] ne '/' && data[next] ne '\0'; next++)
    *dest++ = data[next];

  if (data[next] eq '/')
    { control[Nextname_off] = next+1;
      for ( ; data[next] ne '\0'; next++)
        *dest++ = data[next];
    }
  elif (name ne -1)
    if ( ((next < name) && (context < name)) ||
         ((next > name) && (context > name)) )
      control[Nextname_off] = name;

  if (name eq -1) goto finished;

  if ( ((next < name) && (context < name)) ||
       ((next > name) && (context > name)) )
   { *dest++ = '/';
     for ( ; data[name] ne '\0'; name++)
       *dest++ = data[name];
   }

finished:

  *dest = '\0';

  if (!flatten(IOname))
   { ServerDebug ("Name conversion failed.");
     return(false);
   }

  Debug(Name_Flag, ("IOname is %s", IOname));

  return(true);
}

word flatten(name)
char *name;
{ register char *source = name, *dest = name;
  int  entries = 0;

  while(*source ne '\0')
   { if (*source eq '.')
       { source++;
         if   (*source eq '/') { source++; continue; }
         elif (*source eq '\0')
           { if (entries < 1) return(false);
             dest--; break;
           }
         elif (*source eq '.')
           { source++;
             if (*source eq '/' || *source eq '\0')
               { 
                 if (entries <= 1) return(false);
                 dest--; dest--; while (*dest ne '/')  dest--;
                 if (*source ne '\0') 
                  { dest++; source++; }
                 entries--; continue;
               }
             else
               { *dest++ = '.'; *dest++ = '.'; }
           }
         else *dest++ = '.';
       }

     while (*source ne '/' && *source ne '\0') *dest++ = *source++;
     if (*source ne '\0') { *dest++ = '/'; source++; entries++; }
   }

  *dest = '\0';

  return(true);
}

/*------------------------------------------------------------------------
--
-- Here are some other odds and ends needed by the server
--
------------------------------------------------------------------------*/

/**
*** The following routines provide miscellaneous utilities.
***
*** FormOpenReply() is sets up the reply to Open, Create, and Locate requests,
*** which is a non-trivial job.
***
*** mystrcmp() does a string comparison which ignores differences in case.
*** The server is now case-sensitive, so it is not used as often, but the
*** configuration info in host.con is still case-independent.
*** It returns 0 if the strings are the same, non-zero otherwise, as does
*** C strcmp().
***
*** The swap routine is used for any machine with a different byte-ordering
*** from the processor - if the byte ordering is the same swap is #define'd
*** as a no-op. The current swap routine is suitable for the 68000.
***
*** Clear_bytes() clears an area of memory, as I do not trust memclr on all
*** the hardware I use.
***
*** NewStream() is used by the device servers to create new streams.
***
*** pathcat() is used to form complete pathnames, taking care of the /'s
*** etc
**/



/*----------------------------------------------------------------------------
--
-- FormOpenReply()
--
----------------------------------------------------------------------------*/

/**
*** This utility routine sets up a reply to Open, Locate and Create requests.
*** All the information comes as arguments, or is available in network_name
*** The routine returns the size of the data vector.
***
*** See header file iogsp.h for details of the returned data structure.
**/

word FormOpenReply(type, flags, cap1, cap2)
word type, flags, cap1, cap2;
{ IOCReply1 *reply = (IOCReply1 *) mcb->Control;

  reply->Type         = type;
  reply->Flags        = flags;
  if (type eq Type_File)
   { reply->Access[0] = (byte) 
        (AccMask_R+AccMask_W+AccMask_D+AccMask_A+AccMask_E);
     memset(&(reply->Access[1]), 0, 7);
   }
  else
   memset(&(reply->Access[0]), -1, 8);
  
  reply->Pathname     = 0L;     /* offset for pathname */
  reply->Object       = 0L;     /* object value if no reply port ? */

  strcpy(mcb->Data, network_name);
  if (IOname[0] ne '\0')
   pathcat(mcb->Data, IOname);

  Debug( OpenReply_Flag, ("FormOpenReply : name %s", mcb->Data));
  Debug( OpenReply_Flag, ("              : type %lx, flags %lx, access %x", type, flags, (reply->Access[0] & 0x00FF)) );

#if swapping_needed
     /* trust NHG to pass byte arrays in a vector of words */
  mcb->Control[2] = swap(mcb->Control[2]);
  mcb->Control[3] = swap(mcb->Control[3]);
#endif

  return((word) (strlen(mcb->Data) + 1) );
  use(cap1)
  use(cap2)
}


word mystrcmp(ms1, ms2)
char *ms1, *ms2;
{ register char *s1 = ms1, *s2 = ms2; 
  for (;;)
   { if (*s1 eq '\0')
       return((*s2 eq '\0') ? 0L : -1L);
     elif (*s2 eq '\0')
       return(1L);
     elif(ToUpper(*s1) < ToUpper(*s2))
       return(-1L);
     elif(ToUpper(*s1) > ToUpper(*s2))
       return(1L);
     else
       { s1++; s2++; }
   }
}

void pathcat(s1, s2)
char *s1, *s2;
{ register char *temp;
  for ( temp = s1; *temp ne '\0'; temp++);  /* get to end of s1 */
  if (*(--temp) ne '/')
   *(++temp) = '/';                         /* ensure / at end */
  *(++temp)='\0';
            /* cat s2, skipping initial / if necessary */
  strcat(temp, (*s2 eq '/') ? &(s2[1]) : s2);
}

#if swapping_needed
#if (ST || TRIPOS || SUN3 || SUN4 || SM90 || TR5 || RS6000 || HP9000)
                  /* for the 68000's or SPARC's byte ordering */
                  /* on the Amiga it is written in assembler */
word swap(a)
word a;
{ register word b = 0;
  int i;

  if (a eq 0L)
  {
    return(0L);
  }

  for (i=0; i<4; i++)
    { b <<= 8; b |= (a & 0xFF); a >>= 8; }

  return(b);
}
#endif     /* 68000 || SPARC */
#endif

/**
*** This is called by device servers e.g. Console or Mouse to open a new stream,
*** which involves creating a stream coroutine. The routine is given the message
*** containing the open request, which must be filled in correctly so that it
*** can be returned by General_Stream(). The necessary information is provided
*** as arguments. Creating the coroutine is straightforward and independent of
*** of the device, apart from the handlers for Read etc. requests which are
*** provided as an argument.
***
*** For certain devices the conode extra field must be filled in, so this is
*** another argument.
***
*** The new stream needs a new message port / unique coroutine id, which must
*** be sent back to the processor. This is done by filling in the dest field
*** of the message header. Request_Return() is responsible for swapping the
*** source and destination ports, so it has to be in dest.
**/

void NewStream(type, flags, extra, handlers)
word extra;
word type, flags;
VoidConFnPtr *handlers;
{ word   temp;
  Conode *newco;

  temp = FormOpenReply(type, flags, -1L, -1L);

  newco = NewCo(General_Stream);
  unless(newco)
    { Request_Return(EC_Warn + SS_IOProc + EG_NoMemory + EO_Server, 0L, 0L);
      return;
    }

  PostInsert(&(newco->node), Heliosnode);
  newco->id        = CoCount++;
  newco->timelimit = Now + StreamTimeout;
  newco->name[0]   = 0x7f;
  strcpy(&(newco->name[1]), IOname);
  newco->handlers  = handlers;  
  newco->extra     = (ptr) extra;
  mcb->MsgHdr.Dest = newco->id;

  StartCo(newco);
}

/*------------------------------------------------------------------------
--
-- Routines to manipulate attributes
--
------------------------------------------------------------------------*/

/**
*** The various routines below are used to examine and change device
*** stream attributes, and are the same as in syslib.c apart from problems
*** with 16/32 bit integers.
**/
                /* In an Attribute, the bottom two bits indicate which word */
                /* in the Attributes structure we are interested in, and    */
                /* the rest of the word specifies a single bit.             */
word IsAnAttribute(attr, item)
Attributes *attr;
Attribute item;
{ word *a_ptr = (word *) attr + (item & 0x00000003L);
  item     &= 0xFFFFFFFCL;
  return( *a_ptr & item);
}

void AddAttribute(attr, item)
Attributes *attr;
Attribute item;
{ word *a_ptr = (word *) attr + (item & 0x00000003L);
  item     &= 0xFFFFFFFCL;
  *a_ptr     |= item;
}

void RemoveAttribute(attr, item)
Attributes *attr;
Attribute item;
{ word *a_ptr = (word *) attr + (item & 0x00000003L);
  item     &= 0xFFFFFFFCL;
  *a_ptr     &= ~item;
}


                     /* The input and output speeds of a stream are kept in */
                     /* the bottom byte of the relevant Attributes field.   */
                     /* thus allowing upto 256 different speeds.            */
word GetInputSpeed(attr)
Attributes *attr;
{ return((attr->Input) & 0x000000FFL);
}

word GetOutputSpeed(attr)
Attributes *attr;
{ return((attr->Output) & 0x000000FFL);
}

void SetInputSpeed(attr, speed)
Attributes *attr;
word speed;
{ 
  attr->Input  &= 0xFFFFFF00L;  /* clear bottom byte */
  speed        &= 0x000000FFL;  /* prevent nasty crash if speed invalid */
  attr->Input  |= speed;
}

#if ANSI_prototypes
void SetOutputSpeed(Attributes *attr, word speed)
#else
void SetOutputSpeed(attr, speed)
Attributes *attr;
word speed;
#endif
{ 
  attr->Output &= 0xFFFFFF00L;  /* clear bottom byte */
  speed        &= 0x000000FFL;  /* prevent nasty crash if speed invalid */
  attr->Output |= speed;
}

#if ANSI_prototypes
void InitAttributes(Attributes * pointer)
#else
void InitAttributes(pointer)
Attributes *pointer;
#endif
{ register int i;
  register word *wpointer = (word *) pointer;
  for (i=0; i < 5; i++)
    *wpointer++ = 0L;
}

#if ANSI_prototypes
void CopyAttributes(Attributes * to_ptr, Attributes * from_ptr)
#else
void CopyAttributes(to_ptr, from_ptr)
Attributes *to_ptr, *from_ptr;
#endif
{ int i;
  register word *wto_ptr, *wfrom_ptr;
  wto_ptr = (word *) to_ptr; wfrom_ptr = (word *) from_ptr;
  
  for (i=0; i < 5; i++)
    *wto_ptr++ = swap(*wfrom_ptr++);
}



