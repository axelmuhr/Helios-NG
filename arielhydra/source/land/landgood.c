#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/file.h>
#include <sys/time.h>
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/cms.h>
#include <xview/xv_xrect.h>
#include <xview/win_input.h>
#include "land_dsp.h"

#define  XA 0
#define  XB 500
#define  YA 0
#define  ZA 0
#define	YADD	38

#define	RANDOM()	(random() >> 16)

#define CMAP_SIZE	256	/* number of colormap entries */

#define	WIDTH	800	/* default width of the drawing window */
#define HEIGHT	600	/* default height of the drawing window */

#define WHITE (0)	/* special color names in the colormap */
#define	BLACK (1)

#define MACHINENAME	"Sparc"

#define LINE_COLOR	WHITE

#define	LONGTIME	3600	/* an hour's worth of seconds */

/*
 * command line options
 */
#define	NUM_OPT 1
#define DEV_OPT	0
char *options = "d:";
int option_on[NUM_OPT] = {0};
char *option_arg[NUM_OPT];

/*
 * initialize global parameters
 */
int	nx = WIDTH;
int	ny = HEIGHT;

int	done_drawing = 1;
float steep;
int sealevel;
int ybottom = HEIGHT - 38;
int fdepth = DEPTH;
int watercolor=46, landcolor=80;
int minx = 5000;
int miny = 5000;
int maxx = 0;
int maxy = 0;
struct line3d lines[NLINES];


#define	HOST_CPU	0
#define	DSP		1
int	engine_choice_val = DSP;
int	maxit;

GC	gc;	/* X11 Graphics context */
unsigned long *pixel_table;

Display	*dpy;
Frame	frame;
Canvas	canvas;
Panel_item	depth_item, engine_choice_item, dsp_time, sparc_time;

void hsv2rgb();
static void zoom_proc();
static void draw_landscape();
static void draw_landscape_sparc();
static void engine_proc();
static void clear_canvas();
static void clear_times();
static void dummy();
void frac();
float frandom();

typedef struct {
	int Red, Green, Blue;
} RGB;

main(argc, argv)
int	argc;
char *argv[];
{
	char	time[20], devname[30], cmap_path[200];
	int	i, r, g, b;
	Panel	panel;
	XGCValues	gc_val;
	XGCValues	gcvalues;
	void draw_button();
	void	repaint_proc();
	void	quit();
	Cms	cms;
	static Xv_singlecolor	colors[CMAP_SIZE+1];
	FILE	*cmapfile;

	/*
	 * set up default device name (provide by mand_dsp.h)
	 */
	strcpy(devname, DEFAULT_DEVICE);

	/*
	 * random number seed
	 */
	srandom(getpid());

	/*
	 * process command line arguments to look for a different device
	 */
	getsopt(1, argc, argv, options, option_on, option_arg);

	if (option_on[DEV_OPT])
		strcpy(devname, option_arg[DEV_OPT]);

        /*
         * define colormap
         */
	make_colormap(colors);

	/*
	 * set up screen
	 */
	xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);

	cms = (Cms) xv_create(NULL, CMS,
		CMS_SIZE, CMAP_SIZE,
		CMS_COLORS, colors,
		NULL);

	frame = (Frame) xv_create(XV_NULL, FRAME,
		FRAME_LABEL, argv[0],
		NULL);

	dpy = (Display *) xv_get(frame, XV_DISPLAY);

	panel = (Panel) xv_create(frame, PANEL,
		PANEL_LAYOUT, PANEL_VERTICAL,
		XV_HEIGHT, ny,
		NULL);

	(void) xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Draw",
		PANEL_NOTIFY_PROC, draw_button, 
		NULL);

	(void) xv_create(panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Quit",
		PANEL_NOTIFY_PROC, quit, 
		NULL);

	depth_item = (Panel_item) xv_create(panel, PANEL_NUMERIC_TEXT,
		PANEL_LABEL_STRING, "Depth:",
		PANEL_VALUE, fdepth,
		PANEL_MIN_VALUE, 3,
		PANEL_MAX_VALUE, 16,
		PANEL_LAYOUT, PANEL_VERTICAL,
		PANEL_VALUE_DISPLAY_LENGTH, 6,
		PANEL_VALUE_STORED_LENGTH, 6,
/*		PANEL_NOTIFY_PROC, depth_proc, */
		PANEL_NOTIFY_LEVEL, PANEL_SPECIFIED,
		PANEL_NOTIFY_STRING, "\n\t\r",
		NULL);

	engine_choice_item = (Panel_item) xv_create(panel, PANEL_CHECK_BOX,
		PANEL_LAYOUT, PANEL_VERTICAL,
		PANEL_LABEL_STRING, "Engine:",
		PANEL_CHOICE_STRINGS, BOARDNAME, MACHINENAME, NULL,
		PANEL_VALUE, engine_choice_val,
		PANEL_NOTIFY_PROC, engine_proc,
		NULL);

	(void) xv_create(panel, PANEL_MESSAGE,
		PANEL_LABEL_STRING, "Elapsed Times",
		NULL);

	strcpy(time, BOARDNAME);
	strcat(time, ":");
	dsp_time = (Panel_item) xv_create(panel, PANEL_MESSAGE,
		PANEL_LABEL_STRING, time,
		NULL);

	strcpy(time, MACHINENAME);
	strcat(time, ":");
	sparc_time = (Panel_item) xv_create(panel, PANEL_MESSAGE,
		PANEL_LABEL_STRING, time,
		NULL);

	xv_create(panel, PANEL_MESSAGE, 
		PANEL_LABEL_STRING, "Using Device:",
		NULL);

	xv_create(panel, PANEL_MESSAGE, 
		PANEL_LABEL_STRING, devname,
		NULL);

	window_fit_width(panel);

	canvas = (Canvas) xv_create(frame, CANVAS,
		XV_WIDTH, nx,
		XV_HEIGHT, ny,
		CANVAS_X_PAINT_WINDOW, TRUE,
		CANVAS_REPAINT_PROC, repaint_proc,
		WIN_CMS, cms,
		NULL);
