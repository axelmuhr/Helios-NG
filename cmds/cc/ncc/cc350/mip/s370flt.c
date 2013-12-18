
/* C compiler file s370flt.c: Copyright (C) Codemist Ltd. 1987 */

/* version 4b */

#include <ctype.h>

#include "globals.h"
#include "util.h"
#include "errors.h"

void fltrep_stod(const char *s, DbleBin *p)
{
/* s is a string containing a floating point number (syntax has been     */
/* checked already). p points to two words of store into which the IBM   */
/* representation of the number must be pushed. Do the packing using     */
/* only integer arithmetic to (a) guarantee good rounding and (b) to     */
/* allow cross compilation.                                              */
    int32 ds, dh, dm, dl, bx;     /* floating point result in pieces     */
    int32 th, tm, tl, x, x1, ch, dot;
    ds = 0;
    switch (ch = *s++)
    {
case '-':
        ds = 0x80000000;
case '+':
        ch = *s++;
default:
        break;
    }
    dh = dm = dl = bx = 0;
/* I keep a 3-word value (th, tm, tl) to be 10**(-n) where n digits have */
/* been read so far, and add toint(ch)*t into (dh, dm, dl) at each step  */
    th = 0x00100000;
    tm = tl = x = x1 = dot = 0;
    while (ch=='0') ch = *s++;
    if (ch=='.')
    {   dot = 1;
        while ((ch=*s++)=='0') x -= 1;
    }
/* Now I have dealt with all the leading zeros and so I can used fixed   */
/* point arithmetic to assemble the number.                              */
    while (1)
    {   if (!dot && ch=='.') ch = *s++, dot = 1;
        else if (isdigit(ch))
        {   int32 carry = ((th % 10)<<24) + tm;
            th = th / 10;
            tm = carry / 10;
            tl = (((carry % 10)<<24) + tl) / 10;
            ch = intofdigit(ch);
            dl += ch*tl;
            dm += ch*tm + ((dl>>24) & 0xff);
            dh += ch*th + ((dm>>24) & 0xff);
            dl &= 0x00ffffff;
            dm &= 0x00ffffff;
            if (!dot) x += 1;
            ch = *s++;
        }
        else break;
    }
    if (dh==0)      /* result = 0.0? : normalization => test on dh is OK */
    {   p->msd = 0;   /* representation of 0.0 is zero words               */
        p->lsd = 0;
        return;
    }
    if (ch=='e' || ch=='E')
    {   int32 sx = 0;
        switch (ch=*s++)
        {
    case '-':
            sx = 1;
    case '+':
            ch = *s++;
    default:
            break;
        }
        while (isdigit(ch))             /* read an exponent              */
        {   x1 = 10*x1 + intofdigit(ch);
            ch = *s++;
        }
        if (sx) x -= x1;
        else x += x1;
    }
/* Now I have to multiply or divide the number by some power of 10       */
        while (x>0)
        {   dl *= 10;
            dm = 10*dm + ((dl>>24) & 0xff);
            dh = 10*dh + ((dm>>24) & 0xff);
            dl &= 0x00ffffff;
            dm &= 0x00ffffff;
            while ((dh & 0xff000000)!=0)
            {   dl = (dl>>4) | ((dm&0xf)<<20);
                dm = (dm>>4) | ((dh&0xf)<<20);
                dh = dh>>4;
                bx += 1;    /* hexadecimal exponent */
            }
            x -= 1;
        }
        while (x<0)
        {   int32 carry = ((dh % 10)<<24) + dm;
            dh /= 10;
            dm = carry / 10;
            dl = (dl + ((carry % 10)<<24)) / 10;
            dm += ((dl>>24) & 0xff);
            dh += ((dm>>24) & 0xff);
            dl &= 0x00ffffff;
            dm &= 0x00ffffff;
            while ((dh & 0xfff00000)==0)
            {   dh = (dh<<4) | ((dm>>20)&0xf);
                dm = ((dm<<4) | ((dl>>20)&0xf)) & 0x00ffffff;
                dl = (dl<<4) & 0x00ffffff;
                bx -= 1;
            }
            x += 1;
        }
/* postnormalize to ensure that digit 0x00x00000 is the most significant */
    for (;;)
    {   while ((dh & 0xfff00000)==0)
        {   dh = (dh<<4) | ((dm>>20)&0xf);
            dm = ((dm<<4) | ((dl>>20)&0xf)) & 0x00ffffff;
            dl = (dl<<4) & 0x00ffffff;
            bx -= 1;
        }
        while ((dh & 0xff000000)!=0)
        {   dl = (dl>>4) | ((dm&0xf)<<20);
            dm = (dm>>4) | ((dh&0xf)<<20);
            dh = dh>>4;
            bx += 1;
        }
/* Round up if necessary, using 8 guard bits.                            */
        if ((dl & 0xff00)>0x8000)
        {   dl += 0x00010000;
            dm += (dl >> 24) & 0xff;
            dh += (dm >> 24) & 0xff;
            dl &= 0x00ff0000;
            dm &= 0x00ffffff;
            continue;           /* may need more normalization           */
        }
        else if ((dl & 0xff00)==0x8000)  /* force zero in middle case    */
            dl &= 0x00fe0000;
        break;
    }
    bx += 0x41;             /* assemble final exponent                   */
    if (bx<0)
    {   cc_rerr(fp_rerr_very_small);
        p->msd = 0;
        p->lsd = 0;
        return;
    }
    if (bx>0x7f)
    {   cc_err(fp_err_very_big);
        bx = 0x7f;              /* overflow -> largest possible value    */
        dh = 0x00ffffff;
        dm = 0xffffffff;
        dl = 0x00ff0000;
    }
    dh = ds | (bx<<24) | dh;
    dm = (dm<<8) | ((dl>>16) & 0xff);
    p->msd = dh;
    p->lsd = dm;
    return;
}

