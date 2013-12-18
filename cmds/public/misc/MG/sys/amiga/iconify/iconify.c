/*  :ts=8 bk=0
 *
 * iconify.c:	You asked for it, you got it.
 *
 * Copyright 1987 by Leo L. Schwab.
 * Permission is hereby granted for use in any and all programs, both
 * Public Domain and commercial in nature, provided this Copyright notice
 * is left intact.  Purveyors of programs, at their option, may wish observe
 * the following conditions (in the spirit of hackerdom):
 *	1: You send me a free, registered copy of the program that uses the
 *	   iconify feature,
 *	2: If you're feeling really nice, a mention in the program's
 *	   documentation of my name would be neat.
 *
 *			 		8712.10		(415) 456-3960
 */
#include <exec/types.h>
#include <devices/timer.h>
#include <intuition/intuition.h>
#include "iconify.h"

/*
 * It is recommended that the tick rate not be made too rapid to avoid
 * bogging down the system.
 */
#define	TICKS_PER_SECOND	10

/*
 * Some programmers may not wish to have the added functionality of the
 * ICON_FUNCTION feature.  If you're such a programmer, you may comment out
 * the following #define, which will eliminate the code to handle function
 * calls, and make iconify() even smaller.
 */
#define	USE_FUNCTIONS

/*
 * Jim Mackraz suggested making icons easily identifiable by outside
 * programs, so this constant gets stuffed into the UserData field.
 */
#define	ICON	0x49434f4eL		/*  'ICON'  */


extern void	*OpenWindow(), *GetMsg(), *CreatePort(), *CreateExtIO(),
		*CheckIO();
extern long	OpenDevice(), DoubleClick();


static struct Gadget gadget = {
	NULL,
	0, 0, 0, 0,
	NULL,				/*  Set later  */
	GADGIMMEDIATE,
	WDRAGGING,			/*  Observe the Magic!  */
	NULL,				/*  Set later  */
	NULL, NULL, NULL, NULL,
	0, 0
};

static struct NewWindow windef = {
	0, 0, 0, 0,			/*  Set later  */
	-1, -1,
	GADGETDOWN,
	BORDERLESS | SMART_REFRESH | NOCAREREFRESH,
	&gadget,
	NULL, NULL, NULL, NULL,		/*  Lotsa these  */
	0, 0, 0, 0,
	WBENCHSCREEN
};

static struct Window		*win;

#ifdef USE_FUNCTIONS
static struct timerequest	*tr;
static struct MsgPort		*reply;
#endif


iconify (left, top, width, height, screen, ptr, type)
UWORD *left, *top, width, height;
struct Screen *screen;
APTR ptr;
int type;
{
	register struct IntuiMessage	*msg;
	long				secs = 0, mics = 0,
					cs, cm,
					class,
					sigmask;

	windef.LeftEdge		= *left;
	windef.TopEdge		= *top;
	windef.Width		= width;
	windef.Height		= height;
	windef.Type = (windef.Screen = screen) ? CUSTOMSCREEN : WBENCHSCREEN;

	gadget.Flags		= GADGHCOMP | GRELWIDTH | GRELHEIGHT;

	switch (type & 3) {
	case ICON_IMAGE:
		gadget.Flags		|= GADGIMAGE;
	case ICON_BORDER:
		gadget.GadgetRender	= ptr;
		break;

	case ICON_FUNCTION:
#ifdef USE_FUNCTIONS
		gadget.GadgetRender	= NULL;
#else
		return (0);
#endif
		break;

	default:
		return (0);
	}

	if (!openstuff ())
		return (0);
	sigmask = 1L << win -> UserPort -> mp_SigBit;

#ifdef USE_FUNCTIONS
	if (type == ICON_FUNCTION) {
		sigmask |= 1L << reply -> mp_SigBit;
		tr -> tr_node.io_Command= TR_ADDREQUEST;
		tr -> tr_time.tv_secs	= 0;
		tr -> tr_time.tv_micro	= (1000000L / TICKS_PER_SECOND);
		SendIO (tr);
		/*
		 * Make initialization call to user's function.
		 * Isn't typecasting wonderful?  :-|
		 */
		(* ((void (*)()) ptr)) (win, (WORD) 1);
	}
#endif

	while (1) {
		Wait (sigmask);

#ifdef USE_FUNCTIONS
		if (GetMsg (reply)) {
			/*
			 * Call user's function to do something to the icon.
			 */
			(* ((void (*)()) ptr)) (win, (WORD) 0);
			tr -> tr_time.tv_secs	= 0;
			tr -> tr_time.tv_micro	=
			 (1000000L / TICKS_PER_SECOND);
			SendIO (tr);
		}
#endif

		if (msg = GetMsg (win -> UserPort)) {
			class = msg -> Class;
			cs = msg -> Seconds;
			cm = msg -> Micros;
			ReplyMsg (msg);

			if (class == GADGETDOWN) {
				if (DoubleClick (secs, mics, cs, cm))
					break;
				secs = cs;  mics = cm;
			}
		}
	}

#ifdef USE_FUNCTIONS
	if (type == ICON_FUNCTION) {
		AbortIO (tr);
		WaitIO (tr);
	}
#endif

	*left = win -> LeftEdge;
	*top = win -> TopEdge;
	closestuff ();
	return (1);
}

static
openstuff ()
{
	if (!(win = OpenWindow (&windef)))
		return (0);
	win -> UserData = (BYTE *) ICON;
		
#ifdef USE_FUNCTIONS
	if (!(reply = CreatePort (NULL, NULL)) ||
	    !(tr = CreateExtIO (reply, (long) sizeof (*tr))) ||
	    OpenDevice (TIMERNAME, UNIT_VBLANK, tr, 0L)) {
		closestuff ();
		return (0);
	}
#endif

	return (1);
}

static
closestuff ()
{
#ifdef USE_FUNCTIONS
	if (tr) {
		if (tr -> tr_node.io_Device)
			CloseDevice (tr);
		DeleteExtIO (tr, (long) sizeof (*tr));
	}
	if (reply)		DeletePort (reply);
#endif

	if (win)		CloseWindow (win);
}
