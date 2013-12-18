/* Job execution and handling for GNU Make.
Copyright (C) 1988, 1989 Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GNU Make is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Make; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "make.h"
#include "commands.h"
#include "job.h"
#include "file.h"
#include "variable.h"
#include <errno.h>

static char *rcsid = "$Header: /usr/perihelion/Helios/cmds/gnu/gmake/make-3.57/RCS/job.c,v 1.1 90/08/28 14:37:13 james Exp $"; 

extern int errno;

#if	defined(USG) && !defined(HAVE_VFORK)
#define	vfork	fork
#define	VFORK_NAME	"fork"
#else	/* Have vfork or not USG.  */
#define	VFORK_NAME	"vfork"
#endif	/* USG and don't have vfork.  */
extern int vfork ();

#if	defined(HAVE_SYS_WAIT) || !defined(USG)
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
extern int wait3 ();
#endif

#ifdef HELIOS
#define NSIG 32
#define SIGCLD (SIGUSR2+1)
#define SIGIOT (SIGUSR2+2)
#define SIGBUS (SIGUSR2+3)
#endif

#if	defined(WTERMSIG) || (defined(USG) && !defined(HAVE_SYS_WAIT))
#define	WAIT_T int

#ifndef	WTERMSIG
#define WTERMSIG(x) ((x) & 0x7f)
#endif
#ifndef	WCOREDUMP
#define WCOREDUMP(x) ((x) & 0x80)
#endif
#ifndef	WEXITSTATUS
#define WEXITSTATUS(x) (((x) >> 8) & 0xff)
#endif
#ifndef	WIFSIGNALED
#define WIFSIGNALED(x) (WTERMSIG (x) != 0)
#endif
#ifndef	WIFEXITED
#define WIFEXITED(x) (WTERMSIG (x) == 0)
#endif

#else	/* WTERMSIG not defined and have <sys/wait.h> or not USG.  */

#define WAIT_T union wait
#define WTERMSIG(x) ((x).w_termsig)
#define WCOREDUMP(x) ((x).w_coredump)
#define WEXITSTATUS(x) ((x).w_retcode)
#ifndef	WIFSIGNALED
#define	WIFSIGNALED(x)	(WTERMSIG(x) != 0)
#endif

#endif	/* WTERMSIG defined or USG and don't have <sys/wait.h>.  */


extern int dup2 ();
extern int fork (), wait (), execve ();
extern void _exit ();
extern int geteuid (), getegid ();

#if	!defined(USG) && !defined(sigmask)
#define sigmask(sig) (1 << ((sig) - 1))
#endif

#ifndef USG
extern int getdtablesize ();
#else
#include <sys/param.h>
#define getdtablesize() NOFILE
#endif

extern void wait_to_start_job ();
extern int start_remote_job_p ();
extern int start_remote_job (), remote_status ();


#if	defined(USG) && !defined(HAVE_SIGLIST)
static char *sys_siglist[NSIG];
void init_siglist ();
#else	/* Not USG and or HAVE_SIGLIST.  */
extern char *sys_siglist[];
#endif	/* USG and not HAVE_SIGLIST.  */

int child_handler ();
static void free_child (), start_job ();

/* Chain of all children.  */

struct child *children = 0;

/* Number of children currently running.  */

unsigned int job_slots_used = 0;

/* Nonzero if the `good' standard input is in use.  */

static int good_stdin_used = 0;

/* Write an error message describing the exit status given in
   EXIT_CODE, EXIT_SIG, and COREDUMP, for the target TARGET_NAME.
   Append "(ignored)" if IGNORED is nonzero.  */

static void
child_error (target_name, exit_code, exit_sig, coredump, ignored)
     char *target_name;
     int exit_code, exit_sig, coredump;
     int ignored;
{
  char *ignore_string = ignored ? " (ignored)" : "";

  if (exit_sig == 0)
    error ("*** [%s] Error %d%s", target_name, exit_code, ignore_string);
  else
    {
      char *coredump_string = coredump ? " (core dumped)" : "";
      if (exit_sig > 0 && exit_sig < NSIG)
	error ("*** [%s] %s%s",
	       target_name, sys_siglist[exit_sig], coredump_string);
      else
	error ("*** [%s] Signal %d%s", target_name, exit_sig, coredump_string);
    }
}

