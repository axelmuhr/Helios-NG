/*> ARMlib/h <*/
/*---------------------------------------------------------------------------*/
/* Functions exported from ARMlib (the ARM specific Helios system library)   */
/*---------------------------------------------------------------------------*/

#include <helios.h>             /* for standard definitions */
#include <queue.h>              /* standard queue chain structures */

/*---------------------------------------------------------------------------*/
/*-- EEPROM support ---------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* ReadEEPROM                                                                */
/* ----------                                                                */
/* This function returns the byte at the given location in the EEPROM.       */
/*                                                                           */
/* in:  index = index into EEPROM memory of the required byte location       */
/* out: 8bit value read from the given location                              */
/*---------------------------------------------------------------------------*/
extern byte ReadEEPROM(word index) ;

/*---------------------------------------------------------------------------*/
/* WriteEEPROM                                                               */
/* -----------                                                               */
/* This function writes the byte to the given location in the EEPROM.        */
/*                                                                           */
/* in:  index = index into EEPROM memory of the required byte location       */
/*	value = 8bit value to write to the given location                    */
/* out: 8bit value written to the location                                   */
/*---------------------------------------------------------------------------*/
extern byte WriteEEPROM(word index,byte value) ;

/*---------------------------------------------------------------------------*/
/* This is a list of the value indices used by the Helios system             */
/*---------------------------------------------------------------------------*/

/* TO BE DEFINED */

/*---------------------------------------------------------------------------*/
/*-- Display ----------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* DisplayInfo                                                               */
/* -----------                                                               */
/* This call is used to interrogate the screen position and shape.           */
/*                                                                           */
/* in:  screen = pointer to word to contain the screen start address         */
/*	stride = pointer to word to contain the raster stride (in bytes)     */
/*	X      = pointer to word to contain the displayed raster width       */
/*               (in pixels)                                                 */
/*	Y      = pointer to word to contain the number of rasters            */
/* out: returns 0 if display present and valid, otherwise -1                 */
/*---------------------------------------------------------------------------*/
extern int DisplayInfo(word *screen,word *stride,int *X,int *Y) ;

/*---------------------------------------------------------------------------*/
/*-- Memory Map support -----------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* HWRegisters                                                               */
/* -----------                                                               */
/* Interrogate and update hardware specific registers. This call is really   */
/* only provided for device drivers. It provides atomic update plus the      */
/* ability to read the state of hardware write-only registers.               */
/* See the file <abcARM/asm/SWI.s> for more information.                     */
/*                                                                           */
/* in:  index = register number to interrogate/update                        */
/*      bic   = bitmask of bits to clear                                     */
/*      orr   = bitmask of bits to set                                       */
/*      old   = pointer to word to be written with the original value        */
/*      new   = pointer to word to be written with the updated value         */
/* out: returns 0 if operation succeeded, -1 if FAILED to write.             */
/*---------------------------------------------------------------------------*/
extern int HWRegisters(int index,word bic,word orr,word *old,word *new) ;

/*---------------------------------------------------------------------------*/
/* ROHWRegisters                                                             */
/* -------------                                                             */
/* Provides the ability to interrogate read only registers. See the file     */
/* <abcARM/asm/SWI.s> for more information.                                  */
/*                                                                           */
/* in:  index = register number to interrogate                               */
/*	value = pointer to word to be written with the value read            */
/* out: returns 0 if operation succeeded, -1 if FAILED to write.             */
/*---------------------------------------------------------------------------*/
extern int ROHWRegisters(int index,word *value) ;

/*---------------------------------------------------------------------------*/
/*-- SWI calling ------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* execSWI                                                                   */
/* -------                                                                   */
/* Perform a generic SWI call from C. The header file <abcARM/SWIexec.h>     */
/* contains the actual SWI manifests (names<->numbers). The assembler header */
/* file <abcARM/asm/SWI.s> contains a full description of all the SWI        */
/* interfaces.                                                               */
/*                                                                           */
/* in:  SWInumber = SWI number                                               */
/*	inregs    = vector of words (r0..r10) to be passed to the SWI        */
/*	outregs   = vector where r0..r10 are returned from the SWI           */
/* out:	returns 0 if SWI returned V clear, -1 if SWI returned V set          */
/*---------------------------------------------------------------------------*/
extern int execSWI(word SWInumber,word *inregs,word *outregs) ;

/*---------------------------------------------------------------------------*/
/*-- CRC --------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* crc_ccitt                                                                 */
/* ---------                                                                 */
/* This will calculate a 16bit CRC-CCITT style CRC value for the given data. */
/* The data is treated as a stream of bytes which the CRC is generated from. */
/*                                                                           */
/* in:  data   = address of the first byte to be included                    */
/*      length = length of the area to CRC (multiple of "step" bytes)        */
/*      step   = number of bytes between data bytes                          */
/* out: 16bit CRC value (hi 16bits are clear)                                */
/*---------------------------------------------------------------------------*/
extern word crc_ccitt(char *data,word length,word step) ;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*> EOF ARMlib/h <*/
