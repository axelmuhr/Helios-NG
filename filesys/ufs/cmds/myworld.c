/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 25 August 1992                                               */
/* File: myworld.c                                                    */
/* 
 * This file contains the routines for doing reads/write/ioctls etc
 * to/from a file or device.  It will return its own file descriptors.
 * A maximum of 20 files is allowed.  In addition, either all the files
 * must really be files, or all the files must refer to devices.
 * A maximum of 20 files are allowed.  The routine MyWorld() is called
 * with a flag indicating whether the files are devices (TRUE) or
 * really devices.  It will return 0 if the switch succedded and false
 * otherwise.
 * Notes: (1) Only one device is permitted, but each may have many units.
 *            All the devices are read/write.
 *        (2) A Unit number must follow a colon after the device name
 *            device name when devices are active and opened.
 *        (3) You cannot switch worlds when any files are open.
 *        (4) All programs using this must include "myworld.h" since
 *            open, read, write, close, lseek and ioctl are redefined
 *            MyOpen, MyRead, MyWrite, MyClose, MyLseek, MyIoctl
 */
/* $Id: myworld.c,v 1.3 1993/03/29 09:57:45 nickc Exp $ */
/* $Log: myworld.c,v $
 * Revision 1.3  1993/03/29  09:57:45  nickc
 * Fixed problem where newfs opened two handles to the device.
 * One for reading and one for writing, instead of one for r/w.
 *
 * Revision 1.2  1992/12/03  16:27:38  al
 * Changed MyStat to return a character special device rather than block
 *
 * Revision 1.1  1992/09/16  10:01:43  al
 * Initial revision
 * */

#include "param.h"
#include "errno.h"
#include "types.h"
#include "stat.h"
#include "ioctl.h"
#define DFLT_SIZES
#include "disklabel.h"

#include <helios.h>
#include <stdlib.h>
#include <stdio.h>
#include <gsp.h>
#include <module.h>
#include <sem.h>
#include <device.h>
#include <codes.h>

#define MAXFILES 20

extern int errno;			/* The posix error number */
static int InMyWorld = FALSE;		/* Defines whether device or files */
static int MyNumOpen = 0;		/* Number of files actually open */
static char MyDevName[256];		/* Name of the device */
static int MyDevIsOld = 0;		/* Old type device driver */
static DCB *MyDCB = NULL;		/* Device Control Block */
static int MyUnit[MAXFILES];		/* Only one device but 20 units */
static int MyPos[MAXFILES];		/* The position within the files */
static int MyAtExit = FALSE;		/* Installed ? */

/*************************************************************************/
/*                          Internal Routines                            */
/*************************************************************************/


/*
 * DEVINFO operations
 */

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

void dev_action(DiscReq *req)
{
  Signal(&req->WaitLock);
}

/* This is the absolute read/write routine */
word do_dev_rdwr(word fn, word dev, word timeout, word size, word pos,
				char *buf, word *deverror )
{
  DiscReq req;

  req.DevReq.Request = fn;
  req.DevReq.Action = dev_action;
  req.DevReq.SubDevice = dev;
  req.DevReq.Timeout = timeout;
  req.Size = size;
  req.Pos  = pos;
  req.Buf = buf;

#ifdef DEBUG
	IOdebug("do_dev_rdwr: function %d,subdevice = %d, no. %d, size %d",
				fn,req.DevReq.SubDevice,pos,size);
#endif

  InitSemaphore(&(req.WaitLock),0);
  Operate(MyDCB,&req);
  Wait(&(req.WaitLock));

#ifdef DEBUG
	IOdebug("do_dev_rdwr: done");
#endif
  *deverror = req.DevReq.Result;
  return req.Actual;
}

/* Subunit open and close */
void dev_openaction(DiscOpenCloseReq *req)
{
  Signal(&req->WaitLock);
}

#define MYSPECCODE 0x13126521