extern void block_remote_children (), unblock_remote_children ();

/* Block the child termination signal.  */

void
block_children ()
{
#ifdef USG
  /* Ignoring SIGCLD makes wait always return -1.
     Using the default action does the right thing.  */
  (void) signal (SIGCLD, SIG_DFL);
#else
  (void) sigblock (sigmask (SIGCHLD));
#endif

  block_remote_children ();
}

/* Unblock the child termination signal.  */
void
unblock_children ()
{
#ifdef	USG
  (void) signal (SIGCLD, child_handler);
#else
  (void) sigsetmask (sigblock (0) & ~sigmask (SIGCHLD));
#endif

  unblock_remote_children ();
}

extern int shell_function_pid, shell_function_completed;

/* Handle a child-termination signal (SIGCHLD, or SIGCLD for USG),
   storing the returned status and the new command state (`cs_finished')
   in the `file' member of the `struct child' for the dead child,
   and removing the child from the chain.

   If we were called as a signal handler, SIG should be SIGCHLD
   (SIGCLD for USG).  If instead it is zero, we were called explicitly
   and should block waiting for running children.
   If SIG is < 0, - SIG is the maximum number of children to bury (record
   status of and remove from the chain).  */

int
child_handler (sig)
     int sig;
{
  WAIT_T status;
  unsigned int dead_children = 0;

  if (sig > 0)
    block_remote_children ();

  while (1)
    {
      int remote = 0;
      register int pid;
      int exit_code, exit_sig, coredump;
      register struct child *lastc, *c;
      int child_failed;

      /* First, check for remote children.  */
      pid = remote_status (&exit_code, &exit_sig, &coredump, 0);
      if (pid < 0)
	{
	  /* No remote children.  Check for local children.  */

#if	!defined(USG) || defined(HAVE_SYS_WAIT)
	  if (sig > 0)
	    pid = wait3 (&status, WNOHANG, (struct rusage *) 0);
	  else
	    pid = wait (&status);
#else	/* USG and don't HAVE_SYS_WAIT.  */
	  /* System V cannot do non-blocking waits, so we have two
	     choices if called as a signal handler: handle only one
	     child (there may be more if the signal was blocked),
	     or block waiting for more.  The latter option makes
	     parallelism useless, so we must choose the former.  */
	  pid = wait (&status);
#endif	/* HAVE_SYS_WAIT or not USG.  */

	  if (pid <= 0)
	    /* No local children.  */
	    break;
	  else
	    {
	      /* Chop the status word up.  */
	      exit_code = WEXITSTATUS (status);
	      exit_sig = WIFSIGNALED (status) ? WTERMSIG (status) : 0;
	      coredump = WCOREDUMP (status);
	    }
	}
      else
	/* We got a remote child.  */
	remote = 1;

      /* Check if this is the child of the `shell' function.  */
      if (!remote && pid == shell_function_pid)
	{
	  /* It is.  Leave an indicator for the `shell' function.  */
	  if (exit_sig == 0 && exit_code == 127)
	    shell_function_completed = -1;
	  else
	    shell_function_completed = 1;

	  /* Check if we have reached our quota of children.  */
	  ++dead_children;
	  if (sig < 0 && dead_children == -sig)
	    break;
#if	defined(USG) && !defined(HAVE_SYS_WAIT)
	  else if (sig > 0)
	    break;
#endif
	  else
	    continue;
	}

      child_failed = exit_sig != 0 || exit_code != 0;

      /* Search for a child matching the deceased one.  */
      lastc = 0;
      for (c = children; c != 0; lastc = c, c = c->next)
	if (c->remote == remote && c->pid == pid)
	  break;

      if (c == 0)
	{
	  /* An unknown child died.  */
	  char buf[100];
	  sprintf (buf, "Unknown%s job %d", remote ? " remote" : "", pid);
	  if (child_failed)
	    child_error (buf, exit_code, exit_sig, coredump,
			 ignore_errors_flag);
	  else
	    error ("%s finished.", buf);
	}
      else
	{
	  /* If this child had the good stdin, say it is now free.  */
	  if (c->good_stdin)
	    good_stdin_used = 0;

	  if (child_failed && !c->noerror && !ignore_errors_flag)
	    {
	      /* The commands failed.  Write an error message,
		 delete non-precious targets, and abort.  */
	      child_error (c->file->name, exit_code, exit_sig, coredump, 0);
	      c->file->update_status = 1;
	      if (exit_sig != 0)
		delete_child_targets (c);
	    }
	  else
	    {
	      if (child_failed)
		{
		  /* The commands failed, but we don't care.  */
		  child_error (c->file->name,
			       exit_code, exit_sig, coredump, 1);
		  child_failed = 0;
		}

	      /* If there are more commands to run, try to start them.  */
	      start_job (c);
	      switch (c->file->command_state)
		{
		case cs_running:
		  /* Successfully started.  Loop to reap more children.  */
		  continue;

		case cs_finished:
		  if (c->file->update_status != 0)
		    {
		      /* We failed to start the commands.  */
		      delete_child_targets (c);
		    }
		  break;

		default:
		  error ("internal error: `%s' command_state \
%d in child_handler", c->file->name);
		  abort ();
		  break;
		}
	    }

	  /* Set the state flag to say the commands have finished.  */
	  c->file->command_state = cs_finished;
	  notice_finished_file (c->file);

	  /* Remove the child from the chain and free it.  */
	  if (lastc == 0)
	    children = c->next;
	  else
	    lastc->next = c->next;
	  free_child (c);

	  /* There is now another slot open.  */
	  --job_slots_used;

	  /* If the job failed, and the -k flag was not given, die.  */
	  if (child_failed && !keep_going_flag)
	    die (1);

	  /* See if we have reached our quota for blocking.  */
	  ++dead_children;
	  if (sig < 0 && dead_children == -sig)
	    break;
#if	defined(USG) && !defined(HAVE_SYS_WAIT)
	  else if (sig > 0)
	    break;
#endif
	}
    }

#ifdef	USG
  if (sig > 0)
    (void) signal (sig, child_handler);
#endif

  if (sig > 0)
    unblock_remote_children ();

  return 0;
}


