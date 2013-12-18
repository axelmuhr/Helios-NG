#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * drivhelp.c -- help text for Norcroft C/FORTRAN compiler, version 2.
 * Copyright (C) Codemist Ltd., 1989.
 * Copyright (C) Acorn Computers Ltd., 1989.
 */

/* The help text here is not what is wanted on non-Acorn machines, and  */
/* is also not what is wanted for a Lint system.                        */
/* parameterisation here seems in the process of getting out of hand!   */

#ifndef msg_driver_help         /* to allow options.h to define */

#ifndef TARGET_IS_UNIX
#ifndef TARGET_IS_HELIOS
/* Arthur and Brazil -help text */

#ifdef PASCAL /*ECN*/

#define msg_driver_help \
"\nUsage :       %s [options] file1 file2 ... filen\n\
\n\
Main options:\n\
\n\
-list         Generate a compilation listing\n\
-iso          Compile strictly according to ISO (BS 6192 : 1982)\n\
\n\
-c            Do not link the files being compiled\n\
-C            Prevent the preprocessor from removing comments (Use with -E)\n\
-D<symbol>    Define <symbol> on entry to the compiler\n\
-E            Preprocess the Pascal source code only, do not compile or link it\n\
-F<options>   Enable a selection of compiler defined features\n\
-R<options>   Disable selected run time error checks\n\
-g<options>   Generate code that may be used with the debugger\n\
-I<directory> Include <directory> on the #include search path\n\
-J<directory> Replace the default #include path with <directory>\n\
-L<libs>      Specify a comma-joined list of libraries to be linked with\n\
              instead of the standard library\n\
-o<file>      Instruct the linker to call the object code produced <file>\n\
-P<options>   Generate code to generate 'profile' information\n\
-S            Output %s assembly code instead of object code\n\
-U<symbol>    Undefine <symbol> on entry to the compiler\n\
-W<options>   Disable all or selected warning and error messages\n"

#else

#ifndef FORTRAN

#define msg_driver_help \
"\nUsage :       %s [options] file1 file2 ... filen\n\
\n\
Main options:\n\
\n\
-arthur     }{ Under RISC OS, add the obsolescent Arthur interface library\n\
-super      }{ <c$LibRoot>.o.ArthurLib to the list of libraries to be linked\n\
               with; under SpringBoard add the Brazil supervisor interface\n\
               library \\arm\\clib\\SuperLib.o to the list of libraries\n\
-list          Generate a compilation listing\n\
-pcc           Compile UNIX PCC style C source code\n\
\n\
-c             Do not link the files being compiled\n\
-C             Prevent the preprocessor from removing comments (Use with -E)\n\
-D<symbol>     Define <symbol> on entry to the compiler\n\
-E             Preprocess the C source code only - do not compile or link it\n\
-F<options>    Enable a selection of compiler defined features\n\
-g<options>    Generate code that may be used with the debugger\n\
-I<directory>  Include <directory> on the #include search path\n\
-J<directory>  Replace the default #include path with <directory>\n\
-L<libs>       Specify a comma-joined list of libraries to be linked with\n\
               instead of the standard library\n\
-o<file>       Instruct the linker to call the object code produced <file>\n\
-P<options>    Generate code to generate 'profile' information\n\
-S             Output %s assembly code instead of object code\n\
-U<symbol>     Undefine <symbol> on entry to the compiler\n\
-W<options>    Disable all or selected warning and error messages\n"

#else /* FORTRAN */

#define msg_driver_help \
"\nUsage :       %s [options] file1 file2 ... filen\n\
\n\
Main options:\n\
\n\
-arthur        Add 'arthur' to the list of libraries to be linked with\n\
                 (Arthur only)\n\
-f66           Follow F66 practises or rules when conflicting with F77\n\
-list          Generate a compilation listing\n\
-super         Add 'supervisor' to the list of libraries to be linked with\n\
                 (Brazil only)\n\
\n\
-c             Do not link the files being compiled\n\
-C             Prevent the preprocessor from removing comments (Use with -E)\n\
-D<symbol>     Define <symbol> on entry to the compiler\n\
-E             Preprocess the F77 source code only - do not compile or link it\n\
-F<options>    Enable a selection of compiler defined features\n\
-g<options>    Generate code that may be used with the debugger\n\
-I<directory>  Include <directory> on the #include search path\n\
-J<directory>  Replace the default #include path with <directory>\n\
-L<libs>       Specify a comma-joined list of libraries to be linked with\n\
               instead of the standard library\n\
-o<file>       Instruct the linker to call the object code produced <file>\n\
-P<options>    Generate code to generate 'profile' information\n\
-S             Output %s assembly code instead of object code\n\
-U<symbol>     Undefine <symbol> on entry to the compiler\n\
-W<options>    Disable all or selected warning and error messages\n"

