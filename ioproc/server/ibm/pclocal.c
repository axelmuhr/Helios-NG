/*------------------------------------------------------------------------
--                                                                      --
--                   H E L I O S   I / O   S E R V E R                  --
--                   ---------------------------------                  --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      pclocal.c                                                       --
--                                                                      --
--               The localisation module for IBM PC's and compatibles   --
--                                                                      --
--  Author:  BLV 30/5/88                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: pclocal.c,v 1.30 1994/07/06 10:47:45 mgun Exp $ */
/* Copyright (C) 1988, Perihelion Software Ltd.    			*/

/*{{{  headers and statics */

#define Local_Module
#include <dos.h>
#include <memory.h>
#include <malloc.h>

#include "helios.h"

             /* The following are kept in the assembler module */
extern int floppy_errno, RS232_errno, Centronics_errno, Printer_errno;
extern int version_number;   /* MS-dos version */
#if (RS232_supported || Centronics_supported || Printer_supported)
extern void add_port_node(List *list, char *name, ComsPort *port);
#endif

/*}}}*/
/*{{{  memory management */
#if (use_own_memory_management && !MSWINDOWS)
/*------------------------------------------------------------------------
--
-- The memory management routines
--
------------------------------------------------------------------------*/
/**
*** On the PC I do not trust the microsoft C library's memory
*** allocation routines, so I provide my own. This involves a fair bit of
*** work.
***
*** To keep track of all the memory I use linked lists, surprise surprise.
*** There are two separate linked lists, small_pool for the small bits of data
*** and big_pool for the large chunks.
**/

typedef struct memory_node { Node     node;
                             word     size;	/* not including header */
} memory_node;

List small_pool, big_pool;
PRIVATE char *get_small(), *get_big();
#define memory_big    0x66000000L
#define memory_small  0x77000000L
#define memory_mask   0xFF000000L

void initialise_memory()
{ memory_node *current_node;
  int i;

  InitList(&small_pool); InitList(&big_pool);

  current_node = (memory_node *) _fmalloc(20 * 1024);
  if (current_node eq (memory_node *) NULL)
   { printf("Insufficient memory to run the I/O Server.\n");
     exit(1);
   }
  current_node->size = (20L * 1024L) - (20L + sizeof(memory_node));
  AddTail(&(current_node->node), &small_pool);

  for (i = 0; i < 5; i++)
   { current_node = (memory_node *) _fmalloc(65500);
     if (current_node eq (memory_node *) NULL)
      { if (i < 2)
         { printf("Insufficient memory to run the I/O Server.\n");
           exit(1);
         }
        else
         break;
      }
     current_node->size = 65400L - sizeof(memory_node);
	/* BLV - assume (fairly safely) that DOS allocates memory from	*/
	/* low to high.							*/
     AddTail(&(current_node->node), &big_pool);
   }

  for (;;)
   { current_node = (memory_node *) _fmalloc(20100);
     if (current_node eq (memory_node *) NULL)
         break;
     current_node->size = 20000L - sizeof(memory_node);
     AddTail(&(current_node->node), &big_pool);
   }
}

/**
*** This routine implements "malloc" by calling additional
*** routines below.
**/
char *get_mem(size)
uint size;
{
  if (size < 250)
    return(get_small((word) size));
  else
    return(get_big((word) size));
}


/**
*** free'ing memory is not as easy as adding the memory back to the appropriate
*** linked list because I want to do continuous garbage collection, i.e. 
*** whenever a bit is freed I try to merge it with adjacent free bits. This
*** involves keeping the linked lists sorted, which is no big deal as they
*** tend to be very small in practice. The first step is to work out
*** which pool the memory belongs to (assuming it is valid) and going down
*** the linked list until I find the nearest bits of free memory. Then I
*** try to merge the newly-freed bit with the previous lot, and with the
*** next lot, making sure that I do not generate invalid list nodes. Finally
*** I can put the data back into the free pool, either at the end of the list
*** or just before the next node so that the list remains sorted.
**/

void free_mem(a_ptr)
char *a_ptr;
{ memory_node *current_node, *prev_node, *next_node, *expected, *tmp_node;
  List *memory_list;

  current_node = (memory_node *) a_ptr;
  current_node = &(current_node[-1]);

  if ((current_node->size & memory_mask) eq memory_small)
    memory_list = &small_pool;
  elif ((current_node->size & memory_mask) eq memory_big)
    memory_list = &big_pool;
  else
   { /* printf("Error : free'ing memory that has not been allocated.\n");*/
     return;
   }
  current_node->size &= ~memory_mask;

  expected = (memory_node *) ((char *)a_ptr + current_node->size);

  for ( next_node = (memory_node *)memory_list->head,
        prev_node = (memory_node *)NULL;

        (next_node->node.next ne (Node *) NULL) &&
        ((unsigned long)next_node < (unsigned long)expected);

        prev_node = next_node,
        next_node = (memory_node *)next_node->node.next);

/**
*** At this stage I have :
*** prev_pointer is NULL or the previous element in the free list.
*** next_pointer is dummy or the next element in the free list.
**/

  if (prev_node ne NULL)
    if (((char *)prev_node + (word)sizeof(memory_node) + prev_node->size) eq
         ((char *)current_node) )
       { prev_node->size += (word)sizeof(memory_node) + current_node->size;
         Remove(&(prev_node->node));
         current_node = prev_node;
       }

  if (next_node == expected)
     { current_node->size += (word)sizeof(memory_node) + next_node->size;
       tmp_node = (memory_node *) next_node->node.next;
       Remove(&(next_node->node));
       next_node = tmp_node;
     }

  if (next_node->node.next eq (Node *) NULL)
    AddTail(&(current_node->node), memory_list);
  else
    PreInsert(&(next_node->node), &(current_node->node));
}

/**
*** This is used to get some memory from the small pool. The amount requested
*** is rounded to the nearest multiple of 4 so that memory is always allocated
*** in words rather than bytes. Then I go through the list of available memory
*** finding a suitable chunk. I do not try any fancy searching - the first bit
*** of free memory that is large enough will suffice. I may use this bit
*** as a lump if it is not too much bigger than what I want, or I may split
*** it up into two lumps. If the end of the list is reached then I have run out
*** so I return NULL, and all my code is careful to check for NULL (I hope).
**/
PRIVATE char *get_small(amount)
word amount;
{ memory_node *current_node, *new_node;
  word difference;

  current_node = (memory_node *) small_pool.head;

  amount = 4L * ((amount + 3L) / 4L);

  for (; current_node->node.next ne (Node *) NULL;
         current_node = (memory_node *) current_node->node.next )
   { difference = current_node->size - amount;
     if (difference < 0L) continue;    /* current node is too small */

     if (difference < 50L) /* Is it small enough not to worry about splitting */
       { current_node->size += memory_small;
         Remove(&(current_node->node));
         return( (char *) (current_node + 1));
       }

     current_node->size -= (amount + (word) sizeof(memory_node));
     new_node = (memory_node *)
         ((char *) current_node + sizeof(memory_node) + current_node->size);
     new_node->size     = memory_small + amount;
     return( (char *) (new_node + 1)) ;        
   }

  return(NULL);
}

/**
*** This code is virtually a copy of the above, the only difference being that
*** memory is allocated from the pool of big chunks instead of small ones and
*** I use a looser definition of what constitutes a close match.
**/
PRIVATE char *get_big(amount)
word amount;
{ memory_node *current_node, *new_node;
  word difference;

  current_node = (memory_node *) big_pool.head;

  amount = 4L * ((amount + 3L) / 4L);

  for (; current_node->node.next ne (Node *) NULL;
         current_node = (memory_node *) current_node->node.next )
   { difference = current_node->size - amount;
     if (difference < 0L) continue;    /* current node is too small */

     if (difference < 3000L) /* Is it small enough not to worry about splitting */
       { current_node->size += memory_big;
         Remove(&(current_node->node));
         return( (char *) (current_node + 1));
       }

     current_node->size -= (amount + (word) sizeof(memory_node));
     new_node = (memory_node *)
         ((char *) current_node + sizeof(memory_node) + current_node->size);
     new_node->size = memory_big + amount;
     return( (char *) (new_node + 1)) ;        
   }

  return(NULL);
}

/**
*** This code provides another debugging option, Memory or -x, which gives
*** the number and total size of the bits of free memory.
**/
PRIVATE void add_mem(node, a_ptr, size)
memory_node *node;
word *a_ptr, *size;
{ *a_ptr += 1L;
  *size += node->size;
}

void memory_map()
{ word small = 0L, small_total = 0L, big = 0L, big_total = 0L;
 
  WalkList(&small_pool, add_mem, &small, &small_total);
  WalkList(&big_pool, add_mem, &big, &big_total);

  ServerDebug("In the small memory pool there are %ld nodes, giving %ld bytes.",
         small, small_total);
  ServerDebug("In the big   memory pool there are %ld nodes, giving %ld bytes.",
         big, big_total);

#if 0
  { void rs232_check(void);
    rs232_check();
  }
#endif
}

#endif /* use_own_memory_management */
/*}}}*/
/*{{{  link I/O */
/*------------------------------------------------------------------------------
--- 
--- The link interface
---
------------------------------------------------------------------------------*/

/**
*** The PC implementation uses the link I/O routines in module linkio.c
*** Hence the only routine that has to be supplied here is the
*** resetlnk() routine, which initialises the function pointers in the
*** linkio.c module.
**/

extern int byte_from_link(unsigned char *x);
extern int byte_to_link(int x);
extern int fetch_block(int count, byte *data, int timeout);
extern int send_block(int count, byte *data, int timeout);
extern int rdrdy(void);
extern int wrrdy(void);

extern void tim40_reset(void);
extern void tim40_init_link(void);
extern int tim40_byte_from_link(unsigned char *x);
extern int tim40_byte_to_link(int x);
extern int tim40_fetch_block(int count, byte *data, int timeout);
extern int tim40_send_block(int count, byte *data, int timeout);
extern int tim40_rdrdy(void);
extern int tim40_wrrdy(void);
extern int hunt_fetch_block(int count, byte *data, int timeout);
extern int hunt_send_block(int count, byte *data, int timeout);
extern int tdb_rdrdy(void);
extern int tdb_wrrdy(void);
extern int tdb_fetch_block(int count, byte *data, int timeout);
extern int tdb_send_block(int count, byte *data, int timeout);

extern void spirit40_reset(void);
extern void spirit40_init_link(void);
extern int spirit40_byte_from_link(unsigned char *x);
extern int spirit40_byte_to_link(int x);
extern int spirit40_fetch_block(int count, byte *data, int timeout);
extern int spirit40_send_block(int count, byte *data, int timeout);
extern int spirit40_rdrdy(void);
extern int spirit40_wrrdy(void);

extern void b004_reset(void);
extern void b004_analyse(void);
extern void b004_init_link(void);
extern void mk026_reset(void);
extern void mk026_analyse(void);
extern void mk026_init_link(void);

/* dos-specific functions */
extern int dos_byte_from_link(unsigned char *x);
extern int dos_byte_to_link(int x);
extern int dos_fetch_block(int count, byte *data, int timeout);
extern int dos_send_block(int count, byte *data, int timeout);
extern int dos_rdrdy(void);
extern int dos_wrrdy(void);
extern void dos_reset(void);
extern int dos_init_link(char *devname);
extern void dos_close_link(void);

/* This bit below is the DOS DEVICE Driver access */
PRIVATE char *dosdevdefname = "LINK1";

/* and tmb16 specific */
extern int tmb16_mode(long mode);

/*
 * sang_reset and sang_analyse include DMA preparing parts !
 */
extern void sang_reset(void);
extern void sang_analyse(void);

/*
 * extern void sang_init_link(void); is not needed
 */

extern int dma_send(int count, int page, int offset);
extern int dma_fetch(int count, int page, int offset);
extern void dma_init(void);

/*
 * VY86PID module with ARM processor
 */
extern void  vy86pid_reset(void);
extern void  vy86pid_analyse(void);
extern void  vy86pid_init_link(void);
extern int   vy86pid_byte_from_link(unsigned char *x);
extern int   vy86pid_byte_to_link(int x);
extern int   vy86pid_fetch_block(int count, byte *data, int timeout);
extern int   vy86pid_send_block(int count, byte *data, int timeout);
extern int   vy86pid_rdrdy(void);
extern int   vy86pid_wrrdy(void);

/*
 * Loughborough Sound Images Ltd. C40 board
 */
extern void  qpc_reset(void);
extern void  qpc_init_link(char *name);
extern int   qpc_byte_from_link(unsigned char *x);
extern int   qpc_byte_to_link(int x);
extern int   qpc_fetch_block(int count, byte *data, int timeout);
extern int   qpc_send_block(int count, byte *data, int timeout);
extern int   qpc_rdrdy(void);
extern int   qpc_wrrdy(void);

/*
 * Sang MegaLink C40 board
 */
extern void  megalinkC40_reset(void);
extern void  megalinkC40_init_link(char *name);
extern int   megalinkC40_byte_from_link(unsigned char *x);
extern int   megalinkC40_byte_to_link(int x);
extern int   megalinkC40_fetch_block(int count, byte *data, int timeout);
extern int   megalinkC40_send_block(int count, byte *data, int timeout);
extern int   megalinkC40_rdrdy(void);
extern int   megalinkC40_wrrdy(void);

/* all the following variables are declared in the assembler file pcasm.cpp */
extern int link_base, link_read, link_write, link_in_status;
extern int link_out_status;
extern int link_reset, link_analyse;
extern int control_write;
extern int dma_request, int_enable, dma_channel;
extern int reset_timeout, analyse_timeout;

extern int hunt_hiperf_seg;
extern int hunt_timeout;

/* the following pointers are in linkio.c, and have to be initialised */
extern int  (*rdrdy_fn)();
extern int  (*wrrdy_fn)();
extern int  (*byte_to_link_fn)();
extern int  (*byte_from_link_fn)();
extern int  (*send_block_fn)();
extern int  (*fetch_block_fn)();
extern void (*reset_fn)();
extern void (*analyse_fn)();

/**
*** Initialise the link I/O. Hardware supported is as follows :
***
*** host = PC or AT, in fact there is no difference between the two
***
***
*** TMS320C40 support
*** ~~~~~~~~~~~~~~~~~
*** TIM40       - Simple TIM-40 standard interface (similar to B004).
*** HEPC2       - Hunt Engineering HEPC2 high performance TIM-40 fifo interface.
*** TDMB409     - Transtech name for the above
*** TDB416      - Transtec/A.G.Electronics
*** DSP1        - Hema board, transputer link interface connected to two C40 links
*** SPIRIT40    - Sonitech Spirit40 board with keyhole shared memory
*** QPCV1       - LSI board, version 1
*** QPCV2       - LSI board, version 2
*** MEGALINKC40 - Sang C40 board
***
*** @@@ to be added: DOSDEVICE, CONTEXT (performance lib implementation), etc.
***
*** TRANSPUTER support
*** ~~~~~~~~~~~~~~~~~~
*** for the MEGA-Link-DMA the ML-DMA runs on ATs only.
***
*** ifdef GEMINI, the Transtar router board at base 0x100, with
*** optional DMA. This is B008 compatible.
***
*** else :
*** B004, the original Inmos board, usually at 0x150, no DMA
***
*** B008, the tram board, usually at 0x150, with DMA
***
*** MK026, the Meiko interface board, usually at 0x100, with DMA but
*** no idea how to use it
***
*** CESIUS, from Meiko spinoff company, MK026 compatible but at address
*** 0x180
***
*** SANG - MEGA-Link-DMA from SANG Computersysteme GmbH, usually at 0x150,
*** B004 compatible, with 16 Bit DMA
***
*** ARM support
*** ~~~~~~~~~~~
*** VY86PID - PID Module, uses RS232 lines for communication
**/

PRIVATE int dma_send_block( unsigned int count, byte *data, int timeout);
PRIVATE int dma_fetch_block(unsigned int count, byte *data, int timeout);


PRIVATE int sang_send_block(unsigned int count, byte huge *data, int timeout);
PRIVATE int sang_fetch_block(unsigned int count, byte huge *data, int timeout);

/*
 * additional definitions and additional varibles
 * for link and DMA handling
 */
 

#define LINK_BASE_DEFAULT          0x150
#define LINK_READ_OFFSET          0
#define LINK_WRITE_OFFSET         1
#define LINK_IN_STATUS_OFFSET     2
#define LINK_OUT_STATUS_OFFSET    3
#define LINK_RESET_OFFSET         0x10
#define LINK_ERROR_OFFSET         0x10
#define LINK_ANALYSE_OFFSET       0x11

/*** MEGA-Link HOST CONTROL, AT 16 BIT DMA ***/
#define DMA_REQUEST		0x12
#define INT_ENABLE		0x13

#define FIFO_ACCESS_LOC		0x08
#define DMA_DLA_CNTRL		0x12
#define INT_STAT_LOC		0x13

#define DLA_OFF_WRITE		0x00
#define DLA_OFF_READ		0x01
#define DLA_ON_WRITE		0x02
#define DLA_ON_READ		0x03

PRIVATE int dma_request;
PRIVATE int int_enable;
PRIVATE int fifo_acc;
PRIVATE int dmla_control;
PRIVATE int dmla_info;

#define FIFO_LENGTH		1024
#define FIFO_EMPTY		0x01
#define FIFO_FULL		0x02


