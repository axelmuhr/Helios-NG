/***************************************************************************
 * This test program displays the use of palettes in the drawing functions *
 ***************************************************************************/

#include <stdio.h>
#include <syslib.h>
#include <stdlib.h>
#include "graph.h"

#define MAX   6
#define PERMS 256

int
main(void)
{
  Stream *graph_id;
  int fnrc;
  word i;
  LOGPALETTE *pal;
  word colors[PERMS];
  
  /* initialise the graphics system, without any event handlers */
  
  if (!InitGraphics(NULL, NULL)) {
      fprintf(stderr,"Unable to find graphics server\n\n");
      exit(EXIT_FAILURE);
    }
  
  /* open the window in its maximum form with resizable border and */
  /* do not give the window the input focus.                       */
  
  graph_id = OpenGraph("Test", 200, 200, 64, 64, 
		       WS_OVERLAPPEDWINDOW, SW_SHOWNA);
  
  if (graph_id == (Stream *) NULL) {
      TidyGraphics();
      fprintf(stderr, "Unable to open window\n\n");
      exit(EXIT_FAILURE);
    }
  
  /* set up the values which will be used to generate the palette */
  /* create a palette with enough space */
  pal = CreatePalette(PERMS);
  
  
  for (i=0; i<PERMS; i++) {
      /* either set up the same RGB values in colors : */
      colors[i] = PALETTERGB(i, i, i);
      
      /* set up the user defined palette */
      pal->pal[i].red   = (int)i;
      pal->pal[i].green = (int)i;
      pal->pal[i].blue  = (int)i;
      pal->pal[i].flags = 0;    /* setting this to zero will generally be */
      /* sufficient                             */
      
    }
  
  
  /* enable the use of the palette */
  fnrc = SelectPalette(graph_id, pal);
  
    {
      BITMAP *bmp;
      char map[256*4];
      int k,l;
      
      bmp = CreateDIBitMap(graph_id,16,16);
      
      for (l = 0; l < 4; l++)
	for (k = 0; k < 256; k++)
	  map[l*256 + k] = 0xff ;
      
      for (l = 0; l < 256; l++)
	{
	  for (k = 0; k < 256; k++)
	    {
	      map[k*4]   = 0x00;
	      map[k*4+1] = l;
	      map[k*4+2] = l;
	      map[k*4+3] = l;
	    }
	  SetBitMapBits(bmp,map);
	  DrawBitMap(graph_id,bmp,0,0,SRCCOPY);
	  (void) getchar();
	}
    }
  
  TidyGraphics();
  
  return(0);
}

