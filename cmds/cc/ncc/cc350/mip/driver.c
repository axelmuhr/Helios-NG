/*
 * driver.c - driver for NorCroft compiler (Arthur,Msdos,Unix)
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/11/23 16:41:52 $
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

#ifndef NO_VERSION_STRINGS
extern char driver_version[];
char driver_version[] = "\ndriver.c $Revision: 1.1 $ 176\n";
#endif

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

#include <stddef.h>
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

#include "globals.h"
#include "compiler.h"
#include "fname.h"
#include "version.h"
#include "drivhelp.h"

#ifdef TARGET_IS_C40
/* #define C40_DEBUG */
extern void IOdebug( const char *, ... );
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
#define KEY_MSDOS_FLAG      0x0000020L
#define KEY_PCC             0x0000040L

#ifdef PASCAL /*ECN*/
#define KEY_ISO             0x0000200L
#endif

#define KEY_F66             0x0000080L
#define KEY_FUSSY           0x0000100L
#define KEY_ONETRIP         0x0000200L
#define KEY_UPPER           0x0000400L
#define KEY_STRICT          0x0000800L
#define KEY_LONGLINES       0x0001000L

#define KEY_MSDOS           0x0002000L
#define KEY_COMMENT         0x0004000L
#define KEY_ASM_OUT         0x0008000L
#define KEY_XPROFILE        0x0010000L
#define KEY_MAKEFILE        0x0020000L
#define KEY_PREPROCESS      0x0040000L
#define KEY_PROFILE         0x0080000L
#define KEY_RENAME          0x0100000L
#define KEY_UNIX            0x0200000L
#define KEY_MD              0x0400000L
#define KEY_READONLY        0x0800000L
#define KEY_STDIN           0x1000000L
#define KEY_COUNTS          0x2000000L
#define KEY_PRE102          0x4000000L

#ifdef FORTRAN

/* The following are duplicated from ffe/feint.h. This is a temporary bodge. */
#define EXT_DOUBLECOMPLEX 1
#define EXT_HINTEGER      2
#define EXT_CASEFOLD      4
#define EXT_LCKEYWORDS    8
#define EXT_LCIDS         0x10
#define EXT_FREEFORMAT    0x20
#define EXT_IMPUNDEFINED  0x40
#define EXT_RECURSION     0x80
#define EXT_AUTO          0x100
#define EXT_HOLLERITH     0x200
#define EXT_TOPEXPRESS    0x400
#define EXT_F66           0x800
#define EXT_MIXEDCOMM     0x1000
#define EXT_VMSCHARS      0x2000
#define EXT_VMSCASTS      0x4000
#define EXT_VMSIO         0x8000
#define EXT_VMSTYPES      0x10000

#define OPT_STATICLOCALS  0x100000
#define OPT_DEFHINTEGER   0x200000
#define OPT_DEFDOUBLE     0x400000
#define OPT_IMPUNDEFINED  0x800000
#define OPT_CHECKSUB      0x1000000
#define OPT_NOARGALIAS    0x2000000
#define OPT_LONGLINES     0x4000000
#define FFEOPTS           0xfff00000

#define F66_ONETRIP       1
#define F66_IOSUBLIST     2
#define F66_INTRINSGO     4

#endif

/*************************************************************/
/*                                                           */
/*   Define the environment information structure            */
/*                                                           */
/*************************************************************/

static char *driver_options[] = DRIVER_OPTIONS;

static struct
{
      int                 fhost;
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
      char                *output_file;
      const char          *trailer;

      const char          *link_startup,
                          *profile_startup,
                          *profile_g_startup;

