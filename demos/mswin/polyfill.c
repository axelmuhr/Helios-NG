#include <stdio.h>
#include <stdlib.h>
#include <syslib.h>
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
      exit(0);

   num_pts = 50;
   for (x = 0; x < num_pts * 2; x+=2)
   {  pts[x]   = (int) (((float) rand()/ (float) RAND_MAX) * 300);
      pts[x+1] = (int) (((float) rand()/ (float) RAND_MAX) * 300);
   }

   SetFillAttr(graph_stream, SOLID, RGB(0, 0, 150));
   SetFillMode(graph_stream, WINDING);
   FillPoly(graph_stream, num_pts, &pts[0]);
   FLUSH();
   sleep(3);
   CloseGraph(graph_stream);
   TidyGraphics();
   exit(0);
}  
  	
