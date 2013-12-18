#include "cchdr.h"
#include "xrefs.h"

#define t_hunk       0x3e8
#define t_reloc      0x3e9
#define t_ext        0x3ed
#define t_end        0x3ea

#define ext_def      1
#define ext_abs      2
#define ext_ref      129
#define ext_common   130

FILE *objstream;
char *objfilename;

int ncoderelocs, ndatarelocs, obj_stringpos, obj_symcount;
ExtRef *obj_symlist;
CodeXref *codexrefs;
DataXref *dataxrefs;

int ncodewords;
int codesize;
int32 codemarker;
int32 datamarker;

void objword(w)
int32 w;
{ fwrite(&w, 1, 4, objstream);
}

void obj_outcode( buff, flag, nwords)
int32 *buff;
int32 *flag;
int32 nwords;
{   int32 i;

    for( i = 0; i < nwords; i++)
    {
/*        int32 w = tom68sex( buff[i], flag[i], & 0xff000000 ); */

        objword(buff[i]);
    }
    ncodewords += nwords;
}

void obj_codewrite()
{
    int32 i = 0;

    while(  (codep>>2) - CODEVECSEGSIZE*i > CODEVECSEGSIZE)
        obj_outcode( &(*codeandflagvec[i])[0],
                        &(*codeandflagvec[i])[CODEVECSEGSIZE],
                        CODEVECSEGSIZE),
        i++;
    obj_outcode( &(*codeandflagvec[i])[0],
                    &(*codeandflagvec[i])[CODEVECSEGSIZE],
                    (codep>>2) - CODEVECSEGSIZE*i);
}

void obj_symbdata(type, n)
int32 type;
char *n;
{
    int32 nl = strlen(n);
    int32 i;
    int32 w = type;

    if( nl > 7 ) nl = 7;

    for( i = 0; i < 7; i++)
    {
        w = (w<<8) + ((i < nl)? *n++: ' ');
        if( (i%4) == 2 )
            objword(w) , w = 0;
    }
}

#define ensure_ext_started(type) if( !hunk_started ) \
                          objword(type), \
                           hunk_started = 1
bool dumpexterns(hunk_started,type,objtype,offset)
bool hunk_started;
int32 type;
int32 objtype;
int32 offset;
{
    ExtRef *x;

    for( x = obj_symlist; x != NULL; x = x->extcdr )
    {
        if( (x->extflags & xr_defext) && (x->extflags & type ) )
        {
            ensure_ext_started(objtype);
            obj_symbdata( ext_def, _symname(x->extsym) );
            objword(x->extoffset + offset);
        }
    }
    return(hunk_started);
}

void addlong(offset,value,marker)
int32 offset;
int32 value;
int32 marker;
{
    if( value != 0 )
    {   int32 filep = ftell(objstream);
        int32 w;

        fseek(objstream, marker+offset,SEEK_SET);
        fread(&w, 4, 1, objstream);
        fseek(objstream, marker+offset,SEEK_SET);
        value += w;
        fwrite(&value, 4, 1, objstream);
        fseek( objstream, filep, SEEK_SET);
    }
}

void patchlong(offset,value,marker)
int32 offset;
int32 value;
int32 marker;
{
    if( value != 0 )
    {   int32 filep = ftell(objstream);
        int32 w;

        fseek(objstream, marker+offset,SEEK_SET);
        fwrite(&value, 4, 1, objstream);
        fseek( objstream, filep, SEEK_SET);
    }
}

void patchword(offset,value,marker)
int32 offset;
int16 value;
int32 marker;
{
    if(value != 0)
    {   int32 filep = ftell(objstream);
        int16 w;
        int32 temp;

        fseek(objstream, marker+offset, SEEK_SET);
        fwrite(&value, 2, 1, objstream);
        fseek( objstream, filep, SEEK_SET);
    }
}

