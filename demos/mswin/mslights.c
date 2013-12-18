/**************************************************************
 * mslights.c                                                 *
 * Perihelion Software Ltd				      *
 * Charlton Rd. Shepton Mallet, Somerset UK BA4 5QE           *
 *                                                            *
 * This source code is copyright free and may be adapted or   *
 * distributed without notice to the authors.                 *
 * Any improvements would be welcome by the authors.           *
 *                                                            *
 * This is a conversion of xlights.c which works in the X     *
 * windows environment, so that it runs in the MS-windows     *
 * environment.                                               *
 **************************************************************/

#define _BSD

#include <stdio.h>
#include <syslib.h>
#include <root.h>
#include <string.h>
#include <stdlib.h>
#include <queue.h>
#include <rmlib.h>

#include <graph.h>      /* include the windows graphics code */

#define WNAME "Lights"
#define DisplayFont ANSI_VAR

void  bar( word, word, word, word, word);
word  newbar(word, word);

void  init_info( void );
void  draw_bars(word i);

void  MSEventFn(SHORT, SHORT, word, word);
void  MSMouseFn(word, SHORT, SHORT, word);

word  border;
word  name_width;
word  panel_width;
word  panel_height;
word  panel_start;

word  bar_height;
word  bar_length;
word  bar_x;

word  loadbar_y;
word  membar_y;
word  linkbar_y;


static	int	NetworkWalk(RmProcessor Processor, ...);
static	int	NetworkCount(RmProcessor Processor, ...);

word  panel_y(word i) { return ((i)*(panel_height+border)+border); }

Stream *win;

word grey    	= RGB(128,128,128);
word lblue   	= RGB(128,128,255);
Semaphore running;
word repaint 	= TRUE;
word update  	= TRUE;
word nprocs 	= 0;           	    	/* number of processors tracked */
word pcount 	= 0;
char pname[100];  	   		/* processor name */
word maxw 	= 0;        		/* maximum string size of processor labels */
char *maxname;       			/* the largest processor label - text */

typedef struct Proc_Info {
        char            *Name;
        Object          *Proc;
        word            LoadBar;
        word		LinkBar;
        word		MemBar;
        word		LoadLight;
        word            LinkLight[4];
        word            LocalLight;
        ProcStats       OldInfo;
} Proc_Info;

Proc_Info 	*info = NULL;

ProcStats       NewInfo;        	/* place for new stat info      */
char            name[100];      	/* buffer for name              */


