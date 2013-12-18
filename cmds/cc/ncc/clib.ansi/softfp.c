/* Software implementation of floating point arithmetic                  */
/* Copyright Codemist Ltd, 1987.                                         */

/* Compiling this code is expected to lead to lots of warning messages   */
/* about argument passing type error. This is because I have to deal     */
/* with the interface between FP values and representations.             */

/* This code assumes various NASTY things about the way in which FP      */
/* arguments & results are processed. it is ONLY for use with the        */
/* SOFTWARE_FLOATING_POINT option in the Norcroft C compiler.            */

#include <signal.h>

extern float  _d2f(double a);
extern double _f2d(int a);
extern double _dadd(double a, double b),
              _dsub(double a, double b),
              _dmul(double a, double b),
              _ddiv(double a, double b),
              _dneg(double a),
              _dflt(int n),
              _dfltu(unsigned int u);
extern int    _dgr(double a, double b),
              _dgeq(double a, double b),
              _dls(double a, double b),
              _dleq(double a, double b),
              _deq(double a, double b),
              _dneq(double a, double b),
              _dfix(double a);
extern unsigned int _dfixu(double a);


/* I perform single precision arithmetic by widening to double,          */
/* performing the operation and narrowing back.                          */
/* This is slow but easy to code!                                        */

float _fadd(int a,int b)
{
    return _d2f(_dadd(_f2d(a), _f2d(b)));
}

float _fsub(int a, int b)
{
    return _d2f(_dsub(_f2d(a), _f2d(b)));
}

float _fmul(int a, int b)
{
    return _d2f(_dmul(_f2d(a), _f2d(b)));
}

float _fdiv(int a, int b)
{
    return _d2f(_ddiv(_f2d(a), _f2d(b)));
}

int _fneg(int a)        /* type is really float, but here who cares!     */
{
    return a==0 ? 0 : a ^ 0x80000000;
}


int _fgr(int a, int b)
{
    return _dgr(_f2d(a), _f2d(b));
}

int _fgeq(int a, int b)
{
    return !_dgr(_f2d(b), _f2d(a));
}

int _fls(int a, int b)
{
    return _dgr(_f2d(b), _f2d(a));
}

int _fleq(int a, int b)
{
    return !_dgr(_f2d(a), _f2d(b));
}

int _feq(int a, int b)
{
    return _deq(_f2d(a), _f2d(b));
}

int _fneq(int a, int b)
{
    return !_deq(_f2d(a), _f2d(b));
}

float _fflt(int a)
{
    return _d2f(_dflt(a));
}

float _ffltu(unsigned int a)
{
    return _d2f(_dfltu(a));
}


int _ffix(int a)
{
    return _dfix(_f2d(a));
}

unsigned int _ffixu(int a)
{
    return _dfixu(_f2d(a));
}


/* Now the functions that deal with double precision args. They can have */
/* funny types for their formals now since all references to them have   */
/* already been processed under the shelter of proper extern             */
/* declarations that tell the official story re types.                   */

#ifdef IEEE

int _d2f(int hi, int lo)
{   
    int x = (hi>>20) & 0x7ff;
    int m = hi & 0x000fffff;
    if (x==0)                   /* value is zero - treat specially       */
        return 0;               /* OK */
    else if (x==0x7ff)          /* value is 'infinity'.                  */
        return raise(SIGFPE);
    m = (m << 3) | ((lo >> 29) & 0x7);
    lo = lo << 3;
    if (lo < 0 && (lo!=0x80000000 || (m & 1)!=0))
        if ((m += 1) == 0x00800000)     /* round up & renormalize        */
        {   m = 0;
            x += 1;
        }
    x = x - 0x3ff + 0x7f;
    if (x >= 0xff) return raise(SIGFPE);
    else if (x <= 0)
        return (hi & 0x80000000) | (0 << 23);   /* preserve sign */
    return (hi & 0x80000000) | (x << 23) | m;
}

double _f2d(unsigned int e0)
{
    int d[2];
    int x = (e0>>23) & 0xff;
    int s = e0 & 0x80000000;
    if (x==0)                   /* value is zero - preserve sign, why?   */
        d[0] = s, d[1] = 0;
    else if (x==0xff)           /* value is 'infinity' - preserve sign.  */
        d[0] = s | 0x7ff00000, d[1] = 0;
    else
        d[0] = s | ((x - 0x7f + 0x3ff) << 20) | ((e0 & 0x7fffff) >> 3),
        d[1] = e0 << 29;
    return *(double *)d;
}

#endif
#ifdef IBMFLOAT

int _d2f(int a, int b)      /* value is really a float, arg a double     */
{
    return a;
}

double _f2d(int a)
{   int aa[2];
    aa[0] = a;
    aa[1] = 0;
    return *(double *)aa;
}

#endif

/* Coded without concern about speed, and so quite dreadfully slow.      */
/* Also coded without concern about code density, so rather repetitive   */
/* and bulky. Memory is supposed to be cheap these days.                 */


#ifdef IEEE

