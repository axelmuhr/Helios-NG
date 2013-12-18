/**
*
* Title:  Helios Shell - Administration.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/
#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/shell/RCS/shell.c,v 1.24 1993/08/12 15:19:45 nickc Exp $";
#endif

#include "shell.h"
#ifndef __TRAN
#include <process.h>
#endif
#include <sys/wait.h>


int main(
	 int argc,
	 char *argv[] )
{
#ifdef DEBUGGING
  DebugInit();
  DEBUG("main(%V)",argv);
  DEBUG("stdin = %S", Heliosno (stdin));
  DEBUG("stdout= %S", Heliosno (stdout));
  DEBUG("stderr= %S", Heliosno (stderr));
  debugging = 1;
#endif

#ifndef __TRAN
  /* get better keyboard response on heavily loaded system */
  SetPriority(ServerPri);
#endif

  signal(SIGINT, SIG_IGN);
  if(!catch())				/* catch errors in rc files */
       initialise(argc, argv);
  shell();
  logout(OK);
  return OK;
}

void shell()
{
#ifdef DEBUGGING
  DEBUG("shell()");
#endif
  if (catch() AND (singleline OR exitonerror)) return;
  if (singleline) ignore docmdline();
  else while (docmdline());
}

BOOL docmdline()
{
  char line[LINE_MAX + 1];

#ifdef DEBUGGING
  DEBUG("docmdline()");
#endif
  lineposition = note();
  pendingjobs();
  if (getline(line, FALSE) == NULL)
  {
    unless (interactive AND findvar("ignoreeof")) return FALSE;
    printf("^D\n");
    if (login) error(ERR_USELOGOUT, NULL);
    else error(ERR_USEEXIT, NULL);
    recover();
  }
  ignore runcmdline(line);
  return TRUE;
}

int runcmdline(char *line )
{
  ARGV argv;
  int code = 0;

#ifdef DEBUGGING
  DEBUG("runcmdline('%s')", line);
#endif

  usingcdl = (getvar("cdl") == NULL) ? FALSE : TRUE;
#ifdef HELIOS
  _posixflags((usingcdl ? PE_BLOCK : PE_UNBLOCK), PE_RemExecute);
#endif
  argv = historysub(buildargv(makeargv(line)));
  if (initparse(argv))
  {
    globcmd = readcmdlist(0);
    if (parsingerror) recover();
    unless (globcmd == NULL)
    {
      tidyupparse();
      if (mode & MODE_EXECUTE)
      {
        code = runcmdlist(globcmd);
        if (exitonerror AND code) logout(code);
      }
      freecmd(globcmd);
      globcmd = NULL;
    }
  }
  return code;
}

