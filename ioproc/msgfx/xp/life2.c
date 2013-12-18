#include <string.h>
#include <stdlib.h>
#include <sem.h>
#include <syslib.h>
#include <nonansi.h>
#include <queue.h>
#include <asm.h>

#define TEST		0	/* debug testing			*/
#define GEM		0	/* GEM graphics				*/
#define X		0	/* XWindows graphics			*/
#define MSG		1	/* MicroSoft graphics			*/
#define TIME		0	/* for timing main loop			*/
#define ACCELERATE	0	/* run main loop on fast stack		*/
#define COPYLIFE 	0	/* copy main loop out to fast ram 	*/
#define XOR		0	/* use XOR graphics mode		*/
#define INITONLY	0	/* initialise board only		*/
#define MACROS		1	/* use set/clear pixel macros		*/
#define MINFREE		1	/* keep free pxylists to minimum	*/

#if X
#define xymax 1024
#elif GEM
#define xymax 256
#else /* MSG */
#define xymax 256
#endif

typedef struct pxylist {
	Node		node;
	int		color;
	int		pos;
#if X
	short		array[xymax];
#else
	int		array[xymax];
#endif
} pxylist;

#define maxpxylists (100000/sizeof(pxylist))

extern pxylist *setarray;
extern pxylist *clrarray;

typedef unsigned char Cell;

extern bool finished;
extern int *random(int size);
extern void flush_set(void);
extern void flush_clear(void);
#if MACROS
#define set_pixel(x,y)				\
{						\
	pxylist *p = setarray;			\
	int pos = p->pos;			\
	p->array[pos] = x;			\
	p->array[pos+1] = y;			\
	p->pos= pos+2;				\
	if( p->pos >= xymax ) flush_set();	\
}
#define clr_pixel(x,y)				\
{						\
	pxylist *p = clrarray;			\
	p->array[p->pos] = x;			\
	p->array[p->pos+1] = y;			\
	p->pos+=2;				\
	if( p->pos >= xymax ) flush_clear();	\
}
#else

extern void set_pixel(int x, int y);
extern void clr_pixel(int x, int y);

#endif

void (change_cell)(Cell *new, int x, int y, int width, int height, int change);

Semaphore FreeLock;
List freeq;

Semaphore PendCount;
Semaphore PendLock;
List pending;

int npxylists = 2;

Semaphore ScreenLock;

bool finished = FALSE;

int cycletime = 0;

#if GEM
#include <vdibind.h>

int handle;

int set_interior;
int set_color;
int set_mode;
int set_style;
int set_perimeter;
int style_index;

int work_out[57];
int work_in[11];
#endif

#if X
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

Display *display;
int screen;
Window *win;
GC *gc;
XEvent xevent;
#endif

#if MSG
#include "graph.h"
#endif

void open_work(int x, int y)
{
#if TEST
	IOdebug("open_work %d %d",x,y);
#endif
#if MSG
	_setvideomode(_MRES4COLOR);
#endif
#if GEM
	int i;
	for( i= 0; i < 10 ; i++ ) work_in[i] = 0;
	work_in[0] = 1;
	work_in[6] = 1;
	work_in[10] = 2;
	v_opnwk(work_in, &handle, work_out);	
	if (handle == 0) exit(1);
	v_clrwk(handle);	
#endif
#if X
	if((display = XOpenDisplay( NULL )) == NULL )
	{
		IOdebug("failed to open display");
		exit(1);
	}
	
	screen = DefaultScreen(display);

	win = XCreateSimpleWindow(display, RootWindow(display,screen),
		0,0,x,y,2,BlackPixel(display,screen),WhitePixel(display,screen));

	gc = XCreateGC(display, win, NULL, 0 );

	XStoreName( display, win, "Life");

	XSelectInput( display, win, VisibilityChangeMask | ButtonPressMask | KeyPressMask );

	XMapWindow(display, win);

	XSync( display, 0);

	while( XNextEvent( display, &xevent  ),
		xevent.type != VisibilityNotify ||
		xevent.xvisibility.state != VisibilityUnobscured );
#endif
	InitSemaphore(&ScreenLock,1);
	x = y;
}

void close_work()
{
	Wait(&ScreenLock);
#if TEST
	IOdebug("closewk");
#endif
#if GEM
	v_clswk(handle);
#endif
#if X
	XSync(display,TRUE);
	XFreeGC(display, gc);
	XCloseDisplay(display);
#endif
#if MSG
	FlushMSG(WAIT);
#endif	/* nothing to do */
}

