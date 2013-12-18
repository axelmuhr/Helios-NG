/*
 * File:	vidc.h
 * Subsystem:	Helios/ARM implementation
 * Author:	P.A.Beskeen
 * Date:	Oct '92
 *
 * Description: VIDC (VL86C310) ARM VIDeo Controller manifests.
 *		Includes stereo sound and cursor control.
 *
 *
 * RcsId: $Id: vidc.h,v 1.1 1993/08/03 17:11:45 paul Exp $
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 *     All Rights Reserved.
 * 
 */

#ifndef __vidc_h
#define __vidc_h

/*
 * Definitions for use of the VIDC chip in an A-series computer
 */

/* physical address of VIDC in A-series memory map */

#ifndef VIDC_base
# define VIDC_base	0x3400000
#endif

/*
 * Offsets of (write-only) registers in the VIDC, from the base address at
 * which it is mapped, are defined here using a structure for convenience.
 *
 * See the VIDC databook for detailed explanations of register use.
 */

#define V_dlcr(n) (0+(n)*4)          /* 00-3C display logical colours 0..15 */
#define V_bcr     0x40               /* 40    border colour                 */
#define V_clcr(n) (0x44+((n)-1)*4)   /* 44-4C cursor logical colours 1..3   */
                                     /* 50-5C RESERVED - DO NOT TOUCH       */
#define V_sir(n) (0x60+(((n)+1)&7)*4) /* 60-7C stereo sound positions 7,0..6 */
#define V_hcr     0x80               /* 80    horizontal cycle              */
#define V_hswr    0x84               /* 84    horizontal sync width         */
#define V_hbsr    0x88               /* 88    horizontal border start       */
#define V_hdsr    0x8C               /* 8C    horizontal display start      */
#define V_hder    0x90               /* 90    horizontal display end        */
#define V_hber    0x94               /* 94    horizontal border end         */
#define V_hcsr    0x98               /* 98    horizontal cursor start       */
#define V_hir     0x9C               /* 9C    horizontal interlace          */
#define V_vcr     0xA0               /* A0    vertical cycle                */
#define V_vswr    0xA4               /* A4    vertical sync width           */
#define V_vbsr    0xA8               /* A8    vertical border start         */
#define V_vdsr    0xAC               /* AC    vertical display start        */
#define V_vder    0xB0               /* B0    vertical display end          */
#define V_vber    0xB4               /* B4    vertical border end           */
#define V_vcsr    0xB8               /* B8    vertical cursor start         */
#define V_vcer    0xBC               /* BC    vertical cursor end           */
#define V_sfr     0xC0               /* C0    sound frequency               */
                                     /* C4-DC RESERVED - DO NOT TOUCH       */
#define V_cr      0xE0               /* E0    control register              */
                                     /* E4-FC RESERVED - DO NOT TOUCH       */

/* values for the Stereo Image position registers */

#define SIR_Left_100  1
#define SIR_Left_83   2
#define SIR_Left_67   3
#define SIR_Centre    4
#define SIR_Right_67  5
#define SIR_Right_83  6
#define SIR_Right_100 7


#define VIDC_reg_data(reg, val) ((reg) << 24) | (val)

#define VIDC_WRITE(reg_data)  \
  *(volatile unsigned int *)VIDC_base = reg_data

#define VIDC_colour_reg(reg, val) \
  VIDC_reg_data (reg, val)                        /* data at bits 0:12 */

#define VIDC_image_reg(chan, pos)  \
  VIDC_reg_data (V_sir(chan), pos)                /* data at bits 0:2 */

#define VIDC_display_reg(reg, val)  \
  VIDC_reg_data (reg, (val) << 14)                /* data at bits 14..23 */

#define VIDC_hcsr_normal_reg(val)  \
  VIDC_reg_data (V_hcsr, (val) << 13)             /* data at bits 13..23 */

#define VIDC_hcsr_hi_res_reg(val)  \
  VIDC_reg_data (V_hcsr, (val) << 11)             /* data at bits 11..23 */

#define VIDC_sound_freq_reg(val)  \
  VIDC_reg_data (V_sfr, (1 << 8) | (val))         /* data at bits 0..7 */


/* VIDC CR (Control Register) has following format:
 *
 * bit    7: composite sync bit      0 => off, 1 => on
 *
 * bit    6: interlaced sync bit     0 => off, 1 => on
 *
 * bits 4-5: DMA request timing:
 *              0 => request at end of words 0 & 4
 *              1 => request at end of words 1 & 5
 *              2 => request at end of words 2 & 6
 *              3 => request at end of words 3 & 7
 *           (required setting determined by RAM speed/video data rate etc)
 *
 * bits 2-3: Bits per Pixel:
 *              0 => 1 bit/pixel
 *              1 => 2 bits/pixel
 *              2 => 4 bits/pixel
 *              3 => 8 bits/pixel
 *
 * bits 0-1: Pixel Rate:
 *              0 => 8 MHz
 *              1 => 12 MHz
 *              2 => 16 MHz
 *              3 => 24 MHz
 */

#define VIDC_control_reg(c_sync,i_sync,dma_req,bits_per_pixel,pixel_rate) \
  VIDC_reg_data (V_cr, ((c_sync) << 7) | \
                       ((i_sync) << 6) | \
                       ((dma_req) << 4) | \
                       ((bits_per_pixel) << 2) | \
                       ((pixel_rate) << 0) )

/*
 * Definitions for the video clock control register (where fitted).
 */
#define VIDC_CLOCK	(*(volatile unsigned char *)0x03350048)
#define VCLK_24000	0			/* standard VIDC clock */
#define VCLK_25175	1			/* 25.175 MHz: VGA rate */
#define VCLK_36000	2			/* 36MHz for super VGA */

#define	VIDC_CLOCK_FREQ_DEFAULT	24000		/* in kHz */
         

#endif /*__vidc_h*/


/* end of vidc.h */
