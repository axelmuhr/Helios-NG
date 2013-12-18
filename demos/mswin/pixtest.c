#include <stdio.h>
#include <stdlib.h>
#include <graph.h>

int main (void)
{
   Stream *graph_strm;
   int    x, y, x_loc[1000], y_loc[1000], loc[8];
   
   if (!InitGraphics(NULL, NULL)) 
   {  printf("Unable to initialise graphics\r\n");
      exit(1);
   }   		
   graph_strm = OpenGraph("PutPixel Test", 100, 100, 200, 200, WS_OVERLAPPED | WS_CAPTION | WS_BORDER, SW_SHOWNA);
   if (graph_strm == (Stream *) NULL)
   {  printf("Unable to open graphics window\r\n");
      exit(1);
   } 

/* Generate a bunch of random points, and scale them to fit into a small */
/* rectangle */
 
   for (x = 0; x < 1000; x++)
   {  x_loc[x] = rand();
      y_loc[x] = rand();
   }
   for (x = 0; x < 1000; x++)
   {  x_loc[x] = (int) (((float) x_loc[x] / (float) RAND_MAX) * 40);
      y_loc[x] = (int) (((float) y_loc[x] / (float) RAND_MAX) * 40);
   }   	
   for (x = 0; x < 8; x++)
   {  loc[x] = rand();
      loc[x] = (int) (((float) loc[x] / (float) RAND_MAX) * 150);
   }   	
   
   for (y = 0; y < 8; y++)
   {  /* Plot black pixels */
      for (x = 0; x < 1000; x++)
         PutPixel(graph_strm, x_loc[x] + loc[y], y_loc[x] + loc[y], BLACK);

      /*  Plot white pixels */
      for (x = 0; x < 1000; x++)
         PutPixel(graph_strm, x_loc[x] + loc[y], y_loc[x] + loc[y], WHITE);
   } 

   if (!CloseGraph(graph_strm))
   {  printf("Unable to Close the graph window\r\n");
      exit(1);
   }
   
   if (!TidyGraphics())
   {  printf("Tidy graphics failed\r\n");
      exit(1);
   }
  exit(0);
}  
