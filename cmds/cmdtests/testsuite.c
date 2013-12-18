/*{{{  Includes */

#include <helios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

/*}}}*/
/*{{{  Constants */

#define YES (char *) 1

#define ORDINARY     0
#define NETWORK     (1 << 0)
#define COMPILER    (1 << 1)
#define FAILED	    (1 << 2)  

/*}}}*/
/*{{{  Variables */

static const char * 	ProgName;

struct command_table
  {
    char * name;   char * args;			char * infile; char * outfile; int type;
  }
commands[] =
  {
  { "run",      "-d map -m",                       NULL,         NULL       , ORDINARY },
  { "clear",    NULL,                              NULL,         NULL       , ORDINARY },
  { "touch",    "test.out",                        NULL,         YES        , ORDINARY },
  { "rm",       "x?? *.err *.out *.s",             NULL,         NULL       , ORDINARY },
  { "ascii",    NULL,                              NULL,         YES        , ORDINARY },
  { "c",        "-S testsuite.c",                  NULL,         NULL       , COMPILER },
#ifdef __TRAN
  { "c",        "testsuite.s",                     NULL,         NULL       , COMPILER },
#else
  { "c",        "testsuite.c",                     NULL,         NULL       , COMPILER },
#endif
  { "cp",       "testsuite b.out",                 NULL,         NULL       , ORDINARY },
  { "cmp",      "testsuite b.out",		   NULL,         NULL       , ORDINARY },
  { "objed",    "-nb.out b.out",                   NULL,         YES        , COMPILER },
  { "btoa",     NULL,                              "testsuite.c",YES        , ORDINARY },
  { "atob",     NULL,                              "btoa.out",   YES        , ORDINARY },
  { "basename", "testsuite.c .c",                  NULL,         YES        , ORDINARY },
  { "cache",    "-l map",                          NULL,         NULL       , ORDINARY },
  { "cal",      "5 1957",                          NULL,         YES        , ORDINARY },
  { "split",    "-15 testsuite.c",                 NULL,         NULL       , ORDINARY },
  { "cat",      "x??",                             NULL,         YES        , ORDINARY },
  { "chmod",    "v+r z+r x+r y+r testsuite.c",     NULL,         NULL       , ORDINARY },
  { "compress", "-c testsuite.c",                  NULL,         "comp.out" , ORDINARY },
  { "compress", "-dc",                             "comp.out",   YES        , ORDINARY },
  { "diff",     "testsuite.c compress.out",        NULL,         YES        , ORDINARY },
  { "date",     NULL,                              NULL,         YES        , ORDINARY },
  { "df",       NULL,                              NULL,         YES        , ORDINARY },
  { "diag_ns",  "redirect",                        NULL,         "/logger"  , NETWORK  },
  { "diag_tfm", "redirect",                        NULL,         "/logger"  , NETWORK  },
  { "dirname",  "/helios/bin/dirname",             NULL,         YES        , ORDINARY },
  { "domain",   "lock",                            NULL,         NULL       , NETWORK  },
  { "du",       NULL,                              NULL,         YES        , ORDINARY },
  { "dump",     "testsuite.c",                     NULL,         YES        , ORDINARY },
  { "egrep",    "egrep",                           "testsuite.c",YES        , ORDINARY },
  { "fgrep",    "fgrep",                           "testsuite.c",YES        , ORDINARY },
  { "grep",     "grep",                            "testsuite.c",YES        , ORDINARY },
  { "false",    NULL,                              NULL,         NULL       , ORDINARY },
  { "find",     ". -name testsuite.c -print",      NULL,         YES        , ORDINARY },
  { "findns",   NULL,                              NULL,         YES        , NETWORK  },
  { "findsm",   NULL,                              NULL,         YES        , NETWORK  },
  { "findtfm",  NULL,                              NULL,         YES        , NETWORK  },
  { "gdi",      "/helios/etc/devinfo.src gdi.out", NULL,         YES        , ORDINARY },
  { "head",     "-10 testsuite.c",                 NULL,         YES        , ORDINARY },
  { "id",       NULL,                              NULL,         YES        , ORDINARY },
  { "run",      "-d emacs",                        NULL,         NULL       , ORDINARY },
  { "kill",     "emacs",                           NULL,         NULL       , ORDINARY },
  { "cp",       "testsuite.c /ram",                NULL,         NULL       , ORDINARY },
  { "ln",       "/ram/testsuite.c /ram/ln.out",    NULL,         NULL       , ORDINARY },
  { "loaded",   NULL,                              NULL,         YES        , ORDINARY },
  { "logname",  NULL,                              NULL,         YES        , ORDINARY },
  { "ls",       "/helios/etc",                     NULL,         YES        , ORDINARY },
  { "lstatus",  "/00 2",                           NULL,         YES        , NETWORK  },
  { "make",     NULL,                              NULL,         YES        , NETWORK | COMPILER },
  { "cdl",      "pi.cdl 4",                        NULL,         YES        , NETWORK | COMPILER },
  { "map",      NULL,                              "map.in",     YES        , ORDINARY },
  { "mem",      NULL,                              NULL,         YES        , ORDINARY },
  { "mkdir",    "testdir",                         NULL,         NULL       , ORDINARY },
  { "more",     NULL,                              "testsuite.c",YES        , ORDINARY },
  { "mv",       "testsuite.c testdir",             NULL,         NULL       , ORDINARY },
  { "mv",       "testdir/testsuite.c .",           NULL,         NULL       , ORDINARY },
  { "rmdir",    "testdir",                         NULL,         NULL       , ORDINARY },
  { "network",  "show",                            NULL,         YES        , NETWORK },
  { "objdump",  "worker.o",                        NULL,         NULL       , COMPILER },
  { "od",       "-cx testsuite.c",                 NULL,         YES        , ORDINARY },
  { "pr",       "testsuite.c",                     NULL,         YES        , ORDINARY },
  { "ps",       "all",                             NULL,         YES        , ORDINARY },
  { "refine",   "+rv testsuite.c",                 NULL,         YES        , ORDINARY },
  { "remote",   "/00 ps",                          NULL,         YES        , NETWORK  },
  { "rev",      "xaa",                             NULL,         YES        , ORDINARY },
  { "rmgen",    "/helios/etc/default",             NULL,         YES        , NETWORK  },
  { "sendto",   "`cat whoami.out`",                "sendto.in",  YES        , NETWORK  },
  { "shell",    "-fc echo hello",                  NULL,         YES        , ORDINARY },
  { "sleep",    "1",                               NULL,         NULL       , ORDINARY },
  { "slice",    NULL,                              NULL,         YES        , ORDINARY },
  { "sort",     "xaa",                             NULL,         YES        , ORDINARY },
  { "startns",  NULL,                              NULL,         YES        , NETWORK  },
  { "strings",  "testsuite",                       NULL,         YES        , ORDINARY },
  { "tail",     "-15 testsuite.c",                 NULL,         YES        , ORDINARY },
  { "tcp",      "testsuite.c tcp.out",             NULL,         NULL       , ORDINARY },
  { "tee",      "tee_copy.out",                    "testsuite.c",YES        , ORDINARY },
  { "true",     NULL,                              NULL,         NULL       , ORDINARY },
  { "tty",      NULL,                              NULL,         YES        , ORDINARY },
  { "uptime",   NULL,                              NULL,         YES        , ORDINARY },
  { "users",    NULL,                              NULL,         YES        , NETWORK  },
  { "waitfor",  "/helios",                         NULL,         NULL       , ORDINARY },
  { "wc",       "testsuite.c",                     NULL,         YES        , ORDINARY },
  { "which",    "which",                           NULL,         YES        , ORDINARY },
  { "whichend", NULL,                              NULL,         YES        , ORDINARY },
  { "who",      NULL,                              NULL,         YES        , NETWORK  },
  { "whoami",   NULL,                              NULL,         YES        , NETWORK  },
  { "write",    "`cat whoami.out`",                "write.in",   YES        , NETWORK  },
  { "wall",     NULL,                              "wall.in",    YES        , NETWORK  },
  { "wsh",      NULL,                              NULL,         YES        , ORDINARY },
  { "xlatecr",  "testsuite.c",                     NULL,         NULL       , ORDINARY },
  { "kill",     "map",                             NULL,         NULL       , ORDINARY },
  { NULL,       NULL,                              NULL,         NULL       , ORDINARY }
};

