/* Copyright (C) 1988, 1989 Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GNU Make is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Make; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* $Header: /hsrc/cmds/gnu/gmake/RCS/make.h,v 1.3 1992/06/25 11:07:31 nickc Exp $ */

#include <signal.h>
#include <stdio.h>

#ifndef	sun
#include <sys/types.h>
#endif

#ifndef HELIOS
#include <sys/param.h>
#endif
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif	/* No MAXPATHLEN.  */

#include <sys/stat.h>

#ifdef	USG
#include <string.h>
#define	index(s, c)	strchr((s), (c))
#define	rindex(s, c)	strrchr((s), (c))

#include <memory.h>
#define bcmp(s1, s2, n)	memcmp ((s1), (s2), (n))
#define bzero(s, n)	memset ((s), 0, (n))
#define bcopy(s, d, n)	memcpy ((d), (s), (n))

#else	/* Not USG.  */
#include <strings.h>

extern int bcmp ();
extern void bzero (), bcopy ();

#endif	/* USG.  */

#ifdef	__GNUC__
#define	alloca(n)	__builtin_alloca (n)
#else	/* Not GCC.  */
#ifdef	sparc
#include <alloca.h>
#else	/* Not sparc.  */
extern char *alloca ();
#endif	/* sparc.  */
#endif	/* GCC.  */

#ifndef	iAPX286
#define streq(a, b) \
  ((a) == (b) || \
   (*(a) == *(b) && (*(a) == '\0' || !strcmp ((a) + 1, (b) + 1))))
#else
/* Buggy compiler can't handle this.  */
#define streq(a, b) (strcmp ((a), (b)) == 0)
#endif

/* Add to VAR the hashing value of C, one character in a name.  */
#define	HASH(var, c) \
  ((var += (c)), (var = ((var) << 7) + ((var) >> 20)))

#if defined(__GNUC__) || defined(ENUM_BITFIELDS)
#define	ENUM_BITFIELD(bits)	:bits
#else
#define	ENUM_BITFIELD(bits)
#endif

extern void die ();
extern void fatal ( char *, ... ), error ( char *, ... );
extern void pfatal_with_name (), perror_with_name ();
extern char *savestring (), *concat ();
extern char *xmalloc (), *xrealloc ();
extern char *find_next_token (), *next_token (), *end_of_token ();
extern void collapse_continuations (), collapse_line ();
extern char *sindex (), *lindex ();
extern int alpha_compare ();
extern void print_spaces ();
extern struct dep *copy_dep_chain ();
extern char *find_percent ();

#ifndef	NO_ARCHIVES
extern int ar_name ();
extern int ar_touch ();
extern time_t ar_member_date ();
#endif

extern void dir_load ();
extern int dir_file_exists_p (), file_exists_p (), file_impossible_p ();
extern void file_impossible ();
extern char *dir_name ();

extern void set_default_suffixes (), install_default_implicit_rules ();
extern void convert_to_pattern (), count_implicit_rule_limits ();
extern void create_pattern_rule ();

extern void build_vpath_lists (), construct_vpath_list ();
extern int vpath_search ();

extern void construct_include_path ();

extern int update_goal_chain ();
extern void notice_finished_file ();


extern int glob_pattern_p ();
extern char **glob_filename ();

#ifndef	USG
extern int sigsetmask ();
#endif
extern int kill (), sigblock ();
extern void free ();
extern void abort (), exit ();
extern int unlink (), stat ();
extern void qsort ();
extern int atoi ();

#include <time.h>
#include <fcntl.h>

extern char **environ;

#ifdef	USG
extern char *getcwd ();
#define	getwd(buf)	getcwd (buf, MAXPATHLEN - 2)
#else	/* Not USG.  */
extern char *getwd ();
#endif	/* USG.  */


extern char *reading_filename;
extern unsigned int *reading_lineno_ptr;

extern int just_print_flag, silent_flag, ignore_errors_flag, keep_going_flag;
extern int debug_flag, print_data_base_flag, question_flag, touch_flag;
extern int env_overrides, no_builtin_rules_flag, print_version_flag;
extern int print_directory_flag;

extern unsigned int job_slots;
extern double max_load_average;

extern char *program;

extern unsigned int makelevel;


#define DEBUGPR(msg)							\
  if (debug_flag) { print_spaces (depth); printf (msg, file->name);	\
		    fflush (stdout);  } else
