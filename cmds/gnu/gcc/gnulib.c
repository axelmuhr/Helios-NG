/* Subroutines needed by GCC output code on some machines.  */
/* Compile this file with the Unix C compiler!  */

#include "config.h"

/* Define the C data type to use for an SImode value.  */

#ifndef SItype
#define SItype long int
#endif

/* Define the type to be used for returning an SF mode value
   and the method for turning a float into that type.
   These definitions work for machines where an SF value is
   returned in the same register as an int.  */

#ifndef SFVALUE  
#define SFVALUE int
#endif

#ifndef INTIFY
#define INTIFY(FLOATVAL)  (intify.f = (FLOATVAL), intify.i)
#endif

union double_di { double d; int i[2]; };
union flt_or_int { int i; float f; };

union longlong { int i[2]; unsigned int ui[2]; };


#ifdef WORDS_BIG_ENDIAN
#define HIGH 0
#define LOW 1
#else
#define HIGH 1
#define LOW 0
#endif

#ifdef L_eprintf
#include <stdio.h>
/* This is used by the `assert' macro.  */
void
__eprintf (string, line, filename)
     char *string;
     int line;
     char *filename;
{
  fprintf (stderr, string, line, filename);
}
#endif

#ifdef L_umulsi3
SItype
__umulsi3 (a, b)
     unsigned SItype a, b;
{
  return a * b;
}
#endif

#ifdef L_mulsi3
SItype
__mulsi3 (a, b)
     SItype a, b;
{
  return a * b;
}
#endif

#ifdef L_udivsi3
SItype
__udivsi3 (a, b)
     unsigned SItype a, b;
{
  return a / b;
}
#endif

#ifdef L_divsi3
SItype
__divsi3 (a, b)
     SItype a, b;
{
  return a / b;
}
#endif

#ifdef L_umodsi3
SItype
__umodsi3 (a, b)
     unsigned SItype a, b;
{
  return a % b;
}
#endif

#ifdef L_modsi3
SItype
__modsi3 (a, b)
     SItype a, b;
{
  return a % b;
}
#endif

#ifdef L_lshrsi3
SItype
__lshrsi3 (a, b)
     unsigned SItype a, b;
{
  return a >> b;
}
#endif

#ifdef L_lshlsi3
SItype
__lshlsi3 (a, b)
     unsigned SItype a, b;
{
  return a << b;
}
#endif

#ifdef L_ashrsi3
SItype
__ashrsi3 (a, b)
     SItype a, b;
{
  return a >> b;
}
#endif

#ifdef L_ashlsi3
SItype
__ashlsi3 (a, b)
     SItype a, b;
{
  return a << b;
}
#endif

#ifdef L_cmpdi2
SItype
__cmpdi2 (a, b)
     union longlong a, b;
{
  if (a.i[HIGH] < b.i[HIGH])
    return 0;
  else if (a.i[HIGH] > b.i[HIGH])
    return 2;
  if (a.ui[LOW] < b.ui[LOW])
    return 0;
  else if (a.ui[LOW] > b.ui[LOW])
    return 2;
  return 1;
}
#endif

#ifdef L_ucmpdi2
SItype
__ucmpdi2 (a, b)
     union longlong a, b;
{
  if (a.ui[HIGH] < b.ui[HIGH])
    return 0;
  else if (a.ui[HIGH] > b.ui[HIGH])
    return 2;
  if (a.ui[LOW] < b.ui[LOW])
    return 0;
  else if (a.ui[LOW] > b.ui[LOW])
    return 2;
  return 1;
}
#endif

#ifdef L_divdf3
double
__divdf3 (a, b)
     double a, b;
{
  return a / b;
}
#endif

#ifdef L_muldf3
double
__muldf3 (a, b)
     double a, b;
{
  return a * b;
}
#endif

#ifdef L_negdf2
double
__negdf2 (a)
     double a;
{
  return -a;
}
#endif

#ifdef L_adddf3
double
__adddf3 (a, b)
     double a, b;
{
  return a + b;
}
#endif

#ifdef L_subdf3
double
__subdf3 (a, b)
     double a, b;
{
  return a - b;
}
#endif

#ifdef L_cmpdf2
SItype
__cmpdf2 (a, b)
     double a, b;
{
  if (a > b)
    return 1;
  else if (a < b)
    return -1;
  return 0;
}
#endif

#ifdef L_fixunsdfsi
SItype
__fixunsdfsi (a)
     double a;
{
  return (unsigned SItype) a;
}
#endif

#ifdef L_fixunsdfdi
double
__fixunsdfdi (a)
     double a;
{
  union double_di u;
  u.i[LOW] = (unsigned int) a;
  u.i[HIGH] = 0;
  return u.d;
}
#endif

#ifdef L_fixdfsi
SItype
__fixdfsi (a)
     double a;
{
  return (SItype) a;
}
#endif

#ifdef L_fixdfdi
double
__fixdfdi (a)
     double a;
{
  union double_di u;
  u.i[LOW] = (int) a;
  u.i[HIGH] = (int) a < 0 ? -1 : 0;
  return u.d;
}
#endif