void initialise(
		int argc,
		char **argv )
{
  char path[PATH_MAX + 1];
  char *cmdline = NULL;
  BOOL cdl = FALSE;
  BOOL echo = FALSE;
  BOOL verbose = FALSE;
  BOOL standard = FALSE;
  BOOL parseonly = FALSE;

  login = (BOOL)(*argv++[0] == '-'); argc--;
  shellpid = getpid();
  sysinit();
  InitList(&aliaslist);
  InitList(&historylist);
  InitList(&varlist);
  InitList(&dirlist);
  InitList(&joblist);
  InitList(&filelist);
  InitList(&looplist);

  while (argc > 0 AND **argv == '-')
  {
    char c;
    char *arg = *argv++; argc--;

    arg++;
    until ((c = *arg++) == '\0')
    {
      switch (c)
      {
        case 'c':
        if (*arg == '\0')
        {
          if (argc == 0) exit(1);
          cmdline = *argv++; argc--;
        }
        else cmdline = arg;
        break;

        case 'd':
        debugging = TRUE;
        continue;

        case 'e':
        exitonerror = TRUE;
        continue;

        case 'f':
        fast = TRUE;
        continue;

        case 'i':
        interactive = TRUE;
        continue;

        case 'n':
        parseonly = TRUE;
        continue;

        case 's':
        standard = TRUE;
        continue;

        case 't':
        singleline = TRUE;
        continue;

        case 'v':
        verbose = TRUE;
        continue;

        case 'x':
        echo = TRUE;
        continue;

        case 'z':
        cdl = TRUE;
        continue;

        case 'V':
        set("verbose", makeargv(""));
        continue;

        case 'X':
        set("echo", makeargv(""));
        continue;

        case 'Z':
        set("cdl", makeargv(""));
        continue;
      }
      break;
    }
  }
  if (argc == 0 OR cmdline OR interactive OR standard OR singleline)
  {
    filename = NULL;
    inputfile = stdin;
  }
  else
  {
    argc--;
    filename = *argv++;
    if ((inputfile = fopen(filename, "r")) == NULL)
    {
      syserr(filename);
      exit(1);
    }
    fcntl(fileno(inputfile), F_SETFD, FD_CLOEXEC);
  }
  if (argc > 0) set("argv", dupargv(argv));
  else set("argv", nullargv());
  set("autologout", nummakeargv(10));
  unless (getenv("CDL") == NULL) set("cdl", makeargv(""));
#ifdef HELIOS
  set("console", envmakeargv("CONSOLE"));
#endif
  set("cwd", makeargv(getcwd(path, PATH_MAX)));
  set("home", envmakeargv("HOME"));
#ifdef HELIOS
  MachineName(path);
  set("machine", makeargv(path));
#endif
  set("path", envmakeargv("PATH"));
  set("prompt", makeargv(PROMPT));
  set("shell", makeargv(SHELL_CMD));
  set("status", nummakeargv(OK));
  set("term", envmakeargv("TERM"));
  set("user", envmakeargv("USER"));

#ifdef DEBUGGING
  DEBUG("environment set");
#endif
  unless (interactive) interactive = (BOOL) isatty(fileno(inputfile));
  unless (parseonly) setmode(MODE_EXECUTE);
  unless (fast)
  {
    unsetmode(MODE_HISTORY);
    if(!source(homename(path, CSHRC_FILE)))
	ignore source(homename(path,CSHRC_FILE_V11));
    if (login) 
	if (!source(homename(path, LOGIN_FILE)))
		ignore source(homename(path, LOGIN_FILE_V11));
    if(!record(homename(path, HISTORY_FILE)))
    	ignore record(homename(path, HISTORY_FILE_V11));
#if 0
    /* experimental: PAB doesn't think this time consuming hash is required */
    hash();
#endif
  }
  setmode(MODE_HISTORY);
  if (verbose) set("verbose", makeargv(""));
  if (echo) set("echo", makeargv(""));
  if (cdl) set("cdl", makeargv(""));
  siginit();
  unless (cmdline == NULL)
  {
    int code;

    if ((code = catch()) == 0) code = runcmdline(cmdline);
    logout(code);
  }
}

void logout(int code)
{
#ifdef DEBUGGING
  DEBUG("logout(%d)", code);
#endif

  unless (fast)
  {
    char path[PATH_MAX + 1];
    char *savehist;

    unless ((savehist = getvar("savehist")) == NULL)
    {
      int count;

      if ((count = atoi(savehist)) > 0)
      {
        FILE *file;
	file = fopen(homename(path, HISTORY_FILE), "w");
	if ( file == NULL )
          file = fopen(homename(path, HISTORY_FILE_V11), "w");
        unless (file == NULL)
        {
          fputsublist(file, &historylist, count, TRUE, TRUE, FALSE);
          fclose(file);
        }
      }
    }
    if (login)
      if (!source(homename(path, LOGOUT_FILE)))
	ignore source(homename(path, LOGOUT_FILE_V11));
  }
  freecmd(globcmd);
  freeargv(globargv);

  WalkList(&aliaslist, (WordFnPtr)freesubnode);
  WalkList(&historylist, (WordFnPtr)freesubnode);
  WalkList(&varlist, (WordFnPtr)freesubnode);
  WalkList(&dirlist, (WordFnPtr)freedirnode);
  if (login) WalkList(&joblist, (WordFnPtr)killjob);
  WalkList(&joblist, (WordFnPtr)freejob);
  freelinevector();
  unhash();
  systidy();
  exit(code);
}

char *homename(
	       char *path,
	       char *name )
{
  char *home;

  unless ((home = getvar("home")) == NULL) formfilename(path, home, name);
  else strcpy(path, name);
  return path;
}

