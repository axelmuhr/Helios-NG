/*************************************************************************
**									**
**	       T E R M I N A L   W I N D O W   S E R V E R		**
**	       -------------------------------------------		**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** ttydev.c								**
**									**
**	- Helios specific device interface for Terminal Window Server	**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	12/09/90 : G. Jodlauk					**
*************************************************************************/

#include "tty.h"

#define		TIOCPKT_FLUSHWRITE	0x02

extern DCB *termdcb;
extern int terminating;

/******************************************************/
/* The req structures are dynamic variables and hence */
/* the device must not queue the requests and return! */
/******************************************************/


void device_reqdone(DCB *dcb, TermReq *req)
{
	dcb=dcb,req=req;
}


word device_read(void *buf, int size, int timeout)
{
	TermReq	req;
	
#if 0
	IOdebug("TTY: device_read(%x, %d, %d)", 
		buf, size, timeout);
#endif
	unless (termdcb && (terminating == FALSE))
		return (0);

	req.DevReq.Request = FG_Read;
	req.DevReq.Action = device_reqdone;
	req.DevReq.SubDevice = 0;
	req.DevReq.Timeout = timeout;
	req.Buf = buf;
	req.Size = size;

	Operate(termdcb, &req);

#if 0
	IOdebug("TTY: device_read(%x, %d, %d) = %x",
		buf, size, timeout,
		req.DevReq.Result != 0 ? req.DevReq.Result : req.Actual);
#endif
	return (req.DevReq.Result != 0 ? req.DevReq.Result : req.Actual);
}


word device_write(char *buf, int size, int timeout)
{
	TermReq	req;
	
#if 0
	IOdebug("TTY: device_write(%x,%d,%d)",
		buf, size, timeout);
#endif
	unless (size && termdcb && (terminating == FALSE))
		return (0);

	req.DevReq.Request = FG_Write;
	req.DevReq.Action = device_reqdone;
	req.DevReq.SubDevice = 0;
	req.DevReq.Timeout = timeout;
	req.Buf = buf;
	req.Size = size;

	Operate(termdcb, &req);
	
#if 0
	IOdebug("TTY: device_write(%x,%d,%d) = %x", 
		buf, size, timeout,
		req.DevReq.Result != 0 ? req.DevReq.Result : req.Actual);
#endif
	return (req.DevReq.Result != 0 ? req.DevReq.Result : req.Actual);
}


/****************************************************************/
/* The TermInfoReq used below are static and can be used 	*/
/* to call tha DevReq.Action routine for every request.		*/
/****************************************************************/

void device_set_info_done(DCB *dcb, TermInfoReq *req)
{
	Attributes *a = &req->TermInfo.Attr;

	DeviceWindowSetInfo (a);
}


void device_get_info_done(DCB *dcb, TermInfoReq *req)
{
	Attributes *a = &req->TermInfo.Attr;

	DeviceWindowGetInfo (a);
}


word device_init_info_req(void)
{
	TermInfoReq *get;
	TermInfoReq *set;

	get = (TermInfoReq *) Malloc(sizeof(TermInfoReq));
	if (get == NULL)
		return -1;

	set = (TermInfoReq *) Malloc(sizeof(TermInfoReq));
	if (set == NULL)
	  {
	    Free( get );
	    return -1;
	  }
	
	    
	set->DevReq.Request = FG_SetInfo;
	set->DevReq.Action = device_set_info_done;
	Operate(termdcb, set);

	get->DevReq.Request = FG_GetInfo;
	get->DevReq.Action = device_get_info_done;
	Operate(termdcb, get);
	
	return 0;
}
