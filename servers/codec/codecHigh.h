/* $Header: codecHigh.h,v 1.1 90/08/06 12:58:29 brian Exp $ */
/* $Source: /server/usr/users/a/brian/helioshome/dev/codec/RCS/codecHigh.h,v $ */
/************************************************************************/ 
/* codecHigh.h - Header file for codec device driver library wrapper	*/
/*									*/
/* Copyright (C) 1989 Active Book Company Ltd., Cambridge, England	*/
/*									*/
/* Author: Nick Reeves, November 1989					*/
/************************************************************************/

/*
 * $Log:	codecHigh.h,v $
 * Revision 1.1  90/08/06  12:58:29  brian
 * Initial revision
 * 
 * Revision 1.2  90/02/06  11:00:25  brian
 * First released version.
 * 
 * Revision 1.1  90/02/05  13:12:22  brian
 * Initial revision
 * 
 */


#ifndef CODECHIGH_H
#define CODECHIGH_H

#include <dev/result.h>

#define CODECHIGH_ID RESULT_SOFTUNIT(1)

/*CONTENTS:
 * TYPE DEFINITIONS
 * FUNCTION PROTOTYPES - STREAM AND BUFFER MANAGEMENT
 * FUNCTION PROTOTYPES - TELEPHONE LINE HANDLING
 * GENERAL ERRORS
 * SPECIFIC ERRORS
 */


/************************************************************************/
/* TYPE DEFINITIONS							*/
/************************************************************************/

typedef int CodecHigh_BufHandle;	/* buffer handle */

typedef int CodecHigh_DevHandle;	/* device handle */

typedef enum CodecHigh_DialMode
{
  CodecHigh_DialDTMF,		/* Tone dialling  */
  CodecHigh_DialPulseCode	/* Pulse dialling */
} CodecHigh_DialMode;


typedef int CodecHigh_Gain;	/* setting of variable gain block */


typedef enum CodecHigh_Mode	/* input or output mode */
{
  CodecHigh_ModeInput,
  CodecHigh_ModeOutput
} CodecHigh_Mode;


typedef enum CodecHigh_Route	/* signal source or destination */
{
  CodecHigh_RouteLine,		/* phone line */
  CodecHigh_RouteMike,		/* microphone */
  CodecHigh_RouteSpeaker	/* loudspeaker */
} CodecHigh_Route;


typedef struct CodecHigh_Status	/* status info */
{
  unsigned state:3;		/* see states below */
  unsigned bufsUnready:4;	/* # buffers queued and not ready */
  unsigned bufsReady:4;		/* # buffers queued and ready */
  unsigned errorFlag:1;		/* set if error condition */
} CodecHigh_Status;


#define CodecHigh_StateClosed    0	/* not an open stream */
#define CodecHigh_StateQuiescent 1	/* before CodecHigh_Begin call */
#define CodecHigh_StateBegun     2	/* await stimulus after begin */
#define CodecHigh_StateBusy      3	/* busy / running */
#define CodecHigh_StateEnding    4	/* termination pending after finish */
#define CodecHigh_StateEndedOK   5	/* normally terminated */
#define CodecHigh_StateBadEnd    6	/* overrun, underrun, or abort */


typedef enum CodecHigh_Switch	/* start or end condition */
{
  CodecHigh_SwitchQueue,	/* switch on call to CodecHigh_Queue */
  CodecHigh_SwitchPTT		/* switch on state of PTT button */
} CodecHigh_Switch;


/************************************************************************/
/* FUNCTION PROTOTYPES - STREAM AND BUFFER MANAGEMENT			*/
/************************************************************************/

/*----------------------------------------------------------------------*/
/* CodecHigh_Abort - Immediately aborts current I/O on that stream and	*/
/* releases all buffers. A call to CodecHigh_Begin is needed before     */
/* more I/O can be done.						*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_Abort
(
  CodecHigh_DevHandle devHandle /* as returned by CodecHigh_Open() */
);


/*----------------------------------------------------------------------*/
/* CodecHigh_Begin - Grabs the input/output signal lines (e.g. from	*/
/* automatic speech recognition and indicates the stimuli to start and	*/
/* end sampling.							*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_Begin
(
  CodecHigh_DevHandle devHandle,/* as returned by CodecHigh_Open()	*/
  CodecHigh_Switch start,	/* specifies start condition		*/
  CodecHigh_Switch end,		/* specifies end condition		*/
  int timeout			/* timeout in seconds (-ve => never)	*/
);				/*   for calls to CodecHigh_Wait	*/


/*----------------------------------------------------------------------*/
/* CodecHigh_Close - Closes a stream.					*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_Close
(
  CodecHigh_DevHandle devHandle	/* as returned by CodecHigh_Open() */
);


/*----------------------------------------------------------------------*/
/* CodecHigh_Enquire - Returns status of specified buffer.		*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_Enquire
(
  CodecHigh_DevHandle devHandle, /* as returned by CodecHigh_Open()	*/
  CodecHigh_BufHandle bufHandle, /* as returned by CodecHigh_Queue()	*/

  int *flag			 /* receives status: <>0 => buffer ready,  */
);				 /*  0 => buffer still busy or not started */


/*----------------------------------------------------------------------*/
/* CodecHigh_Finish - Indicates that the immediately previous buffer	*/
/* queued is the final buffer so I/O should be terminated at the end of	*/
/* that buffer, if that end condition was set by CodecHigh_Begin.	*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_Finish
(
  CodecHigh_DevHandle devHandle, /* as returned by CodecHigh_Open() */

  CodecHigh_BufHandle *bufHandle /* receives last buffer handle */
);


