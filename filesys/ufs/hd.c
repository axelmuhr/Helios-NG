/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 29 July 1992                                                 */
/* File: hd.c                                                         */
/*                                                                    */
/* This file contains the UNIX device driver routines for Helios.     */
/* It also contains the controlling routines for the device.          */
/*                                                                    */
/* $Id: hd.c,v 1.2 1992/12/09 17:36:36 al Exp $ */
/* $Log: hd.c,v $
 * Revision 1.2  1992/12/09  17:36:36  al
 * Changed procname to myprocname and back_trace to my_back_trace.
 * I must update these files later to use the proper helios calls.
 *
 * Revision 1.1  1992/09/16  09:29:06  al
 * Initial revision
 * */

#include <string.h>
#include <gsp.h>
#include <module.h>
#include <device.h>
#include <sem.h>
#include <codes.h>

#include "param.h"
#include "vnode.h"
#include "quota.h"
#include "inode.h"
#include "buf.h"
#include "fs.h"
#include "file.h"
#include "ioctl.h"
#define DFLT_SIZES
#include "disklabel.h"

/*
 * Define the drive states.
 */
#define RAWDISK		8
#define ISRAWSTATE(s)	(RAWDISK&(s))		/* Are we in raw state */
#define DISKSTATE(s)	(~RAWDISK&(s))		/* Ignore raw mode */

#define CLOSED		0
#define READLABEL	1
#define	OPEN		2

#define RAWOPEN		(RAWDISK|OPEN)

#define b_cylin		b_resid

/*
 * The DCB for each of the drives, plus partition info required.
 */
struct disk {
	struct disklabel dk_lab;		/* Disk label		*/
	short	dk_wlabel;			/* Label writable?	*/
	short	dk_state;			/* Disk state		*/
	short	dk_protected;			/* Write protected?	*/
	long	dk_reads, dk_writes;		/* No of r/w		*/
	Semaphore dk_numq;			/* Number of bufs in q	*/
	Semaphore dk_guard;			/* Guard for dk_queue.	*/
	struct buf dk_queue;			/* Head of disk queue	*/
	short	dk_pstatus[MAXPARTITIONS];	/* Status of partitions	*/
};
struct hd_devices{
	struct disk	disk[MAXDISKS];		/* Disk information.	*/
	Semaphore	numbufs;		/* Number of buffers q'd*/
	Semaphore	abort;			/* Abort operations */
	int		old;			/* Old style device driver */
	DCB		*dcb;			/* Device Control Block */
};
static struct hd_devices hd[MAX_HELIOS_DEVICES];

/*
 * Prototypes used.
 */
void tokernel(void);
void fromkernel(void);
void *load_devinfo(void);
InfoNode *find_info(void *devinfo, word type, char *name);

/*
 * INTERNAL DEVICE DRIVER ROUTINES
 */
/*
 * Routine to wake up the hd_intr routine below on completion of disk I/O.
 */
static void hd_iodone(DiscReq *req)
{
	Signal(&req->WaitLock);
}

/*
 * The controller interrupt routine.  This checks the minor device queues
 * and performs the required operations on each of the minor devices.
 * The device scheduling is on a round-robin basis with control always 
 * passing to the next minor device.
 */
