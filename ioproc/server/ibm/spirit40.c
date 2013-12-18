/*------------------------------------------------------------------------
--                                                                      --
--                   H E L I O S   I / O   S E R V E R                  --
--                   ---------------------------------                  --
--                                                                      --
--             Copyright (C) 1993, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      spirit40.c                                                      --
--                                                                      --
--	Driver code for Sonitech Spirit40 C40 board.			--
--                                                                      --
--  Author:  NHG 19/1/93                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: spirit40.c,v 1.4 1993/08/12 14:24:55 bart Exp $ */
/* Copyright (C) 1993, Perihelion Software Ltd.    			*/

/*{{{  Includes */
#include <io.h>
#include <memory.h>
#include <malloc.h>

#include "helios.h"

/*}}}*/
/*{{{  Configuration */

#define NPROC		2

#define PC500ms		650

#define DEBUG		1

#define POLL_LOOPS	0

#if 0
#define	TxChanBase	(CBPtr)0x8003e000	/* C40 word address	*/
#define	RxChanBase	(CBPtr)0x8003f000	/* C40 word address	*/
#define ChanSize	 (word)0x00001000	/* in WORDS 		*/
/*#define ChanSize	 (word)0x00000800	/ * in WORDS 		*/
#endif

#define USE_ABORT	0

#define USE_INTERRUPT	0

#define	USE_NMI		0


#define TInit()

#define T(x)
/*}}}*/
/*{{{  Spirit40 stuff */

extern void asm_dl_data(unsigned address, unsigned count, long *array);
extern void asm_ul_data(unsigned address, unsigned count, long *array);
void dsp_hold();
void dsp_reset();
long dsp_ul_long(long address);
void dsp_dl_long(long address, long val);
void dsp_ul_long_array(long address, int count, long *array);
void dsp_dl_long_array(long address, int count, long *array);

/* Control Register bit details ****************************/

#define S40_HOLD_BIT  0x10

#define ResetInactive    0x001     /* 0000 0000 0001b */
#define ResetLoc0Vector  0x002     /* 0000 0000 0010b */
#define ResetLoc1Vector  0x004     /* 0000 0000 0100b */
#define RomEnable        0x008     /* 0000 0000 1000b */
#define GlobalBusEnable  0x010     /* 0000 0001 0000b */
#define NMIInactive      0x020     /* 0000 0010 0000b */
#define Bit32Access      0x040     /* 0000 0100 0000b */
#define Bit16upper       0x080     /* 0000 1000 0000b */
#define IntIIOF1         0X200     /* 0010 0000 0000b */
#define IntIIOF2         0x400     /* 0100 0000 0000b */
#define IntIIOF3         0x800     /* 1000 0000 0000b */
#define IOFlags          0xe00     /* 1110 0000 0000b */

#define IOFGlobalSource	 (IntIIOF1|IntIIOF2)

#define IOFCommPort	 (IntIIOF1|IntIIOF2|IntIIOF3)

#define ControlPort(n)		(base[n]+6)
#define HighAddressPort(n)	(base[n]+4)
#define LowAddressPort(n)	(base[n]+2)
#define DataPort(n)		(base[n]+0)

#define InterruptBit	IntIIOF1

struct
{
	long	MemWidth;
	long	GlobalCR;
	long	LocalCR;
	long	BootSize;
	long	Address;
} Preamble =
{
	32,
	0x1e39fff0,
	0x1e39fff0,
	0,
	0x002ffc00
};

struct
{
	long	Zero;
	long	IVTP;
	long	TVTP;
	long	IACK;
} Postamble =
{
	0,
	0,
	0,
	0x002ff800 /* 0x80001000 */
};
/*}}}*/
/*{{{  Support types */

typedef word Atomic;

typedef long	MPtr;

#define MInc_(m,o)		((m)+(o))
#define MtoC_(m)		((void *)(m))

#define MPSize			1


typedef long	SMPtr;

#define SMWord_(m,o) 		dsp_ul_long((long)((m)+((o)/4)))
#define SetSMWord_(m,o,v) 	dsp_dl_long((long)((m)+((o)/4)),(long)(v))
#define SMData_(d,m,o,s) 	dsp_ul_long_array((long)((m)+((o)/4)),(int)((s)/4),(long *)(d))
#define SetSMData_(m,o,d,s) 	dsp_dl_long_array((long)((m)+((o)/4)),(int)((s)/4),(long *)(d))
#define SMInc_(m,o)		((m)+((o)/4))

#define SMPSize			4



typedef long	SaveState;




