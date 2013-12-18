/****************************************************************/
/*                          Ariel Corp.                         */
/*                        433 River Road                        */
/*                Highland Park, NJ 08904, U.S.A.               */
/*                     Tel:  (908) 249-2900                     */
/*                     Fax:  (908) 249-2123                     */
/*                     BBS:  (908) 249-2124                     */
/*                  E-Mail:  ariel@ariel.com                    */
/*                                                              */
/*                 Copyright (C) 1992 Ariel Corp.               */
/****************************************************************/


/* mand.c 
 *
 *
 * Author: Timothy Andre
 *
 * This is a Mandelbrot set calculation/display program.  This module contains
 * the generic drawing routines and host CPU calculation routines (i.e. the
 * DSP independent routines).  See the accompanying README file for the
 * care and feeding of mand32c.
 */

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
#include "mand_dsp.h"

#define CMAP_SIZE	128	/* number of colormap entries */

#define	WIDTH	350	/* default width of the drawing window */
#define HEIGHT	350	/* default height of the drawing window */

#define	XMIN	-2.3	/* initial set coordinates */
#define	XMAX	 0.8
#define	YMIN	-1.3
#define	YMAX	 1.3

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
int	maxit = DSP_MAXIT;
float  xmin = XMIN;
float  xmax = XMAX;
float  ymin = YMIN;
float  ymax = YMAX;
static int	done_drawing = 1;

#define	HOST_CPU	0
#define	DSP		1
int	engine_choice_val = DSP;
int	maxit;

GC	gc;	/* X11 Graphics context */
unsigned long *pixel_table;

Display	*dpy;
Frame	frame;
Canvas	canvas;
Panel_item	iter_item, engine_choice_item, dsp_time, sparc_time;

static void zoom_proc();
static void draw_mandelbrot();
static void draw_mandelbrot_sparc();
static void engine_proc();
static void clear_canvas();
static void clear_times();
static void dummy();
static int iter_proc();

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
	 * process command line arguments to look for a different device
	 */
	getsopt(1, argc, argv, options, option_on, option_arg);

	if (option_on[DEV_OPT])
		strcpy(devname, option_arg[DEV_OPT]);

        /*
         * define colormap
         */
	if (!findfile("colormap", "VC40PATH", cmap_path)) {
		printf("Could not find colormap file\n");
		exit(1);
	}
        if (!(cmapfile = fopen(cmap_path, "r"))) {
                printf("could not open colormap file\n");
                exit(1);
        }
        for (i=0; i<CMAP_SIZE; i++) {
                fscanf(cmapfile, "%d%d%d", &r, &g, &b);
                colors[128 - i].red = r;
                colors[128 - i].green = g;
                colors[128 - i].blue = b;
        }
	colors[0].red = colors[0].green = colors[0].blue = 255;
        fclose(cmapfile);

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

	iter_item = (Panel_item) xv_create(panel, PANEL_NUMERIC_TEXT,
		PANEL_LABEL_STRING, "Iterations:",
		PANEL_VALUE, maxit,
		PANEL_MIN_VALUE, 10,
		PANEL_MAX_VALUE, 10000,
		PANEL_LAYOUT, PANEL_VERTICAL,
		PANEL_VALUE_DISPLAY_LENGTH, 6,
		PANEL_VALUE_STORED_LENGTH, 6,
		PANEL_NOTIFY_PROC, iter_proc,
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

	xv_set(canvas_paint_window(canvas),
		WIN_EVENT_PROC, zoom_proc,
		WIN_CONSUME_EVENTS, LOC_DRAG, NULL,
		NULL);

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
	if (!init_dsp(devname, PROGNAME)) exit(1);

	xv_main_loop(frame);
}

static int	drawn = 0;

void repaint_proc()
{
	if (!drawn) return;

	nx = (int) xv_get(canvas, XV_WIDTH);
	ny = (int) xv_get(canvas, XV_HEIGHT);
	draw_mandelbrot();
}

void quit()
{
	close_dsp();
	xv_destroy_safe(frame);
}

void draw_button()
{
	drawn = 1;
	xmin = XMIN;
	xmax = XMAX;
	ymin = YMIN;
	ymax = YMAX;
	clear_times();
	clear_canvas();
	draw_mandelbrot();
}

static int iter_proc()
{
	if (drawn) {
		draw_mandelbrot();
	}
	return (PANEL_NONE);
}

/*
 * the zoom_proc() function allows the drawing of a box around the
 * area of interest on the display.  It the redraws the set using the
 * new cooridnates defined by the box.
 */