int hd_ops = 0;
static void hd_intr_server(int major)
{
	struct hd_devices *hdev = &hd[major];
	Semaphore *numbufs = &(hdev->numbufs);
	Semaphore *abort = &(hdev->abort);
	DCB *dcb = hdev->dcb;
	struct disk *this_hd = &(hdev->disk[0]);	/* Start at 0 */
	struct disk *last_hd = &(hdev->disk[MAXDISKS]);	/* Terminate check */
	int searched;
	DiscReq req;
	struct partition *pp;
	struct buf *dp, *bp;
#ifdef DEBUG
printf("hd_intr_server: Helios Major device %d running\n",major);
#endif
	/* Loop until ordered to shut down and no more buffers to work on. */
	for (;;) {
		/* Wait for a block to work on. */
		Wait(numbufs);
		if (TestWait(abort)) {
			/* Forget anything else */
			return;
		}

		/* Look for a disk to work on */
		searched = 0;
		while (!TestWait(&this_hd->dk_numq) && searched != MAXDISKS) {
			this_hd++;
			searched++;
			if (this_hd >= last_hd)
				this_hd = &(hdev->disk[0]);
		}
		if (searched == MAXDISKS) {
			printf("hd_intr: PANIC %d, Failed to find buffer\n",major);
			continue;
		}

		/* Extract the buffer */
		dp = &(this_hd->dk_queue);
		bp = dp->b_actf;

		/* Get the partition (offset) */
		pp = &(this_hd->dk_lab.d_partitions[dkpart(bp->b_dev)]);

		/* Perform the operation */
		req.DevReq.Request = ((bp->b_flags & B_READ)?FG_Read:FG_Write);
		req.DevReq.Action = hd_iodone;
		req.DevReq.SubDevice = dkunit(bp->b_dev);
		req.DevReq.Timeout = -1;	/* No Timeout */
		req.Size = bp->b_bcount;
		req.Buf =  bp->b_un.b_addr;
		req.Pos = bp->b_blkno + pp->p_offset;	/* Sector */

		/* Check if we are writing a disk label */
		if ((req.Pos <= LABELSECTOR) && (!this_hd->dk_wlabel) &&
		    (req.DevReq.Request == FG_Write)) {
			bp->b_error = EACCES;	/* Not writeable */
			goto skip;
		}

		/* Adjust to byte location */
		req.Pos *= this_hd->dk_lab.d_secsize;
#ifdef DEBUG
printf("hd_intr_server: Major %d performing %s on dev=%d  buf=0x%x (dcb=0x%x)\n",
	major,((bp->b_flags & B_READ)?"Read":"Write"),bp->b_dev,buf,dcb);
printf("hd_intr_server: Request=0x%x, SubDevice=%d, Timeout=%d, Size=%d\n",
	req.DevReq.Request,req.DevReq.SubDevice,req.DevReq.Timeout,req.Size);
#endif
		InitSemaphore(&(req.WaitLock),0);
		Operate(dcb,&req);
		Wait(&(req.WaitLock));		/* XXX Implement multiple ops
						 * later.  (i.e.TestWait for
						 * each disk)
						 */
hd_ops++;
#ifdef DEBUG
printf("hd_intr_server:  Major%d Pos=%d, Buf=0x%x  Actual=%d  %s\n",
	major,req.Pos,req.Buf,req.Actual,
	(req.Size==req.Actual)?"O.K.":"ERROR");
#endif

		/* Any problems ? */
		if (req.Size == req.Actual) {
			bp->b_error = 0;
		} else {
			/* Error occurred */
			bp->b_error = EIO;
			bp->b_flags |= B_ERROR;
		}

		/* Operation complete, lock queue and remove bp */
skip:
		Wait(&this_hd->dk_guard);
		dp->b_actf = bp->av_forw;
		bp->b_resid = 0;
		bp->av_forw = NULL;
		Signal(&this_hd->dk_guard);

		/* Need to go into UNIX kernel to call biodone */
		tokernel();
		biodone(bp);
		fromkernel();

		/* Round robin scheduling */
		this_hd++;
		if (this_hd >= last_hd) this_hd = &(hdev->disk[0]);

	}
}

/*
 * EXTERNAL DEVICE DRIVER ROUTINES
 */

/*
 * Initialise the device routines
 */
