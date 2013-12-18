/*
 * File name:	b407dev.h
 *
 *	Header file for b407dev.c.
 *	
 * Version:	1.0
 *
 * Author:	Robert Wipfel
 *
 * Copyright INMOS Limited 1991
 *
 * Revision History:
 *
 *	15-FEB-1991	RAW	Created.
 *
 */

#include <device.h>	/* Generic device interface	*/
#include <syslib.h>	/* Helios system library	*/
#include <nonansi.h>	/* Helios ANSI extensions	*/
#include <queue.h>	/* Queue library		*/
#include <sem.h>	/* Semaphore library		*/
#include <codes.h>	/* Helios error codes		*/
#include <string.h>	/* ANSI string library		*/

/*
 * To B407 hard link number
 */

#ifdef B407_LINK0
#define B407_LINK	0
#else
#ifdef B407_LINK1
#define B407_LINK	1
#else
#ifdef B407_LINK2
#define B407_LINK	2
#else
#define B407_LINK	3	/* If no B407_LINK defined, defaults here */
#endif
#endif
#endif

/*
 * Network device control block
 */

typedef struct
{
	DCB		dcb;
	Semaphore	lock;
	List		readq;
	Semaphore	nreq;
	word		Link;
	char		Addr[6];
	word		alwrites;
	word		alsize;
	char		albuffer[2048];
} NetDCB;

/*
 * External B407 functions
 */
 
extern WORD b407_open_link( WORD );
extern WORD b407_load_firmware( WORD ) ;
extern WORD b407_init_firmware( WORD, char * );
extern void b407_read_packet( WORD, BYTE *, WORD * );
extern void b407_write_packet( WORD, BYTE *, WORD );
extern void b407_close_link( WORD );

