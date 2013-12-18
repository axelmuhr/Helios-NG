/*
    CATS_C40.H Direct, low level C40 routines for Helios v1.3x
    Copyright (C) 1993  Ken Blackler, JET Joint Undertaking

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
    
    The author can be contacted by EMail as: kb@jet.uk
    or at:
    
    		Ken Blackler
    		JET Joint Undertaking
    		Abingdon
    		Oxfordshire
    		England
    		OX14 3EA
    
*/
                                                                              /*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³                                                                            ³
³            INCLUDE FILE: cats_c40.h                                        ³
³                                                                            ³
³                 PURPOSE: Provides CATS - C40 Specific low level structure  ³
³                                                                            ³
³    MODIFICATION HISTORY:                                                   ³
³                                                                            ³
³    Version        Date       Author    Comments                            ³
³    -------     -----------   ------    --------                            ³
³      1.0      10-Dec-1992 K.Blackler  Original Issue                       ³
³      1.01     24-Aug-1993 K.Blackler  Sorted out includes                  ³
³      1.1      24-Aug-1993 K.Blackler  Implement DMA snchronization on xCRDY³
³     **** Second public release version 22/11/1993 ****                     ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄkb@jet.ukÄÄÄÄÙ
                                                                              */

#if !defined __CATS_C40_H__
#define __CATS_C40_H__
	
#include <c40.h>
#include <stddef.h>
#include <queue.h>
#include <event.h>
#include <sem.h>

#if !defined(__CATSUTIL_H__)
#include "catsutil.h"
#endif

#define IdleTillInterrupt()	_word(0x6000000)	/* or	IDLE	*/

#define DMA_PRIORITY_TOCPU             BIN_00
#define DMA_PRIORITY_TODMA             BIN_11
#define DMA_PRIORITY_EQUAL             BIN_01

#define DMA_MODE_NONSTOP               BIN_00
#define DMA_MODE_STOPNOAUTOINIT     	 BIN_01
#define DMA_MODE_AUTOINIT              BIN_10
#define DMA_MODE_AUTOINITANDSTOP       BIN_11

#define DMA_START_RESET                BIN_00
#define DMA_START_HALTON_RW_BOUNDARY   BIN_01
#define DMA_START_HALTON_TFER_BOUNDARY BIN_10
#define DMA_START_START                BIN_11

#define DMA_INTERRUPT_ON_COMPLETE   TRUE
#define DMA_NOINTERRUPT_ON_COMPLETE FALSE

typedef enum tagC40COMPORT
  {
  	c40com0,
  	c40com1,
  	c40com2,
  	c40com3,
  	c40com4,
  	c40com5
  } C40COMPORT;

typedef WORD32 C40ADDRESS;

#define C40_COMPORT_BASEADDRESS 0x00100040L
#define C40_DMA_BASEADDRESS     0x001000A0L

#define wordsizeof(x) (sizeof(x)/sizeof(WORD32))

typedef struct tagUNIFIEDDIE /* Unified DMA Coprocessor Interrupt Enable Register */
  {
    int DMA0Read:2;  /* 0...1 */ /* DMA0 and DMA1 only have two control bits */
    int DMA0Write:2; /* 2...3 */
    
    int DMA1Read:2;  /* 4...5 */
    int DMA1Write:2; /* 6...7 */ 

    int DMA2Read:3;  /* 8..10 */ /* DMA2 to DMA5 have three control bits */
    int DMA2Write:3; /* 11.13 */ /* Symmetric or what! */

    int DMA3Read:3;  /* 14.16 */
    int DMA3Write:3; /* 17.19 */ 

    int DMA4Read:3;  /* 20.22 */
    int DMA4Write:3; /* 23.25 */ 

    int DMA5Read:3;  /* 26.28 */
    int DMA5Write:3; /* 29.31 */ 
  } UNIFIEDDIE;

typedef UNIFIEDDIE *PUNIFIEDDIE;  

