

	/* **Dangerous** */
#define SYS_sbreak	17

#include "hostsys.h"

int _curbrk;
extern volatile int errno;

extern _syscall1(int, int);

extern void * _sbrk(size_t n)
{
  int *ans;
  _curbrk = (_curbrk+7)&(-8);
  errno = 0;
  ans = (int*)_syscall1(SYS_sbreak, _curbrk + (n = (n+7)&(-8)));
  if (ans != 0) return (void *)-1;
  _curbrk += n;
  return (void*)( _curbrk - n);
}
