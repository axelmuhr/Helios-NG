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

#define FG_SetDiags	0x00001fe0	/* Function code to set dlevel	*/
#define FG_GetDiags	0x00001fe8	/* Function code to get dlevel	*/

#define	ERROR		0xffffffff

#define Debug(lvl)	if ((lvl) & DEBUG_LEVEL) DoDebug

#ifdef	DRIVER
#define	DoDebug	IOdebug
#else

#ifndef __servlib_h
#include <servlib.h>
#endif

extern int	DEBUG_LEVEL;

void		DoDebug		( char *fmt, ... );
void		DebugInit	( void );
void		Do_Diags	( ServInfo *srvinfo );
#endif

#endif

/*--- end of debug.h ---*/