bool fltrep_narrow(DbleBin *d, FloatBin *e)
{
    e->val = d->msd;    /* Truncate */
    return 0;
}

bool fltrep_narrow_round(DbleBin *d, FloatBin *e)
/* this is now a function so that caller can report errors soon */
{   int32 hi = d->msd, lo = d->lsd;
    int32 x = (hi>>24) & 0x7f;
    int32 m = hi & 0x00ffffff;
    if (hi==0 && lo==0)         /* value is zero - treat specially       */
    {   e->val = 0;
        return 0;               /* OK */
    }
    if (lo < 0 && (lo!=0x80000000 || (m & 1)!=0))
        if ((m += 1) == 0x01000000)     /* round up & renormalize        */
        {   m = 0x00100000;
            x += 1;
        }
    if (x > 0x7f)
/* With the IBM format there is no separate representation of infinity,  */
/* so (float)1.e999 will give TWO compile-time errors, one when the      */
/* original FP number overflows when parsed in double precision and      */
/* again when it is narrowed down to single precision. Tough.            */
    {   cc_err(fp_err_big_single);
        e->val = (hi & 0x80000000) | 0x7fffffff;
        return 1;                /* overflow */
    }
    else if (x < 0)
/* This can not happen, but I will leave the code in to show what could  */
/* be imagined.                                                          */
    {   cc_rerr(fp_rerr_small_single);
        e->val = (hi & 0x80000000) | (0L << 23);   /* preserve sign */
        return -1;               /* underflow */
    }
    e->val = (hi & 0x80000000) | (x << 24) | m;
    return 0;
}

void fltrep_widen(FloatBin *e,DbleBin *d)
{
    d->msd = e->val;
    d->lsd = 0;
}

/* Implement IBM format floating point arithmetic. For now this uses the */
/* FP arithmetic of the computer you are running on, and so cross        */
/* compilation of floating point code will give trouble.                 */


/* flt_compare returns -1, 0 or +1 for *a<*b, *a==*b or *a>*b. The other */
/* routines return a flag that is 0 if the procedure detected overflow   */
/* or division by zero, or if the operand was out of range for           */
/* conversion to an integral type.                                       */


/* Coded without concern about speed, and so quite dreadfully slow.      */
/* Also coded without concern about code density, so rather repetitive   */
/* and bulky. Memory is supposed to be cheap these days.                 */