void resetlnk()
{ word temp, conf_base, dma_chan;
  char *host, *box, *dosdevname;
  char *tmb16;
  PRIVATE int already_done = 0;

  if (already_done)
   return;
  else
   already_done = 1;
 
     /* extract data from the configuration file */ 
  host      = get_config("host");
  box       = get_config("box");
  conf_base = get_int_config("link_base");
  dma_chan	= get_int_config("dma_channel");

     /* the following are hardware independent, and fairly irrelevant */
     /* by now. */
  if ((temp = get_int_config("reset_timeout")) ne Invalid_config)
    reset_timeout = (int) temp;
  if ((temp = get_int_config("analyse_timeout")) ne Invalid_config)
    analyse_timeout = (int) temp;

            /* the function pointers can be initialised to B004 defaults */ 
  rdrdy_fn          = func(rdrdy);
  wrrdy_fn          = func(wrrdy);
  byte_to_link_fn   = func(byte_to_link);    
  byte_from_link_fn = func(byte_from_link);
  send_block_fn		= func(send_block);
  fetch_block_fn    = func(fetch_block);
  reset_fn          = func(b004_reset);
  analyse_fn        = func(b004_analyse);

  if  (host eq NULL || box eq NULL)
    { ServerDebug("Missing entries in configuration file for HOST and BOX.");
      longjmp(exit_jmpbuf, 1);
    }

#ifdef GEMINI
  { extern void unlock_gemini();
    unlock_gemini();
    link_base       = (conf_base eq Invalid_config) ? 0x100 : (int) conf_base;
    link_read       = link_base + 0x00;
    link_write      = link_base + 0x01;
    link_in_status  = link_base + 0x02;
    link_out_status = link_base + 0x03;
    link_reset      = link_base + 0x10;
    link_analyse    = link_base + 0x11;
    dma_request     = link_base + 0x12;
    int_enable      = link_base + 0x13;
    
    b004_init_link();

    if (dma_chan ne Invalid_config)
     { dma_channel    = (int) dma_chan;
       send_block_fn  = func(dma_send_block);
       fetch_block_fn = func(dma_fetch_block);
       dma_init();
     }
    return;
  }
#else

  if ( (mystrcmp(host, "PC") && mystrcmp(host, "AT") ) ||
        mystrcmp(box, "TIM40") &&
        mystrcmp(box, "HEPC2") &&
        mystrcmp(box, "TDMB409") &&
        mystrcmp(box, "TDB416") &&
	mystrcmp(box, "DSP1") &&
        mystrcmp(box, "SPIRIT40") &&
        mystrcmp(box, "VY86PID") &&
        mystrcmp(box, "QPCV1") &&
        mystrcmp(box, "QPCV2") &&
        mystrcmp(box, "MEGALINKC40") &&
        /* mystrcmp(box, "DOSDEVICE") && */
       (mystrcmp(box, "B004") && mystrcmp(box, "B008") &&
        mystrcmp(box, "SANG") &&
        mystrcmp(box, "MK026") && mystrcmp(box, "CESIUS") &&
        mystrcmp(box, "DOSDEVICE") ) )
    { ServerDebug("Hardware configuration not supported.");
      ServerDebug("Supported hosts are PC and AT.");
      ServerDebug("Supported boards are:");
      ServerDebug("Transputer: B004, B008, MK026, CESIUS, DOSDEVICE%q");
      ServerDebug(" and sang");
      ServerDebug("");
      ServerDebug("C40       : TIM40, HEPC2, TDB416, DSP1, SPIRIT40, TDMB409, QPCV1, QPCV2,");
      ServerDebug("          : MEGALINKC40");
      ServerDebug("Arm       : VY86PID");
      longjmp(exit_jmpbuf, 1);
    }

  /* TMS320C40 'box' implementations */

  if (!mystrcmp(box, "TIM40")) {
    /* TIM40 defaults */
    rdrdy_fn          = func(tim40_rdrdy);
    wrrdy_fn          = func(tim40_wrrdy);
    byte_to_link_fn   = func(tim40_byte_to_link);	/* not actually used */
    byte_from_link_fn = func(tim40_byte_from_link);	/* not actually used */
    send_block_fn     = func(tim40_send_block);
    fetch_block_fn    = func(tim40_fetch_block);
    reset_fn          = func(tim40_reset);
    analyse_fn        = func(tim40_reset);

    link_base       = (conf_base eq Invalid_config) ? 0x150 : (int) conf_base;
    link_read       = link_base + 0x00;
    link_write      = link_base + 0x01;
    link_in_status  = link_base + 0x02;
    link_out_status = link_base + 0x03;
    link_reset      = link_base + 0x06;
    tim40_init_link();		/* currently a no-op */
    return;
   }

  if (!mystrcmp(box, "HEPC2") || !mystrcmp(box, "TDMB409") )
   {
	word	fifo_base;

    /* HEPC2 defaults - same as tim40, with the exception of the block ops. */
    rdrdy_fn          = func(tim40_rdrdy);
    wrrdy_fn          = func(tim40_wrrdy);
    byte_to_link_fn   = func(tim40_byte_to_link);	/* not actually used */
    byte_from_link_fn = func(tim40_byte_from_link);	/* not actually used */
    send_block_fn     = func(hunt_send_block);
    fetch_block_fn    = func(hunt_fetch_block);
    reset_fn          = func(tim40_reset);
    analyse_fn        = func(tim40_reset);

    link_base       = (conf_base eq Invalid_config) ? 0x150 : (int) conf_base;
    link_read       = link_base + 0x00;
    link_write      = link_base + 0x01;
    link_in_status  = link_base + 0x02;
    link_out_status = link_base + 0x03;
    link_reset      = link_base + 0x06;

    hunt_timeout    = link_base + 0x07;

    fifo_base = get_int_config("hepc2_fifo_base");
    if (fifo_base == Invalid_config)
	fifo_base = get_int_config("tdmb40x_fifo_base");
    if (fifo_base == Invalid_config)
	fifo_base = 0xd0000;

#if !(MSWINDOWS)
	/* In the standard DOS environment, the fifo can be accessed	*/
	/* directly.		`					*/
    hunt_hiperf_seg = (fifo_base >> 4) & 0xffff;
#else
	/* In a protected Windows environment, DPMI will be required	*/
	/* to allocate a descriptor which can be used to access the fifo*/
   { union  _REGS	regs;
     struct _SREGS	sregs;

     _segread(&sregs);
     regs.x.ax = 0x02;	/* Segment to descriptor */
     regs.x.bx = (int) ((fifo_base >> 4) & 0x0FFFF);
     _int86x(0x31, &regs, &regs, &sregs);
     if (regs.x.cflag)
      { ServerDebug("Failed to map fifo into address space.");
        longjmp(exit_jmpbuf, 1);
      }
     hunt_hiperf_seg = regs.x.ax;
   }
#endif
    tim40_init_link();		/* currently a no-op */
    return;
   }


  if (!mystrcmp(box, "TDB416")) {
    /* 16 bit interface with one word read buffer on write */
    rdrdy_fn          = func(tdb_rdrdy);
    wrrdy_fn          = func(tdb_wrrdy);
    byte_to_link_fn   = func(tim40_byte_to_link);	/* not actually used */
    byte_from_link_fn = func(tim40_byte_from_link);	/* not actually used */
    send_block_fn     = func(tdb_send_block);
    fetch_block_fn    = func(tdb_fetch_block);
    reset_fn          = func(tim40_reset);
    analyse_fn        = func(tim40_reset);

    link_base       = (conf_base eq Invalid_config) ? 0x150 : (int) conf_base;
    link_read       = link_base + 0x10;	/* 16 bit interface ports */
    link_write      = link_base + 0x12;
    link_in_status  = link_base + 0x14;
    link_out_status = link_base + 0x16;
    link_reset      = link_base + 0x06;
    tim40_init_link();		/* currently a no-op */
    return;
   }

  if( !mystrcmp(box, "SPIRIT40") )
  {
    rdrdy_fn          = func(spirit40_rdrdy);
    wrrdy_fn          = func(spirit40_wrrdy);
    byte_to_link_fn   = func(spirit40_byte_to_link);	/* not actually used */
    byte_from_link_fn = func(spirit40_byte_from_link);	/* not actually used */
    send_block_fn     = func(spirit40_send_block);
    fetch_block_fn    = func(spirit40_fetch_block);
    reset_fn          = func(spirit40_reset);
    analyse_fn        = func(spirit40_reset);
    link_base       = (conf_base eq Invalid_config) ? 0x300 : (int) conf_base;
    spirit40_init_link();
    return;
  }

  if ( !mystrcmp(box, "QPCV1") || !mystrcmp(box, "QPCV2"))
  {
    rdrdy_fn          = func(qpc_rdrdy);
    wrrdy_fn          = func(qpc_wrrdy);
    byte_to_link_fn   = func(qpc_byte_to_link);	/* not actually used */
    byte_from_link_fn = func(qpc_byte_from_link);	/* not actually used */
    send_block_fn     = func(qpc_send_block);
    fetch_block_fn    = func(qpc_fetch_block);
    reset_fn          = func(qpc_reset);
    analyse_fn        = func(qpc_reset);
    qpc_init_link(box);
    return;
  }
  if ( !mystrcmp(box, "MEGALINKC40"))
  {
    rdrdy_fn          = func(megalinkC40_rdrdy);
    wrrdy_fn          = func(megalinkC40_wrrdy);
    byte_to_link_fn   = func(megalinkC40_byte_to_link);		/* not actually used */
    byte_from_link_fn = func(megalinkC40_byte_from_link);	/* not actually used */
    send_block_fn     = func(megalinkC40_send_block);
    fetch_block_fn    = func(megalinkC40_fetch_block);
    reset_fn          = func(megalinkC40_reset);
    analyse_fn        = func(megalinkC40_reset);
    megalinkC40_init_link(box);
    return;
  }

	/* ARM systems */
  if( !mystrcmp(box, "VY86PID") )
  {
    rdrdy_fn          = func(vy86pid_rdrdy);
    wrrdy_fn          = func(vy86pid_wrrdy);
    byte_to_link_fn   = func(vy86pid_byte_to_link);
    byte_from_link_fn = func(vy86pid_byte_from_link);
    send_block_fn     = func(vy86pid_send_block);
    fetch_block_fn    = func(vy86pid_fetch_block);
    reset_fn          = func(vy86pid_reset);
    analyse_fn        = func(vy86pid_analyse);
    vy86pid_init_link();
    return;
  }
   
   /* TRANSPUTER 'box' implementations, also the DSP1 which uses a	*/
   /* transputer link adapter.						*/
  if ((!mystrcmp(box, "b004")) || (!mystrcmp(box, "DSP1")))
   {
    link_base       = (conf_base eq Invalid_config) ? 0x150 : (int) conf_base;
    link_read       = link_base + 0x00;
    link_write      = link_base + 0x01;
    link_in_status  = link_base + 0x02;
    link_out_status = link_base + 0x03;
    link_reset      = link_base + 0x10;
    link_analyse    = link_base + 0x11;
    b004_init_link();
    return;
   }

  if (!mystrcmp(box, "sang"))
   {
    /*
     * The MEGA-Link-DMA is B004 compatible.
     * So these registers remain unchanged.
     */
        link_base       = (conf_base eq Invalid_config)
			? LINK_BASE_DEFAULT
			: (int) conf_base;
        link_read = link_base + LINK_READ_OFFSET;
        link_write = link_base + LINK_WRITE_OFFSET;
        link_in_status = link_base + LINK_IN_STATUS_OFFSET;
        link_out_status = link_base + LINK_OUT_STATUS_OFFSET;
        link_reset = link_base + LINK_RESET_OFFSET;
        link_analyse = link_base + LINK_ANALYSE_OFFSET;
	/*** added ***/

	dma_request = link_base + DMA_REQUEST;
	int_enable = link_base + INT_ENABLE;
	fifo_acc = link_base + FIFO_ACCESS_LOC;
	dmla_control = link_base + DMA_DLA_CNTRL;
	dmla_info = link_base + INT_STAT_LOC;

    b004_init_link();
    /*
     * The initialization for the DMA/C012 will be done 
     * by the reset routines (not on each call for data
     * transfer !)
     */
    reset_fn        = func(sang_reset);
    analyse_fn      = func(sang_analyse);
    send_block_fn  = func(sang_send_block);

    fetch_block_fn = func(sang_fetch_block); 

   }

  if (!mystrcmp(box, "DOSDEVICE"))
   {
      if ((dosdevname = get_config("dosdevname")) == NULL)
         dosdevname = dosdevdefname;
      
      if (dos_init_link(dosdevname)) {	/* Initialise Driver */
         ServerDebug("Dos Device Driver for host board '%s' not found.",dosdevname);
         longjmp(exit_jmpbuf, 1);
      }
      
      if ((tmb16 = get_config("tmb16")) != NULL) {
      	 /* Put into slave mode */
      	 tmb16_mode(0x00040000L);

     	 /* Put into either byte or word mode as requested */
#ifdef TMB16      
      	 if (mystrcmp(tmb16,"word"))
      	    tmb16_mode(0x00050000L);	/* Reset into byte mode */
      	 else
      	    tmb16_mode(0x00060000L);	/* String mode */
#else
	tmb16_mode(0x00050000L);	/* Reset into byte mode always */
#endif
      }
      rdrdy_fn          = func(dos_rdrdy);
      wrrdy_fn          = func(dos_wrrdy);
      byte_to_link_fn   = func(dos_byte_to_link);    
      byte_from_link_fn = func(dos_byte_from_link);
      send_block_fn     = func(dos_send_block);
      fetch_block_fn    = func(dos_fetch_block);
      reset_fn          = func(dos_reset);
      analyse_fn        = func(dos_reset);	/* NO IDEA WHAT IT ACTUALLY IS */
   }
   
  if (!mystrcmp(box, "b008"))
   {
    link_base       = (conf_base eq Invalid_config) ? 0x150 : (int) conf_base;
    link_read       = link_base + 0x00;
    link_write      = link_base + 0x01;
    link_in_status  = link_base + 0x02;
    link_out_status = link_base + 0x03;
    link_reset      = link_base + 0x10;
    link_analyse    = link_base + 0x11;
    dma_request     = link_base + 0x12;
    int_enable      = link_base + 0x13;
    
    b004_init_link();

    if (dma_chan ne Invalid_config)
     { dma_channel    = (int) dma_chan;
       send_block_fn  = func(dma_send_block);
       fetch_block_fn = func(dma_fetch_block);
       dma_init();
     }
    return;
   }

  if (!mystrcmp(box, "MK026")) 
   {
    link_base       = (conf_base eq Invalid_config) ? 0x100 : (int) conf_base;
    link_read       = link_base + 0x00;
    link_write      = link_base + 0x01;
    link_in_status  = link_base + 0x02;
    link_out_status = link_base + 0x03;
    control_write   = link_base + 0x04;
    mk026_init_link();
    reset_fn        = func(mk026_reset);
    analyse_fn      = func(mk026_analyse);
    return;
   }

  if (!mystrcmp(box, "CESIUS"))  /* Another meiko board */
   {
    link_base       = (conf_base eq Invalid_config) ? 0x180 : (int) conf_base;
    link_read       = link_base + 0x00;
    link_write      = link_base + 0x01;
    link_in_status  = link_base + 0x02;
    link_out_status = link_base + 0x03;
    link_reset      = link_base + 0x10;
    link_analyse    = link_base + 0x11;
    mk026_init_link();
    reset_fn        = func(mk026_reset);
    analyse_fn      = func(mk026_analyse);
    return;
   }
#endif		/* GEMINI */
}

void tidy_link(void) {
  char *box;
  
  box       = get_config("box");
  if (!mystrcmp(box, "DOSDEVICE")) {
  	dos_close_link();
  }
}

PRIVATE int dma_send_block(unsigned int count, byte *buffer, int timeout)
{ if (count < 512) return(send_block(count, buffer, timeout));

  {  unsigned int segment = FP_SEG(buffer);
     unsigned int offset  = FP_OFF(buffer);
     unsigned int bufptr, temp2, page;

     bufptr = ((16 * segment) & 0x0FFFF) + offset;      /* real address */
     page = (segment >> 12) & 0x00FF;
     if (bufptr < ((16 * segment) & 0x0FFFF)) page++;   /* overflow to next page */


       /* Now, I must transfer count bytes, at offset temp, within page */
       /* This may overflow... */
     if (((bufptr + count) & 0x0FFFF) > bufptr)   /* no overflow */
      return(!dma_send(count, page, bufptr));
     temp2 = (unsigned int) (0x10000L - (UWORD) count);
     if (!dma_send(temp2, page, bufptr)) return(1);   /* rest of current page */
     return(!dma_send(count - temp2, page+1, 0));     /* and bit of next page */
  }
}

PRIVATE int dma_fetch_block(unsigned int count, byte *buffer, int timeout)
{ if (count < 512) return(fetch_block(count, buffer, timeout));

  {  unsigned int segment = FP_SEG(buffer);
     unsigned int offset  = FP_OFF(buffer);
     unsigned int bufptr, temp2, page;
     bufptr = ((16 * segment) & 0x0FFFF) + offset;      /* real address */
     page = (segment >> 12) & 0x00FF;
     if (bufptr < ((16 * segment) & 0x0FFFF)) page++;   /* overflow to next page */

       /* Now, I must transfer count bytes, at offset temp, within page */
       /* This may overflow... */
     if (((bufptr + count) & 0x0FFFF) > bufptr)   /* no overflow */
      return(!dma_fetch(count, page, bufptr));

     temp2 = (unsigned int) (0x10000L - (UWORD) count);
     if (!dma_fetch(temp2, page, bufptr)) return(1);   /* rest of current page */
     return(!dma_fetch(count - temp2, page+1, 0));     /* and bit of next page */
  }
}

/*
 ************************************************************************
 * adding the Ml-DMA routines by including SANGFIFO.C
 ************************************************************************
 */

#include "sangfifo.c"



/*}}}*/
/*{{{  misc DOS routines */
/*------------------------------------------------------------------------------
--
-- PC-specific utilities
--
------------------------------------------------------------------------------*/

/**
*** Release 3.6 addition. This routine should suspend the program for
*** the specified number of micro-seconds (like Helios's Delay() ),
*** preferably without using up CPU time.
**/

void goto_sleep(Helios_time)
word Helios_time;
{ clock_t delay;

  Helios_time = 10L * Helios_time;
  Helios_time = divlong(Helios_time, OneSec);
  Helios_time = 100L * Helios_time;
  delay = clock() + Helios_time;

  while (delay > clock());
}

word divlong(a, b)
word a, b;
{ ldiv_t temp;
 
  temp = ldiv(a, b);
  return(temp.quot);
}
/**
*** The following bits of code handle time and date stamp conversions.
**/

                 /* this routine takes an MSdos time stamp and converts it to */
                 /* Unix seconds since 1970 */
static word monthlen[13] = { 0L, 31L, 28L, 31L, 30L, 31L, 30L, 31L, 31L, 30L,
                             31L, 30L, 10000L };
PRIVATE word unixtime(MSdate, MStime)
int MSdate, MStime;
{ int  years, months;
  word days, hours, minutes, seconds;
  word count; int i;


  years   = ( MSdate >> 9) & 0x7F;  /* since 1980 */
  months  = ( MSdate & 0x01E0) >> 5;
  days    = ( MSdate & 0x001F);
  hours   = ( MStime >> 11) & 0x1F;
  minutes = ( MStime & 0x07E0) >> 5;
  seconds = ( MStime & 0x001F) * 2;

  count = (((word)years + 10L) * 365L) + 2L;  /* days since 1970, 2 leapyears */
                                              /* between 1970 and 1980 */
  for (i=0; i < years; i++)
    if (i%4 eq 0) count++;          /* leap years since 1980 */
                                    /* year 2000 ??? */
  for (i=1; i < months; i++)
   count = count + monthlen[i];     /* add months */

  if ((years%4 eq 0) && (months > 2))   /* is year a leap year ? */
   count++;

  count += days-1L;       /* this should be days since 1970 */

  count = (24L * count) + hours;
  count = (60L * count) + minutes;
  count = (60L * count) + seconds;
  return(count);
}

                                /* Get an MSdos date stamp given a Unix stamp */
PRIVATE int MSdate_stamp(unix)
word unix;
{ int days = (int) divlong(unix, (24L * 60L * 60L));
  int month = 1, curyear = 1970;
  int result;

  while (days >= ((curyear % 4) ? 365 : 366))      /* get to right year */
   days -= ((curyear++ % 4) ? 365 : 366);

  if (curyear < 1980) return(0);

  monthlen[2] = (curyear % 4) ? 28l : 29L;     /* alter length of February */

  while (days >= (int) monthlen[month])             /* get to right month */
    days -= (int) monthlen[month++];

  monthlen[2] = 28L;              /* restore February */

  result = ((curyear - 1980) << 9) + (month << 5) + days + 1;
  return(result); 
}

                                  /* Get an MSdos time stamp given a Unix one */
