/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1993, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      parsy.c                                                         --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: par_link.c,v 1.1 1993/09/28 14:24:17 bart Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.        			*/

/**
*** THIS CODE IS NOT PART OF THE STANDARD SOURCE RELEASE. IT SHOULD BE
*** USED ONLY IN-HOUSE AND FOR ANY PARSYTEC SOURCE RELEASES.
**/

/*
 * linkp.c	General interface to transputer hardware
 *
 * Copyright (C) Parsytec GmbH, 1989.
 * Copyright (C) CWA mbH, 1991 (for all parts concerning SBUS/BBK-S4).
 *
 * Author:	Matthias Clauss
 *		Design assistents by Armin Joachimsmeyer
 *
 *		Michael Raus (all parts concerning SBUS/BBK-S4)
 *
 *
 * Date:	9.6.1989	Created
 *
 * Version:	1.30
 *
 * Updates:
 *	Vers	date	author	description
 *	1.01	890705	mc	accept decimal int as board-name,
 *	1.02	890707	mc	additional funct 'LinkInfo'
 *	1.03	890801  mc	automatic timeout measurement added
 *	1.04	890803  mc	configuration support added
 *	1.05	890912	mc	semctl(2) call bug fixed
 *	1.06	890915	aj	ReadLink, WriteLink bugs fixed
 *	1.07	900129	mc	mmap SunOs 3.x compatible,
 *				check overflow in timeout counting
 *	1.08	190290	mc	bug from 1.07 return set gid fixed
 *	1.09	270290	mc	mtm-sun basic support added
 *	1.10	140590	mc	mtm-sun 1/2 ncu support
 *	1.11	270690	mc	mtm-sun config support
 *	1.12	090890	mc	create semaphore with egid
 *
 *	1.12*	290791	mr	support of SBus device "BBK-S4" added
 *	1.13	171091	mc	set gid bug 1.12* fixed
 *	1.14	130192 	mr	getpid() inserted (ReadLink(),WriteLink())
 *	1.30*	160192	mr	GetPacket(), PutPackets()
 *		260192	mr	PacketMode(),StreamMode(),PacketInfo(),
 *				WriteLink()/Packet
 *		280191	mr	ResetLink()
 *		170292	mr	clean up the system
 *		090492	mr	ReadLink(): timeout handling (fetching
 *				DPR contents)		
 *	1.30	180592	mc	cleanup, signed *Buffer fixed, sbus scale
 *      1.30a   240993  blv     integrated with I/O Server
 */


/*
 ******************************************************************
 *
 *	Some faster Sparcs (IPX, Sparc 2, ELC and MP maschines
 *	use a cache. This prevents a coupple of polling loops from
 *	getting an impression of the real world. The only 'fix' known
 *	so far:	getpid(2) invalidates the cache.
 *	SBus technical support group does NOT know any more.
 *	So inside every polling loop there is a getpid(2) call.
 *	They may get activated through 
 *
 *	#define IPX
 *
 *******************************************************************
 */


#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <fcntl.h>
#include <grp.h>
#include <signal.h>
#include <values.h>

#ifndef HELIOS_IOSERVER
#include "link.h"
#else
#include "../parsy/link.h"
#endif

#ifndef HELIOS_IOSERVER
static char *SccsId = "@(#)link.c	1.3 dated 5/19/92";		/* Sccs Id */
#endif

#if defined(HELIOS_IOSERVER) && defined(SUN4)
#define IPC 1
#endif

#define	MAXLINE		256	/* line-length in config file */

/* Public functions declared in this file
 *
 *	int   OpenLink    (Name);
 *	int   CloseLink   (LinkId);
 *	int   ReadLink    (LinkId, Buffer, Count, Timeout);
 *	int   WriteLink   (LinkId, Buffer, Count, Timeout);
 *	int   ResetLink   (LinkId);
 *	int   AnalyseLink (LinkId);
 *	int   TestError   (LinkId);
 *	int   TestRead    (LinkId);
 *	int   TestWrite   (LinkId);
 *  === The remaining functions are Parsytec private added
 *	int   InfoLink    (struct ext_link_info **LinkPtr)
 *	char *lerror      (ErrorCode);
 *  === Packet functions added for BBK-S4 Firmware V1.3
 *	int   PacketMode  (LinkId, PacketSize);
 *	int   StreamMode  ();
 *	int   GetPacket   (char *Buffer);
 *	int   PutPackets  (struct packet *Packets, u_char Count);
 *	struct packetInfo *InfoPacket ();
 *  === Internal usage only, please! (s4diag)
 *	word TestFirmwareCode ();
 */



/* The following variables are defined in additional file 'lconf.c'
 * and are declared here as external references.
 * This has been done in order to let the installing administrator
 * chose suitable defaults for all these variables.
 * The only realy interresting variables are:
 *
 *	vme_dev_name		default:   "/dev/vme32d16"
 *	config_file		default:   "/usr/etc/transp/trans_conf"
 *
 * BLV - for the I/O Server these variables are defined in parsy/parsy.c,
 * not in lconf.c
 */

extern char *config_file;
extern char *env_config;
extern char *vme_group_name;
extern char *vme_dev_name;
extern char *sbus_dev_name;
extern key_t sem_link_key;	/* semaphore transputer key */
extern int   default_tenth;	/* default 0.1s loop count */
extern unsigned reset_raise;	/* reset raise time in usec */
extern unsigned reset_fall;	/* reset fall  time in usec */


static int sem_id     = -1;	/* semaphore identifier for transputer links */
static int vme_device = -1;	/* file descriptor for VME-bus device */
static int usr_gid    = -1;	/* real gid */
static int vme_gid    = -1;	/* saved effective gid to access VME-bus */
static int MaxLinkId  = -1;	/* total number of links declared
				 * Valid LinkIds are:
				 * 0 <= LinkId < MaxLinkId
				 */
static int time_tenth = -1;	/* # loops for 1/10 th second
				 * this is the default value. 'init_timeout()'
				 * measures correctly using 'itimer(2)'
				 */
static int time_max   = -1;	/* maximum timeout time in 1/10 th second.
				 * Longer timeout cannot be expressed
				 * on this machine within a 'int'. Otherwise
				 * integer arithmetics overflow will occur.
				 * Set by 'init_timeout()'.
				 */


/* symbolic bus types	*/

#define	VMEBUS	((short)0)	/* VMEBUS device */
#define SBUS	((short)1)	/* SBus device */


#define	SBUSMASK      ((word)0xAA00) /* Mask definitions */
#define RESET         ((word)0x1111)
#define INIT_STREAM   ((word)0x2222)
#define INIT_PACKET   ((word)0x3333)
#define TIMEOUT_READ  ((word)0x4444)
#define TIMEOUT_WRITE ((word)0x5555)
#define VERSION	      ((word)0x6666)

#define	SBUS_MIN_TIMEOUT  3	/*minimum SBus timeout in tenth of seconds*/ 

#ifdef	IPX
#define	STIME_SCALE	  7	/* BBK-S4 timeout is this factor longer */
#else
#define	STIME_SCALE	  2	/* BBK-S4 timeout is this factor longer */
#endif

#define	MAX_SBUS_SLOTS	  4	/* the maximum number of supported 
				 * SBus slots
				 */

#define RESET_TIME   0x1000	/* time for reset link signal active 
              			 * in 1/16 milli second (swapped !!!)
				 */
/* byte is defined in 'link.h' */

typedef unsigned short    word;
typedef unsigned int      lword;

/* describtion of a (ambigious) link adaptor.
 * A value of ANY means "not specified".
 * {VMTM, ANY, 0} means any VMTM board link 0
 * {ANY, ANY, ANY}  means any link on any board
 */

static struct LinkDesc {
	short	b_type;		/* short coded board type */
	short	b_no;		/* board number */
	short	b_slot;		/* SBus slot number */
	short	l_no;		/* link number on board */
	short	bus_type;	/* bus type */
};

/*
 * LinkInfo structure. Similar to FILE structure in <stdio.h>
 */

static struct LinkInfo {
	short	opened;		/* TRUE if successfull opened */
	short	packetlink;	/* TRUE if used as PacketLink */
	caddr_t	usr_ad;		/* ready to use ptr to link hardw. */
	struct LinkDesc ldes;	/* link descriptor */
	caddr_t vme_addr;	/* vme/SBus address of this link */
};

#define	B_type		ldes.b_type
#define	B_no		ldes.b_no
#define	B_slot		ldes.b_slot
#define	L_no		ldes.l_no
#define Bus_type	ldes.bus_type

static struct LinkInfo	*LinkInfo;	/* array of LinkInfo structs
					 * Initialized by init_config()
					 */
/* 
 * FlagAdr structure
 *
 * One of these FlagAdr structures belongs to each available link. The number
 * of available links (MaxId) depends on the contents of the 'trans_conf' file.
 * The FlagAdr structures are initialized by init_config() and each of them 
 * is filled with the correct values as soon as its OpenLink occurs.
 */

static	struct	FlagAdr {
	short	 half_chan_len;		/* half channel length */
	word	 *read_link_rq0;	/* ptr to readlink request flag 0 */
	word	 *read_link_rq1;	/* ptr to readlink request flag 1 */
	word 	 *blockin_ack;		/* ptr to blockin acknow. flag */
	word	 *chan_free;		/* ptr to channel free flag */
	word	 *write_link_rq0;	/* ptr to writelink request flag 0 */
	word	 *write_link_rq1;	/* ptr to writelink request flag 1 */
	word 	 *blockout_ack;		/* ptr to blockout acknow. flag */
	word	 *event_ack;		/* ptr to event acknowledge flag */
	word	 *init_event;		/* ptr to event initialisation */
	word	 *test_read;		/* ptr to test read flag */
	word	 *test_write;		/* ptr to test write flag */
	word	 *read_timeout;		/* ptr to read timeout flag */
	word	 *write_timeout;	/* ptr to write timeout flag */
};

static struct FlagAdr	*FlagAdr;	/* array of FlagAdr structs
					 * Initialized by init_config()
					 */

/*
 * Dual Ported RAM structure (BBK-S4 only)
 * Note: The BBK-S4 has a T222 in the beginning and is sold since 5.92
 *	 with an T225. This has NO impact on this code.
 */

static struct DpR {
	short	channel_off;	/* channel functional page offset */
	short	regs_off;	/* register functional page offset */
	short	com_off;	/* T222 <-> SBus communication register */
	short	channel_len;	/* standard channel length */
	short	regs_len;	/* link register length */
	short	int_s_t2_off;	/* interrupt register offset SBus -> T222 */
	short	int_t2_s_off;	/* interrupt register offset T222 -> SBus */ 
};


/*
 * SBusSlot structure.
 */

static struct SBusSlot {
	int	device;		/* file descriptor for SBus device in this
				 * SBus slot
				 */
	short 	mmap;		/* mmap=1 if memory mapping yet established
				 * for SBus device in this SBus slot
				 */
	unsigned space;		/* address space used by the SBus device
				 * in this SBus slot
				 */
	long	base;		/* SBus slot base address */
	struct DpR  dpr;	/* Dual ported RAM offsets and parameter */
};


/* array of SBusSlot structs. Initialized by init_config() */

static struct SBusSlot *SBusSlot;

/* symbolic board types */

#define	BBKV1	((short)0)	/* BBK-V1 only link adaptor */
#define	BBKV2	((short)1)	/* BBK-V2 1 transputer 2 Mb */
#define	VMTM	((short)2)	/* VMTM 4 transputer, link switch */
#define	BBKV4	((short)3)	/* BBK-V4 4 link-adaptors only */
#define	MTMSUN1	((short)4)	/* MTM-SUN 1 ncu 1-8 transputers */
#define	MTMSUN2	((short)5)	/* MTM-SUN 2 ncu 1-8 transputers */
#define	BBKS4	((short)6)	/* BBK-S4 SBus link adaptor */
#define	LINK	((short)7)	/* Dummy to specifiy a link by its index */


/*
 * here is an array which gives the usual names for
 * each board type.
 * The current maximum size is  8 characters.
 * The position corresponds to the value of symbolic board-names
 * (in file "vmeboard.h" e.g. VMTM).
 */
 
static char *(std_b_name []) = {
	"bbk-v1",
	"bbk-v2",
	"vmtm",
	"bbk-v4",
	"mtm-sun1",
	"mtm-sun2",
	"bbk-s4",
	"--LINK--"
};



/* VME bus/SBus transputer module descriptor.
 * Describes the name (as string) and the type and number
 * of link adaptors on this board.
 * For greater flexibility allow several names for the same board.
 */

#define	MAX_BOARD_LINKS	  9	/* the maximum number of links per board */

static struct board_descr {
	char *name;	/* board name as string */
	short type;	/* board name coded as int */
	short links;	/* number of link adaptors */
	short size[MAX_BOARD_LINKS];
			/* offset of i-th link from board base.
			 * Unfortunatly the VMTM uses {0x10, 0x20, 0x40, 0x80}
			 */
};

/* =======			W A R N I N G 
 * =======
 * =======	At the moment there is no check for consistency
 * =======	in the initialised values between 'links' and
 * =======	the constant 'MAX_BOARD_LINKS'.
 * =======	You are responsible to ensure that :
 * =======		every value 'links' < MAX_BOARD_LINKS
 */

static struct board_descr b_names[] = {
/*   name     type   links   offsets for each link on this board */

{ "bbkv1",   BBKV1,  1, {0x100, 0, 0, 0, 0, 0, 0, 0, 0} },
{ "bbk-v1",  BBKV1,  1, {0x100, 0, 0, 0, 0, 0, 0, 0, 0} },
{ "bbk_v1",  BBKV1,  1, {0x100, 0, 0, 0, 0, 0, 0, 0, 0} },
{ "BBKV1",   BBKV1,  1, {0x100, 0, 0, 0, 0, 0, 0, 0, 0} },
{ "BBK-V1",  BBKV1,  1, {0x100, 0, 0, 0, 0, 0, 0, 0, 0} },
{ "BBK_V1",  BBKV1,  1, {0x100, 0, 0, 0, 0, 0, 0, 0, 0} },
{ "bbkv2",   BBKV2,  1, {0, 0, 0, 0, 0, 0, 0, 0, 0} },
{ "bbk-v2",  BBKV2,  1, {0, 0, 0, 0, 0, 0, 0, 0, 0} },
{ "bbk_v2",  BBKV2,  1, {0, 0, 0, 0, 0, 0, 0, 0, 0} },
{ "BBKV2",   BBKV2,  1, {0, 0, 0, 0, 0, 0, 0, 0, 0} },
{ "BBK-V2",  BBKV2,  1, {0, 0, 0, 0, 0, 0, 0, 0, 0} },
{ "BBK_V2",  BBKV2,  1, {0, 0, 0, 0, 0, 0, 0, 0, 0} },
{ "vmtm",    VMTM,   4, {0x10, 0x20, 0x40, 0x80, 0, 0, 0, 0, 0} },
{ "VMTM",    VMTM,   4, {0x10, 0x20, 0x40, 0x80, 0, 0, 0, 0, 0} },
{ "bbkv4",   BBKV4,  4, {0x10, 0x20, 0x40, 0x80, 0, 0, 0, 0, 0} },
{ "bbk-v4",  BBKV4,  4, {0x10, 0x20, 0x40, 0x80, 0, 0, 0, 0, 0} },
{ "bbk_v4",  BBKV4,  4, {0x10, 0x20, 0x40, 0x80, 0, 0, 0, 0, 0} },
{ "BBKV4",   BBKV4,  4, {0x10, 0x20, 0x40, 0x80, 0, 0, 0, 0, 0} },
{ "BBK-V4",  BBKV4,  4, {0x10, 0x20, 0x40, 0x80, 0, 0, 0, 0, 0} },
{ "BBK_V4",  BBKV4,  4, {0x10, 0x20, 0x40, 0x80, 0, 0, 0, 0, 0} },
{ "mtmsun1", MTMSUN1,5, {0x80, 0x90, 0xA0, 0xB0, 0x100, 0, 0, 0, 0}},
{ "mtm-sun1",MTMSUN1,5, {0x80, 0x90, 0xA0, 0xB0, 0x100, 0, 0, 0, 0}},
{ "mtm_sun1",MTMSUN1,5, {0x80, 0x90, 0xA0, 0xB0, 0x100, 0, 0, 0, 0}},
{ "MTMSUN1", MTMSUN1,5, {0x80, 0x90, 0xA0, 0xB0, 0x100, 0, 0, 0, 0}},
{ "MTM-SUN1",MTMSUN1,5, {0x80, 0x90, 0xA0, 0xB0, 0x100, 0, 0, 0, 0}},
{ "MTM_SUN1",MTMSUN1,5, {0x80, 0x90, 0xA0, 0xB0, 0x100, 0, 0, 0, 0}},
{ "mtmsun2", MTMSUN2,9, {0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0,0x100}},
{ "mtm-sun2",MTMSUN2,9, {0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0,0x100}},
{ "mtm_sun2",MTMSUN2,9, {0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0,0x100}},
{ "MTMSUN2", MTMSUN2,9, {0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0,0x100}},
{ "MTM-SUN2",MTMSUN2,9, {0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0,0x100}},
{ "MTM_SUN2",MTMSUN2,9, {0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0,0x100}},
{ "bbks4",   BBKS4,  4, {0x100,0x480,0x800,0xB80, 0, 0, 0, 0, 0}},
{ "bbk-s4",  BBKS4,  4, {0x100,0x480,0x800,0xB80, 0, 0, 0, 0, 0}},
{ "bbk_s4",  BBKS4,  4, {0x100,0x480,0x800,0xB80, 0, 0, 0, 0, 0}},
{ "BBKS4",   BBKS4,  4, {0x100,0x480,0x800,0xB80, 0, 0, 0, 0, 0}},
{ "BBK-S4",  BBKS4,  4, {0x100,0x480,0x800,0xB80, 0, 0, 0, 0, 0}},
{ "BBK_S4",  BBKS4,  4, {0x100,0x480,0x800,0xB80, 0, 0, 0, 0, 0}},
{   NULL,    0,      0, {0, 0, 0, 0, 0, 0, 0, 0, 0} }
};
			
/*
 *	here the declarations for the hardware layout
 *	of the transputer modules.
 *	Currently supported:
 *		BBK-V1
 *		BBK-V2
 *		VMTM
 *		BBKV4		(VMTM board without transputers)
 *		MTM-SUN1	(only onboard ncu)
 *		MTM-SUN2	(additional ncu, total 2 ncu's)
 *		BBK-S4		(SBus board without transputers)
 *
 */



/* SBus descriptor.
 * Describes name (as string) and type of this board and the
 * number of supported SBus slots with their physical addresses
 * For greater flexibility allow several names for the same board.
 */


static struct sbus_descr {
	char	*name;		/* board name as string */
	short 	type;		/* board name coded as int */
	short 	slots;		/* number of SBus slots */
	int   	mem;		/* used address space */
	long	slot_base[MAX_SBUS_SLOTS];	/* physical SBus addresses */
	struct DpR   dpr;	/* Dual ported RAM offsets and parameter */
};

/* =======			W A R N I N G 
 * =======
 * =======	At the moment there is no check for consistency
 * =======	in the initialised values between 'slots' and
 * =======	the constant 'MAX_SBUS_SLOTS'.
 * =======	You are responsible to ensure that :
 * =======		every value 'slots' < MAX_SBUS_SLOTS
 */

