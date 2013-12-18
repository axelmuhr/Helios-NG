/**
*** mandel.c
***		This program uses the farm library to calculate the
***		Mandelbrot set and display it on an X terminal.
**/
#include <helios.h>
#include <sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <farmlib.h>
#include "mandel.h"

static void	mandel_controlInitialise(void);
static void	mandel_producer(void);
static void	mandel_consumer(void);

int main(int argc, char **argv)
{
  FmControllerInitialise	= &mandel_controlInitialise;
  FmProducer			= &mandel_producer;
  FmConsumer			= &mandel_consumer;
  FmConsumerStack		= 5000;
  FmWorker			= &Mandel_Worker;
  FmFastStack			= TRUE;
  FmFastCode			= TRUE;
  FmJobSize			= sizeof(Mandel_Job);
  FmInitialise();
}

/**----------------------------------------------------------------------------
*** controller initialisation. This routine is responsible for connecting to
*** the X server and setting up the colour information.
**/
static Mandel_ColourInfo	Colours;
static Mandel_ScreenInfo	Screen;
static Semaphore		ScreenReady;

static void mandel_controlInitialise(void)
{ unsigned int	width, height;
  unsigned char	colours[8];

  InitSemaphore(&ScreenReady, 1);
  width = 640; height = 400;

  if (!x_initialise(&width, &height, colours))
   { fprintf(stderr, "Mandel: failed to initialise X interface.\n");
     exit(EXIT_FAILURE);
   }

  atexit(&x_finish);
  
  Colours.Red			= colours[0];
  Colours.Orange		= colours[1];
  Colours.Yellow		= colours[2];
  Colours.Green			= colours[3];
  Colours.Blue			= colours[4];
  Colours.Indigo		= colours[5];
  Colours.Violet		= colours[6];
  Colours.Black			= colours[7];

  Screen.Y_Size			= height;
  Screen.X_Size			= width;
  Screen.BottomLeft_Real	= -2.25;
  Screen.BottomLeft_Imag	= -1.8;
  Screen.TopRight_Real		= 0.75;
  Screen.TopRight_Imag		= 1.5;
}

/**----------------------------------------------------------------------------
*** The producer routine is responsible for generating packets that can
*** be sent out to the workers. First it sends out a packet defining
*** the current colour information, which was determined by the control
*** initialise routine and is fixed for the entire run of the application.
*** Then it enters a loop calculating screen after screen of Mandelbrot
*** pictures, the details being decided by the user via the consumer routine.
*** For every screen the routine waits for the current screen details to
*** be ready (initialised by ControlInitialise and reset by the consumer),
*** sends the current screen information to all workers and then sends
*** out a separate job packet for every scanline.
**/
static void mandel_producer(void)
{
  { Mandel_ColourInfo *info = (Mandel_ColourInfo *) FmGetJobBuffer(sizeof(Mandel_ColourInfo));
    *info = Colours;
    info->Type = Request_ColourInfo;
    FmSendJob(Fm_All, FALSE, info);
  }

  forever
   { Mandel_ScreenInfo	*screen_info;
     Mandel_Job		*job;
     int		i;

     Wait(&ScreenReady);
     screen_info  = (Mandel_ScreenInfo *) FmGetJobBuffer(sizeof(Mandel_ScreenInfo));
     *screen_info = Screen;
     screen_info->Type = Request_ScreenInfo;
     FmSendJob(Fm_All, FALSE, screen_info);

	/* produce jobs for the scanlines, starting at the top */
     for (i = Screen.Y_Size - 1; i >= 0; i--)
      { 
	job          = (Mandel_Job *) FmGetJobBuffer(sizeof(Mandel_Job));
        job->Type    = Request_Job;
        job->Y_Coord = i;
        FmSendJob(Fm_Any, TRUE, job);
      }
   }
}

/**----------------------------------------------------------------------------
*** The consumer routine is responsible for accepting reply packets
*** calculated by the workers and doing useful things with them, for
*** example writing data to the screen.
**/
static void	mandel_consumer(void)
{ unsigned int	tl_x, tl_y, br_x, br_y;
  int		result;

  forever
   {	/* 1) get all the scanlines */
     int		i;
     Mandel_Reply	*reply;

     for (i = 0; i < Screen.Y_Size; i++)
      { 
	reply = FmGetReply(Fm_Any);
        x_draw_scanline(Screen.Y_Size - (reply->Y_Coord + 1), reply->Scanline);
      }

     result = x_get_new_coords(&tl_x, &tl_y, &br_x, &br_y);
     tl_y = Screen.Y_Size - (tl_y + 1);
     br_y = Screen.Y_Size - (br_y + 1);

     if (result == -1)
      exit(0);
     elif (result == 0)
      { Screen.BottomLeft_Real		= -2.25;
	Screen.BottomLeft_Imag		= -1.8;
	Screen.TopRight_Real		= 0.75;
	Screen.TopRight_Imag		= 1.5;
      }
     else
      { double bl_real, bl_imag, tr_real, tr_imag;

        bl_real = Screen.BottomLeft_Real + (((double) tl_x / (double) Screen.X_Size) * (Screen.TopRight_Real - Screen.BottomLeft_Real));
        bl_imag = Screen.BottomLeft_Imag + (((double) br_y / (double) Screen.Y_Size) * (Screen.TopRight_Imag - Screen.BottomLeft_Imag));
        tr_real = Screen.TopRight_Real - (((double) (Screen.X_Size - br_x) / (double) Screen.X_Size) * (Screen.TopRight_Real - Screen.BottomLeft_Real));
        tr_imag = Screen.TopRight_Imag - (((double) (Screen.Y_Size - tl_y) / (double) Screen.Y_Size) * (Screen.TopRight_Imag - Screen.BottomLeft_Imag));

        Screen.BottomLeft_Real = bl_real;
        Screen.BottomLeft_Imag = bl_imag;
        Screen.TopRight_Real   = tr_real;
        Screen.TopRight_Imag   = tr_imag;
      }
     Signal(&ScreenReady);
   } /* forever */
}