word do_dev_open(word fn, word unit, word timeout)
{
  DiscOpenCloseReq req;

  req.DevReq.Request = fn;
  req.DevReq.Action = dev_openaction;
  req.DevReq.SubDevice = unit;
  req.DevReq.Timeout = timeout;
  req.DevReq.Result = MYSPECCODE;	/* Special Code */

#ifdef DEBUG
	IOdebug("do_dev_open: function %d,subdevice = %d",
		fn,req.DevReq.SubDevice);
#endif

  InitSemaphore(&(req.WaitLock),0);
  Operate(MyDCB,&req);
  Wait(&(req.WaitLock));

#ifdef DEBUG
	IOdebug("do_dev_open: done %d unit %d",fn,req,DevReq,SubDevice);
#endif
  if (req.DevReq.Result == MYSPECCODE)
	/* Proper error code */
	return(SS_Device | EC_Error | EG_WrongFn | EO_Medium);	
  else	return(req.DevReq.Result);
}
word do_dev_setinfo(word unit, word type, word secsize, word nsectors,
			word ntracks, word ncylinders, word timeout)
{
  DiscParameterReq dpreq;

  /* Standard Request */
  dpreq.DevReq.Request = FG_SetInfo;
  dpreq.DevReq.Result = MYSPECCODE;
  dpreq.DevReq.Action = dev_openaction;
  dpreq.DevReq.SubDevice = unit;
  dpreq.DevReq.Timeout = timeout;
  InitSemaphore(&dpreq.WaitLock,0);

  /* Setup SetInfo parameters */
  dpreq.DriveType = type;	/* Fixed Disk ? */
  dpreq.SectorSize = secsize;
  dpreq.SectorsPerTrack = nsectors;
  dpreq.TracksPerCyl = ntracks;
  dpreq.Cylinders = ncylinders;

  /* Make it */
  Operate(MyDCB,&dpreq);
  Wait(&(dpreq.WaitLock));
			
  if (dpreq.DevReq.Result == MYSPECCODE)
	/* Proper error code */
	return(SS_Device | EC_Error | EG_WrongFn | EO_Medium);	
  else	return(dpreq.DevReq.Result);
}

ffs(mask)
	register long mask;
{
	register int bit;

	if (!mask)
		return(0);
	for (bit = 1;; ++bit) {
		if (mask&0x01)
			return(bit);
		mask >>= 1;
	}
}

/*
 * Compute checksum for disk label.
 */
dkcksum(lp)
	register struct disklabel *lp;
{
	register u_short *start, *end;
	register u_short sum = 0;

	start = (u_short *)lp;
	end = (u_short *)&lp->d_partitions[lp->d_npartitions];
	while (start < end)
		sum ^= *start++;
	return (sum);
}

/*
 * Attempt to read a disk label from a device.
 * The label must be partly set up before this:
 * secpercyl and anything required in the strategy routine
 * (e.g., sector size) must be filled in before calling us.
 * Returns null on success and an error string on failure.
 */
char *
readdisklabel(fd, lp)
	int fd;
	register struct disklabel *lp;
{
	register char *buf;
	struct disklabel *dlp;
	char *msg = NULL;
	word n, err;

	if ((fd < 0) || (fd >= MAXFILES) || (MyUnit[fd] == -1)) {
		errno = EBADF;
		return("Invalid file descriptor");
	}
	if (lp->d_secperunit == 0)
		lp->d_secperunit = 0x1fffffff;
	lp->d_npartitions = 1;
	if (lp->d_partitions[0].p_size == 0)
		lp->d_partitions[0].p_size = 0x1fffffff;
	lp->d_partitions[0].p_offset = 0;

	/* Get the memory to store the label */
	buf = (char *)Malloc((word)lp->d_secsize);

	/* Read the label */
	n = do_dev_rdwr(FG_Read,MyUnit[fd],-1,(word)lp->d_secsize,
			LABELSECTOR*lp->d_secsize,buf,&err);
        if (n != lp->d_secsize) {
		msg = "I/O error, failed to read correct size";
		IOdebug("readdisklabe2: Disk drive only read %d when requested to read %d",
				n,lp->d_secsize);
	} else if (err) {
		msg = "I/O error";
		IOdebug("Disk drive error 0x%x\n",err);
	} else for (dlp = (struct disklabel *)buf;
	    dlp <= (struct disklabel *)(buf+DEV_BSIZE-sizeof(*dlp));
	    dlp = (struct disklabel *)((char *)dlp + sizeof(long))) {
		if (dlp->d_magic != DISKMAGIC || dlp->d_magic2 != DISKMAGIC) {
			if (msg == NULL)
				msg = "no disk label";
		} else if (dlp->d_npartitions > MAXPARTITIONS ||
			   dkcksum(dlp) != 0)
			msg = "disk label corrupted";
		else {
			*lp = *dlp;
			msg = NULL;
			break;
		}
	}
	Free(buf);
	return (msg);
}