void hd_init(void)
{
	int h,i,j;

	for (h=0; h<MAX_HELIOS_DEVICES; h++) {
		InitSemaphore(&(hd[h].numbufs),0);
		InitSemaphore(&(hd[h].abort),0);
		hd[h].old = 0;
		hd[h].dcb = NULL;
		for (i=0; i<MAXDISKS; i++) {
			hd[h].disk[i].dk_lab = dflt_sizes;
			hd[h].disk[i].dk_wlabel = 0;
			hd[h].disk[i].dk_state = CLOSED;
			hd[h].disk[i].dk_protected = FALSE;
			hd[h].disk[i].dk_reads = 0;
			hd[h].disk[i].dk_writes = 0;
			InitSemaphore(&hd[h].disk[i].dk_numq,0);
			InitSemaphore(&hd[h].disk[i].dk_guard,1);
			hd[h].disk[i].dk_queue.b_actf = NULL;
			hd[h].disk[i].dk_queue.b_forw = NULL;
			for (j=0; j<MAXPARTITIONS; j++)
				hd[h].disk[i].dk_pstatus[j]=CLOSED;
		}
	}
}

/*
 * Given a Helios device name, set this up as a major device unit
 * (one of the MAX_HELIOS_DEVICES) and start a driver routine for it.
 * Returns pointer to error message on failure.  NULL otherwise.
 */
char *hd_start(char *devname, int major)
{
	static char buffer[100];
	void *devinfo;
	InfoNode *deviceinfo;
	DiscDevInfo *ddi;

	/* The major device number */
	if ((major < 0) || (major >= MAX_HELIOS_DEVICES) || (hd[major].dcb)) {
		return("Invalid major device or device already in use");
	}

	/* Get devinfo */
	if ((devinfo = load_devinfo()) == NULL)
		return("Failed to load devinfo file");

	/* Is entry there ? */
	if ((deviceinfo = find_info(devinfo,Info_DiscDev,devname)) == NULL) {
		Free(devinfo);
		return("No entry for device in devinfo");
	}
	ddi = (DiscDevInfo *)RTOA(deviceinfo->Info);

	/* Open it ! */
	if ((hd[major].dcb = OpenDevice(RTOA(ddi->Name),ddi))
	    == NULL) {
		strcpy(buffer,"Failed to open device ");
		strcat(buffer,RTOA(ddi->Name));
		Free(devinfo);
		return(buffer);
	}
	Free(devinfo);

	/* Fork off the device server */
	if (!Fork(5000,hd_intr_server,sizeof(int),major)) {
		return("Failed to start device server");
	}

	/* No errors */
	return(NULL);
}

/*
 * This will close all Major Helios devices with no disk requests.
 * It will return the total number of outstanding requests to Helios devices.
 * The force flag will forcibly close all devices, regardless of the number
 * of requests outstanding.
 */
int hd_shutdown(int force)
{ int total, i, tmp;

  total = 0;
  for (i=0; i < MAX_HELIOS_DEVICES; i++)
    if (hd[i].dcb) {

	/* This one is open */
	if (((tmp = TestSemaphore(&hd[i].numbufs)) <= 0) || force) {
		Signal(&hd[i].abort);	/* Start abort process */
		Signal(&hd[i].numbufs);	/* Wakeup routine */

		Delay(100000);		/* Give device time to exit */
		CloseDevice(hd[i].dcb);
		hd[i].dcb = NULL;
	} else total += tmp;
    }
  return(total);
}

/*
 * This will return the total number of outstanding disk operations.
 */
int hd_disk_operations(void)
{ int total, i, tmp;

  total = 0;
  for (i=0; i < MAX_HELIOS_DEVICES; i++)
    if (hd[i].dcb) {
	/* This one is open */
	if ((tmp = TestSemaphore(&hd[i].numbufs)) > 0) {
		total += tmp;
	}
    }
#ifdef DEBUG
printf("hd_disk_operations: %d outstanding,  %d total",total,hd_ops);
#endif
  return(total);
}

/*
 * Read/Write routine for a buffer.  Finds the proper unit,
 * range checks arguments and schedules the transfer.  Does not wait
 * for the transfer to complete.  All I/O requests must be a multiple
 * of a sector in length.
 */
