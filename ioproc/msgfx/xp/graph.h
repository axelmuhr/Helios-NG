/***
*	graph.h - declare constants and functions for graphics library
*
*	Copyright: Perihelion Software Ltd. All rights reserved 1989
*
*	Purpose: This file declares the graphics library functions and
*		 the manifest constants that are used with them.
*
*******************************************************************************/

#ifndef _graph_h_
#define _graph_h_

/* user-visible declarations for Quick-C Graphics Library */

#ifndef _GRAPH_T_DEFINED

/* structure for _getvideoconfig() as visible to user */
struct videoconfig {
        short numxpixels;       /* number of pixels on X axis */
        short numypixels;       /* number of pixels on Y axis */
        short numtextcols;      /* number of text columns available */
        short numtextrows;      /* number of text rows available */
        short numcolors;        /* number of actual colors */
        short bitsperpixel;     /* number of bits per pixel */
        short numvideopages;    /* number of available video pages */
        short mode;             /* current video mode */
        short adapter;          /* active display adapter */
        short monitor;          /* active display monitor */
        short memory;           /* adapter video memory in K bytes */
};

/* return value of _setlogorg(), etc. */
struct xycoord {
        short xcoord;
        short ycoord;
};

/* structure for text position */
struct rccoord {
        short row;
        short col;
};

#define _GRAPH_T_DEFINED

#endif


/* SETUP AND CONFIGURATION */

int _setvideomode(int);

/* arguments to _setvideomode() */
#define _MAXRESMODE     -3      /* graphics mode with highest resolution */
#define _MAXCOLORMODE   -2      /* graphics mode with most colors */
#define _DEFAULTMODE    -1      /* restore screen to original mode */
#define _TEXTBW40       0       /* 40 x 25 text, 16 grey */
#define _TEXTC40        1       /* 40 x 25 text, 16/8 color */
#define _TEXTBW80       2       /* 80 x 25 text, 16 grey */
#define _TEXTC80        3       /* 80 x 25 text, 16/8 color */
#define _MRES4COLOR     4       /* 320 x 200, 4 color */
#define _MRESNOCOLOR    5       /* 320 x 200, 4 grey */
#define _HRESBW         6       /* 640 x 200, BW */
#define _TEXTMONO       7       /* 80 x 25 text, BW */
#define _HERCMONO       8       /* 720 x 348, BW for HGC */
#define _MRES16COLOR    13      /* 320 x 200, 16 color */
#define _HRES16COLOR    14      /* 640 x 200, 16 color */
#define _ERESNOCOLOR    15      /* 640 x 350, BW */
#define _ERESCOLOR      16      /* 640 x 350, 4 or 16 color */
#define _VRES2COLOR     17      /* 640 x 480, BW */
#define _VRES16COLOR    18      /* 640 x 480, 16 color */
#define _MRES256COLOR   19      /* 320 x 200, 256 color */

void _setactivepage(int);
void _setvisualpage(int);

/* videoconfig adapter values */
/* these manifest constants can be used to determine the type of monitor in */
/* use, using either simple comparisons or the bitwise-AND operator (&) */
#define _MDPA       0x0001      /* Monochrome Display Adapter (MDPA) */
#define _CGA        0x0002      /* Color Graphics Adapter     (CGA)  */
#define _EGA        0x0004      /* Enhanced Graphics Adapter  (EGA)  */
#define _VGA        0x0008      /* Video Graphics Array       (VGA)  */
#define _MCGA       0x0010      /* MultiColor Graphics Array  (MCGA) */
#define _HGC        0x0020      /* Hercules Graphics Card     (HGC)  */

/* videoconfig monitor values */
/* these manifest constants can be used to determine the type of the active */
/* adapter, using either simple comparisons or the bitwise-AND operator (&) */
#define _MONO       0x0001      /* Monochrome */
#define _COLOR      0x0002      /* Color (or Enhanced emulating color) */
#define _ENHCOLOR   0x0004      /* Enhanced Color */
#define _ANALOG     0x0018      /* Analog */