static struct sbus_descr s_names[] = {
/* name     type slots mem    physical address for each SBus slot 
 *  offsets: channels  regs  comm.   channel length  regs length inter.offsets
 */

{ "bbks4",  BBKS4, 4, 0x6000, {0xF8000000, 0xFA000000, 0xFC000000, 0xFE000000},
	      0x4100, 0x5F00, 0x5F80, 0x380,  0x20, 0x5FFC, 0x5FFE },
{ "bbk-s4", BBKS4, 4, 0x6000, {0xF8000000, 0xFA000000, 0xFC000000, 0xFE000000},	
	      0x4100, 0x5F00, 0x5F80, 0x380,  0x20, 0x5FFC, 0x5FFE },
{ "bbk_s4", BBKS4, 4, 0x6000, {0xF8000000, 0xFA000000, 0xFC000000, 0xFE000000},
 	      0x4100, 0x5F00, 0x5F80, 0x380,  0x20, 0x5FFC, 0x5FFE },
{ "BBKS4",  BBKS4, 4, 0x6000, {0xF8000000, 0xFA000000, 0xFC000000, 0xFE000000},
	      0x4100, 0x5F00, 0x5F80, 0x380,  0x20, 0x5FFC, 0x5FFE },
{ "BBK-S4", BBKS4, 4, 0x6000, {0xF8000000, 0xFA000000, 0xFC000000, 0xFE000000},
	      0x4100, 0x5F00, 0x5F80, 0x380,  0x20, 0x5FFC, 0x5FFE },
{ "BBK_S4", BBKS4, 4, 0x6000, {0xF8000000, 0xFA000000, 0xFC000000, 0xFE000000},
	      0x4100, 0x5F00, 0x5F80, 0x380,  0x20, 0x5FFC, 0x5FFE },
{   NULL,    0,    0, 0,      {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0 }
};


#define READYMASK   0x01	/* for instatus and outstatus */




static struct slink_1 {		/* BBK-V1 */
	byte dummy0;		/* link adaptor at 0x100 */
	byte indata;	 /* 0x101 */
	byte dummy1;
	byte outdata;	 /* 0x103 */
	byte dummy2;
	byte instatus;	 /* 0x105 */	/* also stopreset */
	byte dummy3;
	byte outstatus;	 /* 0x107 */
	byte dummy4[0x81-0x7-1];
	byte startreset; /* 0x181 */
};

static struct slink_2 {		/* BBK-V2 */
	byte dummy0;
	byte indata;	 /* 0x1 */
	byte dummy1;
	byte outdata;	 /* 0x3 */
	byte dummy2;
	byte instatus;	 /* 0x5 */
	byte dummy3;
	byte outstatus;	 /* 0x7 */
	byte dummy4[3];
	byte startreset; /* 0xB */
	byte dummy5[3];
	byte stopreset;	 /* 0xF */
};

static struct slink_v {		/* VMTM */
	byte dummy0;
	byte indata;	 /* 0x1 */
	byte dummy1;
	byte outdata;	 /* 0x3 */
	byte dummy2;
	byte instatus;	 /* 0x5 */
	byte dummy3;
	byte outstatus;	 /* 0x7 */
	byte dummy4[5];
	byte startreset; /* 0xD */	/* should  N O T  be read */
	byte dummy5;
	byte stopreset;	 /* 0xF */	/* should  N O T  be read */
};

static struct slink_4 {		/* BBK-V4 */
	byte dummy0;
	byte indata;	 /* 0x1 */
	byte dummy1;
	byte outdata;	 /* 0x3 */
	byte dummy2;
	byte instatus;	 /* 0x5 */
	byte dummy3;
	byte outstatus;	 /* 0x7 */
	byte dummy4[5];
	byte startreset; /* 0xD */	/* should  N O T  be read */
	byte dummy5;
	byte stopreset;	 /* 0xF */	/* should  N O T  be read */
};

static struct slink_m {		/* MTM-SUN */
	byte indata;	 /* 0x0 */
	byte dummy0;
	byte outdata;	 /* 0x2 */
	byte dummy1;
	byte instatus;	 /* 0x4 */
	byte dummy2;
	byte outstatus;	 /* 0x6 */
	byte dummy3;
			  /* the following registers are WRITE ONLY */
	byte startresetLine;  /* 0x8 */	/* set reset link line      */
	byte startresetAdapt; /* 0x9 */	/* set reset link adaptor   */
	byte dummy4[2];
	byte stopresetLine;   /* 0xC */	/* clear reset link line    */
	byte stopresetAdapt;  /* 0xD */	/* clear reset link adaptor */
};

/* There are more advanced features on the MTM-SUN board.
 * At adr range 0x00 .. 0x7F every byte contains the same
 * instatus summary for the first 8 link adaptors. Adaptor-
 * number and bit-number correspond.
 * At adr range 0x180 .. 0x200 every byte contains the same
 * outstatus summary for the first 8 link adaptors. Adaptor-
 * number and bit-number correspond.
 * Link adaptor 8 (the ninth adaptor) is connected to the ncu
 * section on the board and should not be used by users.
 * The dual-ported DRAM section is sized 4 Mb and starts at
 * adr offset 0x1000.
 */ 


static struct slink_s {		/* BBK-S4 */ /* communication buffer
					      * T222 <-> SBus at 0xF80
					      */
 	word 	startreset[4];	/* 0xF80 */ /* start reset link line */
	word 	stopreset[4];	/* 0xF88 */ /* stop reset link line */
	word 	linkalive[4];	/* 0xF90 */ /* link alive register */
	word	resetwait[4];	/* 0xF98 */ /* reset halt flag */
	word	event_ack_sbus;	/* 0xFA0 */ /* event acknowledge to SBus flag */
};

static struct BBKS4_regs {	/* BBK-S4 */ /* read/write register */
	word	testread;	/* 0x00 */ /* test read flag */
	word	readlinkrq[2]; 	/* 0x02 */ /* [0]: ReadLink request */
				/* 0x04 */ /* [1]: number of bytes to read */
	word	blockinack;	/* 0x06 */ /* BlockIn acknowledge */
	word	channelfree;	/* 0x08 */ /* channel free flag */
	word	readtimeout;	/* 0x0A */ /* ReadLink timeout*/
	word	dummy1[2];
	word	testwrite;	/* 0x10 */ /* test write flag */
	word	writelinkrq[2];	/* 0x12 */ /* [0]: WriteLink request */
				/* 0x14 */ /* [1]: number of bytes to write */
	word	blockoutack;	/* 0x16 */ /* BlockOut acknowledge */
	word	writetimeout;	/* 0x18 */ /* WriteLink timeout */
};	


/*
 *	GetPacket(), PutPackets() definitions and variables
 *
 *	version from 26-01-92, 22.00 
 */

#define	NO_OF_DPR_WRITE_BUFS	2
#define	NO_OF_DPR_READ_BUFS	2
#define	DPR_BUF_LEN		0x80	/* buffer length: 128 byte */
#define PACKET_DATA_LEN		0x7C 	/* packet data length: 124 byte */
#define	PACKET_BODY_LEN		0x78	/* packet body length: 120 byte */
#define TOT_HEADER_LEN		0x5	/* total header length: 5 byte */
#define	MAX_REQS		0x7F	/* maximum number of packets to put */
#define	SIZE_OF_LONG		4
#define	LEN_PRXHDR		1	/* parix header length: 1 long word
					 *		          (4 byte)
					 */
/*
 *	Link modes for BBK-S4 only
 */

#define	PACKETMODE		0x0100
#define	STREAMMODE		0x0200
#define	INFOPACKETMODE		0x0300
#define	INITIALIZESTREAMMODE	0x0400
#define	INITIALIZEPACKETMODE	0x0500

/*
 * 	Global variables
 */

static	long	*Memory_write_address[NO_OF_DPR_WRITE_BUFS]; 
					/* virtual address to write to */

static	u_short *No_of_pack_to_write;	/* number of packets to write */
static	u_short *swap;
static	u_char	swap_no_of_pack[2];

static	long	*Long_length_write;
static	u_char	Long_length_write_array[4];
static	long	*Long_length_read;
static	u_char	Long_length_read_array[4];

static	u_short	*LinkMode;

static	u_short *Bff_write[NO_OF_DPR_WRITE_BUFS]; 	
					/* DPR write buffer free flag array */

static	int	No_of_copies;		/* number of long word copies */

static	long	*Memory_read_address[NO_OF_DPR_READ_BUFS];
					/* virtual address to read from */

static	u_short *Bff_read[NO_OF_DPR_READ_BUFS];	/* DPR read buffer free flag array */
/*static	u_short	*Act_DPR_read_buf;*/	/* actual DPR read buffer */
/*static	u_short	*Act_DPR_write_buf;*/	/* actual DPR write buffer */
static	u_short	Act_DPR_write_buf[1];
static	u_short	Act_DPR_read_buf[1];

/*
 * 	Packet definition
 *
 * Some words to prevent confusion with the packet length:
 * 'length' contains the size of the data pointed to by 'body'.
 * Anyway the number of byte transferred through the physical link-line
 * will be 'length' + 4 + 1. Why this?
 * The first byte is the 'length' byte exactly as specified.
 * The next 4 bytes (whatever 'length' contains, even if == 0) are the
 * Parix header, giving the destination of this packet.
 * Then comes the 'length' amount of data bytes. So: The shortest packet
 * which may be send is five bytes on the link-line.
 *
 * The packet-structure is defined in 'linkp.h'

struct	packet  {
	byte	length;		/* Packet header: bits 0..6 : body length
				 *		  bit  7    : EOM/EOP tag * /
	long	pxhdr;		/* Parix header: 2 byte proc-id,
				 *		 2 byte channel-no. * /
	byte	*body;		/* Packet body * /
};
 */


/*
 *	Packet info definition
 */

struct	packetInfo  {
	int	LinkId;		/* LinkId of current PacketLink (-1 if none) */
	int	PacketSize;	/* Maximum packet size */
	int	MaxBuffSize;	/* Maximum buffer size */
	int	InPackets;	/* Number of packets ready to GetPacket() */
	int	OutPackets;	/* Number of free PutPackets() buffers */
	int	OutPending;	/* Estimated number of pending output packets */
};

static	struct	packetInfo  GlobalPacketInfo = {
	-1,	/* Link Id */
	 0,	/* maximum packet size */
	 0,	/* maximum buffer size */
	 0,	/* number of packets to get */
	 0,	/* number of free write buffers */
	 0	/* number of pending out packets */
};

static struct packetInfo *GlobPackInfo;	/* pointer to GlobalPacketInfo */

struct packetInfo	*InfoPacket ();

void 	init_PutPackets ();
void 	init_GetPacket ();
int 	TestFirmwareCode ();

/* static string which receives a more detailed error message
 * Should be written by all error handling pieces of this code.
 */

#define	ERR_LEN	256	/* should be plenty */

char err_buf[ERR_LEN];


/* in order to provide the user with a facility to give the
 * verbal description of an error code, here is the list of
 * error messages.
 */
 
static char *(err_msg[]) = {
	"No error",
	"Cannot read config file",
	"Link adaptor name incorrect",
	"Cannot find (stat) config file",
	"Cannot get group-id for vmebus/SBus group",
	"The config file is not member of the vmebus/SBus group",
	"The config file must not have write permission for other",
	"Cannot open config file for reading",
	"Syntax error in config file",
	"Internal: Pass 1 not at EOF of config file",
	"Internal: Pass 1 cannot rewind config file",		      /* 10 */
	"Cannot allocate memory for LinkInfo array",
	"Board number defined multiple in config file",
	"Error closing the config file",
	"The board-type name is not correct",
	"The board-number is out of range",
	"The link-number on this board is out of range",
	"Syntax error parsing the link-name",
	"Link-name 3 params: The second is not a board-number",
	"Link-name 3 params: The third is not a link-number",
	"Failed to unlock a link, which was locked by myself",	      /* 20 */
	"This link adaptor does not exist",
	"Failed to open VME-Bus device",
	"Failed to mmap(2) page to VME-Bus device",
	"Failed to munmap(2) page from VME-Bus device",
	"The LinkId is not in the valid range",
	"The LinkId has not been opened",
	"Internal: Unknown board type (Read/WriteLink)",
	"Internal: Alive(): VME device not yet opened",
	"Internal: Alive(): Cannot lseek(2) on VME device",
	"The link adaptor is not present on VME-Bus",		      /* 30 */
	"Internal: lock_init(): Cannot get unique sem_key",
	"Cannot lock the semaphore",
	"Cannot unlock the semaphore",
	"Failed to initialize the set of semaphores",
	"Failed to get existing sem_key",
	"Error initializing the semaphore mechanism",
	"Access to link hardware failed",
	"Failed to allocate memory for InfoLink",
	"Cannot read semaphore value",
	"Cannot restore effective group-id to real gid",	      /* 40 */
	"Board is no valid SBus device",
	"Failed to open SBus device",
	"Internal: Unknown bus type",
	"Internal: Alive(): SBus device not yet opened",
	"Internal: Alive(): Cannot lseek(2) on SBus",
	"The Link is not accessable on SBus",
	"Error reading data from SBus link",
	"Error writing data to SBus link",
	"Serious: Error reseting SBus link",
	"Cannot allocate memory for SBusSlot array",		      /* 50 */
	"The board number is not in the valid range",
	"Serious: Error getting write permission from SBus device",
	"The Link does not support packet protocol",
	"Illegal PacketSize (negative, zero or too large)",
	"Internal: Firmware is hanging",
	"Illegal PacketSize does not support double word transfers",
	"Another Link is still used as PacketLink",
	"No open PacketLink",
	"This Link is PacketLink, no stream mode operations allowed",
	"Illegal packet body allignment",			     /* 60 */
	"Illegal buffer allignment",
	"Illegal packet size (greater than defined in PacketMode())",
	"ReadLink not supported in actual BBK-S4 packet protocol version",
	"Timeout getting firmware version number",
	"Firmware has a wrong version number",
	"This Link is currently in use"
};

	

/*
 *	OpenLink
 *
 *	Open the link associated with 'Name'.
 *	If 'Name' is NULL or "" then any free link can be used.
 *	Returns any positive (including 0) integer as a LinkId or
 *	a negative value if the open fails.
 *	The syntax of 'Name' is:
 *
 *	[<board-type> [<board-number>] [<link-adaptor-number>] ]
 *
 *	<board-type>  "BBKV1"|"BBKV2"|"VMTM"|"MTMSUN1"|"MTMSUN2"|
 *		      "BBKS4"	|   <0..MAXSHORT>
 *	<board-number>	B<0..MAXSHORT>
 *	<link-adaptor-number>	L<0..8>
 *
 *	Examples: "VMTM B293 L3" or "BBKV1 B2" or "12" (for HELIOS)
 *
 *	The decimal short value is intended to be used by HELIOS servers,
 *	e.g. 'hydra'. It is the LinkId.
 *
 *	First a systemwide known configuration-file (trans_config)
 *	is read into local memory. This file contains information
 *	about the number, type and VME-bus address / SBus slot number
 *	of transputer
 *	moduls, which are connected to this maschine.
 *	Each module is assigned a unique so called board-number
 *	to identify this particular board.
 *
 *	The configuration file is formated as follows:
 *
 *	<board-number>	<board-type>	<vme-base-address/sbus slot number>
 *
 *	where <board-number> and <board-type> are defined as above
 *	and
 *
 *	<vme-base-address>	C style hexadecimal number "%x" format
 *				and gives the selected base address
 *				for VMTM e.g. "80200000"
 *	or
 *
 *	<sbus slot number>	C style hexadecimal number "%x" format
 *				and gives the number of the SBus slot 
 *				used by this SBus device board
 *
 *	Each link will be locked after beeing opened by a system
 *	semaphore. So each link MUST be freed by calling 'close()'
 *	after using it. Otherwise nobody else can use this link.
 *	The LinkId is interpreted as array-index in a local array
 *	called LinkInfo.
 *
 *	Error codes:
 *
 *	ECONFIG		Cannot read correctly the configuration file
 *			File open/read fail or syntax error
 *	ELNAME		The Link-adaptor name is wrong
 *		All error codes returned by "search_link()"
 *
 *	Acces to global variables:
 *
 *	MaxLinkId	R	Check wether LinkInfo exists
 */


int OpenLink (Name)
char * Name;
{	int 	lerr;			/* local error value */
	int	linkid;			/* link id */
	int	link_in_use;		/* at least one link in use */
	struct 	LinkDesc l_desc;
	struct 	LinkInfo *l_info;	/* for link search */
	

	/* functions called from this module */
	int InitLink ();
	int scan_link_name ();
	int get_link ();
	struct LinkInfo *search_link ();

	if ((lerr=InitLink ()) != OK) {
		/* failed to initialize link interface */
		return (lerr);
	}
	 
	/* convert Name into LinkDescriptor structure */
	if ((lerr=scan_link_name (Name, &l_desc)) < 0) {
		/* error converting name of Linkadaptor */
		return (lerr);
	}

	/* check wether link description was single number.
	 * In this case try to get this and only this link.
	 * Otherwise check all links for the description
	 */
	if ( l_desc.b_type == LINK ) {
		/* try to get this and only this link */
		l_info = &LinkInfo[l_desc.b_no];
		switch (l_info->Bus_type) {
		   case	SBUS:
			if ((linkid = get_link (l_info)) < 0) {
				return (linkid);
			} else {
				return (fill_flags (linkid));
			}
			break;
		   default:
			return ( get_link (l_info) );
			break;
		}		
	}

	/* otherwise search for link described and get it */
	l_info = NULL;
	link_in_use = 0;
	while ( l_info = search_link (l_info, &l_desc) ) {
		/* try to get the link 'l_info' */
		lerr = get_link (l_info);
		if ( (lerr >= 0) || (lerr != EINUSE) ) {
			/* link has been successfully locked or
			 * there was a serious error
			 */
			switch (l_info->Bus_type) {
			   case	SBUS:
				if (lerr >= 0) {
					lerr = fill_flags (lerr);
				}
				break;
		     	   default:
				break;
			}	
			return (lerr);
		}
		/* 'get_link()' has returned EINUSE
		 * keep on searching for an appropriate link
		 */
		link_in_use++;
	} /* while search_link */
	/* failed to open link */
	return (link_in_use ? EINUSE : ENONEXIST);
}


/*
 *	InitLink
 *
 *	initialise the complete link interface. This function is
 *	intended to be used internally (Version 1.02)
 *	Initialisation consists of three independant parts.
 *	Each part is guarded by an according variable, to
 *	prevent multiple initialisations.
 *	The first part is the configuration file and
 *	the array of structure 'LinkInfo'.
 *	The second part is the locking mechanism with
 *	semaphores.
 *	The third part is the timeout scaling.
 *	Since V1.30 the SBus polling loops are different from the
 *	VME loops which are identical with respect to each other.
 *	Not so nice but works (in time): empiric scale factor 'STIME_SCALE'
 *	defined, depending on IPX defined or not.
 *
 *	Global variables accessed
 *
 *	MaxLinkId	Read/Write
 *	LinkInfo	Write
 *	sem_id		Read/Write
 *	time_tenth	Read/Write
 *	time_max	Read/Write
 */
 
static
int InitLink ()
{	int lerr;
	char *conf_file;		/* name of config file */

	/* functions called from this module */
	char *getenv ();
	int init_config ();
	int init_lock ();
	int init_timeout ();

	if (MaxLinkId < 0) {
		/* read in configuration file */
		/* first: check wether environment variable exists
		 * which contains the config-filename
		 */
		conf_file = getenv (env_config);
		if ( conf_file == NULL ) {
		    /* failed to get config_file name
		     * from the environment.
		     * Use default config-file name
		     */
		    conf_file = config_file;
		}
		lerr=init_config (conf_file);
		if (lerr < 0) {
			/* error reading configuration file */
			MaxLinkId = -1;	/* record error */
			return (lerr);
		}
	}

	/* determine wether lock-mechanism is already initialised */
	if ( sem_id < 0 ) {
		/* initialize lock mechanism */
		lerr = init_lock (MaxLinkId); 
		if ( !((lerr == OK) || (lerr <= WARNING)) ) {
			/* failed to initialize lock mechnism */
			return (lerr);
		}
	}

	/* this interface may be run on a wide class of machines.
	 * Especially the CPU performance varies in a wide range.
	 * The fastest way for timeout-checking is downcounting
	 * a local variable. The timeout values are specified
	 * machine-independant in 0.1 seconds.
	 * So there is a machine-specific value which is downcounted
	 * in 0.1 seconds. This value ('time_tenth') is computed
	 * in 'init_timeout' using the 'setitimer(2)'.
	 * Take care to initialise the timeout value only once.
	 * Added with 1.07: To prevent overflow during the usage
	 * of the calculated value, compute the maximum timeout-period
	 * in tenth of seconds and store it in global 'time_max'.
	 */
	if ( time_tenth < 0 ) {
		/* not yet initialised
		 * set default time_tenth and time_max values
		 */
		time_tenth = default_tenth;
		time_max   = (MAXINT / time_tenth) - 1;

		lerr = init_timeout (&time_tenth, &time_max); 
		if ( !((lerr == OK) || (lerr <= WARNING)) ) {
			/* failed to initialize timeout value */
			return (lerr);
		}
	}
	
	return (OK);
}	



/*
 *	init_config
 *
 *	open the transputer configuration file and check owner-ship
 *	and access rights. The configuration file contains
 *	information which MUST not be modified by usual users.
 *	This is the number, type and VME-address/SBus slot number
 *	of all the transputer modules connected to this maschine. 
 *	So the ownership and access rights are checked to be sure
 *	that the correct configuration file is accessed.
 *	Then read the complete file and count the number of
 *	links described.
 *	Allocate memory for the LinkInfo array and set MaxLinkId.
 *	Rewind the configuration file and initialize parts of
 *	the LinkInfo structure for every link encountered.
 *	The result of the owner-ship check determines the supported
 *	bus type. 
 *
 *	VME-bus:Open the character special VME-bus device and set
 *		the global variable 'vme_device' with the file
 *		descriptor.
 *	SBus:	Open each SBus device and set 'SBusSlot[].device'
 *		for each SBus device with the file descriptor.
 *
 *	Save both effective and real gid in global variables
 *	and set effective gid back to real gid (security).
 *
 *	Global variables
 *	struct LinkInfo  *LinkInfo;
 *	int		  MaxLinkId;
 *	int		  vme_device;
 *	int		  SBusSlot[];
 *	int		  vme_gid;
 *	int		  usr_gid;
 *
 */

static
int init_config (Name)
char *Name;
{	struct stat stat_buf;	/* buf to 'stat(2)' config file */
	struct group *grp;	/* group info for VMEBUS/SBus */
	struct LinkInfo *l_inf, /* ptr to next LinkInfo to be filled */
			*t_inf; /* search for multiple board no. */
	struct board_descr *b_desc;
				/* board_descr for search_board_name */
	struct sbus_descr  *s_desc;
				/* sbus_descr for search_sbus_name */
	FILE *config;		/* configuration file */
	char line[MAXLINE];	/* line buffer config file */
	char b_type[MAXLINE];	/* board type as string */
	char *dev_name;		/* SBus device driver name */
	char devicename[100];	/* SBus device driver name (tmp) */
	int  b_no;		/* current board number */
	unsigned long vme_addr;	/* current vme/sbus base address */
	int links=0;		/* number of links detected */
	int slot=-1;		/* SBus slot number */
	short bus_type;		/* bus_type */
	short vme=0;		/* VMEBUS flag */
	int n;
	short l;

	/* function references */
	struct board_descr *search_board_name();
	struct sbus_descr  *search_sbus_name();



	/* Check wether LinkInfo already exists */
	if (MaxLinkId > 0)
		return (WEXISTS);
	
	/* store the real and effective gid for further use */
	vme_gid = getegid ();
	usr_gid = getgid ();

	/* restore effective group-id to real group-id */
	if ( setregid ( -1, usr_gid) ) {
		/* failed to restore real gid */
		return (ERGID);
	}

	/* check owner and permission of configuration file */
	if ( stat (Name, &stat_buf) ) {
		/* cannot 'stat(2)' configuration file */
		return (ESTAT);
	}
		/* in order to free configure names of the vmebus group
		 * get the group name from an external string.
		 */
	if ((grp=getgrnam (vme_group_name)) == NULL) {
		/* cannot find gid for vmebus group */
			return (EGETGID);
	}

	endgrent ();	/* close group file */


	if (stat_buf.st_gid != (short)grp->gr_gid) {
		/* wrong group membership of config file */
		return (EGID);
	}

		/* check permission: no write perm. for other */	
	if (stat_buf.st_mode & (u_short) S_IWOTH) {
		/* wrong permission of config file */
		return (EACCESS);
	}

	/* open configuration file for reading */
	if ((config=fopen (Name, "r")) == NULL) {
		/* failed to open configuration file */
		return (EOPENR);
	}

	/* first pass: Count number of links */

	while ( fgets (line, sizeof(line), config)) {
		/* next line in 'line' */
		if ((line[0] == '#') || (line[0] == '\n'))
			/* comment line: skip */
			continue;
		n = sscanf (line, "%d %s %x", &b_no, b_type, &vme_addr);
		if ( !((n == 0) || (n == 3)) ) {
			/* syntax error */
			return (ESYNTAX);
		}

		/* check board type and add its number of links
		 * to the total link count 'links'.
		 */
		if ( (b_desc = search_board_name (b_type)) == NULL) {
			/* board-type not recognized */
			return (EBNAME);
		} else {
			/* increment total links by number of links
			 * on this board. The links per board is 
			 * recorded in the board_descr structure.
			 */
			links += b_desc->links;
		}
	} /* while fgets  pass 1 */



	/* set total number of available links */
	MaxLinkId = links;
	/* check EOF status */
	if ( !feof (config)) {
		/* pass 1 not at EOF */
		return (E1NOEOF);
	}
	/* rewind configuration file */
	if ( fseek (config, 0L, 0)) {
		/* cannot rewind configuration file */
		return (EREWIND);
	}

	/* allocate memory for LinkInfo array sized MaxLinkId */
	LinkInfo = (struct LinkInfo *) malloc (
			MaxLinkId * sizeof (struct LinkInfo));
	if ( LinkInfo == NULL ) {
		/* failed to allocate memory for LinkInfo array */
		return (EMEM);
	}

	/* allocate memory for FlagAdr array sized MaxLinkId */
	FlagAdr = (struct FlagAdr *) malloc (
			MaxLinkId * sizeof (struct FlagAdr));
	if ( FlagAdr == NULL ) {
		/* failed to allocate memory for FlagAdr array */
		return (EMEM);
	}

	/* allocate memory for SBusSlot array sized MAX_SBUS_SLOTS */
	SBusSlot = (struct SBusSlot *) malloc (
			MAX_SBUS_SLOTS * sizeof (struct SBusSlot));
	if ( SBusSlot == NULL ) {
		/* failed to allocate memory for SBusSlot array */
		return (ESBMEM);
	}


	/* second pass: fill in LinkInfo structure components:
	 *	B_type		int board type
	 *	B_no		board number
	 *	L_no		link number on this board
	 */
	l_inf = LinkInfo;
	while ( fgets (line, sizeof(line), config)) {
		/* next line in 'line' */
		if ((line[0] == '#') || (line[0] == '\n'))
			/* comment line: skip */
			continue;
		n = sscanf (line, "%d %s %x", &b_no, b_type, &vme_addr);
		if ( !((n == 0) || (n == 3)) ) {
			/* syntax error */
			return (ESYNTAX);
		}
		/* check b_no for already existing
		 * value anywhere in LinkInfo array
		 */
		for (t_inf = LinkInfo; t_inf<l_inf; t_inf++) {
			if (t_inf->B_no == b_no) {
				/* board number assigned multiple */
				sprintf (err_buf,
				   "Board number %d assigned twice (index %d)",
				   b_no, t_inf-LinkInfo);
				return (EBNO);
			}
		} /* for all existing links */

		/* check board type and fill in information
		 */
		
		if ( (b_desc = search_board_name (b_type)) == NULL) {
			/* board-type not recognized */
			return (EBNAME);
		} else {
			if (b_desc->type == BBKS4) {
				bus_type = SBUS;
				/* valid SBus device ? */
				if ((s_desc = search_sbus_name(b_type))
								== NULL) {
					/* unknown SBus system */
					return (ESBUSNAME);
				} else {
					/* interpret vme_addr as SBus
					 * slot number and get the 
					 * slot base address from the
					 * SBus descriptor array
					 */ 
					slot = vme_addr;
					vme_addr = s_desc->slot_base[slot];
					/* open SBus device for board  
					 * in SBus slot 'slot'
					 * +++ Do it with vme gid
					 */
					dev_name = &devicename[0];
					sprintf (dev_name, "%s%d",
						 sbus_dev_name, slot);
					/* back to vme-gid */
					if ( setregid ( -1, vme_gid) != 0 ) {
						/* failed to restore vme-gid */
						return (ERGID);
					}
					if ((SBusSlot[slot].device = 
					     open (dev_name,O_RDWR)) < 0) {
						/* failed to open SBus device */
						/* back to real gid */
						setregid ( -1, usr_gid);
						return (ESBUSDEVICE);
					} else {

					/* back to real gid */
					setregid ( -1, usr_gid);

					/* fill in used address space, slot
					 * base address and dual ported ram
					 * offsets/parameters in SBusSlot
					 * structure
					 */
					SBusSlot[slot].space = s_desc->mem;
					SBusSlot[slot].base  = s_desc->
								slot_base[slot];
					SBusSlot[slot].dpr.channel_off =
						s_desc->dpr.channel_off;
					SBusSlot[slot].dpr.regs_off =
						s_desc->dpr.regs_off;
					SBusSlot[slot].dpr.com_off =
						s_desc->dpr.com_off;
					SBusSlot[slot].dpr.channel_len =
						s_desc->dpr.channel_len;
					SBusSlot[slot].dpr.regs_len =
						s_desc->dpr.regs_len;
					SBusSlot[slot].dpr.int_s_t2_off =
						s_desc->dpr.int_s_t2_off;
					SBusSlot[slot].dpr.int_s_t2_off =
						s_desc->dpr.int_s_t2_off;

					} /* else (SBusSlot...) */
				} /* else (s_desc = ...) */
			} /* if (b_type == BBKS4) */
			else {
				bus_type = VMEBUS;
				vme++;
			}
			/* for each link on the board assign
			 * a new struct LinkInfo and fill in
			 * some usefull information
			 */
			for (l=0; l<b_desc->links; l++) {
				l_inf->opened     = FALSE;
				l_inf->packetlink = FALSE;
				l_inf->usr_ad     = NULL;
				l_inf->B_type     = b_desc->type;
				l_inf->B_no       = b_no;
				l_inf->Bus_type   = bus_type;
				if (bus_type == SBUS) {
					l_inf->B_slot  = slot;
					FlagAdr[l].half_chan_len  = -1;
					FlagAdr[l].read_link_rq0  = NULL;
					FlagAdr[l].read_link_rq1  = NULL;
					FlagAdr[l].blockin_ack    = NULL;
					FlagAdr[l].chan_free      = NULL;
					FlagAdr[l].write_link_rq0 = NULL;
					FlagAdr[l].write_link_rq1 = NULL;
					FlagAdr[l].blockout_ack   = NULL;
					FlagAdr[l].event_ack      = NULL;
					FlagAdr[l].init_event     = NULL;
					FlagAdr[l].test_read      = NULL;
					FlagAdr[l].test_write     = NULL;
					FlagAdr[l].read_timeout   = NULL;
					FlagAdr[l].write_timeout  = NULL;
				} else {
					l_inf->B_slot  = -1;
				}
				l_inf->vme_addr = (caddr_t)
						  (vme_addr + b_desc->size[l]);

				l_inf->L_no     = l;

				l_inf++;	/* next LinkInfo */
			} /* for each link on this board */
		} /* else b_desc ... */
	} /* while fgets  pass 2 */

	if (vme > 0) {
		/* open the vme-bus device */
		/* using vme-gid */
		if ( setregid ( -1, vme_gid) != 0 ) {
			/* failed to restore vme-gid */
			return (ERGID);
		}
		if ( (vme_device = open (vme_dev_name, O_RDWR)) < 0 ) {
			/* failed to open VME-bus device */
			/* back to real gid */
			setregid ( -1, usr_gid);
			return (EVMEDEVICE);
		}
		/* back to real gid */
		setregid ( -1, usr_gid);
	}

	/* close config file */
	if ( fclose (config) ) {
		/* error closing configuration file
		 * should not happen: Open for read-only */
		return (ECLOSE);
	} /* if fclose ... */

	/* everything went well */
	return (OK);
}


/*
 *	search_board_name
 *
 *	scan the board descriptor array for the given name string.
 *	If found return pointer to this entry, else return NULL.
 *
 *	Global reference
 *
 *	struct board_descr b_names[];
 */

static
struct board_descr *search_board_name (name)
register char *name;
{	register struct board_descr *b_desc;
	
	/* linear search through b_names array.
	 * search for board name as string.
	 * board_descriptor list is terminated by NULL name.
	 */
	for (b_desc = b_names; b_desc->name; b_desc++) {
		if ( strcmp (b_desc->name, name) == 0) {
			/* matched name found */
			return (b_desc);
		}
	}
	/* 'name' not found */
	return (NULL);
}



/*
 *	TestFirmwareCode (LinkId)
 * 	Request to BBK-S4 firmware sending the firmware version number
 *	Additional alive test in OpenLink (fill_flags)
 */

int TestFirmwareCode (LinkId)
int LinkId;
{
	struct	DpR	    *dpr;	
	struct	slink_s	    *comm;
	struct	LinkInfo    *l_info;

	int     timecnt;	


	/* range check LinkId */
	if ( (LinkId < 0) || (LinkId >= MaxLinkId) ) {
		/* LinkId out of range */
		return (EIDINVAL);
	}

	timecnt = 10 * time_tenth;
	l_info  = &(LinkInfo[LinkId]);
	dpr     = (struct DpR *)&(SBusSlot[l_info->B_slot].dpr);

	/* is that link active */
	if ( LinkInfo[LinkId].opened == FALSE ) {
		/* link not active */
		return (ENOTOPEN);
	}

	comm = (struct slink_s *)((l_info->usr_ad) + dpr->com_off);


	/* set event type */
	comm->stopreset[l_info->L_no] = (word)(VERSION);
	/* set event flag */
	comm->startreset[l_info->L_no] = (word)(VERSION);

	while ( (comm->startreset[l_info->L_no]) != 0)  {
		if ( timecnt-- <= 0 ) {
			/* operation timed out */
			return (ERNOVERSION);
		} /* if timecnt-- */

#ifdef	IPX
		getpid () ;
#endif	IPX
		
	}

	return ((int) (comm->stopreset[l_info->L_no]) );
}





/*
 *	search_sbus_name
 *
 *	scan the sbus descriptor array for the given name string.
 *	If found return pointer to this entry, else return NULL.
 *
 *	Global reference
 *
 *	struct sbus_descr s_names[];
 */

static
struct sbus_descr *
search_sbus_name (name)
register char *name;
{	register struct sbus_descr *s_desc;
	
	/* linear search through s_names array.
	 * search for board name as string.
	 * sbus_descriptor list is terminated by NULL name.
	 */
	for (s_desc = s_names; s_desc->name; s_desc++) {
		if ( strcmp (s_desc->name, name) == 0) {
			/* matched name found */
			return (s_desc);
		}
	}
	/* 'name' not found */
	return (NULL);
}



/*
 *	scan_link_name
 *
 *	scan the given string for a valid link name. Handle
 *	all variations of ambigouty. Put the result into all
 *	components of a struct LinkDesc, which is passed as
 *	an argument. Return an error code (<0) on failure, else OK.
 *
 *	Note:
 *
 *	The given structure is written in ANY case. The default
 *	values are ANY for every component, which
 *	means 'unspecified'.
 *
 *	The syntax of 'Name' is:
 *
 *	[<board-type> [<board-number>] [<link-adaptor-number>] ]
 *
 *	<board-type>  "BBKV1"|"BBKV2"|"VMTM"|"MTMSUN1"|"MTMSUN2"|
 *		      "BBKS4"  |  <0..MAXSHORT>
 *	<board-number>	B<0..MAXSHORT>
 *	<link-adaptor-number>	L<0..8>
 *
 *	Examples: "VMTM B293 L3" or "BBKV1 B2" or "12" (for HELIOS)
 *
 *	For V1.01 added: board-type may be decimal non-negative number.
 *	In this case the 'LinkDesc' structure contains:
 *		b_type		LINK	(Dummy name for identification)
 *		b_no		<the given number>
 *		l_no		ANY
 *
 *      Global variables
 *
 */

static
int scan_link_name (Name, l_desc)
char  	*Name;
struct LinkDesc	*l_desc;	/* ouput */
{	char b_name   [MAXLINE],	/* board type as string   */
	     b_num_str[MAXLINE],	/* board number as string */
	     l_num_str[MAXLINE];	/* link number as string  */
	int n;
	struct board_descr *b_desc;	/* board descriptor */
	short board_no;			/* board number as short */
	short link_no;			/* link number as short */


