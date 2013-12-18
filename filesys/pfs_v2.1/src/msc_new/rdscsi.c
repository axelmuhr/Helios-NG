/*
 * $Header: /Chris/00/helios/msc/RCS/rdscsi.c,v 2.0 91/08/21 18:07:34 chris
 * Exp Locker: chris $
 */

/*************************************************************************
**									**
**		    M S C   D I S C   D E V I C E			**
**		    -----------------------------			**
**									**
**		  Copyright ( C ) 1990, Parsytec GmbH			**
**			 All Rights Reserved.				**
**									**
**									**
** rdscsi.c								**
**									**
**	- Read SCSI Info						**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	xx/xx/90 : C. Fleischer					**
*************************************************************************/

#define __in_rdscsi	1		/* module name flag		 */

#ifndef	__helios_h			/* conditional include headers	 */
#include <helios.h>
#endif

#ifndef	__syslib_h
#include <syslib.h>
#endif

#ifndef	__module_h
#include <module.h>
#endif

#ifndef	__device_h
#include <device.h>
#endif

#ifndef	__string_h
#include <string.h>
#endif

#ifndef	__scsiinfo_h
#define	Structured
#include "scsiinfo.h"
#endif

#define RtoA(x,type)	(x) = ((( word )( x ) == -1 ) ? NULL : \
(( type ) (( word ) &( x ) + ( word )( x ))))

/*************************************************************************
 * CONVERT THE WHOLE SCSIINFO TO ABSOLUTE POINTERS
 *
 * Parameter  :	sinfo	= pointer to ScsiInfo structure
 *
 ************************************************************************/

void
ConvertScsiInfo (void *sinfo)
{
    InfoNode       *info;		/* InfoNode for RPTR conversion	 */
    ScsiDevInfo    *SDinfo;		/* other typed pointers		 */
    CmdInfo        *CMinfo;
    ErrorInfo      *ERinfo;
    CondInfo       *COinfo;
    ReqInfo        *RQinfo;
    ItemInfo       *ITinfo;

    info = (InfoNode *) ((Module *) sinfo + 1);
    do {
    /* Convert InfoNode entries	 */
	RtoA (info->Name, RPTR);
	RtoA (info->Info, RPTR);

    /* get ScsiDevInfo ptr		 */
	SDinfo = (ScsiDevInfo *) info->Info;
    /* convert ScsiDevInfo entries	 */
	RtoA (SDinfo->Ident, char *);
	RtoA (SDinfo->Commands, CmdInfo *);
	RtoA (SDinfo->Errors, ErrorInfo *);
	RtoA (SDinfo->Requests, ReqInfo *);

	CMinfo = SDinfo->Commands;	/* get CmdInfo ptr		 */
	while (CMinfo) {		/* for all CmdInfos		 */
	/* convert CmdInfo entries	 */
	    CMinfo->Next = RtoA (CMinfo->Next, CmdInfo *);
	    CMinfo->Name = RtoA (CMinfo->Name, char *);
	    CMinfo->CDB = RtoA (CMinfo->CDB, byte *);
	    CMinfo->Data = RtoA (CMinfo->Data, byte *);

	    CMinfo = CMinfo->Next;	/* step to next CmdInfo		 */
	}

	ERinfo = SDinfo->Errors;	/* get ErrorInfo ptr		 */
	while (ERinfo) {		/* for all ErrorInfos		 */
	/* convert ErrorInfo entries	 */
	    RtoA (ERinfo->Next, ErrorInfo *);
	    RtoA (ERinfo->Conditions, CondInfo *);


	    COinfo = ERinfo->Conditions;/* get CondInfo ptr		 */
	    while (COinfo) {		/* for all CondInfos		 */
	    /* convert Next ptr		 */
		RtoA (COinfo->Next, CondInfo *);
		COinfo = COinfo->Next;
	    }
	    ERinfo = ERinfo->Next;
	}

	RQinfo = SDinfo->Requests;	/* get ReqInfo ptr		 */
	while (RQinfo) {		/* for all ReqInfos		 */
	/* convert Next and Items ptr	 */
	    RtoA (RQinfo->Next, ReqInfo *);
	    RtoA (RQinfo->Items, ItemInfo *);

	    ITinfo = RQinfo->Items;	/* get ItemInfo ptr		 */
	    while (ITinfo) {		/* for all ItemInfos		 */
	    /* convert Next and Item ptr	 */
		RtoA (ITinfo->Next, ItemInfo *);
		RtoA (ITinfo->Item, CmdInfo *);
		ITinfo = ITinfo->Next;
	    }
	    RQinfo = RQinfo->Next;
	}
	info = (info->Next == 0) ? NULL :
	  (InfoNode *) (((word) & (info->Next)) + ((word) info->Next));
    }
    while (info);
}

/*************************************************************************
 * LOCATE AND LOAD THE SCSIINFO FILE
 *
 * Return     :	pointer to scsiinfo structure
 *		NULL	if the file was not found
 *
 ************************************************************************/

void           *
LoadScsiInfo (void)
{
    Object         *o;			/* DevInfo object		 */
    Stream         *s = NULL;		/* stream to DevInfo object	 */
    void           *sinfo = NULL;	/* pointer to target structure	 */
    int             size;		/* size of ScsiInfo structure	 */
    ImageHdr        hdr;		/* header of object		 */

/* try to find the ScsiInfo	 */
    o = Locate (NULL, "/rom/scsiinfo");	/* in rom		 */


    if (o == NULL)			/* try to find the ScsiInfo in	 */
	o = Locate (NULL, "/loader/ScsiInfo");	/* the nucleus	 */

    if (o == NULL)			/* not found, search on disk	 */
	o = Locate (NULL, "/helios/etc/scsiinfo");

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

    if ((sinfo = Malloc (size)) == NULL)/* and get a buffer.	 */
	goto done;

    if (Read (s, sinfo, size, -1) != size) {	/* read the structure.	 */
	Free (sinfo);
	sinfo = NULL;
    }
    ConvertScsiInfo (sinfo);
done:					/* that's all, close everything	 */
    Close (s);
    Close (o);

    return sinfo;
}

/*************************************************************************
 * FIND A SPECIFIC ENTRY IN THE SCSIINFO
 *
 * Parameter  :	sinfo	= pointer to ScsiInfo structure
 *		index	= number of desired Info node
 * Return     :		pointer to Info node
 *		NULL 	if node not found
 *
 ************************************************************************/

InfoNode       *
FindScsiInfo (void *sinfo, int index)
{
    InfoNode       *info = (InfoNode *) ((Module *) sinfo + 1);
    int             count = 0;

    while (info) {			/* loop until found or end	 */
	if (info->Type == Info_ScsiDev) {	/* right node type	 */
	    if (count == index)
		return info;		/* right node found.		 */
	    else
		count++;		/* count unused node		 */
	}
    /* and check next node		 */
	info = (info->Next == 0) ? NULL :
	  (InfoNode *) (((word) & (info->Next)) + ((word) info->Next));
    }
    return NULL;
}

/*--- end of rdscsi.c ---*/