#ifdef NOCC
	xv_set(canvas_paint_window(canvas),
		WIN_EVENT_PROC, zoom_proc,
		WIN_CONSUME_EVENTS, LOC_DRAG, NULL,
		NULL);
#endif

	window_fit(canvas);
	window_fit(frame);

	pixel_table = (unsigned long *)xv_get(cms, CMS_INDEX_TABLE);

	gcvalues.graphics_exposures = False;
	gcvalues.background = pixel_table[0];
	gc = XCreateGC(xv_get(frame, XV_DISPLAY), xv_get(frame, XV_XID),
		GCBackground | GCGraphicsExposures, &gcvalues);

	/*
	 * Initialize the DSP
	 */
#ifdef USEDSP
	if (!init_dsp(devname, PROGNAME)) exit(1);
#endif

	xv_main_loop(frame);
}

/***************************************************************************
 * define color map
 */
make_colormap(Palette_Array)
Xv_singlecolor Palette_Array[];
{
	int	i, gval;
	float	hue, sat, val;
	RGB	ColorValue;

      hue = 0;
      sat = 1.0;
      val = 63.0;
      for (i=1;  i<100;  i++) {
         hsv2rgb(hue,sat,val,&ColorValue);
	    Palette_Array[i].red = ColorValue.Red;
	    Palette_Array[i].green = ColorValue.Green;
	    Palette_Array[i].blue = ColorValue.Blue;
	    hue = hue + 3.0;
      }
      hue = 0;
      sat = 1.0;
      val = 43.0;
      for (i=100;  i<177;  i++) {       /* blueish-green */
         hsv2rgb(hue,sat,val,&ColorValue);
	    Palette_Array[i].red = ColorValue.Red;
	    Palette_Array[i].green = ColorValue.Green;
	    Palette_Array[i].blue = ColorValue.Blue;
         hue = hue + 2.80;
      }
      for (i=177;  i<183;  i++) {       /* green */
         hsv2rgb(hue,sat,val,&ColorValue);
	    Palette_Array[i].red = ColorValue.Red;
	    Palette_Array[i].green = ColorValue.Green;
	    Palette_Array[i].blue = ColorValue.Blue;
	    hue = hue + 14;
      }
      hue = 299.6;                      /* light brown */
      for (i=183;  i<201;  i++) {
         hsv2rgb(hue,sat,val,&ColorValue);
	    Palette_Array[i].red = ColorValue.Red;
	    Palette_Array[i].green = ColorValue.Green;
	    Palette_Array[i].blue = ColorValue.Blue;
	    hue = hue + 1.4;
    }
    /*val = 38.0; */
    for (i=201;  i<222;  i++) {         /* reddish-brown */
        hsv2rgb(hue,sat,val,&ColorValue);
	   Palette_Array[i].red = ColorValue.Red;
	   Palette_Array[i].green = ColorValue.Green;
	   Palette_Array[i].blue = ColorValue.Blue;
	   hue = hue + 0.6;
        /*sat = sat - 0.03; */
    }
    Palette_Array[0].red     = 0;     /* Set first DAC register to black */
    Palette_Array[0].green     = 0;
    Palette_Array[0].blue     = 0;

    for(i=223, gval=0x20; i < 255; i++, gval++) {
       Palette_Array[i].red = 4*gval;  
       Palette_Array[i].green = 4*gval;
       Palette_Array[i].blue = 4*gval;
    }

    Palette_Array[255].red = 4*0x3f;  /* Set last DAC register to white */
    Palette_Array[255].green = 4*0x3f;
    Palette_Array[255].blue = 4*0x3f;
}

