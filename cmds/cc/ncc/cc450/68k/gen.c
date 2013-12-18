/*
 * C compiler file m68k/gen.c, version 50
 * Copyright (C) Codemist Ltd, 1988.
 * Copyright (C) Perihelion Software, 1988
 */

#ifndef NO_VERSION_STRINGS
extern char gen_version[];
char gen_version[] = "\nm68k/gen.c $Revision: 1.4 $ 50\n";
#endif

/* AM memo: tidy up the repeated calls to global_list4().               */
/* AM memo: fix up the duff references to symdata_().                   */

/* exports:
   void show_instruction(J_OPCODE op,VRegInt r1,VRegInt r2,VRegInt m);
   RealRegister local_base(Binder *b);
   int32 local_address(Binder *b);
    int32 bitreverse(int32 i);
   bool immed_cmp(int32);
   bool fpliteral(FloatCon *val, J_OPCODE op);
      also (unfortunately for peephole):
   void setlabel(LabelNumber *);
   void branch_round_literals(LabelNumber *);
*/

#include "errors.h"
#include "globals.h"
#include "mcdep.h"
#include "mcdpriv.h"
#include "aeops.h"
#include "xrefs.h"
#include "ops.h"
#include "jopcode.h"
#include "store.h"
#include "codebuf.h"
#include "regalloc.h"
#include "builtin.h"   /* for te_xxx */
#include "bind.h"      /* for sym_insert_id */
#include "cg.h"        /* for procflags, greatest_stackdepth */

#define NONLEAF (PROC_ARGPUSH | PROC_ARGADDR | PROC_BIGSTACK | BLKCALL)
/* STACKCHECK (NONLEAF subset) defines when stack check is REALLY needed */
#define STACKCHECK (PROC_BIGSTACK | BLKCALL)

#define word_sized(n) (-32768 <= (n) && (n) <= 32767)
#define unsigned_word_sized(n) ((unsigned)(n) <= 0xffff)
#ifdef TARGET_IS_ATARI
				/* There is a bug in Lattice assembler */
#define byte_sized(n) (-124 /* 8?? */   <= (n) && (n) <= 127)
#else
#define byte_sized(n) (-128 <= (n) && (n) <= 127)
#endif
#ifdef TARGET_IS_68020
static void outfpinstr( instrtype op, int32 ea, ... );
static int32 fpregnum(RealRegister r);
#define move_fpregister(r1,r2)\
   if( r1 != r2) outfpinstr(OP_FMOVETOFP, 0, R, fpregnum(r2), fpregnum(r1));
#endif

#define P_PEEPRET  1
#define P_PEEPCMPZ 2
#define P_PEEPPUSH 4
#define R_STACK (-1)      /* bit of a hack */
#define P_ADCON    8
#define P_PEEPBTST 16   /* result of AND is only used for ==/!= 0       */
#define P_PRE     32      /* Used for pre-auto-index                  */
#define P_POST    64      /* Used for post-auto-index                 */

static int32 nargwords, fp_minus_sp, savedmask, savedwords;
static Symstr *stacklimitname;
static LabelNumber *returnlab;
static bool switchbranch;
static int32 illbits;
#ifdef TARGET_IS_68020
static void outL(int32 l);
static bool fpcmp;
#endif
#ifdef TARGET_IS_HELIOS
int in_stubs;
#endif

static struct {    /* like struct Icode but with RealReg's and a peep field */
  J_OPCODE op;
  int32 peep;
  RealRegister r1,r2;
  int32 m;
} pending;

/* AM thinks uses_returnlab is spurious when  unsetlab_ is working better */
static int32 uses_returnlab;
#define unsetlab1_(x) \
    (!lab_isset_(x) && (x)->u.frefs != NULL)  /* at least one pending ref */
#define unsetlab0_(x) \
    (!lab_isset_(x))                          /* simply not yet set       */

#define swapregs(r1,r2) \
   { RealRegister temp = r1; \
     r1 = r2; \
     r2 = temp; \
   }

extern int32 bitreverse(int32 i);

static void load_adcon(RealRegister r1,Symstr *name,int32 offset);
static void outinstr(instrtype op,...);
static void outRR(HWORD w);
static void routine_exit(int32 condition, bool really_return);
static void show_fp_inst_direct( J_OPCODE op, int32 m, int32 peep,
                  RealRegister r1r, RealRegister r2r, RealRegister mr);

#define LABREF_BRANCHW  0

static int32 to_m68c[] = {
   C_ALWAYS, C_NEVER,
   C_GT, C_LE,
   C_GE, C_LT,
   C_HI, C_LS,
   C_VS, C_VC,
   C_MI, C_PL,
   C_HS, C_LO,
   C_EQ, C_NE
};

#ifdef TARGET_IS_68020
static int32 to_fpc[] = {
   -1, -1,
   FC_GT, FC_LE,
   FC_GE, FC_LT,
   -1, -1,
   -1, -1,
   FC_LT, FC_GT,
   -1, -1,
   FC_EQ, FC_NE
};
#endif

static struct instr instructions[OP_SIZE] =
{
   { M_ORI,   I_CPU+0, 2, { { 3, 6 }, { 0x3f, 0} } },
   { M_ANDI,  I_CPU+0, 2, { { 3, 6 }, { 0x3f, 0} } },
   { M_SUBI,  I_CPU+0, 2, { { 3, 6 }, { 0x3f, 0} } },
   { M_ADDI,  I_CPU+0, 2, { { 3, 6 }, { 0x3f, 0} } },
   { M_EORI,  I_CPU+0, 2, { { 3, 6 }, { 0x3f, 0} } },
   { M_CMPI,  I_CPU+0, 2, { { 3, 6 }, { 0x3f, 0} } },
   { M_BTSTS, I_CPU+1, 1, { { 0x3f, 0} } },
   { M_BCHGS, I_CPU+1, 1, { { 0x3f, 0} } },
   { M_BCLRS, I_CPU+1, 1, { { 0x3f, 0} } },
   { M_BSETS, I_CPU+1, 1, { { 0x3f, 0} } },
   { M_NEGX,  I_CPU+0, 2, { { 3, 6 }, { 0x3f, 0} } },
   { M_CLR,   I_CPU+0, 2, { { 3, 6 }, { 0x3f, 0} } },
   { M_NEG,   I_CPU+0, 2, { { 3, 6 }, { 0x3f, 0} } },
   { M_NOT,   I_CPU+0, 2, { { 3, 6 }, { 0x3f, 0} } },
   { M_TST,   I_CPU+0, 2, { { 3, 6 }, { 0x3f, 0} } },
   { M_PEA,   0, 1, { { 0x3f, 0} } },
   { M_JSR,   0, 1, { { 0x3f, 0} } },
   { M_JMP,   0, 1, { { 0x3f, 0} } },
   { M_MOVEM, 1, 3, { {1, 10}, { 1, 6 }, { 0x3f, 0} } },
   { M_CHK,   I_CPU+0, 2, { { 7, 9 }, { 0x3f, 0} } },
   { M_LEA,   0, 2, { { 7, 9 }, { 0x3f, 0} } },
   { M_MOVEAL,0, 2, { { 7, 9 }, { 0x3f, 0} } },
   { M_MOVEAW,0, 2, { { 7, 9 }, { 0x3f, 0} } },
   { M_BTSTD, I_CPU+0, 3, { { 7, 9 }, { 3, 6} } },
   { M_BCHGD, I_CPU+0, 3, { { 7, 9 }, { 3, 6} } },
   { M_BCLRD, I_CPU+0, 3, { { 7, 9 }, { 3, 6} } },
   { M_BSETD, I_CPU+0, 3, { { 7, 9 }, { 3, 6} } },
   { M_SCC,   0, 2, { { 0xf, 8 },  { 0x3f, 0} } },
   { M_OR,    I_CPU+0, 3, { { 7, 9 }, { 7, 6}, { 0x3f, 0} } },
   { M_SUB,   I_CPU+0, 3, { { 7, 9 }, { 7, 6}, { 0x3f, 0} } },
   { M_SUBAW, 0, 2, { { 7, 9 }, { 0x3f, 0} } },
   { M_SUBAL, 0, 2, { { 7, 9 }, { 0x3f, 0} } },
   { M_CMP,   I_CPU+0, 3, { { 7, 9 }, { 7, 6}, { 0x3f, 0} } },
   { M_CMPAW, I_CPU+0, 2, { { 7, 9 }, { 0x3f, 0} } },
   { M_CMPAL, I_CPU+0, 2, { { 7, 9 }, { 0x3f, 0} } },
   { M_EOR,   I_CPU+0, 3, { { 7, 9 }, { 7, 6}, { 0x3f, 0} } },
   { M_ADD,   I_CPU+0, 3, { { 7, 9 }, { 7, 6}, { 0x3f, 0} } },
   { M_ADDAW, 0, 2, { { 7, 9 }, { 0x3f, 0} } },
   { M_ADDAL, 0, 2, { { 7, 9 }, { 0x3f, 0} } },
   { M_AND,   I_CPU+0, 3, { { 7, 9 }, { 7, 6}, { 0x3f, 0} } },
   { M_DIVU,  I_CPU+0, 2, { { 7, 9 }, { 0x3f, 0} } },
   { M_DIVS,  I_CPU+0, 2, { { 7, 9 }, { 0x3f, 0} } },
   { M_MULU,  I_CPU+0, 2, { { 7, 9 }, { 0x3f, 0} } },
   { M_MULS,  I_CPU+0, 2, { { 7, 9 }, { 0x3f, 0} } },
   { M_ADDQ,  I_CPU+0, 3, { { 7, 9 }, { 7, 6}, { 0x3f, 0} } },
   { M_SUBQ,  I_CPU+0, 3, { { 7, 9 }, { 7, 6}, { 0x3f, 0} } },
   { M_ASLM,  I_CPU+0, 1, { { 0x3f, 0} } },
   { M_ASRM,  I_CPU+0, 1, { { 0x3f, 0} } },
   { M_LSLM,  I_CPU+0, 1, { { 0x3f, 0} } },
   { M_LSRM,  I_CPU+0, 1, { { 0x3f, 0} } },
   { M_ROLM,  I_CPU+0, 1, { { 0x3f, 0} } },
   { M_RORM,  I_CPU+0, 1, { { 0x3f, 0} } },
   { M_ROXLM, I_CPU+0, 1, { { 0x3f, 0} } },
   { M_ROXLM, I_CPU+0, 1, { { 0x3f, 0} } },
   { M_MOVE,  I_CPU+0, 3, { { 3, 12}, { 0x3f, 6 }, { 0x3f, 0} } },
   { M_MOVEQ, I_CPU+0, 2, { { 7, 9 }, { 0xFF, 0} } },
   { M_BCC,   0, 2, { { 0xf, 8 }, { 0xFF, 0} } },
   { M_BSR,   0, 1, { { 0xFF, 0} } },
   { M_RTS,   0, 0 },
   { M_DBCC,  I_CPU+0, 2, { { 0xf, 8 }, { 7, 0} } },
   { M_SWAP,  I_CPU+0, 1, { { 7, 0} } },
   { M_EXTW,  I_CPU+0, 1, { { 7, 0} } },
   { M_EXTL,  I_CPU+0, 1, { { 7, 0} } },
   { M_LINK,  1, 1, { { 7, 0} } },
   { M_UNLK,  0, 1, { { 7, 0} } },
   { M_EXG,   0, 3, { { 7, 9}, { 0x1f, 3}, { 7, 0} } },
   { M_ADDX,  I_CPU+0, 4, { { 7, 9}, { 3, 6}, { 1, 3}, { 7, 0} } },
   { M_SUBX,  I_CPU+0, 4, { { 7, 9}, { 3, 6}, { 1, 3}, { 7, 0} } },
   { M_ASLR,  I_CPU+0, 4, { { 7, 9 }, { 3, 6}, {1, 5},{ 7, 0} } },
   { M_ASRR,  I_CPU+0, 4, { { 7, 9 }, { 3, 6}, {1, 5},{ 7, 0} } },
   { M_LSLR,  I_CPU+0, 4, { { 7, 9 }, { 3, 6}, {1, 5},{ 7, 0} } },
   { M_LSRR,  I_CPU+0, 4, { { 7, 9 }, { 3, 6}, {1, 5},{ 7, 0} } },
   { M_ROLR,  I_CPU+0, 4, { { 7, 9 }, { 3, 6}, {1, 5},{ 7, 0} } },
   { M_RORR,  I_CPU+0, 4, { { 7, 9 }, { 3, 6}, {1, 5},{ 7, 0} } },
   { M_ROXLR, I_CPU+0, 4, { { 7, 9 }, { 3, 6}, {1, 5},{ 7, 0} } },
   { M_ROXRR, I_CPU+0, 4, { { 7, 9 }, { 3, 6}, {1, 5},{ 7, 0} } },
   { M_NOOP,  0, 0 }
#ifdef TARGET_IS_68020
   ,
   { M_BFCHG, I_CPU+1, 1, { { 0x3f, 0} } },
   { M_BFCLR, I_CPU+1, 1, { { 0x3f, 0} } },
   { M_BFEXTS,I_CPU+1, 1, { { 0x3f, 0} } },
   { M_BFEXTU,I_CPU+1, 1, { { 0x3f, 0} } },
   { M_BFFFO, I_CPU+1, 1, { { 0x3f, 0} } },
   { M_BFINS, I_CPU+1, 1, { { 0x3f, 0} } },
   { M_BFSET, I_CPU+1, 1, { { 0x3f, 0} } },
   { M_BFTST, I_CPU+1, 1, { { 0x3f, 0} } },
   { M_DIVL,  I_CPU+1, 1, { { 0x3f, 0} } },
   { M_EXTBL, I_CPU+0, 1, { { 7, 0 } } },
   { M_LINKL, I_CPU+2, 1, { { 7, 0 } } },
   { M_MULL,  I_CPU+1, 1, { { 0x3f, 0} } },
   { M_RTD,   I_CPU+1, 0 }
#ifndef SOFTWARE_FLOATING_POINT
   ,
   { M_FADD,  I_FP+0, 3, { { 1, 14}, {7, 10}, {7, 7} } },
   { M_FSUB,  I_FP+0, 3, { { 1, 14}, {7, 10}, {7, 7} } },
   { M_FMUL,  I_FP+0, 3, { { 1, 14}, {7, 10}, {7, 7} } },
   { M_FDIV,  I_FP+0, 3, { { 1, 14}, {7, 10}, {7, 7} } },
   { M_FMOVETOFP, I_FP+0, 3, { { 1, 14}, {7, 10}, {7, 7} } },
   { M_FMOVEFROMFP, I_FP+0, 3, { {7, 10}, {7, 7}, {0x7f, 0} } },
   { M_FSGLMUL,I_FP+0, 3, { { 1, 14}, {7, 10}, {7, 7} } },
   { M_FSGLDIV,I_FP+0, 3, { { 1, 14}, {7, 10}, {7, 7} } },
   { M_FNEG,  I_FP+0, 3, { { 1, 14}, {7, 10}, {7, 7} } },
   { M_FCMP,  I_FP+0, 3, { { 1, 14}, {7, 10}, {7, 7} } },
   { M_FBCC,  0, 2, { { 1, 1}, {0x3f, 0} } },
   { M_FSCC,  1, 1, { {0x3f, 0} } },
   { M_FMOVEM,0, 3, { { 1, 13}, {3, 11}, {0xff, 0} } }
#endif
#endif
} ;

