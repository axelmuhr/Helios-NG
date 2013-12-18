/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--     gnome.c                                                          --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1989, Perihelion Software Ltd.        */
/* RcsId: $Id: gnome.c,v 1.2 90/11/29 00:16:10 paul Exp $ */


#define Linklib_Module

#include "helios.h"

#define link_fd (link_table[current_link].fildes)

/**
*** The Gnome link podule for the Acorn R140 is supported by a
*** decent device driver, so there are no problems. It even
*** supports ioctls for rdrdy and wrrdy !!! Boards are
*** names /dev/link0 to /dev/link3. 
**/

#define RESETDELAY 330000	/* fix required for ab1 prototype boards! */

#if 1
#include <dev/link.h>
#else

#define LIOSLRESET	_IO(l, 0)	/* Set the link reset line */
#define	LIOCLRESET	_IO(l, 1)	/* Clear the link reset line */
#define	LIOSANALYSE	_IO(l, 2)	/* Set the link analyse line */
#define	LIOCANALYSE	_IO(l, 3)	/* Clear the link analyse line */
#define	LIOSPEED20	_IO(l, 4)	/* Set link speed to 20 MHz */
#define	LIOSPEED10	_IO(l, 5)	/* Set link speed to 10 MHz */
#define	LIOCHIPRESET	_IO(l, 6)	/* Reset the link adapter chip */
#define	LIOSTATUS	_IOR(l, 7, int)	/* Return a set of link status flags */


/* Flag bits in the result of LIOSTATUS (some useful for debugging only) */

#define LS_DATAPRESENT	0x01	/* Data present on input        */
#define LS_OUTPUTREADY	0x02	/* Output ready to receive data */
#define LS_ERRORIN	0x04	/* Reflects ErrorIn signal      */
#define LS_LINKSPEED20	0x08	/* 0 => 10MHz, 1 => 20MHz       */
#define LS_RESETOUT	0x10	/* Reflects ResetOut signal     */
#define LS_ANALYSEOUT	0x20	/* Reflects AnalyseOut signal   */
#define LS_READINTEN	0x40	/* Read interrupt enabled       */
#define LS_WRITEINTEN	0x80	/* Write interrupt enabled      */

#endif

#define Gnome_Max_Link 4
PRIVATE Trans_link gnome_links[4];

void gnome_init_link()
{ number_of_links = Gnome_Max_Link;
  link_table = &(gnome_links[0]);
  strcpy(gnome_links[0].link_name, "/dev/link0");
  strcpy(gnome_links[1].link_name, "/dev/link1");
  strcpy(gnome_links[2].link_name, "/dev/link2");
  strcpy(gnome_links[3].link_name, "/dev/link3");

  for ( ; number_of_links >= 0; number_of_links--)
   { struct stat buf;
     if (stat(link_table[number_of_links].link_name, &buf) eq 0)
      break;                   /* OK, found the last known site */
     if (errno ne ENOENT)      /* Appears to exist, but not currently usable */
      break;
   }       
}

int gnome_open_link(tabno)
int tabno;
{
  link_table[tabno].fildes = open(link_table[tabno].link_name, O_RDWR);
  if (link_table[tabno].fildes eq -1)
   return(0);

  fcntl(link_table[tabno].fildes, F_SETFD, 1);
  ioctl(link_table[tabno].fildes, LIOCHIPRESET, 0);
  if(get_int_config("LINK_SPEED") == 10)
    ioctl(link_table[tabno].fildes, LIOSPEED10, 0);  /* 10 meg link speed */
  else
    ioctl(link_table[tabno].fildes, LIOSPEED20, 0);  /* 20 meg link speed */

  if(get_config("no_reset_target") == NULL)
  {
    int i;

    ioctl(link_table[tabno].fildes, LIOCANALYSE, 0);
    ioctl(link_table[tabno].fildes, LIOSLRESET, 0);
    for (i=0; i<RESETDELAY; ++i); /* Wait a while - support for AB1 prototype */  
    ioctl(link_table[tabno].fildes, LIOCLRESET, 0);
  }

  fcntl(link_table[tabno].fildes, F_SETFD, 1);
  return(1);
} 

void gnome_free_link(tabno)
int tabno;
{ if (link_table[tabno].fildes ne -1)
   {
     /* if we are just attaching into a rom based system we shouldn't reset */
     /* the whole system when we detach! */

     if( ! (get_config("just_attach") != NULL || get_config("attach") != NULL
     	    || get_config("no_reset_target") != NULL))
     {
      int i;

      ioctl(link_table[tabno].fildes, LIOSLRESET);
      for (i=0; i<RESETDELAY; ++i); /* Wait a while - support for AB1 prototype */  
      ioctl(link_table[tabno].fildes, LIOCLRESET);
     }
     close(link_table[tabno].fildes);
   }
  link_table[tabno].fildes = -1;
}

/**
*** At the moment the debug protocol is not supported.
**/
void handle_debug_protocol()
{
}

WORD gnome_rdrdy()
{ int status;
  if (ioctl(link_fd, LIOSTATUS, &status) == -1)
  { ServerDebug("rdrdy: ioctl failed");
    return(FALSE);
  }
  return ((status & LS_DATAPRESENT) );
}

/* Check whether link can accept data */
WORD gnome_wrrdy()
{ int status;
  if (ioctl(link_fd, LIOSTATUS, &status) == -1)
  { ServerDebug("wrrdy: ioctl failed");
    return(0);
  }
  return ((status & LS_OUTPUTREADY) );
}

void gnome_reset_transputer()
{
  int i;

  if (get_config("NO_RESET_TARGET") != NULL)
  	return;

  ioctl(link_fd, LIOCANALYSE, 0);
  ioctl(link_fd, LIOSLRESET, 0);
  for (i=0; i<RESETDELAY; ++i); /* Wait a while - support for AB1 prototype */  
  ioctl(link_fd, LIOCLRESET, 0);
}

void gnome_analyse_transputer()
{
  int i;

  ioctl(link_fd, LIOSANALYSE, 0);
  ioctl(link_fd, LIOSLRESET, 0);
  for (i=0; i<RESETDELAY; ++i); /* Wait a while - support for AB1 prototype */  
  ioctl(link_fd, LIOCLRESET, 0);
  ioctl(link_fd, LIOCANALYSE, 0);
}
