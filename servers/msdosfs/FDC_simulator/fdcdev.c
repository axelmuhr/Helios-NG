/* $Header: /giga/Helios/servers/fdc/RCS/fdcdev.c,v 1.1 91/01/21 12:50:57 martyn Exp $ */
/* $Source: /giga/Helios/servers/fdc/RCS/fdcdev.c,v $ */
/************************************************************************/ 
/* fdcdev.c - Helios/ARM device driver for floppy disc on AB		*/
/*	      Functional Prototype					*/
/*									*/
/* Copyright 1990 Active Book Company Ltd., Cambridge, England		*/
/*									*/
/* Author: Brian Knight, September 1990					*/
/************************************************************************/


/************************************************************************/
/* This driver is for the NEC uPD72068 floppy disc controller. 		*/
/*									*/
/* This version supports multiple drives, but serialises all requests	*/
/* to the controller to simplify interrupt handling. FIQ is used for	*/
/* data transfers and IRQ for all other commands.			*/
/************************************************************************/

/*
 * $Log:	fdcdev.c,v $
 * Revision 1.1  91/01/21  12:50:57  martyn
 * Initial revision
 * 
 * Revision 1.5  90/10/17  08:32:50  brian
 * Made head load time less ambitious too
 * 
 * Revision 1.2  90/10/12  17:13:51  brian
 * Checkpoint of working version
 * 
 * Revision 1.1  90/10/03  17:17:07  brian
 * Initial revision
 * 
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <posix.h>
/*************************************************************************

To be done:

- Problem: no simple way to determine whether an IRQ is caused by the FDC
- Check whether HLT applies after seek
- Hold separate set of SRT, HUT, HLT for each drive and issue
  extra SPECIFY commands when necessary
- Hold Data Transfer Rate and Preshift for each drive; have interface
  for changing them.
- Look to see which drives are actually present.
- Use executive FIQ support.
- Timeouts on device operations.
- Separate Verify operation: as Read, but no data transfer and fewer retries.
- Alter step time and head load time according to data transfer rate.

*************************************************************************/

/* #define DEBUG */

/* Fool helios.h into defining Code */
#define in_kernel
#include <helios.h>
#undef in_kernel

#include <syslib.h>
#include <device.h>
#include <codes.h>
#include <root.h>
#include <link.h>
#include <config.h>
#include <asm.h>
#include <process.h>
#include <nonansi.h>
#include <event.h>
#include <abcARM/fproto.h>

#include <dev/fdcdev.h>
/*----------------------------------------------------------------------*/
/* Macros								*/
/*----------------------------------------------------------------------*/

/* Command bytes */

#define CMD_READDATA		0x06
#define CMD_READDELETEDDATA	0x0C
#define CMD_READID		0x0A
#define CMD_WRITEID		0x0D
#define CMD_WRITEDATA		0x05
#define CMD_WRITEDELETEDDATA	0x09
#define CMD_READDIAGNOSTIC	0x02
#define CMD_SCANEQUAL		0x11
#define CMD_SCANLOWOREQUAL	0x19
#define CMD_SCANHIGHOREQUAL	0x1D
#define CMD_SEEK		0x0F
#define CMD_RECALIBRATE		0x07
#define CMD_SENSEINTSTATUS	0x08
#define CMD_SENSEDEVICESTATUS	0x04
#define CMD_SPECIFY		0x03
#define CMD_SETSTANDBY		0x35
#define CMD_RESETSTANDBY	0x34
#define CMD_SOFTWARERESET	0x36
#define CMD_ENABLEEXTERNAL	0x33
#define CMD_CONTROLINTERNALMODE	0x0B
#define CMD_ENABLEMOTORS	0x0E
#define CMD_SELECTFORMAT	0x4F
#define CMD_STARTCLOCK		0x47
#define CMD_INVALID		0x80

/* Status register bits */

#define SR_D0B		0x01	/* Seek in progress on drive 0	*/
#define SR_D1B		0x02	/* Seek in progress on drive 1	*/
#define SR_D2B		0x04	/* Seek in progress on drive 2	*/
#define SR_D3B		0x08	/* Seek in progress on drive 3	*/
#define SR_CB		0x10	/* Controller busy		*/
#define SR_NDM		0x20	/* Non-DMA mode			*/
#define SR_DIO		0x40	/* Data input/output. 0 => FDC	*/
				/* listening, 1 => FDC sending	*/
#define SR_RQM		0x80	/* ReQuest for Master. (i.e.	*/
				/* ready for data transfer)	*/

/* Status register 0 */

