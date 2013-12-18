/*
 * C compiler file mip/ieeeflt.c, version 4
 * Copyright (C) Codemist Ltd., 1987-1992.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1993/07/27 15:58:32 $
 * Revising $Author: nickc $
 */

/* This module encodes, and performs arithmetic on IEEE values,          */
/* irrespective of the host hardware, by using integer-only arithmetic.  */
/* The routines are largely exported to lex.c sem.c                      */

#include <ctype.h>
#include "globals.h"
#include "errors.h"
#include "util.h"

void fltrep_stod(const char *s, DbleBin *p)
{
/* s is a string containing a floating point number (syntax has been     */
/* checked already). p points to two words of store into which the IEEE  */
/* representation of the number must be pushed. Do the packing using     */
/* only integer arithmetic to (a) guarantee good rounding and (b) ensure */
/* that this compiler works even when FPE is not active.                 */
    int32 ds, dh, dm, dl, bx;     /* floating point result in pieces       */
    int32 th, tm, tl, x, x1, ch, dot;
    ds = 0;
    switch (ch = *s++)
    {
case '-':
        ds = 0x80000000UL;
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
        while ((ch = *s++)=='0') x -= 1;
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
    {   p->msd = 0;
        p->lsd = 0;
        return;
    }
    if (ch=='e' || ch=='E')
    {   int32 sx = 0;
        switch (ch = *s++)
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
            while ((dh & 0xffe00000UL)!=0)
            {   dl = (dl>>1) | ((dm&1)<<23);
                dm = (dm>>1) | ((dh&1)<<23);
                dh = dh>>1;
                bx += 1;
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
            while ((dh & 0xfff00000UL)==0)
            {   dh = (dh<<1) | ((dm>>23)&1);
                dm = ((dm<<1) | ((dl>>23)&1)) & 0x00ffffff;
                dl = (dl<<1) & 0x00ffffff;
                bx -= 1;
            }
            x += 1;
        }
    /* postnormalize to ensure that bit 0x00100000 is the MSB            */
    for (;;)
    {   while ((dh & 0xfff00000UL)==0)
        {   dh = (dh<<1) | ((dm>>23)&1);
            dm = ((dm<<1) | ((dl>>23)&1)) & 0x00ffffff;
            dl = (dl<<1) & 0x00ffffff;
            bx -= 1;
        }
        while ((dh & 0xffe00000UL)!=0)
        {   dl = (dl>>1) | ((dm&1)<<23);
            dm = (dm>>1) | ((dh&1)<<23);
            dh = dh>>1;
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
    bx += 0x3ff;            /* assemble final exponent                   */
    if (bx<=0)
    {   cc_rerr(fp_rerr_very_small);
        p->msd = 0;
        p->lsd = 0;
        return;
    }
    if (bx>0x7ff)
    {   cc_err(fp_err_very_big);
        bx = 0x7ff;             /* overflow -> 'infinity'                */
        dh = dm = dl = 0;
    }
    dh = ds | (bx<<20) | (dh & 0x000fffff);
    dm = (dm<<8) | ((dl>>16) & 0xff);
    p->msd = dh;
    p->lsd = dm;
    return;
}

bool fltrep_narrow(DbleBin *d, FloatBin *e)
/* this is now a function so that caller can report errors soon */
{   int32 hi = d->msd, lo = d->lsd;
    int32 x = (hi>>20) & 0x7ff;
    int32 m = hi & 0x000fffff;
    if (x==0)                   /* value is zero - treat specially       */
    {   e->val = 0;
        return 0;               /* OK */
    }
    else if (x==0x7ff)          /* value is 'infinity'.                  */
    {   e->val = (hi & 0x80000000UL) | 0x7f800000;
        return 0;
    }
    m = (m << 3) | ((lo >> 29) & 0x7);
    lo = lo << 3;
    if (lo < 0 && (lo!=0x80000000UL || (m & 1)!=0))
        if ((m += 1) == 0x00800000)     /* round up & renormalize        */
        {   m = 0;
            x += 1;
        }
    x = x - 0x3ff + 0x7f;
    if (x >= 0xff)
    {   cc_err(fp_err_big_single);
        e->val = (hi & 0x80000000UL) | (0xffL << 23);
        return 1;                /* overflow */
    }
    else if (x <= 0)
    {   cc_rerr(fp_rerr_small_single);
        e->val = (hi & 0x80000000UL) | (0L << 23);   /* preserve sign */
        return -1;               /* underflow */
    }
    e->val = (hi & 0x80000000UL) | (x << 23) | m;
    return 0;
}

bool fltrep_narrow_round(DbleBin *d, FloatBin *e)
{
    return fltrep_narrow(d, e);
}

void fltrep_widen(FloatBin *e, DbleBin *d)
{
    unsigned32 e0 = e->val;
    int32 x = (e0>>23) & 0xff;
    int32 s = e0 & 0x80000000UL;
    if (x==0)                   /* value is zero - preserve sign, why?   */
        d->msd = s, d->lsd = 0;
    else if (x==0xff)           /* value is 'infinity' - preserve sign.  */
        d->msd = s | 0x7ff00000, d->lsd = 0;
    else
        d->msd = s | ((x - 0x7f + 0x3ff) << 20) | ((e0 & 0x7fffff) >> 3),
        d->lsd = e0 << 29;
}

/* The following procedures implement IEEE format arithmetic without any */
/* reliance on the Acorn FPE. They exist here so that I can use this as  */
/* a cross-compiler, running on machines without floating point or with  */
/* floating point that does not agree with the ARM's version.            */

/* There is no support here for denormalized numbers, and infinities etc */
/* are viewed as error cases rather than as valid data.                  */

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
    unsigned32 guard=0, w;
    int32 bx, cx;
    int32 shift;
    bx = (bh >> 20) & 0x7ff;
    cx = (ch >> 20) & 0x7ff;
    shift = bx - cx;
    if (shift < -54 || bx == 0)
    {   a->msd = ch;
        a->lsd = cl;
        return 1;
    }
    if (shift > 54 || cx == 0)
    {   a->msd = bh;
        a->lsd = bl;
        return 1;
    }
    bh = (bh & 0xfffff) | 0x100000;
    ch = (ch & 0xfffff) | 0x100000;
/* Now I need to align the operands                                      */
    if (shift > 0)
    {   int32 rshift;
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
    {   int32 rshift;
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
#define ieee_carry(g, l) (g & 0x80000000UL) && ((l & 1)!=0 || g!=0x80000000UL)
    if (ieee_carry(guard, bl))
    {   if (bl == 0xffffffffUL)
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
    a->msd = (bh & ~0x00100000) | (bx << 20);
    a->lsd = bl;
    return 1;
}

static bool flt_difference(DbleBin *a, unsigned32 bh, unsigned32 bl,
                           unsigned32 ch, unsigned32 cl)
{
    unsigned32 bg=0, cg=0, w, resultsign;
    int32 bx, cx;
    int32 shift;
    bx = (bh >> 20) & 0x7ff;
    cx = (ch >> 20) & 0x7ff;
    shift = bx - cx;
    if (shift < -54 || bx == 0)
    {   a->msd = ch ^ 0x80000000UL;
        a->lsd = cl;
        return 1;
    }
    if (shift > 54 || cx == 0)
    {   a->msd = bh;
        a->lsd = bl;
        return 1;
    }
    bh = (bh & 0xfffff) | 0x100000;
    ch = (ch & 0xfffff) | 0x100000;
/* Now I need to align the operands                                      */
    if (shift > 0)
    {   int32 rshift;
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
    {   int32 rshift;
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
    bg = (bg >> 8) - (cg >> 8) + signed_rightshift_(w,8);
    cg = signed_rightshift_(bg,24);
    bg = (bg << 8) | ((int32)w & 0xff);
    w = (bl & 0xff) - (cl & 0xff) + cg;
    bl = (bl >> 8) - (cl >> 8) + signed_rightshift_(w,8);
    cg = signed_rightshift_(bl,24);
    bl = (bl << 8) | (w & 0xff);
    bh = bh - ch + cg;
/* Subtraction complete in 2s complement form.                           */
    if (bh & 0x80000000UL)  /* Sign of result must be negative             */
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
        resultsign = 0x80000000UL;
    }
    else resultsign = 0;
/* Subtraction now complete in sign & magnitude form                     */
    if (bh == 0 && bl == 0 && bg == 0)
    {   a->msd = a->lsd = 0;           /* Result is absolutely zero          */
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
            {   a->msd = a->lsd = 0;
                return 1;
            }
        }
        while ((bh & 0x001fe000) == 0)
        {   bh = (bh << 8) | (bl >> 24);
            bl = (bl << 8) | (bg >> 24);
            bg = bg << 8;
            bx -= 8;
            if (bx <= 0)
            {   a->msd = a->lsd = 0;
                return 1;
            }
        }
        while ((bh & 0x001c0000) == 0)
        {   bh = (bh << 3) | (bl >> 29);
            bl = (bl << 3) | (bg >> 29);
            bg = bg << 3;
            bx -= 3;
            if (bx <= 0)
            {   a->msd = a->lsd = 0;
                return 1;
            }
        }
        while ((bh & 0x00100000) == 0)
        {   bh = (bh << 1) | (bl >> 31);
            bl = (bl << 1) | (bg >> 31);
            bg = bg << 1;
            bx -= 1;
            if (bx <= 0)
            {   a->msd = a->lsd = 0;
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
    if (ieee_carry(bg, bl))
    {   if (bl == 0xffffffffUL)
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
    a->msd = (bh & ~0x00100000) | (bx << 20) | resultsign;
    a->lsd = bl;
    return 1;
}

bool flt_add(DbleBin *a, DbleBin *b, DbleBin *c)
/*   *a = *b + *c    */
{
    unsigned32 bh = b->msd;
    unsigned32 bl = b->lsd;
    unsigned32 ch = c->msd;
    unsigned32 cl = c->lsd;
    bool ok;
/* Do a case analysis on signs to end up with arithmetic on unsigned floats */
    if (bh & 0x80000000UL)
    {   if (ch & 0x80000000UL)
        {   ok = flt_sum(a, bh & 0x7fffffff, bl, ch & 0x7fffffff, cl);
            a->msd ^= 0x80000000UL;
        }
        else ok = flt_difference(a, ch, cl, bh & 0x7fffffff, bl);
    }
    else if (ch & 0x80000000UL)
        ok = flt_difference(a, bh, bl, ch & 0x7fffffff, cl);
    else ok = flt_sum(a, bh, bl, ch, cl);
    return ok;
}

bool flt_subtract(DbleBin *a, DbleBin *b, DbleBin *c)
/*  *a = *b - *c  */
{
    unsigned32 bh = b->msd;
    unsigned32 bl = b->lsd;
    unsigned32 ch = c->msd;
    unsigned32 cl = c->lsd;
    bool ok;
    if (bh & 0x80000000UL)
    {   if (ch & 0x80000000UL)
            ok = flt_difference(a, ch & 0x7fffffff, cl, bh & 0x7fffffff, bl);
        else
        {   ok = flt_sum(a, bh & 0x7fffffff, bl, ch, cl);
            a->msd ^= 0x80000000UL;
        }
    }
    else if (ch & 0x80000000UL)
        ok = flt_sum(a, bh, bl, ch & 0x7fffffff, cl);
    else ok = flt_difference(a, bh, bl, ch, cl);
    return ok;
}

bool flt_multiply(DbleBin *a, DbleBin *b, DbleBin *c)
/*  *a = *b * *c  */
{
    unsigned32 bh = b->msd;
    unsigned32 bl = b->lsd;
    unsigned32 ch = c->msd;
    unsigned32 cl = c->lsd;
    int32 ax, bx, cx, as;
    unsigned32 ah, al, carry;
    unsigned32 aa[7], bb[4], cc[4];
    int32 i, j;
    as = (bh ^ ch) & 0x80000000UL;    /* sign for result */
    bx = (bh >> 20) & 0x7ff;
    cx = (ch >> 20) & 0x7ff;
    if (bx==0 || cx==0)
    {   a->msd = as;    /* multiplication by 0.0         */
        a->lsd = 0;     /* note treatment of sign.       */
        return 1;
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
    {   unsigned32 w = aa[i] + carry;
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
    if (ieee_carry(carry, al))
    {   if (al == 0xffffffffUL)
        {   al = 0;
            ah += 1;
            if (ah & 0x00200000)
            {   ah = 0x00100000;
                ax += 1;
            }
        }
        else al += 1;
    }
    if (ax >= 0x7ff) return 0;      /* Overflow */
    else if (ax <= 0)
    {   a->msd = as;  /* N.B. keep sign on underflow     */
        a->lsd = 0;
        return 1;
    }
    a->msd = (ah & ~0x00100000) | (ax << 20) | as;
    a->lsd = al;
    return 1;
}

bool flt_divide(DbleBin *a, DbleBin *b, DbleBin *c)
/*  *a = *b / *c   */
{
    unsigned32 bh = b->msd;
    unsigned32 bl = b->lsd;
    unsigned32 ch = c->msd;
    unsigned32 cl = c->lsd;
    unsigned32 ah, al, as = (bh ^ ch) & 0x80000000UL;
    int32 ax, bx, cx, i;
    bx = (bh >> 20) & 0x7ff;
    cx = (ch >> 20) & 0x7ff;
    if (cx == 0) return 0;              /* division by zero              */
    if (bx == 0)
    {   a->msd = as;    /*  0.0 / anything  = 0.0        */
        a->lsd = 0;
        return 1;
    }
    bh = (bh & 0xfffff) | 0x00100000;
    ch = (ch & 0xfffff) | 0x00100000;
    ax = bx - cx + 0x3fe;
    ah = al = 0;
/* Do the division by test-and-subtract                                  */
    for (i = 0; i<55; i++)
    {   if (bh > ch || (bh == ch && bl >= cl))
        {   unsigned32 w = (bl & 0xff) - (cl & 0xff);
/* Do a double length subtraction (oh the carry is a misery)             */
            bl = (bl >> 8) - (cl >> 8) + signed_rightshift_(w,8);
            bh = bh - ch + signed_rightshift_(bl,24);
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
    if (ieee_carry(bh, al))
    {   if (al == 0xffffffffUL)
        {   al = 0;
            ah += 1;
            if (ah & 0x00200000)
            {   ah = 0x00100000;
                ax += 1;
            }
        }
        else al += 1;
    }
    if (ax >= 0x7ff) return 0;      /* Overflow on the division          */
    else if (ax <= 0)
    {   a->msd = as;  /* N.B. keep sign on underflow     */
        a->lsd = 0;
        return 1;
    }
    a->msd = (ah & ~0x00100000) | (ax << 20) | as;
    a->lsd = al;
    return 1;
}

int flt_compare(DbleBin *b, DbleBin *c)
/* (int32)sign(*b-*c)  */
{
/* Integer comparison of the 64bit values almost gives the answer I want */
    int32 bh = b->msd;
    unsigned32 bl = b->lsd;
    int32 ch = c->msd;
    unsigned32 cl = c->lsd;
/* +0.0 is equal to -0.0                                                 */
    if ((bh & 0x7ff00000)==0 && (ch & 0x7ff00000)==0) return 0;
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

bool flt_move(DbleBin *a, DbleBin *b)
/*   *a = *b         */
{
    a->msd = b->msd;
    a->lsd = b->lsd;
    return 1;
}

bool flt_negate(DbleBin *a, DbleBin *b)
/*   *a = - *b       */
{
    a->msd = b->msd ^ 0x80000000UL;
    a->lsd = b->lsd;
    return 1;                   /* can never fail                        */
}

#ifdef INT_TO_FLOAT_NEEDED

/* The following are not used in the Norcroft C compiler                 */

bool flt_itod(DbleBin *a, int32 n)
/*   *a = (double) n */
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
    ax = 0x400 + 51;
    while ((ah & 0x00100000)==0)
    {   ah = (ah << 1) | (al >> 31);
        al = al << 1;
        ax -= 1;
    }
    a->msd = (ah & ~0x00100000) | (ax << 20) | as;
    a->lsd = al;
    return 1;
}

bool flt_utod(DbleBin *a, unsigned n)
/*   *a = (double) u */
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
    ax = 0x400 + 53;
    while ((ah & 0x00100000)==0)
    {   ah = (ah << 1) | (al >> 31);
        al = al << 1;
        ax -= 1;
    }
    a->msd = (ah & ~0x00100000) | (ax << 20);
    a->lsd = al;
    return 1;
}

#endif

bool flt_dtoi(int32 *n, DbleBin *a)
/*   *n = (int32) *a   */
{
    unsigned32 ah, al;
    int32 sign, ax;
    ah = a->msd;
    al = a->lsd;
    if (ah & 0x80000000)
    {   ah &= ~0x80000000;
        sign = 1;
    }
    else sign = 0;
    ax = ah >> 20;
    ah = (ah & 0x000fffff) | 0x00100000;
    if (ax < 0x3fe)                     /* arg < 0.5 ... result 0        */
    {   *n = 0;
        return 1;
    }
    else if (ax > 0x400 + 51) return 0; /* overflow certainly            */
    while (ax != 0x400 + 51)
    {   al = (al >> 1) | (ah << 31);
        ah = ah >> 1;
        ax += 1;
    }
    if (sign)
    {   if (al > 0x80000000) return 0;
        al = -al;
    }
    else if (al >= 0x80000000) return 0;
    *n = al;
    return 1;
}

bool flt_dtou(unsigned32 *u, DbleBin *a)
/*   *u = (unsigned) *a */
{
    unsigned32 ah, al;
    int32 ax;
    ah = a->msd;
    al = a->lsd;
    ax = (ah >> 20) & 0x7ff;
    if ((ah & 0x80000000)!=0 && ax!=0) return 0;    /* negative nonzero  */
    ah = (ah & 0x000fffff) | 0x00100000;
    if (ax < 0x3fe)                     /* arg < 0.5 ... result 0        */
    {   *u = 0;
        return 1;
    }
    else if (ax > 0x400 + 51) return 0; /* overflow certainly            */
    while (ax != 0x400 + 51)
    {   al = (al >> 1) | (ah << 31);
        ah = ah >> 1;
        ax += 1;
    }
    *u = al;
    return 1;
}

/* end of mip/ieeeflt.c */

