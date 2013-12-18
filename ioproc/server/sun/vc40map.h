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


/* $Id: vc40map.h,v 1.1 1994/06/29 13:46:19 tony Exp $ */

/* vc40map.h */

#ifndef VC40MAP_H
#define VC40MAP_H 1

#if defined(MSDOS) && !defined(__MSDOS__) /* MicroSoft always has to be different */
#   define __MSDOS__
#endif

#if defined(__MSDOS__) || defined(__OS2__)
#   include <stdlib.h>
#   if defined(__BORLANDC__)
#       include <dir.h>
#   endif
#endif
#include <fcntl.h>
#include "vc40all.h"
#include "coff.h"
#include "c40util.h"
#include "c40types.h" /* for u_long, etc. */

/*
 * User configurable parameters
 */

/*
 * Maximum number of Hydras supported
 */
#define	MAX_HYDRA	8

/*
 * prototypes
 */
#if defined(__MSDOS__) || defined(__OS2__)
    u_long far c40_map(char far *hydra_descr);
    
#   if defined(EPC)
        int far c40_enint(int c40, u_long ipri, u_long ivec);
        int far c40_dsint(int c40);
        int far c40_open(char far *dspid, int mode);
        short far c40_wait(u_short mask, u_long far *intvec, u_long ticks);
#   endif /* EPC */
        


#elif defined(__STDC__)
    u_long c40_map(char *hydra_descr);
    int c40_open(char *dspid, int mode);
#else
    u_long c40_map();
    int c40_open();
#endif

#endif /* #ifndef VC40MAP_H */

