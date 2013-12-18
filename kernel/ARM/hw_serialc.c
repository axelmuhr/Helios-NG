/*
 * File:	hw_serialc.c
 * Subsystem:	Helios-ARM Executive
 * Author:	JGSmith
 * Date:	930310
 *
 * Description: C-based VL16C551 serial interface driver.
 *
 * Copyright (c) 1993, VLSI Technology Inc.
 * All Rights Reserved.
 *
 */
/*---------------------------------------------------------------------------*/

#include "../kernel.h"	 /* standard Helios Kernel routines and manifests */
#include <config.h>

#include <ARM/vy86pid.h> /* description of the VY86PID I/O world */

/*---------------------------------------------------------------------------*/

#ifndef __VY86PID
#error "This source file should only be used for VY86PID systems"
#endif

/*---------------------------------------------------------------------------*/
/* Notes:
 *
 * Internal interrupt handlers need to be in assembler as they call
 * ExternCheckIRQ and ExternContinueThread in system handler directly.
 * The run Q's are always checked after User handlers have been run, so
 * these do not need to note if they re-schedule any threads.
 *
 * Since this file provides the base Link communications for the
 * VY86PID world, it has to be an internal handler.
 */

/* Default baud rate and line protocol */
#define defbaud (hw_serial_DLR_9600)
#define defprot (hw_serial_LCR_default)
/* FIXME: Eventually a "startup" protocol should be used by both ends
 * of this serial Link to agree on a suitable transmission protocol.
 * For the moment, however, a simple fixed system is employed.
 */

/*---------------------------------------------------------------------------*/
/*-- Link management routines -----------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* InitLink
 * --------
 * Sets up input and output interrupt handlers for given link channel. If a
 * serial port for this link number is not present, then the function simply
 * returns. Called by ExecInit() for first level comms initialisation. The
 * blocking link functions (__linkTx/Rx) must be capable of functioning after
 * this point.
 *
 * channel == channel number == serial port number.
 */

void InitLink(int ln)
{

 /* We only have a single serial port */
 if (ln == 0) /* Logical device Number 0 */
  {
   /* Reset this serial port to its default state */
   hw_SERIAL->IER = 0 ; /* disable all serial interrupt sources */
   GetExecRoot()->Serial1_IER_softcopy = 0 ; /* and ensure softcopy is initialised */

   /* Program the baud rate divisor */
   hw_SERIAL->LCR = hw_serial_LCR_DLAB ; /* access the latches */
   hw_SERIAL->DLL = (defbaud & 0xFF) ; /* divisor LSB */
   hw_SERIAL->DLM = (defbaud >> 8) ; /* divisor MSB */

   hw_SERIAL->SCR = 0x00 ; /* we may use this as a scratch later */

   /* Setup the Line Control Register */
   hw_SERIAL->LCR = defprot ;

   /* FIXME : We should use the FIFOs (to minimise latency) */
   /* Disable the Rx and Tx FIFOs */
   hw_SERIAL->FCR = 0 ; /* disable FIFOs */

   /* Setup the Modem Control Register */
   hw_SERIAL->MCR = (hw_serial_MCR_DTR | hw_serial_MCR_RTS | hw_serial_MCR_OUT2) ;

   /* Enable INTC serial IRQ interrupts - serial interrupts will now be */
   /* accepted by the ARM - still needs serial IER register to be set to */
   /* enable R/W interrupts from the serial chip. */
   hw_INTC->IRQM = GetRoot()->IRQM_softcopy |= hw_intc_enableirq_serial1;
  }

 return ;
}

/*---------------------------------------------------------------------------*/
/* InitLink2
 * ---------
 * This function is called after the link hardware initialisation function
 * InitLink(), during the kernel link guardian initialisation.
 *
 * It is used to set the Tx/Rx channel numbers in link structure that are
 * passed to _/__LinkTx/Rx functions to identify the comms hardware for that
 * particular operation.
 *
 * If one of the channels is found to have no link comms hardware attached,
 * then the link is set to an unconnected state.
 *
 */

