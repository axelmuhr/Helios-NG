
/* C compiler file system.h :  Copyright (C) A.Mycroft and A.C.Norman */

#define CC_VERSION "1.60a"

#ifdef __CC_NORCROFT
#  pragma -e1                 /* temp hack to allow #error to continue */
#endif

/* #define ARM2  */           /* #define this for an ARM-2 system */

#define ENABLE_ALL 1          /* enable all internal debugging opts */

#undef SETJMP_CAN_ONLY_BE_USED_DIRECTLY_BY_NAME      /* ANSI OCT 86 */

#ifdef ACN                    /* arthurs private system */
#  undef   NO_VALOF_BLOCKS
#  define  SETJMP_CAN_ONLY_BE_USED_DIRECTLY_BY_NAME 1
#endif

/***********************************************************************/
/*                                                                     */
/*      C O M P I L E R     F O R    T H E    C    L A N G U A G E     */
/*                                                                     */
/*                       including Pascal compiler                     */
/*                                                                     */
/*                                                                     */
/*              A. C. Norman  and  A. Mycroft.   March 1986            */
/*                                                                     */
/*                                                                     */
/*                       Copyright 1987 ACN/AM                         */
/*         Do not use or redistribute without written permission       */
/*                                                                     */
/***********************************************************************/

/* The following two (non-ansi approved) flags are the approved way of  */
/* testing the machine on which one is compiling.  These are needed to  */
/* determine how to lookup #include files -- see driver.c               */
/* N.B. No other module should do #ifdef ARM or such like.              */
#ifdef ARM
#  define COMPILING_ON_ARM
#endif
#ifdef ibm370
#  define COMPILING_ON_370
#endif
#ifdef clipper
#  define COMPILING_ON_CLIPPER
#endif
#ifdef MSDOS               /* I wish I could change this */
#  define COMPILING_ON_HIGH_C
#endif
#ifdef ACW
#  define COMPILING_ON_ACW
#endif
#ifdef AMD
#  define COMPILING_ON_AMD
#endif
#ifdef AMIGA
#  define COMPILING_ON_AMIGA
#endif
#ifdef TRIPOS
#  define COMPILING_ON_TRIPOS
#  define TRIPOS_OBJECTS
#endif
#ifdef NEWTRIPOS
#  define COMPILING_ON_NEWTRIPOS
#endif

/* The following typedefs may need alteration for obscure host machines */

#ifdef COMPILING_ON_HIGH_C
/* For cross-compilation using High-C on an MSDOS system */
typedef long int            int32;
typedef long unsigned int   unsigned32;
typedef short int           int16;
typedef short unsigned int  unsigned16;
typedef signed char         int8;
typedef unsigned char       unsigned8;
typedef int                 bool;
#define forward             extern
#define pack_into_topbits(n,a) ((n) | \
  ((((long)(a)) & 0x0000ffffL) + ((((unsigned long)(a)) & 0xffff0000) >> 12)))
#define unpack_tag(w) ((w) & 0xff000000L)
#define unpack_pointer(w) ((void *)(((w) & 0xfL)|(((w) << 12) & 0xffff0000L)))
#else
/* This is the normal situation where plain ints are 4-bytes wide */
typedef long int            int32;
typedef long unsigned int   unsigned32;
/* The next two lines avoid an ARM-only C warning.  Provide opt to supress? */
#define int16 short int
#define unsigned16 short unsigned int
typedef signed char         int8;
typedef unsigned char       unsigned8;
typedef int                 bool;
#define forward             static
#define pack_into_topbits(n,a) ((n) | ((int32)(a)))
#define unpack_tag(w) ((w) & 0xff000000L)
#define unpack_pointer(w) ((void *)((w) & 0x00ffffffL))
#endif

#undef TARGET_MACHINE

#ifdef TARGET_IS_ARM
#  define TARGET_MACHINE "ARM"
#endif
#ifdef TARGET_IS_370
#  define TARGET_MACHINE "370"
#endif
#ifdef TARGET_IS_CLIPPER
#  define TARGET_MACHINE "CLIPPER"
#endif
#ifdef TARGET_IS_XPUTER
#  define TARGET_MACHINE "Transputer"
#endif
#ifdef TARGET_IS_ACW
#  define TARGET_MACHINE "ACW"
#endif
#ifdef TARGET_IS_AMD
#  define TARGET_MACHINE "AMD29000"
#endif
#ifdef TARGET_IS_68000
#  define TARGET_MACHINE "68000"
#  define TARGET_IS_68000TYPE
#endif
#ifdef TARGET_IS_68020
#  define TARGET_MACHINE "68020"
#  define TARGET_IS_68000TYPE
#endif
#ifdef TARGET_IS_NULL
#  define TARGET_MACHINE "Null"
#endif

#ifndef TARGET_MACHINE
#  error -dTARGET_IS_ARM assumed
#  define TARGET_IS_ARM 1
#  define TARGET_MACHINE "ARM"
#endif

