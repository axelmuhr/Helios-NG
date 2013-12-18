/****************************************************************/
/*                          Ariel Corp.                         */
/*                        433 River Road                        */
/*                Highland Park, NJ 08904, U.S.A.               */
/*                     Tel:  (908) 249-2900                     */
/*                     Fax:  (908) 249-2123                     */
/*                     BBS:  (908) 249-2124                     */
/*                  E-Mail:  ariel@ariel.com                    */
/*                                                              */
/*                 Copyright (C) 1993 Ariel Corp.               */
/****************************************************************/


/* $Id: c40sundriv.c,v 1.2 1994/06/29 13:46:19 tony Exp $ */
/* c40sundriv.c
 *
 * This file defines the base functions needed in the utility library for
 * use with the SunOS device driver, VC40DSP.
 */
#include <stdio.h>
#include "vc40all.h"
#include "vc40dsp.h"
#include "internal.h"

#if defined( VXWORKS )
#  include "vxWorks.h"
#  include "iosLib.h"	/* for DEV_HDR struct and iosDevFind proto */
#  define BASE_DEV_NAME "/vc40"
#else
#  define BASE_DEV_NAME "/dev/vc40"
#endif

static int raw_open();

/***************************************************************************
 * map an index returned from c40_open() to a file descriptor.  For now,
 * c40_open() just returns the file descriptor, but this will not always be
 * the case.
 */
int c40_id2fd(c40)
int c40;
{
    return (c40);
}

/***************************************************************************
 * c40_open()
 */
int c40_open(devd, mode)
char *devd;
int mode;
{
    char devname[20];
    int brd, ndsp, fd, serno;
    struct vc40info vc40info;

    /*
     * if null string, open the next DSP that's not already busy
     */
    if (devd == NULL || *devd == '\0') {
        brd = 'a';
        ndsp = 1;
        while (1) {
            sprintf(devname, "%s%c%d", BASE_DEV_NAME, brd, ndsp);
	    strcpy (VC40_Device, devname);
#if defined( VXWORKS )
            fd = raw_open(devname, O_RDWR | mode, 0777);
            if ((fd == -1) && (errno == ENOENT)) {
                /*
                 * can not access device special file, assume that's because
                 * there are no more Hydras to try, in which case there's
                 * nothing left to do.
                 */
                return (-1);
            }
            else {
                /*
                 * if not attached, just return this file descriptor
                 */
                if (c40_attach(fd, VC40_DETACHED, NULL) == 0) {
                    return (fd);
                }
                /*
                 * the DSP was attached, close it and try next one
                 */
                close(fd);
            }
            /*
             * device existed but wouldn't open or was attached,
             * so try next device
             */
            ndsp++;
            if (ndsp == 5) {
                ndsp = 1;
                brd++;
            }
#else
            if (access(devname, F_OK) == 0) {  /* device file there, try it */
                fd = raw_open(devname, O_RDWR | mode, 0777);
                if (fd > 0) {   /* open was successful, see it its attached */
                    /*
                     * if not attached, just return this file descriptor
                     */
                    if (c40_attach(fd, VC40_DETACHED, NULL) == 0) {
                        return (fd);
                    }
                    /*
                     * the DSP was attached, close it and try next one
                     */
                    close(fd);
                }
                /*
                 * open was not successful or DSP was attached, try next DSP
                 */
                ndsp++;
                if (ndsp == 5) {
                    ndsp = 1;
                    brd++;
                }
            }
            else {
                /*
                 * can not access device special file, assume that's because
                 * there are no more Hydras to try, in which case there's
                 * nothing left to do.
                 */
                return (-1);
            }
#endif /* !VXWORKS */
        } /* while (1) */
    }
    
    /*
     * try name as given
     */
    if ( (fd = raw_open(devd, O_RDWR | mode, 0777)) != -1) {
        return (fd);
    }

    /*
     * nope, try pre-pending base device name
     */
    strcpy(devname, BASE_DEV_NAME);
    strcat(devname, devd);

    strcpy (VC40_Device, devname);
    
    return (raw_open(devname, O_RDWR | mode, 0777));
}

