#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <helios.h>
#include <graph.h>

int main()
{
   Stream *graph;
   word   x_start, y_start;
   static word colors[9] = {BLACK, BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW,WHITE};
   
   if (!InitGraphics(NULL, NULL)) 
      exit(1);

   graph = OpenGraph(NULL, 0, 0, 200, 200, WS_POPUP | WS_BORDER, SW_SHOWNA);
   if (graph == Null(Stream))
      exit(1);
   
   DrawLine(graph, 10, 10, 10, 180);    /* draw an axis */
   DrawLine(graph, 10, 180, 180, 180);
   y_start = 179;
   
   /* draw a bunch of different color lines within the axis limits */
   for (x_start = 11; x_start < 180; x_start++) 
   {  for (; y_start > 21; y_start -= 21)
      {  SetLineColor(graph, colors[y_start / 21]);
     	 DrawLine(graph, x_start, y_start, x_start, y_start - 21);
      }
      y_start = 179;
   }
   FLUSH();
   sleep(3);
   CloseGraph(graph);
   TidyGraphics();
}   