void set_attr()
{
	Wait(&ScreenLock);
#if GEM
	set_color = vsf_color(handle,1);	/* fill color black */
	set_mode = vswr_mode(handle,1);		/* char mode normal */
	set_perimeter = vsf_perimeter(handle,1);/* visible borders  */
	vsm_type(handle, 1);
	vsm_height(handle,1);
	vsm_color(handle,1);
#if XOR
	vswr_mode(handle, 3);
#endif
#endif

#if TEST
	IOdebug("Set_attr");
#endif

#if X
#if XOR
	XSetFunction(display,gc,GXinvert);
#endif
	XSetForeground(display,gc,BlackPixel(display,screen));
#endif

#if MSG
#endif	/* nothing to do */
	Signal(&ScreenLock);

}

#if X
void doevents()
{
	while( XPending(display) )
	{
		XNextEvent(display,&xevent);
		switch( xevent.type )
		{
		case ButtonPress:
			IOdebug("pyxlists = %d/%d cycle time = %d",
				npxylists,maxpxylists,cycletime);
			break;
		case KeyPress:
			finished = TRUE;
			break;
		default:
			break;
		}
	}
}
#endif
pxylist *setarray = NULL;
pxylist *clrarray = NULL;

void pmarker()
{
	pxylist *array;

	for(;;)
	{
		Wait(&PendCount);

		Wait(&PendLock);
		array = (pxylist *)RemHead(&pending);
		Signal(&PendLock);

		Wait(&ScreenLock);
#if GEM
#if XOR
#else
		vsm_color(handle,array->color);
#endif
		v_pmarker(handle, array->pos/2, array->array);
#endif

#if MSG
		_setcolor(array->color);
		_moveto(array->array[0],array->array[1]);
		_lineto(array->array[pos-2],array->array[pos-1]);

#endif

#if X
#if XOR
#else
		XSetForeground(display,gc,array->color?BlackPixel(display,screen):WhitePixel(display,screen));
#endif
		XDrawPoints(display,win,gc,array->array,array->pos/2,CoordModeOrigin);
		
		doevents();
#endif
#if TEST
		IOdebug("pmarker %x %d %d",array,array->pos,array->color);
#endif
		Signal(&ScreenLock);

		Wait(&FreeLock);
#if MINFREE
		if( freeq.Head->Next != NULL )
		{ Free(array); npxylists--; }
		else
#endif
			AddTail(&freeq,&array->node);
		Signal(&FreeLock);
	}
}

#if TEST
void printboard(Cell *b, int w, int h)
{
	int x,y;
return;
	for( y = 0; y < h ; y++ )
	{
		for( x = 0; x < w; x++ )
		{
			int cell = *((b+y*w)+x);
			printf("%2d ",cell);
		}
		printf("\n");
	}
}

#endif
	
void flush_set()
{
#if TIME
	return;
#endif
#if TEST
IOdebug("flush_set %x %d %d",setarray,setarray->array[0],setarray->array[1]);
setarray->pos=0; 
return;
#endif
	Wait(&PendLock);
	AddTail(&pending,&setarray->node);
	Signal(&PendLock);
	Signal(&PendCount);

	setarray = NULL;
		
	while( setarray == NULL )
	{
		Wait(&FreeLock);
		setarray = (pxylist *)RemHead(&freeq);
		Signal(&FreeLock);
		if( setarray == NULL )
		{
			if( npxylists < maxpxylists )
			{
				setarray = New(pxylist);
				if( setarray != NULL ) { npxylists++; break; }
			}
		}
		else
			break;
	}
	setarray->color = 1;
	setarray->pos = 0;
}

void flush_clear()
{
#if TIME
	return;
#endif
#if TEST
IOdebug("flush_clear"); 
return;
#endif
	Wait(&PendLock);
	AddTail(&pending,&clrarray->node);
	Signal(&PendLock);
	Signal(&PendCount);
		
	clrarray = NULL;
		
	while( clrarray == NULL )
	{
		Wait(&FreeLock);
		clrarray = (pxylist *)RemHead(&freeq);
		Signal(&FreeLock);
		if( clrarray != NULL ) break;
		if( npxylists >= maxpxylists ) continue;
		clrarray = New(pxylist);
		if( clrarray != NULL ) { npxylists++; break; }
	}
	clrarray->color = 0;
	clrarray->pos = 0;
}

void (set_pixel)(int x, int y)
{
	setarray->array[setarray->pos++] = x;
	setarray->array[setarray->pos++] = y;

#if TIME
	setarray->pos = 0;
#else
	if( setarray->pos >= xymax ) flush_set();
#endif
}

void (clr_pixel)(int x, int y)
{
	clrarray->array[clrarray->pos++] = x;
	clrarray->array[clrarray->pos++] = y;
#if TIME
	clrarray->pos = 0;
#else
	if( clrarray->pos >= xymax ) flush_clear();
#endif
}

#define NEW(v,w,xx,yy) (*((v+(yy)*w)+(xx)))