#define ST0_US		0x03	/* Unit Select field		*/
#define ST0_HD		0x04	/* Head number			*/
#define ST0_NR		0x08	/* Device Not Ready		*/
#define ST0_EC		0x10	/* Equipment Check		*/
#define ST0_SE		0x20	/* Seek End			*/
#define ST0_IC		0xC0	/* Interrupt Code		*/

#define ST0_IC_NT	0x00	/* Normal Termination		*/
#define ST0_IC_AT	0x40	/* Abnormal Termination		*/
#define ST0_IC_IC	0x80	/* Invalid Command		*/
#define ST0_IC_AI	0xC0	/* State transition on device	*/

/* Status register 1 */

#define ST1_MA		0x01	/* Missing Address mark		*/
#define ST1_NW		0x02	/* Not Writable			*/
#define ST1_ND		0x04	/* No Data			*/
/*			0x08	   (not used)			*/
#define ST1_OR		0x10	/* Over Run			*/
#define ST1_DE		0x20	/* Data Error			*/
/*			0x40	   (not used)			*/
#define ST1_EN		0x80	/* ENd of cylinder		*/

/* Status register 2 */

#define ST2_MD		0x01	/* Missing address mark in Data field	*/
#define ST2_BC		0x02	/* Bad Cylinder				*/
#define ST2_SN		0x04	/* Scan Not satisfied			*/
#define ST2_SH		0x08	/* Scan equal Hit			*/
#define ST2_WC		0x10	/* Wrong Cylinder			*/
#define ST2_DD		0x20	/* Data error in Data field		*/
#define ST2_CM		0x40	/* Control Mark				*/
/*			0x80	   (not used)				*/

/* Status register 3 */

#define ST3_US		0x03	/* Unit Select (drive number)	*/
#define ST3_HD		0x04	/* Head number			*/
#define ST3_TS		0x08	/* State of 2SIDE input		*/
#define ST3_T0		0x10	/* State of TRK0 input		*/
#define ST3_RY		0x20	/* State of READY input		*/
#define ST3_WP		0x40	/* State of WPRT input		*/
#define ST3_FT		0x80	/* State of FLT input		*/

/* Data Transfer Rates (DR1 and DR0 bits) */

#define DTR_FM_125	0		/* 125 Kbits/s in FM mode	*/
#define DTR_FM_250	1		/* 250 Kbits/s in FM mode	*/
#define DTR_FM_75	2		/*  75 Kbits/s in FM mode	*/
#define DTR_FM_150	3		/* 150 Kbits/s in FM mode	*/

#define DTR_MFM_250	DTR_FM_125	/* 250 Kbit/s in MFM mode	*/
#define DTR_MFM_500	DTR_FM_250	/* 500 Kbit/s in MFM mode	*/
#define DTR_MFM_150	DTR_FM_75	/* 150 Kbit/s in MFM mode	*/
#define DTR_MFM_300	DTR_FM_150	/* 300 Kbit/s in MFM mode	*/

/* Precompensation times (PCS1 and PCS0 bits).				*/
/* Meanings of non-zero values depend on DR1 bit and external crystal	*/
/* frequency - see 72068 data sheet.					*/
/* Values given apply when DR1 is 0.					*/

#define PCS_0		0		/* 0 ns				*/
#define PCS_125		1		/* 125 ns (or 208 or 104)	*/
#define PCS_188		2		/* 188 ns (or 313 or 156)	*/
#define PCS_250		3		/* 250 ns (or 417 or 208)	*/

/* Default settings: these parameters should be settable per drive */

#define DEFAULT_STEPRATETIME	 0xD	/* 5.1ms for high density 3.5"	*/
#define DEFAULT_HEADLOADTIME	0x08	/* 16ms for HD 3.5" floppy	*/
#define DEFAULT_HEADUNLOADTIME	 0xF	/* Maximum: 240/480/405/195 ms	*/
#define DEFAULT_DMAMODE		   1	/* Not DMA mode			*/
#define DEFAULT_GAPSKIP		   1	/* Gap 3 skip when reading/writing   */
					/* Note that 0 seems to be infinity! */
#define DEFAULT_DATA_BYTE	0xE5    /* Used to fill formatted blocks */
					/* Chosen to be unkind to MFM	*/
#define DEFAULT_DTR	DTR_MFM_250	/* Data transfer rate		*/
#define DEFAULT_PCS	PCS_0		/* Precompensation		*/
#define MIN_READY_WAIT	    500*1000	/* Min wait (us) for motor to start */

/* Driver configuration */