#ifdef TARGET_IS_HELIOS
#if 0
bool is_function(Symstr *name)
{  if( symdata_(name) == NULL ) return 1; /* Default to true */
   return( (bindstg_(symdata_(name)) & b_fnconst) != 0 );
}
#else
bool
is_function( Symstr * symbol )
{
  struct Binder *	data;


  /* be paranoid */
  
  if (symbol == NULL)
    syserr( "gen.c: is_function: passed a NULL pointer" );

  /* catch a rare special case */
  
  if (symtype_( symbol ) != s_identifier)
    syserr( "gen.c: is_function: not passed an identifier" );
  
   /* __dataseg has no data associated with it */
  
  if (symbol == bindsym_( datasegment ))
    return NO;
  
  /*
   * We default to NO because even though functions like
   * '__stackoverflow' & '_printf' have no data this function should
   * never be called for them, and local external data will have
   * no data.  ie
   *
   * int func ( void )
   * {
   *    extern int fred;
   *    return fred;
   * }
   *
   * Will call this function with a symbol 'fred' with no data.
   *
   */

  if ((data = bind_global_( symbol )) == NULL)
    {
      struct ExtRef *	ref;

      
      if ((ref = symext_( symbol )) != NULL)
	{
	  /* cope with external symbols */
	  
#define is_data( x )			 ((x)->extflags & xr_data)
	  
	  if (is_data( ref ))
	    {
	      return NO;
	    }
	  else
	    {
	      return YES;
	    }
	}
      else
	{
	  return NO;
	}
    }

  return ((bindstg_( data ) & b_fnconst) != 0 );

} /* is_function */
#endif

#ifndef TARGET_IS_HELIOS
static int32 extern_definition(Symstr *name)
{
   if( bind_global_(name) != NULL )
      return (bindstg_( bind_global_(name) ) & bitofstg_(s_extern) );
   else
      return 1;
}
#endif /* not TARGET_IS_HELIOS */

static int32 static_definition(Symstr *name)
{
   if( bind_global_(name) != NULL )
      return (bindstg_( bind_global_(name) ) & bitofstg_(s_static) );
   else
      return 0;
}

#endif

bool immed_cmp(int32 n)
{
   IGNORE(n);
   return(1);
}

void branch_round_literals(LabelNumber *l)
{
   IGNORE(l);
   syserr(syserr_branch_round_literals);
}

static void conditional_branch_to(int32 condition, LabelNumber *destination,
                                  bool bxxcase)
{
#ifdef TARGET_IS_68020
    int32 fcond = 0;
    if (fpcmp) fcond = FC_FROMQ(condition);
#endif
    condition = C_FROMQ(condition);

    if( destination == RETLAB )
    {
        if( condition == C_ALWAYS && !bxxcase && !switchbranch)
        {
            if( (savedmask == 0) && !(procflags & NONLEAF) )
            {   if( unsetlab1_(returnlab) ) setlabel(returnlab);
                outinstr(OP_RTS);
                return;
            }

            if( unsetlab0_(returnlab) )
            {   setlabel(returnlab);
                routine_exit(C_ALWAYS,1);
                return;
            }
        }

        uses_returnlab = 1, destination = returnlab;
    }

    if( lab_isset_(destination))
    {
        int32 w = destination->u.defn;
        int32 off = w - codep - 2;
#ifdef TARGET_IS_68020
        if( byte_sized(off) && !switchbranch && !fpcmp ) /* short branch */
#else
        if( byte_sized(off) && !switchbranch )           /* short branch */
#endif
        {
            outinstr( OP_BCC, condition, off & 0xff);
        }
        else
        {
#ifdef TARGET_IS_68020
            if( !word_sized(off) )
            {
               if( fpcmp && (condition != C_ALWAYS) )
                  outinstr( OP_FBCC, 1, fcond);
               else
                  outinstr( OP_BCC, condition, 0xff);
               outL( off );
            }
            else
#else /* plain 68000 */
            if( !word_sized(off) )
                syserr(syserr_big_branch, (long)off);
#endif
            {     /* Word sized offset */
#ifdef TARGET_IS_68020
               if( fpcmp && (condition != C_ALWAYS) )
                  outinstr( OP_FBCC, 0, fcond);
               else
#endif
                  outinstr( OP_BCC, condition, 0);
               outRR( off & 0xffff);
            }
        }
    }
    else    /* Forward reference */
    {
        addfref_( destination, codep+2);
/* In 68020 case what do I assume about the offset? */
#ifdef TARGET_IS_68020
        if( fpcmp && (condition != C_ALWAYS) )
           outinstr( OP_FBCC, 0, fcond);
        else
#endif
           outinstr( OP_BCC, condition, 0);
        outRR( 0 );
    }
}

static void cnop()
{
   if( (codep & 2) != 0) outinstr(OP_NOOP);
}

void setlabel(LabelNumber *l)
{
    List *p = l->u.frefs;

    if (asmstream) asm_lablist = mkLabList(asm_lablist, l);
 
    for (; p != (List *)NULL; p = (List *)discard2(p))
    {
        int32 v = car_(p);
        int32 q = ( v & 0x00ffffff );       /* BYTE address */
        int32 w;
        unsigned32 d;

        switch (v & 0xff000000)
        {
        case LABREF_BRANCHW:
            w = code_hword_(q);
            d = (unsigned32)(codep - q);
            if( d > 0x7fff )
                syserr(syserr_big_displacement, (long)d);
            code_hword_(q) = (unsigned16)d;
            break;

        default :
            syserr(syserr_labref_type, (long)v);
        }
    }
/* This next line is the ACN/AM version but I can't see why the constant
   is ored in because lab_setloc sets the top bit of the name field to
   differentiate the union */
/*    lab_setloc_(l, codep | 0x80000000); *//* frefs union checker */
    lab_setloc_(l, codep );
}

RealRegister local_base(Binder *b)
{
    int32 p = bindaddr_(b);
    switch (p & BINDADDR_MASK)
    {   default: syserr(syserr_local_base, (long)p);
        case BINDADDR_ARG: if (procflags & NONLEAF) return R_FP;
                           /* drop through */
        case BINDADDR_LOC: return R_SP;
    }
}

int32 local_address(Binder *b)
{
    int32 p = bindaddr_(b);
    VRegInt gap; gap.r = GAP;
    show_instruction(J_NOOP, gap, gap, gap);  /* NASTY!!! Ensures fp_minus_sp!! */
    switch (p & BINDADDR_MASK)
    {
default:
        syserr(syserr_local_address, (long)p);
case BINDADDR_LOC:
        return fp_minus_sp - (p & ~BINDADDR_MASK);
case BINDADDR_ARG:
        p &= ~BINDADDR_MASK;
        if (!(procflags & NONLEAF))
        {   /* LDRV1 only - hence nargwords > NARGREGS.  SP relative.   */
            /* also note that fp_minus_sp == 0 assumed.  Caveat emptor! */
            if (nargwords <= NARGREGS || fp_minus_sp != 0)
                syserr(syserr_local_addr);
            return p - 4*NARGREGS + 4*(savedwords + 1);
        }
        /* on the 68000 the args go in different places depending on    */
        /* their number. This saves a whole bunch of instructions in    */
        /* the entry and exit sequences!                                */
        return (nargwords > NARGREGS)  ? p + 8 :  /* Skip FPL and RPC */
               p - 4*(nargwords + savedwords) ;
    }
}

static bool datareg(int32 r)
{
   return ( datareg_(r) );
}

static bool addrreg(int32 r)
{
   return ( addrreg_(r) );
}

static int32 addrmode(int32 r)
{
   return ( datareg_(r) ? EA_Dn: EA_An );
}

static int32 Mregnum(RealRegister r)
{  int32 res = Mregnum_(r);
   if( res == -1l ) syserr(syserr_regnum, r);
   return( res );
}

static void outRR(HWORD w)
{   if (w & ~0xffffL) syserr(syserr_outHW,(long)w);
    if ((codep & 2) == 0)
        outcodeword(0, LIT_OPCODE), codep -= 4;  /* ensure room */
    code_hword_(codep) = (unsigned16)w;
    codep += 2;
}

static void outL(int32 l)
{
   outRR( (HWORD)( ((unsigned32)l) >> 16) );
   outRR( (HWORD)( l & 0xffff ) );
}

#ifdef TARGET_IS_HELIOS
void outINIT(void)
{
    codexrefs = (CodeXref *)
       global_list3(SU_Other, codexrefs,X_Init+codebase+codep,0);
    outL(0);
}
#endif

static void voutinstr(instrtype op, va_list a)
{
    struct instr *ins = &instructions[(int)op];
    int i;
    int32 w = ins->baseval;
    int32 arg;
    fieldstr *f = &(ins->field[0]);

    for ( i = 1; i <= ins->nfields; f++,i++)
    {
        arg = va_arg(a,int32);
        if( (arg & f->mask) != arg)
            syserr(syserr_ill_instr, (long)op,(long)arg);
        w |= (arg & f->mask) << f->shift;
    }
    outRR(w); /* , LIT_OPCODE); */

    for( i = 1; i <= (ins->extwords & ~I_MASK); i++)
        outRR(va_arg(a,int32)); /* , LIT_OPCODE); */

#ifdef TARGET_IS_68020
    if( (ins->extwords & I_FP) != 0) fpcmp = 1;
    else if( (ins->extwords & I_CPU) != 0) fpcmp = 0;
#endif
}

static void outinstr(instrtype op, ...)
{
    va_list a;
    va_start(a,op);
    voutinstr(op,a);
    va_end(a);
}

static void eafield(int32 ea, ...)
{
    va_list alst;
    va_start( alst, ea );

    switch (ea & 0x38)
    {
    case EA_Dn :
    case EA_An :
    case EA_AnInd :
    case EA_AnPost :
    case EA_AnPre :
        break ;

    case EA_AnDisp :
    {   int32 n = va_arg(alst,int32);
        if( !word_sized(n) )
            syserr(syserr_addr_disp);
        outRR(n & 0xffff);
        break;
    }
    case EA_AnIndx :
    {   int32 n = va_arg(alst,int32);
        int32 bd = va_arg(alst, int32);

#ifdef TARGET_IS_68020
        int32 bds=0,ods=0,od;
#endif
/* For 020 n is the extension word and we have to look at that
    to find out if there is any more info to put out */
        if( (n & 0xffff) != n )
            syserr(syserr_invalid_index_mode);
#ifdef TARGET_IS_68020

        if( bd == 0 )
            bds = 1;
        else
            if( !byte_sized(bd) || ((n & EM_FULL) != 0) )
            {
                n |= EM_FULL;
                bds = word_sized(bd)? 2: 3;
            }

        od = 0; /* To avoid dataflow anomaly */
        if( n & EM_FULL )
        {   n += bds << 4;

            if( (n&7) != 0 ) /* Pre or post indexed mode? */
            {   od = va_arg(alst,int32);
                ods = (od == 0) ? 0: (word_sized(od)? 1: 2);
                n += ods;
            }
        }
        else n += bd & 0xff;

        outRR(n);

        if( bds > 1  )  /* Base displacement required */
        {   if( bds != 2 ) outL(bd);
            else outRR((HWORD)bd & 0xffff);
        }

        if( ods > 1  )  /* Output displacement required */
        {   if( ods != 2 ) outL( od );
            else outRR( od & 0xffff );
        }
#else
        outRR(n+(bd & 0xff));
#endif
        break;
    }
    case 0x38 :
    {   int32 n = va_arg(alst,int32);
        switch (ea)
        {
        case EA_AbsW :
        case EA_PCDisp :
/* For 020 n is the extension word and we have to look at that
    to find out if there is any more info to put out */
/* but we don't look there!  For 68020 allow n>=32K?                 */
            if( !word_sized(n) )
	      {
                syserr(syserr_pc_disp);
	      }
	    
        case EA_ImmW :
        case EA_PCInd :         /* above check is inappropriate (a-reg) */
                                /* share code with EA_AnIndx?           */
            outRR(n & 0xffff);
            break;

        case EA_AbsL :
        case EA_ImmL :
#ifdef TARGET_IS_68020
        case EA_ImmS :
#endif
            outL(n);
            break;

#ifdef TARGET_IS_68020
        case EA_ImmD :
            outL(n);
            outL(va_arg(alst,int32));
            break;
#endif
        }
        break;
    }
    default :
        syserr(syserr_eff_addr);
    }
    va_end(alst);
}

/* Generate an immediate operand instruction
    op1 if r is a datareg
    op2 if r is a addrreg
*/
static void immop(instrtype op1, instrtype op2, int32 r, int32 n)
{
    int32 Mreg = Mregnum(r);

    if( addrreg(r) )
        outinstr(op2, Mreg, EA_Imm);
    else
        outinstr(op1, LONG0, EA_Dn+Mreg);
    eafield(EA_ImmL,n);
}

int32 bitreverse(int32 n)       /* exported to decins.c */
{
    int32 m1 = 0x0001;
    int32 m2 = 0x8000;
    int32 i;
    int32 r = 0;

    for( i = 1; i <= 16; m1 <<= 1, m2 >>= 1, i++)
        if (n & m1) r |= m2;
    return(r);
}

static void exchange(RealRegister r1, RealRegister r2)
{
   if( datareg(r2) ) swapregs(r1, r2);

   if( datareg(r1) )
   {
      if( datareg(r2) )
      {  outinstr(OP_EXG, Mregnum(r1), 0x8, Mregnum(r2) );
         return;
      }
      else
      {  outinstr(OP_EXG, Mregnum(r1), 0x11, Mregnum(r2) );
         return;
      }
   }
   else
      outinstr(OP_EXG, Mregnum(r1), 0x9, Mregnum(r2) );
}

