/* $Header:$ */
/* $Source:$ */

/*----------------------------------------------------------------------*/
/*                                                     source/pcoord.c  */
/*----------------------------------------------------------------------*/

/* This program to print digitiser events as they are generated: The    */
/*   program also resets the digitiser scaling and calibration          */
/*   co-efficient to the identity before starting                       */

/*----------------------------------------------------------------------*/
/*                                                        Header Files  */
/*----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nonansi.h>
#include <attrib.h>
#include <syslib.h>
#include <helios.h>
#include <math.h>
#include <ioevents.h>
#include <gsp.h>

#include "microlink/digitiser.h"

/*----------------------------------------------------------------------*/
/*                                                    Global variables  */
/*----------------------------------------------------------------------*/

Stream *microlink;
Port eventPort;

/*----------------------------------------------------------------------*/
/*                                                The Microlink Device  */
/*----------------------------------------------------------------------*/

# define MICROLINK_DEVICE "/microlink/digitiser"

/*----------------------------------------------------------------------*/
/*                                             Look-Ahead Declarations  */
/*----------------------------------------------------------------------*/

int openMicrolink(void);

/*----------------------------------------------------------------------*/
/*                                                           main(...)  */
/*----------------------------------------------------------------------*/

int main(int argc, char *argv[])
{ 
  DigitiserInfo info;
  BYTE    data[IOCDataMax];
  IOEvent *event;
  MCB     message;
  word         rc;
  
  if (!openMicrolink())
    { fprintf(stderr, "%s; failed to open microlink\n", argv[0]);
      exit(2);
    }

  GetInfo(microlink,(void*)&info);
  info.cXX = 1;
  info.cXY = 0;
  info.cX1 = 0;
  info.cYX = 0;
  info.cYY = 1;
  info.cY1 = 0;
  info.denom = 1;
  SetInfo(microlink,(void*)&info,sizeof(DigitiserInfo));

  Close(microlink);

  while(1)
  {  int got,eventsGot,j;
     message.Data        = &data[0];
     message.MsgHdr.Dest = eventPort;
     message.Timeout     = -1;

     rc = GetMsg(&message);
     if (rc < 0)
     {  printf("GetMsg failed %x\n", (int)rc);
        break; /* Assume it was timeout! */
     }

     got = message.MsgHdr.DataSize;
     eventsGot = got/sizeof(IOEvent);
     event = (IOEvent *)data;

     for (j = 0; j < eventsGot; ++j, ++event)
     {  printf
        (  "(%4d,%4d) : %08lX\n",
           event->Device.Stylus.X,
           event->Device.Stylus.Y,
           event->Device.Stylus.Buttons
        );
     }
  }
  
  return 0;
}

/*----------------------------------------------------------------------*/
/*                                                  openMicrolink(...)  */
/*----------------------------------------------------------------------*/

int openMicrolink(void)
{ 
  Object *obj;

  /* Open() doesn't seem to work with a NULL context, so we have to do	*/
  /* a Locate() first.							*/

  obj = Locate(CurrentDir, MICROLINK_DEVICE);
  if (obj == NULL) 
    { fprintf(stderr,"Can't locate microlink\n");
      return 0;
    }

  microlink = Open(obj, NULL, O_ReadWrite);
  if (microlink == 0) 
    { fprintf(stderr, "Can't open microlink\n");
      return 0;
    }

  eventPort = EnableEvents(microlink, 1);
  if (eventPort == NullPort)
  {
    printf("EnableEvents on microlink stream failed\n");
    Close(microlink);
    return 0;
  }  
  printf
  ("Succeeded in opening microlink. Port number is 0x%08X\n",(int)eventPort);
  return 1;
}

