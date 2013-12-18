/*
 * Name:	MG 2a
 * 		Amiga virtual terminal keyboard, default console keymap.
 * Created:	Mic Kaczmarczik (mic@emx.cc.utexas.edu)
 * Last edit:	May 14, 1988
 */

#undef	TRUE
#undef	FALSE
#include	"def.h"
#include	"kbd.h"

/*
 * List of function key names, from KFIRST to KLAST
 */
#ifdef	FKEYS
char	*keystrings[] = {
	"Up",		"Down",		"Left",		"Right",
	"Shift-Up",	"Shift-Down",	"Shift-Left",	"Shift-Right",
	"Help",		"The menu",	"The resize gadget", "The mouse",
	"F1",		"F2",		"F3",		"F4",
	"F5",		"F6",		"F7",		"F8",
	"F9",		"F10",		"Shift-F1",	"Shift-F2",
	"Shift-F3",	"Shift-F4",	"Shift-F5",	"Shift-F6",
	"Shift-F7",	"Shift-F8",	"Shift-F9",	"Shift-F10",
	"Mouse",			"Ctrl-Mouse",
	"Shift-Mouse",			"Shift-Ctrl-Mouse",
	"Meta-Mouse",			"Meta-Ctrl-Mouse",
	"Meta-Shift-Mouse",		"Meta-Shift-Ctrl-Mouse",
	"Mode-Mouse",			"Ctrl-Mode-Mouse",
	"Shift-Mode-Mouse",		"Shift-Ctrl-Mode-Mouse",
	"Meta-Mode-Mouse",		"Meta-Ctrl-Mode-Mouse",
	"Meta-Shift-Mode-Mouse",	"Meta-Shift-Ctrl-Mode-Mouse",
	"Echo-Mouse",			"Ctrl-Echo-Mouse",
	"Shift-Echo-Mouse",		"Shift-Ctrl-Echo-Mouse",
	"Meta-Echo-Mouse",		"Meta-Ctrl-Echo-Mouse",
	"Meta-Shift-Echo-Mouse",	"Meta-Shift-Ctrl-Echo-Mouse"
};
#endif

/*
 * Read in a key, doing whatever low-level mapping of ASCII code to
 * 11 bit code.  This has become a bit easier since keymaps.
 */
#define	CSI	0x9b

getkbd()
{
	register int c;
#ifdef	FKEYS
	register int	n;
#endif
loop:
	if ((c = ttgetc()) == CSI) {
		c = ttgetc();
#ifdef	FKEYS
		if (c == '?') {			/* HELP key		*/
			ttgetc();		/* discard '~'		*/
			return (KHELP);
		}
		/* Arrow keys */
		if (c == 'A')
			return (KUP);
		if (c == 'B')
			return (KDOWN);
		if (c == 'C')
			return (KRIGHT);
		if (c == 'D')
			return (KLEFT);
		if (c == 'T')
			return (KSUP);
		if (c == 'S')
			return (KSDOWN);

		/* Shifted left, right arrow */
		if (c == ' ') {
			c = ttgetc();
			if (c == 'A' || c == '@')
				return ((c == 'A') ? (KSLEFT) : (KSRIGHT));
			goto loop;		/* try again, sucker */
		}

		/* Function keys	*/
		if (c >= '0' && c <= '9') {
			n = 0;
			do {
				n = 10*n + c - '0';
				c = ttgetc();
			} while (c>='0' && c<='9');
			if (c == '~' && n < 20)
				return (n < 9) ? (KF1 + n) : (KSF1 + (n - 10));
			else 
				goto loop;	/* Try again */
		}
#endif
		goto loop;		/* Try again */
	}
	return (c);
}

/*
 * Terminal specific keymap initialization, calling bind() to get
 * things done.  All the keys bound here are done globally.
 */

VOID
ttykeymapinit()
{
#ifdef	FKEYS
	KCHAR		c;
	register KEYMAP **mapp = &map_table[0].p_map;
#endif
	static KCHAR	esc_bs[] = { CCHR('['), CCHR('H') };
	static KCHAR	esc_del[] = { CCHR('['), CCHR('?') };
	
#define	BINDC(k,s) (c = k, bindkey(mapp, s, &c, 1))
#define	BINDM(m,s) bindkey(mapp, s, m, (sizeof(m)/sizeof(KCHAR)))
	
	/* Swap the backspace and del keys, at least in normal usage.
	 * This loses the help feature of CTRL-H, but we rebind
	 * CTRL-_ to do the same thing. Under FKEYS, the Help key
	 * calls describe-key-briefly.
	 */
	BINDC(CCHR('_'),	"help-help");	/* CTRL-Backspace */
	BINDM(esc_bs,		"backward-kill-word");
	BINDC(CCHR('H'),	"delete-backward-char");

	BINDC(CCHR('?'),	"delete-char");
	BINDM(esc_del,		"kill-word");
}