int hd_strategy(struct buf *bp)
{
	u_int hdmajor = major(bp->b_dev);
	u_int unit = dkunit(bp->b_dev);
	u_int part = dkpart(bp->b_dev);
	struct disk *du = &(hd[hdmajor].disk[unit]);
	Semaphore *numbufs = &(hd[hdmajor].numbufs);
	struct buf *dp;
	struct partition *p;
	long maxsz, sz;
#ifdef MONITOR
printf("hd_strategy: Called with bp=0x%x, dev=%d, blck=%d  by %s\n",
	bp,bp->b_dev,bp->b_blkno,myprocname(returnlink_(bp)));
#endif
	/* Simple parameter check */
	if ((unit >= MAXDISKS) || (bp->b_blkno < 0) 
	    || (part >= du->dk_lab.d_npartitions)) {
		printf("hd_strategy: major=%d, unit=%d, part=%d, blkno=%d, bcount=%d\n",
			hdmajor,unit,part,bp->b_blkno,bp->b_bcount);
		printf("hd: Error in hd_strategy");
		bp->b_flags |= B_ERROR;
		goto bad;
	}

	/* Check for write protection */
	if (du->dk_protected && ((bp->b_flags & B_READ) == 0)) {
		printf("hd_strategy: %d:%d: write protected\n",hdmajor,unit);
		goto bad;
	}

	if (DISKSTATE(du->dk_state) != OPEN)
		goto q;

	/* Determine size of xfer and make sure it fits. */
	p = &(du->dk_lab.d_partitions[part]);
	maxsz = p->p_size;
	sz = (bp->b_bcount + DEV_BSIZE - 1) >> DEV_BSHIFT;

	/* XXX Check disk label writing at a later stage */

	/* Check the parameters */
	if ((bp->b_blkno < 0) || ((bp->b_blkno + sz) > maxsz)) {
		/* if exactly at end of disk, return an EOF. */
		if (bp->b_blkno == maxsz) {
			bp->b_resid = bp->b_bcount;
			biodone(bp);
			return(0);
		}
		/* or truncate if part of it fits */
		sz = maxsz - bp->b_blkno;
		if (sz <= 0) {
			printf("hd%d: invalid size %d\n",unit,sz);
			goto bad;
		}
		bp->b_bcount = sz << DEV_BSHIFT;
	}
	bp->b_cylin = (bp->b_blkno + p->p_offset) / du->dk_lab.d_secpercyl;

q:
	dp = &(du->dk_queue);

	/* Lock buffer head, add item, free buffer head and signal */
	Wait(&du->dk_guard);		/* Lock */
	disksort(dp, bp);		/* Add */
	Signal(&du->dk_guard);		/* Free */
	Signal(&du->dk_numq);		/* Signal another buffer in this q */
	Signal(numbufs);		/* Signal Device Driver */
	
#ifdef MONITOR
printf("hd_strategy: Done OK\n");
#endif
	return(0);

bad:
#ifdef MONITOR
printf("hd_strategy: Done Failed\n");
#endif
printf("hd_strategy: bad error\n");
	bp->b_error = EINVAL;
	biodone(bp);
	return(1);
}

/*
 * Open a minor device (relative to controller).
 * If the disk has not been opened, it is.  Since the minor device number
 * also refers to the partition, this will also setup the partition state.
 */
