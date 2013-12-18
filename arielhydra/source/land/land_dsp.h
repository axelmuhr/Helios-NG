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


#define	PROGNAME	"landdsp.x40"
#define	DEFAULT_DEVICE	"/dev/vc40a1"
#define	BOARDNAME	"Hydra"
#define	DEPTH		9
#define NLINES  4000

int init_dsp();
void close_dsp();
void stop_timing();
void draw_landscape_dsp();
void draw_lines();
void register_signal();
void draw_lines();
float frandom();

extern int ybottom;
extern int watercolor;
extern int landcolor;
extern int sealevel;
extern float steep;
extern struct line3d lines[];
extern int done_drawing;

struct line3d {
	short	pflag;
	short	color;
	short	x0, y0;
	short	x1, y1;
};
