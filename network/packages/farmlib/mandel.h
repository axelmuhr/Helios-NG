/**
*** mandel.h
***		Header file for the Mandelbrot calculation
***
*** This packet is sent by the producer on start-up, to inform all the
*** workers about the values used for particular colours. This Mandelbrot
*** example uses only eight colours, allowing it to be run on most
*** X terminals. Each colour value will be an index into the X terminal's
*** colour lookup table. This packet is not acknowledged.
**/
typedef struct	Mandel_ColourInfo {
	int	Type;			/* Request_ColourInfo	*/
	int	Red;
	int	Orange;
	int	Yellow;
	int	Green;
	int	Blue;
	int	Indigo;
	int	Violet;
	int	Black;
} Mandel_ColourInfo;

/**
*** This packet is sent by the producer when it starts to draw a new screen.
*** It contains sufficient information to allow each worker to calculate the
*** scanlines it is assigned.
***   Y_Size and X_Size define the number of pixels, e.g. 400 by 640
***   BottomLeft is the complex number representing the bottom-left corner of
***   the screen. Similarly TopRight is another complex number. For the main
***   Mandelbrot picture, BottomLeft is -2.25 + -1.8i, and TopRight is
***   0.75 + 1.5i.
*** This packet is acknowledged.
**/
typedef struct Mandel_ScreenInfo {
	int	Type;			/* Request_ScreenInfo	*/
	int	Y_Size;
	int	X_Size;
	double	BottomLeft_Real;
	double	BottomLeft_Imag;
	double	TopRight_Real;
	double	TopRight_Imag;
} Mandel_ScreenInfo;

/**
*** This packet defines some piece of work to be done by a worker program,
*** in particular one scan-line of the current Mandelbrot picture. 
*** If the window on the X terminal is 640 pixels wide and 400 pixels high
*** then Y_Coord will be between 0 and 399 inclusive, and the worker is supposed
*** to produce a reply packet containing 640 bytes, one for each pixel on
*** the specified line. Note that for this application coordinate (0,0) is
*** the bottom left corner rather than the top left.
**/
typedef struct Mandel_Job {
	int	Type;			/* Request_Job */
	int	Y_Coord;
} Mandel_Job;

/**
*** For the Mandelbrot example there are three different types of packets
*** sent by the producer to the various worker. Each of the packets contains
*** a type field which will be set to one of the values below. Hence the
*** workers can determine what kind of packet they are currently processing.
**/
#define Request_ColourInfo	0
#define	Request_ScreenInfo	1
#define Request_Job		2

/**
*** Only one of the request packets involves a reply from the workers, namely
*** the request to generate a scanline of the Mandelbrot pixel. The required
*** information is held in a Mandel_Reply structure. The structure contains
*** the Y coordinate value specified in the incoming job, so that it is
*** possible to identify where the scanline information should be drawn.
**/
typedef struct	Mandel_Reply {
	int		Y_Coord;
	unsigned char	Scanline[1];
} Mandel_Reply;

#define sizeof_Mandel_Reply	sizeof(int)

/**
*** Various function declarations.
**/
extern	void	Mandel_Worker(void);		/* mandcalc.c	*/

	/* X interface provided by mandelX.c */
extern	int	x_initialise(unsigned int *, unsigned int *, unsigned char colours[8]);
extern	void	x_finish(void);
extern	void	x_draw_scanline(unsigned int y_coord, unsigned char *colour_bytes);
extern	int	x_get_new_coords(unsigned int *, unsigned int *, unsigned int *,
				unsigned int *);