bool dumpcodexrefs(hunk_started)
bool hunk_started;
{
    CodeXref *x;
    int32 off;
    Symstr *s;
    ExtRef *xr;

    for( x = codexrefs; x != NULL; x = x->codexrcdr)
    {
        off = (x->codexroff & 0x00ffffff);
        s   = x->codexrsym;
        xr  = (ExtRef *)(_symext(s));

        switch( x->codexroff & 0xff000000 )
        {
        case X_absreloc:
            if( !(xr->extflags & (xr_defloc | xr_defext)) )
            {
/* This entry handles references to symbols which are defined externally */
/* Locally defined symbols are handled in dumpcodehunkrelocs             */
                ensure_ext_started(t_ext);
                obj_symbdata(ext_ref, _symname(s) );
                objword(1);
                objword( off );
            }
            break;

        case X_PCreloc:
            if( xr->extflags & (xr_defloc | xr_defext) )
            {           /* A 16bit PC reloc which I may as well do now */
                patchword(off, xr->extoffset - off+x->codexrcode, codemarker);
            }
            else        /* An external 16-bit PC reloc */
            {
                syserr("16-bit PC reloc invalid\n");
#ifdef never
                ensure_ext_started(t_ext);
                obj_symbdata(ext_ref16, _symname(s) );
                objword(1);
                objword( off );
#endif
            }
            break;

        default:
            syserr("Bad code relocation\n");
        }
    }
    return(hunk_started);
}

static int32 dumpcodehunkrelocs(nrelocs)
int32 nrelocs;
{
    CodeXref *x;
    bool hunk_started = 0;
    int32 off;
    Symstr *s;
    ExtRef *xr;

    for( x = codexrefs; x != NULL; x = x->codexrcdr )
    {
        off = x->codexroff &0x00ffffff;
        s = x->codexrsym;
        obj_symref(s, xr_refcode+xr_refdata, 0);
        xr = _symext(s);

        if( (x->codexroff & 0xff000000) == X_absreloc &&
            (xr->extflags & (xr_defext | xr_defloc))    )
        {               /* Reference to static or locally defined symbol */
            if( !nrelocs )
            {   objword(t_reloc);
                hunk_started = 1;
                objword(0);
            }
            nrelocs++;
            objword( off );
/* The offset from the symbol is in codexrcode now we have to add  */
/* the offset from the section base to the symbol                  */
            patchlong( off,
                       xr->extoffset + x->codexrcode +
                          ((xr->extflags&xr_data)? codesize:0),
                       codemarker);
        }
    }               
    return( nrelocs );
}

bool dumpdataxrefs(hunk_started)
bool hunk_started;
{
    DataXref *x;
    int32 off;
    Symstr *s;
    ExtRef *xr;

    for( x = dataxrefs; x != NULL; x = x->dataxrcdr)
    {
        off = (x->dataxroff & 0x00ffffff);
        s   = x->dataxrsym;
        xr = (ExtRef *)(_symext(s));

        if( !(xr->extflags & (xr_defext | xr_defloc)) )
        {            /* Defined externally */
            ensure_ext_started(t_ext);
            obj_symbdata(ext_ref, _symname(s) );
            objword(1);
            objword( off + codesize );
        }
    }
    return( hunk_started );
}

int32 dumpdatahunkrelocs(nrelocs)
int32 nrelocs;
{
    DataXref *x;
    int32 off;
    Symstr *s;
    ExtRef *xr;

    for( x = dataxrefs; x != NULL; x = x->dataxrcdr)
    {             /* Defined locally */
        off = (x->dataxroff & 0x00ffffff);
        s   = x->dataxrsym;
        obj_symref(s, xr_refcode+xr_refdata, 0);
        xr  = (ExtRef *)(_symext(s));

        if( xr->extflags & (xr_defext | xr_defloc) )
        {
            if( !nrelocs )
            {   objword(t_reloc);
                objword( 0 );
            }
            nrelocs++;
            objword( off+codesize );
            addlong(off,
                      xr->extoffset +((xr->extflags&xr_data)? codesize: 0),
                      datamarker);
        }
    }
    return( nrelocs );
}