void InitLink2(LinkInfo *l)
{
 word	baudrate = GetConfig()->Speed;

 /* Set the serial link comms baud rate as requested in the config vector. */
 if (baudrate == 19200)
  baudrate = hw_serial_DLR_19200;
 elif (baudrate == 38400)
  baudrate = hw_serial_DLR_38400;
 else
  baudrate = hw_serial_DLR_9600;	/* default to 9600 */

 hw_SERIAL->LCR = hw_serial_LCR_DLAB;	/* access the latches */
 hw_SERIAL->DLL = (baudrate & 0xFF);	/* divisor LSB */
 hw_SERIAL->DLM = (baudrate >> 8);	/* divisor MSB */

 /* Reset the Line Control Register */
 hw_SERIAL->LCR = defprot ;

 /* We only have a single serial port (logical ID 0) */
 if (l->Id == 0)
  {
   /* We do not need to worry about the Mode and State fields */
   l->RxChan = (Channel)(hw_SERIAL) ;
   l->TxChan = (Channel)(hw_SERIAL) ;
  }
 else
  {
   /* Set Link state to dis-connected */
   l->Mode = Link_Mode_Null ;
   l->State = Link_State_Null ;
   l->RxChan = l->TxChan = 0 ;
  }

 return ;
}

/*---------------------------------------------------------------------------*/
/* ResetLinkHardware
 * -----------------
 * This is called if another processor requests this one to terminate.
 * It is only useful on multiprocessor systems.
 *
 * Reset all on-chip links, canceling any current transfers
 * AbortLinkTx/Rx has already been called to cancel any outstanding transfers.
 *
 */

void ResetLinkHardware(void)
{
 /* We only have the single serial port (logical ID 0) */
 /* The "InitLink" routine disables the serial interrupt sources,
  * aswell as returning the device to the default protocol state.
  */
 InitLink(0) ;

 return ;
}

/*---------------------------------------------------------------------------*/
/*-- Link communication routines --------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* _LinkTx
 * -------
 * Simple blocking link reception function. This is used to send a bogus Info
 * message when debugging early versions of the kernel and can be used for
 * initial system debugging in concert with KDebug() and -D KERNELDEBUG2.
 *
 * size : number of bytes to send
 * link : ptr to the link hardware
 * buf  : ptr to buffer holding data
 */

void _LinkTx(word size,Channel link,void *buf)
{
 char      *buff = buf ; /* character based buffer */
 hw_serial *linka = (hw_serial *)link ; /* serial port HW */

 /* FIXME: There are no checks here to ensure that an interrupt driven
  * transfer is not in progress or that an error has occurred.
  */

 while (size)
  {
   /* Check for the Tx Holding Register being Empty */
   if (linka->LSR & hw_serial_LSR_THRE)
    {
     linka->THR = *buff++ ; /* write the byte */
     size-- ; /* and decrement the count */
    }
  }

 return ;
}

/*---------------------------------------------------------------------------*/
/* _LinkRx
 * -------
 * Simple blocking link reception function. This is used to get config vector
 * in kernel startup.
 *
 * size : number of bytes to read
 * link : ptr to the link hardware
 * buf  : ptr to buffer to hold the data
 */

void _LinkRx(word size,Channel link,void *buf)
{
 char      *buff = buf ; /* character based buffer */
 hw_serial *linka = (hw_serial *)link ; /* serial port HW */

 /* FIXME: There are no checks here to ensure that an interrupt driven
  * transfer is not in progress or that an error has occurred.
  */

 while (size)
  {
   /* Check for Rx Data Ready */
   if (linka->LSR & hw_serial_LSR_RDR)
    {
     *buff++ = linka->RBR ; /* read the byte */
     size-- ; /* and decrement the count */
    }
  }

 return ;
}

/*---------------------------------------------------------------------------*/
/* LinkTx
 * ------
 * Sets up and starts a non-blocking transfer to a communications link.
 *
 * First try to throw some bytes down the link by polling, if this will
 * take too long, then setup an interrupt driven transfer and suspend
 * until it completes.
 *
 * @@@ This could be optimised by allowing a number of polls before deciding
 * to setup the interrupt transfer.
 *
 */

