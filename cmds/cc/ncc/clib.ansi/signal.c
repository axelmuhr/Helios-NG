
/* signal.c: ANSI draft (X3J11 Oct 86) library code, section 4.7 */
/* Copyright (C) Codemist Ltd., 1988 */
/* version 0.01d */

/* N.B. machine dependent messages (only) below. */

#include "hostsys.h"
#include <signal.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include "kernel.h"

#define SIGLAST 29  /* one after highest signal number (see <signal.h>) */

static void (*_signalvector[SIGLAST+1])(int);

extern void __ignore_signal_handler(int sig)
{
    sig = sig;          /* reference it */
    return;
}

static void _real_default_signal_handler(int sig)
{   char *s, v[100];
    switch (sig)
    {
case SIGABRT:
        s = "Abnormal termination (e.g. abort() function)";
        break;
case SIGFPE:
        s = &(_kernel_last_oserror()->errmess[0]);
        break;
case SIGILL:
        s = "Illegal instruction (call to non-function/code corrupted)"
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
default:
        sprintf(s = v, "Unknown signal number %d", sig);
        break;
    }
    _sys_msg(s);                   /* ensure out even if stderr problem */
    _postmortem();
}

#pragma -s1

extern void __default_signal_handler(int sig)
{
      _real_default_signal_handler(sig);
}

extern void __error_signal_marker(int sig)
/* This function should NEVER be called - its value is used as a marker     */
/* return from signal (SIG_ERR).   If someone manages to use pass this      */
/* value back to signal and thence get it invoked we make it behave as      */
/* if signal got SIG_DFL:                                                   */
{
    __default_signal_handler(sig);
}

int raise(int sig)
{
    void (*handler)(int);
    if (sig<=0 || sig>=SIGLAST) return (errno = ESIGNUM);
    handler = _signalvector[sig];
    if (handler==__default_signal_handler)
        (*__default_signal_handler)(sig);
    else if (handler!=__ignore_signal_handler)
    {   _signalvector[sig] = __default_signal_handler;
        _kernel_call_client(sig, 0, 0, (_kernel_ccproc *)handler);
    }
    return 0;
}

int _signal_real_handler(int sig)
{
    if (sig<=0 || sig>=SIGLAST) return 0;
    return (_signalvector[sig]!=__default_signal_handler);
}

#pragma -s0

void (*signal(int sig, void (*func)(int)))(int)
{
    void (*oldf)(int);
    if (sig<=0 || sig>=SIGLAST) return __error_signal_marker;
    oldf = _signalvector[sig];
    _signalvector[sig] = func;
    return oldf;
}

void _signal_init()
{
    int i;
    /* do the following initialisation explicitly so code restartable */
    for (i=1; i<SIGLAST; i++) signal(i, __default_signal_handler);
}

/* end of signal.c */
