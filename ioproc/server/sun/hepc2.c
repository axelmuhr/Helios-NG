/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--     hepc2.c - link i/o code for Hunt Engineering HEPC2 board         --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: hepc2.c,v 1.3 1993/03/23 15:13:37 bart Exp $ */
/* Copyright (C) 1992, Perihelion Software Ltd.        			*/

#define Linklib_Module

#include "../helios.h"

#define link_fd (link_table[current_link].fildes)

/**
*** This code is specific to the HEPC device driver interface.
**/

/*
 * I/O controls
 */

#define	HEPC_RESET		(('k'<<8)|0)	/* Reset site */
#define	HEPC_INPUT_PENDING	(('k'<<8)|1)	/* Is input pending */
#define HEPC_TIMEOUT		(('k'<<8)|2)	/* Set timeout */
#define HEPC_OUTPUT_READY	(('k'<<8)|3)	/* Ready to output */

/**
*** The HEPC device driver supports three boards, /dev/hepc0, /dev/hepc1
*** and /dev/hepc2, although only the first has to be installed.
*** Because a standard device driver is used, reading and writing
*** the link can be done using unix read() and write(), without
*** problems. 
*** 
*** The code defines a static table for the two links. tim40_init_link()
*** specifies three link adapters, sets the link_table pointer, and
*** initialises the two entries. tim40_open_link() opens the
*** appropriate link device, stores the file descriptor in the
*** table, and sets the link adapter to a sensible state.
*** tim40_free_link() closes the file descriptor. The reset and
*** and analyse routines do straightforwards ioctl calls.
**/

PRIVATE Trans_link hepc_links[3];

void tim40_init_link()
{ 
  number_of_links = 3;

  link_table = &(hepc_links[0]);
  
  strcpy(hepc_links[0].link_name, "/dev/hepc0");
  strcpy(hepc_links[1].link_name, "/dev/hepc1");
  strcpy(hepc_links[2].link_name, "/dev/hepc2");

  for ( ; number_of_links >= 0; number_of_links--)
   { struct stat buf;
     if (stat(link_table[number_of_links-1].link_name, &buf) eq 0)
      break;                   /* OK, found the last known site */
     if (errno ne ENOENT)      /* Appears to exist, but not currently usable */
      break;
   }       
}

int tim40_open_link(tabno)
int tabno;
{ int j;
  struct flock lock;

  link_table[tabno].fildes = open(link_table[tabno].link_name, O_RDWR);
  if (link_table[tabno].fildes eq -1)
      return(0);
  lock.l_type = F_WRLCK;
  lock.l_whence = 0;
  lock.l_start  = 0L;
  lock.l_len    = 0L;
  if (fcntl(link_table[tabno].fildes, F_SETLK, &lock) eq -1)
   { close(link_table[tabno].fildes);
     return(0);
   }

  if (ioctl(link_table[tabno].fildes, HEPC_RESET) eq -1)
   { close(link_table[tabno].fildes);
     return(0);
   }

  fcntl(link_table[tabno].fildes, F_SETFD, 1);
#if 0
  if (get_config("no_dma") ne (char *) NULL)
   ioctl(link_table[tabno].fildes, HEPC_DMA, HEPC_DMA_OFF);
#endif

  /* the device drive does not support select so we must set this flag */

  link_table[tabno].flags |= Link_flags_not_selectable + Link_flags_word;
  return(1);
} 

void tim40_free_link(tabno)
int tabno;
{ 
if (link_table[tabno].fildes ne -1)
     close(link_table[tabno].fildes);

  link_table[tabno].fildes = -1;
}

PUBLIC void tim40_reset()
{ if (ioctl(link_fd, HEPC_RESET) eq -1)
    ServerDebug("Warning, failed to reset site");
}

PUBLIC int tim40_rdrdy( void )
{
   int res = 0;
   if (ioctl(link_fd,HEPC_INPUT_PENDING, &res) eq -1)
    ServerDebug("Warning, tim40_rdrdy");

  return res; 
}

PUBLIC int tim40_wrrdy( void )
{
  int res = 0;
   if (ioctl(link_fd,HEPC_OUTPUT_READY, &res) eq -1)
    ServerDebug("Warning, tim40_wrrdy");

  return res; 
}

