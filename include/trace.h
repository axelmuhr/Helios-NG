/* trace.h:	Stand-Alone C header			*/
/* $Id: trace.h,v 1.1 1990/11/21 18:46:51 nick Exp $ */

extern void _TraceInit(void);
extern void _Mark(void);
extern void _Trace(word x,...);
extern void _Halt(void);
