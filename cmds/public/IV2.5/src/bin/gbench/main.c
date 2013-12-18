#include <gbench.h>

#ifdef X10
#   include <X10/bitmaps/gray1.bitmap>
#   include <X10/cursors/target.cursor>
#   include <X10/cursors/target_mask.cursor>
#else
#   include <X11/bitmaps/dot>
#   include <X11/bitmaps/gray1>
#endif

char* OpNames[NumOps][1+MaxPositional] = {
    {"arc",	"opts",	"size",	"aspect",   "angle",	"lwidth"},
    {"blit",    "opts",	"size", "offset",   nil,	nil},
    {"map",	"opts",	"size", "nwin",	    nil,	nil},
    {"nop",     "opts",	nil,	nil,	    nil,	nil},
    {"point",	"opts",	nil,	nil,	    nil,	nil},
    {"poly",	"opts",	"size",	"nvert",    "lwidth",   nil},
    {"rect",	"opts",	"size",	"lwidth",    nil,	nil},
    {"text",	"opts",	"nchar","ptsize",   nil,	nil},
    {"vec",	"opts",	"size",	"angle",    "lwidth",   nil}
};

Op Ops[NumOps][4] = {
    {StartArc,	DoArc,	Erase,  Finish},
    {StartBlit,	DoBlit,	Erase,  Finish},
    {StartMap,	DoMap,	Erase,  FinishMap},
    {Start,	DoNop,	Erase,  Finish},
    {StartPoint,DoPoint,Erase,	Finish},
    {StartPoly,	DoPoly,	Erase,  Finish},
    {StartRect, DoRect,	Erase,  Finish},
    {StartText, DoText,	Erase,  Finish},
    {StartVec,	DoVec,	Erase,  Finish}
};

char* ResultMsgs[] = {
    "Succeeded",
    "CmdNotSupported",
    "OptNotSupported",
    "NoFonts",
    "NoOffscreenMem"
};

char* Script[] = {
    "@# Starting script",
    "c",
    "a",
    "a f 10",
    "a f 100",
    "b",
    "m",
    "n",
    "point",
    "poly",
    "poly f 10",
    "poly f 100",
    "r",
    "r f 10",
    "r f 100",
    "t n 1",
    "t n 10",
    "v n 10",
    "v n 100",
    "@# End of script",
    nil
};

static char* pname = "gbench";

void SetFonts(sp,pp)
    State*  sp;
    Params* pp;
{
    int		findex = 0;
    int		foundcount;
    char**	foundlist;
    int		i;
    char*	name;
    char*	np = pp->fonts;
    char	nlist[MaxStr];
    TokenState	ts;

    strcpy(nlist,pp->fonts);
    for (i=0;(i<MaxFonts)&&sp->fontinfo[i];i++) {
	XFreeFont(DARGC sp->fontinfo[i]);
	sp->fontinfo[i] = nil;
    }
    if (name=StrToken(nlist,",",&ts)) do {
#	ifdef X10
	    if (sp->fontinfo[findex]=XOpenFont(name)) {
		sp->fontinfo[findex]->width = XQueryWidth("AA",
		    sp->fontinfo[findex]->id)/2+1;
		sprintf(np,"%s,",name);
		np += strlen(np);
		findex++;
	    }
#	else
	    foundlist = XListFonts(sp->d,name,MaxFonts,&foundcount);
	    for (i=0;(i<foundcount)&&(findex<MaxFonts);i++)
		if (sp->fontinfo[findex]=XLoadQueryFont(sp->d,foundlist[i])) {
		    sprintf(np,"%s,",foundlist[i]);
		    np += strlen(np);
		    findex++;
		}
	    XFreeFontNames(foundlist);
#	endif
    } while ((findex<MaxFonts)&&(name=StrToken(nil,",",&ts)));
    if (np>pp->fonts) *--np = '\0';
    strcpy(sp->fonts,pp->fonts);
}

