/* ttyio.c -- Low-level Atari ST terminal input/output handling
 *
 * author :  Sandra Loosemore (from an earlier version by dec-rex!conroy)
 * date   :  24 Oct 1987
 * changes:  Marion Hakanson -- Jan 1988
 *
 */

#include	"..\..\def.h"

int	nrow;				/* Terminal size, rows.		*/
int	ncol;				/* Terminal size, columns.	*/

#ifdef DO_METAKEY
#define RSHIFT   (0x01)
#define LSHIFT	 (0x02)
#define CTRL	 (0x04)
#define ALTKEY	 (0x08)
#define CAPSLOCK (0x10)

static struct keytab {
    char *unshift;
    char *shift;
    char *capslock;
    } *keytable;


/* Mess with the bit that controls whether we get back all the shift keys
 *	on each keystroke.
 */

static unsigned char oldconterm;
static unsigned char *conterm = (char *)0x00000484;
 
static savect ()
{   oldconterm = *conterm;
    *conterm = (oldconterm | 0x8);
    }

static restct ()
{   *conterm = oldconterm;
    }
#endif /* DO_METAKEY */


/* Setup routines.  "getnrow" and "getncol" are assembly language routines.
 */

VOID ttopen()
{   nrow = getnrow();
    if (nrow > NROW)
	nrow = NROW;
    ncol = getncol();
    if (ncol > NCOL)
	ncol = NCOL;
    tcinsl = tcdell = (nrow * 2) + (ncol / 10);
#ifdef DO_METAKEY
    (VOID) Supexec(savect);
    keytable = (struct keytab *)Keytbl(-1L,-1L,-1L);
#endif /* DO_METAKEY */
    }

VOID ttclose()
{
#ifdef DO_METAKEY
    (VOID) Supexec(restct);
#endif /* DO_METAKEY */
    }


/* Keystrokes are returned as 10-bit quantities.
 *
 * Codes 0-255 are used for the usual character set.
 * Codes 256-511 are used for function keys.
 * Bit 10 (0x200) is the meta bit.
 *
 */
#ifdef FKEYS

static int keycodes[] = {
	0x00,	/* dummy for F0 entry */
	0x3b,	0x3c,	0x3d,	0x3e,	0x3f,
	0x40,	0x41,	0x42,	0x43,	0x44,
	0x54,	0x55,	0x56,	0x57,	0x58,
	0x59,	0x5a,	0x5b,	0x5c,	0x5d,
	0x62,	0x61,	0x52,	0x48,	0x47,
	0x4b,	0x50,	0x4d
        };

char *keystrings[] = {
"",		/* dummy for F0 entry */
"F1",		"F2",		"F3",		"F4",		"F5",
"F6",		"F7",		"F8",		"F9",		"F10",
"F11",		"F12",		"F13",		"F14",		"F15",
"F16",		"F17",		"F18",		"F19",		"F20",
"Help",		"Undo",		"Insert",	"Up",		"Clr/Home",
"Left",		"Down",		"Right"
};
#endif /* FKEYS */

/* Use the ALT key as a meta key.  The problem with this is that it
 *	appears to trash the ascii key code in the low order byte.
 *	Therefore, we have to resort to looking up the key in the
 *	system keystroke translation table.
 * Some non-US keyboards apparently use some ALT combinations to
 *	get real, printing characters.  If you've got one of these
 *	beasts you can use meta-key-mode to turn off recognition
 *	of the ALT key, in which case this routine just returns
 *	whatever BIOS gave as the key value.  If that approach is
 *	distasteful, you can also bind a function key to return
 *	the ALT characters usurped by the meta key mode.
 */

KCHAR getkbd()
{   register int code, bchar;
#ifdef FKEYS
    register int i;
#endif /* FKEYS */
#ifdef DO_METAKEY
    register int shifts;
    extern int use_metakey;		/* set in the generic kbd.c */
#endif /* DO_METAKEY */
    union {
        unsigned long k_rawkey;
        struct {
            unsigned char kb_shifts;	/* shift bits */
            unsigned char kb_code;	/* scan code */
            unsigned char kb_notused;
            unsigned char kb_bchar;	/* bios char */
        } k_break;
    } keyval;
 
    keyval.k_rawkey = Bconin(2);
    code   = keyval.k_break.kb_code;

#ifdef FKEYS
    for (i=KFIRST; i<=KLAST; i++) 	  	 /* Was it an Fkey? */
        if (code == keycodes[i-KFIRST])
	    return((KCHAR)i);
#endif /* FKEYS */

    bchar  = keyval.k_break.kb_bchar;
#ifdef DO_METAKEY
    shifts = keyval.k_break.kb_shifts;

    /*
     * Rule out the case where some shift bit other than what
     * we're interested in is set (if such a beast ever exists).
     * If otherwise, just forget about any special handling here.
     */
    if (use_metakey == TRUE &&
            (shifts & ~(CTRL|LSHIFT|RSHIFT|CAPSLOCK)) == ALTKEY)
        if ((shifts & (CTRL|LSHIFT|RSHIFT|CAPSLOCK)) == 0) /* ALTKEY only */
	    return ((KCHAR)(keytable->unshift[code] | METABIT));
        else if (shifts & CTRL)
	    return ((KCHAR)(bchar | METABIT));
        else if (shifts & (LSHIFT|RSHIFT))
	    return ((KCHAR)(keytable->shift[code] | METABIT));
        else /* (shifts & CAPSLOCK) */
	    return ((KCHAR)(keytable->capslock[code] | METABIT));
    else
        return ((KCHAR)bchar);
#else
    return ((KCHAR)bchar);
#endif
    }
 
/* Establish default keypad bindings.  I've only done the arrow keys
 *	here.
 */


VOID ttykeymapinit() { 
/*  excline("global-set-key f13 previous-line"); */	/* Up arrow */
/*  excline("global-set-key f15 backward-char"); */ 	/* Left arrow */  
/*  excline("global-set-key f16 next-line"); */		/* Down arrow */
/*  excline("global-set-key f17 forward-char"); */		/* Right arrow */
    }


#ifndef NO_DPROMPT
/*
 * Return TRUE if we busy-wait for 2 seconds without any keystrokes;
 * otherwise return FALSE.  See sleep() in misc.c for details.
 */

ttwait() {
    extern long read200hz();
    register keyhit;
    register long waitfor = 400L + Supexec(read200hz);

    while ((keyhit = typeahead()) == 0 && Supexec(read200hz) < waitfor);
    if (keyhit)
        return FALSE;

    return TRUE;
    }
#endif /* NO_DPROMPT */