static bool flt_sum(DbleBin *a, unsigned32 bh, unsigned32 bl,
                                unsigned32 ch, unsigned32 cl)
{
    unsigned32 w;
    int32 bx, cx;
    int32 shift;
    bx = (bh >> 24) & 0x7f;
    cx = (ch >> 24) & 0x7f;
    shift = bx - cx;
    if (shift < -(56/4))
    {   a->msd = ch;
        a->lsd = cl;
        return 1;
    }
    if (shift > (56/4))
    {   a->msd = bh;
        a->lsd = bl;
        return 1;
    }
    bh = bh & 0xffffff;
    ch = ch & 0xffffff;
/* Now I need to align the operands                                      */
    if (shift > 0)
    {   int32 rshift;
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
    {   int32 rshift;
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
    a->msd = bh | (bx << 24);
    a->lsd = bl;
    return 1;
}

static bool flt_difference(DbleBin *a, unsigned32 bh, unsigned32 bl,
                                       unsigned32 ch, unsigned32 cl)
{
    unsigned32 bg=0, cg=0, w, resultsign;
    int32 bx, cx;
    int32 shift;
    bx = (bh >> 24) & 0x7f;
    cx = (ch >> 24) & 0x7f;
    shift = bx - cx;
    if (shift < -(56/4) || bx == 0)
    {   a->msd = ch ^ 0x80000000;
        a->lsd = cl;
        return 1;
    }
    if (shift > (56/4) || cx == 0)
    {   a->msd = bh;
        a->lsd = bl;
        return 1;
    }
    bh = bh & 0xffffff;
    ch = ch & 0xffffff;
/* Now I need to align the operands                                      */
    if (shift > 0)
    {   int32 rshift;
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
    {   int32 rshift;
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
    bg = (bg >> 8) - (cg >> 8) + (((int32)w) >> 8);
    cg = ((int32) bg) >> 24;
    bg = (bg << 8) | (w & 0xff);
    w = (bl & 0xff) - (cl & 0xff) + cg;
    bl = (bl >> 8) - (cl >> 8) + (((int32)w) >> 8);
    cg = ((int32) bl) >> 24;
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
    {   a->msd = a->lsd = 0;           /* Result is absolutely zero          */
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
            {   a->msd = a->lsd = 0;
                return 1;
            }
        }
        while ((bh & 0x00f00000) == 0)
        {   bh = (bh << 4) | (bl >> 28);
            bl = (bl << 4) | (bg >> 28);
            bg = bg << 4;
            bx -= 1;
            if (bx < 0)
            {   a->msd = a->lsd = 0;
                return 1;
            }
        }
    }
    else if (bh & 0x0f000000)
    {   bg = (bl << 28) | (bg >> 4);
        bl = (bh << 28) | (bl >> 4);
        bh = bh >> 4;
        bx += 1;
        if (bx > 0x7f) return 0;        /* Overflow case                 */
    }
    a->msd = bh | (bx << 24) | resultsign;
    a->lsd = bl;
    return 1;
}

bool flt_add(DbleBin *a, DbleBin *b, DbleBin *c)        /*   *a = *b + *c    */
{
    unsigned32 bh = b->msd;
    unsigned32 bl = b->lsd;
    unsigned32 ch = c->msd;
    unsigned32 cl = c->lsd;
    bool ok;
/* Do a case analysis on signs to end up with arithmetic on unsigned floats */
    if (bh & 0x80000000)
    {   if (ch & 0x80000000)
        {   ok = flt_sum(a, bh & 0x7fffffff, bl, ch & 0x7fffffff, cl);
            a->msd ^= 0x80000000;
        }
        else ok = flt_difference(a, ch, cl, bh & 0x7fffffff, bl);
    }
    else if (ch & 0x80000000)
        ok = flt_difference(a, bh, bl, ch & 0x7fffffff, cl);
    else ok = flt_sum(a, bh, bl, ch, cl);
    return ok;
}

bool flt_subtract(DbleBin *a, DbleBin *b, DbleBin *c)   /*   *a = *b - *c    */
{
    unsigned32 bh = b->msd;
    unsigned32 bl = b->lsd;
    unsigned32 ch = c->msd;
    unsigned32 cl = c->lsd;
    bool ok;
    if (bh & 0x80000000)
    {   if (ch & 0x80000000)
            ok = flt_difference(a, ch & 0x7fffffff, cl, bh & 0x7fffffff, bl);
        else
        {   ok = flt_sum(a, bh & 0x7fffffff, bl, ch, cl);
            a->msd ^= 0x80000000;
        }
    }
    else if (ch & 0x80000000)
        ok = flt_sum(a, bh, bl, ch & 0x7fffffff, cl);
    else ok = flt_difference(a, bh, bl, ch, cl);
    return ok;
}

bool flt_multiply(DbleBin *a, DbleBin *b, DbleBin *c)   /*   *a = *b * *c    */
{
    unsigned32 bh = b->msd;
    unsigned32 bl = b->lsd;
    unsigned32 ch = c->msd;
    unsigned32 cl = c->lsd;
    int32 ax, bx, cx, as;
    unsigned32 ah, al, carry;
    unsigned32 aa[7], bb[4], cc[4];
    int32 i, j;
    as = (bh ^ ch) & 0x80000000;    /* sign for result */
    bx = (bh >> 24) & 0x7f;
    cx = (ch >> 24) & 0x7f;
    if ((bh==0 && bl==0) || (ch==0 && cl==0))
    {   a->msd = as;    /* multiplication by 0.0         */
        a->lsd = 0;     /* note treatment of sign.       */
        return 1;
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
    {   unsigned32 w = aa[i] + carry;
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
    if (ax > 0x7f) return 0;      /* Overflow */
    else if (ax < 0)
    {   a->msd = as;  /* N.B. keep sign on underflow     */
        a->lsd = 0;
        return 1;
    }
    a->msd = ah | (ax << 24) | as;
    a->lsd = al;
    return 1;
}

bool flt_divide(DbleBin *a, DbleBin *b, DbleBin *c)     /*   *a = *b / *c    */
{
    unsigned32 bh = b->msd;
    unsigned32 bl = b->lsd;
    unsigned32 ch = c->msd;
    unsigned32 cl = c->lsd;
    unsigned32 ah, al, as = (bh ^ ch) & 0x80000000;
    int32 ax, bx, cx, i;
    bx = (bh >> 24) & 0x7f;
    cx = (ch >> 24) & 0x7f;
    if ((ch & 0x7fffffff) == 0 && cl == 0) return 0; /* division by zero */
    if ((bh & 0x7fffffff)==0 && bl == 0)
    {   a->msd = as;    /*  0.0 / anything  = 0.0        */
        a->lsd = 0;
        return 1;
    }
    bh = bh & 0xffffff;
    ch = ch & 0xffffff;
    ax = bx - cx + 0x40;
    ah = al = 0;
/* Do the division by test-and-subtract                                  */
    for (i = 0; i<=(56/4); i++)
    {   int32 nxt = 0;
        while (bh > ch || (bh == ch && bl >= cl))
        {   unsigned32 w = (bl & 0xff) - (cl & 0xff);
/* Do a double length subtraction (oh the carry is a misery)             */
            bl = (bl >> 8) - (cl >> 8) + ((int32)w >> 8);
            bh = bh - ch + (((int32) bl) >> 24);
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
    if (ax > 0x7f) return 0;        /* Overflow on the division          */
    else if (ax < 0)
    {   a->msd = as;  /* N.B. keep sign on underflow     */
        a->lsd = 0;
        return 1;
    }
    a->msd = ah | (ax << 24) | as;
    a->lsd = al;
    return 1;
}

int flt_compare(DbleBin *b, DbleBin *c)               /* (int32)sign(*b-*c)  */
{
/* Integer comparison of the 64bit values almost gives the answer I want */
    int32 bh = b->msd;
    unsigned32 bl = b->lsd;
    int32 ch = c->msd;
    unsigned32 cl = c->lsd;
/* +0.0 is equal to -0.0                                                 */
    if ((bh & 0x7fffffff) == 0 && bl == 0 &&
        (ch & 0x7fffffff) == 0 && cl == 0) return 0;
    if (bh < 0 && ch >= 0) return -1;
    else if (bh >= 0 && ch < 0) return 1;
    else if (bh < 0)
    {   int32 temp = bh;
        bh = ch & 0x7fffffff;
        ch = temp & 0x7fffffff;
        temp = bl;
        bl = cl;
        cl = temp;
    }
    if (bh < ch) return -1;
    else if (bh > ch) return 1;
    else if (bl < cl) return -1;
    else if (bl > cl) return 1;
    else return 0;
}

bool flt_move(DbleBin *a, DbleBin *b)                  /*   *a = *b         */
{
/* This works for IEEE and IBM formats!                                  */
    a->msd = b->msd;
    a->lsd = b->lsd;
    return 1;
}

bool flt_negate(DbleBin *a, DbleBin *b)                /*   *a = - *b       */
{
/* This works for IEEE and IBM formats!                                  */
    a->msd = b->msd ^ 0x80000000;
    a->lsd = b->lsd;
    return 1;                   /* can never fail                        */
}

#ifdef INT_TO_FLOAT_NEEDED

/* The following are not used in the Norcroft C compiler                 */

bool flt_itod(DbleBin *a, int32 n)                      /*   *a = (double) n */
{
    unsigned32 ah, al;
    int32 as, ax;
    if (n==0)
    {   a->msd = 0;
        a->lsd = 0;
        return 1;
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
    a->msd = ah | (ax << 24) | as;
    a->lsd = al;
    return 1;
}

bool flt_utod(DbleBin *a, unsigned n)                 /*   *a = (double) u */
{
    unsigned32 ah, al;
    int32 ax;
    if (n==0)
    {   a->msd = 0;
        a->lsd = 0;
        return 1;
    }
    ah = 0;
    al = n;
    ax = 0x40 + (56/4);
    while ((ah & 0x00f00000)==0)
    {   ah = (ah << 4) | (al >> 28);
        al = al << 1;
        ax -= 1;
    }
    a->msd = ah | (ax << 24);
    a->lsd = al;
    return 1;
}

#endif

bool flt_dtoi(int32 *n, DbleBin *a)                     /*   *n = (int32) *a   */
{
    unsigned32 ah, al;
    int32 sign, ax;
    ah = a->msd;
    al = a->lsd;
    if (ah & 0x80000000) sign = 1;
    else sign = 0;
    if ((ah & 0x7fffffff)==0 && al==0)
    {   *n = 0;
        return 1;
    }
    ax = (ah >> 24) & 0x7f;
    ah = ah & 0x00ffffff;
    if (ax < 0x40)                      /* arg < 0.5 ... result 0        */
    {   *n = 0;
        return 1;
    }
    else if (ax > 0x40 + (56/4)) return 0; /* overflow certainly         */
    while (ax != 0x40 + (56/4))
    {   al = (al >> 4) | (ah << 28);
        ah = ah >> 4;
        ax += 1;
    }
    if (ah != 0) return 0; /* Overflow */
    if (sign)
    {   al = -al;
        if ((al & 0x80000000)==0) return 0;
    }
    else if (al & 0x80000000) return 0;
    *n = al;
    return 1;
}

bool flt_dtou(unsigned32 *u, DbleBin *a)            /*   *u = (unsigned) *a  */
{
    unsigned32 ah, al;
    int32 ax;
    ah = a->msd;
    al = a->lsd;
    if ((ah & 0x7fffffff)==0 && al==0)
    {   *u = 0;
        return 1;
    }
    ax = (ah >> 24) & 0x7f;
    if ((ah & 0x80000000)!=0) return 0;    /* negative                   */
    ah = ah & 0x00ffffff;
    if (ax < 0x40)                      /* arg < 0.5 ... result 0        */
    {   *u = 0;
        return 1;
    }
    else if (ax > 0x40 + (56/4)) return 0; /* overflow certainly         */
    while (ax != 0x40 + (56/4))
    {   al = (al >> 4) | (ah << 28);
        ah = ah >> 4;
        ax += 1;
    }
    if (ah != 0) return 0; /* Overflow */
    *u = al;
    return 1;
}

/* end of s370flt.c */
