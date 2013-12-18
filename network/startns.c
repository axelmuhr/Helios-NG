/*{{{  Header */

/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990 - 1994, Perihelion Software Ltd.      --
--                        All Rights Reserved.                          --
--                                                                      --
-- startns.c								--
--                                                                      --
--	Purpose : to start up the Helios networking software		--
--                                                                      --
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/startns.c,v 1.13 1994/03/14 11:01:26 nickc Exp $*/

/**
*** This program is usually executed from within the initrc file. This
*** means that memory fragmentation must be considered. The layout at
*** the top of memory is likely to be:
*** trace vector + nucleus data areas
*** init program  - does not use libraries
*** startns       - uses Posix and Clib
*** Posix library
*** C library
*** Resource Management library
*** Network Server
*** Session Manager
***
*** This means that there will be about 10-30K of fragmentation near the
*** top of memory previously occupied by init and startns, probably
*** unavoidable. Hopefully this area will be reused by other software.
***
*** In a single user environment startns may also be executed by the
*** user simply as a shell command or from within a shell script.
*** In such an environment it may be desirable to terminate some or all
*** of the networking software to leave more space for applications,
*** restarting the software when necessary.
**/

/*}}}*/
/*{{{  Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <message.h>
#include <syslib.h>
#include <nonansi.h>
#include <root.h>
#include <posix.h>
#include "private.h"
#include "exports.h"
#include "netutils.h"
#include "rmlib.h"

/*}}}*/
/*{{{  Forward References */

/**
*** A few forward declarations
**/
static	void		read_Nsrc(char *);
static	Object		*locate_root_processor(char *);
static	void		start_SessionManager(Object *);
static	void		check_clashes(Object *, bool, bool);
static	void		usage(void);
	void		fatal(void);
#ifndef SingleProcessor
static	int		count_real_processors(void);
static	void		start_NetworkServer(Object *);
static	Capability	send_resource_map(Object *);
extern	void		read_resource_map(char *);
/*static	void check_resource_map(RmNetwork, RmTaskforce);*/
#endif

/*}}}*/
/*{{{  Variables */

/**
*** This array is used to hold the nsrc options in the form of
*** environment strings. It is also used as the argument vector for the
*** network server and session manager, so the first few slots are taken
*** up by arguments.
**/
#define MaxNsrc_options 64
static string Nsrc_env[MaxNsrc_options];
#define FirstNsrc	4

/**
*** Name of the program, referred to in readmap.c.
**/
char	*ProgramName = "Startns";

/**
*** These are used to hold the network resource map and its
*** associated task force, if any. This data has to be sent to
*** the Network Server once it is up and running.
**/
#ifndef SingleProcessor
	RmNetwork   Network  	= (RmNetwork)NULL;
	RmTaskforce Taskforce	= (RmTaskforce)NULL;
#endif
static bool	do_reset	= 0;
static bool	silent_mode	= 0;
static char	DebugString[27];

/**
*** This is the basic capability for the Network Server. In a protected
*** environment this capability is passed on to the Session Manager
*** which passes it to all Taskforce Managers. Hence system software can
*** request things from the Network Server, but application programs
*** cannot.
**/
static Capability	NetworkCap = { 0, 0, 0, 0, 0, 0, 0, 0 };

/**
*** Used for environment processing to pass on to the Network Server and
*** Session Manager.
**/
static Environ		*my_environ;

/*}}}*/
/*{{{  Code */

/*{{{  main() */

