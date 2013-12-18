#include <gbench.h>

void Start(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int i;
    int	style;

    cp->colorfg = _WhitePixel;
    cp->colorbg = _BlackPixel;
    if (cp->p.maxshift>0) cp->dx = cp->dy = 1;
    else cp->dx = cp->dy = 0;
    cp->gc = sp->gc1;
    cp->shift = 0;
    cp->w = sp->w1;
    sp->dest1 = sp->w1;
    sp->src1 = sp->w1;
#   ifdef X11
	if (cp->o.offdest||cp->o.offsrc) {
	    if (!sp->offpix) {
		cp->o.offdest = cp->o.offsrc = false;
		cp->result = NoOffscreenMem;
	    }
	    else {
		if (cp->o.offdest) sp->dest1 = sp->offpix;
		if (cp->o.offsrc) sp->src1 = sp->offpix;
		XFillRectangle(
		    sp->d,sp->offpix,DefaultGC(sp->d,DefaultScreen(sp->d)),0,0,
		    sp->winsize,sp->winsize
		);
	    }
	}
#   endif X11
    cp->dest = sp->dest1;
    cp->src = sp->src1;
    fprintf(Out(sp,cp),"%s",OpNames[(int)cp->index][0]);
    for (i=0;(i<MaxPositional)&&OpNames[(int)cp->index][1+i];i++) {
	PrintParam(Out(sp,cp),&cp->p,"  ",OpNames[(int)cp->index][1+i],"");
    }
    PrintParam(Out(sp,cp),&cp->p,"  ","count","\n");
    fflush(Out(sp,cp));
    if (cp->o.altwin&&!sp->altwin) {
	MapWin(sp,sp->w2);
	sp->altwin = true;
    }
    else if (sp->altwin&&!cp->o.altwin) {
	UnmapWin(sp,sp->w2);
	sp->altwin = false;
    }
    if (cp->o.overlap) {
	XMapWindow(DARGC sp->cw1);
	if (cp->o.altwin) XMapWindow(DARGC sp->cw2);
    }
    XClearWindow(DARGC sp->w1);
    XClearWindow(DARGC sp->w2);
    XRaiseWindow(DARGC sp->w1);
    if (cp->o.altwin) XRaiseWindow(DARGC sp->w2);
    XSelectInput(DARGC sp->w1,InputMask(cp));
    if (cp->o.altwin) XSelectInput(DARGC sp->w2,InputMask(cp));
#   ifdef X11
	XSetBackground(sp->d,sp->gc1,cp->colorbg);
	XSetForeground(sp->d,sp->gc1,cp->colorfg);
	XSetFunction(sp->d,sp->gc1,cp->func);
	XSetFunction(sp->d,sp->gc2,cp->func);
	if (cp->o.stipple&&cp->o.tile) style = FillOpaqueStippled;
	else if (cp->o.stipple) style = FillStippled;
	else if (cp->o.tile) style = FillTiled;
	else style = FillSolid;
	XSetFillStyle(sp->d,sp->gc1,style);
	XSetFillStyle(sp->d,sp->gc2,style);
	if (cp->o.stipple) style = LineOnOffDash;
	else if (cp->o.tile) style = LineDoubleDash;
	else style = LineSolid;
	XSetLineAttributes(sp->d,sp->gc1,cp->p.lwidth,style,CapButt,JoinMiter);
	XSetLineAttributes(sp->d,sp->gc2,cp->p.lwidth,style,CapButt,JoinMiter);
#   endif X11
    XSync(DARGC 1);
    if (cp->o.profile) ProfControl(1);
}

void StartIteration(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    Pixel colortemp;

    if (!cp->o.drag) {
	cp->shift += cp->dx;
	if ((cp->shift<0)||(cp->shift>cp->p.maxshift)) {
	    cp->dx = -cp->dx;
	    cp->dy = -cp->dy;
	    cp->shift += 2*cp->dx;
	}
	cp->x += cp->dx;
	cp->y += cp->dy;
    }
    if (cp->o.altcolor) {
	colortemp = cp->colorbg;
	cp->colorbg = cp->colorfg;
	cp->colorfg = colortemp;
#	ifdef X11
	    XSetBackground(sp->d,sp->gc1,cp->colorbg);
	    XSetForeground(sp->d,sp->gc1,cp->colorfg);
#	endif X11
    }
    if ((cp->o.altgc)&&(cp->gc==sp->gc1)) cp->gc = sp->gc2;
    else cp->gc = sp->gc1;
    if ((cp->o.altwin)&&(cp->w==sp->w1)) {
	cp->dest = sp->dest2;
	cp->src = sp->src2;
	cp->w = sp->w2;
    }
    else {
	cp->dest = sp->dest1;
	cp->src = sp->src1;
	cp->w = sp->w1;
    }
    if (cp->o.unbatched) XFlush(DARG);
}

