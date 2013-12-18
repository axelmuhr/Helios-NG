#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <helios.h>
#include <graph.h>

void MsMouseFn(word, SHORT, SHORT, word);
#define WS_WINDOWSTYLE (WS_OVERLAPPED | WS_CAPTION )

Stream *graph;

int main ()
{
   FONT   font;
   int    error;
   word	  x, y;
 
   if (!InitGraphics((VoidFnPtr)MsMouseFn, (VoidFnPtr)NULL))
   {
      printf("texttest: Failed to open window\n\n");
      exit(1);
   }
   
   if(!MSMouseSupported())
   {  /* startws not loaded */
      printf("textest: Windows server (startws) not found\n\n");
      exit(1);
   }

   x = GetMaxX();
   graph = OpenGraph("DISTRIBUTED SOFTWARE", (x - 405), 5, 400, 200, WS_WINDOWSTYLE,SW_SHOWNA);

   if (graph == Null(Stream))
      exit(1);

   GetWindowSize(graph, &x, &y);
   FillRect(graph, 0, 0, x, y, BLUE);
   SetBkMode(graph, TRANSPARENT);

   EnableMouse(graph);
   ChangeCursor(graph, CROSS);

   SetTextAlign(graph, TA_CENTRE);
   SetTextSpacing(graph, 3);
   font.height	     	= 18;
   font.width	     	= 12;
   font.tilt	     	= 0;
   font.orientation  	= 0;
   font.weight	     	= W_BOLD;
   font.italic	     	= 0;
   font.underline    	= 0;
   font.strikeout    	= 0;
   font.charset      	= ANSI_FIXED;
   font.quality      	= Q_DEFAULT;
   font.OutPrecision 	= OUT_DEFAULT;
   font.ClipPrecision 	= CLIP_DEFAULT;
   font.pitch_family 	= P_DEFAULT | DECORATIVE;
   strcpy((BYTE *) &(font.face_name[0]), "Arial");

   error = CreateFont(graph, &font);
   if (error == 0)
   {
      CloseGraph(graph);
      TidyGraphics();
      exit(1);
   }

   SetTextColor(graph, YELLOW);
   FLUSH();

   TextOut(graph, 200, 25, "Click right to quit !");
   sleep( 3);
   FillRect(graph, 0, 0, x, y, BLUE);

   while (1) 
   {  TextOut(graph, 200, 50, "HELIOS");
      TextOut(graph, 200, 75, "PARALLEL");
      TextOut(graph, 200, 100, "OPERATING SYSTEM");

      sleep( 3);
      FillRect(graph, 0, 0, x, y, BLUE);

      TextOut(graph, 200, 50, "INMOS TRANSPUTER");
      TextOut(graph, 200, 75, "ACORN ARM");
      TextOut(graph, 200, 100, "TEXAS TMS320C40");
      sleep( 3);
      FillRect(graph, 0, 0, x, y, BLUE);
   }

}

void MsMouseFn(word WinID, SHORT  X, SHORT  Y, word Buttons)
{
   switch (Buttons) 
   {
      case 1 :	   		/* left button down */
	 break;
      case 2 :	   		/* right button down */
	 DisableMouse(graph);
   	 CloseGraph(graph);
   	 TidyGraphics();
   	 exit(0);
      default : break;
   }
   X = X;
   Y = Y;
   WinID = WinID;
}



   