#define reversea(n) ( (((n)&7) << 3) | (((n)>>3) & 7))

#define genmove( dea, dext, size, sea, sext) \
    (outinstr(OP_MOVE, size, reversea(dea), sea), \
     eafield((sea != EA_Imm)? sea: (size == LONG2)? EA_ImmL:EA_ImmW,sext), \
     eafield(dea,dext))

static void move_register(RealRegister r1, RealRegister r2)
{   /* r1 = r2    */
    int32 Mreg2  = Mregnum(r2);
    int32 Mode2  = addrmode(r2);

    if (r1 == R_STACK)
    {
        genmove(EA_AnPre+Mregnum(R_SP), 0, LONG2, Mode2+Mreg2, 0 );
    }
    else if (r1 != r2)
    {
        int32 Mreg1  = Mregnum(r1);
        if( datareg(r1) )
            genmove( EA_Dn+Mreg1, 0, LONG2, Mode2+Mreg2, 0 );
        else
            outinstr(OP_MOVEAL, Mreg1, Mode2+Mreg2);
    }
}

static void op_register(RealRegister r1, RealRegister r2, instrtype op)
{   /*  r1 = op r2  */
    RealRegister tr = addrreg(r1) ? R_DS : r1;
    move_register(tr, r2);
    outinstr(op, LONG0, EA_Dn+Mregnum(tr));
    move_register(r1, tr);
}

static void call_k(Symstr *name, bool tailcall)
{
/* The initial part of this code is independent of operating   */
/* system since if we know how far the routine is there is no  */
/* problem. Things get a bit trickier from there on in.        */
    int32 dest;
    int32 off;

#ifdef TARGET_IS_HELIOS
    if( in_stubs )
    {
        load_adcon(R_AS, name, 0);
        outinstr(tailcall?OP_JMP:OP_JSR, EA_AnInd+Mregnum(R_AS));
        return;
    }
#endif
    
    dest = obj_symref(name, xr_code, 0);
    off  = dest - (codep+codebase+2);
    if( dest != -1)
    {           /* previously defined routine */
        if( byte_sized(off) )
        {   if(tailcall)
                outinstr(OP_BCC,C_ALWAYS, off & 0xff);
            else
                outinstr(OP_BSR,off & 0xff);
            return;
        }
        else
        {    if( word_sized(off) )
            {
                if(tailcall)
                    outinstr(OP_BCC,C_ALWAYS,0);
                else
                    outinstr(OP_BSR,0);
                outRR(off&0xffff);
                return;
            }
        }
    }                /* If none of these then fall through */
#ifndef TARGET_IS_HELIOS
# ifdef SMALL_MODULES_ONLY_EG_AMIGA_DOS
/* The following code cannot work if modules can exceed 32K in size. */
    if( static_definition(name) )
    {
# ifndef STATIC_32BIT_CALLS
        codexrefs = (CodeXref *)
           global_list4(SU_Other, codexrefs,X_PCreloc+codebase+codep+2,name,0);

        if(tailcall)
            outinstr(OP_BCC,C_ALWAYS,0);
        else
            outinstr(OP_BSR,0);
        outRR(0);           /* This will be relocated by m68obj (amiga) */
# else
/* Investigate BSR with 32 bit offset for 68020 */
        codexrefs = (CodeXref *)
        (global_list4(SU_Other, codexrefs,X_absreloc+codebase+codep+2,name,0));

        outinstr(tailcall? OP_JMP:OP_JSR, EA_AbsL);
        eafield(EA_AbsL,0);
# endif
    }
    else    /* Not static */
# endif /* SMALL_MODULES_ONLY_EG_AMIGA_DOS */
    {  codexrefs = (CodeXref *)
          (global_list4(SU_Other, codexrefs,X_absreloc+codebase+codep+2,name,0));
       outinstr((tailcall? OP_JMP: OP_JSR),EA_AbsL);
       eafield(EA_AbsL,0);
/*       code_flag_(codep-4) = code_flag_(codep-2) = LIT_RELADDR; */
    }
    return;

#else   /* TARGET_IS_HELIOS */
/* Static forward or external calls are generated in the same   */
/*  way. The addition to the stublist will be removed if the    */
/*  function is subsequently defined.                           */
    codexrefs = (CodeXref *)
       global_list5(SU_Other, codexrefs,X_PCreloc+codebase+codep+2,name,0,0);

    if(tailcall)
        outinstr(OP_BCC,C_ALWAYS,0);
    else
        outinstr(OP_BSR,0);
    outRR(0);           /* This will be relocated by m68obj */
    request_stub(name);
#endif
}

#ifdef TARGET_IS_HELIOS
void load_static_data_ptr(RealRegister r, bool is_code)
{
    codexrefs = (CodeXref *)
        global_list3(SU_Other, codexrefs,X_Modnum+codebase+codep+2,0);
    outinstr( OP_MOVE, LONG2, reversea(EA_An+Mregnum(r)),
                                      EA_AnDisp+Mregnum(R_DP) );
    eafield(EA_AnDisp, 0);
}

#if 0
/* This routine will find another data symbol with the same offset */
/* as the one provided as its argument. This is used in the helios */
/* compiler when compiling with the -r flag so that we don't       */
/* have lots of __dataseg symbols.                                 */

static Symstr *find_data_start(Symstr *name)
{  ExtRef *r,*dext;
   dext = r = ((ExtRef *)symext_(name))->extcdr;

   for( ; r != NULL; r = r->extcdr )
   {
      if( (r->extflags & xr_data)  && (r->extoffset == dext->extoffset) )
      return r->extsym;
   }
   syserr(syserr_data_sym, dext->extoffset);
   return 0;
}
#endif

#endif

static void add_integer(RealRegister r1, RealRegister r2, int32 n)
{
/* Generate code for r1 = r2 + n.                                        */
    int32 Mreg1  = Mregnum(r1);
    int32 Mode1  = addrmode(r1);

    if (addrreg(r1) && addrreg(r2) && word_sized(n) && n != 0 &&
        !(r1 == r2 && -8 <= n && n <= 8))
    {
        outinstr(OP_LEA,Mregnum(r1), EA_AnDisp+Mregnum(r2));
        eafield(EA_AnDisp,n);
        return;
    }

    move_register(r1,r2);
    if( (-8 <= n) && (n < 0) )
    {
        outinstr(OP_SUBQ, (n==-8)? 0:-n, LONG0, Mode1+Mreg1);
        return;
    }

    if( (0 < n) && (n <= 8) )
    {
        outinstr(OP_ADDQ, (n==8)? 0:n, LONG0, Mode1+Mreg1);
        return;
    }

    if( n == 0 ) return;
    immop(OP_ADDI,OP_ADDAL,r1,n);
}

static void load_adcon(RealRegister r1, Symstr *name, int32 offset)
{
    ExtRef *x ;
    RealRegister tr;
#ifdef TARGET_IS_HELIOS
    int32 d = obj_symref(name, xr_code, 0);
#endif

    x = symext_(name);
#ifndef TARGET_IS_HELIOS
    tr = (r1==R_STACK || addrreg(r1)) ? r1 : R_AS;
# ifdef SMALL_MODULES_ONLY_EG_AMIGA_DOS
/* The following code cannot work if modules can exceed 32K in size.    */
/* However, the backward fn reference use of LEA is always valid.       */
    if (!((x->extflags & xr_data) || extern_definition(name)))
    {
/* This is some sort of reference to an internal code symbol      */
/* either a backward ref. (already defined) or a promised static fn. */
/* At least I hope this can only be a code symbol because I don't */
/* think forward references to data symbols are possible!         */
        codexrefs = (CodeXref *)
            global_list4(SU_Other, codexrefs,X_PCreloc+codebase+codep+2,name,offset);
            /* We should not get the back end to patch up backward refs.  */
        if (r1==R_STACK)
            outinstr(OP_PEA, EA_PCDisp);
        else
            outinstr(OP_LEA, Mregnum(tr), EA_PCDisp);
        eafield(EA_PCDisp,0);
    }
    else
# endif /* SMALL_MODULES_ONLY_EG_AMIGA_DOS */
    {   codexrefs = (CodeXref *)
            global_list4(SU_Other, codexrefs,X_absreloc+codebase+codep+2,name,offset);
        if (r1==R_STACK)
            outinstr(OP_PEA, EA_AbsL);
        else
            outinstr(OP_LEA, Mregnum(tr), EA_AbsL);
/* AM: the next line had 0 for AMIGA_DOS, but was wrong-minded.          */
        eafield(EA_AbsL,offset);
    }
    if(tr != r1)
        move_register(r1,tr);
    return;

#else /* TARGET_IS_HELIOS */
    tr = (r1!=R_STACK && addrreg(r1)) ? r1 : R_AS;
    if( (x->extflags & xr_data) && (x->extflags & (xr_defext+xr_defloc)) )
    {                           /* An internal data symbol */
/*  MOVE.L   modnum(DP),TR
    LEA.L    name+offset(TR),R1
           where name is the offset of the symbol in our data seg

    X is the size of the exported code symbols in our data segment
    it is not known yet - but will be at the end.
*/
        if( x->extflags & xr_defloc )
        {
#if 0
            if (name == bindsym_(datasegment))
            {
               name = find_data_start(name);
               x    = symext_(name);
            }
#endif
        }
        load_static_data_ptr(tr, YES);

        codexrefs = (CodeXref *)
            global_list4(SU_Other, codexrefs,X_DataSymb+codebase+codep+2,name,offset);
        outinstr( OP_LEA, Mregnum(tr), EA_AnDisp+Mregnum(tr));
        eafield(EA_AnDisp, 0);		/* suspect (HELIOS) */
        if(tr != r1)
            move_register(r1,tr);
    }
    else if(  d != -1 && !in_stubs)
    {                           /* A defined internal code symbol */
/*
    LEA.L   xx(PC),R1
      This code is not used if we are generating stubs since
      the symbol will have been defined for the purposes of
      pointing a data variable at it and that is not the
      definition of the symbol that we actually want!
*/
      if (!word_sized( d - (codep + codebase) + offset ))
	{
	  int32 saved_codep = codep;
	  
	  outinstr( OP_LEA, Mregnum(tr), EA_PCDisp );
	  eafield(EA_PCDisp, 0);

	  if (tr != r1)
	    add_integer( r1, tr, d - (saved_codep + codebase) + offset );
	  else
	    add_integer( tr, tr, d - (saved_codep + codebase) + offset );
	}
      else
	{
	  outinstr( OP_LEA, Mregnum(tr), EA_PCDisp );
	  eafield(EA_PCDisp, d-(codep+codebase)+offset);
	  
	  if(tr != r1)
            move_register(r1,tr);
	}
	
    }
    else if( static_definition( name ) )
    {                           /* Not yet defined internal code symbol */
/*
    LEA.L   xx(PC),R1
*/
        codexrefs = (CodeXref *)
            global_list5(SU_Other, codexrefs,X_PCreloc+codebase+codep+2,name,offset,0);
        outinstr( OP_LEA, Mregnum(tr), EA_PCDisp );
        eafield(EA_PCDisp, 0);		/* suspect (HELIOS) */
        if(tr != r1)
            move_register(r1,tr);
    }
/* All we have left are the external code and data symbols */
    else
    {
/*
    MOVE.L  @name(DP),TR
    MOVE.L  name(TR),R1         -- for functions
or
    MOVE.L  @name(DP),TR
    LEA.L   name(TR),R1         -- for data
*/
	if (suppress_module && is_function(name))
	  {
	    codexrefs = (CodeXref *)
	      global_list4(SU_Other, codexrefs,X_DataAddr+codebase+codep+2,name,offset);
	  }
	else
	  {
	    codexrefs = (CodeXref *)
	      global_list4(SU_Other, codexrefs,X_DataModule+codebase+codep+2,name,0);
	    outinstr(OP_MOVE, LONG2, reversea(EA_An+Mregnum(tr)),
		     EA_AnDisp+Mregnum(R_DP));
	    eafield(EA_AnDisp, 0);

	    codexrefs = (CodeXref *)
	      global_list4(SU_Other, codexrefs,X_DataSymb+codebase+codep+2,name,offset);
	  }

        if( is_function(name) )
        {   int32 dea = r1==R_STACK ? EA_AnPre+Mregnum(R_SP) :
                                      addrmode(r1)+Mregnum(r1);
            outinstr(OP_MOVE, LONG2, reversea(dea), EA_AnDisp+Mregnum(tr));
            eafield(EA_AnDisp, 0);
        }
        else
        {   outinstr(OP_LEA, Mregnum(tr), EA_AnDisp+Mregnum(tr));
            eafield(EA_AnDisp, 0);
            if( tr != r1 ) move_register(r1,tr);
        }
    }
#endif
}

#ifdef TARGET_IS_HELIOS
static void load_string(RealRegister r, StringSegList *s)
{
  addfref_( litlab, codep+2 ); 

  outinstr(OP_BSR,0);
  
  return;
}
#else
static void load_string(RealRegister r, StringSegList *s)
{
    int32 sl = 0;
    int32 bsrl,ends;
    int32 i;
    unsigned long w=0;
    char *str;
    StringSegList *sp;

    for(sp = s; sp != NULL; sp = sp->strsegcdr)
       sl += sp->strseglen;

    bsrl = (sl+4) & ~3; /* Allow for 0 terminator */
    cnop();

    if( bsrl <= 0x7e )
        outinstr(OP_BSR,bsrl);
    else
    {
        outinstr(OP_BSR,0);
        eafield(EA_ImmW,bsrl+2);
    }

    ends = s->strseglen;
    str  = s->strsegbase;
    for( i = 0; i < bsrl; i++)
    {
        if( (i < sl) && (i == ends) )
        {   s     = s->strsegcdr;
            str   = s->strsegbase;
            ends += s->strseglen;
        }

        w = (w << 8) + (((i>=sl)? 0: (long)*str++ & 0xff) );
        if( (i & 1) != 0 ) {
	  code_hword_(codep) = (unsigned16)w;
	  code_flag_(codep) = LIT_STRING;
	  codep += 2;
	  w=0;
	}
    }
    setlabel(nextlabel());
    genmove(addrmode(r)+Mregnum(r),0,LONG2,EA_AnPost+Mregnum(R_SP),0);
}
#endif /* TARGET_IS_HELIOS */


