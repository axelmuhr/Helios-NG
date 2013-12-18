/*
 * File name:	b407.h
 *
 *	Header file for b407.c.
 *
 * Version:	1.0
 *
 * Author:	Robert Wipfel
 *
 * Copyright INMOS Limited 1991
 *
 * Revision History:
 *
 *	31-JAN-1991	RAW	Created.
 *
 */

#include <helios.h>	/* Helios standard header	*/
#include <fcntl.h>	/* Posix file control		*/
#include <syslib.h>	/* System library		*/
#include <link.h>	/* Link structure definitions	*/
#include <stdio.h>	/* ANSI standard header		*/
#include <chanio.h>	/* Channel support library	*/

/*
 * Firmware initialisation status
 */
 
#define B407_START_OK	1	/* Firmware started ok		*/
#define B407_START_FAIL	0	/* Firmware failed to start	*/

/*	
 * Buffer sizes for loading firmware
 */

#define	CODE_BUFFER_SIZE	512	/* Bootable code buffer size	*/ 
#define I_PACKET_SIZE		512	/* Maximum ISERVER packet size	*/ 

/*
 * B407 firmware file location
 */

#define B407_FIRMWARE		"/helios/lib/b407.b2h"

/*
 * Minimum / Maximum ethernet packet lengths
 */
 
#define B407_MIN_PACKET_LENGTH	64
#define B407_MAX_PACKET_LENGTH	1514	/* Includes ethernet header */

/*
 * B407 link timeout value
 */

#define	B407_LINK_TIMEOUT	32	/* Timeout when loading code */


