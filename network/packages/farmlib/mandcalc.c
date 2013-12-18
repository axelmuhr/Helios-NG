/**
*** mandcalc.c
***		Calculation part of the Mandelbrot example.
**/
#include <helios.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <farmlib.h>
#include "mandel.h"

/**
*** Each worker needs its own copy of the current colour and screen
*** information. This will be sent by the control program in special packets.
*** Flags are used to check that these packets have actually been received
*** before any calculating is done.
**/
static Mandel_ColourInfo	Colours;
static Mandel_ScreenInfo	Screen;
static bool			Colours_Set	= FALSE;
static bool			Screen_Set	= FALSE;
static void			mandel_eval(Mandel_Job *job);

/**
*** This is the worker routine invoked by the farm library. It loops forever
*** receiving packets from the control program, and takes suitable action
*** depending on the packet type. If the controller is installing new colour
*** or screen information then the worker's private copy of this is updated.
*** Otherwise the packet should be a job to calculate a particular scanline,
*** which is handled by a separate routine mandel_eval().
**/
void Mandel_Worker(void)
{
  forever
   { Mandel_Job *job = FmGetJob();

     switch(job->Type)
      { case Request_ColourInfo :
		{ Mandel_ColourInfo *info = (Mandel_ColourInfo *) job;
		  memcpy(&Colours, info, sizeof(Mandel_ColourInfo));
		  Colours_Set = TRUE;
		  break;	/* no need to acknowledge */
		}

	case Request_ScreenInfo :
		{ Mandel_ScreenInfo *info = (Mandel_ScreenInfo *) job;
		  memcpy(&Screen, info, sizeof(Mandel_ScreenInfo));
		  Screen_Set = TRUE;
		  break;	/* no need to acknowledge */
		}

	case Request_Job :
		unless (Colours_Set && Screen_Set)
		 { fprintf(stderr,
			"Mandel_Worker(%d): received job before parameters.\n",
			FmWorkerNumber);
		   exit(EXIT_FAILURE);
		 }
		if (job->Y_Coord >= Screen.Y_Size)
		 { fprintf(stderr,
			"Mandel_Worker(%d): received invalid y coordinate %d.\n",
			FmWorkerNumber, job->Y_Coord);
		   exit(EXIT_FAILURE);
		 }
		mandel_eval(job);
		break;

	default :
		fprintf(stderr,
			"Mandel_Worker(%d): received invalid packet, type %d.\n",
			FmWorkerNumber, job->Type);
		{ int *x = &(((int *) job)[-7]);
		  fprintf(stderr, "%x %x %x %x %x %x %x %x %x %x\n",
			x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7], x[8], x[9]);
		}
				  
		exit(EXIT_FAILURE);
      }	/* end of switch */
   }	/* end of forever loop */
}

/**
*** This routine calculates a scanline, storing the information in a
*** suitable reply packet, and sends the information back to the consumer
*** part of the controller. For details of the Mandelbrot calculation
*** please consult any good book on fractals.
**/
static void mandel_eval(Mandel_Job *job)
{ Mandel_Reply		*reply;
  double		pixel_real, pixel_imag, cur_real, cur_imag;
  int			i;
  int			iterations;
  int			colour;

		/* Get a buffer in which to calculate the reply		*/
  reply = (Mandel_Reply *) FmGetReplyBuffer(job, sizeof_Mandel_Reply + Screen.X_Size);

		/* Identify the job for which this is a reply		*/
  reply->Y_Coord = job->Y_Coord;

		/* Calculate the imaginary part for this scanline,	*/
		/* fixed for the whole scanline.			*/
  pixel_imag = Screen.BottomLeft_Imag + 
	((job->Y_Coord * (Screen.TopRight_Imag - Screen.BottomLeft_Imag)) /
	 Screen.Y_Size);

		/* Calculate all the pixels for this scanline.		*/
  for (i = 0; i < Screen.X_Size; i++)
   { 		/* Calculate the real part for this pixel		*/
     pixel_real = Screen.BottomLeft_Real +
	((i * (Screen.TopRight_Real - Screen.BottomLeft_Real)) / Screen.X_Size);

     cur_real = pixel_real; cur_imag = pixel_imag;
		
		/* Perform upto 100 Mandelbrot iterations		*/
     for (iterations = 0; iterations < 100; iterations++)
      { double new_real;
        new_real = (cur_real * cur_real) - (cur_imag * cur_imag) +
			pixel_real;
        cur_imag = (2 * cur_real * cur_imag) + pixel_imag;
        cur_real = new_real;

        if (((cur_real * cur_real) + (cur_imag * cur_imag)) > 4.0)
	 break;
       }

		/* Pick a colour depending on the number of iterations	*/
     if   (iterations < 2)   colour = Colours.Red;
     elif (iterations < 4)   colour = Colours.Orange;
     elif (iterations < 7)   colour = Colours.Yellow;
     elif (iterations < 15)  colour = Colours.Green;
     elif (iterations < 28)  colour = Colours.Blue;
     elif (iterations < 55)  colour = Colours.Indigo;
     elif (iterations < 100) colour = Colours.Violet;
     else colour = Colours.Black;

		/* Fill in the colour for this pixel.			*/
     reply->Scanline[i] = colour;
   }	/* for every pixel */

	/* And finally send the whole reply back to the consumer	*/
  FmSendReply(reply);
}