PRIVATE int MStime_stamp(unix)
word unix;
{ int seconds, minutes, hours, result;

  ldiv_t temp;
  temp = ldiv(unix, 60L); seconds = (int) temp.rem; unix = temp.quot;
  temp = ldiv(unix, 60L); minutes = (int) temp.rem; unix = temp.quot;
  temp = ldiv(unix, 24L); hours   = (int) temp.rem;

  result = (hours << 11) + (minutes << 5) + (seconds >>  1);
  return(result);
}


/*------------------------------------------------------------------------------
--
-- The PC device routines
--
------------------------------------------------------------------------------*/

/**
*** I need these register structures for when I do MSdos system calls via the
*** C library routines.
**/
union _REGS rg, outregs;
struct _SREGS segregs;

/**
*** Device initialisation
***
*** PC_setmode is used to set the input and output streams to raw/cooked
*** modes, avoiding problems with ctrl-C, ctrl-S, etc. These should not
*** actually be necessary as the server performs its output at a very low
*** level anyway, but they do no harm.
**/

static void PC_setmode(handle,flag)
int handle,flag;
{ rg.x.ax = 0x4400;
  rg.x.bx = handle;

  _intdos(&rg,&rg);              /* Get current state */
  rg.h.dh = 0;                  /* Ensure high byte clear */
  if (flag)
    rg.h.dl &= 0xDF;            /* Unset raw mode */
  else
    rg.h.dl |= 0x20;            /* Set mode */
  rg.x.ax = 0x4401;             /* Set stream status */
  _intdos(&rg,&rg);
}


/**
*** The time routines, get and set the system time
**/

word get_unix_time()
{ struct dostime_t mytime; struct dosdate_t mydate;
  unsigned int MStime, MSdate;

  _dos_gettime(&mytime); _dos_getdate(&mydate);

  MSdate = ((mydate.year-1980) << 9) + (mydate.month << 5) + mydate.day;
  MStime = (mytime.hour << 11) + (mytime.minute << 5) + (mytime.second >> 2);

  return(unixtime(MSdate, MStime));
}

        /* The code below has to set the current time to unixstamp */
void set_current_time(unixstamp)
word unixstamp;
{ int MSdate, MStime;
  struct dosdate_t date_tmp; struct dostime_t time_tmp;
  MSdate = MSdate_stamp(unixstamp);
  MStime = MStime_stamp(unixstamp);


  date_tmp.day     = (unsigned char) (MSdate & 0x1F);
  date_tmp.month   = (unsigned char) ((MSdate >> 5)  & 0x0F);
  date_tmp.year    = ((MSdate >> 9)  & 0x7F)+1980;       /* ignore dayofweek... */
  time_tmp.hour    = (unsigned char) ((MStime >> 11) & 0x1F); 
  time_tmp.minute  = (unsigned char) ((MStime >> 5)  & 0x3F);
  time_tmp.second  = (unsigned char) ((MStime & 0x1F) << 1);
  time_tmp.hsecond = 0;
  _dos_setdate(&date_tmp); _dos_settime(&time_tmp);
}
/*}}}*/
/*{{{  GEM support */
/**
*** The local GEM VDI calls. I hope that the test for a loaded Gem works
*** for all versions.
**/

void Gem_Testfun(a_ptr)
bool *a_ptr;
{ char far * gemstring;
  int * gemvector = (int *) ((4 * 0xef) & 0xffff);
  int * stringvec;

  stringvec = (int *) &gemstring; 
  gemstring = 0;
  *stringvec++ = *gemvector++;
  *stringvec = *gemvector;

  gemstring += 2;
  if (*gemstring++ eq 'G' && *gemstring++ eq 'E' && *gemstring eq 'M')
      *a_ptr = true;
    else
      *a_ptr = false;
}

void vdi(blockpoint)
int **blockpoint;
{ unsigned long x = (unsigned long) blockpoint;
  rg.x.dx = (int) (x & 0x0000ffffL);
  segregs.ds = (int) ((x >> 16) & 0x0000ffffL);
  rg.x.cx = 0x473;

  _int86x(0xef, &rg, &outregs, &segregs) ;
}
/*}}}*/
/*{{{  multiple windows */
/**
*** This supports multiple windows on the PC by foul means and fair. Each
*** window has its own shadow screen. Only the current window is actually
*** displayed, so if the user switches screens by ALT-F1 I have to redraw
*** the entire screen from its shadow. If Helios is outputting to a
*** window other than the current one the output is just discarded. Similarly
*** when a window other than the current one is polled for keyboard inputs
*** the routine returns straightaway.
***
*** There is a special window, the Server_window, used for the Server's
*** output. If there is any output to the Server_window it is brought to
*** the front straightaway. Each window has a unique identifier, and the
*** Server_window's identifier is always 1. I use a special key sequence,
*** ALT-shift-F1, to toggle between the Server_window and the current window.
*** This may upset people who discover that their input is suddenly going
*** to the wrong window.
***
*** If the user starts up a Gem application, then this takes over the
*** screen completely and no further output is possible. Also note that
*** some of the initialisation of the screen is done when the server window
*** is opened or closed.
**/

#if (multiple_windows && !MSWINDOWS)
#define Server_handle 1
#define Gem_handle    -1
PRIVATE word next_handle = Server_handle;
word current_handle;
PRIVATE word Helios_handle;
PRIVATE word Gem_saved_handle = Server_handle;
void switch_window(int);
extern  void redraw_screen(Window *window);

word create_a_window(name)
char *name;
{ current_handle = next_handle;
  if (next_handle eq Server_handle)
   { extern void vbios_init(void);
     vbios_init();
   }

  return(next_handle++);
}

void close_window(handle)
word handle;
{ if ((handle eq current_handle) && (handle ne Server_handle))
   { if (Special_Exit)
      { current_handle = Server_handle;
        redraw_screen(&Server_window);
      }
     else
       switch_window(0);
   }
  elif (handle eq Server_handle)
   { extern void vbios_tidy(void);
     vbios_tidy();
   }
}

void window_size(handle, x, y)
word handle;
word *x, *y;
{ *x = 80L; *y = 25L;
}

/**
*** This routine is used to check whether output is actually going to the
*** current window or not. It is slightly subtle : if the current window
*** is gem then the output will never be displayed; if the output is
*** destined for the Server's window then that is brought to the front.
*** Otherwise the output must not be displayed.
**/
int check_window(handle)
word handle;
{
  if (handle ne current_handle)
   { if (Server_windows_nopop)
       return(0);   /* Discard output because must not switch */

#if gem_supported
     if (current_handle eq Gem_handle)
       return(0);
#endif

     if (handle eq Server_handle)
      { Helios_handle  = current_handle;
        current_handle = Server_handle;
        redraw_screen(&Server_window);
        return(1);
      }
     else
      return(0);   /* Discard output to this window */
   }
  else
   return(1);
}

void switch_window(int direction)
{ Window *window;

  if (current_handle eq Gem_handle)
    return;

  if (current_handle eq Server_handle)  /* switch to first or last window */
   { if (direction)
      window = (Window *) Window_List.list.head;
     else
      window = (Window *) Window_List.list.tail;
   }
  else    /* find current window */
   { for (window = (Window *) Window_List.list.head;
          window->node.node.next ne (Node *) NULL;
          window = (Window *) window->node.node.next )
      if (window->handle eq current_handle)
          break;
     if (direction)  /* switch to next or previous */
      { window = (Window *) window->node.node.next;
        if (window->node.node.next eq (Node *) NULL)
         window = &Server_window;
      }
     else
      { window = (Window *) window->node.node.prev;
      	if (window->node.node.prev eq (Node *) NULL)
         window = &Server_window;
      }
   }

  if (window ne &Server_window)
   if ( (window->node.node.next eq (Node *) NULL) ||
        (window->node.node.prev eq (Node *) NULL) )
   window = &Server_window;

  current_handle = window->handle;
  redraw_screen(window);
}

/**
*** Special case for switching windows tidily. The routine is called from
*** module gem.c, with true or false.
**/
void switch_to_gem(flag)
{ 
  if (flag)
   { Gem_saved_handle = current_handle;
     current_handle   = Gem_handle;
   }
  else
   { if (Gem_saved_handle eq Server_handle)
      { current_handle = Server_handle;
        redraw_screen(&Server_window);
      }
     else
      { Window *window;
        for (window = (Window *) Window_List.list.head;
             window->node.node.next ne (Node *) NULL;
             window = (Window *) window->node.node.next )
          if (window->handle eq Gem_saved_handle)
            break;
        if (window->node.node.next eq (Node *) NULL)
          window = &Server_window;
        current_handle = window->handle;
        redraw_screen(window);
      }
   }
}

#else
int check_window()
{ return(1);
}
  
#endif /* multiple_windows */

#if !(MSWINDOWS)
/**
*** These routines implement keyboard polling. I need to provide just one
*** routine, read_char_from_keyboard(), which returns a character if there is
*** one available or -1. If there are multiple windows, each window is
*** polled separately, and ALT-F1 and ALT-shift-F1 are used to switch
*** windows.
***
*** For keys which produce more than one character it is important to be able
*** to store the extra characters somewhere
**/
static int keys_head=0, keys_tail=0;
static int keys_buff[16];

static void add_key(key)
int key;
{ keys_buff[keys_head] = key;
  keys_head = (keys_head + 1) &0x0F;
}

/**
*** This is used to deal with function keys. It returns -1 to indicate that
*** no key is available to Helios, or the character to be returned. If there
*** is more than one character the remainder will be buffered.
**/
static int function_key(key, shift, control, alt)
int key, shift, control, alt;
{ word temp;

  if (shift && control)
   {
#if debugger_incorporated
     if (key eq  7) { DebugMode = 1 - DebugMode; return(-1); }
#endif
     if (key eq  8) { Special_Status = true; return(-1); }
     if (key eq  9) { Special_Exit   = true; return(-1); }
     if (key eq 10) { Special_Reboot = true; return(-1); }
   }

#if multiple_windows
 if (((key eq 1) || (key eq 2)) && alt)
   { Window_Testfun(&temp);
     if (temp)
      {
       if (shift)
        { if (current_handle eq Server_handle)
           { Window *window;
             for (window = (Window *) Window_List.list.head;
                  window->node.node.next ne (Node *) NULL;
                  window = (Window *) window->node.node.next )
         	    if (window->handle eq Helios_handle)
               { current_handle = Helios_handle;
                 redraw_screen(window);
               }
           }
          else
           { Helios_handle  = current_handle;
             current_handle = Server_handle;
             redraw_screen(&Server_window);
           }
        }
       else
        switch_window((key eq 1) ? 1 : 0);

       return(-1);
      }
   }
#endif  /* multiple_windows */

  if (alt)
   { if (shift)
       { add_key('3'); add_key('0' + key - 1); }
     else
       { add_key('2'); add_key('0' + key - 1); }
   }
  elif(shift)
   { add_key('1'); add_key('0' + key - 1); }
  else
     add_key('0' + key - 1);

  add_key('~'); 

  return(0x009B);
}

int read_char_from_keyboard(handle)
word handle;
{ extern int keyboard_rtn(void);
  int temp;
  int shiftflags, shift, control, alt, capslock;

#if multiple_windows
  if (handle ne current_handle)
   { if ((current_handle ne Gem_handle) || (handle ne Gem_saved_handle))
       return(-1);
   }
#endif

  if (keys_tail ne keys_head)
   { temp = keys_buff[keys_tail];
     keys_tail = (keys_tail + 1) & 0x0F;
     return(temp);
   }

            /* Check the keyboard for a character, this returns 0 for failure */
  if ((temp = keyboard_rtn()) eq 0)
    return(-1);
            
      /* Having got a character, I may to need to convert it to Helios format */

                        /* pick up shift status immediately in case I need it */
  shiftflags = _bios_keybrd(_KEYBRD_SHIFTSTATUS);

  if (shiftflags & 0x03)   /* check both shift keys */
   shift = 1;
  else
   shift = 0;

  if (shiftflags & 0x40)   /* and for CAPSLOCK */
    capslock = 1;
  else
    capslock = 0;
 
  if (shiftflags & 0x08)    /* and the alt key */
    alt = 1;
  else
    alt = 0;
 
  if (shiftflags & 0x04)    /* check control key */
     control = 1;
  else
     control = 0;

  if ((temp & 0xFF00) eq 0) /* if it is a normal key return it */
    { if (temp <= 0x1A && shift && control) /* could be a debugging flag */
        { int flagchar = temp + 'a' - 1;    /* convert to flag */
          int i;
          if (flagchar eq 'a')
           { if (debugflags eq 0L)
              debugflags = All_Debug_Flags;
             else
              debugflags = 0L;
             return(-1);
           }
          for (i = 0; options_list[i].flagchar ne '\0'; i++)
           if (options_list[i].flagchar eq flagchar)
            { debugflags ^= options_list[i].flag;
              return(-1);
            }
           return(-1);     /* unimplemented debugging flag */
        }

      if (alt && ( (('a' <= temp) && (temp <= 'z')) ||
                   (('A' <= temp) && (temp <= 'Z')) ) )
        return(temp | 0x80);
      else
        return(temp);
    }

  temp &= 0xFF;            /* strip of the bit indicating it is a special key */

                           /* test for the function keys */
  if (0x3b <= temp && temp <= 0x44)
    return(function_key(temp - 0x3A, shift, control, alt));
  elif (0x54 <= temp && temp <= 0x5D)
    return(function_key(temp - 0x53, shift, control, alt));
  elif (0x5E <= temp && temp <= 0x67)
    return(function_key(temp - 0x5D, shift, control, alt));
  elif (0x68 <= temp && temp <= 0x71)
    return(function_key(temp - 0x67, shift, control, alt));
  elif (0x10 <= temp && temp <= 0x32)	/* Alt keys again */
   { static char keytab[] = "qwertyuiop[]  asdfghjkl;'#  zxcvbnm";
     return(0x80 | keytab[temp - 0x10]);
   }
  else
    switch(temp)       /* and the arrow keys - I cannot handle shifts because */
                       /* the PC thinks I am always using right shift */
     { case 0x48 : add_key('A'); return(0x009B); /* Up */
 
       case 0x50 : add_key('B'); return(0x009B); /* Down */

       case 0x4b :
       case 0x73 : add_key('D'); return(0x009B); /* Left */

       case 0x4D :
       case 0x74 : add_key('C'); return(0x009B); /* Right */

       case 0x0F :         /* Shift Tab, would you believe it */
                   return(0x09);

       case 0x52 : add_key('@'); return(0x009B);    /* Insert key */

       case 0x47 : add_key('H'); return(0x009B);    /* Home key */

       case 0x49 : add_key('3'); add_key('z'); return(0x009B); /* page up */

       case 0x53 : return(0x7F);       /* Delete key */

       case 0x4f : add_key('2'); add_key('z'); return(0x009B); /* End */

       case 0x51 : add_key('4'); add_key('z'); return(0x009B); /* Page Down */
       
       default   :  
                   return(-1);          /* key  not recognised */
      }                                 /* So ignore it */
}
#endif
/*}}}*/
/*{{{  RS232 */
#if RS232_supported
/*{{{  description and statics */
/**
*** BLV, 24.11.93
*** The rs232 code has been greatly modified between versions 3.107
*** and 3.108. RS232 has become rather important as it is used to
*** interact with the VY86PID board. In the interests of maintainability
*** nearly all the assembler code has been eliminated and rewritten in C.
***
*** A PC can have upto 7 serial ports, com1 to com7, and I support all of
*** them. For each port I allocate an RS232 structure. This contains the
*** following fields:
***    pointers to input and output buffers
***    counters and maxima for these buffers
***    an overflow buffer to avoid unnecessary XOFF flow control
***    message ports for break and modem events
***    a flags field to maintain the current state.
***    hardware-specific fields, for example port addresses
***
*** In addition there are ComsPort structures, used as an interface
*** between the machine-dependent and independent parts of the I/O Server.
**/

	/* RS232_table holds the hardware details. To implement the vy86pid	*/
	/* board it is public. There are slots for com1 to com7			*/
RS232	*RS232_table	= NULL;

	/* RS232_coms holds the Helios information, including the attributes.	*/
	/* There are upto 9 of these, com1 to com7, default and vy86pid		*/
ComsPort	*RS232_coms	= NULL;

	/* This flag is used to handle line breaks and modem rings.		*/
int	RS232_errno;
	
	/* When using hardware handshaking the I/O Server supports two		*/
	/* different protocols. This boolean and a host.con options controls	*/
	/* which protocol is in use. N.B. the same protocol will be used on all	*/
	/* lines.								*/
PRIVATE int	rs232_duplex	= TRUE;

	/* These keep track of special error conditions. Normally these are not	*/
	/* accessible to the user but they are useful for debugging purposes.	*/
PRIVATE int	rs232_overrun	= FALSE;
PRIVATE int	rs232_framing	= FALSE;
PRIVATE int	rs232_parity	= FALSE;
PRIVATE int	rs232_break	= FALSE;
PRIVATE int	rs232_overflow	= FALSE;

/*}}}*/
/*{{{  hardware description of the 8250 chip */
/**
*** PC RS232 lines are all supposed to be compatible with the 8250 chip used
*** originally, although the actual chips used may vary from machine to
*** machine. The programming interface is a set of nine byte-wide registers
*** accessed via in and out on addresses relative to a suitable base.
*** (Actually the TX, RX and baud registers share the same address). Many of the
*** registers are sets of bits.
***
*** In addition to the 8250 chips it is necessary to worry about the 8259
*** interrupt controller.
**/
	/* Offsets for the various 8250 registers.	*/
#define RX_OFF		0
#define TX_OFF		0
#define BAUDL_OFF	0
#define BAUDH_OFF	1
#define IER_OFF		1
#define IIR_OFF		2
#define LCR_OFF		3
#define MCR_OFF		4
#define LSR_OFF		5
#define MSR_OFF		6

	/* Bits for the various registers		*/
#define	IER_RxAvail		0x01
#define	IER_TxEmpty		0x02
#define	IER_RxStatus		0x04
#define IER_ModemStatus		0x08
#define	IER_AllInterrupts	0x0F

#define IIR_Pending		0x01
#define IIR_Bit0		0x02
#define IIR_Bit1		0x04
#define IIR_Mask		(IIR_Bit1 | IIR_Bit0)
#define IIR_RxStatus		(IIR_Bit1 | IIR_Bit0)
#define IIR_RxAvail		(IIR_Bit1)
#define IIR_TxEmpty		(IIR_Bit0)
#define IIR_ModemStatus		(0)

#define LCR_WordSelectBit0	0x001
#define LCR_WordSelectBit1	0x002
#define LCR_WordSelect5Bits	(0x000)
#define LCR_WordSelect6Bits	(LCR_WordSelectBit0)
#define LCR_WordSelect7Bits	(LCR_WordSelectBit1)
#define LCR_WordSelect8Bits	(LCR_WordSelectBit1 | LCR_WordSelectBit0)
#define LCR_StopBits		0x004
#define LCR_2StopBits		(0x004)
#define LCR_1StopBit		(0x000)
#define LCR_ParityEnable	0x008
#define LCR_EvenParity		0x010
#define LCR_StickParity		0x020
#define LCR_SetBreak		0x040
#define LCR_DLAB		0x080

#define MCR_DTR			0x001
#define MCR_RTS			0x002
#define MCR_OUT1		0x004
#define MCR_OUT2		0x008
#define MCR_LOOP		0x010

