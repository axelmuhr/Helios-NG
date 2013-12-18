/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1993, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      parsy/link.h                                                    --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: link.h,v 1.1 1993/09/28 14:23:58 bart Exp $ */

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
** !! THIS CODE IS NOT PART OF THE STANDARD SOURCE RELEASE. IT SHOULD BE!!
** !! USED ONLY IN-HOUSE AND FOR ANY PARSYTEC SOURCE RELEASES.          !!
** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
**/

/*
 * link.h		VME bus/SBus transputer boards definitions
 *
 * Copyright (C) Parsytec GmbH, 1989.
 * Copyright (C) CWA mbH, 1991 (for all parts concerning SBUS/BBK-S4).
 *
 * Author:      Matthias Clauss
 *
 *		Michael Raus (all parts concerning SBUS/BBK-S4)
 *
 *
 * Date:        13.6.1989        Created
 *
 * Version:     1.08a
 *
 * Updates:     Vers    date    author  description
 *		1.01	1.8.1989 mc	automatic timeout warnings added
 *		1.02	3.8.1989 mc	configuration support added
 *		1.03   29.1.1990 mc	release gid additional error
 *		1.04   27.6.1990 mc	mtm-sun configuration added
 *
 *		1.05  	290791	 mr	support of SBus devices added
 *		1.06	230192	 mr	error codes for packet protocol
 *		1.07	230192	 lt	error codes for packet protocol
 *		1.08	180592	 mc	packet definition, info.slot_no
 *              1.08a   240993   blv    integrated with I/O Server
 *
 */

#ifndef HELIOS_IOSERVER
static char *sccs_id="@(#)link.h	1.2 dated 5/19/92";		/* sccs id */
#endif

/*
 * the array of structure ext_link_info contains the link
 * informations after the call to InfoLink.
 */
struct	ext_link_info	{
	int	link_number;
	int	in_use;		/* this links semaphore is locked */
	int	config_type;	/* If > 0 this link is a configuration link.
				 * The value describes the board type.
				 * A MTM-SUN1 has the build-in ncu, a
				 * MTM-SUN2 one additional ncu (total 2 ncus).
				 * The table lists the flagged link:
				 *   board	link
				 *   =========================
				 *   VMTM	L0
				 *   MTM-SUN1	L4
				 *   MTM-SUN2	L8
				 */
	char	link_name[16];
	char	* link_address;	/* VME-bus address of link-adaptor */
	int	slot_no;	/* SBus slot number (0..3), else -1 */
};

/* values for struct ext_link_info component 'config_type' */

#define	CONF_NULL	0
#define	CONF_VMTM	1
#define	CONF_MTMSUN1	2
#define	CONF_MTMSUN2	3

#define	CONF_LAST	3	/* value of last defined entry */


#define	VERS_NO		0x0103	/* current FirmwareVersion 1.3 */
#define	SBUSRESET	30	/* time of Sbus link reset in 1/10 seconds */

#if (!defined(HELIOS_IOSERVER)) || (!defined(Linklib_Module))
typedef unsigned char     byte;
#endif

/*
 * 	Packet definition
 *
 * Packets are introduced with Version 1.3 to support a T9000 style
 * link-protocoll.
 * Some words to prevent confusion with the packet length:
 * 'length' contains the size of the data, pointed to by 'body'.
 * Anyway the number of bytes transferred through the physical link-line
 * will be 'length' + 4 + 1. Why this?
 * The first byte is the 'length' byte exactly as specified.
 * The next 4 bytes (whatever 'length' contains, even if == 0) are the
 * Parix header, giving the destination of this packet.
 * Then comes the 'length' amount of data bytes. So: The shortest packet
 * which may be send is five bytes on the link-line.
 */

struct	packet  {
	byte	length;		/* Packet header: bits 0..6 : body length
				 *		  bit  7    : EOM/EOP tag */
	long	pxhdr;		/* Parix header: 2 byte proc-id,
				 *		 2 byte channel-no. */
	byte	*body;		/* Packet body */
};

