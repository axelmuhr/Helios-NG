/*
 * $Header: /dsl/HeliosRoot/Helios/cmds/public/stevie/RCS/keymap.h,v 1.1 1993/08/06 15:17:14 nickc Exp $
 *
 * Keycode definitions for special keys
 *
 * On systems that have any of these keys, the routine 'inchar' in the
 * machine-dependent code should return one of the codes here.
 */

#define	K_HELP		0x80
#define	K_UNDO		0x81
#define	K_INSERT	0x82
#define	K_HOME		0x83
#define	K_UARROW	0x84
#define	K_DARROW	0x85
#define	K_LARROW	0x86
#define	K_RARROW	0x87
#define	K_CCIRCM	0x88	/* control-circumflex */

#define	K_F1		0x91	/* function keys */
#define	K_F2		0x92
#define	K_F3		0x93
#define	K_F4		0x94
#define	K_F5		0x95
#define	K_F6		0x96
#define	K_F7		0x97
#define	K_F8		0x98
#define	K_F9		0x99
#define	K_F10		0x9a

#define	K_SF1		0xa1	/* shifted function keys */
#define	K_SF2		0xa2
#define	K_SF3		0xa3
#define	K_SF4		0xa4
#define	K_SF5		0xa5
#define	K_SF6		0xa6
#define	K_SF7		0xa7
#define	K_SF8		0xa8
#define	K_SF9		0xa9
#define	K_SF10		0xaa
