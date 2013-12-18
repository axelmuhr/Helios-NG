/*************************************************************************
**									**
**	       T E R M I N A L   W I N D O W   S E R V E R		**
**	       -------------------------------------------		**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** ttydev.h								**
**									**
**	- Device interface definitions for Terminal Window Server	**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	12/09/90 : G. Jodlauk					**
*************************************************************************/

typedef struct TermReq
{
        DevReq		DevReq;         /* device request               */
	word		Size;		/* tfr size			*/
	void		*Buf;		/* buffer			*/
	word		Actual;		/* data actually transferred	*/
} TermReq;


typedef struct TermInfo
{
	Attributes	Attr;
} TermInfo;

typedef struct TermInfoReq
{
        DevReq          DevReq;         /* device request               */
        TermInfo	TermInfo;	/* info                         */
} TermInfoReq;


typedef	struct TermDeviceInfo
{
	char		*NTE_Name;
	Stream		*read;
	Stream		*write;
} TermDeviceInfo;


/* end of ttydev.h */

