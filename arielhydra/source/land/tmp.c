



/*
  3dfrac 3.0 by Aaron Contorer 1987-1989
  Draws three-dimensional fractal landscapes.

  Please send the author a copy of anything interesting you make
  that uses parts of this code or parts of its design.
*/



/* 900821 RAT  Ported to Turbo C's BGI and added Super VGA 256 color support */


#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include "isvga256.h"
#include "isvgadet.h"
#include "VGAEXTRA.H"
#include "vc40rmap.h"

/*
 * DSP stuff
 */
#define NUMSYMS (sizeof(symnames) / sizeof(char *))
#define STEEP       0
#define SEALEVEL    1
#define YBOTTOM     2
#define WATERCOLOR  3
#define LANDCOLOR   4
#define SEED        5
#define START_FLAG  6
#define READY_FLAG  7
#define INTPRI      8
#define INTVEC      9
#define TESTLINE    10
#define LINES       11
#define DEEP	    12
#define	NL          13
#define DONE_FLAG   14
#define IX0         15
#define IY0         16
#define IX2         17
#define IY2         18
#define IZ0         19
#define IZ1         20
#define IZ2         21
#define IZ3         22

char *symnames[] = {
    "_steep",
    "_sealevel",
    "_ybottom",
    "_WATERCOLOR",
    "_LANDCOLOR",
    "_seed",
    "_start_flag",
    "_ready_flag",
    "_intpri",
    "_intvec",
    "_testline",
    "_lines",
    "_ideep",
    "_nl",
    "_done_flag",
    "_ix0",
    "_iy0",
    "_ix2",
    "_iy2",
    "_iz0",
    "_iz1",
    "_iz2",
    "_iz3",
};

struct symtab   symtab[NUMSYMS];


/*Graphic design*/

#define  XA 0
#define  XB 500
#define  YA 0
#define  ZA 0
#define  YADD 38

u_long	deep = 9;

#define NLINES  4000

struct line3d {
    int color;
    int pflag;
    int y0, x0;
    int y1, x1;
} testline;

struct line3d lines[NLINES];

void hsv2rgb(float h,float s,float v,RGB *Color);
void addline(int x0, int y0, int z0, int x1, int y1, int z1);
void draw_lines(int nlines);
void process_dsp(int dsp);

int dsps_running = 0;

/* Color indices
#define WATERCOLOR 1
#define LANDCOLOR 2 */

int watercolor, landcolor;

int g_driver,g_mode,g_error;   /* !!! */


float steep;
int sealevel;
int ybottom;

/* new vars for 256 color */
DACarray Palette_Array;              /* create array to hold DAC values */
RGB   ColorValue;
float hue,sat,val;
int   count;

/* prototypes */

float frandom(void);
int tt;