static void opr2_register(RealRegister r1, RealRegister r2,
                          RealRegister mr, instrtype op1, instrtype op2)
{
    if( r1 == mr ) swapregs(r2,mr); /* This won't happen on J_SUBR */

    move_register(r1,r2);

    if( datareg(r1) )
        outinstr(op1, Mregnum(r1), 2, addrmode(mr) + Mregnum(mr) );
    else
        outinstr(op2, Mregnum(r1), addrmode(mr) + Mregnum(mr) );
}

static void compare_register(RealRegister r1, RealRegister r2)
{
    if( datareg(r1) )
        outinstr(OP_CMP, Mregnum(r1), 2, addrmode(r2) + Mregnum(r2) );
    else
        outinstr(OP_CMPAL, Mregnum(r1), addrmode(r2) + Mregnum(r2) );

}

static void load_integer(RealRegister r, int32 n)
/* Set register r to the integer n, setting condition codes on scc       */
{
    int32 Mreg = r == R_STACK ? r : Mregnum(r);

    if (r == R_STACK)
    {   if (n == 0) outinstr(OP_CLR, LONG0, EA_AnPre+Mregnum(R_SP));
        else if (word_sized(n))
            {  /* Pushing addrreg immediate word extends sign bit */
                outinstr(OP_PEA, EA_AbsW);
                eafield(EA_AbsW,n);
            }
            else
            {   outinstr(OP_PEA, EA_AbsL);
                eafield(EA_AbsL,n);
            }
    }
    else if( datareg(r) )
    {   if( byte_sized(n) )
            outinstr(OP_MOVEQ, Mreg, n & 0xff);
        else
            genmove(EA_Dn+Mreg, 0, LONG2, EA_Imm, n);
    }
    else             /* load addrreg */
    {   if( n != 0 )
        {
            if( word_sized(n))
            {  /* Loading addrreg immediate word extends sign bit */
                outinstr(OP_MOVEAW, Mreg, EA_Imm); eafield(EA_ImmW,n);
            }
            else
            {   outinstr(OP_MOVEAL, Mreg, EA_Imm); eafield(EA_ImmL,n);
            }
        }
        else
            outinstr(OP_SUBAL, Mreg, EA_An+Mreg);
    }
}

static void opr_register(RealRegister r1, RealRegister r2, RealRegister mr,
                                       instrtype op)
{
/* Generate code for r1 = r2 op mr.                                        */
/*   where op is OP_OR, OP_AND, OP_EOR                                     */

    if( r1 == mr ) swapregs(r2,mr);

    if( datareg(r1) )
    {   int32 opreg;
        move_register(r1,r2);

        if( addrreg(mr) )
        {
            move_register(R_DS,mr);
            opreg = R_DS;
        }
        else
            opreg = mr;

        if( op == OP_EOR)
            outinstr(op, Mregnum(opreg), 6, EA_Dn+Mregnum(r1) );
        else
            outinstr(op, Mregnum(r1), 2, EA_Dn+Mregnum(opreg) );
    }
    else     /* r1 is address reg */
    {   bool exchanged = 0;
/* make r2 the address reg. cos its more efficient that way */
        if (datareg(r2) ) swapregs(r2,mr);

        move_register(R_DS,r2);
        if ( addrreg(mr) )
        {
            exchange(R_A1, mr);
            exchanged = 1;
        }
        if( op == (int32)OP_EOR )
            outinstr(op, Mregnum(exchanged? R_A1: mr), 6,
                             EA_Dn + Mregnum(R_DS));
        else
            outinstr(op, Mregnum(R_DS), 2,
                             EA_Dn + Mregnum(exchanged? R_A1: mr));
        if(exchanged) exchange(R_A1, mr);
        move_register(r1,R_DS);
    }
}

static int32 bitnum(int32 n)
{
    int32 x = 1;
    int32 r ;
    for( r = 0; r < 32 ; r++,x<<=1 )
        if( (n & x) != 0 ) break;
    return r;
}

static void bit_integer(RealRegister r1, RealRegister r2, int32 n)
{   /* set CC(Z-bit) as for r1 = (r2&n) -- r1 may be corrupted.         */
    int32 bcnt = bitcount(n);
    if (bcnt == 1)
    {   RealRegister tr = addrreg(r2) ? R_DS : r2;
        move_register(tr,r2);
        outinstr(OP_BTSTS, EA_Dn+Mregnum(tr),bitnum(n));
    }
    else
    {   RealRegister tr = addrreg(r1) ? R_DS : r1;
        move_register(tr,r2);
        if ((n & 0xffff0000) == 0)
        {   outinstr(OP_ANDI, WORD0, EA_Dn+Mregnum(tr)); eafield(EA_ImmW,n);
        }
        else
        {   outinstr(OP_ANDI, LONG0, EA_Dn+Mregnum(tr)); eafield(EA_ImmL,n);
        }
    }
}

static void op_integer(RealRegister r1, RealRegister r2, int32 n, instrtype opc,
                       bool scc)
{
/* Generate code for r1 = r2 | n. Ensures CC(Z-bit) correct if 'scc'.   */
    RealRegister tr = addrreg(r1) ? R_DS : r1;
    instrtype bitop = (instrtype) -1;
    int32 bcnt = bitcount(n);

    if (n == -1 && !scc)
    { if (opc == (int32)OP_ANDI) { move_register(r1,r2); return; }
      if (opc == (int32)OP_ORI) { load_integer(r1,-1); return; }
    }

    move_register(tr,r2);

  if (!scc)
  {
    if( bcnt == 1 )
    {
        if( opc == (int32)OP_ORI ) bitop = OP_BSETS;
        else if( opc == (int32)OP_EORI ) bitop = OP_BCHGS;
    }
    else
    {   if( (bcnt == 31) && (opc == (int32)OP_ANDI) )
        {   bitop = OP_BCLRS;
            n = ~n;
        }
    }
  }

    if (bitop != -1)
       outinstr(bitop, EA_Dn+Mregnum(tr),bitnum(n));
    else if (!scc && ((n & 0xffff0000) == 0 && opc != (int32)OP_ANDI ||
                      (n & 0xffff0000) == 0xffff0000 && opc == (int32)OP_ANDI))
    {   outinstr(opc, WORD0, EA_Dn+Mregnum(tr)); eafield(EA_ImmW,n);
    }
    else
    {   outinstr(opc, LONG0, EA_Dn+Mregnum(tr)); eafield(EA_ImmL,n);
    }
    move_register(r1,tr);
}

#ifdef TARGET_IS_68020
static void fpregisterop( int32 op, RealRegister r1r, RealRegister r2r,
                          RealRegister mr )
{
    if( r1r == mr ) swapregs(mr,r2r);
    move_fpregister(r1r,r2r)
    outfpinstr(op,0, R, fpregnum(mr), fpregnum(r1r));
}

static void fpconstantop(int32 op, RealRegister r1r, RealRegister r2r,
                        FloatCon *m, int32 size, bool r1_used )
{

   if( r1_used ) move_fpregister(r1r,r2r);
   outfpinstr(op, EA_Imm, M, size, fpregnum( (r1_used)? r1r: r2r) );

   if(size == FPF_D)
       eafield(EA_ImmD, m->floatbin.db.msd, m->floatbin.db.lsd);
   else
       eafield(EA_ImmS, m->floatbin.fb.val);

}
static void muldiv_register(RealRegister r1, RealRegister r2,RealRegister r3,
                            int32 op, bool signop)
{
    RealRegister tr,sr;
/* Make r1 = r2 if possible */
    if( r1 == r3 ) swapregs(r2,r3); /* This won't happen on J_DIVR */

    if( addrreg(r1) ) tr = R_DS;
    else tr = r1;

    if( addrreg(r3) )
    {   if( tr == R_DS )
        {   sr = R_A1;
            exchange(sr, r3);
        }
        else
        {   move_register(R_DS, r3);
            r3 = R_DS;
            sr = r3;
        }
    }
    else sr = r3;
    
    move_register(tr,r2);
    outinstr(op, EA_Dn+Mregnum(sr), (Mregnum(tr) << 12) + Mregnum(tr) +
                                    (signop?0x800:0) );
    if( tr != r1 ) move_register(r1,tr);
    if( sr != r3 ) exchange(sr, r3);
}

static void muldiv_integer(RealRegister r1, RealRegister r2, int32 n,
                           int32 op, bool signop)
{
    RealRegister tr = addrreg(r1) ? R_DS : r1;

    move_register(tr,r2);
    outinstr(op, EA_Imm, (Mregnum(tr) << 12) + Mregnum(tr) +
                         (signop? 0x800:0) );
    outL( n );
    if( tr != r1 ) move_register(r1,tr);
}

static void rem_integer(RealRegister r1, RealRegister r2,
                           int32 m, bool signop)
{
   RealRegister tr,qr= R_DS;

   move_register(qr, r2);

   if( addrreg(r1) )
   {  tr = R_A1;
      move_register(R_AS, R_A1);        /* use R_AS to save/restore A1  */
   }
   else
      tr = r1;

   outinstr(OP_DIVL, EA_Imm, (Mregnum(qr) << 12) + Mregnum(tr) +
                         (signop? 0x800:0) );
   outL(m);

   if( addrreg(r1) )
   {  move_register(r1,tr);
      move_register(R_A1, R_AS);
   }
}

static void rem_register(RealRegister r1, RealRegister r2,
                           RealRegister mr, bool signop)
{
   /* remainder requires 3 D-registers.  This is a pain.  Put the       */
   /* (corrupted) dividend into R_DS, and then use D0(R_A1) and         */
   /* (on HELIOS) D1(R_A2) or (on UNIX) D2(R_V1) if necessary.          */

   RealRegister tr,qr=R_DS;
   RealRegister er;

   move_register(qr, r2);

   if( addrreg(r1) )
   {  tr = (mr == R_A1) ? R_DS2 : R_A1;
      move_register(R_AS, tr);
   }
   else
      tr = r1;

   if( addrreg(mr) )
   {  er = (tr == R_A1) ? R_DS2 : R_A1;
      exchange(er, mr);
   }
   else
      er = mr;

   outinstr(OP_DIVL, EA_Dn+Mregnum(er), (Mregnum(qr) << 12) + Mregnum(tr) +
                         (signop? 0x800:0) );

   if( addrreg(r1) )
   {  move_register(r1,tr);
      move_register(tr, R_AS);
   }

   if( er != mr ) exchange(er, mr);
}

#else  /* plain 68000 */
static void multiply_integer(RealRegister r1, RealRegister r2, int32 n)
{
    RealRegister tr;
    int32 mtr;
    bool clash = r1 == r2;
    bool negated=0;
    int32 lowword;
    int32 hiword;

    if( n < 0 ) n = -n, negated++;

    if( word_sized(n) ) 
       hiword = 0,
       lowword = n;   /* hi word not needed, but zero to be tidy */
    else
    {
       hiword = n >> 16;
       lowword = (signed short)(n & 0xffff);
    }

/* Think of a temporary data register distinct from r2 */
    if( addrreg(r1) || clash )
        tr = R_DS;
    else tr = r1;

    mtr = Mregnum(tr);

    move_register(tr,r2);
/* We now have two copies of the register in tr & r2 since they must */
/* be distinct */

/* The next line saves the bottom word of the register if the 3rd      */
/* multiply is required and the low word of r2 cannot be used as its   */
/* source since it will be trashed or since it is an address reg       */
    if( !word_sized(n) && (clash || addrreg(r2)) )
            genmove(EA_AnPre+Mregnum(R_SP), 0, WORD2,
                         addrmode(r2)+Mregnum(r2), 0);

/* If there is a clash and the destination is a data reg then we can   */
/* form the first result in the result reg thereby avoiding            */
/* a register exchange                                                 */
    if( clash && datareg(r1))
    {   tr = r1;
        mtr = Mregnum(tr);
        r2 = R_DS;
    }

/* Form the first product in tr */
    if( lowword != 1 )
    {  outinstr(OP_SWAP,mtr);
       outinstr(OP_MULU,mtr,EA_Imm); eafield(EA_ImmW, lowword );
       outinstr(OP_SWAP,mtr);
    }
    outinstr(OP_CLR,WORD0,EA_Dn+mtr);

/* Put the first product in the result reg and low word of src reg */
/* in R_DS                                                         */
    if( !clash )
    {   move_register(r1,tr);
        move_register(R_DS,r2);
    }
    else
    {   if( !datareg(r1) ) /* && clash */
            exchange(tr, r1);
    }

/* Form second product in R_DS */
    outinstr(OP_MULU,Mregnum(R_DS),EA_Imm);
    eafield(EA_ImmW, lowword);

/* Add to result */
    opr2_register(r1,r1,R_DS,OP_ADD,OP_ADDAL);

    if( !word_sized(n) )
    {
/* Load high part of integer */
        load_integer(R_DS, hiword);

/* Form third product in R_DS */
        if( clash || !datareg(r2))
            outinstr(OP_MULU,Mregnum(R_DS),EA_AnPost+Mregnum(R_SP));
        else
            outinstr(OP_MULU,Mregnum(R_DS), EA_Dn+Mregnum(r2));
        outinstr(OP_SWAP,Mregnum(R_DS));
        outinstr(OP_CLR,WORD0,EA_Dn+Mregnum(R_DS));

/* Et viola */
        opr2_register(r1,r1,R_DS,OP_ADD,OP_ADDAL);
    }
    if( negated )
       op_register(r1,r1,OP_NEG);
}
#endif /* plain 68000 */

static void compare_integer(RealRegister r, int32 n)
{
/* Compare register r with integer n, setting condition codes            */
    int32 Mreg = Mregnum(r);

    if( n == 0 )
    {
        if( datareg(r) )
        {   outinstr(OP_TST,LONG0,EA_Dn+Mregnum(r));
            return;
        }
        else
        {   move_register(R_DS, r);
            return;
        }
    }
    if( addrreg(r) )
       outinstr(OP_CMPAL,Mreg,EA_Imm);
    else
       outinstr(OP_CMPI,LONG0,EA_Dn+Mregnum(r));
    eafield(EA_ImmL,n);
}

static void stack(int32 m)
{
    m &= M_CPUREGS;
    if(bitcount(m) == 1)
    {   int32 r = bitnum(m);
        genmove(EA_AnPre+Mregnum(R_SP),0,LONG2,((r>=8)?EA_An:EA_Dn)+(r&7),0);
    }
    else
        if(m)
            outinstr(OP_MOVEM, TOMEM, LONG1, EA_AnPre+Mregnum(R_SP),bitreverse(m));
}

