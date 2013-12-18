/*
 * drivhelp.c -- help text for Norcroft C/FORTRAN compiler, version 1b.
 * Copyright (C) Codemist Ltd., 1989.
 * Copyright (C) Acorn Computers Ltd., 1989.
 */

/* The help text here is not what is wanted on non-Acorn machines, and  */
/* is also not what is wanted for a Lint system.                        */
/* parameterisation here seems in the process of getting out of hand!   */

static char *driver_help_text[] = {

#ifdef msg_driver_help         /* to allow options.h to define */
    msg_driver_help,

#else
#ifndef TARGET_IS_UNIX
#ifndef TARGET_IS_HELIOS
/* Arthur and Brazil -help text */

#ifdef PASCAL /*ECN*/

"\nUsage :       %s [options] file1 file2 ... filen",
"",
"Main options:",
"",
"-list         Generate a compilation listing",
"-iso          Compile strictly according to ISO (BS 6192 : 1982)",
"",
"-c            Do not link the files being compiled",
"-C            Prevent the preprocessor from removing comments (Use with -E)",
"-D<symbol>    Define <symbol> on entry to the compiler",
"-E            Preprocess the Pascal source code only, do not compile or link it",
"-F<options>   Enable a selection of compiler defined features",
"-R<options>   Disable selected run time error checks",
"-g<options>   Generate code that may be used with the debugger",
"-I<directory> Include <directory> on the #include search path",
"-J<directory> Replace the default #include path with <directory>",
"-L<libs>      Specify a comma-joined list of libraries to be linked with",
"              instead of the standard library",
"-o<file>      Instruct the linker to call the object code produced <file>",
"-P<options>   Generate code to generate 'profile' information",
"-S            Output assembly code instead of object code",
"-U<symbol>    Undefine <symbol> on entry to the compiler",
"-W<options>   Disable all or selected warning and error messages",

#else

#ifndef FORTRAN

"\nUsage :       %s [options] file1 file2 ... filen",
"",
"Main options:",
"",
"-list          Generate a compilation listing",
"-pcc           Compile UNIX PCC style C source code",
"",
"-c             Do not link the files being compiled",
"-C             Prevent the preprocessor from removing comments (Use with -E)",
"-D<symbol>     Define <symbol> on entry to the compiler",
"-E             Preprocess the C source code only - do not compile or link it",
"-F<options>    Enable a selection of compiler defined features",
"-g<options>    Generate code that may be used with the debugger",
"-I<directory>  Include <directory> on the #include search path",
"-J<directory>  Replace the default #include path with <directory>",
"-L<libs>       Specify a comma-joined list of libraries to be linked with",
"               instead of the standard library",
"-o<file>       Instruct the linker to call the object code produced <file>",
"-P<options>    Generate code to generate 'profile' information",
"-S             Output assembly code instead of object code",
"-U<symbol>     Undefine <symbol> on entry to the compiler",
"-W<options>    Disable all or selected warning and error messages",

#else /* FORTRAN */

"\nUsage :       %s [options] file1 file2 ... filen",
"",
"Main options:",
"",
"-f66           Follow F66 practices or rules when conflicting with F77",
"-list          Generate a compilation listing",
"",
"-c             Do not link the files being compiled",
"-C             Prevent the preprocessor from removing comments (Use with -E)",
"-D<symbol>     Define <symbol> on entry to the compiler",
"-E             Preprocess the F77 source code only - do not compile or link it",
"-F<options>    Enable a selection of compiler defined features",
"-g<options>    Generate code that may be used with the debugger",
"-I<directory>  Include <directory> on the #include search path",
"-J<directory>  Replace the default #include path with <directory>",
"-L<libs>       Specify a comma-joined list of libraries to be linked with",
"               instead of the standard library",
"-o<file>       Instruct the linker to call the object code produced <file>",
"-P<options>    Generate code to generate 'profile' information",
"-S             Output assembly code instead of object code",
"-U<symbol>     Undefine <symbol> on entry to the compiler",
"-W<options>    Disable all or selected warning and error messages",

#endif /* FORTRAN */
#endif /* PASCAL */

#else /* TARGET_IS_HELIOS */
/* Helios -help text */

#ifdef PASCAL /*ECN*/

"\nUsage :       %s [options] file1 file2 ... filen",
"",
"Main options:",
"",
"-list         Generate a compilation listing",
"-iso          Compile strictly according to ISO (BS 6192 : 1982)",
"",
"-c            Do not link the files being compiled",
"-C            Prevent the preprocessor from removing comments (Use with -E)",
"-D<symbol>    Define <symbol> on entry to the compiler",
"-E            Preprocess the Pascal source code only, do not compile or link it",
"-F<options>   Enable a selection of compiler defined features",
"-R<options>   Disable selected run time error checks",
"-g<options>   Generate code that may be used with the debugger",
"-I<directory> Include <directory> on the #include search path",
"-J<directory> Replace the default #include path with <directory>",
"-L<libs>      Specify a comma-joined list of libraries to be linked with",
"              instead of the standard library",
"-o<file>      Instruct the linker to call the object code produced <file>",
"-P<options>   Generate code to generate 'profile' information",
"-S            Output assembly code instead of object code",
"-U<symbol>    Undefine <symbol> on entry to the compiler",
"-W<options>   Disable all or selected warning and error messages",

#else

#ifndef FORTRAN

"\nUsage :       %s [options] file1 file2 ... filen",
"",
"Main options:",
"",
"-list          Generate a compilation listing",
"-pcc           Compile UNIX PCC style C source code",
"",
"-c             Do not link the files being compiled",
"-C             Prevent the preprocessor from removing comments (Use with -E)",
"-D<symbol>     Define <symbol> on entry to the compiler",
"-E             Preprocess the C source code only - do not compile or link it",
"-F<options>    Enable a selection of compiler defined features",
"-g<options>    Generate code that may be used with the debugger",
"-I<directory>  Include <directory> on the #include search path",
"-J<directory>  Replace the default #include path with <directory>",
"-L<libs>       Specify a comma-joined list of libraries to be linked with",
"               instead of the standard library",
"-o<file>       Instruct the linker to call the object code produced <file>",
"-P<options>    Generate code to generate 'profile' information",
"-S             Output assembly code instead of object code",
"-U<symbol>     Undefine <symbol> on entry to the compiler",
"-W<options>    Disable all or selected warning and error messages",
"-Z<option>     Special Helios options for shared library building etc",

#else /* FORTRAN */

#define msg_driver_help ,
"\nUsage :       %s [options] file1 file2 ... filen",
"",
"Main options:",
"",
"-arthur        Add 'arthur' to the list of libraries to be linked with",
"                 (Arthur only)",
"-f66           Follow F66 practises or rules when conflicting with F77",
"-list          Generate a compilation listing",
"-super         Add 'supervisor' to the list of libraries to be linked with",
"                 (Brazil only)",
"",
"-c             Do not link the files being compiled",
"-C             Prevent the preprocessor from removing comments (Use with -E)",
"-D<symbol>     Define <symbol> on entry to the compiler",
"-E             Preprocess the F77 source code only - do not compile or link it",
"-F<options>    Enable a selection of compiler defined features",
"-g<options>    Generate code that may be used with the debugger",
"-I<directory>  Include <directory> on the #include search path",
"-J<directory>  Replace the default #include path with <directory>",
"-L<libs>       Specify a comma-joined list of libraries to be linked with",
"               instead of the standard library",
"-o<file>       Instruct the linker to call the object code produced <file>",
"-P<options>    Generate code to generate 'profile' information",
"-S             Output assembly code instead of object code",
"-U<symbol>     Undefine <symbol> on entry to the compiler",
"-W<options>    Disable all or selected warning and error messages",

#endif /* FORTRAN */
#endif /* PASCAL */

#endif /* TARGET_IS_HELIOS */
#else /* TARGET_IS_UNIX */

#ifdef PASCAL /*ECN*/

#define msg_driver_help ,
"\n\nUsage :       %s [-options] file1 file2 ... filen",
"",
"Main options:",
"",
"-iso          Compile strictly according to ISO (BS 6192 : 1982)",
"-c            Do not invoke the linker to link the files being compiled",
"-C            Prevent the preprocessor from removing comments (Use with -E)",
"-D<symbol>    Define preprocessor <symbol> on entry to the compiler",
"-E            Preprocess the Pascal source code only, do not compile or link it",
"-F<options>   Enable a selection of compiler defined features",
"-g            Generate code that may be used with the debugger",
"-I<directory> Include <directory> on the '#include' search path",
"-list         Generate a compilation listing",
"-M<options>   Generate a 'makefile' style list of dependencies",
"-o <file>     Instruct the linker to name the object code produced <file>",
"-O            Invoke the object code improver",
"-p<options>   Generate code to produce 'profile' information",
"-R            Place all compile time strings in a 'Read only' segment",
"-S            Generate assembly code instead of object code",
"-U<symbol>    Undefine preprocessor <symbol> on entry to the compiler",
"-w<options>   Disable all or selected warning and error messages",

#else

#ifndef FORTRAN

#define msg_driver_help ,
"\n\nUsage :       %s [-options] file1 file2 ... filen",
"",
"Main options:",
"",
"-ansi          Compile ANSI-style C source code",
"-pcc           Compile BSD UNIX PCC-style C source code",
"-c             Do not invoke the linker to link the files being compiled",
"-C             Prevent the preprocessor from removing comments (Use with -E)",
"-D<symbol>     Define preprocessor <symbol> on entry to the compiler",
"-E             Preprocess the C source code only, do not compile or link it",
"-F<options>    Enable a selection of compiler defined features",
"-g             Generate code that may be used with the debugger",
"-I<directory>  Include <directory> on the '#include' search path",
"-list          Generate a compilation listing",
"-M<options>    Generate a 'makefile' style list of dependencies",
"-o <file>      Instruct the linker to name the object code produced <file>",
"-O             Invoke the object code improver",
"-p<options>    Generate code to produce 'profile' information",
"-R             Place all compile time strings in a 'Read only' segment",
"-S             Generate assembly code instead of object code",
"-U<symbol>     Undefine preprocessor <symbol> on entry to the compiler",
"-w<options>    Disable all or selected warning and error messages",

#else /* FORTRAN */

#define msg_driver_help ,
"\n\nBSD compatible ANSI F77 compiler.",
"\nUsage :       %s [-options] file1 file2 ... filen",
"",
"Main options:",
"",
"-c             Do not invoke the linker to link the files being compiled",
"-f66           Follow F66 practices or rules when conflicting with F77",
"-g             Generate code that may be used with the debugger",
"-i2            Make the default integer size 16 bits",
"-list          Generate a compilation listing",
"-onetrip       Compile DO loops that are performed at least once if reached",
"-o <file>      Instruct the linker to name the object code produced <file>",
"-O             Invoke the object code improver",
"-p<options>    Generate code to produce 'profile' information",
"-r             Place all compile time strings in a 'Read only' segment",
"-strict        Accept only programs strictly conforming to ANSI standard",
"-S             Generate assembly code instead of object code",
"-U             Do not convert upper case letters to lower case",
"-w<options>    Disable all or selected warning and error messages",

#endif /* FORTRAN */
#endif /* PASCAL */

#endif /* TARGET_IS_UNIX */

#endif /* msg_driver_help */
    0
};

/* end of mip/drivhelp.h */
