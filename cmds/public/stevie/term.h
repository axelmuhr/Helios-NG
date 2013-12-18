/* $Header: /dsl/HeliosRoot/Helios/cmds/public/stevie/RCS/term.h,v 1.1 1993/08/06 15:17:14 nickc Exp $
 *
 * System-dependent escape sequence definitions.
 */

#ifdef	TERMCAP

extern char *T_EL;		/* erase the entire current line */
extern char *T_IL;		/* insert one line */
extern char *T_DL;		/* delete one line */
extern char *T_SC;		/* save the cursor position */
extern char *T_ED;		/* erase display (may optionally home cursor) */
extern char *T_RC;		/* restore the cursor position */
extern char *T_CI;		/* invisible cursor (very optional) */
extern char *T_CV;		/* visible cursor (very optional) */

extern char *T_CM;		/* cursor motion string */

#else

/*
 * This file contains the machine dependent escape sequences that
 * the editor needs to perform various operations. Some of the sequences
 * here are optional. Anything not available should be indicated by
 * a null string. In the case of insert/delete line sequences, the
 * editor checks the capability and works around the deficiency, if
 * necessary.
 *
 * Currently, insert/delete line sequences are used for screen scrolling.
 * There are lots of terminals that have 'index' and 'reverse index'
 * capabilities, but no line insert/delete. For this reason, the editor
 * routines s_ins() and s_del() should be modified to use 'index'
 * sequences when the line to be inserted or deleted line zero.
 */

/*
 * The macro names here correspond (more or less) to the actual ANSI names
 */

#ifdef	ATARI
#ifdef	MINIX

#define	T_EL	"\033[2K"	/* erase the entire current line */
#define	T_IL	"\033[L"	/* insert one line */
#define	T_DL	"\033[M"	/* delete one line */
#define	T_SC	"\0337"		/* save the cursor position */
#define	T_ED	"\033[2J"	/* erase display (may optionally home cursor) */
#define	T_RC	"\0338"		/* restore the cursor position */
#define	T_CI	""		/* invisible cursor (very optional) */
#define	T_CV	""		/* visible cursor (very optional) */

#else

#define	T_EL	"\033l"		/* erase the entire current line */
#define	T_IL	"\033L"		/* insert one line */
#define	T_DL	"\033M"		/* delete one line */
#define	T_SC	"\033j"		/* save the cursor position */
#define	T_ED	"\033E"		/* erase display (may optionally home cursor) */
#define	T_RC	"\033k"		/* restore the cursor position */
#define	T_CI	"\033f"		/* invisible cursor (very optional) */
#define	T_CV	"\033e"		/* visible cursor (very optional) */

#endif
#endif

#ifdef	UNIX
/*
 * The UNIX sequences are hard-wired for ansi-like terminals. I should
 * really use termcap/terminfo, but the UNIX port was done for profiling,
 * not for actual use, so it wasn't worth the effort.
 */
#define	T_EL	"\033[K"	/* erase the entire current line */
#define	T_IL	"\033[L"	/* insert one line */
#define	T_DL	"\033[M"	/* delete one line */
#define	T_ED	"\033[1;1H\033[J"	/* erase display (may optionally home cursor) */
#define	T_SC	""		/* save the cursor position */
#define	T_RC	""		/* restore the cursor position */
#define	T_CI	""		/* invisible cursor (very optional) */
#define	T_CV	""		/* visible cursor (very optional) */
#endif

#ifdef	OS2
/*
 * The OS/2 ansi console driver is pretty deficient. No insert or delete line
 * sequences. The erase line sequence only erases from the cursor to the end
 * of the line. For our purposes that works out okay, since the only time
 * T_EL is used is when the cursor is in column 0.
 *
 * The insert/delete line sequences marked here are actually implemented in
 * the file os2.c using direct OS/2 system calls. This makes the capability
 * available for the rest of the editor via appropriate escape sequences
 * passed to outstr().
 */
#define	T_EL	"\033[K"	/* erase the entire current line */
#define	T_IL	"\033[L"	/* insert one line - fake (see os2.c) */
#define	T_DL	"\033[M"	/* delete one line - fake (see os2.c) */
#define	T_ED	"\033[2J"	/* erase display (may optionally home cursor) */
#define	T_SC	"\033[s"	/* save the cursor position */
#define	T_RC	"\033[u"	/* restore the cursor position */
#define	T_CI	""		/* invisible cursor (very optional) */
#define	T_CV	""		/* visible cursor (very optional) */
#endif


