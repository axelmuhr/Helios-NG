
extern	struct	_iobuf {
	int	_cnt;
	unsigned char *_ptr;
	unsigned char *_base;
	int	_bufsiz;
	short	_flag;
	char	_file;		 
} _iob[];

extern struct _iobuf 	*fopen();
extern struct _iobuf 	*fdopen();
extern struct _iobuf 	*freopen();
extern struct _iobuf 	*popen();
extern struct _iobuf 	*tmpfile();
extern long	ftell();
extern char	*fgets();
extern char	*gets();

extern char	*ctermid();
extern char	*cuserid();
extern char	*tempnam();
extern char	*tmpnam();

typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
typedef	unsigned short	ushort;		 
typedef	unsigned int	uint;		 

typedef	struct	_physadr { short r[1]; } *physadr;
typedef	struct	label_t	{
	int	val[13];
} label_t;

typedef	struct	_quad { long val[2]; } quad;
typedef	long	daddr_t;
typedef	char *	caddr_t;
typedef	u_long	ino_t;
typedef	long	swblk_t;
typedef	int	size_t;
typedef	long	time_t;
typedef	short	dev_t;
typedef	long	off_t;
typedef	u_short	uid_t;
typedef	u_short	gid_t;
typedef	long	key_t;

typedef	long	fd_mask;

typedef	struct fd_set {
	fd_mask	fds_bits[(((256 )+(( (sizeof(fd_mask) * 8		)	)-1))/( (sizeof(fd_mask) * 8		)	)) ];
} fd_set;

typedef	char *	addr_t;

typedef	int	faultcode_t;	 

void	(*signal())();

void  (*sigset())();
int   sighold();
int   sigrelse();
int   sigignore();

struct	sigvec {
	void	(*sv_handler)();	 
	int	sv_mask;		 
	int	sv_flags;		 
};

struct	sigstack {
	char	*ss_sp;			 
	int	ss_onstack;		 
};

struct	sigcontext {
	int	sc_onstack;		 
	int	sc_mask;		 

	int	sc_sp;			 
	int	sc_pc;			 
	int	sc_ps;			 

};

struct flock {
        short   l_type;		 
        short   l_whence;	 
        long    l_start;	 
        long    l_len;           
        short   l_pid;		 
        short   l_xxx;		 
};

typedef char *va_list;

extern int target_flags;

enum reg_class { NO_REGS, LO_FPA_REGS, FPA_REGS, FP_REGS,
  FP_OR_FPA_REGS, DATA_REGS, DATA_OR_FPA_REGS, DATA_OR_FP_REGS,
  DATA_OR_FP_OR_FPA_REGS, ADDR_REGS, GENERAL_REGS,
  GENERAL_OR_FPA_REGS, GENERAL_OR_FP_REGS, ALL_REGS,
  LIM_REG_CLASSES };

extern enum reg_class regno_reg_class[];

struct _obstack_chunk		 
{
  char  *limit;			 
  struct _obstack_chunk *prev;	 
  char	contents[4];		 
};

struct obstack		 
{
  long	chunk_size;		 
  struct _obstack_chunk* chunk;	 
  char	*object_base;		 
  char	*next_free;		 
  char	*chunk_limit;		 
  int	temp;			 
  int   alignment_mask;		 
  struct _obstack_chunk *(*chunkfun) ();  
  void (*freefun) ();		 
};

void obstack_init (struct obstack *obstack);

void * obstack_alloc (struct obstack *obstack, int size);

void * obstack_copy (struct obstack *obstack, void *address, int size);
void * obstack_copy0 (struct obstack *obstack, void *address, int size);

void obstack_free (struct obstack *obstack, void *block);

void obstack_blank (struct obstack *obstack, int size);

void obstack_grow (struct obstack *obstack, void *data, int size);
void obstack_grow0 (struct obstack *obstack, void *data, int size);

void obstack_1grow (struct obstack *obstack, int data_char);

void * obstack_finish (struct obstack *obstack);

int obstack_object_size (struct obstack *obstack);

int obstack_room (struct obstack *obstack);
void obstack_1grow_fast (struct obstack *obstack, int data_char);
void obstack_blank_fast (struct obstack *obstack, int size);

void * obstack_base (struct obstack *obstack);
void * obstack_next_free (struct obstack *obstack);
int obstack_alignment_mask (struct obstack *obstack);
int obstack_chunk_size (struct obstack *obstack);

extern int xmalloc ();
extern void free ();

struct obstack obstack;

char *handle_braces ();
char *save_string ();
char *concat ();
int do_spec ();
int do_spec_1 ();
char *find_file ();
static char *find_exec_file ();
void validate_switches ();
void validate_all_switches ();

struct compiler
{
  char *suffix;			 

  char *spec;			 

};

