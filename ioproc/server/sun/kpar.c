/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--     kpar.c                                                           --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: kpar.c,v 1.3 1991/10/21 10:17:32 martyn Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.        			*/

#define Linklib_Module

#include "../helios.h"

#define link_fd (link_table[current_link].fildes)
extern int transputer_site;

/**
*** This code is specific to the KPar device driver interface.
**/
#ifdef NEVER
#include <sys/imbio.h>
#else

#define	IMB_DMA_OFF		0
#define	IMB_DMA_READ		1
#define	IMB_DMA_WRITE		2
#define	IMB_DMA_READWRITE	(IMB_DMA_READ|IMB_DMA_WRITE)	
/*
 * I/O controls
 */
#define	IMB_RESET		_IO(k,0)	/* Reset site */
#define	IMB_ANALYSE		_IO(k,1)	/* Analyse site */
#define	IMB_ENABLE_ERRORS	_IO(k,2)	/* Abort i/o on error */
#define	IMB_DISABLE_ERRORS	_IO(k,3)	/* Ignore errors */
#define	IMB_ERROR		_IOR(k,4,int)	/* Is error flag set? */
#define	IMB_INPUT_PENDING	_IOR(k,5,int)	/* Is input pending */
#define IMB_DMA			_IOW(k,6,int)	/* DMA setup (read/write/off */
#endif

/**
*** The KPar device driver supports two B008 boards, /dev/imb0 and
*** /dev/imb1, although the second does not have to be installed.
*** Because a standard device driver is used, reading and writing
*** the link can be done using unix read() and write(), without
*** problems. There is no ioctl for rdrdy() or wrrdy(), so rdrdy()
*** is done by a select() call and wrrdy() is a no-op. Reset and
*** analyse is done by ioctls.
***
*** The code defines a static table for the two links. kpar_init_link()
*** specifies two link adapters, sets the link_table pointer, and
*** initialises the two entries. kpar_open_link() opens the
*** appropriate link device, stores the file descriptor in the
*** table, and sets the link adapter to a sensible state.
*** kpar_free_link() closes the file descriptor. The reset and
*** and analyse routines do straightforwards ioctl calls.
**/

PRIVATE Trans_link kpar_links[2];

void kpar_init_link()
{ number_of_links = 2;
  link_table = &(kpar_links[0]);
  strcpy(kpar_links[0].link_name, "/dev/imb0");
  strcpy(kpar_links[1].link_name, "/dev/imb1");

  for ( ; number_of_links >= 0; number_of_links--)
   { struct stat buf;
     if (stat(link_table[number_of_links-1].link_name, &buf) eq 0)
      break;                   /* OK, found the last known site */
     if (errno ne ENOENT)      /* Appears to exist, but not currently usable */
      break;
   }       
}

int kpar_open_link(tabno)
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

  if (ioctl(link_table[tabno].fildes, IMB_RESET) eq -1)
   { close(link_table[tabno].fildes);
     return(0);
   }

  fcntl(link_table[tabno].fildes, F_SETFD, 1);
  ioctl(link_table[tabno].fildes, IMB_DISABLE_ERRORS);
  if (get_config("no_dma") eq (char *) NULL)
   ioctl(link_table[tabno].fildes, IMB_DMA, IMB_DMA_OFF);

  return(1);
} 

void kpar_free_link(tabno)
int tabno;
{ if (link_table[tabno].fildes ne -1)
   { /* ioctl(link_table[tabno].fildes, IMB_RESET); */
     close(link_table[tabno].fildes);
   }
  link_table[tabno].fildes = -1;
}


void kpar_analyse_transputer()
{
  if (ioctl(link_fd, IMB_ANALYSE) eq -1)
    ServerDebug("Warning, failed to analyse site");
}

void kpar_reset_transputer()
{ if (ioctl(link_fd, IMB_RESET) eq -1)
    ServerDebug("Warning, failed to reset site");
  ioctl(link_fd, IMB_DISABLE_ERRORS);
}
