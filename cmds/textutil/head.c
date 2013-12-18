/* head.c - the UNIX head command
 *
 * Author:	N Clifton
 * Version:	$Id: head.c,v 1.6 1992/06/30 12:49:58 nickc Exp $
 * Date:	$Date: 1992/06/30 12:49:58 $
 *
 * Copyright (c) 1990 - 1992 Perihelion Software Ltd.
 *
 * All rights reserved
 *
 */

/*
 * ToDo :-
 *
 * handle the following :-
 *
 *	head fred -5 jim -20 harry
 *
 * (showing 10 lines from fred, 5 from jim and 20 from harry)
 *
 *	head fred -20
 *
 * (showing 20 lines from fred)
 *
 *	head -5 fred -20
 *
 * (showing 5 lines from fred and printing a warning about the unused -20)
 *
 *	head -5 -20 fred
 *
 * (showing 20 lines from fred and printing a warning about the unused -5)
 *
 *	head -help
 *
 * (showing a help message and terminating)
 *
 */ 

static char *rcsid = "$Header: /hsrc/cmds/textutil/RCS/head.c,v 1.6 1992/06/30 12:49:58 nickc Exp $";

#include <helios.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <posix.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define NO_COUNT_FOUND		-1
#define DEFAULT_NUM_UNITS	10
#define HEAD_BUF_LEN		1024


#define streq( a, b )		(strcmp( a, b ) == 0)

static char *	ProgName 	= NULL;
static bool	count_lines 	= TRUE;
static int	debugging	= 0;
static char *	file_name	= NULL;	


static void
inform(
       const char *	format,
       ... )
/*
 * print a message
 */
{
  va_list	args;
  
  
  va_start( args, format );

  fflush( stderr );

  fseek( stderr, 0L, SEEK_END );

  fprintf( stderr, "%s: ", ProgName );
  
  vfprintf( stderr, format, args );
  
  fprintf( stderr, "\n" );
  
  fflush( stderr );
  
  va_end( args );
  
  return;
  
} /* inform */


static bool
head(
     int		file,
     unsigned int	num_units )
/*
 * read 'num_units' of lines or bytes from the given file
 * descriptor and write them to stdout
 * returns true upon success, false upon failure
 */
{
  static char	buffer[ HEAD_BUF_LEN + 1 ];		/* XXX */
  int		amount;
  int		res;
  
  
  if (num_units < 1)
    return false;
  
  if (file < 0)
    return false;
  
  if (debugging)
    inform( "displaying %d %s of file %s", num_units, count_lines ? "lines" : "chars", file_name );
  	   
  if (count_lines)
    {
      unsigned int	num_lines;
      char *		start;
      char *		end;
      
      
      /*
       * counting lines is difficult.  For efficiency
       * we would like to read large blocks, but this might
       * block on a fifo.  For expediency we should read a
       * character at a time, but this is very slow
       *
       * for now we will do big reads
       *
       */
      
      start = end = buffer;
      num_lines   = 0;
      
      while (num_lines < num_units)
	{
	  int		len;
	  char *	ptr;
	  
	  
	  /* read some data if necessary */
	  
	  if (end == start)
	    {
	      start = buffer;
	      
	      res = read( file, start, HEAD_BUF_LEN );
	      
	      if (res < 0)
		{
		  inform( "error whilst reading input, errno = %d", errno );
		  
		  return false;
		}				
	      
	      if (res == 0)
		{
		  /* end-of-file */
		  
		  return true;
		}

	      end = start + res;
	    }
	  
	  forever
	    {
	      /* look for a line in the data */
	      
	      for (ptr = start; ptr < end; ptr++)
		{
		  if (*ptr == '\n')
		    {
		      /* found a new-line */
		      
		      break;
		    }
		}
	      
	      if (ptr < end)
		{
		  /* found a line - print it out */
		  
		  amount = 0;
		  
		  len = (ptr - start) + 1;	/* include new-line in output */
		  
		  while (amount < len)
		    {	
		      res = write( fileno( stdout ), start + amount, len - amount );
		      
		      if (res < 0)
			{
			  inform( "error whilst writing output, errno = %d", errno );
			  
			  return false;
			}
		      
		      amount += res;
		    }
		  
		  /* reset start */
		  
		  start = ptr + 1;
		  
		  /* increment line count */
		  
		  ++num_lines;
		  
		  /* break out of forever loop */
		  
		  break;	
		}
	      else
		{
		  if (end == buffer + HEAD_BUF_LEN)
		    {
		      /*
		       * no more room for data
		       * try shuffling the data down
		       */
		      
		      if (start > buffer)
			{
			  memcpy( buffer, start, end - start );
			  
			  end = buffer + (end-start);
			  
			  start = buffer;
			}
		      else
			{
			  /*
			   * buffer is full, and it does not contain a line
			   * flush the buffer and read a new line
			   */
			  
			  amount = 0;
			  
			  while (amount < HEAD_BUF_LEN)
			    {	
			      
			      res = write( fileno( stdout ), buffer + amount, HEAD_BUF_LEN - amount );
			      
			      if (res < 0)
				{
				  inform( "error whilst writing output, errno = %d", errno );
				  
				  return false;
				}
			      
			      amount += res;
			    }
			  
			  start = end = buffer;
			  
			  break;
			}
		    }
		  
		  /* no line found - read more data and repeat */
		  
		  res = read( file, end, HEAD_BUF_LEN - (end - buffer) );
		  
		  if (res < 0)
		    {
		      inform( "error whilst reading input, errno = %d", errno );
		    }
		  
		  if (res == 0)
		    {
		      /* end-of-file */
		      
		      return true;
		    }
		  
		  end += res;
		}
	    }
	}
    }
  else
    {
      while (num_units > HEAD_BUF_LEN)
	{
	  /* read in a buffer full */
	  
	  res = read( file, buffer , HEAD_BUF_LEN );
	  
	  if (res < 0)
	    {
	      inform( "error whilst reading input, errno = %d", errno );
	      return false;
	    }
	  
	  if (res == 0)
	    /* end-of-file */
	    return true;
	  
	  /* write out data */
	  
	  res = write( fileno( stdout ), buffer, res );
	  
	  if (res < 0)
	    {
	      inform( "error whilst writing output, errno = %d", errno );
	      return false;
	    }
	  
	  num_units -= res;
	}
      
      /* read in remainder */
      
      res = read( file, buffer, num_units );
      
      if (res < 0)
	{
	  inform( "error whilst reading input, errno = %d", errno );
	  return false;
	}
      
      if (res == 0)
	/* end-of-file */
	return true;
      
      
      /* write out data */
      
      res = write( fileno( stdout ), buffer, num_units );
      
      if (res < 0)
	{
	  inform( "error whilst writing output, errno = %d", errno );
	  return false;
	}
    }
  
  if (debugging)
    inform( "displayed file %s", file_name );
  	   
  return true;
  
} /* head */


