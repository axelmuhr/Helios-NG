#include "hostsys.h"
#include "syscall.h"

int isatty(int fh)
{
  return _sys_istty_(fh);
}