/***************************************************************************
 * RGB colors from HSV model
 * hue    =  pure color of the light
 * sat    =  how much white light is mixed in
 * value  =  max component of RGB
 */
void hsv2rgb(h, s, v, Color)
float h;
float s;
float v;
RGB *Color;
{

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
 Color->Red = 4*a[i];
 if (i>4) i = i - 4; else i = i + 2;
 Color->Green = 4*a[i];
 if (i>4) i = i - 4; else i = i + 2;
 Color->Blue  = 4*a[i];
}

static int	drawn = 0;

void repaint_proc()
{
	if (!drawn) return;

	nx = (int) xv_get(canvas, XV_WIDTH);
	ny = (int) xv_get(canvas, XV_HEIGHT);
	draw_landscape();
}

void quit()
{
/*	close_dsp(); */
	xv_destroy_safe(frame);
}

void draw_button()
{
	drawn = 1;
	clear_times();
	clear_canvas();
	draw_landscape();
}

int get_depth()
{
	return ((int) xv_get(depth_item, PANEL_VALUE));
}

static void draw_landscape()
{
	char	item[10], time[20];
	void	(*landscape_func)();
	struct itimerval        start_time;
	Panel_item	time_item;

	if (engine_choice_val == DSP) {
		landscape_func = draw_landscape_dsp;
		time_item = dsp_time;
		strcpy(item, BOARDNAME);
	}
	else {
		landscape_func = draw_landscape_sparc;
		time_item = sparc_time;
		strcpy(item, MACHINENAME);
	}

	strcpy(time, item);
	strcat(time, ": *");
	xv_set(time_item, PANEL_LABEL_STRING, time, NULL);

	start_time.it_interval.tv_sec = 0;
	start_time.it_interval.tv_usec = 0;
	start_time.it_value.tv_sec = LONGTIME;
	start_time.it_value.tv_usec = 0;
	notify_set_itimer_func(frame, dummy, ITIMER_REAL, &start_time, NULL);

	done_drawing = 0;
	landscape_func();

}

void stop_timing()
{
	char	time[20];
	float	etime, elapsed_time;
	struct itimerval	start_time, end_time;

	done_drawing = 1;
	/*
	 * read the value of the timer and reset it
	 */
	start_time.it_interval.tv_sec = 0;
	start_time.it_interval.tv_usec = 0;
	start_time.it_value.tv_sec = 0;
	start_time.it_value.tv_usec = 0;
	notify_set_itimer_func(frame, dummy, ITIMER_REAL, 
		&start_time, &end_time);

	etime = end_time.it_value.tv_sec + end_time.it_value.tv_usec/1e6;
	elapsed_time = ((float) LONGTIME) - etime;
	if (engine_choice_val == DSP) {
		sprintf(time, "%s: %6.2f", BOARDNAME, elapsed_time);
		xv_set(dsp_time, PANEL_LABEL_STRING, time, NULL);
	}
	else {
		sprintf(time, "%s: %6.2f", MACHINENAME, elapsed_time);
		xv_set(sparc_time, PANEL_LABEL_STRING, time, NULL);
	}
}