/* #define ServerDebug (void) */

















/*}}}*/

#include "sml.h"

/*{{{  Variables */

/* IO port addresses */
int control_reg_value[NPROC];
int base[NPROC];
int nproc;

extern int link_base;
int link_base2;

int rootproc = 0;

LinkInfo Link;

extern int  (*rdrdy_fn)();

int EnableInterrupt = FALSE;
/*}}}*/
/*{{{  Init & Reset */
extern int spirit40_rdrdy(void);

extern void spirit40_init_link(void)
{
	/*ServerDebug("spirit40_init_link: link_base %x",link_base);*/

	base[0] = link_base;
	base[1] = link_base+8;
	nproc = 2;
	
#if NPROC > 2	
	link_base2 = get_int_config("link_base2");

	if( link_base2 != Invalid_config )
	{
		base[2] = link_base2;
		base[3] = link_base2+8;
		nproc = 4;
	}
#endif

	if( (rootproc = get_int_config("c40_root_proc")) == Invalid_config )
		rootproc = 0;
	/*ServerDebug("rootproc = %d",rootproc);*/

}

extern void spirit40_reset(void)
{
	unsigned long addr = 0x00000000;	/* base addr for bootstrap */
	int i;
	long cr;
	
	/*ServerDebug("spirit40_reset");*/

	EnableInterrupt = FALSE;
	rdrdy_fn = func(spirit40_rdrdy);
	
	for( i = 0; i < nproc ; i++ )
	{
		if( i == rootproc )
		{
			/* Set up root processor to boot from global RAM */
			control_reg_value[rootproc] = IOFGlobalSource |
						RomEnable | GlobalBusEnable |
						NMIInactive | Bit32Access;
			/*ServerDebug("CR(rootproc): %x port %x",control_reg_value[i],ControlPort(i));*/
			_outpw(ControlPort(rootproc), control_reg_value[i]);
		}
		else
		{
			/* Setup all other processors to boot from link	*/
			control_reg_value[i] = IOFCommPort | RomEnable |
					GlobalBusEnable | NMIInactive |
					Bit32Access;
			/*ServerDebug("CR(%d): %x port %x",i,control_reg_value[i],ControlPort(i));*/
			_outpw(ControlPort(i), control_reg_value[i]);
		}
	}

	/* now set up the links						*/


	{
		SMPtr srambase;
		word sramsize;
		SMPtr gbase;
		word gsize;

		if( get_config("c40_sml_g1" ) )
		{
			gbase = get_int_config("c40_idrom_gbase1");
			gsize = get_int_config("c40_idrom_gsize1");
		}
		else
		{
			gbase = get_int_config("c40_idrom_gbase0");
			gsize = get_int_config("c40_idrom_gsize0");
		}

		if( gbase == Invalid_config || gsize == Invalid_config )
		{
			ServerDebug("Invalid strobe for shared memory link");
			longjmp( exit_jmpbuf, 1);
		}

		sramsize = get_int_config("c40_sml_size");
		switch( sramsize )
		{
		default:
			ServerDebug("Invalid size for shared RAM, 8k assumed");
		case Invalid_config:
			sramsize = 8;
		case 8:
		case 16:
		case 32:
		case 64:
			sramsize *= 512;	/* set size to half of value */
		}
		srambase = gbase + gsize - (sramsize/2);
		/*ServerDebug("sram %lx %lx %lx",srambase,SMInc_(srambase,sramsize),sramsize);*/
		InitSMLChan(&Link.Channel[0], srambase, sramsize);
		Link.TxChan = &Link.Channel[0];
		InitSMLChan(&Link.Channel[1], SMInc_(srambase,sramsize), sramsize);
		Link.RxChan = &Link.Channel[1];
	}

	dsp_hold();

	/* Set address */
	_outpw(LowAddressPort(rootproc), (unsigned)addr);
	_outpw(HighAddressPort(rootproc), (unsigned)(addr>>16));

	if ((cr = get_int_config("c40_idrom_gbcr")) != Invalid_config)
		Preamble.GlobalCR = (word)cr;
	if ((cr = get_int_config("c40_idrom_lbcr")) != Invalid_config)
		Preamble.LocalCR = (word)cr;

	Preamble.BootSize = bootsize/4;
	
	/* download preamble						*/
	asm_dl_data(DataPort(rootproc), sizeof(Preamble)/2, (long *)&Preamble);
	
	/* Download the bootstrap 					*/
	asm_dl_data(DataPort(rootproc), bootsize/2, (long *)bootstrap);

	/* Download the postamble					*/
	asm_dl_data(DataPort(rootproc), sizeof(Postamble)/2, (long *)&Postamble);
	
	dsp_reset();
}
/*}}}*/
/*{{{  Interface functions */
extern int spirit40_byte_from_link(unsigned char *where)
{
	LinkTransferInfo info;

	/*ServerDebug("spirit40_byte_from_link"); */

#if TESTER
	return 0;
#endif

	info.link = &Link;
	info.size = 1;
	info.buf  = (char *)where;
	Link.RxChan->Timeout = 10;

	SMLRx(&info);

	if( Link.RxChan->Reason == SML_Aborted )
		return 1;
	else return 0;
}

