/* Program to test the DrawLine function */

#include <stdio.h>
#include <stdlib.h>
#include <graph.h>
#include <helios.h>
#include <string.h>

int main()
{
   Stream *graph_stream;
   int    x1, x2, y1, y2, ct;
   word   hDC;
   char   ch, filename[80];
   
   printf("\n\nmetaline saves a bunch of random lines in a metafile called\n");
   printf("lines.met and replays the metafile in a window!!\n\n");
   printf("NOTE: The metafile will remain on your disk!\n");
   printf("The file can be found in your HELIOS home directory!\n\n");
   printf("Hit return to continue    ^C to exit ... ");
   fflush(stdout);
   ch = getchar();

   strcpy(filename, "helios/lines.met");
   
   if (!InitGraphics(NULL, NULL)) {
   	printf("unable to Initialise gaphics\r\n");
   	return(0);
   }	
   graph_stream = OpenGraph("Lines", -1, -1, -1, -1, WS_OVERLAPPEDWINDOW,
			     SW_SHOWNA);
   if (graph_stream == (Stream *) NULL) {
   	printf("Unable to open window\r\n");
   	return(0);
   }

   hDC = CreateMetaFile(graph_stream, filename);
   if (hDC == 0) {
     printf("Unable to open metafile %s\r\n", filename);
     TidyGraphics();
     return(0);
   }

   for (ct = 0; ct < 400; ct++) {   /* generate random lines */
     x1 = rand();
     srand(x1);
     y1 = rand();
     srand(y1);
     x2 = rand();
     srand(x2);
     y2 = rand();
     srand(y2);
     x1 = 10 + ((int) (((float) x1 / (float) RAND_MAX) * 2000));
     y1 = 10 + ((int) (((float) y1 / (float) RAND_MAX) * 2000));
     x2 = 10 + ((int) (((float) x2 / (float) RAND_MAX) * 2000));
     y2 = 10 + ((int) (((float) y2 / (float) RAND_MAX) * 2000));
     DrawLine(graph_stream, x1, y1, x2, y2);  /* draw random lines */
   }
   FLUSH();
   CloseMetaFile(graph_stream, hDC);

   SetMapMode(graph_stream, M_ANISOTROPIC);
   SetDeviceExt(graph_stream, 1, 1);
   SetLogicalExt(graph_stream, 4, 4);

   hDC = GetMetaFile(graph_stream, filename);
   if (hDC == 0) {
     printf("Unable to retrieve meta file %s\r\n", filename);
     TidyGraphics();
     return(0);
   }

   PlayMetaFile(graph_stream, hDC);
   DeleteMetaFile(graph_stream, hDC);
   Delay(OneSec * 3);
   if (!CloseGraph(graph_stream)) {
   	printf("Unable to Close window\r\n");
   	return(0);
   }
   TidyGraphics();
}   