	/* === references to other functions === */
	struct board_descr *search_board_name();

	/* set temp strings to empty strings */
	b_name   [0] = '\0';
	b_num_str[0] = '\0';
	l_num_str[0] = '\0';

	/* first of all: initialise *l_desc with all members set
	 * to ANY, which means 'unspecified'.
	 */
	l_desc->b_type = ANY;	/* any board type    */
	l_desc->b_no   = ANY;	/* any board number  */
	l_desc->l_no   = ANY;	/* any link on board */


	/* most common case: empty string or NULL */
	if ( (Name == NULL) || (strlen (Name) == 0) ) {
		/* any link */
		return (OK);
	}

	n = sscanf (Name, "%s %s %s", b_name, b_num_str, l_num_str);
	if ( n == 0 ) {
		/* Name consists of white space only ==> any link */
		return (OK);
	}

	/* check wether the first parameter is pure decimal numeric.
	 * special processing in this case.
	 */
	if ( (n == 1) && (sscanf (b_name, "%hd", &board_no) == 1) ) {
		/* special case: Numeric specification of link only.
		 * record this by placing the dummy name 'LINK' for
		 * the board-type and the encoded number into 'b_no'.
		 */
		if ( (board_no < 0) || (board_no >= MaxLinkId) ) {
			/* link number out of range */
			return (EBNUM);
		}
		l_desc->b_type = LINK;
		l_desc->b_no   = board_no;
		return (OK);
	}
		
	/* I cannot be sure wether the strings are assigned correctly.
	 * If two parameters are given, nobody knows which one was missing.
	 */
	/* the first parameter supplied must be the board type */
	if ( (b_desc=search_board_name (b_name)) == NULL) {
		/* board name unknown */
		return (EBNAME);
	}
	/* board is found. Take coded board type from board descr. */
	l_desc->b_type = b_desc->type;

	if ( n == 1) {
		/* the first parameter have been processed ok */
		return (OK);
	} /* if one parameter */

	if ( n == 2 ) {
		/* if two parameters are given, the first must be the
		 * board type. The second may be either board-number or
		 * link-number.
		 */
		/* is it a board number ?
		 * Must start with 'B' or 'b' followed by decimal
		 * number. Use 'scanf()' to check.
		 */
		if ( sscanf (b_num_str, "%*[Bb]%hd", &board_no) == 1 ) {
			/* board number detected */
			if ( board_no < 0) {
				/* negative board number not allowed */
				return (EBNUM);
			}
			l_desc->b_no = board_no;
			return (OK);
		} else 
		/* or is it a link number ? */
		if ( sscanf (b_num_str, "%*[Ll]%hd", &link_no) == 1 ) { 
			/* link number detected */
			if ( (link_no < 0) ||
			     (link_no >= MAX_BOARD_LINKS) ) {
				/* link number out of range */ 
				return (ELNUM);
			}
			l_desc->l_no = link_no;
			return (OK);
		}
		/* 2 parameters error: neither board- nor link-number */
		return (ENAMSYNTAX);
	} else {
		/* three parameters are given */
		/* is second parameter a board number ?
		 * Must start with 'B' or 'b' followed by decimal
		 * number. Use 'scanf()' to check.
		 */
		if ( sscanf (b_num_str, "%*[Bb]%hd", &board_no) == 1 ) {
			/* board number detected */
			if ( board_no < 0) {
				/* negative board number not allowed */
				return (EBNUM);
			}
			l_desc->b_no = board_no;
		} else {
			/* the second from three parameters was not
			 * recognized as the board number
			 */
			return (EBOARD3PAR);
		}
		/* the third of three parameters must be recognized
		 * as the link number
		 */
		if ( sscanf (l_num_str, "%*[Ll]%hd", &link_no) == 1 ) { 
			/* link number detected */
			if ( (link_no < 0) ||
			     (link_no >= MAX_BOARD_LINKS) ) {
				/* link number out of range */ 
				return (ELNUM);
			}
			l_desc->l_no = link_no;
			return (OK);
		} else {
			/* the third from three parameters was not
			 * recognized as the link number
			 */
			return (ELINK3PAR);
		}
	} /* else 3 parameters */
	/* NOTREACHED */
}

/*
 *	search_link
 *
 *	the supplied parameter is a pointer to a 'LinkDesc' struct.
 *	This routine starts searching at the position described as follows:
 *	If a NULL value is given, the search will start with the first
 *	LinkInfo element, otherwise with the given value plus one.
 *	The search proceeds until an appropriate link is found or
 *	the end of the LinkInfo array is reached.
 *	Returns an pointer to the found LinkInfo entry or NULL on failure.
 *
 *	Hint
 *	The caller should call 'search_link' with a NULL arg for
 *	'l_info' for the first time. When a value != NULL is received,
 *	he/she should try to lock this link and check it with 'alive'.
 *	If either failed he/she may call 'search_link' again with the
 *	old returned value to proceed with checking.
 *	Incrementing is done here for the users convenience.
 *	The search has failed finally on return of NULL from here.
 *
 *	Global variables
 *
 *	struct LinkInfo *LinkInfo;
 *	int		 MaxLinkId;	/* number of LinkInfo elements * /
 */

static
struct LinkInfo *
search_link (l_info, l_desc)
register struct LinkInfo *l_info;	/* start of search */
register struct LinkDesc *l_desc;	/* describes the link searched for */
{	struct LinkInfo *l_end;	/* points to last+1 LinkInfo elem. */

	/* referenced functions */

	/* where should we start from?
	 * If (l_info == NULL)
	 *	LinkInfo[0];
	 * else
	 *	l_info++;
	 */
	if ( l_info == NULL ) {
		/* set start to begin of LinkInfo[] */
		if ( MaxLinkId < 0 ) {
			/* LinkInfo not yet initialized */
			return (NULL);
		}
		l_info = LinkInfo;
	} else {
		/* advance start of search to next item.
		 * We receive our last match position and
		 * increment it here.
		 */
		l_info++;
	}

	/* check wether l_info points into LinkInfo[] */
	if ( l_info < LinkInfo ) {
		/* illegal value for l_info */
		return (NULL);
	}

	l_end = &LinkInfo[MaxLinkId];	/* --l_end is last valid */
	/* search every link */
	for (;l_info < l_end; l_info++) {
		/* is link already open ? */
		if ( l_info->opened ) {
			/* already open. search next */
			continue;
		}
		/* does the board type match ? */
		if ( (l_desc->b_type == ANY) ||
		     (l_desc->b_type == l_info->B_type) ) {
			/* board type matches.
			 * check board number
			 */
			if ( (l_desc->b_no == ANY) ||
			     (l_desc->b_no == l_info->B_no) ) {
				/* board-type and -number match.
				 * check link number
				 */
				if ( (l_desc->l_no == ANY) ||
				     (l_desc->l_no == l_info->L_no) ) {
					/*
					 * everything has matched
					 * return this link
					 */
					return (l_info);
				} /* if link number match */
			} /* if board number match */
		} /* if board type match */
	} /* for */
	/* no appropriate link found */
	return (NULL);
}


/*
 *	get_link
 *
 *	try to get the link specified by 'l_info' for exclusive use by
 *	the caller. This means the link will be locked using a system wide
 *	semaphore through 'lock_link'. Furthermore 'alive' will check the
 *	link for accessibility of the hardware.
 *	On success the non negative LinkId is returned, otherwise
 *	a negative error number.
 */

static
int get_link (l_info)
struct LinkInfo *l_info;
{	int	ret_val;
	/* functions called from this module */
	int lock_link ();
	int alive ();
	int link_mmap ();
	int unlock_link ();