#ifdef TARGET_IS_68020
static int32 savefpregs( int32 m )
{
   int32 m1;

   if( (m1 = m & M_FVARREGS)== 0 ) return 0;

   m1 >>= R_F0;

   outfpinstr(OP_FMOVEM, EA_AnPre+Mregnum(R_SP),
                  1, 0, m1);

   return( bitcount(m1)*3 );     /* No. of  lwords saved */
}

static void restorefpregs( int32 m )
{
   int32 m1;

   if( (m1 = m & M_FVARREGS)== 0 ) return;

   m1 >>= R_F0;

   outfpinstr(OP_FMOVEM, EA_AnPost+Mregnum(R_SP),
                  0, 2, bitreverse(m1) >> 8);
}
#endif

static void routine_entry(int32 m)
{
/* I think a little explanation maybe necessary here!

   There are three cases :-

1) LEAF procedure ( All variables/arguments are optimised into regs )

Enter :                    Use :

      |        |              |        |
      |--------|              |--------|
 SP-> |   RPC  |              |   RPC  |
      ----------              |--------|
 Args in regs D0-Dm      SP-> | regmask|
 where m <= NARGREGS          ----------

      Entry sequence :
         MOVEM.L  #(regmask&M_VARREGS),-(SP)

      Exit sequence :
         MOVEM.L  (SP)+,#(regmask&M_VARREGS)
         RTS

2) nargswords <= NARGREGS (non-LEAF)

Enter :                    Use :

      |        |              |        |
      |--------|              |--------|
 SP-> |   RPC  |              |   RPC  |
      ----------              |--------|
                         FP-> |   LFP  |
                              |--------|
   Args in regs D0-D(m-1)     | regmask|
   where m <= NARGREGS        |--------|
                              |  args  |
                              |--------|
                         SP-> | locals |
                              ----------

      Entry sequence :
         LINK     FP,#0
         MOVEM.L  #(regmask&M_VARREGS)|(usedargregs),-(SP)

      Exit sequence :
         LEA.L    #-bitcount(regmask)*4(FP),SP
         MOVEM.L  (SP)+,#(regmask&M_VARREGS)|FP
         RTS

3) nargwords > NARGREGS -- we could optimise this if !PROC_ARGADDR

Enter :                    Use :

      |  Arg5  |              |  Arg5  |
      |--------|              |--------|
 SP-> |   RPC  |              |  Arg4  |
      ----------              |--------|
                              |  Arg3  |
 First NARGREGS args in       |--------|
 D0-D3                        |  Arg2  |
                              |--------|
                              |  Arg1  |
                              |--------|
                              |   RPC  |
                              |--------|
                         FP-> |   LFP  |
                              |--------|
                              | regmask|
                              |--------|
                         SP-> | locals |
                              ----------

      Entry sequence :
         MOVEA.L  (SP)+,A0
         MOVEM.L  D0-D3,-(SP)
         MOVE.L   A0,-(SP)
         LINK     FP,#0
         MOVEM.L  (regmask&M_VARREGS),-(SP)

      Exit sequence :
         LEA.L    #-bitcount(regmask)*4(FP),SP
         MOVEM.L  (SP)+,#(regmask&M_VARREGS)|FP
         MOVE.L   (SP),16(SP)
         LEA.L    16(SP),SP
         RTS

   or on the MC68010/020

         LEA.L    #-bitcount(regmask)*4(FP),SP
         MOVEM.L  (SP)+,#(regmask&M_VARREGS)|FP
         RTD      #16

*/
/*    if (debugging(DEBUG_X)) desperate_codetrace(); */

    fp_minus_sp = 0;

    uses_returnlab = 0;

    returnlab = nextlabel();

    if (m < 0) syserr(syserr_enter, (long)m);

    if( procflags & PROC_ARGADDR ) m = NARGREGS+1;

    nargwords = m;
    savedmask = make_m68_mask(regmask & M_VARREGS );
    savedwords = bitcount(savedmask);

    if (!(procflags & NONLEAF))   /* if leaf */
    {   stack(savedmask);
#ifdef TARGET_IS_68020
        savedwords += savefpregs(regmask);
        savedmask  |= (regmask & M_FVARREGS);
#endif
    }
    else
    {
        if (m <= NARGREGS)
        {   int32 argmask = make_m68_mask(regbit(R_A1+m) - regbit(R_A1)) ;
            outinstr(OP_LINK, Mregnum(R_FP), 0);
            stack(argmask | savedmask);
        }
        else
        {   int32 argmask = make_m68_mask(M_ARGREGS) ;
            if (argmask)
	      {
		genmove(EA_An+Mregnum(R_AS),0,LONG2,EA_AnPost+Mregnum(R_SP),0);
                stack(argmask);
                genmove(EA_AnPre+Mregnum(R_SP),0,LONG2,EA_An+Mregnum(R_AS),0);
            }
            outinstr(OP_LINK, Mregnum(R_FP), 0);
            stack(savedmask);
        }

#ifdef TARGET_IS_68020
        savedwords += savefpregs(regmask);
        savedmask  |= (regmask & M_FVARREGS);
#endif
#ifndef TARGET_IS_HELIOS
#ifndef TARGET_IS_UNIX
        if ((procflags & STACKCHECK) && !no_stack_checks)
        {   Symstr *name = stackoverflow;
            add_integer(R_AS,R_SP,-greatest_stackdepth);
            codexrefs = (CodeXref *)
               global_list4(SU_Other, codexrefs,X_absreloc+codebase+codep+2,stacklimitname,0);
            obj_symref(stacklimitname,xr_code,0);
            outinstr(OP_CMPAL, Mregnum(R_AS), EA_AbsL);
            eafield(EA_AbsL,0);
            outinstr(OP_BCC,C_HI,6);
            call_k(name,0);
        }
#endif
#endif
    }
}

static void unstack(int32 m)
{
    m &= M_CPUREGS;
    if(bitcount(m) == 1)
    {   int32 r = bitnum(m);
        genmove(((r>=8)?EA_An:EA_Dn)+(r&7),0,LONG2,EA_AnPost+Mregnum(R_SP),0);
    }
    else
      if(m)
         outinstr(OP_MOVEM, TOREG, LONG1, EA_AnPost+Mregnum(R_SP), (m) );
}

/* Restore all registers for routine exit.  Normally really_return is set */
/* to give a return, but if unset no RTS is generated i.e. this is a tail */
/* call.                                                                  */
static void routine_exit(int32 condition, bool really_return)
{   condition = condition;  /* Keep compiler happy */
    if (procflags & NONLEAF)
    {
        /* The P_PEEPRET peephole trick rests on SP being reset here!   */
        if( savedmask != 0 ) 
        {   outinstr(OP_LEA, Mregnum(R_SP), EA_AnDisp+Mregnum(R_FP));
            eafield(EA_AnDisp, -savedwords*4  );

#ifdef TARGET_IS_68020
            restorefpregs( savedmask );
#endif
            unstack(savedmask | M_LDFP );
        }
        else
            outinstr(OP_UNLK,Mregnum(R_FP));

        if (nargwords > NARGREGS && NARGREGS > 0)
        {
#ifdef TARGET_IS_68020
            if (really_return)
            {   outinstr(OP_RTD, 4*NARGREGS);
                return;
            }
            else
#endif
            {
	      genmove(EA_AnDisp+Mregnum(R_SP), 4*NARGREGS, LONG2,
                        EA_AnInd+Mregnum(R_SP), 0);
                outinstr(OP_LEA, Mregnum(R_SP), EA_AnDisp+Mregnum(R_SP));
                eafield(EA_AnDisp,4*NARGREGS);
            }
        }
    }
    else
    {
#ifdef TARGET_IS_68020
        restorefpregs(savedmask);
#endif
        unstack(savedmask);
    }

    if (really_return)
        outinstr(OP_RTS);
}

static void constant_shift(int32 r1, int32 r2, int32 n, instrtype op)
{
/* r1 = r2 op n
    where op is OP_LSL, OP_LSR, OP_ASR */

    if( datareg(r1) )
    {
        move_register(r1,r2);
        if( (1<=n) && (n<=8) )
        {   outinstr(op, n&7, LONG0, 0, Mregnum(r1));
        }
        else
        {   load_integer(R_DS,n);
            outinstr(op, Mregnum(R_DS), LONG0, 1, Mregnum(r1) );
        }
        return;
    }
    else    /* r1 is address register */
    {
/* This is a bit tricky because we can't shift an address reg.
   So simplify the problem by exchanging with a data reg, shifting,
   and exchanging back again ! */
        if( n <= 8 )
        {
            move_register(R_DS,r2);
            constant_shift(R_DS,R_DS,n,op);
            move_register(r1,R_DS);
        }
        else
        {   move_register(r1,r2);
            exchange(R_A1, r1);
            constant_shift(R_A1,R_A1,n,op);
            exchange(R_A1, r1);
        }
    }
}

static void register_shift(int32 r1, int32 r2, int32 mr, instrtype op)
{
/* r1 = r2 op mr
    where op is OP_LSL, OP_LSR, OP_ASR */

    if( datareg(r1) )
    {
        move_register(r1,r2);
        if( datareg(mr) )
            outinstr(op, Mregnum(mr), LONG0, 1, Mregnum(r1));
        else
        {
            move_register(R_DS, mr);
            outinstr(op, Mregnum(R_DS), LONG0, 1, Mregnum(r1));
        }
        return;
    }
    else    /* r1 is address register */
    {
/* This is a bit tricky because we can't shift an address reg.
   So simplify the problem by exchanging with a data reg, shifting,
   and exchanging back again ! */
        RealRegister exgreg = (mr == R_A1) ? R_DS2 : R_A1;
        move_register(r1,r2);
        exchange(exgreg, r1);
        register_shift(exgreg,exgreg,mr,op);
        exchange(exgreg, r1);
    }
}

static void genloadorstore(int32 ea1, int32 ext1, int32 size,
                    int32 ea2, int32 ext2, int32 load)
{
    if (debugging(DEBUG_LOCALCG))
       cc_msg("%s(%ld), %lx(%lx), %lx(%lx)\n", load?"ld":"st", (long)size,
              (long)ea1, (long)ext1, (long)ea2, (long)ext2);

    if( load )
        genmove(ea1,ext1,size,ea2,ext2);
    else
        genmove(ea2,ext2,size,ea1,ext1);
}

#ifdef TARGET_IS_68020
static void fp_mem_indexed(RealRegister r1r, RealRegister r2r,
                           RealRegister ireg, int32 m, bool loadop,
                           int32 opsize, int32 shift)
{   int32 ea, ext = 0;

    if( ireg != -1 )
    {
        if (addrreg(ireg) && (shift == 0)) swapregs(ireg, r2r);

        if( datareg(r2r) )
        {   move_register(R_AS, r2r);
            r2r = R_AS;
        }
    }

    if( datareg(r2r) )
    {   ireg = r2r;
        r2r = -1;
        ea = EA_AnIndx;
    }
    else
    {
        if( r2r != -1 && ireg != -1 ) ea = EA_AnIndx;
        else
            ea = ( !word_sized(m) )? EA_AnIndx:
                  ( (m == 0)? EA_AnInd: EA_AnDisp);
    }

    if( ea == EA_AnIndx )
    {   ext = EM_L;

        if( r2r == -1 )
            ext |= EM_FULL+EM_BS+EM_IS0;
        else
            ea |= Mregnum(r2r);

        if( ireg == -1 )
            ext |= EM_FULL+EM_IS+EM_IS0;
        else
        {   if( shift != 0 )
               ext |= EM_FULL+EM_IS0+(shift<<9);

            ext |= (addrreg(ireg)? EM_A: EM_D)+(Mregnum(ireg)<<12);
        }
        if( !byte_sized(m) )
            ext |= EM_FULL+EM_IS0;
    }
    else
        ea |= Mregnum(r2r);

    if( loadop )
        outfpinstr(OP_FMOVETOFP, ea, M, opsize, fpregnum(r1r));
    else
        outfpinstr(OP_FMOVEFROMFP, ea, opsize, fpregnum(r1r), 0);

    if( (ea & 070) == EA_AnIndx )
        eafield( ea, ext, m, 0);
    else
        eafield( ea, m );
}
#endif

#ifdef TARGET_HAS_BLOCKMOVE
/*
   r2 is corruptible
   n  is a multiple of 4
*/
#define MINLOOPSIZE  3
#define MAXLOOPSIZE  6
/* The next line defines the maximum amount for transfers
   to be done by register displacement */
#define MAXOFFSETMOVE 3

static void blockmove(RealRegister r1, RealRegister r2, int32 n, int32 dead)
{  int swapped      = 0;
   RealRegister tr1,tr2;
   int32 loops      = 0;
   int32 loopsize   = 0;
   int32 remainder    = MINLOOPSIZE+1;
   int32 saver        = 0;
   int32 i;
   int32 f            = 0;

   n /= 4;                   /* reduce to no. of 4 byte words */

   if( n <= MAXOFFSETMOVE )
   {
      loops = 0;
      remainder = n;
   }
   else
   {
      for( i = MINLOOPSIZE; i <= MAXLOOPSIZE; i++ )
      {
         int32 l = n/i;         /* No. of loops for this loop size */
         int32 r = n%i;         /* Amount left over after l loops  */
         int32 thisf;
         int32 nins, ncode;

         if( l == 1 )
         {  l = 0;
            r += i;
         }

/* This minimisation function looks pretty nasty.           */
/* The first term is the number of instructions executed    */
/* over and above the ideal number. The second term is      */
/* the number of code instructions over and above the ideal */
/* number i.e. 2                                            */
         nins  = l*(i+1)+r+(l?1:0)-n;
         ncode = r + (l?(i+2):0) - 2;

         thisf = nins+ncode;
   
         if( (thisf < f) || (i == MINLOOPSIZE) )
         {
            remainder = r;
            loops     = l;
            loopsize  = i;
            f         = thisf;
         }
      }
   }

/* Now sort out which register is to be used to point to */
/* the destination                                       */
   if (datareg(r1) || (n > MAXOFFSETMOVE && !(dead & J_DEAD_R1)))
   {  move_register(R_AS, r1);
      tr1 = R_AS;
   }
   else
      tr1 = r1;

/* And the source */
   if (datareg(r2) || (n > MAXOFFSETMOVE && !(dead & J_DEAD_R2)))
   {  if (tr1 == R_AS)
      {  if (datareg(r2))
         {  tr2 = R_AS2;   /* tr1 == R_AS therefore R_AS2 may be swapped */
            swapped = 1, exchange(r2, tr2);
         }
         else
            tr2 = r2;
         if (!(dead & J_DEAD_R2)) saver = regbit(tr2);
      }
      else
      {  move_register(R_AS, r2);
         tr2 = R_AS;
      }
   }
   else
      tr2 = r2;

   stack(make_m68_mask(saver));

/* The generated loop will shift a minimum of .5M since DBRA only
   works on the low word  of the counter and the minimum amount
   shifted per loop is 3 long words. However, that is likely
   to be enough to be going on with.
*/
   if( loops != 0 )
   {  int i;
      load_integer(R_DS, loops-1l);  /* -1 for DBRA */

      setlabel(nextlabel());

      for( i = 0; i < loopsize; i++)
         outinstr(OP_MOVE, LONG2, reversea(EA_AnPost+Mregnum(tr1)),
                                           EA_AnPost+Mregnum(tr2)  );
      outinstr(OP_DBCC, C_NEVER, Mregnum(R_DS));
      outRR( (-(loopsize*2+2)) & 0xffff);
   }

   if (n <= MAXOFFSETMOVE)
   {  int i;
      outinstr(OP_MOVE, LONG2, reversea(EA_AnInd+Mregnum(tr1)),
                                        EA_AnInd+Mregnum(tr2)  );
      for( i = 1; i < n; i++)
      {  outinstr(OP_MOVE, LONG2, reversea(EA_AnDisp+Mregnum(tr1)),
                                           EA_AnDisp+Mregnum(tr2)  );
	 eafield(EA_AnDisp,i*4);
         eafield(EA_AnDisp,i*4);
      }
   }
   else
      while( remainder-- )
      {  int mode = (remainder == 0)? EA_AnInd: EA_AnPost;
         outinstr(OP_MOVE, LONG2, reversea(mode+Mregnum(tr1)),
                                           mode+Mregnum(tr2)  );
      }

   unstack(make_m68_mask(saver));

   if( swapped )
      exchange(tr2,r2);
}

