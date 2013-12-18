/*> ABClib/h <*/
/*---------------------------------------------------------------------------*/
/* Functions exported from ABClib                                            */
/* Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom.       */
/*---------------------------------------------------------------------------*/

#ifndef __ABClib_h
#define __ABClib_h

/*---------------------------------------------------------------------------*/

#include <helios.h>             /* for standard definitions */
#include <queue.h>              /* standard queue chain structures */
#include <abcARM/ROMitems.h>	/* CARD BLOCK definitions */

/*---------------------------------------------------------------------------*/
/*-- FIQ --------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* AttachFIQ
 * ---------
 */
extern int  AttachFIQ(word pri,word latency,void *hw,word mask,void *code,word length,word *regset) ;

/* ReleaseFIQ
 * ----------
 */
extern int  ReleaseFIQ(void *hw,word mask) ;

/* DefaultFIQStack
 * ---------------
 */
extern word DefaultFIQStack(void) ;

/*---------------------------------------------------------------------------*/
/*-- IRQ --------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* IntsOff
 * IntsOn
 * -------
 * These functions, respectively, disable and enable IRQs to the processor.
 * in:  no conditions
 * out: no conditions
 */
extern void IntsOff(void) ;		/* disable processor IRQs */
extern void IntsOn(void) ;		/* enable processor IRQs */

/* DisableIRQ
 * EnableIRQ
 * ----------
 * These functions allow individual IRQ sources to be disabled/enabled.
 * in:  Bitmask of the IRQ source to operate on.
 * out: The old IRQ mask.
 */
extern word DisableIRQ(word mask) ;	/* disable IRQ source */
extern word EnableIRQ(word mask) ;	/* enable IRQ source */

/*---------------------------------------------------------------------------*/
/*-- Processes --------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* SetPhysPri
 * ----------
 * This call allows you to change your current process priority. Note: This
 * will only change the priority of this process and NOT the priority of other
 * processes within this task. The function "GetPhysPri()" (in kernel) should
 * be used to interrogate the priority level without updating the current
 * value.
 * in:  desired priority level
 * out: previous priority level
 */
extern word SetPhysPri(word newpri) ;

/*---------------------------------------------------------------------------*/
/*-- EEPROM support ---------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* ReadEEPROM
 * ----------
 * This function returns the byte at the given location in the uController
 * EEPROM.
 * in:  index = index into EEPROM memory of the required byte location
 * out: 8bit value read from the given location
 */
extern byte ReadEEPROM(word index) ;

/* WriteEEPROM
 * -----------
 * This function writes the byte to the given location in the uController
 * EEPROM.
 * in:  index = index into EEPROM memory of the required byte location
 *	value = 8bit value to write to the given location
 * out: 8bit value written to the location
 */
extern byte WriteEEPROM(word index,byte value) ;

/* This is a list of the value indices used by the Helios system */
#define	EEPROM_ServerID		(0)	/* "ServerIndex" value defined below */
#define EEPROM_Stage1Timeout	(1)	/* ((n + 1)  * 30) seconds timeout */
#define EEPROM_Stage2Timeout	(2)	/* (n + 10) seconds timeout */
/* IDD (0-99) country code used to determine what keyboard map to load */
#define EEPROM_KeyMap		(3)	/* top bit set = swap caps/ctrl keys */
/* following 2 keyboard delay values are expressed in microseconds */
#define EEPROM_KeyStartDelay	(4)	/* delay before initial repeat */
#define EEPROM_KeyInterDelay	(5)	/* delay between each repeat */

/*---------------------------------------------------------------------------*/
/*-- RESET support ----------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* ResetKeyState
 * -------------
 * This call returns the bitmap of the special keys pressed during RESET. It
 * is provided to allow system software to act upon the key requests late
 * in the initialisation sequence.
 * in:  no conditions
 * out: bitmap of special keys pressed during RESET sequence
 */
extern word ResetKeyState(void) ;

