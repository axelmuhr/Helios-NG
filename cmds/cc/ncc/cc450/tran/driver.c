/*
 * driver.c - driver for Codemist compiler (RISC OS, DOS, Mac/MPW, Unix)
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Codemist Ltd., 1988-1992.
 * Copyright (C) Advanced RISC Machines Limited, 1990-1992.
 */

/*
 * RCS $Revision: 1.1 $  Codemist 181
 * Checkin $Date: 1995/05/19 11:38:17 $
 * Revising $Author: nickc $
 */

/* AM memo: we should look at a getenv() variable to find alternative    */
/* path names to /usr/include etc:  This would greatly facilitate        */
/* installing as an alternative C compiler and reduce the shambles       */
/* (which is being tidied) in env = 2; below.                            */
/* AM notes that DEC's version of MIPS C compiler uses env variables     */
/*   COMP_HOST_ROOT and COMP_TARGET_ROOT for this.                       */

/* AM, Jul 89: COMPILING and TARGET flag tests have been reviewed.       */
/* The effect is than running a RISCOS compiler on UNIX looks like a     */
/* unix compiler (i.e. -S not -s) except for target-specific flags.      */
/* Beware the TARGET_IS_HELIOS flags as not rationalised.                */

/* #define  TESTING   1 */

/* AM: This file should more properly be called unixdriv.c:  the        */
/* rationale is that it munges the command line and repeatedly calls    */
/* the compiler and possibly the linker just like the unix 'cc' cmd.    */
/* It is target-independent, but rather host dependent.  Accordingly    */
/* it needs not to be used on machines like the ibm370 under MVS etc.,  */
/* where the file munging done is totally inappropriate.                */
/* We need a HOST_ or COMPILING_ON_ test whether to include this file.  */
/* Further, we would like to suppress the LINK option unless the        */
/* host and target are have compatible object format.                   */

/* Host dependencies:
   1. use of getenv("c$libroot").
   2. file name munging.
   3. The HELP text in drivhelp.h
   More?
*/

#ifndef __GNUC__
#  include <stddef.h>
#endif
#ifdef __STDC__
#  include <stdlib.h>
#  include <string.h>
#else
#  include <strings.h>
extern void exit();
extern char *getenv();
extern char *malloc();
extern char *realloc();
extern int  system();
#endif
#include <ctype.h>
#include <stdio.h>
#ifndef COMPILING_ON_MSDOS
#include <signal.h>
#endif
#ifdef __GNUC__
#  include <stddef.h>
#endif

#include "globals.h"
#include "compiler.h"
#include "fname.h"
#include "version.h"
#include "drivhelp.h"
#include "mcdep.h"
#include "prgname.h"

#ifdef TARGET_IS_HELIOS
extern void IOdebug (const char *, ... );
#endif

    /*************************************************************/
    /*                                                           */
    /*   Set up default compilation options (Arthur,Msdos,Unix)  */
    /*                                                           */
    /*************************************************************/

/* The meaning of the bits in the user's number n in '-On' */
#define MINO_CSE            0x1
#define MINO_NAO            0x2
#define MINO_MAX            0x3    /* sum of above */

#define SMALL_COMMAND       128
#define INCLUDE_BUFLEN      64     /* initial size of include buffer  */
#define MAX_TEXT            256    /* The longest filename I can handle */

/* #define BSD_CC              "/usr/ucb/cc" */

#define KEY_ANSI            0x0000001L
#define KEY_HOST_LIB        0x0000002L
#define KEY_HELP            0x0000004L
#define KEY_LINK            0x0000008L
#define KEY_LISTING         0x0000010L
#define KEY_PCC             0x0000020L

#define KEY_STRICT          0x0000040L
#define KEY_COMMENT         0x0000080L
#define KEY_ASM_OUT         0x0000100L
#define KEY_XPROFILE        0x0000200L
#define KEY_MAKEFILE        0x0000400L
#define KEY_PREPROCESS      0x0000800L
#define KEY_PROFILE         0x0001000L
#define KEY_RENAME          0x0002000L
#define KEY_UNIX            0x0004000L
#define KEY_MD              0x0008000L
#define KEY_READONLY        0x0010000L
#define KEY_STDIN           0x0020000L
#define KEY_COUNTS          0x0040000L
#ifdef  RISCiX113                       /* LDS 31-July-92 @@@ */
#  define KEY_RIX120        0x0080000L  /* Sad history; retire soon? */
#  define KEY_NOSYSINCLUDES 0x0000000L  /* Avoid spurious #ifdef RISCiX113 */
#else
#  define KEY_NOSYSINCLUDES 0x0080000L
#endif
#ifdef TARGET_ENDIANNESS_CONFIGURABLE
#  define KEY_LITTLEEND    0x00100000L
#  define KEY_BIGEND       0x00200000L
#endif
#define KEY_ERRORSTREAM    0x00400000L
#ifdef PASCAL /*ECN*/
#define KEY_ISO            0x80000000L
#endif
#ifdef FORTRAN
#define KEY_F66            0x10000000L
#define KEY_ONETRIP        0x20000000L
#define KEY_UPPER          0x40000000L
#define KEY_LONGLINES      0x80000000L

/* The following are duplicated from ffe/feint.h. This is a temporary bodge. */
#define EXT_DOUBLECOMPLEX           1L
#define EXT_HINTEGER                2L
#define EXT_CASEFOLD                4L
#define EXT_LCKEYWORDS              8L
#define EXT_LCIDS                0x10L
#define EXT_FREEFORMAT           0x20L
#define EXT_IMPUNDEFINED         0x40L
#define EXT_RECURSION            0x80L
#define EXT_AUTO                0x100L
#define EXT_HOLLERITH           0x200L
#define EXT_TOPEXPRESS          0x400L
#define EXT_F66                 0x800L
#define EXT_MIXEDCOMM          0x1000L
#define EXT_VMSCHARS           0x2000L
#define EXT_VMSCASTS           0x4000L
#define EXT_VMSIO              0x8000L
#define EXT_VMSTYPES          0x10000L

#define OPT_STATICLOCALS     0x100000L
#define OPT_DEFHINTEGER      0x200000L
#define OPT_DEFDOUBLE        0x400000L
#define OPT_IMPUNDEFINED     0x800000L
#define OPT_CHECKSUB        0x1000000L
#define OPT_NOARGALIAS      0x2000000L
#define OPT_LONGLINES       0x4000000L
#define FFEOPTS            0xfff00000L

#define F66_ONETRIP       1L
#define F66_IOSUBLIST     2L
#define F66_INTRINSGO     4L

#endif

/*************************************************************/
/*                                                           */
/*   Define the environment information structure            */
/*                                                           */
/*************************************************************/

#ifdef FORTRAN
#  define RISCIX_FORTRAN_PRAGMAX \
      (EXT_DOUBLECOMPLEX | EXT_HINTEGER | EXT_CASEFOLD | EXT_LCKEYWORDS |\
       EXT_FREEFORMAT | EXT_IMPUNDEFINED | EXT_RECURSION | EXT_AUTO |\
       EXT_HOLLERITH | EXT_TOPEXPRESS | EXT_F66 | EXT_MIXEDCOMM |\
       EXT_VMSCHARS | EXT_VMSCASTS | EXT_VMSIO | EXT_VMSTYPES |\
       OPT_STATICLOCALS | OPT_NOARGALIAS)
#endif

static char *driver_options[] = DRIVER_OPTIONS;

