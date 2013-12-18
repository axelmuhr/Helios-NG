
/* C compiler file system.h :  Copyright (C) A.Mycroft and A.C.Norman */
/* $Id: system.h,v 1.2 1994/09/23 08:44:45 nickc Exp $ */

#define CC_VERSION "1.5a(3)"
/*#pragma -e1 */              /* temp hack to allow #error to continue */

/*      #define ARM2   */     /* #define this for an ARM-2 system */

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

#ifndef TARGET_MACHINE
#  define TARGET_IS_ARM 1
#  define TARGET_MACHINE "ARM"
#endif

#ifdef COMPILING_ON_XPUTER
#ifndef __CC_NORCROFT
#define __CC_NORCROFT
#endif
#define NO_INSTORE_FILES 1
#endif

#ifdef __CC_NORCROFT
/* calls to all non-ansi functions are removable by macros here */
#ifndef _VFPRINTF_AVAILABLE
/* Cope with new introduction of an integer-only version of vfprintf */
/* #error newer library has _vfprintf in it */
#define _vfprintf vfprintf
#define _VFPRINTF_AVAILABLE 1
#else /* ACE - I added this */
#define _vfprintf vfprintf
#endif
extern void _mapstore(void);    /* for driver.c */
extern void _postmortem(void);   /* for misc.c   */
#ifndef NO_INSTORE_FILES
  extern FILE *_fopen_string_file(const char *,int);  /* for driver.c */
#endif
#else
#  define _vfprintf vfprintf
#  define _mapstore() (void)0
#  define _postmortem() exit(1)
#  define NO_INSTORE_FILES 1
#  define _fopen_string_file(s,n) NULL
#endif

/* note that for object file purposes CC_BANNERlen MUST be a multiple of 4
   and include the final null in CC_BANNER (which should preferably be
   normalised by zero padding in the last word)... */

#ifdef PASCAL
#  define LANGUAGE Pascal
#  define NO_INSTORE_FILES 1
#else
#  define LANGUAGE "C"
#endif

#ifdef COMPILING_ON_XPUTER
#define CC_BANNER     \
  "Norcroft TARGET_MACHINE  LANGUAGE CC_VERSION  __DATE__ \0\0\0"
#define CC_BANNERlen  (sizeof(CC_BANNER) & ~3)
#endif

#define INTWIDTH 4

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

#ifdef TARGET_IS_370
/* The following #define's ensure that external symbols are limited         */
/* to 8 chars without gratuitous changes to every file.                     */
/* I do it with macros to avoid changes to many files and to keep           */
/* names meaningful.  A good alternative is to use 'static' for local       */
/* definitions so that the linker never sees them.                          */
/* Later (!) I want to catch and fix such things with an error message.     */
/* N.B. The ansi spec requires only 6 chars 1 case distinction.             */
#define xglobal_list1 xg1
#define xglobal_list2 xg2
#define xglobal_list3 xg3
#define xglobal_list4 xg4
#define xglobal_list5 xg5
#define xglobal_list6 xg6
#define global_mk_binder     g_mkbind
#define global_mk_tagbinder  g_mktagb
#define syn_list2     sl2
#define syn_list3     sl3
#define syn_list5     sl5
#define mk_cmd_default   mkcmddef
#define mk_cmd_do        mkcmddo
#define stackoverflow    stkover0
#define stackoverflow1   stkover1
#define builtin_init        bltinini
#define builtin_headers     bltinhdr
#define fltrep_narrow       flt_nrrw
#define fltrep_narrow_round flt_nrnd
#define globalize_int      glbl_int
#define globalize_typeexpr glbl_te
#define label_reference    lbl_ref
#define label_resolve      lbl_rslv
#define optimiselist           optmslst
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
#endif

/* end of system.h */
