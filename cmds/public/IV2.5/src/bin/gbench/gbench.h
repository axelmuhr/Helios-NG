#include <ctype.h>
#include <nlist.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#ifdef SUN3
#define FIXLOAD
#endif
#ifdef SUN4
#define FIXLOAD
#endif

#ifdef X10
#   include <X10/Xlib.h>
#   define _BlackPixel		BlackPixel
#   define ButtonPress		ButtonPressed
#   define ButtonPressMask	ButtonPressed
#   define ButtonRelease	ButtonReleased
#   define ButtonReleaseMask	ButtonReleased
#   define DARG
#   define DARGC
#   define DefaultFonts		"4x6,6x10,6x13,9x15,ice34"
#   define _DisplayHeight	DisplayHeight()
#   define _DisplayWidth	DisplayWidth()
#   define Drawable		Window
#   define ExposeRegionMask	ExposeRegion
#   define ExposureMask		ExposeCopy
#   define GC			int
#   define MotionNotify		MouseMoved
#   define PointerMotionMask	MouseMoved
#   define _RootWindow		RootWindow
#   define _WhitePixel		WhitePixel
#   define XClearWindow		XClear
#   define XCreateSimpleWindow	XCreateWindow
#   define XFontStruct		FontInfo
#   define XFreeFont		XCloseFont
#   define XGraphicsExposeEvent	XExposeEvent
#   define XPoint		Vertex
#   define XTextItem		char
#   define XMotionEvent		XMouseMovedEvent
#else
#   include <X11/Xlib.h>
#   include <X11/X.h>
#   include <X11/Xutil.h>
#   include <X11/cursorfont.h>
#   define _BlackPixel		BlackPixel(sp->d,DefaultScreen(sp->d))
#   define BlackPixmap		_BlackPixel
#   define DARG			sp->d
#   define DARGC		DARG,
#   define DefaultFonts		"vg-??"
#   define _DisplayHeight	DisplayHeight(sp->d,DefaultScreen(sp->d))
#   define _DisplayWidth	DisplayWidth(sp->d,DefaultScreen(sp->d))
#   define ExposeRegionMask	0
#   define Pattern		int
#   define _RootWindow		DefaultRootWindow(sp->d)
#   define _WhitePixel		WhitePixel(sp->d,DefaultScreen(sp->d))
#   define WhitePixmap		_WhitePixel
#endif

#define Border		10
#define DefaultAngle	350
#define DefaultAspect	1.
#define DefaultLwidth   1
#define DefaultMaxshift	32
#define DefaultNchar	10
#define DefaultNvert	4
#define DefaultNwin	10
#define DefaultOffset	.5
#define DefaultPtsize	12
#define DefaultSize	100
#define DefaultTimegoal	1
#define DefaultWinsize  _DisplayWidth/2
#define MaxIndepth	10
#define MaxNum		1000000
#define MaxPositional	5
#define MaxStr		120
#define MaxVert		31
#define MaxFonts	16
#define MinAspect	.1
#define nil		0
#define NumOps		9
#define OverlapSize	2
#define Pi		3.141592654
#define PolyAngle	Pi/18
#define Version         1.1

#define Sign(x) ((x)>=0?1:-1)

typedef enum {false,true} bool;

typedef double Load;

typedef void (*Op)();

typedef enum {
    NilIndex = -1,
    ArcIndex,
    BlitIndex,
    MapIndex,
    NopIndex,
    PointIndex,
    PolyIndex,
    RectIndex,
    TextIndex,
    VecIndex
} OpIndex;

typedef enum {StartOp,DoOp,EraseOp,FinishOp} OpType;

typedef struct {
    bool    altcolor;
    bool    altfont;
    bool    altgc;
    bool    altwin;
    bool    drag;
    bool    fill;
    bool    invert;
    bool    offdest;
    bool    offsrc;
    bool    overlap;
    bool    poll;
    bool    polyself;
    bool    polywind;
    bool    profile;
    bool    setdefaults;
    bool    silent;
    bool    stipple;
    bool    tile;
    bool    unbatched;
} OptFlags;

typedef struct {
    int	    angle;
    float   aspect;
    int	    count;
    char    fonts[MaxStr];
    int	    ptsize;
    int	    lwidth;
    int	    maxshift;
    float   offset;
    int	    nchar;
    int	    nwin;
    int	    nvert;
    char    opts[MaxStr];
    char    outfile[MaxStr];
    int	    size;
    char    tag[MaxStr];
    int	    timegoal;
    int	    winsize;
} Params;

typedef unsigned long Pixel;

typedef enum {
    Succeeded,
    CmdNotSupported,
    OptNotSupported,
    NoFonts,
    NoOffscreenMem
} Result;

typedef double Rtime;

typedef struct {
    char*   cur;
    char*   end;
} TokenState;

typedef struct {
    int		bbx;
    int		bby;
    Pixel	colorbg;
    Pixel	colorfg;
    Drawable	dest;
    int		dx;
    int		dy;
    Font	fontid;
    int		func;
    GC		gc;
    OpIndex	index;
    int		iterations;
    OptFlags	o;
    Params	p;
    Result	result;
    int		shift;
    int		size[3];
    Drawable	src;
    double	time;
    Window	w;
    int		x;
    int		y;
} Cmd;

typedef struct {
    bool	    altwin;
    int		    argc;
    int		    argindex;
    char**	    argv;
    Cursor	    cursor;
    Window	    cw1;
    Window	    cw2;
    Display*	    d;
    Params	    defaults;
    Drawable	    dest1;
    Drawable	    dest2;
    char	    disphost[MaxStr];
    char	    fonts[MaxStr];
    XFontStruct*    fontinfo[MaxFonts];
    GC		    gc1;
    GC		    gc2;
    char*	    gxname;
    char	    host[MaxStr];
    int		    indepth;
    FILE*	    infds[MaxIndepth+1];
    char	    instr[MaxStr+1];
    Drawable	    offpix;
    FILE*	    outfd;
    char	    outfile[MaxStr+1];
    char	    outstr[MaxStr+1];
    Pattern	    pattern;
    OpIndex	    previndex;
    Params	    prevparams;
    int		    scriptindex;
    Drawable	    src1;
    Drawable	    src2;
    bool	    strinput;
    bool	    supcmds[NumOps];
    OptFlags	    supopts;
    XTextItem	    texts[MaxStr];
    Pixmap	    tile;
    TokenState	    ts;
    XPoint	    v[MaxVert+1];
    Window	    w1;
    Window	    w2;
    int		    winsize;
} State;

bool	GetCmd();
Load	GetLoad();
OpIndex	GetOpIndex();
Rtime	GetTime();
FILE*	Out();
void	Start(),Erase(),Finish();
void	StartArc(),DoArc();
void	StartBlit(),DoBlit();
void	StartMap(),DoMap(),FinishMap();
void	DoNop();
void	StartPoint(),DoPoint();
void	StartPoly(),DoPoly();
void	StartRect(),DoRect();
void	StartText(),DoText();
void	StartVec(),DoVec();
char*	StrToken();

extern char*	OpNames[NumOps][1+MaxPositional];
extern Op	Ops[NumOps][4];
extern char*	ResultMsgs[];
extern char*	Script[];
