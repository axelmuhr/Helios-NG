
/* signal.c: ANSI draft (X3J11 Oct 86) library code, section 4.7 */
/* Copyright (C) Codemist Ltd., 1990 */

#include <signal.h>
#include "hostsys.h"

struct sigvec {
  void   (*sv_handler)(int);
  int    sv_mask;
  int    sv_onstack;
};

extern volatile int errno;

extern void (*signal (int sig, void (* handler)(int)))(int)
{
  struct sigvec vec, ovec;
  vec.sv_mask = 0;
  vec.sv_onstack = 0;
  vec.sv_handler = handler;
  errno = 0;
  _syscall3(SYS_sigvec, sig, (int)&vec, (int)&ovec);
  if (errno != 0) return SIG_ERR;
  return ovec.sv_handler;
}
