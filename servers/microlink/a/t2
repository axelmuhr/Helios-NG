/* $Header: test2.c,v 1.2 91/01/31 13:51:47 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/source/test/test2.c,v $ */

/*------------------------------------------------------------------------*/
/*                                         microlink/source/test/test2.c  */
/*------------------------------------------------------------------------*/

/* This test program tests the events mechanism of the digitiser server   */
/*   contained within the microlink server.                               */

/*------------------------------------------------------------------------*/
/*                                                          Header files  */
/*------------------------------------------------------------------------*/

#include <helios.h>
#include <string.h>
#include <codes.h>
#include <stddef.h>
#include <stdlib.h>
#include <syslib.h>
#include <task.h>
#include <message.h>
#include <attrib.h>
#include <stdio.h>
#include <process.h>
#include <ioevents.h>
#include <gsp.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "drawp/public.h"

/*------------------------------------------------------------------------*/
/*                                               Look-ahead declarations  */
/*------------------------------------------------------------------------*/

void drawArrow(DpPixmap_t *sc,DpGraphicsContext_t *gc,int x, int y);

/*------------------------------------------------------------------------*/
/*                                                        Pointer buffer  */
/*------------------------------------------------------------------------*/

typedef struct Pointer
{  int val;
   int x,y;
} Pointer;

#define BufSize 20

/*------------------------------------------------------------------------*/
/*                                                             main(...)  */
/*------------------------------------------------------------------------*/

int main()
{  DpPixmap_t               *sc;
   DpGraphicsContext_t     *gc0;
   DpGraphicsContext_t     *gc1;
   DpGraphicsContext_t     *gcx;
   Object                  *dgo;
   Stream                  *dgs;
   IOEvent                  *ev;
   MCB                      ecb;
   Port                     evp;
   byte         evb[IOCDataMax];
   word                     res;
   static Pointer back[BufSize];
   int                      pos;

   sc  = dpMapScreen   ( NULL,0         );
   gc0 = dpConstructGC ( NULL,sc->depth ); gc0->foreground = 0x00000000;
   gc1 = dpConstructGC ( NULL,sc->depth ); gc1->foreground = 0xFFFFFFFF;
   gcx = dpConstructGC ( NULL,sc->depth ); gcx->foreground = 0xFFFFFFFF;
   gcx->function = GXxor;
   dpFillRectangle(sc,gc0,0,0,sc->sizeX,sc->sizeY);

   dgo = Locate(CurrentDir,"/microlink/digitiser");
   if(dgo==NULL)
   {  printf("Failed to locate digitiser object\n");
      return 1;
   }
   
   dgs = Open(dgo,NULL,O_ReadOnly);
   if(dgs==NULL)
   {  printf("Failed to open digitiser object\n");
      return 1;
   }
   
   evp = EnableEvents(dgs,1);
   if(evp==NullPort)
   {  fprintf(stderr,"Failed to enable events\n");
      exit(1);
   }
   
   for(pos=0;pos<BufSize;pos++) back[pos].val = 0;
   pos = 0;
      
   for(;;)
   {  ecb.MsgHdr.Dest =  evp;
      ecb.Data        =  evb;
      ecb.Control     = NULL;
      ecb.Timeout     =   -1;
      res = GetMsg(&ecb);
      if(res<0)
      {  fprintf
         ( stderr,"GetMsg failed waiting for event. Code 0x%08X\n",res );
         exit(1);
      }
      for(ev=(IOEvent*)evb;(byte*)ev-evb<ecb.MsgHdr.DataSize;ev++)
      {  if(back[pos].val) drawArrow(sc,gcx,back[pos].x,back[pos].y);
         back[pos].x = ev->Device.Stylus.X;
         back[pos].y = ev->Device.Stylus.Y;
         back[pos].val = 1;
         drawArrow(sc,gcx,back[pos].x,back[pos].y);
         if(++pos>=BufSize) pos=0;
      }
      Delay(10000);
   }
}

/*------------------------------------------------------------------------*/
/*                                                        drawArrow(...)  */
/*------------------------------------------------------------------------*/

void drawArrow(DpPixmap_t *sc,DpGraphicsContext_t *gc,int x, int y)
{  
   dpDrawLine(sc,gc,x,y,x+10,y);
   dpDrawLine(sc,gc,x,y,x,y+10);
   dpDrawLine(sc,gc,x,y,x+10,y+10);
}
