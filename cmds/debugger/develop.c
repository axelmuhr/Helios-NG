/**
*
* Title:  Helios Debugger - Program development.
*
* Author: Andy England
*
* Date:   September 1988
*
*         (c) Copyright 1988 - 1992, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/develop.c,v 1.4 1992/11/04 14:34:35 nickc Exp $";
#endif

#include "tla.h"

#ifdef HE12
#include <sys/wait.h> 
/* -- crf : Pardebug - replace "< sys/wait.h >" with "<sys/wait.h>" */
#endif

PRIVATE int testbuffer (char *);


/**
*
* _make(debug);
*
* Make the program.
*
**/
PUBLIC void _make(DEBUG *debug)
{
  DISPLAY *display = debug->display;
  char cmd[100];

/* -- crf : 07/08/91 - not used 0
  int  make_status;
*/

#ifndef BUG
  int  proc_id;
#endif

#ifdef OLDCODE
  chdir(debug->env.Objv[0]->Name);
#endif
/*
-- crf : 12/08/91 - clean up use of Environment Objv
*/
  chdir(debug->env.Objv[OV_Cdir]->Name);

  dstart(display);
  if ((proc_id = vfork()) == 0)
  {
    
    dup2(fileno(display->filein), 0);
    dup2(fileno(display->fileout), 1);
    dup2(fileno(display->fileout), 2);
    close(fileno(display->filein));
    close(fileno(display->fileout));
    errno = 0;
    execlp("shell", "shell", "-fc","make", NULL);
    perror("make");
    _exit(errno);
  }

#ifndef BUG 
  {
  int t_state,terror;
  do
  {
    terror = wait(&t_state);
  }until((terror == proc_id) || (terror == -1));

  }
#endif 

  
  sprintf(cmd, "%s", debug->env.Argv[0]);

#ifdef OLDCODE
  chdir(debug->env.Objv[0]->Name);
#endif
/*
-- crf : 12/08/91 - clean up use of Environment Objv
*/
  chdir(debug->env.Objv[OV_Cdir]->Name);

  if ((proc_id = vfork()) == 0)
  {

    close(0);
    close(1);
    close(2);

    sopen(debug->env.Strv[0]);
    sopen(debug->env.Strv[1]);
    sopen(debug->env.Strv[2]);  
    close(fileno(display->filein));
    close(fileno(display->fileout));

    execvp(debug->env.Argv[0],debug->env.Argv);
    perror(cmd);
    _exit(1);
  }


  syskillall(debug);
  cmdjmp(debug, TopLevel);
}

/**
*
* _edit(debug, loc):
*
* Invoke an editor.
*
**/
PUBLIC void _edit(DEBUG *debug, LOCATION loc)
{
  DISPLAY *display = debug->display;
  char *editor;
  char lineopt[20];
#ifndef BUG
  int  proc_id;
#endif
  (void)sprintf(lineopt, "-g%d", loc.line);
  if ((editor = getvar(debug->env.Envv, "EDITOR")) == NULL) editor = "emacs";
  /* ACE: The name stored with the source should be the full name, thus
          eliminating the need to fiddle the current directory.
  */

#ifdef OLDCODE
  chdir(debug->env.Objv[0]->Name);
#endif
/*
-- crf : 12/08/91 - clean up use of Environment Objv
*/
  chdir(debug->env.Objv[OV_Cdir]->Name);

  dstart(display);
  if ((proc_id = vfork()) == 0)
  {
    dup2(fileno(display->filein), 0);
    dup2(fileno(display->fileout), 1);
    dup2(fileno(display->fileout), 2);
    execlp(editor, editor, lineopt, getsource(loc.module)->name, NULL);
    perror(editor);
    _exit(1);
  }
#ifndef BUG 
  {
  int t_state,terror;
  do
  {
    terror = wait(&t_state);
  }until((terror == proc_id) || (terror == -1));

  }
#endif 
  dend(display, FALSE);
  
}

/**
*
* _shell(debug, cmdline);
*
* Execute a shell.
*
**/
PUBLIC void _shell(DEBUG *debug, char *cmdline)
{
  DISPLAY *display = debug->display;
  char *shell;
#ifndef BUG
  int  proc_id;
#endif
  if ((shell = getvar(debug->env.Envv, "SHELL")) == NULL) shell = "shell";

#ifdef OLDCODE
  chdir(debug->env.Objv[0]->Name);
#endif
/*
-- crf : 12/08/91 - clean up use of Environment Objv
*/
  chdir(debug->env.Objv[OV_Cdir]->Name);

  dstart(display);
  if ((proc_id = vfork()) == 0)
  {
    dup2(fileno(display->filein), 0);
    dup2(fileno(display->fileout), 1);
    dup2(fileno(display->fileout), 2);
    if (cmdline == NULL) execlp(shell, shell, NULL);
    else execlp(shell, shell, "-fc", cmdline, NULL);
    perror(shell);
    _exit(1);
  }

#ifndef BUG 
  {
  int t_state,terror;
  do
  {
    terror = wait(&t_state);
  }until((terror == proc_id) || (terror == -1));

  }
#endif 
  dend(display, cmdline != NULL);
}