/* Wait for N children, blocking if necessary.
   If N is zero, wait until we run out of children.
   If ERR is nonzero and we have any children to wait for,
   print a message on stderr.  */

void
wait_for_children (n, err)
     unsigned int n;
     int err;
{
  block_children ();

  if (err && (children != 0 || shell_function_pid != 0))
    {
      fflush (stdout);
      error ("*** Waiting for unfinished jobs....");
    }

  /* Call child_handler to do the work.  */
  (void) child_handler (- (int) n);

  unblock_children ();
}

/* Free the storage allocated for CHILD.  */

static void
free_child (child)
     register struct child *child;
{
  if (child->commands != 0)
    free (child->commands);
  free ((char *) child);
}

/* Start a job to run the commands specified in CHILD.
   CHILD is updated to reflect the commands and ID of the child process.  */

static void
start_job (child)
     register struct child *child;
{
  static int bad_stdin = -1;
  char *end;
  register char *p;
  int backslash;
  char noprint = 0, recursive;
  char **argv;

  if (child->command_ptr == 0 || *child->command_ptr == '\0')
    /* There are no more lines in the expansion of this line.  */
    if (child->file->cmds->command_lines[child->command_line] == 0)
      {
	/* There are no more lines to be expanded.  */
	child->command_ptr = 0;
	child->file->command_state = cs_finished;
	child->file->update_status = 0;
	return;
      }
    else
      {
	/* Expand and run the next line.  */
	child->command_ptr = child->commands
	  = allocated_variable_expand_for_file
	    (child->file->cmds->command_lines[child->command_line++],
	     child->file);
      }

  /* Set RECURSIVE if the unexpanded line contains $(MAKE).  */
  recursive = child->file->cmds->lines_recurse[child->command_line - 1];

  /* Find the end of this line.  Backslash-newlines don't mean the end.  */

  end = child->command_ptr;
  while (*end != '\0')
    {
      p = index (end, '\n');
      if (p == 0)
	{
	  end += strlen (end);
	  break;
	}

      end = p;
      backslash = 0;
      while (*--p == '\\')
	backslash = !backslash;

      if (backslash)
	{
	  ++end;
	  /* If there is a tab after a backslash-newline,
	     remove it, since it was most likely used to line
	     up the continued line with the previous one.  */
	  if (*end == '\t')
	    strcpy (end, end + 1);
	}
      else
	break;
    }

  p = child->command_ptr;

  if (*end == '\0')
    child->command_ptr = 0;
  else
    {
      *end = '\0';
      child->command_ptr = end + 1;
    }

  child->noerror = 0;
  while (*p != '\0')
    {
      if (*p == '@')
	noprint = 1;
      else if (*p == '-')
	child->noerror = 1;
      else if (*p == '+')
	recursive = 1;
      else if (*p != ' ' && *p != '\t')
	break;
      ++p;
    }

  /* If -q was given, just say that updating `failed'.  */
  if (question_flag && !recursive)
    goto error;

  /* There may be some preceding whitespace left if there
     was nothing but a backslash on the first line.  */
  p = next_token (p);

  if (*p == '\0')
    /* There were no commands on this line.  Go to the next.  */
    {
      start_job (child);
      return;
    }

  /* Print out the command.  */

  if (just_print_flag || (!noprint && !silent_flag))
    puts (p);

  /* If -n was given, recurse to get the next line in the sequence.  */

  if (just_print_flag && !recursive)
    {
      start_job (child);
      return;
    }

  /* Collapse backslash-newlines in this line.  */

  collapse_continuations (p);

  /* Figure out an argument list from this command line.  */

  argv = construct_command_argv (p, child->file);

  /* Flush the output streams so they won't have things written twice.  */

  fflush (stdout);
  fflush (stderr);
  
  /* Set up a bad standard input that reads from a broken pipe.  */

  if (bad_stdin == -1)
    {
      /* Make a file descriptor that is the read end of a broken pipe.
	 This will be used for some children's standard inputs.  */
      int pd[2];
      if (pipe (pd) == 0)
	{
	  /* Close the write side.  */
	  (void) close (pd[1]);
	  /* Save the read side.  */
	  bad_stdin = pd[0];
	}
    }

  /* Decide whether to give this child the `good' standard input
     (one that points to the terminal or whatever), or the `bad' one
     that points to the read side of a broken pipe.  */

  child->good_stdin = !good_stdin_used;
  if (child->good_stdin)
    good_stdin_used = 1;

  child->deleted = 0;

  /* Set up the environment for the child.  */
  if (child->environment == 0)
    child->environment = target_environment (child->file);

  if (start_remote_job_p ())
    {
      int is_remote, id, used_stdin;
      if (start_remote_job (argv, child->good_stdin ? 0 : bad_stdin,
			    &is_remote, &id, &used_stdin))
	goto error;
      else
	{
	  if (child->good_stdin && !used_stdin)
	    {
	      child->good_stdin = 0;
	      good_stdin_used = 0;
	    }
	  child->remote = is_remote;
	  child->pid = id;
	}
    }
  else
    {
      /* Wait for the load to be low enough.  */

      wait_to_start_job ();

      /* Fork the child process.  */
      
      child->remote = 0;
      child->pid = vfork ();
      if (child->pid == 0)
	/* We are the child side.  */
	child_execute_job (child->good_stdin ? 0 : bad_stdin, 1,
			   child->file, argv, child->environment);
      else if (child->pid < 0)
	{
	  /* Fork failed!  */
	  perror_with_name (VFORK_NAME, "");
	  goto error;
	}
    }

  /* We are the parent side.  Set the state to
     say the commands are running and return.  */

  child->file->command_state = cs_running;
  return;

 error:;
  child->file->update_status = 1;
  child->file->command_state = cs_finished;
}