BOOL source(char *name )
{
  FILE *file;

#ifdef DEBUGGING
  DEBUG("source('%s')", name);
#endif
  if ((file = fopen(name, "r")) == NULL) return FALSE;
  newfile(file);
  while (docmdline()) {
	if(testbreak())
		{
		innewfile = FALSE;
		recover();
		}
  }

  oldfile();
  return TRUE;
}

BOOL record(char *name)
{
  char line[LINE_MAX + 1];
  FILE *file;

#ifdef DEBUGGING
  DEBUG("record(%s)",name);
#endif
  if ((file = fopen(name, "r")) == NULL) return FALSE;
  until (fgets(line, LINE_MAX, file) == NULL)
  /* ACE: buildargv can error on unmatched ' ! */
    addevent(buildargv(makeargv(line)));
  fclose(file);
  return TRUE;
}

BOOL getinterpreter(
		    char * interp,
		    char * name )
{
  int fd, length, i;
  int index = 0;
  char *shell;

#ifdef DEBUGGING
  DEBUG("getinterpreter = %");
#endif
  if ((fd = open(name, O_RDONLY)) == -1) return FALSE;
  if ((length = read(fd, interp, LINE_MAX)) < 0) return FALSE;
  close(fd);
  for (i = 0; i < length; i++)
  {
    unless (isprint(interp[i]) OR isspace(interp[i])) return FALSE;
  }
  interp[length] = '\0';
  if (interp[0] == '#' AND interp[1] == '!')
  {
    i = 2;
    while (isspace(interp[i]) AND isprint(interp[i])) i++;
    until (isspace(interp[i]) OR interp[i] == '\0')
      interp[index++] = interp[i++];
  }
  if (index > 0)
  {
    interp[index] = '\0';
#ifdef DEBUGGING
    DEBUG ("%s", interp);
#endif
    return TRUE;
  }
  if ((shell = getvar("shell")) == NULL) shell = SHELL_CMD;
  strcpy(interp, shell);
#ifdef DEBUGGING
  DEBUG ("%s", interp);
#endif
  return TRUE;
}

void executecmd(
		char *name,
		ARGV argv )
{
#ifdef DEBUGGING
  DEBUG("executecmd(%s (%s %s %s) %V)", name,
     fdstream(0)->Name, fdstream(1)->Name, fdstream(2)->Name, argv);
#endif
  execvp(name, argv);                        /*JD:change to execvp fromexecv*/
  if (errno == 0)
    errno = ENOMEM;		/* added by NC to cope with bug in kernel */
#ifdef DEBUGGING
  DEBUG("execvp failed, errno = %d", errno);
#endif
  if (errno == ENOEXEC)
  {
    char interp[PATH_MAX + 1];

    if (getinterpreter(interp, name))
    {
      char *newargv[100];
      int i;

      newargv[0] = interp;
      newargv[1] = name;
      for (i = 2; i < 100; i++)
      {
      	if ((newargv[i] = argv[i - 1]) == NULL) break;
      }
      newargv[i] = NULL;
      execvp(interp, newargv);
      if (errno == 0)
        errno = ENOMEM;		/* added by NC to cope with bug in kernel */
#ifdef DEBUGGING
      DEBUG("execvp failed, errno = %d", errno);
#endif
      syserr(interp);
      _exit(1);
    }
    errno = ENOEXEC;
  }
  unless (errno == ENOENT)
  {
    syserr(name);
    _exit(1);
  }
}

int waitforcmd(int pid )
{
  int wpid, status, code;

#ifdef DEBUGGING
  DEBUG("waitforcmd(%d)", pid);
#endif
  if (pid == -1)
	{
	if(testbreak())
	      recover();
	return(-1);
	}
  waitwrpid = pid;
  until ((wpid = wait2(&status, WUNTRACED)) == pid OR (wpid == -1 && errno != EINTR)) 
    if( wpid != -1 ) notifyjob(wpid, status);
  unless (wpid == -1) notifyjob(pid, status);
  waitwrpid = 0;
  set("status", nummakeargv(code = highbyte(status)));
  if(testbreak())
	recover();
#ifdef HELIOS
  ctrlcbegin();
#endif
  return code;
}

