/****************************************************************/
/*                          Ariel Corp.                         */
/*                        433 River Road                        */
/*                Highland Park, NJ 08904, U.S.A.               */
/*                     Tel:  (908) 249-2900                     */
/*                     Fax:  (908) 249-2123                     */
/*                     BBS:  (908) 249-2124                     */
/*                  E-Mail:  ariel@ariel.com                    */
/*                                                              */
/*                 Copyright (C) 1993 Ariel Corp.               */
/****************************************************************/


/* $Id: hydra.h,v 1.1 1994/06/29 13:46:19 tony Exp $ */
/* hydra.h
 *
 * Header defining macros and utilities for use in C code for the DSP
 */

#ifndef HYDRA_H
#define HYDRA_H 1

/*
 * All DSP programs must enable the global interrupt, otherwise Hydramon
 * will be ineffective
 */
#define GIE_ON()    asm(" or 02000h,st");

/*
 * To generate a VME interrupt, HydraMon is invoked via trap 7
 */
#define INTERRUPT_HOST()    asm(" trap 7")  /* for backwards compatibility */
#define VME_INTERRUPT()     asm(" trap 7")
  
/*
 * get copy of configuration from Hydramon
 */
#define GET_CONFIG()        asm(" trap 8")

/*
 * Nice addresses to have around
 * (older compiler can't handle these as enums.  Sigh...
 */
/* V-C40 Hydra */
#define MCR_ADDRESS     0xbf7fc008  /* address of board's MCR */
#define VIRSR_ADDRESS   0xbfff0020  /* address of VIC's interrupt registers */
#define VACBASE         0xbfff4000  /* base address of VAC registers */
#define VICBASE         0xbfff0000  /* base address of VIC registers */

/*
 * bit numbers within the MCR
 */
enum mcr_bits {
    MCR_2EN = 0,    /* DSP 2 ROM enable */
    MCR_2R1 = 1,    /* DSP 2 RESETLOC(0,1) */
    MCR_2R0 = 2,
    MCR_3EN = 3,    /* DSP 3 ROM enable */
    MCR_3R1 = 4,    /* DSP 3 RESETLOC(0,1) */
    MCR_3R0 = 5,
    MCR_4EN = 6,    /* DSP 4 ROM enable */
    MCR_4R1 = 7,    /* DSP 4 RESETLOC(0,1) */
    MCR_4R0 = 8,
    MCR_2I1 = 9,    /* DSP 2 IIOF 1 */
    MCR_2I2 = 10,   /* DSP 2 IIOF 2 */
    MCR_2I3 = 11,   /* DSP 2 IIOF 3 */
    MCR_3I1 = 12,   /* DSP 3 IIOF 1 */
    MCR_3I2 = 13,   /* DSP 3 IIOF 2 */
    MCR_3I3 = 14,   /* DSP 3 IIOF 3 */
    MCR_4I1 = 15,   /* DSP 4 IIOF 1 */
    MCR_4I2 = 16,   /* DSP 4 IIOF 2 */
    MCR_4I3 = 17,   /* DSP 4 IIOF 3 */
    MCR_D2N = 18,   /* DSP 2 NMI */
    MCR_D3N = 19,   /* DSP 3 NMI */
    MCR_D4N = 20,   /* DSP 4 NMI */
    MCR_LED = 21,   /* red LED */
    MCR_EED = 22,   /* EEPROM Data enable */
    MCR_EEC = 23,   /* EEPROM serial clock */
    MCR_WS0 = 24,   /* Wait state bits for ADbus */
    MCR_WS1 = 25,
    MCR_WS2 = 26,
    MCR_WS3 = 27,
    MCR_D2R = 28,   /* DSP 2 reset */
    MCR_D3R = 29,   /* DSP 3 reset */
    MCR_D4R = 30,   /* DSP 4 reset */
    MCR_D1R = 31    /* DSP 1 reset */
};

#endif /* #ifndef HYDRA_H */