/**
*** Main(), process the arguments, read the nsrc file and the resource
*** map, and start up the servers.
**/
int main(int argc, char **argv)
{ char *nsrc_file       = Null(char);
  char *resource_map     = Null(char);
  int run_NetworkServer  = 1;
  int run_SessionManager = 1;
  Object *root_processor = Null(Object);
  char   *root_name	 = Null(char);  

#ifndef SingleProcessor
  RmProgram = Rm_Startns;
#endif

/**
*** Test all the arguments:
***
*** -r    : reset the network
*** -nosm : skip the Session Manager
*** -nons : skip the Network Server
*** -q	  : silent mode for network server
*** -d    : by itself, all debugging options except comms
***         otherwise as follows:
***         b bootstrap
***	    e execute
***	    a allocate
***         r release
***         m monitor
***	    p problems suspected
***	    l links
***	    i initialise
***	    n native
***	    c comms
*** nsrc = filename : specify the network resource file
*** map   : a network resource map
**/

  strcpy(DebugString, " ");

  for ( argv++, argc--; argc-- > 0; argv++)
   { char *current_arg = *argv;

     if (current_arg[0] eq '-')
      { if (!mystrcmp(current_arg, "-r"))
         { if (do_reset)
            puts("Startns: warning, repeated argument -r.");
           do_reset = 1;
           continue;
         }
        if (!mystrcmp(current_arg, "-nosm"))
         { if (!run_SessionManager)
            puts("Startns: warning, repeated argument -nosm.");
           run_SessionManager = 0;
           continue;
         }
        if (!mystrcmp(current_arg, "-nons"))
         { if (!run_NetworkServer)
            puts("Startns: warning, repeated argument -nons.");
           run_NetworkServer = 0;
           continue;
         }
        if (!mystrcmp(current_arg, "-q"))
         { if (silent_mode)
            puts("Startns: warning, repeated argument -q.");
           silent_mode = TRUE;
           continue;
         }
        if ((current_arg[1] eq 'd') || (current_arg[1] eq 'D'))
         { if (current_arg[2] eq '\0')
            strcpy(DebugString, "abdefghijklmnopqrstuvwxyz");
           else
            strcpy(DebugString, &(current_arg[2]));
           continue;
         }
         
        printf("Startns: unknown option %s.\n", current_arg);
        usage();
      }
      
       /* check for "nsrc=name", or "nsrc" "=name", "nsrc" "=" "name" */
       /* or even "nsrc=" "name" or plain "nsrc" "name" */
     if (strlen(current_arg) >= 4)
      { char  temp = current_arg[4];  /* either '=' or '\0' */
        const char *main_error = "Startns: missing nsrc filename.";
        
        current_arg[4] = '\0';
        if (!mystrcmp(current_arg, "nsrc"))
         { if (nsrc_file ne Null(char))
            { printf("Startns: two nsrc files supplied, %s and %s.\n",
               nsrc_file, current_arg);
              usage();
            }
           if (temp eq '=')
            { if (current_arg[5] ne '\0')  /* "nsrc=name" */
               { nsrc_file = &(current_arg[5]);
                 continue;
               }
              if (argc > 0)
               { nsrc_file = *(++argv);     /* "nsrc=" "name" */
                 argc--;
                 continue;
               }
              else
               { puts(main_error);
                 usage();
               }
            }     	
           if (temp ne '\0')
            { printf("Startns: unknown argument %s.\n", current_arg);
              usage();
            }
           if (argc < 1)
            { puts(main_error);
              usage();
            }
           current_arg = *(++argv); argc--;
           if (current_arg[0] ne '=')    /* "nsrc" "name" */
            { nsrc_file = current_arg;
              continue;
            }
           if (current_arg[1] ne '\0')   /* "nsrc" "=name" */
            { nsrc_file = &(current_arg[1]);
              continue;
            }
           if (argc < 1)
            { puts(main_error);
              usage();
            }
           nsrc_file = *(++argv);  /* "nsrc" "=" "name" */
           argc--;
           continue;
         }
        else
         current_arg[4] = temp;
      }

       /* check for "root=name", or "root" "=name", "root" "=" "name" */
       /* or even "root=" "name" or plain "root" "name" */
     if (strlen(current_arg) >= 4)
      { char  temp = current_arg[4];  /* either '=' or '\0' */
        const char *main_error = "Startns: missing root processor name.";
        
        current_arg[4] = '\0';
        if (!mystrcmp(current_arg, "root"))
         { if (root_name ne Null(char))
            { printf("Startns: two root processors given, %s and %s.\n",
               root_name, current_arg);
              usage();
            }
           if (temp eq '=')
            { if (current_arg[5] ne '\0')  /* "root=name" */
               { root_name = &(current_arg[5]);
                 continue;
               }
              if (argc > 0)
               { root_name = *(++argv);     /* "root=" "name" */
                 argc--;
                 continue;
               }
              else
               { puts(main_error);
                 usage();
               }
            }     	
           if (temp ne '\0')
            { printf("Startns: unknown argument %s.\n", current_arg);
              usage();
            }
           if (argc < 1)
            { puts(main_error);
              usage();
            }
           current_arg = *(++argv); argc--;
           if (current_arg[0] ne '=')    /* "root" "name" */
            { root_name = current_arg;
              continue;
            }
           if (current_arg[1] ne '\0')   /* "root" "=name" */
            { root_name = &(current_arg[1]);
              continue;
            }
           if (argc < 1)
            { puts(main_error);
              usage();
            }
           root_name = *(++argv);  /* "root" "=" "name" */
           argc--;
           continue;
         }
        else
         current_arg[4] = temp;
      }

         /* All other possibilities having been eliminated, this argument */
         /* must be the resource map. */
     if (resource_map ne Null(char))
      { printf("Startns: two resource maps supplied, %s and %s.\n",
               resource_map, current_arg);
        usage();
      }
     resource_map = current_arg;
     continue;
     
   }  /* end of argument processing */

  my_environ = getenviron();

/**
*** There is no point in running startns if neither the Network Server nor
*** the Session Manager should be started.
**/
  if (!run_SessionManager && !run_NetworkServer)
   { puts(
     "Startns: both -nosm and -nons options given, there is nothing to do.");
     usage();
   }
   
/**
*** Put in some default values for various file names.
**/
  if (nsrc_file eq Null(char))
   nsrc_file = "/helios/etc/nsrc";
  if (resource_map eq Null(char))
   resource_map = "/helios/etc/default.map";

/**
*** Read in the nsrc file, to get defaults for the net directory and so on.
*** The nsrc file is the main configuration file for the networking software.
**/
  read_Nsrc(nsrc_file);

/**
*** always read in the resource map, even if the network server should not
*** be run. It may be necessary to use the resource map info to determine
*** the root processor on which to run the session manager.
**/
#ifndef SingleProcessor
  read_resource_map(resource_map);
  { int x;

    x = count_real_processors();

#ifdef Limit20
    if (x > 20)
     { fputs("Startns: this version is limited to 20 processors.\n", stderr);
       fatal();
     }
#endif
#ifdef Limit64
    if (x > 64)
     { fputs("Startns: this version is limited to 64 processors.\n", stderr);
       fatal();
     }
#endif
  }
#endif
  
/**
*** Try to locate a root processor. For most networks this is quite easy,
*** but if there are multiple I/O processors in the network things can
*** get confusing. Of course if there are multiple I/O processors then
*** startns should only run during the initial bootstrap, but that
*** assumes too much good behaviour by the user. In particular, using the
*** initrc file it is very possible that the user has already booted up
*** a couple of processors and is running startns remotely because there
*** is insufficient memory on the real root processor.
**/
  root_processor = locate_root_processor(root_name);

  if (root_processor eq Null(Object))
   { char buf[64];
     MachineName(buf);
     printf("Startns: warning, failed to identify the root processor.\n");
     root_processor = Locate(Null(Object), buf);
     if (root_processor ne Null(Object))
      printf("Startns: defaulting to current processor %s\n", 
      		root_processor->Name);
     else
      { printf("Startns: failed to locate own processor %s\n", buf);
        fatal();
      }
   }

/**
*** Check for any clashes, e.g. an existing Network Server or Session Manager.
*** The last thing I want is to have several different Network Servers
*** up and running, quite possibly on different processors. However it is
*** possible that the current network is connected to another one that has
*** its own Network Server, which is perfectly legal. Problems galore.
**/
  check_clashes(root_processor, run_NetworkServer, run_SessionManager);

/**
*** Finally, start up the specified servers. If at all possible, the
*** Network Server and Session Manager should be started up on the
*** root processor - whichever one that is. 
**/
#if 0
#ifndef SingleProcessor
  PrintNetwork(Network);
#endif
#endif

#ifndef SingleProcessor
  if (run_NetworkServer)
   { start_NetworkServer(root_processor);
     NetworkCap = send_resource_map(root_processor);
   }
#endif

  if (run_SessionManager)
   start_SessionManager(root_processor);

/**
*** Totally redundant, the space will be freed anyway.
**/
  if (root_processor ne Null(Object)) Close(root_processor);

/**
*** And startns has finished
**/
  return(EXIT_SUCCESS);   
}

