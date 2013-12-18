#include <gbench.h>

int Abs(n)
    int n;
{
    return n>=0?n:-n;
}

void CopyParams(dp,sp)
    Params* dp;
    Params* sp;

{
    bcopy(sp,dp,sizeof(Params));
}

void Fatal(p,e)
    char*   p;
    char*   e;
{
    fprintf(stderr,"%s:  ",p);
    fprintf(stderr,e,p);
    putc('\n',stderr);
    exit(1);
}

int FontIndex(sp,sizep)
    State*  sp;
    int*    sizep;
{
    int i;
    int	mindist = _DisplayHeight;
    int minindex = -1;
    int minsize = *sizep;
    int newsize;

    for (i=0;(i<MaxFonts)&&sp->fontinfo[i];i++) {
#	ifdef X10
	    newsize = sp->fontinfo[i]->height;
#	else
	    newsize = sp->fontinfo[i]->ascent;
#	endif
	if (Abs(*sizep-newsize)<mindist) {
	    mindist = Abs(*sizep-newsize);
	    minindex = i;
	    minsize = newsize;
	}
    }
    *sizep = minsize;
    return minindex;
}

Load GetLoad()
{
    int		    kmem;
    struct nlist    nl[2];
#   ifdef FIXLOAD
	long	    load;
#   else
	double	    load;
#   endif

    nl[0].n_name = "_avenrun";
    nl[1].n_name = nil;
    nlist("/vmunix",nl);
    if (!nl[0].n_type) return 0.;
    if ((kmem=open("/dev/kmem",0))<0) return 0.;
    lseek(kmem,nl[0].n_value,0);
    read(kmem,&load,sizeof(load));
#   ifdef FIXLOAD
	return load/256.;
#   else
	return load;
#   endif
}

OpIndex GetOpIndex(opname)
    char* opname;
{
    int i;
    
    for (i=0;i<NumOps;i++) {
	if (!strncmp(opname,OpNames[i][0],strlen(opname))) return (OpIndex)i;
    }
    return NilIndex;
}

Rtime GetTime(sp)
    State*  sp;
{
    struct timeval t;

    XSync(DARGC true);
    gettimeofday(&t,0);
    return t.tv_sec+t.tv_usec/1000000.;
}

void Help(f,s)
    FILE*   f;
    char*   s;
{
    putc('#',f);
    fputs(s,f);
    putc('\n',f);
}

int InputMask(cp)
    Cmd* cp;
{
    int im = 0;
    
    if (cp->o.drag) im |= (ButtonPressMask|ButtonReleaseMask);
    if (!cp->o.poll) im |= PointerMotionMask;
    if (cp->o.overlap) im |= (ExposeRegionMask|ExposureMask);
    return im;
}

int Limit(n,min,max)
    int	n;
    int	min;
    int	max;
{
    if (n<min) n = min;
    else if (n>max) n = max;
    return n;
}

void MapWin(sp,w)
    State*  sp;
    Window  w;
{
#   ifdef X11
        XEvent e;
        XSelectInput(sp->d,w,StructureNotifyMask);
#   endif X11
    XMapWindow(DARGC w);
#   ifdef X11
        do
            XNextEvent(sp->d,&e);
        while ((e.type!=MapNotify)||(e.xmap.event!=w));
#   endif X11
}

void MoveClip(sp,cp,x,y)
    State*  sp;
    Cmd*    cp;
    int	    x;
    int	    y;
{
    if (cp->o.overlap) {
	x += cp->p.maxshift/2-OverlapSize/2;
	y += cp->p.maxshift/2-OverlapSize/2;
	XMoveWindow(DARGC sp->cw1,x,y);
	XMoveWindow(DARGC sp->cw2,x,y);
    }
}

void MoveVert(sp,cp,vcount)
    State*  sp;
    Cmd*    cp;
    int	    vcount;
{
    int i;
    
    if (cp->dx||cp->dy) for (i=0;i<=vcount;i++) {
	sp->v[i].x += cp->dx;
	sp->v[i].y += cp->dy;
    }
}