int flt_sum(unsigned int *a, unsigned int bh, unsigned int bl,
                             unsigned int ch, unsigned int cl)
{
    unsigned int guard, w;
    int bx, cx;
    int shift;
    bx = (bh >> 20) & 0x7ff;
    cx = (ch >> 20) & 0x7ff;
    shift = bx - cx;
    if (shift < -54 || bx == 0)
    {   a[0] = ch;
        a[1] = cl;
        return 1;
    }
    if (shift > 54 || cx == 0)
    {   a[0] = bh;
        a[1] = bl;
        return 1;
    }
    bh = (bh & 0xfffff) | 0x100000;
    ch = (ch & 0xfffff) | 0x100000;
/* Now I need to align the operands                                      */
    if (shift > 0)
    {   int rshift;
        if (shift > 32)
        {   guard = cl;
            cl = ch;
            ch = 0;
            shift -= 32;
        }
        rshift = 32 - shift;
        if (guard == 0) guard = cl << rshift;
        else guard = (cl << rshift) | 1;
        cl = (ch << rshift) | (cl >> shift);
        ch = ch >> shift;
    }
    else if (shift < 0)
    {   int rshift;
        if (shift < -32)
        {   guard = bl;
            bl = bh;
            bh = 0;
            shift += 32;
        }
        shift = -shift;
        rshift = 32 - shift;
        if (guard == 0) guard = bl << rshift;
        else guard = (bl << rshift) | 1;
        bl = (bh << rshift) | (bl >> shift);
        bh = bh >> shift;
        bx = cx;
    }
    else guard = 0;
    w = (bl & 0xff) + (cl & 0xff);
    bl = (bl >> 8) + (cl >> 8) + (w >> 8);
    bh += ch + (bl >> 24);
    bl = (bl << 8) + (w & 0xff);
    if (bh & 0x00200000)
    {   guard = (bl << 31) | (guard >> 1);
        bl = (bh << 31) | (bl >> 1);
        bh = bh >> 1;
        bx += 1;
        if (bx >= 0x7ff) return 0;          /* Overflow case             */
    }
/* The magic rules about when and how to round are implemented here      */
#define _needcarry(g, l) (g & 0x80000000) && ((l & 1)!=0 || g!=0x80000000)
    if (_needcarry(guard, bl))
    {   if (bl == 0xffffffff)
        {   bl = 0;
            bh += 1;
            if (bh & 0x00200000)
            {   bh = 0x00100000;
                bx += 1;
                if (bx >= 0x7ff) return 0;      /* Overflow case         */
            }
        }
        else bl += 1;
    }
    a[0] = (bh & ~0x00100000) | (bx << 20);
    a[1] = bl;
    return 1;
}

