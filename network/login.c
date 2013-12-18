/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990-1994, Perihelion Software Ltd.        --
--                        All Rights Reserved.                          --
--                                                                      --
-- login.c								--
--                                                                      --
--	Author:  BLV 21/6/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/login.c,v 1.35 1994/03/10 17:13:16 nickc Exp $*/

/**
*** This login program behaves mostly like the Unix login program, at least
*** as far as the user is concerned. In practice it works very differently.
***
*** Unix login is invoked either by the getty program or from the built-in
*** shell login command. In both cases it is given a single argument, the
*** user name. Getty is responsible for reading this name, not login.
*** On a successful login the program does an execve() of the required
*** program, usually the shell. Hence the shell has the same process ID
*** as the login program, and the parent is now waiting for the shell to
*** terminate.
***
*** Now for Helios login, a very different story. First, the network may or
*** may not include a Session Manager. For small single-user networks
*** there is no point in having a Session Manager, so login has to be able
*** to cope with that. If there is a Session Manager then login has to
*** interact with it very closely.
***
*** login may be invoked in various different ways. It may be called as
*** a one-off, to run one session. Typically this is done from the initrc
*** file or from a shell. Alternatively it may be invoked by the Session
*** Manager. If a window has been registered with the Session Manager
*** using the newuser command then the Session Manager will repeatedly
*** create sessions in that window, by invoking login. The program may be
*** run with zero or one arguments. If there is an argument then this
*** is the default user name. The first time around, instead of prompting
*** for a user name the default name is displayed. The second time around
*** the user has to type in a name. This matches the behaviour of programs
*** like ftp, and is fairly pleasant. If no default name is given then
*** the user always has to type one.
**/

/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <syslib.h>
#include <gsp.h>
#include <string.h>
#include <posix.h>
#include <stdlib.h>
#include <pwd.h>
#include <nonansi.h>
#include <attrib.h>
#include <ctype.h>
#include <codes.h>
#include <servlib.h>
#include <signal.h>
#include <process.h>
#include "rmlib.h"
#include "exports.h"
#include "session.h"
#include "netutils.h"

/**
*** 1) should login be responsible for printing out the version number ?
*** 2) where should it come from ?
**/
#ifdef NEW_SYSTEM
static char *version = "\
\r\n\n\
\t\t           Helios Operating System\n\
\t\t                 Version 2.0\n\
\t\t(C) Copyright 1987-94, Perihelion Distributed Software\n\n\
";
#else
static char *version = "\
\r\n\n\
\t\t           Helios Operating System\n\
\t\t                 Version 1.31\n\
\t\t(C) Copyright 1987-93, Perihelion Distributed Software\n\n\
";
#endif

#define eq 	==
#define ne	!=

/**
*** Forward declarations.
**/
static	void	init_signals(void);
static	Object	*SessionToDelete = Null(Object);
static	Object	*ProgramToKill	 = Null(Object);
static 	void	check_nologin(void);
static	void	show_file(char *);
static	word	local_login(char *);
static	word	session_login(char *);
static	bool	match_password(char *, char *);
static	bool	run_local_session(char *, struct passwd *);
static	bool	handle_login_prompt(char *, char *, word);
static	bool	handle_password_prompt(char *, word);
static	void	display_motd_etc(Object *, struct passwd *);
static	bool	init_window(void);
static	void	prepare_window(void);
static	char	**build_environ(char *, struct passwd *);
static	void	run_actual_session(Object *, Object *, Object **,
		 struct passwd *, char *);
static	Environ	*my_environ;
static	bool	TerminalNoClobber = FALSE;
static	char	*hostid = Null(char);
static	bool	PreAuthenticated = FALSE;
static	void	usage(void);
static	Semaphore	DeleteLock;
#if !defined(SingleSingle) && !defined(SingleProcessor)
static	void	display_netinfo(void);
#endif
static	bool	loop_forever = FALSE;

/*----------------------------------------------------------------------------*/
/**
*** main() - the return code of this program is significant. If the
*** program returns EXIT_FAILURE, something serious happened and there
*** is no point in trying again. If the program returns EXIT_SUCCESS
*** then the parent can try again. Following an authorisation failure
*** or something like that, the program must return EXIT_SUCCESS.
**/

int main(int argc, char **argv)
{ char		*UserName = Null(char);
  word		rc;
  bool		sm_running = RmTestSessionManager();

#ifndef __TRAN
  SetPriority(HighServerPri);
#endif

  init_signals();

  InitSemaphore(&DeleteLock, 1);

#if 0
  /*
   * XXX - NC - 6/5/1993
   *
   * Currently login, without a session manager, does not behave as users expect.
   * That is, once the login session has finished, users are not giving another
   * login prompt, instead they are just left hanging in limbo, unable to do anything
   * except reboot the system.  This is effectively a crash.  The way to correct
   * this behaviour is to enable the if statement below.  Unfortunately, this breaks
   * rlogin, which expects the login program to return once it has completed one
   * session.  Instead, the initrc script that is shipped with Helios, will have the
   * -loop option enabled, allowing login to continue forever.  One day, however, this
   * mess ought to be cleaned up properly.
   */
  
  if (!sm_running)
    loop_forever = TRUE;
#endif
  
  for (argc--, argv++; argc > 0; argc--, argv++)
   { if (**argv eq '-')
      { if ((*argv)[1] eq 'p')
         TerminalNoClobber = TRUE;
	elif ((*argv)[1] eq 'f')
	 PreAuthenticated = TRUE;
        elif ((*argv)[1] eq 'h')
         { if ((*argv)[2] eq '\0')
            { argc--; hostid = *(++argv); }
           else
            hostid = &((*argv)[2]);
#if 0
           if (getuid() ne 0)
            { fputs("login: -h not accepted, a user id is defined.\n", stderr);
              exit(EXIT_FAILURE);
            }
#endif
         }   
        elif (!strcmp(*argv, "-loop"))
         loop_forever = !loop_forever;
        else
         usage();
      }
     else
      { if (UserName ne Null(char)) usage();
        UserName = *argv;
      }
   }

again:   
  unless (init_window())
   { fputs("login: failed to initialise window.\r\n", stderr);
     exit(EXIT_FAILURE);
   }
      
/**
*** Print out the Helios version number. N.B. I am not convinced that 
*** login should do this.
**/
  fputs(version, stderr);
  
  check_nologin();

  my_environ = getenviron();
  
  if (sm_running)  
   rc = session_login(UserName);
  else
   rc = local_login(UserName);

  if (loop_forever && !sm_running)
   { 	/* The default UserName only ever applies the first time round	*/
     UserName = Null(char);
     goto again;
   }
  else
   return((int)rc);
}

