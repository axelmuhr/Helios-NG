/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--     bbk-pc.c                                                           --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: bbk-pc.c,v 1.1 1993/07/09 14:19:11 tony Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.        			*/

#define Linklib_Module

#include "../helios.h"

#define link_fd (link_table[current_link].fildes)
extern int transputer_site;

/**
*** This code is specific to the BBK-PC device driver interface.
**/
#ifdef NEVER

#include <sys/bbkio.h>

#else

/*
 * I/O controls
 */
#define	BBK_RESET		(('k'<<8)|0)	/* Reset site */
#define	BBK_ANALYSE		(('k'<<8)|1)	/* Analyse site */
#define	BBK_ENABLE_ERRORS	(('k'<<8)|2)	/* Abort i/o on error */
#define	BBK_DISABLE_ERRORS	(('k'<<8)|3)	/* Ignore errors */
#define	BBK_ERROR		(('k'<<8)|4)	/* Is error flag set? */
#define	BBK_INPUT_PENDING	(('k'<<8)|5)	/* Is input pending */
#define BBK_TIMEOUT		(('k'<<8)|7)	/* Set timeout */
#define BBK_OUTPUT_READY	(('k'<<8)|8)	/* Ready to output */

#endif

/**
*** The BBK-PC device driver supports 126 BBK-PC boards, /dev/bbk0 ...
*** /dev/bbk125.Because a standard device driver is used, reading and writing
*** the link can be done using unix read() and write(), without
*** problems. 
*** 
*** 
***
*** The code defines a static table for the two links. bbk_init_link()
*** specifies three link adapters, sets the link_table pointer, and
*** initialises the two entries. bbk_open_link() opens the
*** appropriate link device, stores the file descriptor in the
*** table, and sets the link adapter to a sensible state.
*** bbk_free_link() closes the file descriptor. The reset and
*** and analyse routines do straightforwards ioctl calls.
**/

#define NUMBER_OF_LINKS	126

PRIVATE Trans_link kpar_links[NUMBER_OF_LINKS];

void bbk_init_link()
{ 
  int	i;

  number_of_links = NUMBER_OF_LINKS;

  link_table = &(kpar_links[0]);

  for (i = 0; i < NUMBER_OF_LINKS; i++)
  {  
	sprintf (kpar_links[i].link_name, "/dev/bbk%d", i);
  }

  for ( ; number_of_links >= 0; number_of_links--)
   { struct stat buf;
     if (stat(link_table[number_of_links-1].link_name, &buf) eq 0)
     {
      break;                   /* OK, found the last known site */
     }
     if (errno ne ENOENT)      /* Appears to exist, but not currently usable */
     {
      break;
     }
   }       

}

int bbk_open_link(tabno)
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

  if (ioctl(link_table[tabno].fildes, BBK_RESET) eq -1)
   { close(link_table[tabno].fildes);
     return(0);
   }

  fcntl(link_table[tabno].fildes, F_SETFD, 1);
  ioctl(link_table[tabno].fildes, BBK_DISABLE_ERRORS);

/* The BBK-PC does not support DMA */
#ifdef NEVER
  if (get_config("no_dma") ne (char *) NULL)
   ioctl(link_table[tabno].fildes, BBK_DMA, BBK_DMA_OFF);
#endif


  /* the device drive does not support select so we must set this flag */

  link_table[tabno].flags |= Link_flags_not_selectable;
  return(1);
} 

void bbk_free_link(tabno)
int tabno;
{ if (link_table[tabno].fildes ne -1)
   { /* ioctl(link_table[tabno].fildes, BBK_RESET); */
     close(link_table[tabno].fildes);
   }
  link_table[tabno].fildes = -1;
}


PUBLIC void bbk_analyse_transputer()
{
  if (ioctl(link_fd, BBK_ANALYSE) eq -1)
    ServerDebug("Warning, failed to analyse site");
}

PUBLIC void bbk_reset_transputer()
{ if (ioctl(link_fd, BBK_RESET) eq -1)
    ServerDebug("Warning, failed to reset site");
  ioctl(link_fd, BBK_DISABLE_ERRORS);
}

PUBLIC int bbk_rdrdy( void )
{
   int res = 0;
   if (ioctl(link_fd,BBK_INPUT_PENDING, &res) eq -1)
    ServerDebug("Warning, bbk_rdrdy");

   return res; 
}

PUBLIC int bbk_wrrdy( void )
{
  int res = 0;
   if (ioctl(link_fd,BBK_OUTPUT_READY, &res) eq -1)
    ServerDebug("Warning, bbk_wrrdy");

  return res; 
}