static void blockzap(RealRegister r1, int32 n, int32 dead)
{
   RealRegister tr1 =-1;
   int32 loops      = 0;
   int32 loopsize   = 0;
   int32 remainder    = MINLOOPSIZE+1;
   int32 i;
   int32 f            = 0;

   n /= 4;                   /* reduce to no. of 4 byte words */

   if( n <= MAXOFFSETMOVE )
   {
      loops = 0;
      remainder = n;
   }
   else
   {
      for( i = MINLOOPSIZE; i <= MAXLOOPSIZE; i++ )
      {
         int32 l = n/i;         /* No. of loops for this loop size */
         int32 r = n%i;         /* Amount left over after l loops  */
         int32 thisf;
         int32 nins, ncode;

         if( l == 1 )
         {  l = 0;
            r += i;
         }

/* This minimisation function looks pretty nasty.           */
/* The first term is the number of instructions executed    */
/* over and above the ideal number. The second term is      */
/* the number of code instructions over and above the ideal */
/* number i.e. 2                                            */
         nins  = l*(i+1)+r+(l?1:0)-n;
         ncode = r + (l?(i+2):0) - 2;

         thisf = nins+ncode;
   
         if( (thisf < f) || (i == MINLOOPSIZE) )
         {
            remainder = r;
            loops     = l;
            loopsize  = i;
            f         = thisf;
         }
      }
   }

/* Now sort out which register is to be used to point to */
/* the destination                                       */
   if (datareg(r1) || (n > MAXOFFSETMOVE && !(dead & J_DEAD_R1)))
   {  move_register(R_AS, r1);
      tr1 = R_AS;
   }
   else
      tr1 = r1;

/* The generated loop will shift a minimum of .5M since DBRA only
   works on the low word  of the counter and the minimum amount
   shifted per loop is 3 long words. However, that is likely
   to be enough to be going on with.
*/
   if( loops != 0 )
   {  int i;
      load_integer(R_DS, loops-1l);  /* -1 for DBRA */

      setlabel(nextlabel());
      for( i = 0; i < loopsize; i++)
         outinstr(OP_CLR, LONG0, EA_AnPost+Mregnum(tr1));
      outinstr(OP_DBCC, C_NEVER, Mregnum(R_DS));
      outRR( (-(loopsize*2+2)) & 0xffff);
   }

   if (n <= MAXOFFSETMOVE)
   {  int i;
      outinstr(OP_CLR, LONG0, EA_AnInd+Mregnum(tr1));
      for( i = 1; i < n; i++)
      {  outinstr(OP_CLR, LONG0, EA_AnDisp+Mregnum(tr1));
         eafield(EA_AnDisp,i*4);
      }
   }
   else
      while( remainder-- )
      {  int mode = (remainder == 0)? EA_AnInd: EA_AnPost;
         outinstr(OP_CLR, LONG0, mode+Mregnum(tr1));
      }
}
#endif  /* TARGET_HAS_BLOCKMOVE */

static void mem_indexed(RealRegister r1, RealRegister r2, int32 off,
                        int32 size, bool unsign, RealRegister ireg,
                        bool load, int32 shift, bool absaddr)
{
    RealRegister tr;        /* The data value temp reg */
    int32 eatr;

    if( ireg != -1 )
    {
#ifdef never
    {                       /* keep topcc tool happy. */
#endif
/* First make r2 the address reg if possible */
        if( addrreg(ireg) && (shift == 0)) swapregs(ireg,r2);
#ifndef TARGET_IS_68020
    }
#endif
        if (!absaddr && datareg(r2))
        {
            move_register(R_AS,r2);
            r2 = R_AS;
        }
#ifdef TARGET_IS_68020
    }
#endif
/* At this point for the 68000 r2 is an address register.
   For the 68020 r2 may be a data register if it is not an indexed op.
*/
    if( load )
    {   if( unsign )
        {   if( size != LONG2 )
            {
                tr = (r1==R_STACK || addrreg(r1) || (!absaddr &&
                         (r1 == r2 || r1 == ireg))) ? R_DS : r1;
                load_integer(tr,0);
            }
            else
                syserr(syserr_ldrrk), tr = r1;
        }
        else /* signed */
        {
            if (r1==R_STACK || !(addrreg(r1) && size == BYTE2)) tr = r1;
            else tr = R_DS;
        }
    }
    else  /* store command */
    {
        /* store cannot have r1==R_STACK                                */
        if( size == BYTE2 && addrreg(r1) )
            move_register(R_DS,r1), tr = R_DS;
        else
            tr = r1;
    }

    eatr = r1==R_STACK ? EA_AnPre+Mregnum(R_SP) : addrmode(tr)+Mregnum(tr);

    if (absaddr)
    {   Symstr *name = (Symstr *)off; int32 offset = r2;
        codexrefs = (CodeXref *)global_list4(SU_Other,
                        codexrefs,X_absreloc+codebase+codep+2,name,offset);
        genloadorstore(eatr, 0, size, EA_AbsL, offset, load);
    }
    else
#ifdef TARGET_IS_68020
    if (datareg(r2) || (shift != 0) || !word_sized(off))  /* 020 specials */
    {   int32 extword = 0;
        int32 mode    = 0;

        if (ireg == -1)
        {  mode    = EA_AnIndx; /* No reg no. needed because EM_BS */
           extword = (datareg(r2)?EM_D:EM_A)+(Mregnum(r2)<<12)+
                         EM_L+EM_TIMES1+EM_FULL+EM_BS+EM_IS0;
        }
        else
        {  mode    = EA_AnIndx+Mregnum(r2);
           extword = (datareg(ireg)?EM_D:EM_A)+(Mregnum(ireg)<<12)+
                         EM_L+(shift<<9)+EM_FULL+EM_IS0;
        }

        if( load )
        {  outinstr(OP_MOVE, size, reversea(eatr), mode);
           eafield(EA_AnIndx, extword, off ); 
        }
        else
        {
           outinstr(OP_MOVE, size, reversea(mode), eatr);
           eafield(EA_AnIndx, extword, off );
        }
    }
    else
#endif
    {
        if( ireg == -1 )                /* MOVE.x d(An),Rt */
        {
	  if (off == 0)               /* MOVE.x (An),Rt */
                genloadorstore(eatr, 0, size,
                               EA_AnInd+Mregnum(r2),0, load);
            else
	      {
		if (!word_sized( off ))
		  {
		    /* XXX - CHECK THIS CODE !!!!!!!! */
		    
		    load_integer( R_AS, off );
          
		    if( load )                  /* MOVE.x off(An,Ri),Rt */
		      {
			outinstr(OP_MOVE, size,
				 reversea(eatr),
				 EA_AnIndx+Mregnum(r2));
		      }
		    else
		      {
			outinstr(OP_MOVE, size,
				 reversea(EA_AnIndx+Mregnum(r2)),
				 eatr  );
		      }
		    
		    eafield(EA_AnIndx, EM_L + (addrreg(R_AS)?EM_A:EM_D)+
			    (Mregnum(R_AS)<<12),0);
		  }
		else
		  {
		    genloadorstore(eatr, 0, size,
                               EA_AnDisp+Mregnum(r2),off, load);
		  }
	      }
        }
        else
        {   /* Currently this is only called so that off==0 here.       */
#ifndef TARGET_IS_68020
          if (!byte_sized(off))         /* LEA.L  0(An,Ri),AS */
          {                             /* MOVE.L off(AS),Rt  */
              outinstr(OP_LEA, Mregnum(R_AS), EA_AnIndx+Mregnum(r2), 0);
              eafield(EA_AnIndx,
                      (addrreg(ireg)?0x8800:0x800)+(Mregnum(ireg)<<12));
            genloadorstore( eatr, 0, size, EA_AnDisp+Mregnum(r2),off,load);
          }
          else
#endif
          { if( load )                  /* MOVE.x off(An,Ri),Rt */
            {   outinstr(OP_MOVE, size,
                               reversea(eatr),
                               EA_AnIndx+Mregnum(r2));
            }
            else
            {   outinstr(OP_MOVE, size,
                               reversea(EA_AnIndx+Mregnum(r2)),
                               eatr  );
            }
            eafield(EA_AnIndx, EM_L + (addrreg(ireg)?EM_A:EM_D)+
                           (Mregnum(ireg)<<12),off);
          }
        }
    }
/* We can have only loaded a Word or Long into an addrreg and the
   cpu sign extends them when they get loaded with a word */
    if( load )
    {   if( !unsign && size != LONG2 && datareg(tr))
        {
            if( size == BYTE2 )
            {
#ifdef TARGET_IS_68020
                outinstr(OP_EXTBL, Mregnum(tr));
            }
            else
#else   /* plain 68000 */
                outinstr(OP_EXTW, Mregnum(tr));
            }
#endif
            outinstr(OP_EXTL,Mregnum(tr));
        }
    
        if(r1 != tr) move_register(r1, tr);
    }
}

static void show_inst_direct(J_OPCODE op, RealRegister r1, RealRegister r2,
                             int32 m, int32 peep)