/* This is a list of the special key bit-positions in the word returned above.
 * The actual list will be generated from the uController information.
 */
typedef enum {
              ShellBootKey 	= 0,	/* enter the Helios shell     (H) */
	      ZeroMemKey 	= 1,	/* zero memory                (E) */
	      IgnorePatchKey 	= 2	/* ignore FlashEPROM patches  (E) */
             } SpecialKeys ;
/* (E) - Executive handles these cases
 * (H) - Helios kernel handles these cases
 */

/* ServerIndexToName
 * -----------------
 * This will convert the special index byte (as held in EEPROM location
 * "EEPROM_ServerID") to the NULL terminated ASCII name of the relevant
 * server.
 * NOTE: The string returned is the address of the text in a ROM structure.
 *       Code should not treat the returned string as workspace.
 * in:  server index (as stored in EEPROM location "EEPROM_ServerID")
 * out: NULL terminated ASCII server name
 */
extern const char *ServerIndexToName(byte index) ;

/* NameToServerIndex
 * -----------------
 * This performs the opposite operation to "ServerIndexToName". The name
 * can be wildcarded.
 * in:  NULL terminated ASCII name (possibly wildcarded)
 * out: server index (as stored in EEPROM location "EEPROM_ServerID")
 */
extern byte NameToServerIndex(char *name) ;

/* This is a list of the ServerIndex ID numbers for the servers known about
 * by this release. If the system ever processes a number it does not
 * recognise, it should either use the "default value 0" or the special
 * case "shell 256". NOTE: This limits us to 255 known server startups.
 */
typedef enum {
              /* 0..10 special cases */
              EEPROM_ServerIndexDefault		= 0,
	      EEPROM_ServerIndexIOServer	= 1,
	      /* 10..19 ROM CARDs */
	      EEPROM_ServerIndexSysROM		= 10,
	      EEPROM_ServerIndexROMCard1	= 11,
	      EEPROM_ServerIndexROMCard2	= 12,
	      EEPROM_ServerIndexROMCard3	= 13,
	      EEPROM_ServerIndexROMCard4	= 14,
	      EEPROM_ServerIndexROMCard5	= 15,
	      EEPROM_ServerIndexROMCard6	= 16,
	      EEPROM_ServerIndexROMCard7	= 17,
	      EEPROM_ServerIndexROMCard8	= 18,
	      EEPROM_ServerIndexROMCard9	= 19,
	      /* 20..29 RAM CARDs */
	      EEPROM_ServerIndexSysRAM		= 20,
	      EEPROM_ServerIndexRAMCard1	= 21,
	      EEPROM_ServerIndexRAMCard2	= 22,
	      EEPROM_ServerIndexRAMCard3	= 23,
	      EEPROM_ServerIndexRAMCard4	= 24,
	      EEPROM_ServerIndexRAMCard5	= 25,
	      EEPROM_ServerIndexRAMCard6	= 26,
	      EEPROM_ServerIndexRAMCard7	= 27,
	      EEPROM_ServerIndexRAMCard8	= 28,
	      EEPROM_ServerIndexRAMCard9	= 29,
	      /* 30..39 Helios filesystem */
	      EEPROM_ServerIndexFS		= 30,
	      /* 40..99 unallocated */
	      /* 100..125 msdos drives */
	      EEPROM_ServerIndexMSDOSa		= 100,
	      EEPROM_ServerIndexMSDOSb		= 101,
	      EEPROM_ServerIndexMSDOSc		= 102,
	      EEPROM_ServerIndexMSDOSd		= 103,
	      EEPROM_ServerIndexMSDOSe		= 104,
	      EEPROM_ServerIndexMSDOSf		= 105,
	      EEPROM_ServerIndexMSDOSg		= 106,
	      EEPROM_ServerIndexMSDOSh		= 107,
	      EEPROM_ServerIndexMSDOSi		= 108,
	      EEPROM_ServerIndexMSDOSj		= 109,
	      EEPROM_ServerIndexMSDOSk		= 110,
	      EEPROM_ServerIndexMSDOSl		= 111,
	      EEPROM_ServerIndexMSDOSm		= 112,
	      EEPROM_ServerIndexMSDOSn		= 113,
	      EEPROM_ServerIndexMSDOSo		= 114,
	      EEPROM_ServerIndexMSDOSp		= 115,
	      EEPROM_ServerIndexMSDOSq		= 116,
	      EEPROM_ServerIndexMSDOSr		= 117,
	      EEPROM_ServerIndexMSDOSs		= 118,
	      EEPROM_ServerIndexMSDOSt		= 119,
	      EEPROM_ServerIndexMSDOSu		= 120,
	      EEPROM_ServerIndexMSDOSv		= 121,
	      EEPROM_ServerIndexMSDOSw		= 122,
	      EEPROM_ServerIndexMSDOSx		= 123,
	      EEPROM_ServerIndexMSDOSy		= 124,
	      EEPROM_ServerIndexMSDOSz		= 125,
	      /* 126..254 unallocated */
	      /* 255 special Shell entry */
              EEPROM_ServerIndexShell		= 255
             } ServerIndex ;

