/* rrdtest.h : Header file for Robust Ram Disk test programs */

#ifndef _RRDTEST_H
#define _RRDTEST_H

#include <helios.h>

#define __in_dir 1	/* flag that we are in this module */

#include <posix.h>
#include <syslib.h>
#include <gsp.h>
#include <string.h>
#include <errno.h>

#undef __in_dir

#include <limits.h>

/* *******Bodge********
   perror won't link!!
   define it to be an empty statement for now 
*/

#undef perror
#define perror(x) ;

/*----------------------------------------------------------------------*/
/*                                                               Macros */
/*----------------------------------------------------------------------*/

#define MAXNUMFILES  128           /* Maximum number of files in a
				      directory */
#define MAXNUMDIRS   128           /* Maximum number of subdirectories
				      in a directory */
#define FINDUNUSEDFILETRIES 30     /* Number of tries before we give
				      up trying to find an unused filename
				      in a given directory (so that we don't 
				      loop endlessly when it is full). */
#define RELEASEMEMTRIES 256        /* Number of tries before we give up
				      trying to release a 'suitably large'
				      amount of memory by deleting files,
				      in the case when we believe that
				      the system has run out of memory */
#define RELEASEMULTIPLE 10         /* If we run out of memory, we go away
				      and delete lots of files until 
				      the sum of the lengths of all the files
				      that we have deleted is at least 
				      RELEASEMULTIPLE times 
				      SAFETYBLOCKSIZE */
#define ROOTDIR "/ram/sys/slash"   /* Root of filing system */
#define FILENAMESIZE NAME_MAX
#define PATHNAMESIZE PATH_MAX
#define BUFSIZE      256           /* Files are written in blocks of size
				      BUFSIZE */
#define CMDSIZE      256

#define SAFETYBLOCKSIZE 50000      /* We allocate a block of this size 
				      from the heap when we start up, 
				      so that if Malloc fails later we
				      can then free this block and have some
				      room to delete a few files from the 
				      RAM FS. */

#define MINMEMFREE	400000		/* always leave free */

/* The filesystem operations that we test are enumerated below.
   Tweaking the range assigned to each operation will alter the 
   frequency with which it is performed: the greater the range, 
   the more frequently.
*/
#define OPS_CREATE_FILE_LOW  0          /* Create a file */
#define OPS_CREATE_FILE_HIGH 4
#define OPS_APPEND_FILE_LOW  5          /* Append to a file */
#define OPS_APPEND_FILE_HIGH 9
#define OPS_DELETE_FILE_LOW  10         /* Delete a file */
#define OPS_DELETE_FILE_HIGH 14
#define OPS_CREATE_DIR_LOW   15         /* Create a directory */
#define OPS_CREATE_DIR_HIGH  19
#define OPS_DELETE_DIR_LOW   20         /* Delete a directory */
#define OPS_DELETE_DIR_HIGH  24
#define LAST_OP              24

#define DESCEND_RANGE   128
#define DESCEND_THRESH  32

#define VERI_FREQ       256

/* Return codes from file-verify routine */
#define FILE_OK      0
#define FILE_CORRUPT 1


/*----------------------------------------------------------------------*/
/*                                                             typedefs */
/*----------------------------------------------------------------------*/

typedef struct Directory_t
{
  char DirPathName[PATHNAMESIZE]; /* Full pathname of this directory      */
  int  DirLevel;                  /* Depth of directory in tree structure */
  int  DirNumFiles;               /* Number of files in this directory    */
  int  DirNumSub;                 /* Number of subdirectories of this dir */
} Directory;


/*----------------------------------------------------------------------*/
/*                                                     Global variables */
/*----------------------------------------------------------------------*/

extern void *safetyBlock;  /* 'Spare' memory pointer: we free the block    */
			   /* it points to if we run out of memory, delete */
			   /* some stuff, then re-allocate it & carry on   */
extern int  releasingMem;  /* Normally zero, this flag is set during the   */
			   /* process of deleting files when we've run out */
                           /* of memory, in order to release some.         */
extern int cautious, verbose;  /* Command line flags */

/*----------------------------------------------------------------------*/
/*                                                  External references */
/*----------------------------------------------------------------------*/

extern int       VerifyFS(void);
extern Directory *TreeWander(Directory *d);
extern int       FindExistingFileName(Directory *d, char *pathName);
extern void      CheckEntries(Directory *d,
			      int *numValidFiles, int *numValidDirs,
			      int *numInvalidFiles, int *numInvalidDirs);
extern int       TestFile(char *pathName, int level);
extern int       DeleteFile(Directory *d);
extern void      ReleaseMem(void);
#if 0
/* Presently unused ... */
extern int       FindExistingDirName(Directory *d, char *pathName);
#endif

#endif  /* _RRDTEST_H */
