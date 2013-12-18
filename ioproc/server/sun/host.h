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


/* $Id: host.h,v 1.1 1994/06/29 13:46:19 tony Exp $ */

#ifndef HOST_H
#define HOST_H 1

#define BOARD		(('V'<<24)|('C'<<16)|('4'<<8)|('0'))

typedef struct{
	int WhatToDo;
	unsigned long Parameters[14];
	int InterruptSemaphore;
	} HostMessage;

typedef struct{
	int IntNum;
	int IntVector;
	} HostIntStructure;

#define HostBuffLength	(0x400)
typedef unsigned long *HostDataBuff;

#ifndef HYDRA
	/* These are the Hydra reply codes */
	/* When Hydra completes a request, */
	/* it will replace the request code with one of these codes */
	/* to indicate the completion status of the request */
#define Success	(1)
#define Failure	(0)

	/* These are used in conjunction with SetProperty(Daughter) and 
         * GetProperty(daughter) */
	/* to indicate  wether the daughter card is present or not */
#ifndef TRUE
#   define TRUE	(1)
#endif
#ifndef FALSE
#   define FALSE (0)
#endif
#endif

/* These are the request codes for the various Hydra host services */
enum {
    BootADsp = 0x2, CopyStuff = 0x3, Run = 0x4, Halt = 0x5, GetProperty = 0x6,
    HostIntNumber = 0x9, HostIntVector = 0x10, DisableKeyInt = 0x11, 
    EnableKeyInt = 0x12, UserInt = 0x13, READFLASH = 0x20, WRITEFLASH = 0x21,
    ERASEFLASH = 0x23,
};
#define ERASE_PASSWORD 0xdeadbead	/* can't be enum in DOS */

/* These are the codes that specify the various properties that are Gettable 
 * and Settable */
enum {
    UartABaud = 0x80, UartAParity = 0x81, UartABits = 0x82, UartBBaud = 0x83,
    UartBParity = 0x84, UartBBits = 0x85, DRAMSize = 0x86, CpuClock = 0x87,
    LocalSRAM1Size = 0x88, LocalSRAM2Size = 0x89, LocalSRAM3Size = 0x8A,
    LocalSRAM4Size = 0x8B, GlobalSRAM1Size = 0x94, GlobalSRAM2Size = 0x95,
    GlobalSRAM3Size = 0x96, GlobalSRAM4Size = 0x97, DRAMBase = 0x8C,
    HostJTAGBase = 0x8D, LocalJTAGBase = 0x93, Daughter = 0x8E,
    IPCRBase = 0x8F, BoardName = 0x90, Firmware = 0x91, Hardware = 0x92,
    HostJTAGSpace = 0x98, DRAMSpace = 0x99, MotherSerial = 0x9A,
    DRAMSerial = 0x9B, DSPSerial = 0x9C,
};

/* These are used to specify the parity type */
#define EVEN	(2)
#define ODD	(1)

/* VME Address spaces */
#define	VME_A32	(0)
#define	VME_A24	(1)
#define	VME_A16	(2)

#endif /* #ifndef HOST_H */
