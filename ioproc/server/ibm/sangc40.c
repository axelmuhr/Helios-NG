/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1993, Perihelion Software Ltd.           --
--                          All Rights Reserved.                        --
--                                                                      --
--  sangc40.c                                                           --
--                                                                      --
--  Author:  BLV, RAP 23/3/93                                           --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id# */
/* Copyright (C) 1993, Perihelion Software Ltd. 			*/


/**
*** This module contains the support for the Sang Megalink C40 board
**/

#include "../helios.h"

static int c40base=0x200;

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <conio.h>
#include <dos.h>
#include <time.h>

/*
 * ======================================================================== *
 * rep_outsw
 * write xnwords 16-bit-words from the memory area pointed to by xpointer
 * to the AT I/O port given in xport_id
 * by rep outsw
 * ======================================================================== *
 */

void rep_outsw (int xport_id, unsigned short huge *xpointer, int xnwords)
{
    /*
     * import to whole parameter stuff into locals
     * to avoid any quirk of the compiler
     */

    int port_id=xport_id;
    unsigned short huge *pointer = xpointer;
    int nwords=xnwords;

    if (!nwords) return;

    __asm {
	pushf;              /* Save registers               */
	push CX;
	push DS;
	push ES;

	cld ;               /* select increment counting    */

	lds SI, pointer;    /* Load base address            */
	mov DX, port_id;    /* Load port_id location        */
	mov CX, nwords;     /* Load number of moves         */
	rep outsw;          /* String move                  */

	pop ES;             /* Restore registers            */
	pop DS;
	pop CX;
	popf;
    }
    return;
}


/*
 * ======================================================================== *
 * rep_insw
 * read xnwords 16-bit-words from the AT I/O port given in xport_id to
 * the memory area pointed to by xpointer
 * by rep insw
 * ======================================================================== *
 */

void rep_insw (int xport_id, unsigned short huge *xpointer, int xnwords)
{
    /*
     * import to whole parameter stuff into locals
     */
    int port_id=xport_id;
    unsigned short huge *pointer = xpointer;
    int nwords=xnwords;

    if (!nwords) return;

    __asm {
	pushf;              /* Save registers               */
	push CX;
	push DS;
	push ES;

	cld ;               /* select increment counting    */

	les DI, pointer;    /* Load base address            */
	mov DX, port_id;    /* Load port_id location        */
	mov CX, nwords;     /* Load number of moves         */
	rep insw;           /* String move                  */

	pop ES;             /* Restore registers            */
	pop DS;
	pop CX;
	popf;
    }
    return;
}

# define DELAY_VAL ((int)((CLK_TCK>1000)? (40*(CLK_TCK/1000)):((40/(1000/CLK_TCK))+1)))

static void delay (int ticks)
{
    volatile long t1,t2;
    if (clock()<0) {
        for(t1=0L; t1<32000L; t1++) t2=t1;
    } else {
        for(t1=(t2=clock()); t1==t2; t1=clock());
        for (t2=clock(); (t2-t1)<ticks; t2=clock());
    }
}

/*
 * ************************************************************************ *
 * interface port address map variables and definitions
 * flag bit map definitions
 * ************************************************************************ *
 */

/*
 * ======================================================================== *
 * ports & fifo size
 * ======================================================================== *
 */
 
/* offset for IO port                                               */
#define IO_OFFSET       0x000

/* offset for config register                                       */
#define CFG_OFFSET      0x004

/* offset for status/command register                               */
#define STATUS_OFFSET   0x006
#define COMMAND_OFFSET  0x006

/* offset for reset register                                        */
#define RESET_OFFSET    0x008

/* fifo size expressed in 16-bit-words                              */
static int FIFO_LENGTH     = 1024;

/*
 * ------------------------------------------------------------------------ *
 * variables transfered to macros
 * ------------------------------------------------------------------------ *
 */


/* data io port         */ 
#define c40io (c40base+IO_OFFSET)

/* config register      */
#define c40cfg (c40base+CFG_OFFSET)           

/* status register      */
#define c40status (c40base+STATUS_OFFSET)     

/* command register     */
#define c40command (c40base+COMMAND_OFFSET)   

/* reset register       */
#define c40reset (c40base+RESET_OFFSET)       

/*
 * ======================================================================== *
 *
 * register bit map definitions (yet another passe partout chip)
 *
 * REFER TO
 * ------------------------------------------------------------------------ *
 * data sheet for the IDT72510
 * Integrated Devices Technologies, Databook 1990/91 pp 6.19
 * also had a chat with JAS on that item on June 30, 1992 with the result
 * that most work will be done directly by hardware
 *
 * ======================================================================== *
 */

/*
 * ------------------------------------------------------------------------ *
 * command register opcodes
 * ------------------------------------------------------------------------ *
 */

#define COM_RESET       0x0000 

#define COM_SELECTCFG   0x0100 

#define COM_FORMAT      0x0700 /* set to 1 ! */

/*
 * ------------------------------------------------------------------------ *
 * command register operands
 * ------------------------------------------------------------------------ *
 */

