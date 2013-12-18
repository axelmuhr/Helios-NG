#include <device.h>
#include <syslib.h>
#include <nonansi.h>
#include <queue.h>
#include <sem.h>
#include <codes.h>
#include <attrib.h>

#include "ttydev.h"

typedef struct
{
	DCB		dcb;
	Stream		*read;
	Stream		*write;
} TermDCB;

word
DoRead(TermDCB *dcb, TermReq *req)
{
	word timeout = req->DevReq.Timeout;
	word evil = timeout == -1;
	word tstep = OneSec / 40;
	word actual = 0;

	while (	
		((actual = Read(dcb->read, (char *)req->Buf, req->Size, 0)) == 0)
		&&
		(evil || timeout >= 0)
	      )
	{
		Delay(tstep);
		timeout -= tstep;
	}

	return actual;	
}

void DevOperate(TermDCB *dcb, TermReq *req)
{
	switch(req->DevReq.Request)
	{
	case FG_Read:
/*		IOdebug("Terminal Read %x %d",req->Buf,req->Size); */
		req->Actual = DoRead(dcb, req);
		req->DevReq.Result = 0;
		break;		
		
	case FG_Write:
/*		IOdebug("Terminal Write buf %x size %d",req->Buf,req->Size); */
		req->Actual = Write(dcb->write, (char *)req->Buf, req->Size, req->DevReq.Timeout);
		req->DevReq.Result = 0;
		break;
		
	case FG_SetInfo:
/*		IOdebug("Terminal SetInfo called"); */
		req->DevReq.Result = -1;
		return;

	case FG_GetInfo:
/*		IOdebug("Terminal GetInfo called"); */
		req->DevReq.Result = -1;
		return;

	default:
		req->Actual = 0;
		req->DevReq.Result = EC_Error + SS_Device + EG_WrongFn;
		break;
	}
	(*req->DevReq.Action)(dcb,req);
	return;
}

word DevClose(TermDCB *dcb)
{
	Free(dcb);
	return (0);
}


TermDCB *DevOpen(MPtr dev, TermDeviceInfo *info)
{
	TermDCB *dcb;
	
	dcb = (TermDCB *)Malloc(sizeof(TermDCB));
	
   	if( dcb == NULL ) return NULL;

	dcb->read = info->read;
	dcb->write = info->write;

	if ( (dcb->read == NULL) || (dcb->write == NULL) )
	{
		Free (dcb);
		return (NULL);
	}
		
	dcb->dcb.Device = dev;
	dcb->dcb.Operate = DevOperate;
	dcb->dcb.Close = DevClose;

	return (dcb);
}
