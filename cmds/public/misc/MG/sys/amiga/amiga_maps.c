/*
 * Name:	MG 2a
 *		keymap.c setup for Amiga-specific function keys.
 * Created:	14-May-1988 Mic Kaczmarczik (mic@emx.utexas.edu)
 * Last edit:	14-May-1988 Mic Kaczmarczik
 *
 * One big table for coding convenience, because the number of different
 * compilation possibilities makes setting up data structures kind of
 * tricky.  Once again for convenience, the file is #included into keymap.c
 * instead of being linked.  I tried to minimize the changes to keymap.c,
 * while making it possible to have the Amiga function/arrow keys in dired
 * mode as well as fundamental mode.  In the future, we might want to
 * rebind some of the function keys in dired mode, but for right now they
 * do the same thing as in fundamental mode.
 */

static	PF	amiga_keys[] = {
#ifdef	FKEYS
	backline,	/* Up			(0x100)	*/
	forwline,	/* Down				*/
	backchar,	/* Left				*/
	forwchar,	/* Right			*/
	gotobop,	/* Shift-Up			*/
	gotoeop,	/* Shift-Down			*/
	backword,	/* Shift-Left			*/
	forwword,	/* Shift-Right			*/
	desckey,	/* Help			(0x108)	*/
#else
	/* 9 unbound keys */
	rescan, rescan, rescan, rescan, rescan, rescan, rescan, rescan, rescan, 
#endif

#ifdef	DO_MENU
	amigamenu,	/* Menu selction	(0x109)	*/
#else
	rescan,		/* Menu selection	(0x109)	*/
#endif
	refresh,	/* Resize window	(0x10A)	*/
	rescan,		/* used to be Mouse		*/

#ifdef	FKEYS
	filevisit,	/* F1			(0x10C)	*/
	filesave,	/* F2				*/
	forwpage,	/* F3				*/
	enlargewind,	/* F4				*/
	fillpara,	/* F5				*/
	splitwind,	/* F6				*/
	twiddle,	/* F7				*/
	definemacro,	/* F8				*/
	executemacro,	/* F9				*/
	listbuffers,	/* F10				*/
	poptofile,	/* Shift-F1		(0x116)	*/
	filewrite,	/* Shift-F2			*/
	backpage,	/* Shift-F3			*/
	shrinkwind,	/* Shift-F4			*/
	queryrepl,	/* Shift-F5			*/
	onlywind,	/* Shift-F6			*/
	justone,	/* Shift-F7			*/
	finishmacro,	/* Shift-F8			*/
	wallchart,	/* Shift-F9			*/
	quit,		/* Shift-F10		(0x11F)	*/
#else
	rescan, rescan, rescan, rescan, rescan,	/* 20 unbound keys */
	rescan, rescan, rescan, rescan, rescan,
	rescan, rescan, rescan, rescan, rescan,
	rescan, rescan, rescan, rescan, rescan,
#endif

#ifdef	MOUSE
	amigamouse,	/* Mouse		(0x120)	*/
	mforwdel,	/* Ctrl-Mouse			*/
	mreposition,	/* Shift-Mouse			*/
	mdelwhite,	/* Shift-Ctrl-Mouse		*/
	mdelfword,	/* Meta-Mouse			*/
	mkillregion,	/* Meta-Ctrl-Mouse		*/
	mkillline,	/* Meta-Shift-Mouse		*/
	myank,		/* Meta-Shift-Ctrl-Mouse	*/
	mforwpage,	/* Mode-Mouse		(0x128)	*/
	mgotobob,	/* Ctrl-Mode-Mouse		*/
	mbackpage,	/* Shift-Mode-Mouse		*/
	mgotoeob,	/* Shift-Ctrl-Mode-Mouse	*/
	msplitwind,	/* Meta-Mode-Mouse		*/
	menlargewind,	/* Meta-Ctrl-Mode-Mouse		*/
	mdelwind,	/* Meta-Shift-Mode-Mouse	*/
	mshrinkwind,	/* Meta-Shift-Ctrl-Mode-Mouse	*/
#ifdef	DO_ICONIFY
	tticon,		/* Echo-Mouse		(0x130)	*/
#else
	usebuffer,	/* Echo-Mouse		(0x130)	*/
#endif
	spawncli,	/* Ctrl-Echo-Mouse		*/
	killbuffer,	/* Shift-Echo-Mouse		*/
	quit,		/* Shift-Ctrl-Echo-Mouse	*/
	desckey,	/* Meta-Echo-Mouse		*/
	listbuffers,	/* Meta-Ctrl-Echo-Mouse		*/
	wallchart,	/* Meta-Shift-Echo-Mouse	*/
	togglewindow,	/* Meta-Shift-Ctrl-Echo-Mouse	*/
#endif	MOUSE
};


/*
 * Define extra maps for fundamental mode.  Have to provide the number of
 * extra map segments because it's used by the KEYMAPE() macro that builds
 * keymaps.  The keymap setup, while compact, is pretty complex...
 */

#define	NFUND_XMAPS	1

#ifdef	MOUSE
#define	FUND_XMAPS	{KUP,	KEASCMOUSE,	amiga_keys,	(KEYMAP*)NULL}
#else
#define	FUND_XMAPS	{KUP,	KSF10,		amiga_keys,	(KEYMAP*)NULL}
#endif

/*
 * Extra map segments for dired mode -- just use fundamental mode segments
 */
#define	NDIRED_XMAPS	NFUND_XMAPS
#define	DIRED_XMAPS	FUND_XMAPS

