/*------------------------------------------------------------------------
--                                                                      --
--     			H E L I O S   C O M M A N D S			--
--			-----------------------------			--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- run.c								--
--                                                                      --
--	A proper version of the run command.				--
--                                                                      --
--	Author:  BLV 13/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/run.c,v 1.7 1993/12/20 13:42:31 nickc Exp $*/

#include <stdio.h>
#include <syslib.h>
#include <stdlib.h>
#include <nonansi.h>
#include <posix.h>
#include <string.h>
#include <servlib.h>
#include <signal.h>

#define eq ==
#define ne !=

/**
*** find_file() is not currently located. It searches through the command
*** search path for the specified string.
**/
extern void	find_file(char *, char *);

/**
*** forward declarations.
**/
static void	usage(void);
static Object	*CreateWindow(char *);
static int	RunCommand(char **, Object *, bool);
static void	mysignalhandler(int);
static Object	*running_command;


/**
*** main()
***  1) parse the arguments.
***    -d, do not wait for the command to terminate
***    the default command is the shell
***  2) create a new window relative to the current standard streams
***  3) run the command in that window
***  4) get rid of the window
**/
int main(int argc, char **argv)
{ char		**command_args;
  bool		wait_for_child = TRUE;
  Object	*window;
  int		rc;
  static char	*default_args[] = {"shell", Null(char) };
    
  if (argc eq 1)
   command_args = default_args;
  elif (*(argv[1]) eq '-') 
   { if (!strcmp(argv[1], "-d"))
      { wait_for_child = FALSE;
        if (argc > 2)
         command_args = &(argv[2]);
        else
         command_args = default_args;
      }
     else
      usage();
   }
  else
   command_args = &(argv[1]);
   
  window = CreateWindow(command_args[0]);
  if (window eq Null(Object)) return(EXIT_FAILURE);  
  rc = RunCommand(command_args, window, wait_for_child);
  (void) Delete(window, Null(char));
  return(rc);
}

/**
*** [command] is optional, since it will default to the shell.
*** It can be followed by any number of arguments.
**/
static void usage(void)
{ fprintf(stderr, "run : usage, run [-d] [command] [arguments]\n");
  exit(EXIT_FAILURE);
}

/**
*** Creating a new window. This is done by getting an object for the
*** current console server out of the environment.
**/
static Object *CreateWindow(char *command_name)
{ Object	*window_server;
  Object	*new_window;
  char		buffer[NameMax];
  Environ	*env = getenviron();
  
  if (env eq Null(Environ))
   { fputs("run: corrupt environment.\n", stderr);
     return(Null(Object));
   }
  
  { Object	**objv = env->Objv;
    int		i;
    for (i = 0; i <= OV_CServer; i++)  
     if (objv[i] eq Null(Object))
      { fputs("run: incomplete environment.\n", stderr);
        return(Null(Object));
      }
  }

  window_server = env->Objv[OV_CServer];
  if (window_server eq (Object *) MinInt)
   { fputs("run: there is no window server in the current environment.\n",
   		 stderr);
     return(Null(Object));
   }
   
  strncpy(buffer, objname(command_name), NameMax);
  buffer[NameMax - 1] = '\0';
  new_window = Create(window_server, buffer, Type_Stream, 0, Null(BYTE));
  if (new_window eq Null(Object))
   fprintf(stderr, "run : failed to Create window %s/%s", window_server->Name,
   		buffer);
  return(new_window);
}

