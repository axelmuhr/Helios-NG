typedef struct{
	int WhatToDo;
	unsigned long Parameters[15];
	} HostMessage;

#define HostBuffLength	(0x400)
typedef unsigned long *HostDataBuff;

#ifndef HYDRA
	/* These are the Hydra reply codes */
	/* When Hydra completes a request, */
	/* it will replace the request code with one of these codes */
	/* to indicate the completion status of the request */
#define Success	(1)
#define Failure	(0)

	/* These are used in conjunction with SetProperty(Daughter) and GetProperty(daughter) */
	/* to indicate  wether the daughter card is present or not */
#define TRUE		(1)
#define FALSE		(0)
#endif

/* These are the request codes for the various Hydra host services */
#define BootADsp	(2)
#define CopyStuff	(3)
#define Run		(4)
#define GetProperty	(5)
#define SetProperty	(6)
#define Test		(7)

/* These are the codes that specify the various properties that are Gettable and Settable */
#define UartABaud	(0x80)
#define UartAParity	(0x81)
#define UartABits	(0x82)
#define UartBBaud	(0x83)
#define UartBParity	(0x84)
#define UartBBits	(0x85)
#define DRAMSize	(0x86)
#define CpuClock	(0x87)
#define SRAM1Size	(0x88)
#define SRAM2Size	(0x89)
#define SRAM3Size	(0x8A)
#define SRAM4Size	(0x8B)
#define DRAMBase	(0x8C)
#define JTAGBase	(0x8D)
#define Daughter	(0x8E)
#define IPCRBase	(0x8F)

/* These are used to specify the parity type */
#define EVEN	(2)
#define ODD	(1)