static struct EnvTable
{
      int                 unused;
      int32               initial_flags;
      int32               initial_pragmax;

      const char          *default_ansi_path,
                          *default_pcc_path,
                          *default_pas_path;
      const char          *lib_dir;
      char                *lib_root, *pas_lib_root;
      const char          *list;

      const char          *assembler_cmd;

      const char          *link_cmd;
/* 'output_file' is also used for non-link outputs.                     */
/* This unix convenience just causes pain here.                         */
      char                *output_file;
      const char          *link_ext;

      const char          *link_startup,
                          *profile_startup,
                          *profile_g_startup;

      const char          *default_lib,
                          *host_lib,
                          *profile_lib,
                          *fort_lib,
                          *fort_profile_lib,
                          *pas_lib;
}
      setupenv =
#ifdef DRIVER_ENV
    DRIVER_ENV
#else
# ifdef COMPILING_ON_ARM
#  ifdef COMPILING_ON_RISC_OS
    {
      0, (KEY_LINK), 0,
/* Perhaps $.clib, $.plib etc should be called $.lib */
      "$.clib", "$.clib", "$.plib", ".", "$.clib", "$.plib", "l",
      "ObjAsm -quit -stamp",
      "CHAIN:link", NULL, "",
      "", "", "",
      "o.ansilib", "o.hostlib", "o.ansilib", "o.fortlib", "o.fortlib", "o.plib"
    }
#  endif
#  ifdef COMPILING_ON_UNIX
#   ifdef TARGET_IS_HELIOS         /* software rust awaiting paint. */
    {
#    ifdef FORTRAN
      0, (KEY_UNIX | KEY_LINK), RISCIX_FORTRAN_PRAGMAX,
#    else
      0, (KEY_UNIX | KEY_LINK), 0,
#    endif
      "/helios/include", "/helios/include", "/", "/", "", "", "lst",
      "as",
      "ldarm", "a.out", "",
      "/hprod/ARM/lib/cstart.o", "/hprod/ARM/lib/cstart.o", "/hprod/ARM/lib/cstart.o",
      "", "", "", "", ""
    }
#   else /* TARGET_IS_HELIOS */
    {
#    ifdef FORTRAN
      0, (KEY_UNIX | KEY_LINK), RISCIX_FORTRAN_PRAGMAX,
#    else
      0, (KEY_UNIX | KEY_PCC | KEY_LINK), 0,
#    endif
#    ifdef RISCiX113
      "/usr/include/ansi",
#    else
      "/usr/include",
#    endif
      "/usr/include", "/usr/include/iso", "/", "", "", "lst",
      "as",
      "/usr/bin/ld", "a.out", "",
#    ifdef RISCiX113
      "/lib/crt0.o", "/lib/mcrt0.o", "/lib/gcrt0.o",
#    else
      "/usr/lib/crt0.o", "/usr/lib/mcrt0.o", "/usr/lib/gcrt0.o",
#    endif
      "-lc", "", "-lc_p",
#    ifdef FORTRAN_NCLIB
      "-lnfc", "-lnfc_p",
#    else
      "-lF66,-lF77,-lI77,-lU77,-lm", "-lF66_p,-lF77_p,-lI77_p,-lU77_p,-lm_p",
#    endif
     "-lpc"
    }
#   endif  /* TARGET_IS_HELIOS */
#  endif /* COMPILING_ON_UNIX */
# else /* COMPILING_ON_ARM */
#  ifdef COMPILING_ON_MSDOS /* Zortech on MSDOS */
    {
       0, (KEY_LINK), 0,
       "\\arm\\lib", "\\arm\\lib", "", "\\", "\\arm\\lib", "", "lst",
       "armasm -quit -stamp",
       "armlink", NULL, "",
       "", "", "",
       "armlib.o", "hostlib.o", "armlib.o", "", "", ""
    }
#  else
#  ifdef COMPILING_ON_MACINTOSH
/* arm_targetted cross_compilation : fortran & pascal not supported */
/* no default places (no root without knowing volume name) */
     { 0, (KEY_LINK), 0,
       "", "", "", ":", "", "", "lst",
       "armasm -quit -stamp",
       "armlink", NULL, "",
       "", "", "",
       "armlib.o", "hostlib.o", "armlib.o", "", "", ""
     }
#   else
#     ifdef TARGET_IS_ARM
/* arm_targetted cross_compilation : fortran & pascal not supported */
       { 0, KEY_LINK, 0,
         "/usr/local/lib/arm", "/usr/local/lib/arm", "", "/", "/usr/local/lib/arm", "", "lst",
         "armasm -quit -stamp",
         "armlink", NULL, "",
         "", "", "",
         "armlib.o", "hostlib.o", "armlib.o", "", "", ""
       }
#     else
#       error "No proper DRIVER_ENV information"
#     endif /* TARGET_IS_ARM */
#   endif /* COMPILING_ON_MACINTOSH */
#  endif /* COMPILING_ON_MSDOS */
# endif /* COMPILING_ON_ARM */
#endif /* DRIVER_ENV */
;

static char  *(*cc_argv)[], *(*cc_filv)[];
static const char *(*ld_argv)[], *(*ld_filv)[];
static int   cc_argc, cc_filc, ld_argc, ld_filc;
static int   cmd_error_count, main_error_count;
static int32 driver_flags;
#ifdef FORTRAN
static int32 pragmax_flags;
#endif

#ifdef TARGET_IS_KCM
#  define OBJ_EXTN "kcm"
#  define ASM_EXTN "mas"
#else
#ifdef HOST_OBJECT_INCLUDES_SOURCE_EXTN
#  define OBJ_EXTN LANG_EXTN_STRING ".o"
#  define ASM_EXTN LANG_EXTN_STRING ".s"
#  define DEFAULT_STDIN_OBJECT OBJ_EXTN
#else
#  define OBJ_EXTN "o"
#  define ASM_EXTN "s"
#endif
#endif

/*
 * Join two path names and return the result.
 */

static const char *join_path(const char *root, const char *dir,
                             const char *name)
{
  if (root[0] != '\0' && name[0] != '\0')
  {
      char *new_name = (char *)malloc(strlen(root)+strlen(dir)+strlen(name)+1);
      strcpy(new_name, root);
      strcat(new_name, dir);
      strcat(new_name, name);
      return new_name;
  }
  return name;
}

static char *join_strs(const char *s1, const char *s2)
{
  char *s = strcpy((char *)malloc(strlen(s1) + strlen(s2) + 1), s1);
  strcat(s, s2);
  return s;
}

static void new_cc_arg(char *flag)
{
  (*cc_argv)[cc_argc++] = flag;
}

static char *copy_unparse(const char *key,
                          UnparsedName *unparse,  const char *extn)
{   char *new_name;
    size_t k = strlen(key), n;
    UnparsedName u;
/* A NULL extn means use the UnparsedName as-is. A non-NULL extn means */
/* use the extn given and kill any leading path segment. In this case  */
/* we modify a local copy of the UnparsedName rather than unparse.     */
    if (extn)
    {   u = *unparse;
        u.elen = strlen(extn);
        if (u.elen == 0) extn = NULL;
        u.extn = extn;
#ifndef HOST_OBJECT_INCLUDES_SOURCE_EXTN
        u.path = NULL;
        u.plen = 0;
        u.vol  = NULL;
        u.vlen = 0;
        u.type &= ~FNAME_ROOTED;
#endif
        unparse = &u;
    }
/* Allocate space for the returned copy of the name. Allow some spare   */
/* for ^ -> .. (several times) + up to 2 extra path separators + a NUL. */
    n = k + unparse->vlen + unparse->plen + unparse->rlen + unparse->elen + 10;
    new_name = (char *)malloc(n);
    strcpy(new_name, key);
    if (fname_unparse(unparse, FNAME_AS_NAME, new_name+k, n-k) < 0)
        driver_abort("internal fault in \"copy_unparse\""); /* @@@ syserr? */
    return new_name;
}