/*}}}*/
/*{{{  usage() */

/**
*** Various error handling routines.
**/
static void usage(void)
{
  puts(
  "Usage: startns [-r] [-q] [-d[abeilmprc]] [-nons] [-nosm] [nsrc = filename] [resource map]");
  fatal();
}

/*}}}*/
/*{{{  fatal() */

void fatal(void)
{  
  puts("Startns: aborting start-up of the networking software.");
  exit(EXIT_FAILURE);
} 

/*}}}*/
/*{{{  add_config() */

/**
*** Read in the nsrc file. Much of this code has been lifted from the 
*** I/O Server The strings resemble C environment strings. The following bits
*** of code are responsible for reading in the configuration.
*** The file netutils.c contains the get_config() and get_int_config()
*** routines.
**/

/**
*** Routine add_config() adds a new configuration option. The options
*** are not held in a linked list, unlike the I/O Server. Instead
*** I need to pass the options in the set of environment strings,
*** so they must be held in a simple table. The options are all
*** prefixed by the character ~, to distinguish them from any other
*** environment string.
**/

PRIVATE void add_config(option)
char *option;
{ static int next_nsrcOption = FirstNsrc;
  char   *newstring;
  
  if (next_nsrcOption eq (MaxNsrc_options - 1))
   { puts("Startns: too many nsrc options have been given.\n");
     fatal();
   }

  newstring = malloc(strlen(option) + 2);  /* one for ~, one for \0 */
  if (newstring eq Null(char))
   { printf("Startns: out of memory in routine add_config\n");
     fatal();
   }
  newstring[0] = '~';
  strcpy(&(newstring[1]), option);
  Nsrc_env[next_nsrcOption++] = newstring;
}

