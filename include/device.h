/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   K E R N E L                        --
--                     -------------------------                        --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- device.h								--
--                                                                      --
--	Machine independant device interface.				--
--	This is not intended to be a full-blown device system, no	--
-- 	provision is made here for multiple clients, or for making	--
--	the device accessible to more than one client. Rather this is	--
--	designed to encapsulate only the hardware specific parts of a	--
--	device. Sharing the device between clients, and converting to	--
--	a GSP interface is the job of the device's client which should	--
--	generally be a Helios server of some sort.			--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: device.h,v 1.8 1992/11/25 13:17:45 nick Exp $ */

#ifndef __device_h
#define __device_h

#ifndef __helios_h
#include <helios.h>
#endif

#include <syslib.h>
#include <queue.h>
#include <ioevents.h>

/* Device Code Structure						*/

typedef struct Device {
	word		Type;		/* module type = T.Device	*/
	word		Size;		/* size of device in bytes	*/
	char		Name[32];	/* device name			*/
	word		Id;		/* not used 			*/
	word		Version;	/* version number of this device*/
	RPTR		Open;		/* offset of open routine	*/
} Device;

#define DeviceWord_(d,field) MWord_(d,offsetof(Device,field))
#define DeviceName_(dest,d) MData_(dest,d,offsetof(Device,Name),32)
#define DeviceOpen_(d) MRTOA_(MInc_(d,offsetof(Device,Open)))

/* Device Control Block							*/
/* This is intended to be an initial sub-structure of a larger device	*/
/* specific control block.						*/

typedef struct DCB {
	MPtr		Device;		/* pointer to Device struct 	*/
	VoidFnPtr	Operate;	/* action entry point		*/
	WordFnPtr	Close;		/* close device routine		*/
	Stream		*Code;		/* open code stream 		*/
} DCB;

/* Generic Device Request						*/
/* Again this is intended to be an initial sub-structure of something	*/
/* larger.								*/
/* In general there will be at least one more layer on top of this to	*/
/* define things like generic disc, serial and event devices.		*/
/* A further layer may be imposed by the client to store its own	*/
/* transaction information.						*/

typedef struct DevReq
{
	Node		Node;		/* queueing node		*/
	word		Request;	/* request type			*/
	word		Result;		/* error code/result		*/
	VoidFnPtr	Action;		/* termination action		*/
	word		SubDevice;	/* internal sub-device		*/
	word		Timeout;	/* timeout, or -1 == forever	*/
} DevReq;

/* Serial Device Request						*/
/* For a successful Write Actual will always equal Size, a Read is	*/
/* allowed to return successfully with less data than requested.	*/

typedef struct SerialReq
{
	DevReq		DevReq;		/* common reqest structure	*/
	word		Size;		/* tfr size			*/
	void		*Buf;		/* buffer			*/
	word		Actual;		/* data actually transferred	*/
} SerialReq;

/* Disc Device Request							*/
/* Pos and Size are in bytes, and should be multiples of the disc's	*/
/* natural block size. For sucessful requests Actual will always be	*/
/* equal to Size.							*/

typedef struct DiscReq
{
	DevReq		DevReq;		/* standard device request	*/
	word		Pos;		/* tfr start position		*/
	word		Size;		/* tfr size			*/
	void		*Buf;		/* buffer			*/
	word		Actual;		/* data actually transferred	*/
	Semaphore	WaitLock;	/* client waits here		*/
} DiscReq;

/* Disc SubUnit open/close Request                                      */
/* This is used to open/close a disc subunit for reading & writing      */

typedef struct DiscOpenCloseReq
{
	DevReq		DevReq;		/* standard device request	*/
	Semaphore	WaitLock;	/* client waits here		*/
} DiscOpenCloseReq;

/* Disc Parameter Change Request                                        */
/* This is used to change the operating parameters of a disc.  It is    */
/* used by UFS where the disc parameters are maintained on the disc     */
/* itself.  The minimal settings are given to the device driver in      */
/* order to read the disk label, and then this request is made to allow */
/* the disc device driver to operate optimally.                         */