void Erase(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int x = cp->x;
    int y = cp->y;
    int bbx = cp->bbx;
    int bby = cp->bby;

    if (cp->bbx<0) {
	x += cp->bbx;
	bbx = -bbx;
    }
    if (cp->bby<0) {
	y += cp->bby;
	bby = -bby;
    }
#   ifdef X10
	XPixFill(cp->w,x,y,bbx,bby,_BlackPixel,0,GXcopy,AllPlanes);
#   else
	XFillRectangle(
	    sp->d,cp->dest,DefaultGC(sp->d,DefaultScreen(sp->d)),x,y,bbx,bby
	);
#   endif
}

void Finish(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int	    i;

    if (cp->o.profile) ProfControl(0);
    if (cp->o.overlap) {
	XUnmapWindow(DARGC sp->cw1);
	if (cp->o.altwin) XUnmapWindow(DARGC sp->cw2);
    }
#   ifdef X11
	if (cp->o.offdest) {
	    XCopyArea(
		sp->d,sp->offpix,sp->w1,sp->gc1,0,0,sp->winsize,sp->winsize,0,0
	    );
	}
#   endif X11
    XSync(DARGC 0);
    fprintf(Out(sp,cp),"# ");
    if (cp->result!=Succeeded) {
	fprintf(Out(sp,cp),"result=%s",ResultMsgs[(int)cp->result]);
    }
    else {
	fprintf(
	    Out(sp,cp),"hload=%.1f  time=%.3fmsec  rate=%.1f/sec",GetLoad(),
	    cp->time*1000/cp->iterations,cp->iterations/cp->time
	);
    }
    if (strlen(cp->p.tag)) fprintf(Out(sp,cp),"  tag=%s  ",cp->p.tag);
    putc('\n',Out(sp,cp));
    putc('\n',stderr);
    fflush(Out(sp,cp));
    fflush(stderr);
    if (strcmp(sp->fonts,sp->defaults.fonts)) SetFonts(sp,&sp->defaults);
    if (strcmp(sp->outfile,sp->defaults.outfile)) SetOutfile(sp,&sp->defaults);
    if (sp->winsize!=sp->defaults.winsize) SetWinsize(sp,&sp->defaults);
}

void ExecDrag(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    XEvent	    e;
    int		    i;
    XMotionEvent*   mep;
    double	    t1;
    Window	    qw;

#   ifdef X10
	mep = (XMotionEvent*)&e;
#   else
	mep = &e.xmotion;
#   endif
    cp->iterations = 0;
    cp->o.altwin = false;
    XClearWindow(DARGC sp->w1);
    (*Ops[(int)cp->index][(int)StartOp])(sp,cp);
    Help(
	stderr,
	"    Drag test: move cursor in window while pressing a mouse button"
    );
    fflush(stderr);
    XDefineCursor(DARGC cp->w,sp->cursor);
    do XNextEvent(DARGC &e); while (e.type!=ButtonPress);
    t1 = GetTime(sp);
    while (e.type!=ButtonRelease) {
	XSync(DARGC false);
	if (cp->o.poll) {
	    if (!XCheckMaskEvent(DARGC ButtonReleaseMask,&e)) {
		e.type = MotionNotify;
#		ifdef X10
		    XQueryMouse(cp->w,&mep->x,&mep->y,&qw);
#		else
		    XQueryPointer(
			sp->d,cp->w,&qw,&qw,&i,&i,&mep->x,&mep->y,&i
		    );
#		endif
	    }
	}
	else do {
	    XNextEvent(DARGC &e);
	} while (QLength(DARG) && (e.type==MotionNotify));
	(*Ops[(int)cp->index][(int)EraseOp])(sp,cp);
	cp->dx = mep->x-cp->x;
	cp->dy = mep->y-cp->y;
	cp->x = mep->x;
	cp->y = mep->y;
	(*Ops[(int)cp->index][(int)DoOp])(sp,cp);
	cp->iterations++;
    }
    cp->time = GetTime(sp)-t1;
    XUndefineCursor(DARGC cp->w);
    (*Ops[(int)cp->index][(int)FinishOp])(sp,cp);
    XFlush(DARG);
}

void ExecOp(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    
    bool    done;
    int	    i;
    double  t1;

    (*Ops[(int)cp->index][(int)StartOp])(sp,cp);
    cp->iterations = 1;
    done = (bool)(cp->result!=Succeeded);
    while (!done) {
	t1 = GetTime(sp);
	for (i=0;i<cp->iterations;i++) {
	    (*Ops[(int)cp->index][(int)DoOp])(sp,cp);
	}
	cp->time = GetTime(sp)-t1;
	if (((int)cp->time)>=cp->p.timegoal) done = true;
	else {
	    cp->iterations =
		(int)(cp->iterations*cp->p.timegoal/(cp->time+.0001))+1;
	}
    }
    (*Ops[(int)cp->index][(int)FinishOp])(sp,cp);
}

void ExecCmds(sp)
    State* sp;
{
    Cmd	c;

    while (GetCmd(sp,&c)) {
	if (c.o.drag) ExecDrag(sp,&c);
	else ExecOp(sp,&c);
    }
}
