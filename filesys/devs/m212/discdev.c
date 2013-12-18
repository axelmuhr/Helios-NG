
#include <syslib.h>
#include <device.h>
#include <codes.h>
#include <root.h>
#include <link.h>
#include <config.h>
#include <asm.h>

#define muldiv(a,b,c) (((a)*(b))/(c))

#define LinkVector	((Channel *)MinInt)
#define OutLink(n)	(&LinkVector[(n)])
#define InLink(n)	(&LinkVector[(n)+4])

#define Timeout (OneSec*5)

#ifndef DEBUG
#define LinkRx(s,l,b) in_(s,InLink(l),b)
#define LinkTx(s,l,b) out_(s,OutLink(l),b)
#else
void LinkRx(int size, int link, void *buf)
{
	int e = LinkIn(size,link,buf,Timeout);
	if( e != 0 )
		IOdebug("LinkRx(%d,%d,%x) error %x",size,link,buf,e);
}
void LinkTx(int size, int link, void *buf)
{
	int e = LinkOut(size,link,buf,Timeout);
	if( e != 0 )
		IOdebug("LinkTx(%d,%d,%x) error %x",size,link,buf,e);
}
#endif

#define RW			0

#define MULTI

#include "m212.h"

typedef struct DiscDCB {
	DCB		DCB;		/* standard DCB			*/
	word		Link;		/* M212 link			*/
	LinkConf	OldConf;	/* old link config		*/
	Semaphore	Lock;		/* serializing lock		*/
	word		MaxTfr;		/* maximum transfer size	*/
	word		BlockSize;	/* unit of read/write requests	*/
	word		ReadMode;	/* mode for read requests	*/
	word		WriteMode;	/* mode for write requests	*/
	word		Drives;		/* number of drives		*/
	word		Partitions;	/* number of partitions		*/
	DriveInfo	Drive[4];	/* client supplied info	on drives*/
	PartitionInfo	Partition[10];	/* client supplied info on partitions */
} DiscDCB;


int log2(int x)
{
	int i;
	for(i = 0; (x>>i)!=1; i++);
	return i;
}

/* M212 interface */

void WriteCommand(int link, int cmd, ... )
{
	int *data = &cmd + 1;
	LinkTx(1,link,&cmd);
	if( cmd == m2Initialise ) LinkTx(1,link,data);
}

void WriteParameter(int link, int param, int value )
{
	int cmd = m2WriteParameter;
	LinkTx(1,link,&cmd);
	LinkTx(1,link,&param);
	LinkTx(1,link,&value);
}

void WriteDoubleParameter(int link, int param, int value )
{
	WriteParameter(link,param,value&0xff);
	WriteParameter(link,param+1,value>>8);
}

void WriteTripleParameter(int link, int param, int value )
{
	WriteParameter(link,param,value&0xff);
	WriteParameter(link,param+1,(value>>8)&0xff);
	WriteParameter(link,param+2,(value>>16)&0xff);	
}

int ReadParameter(int link, int param)
{
	int cmd = m2ReadParameter;
	LinkTx(1,link,&cmd);
	LinkTx(1,link,&param);
	cmd = 0;
	LinkRx(1,link,&cmd);
	return cmd;
}

int ReadDoubleParameter(int link, int param)
{
	return (ReadParameter(link,param+1)<<8) | ReadParameter(link,param);
}

int ReadTripleParameter(int link, int param)
{
	return 	(ReadParameter(link,param+2)<<16) | 
		(ReadParameter(link,param+1)<<8)  | 	
		ReadParameter(link,param);
}

