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


/* $Id: vc40dsp.h,v 1.2 1994/06/29 13:46:19 tony Exp $ */

/* vc40dsp.h
 *
 * everything needed to use the Hydra SunOS device driver and utility library.
 */

#ifndef VC40DSP_H
#define VC40DSP_H 1

#include <errno.h>

#if defined( unix ) || defined( VXWORKS )
#   include <fcntl.h>
#   include <unistd.h>
#   include <sys/ioctl.h>
#endif

#if defined( unix )
#   include <sys/mman.h>
#endif

#include "vc40all.h"
#include "coff.h"
#include "c40util.h"
#include "c40types.h" /* because we define structures using u_long, etc. */
/*
 * V-C40 driver ioctl()'s
 */
#if defined(__STDC__)
#   define VC40RUN      (_IOW('v',0,u_long))
#   define VC40RESET    (_IO('v',1))
#   define VC40GETINFO  (_IOR('v',2,struct vc40info))
#   define VC40ENINT    (_IOW('v',3,int))
#   define VC40DSINT    (_IO('v',4))
#   define VC40HALT     (_IO('v',5))
#   define VC40SETADDR  (_IOW('v',6,u_long))
#   define VC40DISKEY   (_IO('v',7))
#   define VC40ENKEY    (_IO('v',8))
#   define VC40ATTACH   (_IOWR('v',9,long))
#   define VC40DETACH   (_IOWR('v',10,long))
#   define VC40GETPROP  (_IOWR('v',11,struct vc40property))
#   define VC40TRAP     (_IOW('v',13,u_long))
#   define VC40IOF2     (_IO('v',14))
#   define VC40RIPCR    (_IOWR('v',15,struct ipcr))
#   define VC40WIPCR    (_IOW('v',16,struct ipcr))
#   define VC40WAIT     (_IOW('v',17,int))
#   define VC40POLL     (_IOR('v',18,u_long))
#   define VC40RMCR     (_IOR('v',19,u_long))
#   define VC40WMCR     (_IOW('v',20,u_long))
#   define VC40GETSEM   (_IOW('v',21,u_long))
#   define VC40RELSEM   (_IOW('v',22,u_long))
#   define VC40READFLASH    (_IOWR('v',23,struct h2flash))
#   define VC40WRITEFLASH   (_IOW('v',24,struct h2flash))
#   if defined( VXWORKS )
#       define VC40MMAP         (_IOR('v',25,struct vc40mmap))
#       define VC40CALLBACK     (_IOW('v',26,struct vc40callback))
#   endif
#   define VC40WIPE     (_IO('v',27))
#   define VC40READBCR  (_IOR('v',28,u_short))
#   define VC40WRITEBCR  (_IOW('v',29,u_short))
#   define VC40READICR  (_IOR('v',30,u_short))
#   define VC40WRITEICR  (_IOW('v',31,u_short))

#else
#   define VC40RUN      (_IOW(v,0,u_long))
#   define VC40RESET    (_IO(v,1))
#   define VC40GETINFO  (_IOR(v,2,struct vc40info))
#   define VC40ENINT    (_IOW(v,3,int))
#   define VC40DSINT    (_IO(v,4))
#   define VC40HALT     (_IO(v,5))
#   define VC40SETADDR  (_IOW(v,6,u_long))
#   define VC40DISKEY   (_IO(v,7))
#   define VC40ENKEY    (_IO(v,8))
#   define VC40ATTACH   (_IOWR(v,9,long))
#   define VC40DETACH   (_IOWR(v,10,long))
#   define VC40GETPROP  (_IOWR(v,11,struct vc40property))
#   define VC40TRAP     (_IOW(v,13,u_long))
#   define VC40IOF2     (_IO(v,14))
#   define VC40RIPCR    (_IOWR(v,15,struct ipcr))
#   define VC40WIPCR    (_IOW(v,16,struct ipcr))
#   define VC40WAIT     (_IOW(v,17,int))
#   define VC40POLL     (_IOR(v,18,u_long))
#   define VC40RMCR     (_IOR(v,19,u_long))
#   define VC40WMCR     (_IOW(v,20,u_long))
#   define VC40GETSEM   (_IOW(v,21,u_long))
#   define VC40RELSEM   (_IOW(v,22,u_long))
#   define VC40READFLASH    (_IOWR(v,23,struct h2flash))
#   define VC40WRITEFLASH   (_IOW(v,24,struct h2flash))
#   if defined( VXWORKS )
#       define VC40MMAP         (_IOR(v,25,struct vc40mmap))
#       define VC40CALLBACK     (_IOW(v,26,struct vc40callback))
#   endif
#   define VC40WIPE     (_IO(v,27))
#   define VC40READBCR  (_IOR(v,28,u_short))
#   define VC40WRITEBCR  (_IOW(v,29,u_short))
#   define VC40READICR  (_IOR(v,30,u_short))
#   define VC40WRITEICR  (_IOW(v,31,u_short))
#endif

#define	VC40_DETACHED	(-1)

/*
 * special cases for mmap()
 */
#define VC40MMAPVIC     0x70000000
#define VC40MMAPJMCR    0x60000000

#if defined( VXWORKS )
    struct vc40mmap {
        u_long offset;      /* passed in: offset into Hydra DRAM */
        u_char *addr;       /* passed out: pointer to Hydra DRAM */
    };

    struct vc40callback {
        void (*funcptr)(void *);  /* passed in: function to call from ISR */
        void *arg;                /* passed in: argument to pass to function */
    };
#endif

struct vc40property {
    u_long  propid;     /* property ID */
    u_long  propval;    /* property value */
};

#endif /* VC40DSP_H */