static void dev_openaction(DiscOpenCloseReq *req)
{
  Signal(&req->WaitLock);
}
#define MYSPECCODE 0x13126521
int hd_open(dev_t dev, int flags, int fmt)
{
	u_int major, unit, part;
	struct disk *du;
	struct hd_devices *hdev;
	DCB *dcb;
	DiscOpenCloseReq openreq;
	DiscParameterReq dpreq;
	struct buf *bp;
	int error = 0;
	int wasold = FALSE;

#ifdef MONITOR
 printf("hd_open: Called with dev=%d,  flags=%d,   fmt=%d  by %s\n",
	dev,flags,fmt,myprocname(returnlink_(dev)));
#endif
	/* Get the unit info */
	major = major(dev);
	unit = dkunit(dev);
	part = dkpart(dev);
	hdev = &hd[major];
	dcb = hd[major].dcb;
	if (unit >= MAXDISKS) return(ENXIO);
	du = &(hdev->disk[unit]);

#ifdef DEBUG
printf("hd_open: major %d  unit %d  part %d\n",major,unit,part);
#endif

	/* Has the disk already been opened. */
	if (du->dk_state != CLOSED) {
		/* Already open, so don't mess with it. */
		goto getpart;
	}

	/* Undefined device OR Trap old device driver compatibility */
	if ((dcb == NULL) || hdev->old) {
		/* Old style only supports 1 disk */
		return(ENODEV);
	}

	/* Open the actual disk */
	openreq.DevReq.Request = FG_Open;
	openreq.DevReq.Action = dev_openaction;
	openreq.DevReq.SubDevice = unit;
	openreq.DevReq.Timeout = -1;
	openreq.DevReq.Result = MYSPECCODE;	/* Special Code */
	InitSemaphore(&(openreq.WaitLock),0);
	Operate(dcb,&openreq);
	Wait(&(openreq.WaitLock));

	/* Old style device driver was opened */
	if ((openreq.DevReq.Result == MYSPECCODE) || 
	    ((openreq.DevReq.Result & EG_Mask) == EG_WrongFn))  {
		wasold = TRUE;
	} else if (openreq.DevReq.Result) {
		/* Error in opening device */
		return(ENODEV);
	}

	/* Get the disk label using device driver defaults */
	du->dk_lab = dflt_sizes;
	du->dk_state = READLABEL;
	bp = geteblk(DEV_BSIZE);
	bp->b_dev = dev & 0xFFFFFFF8;	/* Force to boot partition (a) */
	bp->b_blkno = LABELSECTOR;
	bp->b_flags = B_READ;
	hd_strategy(bp);		/* Start the operation. */
	biowait(bp);			/* Wait until complete */
	if (bp->b_flags & B_ERROR) {
		error = ENXIO;
		du->dk_state = CLOSED;
		goto done;
	}

	/* Is label there, otherwise open as raw. */
	if (((struct disklabel *)
	    (bp->b_un.b_addr + LABELOFFSET))->d_magic == DISKMAGIC) {
		du->dk_lab = *(struct disklabel *)
				(bp->b_un.b_addr + LABELOFFSET);
		du->dk_state = OPEN;

		/* Pass disk information back to device driver */
		/* Standard Request */
		dpreq.DevReq.Request = FG_SetInfo;
		dpreq.DevReq.Result = MYSPECCODE;
		dpreq.DevReq.Action = dev_openaction;
		dpreq.DevReq.SubDevice = unit;
		dpreq.DevReq.Timeout = -1;
		InitSemaphore(&dpreq.WaitLock,0);

		/* Setup SetInfo parameters */
		dpreq.DriveType = 0;	/* Fixed Disk ? */
		dpreq.SectorSize = du->dk_lab.d_secsize;
		dpreq.SectorsPerTrack = du->dk_lab.d_nsectors;
		dpreq.TracksPerCyl = du->dk_lab.d_ntracks;
		dpreq.Cylinders = du->dk_lab.d_ncylinders;

		/* Make it */
		Operate(dcb,&dpreq);
		Wait(&(dpreq.WaitLock));
			
		if (dpreq.DevReq.Result)
		  printf("hd_open: Warning; Fault 0x%x on major dev %d FG_SetInfo\n",
			 (dpreq.DevReq.Result==MYSPECCODE) ?
			   (SS_Device|EC_Error|EG_WrongFn|EO_Medium):
			   dpreq.DevReq.Result,
			 major);
	} else {
		printf("hd (dev %d): Bad disk label (%x)\n",bp->b_dev,
    ((struct disklabel *)(bp->b_un.b_addr + LABELOFFSET))->d_magic);
		du->dk_state = RAWOPEN;
	}

done:
	/* Release buffer. */
	bp->b_flags = B_INVAL | B_AGE;
	brelse(bp);

getpart:
#ifdef DEBUG
printf("hd_open: device %d get part %d\n",dev,part);
#endif
	/* Get and set the partition info */
	if (part >= MAXPARTITIONS) return(ENXIO);
	if (du->dk_pstatus[part] != CLOSED) {
		/* Partition is already open, don't mess with it. */
#ifdef DEBUG
printf("hd_open: device %d part %d already open\n",dev,part);
#endif
		return(0);
	}
	if (du->dk_state == RAWOPEN) {
#ifdef DEBUG
printf("hd_open: device %d open as raw part %d\n",dev,part);
#endif
		/* If no label, then only allow raw */
		if (part == (RAWPARTITION - 'a')) {
#ifdef DEBUG
printf("hd_open: device %d opened as raw\n",dev);
#endif
			du->dk_pstatus[part] = RAWOPEN;
		} else {
			du->dk_state = CLOSED;
#ifdef DEBUG
printf("hd_open: device %d refused opening partition %d as raw\n",dev,part);
#endif
			return(ENXIO);	
		}
	} else {
		/* Label found, so check overlap with other open partitions */
		struct partition *pp;
		int start,end,i;

		if (part >= du->dk_lab.d_npartitions) return(ENXIO);
		
		pp = &du->dk_lab.d_partitions[part];
		start = pp->p_offset;
		end = start + pp->p_size;
		for (pp = du->dk_lab.d_partitions, i=0;
		     i < du->dk_lab.d_npartitions;
		     pp++, i++) {
			/* Ends before this starts or starts before this ends */
			if (pp->p_offset + pp->p_size <= start ||
			    pp->p_offset >= end)
				continue;
			if (du->dk_pstatus[i]) {
				printf("hd%d%c: overlaps open partition (%c)\n",
				    unit,part+'a',i+'a');
			}
		}

		du->dk_pstatus[part] = OPEN;
	}
#ifdef DEBUG
printf("hd_open: device %d opened error=%d\n",dev,error);
#endif
	if (!error && wasold) hdev->old = TRUE;
	return(error);
}

