/*
 * Name:	MicroEMACS
 *		Atari 520ST header file.
 * Version:	30
 * Last edit:	22-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
 
#define	NROW	25			/* The "50" is big enough to	*/
#define	NCOL	80			/* deal with the "hi50" screen.	*/
 
/* These i/o functions are NOP's or direct equivalents of BIOS calls.
 *	Make them #define's so we don't have to go through a useless
 *	level of indirection.
 */

#define ttinit()
#define tttidy()
#define ttwindow(top,bot) (top, bot)
#define ttresize()
#define ttnowindow()

/*
 * Function key codes (using 16-bit KEY values)
 */
#define	KFIRST	0x100

#define	KUP	0x100
#define	KDOWN	0x101
#define KLEFT	0x102
#define	KRIGHT	0x103

#define	KSUP	0x104
#define	KSDOWN	0x105
#define	KSLEFT	0x106
#define	KSRIGHT	0x107

#define	KHELP	0x108
#define	KMENU	0x109
#define	KRESIZE	0x10A

#define	KF1	0x10C
#define	KF2	0x10D
#define KF3	0x10E
#define	KF4	0x10F
#define KF5	0x110
#define KF6	0x111
#define KF7	0x112
#define KF8	0x113
#define KF9	0x114
#define KF10	0x115

#define	KSF1	0x116
#define	KSF2	0x117
#define KSF3	0x118
#define	KSF4	0x119
#define KSF5	0x11A
#define KSF6	0x11B
#define KSF7	0x11C
#define KSF8	0x11D
#define KSF9	0x11E
#define KSF10	0x11F

#define	KLAST	KSF10

