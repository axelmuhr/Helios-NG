
/* m68k/unixins.c: copyright (C) Perihelion Software Ltd, 1988 */
/* Copyright (C) Codemist Ltd., 1989.                         */
/* Some Codemist changes to fix bugs and improve extref printing.       */
/* This version changed for UNIX style assembler                        */

#ifndef NO_VERSION_STRINGS
extern char unixins_version[];
char unixins_version[] = "\nm68k/unixins.c $Revision: 1.2 $ 20a\n";
#endif


#ifdef __STDC__
#  include <string.h>
#  include <stdarg.h>
#else
#  include <strings.h>
#endif
#include <ctype.h>

#include "globals.h"
#include "mcdep.h"
#include "mcdpriv.h"
#include "xrefs.h"
#include "store.h"
#include "codebuf.h"
#include "ops.h"
#include "version.h"

#define bitsperword 32

/* 68000 instruction formats */
typedef enum
     {  none=0,  dbit,   sbit,    movep,  ori,     andi,     subi, addi,
        eori,    cmpi,   move,    negx,   movefsr, clr,      neg,  move_ccr,
        not,     move_sr,nbcd,    pea,    swap,    movemfreg,tst,  tas,
        movem_reg,trap,  link,    unlk,   move_usp,movefusp, reset,nop,
        stop,    rte,    rts,     trapv,  rtr,     jsr,      jmp,  chk,
        lea,     addq,   subq,    scc,    dbcc,    bcc,      moveq,or,
        divu,    divs,   sbcd,    sub,    subx,    cmp,      cmpm, eor,
        and,     mulu,   muls,    abcd,   exgd,    exga,     exgm, add,
        addx,    dshift, mshift,  adda,   cmpa,    movea,    suba, ill,
        ori_ccr, ori_sr, andi_ccr,andi_sr,eori_ccr,eori_sr,  extw, extl,
/* 68010 instructions */
        moves_1, movefccr_1,rtd_1,movec_1,bkpt_1,
/* 68020 instructions */
       chm_2,    callm_2,rtm_2, cas_2, linkl_2, extbl_2, mulsl_2, divsl_2,
       trapcc_2, pack_2, unpk_2,bfld_2,fp_2
} instrenum ;

typedef unsigned long instrword;

static int32 pc, dec_off;
static char *outstring;
/* #ifdef TARGET_IS_68020 */
static void write_mulsl(instrword a);
static void write_divsl(instrword a);
static void write_fp_ins(instrword a);
/* #endif */

extern int32 bitreverse(int32 i);   /* imported from gen.c */

#ifdef __STDC__
static int32 asmprintf(const char *f,...)
{
   va_list a;
   int32 l;

   va_start(a, f);
   outstring += (l = vsprintf(outstring, f, a));
   va_end(a);
   return( l );
}
#else
static int32 asmprintf(f,a1,a2,a3,a4,a5)
char *f;
int32 a1, a2, a3, a4, a5;
{  int32 l;
/* This is still dodgy wrt the value that sprintf returns - BSD and SysV differ */
   outstring += (l = sprintf(outstring,f,a1,a2,a3,a4,a5));
   return l;
}
#endif

static void asmprintsym(Symstr *s)
{   /* this should really merge with pr_asmname -- see clipper back end.  */
    asmprintf("%s", symname_(s));
}

static int32 readword()
{
   int32 ins = code_hword_(dec_off);
   pc += 2;
   dec_off += 2;
   return ins;
}

static int32 readlong(void)
{  int32 a = readword();
   return (a<<16)+readword();
}

static void writereg(r)
int32 r;
{
   if( r > 7 ) asmprintf("a%d",r-8);
   else asmprintf("d%d",r);
}

static int32 str_padcol8(int32 n)
{   if (!annotations) n = 7;      /* compact the asm file */
    do {  asmprintf(" "), n++; } while( n < 8 );
    return n;
}

static int32 bits(instrword n, int32 a, int32 b)
{
   int32 aa, bb;
    aa = a; bb = b;
    if (a < b) {
        aa = b; bb = a;
      }
    n = n << (bitsperword - 1 - aa);
    return ( n >> (bitsperword - 1 - aa + bb));
}


