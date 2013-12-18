/* $Header: codec.h,v 1.1 90/08/06 12:58:21 brian Exp $ */
/* $Source: /server/usr/users/a/brian/helioshome/dev/codec/RCS/codec.h,v $ */
/************************************************************************/ 
/* codec.h - Header for ARM Helios device driver for CODEC podule	*/
/*	     on AB functional prototype.				*/
/*									*/
/* Copyright (C) 1989 Active Book Company Ltd., Cambridge, England	*/
/*									*/
/* Author: Brian Knight,  30th October 1989				*/
/************************************************************************/

/*
 * $Log:	codec.h,v $
 * Revision 1.1  90/08/06  12:58:21  brian
 * Initial revision
 * 
 */

#ifndef __CODEC_H
#define __CODEC_H

#ifndef __device_h
#include <device.h>
#endif

#include <dev/podule.h>
#include <dev/result.h>

/* #define TRACING	     1 */    /* Set to activate in-core trace buffer */

#define CODEC_ID RESULT_SOFTUNIT(2)
#define MAXCODECDEVICES 1 /* Maximum number of codec devices */

/* Info parameter to DevOpen() */

typedef struct CodecDevInfo
{
  int logDevNum; /* Logical no. of device to open (0..MAXCODECDEVICES-1) */
} CodecDevInfo;
        
typedef unsigned int Codec_BufHandle; /* Handles used for active buffers */

/* Buffer stream status structure returned by FC_ReadInputStatus and	*/
/* FC_ReadOutputStatus.							*/
typedef struct Codec_Status
{
  unsigned state:3;	  /* See states below 				  */
  unsigned bufsUnready:4; /* No. of buffers queued but not filled/emptied */
  unsigned bufsReady:4;   /* No. of buffers queued and ready 		  */
  unsigned errorFlag:1;	  /* Set if there is an error condition 	  */
} Codec_Status;

/* Possible states of a buffer stream */

#define Codec_StateInvalid   0	/* Not used in open devices		*/
#define Codec_StateQuiescent 1	/* Awaiting FC_BeginRead or FC_BeginWrite */
#define Codec_StateBegun     2  /* Begun but still waiting for stimulus	*/
#define Codec_StateBusy	     3  /* Reading or writing buffers		*/
#define Codec_StateEnding    4	/* Termination pending after FC_Finish* */
#define Codec_StateEndedOK   5	/* Normally terminated			*/
#define Codec_StateBadEnd    6	/* Overrun, underrun or abort		*/

/* Codec device requests. This is a standard serial device */

typedef SerialReq CodecReq;

/*----------------------------------------------------------------------*/
/* Function Codes 							*/
/*									*/
/* Beware of the confusing terminology!					*/
/* The data directions are specified as the codec chip's view of its    */
/* digital side. Hence:							*/
/*									*/
/* "Receive"  means "convert digital to analogue" (write)		*/
/* "Transmit" means "convert analogue to digital" (read)		*/ 
/*----------------------------------------------------------------------*/

#define FC_EnableRead	 (FG_Read  | 0x1)   /* called before first read      */
#define FC_QueueReadBuf  (FG_Read  | 0x2)   /* queue a buffer for reading    */
#define FC_FinishRead	 (FG_Read  | 0x3)   /* previous buffer was final one */
#define FC_AbortRead	 (FG_Read  | 0x4)   /* abort read, release buffers   */
#define FC_BufWait       (FG_Read  | 0x5)   /* wait for read or write buffer */

#define FC_EnableWrite	 (FG_Write | 0x1)   /* called before first write     */
#define FC_QueueWriteBuf (FG_Write | 0x2)   /* queue a buffer for writing    */
#define FC_FinishWrite	 (FG_Write | 0x3)   /* previous buffer was final one */
#define FC_AbortWrite	 (FG_Write | 0x4)   /* abort write, release buffers  */
#define FC_PulseDial	 (FG_Write | 0x5)   /* pulse dial the given number   */