void SetOutfile(sp,pp)
    State*  sp;
    Params* pp;
{
    if (sp->outfd!=stdout) fclose(sp->outfd);
    if (!strcmp(pp->outfile,"stdout")) sp->outfd = stdout;
    else if (!(sp->outfd=fopen(pp->outfile,"a"))) {
	sp->outfd = stdout;
	strcpy(pp->outfile,"stdout");
    }
    strcpy(sp->outfile,pp->outfile);
}

void SetWinsize(sp,pp)
    State*  sp;
    Params* pp;
{
#   ifdef X11
	XSizeHints sh;
#   endif X11

#   ifdef X10
	XChangeWindow(sp->w1,pp->winsize,pp->winsize);
	XChangeWindow(sp->w2,pp->winsize,pp->winsize);
#   else
	sh.flags = USSize|PAspect;
	sh.width = sh.height = pp->winsize;
	sh.min_aspect.x = sh.max_aspect.x = sh.width;
	sh.min_aspect.y = sh.max_aspect.y = sh.height;
	XResizeWindow(sp->d,sp->w1,pp->winsize,pp->winsize);
	XSetNormalHints(sp->d,sp->w1,&sh);
	XResizeWindow(sp->d,sp->w2,pp->winsize,pp->winsize);
	XSetNormalHints(sp->d,sp->w2,&sh);
	if (pp->winsize>sp->winsize||!sp->offpix) {
	    if (sp->offpix) XFreePixmap(sp->d,sp->offpix);
	    sp->offpix = XCreatePixmap(
		sp->d,_RootWindow,pp->winsize,pp->winsize,
		DefaultDepth(sp->d,DefaultScreen(sp->d))
	    );
	}
#   endif
    sp->winsize = pp->winsize;
    XFlush(DARG);
}

void InitParams(sp,pp)
    State*  sp;
    Params* pp;
{
    pp->angle = DefaultAngle;
    pp->aspect = DefaultAspect;
    pp->count = 1;
    strcpy(pp->fonts,DefaultFonts);
    pp->ptsize = DefaultPtsize;
    pp->lwidth = DefaultLwidth;
    pp->maxshift = DefaultMaxshift;
    pp->offset = DefaultOffset;
    pp->nchar = DefaultNchar;
    pp->nwin = DefaultNwin;
    pp->nvert = DefaultNvert;
    *pp->opts = '\0';
    strcpy(pp->outfile,"stdout");
    pp->size = DefaultSize;
    *pp->tag = '\0';
    pp->timegoal = DefaultTimegoal;
    pp->winsize = DefaultWinsize;
    if (strcmp(sp->fonts,pp->fonts)) SetFonts(sp,pp);
    if (strcmp(sp->outfile,pp->outfile)) SetOutfile(sp,pp);
    if (sp->winsize!=pp->winsize) SetWinsize(sp,pp);
}

