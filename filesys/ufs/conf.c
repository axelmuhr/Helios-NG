/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 29 July 1992                                                 */
/* File: conf.c                                                       */
/*                                                                    */
/* This file contains the configuration parameters for the UFS under  */
/* helios.                                                            */
/*                                                                    */
/* $Id: conf.c,v 1.2 1992/10/13 11:47:03 al Exp $ */
/* $Log: conf.c,v $
 * Revision 1.2  1992/10/13  11:47:03  al
 * Syntax fixed to compile for C40
 *
 * Revision 1.1  1992/09/16  09:29:06  al
 * Initial revision
 * */

#include "sys/param.h"
#include "sys/systm.h"
#include "sys/mount.h"
#include "sys/buf.h"
#include "sys/vnode.h"
#include "sys/ioctl.h"
#include "sys/conf.h"
#include "sys/proc.h"

extern int nullop(), enxio(), enodev();

/* System parameters */
#define MAXUSERS 20
#define	NPROC (20 + 16 * MAXUSERS)
#define	NTEXT (80 + NPROC / 8)			/* actually the object cache */
#define	NVNODE (NPROC + NTEXT + 100)

int	maxproc = NPROC;
long	desiredvnodes = NVNODE;
int	maxfiles = 3 * (NPROC + MAXUSERS) + 80;
int 	nbuf = 400;	/* Think of a number */
int 	freebufspace = 400*NBPG;
int 	allocbufspace = 0;

/* The block device driver definitions */
/* Currently only one: hard disk.      */
int	hd_open(dev_t dev, int oflags, int devtype, struct proc *p);
int	hd_close(dev_t dev, int fflag, int devtype, struct proc *p);
int	hd_strategy(struct buf *bp);
int	hd_ioctl(dev_t dev, int cmd, caddr_t data, int fflag, struct proc *p);
int	hd_dump(dev_t dev);
int	hd_size(dev_t dev);

#define NBLKDEV MAX_HELIOS_DEVICES
struct bdevsw bdevsw[NBLKDEV] = {
{  hd_open,	hd_close,	hd_strategy,	hd_ioctl,
   hd_dump,	hd_size,	NULL},
{  hd_open,	hd_close,	hd_strategy,	hd_ioctl,
   hd_dump,	hd_size,	NULL},
{  hd_open,	hd_close,	hd_strategy,	hd_ioctl,
   hd_dump,	hd_size,	NULL},
{  hd_open,	hd_close,	hd_strategy,	hd_ioctl,
   hd_dump,	hd_size,	NULL},
{  hd_open,	hd_close,	hd_strategy,	hd_ioctl,
   hd_dump,	hd_size,	NULL},
{  hd_open,	hd_close,	hd_strategy,	hd_ioctl,
   hd_dump,	hd_size,	NULL},
{  hd_open,	hd_close,	hd_strategy,	hd_ioctl,
   hd_dump,	hd_size,	NULL},
{  hd_open,	hd_close,	hd_strategy,	hd_ioctl,
   hd_dump,	hd_size,	NULL}
};
int nblkdev = sizeof(bdevsw) / sizeof(bdevsw[0]);

/* The character device driver definitions */
/* Currently only one: memory.             */
int	mm_rw(dev_t dev, struct uio *uio, int ioflag);
#define mm_select	seltrue

#define NCHRDEV 1
struct cdevsw cdevsw[NCHRDEV] = {
{  (int (*)(dev_t, int, int, struct proc *))nullop,
   (int (*)(dev_t, int, int, struct proc *))nullop,
   mm_rw,
   mm_rw,
   (int (*)(dev_t, int, caddr_t, int, struct proc *))enodev,
   (int (*)(struct tty *, int))nullop,
   (int (*)(int))nullop,
   NULL,
   mm_select,
   enodev,
   NULL}
};
int nchrdev = sizeof(cdevsw) / sizeof(cdevsw[0]);
int mem_no = 0;	/* major device number of memory special file /dev/kmem */

/* Define the UNIX filesystem operations */
/*
 * This specifies the filesystem used to mount the root.
 * This specification should be done by /etc/config.
 */
extern int ufs_mountroot();
int (*mountroot)() = ufs_mountroot;

/*
 * These define the root filesystem and device.
 */
struct mount *rootfs;
struct vnode *rootdir;	/* The vnode of '/' */

/*
 * Set up the filesystem operations for vnodes.
 * The types are defined in mount.h.
 */
extern	struct vfsops ufs_vfsops;

#define NVFSDEV 5
struct vfsops *vfssw[NVFSDEV] = {
	(struct vfsops *)0,	/* 0 = MOUNT_NONE */
	&ufs_vfsops,		/* 1 = MOUNT_UFS */
	(struct vfsops *)0,	/* 2 = MOUNT_NFS - &nfs_vfsops */
	(struct vfsops *)0,	/* 3 = MOUNT_MFS - &mfs_vfsops */
	(struct vfsops *)0,	/* 4 = MOUNT_PC */
};