#ifdef HELIOS_IOSERVER
	/* Avoid name space pollution and possible clashes	*/
#define OpenLink	parsy_OpenLink
#define CloseLink	parsy_CloseLink
#define ReadLink	parsy_ReadLink
#define WriteLink	parsy_WriteLink
#define ResetLink	parsy_ResetLink
#define AnalyseLink	parsy_AnalyseLink
#define TestError	parsy_TestError
#define TestRead	parsy_TestRead
#define TestWrite	parsy_TestWrite
#define InfoLink	parsy_InfoLink
#define lerror		parsy_lerror
#define PacketMode	parsy_PacketMode
#define StreamMode	parsy_StreamMode
#define GetPacket	parsy_GetPacket
#define PutPackets	parsy_PutPackets
#define InfoPacket	parsy_InfoPacket
#endif

/* external function declarations */

extern	int OpenLink  ();	/* (Name); */
extern	int CloseLink ();	/* (LinkId); */
extern	int ReadLink  ();	/* (LinkId, Buffer, Count, Timeout); */
extern	int WriteLink ();	/* (LinkId, Buffer, Count, Timeout); */
extern	int ResetLink ();	/* (LinkId); */
extern	int AnalyseLink ();	/* (LinkId); */
extern	int TestError ();	/* (LinkId); */
extern	int TestRead  ();	/* (LinkId); */
extern	int TestWrite ();	/* (LinkId); */
extern  int InfoLink  ();	/* (struct ext_link_info **LinkPtr) */
extern  char *lerror  ();	/* (error_code) */

/* packet-functions added for PARIX V1.30 */
extern	int PacketMode ();	/* (LinkId, PacketSize) */
extern	int StreamMode ();	/* implicit parameter: The packet-link */
extern	int GetPacket ();	/* (char *Buffer) */
extern	int PutPackets ();	/* (struct packet *Packets, u_char Count) */
extern	struct packetInfo *InfoPacket ();

/* error codes */
/*
 * Warning: Do not use error numbers <= WARNING as defined below !
 *
 */


#define	OK		1
#define	ECONFIG		-1	/* error reading config file */
#define	ELNAME		-2	/* link adaptor name incorrect */
#define	ESTAT		-3	/* cannot 'stat(2)' config file */
#define	EGETGID		-4	/* cannot get gid for vmebus/sbus group */
#define	EGID		-5	/* config file not in vmebus/sbus group */
#define	EACCESS		-6	/* config file has wrong permission */
				/* write for other */
#define	EOPENR		-7	/* open for read of config file failed */
#define	ESYNTAX		-8	/* config file syntax error */
#define	E1NOEOF		-9	/* pass 1 not at EOF of config file */
#define	EREWIND		-10	/* pass 1 cannot rewind config file */
#define	EMEM		-11	/* mem allocation failed for LinkInfo */
#define	EBNO		-12	/* board number multiple defined in */
				/* config file */
#define	ECLOSE		-13	/* error closing config file */
#define	EBNAME		-14	/* board type name incorrect */
#define	EBNUM		-15	/* board number out of range */
				/* valid range 0..MaxLinkId */
#define	ELNUM		-16	/* link number out of range */
				/* valid range 0..MAX_BOARD_LINKS */
