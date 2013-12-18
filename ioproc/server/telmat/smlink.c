/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      smlink.c                                                        --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %I% %G%\ Copyright (C) 1989, Perihelion Software Ltd.        */

/**
*** This is code specific to the sm90 link interface.
**/

/*  New release of the SM90 Helios Server which takes in account the
    semaphores at the boards
*/

/* -------------------- site on TELMAT workstation --------------------------
-
-  site	  ITFTP32	link number	 SM90 Cpu       3TRP      link number
-	  board number			board number  board number
-	 
-   0	     0		/dev/link0_mt0	     0	0	 8     /dev/link8a_mt0
-   1	     8		/dev/link8_mt0	     0	0	 8     /dev/link8b_mt0
-   2	    16		/dev/link16_mt0	     0	0	 8     /dev/link8c_mt0
-   3	    24		/dev/link24_mt0	     0	0	 24    /dev/link24a_mt0
-   4	    32		/dev/link32_mt0	     0	0	 24    /dev/link24b_mt0
-   5	    40		/dev/link40_mt0	     0	0	 24    /dev/link24c_mt0
-   6	    48		/dev/link48_mt0	     0	0	 32    /dev/link32a_mt0
-   7	    56		/dev/link56_mt0	     0	0	 32    /dev/link32b_mt0
-
-   8	     0		/dev/link0_mt1	     1	0	 32    /dev/link32c_mt0
-   9	     8		/dev/link8_mt1	     1	1	 8     /dev/link8a_mt1
-  10	    16		/dev/link16_mt1	     1	1	 8     /dev/link8b_mt1
-  11	    24		/dev/link24_mt1	     1	1	 8     /dev/link8c_mt1
-----------------------------------------------------------------------------*/

#define Linklib_Module

#include "../helios.h"
#include "semaconst.h"

#define SOLEIL_DBG FALSE

#include <sys/trans.h>

#define link_fd (link_table[current_link].fildes)

#define TRP3_Max_Link    9	/* 3TRP board */
#define ITFTP32_Max_Link 8	/* ITFTP32 board */
#define MAX_MT		 4

PRIVATE Trans_link itftp32_links[ITFTP32_Max_Link * MAX_MT];
PRIVATE Trans_link trp3_links[TRP3_Max_Link * MAX_MT];


/******************************* TRP3 BOARD *********************************/


void trp3_init_link()
{ int i;
  int numlink;
  char carlink;

  number_of_links = TRP3_Max_Link * MAX_MT;
  link_table      = &(trp3_links[0]);
  for (i = 0; i < number_of_links ; i++){
	numlink = ( i % 9 ) / 3;
		numlink = (numlink == 0) ? 8*(numlink + 1) : 8*(numlink + 2) ;
		switch (i %3){
			case 0 : carlink = 'a';break;
			case 1 : carlink = 'b';break;
			case 2 : carlink = 'c';break;
		}
		sprintf(link_table[i].link_name, "/dev/link%d%c_mt%d",numlink,carlink,i/9);

		link_table[i].flags      = Link_flags_unused + Link_flags_uninitialised +
                                Link_flags_firsttime;
		link_table[i].connection = -1;
		link_table[i].fildes     = -1;
		link_table[i].state      = Link_Reset;
		link_table[i].ready      = 0;
}

}

void trp3_free_link(tabno)
int tabno;
{ 
	if (link_table[tabno].fildes ne -1){ 
#if 0
		ioctl(link_table[tabno].fildes, LNRESET_ROOT, 0);
#endif
		close(link_table[tabno].fildes);
		}
	trans_sema(link_table[tabno].link_name, RELEASE);
	link_table[tabno].fildes = -1;
}


void trp3_reset_transputer()
{
  ioctl(link_fd, LNRESET_ROOT, 0);
}

void trp3_analyse_transputer()
{
  ioctl(link_fd,LNRESET_ANALYSE,0);
}


