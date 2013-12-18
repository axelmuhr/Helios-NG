/*
 *
 * Copyright (C) 1985,  by
 * Digital Equipment Corporation, Maynard, Mass.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 *
 * Networks & Communications Software Engineering
 *
 * IDENT HISTORY:
 *
 * 1.00 10-Jul-1985
 *      DECnet-ULTRIX   V1.0
 *
 */

/* "@(#)nsp_addr.h	1.2	1/31/89" */

/*
 * Temporary file for support of the SIOCSNETADDR and SIOCGNETADDR ioctl calls
 * for setting up and reading the DECnet node address.
 */
#include "../../h/ioctl.h"

#define SIOCSNETADDR _IOW('s', 20, int)		/* set DECnet node address */
#define SIOCGNETADDR _IOR('s', 21, int)		/* get DECnet node address */
