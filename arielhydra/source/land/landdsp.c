#include <stdlib.h>

typedef unsigned long u_long;

#define  XA 0
#define  XB 500
#define  YA 0
#define  ZA 0
#define  YADD 38

#define	RANDOM()    (rand() >> 16)

unsigned long *VIC_virsr = (unsigned long *) 0xbfff0020;
#define	HOST_INTERRUPT()			\
	*(VIC_virsr + intpri) = intvec;		\
	*(VIC_virsr) = ((1 << intpri) + 1);	\

#define NLINES  4000

/*
 * elements in structure are packed to reduce VMEbus traffic
 */
struct line3d {
    u_long  flag_and_color;
    u_long  start_point;    /* x0 << 16 | y0 */
    u_long  end_point;      /* x1 << 16 | y1 */
} testline;

struct line3d lines[NLINES];

void addline(int x0, int y0, int z0, int x1, int y1, int z1);
void draw_lines();
void frac(int depth, int x0, int y0, int x2, int y2, 
	int z0, int z1, int z2, int z3);

int nl = 0;

/*
 * host must initialize these!
 */
int ideep;
float steep;                /* */
int sealevel;               /* */
int ybottom;                /* */
int WATERCOLOR, LANDCOLOR;  /* */
int seed = 0;               /* */
volatile int start_flag = 0;
volatile int ready_flag = 0;
volatile int done_flag = 0;
int intpri, intvec;         /* */
int ix0, iy0, ix2, iy2, iz0, iz1, iz2, iz3;


int main()
{
    GIEOn();
    testline.flag_and_color = 0x12345678;
    testline.start_point = 0x11112222;
    testline.end_point = 0x33334444;
    
    while (seed == 0) ;
    
    srand(seed);
    
    while (1) {
        while (!start_flag) ;
        
/*        frac(ideep, XA,YA,XB,ybottom,ZA,ZA,ZA,ZA); */
        frac(ideep, ix0, iy0, ix2, iy2, iz0, iz1, iz2, iz3);
	if (nl > 0) draw_lines();
	start_flag = 0;
	done_flag = 1;
	ready_flag = 1;
	HOST_INTERRUPT();
    }
}


void frac(int depth, int x0, int y0, int x2, int y2, 
	int z0, int z1, int z2, int z3) 
{
    int newz; /* new center point */
    int xmid,ymid,z01,z12,z23,z30;

    /*
     * 50% chance
     */
    if (RANDOM() < 16384) {
        newz = (z0+z1+z2+z3) / 4 + (int)((RANDOM() / 32768.0) * ((y2-y0)* steep));
    }
    else {
        newz = (z0+z1+z2+z3) / 4 - (int)((RANDOM() / 32768.0) * ((y2-y0)* steep));
    }
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
    }
    else {
        if (newz<=sealevel ) { /*above sea level*/
            /*L to R*/
            addline(xmid,ymid,newz, x2,ymid,z12);
            addline(xmid,ymid,newz, x0,ymid,z30);
        }
        else {
            /*below "sea level"*/
            addline(xmid,ymid,sealevel, 0,0,-9999);
        }
    }
}

void addline(int x0, int y0, int z0, int x1, int y1, int z1)
{
    register int tt;
    register int xs, ys, xe, ye;
    register struct line3d *tline = &lines[nl];
    
    if (z1 == -9999) {
        tline->flag_and_color = 0x10000 | WATERCOLOR;
        xs = (y0 >> 1) + x0;
        ys = YADD + y0 + z0;
        tline->start_point = (xs << 16) | ys;
    }
    else {
        tt = (abs(z1) >> 2) + 86;
	if(tt > 127) {
	    tt = 127;
	}
        tline->flag_and_color = tt;
        xs = (y0 >> 1) + x0;
        ys = YADD + y0 + z0;
        xe = (y1 >> 1) + x1;
        ye = (YADD + y1 + z1);
        tline->start_point = (xs << 16) | ys;
        tline->end_point = (xe << 16) | ye;
    }
    nl++;
    if (nl == NLINES) {
        draw_lines();
    }
}


void draw_lines()
{
    struct line3d *tline;
    static int i;
    
    ready_flag = 1;
    
    HOST_INTERRUPT();
    
    while (ready_flag) ;
    
    nl = 0;
}

            