/* Create a `struct child' for FILE and start its commands running.  */

void
new_job (file)
     register struct file *file;
{
  extern unsigned int files_remade;
  register struct commands *cmds = file->cmds;
  register struct child *c;

  if (cmds->command_lines == 0)
    {
      /* Chop CMDS->commands up into lines in CMDS->command_lines.
	 Also set the corresponding CMDS->lines_recurse elements,
	 and the CMDS->any_recurse flag.  */
      register char *p;
      unsigned int nlines, idx;
      char **lines;

      nlines = 5;
      lines = (char **) xmalloc (5 * sizeof (char *));
      idx = 0;
      p = cmds->commands;
      while (*p != '\0')
	{
	  char *end = p;
	find_end:;
	  end = index (end, '\n');
	  if (end == 0)
	    end = p + strlen (p);
	  else if (end > p && end[-1] == '\\')
	    {
	      int backslash = 1;
	      register char *b;
	      for (b = end - 2; b >= p && *b == '\\'; --b)
		backslash = !backslash;
	      if (backslash)
		{
		  ++end;
		  goto find_end;
		}
	    }

	  if (idx == nlines - 1)
	    {
	      nlines += 2;
	      lines = (char **) xrealloc ((char *) lines,
					  nlines * sizeof (char *));
	    }
	  lines[idx++] = savestring (p, end - p);
	  p = end;
	  if (*p != '\0')
	    ++p;
	}
      lines[idx++] = 0;

      if (idx != nlines)
	{
	  nlines = idx;
	  lines = (char **) xrealloc ((char *) lines,
				      nlines * sizeof (char *));
	}

      cmds->command_lines = lines;

      cmds->any_recurse = 0;
      --nlines;
      cmds->lines_recurse = (char *) xmalloc (nlines);
      for (idx = 0; idx < nlines; ++idx)
	{
	  unsigned int len;
	  int recursive;
	  p = lines[idx];
	  len = strlen (p);
	  recursive = (sindex (p, len, "$(MAKE)", 7) != 0
		       || sindex (p, len, "${MAKE}", 7) != 0);
	  cmds->lines_recurse[idx] = recursive;
	  cmds->any_recurse |= recursive;
	}
    }

  if (job_slots > 0)
    /* Wait for a job slot to be freed up.  */
    while (job_slots_used == job_slots)
      wait_for_children (1, 0);

  /* Start the command sequence, record it in a new
     `struct child', and add that to the chain.  */

  block_children ();

  c = (struct child *) xmalloc (sizeof (struct child));
  c->file = file;
  c->command_line = 0;
  c->command_ptr = c->commands = 0;
  c->environment = 0;
  start_job (c);
  switch (file->command_state)
    {
    case cs_running:
      c->next = children;
      children = c;
      /* One more job slot is in use.  */
      ++job_slots_used;
      break;

    case cs_finished:
      free_child (c);
      break;

    default:
      error ("internal error: `%s' command_state == %d in new_job",
	     file->name, (int) file->command_state);
      abort ();
      break;
    }

  unblock_children ();

  ++files_remade;

  if (job_slots == 1 && file->command_state == cs_running)
    {
      /* Since there is only one job slot, make things run linearly.
	 Wait for the child to finish, setting the state to `cs_finished'.  */
      while (file->command_state != cs_finished)
	wait_for_children (1, 0);
    }
}