int runcmdlist(CMD *cmd)
{
  int code = 0;
  int pid;

#ifdef DEBUGGING
 DEBUG("runcmdlist()");
#endif
  for (; cmd; cmd = cmd->next)
  {
    switch (cmd->op)
    {
      case T_SEMICOLON:
      code = waitforcmd(runpipeline(cmd->This));
      break;

      case T_AMPERSAND:
      backgroundtask = TRUE;
      if (isdelimitor(cmd->This->op)) pid = runlist(cmd->This, NULL, READ, WRITE);
      else pid = runpipeline(cmd->This);
      unless (pid == -1) newjob(cmd->This, pid);
      backgroundtask = FALSE;
      break;

      case T_AND:
      unless ((code = waitforcmd(runpipeline(cmd->This))) == 0) return code;
      break;

      case T_OR:
      if ((code = waitforcmd(runpipeline(cmd->This))) == 0) return code;
      break;

      default:
      return waitforcmd(runpipeline(cmd));
    }
  }
  return code;
}

int runpipeline(CMD *cmd)
{
  int fd = READ;
  int fds[2];
  int pid = -1;

#ifdef DEBUGGING
  DEBUG("runpipeline()");
#endif
  for (; cmd; cmd = cmd->next)
  {
    switch (cmd->op)
    {
#ifdef CDL
      case T_PAR:
      case T_FARM:
      case T_SUBORDINATE:
      case T_REVPIPE:
#endif
      case T_PIPE:
#ifdef DEBUGGING
      DEBUG("PIPE");
#endif
#ifdef CDL
      if (usingcdl) return runtaskforce(cmd);
#endif
#ifdef HELIOS
      if (cmd->This->op == T_SIMPLE AND ((ARGV)cmd->This->This)[0] != NULL AND
          findbuiltin(((ARGV)cmd->This->This)[0]) != NULL)
      {
      	if (fifo(fds) == -1)
      	{
          int save_err = errno;
          unless (fd == READ) close(fd);
	  errno = save_err;
          syserr(NULL);
          recover();
        }
      }
      else
#endif
      if (pipe(fds) == -1)
      {
        int	save_err = errno;
        unless (fd == READ) close(fd);
	errno = save_err;
        syserr(NULL);
        recover();
      }
      fcntl(fds[READ], F_SETFD, FD_CLOEXEC);
      ignore runcmd(cmd->This, fd, fds[WRITE]);
      close(fds[WRITE]);
      unless (fd == READ) close(fd);
      fd = fds[READ];
#ifdef DEBUGGING
      DEBUG("talk to cmd on %d", fd);
#endif
      break;

      default:
      pid = runcmd(cmd, fd, WRITE);
      unless (fd == READ) close(fd);
      return pid;
    }
  }
  return pid;
}

int runcmd(
	   CMD *cmd,
	   int  ifd,
	   int  ofd )
{
#ifdef DEBUGGING
  DEBUG("runcmd()");
#endif
  if (cmd->op == T_LIST) return runlist(cmd->This, cmd->ioinfo, ifd, ofd);
#ifdef CDL
  if (usingcdl AND cmd->next != NULL) return runtaskforce(cmd);
#endif
  return runsimplecmd(dupargv((ARGV)cmd->This), cmd->ioinfo, ifd, ofd, CHECKBUILTIN);
}

int runlist(
	    CMD *cmd,
	    IOINFO *ioinfo,
	    int ifd,
	    int ofd )
{
  int pid;

#ifdef DEBUGGING
  DEBUG("runlist()");
#endif
#ifdef UNIX
  if ((pid = fork()) == 0)
  {
    unless (redirect(ioinfo, ifd, ofd)) _exit(1);
    shellpid = getpid(); /* ACE: Is this necessary ? */
    _exit(runcmdlist(cmd));
  }
#else
  if ((pid = vfork()) == 0)
  {
    char buffer[WORD_MAX + 1];

    unless (redirect(ioinfo, ifd, ofd)) _exit(1);
    buffer[0] = '\0';
    sputcmd(buffer, cmd);
    execl(SHELL_CMD, "shell", "-fc", buffer, NULL);
    syserr(NULL);
    _exit(1);
  }
#endif
  if(backgroundtask)
	ignore setpgid(pid, pid);
  if (pid == -1)
  {
    syserr(NULL);
    recover();
  }
  return pid;
}