static void usage(void)
{ fputs("login: usage, login [username]\n", stderr);
  exit(EXIT_FAILURE);
}

/*----------------------------------------------------------------------------*/
/**
*** Unix login refuses to do anything if there is a file /etc/nologin.
*** If this file exists, it is printed out and login should exit with
*** a failure.
**/

static void check_nologin(void)
{ Object *Nologin = Locate(Null(Object), "/helios/etc/nologin");
  if (Nologin eq Null(Object))
   return;
  Close(Nologin);
  show_file("/helios/etc/nologin");
  exit(EXIT_FAILURE);
}

/*---------------------------------------------------------------------------*/
/**
*** A local login. First check whether or not there is a password file.
*** If there is no password file then there is no point in prompting
*** for anything, and a local session is created for the user or anon.
*** When this session has terminated there is little point in running another,
*** because the session will be identical. However, starting another session
*** when the user has accidentally hit ctrl-D is probably preferable to
*** forcing a reboot.
***
*** If the password file does appear to exist, we need a name. This
*** is done by handle_login_prompt(). If a name has been supplied already,
*** that routine simply prints "login: <user>" and copies the name into
*** the buffer. Otherwise the user is asked to type something into the
*** specified buffer. Failure, e.g. because the user simply typed return,
*** is handled by another login prompt immediately.
***
*** Given a user name, the password file is consulted for this name.
*** Failure, i.e. an invalid user name, means asking for a password,
*** outputting an error, going to sleep, and retrying. If the user name
*** is known, there may or may not be an associated password. If not there
*** is no point in prompting for one. Otherwise the password has to be
*** typed in and validated.
**/

static	word	local_login(char *username)
{ char		userbuf[NameMax], passwordbuf[NameMax];
  struct	passwd	*pwd_entry;
  
 	/* According to the Posix spec it appears to be impossible not	*/
 	/* to have a password file. Loop until a login attempt succeeds	*/
  forever
  { 
  	/* If username is NULL the user has to type a name. Otherwise	*/
  	/* the name is displayed. If the routine succeeds then userbuf	*/
  	/* will contain the correct name.				*/
    unless(handle_login_prompt(username, userbuf, (NameMax - 1)))
     { username = Null(char); continue; }

    username = Null(char);	/* The default only applies first time */

    if (userbuf[0] eq '#')
     pwd_entry = getpwnam(&(userbuf[1]));
    else    
     pwd_entry = getpwnam(userbuf);
    if (pwd_entry eq Null(struct passwd))
     { 	/* The name is known to be invalid, ask for a password anyway */
       (void) handle_password_prompt(passwordbuf, (NameMax - 1));
       fputs("login: incorrect\r\n", stderr);
       goto error;
     }

	/* The name is valid, there may or may not be a password */
    if (pwd_entry->pw_passwd ne Null(char))
     if (*(pwd_entry->pw_passwd) ne '\0')
      { unless(handle_password_prompt(passwordbuf, (NameMax - 1)))
         { fputs("login: incorrect\r\n", stderr);
           goto error;
         }
        unless(match_password(passwordbuf, pwd_entry->pw_passwd))
         { fputs("login: incorrect\r\n", stderr);
           goto error;
         }
      }    
      
    if (run_local_session(userbuf, pwd_entry)) break;
            
error:
    Delay(3 * OneSec);	/* invalid login, sleep for a bit */
  }    

  return(EXIT_SUCCESS);
}

/**
*** This routine is responsible for running a session locally, in the
*** absence of a Session Manager. It attempts to validate the current
*** directory and shell specified in the password file, and then calls
*** another routine to do the real work. For some single-processor systems
*** it is login's responsibility to start the Taskforce Manager if the
*** user name begins with a # character.
**/

static Object *start_tfm(char *name, Object *cwd);

static bool run_local_session(char *name, struct passwd *pwd_entry)
{ Object	*cwd;
  Object	*cmd;
  Object	*objv[OV_End + 1];
  Object	*controller = Null(Object);
  
  if ((pwd_entry->pw_dir eq Null(char)) || (*(pwd_entry->pw_dir) eq '\0'))
   { fputs("login: no home directory, Logging in with home = /helios\n",
   	   stderr);
     pwd_entry->pw_dir = "/helios";
   }
  
  cwd = Locate(CurrentDir, pwd_entry->pw_dir);
  if (cwd eq Null(Object))
   { fprintf(stderr, "login : failed to locate current directory %s\n",
   			pwd_entry->pw_dir);
     return(FALSE);
   }

  cmd = Locate(CurrentDir, pwd_entry->pw_shell);
  if (cmd eq Null(Object))
   { fprintf(stderr, "login : failed to locate command %s\n",
   		pwd_entry->pw_shell);
     return(FALSE);
   }

  if (name[0] eq '#')
   { name++;
     controller = start_tfm(name, cwd);
   }
   
  objv[OV_Cdir]		= cwd;
  objv[OV_Task]		= (Object *) MinInt;
  objv[OV_Code]		= (Object *) MinInt;
  objv[OV_Source]	= cmd;
  objv[OV_Parent]	= my_environ->Objv[OV_Task];
  objv[OV_Home]		= cwd;	/* Not login's home directory! */
  objv[OV_Console]	= my_environ->Objv[OV_Console];
  objv[OV_CServer]	= my_environ->Objv[OV_CServer];
  objv[OV_Session]	= my_environ->Objv[OV_Session];
  objv[OV_TFM]		= (controller eq Null(Object)) ?
  			     (Object *) MinInt : controller;
  objv[OV_TForce]	= (Object *) MinInt;
  objv[OV_End]		= Null(Object);     
  run_actual_session(controller, cmd, objv, pwd_entry, name);
  return(TRUE);
}