/***************************************************************************
 * c40_close() - for now, this is identical to close(), but it won't always
 * be so, so use c40_close() in user programs rather than close().
 */
int c40_close(c40_fd)
int c40_fd;
{
    return (close(c40_fd));
}

/***************************************************************************
 * c40_write_long()
 * writes a block of long words (32-bit) to the DSP at the specified DSP
 * address. Uses HydraMon's Copy function.
 */
int c40_write_long(c40_fd, dsp_addr, buf, wcnt)
int c40_fd;		/* file descriptor */
u_long dsp_addr;	/* address on DSP to write buffer to */
void *buf;		/* buffer of longs to write to DSP */
u_long	wcnt;		/* number of longs to write to DSP */
{
    int	rcode;
    
    rcode = ioctl(c40_fd, VC40SETADDR, &dsp_addr);  /* set the DSP address */
    if (rcode == -1) {
        return (-1);
    }
    wcnt *= sizeof(u_long);
    rcode = write(c40_fd, buf, wcnt);    /* write the block */
    if (rcode != wcnt) {
        return (-1);
    }
    return (0);
}

/***************************************************************************
 * c40_read_long()
 * reads a block of long words (32-bit) from the DSP at the specified DSP
 * address. Uses HydraMon's Copy function.
 */
int c40_read_long(c40_fd, dsp_addr, buf, wcnt)
int c40_fd;		/* file descriptor */
u_long dsp_addr;	/* address on DSP to read buffer from */
void *buf;		/* buffer of longs to read from DSP */
u_long	wcnt;		/* number of longs to read from DSP */
{
    int	rcode;
    
    rcode = ioctl(c40_fd, VC40SETADDR, &dsp_addr);  /* set the DSP address */
    if (rcode == -1) {
        return (-1);
    }
    wcnt *= sizeof(u_long);
    rcode = read(c40_fd, buf, wcnt); /* read the block */
    if (rcode != wcnt) {
        return (-1);
    }
    return (0);
}

/***************************************************************************
 * c40_map_shmem()
 *
 * maps in Hydra's DRAM to the user's virtual address space
 */
char *c40_map_shmem(c40_fd, offs, len)
int c40_fd;	/* unit's file descriptor */
u_long offs;	/* offset from the beginning of the unit's DRAM to start */
u_long len;	/* number of *bytes* to map */
{
#if defined( VXWORKS )
    struct vc40mmap mm;

    mm.offset = offs;

    if (ioctl(c40_fd, VC40MMAP, &mm) == OK) {
	return mm.addr;
    }
    else {
	return (NULL);
    }
#else
    char *vpstart;
    u_long psize = getpagesize();	/* get system's page size */
    u_long pstart, poff;
    
    /*
     * calculate beginning of nearest page, and offset from beginning
     * of page.
     */
    pstart = (offs / psize) * psize;
    poff = offs % psize;
    
    /*
     * map the DRAM in
     */
    vpstart = (char *) mmap((caddr_t)0, len+poff, PROT_READ|PROT_WRITE,
                            MAP_SHARED, c40_fd, pstart);
    if (vpstart == (char *)(-1)) {
        return ((char *)-1);
    }
    
    /*
     * return virtual address
     */
    return (vpstart + poff);
#endif /* !VXWORKS */
}

/****************************************************************************
 * c40_map_jmcr()
 *
 * maps in Hydra's JTAG controller and MCR to the user's virtual address space
 */
struct vc40jmcr *c40_map_jmcr(c40_fd)
int c40_fd;	/* unit's file descriptor */
{
#if defined( VXWORKS )
    struct vc40mmap mm;

    mm.offset = VC40MMAPJMCR;

    if (ioctl(c40_fd, VC40MMAP, &mm) == OK) {
	return (struct vc40jmcr *)mm.addr;
    }
    else {
	return ((struct vc40jmcr *) -1);
    }

#else
    struct vc40jmcr *vpstart;
    
    /*
     * map the JTAG/MCR in
     */
    vpstart = (struct vc40jmcr *) mmap((caddr_t)0, sizeof(struct vc40jmcr), 
                  PROT_READ|PROT_WRITE, MAP_SHARED, c40_fd, VC40MMAPJMCR);
    /*
     * return virtual address
     */
    return (vpstart);
#endif /* !VXWORKS */
}