int main(int argc, char **argv)
{  word width, height;   		/* width and height of the window */
   word  caption;			/* height of caption bar */
   word x,y;             		/* top-left position of the window */
   word i;
   TextDim dim;

   InitSemaphore(&running,0);        
   if( argc <= 1 )
   {  RmNetwork	Network;

      /* Get details of the current network into local memory */
      Network = RmGetNetwork();
      if (Network == (RmNetwork) NULL)
      {  fprintf(stderr, "mslights: failed to get network details.\n");
         fprintf(stderr, "      : %s\n", RmMapErrorToString(RmErrno));
         exit(EXIT_FAILURE);
      }

      /* Walk down the current network examining every processor	*/
      (void) RmApplyProcessors(Network, &NetworkCount);

      info = malloc(sizeof(Proc_Info)*nprocs);

      if( info == NULL )
      {  printf("failed to get info\n\n");
         exit(EXIT_FAILURE);
      }

      memset(info,0,sizeof(Proc_Info)*nprocs);

      (void) RmApplyProcessors(Network, &NetworkWalk);


      }
      else
      {  info = malloc(sizeof(Proc_Info)*argc-1);
         if( info == NULL )
         {  printf("failed to get info\n\n");
            exit(EXIT_FAILURE);
         }

         memset(info,0,sizeof(Proc_Info)*argc-1);

         for( i = 1; i < argc; i++ )
         {  char *arg = argv[i];
            word w;

            w = strlen(arg);
            strcpy(pname,"/");
            strcat(pname,arg);
            strcat(pname,"/tasks");

            info[nprocs].Proc = Locate(NULL,pname);
            info[nprocs].Name = arg;

            if( info[nprocs].Proc == NULL ) 
            {  printf("failed to find %s\n\n",arg);
               exit(EXIT_FAILURE);
            }

            if( w > maxw ) maxw = w,maxname = arg;

            nprocs++;
         }
      }

      if(!InitGraphics((VoidFnPtr) MSMouseFn, (VoidFnPtr) MSEventFn))
      {  printf("Windows graphics system not found.\n\n");
         exit(EXIT_FAILURE);
      }
        
      if(!MSEventSupported())
      {  /* startws not loaded */
         printf("mslights: windows server not found\n");
         printf("        - repaint and exit disabled\n");
         printf("        - run startws to enable\n");
      }
        

      border	 	= 2;
      win 		= OpenGraph(NULL,0,0,5,5,WS_POPUP | WS_BORDER, SW_SHOWNA);
      caption 		= GetCaptionSize();
      SelectFont(win,DisplayFont);
      GetTextDim(win,&dim);
      CloseGraph(win);

      name_width 	= maxw * dim.ave_width + border;
      panel_width 	= dim.height*6+4*border+name_width;

      if ((7*border) < dim.height )
         panel_height 	= dim.height;
      else
         panel_height 	= 7*border;

      panel_start 	= name_width + border*2;

      bar_height 	= border;
      bar_x 		= name_width + 3*border;
      bar_length 	= panel_width - 2*border;
      loadbar_y 	= border + bar_height / 2;
      linkbar_y 	= loadbar_y + bar_height + border;
      membar_y 		= linkbar_y + bar_height + border;
	
      x = 0;
      y = 0;

      width 		= panel_width + name_width + 4*border;
      height 		= (panel_height+border)*nprocs + caption + border;

      win = OpenGraph(WNAME,x,y,width,height, 
               WS_OVERLAPPED|WS_BORDER|WS_MINIMIZEBOX|WS_CAPTION|WS_SYSMENU,
               SW_SHOWNA);
              
      if (win == (Stream *)NULL)
      {  printf("Unable to open window\n\n");
         exit(EXIT_FAILURE);
      }
        
      SelectFont(win,DisplayFont);
      GetWindowSize(win,&x,&y);

      while(!TestSemaphore(&running))
      {  if ( repaint && update )  /* repaint complete panel ? */
         {  repaint = FALSE;
            ClearGraph( win );
                                
            for(i = 0; i < nprocs; i++ )
            {  FillRect( win, panel_start,panel_y(i), 
		  name_width + border*2 + panel_width, 
                  panel_y(i)+panel_height, grey);
               info[i].LoadBar = 0;
	       info[i].LinkBar = 0;
	       info[i].MemBar = 0;
               info[i].LoadLight = -1;
               info[i].LinkLight[0] = -1;
               info[i].LinkLight[1] = -1;
               info[i].LinkLight[2] = -1;
               info[i].LinkLight[3] = -1;
               info[i].LocalLight = -1;
            }

            for(i = 0; i < nprocs; i++ )
               TextOut(win,border,panel_y(i),info[i].Name);
         }

         for( i = 0; i < nprocs; i++ )
            draw_bars(i);
         FLUSH();
                Delay(OneSec/4);
      }
      DisableMouse(win);
      CloseGraph(win);
      TidyGraphics();
}

void MSEventFn(SHORT Type, SHORT WinID, word Word1, word Word2)
{
   WinID = WinID;
   switch (Type) 
   {
      case MS_Repaint :
         /* repaint the screen */
         repaint = TRUE;
         break;
      case MS_KBD :
         /* keys pressed */
         {
            SHORT key = (SHORT)(Word1 & 0xffff);
            SHORT syskey = (SHORT)((Word1 & 0xffff0000) >> 16);
            SHORT virtual = (SHORT)(Word2 & 0xffff);
   
            if ((!virtual) && (syskey == 0))
               if(!((key != (SHORT)'q') && (key != (SHORT)'Q')))
		  Signal(&running);
         }
         break;
      case MS_Resize :
         {
            SHORT width = (SHORT)(Word1 & 0xffff);
            SHORT height = (SHORT)((Word1 & 0xffff0000) >> 16);
            
            update = (width != -1) && (height != -1);
         }
         break;
      default :
         break;
   }
}

