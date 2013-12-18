#include <sys/file.h>
#include <sys/mman.h>
#include <stdio.h>
#include <ctype.h>
#include "vc40map.h"

long	strtol();
char *mapvme();

VC40DSP	hydras[4*MAX_HYDRA];
int	hydra_no = 0;

fda32d32 = 0;	/* file descriptor for vme32d32 space */
fda24d32 = 0;	/* file descriptor for vme24d32 space */
fda16d32 = 0;	/* file descriptor for vme16d32 space */

void vc40map(vic_paddr)
u_long vic_paddr;
{
	char	*dram_vaddr;
	int	i, numdsp, dcard;
	int	idx = 4*hydra_no;
	u_long	pval, dram_dsp;
	u_long	dram_paddr, dram_size;
	u_long	*mcr, *dram_top, moffs;
	struct	vc40_vic *vic;

	/*
	 * open VME devices
	 */
	if (!fda16d32) fda16d32 = open(VME_A16D32, O_RDWR);
	if (!fda24d32) fda24d32 = open(VME_A24D32, O_RDWR);
	if (!fda32d32) fda32d32 = open(VME_A32D32, O_RDWR);

	/*
	 * Map in the VIC
	 */
	if (fda16d32 == -1) {
		perror(VME_A16D32);
		exit(1);
	}

	vic = (struct vc40_vic *) mapvme(fda16d32, vic_paddr, 
		sizeof(struct vc40_vic));

	/*
	 * get the DRAM address and size
	 */
	get_property(vic, DRAMBase, &pval);
	dram_dsp = pval;
	dram_paddr = pval << 2;	/* convert DSP space to VME */

	get_property(vic, DRAMSize, &pval);
	dram_size = 4*1024*1024*pval;
#ifdef DEBUG
	printf("DRAM Base: 0x%lx; Size: 0x%lx\n", dram_paddr, dram_size);
#endif
	/*
	 * map the DRAM
	 */
	dram_vaddr = (char *) mapvme(fda32d32, dram_paddr, dram_size);

	/*
	 * map the MCR
	 */
	get_property(vic, HostJTAGBase, &pval);
	pval += 32;	/* MCR is 32 bytes from JTAG */
#if MCR_SPACE == A24
	pval &= 0xffffff;
#endif
#ifdef DEBUG
	printf("MCR Base: 0x%lx\n", pval);
#endif
#if MCR_SPACE == A24
#ifdef DEBUG
	printf("MCR in A24 Space\n");
#endif
	mcr = (u_long *) mapvme(fda24d32, pval, 16);
#else
#ifdef DEBUG
	printf("MCR in A32 Space\n");
#endif
	mcr = (u_long *) mapvme(fda32d32, pval, 16);
#endif

	/*
	 * find # of DSPs
	 */
	get_property(vic, Daughter, &pval);
	dcard = pval;
	if (pval) {
		numdsp = 4;
	}
	else {
		numdsp = 2;
	}
	dram_top = (u_long *)dram_vaddr;
	dram_top += dram_size/4;
	for (i=0; i<numdsp; i++) {
		hydras[idx+i].vic = vic;
		hydras[idx+i].mcr = mcr;
		hydras[idx+i].dram_vaddr = dram_vaddr;
		hydras[idx+i].dram_paddr = dram_paddr;
		hydras[idx+i].dram_size = dram_size;
		hydras[idx+i].dcard = dcard;
		hydras[idx+i].hydra_cmd = (struct hydra_cmd *)(
			dram_top - (i+1)*IOCTRL_SIZE/4);
		moffs = numdsp*IOCTRL_SIZE/4 + (i+1)*IOBUF_SIZE/4;
		hydras[idx+i].xfr_data = dram_top - moffs;
		hydras[idx+i].dsp_dram_io = dram_dsp + dram_size/4 - moffs;
	}
	hydra_no++;
}

/*********************************************************************
  mapvme()
	function to map a set of registers in one virtual page
*/
char *mapvme( vmefd, paddr, maplen)
int vmefd;
long paddr, maplen;
{
	int	page_size;		/* system page size in bytes */
	off_t	page_start;		/* addr of page start	*/
	u_long	page_offset;		/* offset into mapped page	*/
	char	*vpage_start;
	char	*vaddr;

	/*
	 * Find out the page size of the system for mmap args.
	 * Compute the page start address as required by mmap().
	 * Compute the offset into the page to be mapped.
	 */
	page_size = getpagesize();
	page_start = (paddr / page_size) * page_size;
	page_offset = paddr % page_size;

	/*
	 * Map in the needed chunk of main bus space.
	 */
	vpage_start = (char *) mmap((caddr_t)0, maplen, 
			PROT_READ|PROT_WRITE, MAP_SHARED, vmefd, page_start);
	if (vpage_start == (char *)(-1)) {
		perror("mmap call");
		exit(2);
	}

	/*
	 * Calculate the virtual address of the register.
	 */
	vaddr = vpage_start + page_offset;
	return (vaddr);
}

/**************************************************************************
 * get_property()
 *
 * reads an EEPROM value from Hydra via the monitor on DSP 1
 */
int get_property(vic, prop_id, prop_val)
struct vc40_vic *vic;
int prop_id;
u_long *prop_val;
{
	int	i;
	u_long	pval = 0;
	u_long	tlong;

	vic->CTRL_ICR = GetProperty;	/* write command word */
	vic->DSP1_SET_INTR = 0;		/* issue interrupt */
	CMD_HANDSHAKE(vic->CTRL_ICR);	/* wait for cmd ack */
	vic->CTRL_ICR = prop_id;	/* write the property ID */
	CMD_HANDSHAKE(vic->CTRL_ICR);	/* wait for ID ack */

	/*
	 * read the property value, a byte at time, LSB first
	 */
	for (i=0; i<4; i++) {
		pval = pval >> 8;
		tlong = vic->DATA_ICR;	/* read byte */
		vic->CTRL_ICR = 1;	/* signal for next byte */
		pval |= (tlong << 24);	/* `or' into MSB */
		CMD_HANDSHAKE(vic->CTRL_ICR);	/* wait for next byte */
	}
	*prop_val = pval;	/* return property value */
	return (0);		/* success! */
}

