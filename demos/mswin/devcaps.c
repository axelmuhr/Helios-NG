#include <stdio.h>
#include <stdlib.h>
#include <graph.h>

int main()
{
   Stream *graph;

   if (!InitGraphics(NULL, NULL))
      exit(1);

   graph = OpenGraph("DeviceCaps", 0, 0, 200, 200, WS_OVERLAPPEDWINDOW, SW_SHOWNA);

   if (graph == Null(Stream))
      exit(1);

   printf("Technology = %d\r\n", GetDeviceCaps(graph, TECHNOLOGY));
   printf("Horzsize   = %d\r\n", GetDeviceCaps(graph, HORZSIZE));
   printf("VertSize   = %d\r\n", GetDeviceCaps(graph, VERTSIZE));
   printf("HorzRes    = %d\r\n", GetDeviceCaps(graph, HORZRES));
   printf("VertRes    = %d\r\n", GetDeviceCaps(graph, VERTRES));
   printf("BitsPixel  = %d\r\n", GetDeviceCaps(graph, BITSPIXEL));
   printf("Planes     = %d\r\n", GetDeviceCaps(graph, PLANES));
   printf("NumBrushes = %d\r\n", GetDeviceCaps(graph, NUMBRUSHES));
   printf("NumPens    = %d\r\n", GetDeviceCaps(graph, NUMPENS));
   printf("NumMarkers = %d\r\n", GetDeviceCaps(graph, NUMMARKERS));
   printf("NumFonts   = %d\r\n", GetDeviceCaps(graph, NUMFONTS));
   printf("NumColors  = %d\r\n", GetDeviceCaps(graph, NUMCOLORS));
   printf("RasterCaps = %d\r\n", GetDeviceCaps(graph, RASTERCAPS));
   printf("Aspectx    = %d\r\n", GetDeviceCaps(graph, ASPECTX));
   printf("Aspecty    = %d\r\n", GetDeviceCaps(graph, ASPECTY));
   printf("AspectXY   = %d\r\n", GetDeviceCaps(graph, ASPECTXY));
   printf("LogPixelsX = %d\r\n", GetDeviceCaps(graph, LOGPIXELSX));
   printf("LogPixelsY = %d\r\n", GetDeviceCaps(graph, LOGPIXELSY));
   CloseGraph(graph);
   TidyGraphics();
   exit(0);
}
