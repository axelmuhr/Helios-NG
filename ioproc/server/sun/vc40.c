
#define Linklib_Module

/*{{{  Includes */
#include "../helios.h"

#include <sys/mman.h>

#include "vc40dsp.h"
/*}}}*/

/*{{{  Configuration */

#define TESTER		0

#define POLL_LOOPS	0	/* Number of times to busy wait		*/

#define	RX_IN_INTERRUPT	0	/* Do receive processing in interrupt	*/

#define	TX_IN_INTERRUPT	0	/* Do transmit processing in interrupt	*/


#define VC40_MAX_LINKS	1		/* Number of links supported	*/

#if 0
#define VC40_DEVICE	"/dev/vc40b1"	/* The device special file	*/
#endif

#define VC40_SIGNAL	SIGUSR1		/* Inter-processor interrupt	*/

void vc40_signal();
/*}}}*/

/*{{{  Support types */

typedef WORD 			Atomic;

typedef long			MPtr;

#define	MInc_(m,o)		((m)+(o))
#define MtoC_(m)		((void *)(m))
#define CtoM_(m)		((MPtr)(m))

#define MPSize			1

typedef	long			SMPtr;

#define	SMPSize			1;

#define	SMWord_(m,o)		(*((long *)((m)+(o))))
#define	SetSMWord_(m,o,v)	(*((long *)((m)+(o)))=(v))
#define SMData_(d,m,o,s)	cpswap(d,(char *)((m)+(o)),s)
#define SetSMData_(m,o,d,s)	cpswap((char *)((m)+(o)),d,s)
#define SMInc_(m,o)		((m)+(o))

typedef long			SaveState;

char VC40_Device[16];

void cpswap ();

/*}}}*/

#include "sml.h"

/*{{{  Variables */

int		VC40_Fd = -1;		/* Fd of vc40 device			*/

CBPtr		ABuf;		/* Base address of mapped shared RAM	*/

struct vic_ipcr *VMECr;		/* VME VIC control regs			*/

LinkInfo	Links[VC40_MAX_LINKS]; /* links			*/

int EnableInterrupt = FALSE;

extern int  (*rdrdy_fn) ();

int	c40_errno;	/* error number used like errno */

/*
 * Information about the shared memory used for the link required in
 * several functions.
 */

u_long srambase;
u_long sramsize;

extern char        *bootstrap;        /* see module tload.c */
extern word        bootsize;

long nsig = 0;

long nwait = 0;

int	VC40_Fail = 0;

int	VC40_Type = 0;	/* 1 => HYDRA I, 2 => HYDRA II */
/*}}}*/

/*{{{  Debugging */

#if 0

static void show_cb (cb)
CBPtr cb;
{
	int *wb = (int *)SMInc_(cb, CBOverhead);
	
	ServerDebug ("CB: %08x DR %x Ack %x Size %08x Buf %08x %08x %08x %08x",
		cb, CBWord_(cb,DataReady), CBWord_(cb,Ack),
		CBWord_(cb,Size), wb[0], wb[1], wb[2], wb[3]);
}

#else

#define show_cb(cb)

#endif

#ifdef VC40_DEBUG
print_vc40info (struct vc40info *	v)
{
	ServerDebug ("intpri		: %ld", v -> intpri);
	ServerDebug ("intvec		: 0x%lx", v -> intvec);
	ServerDebug ("dram_base	: 0x%lx", v -> dram_base);
	ServerDebug ("dram_size	: 0x%lx", v -> dram_size);
	ServerDebug ("numdsp		: %ld", v -> numdsp);
	ServerDebug ("hardrev\t	: %c", v -> hardrev);
	ServerDebug ("mserno		: 0x%lx", v -> mserno);
	ServerDebug ("firmrevs	: %s", v -> firmrevs);
	ServerDebug ("drvrevs\t	: %s", v -> drvrevs);
	ServerDebug ("board_type	: %s", ((v -> board_type == HYDRAI) ? "HYDRAI" : (v -> board_type == HYDRAII) ? "HYDRAII" : "unknown"));
}
#endif