#define UDIE_DISABLE_ALL  			 0  /* Diables all interrupts for that direction on that COM port */
#define UDIE_ENABLE_COMPORTREADY 1  /* Diables the COM port "READY" interrupt that direction on that COM port */

typedef struct tagIIE
  {
    int TINT0:1;    /*  0 */
    
    int ICFULL0:1;  /*  1 */
    int ICRDY0:1;   /*  2 */
    int OCRDY0:1;   /*  3 */
    int OCEMPTY0:1; /*  4 */
    
    int ICFULL1:1;  /*  5 */
    int ICRDY1:1;   /*  6 */
    int OCRDY1:1;   /*  7 */
    int OCEMPTY1:1; /*  8 */
    
    int ICFULL2:1;  /*  9 */
    int ICRDY2:1;   /* 10 */
    int OCRDY2:1;   /* 11 */
    int OCEMPTY2:1; /* 12 */
    
    int ICFULL3:1;  /* 13 */
    int ICRDY3:1;   /* 14 */
    int OCRDY3:1;   /* 15 */
    int OCEMPTY3:1; /* 16 */
    
    int ICFULL4:1;  /* 17 */
    int ICRDY4:1;   /* 18 */
    int OCRDY4:1;   /* 19 */
    int OCEMPTY4:1; /* 20 */
    
    int ICFULL5:1;  /* 21 */
    int ICRDY5:1;   /* 22 */
    int OCRDY5:1;   /* 23 */
    int OCEMPTY5:1; /* 24 */

    int DMAINT0:1;  /* 25 */
    int DMAINT1:1;  /* 26 */
    int DMAINT2:1;  /* 27 */
    int DMAINT3:1;  /* 28 */
    int DMAINT4:1;  /* 29 */
    int DMAINT5:1;  /* 30 */
    
    int TINT1:1;    /* 31 */
  } IIE;
  
typedef IIE *PIIE;


typedef struct tagUNIFIEDDMACONTROLREG
  {
    int Priority            :2; /*  0...1 */
    int TransferMode        :2; /*  2...3 */
    int                     :2; /*  4...5 */
    int bSourceSynch        :1; /*  6     */
    int bDestinationSynch   :1; /*  7     */
    int bAutoInitStatic     :1; /*  8     */
    int                     :1; /*  9     */
    int bAutoInitSynch      :1; /* 10     */
    int                     :1; /* 11     */
    int bReadBitReversed    :1; /* 12     */
    int bWriteBitReversed   :1; /* 13     */
    int bSplitMode          :1; /* 14     */
    int                     :3; /* 15..17 */
    int bIntOnFinish        :1; /* 18     */
    int                     :1; /* 19     */
    int sTransferFinished   :1; /* 20     */
    int                     :1; /* 21     */
    int bStart              :2; /* 22..23 */
    int                     :2; /* 24..25 */
    int sStatus             :2; /* 26..27 */
    int                     :2; /* 28..29 */
    int bFixedPriority      :1; /* 30     (Only on COM0) */
    int                     :1; /* 31     */
  } UNIFIEDDMACONTROLREG;

typedef UNIFIEDDMACONTROLREG *PUNIFIEDDMACONTROLREG;
typedef volatile UNIFIEDDMACONTROLREG V_UNIFIEDDMACONTROLREG;
typedef V_UNIFIEDDMACONTROLREG *PV_UNIFIEDDMACONTROLREG;

typedef struct tagUNIFIEDCHANNELCONTROL
  {
    UNIFIEDDMACONTROLREG ControlReg;
    C40ADDRESS           SourceAddress;
    unsigned             SourceAddressIndex;
    unsigned             TransferCounter;
    C40ADDRESS           DestinationAddress;
    unsigned             DestinationAddressIndex;
    C40ADDRESS           LinkPointer;
  } UNIFIEDCHANNELCONTROL;

typedef UNIFIEDCHANNELCONTROL *PUNIFIEDCHANNELCONTROL;
typedef const UNIFIEDCHANNELCONTROL *PCUNIFIEDCHANNELCONTROL;

