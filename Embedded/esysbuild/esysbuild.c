/*{{{  Header */

/*
 * esysbuild.c
 *
 *	New, improved sysbuild for embedded Helios.  Configuration information
 * is taken from a file.  The build system is slightly more clever and can
 * recognize  * when some part of the nucleus is missing, but will also allow
 * custom nuclei to be built.
 *
 *	A rom disk server and rom disk file can be included in the nucleus.
 *
 *	 	Copyright (c) 1993 Perihelion Software Ltd
 * 	Copyright (c) 1994 Perihelion Distributed Software Ltd
 *
 * RCS Id: $Id: esysbuild.c,v 1.2 1994/05/16 09:59:39 nickc Exp $
 */

/*}}}*/
/*{{{  Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "defs.h"
#include "externs.h"

/*}}}*/
/*{{{  Variables */

#ifdef DEBUG
int	Debug = TRUE;
int	Info = TRUE;
#else
int	Debug = FALSE;
int	Info = FALSE;
#endif

int	WarningsEnabled = TRUE;
int	ErrorsEnabled 	= TRUE;

int	ShowOnly = FALSE;

char *	ProgName;

FILE *	DebugStream = stderr;

/*}}}*/

/*{{{  Code */

/*{{{  tidyup() */

void tidyup ()
{
	/* close any open file pointers */
	if (ConfigFp != NULL)
	{
		fclose (ConfigFp);
	}

	/* clean up the parse list */
	delete_list ();	

	if (Image != NULL)
	{
		free (Image);
	}

	if (ModFp != NULL)
	{
		fclose (ModFp);
	}

#ifdef HELIOS
	if (RomDiskFp != NULL)
	{
		fclose (RomDiskFp);
	}

	if (RomInFp != NULL)
	{
		fclose (RomInFp);
	}
#endif

	if (NucOutFp != NULL)
	{
		fclose (NucOutFp);
	}

	if (BootStrapFp != NULL)
	{
		fclose (BootStrapFp);
	}
}

/*}}}*/
/*{{{  usage() */

void
usage ( int exit_val )
{
  fprintf (stderr, "usage: %s [-hweDdln] [-c config-file] [-o nucleus]\n", ProgName);
  fprintf (stderr, "		w	Disable warning messages\n");
  fprintf (stderr, "		e	Display errors as warnings\n");
  fprintf (stderr, "		d	Debug information\n");
  fprintf (stderr, "		D	Debug on stdout\n");
  fprintf (stderr, "		l	Long format (display the contents of the nucleus)\n");
  fprintf (stderr, "		n	Display contents of the nucleus, but don't build\n");
  fprintf (stderr, "OR\n");
  fprintf (stderr, "       %s <nucleus-name> {<image-file>}\n", ProgName);

  tidyup ();

  exit (exit_val);
}

/*}}}*/
/*{{{  sysbuild_fatal() */

/* Not turn-offable */

void sysbuild_fatal (char *	fmt,
		     ...)
{
	va_list	args;

	va_start (args, fmt);

	fprintf (stderr, "%s - Fatal Error: ", ProgName);

	vfprintf (stderr, fmt, args);
	
	fprintf (stderr, "\n");

	tidyup ();

	va_end (args);

	exit (SYSBUILDERR_FAIL);
}

/*}}}*/
/*{{{  sysbuild_error() */

void sysbuild_error (char *	fmt,
		    ...)
{
	va_list	args;

	va_start (args, fmt);

	if (ErrorsEnabled)
	{
		fprintf (stderr, "%s - Error: ", ProgName);

		vfprintf (stderr, fmt, args);
	
		fprintf (stderr, "\n");

		tidyup ();

		va_end (args);

		exit (SYSBUILDERR_FAIL);
	}
	else if (WarningsEnabled)
	{
		fprintf (stderr, "%s - Warning: ", ProgName);

		va_start (args, fmt);

		vfprintf (stderr, fmt, args);

		fprintf (stderr, "\n");
	}

	va_end (args);
}

/*}}}*/
/*{{{  sysbuild_warning() */

void sysbuild_warning (char *	fmt,
		      ...)
{
	va_list	args;

	if (WarningsEnabled)
	{
		fprintf (stderr, "%s - Warning: ", ProgName);

		va_start (args, fmt);

		vfprintf (stderr, fmt, args);

		fprintf (stderr, "\n");
	}

	va_end (args);
}

