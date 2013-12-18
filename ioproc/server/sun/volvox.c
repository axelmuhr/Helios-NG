/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      volvox.c                                                        --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.9 20/4/90\ Copyright (C) 1989, Perihelion Software Ltd.        */

#define Linklib_Module


#include "../helios.h"

#if SOLARIS
#include <sys/ioccom.h>
#include <unistd.h>
#endif

#define link_fd (link_table[current_link].fildes)


/**
*** This code is Volvox-board specific. 
*/

/**
**/

#include "transputer.h" 

/**
*** The archipel board is accessed via standard link devices,
*** /dev/vxv0 -> /dev/vxv31, not all of which have to be installed.
*** Because a standard device driver is used, reading and writing
*** the link can be done using unix read() and write(), without
*** problems. There is no ioctl for rdrdy() or wrrdy(), so rdrdy()
*** is done by a select() call and wrrdy() is a no-op. Reset and
 *** analyse is done by ioctls.
***
*** This code supports multiple link adapters at any one time. In the
*** Server at present only one link adapter is used, but in the
*** link daemon upto 32 links are available. Details of all the links
*** are stored in an array link_table[], and the integer current_link
*** specifies which link is currently being accessed. This provides
*** upwards compatibility with e.g. a PC, where there is only ever one
*** link adapter. The macro link_fd expands to link_table[current_link].fildes,
*** i.e. a descriptor for the current link.
**/ 

#define RESET_MASK	0x10
#define ANALYSE_MASK	0x10
	/* BLV - timeout for read and write operations. Used to be 500 */
#define TIMEOUT		50

#define Volvox_Max_Link 32

PRIVATE Trans_link volvox_links[Volvox_Max_Link];


void volvox_init_link()
{ int i;
  number_of_links = Volvox_Max_Link;
  link_table = &(volvox_links[0]);

  for (i = 0; i < Volvox_Max_Link; i++){
   sprintf(volvox_links[i].link_name, "/dev/vxv%d",i);
   volvox_links[i].flags      = Link_flags_unused + Link_flags_uninitialised +
                                  Link_flags_firsttime;
   volvox_links[i].connection = -1;
   volvox_links[i].fildes     = -1;
   volvox_links[i].state      = Link_Reset;
   volvox_links[i].ready      = 0;
 }

  for ( ; number_of_links >= 0; number_of_links--)
   { struct stat buf;
     if (stat(link_table[number_of_links-1].link_name, &buf) eq 0)
      break;                   /* OK, found the last known site */
     if (errno ne ENOENT)      /* Appears to exist, but not currently usable */
      break;
   }       

  if (number_of_links == -1)
    {
      ServerDebug ("Warning, failed to find any volvox devices");

      longjmp_exit;
    }
}


void volvox_reset_transputer()
{
  int mask;

  mask = RESET_MASK;
  if (ioctl(link_fd,VXV_RESET,&mask) eq -1)
    ServerDebug("Warning, failed to reset site !");
}

void volvox_analyse_transputer()
{
  int mask;

  mask = ANALYSE_MASK;
  if (ioctl(link_fd,VXV_ANALYSE,mask) eq -1)
    ServerDebug("Warning, failed to analyse site");
}


int volvox_open_link(tabno)
int tabno;
{ char dev[20];
  int mask;

  dev[0] = '\0';
  
   /* Bug fix, courtesy of Telmat */
  strcpy(dev, link_table[tabno].link_name);

  link_table[tabno].fildes = open(dev, O_RDWR);
  if (link_table[tabno].fildes eq -1)
    {
      ServerDebug ("Warning, failed to open %s", link_table[tabno].link_name);
      return(0);
    }

  mask = RESET_MASK;
  if (ioctl(link_table[tabno].fildes,VXV_RESET,&mask) eq -1)
    {
      /* ServerDebug("Warning, failed to reset site !"); */
      printf ("Warning, failed to reset site !\n");
    }

  mask = TIMEOUT;
  if (ioctl(link_table[tabno].fildes, VXV_W_TOR,&mask)== -1 ) 
    {
      ServerDebug("Warning, failed to set read timeout");
    }

  if (ioctl(link_table[tabno].fildes, VXV_W_TOW,&mask)== -1 ) 
    {
      ServerDebug("Warning, failed to set write timeout");
    }

  return(1);
} 

void volvox_free_link(tabno)
int tabno;
{ 
  int mask;  
  if (link_table[tabno].fildes ne -1){
#if 0
  mask = RESET_MASK;
  if (ioctl(link_table[tabno].fildes,VXV_RESET,&mask) eq -1)
    ServerDebug("Warning, failed to reset site !");
#endif
  	close(link_table[tabno].fildes);
   }
  link_table[tabno].fildes = -1;
}

