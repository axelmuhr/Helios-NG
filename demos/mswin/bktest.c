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

   graph = OpenGraph("BkTest", 0, 0, 200, 200, WS_OVERLAPPEDWINDOW, SW_SHOWNA);

   if (graph == Null(Stream))
      exit(1);

   TextOut(graph, 0, 0, "Background White");		/* default background */
   SetBkColor(graph, BLUE);    				/* blue background */
   TextOut(graph, 0, 20, "Background Blue");
   SetBkColor(graph, YELLOW);  				/* yellow background */
   TextOut(graph, 0, 40, "Background, Yellow");
   sleep(3);
   CloseGraph(graph);
   TidyGraphics();
   exit(0);
}
