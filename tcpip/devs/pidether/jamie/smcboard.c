/*> smcboard.c <*/
/* Routines to find and interrogate the SMC Ethernet boards.
 *
 * $Revision: 1.3 $
 *   $Author: jsmith $
 *     $Date: 93/04/06 14:57:40 $
 *
 * Copyright (c) 1993, VLSI Technology Inc. All Rights Reserved.
 */
/*---------------------------------------------------------------------------*/

#include "helios.h"    /* host environment definitions */
#include "vy86pid.h"   /* description of host hardware */
#include "smc83c584.h" /* PC AT/XT interface controller */
#include "smc83c690.h" /* Ethernet LAN controller */
#include "smcboard.h"  /* SMC Ethernet boards */

/*---------------------------------------------------------------------------*/
/* smcGetBoardInfo
 * ---------------
 * This routine scans the I/O space (from the given base) looking for
 * suitable SMC Ethernet cards.
 * in:	iobase = pointer to word containing the next PC I/O slot to be scanned.
 *	info   = pointer to board description structure to be used. This is
 *		 passed into the routine since we may not have any method
 *		 of allocating memory at this level.
 * out:	returns NULL if no board found, otherwise a pointer to the board
 *	description structure. The "iobase" variable is updated to reflect
 *	the next address to be scanned, or is undefined if NULL is returned.
 *	Similarly the "info" structure is updated, or undefined if NULL is
 *	returned.
 */