/*}}}*/
/*{{{  Code */

/*{{{  usage() */

static void
usage( const char * message, ... )
{
  if (message != NULL)
    {
      va_list	args;
  

      va_start( args, message );

      fprintf( stderr, "%s: ", ProgName );
      
      vfprintf( stderr, message, args );

      fputc( '\n', stderr );
      
      va_end( args );
    }

  fprintf( stderr, "usage: %s [-n|-N] [-c|-C] [-l] [-h] [command]\n", ProgName );
  
#ifdef NEW_SYSTEM
  fprintf( stderr, "       -n       enable testing of network software\n" );
  fprintf( stderr, "       -N       disable testing of network software (default)\n" );
#else
  fprintf( stderr, "       -n       enable testing of network software (default)\n" );
  fprintf( stderr, "       -N       disable testing of network software\n" );
#endif
#ifdef __ARM
  fprintf( stderr, "       -c       enable testing of compiler software\n" );
  fprintf( stderr, "       -C       disable testing of compiler software (default)\n" );
#else
  fprintf( stderr, "       -c       enable testing of compiler software (default)\n" );
  fprintf( stderr, "       -C       disable testing of compiler software\n" );
#endif
  fprintf( stderr, "       -l       lists failed commands upon completion\n" );
  fprintf( stderr, "       -h       display this text and return\n" );
  fprintf( stderr, "       command  start tests at this command\n" );

  return;
}