#define LSR_RxAvail		0x001
#define LSR_OverrunError	0x002
#define LSR_ParityError		0x004
#define LSR_FramingError	0x008
#define LSR_BreakInterrupt	0x010
#define LSR_TxEmpty		0x020
#define LSR_ThEmpty		0x040

#define MSR_DeltaCTS		0x001
#define MSR_DeltaDSR		0x002
#define MSR_TrailRI		0x004
#define MSR_DeltaLSD		0x008
#define MSR_CTS			0x010
#define MSR_DSR			0x020
#define MSR_RI			0x040
#define MSR_LSD			0x080

	/* 8259 support				*/
#define Mask8259		0x021
#define Ctrl8259		0x020
#define EndOfInterrupt8259	0x020

PRIVATE int save_8259;
/*}}}*/
/*{{{  description of hardware handshaking */
/**
*** Hardware handshaking. This is a big problem as there is no real standard
*** as to how the handshake lines are interpreted. Typically an rs232 port has
*** two outputs and two inputs for hardware handshaking:
***   RTS - output, request to send
***   CTS - input,  clear to send
***   DTR - output, data terminal ready
***   DSR - input,  data set ready
***
*** There are lots of additional outputs and inputs, but these can be ignored
*** for the purposes of this discussion.
***
*** For simplex communication, for example a computer to a printer, I believe
*** that the correct scheme is as follows:
***   1) the printer should assert DSR on power-up, following initialisation.
***      It indicates that the device is ready. N.B. for a dumb device such as
***      a printer TX, RTS and DTR are inputs whereas RX, CTS and DSR are outputs.
***   2) when the computer wants to send it asserts the RTS line. This indicates
***      a desire to transmit a PACKET of information, not a byte.
***   3) when the printer is ready to receive data it asserts the CTS line. This
***      is detected by the computer which can now start sending.
***   4) the computer asserts the DTR line to indicate that it is transmitting.
***      It can now transfer the entire packet of data.
***   5) when the packet has been transferred the computer drops RTS and DTR.
***      The printer responds by dropping CTS. The transfer is now complete.
***   6) strictly speaking the printer must accept the entire packet after asserting
***      CTS. In practice it is apparently conventional that the printer can lower
***      CTS in the middle of a transfer to avoid overflow and raise it again later.
***
*** The above scheme makes sense for uni-directional traffic, although it is somewhat
*** heavyweight. It does not cope with bi-directional traffic. According to
*** "Interfacing Standards for Computers" by A.C. Maine, an IEEIE monograph, to
*** support bi-directional traffic between two computers a null modem cable should
*** be inserted between the two. This has the following characteristics:
***
***  1) tx and rx are swapped, so both computers can transmit on pin 2 and receive
***     on 3.
***  2) grounds are connected directly at pin 7 and possibly at pin 1.
***  3) the CTS and DSR inputs are connected to the DTR outputs.
***  4) RTS is ignored.
***
*** Basically if a side is able to receive data then it should assert its DTR output.
*** When it is unable to receive data it should lower its DTR output. This gives
*** very simple xon-xoff flow control at the hardware level.
***
*** A variant of this uses RTS/CTS handshaking. The cable swaps RTS and CTS. If
*** a side is able to accept data then it asserts RTS. Sending is only allowed
*** if CTS is high.
***
*** The I/O Server supports two types of handshaking, using a host.con option.
***   rs232_handshake_scheme	= simplex
***   rs232_handshake_scheme	= duplex
***
*** In the first case the I/O Server will work as described above for the
*** simplex protocol. For simplicity the DSR line is ignored, i.e. the I/O Server
*** assumes it can send provided that CTS is high.
***
*** In the second case the I/O Server will work as follows:
***  a) the I/O Server will only send if CTS is asserted. The other end can
***     xoff simply by lowering whichever signal is connected to the
***     I/O Server's CTS input, typically its RTS or DTR outputs.
***  b) the I/O Server will usually assert both DTR and RTS, one of which
***     should be connected to the other end's CTS input, allowing the other
***     end to send. DTR and RTS are lowered if the overflow buffer is close
***     to filling up.
***  c) the DSR input is ignored completely.
***
*** By default the I/O Server uses the duplex protocol. This is an incompatible
*** change between versions 3.108 and earlier versions, but it is unlikely to
*** affect customers since few if any use the rs232 support and even fewer
*** will use hardware handshaking.
**/
/*}}}*/
/*{{{  rs232_check() for debugging */
/**
*** This routine is used for debugging only, and is not normally compiled
*** in. Typically it would be called from the memory_map() routine.
**/
#if 0
void rs232_check(void)
{
	int	i;

	for (i = 0; i < MaxRS232; i++)
		if (RS232_table[i].port_base != 0)
		{
			ServerDebug("rs232, com%d at %x: flags %x, IER %x, IIR %x, LCR %x, MCR %x, LSR %x, MSR %x",
				i + 1, RS232_table[i].port_base, RS232_table[i].flags,
				_inp(RS232_table[i].port_base + IER_OFF),
				_inp(RS232_table[i].port_base + IIR_OFF),
				_inp(RS232_table[i].port_base + LCR_OFF),
				_inp(RS232_table[i].port_base + MCR_OFF),
				_inp(RS232_table[i].port_base + LSR_OFF),
				_inp(RS232_table[i].port_base + MSR_OFF));
			ServerDebug("rs232, com%d: inbuf %lx, outbuf %lx, incount %d, outcount %d, overflow_count %d",
				i + 1, RS232_table[i].inbuf, RS232_table[i].outbuf, RS232_table[i].incount,
				RS232_table[i].outcount, RS232_table[i].overflow_count);
		}
	if (rs232_overrun)
	{
		ServerDebug("rs232: overrun error has occurred");
		rs232_overrun = FALSE;
	}
	if (rs232_framing)
	{
		ServerDebug("rs232: framing error has occurred");
		rs232_framing = FALSE;
	}
	if (rs232_parity)
	{
		ServerDebug("rs232: parity error has occurred");
		rs232_parity = FALSE;
	}
	if (rs232_break)
	{
		ServerDebug("rs232: line break has occurred");
		rs232_break = FALSE;
	}
				
}
#endif
/*}}}*/
/*{{{  interrupt routine */
/*{{{  sendcha */
/**
*** RS232_sendcha() is called when some other routine believes it is clear
*** to write. This means that the TX register is empty and, if hardware
*** flow control is enabled, the CTS line is high. In practice there may
*** not be anything to write, or the other side may have used a software
*** XOFF to halt further writes. This routine is also responsible for sending
*** XON/XOFF bytes if appropriate.
***
*** Actual transmission of data is done in the writech() routine.
**/
/*{{{  writech() */
/**
*** Transmit a single character down the port, asserting DTR if appropriate.
**/
PRIVATE void RS232_writech(RS232 *port, int ch)
{
	/* Assert DTR if necessary, for simplex communication.			*/
	if ((port->flags & RS232_UseHardware) &&
	    !(rs232_duplex) &&
	    !(port->current_mcr & MCR_DTR))
	{
		port->current_mcr	|= MCR_DTR;
		_outp(port->port_base + MCR_OFF, port->current_mcr);
	}

	_outp(port->port_base + TX_OFF, ch);
	port->flags &= ~(RS232_ClearToWrite | RS232_TXEmpty);
}
/*}}}*/

PRIVATE void RS232_sendcha(RS232 *port)
{
	/* Send flow control bytes even if the other side has XOFF'ed.	*/
	/* This seems a good idea but I am not sure that it is legal.	*/
	if (port->flags & RS232_NeedToXon)
	{
		port->flags &= ~(RS232_NeedToXon | RS232_inXOFF);
		RS232_writech(port, XONByte);
	}
	elif (port->flags & RS232_NeedToXoff)
	{
		port->flags &= ~RS232_NeedToXoff;
		port->flags |= RS232_inXOFF;
		RS232_writech(port, XOFFByte);
	}
	/* For data, do not send anything if the other side has XOFF'ed	*/
	elif ((port->outbuf != (byte *) NULL) &&
	      !(port->flags & RS232_outXOFF))
	{
		RS232_writech(port, *(port->outbuf));
		port->outcount++;
		if (port->outcount >= port->outmax)
			port->outbuf = (byte *) NULL;
		else
			port->outbuf += 1;
	}
}

/*}}}*/
/*{{{  newcha */
/**
*** RS232_newcha() is called when a character has been received, or when an
*** error has occurred that should generate input. No processing is performed
*** on the character. If I have a buffer I put the character into it, checking
*** for the case of a full buffer. Otherwise I try to put it into the
*** overflow buffer if there is any space. If the overflow buffer is filling
*** up, I check whether it is desirable to send an XOFF.
**/
PRIVATE void RS232_newcha(RS232 *port, Attributes *attr, int ch)
{
	/* Can I fit into the existing buffer ?				*/
	if (port->inbuf != (byte *) NULL)
	{
		*(port->inbuf)	 = (byte) ch;
		port->inbuf	+= 1;
		port->incount	+= 1;
		if (port->incount >= port->inmax)
			port->inbuf = (byte *) NULL;
	}
	/* Otherwise try the overflow buffer				*/	
	elif(port->overflow_count < overflow_max)
	{
		port->overflow_buf[port->overflow_count++] = (byte) ch;
		/* Should I XOFF ?					*/
		if (port->overflow_count > (overflow_max / 2))
		{
			if (IsAnAttribute(attr, RS232_IXOFF) &&
			    !(port->flags & RS232_inXOFF))
			{
				port->flags |= RS232_NeedToXoff;
				if (port->flags & RS232_ClearToWrite)
					RS232_sendcha(port);
            		}
			if ((port->flags & RS232_UseHardware) && (rs232_duplex))
			{
				port->current_mcr &= ~(MCR_DTR | MCR_RTS);
				_outp(port->port_base + MCR_OFF, port->current_mcr);
			}
		}
	}
	else
		rs232_overflow	= TRUE;
}

/*}}}*/
/*{{{  IIR_RxStatus */
/**
*** This routine is called when the 8250 has signalled an error
*** on the line: overrun, parity, framing, or break. The exact
*** way of handling this error depends largely on the current
*** attributes. See "The Helios Parallel Operating System", page 137,
*** for details of the errors. Also see Posix 1003.1, section 7.1.2.2,
*** terminal input modes.
**/
PRIVATE void RS232_handleRxStatus(RS232 *port, Attributes *attr)
{
	int		 LSR;

	LSR	= _inp(port->port_base + LSR_OFF);

	if (LSR & LSR_OverrunError)
	{
		/* N.B. overrun errors are not covered by Posix. This scheme	*/
		/* has been invented.						*/
		rs232_overrun	= TRUE;
		unless (IsAnAttribute(attr, RS232_IgnPar))
		{
			RS232_newcha(port, attr, 0x00FF);
			RS232_newcha(port, attr, 0x0001);
		}
		
	}
	elif (LSR & (LSR_ParityError | LSR_FramingError))
	{
		if (LSR & LSR_ParityError)	rs232_parity	= TRUE;
		if (LSR & LSR_FramingError)	rs232_framing	= TRUE;

		if (IsAnAttribute(attr, RS232_InPck))
		{
				/* Read the corrupted character.		*/
			int	ch;
			if (LSR & LSR_RxAvail)
				ch	= _inp(port->port_base + RX_OFF);
			else
				ch	= 0x00;
				
			unless (IsAnAttribute(attr, RS232_IgnPar))
			{
				if (IsAnAttribute(attr, RS232_ParMrk))
				{
					RS232_newcha(port, attr, 0x00FF);
					RS232_newcha(port, attr, 0x0000);
					RS232_newcha(port, attr, ch);
				}
				else
				{
					RS232_newcha(port, attr, 0x0000);
				}
			}
		}
	}
	elif (LSR & LSR_BreakInterrupt)
	{
		rs232_break	= TRUE;

		if (IsAnAttribute(attr, RS232_IgnoreBreak))
			return;
		if (IsAnAttribute(attr, RS232_BreakInterrupt))
		{
			if (port->breakint != NullPort)
			{
				/* This will cause an event message to be sent.	*/
				/* In theory some thread will receive this	*/
				/* event message and generate a SIGINT.		*/
				port->flags	|= RS232_BreakDetected;
				RS232_errno	= 1;
			}
				/* The input and output queues must be flushed.	*/
			if (port->inbuf != (byte *) NULL)
			{
				port->inmax	= -1;
				port->inbuf	= (byte *) NULL;
			}
			if (port->outbuf != (byte *) NULL)
			{
				port->outmax	= -1;
				port->outbuf	= (byte *) NULL;
			}
		}
		else
		{
			if (IsAnAttribute(attr, RS232_ParMrk))
			{
				RS232_newcha(port, attr, 0x00FF);
				RS232_newcha(port, attr, 0x0000);
				RS232_newcha(port, attr, 0x0000);
			}
			else
				RS232_newcha(port, attr, 0x0000);
		}
	}
	
}
/*}}}*/
/*{{{  IIR_RxAvail */
/**
*** This routine copes with available data. If XON/XOFF flow control is used
*** then start and stop bytes need to be handled specially. Otherwise the
*** character has to be put into the input buffer. There are various input
*** modes to be considered.
**/
PRIVATE void RS232_handleRxAvail(RS232 *port, Attributes *attr)
{
	int		 ch;

	while (_inp(port->port_base + LSR_OFF) & LSR_RxAvail)
	{
		ch	= (_inp(port->port_base + RX_OFF) & 0x00FF);

		/* Check for flow control bytes.				*/
		if (((ch == XONByte) || (ch == XOFFByte)) && IsAnAttribute(attr, RS232_IXON))
		{
			if (ch == XONByte)
			{
				port->flags &= ~RS232_outXOFF;
				if (port->flags & RS232_ClearToWrite)
					RS232_sendcha(port);
			}
			elif (ch == XOFFByte)
				port->flags |= RS232_outXOFF;
		}
		else
		{
			/* Process ordinary input character				*/
			if (IsAnAttribute(attr, RS232_Istrip))
				RS232_newcha(port, attr, ch & 0x007F);

			/* Bytes 0x00FF may have to be handled specially to allow for	*/
			/* errors to be detected. FF is an error marker for parity,	*/
			/* framing and break errors.					*/
			elif (ch == 0x00FF)
			{
				RS232_newcha(port, attr, 0x00FF);
				if (!(IsAnAttribute(attr, RS232_IgnPar)) &&
				    IsAnAttribute(attr, RS232_ParMrk))
				{
					RS232_newcha(port, attr, 0x00FF);
				}
			}
			else
				RS232_newcha(port, attr, ch);
		}
	}
}