/* The types of the arguments here are rather unsatisfactory - in        */
/* particular the last one (m) is really a big union.                    */
{
/*    struct DispDesc dispdesc; */
    RealRegister r1r = (int32)r1, r2r = (int32)r2, mr = (int32)m;

#ifdef TARGET_HAS_SCALED_ADDRESSING
    int32 msh = (op&J_SHIFTMASK)>>J_SHIFTPOS;
#else
    int32 msh = 0;
#endif

/* Should this really be arranged as:
 *  union { Symstr *sym; int32 umint; } um;
 *  um.umint = m;
 */

    if (debugging(DEBUG_CG))
    {   cc_msg("GEN: ");
        jopprint_opname(op);
        cc_msg("%ld %ld %ld\n", (long)r1r, (long)r2r, (long)mr);
    }
    if ((codep>>2)>=mustlitby) dumplits2(1);
    if (uses_r1(op) && r1r >= 24L) syserr(syserr_r1r, (long)r1r);
    if (!(peep & P_ADCON) && uses_r2(op) && r2r >= 24L)
        syserr(syserr_r2r, (long)r2r);
    if (uses_r3(op) && mr >= 24L) syserr(syserr_mr, (long)mr);
/* The next line stops the non-optimisation of branches in a switch */
/* branchtable. Which HAVE to be 4 bytes long!                      */
/* AM does not understand why this needs to be done, but note that  */
/* we do not need to mask out DEADBITS since !used_r1/r2/r3.        */
    if( (op != J_BXX) &&
        (op & ~(Q_MASK | J_SIGNED | J_UNSIGNED ) ) != J_B )
      switchbranch = 0;

/* to enable future JOPCODE peephole optimisation expand_jop_macros()
   tries quite hard not to call show_inst() with instructions with
   no effect.
*/
    /* illbits (and peep) checks that unexpected (erroneous!) bits are not set */
    illbits = op & (Q_MASK | J_SIGNED | J_UNSIGNED
#ifdef TARGET_HAS_SCALED_ADDRESSING
                           | J_NEGINDEX | J_SHIFTMASK
#endif
                   );

    switch (op & ~(Q_MASK | J_SIGNED | J_UNSIGNED
#ifdef TARGET_HAS_SCALED_ADDRESSING
                          | J_NEGINDEX | J_SHIFTMASK
#endif
                  | J_DEADBITS))
    {
case J_NOOP:
        break;
#ifdef TARGET_HAS_BLOCKMOVE
case J_MOVC:
        blockmove(r1,r2,m, op&J_DEADBITS);
        break;
case J_CLRC:
        blockzap(r1,m, op&J_DEADBITS);
        break;
#endif
case J_CMPK:
/* N.B. that CMPK can use R_IP as a work register               */
        compare_integer(r2r, m );
        illbits &= ~Q_MASK;
        break;
/* N.B. having someone (e.g. show_instruction) changing the next 3 ops into */
/* ADDK 0; RSBK 0; and RSBK -1; would simplify peepholing them with CMP 0   */
/* and remove 3 cases from this table but using more general code           */
/* No - a better idea: use the peepholer to squash shifts and ops together  */
/* (via V_INTERNAL reg), but also turn solitary shifts into J_MOV+J_SHIFTM  */
/* N.B. This would make J_SHIFTM a P_PEEPn bit local to this file           */
case J_MOVR:
        if (r1r==mr) syserr(syserr_remove_noops);
        move_register(r1r,mr);
        if (r1r == R_STACK) fp_minus_sp += 4, peep &= ~P_PEEPPUSH;
        else if (datareg(r1r)) peep &= ~P_PEEPCMPZ;
        break;
case J_NEGR:
        op_register(r1r,mr,OP_NEG);
        break;
case J_NOTR:
        op_register(r1r,mr,OP_NOT);
        break;
case J_MOVK:
        load_integer(r1r, m);
        if (r1r == R_STACK) fp_minus_sp += 4, peep &= ~P_PEEPPUSH;
        break;

case J_ADDK:
        add_integer(r1r, r2r, m);
        if (datareg(r1r) && m != 0) peep &= ~P_PEEPCMPZ;
        break;
case J_SUBK:
        add_integer(r1r, r2r, -m);
        if (datareg(r1r) && m != 0) peep &= ~P_PEEPCMPZ;
        break;
case J_ANDK:
        if (peep & P_PEEPBTST) bit_integer(r1r, r2r, m);
        else op_integer(r1r, r2r, m, OP_ANDI, (peep & P_PEEPCMPZ) != 0);
        if (datareg(r1r)) peep &= ~(P_PEEPCMPZ|P_PEEPBTST);
        break;
case J_ORRK:
        op_integer(r1r, r2r, m, OP_ORI, (peep & P_PEEPCMPZ) != 0);
        if (datareg(r1r)) peep &= ~P_PEEPCMPZ;
        break;
case J_EORK:
        op_integer(r1r, r2r, m, OP_EORI, (peep & P_PEEPCMPZ) != 0);
        if (datareg(r1r)) peep &= ~P_PEEPCMPZ;
        break;
case J_EXTEND:
        {   RealRegister tr = addrreg(r1r) ? R_DS : r1;
            move_register(tr, r2r);
            switch (m)
            {
    case 0:     /* byte -> word */
                outinstr(OP_EXTW, Mregnum(tr));
                break;
    case 1:     /* byte -> long */
#ifdef TARGET_IS_68020
                outinstr(OP_EXTBL, Mregnum(tr));
                break;
#else
                outinstr(OP_EXTW, Mregnum(tr));
                /* and drop through */
#endif
    case 2:     /* word -> long */
                outinstr(OP_EXTL, Mregnum(tr));
                break;
    default:    /* should never occur */
                break;
            }
            move_register(r1r, tr);
        }
        break;
case J_MULK:
#ifdef TARGET_IS_68020
        muldiv_integer(r1r,r2r,m,OP_MULL, (op & J_SIGNED)!=0);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
#else
        multiply_integer(r1r,r2r,m);
#endif
        break;
#ifdef TARGET_IS_68020
case J_MULR:
        muldiv_register(r1r,r2r,mr,OP_MULL, (op & J_SIGNED)!=0);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
        break;
case J_DIVK:
        muldiv_integer(r1r,r2r,m,OP_DIVL, (op & J_SIGNED)!=0);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
        break;
case J_DIVR:
        muldiv_register(r1r,r2r,mr,OP_DIVL, (op & J_SIGNED)!=0);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
        break;
case J_REMK:
        rem_integer(r1r,r2r,m, (op & J_SIGNED)!=0);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
        break;
case J_REMR:
        rem_register(r1r,r2r,m, (op & J_SIGNED)!=0);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
        break;
#endif
case J_SHRK:
        if (m!=0)
        {   if (m<=0 || m>32) syserr(syserr_silly_shift, (long)m);
            constant_shift(r1r, r2r, m, (op & J_SIGNED)? OP_ASRR: OP_LSRR);
            illbits &= ~(J_SIGNED|J_UNSIGNED);
            if (datareg(r1r) && m != 0) peep &= ~P_PEEPCMPZ;
            break;
        }
        /* drop through if shift by 0 */
case J_SHLK:
        if (m<0 || m>31) syserr(syserr_silly_shift, (long)m);
        constant_shift(r1r, r2r, m, OP_LSLR);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
        if (datareg(r1r) && m != 0) peep &= ~P_PEEPCMPZ;
        break;

case J_SHLR:
        register_shift(r1r,r2r,mr,OP_LSLR);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
        if (datareg(r1r)) peep &= ~P_PEEPCMPZ;
        break;
case J_SHRR:
        register_shift(r1r,r2r,mr,(op&J_SIGNED)?OP_ASRR: OP_LSRR);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
        if (datareg(r1r)) peep &= ~P_PEEPCMPZ;
        break;

#define irrop(op) \
        opr_register(r1r, r2r, mr, op); \
        if (datareg(r1r)) peep &= ~P_PEEPCMPZ;

case J_ANDR:
        irrop(OP_AND); break;
case J_ORRR:
        irrop(OP_OR); break;
case J_EORR:
        irrop(OP_EOR); break;
#undef irrop

#define ir2op(op1,op2) \
        opr2_register(r1r, r2r, mr, op1, op2); \
        if (datareg(r1r)) peep &= ~P_PEEPCMPZ;

case J_ADDR:
/* This processor does have one three address instruction :-            */
/*  LEA.L   0(An,Ri.L),Am                                               */
/* Try and make r1r == r2r                                              */
/* Issue LEA if r1r is distinct from r2r and they are both address regs */
/* otherwise the ADD route will be more efficient                       */
        if(addrreg(mr) && (r1r != r2r) ) swapregs(r2r,mr);
        if( addrreg(r1r) && addrreg(r2r) && (r1r != r2r) )
        {   outinstr(OP_LEA,Mregnum(r1r),EA_AnIndx+Mregnum(r2r));
            eafield(EA_AnIndx,(addrreg(mr)?0x8800:0x0800)+(Mregnum(mr)<<12), 0);
            break;
        }
        ir2op(OP_ADD, OP_ADDAL); break;
case J_SUBR:
        ir2op(OP_SUB, OP_SUBAL); break;
case J_RSBR:
        swapregs(r2,mr);
        ir2op(OP_SUB, OP_SUBAL); break;
case J_CMPR:
        compare_register(r2r,mr);
        illbits &= ~(Q_MASK);
        break;
#undef ir2op

case J_STACK:
        fp_minus_sp = m;
        break;
case J_SETSP:
        {   int32 diff, oldstack = (int32)r2, newstack = m;
            if (fp_minus_sp != oldstack)
                syserr(syserr_setsp_confused,
                       (long)fp_minus_sp, (long)oldstack, (long)newstack);
            diff = newstack - oldstack;
            fp_minus_sp = newstack;
            if (!((procflags & NONLEAF) && (peep & P_PEEPRET)))
            {   /* omit SP fixup before non-leaf return due to calling seq */
                add_integer(R_SP, R_SP, -diff);
            }
        }
        peep &= ~P_PEEPRET;
        break;

case J_ENTER:
        asm_lablist = 0;
        routine_entry(m);
        break;

case J_ENDPROC:
        if( uses_returnlab && unsetlab1_(returnlab) )
            conditional_branch_to(Q_AL, RETLAB, 0);
        cnop();
	dumplits2( 0 );	
        asm_lablist = (LabList *)dreverse((List *)asm_lablist);
        break;

case J_PUSHR:         /* EXPERIMENTAL_68000 */
        m = regbit(r1r);
        /* drop through */
case J_PUSHM:
        stack(make_m68_mask(m));
        fp_minus_sp += 4*bitcount(m);
        break;

case J_POPM:
        unstack(make_m68_mask(m));
        fp_minus_sp -= 4*bitcount(m);
        break;

case J_LABEL:
	{
	  LabelNumber *l = (LabelNumber *)m;
	  if (l->block == DUFF_ADDR && l != returnlab)
	    fprintf(stderr,"Unused label L%ld\n", lab_name_(l)&0x7fffffff);
	  else setlabel((LabelNumber *)m);
	}
        return;

#ifdef TARGET_HAS_SCCK
case J_SCCK:
        {   int32 tr = addrreg(r1r) ? R_DS : r1r;
#ifdef TARGET_IS_68020
            if (fpcmp)
                outinstr(OP_FSCC, EA_Dn+Mregnum(tr), FC_FROMQ(op & Q_MASK));
            else
#endif
                outinstr(OP_SCC, C_FROMQ(op & Q_MASK), EA_Dn+Mregnum(tr));
            if (m & 0xffffff00)
            {
#ifdef TARGET_IS_68020
                outinstr(OP_EXTBL, Mregnum(tr));
#else   /* plain 68000 */
                outinstr(OP_EXTW, Mregnum(tr));
                if (m & 0xffff0000) outinstr(OP_EXTL,Mregnum(tr));
#endif
            }
            op_integer(tr, tr, m, OP_ANDI, 0);
            move_register(r1r, tr);
        }
        illbits &= (~Q_MASK);
        break;
#endif

case J_B:
        conditional_branch_to(op & Q_MASK, (LabelNumber *)m, 0);
        illbits &= (~Q_MASK);
        break;

case J_BXX:
        conditional_branch_to(Q_AL, (LabelNumber *)m, 1);
        break;

case J_LDRK:
        mem_indexed(r1r,r2r,m,LONG2, (op&J_UNSIGNED)!=0,-1,1,0,(peep&P_ADCON)!=0);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
        if (r1r == R_STACK) fp_minus_sp += 4, peep &= ~P_PEEPPUSH;
        else if (datareg(r1r)) peep &= ~P_PEEPCMPZ;
        peep &= ~P_ADCON;
        break;

case J_LDRWK:
        mem_indexed(r1r,r2r,m,WORD2, (op&J_UNSIGNED)!=0,-1,1,0,(peep&P_ADCON)!=0);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
        peep &= ~P_ADCON;
        break;

case J_LDRBK:
        mem_indexed(r1r,r2r,m,BYTE2, (op&J_UNSIGNED)!=0,-1,1,0,(peep&P_ADCON)!=0);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
        peep &= ~P_ADCON;
        break;

case J_LDRR:
        mem_indexed(r1r,r2r,0,LONG2, (op&J_UNSIGNED)!=0,m,1,msh,0);
        illbits &= ~(J_SIGNED|J_UNSIGNED
#ifdef TARGET_HAS_SCALED_ADDRESSING
                     |J_SHIFTMASK
#endif   
        );
        if (r1r == R_STACK) fp_minus_sp += 4, peep &= ~P_PEEPPUSH;
        else if (datareg(r1r)) peep &= ~P_PEEPCMPZ;
        break;

case J_LDRWR:
        mem_indexed(r1r,r2r,0,WORD2, (op&J_UNSIGNED)!=0,m,1,msh,0);
        illbits &= ~(J_SIGNED|J_UNSIGNED
#ifdef TARGET_HAS_SCALED_ADDRESSING
                     |J_SHIFTMASK
#endif   
        );
        break;

case J_LDRBR:
        mem_indexed(r1r,r2r,0,BYTE2, (op&J_UNSIGNED)!=0,m,1,msh,0);
        illbits &= ~(J_SIGNED|J_UNSIGNED
#ifdef TARGET_HAS_SCALED_ADDRESSING
                     |J_SHIFTMASK
#endif   
        );
        break;

case J_STRK:
        mem_indexed(r1r,r2r,m,LONG2, (op&J_UNSIGNED)!=0,-1,0,0,(peep&P_ADCON)!=0);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
        peep &= ~P_ADCON;
        break;

case J_STRWK:
        mem_indexed(r1r,r2r,m,WORD2, (op&J_UNSIGNED)!=0,-1,0,0,(peep&P_ADCON)!=0);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
        peep &= ~P_ADCON;
        break;

case J_STRBK:
        mem_indexed(r1r,r2r,m,BYTE2, (op&J_UNSIGNED)!=0,-1,0,0,(peep&P_ADCON)!=0);
        illbits &= ~(J_SIGNED|J_UNSIGNED);
        peep &= ~P_ADCON;
        break;

case J_STRR:
        mem_indexed(r1r,r2r,0,LONG2, (op&J_UNSIGNED)!=0,m,0,msh,0);
        illbits &= ~(J_SIGNED|J_UNSIGNED
#ifdef TARGET_HAS_SCALED_ADDRESSING
                     |J_SHIFTMASK
#endif   
        );
        break;

case J_STRWR:
        mem_indexed(r1r,r2r,0,WORD2, (op&J_UNSIGNED)!=0,m,0,msh,0);
        illbits &= ~(J_SIGNED|J_UNSIGNED
#ifdef TARGET_HAS_SCALED_ADDRESSING
                     |J_SHIFTMASK
#endif   
        );
        break;

case J_STRBR:
        mem_indexed(r1r,r2r,0,BYTE2, (op&J_UNSIGNED)!=0,m,0,msh,0);
        illbits &= ~(J_SIGNED|J_UNSIGNED
#ifdef TARGET_HAS_SCALED_ADDRESSING
                     |J_SHIFTMASK
#endif   
        );
        break;

case J_STRING:
        load_string(r1r,(StringSegList *)m);

#ifdef TARGET_IS_HELIOS
	if (suppress_module == 1)
	  codeseg_stringsegs( (StringSegList *)m, 1 );
#endif
	
        break;

case J_CALLR:
        {   RealRegister cr;
            if( !addrreg(mr) )
            {
                move_register(R_AS,mr);
                cr = R_AS;
            }
            else cr = mr;
            outinstr(OP_JSR,EA_AnInd+Mregnum(cr));
            break;
        }

case J_CALLK:
            call_k((Symstr *)m,0);
            break;

case J_TAILCALLR:
        {   RealRegister cr;
            routine_exit(C_ALWAYS,0);
            if( !addrreg(r1r) )
            {
                move_register(R_AS,r1r);
                cr = R_AS;
            }
            else cr = r1r;
            outinstr(OP_JMP,EA_AnInd+Mregnum(cr));
            break;
        }

case J_TAILCALLK:
            routine_exit(C_ALWAYS,0);
            call_k((Symstr *)m,1);
            break;

case J_CASEBRANCH:
/* Case branches are always forwards so they will always be 16 offsets */
/* This is a lie -- consider l: ... switch (x) { case 7: goto l;}      */
#ifdef TARGET_IS_68020
            compare_integer(r1r,m-2);
            outinstr(OP_BCC, C_HI, 4);          /* bit idle */
            outinstr(OP_JMP, EA_PCInd);
            eafield(EA_PCInd, 0x0c00+(addrmode(r1r)+Mregnum(r1r)<<12) + 6);
#else
            move_register(R_DS,r1r),
            compare_integer(R_DS,m-2);
            outinstr(OP_BCC, C_HI, 6);          /* bit idle */
            constant_shift(R_DS,R_DS,2,OP_LSLR);
            outinstr(OP_JMP, EA_PCInd);
            eafield(EA_PCInd, 0x0800+(Mregnum(R_DS)<<12) + 6);
#endif
	    setlabel(nextlabel());
            switchbranch = 1;
            break;

case J_ADCON:
            load_adcon(r1,(Symstr *)m,(int32)r2);
            if (r1 == R_STACK) fp_minus_sp += 4, peep &= ~P_PEEPPUSH;
            break;

default:
        show_fp_inst_direct(op, m, peep, r1r, r2r, mr);
        return;
    }

    if (illbits | peep) syserr(syserr_illegal_jopmode, (long)op);
}

