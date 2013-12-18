/*
 * Prime fileio.c for MicroGnuEmacs by Robert A. Larson
 *	system dependent file io routines
 *
 * Prime keeps the parity bit of every character set, as does mg 2a for primos.
 */
#include <errd.ins.cc>
#include <keys.ins.cc>
#include "def.h"

static FILE    *ffp;
char *index(), *rindex();
fortran void cnam$$(), at$(), at$abs(), at$any(), at$hom(), clo$fu();
fortran void gpath$(), dir$se(), cv$fdv();
fortran long srsfx$();

/*
 * Open a file for reading.
 */
ffropen(fn)
char   *fn;
{
	if ((ffp=fopen(fn, "r")) == NULL)
		return (FIOFNF);
	return (FIOSUC);
}

/*
 * Open a file for writing.
 * Return TRUE if all is well, and
 * FALSE on error (cannot create).
 */
ffwopen(fn)
char	*fn;
{
	short unit, type, sfu, code;
	struct {short len; char data[128];} fnb;
	register char *cp = &fnb.data[0];
	struct {short len; char data[32];} basename;

	/* most of this is to get a SAM rather than DAM file ... */
	(void) strcpy(fnb.data, fn);
	fnb.len = strlen(fnb.data);
	(void) srsfx$((short)(k$writ+k$getu+k$nsam), fnb, unit, type,
	    (short)0, (short)0, basename, sfu, code);
#ifdef OPEN_BUG
	if(code==0) {
	    clo$fu(unit, code);
	    fnb.data[fnb.len] = '\0';
	    if((ffp=fopen(fnb.data, "w"))!=NULL) return FIOSUC;
	}
	ewprintf("Cannot open file for writing");
	return FIOERR;
#else
	if (code != 0 || (ffp=fdopen(open("", -2, unit), "w")) == NULL) {
		ewprintf("Cannot open file for writing");
		if(code==0) clo$fu(unit, code);
		return (FIOERR);
	}
	return (FIOSUC);
#endif
}

/*
 * Close a file.
 * Should look at the status.
 */
ffclose()
{
	fclose(ffp);
	return (FIOSUC);
}

/*
 * Write a buffer to the already
 * opened file. bp points to the
 * buffer. Return the status.
 * Check only at the newline and
 * end of buffer.
 */
ffputbuf(bp)
BUFFER *bp;
{
    register char *cp;
    register char *cpend;
    register LINE *lp;
    register LINE *lpend;

    lpend = bp->b_linep;
    lp = lforw(lpend);
    do {
	cp = &ltext(lp)[0];		/* begining of line	*/
	cpend = &cp[llength(lp)];	/* end of line		*/
	while(cp != cpend) {
	    putc(*cp, ffp);
	    cp++;	/* putc may evalualte arguments more than once */
	}
	lp = lforw(lp);
	if(lp == lpend) break;		/* no implied newline on last line */
	putc('\n', ffp);
    } while(!ferror(ffp));
    if(ferror(ffp)) {
	ewprintf("Write I/O error");
	return FIOERR;
    }
    return FIOSUC;
}

/*
 * Read a line from a file, and store the bytes
 * in the supplied buffer. Stop on end of file or end of
 * line.  When FIOEOF is returned, there is a valid line
 * of data without the normally implied \n.
 */
ffgetline(buf, nbuf, nbytes)
register char	*buf;
register int	nbuf;
register int	*nbytes;
{
	register int	c;
	register int	i;

	i = 0;
	while((c = getc(ffp))!=EOF && c!='\n') {
		buf[i++] = c;
		if (i >= nbuf) return FIOLONG;
	}
	if (c == EOF  && ferror(ffp) != FALSE) {
		ewprintf("File read error");
		return FIOERR;
	}
	*nbytes = i;
	return c==EOF ? FIOEOF : FIOSUC;
}

#ifndef NO_BACKUP
/*
 * Rename the file "fname" into a backup copy.
 */
fbackupfile(fname)
char   *fname;
{
    struct {short len; char data[128];} back;
    struct {short len; char data[32];} ent;
    register char *cp;
    short code = 0, i;

    strcpy(back.data, fname);
    strcat(back.data, ".BAK");
    (void) delete(back.data);
    if((cp = rindex(back.data, '>'))!=NULL) {
	if(back.data[0]==('<') && index(back.data, '>')==cp) {
	    strncpy(ent.data, back.data+1, ent.len = (cp - back.data) - 1);
	    at$abs((short)k$setc, ent, (short)0, code);
	} else {
	    back.len = (cp - back.data);
	    if(index(back.data, '>') == cp)
		 at$any((short)k$setc, back, code);
	    else at$((short)k$setc, back, code);
	}
	if(code) return FALSE;
	cp++;
	/* cnam$$ needs word aligned strings */
	strncpy(ent.data, cp, back.len = strlen(cp));
	cp = ent.data;
    } else back.len = strlen(cp = back.data);
    cnam$$((char [])cp, (short)(back.len - 4), (char [])cp, back.len, code);
    at$hom(i);
    return code == 0;
}
#endif

