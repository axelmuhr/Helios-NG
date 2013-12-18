#pragma force_top_level
#pragma include_only_once
/*
 * mip/globals.h - ubiquitously required definitions
 * Copyright (C) Acorn Computers Ltd., 1988.
 * Copyright (C)  Codemist Ltd., 1988.
 */

/*
 * RCS $Revision: 1.5 $
 * Checkin $Date: 1995/05/19 13:57:09 $
 * Revising $Author: nickc $
 */

#ifndef _globals_LOADED
#define _globals_LOADED

#ifdef __STDC__
#  include <stdarg.h>
#else
#  include <varargs.h>
#endif

#ifndef _host_LOADED
#  include "host.h"
#endif
#ifndef _options_LOADED
#  include "options.h"
#endif
#ifndef _target_LOADED
#  include "target.h"
#endif
#ifndef _defaults_LOADED
#  include "defaults.h"
#endif
#ifndef _defs_LOADED
#  include "defs.h"
#endif

/*
 * DUFF_ADDR is used to initialise pointers which may not be dereferenced.
 * The address chosen traps on dereference on the machines we use.
 * No use is made of its value, save that is assumed to compare equal to
 * itself when used to initialise allocators.  It could, functionally,
 * equally well be NULL, or for the queasy, a pointer to one static byte.
 */
#define DUFF_ADDR (VoidStar)0xbadbadad

/* the following lines ease bootstrapping problems: "\v\f\r" respectively */
/* (allowing different settings from the usual ones below in options.h    */
#ifndef CHAR_NL
#  define CHAR_NL 10
#endif
#ifndef CHAR_VT
#  define CHAR_VT 11
#endif
#ifndef CHAR_FF
#  define CHAR_FF 12
#endif
#ifndef CHAR_CR
#  define CHAR_CR 13
#endif
#define CHAR_BEL ('A'==193 ? 0x2f : 7)

#define YES 1
#define NO  0

/* Beware the following, if n==32 then ANSI-undefined.                     */
#define lsbmask(n) (((unsigned32)1 << (n)) - 1)
#define IGNORE(v) (v=v)  /* for silencing compiler moans about unused args */

#define memclr(a,n) memset(a,0,n)

extern int32 pp_pragmavec[];
/*
 * We put xx!=0 for switches that default to ON, and xx>0 for default off.
 */
#define warn_implicit_fns       (pp_pragmavec['a'-'a'] != 0)  /* cc only */
#define memory_access_checks    (pp_pragmavec['c'-'a'] > 0)   /* mip */
#define warn_deprecated         (pp_pragmavec['d'-'a'] != 0)  /* cc */
/* beware that #pragma -e is temporarily used for #error behaviour */
#define fpregargs_disabled      (pp_pragmavec['g'-'a'] > 0)
#define integerlike_enabled     (pp_pragmavec['h'-'a'] != 0)  /* cc */
#define crossjump_enabled       (pp_pragmavec['j'-'a'] != 0)  /* mip */
#define ldm_enabled             (pp_pragmavec['m'-'a'] > 0)   /* arm */
#define multiple_aof_code_areas (pp_pragmavec['o'-'a'] > 0)   /* arm, mip */
#define profile_option          (pp_pragmavec['p'-'a'] > 0)   /* mip, arm */
#define full_profile_option     (pp_pragmavec['p'-'a'] > 1)   /* mip */
#define no_stack_checks         (pp_pragmavec['s'-'a'] > 0)   /* arm */
#define force_top_level         (pp_pragmavec['t'-'a'] != 0)  /* cc */
#define special_variad           pp_pragmavec['v'-'a']         /* cc */
#define no_side_effects         (pp_pragmavec['y'-'a'] > 0)   /* cc */
#define cse_enabled             (pp_pragmavec['z'-'a'] > 0)   /* mip */
#ifdef TARGET_IS_TRAN
#define library_module		(pp_pragmavec['l'-'a'] > 0)
#endif
#ifdef TARGET_IS_C40
#define fast_FP		        (pp_pragmavec['k'-'a'] > 0)	/* cg */
#define few_modules		(pp_pragmavec['l'-'a'] > 0)	/* gen */
#define backtrace_enabled	(pp_pragmavec['m'-'a'] > 0)	/* gen */
#define no_peepholing		(pp_pragmavec['n'-'a'] > 0)	/* gen */
#define much_data		(pp_pragmavec['w'-'a'] != 0)	/* gen */
#endif