/*}}}*/

/*{{{  init, open and free */

int vc40_settype ()
{
	struct vc40info		vc40info;

	if (VC40_Fd == -1)
	{
		VC40_Fd = c40_id2fd (c40_open(VC40_Device, O_RDWR));

		if (VC40_Fd == -1)
		{
			ServerDebug ("Error - failed to open device %s", VC40_Device);

			VC40_Fail = 1;

			return -1;
		}
	}

	if (c40_getinfo (VC40_Fd, &vc40info) == -1)
	{
		ServerDebug ("Error - failed to board info for %s",
				(VC40_Device[0] == '\0') ? "NULL" : VC40_Device);

		c40_close (VC40_Fd);  VC40_Fd = -1;

			VC40_Fail = 1;

		return -1;
	}

	c40_close (VC40_Fd);  VC40_Fd = -1;

	return ((vc40info.board_type == HYDRAI) ? 1
						: (vc40info.board_type == HYDRAII) ? 2 : -1);
}


/*
 * HYDRA I
 *
 * For Hydra I boards, the shared memory link is placed at the end
 * of the chosen global strobe.  send_config () in tload.c modifies
 * the size of the strobe in the config vector, and the kernel simply
 * goes to the end of the given strobe to locate the shared memory link.
 */
#if ANSI_prototypes
void vc40_init_hydrai_link (int	hydrai_fd)
#else
void vc40_init_hydrai_link (hydrai_fd)
int	hydrai_fd;
#endif
{
	/* u_long srambase;	- global variable also used in vc40_reset ()	 */
	/* u_long sramsize;	- global variable also used in vc40_init_link () */
	u_long gbase;
	u_long gsize;

	u_long	sml_offset;
	u_long	sml_length;

	/*
	 * Retrieve the relevant constants from the host.con file.
	 */
	if (get_config ("c40_sml_g1"))
	{
		gbase = get_int_config ("c40_idrom_gbase1");
		gsize = get_int_config ("c40_idrom_gsize1");
	}
	else
	{
		gbase = get_int_config ("c40_idrom_gbase0");
		gsize = get_int_config ("c40_idrom_gsize0");
	}

	if (gbase == Invalid_config || gsize == Invalid_config)
	{
		ServerDebug ("Invalid strobe for shared memory link");

		VC40_Fail = 1;

		return;
	}

	sramsize = get_int_config ("c40_sml_size");

	switch (sramsize)
	{
	default:
		ServerDebug ("Invalid size for shared RAM, 8k assumed");
	case Invalid_config:
		sramsize = 8;

	case 8:
	case 16:
	case 32:
	case 64:
		sramsize *= 1024;	/* convert to KBytes */
	}

	/* srambase is a global, and is used in vc40_reset () later */

	srambase = gbase + gsize - (sramsize/4);
/*
	ServerDebug ("NEW: srambase = 0x%lx", srambase);
	ServerDebug ("NEW: sramsize = 0x%lx", sramsize);
*/
	sml_offset = (srambase - gbase) * 4;	/* convert to bytes */
	sml_length = sramsize;			/* already in bytes */

/*	ServerDebug ("NEW: offset = 0x%lx, length = 0x%lx", sml_offset, sml_length); */

	ABuf = (CBPtr)(c40_map_shmem (hydrai_fd, sml_offset, sml_length));

/*	ServerDebug ("NEW: ABuf = 0x%lx", (long)ABuf); */

	if ((int)ABuf == -1)
	{
		ServerDebug ("Cannot map Hydra I device %s",VC40_Device);

		VC40_Fail = 1;

		return;
	}
}

/*
 * HYDRA II
 *
 * For Hydra II boards, the shared memory link is always placed at
 * 0x80000000.  
 */