	/* try to lock link 'l_info' */
	if ( lock_link (l_info-LinkInfo) == OK ) {
		/* is board accessable ? */
		if ( (ret_val = alive (l_info)) == OK ) {
			/* establish 'mmap(2)' page */
			if ( (ret_val = link_mmap (l_info)) == OK ) {
				/* everything is fine
				 * the link is locked, checked and
				 * mmaped. Mark it as 'opened'.
				 */
				l_info->opened = TRUE;
				return (l_info-LinkInfo);
			} else { /* if link_mmap */
				/* locked & alive but mmap failed */
				if ( unlock_link (l_info-LinkInfo) != OK ) {
					/* should never happen
					 * we have locked this link
					 * and cannot unlock it
					 */
					return (ELOCKCLASH);
				}
				/* unlocked, report mmap error */
				return (ret_val);
			} /* else link_mmap */
		} else {
			/* link locked but not alive
			 * unlock it
			 */
			if ( unlock_link (l_info-LinkInfo) != OK ) {
				/* should never happen
				 * we have locked this link
				 * and cannot unlock it
				 */
				return (ELOCKCLASH);
			}
			/* link hardware not accessable
			 * the previous lock has been released.
			 */
			return (ENOTALIVE);
		} /* else alive */
	} /* if lock_link */
	/* cannot lock link
	 * keep on searching
	 */
	return (EINUSE);
}


/*
 *	fill_flags
 *
 *	Fills FlagAdr structure of the specified link. The FlagAdr structure
 *	is only used by the BBK-S4 routines ReadLink and WriteLink.
 *
 */

static
int fill_flags (LinkId)
int LinkId;
{
	struct	 LinkInfo    *l_info;	/* ptr to LinkInfo[LinkId] */
	struct	 DpR	     *dpr;	/* ptr to SBusSlot[...] */
	struct	 BBKS4_regs  *regs;	/* read/write register */
	struct	 slink_s     *l_ev;	/* communication buffer T222 <-> SBus */
	int	    	     ret_val;

	/* range check LinkId */
	if ( (LinkId < 0) || (LinkId >= MaxLinkId) ) {
		/* LinkId out of range */
		return (EIDINVAL);
	}

	l_info = &(LinkInfo[LinkId]);

	dpr = (struct DpR *)&(SBusSlot[l_info->B_slot].dpr);
	l_ev = (struct slink_s *)(l_info->usr_ad 
		+ SBusSlot[l_info->B_slot].dpr.com_off);

	/* set the regs struct to the address of the read/write
	 * register set
	 */
	regs = (struct BBKS4_regs *)((l_info->usr_ad) + dpr->regs_off
			+ l_info->L_no * dpr->regs_len);

	FlagAdr[LinkId].half_chan_len  = (dpr->channel_len / 2);
	FlagAdr[LinkId].read_link_rq0  = (word *)&(regs->readlinkrq[0]);
	FlagAdr[LinkId].read_link_rq1  = (word *)&(regs->readlinkrq[1]);
	FlagAdr[LinkId].blockin_ack    = (word *)&(regs->blockinack);
	FlagAdr[LinkId].write_link_rq0 = (word *)&(regs->writelinkrq[0]);
	FlagAdr[LinkId].write_link_rq1 = (word *)&(regs->writelinkrq[1]);
	FlagAdr[LinkId].blockout_ack   = (word *)&(regs->blockoutack);
	FlagAdr[LinkId].chan_free      = (word *)&(regs->channelfree);
	FlagAdr[LinkId].event_ack      = (word *)&(l_ev->event_ack_sbus);
	FlagAdr[LinkId].init_event     = (word *)(l_info->usr_ad +
			 SBusSlot[l_info->B_slot].dpr.int_s_t2_off);
	FlagAdr[LinkId].test_read      = (word *)&(regs->testread);
	FlagAdr[LinkId].test_write     = (word *)&(regs->testwrite);
	FlagAdr[LinkId].read_timeout   = (word *)&(regs->readtimeout);
	FlagAdr[LinkId].write_timeout  = (word *)&(regs->writetimeout);

	if ((ret_val = TestFirmwareCode (LinkId)) < 0) {
	   CloseLink (LinkId);
	   return (ret_val);
	} else {	
	   if (ret_val != VERS_NO) {
		return(ERWRONGVERS);
	   }
	   return (LinkId);
	}
}



/*
 *	alive
 *
 *	check wether the specified link is alive. 
 *
 *	VMEBUS:	This is done by 'lseek(2)' to desired address on 
 *		VME-bus device and then try to read a single byte.
 *		It is assumed, that the VME-bus device is already 
 *		open.
 *
 *	SBUS:	This is done by 'lseek(2)' to desired address on
 *		SBus device and then read the link alive flag in the
 *		dual ported ram communication region.
 *		It is assumed, that the SBus device is already open.
 *		It is not possible to mmap and check inside,
 *		this results in a nice core dump (SIGBUS) if there
 *		is no BBK-S4 installed. mc for V1.30.
 *
 *	For security (SunOs) reasons I do not find documented,
 *	the effective gid to allow VME-bus/SBus access should be
 *	temporary restored. This change is done in V 1.08.
 *
 *	Global variables
 *
 *	int		vme_device;
 *	struct SBusSlot	SBusSlot[];
 *	struct LinkInfo *l_info;
 *	int		vme_gid;
 *
 */

static
int alive (l_info)
struct LinkInfo *l_info;
{	off_t 	addr;		/* address of instatus register */
	int	ret_val;	/* return value */
	byte 	ch;		/* dummy to read in instatus register */
	word	salive;		/* dummy word to read bbk-s4 alive */
	struct	SBusSlot sb_slot;   /* SBus slot */
	struct 	slink_s	 *l_alive;  /* communication buffer T222 <-> SBus */
	struct	DpR	    *dpr;	
	int     timecnt;	
	struct	slink_s	    *comm;

	switch (l_info->Bus_type) {
	   case VMEBUS:
		/* check wether VME device is already opened */
		if ( vme_device < 0 ) {
			/* sorry, VME-bus device not open */
			return (EVMENOTOPEN);
		}

		/* compute the address of the instatus register */
		switch (l_info->B_type) {
		   case BBKV1:
			addr = (off_t)&(((struct slink_1 *)l_info->vme_addr)
				->instatus);
			break;
		   case BBKV2 :
			addr = (off_t)&(((struct slink_2 *)l_info->vme_addr)
				->instatus);
			break;
	  	   case VMTM :
			addr = (off_t)&(((struct slink_v *)l_info->vme_addr)
				->instatus);
			break;
		   case BBKV4 :
			addr = (off_t)&(((struct slink_4 *)l_info->vme_addr)
				->instatus);
			break;
		   case MTMSUN1 :
		   case MTMSUN2 :
			addr = (off_t)&(((struct slink_m *)l_info->vme_addr)
				->instatus);
			break;
		   default :
			return (ENOTYPE);
		}

		/* restore the effective gid to VME-bus access */
		if ( setregid ( -1, vme_gid) != 0 ) {
			/* failed to rstore VME-bus gid */
			return (ERGID);
		}

		/* try to access the instatus register */
		if ( lseek (vme_device, addr, L_SET) == -1 ) {
			/* failed to seek to desired position */
			/* back to real gid */
			setregid ( -1, usr_gid);
			return (EVMESEEK);	/* should never happen */
		}
		if ( read (vme_device, &ch, 1) == -1 ) {
			/* board not accessable */
			/* back to real gid */
			setregid ( -1, usr_gid);
			return (ENOTPRESENT);
		}
		/* this link seems to be present */
		/* back to real gid */
		setregid ( -1, usr_gid);
		return (OK);
	   case SBUS:

		/* alive check for link connected with the SBus device */

		sb_slot = SBusSlot[l_info->B_slot];

		 
		/* check wether SBus device is already opened */
		if ( sb_slot.device < 0 ) {
			/* sorry, SBus device not open */
			return (ESBUSNOTOPEN);
		}
		/* restore the effective gid to SBus access */
		if ( setregid ( -1, vme_gid) != 0 ) {
			/* failed to restore VME-bus/SBus gid */
			return (ERGID);
		}

		/* compute the address of linkalive regs */
		l_alive = (struct slink_s *)(sb_slot.dpr.com_off);
 		addr    = (off_t)&(l_alive->linkalive[l_info->L_no]);

		/* try to access the linkalive register */
		if ( lseek (sb_slot.device, addr, L_SET) == -1 ) {
			/* failed to seek to desired position */
			/* back to real gid */
			setregid ( -1, usr_gid);
			return (ESBUSSEEK);	/* should never happen */
		}
		if ( read (sb_slot.device, &salive, sizeof(salive) ) == -1 ) {
			/* board not accessable */
			/* back to real gid */
			setregid ( -1, usr_gid);
			return (ESBUSNOTALIVE);
		}
		if ( salive != (word)(SBUSMASK | 1) ) {
			/* linkalive does not live any more */
			/* back to real gid */
			setregid ( -1, usr_gid);
			return (ESBUSNOTALIVE);
		}

		/* this link seems to be present */
		/* back to real gid */
		setregid ( -1, usr_gid);
		return (OK);


	   default:
		return (EBUSTYPE);
	}
}



/*
 *	link_mmap
 *
 *	Establishes the mapping between this process virtual address
 *	space and the underlying maschine bus (SBus, VME bus on SUN3,
 *	SUN4). This mapping means, that every memory access into an
 *	specific memory region (sized one page) will access directly 
 *	the VME bus/SBus. So the transputer link adaptors are accessed 
 *	very fast and without a lot of installation overhead, which 
 *	would be neccessary when installing a device driver.
 *	Additional feature with version 1.07:
 *	Try to use the old fashion style SunOs 3.x 'mmap()' handling.
 *	The 'mmap' call allways returns NULL.
 *	Returns OK on success and records the ready to use pointer
 *	to the link hardware inside the 'struct LinkInfo'.
 *	Establishing the mapping for a link of an SBus device not used
 *	before results in the setting of the ready to use pointers for
 *	all links connected to this device.
 *
 *	Global variables
 *	int	vme_device;
 *	int	vme_gid;
 *	int	usr_gid;
 *	struct	LinkInfo  *LinkInfo;
 *	struct	SBusSlot   SBusSlot[];
 *
 */

static
int link_mmap (l_info)
struct LinkInfo *l_info;
{	int page_size;		/* number of bytes in system page */
	int file_descr;		/* file descriptor for VME-bus/SBus device */
	unsigned link_base;	/* tmp ptr start of mmaped area */
	unsigned vme_addr;	/* tmp ptr vme/SBus address of link adaptor */
	struct LinkInfo *l_act;	/* points to LinkInfo element */
	struct LinkInfo *l_end;	/* points to last+1 LinkInfo element */


	/* restore the effective gid to VME-bus/SBus access */
	if ( setregid ( -1, vme_gid) != 0 ) {
		/* failed to restore VME-bus/SBus gid */
		return (ERGID);
	}

	if ((l_info->Bus_type == SBUS) && (SBusSlot[l_info->B_slot].mmap == 1)){
		/* memory mapping yet established for this SBus device */
		return (OK);
	}

	switch (l_info->Bus_type) {
	   case	VMEBUS:
		page_size  = getpagesize ();
		file_descr = vme_device;
		vme_addr   = (unsigned)l_info->vme_addr;
		break;
	   case	SBUS:
		switch (l_info->B_type) {
		   case BBKS4:
			page_size  = SBusSlot[l_info->B_slot].space;
			file_descr = SBusSlot[l_info->B_slot].device;
			vme_addr = (unsigned)0;
			break;
		   default:
			/* board no valid SBus device */
			return (ESBUSNAME);
		}
		break;
	   default:
		/* unknown bus type */
		return (EBUSTYPE);
	}
			

	/* try to establish mapped page to VME-bus/SBus */
	/* this should succeed for SunOs 4.x */
	link_base =  (unsigned) mmap ((caddr_t)0, page_size, 
		(PROT_READ | PROT_WRITE), MAP_SHARED, file_descr,
		(vme_addr & ~(page_size-1) ) );
	/* check wether mapping has failed */
	if (link_base == -1) {
		/* mapping failed using SunOs 4.x style mmap call.
		 * Since V1.07 try wether old style (3.x) will succeed.
	 	 */
		link_base = valloc ( (unsigned) page_size);
		if (link_base == NULL) {
			/* failed to allocate page aligned memory */
	   		/* back to real gid */
  			setregid ( -1, usr_gid);
		   	return (EMMAP);
		}
		if ( mmap ((caddr_t)link_base, page_size,
			(PROT_READ | PROT_WRITE), MAP_SHARED, 
			 vme_device, (vme_addr & ~(page_size-1) ) )
		   == (caddr_t) -1) {
			/* SunOs 3.x style 'mmap' has failed */						/* back to real gid */
			setregid ( -1, usr_gid);
			return (EMMAP);
		}
		/* finally succeeded with old fashioned mmap call */
	}
	/* everything is fine. Adjust link_base by page_offset */
	link_base += (vme_addr & (page_size-1) );

	switch (l_info->Bus_type) {
	   case	VMEBUS:
		/* record ready to use ptr for this link in struct LinkInfo */
		l_info->usr_ad = (caddr_t)link_base;
		break;
	   case SBUS:
		/* record ready to use ptr for all links of this SBus
		 * device in structs LinkInfo
		 */
		l_end = &LinkInfo[MaxLinkId];
		for (l_act = &LinkInfo[0];l_act < l_end; l_act++) {
			if (l_act->B_slot == l_info->B_slot)
				l_act->usr_ad = (caddr_t)link_base;
		}
		/* set mmap flag in this device SBusSlot struct */
		SBusSlot[l_info->B_slot].mmap = 1;
		break;
	   default:
		/* unknown bus type */
		return (EBUSTYPE);
	}
		
	/* back to real gid */
	setregid ( -1, usr_gid);
	return (OK);
}


/*
 *	link_unmap
 *
 *	releases the mapping to VME-bus/SBus established by 'link_mmap'.
 *	Returns OK if everthing was the way it should be, otherwise
 *	an error code (which is allways negative).
 *
 *	Global variables
 *	int	vme_gid;
 *	int	usr_gid;
 *	struct	LinkInfo  *LinkInfo;
 *	struct	SBusSlot   SBusSlot[];
 *
 */

static
int link_unmap (l_info)
struct LinkInfo *l_info;
{	int page_size;		/* number of bytes in system page */
	unsigned link_base;	/* tmp ptr start of mmaped area */
	struct LinkInfo *l_act;	/* points to LinkInfo element */
	struct LinkInfo *l_end;	/* points to last+1 LinkInfo element */

	/* restore the effective gid to VME-bus access */
	if ( setregid ( -1, vme_gid) != 0 ) {
		/* failed to rstore VME-bus gid */
		return (ERGID);
	}

	switch (l_info->Bus_type) {
	   case	(VMEBUS):
		page_size = getpagesize ();
		/* page align the virtual address */
		link_base = (unsigned)(l_info->usr_ad) & ~(page_size-1);
		break;
	   case (SBUS):
		/* read the 'opened' flags (struct LinkInfo) of all links
		 * connected to this SBus device and unmap only if none
		 * of these links is still marked 'open'
		 */
		l_end = &LinkInfo[MaxLinkId];
		for (l_act = &LinkInfo[0];l_act < l_end; l_act++)
			if ((l_act->B_slot == l_info->B_slot) &&
			    (l_act->opened == TRUE))
				/* links of this SBus device still open */
				return (OK);
		switch (l_info->B_type) {
		   case BBKS4:
			page_size  = SBusSlot[l_info->B_slot].space;
			/* page align the virtual address */
			link_base  = (unsigned)(SBusSlot[l_info->B_slot].base)
						& ~(page_size-1);
			break;
		   default:
			/* board no valid SBus device */
			return (ESBUSNAME);
		}
		break;
	   default:
		/* unknown bus type */
		return (EBUSTYPE);
	}
		
	/* unmap a single page (VMEBUS) or all mapped pages (SBus) */
	if ( munmap ( (caddr_t) link_base, page_size) ) {
		/* failed to unmap page */
		return (EUNMAP);
	}

	switch (l_info->Bus_type) {
	   case VMEBUS:
		/* unmapped ok, zero ptr in usr space to hardware */
		l_info->usr_ad = NULL;
		break;
	   case SBUS:
		/* unmapped ok, zero ptr in usr space for all links of 	
		 * this SBus device in structs LinkInfo	 */
		l_end = &LinkInfo[MaxLinkId];
		for (l_act = &LinkInfo[0];l_act < l_end; l_act++)
			if (l_act->B_slot == l_info->B_slot)
				l_act->usr_ad = NULL;
		/* unset mmap flag in this device SBusSlot struct */
		SBusSlot[l_info->B_slot].mmap = 0;
		break;
	   default:
		/* unknown bus type */
		return (EBUSTYPE);
	}

