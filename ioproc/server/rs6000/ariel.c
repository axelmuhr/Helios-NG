/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1993, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      ariel.c                                                       --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: hunt.c,v 1.1 1993/01/06 12:31:09 bart Exp $ */
/* Copyright (C) 1993, Perihelion Software Ltd.        			*/

#define Linklib_Module

#include "../helios.h"

#define link_fd (link_table[current_link].fildes)

PRIVATE Trans_link Ariel_link;

void arielhydra_init_link()
{
  number_of_links = 1;
  link_table = &Ariel_link;

  sprintf(Ariel_link.link_name, "/dev/ariel");
  
  Ariel_link.flags      = Link_flags_unused + Link_flags_uninitialised +
                               Link_flags_firsttime + Link_flags_not_selectable;
  Ariel_link.connection = -1;
  Ariel_link.fildes     = -1;
  Ariel_link.state      = Link_Reset;
  Ariel_link.ready      = 0;
}

int arielhydra_open_link(tabno)
int tabno;
{
  return(0);
} 

void arielhydra_free_link(tabno)
int tabno;
{ 
}

void arielhydra_reset()
{
}

int arielhydra_rdrdy( )
{
}

int arielhydra_wrrdy( )
{
}

int arielhydra_byte_to_link(int x)
{
}

int arielhydra_byte_from_link(UBYTE *x)
{
}

int arielhydra_send_block(int amount, BYTE *data, int timeout)
{
}

int arielhydra_fetch_block(int amount, BYTE *data, int timeout)
{
}