#if ANSI_prototypes
void vc40_init_hydraii_link (int	hydraii_fd)
#else
void vc40_init_hydraii_link (hydraii_fd)
int	hydraii_fd;
#endif
{
	/* u_long srambase;	- global variable also used in vc40_reset ()	 */
	/* u_long sramsize;	- global variable also used in vc40_init_link () */

	if (get_config ("c40_sml_g1"))
	{
		ServerDebug ("Error - shared memory link must be on global strobe 0 - ignoring option c40_sml_g1");
	}

	sramsize = get_int_config ("c40_sml_size");

	switch (sramsize)
	{
	default:
	case 64:
	  	/*
		 * The maximum size of the shared memory area is 32K.  Unfortunately
		 * we use the four words of this to contain -
		 * 	the actual shared memory address,
		 *	number of DSP's, interrupt priority, interrupt vector.
		 *
		 * It may be possible to implement a shared link which is ALMOST
		 * 32K, but I've left this for now.
		 */
		ServerDebug ("Invalid size for shared RAM, 8k assumed");
	case Invalid_config:
		sramsize = 8;

	case 8:
	case 16:
	case 32:
		sramsize *= 1024;	/* convert to KBytes */	

		break;
	}

	/* set up base of the shared memory area */
	srambase = 0x80000000;

	/* sml_offset = 0; */		/* convert to bytes */
	/* sml_length = sramsize; */	/* already in bytes */

	ABuf = (CBPtr)(c40_map_shmem (hydraii_fd, 0, sramsize));

/*	ServerDebug ("vc40_init_hydraii_links () - ABuf = 0x%lx", (long)ABuf); */

	if ((int)ABuf == -1 )
	{
		ServerDebug("Cannot map Hydra II device %s", VC40_Device);

		VC40_Fail = 1;

		return;
	}
}

void vc40_init_link ()
{
	int i;
	
	struct vc40info	vc40info;

	u_long	sml_area;

	char *	hydra_device;

	if (Server_Mode eq Mode_Daemon)
	{
		ServerDebug("Hydra: the link daemon cannot support the vc40 board.");
		longjmp(exit_jmpbuf, 1);
	}

	/*
	 * Open the device ...
	 */

	hydra_device = get_config ("vc40_site");

	if (hydra_device != NULL)
	{
		sprintf (VC40_Device, "/dev/vc40%s", hydra_device);

		VC40_Fd = c40_id2fd (c40_open (VC40_Device, O_RDWR));
	}
	else
	{
		/*
		 * The library function c40_open () has code to handle
		 * looking for a vc40 device.
		 */
		VC40_Fd = c40_id2fd (c40_open (NULL, O_RDWR));
	}
	
	if (VC40_Fd == -1 )
	{
		ServerDebug ("Cannot open %s",VC40_Device);

		VC40_Fail = 1;

		return;
	}

	if (c40_getinfo (VC40_Fd, &vc40info) == -1)
	{
		ServerDebug ("failed to get board info for %s",
				(VC40_Device[0] == '\0') ? "NULL" : VC40_Device);

		VC40_Fail = 1;
	}
#ifdef VC40_DEBUG
	else
	{
		print_vc40info (&vc40info);
	}
#endif

	if (vc40info.board_type == HYDRAI)
	{
		VC40_Type = 1;

		vc40_init_hydrai_link (VC40_Fd);
	}
	else if (vc40info.board_type == HYDRAII)
	{
		VC40_Type = 2;

		vc40_init_hydraii_link (VC40_Fd);
	}
	else
	{
		ServerDebug ("Error: unknown board_type %d", vc40info.board_type);

		VC40_Fail = 1;

		return;
	}

	if (ABuf == -1)
	{
		return;
	}

	/*
	 * sml_area - the area in memory used for one link.
	 * The area is split into two channels - transmit & receive
	 */

/*	ServerDebug ("NEW: sramsize = 0x%lx", sramsize); */

/*	memset ((char *)(ABuf, 0, sramsize)); */
	{
		u_long * p = (u_long *)(ABuf);
		for (i = 0; i < sramsize / 4; i++)
		{
			p[i] = 0l;
		}
	}

	sml_area = sramsize / VC40_MAX_LINKS;

/*	ServerDebug ("NEW: sml_area = 0x%lx", sml_area); */

	for (i = 0; i < VC40_MAX_LINKS; i++)
	{
		LinkInfo *link = &Links[i];
		SMLChannel *sc = &(link -> Channel[0]);
		CBPtr cb = SMInc_(ABuf, i * sml_area);

/*		ServerDebug ("NEW: [1] cb = 0x%lx (increment 0x%lx)", (long)cb, i * sml_area); */
		show_cb (cb);

		link->Unit = VC40_Fd;

		InitSMLChan (sc, cb, sml_area / 2);
		link->TxChan = sc;
		show_cb (cb);

		/* sc++; */
		sc = &(link -> Channel[1]);
		cb = SMInc_(cb, sml_area / 2);

/*		ServerDebug ("NEW: [2] cb = 0x%lx (increment 0x%lx)", (long)cb, sml_area / 2); */

		InitSMLChan (sc, cb, sml_area / 2);
		show_cb (cb);

		{
			int	i, *w;

			w = (int *)(SMInc_(cb, CBOverhead));

			for (i = 0; i < 16; i++)
			{
				w[i] = 0xfadeddad;
			}
		}

		link->RxChan = sc;

		link_table[i].link_name[0] = '\0';
	}

	/* now setup signal handler for C40->Host interrupt	*/

	(void)signal (VC40_SIGNAL, vc40_signal);

	if (vc40info.board_type != HYDRAII)
	{
/*		ServerDebug ("Enabling interrupts in vc40_init_link ()"); */

		EnableInterrupt = TRUE;
		c40_enint (VC40_Fd, VC40_SIGNAL);
	}
}