/*
 * Check new disk label for sensibility
 * before setting it.
 */
setdisklabel(olp, nlp, openmask)
	register struct disklabel *olp, *nlp;
	u_long openmask;
{
	register i;
	register struct partition *opp, *npp;

	if (nlp->d_magic != DISKMAGIC || nlp->d_magic2 != DISKMAGIC ||
	    dkcksum(nlp) != 0)
		return (EINVAL);
	while ((i = ffs((long)openmask)) != 0) {
		i--;
		openmask &= ~(1 << i);
		if (nlp->d_npartitions <= i)
			return (EBUSY);
		opp = &olp->d_partitions[i];
		npp = &nlp->d_partitions[i];
		if (npp->p_offset != opp->p_offset || npp->p_size < opp->p_size)
			return (EBUSY);
		/*
		 * Copy internally-set partition information
		 * if new label doesn't include it.		XXX
		 */
		if (npp->p_fstype == FS_UNUSED && opp->p_fstype != FS_UNUSED) {
			npp->p_fstype = opp->p_fstype;
			npp->p_fsize = opp->p_fsize;
			npp->p_frag = opp->p_frag;
			npp->p_cpg = opp->p_cpg;
		}
	}
 	nlp->d_checksum = 0;
 	nlp->d_checksum = dkcksum(nlp);
	*olp = *nlp;
	return (0);
}

/*
 * Write disk label back to device after modification.
 */
writedisklabel(fd, lp)
	int fd;
	register struct disklabel *lp;
{
	char *buf;
	struct disklabel *dlp;
	int labelpart;
	word n, error = 0;

	if ((fd < 0) || (fd >= MAXFILES) || (MyUnit[fd] == -1)) {
		return(EBADF);
	}
	labelpart = 0;	/* We are talking the root partition */
	if (lp->d_partitions[labelpart].p_offset != 0) {
		return (EXDEV);			/* not quite right */
	}

	buf = (char*)Malloc((word)lp->d_secsize);

	/* Read in current partition info */
	n = do_dev_rdwr(FG_Read,MyUnit[fd],-1,(word)lp->d_secsize,
			LABELSECTOR*lp->d_secsize,buf,&error);
        if (n != lp->d_secsize) {
		IOdebug("writedisklabel2: Disk drive only read %d when requested to read %d",n,lp->d_secsize);
	} else if (error) {
		IOdebug("Disk drive error %d\n",error);
	}
	if (error || (n != lp->d_secsize)) goto done;
	for (dlp = (struct disklabel *)buf;
	    dlp <= (struct disklabel *)
	      (buf + lp->d_secsize - sizeof(*dlp));
	    dlp = (struct disklabel *)((char *)dlp + sizeof(long))) {
		if (dlp->d_magic == DISKMAGIC && dlp->d_magic2 == DISKMAGIC &&
		    dkcksum(dlp) == 0) {
			*dlp = *lp;
			n = do_dev_rdwr(FG_Write,MyUnit[fd],-1,
					(word)lp->d_secsize,
					LABELSECTOR*lp->d_secsize,
					buf,&error);
		        if (n != lp->d_secsize) {
				IOdebug("Disk drive only read %d when requested to read %d",n,lp->d_secsize);
			} else if (error) {
				IOdebug("Disk drive error %d\n",error);
			}
			if (!MyDevIsOld) {
			   /* Try and update parameters in driver */
			   if ((error = do_dev_setinfo(MyUnit[fd],
						0,	/* Fixed Disk ? */
						lp->d_secsize,
						lp->d_nsectors,
						lp->d_ntracks,
						lp->d_ncylinders,
						-1)) < 0) {
				IOdebug("Fault %x setting disc unit %d parameters\n",
				  error,MyUnit[fd]);
				errno = ENXIO;
			   } else error = 0;
			}
			goto done;
		}
	}
	error = ESRCH;
done:
	Free(buf);
	return (error);
}