/*
 * Get the value of an external environment variable
 */

static char *pathfromenv(char *def, char *var, char *ifnull)
{
  if (var != NULL) def = getenv(var);
  if (def == NULL)
      return ifnull;
  else {
      def = join_strs(def, "");
#ifdef COMPILING_ON_UNIX
      {   char *s = def;
          for (; *s != 0; s++) if (*s == ':') *s = ',';
      }
#endif
      return def;
  }
}

#ifndef C_ENV_VAR
#  ifdef COMPILING_ON_RISC_OS
#    define C_ENV_VAR "c$libroot"
#    define P_ENV_VAR "p$libroot"
#  else
#    if defined(COMPILING_ON_UNIX) && !defined(RELEASE_VSN)
#      define C_ENV_VAR "CPATH"
#      define P_ENV_VAR "PPATH"
#    else
#      define C_ENV_PATH NULL
#      define P_ENV_PATH NULL
#    endif
#  endif
#endif

static void get_external_environment(void)
{
  char *root = pathfromenv(NULL, C_ENV_VAR, setupenv.lib_root);
  setupenv.lib_root = root;
  if (root[0] != 0)
  {
      if (!(setupenv.initial_flags & KEY_UNIX)) {
          const char *lib = setupenv.lib_dir;
          setupenv.default_lib = join_path(root, lib, setupenv.default_lib);
          setupenv.host_lib    = join_path(root, lib, setupenv.host_lib);
          setupenv.profile_lib = join_path(root, lib, setupenv.profile_lib);
          setupenv.fort_lib    = join_path(root, lib, setupenv.fort_lib);
          setupenv.fort_profile_lib = join_path(root, lib, setupenv.fort_profile_lib);
      }
#ifndef TARGET_IS_HELIOS
      /*
       * XXX - NC - 21/8/1991
       *
       * Why oh why is this done ?????
       *
       * Why do you want to set the default include paths
       * to be the root of the file system ???
       */
      
      setupenv.default_ansi_path = setupenv.default_pcc_path = root;
#endif
  }
#ifdef PASCAL
  root = pathfromenv(root, P_ENV_VAR, setupenv.pas_lib_root);
  setupenv.pas_lib_root = root;
  if (root[0] != 0) {
      if (!(setupenv.initial_flags & KEY_UNIX)) {
          const char *pas_lib = setupenv.lib_dir;
          setupenv.pas_lib = join_path(root, pas_lib, setupenv.pas_lib);
      }
      setupenv.default_pas_path = root;
  }
#endif
}

static void set_default_options(void)
{
  char **argp, *arg;
  /* set up driver options excepting things like -D<letter>id.          */
  for (argp = driver_options, arg = *argp;  arg != NULL;  arg = *(++argp))
  {   if (arg[1] == 'D' && arg[2] != '_' &&
          (driver_flags & (KEY_PCC|KEY_STRICT)) == KEY_STRICT) continue;
      new_cc_arg(arg);
  }
}

/*************************************************************/
/*                                                           */
/*      Find a command line keyword and return flag          */
/*                                                           */
/*************************************************************/

bool cistreq(const char *s1, const char *s2) {
    for ( ; ; ) {
        int ch1 = *s1++, ch2 = *s2++;
        if (safe_tolower(ch1) != ch2) return NO;
        if (ch1 == 0) return YES;
    }
}

static int32 keyword(const char *string)
{
  int count;
  static struct { char *word; int32 key; } const keytab[] = {
      "-help",          KEY_HELP,
      "-h",             KEY_HELP,
      "-link",          KEY_LINK,
      "-list",          KEY_LISTING,
      "-errors",        KEY_ERRORSTREAM,
#ifdef TARGET_ENDIANNESS_CONFIGURABLE
      "-littleend",     KEY_LITTLEEND,
      "-li",            KEY_LITTLEEND,
      "-bigend",        KEY_BIGEND,
      "-bi",            KEY_BIGEND,
#endif
#ifdef PASCAL /*ECN*/
      "-iso",           KEY_ISO,
      "-arthur",        KEY_HOST_LIB,
      "-super",         KEY_HOST_LIB,
      "-counts",        KEY_COUNTS,
#else
#ifndef FORTRAN /* i.e. C, C++, etc */
      "-ansi",          KEY_ANSI,
      "-arthur",        KEY_HOST_LIB,
      "-pcc",           KEY_PCC,
      "-super",         KEY_HOST_LIB,
      "-fussy",         KEY_STRICT,
      "-strict",        KEY_STRICT,
      "-pedantic",      KEY_STRICT,
      "-counts",        KEY_COUNTS,
#  ifdef RISCiX113
      "-riscix1.2",     KEY_RIX120,
#  endif
#else /* FORTRAN */
#  ifndef TARGET_IS_UNIX
      "-bsd",           KEY_PCC,
#  endif
      "-onetrip",       KEY_ONETRIP,
      "-fussy",         KEY_STRICT,
      "-f66",           KEY_F66,
      "-strict",        KEY_STRICT,
      "-extend",        KEY_LONGLINES,
#endif /* FORTRAN */
#endif /* PASCAL */
    };

  for (count = 0;  count < sizeof(keytab)/sizeof(keytab[0]);  ++count)
      if (cistreq(string, keytab[count].word))
          return keytab[count].key;

  return 0L;
}

/*************************************************************/
/*                                                           */
/*      Validate the command line keywords                   */
/*                                                           */
/*************************************************************/

static void validate_flags(void)
{
  int32 flags = driver_flags;

#ifdef WANT_WHINGY_MSGS_EVEN_WHEN_WRONG
  if (ld_argc != 0 && !(flags & KEY_LINK))
      /* Beware: the next line catches most curios, but not -lxxx   */
      cc_msg("Warning: linker flag(s) ignored with -c -E -M or -S\n");
#endif

  if (flags & KEY_ANSI) flags &= ~KEY_PCC;

  if (flags & KEY_HOST_LIB)
  {   if (flags & KEY_UNIX)
      {   cc_msg("Warning: -arthur/-super ignored under unix\n");
          flags &= ~KEY_HOST_LIB;
      }
  }

  if (flags & KEY_COMMENT && !(flags & KEY_PREPROCESS)) flags &= ~KEY_COMMENT;

  if ((flags & (KEY_MAKEFILE+KEY_PREPROCESS+KEY_MD)) ==
               (KEY_MAKEFILE+KEY_PREPROCESS))
  {
      cc_msg("Warning: options -E and -M conflict: -E assumed\n");
      flags &= ~KEY_MAKEFILE;
  }

  if (flags & KEY_COUNTS) flags |= KEY_LISTING;

  if (flags & KEY_PROFILE && flags & KEY_XPROFILE) flags &= ~KEY_PROFILE;

#ifdef FORTRAN
  if (flags & KEY_STRICT)
  {   if (flags & KEY_ONETRIP)
      {   cc_msg("Warning: -onetrip and -strict conflict: -strict assumed\n");
          flags &= ~KEY_ONETRIP;
      }
      if (flags & KEY_F66)
      {   cc_msg("Warning: -f66 and -strict conflict: -strict assumed\n");
          flags &= ~KEY_F66;
      }
      if (flags & KEY_LONGLINES)
      {   cc_msg("Warning: -extend and -strict conflict: -strict assumed\n");
          flags &= ~KEY_LONGLINES;
      }
  }
  if ((flags & KEY_F66) && (flags & KEY_ONETRIP))
  {   cc_msg("Warning: -f66 implies -onetrip\n");
      flags &= ~KEY_ONETRIP;
  }
#endif

  driver_flags = flags;
}