FILE* Out(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    if (cp->o.silent) return stderr;
    else return sp->outfd;
}

void PrintConfig(f,sp)
    FILE*   f;
    State*  sp;
{
    int t = time(0);

    fprintf(f,"# host=%s\n",sp->host);
    fprintf(f,"# display=%s\n",getenv("DISPLAY"));
    fprintf(f,"# graphics=%s\n",sp->gxname);
    fprintf(f,"# time=%s",ctime(&t));
    fprintf(f,"# version=%.1f\n",Version);
    fprintf(f,"# tag=%s\n",sp->defaults.tag);
}

void PrintParam(f,pp,pre,name,post)
    FILE*   f;
    Params* pp;
    char*   pre;
    char*   name;
    char*   post;
{
    fprintf(f,"%s",pre);
    if (!strcmp(name,"angle")) fprintf(f,"%s=%d",name,pp->angle);
    else if (!strcmp(name,"aspect")) fprintf(f,"%s=%.1f",name,pp->aspect);
    else if (!strcmp(name,"count")) fprintf(f,"%s=%d",name,pp->count);
    else if (!strcmp(name,"fonts")) fprintf(f,"%s=%s",name,pp->fonts);
    else if (!strcmp(name,"ptsize")) fprintf(f,"%s=%d",name,pp->ptsize);
    else if (!strcmp(name,"lwidth")) fprintf(f,"%s=%d",name,pp->lwidth);
    else if (!strcmp(name,"maxshift")) fprintf(f,"%s=%d",name,pp->maxshift);
    else if (!strcmp(name,"offset")) fprintf(f,"%s=%.1f",name,pp->offset);
    else if (!strcmp(name,"nchar")) fprintf(f,"%s=%d",name,pp->nchar);
    else if (!strcmp(name,"nwin")) fprintf(f,"%s=%d",name,pp->nwin);
    else if (!strcmp(name,"nvert")) fprintf(f,"%s=%d",name,pp->nvert);
    else if (!strcmp(name,"opts")) fprintf(f,"%s=%s",name,pp->opts);
    else if (!strcmp(name,"outfile")) fprintf(f,"%s=%s",name,pp->outfile);
    else if (!strcmp(name,"size")) fprintf(f,"%s=%d",name,pp->size);
    else if (!strcmp(name,"tag")) fprintf(f,"%s=%s",name,pp->tag);
    else if (!strcmp(name,"timegoal")) fprintf(f,"%s=%d",name,pp->timegoal);
    else if (!strcmp(name,"winsize")) fprintf(f,"%s=%d",name,pp->winsize);
    fprintf(f,"%s",post);
}

void PrintParams(f,sp,pp)
    FILE*   f;
    State*  sp;
    Params* pp;
{
    int i;

    PrintParam(f,pp,"# ","angle","\n");
    PrintParam(f,pp,"# ","aspect","\n");
    PrintParam(f,pp,"# ","count","\n");
    PrintParam(f,pp,"# ","fonts","\n");
    PrintParam(f,pp,"# ","ptsize","\n");
    PrintParam(f,pp,"# ","lwidth","\n");
    PrintParam(f,pp,"# ","maxshift","\n");
    PrintParam(f,pp,"# ","offset","\n");
    PrintParam(f,pp,"# ","nchar","\n");
    PrintParam(f,pp,"# ","nwin","\n");
    PrintParam(f,pp,"# ","nvert","\n");
    PrintParam(f,pp,"# ","opts","\n");
    PrintParam(f,pp,"# ","outfile","\n");
    PrintParam(f,pp,"# ","size","\n");
    PrintParam(f,pp,"# ","tag","\n");
    PrintParam(f,pp,"# ","timegoal","\n");
    PrintParam(f,pp,"# ","winsize","\n");
}