/*
 * The string "fn" is a file name.  Prepend the directory name if
 * it's relative to the current directory.
 */
#ifndef NO_DIR
extern char *wdir;
#endif

char *adjustname(fn)
register char  *fn;
{
    static char fnb[NFILEN];
    register char *cp = fnb;
    register char *cp2;

#ifndef NO_DIR
    if((fn[0] == '*' && fn[1] == '>') || index(fn, '>')==NULL) {
	cp2 = wdir;
	while(*cp2) {
	    *cp = *cp2;
	    cp++;
	    cp2++;
	}
	*cp++ = '>';
	if(fn[1]=='>') fn+=2;
    }
#endif
    while(*fn) {
	*cp = ISUPPER(*fn) ? TOLOWER(*fn) : *fn;
	cp++;
	fn++;
    }
    *cp = '\0';
    return fnb;
}

#ifndef NO_STARTUP
char *startupfile(suffix)
char *suffix;
{
	short code, len;
	static char startname[128];
	register char *cp;

	gpath$((short)k$inia, (short)-3, (char [])startname,
	      (short)(128 - 5), len, code);
	if (code==0) {
	    strcpy(startname + len, ">.MG");
	    if(suffix) {
		startname[len+4] = '.';
		strcpy(startname + len + 5, suffix);
	    }
	    if(access(startname, 4)==0)
		return startname;
	}
	strcpy(startname, "MG*>.MG");
	if(suffix) {
	    startname[7] = '.';
	    strcpy(startname+8, suffix);
	}
	if(access(startname, 4)==0)
	    return startname;
	return (char *)NULL;
}
#endif

/* compare file names */
fncmp(fn1, fn2)
register char *fn1, *fn2;
{
    /* ignore disk name if on one but not the other */
    if(*fn1 != *fn2) {
	if(*fn1 == '<') {
	    fn1 = index(fn1, '>');
	    if(fn1 == NULL) return -1;
	    fn1++;
	} else if(*fn2 == '<') {
	    fn2 = index(fn2, '>');
	    if(fn2 == NULL) return -1;
	    fn2++;
	} else return -1;
    } else fn1++, fn2++;
    /* compare ignoring case */
    while(*fn1) {
	if((*fn1 != *fn2) && (!ISUPPER(*fn1) || (TOLOWER(*fn1) == *fn2))
			  && (!ISUPPER(*fn2) || (TOLOWER(*fn2) == *fn1)))
	    return -1;
	fn1++;
	fn2++;
    }
    return *fn2;
}

#ifndef NO_DIRED
#include "kbd.h"

