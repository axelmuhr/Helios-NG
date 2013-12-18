#include <stdio.h>
#include <stdlib.h>
#include <graph.h>

int main()
{
   Stream *graph;
   word   x, y;
   
   if (!InitGraphics(NULL, NULL))
      exit(1);
   
   graph = OpenGraph(NULL, 10, 10, 200, 200, WS_OVERLAPPEDWINDOW, SW_SHOWNA);
   if (graph == (Stream *) NULL)
      exit(1);
   
   SetMapMode(graph, M_LOMETRIC);
   GetLogicalExt(graph, &x, &y);
   CloseGraph(graph);
   printf("Logical extents are %x %x \r\n", (int)x, (int)y);
   exit(0);
}        
  	