static instrenum find_opcode (instrword a)
{
    switch ( bits(a, 15, 12))
    {

   case 0:
      switch (bits(a, 11, 8)) {
        case 14: return ((bits(a, 7, 6) == 3) ? cas_2 : moves_1);
        case 12: return ((bits(a, 7, 6) == 3) ? cas_2 : cmpi);
        case 10: if (bits(a, 7, 6) ==3) return cas_2;
                 else { if (bits(a, 5, 0) == 0X3C)
                           switch (bits(a, 7, 6)) {
                              case 0: return (eori_ccr);
                              case 1: return (eori_sr);
                              }
                        else return (eori);
                     }
        case  8: return (sbit);
        case  6: if (bits(a, 7, 6) == 3)
                   return ((bits(a, 5, 4) == 0) ? rtm_2 : callm_2);
                 else
                   return (addi);
        case  4: return ((bits(a, 7, 6) == 3) ? chm_2 : subi);
        case  2: if (bits(a, 7, 6) ==3) return (chm_2);
                 else { if (bits(a, 5, 0) == 0X3C)
                           switch (bits(a, 7, 6)) {
                              case 0: return (andi_ccr);
                              case 1: return (andi_sr);
                              }
                        else return (andi);
                     }
        case  0: if (bits(a, 7, 6) ==3) return (chm_2);
                 else { if (bits(a, 5, 0) == 0X3C)
                           switch (bits(a, 7, 6)) {
                              case 0: return (ori_ccr);
                              case 1: return (ori_sr);
                              }
                        else return (ori);
                     }
        default: if ( (bits(a, 5, 3) == 1) &&  bits(a, 8, 6) <= 7
                     && bits(a, 8, 6) >= 4 )
                 return (movep);
                 if ( bits(a, 8, 8) == 1) return (dbit);
                 return (none);
    }

    case  1:case 2:case 3: return ((bits(a, 8, 6) == 1) ? movea : move);
    case  4: switch (bits(a, 11, 8)) {
        case  0: if (bits(a, 7, 6) == 3)
                    return (movefsr);
                 else
                    return (negx);
        case  2: return (bits(a, 7, 6) == 3) ? movefccr_1 : clr;
        case  4: if (bits(a, 7, 6) == 3)
                     return move_ccr;
                 else
                     return neg;
        case  6: if (bits(a, 7, 6) == 3)
                     return move_sr;
                 else
                     return not;
        case  8:
            switch (bits(a, 7, 6))
            {
            case  0: return (bits(a, 5, 3) == 1) ? linkl_2 : nbcd;
            case  1: switch (bits(a, 5, 3)) {
                     case 0 :return swap;
                     case 1 :return bkpt_1;
                     default :return pea;
                     }
            case  2: if (bits(a, 5, 3) == 0)
                        return extw;
                     else return movemfreg;
            case  3: if ((bits(a, 5, 3)) == 0)
                        return extl;
                     else
                        return (movemfreg);
            }

        case 10: if (bits(a, 7, 6) == 3)
                    return (bits(a, 5, 0) == 0X3C) ? ill : tas;
                 else return tst;
        case 12: if (bits(a, 7, 7) == 1)
                    return movem_reg;
                 else {
                    if (bits(a,6,6) == 0)
                       return mulsl_2;
                    else
                       return divsl_2;
                  }
        case 14:
        switch (bits(a, 7, 3)) {
            case  1: return link;
            case  8:               /* fall through */
            case  9: return trap;
            case 10: return link;
            case 11: return unlk;
            case 12: return move_usp;
            case 13: return movefusp;
            case 14: switch (bits(a, 2, 0)) {
                case 0: return reset;
                case 1: return nop;
                case 2: return stop;
                case 3: return rte;
                case 4: return rtd_1;
                case 5: return rts;
                case 6: return trapv;
                case 7: return rtr;
            }
            case 15: return movec_1;
            default: if (bits(a, 7, 6) == 2)
                         return jsr;
                     if (bits(a, 7, 6) == 3)
                         return jmp;
                     return none;
        }
        default: if (bits(a, 8, 6) == 6)
                     return chk;
                 if ((a & 0xfff8) == 0x49c0)
                     return extbl_2;
                 if (bits(a, 8, 6) == 7)
                     return lea;
                 return none;
    }

    case  5: if (bits(a, 7, 3) == 25)
                 return dbcc;
             if (bits(a, 7, 6) == 3) {
                 if ( bits(a, 5, 0) <= 074 && bits(a, 5, 0) >= 072 )
                      return trapcc_2;
                 else
                      return scc;
             }
             if (bits(a, 8, 8) == 0)
                 return addq;
             else return subq;

    case  6:  return bcc;

    case  7: if (bits(a, 8, 8) == 0)
                 return moveq;
             else return none;

    case  8: switch (bits(a, 8, 4))  {
                 case 16: return sbcd;
                 case 20: return pack_2;
                 case 24: return unpk_2;
             }
             switch (bits(a, 8, 6)) {
                 case 7: return divs;
                 case 3:return divu;
                 default: return or;
             }

    case  9: if (bits(a, 5, 4) == 0 && bits(a, 8, 8) == 1 &&
                     bits(a, 7, 6) <= 2 && bits(a, 7, 6) >= 0)
                 return subx;
             if (bits(a, 8, 6) == 3 || bits(a, 8, 6) == 7)
                 return suba;
             return sub;

    case 11: if (bits(a, 8, 8) == 1 && bits(a, 5, 3) == 1 &&
                     bits(a, 7, 6) <= 2 && bits(a, 7, 6) >= 0)
                 return cmpm;
             if (bits(a, 8, 6) <= 6 && bits(a, 8, 6) >= 4)
                 return eor;
             if (bits(a, 8, 6) == 3 || bits(a, 8, 6) == 7)
                 return cmpa;
             return cmp;

    case 12: switch (bits(a, 8, 6)) {
        case  3: return mulu;
        case  7: return muls;
        case  4: if (bits(a, 5, 4) == 0) 
                     return abcd;
                 else
                     return  and;
        case  5: if (bits(a, 5, 3) == 0)
                     return exgd;
                 if (bits(a, 5, 3) == 1)
                     return exga;
                 return and;
        case  6: if (bits(a, 5, 3) == 1)
                 return exgm;
        default: return and;
    }

    case 13: if ((bits(a, 8, 8) == 1) && bits(a, 5, 4) == 0 &&
                     bits(a, 7, 6) <= 2 && bits(a, 7, 6) >= 0)
                 return addx;
             if (bits(a, 8, 6) == 3 || bits(a, 8, 6) == 7)
                 return adda;
             return add;

    case 14: if (bits(a, 7, 6) == 3) {
                if (bits(a, 11, 11) == 0)
                    return mshift;
                else
                    return bfld_2;
             }
             else
                return dshift;

    case 15: return fp_2;
  }
  return (instrenum) 0; /* Artificial (never executed) */
}

static int32 short_value(int32 n)
{
    return ((n == 0) ? 8 : n);
}

static int32 byte_extend(int32 n)
{
    return ((n > 127) ? (n | ~0XFF) : n);
}

static int32 word_extend(int32 n)
{
    return ((n & 0x8000) != 0 ? (n | 0xffff0000) : n);
}