#define var_warn_implicit_fns       pp_pragmavec['a'-'a']
#define var_memory_access_checks    pp_pragmavec['c'-'a']
#define var_warn_deprecated         pp_pragmavec['d'-'a']
#define global_floatreg_var         pp_pragmavec['f'-'a']
#define var_include_once            pp_pragmavec['i'-'a']
#define var_crossjump_enabled       pp_pragmavec['j'-'a']
#define var_ldm_enabled             pp_pragmavec['m'-'a']
#ifdef TARGET_IS_C40
#define var_backtrace_enabled       pp_pragmavec['m'-'a']
#define var_no_peepholing           pp_pragmavec['n'-'a']
#endif
#define var_aof_code_area           pp_pragmavec['o'-'a']
#define var_profile_option          pp_pragmavec['p'-'a']
/* The next pragma provides flags to use during DEVELOPMENT. Permanent */
/* flags should be subsumed into feature, config or a proper pragma.   */
#define var_cc_private_flags        pp_pragmavec['q'-'a']
#define global_intreg_var           pp_pragmavec['r'-'a']
#define var_no_stack_checks         pp_pragmavec['s'-'a']
#define var_force_top_level         pp_pragmavec['t'-'a']
#define var_no_side_effects         pp_pragmavec['y'-'a']
#define var_cse_enabled             pp_pragmavec['z'-'a']

/* static options for compiler */

#define NAMEMAX       256L      /* max no of significant chars in a name  */
#define BIND_HASHSIZE 521L      /* no. of Symstr hash table buckets */

#define SEGSIZE     31744L      /* (bytes) - unit of alloc of hunks         */
                                /* (32K - 1024) for benefit of 16 bit ints  */
#define ICODESEGSIZE  512L      /* Icode vector now allocated in 8k hunks   */
#define CODEVECSEGBITS  9L      /* 2k byte unit of allocation               */
#define CODEVECSEGSIZE (1L<<CODEVECSEGBITS)
#define CODEVECSEGMAX  64L      /* max segments (hence max 128K bytes/proc) */
#define REGHEAPSEGBITS  9L      /* index array for segment of 512 vregs     */
#define REGHEAPSEGSIZE (1L<<REGHEAPSEGBITS)
#define REGHEAPSEGMAX  64L      /* max segments (hence max 32K vregs/proc)  */
/* An old comment claimed that LITPOOLSIZE was 1024 for 'max address range' */
/* I suspect that this is out of date now that litpool overflows gently.    */
#define LITPOOLSEGBITS  5L      /* index array for segment of 32 lits       */
#define LITPOOLSEGSIZE (1L<<LITPOOLSEGBITS)
#define LITPOOLSEGMAX  32L      /* max segments, so 1024 lits ovfl gently   */

/*
 * disable error/warnings...
 */
extern int32 suppress;
/* spare:                       1L */
#define D_ASSIGNTEST            2L
#define D_SHORTWARN             4L
#define D_PPNOSYSINCLUDECHECK   8L
#define D_IMPLICITVOID         16L
#define D_VALOFBLOCKS          32L
#define D_IMPLICITNARROWING    64L
/* spare: was D_ENUM          128L */
#define D_LONGFLOAT           256L
#define D_STRUCTWARN          512L  /* undefined struct/union - now defunct */
#define D_STRUCTPADDING      1024L

#ifdef PASCAL /*ECN*/
#undef D_ASSIGNTEST
#define D_ASSIGNTEST          (~0)
#endif

/* The following are used in some implementations to suppress ERRORS.   */
/* Note: they partly duplicate the -FC (FEATURE_LIMITED_PCC) option.    */
/* Note: suppressing errors makes the implementation non-conforming.    */
#define D_ZEROARRAY       0x10000L
#define D_PPALLOWJUNK     0x20000L
#define D_IMPLICITCAST    0x40000L
#define D_MPWCOMPATIBLE   0x80000L

/* warnings which are disabled by default */
#ifndef D_SUPPRESSED
#  define D_SUPPRESSED (D_SHORTWARN | D_STRUCTWARN | D_STRUCTPADDING)
#endif

#ifdef PASCAL /*ECN*/
/*
 * run time checks
 */