smc_board_info *smcGetBoardInfo(word *iobase,smc_board_info *info)
{
 smc_board_info *rval = NULL ; /* return pointer, set to non-NULL if card found */

 info->next = info ; /* self-referencing structure for the moment */

 /* Step through the remaining I/O address slots */
 for (; (*iobase < 0x400);)
  {
   smc83c584 *ioc = (smc83c584 *)hw_es_IObase(*iobase) ;
   int        found = FALSE ; /* set to TRUE if board found */
   int	      loop ; /* general loop counter */
   vubyte    *dptr ; /* general data pointer */
   byte	      csum ; /* checksum holder */

   info->iobase = (word)ioc ; /* I/O structure address being used */
   info->flags = 0x00000000 ; /* no flags set by default */
   info->rambase = info->rombase = 0xFFFFFFFF ; /* no addresses found yet */
   info->irqnum = 0 ; /* IRQ source not yet defined */

   /* Check the LAN address, ID and checksym bytes for a valid 8bit sum */
   dptr = &(ioc->LAR0) ; /* first LAR byte */
   for (loop = 0; (loop < 8); loop++)
    {
     csum += *dptr ;
     if (loop < 6)
      info->lanaddr[loop] = *dptr ; /* take a copy of the LAN hardware address */
     dptr += sizeof(word) ;
    }
   if (csum == 0xFF) /* checksum is OK, so we may have a board */
    {
     /* Check if the first three LAN address bytes match the SMC allocation */
     /* We may want to remove this check, since we will be able to work with
      * any LAN hardware address, and it may transpire that compatible cards
      * are available with a different allocation.
      */
     if ((ioc->LAR0 == smc_LAR0) && (ioc->LAR0 == smc_LAR0) && (ioc->LAR0 == smc_LAR0))
      {
       byte bid = ioc->LAR6 ; /* board ID byte */

       /* Check that the board revision is OK */
       if (((bid & LAR6_REV_mask) >> LAR6_REV_shift) != 0)
        {
	 int interface = FALSE ; /* set to TRUE if we have interface chip */

	 ... if LAR6_BUSTYPE is TRUE then we have a MCA bus device ... which we don't support ...

	 /* Check for register aliasing. If registers 1..5 and 7 alias the
	  * LAN address bytes then we do not have an interface chip. If any
	  * of the bytes differ then we do.
	  *
	  * FIXME : The following code is nasty in that it ASSUMEs
	  * that the LAR bytes start at offset 8.
	  */
	 dptr = (vubyte *)ioc ; /* reference the registers as a vector of bytes */
	 for (loop = 1; (!interface && (loop < 6)); loop++)
	  if (dptr[hw_es_ADDR(loop)] != dptr[hw_es_ADDR(8 + loop)])
	   interface = TRUE ; /* bytes differ */
	 if ((!interface) && (dptr[hw_es_ADDR(7)] != dptr[hw_es_ADDR(8 + 7)]))
	  interface = TRUE ; /* bytes differ */

	 if (interface)
	  {
	   /* A further check for an interface chip is wether GP2 can be modified */
#define CHECKBYTE (0xA3) /* first check-byte value (second is inverted) */
	   ioc->GP2 = CHECKBYTE ; /* write first check byte */
           /* The structure is "volatile" so we know that the
            * following read will not be optimised out by the
            * compiler. The fact that smc_LAR0 (which is checked
	    * above) may well match the CHECKBYTE is not really a
	    * problem, since the second value write will then be
	    * different (or vice versa).
	    */
           loop = ioc->LAR0 ; /* modify the data bus */
	   if (ioc->GP2 == CHECKBYTE)
	    {
	     /* since we may have been unlucky and used the actual
	      * value already in GP2, we do a second write check.
	      */
	     ioc->GP2 = ~CHECKBYTE ; /* write second value */
             loop = ioc->LAR0 ; /* modify the data bus */
	     if (ioc->GP2 == ~CHECKBYTE)
	      {
	       /* Check the state of the ICR_BIT16 flag. If it is
		* always 1 then we are on a 16bit bus.
		*/
	       if (ioc->ICR & ICR_BIT16)
	        {
		 ioc->ICR = (ioc->ICR & ~ICR_BIT16) ; /* clear the bit, and store */
		 /* We know that GP2 can be written, so ensure that the data bus changes */
		 ioc->GP2 = 0xFF ;
		 if (ioc->ICR & ICR_BIT16)
		  info->flags |= bf_bus16 ; /* bit fixed, so 16bit bus */
		 else
		  info->flags &= ~bf_bus16 ; /* bit changed, so 8bit bus */
	        }
	       else
		info->flags &= ~bf_bus16 ; /* definately an 8bit bus */

	       /* We know we are using an interface chip, so we can
		* check that the jumpers reflect the state we expect.
		*/
	       if I/O found at 2A0 then IJ_INIT_mask should be 0x7 (soft)
	       if I/O found at 280 then IJ_INIT_mask should be 0x5
	       if I/O found at 300 then IJ_INIT_mask should be 0x3
	       .. we can also check that the IAR register reflects the I/O address we are using

	       ... get media type ... Ethernet, StarLAN, Twisted pair ...
		 if LAR6_MEDIA
                  ... ethernet
                 else
                  if board rev == 1 then
                   ... StarLan
                  else
                   ... twisted pair

	       ... all the above is board rev 1 information
	       ... board rev 2 also includes - soft config bit
	       ... board rev 3 also includes - eeprom and 584 chip (if not MCA device)

               ... we need at least board rev 3 ... and this should part of the interface chip check ...

	       ... check EEPROM contents ...
                  ... recall the reserved engineering EEPROM bytes ... bytes 0x50->0x57
                  ... poll until bytes available
                     ... the amount of RAM available depends on wether we are an 8/16 bit card in an 8/16bit slot

	       ... check for presence of 690 or 8390 chip ...
                     ... read TCON (page 2)
                     ... write TCON bits3-4 (unused in 690)
                     ... read TCON
                     ... check bits 3-4
                     ... if match written value then 8390, else 690
                     ... restore original TCON value

	       /* We have satisfied all of the checks */
	       found = TRUE ;

	       ... get RAM size and address ... this depends on the board type ...
	       ... get ROM size and address ... this depends on the board type ...
              }
            }
          }
        }
      }
    }

   *iobase += 0x20 ; /* increment I/O slot ready for next call */

   if (found)
    {
     rval = info ; /* we have initialised the structure */
     break ; /* out of the for loop */
    }
  } /* for */

 return(rval) ;
}

/*---------------------------------------------------------------------------*/
/* smcConfigBoard
 * --------------
 * Configure the given board for use on this host. This might mean moving the
 * I/O, RAM or ROM addresses, or simply selecting a different IRQ source bit.
 * in:  info = pointer to board description structure.
 * out: returns TRUE if board configured OK, otherwise FALSE. The info
 *	structure can be interrogated to find out which configuration entry
 *	failed.
 */
int smcConfigBoard(smc_board_info *info)
{
 ... we may need to scan the other devices linked through (info->next)
 ... to check we do not have a clash. The description structures are held
 ... in a circular list, so we can loop around until reaching our own
 ... description. The current PID world will only allow four PC cards,
 ... using IRQs 3,4,5 and 6. Unfortunately the SMC cards can only generate
 ... IRQs on 3,4 and 5, thus limiting us to three SMC cards on a PID.

 ... we need to ensure that there is no clash of I/O, RAM, ROM and IRQ

 return(FALSE) ;
}

/*---------------------------------------------------------------------------*/
/*> EOF smcboard.c <*/