/*
 * Close a drive.
 */
int hd_close(dev_t dev, int flags, int fmt)
{
	u_int major, unit, part;
	struct disk *du;
	struct hd_devices *hdev;
	DCB *dcb;
	
#ifdef MONITOR
printf("hd_close: Called with dev=%d,  flags=%d,   fmt=%d  by %s\n",
	dev,flags,fmt,myprocname(returnlink_(dev)));
#endif
	
	/* Get the unit info */
	major = major(dev);
	unit = dkunit(dev);
	part = dkpart(dev);
	hdev = &hd[major];
	dcb = hdev->dcb;
	if (unit >= MAXDISKS) return(ENXIO);
	du = &(hdev->disk[unit]);

	if (du->dk_state == CLOSED) return(0);
	if (du->dk_state == RAWOPEN) {
		du->dk_state = CLOSED;
		for (part=0; part<MAXPARTITIONS; part++)
			du->dk_pstatus[part] = CLOSED;
		goto done;
	}

	part = dkpart(dev);
	if (part >= du->dk_lab.d_npartitions) return(ENXIO);
	du->dk_pstatus[part] = CLOSED;
	for (part=0; 
		(part<MAXPARTITIONS) && (du->dk_pstatus[part] == CLOSED); 
		part++);
	if (part == MAXPARTITIONS) du->dk_state = CLOSED;

done:
	if (du->dk_state == CLOSED) {
	   if (hdev->old) {
		hdev->old = FALSE;	/* Only unit open now closed */
#ifdef DEBUG
printf("old unit dev %d was closed",dev);
#endif
	   } else {
		DiscOpenCloseReq openreq;

		openreq.DevReq.Request = FG_Close;
		openreq.DevReq.Action = dev_openaction;
		openreq.DevReq.SubDevice = unit;
		openreq.DevReq.Timeout = -1;
		openreq.DevReq.Result = MYSPECCODE;	/* Special Code */
		InitSemaphore(&(openreq.WaitLock),0);
		Operate(dcb,&openreq);
		Wait(&(openreq.WaitLock));

		if (openreq.DevReq.Result)
		  printf("hd_close: Warning, Fault 0x%x on major dev %d\n",
			 (openreq.DevReq.Result==MYSPECCODE) ?
			   (SS_Device|EC_Error|EG_WrongFn|EO_Medium):
			   openreq.DevReq.Result,
			 major);
	    }
	}

	return(0);
}

