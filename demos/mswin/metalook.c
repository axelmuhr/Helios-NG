/* Program to test the line functions */

#include <graph.h>
#include <stdio.h>
#include <string.h>
#include <helios.h>
#include <string.h>

int main()
{
   Stream *graph;
   word   x_start, y_start;
   static word colors[9] = {BLACK, BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW,
                            WHITE};
   word hDC;
   char ch, filename[80];

   printf("\n\nmetalook saves one vertical line of the sonolook display in \n");
   printf("a metafile called sono.met, and plays the metafile enough times \n");
   printf("to give the same display as that in the sonolook program\n\n");
   printf("NOTE: The metafile will remain on your disk!\n");
   printf("The file can be found in your HELIOS home directory!\n\n");
   printf("Hit return to continue    ^C to exit ... ");
   fflush(stdout);
   ch = getchar();

   strcpy(filename, "helios/sono.met");
   
   if (!InitGraphics(NULL, NULL)) return(0);
   graph = OpenGraph(NULL, 0, 0, 200, 200, WS_POPUP | WS_BORDER, SW_SHOWNA);
   if (graph == (Stream *) NULL) return(0);
   
   DrawLine(graph, 10, 10, 10, 180);    /* draw an axis */
   DrawLine(graph, 10, 180, 180, 180);
   y_start = 179;
   hDC = CreateMetaFile(graph, filename);  
   if (hDC == 0l) {
   	TidyGraphics();
	printf("Unable to open meta file %s\n", filename);
   	return(0);
   }
   
/* draw a bunch of different color lines within the axis limits */
   	
     for (; y_start > 21; y_start -= 21) {
     	SetLineColor(graph, colors[y_start / 21]);
	DrawLine(graph, 0, y_start, 0, y_start - 21);
     }

   CloseMetaFile(graph, hDC);
   hDC = GetMetaFile(graph, filename);
   if (hDC == 0) {
     printf("Unable to get metafile %s!!\r\n", filename);
     TidyGraphics();
     return(0);
   }

   y_start = 179;

   for (x_start = 11; x_start < 180; x_start++) {
      SetDeviceOrg(graph, x_start, 0);
      PlayMetaFile(graph, hDC);
   }
   FLUSH();
   DeleteMetaFile(graph, hDC);
   Delay(OneSec * 3);
   CloseGraph(graph);
   TidyGraphics();
}   