static void show_fp_inst_direct( J_OPCODE op, int32 m, int32 peep,
                  RealRegister r1r, RealRegister r2r, RealRegister mr)
{
#ifdef TARGET_HAS_SCALED_ADDRESSING
    int32 msh = (op & J_SHIFTMASK) >> J_SHIFTPOS;
#endif
    IGNORE(peep);
#ifndef TARGET_IS_68020
    IGNORE(mr);
    IGNORE(r2r);
#endif
    op &= ~J_DEADBITS;                          /* fp ops don't use yet */
    switch(op & ~illbits)
    {
    case J_ADCOND:
    case J_ADCONF:
        {   FloatCon *fc = (FloatCon *)m;
    
	    if( (codep & 2) == 0) outinstr(OP_NOOP);
            outinstr(OP_BSR, op == J_ADCOND? 8: 4);
            if( op == J_ADCOND )
            {
	        outcodeword(fc->floatbin.db.msd, LIT_FPNUM1);
		outcodeword(fc->floatbin.db.lsd, LIT_FPNUM1);
            }
            else
            {
	        outcodeword(fc->floatbin.fb.val, LIT_FPNUM);
            }
	    setlabel(nextlabel());
            genmove(addrmode(r1r)+Mregnum(r1r),0,LONG2,EA_AnPost+Mregnum(R_SP),0);
            break;
        }

#ifdef TARGET_IS_68020
case J_MOVFK:
case J_MOVDK:       /* load single & double literals the same way        */
        {   FloatCon *fc = (FloatCon *)m;
            outfpinstr(OP_FMOVETOFP, EA_Imm, M, (op==J_MOVFK)? FPF_S: FPF_D,
                               fpregnum(r1r) );
            if( op == J_MOVDK )
            {
                outL(fc->floatbin.db.msd);
                outL(fc->floatbin.db.lsd);
            }
            else
            {
                outL(fc->floatbin.fb.val);
            }
            break;
        }
case J_PUSHD:
        outfpinstr(OP_FMOVEFROMFP, EA_AnPre+Mregnum(R_SP),
                                   FPF_D, fpregnum(r1r), 0 );
        fp_minus_sp += 8;
        break;

case J_PUSHF:
        outfpinstr(OP_FMOVEFROMFP, EA_AnPre+Mregnum(R_SP),
                                   FPF_S, fpregnum(r1r), 0 );
        fp_minus_sp += 4;
        break;

case J_CMPFR:
case J_CMPDR:
        outfpinstr(OP_FCMP, 0, R, fpregnum(mr), fpregnum(r2r));
        break;

case J_MOVFDR:
case J_MOVDFR:
        move_fpregister(r1r,mr);
        return;

case J_MOVFR:
case J_MOVDR:
        if (r1r==mr) syserr(syserr_remove_fpnoops);
        move_fpregister(r1r,mr);
        break;

case J_NEGFR:
case J_NEGDR:
        outfpinstr(OP_FNEG, 0, R, fpregnum(mr), fpregnum(r1r) );
        break;

case J_FIXFR:
case J_FIXDR:    /* C 'fix' is truncate towards zero */
        {   RealRegister tr = addrreg(r1r)? R_DS: r1r;
            outfpinstr(OP_FMOVEFROMFP, EA_Dn+Mregnum(tr),
                                   FPF_L, fpregnum(mr), 0);
            if( r1r != tr ) move_register(r1r, tr);
            illbits ^= J_SIGNED;   /* only signed version acceptable */
            break;
        }
case J_FLTFR:
case J_FLTDR:
        if( addrreg(mr) )
        {   move_register(R_DS, mr);
            mr = R_DS;
        }
        outfpinstr(OP_FMOVETOFP, EA_Dn+Mregnum(mr), M,
                               FPF_L, fpregnum(r1r));
        illbits ^= J_SIGNED;   /* only signed version acceptable */
        break;

#if (NARGREGS != 0)
case J_MOVIFR:
/* Load FP register from an integer one                                  */
        if( addrreg(mr) )
        {   move_register(R_DS, mr);
            mr = R_DS;
        }
        outfpinstr(OP_FMOVETOFP, EA_Dn+Mregnum(mr), M,
                                 FPF_S, fpregnum(r1r));
        break;

case J_MOVIDR:
/* Load FP register from 2 integer registers                             */
        if( mr > r2r )
            stack(regbit(mr)|regbit(r2r));
        else
        {   stack(regbit(mr));
            stack(regbit(r2r));
        }
        outfpinstr(OP_FMOVETOFP, EA_AnPost+Mregnum(R_SP), M,
                                 FPF_D, fpregnum(r1r));
        break;
#endif

case J_LDRFR:
        fp_mem_indexed(r1r, r2r, mr, 0, 1, FPF_S, msh);
        illbits &= ~J_SHIFTMASK;
        break;
case J_LDRDR:
        fp_mem_indexed(r1r, r2r, mr, 0, 1, FPF_D, msh);
        illbits &= ~J_SHIFTMASK;
        break;
case J_STRFR:
        fp_mem_indexed(r1r, r2r, mr, 0, 0, FPF_S, msh);
        illbits &= ~J_SHIFTMASK;
        break;
case J_STRDR:
        fp_mem_indexed(r1r, r2r, mr, 0, 0, FPF_D, msh);
        illbits &= ~J_SHIFTMASK;
        break;
case J_LDRFK:
        fp_mem_indexed(r1r, r2r, -1, m, 1, FPF_S, 0);
        break;
case J_LDRDK:
        fp_mem_indexed(r1r, r2r, -1, m, 1, FPF_D, 0);
        break;
case J_STRFK:
        fp_mem_indexed(r1r, r2r, -1, m, 0, FPF_S, 0);
        break;
case J_STRDK:
        fp_mem_indexed(r1r, r2r, -1, m, 0, FPF_D, 0);
        break;

case J_ADDFR:
case J_ADDDR:
        fpregisterop(OP_FADD, r1r, r2r, mr );
        break;

case J_SUBFR:
case J_SUBDR:
        fpregisterop(OP_FSUB, r1r, r2r, mr );
        break;

case J_MULFR:
        fpregisterop(OP_FSGLMUL, r1r, r2r, mr );
        break;
case J_MULDR:
        fpregisterop(OP_FMUL, r1r, r2r, mr );
        break;

case J_DIVFR:
        fpregisterop(OP_FSGLDIV, r1r, r2r, mr );
        break;
case J_DIVDR:
        fpregisterop(OP_FDIV, r1r, r2r, mr );
        break;

case J_ADDFK:
        fpconstantop(OP_FADD, r1r, r2r, (FloatCon *)m, FPF_S, 1 );
        break;
case J_SUBFK:
        fpconstantop(OP_FSUB, r1r, r2r, (FloatCon *)m, FPF_S, 1 );
        break;
case J_MULFK:
        fpconstantop(OP_FSGLMUL, r1r, r2r, (FloatCon *)m, FPF_S, 1 );
        break;
case J_DIVFK:
        fpconstantop(OP_FSGLDIV, r1r, r2r, (FloatCon *)m, FPF_S, 1 );
        break;
case J_CMPFK:
        fpconstantop(OP_FCMP, GAP, r2r, (FloatCon *)m, FPF_S, 0 );
        break;
case J_ADDDK:
        fpconstantop(OP_FADD, r1r, r2r, (FloatCon *)m, FPF_D, 1 );
        break;
case J_SUBDK:
        fpconstantop(OP_FSUB, r1r, r2r, (FloatCon *)m, FPF_D, 1 );
        break;
case J_MULDK:
        fpconstantop(OP_FMUL, r1r, r2r, (FloatCon *)m, FPF_D, 1 );
        break;
case J_DIVDK:
        fpconstantop(OP_FDIV, r1r, r2r, (FloatCon *)m, FPF_D, 1 );
        break;
case J_CMPDK:
        fpconstantop(OP_FCMP, GAP, r2r, (FloatCon *)m, FPF_D, 0 );
        break;
#endif
   default:
        syserr(syserr_unimp_jopmode, (long)op);
   }
}

#ifdef TARGET_IS_68020
static int32 fpregnum(RealRegister r)
{
   if( R_F0 <= r && r <= R_F7 ) return r - R_F0;
   syserr(syserr_fp_reg);
   return(0);  /* To keep the compiler happy */
}

static void outfpinstr( instrtype op, int32 ea, ...)
{   va_list alst;

    va_start(alst, ea);

    outRR(0xF200+ea);
    voutinstr(op, alst);
    va_end(alst);
}
#endif

bool fpliteral(FloatCon *val, J_OPCODE op)
{
    IGNORE(val);
    IGNORE(op);
    return 1;
}

/* Exported routines...                                               */

/* The peepholer: */
void show_instruction(J_OPCODE op, VRegInt vr1, VRegInt vr2, VRegInt vm)
{
    /* it may be better to arrange for two switches - one on 'op' and    */
    /* one on pending.op                                                 */
    int32 peep = 0;                   /* essentially extra bits for 'op' */
    RealRegister r1 = vr1.r, r2 = vr2.r;
    int32 m = vm.i;
    J_OPCODE op2 = op & ~J_DEADBITS;
    J_OPCODE pendop2;
    if (debugging(DEBUG_LOCALCG))
        cc_msg("jop 0x%lx %ld %ld %ld\n",
               (long)op, (long)r1, (long)r2, (long)m);
    switch (pendop2 = pending.op & ~(J_SIGNED|J_UNSIGNED|
#ifdef TARGET_HAS_SCALED_ADDRESSING
                                    J_SHIFTMASK|
#endif
                                    J_DEADBITS))
    {
case J_NOOP:
        goto skip;
case J_SETSP:
        if (op == J_SETSP && pending.m == (int32)r2)
        {   /* squash adjacent SETSP's */
            pending.m = m;
            return;
        }
        /* On the 68K SETSP before RETURN can be elided in a non-leaf   */
        /* proc -- this is because of the use of LINK and FP.           */
        if (op == J_B && (LabelNumber *)m == RETLAB)
            pending.peep |= P_PEEPRET;
        break;
#ifndef TARGET_IS_HELIOS        /* Not all mods done yet! */
case J_ADCON:
        if ((op & J_DEAD_R2) && pending.r1 == r2 &&
/* The next line probably shows a difficiency in regalloc.c -- consider */
/* ADCON r,x; STRK r,r,4.  The R2 field of STRK gets marked DEAD, but   */
/* r is still re-used in the same instruction (regalloc thinks R1       */
/* gets read before R2!).                                               */
            !(reads_r1(op) && r1==r2) &&
              (op2 == J_LDRK || op2 == J_STRK ||
               (op2 & ~(J_UNSIGNED|J_SIGNED)) == J_LDRWK ||
               (op2 & ~(J_UNSIGNED|J_SIGNED)) == J_STRWK ||
               (op2 & ~(J_UNSIGNED|J_SIGNED)) == J_LDRBK ||
               (op2 & ~(J_UNSIGNED|J_SIGNED)) == J_STRBK))
        {   pending.peep |= P_ADCON,
            pending.op = op, pending.r1 = r1,
            pending.r2 += m;     /* (r2,m) = (off,sym) as in ADCON      */
            return;
        }
#endif
        /* drop through to look at ADCON then PUSH                      */
case J_MOVK: case J_MOVR:
case J_LDRK: case J_LDRR:
        /* squeeze moves then push to moves (more cases to do!).        */
        if (op == (J_PUSHR|J_DEAD_R1) && r1 == pending.r1)
        {   pending.peep |= P_PEEPPUSH, pending.r1 = R_STACK;
            return;
        }
        if (pendop2 == J_MOVR || pendop2 == J_LDRK || pendop2 == J_LDRR)
            goto trycmp;
        break;
case J_ADDK: case J_ADDR:
case J_SUBK: case J_SUBR:
             case J_RSBR:
case J_ANDK: case J_ANDR:
case J_ORRK: case J_ORRR:
case J_EORK: case J_EORR:
case J_SHLK: case J_SHLR:
case J_SHRK: case J_SHRR:
        if (!uses_r3(pending.op) && pending.m == 0) break;
trycmp:
        if ((op2 == J_CMPK+Q_EQ || op2 ==J_CMPK+Q_NE ||
             op2 == J_CMPK+Q_UEQ || op2 ==J_CMPK+Q_UNE) &&
            m == 0 && r2 == pending.r1 && datareg(r2))
        {   pending.peep |= P_PEEPCMPZ;
            if (pendop2 == J_ANDK && op & J_DEAD_R2) pending.peep |= P_PEEPBTST;
            return;
        }
        break;
default:
        break;
    }
    show_inst_direct(pending.op, pending.r1, pending.r2, pending.m,
                     pending.peep);
skip:
    pending.op = op, pending.r1 = r1, pending.r2 = r2,
                     pending.m = m, pending.peep = peep;
    switch (op)
    {   case J_ENTER: case J_ENDPROC: case J_STACK: case J_LABEL:
        case J_INFOLINE: case J_INFOBODY:
            /* flush these cases immediately for local_address()  */
            show_inst_direct(pending.op, pending.r1, pending.r2,
                             pending.m, pending.peep);
            pending.op = J_NOOP; pending.peep = 0;
            break;
    }
}

void mcdep_init()
{
#ifndef ARGS_ON_STACK
#ifndef TARGET_IS_HELIOS
/* Each module that I compile will start with                            */
/*      JMP   __main                                                     */
/* so that any complete program can be entered at its first word.        */
# ifdef TRIPOS_OBJECTS
    outRR(0); outRR(0);  /* A long word which will overwritten later */
# else
    codexrefs = (CodeXref *)global_list4(SU_Other, codexrefs,
                                         X_absreloc | (codebase+codep+2),
                                         libentrypoint, 0);
    obj_symref(libentrypoint,xr_code,0);
    outinstr(OP_JMP,EA_AbsL);
    eafield(EA_AbsL,0);
    cnop();
# endif
#endif
#endif
    switchbranch = 0;
    pending.op = J_NOOP, pending.peep = 0;
    stacklimitname = sym_insert("_stacktop",s_identifier);
#ifdef ARGS_ON_STACK
    avoidallocating(R_IP);     /* bit of a hack for now */
#endif
}

void localcg_reinit(void)
{
}

void localcg_tidy(void)
{
}

bool alterscc(Icode *ic) { return 1; }

/* End of section m68k/gen.c */
