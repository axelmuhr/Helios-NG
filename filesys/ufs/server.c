/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 5 August 1992                                                */
/* File: server.c                                                     */
/*                                                                    */
/* This file contains the main routine for the ufs file server.       */
/* It initialises the device, starts up the device server, mounts     */
/* the root filing system and calls the server.                       */
/*                                                                    */
/* $Id: server.c,v 1.8 1993/04/08 14:49:20 nickc Exp $ */
/* $Log: server.c,v $
 * Revision 1.8  1993/04/08  14:49:20  nickc
 * changed displayed version number to 2.02 to reflect support for lstat()
 *
 * Revision 1.7  1992/11/30  09:54:40  al
 * Removed Beta from version number.
 *
 * Revision 1.6  1992/11/13  16:27:21  paul
 * GetSysBase now returns word address for C40, so fixed code accordingly
 *
 * Revision 1.5  1992/11/05  15:32:09  al
 * Fixed syntax error which NickC's compiler bitched about.
 *
 * Revision 1.4  1992/11/05  15:09:16  al
 * Version updated to Beta 2.01 which includes support for external symbolic
 * links.
 *
 * Revision 1.3  1992/10/19  11:34:17  al
 * Fixed to allow ufs to be installed in the nucleus.
 *
 * Revision 1.2  1992/10/05  16:26:50  al
 * Alpha test now reads Beta test.
 *
 * Revision 1.1  1992/09/16  09:29:06  al
 * Initial revision
 * */

#include "param.h"
#include "proc.h"
#include "user.h"
#include "malloc.h"
#include "buf.h"
#include "sem.h"
#include "queue.h"
#include "filedesc.h"
#include "mount.h"
#include "vnode.h"
#include "disklabel.h"

#include <helios.h>
#include <syslib.h>
#include <stdarg.h>
#include <servlib.h>
#include <codes.h>
#include <gsp.h>
#include <module.h>
#include <device.h>
#include <root.h>

/*
 * HELIOS filesystem resource variables.
 */
char *progname;		/* The name of this program. */
char *shortprogname;	/* The short name of this program */
char *canonname;	/* The canonical path name of the F/S under HELIOS */
char *mountname;	/* The name the root is mounted on in Helios. */
int debug2io = 0;	/* Output goes to I/O server screen  (default:no) */
Environ env;		/* The user environment */

static char version[] = 
	"Helios File System Version 2.02 (BSD 4.4 Compatible)\n";

void tokernel(void);
void fromkernel(void);
int init_unix(void);
void hd_init(void);
char *hd_start(char *devname, int major);
int hd_shutdown(int force);
void usage(void);
void dispatcher(Port p);
void *load_devinfo(void);
InfoNode *find_info(void *devinfo, word type, char *name);

/*
 * stdlib is not used, so define atoi here to work on +ve only.
 * A -ve is returned on failure.
 */
int atoi(char *num)
{	int val = 0;
	while ((*num >= '0') && (*num <= '9')) {
		val = (val*10) + (*num++ - '0');
	}
	if (*num) 
		return(-1);
	else	return(val);
}

/*
 * Conversion from hex to int.
 */
int hex2int(char *s)
{
	int n = 0;
	int i;
	for( i = 0; i < 8; i++ )
	{
		char c = s[i];
		if  ( '0' <= c && c <= '9' ) n = (n<<4) + c - '0';
		elif( 'a' <= c && c <= 'f' ) n = (n<<4) + c - 'a' + 10;
		else 			     n = (n<<4) + c - 'A' + 10;
	}
	return n;
}

/* 
 * Add a name to the name table, returning the object associated with it.
 */
Object *AddName(char *name, Port reqport)
{
	Object *o, *r;
	char mcname[100];
	NameInfo info;
	
	MachineName(mcname);
	o = Locate(NULL,mcname);
	
	info.Port = reqport;
	info.Flags = Flags_StripName;
	info.Matrix = DefNameMatrix;
	info.LoadData = NULL;
	
	r = Create(o, name, Type_Name, sizeof(NameInfo), (byte *)&info);

	Close(o);
	return(r);
}