/*
 * The ioctl routine.  
 */
int hd_ioctl(dev_t dev, int cmd, caddr_t addr, int flag)
{
	u_int major = major(dev);
	u_int unit = dkunit(dev);
	u_int part = dkpart(dev);
	struct disk *du;
	int error = 0;

#ifdef MONITOR
printf("hd_ioctl: Called with dev=%d,  cmd=0x%x,  flag=%d  by %s\n",
	dev,cmd,flag,myprocname(returnlink_(dev)));
printf("DIOCGDINFO:%x DIOCGPART:%x DIOCSDINFO:%x \n\tDIOCWLABEL:%x DIOCWDINFO:%x\n",
	DIOCGDINFO,DIOCGPART,DIOCSDINFO,DIOCWLABEL,DIOCWDINFO);
#endif

	if ((unit >= MAXDISKS) || (part >= MAXPARTITIONS)) return(ENXIO);
	du = &hd[major].disk[unit];

	switch(cmd) {
	case DIOCGDINFO:
		*(struct disklabel *)addr = du->dk_lab;
#ifdef DEBUG
printf("hd_ioctl: dev_bsize = %d\n",du->dk_lab.d_secsize);
#endif
		break;
	case DIOCGPART:
		((struct partinfo *)addr)->disklab = &du->dk_lab;
		((struct partinfo *)addr)->part =
			&du->dk_lab.d_partitions[part];
		break;
	case DIOCSDINFO:
		if ((flag & FWRITE) == 0)
			error = EBADF;
		else
			error = setdisklabel(&du->dk_lab,
					(struct disklabel *)addr,0);
		/* XXX AMS Set this info at controller as well. */
		break;
	case DIOCWLABEL:
		if ((flag & FWRITE) == 0)
			error = EBADF;
		else
			du->dk_wlabel = *(int *)addr;
		break;
	case DIOCWDINFO:
		if ((flag & FWRITE) == 0) {
			error = EBADF;
		} else if ((error = setdisklabel(&du->dk_lab,
					(struct disklabel *)addr,0)) == 0) {
			int wlab;

			/* XXX AMS Set this info at controller as well. */
			wlab = du->dk_wlabel;
			du->dk_wlabel = 1;
			error = writedisklabel(dev,hd_strategy,
						&du->dk_lab,part);
			du->dk_wlabel = wlab;			
		}
		break;
	default:
		error = ENOTTY;
		break;
	}

	return(error);
}

/*
 * This returns the size of the passed device in number of 512 blocks (?)
 */
int hd_size(dev_t dev)
{
	u_int major = major(dev);
	u_int unit = dkunit(dev);
	u_int part = dkpart(dev);
	struct disk *du;
	int val;

	if (unit >= MAXDISKS) return(-1);
	du = &hd[major].disk[unit];
	if (du->dk_state == CLOSED) {
		val = hd_open(dev,0,0);
		if (val < 0) return(-1);
	}
	return((int)((u_long)du->dk_lab.d_partitions[part].p_size *
		du->dk_lab.d_secsize / 512));
}

/*
 * This routine performs the unit core dump.  Since we are not UNIX
 * we don't.
 */
int hd_dump(dev_t dev)
{
	printf("hd_dump: PANIC called with dev=%d\n",dev);
	return(0);
}