/*}}}*/
/*{{{  IIR_TxEmpty */
/**
*** This routine is invoked when a character has been transmitted and the
*** TX register is empty. It may or may not be possible to send a
*** character, depending on the flow control mechanism being used.
**/
PRIVATE void RS232_handleTxEmpty(RS232 *port, Attributes *attr)
{
	while ((_inp(port->port_base + LSR_OFF) & LSR_TxEmpty) &&
		!(port->flags & RS232_TXEmpty))
	{
		port->flags	|= RS232_TXEmpty;
		if (((port->flags & RS232_UseHardware) && (port->flags & RS232_CTS)) ||
		    !(port->flags & RS232_UseHardware))
		{
			port->flags |= RS232_ClearToWrite;
			RS232_sendcha(port);
		}
	}
}
/*}}}*/
/*{{{  IIR_ModemStatus */
/**
*** This routine is called when there has been a change on one of the
*** modem lines. DSR and RLSD are ignored. CTS is important if hardware
*** flow control has been enabled. Modem rings may involve sending
*** event messages to a suitable thread in the Helios network.
**/
PRIVATE void RS232_handleModemStatus(RS232 *port, Attributes *attr)
{
	int		 MSR;

	MSR	= _inp(port->port_base + MSR_OFF);

	if ((MSR & MSR_TrailRI) && (MSR & MSR_RI))
		if (port->modemint != NullPort)
		{
			/* This will cause the event to be sent.	*/
			port->flags |= RS232_RingDetected;
			RS232_errno  = 1;
		}

	if (MSR & CTS)
	{
		port->flags |= RS232_CTS;
		if ( (port->flags & RS232_UseHardware) &&
		     (port->flags & RS232_TXEmpty) &&
		    !(port->flags & RS232_ClearToWrite))
		{
			port->flags |= RS232_ClearToWrite;
			RS232_sendcha(port);
		}
	}
	elif (port->flags & RS232_CTS)
	{
		port->flags &= ~RS232_CTS;
		if (port->flags & RS232_UseHardware)
			port->flags &= ~RS232_ClearToWrite;
	}		
		    	
}
/*}}}*/
/*{{{  interrupt routine */
/**
*** This code is called by the assembler interrupt routine. The latter
*** may be invoked directly in the case of the DOS I/O Server or
*** via DPMI for the Windows I/O Server.
***
*** This code loops through all defined RS232 ports checking the
*** interrupt identification register. If there is a pending interrupt
*** then the exact interrupt is identified and handled by calling one
*** of the routines above. Once all the ports have been checked the
*** 8259 interrupt controller is informed.
***
*** This code assumes that interrupts are currently disabled. In fact
*** under DPMI this will not usually be the case, but the DPMI host will
*** maintain virtual interrupt states for each client to ensure that
*** as far as the client is concerned it can enable and disable interrupts
*** and expect the right things to happen.
**/
void RS232_InterruptHandler(void)
{
	int		 i;
	RS232		*port;
	Attributes	*attr;

	for (i = 0; i < MaxRS232; i++)
	{
		port = &(RS232_table[i]);
		if (port->port_base != 0)
		{
			int iir;

			for (iir = _inp(port->port_base + IIR_OFF);
			     (iir & IIR_Pending) == 0;
			     iir = _inp(port->port_base + IIR_OFF))
			{		
				attr	= port->attr;
				switch(iir & IIR_Mask)
				{
					case	IIR_RxStatus :
						RS232_handleRxStatus(port, attr);
						break;
					case	IIR_RxAvail :
						RS232_handleRxAvail(port, attr);
						break;
					case	IIR_TxEmpty :
						RS232_handleTxEmpty(port, attr);
						break;
					case	IIR_ModemStatus :
						RS232_handleModemStatus(port, attr);
						break;
				}
			}
		}
	}

	_outp(Ctrl8259, EndOfInterrupt8259);
}
/*}}}*/
/*}}}*/
/*{{{  send and receive routines */
/*{{{  RS232_send() */
/**
*** The send routine. This is called to initiate a transmission
*** down the serial line. It is guaranteed that no other
*** coroutine will be attempting to send down the serial line
*** at the same time.
**/
word RS232_send(ComsPort *comsport, word amount, UBYTE *buffer)
{
	int	 id;
	RS232	*port;

	id = (int) comsport->id;
	port = &(RS232_table[id]);

	/* Interrupts MUST be disabled while messing about with the	*/
	/* RS232 structure.						*/  
	_disable();

	if (port->outbuf ne (byte *) NULL)         /* Port in use ? */
	{
		_enable();
		ServerDebug("RS232: internal error, multiple sends in progress.");
		return(false);
	}

	/* Set up the transmission buffer and counters.			*/
	port->outmax   = (uint) amount;
	port->outbuf   = buffer;
	port->outcount = 0;

	/* If hardware handshaking is in use then, depending on the	*/
	/* protocol, assert the RequestToSend RTS line.			*/
	if ((port->flags & RS232_UseHardware) && !(rs232_duplex))
	{
		port->current_mcr	|= MCR_RTS;
		_outp(port->port_base + MCR_OFF, port->current_mcr);
	}

	/* If possible send the first byte. Interrupt code will take	*/
	/* over from here on.						*/
	if (port->flags & RS232_ClearToWrite)
		RS232_sendcha(port);

	/* The transmission is now in progress. 			*/
	_enable();
	return(TRUE);
}
/*}}}*/
/*{{{  RS232_pollwrite() */
/**
*** This routine is used to monitor the progress of an RS232 transmission.
*** It will be called regularly by a polling coroutine. There are three
*** possible return values: transmission has finished, transmission is
*** continuing, and transmission has been aborted because of an error.
**/
word RS232_pollwrite(ComsPort *comsport)
{
	int	 id;
	RS232	*port;
	word	 result;
	int	 tmp;

	id	= (int) comsport->id;
	port	= &(RS232_table[id]);

	Debug(Com_Flag, ("RS232 write : written %d of %d bytes, flags is %x", \
			port->outcount, port->outmax, port->flags) );

#if 0
	if (rs232_overrun || rs232_framing || rs232_parity || rs232_break || rs232_overflow)
	{
		ServerDebug("pollwrite: rs232 error, overrun %d, framing %d, parity %d, break %d, overflow %d",
			rs232_overrun, rs232_framing, rs232_parity, rs232_break, rs232_overflow);
		rs232_overrun = rs232_framing = rs232_parity = rs232_break = rs232_overflow = FALSE;
	}
#endif
			
	/* Prevent interrupt routines from messing about with the RS232	*/
	/* structure while the various conditions are being tested.	*/
	_disable();

	/* Cope with the line break condition. The interrupt routine	*/
	/* will have set outmax to the magic -1 value.			*/
	if (port->outmax eq -1)      /* Break occurred */
	{
		/* Clean up the handshake lines if necessary.		*/
		if ((port->flags & RS232_UseHardware) && !(rs232_duplex))
		{
			port->current_mcr &= ~(MCR_RTS | MCR_DTR);
			_outp(port->port_base + MCR_OFF, port->current_mcr);
		}
		result = (word) port->outcount;
	}
 
	/* This handles the end of transmission.			*/
	elif (port->outcount >= port->outmax)
	{
		port->outbuf  = (byte *) NULL;
		port->outcount = 0;
		port->outmax   = 0;
		if ((port->flags & RS232_UseHardware) && !(rs232_duplex))
		{
			port->current_mcr &= ~(MCR_RTS | MCR_DTR);
			_outp(port->port_base + MCR_OFF, port->current_mcr);
		}
		result = -1L;
	}
	/* Otherwise the transmission is continuing.			*/
	else
		result = -2L;		
#if 1
	/* Check for lost CTS interrupt					*/
	tmp	= _inp(port->port_base + MSR_OFF);
	if (tmp & MSR_CTS)
	{
		unless(port->flags & RS232_CTS)
		{
			port->flags |= RS232_CTS;
			if ( (port->flags & RS232_UseHardware) &&
			     (port->flags & RS232_TXEmpty) &&
			    !(port->flags & RS232_ClearToWrite))
			{
				port->flags |= RS232_ClearToWrite;
				RS232_sendcha(port);
			}
		}
	}
	else
		port->flags &= ~RS232_CTS;

	/* Check for lost txempty interrupt as well ?			*/
#endif		
			
		
	_enable(); 
	return(result);
}
/*}}}*/
/*{{{  RS232_abortwrite() */
/**
*** This routine is invoked when a transmission has timed out.
*** Hopefully this will never actually happen as things will
*** get rather confused.
**/
word RS232_abortwrite(ComsPort *comsport)
{
	int		 id;
	RS232		*port;
	unsigned int	 temp;

	id = (int) comsport->id;
	port = &(RS232_table[id]);

	_disable();

	port->outbuf   = (byte *) NULL;
	port->outmax   = 0;
	temp           = port->outcount;
	port->outcount = 0;

	/* Clean up the handshake lines if necessary.		*/
	if ((port->flags & RS232_UseHardware) && !(rs232_duplex))
	{
		port->current_mcr &= ~(MCR_RTS | MCR_DTR);
		_outp(port->port_base + MCR_OFF, port->current_mcr);
	}

	_enable();
  
	return((word) temp);
}
/*}}}*/
/*{{{  RS232_receive() */
/**
*** This routine is called to initiate an RS232 transmission. It is
*** possible that some or all of the data has already been received
*** and is in the overflow buffer. If so the counters are set up
*** such that the first call to pollread() will indicate success.
*** If not all of the data has been received yet then the buffer
*** and counter must be set up. There are also various nasties to
*** cope with XON/XOFF and with hardware flow control.
**/
word RS232_receive(ComsPort *comsport, word amount, UBYTE *buffer)
{
	int	 id;
	RS232	*port;

	id	= (int) comsport->id;
	port	= &(RS232_table[id]);

	_disable();
	if (port->overflow_count > 0)
	{
		/* Check whether all of the amount required is already	*/
		/* in the overflow buffer.				*/
		if (port->overflow_count >= (uint) amount)
		{
			memcpy(buffer, port->overflow_buf, (uint) amount);
			port->overflow_count -= (uint) amount;
			if (port->overflow_count > 0)
				memmove(port->overflow_buf, &(port->overflow_buf[(uint) amount]),
					port->overflow_count);
			/* Amount received == amount required so pollread succeeds	*/
			/* immediately.							*/
		        port->inmax   = (uint) amount;
		        port->incount = (uint) amount;
		}
		/* Otherwise only some of the data has been received.	*/
		else
		{
			memcpy(buffer, port->overflow_buf, port->overflow_count);
			port->inmax		= (uint) amount;
			port->incount		= port->overflow_count;
			port->overflow_count	= 0;
			port->inbuf		= &(buffer[port->incount]);
		}
		/* Since some or all of the overflow buffer has been	*/
		/* cleared out it is necessary to undo any hardware or	*/
		/* software XOFF flow controll.				*/
		if ((port->flags & RS232_UseHardware) && rs232_duplex &&
		    ((port->current_mcr & (MCR_DTR | MCR_RTS)) != (MCR_DTR | MCR_RTS)))
		{
			port->current_mcr	|= (MCR_DTR | MCR_RTS);
			_outp(port->port_base + MCR_OFF, port->current_mcr);
		}
		if (port->flags & RS232_inXOFF)
		{
			port->flags |= RS232_NeedToXon;
			if (port->flags & RS232_ClearToWrite)
				RS232_sendcha(port);
		}

	}
	/* Otherwise no data has been received, the overflow buffer is empty,	*/
	/* so the I/O Server will not have XOFF'ed.				*/
	else
	{
		port->inmax	= (uint) amount;
		port->incount	= 0;
		port->inbuf	= buffer;
	}

	_enable();
	return(true);
}
/*}}}*/
/*{{{  RS232_pollread() */
/**
*** This routine is called by a polling coroutine to monitor the progress of
*** an RS232 reception. It can return one of three condition: reception has
*** completed, reception is continuing, or reception has been aborted because
*** of a line break.
**/
word RS232_pollread(ComsPort *comsport)
{
	int	 id;
	RS232	*port;
	word	 result;
	
	id   = (int) comsport->id;
	port = &(RS232_table[id]);

#if 0
	if (rs232_overrun || rs232_framing || rs232_parity || rs232_break || rs232_overflow)
	{
		ServerDebug("pollread: rs232 error, overrun %d, framing %d, parity %d, break %d, overflow %d",
			rs232_overrun, rs232_framing, rs232_parity, rs232_break, rs232_overflow);
		rs232_overrun = rs232_framing = rs232_parity = rs232_break = rs232_overflow = FALSE;
	}
#endif

	if (debugflags & Com_Flag)
		ServerDebug("RS232 read : read %d of %d bytes, flags is %x",
				port->incount, port->inmax, port->flags);
	_disable();

	/* Cope with line breaks, the interrupt routine will have set inmax	*/
	/* to a magic -1 value.							*/ 
	if (port->inmax == -1)
	{
		result	= (word) port->incount;
	}
	/* Cope with end of reception.						*/
	elif (port->incount >= port->inmax)
	{
		port->inbuf	= (byte *) NULL;
		port->inmax	= 0;
		port->incount	= 0;
		result		= -1L;
	}
	/* Otherwise reception is still in progress.				*/
	else
	{
		result		= -2L;
	}
		

	_enable();
	return(result);
}
/*}}}*/
/*{{{  RS232_abortread() */
/**
*** This routine aborts a reception currently in progress, typically
*** because of a timeout.
**/
word RS232_abortread(ComsPort *comsport)
{
	int	 id;
	RS232	*port;
	word	 temp;

	id   = (int) comsport->id;
	port = &(RS232_table[id]);

	_disable();
	port->inbuf   = (byte *) NULL;
	port->inmax   = 0;
	temp          = (word) port->incount;
	port->incount = 0;
	_enable();
	
	return(temp);
}
/*}}}*/
/*{{{  RS232_done() */
/**
*** This routine is called when a stream coroutine exits, because of
*** a close request or for any other reason. It is responsible for
*** cleaning up:
***
*** 1) if appropriate, generate a line break.
*** 2) clear out the various event ports.
*** 3) clear out any flags which are likely to have been specific
***    to the current connection.
**/
void RS232_done(ComsPort *comsport)
{
	RS232		*port;
	int		 id;

	id   = (int) comsport->id;
	port = &(RS232_table[id]);

	_disable();
	port->breakint	 = NullPort;
	port->modemint	 = NullPort;
	port->flags	&= ~(RS232_inXOFF | RS232_outXOFF | RS232_BreakDetected |
				RS232_NeedToXoff | RS232_NeedToXon);
	_enable();

	/* If the HUPCL attribute is enabled, cause a line break.		*/
	if (IsAnAttribute(port->attr, RS232_HupCl))
	{
		port->current_lcr	|= LCR_SetBreak;
		_outp(port->port_base + LCR_OFF, port->current_lcr);
		goto_sleep(250000L);
		port->current_lcr	&= ~LCR_SetBreak;
		_outp(port->port_base + LCR_OFF, port->current_lcr);
	}		
}
/*}}}*/
/*{{{  RS232_disable_events() */
void RS232_disable_events(ComsPort *comsport)
{
	RS232	*port;
	int	 id;

	id   = (int) comsport->id;
	port = &(RS232_table[id]);

	_disable();
	port->breakint = NullPort;
	port->modemint = NullPort;
	port->flags &= ~(RS232_BreakDetected | RS232_RingDetected);
	_enable();
}
/*}}}*/
/*{{{  RS232_enable_events() */
void RS232_enable_events(ComsPort *comsport, word mask, word event_port)
{
	RS232	*port;
	int	 id;

	id   = (int) comsport->id;
	port = &(RS232_table[id]);

	_disable();
	if (mask & Event_SerialBreak)
	{
		port->breakint = event_port;
		port->flags   &= ~RS232_BreakDetected;
	}
	else
	{
		port->modemint	 = event_port;
		port->flags	&= ~RS232_RingDetected;
	}
	_enable();
}
/*}}}*/
/*{{{  RS232_check_events() */
/**
*** If a line break or a modem ring is detected by the interrupt
*** routine then it will set a flag in the appropriate RS232
*** structure and set the RS232_errno flag. The errno flag is
*** checked by the I/O Server's main polling loop and it will
*** call this routine.
**/
void RS232_check_events()
{
	int		 i;
	RS232		*port;
	extern void	RS232_send_event(Port port, word event);

	for (i=0; i < MaxRS232; i++)
		if (RS232_table[i].port_base != 0)
	{
		port = &(RS232_table[i]);
		if ((port->flags & RS232_BreakDetected) && (port->breakint ne NullPort))
			RS232_send_event(port->breakint, Event_SerialBreak);
		if ((port->flags & RS232_RingDetected) && (port->modemint ne NullPort))
			RS232_send_event(port->modemint, Event_ModemRing);
		_disable();
		port->flags &= ~(RS232_BreakDetected + RS232_RingDetected);
		_enable();
	}
}
/*}}}*/
/*}}}*/
/*{{{  configuration */
/**
*** This routine is called during initialisation and whenever a Helios
*** application needs to change the RS232 port configuration. It is
*** somewhat convoluted... The various attributes are examined and
*** the appropriate 8250 register values are determined. Also various
*** relevant flags in the RS232 structure are updated. The new
*** register values are not actually updated until the end of the
*** routine.
**/
void RS232_configure(ComsPort *comsport)
{
	RS232		*port;
	int		 id;
	int		 LCR = 0, baud_high = 0, baud_low = 0, do_break = 0;
	Attributes	*attr;

	id		= (int) comsport->id;
	port		= &(RS232_table[id]);

	attr		= &(comsport->attr);
	port->attr	= attr;

	switch((int) (GetInputSpeed(attr)))
	{
		case RS232_B50    : baud_high = 0x09; break;
		case RS232_B75    : baud_high = 0x06; break;
		case RS232_B110   : baud_high = 0x04; baud_low = 0x17; break;
		case RS232_B134   : baud_high = 0x03; baud_low = 0x59; break;
		case RS232_B150   : baud_high = 0x03; break;
		case RS232_B200   : baud_high = 0x02; baud_low = 0x0040; break;
		case RS232_B300   : baud_high = 0x01; baud_low = 0x0080; break;
		case RS232_B600   : baud_low = 0x00c0; break;
		case RS232_B1200  : baud_low = 0x0060; break;
		case RS232_B1800  : baud_low = 0x0040; break;
		case RS232_B2400  : baud_low = 0x0030; break;
		case RS232_B4800  : baud_low = 0x0018; break;
		case RS232_B19200 : baud_low = 0x0006; break;
		case RS232_B38400 : baud_low = 0x0003; break;
		case RS232_B56000 : baud_low = 0x0002; break;

		/* B0 means generate a line break. Unfortunately I do not	*/
		/* know the old baud rate so I default back to 9600.		*/
		case RS232_B0     : do_break++;

		/* Any unrecognised baud rate is converted to 9600.		*/
		case RS232_B9600  :
		default           : baud_low = 0x000C;
                         SetInputSpeed(attr, (word) RS232_B9600);
                         break;
	}

	if (IsAnAttribute(attr, RS232_Csize_5))
		LCR |= LCR_WordSelect5Bits;
	elif (IsAnAttribute(attr, RS232_Csize_6))
		LCR |= LCR_WordSelect6Bits;
	elif (IsAnAttribute(attr, RS232_Csize_7))
		LCR |= LCR_WordSelect7Bits;
	else
		LCR |= LCR_WordSelect8Bits;

	if (IsAnAttribute(attr, RS232_Cstopb))
		LCR |= LCR_2StopBits;

	if (IsAnAttribute(attr, RS232_ParEnb))
		LCR |= LCR_ParityEnable;

	if (!IsAnAttribute(attr, RS232_ParOdd))
		LCR |= LCR_EvenParity;

	/* The rest of the code can interact in nasty ways with the	*/
	/* interrupt routine. Therefore interrupts have to be disabled	*/
	/* for the time being.						*/
	_disable();

	/* Block until the transmitter and transmitter holder register	*/
	/* are empty. Otherwise we risk corrupting outgoing data that	*/
	/* has not managed to flush yet.				*/
	while ((_inp(port->port_base + LSR_OFF) & (LSR_TxEmpty | LSR_ThEmpty)) != (LSR_TxEmpty | LSR_ThEmpty))
		;	

	/* Now check the hardware handshaking protocol. If this		*/
	/* should be used then, depending on simplex vs duplex protocol,*/
	/* some of the lines should be asserted. This code assumes that	*/
	/* there are no transmissions or receptions in progress at the	*/
	/* moment.							*/
	if (IsAnAttribute(attr, RS232_CLocal))
	{
		port->flags	&= ~RS232_UseHardware;
		if (port->flags & RS232_TXEmpty)
			port->flags |= RS232_ClearToWrite;
		if (port->current_mcr & (MCR_RTS | MCR_DTR))
		{
			port->current_mcr &= ~(MCR_RTS | MCR_DTR);
			_outp(port->port_base + MCR_OFF, port->current_mcr);
		}
	}
	else
	{
		port->flags	|= RS232_UseHardware;
		if (rs232_duplex && (port->overflow_count < overflow_max) &&
		   ((port->current_mcr & (MCR_RTS | MCR_DTR)) != (MCR_RTS | MCR_DTR)))
		{
			port->current_mcr |= (MCR_RTS | MCR_DTR);
			_outp(port->port_base + MCR_OFF, port->current_mcr);
		}
		if ((port->flags & RS232_CTS) && (port->flags & RS232_TXEmpty))
			port->flags |= RS232_ClearToWrite;
		else
			port->flags &= ~RS232_ClearToWrite;
	}

	/* XON/XOFF flow control. If this is being disabled then I need to	*/
	/* clear out any flags indicating that the I/O Server has xoff'ed or	*/
	/* has been xoff'ed.							*/
	if (!IsAnAttribute(attr, RS232_IXOFF))
		port->flags &= ~RS232_inXOFF;
	if (!IsAnAttribute(attr, RS232_IXON))
		port->flags &= ~RS232_outXOFF;
		

	/* Set up the baud rate and the new LCR.				*/
	port->current_lcr	= LCR;
	_outp(port->port_base + LCR_OFF, LCR | LCR_DLAB);
	_outp(port->port_base + BAUDL_OFF, baud_low);
	_outp(port->port_base + BAUDH_OFF, baud_high);
	_outp(port->port_base + LCR_OFF, LCR);

	if (port->flags & RS232_ClearToWrite)
		RS232_sendcha(port);

	/* If there is no current rx, clear out the overflow buffer. This is	*/
	/* in case junk characters were received because of wrong baud rate	*/
	/* or any similar reason.						*/
	if (port->inbuf == NULL)
		port->overflow_count	= 0;
		
	/* Everything should be set up nicely now.				*/
	_enable();

	/* If the baud rate was RS232_B0, generate a line break.		*/
	if (do_break)
	{
		_outp(port->port_base + LCR_OFF, LCR | LCR_SetBreak);
		goto_sleep(250000L);
		_outp(port->port_base + LCR_OFF, LCR);
	}

	rs232_overrun	= FALSE;
	rs232_framing	= FALSE;
	rs232_parity	= FALSE;
	rs232_break	= FALSE;
	rs232_overflow	= FALSE;
}

/*}}}*/
/*{{{  initialisation and tidying */
/*{{{  find_port_base() */
/**
*** Given a coms port number return the base address, or 0 for error.
**/
static int find_port_base(int port)
{
	char	buf[16];
	word	value;
	
	if (port eq 0)
		return(0x03f8);
	elif (port eq 1)
		return(0x02f8);

	strcpy(buf, "com0_base");
	buf[3]	= port + '1';
	value	= get_int_config(buf);
	if (value eq Invalid_config)
		return(0);
	else
		return((int) value);
}
/*}}}*/
/*{{{  RS232_enable_interrupts() and disable_interrupts() */
/**
*** Interrupt management. The RS232 ports are normally hooked up to vectors
*** 0x0C for com1 and 0x0B for com2. Under DOS standard DOS calls can be used
*** to save the current vectors and install new ones. Under Windows it is
*** necessary to use DPMI.
**/