void draw_lines(nlines)
int nlines;
{
	int	i;
	struct line3d *tline;
	Xv_Window pw = canvas_paint_window(canvas);
	Window	win = (Window) xv_get(pw, XV_XID);

	for (i=0; i<nlines; i++) {
		tline = &lines[i];
		if (tline->pflag) {
			XSetForeground(dpy, gc, pixel_table[watercolor]);
			XDrawPoint(dpy, win, gc, tline->x0, tline->y0);
		}
		else {
			XSetForeground(dpy, gc, pixel_table[tline->color]);
			XDrawLine(dpy, win, gc, tline->x0, tline->y0, 
				tline->x1, tline->y1);
		}
	}
}

void addline(x0, y0, z0, x1, y1, z1)
int x0, y0, z0;
int x1, y1, z1;
{
	int	tt;
	int	bx, by, ex, ey;
	Xv_Window pw = canvas_paint_window(canvas);
	Window	win = (Window) xv_get(pw, XV_XID);

	if (z1 == -9999) {
		XSetForeground(dpy, gc, pixel_table[watercolor]);
		XDrawPoint(dpy, win, gc, (y0 >> 1) + x0, YADD + y0 + z0);
	}
	else {
bx = (y0 >> 1) + x0;
by = YADD+y0+z0;
ex = (y1 >> 1) + x1;
ey = YADD+y1+z1;
		tt = (abs(z1) >> 1) + 172;
		if (tt > 255) tt = 255;
		XSetForeground(dpy, gc, pixel_table[tt]);
		XDrawLine(dpy, win, gc, bx, by, ex, ey);
	}
}


static void engine_proc()
{
	int	value;

	if (!done_drawing) {
		xv_set(engine_choice_item, PANEL_VALUE, 
			engine_choice_val, NULL);
		return;
	}

	/*
	 * force the check box to behave like an exclusive choice, because
	 * I don't like the 3-D look of the choice buttons
	 */
	value = xv_get(engine_choice_item, PANEL_VALUE);
	if (value == engine_choice_val) return;
	if (value != 0) {
		value &= ~engine_choice_val;
		engine_choice_val = value;
	}
	xv_set(engine_choice_item, PANEL_VALUE, engine_choice_val, NULL);
	if (drawn) {
		clear_canvas();
		draw_landscape();
	}
}

static void draw_landscape_sparc()
{
	steep = (frandom() / 2.5) + 0.55;
	sealevel = (int)(17*frandom()) - 8;
	frac(fdepth, XA,YA,XB,ybottom,ZA,ZA,ZA,ZA);
	stop_timing();
}