int trp3_open_link(tabno)
int tabno;
{ 
  int linksm;
  int numlink;
  int process;

	process=trans_sema(link_table[tabno].link_name,ACQUIRE);
	switch(process)
 		{
		case TRANS_FREE: 
			break;
		case SEMA_ERROR: 
			return (0);
  		case TRANS_UNKNOW: 
			printf("Cannot recognize site %s\n\r",link_table[tabno].link_name);
		  	break;	/* return (0); */
  		default: 
			printf("**** ABORT HELIOS : SITE %d already in use by another process \r\n",tabno);
	       		return (0);
  		}

	link_table[tabno].fildes = open(link_table[tabno].link_name, O_RDWR);
	if (link_table[tabno].fildes eq -1){ 
#if 0
		printf("cannot open link device : %s\r\n",link_table[tabno].link_name);
#endif
		trans_sema(link_table[tabno].link_name,RELEASE);
		return(0);
		}

	fcntl(link_table[tabno].fildes, F_SETFD, 1);
	ioctl(link_table[tabno].fildes, LNRESET_ROOT, 0);

	ioctl(link_table[tabno].fildes, LNSETTIMEOUT, 1000); 
 
	return(1);
}
 

/**
*** for now, a server can only talk to one link
**/

/******************************* ITFTP32 BOARD *********************************/


void itftp32_init_link()
{ int i;

  number_of_links = ITFTP32_Max_Link * MAX_MT;
  link_table      = &(itftp32_links[0]);
  for (i = 0; i < number_of_links ; i++){
	sprintf(link_table[i].link_name, "/dev/link%d_mt%d",(i %8) * 8, i / 8);

	link_table[i].flags      = Link_flags_unused + Link_flags_uninitialised +
                                Link_flags_firsttime;
	link_table[i].connection = -1;
	link_table[i].fildes     = -1;
	link_table[i].state      = Link_Reset;
	link_table[i].ready      = 0;
   }

}


void itftp32_reset_transputer()
{
  ioctl(link_fd,LNRESET_ROOT, 0);
}


void itftp32_analyse_transputer()
{
  ioctl(link_fd,LNRESET_ANALYSE,0);
}




int itftp32_open_link(tabno)
int tabno;
{ 
  int linksm;
  int numlink;
  int process;

	switch (tabno % 8){
		case 	0: linksm = -1;	break;
		case 	1: linksm =  1;	break; 
		case 	2: linksm = -1; break; 
		case 	3: linksm =  3;	break; 
		case 	4: linksm =  4;	break; 
		case 	5: linksm = -1;	break; 
		case 	6: linksm = -1;	break; 
		case 	7: linksm = -1;	break; 
		default	 : linksm = -1;
	}

	if (linksm == -1) {
#if 0
		printf("Invalid site number : %d\n\r",tabno);
#endif
		}

  	if ( linksm != -1 ){
  		process=trans_sema(link_table[tabno].link_name,ACQUIRE);
		switch(process)
  		{
			case TRANS_FREE: 
				break;
			case SEMA_ERROR: 
				return (0);
	  		case TRANS_UNKNOW: 
				printf("Cannot recognize site %s\n\r",link_table[tabno].link_name);
			  	break;	/* return (0); */
	  		default: 
				printf("**** ABORT HELIOS : SITE %d already in use by another process \r\n",tabno);
	          		return (0);
  		}
	}
	else return(0);

	link_table[tabno].fildes = open(link_table[tabno].link_name, O_RDWR);
	if (link_table[tabno].fildes eq -1){ 
#if 0
		printf("cannot open link device : %s\r\n",link_table[tabno].link_name);
#endif
		trans_sema(link_table[tabno].link_name,RELEASE);
		return(0);
		}

	fcntl(link_table[tabno].fildes, F_SETFD, 1);
	ioctl(link_table[tabno].fildes, LNRESET_ROOT, 0);

	ioctl(link_table[tabno].fildes, LNSETTIMEOUT, 1000); 

	return(1);
}
 
void itftp32_free_link(tabno)
int tabno;
{ 
	if (link_table[tabno].fildes ne -1){ 
#if 0
		ioctl(link_table[tabno].fildes, LNRESET_ROOT, 0);
#endif
		close(link_table[tabno].fildes);
		}
	trans_sema(link_table[tabno].link_name, RELEASE);
	link_table[tabno].fildes = -1;
}


void fatal()
{ printf("Bad news : routine fatal() has been called\r\n");
}

