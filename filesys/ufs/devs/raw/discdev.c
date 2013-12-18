/*
 * Raw disc device driver support.
 *
 * $Id: discdev.c,v 1.1 1992/09/16 10:45:36 al Exp $
 * $Log: discdev.c,v $
 * Revision 1.1  1992/09/16  10:45:36  al
 * Initial revision
 *
 */

#include <syslib.h>
#include <device.h>
#include <codes.h>
#include <root.h>
#include <gsp.h>
#include <module.h>
#include <string.h>

#define MAX_TFR		(32*1024)	/* max server tfr size		*/
#define MAXUNITS 8			/* 8 partitions			*/

word addint(char *s, word i);

typedef struct Unit {
	Stream		*Server;	/* stream to disc server	*/
	char		*streamname;	/* base of partition file name	*/
} Unit;

/* Device Control Block */
typedef struct DiscDCB {
	DCB		DCB;		/* standard DCB			*/
	Semaphore	Lock;		/* serializing lock		*/
	Unit		unit[MAXUNITS];
} DiscDCB;

void DevOperate(DiscDCB *dcb, DiscReq *req)
{
	word size = req->Size;
	word actual;
	word done = 0;
	word error = Err_Null;
	byte *buf = req->Buf;
	word i = req->DevReq.SubDevice;
	Object *o;
	Stream *s;

	Wait(&dcb->Lock); 	/* serialize access to the server */	

#ifdef DEBUG
IOdebug("raw: devoperate on subdevice %d fn 0x%x",
		i,req->DevReq.Request & FG_Mask);
#endif

	if ((i < 0) || (i >= MAXUNITS) || 
	    ((dcb->unit[i].Server == NULL) && 
	     ((req->DevReq.Request & FG_Mask) != FG_Open))) {
		/* set error/result */
		error = SS_Device|EC_Error|EG_Unknown|EO_Medium;
		goto done;
	}


	switch(req->DevReq.Request & FG_Mask)	{
	case FG_Read:
		req->Actual = 0;
		dcb->unit[i].Server->Pos = req->Pos;
		while( done < size )
		{
			word tfr = size - done;
			if( tfr > MAX_TFR ) tfr = MAX_TFR;

#ifdef DEBUG
IOdebug("raw: FG_Read: Unit %d, Pos %d tfr %d",i,dcb->unit[i].Server->Pos,tfr);
#endif
			actual = Read(dcb->unit[i].Server,buf,tfr,req->DevReq.Timeout);

			if( actual != tfr ) {
				error = Result2(dcb->unit[i].Server);
#ifdef DEBUG
IOdebug("raw: FG_Read: Unit %d, actual %d error 0x%x",i,actual,error);
#endif
				break;
			}
			
			req->Actual += tfr;
			done += tfr;
			buf += tfr;
		}
		break;
		
	case FG_Write:
		req->Actual = 0;
		dcb->unit[i].Server->Pos = req->Pos;
		while( done < size )
		{
			word tfr = size - done;
			if( tfr > MAX_TFR ) tfr = MAX_TFR;
			
#ifdef DEBUG
IOdebug("raw: FG_Write: Unit %d, Pos %d tfr %d",i,dcb->unit[i].Server->Pos,tfr);
#endif
			actual = Write(dcb->unit[i].Server,buf,tfr,req->DevReq.Timeout);

			if( actual != tfr ) {
				error = Result2(dcb->unit[i].Server); 
#ifdef DEBUG
IOdebug("raw: FG_Write: Unit %d, actual %d error 0x%x",i,actual,error);
#endif
				break;
			}
			
			req->Actual += tfr;
			done += tfr;
			buf += tfr;
		}
		break;

	case FG_Seek:
		error = Seek(dcb->unit[i].Server,S_Beginning,
		                   req->Pos);
		if( error == -1 )
			error = Result2(dcb->unit[i].Server);
		break;

	case FG_Open: {
		char rawname[20];

		if (dcb->unit[i].Server) break;	/* Already Open */

		dcb->unit[i].streamname = "/rawdisk";
		strcpy(rawname,dcb->unit[i].streamname);
		strcat(rawname,"/");
		if (i == 0) strcat(rawname,"0");
		else addint(rawname,i);

		o = Locate(NULL,rawname);
	
		/* Rawdisk does not exist */
		if (o == NULL) {
			error=SS_Device|EC_Error|EG_Unknown|EO_Medium;
			break;
		}

		s = Open(o,NULL,O_ReadWrite|O_Truncate);
		if (s == NULL) {
			error=SS_Device|EC_Error|EG_Invalid|EO_Medium;
			break;
		}
		dcb->unit[i].Server = s;
		Close(o);
#ifdef DEBUG
IOdebug("raw: devopen subdevice %d opened",i);
#endif	
	}
		break;
	case FG_Close:
		Close(dcb->unit[i].Server);
		dcb->unit[i].Server = NULL;
#ifdef DEBUG
IOdebug("raw: devclose subdevice %d closed",i);
#endif	
		break;
	case FG_SetInfo:
		/* Nothing to do since this is handled by the I/O Server */
		break;
	case FG_GetSize:
		error = GetFileSize(dcb->unit[i].Server);
		break;			
	case FG_Format:
	    {
		FormatReq *freq = (FormatReq *)req;
		IOdebug("rawdisk format start %d end %d int %d ts %d cs %d",
			freq->StartCyl,freq->EndCyl,freq->Interleave,
			freq->TrackSkew,freq->CylSkew);
	    }
		break;
	default:
		error = SS_Device|EC_Error|EG_WrongFn|EO_Medium;
		break;
	}

done:	
	Signal(&dcb->Lock);		/* relinquish Lock	*/
	
	if( error == -1 )
		error = Result2(dcb->unit[i].Server);
	req->DevReq.Result = error;	/* set error/result		*/

	(*req->DevReq.Action)(req);	/* call back to client		*/
#ifdef DEBUG
IOdebug("raw: devoperate error 0x%x subdevice %d fn 0x%x",
		error,i,req->DevReq.Request & FG_Mask);
#endif
}

word DevClose(DiscDCB *dcb)
{	int i;

	Wait(&dcb->Lock);
	
	for(i=0; i<MAXUNITS; i++)
		if (dcb->unit[i].Server)
			Close(dcb->unit[i].Server);

	Free(dcb);
	return(Err_Null);
}

DiscDCB *DevOpen(Device *dev, DiscDevInfo *info)
{
	DiscDCB *dcb = Malloc(sizeof(DiscDCB));
	int j;			/* Unit count */

	if (dcb == NULL) return NULL;

	/* Setup server table */
	for (j=0; j<MAXUNITS; j++)
		dcb->unit[j].Server = NULL;

	dcb->DCB.Device = dev;
	dcb->DCB.Operate = DevOperate;
	dcb->DCB.Close = DevClose;
	InitSemaphore(&dcb->Lock,1);

	return(dcb);
}

word addint(char *s, word i)
{	
  int len;

  if (i == 0) return strlen(s);
  len = addint(s,i/10);
  s[len] = (i%10) + '0';
  s[len+1] = '\0';
  return(len+1);
}