static void give_help(const char *command_name)
{
#ifdef HOST_WANTS_NO_BANNER
    fprintf(stdout, "%s\n", CC_BANNER);
#endif
    {   char **p = driver_help_text;
        fprintf(stdout, *p, command_name);
        while (*++p != NULL) fprintf(stdout, "%s\n", *p);
    }
}

/*************************************************************/
/*                                                           */
/*      Set command line options for current flags           */
/*                                                           */
/*************************************************************/

#ifdef FORTRAN
static char pragmax[16];
static char pragmaw[16];
#endif

static void set_flag_options(void)
{
  int32 flags = driver_flags;
#ifdef FORTRAN
  int32 pragmaw_flags = 0;
  if (flags & KEY_ONETRIP) pragmaw_flags |= F66_ONETRIP;
  if (flags & KEY_F66)
      pragmaw_flags |= (F66_ONETRIP + F66_IOSUBLIST + F66_INTRINSGO);
  if (flags & KEY_LONGLINES) pragmax_flags |= OPT_LONGLINES;
  if (flags & KEY_STRICT) pragmax_flags = 0;
  sprintf(pragmaw, "-ZPW%-lu", pragmaw_flags);
  sprintf(pragmax, "-ZPX%-lu", pragmax_flags);
  new_cc_arg(pragmaw);
  new_cc_arg(pragmax);
#endif

  if (flags & KEY_STRICT)       new_cc_arg("-ZF");
  if (flags & KEY_MAKEFILE)
      new_cc_arg(flags & KEY_NOSYSINCLUDES ? "-M<" : "-M");
  if (flags & KEY_PCC)          new_cc_arg("-ZU");
#ifdef PASCAL /*ECN*/
  if (flags & KEY_ISO)          new_cc_arg("-ZZ");
#endif
  if (flags & KEY_PREPROCESS &&
      flags & KEY_COMMENT)      new_cc_arg("-C");
#ifdef TARGET_ENDIANNESS_CONFIGURABLE
  if (flags & KEY_LITTLEEND)    new_cc_arg("-ZE1");
  if (flags & KEY_BIGEND)       new_cc_arg("-ZE0");
#endif
#ifdef TARGET_IS_UNIX
#  ifdef COMPILING_ON_ACORN_KIT
  /* Horrid bodge to remove -ZS<system> and -ZI<file> if -strict... */
  /* AM: why?  Maybe of more general use?                           */
  if (flags & KEY_STRICT)
  {   int f, t;
      for (f = t = 1;  f < cc_argc;  ++f)
      {   char *arg = (*cc_argv)[f];
          if (arg[1] != 'Z' || arg[2] != 'I' && arg[2] != 'S')
              (*cc_argv)[t++] = arg;
      }
      cc_argc = t;
  }
#  endif
#endif
}

/*************************************************************/
/*                                                           */
/*     Add an option to a link or assembler command.         */
/*                                                           */
/*************************************************************/

static int cmd_cat(char *cmd, int posn, const char *extra)
{
  int l = strlen(extra);
  if (posn != 0 && l != 0)
  {
      if (cmd != NULL) cmd[posn] = ' ';
      ++posn;
  }
  if (cmd != NULL && l != 0) strcpy(cmd + posn, extra);
  return posn + l;
}

/*************************************************************/
/*                                                           */
/*      Call the assembler to assemble a '.s' file           */
/*                                                           */
/*************************************************************/

#ifndef HOST_CANNOT_INVOKE_ASSEMBLER

#ifndef target_asm_options_
#  define target_asm_options_(x) ""
#endif

static int assembler(const char *asm_file, const char *obj_file)
{
  int32 flags = driver_flags;
  char *cmd;
  char small_cmd[SMALL_COMMAND];

#ifndef NO_CONFIG
  config_init();
#endif
  cmd = NULL;
  for (;;)
  {   /* once round to count the length and once to copy the strings... */
      int cmdlen = 0;
      int32 endian;
      cmdlen = cmd_cat(cmd, cmdlen, setupenv.assembler_cmd);
      endian =
#ifdef TARGET_ENDIANNESS_CONFIGURABLE
         driver_flags & KEY_LITTLEEND ? CONFIG_ENDIANNESS_SET :
         driver_flags & KEY_BIGEND ? CONFIG_ENDIANNESS_SET+CONFIG_BIG_ENDIAN :
#endif
         0;
      cmdlen = cmd_cat(cmd, cmdlen, target_asm_options_(endian));
      if (flags & KEY_READONLY) cmdlen = cmd_cat(cmd, cmdlen, " -R");
      if (!(flags & KEY_UNIX))  cmdlen = cmd_cat(cmd, cmdlen, asm_file);
      if (flags & KEY_UNIX)     cmdlen = cmd_cat(cmd, cmdlen, "-o");
      cmdlen = cmd_cat(cmd, cmdlen, obj_file);
      if (flags & KEY_UNIX)     cmdlen = cmd_cat(cmd, cmdlen, asm_file);
      if (cmd != NULL) break;
      if (cmdlen < SMALL_COMMAND)
          cmd = small_cmd;
      else
          cmd = (char *)malloc(cmdlen+1);
  }
#ifdef TESTING
  cc_msg("%s\n", cmd);
#endif
  return system(cmd);
}

#endif

/*************************************************************/
/*                                                           */
/*      Link compiled files together                         */
/*                                                           */
/*************************************************************/

#ifndef target_lib_name_
#  define target_lib_name_(x,e) x
#endif

#ifndef HOST_CANNOT_INVOKE_LINKER

