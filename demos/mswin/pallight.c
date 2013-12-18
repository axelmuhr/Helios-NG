/**************************************************************
 * mslights.c                                                 *
 *                  					      *
 * Converted by: M I Gunning and S A Wilson                   *
 *                                                            *
 * This is a conversion of xlights.c which works in the X     *
 * windows environment, so that it runs in the MS-windows     *
 * environment.  This particular example shows how to use the *
 * palettes in the graph library.  If the machine does not    *
 * support palettes, the colours will be approximated by      *
 * dithering.                                                 *
 **************************************************************/

#include <stdio.h>
#include <syslib.h>
#include <root.h>
#include <string.h>
#include <stdlib.h>

#include <graph.h>      /* include the windows graphics code */

#define WNAME "Lights"
#define DisplayFont ANSI_VAR


word border;
word name_width;
word name_x;
word name_y;
word light_width;
word light_height;
word panel_width;
word panel_height;
word panel_start;
word panel_x;
word load_x;
word load_y;
word link_y;
word caption;
word offset;

word panel_y(word i) { return ((i)*(panel_height+2*border)+border); }
word link_x(word i)  { return ((4*border+light_width)+((i)*(light_width+2*border)+3*border+name_width) + offset); }

Stream *win;

word grey = PALETTERGB(128,128,128);
word pink = PALETTERGB(255,0,128);
word lred = PALETTERGB(255,128,128);
word orange = PALETTERGB(255,128,0);
word dyellow = PALETTERGB(192,192,0);
word lgreen = PALETTERGB(0,255,128);
word lblue = PALETTERGB(128,128,255);
word dblue = PALETTERGB(0,0,128);
word blue = PALETTERGB(0,0,255);
word green = PALETTERGB(0,255,0);
word yellow = PALETTERGB(255,255,0);
word red = PALETTERGB(255,0,0);

LOGPALETTE *pal;

char **colnames;
#define ncolors  11

word colors[ncolors];

word running;
word repaint = TRUE;
word update = TRUE;

word nprocs = 0;                     /* number of processors tracked */

typedef struct Proc_Info {
        char            *Name;
        Object          *Proc;
        word            LoadLight;
        word            LinkLight[4];
        word            LocalLight;
        ProcStats       OldInfo;
} Proc_Info;

Proc_Info *info = NULL;

ProcStats       NewInfo;        /* place for new stat info      */
char            name[100];      /* buffer for name              */

void
SetPal(
       PALETTEENTRY *	entry,
       word		color )
{
   entry->red = GETR(color);
   entry->green = GETG(color);
   entry->blue = GETB(color);
   entry->flags = 0;
}

void
light(word value, word x, word y)
{
    if (update)
        FillRect(win,x,y,x+light_width,y+light_height,value);
}

word newlight(word value, word max)
{
        if( value >= max ) value=max-1;

        if( value == 0 ) return BLACK;

        return colors[(value*ncolors)/max];
}