static int32 op_size(int32 op, instrword a, int32 w )
{
    switch (op)
    {
case ori:
case andi:
case subi:
case addi:
case eori:
case cmpi:
case negx:
case clr:
case neg:
case not:
case tst:
case addq:
case subq:
case or:
case sub:
case subx:
case cmp:
case cmpm:
case eor:
case and:
case add:
case addx:
case dshift:
        return bits(a, 7, 6);

case move:
        switch (bits(a, 13, 12))
        {
    case 1: return 0;
    case 2: return 2;
    case 3: return 1;
         }
case movea:
        return (bits(a, 13, 12) == 3) ? 1 : 2;

case adda:
case suba:
case cmpa:
        return (bits(a, 8, 6) == 3) ? 1 : 2;

case movem_reg:
case movemfreg:
case movep:
        return (bits(a, 6, 6) == 0) ? 1 : 2;

case fp_2:    /* This case w = 2nd word of instr */
        switch( bits(w, 12, 10) )
        {
    case 0:  return 2;
    case 1:  return 2;
    case 2:  return 4;
    case 3:  return 4;
    case 4:  return 1;
    case 5:  return 3;
    case 6:  return 4;
        } 

case mulsl_2:
case divsl_2:
        return 2;

default:
        return 1;
    }
}

static char *condition_code(instrenum op, instrword a)
{
    switch (bits(a, 11, 8))
    {
case  0: return (op == bcc ? "ra": "t");
case  1: return (op == bcc ? "sr": "ra");
case  2: return "hi";
case  3: return "ls";
case  4: return "cc";
case  5: return "cs";
case  6: return "ne";
case  7: return "eq";
case  8: return "vc";
case  9: return "vs";
case 10: return "pl";
case 11: return "mi";
case 12: return "ge";
case 13: return "lt";
case 14: return "gt";
case 15: return "le";
    }
    return(0); /* Artificial (never executed) */
}

static void write_op(char *instruction, int32 n)
{
   int32 s;
   asmprintf("%s", instruction );
   s = strlen(instruction);
   if ( n == 0  ) asmprintf("b");
   if ( n == 1  ) asmprintf("w");
   if ( n == 2  ) asmprintf("l");
   if ( n >= 0 )  s++ ;
   str_padcol8(s);
}

static void w_index(int32 a, int32 reg, int32 l, int32 scl)
{
   asmprintf((a == 1  ?  "a%d" : "d%d"), reg);
   if (l == 1) asmprintf(":l");
   if (scl != 0)
      asmprintf(":%d", (1 << scl) );
}

static void mode6orpc3(int32 arg, int32 mode)
{
   int32 b = readword();
   int32 d = b & 0xff;
   int32 scl = bits(b,10,9);
   int32 reg = bits(b,14,12);
   int32 a   = bits(b,15,15);
   int32 l   = bits(b,11,11);
   int32 pcmode = mode == 7;
   int32 pcx = pc - 2;
   int32 iis, bds, is, bs;

    if ( bits(b,8,8) == 0 )
    {  if ( d > 127 )
          d = byte_extend(d);
       if (pcmode) asmprintf("pc($%04x,", d+pcx);
       else asmprintf("a%d@(0x%x,", arg, d);
       w_index(a,reg,l,scl);
       asmprintf(")");
    }
    else {
      iis = bits(b,2,0);
      bds = bits(b,5,4);
      is  = bits(b,6,6);
      bs  = bits(b,7,7);

      asmprintf((pcmode ? "pc" : "a%1d"), arg);
      asmprintf("(");

      if (iis != 0) asmprintf("[");
      if (bds != 1) {
          if (bds == 3) asmprintf("$%x,", readlong() + (pcmode ? pcx : 0) );
          else asmprintf( (pcmode ? "%04x," : "%d,"),
			  readword() + (pcmode ? pcx : 0) );
       }

       if (bs == 1) asmprintf("Z");

       if (iis <= 7 && iis >= 5) asmprintf("]");

       if (is == 0) {
          asmprintf("0,");
          w_index(a,reg,l,scl);
       }

       if (iis <= 3 && iis >= 1) asmprintf("]");

       if ( bits(is,1,0) > 1 ) {
          if (bits(is,1,0) == 3)
             asmprintf("%08x", readlong());
          else
             asmprintf("%d", readword());
       }

       if (iis != 0) asmprintf("]");
       asmprintf(")");
    }
}

static void ea(int32 op, instrword a, int32 w)
{
    int32 mode = bits(a, 5, 3);
    int32 arg = bits(a, 2, 0);
    int32 size;

    switch (mode) {
        case 0: asmprintf("d%d", arg);
                break;
        case 1: asmprintf("a%d", arg);
                break;
        case 2: asmprintf("a%d@", arg);
                break;
        case 3: asmprintf("a%d@+", arg);
                break;
        case 4: asmprintf("a%d@-", arg);
                break;
        case 5: asmprintf("a%d@(0x%x)", arg, word_extend(readword()));
                break;
        case 6: mode6orpc3(arg,mode);
                break;
        case 7:
        switch (arg) {
            case 0: /* absolute short */
                    asmprintf("$%04x", readword());
                    break;
            case 1: /* absolute long (e.g. extref) */
                {   Symstr *s = decode_external(pc);
                    int32 w = readlong();
                    if (s == 0) asmprintf("$%08lx", (long)w);
                    else
                    {   asmprintsym(s);
                        if (w!=0) asmprintf("+$%lx", (long)w);
                    }
                }
                break;
            case 2: /* PC indirect with disp */
                {   int32 opc = pc;
                    asmprintf("$%04x",
                      (word_extend(readword()) + opc) & 0XFFFFFF);
                }
                break;

/* PC mode 3 not complete for 020 yet */

            case 3: mode6orpc3(arg,mode);
                    break;

            case 4: { size = op_size(op, a, w);
                      switch( size )
                      { int32 a,b,c;

                      case 0:
                      case 1:
                        asmprintf("#%d", readword()); break;
                      case 2:
                        asmprintf("#$%08x", readlong() ); break;
                      case 3:
                        a = readlong();
                        b = readlong();
                        asmprintf("#$%08x%08x", a, b);
                        break;
                      case 4:
                        a = readlong();
                        b = readlong();
                        c = readlong();
                        asmprintf("#$%08x%08x%08x", a, b, c);
                        break;
                      }
                      break;
                    }
           default: asmprintf("???");
        }
    }
}

