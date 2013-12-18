/*
 * xdemo.c : 	C source for the X front end to the Farm Library mandelbrot
 *
 *   Copyright (c) 1991 Perihelion Software Ltd.
 *     All rights reserved.
 *
 * Author :	N Clifton
 * Version :	$Revision: 1.1 $
 * Date :	$Date: 1992/09/09 12:00:26 $
 * Id :		$Id: mandelx.c,v 1.1 1992/09/09 12:00:26 bart Exp $
 */

/*
 * exported functions are ...
 *
 * int x_initialise(
 * 	unsigned int *	width_in_pixels,		
 *	unsigned int *	height_in_pixels,		
 *	unsigned char 	eight_colours_available[ 8 ]
 *      )	
 *
 * void x_finish( void )
 * 
 * void x_draw_scanline(
 *	unsigned int	y_coord,
 *	unsigned char *	colour_bytes
 *	)
 *
 * int x_get_new_coords(
 *	unsigned int *	top_left_x,
 *	unsigned int *	top_left_y,
 *	unsigned int *	bottom_right_x,
 *	unsigned int *	bottom_right_y
 *	)
 *
 */


/* header files */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

/* macros */

#ifndef min
#define min( a, b )	((a) < (b) ? (a) : (b))
#define max( a, b )	((a) > (b) ? (a) : (b))
#endif

#define get_colour( pix, name )	\
  XParseColor( dpy, cmap, name, &colour );	\
  \
  colour.pixel = pix;	\
  \
  XStoreColor( dpy, cmap, &colour );	\
  \
  if (invert)	\
  {	\
    colour.pixel = n + pix;	\
      \
    XStoreColor( dpy, cmap, &colour );	\
  }


/* constants */
/* external variables */
/* local variables */

static Display *	dpy      = NULL;	/* connection to the X sever 				*/
static Window		win      = 0;		/* window displayed on the X server 			*/
static Visual *		visual	 = NULL;	/* visual chosen for representing mandelbrot 		*/
static int		depth	 = 0;		/* depth of the chosen visual 				*/
static GC		line_gc  = 0;		/* graphics context for line     drawing opreations	*/
static GC		image_gc = 0;		/* graphics context for scanline drawing opreations	*/
static int		width    = 0;		/* widht of the window in pixels 			*/
static int		height   = 0;		/* height of the window in pixels 			*/
static XImage *		image    = NULL;	/* image structure used to draw scanlines		*/


/* functions */

static void
debug( const char * format, ... )
{
  va_list	args;
	

  va_start( args, format );

  fflush( stderr );
	
  fseek( stderr, 0L, SEEK_END );

  vfprintf( stderr, (char *)format, args );

  fputc( '\n', stderr );
	
  fflush( stderr );
	
  va_end( args );

  return;
  
} /* debug */


/*
 * x_initialise
 *
 * Initialises all of the X specific things for the demo.
 * It tries to create a window of the given size upon the X server.
 * (The real size of the window is returned).
 * In addition it allocates 8 colours and returns these in the array provided.
 * If the function succeeds it returns non-zero,
 * if it fails it returns zero.
 */