      const char          *default_lib,
                          *host_lib,
                          *profile_lib,
                          *fort_lib,
                          *pas_lib; /* Should really have a list of these */
}
      env_table[] =
{
#ifdef DRIVER_ENV
    DRIVER_ENV
#else
# ifdef COMPILING_ON_ARM
#  ifdef COMPILING_ON_RISC_OS
    {
      FNAME_ACORN, (KEY_LINK), 0,
/* Perhaps $.clib, $.plib etc should be called $.lib */
      "$.clib", "$.clib", "$.plib", ".", "$.clib", "$.plib", "l",
      "ObjAsm -quit -stamp",
      "CHAIN:link", NULL, "",
      "", "", "",
      "o.stubs", "o.arthurlib", "o.ansilib", "o.fortlib", "o.plib"
    },
#  endif
#  ifdef COMPILING_ON_SPRINGBOARD
    {
      FNAME_MSDOS, (KEY_MSDOS | KEY_LINK), 0,
      "\\arm\\clib", "\\arm\\clib", "\\arm\\plib", "\\", "\\arm\\clib", "\\arm\\plib", "lst",
      "objasm -quit -stamp",
      "CHAIN:link", NULL, "axe",
      "", "", "",
      "ansilib.o", "superlib.o", "", "fortlib.o", "plib.o",
    },
#  endif
#  ifdef COMPILING_ON_UNIX
    {
#    ifndef FORTRAN
#      ifdef TARGET_IS_HELIOS               /* re-check this one */
      FNAME_UNIX, (KEY_UNIX | KEY_LINK), 0,
#      else
      FNAME_UNIX, (KEY_UNIX | KEY_PCC | KEY_LINK), 0,
#      endif
#    else /* FORTRAN */
      FNAME_UNIX, (KEY_UNIX | KEY_LINK),
      (EXT_DOUBLECOMPLEX | EXT_HINTEGER | EXT_CASEFOLD | EXT_LCKEYWORDS |
       EXT_FREEFORMAT | EXT_IMPUNDEFINED | EXT_RECURSION | EXT_AUTO |
       EXT_HOLLERITH | EXT_TOPEXPRESS | EXT_F66 | EXT_MIXEDCOMM |
       EXT_VMSCHARS | EXT_VMSCASTS | EXT_VMSIO | EXT_VMSTYPES |
       OPT_STATICLOCALS | OPT_NOARGALIAS),
#    endif /* FORTRAN */
#    ifdef TARGET_IS_HELIOS         /* software rust awaiting paint. */
      "/helios/include", "/helios/include", "/", "/", "", "", "lst",
      "as",
      "armlink", "a.out", "",
      "/helios/lib/cstart.o", "/helios/lib/cstart.o", "/helios/lib/cstart.o",
      "", "", "", ""
#    else
      "/usr/include/ansi", "/usr/include", "/usr/include/iso", "/", "", "", "lst",
      "as",
      "/usr/bin/ld", "a.out", "",
      "/usr/lib/crt0.o", "/usr/lib/mcrt0.o", "/usr/lib/gcrt0.o",
/* Scanning the Fortran library is done in other ways for Unix */
      "-lc", "", "-lc_p", "", "-lpc"
#    endif
    }
#  endif /* COMPILING_ON_UNIX */
# else /* COMPILING_ON_ARM */
/*
 * For John Fitch on the Clipper, inter alia...   This is another not fully
 * thought out area the entries set up here probably need to be set up via
 * more macros in optiopns.h and host.h: things like search paths, linker
 * commands etc are mostly host specific.
 */
    {
      FNAME_UNIX, ( KEY_UNIX | KEY_LINK ), 0,
      "/users.xenakis/jpff/include.ncc", "/usr/include", "/usr/include/iso", "/", "", "", "lst",
      "as",
      "ld -X -T 8000 -e ___main", "a.out", "",
      "/lib/ncrt0.o", "/lib/ncrt0.o", "/usr/lib/gcrt0.o",
      "-lansi -lc", "", "-lansi -lc_p", "", "-lpc"
    }
#  endif /* COMPILING_ON_ARM */
#endif /* DRIVER_ENV */
},
*setup = env_table;

#ifdef COMPILING_ON_UNIX
#  define FTYPE FNAME_UNIX
#endif
#ifdef COMPILING_ON_MVS
#  define FTYPE FNAME_ACORN     /* a bit of a hack */
#endif
#ifdef COMPILING_ON_RISC_OS
#  define FTYPE FNAME_ACORN
#endif
#ifdef COMPILING_ON_SPRINGBOARD
#  define FTYPE FNAME_MSDOS
#endif
#ifdef COMPILING_ON_HELIOS
#  define FTYPE FNAME_UNIX
#endif

static char  *(*cc_argv)[], *(*cc_filv)[];
static const char *(*ld_argv)[], *(*ld_filv)[];
static int   cc_argc, cc_filc, ld_argc, ld_filc;
static int   cmd_error_count, main_error_count;
static int32 driver_flags;
#ifdef FORTRAN
static int32 pragmax_flags;
#endif

#ifndef COMPILING_ON_UNIX
/*
 * Join two path names and return the result.
 */

static const char *join_path(const char *root, const char *dir,
                             const char *name)
{
  if (root[0] != '\0' && name[0] != '\0')
  {
      char *new_name = malloc(strlen(root)+strlen(dir)+strlen(name)+1);
      strcpy(new_name, root);
      strcat(new_name, dir);
      strcat(new_name, name);
      return new_name;
  }
  return name;
}
#endif

static char *join_strs(const char *s1, const char *s2)
{
  char *s = strcpy(malloc(strlen(s1) + strlen(s2) + 1), s1);
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
        u.path = NULL;
        u.plen = 0;
        unparse = &u;
    }
/* Allocate space for the returned copy of the name. Allow some spare  */
/* for ^ -> .. (several times) + up to 2 path separators + a NUL. Thus */
/* we can survive up to 7 '^.'s in a path being translated to Unix/DOS */
/* form. In practice, tramslation is the other way - ../ -> ^.         */
/* @@@ LDS knows AM won't like the delicay of this argument! Sorry.    */
    n = k + unparse->plen + unparse->rlen + unparse->elen + 10;
    new_name = malloc(n);
    strcpy(new_name, key);
    fname_unparse(unparse, setup->fhost, new_name+k, n-k);
    return new_name;
}

/*
 * Get the value of an external environment variable
 */