/* reset command operands, command is used after fifo length detection      */

#define COMRES_NOP      0x00
#define COMRES_2PC      0x01
#define COMRES_2C40     0x02
#define COMRES_FIFO     (COMRES_2PC|COMRES_2C40)
#define COMRES_DMA      0x04
#define COMRES_ALL      (COMRES_DMA|COMRES_2PC|COMRES_2C40)

/* command configuration register selection                 */

#define COMCFG_2C40_NEAREMPTY   0x00
#define COMCFG_2PC_NEARFULL     0x03
#define COMCFG_FLAG_ASSIGNMENT  0x04

/* status register format                                   */

#define COMFORMAT_0             0x00
#define COMFORMAT_1             0x01


/*
 * status register format 1 codes
 * (NEAR)FULL/EMPTY4??? means the fifo that writes data to ???
 */

#define STATUSINFORMAT0         0x0000
#define STATUSINFORMAT1         0x0800

/* for write access polling by PC */

#define EMPTY4C40               0x0010
#define NEAREMPTY4C40           0x0020
#define FULL4C40                0x1000 
#define NEARFULL4C40            0x2000 

/* for read access polling by PC */

#define EMPTY4PC                0x4000
#define NEAREMPTY4PC            0x8000
#define FULL4PC                 0x0040 
#define NEARFULL4PC             0x0080 

/*
 * ************************************************************************ *
 * Library section
 * ************************************************************************ *
 */

/*
 * ======================================================================== *
 * INTERNALS
 * ------------------------------------------------------------------------ *
 * ping()
 *      Is a macro for runtime neutral control-break-checks 
 * MIN_WORDS
 *      Is the minmal number of words to be transmitted
 *      by rep in/outsw. Fewer words are transmitted by polling
 * ======================================================================== *
 */

static int keyping=10000;

#if 1
	/* BLV - polling the keyboard is a really bad idea in the	*/
	/* I/O Server.							*/
#define ping() goto_sleep(1L)
#else
#define ping() {if(!(--keyping)){keyping=_kbhit();keyping=10000;}}
#endif

#define MIN_WORDS 16
 
/*
 * ------------------------------------------------------------------------ *
 * FIFO full/empty status register macros
 * ------------------------------------------------------------------------ *
 */

#define C40_FULL() ((_inpw(c40status)&FULL4C40)!=0)
#define PC_FULL()  ((_inpw(c40status)&FULL4PC)!=0)

#define PC_NEARLY_FULL() ((_inpw(c40status)&NEARFULL4PC)!=0)

#define C40_EMPTY() ((_inpw(c40status)&EMPTY4C40)!=0)
#define PC_EMPTY() ((_inpw(c40status)&EMPTY4PC)!=0)

#define C40_NEARLY_EMPTY() ((_inpw(c40status)&NEAREMPTY4C40)!=0)

/**
*** Reset involves asserting a global C40 network reset and possibly
*** resetting the link adapter as well, if that is a separate option
*** in the hardware. In addition the reset routine may be used to
*** reset any static variables etc. used within this module.
**/
void megalinkC40_reset(void)
{
    unsigned reset_count, fifo_count, status_reg, config_reg;
    int g;
    reset_count = 0;

        /* deassert reset */
        _outpw (c40reset, 0);
        /* wait 40 ms */
        delay(DELAY_VAL);
    
        /* assert reset */
        _outpw(c40reset,1);
        /* wait 40 ms */
        delay(DELAY_VAL);
        
        /* deassert reset */
        _outpw (c40reset, 0);
        /* wait 40 ms */
        delay(DELAY_VAL);
        /* check status register to be in format 0 */
        /* since REV 004+ */
        status_reg = _inpw(c40status);

	/* install status register in format 1 */
        _outpw(c40command,COM_FORMAT|COMFORMAT_1);
        status_reg = _inpw(c40status);

        /* REV 005+: FIND FIFO LENGTH */
        /* set flag pin asignment */
        _outpw(c40command,COM_SELECTCFG|COMCFG_FLAG_ASSIGNMENT);
        config_reg = _inpw(c40cfg);

	/* disable c40 link */
        config_reg &=0xff00;
        config_reg |=0x0064;
        _outpw(c40command,COM_SELECTCFG|COMCFG_FLAG_ASSIGNMENT);
        _outpw(c40cfg,config_reg);
        /* fill fifo */
        for (fifo_count=0; !C40_FULL(); fifo_count++) {
            _outpw(c40io,1);
        }
        /* set length as detected */
        FIFO_LENGTH = fifo_count;

        /* REV 006+: do not reset c40 twice */
        reset_count++;
        _outpw(c40command,COM_RESET|COMRES_ALL);
        delay(50);
        status_reg = _inpw(c40status);
}

/**
*** Byte I/O is illegal since C40 links always transfer words.
**/
int megalinkC40_byte_to_link(int x)
{
	ServerDebug("megalinkC40_byte_to_link: internal error, this routine cannot be called.");
	longjmp(exit_jmpbuf, 1);
}