extern int32 rtcheck;
#define RTCHECK_ARRAY                   1L   /* pc */
#define RTCHECK_CASE                    2L   /* pc */
#define RTCHECK_NIL                     4L   /* pc */
#define RTCHECK_REFERENCE               8L   /* pc */
#define RTCHECK_ASSERT                 16L   /* pc */
#define RTCHECK_DEADCODE               32L   /* pc */
#endif

/*
 * features
 */
extern int32 feature;
#define FEATURE_SAVENAME                            1L  /* arm(gen), mip */
#define FEATURE_NOUSE                               2L  /* mip(bind) */
#define FEATURE_PPNOUSE                             4L  /* cc */
#define FEATURE_RELOCATE                            8L  /* UNUSED */
#define FEATURE_FILEX                            0x10L  /* UNUSED */
#define FEATURE_PREDECLARE                       0x20L  /* mip(bind) */
#define FEATURE_ANOMALY                          0x40L  /* mip(regalloc) */
#define FEATURE_ANNOTATE                         0x80L  /* arm(asm), potentially mip */
#define FEATURE_WARNOLDFNS                      0x100L  /* cc */
#define FEATURE_TELL_PTRINT                     0x200L  /* cc */
#define FEATURE_UNEXPANDED_LISTING              0x400L  /* cc */
#define FEATURE_USERINCLUDE_LISTING             0x800L  /* cc */
#define FEATURE_SYSINCLUDE_LISTING             0x1000L  /* cc */
#define FEATURE_6CHARMONOCASE                  0x2000L  /* cc */
#define FEATURE_ALLOWCOUNTEDSTRINGS            0x4000L
#ifdef TARGET_IS_C40
#define FEATURE_OLD_STUBS                      0x8000L  /* gen */
#else
#ifdef TARGET_IS_TRAN
/*
 * Addition to keep fussy people at IGM happy.  Stops warnings about C++
 * identifiers being found.
 * Tony 8/2/95.
 */
#define FEATURE_NO_CPLUSPLUS_WARNINGS	       0x8000L
#endif
#endif /* TARGET_IS_C40 */
#define FEATURE_PCC                           0x10000L  /* cc, mip(bind, misc) */
#define FEATURE_NOWARNINGS                    0x20000L  /* mip(misc) */
#define FEATURE_PPCOMMENT                     0x40000L  /* cc */
#define FEATURE_WR_STR_LITS                   0x80000L  /* mip(flowgraf) */
#define FEATURE_SIGNED_CHAR                  0x100000L  /* cc */
#define FEATURE_FUSSY                        0x200000L  /* for pedants & paranoiacs */
#define FEATURE_UNIX_STYLE_LONGJMP           0x400000L /* mip */
#define FEATURE_LET_LONGJMP_CORRUPT_REGVARS  0x800000L  /* mip *//* meaningful only if _UNIX_STYLE_LONGJMP */
#define FEATURE_AOF_AREA_PER_FN             0x1000000L  /* arm(aaof) */
#define FEATURE_VERBOSE                     0x2000000L  /* mip */
#define FEATURE_DONTUSE_LINKREG             0x4000000L  /* mip */
#define FEATURE_LIMITED_PCC                 0x8000000L  /* pp, sem */
#define FEATURE_KANDR_INCLUDE              0x10000000L  /* mip */
#define FEATURE_INLINE_CALL_KILLS_LINKREG  0x20000000L /* mip */
#ifdef PASCAL /*ECN*/
#define FEATURE_ISO                        0x40000000L  /* cc */
#endif
/* NB cannot use the value 0x80000000L as it must not be negative */

#ifdef PASCAL /*ECN*/
#undef FEATURE_PREDECLARE
#undef FEATURE_WARNOLDFNS
#undef FEATURE_SYSINCLUDE_LISTING
#undef FEATURE_WR_STR_LITS
#undef FEATURE_PCC
#undef FEATURE_ANOMALY
#undef FEATURE_TELL_PTRINT
#undef FEATURE_6CHARMONOCASE
#define FEATURE_PREDECLARE         0
#define FEATURE_WARNOLDFNS         0
#define FEATURE_SYSINCLUDE_LISTING 0
#define FEATURE_WR_STR_LITS        0
#define FEATURE_PCC                0
#define FEATURE_ANOMALY            0
#define FEATURE_TELL_PTRINT        0
#define FEATURE_6CHARMONOCASE      0
#endif

/*
 * Dynamic configuration flags.
 */