/*----------------------------------------------------------------------------*/
/** Starting a TFM. This is legal only in single-user single-processor mode,
*** i.e. Tiny Helios. In any multi-user mode the TFM must be started by the
*** Session Manager.
**/
#ifndef SingleSingle

static Object *start_tfm(char *name, Object *cwd)
{ name = name; cwd = cwd;
  return(Null(Object));
}

#else

static Object *start_tfm(char *name, Object *cwd)
{ Object	*ProcMan	= Null(Object);
  Object	*TFM		= Null(Object);
  Stream	*TfmStream	= Null(Stream);
  Object	*PipeServer	= Null(Object);
  Object	*Pipe		= Null(Object);
  Stream	*PipeStream	= Null(Stream);
  Object	*TaskforceManager;
  Object	*result		= Null(Object);
  Environ	Env;
  Stream	*Strv[5];
  Object	*Objv[OV_End + 1];
  char		*Argv[4];
  char		pipe_name[NameMax];
  static	int pipe_number = 1;
  word		rc = 1;
  
  TaskforceManager = Locate(Null(Object), "/helios/lib/tfm");
  if (TaskforceManager eq Null(Object))
   { fputs("login: failed to locate Taskforce Manager /helios/lib/tfm\n", stderr);
     goto done;
   }
     
  Strv[0]		= Null(Stream);
      
  PipeServer	= Locate(Null(Object), "/pipe");
  if (PipeServer eq Null(Object))
   { fputs("login: failed to locate pipe server\n", stderr);
     goto done;
   }

  strcpy(pipe_name, "sm.");
  addint(pipe_name, pipe_number++);
  Pipe = Create(PipeServer, pipe_name, Type_Pipe, 0, Null(BYTE));
  Close(PipeServer); PipeServer = Null(Object);
  if (Pipe eq Null(Object))
   { fputs("login: failed to create pipe to TFM\n", stderr);
     goto done;
   }

  PipeStream = PseudoStream(Pipe, O_ReadWrite);
  if (PipeStream eq Null(Stream))
   { fputs("login: failed to open pipe stream\n", stderr);
     (void) Delete(Pipe, Null(char)); Close(Pipe); Pipe = Null(Object);
     goto done;
   }

  Strv[0] = PseudoStream(Pipe, O_ReadWrite);
  if (Strv[0] eq Null(Stream))
   { fputs("login: failed to open pipe to tfm\n", stderr);
     Close(PipeStream); PipeStream = Null(Stream);
     (void) Delete(Pipe, Null(char)); Close(Pipe); Pipe = Null(Object);
     goto done;
   }
    
  ProcMan = Locate(Null(Object), "/tasks");        
  if (ProcMan eq Null(Object))
   { fputs("login: failed to locate Processor Manager\n", stderr);
     goto done; 
   }

  TFM = Execute(ProcMan, TaskforceManager);
  if (TFM eq Null(Object))
   { fprintf(stderr, "login: failed to execute Taskforce Manager, fault 0x%08lx\n",
   		 Result2(ProcMan));
     goto done;
   }
  TfmStream = Open(TFM, Null(char), O_ReadWrite);
  if (TfmStream eq Null(Stream))
   { fprintf(stderr, "login: failed to access Taskforce Manager %s, fault 0x%08lx\n",
   		TFM->Name, Result2(TFM));
     (void) Delete(TFM, Null(char));
     Close(TFM);
     goto done;
   }

  Objv[OV_Cdir]		= cwd;
  Objv[OV_Task]		= TFM;
  Objv[OV_Code]		= (Object *) MinInt;
  Objv[OV_Source]	= TaskforceManager;
  Objv[OV_Parent]	= my_environ->Objv[OV_Task];
  Objv[OV_Home]		= Objv[OV_Cdir];
  Objv[OV_Console]	= my_environ->Objv[OV_Console];
  Objv[OV_CServer]	= (Object *) MinInt;
  Objv[OV_Session]	= (Object *) MinInt;
  Objv[OV_TFM]		= (Object *) MinInt;	/* does not yet exist */
  Objv[OV_TForce]	= (Object *) MinInt;
  Objv[OV_End]		= Null(Object);
  Strv[1]		= my_environ->Strv[1];
  Strv[2]		= my_environ->Strv[2];
  Strv[3]		= my_environ->Strv[3];
  Strv[4]		= Null(Stream);
  Argv[0]		= "SessionManager";
  Argv[1]		= name;
  Argv[2]		= "@0000000000000000";
  Argv[3]		= Null(char);
  Env.Objv		= Objv;
  Env.Strv		= Strv;
  Env.Argv		= Argv;
  Env.Envv		= environ;
    
  if (SendEnv(TfmStream->Server, &Env) < Err_Null)
   { fprintf(stderr, "login: failed to send environment to %s, fault 0x%08lx\n",
   		TfmStream->Name, Result2(TfmStream));
     goto done;
   }

  Close(Strv[0]); Strv[0] = Null(Stream);

  { Capability	cap;
    Object	*tmp;
    char	buffer[NameMax + 1];

    (void) Read(PipeStream, (BYTE *) &cap, sizeof(Capability), 2 * OneSec);
    Delay(OneSec / 2);	/* unfortunately needed */
    buffer[0] = '/';
    strcpy(&(buffer[1]), name);
    tmp = Locate(Null(Object), buffer);
    if (tmp eq Null(Object))
     { fprintf(stderr, "login: failed to locate Taskforce Manager %s\n", buffer);
       goto done;
     }
    tmp->Access = cap;
    result = Locate(tmp, "tfm");
    Close(tmp);
    if (result eq Null(Object))
     { fprintf(stderr, "login: failed to locate Taskforce Manager %s/tfm\n", buffer);
       goto done;
     }
  }
  
  Close(PipeStream); PipeStream = Null(Stream);

  rc = Err_Null;
  
done:
  if (TaskforceManager ne Null(Object)) Close(TaskforceManager);
  if (ProcMan ne Null(Object)) Close(ProcMan);
  if (rc ne Err_Null)
   { if (TfmStream ne Null(Stream))
      { SendSignal(TfmStream, SIGKILL);
        Close(TfmStream);
        TfmStream = Null(Stream);
        if (TFM ne Null(Object))
         { (void) Delete(TFM, Null(char));
           Close(TFM);
         }
      }
   }
  if (TFM ne Null(Object)) Close(TFM);
  if (PipeServer ne Null(Object)) Close(PipeServer);
  if (Pipe ne Null(Object)) Close(Pipe);
  if (PipeStream ne Null(Stream)) Close(PipeStream);
  if (Strv[0] ne Null(Stream)) Close(Strv[0]);
  if (TfmStream ne Null(Stream)) Close(TfmStream);  
  return(result);
}
#endif

