/* fault.h: Fault library interface	*/
/* $Id: fault.h,v 1.3 1993/07/06 13:30:58 nickc Exp $ */

#include <syslib.h>

#define FDBBUFMAX	1024	/* size of FDB buffer	*/

typedef struct FDB {
	Stream	*stream;	/* fault database file 	*/
	word	pos;		/* buffer pos		*/
	word	upb;		/* buffer upb		*/
	char	buf[FDBBUFMAX];	/* file buffer		*/
} FDB;

extern FDB *fdbopen(string name);
extern void fdbclose(FDB *fdb);
extern void fdbrewind(FDB *fdb);
extern int fdbfind(FDB *fdb, char *pClass, word code, char *text, word tsize);

extern void Fault(word code, char *msg, word msize);

/* end if fault.h */