static void linker(int32 flags)
{
#ifndef TARGET_IS_NULL          /* Hmmm, but, but ... */
  int count;
  const char *startup;
#ifdef TARGET_IS_ARM
  int32 endian =
#ifdef TARGET_ENDIANNESS_CONFIGURABLE
         driver_flags & KEY_LITTLEEND ? CONFIG_ENDIANNESS_SET :
         driver_flags & KEY_BIGEND ? CONFIG_ENDIANNESS_SET+CONFIG_BIG_ENDIAN :
#endif
         0;
#endif
  

#ifndef NO_CONFIG
  config_init();
#endif
  
  switch (flags & (KEY_PROFILE | KEY_XPROFILE))
  {
case KEY_PROFILE:   startup = setupenv.profile_startup;    break;
case KEY_XPROFILE:  startup = setupenv.profile_g_startup;  break;
default:            startup = setupenv.link_startup;       break;
  }

#ifndef LINKER_IS_SUBPROGRAM
  { char *cmd = NULL;
    int cmdlen;
    char small_cmd[SMALL_COMMAND];

    for (;;)
    {   /* once round to count the length and once to copy the strings... */
        cmdlen = 0;
        cmdlen = cmd_cat(cmd, cmdlen, setupenv.link_cmd);

        for (count = 0;  count < ld_argc;  ++count)
            cmdlen = cmd_cat(cmd, cmdlen, (*ld_argv)[count]);

        cmdlen = cmd_cat(cmd, cmdlen, "-o");
        cmdlen = cmd_cat(cmd, cmdlen, setupenv.output_file);
        cmdlen = cmd_cat(cmd, cmdlen, startup);

        for (count = 0;  count < ld_filc;  ++count)
            cmdlen = cmd_cat(cmd, cmdlen, (*ld_filv)[count]);

        count = cmdlen;

        if (flags & KEY_HOST_LIB)
            cmdlen = cmd_cat(cmd, cmdlen, target_lib_name_(setupenv.host_lib, endian));

#ifdef PASCAL
        cmdlen = cmd_cat(cmd, cmdlen, target_lib_name_(setupenv.pas_lib, endian));
#endif
#ifdef FORTRAN
        cmdlen = cmd_cat(cmd, cmdlen, target_lib_name_(setupenv.fort_lib, endian));
#endif
        cmdlen = cmd_cat(cmd, cmdlen, target_lib_name_(setupenv.default_lib, endian));

        if (cmd != NULL) break;
        if (cmdlen < SMALL_COMMAND)
            cmd = small_cmd;
        else
            cmd = (char *)malloc(cmdlen+1);
    }
    while (count < cmdlen)
    { /* space-separate, rather than comma-join, the library list */
      if (cmd[count] == ',') cmd[count] = ' ';  ++count;
    }

#ifdef TESTING
    cc_msg("%s\n", cmd);
#endif

    count = system(cmd);
  }
#else /* LINKER_IS_SUBPROGRAM */
  { const char **argv = (const char **)malloc((ld_argc+ld_filc+8) * sizeof(char **));
    extern int do_link(int argc, const char **argv);

    argv[0] = setupenv.link_cmd;
    memcpy(&argv[1], *ld_argv, ld_argc * sizeof(char **));
    count = ld_argc+1;
    argv[count++] = "-o";
    argv[count++] = setupenv.output_file;
    if (*startup != 0) argv[count++] = startup;
    memcpy(&argv[count], *ld_filv, ld_filc * sizeof(char **));
    count += ld_filc;
    if (flags & KEY_HOST_LIB)
      argv[count++] = join_strs(target_lib_name_(setupenv.host_lib, endian), "");

#ifdef PASCAL
    argv[count++] = join_strs(target_lib_name_(setupenv.pas_lib, endian), "");
#endif
#ifdef FORTRAN
    argv[count++] = join_strs(target_lib_name_(setupenv.fort_lib, endian), "");
#endif
    argv[count++] = target_lib_name_(setupenv.default_lib, endian);
    argv[count] = 0;
    count = do_link(count, argv);
  }
#endif /* LINKER_IS_SUBPROGRAM */
  
  if (count != 0) ++main_error_count;

  /*
   * In PCC mode delete the '.o' file if only one file was compiled.
   * NB. (count==0) is used to check the link was ok.
   */
  if (
#ifndef PASCAL
      (flags & KEY_PCC) &&
#endif
      (cc_filc == 1) && (ld_filc == 1) && (count==0))
      remove((*ld_filv)[0]);
#else  /* not TARGET_IS_NULL */
  flags = flags;  /* keep ncc happy */
#endif /* not TARGET_IS_NULL */
}
#endif /* not HOST_CANNOT_INVOKE_LINKER */

/*
 * Process input file names.
 */

static void process_file_names(int filc, char *filv[])
{
  int count, saved_cc_argc;
  int32 flags = driver_flags;
  UnparsedName unparse;

  /*
   * Reset cc_filc here - we use it to count the actual number of .c files
   * (so linker() can delete the intermediate object file in the case that
   * there is exactly 1 C file), rather than the number of files to be
   * processed by this function.
   */
  cc_filc = 0;
  saved_cc_argc = cc_argc;

  for (count = 0; count < filc;  ++count)
  {   char *current = filv[count];
      int extn_ch;

      if (strlen(current) > MAX_TEXT-5)
      {   cc_msg("Overlong filename ignored: %s\n", current);
          continue;
      }

      cc_argc = saved_cc_argc;
      fname_parse(current, FNAME_SUFFIXES, &unparse);

      if (unparse.extn == NULL)
#ifndef COMPILING_ON_UNIX
      /* On non-Unix hosts use a sensible default if no extension was given */
      {   unparse.extn = LANG_EXTN_STRING;
          unparse.elen = sizeof(LANG_EXTN_STRING)-1;
      }
      extn_ch = unparse.extn[0];
#else
          extn_ch = 0;
      else
          extn_ch = unparse.extn[0];
#endif

      switch(extn_ch)
      {
#ifndef HOST_CANNOT_INVOKE_LINKER
#  ifdef COMPILING_ON_UNIX
case 'a':
#  else
case 'O':
#  endif
case 'o':     if ((flags & (KEY_PREPROCESS+KEY_MAKEFILE)) == 0)
              {
                  (*ld_filv)[ld_filc++] = copy_unparse("", &unparse, NULL);
                  break;
              }
              else cc_msg("Warning: %s -E/M %s - inconsistent options\n",
                           (*cc_argv)[0], current);
              /* and fall through ... */
#endif

default:      if ((flags & (KEY_PREPROCESS+KEY_MAKEFILE)) == 0)
              {
                  cc_msg("Error: type of '%s' unknown (file ignored)\n",
                          current);
                  ++cmd_error_count;
                  continue;
              }
              /* fall through again (-E, -M) */

#ifndef HOST_CANNOT_INVOKE_ASSEMBLER
#  ifndef COMPILING_ON_UNIX
case 'S':
#  endif
case 's':     if ((flags & (KEY_PREPROCESS+KEY_MAKEFILE)) == 0)
              {   const char *asm_file = copy_unparse("", &unparse, NULL);
                  const char *obj_file = copy_unparse("", &unparse, OBJ_EXTN);
                  if (assembler(asm_file, obj_file) != 0)
                  {   main_error_count++;
                      remove(obj_file);
                  }
                  /* and fall through... foo.s... */
              }
              /* fall through again (-E, -M, foo.s) */
#endif

#ifdef COMPILING_ON_UNIX
case 'i':     /* for X/Open compliance */
#else
case LANG_UC_EXTN:
#endif
case LANG_EXTN:
          {   char *out_file = setupenv.output_file;
              char *out_name = NULL;
              if (flags & KEY_ASM_OUT)
              {
                  if (flags & KEY_RENAME && filc == 1)
                      /* Assert: KEY_RENAME => setupenv.output_file != NULL */
                      new_cc_arg(join_strs("-S", out_name = out_file));
                  else
                  {   out_name = copy_unparse("-S", &unparse, ASM_EXTN);
                      new_cc_arg(out_name);
                      out_name += 2;                       /* skip the -S */
                  }
                  new_cc_arg("-C");             /* suppress object output */
              }

              if (flags & KEY_LISTING)
                  new_cc_arg(copy_unparse("-L", &unparse, setupenv.list));
/*
 * Maybe the -counts option should take a file name.  For the moment
 * it just uses a fixed one...
 */
              if (flags & KEY_COUNTS) new_cc_arg("-K");

              if (flags & KEY_MD)
                  new_cc_arg(copy_unparse("-M", &unparse, "d"));

              new_cc_arg(copy_unparse("", &unparse, NULL));

              if (out_file == NULL                  ||
                   filc != 1                        ||
                   flags & (KEY_LINK)               ||
                  (flags & KEY_RENAME) == 0)
              {   /*
                   * No-default-output-file ||
                   * more-than-1-file || going-to-link || no -o <file>
                   */
                  out_file = copy_unparse("", &unparse, OBJ_EXTN);
              }
                  /* else...
                   * -o && no-link && 1-file && default-op-file
                   */
              new_cc_arg(out_file);
              if (out_name == NULL) out_name = out_file;

              if ((flags & (KEY_MAKEFILE+KEY_PREPROCESS)) == 0)
              {
                  (*ld_filv)[ld_filc++] = out_file;
                  if (safe_tolower(unparse.extn[0]) == 's')
                     /* already called the assembler so... */ break;
              }

#ifdef TESTING
{   int j;
    for (j = 0;  j < cc_argc;  ++j) cc_msg("%s\n", (*cc_argv)[j]);
}
#endif
              (*cc_argv)[cc_argc] = NULL;
              if (ccom(cc_argc, *cc_argv))
              {   ++main_error_count;
#ifdef COMPILING_ON_RISC_OS
/* The next line is dirty and should be done by checking return code,   */
/* not peeking at other's variables.                                    */
                  if (errorcount)  /* only delete o/p file if serious errors */
#endif
                      remove(out_name);
              }
          }
          /* and for the benefit of linker(), count the sources */
          ++cc_filc;
          break;
      }

      /*
       * If no output file has been given, derive one from the 1st file name.
       */
      if (setupenv.output_file == NULL && flags & KEY_LINK)
          setupenv.output_file = copy_unparse("", &unparse, setupenv.link_ext);
  }
}

