#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <graph.h>

int main()
{
   Stream *graph, *prn;
   int    x1, x2, y1, y2, ct;
   char   ch;
   
   printf("\n\nprnline draw a bunch of random lines in the, and \n");
   printf("while drawing the lines in a window, initialise a print \n");
   printf("job, to print the image created by metaline.\n\n");
   printf("Hit return to continue    ^C to exit ...");
   fflush(stdout);
   ch = getchar();
   
   if (!InitGraphics(NULL, NULL))
   {  printf("unable to Initialise gaphics\r\n");
      exit(1);
   }
	
   graph = OpenGraph("Printer", -1, -1, -1, -1, WS_OVERLAPPEDWINDOW, SW_SHOWNA);

   if (graph == Null(Stream))
   {  printf("Unable to open window\r\n");
      exit(1);
   }
   
   prn = InitPrinter();
   if (prn == Null(Stream)) 
   {  printf("Unable to initialise the printer!\r\n");
      CloseGraph(graph);
      TidyGraphics();
      exit(1);
   }
   
   if (!PrintGraphics(prn, "/helios/lines.met"))
   {  printf("Unable to print graphics!\r\n");
      CloseGraph(graph);
      TidyGraphics();
      exit(1);
   }

   ClosePrinter(prn);

   SetMapMode(graph, M_ANISOTROPIC);
   SetDeviceExt(graph, 1, 1);
   SetLogicalExt(graph, 3, 3);

   for (ct = 0; ct < 50; ct++) 
   {  for (ct = 0; ct < 400; ct++) 
      {  /* generate random lines */
	 x1 = rand();
	 srand(x1);
	 y1 = rand();
	 srand(y1);
	 x2 = rand();
	 srand(x2);
	 y2 = rand();
	 srand(y2);
	 x1 = 10 + ((int) (((float) x1 / (float) RAND_MAX) * 900));
	 y1 = 10 + ((int) (((float) y1 / (float) RAND_MAX) * 600));
	 x2 = 10 + ((int) (((float) x2 / (float) RAND_MAX) * 900));
	 y2 = 10 + ((int) (((float) y2 / (float) RAND_MAX) * 600));
	 DrawLine(graph, x1, y1, x2, y2);  /* draw random lines */
      }
      FLUSH();
      sleep(5);
   }

   if (!CloseGraph(graph))
   {  printf("Unable to Close window\r\n");
      exit(1);
   }

   TidyGraphics();
   exit(0);
}   