void InitM2(DiscDCB *dcb)
{
	DriveInfo *info;
	word link = dcb->Link;
	int drive;
	
	/* setup controller */
	WriteParameter	(link, m2ControllerAccess,m2ExternalWriteClock);
	
	for( drive = 0; drive < dcb->Drives; drive++ )
	{
		info = &dcb->Drive[drive];
		WriteParameter	(link, m2DesiredDrive, info->DriveId);
		WriteCommand	(link, m2SelectDrive);
		WriteCommand	(link, m2Initialise, info->DriveType);
		WriteCommand	(link, m2Restore);
		WriteParameter	(link, m2Addressing, m2LogicalAddressing|m2IncrementLogical|m2IncrementBuffer );
		WriteParameter	(link, m2SectorSizeLg2, log2(info->SectorSize) );
		WriteParameter	(link, m2NumberOfSectors, info->SectorsPerTrack );
		WriteParameter	(link, m2NumberOfHeads, info->TracksPerCyl );
		WriteDoubleParameter(link, m2NumberOfCylinders0, info->Cylinders);
		WriteParameter	(link, m2DesiredSectorBuffer, 0);
	}
	
	/* find out how much buffering the m212 has */
	dcb->MaxTfr = ReadParameter(link, m2BufferSize) * 256;

	WriteCommand	(link, m2EndOfSequence);
}