	/* back to real gid */
	setregid ( -1, usr_gid);
	return (OK);
}



/*
 *	CloseLink
 *
 *	Close the active link 'LinkId'. Returns 1 on success or a negative
 *	error code on failure.
 *	It is an error to close a link which is not in open-state. The
 *	link is unlocked and unmapped.
 *
 *	Global variable
 *	struct LinkInfo[LinkId];
 *	int MaxLinkId;
 *
 */

int CloseLink (LinkId)
int LinkId;
{	struct LinkInfo *l_info;	/* ptr to LinkId LinkInfo struct */
	int lerr;			/* local error value */

	/* range check LinkId */
	if ( (LinkId < 0) || (LinkId >= MaxLinkId) ) {
		/* LinkId out of range */
		return (EIDINVAL);
	}

	/* is that link active */
	if ( LinkInfo[LinkId].opened == FALSE ) {
		/* link not active */
		return (ENOTOPEN);
	}

	l_info = &LinkInfo[LinkId];

	/* release mapping */
	if ( link_unmap (l_info) != OK ) {
		/* failed to unmap link */
		return (EUNMAP);
	}


	/* unlock this link */
	if ( (lerr = unlock_link (l_info-LinkInfo)) != OK ) {
		/* failed to unlock this link */
		return (lerr);
	}

	/* finally mark link as free */
	l_info->opened = FALSE;
	return (OK);
}


/*
 *	ReadLink
 *
 *	Read 'Count' bytes into 'Buffer' from the specified link.
 *	'LinkId' is a valid link identifier, opened with 'OpenLink()'.
 *	'Timeout' is a non negative integer representing tenth of seconds.
 *	A 'Timeout' value of zero is an infinite timeout.
 *	The timeout is for the complete operation.
 *	If timeout is positive then ReadLink may return having read
 *	less than the number of bytes asked for.
 *	Return the number of bytes placed into 'Buffer' (which may be zero)
 *	or negative to indicate an error.
 *	SBUS: It is impossible to determine the number of bytes already read
 *	if a timeout occures. In this case the complete dpr buffer (448bytes)
 *	is transferred to the user.
 *
 *	Global variable
 *
 *	int 	time_tenth;
 *	int 	time_max;
 *	struct	SBusSlot  SBusSlot[];
 *	struct	LinkInfo  *LinkInfo;
 *
 */

int ReadLink (LinkId, Buffer, Count, Timeout)
int 	LinkId;
byte 	*Buffer;
u_int  	Count;
int 	Timeout;
{	short 	 tmout;			/* flag wether timeout active */
	u_int	 transfrd;		/* number of transfered bytes */
	int      timecnt;		/* timeout loop down counter */
	caddr_t  link;			/* ptr to base of link hardware */
	byte	 *startadr;		/* channel start address */
	u_int 	 cnt_tmp;		/* tmp byte counter */
	struct	 LinkInfo    *l_info;	/* ptr to LinkInfo[LinkId] */
	struct	 DpR	     *dpr;	/* ptr to SBusSlot[...] */
	struct	 slink_s     *comm;	/* communication register */
	short	 num_words_chan;	/* number of long words / channel */
	long	 *long_buf1;		/* ptr to begin of DPR */
	long	 *long_buf2;		/* long aligned ptr to user buffer */
	long	 long_buf;		/* buffer for single byte transfer */
	byte	 *byte_buf;		/* buffer for single byte transfer */
	short	 i;			/* loop counter */
	short	 single_bytes;		/* number of single bytes */
	short	 single_words;		/* number of single words */
	int 	 mask = READYMASK;
	word	 smask = (word)(SBUSMASK | 1);
	struct	 slink_s     *l_ev;	/* communication buffer T222 <-> SBus */
	byte	 swap_bytes[2];
	byte	 swap;
	word	 *swap_word;

	
	/* range check LinkId */
	if ( (LinkId < 0) || (LinkId >= MaxLinkId) ) {
		/* LinkId out of range */
		return (EIDINVAL);
	}

	/* is that link active */
	if ( LinkInfo[LinkId].opened == FALSE ) {
		/* link not active */
		return (ENOTOPEN);
	}

	/* range check of Count */
	if ( Count <= 0 ) {
		return (0);
	}

	transfrd = 0;
	tmout    = FALSE;
	l_info = &(LinkInfo[LinkId]);

	switch (l_info->Bus_type) {
   case VMEBUS:
	if ( Timeout > 0 ) {
		/* guard operation with timeout */
		tmout = TRUE;
		/* compute timeout downcounter value */
		if (Timeout > time_max)
			Timeout = time_max;
		timecnt = Timeout * time_tenth;
	}

	link     = LinkInfo[LinkId].usr_ad;

	switch (LinkInfo[LinkId].B_type) {
 	   case BBKV1 :
		for (; Count > 0; Count--) {
			while (( ((byte)((struct slink_1 *)link)->instatus) 
							     & mask) == 0) {
				/* no byte ready for input */
				if ( tmout ) {
					if ( timecnt-- <= 0 ) {
						/* operation timed out */
						return (transfrd);
					} /* if timecnt-- */
				} /* if guard timeout */
	 		} /* while instatus not_ready */

			/* link ready for input */
			 *Buffer++ = ((struct slink_1 *)link)->indata;

			/* increment the number of transfered bytes */
	 		transfrd++;
		} /* for Count>0 */
		return (transfrd);

	   case BBKV2 :
		for (; Count > 0; Count--) {
	 		while (( ((byte)((struct slink_2 *)link)->instatus)
							     & mask) == 0) {
				/* no byte ready for input */
				if ( tmout ) {
					if ( timecnt-- <= 0 ) {
						/* operation timed out */
						return (transfrd);
					} /* if timecnt-- */
				} /* if guard timeout */
	 		} /* while instatus not_ready */

	 		/* link ready for input */
	 		*Buffer++ = ((struct slink_2 *)link)->indata;

	 		/* increment the number of transfered bytes */
	 		transfrd++;
		} /* for Count>0 */
		return (transfrd);

      	case VMTM :
		for (; Count > 0; Count--) {
			while (( ((byte)((struct slink_v *)link)->instatus) 
							     & mask) == 0) {
				/* no byte ready for input */
				if ( tmout ) {
					if ( timecnt-- <= 0 ) {
						/* operation timed out */
						return (transfrd);
					} /* if timecnt-- */
				} /* if guard timeout */
	   		} /* while instatus not_ready */

			/* link ready for input */
	 		*Buffer++ = ((struct slink_v *)link)->indata;

	 		/* increment the number of transfered bytes */
	 		transfrd++;
		} /* for Count>0 */
		return (transfrd);

      	case BBKV4 :
		for (; Count > 0; Count--) {
	   		while (( ((byte)((struct slink_4 *)link)->instatus) 
							     & mask) == 0) {
				/* no byte ready for input */
				if ( tmout ) {
					if ( timecnt-- <= 0 ) {
						/* operation timed out */
						return (transfrd);
					} /* if timecnt-- */
				} /* if guard timeout */
	  		 } /* while instatus not_ready */

		 	/* link ready for input */
		 	*Buffer++ = ((struct slink_4 *)link)->indata;
	
		 	/* increment the number of transfered bytes */
	   		transfrd++;
		} /* for Count>0 */
		return (transfrd);

      	case MTMSUN1 :
      	case MTMSUN2 :
		for (; Count > 0; Count--) {
	   		while (( ((byte)((struct slink_m *)link)->instatus)
							     & mask) == 0) {
				/* no byte ready for input */
				if ( tmout ) {
					if ( timecnt-- <= 0 ) {
						/* operation timed out */
						return (transfrd);
					} /* if timecnt-- */
				} /* if guard timeout */
	  		} /* while instatus not_ready */

	   		/* link ready for input */
	   		*Buffer++ = ((struct slink_m *)link)->indata;

	   		/* increment the number of transfered bytes */
	   		transfrd++;
		} /* for Count>0 */
		return (transfrd);
      	default :
		return (ENOTYPE);
	} /* switch board type */

   case SBUS:

	if ( Timeout > 0 ) {
		/* guard operation with timeout */

		tmout = TRUE;
		/* compute timeout downcounter value */
		if (Timeout < SBUS_MIN_TIMEOUT){
			Timeout = SBUS_MIN_TIMEOUT;}
		if (Timeout > time_max){
			Timeout = time_max;}
		timecnt = Timeout * time_tenth / STIME_SCALE;
	}

	switch (LinkInfo[LinkId].B_type) {
      	   case BBKS4:
		if (LinkId == GlobalPacketInfo.LinkId) {
		   /* no stream read possible on PacketLink */
		   return (ENOSTREAMLINK);
		}

		swap_word      = (word*)swap_bytes;
		dpr = (struct DpR *)&(SBusSlot[l_info->B_slot].dpr);		
		comm = (struct slink_s *)((l_info->usr_ad) + dpr->com_off);
		num_words_chan = FlagAdr[LinkId].half_chan_len / sizeof (long);
		byte_buf = (byte *)&(long_buf);

		while ( *FlagAdr[LinkId].test_read != smask) {
		   /* no byte ready for input */
		   if ( tmout ) {
			if ( timecnt-- <= 0 ) {
			   /* operation timed out */
			   /* set event type */
			   comm->stopreset[l_info->L_no] = (word)(TIMEOUT_READ);
			   /* set event flag */
			   comm->startreset[l_info->L_no] = (word)(TIMEOUT_READ);
		
			   while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
				getpid () ;
#endif	IPX
			
			   }
			   return (transfrd);
			} /* if timecnt-- */
		   } /* if guard timeout */
#ifdef	IPX
		   getpid (); /* force system to actualize cache */
#endif	IPX
  		} /* while testread not ready */

		cnt_tmp = Count;

		/* compute start address of channel buffer */
		startadr = (byte *)(l_info->usr_ad + dpr->channel_off 
					+ l_info->L_no * dpr->channel_len);
		long_buf1 = (long *)startadr;

		/**************** Buffer n o t long alligned ******************/
		if ((single_bytes = (u_long)(Buffer) % sizeof (long)) > 0) {

		   while ( *FlagAdr[LinkId].test_read != smask) {
		      /* no byte ready for input */
	 	      if ( tmout ) {
		         if ( timecnt-- <= 0 ) {
			    /* operation timed out */
			    /* set event type */
			    comm->stopreset[l_info->L_no] = (word)(TIMEOUT_READ);
			    /* set event flag */
			    comm->startreset[l_info->L_no] = (word)(TIMEOUT_READ);

			    while ( (comm->startreset[l_info->L_no]) != 0) {
#ifdef	IPX
			       getpid () ;
#endif	IPX
			
			    }
			    return (transfrd);
			} /* if timecnt-- */
		      } /* if guard timeout */
#ifdef	IPX
		      getpid (); /* force system to actualize cache */
#endif	IPX
	  	   } /* while testread not ready */
		   single_bytes = 4 - single_bytes;/*LT*/

		   if (single_bytes > Count)
		      single_bytes = Count;

		   *swap_word    = (word)(single_bytes);
		   swap 	 = swap_bytes[0];
		   swap_bytes[0] = swap_bytes[1];
		   swap_bytes[1] = swap;

		   /* set number of bytes to transfer
		    * and ReadLink request flag
		    */
		   *FlagAdr[LinkId].read_link_rq1 = *swap_word;

		   while ( *FlagAdr[LinkId].read_link_rq0 == 0) {
		      /* BlockIn acknowledge flag not set */
		      if ( tmout ) {
			 if ( timecnt-- <= 0 ) {
			    /* operation really timed out */

			    /* fetch insecure DPR buffer contents */
			    long_buf = *long_buf1;
			    for (i = 0; i < single_bytes; i++)
			        *Buffer++ = byte_buf[i];
#ifdef	DEBUG
	printf ("\ntimeout! DPR buffer fetched.\n");
	fflush (stdout);
#endif	DEBUG
			    /* set event type */
			    comm->stopreset[l_info->L_no] = (word)(TIMEOUT_READ);
			    /* set event flag */
			    comm->startreset[l_info->L_no] = (word)(TIMEOUT_READ);
		
			    while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			       getpid () ;
#endif	IPX
			    }
			    return (transfrd);
			 } /* if timecnt-- */
		      } /* if guard timeout */
#ifdef	IPX
		      getpid (); /* force system to actualize cache */
#endif	IPX
  		   } /* while end of BlockIn not acknowledged */

		   *FlagAdr[LinkId].read_link_rq0 = 0;
		   long_buf = *long_buf1;
		   for (i = 0; i < single_bytes; i++)
		       *Buffer++ = byte_buf[i];
		   transfrd += single_bytes;
		   cnt_tmp  -= single_bytes;
#ifdef	IPX
		   getpid (); /* force system to actualize cache */
#endif	IPX
		}
		/************** rest of buffer now long alligned **************/

		if (cnt_tmp == 0) {
			return (transfrd); /*** everything's done -> return ***/
		}

		long_buf2 = (long *)Buffer;

		/*** transfer complete data block of length channel_len /2  ***/
		while (cnt_tmp >= FlagAdr[LinkId].half_chan_len) {
		   while ( *FlagAdr[LinkId].test_read != smask) {
		      /* no byte ready for input */
		      if ( tmout ) {
			 if ( timecnt-- <= 0 ) {
			    /* operation timed out */
			    /* set event type */
			    comm->stopreset[l_info->L_no] = (word)(TIMEOUT_READ);
			    /* set event flag */
			    comm->startreset[l_info->L_no] = (word)(TIMEOUT_READ);
		
			    while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			       getpid () ;
#endif	IPX
			
			    }
			    return (transfrd);
			 } /* if timecnt-- */
		      } /* if guard timeout */
#ifdef	IPX
		      getpid (); /* force system to actualize cache */
#endif	IPX
	  	   } /* while testread not ready */
		   *swap_word    = (word)(FlagAdr[LinkId].half_chan_len);
		   swap 	 = swap_bytes[0];
		   swap_bytes[0] = swap_bytes[1];
		   swap_bytes[1] = swap;

		   /* set number of bytes to transfer
		    * and ReadLink request flag
		    */
		   *FlagAdr[LinkId].read_link_rq1 = *swap_word;

	 	   while ( *FlagAdr[LinkId].read_link_rq0 == 0) {
		      if ( tmout ) {
			 if ( timecnt-- <= 0 ) {
			    /* operation really timed out */

			    /* fetch insecure DPR buffer contents */
			    for (i = 0; i < num_words_chan; i++)
				*long_buf2++ = *long_buf1++;		
#ifdef	DEBUG
	printf ("\ntimeout! DPR buffer fetched.\n");
	fflush (stdout);
#endif	DEBUG
			    /* set event type */
			    comm->stopreset[l_info->L_no] = (word)(TIMEOUT_READ);
			    /* set event flag */
			    comm->startreset[l_info->L_no] = (word)(TIMEOUT_READ);
		
			    while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			       getpid () ;
#endif	IPX
			
		  	    }
			    return (transfrd);
			 } /* if timecnt-- */
		       } /* if guard timeout */
#ifdef	IPX
		       getpid (); /* force system to actualize cache */
#endif	IPX
	  	   } /* while end of BlockIn not acknowledged */

		   *FlagAdr[LinkId].read_link_rq0 = 0;

		   /*** read complete data block ***/
		   for (i = 0; i < num_words_chan; i++)
			*long_buf2++ = *long_buf1++;
		
		   long_buf1 = (long *)startadr;

		   transfrd += FlagAdr[LinkId].half_chan_len;
		   cnt_tmp  -= FlagAdr[LinkId].half_chan_len;

		} /******** transfer of complete data blocks completed ********/

		if (cnt_tmp == 0) {
			return (transfrd); /*** everything's done -> return ***/
		}

		single_words = cnt_tmp / sizeof(long);

		/******************* transfer rest of bytes *******************/
		if (single_words) {
		   /************** transfer rest of long words ***********/
		   while ( *FlagAdr[LinkId].test_read != smask) {
		      /* no byte ready for input */
		      if ( tmout ) {
			 if ( timecnt-- <= 0 ) {
			    /* operation timed out */
			    /* set event type */
			    comm->stopreset[l_info->L_no] = (word)(TIMEOUT_READ);
			    /* set event flag */
			    comm->startreset[l_info->L_no] = (word)(TIMEOUT_READ);
		
			    while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			       getpid () ;
#endif	IPX					
			    }
			    return (transfrd);
			 } /* if timecnt-- */
		      } /* if guard timeout */
#ifdef	IPX
		      getpid (); /* force system to actualize cache */
#endif	IPX
	  	   } /* while testread not ready */

		   *swap_word    = (word)(single_words * sizeof (long));
		   swap          = swap_bytes[0];
		   swap_bytes[0] = swap_bytes[1];
		   swap_bytes[1] = swap;

		   /* set number of bytes to transfer
	   	    * and ReadLink request flag
		    */
		   *FlagAdr[LinkId].read_link_rq1 = *swap_word;

		   while ( *FlagAdr[LinkId].read_link_rq0 == 0) {
		      /* BlockIn acknowledge flag not set */
		      if ( tmout ) {
			 if ( timecnt-- <= 0 ) {
			    /* operation really timed out */

			    /* fetch insecure DPR buffer contents */
			    for (i = 0; i < single_words; i++)
				*long_buf2++ = *long_buf1++;		
#ifdef	DEBUG
	printf ("\ntimeout! DPR buffer fetched.(single long words)\n");
	fflush (stdout);
#endif	DEBUG
			    /* set event type */
			    comm->stopreset[l_info->L_no] = (word)(TIMEOUT_READ);
			    /* set event flag */
			    comm->startreset[l_info->L_no] = (word)(TIMEOUT_READ);
		
			    while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			       getpid () ;
#endif	IPX
			
			    }
			    return (transfrd);
			 } /* if timecnt-- */
		      } /* if guard timeout */
#ifdef	IPX
		      getpid (); /* force system to actualize cache */
#endif	IPX
  		   } /* while end of BlockIn not acknowledged */

		   *FlagAdr[LinkId].read_link_rq0 = 0;

		   for (i = 0; i < single_words; i++) {
		      *long_buf2++ = *long_buf1++;
		   }

		   long_buf1 = (long *)startadr;
		   transfrd += single_words * sizeof (long);
		   cnt_tmp  -= single_words * sizeof (long);
		}  /******* transfer of single long words completed ******/

		if (cnt_tmp == 0) {
			return (transfrd); /*** everything's done -> return ***/
		}

		/******************* transfer single bytes ********************/
		while ( *FlagAdr[LinkId].test_read != smask) {
		   /* no byte ready for input */
		   if ( tmout ) {
		      if ( timecnt-- <= 0 ) {
			 /* operation timed out */
			 /* set event type */
			 comm->stopreset[l_info->L_no] = (word)(TIMEOUT_READ);
			 /* set event flag */
			 comm->startreset[l_info->L_no] = (word)(TIMEOUT_READ);
		
			 while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			    getpid () ;
#endif	IPX			
			 }
			 return (transfrd);
		      } /* if timecnt-- */
		   } /* if guard timeout */
#ifdef	IPX
		   getpid (); /* force system to actualize cache */
#endif	IPX
  		} /* while testread not ready */

		*swap_word    = (word)(cnt_tmp);
		swap 	      = swap_bytes[0];
		swap_bytes[0] = swap_bytes[1];
		swap_bytes[1] = swap;

		/* set number of bytes to transfer
		 * and ReadLink request flag
		 */
		*FlagAdr[LinkId].read_link_rq1 = *swap_word;

		while ( *FlagAdr[LinkId].read_link_rq0 == 0) {
		   /* BlockIn acknowledge flag not set */
		   if ( tmout ) {
		      if ( timecnt-- <= 0 ) {
			 /* operation really timed out */

			 /* fetch insecure DPR buffer contents */
		 	 Buffer = (byte *)long_buf2;
			 long_buf = *long_buf1;
			 for (i = 0; i < cnt_tmp; i++)
			     *Buffer++ = byte_buf[i];
#ifdef	DEBUG
	printf ("\ntimeout! DPR buffer fetched.(single bytes)\n");
	fflush (stdout);
#endif	DEBUG
			 /* set event type */
			 comm->stopreset[l_info->L_no] = (word)(TIMEOUT_READ);
			 /* set event flag */
			 comm->startreset[l_info->L_no] = (word)(TIMEOUT_READ);
		
			 while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			    getpid () ;
#endif	IPX
			 }
			 return (transfrd);
		      } /* if timecnt-- */
		   } /* if guard timeout */
#ifdef	IPX
		   getpid (); /* force system to actualize cache */
#endif	IPX
  		} /* while end of BlockIn not acknowledged */

		*FlagAdr[LinkId].read_link_rq0 = 0;

		Buffer = (byte *)long_buf2;
		long_buf = *long_buf1;
		for (i = 0; i < cnt_tmp; i++)
			*Buffer++ = byte_buf[i];

		transfrd += cnt_tmp;

		return (transfrd);

	   default :
		/* board no valid SBus device */
		return (ESBUSNAME);

	} /* switch (LinkInfo ...) */

   default:
	/* unknown bus type */
	return (EBUSTYPE);
     	} /* switch (l_info->Bus_type) */
}



/*
 *	WriteLink
 *
 *	Write 'Count' bytes from 'Buffer' to the specified link.
 *	'LinkId' is a valid link identifier, opened with 'OpenLink()'.
 *	'Timeout' is a non negative integer representing tenth of seconds.
 *	A 'Timeout' value of zero is an infinite timeout.
 *	The timeout is for the complete operation.
 *	If timeout is positive then WriteLink may return having written
 *	less than the number of bytes asked for.
 *	Return the number of bytes actually written (which may be zero)
 *	or negative to indicate an error.
 *	Actually written means send to transputer with acknowledge
 *	received.
 *
 *	Implementation Note (VME-bus boards only)
 *
 *	The used link-adaptor is a C011/C012. The output-status is
 *	assumed to be 1 always. When sending a byte it is written
 *	to the output-data register and then the output-status
 *	register is checked (with timeout) until output-status is
 *	1 again (1 means: ready for next byte).
 *	Once when entering this function the output-status register
 *	is checked for containing a 1. If this check failes, 'WriteLink'
 *	immideatly returns 0.
 *
 *	Global variable
 *
 *	int 	time_tenth;
 *	int 	time_max;
 *	struct	SBusSlot  SBusSlot[];
 *	struct	LinkInfo  *LinkInfo;
 */

