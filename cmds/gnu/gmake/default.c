/* Data base of default implicit rules for GNU Make.
Copyright (C) 1988, 1989 Free Software Foundation, Inc.
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

#include "make.h"
#include "rule.h"
#include "dep.h"
#include "file.h"
#include "commands.h"
#include "variable.h"

static char *rcsid = "$Header: /dsl/HeliosRoot/Helios/cmds/gnu/gmake/RCS/default.c,v 1.1 1990/08/28 14:32:04 james Exp $";

/* This is the default list of suffixes for suffix rules.
   `.s' must come last, so that a `.o' file will be made from
   a `.c' or `.p' or ... file rather than from a .s file.  */

static char default_suffixes[]
  = ".out .a .ln .o .c .cc .p .f .F .r .y .l .s .S \
.mod .sym .def .h .info .dvi .tex .texinfo .cweb .web .sh .elc .el";

static struct pspec default_pattern_rules[] =
  {
    "(%)", "%",
    "$(AR) $(ARFLAGS) $@ $<",

    /* The X.out rules are only in BSD's default set because
       BSD Make has no null-suffix rules, so `foo.out' and
       `foo' are the same thing.  */
    "%.out", "%",
    "@rm -f $@ \n cp $< $@",

    0, 0, 0
  };

static struct pspec default_terminal_rules[] =
  {
    /* RCS.  */
    "%", "%,v",
    "test -f $@ || $(CO) $(COFLAGS) $< $@",
    "%", "RCS/%,v",
    "test -f $@ || $(CO) $(COFLAGS) $< $@",

    /* SCCS.  */
    "%", "s.%",
    "$(GET) $(GFLAGS) $<",
    "%", "SCCS/s.%",
    "$(GET) $(GFLAGS) $<",

    0, 0, 0
  };

static char *default_suffix_rules[] =
  {
    ".o",
    "$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".s",
    "$(LINK.s) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".S",
    "$(LINK.S) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".c",
    "$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".cc",
    "$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".f",
    "$(LINK.f) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".p",
    "$(LINK.p) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".F",
    "$(LINK.F) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".r",
    "$(LINK.r) $^ $(LOADLIBES) $(LDLIBS) -o $@",
    ".mod",
    "$(COMPILE.mod) -o $@ -e $@ $^",

    ".def.sym", 
    "$(COMPILE.def) -o $@ $<",

    ".sh",
    "cat $< >$@ \n chmod a+x $@",

    ".s.o",
#ifndef	M_XENIX
    "$(COMPILE.s) -o $@ $<",
#else	/* Xenix.  */
    "$(COMPILE.s) -o$@ $<",
#endif	/* Not Xenix.  */
    ".c.o",
    "$(COMPILE.c) $< $(OUTPUT_OPTION)",
    ".cc.o",
    "$(COMPILE.cc) $< $(OUTPUT_OPTION)",
    ".f.o",
    "$(COMPILE.f) $< $(OUTPUT_OPTION)",
    ".p.o",
    "$(COMPILE.p) $< $(OUTPUT_OPTION)",
    ".F.o",
    "$(COMPILE.F) $< $(OUTPUT_OPTION)",
    ".r.o",
    "$(COMPILE.r) $< $(OUTPUT_OPTION)",
    ".mod.o",
    "$(COMPILE.mod) -o $@ $<",

    ".c.ln",
    "$(LINT.c) -C$* $<",
    ".y.ln",
    "$(YACC.y) $< \n $(LINT.c) -C$* y.tab.c \n $(RM) y.tab.c",
    ".l.ln",
    "@$(RM) $*.c \n $(LEX.l) $< > $*.c \n\
$(LINT.c) -i $*.c -o $@ \n $(RM) $*.c",

    ".y.c",
    "$(YACC.y) $< \n mv -f y.tab.c $@",
    ".l.c",
    "@$(RM) $@ \n $(LEX.l) $< > $@",

    ".F.f",
    "$(PREPROCESS.F) $< $(OUTPUT_OPTION)",
    ".r.f",
    "$(PREPROCESS.r) $< $(OUTPUT_OPTION)",

    /* This might actually make lex.yy.c if there's no %R%
       directive in $*.l, but in that case why were you
       trying to make $*.r anyway?  */
    ".l.r",
    "$(LEX.l) $< > $@ \n mv -f lex.yy.r $@",

    ".S.s",
    "$(PREPROCESS.S) $< > $@",

    ".texinfo.info",
    "$(MAKEINFO) $<",

    ".tex.dvi",
    "$(TEX) $<",

    ".texinfo.dvi",
    "$(TEXINDEX) $(wildcard $(foreach _s_,cp fn ky pg tp vr,$*.$(_s_)))\n\
     -$(foreach _f_,$(wildcard $(foreach _s_,aux cp fn ky pg tp vr,$*.$(_s_)))\
,cp $(_f_) $(_f_)O;)\n\
     -$(TEX) $< \n\
     $(foreach _f_,$(wildcard $(foreach _s_,aux cp fn ky pg tp vr,\
$*.$(_s_))),cmp -s $(_f_)O $(_f_) ||) \\\n\
($(TEXINDEX) $(wildcard $(foreach _s_,cp fn ky pg tp vr,$*.$(_s_))); \\\n\
$(TEX) $<) \n\
     -rm -f $(addsuffix O,$(wildcard $(foreach _s_,\
aux cp fn ky pg tp vr,$*.$(_s_))))",

    ".cweb.c",
    "$(CTANGLE) $<",

    ".web.p",
    "$(TANGLE) $<",

    ".cweb.tex",
    "$(CWEAVE) $<",

    ".web.tex",
    "$(WEAVE) $<",

    0
  };