void frac(depth, x0,y0,x2,y2,z0,z1,z2,z3) {
int newz; /* new center point */
int xmid,ymid,z01,z12,z23,z30;

  if (RANDOM() < 16384) /* 50% chance */
	 newz = (z0+z1+z2+z3) / 4 + (int)((RANDOM() / 32768.0) * ((y2-y0)* steep));
  else
	 newz = (z0+z1+z2+z3) / 4 - (int)((RANDOM() / 32768.0) * ((y2-y0)* steep));
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

static void clear_canvas()
{
	Xv_Window pw = canvas_paint_window(canvas);
	Window	win = (Window) xv_get(pw, XV_XID);

	XSetForeground(dpy, gc, pixel_table[WHITE]);
	XFillRectangle(dpy, win, gc, 0,0, nx, ny);
}

static void clear_times()
{
	char	time[20];

	strcpy(time, BOARDNAME);
	strcat(time, ":");
	xv_set(dsp_time, PANEL_LABEL_STRING, time, NULL);
	strcpy(time, MACHINENAME);
	strcat(time, ":");
	xv_set(sparc_time, PANEL_LABEL_STRING, time, NULL);
}

void register_signal(signum, sigfunc)
int signum;
void (*sigfunc)();
{
	notify_set_signal_func(frame, sigfunc, signum, NOTIFY_SYNC);
}

float frandom() 
{
	int	a;
	a = RANDOM();
	return (a/32768.0);
}

static void dummy()
{
}

/***************************************************************************
 * getsopt() - function to process command line options.
 *
 * Author: Timothy Andre
 *
 * Options are indicated on the command line by a leading `/' or `-'.  The
 * option name may be only one character long, and may be any valid ASCII
 * character.  Allowed options are taken from the string "options".  If the
 * option name in options is followed by a ":" then that option takes an
 * additional command line parameter.  Any characters immediately following
 * the option letter (with no intervening white space) are take to be the
 * option's argument.  If white space follows the option letter, then the
 * next command line parameter is taken to be the option's argument.
 * 
 * The option_on[] array contains one element for every allowed option.  It
 * should be initialized to all zeros.  If an option is specified, then the
 * corresponding element in option_on[] is set to 1.  Additionally, if the
 * option takes an additional parameter, the corresponding element in
 * character pointer array option_arg[] is set to point to the parameter.
 * 
 * The optind parameter indicates where on the command line to start
 * processing options.  This is usually initialized to 1, thus all options
 * must preceed arguments to the program.  This is not a hard and fast
 * rule, though. 
 * 
 * The special option "--" or "//" indicates that the rest of the command
 * line is to processed without searching for options, allowing the use of
 * the option character as an argument to the program.
 * 
 * Example:
 * 
 * In this example, the allowed options are a, b, c, D, e, and f.  The b
 * and f options each take an additional parameter.
 * 
 * #define	NUMOPTS	6
 * 
 * char	*options = "ab:cDef:"
 * char	*option_arg[NUMOPTS];
 * int	option_on[NUMOPTS] = {0};
 * int	optind = 1;
 * int	first_arg;
 * 
 * ...
 * 
 * first_arg = getlopt(optind, argc, argv, options, option_on, option_arg)
 * 
 * 
 * 
 * If the program were invoked as:
 * 
 * 	prog -fhi -ecD -b hello this is a test
 * 	
 * 
 * Then the options would be processed as follows:
 * 
 * 	option_on[0] = 0	"a" option not specified
 * 	option_on[1] = 1	"b" option is given
 * 	option_on[2] = 1	"c" option is given
 * 	option_on[3] = 1	"D" option is given
 * 	option_on[4] = 1	"e" option is given
 * 	option_on[5] = 1	"f" option is given
 * 	
 * 	*option_arg[1] = "hello"	"b" option's parameter
 * 	*option_arg[5] = "hi"		"f" option's parameter
 * 	
 * 	first_arg = 5		return value indicates the index of the
 * 				first argument to the program
 * 				
 * 
 ***************************************************************************/
 

#define FALSE	0
#define	TRUE	1

#define	SW_CHR1	'-'
#define	SW_CHR2	'/'

int getsopt(optind, argc, argv, options, option_on, option_arg)
int optind, argc, option_on[];
char *argv[], *options, *option_arg[];
{
char	tsw, *optptr, *tptr;
int	i, arg, opt;
int	optnum;
	
while ( optind < argc && ((tsw = argv[optind][0]) == SW_CHR1 ||
    tsw == SW_CHR2)) {

    optptr = argv[optind]+1;    /* ptr to option letter */

    /*
     * look for the special case "--" or "//"
     */
    if (*optptr == SW_CHR1 || *optptr == SW_CHR2) 
    	return(optind+1);
    
    /*
     * process all options in the current arv[] element
     */	
    while (*optptr != NULL) {
        tptr = options;             /* ptr to valid options */
        optnum = 0;
        
        /*
         * search for valid option
         */
        while (*tptr != NULL && *tptr != *optptr) {
            tptr++;
            if (*tptr == ':') tptr++;
            optnum++;
        }
        /*
         * check if option found
         */
        if (*tptr == NULL) {
            fprintf(stderr, "Unrecognized option `%c'\n", *optptr);
            exit(1);
        }
        
        /*
         * check if option needs an argument
         */
        tptr++;
        option_on[optnum] = TRUE;
        if (*tptr == ':') {     /* process option's argument */
            optptr++;
            if (*optptr == NULL) {
                optind++;
                if (optind >= argc) {
                    fprintf(stderr, "Option `%c' missing argument\n", *(optptr-1));
                    exit(1);
                }
                option_arg[optnum] = argv[optind];
   	    }
	    else {
	        option_arg[optnum] = optptr;
	    }
	    break;  /* investigate next argv[] element */
	}
	optptr++;   /* next character in current argv[] element */
    }
    optind++;   /* next arg[] element */
}
return optind;
}