/*}}}*/
/*{{{  read_Nsrc() */

/**
*** This routine reads in the whole nsrc file, line by line, and
*** processes it.
**/
static void read_Nsrc(char *filename)
{ FILE   *nsrc;
  char   buff[256];

  memset(Nsrc_env, 0, sizeof(char *) * MaxNsrc_options);
  
  if (filename[0] eq '/')   /* absolute filename */
   nsrc = fopen(filename, "r");
  else
   { strcpy(buff, "/helios/etc/");   /* relative to /helios/etc */
     strcat(buff, filename);
     nsrc = fopen(buff, "r");
    }

  if (nsrc eq Null(FILE))
   { printf("Startns: unable to open nsrc file %s\n", filename);
     fatal();
   }
               /* read in the file, one line at a time */
  while(fgets(buff, 255, nsrc) ne NULL)
   { char   *line = buff;
     int    length;

     while (isspace(*line)) line++;
     length = strlen(line) - 1;
     if (length < 0) continue;     /* blank line */
     if (line[0] eq '#') continue;  /* comment */
             /* fgets() leaves a newline character in the buffer */
     if ((line[length] eq '\n') || (line[length] eq '\r'))
       line[length] = ' ';
     while ((isspace(line[length])) && (length >= 0) )
      length--;
     if (length < 0) continue;     /* blank line */
     line[++length] = '\0';     
     add_config(line);
   }

  fclose(nsrc);
}

/*}}}*/
/*{{{  start_NetworkServer() */

/**
*** Code to start up the Network Server and Session Manager. This code
*** has to set up the argument vector.
**/
static void start_server(Object *processor, char *server, char **argv);