static void write_opi(char *instruction, int32 op, instrword a)
{
    int32 size;
    size = op_size(op, a, 0);
    write_op(instruction, size);
    if (size == 0)  asmprintf("#%d,", readword());
    if (size == 1)  asmprintf("#%d,", readword());
    if (size == 2)  asmprintf("#$%08X,", readlong());
    ea(op, a, 0);
}

static void register_list(int32 m, void (*preg)(int32) )
{
   int32 i,freg = 0;
   int32 state = 0;
   int32 list_started = 0;

   for( i = 0; i <= 16; i++)     /* 16 to flush A7 if set */
   {  int32 x;

      if( (x = bits(m,i,i)) != state )
      {
         state = x;
         if(state)
         {  if( list_started )
               asmprintf("/");
            preg(i);
            freg = i;               /* Record first reg in list */
            list_started = 1;
         }
         else
         {
            if( (i-1) != freg )
            {
               asmprintf("-");
               preg(i-1);
            }
         }
      }
   }
}

void spr_pcrel(char *, int32, int32);

static void write_instruction(instrenum op, instrword a)
{   int32 dir, addr, pcx, b, mode, val;
    char s[10];
    char *type, *str, *sh_op, *condition;

    switch (op) {
default:
case none:
    write_op("?DC", 1);
    asmprintf("$%04x", a);
    break;

    case abcd:
       write_op("ABCD", -1);
       if (bits(a, 3, 3) == 0)
          asmprintf("-a%d@,-a%d@", bits(a, 2, 0), bits(a, 11, 9));
       else
          asmprintf("d%d,d%d", bits(a, 2, 0), bits(a, 11, 9));
    break;

    case add:
    write_op("add", bits(a, 8, 6));
    if (bits(a, 8, 6) <= 2 && bits(a, 8, 6) >= 0)
    {  ea(op, a, 0);
       asmprintf(",d%d", bits(a, 11, 9));
    }
    if (bits(a, 8, 6) <= 6 && bits(a, 8, 6) >= 4)
    {  asmprintf("d%d,", bits(a, 11, 9));
        ea(op, a, 0);
    }
    break;

    case adda:
    write_op("add", op_size(op, a, 0));
    ea(op, a, 0);
    asmprintf(",a%d", bits(a, 11, 9));
    break;

    case addi:
    write_opi("add",op,  a);
    break;

    case addq:
    write_op("addq", bits(a, 7, 6));
    asmprintf("#%d,", short_value(bits(a, 11, 9)));
    ea(op, a, 0);
    break;

    case addx:
    write_op("ADDX", bits(a, 7, 6));
    if (bits(a, 3, 3) == 0)
       asmprintf("-a%d@,-a%d@", bits(a, 2, 0), bits(a, 11, 9));
    else asmprintf("d%d,d%d", bits(a, 2, 0), bits(a, 11, 9));
    break;

    case and:
    write_op("AND", bits(a, 8, 6));
    if (bits(a, 8, 6) <= 2 && bits(a, 8, 6) >= 0)
    {  ea(op, a, 0);
        asmprintf(",d%d", bits(a, 11, 9));
    }
    if (bits(a, 8, 6) <= 6 && bits(a, 8, 6) >= 4)
    {  asmprintf("d%d,", bits(a, 11, 9));
        ea(op, a, 0);
    }
    break;

    case andi:
    write_opi("and",op,  a);
    break;

    case andi_sr:         /* not complete */
    asmprintf("ANDI_SR");
    break;

    case andi_ccr:        /* not complete */
    asmprintf("ANDI_CCR");
    break;


    case dshift:
    {   dir = bits(a, 8, 8);
        switch (bits(a, 4, 3))
        {   default:
            case 0: sh_op =  ((dir == 0 ) ?  "asr" : "asl"); break;
            case 1: sh_op =  ((dir == 0 ) ?  "lsr" : "lsl"); break;
            case 2: sh_op =  ((dir == 0 ) ?  "roxr": "roxl"); break;
            case 3: sh_op =  ((dir == 0 ) ?  "ror" : "rol");break;
        }
        write_op(sh_op, bits(a, 7, 6));
        if( bits(a, 5, 5) == 0 )
           asmprintf("#%d,d%d", short_value(bits(a, 11, 9)), bits(a, 2, 0));
        else asmprintf("d%d,d%d", bits(a, 11, 9), bits(a, 2, 0));
        break;
    }

    case mshift:
    {   dir = bits(a, 8, 8);
        switch (bits(a, 10, 9))
        {   default:
            case 0: sh_op =  ((dir == 0 ) ?  "asr" : "asl"); break;
            case 1: sh_op =  ((dir == 0 ) ?  "lsr" : "lsl"); break;
            case 2: sh_op =  ((dir == 0 ) ?  "roxr": "roxl"); break;
            case 3: sh_op =  ((dir == 0 ) ?  "ror" : "rol"); break;
        }
        write_op(sh_op, -1);
        ea(op, a, 0);
        break;
    }

    case scc:
    {   condition = condition_code(op, a);

        strcpy(s,"s");
        strcat(s,condition);
        write_op(s,-1);
        ea(op,a, 0);
        break;
    }

    case bcc:
    case dbcc: {
        addr = bits(a, 7, 0);
        pcx = pc;
        condition = condition_code(op, a);

        if (op == bcc );
           strcpy(s,"b");
        if (op == dbcc ) 
           strcpy(s,"db");
        strcat(s,condition);
        if (op == bcc)
        { if (addr > 127 )  addr = byte_extend(addr);
           if (addr != 0)
              strcat(s,"s");
        }
        else addr = 0;

        write_op(s,-1);

        if (addr == 0 )  addr = word_extend(readword());

        if (addr == 0XFF)
           addr = readlong();
				/* Decode this as a label */
	{
	  char buff[80];
	  spr_pcrel(buff, addr, pcx);
	  asmprintf(buff);
	}
        break;
    }

    case dbit:
    {  switch (bits(a, 7, 6))
        {   default:
            case 0: type = "btst"; break;
            case 1: type = "BCHG"; break;
            case 2: type = "BCLR"; break;
            case 3: type = "bset"; break;
        }
        write_op(type, -1);
        asmprintf("d%d,", bits(a, 11, 9));
        ea(op, a, 0);
        break;
    }

    case sbit:
    {  b = readword();
       switch (bits(a, 7, 6))
        {   default:
            case 0: type = "btst"; break;
            case 1: type = "BCHG"; break;
            case 2: type = "BCLR"; break;
            case 3: type = "bset"; break;
        }
        write_op(type, -1);
        asmprintf("#%d,", b);
        ea(op, a, 0);
        break;
    }

    case chk:
    write_op("CHK", -1);
    ea(op, a, 0);
    asmprintf(",d%d", bits(a, 11, 9));
    break;

    case clr:
    write_op("clr", bits(a, 7, 6));
    ea(op, a, 0);
    break;

    case cmp:
    write_op("cmp", bits(a, 8, 6));
    ea(op, a, 0);
    asmprintf(",d%d", bits(a, 11, 9));
    break;

    case cmpa:
    write_op("cmp", op_size(op, a, 0));
    ea(op, a, 0);
    asmprintf(",a%d", bits(a, 11, 9));
    break;

    case cmpi:
    write_opi("cmp",op,  a);
    break;

    case cmpm:
    write_op("CMPM", bits(a, 7, 6));
    asmprintf("(A%d)+,(A%d)+", bits(a, 2, 0), bits(a, 11, 9));
    break;

    case divs:
    write_op("divs", -1);
    ea(op, a, 0);
    asmprintf(",d%d", bits(a, 11, 9));
    break;

    case divu:
    write_op("DIVU", -1);
    ea(op, a, 0);
    asmprintf(",d%d", bits(a, 11, 9));
    break;

    case eor:
    write_op("eor", bits(a, 8, 6));
    asmprintf("d%d,", bits(a, 11, 9));
    ea(op, a, 0);
    break;

    case eori:
    write_opi("eor",op,  a);
    break;

    case eori_ccr:         /* notcomplete */
    asmprintf("EORI_CCR");
    break;

    case eori_sr:         /* notcomplete */
    asmprintf("EORI_CCR");
    break;

    case exgm:
    case exgd:
    case exga:
    {   mode = bits(a, 7, 3);
        str = "d%d,d%d";
        write_op("EXG", -1);
        if (mode == 9 )  str = "a%d,a%d";
        if (mode == 17 )  str = "d%d,a%d";
        asmprintf(str, bits(a, 11, 9), bits(a, 2, 0));
        break;
    }

    case extbl_2:
    case extw:
    case extl:
    if( op == extbl_2 )
       write_op("EXTBL",2);
    else
       write_op("EXT", bits(a, 8, 6) == 2  ?  1 : 2);
    writereg(bits(a, 2, 0));
    break;


    case ill:
    write_op("ILLEGAL", -1);
    break;

    case jmp:
    write_op("jmp", -1);
    ea(op, a, 0);
    break;

    case jsr:
    write_op("jsr", -1);
    ea(op, a, 0);
    break;

    case lea:
    write_op("lea", -1);
    ea(op, a, 0);
    asmprintf(",a%d", bits(a, 11, 9));
    break;

    case link:
    {  int32 off = bits(a,7,3) == 1 ? readlong() : word_extend(readword());
       write_op("link", -1);
       asmprintf("a%d,#%d", bits(a, 2, 0), off);
    }
    break;

    case move:
    {  b = (bits(a, 8, 6) << 3) | bits(a, 11, 9) | (bits(a, 15, 12) << 12);
        write_op("mov", op_size(op, a, 0));
        ea(op, a, 0);
        asmprintf(",");
        ea(op, b, 0);
        break;
    }

    case movea:
    write_op("mov", op_size(op, a, 0));
    ea(op, a, 0);
    asmprintf(",a%d", bits(a, 11, 9));
    break;

    case move_ccr:
    write_op("move", -1);
    ea(op, a, 0);
    asmprintf(",CCR");
    break;

    case move_sr:
    write_op("move", -1);
    ea(op, a, 0);
    asmprintf(",SR");
    break;

    case movefsr:
    write_op("move", -1);
    asmprintf("SR,");
    ea(op, a, 0);
    break;

    case move_usp:
    case movefusp:
    write_op("move", -1);
    if ( bits(a, 3, 3) == 0 )
       asmprintf("a%d,USP", bits(a, 2, 0));
    else asmprintf("USP,a%d", bits(a, 2, 0));
    break;

    case movemfreg:
    case movem_reg:
    write_op("movem", bits(a, 6, 6) == 0  ?  1 : 2);
    if (  bits(a, 10, 10) == 0 )
    {   register_list( (bits(a,5,3) == 4)? bitreverse(readword()):
                                             readword(), writereg);
        asmprintf(",");
        ea(op, a, 0);
    }
    else
    {  ea(op, a, 0);
       asmprintf(",");
       register_list(readword(), writereg);
    }
    break;

    case movep:
    write_op("MOVEP", bits(a, 6, 6) == 0  ?  1 : 2);
    if ( bits(a, 7, 7))
       asmprintf("a%d@(0x%x),d%d", bits(a, 2, 0), readword(), bits(a, 11, 9));
    else asmprintf("d%d,a%d@(0x%x)", bits(a, 11, 9), bits(a, 2, 0), readword());
    break;

    case moveq:
    {   val = a & 0XFF;
        if (val > 127 )  val = byte_extend(val);
        write_op("moveq", -1);
        asmprintf("#%d,d%d", val, bits(a, 11, 9));
        break;
    }

    case muls:
    write_op("muls", -1);
    ea(op, a, 0);
    asmprintf(",d%d", bits(a, 11, 9));
    break;

    case mulu:
    write_op("mulu", -1);
    ea(op, a, 0);
    asmprintf(",d%d", bits(a, 11, 9));
    break;

    case nbcd:
    write_op("NBCD", -1);
    ea(op, a, 0);
    break;

    case neg:
    write_op("neg", bits(a, 7, 6));
    ea(op, a, 0);
    break;

    case negx:
    write_op("NEGX", bits(a, 7, 6));
    ea(op, a, 0);
    break;

    case nop:
    asmprintf("nop");
    break;

    case not:
    write_op("not", bits(a, 7, 6));
    ea(op, a, 0);
    break;

    case or:
    write_op("or", bits(a, 7, 6));
    if (bits(a, 8, 6) <= 2 && bits(a, 8, 6) >= 0)
    {  ea(op, a, 0);
        asmprintf(",d%d", bits(a, 11, 9));
    }
    if (bits(a, 8, 6) <= 6 && bits(a, 8, 6) >= 4)
    {  asmprintf("d%d,", bits(a, 11, 9));
        ea(op, a, 0);
    }
    break;

    case ori:
    write_opi("or", op,  a);
    break;


    case ori_sr:        /* not complete */
    asmprintf("ORI_SR");
    break;


    case ori_ccr:       /* not complete */
    asmprintf("ORI_CCR");
    break;

    case pea:
    write_op("pea", -1);
    ea(op, a, 0);
    break;

    case reset:
    asmprintf("RESET");
    break;

    case rtd_1:
    write_op("RTD", -1);
    asmprintf("#%d", word_extend(readword()));
    break;

    case rte:
    asmprintf("rte");
    break;

    case rtr:
    asmprintf("rtr");
    break;

    case rts:
    asmprintf("rts");
    break;

    case sbcd:
    write_op("SBCD", -1);
    if ( bits(a, 3, 3) == 0 )
        asmprintf("-a%d@,-a%d@", bits(a, 2, 0), bits(a, 11, 9));
        else asmprintf("d%d,d%d", bits(a, 2, 0), bits(a, 11, 9));
    break;

    case stop:
    asmprintf("stop");
    break;

    case sub:
    write_op("sub", bits(a, 8, 6));
    if (bits(a, 8, 6) <= 2 && bits(a, 8, 6) >= 0)
    {  ea(op, a, 0);
       asmprintf(",d%d", bits(a, 11, 9));
    }
    if (bits(a, 8, 6) <= 6 && bits(a, 8, 6) >= 4)
    {  asmprintf("d%d,", bits(a, 11, 9));
       ea(op, a, 0);
    }
    break;

    case suba:
    write_op("sub", op_size(op, a, 0));
    ea(op, a, 0);
    asmprintf(",a%d", bits(a, 11, 9));
    break;

    case subi:
    write_opi("sub", op, a);
    break;

    case subq:
    write_op("subq", bits(a, 7, 6));
    asmprintf("#%d,", short_value(bits(a, 11, 9)));
    ea(op, a, 0);
    break;

    case subx:
    write_op("SUBX", bits(a, 7, 6));
    if ( bits(a, 3, 3) == 0 )
       asmprintf("-a%d@,-a%d@", bits(a, 2, 0), bits(a, 11, 9));
    else asmprintf("d%d,d%d", bits(a, 2, 0), bits(a, 11, 9));
    break;

    case swap:
    write_op("swap", -1);
    asmprintf("d%d", bits(a, 2, 0));
    break;

    case tas:
    write_op("TAS", -1);
    ea(op, a, 0);
    break;

    case trap:
    write_op("trap",-1);
    asmprintf("#%d", a & 0XF);
    break;

    case trapv:
    write_op("TRAPV", -1);
    break;

    case tst:
    write_op("tst", bits(a, 7, 6));
    ea(op, a, 0);
    break;

    case unlk:
    write_op("unlk", -1);
    asmprintf("a%d", bits(a, 2, 0));
    break;

    case mulsl_2:
    write_mulsl(a);
    break;

    case divsl_2:
    write_divsl(a);
    break;

    case fp_2:
    write_fp_ins(a);
    break;
    }
}