static char *default_variables[] =
  {
    "AR", "ar",
    "ARFLAGS", "rv",
    "AS", "as",
#ifndef HELIOS
    "CC", "cc",
#else
    "CC", "c",
#endif
    "C++", "g++",
    "CO", "co",
    "CPP", "$(CC) -E",
    "FC", "f77",
    /* System V uses these, so explicit rules using them should work.
       However, there is no way to make implicit rules use them and FC.  */
    "F77", "$(FC)",
    "F77FLAGS", "$(FFLAGS)",
#ifdef	USG
    "GET", "get",
#else
    "GET", "sccs get",
#endif
    "LD", "ld",
#ifndef HELIOS
    "LEX", "lex",
#else
    "LEX", "flex",
#endif
    "LINT", "lint",
    "M2C", "m2c",
    "PC", "pc",
#ifndef HELIOS
    "YACC", "yacc",	/* Or "bison -y"  */
#else
    "YACC", "bison -y",	/* Or "bison -y"  */
#endif
    "MAKEINFO", "makeinfo",
    "TEX", "tex",
    "TEXINDEX", "texindex",
    "WEAVE", "weave",
    "CWEAVE", "cweave",
    "TANGLE", "tangle",
    "CTANGLE", "ctangle",

    "RM", "rm -f",

    "LINK.o", "$(CC) $(LDFLAGS) $(TARGET_ARCH)",
    "COMPILE.c", "$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c",
    "LINK.c", "$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)",
    "COMPILE.cc", "$(C++) $(C++FLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c",
    "LINK.cc", "$(C++) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)",
    "YACC.y", "$(YACC) $(YFLAGS)",
    "LEX.l", "$(LEX) $(LFLAGS) -t",
    "COMPILE.f", "$(FC) $(FFLAGS) $(TARGET_ARCH) -c",
    "LINK.f", "$(FC) $(FFLAGS) $(LDFLAGS) $(TARGET_ARCH)",
    "COMPILE.F", "$(FC) $(FFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c",
    "LINK.F", "$(FC) $(FFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)",
    "COMPILE.r", "$(FC) $(FFLAGS) $(RFLAGS) $(TARGET_ARCH) -c",
    "LINK.r", "$(FC) $(FFLAGS) $(RFLAGS) $(LDFLAGS) $(TARGET_ARCH)",
    "COMPILE.def", "$(M2C) $(M2FLAGS) $(DEFFLAGS) $(TARGET_ARCH)",
    "COMPILE.mod", "$(M2C) $(M2FLAGS) $(MODFLAGS) $(TARGET_ARCH)",
    "COMPILE.p", "$(PC) $(PFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c",
    "LINK.p", "$(PC) $(PFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)",
    "COMPILE.s", "$(AS) $(ASFLAGS) $(TARGET_MACH)",
    "COMPILE.S", "$(CC) $(ASFLAGS) $(CPPFLAGS) $(TARGET_MACH) -c",
    "LINT.c", "$(LINT) $(LINTFLAGS) $(CPPFLAGS) $(TARGET_ARCH)",
#ifndef	M_XENIX
    "PREPROCESS.S", "$(CC) -E $<",
#else	/* Xenix.  */
    "PREPROCESS.S", "$(CC) -EP $<",
#endif	/* Not Xenix.  */
    "PREPROCESS.F", "$(FC) $(FFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -F",
    "PREPROCESS.r", "$(FC) $(FFLAGS) $(RFLAGS) $(TARGET_ARCH) -F",

#ifndef	NO_MINUS_C_MINUS_O
#ifndef	M_XENIX
    "OUTPUT_OPTION", "-o $@",
#else	/* Xenix.  */
    "OUTPUT_OPTION", "-Fo$@",
#endif	/* Not Xenix.  */
#endif

    0, 0
  };

/* Set up the default .SUFFIXES list.  */

void
set_default_suffixes ()
{
  suffix_file = enter_file (".SUFFIXES");

  if (no_builtin_rules_flag)
    (void) define_variable ("SUFFIXES", 8, "", o_default, 0);
  else
    {
      char *p = default_suffixes;
      suffix_file->deps = (struct dep *)
	multi_glob (parse_file_seq (&p, '\0', sizeof (struct dep)),
		    sizeof (struct dep));
      (void) define_variable ("SUFFIXES", 8, default_suffixes, o_default, 0);
    }
}

/* Install the default pattern rules and enter
   the default suffix rules as file rules.  */

void
install_default_implicit_rules ()
{
  register struct pspec *p;
  register char **s;
  
  if (no_builtin_rules_flag)
    return;

  for (p = default_pattern_rules; p->target != 0; ++p)
    install_pattern_rule (p, 0);

  for (p = default_terminal_rules; p->target != 0; ++p)
    install_pattern_rule (p, 1);

  for (s = default_suffix_rules; *s != 0; s += 2)
    {
      register struct file *f = enter_file (s[0]);
      f->cmds = (struct commands *) xmalloc (sizeof (struct commands));
      f->cmds->filename = 0;
      f->cmds->commands = s[1];
      f->cmds->command_lines = 0;
    }

  for (s = default_variables; *s != 0; s += 2)
    (void) define_variable (s[0], strlen (s[0]), s[1], o_default, 1);
}