/*
 * Extract compilation options from the command line.
 */

static void bad_option(const char *opt)
{
  cc_msg("Error: bad option '%s': ignored\n", opt);
  ++cmd_error_count;
}

#ifdef FORTRAN
static void nimp_option(const char *opt)
{
  cc_msg("Error: unimplemented option '%s': ignored\n", opt);
  ++cmd_error_count;
}
#endif

static char *new_include(const char *file, char *list)
{
  unsigned len, l, lim;

  if (list == NULL)
  {
      len = 0;
      list = (char *)malloc(INCLUDE_BUFLEN);
  }
  else
  {
      len = strlen(list);
      lim = INCLUDE_BUFLEN;
      while (lim <= len) lim *= 2;    /* -> 2**k */
      l = len + strlen(file) + 2;     /* + ',' + NUL */
      if (l > lim)
      {
          while (l > lim) lim *= 2;
          list = (char *)realloc(list, lim);
      }
      list[len++] = ',';
  }
  strcpy(list + len, file);
  return list;
}

static void read_keyword_options(int argc, char *argv[])
{
  int count;
  for (count=1; count < argc;  ++count)
  {
      int32 key = keyword(argv[count]);
      if (key == 0L)
      {   int n = count;
          KW_Status status = mcdep_keyword(argv[count], &n, argv);
          if (status != KW_OK && status != KW_NONE) {
              int c = '\'';
              cc_msg("Error: bad option ");
              while (count <= n) {
                  cc_msg("%c%s", c, argv[count]);
                  argv[count++] = NULL;
                  c = ' ';
              }
              count--;
              cc_msg("': ignored\n");
              ++cmd_error_count;
          } else
              count = n;
      } else {
          char *keyword = argv[count];
          argv[count] = NULL;
#if defined(FORTRAN) && !defined(TARGET_IS_UNIX)
          if (key == KEY_PCC)
              pragmax_flags = RISCIX_FORTRAN_PRAGMAX;
          else
#endif
          if (key == KEY_ERRORSTREAM) {
              if (++count >= argc) {
                  cc_msg("Missing file argument for %s\n", keyword);
                  exit(1);
              } else {
                  FILE *e = fopen(argv[count], "w");
                  if (e == NULL) {
                      cc_msg("Can't open %s for output\n", argv[count]);
                      exit(1);
                  } else {
                      fclose(e);
                      freopen(argv[count], "w", stderr);
                  }
                  argv[count] = NULL;
              }
          } else
              driver_flags |= key;
      }
  }
#ifdef PASCAL /*ECN*/
  driver_flags |= KEY_ANSI;
#endif
}

#ifdef FORTRAN
#  define FortranUnimplementedOption(ch,current)\
    if (current[1] == ch) { nimp_option(current); break; }
#else
#  define FortranUnimplementedOption(ch,current)
#endif