void DevOperate(DiscDCB *dcb, DiscReq *req)
{
	word pos = req->Pos;
	word size = req->Size*dcb->BlockSize;
	word link = dcb->Link;
	word done = 0;
	word error = Err_Null;
	byte *buf = req->Buf;
	word err = 0;
	word reason;
	PartitionInfo *pi = &dcb->Partition[req->DevReq.SubDevice];
	DriveInfo *info = &dcb->Drive[pi->Drive];

	if( 	req->DevReq.SubDevice < 0 || 
		req->DevReq.SubDevice >= dcb->Partitions )
	{
		error = EC_Error|EG_Parameter|5;
		goto done;
	}
	
	/* For the present we will do all disk transfers	*/
	/* synchronously. Later we will want to make this queue	*/
	/* requests and deal with them in a more optimal order.	*/
	/* That would require us to run a seperate process to	*/
	/* do the actual disc accesses.				*/
	
	Wait(&dcb->Lock);

	switch( req->DevReq.Request & FG_Mask )
	{
	case FG_Read:
#if RW
IOdebug("R%d,%d[%d]%",req->DevReq.SubDevice,req->Pos/4096,size/4096);
#endif
		req->Actual = 0;
	
		pos = muldiv(pos,dcb->BlockSize,info->SectorSize)+pi->StartSector;
#if RW
IOdebug("(%d[%d])%",pos,size/info->SectorSize);
#endif

		/* once the first command is accepted, we have control */
		WriteParameter	(link, m2DesiredDrive, info->DriveId);
		WriteCommand	(link, m2SelectDrive);
		WriteTripleParameter(link, m2LogicalSector,pos);
		WriteParameter	(link, m2DesiredSectorBuffer, 0);

		switch( dcb->ReadMode )
		{
		case 0:			/* M212 Mode 1		*/
		
		while( done < size )
		{
			word sectors, sect;
			word tfr = size - done;
			
			if( tfr > dcb->MaxTfr ) tfr = dcb->MaxTfr;
			sectors = tfr/info->SectorSize;
		
			/* setup and perform read */
			WriteParameter	(link, m2DesiredSectorBuffer, 0);	

			for( sect = 0; sect < sectors; sect++ )
				WriteCommand(link, m2ReadSector);
			
			err = ReadParameter(link, m2Error );
			
			if( err == 0 )
			{
				WriteParameter	(link, m2DesiredSectorBuffer, 0);
				for( sect = 0; sect < sectors; sect++ )
				{
					WriteCommand(link, m2ReadBuffer );
					LinkRx(info->SectorSize,link,buf);
					buf += info->SectorSize;
				}
			}
			req->Actual += tfr;
			done += tfr;
		}
		break;
			
		case 1:			/* MULTI buffered	*/
		
		while( done < size )
		{
			word sectors;
			word tfr = size - done;
			
			if( tfr > dcb->MaxTfr ) tfr = dcb->MaxTfr;
			sectors = tfr/info->SectorSize;
		
			/* setup and perform read */
			WriteParameter	(link, m2DesiredSectorBuffer, 0);	
			WriteTripleParameter(link, m2MultiNumSectors, sectors);
			WriteParameter(link, m2MultiMode, m2UpdateLogical|m2Buffered );
			
			WriteCommand(link, m2MultiReadSector );
			
			err = ReadParameter(link, m2Error );
		
			if( err == 0 )
			{
				WriteCommand(link, m2MultiReadBuffer);
				LinkRx(1,link,&err);
				if( err == 0 )
				{
					LinkRx(tfr,link,buf);
					buf += tfr;
				}
			}
			req->Actual += tfr;
			done += tfr;
		}
		break;
				
		case 2:			/* MULTI unbuffered	*/
		{
			word sectors = size/info->SectorSize;
			word sect;
			word ssize = info->SectorSize;
			
			WriteTripleParameter(link, m2MultiNumSectors, sectors);
			WriteParameter(link, m2MultiMode, m2AbortOnFail );
			
			WriteCommand(link, m2MultiReadSector );
			LinkRx(1,link,&err);
			
			if( err == 0 )
			{
				for( sect = 0; err == 0 && sect < sectors; sect++ )
				{
					LinkRx(1,link,&err);
					if( err == 0 )
					{
						LinkRx(ssize,link,buf);
						buf += ssize;
						req->Actual += ssize;
					}
				}
			}
			break;
		}

		} /* end of switch */

		break;
		
	case FG_Write:
		req->Actual = 0;
#if RW
IOdebug("W%d,%d[%d]%",req->DevReq.SubDevice,req->Pos/4096,size/4096);
#endif
		pos = muldiv(pos,dcb->BlockSize,info->SectorSize)+pi->StartSector;

#if RW
IOdebug("(%d[%d])%",pos,size/info->SectorSize);
#endif
		/* once the first command is accepted, we have control */
		WriteParameter	(link, m2DesiredDrive, info->DriveId);
		WriteCommand	(link, m2SelectDrive);
		WriteParameter	(link, m2Addressing, m2LogicalAddressing|m2IncrementLogical|m2IncrementBuffer );
		WriteTripleParameter(link, m2LogicalSector,pos);
		WriteParameter	(link, m2DesiredSectorBuffer, 0);
		
		switch( dcb->WriteMode )
		{
		
		case 0:			/* M212 Mode 1		*/
		while( done < size )
		{
			word sectors, sect;
			word tfr = size - done;
			
			if( tfr > dcb->MaxTfr ) tfr = dcb->MaxTfr;
			sectors = tfr/info->SectorSize;

			/* setup and perform read */
			WriteParameter	(link, m2DesiredSectorBuffer, 0);	
			
			for( sect = 0; sect <sectors ; sect++ )
			{
				WriteCommand(link, m2WriteBuffer);
				LinkTx(info->SectorSize,link,buf);
				buf += info->SectorSize;
			}

			err = ReadParameter(link, m2Error );
			
			if( err == 0 )
			{
				WriteParameter	(link, m2DesiredSectorBuffer, 0);	
				for( sect = 0; sect < sectors ; sect++ )
					WriteCommand(link, m2WriteSector );
				err = ReadParameter(link, m2Error );
			}
			req->Actual += tfr;
			done += tfr;
		}
		break;
						
		case 1:			/* MULTI buffered	*/
		while( done < size )
		{
			word sectors;
			word tfr = size - done;
			
			if( tfr > dcb->MaxTfr ) tfr = dcb->MaxTfr;
			sectors = tfr/info->SectorSize;

			/* setup and perform read */
			WriteParameter	(link, m2DesiredSectorBuffer, 0);	
			WriteTripleParameter(link, m2MultiNumSectors, sectors);
			WriteParameter(link, m2MultiMode, m2UpdateLogical|m2Buffered );

			WriteCommand(link, m2MultiWriteBuffer );
			LinkRx(1,link,&err);
			if( err == 0 )
			{
				LinkTx(tfr,link,buf);
				buf += tfr;
			}

			err = ReadParameter(link, m2Error );
			
			if( err == 0 )
			{
				WriteCommand(link, m2MultiWriteSector );
				err = ReadParameter(link, m2Error );
			}
		
			req->Actual += tfr;
			done += tfr;
		}	
		break;
		
		case 2:			/* MULTI unbuffered		*/
		{
			word sectors = size/info->SectorSize;
			word ssize = info->SectorSize;
			byte *end = buf + size;
						
			/* setup and perform write */
			WriteParameter	(link, m2DesiredSectorBuffer, 0);
			WriteTripleParameter(link, m2MultiNumSectors, sectors);
			WriteParameter	(link, m2MultiMode, m2AbortOnFail );

			WriteCommand(link, m2MultiWriteSector );
			LinkRx(1,link,&err);
			
			if( err == 0 )
			{
				LinkTx(ssize,link,buf);
				buf += ssize;
				while( buf != end && err == 0 )
				{
					LinkTx(ssize,link,buf);
					LinkRx(1,link,&err);
					buf += ssize;
				}
				if( err == 0 )
					LinkRx(1,link,&err);
				req->Actual += size;
			}
			
			err = ReadParameter(link, m2Error );
			break;		
		}	

		} /* end of switch */

		break;

	case FG_Format:			/* format cylinder	*/
	{
		FormatReq *freq = (FormatReq *)req;
		word track;
		word cyl;
		word skew = 0;
		word addressing;
		word start = freq->StartCyl+pi->StartCyl;
		word end = freq->EndCyl+pi->StartCyl;
		
		WriteParameter	(link, m2DesiredDrive, info->DriveId);
		WriteCommand	(link, m2SelectDrive);
		addressing = ReadParameter(link, m2Addressing );
		WriteParameter	(link, m2Addressing, 0);
		WriteParameter	(link, m2Interleave, freq->Interleave );

		for( cyl = start; err == 0 && cyl <= end && cyl < pi->EndCyl; cyl++ )
		{
			WriteDoubleParameter(link, m2DesiredCylinder, cyl);
			for( track = 0; err == 0 && track < info->TracksPerCyl; track++ )
			{
				WriteParameter	(link, m2Skew, skew);
				WriteParameter	(link, m2DesiredHead, track);
				WriteCommand	(link, m2FormatTrack);
				err = ReadParameter(link, m2Error );
				skew = (skew + freq->TrackSkew)%info->SectorsPerTrack;
			}
			skew = (skew + freq->CylSkew)%info->SectorsPerTrack;
		}

		WriteParameter	(link, m2Addressing, addressing );
				
		break;
	}	

	/* Write data to bootstrap area of disc. The request is just	*/
	/* like a Write request. The SubDevice is used to identify the	*/
	/* physical disc to be written. This has to be a device operation*/
	/* because the exact operations required to do this will differ	*/
	/* for different controllers.					*/
	case FG_WriteBoot:
	{
		byte save[32];
		int i;
		word sectors;
		word sect;
		word ssize;
		
		WriteParameter	(link, m2DesiredDrive, info->DriveId);
		WriteCommand	(link, m2SelectDrive);

		/* first save current drive parameters */
				
		for(i = 0; i < 32; i++ ) save[i] = ReadParameter(link, i);
		
		/* reset drive to defaults */
		
		WriteCommand	(link, m2Initialise, info->DriveType);
		WriteCommand	(link, m2Restore);
		
		ssize = 256;
		sectors = (size+ssize-1)/ssize;
		
		/* first we format the tracks which will contain the bootstrap */

		while( err == 0 && ReadTripleParameter(link, m2LogicalSector ) <= sectors )
		{
			WriteCommand(link, m2FormatTrack );
			err = ReadParameter(link, m2Error );
		}
		
		/* now write data out */
		WriteTripleParameter(link, m2LogicalSector, 0 );
		
		for( sect = 0; err == 0 && sect < sectors ; sect++ )
		{
			WriteCommand(link, m2WriteBuffer );
			LinkTx(ssize, link, buf );
			buf += ssize;
			err = ReadParameter(link, m2Error );
			if( err != 0 ) break;
			WriteCommand(link, m2WriteSector );
			err = ReadParameter(link, m2Error );
		}
		
		/* finally restore original drive parameters */

		for(i = 0; i < 32; i++ ) WriteParameter(link, i, save[i] );

		break;
	}

	case FG_GetSize:
		error = info->SectorSize * info->SectorsPerTrack * 
			info->TracksPerCyl * (pi->EndCyl-pi->StartCyl+1);
		break;			
	}

		
	if( err != 0 )
	{
		reason = ReadParameter(link, m2Reason );
		error = EC_Error|EG_Broken|(err<<8)|reason;
#if RW
IOdebug("M212 error %x %",error);
if( err == 4 )
{
	IOdebug("bad parameter %x %",ReadTripleParameter(link, reason));
}
#endif
	}

	WriteCommand(link, m2EndOfSequence );
#if RW
IOdebug("!");
#endif
	Signal(&dcb->Lock);

done:	
	req->DevReq.Result = error;

	/* call back to client */
	(*req->DevReq.Action)(req);
}