/*
 * Return the malloced file name of the canonical path name of the
 * root mount point of the file system under Helios.
 */
char *make_canonical(char *mount)
{	char fullname[MAXPATHLEN];
	char *ret;

	MachineName(fullname);
	pathcat(fullname,mount);
	ret = (char *)Malloc(strlen(fullname)+1);
	strcpy(ret,fullname);
	return(ret);
}

/*
 * Return a ptr to the result of the argument in the environment.
 */
char *getenvarg(char *arg)
{	char **argvv = env.Envv;
	int notfound = TRUE;
	char *ptr1, *ptr2;

	while (*argvv && notfound) {
		for (ptr1 = *argvv++, ptr2 = arg;
			*ptr1 && *ptr2 && (*ptr1 == *ptr2);
			ptr1++, ptr2++);
		if ((*ptr1 == '=') && (!*ptr2))
			return(++ptr1);
	}
	return(NULL);
}

/*
 * The MAIN routine.
 */
extern struct filedesc0 filedesc0;
extern int (*mountroot)();
extern struct vnode *rootvp;
extern int freebufspace;
extern int maxfiles;
uid_t rootuid;
gid_t rootgid;
bool in_nucleus;

int main(void)
{
	char **argv;
	int argc;
	int val, dbgtmp;
	char *rootdevname, *devname, *opt, *mountpoint;
	dev_t rootdev;
	int root_major = 0;
	Port ufs_port;
	Object *ufs_object;
	void *devinfo;

#ifdef __TRAN
	in_nucleus = (word)main < (word)GetRoot(); /* better test needed */
#else
# ifdef __C40
	  {
	    word base = GetSysBase();	/* returns word address on C40 */

	    in_nucleus = (
			  (base < (word)main) &&
			  ((word)main < base + WP_GetWord(base, 0))
			  );
	  }
# else
	  {
	    word *	base = GetSysBase();

	    in_nucleus = (
			  (base < (word *)main) &&
			  ((word *)main < base + *base)
			  );
	  }
# endif
#endif /* __TRAN */

	if (in_nucleus) {
		void *devinfo;
		InfoNode *filesysinfo, *deviceinfo;
		FileSysInfo *fsi;
		DiscDevInfo *ddi;
		DriveInfo *di;
		
		/* Set up default values */
		shortprogname = progname = "ufs";
		rootuid = rootgid = 0;

		/* If helios already exists, then install as ufs */

		mountpoint = "helios";
		if ((ufs_object = Locate(NULL,"/helios")) != NULL) {
			Close(ufs_object);	/* It exists */
			mountpoint = "ufs";	/* Mount as ufs */
		}
		mountname = (char *)Malloc(strlen(mountpoint)+2);
		mountname[0] = '/';	mountname[1] = '\0';
		strcat(mountname,mountpoint);
		canonname = make_canonical(mountpoint);
		GetRoot()->Flags |= Root_Flags_special;

		/* Setup the Helios Device World */
		hd_init();

		/* Check if there is a logger */
		if ((ufs_object = Locate(NULL,"/logger")) == NULL) {
			debug2io = 2;	/* Output to nowhere */
		} else {
			debug2io = 1;	/* Output to logger */
			Close(ufs_object);
		}

		/* Look for helios filesystem in devinfo */
		if ((devinfo = load_devinfo()) == NULL) {
			printf("Failed to load devinfo file");
			Free(mountname);
			Exit(1);
		}

		/* Is entry there ? */
		if ((filesysinfo = find_info(devinfo,Info_FileSys,mountpoint)) 
		   == NULL) {
			printf("No entry for filesystem %s in devinfo",
				mountpoint);
			Free(devinfo);
			Free(mountname);
			Exit(1);
		}
		fsi = (FileSysInfo *)RTOA(filesysinfo->Info);

		/* Set up the root name and other defaults */
		rootdevname = RTOA(fsi->DeviceName);
		if (fsi->CacheSize >= 16*NBPG) {
			freebufspace = fsi->CacheSize;
			nbuf = freebufspace / NBPG;
		}
		if (fsi->MaxInodes >= 64) {
			desiredvnodes = fsi->MaxInodes;
			maxfiles = desiredvnodes;
		}
		
		/* Look for the root device unit number */
		if ((deviceinfo = find_info(devinfo,Info_DiscDev,rootdevname)) 
		   == NULL) {
			printf("No entry for device %s in devinfo",
				rootdevname);
			Free(devinfo);
			Free(mountname);
			Exit(1);
		}
		ddi = (DiscDevInfo *)RTOA(deviceinfo->Info);
		di = (DriveInfo *)RTOA(ddi->Drives);	/* 1st drive only */
		if ((rootdev = di->DriveId) == -1) {
			rootdev = 0;	/* This is the default */
		}
		rootdev = makedev(root_major,dkminor(rootdev,0));
		if ((opt = hd_start(rootdevname,root_major)) != NULL) {
			printf("Rootdev (major %d) %s:%d, %s\n",
				root_major,rootdevname,val,opt);
			Free(mountname);
			Exit(1);
		}
	} else { /* Not in_nucleus */
		/* Initialise Environment */
		GetEnv(MyTask->Port, &env);

		/* Get program name from environment */
		for (argc=0, argv = env.Argv; *argv; argv++, argc++);
		argv = env.Argv;
		progname = *argv++;
		argc--; 

		/* Find the short name */
		shortprogname = progname + strlen(progname) - 1;
		while ((shortprogname > progname) && (*shortprogname != '/')) 
			shortprogname--;
		if (*shortprogname == '/') shortprogname++;

		/* Find and set the startup user and group ids */
		if ((opt = getenvarg("_UID")) == NULL)
			rootuid = 0;
		else rootuid = hex2int(opt);
		if ((opt = getenvarg("_GID")) == NULL)
			rootgid = 0;
		else rootgid = hex2int(opt);
	
		/* Save the default IO destination, and force IO to console */
		dbgtmp = debug2io;
		debug2io = 0;

		/* Parse the arguments */
		while ((**argv == '-') && argc) {
			opt = *argv + 1;
			switch (*opt) {
			case 'd':
				/* debug messages required */
				dbgtmp = 1;
				break;
			case 'c':
				/* Change in the cache size */
				argv++;	argc--;
				val = atoi(*argv);
				if (val < 16*NBPG)
					printf("Invalid cache size %s. -Ignored.\n",*argv);
				else freebufspace = val;
				break;
			case 'b':
				/* Change in the number of buffers */
				argv++;	argc--;
				val = atoi(*argv);
				if (val < 20)
					printf("Invalid number of buffers %s. -Ignored.\n",*argv);
				else nbuf = val;
				break;
			case 'f':
				/* Change in the maximum open files */
				argv++;	argc--;
				val = atoi(*argv);
				if (val < 20)
					printf("Invalid open file maximum %s. -Ignored.\n",*argv);
				else maxfiles = val;
				break;
			case 'v':
				/* Change in the number of desired vnodes */
				argv++;	argc--;
				val = atoi(*argv);
				if (val < 64)
					printf("Invalid vnode maximum %s. -Ignored.\n",*argv);
				else desiredvnodes = val;
				break;
			case 'r':
				/* Major device number for root device */
				argv++; argc--;
				val = atoi(*argv);
				if ((val < 0) || (val > MAX_HELIOS_DEVICES))
					printf("Invalid major root device number %s. -Ignored.\n",*argv);
				else root_major=val;
			default:
				printf("Unknown option '-%c'\n",*opt);
				usage();
				break;
			}
			argv++;	argc--;
		}

		/* Setup the mount name */
		if (argc < 2) usage();
		mountpoint = *argv++;	argc--;
		mountname = (char *)Malloc(strlen(mountpoint)+2);
		mountname[0] = '/';	mountname[1] = '\0';
		strcat(mountname,mountpoint);
		canonname = make_canonical(mountpoint);

		/* Setup the Helios Device World */
		hd_init();

		/* Extract the major device number */
		rootdevname = *argv++;	argc--;
		for (opt=rootdevname; *opt && (*opt != ':'); opt++);
		if (!*opt) {
			printf("Expected unit number after root device name %s\n",
				rootdevname);
			usage();
		}
		*(opt++) = '\0';
		if ((val = atoi(opt)) < 0) {
			printf("Invalid unit number %s for root %s\n",opt,rootdevname);
			usage();
		}
		rootdev = makedev(root_major,dkminor(val,0));
		if ((opt = hd_start(rootdevname,root_major)) != NULL) {
			printf("Rootdev (major %d) %s:%d, %s\n",
				root_major,rootdevname,val,opt);
			Free(mountname);
			Exit(1);
		}

		/* Open any other devices */
		if (root_major == 0) val=1; else val=0;
		while (argc && (val < MAX_HELIOS_DEVICES)) {
			/* Extract the device name and unit number */
			devname = *argv++;	argc--;
			/* Startup device driver for it. */
			if ((opt = hd_start(devname,val)) != NULL) {
				printf("Major device %d, %s: %s\n",val,devname,opt);
				hd_shutdown(TRUE);
				Free(mountname);
				Exit(1);
			}

			if (++val == root_major) val++;
		}
		if (argc) {
			printf("Remaining arguments from and after %s ignored\n",
				*argv);
		}

		/* Restore the default IO destination */
		debug2io = dbgtmp;
	} /* Not in_nucleus */

	/* Print startup message */
	printf(version);

	/* Initialise UNIX and device driver routines */
	if (init_unix()) {
		panic("HELIOS; Failed to initialise UNIX kernel");
		goto shutdown;
	}

	/* Go into the kernel */
	tokernel();

	/* Start mounting the root filesystem */
	printf("Mounting root filesystem on device '%s' dev:%d  Mount point '%s'\n",
		rootdevname,rootdev,mountname);
	vfsinit();
	if (bdevvp(rootdev,&rootvp)) {
		panic("can't setup bdevvp for rootdev");
		goto shutdown;
	}
	if ((*mountroot)()) {
		panic("cannot mount root");
		goto shutdown;
	}

	/* Get vnode for '/' */
	if (VFS_ROOT(rootfs, &rootdir)) {
		panic("cannot find root vnode");
		goto shutdown;
	}
	filedesc0.fd_fd.fd_cdir = rootdir;
	VREF(filedesc0.fd_fd.fd_cdir);
	VOP_UNLOCK(rootdir);
	filedesc0.fd_fd.fd_rdir = NULL;
	printf("Disk mounted O.K.\n");

	/* Setup a Helios Message Port */
	if ((ufs_port = NewPort()) == NullPort) {
		panic("HELIOS; Failed to obtain a message port");
		goto shutdown;
	}
	if ((ufs_object = AddName(mountpoint,ufs_port)) == NULL) {
		panic("HELIOS; Failed to create the name table entry");
		FreePort(ufs_port);
		goto shutdown;
	}

	/* Root process is done */
	fromkernel();

	if (in_nucleus) {
		/* Reply to procman that we have started */
		MCB m;
		word e;
		InitMCB(&m,0,MyTask->Parent,NullPort,0x456);
		e = PutMsg(&m);

		/* Free other resources */
		Free(devinfo);
	}

	/* Run the dispatcher */
	dispatcher(ufs_port);

	/* Free the message port and other Helios objects */
	FreePort(ufs_port);
	Delete(ufs_object, NULL);

	/* ShutDown */
shutdown:
	printf("Shutting down\n");	dbgtmp = 0;
	while ((val = hd_shutdown((dbgtmp < 120)?FALSE:TRUE)) > 0) {
		Delay(OneSec);		/* Wait until all buffers flushed */
		printf("%d disk operations outstanding, waiting ...\n",val);
		dbgtmp++;
	}
	Free(canonname);
	Free(mountname);
	printf("Down\n");
	Exit(0);
}

/*
 * This is the usage message.
 */
void usage(void)
{
	printf("usage; %s [-d] [-c cachesize] [-b numbuf] [-f maxfile] [-v maxvnode]\n",
		progname);
	printf("\t[-r root] mountpoint discdevice:unit [discdevice ...]\n");
	Exit (1);
}