int flt_difference(unsigned int *a, unsigned int bh, unsigned int bl,
                                    unsigned int ch, unsigned int cl)
{
    unsigned int bg=0, cg=0, w, resultsign;
    int bx, cx;
    int shift;
    bx = (bh >> 20) & 0x7ff;
    cx = (ch >> 20) & 0x7ff;
    shift = bx - cx;
    if (shift < -54 || bx == 0)
    {   a[0] = ch ^ 0x80000000;
        a[1] = cl;
        return 1;
    }
    if (shift > 54 || cx == 0)
    {   a[0] = bh;
        a[1] = bl;
        return 1;
    }
    bh = (bh & 0xfffff) | 0x100000;
    ch = (ch & 0xfffff) | 0x100000;
/* Now I need to align the operands                                      */
    if (shift > 0)
    {   int rshift;
        if (shift > 32)
        {   cg = cl;
            cl = ch;
            ch = 0;
            shift -= 32;
        }
        rshift = 32 - shift;
        if (cg == 0) cg = cl << rshift;
        else cg = (cl << rshift) | 1;
        cl = (ch << rshift) | (cl >> shift);
        ch = ch >> shift;
        bg = 0;
    }
    else if (shift < 0)
    {   int rshift;
        if (shift < -32)
        {   bg = bl;
            bl = bh;
            bh = 0;
            shift += 32;
        }
        shift = -shift;
        rshift = 32 - shift;
        if (bg == 0) bg = bl << rshift;
        else bg = (bl << rshift) | 1;
        bl = (bh << rshift) | (bl >> shift);
        bh = bh >> shift;
        cg = 0;
        bx = cx;
    }
    else bg = cg = 0;
/* Now for a subtraction, taking account of signs. Ugh.                  */
/* I rely on right shifts on signed types being arithmetic in struggles  */
/* to implement multiple precision arithmetic without a proper add-carry */
/* operator in my language.                                              */
    w = (bg & 0xff) - (cg & 0xff);
    bg = (bg >> 8) - (cg >> 8) + (((int)w) >> 8);
    cg = ((int) bg) >> 24;
    bg = (bg << 8) | (w & 0xff);
    w = (bl & 0xff) - (cl & 0xff) + cg;
    bl = (bl >> 8) - (cl >> 8) + (((int)w) >> 8);
    cg = ((int) bl) >> 24;
    bl = (bl << 8) | (w & 0xff);
    bh = bh - ch + cg;
/* Subtraction complete in 2s complement form.                           */
    if (bh & 0x80000000)  /* Sign of result must be negative             */
    {   if (bg == 0)
        {   if (bl == 0) bh = -bh;
            else
            {   bl = -bl;
                bh = ~bh;
            }
        }
        else
        {   bg = -bg;
            bl = ~bl;
            bh = ~bh;
        }
        resultsign = 0x80000000;
    }
    else resultsign = 0;
/* Subtraction now complete in sign & magnitude form                     */
    if (bh == 0 && bl == 0 && bg == 0)
    {   a[0] = a[1] = 0;           /* Result is absolutely zero          */
        return 1;
    }
/* Must have a normalized result.                                        */
    if ((bh & 0x00300000)==0)       /* need to renormalize?              */
    {   while (bh == 0)
        {   bh = bl >> 11;
            bl = (bl << 21) | (bg >> 11);
            bg = bg << 21;
            bx -= 21;
            if (bx <= 0)
            {   a[0] = a[1] = 0;
                return 1;
            }
        }
        while ((bh & 0x001fe000) == 0)
        {   bh = (bh << 8) | (bl >> 24);
            bl = (bl << 8) | (bg >> 24);
            bg = bg << 8;
            bx -= 8;
            if (bx <= 0)
            {   a[0] = a[1] = 0;
                return 1;
            }
        }
        while ((bh & 0x001c0000) == 0)
        {   bh = (bh << 3) | (bl >> 29);
            bl = (bl << 3) | (bg >> 29);
            bg = bg << 3;
            bx -= 3;
            if (bx <= 0)
            {   a[0] = a[1] = 0;
                return 1;
            }
        }
        while ((bh & 0x00100000) == 0)
        {   bh = (bh << 1) | (bl >> 31);
            bl = (bl << 1) | (bg >> 31);
            bg = bg << 1;
            bx -= 1;
            if (bx <= 0)
            {   a[0] = a[1] = 0;
                return 1;
            }
        }
    }
    else if (bh & 0x00200000)
    {   bg = (bl << 31) | (bg >> 1);
        bl = (bh << 31) | (bl >> 1);
        bh = bh >> 1;
        bx += 1;
        if (bx >= 0x7ff) return 0;      /* Overflow case                 */
    }
/* The magic rules about when and how to round are implemented here      */
    if (_needcarry(bg, bl))
    {   if (bl == 0xffffffff)
        {   bl = 0;
            bh += 1;
            if (bh & 0x00200000)
            {   bh = 0x00100000;
                bx += 1;
                if (bx >= 0x7ff) return 0;      /* Overflow case         */
            }
        }
        else bl += 1;
    }
    a[0] = (bh & ~0x00100000) | (bx << 20) | resultsign;
    a[1] = bl;
    return 1;
}

#endif

#ifdef IBMFLOAT

int flt_sum(unsigned int *a, unsigned int bh, unsigned int bl,
                             unsigned int ch, unsigned int cl)
{
    unsigned int w;
    int bx, cx;
    int shift;
    bx = (bh >> 24) & 0x7f;
    cx = (ch >> 24) & 0x7f;
    shift = bx - cx;
    if (shift < -(56/4))
    {   a[0] = ch;
        a[1] = cl;
        return 1;
    }
    if (shift > (56/4))
    {   a[0] = bh;
        a[1] = bl;
        return 1;
    }
    bh = bh & 0xffffff;
    ch = ch & 0xffffff;
/* Now I need to align the operands                                      */
    if (shift > 0)
    {   int rshift;
        if (shift > 8)
        {   cl = ch;
            ch = 0;
            shift -= 8;
        }
        rshift = 8 - shift;
        shift *= 4;
        rshift *= 4;
        cl = (ch << rshift) | (cl >> shift);
        ch = ch >> shift;
    }
    else if (shift < 0)
    {   int rshift;
        if (shift < -8)
        {   bl = bh;
            bh = 0;
            shift += 8;
        }
        shift = -shift;
        rshift = 8 - shift;
        shift *= 4;
        rshift *= 4;
        bl = (bh << rshift) | (bl >> shift);
        bh = bh >> shift;
        bx = cx;
    }
/* Now I just need to do a two-word addition                             */
    w = (bl & 0xff) + (cl & 0xff);
    bl = (bl >> 8) + (cl >> 8) + (w >> 8);
    bh += ch + (bl >> 24);
    bl = (bl << 8) + (w & 0xff);
    if (bh & 0xff000000)
/* Maybe I need to renormalize?                                          */
    {   bl = (bh << 28) | (bl >> 4);
        bh = bh >> 4;
        bx += 1;
        if (bx > 0x7f) return 0;            /* Overflow case             */
    }
/* I truncated rather than rounded.                                      */
    a[0] = bh | (bx << 24);
    a[1] = bl;
    return 1;
}

