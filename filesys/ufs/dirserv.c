/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 7 August 1992                                                */
/* File: dirserv.c                                                    */
/*                                                                    */
/*
 * This file contains the directory server and its service routines.
 * Other service routines for open files may be found in fileserv.c 
 * with the main service routines in services.c
 */
/*
 * $Id: dirserv.c,v 1.2 1992/10/13 11:47:03 al Exp $ 
 * $Log: dirserv.c,v $
 * Revision 1.2  1992/10/13  11:47:03  al
 * Syntax fixed to compile for C40
 *
 * Revision 1.1  1992/09/16  09:29:06  al
 * Initial revision
 *
 */

#include "param.h"
#include "malloc.h"
#include "buf.h"
#include "filedesc.h"
#include "file.h"
#include "user.h"
#include "stat.h"
#include "proc.h"
#include "mount.h"
#include "vnode.h"
#include "fcntl.h"
#include "dir.h"

#include <helios.h>
#include <syslib.h>
#include <stdarg.h>
#include <servlib.h>
#include <codes.h>
#include <gsp.h>
#include <module.h>
#include <device.h>
#include <dirent.h>

#include "dispatch.h"

/*
 * Internal structure to the directory server.
 */
struct mydirlist {
	DirEntry direntry;
	struct mydirlist *next;
};

/*
 * Prototypes for HELIOS->UNIX system call routines (direct into kernel).
 */

/*
 * Prototypes for UNIX system call routines.
 */
extern getdirentries(), lstat(), close();

/*
 * Read in the directory entries into an internal format which
 * may be sent to Helios.
 * XXX AMS Later use sysvntype to get the system type rather than lstat.
 */
static void load_dir_entries(struct ufsrw *ar, struct mydirlist **MyDir, 
		      word *nentries, char *dirname)
{
	struct getdirent_args dir_ent;
	struct fd_args fd_args;
	struct stat_args stat_args;	
	struct stat resstat;

	int offset, error, retval;
	struct direct *dir;
	struct mydirlist *md, *lastadd;
	long base = 0;
	char fullname[MAXPATHLEN];
		
#ifdef SHOWCALLS
printf("load_dir_entries called with '%s'\n",dirname);
#endif

	*nentries = 0;

	/* Dummy 1st Entry */
	lastadd = (struct mydirlist *)malloc(sizeof(struct mydirlist),M_TEMP,M_WAITOK);
	if (lastadd == NULL) panic("ufs: No memory left to read directory.");
	else {
		*MyDir = lastadd;
		lastadd->next = NULL;
	}

	/* Read in the entire directory, creating a list of entries */
	do {	/* Get MAXBSIZE blocks of data */
		dir_ent.fd = ar->handle;
		dir_ent.buf = &ar->buf[0];
		dir_ent.count = MAXBSIZE;
		dir_ent.basep = &base;

		/* Get the block */
		error = syscall(ar->client,getdirentries,
				(struct syscall_args *)&dir_ent,&retval);
		if (error) break;
#ifdef DEBUG
printf("load_dir_entries: read %d bytes of a block\n",retval);
#endif
						
		/* Extract the entries.  1st read the directory, getting the
		 * names, then close the directory and get info from each entry.
		 * This prevents a deadlock from happening.
		 */
		for (offset = 0; offset < retval; offset += dir->d_reclen) {
#ifdef DEBUG
printf("load_dir_entries: extract entry, offset=%d  retval=%d\n",offset,retval);
#endif

 			/* Setup Pointer To Directory */
			dir = (struct direct *)&ar->buf[offset];
#ifdef __C40
#undef d_ino
#endif
			if (dir->d_ino == 0) continue;
			
			/* Add the entry to the list */
			md = (struct mydirlist *)malloc(sizeof(struct mydirlist),M_TEMP,M_WAITOK);
			if (md == NULL) panic("ufs: No memory left to read directory.");
			else {
				(*nentries)++;
				md->direntry.Type = Type_File;	/* Defaults */
				md->direntry.Flags = 0;
				md->direntry.Matrix = DefFileMatrix;
				if (dir->d_namlen > 31) /* Truncate */
					dir->d_name[31] = '\0';
				strcpy(md->direntry.Name,dir->d_name);
				lastadd->next = md;
				md->next = NULL;
				lastadd = md;
			}
		}
	} while (retval);
#ifdef DEBUG
printf("load_dir_entries: done loop\n");
#endif
	/* Close the directory */
	fd_args.fd = ar->handle;
	error = syscall(ar->client,close,(struct syscall_args *)&fd_args,&retval);
	
	/* Free the dummy 1st entry */
	lastadd = *MyDir;
	*MyDir = lastadd->next;
	free(lastadd,M_TEMP);

	/* Get info from each Entry */
	for (lastadd = *MyDir; lastadd; lastadd = lastadd->next) {
		/* Get the attributes by looking them up */
		strcpy(fullname,dirname);
		pathcat(fullname,lastadd->direntry.Name);

		/* Do the stat on the file */
		stat_args.fname = fullname;
		stat_args.ub = &resstat;
		error = syscall(ar->client,lstat,
			(struct syscall_args *)&stat_args,&retval);
		if (error) {
			continue;
		}

		/* Extract the stat data and place into mydirentry */
		switch (resstat.st_mode & S_IFMT) {
		case S_IFREG:
		case S_IFBLK:
		case S_IFCHR:
			lastadd->direntry.Type = Type_File;
			break;
		case S_IFDIR:
			lastadd->direntry.Type = Type_Directory;
			break;
		case S_IFLNK:
			lastadd->direntry.Type = Type_Link;
			break;
		case S_IFIFO:
			lastadd->direntry.Type = Type_Fifo;
			break;
		case S_IFSOCK:
			lastadd->direntry.Type = Type_Socket;
			break;
		default:
			lastadd->direntry.Type = 0;
			break;
		}
		lastadd->direntry.Matrix = 
			mode2matrix(lastadd->direntry.Type,resstat.st_mode);
		lastadd->direntry.Flags = 0;	/* XXX */
	}
#ifdef SHOWCALLS
printf("load_dir_entries done, found %d entries\n",*nentries);
#endif
}