void LinkTx(word size,LinkInfo *link,void *buf)
{
 char *buff = buf ; /* character based buffer pointer */

 /* If link is being used for add-on comms, use external fn. */
 if (link->TxFunction)
  _ELinkTx(size,link,buff) ; /* External Link Tx */
 else
  {
   hw_serial *linka = (hw_serial *)link->TxChan ;

   /* Poll across the data if the Tx register is empty */
   while (size && (linka->LSR & hw_serial_LSR_THRE))
    {
     linka->THR = *buff++ ; /* write the byte */
     size-- ; /* and decrement the count */
    }

   if (size != 0)
    {
     ExecRoot *eroot = GetExecRoot() ;
     LinkReq  *intr_info = &(eroot->LinkOutStat[link->Id]) ;


     /* Set info to be passed to interrupt handler */
     intr_info->Count = size ;
     intr_info->Buf = buff ;

     /* Guard against link ready interrupts happening before we are
      * successfully Suspend()ed.
      */
     DisableIRQ() ;

     /* Enable Link output ready interrupts */
     linka->IER = (eroot->Serial1_IER_softcopy |= hw_serial_IER_ETBEI) ;

     /* Interrupt handler will restart our thread when the transfer is
      * complete. AbortLinkTx() may also cause us to return.
      */
     Suspend(&link->TxThread,THREAD_LINKTX) ;

     /* We will return with interrupts enabled again */
    }
  }

 return ;
}

/*---------------------------------------------------------------------------*/
/* LinkRx
 * ------
 * Sets up and starts a non-blocking transfer from a communications link.
 *
 * First try to grab some bytes from the link by polling, if this will
 * take too long, then setup and interrupt driven transfer and suspend
 * until it completes.
 *
 * @@@ This could be optimised by allowing a number of polls before deciding
 * to setup the interrupt transfer.
 *
 */

void LinkRx(word size,LinkInfo *link,void *buf)
{
 char *buff = buf ; /* character based buffer */

 /* If link is being used for add-on comms, use external fn. */
 if (link->RxFunction)
  _ELinkRx(size,link,buff) ; /* External LinkRx */
 else
  {
   hw_serial *linka = (hw_serial *)link->RxChan ;

   /* Poll across the data if the Rx buffer is empty */
   while (size && (linka->LSR & hw_serial_LSR_RDR))
    {
     *buff++ = linka->RBR ; /* read the byte */
     size-- ; /* and decrement the count */
    }

   if (size != 0)
    {
     ExecRoot *eroot = GetExecRoot() ;
     LinkReq  *intr_info = &(eroot->LinkInStat[link->Id]) ;

     /* Set info to be passed to interrupt handler */
     intr_info->Count = size ;
     intr_info->Buf = buff ;

     /* Guard against link ready interrupts happening before we are
      * successfully Suspend()ed.
      */
     DisableIRQ() ;

     /* Enable serial input ready interrupts */
     linka->IER = (eroot->Serial1_IER_softcopy |= hw_serial_IER_ERBFI) ;

     /* Interrupt handler will restart our thread when the transfer is
      * complete. AbortLinkRx() may also cause us to return.
      */
     Suspend(&link->RxThread,THREAD_LINKRX) ;

     /* We will return with interrupts enabled again */
    }
  }

 return ;
}

/*---------------------------------------------------------------------------*/
/* _AbortLinkTx/Rx
 * ---------------
 * These functions are usually called by the port TimeoutHandler() to abort a
 * tranfer that has not completed within its allotted timeout. The transfer
 * should be aborted immediately, and the link left in a state such that
 * another LinkTx/Rx operation will work normally (if the other side is kosha).
 *
 * A higher level AbortLinkTx/Rx() takes care of NULLing the link->Tx/RxThread
 * pointer to note that the link is no longer in use.
 */

void _AbortLinkTx(LinkInfo *link)
{
 /* Stop any further link output interrupts occuring. If they are not
  * occuring anyway, then no harm has been done.
  */
 ((hw_serial *)(link->TxChan))->IER = (GetExecRoot()->Serial1_IER_softcopy &= ~hw_serial_IER_ETBEI) ;
 return ;
}

/*---------------------------------------------------------------------------*/

void _AbortLinkRx(LinkInfo *link)
{
 /* Stop any further link input interrupts occuring. If they are not
  * occuring anyway, then no harm has been done.
  */
 ((hw_serial *)(link->RxChan))->IER = (GetExecRoot()->Serial1_IER_softcopy &= ~hw_serial_IER_ERBFI) ;
 return ;
}

/*---------------------------------------------------------------------------*/
/*> EOF hw_serialc.c <*/
