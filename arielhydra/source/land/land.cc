



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

/*Graphic design*/

#define  DEEP 8
#define  XA 0
#define  XB 500
#define  YA 0
#define  ZA 0
#define  YADD 38

void addline(int x0, int y0, int z0, int x1, int y1, int z1);

/* Color indices
#define WATERCOLOR 1
#define LANDCOLOR 2 */

int WATERCOLOR, LANDCOLOR;

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
void waitabit(void);
int tt;


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



void frac(depth, x0,y0,x2,y2,z0,z1,z2,z3) {
int newz; /* new center point */
int xmid,ymid,z01,z12,z23,z30;

  if (kbhit()) return;

  if (rand() < 16384) /* 50% chance */
	 newz = (z0+z1+z2+z3) / 4 + (int)((rand() / 32768.0) * ((y2-y0)* steep));
  else
	 newz = (z0+z1+z2+z3) / 4 - (int)((rand() / 32768.0) * ((y2-y0)* steep));
  xmid = (x0+x2) >> 1;
  ymid = (y0+y2) >> 1;
  z12 = (z1+z2) >> 1;
  z30 = (z3+z0) >> 1;
  z01 = (z0+z1) >> 1;
  z23 = (z2+z3) >> 1;
  depth--;
  if (depth>=0 ) {
    frac(depth, x0,y0, xmid,ymid, z0,z01,newz,z30);
    frac(depth, xmid,y0, x2,ymid, z01,z1,z12,newz);
    frac(depth, x0,ymid, xmid,y2, z30,newz,z23,z3);
    frac(depth, xmid,ymid, x2,y2, newz,z12,z2,z23);
  } else {
    if (newz<=sealevel ) { /*above sea level*/
      /*L to R*/
      addline(xmid,ymid,newz, x2,ymid,z12);
      addline(xmid,ymid,newz, x0,ymid,z30);
    } else {
      /*below "sea level"*/
      addline(xmid,ymid,sealevel, 0,0,-9999);
    }
  }
}


void waitabit(void) {
   getch();
}


int main()
{
char c;
int i;
unsigned char gval;
unsigned seed;

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

  do {
    c=getch();
  } while (c != 27 && c != ' ');
  if (c==32 ) {

	 installuserdriver("ISVGA256",DetectISVGA256);
	 g_driver = DETECT;
	 initgraph(&g_driver, &g_mode,"");

      g_error = graphresult();
	 if (g_error != 0) {
	    printf("%s \n",grapherrormsg(g_error));
	    exit(1);
	 }

	 ybottom = getmaxy()-38;

	 WATERCOLOR = 46;
	 LANDCOLOR = GREEN;

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

    srand(seed);

    while (1) {
	 cleardevice();
	 steep = (frandom() / 2.5) + 0.55;
	 sealevel = (int)(17*frandom()) - 8;
	 frac(DEEP, XA,YA,XB,ybottom,ZA,ZA,ZA,ZA);
	 if (!kbhit())
	    waitabit();
	 else {
	    if(getch() == 0x1b)
		  break;
	 }
    }
    restorecrtmode();
  }
  return(0);
}

void addline(int x0, int y0, int z0, int x1, int y1, int z1)
{
    if (z1 == -9999) {
        putpixel((y0 >> 1) + x0, YADD + y0 + z0, WATERCOLOR);
    }
    else {
        tt = (abs(z1) >> 1) + 172;
	if(tt > 255) {
	    tt = 255;
	}
	setcolor(tt);
        line((y0 >> 1)+x0, YADD+y0+z0,(y1 >> 1)+x1, YADD+y1+z1);
    }
}