struct compiler compilers[] =
{
  {".c",
   "cpp %{nostdinc} %{C} %{v} %{D*} %{U*} %{I*} %{M*} %{trigraphs} -undef         -D__GNUC__ %{ansi:-trigraphs -$ -D__STRICT_ANSI__} %{!ansi:%p} %P        %c %{O:-D__OPTIMIZE__} %{traditional} %{pedantic}	%{Wcomment*} %{Wtrigraphs} %{Wall} %C        %i %{!M*:%{!E:%{!pipe:%g.cpp}}}%{E:%{o*}}%{M*:%{o*}} |\n    %{!M*:%{!E:cc1 %{!pipe:%g.cpp} %1 		   %{!Q:-quiet} -dumpbase %i %{Y*} %{d*} %{m*} %{f*} %{a}		   %{g} %{O} %{W*} %{w} %{pedantic} %{ansi} %{traditional}		   %{v:-version} %{gg:-symout %g.sym} %{pg:-p} %{p}		   %{pg:%{fomit-frame-pointer:%e-pg and -fomit-frame-pointer are incompatible}}		   %{S:%{o*}%{!o*:-o %b.s}}%{!S:-o %{|!pipe:%g.s}} |\n              %{!S:as %{R} %{j} %{J} %{h} %{d2} %a %{gg:-G %g.sym}		      %{c:%{o*}%{!o*:-o %w%b.o}}%{!c:-o %d%w%b.o}                      %{!pipe:%g.s}\n }}}"},
  {".cc",
   "cpp -+ %{nostdinc} %{C} %{v} %{D*} %{U*} %{I*} %{M*}         -undef -D__GNUC__ %p %P        %c %{O:-D__OPTIMIZE__} %{traditional} %{pedantic}	%{Wcomment*} %{Wtrigraphs} %{Wall} %C        %i %{!M*:%{!E:%{!pipe:%g.cpp}}}%{E:%{o*}}%{M*:%{o*}} |\n    %{!M*:%{!E:cc1plus %{!pipe:%g.cpp} %1		   %{!Q:-quiet} -dumpbase %i %{Y*} %{d*} %{m*} %{f*} %{a}		   %{g} %{O} %{W*} %{w} %{pedantic} %{traditional}		   %{v:-version} %{gg:-symout %g.sym} %{pg:-p} %{p}		   %{pg:%{fomit-frame-pointer:%e-pg and -fomit-frame-pointer are incompatible}}		   %{S:%{o*}%{!o*:-o %b.s}}%{!S:-o %{|!pipe:%g.s}} |\n              %{!S:as %{R} %{j} %{J} %{h} %{d2} %a %{gg:-G %g.sym}		      %{c:%{o*}%{!o*:-o %w%b.o}}%{!c:-o %d%w%b.o}                      %{!pipe:%g.s}\n }}}"},
  {".i",
   "cc1 %i %1 %{!Q:-quiet} %{Y*} %{d*} %{m*} %{f*} %{a}	%{g} %{O} %{W*} %{w} %{pedantic} %{ansi} %{traditional}	%{v:-version} %{gg:-symout %g.sym} %{pg:-p} %{p}	%{S:%{o*}%{!o*:-o %b.s}}%{!S:-o %{|!pipe:%g.s}} |\n    %{!S:as %{R} %{j} %{J} %{h} %{d2} %a %{gg:-G %g.sym}            %{c:%{o*}%{!o*:-o %w%b.o}}%{!c:-o %d%w%b.o} %{!pipe:%g.s}\n }"},

  {".s",
   "%{!S:as %{R} %{j} %{J} %{h} %{d2} %a             %{c:%{o*}%{!o*:-o %w%b.o}}%{!c:-o %d%w%b.o} %i\n }"},

  {".S",
   "cpp %{nostdinc} %{C} %{v} %{D*} %{U*} %{I*} %{M*} %{trigraphs}         -undef -D__GNUC__ -$ %p %P        %c %{O:-D__OPTIMIZE__} %{traditional} %{pedantic}	%{Wcomment*} %{Wtrigraphs} %{Wall} %C        %i %{!M*:%{!E:%{!pipe:%g.cpp}}}%{E:%{o*}}%{M*:%{o*}} |\n    %{!M*:%{!E:%{!S:as %{R} %{j} %{J} %{h} %{d2} %a                     %{c:%{o*}%{!o*:-o %w%b.o}}%{!c:-o %d%w%b.o}		    %{!pipe:%g.s}\n }}}"},

  {0, 0}
};

char *link_spec = "%{!c:%{!M*:%{!E:%{!S:ld %{o*} %l %{A} %{d} %{e*} %{N} %{n} %{r} %{s} %{S} %{T*} %{t} %{u*} %{X} %{x} %{z} %{y*} %{!nostdlib:%S}  %{L*} %o %{!nostdlib:gnulib%s %{g:-lg} %L}\n }}}}";

char *temp_filename;

int temp_filename_length;

struct temp_file
{
  char *name;
  struct temp_file *next;
};

struct temp_file *always_delete_queue;
struct temp_file *failure_delete_queue;