#define FLOPPYIRQ	   44		/* Magic number for Jamie	*/
#define TIMERPROCSTACK	 5000		/* Stack size of timer process	*/
#define TIMERPROCPERIOD OneSec		/* Timer process loop time (us)	*/
#define MOTOROVERRUNCS	5*100		/* Motor overrun time (cs)	*/
#define MAXSECTORS	   32		/* Limit on sectors per track	*/
#define MAXXFERTRIES       10		/* Max attempts at data transfer */


/*----------------------------------------------------------------------*/
/* Types								*/
/*----------------------------------------------------------------------*/

typedef volatile u_char vu_char;

/* Structure which maps onto the memory-mapped registers of the fdc	*/
/* chip on the functional prototype. Char fields are used to encourage	*/
/* the compiler to generate byte memory accesses.                       */

typedef struct FDCRegs
{
  vu_char statusAux;		/* status & aux command register	*/
  vu_char _pad[0x1f];
  vu_char data;			/* read/write data register		*/
} FDCRegs, *RefFDCRegs;


/* Structure used to hold the current state of each drive, and to	*/
/* return results from the IRQ routine.					*/

typedef struct DriveState
{
  ErrorCounts errorCounts;	/* Structure containing error counts	*/
  unsigned    lastCommandTime;	/* Time (from _cputime) of last command	*/
  unsigned    flags;		/* Various status flags			*/
  word	      sectorOffset;	/* Number of first sector on track	*/
  u_char      sectorSizeCode;	/* Controller's code for sector size	*/
  u_char      currentCommand;	/* Command in progress (0 if none)	*/
  u_char      resultST0;	/* ST0 result from read/write/format	*/
  u_char      resultST1;	/* ST1					*/
  u_char      resultST2;	/* ST2					*/
  u_char      resultC;		/* Cylinder				*/
  u_char      resultH;		/* Head					*/
  u_char      resultR;		/* Record (sector)			*/
  u_char      resultN;		/* Number of bytes per sector		*/
  u_char      resultPCN;	/* Present Cylinder Number (from SIS)	*/
} DriveState, *RefDriveState;

/* Drive status flags */

#define DSF_MOTORON	0x01	/* Drive motor is running		*/

/* Type of the routine SetFIQRegs (which is in machine code) and the	*/
/* structure used for its argument.					*/

typedef struct FIQRegs
{
  unsigned r8_fiq;
  unsigned r9_fiq;
  unsigned r10_fiq;
  unsigned r11_fiq;
  unsigned r12_fiq;
  unsigned r13_fiq;
} FIQRegs;

typedef void SetFIQRtn(FIQRegs *fiqRegs);

/* Device Control Block */

typedef struct DiscDCB
{
  DCB		dcb;		      /* standard DCB			*/
  volatile RefFDCRegs fdcRegs;	      /* memory mapped FDC registers	*/
  Semaphore	lock;		      /* main device serializing lock	*/
  Semaphore	intLock;	      /* for waiting for int routine	*/
  Semaphore	timerProcLock;	      /* used to await timer proc death	*/
  word		maxTfr;		      /* maximum transfer size		*/
  word		blockSize;	      /* unit of Size in read/write 	*/
  int		driverClosing;	      /* set to kill driver processes	*/
  Event		irqHandler;	      /* Event struct for IRQ handler	*/
  int		activeDrive;	      /* Drive expected to cause int	*/
  word		drives;		      /* number of drives		*/
  word		partitions;	      /* number of partitions		*/
  DriveInfo	drive[MAXDRIVES];     /* client supplied info on drives	*/
  PartitionInfo	partition[MAXPARTS];  /* client info on partitions 	*/
  DriveState	driveState[MAXDRIVES];/* state of each drive		*/
  SetFIQRtn	*setFIQRegs;	      /* pointer to SetFIQRegs routine	*/
  unsigned	setFIQRegsCode[6];    /* KLUDGE: SetFIQRegs code itself	*/
} DiscDCB;

typedef enum MotorState {MotorOff, MotorOn} MotorState;
typedef enum TransferType {ReadTransfer, WriteTransfer} TransferType;

/*----------------------------------------------------------------------*/
/* Forward references							*/
/*----------------------------------------------------------------------*/

void DevOperate(DiscDCB *dcb, DiscReq *req);
word DevClose(DiscDCB *dcb);
void InitController(DiscDCB *dcb);
void WaitForController(RefFDCRegs fdcRegs);
void SendCommand(RefFDCRegs fdcRegs, u_char command);
void SendAuxCommand(RefFDCRegs fdcRegs, u_char command);
void SendCommand2(RefFDCRegs fdcRegs, u_char b1, u_char b2);
void SendCommand3(RefFDCRegs fdcRegs, u_char b1, u_char b2, u_char b3);
void SendCommand6(RefFDCRegs fdcRegs, u_char b1, u_char b2, u_char b3,
		  u_char b4, u_char b5, u_char b6);