void vc40_enint ()
{
	/*
	 * The interupt vectors on the Hydra II board can only be set up
	 * on the host side, hence we wait for the kernel to set a flag on 
	 * completion of the initialisation of the shared memory area.
	 */

	SMLChannel *	sc = Links[current_link].RxChan;
	CBPtr		cb = sc -> Cb;

/*	ServerDebug ("@vc40_enint () - cb = 0x%lx", (long)cb); */

	while (CBWord_(cb,Protocol) == 0)
	{
		;
	}

/*	ServerDebug ("vc40_enint () - enabling interupts (Protocol: 0x%lx)",
			CBWord_(cb, Protocol)); */

	if (c40_enint (VC40_Fd, VC40_SIGNAL) == -1)
	{
		ServerDebug ("vc40_enint () - failed to enable interupts");
	}

/*	ServerDebug ("vc40_enint () - resetting protocol flag"); */

	SetCBWord_(cb, Protocol, 0);

	EnableInterrupt = TRUE;

/*	ServerDebug ("vc40_enint () - THE END"); */
}

#if ANSI_prototypes
int vc40_open_link (int	tabno)
#else
int vc40_open_link (tabno)
int tabno;
#endif
{
	if (tabno > VC40_MAX_LINKS || VC40_Fail)
	{
		return 0;
	}
	
	link_table[tabno].fildes = tabno;
	link_table[tabno].flags = Link_flags_not_selectable;

	return 1;
}

#if ANSI_prototypes
void vc40_free_link (int	tabno)
#else
void vc40_free_link (tabno)
int	tabno;
#endif
{
	c40_reset (Links[current_link].Unit);

	c40_close (Links[current_link].Unit);

	link_table[tabno].fildes = -1;

/*	ServerDebug ("nwait %d nsig %d", nwait, nsig); */
}

/*}}}*/
/*{{{  reset */

char peekbuf[8*1024];
int *wpb = (int *)peekbuf;

/*
 * The IO Server needs to tell the bootstrap code the Shared Memory Link address.
 * It does this by placing it in a specific location in memory which the bootstrap
 * knows about.  For Hydra I boards this location is 0x8d000000 and for Hydra II
 * it is 0x80004000.
 * The words following this address also contain information to be passed to the
 * kernel - the number of DSP's, the interrupt priority and the interrupt vector.
 */