int WriteLink (LinkId, Buffer, Count, Timeout)
int	LinkId;
byte	*Buffer;
u_int	Count;
int	Timeout;
{	short 	tmout;			/* flag wether timeout active */
	u_int 	transfrd;		/* number of transfered bytes */
	int     timecnt;		/* timeout loop down counter */
	caddr_t link;			/* ptr to base of link hardware */
	byte	*startadr;		/* channel start address */
	u_int	cnt_tmp;		/* tmp byte counter */
	struct	LinkInfo    *l_info;	/* ptr to LinkInfo[LinkId] */
	struct	DpR	    *dpr;	/* ptr to SBusSlot[...] */
	struct	slink_s     *l_ev;	/* communication buffer T222 <-> SBus */
	struct	slink_s	    *comm;	/* communication register */
	long	*long_buf1;		/* ptr to channel memory */
	long	*long_buf2;		/* ptr to user data */
	long	long_buf;		/* single bytes buffer */
	byte 	*byte_buf;		/* single bytes buffer */
	short	num_words_chan;		/* number of long words / channel */
	short	i;			/* loop counter */
	short	single_bytes;		/* number of single bytes */
	short	single_words;		/* number of single words */
	int 	mask = READYMASK;
	word	smask = (word)(SBUSMASK | 1);
	byte	swap_bytes[2];
	byte	swap;
	word	*swap_word;

	/* range check LinkId */
	if ( (LinkId < 0) || (LinkId >= MaxLinkId) ) {
		/* LinkId out of range */
		return (EIDINVAL);
	}

	/* is that link active */
	if ( LinkInfo[LinkId].opened == FALSE ) {
		/* link not active */
		return (ENOTOPEN);
	}

	/* range check of Count */
	if ( Count <= 0 ) {
		return (0);
	}

	transfrd = 0;
	tmout    = FALSE;
	l_info = &(LinkInfo[LinkId]);

	switch (l_info->Bus_type) {
   case VMEBUS:
	if ( Timeout > 0 ) {
		/* guard operation with timeout */
		tmout = TRUE;
		/* compute timeout downcounter value */
		if (Timeout > time_max)
			Timeout = time_max;
		timecnt = Timeout * time_tenth;
	}

	link     = LinkInfo[LinkId].usr_ad;

	switch (LinkInfo[LinkId].B_type) {
      	   case BBKV1 :
		if (( ((byte)((struct slink_1 *)link)->outstatus)
							 & mask) == 0) {
   		   /* should never happen.
	    	    * output-status not ready for next byte
	    	    * return 0
	    	    */
	   	   return (0);
		}
		for (; Count > 0; Count--) {
	   	   /* link ready for output, send byte */
	   	   ((struct slink_1 *)link)->outdata = *Buffer++;

		   /* wait until byte send. check output-status */
	   	   while (( ((byte)((struct slink_1 *)link)->outstatus)
							 & mask) == 0) {
			/* no byte ready for output */
			if ( tmout ) {
				if ( timecnt-- <= 0 ) {
					/* operation timed out */
					return (transfrd);
				} /* if timecnt-- */
			} /* if guard timeout */
		   } /* while outstatus not_ready */

		   /* increment the number of transfered bytes */
		   transfrd++;
		} /* for Count>0 */
		return (transfrd);

      	   case BBKV2 :
		if (( ((byte)((struct slink_2 *)link)->outstatus)
						    & mask) == 0) {
		   /* should never happen.
		    * output-status not ready for next byte
	    	    * return 0
	    	    */
	   	   return (0);
		}
		for (; Count > 0; Count--) {
		   /* link ready for output, send byte */
	   	   ((struct slink_2 *)link)->outdata = *Buffer++;

		   /* wait until byte send. check output-status */
		   while (( ((byte)((struct slink_2 *)link)->outstatus) 
							  & mask) == 0) {
			/* no byte ready for output */
			if ( tmout ) {
				if ( timecnt-- <= 0 ) {
					/* operation timed out */
					return (transfrd);
				} /* if timecnt-- */
			} /* if guard timeout */
	   	   } /* while outstatus not_ready */

		   /* increment the number of transfered bytes */
		   transfrd++;
		} /* for Count>0 */
		return (transfrd);

           case VMTM :
		if (( ((byte)((struct slink_v *)link)->outstatus)
						    & mask) == 0) {
	   	   /* should never happen.
		    * output-status not ready for next byte
	   	    * return 0
	    	    */
		   return (0);
		}
		for (; Count > 0; Count--) {
	   	   /* link ready for output, send byte */
	 	   ((struct slink_v *)link)->outdata = *Buffer++;

		   /* wait until byte send. check output-status */
		   while (( ((byte)((struct slink_v *)link)->outstatus) 
							  & mask) == 0) {
			/* no byte ready for output */
			if ( tmout ) {
				if ( timecnt-- <= 0 ) {
					/* operation timed out */
					return (transfrd);
				} /* if timecnt-- */
			} /* if guard timeout */
	   	   } /* while outstatus not_ready */

		   /* increment the number of transfered bytes */
		   transfrd++;
		} /* for Count>0 */
		return (transfrd);

           case BBKV4 :
		if (( ((byte)((struct slink_4 *)link)->outstatus) 
						    & mask) == 0) {
	 	   /* should never happen.
	    	    * output-status not ready for next byte
	   	    * return 0
	   	    */
		   return (0);
		}
		for (; Count > 0; Count--) {
	   	   /* link ready for output, send byte */
	  	   ((struct slink_4 *)link)->outdata = *Buffer++;

		   /* wait until byte send. check output-status */
		   while (( ((byte)((struct slink_4 *)link)->outstatus) 
							  & mask) == 0) {
			/* no byte ready for output */
			if ( tmout ) {
				if ( timecnt-- <= 0 ) {
					/* operation timed out */
					return (transfrd);
				} /* if timecnt-- */
			} /* if guard timeout */
	   	   } /* while outstatus not_ready */

		   /* increment the number of transfered bytes */
		   transfrd++;
		} /* for Count>0 */
		return (transfrd);

           case MTMSUN1 :
     	   case MTMSUN2 :
		if (( ((byte)((struct slink_m *)link)->outstatus) 
						    & mask) == 0) {
		   /* should never happen.
	    	    * output-status not ready for next byte
		    * return 0
	    	    */
		   return (0);
		}
		for (; Count > 0; Count--) {
		   /* link ready for output, send byte */
	  	   ((struct slink_m *)link)->outdata = *Buffer++;

		   /* wait until byte send. check output-status */
		   while (( ((byte)((struct slink_m *)link)->outstatus)
							  & mask) == 0) {
			/* no byte ready for output */
			if ( tmout ) {
				if ( timecnt-- <= 0 ) {
					/* operation timed out */
					return (transfrd);
				} /* if timecnt-- */
			} /* if guard timeout */
	   	   } /* while outstatus not_ready */

		   /* increment the number of transfered bytes */
		   transfrd++;
		} /* for Count>0 */
		return (transfrd);
     	default :
		return (ENOTYPE);
	} /* switch board type */

   case SBUS:
	if ( Timeout > 0 ) {
		/* guard operation with timeout */
		tmout = TRUE;
		/* compute timeout downcounter value */
		if (Timeout < SBUS_MIN_TIMEOUT){
			Timeout = SBUS_MIN_TIMEOUT;}
		if (Timeout > time_max){
			Timeout = time_max;}
		timecnt = Timeout * time_tenth / STIME_SCALE;
	}

	switch (LinkInfo[LinkId].B_type) {
      	   case BBKS4:

		if (LinkId == GlobalPacketInfo.LinkId) {
		   /* no stream write possible on PacketLink */
		   return (ENOSTREAMLINK);
		}

		swap_word      = (word*)swap_bytes;
		*FlagAdr[LinkId].write_link_rq0 = 0;
                dpr = (struct DpR *)&(SBusSlot[l_info->B_slot].dpr);
		comm = (struct slink_s *)((l_info->usr_ad) + dpr->com_off);
		num_words_chan = FlagAdr[LinkId].half_chan_len / sizeof (long);

		while ( *FlagAdr[LinkId].test_write != smask) {
		   /* no byte ready for output */
		   if ( tmout ) {
		      if ( timecnt-- <= 0 ) {
			 /* operation timed out */
			 /* set event type */
			 comm->stopreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
			 /* set event flag */
			 comm->startreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
		
			 while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			    getpid () ;
#endif	IPX
			 }
			 return (transfrd);
		      } /* if timecnt-- */
		   } /* if guard timeout */
#ifdef	IPX
		   getpid (); /* force system to actualize cache */
#endif	IPX
  		} /* while testwrite not ready */

		cnt_tmp = Count;
		/* compute start address of channel buffer */
		startadr = (byte *)(l_info->usr_ad
			 + dpr->channel_off + l_info->L_no * dpr->channel_len
			 + (int)(FlagAdr[LinkId].half_chan_len));
		long_buf1 = (long *)startadr;
		
		/**************** Buffer n o t long alligned ******************/
		if ((single_bytes = (u_long)(Buffer) % sizeof (long)) > 0) {
		   single_bytes = 4 - single_bytes; /*LT*/
			
		   if (single_bytes > Count)
		      single_bytes = Count;

		   while ( *FlagAdr[LinkId].test_write != smask) {
		      /* no byte ready for output */
		      if ( tmout ) {
			 if ( timecnt-- <= 0 ) {
			    /* operation timed out */
			    /* set event type */
			    comm->stopreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
			    /* set event flag */
			    comm->startreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
		
			    while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			       getpid () ;
#endif	IPX
			    }
			    return (transfrd);
			 } /* if timecnt-- */
		      } /* if guard timeout */
#ifdef	IPX
		      getpid (); /* force system to actualize cache */
#endif	IPX
  		   } /* while testwrite not ready */

		   byte_buf = (byte *)&(long_buf);
		   for (i = 0; i < single_bytes; i++)
		       byte_buf[i] = *Buffer++;
		   *long_buf1 = long_buf;
			
		   *swap_word    = (word)(single_bytes);
		   swap 	 = swap_bytes[0];
		   swap_bytes[0] = swap_bytes[1];
		   swap_bytes[1] = swap;

		   /* set number of bytes to transfer
		    * and WriteLink request flag
		    */
		   *FlagAdr[LinkId].write_link_rq1 = *swap_word;			
		   /* BlockOut acknowledge flag not set */
		   while ( *FlagAdr[LinkId].write_link_rq0 == 0) {
		      if ( tmout ) {
			 if ( timecnt-- <= 0 ) {
			    /* operation really timed out */
			    /* set event type */
			    comm->stopreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
			    /* set event flag */
			    comm->startreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
		
			    while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			      getpid () ;
#endif	IPX
			    }
			    return (transfrd);
			 } /* if (timecnt-- ..) */
		      } /* if guard timeout */
#ifdef	IPX
		      getpid (); /* force system to actualize cache */
#endif	IPX
	  	   } /* while end of blockOut not acknowledged */
			
		   *FlagAdr[LinkId].write_link_rq0 = 0;

		   transfrd += single_bytes;
		   /*Buffer   += (byte)(single_bytes);*//*LT*/
		   cnt_tmp  -= single_bytes;

		} /* end of single byte transfer */
		/************** rest of buffer now long alligned **************/

		long_buf2 = (long *)Buffer;

		/*** transfer complete data blocks of length channel_len/2 ***/

		/* swap count bytes */
		*swap_word    = (word)FlagAdr[LinkId].half_chan_len;
		swap 	      = swap_bytes[0];
		swap_bytes[0] = swap_bytes[1];
		swap_bytes[1] = swap;

		while (cnt_tmp >= FlagAdr[LinkId].half_chan_len) {
		   while ( *FlagAdr[LinkId].test_write != smask) {
		      /* no byte ready for output */
		      if ( tmout ) {
			 if ( timecnt-- <= 0 ) {
			    /* operation timed out */
			    /* set event type */
		   	    comm->stopreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
			    /* set event flag */
			    comm->startreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
		
			    while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			       getpid () ;
#endif	IPX
			    }
			    return (transfrd);
			 } /* if timecnt-- */
		      } /* if guard timeout */
#ifdef	IPX
		      getpid (); /* force system to actualize cache */
#endif	IPX
 		   } /* while testwrite not ready */

		   /****** write complete data block ******/
		   for (i=0; i< num_words_chan; i++) 
		       *long_buf1++ = *long_buf2++;
		
		   long_buf1 = (long *)startadr;

		   /* set Number of Bytes to Write */
		   *FlagAdr[LinkId].write_link_rq1 = *swap_word;

		   while ( *FlagAdr[LinkId].write_link_rq0 == 0 ) {
		      /* still bytes to write */
		      if ( tmout ) {
			 if ( timecnt-- <= 0 ) {
			    /* operation really timed out */
			    /* set event type */
			    comm->stopreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
			    /* set event flag */
			    comm->startreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
		
			    while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
				getpid () ;
#endif	IPX
			    }
			    return (transfrd);
			 } /* if timecnt-- */
		      } /* if guard timeout */
#ifdef	IPX
		      getpid (); /* force system to actualize cache */
#endif	IPX
	  	   } /* while end of write to link not acknowledged */

		   *FlagAdr[LinkId].write_link_rq0 = 0;

		   transfrd += FlagAdr[LinkId].half_chan_len;
		   cnt_tmp  -= FlagAdr[LinkId].half_chan_len;
		} /******** transfer of complete data blocks completed ********/


		/******************* transfer rest of bytes *******************/

		if (cnt_tmp == 0) {
			return (transfrd); /*** everything's done -> return ***/
		}

		single_words = cnt_tmp / sizeof(long);

		if (single_words) {
		   /************** transfer rest of long words ***********/

		   while ( *FlagAdr[LinkId].test_write != smask) {
		      /* no byte ready for output */
		      if ( tmout ) {
			 if ( timecnt-- <= 0 ) {
			    /* operation timed out */
			    /* set event type */
			    comm->stopreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
		   	    /* set event flag */
			    comm->startreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
		
			    while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			      getpid () ;
#endif	IPX
			    }
			    return (transfrd);
			 } /* if timecnt-- */
		      } /* if guard timeout */
#ifdef	IPX
		      getpid (); /* force system to actualize cache */
#endif	IPX  	
		   } /* while testwrite not ready */

		   for (i = 0; i < single_words; i++) {
		       *long_buf1++ = *long_buf2++;
		   }

		   long_buf1 = (long *)startadr;

		   *swap_word    = (word)(single_words * sizeof (long));
		   swap 	 = swap_bytes[0];
		   swap_bytes[0] = swap_bytes[1];
		   swap_bytes[1] = swap;

		   *FlagAdr[LinkId].write_link_rq1 = *swap_word;

		   while ( *FlagAdr[LinkId].write_link_rq0 == 0 ) {
		      /* still bytes to write */
		      if ( tmout ) {
			 if ( timecnt-- <= 0 ) {
			    /* operation really timed out */
			    /* set event type */
			    comm->stopreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
			    /* set event flag */
			    comm->startreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
		
			    while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			       getpid () ;
#endif	IPX
			    }
			    return (transfrd);
			 } /* if timecnt-- */
		      } /* if guard timeout */
#ifdef	IPX
		      getpid (); /* force system to actualize cache */
#endif	IPX  
		   } /* while end of write not acknowledged */

		   *FlagAdr[LinkId].write_link_rq0 = 0;

		   transfrd += single_words * sizeof (long);
		   cnt_tmp -= single_words * sizeof (long);

		}  /******* transfer of single long words completed ******/

		if (cnt_tmp == 0) {
			return (transfrd); /*** everything's done -> return ***/
		}

		/******************* transfer single bytes ********************/
		while ( *FlagAdr[LinkId].test_write != smask ) {
		   /* no byte ready for output */
		   if ( tmout ) {
		      if ( timecnt-- <= 0 ) {
			 /* operation timed out */
			 /* set event type */
			 comm->stopreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
			 /* set event flag */
			 comm->startreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
		
			 while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			    getpid () ;
#endif	IPX
			 }
			 return (transfrd);
		      } /* if timecnt-- */
		   } /* if guard timeout */
#ifdef	IPX
		   getpid (); /* force system to actualize cache */
#endif	IPX
  		} /* while testwrite not ready */

		Buffer = (byte *)long_buf2;
		byte_buf = (byte *)&(long_buf);
		for (i = 0; i < cnt_tmp; i++)
			byte_buf[i] = *Buffer++;
		*long_buf1 = long_buf;

		*swap_word    = (word)(cnt_tmp);
		swap 	      = swap_bytes[0];
		swap_bytes[0] = swap_bytes[1];
		swap_bytes[1] = swap;

		*FlagAdr[LinkId].write_link_rq1 = *swap_word;

		while ( *FlagAdr[LinkId].write_link_rq0 == 0) {
		   /* BlockOut acknowledge flag not set */
		   if ( tmout ) {
		      if ( timecnt-- <= 0 ) {
			 /* operation really timed out */
			 /* set event type */
		  	 comm->stopreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
			 /* set event flag */
			 comm->startreset[l_info->L_no] = (word)(TIMEOUT_WRITE);
		
			 while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
			    getpid () ;
#endif	IPX
			 }
			 return (transfrd);
		      } /* if timecnt-- */
		   } /* if guard timeout */
#ifdef	IPX
		   getpid (); /* force system to actualize cache */
#endif	IPX
  		} /* while end of write not acknowledged */

		*FlagAdr[LinkId].write_link_rq0 = 0;

		transfrd += cnt_tmp;
		return (transfrd);

	   default :
		/* board no valid SBus device */
		return (ESBUSNAME);

	} /* switch (LinkInfo ...) */

   default:
	/* unknown bus type */
	return (EBUSTYPE);
	/* switch (l_info->Bus_type) */
   }
}

/*
 *	PacketMode
 *
 *	Switch the specified link to packet mode. This link is called
 *	'PacketLink'. The link 'LinkId' must already have been opened 
 *	successfully by use of 'OpenLink()'. The specified link must
 * 	belong to a board supporting packet protocol (BBK-S4 only). 
 *	'PacketSize' is the maximum size of any packet transfered over
 * 	this PacketLink. It is used by the firmware to make optimal
 * 	use of buffer memory on the BBK-S4.
 *
 *	For a single user there is at most one link switched to packet
 *	mode, which must not be used in stream mode (ReadLink(), Write
 *	Link()).
 *
 *	Returns 0 on success and a negative error code otherwise.
 *
 *
 *	Global variables
 *
 *	struct	LinkInfo	*LinkInfo;
 *	struct 	packetInfo	GlobalPacketInfo;
 *	struct	FlagAdr		*FlagAdr;
 *	
 */

int PacketMode (LinkId, PacketSize)
int LinkId;
int PacketSize;
{	
	struct	LinkInfo  *l_info;	/* ptr to LinkInfo[LinkId] */
	struct	DpR	  *dpr;		/* ptr to SBusSlot[...] */
	struct	slink_s	  *comm;	/* communication register */

	/* range check LinkId */
	if ( (LinkId < 0) || (LinkId >= MaxLinkId) ) {
		/* LinkId out of range */
		return (EIDINVAL);
	}

	/* is that link active */
	if ( LinkInfo[LinkId].opened == FALSE) {
		/* link not active */
		return (ENOTOPEN);
	}
	
	l_info = &(LinkInfo[LinkId]);

	dpr = (struct DpR *)&(SBusSlot[l_info->B_slot].dpr);

	/* compute the address of the T222 <-> SBus
	 * communication registers and set the comm
	 * struct to this address
	 */

	comm = (struct slink_s *)((l_info->usr_ad) + dpr->com_off);

	/* does the specified link support packet protocol */
	if ( (LinkInfo[LinkId].Bus_type != SBUS)
			 || (LinkInfo[LinkId].B_type != BBKS4) ) {
		/* link does not support packet protocol */
		return (ENOPMODE);
	}

	/* range check PacketSize */
	if ( (PacketSize <= 0) || (PacketSize > PACKET_DATA_LEN) ) {
		/* PacketSize out of range */
		return (EBIGPACKET);
	}

	/* does PacketSize allows long word transfers */
	if ( PacketSize % 4 > 0)  {
		/* PacketSize doesn't allow long word transfers */
		return (EPACKNOTLONG);
	}

	/* another link used as PacketLink */
	if ( GlobalPacketInfo.LinkId != -1 ) {
		/* another link used as PacketLink */
		return (EPACKLINKOCC);
	}

	/* ok, everything's fine */

	/*initialize GetPacket and PutPackets variables */
	(void) init_GetPacket (LinkId);
	(void) init_PutPackets (LinkId);

	/* set event type */
	comm->stopreset[l_info->L_no] = (word)(INIT_PACKET);

	/* set event flag */
	comm->startreset[l_info->L_no] = (word)(INIT_PACKET);
		
	while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
		getpid () ;
#endif	IPX
		;
	}

	GlobPackInfo = (struct packetInfo*)&(GlobalPacketInfo);
	/* change link mode from stream mode to packet mode */
	LinkInfo[LinkId].packetlink = TRUE;

	/* actualize GlobalPacketInfo */
	GlobalPacketInfo.LinkId      = LinkId;
	GlobalPacketInfo.PacketSize  = PacketSize;
	GlobalPacketInfo.MaxBuffSize = FlagAdr[LinkId].half_chan_len;

	/* link mode successfully changed to packet mode */

	return (OK);

}


/*
 *	StreamMode
 *
 *	Switch back the PacketLink to the default stream operation mode.
 *	The packet mode operation on the PacketLink is terminated defi-
 *	nitely. The output queue may contain pending packets. The total
 *	number of disgarded packets is reported as the return value.
 *
 *	To know for a certainty that there are no pending packets, 'Info
 *	Packet()' should be called before calling 'StreamMode()'.
 *
 *	The firmware parametrisation (mode and packet size) is controlled
 *	by 'OpenLink()' and 'PacketMode()'. They will also detect if the 
 *	firmware hangs and will report. In such a case the additionally 
 *	supplied programm 's4diag' may be called to force a power-on reset
 *	at the BBK-S4.
 *
 *	Returns the number of packets scheduled (by 'PutPackets()') but
 *	not yet transfered via link. The return value 0 signals that no
 *	packet was wasted. Otherwise a negative error code will be returned.
 *
 *	Global variables
 *
 *	struct	LinkInfo  	*LinkInfo;
 *	struct	packetInfo	GlobalPacketInfo;
 *	
 */

int	StreamMode ()
{	
	struct	packetInfo   *packInfo;
	struct	LinkInfo  *l_info;	/* ptr to LinkInfo[LinkId] */
	struct	DpR	  *dpr;		/* ptr to SBusSlot[...] */
	struct	slink_s	  *comm;	/* communication register */
	

	l_info = &(LinkInfo[GlobalPacketInfo.LinkId]);

	dpr = (struct DpR *)&(SBusSlot[l_info->B_slot].dpr);

	/* compute the address of the T222 <-> SBus
	 * communication registers and set the comm
	 * struct to this address
	 */

	comm = (struct slink_s *)((l_info->usr_ad)
					+ dpr->com_off);

	if ( (packInfo = InfoPacket()) == NULL )  {
		/* no open PacketLink */
		return (ENOPALINK);
	}

	/* set BBK-S4 stream mode flag */
	*LinkMode = STREAMMODE;


	/* set event type */
	comm->stopreset[l_info->L_no] = (word)(INIT_STREAM);

	/* set event flag */
	comm->startreset[l_info->L_no] = (word)(INIT_STREAM);

		
	while ( (comm->startreset[l_info->L_no]) != 0)  {
#ifdef	IPX
		getpid () ;
#endif	IPX
		;
	}

	/* change link mode from packet mode to stream mode */
	LinkInfo[GlobalPacketInfo.LinkId].packetlink = FALSE;
	
	/* actualize GlobalPacketInfo */
	GlobalPacketInfo.LinkId      = -1;

	/* link successfully changed from stream to packet mode */

	return (packInfo->OutPending);
}


/*
 *	InfoPacket
 *
 *	Used to get detailed information about the status of the
 *	PacketLink. This information is a 'snapshot' of the current
 *	situation. So the data are not guaranteed to be consistent.
 *	Nevertheless the most important aspects of these informations
 *	are covered. Every change of data during the snapshot is to 
 *	the safe side, that means only more InPackets, more free 
 *	OutPackets and less pending output packets actually than
 *	reported.
 *
 *	The packetInfo structure contains the following components:
 *
 *	struct	packetInfo  {
 *	    int LinkId;		'LinkId' of current PacketLink (-1 if none)
 *	    int PacketSize;	Maximum packet size, see 'PacketMode()'
 *	    int	MaxBuffSize;	Maximum buffer size. Cannot write more
 *				bytes at once
 *	    int	InPackets;	Number of packets ready to 'GetPacket()'
 *	    int	OutPackets;	Number of packets free in 'PutPackets()'
 *				buffer
 *	    int OutPending;	Estimate on number of pending output packets
 *	}
 *
 *	Returns a pointer to a (static) 'packetInfo' structure or NULL if
 *	there is no PacketLink.
 *
 *	Global variables
 *
 *	struct	LinkInfo  	*LinkInfo;
 *	struct	packetInfo	GlobalPacketInfo;
 *	struct	packetInfo	*GlobPackInfo;
 *	
 */

struct	packetInfo *InfoPacket ()
{	u_short	*int_buf;
	long	*BBKInfo;
	byte	byte_buffer[4];
	byte	swap;
	
	/* no open PacketLink */
	if ( GlobalPacketInfo.LinkId == -1 ) {
		/* no open PacketLink */
		return (NULL);
	}	

	/* set BBK-S4 InfoPacket mode flag */
	*LinkMode = INFOPACKETMODE;

	/* get number of:  - number of packets ready to GetPacket()
	 * 		   - number of packets free in PutPackets() buffer
	 *		   - pending output packets
	 */
	*No_of_pack_to_write = 0xFF00;

	while ( *No_of_pack_to_write != 0) {
#ifdef	IPX
		getpid () ;
#endif	IPX
		;
	}

	BBKInfo  = (long*)byte_buffer;
	int_buf = (u_short*)byte_buffer;

	*BBKInfo = Memory_write_address[0][0];

	swap = byte_buffer[0];
	byte_buffer[0] = byte_buffer[1];
	byte_buffer[1] = swap;
	swap = byte_buffer[2];
	byte_buffer[2] = byte_buffer[3];
	byte_buffer[3] = swap;