void PrintHelp(f)
    FILE* f;
{
    int	    i;
    int	    j;

    Help(f,"Commands:");
    for (i=0;i<NumOps;i++) {
	fprintf(f,"#  %-8s",OpNames[i][0]);
	for (j=0;(j<MaxPositional)&&OpNames[i][1+j];j++) {
	    fprintf(f," [%-6s]",OpNames[i][1+j]);
	}
	fprintf(f," [n=v]*\n");
    }
    Help(f,"  config");
    Help(f,"  defaults [n=v]*");
    Help(f,"  help");
    Help(f,"  init");
    Help(f,"  script   [filename]");
    Help(f,"  quit");
    Help(f,"  !");
    Help(f,"  #");
    Help(f,"Options:");
    Help(f,"  ac    Alternate colors");
    Help(f,"  af    Alternate fonts");
    Help(f,"  ag    Alternate graphics contexts");
    Help(f,"  aw    Alternate windows");
    Help(f,"  d     Drag");
    Help(f,"  f     Fill");
    Help(f,"  i     Invert");
    Help(f,"  m     Monitor for profiling");
    Help(f,"  n     No options");
    Help(f,"  o     Overlap");
    Help(f,"  os    Offscreen source");
    Help(f,"  od    Offscreen destination");
    Help(f,"  p     Poll");
    Help(f,"  ps    Polygon self-intersecting");
    Help(f,"  pw    Polygon winding number fill");
    Help(f,"  r     Reset defaults");
    Help(f,"  s     Stipple");
    Help(f,"  t     Tile");
    Help(f,"  u     Unbatched");
}

void ProfControl(i)
    int i;
{
# ifdef GPROF
    moncontrol(i);
# endif GPROF
}

char* StrToken(s1,s2,tsp)
    char*	s1;
    char*	s2;
    TokenState*	tsp;
{
    char* bp;

    if (s1) {
	tsp->cur = s1;
	tsp->end = s1+strlen(s1);
    }
    else {
	tsp->cur += strlen(tsp->cur)+1;
	if (tsp->cur>=tsp->end) return nil;
    }
    tsp->cur += strspn(tsp->cur,s2);
    if (bp=strpbrk(tsp->cur,s2)) *bp = '\0';
    if (tsp->cur>=tsp->end) return nil;
    else return tsp->cur;
}

void UnmapWin(sp,w)
    State*  sp;
    Window  w;
{
#   ifdef X11
        XEvent e;
        XSelectInput(sp->d,w,StructureNotifyMask);
#   endif X11
    XUnmapWindow(DARGC w);
#   ifdef X11
        do
            XNextEvent(sp->d,&e);
        while ((e.type!=UnmapNotify)||(e.xunmap.event!=w));
#   endif X11
}

#ifdef X10
void X10Draw(sp,cp,vcount)
    State*  sp;
    Cmd*    cp;
    int	    vcount;
{
    if (cp->o.fill) {
	if (cp->o.tile) {
	    XDrawTiled(cp->w,sp->v,vcount,sp->tile,cp->func,AllPlanes);
	}
	else XDrawFilled(cp->w,sp->v,vcount,cp->colorfg,cp->func,AllPlanes);
    }
    else {
	if (cp->o.stipple) {
	    XDrawDashed(
		cp->w,sp->v,vcount,cp->p.lwidth,cp->p.lwidth,cp->colorfg,
		sp->pattern,cp->func,AllPlanes
	    );
	}
	else if (cp->o.tile) {
	    XDrawPatterned(
		cp->w,sp->v,vcount,cp->p.lwidth,cp->p.lwidth,cp->colorfg,
		cp->colorbg,sp->pattern,cp->func,AllPlanes
	    );
	}
	else if (vcount==2) {
	    XLine(
		cp->w,sp->v[0].x,sp->v[0].y,sp->v[1].x,sp->v[1].y,cp->p.lwidth,
		cp->p.lwidth,cp->colorfg,cp->func,AllPlanes
	    );
	}
	else {
	    XDraw(
		cp->w,sp->v,vcount,cp->p.lwidth,cp->p.lwidth,cp->colorfg,
		cp->func,AllPlanes
	    );
	}
    }
}
#endif X10