/****************************************************************************
 * c40_map_jtag()
 *
 * maps in Hydra's JTAG controller
 * RETURNS NULL on error, else returns pointer to JTAG controller
 */
u_long *c40_map_jtag(c40id)
int c40id;      /* unit's file descriptor */
{
    struct vc40info vc40info;

#if defined( VXWORKS )
    struct vc40mmap mm;
#else
    u_long *vpstart, psize, offs, paddr;
#endif

    /*
     * find out what kind of board we're talking to
     */
    if (c40_getinfo(c40id, &vc40info) != 0) {
        return (NULL);
    }

#if defined( VXWORKS )
    switch (vc40info.board_type) {
      case HYDRAI:
      case HYDRAIVSB:
        mm.offset = VC40MMAPJMCR;

        if (ioctl(c40id, VC40MMAP, &mm) == OK) {
            return (((u_long *)mm.addr)+32);
        }
        else {
            return (NULL);
        }
        break;

      case HYDRAII:
	/* 
	 * Hydra-II VxWorks driver accepts the jtag offset as an input 
	 * (mm.offset) and returns the VME address in mm.addr
	 */
	mm.offset = H2_MMAP_O_JTAG;
	if (ioctl (c40id, VC40MMAP, &mm) == OK)
	    return ((u_long *) mm.addr);
	else 
	    return (NULL);
	break;

      default:
        fprintf(stderr, "VxWorks: unknown board type in c40_map_jtag()\n");
        errno = EINVAL;
        return (NULL);
    }

#else

    /*
     * map the JTAG in and compute virtual address
     */
    switch (vc40info.board_type) {
      case HYDRAI:
      case HYDRAIVSB:
        vpstart = (u_long *) mmap((caddr_t)0, sizeof(struct vc40jmcr), 
                                  PROT_READ | PROT_WRITE, MAP_SHARED, c40id,
                                  VC40MMAPJMCR);
        vpstart += 32;  /* JTAG controller is 32 words from base */
        break;

      case HYDRAII:
        paddr = HYDRAII_JTAG;
        psize = getpagesize();
        offs = paddr % psize;
        paddr = paddr - offs;
        vpstart = (u_long *) mmap((caddr_t)0, 256, PROT_READ | PROT_WRITE,
                                  MAP_SHARED, c40id, paddr);
        vpstart += (offs/4);
        break;

      case SBUS_AXDS:
        vpstart = (u_long *) mmap((caddr_t)0, 4096, PROT_READ | PROT_WRITE,
                                  MAP_SHARED, c40id, 0);
        vpstart += 0x200;   /* point to JTAG registers */
        break;
                                  

      default:
        errno = EINVAL;
        return (NULL);
    } /* switch (board_type) */

    return (vpstart);
#endif  /* !VXWORKS */
}

/****************************************************************************
 * c40_map_ipcr()
 *
 * maps in Hydra's IPCRs to the user's virtual address space
 */
struct vic_ipcr *c40_map_ipcr(c40_fd)
int c40_fd;	/* unit's file descriptor */
{
#if defined( VXWORKS)
    struct vc40mmap mm;

    mm.offset = VC40MMAPVIC;

    if (ioctl(c40_fd, VC40MMAP, &mm) == OK) {
	return (struct vic_ipcr *)mm.addr;
    }
    else {
	return ((struct vic_ipcr *) -1);
    }
#else
    struct vic_ipcr *vpstart;
    
    /*
     * map the VIC
     */
    vpstart = (struct vic_ipcr *) mmap((caddr_t)0, 64, 
                  PROT_READ|PROT_WRITE, MAP_SHARED, c40_fd, VC40MMAPVIC);
    /*
     * return virtual address
     */
    return (vpstart);
#endif /* !VXWORKS */
}

