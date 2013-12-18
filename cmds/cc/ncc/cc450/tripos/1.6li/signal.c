
/* signal.c: ANSI draft (X3J11 Oct 86) library code, section 4.7 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01d */

/* N.B. machine dependent messages (only) below. */

#include "hostsys.h"
#include <signal.h>
#include <stdio.h>
#include <stddef.h>

#define SIGLAST 10  /* one after highest signal number (see <signal.h>) */
#define ESIGNUM 3   /* Also present in "error.c" */

static void (*_signalvector[SIGLAST])(int);

extern void _ignore_signal_handler(int sig)
{
    sig = sig;          /* reference it */
    return;
}

extern void _default_signal_handler(int sig)
{
    char *s, v[100];
    switch (sig)
    {
case SIGABRT:
        s = "Abnormal termination (e.g. abort() function)";
        break;
case SIGFPE:
        s = "Arithmetic error (division by zero/floating overflow)";
        break;
case SIGILL:
        s = "Illegal instruction (call to non-function/code corrupted)"
#ifdef ARM
            "\n[is the floating point emulator installed?]"
#endif
        ;
        break;
case SIGINT:
        s = "Interrupt received from user";
        break;
case SIGSEGV:
        s = "Illegal address (e.g. wildly outside array bounds)";
        break;
case SIGTERM:
        s = "Termination request received";
        break;
case SIGSTAK:
        s = "Stack overflow";
        break;
default:
        sprintf(s = v, "Unknown signal number %d", sig);
        break;
    }
    _sys_msg(s);                   /* ensure out even if stderr problem */
    _postmortem();
}

extern void _error_signal_marker(int sig)
/* This function should NEVER be called - its value is used as a marker     */
/* return from signal (SIG_ERR).   If someone manages to use pass this      */
/* value back to signal and thence get it invoked we make it behave as      */
/* if signal got SIG_DFL:                                                   */
{
    _default_signal_handler(sig);
}

int raise(int sig)
{
    void (*handler)(int);
    if (sig<=0 || sig>=SIGLAST) return (errno = ESIGNUM);
    handler = _signalvector[sig];
    if (handler==_default_signal_handler)
        (*_default_signal_handler)(sig);
    else if (handler!=_ignore_signal_handler)
    {   _signalvector[sig] = _default_signal_handler;
        (*handler)(sig);
    }
    return 0;
}

void (*signal(int sig, void (*func)(int)))(int)
{
    void (*oldf)(int);
    if (sig<=0 || sig>=SIGLAST) return _error_signal_marker;
    oldf = _signalvector[sig];
    _signalvector[sig] = func;
    return oldf;
}

void _signal_init()
{
    int i;
    /* do the following initialisation explicitly so code restartable */
    for (i=1; i<SIGLAST; i++) signal(i, _default_signal_handler);
}

/* end of signal.c */
