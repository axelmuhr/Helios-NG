#define BOARD		(('V'<<24)|('C'<<16)|('4'<<8)|('0'))
#define FIRMWARE_REV	((' '<<24)|('0'<<16)|('.'<<8)|('3'))


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

	/* These are used in conjunction with SetProperty(Daughter) and GetProperty(daughter) */
	/* to indicate  wether the daughter card is present or not */
#define TRUE		(1)
#define FALSE		(0)
#endif

/* These are the request codes for the various Hydra host services */
#define BootADsp		(0x2)
#define CopyStuff		(0x3)
#define Run			(0x4)
#define Halt			(0x5)
#define GetProperty		(0x6)
#define SetProperty		(0x7)
#define Test			(0x8)
#define HostIntNumber		(0x9)
#define HostIntVector		(0x10)

/* These are the codes that specify the various properties that are Gettable and Settable */
#define UartABaud		(0x80)
#define UartAParity		(0x81)
#define UartABits		(0x82)
#define UartBBaud		(0x83)
#define UartBParity		(0x84)
#define UartBBits		(0x85)
#define DRAMSize		(0x86)
#define CpuClock		(0x87)
#define SRAM1Size		(0x88)
#define SRAM2Size		(0x89)
#define SRAM3Size		(0x8A)
#define SRAM4Size		(0x8B)
#define DRAMBase		(0x8C)
#define HostJTAGBase		(0x8D)
#define LocalJTAGBase		(0x93)
#define Daughter		(0x8E)
#define IPCRBase		(0x8F)
#define BoardName		(0x90)
#define Firmware		(0x91)
#define Hardware		(0x92)

/* These are used to specify the parity type */
#define EVEN	(2)
#define ODD	(1)