static void read_compile_options(int argc, char *argv[])
{
  int count;
  int32 flags;
  char *include_path = new_include("-", NULL),
       *system_path = new_include("-", NULL),
       *library_path = NULL;

  flags = driver_flags;
  for (count=1; count < argc;  ++count)
  {
      char *current = argv[count];
      char *next;

      if (current == NULL) continue;        /* already processed */

      if (current[0] == '-')
      {   int uc_opt = current[1];
          uc_opt = safe_toupper(uc_opt);
          switch(uc_opt)
          {
#ifdef FORTRAN
      case 'F': if (current[1] == 'f') break;
/* @@@ LDS 10Aug89 - DO NOT CATCH 'O' HERE - IT BREAKS Unix Makefiles */
      case 'M': if (current[1] == 'M') break;
      case 'U':
      case 'V':
#else
#ifndef DISABLE_ERRORS
      case 'E': if (current[2] == 'S' && current[3] == 0) break;
#endif
      case 'O': if (current[1] == 'o') break;
#ifndef PASCAL /*ECN*/
      case 'R':
#endif
#endif
      case 'C':
      case 'S':
                if (current[2] == 0) break;
                bad_option(current);
          }

          switch(uc_opt)
          {
#ifdef FORTRAN
  case '1':   flags |= KEY_ONETRIP;
              break;

  case 'C':   if (current[1] == 'c') flags &= ~KEY_LINK;
              else pragmax_flags |= OPT_CHECKSUB;
              break;

  case 'I':   if ((current[1] == 'i') && (current[2] == '2'))
                  pragmax_flags |= OPT_DEFHINTEGER;
              else goto may_take_next_arg;
              break;

  case 'N':   break;

  case 'O':   if (current[1] == 'o') goto may_take_next_arg;
              if (current[2] != 0) /* -O has no effect currently */
              {
                  int n = (current[2] - '0');
                  if ((current[3] != 0) || (n < 0) || (n > MINO_MAX))
                      bad_option(current);
                  else
                  {
                      new_cc_arg((n & MINO_CSE) ? "-ZPZ1" : "-ZPZ0");
                      if (n & MINO_NAO) pragmax_flags |= OPT_NOARGALIAS;
                      else pragmax_flags &= ~OPT_NOARGALIAS;
                  }
              }
              break;

  case 'R':   if (current[1] == 'R') nimp_option(current);
              else {
                if (current[2] == '8')
                  pragmax_flags |= OPT_DEFDOUBLE;
                else {
                  new_cc_arg("-R");
                  if (flags & KEY_UNIX) flags |= KEY_READONLY;
                }
              } break;

  case 'U':   if (current[1] == 'U') pragmax_flags &= ~EXT_CASEFOLD;
              else pragmax_flags |= OPT_IMPUNDEFINED;
              break;

  case 'V':   if (current[1] == 'v') {
                 new_cc_arg("-FB");
                 cc_msg("%s\n",CC_BANNER);
              }
              else goto link_command;
              break;

#else /* FORTRAN */
  case 'C':   if (current[1] == 'c')
                  flags &= ~KEY_LINK;
              else
                  flags |= KEY_COMMENT;
              break;

  case 'I':
  case 'J':   goto may_take_next_arg;

  case 'O':   if (current[1] == 'o') goto may_take_next_arg;
              new_cc_arg("-ZPZ1");
              break;

#endif /* FORTRAN */

  case '\0':  flags |= KEY_STDIN;    /* '-' on its own denotes stdin... */
              break;

  case 'E':   FortranUnimplementedOption('E', current);
#ifdef DISABLE_ERRORS
              /* provide support for -Exyz to suppress certain errors.  */
              if (current[2] == 0)
#else
#  ifdef COMPILING_ON_UNIX
              if (current[1] == 'e')
                  goto may_take_next_arg;      /* X/Open -e epsym to ld */
              else
#  endif
#endif
                  flags = (flags | KEY_PREPROCESS) & ~KEY_LINK;
              new_cc_arg(current);           break;

  case 'F':   FortranUnimplementedOption('F', current);
              new_cc_arg(current);           break;

  case 'G':   new_cc_arg(current);
              if (flags & KEY_UNIX)
                  library_path = new_include("-lg", library_path);
              else
                  (*ld_argv)[ld_argc++] = "-d";
              break;

  case 'L':   if (flags & KEY_UNIX)
              {
                  if (current[1] == 'l')
                      library_path = new_include(current, library_path);
                  else
                      (*ld_argv)[ld_argc++] = current;
              }
              else goto may_take_next_arg;
              break;

  case 'M':   FortranUnimplementedOption('m', current);
#ifdef COMPILING_ON_UNIX
              if (current[1] == 'm')            /* request link map     */
                  (*ld_argv)[ld_argc++] = current;
              else
#endif
              switch(safe_toupper(current[2]))
              {
      case 'X':   if (current[3] != 0) goto defolt;
                  flags |= KEY_NOSYSINCLUDES;
                  /* fall through... */
      case '\0':  flags = (flags | KEY_MAKEFILE) & ~KEY_LINK;
                  break;
      case 'D':   if (current[3] == '\0')
                  {   flags |= KEY_MD;
                      break;
                  }
      default:
      defolt:     bad_option(current);
              }
              break;

  case 'P':   uc_opt = current[2];
              uc_opt = safe_toupper(uc_opt);
              switch(uc_opt)
              {
      case '\0':  flags |= KEY_PROFILE;
                  break;
      case 'G':
      case 'X':   if (current[3] == '\0')
                  {   flags |= KEY_XPROFILE;
                      break;
                  }
      default:    bad_option(current);
                  continue;
              }
              new_cc_arg(current);
              if (setupenv.profile_lib[0] != '\0') {
                  setupenv.default_lib = setupenv.profile_lib;
                  setupenv.fort_lib = setupenv.fort_profile_lib;
              }
              break;

#ifndef FORTRAN
  case 'R':
#ifdef COMPILING_ON_UNIX
              if (current[1] == 'r')
                  (*ld_argv)[ld_argc++] = "-r";    /* X/Open compliance */
              else
#endif
              {   new_cc_arg(current);
                  if (flags & KEY_UNIX) flags |= KEY_READONLY;
              }
              break;
#endif

  case 'S':
#ifdef COMPILING_ON_UNIX
              if (current[1] == 's')
                  (*ld_argv)[ld_argc++] = "-s";
              else
#endif
                  flags = (flags & ~KEY_LINK) | KEY_ASM_OUT;
              break;

#ifdef TARGET_IS_XPUTER
  case 'T':
      new_cc_arg(current);
      break;
#endif

  case 'W':   new_cc_arg(current);
              break;

  case 'Z':   uc_opt = current[2];
              uc_opt = safe_toupper(uc_opt);
              switch(uc_opt)
              {
      case '\0':  /* Pass on '-z' to the linker */
                  goto link_command;

#ifdef TARGET_IS_HELIOS         /* the miserable -zr helios option */
      case 'L':                 /* -zl for replacement for -zr */
      case 'R':   flags &= ~KEY_LINK;
      case 'S':                 /* -zs for split module tables */
      case 'D':                 /* extra addressability mode */
                  /* drop through */
#endif
      default:    /* leave checking for errors here to compiler.c */
                  new_cc_arg(current);
                  break;

      case 'I':   goto may_take_next_arg;
              }
              break;

  link_command:
  default:
#ifndef HOST_CANNOT_INVOKE_LINKER
              (*ld_argv)[ld_argc++] = current;  break;
#else
              bad_option(current); break;
#endif

#ifndef FORTRAN
  case 'U':
#endif
  case 'D':
  may_take_next_arg:
              uc_opt = current[1];
              uc_opt = safe_toupper(uc_opt);
              if (uc_opt == 'Z')
              {   uc_opt = current[2];
                  uc_opt = safe_toupper(uc_opt);
                  if (uc_opt == 'I') uc_opt = 'S'; else uc_opt = 'Z';
              }
              if (current[2] == 0 || uc_opt == 'S' ||
                  current[3] == 0 && uc_opt == 'Z')
              {
                  if (++count < argc)
                      next = argv[count];
                  else
                  {
                      driver_abort("No argument to last compiler option");
                      return;
                  }
              }
              else if (uc_opt == 'Z')
                  next = current + 3;
              else
                  next = current + 2;
              switch (uc_opt)
              {
#ifdef COMPILING_ON_UNIX
      case 'E':            /* actually can only be "-e" here... X/Open */
#endif
      case 'U':
#ifdef COMPILING_ON_UNIX
                  if (current[1] != uc_opt)  /* e or u */
                  {   next = join_strs("-e ", next);
                      next[1] = current[1];
                      (*ld_argv)[ld_argc++] = next;           /* X/Open */
                      break;
                  } /* 'E' and 'U' fall through */
#endif
      case 'D':   if (current[2] == 0) current = join_strs(current, next);
                  new_cc_arg(current);
                  break;
      case 'I':   include_path = new_include(next, include_path);
                  break;
      case 'J':   system_path  = new_include(next, system_path);
                  break;
      case 'L':   library_path = new_include(next, library_path);
                  break;
      case 'O':   flags |= KEY_RENAME;
                  {   UnparsedName unparse;
                      fname_parse(next, FNAME_SUFFIXES, &unparse);
                      setupenv.output_file = copy_unparse("", &unparse, NULL);
                  }
                  break;
      case 'S':   /* -ZI<system> file */
                  new_cc_arg(join_strs("-ZS", current+3));
                  new_cc_arg(join_strs("-ZI", next));
                  break;
      case 'Z':   if (current[3] == 0) current = join_strs(current, next);
                  new_cc_arg(current);
                  break;
              }
          }
      }
      else (*cc_filv)[cc_filc++] = current;
  }

  driver_flags = flags;

#ifdef RISCiX113
  if (!(flags & KEY_RIX120))
      new_cc_arg("-D__type=___type");
#endif

  /*
   * Add '-I' and '-J' to the command line for compiler.c
   */
  if (system_path[1] == ',')
  {
      system_path[1] = 'J';
      if (include_path[1] == ',')
      {
          include_path[1] = 'I';
          new_cc_arg(include_path);
      }
      new_cc_arg(system_path);
  }
  else
  {
      /*
       * No -J so add default system path to -I list
       */
#ifdef PASCAL
      const char *path = setupenv.default_pas_path;
#else
# ifdef TARGET_IS_HELIOS
      /*
       * XXX - NC - 21/8/1991
       *
       * I do not understand the original code here.  Why should the
       * ansi path only be used if the ANSI flag is defined, and the
       * backward compatability flag is set ???
       */

      const char * path;


      if (flags & KEY_PCC)
	{
          path = setupenv.default_pcc_path;
	}
      else
	{
          path = setupenv.default_ansi_path;
	}
      
# else /* ! TARGET_IS_HELIOS */
      const char *path = setupenv.default_pcc_path;
#  ifdef RISCiX113
      if (((flags & KEY_ANSI) || !(flags & (KEY_PCC))) &&
          !(flags & KEY_RIX120))
          path = setupenv.default_ansi_path;
#  endif
# endif /* TARGET_IS_HELIOS */
#endif /* PASCAL */

#ifdef TARGET_IS_HELIOS			/* XXX - NC */
      include_path[1] = 'I';
      new_cc_arg(include_path);
      system_path = new_include(path, system_path);
      system_path[1] = 'J';
      new_cc_arg(system_path);
#else
      include_path = new_include(path, include_path);
      include_path[1] = 'I';
      new_cc_arg(include_path);
#endif
  }

  /*
   * If compiling on Unix and in Ansi mode add extra libraries, then
   * if 'library_path' is not empty assign it to 'default_lib'.
   */
#ifndef TARGET_IS_HELIOS
/*
 * ACN: Oh misery - if I am compiling and linking under Unix, but to generate
 * Helios binaries, I do not want to scan the extra libraries indicated
 * here.  I think the problem here is mostly because TARGET_IS_HELIOS
 * improperly sets the KEY_UNIX bit in flags - but to invent a KEY_HELIOS
 * would also be some work since elsewhere in this code there seems
 * to be a dichotomy assumed between Unix and Risc-OS...
 * AM: except for this file the alternative of MVS should be possible!
 * Thus the conditional
 * compilation here seems (in the short term) my easiest way out.
 */
#  ifdef RISCiX113
  if (((flags & KEY_ANSI) || !(flags & (KEY_PCC))) &&
      !(flags & KEY_RIX120))
  {
      library_path = new_include("-lansi,-lm", library_path);
  }
#  endif
#endif

  if (library_path != NULL)
  {
      if (flags & KEY_UNIX)
          library_path = new_include(setupenv.default_lib, library_path);
      setupenv.default_lib = library_path;
  }
}