/*
 * Close the device on exiting.
 */
void DoMyExit(void)
{	int i;

	for (i=0; i<MAXFILES; i++) {
		MyUnit[i] = -1;
		MyPos[i] = 0;
	}
	if (MyDCB) {
		CloseDevice(MyDCB);
		MyDCB = NULL;
		MyDevName[0] = '\0';
	}
}

/*************************************************************************/
/*                          External Routines                             */
/*************************************************************************/

/*
 * The Switch!
 */
int MyWorld(int flag)
{	int i;

	if (MyNumOpen)
		return(TRUE);	/* Cannot switch when files are open */

	/* Initialise */
	InMyWorld = flag;
	for (i=0; i<MAXFILES; i++) {
		MyUnit[i] = -1;
		MyPos[i] = 0;
	}
	MyDevName[0] = '\0';
	if (!MyAtExit) {
		MyAtExit++;
		atexit(DoMyExit);
	}

	return(FALSE);
}

/*
 * Open file/device.
 */
int MyOpen(char *name, int mode)
{	int ret, unit;
	void *devinfo;
	InfoNode *deviceinfo;
	DiscDevInfo *ddi;
	char *ch;
	int i;

#ifdef DEBUG
	IOdebug("MyOpen: opening %s mode %d",name,mode);
#endif
	if (InMyWorld) {
		struct disklabel label;

		/* Get the unit number */
		for(ch=name; *ch && (*ch != ':'); ch++);
		if (!*(ch++) || ((unit = atoi(ch)) < 0)) {
			errno = ENOENT;
			return(-1);
		}

		/* Find a free descriptor */
		for (ret=0; (ret < MAXFILES) && (MyUnit[ret] != -1); ret++);
		if (ret == MAXFILES) {
			errno = ENFILE;
			fprintf(stderr,"MyOpen: PANIC Cannot find free\n");
			return(-1);
		}

		/* 1st file descriptor ? */
		if (MyDevName[0] == '\0') {
			strcpy(MyDevName,name);
			for (ch=MyDevName; *ch && (*ch != ':'); ch++);
			*ch = '\0';	/* Remove Unit? */

			/* Get the device */
			if ((devinfo = load_devinfo()) == NULL) {
				fprintf(stderr,"MyOpen: Cannot open devinfo\n");
				errno = EIO;
				MyDevName[0] = '\0';
				return(-1);
			}
			if ((deviceinfo = find_info(devinfo,Info_DiscDev,
						MyDevName)) == NULL) {
				errno = ENOENT;
				fprintf(stderr,"MyOpen: Cannot find info\n");
				MyDevName[0] = '\0';
				Free(devinfo);
				return(-1);
			}
			ddi = (DiscDevInfo *)RTOA(deviceinfo->Info);

			/* Open the device */
			if ((MyDCB = OpenDevice(RTOA(ddi->Name),ddi)) == NULL) {
				errno = ENOENT;
				fprintf(stderr,"MyOpen: Cannot open device\n");
				MyDevName[0] = '\0';
				Free(devinfo);
				return(-1);
			}

			Free(devinfo);
		} else {
			/* Device names MUST match */
			for (i=0, ch=name; 
			     (i<strlen(MyDevName)) && (*ch==MyDevName[i]);
			     i++, ch++);
			if ((i != strlen(MyDevName)) || (*ch != ':')) {
				fprintf(stderr,"MyOpen: Only 1 device driver supported\n");
				errno = ENFILE;
				return(-1);
			}
		}

		/* Set the unit number */
		MyUnit[ret] = unit;
		MyPos[ret] = 0;		/* At beginning of disk */

		/* Open the subdevice */
		if ((i = do_dev_open(FG_Open,unit,-1)) < 0) {
			/* Is this an old driver ? */
			if ((EG_Mask & i) == EG_WrongFn) {
				/* Yes, nothing else can be open */
				MyDevIsOld = 1;
#ifdef 0
				if (MyNumOpen) {
					fprintf(stderr,"MyOpen: Device driver supports only 1 Unit\n");
					errno = EBUSY;
					return(-1);
				}
#endif
			} else {
				/* Something else went wrong */
				fprintf(stderr,"MyOpen: Fault %x on %s open\n",i,name);
				errno = EIO;
				return(-1);
			}
		} else MyDevIsOld = 0;

		/* No use in reading label if device is old */
		if (!MyDevIsOld) {
		    /* Setup the default disk label */
		    memcpy(&label,&dflt_sizes,sizeof(struct disklabel));

		    /* Read the disk label */
		    if ((ch = readdisklabel(ret,&label)) == NULL) {
			/* Label is read, setup parameters */
			if ((i = do_dev_setinfo(unit,
						0,	/* Fixed Disk ? */
						label.d_secsize,
						label.d_nsectors,
						label.d_ntracks,
						label.d_ncylinders,
						-1)) < 0) {
			  fprintf(stderr,
				  "MyOpen: Fault %x setting %s parameters\n",
				  i,name);
			}
		    }
		    /* Ignore actual readdisklabel errors */
		}
	} else {
		ret = open(name,mode);
	}
	if (ret >= 0) MyNumOpen++;
#ifdef DEBUG
	IOdebug("MyOpen: opening %s OK, fd=%d",name,ret);
#endif
	return(ret);
}

