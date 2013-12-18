/* $Header: /giga/HeliosRoot/Helios/servers/fdc/RCS/fdcdev.c,v 1.2 1991/10/09 11:22:32 paul Exp $ */
/* $Source: /giga/HeliosRoot/Helios/servers/fdc/RCS/fdcdev.c,v $ */
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
 * $Log: fdcdev.c,v $
 * Revision 1.2  1991/10/09  11:22:32  paul
 * changed include path for fpproto.h
 *
 * Revision 1.1  1991/01/21  12:50:57  martyn
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

typedef unsigned char   u_char;
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
word ReadData(DiscDCB *dcb, DiscReq *req, PartitionInfo *partInfo, word drive,
	      DriveInfo *driveInfo);
word WriteData(DiscDCB *dcb, DiscReq *req, PartitionInfo *partInfo, word drive,
	       DriveInfo *driveInfo);
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

  if (dcb == NULL ) goto OpenFail;
  
  dcb->dcb.Device    = dev;
  dcb->dcb.Operate   = DevOperate;
  dcb->dcb.Close     = DevClose;
  dcb->driverClosing = 0;

  InitSemaphore(&dcb->lock,    1); /* Main serialising lock of driver 	*/
  InitSemaphore(&dcb->intLock, 0); /* Used to wait for interrupts    	*/

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
    /* IOdebug("drive %d", drive); */
    dcb->drive[drive] = *dvi;
    dcb->driveState[drive].sectorSizeCode = SectorSizeCode(dvi);
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
    /* IOdebug("partition %d", i); */
    dcb->partition[i] = *pii;
    if (pii->Next == -1) break; /* Note use of -1 for end of list */
    pii = (PartitionInfo *)RTOA(pii->Next);
  }
  dcb->partitions = i+1;
      
  InitController(dcb);

  /* Start the timer process */

  InitSemaphore(&dcb->timerProcLock, 0); /* Used to synchronise on closing */

  if (Fork(TIMERPROCSTACK, TimerProcess, sizeof(dcb), dcb) == 0)
  {
    IOdebug("fdcdev: failed to start timer process");
    goto OpenFail;
  }
    
  /* Set up the interrupt routine */
 
  dcb->irqHandler.Pri  = FLOPPYIRQ;
  dcb->irqHandler.Code = (WordFnPtr)IRQHandler;
  dcb->irqHandler.Data = dcb;    /* Pass DCB address to int routine */
  SetEvent(&dcb->irqHandler); 

  UseIRQ(); /* Allow floppy controller to generate IRQs */

  /* Recalibrate each drive */
  for (drive = 0; drive < dcb->drives; ++drive)
  {
    EnsureMotorRunning(dcb, drive);
    Recalibrate(dcb, drive, &dcb->drive[drive]);
  }

  /* Install the SetFIQRegs routine in the DCB */
  InstallSetFIQRegs(dcb);

  return dcb;

OpenFail:
  Free(dcb);
  return NULL;
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Set the controller chip to a standard initial state.			*/
/* The values of various timings are built into the driver here. If	*/
/* necessary, they could be passed as parameters when the device is 	*/
/* opened.								*/
/*----------------------------------------------------------------------*/