static void write_opf(char *op, int32 w, int32 ss)
{
   char *opts = "lsxpwdb";
   int32 l;

   l = asmprintf("%s",op);
   asmprintf("%c", ((w & 0x4000) == 0)? *(opts+2): *(opts+ss) );

   str_padcol8(l+1);
}

static void fpgen0(instrword a, int32 w)
{
   int32 argtype = 3;
   int32 s = bits(w,12,10);
   int32 d = bits(w,9,7);
   int32 m = bits(w,14,14);
   char *insstr;

   if( ( s == 7 ) && ( m == 1 ) )
   {
      write_opf("FMOVECR",w,0);
      asmprintf("#$%02x,FP%d", w & 0x3f, bits(w,9,7));
      return;
   }
   else
   {
      switch( w & 0x7f )
      {
      case 0x00: argtype = 2; insstr = "FMOVE"; break;
      case 0x02: insstr = "FSINH"; break;
      case 0x03: insstr = "FINTRZ"; break;
      case 0x04: insstr = "FSQRT"; break;
      case 0x06: insstr = "FLOGNP1"; break;
      case 0x08: insstr = "FETOXM1"; break;
      case 0x09: insstr = "FTANH"; break;
      case 0x0a: insstr = "FATAN"; break;
      case 0x0c: insstr = "FASIN"; break;
      case 0x0d: insstr = "FATANH"; break;
      case 0x0e: insstr = "FSIN"; break;
      case 0x0f: insstr = "FTAN"; break;
      case 0x10: insstr = "FETOX"; break;
      case 0x11: insstr = "FTWOTOX"; break;
      case 0x12: insstr = "FTENTOX"; break;
      case 0x14: insstr = "FLOGN"; break;
      case 0x15: insstr = "FLOG10"; break;
      case 0x16: insstr = "FLOG2"; break;
      case 0x19: insstr = "FCOSH"; break;
      case 0x1a: insstr = "FNEG"; break;
      case 0x1c: insstr = "FACOS"; break;
      case 0x1e: insstr = "FGETEXP"; break;
      case 0x1f: insstr = "FGETMAN"; break;
      case 0x20: argtype = 2; insstr = "FDIV"; break;
      case 0x21: argtype = 2; insstr = "FMOD"; break;
      case 0x22: argtype = 2; insstr = "FADD"; break;
      case 0x23: argtype = 2; insstr = "FMUL"; break;
      case 0x24: argtype = 2; insstr = "FSGLDIV"; break;
      case 0x25: argtype = 2; insstr = "FREM"; break;
      case 0x26: argtype = 2; insstr = "FSCALE"; break;
      case 0x27: argtype = 2; insstr = "FSGLMUL"; break;
      case 0x28: argtype = 2; insstr = "FSUB"; break;
      case 0x30: argtype = 4; insstr = "FSINCOS"; break;
      case 0x31: argtype = 4; insstr = "FSINCOS"; break;
      case 0x32: argtype = 4; insstr = "FSINCOS"; break;
      case 0x33: argtype = 4; insstr = "FSINCOS"; break;
      case 0x34: argtype = 4; insstr = "FSINCOS"; break;
      case 0x35: argtype = 4; insstr = "FSINCOS"; break;
      case 0x36: argtype = 4; insstr = "FSINCOS"; break;
      case 0x37: argtype = 4; insstr = "FSINCOS"; break;
      case 0x38: argtype = 2; insstr = "FCMP"; break;
      case 0x3a: argtype = 1; insstr = "FTST"; break;
      case 0x3d: argtype = 2; insstr = "FCOS"; break;
      default:
         asmprintf("????");
         return;
      }
   }
   write_opf(insstr,w,s);

   if( m != 0 )
      ea( fp_2, a, w);
   else
      asmprintf("FP%d", s);

   switch( argtype )
   {
   case 1:
      break;

   case 2:
      asmprintf(",FP%d", d);
      break;

   case 3:
      if( (m != 0) || ( (m == 0) && (s != d) ) )
         asmprintf(",FP%d",d);
      break;

   case 4:
      asmprintf(",FP%d:FP%d",bits(w,2,0),d);
      break;
   }
}