#ifndef SingleProcessor
static void start_NetworkServer(Object *processor)
{ 
  Nsrc_env[0] = "startns-magic";
  if (do_reset)
   Nsrc_env[1] = "-r";
  else
   Nsrc_env[1] = " ";
  if (silent_mode)
   Nsrc_env[2] = "-q";
  else
   Nsrc_env[2] = " ";
  Nsrc_env[3]  = DebugString;

  start_server(processor, "netserv", Nsrc_env);
}
#endif

/*}}}*/
/*{{{  start_SessionManager() */

static void start_SessionManager(Object *processor)
{ char	capbuf[20];
  
  Nsrc_env[0] = "startns-magic";
  (void) DecodeCapability(capbuf, &NetworkCap);
  Nsrc_env[1] = capbuf;
  if (do_reset)
   Nsrc_env[2] = "-r";
  else
   Nsrc_env[2] = " ";
  Nsrc_env[3] = DebugString;
  start_server(processor, "session", Nsrc_env); 
}

/*}}}*/
/*{{{  start_server() */

static void start_server(Object *processor, char *server, char **Argv)
{ char *net_dir = get_config("net_directory", &(Nsrc_env[FirstNsrc]));
  Object *dir, *bin, *prog;
  Stream *prog_stream;
  Environ env;
  Object  *Objv[OV_End + 1];
  Object  *procman;

  if (net_dir eq Null(char)) net_dir = "/helios/lib";

  dir = Locate(Null(Object), net_dir);
  if (dir eq Null(Object))
   { printf("Startns: failed to locate net directory %s\n", net_dir);
     fatal();
   }
  bin = Locate(dir, server);
  if (bin eq Null(Object))
   { printf("Startns: failed to locate server %s in directory %s\n",
            server, dir->Name);
     fatal();
   }
  Close(dir);
  procman = Locate(processor, "/tasks");
  if ((procman eq Null(Object)) && (processor ne Null(Object)))
   printf("Startns: warning, failed to locate Processor Manager of %s\n",
            processor);
  prog = Execute(procman, bin);
  if (prog eq Null(Object))
   { printf("Startns: failed to execute server %s, fault 0x%08lx\n",
            bin->Name, Result2(bin));
     fatal();
   }

  prog_stream = Open(prog, Null(char), O_ReadWrite);
  if (prog_stream eq Null(Stream))
   { printf("Startns: failed to open program %s, fault 0x%08lx\n",
            prog->Name, Result2(prog));
     fatal();
   }
     
  Objv[OV_Cdir]		= my_environ->Objv[OV_Cdir];
  Objv[OV_Task]		= prog;
  Objv[OV_Code]		= (Object *) MinInt;
  Objv[OV_Source]	= bin;
  Objv[OV_Parent]	= my_environ->Objv[OV_Task];
  Objv[OV_Home] 	= my_environ->Objv[OV_Home];
  Objv[OV_Console]	= my_environ->Objv[OV_Console];
  Objv[OV_CServer]	= my_environ->Objv[OV_CServer];
  Objv[OV_Session]	= my_environ->Objv[OV_Session];
  Objv[OV_TFM]		= my_environ->Objv[OV_TFM];
  Objv[OV_TForce]	= (Object *) MinInt;
  Objv[OV_End]		= Null(Object);
  
  env.Argv = Argv;
  env.Envv = my_environ->Envv;
  env.Objv = Objv;
  env.Strv = my_environ->Strv;

  if (SendEnv(prog_stream->Server, &env) < 0)
   { printf("Startns: failed to send environment to server %s\n",
             server);
      fatal();
   }
  Close(prog_stream);
  Close(prog);
  Close(bin);   
}

/*}}}*/
#ifndef SingleProcessor
/*{{{  count_IOprocs() */

static int count_IOprocs(RmProcessor Processor, ...)
{
  if ((RmGetProcessorPurpose(Processor) & RmP_Mask) eq RmP_IO)
   return(1);
  else
   return(0);
}

/*}}}*/
/*{{{  find_IOprocs() */