/*{{{  my_dos_setvect() */
/**
*** There appear to be problems with the Microsoft version of
*** dos_setvect(), involving the __interrupt attribute.
**/
PRIVATE void my_dos_setvect(int x, VoidFnPtr f)
{
	union	_REGS	 regs;
	struct	_SREGS	 sregs;
	byte		*tempptr	= (byte *) f;

	regs.h.ah		= 0x25;
	regs.h.al		= x;
	regs.x.dx		= FP_OFF(tempptr);
	_segread(&sregs);
	sregs.ds		= FP_SEG(tempptr);
	_int86x(0x21, &regs, &regs, &sregs);
}
/*}}}*/

PRIVATE int	ints_used	= 0;
extern	void	RS232_interrupt(void);

#if !(MSWINDOWS)
PRIVATE VoidFnPtr	com1_save, com2_save;

PRIVATE void RS232_enable_interrupts(int int_nos)
{
	ints_used	= int_nos;

	if (int_nos & 0x01)
	{
		com1_save	= (VoidFnPtr) _dos_getvect(0x0C);
		my_dos_setvect(0x0C, func(RS232_interrupt));
	}
	if (int_nos & 0x02)
	{
		com2_save	= (VoidFnPtr) _dos_getvect(0x0B);
		my_dos_setvect(0x0B, func(RS232_interrupt));
	}
}

PRIVATE void RS232_disable_interrupts(void)
{
	if (ints_used & 0x01)
		my_dos_setvect(0x0C, com1_save);
	if (ints_used & 0x02)
		my_dos_setvect(0x0B, com2_save);
}
#else
PRIVATE int	com1_saveh = 0, com1_savel = 0;
PRIVATE int	com2_saveh = 0, com2_savel = 0;
PRIVATE int	callbackh  = 0, callbackl  = 0;
PRIVATE UWORD	dMem		= 0L;

PRIVATE void RS232_enable_interrupts(int int_nos)
{
	union	_REGS	 regs;
	struct	_SREGS	 sregs;
	byte		*tempptr;

	dMem		= GlobalDosAlloc(0x32);
	if (dMem == 0)
	{
		ServerDebug("RS232_enable_interrupts: failed to allocate DOS memory");
		longjmp(exit_jmpbuf, 1);
	}

	_segread(&sregs);
	regs.x.ax		= 0x303;		/* Alloc real mode callback	*/
	tempptr			= (byte *) &RS232_interrupt;
	sregs.ds		= FP_SEG(tempptr);	/* ds:si == function		*/
	regs.x.si		= FP_OFF(tempptr);
	sregs.es		= LOWORD(dMem);		/* es:di == DOS buffer		*/
	regs.x.di		= 0;
	_int86x( 0x31,&regs, &regs, &sregs);
	if (regs.x.cflag)
	{
		ServerDebug("RS232_enable_interrupts: failed to allocate DPMI callback");
		longjmp(exit_jmpbuf, 1);
	}
	callbackh	= regs.x.cx;
	callbackl	= regs.x.dx;
	
	if (int_nos & 0x01)
	{
		_segread(&sregs);
		regs.x.ax		= 0x200;	/* Get real mode interrupt vector	*/
		regs.h.bl		= 0x0C;		/* for com1.				*/
		_int86x(0x31, &regs, &regs, &sregs);
		com1_saveh		= regs.x.cx;
		com1_savel		= regs.x.dx;

		_segread(&sregs);
		regs.x.ax		= 0x201;	/* Set real mode interrupt vector	*/
		regs.h.bl		= 0x0C;
		regs.x.cx		= callbackh;
		regs.x.dx		= callbackl;
		_int86x(0x31, &regs, &regs, &sregs);
	}
	if (int_nos & 0x02)
	{
		_segread(&sregs);
		regs.x.ax		= 0x200;	/* Get real mode interrupt vector	*/
		regs.h.bl		= 0x0B;		/* for com2.				*/
		_int86x(0x31, &regs, &regs, &sregs);
		com2_saveh		= regs.x.cx;
		com2_savel		= regs.x.dx;

		_segread(&sregs);
		regs.x.ax		= 0x201;	/* Set real mode interrupt vector	*/
		regs.h.bl		= 0x0B;
		regs.x.cx		= callbackh;
		regs.x.dx		= callbackl;
		_int86x(0x31, &regs, &regs, &sregs);
	}

	ints_used	= int_nos;
}

PRIVATE void RS232_disable_interrupts(void)
{
	union	_REGS	 regs;
	struct	_SREGS	 sregs;

	if (ints_used & 0x01)
	{
		_segread(&sregs);
		regs.x.ax		= 0x201;	/* Set real mode interrupt vector	*/
		regs.h.bl		= 0x0C;
		regs.x.cx		= com1_saveh;
		regs.x.dx		= com1_savel;
		_int86x(0x31, &regs, &regs, &sregs);
	}
	if (ints_used & 0x02)
	{
		_segread(&sregs);
		regs.x.ax		= 0x201;	/* Set real mode interrupt vector	*/
		regs.h.bl		= 0x0B;
		regs.x.cx		= com2_saveh;
		regs.x.dx		= com2_savel;
		_int86x(0x31, &regs, &regs, &sregs);
	}

	if ((callbackh != 0) || (callbackl != 0))
	{
		_segread(&sregs);
		regs.x.cx	= callbackh;
		regs.x.dx	= callbackl;
		_int86x( 0x31, &regs, &regs, &sregs);
	}

	if (dMem != 0L)
		GlobalDosFree(LOWORD(dMem));
}
#endif
/*}}}*/
/*{{{  RS232_initialise_hardware() */
/**
*** RS232_initialise_hardware() is called during the initialise_devices()
*** phase. The host.con file has been read in. This code is responsible
*** for setting up all the data structures etc.
**/
static void RS232_initialise_hardware()
{
	int	 i;
	char	*config_option;
	word	 word_option;
	int	 port_number;
	int	 number_ports;

	RS232_table	= (RS232 *) malloc(MaxRS232 * 	sizeof(RS232));
	RS232_coms	= (ComsPort *) malloc((MaxRS232 + 2) * sizeof(ComsPort));
	if ((RS232_table == NULL) || (RS232_coms == NULL))
	{
		ServerDebug("Out of memory initialising RS232 hardware");
		longjmp(exit_jmpbuf, 1);
	}
		/* Start with the safe initialisations.			*/
	for (i = 0; i < MaxRS232; i++)
	{
		RS232_table[i].flags		= RS232_TXEmpty;
		RS232_table[i].incount		= 0;
		RS232_table[i].inmax		= 0;
		RS232_table[i].inbuf		= NULL;
		RS232_table[i].outcount		= 0;
		RS232_table[i].outmax		= 0;
		RS232_table[i].outbuf		= NULL;
		RS232_table[i].attr		= &(RS232_coms[i].attr);
		RS232_table[i].breakint		= NullPort;
		RS232_table[i].modemint		= NullPort;
		RS232_table[i].overflow_count	= 0;
		RS232_table[i].id		= i;
		RS232_table[i].port_base	= 0;
		RS232_table[i].current_mcr	= 0;
		RS232_table[i].current_lcr	= 0;
	}

	for (i = 0; i <= VY86PID_Port; i++)
	{
		RS232_coms[i].send_fn		= func(RS232_send);
		RS232_coms[i].pollwrite_fn	= func(RS232_pollwrite);
		RS232_coms[i].abortwrite_fn	= func(RS232_abortwrite);
		RS232_coms[i].receive_fn	= func(RS232_receive);
		RS232_coms[i].pollread_fn	= func(RS232_pollread);
		RS232_coms[i].abortread_fn	= func(RS232_abortread);
		RS232_coms[i].configure_fn	= func(RS232_configure);
		RS232_coms[i].done_fn		= func(RS232_done);
		RS232_coms[i].error_fn		= func(RS232_error_handler);
		InitAttributes(&(RS232_coms[i].attr));
		SetInputSpeed(&(RS232_coms[i].attr), (word) RS232_B9600);
		SetOutputSpeed(&(RS232_coms[i].attr), (word) RS232_B9600);
		AddAttribute(&(RS232_coms[i].attr),   RS232_Csize_8);
		AddAttribute(&(RS232_coms[i].attr),   RS232_IgnPar);
		AddAttribute(&(RS232_coms[i].attr),   RS232_IXON);
		AddAttribute(&(RS232_coms[i].attr),   RS232_IXOFF);
		AddAttribute(&(RS232_coms[i].attr),   RS232_CLocal);
		AddAttribute(&(RS232_coms[i].attr),   RS232_BreakInterrupt);
		InitSemaphore(&(RS232_coms[i].lock), 1);
		RS232_coms[i].id		= -1;
	}

		/* Now work out which ports are actually used. This depends	*/
		/* on the rs232_ports entry in host.con, and on whether or not	*/
		/* the vy86pid board is being used.				*/
	config_option	= get_config("box");
	if (!mystrcmp(config_option, "vy86pid"))
	{
		config_option	= get_config("vy86pid_port");
		if (config_option eq NULL)
			port_number = 0;
		else
		{
			if (!mystrcmp(config_option, "com1"))
				port_number = 0;
			elif (!mystrcmp(config_option, "com2"))
				port_number = 1;
			else
			{
				ServerDebug("Invalid host.con option for vy86pid_port, should be com1 or com2.");
				longjmp(exit_jmpbuf, 1);
			}
		}

		RS232_coms[VY86PID_Port].id		= port_number;
		RS232_table[port_number].port_base	= find_port_base(port_number);
	}

		/* Now check the rs232_ports entry					*/
	config_option = get_config("rs232_ports");
	if (config_option ne NULL)
	{
		number_ports = 0;
		for (i = 0; ; config_option++)
		{
			if (*config_option eq '\0')
				break;
			elif (isspace(*config_option) || (*config_option eq ','))
				continue;
			elif ((*config_option < '1') || (*config_option > '7'))
			{
				ServerDebug("Illegal entry in host.con file for rs232_ports");
				longjmp(exit_jmpbuf, 1);
			}
			else
				port_number = *config_option - '1';

			if (RS232_table[port_number].port_base != 0)
			{
				ServerDebug("Error in host.con file, multiple use of rs232 port com%d", port_number + 1);
				longjmp(exit_jmpbuf, 1);
			}
			
			RS232_table[port_number].port_base = find_port_base(port_number);
			if (RS232_table[port_number].port_base == 0)
			{
				ServerDebug("Error, missing host.con option for com%d_base", port_number + 1);
				longjmp(exit_jmpbuf, 1);
			}

			RS232_coms[port_number].id	= port_number;
			number_ports++;
		}

		if (number_ports eq 0)
		{
			ServerDebug("Error in host.con entry for rs232_ports, no ports have been specified.");
			longjmp(exit_jmpbuf, 1);
		}
		
		/* Now work out the /rs232/default port.			*/	
		config_option = get_config("default_rs232");
		if (config_option ne NULL)
		{
			if ((strncmp(config_option, "com", 3)) ||
			    ((config_option[3] < '1') || (config_option[3] > '7')))
			{
				ServerDebug("Error, invalid host.con entry for default_rs232");
				longjmp(exit_jmpbuf, 1);
			}
			port_number = config_option[3] - '1';

			if (RS232_coms[port_number].id == -1)
			{
				ServerDebug("Error, default_rs232 port %s has not been enabled", config_option);
				longjmp(exit_jmpbuf, 1);
			}

			RS232_coms[Default_Port].id	= port_number;

			/* If there is only one port then provide default only	*/
			if (number_ports == 1)
				RS232_coms[port_number].id	= -1;
		}
			/* If there is only one port it must be the default	*/
		elif (number_ports == 1)
		{
			for (i = 0; RS232_coms[i].id == -1; i++)
			;
			RS232_coms[Default_Port].id	= i;
			RS232_coms[i].id		= -1;
		}
		else
		{
			/* Multiple ports, pick the first one.				*/
			for (i = 0; RS232_coms[i].id == -1; i++)
			;
			RS232_coms[Default_Port].id	= i;
		}
	}

		/* After all that work, are any of the ports in use ?		*/
	number_ports = 0;
	for (i = 0; i < MaxRS232; i++)
		if (RS232_table[i].port_base != 0)
			number_ports++;

	if (number_ports == 0)
		return;
		
		/* For all the ports in use, save the current register values.	*/
		/* Also check the current CTS line.				*/
	for (i = 0; i < MaxRS232; i++)
		if (RS232_table[i].port_base != 0)
		{
			int	MSR;
			
			RS232_table[i].LCR	= _inp(RS232_table[i].port_base + LCR_OFF);
			_outp(RS232_table[i].port_base + LCR_OFF, RS232_table[i].LCR | LCR_DLAB);
			RS232_table[i].BAUDH	= _inp(RS232_table[i].port_base + BAUDH_OFF);
			RS232_table[i].BAUDL	= _inp(RS232_table[i].port_base + BAUDL_OFF);
			_outp(RS232_table[i].port_base + LCR_OFF, RS232_table[i].LCR);
			RS232_table[i].IER	= _inp(RS232_table[i].port_base + IER_OFF);
			RS232_table[i].MCR	= _inp(RS232_table[i].port_base + MCR_OFF);

			RS232_table[i].current_lcr = RS232_table[i].LCR;
			RS232_table[i].current_mcr = RS232_table[i].MCR | MCR_OUT2;
			
			MSR			= _inp(RS232_table[i].port_base + MSR_OFF);
			if (MSR & MSR_CTS)
				RS232_table[i].flags |= RS232_CTS;
		}			
		
		/* Sort out the interrupts options.				*/
	word_option = get_int_config("RS232_INTERRUPT");
	if (word_option eq 1L)
		RS232_enable_interrupts(1);
	elif (word_option eq 2L)
		RS232_enable_interrupts(2);
	else
		RS232_enable_interrupts(3);

		/* Now enable interrupts within all the 8250 chips.		*/
	for (i = 0; i < MaxRS232; i++)
		if (RS232_table[i].port_base != 0)
		{
			/* Make sure that OUT2 is asserted because that goes to the	*/
			/* interrupt controller.					*/
			_outp(RS232_table[i].port_base + MCR_OFF, RS232_table[i].current_mcr);

			/* Then the interrupt enable register itself.			*/
			_outp(RS232_table[i].port_base + IER_OFF, IER_AllInterrupts);
		}
		
		/* And enable interrupts within the 8259			*/
	save_8259	= _inp(Mask8259);
	_outp(Mask8259, save_8259 & 0x00E7);
	_outp(Ctrl8259, EndOfInterrupt8259);
}
/*}}}*/
/*{{{  RS232_control_fifo() */

void RS232_control_fifo(ComsPort *comsport, bool on)
{
	int	id;
	RS232	*port;

	id	= (int) comsport->id;
	port	= &(RS232_table[id]);
	
			/* 16550 compatibility - If a PC contains a 16550 	*/
			/* serial chip rather than an old 8250, then we can 	*/
			/* make use of its fifo buffering to enable higher 	*/
			/* reliability at high speeds. We do this by writing 	*/
			/* to the fifo control register (FCR). In the 8250 	*/
			/* this at the same position as the read only IIR 	*/
			/* register. The operation should therefore be 		*/
			/* compatible. Set 16550 fifo mode on with a trigger 	*/
			/* level of 8 in the 16 byte fifo. The fifo is used for */
			/* both Tx/Rx. 						*/
	if (on)
		_outp(port->port_base + IIR_OFF, 0x81);
	else
		_outp(port->port_base + IIR_OFF, 0);
}

/*}}}*/
/*{{{  RS232_tidy() */
/**
*** RS232_Tidy(), if there are any rs232 ports then disable the interrupts,
*** and restore the chips to their original state.
**/
static void RS232_tidy()
{
	int	i;

	if ((RS232_table == NULL) || (RS232_coms == NULL))
		return;
		
	for (i = 0; i < MaxRS232; i++)
		if (RS232_table[i].port_base != 0)
			break;
	if (i >= MaxRS232) return;	/* no ports in use	*/

	RS232_disable_interrupts();

	/* Clear out the 8259 interrupt controller.		*/
	_outp(Mask8259, save_8259);
	
	for (i = 0; i < MaxRS232; i++)
	{
		_outp(RS232_table[i].port_base + LCR_OFF, RS232_table[i].LCR + LCR_DLAB);
		_outp(RS232_table[i].port_base + BAUDH_OFF, RS232_table[i].BAUDH);
		_outp(RS232_table[i].port_base + BAUDL_OFF, RS232_table[i].BAUDL);
		_outp(RS232_table[i].port_base + LCR_OFF, RS232_table[i].LCR);
		_outp(RS232_table[i].port_base + IER_OFF, RS232_table[i].IER);
		_outp(RS232_table[i].port_base + MCR_OFF, RS232_table[i].MCR);
	}

	free(RS232_table);
	free(RS232_coms);
}
/*}}}*/
/*{{{  RS232_testfun() */
/**
*** RS232_Testfun() is called when the Server initialises itself, and has to
*** check whether there are any serial ports attached. If not, the Server will
*** not create a device server /rs232. All the real initialisation has been
*** done already. Furthermore, if any of the coms ports are accessible to
*** Helios then there will be a default port.
**/
void RS232_Testfun(a_ptr)
bool *a_ptr;
{
	if (RS232_coms[Default_Port].id ne -1)
		*a_ptr = true;
	else
		*a_ptr = false;
}
/*}}}*/
/*{{{  RS232_initlist() */
/**
*** RS232_initlist() sets up the linked list of RS232 ports (com1, com2 etc).
*** All the hard work has been done already, so it is just a case of
*** walking down the table of possible ports.
**/
word RS232_initlist(list, port)
List *list;
ComsPort **port;
{
	int	i;
	int	number_ports;
	char	buf[8];
	
	number_ports	= 0;
	
	for (i = 0; i < MaxRS232; i++)
		if (RS232_coms[i].id ne -1L)
		{
			number_ports++;
			strcpy(buf, "com0");
			buf[3] = i + '1';
			add_port_node(list, buf, &(RS232_coms[i]));
		}
	*port = &(RS232_coms[Default_Port]);
	return(number_ports);
}
/*}}}*/
/*}}}*/
#endif /* RS232_supported */
/*}}}*/
/*{{{  Centronics */
/**
*** The Centronics device - this is supported by polling, using the bios
*** printer routines. Note that the Centronics device cannot be read. On the
*** PC there can be upto three Centronics ports, the exact number being
*** obtained via the equiplist bios call.
**/

#if Centronics_supported
#define MaxCentronics 3
PRIVATE UBYTE *Centronics_write_buffer[3];
PRIVATE word  Centronics_written[3], Centronics_towrite[3];
PRIVATE void  analyse_Centronics_error(unsigned int result);
PRIVATE word  Centronics_send(ComsPort *, word, UBYTE *);
PRIVATE word  Centronics_pollwrite(ComsPort *);
PRIVATE word  Centronics_abortwrite(ComsPort *);
PRIVATE word  Centronics_fail(void);
extern  void  Centronics_error_handler(void);
PRIVATE ComsPort Centronics_coms[MaxCentronics];
#define Centronics_error 0x0025
#define Centronics_ready 0x0080