void dir_server(MyMCB *mymcb, ClientState *client, word handle, char *name)
{
  MCB *mcb = &mymcb->mcb;
  Port reqport = mcb->MsgHdr.Reply;
  struct ufsrw *rw = (struct ufsrw *)malloc(sizeof(struct ufsrw),M_TEMP,M_WAITOK);
  word oldtimeout = mcb->Timeout;
  struct mydirlist *mydir;
  struct mydirlist *md;
  word nentries, e;
  word totaltimeout = 0;

  if (rw == NULL) {
	ErrorMsg(mcb,EC_Error|EG_NoMemory|EO_File);
	printf("dir_server panic; No memory left to service requests.\n");
	return;
  }
	
  /* Setup Credentials */
  rw->client = client;
  rw->handle = handle;
  rw->fp = client->p.p_fd->fd_ofiles[handle];	/* Quick Reference */
  mydir = NULL;

  load_dir_entries(rw,&mydir,&nentries,name);
	
  /* Process Messages */
  while (!client->force_close) {
	InitMCB(mcb,0,reqport,NullPort,0);
	mcb->Timeout = IdleTimeout;	/* Idle, check for force close */
	e = GetMsg(mcb);
	mcb->Timeout = oldtimeout;	/* Restore */
	if (e < 0) {
	    if ((e & EG_Mask) == EG_Timeout) {
		totaltimeout += IdleTimeout;
		if (totaltimeout >= StreamTimeout) {
		    printf("Timeout close of directory %s (%d) for user %d\n",
				client->ofname,client->p.p_pid,
				client->p.p_cred->p_ruid);
		    goto done;
		}
	    }
#ifdef DEBUG
printf("dir_server: serving open directory %s for 0x%x\n",client->ofname,client);
#endif
	    continue;
	}
		
	mcb->MsgHdr.FnRc = SS_HardDisk;
	totaltimeout = 0;	/* Reset since valid message */
	switch (e & FG_Mask) {
	case FG_Read:	{
		ReadWrite *rw = (ReadWrite *)mcb->Control;
		word pos = rw->Pos;
		word size = rw->Size;
		word dpos = 0;
		word tfr = 0;
		word seq = 0;
		word dirsize = nentries * sizeof(DirEntry);
		Port reply = mcb->MsgHdr.Reply;
			
		/* Read at end ? */
		if (pos == dirsize) {
			InitMCB(mcb,0,reply,NullPort,ReadRc_EOF);
			e = PutMsg(mcb);
			break;
		}
			
		/* Read Invalid ? */
		if ((pos < 0) || (pos > dirsize) || 
		    (pos % sizeof(DirEntry) != 0)) {
		    	ErrorMsg(mcb,EC_Error+EG_Parameter+1);
		    	break;
		}
			
		/* Ensure not too much */
		if (pos + size > dirsize) size = dirsize - pos;

		InitMCB(mcb,MsgHdr_Flags_preserve,reply,NullPort,ReadRc_More);
		md = mydir;
		while (size >= sizeof(DirEntry)) {
			/* Skip to location */
			if (dpos < pos) {
				dpos += sizeof(DirEntry);
				md = md->next;
				continue;
			}
				
			/* Prepare Reply */ 
			MarshalData(mcb,sizeof(DirEntry),
					(byte *)&md->direntry);
				
			/* Adjust Counters */
			md = md->next;
			dpos += sizeof(DirEntry);
			tfr += sizeof(DirEntry);
			pos += sizeof(DirEntry);
			size -= sizeof(DirEntry);
				
			/* Check if can send yet ? */
			if ((size < sizeof(DirEntry)) ||
			    (IOCDataMax - tfr < sizeof(DirEntry))) {
			    	if (size < sizeof(DirEntry)) {
			    	   /* EOF/EOD */
			    	   mcb->MsgHdr.Flags = 0;
			    	   if (dpos == dirsize) 
		    		  	mcb->MsgHdr.FnRc = ReadRc_EOF|seq;
		    		   else	mcb->MsgHdr.FnRc = ReadRc_EOD|seq;
			    	}
				    	
			    	/* Send the damned thing. */
			    	e = PutMsg(mcb);
			    	seq += ReadRc_SeqInc;
			    	InitMCB(mcb,MsgHdr_Flags_preserve,
			    		reply,NullPort,ReadRc_More|seq);
			    	tfr = 0;
			}
		}

		/* Ensure last bit is sent */
		if (tfr > 0) {
	    		mcb->MsgHdr.Flags = 0;
	    		if (dpos == dirsize) 
	    			mcb->MsgHdr.FnRc = ReadRc_EOF|seq;
	    		else	mcb->MsgHdr.FnRc = ReadRc_EOD|seq;

		    	e = PutMsg(mcb);
		}
		break;
	}
	case FG_GetSize: {
		InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,0);
		MarshalWord(mcb,nentries*sizeof(DirEntry));
		e = PutMsg(mcb);
		break;
	}
	case FG_Close:		goto done;	break;
	default:
		ErrorMsg(mcb,EC_Error|EG_WrongFn|EO_Directory);
		e = Err_Null;
		break;
	}

	/* e should have been set to the last error in the switch */
	if (e < Err_Null) {
		printf("Directory '%s':%d Read; client %d disappeared.  Fault 0x%x\n",
			client->ofname, client->p.p_pid,
			client->p.p_ucred->cr_uid, e);
	}
  }	

  /* Fell through, so must have forced a closure */
  printf("Forcing close of directory %s (%d) for user %d\n",
		client->ofname,client->p.p_pid,
		client->p.p_cred->p_ruid);

done:
  /* Free Buffers and return */
  while (mydir != NULL) {
	md = mydir;
	mydir = md->next;
	free(md,M_TEMP);
}
  free(rw,M_TEMP);
  return;
}