void 
InitController(DiscDCB *dcb)
{
  RefFDCRegs fdcRegs    = dcb->fdcRegs;
  word       drive0Type = dcb->drive[0].DriveType;

  /* FUDGE: disc format and transfer rate are set from type of drive 0.	*/
  /* They should be set as each drive is used.				*/

  /* Set internal mode, specifying data transfer rate & precompensation */
  /* Note that it is not necessary to look at the MFM flag to set the	*/
  /* transfer rate.							*/

  SendAuxCommand(fdcRegs, 
		 CMD_CONTROLINTERNALMODE |
		 (drive0Type & DT_HIGHDEN ? DTR_MFM_500 : DTR_MFM_250) << 6 |
		 (DEFAULT_PCS << 4));
  ReadResult(fdcRegs); /* Correct result is INVALID command! */	

  /* Select the disc format */

  SendAuxCommand(fdcRegs, 
		 CMD_SELECTFORMAT | (drive0Type & DT_IBM ? 0x00 : 0x10));
  ReadResult(fdcRegs); /* Correct result is INVALID command! */	

  /* Issue a SPECIFY command to set up Step Rate Time, Head Load Time,	*/
  /* Head Unload Time, and whether or not DMA is being used.		*/

  SendCommand3(fdcRegs, CMD_SPECIFY,
	       (DEFAULT_STEPRATETIME << 4) | DEFAULT_HEADUNLOADTIME,
	       (DEFAULT_HEADLOADTIME << 1) | DEFAULT_DMAMODE);
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Routine executed by the driver's timer process to perform periodic	*/
/* actions (such as turning off the motors of drives which have not	*/
/* been used recently).							*/
/*----------------------------------------------------------------------*/

void
TimerProcess(DiscDCB *dcb)
{
  /* Process runs until driver announces it is closing down */

  while (!dcb->driverClosing)
  {
    unsigned csTime; /* Time in centiseconds */
    word     drive;

    Delay(TIMERPROCPERIOD); /* Have a nap */
    Wait(&dcb->lock); /* Get exclusive access to device */
    csTime = (unsigned)_cputime(); /* Find out the time */

    /* Turn off motors of idle drives */

    for (drive = 0; drive < dcb->drives; ++drive)
    {
      RefDriveState driveState = &dcb->driveState[drive];

      /* Check that motor is on and no command is in progress		*/

      if (((driveState->flags & DSF_MOTORON) != 0) &&
	  (driveState->currentCommand == 0))
      {
	unsigned csIdle = csTime - driveState->lastCommandTime;
			  /* Unsigned subtract should be OK even	*/
			  /* if time has wrapped round.			*/

	if (csIdle > MOTOROVERRUNCS)
	{
	  MotorControl(dcb, drive, MotorOff);
	}
      }
    }

    Signal(&dcb->lock); /* Allow others to use device */
  }

  /* This process has to be careful about exiting so that the driver	*/
  /* does not close until the timer process has finished touching the	*/
  /* DCB.								*/

  Signal(&dcb->timerProcLock); /* Allow driver to close */

  /* The driver may have closed by now, but this process should still	*/
  /* be able to exit safely.						*/
}

/*----------------------------------------------------------------------*/

word 
DevClose(DiscDCB *dcb)
{
  Wait(&dcb->lock);
  dcb->driverClosing = 1; /* Tell driver processes to shut down */

  /* Release lock while waiting for the timer process to finish */

  Signal(&dcb->lock);
  Wait(&dcb->timerProcLock);
  Wait(&dcb->lock); /* Reclaim lock to complete close operation */

  /* Reset the controller to stop all the motors etc. */

  DisableIRQ(INT_FLOPPY);
  SendAuxCommand(dcb->fdcRegs, CMD_SOFTWARERESET);

  RemEvent(&dcb->irqHandler);
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

  /* All disk transfers are done synchronously in this version.		*/
  /* This could be improved if necessary by keeping a queue of requests	*/
  /* and running a separate process to carry out the transfers in the   */
  /* most efficient order.						*/

  Wait(&dcb->lock);

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
    EnsureMotorRunning(dcb, drive);
    error = ReadData(dcb, req, partInfo, drive, driveInfo);
    break;

  case FG_Write:
    EnsureMotorRunning(dcb, drive);
    error = WriteData(dcb, req, partInfo, drive, driveInfo);
    break;

  case FG_Format:
    EnsureMotorRunning(dcb, drive);
    error = FormatCylinders(dcb, req, partInfo, drive, driveInfo);
    break;

  case FG_WriteBoot:	/* Write data to bootstrap area of disc. */
    EnsureMotorRunning(dcb, drive);
    error = WriteBoot(dcb, req, partInfo, drive, driveInfo);
    break;

  case FG_GetSize:
    error = driveInfo->SectorSize * driveInfo->SectorsPerTrack * 
            driveInfo->TracksPerCyl * 
	    (partInfo->EndCyl - partInfo->StartCyl+1);
    break;      

  default:
    /* May be a private command: look at whole request word */
    switch (req->DevReq.Request)
    {
    case FF_EnableMotor:
      MotorControl(dcb, drive, MotorOn); 
      break;

    case FF_DisableMotor:
      MotorControl(dcb, drive, MotorOff); 
      break;

    case FF_ReadId:
      EnsureMotorRunning(dcb, drive);
      error = ReadId(dcb, drive, driveInfo);
      break;

    case FF_Seek:
      EnsureMotorRunning(dcb, drive);
      error = DoSeek(dcb, drive, req->Pos);
      break;

    case FF_Recalibrate:
      EnsureMotorRunning(dcb, drive);
      error = Recalibrate(dcb, drive, driveInfo);
      break;

    case FF_SimpleCommand: /* For debugging FDC interface only */
      error = SimpleCommand(dcb, drive, driveInfo);
      break;

    case FF_ReadErrCounts: /* Return error counts structure for this drive */
      *(ErrorCounts *)req->Buf = dcb->driveState[drive].errorCounts;
      break;

    default:
      error = SS_Device|EC_Error | EG_FnCode;
      break;
    }
    break;
  }

OperateExit:
  Signal(&dcb->lock);
  req->DevReq.Result = error;

  /* call back to client */
  (*req->DevReq.Action)(req);
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Perform the Sense Interrupt Status command				*/
/*----------------------------------------------------------------------*/

void
SenseInterruptStatus(RefFDCRegs fdcRegs, u_char *st0, u_char *cyl)
{
  SendCommand(fdcRegs, CMD_SENSEINTSTATUS);
  *st0 = ReadResult(fdcRegs);

  /* This command is invalid if no interrupt is outstanding */
  if (*st0 != CMD_INVALID)
    *cyl = ReadResult(fdcRegs);
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Check the status registers returned after a data transfer operation.	*/
/* Return a Helios error code.						*/
/*----------------------------------------------------------------------*/

word
CheckDataTransferResults(u_char st0, u_char st1, u_char st2, 
			 ErrorCounts *errorCounts)
{
  word error = 0;

  /* Determining whether or not the transfer worked requires careful	*/
  /* inspection of the status registers. Since we have no control 	*/
  /* the Terminal Count (TC) input to the chip successful transfers	*/
  /* appear to fail with an "End of Cylinder" error, so it is not	*/
  /* sufficient just to inspect the interrupt code in ST0.		*/

  if ((st0 & ST0_IC) != ST0_IC_NT)
  {
    /* Operation may have failed: check error flags. Each flag is	*/
    /* tested in turn so that all faults will be reported when		*/
    /* debugging is turned on.						*/

    /* IOdebug("st0 0x%x, st1 0x%x, st2 0x%x", st0, st1, st2); */

    if (st0 & ST0_NR) /* Not Ready */
    {
#ifdef DEBUG
      IOdebug("device not ready");
#endif /* DEBUG */
      ++errorCounts->notReady;
      error = SS_Device | EC_Error | EG_Broken; /* Helpful error code TBD */
    }

    if (st0 & ST0_EC) /* Equipment Check */
    {
#ifdef DEBUG
      IOdebug("equipment check");
#endif /* DEBUG */
      ++errorCounts->equipmentCheck;
      error = SS_Device | EC_Error | EG_Broken; /* Helpful error code TBD */
    }

    if (st1 & ST1_DE) /* Data Error */
    {
#ifdef DEBUG
      IOdebug("data error");
#endif /* DEBUG */
      ++errorCounts->dataError;
      error = SS_Device | EC_Error | EG_Broken; /* Helpful error code TBD */
    }

    if (st1 & ST1_OR) /* Overrun */
    {
#ifdef DEBUG
      IOdebug("data overrun");
#endif /* DEBUG */
      ++errorCounts->overrun;
      error = SS_Device | EC_Error | EG_Broken; /* Helpful error code TBD */
    }

    if (st1 & ST1_ND) /* No Data */
    {
#ifdef DEBUG
      IOdebug("no data");
#endif /* DEBUG */
      ++errorCounts->noData;
      error = SS_Device | EC_Error | EG_Broken; /* Helpful error code TBD */
    }

    if (st1 & ST1_NW) /* Not Writable */
    {
#ifdef DEBUG
      IOdebug("not writable");
#endif /* DEBUG */
      ++errorCounts->notWritable;
      error = SS_Device | EC_Error | EG_Broken; /* Helpful error code TBD */
    }

    if (st1 & ST1_MA) /* Missing Address Mark */
    {
#ifdef DEBUG
      IOdebug("missing address mark");
#endif /* DEBUG */
      ++errorCounts->missingAddressMark;
      error = SS_Device | EC_Error | EG_Broken; /* Helpful error code TBD */
    }

    if (st2 & ST2_CM) /* Control Mark */
    {
#ifdef DEBUG
      IOdebug("deleted data address mark encountered");
#endif /* DEBUG */
      ++errorCounts->controlMark;
      error = SS_Device | EC_Error | EG_Broken; /* Helpful error code TBD */
    }

    if (st2 & ST2_DD) /* Data error in Data field */
    {
#ifdef DEBUG
      IOdebug("CRC error");
#endif /* DEBUG */
      ++errorCounts->crcError;
      error = SS_Device | EC_Error | EG_Broken; /* Helpful error code TBD */
    }

    if (st2 & ST2_WC) /* Wrong Cylinder */
    {
#ifdef DEBUG
      IOdebug("wrong cylinder");
#endif /* DEBUG */
      ++errorCounts->wrongCylinder;
      error = SS_Device | EC_Error | EG_Broken; /* Helpful error code TBD */
    }

    if (st2 & ST2_BC) /* Bad Cylinder */
    {
#ifdef DEBUG
      IOdebug("bad cylinder");
#endif /* DEBUG */
      ++errorCounts->badCylinder;
      error = SS_Device | EC_Error | EG_Broken; /* Helpful error code TBD */
    }

    if (st2 & ST2_MD) /* Missing address mark in Data field */
    {
#ifdef DEBUG
      IOdebug("missing address mark in data field");
#endif /* DEBUG */
      ++errorCounts->missingAddrMarkInData;
      error = SS_Device | EC_Error | EG_Broken; /* Helpful error code TBD */
    }

    /* If no error flag has been found yet and the EN flag is not set	*/
    /* then we have not determined the reason for the abnormal		*/
    /* termination.							*/

    if ((error == 0) && ((st1 & ST1_EN) == 0))
    {
      IOdebug("mysterious abnormal termination: st0 0x%x, st1 0x%x, st2 0x%x",
	      st0, st1, st2);
      ++errorCounts->unknown;
      error = SS_Device | EC_Error | EG_Unknown;
    }
  }

  return error;
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Read data from or write data to disc.				*/
/* Called with dcb->lock already claimed.				*/
/*----------------------------------------------------------------------*/

word
ReadOrWriteData(DiscDCB *dcb, DiscReq *req, PartitionInfo *partInfo, 
		word drive, DriveInfo *driveInfo, TransferType direction)
{
  word   	firstTrack, firstSectorOnFirstTrack;
  word		lastTrack, lastSectorOnLastTrack;
  word		firstSectorOnThisTrack;
  word		track;
  word		sectorsPerTrack = driveInfo->SectorsPerTrack;
  word		tracksPerCyl = driveInfo->TracksPerCyl;
  word 		error    = Err_Null;
  RefFDCRegs    fdcRegs  = dcb->fdcRegs;
  RefDriveState driveState = &dcb->driveState[drive];
  word		sectorOffset = driveState->sectorOffset;
  u_char        st0, st1, st2;
  byte   	*bufStart = req->Buf;
  byte		*bufPtr = bufStart; /* Current buffer position */
  FIQRegs	fiqRegs;
  u_char	command;

  req->Actual = 0;
  
  /* Find and check the absolute disc addresses of the first and last	*/
  /* sectors to	be transferred						*/

  error = DiscAddresses(dcb, partInfo, driveInfo, req,
			&firstTrack, &firstSectorOnFirstTrack,
			&lastTrack,  &lastSectorOnLastTrack);
  if (error) goto ReadWriteExit;

  /* Construct the controller command byte */

  if (direction == ReadTransfer)
    command = CMD_READDATA | 0x00 /* Not Multitrack */   |
              (driveInfo->DriveType & DT_MFM ? 0x40 : 0) |
	      0x20 /* Skip */;
  else
    command = CMD_WRITEDATA | 0x00 /* Not multitrack */  |
              (driveInfo->DriveType & DT_MFM ? 0x40 : 0);

  /* Install the FIQ handler and set the banked FIQ registers.		*/
  /* The handler and register values remain in place for the whole	*/
  /* multi-track read or write operation.				*/

  if (direction == ReadTransfer)
    InstallReadFIQHandler();	/* Put read FIQ routine in place */
  else
    InstallWriteFIQHandler();	/* Put write FIQ routine in place */

  fiqRegs.r8_fiq  = 0;			 /* (don't care)		*/
  fiqRegs.r9_fiq  = 0;			 /* (don't care)		*/
  fiqRegs.r10_fiq = (unsigned)bufStart;	 /* Initial buffer address	*/
  fiqRegs.r11_fiq = (unsigned)&fdcRegs->statusAux; /* Addr of FDC status reg */
  fiqRegs.r12_fiq = (unsigned)&fdcRegs->data; /* Address of FDC data reg */
  fiqRegs.r13_fiq = FIQ_MASK;            /* Address of FP FIQ mask reg	*/

  (*dcb->setFIQRegs)(&fiqRegs);	/* Set the banked FIQ registers */

  /* Loop, doing one transfer operation for each track.			*/
  /* It does not seem possible to use multitrack transfers on the FP,	*/
  /* as you need to assert the TC signal to stop the transfer on the	*/
  /* correct sector.							*/

  /* The start sector may be non-zero for the first track only.		*/
  firstSectorOnThisTrack = firstSectorOnFirstTrack; 

  for (track = firstTrack; track <= lastTrack; ++track)
  {
    word tries;
    word lastSectorOnThisTrack = (track == lastTrack ? lastSectorOnLastTrack : 
				  		       sectorsPerTrack - 1);
    word bytesThisTime = (lastSectorOnThisTrack - firstSectorOnThisTrack + 1) *
                         driveInfo->SectorSize;

    u_char head = (u_char)(track % tracksPerCyl);
    u_char cyl  = (u_char)(track / tracksPerCyl);
/*
    IOdebug("track %d, firstSec %d, lastSec %d, cyl %d, head %d",
	    track, firstSectorOnThisTrack + sectorOffset,
	    lastSectorOnThisTrack + sectorOffset, cyl, head);
*/

    /* Try the operation until it succeeds or a retry count is exceeded */

    for (tries = 0; tries < MAXXFERTRIES; ++tries)
    {
      error = DoSeek(dcb, drive, cyl);

      if (error == 0)
      {
	/* Enable FIQ for data transfer: will enable IRQ for results */
	UseFIQ(); 

	SendCommand9(fdcRegs, command,
		     (u_char)((word)head << 2 | drive),
		     cyl, head, 
		     (u_char)(firstSectorOnThisTrack + sectorOffset),
		     driveState->sectorSizeCode,
		     (u_char)(lastSectorOnThisTrack + sectorOffset),
		     DEFAULT_GAPSKIP,
		     0xFF /* irrelevant if N parameter is > 0 */);

	HardenedWait(&dcb->intLock); /* Wait for transfer to complete */

	/* Check results from interrupt routine */

	st0   = driveState->resultST0;
	st1   = driveState->resultST1;
	st2   = driveState->resultST2;
	error = CheckDataTransferResults(st0, st1, st2, 
					 &driveState->errorCounts);
      }

      if (error == 0) break; /* The data transfer worked */

      /* There has been an error, but we retry a few times.		*/

      if (tries < MAXXFERTRIES)
      {
	++driveState->errorCounts.softErrors;

	/* Use different retry strategies on different attempts. */

	switch (tries % 3)
	{
	  case 0: break;	/* Retry in place */

	  case 1: /* Go to the last cylinder and back*/
	    DoSeek(dcb, drive, driveInfo->Cylinders - 1); break;

	  case 2: /* Go to cylinder 0 and back */
	    Recalibrate(dcb, drive, driveInfo); break;
	}

	/* Must rewrite the FIQ regs to restore the buffer pointer */
	fiqRegs.r10_fiq = (unsigned)bufPtr; /* Other regs unchanged */
	(*dcb->setFIQRegs)(&fiqRegs);	/* Set the banked FIQ registers */
      }
    } /* End of retry loop */

    if (error) goto ReadWriteExit; /* Abandon the transfer */

    req->Actual += bytesThisTime; /* Update the amount transferred */
    bufPtr      += bytesThisTime; /* Update the buffer position    */
    firstSectorOnThisTrack = 0; /* For each track except first in a transfer */
  }

ReadWriteExit:
  if (error) ++driveState->errorCounts.hardErrors;
  return error;
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Read data from disc.							*/
/* Called with dcb->lock already claimed.				*/
/*----------------------------------------------------------------------*/

word
ReadData(DiscDCB *dcb, DiscReq *req, PartitionInfo *partInfo, word drive,
	 DriveInfo *driveInfo)
{
  return ReadOrWriteData(dcb, req, partInfo, drive, driveInfo, ReadTransfer);
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Write data to disc.							*/
/* Called with dcb->lock already claimed.				*/
/*----------------------------------------------------------------------*/

word
WriteData(DiscDCB *dcb, DiscReq *req, PartitionInfo *partInfo, word drive,
	  DriveInfo *driveInfo)
{
  return ReadOrWriteData(dcb, req, partInfo, drive, driveInfo, WriteTransfer);
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Return the recommended size of Gap3 given the sector size and	*/
/* recording mode.							*/
/*----------------------------------------------------------------------*/
u_char
Gap3Size(word sectorSize, word mfm)
{
  u_char gap3;

  if (mfm)
  {
    switch (sectorSize)
    {
    case  256: gap3 =  54; break;
    case  512: gap3 =  84; break;
    case 1024: gap3 = 116; break;

    default:
      IOdebug("don't know Gap3 for MFM sector size %d", sectorSize);
      gap3 = 116; /* Might work */
      break;
    }
  }
  else
  {
    switch (sectorSize)
    {
    case 128: gap3 = 27; break;
    case 256: gap3 = 42; break;
    case 512: gap3 = 58; break;

    default:
      IOdebug("don't know Gap3 for FM sector size %d", sectorSize);
      gap3 = 58; /* Might work */
      break;
    }
  }

  return gap3;
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Format one or more cylinders.					*/
/* Called with dcb->lock already claimed.				*/
/*----------------------------------------------------------------------*/

word
FormatCylinders(DiscDCB *dcb, DiscReq *req, PartitionInfo *partInfo,
		word drive, DriveInfo *driveInfo)
{
  FormatReq *fReq = (FormatReq *)req;
  word error = 0;
  word head;
  word cyl;
  word startCyl        = fReq->StartCyl + partInfo->StartCyl;
  word endCyl          = fReq->EndCyl + partInfo->StartCyl;
  word interleave      = fReq->Interleave;
  word sectorsPerTrack = driveInfo->SectorsPerTrack;
  word skew	       = 0;
  u_char gap3Length    = Gap3Size(driveInfo->SectorSize, 
				  driveInfo->DriveType & DT_MFM);
  RefFDCRegs    fdcRegs      = dcb->fdcRegs;
  RefDriveState driveState   = &dcb->driveState[drive];
  word		sectorOffset = driveState->sectorOffset;
  u_char        st0, st1, st2;
  u_char        command = CMD_WRITEID |
                          (driveInfo->DriveType & DT_MFM ? 0x40 : 0);

  if ((startCyl < partInfo->StartCyl) || (startCyl > partInfo->EndCyl))
    return SS_Device | EC_Error | EG_Parameter | 7; /* Invalid start cyl */
    
  if ((endCyl < partInfo->StartCyl) || (endCyl > partInfo->EndCyl))
    return SS_Device | EC_Error | EG_Parameter | 8; /* Invalid end cyl */
    
  InstallWriteFIQHandler();	/* Put FIQ handler in place */

  /* Loop, issuing a command for each track to be formatted		*/

  for (cyl = startCyl; error == 0 && cyl <= endCyl; ++cyl)
  {
    for (head = 0; error == 0 && head < driveInfo->TracksPerCyl; ++head)
    {
      word    sectorOrder[MAXSECTORS]; /* Map of sectors for this track */
      u_char  formatData[MAXSECTORS*4];
      u_char  *buf;
      word    sectorPos, sector;
      FIQRegs fiqRegs;

      skew = (skew + fReq->TrackSkew) % sectorsPerTrack;
      sectorPos = skew; /* Put sector 0/1 at the current skew offset */

      for (sector = 0; sector < sectorsPerTrack; ++sector)
	sectorOrder[sector] = -1; /* Clear the order array */

      /* Work out the sector arrangement for this track */
      for (sector = 0; sector < sectorsPerTrack; ++sector)
      {
	/* Find the next free sector position after the desired slot */
	while (sectorOrder[sectorPos] != -1)
	  sectorPos = (sectorPos + 1) % sectorsPerTrack;
	sectorOrder[sectorPos] = sector;

	/* Step on `interleave' blocks before trying to place next sector */
        sectorPos = (sectorPos + interleave) % sectorsPerTrack;
      }

      /* Set up the buffer of data which will be requested by the 	*/
      /* controller during formatting. This contains 4 bytes for each	*/
      /* sector: cylinder no., head no., sector no., sector size.	*/

      buf = &formatData[0];
      for (sector = 0; sector < sectorsPerTrack; ++sector)
      {
	*buf++ = (u_char)cyl;
	*buf++ = (u_char)head;
	*buf++ = (u_char)(sectorOrder[sector] + sectorOffset);
	*buf++ = driveState->sectorSizeCode;
      }

      if ((error = DoSeek(dcb, drive, cyl)) != 0) goto FormatExit;

      /* Set the banked FIQ registers (buffer address needs resetting	*/
      /* each time round).						*/

      fiqRegs.r8_fiq  = 0;		 /* (don't care)		     */
      fiqRegs.r9_fiq  = 0;		 /* (don't care)	  	     */
      fiqRegs.r10_fiq = (unsigned)&formatData[0]; /* Initial buf address     */
      fiqRegs.r11_fiq = (unsigned)&fdcRegs->statusAux; /* FDC status reg     */
      fiqRegs.r12_fiq = (unsigned)&fdcRegs->data; /* Address of FDC data reg */
      fiqRegs.r13_fiq = FIQ_MASK;            /* Address of FP FIQ mask reg   */

      (*dcb->setFIQRegs)(&fiqRegs);	/* Set the banked FIQ registers */

      UseFIQ(); /* Enable FIQ for data transfer: will enable IRQ for results */

      SendCommand6(fdcRegs, command,
		   (u_char)((word)head << 2 | drive),
		   driveState->sectorSizeCode,
		   (u_char)sectorsPerTrack,
		   gap3Length,
		   DEFAULT_DATA_BYTE);

      HardenedWait(&dcb->intLock); /* Wait for format to complete */

      /* Check results from interrupt routine */

      st0     = driveState->resultST0;
      st1     = driveState->resultST1;
      st2     = driveState->resultST2;

      error = CheckDataTransferResults(st0, st1, st2, 
				       &driveState->errorCounts);
      if (error) goto FormatExit;
    }
    skew = (skew + fReq->CylSkew) % sectorsPerTrack;
  }

FormatExit:
  if (error) ++driveState->errorCounts.hardErrors;
  return error;
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Write the bootstrap area of the disc.				*/
/* Called with dcb->lock already claimed.				*/
/*----------------------------------------------------------------------*/

word
WriteBoot(DiscDCB *dcb, DiscReq *req, PartitionInfo *partInfo, word drive,
	  DriveInfo *driveInfo)
{
#ifdef rewritten
  /* Write data to bootstrap area of disc. The request is just  */
  /* like a Write request. The SubDevice is used to identify the  */
  /* physical disc to be written. This has to be a device operation*/
  /* because the exact operations required to do this will differ  */
  /* for different controllers.          */
  {
    byte save[32];
    int i;
    word sectors;
    word sect;
    word ssize;
    
    WriteParameter  (link, m2DesiredDrive, info->DriveId);
    WriteCommand  (link, m2SelectDrive);

    /* first save current drive parameters */
        
    for (i = 0; i < 32; i++ ) save[i] = ReadParameter(link, i);
    
    /* reset drive to defaults */
    
    WriteCommand  (link, m2Initialise, info->DriveType);
    WriteCommand  (link, m2Restore);
    
    ssize = 256;
    sectors = (size+ssize-1)/ssize;
    
    /* first we format the tracks which will contain the bootstrap */

    while( err == 0 && ReadTripleParameter(link, m2LogicalSector ) <= sectors )
    {
      WriteCommand(link, m2FormatTrack );
      err = ReadParameter(link, m2Error );
    }
    
    /* now write data out */
    WriteTripleParameter(link, m2LogicalSector, 0 );
    
    for (sect = 0; err == 0 && sect < sectors ; sect++ )
    {
      WriteCommand(link, m2WriteBuffer );
      LinkTx(ssize, link, buf );
      buf += ssize;
      err = ReadParameter(link, m2Error );
      if (err != 0 ) break;
      WriteCommand(link, m2WriteSector );
      err = ReadParameter(link, m2Error );
    }
    
    /* finally restore original drive parameters */

    for (i = 0; i < 32; i++ ) WriteParameter(link, i, save[i] );

    break;
  }
#endif /* rewritten */
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Turn the motor on or off on the specified drive, and record its new	*/
/* state.								*/
/* Called with dcb->lock already claimed.				*/
/*----------------------------------------------------------------------*/

void
MotorControl(DiscDCB *dcb, word drive, MotorState newState)
{
  RefFDCRegs    fdcRegs = dcb->fdcRegs;
  RefDriveState driveState = &dcb->driveState[drive];
  u_char	command;
  int		d;
  
  /* Update the record of the motor state */

  if (newState == MotorOn)
    driveState->flags |= DSF_MOTORON;
  else
    driveState->flags &= ~DSF_MOTORON;

  /* The ENABLE MOTORS command sets the states of the motors on all	*/
  /* drives, so we have to reiterate the states of all the other ones.	*/

  command = CMD_ENABLEMOTORS;
  
  for (d = 0; d < dcb->drives; ++d)
  {
    if (dcb->driveState[d].flags & DSF_MOTORON)
      command |= (1 << (d+4));
  }

  SendAuxCommand(fdcRegs, command);
  ReadResult(fdcRegs); /* Correct result in INVALID, so ignore it */
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Perform a seek operation on the specified drive.			*/
/* The error codes assume that this call is in response to a request in	*/
/* the standard format. Drive has already been checked but cyl has not.	*/
/* Called with dcb->lock already claimed.				*/
/*----------------------------------------------------------------------*/

word
DoSeek(DiscDCB *dcb, word drive, word cyl)
{
  word 		error       = Err_Null;
  RefFDCRegs    fdcRegs     = dcb->fdcRegs;
  DriveState    *driveState = &dcb->driveState[drive];
  u_char        st0, pcn;

  if ((cyl < 0) || (cyl >= dcb->drive[drive].Cylinders))
  {
    error = SS_Device|EC_Error|EG_Parameter|7;
    goto SeekExit;
  }

  SendCommand3(fdcRegs, CMD_SEEK, (u_char)drive, (u_char)cyl);

  /* Record in dcb that seek operation is in progress on this drive	*/
  /* Make sure chip is generating IRQ rather than FIQ			*/

  HardenedWait(&dcb->intLock); /* Wait for seek to complete */

  st0 = driveState->resultST0;
  pcn = driveState->resultPCN;

  if ((st0 & ST0_SE) == 0)
  {
    error = SS_Device|EC_Error|EG_Broken|2;
#ifdef DEBUG
    IOdebug("seek not complete");
#endif
    ++driveState->errorCounts.seekNotComplete;
    goto SeekExit;
  }

  if (pcn != cyl)
  {
    error = SS_Device|EC_Error|EG_Broken|3;
#ifdef DEBUG
    IOdebug("seek got to wrong cylinder (%d not %d)", pcn, cyl);
#endif
    ++driveState->errorCounts.seekToWrongCylinder;
    goto SeekExit;
  }

SeekExit:
  return error;
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Read a sector id from disc.						*/
/* Called with dcb->lock already claimed.				*/
/*----------------------------------------------------------------------*/

word
ReadId(DiscDCB *dcb, word drive, DriveInfo *driveInfo)
{
  word 		error    = Err_Null;
  RefFDCRegs    fdcRegs  = dcb->fdcRegs;
  u_char        st0, st1, st2;
  u_char	resultC, resultH, resultR, resultN;
  DriveState    *driveState = &dcb->driveState[drive];

  SendCommand2(fdcRegs,  
	       CMD_READID | (driveInfo->DriveType & DT_MFM ? 0x40 : 0),
	       (u_char)drive);

  HardenedWait(&dcb->intLock); /* Wait for read to complete */

  st0     = driveState->resultST0;
  st1     = driveState->resultST1;
  st2     = driveState->resultST2;
  resultC = driveState->resultC;
  resultH = driveState->resultH;
  resultR = driveState->resultR;
  resultN = driveState->resultN;

  IOdebug("st0 %x, st1 %x, st2 %x", st0, st1, st2);
  IOdebug("C %x, H %x, R %x, N %x", resultC, resultH, resultR, resultN);

  return error;
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Recalibrate the specified drive so the head is on cylinder 0.	*/
/* Called with dcb->lock already claimed.				*/
/*----------------------------------------------------------------------*/

word
Recalibrate(DiscDCB *dcb, word drive, DriveInfo *driveInfo)
{
  word 		error    = Err_Null;
  RefFDCRegs    fdcRegs  = dcb->fdcRegs;
  u_char        st0, pcn;
  DriveState    *driveState = &dcb->driveState[drive];

  SendCommand2(fdcRegs, CMD_RECALIBRATE, (u_char)drive);

  HardenedWait(&dcb->intLock); /* Wait for recalibration to complete */

  st0     = driveState->resultST0;
  pcn     = driveState->resultPCN;

  return error;
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Issue a simple command and poll for the reply with device interrupts	*/
/* disabled. This allows inspection of the FDC interface on a 'scope.	*/
/* Called with dcb->lock already claimed.				*/
/*----------------------------------------------------------------------*/

word
SimpleCommand(DiscDCB *dcb, word drive, DriveInfo *driveInfo)
{
  RefFDCRegs fdcRegs = dcb->fdcRegs;

  DisableIRQ(INT_FLOPPY);
  SendCommand(fdcRegs, CMD_SENSEDEVICESTATUS);
  SendCommand(fdcRegs, (u_char)drive);
  ReadResult(fdcRegs);
  EnableIRQ(INT_FLOPPY);

  return 0;
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Ensure that a drive's motor is running and up to speed.		*/
/*----------------------------------------------------------------------*/

void
EnsureMotorRunning(DiscDCB *dcb, word drive)
{
  DriveState *driveState = &dcb->driveState[drive];

  if ((driveState->flags & DSF_MOTORON) == 0)
  {
    RefFDCRegs fdcRegs = dcb->fdcRegs;

    MotorControl(dcb, drive, MotorOn);

    /* Wait a minimum time for the motor to start, as some drive	*/
    /* interfaces have the READY line permanently asserted.		*/

    Delay(MIN_READY_WAIT);

    /* Wait for drive to say it is ready - i.e. motor up to speed */
    
    for (;;)
    {
      u_char st3;

      SendCommand2(fdcRegs, CMD_SENSEDEVICESTATUS, (u_char)drive);
      st3 = ReadResult(fdcRegs);

      if (st3 & ST3_RY) break; /* Drive is now ready */

      Delay(OneSec/10); /* Try again later */
    }
  }
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/*			Interrupt routines				*/
/************************************************************************/

/************************************************************************/
/* Low-level device IRQ routine. It is called from the executive	*/
/* and passed the DCB address.						*/
/************************************************************************/

int IRQHandler(DiscDCB *dcb)
{
  /* WARNING: on the functional prototype, there is no obvious way to	*/
  /* 	      detect that the FDC is trying to interrupt. At present,	*/
  /*	      this routine always assumes that the FDC is interrupting,	*/
  /*	      so must be the only IRQ handler present.			*/

  u_char     status;
  RefFDCRegs fdcRegs = dcb->fdcRegs;
  
  /* There are 4 possible causes of interrupt:				*/
  /*  1) Result phase of read, write or format operation		*/
  /*  2) READY signal from drive has changed state			*/
  /*  3) End of SEEK or RECALIBRATE command				*/
  /*  4) Non-DMA data transfer						*/
  /*									*/
  /* (1) and (4) can be distinguished by inspection of the status 	*/
  /* register alone, so are handled first. (In fact, (4) should never	*/
  /* be seen by the IRQ handler in this driver.) Identification of (2)	*/
  /* and (3) requires the Sense Interrupt Status command.		*/

  /* Wait for RQM flag to come on. It isn't obvious from the 72068 data	*/
  /* sheet that this is a sensible thing to do, but the example		*/
  /* interrupt handler does it.						*/

  do { status = fdcRegs->statusAux; } while ((status & SR_RQM) == 0);

  /* If DIO is 1, this is a read/write/format result interrupt, so we	*/
  /* read the result bytes to clear it.					*/
  /* If DIO is 0, we must issue Sense Interrupt Status to find the 	*/
  /* cause and clear the interrupt.					*/

  if (status & SR_DIO)
  {
    /* Get data structure for drive currently doing read/write/format	*/
    RefDriveState driveState = &dcb->driveState[dcb->activeDrive];

    /* Collect the results */
    driveState->resultST0 = ReadResult(fdcRegs);
    driveState->resultST1 = ReadResult(fdcRegs);
    driveState->resultST2 = ReadResult(fdcRegs);
    driveState->resultC   = ReadResult(fdcRegs);
    driveState->resultH   = ReadResult(fdcRegs);
    driveState->resultR   = ReadResult(fdcRegs);
    driveState->resultN   = ReadResult(fdcRegs);

    HardenedSignal(&dcb->intLock); /* Wake up the foreground process */
  }
  else
  {
    RefDriveState driveState;
    u_char        st0, pcn;
    int		  ic, drive;

    SenseInterruptStatus(fdcRegs, &st0, &pcn); /* Find the interrupt cause */

    ic         = st0 & ST0_IC; /* Extract interrupt code */
    drive      = st0 & ST0_US; /* Find which drive interrupted */
    driveState = &dcb->driveState[drive]; /* Get its state structure */

    /* Ignore READY line state transitions in this version */

    if (ic != ST0_IC_AI)
    {
      driveState->resultST0 = st0;
      driveState->resultPCN = pcn;
      HardenedSignal(&dcb->intLock); /* Wake up process waiting for seek */
    }
  }

  return 1; /* Always claim that this was an FDC interrupt */
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Install the pointer to the routine which sets FIQ registers		*/
/*----------------------------------------------------------------------*/

void
InstallSetFIQRegs(DiscDCB *dcb)
{
  /* THIS IS AN APPALLING KLUDGE CREATED IN DESPERATION AFTER REPEATED	*/
  /* FAILURES TO LINK IN THE ASSEMBLY CODE OF THIS ROUTINE. THE CODE	*/
  /* IS INSTALLED IN THE DCB ITSELF.					*/

  unsigned *code = &dcb->setFIQRegsCode[0];

  /* Set up the pointer to the code */
  dcb->setFIQRegs = (SetFIQRtn *)code;
  /* IOdebug("SetFIQRegs code is at 0x%x", (int)code); */

  /* Copy the code into the DCB */
  *code++ = 0xE1A0100E; /* MOV     r1,lr       ; Put link in unbanked reg   */
  *code++ = 0xEF30000D; /* SWI     exec_EnterSVC ; Enter SVC mode	    */
  *code++ = 0xE33FF001; /* TEQP    pc,#FIQMode ; Enter FIQ mode		    */
  *code++ = 0xF1A00000; /* MOVNV   r0,r0       ; Wait for mode change	    */
  *code++ = 0xE8903F00; /* LDMIA   r0,{r8_fiq-r13_fiq} ; Load FIQ registers */
  *code++ = 0xE1B0F001; /* MOVS    pc,r1       ; Return in original mode    */
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Install the FIQ handler for read operations				*/
/*----------------------------------------------------------------------*/

void
InstallReadFIQHandler(void)
{
  /* THIS IS AN APPALLING KLUDGE CREATED IN DESPERATION AFTER REPEATED	*/
  /* FAILURES TO LINK IN THE ASSEMBLY CODE OF THE FIQ ROUTINE!		*/

  unsigned *v = (unsigned *)0x1C;	/* Address of ARM FIQ vector */

  /* FUDGE to get round bug in Helios 900810 */
  /* *(unsigned char *)0x00400030 = 0; */ /* Set physical mapping in MMU */

  *v++ = 0xE4DB8000; /* LDRB   r8_fiq,[statusReg] ; Get controller status    */
  *v++ = 0xE3180020; /* TST    r8_fiq,#SR_NDM     ; Is this a data xfer int? */
  *v++ = 0x14DC8000; /* LDRNEB r8_fiq,[dataReg]   ; Yes: Get data byte       */
  *v++ = 0x14CA8001; /* STRNEB r8_fiq,[bufPtr],#1 ; Put in buf and step ptr  */
  *v++ = 0x125EF004; /* SUBNES pc,r14_fiq,#4      ; Return from FIQ	     */
		     /* ; Not a data xfer int, so let the IRQ show through   */
  *v++ = 0xE3A08000; /* MOV    r8_fiq,#0				     */
  *v++ = 0xE4CD8000; /* STRB   r8_fiq,[fiqMaskReg]; Disable all FIQ sources  */
  *v++ = 0xE25EF004; /* SUBS   pc,r14_fiq,#4      ; Return		     */
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Install the FIQ handler for write operations				*/
/*----------------------------------------------------------------------*/

void
InstallWriteFIQHandler(void)
{
  /* THIS IS AN APPALLING KLUDGE CREATED IN DESPERATION AFTER REPEATED	*/
  /* FAILURES TO LINK IN THE ASSEMBLY CODE OF THE FIQ ROUTINE!		*/

  unsigned *v = (unsigned *)0x1C;	/* Address of ARM FIQ vector */

  *v++ = 0xE4DB8000; /* LDRB   r8_fiq,[statusReg] ; Get controller status    */
  *v++ = 0xE3180020; /* TST    r8_fiq,#SR_NDM     ; Is this a data xfer int? */
  *v++ = 0x14DA8001; /* LDRNEB r8_fiq,[bufPtr],#1 ; Yes: Get byte & step ptr */
  *v++ = 0x14CC8000; /* STRNEB r8_fiq,[dataReg]   ;      Send to controller  */
  *v++ = 0x125EF004; /* SUBNES pc,r14_fiq,#4      ;      Return from FIQ     */
		     /* ; Not a data xfer int, so let the IRQ show through   */
  *v++ = 0xE3A08000; /* MOV    r8_fiq,#0				     */
  *v++ = 0xE4CD8000; /* STRB   r8_fiq,[fiqMaskReg]; Disable all FIQ sources  */
  *v++ = 0xE25EF004; /* SUBS   pc,r14_fiq,#4      ; Return		     */
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Record the time that the last command was executed for this drive	*/
/*----------------------------------------------------------------------*/

void
RecordCommandTime(DiscDCB *dcb, word drive)
{
  dcb->driveState[drive].lastCommandTime = (unsigned)_cputime();
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Convert a disc data position into cylinder, head and sector numbers.	*/
/* Returns 1 if happy with the address, 0 otherwise.			*/
/*----------------------------------------------------------------------*/

int
DiscAddress(DiscDCB *dcb, PartitionInfo *pi, DriveInfo *driveInfo, word pos, 
	    word *cyl /*out*/, word *head /*out*/, word *sector /*out*/)
{
  word sectorNo;	/* Offset into whole disc in sectors */
  word cylNo;
  word trackNo;

  sectorNo = (pos * dcb->blockSize) / driveInfo->SectorSize + pi->StartSector;
  trackNo  = sectorNo / driveInfo->SectorsPerTrack;
  cylNo    = trackNo / driveInfo->TracksPerCyl;

  /* Check that the cylinder is within range */
  if ((cylNo < 0) || (cylNo >= driveInfo->Cylinders))
  {
#ifdef DEBUG
    IOdebug("cylinder %ld out of range", cylNo);
#endif
    return 0;
  }

  /* This address is OK */
  *cyl    = cylNo;
  *head   = trackNo % driveInfo->TracksPerCyl;
  *sector = sectorNo % driveInfo->SectorsPerTrack;
  return 1;
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Convert the position and size in a data transfer request into the	*/
/* absolute disc addresses of the first and last sectors to be		*/
/* transferred.								*/
/* Returns 0 if happy with the addresses, and an error code otherwise.	*/
/*----------------------------------------------------------------------*/

word
DiscAddresses(DiscDCB *dcb, PartitionInfo *pi, DriveInfo *driveInfo,
	      DiscReq *req, 
	      word *firstTrack /*out*/, word *firstSectorOnFirstTrack /*out*/,
	      word *lastTrack /*out*/,  word *lastSectorOnLastTrack /*out*/)
{
  word tracksPerCyl    = driveInfo->TracksPerCyl;
  word sectorsPerTrack = driveInfo->SectorsPerTrack;

  /* Limits of partition as absolute sector numbers on the disc		*/
  word partFirstSect = (pi->StartCyl * tracksPerCyl * sectorsPerTrack) +
       		       pi->StartSector;
  word partLastSect  = ((pi->EndCyl + 1) * tracksPerCyl * sectorsPerTrack) - 1;

  word sectorSize      = driveInfo->SectorSize;
  word bytePos         = req->Pos * dcb->blockSize;
  word bytesToRead     = req->Size * dcb->blockSize;
  word nSectors        = (bytesToRead + sectorSize - 1) / sectorSize;
  word startSectInPart = (bytePos + sectorSize - 1) / sectorSize;
  word endSectInPart   = startSectInPart + nSectors - 1;
  word startSector, endSector;

  /* Get absolute disc addresses of start of transfer */
  startSector = partFirstSect + startSectInPart; 

  /* Check that the first sector is within the partition */
  if ((startSector < partFirstSect) || (startSector > partLastSect))
  {
#ifdef DEBUG
    IOdebug("first sector %d out of partition", startSector);
#endif
    return SS_Device | EC_Error | EG_Parameter | 7; /* Invalid Pos */
  }

  /* Get absolute disc address of end of transfer */
  endSector = partFirstSect + endSectInPart;

  /* Check that the last sector is within the partition */
  if ((endSector < partFirstSect) || (endSector > partLastSect))
  {
#ifdef DEBUG
    /* IOdebug("last sector %d out of partition", endSector); */
#endif
    return SS_Device | EC_Error | EG_Parameter | 8; /* Invalid Size */
  }

  /* This transfer is entirely within the partition */

  *firstTrack              = startSector / sectorsPerTrack;
  *firstSectorOnFirstTrack = startSector % sectorsPerTrack;
  *lastTrack               = endSector / sectorsPerTrack;
  *lastSectorOnLastTrack   = endSector % sectorsPerTrack;

  return 0;
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Determine the controller's code for the sector size of a drive.	*/
/*----------------------------------------------------------------------*/

u_char
SectorSizeCode(DriveInfo *driveInfo)
{
  u_char code;

  /* The code depends on the sector size and the recording mode */

  switch (driveInfo->SectorSize)
  {
    case 128:  code = 0; break;
    case 256:  code = 1; break;
    case 512:  code = 2; break;
    case 1024: code = 3; break;
    case 2048: code = 4; break;
    case 4096: code = 5; break;
    case 8192: code = 6; break;

    default: code = 0xFF; break; /* Invalid value */
  }

  /* is this adjustment correct???? */
  /* if (driveInfo->DriveType & DT_MFM) --code; */ /* Adjust code if MFM format */

  return code;
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Set the interrupt mask so that the FDC asserts FIQ as well as IRQ	*/
/************************************************************************/

void
UseFIQ(void)
{
  *(unsigned char *)FIQ_MASK = INT_FLOPPY;
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Allow the FDC to generate IRQs					*/
/************************************************************************/

void
UseIRQ(void)
{
  EnableIRQ(INT_FLOPPY);
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Busy wait until the controller is ready to receive a command		*/
/************************************************************************/

void
WaitForController(RefFDCRegs fdcRegs)
{
  for (;;)
  {
    u_char status = fdcRegs->statusAux;
    if ((status & SR_CB) == 0) break; /* Controller is ready */
  }
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Send one command byte to the controller				*/
/************************************************************************/

void
SendCommand(RefFDCRegs fdcRegs, u_char command)
{
  u_char status;

  /* Busy wait until the controller is listening for a command */
  for (;;)
  {
    status = fdcRegs->statusAux;
    /* IOdebug("status 0x%x", status); */
    if (((status & SR_DIO) == 0) && ((status & SR_RQM) != 0)) break;
  }

  fdcRegs->data = command;
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Send one auxilliary command byte to the controller			*/
/************************************************************************/

void
SendAuxCommand(RefFDCRegs fdcRegs, u_char command)
{
  u_char status;

  /* Busy wait until the controller is listening for a command */
  for (;;)
  {
    status = fdcRegs->statusAux;
    if (((status & SR_DIO) == 0) && ((status & SR_RQM) != 0)) break;
  }

  fdcRegs->statusAux = command; /* Aux commands are sent to separate reg */
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Send a 2-byte command indivisibly					*/
/************************************************************************/

void
SendCommand2(RefFDCRegs fdcRegs, u_char b1, u_char b2)
{
  DisableIRQ(INT_FLOPPY);
  SendCommand(fdcRegs, b1); 
  SendCommand(fdcRegs, b2); 
  EnableIRQ(INT_FLOPPY);
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Send a 3-byte command indivisibly					*/
/************************************************************************/

void
SendCommand3(RefFDCRegs fdcRegs, u_char b1, u_char b2, u_char b3)
{
  DisableIRQ(INT_FLOPPY);
  SendCommand(fdcRegs, b1); 
  SendCommand(fdcRegs, b2); 
  SendCommand(fdcRegs, b3); 
  EnableIRQ(INT_FLOPPY);
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Send a 9-byte command indivisibly					*/
/************************************************************************/

void
SendCommand9(RefFDCRegs fdcRegs, u_char b1, u_char b2, u_char b3, u_char b4,
	     u_char b5, u_char b6, u_char b7, u_char b8, u_char b9)
{
  /*IOdebug("command9 %x %x %x %x %x %x %x %x %x", b1,b2,b3,b4,b5,b6,b7,b8,b9);*/
  DisableIRQ(INT_FLOPPY);
  SendCommand(fdcRegs, b1); 
  SendCommand(fdcRegs, b2); 
  SendCommand(fdcRegs, b3); 
  SendCommand(fdcRegs, b4); 
  SendCommand(fdcRegs, b5); 
  SendCommand(fdcRegs, b6); 
  SendCommand(fdcRegs, b7); 
  SendCommand(fdcRegs, b8); 
  SendCommand(fdcRegs, b9); 
  EnableIRQ(INT_FLOPPY);
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Send a 6-byte command indivisibly					*/
/************************************************************************/

void
SendCommand6(RefFDCRegs fdcRegs, u_char b1, u_char b2, u_char b3, u_char b4,
	     u_char b5, u_char b6)
{
  DisableIRQ(INT_FLOPPY);
  SendCommand(fdcRegs, b1); 
  SendCommand(fdcRegs, b2); 
  SendCommand(fdcRegs, b3); 
  SendCommand(fdcRegs, b4); 
  SendCommand(fdcRegs, b5); 
  SendCommand(fdcRegs, b6); 
  EnableIRQ(INT_FLOPPY);
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* Read one result byte from the controller				*/
/************************************************************************/

u_char
ReadResult(RefFDCRegs fdcRegs)
{
  u_char status;

  /* Busy wait until the controller is ready to send a result */
  for (;;)
  {
    status = fdcRegs->statusAux;
    if (((status & SR_DIO) != 0) && ((status & SR_RQM) != 0)) break;
  }

  return fdcRegs->data;
}

/*----------------------------------------------------------------------*/

/* End of fdcdev.c */