/*
 * Main.
 */

int main(int argc, char *argv[])
{
  int count;
  bool is_cpp = NO;
  char *progname;
#if (defined(COMPILING_ON_UNIX) && defined(FORTRAN))
  progname = "f77"; /* rather than the "f77comp" we would get from argv[0] */
#else
  char p[32];
  progname = program_name(argv[0], p, 32);
#endif

  main_error_count = cmd_error_count = 0;

#ifdef CHECK_AUTHORIZED
  check_authorized();
#endif

  errstate_init();   /* needed before cc_msg */

#ifndef HOST_WANTS_NO_BANNER
    cc_msg("%s\n", CC_BANNER);
#endif
#ifndef COMPILING_ON_UNIX
#ifndef COMPILING_ON_MSDOS
  (void) signal(SIGINT, compile_abort);
#endif
#else
#  ifdef DRIVER_PRE_RELEASE_MSG
     cc_msg("%s\n",DRIVER_PRE_RELEASE_MSG);
#  endif
  /* The signal ignore state can be inherited from the parent... */
#ifndef COMPILING_ON_MSDOS
#ifdef __STDC__
#  define sig_ign ((void (*)(int))(SIG_IGN))
#else
#  define sig_ign SIG_IGN
#endif
  if (signal(SIGINT,  sig_ign) != sig_ign)
    (void) signal(SIGINT, compile_abort);
#ifdef SIGHUP
  if (signal(SIGHUP,  sig_ign) != sig_ign)
    (void) signal(SIGINT, compile_abort);
#endif
  if (signal(SIGTERM, sig_ign) != sig_ign)
    (void) signal(SIGINT, compile_abort);
#endif
#endif

  get_external_environment();
  driver_flags = setupenv.initial_flags;
#ifdef FORTRAN
  pragmax_flags = setupenv.initial_pragmax;
#endif

  cc_argv = (char *(*)[]) malloc(sizeof(char *) *
                          (argc + sizeof(driver_options)/sizeof(char *) + 3));
  cc_filv = (char *(*)[]) malloc(sizeof(char *) * argc);
  ld_argv = (const char *(*)[]) malloc(sizeof(const char *) * argc);
  ld_filv = (const char *(*)[]) malloc(sizeof(const char *) * argc);
  cc_argc = cc_filc = ld_argc = ld_filc = 0;

  new_cc_arg(argv[0]);

  count = strlen(argv[0]);
  if (count >= 3 && strcmp(&argv[0][count-3], "cpp") == 0)
  {    /* The compiler was called as '...cpp' - treat as 'cc -E'.  */
       driver_flags = (driver_flags | KEY_PREPROCESS) & ~KEY_LINK;
       new_cc_arg("-E");
       is_cpp = YES;
  }

  if (argc == 1) {
  /* used with no argument */
      give_help(progname);
      exit(1);
  }

  read_keyword_options(argc, argv);
  set_default_options();
  read_compile_options(argc, argv);

  validate_flags();
  if (driver_flags & KEY_HELP)
  {   give_help(progname);
      exit(0);
  }

  set_flag_options();

  if (is_cpp)
  {   if (cc_filc >= 2)
      {   if (cc_filc > 2)
              cc_msg("More than 2 file arguments to cpp ignored\n");
          if (freopen((*cc_filv)[1], "w", stdout) == NULL)
          {   cc_msg("Can't open output file %s\n", (*cc_filv)[1]);
              exit(EXIT_error);
          }
          cc_filc = 1;
      }
  }

  if (cc_filc > 0)
  {
      if (driver_flags & KEY_STDIN)
          cc_msg("stdin ('-') combined with other files -- ignored\n");
      process_file_names(cc_filc, *cc_filv);
  }
  else if (is_cpp ||
           ((driver_flags & KEY_STDIN)
#ifdef DEFAULT_STDIN_OBJECT
            && ((driver_flags & (KEY_PREPROCESS|KEY_MAKEFILE|KEY_ASM_OUT))
                || setupenv.output_file != NULL)
#endif
          ))
  {   char *output_file = "-";
/* was: output_file = setupenv.output_file; but writes asm to a.out     */
/* we need to separate the differing uses of "output_file".             */
#ifdef DEFAULT_STDIN_OBJECT
      output_file = setupenv.output_file;
      if (!(driver_flags & (KEY_PREPROCESS|KEY_MAKEFILE|KEY_ASM_OUT)) &&
          setupenv.output_file == NULL)
          output_file = DEFAULT_STDIN_OBJECT;
#endif
      new_cc_arg("-");
      if (output_file != NULL) {
          if (driver_flags & KEY_ASM_OUT)
              new_cc_arg(join_strs("-S", output_file));
          else
              new_cc_arg(output_file);
      }
      (*cc_argv)[cc_argc] = 0;
      if (ccom(cc_argc, *cc_argv)) ++main_error_count;
  }
  else cc_msg("Warning: %s command with no effect\n", progname);

#ifndef HOST_CANNOT_INVOKE_LINKER
  if ((main_error_count == 0) && (driver_flags & KEY_LINK) && (ld_filc > 0))
      linker(driver_flags);
#endif

  /*
   * The SUN ignores the return value from main so exit() instead
   */
  if (main_error_count + cmd_error_count > 0) exit(EXIT_error);
  exit(0);

  return 0;
}

/* End of driver.c */