static void get_external_environment(void)
{
  char *root = NULL;

#ifdef COMPILING_ON_UNIX
#  ifndef RELEASE_VSN
  root = getenv("CPATH");
#  endif
#endif
#ifdef COMPILING_ON_RISC_OS
  root = getenv("c$libroot");
#endif

  if (root != NULL)
  {
      root = join_strs(root, "");
#ifdef COMPILING_ON_UNIX
      {char *s = root;  while (*s) {if (*s == ':') *s = ','; ++s;}}
#endif
      setup->lib_root = root;
  }
  else root = setup->lib_root;

  if (root[0] != 0)
  {
#ifndef COMPILING_ON_UNIX
      const char *lib = setup->lib_dir;
      setup->default_lib = join_path(root, lib, setup->default_lib);
      setup->host_lib    = join_path(root, lib, setup->host_lib);
      setup->profile_lib = join_path(root, lib, setup->profile_lib);
      setup->fort_lib    = join_path(root, lib, setup->fort_lib);

      cc_msg( "default: %s\n", setup->default_lib );
      cc_msg( "host:    %s\n", setup->host_lib );
      cc_msg( "profile: %s\n", setup->profile_lib );
      cc_msg( "fort:    %s\n", setup->fort_lib );
      
#endif
#ifndef TARGET_IS_C40
      /*
       * XXX - NC - 21/8/1991
       *
       * Why oh why is this done ?????
       *
       * Why do you want to set the default include paths
       * to be the root of the file system ???
       */
      
      setup->default_ansi_path = setup->default_pcc_path = root;
#endif
  }
  
#ifdef PASCAL
#ifdef COMPILING_ON_UNIX
#  ifndef RELEASE_VSN
  root = getenv("PPATH");
#  endif
#endif
#ifdef COMPILING_ON_RISC_OS
  root = getenv("p$libroot");
#endif
  if (root != NULL)
  {
      root = join_strs(root, "");
#ifdef COMPILING_ON_UNIX
      {char *s = root;  while (*s) {if (*s == ':') *s = ','; ++s;}}
#endif
      setup->pas_lib_root = root;
  }
  else root = setup->pas_lib_root;
  if (root[0] != 0) {
#ifndef COMPILING_ON_UNIX
      const char *pas_lib = setup->lib_dir;
      setup->pas_lib = join_path(root, pas_lib, setup->pas_lib);
#endif
      setup->default_pas_path = root;
  }
#endif
}

static void set_default_options(void)
{
  char **argp, *arg;
  /* set up driver options excepting things like -D<letter>id.          */
  for (argp = driver_options, arg = *argp;  arg != NULL;  arg = *(++argp))
  {   if (arg[1] == 'D' && arg[2] != '_' &&
          (driver_flags & (KEY_PCC|KEY_FUSSY)) == KEY_FUSSY) continue;
      new_cc_arg(arg);
  }
}

/*************************************************************/
/*                                                           */
/*      Find a command line keyword and return flag          */
/*                                                           */
/*************************************************************/

static int32 keyword(const char *string)
{
  int count, j, ch;
  static struct { char *word; int32 key; } keytab[] = {
#ifdef PASCAL /*ECN*/
      "-iso",    KEY_ISO,
      "-arthur", KEY_HOST_LIB,
      "-super",  KEY_HOST_LIB,
      "-help",   KEY_HELP,
      "-link",   KEY_LINK,
      "-msdos",  KEY_MSDOS_FLAG,
      "-counts", KEY_COUNTS,
      "-?",      KEY_HELP
#else
#ifndef FORTRAN /* i.e. C, C++, etc */
      "-ansi",    KEY_ANSI,
      "-arthur",  KEY_HOST_LIB,
      "-help",    KEY_HELP,
      "-link",    KEY_LINK,
      "-list",    KEY_LISTING,
      "-msdos",   KEY_MSDOS_FLAG,
      "-pcc",     KEY_PCC,
      "-super",   KEY_HOST_LIB,
      "-fussy",   KEY_FUSSY,
      "-strict",  KEY_STRICT,
      "-counts",  KEY_COUNTS,
      "-pre1.2",  KEY_PRE102,
      "-?",       KEY_HELP
#else /* FORTRAN */
      "-help",    KEY_HELP,
      "-link",    KEY_LINK,
      "-onetrip", KEY_ONETRIP,
      "-list",    KEY_LISTING,
      "-fussy",   KEY_FUSSY,
      "-f66",     KEY_F66,
      "-strict",  KEY_STRICT,
      "-extend",  KEY_LONGLINES,
      "-?",       KEY_HELP
#endif /* FORTRAN */
#endif /* PASCAL */
    };

  
  for (count = 0;  count < sizeof (keytab) / sizeof (keytab[0]);  ++count)
    {
      char *	current = keytab[ count ].word;


      for (ch = *string, j = 0;  ch != '\0';  ch = string[++j])
	{
	  ch = safe_tolower(ch);

	  if (ch != *current)
	    break;

	  ++current;
	}

      /* (ch == 0 || ch != *current) && string[0..j) == key[][0..j) */

      if (ch == '\0' && *current == '\0')
	{	  
	  return keytab[count].key;
	}      
    }
  
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

  if (flags & KEY_MSDOS_FLAG && !(flags & KEY_MSDOS))
  {   cc_msg("Warning: -msdos ignored invalid except on SpringBoard\n");
      flags &= ~KEY_MSDOS_FLAG;
  }

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

  return;
}

static void give_help(const char *command_name)
{
#ifdef COMPILING_ON_UNIX
      cc_msg(CC_BANNER);
#endif
      (void) fprintf(stderr, msg_driver_help, command_name, TARGET_MACHINE);
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
  if (flags & KEY_ONETRIP)     pragmaw_flags |= F66_ONETRIP;
  if (flags & KEY_F66)
      pragmaw_flags |= (F66_ONETRIP + F66_IOSUBLIST + F66_INTRINSGO);
  if (flags & KEY_LONGLINES) pragmax_flags |= OPT_LONGLINES;
  if (flags & KEY_STRICT) pragmax_flags = 0;
  sprintf(pragmaw, "-ZPW%-lu", pragmaw_flags);
  sprintf(pragmax, "-ZPX%-lu", pragmax_flags);
  new_cc_arg(pragmaw);
  new_cc_arg(pragmax);
#endif

  if (flags & KEY_FUSSY)        new_cc_arg("-ZF");
  if (flags & KEY_MAKEFILE)     new_cc_arg("-M");
  if (flags & KEY_PCC)          new_cc_arg("-ZU");
#ifdef PASCAL /*ECN*/
  if (flags & KEY_ISO)          new_cc_arg("-ZZ");
#endif
  if (flags & KEY_PREPROCESS &&
      flags & KEY_COMMENT)      new_cc_arg("-C");
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

  return;
  
}

