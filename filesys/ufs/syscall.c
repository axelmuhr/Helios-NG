/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 6 August 1992                                                */
/* File: syscall.c                                                    */
/*                                                                    */
/*
 * This file contains the system call interface to UNIX from Helios.
 */
/*
 * $Id: syscall.c,v 1.3 1992/12/09 17:36:36 al Exp $ 
 * $Log: syscall.c,v $
 * Revision 1.3  1992/12/09  17:36:36  al
 * Changed procname to myprocname and back_trace to my_back_trace.
 * I must update these files later to use the proper helios calls.
 *
 * Revision 1.2  1992/10/13  11:47:03  al
 * Syntax fixed to compile for C40
 *
 * Revision 1.1  1992/09/16  09:29:06  al
 * Initial revision
 *
 */

#include <helios.h>
#include <syslib.h>
#include <stdarg.h>
#include <servlib.h>
#include <codes.h>
#include <gsp.h>
#include <module.h>
#include <device.h>
#include <limits.h>

#include "param.h"
#include "buf.h"
#include "filedesc.h"
#include "file.h"
#include "namei.h"
#include "specdev.h"
#include "user.h"
#include "proc.h"
#include "mount.h"
#include "vnode.h"
#include "quota.h"
#include "inode.h"
#include "time.h"
#include "malloc.h"
#include "socketvar.h"
#include "fcntl.h"
#include "uio.h"

#include "dispatch.h"

/*
 * HELIOS filesystem system variables.
 */

/*
 * Prototypes for UNIX system call routines.
 */
extern chmod();

/*
 * Necessary stub routines for interfacing Helios directly into the
 * kernel.
 */

/*
 * General routine to delete a file/directory off the disk.  The
 * name is passed and depending whether the target is a file or a directory
 * the corresponding routine will be called.  This is a merge of unlink and
 * rmdir.
 */
int sysdelete(p, uap, retval)
	register struct proc *p;
	register struct name_args *uap;
	int *retval;
{
	struct nameidata nd;
	struct nameidata *ndp = &nd;
	struct vnode *vp;
	int error;
	bool isdir;
	
#ifdef SHOWCALLS
	IOdebug("ufs: sysdelete called: '%s'",uap->name);
#endif
	/* Setup for the name search */
	ndp->ni_nameiop = DELETE | LOCKPARENT | LOCKLEAF;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->name;

	/* Lookup the object 1st */
	error = namei(ndp,p);
	if (!error) {	/* Object found, do appropriate deletion. */
		vp = ndp->ni_vp;
		
		if (isdir = (vp->v_type == VDIR)) {
			/* Delete directory.  AMS- Code from UNIX rmdir */
			/*
			 * No rmdir "." please.
			 */
			if (ndp->ni_dvp == vp)
				error = EINVAL;
			else if (vp->v_flag & VROOT)
				/*
				 * Don't unlink a mounted directory.
				 */
				error = EBUSY;

		} else {
			/* Delete a file.  -AMS Code from UNIX unlink. */
			if (vp->v_flag & VROOT) 
				/*
				 * Don't unlink a mounted file.
				 */
				error = EBUSY;
		}
		if (!error) {
			if (isdir) 	error = VOP_RMDIR(ndp,p);
			else 		error = VOP_REMOVE(ndp,p);
		} else {
			VOP_ABORTOP(ndp);
			if (ndp->ni_dvp == vp)
				vrele(ndp->ni_dvp);
			else
				vput(ndp->ni_dvp);
			vput(vp);
		}
	}
	return(error);
}

/*
 * Tp prevent excess reading from disk, this implements the UNIX code namei.
 * This is because this routine must know if an object is a
 * file or a directory when it calls chmod.  If a lstat were to be done, this
 * would result in superfluous information as well as unnecessary read
 * from the disk.
 *
 * Note that it does not need to call syscall as it should already be in the 
 * unix kernel.
 */
int syschmod(p, uap, retval)
	struct proc *p;
	struct matrix_args *uap;
	int *retval;
{
	struct mode_args mode_args;
	struct nameidata nd;
	struct nameidata *ndp = &nd;
	int error;

	ndp->ni_nameiop = LOOKUP | LOCKLEAF | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->name;
	error = namei(ndp,p);
	if (!error) {
		vput(ndp->ni_vp);	/* Unlock */

		mode_args.name = uap->name;
		mode_args.mode = matrix2mode(ndp->ni_vp->v_type,uap->matrix);
#ifdef DEBUG
		IOdebug("chmod actual = 0x%x   want = 0x%x   matrix = 0x%x",
		VTOI(ndp->ni_vp)->i_mode,
		matrix2mode(ndp->ni_vp->v_type,uap->matrix),
		uap->matrix);
#endif
		error = chmod(p,&mode_args,retval);
	}
	return(error);	
}
 
/*
 * For similar reasons as above (prevent excess reads) this system call
 * looks up the name passed in the name cache, or if not there pulls
 * the inode from disk, by using the namei call.  The vnode type is 
 * returned.
 */
int sysvntype(p, uap, retval)
	register struct proc *p;
	register struct vntype_args *uap;
	int *retval;
{
	struct nameidata nd;
	struct nameidata *ndp = &nd;
	int error;
	
	ndp->ni_nameiop = LOOKUP | LOCKLEAF | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->name;
	if ((error = namei(ndp,p)) == 0) {
		*(uap->vtype) = ndp->ni_vp->v_type;
		vput(ndp->ni_vp);
	}
	return(error);
}

/* 
 * System Call from HELIOS to UNIX.
 */
int syscall(	ClientState *cs, 
		int (*fn)(), 
		struct syscall_args *uap,
		int *retval )
{
	int error;

#ifdef DEBUG
IOdebug("syscall %s made ",(char *)myprocname(*fn));
#endif

	if (cs->SysCall != NULL) {
		IOdebug("SysCall %s: Call to %s already in progress",
			 (char *)myprocname(fn),myprocname(cs->SysCall));
		return(EALREADY);
	}

	/* Set the call and get kernel */
	cs->SysCall = (VoidFnPtr)fn;
	tokernel();

	/* Setup the process state */
	curproc = &cs->p;

	/* Make the call */
#ifdef DEBUG
IOdebug("syscall: about to do its call");
#endif
	error = (*fn)(&(cs->p),uap,retval);

	/* Reset state */
 	cs->SysCall = NULL; 

	/* Free the kernel */
	fromkernel();
#ifdef DEBUG
IOdebug("syscall %s done: error %d (0x%x)",(char *)myprocname(fn),error,error);
#endif
	/* Back To Originator */
	return(error);
}

