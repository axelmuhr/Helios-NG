#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <graph.h>
#include <helios.h>

int main()
{
   Stream *graph;
   int    x1, x2, y1, y2, ct;
   
   if (!InitGraphics(NULL, NULL))
   {  printf("unable to Initialise gaphics\r\n");
      exit(1);
   }	
   graph = OpenGraph("Lines", -1, -1, -1, -1, WS_OVERLAPPEDWINDOW, SW_SHOWNA);

   if (graph == (Stream *) NULL)
   {  printf("Unable to open window\r\n");
      exit(1);
   }

   for (ct = 0; ct < 200; ct++)
   {  /* generate random lines */
      x1 = rand();
      srand(x1);
      y1 = rand();
      srand(y1);
      x2 = rand();
      srand(x2);
      y2 = rand();
      srand(y2);
      x1 = 10 + ((int) (((float) x1 / (float) RAND_MAX) * 500));
      y1 = 10 + ((int) (((float) y1 / (float) RAND_MAX) * 380));
      x2 = 10 + ((int) (((float) x2 / (float) RAND_MAX) * 500));
      y2 = 10 + ((int) (((float) y2 / (float) RAND_MAX) * 380));
      DrawLine(graph, x1, y1, x2, y2);  /* draw random lines */
   }
   FLUSH();
   sleep(3);
   if (!CloseGraph(graph))
   {
   	printf("Unable to Close window\r\n");
   	exit(1);
   }
   TidyGraphics();
   exit(0);
}   
