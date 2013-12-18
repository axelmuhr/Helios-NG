/* Test of alarm() and SIGALRM handler - BJK 4th March 1991 */

/*
This test fails in the same way on both the ARM and Tranny.

It prints a continuos stream of "AlarmProc(0x8)" IOdebugs
- There should be five second intervals between the two!

If the number printed loop is ifdef'ed in, then it works correctly.

This source can be found in: /hsrc/cmds/com/ARM/alarmtst.c

*/


#include <helios.h>
#include <stdio.h>
#include <signal.h>
#include <syslib.h> /* for Delay() */

#define DELAY 5 /* seconds */

void AlarmProc(int);

int main(int argc, char **argv)
{
  int  i;
#if 1
  struct sigaction sig;
  
  sig.sa_mask = 0;
  sig.sa_flags = SA_ASYNC;	/* means that signals can be delived asynchronously */
  sig.sa_handler = AlarmProc;	/* but will be in a different thread and hence */
				/* longjmp() cannot be used! */
  IOdebug("sigaction returned %x", sigaction(SIGALRM, &sig, NULL));
  
#else
  void (*oldAlarmProc)(int);
  
  IOdebug("Old SIGALRM sighandler %x", (oldAlarmProc = signal(SIGALRM,AlarmProc))) ;
#endif

  alarm(DELAY);

  IOdebug("waiting") ;

#if 1
  /* doing some posix calls while waiting make it work */
  for (i=0; i< 1000000; printf("%d, ",i++));/*null stat*/
#endif

#if 0
  /* if posix pause()/sleep() are used, then it will work for one AlarmProc */
  /* as expected */
  sleep(100);
#else
  Delay(100*OneSec);
#endif

  IOdebug("completed") ;
}

void AlarmProc(int sig)
{
  IOdebug("AlarmProc(0x%x)",sig) ;
  alarm(DELAY); /* this seems to cause an immediate return to the handler! */
}