/***************************************************************************
 * Tells DSP to start running from the given location
 */
int c40_run(c40, dspaddr)
int c40;        /* file descriptor */
u_long dspaddr; /* address to run from */
{
    return (ioctl(c40, VC40RUN, &dspaddr));
}

/***************************************************************************
 * Fires an IOF2 interrupt on the DSP.  On DSP 1 this assumes that VIC is
 * programmed to generate IOF2 interrupts! WARNING: this also assumes that
 * Hydramon has been removed from the DSP, as IOF2 is the Hydramon interrupt!
 */
int c40_iof2(c40)
int c40;        /* file descriptor */
{
    return (ioctl(c40, VC40IOF2, NULL));
}

/***************************************************************************
 * reset a DSP
 */
int c40_reset(c40)
int c40;    /* file descriptor */
{
    return (ioctl(c40, VC40RESET, NULL));
}

/***************************************************************************
 * Wipe DSP 1 of a V-C40 Hydra.  All other DSPs/boards just do a reset.
 */
int c40_wipe(c40)
int c40;    /* file descriptor */
{
    return (ioctl(c40, VC40WIPE, NULL));
}

/***************************************************************************
 * halt a DSP
 */
int c40_halt(c40)
int c40;    /* file descriptor */
{
    return (ioctl(c40, VC40HALT, NULL));
}

/***************************************************************************
 * returns info. about a DSP / Hydra
 */
int c40_getinfo(c40, vc40info)
int c40;    /* file descriptor */
struct vc40info *vc40info;
{
    return (ioctl(c40, VC40GETINFO, vc40info));
}

/***************************************************************************
 * enables interrupts from a DSP
 */
int c40_enint(c40, signum)
int c40;    /* file descriptor */
int signum; /* signal to deliver to process */
{
    return (ioctl(c40, VC40ENINT, &signum));
}

/***************************************************************************
 * disables interrupts from a DSP
 */
int c40_dsint(c40)
int c40;    /* file descriptor */
{
    return (ioctl(c40, VC40DSINT, NULL));
}

/***************************************************************************
 * issues a trap to a DSP
 */
int c40_trap(c40, trapnum)
int c40;    /* file descriptor */
u_long trapnum;
{
    return (ioctl(c40, VC40TRAP, &trapnum));
}

/***************************************************************************
 * disables HydraMon's keboard interrupt
 */
int c40_dskey(c40)
int c40;    /* file descriptor */
{
    return (ioctl(c40, VC40DISKEY, NULL));
}

/***************************************************************************
 * enables HydraMon's keboard interrupt
 */
int c40_enkey(c40)
int c40;    /* file descriptor */
{
    return (ioctl(c40, VC40ENKEY, NULL));
}

/***************************************************************************
 * sets the attach code
 */
int c40_attach(c40, acode, ocode)
int c40;    /* file descriptor */
u_long acode;
u_long *ocode;
{
    int rval;
    u_long  attach = acode;
    
    rval = ioctl(c40, VC40ATTACH, &attach);
    if (ocode) {
        *ocode = attach;    /* return old attach code */
    }
    return (rval);
}

/***************************************************************************
 * detaches from a DSP
 */
int c40_detach(c40, acode)
int c40;    /* file descriptor */
u_long acode;   /* attach code - must match current attach ID to detach */
{
    return (ioctl(c40, VC40DETACH, &acode));
}

/***************************************************************************
 * gets a property from the EEPROM
 */
int c40_get_property(c40, propid, propval)
int c40;    /* file descriptor */
u_long propid;
u_long *propval;
{
    int rcode;
    struct vc40property c40prop;

    c40prop.propid = propid;
    rcode = ioctl(c40, VC40GETPROP, &c40prop);
    *propval = c40prop.propval;
    return (rcode);
}