int flt_difference(unsigned int *a, unsigned int bh, unsigned int bl,
                                    unsigned int ch, unsigned int cl)
{
    unsigned int bg=0, cg=0, w, resultsign;
    int bx, cx;
    int shift;
    bx = (bh >> 24) & 0x7f;
    cx = (ch >> 24) & 0x7f;
    shift = bx - cx;
    if (shift < -(56/4) || (bh == 0 && bl == 0))
    {   a[0] = ch ^ 0x80000000;
        a[1] = cl;
        return 1;
    }
    if (shift > (56/4) || (ch == 0 && cl == 0))
    {   a[0] = bh;
        a[1] = bl;
        return 1;
    }
    bh = bh & 0xffffff;
    ch = ch & 0xffffff;
/* Now I need to align the operands                                      */
    if (shift > 0)
    {   int rshift;
        if (shift > 8)
        {   cg = cl;
            cl = ch;
            ch = 0;
            shift -= 8;
        }
        rshift = 8 - shift;
        shift *= 4;
        rshift *= 4;
        if (cg == 0) cg = cl << rshift;
        else cg = (cl << rshift) | 1;
        cl = (ch << rshift) | (cl >> shift);
        ch = ch >> shift;
        bg = 0;
    }
    else if (shift < 0)
    {   int rshift;
        if (shift < -8)
        {   bg = bl;
            bl = bh;
            bh = 0;
            shift += 8;
        }
        shift = -shift;
        rshift = 8 - shift;
        shift *= 4;
        rshift *= 4;
        if (bg == 0) bg = bl << rshift;
        else bg = (bl << rshift) | 1;
        bl = (bh << rshift) | (bl >> shift);
        bh = bh >> shift;
        cg = 0;
        bx = cx;
    }
    else bg = cg = 0;
/* Now for a subtraction, taking account of signs. Ugh.                  */
/* I rely on right shifts on signed types being arithmetic in struggles  */
/* to implement multiple precision arithmetic without a proper add-carry */
/* operator in my language.                                              */
    w = (bg & 0xff) - (cg & 0xff);
    bg = (bg >> 8) - (cg >> 8) + (((int)w) >> 8);
    cg = ((int) bg) >> 24;
    bg = (bg << 8) | (w & 0xff);
    w = (bl & 0xff) - (cl & 0xff) + cg;
    bl = (bl >> 8) - (cl >> 8) + (((int)w) >> 8);
    cg = ((int) bl) >> 24;
    bl = (bl << 8) | (w & 0xff);
    bh = bh - ch + cg;
/* Subtraction complete in 2s complement form.                           */
    if (bh & 0x80000000)  /* Sign of result must be negative             */
    {   if (bg == 0)
        {   if (bl == 0) bh = -bh;
            else
            {   bl = -bl;
                bh = ~bh;
            }
        }
        else
        {   bg = -bg;
            bl = ~bl;
            bh = ~bh;
        }
        resultsign = 0x80000000;
    }
    else resultsign = 0;
/* Subtraction now complete in sign & magnitude form                     */
    if (bh == 0 && bl == 0 && bg == 0)
    {   a[0] = a[1] = 0;           /* Result is absolutely zero          */
        return 1;
    }
/* Must have a normalized result.                                        */
    if ((bh & 0x00f00000)==0)       /* need to renormalize?              */
    {   while (bh == 0)
        {   bh = bl >> 8;
            bl = (bl << 24) | (bg >> 8);
            bg = bg << 24;
            bx -= 6;
            if (bx < 0)
            {   a[0] = a[1] = 0;
                return 1;
            }
        }
        while ((bh & 0x00f00000) == 0)
        {   bh = (bh << 4) | (bl >> 28);
            bl = (bl << 4) | (bg >> 28);
            bg = bg << 4;
            bx -= 1;
            if (bx < 0)
            {   a[0] = a[1] = 0;
                return 1;
            }
        }
    }
    else if (bh & 0x0f000000)
    {/* bg = (bl << 28) | (bg >> 4); */ /* guard word not needed any more */
        bl = (bh << 28) | (bl >> 4);
        bh = bh >> 4;
        bx += 1;
        if (bx > 0x7f) return 0;        /* Overflow case                 */
    }
    a[0] = bh | (bx << 24) | resultsign;
    a[1] = bl;
    return 1;
}

#endif

double _dadd(unsigned int bh, unsigned int bl,
             unsigned int ch, unsigned int cl)
{
    int ok;
    double a[1];
/* Do a case analysis on signs to end up with arithmetic on unsigned floats */
    if (bh & 0x80000000)
    {   if (ch & 0x80000000)
        {   ok = flt_sum((unsigned int *)a, bh & 0x7fffffff, bl,
                                            ch & 0x7fffffff, cl);
            ((int *)a)[0] ^= 0x80000000;
        }
        else ok = flt_difference((unsigned int *)a, ch, cl,
                                                    bh & 0x7fffffff, bl);
    }
    else if (ch & 0x80000000)
        ok = flt_difference((unsigned int *)a, bh, bl, ch & 0x7fffffff, cl);
    else ok = flt_sum((unsigned int *)a, bh, bl, ch, cl);
    if (!ok) raise(SIGFPE);
    return a[0];
}

