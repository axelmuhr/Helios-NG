/*------------------------------------------------------------------------
--                                                                      --
--     			H E L I O S   C O M M A N D S			--
--			-----------------------------			--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- wsh.c								--
--                                                                      --
--	A proper version of the wsh command.				--
--                                                                      --
--	Author:  BLV 13/7/90						--
--	Rewrite: BLV 8.6.92 to use RmLib				--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /users/nickc/RTNucleus/network/RCS/wsh.c,v 1.6 1993/08/11 10:57:27 bart Exp $*/

/*{{{  header files etc */
#include <stdio.h>
#include <syslib.h>
#include <stdlib.h>
#include <nonansi.h>
#include <posix.h>
#include <string.h>
#include <servlib.h>
#include <signal.h>
#include <fault.h>
#include "rmlib.h"

#define eq ==
#define ne !=

	/* find_file is not currently documented. It searches through	*/
	/* the environment's search path for the specified string.	*/
extern void	find_file(char *, char *);
/*}}}*/

/*{{{  usage() */
static void usage(void)
{ 
#ifdef SingleProcessor
  fputs("wsh: usage, wsh\n", stderr);
#else
  fputs("wsh: usage, wsh [processor]\n", stderr);
#endif
  exit(EXIT_FAILURE);
}
/*}}}*/
/*{{{  creating a new window */
/**
*** This code creates a new window using the window server held in the
*** program's environment.
**/
static Object *CreateWindow(void)
{ Object	*new_window;
  Environ	*env		= getenviron();

  if (env eq NULL)
   { fputs("wsh: corrupt environment.\n", stderr);
     exit(EXIT_FAILURE);
   }

	/* Check that the CServer object is actually defined.	*/
  { Object	**objv	= env->Objv;
    int		  i;
    for (i = 0; i <= OV_CServer; i++)
     if (objv[i] eq Null(Object))
      { fputs("wsh: incomplete environment.\n", stderr);
	exit(EXIT_FAILURE);
      }
  }

  new_window = Create(env->Objv[OV_CServer], "shell", Type_Stream, 0, NULL);
  if (new_window eq NULL)
   { fprintf(stderr, "wsh: failed to create new window %s/shell.",
		env->Objv[OV_CServer]->Name);
     exit(EXIT_FAILURE);
   }
  return(new_window);
}
/*}}}*/
/*{{{  find the TFM for this session, if appropriate */
static Object	*find_tfm(char *processor_name)
{ Environ	 *env	= getenviron();
  Object	**objv	= env->Objv;
  int		  i;
  int		  flags;

	/* If CDL is not set, and no processor is specified, then no	*/
	/* TFM is needed.						*/
  flags = _posixflags(PE_BLOCK, 0);
  if (((flags & PE_RemExecute) eq 0) && (processor_name eq NULL))
   return(NULL);

	/* CDL is set, or a particular processor has been specified.	*/
	/* Return the TFM from the environment, if any.			*/
  for (i = 0; i < OV_TFM; i++)
   if (objv[i] eq NULL)
    return(NULL);
  if (objv[OV_TFM] eq (Object *) MinInt)
   return(NULL);
  else
   return(objv[OV_TFM]);
}
/*}}}*/
/*{{{  run the shell using system library calls */
/**
*** This routine uses system library calls to run the new shell. I would
*** prefer to use posix vfork() and execve(), zapping the environment
*** appropriately, but posix does not adequately define mechanisms for
*** detaching a program.
**/
static bool simple_run(Object *new_window, Object *controller, char *processor_name)
{ char		 command_name[IOCDataMax];
  Stream	*window_stream;
  Object	*objv[OV_End + 1];
  Stream	*strv[5];
  char		*argv[2];
  Environ	*my_environ	= getenviron();
  Environ	 sending;
  Stream	*program_stream	= Null(Stream);
  Object	*remote_processor;

  if (processor_name ne NULL)
   { char	full_name[IOCDataMax];

	/* A particular processor has been specified, but there is no	*/
	/* TFM. Hence it is necessary to Locate the target processor	*/
	/* and turn its Processor Manager into the controller.		*/
     if (*processor_name eq '/')
      strcpy(full_name, processor_name);
     else
      { full_name[0] = '/'; strcpy(&(full_name[1]), processor_name); }

     remote_processor	= Locate(Null(Object), full_name);
     if (remote_processor eq Null(Object))
      { fprintf(stderr, "wsh: cannot find processor %s\n", full_name);
	return(FALSE);
      }

     controller		= Locate(remote_processor, "tasks");
     if (controller eq Null(Object))
      { fprintf(stderr, "wsh: cannot find %s/tasks\n", remote_processor->Name);
        return(FALSE);
      }
   }

	/* At this point "controller" can be one of the following :	*/
	/*  a) this session's TFM, if CDL is set and no processor has	*/
	/*     been specified.						*/
	/*  b) a particular Processor Manager, if a processor has been	*/
	/*     specified but there is no TFM				*/
	/*  c) NULL, to run the shell locally.				*/
	/* The full environment can now be constructed.			*/

  objv[OV_Cdir]		= my_environ->Objv[OV_Cdir];
  objv[OV_Task]		= (Object *) MinInt;
  objv[OV_Code]		= (Object *) MinInt;
  objv[OV_Source]	= (Object *) MinInt;
  objv[OV_Parent]	= my_environ->Objv[OV_Task];
  objv[OV_Home]		= my_environ->Objv[OV_Home];
  objv[OV_Console]	= new_window;
  objv[OV_CServer]	= my_environ->Objv[OV_CServer];
  objv[OV_Session]	= my_environ->Objv[OV_Session];
  objv[OV_TFM]		= my_environ->Objv[OV_TFM];
  objv[OV_TForce]	= (Object *) MinInt;
  objv[OV_End]		= Null(Object);

  window_stream		= Open(new_window, NULL, O_ReadWrite);
  if (window_stream eq Null(Stream))
   { fprintf(stderr, "wsh: failed to open window %s\n", new_window->Name);
     return(FALSE);
   }
  window_stream->Flags	|= Flags_OpenOnGet;	/* prevent premature deletion */
	/* stdin should be a different stream from stdout/stderr, or	*/
	/* the Posix library can get upset.				*/
  strv[0] = window_stream;
  strv[1] = strv[2] = CopyStream(window_stream);
  if (strv[1] eq NULL)
   { fprintf(stderr, "wsh: out of memory\n");
     return(FALSE);
   }
  strv[0]->Flags &= ~O_WriteOnly;
  strv[1]->Flags &= ~O_ReadOnly;
  strv[3] = my_environ->Strv[3];
  strv[4] = Null(Stream);

  argv[0]		= "shell";
  argv[1]		= NULL;

  sending.Strv		= strv;
  sending.Objv		= objv;
  sending.Envv		= my_environ->Envv;
  sending.Argv		= argv;

	/* The shell may be in a cache somewhere.	*/
  find_file(command_name, "shell");
  objv[OV_Source]	= Locate(cdobj(), command_name);
  if (objv[OV_Source] eq NULL)
   { fputs("wsh: failed to locate shell\n", stderr);
     return(FALSE);
   }

  objv[OV_Task]		= Execute(controller, objv[OV_Source]);
  if (objv[OV_Task] eq NULL)
   { fputs("wsh: failed to run shell.\n", stderr);
     exit(EXIT_FAILURE);
   }

  program_stream	= Open(objv[OV_Task], NULL, O_ReadWrite);
  if (program_stream eq NULL)
   { int i;
     for (i = 0; i < 3; i++) Delete(objv[OV_Task], NULL);  
     fprintf(stderr, "wsh: failed to open %s\n", objv[OV_Task]->Name);
     return(FALSE);
   }

  (void) SendEnv(program_stream->Server, &sending);
  Delay(OneSec);		/* to let the shell open its window	*/
  Close(program_stream);	/* detach */
  return(TRUE);
}
/*}}}*/
/*{{{  run the shell using RmLib calls */
/**
*** Use the Resource Management library to run the shell on a particular
*** processor.
**/
#ifndef SingleProcessor
static bool rmlib_run(Object *new_window, char *processor_name)
{ RmNetwork	 real_network	= RmGetNetwork();
  RmProcessor	 real_processor;
  RmProcessor	 obtained_processor;
  RmTask	 shell_template;
  RmTask	 running_shell;
  char		 command_name[IOCDataMax];
  FILE		*stderr_save;

  if (real_network eq (RmNetwork) NULL)
   { fputs("wsh: failed to get network details.\n", stderr);
     return(FALSE);
   }
  real_processor = RmLookupProcessor(real_network, processor_name);
  if (real_processor eq NULL)
   { fprintf(stderr, "wsh: failed to find processor %s in the network.\n",
		processor_name);
     return(FALSE);
   }
  obtained_processor = RmObtainProcessor(real_processor);
  if (obtained_processor eq (RmProcessor) NULL)
   { fprintf(stderr, "wsh: failed to get accesss to processor %s\n", processor_name);
     return(FALSE);
   }

  find_file(command_name, "shell");
  shell_template	= RmNewTask();
  if (shell_template eq (RmTask) NULL)
   { fputs("wsh: out of memory.\n", stderr);
     return(FALSE);
   }

  RmSetTaskId(shell_template, "shell");
  RmSetTaskCode(shell_template, command_name);

	/* At this point it is necessary to zap this program's		*/
	/* environment to refer to the new window. The OV_Console	*/
	/* context object should be made to refer to the new window,	*/
	/* and file descriptors 0, 1 and 2 should be streams to the	*/
	/* new window. Strictly speaking this should happen inside a	*/
	/* vfork()'ed child.						*/
  { int		 stderr_fd_save	= dup(2);
    int 	 new_window_fd, new_window_fd2;
    Stream	*new_window_stream, *new_window_stream2;
    Environ	*my_environ	= getenviron();

	/* Save the standard error stream. It will have to be zapped	*/
	/* before running the task in the new window.			*/
    if (stderr_fd_save < 0)
     { fputs("wsh: error duplicating stderr file descriptor.\n", stderr);
       return(FALSE);
     }
    stderr_save = fdopen(stderr_fd_save, "w");
    if (stderr_save eq NULL)
     { fputs("wsh: error duplicating stderr stream.\n", stderr);
       return(FALSE);
     }

	/* Open a stream to the new window, first at the Helios level	*/
	/* and then at the Posix level.					*/
    new_window_stream	= Open(new_window, NULL, O_ReadWrite);
    if (new_window_stream eq NULL)
     { fputs("wsh: failed to access new window.\n", stderr);
       return(FALSE);
     }
    new_window_stream2		 = CopyStream(new_window_stream);
    if (new_window_stream2 eq NULL)
     { fputs("wsh: out of memory accessing window.\n", stderr);
       return(FALSE);
     }
    new_window_stream->Flags	&= ~O_WriteOnly;
    new_window_fd		 = sopen(new_window_stream);
    new_window_stream2->Flags	&= ~O_ReadOnly;
    new_window_fd2		 = sopen(new_window_stream2);
    if ((new_window_fd < 0) || (new_window_fd2 < 0))
     { fputs("wsh: failed to get file descriptor for new window.\n", stderr);
       return(FALSE);
     }

	/* Set stdin, stdout and stderr to refer to the new window.	*/
    close(0); close(1); close(2);
    if ((dup2(new_window_fd, 0) < 0) || (dup2(new_window_fd2, 1) < 0) ||
	(dup2(new_window_fd2, 2) < 0))
     { fputs("wsh: failed to redirect standard streams to new window.\n", stderr_save);
       return(FALSE);
     }

    Close(my_environ->Objv[OV_Console]);
    my_environ->Objv[OV_Console] = new_window;
  }
  
  running_shell = RmExecuteTask(obtained_processor, shell_template, NULL);
  if (running_shell eq (RmTask) NULL)
   { fputs("wsh: failed to execute shell.\n", stderr_save);
     return(FALSE);
   }
  RmLeaveTask(running_shell);
  return(TRUE);
}
#endif
/*}}}*/

/*{{{  main() */
int main(int argc, char **argv)
{ char		*processor_name = NULL;
  Object	*new_window;
  Object	*tfm;
  bool		 result;

#ifdef SingleProcessor
  if (argc > 1)
   usage();
#else
  if (argc eq 2)
   processor_name = argv[1];
  elif (argc > 2)
   usage();
#endif

  new_window	= CreateWindow();
  tfm		= find_tfm(processor_name);

	/* If running outside a session, or if no particular processor	*/
	/* has been specified, system library-level Execute() calls	*/
	/* will be required. Posix does not provide a sensible way	*/
	/* of detaching.						*/
  if ((tfm eq NULL) || (processor_name eq NULL))
   result = simple_run(new_window, tfm, processor_name);

#ifndef SingleProcessor
  else
	/* If there is a TFM and a processor has been specified then	*/
	/* the Resource Management library's RmExecuteTask() routine	*/
	/* should be used.						*/
   result = rmlib_run(new_window, processor_name);  
#endif

	/* This Delete should set a flag in the window server, causing	*/
	/* it to go destroy the window once all the clients have gone	*/
	/* away.							*/
  Delete(new_window, NULL);
  return(result ? EXIT_SUCCESS : EXIT_FAILURE);
}
/*}}}*/