void SendCommand9(RefFDCRegs fdcRegs, u_char b1, u_char b2, u_char b3, 
		  u_char b4, u_char b5, u_char b6, u_char b7, u_char b8,
		  u_char b9);
void SenseInterruptStatus(RefFDCRegs fdcRegs, u_char *st0, u_char *cyl);
word DoSeek(DiscDCB *dcb, word drive, word cyl);
word Recalibrate(DiscDCB *dcb, word drive, DriveInfo *driveInfo);
word SimpleCommand(DiscDCB *dcb, word drive, DriveInfo *driveInfo);
word ReadData(DiscReq *req, Stream *dos);
word WriteData(DiscReq *req, Stream *dos);
word FormatDisc(DiscReq *req, Stream *dos);
word WriteBoot(DiscDCB *dcb, DiscReq *req, PartitionInfo *partInfo, word drive,
	       DriveInfo *driveInfo);
word FormatCylinders(DiscDCB *dcb, DiscReq *req, PartitionInfo *partInfo,
		     word drive, DriveInfo *driveInfo);
word ReadId(DiscDCB *dcb, word drive, DriveInfo *driveInfo);
void MotorControl(DiscDCB *dcb, word drive, MotorState newState);
void EnsureMotorRunning(DiscDCB *dcb, word drive);
void RecordCommandTime(DiscDCB *dcb, word drive);
void UseFIQ(void);
void InstallSetFIQRegs(DiscDCB *dcb);
void InstallReadFIQHandler(void);
void InstallWriteFIQHandler(void);
void UseIRQ(void);
int IRQHandler(DiscDCB *dcb);
u_char ReadResult(RefFDCRegs fdcRegs);
void TimerProcess(DiscDCB *dcb);
u_char SectorSizeCode(DriveInfo *driveInfo);
int DiscAddress(DiscDCB *dcb, PartitionInfo *pi, DriveInfo *info, word pos, 
		word *cyl /*out*/, word *head /*out*/, word *sector /*out*/);
word DiscAddresses(DiscDCB *dcb, PartitionInfo *pi, DriveInfo *driveInfo,
	           DiscReq *req, 
		   word *firstTrack, word *firstSectorOnFirstTrack,
		   word *lastTrack,  word *lastSectorOnLastTrack);

/*----------------------------------------------------------------------*/
/* Symbols in the assembler part of the driver				*/
/*----------------------------------------------------------------------*/

extern void ReadFIQHandler(void); /* Start of read FIQ handler routine	*/
extern int *ReadFIQEnd;		/* Word after end of read FIQ handler	*/
extern int *WriteFIQHandler;	/* Start of write FIQ handler routine	*/
extern int *WriteFIQEnd;       	/* Word after end of write FIQ handler	*/

/************************************************************************/


/*----------------------------------------------------------------------*/
/* Open the device							*/
/*----------------------------------------------------------------------*/

DiscDCB *DevOpen(Device *dev, DiscDevInfo *info)
{
  DriveInfo     *dvi;
  PartitionInfo *pii;
  DiscDCB       *dcb = Malloc(sizeof(DiscDCB));
  word drive, i;
  char buffer[720*1024];
  Stream *dos;

  if (dcb == NULL ) goto OpenFail;
  
  dcb->dcb.Device    = dev;
  dcb->dcb.Operate   = DevOperate;
  dcb->dcb.Close     = DevClose;
  dcb->driverClosing = 0;


  dcb->blockSize = info->Addressing;
  dcb->fdcRegs   = (RefFDCRegs)info->Controller;

  /* Initialise the drive state structures */

  for (drive = 0; drive < MAXDRIVES; ++drive)
  {
    RefDriveState ds = &dcb->driveState[drive];
    int           *eField = (int *)&ds->errorCounts;
    int		  i;

    for (i = 0; i < (sizeof(ErrorCounts)/sizeof(int)); ++i) *eField++ = 0;

    ds->lastCommandTime = 0;
    ds->flags           = 0;
    ds->currentCommand  = 0;
    ds->sectorOffset    = 0;    /* 0 for non-IBM discs */
    ds->sectorSizeCode  = 0xFF; /* Invalid value */
  }
  
  /* Copy the drive information structures */
  dvi = (DriveInfo *)RTOA(info->Drives);
  for (drive = 0; drive < MAXDRIVES; drive++)
  {
    dcb->drive[drive] = *dvi;
    dcb->driveState[drive].sectorSizeCode = 0;
    /* Force sector numbering from 1 for IBM discs */
    if (dvi->DriveType & DT_IBM)
      dcb->driveState[drive].sectorOffset = 1;
    if (dvi->Next == -1 ) break; /* Note use of -1 for end of list */
    dvi = (DriveInfo *)RTOA(dvi->Next);
  }
  dcb->drives = drive+1;  
  
  pii = (PartitionInfo *)RTOA(info->Partitions);
  for (i = 0 ; i < MAXPARTS ; i++ )
  {
    dcb->partition[i] = *pii;
    if (pii->Next == -1) break; /* Note use of -1 for end of list */
    pii = (PartitionInfo *)RTOA(pii->Next);
  }
  dcb->partitions = i+1;
      
  /* Start the timer process */

  /* Set up the interrupt routine */
 

  return dcb;

OpenFail:
  Free(dcb);
  return NULL;
}

