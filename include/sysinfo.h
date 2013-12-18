/* sysinfo.h:	Stand-Alone C header			*/
/* $Id: sysinfo.h,v 1.1 1990/11/21 18:46:49 nick Exp $ */

#include <helios.h>

typedef struct SysInfo
{
	byte	*freestore;		/* first byte of free store	*/
	word	*modtab;		/* program module table		*/
	word	bootlink;		/* number of boot link		*/
	word	*TraceVec;		/* address of trace vector	*/
} SysInfo;

#ifdef SysBase
#undef SysBase
#endif
#define SysBase ((word *)(MinInt+0x1000))


#define _SYSINFO ((SysInfo *)RTOA(SysBase[0]))
