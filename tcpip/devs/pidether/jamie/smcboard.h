/*> smcboard.h <*/
/* Hardware descriptions of the SMC Ethernet LAN controller boards.
 *
 * $Revision: 1.3 $
 *   $Author: jsmith $
 *     $Date: 93/04/03 12:39:09 $
 *
 * Copyright (c) 1993, VLSI Technology Inc. All Rights Reserved.
 */
/*---------------------------------------------------------------------------*/

#ifndef __smcboard_h
#define __smcboard_h

/*---------------------------------------------------------------------------*/
/* smc_board_info
 * --------------
 * This is the main structure responsible for holding information about a
 * particular SMC Ethernet card.
 */

typedef enum
{
 bf_bus16 = (1 << 0), /* Set if board on 16bit bus, clear for an 8bit bus */
 bf_16bit = (1 << 1), /* Set if board is a 16bit board, clear for an 8bit board */
 bf_valid = (1 << 2), /* Board has been configured, and is available */
 bf_unused = (1 << 3) /* First free flag */
} smc_board_flags ;

typedef struct smc_board_info
{
 struct smc_board_info *next ;   /* used if multiple Ethernet cards */
 word			iobase ; /* address of I/O in host world */
 smc_board_flags	flags ;  /* board description flags */
 word			rambase ;
 word			ramsize ;
 word			rombase ;
 word			romsize ;
 byte			irqnum ; /* IRQ number used by the board */
 byte			lanaddr[6] ; /* LAN hardware address */
} smc_board_info ;

/*---------------------------------------------------------------------------*/
/* The following three manifests define the LAN address allocation for SMC
 * Ethernet boards.
 */
#define smc_LAR0 (0x00)
#define smc_LAR1 (0x00)
#define smc_LAR2 (0xC0)

/*---------------------------------------------------------------------------*/

#endif /* __smcboard_h */

/*---------------------------------------------------------------------------*/
/*> EOF smcboard.h <*/
