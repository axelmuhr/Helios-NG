/*
 * $Header: /Chris/00/helios/msc/RCS/rdevinfo.h,v 2.0 91/08/21 18:07:21 chris
 * Exp Locker: chris $
 */

/*************************************************************************
**									**
**		       D E V I C E   S U P P O R T			**
**		       ---------------------------			**
**									**
**	     Copyright (C) 1989, Perihelion Ltd, Parsytec GmbH		**
**			  All Rights Reserved.				**
**									**
**									**
** rdevinfo.h								**
**									**
**	- Definitions and prototyping					**
**									**
**************************************************************************
** HISTORY  :								**
** ----------								**
** Author   :	04/01/90  C. Fleischer					**
*************************************************************************/


#ifndef	__rdevinfo_h
#define	__rdevinfo_h

#include <helios.h>
#include <syslib.h>
#include <module.h>
#include <device.h>
#include <string.h>

/*------------------------  Local prototypes  --------------------------*/

void           *load_devinfo (void);
InfoNode       *find_info (void *devinfo, word type, char *name);

#endif

/*--- end of rdevinfo.h ---*/