/*************************************************************/
/*                                                           */
/*      Add an option to a link or assembler command.         */
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

static int assembler(const char *asm_file, const char *obj_file)
{
  int32 flags = driver_flags;
  char *cmd;
  char small_cmd[SMALL_COMMAND];

  cmd = NULL;
  for (;;)
  {   /* once round to count the length and once to copy the strings... */
      int cmdlen = 0;
      cmdlen = cmd_cat(cmd, cmdlen, setup->assembler_cmd);
      if (flags & KEY_READONLY) cmdlen = cmd_cat(cmd, cmdlen, " -R");
      if (!(flags & KEY_UNIX))  cmdlen = cmd_cat(cmd, cmdlen, asm_file);
      if (flags & KEY_UNIX)     cmdlen = cmd_cat(cmd, cmdlen, "-o");
      cmdlen = cmd_cat(cmd, cmdlen, obj_file);
      if (flags & KEY_UNIX)     cmdlen = cmd_cat(cmd, cmdlen, asm_file);
      if (cmd != NULL) break;
      if (cmdlen < SMALL_COMMAND)
          cmd = small_cmd;
      else
          cmd = malloc(cmdlen+1);
  }
#ifdef TESTING
  cc_msg("%s\n", cmd);
#endif
  return system(cmd);
}

/*************************************************************/
/*                                                           */
/*      Link compiled files together                         */
/*                                                           */
/*************************************************************/

static void linker(int32 flags)
{
#ifndef TARGET_IS_NULL          /* Hmmm, but, but ... */
  int count, cmdlen;
  char *cmd, small_cmd[SMALL_COMMAND];
  const char *startup;


#ifdef C40_DEBUG
  IOdebug( "linker: called" );
#endif
  
  cmd = NULL;

  for (;;)
  {   /* once round to count the length and once to copy the strings... */
      cmdlen = 0;
      cmdlen = cmd_cat(cmd, cmdlen, setup->link_cmd);

      for (count = 0;  count < ld_argc;  ++count)
          cmdlen = cmd_cat(cmd, cmdlen, (*ld_argv)[count]);

      cmdlen = cmd_cat(cmd, cmdlen, "-o");
      cmdlen = cmd_cat(cmd, cmdlen, setup->output_file);

      switch (flags & (KEY_PROFILE | KEY_XPROFILE))
      {
case KEY_PROFILE:   startup = setup->profile_startup;    break;
case KEY_XPROFILE:  startup = setup->profile_g_startup;  break;
default:            startup = setup->link_startup;       break;
      }
      cmdlen = cmd_cat(cmd, cmdlen, startup);

      for (count = 0;  count < ld_filc;  ++count)
          cmdlen = cmd_cat(cmd, cmdlen, (*ld_filv)[count]);

      count = cmdlen;
#ifdef PASCAL
      /* Pascal library must be before C library */
      /* ? Why is fortran library after C library ? */
      cmdlen = cmd_cat(cmd, cmdlen, setup->pas_lib);
#endif

      cmdlen = cmd_cat(cmd, cmdlen, setup->default_lib);

      if (flags & KEY_HOST_LIB)
          cmdlen = cmd_cat(cmd, cmdlen, setup->host_lib);

#ifdef FORTRAN
      cmdlen = cmd_cat(cmd, cmdlen, setup->fort_lib);
#endif

      if (cmd != NULL) break;
      if (cmdlen < SMALL_COMMAND)
          cmd = small_cmd;
      else
          cmd = malloc(cmdlen+1);
  }
  
  while (count < cmdlen)
  {   /* space-separate, rather than comma-join, the library list */
      if (cmd[count] == ',') cmd[count] = ' ';  ++count;
  }

#ifdef TESTING
  cc_msg("%s\n", cmd);
#endif

#ifdef C40_DEBUG
  IOdebug( "linker: cmd = %s", cmd );
#endif
  
  count = system( cmd );

#ifdef C40_DEBUG
  IOdebug( "linker: system returned %d", count );
#endif
  
  if (count != 0) ++main_error_count;

  /*
   * In PCC mode delete the '.o' file if only one file was compiled.
   * NB. (count==0) is used to check the link was ok.
   */
  
  if ((flags & KEY_PCC) && (cc_filc == 1) && (count==0))
      remove((*ld_filv)[0]);
#else
  flags = flags;  /* keep ncc happy */
#endif

#ifdef C40_DEBUG
  IOdebug( "linker: finished" );
#endif
  
  return;
}

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
    {
      char *	current = filv[count];
      int	extn_ch;

    
      if (strlen(current) > MAX_TEXT-5)
	{
	  cc_msg("Overlong filename ignored: %s\n", current);
	
	  continue;
	}

      cc_argc = saved_cc_argc;

      fname_parse(current, FTYPE, FNAME_SUFFIXES, &unparse);

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
#ifdef COMPILING_ON_UNIX
case 'a':
#else
case 'O':
#endif
case 'o':     if ((flags & (KEY_PREPROCESS+KEY_MAKEFILE)) == 0)
              {
                  (*ld_filv)[ld_filc++] = copy_unparse("", &unparse, NULL);
                  break;
              }
              else cc_msg("Warning: %s -E/M %s - inconsistent options\n",
                           (*cc_argv)[0], current);
              /* and fall through ... */

default:      if ((flags & (KEY_PREPROCESS+KEY_MAKEFILE)) == 0)
              {
                  cc_msg("Error: type of '%s' unknown (file ignored)\n",
                          current);
                  ++cmd_error_count;
                  continue;
              }
              /* fall through again (-E, -M) */

#ifndef COMPILING_ON_UNIX
case 'S':
#endif
case 's':     if ((flags & (KEY_PREPROCESS+KEY_MAKEFILE)) == 0)
              {   const char *asm_file = copy_unparse("", &unparse, NULL);
                  const char *obj_file = copy_unparse("", &unparse, "o");
                  if (assembler(asm_file, obj_file) != 0)
                  {   main_error_count++;
                      remove(obj_file);
                  }
                  /* and fall through... foo.s... */
              }
              /* fall through again (-E, -M, foo.s) */

#ifndef COMPILING_ON_UNIX
case LANG_UC_EXTN:
#endif
case LANG_EXTN:
          {   char *out_file = setup->output_file;
              char *out_name = NULL;

              if (flags & KEY_ASM_OUT)
              {
                  if (flags & KEY_RENAME && filc == 1)
                      /* Assert: KEY_RENAME => setup->output_file != NULL */
                      new_cc_arg(join_strs("-S", out_name = out_file));
                  else
                  {   
#ifdef TARGET_IS_KCM
		      out_name = copy_unparse("-S", &unparse, "mas");
#else
		      out_name = copy_unparse("-S", &unparse, "s");
#endif
                      new_cc_arg(out_name);
                      out_name += 2;                       /* skip the -S */
                  }
                  new_cc_arg("-C");             /* suppress object output */
              }

              if (flags & KEY_LISTING)
                  new_cc_arg(copy_unparse("-L", &unparse, setup->list));
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
                   flags & (KEY_LINK+KEY_MAKEFILE)  ||
                  (flags & KEY_RENAME) == 0)
              {   /*
                   * No-default-output-file ||
                   * more-than-1-file || going-to-link || no -o <file>
                   */
                  out_file = copy_unparse("", &unparse, "o");
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
      if (setup->output_file == NULL && flags & KEY_LINK)
          setup->output_file = copy_unparse("", &unparse, setup->trailer);
  }

  return;

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
      list = malloc(INCLUDE_BUFLEN);
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
          list = realloc(list, lim);
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
      if (key == 0L) continue;
#ifdef FORTRAN
      if (key == KEY_PCC)
          pragmax_flags = (EXT_DOUBLECOMPLEX | EXT_HINTEGER | EXT_CASEFOLD |
                        EXT_LCKEYWORDS | EXT_FREEFORMAT | EXT_IMPUNDEFINED |
               EXT_RECURSION | EXT_AUTO | EXT_HOLLERITH | OPT_STATICLOCALS);
      else
#endif
          driver_flags |= key;
  }
