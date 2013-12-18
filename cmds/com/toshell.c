/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- toshell.c								--
--                                                                      --
--	Author:  PAB 21/9/90 - Hacked from BLV login			--
--                                                                      --
------------------------------------------------------------------------*/
#ifdef __TRAN
static char *rcsid = "$Id: toshell.c,v 1.9 1994/03/08 11:37:21 nickc Exp $";
#endif

/* This program is a reduced version of the full login program.		*/
/* Its only duty in life is to set up the environment and run the	*/
/* initial shell.							*/
/* Some of Barts comments may now be misleading...			*/

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
*** Login may be invoked in various different ways. It may be called as
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

/**
*** 1) should login be responsible for printing out the version number ?
*** 2) where should it come from ?
**/
#ifdef __C40
static char *version = "\
\r\n\
\t\t            Helios Operating System\n\
\t\t                 Version 1.3\n\n\
\t\t            TMS320C40 Implementation\n\n\
\t\tCopyright (C) 1987-92, Perihelion Software Ltd\n\n\
";
#elif defined(__ARM)
static char *version = "\
\r\n\
\t\t            Helios Operating System\n\
\t\t                 Version 1.3\n\n\
\t\t               ARM Implementation\n\n\
\t\tCopyright (C) 1987-92, Perihelion Software Ltd\n\n\
";
#else
static char *version = "\
\r\n\
\t\t            Helios Operating System\n\
\t\t                 Version 1.3\n\
\t\tCopyright (C) 1987-92, Perihelion Software Ltd\n\n\
";
#endif

#define eq 	==
#define ne	!=

#pragma -s1		/* disable stack checking */
#pragma -g0		/* and do not put the names into the code */

/**
*** Forward declarations.
**/
static	void	show_file(char *);
static	void	run_local_session(char *, struct passwd *);
static	void	display_motd_etc(Object *);
static	bool	init_window(void);
static	void	prepare_window(void);
static	char	**build_environ(char *, struct passwd *);
static	void	run_actual_session(Object *, Object *, Object **,
		 struct passwd *, char *);
static	Environ	*my_environ;

/*----------------------------------------------------------------------------*/
/**
*** main() - the return code of this program is significant. If the
*** program returns EXIT_FAILURE, something serious happened and there
*** is no point in trying again. If the program returns EXIT_SUCCESS
*** then the parent can try again. Following an authorisation failure
*** or something like that, the program must return EXIT_SUCCESS.
**/

int main(int argc, char **argv)
{ char		*UserName;
  struct passwd pwd_entry = {
#ifdef __ARM
	/* char		*pw_name;   */	"abc",			/* USER */
#else
	/* char		*pw_name;   */	"guest",		/* USER */
#endif
	/* char		*pw_passwd; */	NULL,
	/* uid_t	pw_uid;     */  1,			/* user Id */
	/* uid_t	pw_gid;     */  1,			/* group Id */
	/* char		*pw_gecos;  */  NULL,			/* not used */
	/* char		*pw_dir;    */  "/helios/users/guest",	/* HOME */
	/* char		*pw_shell;  */  "/helios/bin/shell"	/* SHELL */
  };

  if (argc > 2)
   { fputs("toshell: Too many arguments.\r\n", stderr);
     exit(EXIT_FAILURE);
   }

  unless (init_window())
   { fputs("toshell: failed to initialise window.\r\n", stderr);
     exit(EXIT_FAILURE);
   }

  if (argc eq 2)
   UserName = argv[1];
  else
#ifdef __ARM /* @@@ for the time being */
   UserName = "abc";
#else
   UserName = "guest";
#endif

/**
*** Print out the Helios version number. N.B. I am not convinced that 
*** login should do this.
**/
  fputs(version, stderr);
  
  my_environ = getenviron();
  
/* According to the Posix spec it appears to be impossible not	*/
/* to have a password file. Loop until a login attempt succeeds	*/
/* Heres a big RASBERRY from PAB! */

  run_local_session(UserName, &pwd_entry);

  return(EXIT_SUCCESS);
}