extern int spirit40_byte_to_link(int data)
{
	char c = data;
	LinkTransferInfo info;

	/*ServerDebug("spirit40_byte_to_link: data %x",data); */

#if TESTER
	return 0;
#endif
	info.link = &Link;
	info.size = 1;
	info.buf  = &c;
	Link.TxChan->Timeout = 10;

	SMLTx(&info);

	if( Link.TxChan->Reason == SML_Aborted )
		return 1;
	else return 0;
}

extern int spirit40_fetch_block(unsigned int count, byte *data, int timeout)
{

	LinkTransferInfo info;

	/*ServerDebug("spirit40_fetch_block(%x,%lx,%x)",count,data,timeout);*/

	info.link = &Link;
	info.size = count;
	info.buf  = data;
	Link.RxChan->Timeout = timeout;

#if TESTER
	return 0;
#endif

	SMLRx(&info);

#if 0
	ServerDebug("Data: %02x %02x %02x %02x : %02x %02x %02x %02x",
				data[0],data[1],data[2],data[3],
				data[4],data[5],data[6],data[7]);
#endif
	if( Link.RxChan->Reason == SML_Aborted )
		return count;
	else return 0;

}

extern int spirit40_send_block(unsigned int count, byte *data, int timeout)
{
	LinkTransferInfo info;
#if 0
	ServerDebug("spirit40_send_block(%d,%lx,%x)",count,data,timeout);
	ServerDebug("Data: %02x %02x %02x %02x : %02x %02x %02x %02x",
				data[0],data[1],data[2],data[3],
				data[4],data[5],data[6],data[7]);
#endif
#if TESTER 
	return 0;
#endif

	info.link = &Link;
	info.size = count;
	info.buf  = data;
	Link.TxChan->Timeout = timeout;

	SMLTx(&info);

	if( Link.TxChan->Reason == SML_Aborted )
		return count;
	else return 0;
}

extern int spirit40_rdrdy1(void)
{
	return SMLRxRdy(&Link);
}

extern int spirit40_rdrdy(void)
{
	/*ServerDebug("spirit40_rdrdy");*/

	rdrdy_fn = func(spirit40_rdrdy1);

	EnableInterrupt = TRUE;
	
	return SMLRxRdy(&Link);
}

extern int spirit40_wrrdy(void)
{
	/*ServerDebug("spirit40_wrrdy"); */

	return SMLTxRdy(&Link);	
}

/*}}}*/
/*{{{  DSP functions */
/* Reset all C40s and disable global bus on processor 0			*/
void dsp_hold()
{
	int i;
	
	for( i = 0; i < nproc; i++ )
	{
		control_reg_value[i] &= ~ResetInactive;
		/*ServerDebug("CR[%d] value from dsp_hold is  %x",i,control_reg_value[i]);*/
		_outpw(ControlPort(i), control_reg_value[i]);		
	}
}


void dsp_reset()
{
	int i;

	for( i = 0; i < nproc; i++ )
	{
		int j;
		control_reg_value[i] |= ResetInactive | GlobalBusEnable;
		/*ServerDebug("CR[%d] value from dsp_reset is  %x",i,control_reg_value[i]);*/

		_outpw(ControlPort(i), control_reg_value[i]);
	}
}