void MSMouseFn(word WinID, SHORT X, SHORT Y, word Buttons)
{
   Buttons = Buttons;
   WinID = WinID;
   Y = Y;
   
   if (X > panel_start)
      ChangeCursor(win, CROSS);
   else
      ChangeCursor(win, ARROW);
}

void draw_bars(word i)
{
	Proc_Info *ii = info+i;
	word l;
	word new;
	word linkload = 0;

	ServerInfo(ii->Proc, (byte *)&NewInfo);
				
	new = newbar(NewInfo.Load,1000);
				
	if( new != ii->LoadBar )
	{
		bar(new,ii->LoadBar,lblue,bar_x,
		    panel_y(i)+loadbar_y);
		ii->LoadBar = new;
	}
	for( l = 0; l < 4; l++ )
	{
		linkload += 
			NewInfo.Link[l].In
			-ii->OldInfo.Link[l].In
			+NewInfo.Link[l].Out
			-ii->OldInfo.Link[l].Out;
	}
	linkload += (NewInfo.LocalTraffic
		   -ii->OldInfo.LocalTraffic)/10;
		
	new = newbar( linkload, 20000);
					      
	if( new != ii->LinkBar )
	{
		bar(new, ii->LinkBar, YELLOW, bar_x,
		    panel_y(i)+linkbar_y);
		ii->LinkBar = new;
	}

	new = newbar( NewInfo.MemMax-NewInfo.MemFree,
			NewInfo.MemMax);

	if( new != ii->MemBar )
	{
		bar(new,ii->MemBar, RED, bar_x,
		     panel_y(i)+membar_y);
		ii->MemBar = new;
	}
	ii->OldInfo = NewInfo;
}

void bar( word new, word old, word colour, word x, word y )
{
	
	if( new > old )
	{       /* extend bar */
   		SetLineColor(win, colour);
   		SetLineStyle(win, SOLID);
   		SetLineWidth(win, border);
                DrawLine(win, x+old, y, x+new, y);   		
	}
	else
	{       /* shorten bar */
   		SetLineColor(win, grey);
   		SetLineStyle(win, SOLID);
   		SetLineWidth(win, 2);
                DrawLine(win, x+new, y , x+old, y);   		

	}
	
}


/*{{{  newbar function */
word newbar( word value, word max )
{
	if ( value >= max )
	{
		value = max - 1;
	}

	if ( value == 0 ) return 0;
		
	return (value * bar_length) / max;
}

/*}}}*/

/* This routine is called for every processor in the network.	*/
static	int	NetworkWalk(RmProcessor Processor, ...)
{

   if (RmGetProcessorPurpose(Processor) == RmP_Helios)
   {  char *arg;
      word w;

      arg = (char *)RmGetProcessorId(Processor);
      w = strlen(arg);
      strcpy(pname,"/");
      strcat(pname,arg);
      strcat(pname,"/tasks");

      info[pcount].Proc = Locate(NULL,pname);
      info[pcount].Name = arg;

      if( info[pcount].Proc == NULL ) 
      {  printf("failed to find %s\n\n",arg);
         exit(EXIT_FAILURE);
      }

      if( w > maxw ) 
         maxw = w,maxname = arg;

      pcount++;
   }
   return(0);
}

/* This routine is called for every processor in the network.	*/
static	int	NetworkCount(RmProcessor Processor, ...)
{

  if (RmGetProcessorPurpose(Processor) == RmP_Helios)
     nprocs++;
}