/*----------------------------------------------------------------------------*/
/**
*** Session login: in part this is similar to handling a local login. The same
*** problems re. typing in names and passwords have to be handled.
*** However, once the little details are out of the way the
*** problem of actually creating the session are rather different.
**/

static  word	run_sm_session(Object *session, RmSessionInfo *info);

static	word	session_login(char *username)
{ bool 		checking_password = RmTestPasswordsRequired();
  char 		userbuf[NameMax];
  char		passwordbuf[NameMax];
  Object	*session;
  RmSessionInfo	info;
  WORD		fault;  
  WORD		rc;
  struct	passwd	*pwd_entry;

  forever
   {	/* If username is NULL the user has to type a name. Otherwise	*/
   	/* the name is displayed. If a name is typed then userbuf will	*/
   	/* be used to hold it.						*/ 
     unless(handle_login_prompt(username, userbuf, (NameMax - 1)))
      { username = Null(char); continue; }
      
     username = Null(char);	/* default only applies first time */

#if !defined(SingleSingle) && !defined(SingleProcessor)
     if (!strcmp(userbuf, "info"))
      { display_netinfo(); continue; }
#endif

     passwordbuf[0] = '\0';
     if (checking_password)
      { pwd_entry = getpwnam(userbuf);
	if (pwd_entry eq NULL)
	  { (void) handle_password_prompt(passwordbuf, (NameMax - 1));
            fputs("login: incorrect\r\n", stderr);
            goto error;
          }
	 if (pwd_entry->pw_passwd ne NULL)
	  if (*(pwd_entry->pw_passwd) ne '\0')
	   unless(handle_password_prompt(passwordbuf, (NameMax - 1)))
	    { fputs("login: incorrect\r\n", stderr);
	      goto error;
	    }
      }     

     session = RmCreateSession(userbuf, passwordbuf, Heliosno(stdin), &fault);
     if (session eq Null(Object))
      { if ((fault & EO_Mask) eq EO_Password)
         fputs("login: incorrect\r\n", stderr);
	else
         { fputs(
         "login: not enough network resources to create another session.\r\n",
                 stderr);
           fputs("login: please try again later.\r\n", stderr);
         }
        goto error;
      }

     Wait(&DeleteLock);
     SessionToDelete = session;	/* for the signal handlers */
     Signal(&DeleteLock);
     
     unless(RmGetCapabilities(session, &info, &rc))
      { if ((rc & SS_Mask) eq 0)
         switch (rc & (EC_Mask + EG_Mask + EO_Mask))
          { case EC_Error + EG_Invalid + EO_Session :
            case EC_Error + EG_Invalid + EO_Memory :
             fputs("login: serious, unexpected error from session library\n",
               		stderr);
             break;
            case EC_Error + EG_WrongSize + EO_Session :
             fprintf(stderr, "login: serious, error getting size of session %s\n",
             		session->Name);
             break;
            case EC_Error + EG_NoMemory + EO_Message :
             fputs("login: not enough memory.\n", stderr);
             break;
            case EC_Error + EG_Broken + EO_Stream :
             fputs("login: error communicating with session manager.\n", stderr);
             break;
            case EC_Error + EG_Unknown + EO_TFM :
             fputs("login: error contacting user's Taskforce manager.\n", stderr);
             break;
            case EC_Error + EG_Unknown + EO_Directory :
             fputs("login: error, cannot find the home directory.\n", stderr);
             break;
            case EC_Error + EG_Unknown + EO_Program :
             fputs("login: error, cannot find default program.\n", stderr);
             break;
            default :
             fprintf(stderr,
              "login: error 0x%08lx when interacting with session manager.\n",
             	rc);
            }
        else
         fprintf(stderr,
         	  "login: error 0x%08lx when interacting with session manager\n",
          	  rc);    
        Wait(&DeleteLock);
        if (SessionToDelete ne Null(Object))
         { (void) Delete(SessionToDelete, Null(char));
           SessionToDelete = Null(Object);
         }
        Signal(&DeleteLock);
        Close(session);      
        goto error;
      }
      
     rc = run_sm_session(session, &info);    
     Wait(&DeleteLock);
     if (SessionToDelete ne Null(Object))
      { (void) Delete(SessionToDelete, Null(char));
        SessionToDelete = Null(Object);
      }
     Signal(&DeleteLock);
     (void) Close(session);
     if (info.TFM ne Null(Object)) (void) Close(info.TFM);
     (void) Close(info.CurrentDirectory);
     (void) Close(info.Program);
     return(rc);
     
error:
     Delay(3 * OneSec);	/* invalid login, sleep for a bit */
  }
}