double _dsub(unsigned int bh, unsigned int bl,
             unsigned int ch, unsigned int cl)
{
    int ok;
    double a[1];
    if (bh & 0x80000000)
    {   if (ch & 0x80000000)
            ok = flt_difference((unsigned int *)a, ch & 0x7fffffff, cl,
                                                   bh & 0x7fffffff, bl);
        else
        {   ok = flt_sum((unsigned int *)a, bh & 0x7fffffff, bl, ch, cl);
            ((unsigned int *)a)[0] ^= 0x80000000;
        }
    }
    else if (ch & 0x80000000)
        ok = flt_sum((unsigned int *)a, bh, bl, ch & 0x7fffffff, cl);
    else ok = flt_difference((unsigned int *)a, bh, bl, ch, cl);
    if (!ok) raise(SIGFPE);
    return a[0];
}

#ifdef IEEE

double _dmul(unsigned int bh, unsigned int bl,
             unsigned int ch, unsigned int cl)
{
    double a[1];
    int ax, bx, cx, as;
    unsigned int ah, al, carry;
    unsigned int aa[7], bb[4], cc[4];
    int i, j;
    as = (bh ^ ch) & 0x80000000;    /* sign for result */
    bx = (bh >> 20) & 0x7ff;
    cx = (ch >> 20) & 0x7ff;
    if (bx==0 || cx==0)
    {   ((unsigned int *)a)[0] = as;    /* multiplication by 0.0         */
        ((unsigned int *)a)[1] = 0;     /* note treatment of sign.       */
        return a[0];
    }
    bh = (bh & 0xfffff) | 0x100000;
    ch = (ch & 0xfffff) | 0x100000;
/* I split the operands into 14-bit chunks and do a long multiplication. */
/* As coded here I put more effort than might really be needed into the  */
/* low order bits of the product, but for now I am more concerned with   */
/* ease of coding and accuracy of results than with absolute speed.      */
/* On the ARM it MIGHT be that a shift-and-add long multiply coded at    */
/* the level would be faster?                                            */
    bb[0] = bh >> 7;
    bb[1] = ((bh & 0x7f) << 7) | (bl >> 25);
    bb[2] = (bl >> 11) & ~0x003fc000;
    bb[3] = (bl << 3) & 0x3fff;
    cc[0] = ch >> 7;
    cc[1] = ((ch & 0x7f) << 7) | (cl >> 25);
    cc[2] = (cl >> 11) & ~0x003fc000;
    cc[3] = (cl << 3) & 0x3fff;
    aa[0] = aa[1] = aa[2] = aa[3] = aa[4] = aa[5] = aa[6] = 0;
    for (i=0; i<4; i++)
        for (j=0; j<4; j++)
            aa[i+j] += bb[i] * cc[j];
    carry = 0;
    for (i=6; i!=0; i--)
    {   unsigned int w = aa[i] + carry;
        aa[i] = w & 0x3fff;
        carry = w >> 14;
    }
    carry = aa[0] + carry;
    ax = bx + cx - 0x3fe;
    if ((carry & 0x08000000) == 0)
    {   carry = (carry << 1) | (aa[1] >> 13);
        aa[1] = ((aa[1] << 1) & ~0xc000) | (aa[2] >> 13);
        aa[2] = ((aa[2] << 1) & ~0xc000) | (aa[3] >> 13);
/* aa[3] to aa[6] are guard digits and do not need shifting here (!)     */
        ax -= 1;
    }
    ah = carry >> 7;
    al = ((carry & 0x7f) << 25) | (aa[1] << 11) | (aa[2] >> 3);
    carry = ((aa[2] & 0x3) | aa[3] | aa[4] | aa[5] | aa[6]) |
            ((aa[2] & 0x4) << 29);
/* The magic rules about when and how to round are implemented here      */
    if (_needcarry(carry, al))
    {   if (al == 0xffffffff)
        {   al = 0;
            ah += 1;
            if (ah & 0x00200000)
            {   ah = 0x00100000;
                ax += 1;
            }
        }
        else al += 1;
    }
    if (ax >= 0x7ff) raise(SIGFPE);      /* Overflow */
    else if (ax <= 0)
    {   ((unsigned int *)a)[0] = as;  /* N.B. keep sign on underflow     */
        ((unsigned int *)a)[1] = 0;
        return a[0];
    }
    ((unsigned int *)a)[0] = (ah & ~0x00100000) | (ax << 20) | as;
    ((unsigned int *)a)[1] = al;
    return a[0];
}

