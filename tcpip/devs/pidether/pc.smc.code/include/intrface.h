/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1989
*******************************************************************************
******************************************************************************/

/************************************************************************

this is the definitions header file for the screen interface portion 
	of the sample driver

*************************************************************************/

#define	ESCAPE		0x1B		/* ASCII for ESC & kbd code for ESC */
#define	RETURN		13		/* keyboard code for 'return' */
#define	SPACE		32		/* keyboard code for 'space' */
#define	EXT_ASCII	00		/* flags an extended kbd code */
#define	UP_CURSOR	72		/* Extended kbd code for up arrow */
#define	DOWN_CURSOR	80		/* Extended kbd code for down arrow */
#define	SCREEN_WIDTH	80		/* Width of visual screen */
#define	BELL		0x07		/* code to ring the bell */

/*** Graphics character definitions ***/
#define	RGT_CONNECT	0xB9		/* connect char for right side */
#define	VERT_BAR	0xBA		/* vertical bar char */
#define	TOP_RGT_CORNER	0xBB		/* upper right corner */
#define	BTM_RGT_CORNER	0xBC		/* lower right corner */
#define	BTM_LFT_CORNER	0xC8		/* lower left corner */
#define	TOP_LFT_CORNER	0xC9		/* upper left corner */
#define	BTM_CONNECT	0xCA		/* connect char for bottom */
#define	TOP_CONNECT	0xCB		/* connect char for top */
#define	LFT_CONNECT	0xCC		/* connect char for left side */
#define	HORIZ_BAR	0xCD		/* horizontal bar char */
#define	CENTER_CONNECT	0xCE		/* cross connect char for center */

/*** Screen attributes ***/
#define	ALL_OFF		0
#define	BOLD		1
#define	UNDERSCORE	4
#define	BLINK		5
#define	REVERSE		7
#define	INVISIBLE	8

/*** Graphics color definitions ***/
#define	FOR_BASE	30		/* base for foreground colors */
#define	BACK_BASE	40		/* base for background colors */

#define	BLACK		0		/* add these to the base for a color */
#define	RED		1
#define	GREEN		2
#define	YELLOW		3
#define	BLUE		4
#define	MAGENTA		5
#define	CYAN		6
#define	WHITE		7

/*** Menu field definitions ***/
#define	MODE_FIELD		0
#define	ITERATION_FIELD		1
#define	FRAME_LENGTH_FIELD	2
#define	SRC_ADDR_FIELD		3
#define	RESPONDER_FIELD		4
#define	IO_MODE_FIELD		5

