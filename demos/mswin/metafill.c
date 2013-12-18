#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <posix.h>
#include <graph.h>
#include <helios.h>
#include <string.h>

int main()
{
  Stream *graph;
  word   num_pts, pts[100], x, hDC;
  char   ch, filename[80];
  
  printf("\n\nmetafill will save a polygonal image in a metafile\n");
  printf("meta.fil and will play it back in the window.\n\n");
  printf("The file will be stored on the ram disk.\n\n");
  printf("Hit return to continue    ^C to exit ...");

  fflush(stdout);
  ch = getchar();
  
  strcpy(filename, "helios/meta.fil");

  if (!InitGraphics(NULL, NULL))
     exit(1);

  graph = OpenGraph(NULL, 0, 0, 300, 300, WS_OVERLAPPEDWINDOW,SW_SHOWNA);

  if (graph == Null(Stream))
     exit(1);

  num_pts = 50;
  for (x = 0; x < num_pts * 2; x+=2)
  {  pts[x]   = (int) (((float) rand()/ (float) RAND_MAX) * 300);
     pts[x+1] = (int) (((float) rand()/ (float) RAND_MAX) * 300);
  }

  hDC = CreateMetaFile(graph, filename);
  if (hDC == 0)
  {  printf("Unable to open meta file %s!\r\n", filename);
     TidyGraphics();
     exit(1);
  }

  SetFillAttr(graph, SOLID, RGB(0, 0, 150));
  SetFillMode(graph, WINDING);
  FillPoly(graph, num_pts, &pts[0]);
  FLUSH();
  CloseMetaFile(graph, hDC);
  hDC = GetMetaFile(graph, filename);
  if (hDC == 0)
  {  printf("Unable to retireve meta file %s!\r\n", filename);
     TidyGraphics();
     exit(1);
  }

  PlayMetaFile(graph, hDC);
  DeleteMetaFile(graph, hDC);
  sleep(3);
  unlink("/helios/meta.fil");
  CloseGraph(graph);
  TidyGraphics();
  exit(0);
}  
  	