/*----------------------------------------------------------------------*/

word 
DevClose(DiscDCB *dcb)
{
  dcb->driverClosing = 1; /* Tell driver processes to shut down */

  /* Release lock while waiting for the timer process to finish */


  /* Reset the controller to stop all the motors etc. */

  Free(dcb);
  return Err_Null;
}

/*----------------------------------------------------------------------*/

void 
DevOperate(DiscDCB *dcb, DiscReq *req)
{
  word          error = Err_Null;
  word   	drive, part;
  PartitionInfo *partInfo;
  DriveInfo     *driveInfo;
  Stream *dos = fdstream(open("/helios/tmp/dos", O_RDWR));

  /* All disk transfers are done synchronously in this version.		*/
  /* This could be improved if necessary by keeping a queue of requests	*/
  /* and running a separate process to carry out the transfers in the   */
  /* most efficient order.						*/


  req->Actual = 0;

  /* Common code to find and check the partition and drive numbers.	*/
  /* This will have to be disabled for any functions which do not refer	*/
  /* to a specific drive.						*/

  part = req->DevReq.SubDevice;
  if (part < 0 || part >= dcb->partitions)
  {
    error = SS_Device|EC_Error|EG_Parameter|5;
    goto OperateExit;
  }

  partInfo  = &dcb->partition[part];
  drive     = partInfo->Drive;
  driveInfo = &dcb->drive[drive];

  /* Record time of last command for this drive */

  RecordCommandTime(dcb, drive);

  /* Do the requested function */

  switch (req->DevReq.Request & FG_Mask)
  {
  case FG_Read:
    error = ReadData(req, dos);
    break;

  case FG_Write:
    error = WriteData(req, dos);
    break;

  case FG_Format:
    error = FormatDisc(req, dos);
    break;

  default:
    /* May be a private command: look at whole request word */
    switch (req->DevReq.Request)
    {
    default:
      error = SS_Device|EC_Error | EG_FnCode;
      break;
    }
    break;
  }

OperateExit:
  req->DevReq.Result = error;

  close(dos);
  /* call back to client */
  (*req->DevReq.Action)(req);
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Read data from disc.							*/
/* Called with dcb->lock already claimed.				*/
/*----------------------------------------------------------------------*/

word
ReadData(DiscReq *req, Stream *dos)
{
  Seek(dos, S_Beginning, req->Pos*512);
  req->Actual = Read(dos, req->Buf, req->Size*512, -1);
  req->DevReq.Result = (req->Actual == req->Size*512) ? 0 : -1;
  return 0;
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Write data to disc.							*/
/* Called with dcb->lock already claimed.				*/
/*----------------------------------------------------------------------*/

word
WriteData(DiscReq *req, Stream *dos)
{
  Seek(dos, S_Beginning, req->Pos*512);
  req->Actual = Write(dos, req->Buf, req->Size*512, -1);
  req->DevReq.Result = (req->Actual == req->Size*512) ? 0 : -1;
  return 0;
}


/*----------------------------------------------------------------------*/
/* Record the time that the last command was executed for this drive	*/
/*----------------------------------------------------------------------*/

void
RecordCommandTime(DiscDCB *dcb, word drive)
{
  dcb->driveState[drive].lastCommandTime = (unsigned)_cputime();
}

word
FormatDisc(DiscReq *req, Stream *dos)
{
  char block[512];
  int i;

  req->Actual = 0;

  for(i = 0 ; i < 512 ; i++)
	block[i] = 0xE5;

  Seek(dos, S_Beginning, 0);

  for(i = 0; i < 737280/512 ; i++)
	req->Actual += Write(dos, block, 512, -1);
  req->DevReq.Result = 0 ;
  return 0;
}

