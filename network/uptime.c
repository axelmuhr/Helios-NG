/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- uptime.c								--
--		Give a message saying how long the network has been up	--
-- 	        (probably anyway, it is difficult to be certain)	--
--                                                                      --
--	Author:  BLV 2/9/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/uptime.c,v 1.4 1993/12/20 13:38:28 nickc Exp $*/

#include <stdio.h>
#include <syslib.h>
#include <time.h>

/**
*** The current algorithm locates the error logger device, part of the
*** I/O Server. This gives an accurate time stamp for when that
*** I/O Server came up. If the I/O Server is the one that booted the
*** root processor everything is fine. Otherwise the value may be silly.
BLV Think of a better algorithm.
**/

int main(void)
{ ObjInfo info;
  Object  *o;
  time_t  now;
  word    uptime, hours, minutes, seconds;
  
  now = time((time_t *) NULL);
  
  o = Locate(Null(Object), "/logger");
  if (o == Null(Object))
   { fprintf(stderr, "Error : failed to locate /logger.\n");
     return(1);
   }
   
  ObjectInfo(o, Null(char), (byte *) &info);
  
  uptime  = (word) now - info.Dates.Creation;
  seconds = uptime % 60; uptime /= 60;
  minutes = uptime % 60; uptime /= 60;
  hours   = uptime % 24; uptime /= 24;

  printf("%.24s : up %ld days, %2ld:%02ld.%02ld.\n", ctime(&now),
         uptime, hours, minutes, seconds);
  return(0);	
}
