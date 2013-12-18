#include <gbench.h>

void StartArc(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    double  adjust;
    int	    area = cp->p.size*cp->p.size;
    int	    origin;
    double  theta;

    cp->size[0] = sqrt(area/cp->p.aspect);
    cp->size[1] = sqrt(area*cp->p.aspect);
    if (cp->p.aspect!=1.) {
	theta = cp->p.angle*Pi/180;
	if (theta<(Pi/2)) adjust = 0.;
	else if (theta<(3*Pi/2)) adjust = Pi;
	else adjust = 2*Pi;
	theta = atan(tan(theta)/cp->p.aspect)+adjust;
	cp->size[2] = theta*180/Pi*64;
    }
    else cp->size[2] = cp->p.angle*64;
    origin = Border+cp->p.lwidth;
    cp->x = cp->y = Border;
    cp->bbx = cp->size[0]+2*cp->p.lwidth;
    cp->bby = cp->size[1]+2*cp->p.lwidth;
    if (cp->o.fill) {
	MoveClip(
	    sp,cp,cp->x+cp->bbx/2-OverlapSize,cp->y+cp->bby/2-OverlapSize
	);
    }
    else MoveClip(sp,cp,cp->x,cp->y+cp->bby/2);
    Start(sp,cp);
}

void DoArc(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int i;
    int x = cp->x+cp->p.lwidth;
    int y = cp->y+cp->p.lwidth;

    for (i=0;i<cp->p.count;i++) {
	StartIteration(sp,cp);
#	ifdef X11
	    if (cp->o.fill) {
		XFillArc(
		    sp->d,cp->dest,cp->gc,x,y,cp->size[0],cp->size[1],0,
		    cp->size[2]
		);
	    }
	    else {
		XDrawArc(
		    sp->d,cp->dest,cp->gc,x,y,cp->size[0],cp->size[1],0,
		    cp->size[2]
		);
	    }
#	endif X11
    }
}

void StartBlit(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    Pixel   colorfg;
    int	    i;

    Start(sp,cp);
    cp->bbx = cp->bby = 0;
    colorfg = cp->colorfg;
    MoveClip(sp,cp,Border+cp->p.size/2,Border+cp->p.size/2);
    for (i=0;i<2;i++) {
	StartIteration(sp,cp);
#	ifdef X10
	    XPixFill(
		cp->w,Border,Border,cp->p.size,cp->p.size,colorfg,0,cp->func,
		AllPlanes
	    );
#	else
	    XFillRectangle(
		sp->d,cp->src,sp->gc1,Border,Border,cp->p.size,cp->p.size
	    );
#	endif
    }
    cp->x = cp->y = Border;
    cp->size[0] = cp->p.offset*cp->p.size;
}

void DoBlit(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    XEvent		    e;
    int			    i;
    XGraphicsExposeEvent*   xep;

#   ifdef X10
	xep = (XGraphicsExposeEvent*)&e;
#   else
	xep = &e.xgraphicsexpose;
#   endif
    for (i=0;i<cp->p.count;i++) {
	StartIteration(sp,cp);
#	ifdef X10
	    XCopyArea(
		cp->w,Border,Border,cp->x+cp->size[0],cp->y+cp->size[0],
		cp->p.size,cp->p.size,cp->func,AllPlanes
	    );
	    if (cp->o.overlap) do {
		XMaskEvent(ExposeRegion|ExposureMask,&e);
		if (e.type==ExposeRegion) {
		    XPixFill(
			cp->w,xep->x,xep->y,xep->width,xep->height,cp->colorfg,
			0,cp->func,AllPlanes
		    );
		}
	    } while (e.type!=ExposureMask);
#	else
	    XCopyArea(
		sp->d,cp->src,cp->dest,cp->gc,Border,Border,cp->p.size,
		cp->p.size,cp->x+cp->size[0],cp->y+cp->size[0]
	    );
	    if (cp->o.overlap) do {
		XMaskEvent(sp->d,ExposureMask,&e);
		if (e.type==GraphicsExpose) {
		    XFillRectangle(
			sp->d,cp->dest,cp->gc,xep->x,xep->y,xep->width,
			xep->height
		    );
		}
	    } while ((e.type==GraphicsExpose)&&xep->count);
#	endif
    }
}

void StartMap(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int	    wheight;
    int	    i;
    int	    ncols = _DisplayWidth/cp->p.size;
#   ifdef X11
        XSetWindowAttributes swa;
#   endif X11

    cp->x = cp->y = cp->bbx = cp->bby = 0;
    cp->p.nwin = Limit(cp->p.nwin,1,ncols*_DisplayHeight/cp->p.size);
    cp->o.altwin = false;
    wheight = cp->p.size*(1+cp->p.nwin/ncols);
    Start(sp,cp);
    UnmapWin(sp,cp->w);
    cp->w = XCreateSimpleWindow(
	DARGC _RootWindow,0,0,_DisplayWidth,wheight,1,WhitePixmap,BlackPixmap
    );
    for (i=0;i<cp->p.nwin;i++) {
	XCreateSimpleWindow(
	    DARGC cp->w,cp->p.size*(i%ncols),cp->p.size*(i/ncols),cp->p.size,
	    cp->p.size,1,WhitePixmap,BlackPixmap
	);
    }
#   ifdef X11
        swa.override_redirect = 1;
        XChangeWindowAttributes(sp->d,cp->w,CWOverrideRedirect,&swa);
#   endif X11
    XRaiseWindow(DARGC cp->w);
    XMapWindow(DARGC cp->w);
    XSelectInput(DARGC cp->w,InputMask(cp));
}