int
x_initialise(
	     unsigned int *	width_in_pixels,		/* value passed in and returned */
	     unsigned int *	height_in_pixels,		/* value passed in and returned */
	     unsigned char 	eight_colours_available[ 8 ]	/* values returned */
	     )
{
  Window	root;
  XVisualInfo 	vinfo;
  XVisualInfo *	pvinfo;
  unsigned long	mask;
  int		n;
  XClassHint	hint;
  XEvent	e;					
  int		screen;
  Colormap	cmap;
  

  if ((dpy = XOpenDisplay( NULL )) == NULL)
    {
      debug( "unable to open display %s", XDisplayName( NULL ) );

      return 0;
    }
  
  screen 	= DefaultScreen( dpy );
  root		= RootWindow( dpy, screen );
  
  vinfo.screen = screen;
  vinfo.class  = PseudoColor;
  
  mask = VisualScreenMask | VisualClassMask;
  
  if ((pvinfo = XGetVisualInfo( dpy, mask, &vinfo, &n )) == NULL)
    {
      cmap = DefaultColormap( dpy, screen );
      visual = DefaultVisual( dpy, screen );
      depth  = DefaultDepth(  dpy, screen );
    }
  else
    {
      int	n;


      visual = pvinfo[ 0 ].visual;
      depth  = pvinfo[ 0 ].depth;
      n      = pvinfo[ 0 ].colormap_size;
      
      cmap = XCreateColormap( dpy, root, pvinfo[ 0 ].visual, AllocAll );
      
      if (n >= 8)
	{
	  XColor	colour;
	  int		invert;


	  invert = (n >= 16);

	  n -= 8;
  
	  colour.flags = DoRed | DoGreen | DoBlue;

	  get_colour( 0, "red" );
	  get_colour( 1, "orange" );
	  get_colour( 2, "yellow" );
	  get_colour( 3, "green" );
	  get_colour( 4, "blue" );
	  get_colour( 5, "#7800e0" );
	  get_colour( 6, "violet" );
	  get_colour( 7, "black" );
	}
      else
	{
	  cmap   = DefaultColormap( dpy, screen );
	  visual = DefaultVisual( dpy, screen );
	  depth  = DefaultDepth(  dpy, screen );
	}
      
      XFree( (char *)pvinfo );
    }
  
  if (width_in_pixels == NULL)
    width = 640;
  else
    width = *width_in_pixels;

  if (height_in_pixels == NULL)
    height = 480;
  else
    height = *height_in_pixels;
  
  win = XCreateSimpleWindow( dpy, root, 0, 0, width, height, 2,
			      BlackPixel( dpy, screen ), WhitePixel( dpy, screen ) ); 
  
  if (win == NULL)
    {
      debug( "unable to create output window" );

      XCloseDisplay( dpy );

      return 0;
    }
  
  XSetWindowColormap( dpy, win, cmap );
  
  if ((line_gc = XCreateGC( dpy, win, NULL, 0 )) == NULL)
    {
      debug( "unable to create line drawing GC for window" );
    }
  
  if ((image_gc = XCreateGC( dpy, win, NULL, 0 )) == NULL)
    {
      debug( "unable to create scanline drawing GC for window" );
    }
  
  XSetLineAttributes( dpy, line_gc, 0, LineSolid, CapButt, JoinMiter );
  
  XSetFunction( dpy, line_gc, GXinvert );
  
  XStoreName( dpy, win, "Farm Library Mandelbrot Demonstration" );
  
  XSelectInput( dpy, win, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | KeyPressMask | StructureNotifyMask );
  
  XDefineCursor( dpy, win, XCreateFontCursor( dpy, XC_watch ) );
  
  hint.res_name	 = "mandelbrot";
  hint.res_class = "demo";
  
  XSetClassHint( dpy, win, &hint );
  
  XMapWindow( dpy, win );
  
  XSync( dpy, 0 );
  
  /*
   * wait until window mapped and fully visible
   */
  
  while (XNextEvent( dpy, &e ),
	 e.type != ConfigureNotify)
    {
      ;
    }
  
  width  = e.xconfigure.width;
  height = e.xconfigure.height;
  
  XSync( dpy, 0 );

  if (width_in_pixels != NULL)
    *width_in_pixels = width;

  if (height_in_pixels != NULL)
    *height_in_pixels = height;

  if (eight_colours_available != NULL)
    {
      eight_colours_available[ 0 ] = 0;
      eight_colours_available[ 1 ] = 1;
      eight_colours_available[ 2 ] = 2;
      eight_colours_available[ 3 ] = 3;
      eight_colours_available[ 4 ] = 4;
      eight_colours_available[ 5 ] = 5;
      eight_colours_available[ 6 ] = 6;
      eight_colours_available[ 7 ] = 7;      
    }
  
  return ~0;
  
} /* x_initialise */
  

/*
 * x_finish
 *
 * Cleanly shuts down the connection to the X server
 */

void
x_finish( void )
{
  if (dpy != NULL)
    {
      if (line_gc)
	{
	  XFreeGC( dpy, line_gc );

	  line_gc = 0;
	}

      if (image_gc)
	{
	  XFreeGC( dpy, image_gc );

	  image_gc = 0;
	}

      if (win)
	{
	  XDestroyWindow( dpy, win );

	  win = NULL;
	}

      XCloseDisplay( dpy );

      dpy = NULL;
    }

  return;
  
} /* x_finish */


/*
 * x_draw_scanline
 *
 * displays a scanline at the indicated pixel position
 *
 */

