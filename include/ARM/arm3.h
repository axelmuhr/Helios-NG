/*
 * File:	arm3.h
 * Subsystem:	Helios/ARM implementation
 * Author:	P.A.Beskeen
 * Date:	Oct '92
 *
 * Description: ARM3 specific manifests
 *
 *
 * RcsId: $Id: arm3.h,v 1.1 1993/08/03 17:11:45 paul Exp $
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 *     All Rights Reserved.
 * 
 */

#ifndef	__arm3_h
#define	__arm3_h

/* coprocessor 15 cache control bits */

#define	CACHE_ON	0
#define	CACHE_OFF	1
#define	CACHE_SET	2
#define	CACHE_GET	3
#define	CACHE_NOPROTECT	4

#define	CACHEABLE	0
#define	UPDATEABLE	1
#define	DISRUPTIVE	2
#define	CONTROL		3


#endif /*__arm3_h*/


/* end of arm3.h */