void DoMap(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int i;

    for (i=0;i<cp->p.count;i++) {
	XMapSubwindows(DARGC cp->w);
	XUnmapSubwindows(DARGC cp->w);
    }
}

void FinishMap(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    Finish(sp,cp);
    XUnmapWindow(DARGC cp->w);
    XDestroyWindow(DARGC cp->w);
    cp->w = sp->w1;
    MapWin(sp,cp->w);
    XFlush(DARG);
}

void DoNop(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int	i;

#   ifdef X11
	for (i=0;i<cp->p.count;i++) XNoOp(sp->d);
#   endif X11
}

void StartPoint(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    cp->x = cp->y = Border;
    cp->bbx = cp->bby = 1;
    MoveClip(sp,cp,cp->x,cp->y);
    Start(sp,cp);
}

void DoPoint(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int i;
    
#   ifdef X11
	for (i=0;i<cp->p.count;i++) {
	    StartIteration(sp,cp);
	    XDrawPoint(sp->d,cp->dest,cp->gc,cp->x,cp->y);
	}
#   endif
}

void StartPoly(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int	    fillrule;
    int	    i;
    int	    index;
    int	    origin = Border+cp->p.lwidth;
    int	    r = cp->p.size/2;
    double  theta;

    if (cp->o.polyself) cp->p.nvert = cp->p.nvert/2*2+1;
    theta = 2*Pi/cp->p.nvert;
    cp->x = cp->y = Border;
    cp->bbx = cp->bby = cp->p.size+2*cp->p.lwidth;
    for (i=0;i<=cp->p.nvert;i++) {
	index = cp->o.polyself?(i*(cp->p.nvert/2))%cp->p.nvert:i;
	sp->v[i].x = origin+r*(1+cos(PolyAngle+index*theta));
	sp->v[i].y = origin+r*(1-sin(PolyAngle+index*theta));
    }
#   ifdef X10
	sp->v[0].flags = VertexStartClosed;
	for (i=1;i<cp->p.nvert;i++) sp->v[i].flags = 0;
	sp->v[cp->p.nvert].flags = VertexEndClosed;
#   else
	fillrule = cp->o.polywind?WindingRule:EvenOddRule;
	XSetFillRule(sp->d,sp->gc1,fillrule);
	XSetFillRule(sp->d,sp->gc2,fillrule);
#   endif
    if (cp->o.fill) MoveClip(sp,cp,cp->x+r,cp->x+r);
    else MoveClip(sp,cp,sp->v[0].x,sp->v[0].y);
    Start(sp,cp);
}

void DoPoly(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int i;
#   ifdef X11
	int shape = cp->o.polyself?Complex:Convex;
#   endif X11

    MoveVert(sp,cp,cp->p.nvert);
    for (i=0;i<cp->p.count;i++) {
	StartIteration(sp,cp);
#	ifdef X10
	    X10Draw(sp,cp,cp->p.nvert+1);
#	else
	    if (cp->o.fill) {
		XFillPolygon(
		    sp->d,cp->dest,cp->gc,sp->v,cp->p.nvert,shape,
		    CoordModeOrigin
		);
	    }
	    else {
		XDrawLines(
		    sp->d,cp->dest,cp->gc,sp->v,cp->p.nvert+1,CoordModeOrigin
		);
	    }
#	endif
    }
}

void StartRect(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int i;
    int origin = Border+cp->p.lwidth;

    cp->x = cp->y = Border;
    cp->bbx = cp->bby = cp->p.size+2*cp->p.lwidth;
#   ifdef X10
	for (i=0;i<5;i++) {
	    sp->v[i].x = origin;
	    sp->v[i].y = origin;
	}
	sp->v[1].x += cp->p.size;
	sp->v[2].x += cp->p.size;
	sp->v[2].y += cp->p.size;
	sp->v[3].y += cp->p.size;
	sp->v[0].flags = VertexStartClosed;
	for (i=1;i<4;i++) sp->v[i].flags = 0;
	sp->v[4].flags = VertexEndClosed;
#   endif X10
    if (cp->o.fill) MoveClip(sp,cp,cp->x+cp->bbx/2,cp->y+cp->bby/2);
    else MoveClip(sp,cp,cp->x,cp->y);
    Start(sp,cp);
}