/*
 * Close File/Device.
 */
int MyClose(int fd)
{
	int ret;

#ifdef DEBUG
	IOdebug("MyClose: close fd %d",fd);
#endif
	if (InMyWorld) {
		if ((fd >= 0) && (fd < MAXFILES) && (MyUnit[fd] != -1)) {
			/* Close the sub unit (if valid) */
			if (!MyDevIsOld) {
				do_dev_open(FG_Close,MyUnit[fd],-1);
			}

			MyUnit[fd] = -1;
			ret = 0;
		} else {
			ret = -1;
			errno = EBADF;
		}
	} else ret = close(fd);

	if (ret == 0) 
		if (--MyNumOpen == 0) DoMyExit();
#ifdef DEBUG
	IOdebug("MyClose: close fd %d  err=%d",fd,ret);
#endif
	return(ret);
}

/*
 * Seek to a location in the file/device.
 */
long MyLseek(int fd, int offset, int whence)
{
	int ret;

#ifdef DEBUG
	IOdebug("MyLseek: fd %d  offset %d   whence %d",fd,offset,whence);
#endif
	if (InMyWorld) {
		if ((fd < 0) || (fd >= MAXFILES) || (MyUnit[fd] == -1)) {
			errno = EBADF;
			return(-1);
		}
		switch(whence) {
		case SEEK_CUR:
			offset += MyPos[fd];
			/* Any fall into ... */
		case SEEK_SET:
			if (offset>=0) {
				MyPos[fd] = offset;
				ret = offset;
			} else {
				errno = EINVAL;
				return(-1);
			}
			break;
		case SEEK_END:	/* There is no end in a device */
		default:
			errno = EINVAL;
			return(-1);
		}
	} else ret = lseek(fd,offset,whence);
#ifdef DEBUG
	IOdebug("MyLseek: fd %d  offset %d   whence %d   err=%d",
			fd,offset,whence,ret);
#endif
	return(ret);
}