static void writefpcr(int32 n)
{
   char s[15];
   bool started = 0;

   s[0] = 0;
   if( n & 1 )
   {  strcat(s,"FPIAR");
      started = 1;
   }
   if( n & 2 )
   {  if( started ) strcat(s,"/");
      else started = 1;

      strcat(s,"FPSR");
   }

   if( n & 4 )
   {  if( started ) strcat(s,"/");
      else started = 1;

      strcat(s,"FPCR");
   }
   asmprintf(s);
}

static void fpmovefpcr(instrword a, int32 w)
{
   int32 dir = bits(w,13,13);
   int32 fpr = bits(w,12,10);

   write_op("FMOVE",2);

   if( dir == 1 )
   {
      writefpcr(fpr);
      asmprintf(",");
      ea(fp_2,a, w);
   }
   else
   {
      ea(fp_2,a, w);
      asmprintf(",");
      writefpcr(fpr);
   }
}

static void fpmovefromfp(instrword a, int32 w)
{
   int32 s = bits(w,9,7);
   int32 d = bits(w,12,10);
   int32 k = bits(w,6,0);

   write_opf("FMOVE",w,d);

   asmprintf("FP%d,",s);
   ea(fp_2,a, w);

   if( d == 3 )
      asmprintf("{#%d}",k);

   if( d == 7 )
      asmprintf("{d%d}",bits(k,6,4));
}

