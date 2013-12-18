/*************************************************************************
**									**
**		     S Y S T E M   D E B U G G I N G			**
**		     -------------------------------			**
**									**
**		    Copyright (C) 1990, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** diags.h								**
**									**
**	- SetDiags and GetDiags prototypes				**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	20/04/90 : C. Fleischer					**
*************************************************************************/

#ifndef __diags_h
#define __diags_h

#ifndef __helios_h
#include <helios.h>
#endif

word		SetDiags	( char *server, word diags );
word		GetDiags	( char *server, word *diags );

#endif

/*--- end of diags.h ---*/
