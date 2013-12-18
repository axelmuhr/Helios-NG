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


/* $Id: vc40.h,v 1.1 1994/06/29 13:46:19 tony Exp $ */
/* vc40.h
 *
 * This header includes the correct Hydra header depending on the
 * command line define:
 *      -DSUNOS     For SunOS device driver
 *      -DSUNMAP    For SunOS memory-mapped library
 *      -DLYNXMAP   For LynxOS memory-mapped library
 *      -DEPC       For Radisys EPC-5 s/w (MS DOS based).
 */

#ifndef VC40_H
#define VC40_H 1

#include "vc40all.h"
#if defined( SUNOS ) || defined( VXWORKS ) 
#   include "vc40dsp.h"
#endif

#if defined(SUNMAP) || defined(LYNXMAP) || defined(EPC)
#   include "vc40map.h"
#endif

#endif /* #ifndef VC40_H */