double _ddiv(unsigned int bh, unsigned int bl,
             unsigned int ch, unsigned int cl)
{
    double a[1];
    unsigned int ah, al, as = (bh ^ ch) & 0x80000000;
    int ax, bx, cx, i;
    bx = (bh >> 20) & 0x7ff;
    cx = (ch >> 20) & 0x7ff;
    if (cx == 0) raise(SIGFPE);         /* division by zero              */
    if (bx == 0)
    {   ((unsigned int *)a)[0] = as;    /*  0.0 / anything  = 0.0        */
        ((unsigned int *)a)[1] = 0;
        return a[0];
    }
    bh = (bh & 0xfffff) | 0x00100000;
    ch = (ch & 0xfffff) | 0x00100000;
    ax = bx - cx + 0x3fe;
    ah = al = 0;
/* Do the division by test-and-subtract                                  */
    for (i = 0; i<55; i++)
    {   if (bh > ch || (bh == ch && bl >= cl))
        {   unsigned int w = (bl & 0xff) - (cl & 0xff);
/* Do a double length subtraction (oh the carry is a misery)             */
            bl = (bl >> 8) - (cl >> 8) + ((int)w >> 8);
            bh = bh - ch + (((int) bl) >> 24);
            bl = (bl << 8) | (w & 0xff);
            ah = (ah << 1) | (al >> 31);
            al = (al << 1) | 1;
        }
        else
        {   ah = (ah << 1) | (al >> 31);
            al = al << 1;
        }
        bh = (bh << 1) | (bl >> 31);
        bl = bl << 1;
    }
    bh |= bl;                                    /* sticky bits now here */
    bh = (bh & 0xff) | (bh >> 8);                /* top bit clear.       */
    if (ah & 0x00400000)
    {   bh |= al & 1;
        al = (al >> 1) | (ah << 31);
        ah = ah >> 1;
        ax += 1;
    }
    bh = bh | (al << 31);
    al = (al >> 1) | (ah << 31);
    ah = ah >> 1;
/* The magic rules about when and how to round are implemented here      */
    if (_needcarry(bh, al))
    {   if (al == 0xffffffff)
        {   al = 0;
            ah += 1;
            if (ah & 0x00200000)
            {   ah = 0x00100000;
                ax += 1;
            }
        }
        else al += 1;
    }
    if (ax >= 0x7ff) raise(SIGFPE); /* Overflow on the division          */
    else if (ax <= 0)
    {   ((unsigned int *)a)[0] = as;  /* N.B. keep sign on underflow     */
        ((unsigned int *)a)[1] = 0;
        return a[0];
    }
    ((int *)a)[0] = (ah & ~0x00100000) | (ax << 20) | as;
    ((int *)a)[1] = al;
    return a[0];
}

#endif

#ifdef IBMFLOAT

double _dmul(unsigned int bh, unsigned int bl,
             unsigned int ch, unsigned int cl)
{
    double a[1];
    int ax, bx, cx, as;
    unsigned int ah, al, carry;
    unsigned int aa[7], bb[4], cc[4];
    int i, j;
    as = (bh ^ ch) & 0x80000000;    /* sign for result */
    bx = (bh >> 24) & 0x7f;
    cx = (ch >> 24) & 0x7f;
    if ((bh==0 && bl==0) || (ch==0 && cl==0))
    {   ((unsigned int *)a)[0] = as;    /* multiplication by 0.0         */
        ((unsigned int *)a)[1] = 0;     /* note treatment of sign.       */
        return a[0];
    }
    bh = bh & 0xffffff;
    ch = ch & 0xffffff;
/* I split the operands into 14-bit chunks and do a long multiplication. */
/* As coded here I put more effort than might really be needed into the  */
/* low order bits of the product, but for now I am more concerned with   */
/* ease of coding and accuracy of results than with absolute speed.      */
    bb[0] = bh >> 10;
    bb[1] = ((bh & 0x3ff) << 4) | (bl >> 28);
    bb[2] = (bl >> 14) & 0x3fff;
    bb[3] = bl & 0x3fff;
    cc[0] = ch >> 10;
    cc[1] = ((ch & 0x3ff) << 4) | (cl >> 28);
    cc[2] = (cl >> 14) & 0x3fff;
    cc[3] = cl & 0x3fff;
    aa[0] = aa[1] = aa[2] = aa[3] = aa[4] = aa[5] = aa[6] = 0;
    for (i=0; i<4; i++)
        for (j=0; j<4; j++)
            aa[i+j] += bb[i] * cc[j];
    carry = 0;
    for (i=6; i!=0; i--)
    {   unsigned int w = aa[i] + carry;
        aa[i] = w & 0x3fff;
        carry = w >> 14;
    }
    carry = aa[0] + carry;
    ax = bx + cx - 0x40;
    if ((carry & 0x0f000000) == 0)
    {   carry = (carry << 4) | (aa[1] >> 10);
        aa[1] = ((aa[1] << 4) & 0x3fff) | (aa[2] >> 10);
        aa[2] = ((aa[2] << 4) & 0x3fff) | (aa[3] >> 10);
        ax -= 1;
    }
    ah = carry >> 4;
    al = ((carry & 0xf) << 28) | (aa[1] << 14) | aa[2];
    if (ax > 0x7f) raise(SIGFPE); /* Overflow */
    else if (ax < 0)
    {   ((unsigned int *)a)[0] = as;  /* N.B. keep sign on underflow     */
        ((unsigned int *)a)[1] = 0;
    }
    else
    {   ((unsigned int *)a)[0] = ah | (ax << 24) | as;
        ((unsigned int *)a)[1] = al;
    }
    return a[0];
}