/*}}}*/
/*{{{  sysbuild_debug() */

void sysbuild_debug (char *	fmt,
		    ...)
{
	va_list	args;

	if (!Debug) 	return;

	fprintf (DebugStream, "%s - Debug: ", ProgName);

	va_start (args, fmt);

	vfprintf (DebugStream, fmt, args);

	va_end (args);

	fprintf (DebugStream, "\n");
}

/*}}}*/
/*{{{  sysbuild_info() */

void sysbuild_info (char *	fmt,
		   ...)
{
	va_list	args;

	if (!Info) 	return;

	va_start (args, fmt);

	vfprintf (stdout, fmt, args);

	va_end (args);

	fprintf (stdout, "\n");
}

/*}}}*/
/*{{{  setup() */

/*
 *	Parses the arguments and initialises the global variables.
 */
int
setup (int	argc,
       char *	argv[] )
{
  int		a;
  char *	this_arg;
  int		retval = SYSBUILD_FAIL;

  
  init_configdata ();
  
  if (argc == 1)
    {
      /* use the default values */
      return SYSBUILD_OK;
    }
  
  for (a = 1; a < argc; a++)
    {
      this_arg = argv[a];
      
      if (*this_arg == '-')
	{
	  this_arg++;
	  
	  if (retval == SYSBUILD_OLD)
	    sysbuild_error( "cannot accept command line parameters with old-style sysbuild" );
	  else
	    retval = SYSBUILD_OK;
	  
	  while (*this_arg != '\0')
	    {
	      switch (*this_arg)
		{
		case 'h':
		  usage (0);
		  
		  break;
		  
		case 'w':
		  /* Turn of warning messages */
		  WarningsEnabled = FALSE;
		  
		  break;
		  
		case 'e':
		  /* Error messages become warning messages */
		  ErrorsEnabled = FALSE;
		  
		  break;
		  
		case 'D':
		  DebugStream = stdout;
		case 'd':
		  Debug = !Debug;
		  
		  break;
		  
		case 'l':
		  Info = TRUE;
		  
		  break;
		  
		case 'n':
		  ShowOnly = TRUE;
		  Info = TRUE;
		  
		  break;
		  
		case 'c':
		  this_arg++;
		  
		  if (*this_arg == '\0')
		    {
		      a++;
		      
		      if (a == argc)
			{
			  sysbuild_fatal ("No config file following -c option");
			}
		      this_arg = argv[a];
		    }
		  
		  strcpy (ConfigData.config_file, this_arg);
		  
		  *this_arg = '\0';
		  
		  break;
		  
		case 'o':
		  this_arg++;
		  
		  if (*this_arg == '\0')
		    {
		      a++;
		      
		      if (a == argc)
			{
			  sysbuild_error ("No nucleus file following -o option");
			}
		      
		      this_arg = argv[a];
		    }
		  
		  strcpy (ConfigData.nucleus, this_arg);
		  
		  *this_arg = '\0';
		  
		  break;
		  
		default:
		  sysbuild_warning ( "Unknown command line option '%c'", *this_arg);
		  
		  break;
		}
	      
	      if (*this_arg != '\0')
		this_arg++;
	    }
	}
      else
	{
	  if (retval == SYSBUILD_OK)
	    sysbuild_warning( "Ignoring command line word '%s'", this_arg );
	  else
	    retval = SYSBUILD_OLD;
	}
    }
  
  return retval;
}

/*}}}*/
/*{{{  main() */

int
main( int	argc,
     char *	argv[] )
{
  ProgName = argv[0];
       
  switch (setup ( argc, argv ))
    {
    default:
    case SYSBUILD_FAIL: break;
    case SYSBUILD_OK:
      parse_config_file (ConfigData.config_file);
  
      update_configdata ();
  
      print_configdata ();
  
      if (!ShowOnly)
	{
	  make_nucleus ();
  
	  check_bootstrap ();
  
	  output_image ();
	}
      
      break;
      
    case SYSBUILD_OLD:
      sysbuild( argc, argv );
      break;
    }
    
  tidyup ();
}

/*}}}*/

/*}}}*/