#if defined TARGET_IS_ARM && !defined ARM2    /* arm1's becoming obsolete */
#  undef TARGET_MACHINE
#  define TARGET_MACHINE "ARM-1"              /* for the banner */
#endif

#ifdef __CC_NORCROFT
/* Calls to all non-ansi functions are removable by macros here.           */
/* Note that the system makes direct calls to _vfprintf and _vsprintf      */
/* to avoid loading floating point code.                                   */
extern int _vfprintf(FILE *stream, const char *format, va_list arg);
extern int _vsprintf(char *s, const char *format, va_list arg);
extern void _write_profile(char *filename);     /* for driver.c */
extern void _postmortem(void);                  /* for misc.c   */
#  ifdef COMPILING_ON_ARM
extern int _osbyte(int a, int x, int y, int c); /* Used in driver.c if ARM */
#  endif
#  ifndef NO_INSTORE_FILES
extern FILE *_fopen_string_file(const char *,int32);  /* for driver.c */
#  endif
#  ifndef __LIB_VERSION     /* add to stdio.h in 1.58+ */
#   error Using old library/headers for bootstrap
#   define _vsprintf vsprintf
#   define _write_profile(filename) (void)0
#  endif
#else /* __CC_NORCROFT */
#  define _vfprintf vfprintf
#  define _vsprintf vsprintf
#  define _write_profile(filename) (void)0
#  ifdef COMPILING_ON_HIGH_C
#    define _postmortem() stackdump(2)
#    ifndef NO_INSTORE_FILES
      extern FILE *_fopen_string_file(const char *n,int32 l);
#    endif
#  else
#    define _postmortem() exit(1)
#    define NO_INSTORE_FILES 1   /* so _fopen_string_file() not used */
#  endif
#endif /* __CC_NORCROFT */

/* The following is intended as a general cross compilation pattern. */
#ifdef TARGET_HAS_EBCDIC
#  define char_translation(x) ('A' == 193 ? (x) : _atoe[x])
#  define char_untranslation(x) ('A' == 193 ? (x) : _etoa[x])
   extern char _atoe[], _etoa[];
#else
#  define char_translation(x) (x)
#  define char_untranslation(x) (x)
#endif

#ifdef PASCAL
#  define LANGUAGE "Pascal"
#  define NO_INSTORE_FILES 1
#else
#  define LANGUAGE "C"
#endif

/* The following two lines are the start of trying to separate compile-time */
/* and run-time sizeof(int32) parameterisations.  Beware, there are still     */
/* lots of 4's and 32's in the system.                                      */

#define INTWIDTH 4L       /* compile-time sizeof(int32) -- dying.      */
/* the next line is the RUNTIME size of int - i.e. sizeoftype(te_int)  */
#define sizeof_int 4
#if defined ALIGN2
#define alignof_int 2
#define alignof_struct 2
#else
#define alignof_int 4     /* Acorn want a version with alignof_int=2   */
#define alignof_struct 4  /* Acorn want a version with alignof_int=2   */
#endif
#ifdef PASCAL
/***********************************************************************/
/* Pascal language as defined in BS 6192 / ISO.   (Unfinished)         */
/***********************************************************************/
#else
/***********************************************************************/
/* The language accepted here according to the October 86 draft        */
/* proposed ANSI standard, obtainable from BSI.                        */
/***********************************************************************/
#endif

#ifdef COMPILING_ON_370
/* The following #define's ensure that external symbols are limited         */
/* to 8 chars without gratuitous changes to every file.                     */
/* I do it with macros to avoid changes to many files and to keep           */
/* names meaningful.  A good alternative is to use 'static' for local       */
/* definitions so that the linker never sees them.                          */
/* Later (!) I want to catch and fix such things with an error message.     */
/* N.B. The ansi spec requires only 6 chars 1 case distinction.             */
#define xglobal_list3 xg3
#define xglobal_list4 xg4
#define global_mk_binder     g_mkbind
#define syn_list2     sl2
#define mk_cmd_default   mkcmddef
#define mk_cmd_do        mkcmddo
#define builtin_init        bltinini
#define builtin_headers     bltinhdr
#define fltrep_narrow       flt_nrrw
#define fltrep_narrow_round flt_nrnd
#define globalize_int      glbl_int
#define globalize_typeexpr glbl_te
#define label_reference    lbl_ref
#define label_resolve      lbl_rslv
#define cautious_mcrepofexpr   cmcrepe
#define cautious_mcrepoftype   cmcrept
#define codebuf_init             cseg_ini
#define codebuf_reinit           cseg_rei
#define codebuf_reinit2          cseg_re2
#define codeseg_stringsegs       cseg_str
#define codeseg_flush            cseg_fls
#define codeseg_function_name    cseg_fnn
#define lit_findadcon            lit_fadc
#define lit_findword             lit_fdwd
#define regalloc_init            regall_i
#define regalloc_reinit          regall_r
#define regalloc_tidy            regall_t
#define implicit_return_ok       impl_ret
#define implicit_decl            impl_dcl
#define listing_file             liststream   /* avoid listing_diagnostics */
#endif

/* end of system.h */