void savefds(int *fds)
{
  int i;

  for (i = 0; i < 3; i++) fds[i] = dup(i);
  if(!fdssaved)
	for(i = 0 ; i < 3 ; i++)
		sfds[i] = fds[i];
  fdssaved++;
}

void restorefds(int *fds)
{
  int i;

  for (i = 0; i < 3; i++)
  {
    close(i);
    dup(fds[i]);
    close(fds[i]);
  }
  fdssaved--;
}

int runbuiltin(
	       int (*func)(),
	       ARGV argv,
	       IOINFO *ioinfo,
	       int ifd,
	       int ofd )
{
  int code, pid;
  int fds[3];

#ifdef DEBUGGING
  DEBUG("runbuiltin(fn,%s)",argv[0]);
#endif

  if (needsfullsubs(func)) argv = fullsub(argv);
  else argv = smallsub(argv);
  savefds(fds);
  unless (redirect(ioinfo, ifd, ofd))
  {
    restorefds(fds);
    freeargv(argv);
    recover();
  }
  if (findvar("echo")) fputargv(stderr, argv, TRUE);
  code = (*func)(lenargv(argv), argv);
  freeargv(argv);     /* are these freed if (*func() longjmps ?? */
  restorefds(fds);
  if ((pid = vfork()) == 0) _exit(code);
  if (pid == -1)
  {
    syserr(NULL);
    recover();
  }
  return pid;
}

int runsimplecmd(
		 ARGV argv,
		 IOINFO *ioinfo,
		 int ifd,
		 int ofd,
		 int bcmdcheck )
{
  BUILTIN *builtin;
  int pid = -1;

/* move 'name' declaration to here from inner 'until' loop */
/* to resolve memory leakage problem B433                  */
/* MJT 08/10/90                                            */

  char name[PATH_MAX + 1];

  
#ifdef DEBUGGING
  DEBUG("runsimplecmd(%s)",argv[0]);
#endif
  if (argv[0] == NULL)
  {
    freeargv(argv);
    return -1;
  }
  if(testbreak())
	recover();

  if(bcmdcheck)		/* if asked to check for builtin commands */
  unless ((builtin = findbuiltin(argv[0])) == NULL) {
    return runbuiltin(builtin->func, argv, ioinfo, ifd, ofd);
  }
  argv = fullsub(argv);
#if 0
  if(lenargv(argv) + lenargv(environ) > MAX_ARGV)
	{	
	fprintf(stderr, "Arg list too long\n");
	fflush(stderr);
	freeargv(argv);
	return pid;
	}
#endif
  if ((pid = vfork()) == 0)
  {
    unless (redirect(ioinfo, ifd, ofd)) _exit(1);
    if (findvar("echo")) fputargv(stderr, argv, TRUE);
    if (isabspath(argv[0]))
    {
#ifdef HELIOS
      if (lookforcmd(".", argv[0]))
#endif
      executecmd(argv[0], argv);
    }
    else
    {
      ARGV pathargv;
      char *path;

      unless ((pathargv = findvar("path")) == NULL)
      {
        until ((path = *pathargv++) == NULL)
        {
          if (lookforcmd(path, argv[0]))
          {
            formfilename(name, path, argv[0]);
            executecmd(name, argv);
          }
        }
      }
    }
    error(ERR_NOTFOUND, argv[0]);
    _exit(1);
  }
  if(backgroundtask)
	ignore setpgid(pid, pid);
  freeargv(argv);
  if (pid == -1)
  {
    syserr(NULL);
    recover();
  }
  return pid;
}