typedef struct DiscParameterReq
{
	DevReq		DevReq;		/* standard device request	*/
	Semaphore	WaitLock;	/* client waits here		*/
	word		DriveType;	/* type of drive		*/
	word		SectorSize;	/* size of physical sectors	*/
	word		SectorsPerTrack;/* sectors per track		*/
	word		TracksPerCyl;	/* tracks per cylinder		*/
	word		Cylinders;	/* cylinders on disc		*/
	word		RPM;		/* Rotational Speed		*/
	word		Interleave;	/* Hardware sector interleave	*/
	word		TrackSkew;	/* Sector 0 skew, per track	*/
	word		CylSkew;	/* Sector 0 skew, per cyl	*/
	word		HeadSwitch;	/* Head switch time, usec	*/
	word		TrackSeek;	/* Track-to-track seek, usec	*/
} DiscParameterReq;

/* Format Cylinder Request						*/
/* This is used to format a group of cylinders on disc. Interleave gives*/
/* the sector interleave on a single track. The two skew parameters give*/
/* the inter-track and inter-cylinder offsets. 				*/

typedef struct FormatReq
{
	DevReq		DevReq;		/* standard device request	*/
	word		StartCyl;	/* first cylinder to format	*/
	word		EndCyl;		/* last cylinder to format	*/
	word		Interleave;	/* block interleave		*/
	word		TrackSkew;	/* inter-track skew		*/
	word		CylSkew;	/* inter-cylinder skew		*/
	Semaphore	WaitLock;	/* client waits here		*/
} FormatReq;

/* Eventing Device Get/Set Info						*/
/* This allows the client to setup device specific parameters		*/
/* (e.g. mouse sensitivity, resolution etc.				*/

typedef struct EventInfoReq {
	DevReq		DevReq;		/* common request structure	*/
	void		*Info;		/* pointer to info (dev spec)	*/
} EventInfoReq;

/* Eventing Device Request						*/
/* A number of these requests should be passed to the device, one is	*/
/* returned each time an event occurs.					*/

typedef struct EventReq {
	DevReq		DevReq;		/* common request structure	*/
	IOEvent		IOEvent;	/* event which occured		*/
} EventReq;


/* Network device requests */

typedef struct NetDevReq
{
        DevReq          DevReq;         /* device request               */
        word            Size;           /* transfer size                */
        void            *Buf;           /* data buffer                  */
        word            Actual;         /* data actually transferred    */
} NetDevReq;

typedef struct NetInfo
{
        word            Mask;           /* bitmask                      */
        word            Mode;           /* interface mode               */
        word            State;          /* current state                */
        byte            Addr[8];        /* node address                 */
} NetInfo;

#define NetInfo_Mask_Mode       1
#define NetInfo_Mask_State      2
#define NetInfo_Mask_Addr       4

typedef struct NetInfoReq
{
        DevReq          DevReq;         /* device request               */
        NetInfo         NetInfo;        /* info                         */
} NetInfoReq;

/* macro to call device Operate entry					*/
#define Operate(dcb,req)	((dcb)->Operate)(dcb,req)

extern DCB    *OpenDevice(string name,void *info);
extern word   CloseDevice(DCB *dcb);


/* This macro should be used to construct strings in local or malloced	*/
/* RAM instead of using string constants, which may not be accessible on*/
/* some machines in the absence of a static data area.			*/

#define SetString_(buf,ix,a,b,c,d) (((int *)(buf))[ix] =\
					 (a) | (b)<<8 | (c)<<16 | (d)<<24)