/*}}}*/
/*{{{  runcmd() */

int
runcmd( struct command_table * com )
{
  char buffer[ 256 ];
  char nextbit[ 80 ];
  
  
  sprintf( buffer, "%s ", com->name );
  
  if (com->args)
    {
      sprintf( nextbit, "%s ", com->args );
      strcat( buffer, nextbit );
    }
  
  if (com->infile)
    {
      strcat( buffer, "< " );
      sprintf( nextbit, "%s ", com->infile );
      strcat( buffer, nextbit );
    }
  
  if (com->outfile)
    {
      strcat( buffer, "> " );
      
      if (com->outfile == YES)
	sprintf( nextbit, "%s.out ", com->name );
      else
	sprintf( nextbit, "%s ", com->outfile );
      
      strcat( buffer, nextbit );
    }
  
  strcat( buffer, ">>2 " );
  
  sprintf( nextbit, "%s.err", com->name );
  
  strcat( buffer, nextbit );
  
  printf( "command \"%s\" - ", buffer );
  
  fflush( stdout );
  
  return (system( buffer ));
}

/*}}}*/
/*{{{  main() */

int
main( int argc, char ** argv )
{
  int 				i;
  int				cmd = 0;
  int				retval;
  int				fails = 0;
  struct command_table *	p;
#ifdef NEW_SYSTEM
  bool				test_network   = FALSE;
#else
  bool				test_network   = TRUE;
#endif
#ifdef __ARM
  bool				test_compiling = FALSE;
#else
  bool				test_compiling = TRUE;
#endif
  bool				list_failures  = FALSE;
  
  
  ProgName = argv[ 0 ];

  while (argc-- > 1)
    {
      /* Parse command line arguments */
      
      if (argv[ argc ][ 0 ] == '-')
	{
	  switch (argv[ argc ][ 1 ])
	    {
	    case 'n':
	      test_network = TRUE;
	      break;

	    case 'N':
	      test_network = FALSE;
	      break;

	    case 'c':
	      test_compiling = TRUE;
	      break;

	    case 'C':
	      test_compiling = FALSE;
	      break;
	      
	    case 'l':
	      list_failures = TRUE;
	      break;
	      
	    case 'h':
	    case 'H':
	    case '?':
	      usage( NULL );
	      return EXIT_SUCCESS;
	      
	    default:
	      usage( "unknown command line switch '%s' - ignored", argv[ argc ] );
	      break;
	    }
	}
      else
	{
	  /* Locate first command user wishes to test */
	  
	  for (p = commands; p->name && strcmp( p->name, argv[ argc ] ); p++, cmd++)
	    ;

	  if (p->name)
	    {
	      /* Make sure that if a command is specifically asked for its testing is enabled */
	      
	      if ((p->type & NETWORK) && test_network == FALSE)
		test_network = TRUE;
	      
	      if ((p->type & COMPILER) && test_compiling == FALSE)
		test_compiling = TRUE;
	    }
	  else
	    {
	      usage( "unable to locate command '%s' - starting at beginning", argv[ argc ] );
	      cmd = 0;
	    }
	}
    }

  /* Run the tests */
  
  for (i = 0, p = commands + cmd; p->name; p++)
    {
      if (((p->type & COMPILER) && !test_compiling) ||
	  ((p->type & NETWORK)  && !test_network)    )
	continue;
      
      retval = runcmd( p );
      i++;
      
      if (!strcmp( p->name, "false" ))
	retval = !retval;
      
      if (retval != EXIT_SUCCESS)
	{
	  p->type = FAILED;
	  fails++;
	}

      printf( retval ? "FAILED\n" : "OK\n" );
      
      fflush(stdout);
    }

  /* Report the results */
  
  printf( "\n%d commands executed, %d failed\n", i, fails );

  if (list_failures && fails > 0)
    {
      printf( "Failed commands were:\n" );

      for (p = commands; p->name; p++)
	if (p->type & FAILED)
	  printf( "  %s\n", p->name );
    }
  
  /* Tidy up after a successful completion */

  if (fails == 0)
    system( "rm *.err *.out xa? *.s" );
  
  return fails > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

/*}}}*/

/*}}}*/

/* end of testsuite.c */