void vc40_reset ()
{
	u_long addr = 0x002ffc00;
	struct vc40info info;
	struct
	{
		word srambase;
		word ndsp;
		word intpri;
		word intvec;
	} hydrai_info;

	struct
	{
		word	dr;
		word	ack;
		word	sze;
	} CBInitInfo;

	u_long	communication_address;

	int res;

	int i;
	u_long cw;

	CBInitInfo.dr = 0;
	CBInitInfo.ack = 0;
	CBInitInfo.sze = 0xdeadbeef;

/*	ServerDebug("vc40_reset"); */

	res = c40_reset(Links[current_link].Unit);
	if (res !=  0)
	{
		ServerDebug ("c40_reset: %d %d", res, errno);

		longjmp_exit;

		return;
	}

	for (i = 0; i < bootsize; i+=4)
	{
		int *w = (int *)(bootstrap+i);
		*w = swap(*w);
	}
		
	res = c40_getinfo (Links[current_link].Unit,&info);
	if (res != 0)
	{
		ServerDebug ("c40_getinfo: %d %d", res, errno);

		longjmp_exit;

		return;
	}

	if (info.board_type == HYDRAI)
	{
		VC40_Type = 1;

		communication_address = 0x8d000000;

		hydrai_info.srambase = srambase;
		hydrai_info.ndsp = info.numdsp;
		hydrai_info.intpri = info.intpri;
		hydrai_info.intvec = info.intvec;
	}
	else if (info.board_type == HYDRAII)
	{
		VC40_Type = 2;
		/* communication_address = 0xc0000000; */
	}
	else
	{
		ServerDebug ("Error: unknown board type 0x%lx", info.board_type);

		longjmp_exit;

		return;
	}

/*
	ServerDebug ("vc40_reset () - communication address = 0x%lx", communication_address);

	ServerDebug ("vc40_reset () - srambase = 0x%lx", srambase);
*/
	/* clear data ready and ack flags */
	res = c40_write_long (Links[current_link].Unit,
			      srambase,
			      &CBInitInfo,
			      sizeof (CBInitInfo) / sizeof (u_long));
	if (res != 0)
	{
		ServerDebug ("failed to initialise CB in vc40-reset ()");

		longjmp_exit;

		return;
	}

	if (info.board_type == HYDRAI)
	{
/*
		ServerDebug ("vc40_reset () - hydrai_info:[0x%lx, %d, %d, 0x%x]",
					hydrai_info.srambase,
					hydrai_info.ndsp,	
					hydrai_info.intpri,
					hydrai_info.intvec);
*/
		res = c40_write_long (Links[current_link].Unit,
				      communication_address,
				      &hydrai_info,
				      sizeof(hydrai_info)/sizeof(u_long));
		if (res !=  0)
		{
			ServerDebug ("c40_write_long2: %d %d", res, c40_errno);

			longjmp_exit;
	
			return;
		}
	}

	/*
	 *   We don't need to send any information to the nucleus with a
	 * Hydra II board.
	 */

/*	ServerDebug ("vc40_reset () - writing bootstrap @ 0x%lx", addr); */
	
	res = c40_write_long (Links[current_link].Unit, addr, bootstrap, bootsize/sizeof(u_long));
	if (res != 0)
	{
		ServerDebug ("c40_write_long: %d %d", res, c40_errno);

		longjmp_exit;

		return;
	}

	res = c40_run (Links[current_link].Unit, addr); 
	if (res !=  0)
	{
		ServerDebug ("c40_run: %d %d", res, errno);

		longjmp_exit;

		return;
	}
}

/*}}}*/
/*{{{  interface routines */

int vc40_rdrdy1 ()
{
	return (SMLRxRdy (&Links[current_link]));
}