int main()
{
char c;
int i, newz;
unsigned char gval;
unsigned seed;
int dsp = 1;
int numdsp;
int xmid, ymid;
u_long eaddr, ready_flag, done_flag;

  clrscr();
  printf("\n3dfrac version 3.0 by Aaron Contorer, 1987-1989\n");
  printf("This is a graphics demo program that creates images resembling\n");
  printf("landscapes using three-dimensional fractal geometry.  These\n");
  printf("images are generated as you watch, using random numbers and a\n");
  printf("simple mathematical formula.\n\n");
  printf("This program works only on machines with EGA or VGA graphics\n");
  printf("cards.  You are free to copy it if you wish.\n");
  printf("Press <space> to run it now, or <esc> to cancel.\n");
  printf("Once the program is running, presssing any key will stop it.\n\n");
  printf("Author:  Aaron Contorer, president, Contorer Computing\n");
  printf("Mailing address:  Post Office Box 5056, Champaign, IL 61825, USA\n");

  printf("\n\n\nConverted to Turbo C and added Super VGA 256 color support\n");
  printf("              - Reagan Thomas of Thomas Design\n");
  
#ifndef NOCC
	 installuserdriver("ISVGA256", 0L);
	 g_driver = 134;
	 g_mode = 13;
	 initgraph(&g_driver, &g_mode,"");

      g_error = graphresult();
	 if (g_error != 0) {
	    printf("%s \n",grapherrormsg(g_error));
	    exit(1);
	 }

	 ybottom = getmaxy()-38;

	 watercolor = 46;
	 landcolor = GREEN;

      hue = 0;
      sat = 1.0;
      val = 63.0;
      for (i=1;  i<100;  i++) {
         hsv2rgb(hue,sat,val,&ColorValue);
	    Palette_Array[i][red] = ColorValue.Red;
	    Palette_Array[i][grn] = ColorValue.Green;
	    Palette_Array[i][blu] = ColorValue.Blue;
	    hue = hue + 3.0;
      }
      hue = 0;
      sat = 1.0;
      val = 43.0;
      for (i=100;  i<177;  i++) {       /* blueish-green */
         hsv2rgb(hue,sat,val,&ColorValue);
	    Palette_Array[i][red] = ColorValue.Red;
	    Palette_Array[i][grn] = ColorValue.Green;
	    Palette_Array[i][blu] = ColorValue.Blue;
         hue = hue + 2.80;
      }
      for (i=177;  i<183;  i++) {       /* green */
         hsv2rgb(hue,sat,val,&ColorValue);
	    Palette_Array[i][red] = ColorValue.Red;
	    Palette_Array[i][grn] = ColorValue.Green;
	    Palette_Array[i][blu] = ColorValue.Blue;
	    hue = hue + 14;
      }
      hue = 299.6;                      /* light brown */
      for (i=183;  i<201;  i++) {
         hsv2rgb(hue,sat,val,&ColorValue);
	    Palette_Array[i][red] = ColorValue.Red;
	    Palette_Array[i][grn] = ColorValue.Green;
	    Palette_Array[i][blu] = ColorValue.Blue;
	    hue = hue + 1.4;
    }
    /*val = 38.0; */
    for (i=201;  i<222;  i++) {         /* reddish-brown */
        hsv2rgb(hue,sat,val,&ColorValue);
	   Palette_Array[i][red] = ColorValue.Red;
	   Palette_Array[i][grn] = ColorValue.Green;
	   Palette_Array[i][blu] = ColorValue.Blue;
	   hue = hue + 0.6;
        /*sat = sat - 0.03; */
    }
    Palette_Array[0][red]     = 1;     /* Set first DAC register to black */
    Palette_Array[0][grn]     = 2;
    Palette_Array[0][blu]     = 3;

    for(i=223, gval=0x20; i < 255; i++, gval++) {
       Palette_Array[i][red]     = gval;     /* Set first DAC register to black */
       Palette_Array[i][grn]     = gval;
       Palette_Array[i][blu]     = gval;
    }

    Palette_Array[255][red]   = 0x3f;  /* Set last DAC register to white */
    Palette_Array[255][grn]   = 0x3f;
    Palette_Array[255][blu]   = 0x3f;
    dacpalette(Palette_Array);      /* load DAC registers with new colors */

    _AX = 0;

    geninterrupt(0x1a);	/* get lo word of timer tick count */
    seed = _DX;
#endif
//seed = 1;

    srand(seed);
    numdsp = vc40map(0xf000);

    printf("Attached %d-headed Hydra\n", numdsp);
    getch();
//numdsp = 1;
    for (dsp=1; dsp<=numdsp; dsp++) {
        c40_halt(dsp);
        if (dsp==1) {
            c40_load(dsp, "landdsp.x40", &eaddr, NUMSYMS, symnames, symtab);
        }
        else {
            c40_load(dsp, "landdsp.x40", &eaddr, 0, NULL, NULL);
        }
    }
printf("DSPs are loaded\n");
    for (i=0; i<NUMSYMS; i++) {
    	if (symtab[i].type == T_UNDEF) {
    		printf("can't find symbol `%s'\n", symnames[i]);
    		exit(1);
    	}
    }
    
    if (rand() < 16384) {
        newz = ZA + (int)((rand() / 32768.0) * ((ybottom-YA)* steep));
    }
    else {
        newz = ZA - (int)((rand() / 32768.0) * ((ybottom-YA)* steep));
    }
    for (dsp=1; dsp<=numdsp; dsp++) {
printf("Sending parameters to DSP %d\n", dsp);

#ifdef USE_INT
        c40_enint(dsp, 2L, 0x50L);
#endif
c40_get_long(dsp, symtab[DONE_FLAG].val.l, &done_flag);
printf("done flag: 0x%lx\n", done_flag);
        c40_run(dsp, eaddr);

        c40_put_long(dsp, symtab[INTPRI].val.l, 2L);
        c40_put_long(dsp, symtab[INTVEC].val.l, 0x50L);
        c40_put_long(dsp, symtab[YBOTTOM].val.l, (long) ybottom);
        c40_put_long(dsp, symtab[WATERCOLOR].val.l, (long) watercolor);
        c40_put_long(dsp, symtab[LANDCOLOR].val.l, (long) landcolor);
#ifdef GOOD
        c40_put_long(dsp, symtab[DEEP].val.l, (long) deep);
        c40_put_long(dsp, symtab[IX0].val.l, (long) XA);
        c40_put_long(dsp, symtab[IY0].val.l, (long) YA);
        c40_put_long(dsp, symtab[IX2].val.l, (long) XB);
        c40_put_long(dsp, symtab[IY2].val.l, (long) ybottom);
        c40_put_long(dsp, symtab[IZ0].val.l, (long) ZA);
        c40_put_long(dsp, symtab[IZ1].val.l, (long) ZA);
        c40_put_long(dsp, symtab[IZ2].val.l, (long) ZA);
        c40_put_long(dsp, symtab[IZ3].val.l, (long) ZA);
#else
        c40_put_long(dsp, symtab[DEEP].val.l, (long) deep-1);
        xmid = (XA + XB) / 2;
        ymid = (YA + ybottom) / 2;
        switch(dsp) {
            case 1: {
printf("dsp 1\n");
                c40_put_long(dsp, symtab[IX0].val.l, (long) XA);
                c40_put_long(dsp, symtab[IY0].val.l, (long) YA);
                c40_put_long(dsp, symtab[IX2].val.l, (long) xmid);
                c40_put_long(dsp, symtab[IY2].val.l, (long) ymid);
                c40_put_long(dsp, symtab[IZ0].val.l, (long) ZA);
                c40_put_long(dsp, symtab[IZ1].val.l, (long) ZA);
                c40_put_long(dsp, symtab[IZ2].val.l, (long) newz);
                c40_put_long(dsp, symtab[IZ3].val.l, (long) ZA);
	        c40_put_long(dsp, symtab[SEED].val.l, (long) rand());
                break;
            }

            case 2: {
printf("dsp 2\n");
                c40_put_long(dsp, symtab[IX0].val.l, (long) xmid);
                c40_put_long(dsp, symtab[IY0].val.l, (long) YA);
                c40_put_long(dsp, symtab[IX2].val.l, (long) XB);
                c40_put_long(dsp, symtab[IY2].val.l, (long) ymid);
                c40_put_long(dsp, symtab[IZ0].val.l, (long) ZA);
                c40_put_long(dsp, symtab[IZ1].val.l, (long) ZA);
                c40_put_long(dsp, symtab[IZ2].val.l, (long) ZA);
                c40_put_long(dsp, symtab[IZ3].val.l, (long) newz);
	        c40_put_long(dsp, symtab[SEED].val.l, (long) rand());
                break;
            }
                
            case 3: {
                c40_put_long(dsp, symtab[IX0].val.l, (long) XA);
                c40_put_long(dsp, symtab[IY0].val.l, (long) YA);
                c40_put_long(dsp, symtab[IX2].val.l, (long) (XA+XB)>>1);
                c40_put_long(dsp, symtab[IY2].val.l, (long) (YA+ybottom)>>1);
                c40_put_long(dsp, symtab[IZ0].val.l, (long) ZA);
                c40_put_long(dsp, symtab[IZ1].val.l, (long) ZA);
                c40_put_long(dsp, symtab[IZ2].val.l, (long) newz);
                c40_put_long(dsp, symtab[IZ3].val.l, (long) ZA);
	        c40_put_long(dsp, symtab[SEED].val.l, (long) rand());
                break;
            }
                
            case 4: {
                c40_put_long(dsp, symtab[IX0].val.l, (long) XA);
                c40_put_long(dsp, symtab[IY0].val.l, (long) YA);
                c40_put_long(dsp, symtab[IX2].val.l, (long) (XA+XB)>>1);
                c40_put_long(dsp, symtab[IY2].val.l, (long) (YA+ybottom)>>1);
                c40_put_long(dsp, symtab[IZ0].val.l, (long) ZA);
                c40_put_long(dsp, symtab[IZ1].val.l, (long) ZA);
                c40_put_long(dsp, symtab[IZ2].val.l, (long) newz);
                c40_put_long(dsp, symtab[IZ3].val.l, (long) ZA);
	        c40_put_long(dsp, symtab[SEED].val.l, (long) rand());
                break;
            }

        }   /* switch */
        c40_put_long(dsp, symtab[SEED].val.l, (long) seed);
        
#endif
    }
    getch();

    while (1) {
	cleardevice();
	steep = (frandom() / 2.5) + 0.55;
	sealevel = (int)(17*frandom()) - 8;
	for (dsp=1; dsp<=numdsp; dsp++) {
            c40_put_dsp_float(dsp, symtab[STEEP].val.l, steep);
            c40_put_long(dsp, symtab[SEALEVEL].val.l, sealevel);
            c40_put_long(dsp, symtab[DONE_FLAG].val.l, 0L);
            c40_put_long(dsp, symtab[START_FLAG].val.l, 1L);
        }
        
        dsps_running = numdsp;
        while (dsps_running > 0) {

#ifdef USE_INT
	    if (!c40_wait_int(1, 1000)) break;  /* wait for interrupt */
#endif
            for (dsp=1; dsp<=numdsp; dsp++) {
                c40_get_long(dsp, symtab[READY_FLAG].val.l, &ready_flag);
                if (ready_flag) {
                    process_dsp(dsp);
                }
                kbhit();
	    }
	}
printf("\a");
        if (kbhit()) {
            if (getch() == 27) break;
        }
	while (!kbhit()) ; 
	if (getch() == 27) break;
    }
    restorecrtmode();
    return(0);
}