	GlobalPacketInfo.InPackets  = (int)*int_buf++;
	GlobalPacketInfo.OutPackets = (int)*int_buf--;

	*BBKInfo = Memory_write_address[0][1];

	swap = byte_buffer[0];
	byte_buffer[0] = byte_buffer[1];
	byte_buffer[1] = swap;

	GlobalPacketInfo.OutPending = (int)*int_buf;

	/* set BBK-S4 packet mode flag */
	*LinkMode = PACKETMODE; 

	return (GlobPackInfo);
}



/*
 *	init_GetPacket
 *
 *	Global variable
 *
 *	struct	LinkInfo  *LinkInfo	
 */

static
void init_GetPacket (LinkId)
int LinkId;
{
	int	mem_offset;
	int	act_buf_read_offset;
	int	bff_offset;
	int	i;
	caddr_t	page_adr;		/* virtual address of page */

	page_adr = (caddr_t)(LinkInfo[LinkId].usr_ad);

	switch (LinkInfo[LinkId].L_no)  {
	   case 0:
		mem_offset = 0x4100;
		bff_offset = 0x5F06;
		act_buf_read_offset = 0x5F0E;
		break;
	   case 1:
		mem_offset = 0x4480;
		bff_offset = 0x5F26;
		act_buf_read_offset = 0x5F2E;
		break;
	   case 2:
		mem_offset = 0x4800;
		bff_offset = 0x5F46;
		act_buf_read_offset = 0x5F4E;
		break;
	   case 3:
		mem_offset = 0x4B80;
		bff_offset = 0x5F66;
		act_buf_read_offset = 0x5F6E;
		break;
	}

	Long_length_read = (long*)Long_length_read_array;
	*Long_length_read = 0x00000000; 

	Memory_read_address[0] = (long*)(page_adr + mem_offset);
	for (i = 1; i < NO_OF_DPR_READ_BUFS; i++) {
		Memory_read_address[i] = (long*)(Memory_read_address[i-1] 
						  + (DPR_BUF_LEN / 4) );	
	    }

	*Act_DPR_read_buf = 0;

	Bff_read[0] = (u_short*)(page_adr + bff_offset);

	for (i = 1; i < NO_OF_DPR_READ_BUFS; i++) {
	    Bff_read[i] = (u_short*)(Bff_read[i-1] + 0x1);
	}

	return;

}



/*
 *	GetPacket
 *
 *	Read a single packet from the PacketLink into the buffer. The
 *	buffer pointed to by 'Buffer' must be large enough to hold a
 *	packet of maximum size (not checked). The start address of
 *	'Buffer' has to be long alligned.
 *
 *	This function is strictly non-blocking and will return very fast.
 *	There is no waiting of any form.
 *
 *	Returns the number of bytes transfered or a negative error code.
 *	In case of no available packet ready to transfer, 0 is returned.
 *
 *	Global variables
 *
 *	struct	packetInfo	GlobalPacketInfo;
 *
 */

int GetPacket (Buffer)
char *Buffer;
{	long	*mem_ptr;
	long	*pattern;
	byte	*byte_buf;
	long	long_buf;
	byte	*bytes;
	int	j;
	int	no_of_copies;
	int	single_bytes;
	int	length;


	if (GlobalPacketInfo.LinkId == -1)  {
	   /* there is no open PacketLink */
	   return (ENOPALINK);
	}

	if ((u_long)Buffer % SIZE_OF_LONG > 0) {
	   /* buffer not long alligned */
	   return (ENOTALLIGN);
	}

	pattern = (long*)Buffer;

	mem_ptr = (long*)(Memory_read_address[*Act_DPR_read_buf]);

#ifdef	IPX
	getpid () ;
#endif	IPX

        if (*Bff_read[*Act_DPR_read_buf] == 0x0100) return (0); 

	*Long_length_read = *mem_ptr++;

	if ( (length = (int)(Long_length_read_array[2] & 0x7F)) >
			     GlobalPacketInfo.PacketSize)  {
	   /* packet length greater than PacketSize */	
	   return (EPACKLONG);
	}
	
	no_of_copies = length / SIZE_OF_LONG;

	if (Long_length_read_array[2] % SIZE_OF_LONG > 0) no_of_copies++;

	for (j = 0; j < no_of_copies; j++)
	    *pattern++ = *mem_ptr++;

	*Bff_read[*Act_DPR_read_buf] = 0x0100; 

	if (++*Act_DPR_read_buf == NO_OF_DPR_READ_BUFS) *Act_DPR_read_buf = 0;

	return ((int)Long_length_read_array[2]);
}



/*
 *	init_PutPackets
 *
 *	Global variable
 *
 *	struct	LinkInfo  *LinkInfo	
 */

static
void init_PutPackets (LinkId)
int LinkId;		/* link number */
{
	int	mem_offset;
	int	flag_offset;
	int	bff_offset;
	int	act_buf_write_offset;
	int 	link_mode_offset;
	int	i;
	caddr_t	page_adr;		/* virtual address of page */

	page_adr = (caddr_t)(LinkInfo[LinkId].usr_ad);

	switch (LinkInfo[LinkId].L_no)  {
	   case 0:
		mem_offset  = 0x42C0;
		flag_offset = 0x5F14;
		bff_offset  = 0x5F16;
		act_buf_write_offset = 0x5F1C;
		link_mode_offset     = 0x5F1E;
		break;
	   case 1:
		mem_offset  = 0x4640;
		flag_offset = 0x5F34;
		bff_offset  = 0x5F36;
		act_buf_write_offset = 0x5F3C;
		link_mode_offset     = 0x5F3E;
		break;
	   case 2:
		mem_offset  = 0x49C0;
		flag_offset = 0x5F54;
		bff_offset  = 0x5F56;
		act_buf_write_offset = 0x5F5C;
		link_mode_offset     = 0x5F5E;
		break;
	   case 3:
		mem_offset  = 0x4D40;
		flag_offset = 0x5F74;
		bff_offset  = 0x5F76;
		act_buf_write_offset = 0x5F7C;
		link_mode_offset     = 0x5F7E;
		break;
	}


	Memory_write_address[0] = (long*)(page_adr + mem_offset);
	for (i = 1; i < NO_OF_DPR_WRITE_BUFS; i++) {	
		Memory_write_address[i] = (long*)(Memory_write_address[i-1] 
						  + (DPR_BUF_LEN / 4) );
	}

	*Act_DPR_write_buf = 0;

	LinkMode = (u_short*)(page_adr + link_mode_offset);

	No_of_pack_to_write = (u_short*)(page_adr + flag_offset);
	swap = (u_short*)swap_no_of_pack;
	swap_no_of_pack[1] = 0x00;
	Long_length_write = (long*)Long_length_write_array;
	*Long_length_write = 0x00000000; 

	Bff_write[0] = (u_short*)(page_adr + bff_offset);

	for (i = 1; i < NO_OF_DPR_WRITE_BUFS; i++) {
	    Bff_write[i] = (u_short*)(Bff_write[i-1] + 0x1);
	}
        
	return;
}


/*
 *	PutPackets
 *
 *	Write as much packets to the PacketLink as possible. Each packet
 *	is described by an element of 'Packets', which is structured as an
 *	array of packets. 'Count' specifies the maximum number of valid
 *	packets to send.
 *
 *	A packet is structured as follows:
 *	
 *	struct	packet  {
 *	  byte  length;	Packet header: bits 0..6:  packet body length (bytes)
 *						 + 4 byte (pxhdr)
 *				       bit 7	:  tag bit
 *	  int32 pxhdr;	Parix header: 2 byte processor id, 2 byte channel no.
 *	  byte  *body;	Packet body
 *	}
 *
 *	The start address of 'body' has to be long alligned. The best per-
 *	formance will be achieved by setting the body length to multiples
 *	of long words (int32).
 *
 *	The operation is strictly non-blocking. There is no waiting of any
 *	kind. Each packet is tried to be transfered via the PacketLink as
 *	long as enough space is left on the BBK-S4 for a direct transfer.
 *	The number of directly transfered packets is returned. If it is not
 *	possible to send even a single packet, the return value is 0.
 *	
 *	Returns the number of packets transfered or a negative error code.
 *
 *	Global variables
 *
 *	struct	packetInfo	GlobalPacketInfo;
 *
 */

int PutPackets(Packets, Count)
struct packet	*Packets;
u_char Count;
{
	long	*long_buf;
	long	*mem_ptr;
	long	long_buffer;
	byte	*bytes;
	byte	*byte_buf;
	int	act_buf;
	int	i,j;
	int	no_of_copies;
	int	length;
	int	quit;

	if (GlobalPacketInfo.LinkId == -1)  {
	   /* there is no open PacketLink */
	   return (ENOPALINK);
	}

	swap_no_of_pack[1] = 0x00;
	swap_no_of_pack[0] = (u_char)Count;

	i = 0;
	quit = -100000;

	while ((quit == -100000) && (i < (int) Count)) {
	    Long_length_write_array[2] = (u_char)(Packets[i].length);

	    if ( (length = (int)(Long_length_write_array[2] & 0x7F)) >
				 GlobalPacketInfo.PacketSize)  {
		/* packet length greater than PacketSize */	
		quit = EPACKLONG;
	    }
		
	    no_of_copies = ( length / SIZE_OF_LONG ) - 1;

	    if (Long_length_write_array[2] % SIZE_OF_LONG != 0)
		no_of_copies++;
 
	    mem_ptr = (long*)(Memory_write_address[*Act_DPR_write_buf]);
	    long_buf = (long*)Packets[i].body;

	    if ((u_long)long_buf % SIZE_OF_LONG != 0)  {
 		/* packet body not long alligned */
	       	quit = EPACKBODYALL;
	    }

#ifdef	IPX
	    getpid () ;
#endif	IPX

	    if (*Bff_write[*Act_DPR_write_buf] != 0x0100) {
	       	quit = i;
	    }

	    if (quit == -100000) {

		    *mem_ptr++ = *Long_length_write;
		    *mem_ptr++ = Packets[i].pxhdr; 

		    for (j = 0; j < no_of_copies; j++) {
			*mem_ptr++ = *long_buf++;
		    }

		    *Bff_write[*Act_DPR_write_buf] = 0x0000;

		    *No_of_pack_to_write = 11;

		    if (*Act_DPR_write_buf < (NO_OF_DPR_WRITE_BUFS - 1)) {  
			*Act_DPR_write_buf = *Act_DPR_write_buf + 1;
		     } else {
			*Act_DPR_write_buf = 0;
		     }

		     i++;
	      } /* if (quit == -100000)*/

	}/* while ((quit == -100000) && (i < Count)) */

        while (*No_of_pack_to_write != 0x0000) {
#ifdef	IPX
	      getpid () ;
#endif	IPX
	}  

	if (i == (int) Count)
		quit = i;

	return (quit);
}

/*
 *	ResetLink
 *
 *	Reset the specified link line.
 *	'LinkId' is a valid link identifier, opened with 'OpenLink()'.
 *	Returns 1 if the reset is successful, a negative error code
 *	otherwise.
 *
 *	Global variables
 *
 *	struct	SBusSlot  SBusSlot[];
 *	struct	LinkInfo  *LinkInfo;
 *
 */

int ResetLink (LinkId)
int 	      LinkId;
{	register caddr_t link;		/* ptr to base of hardware */
	byte c;				/* byte for dummy read/write */
	struct	LinkInfo  *l_info;	/* ptr to LinkInfo[LinkId] */
	struct	DpR	  *dpr;		/* ptr to SBusSlot[...] */
	struct	slink_s	  *comm;	/* communication register */
	word	resetflag;		/* start/stop reset flag */
	short	tmout;			/* flag wether timeout is active */
	long	timecnt;		/* timeout loop down counter */
	struct	BBKS4_regs  *regs;	/* read/write register */
	word	*read_link_rq0;		/* ptr to readlink request flag 0 */
	word	*read_link_rq1;		/* ptr to readlink request flag 1 */
	word	*write_link_rq0;	/* ptr to writelink request flag 0 */
	word	*write_link_rq1;	/* ptr to writelink request flag 1 */
	word	*chan_free;		/* ptr to channel free flag */
	byte	*reset_val;		/* reset value */
	int     timecnt_int;		/* timeout loop down counter */

 	byte	 swap_bytes[2];
	byte	 swap;
	word	 *swap_word;

	register word	smask = (word)(SBUSMASK | 1);

	/* range check LinkId */
	if ( (LinkId < 0) || (LinkId >= MaxLinkId) ) {
		/* LinkId out of range */
		return (EIDINVAL);
	}

	/* is that link active */
	if ( LinkInfo[LinkId].opened == FALSE ) {
		/* link not active */
		return (ENOTOPEN);
	}
	l_info = &(LinkInfo[LinkId]);

	switch (l_info->Bus_type) {
	   case VMEBUS:
		link = LinkInfo[LinkId].usr_ad;

		switch (LinkInfo[LinkId].B_type) {
		   case BBKV1 :
			/* start reset */
			c = ((struct slink_1 *)link)->startreset;

			/* wait a little bit */
			usleep (reset_raise);

			/* stop reset */
			c = ((struct slink_1 *)link)->instatus;

			/* wait a little bit */
			usleep (reset_fall);
			break;
		   case BBKV2 :
			/* start reset */
			c = ((struct slink_2 *)link)->startreset;

			/* wait a little bit */
			usleep (reset_raise);

			/* stop reset */
			c = ((struct slink_2 *)link)->stopreset;

			/* wait a little bit */
			usleep (reset_fall);
			break;
		   case VMTM   :
			/* start reset */
			((struct slink_v *)link)->startreset = (byte)0;

			/* wait a little bit */
			usleep (reset_raise);

			/* stop reset */
			((struct slink_v *)link)->stopreset  = (byte)0;

			/* wait a little bit */
			usleep (reset_fall);
			break;
		   case BBKV4   :
			/* start reset */
			((struct slink_4 *)link)->startreset = (byte)0;

			/* wait a little bit */
			usleep (reset_raise);

			/* stop reset */
			((struct slink_4 *)link)->stopreset  = (byte)0;

			/* wait a little bit */
			usleep (reset_fall);
			break;
		   case MTMSUN1   :
		   case MTMSUN2   :
			/* the MTM-SUN is special in the way it differs 
			 * between reset at the link-line and reset of
			 * the link adaptor itself. This is very important
			 * for Super- and MultiCluster access to the NCM
			 * (ringing).
		 	 * At V1.08 this functions models the VMTM behaviour.
		 	 * An additional function should be implemented to
			 * obey the defined ringing procedure (for future).
		 	 */
			/* start reset. First line then adaptor */
			((struct slink_m *)link)->startresetLine  = (byte)0;
			((struct slink_m *)link)->startresetAdapt = (byte)0;

			/* wait a little bit */
			usleep (reset_raise);

			/* stop reset. First line then adaptor */
			((struct slink_m *)link)->stopresetLine  = (byte)0;
			((struct slink_m *)link)->stopresetAdapt = (byte)0;

			/* wait a little bit */
			usleep (reset_fall);
			break;
	   	default :
			break;
		} /* switch (LinkInfo ...) */
		break;
	   case SBUS:
		switch (LinkInfo[LinkId].B_type) {
	   	   case BBKS4 :
			dpr = (struct DpR *)&(SBusSlot[l_info->B_slot].dpr);

			/* compute the address of the T222 <-> SBus
			 * communication registers and set the comm
			 * struct to this address
			 */

 			swap_word      = (word*)swap_bytes;
	
			comm = (struct slink_s *)((l_info->usr_ad)
							+ dpr->com_off);


			regs = (struct BBKS4_regs *)((l_info->usr_ad) +
				 dpr->regs_off+ l_info->L_no * dpr->regs_len);
			read_link_rq0 = (word *)&(regs->readlinkrq[0]);
			read_link_rq1 = (word *)&(regs->readlinkrq[1]);

			write_link_rq0 = (word *)&(regs->writelinkrq[0]);
			write_link_rq1 = (word *)&(regs->writelinkrq[1]);

 			*swap_word    = (word)(RESET_TIME);
			swap 	      = swap_bytes[0];
			swap_bytes[0] = swap_bytes[1];
			swap_bytes[1] = swap;	

			/* set event type */
			comm->stopreset[l_info->L_no] = (word)(RESET);

			/* set startreset flag */
			comm->startreset[l_info->L_no] = (word)(*swap_word);

			tmout = FALSE;      /*!!!!!!!!! without TIMEOUT */
			timecnt = (long)(time_tenth * SBUSRESET);

			/* read event flag */
			while ((comm->startreset[l_info->L_no]) != 0)
				if (tmout) {
				   if (timecnt-- <= 0) {
				      /* operation timed out */

				      if (LinkId == GlobalPacketInfo.LinkId) {
					 /* change link mode to stream mode */
				      	 LinkInfo[LinkId].packetlink = FALSE;
	
			 	     	 /* actualize GlobalPacketInfo */
				     	 GlobalPacketInfo.LinkId      = -1;

				     	 /* set BBK-S4 stream mode flag */
				     	 *LinkMode = STREAMMODE;
				      }

				      return (ESBUSLINKRESET);
				   }
				}
	
			
			timecnt = (long)(time_tenth );
			while (timecnt-->0) {
				if (tmout) {
				   if (timecnt-- > 0) {
				   }
				}
			} /* while (timecnt-- > 0) */

			if (LinkId == GlobalPacketInfo.LinkId) {
			   /* change link mode to stream mode */
			   LinkInfo[LinkId].packetlink = FALSE;
	
			   /* actualize GlobalPacketInfo */
			   GlobalPacketInfo.LinkId      = -1;

			   /* set BBK-S4 stream mode flag */
			   *LinkMode = STREAMMODE;
			}
			break;

		   default :{
			/* board no valid SBus device */
			return (ESBUSNAME);}
		} /* switch (LinkInfo ...) */
		break;
	   default:{
		/* unknown bus type */
		return (EBUSTYPE);}
	} /* switch (l_info->Bus_type) */
	/* everything is fine */

   	return (OK);
}


/*
 *	AnalyseLink
 *
 *	Inmos says: Analyse the subsystem associated with the specified link.
 *	Parsytec boards have the famous feature of beeing individual reset. 
 *	This reset always do the reset/analyse sequence.
 *	In order to offer all interface routines, we do a 'ResetLink' here.
 *	Return values see 'ResetLink'.
 *
 */

int AnalyseLink (LinkId)
int LinkId;
{
	return ( ResetLink (LinkId) );
}


/*
 *	TestError
 *
 *	Parsytec boards do not have any possibility to check the error
 *	status of a neighbour transputer.
 *	In order to offer all interface routines, we return always
 *	zero, which means: No errror set.
 *	Inmos: Returns 1 if error is set, 0 if it is not and a negative
 *	error code otherwise.
 *
 */


int TestError (LinkId)
int LinkId;
{
	/* range check LinkId */
	if ( (LinkId < 0) || (LinkId >= MaxLinkId) ) {
		/* LinkId out of range */
		return (EIDINVAL);
	}

	/* is that link active */
	if ( LinkInfo[LinkId].opened == FALSE ) {
		/* link not active */
		return (ENOTOPEN);
	}

	return (0);	/* No error set */
}


/*
 *	TestRead
 *
 *	Test input status of the link.
 *	Returns 1 if ReadLink will return one byte without timeout,
 *	0 if it may not and a negative error code.
 *
 *	Global variables
 *
 *	struct	SBusSlot  SBusSlot[];
 *	struct	LinkInfo  *LinkInfo;
 *
 */

int TestRead (LinkId)
int LinkId;
{	register caddr_t link;		/* ptr to base of hardware */
	register int mask = READYMASK;
	struct	LinkInfo  *l_info;	/* ptr to LinkInfo[LinkId] */
	struct	DpR	  *dpr;		/* ptr to SBusSlot[...] */
	word	testread;		/* testread flag */



	/* range check LinkId */
	if ( (LinkId < 0) || (LinkId >= MaxLinkId) ) {
		/* LinkId out of range */
		return (EIDINVAL);
	}

	/* is that link active */
	if ( LinkInfo[LinkId].opened == FALSE ) {
		/* link not active */
		return (ENOTOPEN);
	}
	l_info = &(LinkInfo[LinkId]);

	switch (l_info->Bus_type) {
	   case VMEBUS:
		link = LinkInfo[LinkId].usr_ad;

		switch (LinkInfo[LinkId].B_type) {
		   case BBKV1 :
			return ( ((byte)((struct slink_1 *)link)->instatus)
				 & mask);
		   case BBKV2 :
			return ( ((byte)((struct slink_2 *)link)->instatus)
				 & mask);
		   case VMTM :
			return ( ((byte)((struct slink_v *)link)->instatus)
				 & mask);
		   case BBKV4 :
			return ( ((byte)((struct slink_4 *)link)->instatus)
				 & mask);
		   case MTMSUN1 :
		   case MTMSUN2 :
			return ( ((byte)((struct slink_m *)link)->instatus)
				 & mask);
		   default :
			return (ENOTYPE);
		} /* switch (LinkInfo ...) */
	   case SBUS:
		switch (LinkInfo[LinkId].B_type) {
	   	   case BBKS4 :

			dpr = (struct DpR *)&(SBusSlot[l_info->B_slot].dpr);
			
			/* compute the address of the read/write register set */
			link = l_info->usr_ad + dpr->regs_off
				+ l_info->L_no * dpr->regs_len;
			/* read testread flag */
			testread = (word)(((struct BBKS4_regs *)link)
					   ->testread);
			if (testread == (word)(SBUSMASK | 1)){
				return (1);}
			else{
				return (0);}
		   default :{
			/* board no valid SBus device */
			return (ESBUSNAME);}
		} /* switch (LinkInfo ...) */
	   default:{
		/* unknown bus type */
		return (EBUSTYPE);}
	} /* switch (l_info->Bus_type) */
	/* NOTREACHED */
}


