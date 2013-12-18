/* $Header: /usr/perihelion/Helios/cmds/gnu/gmake/make-3.57/RCS/job.h,v 1.1 90/08/28 14:37:37 james Exp $ */

/* Structure describing a running or dead child process.  */

struct child
  {
    struct child *next;		/* Link in the chain.  */

    struct file *file;		/* File being remade.  */

    char **environment;		/* Environment for commands.  */

    char *commands;		/* Commands being executed.  */
    char *command_ptr;		/* Pointer into above.  */
    unsigned int command_line;	/* Index into file->cmds->command_lines.  */

    int pid;			/* Child process's ID number.  */
    unsigned int remote:1;	/* Nonzero if executing remotely.  */

    unsigned int noerror:1;	/* Nonzero if commands contained a `-'.  */

    unsigned int good_stdin:1;	/* Nonzero if this child has a good stdin.  */
    unsigned int deleted:1;	/* Nonzero if targets have been deleted.  */
  };

extern struct child *children;

extern void new_job ();
extern void wait_for_children ();
extern void block_children (), unblock_children ();

extern char **construct_command_argv ();
extern void child_execute_job ();
extern void exec_command ();

extern unsigned int job_slots_used;