/*----------------------------------------------------------------------*/
/* CodecHigh_Open - Opens a codec device stream. Returns a negative	*/
/* Result for error.							*/
/*----------------------------------------------------------------------*/
CodecHigh_DevHandle 
CodecHigh_Open
(
  char            *deviceName,	/* name of device to open      */
  CodecHigh_Mode  mode,		/* input or output             */
  int             sampleRate,	/* input and output must match */
  CodecHigh_Route route		/* signal source/destination   */
);


/*----------------------------------------------------------------------*/
/* CodecHigh_Queue - Queues a buffer for I/O transfer. Returns a	*/
/* negative Result for error.						*/
/*----------------------------------------------------------------------*/
CodecHigh_BufHandle 
CodecHigh_Queue
(		
  CodecHigh_DevHandle devHandle, /* as returned by CodecHigh_Open() 	*/
  void *bufferStart,		 /* pointer to buffer 			*/
  int bufferLength		 /* length of buffer in bytes 		*/
);


/*----------------------------------------------------------------------*/
/* CodecHigh_ReadStatus - Reads the status of a device stream.		*/
/*----------------------------------------------------------------------*/
Result
CodecHigh_ReadStatus
(
  CodecHigh_DevHandle devHandle, /* as returned by CodecHigh_Open() 	*/
  CodecHigh_Status    *status	 /* receives current status		*/
);


/*----------------------------------------------------------------------*/
/* CodecHigh_Wait - Waits for a queued buffer.				*/
/*----------------------------------------------------------------------*/
Result
CodecHigh_Wait
(
  CodecHigh_DevHandle devHandle, /* as returned by CodecHigh_Open()          */
  CodecHigh_BufHandle bufHandle, /* as returned by CodecHigh_Queue()         */
  int *transferred		 /* receives actual no. of bytes transferred */
);


/************************************************************************/
/* FUNCTION PROTOTYPES - TELEPHONE LINE HANDLING			*/
/************************************************************************/

/*----------------------------------------------------------------------*/
/* CodecHigh_Dial - Dials the given telephone number and waits till	*/
/* answered or timeout. (I.e. goes offhook, waits for dial tone, dials  */
/* digits, waits for ringing tone, waits for end of ringing tone.)	*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_Dial
(
  CodecHigh_DevHandle devHandle,/* as returned by CodecHigh_Open() 	*/
  CodecHigh_DialMode dialMode,	/* DTMF or pulse code 			*/
  char *phoneString,		/* ASCII string of number to dial 	*/

  int *flag			/* receives status: 0 => answered ok 	*/
);				/*  <>0 => timeout			*/


/*----------------------------------------------------------------------*/
/* CodecHigh_Answer - Auto-answers the telephone on receipt of ring 	*/
/* tone and waits until called or timeout. (I.e. waits for ringing	*/
/* tone, waits for enough ringing, goes offhook.)			*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_Answer
(
  CodecHigh_DevHandle devHandle, /* as returned by CodecHigh_Open() 	    */
  int                 ringTime,	 /* ring time (in seconds) before answering */
  int                 timeout,	 /* timeout in seconds ( <0 for never)      */

  int                 *flag	 /* receives status: 0 => answered ok	    */
);				 /*  <>0 => timeout 			    */


/*----------------------------------------------------------------------*/
/* CodecHigh_HangUp - Hangs up phone line.				*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_HangUp
(
  CodecHigh_DevHandle devHandle	/* as returned by CodecHigh_Open() */
);


/*----------------------------------------------------------------------*/
/* CodecHigh_SetGain - Sets Variable Gain Block.			*/
/*----------------------------------------------------------------------*/
Result 
CodecHigh_SetGain
(
  CodecHigh_DevHandle devHandle, /* as returned by CodecHigh_Open() */
  CodecHigh_Gain      newGain,	 /* new VGB setting 		    */
 
  CodecHigh_Gain      *oldGain	 /* receives old VGB setting	    */
);


/************************************************************************/
/* GENERAL ERRORS							*/
/************************************************************************/

/* parameter value out of range */
#define CODECHIGH_GERRORPARAMETER GERROR(CODECHIGH, Parameter, Error)

/* resource in use or locked */
#define CODECHIGH_GERRORINUSE GERROR(CODECHIGH, InUse, Error)

/************************************************************************/
/* SPECIFIC ERRORS							*/
/************************************************************************/

/* parameter values confict */
#define CODECHIGH_SERRORPARAMCONFLICT SERROR(CODECHIGH, 254, Error)

/* out of sequence function call */
#define CODECHIGH_SERRORFUNCORDER SERROR(CODECHIGH, 253, Error)

/* out of sequence buffer for CodecHigh_wait */
#define CODECHIGH_SERRORBUFORDER SERROR(CODECHIGH, 252, Error)

/* unknown device stream handle */
#define CODECHIGH_SERRORDEVHANDLE SERROR(CODECHIGH, 251, Error)

/* unknown buffer handle */
#define CODECHIGH_SERRORBUFHANDLE SERROR(CODECHIGH, 250, Error)

/* underrun */
#define CODECHIGH_SERRORUNDERRUN SERROR(CODECHIGH, 249, Error)

/* overrun */
#define CODECHIGH_SERROROVERRUN SERROR(CODECHIGH, 248, Error)

/* hardware error */
#define CODECHIGH_SERRORHARDWARE SERROR(CODECHIGH, 247, Error)

/* timeout */
#define CODECHIGH_SERRORTIMEOUT SERROR(CODECHIGH, 246, Error)

/* not implemented */
#define CODECHIGH_SERRORNOTIMPLEMENTED SERROR(CODECHIGH, 245, Error)

/* no device */
#define CODECHIGH_SERRORNODEVICE SERROR(CODECHIGH, 244, Error)

#endif

/*----------------------------------------------------------------------*/

/* End of codecHigh.h */