double _ddiv(unsigned int bh, unsigned int bl,
             unsigned int ch, unsigned int cl)
{
    double a[1];
    unsigned int ah, al, as = (bh ^ ch) & 0x80000000;
    int ax, bx, cx, i;
    bx = (bh >> 24) & 0x7f;
    cx = (ch >> 24) & 0x7f;
    if ((ch & 0x7fffffff) == 0 && cl == 0) raise(SIGFPE); /* .. by zero  */
    if ((bh & 0x7fffffff)==0 && bl == 0)
    {   ((unsigned int *)a)[0] = as;    /*  0.0 / anything  = 0.0        */
        ((unsigned int *)a)[1] = 0;
        return a[0];
    }
    bh = bh & 0xffffff;
    ch = ch & 0xffffff;
    ax = bx - cx + 0x40;
    ah = al = 0;
/* Do the division by test-and-subtract                                  */
    for (i = 0; i<=(56/4); i++)
    {   int nxt = 0;
        while (bh > ch || (bh == ch && bl >= cl))
        {   unsigned int w = (bl & 0xff) - (cl & 0xff);
/* Do a double length subtraction (oh the carry is a misery)             */
            bl = (bl >> 8) - (cl >> 8) + ((int)w >> 8);
            bh = bh - ch + (((int) bl) >> 24);
            bl = (bl << 8) | (w & 0xff);
            nxt++;
        }
        ah = (ah << 4) | (al >> 28);
        al = (al << 4) | nxt;
        bh = (bh << 4) | (bl >> 28);
        bl = bl << 4;
    }
    if (ah & 0xff000000)
    {   al = (al >> 4) | (ah << 28);
        ah = ah >> 4;
        ax += 1;
    }
    if (ax > 0x7f) raise(SIGFPE);     /* Overflow on the division        */
    else if (ax < 0)
    {   ((unsigned int *)a)[0] = as;  /* N.B. keep sign on underflow     */
        ((unsigned int *)a)[1] = 0;
        return a[0];
    }
    ((int *)a)[0] = ah | (ax << 24) | as;
    ((int *)a)[1] = al;
    return a[0];
}

#endif

double _dneg(int bh, int bl)
{
    int aa[2];
    if (bh == 0 && bl == 0) aa[0] = aa[1] = 0;
    else
    {   aa[0] = bh ^ 0x80000000;
        aa[1] = bl;
    }
    return *(double *)aa;
}


int _dgeq(double a, double b)
{
    return !_dgr(b, a);
}

int _dls(double a, double b)
{
    return _dgr(b, a);
}

int _dleq(double a, double b)
{
    return !_dgr(a, b);
}

int _dgr(int bh, unsigned int bl,
         int ch, unsigned int cl)
{
/* +0.0 is equal to -0.0                                                 */
#ifdef IEEE
    if ((bh & 0x7ff00000)==0 && (ch & 0x7ff00000)==0) return 0;
#endif
#ifdef IBMFLOAT
    if ((bh & 0x7fffffff) == 0 && bl == 0 &&
        (ch & 0x7fffffff) == 0 && cl == 0) return 0;
#endif
    if (bh < 0 && ch >= 0) return 0;
    else if (bh >= 0 && ch < 0) return 1;
    else if (bh < 0)
    {   int temp = bh;
        bh = ch & 0x7fffffff;
        ch = temp & 0x7fffffff;
        temp = bl;
        bl = cl;
        cl = temp;
    }
    if (bh < ch) return 0;
    else if (bh > ch) return 1;
    else if (bl < cl) return 0;
    else if (bl > cl) return 1;
    else return 0;
}

int _dneq(double a, double b)
{
    return !_deq(a, b);
}

int _deq(unsigned int bh, unsigned int bl,
         unsigned int ch, unsigned int cl)
{
    if (bl != cl) return 0;
    if (bh == ch) return 1;
/* I ensure that +0 == -0 here.                                          */
    if (bh != 0 && bh != 0x80000000) return 0;
    if (ch != 0 && ch != 0x80000000) return 0;
    else return 1;
}


#ifdef IEEE

double _dflt(int n)
{
    unsigned int ah, al;
    double a[1];
    int as, ax;
    if (n==0)
    {   ((unsigned int *)a)[0] = 0;
        ((unsigned int *)a)[1] = 0;
        return a[0];
    }
    else if (n>0) as = 0;
    else
    {   as = 0x80000000;
        n = -n;             /* from here on n is thought of as unsigned  */
    }
    ah = 0;
    al = n;
    ax = 0x400 + 51;
    while ((ah & 0x00100000)==0)
    {   ah = (ah << 1) | (al >> 31);
        al = al << 1;
        ax -= 1;
    }
    ((unsigned int *)a)[0] = (ah & ~0x00100000) | (ax << 20) | as;
    ((unsigned int *)a)[1] = al;
    return a[0];
}