#ifdef	DOS
/*
 * DOS sequences
 *
 * Some of the following sequences require the use of the "nansi.sys"
 * console driver. The standard "ansi.sys" driver doesn't support
 * sequences for insert/delete line.
 */
#define	T_EL	"\033[K"	/* erase the entire current line */
#define	T_IL	"\033[L"	/* insert line (requires nansi.sys driver) */
#define	T_DL	"\033[M"	/* delete line (requires nansi.sys driver) */
#define	T_ED	"\033[2J"	/* erase display (may optionally home cursor) */
#define	T_SC	"\033[s"	/* save the cursor position */
#define	T_RC	"\033[u"	/* restore the cursor position */
#define	T_CI	""		/* invisible cursor (very optional) */
#define	T_CV	""		/* visible cursor (very optional) */
#endif

#endif

/*
 * Machine-variant screen handling definitions.
 *
 * Define some macros which for invoking screen functions, whether by
 * callling a bios function or outputting an escape sequence to be
 * interpreted by a PC console driver or terminal.
 *
 * At this writing, not all of Stevie has been converted to use these
 * macros.  So far, only DOS and PC BIOS versions are completely converted.
 * Other versions are partly converted (because of changes I made in Stevie's
 * common code), but they have not been tested.  I'll convert others which I'm
 * in a position to test, but I'll leave any I can't test alone.  Hopefully,
 * this will minimize any damage to working versions which I can't test. -LAS
 */

#ifdef BIOS

#define	CANDL		TRUE		/* Can delete lines */
#define	CANIL		TRUE		/* Can insert lines */
#define	CLEOL		bios_t_el()	/* Erase to end-of-line */
#define	CLS		bios_t_ed()	/* Erase entire display */
#define	CRTDL(r,l)	bios_t_dl(r,l)	/* Delete lines from display */
#define	CRTIL(r,l)	bios_t_il(r,l)	/* Insert lines in display */
#define	CUROFF		bios_t_ci()	/* Make cursor invisible */
#define	CURON		bios_t_cv()	/* Make cursor visible */
#define	RESCUR		bios_t_rc()	/* Restore saved cursor position */
#define	SAVCUR		bios_t_sc()	/* Save cursor position */

#else		/* Not BIOS */

#define	CANDL		(T_DL[0]!='\0')	/* Determine if can delete lines */
#define	CANIL		(T_IL[0]!='\0')	/* Determine if can insert lines */
#define	CRTDL(r,l)	DO_DL(r,l)	/* Delete lines from display */
#define	CRTIL(r,l)	DO_IL(r,l)	/* Insert lines in display */
#ifdef TERMCAP
#define	CLEOL		tputs(T_EL,1,outchar)	/* Erase to end-of-line */
#define	CLS		tputs(T_ED,1,outchar)	/* Erase entire display */
#define	CUROFF		tputs(T_CI,1,outchar)	/* Make cursor invisible */
#define	CURON		tputs(T_CV,1,outchar)	/* Make cursor visible */
#define	RESCUR		tputs(T_RC,1,outchar)	/* Restore saved cursor position */
#define	SAVCUR		tputs(T_SC,1,outchar)	/* Save cursor position */
#else
#define	CLEOL		outstr(T_EL)	/* Erase to end-of-line */
#define	CLS		outstr(T_ED)	/* Erase entire display */
#define	CUROFF		outstr(T_CI)	/* Make cursor invisible */
#define	CURON		outstr(T_CV)	/* Make cursor visible */
#define	RESCUR		outstr(T_RC)	/* Restore saved cursor position */
#define	SAVCUR		outstr(T_SC)	/* Save cursor position */
#endif

#define	DO_DL(r,l) {\
	int __xx_knt = l;\
	while (__xx_knt-- > 0) {outstr(T_DL);}\
}

#define	DO_IL(r,l) {\
	int __xx_knt = l;\
	while (__xx_knt-- > 0) {outstr(T_IL);}\
}

#endif		/* Not BIOS */