/**
*
* _cd(debug, path);
*
* Change current working directory.
*
**/
PUBLIC void _cd(DEBUG *debug, char *path)
{
  Object *cwd;

  if (path == NULL)
   if ((path = getvar(debug->env.Envv, "HOME")) == NULL)
     cmderr(debug, "No home directory");

#ifdef OLDCODE
  if ((cwd = Locate(debug->env.Objv[0], path)) == NULL)
#endif
/*
-- crf : 12/08/91 - clean up use of Environment Objv
*/
  if ((cwd = Locate(debug->env.Objv[OV_Cdir], path)) == NULL)
    cmderr(debug, "No such directory");
#ifdef OLDCODE
  Close(debug->env.Objv[0]);
  debug->env.Objv[0] = cwd;
#endif
/*
-- crf : 12/08/91 - clean up use of Environment Objv
*/
  Close(debug->env.Objv[OV_Cdir]);
  debug->env.Objv[OV_Cdir] = cwd;

  cmdmsg( debug, "Current directory is now '%s'", path );
  
  return;  
}

/**
*
* _pwd(debug);
*
* Print current working directory.
*
**/
PUBLIC void _pwd(DEBUG *debug)
{
  DISPLAY *display = debug->display;

  dlock(display);
#ifdef OLDCODE
  dprintf(display, "Current Directory is %s", debug->env.Objv[0]->Name);
#endif
/*
-- crf : 12/08/91 - clean up use of Environment Objv
*/
  dprintf(display, "Current Directory is %s", debug->env.Objv[OV_Cdir]->Name);
  dunlock(display);
}

/**
*
* _help(debug);
*
* Display help file.
*
**/
PUBLIC void _help(DEBUG *debug, char *topic)
{
  DISPLAY *display = debug->display;
#ifndef BUG
  int  proc_id;
#endif

  dstart(display);
  if (topic == NULL)
  {
    if ((proc_id = vfork()) == 0)
    {
      dup2(fileno(display->filein), 0);
      dup2(fileno(display->fileout), 1);
      dup2(fileno(display->fileout), 2);
      execlp("more", "more", "/helios/etc/debug.hlp", NULL);
      perror("more");
      _exit(1);
    }
#ifndef BUG 
  {
  int t_state,terror;
  do
  {
    terror = wait(&t_state);
  }until((terror == proc_id) || (terror == -1));

  }
#endif 
  }
  else
  {
    FILE *	file;
    char	buffer[100];
    char	cmpbuffer[100];
    char	cmptopic[100];
    char *	cb, *ct;
    int		count = 0;
    word	no_read = 0;
    bool	control = TRUE;
    char	c[1];

    int nothing_found = TRUE ; /* -- crf : give message if nothing found */

    if ((file = my_fopen("/helios/etc/debug.hlp", "r")) == NULL)
    {
      dend(display, FALSE);
      cmderr(debug, "Cannot find help file");
    }

/*
-- crf : 19/07/91 - (minor) bug 690
-- "help <topic>" should not be case sensitive
-- Convert topic and buffer to lowercase, and then compare ...

-- 28/07/91 - I have not put in my bug fix since Carsten has already fixed
-- this.
*/

      ct = &cmptopic[0];

    until (fgets(buffer, 100, file) == NULL)
    {
      strcpy (cmpbuffer, buffer);
      strcpy (cmptopic, topic);
      cb = &cmpbuffer[0];

      while ( *ct != '\0' AND control == TRUE ) 	/** CR: to avoid case sensitivity */
      {						/* of the topic search. I hope this */
      	*ct = tolower (*ct);	       			/* satisfies Craig */
      	ct++;                                           /* -- crf : sure does ! */
      }
      control = FALSE;

      while ( *cb != '\0' )	/* CR: see above */
      {
      	*cb = tolower (*cb);
      	cb++;
      }
      
      if (strncmp(cmptopic, cmpbuffer, strlen(topic)) == 0)
      {
      	do
        {
          nothing_found = FALSE ; /* -- crf */
          if (++count >= display->height)
          {
            dprintf(display,"More : (y/n)");
            fflush(display->fileout);
	    do
	    {
	      no_read = Read(Heliosno(display->filein),c,1,OneSec);
	    }while(no_read == 0);

            if (c[0] != 'y' AND c[0] != 'Y')
            {
              break;
            }
            count = 0;
          }
          dprintf(display, "%s", buffer);
          if (testbuffer(buffer)) break;
        } until (fgets(buffer, 100, file) == NULL);
        break;
      }
    }
/*
-- crf : give message if nothing found
*/
      if (nothing_found)
          dprintf (display, "Nothing found for \"%s\"\n", topic) ;

    fclose(file);
  }
  dend(display, TRUE);
}


PRIVATE int testbuffer (char buffer[100])
{
  int i;
  int k = 0;
  
  
  for (i = 0; i <= 7; i++)
  {
    if ( buffer[i] == '-')
    {
      k++;
    }
  }
  if (k >= 5) return TRUE;
  else return FALSE;
}
