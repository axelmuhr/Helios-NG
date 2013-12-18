#include <stdio.h>
#include <syslib.h>
#include <stdlib.h>
#include <signal.h>
#include <graph.h>

#define MAX   6
#define PERMS 216

int main(void)
{
   Stream *graph;
   int 	fnrc, i, j;
   double width, height;
   int 	r, g, b;
   char perms[MAX];
   LOGPALETTE *pal;
   word colors[PERMS];

   /* initialise the graphics system, without any event handlers */
   
   if (!InitGraphics(NULL, NULL))
   {  fprintf(stderr,"Unable to find graphics server\n\n");
      exit(EXIT_FAILURE);
   }
   
   /* open the window in its maximum form with resizable border and */
   /* do not give the window the input focus.                       */
   
   graph = OpenGraph("Palette Test", 0, 0, GetMaxX(), GetMaxY(), WS_OVERLAPPEDWINDOW, SW_SHOWNA);
                        
   if (graph == Null(Stream)) 
   {  TidyGraphics();
      fprintf(stderr, "Unable to open window\n\n");
      exit(1);
   }
   
   /* set up the values which will be used to generate the palette */
   perms[0] = 0;
   perms[1] = 51;
   perms[2] = 102;
   perms[3] = 153;
   perms[4] = 204;
   perms[5] = 255;
   
   /* create a palette with enough space */
   pal = CreatePalette(PERMS);
   
   r = 0;   g = 0;   b = 0;
   
   /* the following loop generates the color palette as well as setting */
   /* up the color used by the program.  Either the PALETTERGB color or */
   /* the PALETTEINDEX color value may be used - it produces the same   */
   /* result.                                                           */
   
   for (i=0; i<PERMS; i++)
   {  /* either set up the same RGB values in colors : */
      colors[i] = PALETTERGB(perms[r], perms[g], perms[b]);
      
      /* or use the index value into the user defined palette : */
      /* colors[i] = PALETTEINDEX(i); */
      
      /* set up the user defined palette */
      pal->pal[i].red 	= perms[r];
      pal->pal[i].green = perms[g];
      pal->pal[i].blue 	= perms[b];
      pal->pal[i].flags = 0;    /* setting this to zero will generally be */
                                /* sufficient                             */
      
      if ((++r) == MAX)
      {  r = 0;
         if ((++g) == MAX) 
         {  g = 0;
            b++;
         }
      }
   }
   
   /* work out the width and height of a color block */
   width = (double)(GetMaxX() - 2*GetResizeFrameX()) / 18.0;
   height = (double)(GetMaxY() - GetCaptionSize() - 2*GetResizeFrameY()) / 12.0;
   
   /* enable the use of the palette */
   fnrc = SelectPalette(graph, pal);
   
   /* within this loop we display the colors */
   for (i=0; i<12; i++)
      for (j=0; j<18; j++)
         FillRect(graph, (int)((double)j*width), (int)((double)i*height), 
           (int)((double)(j+1)*width), (int)((double)(i+1)*height), colors[i*18+j]);
   
   FLUSH();
   
   sleep(3);

   CloseGraph(graph);
   TidyGraphics();
   
   exit(0);
}