/**
*** This runs a command using Helios calls only. An attempt is made to
*** open the specified window. If successful the environment is built
*** up, and an attempt is made to locate the program. If successful the
*** program is loaded into memory on the same processor, executed
*** locally, and is sent its environment. Unless the detach option has
*** been given some signal handling is done, so that ctrl-C is forwarded
*** to the child process. Also, run may or may not wait for the child to
*** terminate.
**/
static int RunCommand(char **command_args, Object *window, bool wait_for_child)
{ char		command_name[IOCDataMax];
  Stream	*window_stream = Open(window, Null(char), O_ReadWrite);
  Object	*objv[OV_End + 1];
  Stream	*strv[5];
  Environ	*my_environ = getenviron();
  Environ	sending;
  Stream	*program_stream = Null(Stream);
  word		rc = Err_Null;
          
  objv[OV_Cdir]		= my_environ->Objv[OV_Cdir];
  objv[OV_Task]		= (Object *) MinInt;
  objv[OV_Code]		= (Object *) MinInt;
  objv[OV_Source]	= (Object *) MinInt;
  objv[OV_Parent]	= my_environ->Objv[OV_Task];
  objv[OV_Home]		= my_environ->Objv[OV_Home];
  objv[OV_Console]	= window;
  objv[OV_CServer]	= my_environ->Objv[OV_CServer];
  objv[OV_Session]	= my_environ->Objv[OV_Session];
  objv[OV_TFM]		= my_environ->Objv[OV_TFM];
  objv[OV_TForce]	= (Object *) MinInt;
  objv[OV_End]		= Null(Object);

  if (window_stream eq Null(Stream))
   { fprintf(stderr, "run : failed to open window %s\n", window->Name);
     goto fail;
   }

  window_stream->Flags |= Flags_OpenOnGet;   
  strv[0] = window_stream;
  strv[1] = strv[2] = CopyStream(window_stream);
  if (strv[1] eq NULL)
   { fprintf(stderr, "run: out of memory\n");
     goto fail;
   }
  strv[0]->Flags &= ~O_WriteOnly;
  strv[1]->Flags &= ~O_ReadOnly;
  strv[3] = my_environ->Strv[3];
  strv[4] = (Stream *) MinInt;
  
  sending.Strv = strv;
  sending.Objv = objv;
  sending.Envv = my_environ->Envv;
  sending.Argv = command_args;
    
  if (*(command_args[0]) eq '/')
   strcpy(command_name, command_args[0]);
  else
   find_file(command_name, command_args[0]);

  objv[OV_Source] = Locate(CurrentDir, command_name);
  if (objv[OV_Source] eq Null(Object))
   { fprintf(stderr, "run : failed to locate command %s\n", command_args[0]);
     goto fail;
   }  
  
  objv[OV_Code] = (Object *) MinInt;

  if (getenv("CDL") ne Null(char))
   { Object	*tfm = my_environ->Objv[OV_TFM];
     int	i;
     for (i = 0; i < OV_TFM; i++)
      if (my_environ->Objv[i] eq Null(Object))
       { tfm = Null(Object); break; }
     if (tfm eq (Object *) MinInt) tfm = Null(Object);       
     objv[OV_Task] = Execute(tfm, objv[OV_Source]);
   }
  else	/* run it locally */
   objv[OV_Task] = Execute(Null(Object), objv[OV_Source]);

  if (objv[OV_Task] eq Null(Object))
   { fprintf(stderr, "run: failed to execute command %s\n",
   	objv[OV_Source]->Name);
     goto fail;
   }
  program_stream = Open(objv[OV_Task], Null(char), O_ReadWrite);
  if (program_stream eq Null(Stream))
   { fprintf(stderr, "run: failed to open task %s\n", objv[OV_Task]->Name);
     goto fail;
   }

  running_command = objv[OV_Task];

  if (wait_for_child)  
   { struct sigaction	temp;
     if (sigaction(SIGINT, Null(struct sigaction), &temp) ne 0)
      { fprintf(stderr, "run: warning, failed to access signal handling facilities.\n");
        goto skip_signal;
      }
     temp.sa_handler	= &mysignalhandler;
     temp.sa_flags	|= SA_ASYNC;
     if (sigaction(SIGINT, &temp, Null(struct sigaction)) ne 0)
      fprintf(stderr, "run: warning, failed to modify signal handling facilities.\n");
   }
skip_signal:

  (void) SendEnv(program_stream->Server, &sending);   

  if (wait_for_child)
   { if (InitProgramInfo(program_stream, PS_Terminate) < Err_Null)
      { fprintf(stderr, "run: failed to wait for task %s\n",
      		objv[OV_Task]->Name);
      	goto done;
      }
     rc = GetProgramInfo(program_stream, Null(WORD), -1);
     if (rc ne 0)
      { rc = rc >> 8;	/* ignore bottom byte */
        Delay(OneSec / 2);
      }
   }
  else
   Delay(OneSec);
   
done:   
  Close(window_stream);
  Close(program_stream);
  Close(objv[OV_Task]);
  Close(objv[OV_Source]);
  return (int)(rc);
  
fail:  
  if (window_stream ne Null(Stream)) Close(window_stream);
  if (program_stream ne Null(Stream)) Close(program_stream);
  if (objv[OV_Task] ne Null(Object))
   { (void) Delete(objv[OV_Task], Null(char));
     (void) Close(objv[OV_Task]);
   }
  if (objv[OV_Source] ne Null(Object)) Close(objv[OV_Source]);
  return(EXIT_FAILURE);
} 

static void mysignalhandler(int x)
{ Stream	*program_stream = PseudoStream(running_command, O_ReadWrite);
  if (program_stream ne Null(Stream))
   { SendSignal(program_stream, SIGINT);
     Close(program_stream);
   }
  x = x;
}