#ifdef PASCAL /*ECN*/
  driver_flags |= KEY_ANSI;
#endif
}

static void read_compile_options(int argc, char *argv[])
{
  int count;
  int32 flags;
  char *include_path = new_include("-", NULL),
        *system_path = new_include("-", NULL),
       *library_path = NULL;

  flags = driver_flags;
  
#ifndef FORTRAN

  for (count=1; count < argc;  ++count)
  {
      char *current = argv[count];
      char *next;
      int32 key = keyword(current);

      
      if (key != 0L) continue;        /* already processed */

      if (current[0] == '-')
      {   int uc_opt = current[1];
          uc_opt = safe_toupper(uc_opt);

          switch(uc_opt)
          {
#ifndef INMOSC
      case 'E': if (current[2] == 'S' && current[3] == 0) break;
#endif
      case 'O': if (current[1] == 'o') break;
      case 'C':
#ifndef PASCAL /*ECN*/
      case 'R':
#endif
      case 'S':
                if (current[2] == 0) break;
                bad_option(current);
          }

          switch(uc_opt)
          {
  case '\0':  flags |= KEY_STDIN;    /* '-' on its own denotes stdin... */
              break;

  case 'C':   if (current[1] == 'c')
                  flags &= ~KEY_LINK;
              else
                  flags |= KEY_COMMENT;
              break;

  case 'E':
#ifdef INMOSC
              /* provide support for -Exyz to suppress certain errors.  */
              if (current[2] == 0)
#endif
                  flags = (flags | KEY_PREPROCESS) & ~KEY_LINK;
              /* and fall through... */
  case 'F':   new_cc_arg(current);           break;

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

  case 'M':
#ifdef COMPILING_ON_UNIX
              if (current[1] == 'm')            /* request link map      */
                  (*ld_argv)[ld_argc++] = current;
              else
#endif
              switch(safe_toupper(current[2]))
              {
      case '\0':  flags = (flags | KEY_MAKEFILE) & ~KEY_LINK;
                  break;
      case 'D':   if (current[3] == '\0')
                  {
                      flags |= KEY_MD;
                      break;
                  }
      default:    bad_option(current);
              }
              break;

  case 'O':   if (current[1] == 'o') goto may_take_next_arg;
              new_cc_arg("-ZPZ1");
              break;

  case 'P':   uc_opt = current[2];
              uc_opt = safe_toupper(uc_opt);
              switch(uc_opt)
              {
      case '\0':  flags |= KEY_PROFILE;
                  new_cc_arg(current);
                  if (setup->profile_lib[0] != '\0')
                      setup->default_lib = setup->profile_lib;
                  break;

      case 'G':
      case 'X':   if (current[3] == '\0')
                  {
                      flags |= KEY_XPROFILE;
                      new_cc_arg(current);
                      if (setup->profile_lib[0] != '\0')
                          setup->default_lib =
                              setup->profile_lib;
                      break;
                  }

      default:    bad_option(current);
              }
              break;

  case 'R':   new_cc_arg(current);
              if (flags & KEY_UNIX) flags |= KEY_READONLY;
              break;

  case 'S':
#ifdef COMPILING_ON_UNIX
              if (current[1] == 's')
                  (*ld_argv)[ld_argc++] = "-s";
              else
#endif
                  flags = (flags & ~KEY_LINK) | KEY_ASM_OUT;
              break;

  case 'W':   new_cc_arg(current);
              break;

  case 'Z':   uc_opt = current[2];
              uc_opt = safe_toupper(uc_opt);

              switch(uc_opt)
              {
      case '\0':  /* Pass on '-z' to the linker */
                  (*ld_argv)[ld_argc++] = current;
                  break;

#ifdef TARGET_IS_HELIOS         /* the miserable -zr helios option */
      case 'L':                 /* -zl for replacement for -zr */
      case 'R':   flags &= ~KEY_LINK;
      case 'S':                 /* -zs for split module tables */
      case 'D':                 /* extra addressability mode */ 
		  /* drop through */
#endif
      case 'A': case 'C': case 'F': case 'J': case 'U':
#ifndef TARGET_IS_UNIX
      case 'O': case 'X':
      case '1': case '2':
#ifdef REGSTATS
/*
 * These lines are here while ACN does some experiments - but note that
 * ACN thinks it is a bit nasty that driver does some syntax checking on
 * options AS WELL as compiler doing so - especially in the case of
 * mcdep_init options I think that things should NOR be checked here
 * at all...   See also another #ifdef REGSTATS.
 */
      case '3': case '4':
      case '5': case '6':
      case '7': case '8':
      case '9':
#endif
#endif
                  if (current[3]=='\0')
		    {
                      new_cc_arg(current);
		    }
                  else
		    {
                      bad_option(current);
		    }
		  
                  break;

      case 'B':
#ifndef TARGET_IS_UNIX
      case 'M':
#endif
#ifdef REGSTATS
                  if (YES)
#else
                  if (current[3] == 0 ||
                      current[4] == 0 && isdigit(current[3]))
#endif
                      new_cc_arg(current);
                  else
                      bad_option(current);
                  break;

      case 'I':
      case 'K':
      case 'P':
      case 'Q':   goto may_take_next_arg;

      default:    bad_option(current);
                  break;
              }
              break;

  default:    (*ld_argv)[ld_argc++] = current;  break;

  case 'D':  case 'I':  case 'J':  case 'U':
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
      case 'D':
      case 'U':   if (current[2] == 0) current = join_strs(current, next);
                  new_cc_arg(current);
                  break;
      case 'I':   include_path = new_include(next, include_path);
                  break;
      case 'J':   system_path  = new_include(next, system_path);
                  break;
      case 'L':   library_path = new_include(next, library_path);
                  break;
      case 'O':   flags |= KEY_RENAME;
                  setup->output_file = next;
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
      else
	{
	  (*cc_filv)[cc_filc++] = current;
	}      
  }

#else /* FORTRAN */

  for (count=1; count < argc;  ++count)
  {
      char *current = argv[count];
      char *next;
      int32 key = keyword(current);

      if (key != 0L) continue;        /* already processed */

      if (current[0] == '-')
      {   int uc_opt = current[1];
          uc_opt = safe_toupper(uc_opt);
          switch(uc_opt)
          {
      case 'F': if (current[1] == 'f') break;
/* @@@ LDS 10Aug89 - DO NOT CATCH 'O' HERE - IT BREAKS Unix Makefiles */
      case 'M': if (current[1] == 'M') break;
      case 'C':
      case 'U':
      case 'V':
      case 'S':
                if (current[2] == 0) break;
                bad_option(current);
          }

          switch(uc_opt)
          {
  case '\0':  flags |= KEY_STDIN;    /* '-' on its own denotes stdin... */
              break;

  case '1':   flags |= KEY_ONETRIP;
              break;

  case 'C':   if (current[1] == 'c') flags &= ~KEY_LINK;
              else pragmax_flags |= OPT_CHECKSUB;
              break;

  case 'E':   if (current[1] == 'E') nimp_option(current);
              else {
                  flags = (flags | KEY_PREPROCESS) & ~KEY_LINK;
                  new_cc_arg(current);
              } break;

  case 'F':   if (current[1] == 'F') nimp_option(current);
              else new_cc_arg(current);
              break;

  case 'G':   new_cc_arg(current);
              if (flags & KEY_UNIX)
                  library_path = new_include("-lg", library_path);
              else
                  (*ld_argv)[ld_argc++] = "-d";
              break;

  case 'I':   if ((current[1] == 'i') && (current[2] == '2'))
                  pragmax_flags |= OPT_DEFHINTEGER;
              else goto may_take_next_arg;
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

  case 'M':   if (current[1] == 'm') nimp_option(current);
              else switch(safe_toupper(current[2]))
              {
      case '\0':  flags = (flags | KEY_MAKEFILE) & ~KEY_LINK;
                  break;
#ifndef RELEASE_VSN
      case 'D':   if (current[3] == '\0')
                  {
                      flags |= KEY_MD;
                      break;
                  }
#endif
      default:    bad_option(current);
              }
              /* drop through */
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

  case 'P':   uc_opt = current[2];
              uc_opt = safe_toupper(uc_opt);
              switch(uc_opt)
              {
      case '\0':  flags |= KEY_PROFILE;
                  new_cc_arg(current);
                  if (setup->profile_lib[0] != '\0')
                      setup->default_lib = setup->profile_lib;
                  break;

      case 'G':
      case 'X':   if (current[3] == '\0')
                  {
                      flags |= KEY_XPROFILE;
                      new_cc_arg(current);
                      if (setup->profile_lib[0] != '\0')
                          setup->default_lib =
                              setup->profile_lib;
                      break;
                  }

      default:    bad_option(current);
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

  case 'S':
#ifdef COMPILING_ON_UNIX
              if (current[1] == 's')
                  (*ld_argv)[ld_argc++] = "-s";
              else
#endif
                  flags = (flags & ~KEY_LINK) | KEY_ASM_OUT;
              break;

  case 'U':   if (current[1] == 'U') pragmax_flags &= ~EXT_CASEFOLD;
              else pragmax_flags |= OPT_IMPUNDEFINED;
              break;

  case 'V':   if (current[1] == 'v')
              {
                 new_cc_arg("-FB");
                 cc_msg("%s\n",CC_BANNER);
              }
              else goto link_command;
              break;

  case 'W':   new_cc_arg(current);
              break;

  case 'Z':   uc_opt = current[2];
              uc_opt = safe_toupper(uc_opt);
              switch(uc_opt)
              {
      case '\0':  /* Pass on '-z' to the linker */
                  (*ld_argv)[ld_argc++] = current;
                  break;

#ifdef TARGET_IS_HELIOS         /* the miserable -zr helios option */
      case 'L':
      case 'R':   flags &= ~KEY_LINK;
      case 'S':
      case 'D':
                  /* drop through */
#endif
      case 'A': case 'C': case 'F': case 'U':
#ifndef TARGET_IS_UNIX
      case 'O': case 'X':
      case '1': case '2':
#endif
                  if (current[3]=='\0')
                      new_cc_arg(current);
                  else
                      bad_option(current);
                  break;

      case 'B':
#ifndef TARGET_IS_UNIX
      case 'M':
#endif
                  if (current[3] == 0 ||
                      current[4] == 0 && isdigit(current[3]))
                      new_cc_arg(current);
                  else
                      bad_option(current);
                  break;

      case 'P':
      case 'Q':   goto may_take_next_arg;

      default:    bad_option(current);
                  break;
              }
              break;

  link_command:
  default:    (*ld_argv)[ld_argc++] = current;  break;

  case 'D':  case 'J':
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
      case 'D':
      case 'U':   if (current[2] == 0) current = join_strs(current, next);
                  new_cc_arg(current);
                  break;
      case 'I':   include_path = new_include(next, include_path);
                  break;
      case 'J':   system_path  = new_include(next, system_path);
                  break;
      case 'L':   library_path = new_include(next, library_path);
                  break;
      case 'O':   flags |= KEY_RENAME;
                  setup->output_file = next;
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

#endif /* ! FORTRAN */

  driver_flags = flags;

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
       *
       * XXX - NC - no no no this is wrong,
       * system and user paths should be kept seperate ...
       *
       */
    
#ifdef PASCAL
      const char *path = setup->default_pas_path;
#else
#ifdef TARGET_IS_C40
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
          path = setup->default_pcc_path;
	}
      else
	{
          path = setup->default_ansi_path;
	}
      
#else /* ! TARGET_IS_C40 */
      
      const char *path = setup->default_pcc_path;
      if (((flags & KEY_ANSI) || !(flags & KEY_PCC)) && (flags & KEY_PRE102))
	{
          path = setup->default_ansi_path;
	}
#endif /* TARGET_IS_C40 */
#endif /* !PASCAL */

#if 1
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

#ifdef FORTRAN
  /*
   * If linking fortran, we need to link to libc.a, libm.a, libF77.a,
   * libI77.a and libU77.a (for now, not considering profiling versions)
   */

  if (flags & KEY_UNIX)
  {
      if ((flags & KEY_PROFILE) || (flags & KEY_XPROFILE))
#ifdef FORTRAN_NCLIB
          library_path = new_include("-lnfc_p", library_path);
#else
          library_path = new_include("-lF66_p,-lF77_p,-lI77_p,-lU77_p,-lm_p",
                                     library_path);
#endif
      else
#ifdef FORTRAN_NCLIB
          library_path = new_include("-lnfc",library_path);
#else
          library_path = new_include("-lF66,-lF77,-lI77,-lU77,-lm",
                                     library_path);
#endif
  }
#else /* FORTRAN */

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
  if ((flags & KEY_ANSI || !(flags & KEY_PCC)) && flags & KEY_PRE102)
  {
      library_path = new_include("-lansi,-lm", library_path);
  }
#endif
#endif /* FORTRAN */

  if (library_path != NULL)
  {
      if (flags & KEY_UNIX)
          library_path = new_include(setup->default_lib, library_path);
      setup->default_lib = library_path;
  }
}

/*
 * Main.
 */

int main(int argc, char *argv[])
{
  int count;
  bool is_cpp = NO;
  main_error_count = cmd_error_count = 0;


#ifdef C40_DEBUG
  IOdebug( "Norcroft C Compiler: starting" );
#endif

  err_init();   /* needed before cc_msg */

#ifdef C40_DEBUG
  IOdebug( "Ncc: err_init has returned" );
#endif
  
#ifndef COMPILING_ON_UNIX
#ifndef COMPILING_ON_MSDOS
  (void) signal(SIGINT, compile_abort);
#endif
  cc_msg("%s\n",CC_BANNER);
#else
#  ifdef DRIVER_PRE_RELEASE_MSG
     cc_msg("%s\n",DRIVER_PRE_RELEASE_MSG);
#  endif
  
  /* The signal ignore state can be inherited from the parent... */
  
#ifndef COMPILING_ON_MSDOS
#define sig_ign ((void (*)(int))(SIG_IGN))
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

#ifdef C40_DEBUG
  IOdebug( "Ncc: got environment variables" );
#endif
  
  driver_flags = setup->initial_flags;
  
#ifdef FORTRAN
  pragmax_flags = setup->initial_pragmax;
#endif

  cc_argv = (char *(*)[]) malloc(sizeof(char *) *
                          (argc + sizeof(driver_options)/sizeof(char *) + 3));
  cc_filv = (char *(*)[]) malloc(sizeof(char *) * argc);
  ld_argv = (const char *(*)[]) malloc(sizeof(const char *) * argc);
  ld_filv = (const char *(*)[]) malloc(sizeof(const char *) * argc);
  cc_argc = cc_filc = ld_argc = ld_filc = 0;

  new_cc_arg(argv[0]);

#ifdef C40_DEBUG
  IOdebug( "Ncc: arg arrays set up" );
#endif
  
  count = strlen(argv[0]);
  
  if (count >= 3 && strcmp(&argv[0][count-3], "cpp") == 0)
    {
      /* The compiler was called as '...cpp' - treat as 'cc -E'.  */

      driver_flags = (driver_flags | KEY_PREPROCESS) & ~KEY_LINK;

      new_cc_arg("-E");

      is_cpp = YES;
    }

#ifdef C40_DEBUG
  IOdebug( "Ncc: prog name checked" );
#endif
  
  read_keyword_options(argc, argv);

#ifdef C40_DEBUG
  IOdebug( "Ncc: keywords read" );
#endif
  
  set_default_options();

#ifdef C40_DEBUG
  IOdebug( "Ncc: default options set" );
#endif
  
  read_compile_options(argc, argv);

#ifdef C40_DEBUG
  IOdebug( "Ncc: compiler options read" );
#endif
  
  validate_flags();

#ifdef C40_DEBUG
  IOdebug( "Ncc: command line flags processed" );
#endif
  
  if (driver_flags & KEY_HELP)
    {
#ifdef C40_DEBUG
      IOdebug( "Ncc: giving help" );
#endif
      
      give_help(argv[0]);

      exit(0);
    }

#ifdef C40_DEBUG
  IOdebug( "Ncc: setting flag options" );
#endif
  
  set_flag_options();

#ifdef C40_DEBUG
  IOdebug( "Ncc: options set" );
#endif
  
  if (is_cpp)
    {
      if (cc_filc >= 2)
	{
	  if (cc_filc > 2)
	    cc_msg("More than 2 file arguments to cpp ignored\n");

	  if (freopen((*cc_filv)[1], "w", stdout) == NULL)
	    {
	      cc_msg("Can't open output file %s\n", (*cc_filv)[1]);

	      exit(EXIT_error);
	    }

	  cc_filc = 1;
	}

#ifdef C40_DEBUG
      IOdebug( "Ncc: cpp file opened" );
#endif
    }

  if (cc_filc > 0)
    {
      if (driver_flags & KEY_STDIN)
	cc_msg("Invalid filename '-' ignored\n");

#ifdef C40_DEBUG
      IOdebug( "Ncc: about to process %d files %s (%x)", cc_filc, **cc_filv, **cc_filv );
#endif
    
      process_file_names(cc_filc, *cc_filv);

#ifdef C40_DEBUG
      IOdebug( "Ncc: processed" );
#endif
    }
  else if (driver_flags & (KEY_PREPROCESS | KEY_MAKEFILE | KEY_ASM_OUT) &&
           driver_flags & KEY_STDIN || is_cpp)
    {
#ifdef C40_DEBUG
      IOdebug( "Ncc: processing stdin" );
#endif
      
      if (ccom(cc_argc, *cc_argv)) ++main_error_count;

#ifdef C40_DEBUG
      IOdebug( "Ncc: stdin processed" );
#endif
    }
#ifdef PASCAL /*ECN*/
  else cc_msg("Warning: pc command with no effect\n");
#else
  else
    {
      cc_msg("Warning: %s command with no effect\n", (*cc_argv)[0]);   /* yes? */
    }
#endif

#ifdef C40_DEBUG
  IOdebug( "Ncc: compilations finished" );
#endif
  
  if ((main_error_count == 0) && (driver_flags & KEY_LINK) && (ld_filc > 0))
    {
#ifdef C40_DEBUG
      IOdebug( "Ncc: linking" );
#endif
      
      linker(driver_flags);

#ifdef C40_DEBUG
      IOdebug( "Ncc: linked" );
#endif
    }

#ifdef C40_DEBUG
  IOdebug( "Ncc: finished" );
#endif
  
  /*
   * The SUN ignores the return value from main so exit() instead
   */
  
  if (main_error_count + cmd_error_count > 0)
    exit(EXIT_error);

  exit(0);

  return 0;
}

/* End of driver.c */