static  word	run_sm_session(Object *session, RmSessionInfo *info)
{ struct passwd passwd;
  Object	*objv[OV_End + 1];
  Object	*TFM = Null(Object);

  if (info->TFM ne Null(Object))  
   { TFM = Locate(info->TFM, "tfm");
     if (TFM eq Null(Object))
      { fprintf(stderr,
            "login: internal error communicating with Taskforce Manager %s\r\n",
            info->TFM->Name);
        return(EXIT_FAILURE);
      }
   }

  passwd.pw_name	= info->UserName;
  passwd.pw_passwd	= Null(char);
  passwd.pw_uid		= info->Uid;
  passwd.pw_gid		= info->Gid;
  passwd.pw_gecos	= info->Comment;
  passwd.pw_dir		= info->CurrentDirectory->Name;
  passwd.pw_shell	= info->Program->Name;
  
  objv[OV_Cdir]		= info->CurrentDirectory;
  objv[OV_Task]		= (Object *) MinInt;
  objv[OV_Code]		= (Object *) MinInt;
  objv[OV_Source]	= info->Program;
  objv[OV_Parent]	= my_environ->Objv[OV_Task];
  objv[OV_Home]		= info->CurrentDirectory;
  objv[OV_Console]	= my_environ->Objv[OV_Console];
  objv[OV_CServer]	= my_environ->Objv[OV_CServer];
  objv[OV_Session]	= session;
  objv[OV_TFM]		= (TFM eq Null(Object)) ? (Object *) MinInt : TFM;
  objv[OV_TForce]	= (Object *) MinInt;
  objv[OV_End]		= Null(Object);

  run_actual_session(TFM, info->Program, objv, &passwd,
  			    objname(session->Name));
  return(EXIT_SUCCESS);
}

/*----------------------------------------------------------------------------*/
/**
*** run_actual_session() : do the real work of creating a session.
*** The arguments are as follows:
***   a parent, either NULL to run locally, or a Taskforce Manager
***   a command to execute, complete with capability
***   an object vector
***   a password structure to help build the environment
***   the session name, usually the same as the user name
***   
*** There is a bit of house keeping, to do with the message of the day
*** and so on. Then an attempt is made to create the actual environment
*** to be sent to the shell. This consists of three standard streams,
*** all identical to stdin. The objects are passed in a separate vector,
*** as an argument to this routine. There is only one argument, which must
*** be constructed from the command name. The environment is slightly
*** trickier.
**/
static void run_actual_session(Object *parent, Object *command,
	Object **Objv, struct passwd *pwd_entry, char *login_name)
{ char		*Argv[2], buf[80];
  char		**Envv;
  Environ	Env;

  display_motd_etc(Objv[0], pwd_entry);
  prepare_window();
  
  { char	*temp;
    buf[0] = '-';
    temp = strrchr(command->Name, '/');
    if (temp eq Null(char))
     temp = command->Name;
    else
     temp++;
    strncat(&(buf[1]), temp, 78);
    Argv[0] = buf;
    Argv[1] = Null(char);
  }
  Envv = build_environ(login_name, pwd_entry);
  
  Env.Strv = my_environ->Strv;
  Env.Objv = Objv;
  Env.Argv = Argv;
  Env.Envv = Envv;

  { Object	*Exec = Null(Object);
    Stream	*Program = Null(Stream);

    if (parent eq Null(Object))
     { parent = Locate(Null(Object), "/tasks");
       if (parent eq Null(Object))
        { fputs("login : failed to locate processor manager.\n", stderr);
          return;
        }
     }

    Exec = Execute(parent, command);
    if (Exec eq Null(Object))
     { fprintf(stderr, "login : failed to execute program %s, fault 0x%08lx\n",
     		command->Name, Result2(parent));
       goto done;
     }
    ProgramToKill = Exec;
    Objv[OV_Task] = Exec;
        
    Program = Open(Exec, Null(char), O_ReadWrite);
    if (Program eq Null(Stream))
     { fprintf(stderr, "login : failed to access program %s, fault 0x%08lx\n",
     		Exec->Name, Result2(Exec));
       (void) Delete(Exec, Null(char));	/* Might work... */
       goto done;
     }       

    if (SendEnv(Program->Server, &Env) < 0)        
     { fprintf(stderr,
               "login : failed to send program environment, fault 0x%08lx\n",
     		Result2(Program));
       (void) Delete(Exec, Null(char));
       goto done;
     }
/**
BLV - at this point it is necessary to do something nasty to get the same
BLV effect as Unix execve, i.e. this program can exit and the parent program
BLV will wait until the new child has terminated. For now just leave
BLV login running and waiting for termination.
**/
   { WORD	error = EK_Timeout;

     if (InitProgramInfo(Program, PS_Terminate) >= Err_Null)
      while (error eq EK_Timeout)
       { error = GetProgramInfo(Program, NULL, 20 * OneSec);
         if (error eq EK_Timeout)
          { Object *tmp = Locate(Null(Object), Program->Name);
	    if (tmp eq Null(Object))
	     { fprintf(stderr, "login: program %s has disappeared.\n",
			Program->Name);
               error = EC_Error;
	     }
	    else
	     Close(tmp);
 	  }
       }
     else
      fprintf(stderr, "InitProgramInfo : error %lx", Result2(Program));
   }

done:
	/* General tidy-up */
   if (Program ne Null(Stream))	(void) Close(Program);
   ProgramToKill = Null(Object);
   if (Exec ne Null(Object))	(void) Close(Exec);
   if (parent ne Null(Object))	(void) Close(parent);
   if (command ne Null(Object))	(void) Close(command);
	/* Close all objects in objv ? */   
  }
}

/*----------------------------------------------------------------------------*/
/**
*** Building up a suitable set of environment strings. The login program is
*** responsible for setting the following strings:
***   HOME  - the home directory as per the password file entry
***          This does not incorporate a capability
***   SHELL - the command as per the password file entry
***   USER  - the actual user name, as per password file
***   SESSION - the login name. This may be the same as the user name,
***	        or it may have had a magic number added by the Session
***	        Manager
***   PATH - /helios/bin
***   TERM - ANSI
***   CONSOLE - Heliosno(stdin)->Name
***   PROCTYPE - T800, T414, ...
***
*** Any existing environment strings that do not clash with the above
*** are also incorporated, but ones which do clash are ignored.
***
*** Since login will only ever send one set of environment strings,
*** it can use a static variable. The strings are constructed on the stack
*** and then copied into a new piece of memory.
**/

static char	**env_strings = Null(char *);

	/* Add a single environment string to the table, expanding the */
	/* table as required. */
