#include <stdio.h>
#include <stdlib.h>
#include <graph.h>
#include <helios.h>
#include <sem.h>

void MsMouseFn(word, SHORT, SHORT, word);

static SHORT mousex = 0, mousey = 0, mousel = 0, mouser = 0;
static Stream *graph;

Semaphore Go;

int main(void)
{
   char   ch;
   
   printf("\n\nmousetst demonstrates the use of the mouse. Hold left button\n");
   printf("down in window and move cursor to draw lines. Press right button\n");
   printf("to exit.\n\n");
   printf("Hit return to continue   ^C to exit ...");
   fflush(stdout);
   ch = getchar();

   InitSemaphore(&Go, 0);

   if (!InitGraphics((VoidFnPtr)MsMouseFn, (VoidFnPtr)NULL))
      exit(0);

   graph = OpenGraph("MouseTest", 100, 100, 200, 200, WS_OVERLAPPED | WS_CAPTION | WS_BORDER, SW_SHOWNORMAL);
   if (graph == (Stream *) NULL)
      exit(1);

   EnableMouse(graph);
   ChangeCursor(graph, CROSS);
   while (mouser == 0) 
   {  Wait(&Go);
      if (mousel == 1) 
      {  MoveTo(graph, mousex, mousey);
	 while (mousel == 1)
         {  /* if left button down, draw lines as cursor moves */
	    Wait(&Go);
	    LineTo(graph, mousex, mousey);   /* draw line */
	    FLUSH();
         }
      }
   }
   ChangeCursor(graph, ARROW);
   DisableMouse(graph);
   CloseGraph(graph);
   TidyGraphics();
   exit(0);
}


void MsMouseFn(word WinID, SHORT  X, SHORT  Y, word Buttons)
{
   switch (Buttons) 
   {
      case 1 :	   		/* left button down */
	 mousel = 1;
	 break;

      case 2 :	   		/* right button down */
	 mouser = 1;
	 break;

      case 0x8001 :
	 mousel = 0;
	 break;

      case 0x8002 :  		/* right button up */
	 mouser = 0;
	 break;

      default : break;
   }
   mousex = X;
   mousey = Y;
   if (TestSemaphore(&Go) <= 0) 
      Signal(&Go);
   WinID = WinID;
}



   