int vc40_rdrdy ()
{
	/* EnableInterrupt = TRUE; */

	rdrdy_fn = func (vc40_rdrdy1);

	return (SMLRxRdy (&Links[current_link]));
}

int vc40_wrrdy ()
{
	return (SMLTxRdy (&Links[current_link]));
}

#if ANSI_prototypes
int vc40_byte_to_link (int	data)
#else
int vc40_byte_to_link (data)
int 	data;
#endif
{
	char c = data;
	LinkTransferInfo info;

/*	ServerDebug ("@vc40_byte_to_link (0x%02x)", data); */

	info.link = &Links[current_link];
	info.size = 1;
	info.buf  = CtoM_(&c);

	SMLTx (&info);

	if (Links[current_link].TxChan->Reason == SML_Aborted)
		return 1;
	else
		return 0;
}

#if ANSI_prototypes
int vc40_byte_from_link (UBYTE *	where)
#else
int vc40_byte_from_link (where)
UBYTE *	where;
#endif
{
	LinkTransferInfo info;

/*	ServerDebug ("@vc40_byte_from_link ()"); */

	info.link = &Links[current_link];
	info.size = 1;
	info.buf  = CtoM_(where);

	SMLRx (&info);

	if (Links[current_link].RxChan->Reason == SML_Aborted)
	{
/*		ServerDebug ("vc40_byte_from_link () - read byte 0x%x", *where); */
		return 1;
	}
	else
	{
/*		ServerDebug ("vc40_byte_from_link () - failed to read byte"); */
		return 0;
	}
}

#if ANSI_prototypes
int vc40_send_block (int	count,
		     char *	data,
		     int	timeout)
#else
int vc40_send_block (count, data, timeout)
int 	count;
char 	*data;
int 	timeout;
#endif
{
	LinkTransferInfo info;

/*	ServerDebug ("@vc40_send_block (%d, 0x%08lx, %d)", count, *(long *)data, timeout); */

	info.link = &Links[current_link];
	info.size = count;
	info.buf  = CtoM_(data);

	SMLTx (&info);

	if (Links[current_link].TxChan->Reason == SML_Aborted)
		return count;
	else
		return 0;
}

#if ANSI_prototypes
int vc40_fetch_block (int	count,
		      char *	data,
		      int	timeout)
#else
int vc40_fetch_block (count, data, timeout)
int 	count;
char	*data;
int 	timeout;
#endif
{
	LinkTransferInfo info;

/*	ServerDebug ("@vc40_fetch_block (%d, data ptr, %d)", count, timeout); */

	info.link = &Links[current_link];
	info.size = count;
	info.buf  = CtoM_(data);

	SMLRx (&info);

	if (Links[current_link].RxChan->Reason == SML_Aborted)
	{
/*		ServerDebug ("vc40_fetch_block () - failed to read %d bytes", count); */
		return count;
	}
	else
	{
/*		ServerDebug ("vc40_fetch_block () - read %d bytes, starting with %x", count, data[0]); */
		return 0;
	}

}


void vc40_analyse ()
{
	vc40_reset ();
}


/*}}}*/
/*{{{  Interrupt */

int	SIMsgCount = 0;

#if ANSI_prototypes
static void SendInterrupt (word *	vector)
#else
static void SendInterrupt (vector)
word *	vector;
#endif
{
	/* ServerDebug ("SendInterrupt (%d)", (long)vector); */

	if (!EnableInterrupt)
	  {
	    /* ServerDebug ("SendInterrupt () - not enabled"); */
	    return;
	  }

	if (c40_iof2 (Links[current_link].Unit) == -1)
	  {
	    ServerDebug ("SendInterrupt () - ioctl () failed, errno = %d", errno);
	  }
/*
	else
	  {
	    ServerDebug ("SendInterrupt () - ioctl () succeeded");
	  }
*/
}

/*}}}*/
/*{{{  vc40_signal */

void vc40_signal ()
{
	nsig++;

/*	ServerDebug ("vc40_signal () - call number %d", nsig); */
}
/*}}}*/
/*{{{  C40 routines */