static void	add_env(char *str)
{ char		*newstring;
  static word	table_count, next_free;

  if (str ne Null(char))
   { newstring = (char *) malloc(strlen(str) + 1);
     if (newstring eq Null(char)) goto nomemory;
     strcpy(newstring, str);
   }
  else
   newstring = str;
      
  if (env_strings eq Null(char *))
   { env_strings = (char **) malloc( 8 * sizeof(char *));
     if (env_strings eq Null(char *)) goto nomemory;
     table_count = 8;
     next_free   = 0;
   }     

   if (next_free >= table_count)
    { char **new_env = (char **) malloc(2 * (int)table_count * sizeof(char *));
      if (new_env eq Null(char *)) goto nomemory;
      memcpy(new_env, env_strings, (int)table_count * sizeof(char *));
      (void) free(env_strings);
      env_strings = new_env;
      table_count = 2 * table_count;
    }
    
   env_strings[next_free++] = newstring;
   return;
   
nomemory:
   fputs("login : out of memory when building environment.\n", stderr);
   exit(EXIT_FAILURE);      
}

static char **build_environ(char *login_name, struct passwd *pwd_entry)
{ static char buf[256];
  char	 **my_environ;
  static char *known_strings[] =
    { "_UID=", "_GID=", "HOME=", "SHELL=", "USER=", "SESSION=",
      "PATH=", "TERM=", "CONSOLE=", "PROCTYPE=",
      Null(char)
    };

	/* add strings so let the Posix library know about Uid's etc */
  sprintf(buf, "_UID=%08X", pwd_entry->pw_uid);
  add_env(buf);
  sprintf(buf, "_GID=%08X", pwd_entry->pw_gid);
  add_env(buf);
     
	/* Cope with all the defaults */
  strcpy(buf, known_strings[2]);
  strncat(buf, pwd_entry->pw_dir, 200);
  add_env(buf);
  strcpy(buf, known_strings[3]);
  strncat(buf, pwd_entry->pw_shell, 200);
  add_env(buf);
  strcpy(buf, known_strings[4]);
  strncat(buf, pwd_entry->pw_name, 200);
  add_env(buf);
  strcpy(buf, known_strings[5]);
  strncat(buf, login_name, 200);
  add_env(buf);
  strcpy(buf, known_strings[6]);
  strncat(buf, "/helios/bin", 200);
  add_env(buf);
  strcpy(buf, known_strings[7]);
  strncat(buf, "ansi", 200);
  add_env(buf);
  strcpy(buf, known_strings[8]);
  strncat(buf, Heliosno(stdin)->Name, 200);
  add_env(buf);
  strcpy(buf, known_strings[9]);

#ifdef __ARM
    strcat(buf, "Arm");
#endif
#ifdef __C40
    strcat(buf, "C40");
#endif
#ifdef __i860
    strcat(buf, "i860");
#endif
#ifdef __M68K
    strcat(buf, "M68K");
#endif
#ifdef __TRAN
  {
    char *tmp;
    switch(MachineType())
     { case	800 :
       case	805 :
       case	801 : tmp = "T800"; break;
       case	414 : tmp = "T414"; break;
       case	425 : tmp = "T425"; break;
       case	400 : tmp = "T400"; break;
       default	    : tmp = "Unknown";
     }
    strcat(buf, tmp);
  }
#endif

  add_env(buf);

  for (my_environ = environ; *my_environ ne Null(char); my_environ++)
   { char **skips;
     for (skips = known_strings; *skips ne Null(char); skips++)
      if (strncmp(*my_environ, *skips, strlen(*skips)) eq 0)
       goto no_addenv;
     add_env(*my_environ);

no_addenv:
     continue;
   }       

  add_env(Null(char));	/* to terminate the table */     
  return(env_strings);
}

/*----------------------------------------------------------------------------*/
/**
*** Screen handling, etc.
***
*** login should be running inside a suitable window. There is a test to
*** ensure that all three standard streams are interactive, so it is likely
*** that login is running sensibly. However, these streams might be fifo's
*** or something equally nasty. I doubt that the system will be adversely
*** affected, whatever happens.
***
*** The current state of the window is not known, in particular the current
*** attributes may be very strange, so these have to be reset. This is done
*** once, called from main(). If the GetAttributes() or SetAttributes()
*** calls fail, login is not running in a window. 
***
*** Just before running the shell the window attributes should be changed
*** again, to a sensible default for applications.
**/

static Attributes	attr;

static bool	init_window(void)
{ 
  unless (isatty(0) && isatty(1) && isatty(2))
   { fputs("login: must be run interactively.\r\n", stderr);
     exit(EXIT_FAILURE);
   }

  if (GetAttributes(Heliosno(stdin), &attr) < 0)
   return(FALSE);
  RemoveAttribute(	&attr, ConsoleIgnoreBreak);
  AddAttribute(		&attr, ConsoleBreakInterrupt);
  AddAttribute(		&attr, ConsolePause);
  AddAttribute(		&attr, ConsoleRawInput);/* login reads one char at */
  RemoveAttribute(	&attr, ConsoleEcho);	/* a time, and does its own */
  						/* processing */
  RemoveAttribute(	&attr, ConsoleRawOutput);
  
  return(SetAttributes(Heliosno(stdin), &attr) >= 0);
}

static void prepare_window(void)
{ RemoveAttribute(	&attr, ConsoleRawInput);
  AddAttribute(		&attr, ConsoleEcho);
  (void) SetAttributes(Heliosno(stdin), &attr);
}

/*----------------------------------------------------------------------------*/
/**
*** As a security measure both names and passwords should be read one
*** character at a time. This means that there will be lots of different
*** messages containing the data, rather than a single message with a whole
*** line of typed-in text. The data can still be intercepted and combined
*** with considerable effort, but there is no point in making life easy.
***
*** A default name may be provided, the first time this routine is called.
*** Otherwise the first argument will be NULL. The second argument is
*** a buffer in which the name should be placed. The third argument
*** is the size of this buffer, to guard against accidental buffer
*** overflows doing nasty things to the stack a la internet worm.
**/

