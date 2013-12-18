/*
 * vc40drv.h
 *
 * V-C40 Driver Include file
 */
#define	DEBUG 0

#define	MAP_ALL_DRAM

#include <sys/param.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/uio.h>
#include <sys/map.h>
#include <sys/syslog.h>
#include <sys/vmmac.h>
#include <machine/psl.h>
#include <machine/param.h>
#include <sundev/mbvar.h>
#if defined(sun4)
#	include <sun4/pte.h>
#	define SYSTEM_PAGE_SIZE	(1 << 13)
#elif defined(sun3)
#	include <sun3/pte.h>
#	define SYSTEM_PAGE_SIZE	(1 << 13)
#endif
#include "host.h"


#define	VC40_DRV_NAME	"vc40dsp rev 0.0"
#define	VC40_DEV_NAME	"vc40dsp"

/*
 * maximum number of V-C40 Hydra's the driver will support.  There is not much
 * point in making this number smaller, as only this many _pointers_ to unit
 * structures will be allocated.  Actual unit structures will only be allocated
 * for V-C40's that have been successfully attached.
 */
#define	MAX_NVC40	33	/* must be a multiple of four plus 1! */
#define	IOBUF_SIZE	0x1000	/* DRAM I/O buffer size (in bytes) */
#define	IOCTRL_SIZE	0x40	/* Size of I/O control buffer (in bytes) */

#define	CTRL_ICR	icr0	/* VIC ICR to use for control words */
#define	DATA_ICR	icr1	/* VIC ICR to use for data transfer */
#define	DSP1_SET_INTR	set_icms0	/* Where to assert DSP1's interrupt */
#define	DSP1_CLR_INTR	clr_icms0	/* Where to clear DSP1's interrupt */
#define	UNIT_SIZE	sizeof(struct vc40_units_struct)
#define	VC40_DETACHED	(-1)

#define	JTAG_NPAGES	1	/* number of physical pages needed to map in the
				   JTAG interface and MCR */

#define	DSP1	0
#define	DSP2	1
#define	DSP3	2
#define	DSP4	3

/*
 * bits in the MCR
 */
#define	MCR_L3		0x80000000
#define	MCR_D4R		0x40000000
#define	MCR_D3R		0x20000000
#define	MCR_D2R		0x10000000

#define	MCR_WS3		0x08000000
#define	MCR_WS2		0x04000000
#define	MCR_WS1		0x02000000
#define	MCR_WS0		0x01000000

#define	MCR_EEC		0x00800000
#define	MCR_EED		0x00400000
#define	MCR_L2		0x00200000
#define	MCR_D4N		0x00100000

#define	MCR_D3N		0x00080000
#define	MCR_D2N		0x00040000
#define	MCR_4I3		0x00020000
#define	MCR_4I2		0x00010000

#define	MCR_4I1		0x00008000
#define	MCR_3I3		0x00004000
#define	MCR_3I2		0x00002000
#define	MCR_3I1		0x00001000

#define	MCR_2I3		0x00000800
#define	MCR_2I2		0x00000400
#define	MCR_2I1		0x00000200
#define	MCR_4R0		0x00000100

#define	MCR_4R1		0x00000080
#define	MCR_4EN		0x00000040
#define	MCR_3R0		0x00000020
#define	MCR_3R1		0x00000010

#define	MCR_3EN		0x00000008
#define	MCR_2R0		0x00000004
#define	MCR_2R1		0x00000002
#define	MCR_2EN		0x00000001

#define	vc40_select	nulldev
#ifndef MAP_ALL_DRAM
#	define	vc40_mmap	nodev
#endif

int get_property();
int set_property();
u_long *vc40_mapin();
void vc40_mapout();
void vc40_minphys();
void vc40_strategy();
int vc40_read_block();
int vc40_write_block();
int vc40_cmd_interrupt();
int vc40_reset();

#define	VC40_UNIT(dev)	(minor(dev))
#define	UNIT1(unit)	(unit - ((unit - 1) & 0x3))
#define	DSP(unit)	((unit - 1) & 0x3)