word DevClose(DiscDCB *dcb)
{
	Wait(&dcb->Lock);
	
	unless( dcb->OldConf.Mode==Link_Mode_Dumb &&
	        dcb->OldConf.State==Link_State_Running) FreeLink(dcb->Link);
	        
	Configure(dcb->OldConf);

	return Err_Null;
}

DiscDCB *DevOpen(Device *dev, DiscDevInfo *info)
{
	DriveInfo *dvi;
	PartitionInfo *pii;
	DiscDCB *dcb = Malloc(sizeof(DiscDCB));
	LinkInfo linfo;
	LinkConf c;
	int i;

	if( dcb == NULL ) goto fail;
	
	if( LinkData(info->Controller,&linfo) != Err_Null ) goto fail;
	
	c.Id = info->Controller;
	c.Mode = Link_Mode_Dumb;
	c.State = 0;
	c.Flags = 0;
	
	if( Configure(c) != Err_Null ) goto fail;

#if 0	
	if( AllocLink(info->Controller) !=Err_Null ) goto fail;
#else
	/* allows us to run fs & de in parallel */
	AllocLink(info->Controller);
#endif
	
	dcb->DCB.Device = dev;
	dcb->DCB.Operate = DevOperate;
	dcb->DCB.Close = DevClose;

	InitSemaphore(&dcb->Lock,1);
	dcb->Link = info->Controller;
	dcb->OldConf = *(LinkConf *)&linfo;

	dcb->BlockSize = info->Addressing;
	dcb->ReadMode = info->Mode & 0xf;
	dcb->WriteMode = (info->Mode>>4) & 0xf;
	
	dvi = (DriveInfo *)RTOA(info->Drives);
	for( i = 0 ; i < 4 ; i++ )
	{
		dcb->Drive[i] = *dvi;
		if( dvi->Next == -1 ) break;
		dvi = (DriveInfo *)RTOA(dvi->Next);
	}
	dcb->Drives = i+1;	
	
	pii = (PartitionInfo *)RTOA(info->Partitions);
	for( i = 0 ; i < 10 ; i++ )
	{
		dcb->Partition[i] = *pii;
		if( pii->Next == -1 ) break;
		pii = (PartitionInfo *)RTOA(pii->Next);
	}
	dcb->Partitions = i+1;
			
	InitM2(dcb);
		
	return dcb;
	
fail:
	Free(dcb);
	return NULL;
}