BOOL redirect(
	      IOINFO *ioinfo,
	      int ifd,
	      int ofd )
{
  OPENINFO *inputinfo = (ioinfo == NULL) ? NULL : ioinfo->input;
  OPENINFO *outputinfo = (ioinfo == NULL) ? NULL : ioinfo->output;
  OPENINFO *diaginfo = (ioinfo == NULL) ? NULL : ioinfo->diag;
  char *name;

#ifdef DEBUGGING
 DEBUG("redirect(%d,%d) (%x %x %x)", ifd, ofd, inputinfo, outputinfo, diaginfo);
#endif
  unless (ifd == READ)
  {
    unless (inputinfo == NULL)
    {
      error(ERR_INPUT, NULL);
      return FALSE;
    }
    close(READ);
    dup(ifd);
    close(ifd);
  }
  unless (ofd == WRITE)
  {
    unless (outputinfo == NULL)
    {
      error(ERR_OUTPUT, NULL);
      return FALSE;
    }
    close(WRITE);
    dup(ofd);
    close(ofd);
  }
  if (diaginfo != NULL AND outputinfo != NULL AND outputinfo->flags & FLAG_STDERR)
  {
    error(ERR_DIAGNOSTIC, NULL);
    return FALSE;
  }
  unless (diaginfo == NULL)
  {
    int flags = O_WRONLY | O_CREAT;
    ARGV argv = fullsub(makeargv(diaginfo->name));

    freememory((int *)diaginfo->name);
    diaginfo->name = name = strdup(argv[0]);
    freeargv(argv);
    if (diaginfo->op == T_APPENDDIAG) flags |= O_APPEND;
    else flags |= O_TRUNC;
    if (findvar("noclobber") AND (diaginfo->flags & FLAG_CLOBBER) == 0)
      flags |= O_EXCL;
    close(DIAGNOSTIC);
    if (open(name, flags, 0666) == -1)
    {
      syserr(name);
      return FALSE;
    }
  }
  unless (outputinfo == NULL)
  {
    int flags = O_WRONLY | O_CREAT;
    ARGV argv = fullsub(makeargv(outputinfo->name));

    freememory((int *)outputinfo->name);
    outputinfo->name = name = strdup(argv[0]);
    freeargv(argv);
    if (outputinfo->op == T_APPEND) flags |= O_APPEND;
    else flags |= O_TRUNC;
    if (findvar("noclobber") AND (outputinfo->flags & FLAG_CLOBBER) == 0)
      flags |= O_EXCL;
    close(WRITE);
    if (open(name, flags, 0666) == -1)
    {
      syserr(name);
      return FALSE;
    }
    if (outputinfo->flags & FLAG_STDERR)
    {
      close(DIAGNOSTIC);
      dup(WRITE);
    }
  }
  unless (inputinfo == NULL)
  {
    if (inputinfo->op == T_READ)
    {
      ARGV argv = fullsub(makeargv(inputinfo->name));

      freememory((int *)inputinfo->name);
      inputinfo->name = name = strdup(argv[0]);
      freeargv(argv);
      close(READ);
      if (open(name, O_RDONLY, 0) == -1)
      {
      	syserr(name);
      	return FALSE;
      }
    }
    else
    {
      FILE *file;
      char *terminator = inputinfo->name;
      char line[LINE_MAX + 1];
      int length = strlen(terminator);

      if ((file = fopen(TEMP_FILE, "w")) == NULL)
      {
      	syserr(TEMP_FILE);
      	return FALSE;
      }
      forever
      {
        unless (getline(line, TRUE))
        {
          error(ERR_TERMINATOR, terminator);
          fclose(file);
          return FALSE;
        }
        if (strnequ(terminator, line, length) AND line[length] == '\n') break;
        fputs(line, file);
      } 
      fclose(file);
      close(READ);
      if (open(TEMP_FILE, O_RDONLY, 0) == -1)
      {
      	syserr(TEMP_FILE);
      	return FALSE;
      }
    }
  }
  return TRUE;
}

#ifdef TEST_FDS
#undef open
#undef close
#undef dup
#undef pipe

int myopen(char *name, int mode, int pmode)
{
  int fd = open(name, mode, pmode);

  DEBUG("open('%s') -> %d", name, fd);
  return fd;
}

int myclose(int fd)
{
  int err = close(fd);

  DEBUG("close(%d) -> %d", fd, err);
  return err;
}

int mydup(int fd)
{
  int newfd = dup(fd);

  DEBUG("dup(%d) -> %d", fd, newfd);
  return newfd;
}

int mypipe(int fd[2])
{
  int err = pipe(fd);

  DEBUG("pipe() -> %d (%d, %d)", err, fd[0], fd[1]);
  return err;
}
#endif

#ifdef DEBUGGING
#include "dodebug.c"
#endif

#ifdef CDL
#include "cdl.c"
#endif