#define	CMD_HANDSHAKE(val)			\
    {						\
	int iii;				\
	if (val) {				\
		for (iii=0; iii<1000000 && (val); iii++) ;	\
		if (val) {			\
			fprintf(stderr, "DSP1 command handshake failed\n"); \
			return (EBUSY);		\
		}				\
	}					\
    }						\



struct vc40_units_struct {
	int	open_inhibit;	/* prevents opening */
	int	unit_open;	/* open flag */
	int	intpri;		/* interrupt priority level of this DSP */
	int	intvec;		/* interrupt vector of this DSP */
	struct proc	*uproc;	/* user process to receive signals */
	int	usignum;	/* signal number to deliver to user process */
	int	attach_code;	/* attach code for independent DSP operation */
	struct vc40_vic	*vic;	/* pointer to V-C40's VIC */
	struct buf	buf;	/* used for I/O strategy */
	int	dcard;		/* Daughter card presence flag */
	u_long	*mcr;		/* pointer to V-C40's Main Control Register */
	u_long	*jtag;		/* pointer to V-C40's TestBus controller */
	u_long	io_addr;	/* saved I/O address during read and write */
	u_long	dram_paddr;	/* DRAM base physical address */
	u_long	dram_size;	/* size of V-C40's DRAM (bytes) */
	u_long	dsp_dram_io;	/* address of DRAM data xfr area in DSP space */
	struct hydra_cmd *hydra_cmd;	
				/* pointer to unit's data xfr ctrl area */
	u_long	*xfr_data;	/* pointer to unit's data xfr area */
	char	*dram_vaddr;	/* pointer to data xfr area (for unmap) */
	u_long	dram_map_npgs;	/* number of pages mapped for xfr area */
};

/*
 * VIC registers from VME side
 */
struct vc40_vic {
	char	null0;	/* illegal access */
	char	icr0;
	char	null1;	/* illegal access */
	char	icr1;
	char	null2;	/* illegal access */
	char	icr2;
	char	null3;	/* illegal access */
	char	icr3;
	char	null4;	/* illegal access */
	char	icr4;
	char	null5;	/* illegal access */
	char	icr5;
	char	null6;	/* illegal access */
	char	icr6;
	char	null7;	/* illegal access */
	char	icr7;
	char	clr_icgs0;	/* 0x10 */
	char	set_icgs0;
	char	clr_icgs1;
	char	set_icgs1;
	char	clr_icgs2;
	char	set_icgs2;
	char	clr_icgs3;
	char	set_icgs3;	/* 0x17 */
	char	null8[8];
	char	clr_icms0;	/* 0x20 */
	char	set_icms0;
	char	clr_icms1;
	char	set_icms1;
	char	clr_icms2;
	char	set_icms2;
	char	clr_icms3;
	char	set_icms3;
};

/*
 * HydraMon command structures
 */
struct hydra_copy {	/* CopyStuff command */
	u_long	src;	/* DSP source address (in DSP space) */
	u_long	dst;	/* DSP destination address (in DSP space */
	u_long	len;	/* number of words to copy */
	u_long	nulls[12];	/* 12 unused words */
};

struct hydra_boot {	/* reset and boot DSP command */
	u_long	dspnum;	/* DSP to be reset */
	u_long	nulls[14];	/* 14 unused words */
};

struct hydra_run {	/* branch to address command */
	u_long	addr;	/* address to branch to (in DSP space) */
	u_long	nulls[14];	/* 14 unused words */
};

struct generic_cmd {
	u_long	param1, param2, param3, param4, param5, param6, param7, param8;
	u_long	param9, param10, param11, param12, param13, param14, param15;
};

struct hydra_cmd {	/* generic Hydra command structure */
	u_long	cmd;	/* command token */
	union {		/* command parameters */
		struct hydra_copy	copy_cmd;
		struct hydra_boot	boot_cmd;
		struct hydra_run	run_cmd;
		struct generic_cmd	cmd;
	} param;
};

