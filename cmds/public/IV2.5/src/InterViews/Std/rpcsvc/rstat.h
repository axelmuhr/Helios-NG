/*
 * C++ interface to Unix remote stat information
 */

#ifndef rpcsvc_rstat_h

#define rstat rstat_h_rstat

#include "//usr/include/rpcsvc/rstat.h"

#undef rstat

/* just in case standard header didn't */
#ifndef rpcsvc_rstat_h
#define rpcsvc_rstat_h
#endif

extern int rstat(const char*, statstime*);

#endif