/*---------------------------------------------------------------------------*/
/*-- Display ----------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* DisplayInfo
 * -----------
 * This call is used to interrogate the screen position and shape.
 * in:  screen = pointer to word to contain the screen start address
 *	stride = pointer to word to contain the raster stride (in bytes)
 *	X      = pointer to word to contain the displayed raster width (pixels)
 *	Y      = pointer to word to contain the number of rasters
 * out: returns 0 if display present and valid, otherwise -1
 */
extern int DisplayInfo(word *screen,word *stride,int *X,int *Y) ;

/*---------------------------------------------------------------------------*/
/*-- CARD -------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* The system supports ABC (vendor) specific JEIDA v.4 formatted CARDs	     */

typedef struct CardEvent {
                          struct Node  Node ;   /* link in CardEvent chain */
                          word         Type ;   /* bitmask of CARD types */
                          WordFnPtr    Handler ;/* CARD handler function */
                          void        *data ;   /* private data for handler */
                          /* the following is set by the Kernel/Executive */
                          word        *ModTab ; /* module table pointer */
                         } CardEvent ;

/* CardSlots
 * ---------
 * in:  no conditions
 * out: returns the number of slots (1..n) available
 */
extern int  CardSlots(void) ;      

/* CardDefineEventHandler
 * CardReleaseEventHandler
 * -----------------------
 * These calls provide the means for registering/de-registering handlers for
 * CARDs containing a set of types (presented as a bitmask).
 * in:  structure defining the handler environment
 * out: successful completion boolean
 */
extern int  CardDefineEventHandler(CardEvent *cardinfo) ;
extern int  CardReleaseEventHandler(CardEvent *cardinfo) ;

/* CardStatus
 * ----------
 * This call returns CARD presence state for a particular slot. If a CARD
 * is available in the given slot then other useful information is returned
 * in the passed variables. The battery presence/level value is returned
 * directly from the hardware (refer to the relevant hardware header file).
 * in:  slot number
 *	pointer to word where the area type bitmask is to be stored
 *	pointer to word where the number of areas value is to be stored
 *	pointer to word where the battery presence/level value is to be stored
 *	pointer to word where the write-protect-switch status is to be stored
 *	pointer to word where the pointer to the CARD name will be stored
 * out: returns error code (as defined below for "CardInfo")
 */
extern int  CardStatus(int slot,word *types,word *numareas,word *battery,word *writeprotect,char **cardname) ;

/* The battery state is returned as a 2bit value */
/* THE FOLLOWING INFORMATION SHOULD REALLY BE DERIVED FROM THE RELEVANT
 * HARDWARE DESCRIPTION HEADER FILE.
 */
