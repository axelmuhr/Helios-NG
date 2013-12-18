/*
 * File:	memc.h
 * Subsystem:	Helios-ARM implementation
 * Author:	P.A.Beskeen
 * Date:	Oct '92
 *
 * Description: MEMC (VL86C110) ARM MEMory Controller manifests
 *		Includes support for DMA, RAM, ROM and MMU.
 *
 *
 * RcsId: $Id: memc.h,v 1.1 1993/08/03 17:11:45 paul Exp $
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 *     All Rights Reserved.
 * 
 */

#ifndef	__memc_h
#define	__memc_h


/* The memory map of an ARM based system as defined by a MEMC
 */
#define LOGRAM_base	0x00000000	/* Logical (mapped) RAM address	*/
#define PHYSRAM_base	0x02000000	/* Physical RAM base address	*/
#define IO_base		0x03000000	/* External I/O address space	*/
#ifndef IOC_base
# define IOC_base	0x03200000	/* I/O Controller base address	*/
#endif
#ifndef VIDC_base
# define VIDC_base	0x03400000	/* Video Controller base address */
#endif
#define	MEMC_base	0x03600000	/* Memory Controller address	*/
#define CAM_base        0x03800000	/* Contents Addressable Memory	*/

#define ROM_base	(HiROM_base)	/* Standard ROM mapping		*/
#define LoROM_base	(0x03400000)	/* Lo-mapped ROM base address	*/
#define HiROM_base	(0x03800000)	/* Hi-mapped ROM base address	*/


/* Description of MEMory Controller chip.  This beasty has some odd
 * characteristics, notably that it has no access to the system data
 * bus, only to the address bus.  Hence setting it up involves writing
 * (any old data) to a range of I/O space addresses in which some bits
 * of the address specify the MEMC register to be written, and others
 * are the actual data to be latched into the register.  For present
 * purposes we have no interest in the CAM settings (i.e. logical-to-
 * physical page mapping), only in the DAGs and Control register.
 * The unusual architecture makes the set-up code look funny so we
 * provide some macros.
 */

/* MEMC DAG (DMA Address Generator) register numbers
 */
#define M_vinit   0
#define M_vstart  1
#define M_vend    2
#define M_cinit   3
#define M_sstart  4
#define M_sendN   5
#define M_sptr    6

/* Macro to set up one of the MEMC DAG registers
 */
#define MEMC_set_DAG(dag_reg_num, phys_ad) \
 *((volatile int *)(MEMC_base + (dag_reg_num << 17) + (((phys_ad) >> 4) << 2))) = 0


#ifdef	_KERNEL
extern int update_memc ();
#define MEMC_update_CR(bits, mask) update_memc (bits, mask)
#endif

#define MEMC_CR_PAGE_SIZE_MASK     (3 << 2)
#define MEMC_CR_PAGE_SIZE_4KB      (0 << 2)
#define MEMC_CR_PAGE_SIZE_8KB      (1 << 2)
#define MEMC_CR_PAGE_SIZE_16KB     (2 << 2)
#define MEMC_CR_PAGE_SIZE_32KB     (3 << 2)

#define MEMC_CR_LOROM_SPEED_MASK   (3 << 4)
#define MEMC_CR_LOROM_SPEED_450    (0 << 4)
#define MEMC_CR_LOROM_SPEED_325    (1 << 4)
#define MEMC_CR_LOROM_SPEED_200    (2 << 4)
#define MEMC_CR_LOROM_SPEED_200N   (3 << 4)

#define MEMC_CR_HIROM_SPEED_MASK   (3 << 6)
#define MEMC_CR_HIROM_SPEED_450    (0 << 6)
#define MEMC_CR_HIROM_SPEED_325    (1 << 6)
#define MEMC_CR_HIROM_SPEED_200    (2 << 6)
#define MEMC_CR_HIROM_SPEED_200N   (3 << 6)

#define MEMC_CR_RAM_REFRESH_MASK   (3 << 8)
#define MEMC_CR_RAM_REFRESH_OFF1   (0 << 8)
#define MEMC_CR_RAM_REFRESH_VFLY   (1 << 8)
#define MEMC_CR_RAM_REFRESH_OFF2   (2 << 8)
#define MEMC_CR_RAM_REFRESH_CONT   (3 << 8)

#define MEMC_CR_RAM_REFRESH_NONE   MEMC_CR_RAM_REFRESH_OFF1


#define MEMC_CR_VIDEO_DMA_ENABLE   (1 << 10)
#define MEMC_CR_SOUND_DMA_ENABLE   (1 << 11)
#define MEMC_CR_OS_MODE_ENABLE     (1 << 12)

#ifdef  __STDC__
#define MEMC_set_CR_field(field, val) \
  MEMC_update_CR (MEMC_CR_##field##_##val, MEMC_CR_##field##_MASK)
#else
#define MEMC_set_CR_field(field, val) \
  MEMC_update_CR (MEMC_CR_/**/field/**/_/**/val, MEMC_CR_/**/field/**/_MASK)
#endif
  
#define MEMC_set_in_CR(bits)    MEMC_update_CR (bits, bits)
#define MEMC_clear_in_CR(bits)  MEMC_update_CR (0, bits)


#endif /*__memc_h*/


/* end of memc.h */