static	bool	handle_login_prompt(char *name, char *buf, word maxlen)
{ word i;

  if (name ne Null(char))
   { printf("\rlogin: %s\n", name);
     strncpy(buf, name, (int)maxlen);
     buf[maxlen] = '\0';
     return(TRUE);
   }
  
  fputs("\rlogin: ", stdout); fflush(stdout);
  
  i = 0;
again:  

  while (i <= maxlen)
   { word result = read(0, &(buf[i]), 1);

     if (result < 0)
	exit(EXIT_FAILURE);

     if (result == 0)	/* End-of-file not possible, because raw input */
      continue;		/* has been set */

     switch(buf[i])
      { case 0x04 : buf[i] = '\0';
      		    puts("\r"); fflush(stdout);
      		    return(FALSE);
        case '\r' :
        case '\n' : 

       		    buf[i] = '\0';
                    puts("\r"); fflush(stdout);
		    return((i eq 0) ? FALSE : TRUE);
        case '\b' :		/* backspace */
        case 0x7F :		/* Delete */
        	    if (i ne 0)
        	     { --i;
        	       fputs("\b \b", stdout);
        	       fflush(stdout);
        	     }
        	    continue;

        /* Possibly deal with other special characters */

        default	   : if (isprint(buf[i]))
                      { putchar(buf[i++]);
                        fflush(stdout);
                      }
                     /* Beep ? Or something similar ? */
      }
   }
   
	/* When this point is reached the buffer has overflowed */
	/* Wait for the user to press backspace or delete, then */
	/* go back.						*/
   { char tempbuf[4];
     forever 
      { while( read(0, tempbuf, 1) <= 0);   /* Get a character */
        if ((tempbuf[0] eq '\b') || (tempbuf[0] eq 0x7F))
         { i--;
           fputs("\b \b", stdout); fflush(stdout);
           goto again;
         }
        else
         { putchar(0x07);	/* Definitely beep */
           fflush(stdout);
         }
      }
   }
}

/**
*** Much the same code for passwords, but without any echoing
**/
static	bool	handle_password_prompt(char *buf, word maxlen)
{ word i;

  fputs("\rPassword: ", stdout); fflush(stdout);
  
  i = 0;
again:  

  while (i <= maxlen)
   { word result = read(0, &(buf[i]), 1);

    if (result < 0)
     exit(EXIT_FAILURE);

    if (result == 0)	/* End-of-file not possible, because raw input */
      continue;		/* has been set */

     switch(buf[i])
      { case 0x04 : buf[i] = '\0';
      		    puts("\r"); fflush(stdout);
      		    return(FALSE);
        case '\r' :
        case '\n' : buf[i] = '\0';
                    puts("\r"); fflush(stdout);
                    return(TRUE);
        case '\b' :		/* backspace */
        case 0x7F :		/* Delete */
        	    if (i ne 0)
        	     --i;
        	    continue;
        /* Possibly deal with other special characters */

        default	   : if (isprint(buf[i]))
                      i++;
      }
   }
   
	/* When this point is reached the buffer has overflowed */
	/* Wait for the user to press backspace or delete, then */
	/* go back.						*/
   { char tempbuf[4];
     forever 
      { while( read(0, tempbuf, 1) <= 0);   /* Get a character */
        if ((tempbuf[0] eq '\b') || (tempbuf[0] eq 0x7F))
         { i--;
           goto again;
         }
        else
         { putchar(0x07);	/* Definitely beep */
           fflush(stdout);
         }
      }
   }
}

/*----------------------------------------------------------------------------*/
/**
*** Password matching. A simple encryption of the supplied password is done
*** and compared with the value stored in the password file.
**/
static	bool	match_password(char *typed, char *pwd_file)
{ char	encoded[Passwd_Max];
  EncodePassword(typed, encoded);
  return(strcmp(encoded, pwd_file) eq 0);
}

/*----------------------------------------------------------------------------*/
/**
*** This routine is used after a successful login, but before starting the
*** shell or whatever program should be run. The Unix version does the
*** following things:
***    1) update accounting files
***    2) check for mail
***    3) display the message of the day
***    4) display the time when the user last logged in
***
*** Accounting files will not be added until Helios 1.3 or later. Mail
*** involves checking the directory /helios/local/spool/mail/<user>
*** The message of the day can be handled, but as yet
*** there is no equivalent to the file /etc/utmp to store details of
*** the last login. Hence the routine is fairly easy.
***
*** Apart from step 1, all of this can be suppressed by having a file
*** .hushlogin in the user's home directory, which should be passed as
*** an argument.
**/
static void	display_motd_etc(Object *cwd, struct passwd *pwd_entry)
{ 
  { Object	*silent = Locate(cwd, ".hushlogin");
    if (silent ne Null(Object))
     { Close(silent); return; }
  }

	/* BLV - show last login, using utmp and wtmp ? */ 

  show_file("/helios/etc/motd");

	/* BLV - cope with different helioi directories 		*/
	/* BLV - sparky can distinguish between mail and new mail.	*/
  { char	buf[128];
    Object	*spool_file;

    strcpy(buf, "/helios/local/spool/mail/");
    strcat(buf, pwd_entry->pw_name);
    spool_file = Locate(Null(Object), buf);
    if (spool_file ne Null(Object))
     { ObjInfo	info;
       word	result = ObjectInfo(spool_file, Null(char), (BYTE *) &info);
       Close(spool_file);
       if ((result >= Err_Null) && (info.Size > 0))
        puts("You have mail.");
     }
  }
}


/**
*** Print out the specified file. This may be the message of the day,
*** or the nologin file.
**/
static void show_file(char *filename)
{ FILE	*file = fopen(filename, "r");
  char	buffer[80];

  if (file eq Null(FILE))
   return;	/* BLV - give error message ? */

  until(fgets(buffer, 80, file) eq Null(char))
   fputs(buffer, stdout);

  fclose(file);
}


/**
*** When the session manager is up and running and the user logs in as
*** info, the system produces some output displaying the current
*** network statistics, i.e. how many people are logged on and who is
*** using all the processors.
**/

#if !defined(SingleSingle) && !defined(SingleProcessor)
typedef	struct	OwnerDetails {
	Node		Node;
	int		Owner;
	int		Count;
} OwnerDetails;