#if 0
#define change_cell(new,x,y,width,height,change)			\
{									\
	int px = x-1;							\
	int py = y-1;							\
	int nx = x+1;							\
	int ny = y+1;							\
	int edge = 0;							\
	if( px==-1) px = width-1,edge=1;				\
	if( py==-1) py = height-1,edge=1;				\
	if( nx==width) nx = 0,edge=1;					\
	if( ny==height) ny = 0,edge=1;					\
	if( edge )							\
	{								\
		NEW(new, width, x, y) += change*9;			\
		NEW(new, width,px,py) += change;			\
		NEW(new, width, x,py) += change;			\
		NEW(new, width,nx,py) += change;			\
		NEW(new, width,px, y) += change;			\
		NEW(new, width,nx, y) += change;			\
		NEW(new, width,px,ny) += change;			\
		NEW(new, width, x,ny) += change;			\
		NEW(new, width,nx,ny) += change;			\
	}								\
	else {								\
		Cell *cell = (new+y*width)+x;				\
		*cell = *cell + change*9;				\
		*(cell-width-1) += change;				\
		*(cell-width  ) += change;				\
		*(cell-width+1) += change;				\
		*(cell      -1) += change;				\
		*(cell      +1) += change;				\
		*(cell+width-1) += change;				\
		*(cell+width  ) += change;				\
		*(cell+width+1) += change;				\
	}								\
	set_pixel(x,y);							\
}
#else
void (change_cell)(Cell *new, int x, int y, int width, int height, int change)
{
	int px = x-1;
	int py = y-1;
	int nx = x+1;
	int ny = y+1;
	
	if( px==-1) px = width-1;
	if( py==-1) py = height-1;
	if( nx==width) nx = 0;
	if( ny==height) ny = 0;
	
/*IOdebug(" change_cell:  p(%d,%d) (%d,%d) n(%d,%d)",px,py,x,y,nx,ny);*/

	NEW(new, width, x, y) += change*9;
	NEW(new, width,px,py) += change;
	NEW(new, width, x,py) += change;
	NEW(new, width,nx,py) += change;
	NEW(new, width,px, y) += change;
	NEW(new, width,nx, y) += change;
	NEW(new, width,px,ny) += change;
	NEW(new, width, x,ny) += change;
	NEW(new, width,nx,ny) += change;

#if XOR
	set_pixel(x,y);
#else
	if( change > 0 ) { set_pixel(x,y); }
	else { clr_pixel(x,y); }
#endif
}
#endif


Cell *boards[2];


Cell *init_board(int x, int y)
{
	int i;
	int bsize = x*y;
	Cell *board = malloc(sizeof(Cell)*bsize);
	for( i = 0; i < bsize; i++ ) board[i] = 0;
	return board;
}

void init_life(int w, int h)
{
	Cell *b0 = boards[0] = init_board(w,h);
	boards[1] = init_board(w,h);
	
	setarray = New(pxylist);
	setarray->color = 1;
	setarray->pos = 0;

	clrarray = New(pxylist);
	clrarray->color = 0;
	clrarray->pos = 0;

	InitSemaphore(&FreeLock,1);
	InitSemaphore(&PendCount,0);
	InitSemaphore(&PendLock,1);
	InitList(&freeq);
	InitList(&pending);
	
	Fork(10000,pmarker,0);
#if 1
	{
		int x,y;
		for( y = 0; y < h; y++ )
		{
			for( x = 0; x < w ; x++ )
				if( (rand()&7) == 0 )
				{
					change_cell(b0,x,y,w,h,1);
				}
		}
	}
#else
	{
		int cx = x/2;
		int cy = y/2;
		
		change_cell(b0,cx,cy,x,y,1);
		change_cell(b0,cx-1,cy,x,y,1);
		change_cell(b0,cx+1,cy,x,y,1);
	}
#endif

#if TEST
	printboard(b0,w,h);
#endif

}