#include "c40sundriv.c"

/*}}}*/
/*{{{  Await */
/*--------------------------------------------------------
-- Await						--
--							--
-- Wait for the given Atomic variable to become non-	--
-- zero.						--
--							--
--------------------------------------------------------*/

#if ANSI_prototypes
static void Await (SMLChannel *	sc,
		   SMPtr	atom)
#else
static void Await (sc, atom)
SMLChannel *	sc;
SMPtr 		atom;
#endif
{
	WORD poll;

	sc->Reason = SML_Wakeup;

	nwait++;

#if 0
ServerDebug ("Await (sc: 0x%lx, atom: 0x%lx[0x%lx])", (long)sc, atom, SMWord_(atom, 0));
show_cb (sc -> Cb);
#endif

#if 1
while (SMWord_(atom, 0) == 0)
{
	;
}
#endif

#if POLL_LOOPS > 0
	poll = POLL_LOOPS;

	while (poll--)
	{
		if (SMWord_(atom, 0) != 0)
		{
			return;
		}
	}
#else
	if (SMWord_(atom, 0) != 0)
	{
/*		ServerDebug ("Await () - returning, SMWord_(atom:0x%lx, 0) = %d", (long)atom, SMWord_(atom, 0)); */

		return;
	}
#endif

	/* Wait for 5 seconds for something to happen. If it doesn't	*/
	/* abort the transfer.						*/

	poll = 20;

	while (poll--)
	{
		sleep (1);

/*		ServerDebug ("\tAwait %x[%x] (polling)", atom, SMWord_(atom, 0)); */
		show_cb (sc -> Cb);

		if (SMWord_(atom,0) != 0) 
		{
/*			ServerDebug ("Await %x[%x] done", atom, SMWord_(atom, 0)); */
			return;
		}
	}

/*	ServerDebug ("Await %x[%x] abort", atom, SMWord_(atom, 0)); */

	sc->Reason 		= SML_Aborted;
	
	return;

}
/*}}}*/
/*{{{  cpswap */
#if SUN4
#if ANSI_prototypes
void cpswap (char *	dst,
	     char *	src,
	     int	size)
#else
void cpswap (dst, src, size)
char *	dst;
char *	src;
int 	size;
#endif
{
	register char *d = dst;
	register char *s = src;
	register int   z = size;

#if 0
	if (z <= 0 )
	{
		ServerDebug ("cpswap: d %x s %x z %x", d, s, z);
	}
#endif

#if 1
	while (z >= 16)
	{
		d[0] = s[3];
		d[1] = s[2];
		d[2] = s[1];
		d[3] = s[0];

		d[4] = s[7];
		d[5] = s[6];
		d[6] = s[5];
		d[7] = s[4];

		d[8] = s[11];
		d[9] = s[10];
		d[10] = s[9];
		d[11] = s[8];

		d[12] = s[15];
		d[13] = s[14];
		d[14] = s[13];
		d[15] = s[12];

		d += 16;
		s += 16;
		z -= 16;
	}
#endif
#if 1
	while (z >= 8)
	{
		d[0] = s[3];
		d[1] = s[2];
		d[2] = s[1];
		d[3] = s[0];

		d[4] = s[7];
		d[5] = s[6];
		d[6] = s[5];
		d[7] = s[4];

		d += 8;
		s += 8;
		z -= 8;
	}
#endif
#if 1
	while (z >= 4)
	{
		d[0] = s[3];
		d[1] = s[2];
		d[2] = s[1];
		d[3] = s[0];

		d += 4;
		s += 4;
		z -= 4;
	}
#endif
	if (z <= 0)
	{
		return;
	}

	while (z--)
	{
		d[z^3] = s[z];
	}

#if 0
	if (((long)d & 3) == 0 && ((long)s & 3) == 0)
	{
		/* aligned source and dest */
	}
#endif
}

#endif
/*}}}*/

#include "smlgen.c"