struct videoconfig * _getvideoconfig(struct videoconfig *);


/* COORDINATE SYSTEMS */

struct xycoord _setlogorg(int, int);
struct xycoord _getlogcoord(int, int);
struct xycoord _getphyscoord(int, int);

void _setcliprgn(int, int, int, int);
void _setviewport(int, int, int, int);


/* OUTPUT ROUTINES */

/* control parameters for Rectangle, Ellipse and Pie */
#define _GBORDER        2       /* draw outline only */
#define _GFILLINTERIOR  3       /* fill using current fill mask */

#define _GCLEARSCREEN 0
#define _GVIEWPORT    1
#define _GWINDOW      2

void _clearscreen(int);

void _moveto(int, int);
void _lineto(int, int);

struct xycoord _getcurrentposition(void);

int _rectangle(int, int, int, int, int);
int _ellipse(int, int, int, int, int);
int _pie(int, int, int, int, int, int, int, int, int);

void _arc(int, int, int, int, int, int, int, int);
void _setpixel(int, int);

int _getpixel(int, int);
int _floodfill(int, int, int);


/* PEN COLOR, LINE STYLE, FILL PATTERN */

void _setcolor(int);
int _getcolor(void);

void _setlinestyle(unsigned int);
unsigned int _getlinestyle(void);

void _setfillmask(unsigned char *);
unsigned char * _getfillmask(unsigned char *);

/* COLOR SELECTION */

int _setbkcolor(int);
int _getbkcolor(void);

int _remappalette(int, int);
int _remapallpalette(long *);
int _selectpalette(int);


/* TEXT */
#define _GCURSOROFF 0
#define _GCURSORON  1

#define _GWRAPOFF   0
#define _GWRAPON    1

void _settextwindow(int, int, int, int);
void _outtext(char *);
int _wrapon(int);
int _displaycursor(int);

void _settextposition(int, int);
struct rccoord _gettextposition(void);

void _settextcolor(int);
int _gettextcolor(void);


/* SCREEN IMAGES */
int _imagesize(int, int, int, int);
void _getimage(int, int, int, int, char *);
void _putimage(int, int, char *, int);

/* "action verbs" for _putimage() */
#define _GPSET          3
#define _GPRESET        2
#define _GAND           1
#define _GOR            0
#define _GXOR           4

/* universal color values: */
#define _BLACK          0x000000L
#define _BLUE           0x2a0000L
#define _GREEN          0x002a00L
#define _CYAN           0x2a2a00L
#define _RED            0x00002aL
#define _MAGENTA        0x2a002aL
#define _BROWN          0x00152aL
#define _WHITE          0x2a2a2aL
#define _GRAY           0x151515L
#define _LIGHTBLUE      0x3F1515L
#define _LIGHTGREEN     0x153f15L
#define _LIGHTCYAN      0x3f3f15L
#define _LIGHTRED       0x15153fL
#define _LIGHTMAGENTA   0x3f153fL
#define _LIGHTYELLOW    0x153f3fL
#define _BRIGHTWHITE    0x3f3f3fL

/* mono mode F color values: */
#define _MODEFOFF       0L
#define _MODEFOFFTOON   1L
#define _MODEFOFFTOHI   2L
#define _MODEFONTOOFF   3L
#define _MODEFON        4L
#define _MODEFONTOHI    5L
#define _MODEFHITOOFF   6L
#define _MODEFHITOON    7L
#define _MODEFHI        8L

/* mono mode 7 color values: */
#define _MODE7OFF       0L
#define _MODE7ON        1L
#define _MODE7HI        2L

#define WAIT 1
#define PASS 0

extern	void	FlushMSG( int type );
extern	int	dsize;
#endif

/***  end of graph.h  ***/
