#include <gbench.h>

Result ParseOpts(os,sfp,ofp)
    char*	os;
    OptFlags*	sfp;
    OptFlags*	ofp;
{
    int		i;
    char	ps[MaxStr];
    bool*	ofa = (bool*)ofp;
    char*	osp = os;
    Result	result = Succeeded;
    bool*	sfa = (bool*)sfp;
    char*	t;
    TokenState	ts;

    for (i=0;i<(sizeof(OptFlags)/sizeof(bool));i++) ofa[i] = false;
    strcpy(ps,osp);
    *osp = '\0';
    if (t=StrToken(ps,",",&ts)) do {
	i = -1;
	if (!strcmp(t,"ac")) i = &ofp->altcolor-ofa;
	else if (!strcmp(t,"af")) i = &ofp->altfont-ofa;
	else if (!strcmp(t,"ag")) i = &ofp->altgc-ofa;
	else if (!strcmp(t,"aw")) i = &ofp->altwin-ofa;
	else if (!strcmp(t,"d")) i = &ofp->drag-ofa;
	else if (!strcmp(t,"f")) i = &ofp->fill-ofa;
	else if (!strcmp(t,"i")) i = &ofp->invert-ofa;
	else if (!strcmp(t,"m")) i = &ofp->profile-ofa;
	else if (!strcmp(t,"o")) i = &ofp->overlap-ofa;
	else if (!strcmp(t,"os")) i = &ofp->offsrc-ofa;
	else if (!strcmp(t,"od")) i = &ofp->offdest-ofa;
	else if (!strcmp(t,"p")) i = &ofp->poll-ofa;
	else if (!strcmp(t,"ps")) i = &ofp->polyself-ofa;
	else if (!strcmp(t,"pw")) i = &ofp->polywind-ofa;
	else if (!strcmp(t,"r")) i = &ofp->setdefaults-ofa;
	else if (!strcmp(t,"s")) i = &ofp->stipple-ofa;
	else if (!strcmp(t,"t")) i = &ofp->tile-ofa;
	else if (!strcmp(t,"u")) i = &ofp->unbatched-ofa;
	if ((i>=0)&&!ofa[i]) {
	    if (!sfa[i]) result = OptNotSupported;
	    ofa[i] = true;
	    sprintf(osp,"%s,",t);
	    osp += strlen(osp);
	}
    } while (t=StrToken(nil,",",&ts));
    if (osp>os) *--osp = '\0';
    return result;
}

char* GetToken(sp,first)
    State*  sp;
    bool    first;
{
    if (!sp->strinput) {
	if (first) {
	    while((sp->argindex<sp->argc)&&(*sp->argv[sp->argindex]!='-')) {
		sp->argindex++;
	    }
	    if (sp->argindex>=sp->argc) return nil;
	    else return sp->argv[sp->argindex++];
	}
	else {
	    if (*sp->argv[sp->argindex]=='-') return nil;
	    else return sp->argv[sp->argindex++];
	}
    }
    else return StrToken(first?sp->instr:nil," ",&sp->ts);
}

int ScanInt(s,min,max)
    char*   s;
    int	    min;
    int	    max;
{
    int i;

    sscanf(s,"%d",&i);
    return i = Limit(i,min,max);
}

void SetParam(sp,pp,n,v,concat)
    State*  sp;
    Params* pp;
    char*   n;
    char*   v;
    bool    concat;
{
    if (!strcmp(n,"angle")) pp->angle = ScanInt(v,0,360);
    else if (!strcmp(n,"aspect")) {
	sscanf(v,"%f",&pp->aspect);
	if (pp->aspect<MinAspect) pp->aspect = MinAspect;
    }
    else if (!strcmp(n,"count")) pp->count = ScanInt(v,0,MaxNum);
    else if (!strcmp(n,"fonts")) strcpy(pp->fonts,v);
    else if (!strcmp(n,"lwidth")) pp->lwidth = ScanInt(v,0,_DisplayWidth);
    else if (!strcmp(n,"maxshift")) pp->maxshift = ScanInt(v,0,_DisplayWidth);
    else if (!strcmp(n,"nchar")) pp->nchar = ScanInt(v,0,MaxStr);
    else if (!strcmp(n,"nvert")) pp->nvert = ScanInt(v,3,MaxVert);
    else if (!strcmp(n,"nwin")) pp->nwin = ScanInt(v,0,MaxNum);
    else if (!strcmp(n,"offset")) sscanf(v,"%f",&pp->offset);
    else if (!strcmp(n,"opts")) {
	if (concat) {
	    if (strlen(pp->opts)&&strlen(v)) strcat(pp->opts,",");
	    strcat(pp->opts,v);
	}
	else strcpy(pp->opts,v);
    }
    else if (!strcmp(n,"outfile")) strcpy(pp->outfile,v);
    else if (!strcmp(n,"ptsize")) pp->ptsize = ScanInt(v,0,_DisplayHeight);
    else if (!strcmp(n,"size")) pp->size = ScanInt(v,0,_DisplayWidth);
    else if (!strcmp(n,"tag")) strcpy(pp->tag,v);
    else if (!strcmp(n,"timegoal")) pp->timegoal = ScanInt(v,0,MaxNum);
    else if (!strcmp(n,"winsize")) pp->winsize = ScanInt(v,1,_DisplayHeight);
}

