/*
 * C++ interface to Unix signals
 */

#ifndef signal_h
#define signal_h

/*
 * Use standard system definitions, but ignore forward function declarations
 * because they probably don't have argument prototypes.
 */

#define signal c_signal
#include <sys/signal.h>
#undef signal

typedef void (*SignalHandler)(...);

extern SignalHandler signal(int, SignalHandler);
extern int kill(int, int);

#define SignalBad ((SignalHandler)-1)
#define SignalDefault ((SignalHandler)0)
#define SignalIgnore ((SignalHandler)1)

/*
 * BSD-specific signal routines.  On hpux, some of them are declared
 * with long parameters.
 */

#ifdef hpux
extern long sigsetmask(long mask);
extern long sigblock(long mask);
extern long sigpause(long mask);
extern int sigvec(int sig, struct sigvec* v, struct sigvec* prev);
#else
extern int sigsetmask(int mask);
extern int sigblock(int mask);
extern int sigpause(int mask);
extern int sigvec(int sig, struct sigvec* v, struct sigvec* prev);
#endif

#endif