BUFFER *dired_(dirname)
char *dirname;
{
    register BUFFER *bp;
    LINE *lp, *blp;
    BUFFER *findbuffer();
    short dirunit, j, code, type, init, counts[4], i, nent;
    struct {short len; char dat[128];} dir;
    struct ent {short len; char dat[32];} base;
    static struct ent wild[1] = {{2, {'@','@'}}};
    static struct {
	short vers;
	struct ent *wp;
	short count;
	struct {unsigned dirs:1, seg_dirs:1, files:1, acats:1, rbf:1, spare:11;} desired;
	long mb, ma, bb, ba, cb, ca, ab, aa;
    } sel = {
	1,
	&wild[0],
	1,
	{1, 1, 1, 0, 0, 0},
	0, 0, 0, 0, 0, 0, 0, 0,
    }, *s = &sel;
#define MAXSE 16
    struct {
	struct {unsigned type:8, length:8;} ecw;
	struct ent name;
	struct {unsigned spare:5, odel:1, owrite:1, oread:1, delp:1,
	    spare2:4, nodel:1, nowrite:1, noread:1;} prot;
	struct {unsigned lrat:1, dumped:1, dos:1, spec:1, rwl: 2,
	    spare:2, type:8;} info;
	long dtm;
	short nondefault, logical_type, trunc;
	long dtb, dtc, dta;
    } ret[MAXSE], *r = &ret[0];
    static char *types[] = {"SAM", "DAM", "SEGSAM", "SEGDAM", "UFD",
	"ACAT", "CAM"};
    static char *rwlock[] = {"sys", "excl", "updt", "none"};

    if((dirname = adjustname(dirname)) == NULL) {
	ewprintf("Bad directory name");
	return NULL;
    }
    (void) strncpy(dir.dat, dirname, dir.len = strlen(dirname));
    if(dir.dat[dir.len-1]=='>') dir.len--;
    else strcat(dirname, ">");
    if((bp = findbuffer(dirname)) == NULL) {
	ewprintf("Could not create directory buffer");
	return NULL;
    }
    if(bclear(bp) != TRUE) return NULL;
    (void) srsfx$((short)(k$read+k$getu), dir, dirunit, type,
	(short)0, (short)0, base, j, code);
    if(code!=0) {
	ewprintf("Could not open directory '%s'", dirname);
	return NULL;
    }
    if(type<2 || type > 4) {
	clo$fu(dirunit, code);
	ewprintf("Not a directory '%s'", dirname);
	return NULL;
    }
    for(init = 0x8000; ; init = 0) {
	dir$se(dirunit, type, init, s, r, (short)MAXSE,
	    (short)((sizeof ret[0])/2), nent, (short [])counts, (short)4, code);
	if(code!=0 && (code!=e$eof || nent==0)) break;
	for(i=0; i < nent; i++) {
	    cv$fdv(ret[i].dtm, j, base);
	    for(j=0; j<ret[i].name.len; j++)
		if(ISUPPER(ret[i].name.dat[j]))
		    ret[i].name.dat[j] = TOLOWER(ret[i].name.dat[j]);
#define D_FILEPOS 36
	    if((lp = lalloc(D_FILEPOS + 1 + ret[i].name.len)) != NULL) {
		sprintf(lp->l_text, "  %-6s  %-4s  %.18s  %.*s",
		    types[ret[i].info.type], rwlock[ret[i].info.rwl],
		    base.dat, ret[i].name.len, ret[i].name.dat);
		llength(lp)--;
		for(blp=bp->b_linep->l_fp; blp!=bp->b_linep; blp=blp->l_fp) {
		    j = strncmp(ret[i].name.dat, &blp->l_text[D_FILEPOS],
			ret[i].name.len);
		    if(j < 0 || (j==0 && ret[i].name.len <=
			llength(blp)-D_FILEPOS)) break;
		}
		lp->l_fp = blp;
		lp->l_bp = blp->l_bp;
		blp->l_bp = lp;
		lp->l_bp->l_fp = lp;
	    } else {
		clo$fu(dirunit, code);
		return NULL;
	    }
	}
    }
    if(code!=e$eof) ewprintf("Directory read error %d", code);
    clo$fu(dirunit, code);
    bp->b_dotp = lforw(bp->b_linep);
    bp->b_doto = 0;
    bp->b_markp = NULL;
    strncpy(bp->b_fname, dirname, NFILEN);
    if((bp->b_modes[0] = name_mode("dired")) == NULL) {
	bp->b_modes[0] = &map_table[0];
	ewprintf("Could not find mode dired");
	return NULL;
    }
    bp->b_nmodes = 0;
    return bp;
}

d_makename(lp, fn)
register LINE *lp;
register char *fn;
{
    register char *cp;

    if(llength(lp) <= D_FILEPOS) return ABORT;
    (VOID) strcpy(fn, curbp->b_fname);
    cp = fn + strlen(fn);
    bcopy(&lp->l_text[D_FILEPOS], cp, llength(lp) - D_FILEPOS);
    cp[llength(lp) - D_FILEPOS] = '\0';
    return strncmp(&lp->l_text[2], "UFD", 3)==0 ||
	strncmp(&lp->l_text[2], "SEG", 3)==0;
}

rename(old, new)
char *old, *new;
{
    struct {short len; char dat[128];} f;
    struct {short len; char dat[32];} oldent;
    char *cp;
    short code;

    old = adjustname(old);
    cp = rindex(old, '>');
    strncpy(f.dat, old, f.len = cp - old);
    at$((short)k$setc, f, code);
    if(code!=0) return -1;
    cp++;
    strncpy(oldent.dat, cp, oldent.len = strlen(cp));
    strncpy(f.dat, new, f.len = strlen(new));
    cnam$$((char [])oldent.dat, oldent.len, (char [])f.dat, f.len, code);
    if(code!=0) {
	at$hom(code);
	return -1;
    }
    at$hom(code);
    return 0;
}
#endif

#ifndef NO_DIR
char *getwd(cwd)
char *cwd;
{
    char homedir[128];	     /* cwd may not be word alligned */
    short len, code;
    register char *cp1, *cp2;

    gpath$((short)k$homa, (short)-2, (char [])homedir, (short)128, len, code);
    if(code!=0) return NULL;
    cp1 = cwd;
    cp2 = homedir;
    while(len--) {
	*cp1 = ISUPPER(*cp2) ? TOLOWER(*cp2) : *cp2;
	cp1++;
	cp2++;
    }
    *cp1 = '\0';
    return cwd;
}
#endif