#define	ENAMSYNTAX	-17	/* link name syntax error */
#define	EBOARD3PAR	-18	/* link name 3 params: 2nd not board-no */
#define	ELINK3PAR	-19	/* link name 3 params: 3rd not link-no */
#define	ELOCKCLASH	-20	/* have locked link but failed to unlock */
#define	ENONEXIST	-21	/* link does not exist */
#define	EVMEDEVICE	-22	/* failed to open VME-bus device */
#define	EMMAP		-23	/* failed to mmap page to VME-bus */
#define	EUNMAP		-24	/* failed to munmap page to VME-bus */
#define	EIDINVAL	-25	/* LinkId out of valid range */
#define	ENOTOPEN	-26	/* LinkId not opened */
#define	ENOTYPE		-27	/* Unknown board type (Read/WriteLink) */
#define	EVMENOTOPEN	-28	/* alive(): vme_device not opened yet */
#define	EVMESEEK	-29	/* alive(): cannot lseek(2) on VME-bus */
#define	ENOTPRESENT	-30	/* link adaptor not present on VME-bus */
#define	ESEMGET		-31	/* cannot get unique sem_key (lock_init) */
#define	ESEMLOCK	-32	/* cannot lock semaphore */
#define	ESEMUNLOCK	-33	/* cannot unlock semaphore */
#define	ESEMINIT	-34	/* error during semaphore initialization */
#define	ESEM2GET	-35	/* cannot get existing sem_key */
#define	EINILOCK	-36	/* 'lock_init' failed (init sems failed) */
#define	ENOTALIVE	-37	/* access to link hardware failed */
#define	ENOMEMINFO	-38	/* LinkInfo failed to allocate memory */
#define	ESEMINUSE	-39	/* cannot read semaphore value */
#define	ERGID		-40	/* cannot restore real group-id */
#define ESBUSNAME	-41	/* board no valid SBus device */
#define ESBUSDEVICE	-42	/* failed to open SBus device */
#define EBUSTYPE	-43	/* unknown bus type */
#define	ESBUSNOTOPEN	-44	/* alive(): sbus_device[.] not opened yet */
#define ESBUSSEEK	-45	/* alive(): cannot lseek(2) on SBus */
#define ESBUSNOTALIVE	-46	/* link not accessable on SBus */
#define	EREADLINK	-47	/* ReadLink(): error accessing DpR */
#define	EWRITELINK	-48	/* WriteLink(): error accessing DpR */
#define ESBUSLINKRESET  -49	/* Serious: cannot reset SBus link */
#define ESBMEM		-50	/* mem allocation failed for SBusSlot */
#define	EBOARDINVAL	-51	/* board number out of valid range */
#define	ESBUSWRITEEVENT	-52	/* Serious: cannot get write permission */
#define	ENOPMODE	-53	/* LinkId does not support packet protocol */
#define	EBIGPACKET	-54	/* illegal packet size (negativ or too big) */
#define	EFIRMHANG	-55	/* internal error: Firmware is hanging */
#define	EPACKNOTLONG	-56	/* illegal packet size (no long transfer) */
#define	EPACKLINKOCC	-57	/* another link still used as packet link */
#define	ENOPALINK	-58	/* there is no open PacketLink */
#define ENOSTREAMLINK	-59	/* specified link is no stream link */
#define EPACKBODYALL	-60	/* illegal packet body allignment */
#define	ENOTALLIGN	-61	/* illegal buffer allignment */
#define	EPACKLONG	-62	/* packet body too long */
#define	EREADNOTSUPP	-63	/* ReadLink not supported in packet protocol */
#define	ERNOVERSION	-64	/* Timeout reading firmware version */
#define	ERWRONGVERS	-65	/* firmware has a wrong version number */
#define	EINUSE		-66	/* this link is in use */

/* warning codes */

#define	WARNING		-1000	/* start of warning number. Going down */
#define	WEXISTS		-1000	/* LinkInfo already existant */
#define	WUSED		-1001	/* transputer link locked */
#define	WSIGVTINSTALL	-1002	/* failed to install SIGVTALRM handler */
#define	WSETITIMER	-1003	/* failed to start 'itimer(2)' */
#define	WTIMEOUTFAIL	-1004	/* timeout measurement caught no signal */

/* some usefull constants */
#ifndef FALSE
#define	FALSE	0
#endif
#ifndef TRUE
#define	TRUE	(-1)
#endif
#define	ANY	((short) -1)	/* any board-type, -number, link-number */
#define	INUSE	1

#define	Public