static int find_IOprocs(RmProcessor Processor, ...)
{
  if ((RmGetProcessorPurpose(Processor) & RmP_Mask) eq RmP_IO)
   return((int) Processor);
  else
   return(0);
}

/*}}}*/
/*{{{  find_neighbour() */

static int find_neighbour(RmProcessor Processor, ...)
{ va_list	args;
  RmProcessor	target;
  int		i;
  
  va_start(args, Processor);
  target = va_arg(args, RmProcessor);
  va_end(args);
  
  for (i = 0; i < RmCountLinks(Processor); i++)
   if (RmFollowLink(Processor, i, Null(int)) eq target)
    return((int) Processor);
  return(0);
}

/*}}}*/
/*{{{  build_fullname() */

static void build_fullname(char *buff, RmProcessor Processor)
{
  if (Processor->ObjNode.Parent eq Null(DirNode))
   { buff[0] = '/'; buff[1] = '\0';
     pathcat(buff, Processor->ObjNode.Name);
   }
  else
   { build_fullname(buff, (RmProcessor) Processor->ObjNode.Parent);
     pathcat(buff, Processor->ObjNode.Name);
   }
}

/*}}}*/
#endif
/*{{{  locate_root_processor() */

/**
*** Code to determine the root processor of a network. This is non-trivial.
*** If there is only one processor currently active, or if there are
*** two including an I/O processor, then the current processor must be the
*** one. If life is not so simple, i.e. startns is being executed again
*** within a running network, there are various tricks to try out.
*** There may be an option in the nsrc file, 
*** e.g. root_processor = /Cluster/00, in which case I can try to locate it.
*** If not, the code walks down the network counting the number of
*** I/O Processors. Hopefully there is only one. If so I try to find it,
*** and the processor next to it, and that is the root node. Other
*** tricks may be added in future if I think of them.
***
*** The code relies on several RmApply's and RmSearch's.
**/

static Object *locate_root_processor(char *name)
{ Object *result;
  char   fullname[64];

    /* if a name has been given on the command line, use it */
  if (name ne Null(char))
   { strcpy(fullname, name); goto gotname; }

    /* if this is the only intelligent processor in the network, */
    /* execute locally */
  { LinkInfo ldata; int i;
    int number_conns = 0;
    for (i = 0; ; i++)
     if (LinkData(i, &ldata) eq Err_Null)
      { if ((ldata.Mode eq Link_Mode_Intelligent) && 
            !(ldata.Flags & Link_Flags_ioproc) )
         number_conns++;
      }
     else
      break;
     if (number_conns eq 0)  /* All alone in the world... */
      { (void) MachineName(fullname); goto gotname; }
   }

  { char *result = get_config("root_processor", &(Nsrc_env[FirstNsrc]));
    if (result ne Null(char))
     { strcpy(fullname, result); goto gotname; }
  }

#ifndef SingleProcessor  
  { int    number_IOprocs;
    number_IOprocs = RmApplyProcessors(Network, &count_IOprocs);
    if (number_IOprocs eq 1)
     { RmProcessor IOproc, neighbour;
       int links;
       IOproc = (RmProcessor) RmSearchProcessors(Network, &find_IOprocs);
       if (IOproc eq (RmProcessor)NULL)
        { puts("Startns: warning, consistency failure 1 in resource map.");
          return(Null(Object));
        }
 
       links = RmCountLinks(IOproc);
       if (links eq 0)
        { neighbour = (RmProcessor)
             RmSearchProcessors(Network, &find_neighbour, IOproc);
          if (neighbour eq (RmProcessor)NULL)
           { puts("Startns: warning, consistency failure 2 in resource map.");
             return(Null(Object));
           }
        }
       elif (links eq 1)
        { neighbour = RmFollowLink(IOproc, 0, Null(int));
          if (neighbour eq RmM_NoProcessor)
           { puts("Startns: warning, consistency failure 3 in resource map.");
             return(Null(Object));
           }
        }
       build_fullname(fullname, neighbour);
       goto gotname;
    }
  }
#endif	/* SingleProcessor */
  
 return(Null(Object));

gotname:
 { char *current;

   for (current = fullname; *current ne '\0'; )
    { 
      result = Locate(Null(Object), current);
      if (result ne Null(Object)) return(result);
      for (current++; (*current ne '/') && (*current ne '\0'); current++);
    }
   printf("Startns: failed to locate root processor %s.\n",
          fullname);
   return(Null(Object));
 }
}

