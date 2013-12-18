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

/* $Id: */

/* c40types.h
 * 
 * centralized place to typedef u_{char,short,long}
 */

#ifndef C40TYPES_H
#define C40TYPES_H

#include "portable.h"

/* 
 * u_{char,short,long} definitions 
 */

#if defined(unix) || defined(VXWORKS) || defined(vxworks)
#   if defined(VXWORKS) || defined(vxworks)
#      include "vxWorks.h"
#   endif
#   include <sys/types.h>
#else
    typedef unsigned long u_long;
    typedef unsigned short u_short;
    typedef unsigned char u_char;
#endif

#endif /* C40TYPES_H */