int megalinkC40_byte_from_link(char *where)
{
	ServerDebug("megalinkC40_byte_from_link: internal error, this routine cannot be called.");
	longjmp(exit_jmpbuf, 1);
}

/**
*** The fetch_block() and send_block() routines transfer the specified
*** number of bytes (not words) starting at the specified buffer. The number
*** of bytes should be a multiple of four, and the buffer is guaranteed to
*** be 32-bit aligned. These two routines should be implemented as efficiently
*** as possible.
***
*** The timeout argument can be ignored, it is only present for historical
*** reasons. All I/O operations should abort after between two and five
*** seconds so that the I/O Server does not hang up.
***
*** The return code is the number of bytes that were NOT sent. For example
*** if send_block() transfers 32 bytes out of 128 within a reasonable time
*** then it should return 96
**/
int megalinkC40_fetch_block(unsigned int count, char *buf, int timeout)
{
	int *run = (int *) buf;    /* runs through destination area    */
	int still_open = count/2;       /* number of words not yet received */
	if ((count % 4) != 0)
	{
		ServerDebug("megalinkC40_fetch_block: internal error, called with count %d", count);
		longjmp(exit_jmpbuf, 1);
	}
    if (still_open<MIN_WORDS) {
	/* handle small packets */
	for(; still_open>0; still_open--,run++) {
	    /* wait until (at least) one FIFO location is valid */
	    while (PC_EMPTY()) ping();
	    /* read word */
	    *run = _inpw(c40io);
	}
    } else {
	/* handle large packets */
	for (;
	    still_open >= FIFO_LENGTH;
	    still_open-=FIFO_LENGTH,run+=FIFO_LENGTH
	) {
	    /* wait until fifo has a full size data stream */
	    while (!PC_FULL()) ping();
	    /* read data by rep insw */
	    rep_insw(c40io, run, FIFO_LENGTH);
	}
	/* handle tail */
	if (still_open>0) {
	    /* enable 'nearly full' for some words */
	    _outpw(c40command,COM_SELECTCFG|COMCFG_2PC_NEARFULL);
	    /*
	     * check if [FIFO_LENGTH-still_open..FIFO_LENGTH] words are
	     * available
	     */
	    _outpw(c40cfg,FIFO_LENGTH-still_open);
	    /* wait until fifo has full data stream */
	    while(!PC_NEARLY_FULL()) ping();
	    /* read data by rep insw */
	    rep_insw(c40io, run, still_open);
	}
    }
	return 0;
}

int megalinkC40_send_block(unsigned int count, char *buf, int timeout)
{
	int	*int_buf;
	int	 i;
	int     *run;    /* runs through source data     */
    int still_open = count/2;        /* number of words not yet sent */
	run = (int *) buf;
	if ((count % 4) != 0)
	{
		longjmp(exit_jmpbuf, 1);
	}

    if (still_open<MIN_WORDS) {
	/* handle small packets */
	for(; still_open>0; still_open--,run++) {
	    /* wait until (at least) one FIFO location is free */
	    while (!C40_EMPTY()) ;
	    /* send word */
	    _outpw(c40io,*run);
	}
    } else {
	/* handle large packets */
	for (;
	    still_open >= FIFO_LENGTH;
	    still_open-=FIFO_LENGTH,run+=FIFO_LENGTH
	) {
	    /* wait until fifo can accept full size data stream */
	    while (!C40_EMPTY()) ping();
	    /* send data by rep outsw */
	    rep_outsw(c40io, run, FIFO_LENGTH);
	}
	/* handle tail */
	if (still_open>0) {
	    /* enable 'nearly empty' for some words */
	    _outpw(c40command,COM_SELECTCFG|COMCFG_2C40_NEAREMPTY);
	    /*
	     * check if max. [0..FIFO_LENGTH-still_open] words are used.
	     * - that means that still_open words can be pushed into
	     * the FIFO
	     */
	    _outpw(c40cfg,FIFO_LENGTH-still_open);
	    /* wait until fifo will accept full data stream */
	    while(!C40_NEARLY_EMPTY()) ping();
	    /* send data by rep outsw */
	    rep_outsw(c40io, run, still_open);
	}
    }
	return 0;
}

/**
*** rdrdy() returns 0 if there is no data waiting to be read from the link,
*** non-zero if there is some data waiting. wrrdy() returns 0 if it is not
*** currently possible to send data, non-zero if it is possible to send data.
*** wrrdy() is not particularly important and can always return non-zero,
*** provided that send_block() will timeout after a few seconds.
**/
int megalinkC40_rdrdy(void)
{
    return 1-PC_EMPTY();
}

int megalinkC40_wrrdy(void)
{
    return 1;
}

/**
*** Initialisation. This routine is called only once, when the I/O Server
*** starts up. It should perform any one-off initialisations.
**/
void megalinkC40_init_link(void)
{
    word x = get_int_config("link_base");
    if (x != Invalid_config)
     c40base = (int) x;
}