static void zoom_proc(window, event, arg)
Xv_Window	window;
Event		*event;
Notify_arg	arg;
{
	static int	flag = 0;
	static int	first_time = 1;
	static int	ox, oy = 20;
	static int	x, y;
	static int	rx, lx, ty, by;
	static XImage	*tline, *bline, *lline, *rline;
	int	tmp;
	double	xm, ym, nxmin, nymin, nxmax, nymax;
	Xv_Window pw = canvas_paint_window(canvas);
	Window	win = (Window) xv_get(pw, XV_XID);

	/*
	 * if window is resized smaller, then WIN_RESIZE alone is issued
	 * and the repaint procedure will not be called automatically,
	 * so look for it here.  If the window is resized larger,
	 * both WIN_RESIZE and WIN_REPAINT are issued and the repaint
	 * procedure is called automatically
	 */
	if (event_id(event) == WIN_RESIZE) {
		int newx = (int) xv_get(canvas, XV_WIDTH);
		int newy = (int) xv_get(canvas, XV_HEIGHT);
		if (newx <= nx && newy <= ny)
			repaint_proc();

		return;
	}
	if (event_id(event) != LOC_DRAG && event_id(event) != MS_LEFT)  return;

	if (!flag) {
		flag = 1;
		ox = event_x(event);
		oy = event_y(event);
		first_time = 1;
	}
	else {
		if (!first_time) {
			XPutImage(dpy, win, gc, tline, 
				0, 0, lx, ty, rx-lx+1, 1);
			XDestroyImage(tline);
			XPutImage(dpy, win, gc, bline, 
				0, 0, lx, by, rx-lx+1, 1);
			XDestroyImage(bline);
			XPutImage(dpy, win, gc, lline,
				0, 0, lx, ty, 1, by-ty+1);
			XDestroyImage(lline);
			XPutImage(dpy, win, gc, rline,
				0, 0, rx, ty, 1, by-ty+1);
			XDestroyImage(rline);

		}
		if (event_id(event) == LOC_DRAG) {
			first_time = 0;
			x = event_x(event);
			y = event_y(event);
			if (x <= ox) {
				lx = x;
				rx = ox;
			}
			else {
				lx = ox;
				rx = x;
			}

			if (y <= oy) {
				ty = y;
				by = oy;
			}
			else {
				ty = oy;
				by = y;
			}
			tline = XGetImage(dpy, win, lx, ty, rx-lx+1, 1, 
				AllPlanes, ZPixmap);
			bline = XGetImage(dpy, win, lx, by, rx-lx+1, 1, 
				AllPlanes, ZPixmap);
			lline = XGetImage(dpy, win, lx, ty, 1, by-ty+1,
				AllPlanes, ZPixmap);
			rline = XGetImage(dpy, win, rx, ty, 1, by-ty+1,
				AllPlanes, ZPixmap);
			XSetForeground(dpy, gc, pixel_table[LINE_COLOR]);
			XDrawLine(dpy, win, gc, lx, ty, rx, ty);
			XDrawLine(dpy, win, gc, lx, by, rx, by);
			XDrawLine(dpy, win, gc, lx, ty, lx, by);
			XDrawLine(dpy, win, gc, rx, ty, rx, by);
		}
		else {
			char	time[20];

			flag = 0;
			xm = (xmax - xmin) / (nx - 1);
			ym = (ymin - ymax) / (ny - 1);
			nxmin = xmin + lx*xm;
			nxmax = xmin + rx*xm;
			nymin = ym*by + ymax;
			nymax = ym*ty + ymax;
			xmax = nxmax;
			xmin = nxmin;
			ymax = nymax;
			ymin = nymin;
			clear_times();
			draw_mandelbrot();
		}
	}
}

int get_maxit()
{
	return ((int) xv_get(iter_item, PANEL_VALUE));
}

static void draw_mandelbrot()
{
	char	item[10], time[20];
	void	(*mandelbrot_func)();
	struct itimerval        start_time;
	Panel_item	time_item;

	if (engine_choice_val == DSP) {
		mandelbrot_func = draw_mandelbrot_dsp;
		time_item = dsp_time;
		strcpy(item, BOARDNAME);
	}
	else {
		mandelbrot_func = draw_mandelbrot_sparc;
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
	mandelbrot_func();

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


void draw_mandelbrot_line(line, y)
PIXEL_TYPE *line;
int y;
{
	int	x, ox;
	PIXEL_TYPE oval;
	Xv_Window pw = canvas_paint_window(canvas);
	Window	win = (Window) xv_get(pw, XV_XID);

	ox = 0;
	oval = *line;
	for (x=0; x<nx; x++, line++) {
		if (oval != *line) {
			XSetForeground(dpy, gc, pixel_table[oval]);
			if (x-ox > 1) {
				XDrawLine(dpy, win, gc, ox, y, x-1, y);
			}
			else {
				XDrawPoint(dpy, win, gc, x-1, y);
			}
			ox = x;
			oval = *line;
		}
	}
	XSetForeground(dpy, gc, pixel_table[oval]);
	XDrawLine(dpy, win, gc, ox, y, nx-1, y);
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
		draw_mandelbrot();
	}
}

static void draw_mandelbrot_sparc()
{
	int	x, y, it;
	PIXEL_TYPE	line[MAXLINE];
	float	a, b;
	float	xstep = (xmax - xmin) / nx;
	float	ystep = (ymax - ymin) / ny;
	float	real, imag, r2, i2, mag;

	for (y=0; y<ny; y++) {
		b = ymin + y*ystep;
		for (x=0; x<nx; x++) {
			a = xmin + x*xstep;
			real = a;
			imag = b;
			for (it=125; it>1; it--) {
				mag = (r2=real*real) + (i2=imag*imag);
				if (mag > 4.0) break;
				imag = 2*real*imag + b;
				real = r2 - i2 + a;
			}
			line[x] = it;
		}
		draw_mandelbrot_line(line, ny-y-1);
	}
	stop_timing();
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