void dsp_int(void)
{
	word cr = control_reg_value[rootproc];
	int i;
#if USE_NMI
	cr &= ~NMIInactive;
	_outpw(ControlPort(rootproc), cr);

	for( i = 100; i--; );
	
	cr |= NMIInactive;
	_outpw(ControlPort(rootproc), cr);
#else	
	/* The C40 should set up its IIF register to make the selected	*/
	/* interrupt edge triggered. Here we cause a falling edge on	*/
	/* the selected interrupt.					*/
	/*ServerDebug("dsp_int CR %x",cr);	*/
	cr |= InterruptBit;		/* Set interrupt bit		*/
	_outpw(ControlPort(rootproc), cr);
	for( i = 100; i--; );
	/*ServerDebug("dsp_int CR %x",cr);	*/
	cr &= ~InterruptBit;		/* Zero interrupt bit		*/
	_outpw(ControlPort(rootproc), cr);
	for( i = 100; i--; );	
	/* ServerDebug("dsp_int CR %x",cr);	*/
	/* Restore bit to original value				*/
	_outpw(ControlPort(rootproc), control_reg_value[0]);
	/*ServerDebug("dsp_int CR %x",control_reg_value[rootproc]);*/
#endif	
}

long dsp_ul_long(long address)
{
	long val = 0;

	/* Set address */
	_outpw(LowAddressPort(rootproc), (unsigned)address);
	_outpw(HighAddressPort(rootproc), (unsigned)(address >> 16));

	/* Upload data */
	_inpw(DataPort(rootproc));	/* Dummy read. Two dummy reads required */
	_inpw(DataPort(rootproc));	/* before uploading the data		*/

	/* Get Data */
	val = _inpw(DataPort(rootproc));
	val |= (_inpw(DataPort(rootproc))<<16);

	return val;
}

void dsp_dl_long(long address, long val)
{
	/* Set address */
	_outpw(LowAddressPort(rootproc), (unsigned)address);
	_outpw(HighAddressPort(rootproc), (unsigned)(address >> 16));

	/* Send Data */
	_outpw(DataPort(rootproc),val&0xFFFF);
	_outpw(DataPort(rootproc),(val>>16)&0xFFFF);	
}

void dsp_ul_long_array(long address, unsigned int count, long *array)
{
/*	ServerDebug("dsp_ul_long_array: addr %lx, count %d, array %lx",address,count,array);*/

	/* Set address */
	_outpw(LowAddressPort(rootproc), (unsigned)address);
	_outpw(HighAddressPort(rootproc), (unsigned)(address >> 16));

	/* Upload data */
	_inpw(DataPort(rootproc));	/* Dummy read. Two dummy reads required */
	_inpw(DataPort(rootproc));	/* before uploading the data		*/

	asm_ul_data(DataPort(rootproc), 2*count, array);    /* Actual data uploading */
}


void dsp_dl_long_array(long address, unsigned int count, long *array)
{
/*	ServerDebug("dsp_dl_long_array: addr %lx, count %d, array %lx",address,count,array);*/

	/* Set address */
	_outpw(LowAddressPort(rootproc), (unsigned)address);
	_outpw(HighAddressPort(rootproc), (unsigned)(address>>16));

	/* Download the data */
	asm_dl_data(DataPort(rootproc), 2*count, array);
}

/*}}}*/
/*{{{  asm functions */

#if 0

/* C versions of assembler routines for testing				*/

extern void asm_dl_data(unsigned address, unsigned count, long *array)
{
	int i;
	int *a = (int *)array;
/*	ServerDebug("asm_dl_data: port %x count %d array %lx",address,count,array);*/
#if 1
	for( i = 0; i < count; i++ ) _outpw(address, a[i]);
#endif
}

extern void asm_ul_data(unsigned address, unsigned count, long *array)
{
	int i;
	int *a = (int *)array;
/*	ServerDebug("asm_ul_data: port %x count %d array %lx",address,count,array);*/

	for( i = 0; i < count; i++ ) a[i] = _inpw(address);

}

#endif
/*}}}*/
/*{{{  Await */
/*--------------------------------------------------------
-- Await						--
--							--
-- Wait for the given Atomic variable to become non-	--
-- zero.						--
--							--
--------------------------------------------------------*/

static void Await(SMLChannel *sc, SMPtr atom)
{
	int iloop;
/* ServerDebug("Await %lx timeout %ld",atom,sc->Timeout); */

	while( sc->Timeout-- >= 0 )
	{
		iloop = PC500ms;
		while( iloop-- >= 0 ) 
			if( SMWord_(atom,0) != 0 ) goto done;
	}
done:
/* ServerDebug("Await %lx done time left %ld %d",atom,sc->Timeout,iloop); */

	if( SMWord_(atom,0) == 0 ) sc->Reason = SML_Aborted;
	else sc->Reason = SML_Wakeup;
}
/*}}}*/
/*{{{  SendInterrupt */
static void SendInterrupt(word *intinfo)
{
/*	ServerDebug("SendInterrupt"); */
	
	if( EnableInterrupt ) dsp_int();
}
/*}}}*/

#include "smlgen.c"