void lifecycle(Cell *old, Cell *new, int width, int height, int *changes)
{
	int wplus1 = width+1;
	int wless1 = width-1;
	int hless1 = height-1;
	Cell *end = old+width*height;
	Cell *p;
	
	memcpy( new, old, sizeof(Cell)*width*height);

	for(p = old ; p != end ; p++ )
	{
		int change = changes[*p];
		if( change )
		{
			int pos = p-old;
			int y = pos / width;
			int x = pos % width;

			Cell *ncell = new + pos;

			if( x == 0 || y == 0 || x == wless1 || y == hless1 )
			{
#if 0
				int px = x-1;
				int py = y-1;
				int nx = x+1;
				int ny = y+1;
				if( px==-1) px = width-1;
				if( py==-1) py = height-1;
				if( nx==width) nx = 0;
				if( ny==height) ny = 0;
				*ncell = *ncell + change*9;
				NEW(new, width,px,py) += change;
				NEW(new, width, x,py) += change;
				NEW(new, width,nx,py) += change;
				NEW(new, width,px, y) += change;
				NEW(new, width,nx, y) += change;
				NEW(new, width,px,ny) += change;
				NEW(new, width, x,ny) += change;
				NEW(new, width,nx,ny) += change;
#else
				change_cell(new,x,y,width,height,change);
#endif
			}
			else {
#if 0
				*(ncell-wless1) = *(ncell-wless1) + change;
				*(ncell-width) = *(ncell-width) + change;
				*(ncell-wplus1) = *(ncell-wplus1) + change;
				*(ncell+1) = *(ncell+1) + change;
				*(ncell-1) = *(ncell-1) + change;
				*(ncell+wless1) = *(ncell+wless1) + change;
				*(ncell+width) = *(ncell+width) + change;
				*(ncell+wplus1) = *(ncell+wplus1) + change;
#endif
#if 0
				Cell *c;
				c = ncell-wless1; *c = *c + change;
				c = ncell-width;  *c = *c + change;
				c = ncell-wplus1; *c = *c + change;
				c = ncell-1;      *c = *c + change;
				c = ncell+1;      *c = *c + change;
				c = ncell+wless1; *c = *c + change;
				c = ncell+width;  *c = *c + change;
				c = ncell+wplus1; *c = *c + change;
#endif
#if 1
				Cell *c;
				c = ncell-wless1; sb_(c , sum_(*c , change));
				c = ncell-width;  sb_(c , sum_(*c , change));
				c = ncell-wplus1; sb_(c , sum_(*c , change));
				c = ncell-1;      sb_(c , sum_(*c , change));
				c = ncell+1;      sb_(c , sum_(*c , change));
				c = ncell+wless1; sb_(c , sum_(*c , change));
				c = ncell+width;  sb_(c , sum_(*c , change));
				c = ncell+wplus1; sb_(c , sum_(*c , change));
#endif
				*ncell = *ncell + change*9;
#if XOR
				set_pixel(x,y);
#else
				if( change > 0 ) { set_pixel(x,y); }
				else { clr_pixel(x,y); }
#endif
			}

		}
	}
}

static statetab[18] = {  0, 0, 0,+1, 0, 0, 0, 0, 0,	/* dead cells	*/
			-1,-1, 0, 0,-1,-1,-1,-1,-1 };	/* live cells	*/

void life(Cell **boards, int width, int height, int cycles)
{
	int cycle = 0;
	for( cycle = 0; !finished && cycle < cycles; cycle++ )
	{
		int start = clock();
		lifecycle(boards[cycle&1],boards[(cycle+1)&1],
				width,height,statetab);
		cycletime = clock()-start;
				
#if TEST
		printboard(boards[(cycle+1)&1],width,height);
#endif
	}
}

int *random(int size)
{
	int *v = malloc((size+1)*sizeof(word));
	int i;

	for( i = 0; i <= size; i++ ) v[i] = -1;
	for( i = 1; i <= size; i++ )
	{
		int nx;
		do {
			nx = rand() % size;
			if( nx < 0 ) nx = -nx;
			nx++;
		} while( v[nx] != -1 );
		v[nx] = i;
	}
	return v;
}

int main(int argc, char **argv)
{
#if TEST
	int x = 10;
	int y = 10;
	int cycles = 1;
#endif
#if X
	int x = 200;
	int y = 200;
	int cycles = MaxInt;
#endif
#if GEM
	int x = 320;
	int y = 200;
	int cycles = 50;
#endif
#if MSG
	int x = 320;
	int y = 200;
	int cycles = 100;
#endif
	int start, time;
#if ACCELERATE	
	Carrier *c = AllocFast(1000,&MyTask->MemPool);

	if( c == NULL ) return 1;
#endif
	srand(GetDate());
	
	
	if( argc > 1 ) x = atoi(argv[1]);
	if( argc > 2 ) y = atoi(argv[2]);
	if( argc > 3 ) cycles = atoi(argv[3]);

	open_work(x,y);

	set_attr();

	init_life(x,y);

	start = clock();

#if INITONLY	
	Delay(2000000);
#else
#if COPYLIFE
	AccelerateCode(life);
#endif
#if ACCELERATE
	Accelerate(c,life,16,boards,x,y,cycles);
#else
	life(boards,x,y,cycles);
#endif
#endif
	flush_set();
	flush_clear();
	
	while( TestSemaphore(&PendCount) > 0 );
	
	time = clock()-start;
	
	close_work();	

#if 1
	IOdebug("time = %d csec, %d csec/cycle",time,time/cycles);
	IOdebug("pxylists = %d",npxylists);	
#endif	
	return 0;	
}
