/*************************************************************************
**									**
**	                  I O S   N U C L E U S              		**
**	                  ---------------------           		**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** dispatch.h								**
**									**
**	- Prototypes for the abortable dispatcher			**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	01/12/89 : C. Fleischer					**
*************************************************************************/


#include <syslib.h>
#include <servlib.h>

#define	Malloc	DispMalloc

void		*DispMalloc (word size);
void		Dispatcher (DispatchInfo *info);

/*--- end of dispatch.h ---*/

