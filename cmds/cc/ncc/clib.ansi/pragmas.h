/*
 * pragmas.h.          Copyright (C) Acorn Computers Ltd, 1988
 */

/*
 * This header file is provided for purposes of documentation only, and
 * the macros which it defines are not directly useful - but in effect
 * the expansions shown here are implemented by the preprocessor after the
 * #pragma directive, so the names defined here as aliases for the basic
 * and cryptic way of setting options are generally available.
 */


/* Warnings about various non-ANSIisms.
 * These are all on by default; they can be turned off globally
 * by -w{f,c,d}.
 */
#define WARN_IMPLICIT_FN_DECLS -a1
#define NO_WARN_IMPLICIT_FN_DECLS -a0

#define WARN_IMPLICIT_CASTS -b1
#define NO_WARN_IMPLICIT_CASTS -b0

#define WARN_DEPRECATED -d1
#define NO_WARN_DEPRECATED -d0

/* printf/scanf argument checking.  For any function declared while
 * CHECK_FORMATS is not off, calls of the function with a literal format
 * string will have the types of the other arguments checked against those
 * implied by the format string, and a warning will be given if they don't
 * match.
 * Off by default.
 */
#define NO_CHECK_FORMATS -v0
#define CHECK_PRINTF_FORMATS -v1
#define CHECK_SCANF_FORMATS -v2

/* Stack overflow checking.
 * On by default */
#define CHECK_STACK -s1
#define NO_CHECK_STACK- -s0

/* Checks before pointer dereference that the pointer value is plausible.
 * Off by default.
 */
#define CHECK_MEMORY_ACCESSES -c1
#define NO_CHECK_MEMORY_ACCESSES -c0

/* Control of profiling (also globally by -p)
 * PROFILE generates counts just for function entries.
 * PROFILE_STATEMENTS generates counts for each branch point.
 * Off by default.
 */
#define NO_PROFILE -p0
#define PROFILE -p1
#define PROFILE_STATEMENTS -p2

/* NO_SIDE_EFFECTS promises that functions declared while it is on neither read
 * nor modify global state, and thus are candidates for CSE.
 * Off by default.
 */
#define NO_SIDE_EFFECTS -y1
#define SIDE_EFFECTS -y0

/* Common subexpression elimination.  This is expected always to cause programs
 * to be faster and smaller.  However, for pathological cases it can consume very
 * large amounts of both space and time.
 * On by default (?????????????? or off???????????)
 */
#define OPTIMISE_CSE -z1
#define NO_OPTIMISE_CSE -z0

#define OPTIMISE_CROSSJUMP -j1
#define NO_OPTIMISE_CROSSJUMP -j0

#define OPTIMISE_MULTIPLE_LOADS -m1
#define NO_OPTIMISE_MULTIPLE_LOADS -m0

/* Global integer register n is pragma -rn.
   Global floating register n is pragma -fn.
 */