static void writefpreg(int32 n)
{
   asmprintf("FP%d",n);
}

static void fpmovem(instrword a, int32 w)
{
   int32 rlist = bits(w,7,0);           /* AM changed 6 to 7 Aug 89 (check) */
   int32 mode  = bits(w,12,11);
   int32 dir   = bits(w,13,13);
   int32 dreg  = bits(w,6,4);

   write_op("FMOVEM.X",-1);

   switch( mode )
   {
   case 0:
   case 2:
      if( mode == 2 ) rlist = bitreverse(rlist) >> 8;
      if( dir == 1 )
      {
         register_list( rlist, writefpreg);
         asmprintf(",");
         ea(fp_2, a, 0);
      }
      else
      {
         ea(fp_2, a, 0);
         asmprintf(",");
         register_list( rlist, writefpreg);
      }
      break;

   case 3:
   case 1:
      if( dir == 1 )
      {
         asmprintf("d%d,",dreg);
         ea(fp_2, a, 0);
      }
      else
      {
         ea(fp_2, a, 0);
         asmprintf(",d%d",dreg);
      }
   }
}

static void fpgen(instrword a)
{
   int32 w = readword();

   switch( bits(w, 15, 13) )
   {
   case 0:
   case 2:
      fpgen0(a,w);
      break;
   case 1:
      asmprintf("????");
      break;
   case 3:
      fpmovefromfp(a,w);
      break;
   case 4:
   case 5:
      fpmovefpcr(a,w);
      break;
   case 6:
   case 7:
      fpmovem(a,w);
      break;
   }
}

