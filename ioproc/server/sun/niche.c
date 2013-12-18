/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      niche.c                                                       --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: niche.c,v 1.9 1994/06/29 13:46:19 tony Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.        			*/

#define Linklib_Module

#include "../helios.h"

#include <sys/ioccom.h>

#if SOLARIS
#include <unistd.h>
#endif

#define link_fd (link_table[current_link].fildes)

/**
*** This code is Niche-board specific. The manifests have
*** been taken from the header file niche/nadriver.h.
**/

#ifdef NEVER
#include <niche/nadriver.h>
#else

#if (__GNUC__ > 0)
# define QUOTED_LETTER
#endif

#if SOLARIS
# define QUOTED_LETTER
#endif

#ifdef QUOTED_LETTER

#define SITE_RESET           _IO('n', 0)
#define DISABLE_ERRORS       _IO('n', 8)
#define SITE_ANALYSE         _IO('n', 1)
#define SITE_SUN0_SSDOWN     0x00000000
#define SITE_SUBSYSTEM_CTL   _IOW('n', 6, int)

#undef QUOTED_LETTER
#else

#define SITE_RESET           _IO(n, 0)
#define DISABLE_ERRORS       _IO(n, 8)
#define SITE_ANALYSE         _IO(n, 1)
#define SITE_SUN0_SSDOWN     0x00000000
#define SITE_SUBSYSTEM_CTL   _IOW(n, 6, int)
#endif /* SOLARIS */

#endif

/**
*** The transtech board is accessed via standard link devices,
*** /dev/nap0 -> /dev/nap31, not all of which have to be installed.
*** Because a standard device driver is used, reading and writing
*** the link can be done using unix read() and write(), without
*** problems. There is no ioctl for rdrdy() or wrrdy(), so rdrdy()
*** is done by a select() call and wrrdy() is a no-op. Reset and
*** analyse is done by ioctls.
***
*** The code defines a static table for the 32 links. niche_init_link()
*** specifies 32 link adapters, sets the link_table pointer, and
*** initialises the entries. niche_open_link() opens the
*** appropriate link device, stores the file descriptor in the
*** table, and sets the link adapter to a sensible state.
*** niche_free_link() closes the file descriptor. The reset and
*** and analyse routines do straightforwards ioctl calls.
**/

#define Niche_Max_Link 32

PRIVATE Trans_link niche_links[Niche_Max_Link];

void niche_init_link()
{ int i;
  number_of_links = Niche_Max_Link;
  link_table = &(niche_links[0]);
  for (i = 0; i < Niche_Max_Link; i++){
   sprintf(niche_links[i].link_name, "/dev/nap%d",i);

   niche_links[i].flags      = Link_flags_unused + Link_flags_uninitialised +
                                  Link_flags_firsttime;

   if (!mystrcmp(get_config("box"), "TB400"))
    niche_links[i].flags    |= Link_flags_not_selectable;

   niche_links[i].connection = -1;
   niche_links[i].fildes     = -1;
   niche_links[i].state      = Link_Reset;
   niche_links[i].ready      = 0;
   }

  for ( ; number_of_links >= 0; number_of_links--)
   { struct stat buf;
     if (stat(link_table[number_of_links-1].link_name, &buf) eq 0)
      break;                   /* OK, found the last known site */
     if (errno ne ENOENT)      /* Appears to exist, but not currently usable */
      break;
   }       
}

#if ANSI_prototypes
int niche_open_link (int tabno)
#else
int niche_open_link(tabno)
int tabno;
#endif
{
  int j;
  char dev[20];
  int subsystem_mode = SITE_SUN0_SSDOWN;

  /* Keep Solaris C++ compiler happy */
  dev[0] = '\0';

   /* Bug fix, courtesy of Telmat */
  strcpy(dev, link_table[tabno].link_name);

  link_table[tabno].fildes = open(dev, O_RDWR);
  if (link_table[tabno].fildes eq -1)
      return(0);

           /* set up subsystem control */
  if (mystrcmp(get_config("box"), "TB400"))
   if (link_table[tabno].flags & Link_flags_uninitialised)
   {   /* change from /dev/nap to /dev/nas */
     dev[7] = 's';
     j = open(dev, O_RDWR);
     if (j < 0)
        return(0);

     if (ioctl(j, SITE_SUBSYSTEM_CTL, &subsystem_mode) eq -1)
      ServerDebug("Warning : error in subsystem control for site %s", dev);
     close(j);
   }        

  fcntl(link_table[tabno].fildes, F_SETFD, 1);
  ioctl(link_table[tabno].fildes, SITE_RESET);
  ioctl(link_table[tabno].fildes, DISABLE_ERRORS);
  return(1);
} 

void niche_free_link(tabno)
int tabno;
{ if (link_table[tabno].fildes ne -1)
   { /* ioctl(link_table[tabno].fildes, SITE_RESET); */
     close(link_table[tabno].fildes);
   }
  link_table[tabno].fildes = -1;
}

void niche_reset_transputer()
{
  if (ioctl(link_fd,SITE_RESET) eq -1)
    ServerDebug("Warning, failed to reset site");
  ioctl(link_fd, DISABLE_ERRORS);
}

void niche_analyse_transputer()
{
  if (ioctl(link_fd,SITE_ANALYSE,0) eq -1)
  if (ioctl(link_fd, DISABLE_ERRORS) eq -1)
    ServerDebug("Warning, failed to analyse site");
}