#endif /* FORTRAN */
#endif /* PASCAL */

#else /* TARGET_IS_HELIOS */
/* Helios -help text */

#ifdef PASCAL /*ECN*/

#define msg_driver_help \
"\nUsage :       %s [options] file1 file2 ... filen\n\
\n\
Main options:\n\
\n\
-list         Generate a compilation listing\n\
-iso          Compile strictly according to ISO (BS 6192 : 1982)\n\
\n\
-c            Do not link the files being compiled\n\
-C            Prevent the preprocessor from removing comments (Use with -E)\n\
-D<symbol>    Define <symbol> on entry to the compiler\n\
-E            Preprocess the Pascal source code only, do not compile or link it\n\
-F<options>   Enable a selection of compiler defined features\n\
-R<options>   Disable selected run time error checks\n\
-g<options>   Generate code that may be used with the debugger\n\
-I<directory> Include <directory> on the #include search path\n\
-J<directory> Replace the default #include path with <directory>\n\
-L<libs>      Specify a comma-joined list of libraries to be linked with\n\
              instead of the standard library\n\
-o<file>      Instruct the linker to call the object code produced <file>\n\
-P<options>   Generate code to generate 'profile' information\n\
-S            Output %s assembly code instead of object code\n\
-U<symbol>    Undefine <symbol> on entry to the compiler\n\
-W<options>   Disable all or selected warning and error messages\n"

#else

#ifndef FORTRAN

#ifdef TARGET_HAS_PROFILE
  
#define msg_driver_help \
"\nUsage :       %s [options] file1 file2 ... filen\n\
\n\
Main options:\n\
\n\
-list          Generate a compilation listing\n\
-pcc           Compile UNIX PCC style C source code\n\
\n\
-c             Do not link the files being compiled\n\
-C             Prevent the preprocessor from removing comments (Use with -E)\n\
-D<symbol>     Define <symbol> on entry to the compiler\n\
-E             Preprocess the C source code only - do not compile or link it\n\
-F<options>    Enable a selection of compiler defined features\n\
-g<options>    Generate code that may be used with the debugger\n\
-I<directory>  Include <directory> on the #include search path\n\
-J<directory>  Replace the default #include path with <directory>\n\
-L<libs>       Specify a comma-joined list of libraries to be linked with\n\
               instead of the standard library\n\
-o<file>       Instruct the linker to call the object code produced <file>\n\
-P<options>    Generate code to generate 'profile' information\n\
-S             Output %s assembly code instead of object code\n\
-U<symbol>     Undefine <symbol> on entry to the compiler\n\
-W<options>    Disable all or selected warning and error messages\n\
-Z<option>     Special Helios options for shared library building etc\n"

#else /* ! TARGET_HAS_PROFILE */

#define msg_driver_help \
"\nUsage :       %s [options] file1 file2 ... filen\n\
\n\
Main options:\n\
\n\
-list          Generate a compilation listing\n\
-pcc           Compile UNIX PCC style C source code\n\
\n\
-c             Do not link the files being compiled\n\
-C             Prevent the preprocessor from removing comments (Use with -E)\n\
-D<symbol>     Define <symbol> on entry to the compiler\n\
-E             Preprocess the C source code only - do not compile or link it\n\
-F<options>    Enable a selection of compiler defined features\n\
-g<options>    Generate code that may be used with the debugger\n\
-I<directory>  Include <directory> on the #include search path\n\
-J<directory>  Replace the default #include path with <directory>\n\
-L<libs>       Specify a comma-joined list of libraries to be linked with\n\
               instead of the standard library\n\
-o<file>       Instruct the linker to call the object code produced <file>\n\
-S             Output %s assembly code instead of object code\n\
-U<symbol>     Undefine <symbol> on entry to the compiler\n\
-W<options>    Disable all or selected warning and error messages\n\
-Z<option>     Special Helios options for shared library building etc\n"

#endif /* TARGET_HAS_PROFILE */
  
#else /* FORTRAN */