#ifdef L_floatsidf
double
__floatsidf (a)
     SItype a;
{
  return (double) a;
}
#endif

#ifdef L_floatdidf
double
__floatdidf (u)
     union double_di u;
{
  register double hi
    = ((double) u.i[HIGH]) * (double) 0x10000 * (double) 0x10000;
  register double low = (unsigned int) u.i[LOW];
  return hi + low;
}
#endif

#ifdef L_addsf3
SFVALUE
__addsf3 (a, b)
     union flt_or_int a, b;
{
  union flt_or_int intify;
  return INTIFY (a.f + b.f);
}
#endif

#ifdef L_negsf2
SFVALUE
__negsf2 (a)
     union flt_or_int a;
{
  union flt_or_int intify;
  return INTIFY (-a.f);
}
#endif

#ifdef L_subsf3
SFVALUE
__subsf3 (a, b)
     union flt_or_int a, b;
{
  union flt_or_int intify;
  return INTIFY (a.f - b.f);
}
#endif

#ifdef L_cmpsf2
SItype
__cmpsf2 (a, b)
     union flt_or_int a, b;
{
  if (a.f > b.f)
    return 1;
  else if (a.f < b.f)
    return -1;
  return 0;
}
#endif

#ifdef L_mulsf3
SFVALUE
__mulsf3 (a, b)
     union flt_or_int a, b;
{
  union flt_or_int intify;
  return INTIFY (a.f * b.f);
}
#endif

#ifdef L_divsf3
SFVALUE
__divsf3 (a, b)
     union flt_or_int a, b;
{
  union flt_or_int intify;
  return INTIFY (a.f / b.f);
}
#endif

#ifdef L_truncdfsf2
SFVALUE
__truncdfsf2 (a)
     double a;
{
  union flt_or_int intify;
  return INTIFY (a);
}
#endif

#ifdef L_extendsfdf2
double
__extendsfdf2 (a)
     union flt_or_int a;
{
  union flt_or_int intify;
  return a.f;
}
#endif

#ifdef L_varargs
#ifdef sparc
	asm (".global ___builtin_saveregs");
	asm ("___builtin_saveregs:");
	asm ("st %i0,[%fp+68]");
	asm ("st %i1,[%fp+72]");
	asm ("st %i2,[%fp+76]");
	asm ("st %i3,[%fp+80]");
	asm ("st %i4,[%fp+84]");
	asm ("retl");
	asm ("st %i5,[%fp+88]");
#else
__builtin_saveregs ()
{
  abort ();
}
#endif
#endif

#ifdef L_bb_init_func
#if defined (sun) && defined (m68k)
struct bb
{
  int initialized;
  char *filename;
  int *counts;
  int ncounts;
  int zero_word;
  int *addresses;
};

__bb_init_function (blocks)
	struct bb *blocks;
{
  extern int __tcov_init;

  if (! ___tcov_init)
    ___tcov_init_func ();

  ___bb_link (blocks->filename, blocks->counts, blocks->nblocks);
}

#endif
#endif

/* frills for C++ */

#ifdef L_builtin_new
typedef void (*vfp)();

extern vfp __new_handler;

char *
__builtin_new (sz)
     long sz;
{
  char *p;

  p = (char *)malloc (sz);
  if (p == 0)
    (*__new_handler) ();
  return p;
}
#endif

#ifdef L_builtin_New
typedef void (*vfp)();

static void
default_new_handler ();

vfp __new_handler = default_new_handler;

char *
__builtin_vec_new (p, maxindex, size, ctor)
     char *p;
     int maxindex, size;
     void (*ctor)();
{
  int i, nelts = maxindex + 1;
  char *rval;

  if (p == 0)
    p = (char *)__builtin_new (nelts * size);

  rval = p;

  for (i = 0; i < nelts; i++)
    {
      ctor (p);
      p += size;
    }

  return rval;
}

vfp
__set_new_handler (handler)
     vfp handler;
{
  vfp prev_handler;

  prev_handler = __new_handler;
  if (handler == 0) handler = default_new_handler;
  __new_handler = handler;
  return prev_handler;
}

vfp
set_new_handler (handler)
     vfp handler;
{
  return __set_new_handler (handler);
}

static void
default_new_handler ()
{
  /* don't use fprintf (stderr, ...) because it may need to call malloc.  */
  write (2, "default_new_handler: out of memory... aaaiiiiiieeeeeeeeeeeeee!\n", 65);
  /* don't call exit () because that may call global destructors which
     may cause a loop.  */
  _exit (-1);
}
#endif

#ifdef L_builtin_del
typedef void (*vfp)();

void
__builtin_delete (ptr)
     char *ptr;
{
  if (ptr)
    free (ptr);
}

void
__builtin_vec_delete (ptr, maxindex, size, dtor, auto_delete_vec, auto_delete)
     char *ptr;
     int maxindex, size;
     void (*dtor)();
     int auto_delete;
{
  int i, nelts = maxindex + 1;
  char *p = ptr;

  ptr += nelts * size;

  for (i = 0; i < nelts; i++)
    {
      ptr -= size;
      dtor (ptr, auto_delete);
    }

  if (auto_delete_vec)
    free (p);
}

#endif

