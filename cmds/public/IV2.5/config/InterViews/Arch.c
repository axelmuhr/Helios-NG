/*
 * To add support for another platform:
 * 
 *     1.  Identify a machine-specific cpp symbol.  If your preprocessor 
 *         doesn't have any built in, you'll need to add the symbol to the
 *         cpp_argv table in imake/imake.c and rebuild imake with the 
 *         BOOTSTRAPCFLAGS variable set (see the macII for an example).
 *
 *     2.  Add all machine-specific cpp symbols (either defined by you or by
 *         the preprocessor or compiler) to the predefs table in
 *         makedepend/main.c.
 *
 *     3.  Put a new #ifdef block below that defines MacroIncludeFile and
 *         MacroFile for your new platform and then #undefs the machine-
 *         specific preprocessor symbols (to avoid problems with file names).
 *
 *     4.  Create a .cf file with the name given by MacroFile.
 */

#ifdef ibm032
#undef ibm032
#undef RT
#undef rt
#define RtArchitecture
#define MachineDep RT
#define machinedep rt
#define MacroIncludeFile "rt.cf"
#define MacroFile rt.cf
#endif /* ibm032 */

#ifdef ultrix
#undef ultrix
#define UltrixArchitecture
#ifdef vax
#undef VAX
#undef vax
#define VaxArchitecture
#define MachineDep VAX
#define machinedep vax
#endif
#ifdef mips
#undef mips
#undef MIPSEL
#undef mipsel
#define MipsArchitecture
#define MachineDep MIPSEL
#define machinedep mipsel
#endif
#define MacroIncludeFile "ultrix.cf"
#define MacroFile ultrix.cf
#endif

#if defined(vax) && !defined(UltrixArchitecture)
#undef VAX
#undef vax
#define VaxArchitecture
#define MachineDep VAX
#define machinedep vax
#define MacroIncludeFile "bsd.cf"
#define MacroFile bsd.cf
#endif

#ifdef sun
#undef sun
#define SunArchitecture
#ifdef mc68020
#undef SUN3
#undef sun3
#define MachineDep SUN3
#define machinedep sun3
#endif
#ifdef sparc
#undef SUN4
#undef sun4
#define MachineDep SUN4
#define machinedep sun4
#endif
#ifdef i386
#undef SUN386i
#undef sun386i
#define MachineDep SUN386i
#define machinedep sun386i
#endif
#ifndef MachineDep
#undef SUN
#undef sun
#define MachineDep SUN
#define machinedep sun
#endif
#define MacroIncludeFile "sun.cf"
#define MacroFile sun.cf
#endif /* sun */

#ifdef hpux
#undef hpux
#define HPArchitecture
#ifdef hp9000s300
#undef HP300
#undef hp300
#define MachineDep HP300
#define machinedep hp300
#else
#ifdef hp9000s200
#undef HP200
#undef hp200
#define MachineDep HP200
#define machinedep hp200
#endif
#endif
#ifdef hp9000s500
#undef HP500
#undef hp500
#define MachineDep HP500
#define machinedep hp500
#endif
#ifdef hp9000s800
#undef HP800
#undef hp800
#define MachineDep HP800
#define machinedep hp800
#endif
#ifndef MachineDep
#undef HP
#undef hp
#define MachineDep HP
#define machinedep hp
#endif
#define MacroIncludeFile "hp.cf"
#define MacroFile hp.cf
#endif /* hpux */

#ifdef apollo
#undef APOLLO
#undef apollo
#define ApolloArchitecture
#define MachineDep APOLLO
#define machinedep apollo
#define MacroIncludeFile "apollo.cf"
#define MacroFile apollo.cf
#endif /* apollo */

#ifdef M4310
#undef M4310
#undef PEGASUS
#undef pegasus
#define PegasusArchitecture
#define MachineDep PEGASUS
#define machinedep pegasus
#define MacroIncludeFile "pegasus.cf"
#define MacroFile pegasus.cf
#endif /* M4310 */

#ifdef M4330
#undef M4330
#undef m4330
#define M4330Architecture
#define MachineDep M4330
#define machinedep m4330
#define MacroIncludeFile "m4330.cf"
#define MacroFile m4330.cf
#endif /* M4330 */

/* A/UX cpp has no unique symbol:  build imake with BOOTSTRAPCFLAGS=-DmacII */
#ifdef macII
#undef MACII
#undef macii
#define MacIIArchitecture
#define MachineDep MACII
#define machinedep macII
#define MacroIncludeFile "macII.cf"
#define MacroFile macII.cf
#endif /* macII */

#ifdef CRAY
#undef CRAY
#undef cray
#define CrayArchitecture
#define MachineDep CRAY
#define machinedep cray
#define MacroIncludeFile "cray.cf"
#define MacroFile cray.cf
#endif /* CRAY */

#ifdef sgi
#undef SGI
#undef sgi
#undef mips
#define SGIArchitecture
#define MipsArchitecture
#define MachineDep SGI
#define machinedep sgi
#define MacroIncludeFile "sgi.cf"
#define MacroFile sgi.cf
#endif

#if defined(mips) && !defined(UltrixArchitecture) && !defined(SGIArchitecture)
#undef mips
#undef umips
#undef MIPSEB
#undef mipseb
#define MipsArchitecture
#define UMipsArchitecture
#define MachineDep MIPSEB
#define machinedep mipseb
#define MacroIncludeFile "umips.cf"
#define MacroFile umips.cf
#endif

#ifndef MachineDep
/**/# WARNING: Imake.tmpl not configured; guessing at definitions!!!
/**/# This might mean that BOOTSTRAPCFLAGS wasn't set when building imake.
#undef UNKNOWN
#undef unknown
#define MachineDep UNKNOWN
#define machinedep unknown
#define MacroIncludeFile "generic.cf"
#define MacroFile generic.cf
#endif

/*
 * Identify the architecture for scripts/cpu.sh.
 */
/**/# architecture:  MachineDep
