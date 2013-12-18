/*------------------------------------------------------------------------
--                                                                      --
--     			H E L I O S   C O M M A N D S			--
--			-----------------------------			--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- remote.c								--
--                                                                      --
--	A proper version of the remote command.				--
--                                                                      --
--	Author:  BLV 13/7/90						--
--	Rewrite: BLV 9.6.92, to use RmLib				--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/remote.c,v 1.10 1994/02/21 17:58:26 nickc Exp $*/

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
#include <time.h>
#include "rmlib.h"

#define eq ==
#define ne !=

/**
*** find_file() is not currently located. It searches through the command
*** search path for the specified string.
**/
extern void	find_file(char *, char *);
/*}}}*/

/*{{{  signal handling */
static	Object	*Global_ProgObject	= Null(Object);
#ifndef NoRmLib
static	RmTask	 Global_RunningTask	= (RmTask) NULL;
#endif
  
static	void	 my_signalhandler(int x)
{ if (Global_ProgObject ne Null(Object))
   { Stream	*temp = PseudoStream(Global_ProgObject, O_ReadWrite);
     SendSignal(temp, x);
     Close(temp);
   }
#ifndef NoRmLib
  elif (Global_RunningTask ne (RmTask) NULL)
   RmSendTaskSignal(Global_RunningTask, x);
#endif
}
/*}}}*/
/*{{{  usage() */
static void usage(void)
{ fputs("remote: usage, remote [-d] processor command [args]\n", stderr);
  exit(EXIT_FAILURE);
}
/*}}}*/
#ifndef NoRmLib
/*{{{  find the TFM for this session, if there is one */
static Object	*find_tfm(void)
{ Environ	 *env	= getenviron();
  Object	**objv	= env->Objv;
  int		  i;

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
/*{{{  run the program using RmLib calls */
/**
*** Use the Resource Management library to run the shell on a particular
*** processor.
**/
static int rmlib_run(char *processor_name, char **command_args, bool detach)
{ RmNetwork	 real_network	= RmGetNetwork();
  RmProcessor	 real_processor;
  RmProcessor	 obtained_processor;
  RmTask	 command_template;
  RmTask	 running_command;
  char		 command_name[IOCDataMax];
  int		 rc;
  int		 i;

  
  if (real_network eq (RmNetwork) NULL)
   { fprintf(stderr,"remote: failed to get network details. (%x)\n", RmErrno);
     return(EXIT_FAILURE);
   }
  real_processor = RmLookupProcessor(real_network, processor_name);
  if (real_processor eq NULL)
   { fprintf(stderr, "remote: failed to find processor %s in the network.\n",
		processor_name);
     return(EXIT_FAILURE);
   }
  
  obtained_processor = RmObtainProcessor(real_processor);
  if (obtained_processor eq (RmProcessor) NULL)
   { fprintf(stderr, "remote: failed to get access to processor %s. (%x)\n",
	     processor_name, RmErrno);
     return(EXIT_FAILURE);
   }

  if (*(command_args[0]) eq '/')
   strcpy(command_name, command_args[0]);
  else
   find_file(command_name, command_args[0]);
  
  command_template	= RmNewTask();
  if (command_template eq (RmTask) NULL)
   { fputs("remote: out of memory.\n", stderr);
     return(EXIT_FAILURE);
   }

  RmSetTaskId(command_template, command_args[0]);
  RmSetTaskCode(command_template, command_name);
  for (i = 1; command_args[i] ne NULL; i++)
   RmAddTaskArgument(command_template, i, command_args[i]);
     
  running_command = RmExecuteTask(obtained_processor, command_template, NULL);
  
  if (running_command eq (RmTask) NULL)
   { fputs("remote: failed to execute command.\n", stderr);
     /* NB/ The error code returned is probably bogus, so ignore it */
     return(EXIT_FAILURE);
   }
  
  Global_RunningTask = running_command;

  if (detach)
   { RmLeaveTask(running_command);
     rc = 0;
   }
  else
    {
      rc = RmWaitforTask(running_command) >> 8;
    }
  
  return(rc);
}
/*}}}*/
#endif
/*{{{  run the program using system library calls */
/**
*** This routine uses system library calls to run the program on
*** a particular processor.
**/
static int simple_run(char *processor_name, char **command_args, bool detach)
{ Object	*objv[OV_End + 1];
  Environ	*my_environ	= getenviron();
  Environ	 sending;
  Stream	*program_stream	= Null(Stream);
  Object	*remote_processor;
  Object	*controller;
  int		 rc;

  { char	full_name[IOCDataMax];

    if (*processor_name eq '/')
     strcpy(full_name, processor_name);
    else
     { full_name[0] = '/'; strcpy(&(full_name[1]), processor_name); }

    remote_processor	= Locate(Null(Object), full_name);
    if (remote_processor eq Null(Object))
     { fprintf(stderr, "remote: cannot find processor %s\n", full_name);
       return(EXIT_FAILURE);
     }

    controller		= Locate(remote_processor, "tasks");
    if (controller eq Null(Object))
     { fprintf(stderr, "remote: cannot find %s/tasks\n", remote_processor->Name);
       return(EXIT_FAILURE);
     }
  }

	/* "controller" is now the target processor manager.		*/
	/* The full environment can now be constructed.			*/
  objv[OV_Cdir]		= my_environ->Objv[OV_Cdir];
  objv[OV_Task]		= (Object *) MinInt;
  objv[OV_Code]		= (Object *) MinInt;
  objv[OV_Source]	= (Object *) MinInt;
  objv[OV_Parent]	= my_environ->Objv[OV_Task];
  objv[OV_Home]		= my_environ->Objv[OV_Home];
  objv[OV_Console]	= my_environ->Objv[OV_Console];
  objv[OV_CServer]	= my_environ->Objv[OV_CServer];
  objv[OV_Session]	= my_environ->Objv[OV_Session];
  objv[OV_TFM]		= my_environ->Objv[OV_TFM];
  objv[OV_TForce]	= (Object *) MinInt;
  objv[OV_End]		= Null(Object);

  sending.Strv		= my_environ->Strv;
  sending.Objv		= objv;
  sending.Envv		= my_environ->Envv;
  sending.Argv		= command_args;

	/* Find the program to execute.					*/
  { char	 command_name[IOCDataMax];
    if (*(command_args[0]) eq '/')
     strcpy(command_name, command_args[0]);
    else
     { command_name[0] = '\0';
       find_file(command_name, command_args[0]);
       if (command_name[0] eq '\0')
        { fprintf(stderr, "remote: cannot find program %s\n", command_args[0]);
	  return(EXIT_FAILURE);
        }
     }
        objv[OV_Source]	= Locate(cdobj(), command_name);
    if (objv[OV_Source] eq NULL)
     { fprintf(stderr, "remote: failed to find program %s\n", command_args[0]);
      return(EXIT_FAILURE);
     }
  }

  objv[OV_Task]		= Execute(controller, objv[OV_Source]);
  if (objv[OV_Task] eq NULL)
   { fprintf(stderr, "remote: failed to run program %s\n", objv[OV_Source]->Name);
     return(EXIT_FAILURE);
   }
  Global_ProgObject = objv[OV_Task]; /* for signal handling */

  program_stream	= Open(objv[OV_Task], NULL, O_ReadWrite);
  if (program_stream eq NULL)
   { int i;
     for (i = 0; i < 3; i++) Delete(objv[OV_Task], NULL);  
     fprintf(stderr, "remote: failed to open %s\n", objv[OV_Task]->Name);
     return(EXIT_FAILURE);
   }

  (void) SendEnv(program_stream->Server, &sending);

  if (!detach)
   { if (InitProgramInfo(program_stream, PS_Terminate) < Err_Null)
      { fprintf(stderr, "remote: failed to wait for task %s\n", program_stream->Name);
	return(EXIT_FAILURE);
      }
     rc = (int)GetProgramInfo(program_stream, NULL, -1);
     if (rc ne 0)
      rc >>= 8;		/* Helios->Posix return code translation */
   }
  else
   rc = 0;

  Global_ProgObject = NULL;
  Close(program_stream);
  return(rc);
}
/*}}}*/
/*{{{  main() */

int main(int argc, char **argv)
{ char		 *processor_name = NULL;
#ifndef NoRmLib
  Object	 *tfm;
#endif
  int		  result;
  char		**command_args;
  bool		  detach	= FALSE;

  if (argc < 3) usage();
  if (!strcmp(argv[1], "-d"))
   { detach = TRUE;
     if (argc < 4) usage();
     processor_name = argv[2];
     command_args   = &(argv[3]);
   }
  else
   { processor_name = argv[1];
     command_args   = &(argv[2]);
   }
  
	/* If the program is not going to be detached immediately,	*/
	/* trap incoming signals so that they can be forwarded.		*/
  { struct sigaction	temp;
    if (sigaction(SIGINT,NULL, &temp) < 0)
     { fputs("remote: failed to access signal handling facilities.\n", stderr);
       return(EXIT_FAILURE);
     }
    temp.sa_handler = &my_signalhandler;
    temp.sa_flags  |= SA_ASYNC;
    if (sigaction(SIGINT, &temp, NULL) < 0)
     { fputs("remote: failed to modify signal handling facilities.\n", stderr);
       return(EXIT_FAILURE);
     }
  }
	/* If running outside a session, system library-level Execute()	*/
	/* calls will be required. Posix does not provide a sensible	*/
	/* of detaching or of specifying a processor.			*/
	/* If there is a TFM then the Resource Management library's	*/
	/* RmExecuteTask() routine should be used.			*/
#ifndef NoRmLib
  tfm = find_tfm();
  if (tfm ne NULL)
   result = rmlib_run(processor_name, command_args, detach);  
  else
#endif
   result = simple_run(processor_name, command_args, detach);

  return(result);
}

/*}}}*/
