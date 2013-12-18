#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * version.h
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:02:48 $
 * Revising $Author: nickc $
 */

/*
 * Defines the compiler banner.  The functional interface is preferable
 * to a #define of the string (which in any case isn't possible for a non-Ansi
 * compiler) because it saves space for multiple instances of the string,
 * at negligible expense in time.  Also, it minimises recompilation when
 * all that has changed is the version
 */

#ifndef _version_h
#define _version_h 1

extern char *version_banner(void);

#define CC_BANNER version_banner()

/* note that for object file purposes CC_BANNERlen MUST be a multiple of 4
   and include the final null in CC_BANNER (which should preferably be
   normalised by zero padding in the last word)... */

#define CC_BANNERlen ((strlen(CC_BANNER) + 4L) & ~3L)

#endif /* version_h */