/***************************************************************************
 * sleeps until an interrupt occurs
 * The VC40WAIT ioctl() does not return any errors
 */
int c40_wait(c40, timeout)
int c40;
int timeout;
{
    if (ioctl(c40, VC40WAIT, &timeout) != 0) {
        return (-1);
    }
    return (0);
}

/***************************************************************************
 * returns the number of interrupts since the last call.
 * The VC40POLL ioctl() does not return any errors.
 */
int c40_poll(c40, icnt)
u_long *icnt;
{
    return (ioctl(c40, VC40POLL, icnt));
}

/***************************************************************************
 * reads the MCR
 */
int c40_read_mcr(c40, mcrvalp)
u_long *mcrvalp;
{
    return (ioctl(c40, VC40RMCR, mcrvalp));
}

/***************************************************************************
 * writes the MCR
 */
int c40_write_mcr(c40, mcrval)
u_long mcrval;
{
    return (ioctl(c40, VC40WMCR, &mcrval));
}

/***************************************************************************
 * reads an IPCR
 */
int c40_read_ipcr(c40, ipcrn, ipcrvp)
int c40;
int ipcrn;
u_char *ipcrvp;
{
    struct ipcr ipcr;

    ipcr.ipcr_num = ipcrn;
    if (ioctl(c40, VC40RIPCR, &ipcr) != 0) {
        return (-1);
    }
    *ipcrvp = ipcr.ipcr_data;
    return (0);
}

/***************************************************************************
 * writes an IPCR
 */
int c40_write_ipcr(c40, ipcrn, ipcrv)
int c40;
int ipcrn;
u_char ipcrv;
{
    struct ipcr ipcr;

    ipcr.ipcr_num = ipcrn;
    ipcr.ipcr_data = ipcrv;
    return (ioctl(c40, VC40WIPCR, &ipcr));
}

/***************************************************************************
 * reads shared memory - for compatibility with DOS functions.
 * WARNING: This function may destroy DRAM mappings already in effect by
 * the caller!
 * Returns 0 on success, -1 on mapping failure.
 */
int c40_read_shmem(c40, woffs, bufp, nwords)
int c40;
u_long woffs;
u_long *bufp;
u_long nwords;
{
    u_long *dramp;
    
    dramp = (u_long *) c40_map_shmem(c40, 4*woffs, 4*nwords);
    if (dramp == NULL || dramp == (u_long *) -1) {
        return (-1); /* errno should be set by c40_map_shmem() */
    }
    while ( (nwords--) > 0) {
        *bufp++ = *dramp++;
    }
#if !defined( VXWORKS )
    munmap(dramp, 4*nwords);
#endif
    return (0);
}

/***************************************************************************
 * writes shared memory - for compatibility with DOS functions.
 * WARNING: This function may destroy DRAM mappings already in effect by
 * the caller!
 * Returns 0 on success, -1 on mapping failure.
 */
int c40_write_shmem(c40, woffs, bufp, nwords)
int c40;
u_long woffs;
u_long *bufp;
u_long nwords;
{
    u_long *dramp;
    
    dramp = (u_long *) c40_map_shmem(c40, 4*woffs, 4*nwords);
    if (dramp == NULL || dramp == (u_long *) -1) {
        return (-1); /* errno should be set by c40_map_shmem() */
    }
    while ( (nwords--) > 0) {
        *dramp++ = *bufp++;
    }
#if !defined( VXWORKS )
    munmap(dramp, 4*nwords);
#endif
    return (0);
}

/***************************************************************************
 * read a JTAG register.  Yes, this is a stupid way of doing it, but for
 * now it will have to do.  In the next major revision, the JTAG/MCR will
 * remain mapped in all the time.
 */
