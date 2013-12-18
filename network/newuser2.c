/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- newuser2.c								--
--		This program is used in a single-user single-processor	--
-- environment to start login running in a loop.			--
--                                                                      --
--	Author:  BLV 3/4/91						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/newuser2.c,v 1.3 1994/03/01 12:40:14 nickc Exp $*/

#include <string.h>
#include <stdarg.h>
#include <syslib.h>
#include <posix.h>
#include <codes.h>
#include <nonansi.h>
#include <stdlib.h>

#ifndef eq
#define eq ==
#define ne !=
#endif

/**
*** forward declarations.
**/
static int	RunCommand(char **);

int main(int argc, char **argv)
{ char		*command_args[4];
  int		rc;

  command_args[0] = "/helios/bin/login";
  command_args[1] = "-loop";
  command_args[2] = argv[1];	/* name or null */
  command_args[3] = Null(char);
    
  rc = RunCommand(command_args);
  return(rc);
  argc = argc;
}

static int RunCommand(char **command_args)
{ Object	*objv[OV_End + 1];
  Environ	*my_environ = getenviron();
  Environ	sending;
  Stream	*program_stream = Null(Stream);
  int		rc = (int) Err_Null;
          
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

  sending.Strv = my_environ->Strv;
  sending.Objv = objv;
  sending.Envv = my_environ->Envv;
  sending.Argv = command_args;
    
  objv[OV_Source] = Locate(CurrentDir, command_args[0]);
  if (objv[OV_Source] eq Null(Object))
   { fprintf(stderr, "newuser : failed to locate command %s\n", command_args[0]);
     goto fail;
   }  
  
  objv[OV_Code] = (Object *) MinInt;

  objv[OV_Task] = Execute(Null(Object), objv[OV_Source]);
  if (objv[OV_Task] eq Null(Object))
   { fprintf(stderr, "newuser: failed to execute command %s\n",
   	objv[OV_Source]->Name);
     goto fail;
   }

  program_stream = Open(objv[OV_Task], Null(char), O_ReadWrite);
  if (program_stream eq Null(Stream))
   { fprintf(stderr, "newuser: failed to open task %s\n", objv[OV_Task]->Name);
     goto fail;
   }

  (void) SendEnv(program_stream->Server, &sending);   

  Delay(OneSec);
   
  Close(program_stream);
  Close(objv[OV_Task]);
  Close(objv[OV_Source]);
  return(rc);
  
fail:  
  if (program_stream ne Null(Stream)) Close(program_stream);
  if (objv[OV_Task] ne Null(Object))
   { (void) Delete(objv[OV_Task], Null(char));
     (void) Close(objv[OV_Task]);
   }
  if (objv[OV_Source] ne Null(Object)) Close(objv[OV_Source]);
  return(EXIT_FAILURE);
} 