/* Replace the current process with one executing the command in ARGV.
   STDIN_FD and STDOUT_FD are used as the process's stdin and stdout;
   FILE, if not nil, is used as the variable-set to expand `PATH' and `SHELL';
   ENVP is the environment of the new program.  This function won't return.  */

void
child_execute_job (stdin_fd, stdout_fd, file, argv, envp)
     int stdin_fd, stdout_fd;
     struct file *file;
     char **argv, **envp;
{
  char *path;

  if (stdin_fd != 0)
    (void) dup2 (stdin_fd, 0);
  if (stdout_fd != 1)
    (void) dup2 (stdout_fd, 1);

  /* Free up file descriptors.  */
  {
    register int d;
    int max = getdtablesize ();
    for (d = 3; d < max; ++d)
      (void) close (d);
  }

  /* Don't block children for our child.  */
  unblock_children ();

  path = allocated_variable_expand_for_file ("$(PATH)", file);

  /* Run the command.  */
  exec_command (argv, envp, path);

  /* If exec_command returned, then we should use the shell.  */
  {
    int argc;
    char **shell_argv;

    argc = 0;
    while (argv[argc] != 0)
      ++argc;
    shell_argv = (char **) alloca ((2 + argc + 1) * sizeof (char *));
    shell_argv[0] = variable_expand_for_file ("$(SHELL)", file);
#ifdef HELIOS
    shell_argv[1] = "-fec";
#else
    shell_argv[1] = "-c";
#endif
    do
      shell_argv[2 + argc] = argv[argc];
    while (argc-- > 0);
    exec_command (shell_argv, envp, path);

    /* If that returned, die.  */
    _exit (127);
  }
}