static void fpcond(char *op, int32 pred, int32 sz)
{
   char *bop;
   char s[10];

   s[0] = 0;
   strcat(s, op);

   switch( pred )
   {
   default:
   case 0x00:  bop = "F"; break;
   case 0x01:  bop = "EQ"; break;
   case 0x02:  bop = "OGT"; break;
   case 0x03:  bop = "OGE"; break;
   case 0x04:  bop = "OLT"; break;
   case 0x05:  bop = "OLE"; break;
   case 0x06:  bop = "OGL"; break;
   case 0x07:  bop = "OR"; break;
   case 0x08:  bop = "UN"; break;
   case 0x09:  bop = "UEQ"; break;
   case 0x0a:  bop = "UGT"; break;
   case 0x0b:  bop = "UGE"; break;
   case 0x0c:  bop = "ULT"; break;
   case 0x0d:  bop = "NLE"; break;
   case 0x0e:  bop = "NE"; break;
   case 0x0f:  bop = "T"; break;
   case 0x10:  bop = "SF"; break;
   case 0x11:  bop = "SEQ"; break;
   case 0x12:  bop = "GT"; break;
   case 0x13:  bop = "GE"; break;
   case 0x14:  bop = "LT"; break;
   case 0x15:  bop = "LE"; break;
   case 0x16:  bop = "GL"; break;
   case 0x17:  bop = "GLE"; break;
   case 0x18:  bop = "NGLE"; break;
   case 0x19:  bop = "NGL"; break;
   case 0x1a:  bop = "NLE"; break;
   case 0x1b:  bop = "NLT"; break;
   case 0x1c:  bop = "NGE"; break;
   case 0x1d:  bop = "NGT"; break;
   case 0x1e:  bop = "SNE"; break;
   case 0x1f:  bop = "ST"; break;
   }
   strcat(s,bop);
   write_op(s,sz);
}

static void fpdbcc(instrword a)
{
   int32 fpr = bits(a,2,0);
   int32 pcx = pc;
   int32 pred = readword();
   int32 off = readword();

   fpcond("FDB",pred, -1);
   asmprintf("d%d,%06x",fpr,off+pcx);
}

static void fpbcc(instrword a)
{
   int32 pred = bits(a,5,0);
   int32 s = bits(a,6,6);
   int32 pcx = pc;
   int32 addr;

   fpcond("FB",pred,s+1);

   addr = s==1 ? readlong() : readword();
   asmprintf("%06x",addr+pcx);
}

static void fptcc(instrword a)
{
   int32 pred = readword();
   int32 mode = bits(a,2,0);

   fpcond("FTRAP",pred, (mode == 4)? 7: mode-1);

   if( mode == 4 ) return;

   if( mode == 2 )
   {
      asmprintf("%04x",readword());
      return;
   }

   if( mode == 3 )
   {
      asmprintf("%08x",readlong());
      return;
   }
}

static void fpscc(instrword a)
{
   int32 pred = readword();

   fpcond("FS",pred,0);
   ea(fp_2,a, 0); /* op size irrelevant */
}

static void fpsave(instrword a)
{
   write_op("FSAVE", -1);
   ea(fp_2,a, 0); /* op size irrelevant */
}

static void fprestore(instrword a)
{
   write_op("FRESTORE", -1);
   ea(fp_2,a, 0); /* op size irrelevant */
}

static void write_fp_ins(instrword a)
{
   switch( bits(a,8,6) )
   {
   case 0:
      fpgen(a);
      break;
   case 1:
      switch( bits(a,5,3) )
      {
      case 1:
         fpdbcc(a);
         break;
      case 7:
         fptcc(a);
         break;
      default:
         fpscc(a);
         break;
      }
      break;

   case 2:
   case 3:
      fpbcc(a);
      break;

   case 4:
      fpsave(a);
      break;

   case 5:
      fprestore(a);
      break;
   case 6:
   case 7:
      asmprintf("????");
      break;
   }
}

static void write_mulsl(instrword a)
{
    instrword w = readword();
    char *ins = bits(w,11,11)? "muls": "mulu";
    int32 dh  = bits(w,2,0);
    int32 dl  = bits(w,14,12);
    int32 sz  = bits(w,10,10);

    write_op(ins, 2);
    ea(mulsl_2, a, 0);
    asmprintf(",");
    if( sz != 0 )
    {  writereg(dh); asmprintf(":");
       writereg(dl);
    }
    else
       writereg(dl);
}

static void write_divsl(instrword a)
{
    instrword w = readword();
    char *ins;
    int32 dr  = bits(w,2,0);
    int32 dq  = bits(w,14,12);
    int32 sz  = bits(w,10,10);

    ins = bits(w,11,11)? ( (sz != 0)? "DIVSL": "DIVS" ):
                         ( (sz != 0)? "DIVUL": "DIVU" );
    write_op(ins,2);
    ea(divsl_2, a, 0);
    asmprintf(",");
    if( ( sz != 0 ) | (dr != dq) )
    {  writereg(dr); asmprintf(":");
       writereg(dq);
    }
    else
       writereg(dr);
}


/* exported routine */

int32 decode_instruction(int32 off,char *s)
{
      instrword ins = 0;

      dec_off = off;
      pc = off+codebase;
      outstring = s;
      strcpy(s, "?");
      ins = readword();
      write_instruction(find_opcode(ins), ins);
      return (dec_off-off);
}

/* end of m68k/unixins.c */