#if 1	/* simple view */
/* The JEIDA specification does not provide a proper battery presence signal.
 * Therefore we cannot tell easily if the CARD is supposed to be used for
 * RAM filing system, or for heap.
 */
#define BATTERY_PRESENT	(1 << 0)	/* battery is present */
#define BATTERY_OK	(1 << 1)	/* battery is good */
#else	/* JEIDA view of the bit meanings */
#define BATTERY_OK	(0x3)
#define BATTERY_FAILING	(0x1)
#define BATTERY_FAILED	(0x2)	/* JEIDA attach BAD to this aswell */
#define BATTERY_BAD	(0x0)
#endif

/* CardInfo
 * --------
 * This call will return information about a particular area of a CARD.
 * in:  slot number
 *      area number
 *      pointer to word where the area type bitmask is to be stored
 *	pointer to word where the area size value is to be stored
 *	pointer to word where the area base address is to be stored
 * out: returns error code as follows:
 */
#define CARDerr_none		(0)  /* no error - successful completion */
#define CARDerr_badslot		(-1) /* invalid slot number (1..CARD_limit) */
#define CARDerr_nocard		(-2) /* no CARD present in the slot */
#define CARDerr_badformat       (-3) /* CARD contains invalid CIS structure */
#define CARDerr_badsum          (-4) /* CIS checksum failure */
#define CARDerr_badarea         (-5) /* invalid area number (defined in CIS) */

extern int CardInfo(int slot,int area,word *type,word *size,word *addr) ;

/* AcceptCardExtraction
 * RefuseCardExtraction
 * --------------------
 * These functions are called by the handlers during a CARD removal event.
 * They provide a call-back into the system-wide CARD handler system so that
 * it can respond accordingly to actual CARD removal.
 * in:  slot number of CARD
 * out: no conditions
 */
extern void AcceptCardExtraction(int slot) ;
extern void RefuseCardExtraction(int slot) ;

/* CardHandler
 * -----------
 * This function does NOT exist. This information is provided purely for
 * reference.
 *
 * word CardHandler(bool insert,int slot,word type) ;
 * where:
 *  insert	: TRUE is CARD is being inserted; FALSE if being removed
 *  slot	: slot number of the CARD
 *  type        : bitmask of types present on this CARD
 *
 * NOTE: The CardHandler is only called if at least one area is of the correct
 *       type. There may be multiple areas of the correct type, with calls to
 *	 "CardInfo" being required to ascertain the number of areas and their
 *	 indices.
 */

/* NOTE: The currently allocated CARD type bit-positions are defined in the
 *       header file "PCcard.h"
 */

/* CardRemovedEvent
 * ----------------
 * This function is used by external applications that may wish to
 * re-initialise a CARD (ie. format a RAMFS CARD). It will simulate the
 * necessary CARD removal sequence to notify any suitable handlers that
 * may be present.
 * in:  slot number
 * out: -1 (TRUE) if CARD has been successfully "removed", or 0 (FALSE) if
 *      a handler called "CardRefuseExtraction".
 */
extern int CardRemovedEvent(int slot) ;

/* CardInsertedEvent
 * -----------------
 * This function is used by external applications that may have initialised
 * a CARD. It will simulate the necessary CARD insertion sequence to notify
 * any suitable handlers that may be present.
 * in:  slot number
 * out: no conditions
 */
extern void CardInsertedEvent(int slot) ;

/*---------------------------------------------------------------------------*/
/*-- FlashEPROM -------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* This should really be in the hardware description */
#define flash_size      (128 << 10)     /* 128K FlashEPROM size */

/* FlashCheck
 * ----------
 * in:  no conditions
 * out: returns 0 if FlashEPROM not found (unrecognised) otherwise -1
 */
extern int FlashCheck(void) ;

/* FlashErase
 * ----------
 * in:  no conditions
 * out: returns -1 if the operation succeeded, otherwise 0
 */
extern int FlashErase(void) ;