/* Replace the current process with one running the command
   in ARGV, with environment ENVP.  The program named in ARGV[0]
   is searched for in PATH.  This function does not return.  */

void
exec_command (argv, envp, path)
     char **argv, **envp;
     char *path;
{
  char *program;

  if (*path == '\0' || index (argv[0], '/') != 0)
    program = argv[0];
  else
    {
      unsigned int len;

#ifndef	USG
      extern int getgroups ();
      static int groups[NGROUPS];
      static int ngroups = -1;
      if (ngroups == -1)
	ngroups = getgroups (NGROUPS, groups);
#endif	/* Not USG.  */

      len = strlen (argv[0]) + 1;
      program = (char *) alloca (strlen (path) + 1 + len);
      do
	{
	  struct stat st;
	  int perm;
	  char *p;

	  p = index (path, ':');
	  if (p == 0)
	    p = path + strlen (path);

	  if (p == path)
	    bcopy (argv[0], program, len);
	  else
	    {
	      bcopy (path, program, p - path);
	      program[p - path] = '/';
	      bcopy (argv[0], program + (p - path) + 1, len);
	    }

	  if (stat (program, &st) == 0
	      && (st.st_mode & S_IFMT) == S_IFREG)
	    {
	      if (st.st_uid == geteuid ())
		perm = (st.st_mode & 0100);
	      else if (st.st_gid == getegid ())
		perm = (st.st_mode & 0010);
	      else
		{
#ifndef	USG
		  register int i;
		  for (i = 0; i < ngroups; ++i)
		    if (groups[i] == st.st_gid)
		      break;
		  if (i < ngroups)
		    perm = (st.st_mode & 0010);
		  else
#endif	/* Not USG.  */
		    perm = (st.st_mode & 0001);
		}

	      if (perm != 0)
		goto run;
	    }

	  path = p + 1;
	} while (*path != '\0');

      error ("%s: Command not found", argv[0]);
      _exit (127);
    }

 run:;
  /* Make might be installed set-gid kmem so that the load average
     code works, so we want to make sure we use the real gid.  */
  (void) setgid (getgid ());

  execve (program, argv, envp);

  if (errno != ENOEXEC)
    {
      perror_with_name ("execve: ", program);
      _exit (127);
    }
}

/* Figure out the argument list necessary to run LINE as a command.
   Try to avoid using a shell.  This routine handles only ' quoting.
   Starting quotes may be escaped with a backslash.  If any of the
   characters in sh_chars[] is seen, or any of the builtin commands
   listed in sh_cmds[] is the first word of a line, the shell is used.

   FILE is the target whose commands these are.  It is used for
   variable expansion for $(SHELL) and $(IFS).  */

char **
construct_command_argv (line, file)
     char *line;
     struct file *file;
{
#ifndef HELIOS
  static char sh_chars[] = "#;\"*?[]&|<>(){}=$`";
  static char *sh_cmds[] = { "cd", "eval", "exec", "exit", "login",
			     "logout", "set", "umask", "wait", "while", "for",
			     "case", "if", ":", ".", "break", "continue",
			     "export", "read", "readonly", "shift", "times",
			     "trap", "switch", 0 };
#else
  static char sh_chars[] = "#;\"*?[]&|<>(){}=$`^'";
  static char *sh_cmds[] = {	  "alias",  "alloc",  "bg",  "break",  "breaksw",
				  "case",  "cd",  "chdir",  "continue",  "default",
				  "dirs",  "echo",  "else",  "end",  "endif",
				  "endsw",  "eval",  "exec",  "exit",  "fault",
				  "fg",  "foreach",  "glob",  "goto",  "hashstat",
				  "history",  "if",  "jobs",  "kill",  "limit",
				  "login",  "logout",  "nice",  "nohup",  "notify",
				  "onintr",  "popd",  "printenv",  "pushd",  "pwd",
				  "rehash",  "repeat",  "set",  "setenv",  "shift",
				  "source",  "stop",  "suspend",  "switch",  "time",
				  "umask",  "unalias",  "unhash",  "unlimit",  "unset",
				  "unsetenv",  "version",  "wait",  "while",  "@",
				   0
				};

#endif
  register int i;
  register char *p;
  register char *ap;
  char *end;
  int instring;
  char **new_argv = 0;

