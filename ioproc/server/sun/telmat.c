/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      telmat.c                                                        --
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
*** This code is telmat-board specific. 
*/

/**
**/

#include "telmat.h" 

/**
*** The telmat board is accessed via standard link devices,
*** /dev/telmat0 -> /dev/telmat31, not all of which have to be installed.
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

#define TIMEOUT		10

#define telmat_Max_Link 32

PRIVATE Trans_link telmat_links[telmat_Max_Link];



void telmat_init_link()
{ int i;
  number_of_links = telmat_Max_Link;
  link_table = &(telmat_links[0]);

  for (i = 0; i < telmat_Max_Link; i++){
   sprintf(telmat_links[i].link_name, "/dev/telmat%d",i);
   telmat_links[i].flags      = Link_flags_unused + Link_flags_uninitialised +
                                  Link_flags_firsttime;
   telmat_links[i].connection = -1;
   telmat_links[i].fildes     = -1;
   telmat_links[i].state      = Link_Reset;
   telmat_links[i].ready      = 0;
   }
#if 0
 for ( ; number_of_links >= 0; number_of_links--)
   { struct stat buf;
     if (stat(link_table[number_of_links].link_name, &buf) eq 0)
      break;                   /* OK, found the last known site */
     if (errno ne ENOENT)      /* Appears to exist, but not currently usable */
      break;
   }  
#endif   
#if tgo
printf("init ok \n\r");
#endif
  
}


void telmat_reset_transputer()
{
int mask;
#if tgo
printf("reset trp \n\r");
#endif

  if (ioctl(link_fd,VXV_RESET,&mask) eq -1)
    ServerDebug("Warning, failed to reset site !");

   mask = TIMEOUT;
/* set the read timeout */
  if (ioctl(link_fd, VXV_W_TOR,&mask)== -1 ) 
    ServerDebug("Warning, failed to set read timeout");
/* set the write timeout */
  if (ioctl(link_fd, VXV_W_TOW,&mask)== -1 ) 
    ServerDebug("Warning, failed to set write timeout ");
}

void telmat_analyse_transputer()
{
  int mask;

  if (ioctl(link_fd,VXV_ANALYSE,mask) eq -1)
    ServerDebug("Warning, failed to analyse site");
  mask = TIMEOUT;
  if (ioctl(link_fd, VXV_W_TOR,&mask)== -1 )
    ServerDebug("Warning, failed to set read timeout ");
  if (ioctl(link_fd, VXV_W_TOW,&mask)== -1 )
    ServerDebug("Warning, failed to set write timeout ");
}


int telmat_open_link(tabno)
int tabno;
{ int j;
  char dev[20];
  int mask;
#if tgo
printf("open trp \n\r");
#endif
  
   /* Bug fix, courtesy of Telmat */
  strcpy(dev, link_table[tabno].link_name);

#if tgo
printf("open trp %s\n\r",dev);
#endif
  link_table[tabno].fildes = open(dev, O_RDWR);
#if tgo
printf("open trp %d\n\r",link_table[tabno].fildes);
#endif
  if (link_table[tabno].fildes eq -1)
      return(0);

  if (ioctl(link_table[tabno].fildes,VXV_RESET,&mask) eq -1)
    ServerDebug("Warning, failed to reset site !");

  mask = TIMEOUT;
/* set the read timeout */
  if (ioctl(link_table[tabno].fildes, VXV_W_TOR,&mask)== -1 ) 
    ServerDebug("Warning, failed to set read timeout");
/* set the write timeout */
  if (ioctl(link_table[tabno].fildes, VXV_W_TOW,&mask)== -1 ) 
    ServerDebug("Warning, failed to set write timeout ");

  return(1);
} 

void telmat_free_link(tabno)
int tabno;
{ 
  int mask;  
#if tgo
printf("close trp \n\r");
#endif
  if (link_table[tabno].fildes ne -1){
  	/*if (ioctl(link_table[tabno].fildes,VXV_RESET,&mask) eq -1)
    		ServerDebug("Warning, failed to reset site !"); */
  	close(link_table[tabno].fildes);
   }
  link_table[tabno].fildes = -1;
}