void
x_draw_scanline(
		unsigned int	y_coord,	/* pixel coordinate from top left hand corner of window */
		unsigned char *	colour_bytes	/* a width array of bytes from those returned by x_initialise */
		)
{
  if (y_coord >= height ||
      colour_bytes == NULL)
    return;
  
  if (image == NULL)
    {
      image = XCreateImage( dpy, visual, depth,
			   ZPixmap,
			   0,
			   (char *)colour_bytes,
			   width,
			   1, 8, 0 );
      
      if (image == NULL)
	{
	  debug( "failed to create image" );
	}
    }
  else
    {
      image->data = (char *)colour_bytes;
    }
  
  XPutImage( dpy, win, image_gc, image, 0, 0, 0, y_coord, width, 1 );
  
  XSync( dpy, 0 );
  
  return;
  
} /* x_draw_scanline */


static void
justify(
	int *	startx,
	int *	starty,
	int *	endx,
	int *	endy )
{
  int	tmp;
  
  
  if (*startx > *endx)
    {
      tmp	= *startx;
      *startx	= *endx;
      *endx	= tmp;
    }
  
  if (*starty > *endy)
    {
      tmp	= *starty;
      *starty	= *endy;
      *endy	= tmp;
    }
  
  if (*startx > *endx - 5)
    {
      *endx = *startx + 5;;
    }
  
  if (*starty > *endy - 5)
    {
      *endy = *starty + 5;
    }
  
} /* justify */


/*
 * x_get_new_coords
 *
 * Waits for the user to press a mouse button and then
 * allows the user to draw a "rubber band" box over the display
 * area.  When the mouse button is released the coordinates of
 * the chosen area are returned.  The function returns
 *
 *	-1	if the program should quit	(user pressed 'q')
 *	0	if the program should redisplay
 *		the full Mandelbrot set		(user pressed 't')
 *	1	if new coordinates were selected
 */