/*}}}*/
/*{{{  check_clashes() */

/**
*** check_clashes(). This routine tests for the presence of a Network Server
*** or Session Manager already present on the chosen root processor.
**/
static void check_clashes(Object *root_processor, bool run_ns, bool run_sm)
{ Object	*NetworkServer;
  Object	*SessionManager;
  int		error = 0;
  
  if (run_ns)
   NetworkServer = Locate(root_processor, "ns");
  else
   NetworkServer = Null(Object);
   
  if (run_sm)
   SessionManager = Locate(root_processor, "sm");
  else
   SessionManager = Null(Object);

  if (NetworkServer ne Null(Object))
   { printf("Startns: there is already a Network Server %s.\n",
   		NetworkServer->Name);
     error++;
   }
  if (SessionManager ne Null(Object))
   { printf("Startns: there is already a Session Manager %s.\n",
   		SessionManager->Name);
     error++;
   }
  if (error) fatal();
}

/*}}}*/
#ifndef SingleProcessor
/*{{{  count_real_processors() */

/**
*** Count the number of real processors in the network
**/

/*{{{  count_real_processors_aux() */

static	int	count_real_processors_aux(RmProcessor Processor, ...)
{ int	purpose;

  purpose = RmGetProcessorPurpose(Processor) & RmP_Mask;
  if (purpose eq RmP_Helios)
   return(1);
  else
   return(0);
}

/*}}}*/

static	int count_real_processors(void)
{ return(RmApplyProcessors(Network, &count_real_processors_aux));
}

/*}}}*/
#endif
#ifndef SingleProcessor
/*{{{  send_resource_map() */

/**
*** Once a Network Server has started up,
**/

static Capability send_resource_map(Object *root_processor)
{ int			retries;
  Object		*NetworkServer = Null(Object);
  RmServer		Server;
  int			return_code;
  Capability		result;
  RmFilterStruct	Filter;
  RmRequest		request;
  RmReply		reply;
  char			path[109];
  char			*temp;

  for (retries = 0; (retries < 10) && (NetworkServer eq Null(Object));
       retries++, Delay(OneSec / 4))
    NetworkServer = Locate(root_processor, "ns");

  if (NetworkServer eq Null(Object))
   { puts("Startns: failed to locate new Network Server.");
     fatal();
   }

	/* /00/ns -> /00/.socket, .NS_ctos	*/  
  strcpy(path, NetworkServer->Name);
  temp = objname(path);
  strcpy(temp, ".socket");

/*  return_code =  RmOpenServer(path, ".NS_ctos", Null(Capability), &Server);*/
  return_code =  RmOpenServer("/.socket", ".NS_ctos", Null(Capability), &Server);
  if (return_code ne RmE_Success)
   { printf("Startns: failed to contact Network Server, RmLib error %s\n",
   		RmMapErrorToString(return_code));
     fatal();
   }

  Clear(request); Clear(reply);
  request.FnRc	= RmC_Startns;
  Filter.Network	= NULL;
  Filter.Processor	= NULL;
  Filter.Task		= NULL;
  Filter.Taskforce	= NULL;
  Filter.SendHardware	= TRUE;
  request.Filter	= &Filter;
  request.Network	= Network;
  request.Taskforce	= Taskforce;

  return_code = RmXch(&Server, &request, &reply);
  if (return_code ne RmE_Success)
   { fputs("Startns: error retrieving reply code from Network Server.\n", stderr);
     fprintf(stderr, "Startns: fault was %s\n", RmMapErrorToString(return_code));
     fatal();
   }
  memcpy(&result, &(reply.Reply1), sizeof(Capability));
  /* RmCloseServer(Server);*/
  return(result);
}

/*}}}*/
#endif

/*}}}*/