/* FlashVerify
 * -----------
 * in:  buffer = pointer to the byte aligned buffer address
 *      amount = number of bytes in the buffer
 * out: returns -1 if the buffer matches the FlashEPROM, otherwise 0
 */
extern int FlashVerify(char *buffer,word amount) ;

/* FlashWrite
 * ----------
 * in:  buffer = data bytes to be written to &00000000 in the FlashEPROM
 *      amount = number of bytes to write
 * out: -1 write succeeded
 */
extern int FlashWrite(char *buffer,word amount) ;

/*---------------------------------------------------------------------------*/
/*-- Memory Map support -----------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* ClaimHWMemory
 * -------------
 * This functions places the calling process into hardware mode. This allows
 * user processes to access the IO memory of the Active Book. It is primarily
 * designed for device drivers that need to enable/disable hardware events
 * outside IRQ/SVC mode. This state should be held for a minimal amount of
 * time... and should be reverted using the "ReleaseHWMemory" call.
 * in:  no conditions
 * out: no conditions
 */
void ClaimHWMemory(void) ;

/* ReleaseHWMemory
 * ---------------
 * This call reverts the process back to the normal user memory map.
 * in:  no conditions
 * out: no conditions
 */
void ReleaseHWMemory(void) ;

/* HWRegisters
 * -----------
 * in:  index = register number to interrogate/update (see file "SWI.s")
 *      bic   = bitmask of bits to clear
 *      orr   = bitmask of bits to set
 *      old   = pointer to word to be written with the original value
 *      new   = pointer to word to be written with the updated value
 * out: returns 0 if operation succeeded, -1 if FAILED to write.
 */
int HWRegisters(int index,word bic,word orr,word *old,word *new) ;

/* ROHWRegisters
 * -------------
 * in:  index = register number to interrogate (see file "SWI.s")
 *	value = pointer to word to be written with the value read
 * out: returns 0 if operation succeeded, -1 if FAILED to write.
 */
int ROHWRegisters(int index,word *value) ;

/*---------------------------------------------------------------------------*/
/*-- Microlink --------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* ML_Transmit - Transmit a microlink message to the microcontroller.	     */
/* 									     */
/* On entry, `buf' should contain a message conforming to the microlink	     */
/* protocol. The length is implicit in the message header.		     */
/* 									     */
/* Only one transmission may be outstanding at once: if there is already one */
/* in progress, this routine returns immediately with a negative error code. */
/*									     */
/* Otherwise, the routine does not return until the message has been sent    */
/* (even if the microlink channel has been reset by a break signal in the    */
/* meantime). It then returns zero.					     */
/*---------------------------------------------------------------------------*/
word ML_Transmit(ubyte *buf);


/*---------------------------------------------------------------------------*/
/* ML_SetUpRx - Set up a reception buffer for a microlink message.	     */
/* 									     */
/* On entry, `buf' should point to a buffer big enough to hold the largest   */
/* message of the specified type (1 byte for a short message, or 34 bytes    */
/* for a long/extended message).					     */
/* `msgType' contains the short/long flag and the type field for the message */
/* of interest, so has one of the formats:				     */
/*									     */
/*          0TTTTxxx for a short message of type TTTT, or		     */
/*	    1TTTTTxx for a long or extended message of type TTTTT	     */
/*									     */
/* The data and length code bits (`x' above) are ignored here.		     */
/*									     */
/* This call returns immediately; ML_WaitForRx is used to wait for a message */
/* to arrive in the buffer.						     */
/*									     */
/* If the call fails, the result is a negative Helios error code.	     */
/* If it succeeds the result is a handle which should be passed to a later   */
/* call of ML_WaitForRx.						     */
/*---------------------------------------------------------------------------*/
word ML_SetUpRx(ubyte *buf, ubyte msgType);