#define FC_ReadRawStatus    (FG_GetInfo | 0x0) /* return h/w status flags  */
#define FC_ReadInputStatus  (FG_GetInfo | 0x1) /* return driver status     */
#define FC_ReadOutputStatus (FG_GetInfo | 0x2) /* return driver status     */
#define FC_Enquire	    (FG_GetInfo | 0x3) /* enquire about one buffer */

#ifdef TRACING
#define FC_GetTraceBuf	    (FG_GetInfo | 0x4) /* find location of trace buf */
#endif

#define FC_LPLow	 (FG_SetInfo | 0x0) /* LP = 0 */
#define FC_LPHigh	 (FG_SetInfo | 0x1) /* LP = 1 */
#define FC_OffHook	 (FG_SetInfo | 0x2) /* Take phone line off hook */
#define FC_OnHook	 (FG_SetInfo | 0x3) /* Put phone line on hook */
#define FC_PhoneInput	 (FG_SetInfo | 0x4) /* Audio input from telephone */
#define FC_MicInput	 (FG_SetInfo | 0x5) /* Audio input from microphone */
#define FC_PhoneOutput	 (FG_SetInfo | 0x6) /* Audio output to telephone */
#define FC_SpeakerOutput (FG_SetInfo | 0x7) /* Audio output to loudspeaker */


/* Flag bits in result of FC_RawStatus (some useful for debugging only) */
/* Note that these are inverted flags.                               */

/*				0x01	   unused			*/
#define CODEC_STATUSTNFF	0x02	/* tx fifo not full		*/
#define CODEC_STATUSTNHF	0x04	/* tx fifo less than half full	*/
#define CODEC_STATUSTNEF	0x08	/* tx fifo not empty		*/
#define CODEC_STATUSRNFF	0x10	/* rx fifo not full		*/
#define CODEC_STATUSRNHF	0x20	/* rx fifo less than half full	*/
#define CODEC_STATUSRNEF	0x40	/* rx fifo not empty		*/
#define CODEC_STATUSNRI		0x80	/* not ringing indicator     	*/

/*----------------------------------------------------------------------*/

#ifdef TRACING
# define TR_REQUEST    101
# define TR_READINT    102
# define TR_WRITEINT   103
# define TR_STARTWAIT  104
# define TR_QREADBUF   105
# define TR_QWRITEBUF  106
# define TR_OVERRUN    107
# define TR_UNDERRUN   108
# define TR_READSIG    109
# define TR_READNEXT   110
# define TR_READEXIT   111
# define TR_WRITESIG   112
# define TR_WRITENEXT  113
# define TR_WRITEEXIT  114
# define TR_ENDWAIT    115
# define TR_REN	       116
# define TR_STARTWRITE 117
#endif

/*----------------------------------------------------------------------*/

/* Specific Errors */

/* Overrun on reading */
#define CODEC_SERROROVERRUN	SERROR(CODEC, 255, Error)

/* Underrun on writing */
#define CODEC_SERRORUNDERRUN	SERROR(CODEC, 254, Error)

/* Attempt to do two reads or writes in parallel (can't happen!) */
#define CODEC_SERRORINUSE	SERROR(CODEC, 253, Error)

/* Bad buffer handle */
#define CODEC_SERRORBUFHANDLE	SERROR(CODEC, 252, Error)

/* Function call out of sequence */
#define CODEC_SERRORFUNCORDER	SERROR(CODEC, 251, Error)

/* Operation attempted on finishing stream */
#define CODEC_SERRORFINISHING	SERROR(CODEC, 250, Error)

/* Too many I/O buffers queued at once */
#define CODEC_SERRORTOOMANYBUFS	SERROR(CODEC, 249, Error)

/* No buffers left */
#define CODEC_SERRORNOBUFFERS	SERROR(CODEC, 248, Error)

/* Request for out-of-order buffer */
#define CODEC_SERRORBUFORDER	SERROR(CODEC, 247, Error)

/* Invalid phone number */
#define CODEC_SERRORPHONENUMBER SERROR(CODEC, 246, Error)

/************************************************************************/

#endif /* __CODEC_H */

/* End of codec.h */
