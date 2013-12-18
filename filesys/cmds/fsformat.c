
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <syslib.h>
#include <device.h>
#include <setjmp.h>
#include <codes.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <gsp.h>

#define BSIZE 4096

#include "rdevinfo.c"

#define _arg_ if(*arg==0) arg = *++argv;

void tidyup(int rc);

DCB *discdcb;
DiscDevInfo *ddi;
void *devinfo;


void error(char *str,...)
{
	va_list a;
	va_start(a,str);
	fprintf(stderr,"ERROR: ");
	vfprintf(stderr,str,a);
	fprintf(stderr,"\n");
	tidyup(1);
}

void warn(char *str,...)
{
	va_list a;
	va_start(a,str);
	fprintf(stderr,"WARNING: ");
	vfprintf(stderr,str,a);
	fprintf(stderr,"\n");
}


PartitionInfo *getpart(int partition)
{
	PartitionInfo *pii  = (PartitionInfo *) RTOA(ddi->Partitions);
	
	while( partition-- )
	{
		if( pii->Next == -1 ) return NULL;
		pii = (PartitionInfo *) RTOA(pii->Next);
	}
	return pii;
}

DriveInfo *getdrive(int Drive)
{
	DriveInfo *dvi  = (DriveInfo *) RTOA(ddi->Drives);
	
	while( Drive-- )
	{
		if( dvi->Next == -1 ) return NULL;
		dvi = (DriveInfo *) RTOA(dvi->Next);
	}
	return dvi;
}

void init_disc(char *name)
{
	InfoNode *info;
	
	devinfo = load_devinfo();
	
	info = find_info(devinfo,Info_DiscDev,name);
	if( info == NULL ) error("no such device %s",name);
	
	ddi = (DiscDevInfo *)RTOA(info->Info);

	discdcb = OpenDevice(RTOA(ddi->Name),ddi);
	
	if( discdcb == NULL ) error("cannot create device driver");
}

void tidy_disc(void)
{
	word e;
	
	if( discdcb == NULL ) return;

	e = CloseDevice(discdcb);
	
	if( e != Err_Null ) warn("failed to close disc device: %x",e);
}

void fmt_action(FormatReq *req)
{
	Signal(&req->WaitLock);
}


void format(int partition, int interleave, int trackskew, int cylskew)
{
	int start = 0;
	int end = 611;
	int cyl;
	FormatReq req;
	PartitionInfo *pii  = getpart(partition);
	DriveInfo *dvi;

	if( pii == NULL ) error("no partition %d",partition);

	if( pii->Drive == -1 ) pii->Drive = 0;
			
	dvi = getdrive(pii->Drive);
	
	if( dvi == NULL ) error("no drive %d",pii->Drive);

	if( pii->EndCyl == -1 ) pii->EndCyl = dvi->Cylinders-1;
	
	start = pii->StartCyl;
	end   = pii->EndCyl;

	req.DevReq.Request = FG_Format;
	req.DevReq.Action = fmt_action;
	req.DevReq.SubDevice = partition;
	InitSemaphore(&req.WaitLock,0);
	
	req.Interleave = interleave;
	req.TrackSkew = trackskew;
	req.CylSkew = cylskew;
	req.StartCyl = start;
	req.EndCyl = end;

	Operate(discdcb,&req);	

	Wait(&req.WaitLock);
		
	if( req.DevReq.Result < 0 ) error("Error from disc: %x",req.DevReq.Result);
}

int main(int argc, char **argv)
{
	char answer[10];
	char *devname = NULL;
	int interleave = 1;
	int trackskew = 0;
	int cylskew = 0;
	int partition = 0;
	
	for( argv++; *argv; argv++ )
	{
		char *arg = *argv;
		if( *arg == '-' )
		{
			arg++;
			switch( *arg++ )
			{
			case 'i': 
				_arg_;
				interleave = atoi(arg);
				break;
			case 't': 
				_arg_;
				trackskew = atoi(arg);
				break;
			case 'c': 
				_arg_;
				cylskew = atoi(arg);
				break;
				
			case 'p':
				_arg_;
				partition = atoi(arg);
				break;
			}
		}
		else devname = arg;
	}
	
	if( devname == NULL )
	{
		printf("usage: fsformat [-i<interleave>] [-t<trackskew>] [-c<cylskew>] [-p<partition>] device\n");
		exit(1);
	}
	
	init_disc(devname);

	printf("Format:\n\tdevice %10s\n\tpartition     %3d\n",devname,partition);
	printf("\tinterleave    %3d\n\ttrack skew    %3d\n\tcylinder skew %3d\n",
		interleave,trackskew,cylskew);
	
	printf("\n=================== WARNING ===================\n");
	printf("This command will destroy all data on the disc\n");
	printf("Are you sure you want to do this (Y/N)? ");
	fflush(stdout);
	fgets(answer,10,stdin);

	if( strcmp(answer,"y\n")==0 || strcmp(answer,"Y\n")==0 )
		format(partition,interleave,trackskew,cylskew);
	
	tidyup(0);
}

void tidyup(int rc)
{
	tidy_disc();
	
	exit(rc);	
}