int MyIoctl(int fd, int cmd, char *addr)
{
	int ret;
	char *msg;
	
#ifdef DEBUG
	IOdebug("MyIctl: fd %d  cmd 0x%x",fd,cmd);
#endif
	if (InMyWorld) {
		if ((fd < 0) || (fd >= MAXFILES) || (MyUnit[fd] == -1)) {
			errno = EBADF;
			return(-1);
		}
		switch(cmd) {
		case DIOCGDINFO:
			/* Setup the default disk label */
			memcpy(addr,&dflt_sizes,sizeof(struct disklabel));

			/* Read and return the disk label */
			if ((msg = readdisklabel(fd,(struct disklabel *)addr))
			    != NULL) {
				fprintf(stderr,"Reading disk label: %s\n",msg);
				errno = EIO;
				return(-1);
			}
			ret = 0;
			break;
		case DIOCSDINFO:
			ret = setdisklabel(&dflt_sizes,
					(struct disklabel *)addr,0);
			break;
		case DIOCWLABEL:
			/* Ignore this as ours is always writeable */
			ret = 0;
			break;
		case DIOCWDINFO:
			if ((ret = setdisklabel(&dflt_sizes,
			    (struct disklabel *)addr,0)) != 0) {
				errno = ret;
				return(-1);
			}
			if ((ret = writedisklabel(fd,
			    (struct disklabel *)addr)) != 0) {
				errno = ret;
				return(-1);
			}
			break;
		default:
			fprintf(stderr,"MyIoctl: Invalid command 0x%x\n",cmd);
			errno = ENOTTY;
			return(-1);
		}
	} else ret = ioctl(fd,cmd,addr);
#ifdef DEBUG
	IOdebug("MyIctl: fd %d  cmd 0x%x  err=%d",fd,cmd,ret);
#endif
	return(ret);
}

/*
 * The read operation.
 */
int MyRead(int fd, char *buf, unsigned int num)
{
	int ret;
	word deverr;
	
#ifdef DEBUG
	IOdebug("MyRead: fd %d  num %d",fd,num);
#endif
	if (InMyWorld) {
		if ((fd < 0) || (fd >= MAXFILES) || (MyUnit[fd] == -1)) {
			errno = EBADF;
			return(-1);
		}

		/* Read from the device */
		ret = do_dev_rdwr(FG_Read, MyUnit[fd], -1, num, MyPos[fd],
					buf, &deverr);
		if (deverr) {
			errno = EIO;
			return(-1);
		}
		MyPos[fd] += ret;
	} else ret = read(fd,buf,num);
#ifdef DEBUG
	IOdebug("MyRead: fd %d  num %d  err=%d",fd,num,ret);
#endif
	return(ret);
}

/*
 * The write operation
 */
int MyWrite(int fd, char *buf, unsigned int num)
{
	int ret;
	word deverr;

#ifdef DEBUG
	IOdebug("MyWrite: fd %d  num %d",fd,num);
#endif
	if (InMyWorld) {
		if ((fd < 0) || (fd >= MAXFILES) || (MyUnit[fd] == -1)) {
			errno = EBADF;
			return(-1);
		}

		/* Write to the device */
		ret = do_dev_rdwr(FG_Write, MyUnit[fd], -1, num, MyPos[fd],
					buf, &deverr);
		if (deverr) {
			errno = EIO;
			return(-1);
		}
		MyPos[fd] += ret;
	} else ret = write(fd,buf,num);
#ifdef DEBUG
	IOdebug("MyWrite: fd %d  num %d  err=%d",fd,num,ret);
#endif
	return(ret);
}

/*
 * Do the stat call.
 */
int MyStat(char *path, struct stat *buf)
{
	int ret;
	if (InMyWorld) {
		/* Fake a stat call */
		buf->st_dev = 0;	/* inode's device */
		buf->st_ino = 0;	/* inode's number */
		buf->st_mode = S_IFCHR;	/* inode protection mode */
		buf->st_nlink = 1;	/* number of hard links */
		buf->st_uid = 0;	/* user ID of the file's owner */
		buf->st_gid = 0;	/* group ID of the file's group */
		buf->st_rdev = 0;	/* device type */
		buf->st_size = MinInt;	/* file size, in bytes */
		buf->st_atime = 0;	/* time of last access */
		buf->st_mtime = 0;	/* time of last data modification */
		buf->st_ctime = 0;	/* time of last file status change */
		buf->st_blksize = 512;	/* optimal blocksize for I/O */
		buf->st_blocks = 0;	/* blocks allocated for file */
		buf->st_flags = 0;	/* user defined flags for file */
		buf->st_gen = 0;	/* file generation number */
		ret = 0;
	} else ret = stat(path,buf);
	return(ret);
}

