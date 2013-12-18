#include <stdio.h>
#include <stdlib.h>
#include <graph.h>
#include <helios.h>
#include <signal.h>

int main()
{
   Stream *graph;
   
   if (!InitGraphics(NULL, NULL))
      exit(1);

   graph = OpenGraph(NULL, 0, 0, 500, 300, WS_OVERLAPPEDWINDOW, SW_SHOWNA);

   if (graph == Null(Stream))
      exit(1);

   SetFillAttr(graph, SOLID, BLUE);
   SetMapMode(graph, M_ISOTROPIC);		/* set mapping mode */
   SetLogicalExt(graph, 4, 1);			/* get equal units on both axes */
   SetDeviceExt(graph, 5, 1); 
   if (!Ellipse(graph, 10, 10, 380, 250)) 
   {  printf("Ellipse failed\r\n");
      exit(1);
   }

   {  int i;
      for(i = 0; i < 100; i = i+9)
         Circle(graph, (250 +i), (150+i), (100-i)); 
      FLUSH();

      for(i = 0; i < 10; i++)
         Rectangle(graph, (10 + 2*i), (100 + 2*i), (150 - 3*i), (200 - 3*i));
   }
   FLUSH();
   sleep(3);

   CloseGraph(graph);   	
   TidyGraphics();
   exit(0);
}   