/**
*** Centronics_Testfun() is similar to RS232_Testfun() above.
**/

void Centronics_Testfun(a_ptr)
bool *a_ptr;
{ 
  int ports = _bios_equiplist();
  ports = (ports >> 14) & 0x03;
  if (ports > 0) 
    *a_ptr = true;
  else  
    *a_ptr = false;
}


word Centronics_initlist(list, port)
List *list;
ComsPort **port;
{ int  ports, i;
  char name[5], *config;

  ports = _bios_equiplist();
  ports = (ports >> 14) & 0x03;    /* This gives the number of parallel ports */

  for (i = 0; i < MaxCentronics; i++) Centronics_coms[i].id = -1L;

  for (i = 0; i < ports; i++)
   { Centronics_coms[i].send_fn       = func(Centronics_send);
     Centronics_coms[i].abortwrite_fn = func(Centronics_abortwrite);
     Centronics_coms[i].pollwrite_fn  = func(Centronics_pollwrite);
     Centronics_coms[i].receive_fn    = func(Centronics_fail);
     Centronics_coms[i].configure_fn  = func(Ignore);
     Centronics_coms[i].done_fn       = func(Ignore);
     Centronics_coms[i].error_fn      = func(Centronics_error_handler);
     Centronics_coms[i].id            = (word) i;
   }
    
  if (ports eq 1)                    /* Treat as special case          */
   { *port = &(Centronics_coms[0]); /* default is the only entry      */
     return(0L);                     /* And LPT1 is the default        */
   }

  for (i = 0; i < ports; i++)      /* sort out each port */
   { strcpy(name, "lpt0");
     name[3] = '0' + i + 1;
     add_port_node(list, name, &(Centronics_coms[i]));
     Centronics_write_buffer[i-1] = (UBYTE *) NULL;
   }

  config = get_config("Default_Centronics");
  if (config ne (char *) NULL)
    { if ((*config ne 'l') && (*config ne 'L'))   /* Check entry is LPTx */
       goto error;
      config++;
      if ((*config ne 'p') && (*config ne 'P'))
       goto error;
      config++;
      if ((*config ne 't') && (*config ne 'T'))
       goto error;
      i =  (*(++config) - '0');
      if ((i < 1) || (i > ports) )
       goto error;
      else
       *port = &(Centronics_coms[i]);
    }
  else
   *port = &(Centronics_coms[0]);

  return((word) ports);             /* Everything succeeded */

error:                              /* The configuration entry is faulty */
  ServerDebug("*** Warning : invalid entry in configuration file for Default_Centronics.");
  ServerDebug("*** Valid entries are LPT%d to LPT%d.", 1, ports);
  longjmp(exit_jmpbuf, 1);
}

PRIVATE word Centronics_fail()
{ return(false);
}

PRIVATE word Centronics_send(port, amount, buffer)
ComsPort *port;
word  amount;
UBYTE *buffer;
{ int id;

  id = (int) port->id;
  if (Centronics_write_buffer[id] ne (UBYTE *) NULL)
   return(false);
  Centronics_write_buffer[id] = buffer;
  Centronics_written[id]      = 0L;
  Centronics_towrite[id]      = amount;
  Centronics_errno            = 0L;
  return(true);
}

PRIVATE word Centronics_pollwrite(port)
ComsPort *port;
{ int id;
  unsigned int result;

  id = (int) port->id;

  for ( ; Centronics_written[id] < Centronics_towrite[id] ;)
    { result = _bios_printer(_PRINTER_STATUS, id, 0);
      if (result & Centronics_error)
       { analyse_Centronics_error(result);
         Centronics_write_buffer[id] = (UBYTE *) NULL;
         return(Centronics_written[id]);
       }
      if (result & Centronics_ready)
       { result = _bios_printer(_PRINTER_WRITE, id,
                                *(Centronics_write_buffer[id])++);
         if (result & Centronics_error)
          { analyse_Centronics_error(result);
            Centronics_write_buffer[id] = (UBYTE *) NULL;
            return(Centronics_written[id]);
          }
         Centronics_written[id]++;
       }
      else
       return(-2L);                /* Cannot send so return to loop */
    }
                                   /* All written */
  Centronics_write_buffer[id] = (UBYTE *) NULL;
  return(-1L);                      /* Indicate success */  
}

PRIVATE word Centronics_abortwrite(port)
ComsPort *port;
{ int id;

  id = (int) port->id;

  Centronics_write_buffer[id] = (UBYTE *) NULL;
  return(Centronics_written[id]);
}

PRIVATE void analyse_Centronics_error(result)
unsigned int result;
{
  if (result & 0x0010)
    Centronics_errno = printer_offline;
  elif (result & 0x0020)
    Centronics_errno = printer_outofpaper;
  elif (result & 0x0008)
    Centronics_errno = printer_error;
  else
    Centronics_errno = printer_invalid;
}

#endif  /* Centronics_supported */
/*}}}*/
/*{{{  printers */
/**
***	The printers device support routines.
**/

#if Printer_supported


void Printer_Testfun(a_ptr)
bool *a_ptr;
{ int ports, i;
  ports = 0;

  for (i = 0; i < MaxRS232; i++)
   if (RS232_coms[i].id ne -1L) ports++;
  for (i = 0; i < MaxCentronics; i++)
   if (Centronics_coms[i].id ne -1L) ports++;

  if (ports > 0)
   *a_ptr = true;
  else
   *a_ptr = false;
}


word Printer_initlist(list, port)
List *list;
ComsPort **port;
{ int ports, i; 
  char *config;
  char name[8];

  ports = 0;   /* how many printer ports are there ? */
  for (i = 0; i < MaxRS232; i++)
   if (RS232_coms[i].id ne -1L) ports++;
  for (i = 0; i < MaxCentronics; i++)
   if (Centronics_coms[i].id ne -1L) ports++;

  if (ports <= 1)  /* there is only one, so just fill in default */
   { for (i = 0; i < MaxRS232; i++)
      if (RS232_coms[i].id ne -1L)
       { *port = &(RS232_coms[i]); return(0L); }
     for (i = 0; i < MaxCentronics; i++)
      if (Centronics_coms[i].id ne -1L)
       { *port = &(Centronics_coms[i]); return(0L); }
   }

         /* there are multiple printers, put all of them into the list */
  for (i = 0; i < MaxRS232; i++)
   if (RS232_coms[i].id ne -1L)
    { strcpy(name, "com0");
      name[3] = '0' + i + 1;
      add_port_node(list, name, &(RS232_coms[i]));
    }
  for (i = 0; i < MaxCentronics; i++)
   if (Centronics_coms[i].id ne -1L)
    { strcpy(name, "lpt0");
      name[3] = '0' + i + 1;
      add_port_node(list, name, &(Centronics_coms[i]));
    }

  config = get_config("default_printer");
  if (config eq NULL)  /* none specified, first parallel or serial will do */
   { for (i = 0; i < MaxCentronics; i++)
      if (Centronics_coms[i].id ne -1L)
       { *port = &(Centronics_coms[i]); return((word) ports); }
     for (i = 0; i < MaxRS232; i++)
      if (RS232_coms[i].id ne -1L)
       { *port = &(RS232_coms[i]); return((word) ports); }
   }

  if (strlen(config) ne 4)
      goto error;
  if ( ((config[0] eq 'C') || (config[0] eq 'c')) &&
       ((config[1] eq 'O') || (config[1] eq 'o')) &&
       ((config[2] eq 'M') || (config[2] eq 'm')))
   { i = config[3] - '0';
     if ((i < 1) || (i > MaxRS232))
      goto error;
     if (RS232_coms[i-1].id eq -1L)
      goto error;
     *port = &(RS232_coms[i-1]);
   }
  elif ( ((config[0] eq 'L') || (config[0] eq 'l')) &&
         ((config[1] eq 'P') || (config[1] eq 'p')) &&
         ((config[2] eq 'T') || (config[2] eq 't')) )
    { i = config[3] - '0';
      if ((i < 1) || (i > MaxCentronics))
       goto error;
      if (Centronics_coms[i-1].id eq -1L)
       goto error;
      *port = &(Centronics_coms[i-1]);
    }
  else goto error;
 
  return((word) ports);

error:
  ServerDebug("*** Warning : invalid entry in configuration file for Default_Printer.");
  ServerDebug("*** Valid entries are LPT1 to LPT3 or COM1 to COM7.");          
  longjmp(exit_jmpbuf, 1);
}

#endif /* Printer_supported */
/*}}}*/
/*{{{  mouse */
/**
*** The local mouse routines.
**/

#if mouse_supported

extern  void new_mouse(int x, int y, word buttons);  /* in devices.c */

extern int  reset_mouse(void);      /* assembler routines */
extern void enable_mouse(void);
extern void disable_mouse(void);
extern void set_mouse_resolution(int res);
extern int Mouse_Relative;
 
PRIVATE int  mouse_loaded = 0;
PRIVATE int  mouse_active = 0;
PRIVATE int  mouse_resolution, mouse_divisor;
PRIVATE int actual_x = 16384, actual_y = 16384;
PRIVATE int deltax = 0, deltay = 0;
PRIVATE int buttons_state = 0;
  
void initialise_mouse()
{ word res = get_int_config("mouse_resolution");
  word div = get_int_config("mouse_divisor");

  actual_x = 16384; actual_y = 16384;	/* current internal coordinates */
  deltax = 0; deltay = 0;               /* accumulated deltas in clicks */
                                        /* must change by mouse_div to */
                                        /* affect actual_x actual_y */

  
  if (res eq Invalid_config)
   mouse_resolution = 1;
  else
   mouse_resolution = (int) res;

  if (div eq Invalid_config)
   mouse_divisor = 1;
  else
   mouse_divisor = (int) div;
   
  if (mouse_divisor eq 0) mouse_divisor = 1;
  if (mouse_resolution eq 0) mouse_resolution = 1;
  
    
  if (reset_mouse()) 
   {
     mouse_loaded = 1;
     set_mouse_resolution(1);  /* only affect time of first int */
   }  
}

void tidy_mouse()
{ if (mouse_loaded && mouse_active)
   disable_mouse();
  mouse_active = 0;
}

void start_mouse()
{ if (!mouse_loaded) return;
  enable_mouse();
  mouse_active = 1;
}

void stop_mouse()
{ if (!mouse_loaded) return;
  disable_mouse();
  mouse_active = 0;
}

void mouse_something(dx, dy, buttons)
int dx, dx, buttons;
{ 
  int xorbuttons;
  int deltax2,deltay2;
#define but_left	1
#define but_right	2
#define but_middle	4

  /* add up deltas to check if we have moved enough */

  deltax2 = (deltax += dx)/mouse_divisor;
  deltay2 = (deltay += dy)/mouse_divisor;
  
  if ((abs(deltax2) >= mouse_resolution) ||
      (abs(deltay2) >= mouse_resolution))
  {
    if (Mouse_Relative)
    {	
      actual_x =deltax2;
      actual_y =deltay2;	
    }
    else
    {  
      actual_x+=deltax2;
      actual_y+=deltay2;	
      actual_y &= 0x7FFF;
      actual_x &= 0x7FFF;
    }  

    new_mouse(actual_x, actual_y,Buttons_Unchanged);
    deltax = deltax%mouse_divisor;
    deltay = deltay%mouse_divisor;
    deltax2 = 0;
    deltay2 = 0;
  }	   

  /* have the buttons changed ? */
  /* probably want real pos if they have */

  if (xorbuttons = (buttons^buttons_state))
  {
    /*	
    ** the following lines should be ok
    ** wether we have already done a new_mouse or not  	 
    */
    
    if (Mouse_Relative)
    {	
      actual_x =deltax2;
      actual_y =deltay2;	
    }
    else
    {  
      actual_x+=deltax2;
      actual_y+=deltay2;	
      actual_y &= 0x7FFF;
      actual_x &= 0x7FFF;
    }  

    deltax = deltax%mouse_divisor;
    deltay = deltay%mouse_divisor;
    
    if (xorbuttons & but_left)
    {
      new_mouse(actual_x,actual_y,
         buttons&but_left ? Buttons_Button0_Down : Buttons_Button0_Up);
    } 	
   
    if (xorbuttons & but_right)
    {
      new_mouse(actual_x,actual_y,
         buttons&but_right ? Buttons_Button1_Down : Buttons_Button1_Up);
    }
    	
    if (xorbuttons & but_middle)
    {
      new_mouse(actual_x,actual_y,
         buttons&but_middle ? Buttons_Button2_Down : Buttons_Button2_Up);
    } 	
   
    buttons_state = buttons; 
  }	
  
}
#endif /* mouse_supported */
/*}}}*/
/*{{{  raw keyboard */
#if keyboard_supported
                      /* The local keyboard routines */
#define Max_Key 0x7F  /* Highest scan code */
PRIVATE int  keyboard_active;
PRIVATE int  E0_code, E1_code, shift, control;
PRIVATE byte keys_down[Max_Key];
extern  void enable_keyboard(void);
extern  void disable_keyboard(void);

void initialise_keyboard()
{ keyboard_active = 0;
}

void tidy_keyboard()
{ if (keyboard_active)
   disable_keyboard();
  keyboard_active = 0;
}

void start_keyboard()
{ keyboard_active = 1;
  E0_code = 0; E1_code = 0; shift = 0; control = 0;
  memset(&(keys_down[0]), 0, Max_Key);
  enable_keyboard();
}

void stop_keyboard()
{ disable_keyboard();
  keyboard_active = 0;
}

extern  void new_keyboard(word updown, word scancode);

void keyboard_something(scancode)
unsigned int scancode;
{
  if (scancode eq 0x00E0)
   { E0_code = 1; return; }
  elif (scancode eq 0x00E1)
   { E1_code = 1; return; }

  if (E0_code)
   { int top_bit = scancode & 0x0080;
     E0_code = 0; scancode &= 0x007f;  
     switch(scancode)
      { case 0x001c : scancode = 0x0061 + top_bit; break;  /* numeric enter */
        case 0x001d : scancode = 0x0062 + top_bit; break;  /* right ctrl key */
        case 0x0035 : scancode = 0x0063 + top_bit; break;  /* numeric / */
        case 0x0037 : scancode = 0x0064 + top_bit; break;  /* Print screen */
        case 0x0038 : scancode = 0x0065 + top_bit; break;  /* right alt key */
        case 0x0047 : scancode = 0x0066 + top_bit; break;  /* home */
        case 0x0048 : scancode = 0x0067 + top_bit; break;  /* cursor up */
        case 0x0049 : scancode = 0x0068 + top_bit; break;  /* page up */
        case 0x004b : scancode = 0x0069 + top_bit; break;  /* cursor left */
        case 0x004d : scancode = 0x006a + top_bit; break;  /* cursor right */
        case 0x004f : scancode = 0x006b + top_bit; break;  /* end */
        case 0x0050 : scancode = 0x006c + top_bit; break;  /* cursor down */
        case 0x0051 : scancode = 0x006d + top_bit; break;  /* page down */
        case 0x0052 : scancode = 0x006e + top_bit; break;  /* insert */
        case 0x0053 : scancode = 0x0070 + top_bit; break;  /* delete */
        case 0x002a :
        case 0x0036 :        /* emulated shifts */
        default     : return;
      }
   }

  if (E1_code)          /* Generated by the Pause key only */
   switch(scancode)
    { case 0x001d :     /* emulated shifts, ignore */
      case 0x009d : return;   
      case 0x0045 : E1_code = 0; new_keyboard(Keys_KeyDown, 0x006fL); return;
      case 0x00c5 : E1_code = 0; new_keyboard(Keys_KeyUp, 0x006fL); return;
      default     : E1_code = 0; return;
    }

  if ((scancode & 0x0080) ne 0)    /* Test for key up */
   { scancode &= 0x007F;
     keys_down[scancode] = 0;
     if ((scancode eq 0x2a) || (scancode eq 0x36)) shift--;
     if ((scancode eq 0x1d) || (scancode eq 0x62)) control--;
     if (control && shift) return;
     new_keyboard(Keys_KeyUp, (word) scancode);
     return;
   }

                                   /* Bypass hardware auto repeat */
  if (keys_down[scancode] ne 0)
    return;     
  else
    keys_down[scancode] = 1;

  if (shift && control)
   { switch(scancode)
      {
#if debugger_incorporated
        case 0x0041 : DebugMode = 1 - DebugMode; break;
#endif
        case 0x0042 : Special_Status = true; break;
        case 0x0043 : Special_Exit = true; break;
        case 0x0044 : Special_Reboot = true; break;

        case 0x0010 : debugflags ^= Quit_Flag;        break;
        case 0x0011 : debugflags ^= Write_Flag;       break;
        /* 0x0012, 'e' unused */
        case 0x0013 : debugflags ^= Read_Flag;        break;
        case 0x0014 : debugflags ^= Timeout_Flag;     break;
        case 0x0015 : debugflags |= ListAll_Flag;     break;
        case 0x0016 : debugflags |= Nopop_Flag;       break;
        case 0x0017 : debugflags ^= Init_Flag;        break;
        case 0x0018 : debugflags ^= Open_Flag;        break;
        case 0x0019 : debugflags ^= Close_Flag;       break;
        /* 0x001E, 'a' is all */
        case 0x001F : debugflags ^= Search_Flag;      break;
        case 0x0020 : debugflags ^= Delete_Flag;      break;
        case 0x0021 : debugflags ^= FileIO_Flag;      break;
        case 0x0022 : debugflags ^= Graphics_Flag;    break;
        case 0x0023 : debugflags ^= HardDisk_Flag;    break;
        case 0x0024 : debugflags ^= Directory_Flag;   break;
        case 0x0025 : debugflags ^= Keyboard_Flag;    break;
        case 0x0026 : debugflags ^= Log_Flag;         break;
        case 0x002C : debugflags ^= Reconfigure_Flag; break;
        case 0x002D : debugflags ^= Memory_Flag;      break;
        case 0x002E : debugflags ^= Com_Flag;         break;
        case 0x002F : debugflags ^= OpenReply_Flag;   break;
        case 0x0030 : debugflags ^= Boot_Flag;        break;
        case 0x0031 : debugflags ^= Name_Flag;        break;
        case 0x0032 : debugflags ^= Message_Flag;     break;

        case 0x001E : if (debugflags eq 0L)
                       debugflags = All_Debug_Flags;
                      else
                       debugflags = 0L;
                      break;
      }
     return;
   }

  if ((scancode eq 0x2a) || (scancode eq 0x36)) shift++;
  if ((scancode eq 0x1d) || (scancode eq 0x62)) control++;

  new_keyboard(Keys_KeyDown, (word) scancode);
}
   
#endif /* Keyboard_supported */
/*}}}*/
/*{{{  disk I/O */
/*------------------------------------------------------------------------------
--
-- These routines implement the PC specific file handling routines.
--
------------------------------------------------------------------------------*/

/**
*** Find out which drives are connected, by trying each one in turn. Also, find
*** out which of these are floppies. There appears to be no way of doing this,
*** which is a big problem because there may be multiple logical drives
*** mapping to the same physical drive. Hence I use a configuration entry.
**/