/*---------------------------------------------------------------------------*/
/* ML_WaitForRx - wait for a reception buffer to receive a message.	     */
/*									     */
/* `rxHandle' should be a handle previously issued by ML_SetUpRx.	     */
/* `timeout'  is in microseconds.					     */
/*									     */
/* The result is zero if a message is successfully received into the buffer  */
/* associated with `rxHandle'; the handle then becomes invalid. The length   */
/* of the message is implicit in its header.				     */
/*									     */
/* A negative result is an error code indicating why nothing was received    */
/* (e.g. handle invalid, timeout expired).				     */
/*---------------------------------------------------------------------------*/
word ML_WaitForRx(word rxHandle, word timeout);


/*---------------------------------------------------------------------------*/
/* Structure used to describe an event message handler.			     */
/*---------------------------------------------------------------------------*/

typedef struct ML_MsgHandler
{
  ubyte	    msgType;		/* Type of message to be handled	*/
  VoidFnPtr func;		/* Function to be called		*/
  void      *arg;		/* Argument to be passed to function	*/
  word	    *modTab;		/* For use by kernel			*/
  struct ML_MsgHandler *next;	/* For use by kernel			*/
} ML_MsgHandler;


/*---------------------------------------------------------------------------*/
/* ML_RegisterHandler - provide a handler function for an event message.     */
/*									     */
/* The result is 0 if successful or a negative error code if not.	     */
/*									     */
/* The fields `msgType', `func' and `arg' of the handler structure should    */
/* be set up before the call is made.					     */
/*									     */
/* `msgType' has the same format as in ML_SetUpRx and specifies the type of  */
/* message which `func' handles. It will normally be the type of an	     */
/* unsolicited `event' message from the microcontroller.		     */
/*									     */
/* The handler function should be declared as:				     */
/*									     */
/*   void func(ubyte *buf, void *arg);					     */
/*									     */
/* Each time that a message of the appropriate type is received, `func' will */
/* be called. Its argument `buf' will point to the buffer containing the     */
/* message. This buffer belongs to the executive and is valid only while     */
/* the handler is being called, so the handler must not write to it and must */
/* copy out any information it wants to save before exiting. `arg' will be   */
/* the value supplied when the handler was registered.			     */
/*									     */
/* Note that the handler is called in SVC mode in an executive thread, so is */
/* restricted in what it may do. Some guidelines:			     */
/*									     */
/*  - Exit as quickly as possible (say < 100us)				     */
/*  - Be frugal with the stack (say < 1Kbyte)				     */
/*  - Restrict external calls to HardenedSignal and ML_DetachHandler	     */
/*  - Access external variables only via `arg'				     */
/*									     */
/* Typically, a handler will copy some information out of the message to     */
/* somewhere accessed via `arg', and then either HardenedSignal a semaphore  */
/* or set a flag reached via `arg', to communicate with its foreground	     */
/* process.								     */
/*									     */
/* Note that the supplied handler structure will be linked to a list inside  */
/* the kernel, so the caller must ensure that it remains intact until it     */
/* is detached.								     */
/*---------------------------------------------------------------------------*/
word ML_RegisterHandler(ML_MsgHandler *handler);


/*---------------------------------------------------------------------------*/
/* ML_DetachHandler - remove a previously registered message handler.	     */
/*									     */
/* `handler' is a pointer passed to a previous call to ML_RegisterHandler.   */
/*									     */
/* The result is zero if successful, or a negative error code if not.	     */
/*---------------------------------------------------------------------------*/
word ML_DetachHandler(ML_MsgHandler *handler);


/*---------------------------------------------------------------------------*/
/* ML_Reset - reset the microlink channel to the microcontroller.	     */
/*									     */
/* This should be called only when communication with the microcontroller    */
/* appears to be failing. It initiates a break sequence to resynchronise     */
/* the channel.								     */
/*---------------------------------------------------------------------------*/
void ML_Reset(void);


/*---------------------------------------------------------------------------*/
/*-- SWI calling ------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* execSWI
 * -------
 * Perform a generic SWI call from C.
 * in:  SWInumber : SWI number
 *	inregs    : vector of words (r0..r10) to be passed to the SWI
 *	outregs   : vector where r0..r10 are returned from the SWI
 * out:	returns 0 if SWI returned V clear, -1 if SWI returned V set
 */