int
main(
     int	argc,
     char **	argv )
/*
 * main loop
 */
{
  signed int	num_units;		/* number of lines to read */
  int		i;
  int		j;
  
  
  /* save name of program */
  
  if (strrchr( argv[ 0 ], '/' ))
    {
      ProgName = strrchr( argv[ 0 ], '/' ) + 1;
    }
  else
    {
      ProgName = argv[ 0 ];
    }
  
  /*
   * scan arguments looking for a <-count> option.
   * If one is found, remove it and set 'num_units'
   * If more than one is found, print a informing and
   * use latest value
   */
  
  num_units = NO_COUNT_FOUND;
  
  for (i = j = 1; i < argc; i++)
    {
      char *	arg = argv[ i ];
      
      
      if (arg[ 0 ] == '-')
	{
	  switch (arg[ 1 ])
	    {
	    case '\0':
	      /* an argument of '-' on its own means read stdin */
	      
	      if (j < i)
		argv[ j ] = arg;
	      
	      j++;

	      break;

	    case 'c':
	      if (arg[ 2 ] == '\0')
		count_lines = false;
	      else
		inform( "unrecognised command line option %s", arg );
	      break;

	    case 'l':
	      if (arg[ 2 ] == '\0')
		count_lines = true;
	      else
		inform( "unrecognised command line option %s", arg );
	      break;

	    case 'n':
		{
		  signed int	n;
		  
		  
		  if (arg[ 2 ] == '\0')
		    {
		      if (++i == argc)
			{
			  inform( "number must follow -n option" );
			  
			  exit( 3 );
			}
		      
		      n = atoi( argv[ i ] );
		      
		      if (n < 1)
			{
			  inform( "unexpected value for -n option: %s, - ignored", argv[ i ] );
			}
		      else
			{
			  if (num_units != NO_COUNT_FOUND)
			    {
			      inform( "more than one '-n' option found, using latest value (%d)", n );
			    }
			  
			  num_units = n;
			}
		    }
		  else
		    {
		      n = atoi( &argv[ i ][ 2 ] );
		      
		      if (n < 1)
			{
			  inform( "unexpected value for -n option: %s, - ignored", arg );
			}
		      else
			{
			  if (num_units != NO_COUNT_FOUND)
			    {
			      inform( "more than one '-n' option found, using latest value (%d)", n );
			    }
			  
			  num_units = n;
			}
		    }
		}
	      break;

	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
		{
		  signed int	n;
		  
		  
		  n = atoi( &arg[ 1 ] );
		  
		  if (n < 1)
		    {
		      inform( "unexpected option: %s, - ignored", arg );
		    }
		  else
		    {
		      if (num_units != NO_COUNT_FOUND)
			{
			  inform( "more than one '-count' option found, using latest value (%d)", n );
			}
		      
		      num_units = n;
		    }
		}
	      break;

	    case 'd':
	      debugging++;
	      break;

	    case 'h':
	    case '?':
	      inform( "command line options" );
	      inform( "-c     => count characters" );
	      inform( "-l     => count lines" );
	      inform( "<num>  => number of lines or characters to display" );
	      inform( "-d     => enable debugging" );
	      inform( "-help  => display this information" );
	      inform( "<file> => file to display" );
	      inform( "-      => display from stdin" );
	      
	      return EXIT_SUCCESS;
	      
	    default:
	      inform( "unknown command line argument %s", arg );
	      break;
	    }
	}
      else
	{
	  /* copy arg */
	  
	  if (j < i)
	    argv[ j ] = arg;
	  
	  j++;
	}
    }
  
  /* adjust argc to take account of delete arguments */
  
  argc = j;
  
  if (num_units == NO_COUNT_FOUND)
    num_units = DEFAULT_NUM_UNITS;
  
  /*
   * now read input files
   */
  
  if (argc > 1)
    {
      for (i = 1; i < argc; i++)
	{
	  file_name = argv[ i ];	  
	  
	  if (streq( file_name, "-" ))
	    {
	      file_name = "<stdin>";
	    }
	  
	  if (argc > 2)
	    {
	      /* print a header */
	      
	      if (i == 1)
		{
		  fprintf( stdout, "==> %s <==\n", file_name );
		}
	      else
		{
		  fprintf( stdout, "\n==> %s <==\n", file_name );
		}
	    }
	  
	  if (streq( argv[ i ], "-" ))
	    {
	      /* a filename of '-' means read stdin */
	      
	      if (!head( fileno( stdin ), num_units ))
		{
		  inform( "failed to process %s", file_name );
		}
	    }
	  else
	    {
	      struct stat	stat_buf;
	      int		file;
	      
	      
	      /* try to stat the file */
	      
	      if (stat( file_name, &stat_buf ) != 0)
		{
		  switch (errno)
		    {
		    case EACCES:
		      inform( "%s - permission denied", file_name );
		      break;
		      
		    case ENOENT:
		      inform( "cannot find %s", file_name );
		      break;
		      
		    case ENOTDIR:
		      inform( "%s has a path name with a non-directory component", file_name );
		      break;
		      
		    case ENAMETOOLONG:
		      inform( "the path name %s is too long", file_name );
		      break;
		      
		    default:
		      inform( "unexpected error stating %s, errno = %d", file_name, errno );
		      break;
		    }
		  
		  continue;
		}
	      
	      if (!S_ISREG( stat_buf.st_mode ) && !S_ISFIFO( stat_buf.st_mode ))
		{
		  if (S_ISDIR( stat_buf.st_mode ))
		    {
		      inform( "%s is a directory - ignoring", file_name );
		    }
		  else
		    {
		      inform( "%s is an unknown type of file (stat mode = %x)- ignoring",
			   stat_buf.st_mode, file_name );
		    }
		  
		  continue;
		}
	      
	      /* try to open the file */
	      
	      if ((file = open( file_name, O_RDONLY )) < 0)
		{
		  /* open failed */
		  
		  switch (errno)
		    {
		    case EACCES:
		      inform( "unable to open %s for reading", file_name );
		      break;
		      
		    case EINTR:
		      inform( "open interrupted by a signal - aborting" );
		      exit( 1 );
		      
		    case ENFILE:	
		    case EMFILE:
		      inform( "too many open files - aborting" );
		      exit( 2 );
		      
		    default:
		      inform( "unexpected error trying to open %s, errno = %d",
			   file_name, errno );
		      break;
		    }
		  
		  continue;
		}
	      
	      /* read the file */
	      
	      if (!head( file, num_units ))
		{
		  inform( "failed to process %s", file_name );
		}
	      
	      /* close the file */
	      
	      if (close( file ) != 0)
		{
		  inform( "error whilst closing file %s, errno = %d",
		       file_name, errno );
		}
	      
	      /* do next file */
	    }
	}
    }
  else
    {
      /* no filenames given - read stdin */

      file_name = "<stdin>";
      
      if (!head( fileno( stdin ), num_units ))
	{
	  inform( "failed to process <stdin>" );
	}
    }
  
  /*
   * finished
   */
  
  return EXIT_SUCCESS;

  rcsid = rcsid;
  
} /* main */
