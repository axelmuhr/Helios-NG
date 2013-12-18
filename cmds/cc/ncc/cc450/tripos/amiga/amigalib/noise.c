#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "exec/types.h"
#include "exec/exec.h"
#include "exec/resident.h"
#include "exec/semaphores.h"

#include "graphics/gels.h"
#include "graphics/regions.h"

#include "devices/keymap.h"
#include "intuition/intuition.h"

#include "functions/intuitionfns.h"
#include "functions/execfns.h"
#include "functions/support.h"

#include "devices/audio.h"

struct NewWindow nw  = {
   100, 25,
   440, 150,
   1, 2,
   CLOSEWINDOW|MOUSEMOVE|MOUSEBUTTONS|NEWSIZE,
   WINDOWSIZING|WINDOWDRAG|WINDOWDEPTH|WINDOWCLOSE|SMART_REFRESH|ACTIVATE,
   NULL,
   NULL,
   (UBYTE *)"Noise Window",
   NULL,
   NULL,
   10, 10,
   1000, 1000,
   WBENCHSCREEN
};

struct Window *w;

#define LEFT0F 1
#define RIGHT0F 2
#define RIGHT1F 4
#define LEFT1F 8

struct IOAudio *audioreq;
struct IOAudio *modifyreq;
struct MsgPort *nport;
UBYTE amap[] = { RIGHT0F|RIGHT1F, LEFT1F, RIGHT0F, RIGHT1F };
UBYTE noisedata[] = { -128, 127 };
UBYTE *chipnoisedata;
double freqbase;
double powerbase;
int *freqtable = NULL;

double log10of2;

#define K 3579545
#define PERIOD(f,l) (K/(f*l))
#define MAXPERIODK 65535
#define MINPERIODK 127
#define MAXVOLUME 63

void start_noise(void)
{
   nport = CreatePort(NULL,0);

   chipnoisedata = (UBYTE *)AllocMem(sizeof(noisedata), MEMF_CHIP);
   memcpy(chipnoisedata, noisedata, sizeof(noisedata));

   audioreq = (struct IOAudio *)CreateExtIO(nport, sizeof(*audioreq)*2);

   modifyreq = audioreq+1;

   audioreq->ioa_Data     = amap;
   audioreq->ioa_Length   = sizeof(amap);
   OpenDevice(AUDIONAME, 0, &(audioreq->ioa_Request), 0);

   audioreq->ioa_Request.io_Command = CMD_WRITE;
   audioreq->ioa_Request.io_Message.mn_Length = sizeof(audioreq);

   audioreq->ioa_Data     = chipnoisedata;
   audioreq->ioa_Length   = sizeof(noisedata);
   audioreq->ioa_Period   = PERIOD(440, sizeof(noisedata));
   audioreq->ioa_Volume   = 40;
   audioreq->ioa_Request.io_Flags = ADIOF_PERVOL;
   audioreq->ioa_Cycles   = 0;    /* Infinity */

   memcpy(modifyreq, audioreq, sizeof(*audioreq));
   modifyreq->ioa_Request.io_Command = ADCMD_PERVOL;

   BeginIO(&(audioreq->ioa_Request));
}

void modifynoise(UWORD period, UWORD volume)
{
   modifyreq->ioa_Period = period;
   modifyreq->ioa_Volume = volume;
   BeginIO(&(modifyreq->ioa_Request));
   WaitIO(&(modifyreq->ioa_Request));
}

void stop_noise(void)
{
   AbortIO(&(audioreq->ioa_Request));
   FreeMem(chipnoisedata, sizeof(noisedata) );
   CloseDevice(&(audioreq->ioa_Request));
   DeleteExtIO(&(audioreq->ioa_Request), sizeof(*audioreq)*2);
   DeletePort(nport);
}

#define log2(x) (log10(x)/log10of2)

void makefreqtable(int size)
{  int i;
   double floatwidth = (float)size;

   if( freqtable ) free(freqtable);
   freqtable = (int *)malloc(size * sizeof(*freqtable) );

   for( i = 0; i < size; i++ )
      freqtable[i] = (freqbase * pow(powerbase, ((double)i)/floatwidth));
}

void mytidyup(void)
{
   stop_noise();
   free(freqtable);
   CloseWindow(w);
}

int main(int argc, char *argv[])
{
   double maxp = (double)MAXPERIODK*sizeof(noisedata)/K;
   double minp = (double)MINPERIODK*sizeof(noisedata)/K;
   double octaves;

#ifdef DEBUG
   printf("maxp, minp = %f, %f\n",maxp, minp);
#endif
   atexit( mytidyup );

   IntuitionBase = OpenLibrary("intuition.library",0);

   log10of2 = log10(2.);
   octaves  = log2(maxp/minp);
#ifdef DEBUG
   printf("octaves = %f\n",octaves);
#endif
   powerbase = pow(2, octaves);

   freqbase = 1./maxp;
#ifdef DEBUG
   printf("freqbase = %f\n",freqbase);
#endif
#ifdef DEBUG
   printf("maxfreq = %f\n",(double)(1.0)/minp);
#endif

   w = (struct Window *)OpenWindow(&nw);
   printf("Wait for it!\n");
   makefreqtable(w->Width);
   printf("Ok go\n");

   start_noise();

   while(1)
   {
      struct IntuiMessage *m;
      ULONG class;
      USHORT code;
      SHORT mousex, mousey;
      int exitflag = 0;

      WaitPort(w->UserPort);
      m = (struct IntuiMessage *)GetMsg(w->UserPort);
      class = m->Class;
      code  = m->Code;
      mousex = m->MouseX;
      mousey = m->MouseY;

      ReplyMsg((struct Message *)m);

      switch( class )
      {
      case CLOSEWINDOW :
         exitflag = 1;
         break;

      case NEWSIZE:
         makefreqtable(w->Width);
         break;

      case MOUSEBUTTONS :
         switch( code )
         {
         case SELECTDOWN:
            ReportMouse(w,1);
            break;
         case SELECTUP:
            ReportMouse(w,0);
            continue;
         case MENUDOWN:
         case MENUUP:
            continue;
         }
 /* Fall through on SELECTDOWN */
      case MOUSEMOVE:
         {  double newfreq;
            int newp;

            if( mousex > w->Width ) mousex = w->Width;
            else if( mousex < 0 ) mousex = 0;
            if( mousey > w->Height ) mousey = w->Height;
            else if( mousey < 0 ) mousey = 0;

            newfreq = freqtable[mousex];
#ifdef DEBUG
            printf("newfreq = %f\n",newfreq);
#endif
            newp = (int)PERIOD(newfreq, sizeof(noisedata));
#ifdef DEBUG
            printf("newp = %d\n",newp);
#endif
            modifynoise( newp, (mousey*MAXVOLUME)/w->Height);
         }
         break;
      }
      if( exitflag ) break;
   }

   return( 0 );
}
