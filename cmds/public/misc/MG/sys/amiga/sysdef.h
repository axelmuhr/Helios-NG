/*
 * Name:	MicroEMACS
 * Version:	MG 2a
 *		Commodore Amiga system header file.
 */

/* Neither can lattice 4 */
extern char *offset_dummy;		/* Manx 3.2 can't handle 0->	*/
#define OFFSET(type,member) \
 ((char *)&(((type *)offset_dummy)->member)-(char *)((type *)offset_dummy))

#ifdef	MANX
#define	PCC	0			/* "[]" works.			*/
#else
#define	PCC	1			/* "[]" does not work.		*/
#endif

#define	VARARGS
#define	DPROMPT				/* we always want delayed prompts */
#define	KBLOCK	4096			/* Kill grow.			*/
#define	GOOD	0			/* Good exit status.		*/
#define	SYSINIT	sysinit()		/* System-specific initialization */
#define SYSCLEANUP syscleanup()		/* System-specific cleanup	*/
#define MALLOCROUND(m)	(m+=7,m&=~7)	/* Round up to 8 byte boundary	*/
#define NULL	((char *) 0)		/* These are crass, but		*/ 
#define	EOF	-1			/* will work`			*/

/*
 * Macros used by the buffer name making code.
 * Start at the end of the file name, scan to the left
 * until BDC1 (or BDC2, if defined) is reached. The buffer
 * name starts just to the right of that location, and
 * stops at end of string (or at the next BDC3 character,
 * if defined). BDC2 and BDC3 are mainly for VMS.
 */
#define	BDC1	':'			/* Buffer names.		*/
#define	BDC2	'/'


/*
 * Typedefs for internal key type and how big a region can be.
 */

typedef short	KCHAR;	/* type used to represent Emacs characters */
typedef	long	RSIZE;	/* size of a region	*/

#define	MAXPATH	128	/* longest expected directory path	*/

#define	bcopy(src,dest,len) movmem(src,dest,len)

#define fncmp Strcmp

#ifndef NO_DIRED
#define rename(s1,s2) (Rename(s1,s2) == -1 ? 0 : -1)
#define unlinkdir(s1) (DeleteFile(s1) == -1 ? 0 : -1)
#endif
