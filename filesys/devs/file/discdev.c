#include <syslib.h>
#include <device.h>
#include <codes.h>
#include <root.h>
#include <gsp.h>
#include <module.h>

#define MAX_TFR		(32*1024)	/* max server tfr size		*/
#define MAXUNITS 2			/* disc and tape		*/

word addint(char *s, word i);

typedef struct Unit {
	Stream		*Server;	/* stream to disc server	*/
	word		BlockSize;	/* units of Pos and Size fields	*/
	char		*streamname;	/* base of partition file name	*/
} Unit;

/* Device Control Block */
typedef struct DiscDCB {
	DCB		DCB;		/* standard DCB			*/
	Semaphore	Lock;		/* serializing lock		*/
	int		nunits;
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

	Wait(&dcb->Lock); 	/* serialize access to the server */	

	switch( req->DevReq.Request & FG_Mask )
	{
	case FG_Read:
		req->Actual = 0;
		dcb->unit[i].Server->Pos = req->Pos*dcb->unit[0].BlockSize;
		size *= dcb->unit[i].BlockSize;
		while( done < size )
		{
			word tfr = size - done;
			if( tfr > MAX_TFR ) tfr = MAX_TFR;

			actual = Read(dcb->unit[i].Server,buf,tfr,req->DevReq.Timeout);

			if( actual != tfr ) 
			{ error = Result2(dcb->unit[i].Server); break; }
			
			req->Actual += tfr;
			done += tfr;
			buf += tfr;
		}
		break;
		
	case FG_Write:
		req->Actual = 0;
		dcb->unit[i].Server->Pos = req->Pos*dcb->unit[i].BlockSize;
		size *= dcb->unit[i].BlockSize;
		while( done < size )
		{
			word tfr = size - done;
			if( tfr > MAX_TFR ) tfr = MAX_TFR;
			
			actual = Write(dcb->unit[i].Server,buf,tfr,req->DevReq.Timeout);

			if( actual != tfr ) 
			{ error = Result2(dcb->unit[i].Server); break; }
			
			req->Actual += tfr;
			done += tfr;
			buf += tfr;
		}
		break;

	case FG_Seek:
		error = Seek(dcb->unit[i].Server,S_Beginning,
		                   req->Pos*dcb->unit[i].BlockSize);
		if( error == -1 )
			error = Result2(dcb->unit[i].Server);
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
	}

done:	
	Signal(&dcb->Lock);		/* relinquish Lock	*/
	
	if( error == -1 )
		error = Result2(dcb->unit[i].Server);
	req->DevReq.Result = error;	/* set error/result		*/

	(*req->DevReq.Action)(req);	/* call back to client		*/
}

word DevClose(DiscDCB *dcb)
{	int i;

	Wait(&dcb->Lock);
	
	for(i =0; i<dcb->nunits; i++)
		Close(dcb->unit[i].Server);

	return Err_Null;
}

DriveInfo *finddrive(DiscDevInfo *info, int n)
{
	DriveInfo *di = (DriveInfo *)RTOA(info->Drives);
	while(n--)
		di = (DriveInfo *)RTOA(di->Next);
	return di;
}

DiscDCB *DevOpen(Device *dev, DiscDevInfo *info)
{
	DiscDCB *dcb = Malloc(sizeof(DiscDCB));
	PartitionInfo *pi;
	int i = 0;		/* Unit count */
	if( dcb == NULL ) return NULL;

	dcb->unit[0].streamname = "/d/filesys";
	dcb->unit[1].streamname = "/helios/tmp/tape";

	pi = (PartitionInfo *)RTOA(info->Partitions);

	while(1)
	{	Object *o;
		Stream *s;
		DriveInfo *di = finddrive(info,i);

		o = Locate(NULL,dcb->unit[i].streamname);
	
#ifdef IODEBUG
	IOdebug("Drive id for unit %d is %d",i,di->DriveId);
#endif
		if( o == 0 )	/* create tape if not present */
		{	char mcname[100];
			Object *mo;
			if( i == 0 ) return NULL;
			MachineName(mcname);
			mo = Locate(NULL,mcname);
			o = Create(mo,dcb->unit[i].streamname,
					Type_File,0,0);
			Close(mo);
			if( o == NULL ) return 0;
		}
		if( o->Type == Type_Directory )
		{
			char rawname[20];
			strcpy(rawname,dcb->unit[i].streamname);
			strcat(rawname,"/");
			if( di->DriveId == 0 ) strcat(rawname,"0");
			else
			  addint(rawname,di->DriveId);

			o = Locate(NULL,rawname);
		}
	
		if( o == NULL ) return NULL;

		s = Open(o,NULL,O_ReadWrite);

		if( s == NULL ) { Close(o); return NULL; }
		dcb->unit[i].Server = s;
	
		/* The only Devinfo field we are interested in is the	*/
		/* Addressing. All the others are for the Server.	*/
		
		dcb->unit[i].BlockSize = info->Addressing;
		Close(o);
	
		if( pi->Next == -1 ) break;
		pi = (PartitionInfo *)RTOA(pi->Next);
		i++;
	}
	dcb->DCB.Device = dev;
	dcb->DCB.Operate = DevOperate;
	dcb->DCB.Close = DevClose;
	
		
	InitSemaphore(&dcb->Lock,1);
		
		
	return dcb;
}

word addint(char *s, word i)
{	
  int len;
  if( i == 0 ) return strlen(s);

  len = addint(s,i/10);
  
  s[len] = (i%10) + '0';
  
  s[len+1] = '\0';
  
  return len+1;
}

