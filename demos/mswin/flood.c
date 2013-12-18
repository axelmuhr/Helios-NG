#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <graph.h>
#include <helios.h>

int main()
{
  Stream *graph;
  word	 x, y;

  if (!InitGraphics(NULL, NULL))
     exit(1);

  graph = OpenGraph(NULL, 100, 100, 50, 200, WS_OVERLAPPEDWINDOW, SW_SHOWNA);
  if (graph == Null(Stream))
     exit(1);

  GetWindowSize(graph, &x, &y);

  SetFillAttr(graph, BLUE, H_CROSS);
  FillRect(graph, 0, 0, x - 5, y - 5, BLUE);	/* fill rect in blue */
  FLUSH();
  sleep(2);
  FillRect(graph, 0, 0, x - 10 , y - 10, RED);   		/* fill window in red */
  FLUSH();
  sleep(2);
  FillRect(graph, 0, 0, x - 15, y - 15, GREEN);	/* fill rect in green */
  FLUSH();
  sleep(2);
  FillRect(graph, 0, 0, x -20 , y - 20 , YELLOW);		/* fill window in yellow */
  FLUSH();
  sleep(2);
  FillRect(graph, 0, 0, x - 25, y - 25, RGB(127, 127, 127));  /* fill rect in gray */ 
  FLUSH();
  sleep(2);
  FillRect(graph, 0, 0, x - 30 , y - 30, RGB(0, 127, 255));  /* fill window in light blue */
  FLUSH();
  sleep(2);
  CloseGraph(graph);
  TidyGraphics();
  exit(0);
}