  /* See if it is safe to parse commands internally.  */
  p = variable_expand_for_file ("$(SHELL)", file);
#ifdef HELIOS
  if (strcmp (p, "/helios/bin/shell"))
#else
  if (strcmp (p, "/bin/sh"))
#endif
    goto slow;
  p = variable_expand_for_file ("$(IFS)", file);
  for (ap = p; *ap != '\0'; ++ap)
    if (*ap != ' ' && *ap != '\t' && *ap != '\n')
      goto slow;

  i = strlen (line) + 1;

  /* More than 1 arg per character is impossible.  */
  new_argv = (char **) xmalloc (i * sizeof (char *));

  /* All the args can fit in a buffer as big as LINE is.   */
  ap = new_argv[0] = (char *) xmalloc (i);
  end = ap + i;

  /* I is how many complete arguments have been found.  */
  i = 0;
  instring = 0;
  for (p = line; *p != '\0'; ++p)
    {
      if (ap > end)
	abort ();

      if (instring)
	{
	  /* Inside a string, just copy any char except a closing quote.  */
	  if (*p == '\'')
	    instring = 0;
	  else
	    *ap++ = *p;
	}
      else if (index (sh_chars, *p) != 0)
	/* Not inside a string, but it's a special char.  */
	goto slow;
      else
	/* Not a special char.  */
	switch (*p)
	  {
	  case '\\':
	    if (p[1] != '\0' && p[1] != '\n')
	      /* Copy and skip the following char.  */
	      *ap++ = *++p;
	    break;

	  case '\'':
	    instring = 1;
	    break;

	  case '\n':
	  case ' ':
	  case '\t':
	    /* We have the end of an argument.
	       Terminate the text of the argument.  */
	    *ap++ = '\0';
	    new_argv[++i] = ap;
	    /* If this argument is the command name,
	       see if it is a built-in shell command.
	       If so, have the shell handle it.  */
	    if (i == 1)
	      {
		register int j;
		for (j = 0; sh_cmds[j] != 0; ++j)
		  if (streq (sh_cmds[j], new_argv[0]))
		    goto slow;
	      }

	    /* Ignore multiple whitespace chars.  */
	    p = next_token (p);
	    /* Next iteration should examine the first nonwhite char.  */
	    --p;
	    break;

	  default:
	    *ap++ = *p;
	    break;
	  }
    }

  /* Terminate the last argument and the argument list.  */

  *ap = '\0';
  if (new_argv[i][0] != '\0')
    ++i;
  new_argv[i] = 0;

  if (new_argv[0] == 0)
    /* Line was empty.  */
    return 0;
  else
    return new_argv;

 slow:;
  if (new_argv != 0)
    free (new_argv);
  new_argv = (char **) xmalloc (4 * sizeof (char *));
  new_argv[0] = variable_expand_for_file ("$(SHELL)", file);
#ifdef HELIOS
  new_argv[1] = "-fec";
#else
  new_argv[1] = "-c";
#endif
  new_argv[2] = line;
  new_argv[3] = 0;

  return new_argv;
}

#if	defined(USG) && !defined(HAVE_SIGLIST)
/* Initialize sys_siglist.  */

