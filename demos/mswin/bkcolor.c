#include <stdio.h>
#include <stdlib.h>
#include <graph.h>
#include <helios.h>
#include <signal.h>

int main()
{
   Stream *graph;
   word   x, y;
   
   if (!InitGraphics(NULL, NULL))
      exit(1);

   graph = OpenGraph(NULL, 0, 0, 400, 200, WS_OVERLAPPEDWINDOW, SW_SHOWNA);
   if (graph == Null(Stream))
      exit(1);

   GetWindowSize(graph, &x, &y);
   FillRect(graph, 0, 0, x, y, BLUE);
   SetBkMode(graph, TRANSPARENT);
   SetTextColor(graph, RED);
   TextOut(graph, 0, 10, "RED text on a BLUE background!!");
   sleep(3);
   CloseGraph(graph);
   TidyGraphics();
   exit(0);
}   
   