int c40_read_jtag(c40id, jtagn, jtagvp)
int c40id;
int jtagn;
u_long *jtagvp;
{
#if defined( VXWORKS )
    struct vc40jmcr *jmcr = c40_map_jmcr(c40id);

    if (jmcr == (struct vc40jmcr *) -1) {
        return (-1);
    }
    *jtagvp = jmcr->jtag[jtagn];

    return 0;
#else
    struct vc40jmcr *jmcr;

    jmcr = (struct vc40jmcr *) mmap((caddr_t)0, sizeof(struct vc40jmcr), 
                  PROT_READ|PROT_WRITE, MAP_SHARED, c40id, VC40MMAPJMCR);
    if (jmcr == NULL) {
        return (-1);
    }
    *jtagvp = jmcr->jtag[jtagn];

    munmap(jmcr, sizeof(struct vc40jmcr));

    return 0;
#endif
}

/***************************************************************************
 * write a JTAG register.  Yes, this is a stupid way of doing it, but for
 * now it will have to do.  In the next major revision, the JTAG/MCR will
 * remain mapped in all the time.
 */
int c40_write_jtag(c40id, jtagn, jtagv)
int c40id;
int jtagn;
u_long jtagv;
{
#if defined( VXWORKS )
    struct vc40jmcr *jmcr = c40_map_jmcr(c40id);

    if (jmcr == (struct vc40jmcr *) -1) {
        return (-1);
    }
    jmcr->jtag[jtagn] = jtagv;

    return 0;
#else
    struct vc40jmcr *jmcr;

    jmcr = (struct vc40jmcr *) mmap((caddr_t)0, sizeof(struct vc40jmcr), 
                  PROT_READ|PROT_WRITE, MAP_SHARED, c40id, VC40MMAPJMCR);
    if (jmcr == NULL) {
        return (-1);
    }
    jmcr->jtag[jtagn] = jtagv;

    munmap(jmcr, sizeof(struct vc40jmcr));

    return 0;
#endif
}

/***************************************************************************
 * read the flash memory of Hydra-II
 */

/* not tested -- see c40map.c for working versions... */

int c40_read_flash(c40id, foffs, bbuf, wcnt, i_flash_width)
int c40id;
u_long foffs;
Void_p bbuf;
u_long wcnt;
int i_flash_width;
{
    int rcode;
    u_long flash_width;
    struct h2flash h2flash;
    u_long *bufp = (u_long *) bbuf;

    /*
     * find out the flash memory width (either 16 bits or 32 bits) so
     * that offsets can be computed properly
     */
    h2flash.foffs = 0;  /* offset of width parameter */
    h2flash.wcnt = 1;
    rcode = ioctl(c40id, VC40READFLASH, &h2flash);
    if (rcode != 0) {
        return (rcode);
    }
    printf("flash width is %d\n", h2flash.data[0]);

    return (0);
}
/***************************************************************************
 * write the flash memory of Hydra-II
 */

/* not tested -- see c40map.c for working versions... */

int c40_write_flash(c40id, foffs, bbuf, wcnt, i_flash_width)
int c40id;
u_long foffs;
Void_p bbuf;
u_long wcnt;
int i_flash_width;
{
    int rcode;
    u_long flash_width, lcnt, i;
    struct h2flash h2flash;
    u_long *bufp = (u_long *) bbuf;

    /*
     * find out the flash memory width (either 16 bits or 32 bits) so
     * that offsets can be computed properly
     */
    h2flash.foffs = 0;  /* offset of width parameter */
    h2flash.wcnt = 1;
    rcode = ioctl(c40id, VC40READFLASH, &h2flash);
    if (rcode != 0) {
        return (rcode);
    }
    printf("flash width is %d\n", h2flash.data[0]);

    h2flash.foffs = foffs;
    while (wcnt > 0) {
        lcnt = (wcnt > MAXFLASHDATA) ? MAXFLASHDATA : wcnt;
        for (i=0; i<lcnt; i++) {
            h2flash.data[i] = *bufp++;
        }
        wcnt -= lcnt;
        rcode = ioctl(c40id, VC40WRITEFLASH, &h2flash);
        if (rcode != 0) {
            return (rcode);
        }
    }

    return (0);
}

