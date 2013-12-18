/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- link.h								--
--                                                                      --
--	Link structure definition					--
--                                                                      --
--	Author:  NHG 6/9/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: link.h,v 1.12 1993/07/27 13:59:18 paul Exp $ */

#ifndef __link_h
#define __link_h

#ifndef __helios_h
#include <helios.h>
#endif

#include <queue.h>
#include <message.h>
#include <sem.h>
#include <config.h>
#ifndef __TRAN
# include <cpustate.h>
#endif

#ifndef _ID_
#define _ID_
struct Id { word TopSecret; };
struct DBInfo { word AlsoSecret; }
#endif

typedef struct LinkInfo {
	byte		Flags;		/* flag byte			*/
	byte		Mode;		/* link mode/type		*/
	byte		State;		/* link state			*/
	byte		Id;		/* link id used in ports etc.	*/
#ifdef __TRAN
	Channel		*TxChan;	/* address of tx channel	*/
	Channel		*RxChan;	/* address of reception channel	*/
#else
	Channel		TxChan;		/* identification of tx channel	*/
	Channel		RxChan;		/* identification of rx channel	*/
					/* WPTR to link ctrl reg on C40	*/
#endif
	struct Id	*TxUser;	/* pointer to user of tx chan	*/
	struct Id	*RxUser;	/* user of rx chan		*/
	word		MsgsIn;		/* number of input messages	*/
	word		MsgsOut;	/* number of output messages	*/
	struct Id	*TxQueue;	/* queue of waiting transmitters*/
	struct Id	*RxId;		/* current message receiver	*/
	WordFnPtr	TxFunction;	/* optional device Tx function	*/
	WordFnPtr	RxFunction;	/* optional device Rx function	*/
	struct Id	*Sync;		/* synchronisation point	*/
	Port		LocalIOCPort;	/* port to be used by our LinkIOC */
	Port		RemoteIOCPort;	/* port to remote IOC		*/
	word		Incarnation;	/* remote processors incarnation number */
	word		MsgsLost;	/* messages lost/destroyed	*/
	struct DBInfo	*DBInfo;	/* double buffer process info	*/
	word		Timeout;	/* hardware-specific timeout	*/
#ifndef __TRAN
					/* must preserve ordering of next two */
	SaveState	*TxThread;	/* LinkTx thread 		*/
	SaveState	*RxThread;	/* LinkRx thread		*/
# ifdef __C40
	word		DMAEng;		/* shared split mode DMA engine	*/
					/* WPTR to DMA control register	*/
# endif
# if defined (__C40) || defined (__ARM)
	Semaphore	HalfDuplex;	/* signaled when link available	*/
# endif
#endif
} LinkInfo;


#define Link_Flags_parent	0x40	/* indicates the link which booted us */
#define Link_Flags_ioproc	0x20	/* indicates an io processor	*/
#define Link_Flags_debug	0x10	/* debugging link for IOdebug	*/
#define Link_Flags_report	0x08	/* report state changes		*/
#define Link_Flags_stopped	0x04	/* link traffic has been stopped*/
#if defined(__C40) || defined(__ARM)
# define Link_Flags_HalfDuplex	0x80	/* use half duplex protocol */
#endif

#define Link_Mode_Null		0	/* not connected to anything	*/
#define Link_Mode_Dumb		1	/* link is a dumb device	*/
#define Link_Mode_Intelligent	2	/* part of Helios network	*/
#define Link_Mode_Special	3	/* link is a non-std comms link */

#define Link_State_Null		0	/* not connected to anything	*/
#define Link_State_Booting	1	/* booting remote processor	*/
#define Link_State_Dumb		2	/* dumb device			*/
#define Link_State_Running	3	/* live network link		*/
#define Link_State_Timedout	4	/* doing idle exchange		*/
#define Link_State_Crashed	5	/* remote processor has crashed	*/
#define Link_State_Dead		6	/* remote processor not running	*/


extern word BootLink(word link, void *image, Config *config, word confsize);
extern word EnableLink(word link);
extern word SoftReset(word link);
extern word AllocLink(word link);
extern word FreeLink(word link);
extern word Reconfigure(LinkConf *lcvec);
extern word Configure(LinkConf newconf);
extern void Terminate(void);
extern word LinkData(word link, LinkInfo *info);
extern word LinkIn(word size, word link, void *buf, word timeout);
extern word LinkOut(word size, word link, void *buf, word timeout);

#ifdef __C40
extern word MP_LinkIn(word size, word link, MPtr buf, word timeout);
extern word MP_LinkOut(word size, word link, MPtr buf, word timeout);
#endif


#ifndef __TRAN
/* Structure used to pass information to a link transfer procedure.	*/
/* (This is needed because CallWithModTab passes only two arguments.)	*/

typedef struct LinkTransferInfo {
	word		size;	/* Transfer size (0 for AbortTx/Rx())	*/
	LinkInfo	*link;	/* Link structure			*/
#ifdef __C40
	MPtr		buf;	/* Buffer pointer			*/
#else
	void		*buf;	/* Buffer pointer			*/
#endif
} LinkTransferInfo;

/* Names for the fields of the LinkInfo structure which are (mis)used 	*/
# define LinkInfo_DCB		TxChan	/* DCB pointer			*/
# define LinkInfo_ModTab	RxChan	/* Module table pointer		*/

# ifdef __ABC
/* Export extra link routines to allow a serial servers, etc to run 	*/
/* link guardians on their ports.					*/

void IntelligentServer(LinkInfo *link);

#  define SERIALLINKID 1	/* Link 1 is the serial line */
# endif
#endif /* __ABC */


#endif /* __link_h */


/* -- End of link.h */