void MSEventFn(SHORT Type, SHORT WinID, word Word1, word Word2)
{
   switch (Type) {
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
               running = (key != (SHORT)'q') && (key != (SHORT)'Q');
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

int
main(int argc, char **argv)
{
        word width, height;   /* width and height of the window */
        word x,y;             /* top-left position of the window */
        word i;
        char pname[100];     /* processor name */
        word maxw = 0;        /* maximum string size of processor labels */
        char *maxname;       /* the largest processor label - text */
        TextDim dim,dim1;
        
        if( argc <= 1 )
        {
                printf("usage: %s processors...\n",argv[0]);
                exit(1);
        }

        info = malloc(sizeof(Proc_Info)*argc-1);

        if( info == NULL )
        {
                printf("failed to get info\n\n");
                exit(1);
        }

        memset(info,0,sizeof(Proc_Info)*argc-1);

        for( i = 1; i < argc; i++ )
        {
                char *arg = argv[i];
                word w;

                w = strlen(arg);
                strcpy(pname,"/");
                strcat(pname,arg);
                strcat(pname,"/tasks");

                info[nprocs].Proc = Locate(NULL,pname);
                info[nprocs].Name = arg;

                if( info[nprocs].Proc == NULL ) 
                {
                	printf("failed to find %s\n\n",arg);
                	exit(1);
                }

                if( w > maxw ) maxw = w,maxname = arg;

                nprocs++;
        }
        
        running = InitGraphics((VoidFnPtr)NULL, (VoidFnPtr) MSEventFn);
        if (!running)
        {
        	printf("Windows graphics system not found.\n\n");
        	exit(1);
        }
        
        running = MSEventSupported();

        if (!running) {
           /* startws not loaded */
           IOdebug("MSLights: windows server not found");
           IOdebug("        - repaint and exit disabled");
           IOdebug("        - run startws to enable");
           running = TRUE;
        }
        
        pal = CreatePalette(ncolors);
        SetPal(&(pal->pal[0]), dblue);
        SetPal(&(pal->pal[1]), blue);
        SetPal(&(pal->pal[2]), lblue);
        SetPal(&(pal->pal[3]), green);
        SetPal(&(pal->pal[4]), lgreen);
        SetPal(&(pal->pal[5]), yellow);
        SetPal(&(pal->pal[6]), dyellow);
        SetPal(&(pal->pal[7]), orange);
        SetPal(&(pal->pal[8]), lred);
        SetPal(&(pal->pal[9]), pink);
        SetPal(&(pal->pal[10]), red);

        caption = GetCaptionSize();
        border = 2;
        win = OpenGraph(NULL,0,0,5,5,WS_POPUP | WS_BORDER, SW_SHOWNA);
        GetTextDim(win,&dim1);
        SelectFont(win,DisplayFont);
        GetTextDim(win,&dim);
        CloseGraph(win);
        
        name_width = maxw * dim.ave_width;
        name_x = border;
        name_y = 0;
        light_width = dim.height - 2*border;
        light_height = light_width;
        panel_width = (light_width+2*border)*6+4*border+name_width;
        panel_height = (light_height+2*border);
        panel_start = panel_x+name_width+border*3;
        panel_x = border;
        load_x = name_width+3*border;
        load_y = border;
        link_y = border;
        x = 0;
        y = 0;

        width = panel_width+3*border;
        height = (panel_height+2*border)*nprocs + caption;

        colors[0] = dblue;
        colors[1] = blue;
        colors[2] = lblue;
        colors[3] = green;
        colors[4] = lgreen;
        colors[5] = yellow;
        colors[6] = dyellow;
        colors[7] = orange;
        colors[8] = lred;
        colors[9] = pink;
        colors[10] = red;
        
        win = OpenGraph(WNAME,x,y,width,height, 
              WS_OVERLAPPED|WS_BORDER|WS_MINIMIZEBOX|WS_CAPTION|WS_SYSMENU,
              SW_SHOWNA);
        SelectPalette(win, pal);
              
        if (win == (Stream *)NULL)
        {
        	printf("Unable to open window\n\n");
        	exit(1);
        }
        
        SelectFont(win,DisplayFont);
        GetWindowSize(win,&x,&y);
        if (x > width)
        {
		offset = (x-width) / 2;
        	panel_start += offset;
        	panel_width += offset;
        	load_x += offset;
        }
        else
        	offset = 0;

        for(; running ;)
        {
                if ( repaint && update )  /* repaint complete panel ? */
                {
                          repaint = FALSE;
                          ClearGraph( win );
                                
                          for(i = 0; i < nprocs; i++ )
                          {
                                  FillRect(win,panel_start,panel_y(i),
                                           panel_x+panel_width,
                                           panel_y(i)+panel_height,
                                           grey);

                                  info[i].LoadLight = -1;
                                  info[i].LinkLight[0] = -1;
                                  info[i].LinkLight[1] = -1;
                                  info[i].LinkLight[2] = -1;
                                  info[i].LinkLight[3] = -1;
                                  info[i].LocalLight = -1;
                          }

                          for(i = 0; i < nprocs; i++ )
                          	TextOut(win,panel_x+name_x,panel_y(i)+name_y,
                          	        info[i].Name);
                }

                for( i = 0; i < nprocs; i++ )
                {
                        Proc_Info *ii = info+i;
                        word l;
                        word new;

                        ServerInfo(ii->Proc,(byte *)&NewInfo);

                        new = newlight(NewInfo.Load,1000);

                        if( new != ii->LoadLight )
                        {
                                light(new,panel_x+load_x,panel_y(i)+load_y);
                                ii->LoadLight = new;
                        }
                        for( l = 0; l < 4; l++ )
                        {
                                new = newlight(
                                        NewInfo.Link[l].In+
                                        NewInfo.Link[l].Out-
                                        ii->OldInfo.Link[l].In-
                                        ii->OldInfo.Link[l].Out,
                                        10000);

                                if( new != ii->LinkLight[l] )
                                {
                                        light(new,panel_x+link_x(l),
                                        panel_y(i)+link_y);
                                        ii->LinkLight[l] = new;
                                }
                        }
                        new = newlight( NewInfo.LocalTraffic-
                                        ii->OldInfo.LocalTraffic,
                                        10000);
                        if( new != ii->LocalLight )
                        {
                                light(new,panel_x+link_x(4),
                                panel_y(i)+link_y);
                                ii->LocalLight = new;
                        }
                        ii->OldInfo = NewInfo;
                }
                FLUSH();
                Delay(OneSec/3);
        }
        TidyGraphics();

	return 0;
	
} /* main */