/***************************************************************************
 * Request a semaphore from the DPR (Hydra-II only).  If semaphore is not
 * available, request is left pending until it becomes available.  Use
 * RELSEM to release a semaphore or a pending request.
 */
c40_getsem(c40id, semno)
int c40id;
int semno;
{
    return (ioctl(c40id, VC40GETSEM, &semno));
}

/***************************************************************************
 * Release a semaphore.
 */
c40_relsem(c40id, semno)
int c40id;
int semno;
{
    return (ioctl(c40id, VC40RELSEM, &semno));
}


#if !defined( VXWORKS )
/***************************************************************************
 * maps in and returns a pointer to Hydra-II's control register area
 */
struct h2ctrl *c40_map_h2ctrl(c40id)
int c40id;
{
    char *vpstart;
    int paddr, psize, offs;
    struct vc40info vc40info;
    struct h2ctrl *h2ctrl;

    if (c40_getinfo(c40id, &vc40info) != 0) {
        return (NULL);
    }
    if (vc40info.board_type != HYDRAII) {
        errno = EINVAL;
    }

    paddr = HYDRAII_CTRL;
    psize = getpagesize();
    offs = paddr % psize;
    paddr = paddr - offs;
    vpstart = (char *) mmap((caddr_t)0, sizeof(struct h2ctrl),
                            PROT_READ | PROT_WRITE, MAP_SHARED, c40id, paddr);
    vpstart += offs;
    return ( (struct h2ctrl *) vpstart);
}
#endif

/***************************************************************************
 * raw_open() -- open() wrapper, different for vxworks because of kernel
 *               braindamage.
 */

static int raw_open (name, o_flags, prot_mask)
char *name;
int o_flags;
int prot_mask;
{

#if !defined( VXWORKS )
    /* just a wrapper...*/
    return (open (name, o_flags, prot_mask));
#else
    /* Under VxWorks, an open of "/vc40a16" will open /vc40a1, if /vc40a16 
     * doesn't exist. We have to protect against that.
     */

    DEV_HDR *devhdr;
    char name_tail_buf[16];
    char *name_tail = name_tail_buf;
    
    /* use iosDevFind to look up device name. Note how this works: If 
     * /vc40a/[1-4] exist and iosDevFind is passed "/vc40a16", it'll return
     * a string "6" in the name-tail field. If it's passed "/vc40a", it'll
     * return the devhdr of the default device (/null). Simply horrible.
     */
    devhdr = iosDevFind (name, &name_tail);

    /* if name_tail points to whole name, there's no such device. */
    if (strcmp (name, name_tail) == 0)
    {
	errno = ENOENT;
	return (-1);
    }

    /* if name_tail isn't a zero-length string, it found a "match" as 
     * described above. We don't want that, either!
     */

    if (strlen (name_tail) != 0)
    {
	errno = ENOENT;
	return (-1);
    }

    /* safe to open... */

    return (open (name, o_flags, prot_mask));
#endif /* vxworks */
}

/***************************************************************************
 * read a DSP's BCR (Boot Control Register)
 * Hydra-II only.
 */
int c40_read_bcr(c40id, bcrval)
int c40id;
u_short *bcrval;
{
    return (ioctl(c40id, VC40READBCR, bcrval));
}

/***************************************************************************
 * write a DSP's BCR (Boot Control Register)
 * Hydra-II only.
 */
int c40_write_bcr(c40id, bcrval)
int c40id;
u_short bcrval;
{
    return (ioctl(c40id, VC40WRITEBCR, &bcrval));
}

/***************************************************************************
 * read a DSP's ICR (Interrupt Control Register)
 * Hydra-II only.
 */
int c40_read_icr(c40id, icrval)
int c40id;
u_short *icrval;
{
    return (ioctl(c40id, VC40READICR, icrval));
}

/***************************************************************************
 * write a DSP's ICR (Interrupt Control Register)
 * Hydra-II only.
 */
int c40_write_icr(c40id, icrval)
int c40id;
u_short icrval;
{
    return (ioctl(c40id, VC40WRITEICR, &icrval));
}
