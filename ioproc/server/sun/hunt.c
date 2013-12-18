/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      hunt.c                                                       --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: hunt.c,v 1.5 1994/06/29 13:46:19 tony Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.        			*/

#define Linklib_Module

#include "../helios.h"

#include <sys/ioccom.h>

#if SOLARIS
#include <unistd.h>
#endif

#define link_fd (link_table[current_link].fildes)

#ifdef NEVER
#include <sys/hevio.h>
#else
#if (__GNUC__ > 0)
# define QUOTED_LETTER
#endif

#if SOLARIS
# define QUOTED_LETTER
#endif

#ifdef QUOTED_LETTER

#define HEVRST               _IO('k',1)
#define HEVRDCONF            _IOR('k',2,int)
#define HEV_GET_READREADY    _IOR('k',3,int)
#define HEV_GET_WRITEREADY   _IOR('k',4,int)
#define HEV_GET_XFER_GRAIN   _IOR('k',5,int)
#define HEV_SET_BYTEORDER    _IOW('k',6,int)

#undef QUOTED_LETTER

#else

#define HEVRST               _IO(k,1)
#define HEVRDCONF            _IOR(k,2,int)
#define HEV_GET_READREADY    _IOR(k,3,int)
#define HEV_GET_WRITEREADY   _IOR(k,4,int)
#define HEV_GET_XFER_GRAIN   _IOR(k,5,int)
#define HEV_SET_BYTEORDER    _IOW(k,6,int)

#endif /* QUOTED_LETTER */

#endif /* NEVER */

/**
*** The Hunt boards are accessed via standard link devices,
*** /dev/hesb40 and /dev/hev40, with only device per machine.
*** Because a standard device driver is used, reading and writing
*** the link can be done using unix read() and write(), without
*** problems. 
**/

#define Hunt_Max_Link 4

PRIVATE Trans_link hunt_links[Hunt_Max_Link];

void hunt_init_link()
{ int i;
  char *box = get_config("box");
  char  devbase[8];

  number_of_links = Hunt_Max_Link;
  link_table = &(hunt_links[0]);

  if (!mystrcmp(box, "HEV40"))
   strcpy(devbase, "hev");
  else
   strcpy(devbase, "hes");
   
  for (i = 0; i < Hunt_Max_Link; i++)
   { sprintf(hunt_links[i].link_name, "/dev/%s%d", devbase, i);
  
     hunt_links[i].flags      = Link_flags_unused + Link_flags_uninitialised +
                                  Link_flags_firsttime + Link_flags_not_selectable +
				  Link_flags_word;
     hunt_links[i].connection = -1;
     hunt_links[i].fildes     = -1;
     hunt_links[i].state      = Link_Reset;
     hunt_links[i].ready      = 0;
   }

  for ( ; number_of_links >= 0; number_of_links--)
   { struct stat buf;
     if (stat(link_table[number_of_links-1].link_name, &buf) eq 0)
      break;                   /* OK, found the last known site */
     if (errno ne ENOENT)      /* Appears to exist, but not currently usable */
      break;
   }
}

int hunt_open_link(tabno)
int tabno;
{ int j;

  link_table[tabno].fildes = open(link_table[tabno].link_name, O_RDWR);
  if (link_table[tabno].fildes eq -1)
    {
      ServerDebug ("failed to open %s", link_table[tabno].link_name);
      return(0);
    }
  fcntl(link_table[tabno].fildes, F_SETFD, 1);
  j = 0;
  ioctl(link_table[tabno].fildes, HEV_SET_BYTEORDER, &j);
  ioctl(link_table[tabno].fildes, HEVRST);

  return(1);
} 

void hunt_free_link(tabno)
int tabno;
{ if (link_table[tabno].fildes ne -1)
   close(link_table[tabno].fildes);
  link_table[tabno].fildes = -1;
}

void hunt_reset_c40()
{
  if (ioctl(link_fd,HEVRST) eq -1)
   ServerDebug("Warning, failed to reset site");
}

int hunt_rdrdy( )
{
   int res = 0;
#ifdef SOLARIS

#define MAX_OFFSET	0x8000
   /*
    * Due to a bug in Solaris, if the device driver attempts to do more than
    * 2GBytes of sequential I/O on the device, it will fail with EINVAL because
    * the file offset cannot grow beyond 2GBytes.  This is true even if the
    * offset is completely irrelevant.  The fix for this is to do a periodic
    * lseek () to reset the offset to 0.
    */
   if (lseek (link_fd, 0, SEEK_CUR) > MAX_OFFSET)
     {
       lseek (link_fd, 0, SEEK_SET);
     }
#endif
      
   if (ioctl(link_fd,HEV_GET_READREADY, &res) eq -1)
    ServerDebug("Warning, hunt_rdrdy");

  return res; 
}

int hunt_wrrdy( )
{
  int res = 0;
   if (ioctl(link_fd,HEV_GET_WRITEREADY, &res) eq -1)
    ServerDebug("Warning, hunt_wrrdy");

  return res; 
}