/*
 *	TestWrite
 *
 *	Test output status of the link.
 *	Returns 1 if WriteLink can write one byte without timeout,
 *	0 if it may not and a negative error code.
 *
 *	Global variables
 *
 *	struct	SBusSlot  SBusSlot[];
 *	struct	LinkInfo  *LinkInfo;
 */

int TestWrite (LinkId)
int LinkId;
{	register caddr_t link;		/* ptr to base of hardware */
	register int mask = READYMASK;
	struct	LinkInfo  *l_info;	/* ptr to LinkInfo[LinkId] */
	struct	DpR	  *dpr;		/* ptr to SBusSlot[...] */
	word	testwrite;		/* testwrite flag */

	/* range check LinkId */
	if ( (LinkId < 0) || (LinkId >= MaxLinkId) ) {
		/* LinkId out of range */
		return (EIDINVAL);
	}

	/* is that link active */
	if ( LinkInfo[LinkId].opened == FALSE ) {
		/* link not active */
		return (ENOTOPEN);
	}
	l_info = &(LinkInfo[LinkId]);

	switch (l_info->Bus_type) {
	   case VMEBUS:
		link = LinkInfo[LinkId].usr_ad;

		switch (LinkInfo[LinkId].B_type) {
		   case BBKV1 :
			return ( ((byte)((struct slink_1 *)link)->outstatus) 
				 & mask);
		   case BBKV2 :
			return ( ((byte)((struct slink_2 *)link)->outstatus)
				 & mask);
		   case VMTM :
			return ( ((byte)((struct slink_v *)link)->outstatus)
				 & mask);
		   case BBKV4 :
			return ( ((byte)((struct slink_4 *)link)->outstatus)
				 & mask);
		   case MTMSUN1 :
		   case MTMSUN2 :
			return ( ((byte)((struct slink_m *)link)->outstatus)
				 & mask);
		   default :
			return (ENOTYPE);
		} /* switch (LinkInfo ...) */
	   case SBUS:
		switch (LinkInfo[LinkId].B_type) {
	   	   case BBKS4 :
			dpr = (struct DpR *)&(SBusSlot[l_info->B_slot].dpr);
			


			/* compute the address of the read/write register set */
			link = l_info->usr_ad + dpr->regs_off
				+ l_info->L_no * dpr->regs_len;
			/* read testread flag */
			testwrite = (word)(((struct BBKS4_regs *)link)
					   ->testwrite);
			if (testwrite == (word)(SBUSMASK | 1)){

				return (1);}
			else{

				return (0);}
		   default :{

			/* board no valid SBus device */
			return (ESBUSNAME);}
		} /* switch (LinkInfo ...) */
	   default:{

		/* unknown bus type */
		return (EBUSTYPE);}
	} /* switch (l_info->Bus_type) */
	/* NOTREACHED */
}


/*
 *	lock_link
 *
 *	try to lock the supplied LinkId by using a semaphore.
 *	Return OK if locked successfully, a negative error code
 *	otherwise.
 *
 *	Global variables
 *
 */

static
int lock_link (LinkId)
int LinkId;	/* 0..(MaxLinkId-1) */
{	struct sembuf sops;	/* semop command struct */
	int rval;		/* return value for 'semop(2)' */

	/* range check LinkId */
	if ( (LinkId < 0) || (LinkId >= MaxLinkId) ) {
		/* LinkId out of range */
		return (EIDINVAL);
	}

	/* has the set of semaphores beeing opened */
	if ( sem_id < 0 ) {
		/* lock mechanism initialized not successfully */
		return (ESEMINIT);
	}

	sops.sem_num = LinkId;
	sops.sem_op  = -1;			/* decrement semval by one */
	sops.sem_flg = (IPC_NOWAIT | SEM_UNDO);	/* do not block op and
						 * undo it on exit
						 * (e.g. SIGKILL )
						 */
	/* restore the effective gid to VME-bus/SBus access */
	if ( setregid ( -1, vme_gid) != 0 ) {
		/* failed to rstore VME-bus/SBus gid */
		return (ERGID);
	}

	rval = semop (sem_id, &sops, 1);
	/* lock successfully ? */
	if ( rval >= 0) {
		/* all paletti */
		/* back to real gid */
		setregid ( -1, usr_gid);
		return (OK);
	}
	if ( (rval == -1) && (errno == EAGAIN) ) {
		/* sorry, this link is in use at the moment */
		/* back to real gid */
		setregid ( -1, usr_gid);
		return (WUSED);
	}
	/* should not happen */
	/* back to real gid */
	setregid ( -1, usr_gid);
	return (ESEMLOCK);
}


/*
 *	unlock_link
 *
 *	unlocks the specified LinkId.
 *	Return OK in case of success or a negative error code otherwise.
 *
 */

static
int unlock_link (LinkId)
int LinkId;	/* 0..(MaxLinkId-1) */
{	struct sembuf sops;	/* semop command struct */
	int rval;		/* return value for 'semop(2)' */

	/* range check LinkId */
	if ( (LinkId < 0) || (LinkId >= MaxLinkId) ) {
		/* LinkId out of range */
		return (EIDINVAL);
	}
	/* restore the effective gid to VME-bus/SBus access */
	if ( setregid ( -1, vme_gid) != 0 ) {
		/* failed to rstore VME-bus/SBus gid */
		return (ERGID);
	}
	/* before doing the 'signal' operation, check wether
	 * the according semaphore is already 1
	 */
	rval = semctl (sem_id, LinkId, GETVAL);
	if ( rval == -1 ) {
		/* failed to get semval */
		/* back to real gid */
		setregid ( -1, usr_gid);
		return (ESEMUNLOCK);
	}
	if ( rval >= 1) {
		/* this link is already unlocked.
		 * should return a warning, but now OK
		 */
		/* back to real gid */
		setregid ( -1, usr_gid);
		return (OK);
	}
	/* do semaphore 'signal' operation */
	sops.sem_num = LinkId;
	sops.sem_op  = 1;			/* increment semval by one */
	sops.sem_flg = (IPC_NOWAIT | SEM_UNDO);	/* do not block op and
						 * undo it on exit
						 * (e.g. SIGKILL )
						 */
	rval = semop (sem_id, &sops, 1);
	/* lock successfully ? */
	if ( rval == 0) {
		/* all paletti */
		/* back to real gid */
		setregid ( -1, usr_gid);
		return (OK);
	}
	/* should not happen */
	/* back to real gid */
	setregid ( -1, usr_gid);
	return (ESEMUNLOCK);
}


/*
 *	in_use_link
 *
 *	check wether the specified link is allready in use.
 *	The according semaphore is checked.
 *	Return != 0 when currently in use, 0 if unused or
 *	a negative error code.
 */
 
static
int in_use_link (LinkId)
int LinkId;	/* 0..(MaxLinkId-1) */
{	int rval;		/* return value for 'semctl(2)' */

	/* range check LinkId */
	if ( (LinkId < 0) || (LinkId >= MaxLinkId) ) {
		/* LinkId out of range */
		return (EIDINVAL);
	}
	/* restore the effective gid to VME-bus/SBus access */
	if ( setregid ( -1, vme_gid) != 0 ) {
		/* failed to restore VME-bus/SBus gid */
		return (ERGID);
	}
	/* read the semaphore value */
	rval = semctl (sem_id, LinkId, GETVAL);
	if ( rval == -1 ) {
		/* failed to get semval */
		/* back to real gid */
		setregid ( -1, usr_gid);
		return (ESEMINUSE);
	}
	if ( rval >= 1) {
		/* this link is currently unlocked (not in use).
		 */
		/* back to real gid */
		setregid ( -1, usr_gid);
		return (0);
	}
	/* the link is currently in use */
	/* back to real gid */
	setregid ( -1, usr_gid);
	return (INUSE);
}


/*
 *	init_lock
 *
 *	initialize the locking mechanism, which is a system wide
 *	set of semaphores.
 *	When leaving this function successfully, it is garanteed
 *	that a semaphore with sem_link_key exists with the number of
 *	needed semaphores ('nlocks'). The semaphore identifier is stored
 *	in global variable 'sem_id'.
 *	090890 mc v 1.12: Create semaphore with effective gid.
 *
 *	Global variables
 *	int sem_id;
 *	int vme_gid;	RO
 *	int usr_gid;	RO
 *
 */

static
int init_lock (nlocks)
int nlocks;
{	int lsemid;
	int semval;		/* value of init semaphore */
	union semun sem_arg;	/* required as semctl(2) 4th parameter */
	int i;

	/* does this semaphore already exist ? */
	/* permission: read: 4, alter: 2; here: read/alter for user+group,
	 *	read for other
	 */
	/* restore the effective gid to VME-bus access */
	if ( setregid ( -1, vme_gid) != 0 ) {
		/* failed to rstore VME-bus gid */
		return (ERGID);
	}

	lsemid = semget (sem_link_key, nlocks+1, (IPC_CREAT|IPC_EXCL|0664));
	if ( lsemid == -1 ) {
		if ( errno == EEXIST ) {
		   /* semaphore already exists
		    * first: get semid of existing semaphore set
		    */
		   lsemid = semget (sem_link_key, nlocks+1, 0664);
		   if ( lsemid == -1 ) {
			/* should never happen */
			/* back to real gid */
			setregid ( -1, usr_gid);
			return (ESEM2GET);
		   }
		   /* now leave semid global */
		   sem_id = lsemid;
		   
		   /* Must check now wether sem-set is initialized.
		    * Done very dirty here: active waiting for
		    * semaphore initialized set.
		    */
		   for (;;) {
			/* active waiting until all semaphores init. */
			if ( (semval=semctl (sem_id, nlocks, GETVAL)) == -1 ) {
				/* cannot get value of init semaphore */
				/* back to real gid */
				setregid ( -1, usr_gid);
				return (ESEMINIT);
			}
			if ( semval > 0 ) {
				/* now it is sure that all semaphores
				 * have been initialized
				 */
				/* back to real gid */
				setregid ( -1, usr_gid);
				return (OK);
			} else {
				/* active wait until init semaphore == 1 */
				sleep (1);
			} /* else init semval > 0 */
		   } /* for-ever */
		} else {
			/* cannot open/create semaphore */
			/* back to real gid */
			setregid ( -1, usr_gid);
			return (ESEMGET);
		}
	}
	/* semaphore set created new */
	/* now leave semid global */
	sem_id = lsemid;

	/* inilialize all semaphores to semval = 1 */
	sem_arg.val = 1;
	for (i=0; i<nlocks; i++) {
		/* set each semval to 1 */
		if ( semctl (sem_id, i, SETVAL, sem_arg) == -1) {
			/* error initializing value for semaphore i
			 * should never happen, so bad error recovery
			 */
			/* back to real gid */
			setregid ( -1, usr_gid);
			return (ESEMINIT);
		}
	} /* for each semaphore */
	/* finally set semaphore nlocks to 1 to indicate
	 * initialization complete
	 */
	if ( semctl (sem_id, nlocks, SETVAL, sem_arg) == -1) {
		/* error initializing value for init semaphore
		 * should never happen, so bad error recovery
		 */
		/* back to real gid */
		setregid ( -1, usr_gid);
		return (ESEMINIT);
	}
	/* back to real gid */
	setregid ( -1, usr_gid);
	return (OK);
}


/* global variable used only between 'init_timeout()' and 'vt_handler()' */

static short tmcheck;	/* flag wether timeout checking active */
static struct itimerval end_time = {	/* itimer values */
		{ 0L, 0L    },	/* timer intervall (seconds, useconds)
				 * single shot after 0.1 seconds */
		{ 0L, 100000L}	/* 0.1 seconds == 100000 Useconds */
};


/*
 *	init_timeout
 *
 * This interface may be run on a wide class of machines.
 * Especially the CPU performance varies in a wide range.
 * The fastest way for timeout-checking is downcounting
 * a local variable. The timeout values are specified
 * machine-independant in units 0.1 seconds.
 * So there is a machine-specific value which is downcounted
 * in 0.1 seconds. This value ('time_tenth') is computed
 * here using the 'setitimer(2)'.
 * The measurement is not perfectly. The prototype loop
 * assumes that no data transfer happenes (idle loops are
 * timed more accurately than heavily busy loops).
 * A local timeout variable is set to MAXINT. When starting
 * the loop, the 'itimer(2)' is started with virtual-time
 * (which means process-time).
 * After 0.1 seconds the itimer raises a SIGVTALRM signal
 * which is caught in 'vt_handler' and reads out the current
 * timeout value and forces the loop to terminate.
 * Since 1.07 : Care must be taken for integer arithmetics
 * overflow when using the scaling factor 'time_tenth'. On
 * very fast machines with very long timeout-values the
 * multiplication may overflow. To prevent this and avoid
 * overflows at all, compute in global variable 'time_max'
 * the maximum timeout-period in units of 1/10 th seconds.
 * If timeouts are requested for longer times they will be
 * substituted with 'time_max'.
 *
 * returns OK if everything is ok and the passed 'time_tenth'
 * and 'time_max' variables are set.
 * If anything went wrong, only a warning-code is returned
 * and the parameter stays unchanged.
 *
 *	Global variables:
 *
 *	tmcheck		R	control downcounting loop
 *
 *	Error codes:
 *
 *
 */

static
int init_timeout (tenth, maxtime)
int *tenth;
int *maxtime;
{	int timecnt;			/* timeout loop down counter */
	register caddr_t link;		/* ptr to base of hardware */
	register int mask = READYMASK;
	struct slink_v dummy;		/* dummy struct for access */

	/* functions called from this module */
	void vt_handler ();

	/* install handler for SIGVTALRM first */
	if ( signal (SIGVTALRM, vt_handler) == (void (*)()) -1 ) {
		/* failed to install SIGVTALRM handler
		 * should never happen
		 */
		return (WSIGVTINSTALL);
	}
	/* initialise local variables */
	timecnt = MAXINT;	/* set local timeout variable to MAXINT */
	tmcheck = TRUE;		/* while (tmcheck) <do_checking> */
	dummy.instatus = 0;	/* asure while condition is TRUE */
	link    = (caddr_t) &dummy;

	/* start itimer */
	if ( setitimer (ITIMER_VIRTUAL, &end_time, (struct itimerval *) 0)) {
		/* failed to sety itimer */
		return (WSETITIMER);
	}

	/* fall into prototype poll-loop */
	while (( ((byte)((struct slink_v *)link)->instatus) & mask) == 0) {
		/* no byte ready for input */
		if ( tmcheck ) {
			/* in real loop also done */
			if ( timecnt-- <= 0 ) {
				/* operation timed out */
				return (WTIMEOUTFAIL);
			} /* if timecnt-- */
		} else {
			/* vt_handler has been called by SIGVTALRM
			 * That means 0.1 seconds in virtual process
			 * time has elapsed.
			 * Leave the while loop
			 */
			break;
		} /* else tmcheck */
#ifdef	IPX
		getpid();	/* simulate uncache */
#endif

	} /* while instatus not_ready */

	/* Compute the number of iterations
	 * in the loop above
	 */
	*tenth = (MAXINT - timecnt);

	/* compute the maximum timeout-period in units of
	 * 1/10 th seconds to avoid integer overflows.
	 */
	*maxtime = (MAXINT / *tenth) - 1;
	return (OK);
}


/*
 *	vt_handler
 *
 *	Handles SIGVTALRM for init_timeout()' function some
 *	lines above. It is called by the 'itimer(2)' after 0.1 seconds
 *	virtual process-time have elapsed.
 *	The only action taken is to set a flag (for down-counting loop)
 *	in 'init_timeout()' to false.
 *
 *	Global variables used
 *
 *	tmcheck		W	Set to FALSE
 */

static
void vt_handler (sig, code, scp, addr)
int sig, code;
struct sigcontext *scp;
char *addr;
{
	tmcheck = FALSE;
}


/*
 *	config_link_type
 *
 *	Determines, wether the according link (as specified by 'LinkId')
 *	may be used for the configuration of a transputer-network topology.
 *	At V1.11 VMTM and MTM-SUN boards with onboard and additional ncu
 *	are supported. All possible values are defined in 'link.h'
 *	for the 'struct ext_link_info' component 'config_type' as CONF_*.
 *	The defined return values are:
 *	  CONF_NULL	0	not concerned at all with configuration
 *	  CONF_VMTM	1	link is connected with a transputer, which
 *				has on its link 1 a link-switch (C004) attached.
 *	  CONF_MTMSUN1	2	this link is connected to the config-in link
 *				(link 4) of a MTM-SUN with onboard ncu.
 *	  CONF_MTMSUN2	3	this link is connected to the config-in link
 *				(link 8) of a MTM-SUN with an additional ncu.
 *
 *	Global variables
 *
 *      struct LinkInfo LinkInfo[LinkId];
 */

static
int config_link_type (LinkId)
int LinkId;
{
	/* this is an internal function. LinkId not checked */

	/* check for VMTM board and link 0 */
	if ( (LinkInfo[LinkId].B_type == VMTM) &&
	     (LinkInfo[LinkId].L_no   == 0) ) {
		/* link 0 of a VMTM board */
		return (CONF_VMTM);
	}

	/* check for MTM-SUN1 board (1 ncu) and link 4 */
	if ( (LinkInfo[LinkId].B_type == MTMSUN1) &&
	     (LinkInfo[LinkId].L_no   == 4) ) {
		/* link 4 of a MTM-SUN1 board */
		return (CONF_MTMSUN1);
	}

	/* check for MTM-SUN2 board (2 ncu's) and link 8 */
	if ( (LinkInfo[LinkId].B_type == MTMSUN2) &&
	     (LinkInfo[LinkId].L_no   == 8) ) {
		/* link 8 of a MTM-SUN2 board */
		return (CONF_MTMSUN2);
	}
	/* none of the configuration links */
	return (CONF_NULL);
}



/*
 *	InfoLink
 *
 *	Local extension done by Parsytec. This function gives
 *	information about all the link hardware on this machine.
 *	It is passed the address of an pointer to the 'ext_link_info'
 *	structure. If this pointer is NULL, enough memory is allocated
 *	and the pointer is changed to point onto it. Then the array is
 *	filled with information. Here a short description:
 *
 *		link_number	is the index in this array and the
 *				corresponding LinkId of this link.
 *		in_use		0 if link not in use, >0 if currently
 *				used (as recorded by the semaphore)
 *				or <0 if semaphore access has failed
 *		config_type	0 if not usable for configuration (of
 *				transputer topology), >0 if the according
 *				link may be configured. The value is one
 *				of the CONF_* defined in 'link.h'.
 *		link_name	is the link name as string, formated:
 *				"nnnnnnnn bxx lx"
 *					n	board name  (%-8s)
 *					bxx	board number (%2d)
 *					lx	link number on board
 *							(%1d)
 *		link_address	the VME-bus/SBus address of this link
 */
 
int InfoLink (LinkPtr)
struct ext_link_info  **LinkPtr;
{	int id, lerr;
	struct LinkInfo *l_info;
	struct ext_link_info *x_info;
	char l_name[16];	/* local link name: ext_link_info.link_name */

	/* referenced functions */
	int InitLink ();
	int in_use_link ();
	int config_link_type ();
	
	
	/* check wether LinkInfo contains valid info */
	if ( MaxLinkId < 0 ) {
		/* no yet, initialise LinkInfo */
		if ((lerr=InitLink ()) != OK) {
			/* failed to initilize link interface */
			return (lerr);
		}
	}
	
	/* is the users 'ext_link_info' allready allocated ? */
	if ( *LinkPtr == NULL ) {
		/* no, so allocate memory for it */
		*LinkPtr = (struct ext_link_info *) malloc (
				MaxLinkId * sizeof (struct ext_link_info));
		if ( *LinkPtr == NULL ) {
			/* allocation failed */
			return (ENOMEMINFO);
		}
		/* have successfully allocated memory for
		 * an array of struct 'ext_link_info'
		 */
	}
	
	/* fill in 'ext_link_info' components */
	for (l_info=LinkInfo, x_info=*LinkPtr, id=0; id<MaxLinkId; id++) {
		x_info->link_number  = id;
		x_info->in_use       = in_use_link (id);
		x_info->config_type  = config_link_type (id);
		sprintf (l_name, "%-8s b%-2hd l%1hd", 
			std_b_name[l_info->B_type], l_info->B_no, l_info->L_no);
		strcpy (x_info->link_name, l_name);
		x_info->link_address = (char *)l_info->vme_addr;
		x_info->slot_no      = (int) l_info->B_slot;
		x_info++;
		l_info++;
	}
	/* and now, and then: return number of links in struct */
	return (MaxLinkId);
}


/*
 *	lerror
 *
 *	Return a pointer to a string giving the explanation of
 *	the error-code supplied as a parameter.
 *	Usually error-codes are negative values. The positive
 *	error code will be accepted as well.
 *	In case of an internal error (error-code out of range)
 *	the returned pointer point to an according error message.
 *
 *	Global variables
 *	char *(err_msg[]);
 */
 
char *lerror (err_code)
int err_code;
{
	/* map given code into positive values and range check */
	if ( err_code < 0 ) {
		err_code = -err_code;
	}
	if ( (err_code < 0) ||
	     (err_code > (sizeof (err_msg) / sizeof (char *))) ){
		/* supplied error code out of range */
		return ("lerror: The error code is out of valid range");
	} else {
		return ( err_msg[err_code] );
	}
}