typedef struct FileSysInfo {
	RPTR		DeviceName;		/* disc controller	*/
	RPTR		FsCode;			/* code of file system	*/
	RPTR		Volumes;		/* list of volumes	*/
	word		BlockSize;		/* size of blocks	*/
	word		CacheSize;		/* size of cache in K	*/
	word		SyncOp;			/* synchronous writes?	*/
	word		MaxInodes;		/* incore inodes	*/
	word		SmallPkt;		/* size of small pkt	*/
	word		MediumPkt;		/* size of medium pkt	*/
	word		HugePkt;		/* size of huge pkt	*/
	word		MaxPktSize;		/* == HugePkt		*/
	word		SmallCount;		/* # of small pkts	*/
	word		MediumCount;		/* # of medium pkts	*/
	word		HugeCount;		/* # of huge pkts	*/
	word		PossIndir;
	word		PossDir;
	word		MinGood;
	word		BitMapErrs;
} FileSysInfo;

typedef struct VolumeInfo {
	RPTR		Next;			/* link to next volume	*/
	RPTR		Name;			/* volume name		*/
	RPTR		Partitions;		/* list of partitions	*/
	word		CgSize;			/* cylinder group size	*/
	word		CgCount;		/* # of cylinder groups */
	word		CgOffset;		/* cg skew		*/
	word		MinFree;		/* min free blocks	*/
	word		Type;			/* raw or structured	*/
} VolumeInfo;

/* Values for VolumeInfo->Type */
#define vvt_raw 1
#define vvt_structured 2

typedef struct Partition {
	RPTR		Next;			/* link to next partition	*/
	word		Partition;		/* partition index in device	*/
} Partition;


typedef struct DiscDevInfo {
	RPTR		Name;			/* name of driver		*/
	word		Controller;		/* address of controller	*/
	word		Addressing;		/* unit of Pos and size		*/
	word		Mode;			/* flag/mode bits		*/
	RPTR		Drives;			/* list of drive info structs	*/
	RPTR		Partitions;		/* list of partitions		*/
} DiscDevInfo;

typedef struct DriveInfo {
	RPTR		Next;			/* next drive in list		*/
	word		DriveId;		/* Id of drive in controller	*/
	word		DriveType;		/* type of drive		*/
	word		SectorSize;		/* size of physical sectors	*/
	word		SectorsPerTrack;	/* sectors per track		*/
	word		TracksPerCyl;		/* tracks per cylinder		*/
	word		Cylinders;		/* cylinders on disc		*/
} DriveInfo;

typedef struct PartitionInfo {
	RPTR		Next;			/* next partition in device	*/
	word		Drive;			/* drive number			*/
	word		StartCyl;		/* first cyl of partition	*/
	word		EndCyl;			/* last cyl of partition	*/
	word		StartSector;		/* first sector of partition	*/
} PartitionInfo;

typedef struct SerialInfo {
	RPTR		DeviceName;		/* name of device code	*/
	RPTR		ServerName;		/* name of server code	*/
	RPTR		Name;			/* name of server in NT	*/
	RPTR		Lines;			/* list of lines	*/
	word		Address;		/* device base address	*/	
} SerialInfo;

typedef struct Line {
	RPTR		Next;			/* next line in device	*/
	RPTR		Name;			/* line name in server	*/
	word		Offset;			/* offset in device h/w	*/
} Line;

typedef struct EventInfo {
	RPTR		DeviceName;		/* name of device code	*/
	RPTR		ServerName;		/* name of server code	*/	
	RPTR		Name;			/* server name in NT	*/
	word		Address;		/* device base address	*/
} EventInfo;

typedef struct NetDevInfo {
	RPTR		Device;			/* name of device code	*/
	word		Controller;		/* address of controller*/
	word		Mode;			/* device mode		*/
	word		State;			/* device state		*/
	byte		Address[8];		/* device network address*/
} NetDevInfo;

typedef struct InfoNode {
	RPTR		Next;			/* next in list		*/
	word		Type;			/* node type		*/
	RPTR		Name;			/* node name		*/
	RPTR		Info;			/* node info		*/
} InfoNode;

#define	Info_FileSys	1
#define Info_DiscDev	2
#define Info_Serial	3
#define Info_Event	4
#define Info_Net	5

#endif

/* -- End of device.h */
