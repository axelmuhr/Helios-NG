/*
 * This file demonstrates how to install a signal handler
 * under Helios.  The normal method (calling signal()),
 * will work provided that POSIX functions are being
 * called.  The other method (using sigaction()) will
 * ensure that the handler is called, even if no POSIX
 * calls are made.  The sigaction() method is only needed
 * for signals that are asynchronous (SIGABRT, SIGHUP,
 * SIGINT, SIGKILL, SIGQUIT and SIGTERM).  The sigaction()
 * method has one drawback:
 *
 *	The signal handler may not call longjmp()
 *
 * The reason for this is that asynchronous signal handlers
 * are called on their own stack, NOT the stack of the program
 * where they were installed.  This means that longjmp()
 * cannot be used, since the signal handler's stack has nowhere
 * to longjmp() to.  Synchronous signal handlers execute on the
 * stack of the program that installed them, so longjmp() should
 * work.
 *
 * As a general word of advice, avoid using longjmp() :-)
 *
 * This file is copyright (c) 1994 Perihelion Distributed Software.
 */

#include <stdio.h>
#include <helios.h>
#include <signal.h>
#include <syslib.h>

void
handler( int sig )
{
  printf( "signal %d received\n", sig );
}
  
int
main( void )
{
#if 1
  struct sigaction New;
  New.sa_handler = handler;
  New.sa_mask    = 0;
  New.sa_flags   = SA_SETSIG | SA_ASYNC;
  sigaction( SIGINT, &New, NULL );
#else
  signal( SIGINT, handler );
#endif
  
  printf( "hello world\n" );

#if 1
  for (;;) /* do nothing */ ;
#else
  while (1)
    {
      printf( "." );
      fflush( stdout );
      
      Delay( OneSec );
    }
#endif

  return 0;
}
