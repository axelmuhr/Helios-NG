/*
 * $Header: /Chris/00/helios/msc/RCS/rdevinfo.c,v 2.0 91/08/21 18:07:28 chris
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
** rdevinfo.c								**
**									**
**	- GSP Interface for the Serial Line Server			**
**									**
**************************************************************************
** HISTORY  :								**
** ----------								**
** Author   :  	01/09/89  N. Garnett					**
** Modified :	04/01/90  C. Fleischer	- expanded to source module	**
*************************************************************************/

#define __in_rdevinfo	1		/* module name flag		 */

#include "rdevinfo.h"

/*************************************************************************
 * LOCATE AND LOAD THE DEVINFO FILE
 *
 * Parameter  :	- nothing -
 * Return     :		pointer to devinfo structure
 *
 ************************************************************************/

void           *
load_devinfo (void)
{
    Object         *o;			/* DevInfo object		 */
    Stream         *s = NULL;		/* stream to DevInfo object	 */
    void           *devinfo = NULL;	/* pointer to target structure	 */
    int             size;		/* size of DevInfo structure	 */
    ImageHdr        hdr;		/* header of object		 */

/*
 * o = Locate (NULL, "/rom/devinfo");	/@*@ try to locate the DevInfo	@*@/
 * /@*@ from rom			@*@/
 * 
 * if (o == NULL)    					o = Locate (NULL,
 * "/loader/DevInfo");	/@*@ try to locate	@*@/ /@*@ the DevInfo in the
 * nucleus	@*@/
 * 
 * if (o == NULL)  			/@*@ not found, search on disk	@*@/
 */
    o = Locate (NULL, "/helios/etc/devinfo");

    if (o == NULL)			/* still not found.		 */
	return NULL;

    s = Open (o, NULL, O_ReadOnly);	/* open a stream to the object	 */

    if (s == NULL) {			/* failed to open the stream.	 */
	Close (o);
	return NULL;
    }
    if (Read (s, (byte *) & hdr, sizeof (hdr), -1) != sizeof (hdr))
	goto done;			/* read the image header,	 */

    if (hdr.Magic != Image_Magic)	/* check the magic word		 */
	goto done;

    size = hdr.Size;			/* extract the structure size	 */

    if ((devinfo = Malloc (size)) == NULL)	/* and get a buffer.	 */
	goto done;

    if (Read (s, devinfo, size, -1) != size) {	/* read the structure.	 */
	Free (devinfo);
	devinfo = NULL;
    }
done:					/* that's all, close everything	 */
    Close (s);
    Close (o);

    return devinfo;
}


/*************************************************************************
 * FIND A SPECIFIC ENTRY IN THE DEVINFO
 *
 * Parameter  :	devinfo	= pointer to devinfo structure
 *		type	= type of desired Info node
 *		name	= name of the Info node
 * Return     :		pointer to Info node
 *		NULL 	if node not found
 *
 ************************************************************************/

InfoNode       *
find_info (void *devinfo, word type, char *name)
{
    InfoNode       *info = (InfoNode *) ((Module *) devinfo + 1);

    forever				/* loop until found or end	 */
    {
	if (strcmp (name, RTOA (info->Name)) == 0 && info->Type == type)
	    return info;		/* right node found.		 */

	if (info->Next == 0)
	    break;			/* end of DevInfo reached	 */

	info = (InfoNode *) RTOA (info->Next);	/* else check next node	 */
    }
    return NULL;
}