extern int32 config;
#define CONFIG_HAS_MULTIPLY      1L  /* Useful for ARM1 vs ARM2 */
#define CONFIG_SLOW_COND_FP_EXEC 2L  /* Useful for ARM1 vs ARM2 */
#define CONFIG_INDIRECT_SETJMP   4L  /* a fn ptr may point to setjmp() */
#define CONFIG_CLIPPER30         8L  /* Special for Intergraph Clipper */
#define CONFIG_ALT_REGUSE     0x10L  /* to generalise normal_sp_sl     */
#define CONFIG_FPREGARGS      0x20L
#define CONFIG_BIG_ENDIAN     0x40L
#define CONFIG_ENDIANNESS_SET 0x80L  /* if this is set (by config_init())  */
                                     /* default of target byte sex to host */
                                     /* does not apply                     */
#define CONFIG_NO_UNALIGNED_LOADS 0x100L
#define CONFIG_REENTRANT_CODE 0x200L

#ifdef TARGET_IS_BIG_ENDIAN
#define target_lsbytefirst 0
#else
#ifdef TARGET_IS_LITTLE_ENDIAN
#define target_lsbytefirst 1
#else
#define target_lsbytefirst ((config & CONFIG_BIG_ENDIAN) == 0)
#endif
#endif

extern bool target_lsbitfirst;       /* ordering for bitfields within word */
extern bool host_lsbytefirst;

extern FILE *asmstream, *objstream;
extern char *sourcefile, *objectfile;
extern int32 xwarncount, warncount, recovercount, errorcount;
extern bool list_this_file;
extern FILE *listingstream;
extern bool implicit_return_ok;
extern char *phasename;
extern struct CurrentFnDetails {
    Symstr *symstr; 
    int xrflags;
} currentfunction;

#ifdef TARGET_IS_ARM
  extern int arthur_module;
#endif

extern FloatCon *real_of_string(const char *s, int32 flag);
extern FloatCon *real_to_real(FloatCon *fc, SET_BITMAP m);
extern FloatCon *int_to_real(int32 n, int32 u, SET_BITMAP m);

extern int32 length(List *l);
extern List *dreverse(List *x);
extern List *nconc(List *x, List *y);

#ifndef max
extern int32 max(int32 a, int32 b);
#endif
extern int32 bitcount(int32 n);

extern void errstate_init(void);

extern int32 cse_debugcount;
extern int32 localcg_debugcount;
extern int32 syserr_behaviour;

#ifdef __CC_NORCROFT
  /*
   * The next procedure takes a string as a format... check args.
   * Note use of a cryptic pragma rather than a nice spelt out one,
   * since the option is only intended for support of this compiler
   */
  #pragma -v3
#endif
extern void cc_msg(char *s, ...);    /* NB still an uncompressed string */
#ifdef __CC_NORCROFT
  #pragma -v0
#endif

/* The following 4 functions are used by the Fortran front end. */
extern void cc_rerr_l(int32 line, char *errorcode, va_list a);
extern void cc_warn_l(int32 line, char *errorcode, va_list a);
extern void cc_err_l(int32 line, char *errorcode, va_list a);
extern void cc_fatalerr_l(int32 line, char *errorcode, va_list a);

extern void compile_abort(int);
extern void summarise(void);
extern void listing_diagnostics(void);

extern void fltrep_stod(const char *s, DbleBin *p);
extern bool fltrep_narrow(DbleBin *d, FloatBin *e);
extern bool fltrep_narrow_round(DbleBin *d, FloatBin *e);
extern void fltrep_widen(FloatBin *e, DbleBin *d);
extern bool flt_add(DbleBin *a, DbleBin *b, DbleBin *c);
extern bool flt_subtract(DbleBin *a, DbleBin *b, DbleBin *c);
extern bool flt_multiply(DbleBin *a, DbleBin *b, DbleBin *c);
extern bool flt_divide(DbleBin *a, DbleBin *b, DbleBin *c);
extern int flt_compare(DbleBin *b, DbleBin *c);
extern bool flt_move(DbleBin *a, DbleBin *b);
extern bool flt_negate(DbleBin *a, DbleBin *b);
extern bool flt_dtoi(int32 *n, DbleBin *a);
extern bool flt_dtou(unsigned32 *u, DbleBin *a);

#ifndef NON_CODEMIST_MIDDLE_END
extern void reg_setallused(RealRegSet *s);      /* not as nasty as memcpy */
#endif

#endif

/* end of mip/globals.h */
