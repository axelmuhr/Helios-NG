/*
 * Name:	MG 2x
 *		Directory I/O routines, by Stephen Walton
 *		Version of 5-May-88
 */

#ifndef NO_DIR

#include "sysdef.h"
#include <libraries/dosextens.h>
#include <exec/memory.h>

extern	char		MyDirName[MAXPATH], *strncat();

char *getwd(path)
char *path;
{
	strcpy(path,MyDirName);
	return path;
}

chdir(path)
char *path;
{
	BPTR Lock(), AttemptLock, CurrentDir();
	long PathName(), len;
	struct FileInfoBlock *fib;
	void *AllocMem();
	int retval;

	AttemptLock = Lock(path, ACCESS_READ);
	if (!AttemptLock)
		return -1;
	fib = (struct FileInfoBlock *) AllocMem((long)
					        sizeof(struct FileInfoBlock),
						MEMF_CLEAR);
	Examine(AttemptLock, fib);
	if (fib->fib_DirEntryType < 0) {
		retval = -1;
		UnLock(AttemptLock);
		goto clean;
	}
	UnLock(CurrentDir(AttemptLock));	/* do the thing		*/
	if (PathName(AttemptLock, MyDirName, MAXPATH/31L) == 0)
		MyDirName[0] = '\0';
	retval = 0;				/* Success!		*/
    clean:
	FreeMem((void *) fib, (long) sizeof(struct FileInfoBlock));
	return retval;
}
#endif
