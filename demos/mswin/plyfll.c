#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <graph.h>
#include <helios.h>

int main()
{
   Stream *graph_stream;
   word   num_pts, pts[100], x;
  
   if (!InitGraphics(NULL, NULL))
      exit(1);

   graph_stream = OpenGraph(NULL, 0, 0, 300, 300, WS_OVERLAPPEDWINDOW, SW_SHOWNA);

   if (graph_stream == Null(Stream))
      exit(1);

   /* generate a 50 random points */

   num_pts = 50;
   for (x = 0; x < num_pts * 2; x+=2) 
   {  pts[x]   = (int) (((float) rand()/ (float) RAND_MAX) * 300);
      pts[x+1] = (int) (((float) rand()/ (float) RAND_MAX) * 300);
   }

   SetFillAttr(graph_stream, SOLID, BLUE);  		/* Solid blue fill style */
   SetFillMode(graph_stream, ALTERNATE);   		/* alternate mode */
   FillPoly(graph_stream, num_pts / 2, &pts[0]);   	/* Fill a 25 point polygon */
  
   SetFillAttr(graph_stream, H_DIAGCROSS, RED);  	/* Red Hatched fill style */
   SetFillMode(graph_stream, WINDING);    		/* winding mode */
   FillPoly(graph_stream, num_pts / 2, &pts[num_pts]); 	/* 25 point polygon */
   FLUSH();
   sleep(3);
   CloseGraph(graph_stream);
   TidyGraphics();
   exit(0);
}  
  	