void process_dsp(int dsp)
{
    u_long  done_flag, nlval;
    
    /*
     * first check to see if this DSP is done, if so decrement the
     * active DSP count and kick out - clear READY_FLAG so that we don't come
     * back here until next fractal is run
     */
    c40_get_long(dsp, symtab[DONE_FLAG].val.l, &done_flag);
    if (done_flag) {
        dsps_running--;
        c40_put_long(dsp, symtab[READY_FLAG].val.l, 0L);
        return ;
    }
    
    /*
     * read the number of lines to draw and the vector enpoints
     */
    c40_get_long(dsp, symtab[NL].val.l, &nlval);
    c40_read_long(dsp, symtab[LINES].val.l, (u_long *)lines, 3*nlval);
    
    /*
     * Tell DSP to start next batch while we draw the lines
     */
    c40_put_long(dsp, symtab[READY_FLAG].val.l, 0L);

    draw_lines(nlval);
}

void draw_lines(int nlines)
{
    int i;
    struct line3d *tline;
    
    for (i=0; i<nlines; i++) {
        tline = &lines[i];
        if (tline->pflag) {
            putpixel(tline->x0, tline->y0, tline->color);
        }
        else {
            setcolor(tline->color);
            line(tline->x0, tline->y0, tline->x1, tline->y1);
        }
    }
}
            

/*----------------- RGB colors from HSV model ---------------------------*/
/* hue    =  pure color of the light
   sat    =  how much white light is mixed in
   value  =  max component of RGB */

void hsv2rgb(float h,float s,float v,RGB *Color) {

float h1,f,a[7];
int   i;

 h1 = h / 60;
 i  = h1;
 f  = h1 - i;
 a[1] = v;
 a[2] = v;
 a[3] = v * (1 - (s*f));
 a[4] = v * (1 - s);
 a[5] = a[4];
 a[6] = v * (1-(s*(1-f)));
 if (i>4) i = i - 4; else i = i + 2;
 Color->Red = a[i];
 if (i>4) i = i - 4; else i = i + 2;
 Color->Green = a[i];
 if (i>4) i = i - 4; else i = i + 2;
 Color->Blue  = a[i];
}

float frandom(void) { /* Return random value 0 <= x < 1 */

 return rand() / 32768.0;
}





