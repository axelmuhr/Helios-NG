/*************************************************************************
**									**
**		     S Y S T E M   D E B U G G I N G			**
**		     -------------------------------			**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** debug.h								**
**									**
**	- Debug macro definitions and function prototypes		**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	29/11/89 : C. Fleischer					**
*************************************************************************/

#ifndef __debug_h
#define __debug_h

#define	ERROR		0xffffffff

#define Debug(lvl)	if ((lvl) & DEBUG_LEVEL) DoDebug

#ifdef	DRIVER
#define	DoDebug	IOdebug
#else

extern int	DEBUG_LEVEL;

void		DoDebug (char *fmt, ...);
void		DebugInit (void);

#endif

#endif

/*--- end of debug.h ---*/