int
x_get_new_coords(
		 unsigned int *	top_left_x,
		 unsigned int *	top_left_y,
		 unsigned int *	bottom_right_x,
		 unsigned int *	bottom_right_y
		 )
{
  int		ret_val = -2;
  int		startx;
  int		starty;
  int		endx;
  int		endy;
  XPoint	points[ 5 ];
  XEvent	e;
  enum
    {
      waiting_for_press,
      waiting_for_release
    }
  state = waiting_for_press;


  /* verify input */

  if (top_left_x     == NULL ||
      top_left_y     == NULL ||
      bottom_right_x == NULL ||
      bottom_right_y == NULL )
    {
      return -1;
    }

  /* eat events before the loop starts */

  while (XCheckMaskEvent( dpy, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | KeyPressMask, &e ))
    ;

  /* change the cursor */
  
  XDefineCursor( dpy, win, XCreateFontCursor( dpy, XC_crosshair ) );

  XFlush( dpy );

  /* loop waiting for user input */
  
  while (ret_val == -2)
    {
      XNextEvent( dpy, &e );
      
      switch (e.type)
	{
	case KeyPress:
	  if (e.type == KeyPress)
	    {
	      int	c = XLookupKeysym( &e.xkey, 0 );
	      
	      
	      if (c == 'q')
		{
		  /* quit */
		  
		  ret_val = -1;
		}
	      else if (c == 't')
		{
		  /* go back to top level Mandelbrot */
		  
		  ret_val = 0;
		}
	      else if (c == 'l')
		{
		  /* redraw screen */
		  
		  *top_left_x     = 0;
		  *top_left_y     = 0;
		  *bottom_right_x = width - 1;
		  *bottom_right_y = height - 1;

		  ret_val = 1;
		}
	    }
	  
	  break;
	  
	case ConfigureNotify:
	  if (e.xconfigure.width  != width ||
	      e.xconfigure.height != height )
	    {
	      width  = e.xconfigure.width;
	      height = e.xconfigure.height;

	      /* redraw screen at new size */
	      
	      *top_left_x     = 0;
	      *top_left_y     = 0;
	      *bottom_right_x = width - 1;
	      *bottom_right_y = height - 1;

	      ret_val = 1;
	    }
	  
	  break;
	  
	case ButtonPress:				
	  
	  if (state == waiting_for_press)
	    {
	      startx	= e.xbutton.x;
	      starty	= e.xbutton.y;
	      
	      endx	= startx + 1;
	      endy	= starty + 1;
	      
	      points[ 0 ].x = startx; points[ 0 ].y = starty;
	      points[ 1 ].x = endx;   points[ 1 ].y = starty;
	      points[ 2 ].x = endx;   points[ 2 ].y = endy;
	      points[ 3 ].x = startx; points[ 3 ].y = endy;
	      points[ 4 ].x = startx; points[ 4 ].y = starty;
	      
	      XDrawLines( dpy, win, line_gc, points, 5, CoordModeOrigin );
	      
	      state = waiting_for_release;
	    }
	  
	  break;
	  
	case ButtonRelease:
	  
	  if (state == waiting_for_release)
	    {
	      endx = e.xbutton.x;
	      endy = e.xbutton.y;
	      
	      justify( &startx, &starty, &endx, &endy );
	      
	      XDrawLines( dpy, win, line_gc, points, 5, CoordModeOrigin );
	      
	      points[ 0 ].x = startx; points[ 0 ].y = starty;
	      points[ 1 ].x = endx;   points[ 1 ].y = starty;
	      points[ 2 ].x = endx;   points[ 2 ].y = endy;
	      points[ 3 ].x = startx; points[ 3 ].y = endy;
	      points[ 4 ].x = startx; points[ 4 ].y = starty;
	      
	      XDrawLines( dpy, win, line_gc, points, 5, CoordModeOrigin );
	      
	      *top_left_x     = min( startx, endx );
	      *top_left_y     = min( starty, endy );
	      *bottom_right_x = max( startx, endx );
	      *bottom_right_y = max( starty, endy );

	      ret_val = 1;
	    }
	  
	  break;
	  
	case MotionNotify:
	  if (state == waiting_for_release)
	    {
	      int	tmpstartx;
	      int	tmpstarty;
	      int	tmpendx;
	      int	tmpendy;
	      XEvent	ev;
	      
	      
	      if (XCheckTypedEvent( dpy, MotionNotify, &ev ))
		{
		  XPutBackEvent( dpy, &ev );
		  break;
		}
	      
	      XDrawLines( dpy, win, line_gc, points, 5, CoordModeOrigin );
	      
	      tmpstartx = startx;
	      tmpstarty = starty;	
	      tmpendx   = e.xmotion.x;
	      tmpendy   = e.xmotion.y;
	      
	      justify( &tmpstartx, &tmpstarty, &tmpendx, &tmpendy );
	      
	      points[ 0 ].x = tmpstartx; points[ 0 ].y = tmpstarty;
	      points[ 1 ].x = tmpendx;   points[ 1 ].y = tmpstarty;
	      points[ 2 ].x = tmpendx;   points[ 2 ].y = tmpendy;
	      points[ 3 ].x = tmpstartx; points[ 3 ].y = tmpendy;
	      points[ 4 ].x = tmpstartx; points[ 4 ].y = tmpstarty;
	      
	      XDrawLines( dpy, win, line_gc, points, 5, CoordModeOrigin );
	      
	      XSync( dpy, 0 );
	    }
	  
	  break;
	  
	default:
	  break;
	  
	} /* switch */
    }

  /* restore cursor */
  
  XDefineCursor( dpy, win, XCreateFontCursor( dpy, XC_watch ) );

  /* return result */
  
  return ret_val;
  
} /* x_get_new_coords */
  
#define xxTEST
#if defined TEST

int
main( void )
{
  unsigned int		w = 640;
  unsigned int		h = 480;
  unsigned char		cols[ 8 ];
  unsigned char *	scanline = NULL;
  
 
  if (!x_initialise( &w, &h, cols ))
    {
      debug( "init failed" );

      return 0;
    }

  for (;;)
    {
      unsigned int	x;
      unsigned int	y;
      unsigned int	tlx;
      unsigned int	tly;
      unsigned int	brx;
      unsigned int	bry;

      
      if (scanline)
	scanline = (unsigned char *)realloc( scanline, w );
      else
	scanline = (unsigned char *)malloc( w );
      
      for (y = 0; y < h; y++)
	{
	  for (x = 0; x < w; x++)
	    {
	      scanline[ x ] = y % 8;
	    }

	  x_draw_scanline( y, scanline );
	}

      switch (x_get_new_coords( &tlx, &tly, &brx, &bry ))
	{
	case -1:
	  x_finish();
	  return 1;

	case 0:
	  break;
	  
	case 1:
	  break;
	}
    }
}

#endif /* TEST */

/* end of xdemo.c */

/* @@ emacs customization */

/* Local Variables: */
/* mode: c */
/* outline-regexp: "^[a-zA-Z_]*(" */
/* eval: (outline-minor-mode 1) */
/* End: */