double _dfltu(unsigned int n)
{
    unsigned int ah, al;
    double a[1];
    int ax;
    if (n==0)
    {   ((unsigned int *)a)[0] = 0;
        ((unsigned int *)a)[1] = 0;
        return a[0];
    }
    ah = 0;
    al = n;
    ax = 0x400 + 51;
    while ((ah & 0x00100000)==0)
    {   ah = (ah << 1) | (al >> 31);
        al = al << 1;
        ax -= 1;
    }
    ((unsigned int *)a)[0] = (ah & ~0x00100000) | (ax << 20);
    ((unsigned int *)a)[1] = al;
    return a[0];
}

int _dfix(unsigned int ah, unsigned int al)
{
    int sign, ax;
    if (ah & 0x80000000)
    {   ah &= ~0x80000000;
        sign = 1;
    }
    else sign = 0;
    ax = ah >> 20;
    ah = (ah & 0x000fffff) | 0x00100000;
    if (ax < 0x3ff)                     /* arg < 1.0 ... result 0        */
        return 0;
    else if (ax > 0x400 + 51) return raise(SIGFPE); /* overflow          */
    while (ax != 0x400 + 51)
    {   al = (al >> 1) | (ah << 31);
        ah = ah >> 1;
        ax += 1;
    }
    if (sign)
    {   if (al > 0x80000000) return raise(SIGFPE);
        al = -al;
    }
    else if (al >= 0x80000000) return raise(SIGFPE);
    return al;
}

unsigned int _dfixu(unsigned int ah, unsigned int al)
{
    int ax;
    ax = (ah >> 20) & 0x7ff;
    if ((ah & 0x80000000)!=0 && ax!=0) return raise(SIGFPE);
                                       /* negative nonzero  */
    ah = (ah & 0x000fffff) | 0x00100000;
    if (ax < 0x3ff)                     /* arg < 1.0 ... result 0        */
        return 0;
    else if (ax > 0x400 + 51) return raise(SIGFPE); /* overflow          */
    while (ax != 0x400 + 51)
    {   al = (al >> 1) | (ah << 31);
        ah = ah >> 1;
        ax += 1;
    }
    return al;
}

#endif

#ifdef IBMFLOAT

double _dflt(int n)
{
    double a[1];
    unsigned int ah, al;
    int as, ax;
    if (n==0)
    {   ((unsigned int *)a)[0] = 0;
        ((unsigned int *)a)[1] = 0;
        return a[0];
    }
    else if (n>0) as = 0;
    else
    {   as = 0x80000000;
        n = -n;             /* from here on n is thought of as unsigned  */
    }
    ah = 0;
    al = n;
    ax = 0x40 + (56/4);
    while ((ah & 0x00f00000)==0)
    {   ah = (ah << 4) | (al >> 28);
        al = al << 4;
        ax -= 1;
    }
    ((unsigned int *)a)[0] = ah | (ax << 24) | as;
    ((unsigned int *)a)[1] = al;
    return a[0];
}

double _dfltu(unsigned int n)
{
    double a[1];
    unsigned int ah, al;
    int ax;
    if (n==0)
    {   ((unsigned int *)a)[0] = 0;
        ((unsigned int *)a)[1] = 0;
        return a[0];
    }
    ah = 0;
    al = n;
    ax = 0x40 + (56/4);
    while ((ah & 0x00f00000)==0)
    {   ah = (ah << 4) | (al >> 28);
        al = al << 4;
        ax -= 1;
    }
    ((unsigned int *)a)[0] = ah | (ax << 24);
    ((unsigned int *)a)[1] = al;
    return a[0];
}

int _dfix(unsigned int ah, unsigned int al)
{
    int sign, ax;
    if (ah & 0x80000000) sign = 1;
    else sign = 0;
    if ((ah & 0x7fffffff)==0 && al==0) return 0;
    ax = (ah >> 24) & 0x7f;
    ah = ah & 0x00ffffff;
    if (ax < 0x41)                      /* arg < 1.0 ... result 0        */
        return 0;
    else if (ax > 0x40 + (56/4)) return raise(SIGFPE); /* overflow       */
    while (ax != 0x40 + (56/4))
    {   al = (al >> 4) | (ah << 28);
        ah = ah >> 4;
        ax += 1;
    }
    if (ah != 0) return raise(SIGFPE); /* Overflow */
    if (sign)
    {   al = -al;
        if ((al & 0x80000000)==0) return raise(SIGFPE);
    }
    else if (al & 0x80000000) return raise(SIGFPE);
    return al;
}

unsigned int _dfixu(unsigned int ah, unsigned int al)
{
    int ax;
    if ((ah & 0x7fffffff)==0 && al==0) return 0;
    ax = (ah >> 24) & 0x7f;
    if ((ah & 0x80000000)!=0) return raise(SIGFPE);    /* negative       */
    ah = ah & 0x00ffffff;
    if (ax < 0x41)                      /* arg < 1.0 ... result 0        */
        return 0;
    else if (ax > 0x40 + (56/4)) return raise(SIGFPE); /* overflow       */
    while (ax != 0x40 + (56/4))
    {   al = (al >> 4) | (ah << 28);
        ah = ah >> 4;
        ax += 1;
    }
    if (ah != 0) return raise(SIGFPE); /* Overflow */
    return al;
}

#endif

/* end of softfp.c */