void InitState(sp)
    State* sp;
{
    char*   cp;
    int	    i;
    int	    j;
#   ifdef X11
	XGCValues   gcv;
	XSizeHints  sh;
#   endif X11

    sp->altwin = false;
    sp->argindex = 1;
    *sp->fonts = '\0';
    for (i=0;i<MaxFonts;i++) sp->fontinfo[i] = nil;
    sp->infds[0] = stdin;
    sp->indepth = 0;
    sp->offpix = nil;
    sp->outfd = stdout;
    strcpy(sp->outfile,"stdout");
    sp->winsize = DefaultWinsize;
    InitParams(sp,&sp->defaults);
    sp->previndex = NilIndex;
    InitParams(sp,&sp->prevparams);
    gethostname(sp->host,MaxStr);
    strncpy(sp->disphost,getenv("DISPLAY"),MaxStr);
    if (cp=strchr(sp->disphost,':')) *cp = '\0';
    if (!strcmp(sp->disphost,"unix")) strcpy(sp->disphost,sp->host);
    for (i=0;i<MaxStr;i++) sp->outstr[i] = 'A'+(i%26);
    sp->scriptindex = -1;
    sp->w1 = XCreateSimpleWindow(
	DARGC _RootWindow,0,0,sp->winsize,sp->winsize,2,WhitePixmap,BlackPixmap
    );
    sp->cw1 = XCreateSimpleWindow(
	DARGC sp->w1,0,0,OverlapSize,OverlapSize,1,WhitePixmap,BlackPixmap
    );
    sp->w2 = XCreateSimpleWindow(
	DARGC _RootWindow,sp->winsize,0,sp->winsize,sp->winsize,2,WhitePixmap,
	BlackPixmap
    );
    sp->cw2 = XCreateSimpleWindow(
	DARGC sp->w2,0,0,OverlapSize,OverlapSize,1,WhitePixmap,BlackPixmap
    );
    sp->dest1 = sp->w1;
    sp->src1 = sp->w1;
    sp->dest2 = sp->w2;
    sp->src2 = sp->w2;
    for (i=0;i<NumOps;i++) sp->supcmds[i] = true;
    for (i=0;i<(sizeof(OptFlags)/sizeof(bool));i++) {
	((bool*)&sp->supopts)[i] = true;
    }
#   ifdef X10
	sp->supcmds[(int)ArcIndex] = false;
	sp->supcmds[(int)NopIndex] = false;
	sp->supcmds[(int)PointIndex] = false;
	sp->supopts.altfont = false;
	sp->supopts.altgc = false;
	sp->supopts.offdest = false;
	sp->supopts.offsrc =false;
	sp->supopts.polywind = false;
	sp->cursor = XCreateCursor(
	    target_width,target_height,target_bits,target_mask_bits,
	    target_x_hot,target_y_hot,_WhitePixel,_BlackPixel,GXcopy
	);
	sp->gxname = "X10";
	sp->offpix = sp->w1;
	sp->pattern = XMakePattern(1,2,4);
	sp->tile = XMakePixmap(
	    XStoreBitmap(gray1_width,gray1_height,gray1_bits),_WhitePixel,
	    _BlackPixel
	);
#   else
	sp->cursor = XCreateFontCursor(sp->d,XC_target);
	gcv.foreground = _WhitePixel;
	gcv.background = _BlackPixel;
	gcv.tile = XCreatePixmapFromBitmapData(
	    sp->d,_RootWindow,gray1_bits,gray1_width,gray1_height,_WhitePixel,
	    _BlackPixel,DefaultDepth(sp->d,DefaultScreen(sp->d))
	);
	gcv.stipple = XCreateBitmapFromData(
	    sp->d,_RootWindow,dot_bits,dot_width,dot_height
	);
	sp->gc1 = XCreateGC(
	    sp->d,_RootWindow,GCForeground|GCBackground|GCTile|GCStipple,&gcv
	);
	sp->gc2 = XCreateGC(
	    sp->d,_RootWindow,GCForeground|GCBackground|GCTile|GCStipple,&gcv
	);
	sp->gxname = "X11";
	sh.flags = USPosition|USSize|PAspect;
	sh.x = sh.y = 0;
	sh.width = sh.height = sp->winsize;
	sh.min_aspect.x = sh.max_aspect.x = sh.width;
	sh.min_aspect.y = sh.max_aspect.y = sh.height;
	XSetStandardProperties(
	    sp->d,sp->w1,"gbench w1","gbench w1",None,sp->argv,sp->argc,&sh
	);
	sh.x = sp->winsize;
	XSetStandardProperties(
	    sp->d,sp->w2,"gbench w2","gbench w2",None,sp->argv,sp->argc,&sh
	);
	for (i=0;i<MaxStr;i++) {
	    sp->texts[i].chars = sp->outstr+i;
	    sp->texts[i].nchars = 1;
	    sp->texts[i].delta = 0;
	}
#   endif
    MapWin(sp,sp->w1);
    XClearWindow(DARGC sp->w1);
    XFlush(DARG);
}

int main(argc,argv)
    int	    argc;
    char*   argv[];
{
    State   s;
    State*  sp = &s;

    s.argc = argc;
    s.argv = argv;
    ProfControl(0);
    if (!(s.d=XOpenDisplay(0))) Fatal(pname,"Couldn't open display");
    InitState(&s);
    ExecCmds(&s);
    XFlush(DARG);
    return 0;
}