word get_drives(floppies)
word *floppies; 
{ unsigned int temp, current_drive;
  int i;
  word result = 3L, mask = 4L;
  char *floplist = get_config("floppies");

  for (i = 2; i < 26; i++)
   { char name[8];
     strcpy(name, "a:\\*.*");
     name[0] = (char) ('a' + i);
     if (_dos_findfirst(name ,search_FileOrDir, (struct find_t *)
         &searchbuffer) eq 0)
      result |= mask;
     mask += mask;
   }

  *floppies = 0L;
  if (floplist eq NULL)
    { *floppies = 0x01L; result &= 0xfffffffDL; }
  else
    { for ( ; *floplist ne '\0'; floplist++)
       { if (islower(*floplist))
           *floppies |= (0x01L << (*floplist - 'a'));
         elif (isupper(*floplist))
           *floppies |= (0x01L << (*floplist - 'A'));
       }
    }

  floppy_errno = 0;
  return(result);
}

/**
*** The following routines implement the odds and ends described in module
*** files.c
**/

word create_file(name)
char *name;
{ int handle;
  if (_dos_creat(name, FileAttr_File, &handle))
    return(false);

  (void) _dos_close(handle);                    /* Do not leave the file open */
  return(true);
} 


word get_file_info(name, Heliosinfo)
char    *name;
ObjInfo *Heliosinfo;
{ int  itsadirectory = ((searchbuffer.attr eq FileAttr_Dir) ? 1 : 0); 
  word time = unixtime(searchbuffer.date, searchbuffer.time);

  { int i, j;
    for (i=0,j=0; i < 14; i++)             /* copy name excluding spaces */
      if (searchbuffer.name[i] eq ' ')
        continue;
      elif (searchbuffer.name[i] eq '\0')
        break;
      else                      /* lower case everything */
        Heliosinfo->DirEntry.Name[j++] = ToLower(searchbuffer.name[i]);
    Heliosinfo->DirEntry.Name[j] = '\0';
  }


                          /* we have got the info, now convert and store it */
  Heliosinfo->DirEntry.Type = swap(itsadirectory ? Type_Directory : Type_File);
  if (itsadirectory)
   Heliosinfo->DirEntry.Matrix = swap(DefDirMatrix);
  else
   { Heliosinfo->DirEntry.Matrix = DefFileMatrix | AccMask_E;
     if (searchbuffer.attr & _A_RDONLY)
      Heliosinfo->DirEntry.Matrix &= ~0x42424242L;
     Heliosinfo->DirEntry.Matrix = swap(Heliosinfo->DirEntry.Matrix);
   }

  Heliosinfo->DirEntry.Flags  = swap(0L);
  Heliosinfo->Account   = swap(0L);
  Heliosinfo->Size      = swap(searchbuffer.size);
  Heliosinfo->Creation  = swap(time);
  Heliosinfo->Access    = swap(time);
  Heliosinfo->Modified  = swap(time);

  return(true);
}

word set_file_date(name, unixstamp)
char *name;
word unixstamp;
{ int MSdate, MStime;
  int handle;
                                   /* Get the stamps in the right format */
  MSdate = MSdate_stamp(unixstamp);
  MStime = MStime_stamp(unixstamp);

  if (_dos_open(name,  (int) O_ReadOnly, &handle) ne 0)
    return(false);

  _dos_setftime(handle, MSdate, MStime);
                           /* close the file again now that I am finished */
  _dos_close(handle);

  return(true);
}

word get_drive_info(name, reply)
char *name;
servinfo *reply;
{ struct diskfree_t data;
  int  drive;
  reply->type = swap(Type_Directory);

  if (name[0] eq '\\')
   drive = 0;
  elif ((ToLower(name[0]) < 'a') || (ToLower(name[0]) > 'z'))
   return(false);
  else
   drive = ToLower(name[0]) - 'a' + 1;      /* drive identifier, A=1 etc.*/

  if (_dos_getdiskfree(drive, &data)) return(false);
  reply->size = swap((word) data.total_clusters *
         (word) data.sectors_per_cluster * (word) data.bytes_per_sector);
  reply->used = swap((word) data.avail_clusters *
         (word) data.sectors_per_cluster * (word) data.bytes_per_sector);
  reply->alloc = swap((word) data.sectors_per_cluster *
         (word) data.bytes_per_sector);

  return(true);
}

int open_file(name, Heliosmode)
char *name;
word Heliosmode;
{ int  handle;
  word itsmode, mymode;

  itsmode = Heliosmode & 0x0F;  /* ignore create bit */
  if (itsmode eq O_ReadOnly)
    mymode = OpenMode_ReadOnly;
  elif (itsmode eq O_WriteOnly)
    mymode = OpenMode_WriteOnly;
  elif (itsmode eq O_ReadWrite)
    mymode = OpenMode_ReadWrite;
  else
     return(0);

  if(Heliosmode & O_Truncate)
   { if (_dos_creat(name, FileAttr_File, &handle) ne 0)
       handle = 0;
   } 
  else
   { if (_dos_open(name, (int) mymode, &handle) ne 0)
       handle = 0;
   }
 
  if ((handle eq 0) && (O_Create & Heliosmode))
    if (_dos_creat(name, FileAttr_File, &handle) ne 0)
       handle = 0;

  if ((handle eq 0) && (errno eq EMFILE))
   Server_errno = EC_Error + SS_IOProc + EG_NoResource + EO_Server;

  return(handle);
}

/**
*** Do_Search() is responsible for searching through the entire directory
*** whose name it is given, converting the information obtained to a
*** Helios DirEntryStructure, and storing that in the linked list passed as
*** argument. This conversion is done by routine add_node(). For details of
*** Fsfirst() and Fsnext() please consult an MSdos manual. The routine
*** returns -1 to indicate an error, otherwise the number of entries in the
*** list.
**/

PRIVATE word add_node(List *header);

word search_directory(pathname, header)
char    *pathname;
List    *header;
{ word result;
  int count = 0;

  strcat(pathname, "\\*.*");      /* get a name for search for first */

  result = (word) _dos_findfirst(pathname, search_FileOrDir,
          (struct find_t *) &searchbuffer);

  for (;;)
    {
      if (result eq 0)    /* not reached the end of the search */
        { count++; unless(add_node(header)) return(-1); }
      elif ((result eq 18) || (result eq 2) )
        return(count);           /* end of search */
      else
        return(-1);

      result = _dos_findnext((struct find_t *) &searchbuffer);
    }
}

/**
*** When the search through the directory has revealed another entry, all
*** the information about this entry will be stored in searchbuffer. It
*** is necessary to convert this information to the Helios DirEntry
*** structure, amongst other things changing the name to lower case because
*** it looks nicer. This done I can add the entry to the linked list.
**/

PRIVATE word add_node(header)
List *header;
{ DirEntryNode *newnode;
  word type;
  int i, j;

  if (searchbuffer.attr eq search_FileOrDir)
    type = Type_Directory;
  else
    type = Type_File;

  newnode = NewDirEntryNode();
  unless(newnode) return(false);
      
  clear_bytes((char *) newnode, sizeof(DirEntryNode));
  (newnode->direntry).Type     = swap(type);
  (newnode->direntry).Flags    = swap(0L);
  (newnode->direntry).Matrix   = swap( (type eq Type_Directory) ? 
                                       DefDirMatrix : (DefFileMatrix | AccMask_E) );
  for (i=0,j=0; i < 14; i++)             /* copy name excluding spaces */
    if (searchbuffer.name[i] eq ' ') continue;
    elif (searchbuffer.name[i] eq '\0')
      break;
    else                               /* lower case everything - prettier */  
      (newnode->direntry).Name[j++] = ToLower(searchbuffer.name[i]);
  (newnode->direntry).Name[j] = '\0';

  AddTail(&(newnode->node), header);    /* put the node at the end of the list */

  return(true);
}

word format_floppy(char *name, word a, word b, word c, char *d)
{ return(false);
}


/**
*** Getting the size is fairly expensive, as I need to seek to the end of file
*** and then back to my old position. In theory I could avoid the second seek
*** by fiddling with the file position pointer in the File_extra field, but
*** I prefer not to do that.
**/

word get_file_size(handle, old_pos)
int  handle;
word old_pos;
{ word end;

  end = seek_in_file(handle, 0L, 1);   /* position of EOF */

  if ((seek_in_file(handle, old_pos, 0 ) eq -1L) || (end eq -1L))
    return(-1L); 
  else
    return(end);
}


/**
*** The MSdos rename is not entirely satisfactory because it cannot overwrite
*** an existing file. Hence I may have to delete the file. Ofcourse I had
*** better check that the two files are not the same before I start deleting.
**/
word rename_object(fromname, toname)
char *fromname, *toname;
{ int temp;

  if (!mystrcmp(fromname, toname))                 /* rename on top of itself */
    return(true);

	/* do not allow renaming of read-only files	*/
  if (searchbuffer.attr & _A_RDONLY)
   return(false);

         /* On the PC I cannot rename a file on top of an existing one, so I  */
         /* have to delete the destination if it exists                       */
  if (_dos_findfirst(toname, search_FileOrDir, 
                     (struct find_t *) &searchbuffer) eq 0)
    remove(toname);

  temp = rename(fromname, toname);
  return(temp eq 0 ? true : false);
}

word read_from_file(handle, amount, buffer)
int handle;
word amount;
byte *buffer;
{ unsigned int read;
  int          x;
  
  if ((x = _dos_read(handle, buffer, (unsigned int) amount, &read)) ne 0)
{ 
    return(-1L);
}
  else
   return((word) read);
}

word write_to_file(handle, amount, buffer)
int handle;
word amount;
byte *buffer;
{ unsigned int written;

  if (_dos_write(handle, buffer, (unsigned int) amount, &written) ne 0)
   return(false);

  if (written ne (unsigned int) amount)
   return(false);
  else
   return(true);
}

word seek_in_file(int handle, word offset, int mode)
{ union _REGS inregs;
  union _REGS outregs;
  int        result;
  word       newpos;
  
  inregs.h.ah = 0x42;
  if (mode eq 0)
   inregs.h.al = 0;
  else
   inregs.h.al = 2;
  inregs.x.bx = handle;
  inregs.x.cx = (int) ((offset >> 16) & 0x0FFFF);
  inregs.x.dx = (int) (offset & 0x0FFFF);
  result = _intdos(&inregs, &outregs);

  if (outregs.x.cflag)
   { 
     return(-1);
   }
   
  newpos  = (word) outregs.x.dx;
  newpos  = (newpos << 16) & 0xFFFF0000L;
  newpos |= (word) outregs.x.ax;
  if (newpos < 0L)
   { inregs.h.ah = 0x42;	/* go back to position 0 */
     inregs.h.al = 0;
     inregs.x.bx = handle;
     inregs.x.cx = 0;
     inregs.x.dx = 0;
     (void) _intdos(&inregs, &outregs);
     return(0L);
   }
  else
   return(newpos);
}
/*}}}*/
/*{{{  raw disk and rom disk */
/**
*** Rawdisk support
**/

#define Max_Drives 8

static uint drives[Max_Drives];
static word sector_offsets[Max_Drives];
static word disk_sizes[Max_Drives];
extern uint disk_read(int drive, uint no_sects, word first_sec, byte *buffer);
extern uint disk_write(int drive, uint no_sects, word first_sec, byte *buffer);

int number_rawdisks()
{ char *drive_id = get_config("rawdisk_drive");
  int no_fats, sizeof_fat;
  unsigned char *buf = (unsigned char *) &(misc_buffer1[0]);
  int no_drives = 0;

  if (drive_id eq (char *) NULL)
   return(0);

  for ( ; (*drive_id ne '\0') && (no_drives < Max_Drives); drive_id++)
  { if (isupper(*drive_id)) *drive_id = tolower(*drive_id);
    if ((*drive_id < 'a') || (*drive_id > 'z'))
      { ServerDebug("Invalid rawdisk drive %c in host.con", *drive_id);
        continue;
      }

    drives[no_drives] = *drive_id - 'a';
    if (drives[no_drives] eq 2)
     { ServerDebug("Drive C cannot be a raw disk.");
       continue;
     }

    if (disk_read(drives[no_drives], 1, 0L, buf) ne 0)
     { ServerDebug("Error reading boot sector of raw disk %c", *drive_id);
       continue;
     }

    no_fats               = (int) buf[16];
    sizeof_fat            = (int) buf[23];
    sizeof_fat            = (sizeof_fat << 8) + (int) buf[22];
    disk_sizes[no_drives]  = (word) buf[20];
    disk_sizes[no_drives]  = (256L * disk_sizes[no_drives]) + (word) buf[19];

        /* MS-dos 4.0 has an incompatible structure */
    if ((disk_sizes[no_drives] eq 0L) && (version_number >= 4)) 
     {
       disk_sizes[no_drives] = (word) buf[35];
       disk_sizes[no_drives] = (256L * disk_sizes[no_drives]) + (word) buf[34];
       disk_sizes[no_drives] = (256L * disk_sizes[no_drives]) + (word) buf[33];
       disk_sizes[no_drives] = (256L * disk_sizes[no_drives]) + (word) buf[32];
     }

     /* Allow 1 sector for root, sectors for FATs, and 1 for root directory */  
    sector_offsets[no_drives] = (word) (1 + (no_fats * sizeof_fat) + 1);
    disk_sizes[no_drives] -= sector_offsets[no_drives];
    no_drives++;
  }
 return(no_drives);
}

word size_rawdisk(drive)
int drive;
{ return(disk_sizes[drive]);
}

word read_rawdisk(hdrive, offset, size, buff)
int hdrive;
word offset;
word size;
byte *buff;
{ int x;
  x = disk_read(drives[hdrive], (uint) size, sector_offsets[hdrive] + offset,
                buff);

  return((word) !x);
}


word write_rawdisk(hdrive, offset, size, buff)
int hdrive;
word offset;
word size;
byte *buff;
{ int x = disk_write(drives[hdrive], (uint) size,
                     sector_offsets[hdrive] + offset, buff);

  return((word) !x);
}

#if Romdisk_supported

int number_romdisks()
{
  return 1;	
}

word size_romdisk(drive)
int drive;
{
  return(0x180000L);	
}

#define pagesize 0x10000L

static unsigned char* huntmem = (unsigned char*) 0xD0000000;

word read_romdisk(hdrive, offset, size, buff)
int hdrive;
word offset;
word size;
byte* buff;
{
  /*
  ** need to calcualte which page we are in
  */
  
  word page  = ((offset+=0x80000) / pagesize);
  word pageoff = offset & (pagesize-1);
  word pagend = (page+1) * pagesize;
  word avail  = pagend - offset;
  word wanted = size;
  word toget;
  
  while (size > 0)
  {
    if (page > 31)
    {
      if (size eq wanted) return -1;   /* END OF FILE FUDGE */	
      break; 
    }  
    outp(0x360,page);
    toget =  (size > avail) ? avail : size;
    
    memcpy(buff, &huntmem[pageoff], toget);
    
    size -= toget;
    buff += toget;
    page ++;
    pageoff = 0;
    avail = pagesize;
  }
  outp(0x360,0);
    
  return wanted - size;
}
#endif
/*}}}*/
/*{{{  etc directory */
/**
*** File name translation. To allow Helios to be run over a network where
*** the bin and lib directories reside on the file server but the etc
*** directory is local, accesses to /helios/etc may involve an additional
*** name change. Also, /helios/local/spool and /helios/tmp require special
*** treatment.
**/
static	char	*etc_directory;
static	char	etc_name[128];
static	int	etc_length;
static	char	*tmp_directory;
static	char	tmp_name[128];
static	int	tmp_length;
static	char	*spool_directory;
static	char	spool_name[128];
static	int	spool_length;

static void initialise_etc_directory(void)
{
  etc_directory = get_config("etc_directory");
  if (etc_directory ne NULL)
   { strcpy(etc_name, Heliosdir);
     strcat(etc_name, "\\etc");
     etc_length = strlen(etc_name);
   }

  tmp_directory = get_config("tmp_directory");
  if (tmp_directory ne NULL)
   { strcpy(tmp_name, Heliosdir);
     strcat(tmp_name, "\\tmp");
     tmp_length = strlen(tmp_name);
   }

  spool_directory = get_config("spool_directory");
  if (spool_directory ne NULL)
   { strcpy(spool_name, Heliosdir);
     strcat(spool_name, "\\local\\spool");
     spool_length = strlen(spool_name);
   }
}

	/* Accesses to /helios/etc have to be intercepted.	*/
	/* This routine is called after the name translation in	*/
	/* files.c, so the name should be something like	*/
	/* <heliosdir>\xyz					*/
void check_helios_name(char *name)
{
  if (etc_directory ne NULL)
   { if (!strncmp(name, etc_name, etc_length))
      { strcpy(misc_buffer1, etc_directory);
        strcat(misc_buffer1, &(name[etc_length]));
        strcpy(name, misc_buffer1);
        return;
      }
   }

  if (tmp_directory ne NULL)
   { if (!strncmp(name, tmp_name, tmp_length))
      { strcpy(misc_buffer1, tmp_directory);
        strcat(misc_buffer1, &(name[tmp_length]));
        strcpy(name, misc_buffer1);
        return;
      }
   }

  if (spool_directory ne NULL)
   { if (!strncmp(name, spool_name, spool_length))
      { strcpy(misc_buffer1, spool_directory);
        strcat(misc_buffer1, &(name[spool_length]));
        strcpy(name, misc_buffer1);
        return;
      }
   }
}
/*}}}*/
/*{{{  misc. device initialisation */
/*
** These have been moved to the bottom so that they
** can initialise anything in this file
*/

extern void set_interrupts(void);
extern void restore_interrupts(void);

/**
*** This routine disables ctrl-C and ctrl-Break handling yet again, and
*** sets the standard streams to raw mode. Then it installs some
*** interrupt vectors, e.g. to trap divide by 0.
***
*** To support the vy86pid board this routine is also responsible for
*** initialising the RS232 hardware.
**/
void PC_initialise_devices()
{ 
  int i;

  initialise_etc_directory();

#if RS232_supported
  RS232_initialise_hardware();
#endif
#if Centronics_supported
  for (i = 0; i < MaxCentronics; i++) 
   Centronics_coms[i].id = -1L;
#endif

#if Ether_supported
  { extern void initialise_ethernet();
    initialise_ethernet();
  }
#endif

#if !(MSWINDOWS)
  rg.h.ah = 0x33;
  rg.h.al = 1;
  rg.h.dl = 0;
  _intdos(&rg,&rg);
  /* Set input and output to/from raw mode */
  PC_setmode(0, 0);  /* For input */
  PC_setmode(1, 0);  /* For output */
#endif

      /* Install my own interrupt vectors, see pcasm.asm */
  set_interrupts();
}

/**
*** The inverse of the above.
**/
void PC_restore_devices()
{
#if !(MSWINDOWS)
  rg.h.ah = 0x33;
  rg.h.al = 1;
  rg.h.dl = 1;
  _intdos(&rg,&rg);
  /* Set input and output to/from raw mode */
  PC_setmode(0, 1);  /* For input */
  PC_setmode(1, 1);  /* For output */
#endif

#if RS232_supported
  RS232_tidy();
#endif

#if Ether_supported
  { extern void tidy_ethernet();
    tidy_ethernet();
  }
#endif

     /* restore the interrupt vectors */
  restore_interrupts();
}

/*}}}*/


