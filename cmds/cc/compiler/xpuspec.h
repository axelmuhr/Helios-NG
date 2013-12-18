
/* C compiler file xpuspec.h :  Copyright (C) A.Mycroft and A.C.Norman */
/* version 0.01 */
/* $Id: xpuspec.h,v 1.1 1990/09/13 17:11:19 nick Exp $ */

#ifndef _XPUSPEC_LOADED

#define _XPUSPEC_LOADED TRUE

#undef TARGET_HAS_COND_EXEC         
#undef TARGET_HAS_SCALED_ADDRESSING 

#define TARGET_COUNT_IS_PROC          TRUE
#define TARGET_HAS_SWITCH_BRANCHTABLE TRUE

#define TARGET_HAS_MULTIPLY         TRUE

#define TARGET_LACKS_UNSIGNED_FIX   TRUE

/* I think this should really be true, but I then get confused ! */
#undef TARGET_HAS_OTHER_IEEE_ORDER 

/* The transputer will misbehave on large shifts, so mask them */
#define mcdep_fixupshift(n) ((n) & 255)

#endif

/* end of xpuspec.h */