void
record_temp_file (filename, always_delete, fail_delete)
     char *filename;
     int always_delete;
     int fail_delete;
{
  register char *name;
  name = (char *) xmalloc (strlen (filename) + 1);
  strcpy (name, filename);

  if (always_delete)
    {
      register struct temp_file *temp;
      temp = (struct temp_file *) xmalloc (sizeof (struct temp_file));
      temp->next = always_delete_queue;
      temp->name = name;
      always_delete_queue = temp;
    }

  if (fail_delete)
    {
      register struct temp_file *temp;
      temp = (struct temp_file *) xmalloc (sizeof (struct temp_file));
      temp->next = failure_delete_queue;
      temp->name = name;
      failure_delete_queue = temp;
    }
}

void
delete_temp_files (success)
     int success;
{
  register struct temp_file *temp;

  for (temp = always_delete_queue; temp; temp = temp->next)
    {

	unlink (temp->name);
    }

  if (! success)
    for (temp = failure_delete_queue; temp; temp = temp->next)
      {

	  unlink (temp->name);
      }

  always_delete_queue = 0;
  failure_delete_queue = 0;
}

void
clear_failure_queue ()
{
  failure_delete_queue = 0;
}

void
choose_temp_base ()
{
  register char *foo = "/tmp/ccXXXXXX";
  temp_filename = (char *) xmalloc (strlen (foo) + 1);
  strcpy (temp_filename, foo);
  mktemp (temp_filename);
  temp_filename_length = strlen (temp_filename);
}

char **argbuf;

int argbuf_length;

int argbuf_index;

unsigned char vflag;

char *programname;

char *user_exec_prefix = 0;

char *env_exec_prefix = 0;

char *standard_exec_prefix = "/usr/local/lib/gcc-" ;
char *standard_exec_prefix_1 = "/usr/lib/gcc-";

char *standard_startfile_prefix = "/usr/local/lib/" ;
char *standard_startfile_prefix_1 = "/lib/";
char *standard_startfile_prefix_2 = "/usr/lib/";

void
clear_args ()
{
  argbuf_index = 0;
}

void
store_arg (arg, delete_always, delete_failure)
     char *arg;
     int delete_always, delete_failure;
{
  if (argbuf_index + 1 == argbuf_length)
    {
      argbuf = (char **) realloc (argbuf, (argbuf_length *= 2) * sizeof (char *));
    }

  argbuf[argbuf_index++] = arg;
  argbuf[argbuf_index] = 0;

  if (delete_always || delete_failure)
    record_temp_file (arg, delete_always, delete_failure);
}

static char *
find_exec_file (prog)
     char *prog;
{
  int win = 0;
  char *temp;
  int size;

  size = strlen (standard_exec_prefix);
  if (user_exec_prefix != 0 && strlen (user_exec_prefix) > size)
    size = strlen (user_exec_prefix);
  if (env_exec_prefix != 0 && strlen (env_exec_prefix) > size)
    size = strlen (env_exec_prefix);
  if (strlen (standard_exec_prefix_1) > size)
    size = strlen (standard_exec_prefix_1);
  size += strlen (prog) + 1;
  temp = (char *) xmalloc (size);

  if (user_exec_prefix)
    {
      strcpy (temp, user_exec_prefix);
      strcat (temp, prog);
      win = (access (temp, 1 ) == 0);
    }

  if (!win && env_exec_prefix)
    {
      strcpy (temp, env_exec_prefix);
      strcat (temp, prog);
      win = (access (temp, 1 ) == 0);
    }

  if (!win)
    {
      strcpy (temp, standard_exec_prefix);
      strcat (temp, prog);
      win = (access (temp, 1 ) == 0);
    }

  if (!win)
    {
      strcpy (temp, standard_exec_prefix_1);
      strcat (temp, argbuf[0]);
      win = (access (temp, 1 ) == 0);
    }

  if (win)
    return temp;
  else
    return 0;
}

int last_pipe_input;

static int
pexecute (func, program, argv, not_last)
     char *program;
     int (*func)();
     char *argv[];
     int not_last;
{
  int pid;
  int pdes[2];
  int input_desc = last_pipe_input;
  int output_desc = 1 ;

  if (not_last)
    {
      if (pipe (pdes) < 0)
	pfatal_with_name ("pipe");
      output_desc = pdes[1 ];
      last_pipe_input = pdes[0 ];
    }
  else
    last_pipe_input = 0 ;

  pid = fork  ();

  switch (pid)
    {
    case -1:
      pfatal_with_name ("vfork");
      break;

    case 0:  
      if (input_desc != 0 )
	{
	  close (0 );
	  dup (input_desc);
	  close (input_desc);
	}
      if (output_desc != 1 )
	{
	  close (1 );
	  dup (output_desc);
	  close (output_desc);
	}

      if (last_pipe_input != 0 )
	close (last_pipe_input);

      (*func) (program, argv);
      perror_exec (program);
      exit (-1);

    default:

      if (input_desc != 0 )
	close (input_desc);
      if (output_desc != 1 )
	close (output_desc);

      return pid;
    }
}