extern int execSWI(word SWInumber,word *inregs,word *outregs) ;

/*---------------------------------------------------------------------------*/
/*-- CRC --------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* crc_ccitt
 * ---------
 * This will calculate a 16bit CRC-CCITT style CRC value for the given data.
 * The data is treated as a stream of bytes which the CRC is generated from.
 * in:  data   : address of the first byte to be included
 *      length : length of the area to CRC (multiple of "step" bytes)
 *      step   : number of bytes between data bytes
 * out: 16bit CRC value (hi 16bits are clear)
 */
extern word crc_ccitt(char *data,word length,word step) ;

/*---------------------------------------------------------------------------*/
/*-- IDLE activity time-outs ------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* The IDLE activity time-out scheme relies on users of the system (device
 * drivers, applications, etc.) to reset the dead-mans handle that is the
 * system IDLE state.
 *
 * The power-down is accomplished in two stages:
 *	stage 1 : time-out is defined as multiples of 30seconds (1..256)
 *	stage 2 : is a single time-out between 10 and 265seconds
 *
 * When stage 1 has timed out the LCD will be turned off and all IRQ, FIQ
 * and DMA sources will be disabled (other than the microlink interface to
 * the uController). The "Power" LED will start flashing, notifying the user
 * of imminent power-down. If no further activity occurs during the stage 2
 * time-out period then the system will shutdown completely, and the "Power"
 * LED will be extinguished. NOTE: only uController events will wake the
 * system up when in stage 2 (buttons, pen, RingIn etc.).
 *
 * The current (default) timeout values can be changed by using the
 * EEPROM read/write commands defined above.
 */

/* ResetSystemTimeout
 * ------------------
 * This function is called as a direct result of some hardwired event. eg. this
 * function will be called whenever a device driver event occurs. It will
 * reset the timeout to the currently defined start value.
 * in:  32bit caller identity code (currently unused)
 * out: no conditions
 */
extern void ResetSystemTimeout(word IDcode) ;

/* ResetUserTimeout
 * ----------------
 * This function is called as a direct result of user action resetting the
 * dead-mans handle. eg. it should be called when the user performs a
 * spread-sheet recalculation that may take longer than the timeout period.
 * It will reset the timeout to the currently defined start value.
 * in:  32bit caller identity code (currently unused)
 * out: no conditions
 */
extern void ResetUserTimeout(word IDcode) ;

/* NOTE: The caller identity codes should be allocated to the various types
 * 	 of event that may force a timeout reset. At the moment this
 *	 information is not used. One possible use is in validating that all
 *	 event handlers do indeed call the reset functions at regular
 *	 intervals.
 *	 UPDATE THIS enumERATION TO INCLUDE ALL POSSIBLE EVENT SOURCES
 */
typedef enum {
              eventDigitiser,	/* digitiser pen change */
	      eventButton,	/* digitiser button change */
	      eventKeyboard,	/* keyboard state change */
	      eventSerial,	/* Serial interface event */
	      eventFax,		/* FAX Tx/Rx event */
	      eventIOCard,	/* external JEIDA v4 IO CARD event */
	      eventGeneric	/* no specific event */
             } eventIDcodes ;

/*---------------------------------------------------------------------------*/
/*-- PowerDown management functions -----------------------------------------*/
/*---------------------------------------------------------------------------*/

/* LCD off   */
/* LCD on    */
/* PowerDown */

/* The system provides support for shutting the world down after a configured
 * IDLE period. It will have internal code to provide the functionality of
 * these calls. The interface at this level is provided to allow hi-level code
 * to "switch off" the world (eg. shutdown from GUI selected by the user).
 */

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#endif /* __ABClib_h */

/*---------------------------------------------------------------------------*/
/*> EOF ABClib/h <*/
