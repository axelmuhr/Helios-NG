/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 4 September 1992                                             */
/* File: ufsfmt.c                                                     */
/*                                                                    */
/* 
 * This program is used to format a new disk for UFS.
 *
 * Usage: ufsfmt discdevice:unit
 *
 *	where
 *		discdevice 	is the name of the disc entry in devinfo
 *		unit 		is the unit identifier for the device driver
 *
 * $Id: ufsfmt.c,v 1.1 1992/09/16 10:01:43 al Exp $
 * $Log: ufsfmt.c,v $
 * Revision 1.1  1992/09/16  10:01:43  al
 * Initial revision
 *
 */

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

#define DKTYPENAMES
#include <sys/disklabel.h>

/* All disks must be formatted with a 512 byte sector size. */
#define DEV_BSIZE 512

#define _arg_	if (*arg==0) arg = *++argv;

void tidyup(int rc);

DCB *discdcb;
DiscDevInfo *ddi;
void *devinfo;
char *progname;

/* Routines to get the device driver etc info from devinfo */
void *load_devinfo(void)
{
  Stream *s = NULL;
  Object *o;
  void *devinfo = NULL;
  int size;
  ImageHdr hdr;

  /* Locate the devinfo information */
  o = Locate(NULL,"/rom/devinfo");
  if (o == NULL) o = Locate(NULL,"/loader/DevInfo");
  if (o == NULL) o = Locate(NULL,"/helios/etc/devinfo");
  if (o == NULL) return NULL;

  /* Open it and read it */
  s = Open(o,NULL,O_ReadOnly);
  if (s == NULL) { 
    Close(o);
    return NULL;
  }
  if (Read(s,(byte *)&hdr,sizeof(hdr),-1) == sizeof(hdr)) {
    /* The header was read o.k., just check it */
    if (hdr.Magic == Image_Magic ) {
      /* Header was fine, read in from the file */
      size = hdr.Size;
      devinfo = Malloc(size);
      if (devinfo != NULL) {
      	/* Malloc OK, actual read */
        if (Read(s,devinfo,size,-1) != size) { 
          /* Read Failed */
          Free(devinfo);
          devinfo = NULL;
        }
      }
    }
  }
  Close(s);
  Close(o);

  return(devinfo);
}

InfoNode *find_info(void *devinfo, word type, char *name)
{ InfoNode *info = (InfoNode *)((Module *)devinfo + 1);

  forever {
    if ((strcmp(name,RTOA(info->Name)) == 0) && (info->Type == type))
      return(info);
		    
    if (info->Next == 0) break;
    info = (InfoNode *)RTOA(info->Next);
  }
  return(NULL);
}

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

void tidyup(int rc)
{
	tidy_disc();
	
	exit(rc);	
}

void fmt_action(FormatReq *req)
{
	Signal(&req->WaitLock);
}


void format(int unit, int interleave, int trackskew, int cylskew, int end)
{
	FormatReq req;

	req.DevReq.Request = FG_Format;
	req.DevReq.Action = fmt_action;
	req.DevReq.SubDevice = unit;
	InitSemaphore(&req.WaitLock,0);
	
	req.Interleave = interleave;
	req.TrackSkew = trackskew;
	req.CylSkew = cylskew;
	req.StartCyl = 0;	/* From the beginning */
	req.EndCyl = end;	/* To the end */

	Operate(discdcb,&req);	

	Wait(&req.WaitLock);
		
	if( req.DevReq.Result < 0 ) error("Error from disc: %x",req.DevReq.Result);
}

int main(int argc, char **argv)
{
	char answer[10];
	char *devname = NULL;
	char *labelname = NULL;
	struct disklabel *dl;
	char *arg;
	int end = -1;
	int interleave = -1;
	int trackskew = -1;
	int cylskew = -1;
	int unit = -1;

	/* Get the program name */
	for (progname = *argv + strlen(*argv); 
		(progname > *argv) && (*progname != '/');
		progname--);
	if (*progname == '/') progname++;
	
	/* Parse the arguments */
	for (argv++; *argv; argv++) {
		arg = *argv;

		if (*arg == '-') {
			arg++;
			switch(*arg++) {
			case 'e': 
				_arg_;
				end = atoi(arg);
				break;
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
			}
		} else if (devname == NULL) {
			devname = arg;
		} else if (labelname == NULL) {
			labelname = arg;
		}
	}

	/* Extract the unit number */
	for (arg = devname; *arg && (*arg != ':'); arg++);
	if (*arg) {
		*arg++ = '\0';	/* Terminate device name */
		if (*arg) unit = atoi(arg);
	}
	
	if ((devname == NULL) || (unit == -1)) {
		printf("usage: %s %s %s\n",
			progname,
			"[-i<interleave>] [-t<trackskew>] [-c<cylskew>]",
			"[-e<endcyl>] discdevice:unit\n");
		exit(1);
	}

	/* If there is a label, use it */
	if (labelname != NULL) {
		if ((dl = getdiskbyname(labelname)) == NULL) {
			fprintf(stderr, "%s: unknown disk type\n", labelname);
			exit(1);
		}
		if (end < 0) end = dl->d_ncylinders;
		if (interleave < 0) interleave = dl->d_interleave;
		if (trackskew < 0) trackskew = dl->d_trackskew;
		if (cylskew < 0) cylskew = dl->d_cylskew;
	}
	
	/* If not set resort to defaults */
	if (end < 0) end = 0;
	if (interleave < 0) interleave = 1;
	if (trackskew < 0) trackskew = 0;
	if (cylskew < 0) cylskew = 0;

	init_disc(devname);

	printf("Format:\n\tdevice %10s\n\tunit          %3d\n",devname,unit);
	printf("\tinterleave    %3d\n\ttrack skew    %3d\n\tcylinder skew %3d\n",
		interleave,trackskew,cylskew);
	
	printf("\n=================== WARNING ===================\n");
	printf("This command will destroy all data on the disc\n");
	printf("Are you sure you want to do this (Y/N)? ");
	fflush(stdout);
	fgets(answer,10,stdin);

	if( strcmp(answer,"y\n")==0 || strcmp(answer,"Y\n")==0 )
		format(unit,interleave,trackskew,cylskew,end);
	
	tidyup(0);
}