int
execute ()
{
  int i, j;
  int n_commands;		 
  char *string;
  struct command
    {
      char *prog;		 
      char **argv;		 
      int pid;			 
    };

  struct command *commands;	 

  for (n_commands = 1, i = 0; i < argbuf_index; i++)
    if (strcmp (argbuf[i], "|") == 0)
      n_commands++;

  commands
    = (struct command *) __builtin_alloca  (n_commands * sizeof (struct command));

  commands[0].prog = argbuf[0];  
  commands[0].argv = &argbuf[0];
  string = find_exec_file (commands[0].prog);
  if (string)
    commands[0].argv[0] = string;

  for (n_commands = 1, i = 0; i < argbuf_index; i++)
    if (strcmp (argbuf[i], "|") == 0)
      {				 
	argbuf[i] = 0;	 
	commands[n_commands].prog = argbuf[i + 1];
	commands[n_commands].argv = &argbuf[i + 1];
	string = find_exec_file (commands[n_commands].prog);
	if (string)
	  commands[n_commands].argv[0] = string;
	n_commands++;
      }

  argbuf[argbuf_index] = 0;

  if (vflag)
    {
      for (i = 0; i < n_commands ; i++)
	{
	  char **j;

	  for (j = commands[i].argv; *j; j++)
	    fprintf ((&_iob[2]) , " %s", *j);

	  if (i + 1 != n_commands)
	    fprintf ((&_iob[2]) , " |");
	  fprintf ((&_iob[2]) , "\n");
	}
      fflush ((&_iob[2]) );

    }

  last_pipe_input = 0 ;
  for (i = 0; i < n_commands; i++)
    {
      extern int execv(), execvp();
      char *string = commands[i].argv[0];

      commands[i].pid = pexecute ((string != commands[i].prog ? execv : execvp),
				  string, commands[i].argv,
				  i + 1 < n_commands);

      if (string != commands[i].prog)
	free (string);
    }

  {
    int ret_code = 0;

    for (i = 0; i < n_commands; i++)
      {
	int status;
	int pid;
	char *prog;

	pid = wait (&status);
	if (pid < 0)
	  abort ();

	if (status != 0)
	  {
	    int j;
	    for (j = 0; j < n_commands; j++)
	      if (commands[j].pid == pid)
		prog = commands[j].prog;

	    if ((status & 0x7F) != 0)
	      fatal ("Program %s got fatal signal %d.", prog, (status & 0x7F));
	    if (((status & 0xFF00) >> 8) >= 1 )
	      ret_code = -1;
	  }
      }
    return ret_code;
  }
}

struct switchstr
{
  char *part1;
  char *part2;
  int valid;
};

struct switchstr *switches;

int n_switches;

char **infiles;

int n_infiles;

char **outfiles;

void
process_command (argc, argv)
     int argc;
     char **argv;
{
  extern char *getenv ();
  register int i;
  n_switches = 0;
  n_infiles = 0;

  env_exec_prefix = getenv ("GCC_EXEC_PREFIX");

  for (i = 1; i < argc; i++)
    {
      if (argv[i][0] == '-' && argv[i][1] != 'l')
	{
	  register char *p = &argv[i][1];
	  register int c = *p;

	  switch (c)
	    {
	    case 'B':
	      user_exec_prefix = p + 1;
	      break;

	    case 'v':	 
	      vflag++;
	      n_switches++;
	      break;

	    default:
	      n_switches++;

	      if (((c) == 'D' || (c) == 'U' || (c) == 'o' || (c) == 'e' || (c) == 'T' || (c) == 'u' || (c) == 'I' || (c) == 'Y' || (c) == 'm' || (c) == 'L')  && p[1] == 0)
		i++;
	      else if ((!strcmp (p, "Tdata")) )
		i++;
	    }
	}
      else
	n_infiles++;
    }

  switches = ((struct switchstr *)
	      xmalloc ((n_switches + 1) * sizeof (struct switchstr)));
  infiles = (char **) xmalloc ((n_infiles + 1) * sizeof (char *));
  n_switches = 0;
  n_infiles = 0;

  for (i = 1; i < argc; i++)
    {
      if (argv[i][0] == '-' && argv[i][1] != 'l')
	{
	  register char *p = &argv[i][1];
	  register int c = *p;

	  if (c == 'B')
	    continue;
	  switches[n_switches].part1 = p;
	  if ((((c) == 'D' || (c) == 'U' || (c) == 'o' || (c) == 'e' || (c) == 'T' || (c) == 'u' || (c) == 'I' || (c) == 'Y' || (c) == 'm' || (c) == 'L')  && p[1] == 0)
	      || (!strcmp (p, "Tdata")) )
	    switches[n_switches].part2 = argv[++i];
	  else
	    switches[n_switches].part2 = 0;
	  switches[n_switches].valid = 0;
	  n_switches++;
	}
      else
	infiles[n_infiles++] = argv[i];
    }

  switches[n_switches].part1 = 0;
  infiles[n_infiles] = 0;
}

