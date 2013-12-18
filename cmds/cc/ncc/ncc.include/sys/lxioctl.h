/* @(#)lxioctl.h	1.1	(ULTRIX)	1/27/89 */

/*
 * Shadowfax IOCTL definitions
 */
 
/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/***************************************************************************
*	revision history:
****************************************************************************
*
* 06 jul 87  longo  added command for console exposure event reporting
* 15 apr 87  longo  changed device programming structure reference
* 11 apr 87  longo  removed LX_SUBSYSTEM_CONFIG command
* 10 mar 87  longo  chaged LX_MAXDEVICES to LX_NUM_QUEUE_TYPES
* 22 feb 87  longo  added LX_SUBSYSTEM_CONFIG and LX_PRG_SPARE
* 16 feb 87  longo  added console constrol commands
* 04 feb 87  longo  changed MAXDEVICES to LX_MAXDEVICES
* 29 jan 87  longo  changed dial status ioctl command content
* 28 jan 87  longo  changed location of this file to <sys/lxioctl.h>
* 23 jan 87  longo  changed include from lxevent.h to lxuser.h
* 24 dec 86  longo  changed LX_STRM_RELEASE_BLOCKS from read to read/write
* 18 dec 86  longo  changed _vs_event to say lx_event
* 04 dec 86  longo  removed LX_CUR command as obsolete
* 18 nov 86  longo  added LX_MAP_ACP and LX_UNMAP_ACP commands
* 10 nov 86  longo  added LX_STRM_MAP_ADDRESS command
* 10 oct 86  longo  added entries for input device prg'ing and cursor def
* 20 aug 86  longo  changed LX_DETACH to pass only a char (vse_type)
* 17 jul 86  longo  added structure reference to LX_STRMEM_ADRS
* 15 jul 86  longo  added structure memory allocation commands
* 19 jun 86  longo  added hardware/software reset commands
* 08 jun 86  longo  added BVP command words
* 24 mar 86  longo  created
*
***************************************************************************/
 
#ifdef KERNEL
#include "../h/ioctl.h"
#include "../h/lxuser.h"
#include "../h/errlog.h"
#else
#include <sys/ioctl.h>
#include <sys/lxuser.h>
#include <sys/errlog.h>
#endif
 
/* ioctl commands for input delivery control */
 
#define LX_ALLOC_Q	_IOR('x', 1, LX_EVENT_QUEUE *)
#define LX_DEALLOC_Q	_IOW('x', 2, LX_EVENT_QUEUE *)
#define LX_ATTACH	_IOW('x', 3, struct attach)
#define LX_DETACH	_IOW('x', 4, char)
#define LX_GETDEFAULT	_IOR('x', 5, LX_EVENT_QUEUE *)
#define LX_INPUT_STOP	_IO('x', 6)
#define LX_INPUT_RESTART _IO('x', 7)
#define LX_INQUIRE_Q	_IOR('x', 8, LX_EVENT_QUEUE*[LX_NUM_QUEUE_TYPES])
#define LX_REGISTER	_IOW('x', 9, LX_EVENT_QUEUE *)
#define LX_UNREGISTER	_IOW('x', 10, LX_EVENT_QUEUE *)
#define LX_SOFT_RESET	_IO('x', 11)
#define LX_HARD_RESET	_IO('x', 12)
 
/* ioctl commands for programming the input devices */
 
#define LX_PRG_KB	_IOW('x', 30, struct prg_device)
#define LX_PRG_MOUSE	_IOW('x', 31, struct prg_device)
#define LX_PRG_TABLET	_IOW('x', 32, struct prg_device)
#define LX_PRG_DIALBOX	_IOW('x', 33, struct prg_device)
#define LX_PRG_BBOX	_IOW('x', 34, struct prg_device)
#define LX_PRG_SPARE	_IOW('x', 35, struct prg_device)
 
/* ioctl commands for structure memory management */
 
#define LX_STRM_ADRS 	  	  _IOWR('x', 40, struct lx_strm_map)
#define LX_STRM_ALLOCATE_BLOCKS   _IOWR('x', 41, struct lx_strm_allocate_blocks)
#define LX_STRM_ALLOCATE_SPECIFIC _IOWR('x',42, struct lx_strm_allocate_specific)
#define LX_STRM_RELEASE_BLOCKS	  _IOWR('x', 43, struct lx_strm_release_blocks)
#define LX_STRM_MAP_ADDRESS	  _IOWR('x', 44, struct lx_strm_map_address)
 
/* miscellaneous commands  */
 
#define LX_GET_TEST     	_IOR('x', 50, struct el_lxerr)
 
/* console commands */
 
#define LX_CONS_STAND_MODE	_IO('x', 60) /* enter standalone display mode */
#define LX_CONS_LOOP_MODE	_IO('x', 61) /* enter loopback display mode */
#define LX_CONS_OPER_MODE	_IO('x', 62) /* enter operation display mode */
#define LX_CONS_MCODE_LOAD	_IO('x', 63) /* micro-code loading begining */
#define LX_CONS_EXPOSE		_IOW('x', 64, struct expose_info)
 
/* temporary ioctl's for testing purposes */
 
#define LX_TEST		_IOW('x', 100, struct lx_event) /* pass dummy event */
#define LX_MAP_ACP	_IOR('x', 101, char *) 	/* map the ACP */
#define LX_UNMAP_ACP	_IO('x', 102) 		/* unmap the ACP */