void DoRect(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int	i;
    int x = cp->x+cp->p.lwidth;
    int y = cp->y+cp->p.lwidth;

#   ifdef X10
    if (!cp->o.fill) MoveVert(sp,cp,5);
#   endif X10
    for (i=0;i<cp->p.count;i++) {
	StartIteration(sp,cp);
#	ifdef X10
	    if (cp->o.fill) {
		if (cp->o.tile) {
		    XTileFill(
			cp->w,x,y,cp->p.size,cp->p.size,sp->tile,0,cp->func,
			AllPlanes
		    );
		}
		else {
		    XPixFill(
			cp->w,x,y,cp->p.size,cp->p.size,cp->colorfg,0,cp->func,
			AllPlanes
		    );
		}
	    }
	    else X10Draw(sp,cp,5);
#	else
	    if (cp->o.fill) {
		XFillRectangle(
		    sp->d,cp->dest,cp->gc,x,y,cp->p.size,cp->p.size
		);
	    }
	    else {
		XDrawRectangle(
		    sp->d,cp->dest,cp->gc,x,y,cp->p.size,cp->p.size
		);
	    }
#	endif
    }
}

void StartText(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int findex = FontIndex(sp,&cp->p.ptsize);
    int	i;

    cp->x = cp->y = Border;
    Start(sp,cp);
    if (findex<0) {
	cp->p.ptsize = 0;
	cp->fontid = nil;
	cp->bbx = cp->bby = 0;
	cp->result = NoFonts;
    }
    else {
#	ifdef X10
	    cp->p.ptsize = sp->fontinfo[findex]->height;
	    cp->fontid = sp->fontinfo[findex]->id;
	    cp->bbx = cp->p.nchar*sp->fontinfo[findex]->width;
	    cp->bby = sp->fontinfo[findex]->height;
#	else
	    cp->p.ptsize = sp->fontinfo[findex]->ascent;
	    XSetFont(sp->d,sp->gc1,sp->fontinfo[findex]->fid);
	    XSetFont(sp->d,sp->gc2,sp->fontinfo[findex]->fid);
	    cp->fontid = sp->fontinfo[findex]->fid;
	    cp->size[0] = sp->fontinfo[findex]->max_bounds.ascent;
	    cp->bbx = cp->p.nchar*sp->fontinfo[findex]->max_bounds.width;
	    cp->bby = cp->size[0]+sp->fontinfo[findex]->max_bounds.descent;
	    if (cp->o.altfont) for (i=0;i<cp->p.nchar;i++) {
		sp->texts[i].font = cp->fontid;
	    }
#	endif
    }
    MoveClip(sp,cp,cp->x+cp->bbx/2,cp->y+cp->bby/2);
}

void DoText(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int	i;

    if (cp->result==NoFonts) return;
    for (i=0;i<cp->p.count;i++) {
	StartIteration(sp,cp);
#	ifdef X10
	    if (cp->o.tile) {
		XTextPad(
		    cp->w,cp->x,cp->y,sp->outstr,cp->p.nchar,cp->fontid,0,0,
		    cp->colorfg,cp->colorbg,cp->func,AllPlanes
		);
	    }
	    else {
		XTextMaskPad(
		    cp->w,cp->x,cp->y,sp->outstr,cp->p.nchar,cp->fontid,0,0,
		    cp->colorfg,cp->func,AllPlanes
		);
	    }
#	else
	    if (cp->o.altfont) {
		XDrawText(
		    sp->d,cp->dest,cp->gc,cp->x,cp->y+cp->size[0],sp->texts,
		    cp->p.nchar
		);
	    }
	    else if (cp->o.tile) {
		XDrawImageString(
		    sp->d,cp->dest,cp->gc,cp->x,cp->y+cp->size[0],sp->outstr,
		    cp->p.nchar
		);
	    }
	    else {
		XDrawString(
		    sp->d,cp->dest,cp->gc,cp->x,cp->y+cp->size[0],sp->outstr,
		    cp->p.nchar
		);
	    }
#	endif
    }
}

void StartVec(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    double  theta = cp->p.angle*Pi/180;
    int	    xoffset = cp->p.lwidth*Sign(cos(theta));
    int	    yoffset = cp->p.lwidth*Sign(-sin(theta));
    int	    xorigin = sp->winsize/2;
    int	    yorigin = sp->winsize/2;

#   ifdef X10
	sp->v[0].flags = 0;
	sp->v[1].flags = 0;
#   endif X10
    cp->x = xorigin;
    cp->y = yorigin;
    xorigin += xoffset;
    yorigin += yoffset;
    cp->bbx = cp->p.size*cos(theta);
    cp->bby = cp->p.size*-sin(theta);
    sp->v[0].x = xorigin;
    sp->v[0].y = yorigin;
    sp->v[1].x = xorigin+cp->bbx;
    sp->v[1].y = yorigin+cp->bby;
    cp->bbx += 2*xoffset;
    cp->bby += 2*yoffset;
    cp->o.fill = false;
    MoveClip(sp,cp,cp->x+cp->bbx/2,cp->y+cp->bby/2);
    Start(sp,cp);
}

void DoVec(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    int i;

    MoveVert(sp,cp,2);
    for (i=0;i<cp->p.count;i++) {
	StartIteration(sp,cp);
#	ifdef X10
	    X10Draw(sp,cp,2);
#	else
	    XDrawLine(
		sp->d,cp->dest,cp->gc,sp->v[0].x,sp->v[0].y,sp->v[1].x,
		sp->v[1].y
	    );
#	endif
    }
}
