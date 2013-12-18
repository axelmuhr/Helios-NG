/* salib.h:	Stand-Alone C header			*/
/* $Id: salib.h,v 1.1 1990/11/21 18:46:45 nick Exp $ */

#include <helios.h>

/* extras for stand-alone system */

extern void *memtop(void);
extern int memtest(word *base, int size);
extern void *malloc_fast(word size);
extern void free_fast(void *addr);

extern int mctype(void);

extern void exit(int);