#define msg_driver_help \
"\nUsage :       %s [options] file1 file2 ... filen\n\
\n\
Main options:\n\
\n\
-arthur        Add 'arthur' to the list of libraries to be linked with\n\
                 (Arthur only)\n\
-f66           Follow F66 practises or rules when conflicting with F77\n\
-list          Generate a compilation listing\n\
-super         Add 'supervisor' to the list of libraries to be linked with\n\
                 (Brazil only)\n\
\n\
-c             Do not link the files being compiled\n\
-C             Prevent the preprocessor from removing comments (Use with -E)\n\
-D<symbol>     Define <symbol> on entry to the compiler\n\
-E             Preprocess the F77 source code only - do not compile or link it\n\
-F<options>    Enable a selection of compiler defined features\n\
-g<options>    Generate code that may be used with the debugger\n\
-I<directory>  Include <directory> on the #include search path\n\
-J<directory>  Replace the default #include path with <directory>\n\
-L<libs>       Specify a comma-joined list of libraries to be linked with\n\
               instead of the standard library\n\
-o<file>       Instruct the linker to call the object code produced <file>\n\
-P<options>    Generate code to generate 'profile' information\n\
-S             Output %s assembly code instead of object code\n\
-U<symbol>     Undefine <symbol> on entry to the compiler\n\
-W<options>    Disable all or selected warning and error messages\n"

#endif /* FORTRAN */
#endif /* PASCAL */

#endif /* TARGET_IS_HELIOS */
#else /* TARGET_IS_UNIX */

#ifdef PASCAL /*ECN*/

#define msg_driver_help \
"\n\nUsage :       %s [-options] file1 file2 ... filen\n\
\n\
Main options:\n\
\n\
-iso          Compile strictly according to ISO (BS 6192 : 1982)\n\
-c            Do not invoke the linker to link the files being compiled\n\
-C            Prevent the preprocessor from removing comments (Use with -E)\n\
-D<symbol>    Define preprocessor <symbol> on entry to the compiler\n\
-E            Preprocess the Pascal source code only, do not compile or link it\n\
-F<options>   Enable a selection of compiler defined features\n\
-g            Generate code that may be used with the debugger\n\
-I<directory> Include <directory> on the '#include' search path\n\
-list         Generate a compilation listing\n\
-M<options>   Generate a 'makefile' style list of dependencies\n\
-o <file>     Instruct the linker to name the object code produced <file>\n\
-O            Invoke the object code improver\n\
-p<options>   Generate code to produce 'profile' information\n\
-R            Place all compile time strings in a 'Read only' segment\n\
-S            Generate %s assembly code instead of object code\n\
-U<symbol>    Undefine preprocessor <symbol> on entry to the compiler\n\
-w<options>   Disable all or selected warning and error messages\n"

#else

#ifndef FORTRAN

#define msg_driver_help \
"\n\nUsage :       %s [-options] file1 file2 ... filen\n\
\n\
Main options:\n\
\n\
-ansi          Compile ANSI-style C source code\n\
-pcc           Compile BSD UNIX PCC-style C source code\n\
-c             Do not invoke the linker to link the files being compiled\n\
-C             Prevent the preprocessor from removing comments (Use with -E)\n\
-D<symbol>     Define preprocessor <symbol> on entry to the compiler\n\
-E             Preprocess the C source code only, do not compile or link it\n\
-F<options>    Enable a selection of compiler defined features\n\
-g             Generate code that may be used with the debugger\n\
-I<directory>  Include <directory> on the '#include' search path\n\
-list          Generate a compilation listing\n\
-M<options>    Generate a 'makefile' style list of dependencies\n\
-o <file>      Instruct the linker to name the object code produced <file>\n\
-O             Invoke the object code improver\n\
-p<options>    Generate code to produce 'profile' information\n\
-R             Place all compile time strings in a 'Read only' segment\n\
-S             Generate %s assembly code instead of object code\n\
-U<symbol>     Undefine preprocessor <symbol> on entry to the compiler\n\
-w<options>    Disable all or selected warning and error messages\n"

#else /* FORTRAN */

#define msg_driver_help \
"\n\nBSD compatible ANSI F77 compiler.\n\
\nUsage :       %s [-options] file1 file2 ... filen\n\
\n\
Main options:\n\
\n\
-c             Do not invoke the linker to link the files being compiled\n\
-f66           Follow F66 practises or rules when conflicting with F77\n\
-g             Generate code that may be used with the debugger\n\
-i2            Make the default integer size 16 bits\n\
-list          Generate a compilation listing\n\
-onetrip       Compile DO loops that are performed at least once if reached\n\
-o <file>      Instruct the linker to name the object code produced <file>\n\
-O             Invoke the object code improver\n\
-p<options>    Generate code to produce 'profile' information\n\
-r             Place all compile time strings in a 'Read only' segment\n\
-strict        Accept only programs strictly conforming to ANSI standard\n\
-S             Generate %s assembly code instead of object code\n\
-U             Do not convert upper case letters to lower case\n\
-w<options>    Disable all or selected warning and error messages\n"

#endif /* FORTRAN */
#endif /* PASCAL */

#endif /* TARGET_IS_UNIX */

#endif /* msg_driver_help */

/* end of mip/drivhelp.h */
