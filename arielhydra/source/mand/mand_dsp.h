/****************************************************************/
/*                          Ariel Corp.                         */
/*                        433 River Road                        */
/*                Highland Park, NJ 08904, U.S.A.               */
/*                     Tel:  (908) 249-2900                     */
/*                     Fax:  (908) 249-2123                     */
/*                     BBS:  (908) 249-2124                     */
/*                  E-Mail:  ariel@ariel.com                    */
/*                                                              */
/*                 Copyright (C) 1991 Ariel Corp.               */
/****************************************************************/


#define	PROGNAME	"manddsp.x40"
#define	DEFAULT_DEVICE	"/dev/vc40a1"
#define	BOARDNAME	"Hydra"
#define	MAXLINE		1200
#define	DSP_MAXIT	125


typedef long PIXEL_TYPE;

extern float	xmin, xmax, ymin, ymax;
extern int	nx, ny, maxit;

int init_dsp();
void close_dsp();
void stop_timing();
void draw_mandelbrot_dsp();
void draw_mandelbrot_line();
void register_signal();