static	List	OwnerList;
static	int	NumberProcessors;
static	int	IOCount;
static	int	RouterCount;
static	int	NetworkWalk(RmProcessor Processor, ...);
static	WORD	ShowOwners(Node *node, WORD arg);
static	WORD	MatchOwner(Node *node, WORD arg);

static void display_netinfo(void)
{ RmNetwork	Network;

  InitList(&(OwnerList));  
  IOCount = RouterCount = 0; 
  
	/* Get details of the current network into local memory */
  Network = RmGetNetwork();
  if (Network == (RmNetwork) NULL)
   { fputs("login: failed to get network details\n", stderr);
     return;
   }
  NumberProcessors = RmCountProcessors(Network);
  
	/* Walk down the current network examining every processor	*/
	/* Build the ownership list as the program goes along.		*/
  (void) RmApplyProcessors(Network, &NetworkWalk);

	/* Output the results by walking down the owner list.		*/
  (void) WalkList(&OwnerList, &ShowOwners);
  if (IOCount > 0)
   printf("%-10s : %3d processor%s, %2d%% of the network\n",
  	"I/O", IOCount, 
  	(IOCount > 1) ? "s" : " ",
  	(100 * IOCount) / NumberProcessors);

  if (RouterCount > 0)
   printf("%-10s : %3d processor%s, %2d%% of the network\n",
  	"Router", RouterCount, 
  	(RouterCount > 1) ? "s" : " ",
  	(100 * RouterCount) / NumberProcessors);
  	
	/* Free the network and the owner list */
  RmFreeNetwork(Network);
  until(EmptyList_(OwnerList)) Free(RemHead(&OwnerList));
}

	/* This routine is called for every processor in the network.	*/
	/* Note that if the network contains subnets then this routine	*/
	/* MUST do the recursion into the subnet.			*/
static	int	NetworkWalk(RmProcessor Processor, ...)
{ int		Owner;
  OwnerDetails	*details;
  int		purpose = RmGetProcessorPurpose(Processor) & RmP_Mask;
  
  if (purpose eq RmP_IO)
   { IOCount++; return(0); }
  elif (purpose eq RmP_Router)
   { RouterCount++; return(0); }
   
	/* Get the current processor owner, and see if this owner	*/
	/* is already known.						*/
  Owner		= RmGetProcessorOwner(Processor);
  details	= (OwnerDetails *) SearchList(&OwnerList, &MatchOwner, Owner);

	/* If the user is already known then the search will have found	*/
	/* an OwnerDetails structure that can be updated. Otherwise	*/
	/* a new structure must be allocated and initialised.		*/
  if (details != (OwnerDetails *) NULL)
   details->Count++;
  else
   { details	= (OwnerDetails *) malloc(sizeof(OwnerDetails));
     if (details == (OwnerDetails *) NULL)
      { fputs("login : out of memory building owner list\n", stderr);
        return(0);
      }
     details->Owner	= Owner;
     details->Count	= 1;
#ifdef SYSDEB
     details->Node.Next = details->Node.Prev = &details->Node;
#endif
     AddTail(&(OwnerList), &(details->Node));
   }

	/* for this program the return value is ignored anyway.	*/
  return(0);
}

	/* Match a processor's owner with an entry in the current	*/
	/* list of owners.						*/
static	WORD	MatchOwner(Node *node, WORD arg)
{ OwnerDetails	*details	= (OwnerDetails *) node;

  if (details->Owner == arg)
   return(1);
  else
   return(0);
}

	/* Print out the results of the search.				*/
static	WORD	ShowOwners(Node *node, WORD arg)
{ OwnerDetails	*details = (OwnerDetails *) node;

  printf("%-10s : %3d processor%s, %2d%% of the network\n",
  	RmWhoIs(details->Owner), details->Count, 
  	(details->Count > 1) ? "s" : " ",
  	(details->Count * 100) / NumberProcessors);
  return(0);
  arg = arg;
}

#endif	/* Multi-user multi-processor system */

/**
*** Signal handling. For telnet logins the session may be aborted
*** by a SIGTERM signal sent to the login process. Similar things
*** may happen with other I/O systems, or automatically by
*** administration hardware. The login program retains two objects
*** as statics, one for the Session entry in the Session Manager which
*** must be deleted, and one for the program executed by login that
*** must be aborted.
**/
static void	mysignalhandler(int);

static void init_signals(void)
{ struct sigaction	temp;
  if (sigaction(SIGINT, Null(struct sigaction), &temp) ne 0)
   { fprintf(stderr, "login: warning, failed to access signal handling facilities.\n");
     return;
   }
  temp.sa_handler	= &mysignalhandler;
  temp.sa_flags	|= SA_ASYNC;
  if (sigaction(SIGINT, &temp, Null(struct sigaction)) ne 0)
   { fprintf(stderr, "login: warning, failed to modify signal handling facilities.\n");
     return;
   }

  if (sigaction(SIGTERM, Null(struct sigaction), &temp) ne 0)
   { fprintf(stderr, "login: warning, failed to access signal handling facilities.\n");
     return;
   }

  temp.sa_handler	 = &mysignalhandler;
  temp.sa_flags		|= SA_ASYNC;
  if (sigaction(SIGTERM, &temp, Null(struct sigaction)) ne 0)
   { fprintf(stderr, "login: warning, failed to modify signal handling facilities.\n");
     return;
   }
}

static void mysignalhandler(int x)
{ x = x;
  
  if (ProgramToKill ne Null(Object))
   { Stream *s = Open(ProgramToKill, Null(char), O_ReadWrite);
     ProgramToKill = Null(Object);
     if (s ne Null(Stream))
      SendSignal(s, SIGHUP);
   }
  Wait(&DeleteLock);
  if (SessionToDelete ne Null(Object))
   { 
     Delay(3 * OneSec);   /* Give the shell a chance to exit normally */
     (void) Delete(SessionToDelete, Null(char));
     SessionToDelete = Null(Object);
   }
  Signal(&DeleteLock);

  Exit(EXIT_FAILURE << 8);
}