void ParseParams(sp,cp,pp,print)
    State*	sp;
    Cmd*	cp;
    Params*	pp;
    bool	print;
{
    int	    i;
    char*   t = nil;
    char*   v;

    if (cp->index!=NilIndex) {
	for (
	    i=0;
	    (i<MaxPositional)&&OpNames[(int)cp->index][1+i]&&
		(t=GetToken(sp,false))&&!strchr(t,'=');
	    i++
	) {
	    SetParam(sp,pp,OpNames[(int)cp->index][1+i],t,true);
	}
    }
    do if (t&&(v=strchr(t,'='))) {
	*v++ = '\0';
	SetParam(sp,pp,t,v,false);
	if (print) PrintParam(Out(sp,cp),pp,"  ",t,"");
	*--v = '=';
    } while (t=GetToken(sp,false));
    if (print) {
	putc('\n',Out(sp,cp));
	fflush(Out(sp,cp));
    }
    if (strcmp(sp->fonts,pp->fonts)) SetFonts(sp,pp);
    if (strcmp(sp->outfile,pp->outfile)) SetOutfile(sp,pp);
    if (sp->winsize!=pp->winsize) SetWinsize(sp,pp);
    if (ParseOpts(pp->opts,&sp->supopts,&cp->o)!=Succeeded) {
	cp->result = OptNotSupported;
    }
}

void ParseCmd(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    cp->result = sp->supcmds[(int)cp->index]?Succeeded:CmdNotSupported;
    ParseParams(sp,cp,&cp->p,false);
    cp->func = cp->o.invert?GXinvert:GXcopy;
    sp->previndex = cp->index;
    CopyParams(&sp->prevparams,&cp->p);
    if (cp->o.setdefaults) CopyParams(&sp->defaults,&cp->p);
}

void PushInput(sp,fn)
    State*  sp;
    char*   fn;
{
    if ((sp->indepth<MaxIndepth)&&!(sp->infds[++sp->indepth]=fopen(fn,"r"))) {
	sp->indepth--;
    }
}

bool PopInput(sp)
    State*  sp;
{
    bool nonempty = (bool)(sp->indepth>0);

    if (nonempty) sp->indepth--;
    return nonempty;
}

void PrintComment(sp,cp,t)
    State*  sp; 
    Cmd*    cp;
    char*   t;
{
    char* te;

    if (sp->strinput) {
	te = t+strlen(t);
 	if (te<sp->ts.end) *te = ' ';
	fprintf(Out(sp,cp),"%s\n",t);
    }
    else {
	do fprintf(Out(sp,cp),"%s ",t); while (t=GetToken(sp,false));
	putc('\n',Out(sp,cp));
    }
}

bool GetLine(sp)
    State*  sp;
{
    char* cp;

    if (sp->scriptindex>=0) {
	strcpy(sp->instr,Script[sp->scriptindex]);
	if (!Script[++sp->scriptindex]) sp->scriptindex = -1;
	sp->strinput = true;
    }
    else if (sp->argindex>=sp->argc) {
	while (!fgets(sp->instr,MaxStr,sp->infds[sp->indepth])) {
	    if (!PopInput(sp)) return false;
	}
	if (cp=strchr(sp->instr,'\n')) *cp='\0';
	sp->strinput = true;
    }
    else sp->strinput = false;
    return true;
}

bool GetCmd(sp,cp)
    State*  sp;
    Cmd*    cp;
{
    char*   t;
    int     tlen;

    while (true) {
	cp->index = NilIndex;
	if (!GetLine(sp)) return false;
	if (t=GetToken(sp,true)) {
	    if (*t=='-') t++;
	    if (*t=='@') {
		t++;
		cp->o.silent = true;
	    }
	    else cp->o.silent = false;
	    tlen = strlen(t);
	    if (!strncmp(t,"config",tlen)) {
		fprintf(Out(sp,cp),"config\n");
		PrintConfig(Out(sp,cp),sp);
		putc('\n',Out(sp,cp));
	    }
	    else if (!strncmp(t,"defaults",tlen)) {
		fprintf(Out(sp,cp),"defaults");
		ParseParams(sp,cp,&sp->defaults,true);
		PrintParams(stderr,sp,&sp->defaults);
		putc('\n',stderr);
	    }
	    else if (!strncmp(t,"help",tlen)) {
		PrintHelp(Out(sp,cp));
		putc('\n',stderr);
	    }
	    else if (!strncmp(t,"init",tlen)) {
		fprintf(Out(sp,cp),"init\n");
		InitParams(sp,&sp->defaults);
		PrintParams(stderr,sp,&sp->defaults);
		putc('\n',stderr);
	    }
	    else if (!strncmp(t,"quit",tlen)) {
		return false;
	    }
	    else if (!strncmp(t,"script",tlen)) {
		fprintf(Out(sp,cp),"script");
		if (t=GetToken(sp,false)) fprintf(Out(sp,cp)," %s",t);
		putc('\n',Out(sp,cp));
		if (t) PushInput(sp,t);
		else sp->scriptindex = 0;
	    }
	    else if (*t=='#') {
		PrintComment(sp,cp,t);
	    }
	    else if ((*t=='!')&&((cp->index=sp->previndex)!=NilIndex)) {
		CopyParams(&cp->p,&sp->prevparams);
		ParseCmd(sp,cp);
		return true;
	    }
	    else if ((cp->index=GetOpIndex(t))!=NilIndex) {
		CopyParams(&cp->p,&sp->defaults);
		ParseCmd(sp,cp);
		return true;
	    }
	}
	else putchar('\n');
    }
}
