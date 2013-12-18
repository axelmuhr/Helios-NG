
/* signal.h: ANSI draft (Oct 86) library header, section 4.7 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 */

#ifndef __signal_h
#define __signal_h

typedef int sig_atomic_t;

extern void _ignore_signal_handler(int);
extern void _default_signal_handler(int);
extern void _error_signal_marker(int);

#define SIG_IGN _ignore_signal_handler
#define SIG_DFL _default_signal_handler
#define SIG_ERR _error_signal_marker

#define SIGABRT 1   /* abort                         */
#define SIGFPE  2   /* arithmetic exception          */
#define SIGILL  3   /* illegal instruction           */
#define SIGINT  4   /* attention request from user   */
#define SIGSEGV 5   /* bad memory access             */
#define SIGTERM 6   /* termination request           */
#define SIGSTAK 7   /* stack overflow                */
/* Signal numbers 8 and 9 are available for the user */

extern void (*signal (int sig, void (*func)(int)))(int);
extern int raise(int sig);

#endif

/* end of signal.h */