typedef struct tagCHANNELREGISTER
  {
    union
      {
        /*SPLITDMACONTROLREG SplitControlReg;*/
        UNIFIEDDMACONTROLREG UnifiedControlReg;
        unsigned wordControlReg;
      } ControlRegister;
    C40ADDRESS SourceAddress;
    unsigned SourceAddressIndex;
    unsigned TransferCounter;
    C40ADDRESS DestinationAddress;
    unsigned DestinationAddressIndex;
    C40ADDRESS LinkPointer;
    unsigned AuxTransferCounter;
    C40ADDRESS AuxLinkPointer;
  } CHANNELREGISTER;

typedef struct tagCOMPORTCONTROLREG
	{
		int                   :2;  /*  0...1 */
		int PortDirection     :1;  /*  2     */
		int InputChannelHalt  :1;  /*  3     */
		int OutputChannelHalt :1;  /*  4     */
		unsigned OutputLevel  :4;  /*  5...8 */
		unsigned InputLevel   :4;  /*  9..12 */
		int                   :19; /* 13..31 */
	} COMPORTCONTROLREG;

void MachineInfo(char *pCluster,char *pName); /* Gets the cluster and processor name */

#define AT_TO_COMPORT   1
#define AT_FROM_COMPORT 2

void C40CreateUnifiedModeTransferExC40(int fDirection,PUNIFIEDCHANNELCONTROL pControl,C40ADDRESS pBuffer,int nBytes,unsigned idCom, word wPriority, BOOL bInterrupt, WORD wCarryOn,int nSkip);
void    C40CreateUnifiedModeTransferEx(int fDirection,PUNIFIEDCHANNELCONTROL pControl,     void *pBuffer,int nBytes,unsigned idCom, word wPriority, BOOL bInterrupt, WORD wCarryOn,int nSkip);
void      C40CreateUnifiedModeTransfer(int fDirection,PUNIFIEDCHANNELCONTROL pControl,     void *pBuffer,int nBytes,unsigned idCom, word wPriority, BOOL bInterrupt, WORD wCarryOn);
void C40SetUnifiedTransferLink(PUNIFIEDCHANNELCONTROL pControl,PUNIFIEDCHANNELCONTROL pNextControl);

void C40MemoryDump(C40ADDRESS SourceAddress,int nWords);
void C40StartLinkedTransfers(C40COMPORT DMAChanNo,PUNIFIEDCHANNELCONTROL pLinkedControlList);
void C40StopLinkedTransfers(C40COMPORT DMAChanNo);

word C40IsDMABusy(int DMAChanNo);
int C40IsThereDataToRead(int DMAChanNo);
int C40IsThereSpaceToWrite(int nCommPort);
void C40SimpleRead(int nCommPort, void *pBuffer, unsigned nWords);
void C40SimpleWrite(int nCommPort, void *pBuffer, unsigned nWords);
void C40ResetComport(int nCommPort); /* Clears any data in the input FIFO */

void C40ShowComportControlReg(C40COMPORT nComPort);
void C40ShowIIERegister(void);
void C40ShowDMAControlReg(C40COMPORT nComport);

BOOL AllocateLink(C40COMPORT nComPort);
BOOL DeAllocateLink(C40COMPORT nComPort);

#define C40_INTERRUPT_STACK_SIZE 600

void *C40InstallInterruptHandler(C40COMPORT nComPort,void *pParams);
void C40RemoveInterruptHandler(C40COMPORT nComPort,void *pStack);

void DMAINT0InterruptHandler(void *pParams);
void dmaint0(void *pStack, void *pParams);
void undmaint0(void);

void DMAINT4InterruptHandler(void *pParams);
void dmaint4(void *pStack, void *pParams);
void undmaint4(void);

void DMAINT5InterruptHandler(void *pParams);
void dmaint5(void *pStack, void *pParams);
void undmaint5(void);

#endif