void
init_siglist ()
{
  char buf[100];
  register unsigned int i;

  for (i = 0; i < NSIG; ++i)
    switch (i)
      {
      default:
	sprintf (buf, "Signal %u", i);
	sys_siglist[i] = savestring (buf, strlen (buf));
	break;
      case SIGHUP:
	sys_siglist[i] = "Hangup";
	break;
      case SIGINT:
	sys_siglist[i] = "Interrupt";
	break;
      case SIGQUIT:
	sys_siglist[i] = "Quit";
	break;
      case SIGILL:
	sys_siglist[i] = "Illegal Instruction";
	break;
      case SIGTRAP:
	sys_siglist[i] = "Trace Trap";
	break;
      case SIGIOT:
	sys_siglist[i] = "IOT Trap";
	break;
#ifdef	SIGEMT
      case SIGEMT:
	sys_siglist[i] = "EMT Trap";
	break;
#endif
#ifdef	SIGDANGER
      case SIGDANGER:
	sys_siglist[i] = "Danger signal";
	break;
#endif
      case SIGFPE:
	sys_siglist[i] = "Floating Point Exception";
	break;
      case SIGKILL:
	sys_siglist[i] = "Killed";
	break;
      case SIGBUS:
	sys_siglist[i] = "Bus Error";
	break;
      case SIGSEGV:
	sys_siglist[i] = "Segmentation fault";
	break;
#ifdef SIGSYS
      case SIGSYS:
	sys_siglist[i] = "Bad Argument to System Call";
	break;
#endif
      case SIGPIPE:
	sys_siglist[i] = "Broken Pipe";
	break;
      case SIGALRM:
	sys_siglist[i] = "Alarm Clock";
	break;
      case SIGTERM:
	sys_siglist[i] = "Terminated";
	break;
      case SIGUSR1:
	sys_siglist[i] = "User-defined signal 1";
	break;
      case SIGUSR2:
	sys_siglist[i] = "User-defined signal 2";
	break;
#ifdef	SIGCLD
      case SIGCLD:
#endif
#if	defined(SIGCHLD) && !defined(SIGCLD)
      case SIGCHLD:
#endif
	sys_siglist[i] = "Child Process Exited";
	break;
#ifdef	SIGPWR
      case SIGPWR:
	sys_siglist[i] = "Power Failure";
	break;
#endif
#ifdef	SIGVTALRM
      case SIGVTALRM:
	sys_siglist[i] = "Virtual Timer Alarm";
	break;
#endif
#ifdef	SIGPROF
      case SIGPROF:
	sys_siglist[i] = "Profiling Alarm Clock";
	break;
#endif
#ifdef	SIGIO
      case SIGIO:
	sys_siglist[i] = "I/O Possible";
	break;
#endif
#ifdef	SIGWINDOW
      case SIGWINDOW:
	sys_siglist[i] = "Window System Signal";
	break;
#endif
#ifdef	SIGSTOP
      case SIGSTOP:
	sys_siglist[i] = "Stopped (signal)";
	break;
#endif
#ifdef	SIGTSTP
      case SIGTSTP:
	sys_siglist[i] = "Stopped";
	break;
#endif
#ifdef	SIGCONT
      case SIGCONT:
	sys_siglist[i] = "Continued";
	break;
#endif
#ifdef	SIGTTIN
      case SIGTTIN:
	sys_siglist[i] = "Stopped (tty input)";
	break;
#endif
#ifdef	SIGTTOU
      case SIGTTOU:
	sys_siglist[i] = "Stopped (tty output)";
	break;
#endif
#ifdef	SIGURG
      case SIGURG:
	sys_siglist[i] = "Urgent Condition on Socket";
	break;
#endif
#ifdef	SIGXCPU
      case SIGXCPU:
	sys_siglist[i] = "CPU Limit Exceeded";
	break;
#endif
#ifdef	SIGXFSZ
      case SIGXFSZ:
	sys_siglist[i] = "File Size Limit Exceeded";
	break;
#endif
      }
}
#endif	/* USG and not HAVE_SIGLIST.  */

#if	defined(USG) && !defined(USGr3) && !defined(HAVE_DUP2)
int
dup2 (old, new)
     int old, new;
{
  int fd;

  (void) close (new);
  fd = dup (old);
  if (fd != new)
    {
      (void) close (fd);
      errno = EMFILE;
      return -1;
    }

  return fd;
}
#endif	/* USG and not USGr3 and not HAVE_DUP2.  */
