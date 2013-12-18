/*
 * Name:	MicroEMACS
 *		Amiga console device virtual terminal header file
 * Version:	MG 2a
 * Last edit:	28-Nov-87 ...!seismo!ut-sally!ut-ngp!mic (mic@emx.cc.utexas.edu)
 * Created:	20-Apr-86 ...!seismo!ut-sally!ut-ngp!mic
 */

#define	GOSLING				/* Compile in fancy display. */
#define	TOP_OFFSET	11		/* # raster lines from top of window */

#ifndef	NROW
#define	NROW		51		/* Max rows (interlaced screen)	  */
#endif

#ifndef	NCOL
#define	NCOL		85		/* Max cols (MoreRows, borderless) */
#endif

#ifndef LR_BORDER
#define	LR_BORDER (3 + 20)		/* Vertical border size (pixels)  */
#endif

#ifndef TB_BORDER
#define	TB_BORDER (TOP_OFFSET + 2)	/* Horizontal border size (pixels)*/
#endif

#ifndef	INIT_ROWS
#define	INIT_ROWS 24			/* Desired initial window height  */
#endif

#ifndef	INIT_COLS
#define	INIT_COLS 80			/* Desired initial window width	  */
#endif

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

#define	KW___MOUSE	0x120
#define	KW__CMOUSE	0x121
#define	KW_S_MOUSE	0x122
#define	KW_SCMOUSE	0x123
#define	KWA__MOUSE	0x124
#define	KWA_CMOUSE	0x125
#define	KWAS_MOUSE	0x126
#define	KWASCMOUSE	0x127
#define	KM___MOUSE	0x128
#define	KM__CMOUSE	0x129
#define	KM_S_MOUSE	0x12A
#define	KM_SCMOUSE	0x12B
#define	KMA__MOUSE	0x12C
#define	KMA_CMOUSE	0x12D
#define	KMAS_MOUSE	0x12E
#define	KMASCMOUSE	0x12F
#define	KE___MOUSE	0x130
#define	KE__CMOUSE	0x131
#define	KE_S_MOUSE	0x132
#define	KE_SCMOUSE	0x133
#define	KEA__MOUSE	0x134
#define	KEA_CMOUSE	0x135
#define	KEAS_MOUSE	0x136
#define	KEASCMOUSE	0x137

#define	KLAST	KEASCMOUSE

/*
 * Mouse key encoding stuff...  The bit fields are:
 *
 *		   4 3	   2 	  1	 0
 *		| where	| ALT | SHIFT | CTRL
 *
 * Where ALT, SHIFT, and CTRL indicate qualifiers, and the 2-bit
 * where field indicates whether the click was (initially) in a window,
 * a mode line, or the echo line.  The mouse functions are smart enough
 * to remap themselves if necessary; we implement these as keys so
 * users can rebind things to their taste.
 */
#define	M_X_ZERO	' '
#define	M_Y_ZERO	' '
#define	MQ_OFFSET	0x40
#define	MQ_NOQUAL	0x00
#define	MQ_CTRL		0x01
#define	MQ_SHIFT	0x02
#define	MQ_ALT		0x04
#define	MQ_WINDOW	0x00
#define	MQ_MODE		0x08
#define	MQ_ECHO		0x10
#define	MQ_WHERE(m)	(m & 0x18)	/* get where field */
#define	MQ_QUALS(m)	(m & 0x07)	/* get qualifier field */

/*
 * Intuition menu interface.  Each set of menu items kept in a table of
 * MenuBinding structures, which is in turn kept in a table of MenuInfo
 * structures. These tables are indexed via the menu and item numbers to
 * find the internal extended name of the function associated with a
 * certain item.
 */
#define	MN_OFFSET	' '		/* menu char - ' ' = real code */
struct MenuBinding {
	char	*Command;
	int	(*Function)();
};

struct MenuInfo {
	char *Name;			/* name of menu			*/
	short NumItems;			/* # of items			*/
	struct MenuBinding *Items;	/* item name, internal binding	*/
};

#define NITEMS(arr) (sizeof(arr) / (sizeof(arr[0])))

/*
 * If either MENU, or BROWSER is defined, we need to define
 * DO_MENU to get the code for dealing with menu selections
 * compiled in.
 */

#ifdef	MENU
#define	DO_MENU
#else
#ifdef	BROWSER
#define	DO_MENU
#endif	BROWSER
#endif	MENU

/*
 * MODE_RENDITION and TEXT_RENDITION determine the way the mode line and
 * text area are rendered (using the SGR sequence).  TEXT_* and MODE_* set
 * the foreground (FG) and background (BG) color to the specified number.
 * If you* #define CHANGE_COLOR, you can redefine these dynamically.
 */

#ifndef MODE_RENDITION
#define	MODE_RENDITION 7
#endif

#ifndef TEXT_RENDITION
#define	TEXT_RENDITION 0
#endif

#ifndef	TEXT_FG
#define TEXT_FG 1
#endif

#ifndef TEXT_BG
#define TEXT_BG 0
#endif

#ifndef	MODE_FG
#define MODE_FG 1
#endif

#ifndef	MODE_BG
#define MODE_BG 0
#endif

/*
 * Return the width and height of
 * the default font for a window.
 */

#define	FontWidth(w) (w)->RPort->TxWidth
#define	FontHeight(w) (w)->RPort->TxHeight