static void dumpxs()
{   bool hunk_started;
    int32 nrelocs = 0;
    int32 relocpos = ftell( objstream );

    nrelocs = dumpdatahunkrelocs(nrelocs);
    nrelocs = dumpcodehunkrelocs(nrelocs);
    if ( nrelocs ) patchlong( 4, nrelocs, relocpos );

    hunk_started = dumpexterns(0,xr_code,t_ext,0);
    hunk_started = dumpcodexrefs(hunk_started);
    hunk_started = dumpexterns(hunk_started,xr_data,t_ext,codesize);
    hunk_started = dumpdataxrefs(hunk_started);
    if( hunk_started ) objword( 0 );
}

int32 dumpdata()
{
  DataInit *p;
  int32 databytes = 0;
  int32 zero = 0;

  datamarker = ftell(objstream);

  if( !datainitp ) return(0);

  for (p = datainitp; p != 0; p = p->datacdr)
  { int rpt, sort, len, val;
    Symstr *sv; FloatCon *fc;
    rpt = p->rpt, sort = p->sort, len = p->len, val = p->val;
    switch (sort)
    {   case LIT_LABEL:   /* name only present for c.armasm */
            break;
        default:  syserr("internal error in obj_gendata(%d)", sort);
        case LIT_BBBB:    /* the next 4 are the same as LIT_NUMBER except   */
        case LIT_HH:      /* for (as yet unsupported) cross compilation.    */
        case LIT_BBH:
        case LIT_HBB:
        case LIT_NUMBER:
            if (len != 4) syserr("obj_data len=%d", len);
            databytes+=len*rpt;
            /* beware: sex dependent... */
            while (rpt-- != 0) fwrite(&val, len, 1, objstream);
            break;
        case LIT_FPNUM:
            fc = (FloatCon *)val;
            databytes+=len*rpt;
            /* do we need 'len' when the length is in fc->floatlen?? */
            if (len == 4 || len == 8);
            else syserr("internal error %dEL%d'%s'\n", rpt, len, fc->floatstr);
            while (rpt-- != 0)
              fwrite(&(fc->floatbin), len, 1, objstream);
            break;
        case LIT_ADCON:              /* (possibly external) name + offset */
            sv = (Symstr *)len;  /* this relocation is also in dataxrefs */
            databytes+=4*rpt;
            /* beware: sex dependent... */
            while (rpt-- != 0) fwrite(&val, 4, 1, objstream);
            break;
    }
  }

  if((databytes & 3) != 0) fwrite(&zero, 4-(databytes&3),1,objstream);
  databytes = (databytes+3)&-4;

  return(databytes);
}

#ifdef __STDC__
int32 obj_symref(Symstr *s, int32 flags, int32 loc)
#else
int32 obj_symref(s, flags, loc)
Symstr *s;
int32 flags;
int32 loc;
#endif
{
    ExtRef *x;

    if( (x = _symext(s)) == 0 )     /* if not already defined */
    {
#ifdef never
        x = obj_symlist = _symext(s) =
                global_list5(obj_symlist, s, obj_symcount++, 0, 0);
#endif
        x = GlobAlloc(sizeof(ExtRef));
        x->extcdr = obj_symlist,
          x->extsym = s,
          x->extindex = obj_symcount++,
          x->extflags = 0,
          x->extoffset = 0;
        obj_symlist = _symext(s) = x;
    }

    x ->extflags |= flags;

    if( flags & xr_defloc+xr_defext) x->extoffset = loc;
    return( x->extflags & xr_code )? x->extoffset: -1;
}

void obj_init()
{
    obj_symcount = 0;
    obj_symlist  = NULL;
    dataxrefs    = NULL;
    codexrefs    = NULL;
    ncodewords   = 0;
}

void obj_header()
{
    objword(t_hunk);
    objword(0);               /* filled in later */
    codemarker = ftell(objstream);
}

void obj_trailer()
{
   codesize = ncodewords << 2;
   ncodewords += (dumpdata() >> 2)+2;
   objword(0);    /* end of GIT */
   objword(0);    /* HRG */
   patchlong( -4, ncodewords, codemarker);
   patchlong( 0, ncodewords, codemarker);

   dumpxs();
   objword(t_end);
}