char *input_filename;
int input_file_number;
int input_filename_length;
int basename_length;
char *input_basename;

int arg_going;

int delete_this_arg;

int this_is_output_file;

int this_is_library_file;

int
do_spec (spec)
     char *spec;
{
  int value;

  clear_args ();
  arg_going = 0;
  delete_this_arg = 0;
  this_is_output_file = 0;
  this_is_library_file = 0;

  value = do_spec_1 (spec, 0);

  if (value == 0)
    {
      if (argbuf_index > 0 && !strcmp (argbuf[argbuf_index - 1], "|"))
	argbuf_index--;

      if (argbuf_index > 0)
	value = execute ();
    }

  return value;
}

int
do_spec_1 (spec, inswitch)
     char *spec;
     int inswitch;
{
  register char *p = spec;
  register int c;
  char *string;

  while (c = *p++)

    switch (inswitch ? 'a' : c)
      {
      case '\n':

	if (arg_going)
	  {
	    ({ struct obstack *__o = (&obstack);	((__o->next_free + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, 1) : 0),	*(__o->next_free)++ = ( 0);	(void) 0; }) ;
	    string = ({ struct obstack *__o = (&obstack);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ;
	    if (this_is_library_file)
	      string = find_file (string);
	    store_arg (string, delete_this_arg, this_is_output_file);
	    if (this_is_output_file)
	      outfiles[input_file_number] = string;
	  }
	arg_going = 0;

	if (argbuf_index > 0 && !strcmp (argbuf[argbuf_index - 1], "|"))
	  {
	    int i;
	    for (i = 0; i < n_switches; i++)
	      if (!strcmp (switches[i].part1, "pipe"))
		break;

	    if (i < n_switches)
	      {
		switches[i].valid = 1;
		break;
	      }
	    else
	      argbuf_index--;
	  }

	if (argbuf_index > 0)
	  {
	    int value = execute ();
	    if (value)
	      return value;
	  }
	clear_args ();
	arg_going = 0;
	delete_this_arg = 0;
	this_is_output_file = 0;
	this_is_library_file = 0;
	break;

      case '|':
	if (arg_going)
	  {
	    ({ struct obstack *__o = (&obstack);	((__o->next_free + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, 1) : 0),	*(__o->next_free)++ = ( 0);	(void) 0; }) ;
	    string = ({ struct obstack *__o = (&obstack);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ;
	    if (this_is_library_file)
	      string = find_file (string);
	    store_arg (string, delete_this_arg, this_is_output_file);
	    if (this_is_output_file)
	      outfiles[input_file_number] = string;
	  }

	({ struct obstack *__o = (&obstack);	((__o->next_free + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, 1) : 0),	*(__o->next_free)++ = ( c);	(void) 0; }) ;
	arg_going = 1;
	break;

      case '\t':
      case ' ':
	if (arg_going)
	  {
	    ({ struct obstack *__o = (&obstack);	((__o->next_free + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, 1) : 0),	*(__o->next_free)++ = ( 0);	(void) 0; }) ;
	    string = ({ struct obstack *__o = (&obstack);	void *value = (void *) __o->object_base;	__o->next_free	= (((((__o->next_free) - (char *)0) +__o->alignment_mask)	& ~ (__o->alignment_mask)) + (char *)0) ;	((__o->next_free - (char *)__o->chunk	> __o->chunk_limit - (char *)__o->chunk)	? (__o->next_free = __o->chunk_limit) : 0);	__o->object_base = __o->next_free;	value; }) ;
	    if (this_is_library_file)
	      string = find_file (string);
	    store_arg (string, delete_this_arg, this_is_output_file);
	    if (this_is_output_file)
	      outfiles[input_file_number] = string;
	  }
	arg_going = 0;
	delete_this_arg = 0;
	this_is_output_file = 0;
	this_is_library_file = 0;
	break;

      case '%':
	switch (c = *p++)
	  {
	  case 0:
	    fatal ("Invalid specification!  Bug in cc.");

	  case 'b':
	    ({ struct obstack *__o = (&obstack);	int __len = ( basename_length);	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	memcpy ( __o->next_free, input_basename, __len) ;	__o->next_free += __len;	(void) 0; }) ;
	    arg_going = 1;
	    break;

	  case 'd':
	    delete_this_arg = 2;
	    break;

	  case 'e':

	    {
	      char *q = p;
	      char *buf;
	      while (*p != 0 && *p != '\n') p++;
	      buf = (char *) __builtin_alloca  (p - q + 1);
	      strncpy (buf, q, p - q);
	      error ("%s", buf);
	      return -1;
	    }
	    break;

	  case 'g':
	    ({ struct obstack *__o = (&obstack);	int __len = ( temp_filename_length);	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	memcpy ( __o->next_free, temp_filename, __len) ;	__o->next_free += __len;	(void) 0; }) ;
	    delete_this_arg = 1;
	    arg_going = 1;
	    break;

	  case 'i':
	    ({ struct obstack *__o = (&obstack);	int __len = ( input_filename_length);	((__o->next_free + __len > __o->chunk_limit)	? _obstack_newchunk (__o, __len) : 0);	memcpy ( __o->next_free, input_filename, __len) ;	__o->next_free += __len;	(void) 0; }) ;
	    arg_going = 1;
	    break;

	  case 'o':
	    {
	      register int f;
	      for (f = 0; f < n_infiles; f++)
		store_arg (outfiles[f], 0, 0);
	    }
	    break;

	  case 's':
	    this_is_library_file = 1;
	    break;

	  case 'w':
	    this_is_output_file = 1;
	    break;

	  case '{':
	    p = handle_braces (p);
	    if (p == 0)
	      return -1;
	    break;

	  case '%':
	    ({ struct obstack *__o = (&obstack);	((__o->next_free + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, 1) : 0),	*(__o->next_free)++ = ( '%');	(void) 0; }) ;
	    break;

	  case '1':
	    do_spec_1 ("" , 0);
	    break;

	  case 'a':
	    do_spec_1 ("%{m68000:-mc68010}%{mc68000:-mc68010}%{!mc68000:%{!m68000:-mc68020}}" , 0);
	    break;

	  case 'c':
	    do_spec_1 ((1  ? "%{funsigned-char:-D__CHAR_UNSIGNED__}"	: "%{!fsigned-char:-D__CHAR_UNSIGNED__}") , 0);
	    break;

	  case 'C':
	    do_spec_1 ("%{!msoft-float:%{mfpa:-D__HAVE_FPA__ }%{!mfpa:-D__HAVE_68881__ }}%{!ansi:%{m68000:-Dmc68010}%{mc68000:-Dmc68010}%{!mc68000:%{!m68000:-Dmc68020}}}" , 0);
	    break;

	  case 'l':
	    do_spec_1 ("" , 0);
	    break;

	  case 'L':
	    do_spec_1 ("%{!p:%{!pg:-lc}}%{p:-lc_p}%{pg:-lc_p} %{a:/usr/lib/bb_link.o} " , 0);
	    break;

	  case 'p':
	    do_spec_1 ("-Dmc68000 -Dsun -Dunix" , 0);
	    break;

	  case 'P':
	    {
	      char *x = (char *) __builtin_alloca  (strlen ("-Dmc68000 -Dsun -Dunix" ) * 2 + 1);
	      char *buf = x;
	      char *y = "-Dmc68000 -Dsun -Dunix" ;

	      while (1)
		{
		  if (! strncmp (y, "-D", 2))
		    {
		      *x++ = '-';
		      *x++ = 'D';
		      *x++ = '_';
		      *x++ = '_';
		      y += 2;
		    }
		  else if (*y == ' ' || *y == 0)
		    {
		      *x++ = '_';
		      *x++ = '_';
		      if (*y == 0)
			break;
		      else
			*x++ = *y++;
		    }
		  else
		    *x++ = *y++;
		}
	      *x = 0;

	      do_spec_1 (buf, 0);
	    }
	    break;

	  case 'S':
	    do_spec_1 ("%{pg:gcrt0.o%s}%{!pg:%{p:mcrt0.o%s}%{!p:crt0.o%s}}	   %{mfpa:Wcrt1.o%s}					   %{msoft-float:Fcrt1.o%s}				   %{!mfpa:%{!msoft-float:Mcrt1.o%s}}" , 0);
	    break;

	  default:
	    abort ();
	  }
	break;

      default:
	({ struct obstack *__o = (&obstack);	((__o->next_free + 1 > __o->chunk_limit)	? _obstack_newchunk (__o, 1) : 0),	*(__o->next_free)++ = ( c);	(void) 0; }) ;
	arg_going = 1;
      }

  return 0;		 
}

char *
handle_braces (p)
     register char *p;
{
  register char *q;
  char *filter;
  int pipe = 0;
  int negate = 0;

  if (*p == '|')

    pipe = 1, ++p;

  if (*p == '!')

    negate = 1, ++p;

  filter = p;
  while (*p != ':' && *p != '}') p++;
  if (*p != '}')
    {
      register int count = 1;
      q = p + 1;
      while (count > 0)
	{
	  if (*q == '{')
	    count++;
	  else if (*q == '}')
	    count--;
	  else if (*q == 0)
	    abort ();
	  q++;
	}
    }
  else
    q = p + 1;

  if (p[-1] == '*' && p[0] == '}')
    {
      register int i;
      --p;
      for (i = 0; i < n_switches; i++)
	if (!strncmp (switches[i].part1, filter, p - filter))
	  give_switch (i);
    }
  else
    {
      register int i;
      int present = 0;

      if (p[-1] == '*')
	{
	  for (i = 0; i < n_switches; i++)
	    {
	      if (!strncmp (switches[i].part1, filter, p - filter - 1))
		{
		  switches[i].valid = 1;
		  present = 1;
		}
	    }
	}
      else
	{
	  for (i = 0; i < n_switches; i++)
	    {
	      if (!strncmp (switches[i].part1, filter, p - filter)
		  && switches[i].part1[p - filter] == 0)
		{
		  switches[i].valid = 1;
		  present = 1;
		  break;
		}
	    }
	}

      if (present != negate)
	{
	  if (*p == '}')
	    {
	      give_switch (i);
	    }
	  else
	    {
	      if (do_spec_1 (save_string (p + 1, q - p - 2), 0) < 0)
		return 0;
	    }
	}
      else if (pipe)
	{

	  do_spec_1 ("-");
	}
    }

  return q;
}

give_switch (switchnum)
     int switchnum;
{
  do_spec_1 ("-", 0);
  do_spec_1 (switches[switchnum].part1, 1);
  do_spec_1 (" ", 0);
  if (switches[switchnum].part2 != 0)
    {
      do_spec_1 (switches[switchnum].part2, 1);
      do_spec_1 (" ", 0);
    }
  switches[switchnum].valid = 1;
}

char *
find_file (name)
     char *name;
{
  int size;
  char *temp;
  int win = 0;

  size = strlen (standard_exec_prefix);
  if (user_exec_prefix != 0 && strlen (user_exec_prefix) > size)
    size = strlen (user_exec_prefix);
  if (env_exec_prefix != 0 && strlen (env_exec_prefix) > size)
    size = strlen (env_exec_prefix);
  if (strlen (standard_exec_prefix) > size)
    size = strlen (standard_exec_prefix);
  if (strlen (standard_exec_prefix_1) > size)
    size = strlen (standard_exec_prefix_1);
  if (strlen (standard_startfile_prefix) > size)
    size = strlen (standard_startfile_prefix);
  if (strlen (standard_startfile_prefix_1) > size)
    size = strlen (standard_startfile_prefix_1);
  if (strlen (standard_startfile_prefix_2) > size)
    size = strlen (standard_startfile_prefix_2);
  size += strlen (name) + 1;

  temp = (char *) __builtin_alloca  (size);

  if (user_exec_prefix)
    {
      strcpy (temp, user_exec_prefix);
      strcat (temp, name);
      win = (access (temp, 4 ) == 0);
    }

  if (!win && env_exec_prefix)
    {
      strcpy (temp, env_exec_prefix);
      strcat (temp, name);
      win = (access (temp, 4 ) == 0);
    }

  if (!win)
    {
      strcpy (temp, standard_exec_prefix);
      strcat (temp, name);
      win = (access (temp, 4 ) == 0);
    }

  if (!win)
    {
      strcpy (temp, standard_exec_prefix_1);
      strcat (temp, name);
      win = (access (temp, 4 ) == 0);
    }

  if (!win)
    {
      strcpy (temp, standard_startfile_prefix);
      strcat (temp, name);
      win = (access (temp, 4 ) == 0);
    }

  if (!win)
    {
      strcpy (temp, standard_startfile_prefix_1);
      strcat (temp, name);
      win = (access (temp, 4 ) == 0);
    }

  if (!win)
    {
      strcpy (temp, standard_startfile_prefix_2);
      strcat (temp, name);
      win = (access (temp, 4 ) == 0);
    }

  if (!win)
    {
      strcpy (temp, "./");
      strcat (temp, name);
      win = (access (temp, 4 ) == 0);
    }

  if (win)
    return save_string (temp, strlen (temp));
  return name;
}

void
fatal_error (signum)
     int signum;
{
  signal (signum, 	(void (*)())0 );
  delete_temp_files (0);

  kill (getpid (), signum);
}

int
main (argc, argv)
     int argc;
     char **argv;
{
  register int i;
  int value;
  int nolink = 0;
  int error_count = 0;

  programname = argv[0];

  if (signal (2	, 	(void (*)())1 ) != 	(void (*)())1 )
    signal (2	, fatal_error);
  if (signal (1	, 	(void (*)())1 ) != 	(void (*)())1 )
    signal (1	, fatal_error);
  if (signal (15	, 	(void (*)())1 ) != 	(void (*)())1 )
    signal (15	, fatal_error);

  argbuf_length = 10;
  argbuf = (char **) xmalloc (argbuf_length * sizeof (char *));

  _obstack_begin ((&obstack), 0, 0, xmalloc , free ) ;

  choose_temp_base ();

  process_command (argc, argv);

  if (vflag)
    {
      extern char *version_string;
      fprintf ((&_iob[2]) , "gcc version %s\n", version_string);
      if (n_infiles == 0)
	exit (0);
    }

  if (n_infiles == 0)
    fatal ("No input files specified.");

  outfiles = (char **) xmalloc (n_infiles * sizeof (char *));
  memset (outfiles,0, n_infiles * sizeof (char *)) ;

  for (i = 0; i < n_infiles; i++)
    {
      register struct compiler *cp;

      input_filename = infiles[i];
      input_filename_length = strlen (input_filename);
      input_file_number = i;

      outfiles[i] = input_filename;

      for (cp = compilers; cp->spec; cp++)
	{
	  if (strlen (cp->suffix) < input_filename_length
	      && !strcmp (cp->suffix,
			  infiles[i] + input_filename_length
			  - strlen (cp->suffix)))
	    {
	      register char *p;

	      input_basename = input_filename;
	      for (p = input_filename; *p; p++)
		if (*p == '/')
		  input_basename = p + 1;
	      basename_length = (input_filename_length - strlen (cp->suffix)
				 - (input_basename - input_filename));
	      value = do_spec (cp->spec);
	      if (value < 0)
		error_count = 1;
	      break;
	    }
	}

      if (! cp->spec && nolink)
	{

	  error ("%s: linker input file unused since linking not done",
		 input_filename);
	}

      clear_failure_queue ();
    }

  if (! nolink && error_count == 0)
    {
      value = do_spec (link_spec);
      if (value < 0)
	error_count = 1;
    }

  validate_all_switches ();

  for (i = 0; i < n_switches; i++)
    if (! switches[i].valid)
      error ("unrecognized option `-%s'", switches[i].part1);

  delete_temp_files (error_count == 0);

  exit (error_count);
}
xmalloc (size)
     int size;
{
  register int value = malloc (size);
  if (value == 0)
    fatal ("Virtual memory full.");
  return value;
}

xrealloc (ptr, size)
     int ptr, size;
{
  register int value = realloc (ptr, size);
  if (value == 0)
    fatal ("Virtual memory full.");
  return value;
}

char *
concat (s1, s2, s3)
     char *s1, *s2, *s3;
{
  int len1 = strlen (s1), len2 = strlen (s2), len3 = strlen (s3);
  char *result = (char *) xmalloc (len1 + len2 + len3 + 1);

  strcpy (result, s1);
  strcpy (result + len1, s2);
  strcpy (result + len1 + len2, s3);
  *(result + len1 + len2 + len3) = 0;

  return result;
}

char *
save_string (s, len)
     char *s;
     int len;
{
  register char *result = (char *) xmalloc (len + 1);

  memcpy ( result,s, len) ;
  result[len] = 0;
  return result;
}

pfatal_with_name (name)
     char *name;
{
  extern int errno, sys_nerr;
  extern char *sys_errlist[];
  char *s;

  if (errno < sys_nerr)
    s = concat ("%s: ", sys_errlist[errno], "");
  else
    s = "cannot open %s";
  fatal (s, name);
}

perror_with_name (name)
     char *name;
{
  extern int errno, sys_nerr;
  extern char *sys_errlist[];
  char *s;

  if (errno < sys_nerr)
    s = concat ("%s: ", sys_errlist[errno], "");
  else
    s = "cannot open %s";
  error (s, name);
}

perror_exec (name)
     char *name;
{
  extern int errno, sys_nerr;
  extern char *sys_errlist[];
  char *s;

  if (errno < sys_nerr)
    s = concat ("installation problem, cannot exec %s: ",
		sys_errlist[errno], "");
  else
    s = "installation problem, cannot exec %s";
  error (s, name);
}

fatal (msg, arg1, arg2)
     char *msg, *arg1, *arg2;
{
  error (msg, arg1, arg2);
  delete_temp_files (0);
  exit (1);
}

error (msg, arg1, arg2)
     char *msg, *arg1, *arg2;
{
  fprintf ((&_iob[2]) , "%s: ", programname);
  fprintf ((&_iob[2]) , msg, arg1, arg2);
  fprintf ((&_iob[2]) , "\n");
}

void
validate_all_switches ()
{
  struct compiler *comp;
  register char *p;
  register char c;

  for (comp = compilers; comp->spec; comp++)
    {
      p = comp->spec;
      while (c = *p++)
	if (c == '%' && *p == '{')
	  validate_switches (p + 1);
    }

  p = link_spec;
  while (c = *p++)
    if (c == '%' && *p == '{')
      validate_switches (p + 1);
}

void
validate_switches (start)
     char *start;
{
  register char *p = start;
  char *filter;
  register int i;

  if (*p == '|')
    ++p;

  if (*p == '!')
    ++p;

  filter = p;
  while (*p != ':' && *p != '}') p++;

  if (p[-1] == '*')
    {
      --p;
      for (i = 0; i < n_switches; i++)
	if (!strncmp (switches[i].part1, filter, p - filter))
	  switches[i].valid = 1;
    }
  else
    {
      for (i = 0; i < n_switches; i++)
	{
	  if (!strncmp (switches[i].part1, filter, p - filter)
	      && switches[i].part1[p - filter] == 0)
	    switches[i].valid = 1;
	}
    }
}

