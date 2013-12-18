#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <graph.h>
#include <helios.h>

int main()
{
   Stream *graph;

   if (!InitGraphics(NULL, NULL))
      exit(1);

   graph = OpenGraph(NULL, 100, 100, 200, 100, WS_POPUP | WS_BORDER, SW_SHOWNA);
   if (graph == Null(Stream))
      exit(1);

   SetBkColor(graph, RED);
   SetLineColor(graph, RGB(127, 127, 127));
   SetLineStyle(graph, SOLID);
   SetLineWidth(graph, 3);

   DrawLine(graph, 10, 10, 150, 10);
   SetLineColor(graph, BLUE);
   DrawLine(graph, 30, 30, 150, 30);
   SetLineColor(graph, RED);
   DrawLine(graph, 50, 50, 150, 50);
   SetLineColor(graph, RGB(0, 127, 255));
   DrawLine(graph, 80, 80, 150, 80);
   SetLineWidth(graph, 1);
   SetLineStyle(graph, DASHDOTDOT);
   SetLineColor(graph, BLACK);
   DrawLine(graph, 65, 65, 150, 65);
   FLUSH();
   
   sleep(3);
   CloseGraph(graph);
   TidyGraphics();
   exit(0);
}
