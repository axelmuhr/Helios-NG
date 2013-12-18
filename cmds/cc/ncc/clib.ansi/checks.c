#include <stdio.h>
#include <stdlib.h>
#include "hostsys.h"

#ifndef __titan
extern unsigned int etext;
#define ETEXT etext
#else
extern unsigned int _etext;
#define ETEXT _etext
#endif

#ifdef __clipper
#define LOW_LIMIT	0x8000
#else
# ifdef __mips
#  define LOW_LIMIT	0x400000
# else
# define LOW_LIMIT	1
# endif
#endif
extern void *_sbrk(int);

unsigned int _rd1chk(unsigned int x)
{
  if ((x < LOW_LIMIT) || (x > (unsigned int)_sbrk(0))) {
    fprintf(stderr,"Read check failure %x\n", x);
    exit(1);
  }
  return x;
}

unsigned int _rd2chk(unsigned int x)
{
  if ((x&1) != 0 || (x < LOW_LIMIT) || (x > (unsigned int)_sbrk(0))) {
    fprintf(stderr,"Read check failure %x\n", x);
    exit(1);
  }
  return x;
}

unsigned int _rd4chk(unsigned int x)
{
  if (((x&3) != 0) || (x < LOW_LIMIT) || (x > (unsigned int)_sbrk(0))) {
    fprintf(stderr,"Read check failure %x\n", x);
    exit(1);
  }
  return x;
}

unsigned int _wr1chk(unsigned int x)
{
  if ((x < ETEXT) || (x > (unsigned int)_sbrk(0))) {
    fprintf(stderr,"Write check failure %x\n", x);
    exit(1);
  }
  return x;
}

unsigned int _wr2chk(unsigned int x)
{
  if ((x&1) != 0 || (x < ETEXT) || (x > (unsigned int)_sbrk(0))) {
    fprintf(stderr,"Write check failure %x\n", x);
    exit(1);
  }
  return x;
}

unsigned int _wr4chk(unsigned int x)
{
  if (((x&3) != 0) || (x < ETEXT) || (x > (unsigned int)_sbrk(0))) {
    fprintf(stderr,"Write check failure %x\n", x);
    exit(1);
  }
  return x;
}