/** This routine is responsible for running a session locally, in the
*** absence of a Session Manager. It attempts to validate the current
*** directory and shell specified in the password file, and then calls
*** another routine to do the real work.
**/
static void run_local_session(char *name, struct passwd *pwd_entry)
{ Object	*cwd;
  Object	*cmd;
  Object	*objv[OV_End + 1];
  
  cwd = Locate(CurrentDir, pwd_entry->pw_dir);
  if (cwd eq Null(Object))
   { fprintf(stderr, "toshell: failed to locate current directory %s\n",
   			pwd_entry->pw_dir);
     exit(EXIT_SUCCESS);
   }

  cmd = Locate(CurrentDir, pwd_entry->pw_shell);
  if (cmd eq Null(Object))
   { fprintf(stderr, "toshell: failed to locate command %s\n",
   		pwd_entry->pw_shell);
     exit(EXIT_SUCCESS);
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
  objv[OV_TFM]		= (Object *) MinInt;	/* must be MinInt */
  objv[OV_TForce]	= (Object *) MinInt;
  objv[OV_End]		= Null(Object);     
  run_actual_session(Null(Object), cmd, objv, pwd_entry, name);
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
  
  display_motd_etc(Objv[0]);
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
        { fputs("toshell: failed to locate processor manager.\n", stderr);
          return;
        }
     }

    Exec = Execute(parent, command);
    if (Exec eq Null(Object))
     { fprintf(stderr, "toshell: failed to execute program %s, fault 0x%08lx\n",
     		command->Name, Result2(parent));
       goto done;
     }
    Objv[OV_Task] = Exec;
        
    Program = Open(Exec, Null(char), O_WriteOnly);
    if (Program eq Null(Stream))
     { fprintf(stderr, "toshell : failed to access program %s, fault 0x%08lx\n",
     		Exec->Name, Result2(Exec));
       (void) Delete(Exec, Null(char));	/* Might work... */
       goto done;
     }       

    if (SendEnv(Program->Server, &Env) < 0)        
     { fprintf(stderr,
               "toshell: failed to send program environment, fault 0x%08lx\n",
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
   { WORD	buf[2];
     WORD	size;
     if ((size = InitProgramInfo(Program, PS_Terminate)) >= Err_Null)
      { if (size > 2)
         { fprintf(stderr, "InitProgramInfo : funny size %ld", size);
           goto done;
         }
        else {
         (void) GetProgramInfo(Program, buf, -1);
        }
      }
     else
      fprintf(stderr, "InitProgramInfo : error %lx", size);
   }

done:
	/* General tidy-up */
   if (Program ne Null(Stream))	(void) Close(Program);
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
   fputs("toshell: out of memory when building environment.\n", stderr);
   exit(EXIT_FAILURE);      
}

static char **build_environ(char *login_name, struct passwd *pwd_entry)
{ static char buf[256];
  char	 **my_environ;
  static char *known_strings[] =
    { "HOME=", "SHELL=", "USER=", "SESSION=", "PATH=", "TERM=", "CONSOLE=",
    	Null(char)
    };
    
	/* Cope with all the defaults */
  strcpy(buf, known_strings[0]);
  strncat(buf, pwd_entry->pw_dir, 200);
  add_env(buf);
  strcpy(buf, known_strings[1]);
  strncat(buf, pwd_entry->pw_shell, 200);
  add_env(buf);
  strcpy(buf, known_strings[2]);
  strncat(buf, pwd_entry->pw_name, 200);
  add_env(buf);
  strcpy(buf, known_strings[3]);
  strncat(buf, login_name, 200);
  add_env(buf);
  strcpy(buf, known_strings[4]);
  strncat(buf, "/helios/bin", 200);
  add_env(buf);
  strcpy(buf, known_strings[5]);
  strncat(buf, "ansi", 200);
  add_env(buf);
  strcpy(buf, known_strings[6]);
  strncat(buf, Heliosno(stdin)->Name, 200);
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
*** Login should be running inside a suitable window. There is a test to
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
   { fputs("toshell: must be run interactively.\r\n", stderr);
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
*** This routine is used after a successful login, but before starting the
*** shell or whatever program should be run. The Unix version does the
*** following things:
***    1) update accounting files
***    2) check for mail
***    3) display the message of the day
***    4) display the time when the user last logged in
***
*** Accounting files will not be added until Helios 1.3, and mail may be
*** even further away. The message of the day can be handled, but as yet
*** there is no equivalent to the file /etc/utmp to store details of
*** the last login. Hence the routine is fairly easy.
***
*** Apart from step 1, all of this can be suppressed by having a file
*** .hushlogin in the user's home directory, which should be passed as
*** an argument.
**/
static void	display_motd_etc(Object *cwd)
{ 
  { Object	*silent = Locate(cwd, ".hushlogin");
    if (silent ne Null(Object))
     { Close(silent); return; }
  }

/* BLV - show last login */ 
  show_file("/helios/etc/motd");
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
