/*
 * csescan.c: CSE available expression analysis
 * Copyright (C) Acorn Computers Ltd., 1988.
 * Copyright (C) Advanced Risc Machines Ltd., 1991
 */

/*
 * RCS $Revision: 1.3 $ Codemist 123
 * Checkin $Date: 1993/07/27 14:25:41 $
 * Revising $Author: nickc $
 */

#ifdef __STDC__
#  include <string.h>
#  include <time.h>
#  include <stddef.h>
#else
#  include <strings.h>
#  include "time.h"
#  include "stddef.h"
#endif

#include "globals.h"
#include "cse.h"
#include "cseguts.h"
#include "aeops.h"
#include "jopcode.h"
#include "store.h"
#include "cgdefs.h"
#include "builtin.h"   /* for sim */
#include "mcdep.h"     /* immed_cmp */
#include "flowgraf.h"  /* is_exit_label */
#include "regalloc.h"  /* vregister */
#include "errors.h"

#define HASHSIZE 1024
#define hash(op,a,b) ( ((op) + (((int32)(a)) * 7)) & (HASHSIZE-1) )

#define LOCHASHSIZE 512
#define lochash(type,a,b) ( ((type) + (int32)(a)) & (LOCHASHSIZE-1) )

Exprn **exprnindex[EXPRNINDEXSIZE];

#define LOCSEGSIZE 512
#define LOCINDEXSIZE 64
#define LOCSEGBITS 9

static Location **locindex[LOCINDEXSIZE];
#define loc_(id) (locindex[(id)>>LOCSEGBITS])[(id)&(LOCSEGSIZE-1)]

#define addtoreglist(r, l) l = (RegList *) syn_cons2(l, r)

static Location **locations; /* list of Locations used */
static Exprn **cse_tab;      /* hash table of Exprns */
static Exprn *heapptr;

#define CSEIDSEGSIZE 128     /* This must be a multiple of the VRegSet chunk
                                size (or its intended space-saving fails)
                              */
int32 cse_debugcount;

static int32 cseidsegment;
static int32 csealiasid, csealiaslimit;
static int32 csenonaliasid, csenonaliaslimit;

static VRegSetP availableexprns,
                liveexprns,
                wantedexprns,
                killedlocations;

static VRegSetP loadrs;

#define CALLLOC (-1) /* a fake locid to say 'has calls' */
#define isrealloc(p) ((p) > CALLLOC)

#ifdef TARGET_ALLOWS_COMPARE_CSES
#  define CCLOC   (-2) /* fake locid meaning 'alters condition codes */
static VRegSetP compares;

#  ifndef CSE_COMPARE_MASK
#    define CSE_COMPARE_MASK Q_MASK  /* just one op for all compares */
#  endif

#endif

#define J_NEK J_INIT     /* really, we should have a different jopcode for this */
#define J_HEAPPTR J_NOOP /* and for this */

#define IsLive(x) (vregset_member(exid_(x), liveexprns))
#define LocKills(x, p) (vregset_member(x, (p)->users) || vregset_member(x, (p)->aliasusers))

static int32 locationid;

typedef struct FloatConList {
    struct FloatConList *cdr;
    FloatCon *f;
} FloatConList;

static FloatConList *floatconlist;

struct ExSet {
    ExSet *cdr;
    Exprn *exprn;
};

struct LocList {
    LocList *cdr;
    Location *loc;
};

static LocList *LocList_New(LocList *next, Location *loc) {
  return (LocList *)CSEList2(next, loc);
}

static LocList *LocList_DiscardOne(LocList *locs) {
  return (LocList *)discard2((VoidStar)locs);
}

typedef struct RegValue {
    struct RegValue *cdr;
    VRegnum reg;
    ExSet *value;
} RegValue;

#define rv_reg_(r) ((r)->reg)
#define rv_val_(r) ((r)->value)

static RegValue *knownregs;

static RegValue *RegValue_DiscardOne(RegValue *rv) {
  return (RegValue *)discard3((VoidStar)rv);
}

static RegValue *RegValue_New(RegValue *next, VRegnum reg, ExSet *val) {
  return (RegValue *)CSEList3(next, reg, val);
}

typedef struct LocSet LocSet;
struct LocSet {
    LocSet *cdr;
    Location *loc;
    ExSet *oldval;
};

#define LocSet_Member(x, s) member((VRegnum)x, (RegList *)s)

static LocSet *LocSet_New(LocSet *next, Location *loc) {
  return (LocSet *)CSEList3(next, loc, NULL);
}

static LocSet *LocSet_DiscardOne(LocSet *locs) {
  return (LocSet *)discard3((VoidStar)locs);
}

static LocSet *LocSet_Copy(LocSet *locs) {
  LocSet *res = NULL, **resp = &res;
  for (; locs != NULL; locs = cdr_(locs)) {
    LocSet *p = LocSet_New(NULL, locs->loc);
    *resp = p; resp = &cdr_(p);
  }
  return res;
}

static void LocSet_Discard(LocSet *locs) {
  for (; locs != NULL; locs = LocSet_DiscardOne(locs)) /* nothing */;
}

static bool LocSet_Subset(LocSet *sub, LocSet *super) {
  for (; sub != NULL; sub = cdr_(sub))
    if (!LocSet_Member(sub->loc, super))
      return NO;
  return YES;
}

typedef struct StoreAccessList {
    struct StoreAccessList *cdr;
    LocSet *locs;
    Icode *ic;
} StoreAccessList;

static StoreAccessList *storeaccesses;

static StoreAccessList *StoreAccessList_New(StoreAccessList *next, LocSet *locs, Icode *ic) {
  return (StoreAccessList *)CSEList3(next, locs, ic);
}

static StoreAccessList *StoreAccessList_DiscardOne(StoreAccessList *sa) {
  return (StoreAccessList *)discard3((VoidStar)sa);
}

static StoreAccessList *StoreAccessList_Copy(StoreAccessList *sa) {
  StoreAccessList *res = NULL, **resp = &res;
  for (; sa != NULL; sa = cdr_(sa)) {
    StoreAccessList *p = StoreAccessList_New(NULL, LocSet_Copy(sa->locs), sa->ic);
    *resp = p; resp = &cdr_(p);
  }
  return res;
}

static void StoreAccessList_Discard(StoreAccessList *sa) {
  for (; sa != NULL; sa = StoreAccessList_DiscardOne(sa))
    LocSet_Discard(sa->locs);
}

#ifdef ENABLE_CSE

/*
 * Debugging stuff.
 */

void cse_print_loc(Location *x)
{
    switch (loctype_(x)) {
default:
        cc_msg("<odd-loc>"); break;
    case LOC_VAR:
    case LOC_PVAR:
        cc_msg("'%s'", symname_(bindsym_(locbind_(x))));
        break;
    case LOC_(MEM_B):
    case LOC_(MEM_W):
    case LOC_(MEM_I):
    case LOC_(MEM_LL):
    case LOC_(MEM_F):
    case LOC_(MEM_D):
        cc_msg("[(%ld), #%ld]", (long)exid_(locbase_(x)), (long)locoff_(x));
        break;
    }
}

void cse_print_node(Exprn *p)
{
    if (p == NULL) { cc_msg("null\n"); return; }
    cc_msg("node %ld: ", (long)exid_(p));
    jopprint_opname(exop_(p));
    if (exop_(p) == CSE_LOADR) {
        cc_msg(" r%ld\n", (long)e1_(p)); return;
    }
    switch (extype_(p)) {
    case E_UNARYK:
        cc_msg("#%ld", (long)e1k_(p));
        break;
    case E_UNARY:
        cc_msg("(%ld)", (long)exid_(e1_(p)));
        break;
    case E_BINARYK:
        cc_msg("(%ld), #%ld", (long)exid_(e1_(p)), (long)e2k_(p));
        break;
    case E_BINARY:
        cc_msg("(%ld), (%ld)", (long)exid_(e1_(p)), (long)exid_(e2_(p)));
        break;
    case E_LOAD:
        cse_print_loc(exloc_(p));
        break;
    case E_MISC:
        cc_msg("**");
        break;
    case E_CALL:
        {   int32 i;
            cc_msg("%s(", symname_(bindsym_(exfn_(p))));
            for (i = 0 ; i < exnargs_(p) ; i++) {
                if (i != 0) cc_msg(", ");
                cc_msg("(%ld)", (long)exid_(exarg_(p, i)));
            }
            cc_msg(")");
            if (exnres_(p) > 1) cc_msg("=>%ld", exnres_(p));
        }
        break;
    }
    cc_msg("\n");
}

void cse_printexits(int32 flags, LabelNumber *exit, LabelNumber *exit1)
{
    VRegInt gap, m;
    gap.r = GAP;
    if (flags & BLK2EXIT) {
        m.l = exit1;
        print_jopcode(J_B + (flags & Q_MASK), gap, gap, m);
    }
    if (!(flags & BLK0EXIT)) {
        m.l = exit;
        print_jopcode(J_B, gap, gap, m);
    }
}

static void ExSet_Print(ExSet *set, const char *s) {
  char c = '{';
  for (; set != NULL; set = cdr_(set)) {
    cc_msg("%c%ld", c, (long)exid_(set->exprn));
    c = ' ';
  }
  if (c != '{') cc_msg("}%s", s);
}

static void StoreAccessList_Print(StoreAccessList *p, const char *s) {
  char *s1 = "store accesses: ";
  for (; p != NULL; p = cdr_(p)) {
    LocSet *locs = p->locs;
    char *s2 = " {";
    cc_msg("%s%s", s1, loads_r1(p->ic->op) ? "LD": "ST");
    for (; locs != NULL; locs = cdr_(locs)) {
      cc_msg(s2); cse_print_loc(locs->loc);
      s2 = ", ";
    }
    cc_msg("}");
    s1 = ", ";
  }
  cc_msg("%s", s);
}

static void RegValue_Print(RegValue *p, const char *s) {
  for (; p != NULL; p = cdr_(p)) {
    cc_msg("r%ld = ", p->reg);
    ExSet_Print(p->value, cdr_(p) == NULL ? s : " ");
  }
}

#else

void cse_print_loc(Location *x)
{
    IGNORE(x);
}

void cse_print_node(Exprn *p)
{
    IGNORE(p);
}

void cse_printexits(int32 flags, LabelNumber *exit, LabelNumber *exit1)
{
    IGNORE(flags); IGNORE(exit); IGNORE(exit1);
}

static void ExSet_Print(ExSet *set, char *s) {
  IGNORE(set);
}

static void StoreAccessList_Print(StoreAccessList *p, const char *s) {
    IGNORE(p); IGNORE(s);
}

#endif /* ENABLE_CSE */

#ifdef TARGET_HAS_SCALED_ADDRESSING
#  define opisshifted(op) (((op) & J_SHIFTMASK) != 0)
#  define unshiftedop(op) ((op) & ~J_SHIFTMASK)
#  define opwithshift(op, op1) ((op) | ((op1) & J_SHIFTMASK))
#  define NEGINDEX J_NEGINDEX
#else
#  define opisshifted(op) NO
#  define unshiftedop(op) (op)
#  define opwithshift(op, op1) (op)
#  define NEGINDEX 0L
#endif

#define CantBeSubExprn(x) ((x) == NULL || (x) == heapptr || exop_(x) == J_NEK)

#define ExprnToSet(e) ExSet_Insert(e, NULL)
#define ExSet_Copy(e) ExSet_Append(e, NULL)
#define ExSet_DiscardOne(e) ((ExSet *)discard2(e))

static bool ExSet_Member(Exprn *e, ExSet *set) {
  ExSet *p = set;
  for (; p != NULL; p = cdr_(p))
    if (p->exprn == e) return YES;
  return NO;
}

static ExSet *ExSet_Insert(Exprn *e, ExSet *set) {
  return (ExSet_Member(e, set)) ? set : (ExSet *) CSEList2(set, e);
}

static ExSet *ExSet_NDelete(Exprn *e, ExSet *set) {
  ExSet *p, **prev = &set;
  for (; (p = *prev) != NULL; prev = &cdr_(p))
    if (p->exprn == e) {
      *prev = ExSet_DiscardOne(p);
      break;
    }
  return set;
}

static void ExSet_Discard(ExSet *set) {
  for (; set != NULL; )
    set = ExSet_DiscardOne(set);
}

static ExSet *LiveExSet(ExSet *set) {
  ExSet *p, **prev = &set;
  while ((p = *prev) != NULL)
    if (!IsLive(p->exprn))
      *prev = ExSet_DiscardOne(p);
    else
      prev = &cdr_(p);

  return set;
}

static ExSet *ExSet_Intersection(ExSet *s, ExSet *s1) {
  ExSet *p, **prev = &s;
  while ((p = *prev) != NULL)
    if (ExSet_Member(p->exprn, s1))
      prev = &cdr_(p);
    else
      *prev = ExSet_DiscardOne(p);
  return s;
}

static bool AlreadyNarrowed(Exprn *e, J_OPCODE op);

static ExSet *NarrowedSet(ExSet *set, J_OPCODE op) {
  ExSet *p, *res = NULL;
  for (p = set; p != NULL; p = cdr_(p))
    if (AlreadyNarrowed(p->exprn, op))
      res = ExSet_Insert(p->exprn, res);

  return res;
}

typedef bool ExSetMapFn(Exprn *, void *);

static bool ExSet_Map(ExSet *p, ExSetMapFn *f, void *a) {
  for (; p != NULL; p = cdr_(p))
    if (!f(p->exprn, a)) return NO;
    return YES;
}

static ExSet *ExSet_Append(ExSet *a, ExSet *b) {
  ExSet *res = b;
  for (; a != NULL; a = cdr_(a))
    res = ExSet_Insert(a->exprn, res);
  return res;
}

static bool ExSetsOverlap(ExSet *a, ExSet *b) {
  ExSet *p;
  for (p = a; p != NULL; p = cdr_(p))
    if (!CantBeSubExprn(p->exprn) && ExSet_Member(p->exprn, b))
      return YES;
  return NO;
}

static ExSet *OpInSet(ExSet *set, J_OPCODE op, int32 ignorebits) {
  for (; set != 0; set = cdr_(set))
    if ((exop_(set->exprn) & ~ignorebits) == op)
      return set;
  return NULL;
}

#if defined TARGET_HAS_ROTATE || !defined TARGET_IS_HELIOS
static ExSet *OpInSetWithE1(ExSet *set, J_OPCODE op, int32 ignorebits, Exprn *e1) {
  for (; set != 0; set = cdr_(set))
    if ((exop_(set->exprn) & ~ignorebits) == op &&
        e1_(set->exprn) == e1)
      return set;
  return NULL;
}
#endif

static int32 cse_optype(int32 op)
{
/* The order of these tests is significant, as remarked below .. */
  if (isproccall_(op)) return E_CALL;
  if (j_is_adcon(op)) return E_UNARYK;
  if (uses_mem(op)) return E_LOAD;
  if (j_is_diadr(op)) return E_BINARY;
  if (j_is_diadk(op)) return E_BINARYK;
  if (reads_r3(op)) return E_UNARY;  /* must be after j_is_diadr(op) */
  if (loads_r1(op)) return E_UNARYK; /* must be at end */
  return E_MISC;
}

static int32 locsize(LocType type)
{
  switch (type) {
  case LOC_(MEM_B): return 1;
  case LOC_(MEM_W): return 2;
  case LOC_(MEM_I): return 4;
  case LOC_(MEM_LL): return 8;
/* we should really get these values from sizeof_short &c?              */
  case LOC_(MEM_F): return 4;
  case LOC_(MEM_D): return 8;
/* the next line is dubious -- what about LOC_VAR/LOC_PVAR?          */
/* Harry Meekings and AM just (13/4/92) agreed to put MEM_xxx parts in  */
/* LOC_VAR and LOC_PVAR fields.  Soon to be done!                       */
  default:       return 4;
  }
}

static LocType loctype(J_OPCODE op)
{ int32 mem = j_memsize(op);
  switch (mem) {
  case MEM_D:
  case MEM_F:
  case MEM_I:
  case MEM_B:
  case MEM_W:
  case MEM_LL: return LOC_(mem);
  default:    syserr(syserr_loctype); return LOC_(MEM_I);
  }
}

static bool overlap(LocType typea, int32 a,
                    LocType typeb, int32 b)
{
    return ((a >= b && a < (b+locsize(typeb))) ||
            (b >= a && b < (a+locsize(typea))));
}

Exprn *adconbase(Exprn *ex, bool allowvaroffsets)
{
    J_OPCODE op = unshiftedop(exop_(ex));
    if (ex == NULL) syserr(syserr_adconbase);
    if ( ex == heapptr ||
         op == J_ADCON || op == J_ADCONV)
        return ex;
    else if (op == J_LDRK) {
        Location *loc = exloc_(ex);
        if (loctype_(loc) == LOC_VAR && (bindstg_(locbind_(loc)) & b_noalias))
            return ex;
    } else if (op == J_ADDR) {
        if (!allowvaroffsets)
            return NULL;
        else {
            Exprn *r = adconbase(e1_(ex), YES);
            if (r != NULL) return r;
            if (!opisshifted(exop_(ex)))
              return adconbase(e2_(ex), YES);
        }
    } else if ( op == J_SUBK || op == J_ADDK ||
                (op == J_SUBR && allowvaroffsets) )
        return adconbase(e1_(ex), allowvaroffsets);
    return NULL;
}

static bool possiblealias(Location *loc,
                          LocType type, Exprn *base, int32 off)
{
    /* Determine whether loc may be an alias of {type,base,off}
       (type known to be LOC_(MEM_x)) */
    /* NB required to return NO if loc is the same as {type,base,off} */

    LocType t = loctype_(loc);
    Exprn *b = locbase_(loc);
    if (t == LOC_PVAR) {
        Exprn *b1 = adconbase(base, YES);
        return ( b1 == NULL ||
                 (exop_(b1) == J_ADCONV && e1b_(b1) == locbind_(loc)));
    }
    if (t & LOC_anyVAR) return NO;
    if (b != base) {
        Exprn *base1 = adconbase(b, YES),
              *base2 = adconbase(base, YES);
        if (base1 == NULL || base2 == NULL)
            return YES;
        else if (base1 == base2)
            return YES;
        else if (exop_(base1) == J_ADCON && exop_(base2) == J_ADCON &&
                 (bindstg_(e1b_(base2)) & bitofstg_(s_static)) &&
                 (bindstg_(e1b_(base1)) & bitofstg_(s_static))) {
            if (e1b_(base1) == datasegment)
                return overlap(type, off + bindaddr_(e1b_(base2)), t, locoff_(loc));
            else if (e1b_(base2) == datasegment)
                return overlap(type, off, t, locoff_(loc)+bindaddr_(e1b_(base1)));
        }
        return NO;
    }
    return (!(t == type && off == locoff_(loc)) &&
            overlap(type, off, t, locoff_(loc)));
}

static void updateusers(int32 id, Exprn *p)
{ /*
   * Update the 'users' field for all locations which are leaves of the
   * expression p to include the expression node id.
   * On the assumption that expressions are small, no attempt is made to be
   * clever about getting at the leaves.
   */
    if (p == NULL) return;
    switch (extype_(p)) {
    case E_BINARY:
        updateusers(id, e2_(p));
    case E_BINARYK:
    case E_UNARY:
        updateusers(id, e1_(p));
        break;
    case E_LOAD:
        {   Location *loc = exloc_(p);
            LocType type = loctype_(loc);
            if (!(type & LOC_anyVAR)) {
                Exprn *base = locbase_(loc);
                int32 off = locoff_(loc);
                int32 i;
                for (i = 0 ; i < LOCINDEXSIZE ; i++) {
                    Location **index = locindex[i];
                    int32 j;
                    if (index == 0) break;
                    for (j = 0 ; j < LOCSEGSIZE ; j++) {
                        Location *q = index[j];
                        if (q == 0) break;
                        if (possiblealias(q, type, base, off))
                            cseset_insert(id, q->aliasusers, NULL);
                    }
                }
                updateusers(id, base);
            }
            cseset_insert(id, loc->users, NULL);
            break;
        }
    case E_CALL:
        {   int32 i;
            for (i = 0 ; i < exnargs_(p) ; i++)
                updateusers(id, exarg_(p, i));
        }
        break;
    default:
        break;
    }
}

static bool maybealias(Exprn *p)
{ /*
   * Returns true if p or some part of it may have aliases.
   * (Used to give things which do and don't distinct sets of ids,
   *  to reduce the space usage of killed sets).
   */
    if (p == NULL) return NO;
    switch (extype_(p)) {
    case E_BINARY:
        if (maybealias(e2_(p))) return YES;
    case E_BINARYK:
    case E_UNARY:
        return maybealias(e1_(p));
    case E_LOAD:
        {   Location *loc = exloc_(p);
            return ispublic(loc);
        }
    case E_CALL:
        {   int32 i;
            for (i = 0 ; i < exnargs_(p) ; i++)
                if (maybealias(exarg_(p, i))) return YES;
            return NO;
        }
    default:
        return NO;
    }
}

static FloatCon *canonicalfpconst(FloatCon *old)
{
    FloatConList *p;
    for ( p = floatconlist ; p != NULL ; p = cdr_(p) )
        if (p->f->floatlen == old->floatlen &&
            p->f->floatbin.irep[0] == old->floatbin.irep[0] &&
            p->f->floatbin.irep[1] == old->floatbin.irep[1])
            return p->f;
    floatconlist = (FloatConList *) syn_cons2(floatconlist, old);
    return old;
}

typedef struct {
    int32 expid;
    bool killed; } ULWRec;


static void ulw_cb(int32 id, VoidStar arg)
{
    ULWRec *ulw = (ULWRec *) arg;
    if (isrealloc(id) && !ulw->killed) {
        ulw->killed = LocKills(ulw->expid, loc_(id));
    }
}

bool killedinblock(int32 expid)
{
    /* (For use only from within cse_scanblock).
       Determine whether the Exprn with id expid could reach the current
       icode position from the block head (whether it has been killed yet
       in the block).
     */
    ULWRec ulw; ulw.expid = expid; ulw.killed = NO;
    cseset_map(killedlocations, ulw_cb, (VoidStar) &ulw);
    return ulw.killed;
}

#define OldLive 1
#define OldAvail 2

static int updateliveandwanted(Exprn *exp, int flags)
{
  /* Update the set of live expressions to include expid (the return value
   * indicates whether it was already in the set).
   * If the expression has not been killed, add it also to the set of wanted
   * expressions
   */
    int oldlive;
    bool old;
    int32 expid = exid_(exp);
    cseset_insert(expid, liveexprns, &old);
    oldlive = old ? OldLive : 0;
    if (!(flags & U_NOTDEF2)) {
        cseset_insert(expid, availableexprns, &old);
        if (old) oldlive |= OldAvail;
        if (!(flags & U_NOTREF)) {
            if (cseset_member(CALLLOC, killedlocations)) {
                int32 i;
                for (i = 0 ; i < LOCINDEXSIZE ; i++) {
                    Location **index = locindex[i];
                    int32 j;
                    if (index == 0) break;
                    for (j = 0 ; j < LOCSEGSIZE ; j++) {
                        Location *loc = index[j];
                        if (loc == 0) break;
                        if (ispublic(loc) && LocKills(expid, loc))
                            return oldlive;
                    }
                }
            }
#ifdef TARGET_ALLOWS_COMPARE_CSES
            if (cseset_member(CCLOC, killedlocations) &&
                is_compare(exop_(exp)))
                return oldlive;
#endif
            if (!killedinblock(expid))
                cseset_insert(expid, wantedexprns, NULL);
        }
    }
    return oldlive;
}

static BlockHead *cse_currentblock;
static Icode *currenticode;

ExprnUse *ExprnUse_New(ExprnUse *old, int flags, int valno)
{
    return (ExprnUse *)CSEList3(old, cse_currentblock,
               vfi_(valno, flags, currenticode - blkcode_(cse_currentblock)));
}

static bool samearglist(int32 n, Exprn *a[], Exprn *b[])
{
    int32 i;
    for (i = 0 ; i < n ; i++)
        if (a[i] == NULL || a[i] != b[i]) return NO;
    return YES;
}

static bool SubExprn(Exprn *sub, Exprn *p)
{   if (p == NULL) return NO;
    if (p == sub) return YES;
    switch (extype_(p)) {
    case E_BINARY:
        if (SubExprn(sub, e2_(p))) return YES;
    case E_BINARYK:
    case E_UNARY:
        return SubExprn(sub, e1_(p));
    case E_LOAD:
        {   Location *loc = exloc_(p);
            if (loctype_(loc) & LOC_anyVAR) return NO;
            return SubExprn(sub, locbase_(loc));
        }
    case E_CALL:
        {   int32 i;
            for (i = 0 ; i < exnargs_(p) ; i++)
                if (SubExprn(sub, exarg_(p, i))) return YES;
        }
    }
    return NO;
}

static void useoldexprn(Exprn *p, int flags) {
  /* if the expression is already live in this block, then local cse
   * will kill this occurrence, so I don't want to remember its position.
   */
  if (!(flags & U_NOTDEF)) {
    int oldlive = updateliveandwanted(p, flags);
    if (!(oldlive & OldLive) || (flags & U_STORE)) {
    /* If it has become alive, having previously been alive, there may
     * still be some locations whose value I think it is.  (Up to now,
     * I would have known they weren't because it wasn't alive).  Their
     * values must be killed here.
     */
      LocList *q = exlocs_(p);
      for (; q != NULL; q = LocList_DiscardOne(q)) {
        Location *loc = q->loc;
        locvalue_(loc) = ExSet_NDelete(p, locvalue_(loc));
      }
      exlocs_(p) = NULL;
      { RegValue *r, **prevp = &knownregs;
        for (; (r = *prevp) != NULL ; ) {
          rv_val_(r) = ExSet_NDelete(p, rv_val_(r));
          if (rv_val_(r) == NULL)
            *prevp = RegValue_DiscardOne(r);
          else
            prevp = &cdr_(r);
        }
      }
      if (!(oldlive & OldLive)) {
        StoreAccessList **sap = &storeaccesses;
        StoreAccessList *sa;
        while ((sa = *sap) != NULL) {
          LocSet **locsp = &sa->locs;
          LocSet *locs;
          while ((locs = *locsp) != NULL)
            if (SubExprn(p, locs->loc->load))
              *locsp = LocSet_DiscardOne(locs);
            else
              locsp = &cdr_(locs);
          if (sa->locs == NULL)
            *sap = StoreAccessList_DiscardOne(sa);
          else
            sap = &cdr_(sa);
        }
      }
    }
    if (!(oldlive & OldAvail) || (flags & U_STORE)) {
      if (!(flags & U_NOTDEF2))
        exuses_(p) = ExprnUse_New(exuses_(p), flags & ~U_STORE, 0);
      exwaslive_(p) = NO;
    }
  }
}

static Exprn *find_exprn(int32 op, Exprn *a, Exprn *b, Exprn *arg[], int flags)
{
    Exprn *p, *prev;
    Exprn **list;
    int32 type;
    if (op == J_ADDK || op == J_SUBK) {
      if (a == heapptr)
        return heapptr;
      else if (exop_(a) == J_NEK) {
        int32 k = (int32)b;
        if (op == J_SUBK) k = -k;
        op = J_NEK;
        a = (Exprn *)(e1k_(a) + k);
        b = NULL;
        flags = U_NOTDEF2+U_NOTREF;
      }
    } else if (op == J_ADDR && a == heapptr)
        return heapptr;

    list = &cse_tab[hash(op, a, b)];
    type = cse_optype(op);

    switch (type) {
    case E_BINARY:
        if (CantBeSubExprn(b)) return NULL;
    case E_BINARYK:
        if (CantBeSubExprn(a)) return NULL;
        for (prev = NULL, p = *list; p != NULL; prev = p, p = cdr_(p)) {
            if (exop_(p) == op && a == e1_(p) && b == e2_(p)) { /* eureka! */
                if (prev != NULL) {
                    cdr_(prev) = cdr_(p); cdr_(p) = *list; *list = p;
                }
                useoldexprn(p, flags);
                return p;
            }
        }
        if (flags & U_PEEK) return NULL;  /* just checking */
        p = (Exprn *) CSEAlloc(
                       (int) (offsetof(Exprn, u.binary.e2) + sizeof(Exprn *)));
        e2_(p)   = b;
        break;
    case E_UNARY:
        if (CantBeSubExprn(a)) return NULL;
    case E_LOAD:
    case E_UNARYK:
        for (prev = NULL, p = *list; p != NULL; prev = p, p = cdr_(p)) {
            if (exop_(p) == op && a == e1_(p)) {
                if (prev != NULL) {
                    cdr_(prev) = cdr_(p); cdr_(p) = *list; *list = p;
                }
                useoldexprn(p, flags);
                return p;
            }
        }
        if (flags & U_PEEK) return NULL;  /* just checking */
        p = (Exprn *) CSEAlloc(
                        (int) (offsetof(Exprn, u.unary.e1) + sizeof(Exprn *)));
        break;
    case E_CALL:
        {   int32 i, n = argres_nargs_((int32)b);
            for (i = 0 ; i < n ; i++)
                if (CantBeSubExprn(arg[i])) return NULL;
            for (prev = NULL, p = *list; p != NULL; prev = p, p = cdr_(p)) {
                if (exop_(p) == op && a == e1_(p) && b == e2_(p) &&
                    samearglist(n, &exarg_(p, 0), arg)) {
                    if (prev != NULL) {
                        cdr_(prev) = cdr_(p); cdr_(p) = *list; *list = p;
                    }
                    useoldexprn(p, flags);
                    return p;
                }
            }
            if (flags & U_PEEK) return NULL;  /* just checking */
            p = (Exprn *) CSEAlloc(
                 (int) (offsetof(Exprn, u.call.arg[0]) + n * sizeof(Exprn *)));
            for (i = 0 ; i < n ; i++)
                exarg_(p, i) = arg[i];
            e2_(p)   = b; /* @@@ this updates call.nargs hence exfntype_()! */
        }
        break;
    default:
        p = NULL;   /* to keep dataflow happy */
        syserr(syserr_find_exprn, (long)type);
    }
    cdr_(p)   = *list;
    exop_(p)  = op;
    p->nodeid = type;  /* id 0 (type is needed for maybealias) */
    exwaslive_(p) = NO;
    {   int32 id; int32 alias;
        exuses_(p) = (flags & (U_NOTDEF+U_NOTDEF2)) ? NULL : ExprnUse_New(NULL, flags, 0);
        e1_(p)   = a;
        *list = p;
        exlocs_(p) = NULL;
        if (maybealias(p)) {
            if (++csealiasid >= csealiaslimit) {
                csealiasid = cseidsegment;
                csealiaslimit = (cseidsegment += CSEIDSEGSIZE);
            }
            id = csealiasid;
            alias = EX_ALIAS;
        } else {
            if (++csenonaliasid >= csenonaliaslimit) {
                csenonaliasid = cseidsegment;
                csenonaliaslimit = (cseidsegment += CSEIDSEGSIZE);
            }
            id = csenonaliasid;
            alias = 0;
        }
        p->nodeid = mknodeid_(id, alias|type);  /* rewrite with correct id */
        updateusers(id, p);
        if (!(flags & U_NOTDEF)) updateliveandwanted(p, flags);

        {   Exprn **index = exprnindex[id>>EXPRNSEGBITS];
            if (index == NULL) {
                index = (Exprn **) CSEAlloc(EXPRNSEGSIZE * sizeof(Exprn **));
                *(cseallocrec.statsbytes) += EXPRNSEGSIZE * sizeof(Exprn **);
                exprnindex[id>>EXPRNSEGBITS] = index;
                memclr(index, EXPRNSEGSIZE * sizeof(Exprn **));
            }
            index[id & (EXPRNSEGSIZE-1)] = p;
        }
    }
    if (debugging(DEBUG_CSE)) cse_print_node(p);
    return p;
}


static Exprn *find_movk(int32 n, int flags)
{
    return find_exprn(J_MOVK, (Exprn *)n, NULL, NULL, flags);
}

/*
 * Stuff about reading/writing/corrupting Locations.
 */

static Location *find_loc(LocType type, Exprn *base, int32 k, J_OPCODE load,
                          int32 loctypeflags)
{
    int32 id1 = k;
    Location *p, *prev;
    Location **list = &locations[lochash(type, id1, (int32)base)];
    if (!(type & LOC_anyVAR) && exop_(base) == J_NEK)
        return NULL;
    for (prev = NULL, p = *list; p != NULL; prev = p, p = cdr_(p)) {
        if (loctype_(p) != type  &&
             !(loctype_(p) & LOC_anyVAR  &&  type & LOC_anyVAR))
            continue;
        if (locoff_(p) != id1) continue;
        if (type & LOC_anyVAR || locbase_(p) == base) {
            if (prev != NULL) {
                cdr_(prev) = cdr_(p); cdr_(p) = *list; *list = p;
            }
            return p;
        }
    }
    {   /* @@@ next line (SizeOfLocVar) looks wrong!                 */
        int32 size = type == LOC_VAR ? SizeOfLocVar : sizeof(Location);
        Location **index = locindex[locationid>>LOCSEGBITS];
        p = (Location *) CSEAlloc(size);
        if (index == NULL) {
            index = (Location **) CSEAlloc(LOCSEGSIZE * sizeof(Location **));
            *(cseallocrec.statsbytes) += LOCSEGSIZE * sizeof(Location **);
            locindex[locationid>>LOCSEGBITS] = index;
            memclr(index, LOCSEGSIZE * sizeof(Location **));
        }
        index[locationid & (LOCSEGSIZE-1)] = p;
    }
    cdr_(p)     = *list;
    locvalue_(p)= NULL;
    p->users    = NULL;
    p->aliasusers = NULL;
    locoff_(p)  = id1;
    if (type == LOC_VAR) {
        SET_BITMAP stg = bindstg_((Binder *)id1);
        if (stg & (b_addrof | b_globalregvar |
                   bitofstg_(s_static) | bitofstg_(s_extern))) {
            type = LOC_PVAR;
        }
    } else
        locbase_(p) = base;
    p->idandtype = mkidandtype_(locationid++, type | loctypeflags);
    if (!(type & LOC_anyVAR)) {
        int32 i;
        for (i = 0 ; i < LOCINDEXSIZE ; i++) {
            Location **index = locindex[i];
            int32 j;
            if (index == 0) break;
            for (j = 0 ; j < LOCSEGSIZE ; j++) {
                Location *q = index[j];
                if (q == 0) break;
                if (possiblealias(q, type, base, id1))
                    cseset_union(p->aliasusers, q->users);
            }
        }
    }
    p->load = find_exprn(load, (Exprn *)p, 0, NULL, U_NOTDEF+U_NOTREF);
    return (*list = p);
}

static J_OPCODE cse_J_LDRK_w[] = MEM_to_J_LDRxK_table;
#define j_to_ldrk(a) cse_J_LDRK_w[j_memsize(a)]

static bool is_narrow(Location *loc)
{
    int32 type = loctype_(loc);
    return (type == LOC_(MEM_B) || type == LOC_(MEM_W));
}

static bool is_flavoured(J_OPCODE op, Location *loc)
{
    if ((op & (J_SIGNED | J_UNSIGNED)) != 0)
        return is_narrow(loc);
    return NO;
}

static Exprn *flavoured_load(J_OPCODE op, Location *loc, int flags)
{
    return find_exprn(j_to_ldrk(op) | (op & (J_SIGNED | J_UNSIGNED)),
                      (Exprn *)loc, 0, NULL, flags);
}

static bool AlreadyNarrowed(Exprn *e, J_OPCODE op)
{
   /* The intent here is to catch values which have been explicitly narrowed.
      But while we're at it, it's no great effort to notice some expressions
      whose results are already narrow.
    */
   /* op is known to be flavoured, so its size is either MEM_B or MEM_W */
   int32 shift = 24, limit = 256;
   J_OPCODE eop = exop_(e);
   if (j_memsize(op) != MEM_B) shift = 16, limit = 0x10000;
   if (op & J_UNSIGNED)
       return (eop == J_ANDK && e2k_(e) >= 0 && e2k_(e) < limit) ||
              (eop == J_MOVK && e1k_(e) >= 0 && e1k_(e) < limit);
   else {
       limit = limit >> 1;
       return (( eop == J_ANDK && e2k_(e) >= 0 && e2k_(e) < limit) ||
               ( eop == (J_SHRK | J_SIGNED) && e2k_(e) >= shift &&
                 (exop_(e1_(e)) & ~(J_SIGNED | J_UNSIGNED)) == J_SHRK &&
                 e2k_(e1_(e)) == e2k_(e)) ||
               ( eop == J_MOVK && e1k_(e) >= -limit && e1k_(e) < limit));
   }
}

static ExSet *loc_read(Location *p, int exprnflags, J_OPCODE op)
{
  /* op is to tell us about sign/zero-extension if the location is
     a narrow one. */
  ExSet *e = LiveExSet(locvalue_(p));
  if (!IsLive(p->load)) e = NULL;
  locvalue_(p) = e;  /* Otherwise, if an exprn in locvalue_(p) subsequently
                        becomes live we are in trouble
                      */
  e = ExSet_Copy(e);
  { Exprn *load;
    if (is_flavoured(op, p)) {
      load = flavoured_load(op, p, exprnflags);
    } else {
      load = p->load;
      if ( !IsLive(load) &&
           is_narrow(p) &&
           ( (op & (J_UNSIGNED | J_SIGNED)) == 0)) {
          /* If we have a plain load of a narrow location, the value
             from a previous flavoured load will do.  (This works only
             locally, of course - a way to do it non-locally would
             supersede this code
           */
        Exprn *e1 = flavoured_load(op | J_SIGNED, p, U_NOTREF+U_NOTDEF+U_PEEK);
        if (e1 != NULL && IsLive(e1)) {
          load = e1;
        } else {
          e1 = flavoured_load(op | J_UNSIGNED, p, U_NOTREF+U_NOTDEF+U_PEEK);
          if (e1 != NULL && IsLive(e1))
            load = e1;
        }
      }
      useoldexprn(load, exprnflags);
    }

    if (is_flavoured(op, p))
    /* we must discard all but a correctly extended values for the flavour
       of load wanted.
     */
      e = NarrowedSet(e, op);
    return ExSet_Insert(load, e);
  }
}

static bool AddToLocs(Exprn *e, void *loc) {
    exlocs_(e) = LocList_New(exlocs_(e), (Location *)loc);
    return YES;
}

static void setlocvalue(Location *p, ExSet *val, VRegSetP localiases)
{
    locvalue_(p) = ExSet_Copy(val);
    ExSet_Map(val, AddToLocs, p);
    { VRegSetP es = cseset_copy(p->users);
      cseset_union(es, p->aliasusers);
      cseset_difference(availableexprns, es);
      cseset_difference(es, localiases);
      cseset_difference(liveexprns, es);
      cseset_discard(es);
    }
    cseset_insert(locid_(p), killedlocations, NULL);
    /* Now for any locations whose known values depended on p, we don't know
     * their values anymore.  It's horribly expensive to go through them here,
     * though - instead, we check for liveness when we look at the value.  In
     * addition, we need to ensure that the value doesn't become live again in
     * between - so every time an expression becomes live, we destroy the value
     * of all locations with that value.
     */
}

static bool SetExWasLive(Exprn *e, void *ignore) {
    IGNORE(ignore);
    exwaslive_(e) = YES;
    if (exop_(e) == J_RESULT2) exwaslive_(e1_(e)) = YES;
    return YES;
}

static void setreg(VRegnum reg, ExSet *val, bool newvalue)
{
    RegValue **prevp = &knownregs, *r;
#if 0
/*  NOTDEF means that ex can't become live */
    Exprn *ex = find_exprn(CSE_LOADR, (Exprn *) reg, 0, NULL, U_NOTDEF+U_NOTREF+U_PEEK);
    if (ex != NULL) cseset_delete(exid_(ex), availableexprns, NULL);
#endif
    for (; (r = *prevp) != NULL ; prevp = &cdr_(r)) {
        if (rv_reg_(r) == reg) {
            if (newvalue) {
                ExSet_Discard(rv_val_(r));
                rv_val_(r) = NULL;
            }
            if (val == NULL) {
                *prevp = RegValue_DiscardOne(r);
                return;
            }
            break;
        }
    }
    if (r == NULL ) {
        if (val == NULL) return;
        knownregs = RegValue_New(knownregs, reg, NULL);
        r = knownregs;
    }
    rv_val_(r) = ExSet_Append(val, rv_val_(r));
    ExSet_Map(val, SetExWasLive, NULL);
}

static ExSet *valueinreg(VRegnum reg)
{  /* Returns the value held in the argument register if it is known,
    * otherwise NULL.
    */
    RegValue **prevp = &knownregs, *r;
    for (; (r = *prevp) != NULL ; prevp = &cdr_(r))
        if (rv_reg_(r) == reg) {
            ExSet *e = LiveExSet(rv_val_(r));
            if (e == NULL)
                *prevp = RegValue_DiscardOne(r);
            else
                rv_val_(r) = e;
            return e;
        }
    return NULL;
}

static ExSet *readreg(VRegnum reg)
{  /* Returns the value held in the argument register if it is known,
    * otherwise an Exprn describing the register (used only for base/
    * index registers of store accesses).
    */
    ExSet *res = valueinreg(reg);
    if (res != NULL) return res;
    {   Exprn *r = find_exprn(CSE_LOADR, (Exprn *) reg, 0, NULL, U_NOTDEF+U_NOTREF);
        cseset_insert(exid_(r), loadrs, NULL);
        cseset_insert(exid_(r), liveexprns, NULL);
        res = ExprnToSet(r);
        setreg(reg, res, YES);
        return res;
    }
}

static void setmem(Location *p, ExSet *val, VRegSetP localiases)
{
    setlocvalue(p, val, localiases);
}

static void setvar(Location *p, ExSet *val, VRegSetP localiases)
{
    int32 i;
    setlocvalue(p, val, localiases);
    /* Now if p may be an alias for anything, we must discard
       the known value of that thing. */
    if (loctype_(p) == LOC_PVAR) {
        for (i = 0 ; i < LOCINDEXSIZE ; i++) {
            Location **index = locindex[i];
            int32 j;
            if (index == 0) break;
            for (j = 0 ; j < LOCSEGSIZE ; j++) {
                Location *q = index[j];
                if (q == 0) break;
                if (!(loctype_(q) & LOC_anyVAR) &&
                    possiblealias(p, loctype_(q), locbase_(q), locoff_(q)) )
                    setlocvalue(q, NULL, localiases);
            }
        }
    }
}

static void corruptmem(void)
{   int32 i;
    for (i = 0 ; i < LOCINDEXSIZE ; i++) {
        Location **index = locindex[i];
        int32 j;
        if (index == 0) break;
        for (j = 0 ; j < LOCSEGSIZE ; j++) {
            Location *q = index[j];
            if (q == 0) break;
            if (ispublic(q)) setlocvalue(q, NULL, NULL); /* value of p may be changed */
        }
    }
    /* Also, all future public expressions must be killed.  We record this
     * as location CALLLOC (real locations start at 0).
     */
    cseset_insert(CALLLOC, killedlocations, NULL);
}

/*
 * End of stuff about Locations.
 */

static void blocksetup(void)
{
    int32 i;
    for (i = 0 ; i != HASHSIZE ; i++) {
        Exprn *p = cse_tab[i];
        for ( ; p != NULL ; p = cdr_(p)) {
            exwaslive_(p) = NO;
            while (exlocs_(p) != NULL)
                exlocs_(p) = LocList_DiscardOne(exlocs_(p));
        }
    }
    availableexprns = NULL;
    liveexprns = NULL;
    cseset_insert(0, liveexprns, NULL);
    wantedexprns = NULL;
    killedlocations = NULL;
}

static void cse_corrupt_register(VRegnum r)
{
    setreg(r, NULL, YES);
}

#ifdef TARGET_HAS_SCALED_ADDRESSING
static int32 shiftedval(J_OPCODE op, int32 b)
{
    if (opisshifted(op)) {
        int32 msh = (op & J_SHIFTMASK) >> J_SHIFTPOS;
        if ((msh & SHIFT_RIGHT) == 0)
            b = b << (msh & SHIFT_MASK);
        else if (msh & SHIFT_ARITH)
            b = b >> (msh & SHIFT_MASK);
        else
            b = (int32) (((unsigned32) b) >> (msh & SHIFT_MASK));
    }
    return (op & J_NEGINDEX) ? -b : b;
}

#else
#define shiftedval(op,b) (b)
#endif

#ifdef ONE_FINE_DAY

static bool exprisexprplusk(Exprn *e1, Exprn *e2, int32 *n1, int32 *n2)
{
    if (exop_(e1) == J_ADDK && e1_(e1) == e2) {
        *n1 = e2k_(e1); return YES;
    } else if (exop_(e1) == J_SUBK && e1_(e1) == e2) {
        *n2 = e2k_(e1); return YES;
    } else if (exop_(e2) == J_ADDK && e1_(e2) == e1) {
        *n2 = e2k_(e2); return YES;
    } else if (exop_(e2) == J_ADDK && e1_(e2) == e1) {
        *n1 = e2k_(e1); return YES;
    }
    return NO;
}

#endif

static void KillArc(BlockHead *block, LabelNumber *lab, bool recurse) {
    if (!is_exit_label(lab) && !blk_scanned_(lab->block)) {
        BlockHead *b = lab->block;
        if (blk_pred_(b) == NULL) return;
        blk_pred_(b) = (BlockList *)ndelete((VRegnum)block, (RegList *)blk_pred_(b));
        if (!recurse || blk_pred_(b) != NULL || (blkflags_(b) & BLKSWITCH)) return;
        if (blkflags_(b) & BLK2EXIT) KillArc(b, blknext1_(b), YES);
        KillArc(b, blknext_(b), YES);
    }
}

static LabelNumber *ComparisonDest(int32 a, int32 b, BlockHead *block) {
    bool taken;
    switch (blkflags_(block) & Q_MASK) {
    case Q_UEQ:
    case Q_EQ:  taken = (a == b); break;
    case Q_UNE:
    case Q_NE:  taken = (a != b); break;
    case Q_HS:  taken = ((unsigned32)a >= (unsigned32)b); break;
    case Q_LO:  taken = ((unsigned32)a < (unsigned32)b); break;
    case Q_HI:  taken = ((unsigned32)a > (unsigned32)b); break;
    case Q_LS:  taken = ((unsigned32)a <= (unsigned32)b); break;
    case Q_GE:  taken = (a >= b); break;
    case Q_LT:  taken = (a < b); break;
    case Q_GT:  taken = (a > b); break;
    case Q_LE:  taken = (a <= b); break;
    case Q_AL:  taken = YES; break;
    case Q_NOT: taken = NO; break;
    default:    syserr(syserr_removecomparison, (long)blkflags_(block));
    case Q_XXX: return NULL;
    }
    return taken ? blknext1_(block) : blknext_(block);
}

static void RemoveComparison(int32 a, int32 b, BlockHead *block)
{
    LabelNumber *dest = ComparisonDest(a, b, block);
    LabelNumber *conddest = blknext1_(block);
    if (dest == NULL) return;
    if (debugging(DEBUG_CSE))
        cc_msg("Comparison removed: %sbranch always taken\n",
               dest == conddest ? "" : "other ");
    { LabelNumber *other = dest != conddest ? conddest : blknext_(block);
      blknext_(block) = dest;
      blkflags_(block) &= ~(Q_MASK | BLK2EXIT);
      currenticode->op = J_NOOP;
      /* To give non-local value propagation a better chance, remove this block
         from the predecessors of the block which is the target of the untakeable
         branch (recursively, if that block then becomes unreachable).
       */
      KillArc(block, other, YES);
    }
}

static ExSet *newexprnpart(int32 part, ExSet *set)
{
    ExSet *res = NULL;
    for (; set != NULL; set = cdr_(set)) {
      Exprn *e = set->exprn;
      Exprn *e1 = find_exprn(part, e, NULL, NULL, U_NOTREF+U_NOTDEF);
      if (IsLive(e)) {
        cseset_insert(exid_(e1), liveexprns, NULL);
        cseset_insert(exid_(e1), availableexprns, NULL);
      }
      res = ExSet_Insert(e1, res);
    }
    return res;
}

typedef bool EvalFn(int32 *, int32, int32);

static bool sdiv(int32 *resp, int32 a0, int32 a1) {
/* nb arguments reversed from natural order */
    if (a0 == 0) { cc_warn(sem_warn_divrem_0, s_div); return NO; }
    *resp = a1 / a0;
    return YES;
}

static bool srem(int32 *resp, int32 a0, int32 a1) {
/* nb arguments reversed from natural order */
    if (a0 == 0) { cc_warn(sem_warn_divrem_0, s_rem); return NO; }
    *resp = a1 % a0;
    return YES;
}

static bool udiv(int32 *resp, int32 a0, int32 a1) {
/* nb arguments reversed from natural order */
    if (a0 == 0) { cc_warn(sem_warn_divrem_0, s_div); return NO; }
    *resp = (int32)(((unsigned32)a1) / a0);
    return YES;
}

static bool urem(int32 *resp, int32 a0, int32 a1) {
/* nb arguments reversed from natural order */
    if (a0 == 0) { cc_warn(sem_warn_divrem_0, s_rem); return NO; }
    *resp = (int32)(((unsigned32)a1) % a0);
    return YES;
}


#ifndef TARGET_HAS_MULTIPLY
static bool mul(int32 *resp, int32 a0, int32 a1) {
    *resp = a0 * a1;
    return YES;
}
#endif

static bool EvalUnary(J_OPCODE op, int32 *resp, ExSet *ex) {
  ExSet *c1 = OpInSet(ex, J_MOVK, 0);
  if (c1 != NULL) {
    int32 a1 = shiftedval(op, e1k_(c1->exprn));
    switch (unshiftedop(op)) {
    case J_NOTR: *resp = ~a1; break;
    case J_MOVR: *resp =  a1; break;
    case J_NEGR: *resp = -a1; break;
    }
    if (debugging(DEBUG_CSE) && CSEDebugLevel(1))
      cc_msg("Compile-time evaluable = %ld\n", *resp);
    return YES;
  }
  return NO;
}

static bool EvalBinary(J_OPCODE op, int32 *resp, Exprn *ax, int32 b)
{
    int32 a;
    bool done = YES;
    if (exop_(ax) != J_MOVK) return NO;
    a = e1k_(ax);
    switch (unshiftedop(op & ~(J_SIGNED | J_UNSIGNED))) {
    case J_EXTEND:*resp = (b == 0 || b == 1) ? signed_rightshift_(a<<24, 24) :
                                               signed_rightshift_(a<<16, 16);
                  break;
    case J_ADDK: *resp = a + b; break;
    case J_MULK: *resp = a * b; break;
    case J_ANDK: *resp = a & b; break;
    case J_ORRK: *resp = a | b; break;
    case J_EORK: *resp = a ^ b; break;
    case J_SUBK: *resp = a - b; break;
    case J_RSBK: *resp = b - a; break;

    case J_DIVK: done = (op & J_UNSIGNED) ? udiv(resp, b, a) : sdiv(resp, b, a);
                 break;
    case J_REMK: done = (op & J_UNSIGNED) ? urem(resp, b, a) : srem(resp, b, a);
                 break;

#ifdef TARGET_LACKS_RIGHTSHIFT
    case J_SHLK: if (b >= 0) { *resp = a << b; break; }
                 b = -b;
                 /* fall through to shrk */
#else
    case J_SHLK: *resp = a << b; break;
#endif
    case J_SHRK: *resp = (op & J_UNSIGNED) ? (int32) (((unsigned32) a) >> b) :
                                             TARGET_RIGHTSHIFT(a, b);
                 break;
#ifdef TARGET_HAS_ROTATE
/* Hmm, ROLK is probably more common than RORK.                         */
    case J_RORK: *resp = ((unsigned32)a << (32-b)) | ((unsigned32)a >> b);
                 break;
#endif
    default: syserr(syserr_evalconst, (long)op);
             return NO;
    }
    if (done && debugging(DEBUG_CSE) && CSEDebugLevel(1))
        cc_msg("Compile-time evaluable = %ld\n", *resp);

    return done;
}

static EvalFn *evaluablefn(Expr *fn, VRegnum res, int *args)
{
#ifndef TARGET_HAS_DIVIDE
    if (fn == arg1_(sim.divfn))  { *args = 2; return res == R_A1 ? sdiv : srem; }
    if (fn == arg1_(sim.udivfn)) { *args = 2; return res == R_A1 ? udiv : urem; }
#endif
#if defined(TARGET_LACKS_REMAINDER) || !defined(TARGET_HAS_DIVIDE)
    if (fn == arg1_(sim.remfn))  { *args = 2; return srem; }
    if (fn == arg1_(sim.uremfn)) { *args = 2; return urem; }
#endif
#ifndef TARGET_HAS_MULTIPLY
    if (fn == arg1_(sim.mulfn))  { *args = 2; return mul; }
#endif
    return 0;
}

static bool evaluablecall(Expr *fn, VRegnum res, int32 iargs, int32 fargs, ExSet *arg[], int32 *resp)
{
    int args;
    EvalFn *op = evaluablefn(fn, res, &args);
    bool done = NO;
    if (op == 0) return NO;
    /* All compile-time evaluable fns are binary currently.
       Software FP?
     */
    if (args == iargs && fargs == 0 && arg[0] != NULL) {
        if (args == 1) {
            ExSet *c0 = OpInSet(arg[0], J_MOVK, 0);
            done = (c0 != 0 && op(resp, e1k_(c0->exprn), 0));

        } else if (/* args == 2 && */ arg[1] != NULL) {
            ExSet *c0 = OpInSet(arg[0], J_MOVK, 0);
            ExSet *c1 = OpInSet(arg[1], J_MOVK, 0);
            done = (c0 != 0 && c1 != 0 &&
                    op(resp, e1k_(c0->exprn), e1k_(c1->exprn)));
        }
    }

    if (done && debugging(DEBUG_CSE) && CSEDebugLevel(1))
      cc_msg("Compile-time evaluable fn = %ld\n", *resp);
    return done;
}

static void add_store_access(LocSet *x) {
    if (!stores_r1(currenticode->op)) {
      StoreAccessList *a, **ap = &storeaccesses;
      while ((a = *ap) != NULL)
        if (a->ic->op != J_NOOP && !stores_r1(a->ic->op) && LocSet_Subset(a->locs, x)) {
          LocSet_Discard(a->locs);
          *ap = StoreAccessList_DiscardOne(a);
        } else
          ap = &cdr_(a);
    }
    storeaccesses = StoreAccessList_New(storeaccesses, x, currenticode);
    if (debugging(DEBUG_CSE) && CSEDebugLevel(4)) StoreAccessList_Print(storeaccesses, "\n");
}

#define LocInSet(a, s) member((VRegnum)a, (RegList *)s)

static LocType MaxLocType(LocSet *locs, Location **locp) {
    Location *loc = locs->loc;
    LocType type = loctype_(loc);
    while ((locs = cdr_(locs)) != NULL)
        if (loctype_(locs->loc) > type) {
            loc = locs->loc;
            type = loctype_(loc);
        }
    *locp = loc;
    return type;
}

static void killoldunusedstore(Location *loc, int ignorealiascount)
{ /* If the last reference to loc was in a store instruction, discard it.
     We can ignore stores to something which may be an alias of loc,
     but not loads from a possible alias.
   */
  StoreAccessList *a = storeaccesses;
  LocType type = loctype_(loc);
  for (; a != NULL; a = cdr_(a), --ignorealiascount) {
    if (locbase_(loc) != heapptr && LocInSet(loc, a->locs)) {
      Icode *store = a->ic;
      J_OPCODE op = store->op;
      if (stores_r1(op)) {
        store->op = J_NOOP;
        if (debugging(DEBUG_CSE)) {
          cc_msg("-- killed unused");
          print_jopcode(op, store->r1, store->r2, store->m);
        }
      }
      return;
    } else if (a->ic->op != J_NOOP && !stores_r1(a->ic->op) && type != LOC_VAR &&
               ignorealiascount <= 0) {
      Location *loca;
      LocType typea = MaxLocType(a->locs, &loca);
     /* asymmetry of possiblealias is annoying here */
      if (typea == LOC_PVAR) {
        if ( type != LOC_PVAR &&
             possiblealias(loca, type, locbase_(loc), locoff_(loc)))
          return;
      } else if (typea != LOC_VAR) {
        bool notalias = YES;
        LocSet *s = a->locs;
        for (; s != NULL; s = cdr_(s))
          if (possiblealias(loc, loctype_(s->loc), locbase_(s->loc), locoff_(s->loc)))
            notalias = NO;
          else {
            notalias = YES; break;
          }
        if (!notalias) return;
      }
    }
  }
}

static Exprn *LocSetBase(LocSet *locs) {
  Exprn *base = NULL;
  for (; locs != NULL; locs = cdr_(locs))
    if ((base = adconbase(locbase_(locs->loc), YES)) != NULL)
      break;
  return base;
}

static int NonAliasLoadCount(LocSet *locs) {
  Location *maxloc;
  LocType maxtype = MaxLocType(locs, &maxloc);
  StoreAccessList *a = storeaccesses;
  int i = 0;
  Exprn *base = (maxtype & LOC_anyVAR) ? NULL : LocSetBase(locs);

  for (; a != 0; a = cdr_(a), i++)
    if (a->ic->op != J_NOOP && !stores_r1(a->ic->op) && maxtype != LOC_VAR) {
      Location *loc1;
      switch (MaxLocType(a->locs, &loc1)) {
      case LOC_VAR:
        break;

      case LOC_PVAR:
        if (maxtype == LOC_PVAR) {
          if (locbind_(loc1) == locbind_(maxloc)) return i;
        } else if (base == NULL)
          return i;
        break;

      default:
        { Exprn *base1 = LocSetBase(a->locs);
          if (base == NULL || base1 == NULL || base1 == base)
            return i;
        }
      }
    }
  return i;
}

static void storein_i(VRegnum r1, ExSet *val, LocSet *locs) {
  LocSet *p; ExSet *loadexs = NULL;
  VRegSetP loads = NULL;
  VRegSetP waslive = NULL;
  int32 i;
  bool old;
  for (i = 0, p = locs; p != NULL; i++, p = cdr_(p)) {
    if (IsLive(p->loc->load)) cseset_insert(i, waslive, &old);
    p->oldval = loc_read(p->loc, U_NOTREF, exop_(p->loc->load));
  }
  for (p = locs; p != NULL; p = cdr_(p)) {
    if (ExSetsOverlap(p->oldval, val)) {
      if (debugging(DEBUG_CSE)) {
        cc_msg("-- redundant "); jopprint_opname(currenticode->op);
        cc_msg("\n");
      }
      currenticode->op = J_NOOP;
      for (p = locs; p != NULL; p = cdr_(p))
        locvalue_(p->loc) = ExSet_Append(val, p->oldval);
      return;
    }
  }
  { int n = NonAliasLoadCount(locs);
    for (i = 0, p = locs; p != NULL; i++, p = cdr_(p)) {
      loadexs = ExSet_Insert(p->loc->load, loadexs);
      cseset_insert(exid_(p->loc->load), loads, NULL);
      if (cseset_member(i, waslive)) {
      /* There is a load or store of this location within this block which
       * is still valid.  If a store, we may be able to eradicate it (if
       * there is no intervening load from a possible alias).
       */
        killoldunusedstore(p->loc, n);
      }
    }
  }
  for (p = locs; p != NULL; p = cdr_(p)) {
    ExSet_Discard(p->oldval);
    if (loctype_(p->loc) & LOC_anyVAR)
      setvar(p->loc, val, loads);
    else
      setmem(p->loc, val, loads);
    useoldexprn(p->loc->load, U_NOTREF+U_STORE);
    exwaslive_(p->loc->load) = YES;
  }
  cseset_discard(loads);
  cseset_discard(waslive);
  add_store_access(locs);
  setreg(r1, loadexs, NO);
}

static void storein(VRegnum r1, Location *loc, ExSet *val) {
  storein_i(r1, val, LocSet_New(NULL, loc));
}

static void storeink(VRegnum r1, ExSet *val, LocType type, int32 m, J_OPCODE load, ExSet *bases) {
  LocSet *locs = NULL;
  int32 flags = LOC_REALBASE;
  for (; bases != NULL; bases = cdr_(bases)) {
    Location *loc = find_loc(type, bases->exprn, m, load, flags);
    if (loc != NULL) {
      locs = LocSet_New(locs, loc);
      flags = 0;
    }
  }
  if (locs != NULL)
    storein_i(r1, val, locs);
}

static VRegSetP deleteloadvariant(Location *loc, int32 variant, VRegSetP set)
{
    Exprn *e = find_exprn(exop_(loc->load) | variant, (Exprn *)loc, 0, NULL,
                          U_NOTREF+U_NOTDEF+U_PEEK);
    if (e != NULL)
        return cseset_delete(exid_(e), set, NULL);
    else
        return set;
}

static VRegSetP deleteloads(Location *loc, VRegSetP set)
{
     if (loc == NULL) return set;
     cseset_delete(exid_(loc->load), set, NULL);
     if (is_narrow(loc)) {
     /* must remove all flavours of load */
          set = deleteloadvariant(loc, J_SIGNED, set);
          set = deleteloadvariant(loc, J_UNSIGNED, set);
     }
     return set;
}

#ifndef TARGET_LDRK_MAX
#  define validdisplacement(a, b) (YES)
#else

static bool validdisplacement(J_OPCODE op, int32 n) {
    int32 type = j_memsize(op);
    int32 mink = TARGET_LDRK_MIN, maxk = TARGET_LDRK_MAX;
    int32 quantum = 1;
#ifdef TARGET_LDRFK_MAX
    if (type == MEM_F || type == MEM_D) {
        mink = TARGET_LDRFK_MIN;
        maxk = TARGET_LDRFK_MAX;
    }
#endif
#ifdef TARGET_LDRK_QUANTUM
    quantum = target_ldrk_quantum(locsize(loctype(op)),
                                  (type == MEM_F || type == MEM_D));
#endif
    return n >= mink && n <= maxk && (n & -quantum) == n;
}

#endif /* TARGET_LDRK_MAX */

static bool OpIs32MinusEx(Exprn *e, Exprn *ex) {
  if (exop_(e) == J_SUBR) {
    Exprn *e1 = e1_(e);
    return (exop_(e1) == J_MOVK && e1k_(e1) == 32 &&
            e2_(e) == ex);
  }
  return NO;
}

static Exprn *SHRR_IsLogical(ExSet *e1, ExSet *e2) {
  ExSet *e = NULL;
  if ((e = OpInSet(e1, J_SHRR, J_SIGNED | J_UNSIGNED)) == NULL) {
    if ((e = OpInSet(e2, J_SHRR, J_SIGNED | J_UNSIGNED)) == NULL) {
      return NULL;
    } else {
      ExSet *t = e1; e1 = e2; e2 = t;
    }
  }
  { Exprn *ee = e->exprn;
    Exprn *shift = e2_(ee);
    Exprn *ex;
    e = OpInSet(e2, J_SUBK, 0);
    if (e != NULL && e2k_(e->exprn) == 1) {
      ex = e->exprn;
      ex = e1_(ex);
      if ((exop_(ex) & ~(J_SIGNED | J_UNSIGNED)) == J_SHLR &&
          exop_(e1_(ex)) == J_MOVK && e1k_(e1_(ex)) == 1 &&
            OpIs32MinusEx(e2_(ex), shift))
          return ee;
    } else {
      e = OpInSet(e2, J_NOTR, 0);
      if (e != NULL) {
        ex = e1_(e->exprn);
        if ((exop_(ex) & ~(J_SIGNED | J_UNSIGNED)) == J_SHLR &&
            exop_(e1_(ex)) == J_MOVK && e1k_(e1_(ex)) == -1 &&
            OpIs32MinusEx(e2_(ex), shift))
          return ee;
      }
    }
  }
  return NULL;
}

static bool SignBitClear(ExSet *e) {
  ExSet *ex;
  return (OpInSet(e, J_LDRBK | J_UNSIGNED, NEGINDEX) ||
          OpInSet(e, J_LDRWK | J_UNSIGNED, NEGINDEX) ||
          OpInSet(e, J_LDRBVK | J_UNSIGNED, NEGINDEX) ||
          OpInSet(e, J_LDRWVK | J_UNSIGNED, NEGINDEX) ||
          OpInSet(e, J_LDRBR | J_UNSIGNED, NEGINDEX) ||
          OpInSet(e, J_LDRWR | J_UNSIGNED, NEGINDEX) ||
          ((ex = OpInSet(e, J_MOVK, 0)) != NULL && e1k_(ex->exprn) > 0) ||
          ((ex = OpInSet(e, J_ANDK, 0)) != NULL && e1k_(ex->exprn) > 0));
}

#ifdef TARGET_HAS_ROTATE
static bool FindShiftPair(ExSet *e1, ExSet *e2, ExSet **shl, ExSet **shr) {
  ExSet *e;
  for (; (e1 = OpInSet(e1, J_SHLR, J_SIGNED|J_UNSIGNED)) != NULL;
         e1 = cdr_(e1))
    if ((e = OpInSetWithE1(e2, J_SHRR|J_UNSIGNED, 0, e1_(e1->exprn))) != NULL)
      {
      *shl = e1; *shr = e; return YES;
    }
  return NO;
}

static Exprn *ORRR_IsRORR(ExSet *e1, ExSet *e2) {
  ExSet *ex1, *ex2;

  if (!FindShiftPair(e1, e2, &ex1, &ex2) &&
      !FindShiftPair(e2, e1, &ex1, &ex2))
    return NULL;

  { Exprn *s1 = e2_(ex1->exprn),
          *s2 = e2_(ex2->exprn);
    if (OpIs32MinusEx(s2, s1) || OpIs32MinusEx(s1, s2))
      if (IsLive(ex2->exprn))
        return ex2->exprn;
  }
  return NULL;
}
#endif

#ifdef TARGET_HAS_SCALED_ADDRESSING
static bool ShiftedOutBitsKnownZero(J_OPCODE op, ExSet *e) {
  int32 shift = (op & J_SHIFTMASK) >> J_SHIFTPOS;
  int32 shiftby = shift & SHIFT_MASK;
  if (shiftby == 0)
      return YES;
  else if (shift & SHIFT_RIGHT) {
      ExSet *ex = OpInSet(e, J_ANDK, 0);
      if (ex != NULL && (e2k_(ex->exprn) & ((1L << shiftby) - 1L)) == 0)
          return YES;
      ex = OpInSet(e, J_SHLK, 0);
      if (ex != NULL && e2k_(ex->exprn) > shiftby)
          return YES;
  } else {
      ExSet *ex = OpInSet(e, J_ANDK, 0);
      if (ex != NULL && (e2k_(ex->exprn) & (0xffffffffL << (32-shiftby))) == 0)
          return YES;
      ex = OpInSet(e, J_SHRK+J_UNSIGNED, 0);
      if (ex != NULL && e2k_(ex->exprn) > shiftby)
          return YES;
  }
  return NO;
}
#endif

#ifdef TARGET_ALLOWS_COMPARE_CSES
static bool AddCompare(Exprn *node, void *ignore) {
  IGNORE(ignore);
  cseset_insert(exid_(node), compares, NULL);
  return YES;
}
#endif

static bool AddUse(Exprn *e, void *ignore) {
  IGNORE(ignore);
  if (!cseset_member(exid_(e), availableexprns))
    exuses_(e) = ExprnUse_New(exuses_(e), 0, 0);
  return YES;
}

#ifdef RANGECHECK_SUPPORTED
static bool ExprnWasntLive(Exprn *e, void *c) {
  if (exwaslive_(e)) {
    *(Exprn **)c = e;
    return NO;
  } else {
    return YES;
  }
}
#endif

static ExSet *FindRRSet(int32 op, ExSet *a, ExSet *b, int flags) {
  ExSet *res = NULL;
  int acount = 0, ocount = 0;
  for (; a != NULL && (++acount <= 4 || ocount < 16); a = cdr_(a)) {
    int bcount = 0;
    ExSet *p = b;
    for (; p != NULL && (++bcount <= 4 || ocount < acount * 4); p = cdr_(p)) {
      Exprn *e = find_exprn(op, a->exprn, p->exprn, NULL, flags);
      if (e != NULL) {
        res = ExSet_Insert(e, res);
        ocount++;
      }
    }
  }
  return res;
}

static ExSet *FindRKSet(int32 op, ExSet *a, int32 m, int flags) {
  ExSet *res = NULL;
  for (; a != NULL; a = cdr_(a)) {
    Exprn *e = find_exprn(op, a->exprn, (Exprn *)m, NULL, flags);
    if (e != NULL)
      res = ExSet_Insert(e, res);
  }
  return res;
}

/* @@@ pure OPSYSK */
static struct {
  Exprn *f;
  int32 argres;
} fcsrec;

static ExSet *FindCallSet_aux(
    ExSet *arg[], Exprn *argex[], int32 argno, int32 maxarg, ExSet *res) {
  ExSet *p = arg[argno];
  for (; p != NULL; p = cdr_(p)) {
    argex[argno] = p->exprn;
    if (argno == maxarg) {
      Exprn *e = find_exprn(J_CALLK, fcsrec.f, (Exprn *)fcsrec.argres, argex, 0);
      if (e != NULL) res = ExSet_Insert(e, res);
    } else {
      res = FindCallSet_aux(arg, argex, argno+1, maxarg, res);
    }
  }
  return res;
}

static ExSet *FindCallSet(
    Symstr *f, int32 restype, int32 argdesc, ExSet *arg[]) {
  Exprn *argex[NARGREGS==0 ? 1 : NARGREGS];
  fcsrec.f = (Exprn *)f; fcsrec.argres = argres_setrestype_((argdesc & ~K_FLAGS), restype);
  return FindCallSet_aux(arg, argex, 0, argres_nargs_(argdesc)-1, NULL);
}

static ExSet *FindRes2CallSet(
    Symstr *f, int32 restype, int32 argdesc, ExSet *arg[], Icode *c) {
  ExSet *res = FindCallSet(f, restype, argdesc, arg);
  ExSet *p;
  for (p = res; p != NULL; p = cdr_(p)) {
    Exprn *e = p->exprn;
     /* care needs to be exercised here - the call of find_exprn
        will have made e live, so its use in the RESULT2 exprn below
        will always set exwaslive_.
      */
    bool live = exwaslive_(e);
    Exprn *e2 = find_exprn(J_RESULT2, e, NULL, NULL, U_NOTREF+U_NOTDEF);
    if (!live) {
      exwaslive_(e) = NO;
      exwaslive_(e2) = NO;
    }
    if (&useicode_(exuses_(e)) == c)
      setvalno_(exuses_(e), 1);
    p->exprn = e2;
  }
  return res;
}

static Location *rr_loc(J_OPCODE op, ExSet *e1, ExSet *e2) {
/* Note here that if scaled addressing is not supported the SUB option  */
/* here will never get activated. The apparent test on the NEGINDEX bit */
/* is redundant but harmless.                                           */
  J_OPCODE op1 = opwithshift(((op & NEGINDEX) ? J_SUBR : J_ADDR), op);
  ExSet *rrset = FindRRSet(op1, e1, e2, U_NOTREF+U_NOTDEF);
  return rrset == NULL ? NULL :
                         find_loc(loctype(op), rrset->exprn, 0, j_to_ldrk(op), 0);
}

typedef struct {
  J_OPCODE op;
  Location *loc;
  VRegnum r1;
  bool isload;
} WTLRec;

static bool WorthTryingLocalCSE(Exprn *e, void *w) {
  WTLRec *wp = (WTLRec *)w;
  return ( ( wp->isload &&
             ( extype_(e) != E_LOAD || exloc_(e) != wp->loc)
             /* That is, e is a propagated value */
           ) ||
           ( ( exwaslive_(e) ||
               ( exop_(e) == J_RESULT2 &&
                 exwaslive_(e1_(e))
               )
             ) &&

          /* Surprisingly, it seems to be necessary here to make loads
             of a non-known value into CSEs.  That is,
              ( isload ? node != ... : (exwaslive_(node) || ...) )
             produces a non-working compiler.  What's going on here ?
           */
          /* isint_realreg_(r1) means that the cost of making this a CSE
             reference is likely to be larger by one MOVR.  So if it is
             anyway marginal, we don't make it a CSE.  Something less
             ad-hoc would be an improvement.
           */
             ( !isint_realreg_(wp->r1) ||
               !( wp->op == J_MOVK || wp->op == J_ADCON || wp->op == J_ADCONV)
             )
           )
         );
}

/* Semi-global value propagation.
   Handles forward propagation only (no loops in path)
*/

typedef struct LocValList LocValList;

struct LocValList {
  LocValList *cdr;
  Location *loc;
  ExSet *vals;
};

struct SavedLocVals {
  SavedLocVals *cdr;
  BlockHead *exporter;
  LocValList *locvals;
  StoreAccessList *storeaccesses;
  RegValue *exportedregs;
};

typedef struct {
  int32 mask;
  VRegnum r2;
  ExSet *r2vals;
  int32 m;
} CmpRec;

#ifdef ENABLE_CSE

static void PrintSavedLocVals(LocValList *p, const char *s1, const char *s2) {
  cc_msg("%s{", s1);
  for (; p != NULL; p = cdr_(p)) {
    cc_msg(" {"); cse_print_loc(p->loc); cc_msg(" = "); ExSet_Print(p->vals, "}");
  }
  cc_msg("}%s", s2);
}

#else

static void PrintSavedLocVals(LocValList *p) {
  IGNORE(p);
}

#endif

static LocValList *LocValList_New(LocValList *next, Location *loc, ExSet *vals) {
  return (LocValList *)CSEList3(next, loc, vals);
}

static LocValList *LocValList_DiscardOne(LocValList *vals) {
  return (LocValList *)discard3((VoidStar)vals);
}

static LocValList *LocValList_FindLoc(LocValList *vals, Location *loc) {
  for (; vals != NULL; vals = cdr_(vals))
    if (vals->loc == loc) return vals;
  return NULL;
}

static LocValList *LocValList_Copy(LocValList *v) {
  LocValList *res = NULL, **resp = &res;
  for (; v != NULL; v = cdr_(v)) {
    LocValList *p = LocValList_New(NULL, v->loc, ExSet_Copy(v->vals));
    *resp = p; resp = &cdr_(p);
  }
  return res;
}

static void ImportLocVals(BlockHead *block) {
  if (blk_pred_(block) != NULL && !(var_cc_private_flags & 64L)) {
    if (debugging(DEBUG_CSE) && CSEDebugLevel(2)) {
      SavedLocVals *p = blk_locvals_(block);
      BlockList *pred = blk_pred_(block);
      char *s = "\n  ";
      cc_msg("  predecessors"); for (; pred != NULL; pred = pred->blklstcdr) cc_msg(" %ld", blklabname_(pred->blklstcar));
      for (; p != NULL; p = cdr_(p)) {
        cc_msg("  %s [%ld] ", s, blklabname_(p->exporter));
        PrintSavedLocVals(p->locvals, "", "  ");
        if (p->storeaccesses != NULL) StoreAccessList_Print(p->storeaccesses, "  ");
        if (p->exportedregs != NULL) RegValue_Print(p->exportedregs, "");
        cc_msg("\n");
        s = "+ ";
      }
    }
    if (length((List *)blk_pred_(block)) == length((List *)blk_locvals_(block))) {
      SavedLocVals *sv = blk_locvals_(block);
      /* LocValLists are now no longer shared, but we still need to make a copy
         since the original LocValLists may be wanted later */
      LocValList *vals = LocValList_Copy(sv->locvals);
      if (cdr_(sv) == NULL) storeaccesses = StoreAccessList_Copy(sv->storeaccesses);
      knownregs = sv->exportedregs;
      for (; (sv = cdr_(sv)) != NULL; ) {
        LocValList *p = sv->locvals;
        LocValList *v, **vp = &vals;
        for (; (v = *vp) != NULL;) {
          LocValList *t = LocValList_FindLoc(p, v->loc);
          if (t == NULL) {
            ExSet_Discard(v->vals);
            *vp = LocValList_DiscardOne(v);
          } else {
            v->vals = ExSet_Intersection(v->vals, t->vals);
            if (v->vals == NULL)
              *vp = LocValList_DiscardOne(v);
            else
              vp = &cdr_(v);
          }
        }
        if (sv->exportedregs != NULL) knownregs = sv->exportedregs;
      }
      if (debugging(DEBUG_CSE) && CSEDebugLevel(2))
        if (blk_pred_(block)->blklstcdr != NULL)
          PrintSavedLocVals(vals, " => ", "\n");
      for (; vals != NULL; vals = LocValList_DiscardOne(vals)) {
        ExSet *s;
        bool old;
        locvalue_(vals->loc) = vals->vals;
        ExSet_Map(vals->vals, AddToLocs, vals->loc);
        for (s = vals->vals; s != NULL; s = cdr_(s))
          cseset_insert(exid_(s->exprn), liveexprns, &old);
        cseset_insert(exid_(vals->loc->load), liveexprns, &old);
      }
      { StoreAccessList *sa = storeaccesses;
        RegValue *rv = knownregs;
        bool old;
        for (; sa != NULL; sa = cdr_(sa)) {
          LocSet *locs = sa->locs;
          for (; locs != NULL; locs = cdr_(locs))
            cseset_insert(exid_(locs->loc->load), liveexprns, &old);
        }
        for (; rv != NULL; rv = cdr_(rv)) {
          ExSet *vals = rv->value;
          for (; vals != NULL; vals = cdr_(vals))
            cseset_insert(exid_(vals->exprn), liveexprns, &old);
        }
      }
    }
  }
}

static void SavedLocVals_Add(BlockHead *block, BlockHead *b, LocValList *lv,
                             StoreAccessList *sa, RegValue *rv) {
  blk_locvals_(block) = (SavedLocVals *)syn_list5(blk_locvals_(block), b, lv, sa, rv);
}

static void SaveLocVals(
    BlockHead *block, LabelNumber *lab, LocValList *locvals,
    J_OPCODE op, ExSet *r2vals, int32 n,
    StoreAccessList *sl, RegValue *rv) {
  if (!is_exit_label(lab) && !blk_scanned_(lab->block)) {
    LocValList *lv = LocValList_Copy(locvals);
    if (op != J_NOOP) {
      for (; r2vals != NULL; r2vals = cdr_(r2vals)) {
        int32 n1 = n;
        Location *loc = NULL;
        Exprn *r2ex = r2vals->exprn;
        if (extype_(r2ex) == E_LOAD)
          loc = exloc_(r2ex);
        else if ((exop_(r2ex) == J_ADDK || exop_(r2ex) == J_SUBK) &&
                 extype_(e1_(r2ex)) == E_LOAD) {
          loc = exloc_(e1_(r2ex));
          if (exop_(r2ex) == J_ADDK)
            n1 -= e2k_(r2ex);
          else
            n1 += e2k_(r2ex);
        }
        if (loc != NULL) {
          Exprn *ex = find_exprn(op, (Exprn *)n1, NULL, NULL, U_NOTDEF2+U_NOTREF);
          LocValList *p = LocValList_FindLoc(lv, loc);
          if (p == NULL)
            lv = LocValList_New(lv, loc, ExprnToSet(ex));
          else
            p->vals = ExSet_Insert(ex, p->vals);
        }
      }
    }
    SavedLocVals_Add(lab->block, block, lv, StoreAccessList_Copy(sl), rv);
  }
}

static RegValue *ExportedR2Val(CmpRec *cmpk, LabelNumber *lab) {
  if (!is_exit_label(lab) && !blk_scanned_(lab->block)) {
    BlockHead *b = lab->block;
    if ((blkflags_(b) & BLK2EXIT) && blklength_(b) <= 1) {
      ExSet *rvals = valueinreg(cmpk->r2);
      if (rvals != NULL)
        return RegValue_New(NULL, cmpk->r2, ExSet_Copy(rvals));
    }
  }
  return NULL;
}

static void RewriteNext(BlockHead *b, LabelNumber *oldlab, LabelNumber *newlab,
                        SavedLocVals *sv) {
  bool ok = NO;
  if (blkflags_(b) & BLKSWITCH) {
    int32 n = blktabsize_(b);
    LabelNumber **table = blktable_(b);
    for (; --n >= 0; )
      if (table[n] == oldlab)
        ok = YES, table[n] = newlab;
  } else {
    if ((blkflags_(b) & BLK2EXIT) && blknext1_(b) == oldlab)
      ok = YES, blknext1_(b) = newlab;
    if (blknext_(b) == oldlab)
      ok = YES, blknext_(b) = newlab;
  }
  if (!ok) syserr("RewriteNext %ld: %ld", blklabname_(b), lab_name_(oldlab));
  if (!is_exit_label(newlab)) {
    if (cse_AddPredecessor(newlab, b))
      SavedLocVals_Add(newlab->block, sv->exporter, LocValList_Copy(sv->locvals),
                                      StoreAccessList_Copy(sv->storeaccesses), sv->exportedregs);
  }
}

static void ExportLocVals(BlockHead *block, CmpRec *cmpk, bool sideeffectfree) {
  /* Also, as a side effect, clear the known value of all Locations */
  int32 i;
  SavedLocVals *sv = blk_locvals_(block);
  LocValList *locvals = NULL;
  if (sideeffectfree &&
      !(blkflags_(block) & (BLKLOOP|BLK2EXIT|BLKSWITCH)) &&
      !is_exit_label(blknext_(block)) && !blk_scanned_(blknext_(block)->block)) {
    LabelNumber *lab = blklab_(block);
    for (; sv != NULL; sv = cdr_(sv)) {
      BlockHead *prev = sv->exporter;
      if (debugging(DEBUG_CSE) && CSEDebugLevel(2))
        cc_msg("Block %ld successor rewritten from %ld to %ld\n",
               blklabname_(prev), lab_name_(lab), lab_name_(blknext_(block)));
      KillArc(prev, lab, NO);
      RewriteNext(prev, lab, blknext_(block), sv);
    }
  }
  for (sv = blk_locvals_(block); sv != NULL; sv = cdr_(sv)) {
    LocValList *lv = sv->locvals;
    for (; lv != 0; lv = LocValList_DiscardOne(lv))
      ExSet_Discard(lv->vals);
    StoreAccessList_Discard(sv->storeaccesses);
  }
  for (i = 0 ; i < LOCINDEXSIZE ; i++) {
    Location **index = locindex[i];
    int32 j;
    if (index == 0) break;
    for (j = 0 ; j < LOCSEGSIZE ; j++) {
      Location *q = index[j];
      ExSet *vals;
      if (q == NULL) break;
      vals = locvalue_(q);
      locvalue_(q) = NULL;
      if (IsLive(q->load)) {
        vals = LiveExSet(vals);
        if (vals != NULL) locvals = LocValList_New(locvals, q, vals);
      } else
        ExSet_Discard(vals);
    }
  }
  if (blk_pred_(block) != NULL) {
    if (!(var_cc_private_flags & 64L)) {
      if (blkflags_(block) & BLKSWITCH) {
        int32 n = blktabsize_(block);
        LabelNumber **table = (LabelNumber **)CSEAlloc(n * sizeof(LabelNumber *));
        memcpy(table, blktable_(block), (size_t)n * sizeof(LabelNumber *));
        while (--n > 0) {
          LabelNumber *lab = table[n];
          if (lab != NULL) {
            int32 j;
            J_OPCODE op = J_MOVK;
            for (j = n; --j >= 0; )
              if (table[j] == lab) op = J_NOOP, table[j] = 0;

            SaveLocVals(block, lab, locvals, op, cmpk->r2vals, n-1, NULL, NULL);
          }
        }
        if (table[0] != NULL)
          SaveLocVals(block, table[0], locvals, J_NOOP, NULL, 0, NULL, NULL);

      } else if (blkflags_(block) & BLK2EXIT) {
        J_OPCODE op, op1;
        if (cmpk->mask == Q_EQ)
          op = J_NEK, op1 = J_MOVK;
        else if (cmpk->mask == Q_NE)
          op = J_MOVK, op1 = J_NEK;
        else
          op = op1 = J_NOOP;
        SaveLocVals(block, blknext1_(block), locvals, op1, cmpk->r2vals, cmpk->m,
                    NULL, ExportedR2Val(cmpk, blknext1_(block)));
        SaveLocVals(block, blknext_(block), locvals, op, cmpk->r2vals, cmpk->m,
                    NULL, ExportedR2Val(cmpk, blknext_(block)));

      } else
        SaveLocVals(block, blknext_(block), locvals, J_NOOP, NULL, 0, storeaccesses,
                    cmpk->mask == Q_AL ? NULL : ExportedR2Val(cmpk, blknext_(block)));

    }
  }
  for (; locvals != NULL; locvals = LocValList_DiscardOne(locvals))
    ExSet_Discard(locvals->vals);
}

static void MaybeKillBranchToBlock(BlockHead *block, ExSet *e1, int32 m) {
  SavedLocVals *sv = blk_locvals_(block),
               *nextsv;
  for (; sv != NULL; sv = nextsv) {
    nextsv = cdr_(sv);
    if (!(blkflags_(sv->exporter) & BLKSWITCH)) {
      ExSet *p;
      for (p = e1; p != NULL; p = cdr_(p))
        if (extype_(p->exprn) == E_LOAD) {
          LocValList *vals = LocValList_FindLoc(sv->locvals, exloc_(p->exprn));
          if (vals != NULL) {
            ExSet *c1 = OpInSet(vals->vals, J_MOVK, 0);
            if (c1 != NULL) {
              BlockHead *prev = sv->exporter;
              LabelNumber *lab = blklab_(block);
              LabelNumber *nextlab = ComparisonDest(e1k_(c1->exprn), m, block);
              if (debugging(DEBUG_CSE) && CSEDebugLevel(2))
                cc_msg("Block %ld successor rewritten from %ld to %ld\n",
                       blklabname_(prev), lab_name_(lab), lab_name_(nextlab));

              KillArc(prev, lab, NO);
              RewriteNext(prev, lab, nextlab, sv);
              break;
            }
          }
        }
    }
  }
}

typedef struct {
  Binder *base;
  int32 k;
} AdconRec;

static J_OPCODE AdconInSet(ExSet *e, AdconRec *ad) {
  ExSet *c;
  J_OPCODE op;
  if ( (c = OpInSet(e, J_ADDK, 0)) != NULL &&
       ((op = exop_(e1_(c->exprn))) == J_ADCONV || op == J_ADCON)) {
    ad->k = e2k_(c->exprn);
    ad->base = e1b_(e1_(c->exprn));
    return op;
  }
  if ((c = OpInSet(e, J_ADCONV, 0)) != NULL) {
    ad->k = 0;
    ad->base = e1b_(c->exprn);
    return J_ADCONV;
  }
  if ((c = OpInSet(e, J_ADCON, 0)) != NULL) {
    ad->k = 0;
    ad->base = e1b_(c->exprn);
    return J_ADCON;
  }
  return 0;
}

typedef struct ExSetList { /* Dies with J_POP */
  struct ExSetList *cdr;
  ExSet *set;
} ExSetList;

static void OpRewritten(Icode *c) {
    if (debugging(DEBUG_CSE) && CSEDebugLevel(1)) {
      cc_msg("rewritten: ");
      print_jopcode(c->op, c->r1, c->r2, c->m);
    }
}

static void cse_scanblock(BlockHead *block)
{
#define find_adconv(m) \
  find_exprn(J_ADCONV, (Exprn *) m, 0, NULL, U_NOTDEF+U_NOTREF)
#define estack_push(e) \
  (estack = (ExSetList *)syn_cons2(estack, ExSet_Copy(e)))
#define estack_pop() \
  (estack = (ExSetList *)discard2((VoidStar)estack))

  Icode *c, *limit;
  CmpRec cmpk;
  ExSetList *estack = NULL;
  Location *loc = NULL;
  int32 flags = blkflags_(block);        /* remember these now, because they may */
  LabelNumber *exit = blknext_(block);   /* get altered (by value propagation to */
#ifdef ENABLE_CSE
  LabelNumber *exit1 = blknext1_(block); /* a compare) before we reach the end.  */
#endif
  bool sideeffectfree = YES;
  VRegSetP setnotused = NULL;
  cmpk.mask = Q_AL;
  storeaccesses = NULL;
  cse_currentblock = block;
  blocksetup();
  ImportLocVals(block);
  for (c = blkcode_(block), limit = c + blklength_(block); c < limit; ++c) {
    ExSet *values = NULL;
    ExSet *valuesToStore;
    ExSet *e1, *e2;
    Exprn *node = NULL;
    int32 op = c->op;
    VRegInt r1 = c->r1, r2 = c->r2, m = c->m;
    bool isload = NO,
         symmetric = NO;
    currenticode = c;

    if (debugging(DEBUG_CSE) && CSEDebugLevel(1)) print_jopcode(op, r1, r2, m);
    e1 = NULL; e2 = NULL; node = NULL;
    /*
     * Convert all instructions to nodes of the DAG.
     * Do copy propagation on the fly.
     */

    if (reads_r1(op)) { e1 = valueinreg(r1.r); cseset_delete(r1.r, setnotused, NULL); }
    if (reads_r2(op)) { e1 = valueinreg(r2.r); cseset_delete(r2.r, setnotused, NULL); }
    if (reads_r3(op)) {
        e2 = valueinreg(m.r);
        cseset_delete(m.r, setnotused, NULL);
    } else
        e2 = (ExSet *)DUFF_ADDR;

    if (loads_r1(op)) {
      if (isproccall_(op)) {
        int32 i;
        for (i=0; i<NARGREGS; i++)
          cseset_delete(virtreg(R_A1+i, INTREG), setnotused, NULL);
#ifdef TARGET_STRUCT_RESULT_REGISTER
        if (r2.i & K_STRUCTRET)
          cseset_delete(virtreg(TARGET_STRUCT_RESULT_REGISTER, INTREG), setnotused, NULL);
#endif
#ifndef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
        for (i=0; i<NFLTARGREGS; i++)
          cseset_delete(virtreg(R_FA1+i, DBLREG), setnotused, NULL);
#endif
      } else
        cseset_insert(r1.r, setnotused, NULL);
    }
    switch (unshiftedop(op)) {
    case J_SHRK | J_SIGNED:
    case J_SHRR | J_SIGNED:
      if (!opisshifted(op) && SignBitClear(e1)) {
        c->op = op = op ^ (J_SIGNED | J_UNSIGNED);
        OpRewritten(c);
      }
      break;

#ifdef TARGET_HAS_ROTATE
/* It is probably best to treat ROTATEs like SHIFT.  There are hence    */
/* 4 of them RO{LR}{RK}, but if TARGET_LACKS_ROL then ROL's are done    */
/* with a ROR of 32-n, thus improving current code (for n non-const).   */
/* ARM should probably have: TARGET_HAS_ROTATE, TARGET_LACKS_ROL        */
/* By the way, unless TARGET_HAS_SCALED_OPS then this code catches      */
/* rotates by variables, but not by constants!                          */
    case J_ORRR:
#ifdef TARGET_HAS_SCALED_ADDRESSING
      if (opisshifted(op)) {
        int32 shift = (op & J_SHIFTMASK) >> J_SHIFTPOS;
        ExSet *ex;
        if ( ( ( !(shift & SHIFT_RIGHT) &&
                 (ex = OpInSet(e1, J_SHRK | J_UNSIGNED, 0)) != NULL) ||
               ( (shift & (SHIFT_RIGHT+SHIFT_ARITH)) == SHIFT_RIGHT &&
                 (ex = OpInSet(e1, J_SHLK, J_SIGNED+J_UNSIGNED)) != NULL) ) &&
             ExSet_Member(e1_(ex->exprn), e2) &&
             (shift & SHIFT_MASK) == (32 - e2k_(ex->exprn)) &&
             IsLive(ex->exprn)) {
          Icode *ip = &useicode_(exuses_(ex->exprn));
          c->op = op = J_RORK;
          c->r2 = r2 = ip->r2;
          c->m.i = m.i = (shift & SHIFT_RIGHT) ? 32 - e2k_(ex->exprn) : e2k_(ex->exprn);
          OpRewritten(c);
          e1 = e2;
          e2 = (ExSet *) DUFF_ADDR;
        }
      } else
#endif
        {
        Exprn *shex = ORRR_IsRORR(e1, e2);
        if (shex != NULL && IsLive(shex)) {
          Icode *ip = &useicode_(exuses_(shex));
          e1 = ExprnToSet(e1_(shex));
          e2 = ExprnToSet(e2_(shex));
          c->op = op = J_RORR;
          c->r2 = r2 = ip->r2;
          c->m = m = ip->m;
          OpRewritten(c);
        }
      }
      break;
#endif

    case J_ANDR:
      {
#ifdef TARGET_HAS_SCALED_ADDRESSING
        int32 shift = (op & J_SHIFTMASK) >> J_SHIFTPOS;
        if (shift & SHIFT_RIGHT) {
          ExSet *c1 = OpInSet(e1, J_MOVK, 0);
          if (c1 != NULL) {
            shift &= SHIFT_MASK;
            if (e1k_(c1->exprn) == (1L << (32 - shift)) - 1L) {
              c->op = op = J_SHRK | J_UNSIGNED;
              c->r2 = m;
              c->m.i = m.i = shift;
              OpRewritten(c);
              e1 = e2;
              e2 = (ExSet *) DUFF_ADDR;
            }
          }
        } else if (shift == 0)
#endif
          {
          Exprn *shex = SHRR_IsLogical(e1, e2);
          if (shex != NULL && IsLive(shex)) {
            Icode *ip = &useicode_(exuses_(shex));
            e1 = ExprnToSet(e1_(shex));
            e2 = ExprnToSet(e2_(shex));
            c->op = op = J_SHRR | J_UNSIGNED;
            c->r2 = r2 = ip->r2;
            c->m = m = ip->m;
            OpRewritten(c);
          }
        }
      }
      break;
    }

    switch (unshiftedop(op) & ~(Q_MASK | J_SIGNED | NEGINDEX | J_ALIGNMENT)) {
    case J_CALLK:
#ifdef TARGET_HAS_DIVREM_FUNCTION
      if ((r2.i & K_PURE) && (            /* check not needed?    */
           (Expr *)m.sym == arg1_(sim.divfn) ||
           (Expr *)m.sym == arg1_(sim.udivfn)))
        c->r2.i = r2.i = k_setresultregs_(r2.i, 2);
#endif
    case J_CALLR:
    case J_OPSYSK:  /* @@@ pure OPSYSK */
      { int32 i;
        sideeffectfree = NO;
        if (op == J_CALLK && (r2.i & K_PURE) &&
            k_regargwords_(r2.i) == k_argwords_(r2.i)) {
          ExSet *arg[NARGREGS+NFLTARGREGS==0 ? 1 : NARGREGS+NFLTARGREGS];
          int32 val;
          int32 iargs = k_intregs_(r2.i),
                fargs = k_fltregs_(r2.i);
          for (i = 0 ; i < fargs; i++)
            arg[i] = valueinreg(virtreg(R_FA1+i, DBLREG));
          for (; i < iargs; i++)
            arg[i] = valueinreg(virtreg(R_A1+i, INTREG));
          if (evaluablecall((Expr *)m.sym, r1.r, iargs, fargs, arg, &val)) {
            c = trytokillargs(c, blkcode_(block), (c+1) < limit);
            currenticode = c;
            c->m.i  = val;
            r1 = c->r1;
            goto ForgeMOVK;
          }
#ifdef TARGET_HAS_DIV_10_FUNCTION
          if (
#ifndef TARGET_HAS_DIVREM_FUNCTION
              (Expr *)m.sym == arg1_(sim.remfn) ||
              (Expr *)m.sym == arg1_(sim.uremfn) ||
#endif
              (Expr *)m.sym == arg1_(sim.divfn) ||
              (Expr *)m.sym == arg1_(sim.udivfn) ) {

            ExSet *v1 = OpInSet(arg[0], J_MOVK, 0);
            if (v1 != NULL && e1k_(v1->exprn) == 10) {
              VRegnum a1 = virtreg(R_A1, INTREG),
                      a2 = virtreg(R_A1+1, INTREG);
              Icode *a1p = NULL, *a2p = NULL;
              Icode *p = c, *base = blkcode_(block);
              for (; --p >= base; ) {
                if (loads_r1(p->op)) {
                  if (a1p == NULL && p->r1.r == a1) {
                    a1p = p;
                    if (a2p != NULL) break;
                  } else if (a2p == NULL && p->r1.r == a2) {
                    a2p = p;
                    if (a1p != NULL) break;
                  }
                }
              }
              if (a1p != NULL && a2p != NULL) {
                arg[0] = arg[1];
                a2p->r1.r = a1;
                a1p->r1.r = vregister(INTREG);
                iargs = 1;
                r2.i = c->r2.i = k_argdesc_(1, 0, 2, 1, r2.i & K_FLAGS);
                m.sym = c->m.sym =
#ifndef TARGET_HAS_DIVREM_FUNCTION
                    (Expr *)m.sym == arg1_(sim.remfn) ? (Symstr *)arg1_(sim.rem10fn) :
                    (Expr *)m.sym == arg1_(sim.uremfn) ? (Symstr *)arg1_(sim.urem10fn) :
#endif
                    (Expr *)m.sym == arg1_(sim.divfn) ? (Symstr *)arg1_(sim.div10fn) :
                                                        (Symstr *)arg1_(sim.udiv10fn);
                OpRewritten(c);
              }
            }
          }
#endif
/* It is not clear to AM that we need vregsort below, the reg suffices! */
          if (k_resultregs_(r2.i) > 1 && r1.r != R_A1) {
            values = FindRes2CallSet(m.sym, vregsort(r1.r), r2.i, arg, c);
          } else {
            values = FindCallSet(m.sym, vregsort(r1.r), r2.i, arg);
          }
        } else {
          corruptmem();
        }
/* @@@ This set of things amounts to  'corrupt all non-callee-save registers'.
   Is there a better way to express this now? */
/* @@@ REVIEW/bugfix in the light of ACN code to preserve such regs in */
/* regalloc if regmaskvec for callee allows?                              */
        for (i=0; i<NARGREGS; i++)
          cse_corrupt_register(virtreg(R_A1+i, INTREG));
#ifdef TARGET_STRUCT_RESULT_REGISTER
        if (r2.i & K_STRUCTRET)
          cse_corrupt_register(virtreg(TARGET_STRUCT_RESULT_REGISTER, INTREG));
#endif
#ifndef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
        for (i=0; i<NFLTARGREGS; i++)
          cse_corrupt_register(virtreg(R_FA1+i, DBLREG));
#endif
            /* what about R_T1...R_Tn and R_FT1...R_FTn?                */
#ifndef TARGET_STACKS_LINK
        cse_corrupt_register(virtreg(R_LR, INTREG));
#endif
      }
      if (op == J_CALLK &&
          !(var_cc_private_flags & 32L) &&
          returnsheapptr(bindsym_(m.b)))
        node = heapptr;
      break;

    case J_ADCON:
    case J_ADCOND: case J_ADCONF:
    case J_FNCON:
    case J_ADCONV:
      node = find_exprn(op, (Exprn *)m.b, 0, NULL, 0);
      break;

    case J_PUSHR:
    case J_PUSHF:
      estack_push(e1);
      break;

    case J_PUSHD:
      if (e1) {
        estack_push(newexprnpart(CSE_WORD2, e1));
        estack_push(newexprnpart(CSE_WORD1, e1));
      } else {
        estack_push(NULL); estack_push(NULL);
      }
      break;

/* AM: note that when J_POP goes, the entire 'estack' edifice dies too! */
    case J_POP:
      { RegList *p = (RegList *)dreverse((List *)m.rl);
        RegList *pcopy = p;
        for (; p != NULL; p = p->rlcdr) {
          ExSet *e;
          if (estack == NULL) {
            e = NULL;
          } else {
            e = LiveExSet(estack->set);
            estack_pop();
          }
          setreg(p->rlcar, e, YES);
        }
        c->m.rl = (RegList *)dreverse((List *)pcopy);
      }
      break;

    case J_LDRVK:  case J_LDRLVK:
    case J_LDRFVK: case J_LDRDVK:
    case J_LDRBVK: case J_LDRWVK:
      { Exprn *base = find_adconv(m.b);
        loc = find_loc(loctype(op), base, r2.r, j_to_ldrk(op), 0);
        values = loc_read(loc, 0, op);
      }
      isload = YES;
      break;

    case J_STRVK:  case J_STRLVK:
    case J_STRFVK: case J_STRDVK:
    case J_STRBVK: case J_STRWVK:
      { Exprn *base = find_adconv(m.b);
        ExSet *val = valueinreg(r1.r);
        sideeffectfree = NO;
        loc = find_loc(loctype(op), base, r2.r, j_to_ldrk(op), 0);
        storein(r1.r, loc, val);
      }
      break;

    case J_LDRR:  case J_LDRLR:
    case J_LDRFR: case J_LDRDR:
    case J_LDRBR: case J_LDRWR:
      { ExSet *c2;
        e1 = readreg(r2.r);
        e2 = readreg(m.r);
        c2 = OpInSet(e2, J_MOVK, 0);
        if (c2 != NULL) {
          int32 a2 = shiftedval(op, e1k_(c2->exprn));
          if (validdisplacement(op, a2)) {
            c->op = op = j_to_ldrk(op) | (op & (J_SIGNED | J_UNSIGNED));
            c->m.i = m.i = a2;
            OpRewritten(c);
            goto TransformedToLDRK;
          }
        }
        loc = rr_loc(op, e1, e2);
        if (loc != NULL) values = loc_read(loc, 0, op);
      }
      isload = YES;
      break;

    case J_LDRK:  case J_LDRLK:
    case J_LDRFK: case J_LDRDK:
    case J_LDRBK: case J_LDRWK:
      e1 = readreg(r2.r);
TransformedToLDRK:
      { ExSet *p = e1;
        for (; p != NULL; p = cdr_(p))
          if ((loc = find_loc(loctype(op), p->exprn, m.i, j_to_ldrk(op), LOC_REALBASE)) != NULL)
            break;
      }
      if (loc != NULL) values = loc_read(loc, 0, op);
      isload = YES;
      break;

    case J_STRR:  case J_STRLR:
    case J_STRFR: case J_STRDR:
    case J_STRBR: case J_STRWR:
      { ExSet *c2;
        sideeffectfree = NO;
        valuesToStore = valueinreg(r1.r);
        e1 = readreg(r2.r);
        e2 = readreg(m.r);
        c2 = OpInSet(e2, J_MOVK, 0);
        if (c2 != NULL) {
          int32 a2 = shiftedval(op, e1k_(c2->exprn));
          if (validdisplacement(op, a2)) {
            c->op = op = J_LDtoST(j_to_ldrk(op));
            c->m.i = m.i = a2;
            OpRewritten(c);
            goto TransformedToSTRK;
          }
        }
        loc = rr_loc(op, e1, e2);
        if (loc != NULL) storein(r1.r, loc, valuesToStore);
      }
      break;

    case J_STRK:  case J_STRLK:
    case J_STRFK: case J_STRDK:
    case J_STRBK: case J_STRWK:
      sideeffectfree = NO;
      valuesToStore = valueinreg(r1.r);
      e1 = readreg(r2.r);
TransformedToSTRK:
      { AdconRec ad;
        if (AdconInSet(e1, &ad) == J_ADCONV) {
          Exprn *base = find_adconv(ad.base);
          op = c->op = J_addvk(op);
          r2.r = c->r2.r = m.i + ad.k;
          m.b = c->m.b = ad.base;
          OpRewritten(c);

          loc = find_loc(loctype(op), base, r2.r, j_to_ldrk(op), 0);
          storein(r1.r, loc, valuesToStore);
        } else
          storeink(r1.r, valuesToStore, loctype(op), m.i, j_to_ldrk(op), e1);
      }
      break;

    case J_LDRV1: case J_LDRFV1: case J_LDRDV1:
      op = J_V1toV(op);
    case J_LDRV: case J_LDRFV: case J_LDRDV:
      loc = find_loc(LOC_VAR, NULL, m.i, j_to_ldrk(op), 0);
      values = loc_read(loc, 0, op);
      isload = YES;
      break;

    case J_STRV: case J_STRFV: case J_STRDV:
      sideeffectfree = NO;
      loc = find_loc(LOC_VAR, NULL, m.i, j_to_ldrk(op), 0);
      storein(r1.r, loc, e1);
      break;

    case J_MOVR:
      if (EvalUnary(op, &c->m.i, e2)) goto ForgeMOVK;
    case J_MOVFR: case J_MOVDR:
      if (e2 != NULL) {
        values = e2;
        ExSet_Map(e2, AddUse, NULL);
      }
      break;

#ifdef TARGET_HAS_BLOCKMOVE
/* AM: In the following 'n' is a multiple of alignof_struct.  However,  */
/* it may be desirable to know whether the front-end can guarantee      */
/* that a MOVC of (say) 12 can is word-aligned if alignof_struct=1.     */
/* Future direction: use J_SHIFTMASK for this?                          */
    case J_MOVC:   /* MOVC dest, source, n */
    case J_CLRC:   /* CLRC dest, -, n      */
      /* These have implications for value propagation, and for
         elimination of unwanted stores.
         Also, they may (depending on target) destroy some registers.
         Because this is in flux, there are no written rules.
       */
      { ExSet *c1 = OpInSet(e1, J_ADCON, 0);
        sideeffectfree = NO;
        if (c1 == NULL) c1 = OpInSet(e1, J_ADCONV, 0);
        if (c1 == NULL) {
          corruptmem();
        } else {
          int32 i = 0, mem = MEM_I, span = 4; J_OPCODE ldop = J_LDRK;
          if (alignof_struct < 4) mem = MEM_B, span = 1, ldop = J_LDRBK;
          for (; i < m.i; i += span) {
            Location *loc = find_loc(LOC_(mem), c1->exprn, i, ldop, LOC_REALBASE);
            setlocvalue(loc, NULL, NULL);
          }
        }
      }
      node = NULL;
      break;
#endif

    case J_SETSPENV:
    case J_SETSPGOTO:
      setsplist = (SetSPList*)CSEList3((int32)setsplist,
                                       (int32)cse_currentblock,
                                       (int32)currenticode);
    case J_ENTER:
    case J_INIT: case J_INITF: case J_INITD:
    case J_INFOLINE: case J_INFOSCOPE: case J_INFOBODY:
    case J_COUNT:
    case J_WORD:
      sideeffectfree = NO;
      node = NULL;
      break;

    case J_CASEBRANCH:
      sideeffectfree = NO;
      node = NULL;
      cmpk.r2vals = e1;
      break;

    case J_CMPFK: case J_CMPDK:
      e2 = ExprnToSet((Exprn *)canonicalfpconst(m.f));

    case J_CMPFR: case J_CMPDR:
      if (e1 != NULL && e2 != NULL) {
        if (ExSetsOverlap(e1, e2)) {
          RemoveComparison(0, 0, block);
        }
#ifdef TARGET_ALLOWS_COMPARE_CSES
        else {
          values = FindRRSet(op & ~CSE_COMPARE_MASK, e1, e2, 0);
          ExSet_Map(values, AddCompare, NULL);
        }
#endif
      }
      break;

#ifdef RANGECHECK_SUPPORTED
    case J_CHKLR: case J_CHKUR:
      { ExSet *c2 = OpInSet(e2, J_MOVK, 0);
        if (c2 == NULL) {
          values = FindRRSet(op, e1, e2, 0);
          break;
        }
        m.i = shiftedval(op, e1k_(c2->exprn));
      }
      /* and fall through */
    case J_CHKLK: case J_CHKUK:
      { ExSet *c1 = OpInSet(e1, J_MOVK, 0);
        if (c1 != NULL) {
          int32 index = e1k_(c1->exprn);
          int32 realop = unshiftedop(op) & ~(Q_MASK | J_SIGNED);
          if ((realop == J_CHKLK || realop == J_CHKLR) ? (index >= m.i) :
                                                         (index <= m.i)) {
            c->op = J_NOOP;
            OpRewritten(c);
            node = NULL;
            break;
          }
        }
      }
      values = FindRKSet(op, e1, m.i, 0);
      break;

    case J_CHKNEK:
      { ExSet *c1 = OpInSet(e1, J_MOVK, 0);
        if (c1 != NULL && e1k_(c1->exprn) != m.i) {
          c->op = J_NOOP;
          OpRewritten(c);
          node = NULL;
          break;
        }
      }
      values = FindRKSet(op, e1, m.i, 0);
      break;

    case J_CHKNEFR:
    case J_CHKNEDR:
      break;

#endif
    case J_CMPR:
      if (e1 == NULL || e2 == NULL)
        break;

      { ExSet *c2 = OpInSet(e2, J_MOVK, 0);
        int32 cond = op & Q_MASK;
        int32 n1 = 0, n2 = 0;
        if (c2 == NULL) {
          if (ExSetsOverlap(e1, e2)) {
            RemoveComparison(0, 0, block);

          } else if ((c2 = OpInSet(e1, J_MOVK, 0)) != NULL &&
                     (cond == Q_EQ || cond == Q_NE ||
                      cond == Q_UEQ || cond == Q_UNE)
#ifdef TARGET_HAS_SCALED_ADDRESSING
                     && ShiftedOutBitsKnownZero(op, e2)
#endif
                     )
            {
            n1 = e1k_(c2->exprn);
            n2 = shiftedval(op ^ (SHIFT_RIGHT << J_SHIFTPOS), n1);
            if (shiftedval(op, n2) != n1)
              RemoveComparison(0, 1, block);
            else {
              e1 = e2;
              r2.r = c->r2.r = m.r;
              goto ConvertToCMPK;
            }
          }
#ifdef ONE_FINE_DAY
/*@@@ something along the following lines may well be a good idea, but I have
      to be sure that the constant was actually written, not inferred from the
      known value of some variable - otherwise, the possibility of overflow
      prohibits this.
 */
          else if (!opisshifted(op) && exprisexprplusk(e1, e2, &n1, &n2)) {
            RemoveComparison(n1, n2, block);
          }
#endif
#ifdef TARGET_ALLOWS_COMPARE_CSES
          else {
            values = FindRRSet(op & ~CSE_COMPARE_MASK, e1, e2, 0);
            ExSet_Map(values, AddCompare, NULL);
          }
#endif
          break;

        } else {
          ExSet *c1 = OpInSet(e1, J_MOVK, 0);
          n2 = shiftedval(op, e1k_(c2->exprn));
          if (c1 != NULL) {
            RemoveComparison(e1k_(c1->exprn), n2, block);
            break;
          }
        }
ConvertToCMPK:
        op = c->op = J_RTOK(unshiftedop(op));
        m.i = c->m.i = n2;
        OpRewritten(c);
      }
      /* and the second arg. constant, first not case falls through */
    case J_CMPK:
      { ExSet *c1 = OpInSet(e1, J_MOVK, 0);
        int32 mask = op & (Q_MASK & ~Q_UBIT);
        AdconRec ad;
        cmpk.mask = mask; cmpk.r2 = r2.r; cmpk.r2vals = e1; cmpk.m = m.i;
        if (c1 != NULL) {
          RemoveComparison(e1k_(c1->exprn), m.i, block);
        } else if ( (mask == Q_EQ || mask == Q_NE)
                    &&
                    (  /* Special for CFront's demented constructor invocation */
                      (m.i == 0 && AdconInSet(e1, &ad) != 0 && !(bindstg_(ad.base) & bitofstg_(s_weak))) ||
                      ( (c1 = OpInSet(e1, J_NEK, 0)) != NULL &&
                        e1k_(c1->exprn) == m.i) ) )

          RemoveComparison(1, 0, block);
        else {
          if (sideeffectfree && setnotused == NULL && !(flags & BLKREXPORTED))
            MaybeKillBranchToBlock(block, e1, m.i);
          if (!immed_cmp(m.i)) {
            op = J_MOVK;
            node = find_movk(m.i, 0);
          }
#ifdef TARGET_ALLOWS_COMPARE_CSES
          else if (e1 != NULL) {
            values = FindRKSet(op & ~CSE_COMPARE_MASK, e1, m.i, 0);
            ExSet_Map(values, AddCompare, NULL);
          }
#endif
        }
      }
      break;

    case J_USE: case J_USEF: case J_USED:
    case J_VSTORE:
      /* I think I have to assume that this directly follows a load
       * or store.  I can't get from the register to the store location
       * in the case of stores, if the value being stored was known.
       */
      sideeffectfree = NO;
      wantedexprns = deleteloads(loc, wantedexprns);
      availableexprns = deleteloads(loc, availableexprns);
      liveexprns = deleteloads(loc, liveexprns);
      node = NULL;
      break;

    case J_MOVIFR: case J_MOVIDR: case J_MOVFIR:
      cse_corrupt_register(r1.r);
      node = NULL;
      break;

    case J_MOVDIR: case J_MOVLIR:
      cse_corrupt_register(r1.r);
      cse_corrupt_register(r2.r);
      node = NULL;
      break;

    case J_MOVFK: case J_MOVDK:
      node = find_exprn(op, (Exprn *)canonicalfpconst(m.f), 0, NULL, 0);
      break;

    case J_STRING:
      node = find_exprn(op, (Exprn *)m.s, 0, NULL, 0);
      break;

    case J_NEGR:
    case J_NOTR:
      if (EvalUnary(op, &c->m.i, e2)) goto ForgeMOVK;

    case J_NEGFR: case J_NEGDR:
    case J_FLTFR: case J_FLTDR:
    case J_FIXFR: case J_FIXDR:
    case J_MOVDFR: case J_MOVFDR:
      if (e2 != NULL)
        values = FindRKSet(op, e2, 0, 0);
      break;

    case J_ADDFK: case J_ADDDK:
    case J_MULFK: case J_MULDK:
    case J_SUBFK: case J_SUBDK:
    case J_DIVFK: case J_DIVDK:
      if (e1 != NULL)
        values = FindRKSet(op, e1, (int32)canonicalfpconst(m.f), 0);
      break;

    case J_ADDFR: case J_ADDDR:
    case J_MULFR: case J_MULDR:
    case J_SUBFR: case J_SUBDR:
    case J_DIVFR: case J_DIVDR:
      if (e1 != NULL && e2 != NULL)
      { /* could be code here as in cg to fold fp constops...           */
#ifndef TARGET_LACKS_FP_LITERALS
        /* ... and also some code in here as in cg for J_RTOK().        */
#endif
        values = FindRRSet(op, e1, e2, 0);
      }
      break;

    case J_INLINE1: case J_INLINE1F: case J_INLINE1D:
      if (e1 != NULL)
        values = FindRKSet(op, e1, m.i, 0);
      break;

    case J_ADDR: case J_MULR: case J_ANDR: case J_ORRR: case J_EORR:
      symmetric = YES;
    case J_SUBR: case J_DIVR: case J_RSBR: case J_REMR:
    case J_SHLR: case J_SHRR: case J_RORR:
      if (e1 != NULL && e2 != NULL) {
        ExSet *c2 = OpInSet(e2, J_MOVK, 0);
        if (c2 != NULL) {
          int32 a2 = shiftedval(op, e1k_(c2->exprn));
          ExSet *c1 = OpInSet(e1, J_MOVK, 0);
          if (c1 != NULL &&
              EvalBinary(J_RTOK(op), &c->m.i, c1->exprn, a2)) {
            goto ForgeMOVK;
          } else if (jop_canRTOK(unshiftedop(op))) {
            op = c->op = J_RTOK(unshiftedop(op));
            c->m.i = a2;
            OpRewritten(c);
            values = FindRKSet(op, e1, a2, 0);
            break;
          }
        }
        if (!opisshifted(op) && symmetric) {
          ExSet *c1 = OpInSet(e1, J_MOVK, 0);
          if (c1 != NULL && jop_canRTOK(op)) {
            op = c->op = J_RTOK(op);
            c->r2.i = m.r;
            c->m.i = m.i = e1k_(c1->exprn);
            OpRewritten(c);
            values = FindRKSet(op, e2, m.i, 0);
            break;
          }
        }
        values = FindRRSet(op, e1, e2, 0);
      }
      break;

    case J_ADDK: case J_MULK: case J_ANDK: case J_ORRK: case J_EORK:
    case J_SUBK: case J_DIVK: case J_RSBK: case J_REMK:
    case J_SHLK: case J_SHRK: case J_RORK:
    case J_EXTEND:
      if (e1 != NULL) {
        ExSet *c1 = OpInSet(e1, J_MOVK, 0);
        if (c1 != NULL &&
#ifdef TARGET_LACKS_MULTIPLY_LITERALS /* @@@ eh? */
            (unshiftedop(op) & ~(Q_MASK | J_SIGNED)) != J_MULK &&
#endif
#ifdef TARGET_LACKS_DIVIDE_LITERALS
            (unshiftedop(op) & ~(Q_MASK | J_SIGNED)) != J_DIVK &&
            (unshiftedop(op) & ~(Q_MASK | J_SIGNED)) != J_REMK &&
#endif
            EvalBinary(op, &c->m.i, c1->exprn, m.i) ) {
          goto ForgeMOVK;
        } else {
          values = FindRKSet(op, e1, m.i, 0);
        }
      }
      break;

    ForgeMOVK:
      op = c->op = J_MOVK;
      c->r2.r = GAP;
      OpRewritten(c);
    case J_MOVK:
      node = find_movk(c->m.i, (c+1 < limit && (c+1)->op == J_STRV) ||
                               isint_realreg_(r1.r) ? U_NOTDEF2+U_NOTREF : 0);
      break;

    default:
      syserr(syserr_scanblock, (long)op);
      node = NULL;
      break;

    } /* switch */

    if (values == NULL && node != NULL)
      values = ExprnToSet(node);

    if (debugging(DEBUG_CSE) && CSEDebugLevel(1))
      ExSet_Print(values, "\n");

#ifdef RANGECHECK_SUPPORTED
    if (j_is_check(op) && values != NULL) {
      if (!ExSet_Map(values, ExprnWasntLive, &node)) {
        c->op = J_NOOP;
        if (debugging(DEBUG_CSE) && CSEDebugLevel(1))
          cc_msg("-- local CSE reference [%ld] -> NOOP\n", exid_(node));
      } else {
        ExSet_Map(values, SetExWasLive, NULL);
      }
    } else
#endif
    if (loads_r1(op)) {
    /* what I really want to say here is try for a local cse only if nothing
       in values suggests it's not sensible
     */
      WTLRec wtlrec;
      wtlrec.op = op; wtlrec.loc = loc;
      wtlrec.r1 = r1.r; wtlrec.isload = isload;
      if ( values != NULL &&
           !register_movep(op) &&
           ExSet_Map(values, WorthTryingLocalCSE, &wtlrec) ) {

        node = values->exprn;
        /* the first is as good as any - definitely only want one */
        if (exop_(node) == J_RESULT2) {
          Exprn *e1 = e1_(node);
          if (cseset_member(exid_(e1), availableexprns))
            isload &= !addlocalcse(e1, 1, block);
        } else if (exop_(node) != CSE_WORD1 && exop_(node) != CSE_WORD2) {
          if (cseset_member(exid_(node), availableexprns))
            isload &= !addlocalcse(node, 0, block);
        }
        /* If addlocalcse returned NO, maybe we should think about
           patching the load (if isload).  Or maybe we should do it
           anyway, since the CSE might later be discarded.  If we don't,
           the CSE had better not be discarded! (or a believed to be
           redundant store of the value used here may be killed).
         */
      }
      if (isload)
        add_store_access(LocSet_New(NULL, loc));
      setreg(r1.r, values, YES);
    }
#ifdef TARGET_ALLOWS_COMPARE_CSES
    if (alterscc(c))
      cseset_insert(CCLOC, killedlocations, NULL);
#endif
  }
  while (estack != NULL) estack_pop();

  if (!(flags & BLK2EXIT) && is_exit_label(exit)) {
    /* We may kill all unused stores to local objects.  regalloc will do
       better with non-address-taken binders, but not with address-taken
       ones or local structs.
     */
    StoreAccessList *a;
    for (a = storeaccesses; a != NULL; a = cdr_(a)) {
      Icode *ic = a->ic;
      if (stores_r1(ic->op)) {
        Location *loc;
        LocType type = MaxLocType(a->locs, &loc);
        if (type & LOC_anyVAR) {
          if (IsLive(loc->load) &&
              (bindstg_(locbind_(loc)) &
               (bitofstg_(s_auto) | b_globalregvar))
                  == bitofstg_(s_auto))
            killoldunusedstore(loc, 0);
        } else {
          LocSet *locs = a->locs;
          for (; locs != NULL; locs = cdr_(locs))
            if (exop_(locbase_(loc)) == J_ADCONV) {
              if (IsLive(loc->load))
                killoldunusedstore(loc, 0);
              break;
            }
        }
      }
    }
  }
  ExportLocVals(block, &cmpk, sideeffectfree && setnotused == NULL);
  blk_scanned_(block) = YES;
  for (; knownregs != NULL; knownregs = RegValue_DiscardOne(knownregs))
    ExSet_Discard(rv_val_(knownregs));
  StoreAccessList_Discard(storeaccesses);
  storeaccesses = NULL;
#ifdef ENABLE_CSE
  if (debugging(DEBUG_CSE) && CSEDebugLevel(1))
    cse_printexits(flags, exit, exit1);
#endif
  blk_available_(block) = availableexprns;
  blk_wanted_(block) = wantedexprns;
  blk_killed_(block) = killedlocations;
#undef find_adconv
#undef find_addrr
#undef estack_push
}

static void csescan_setup(void)
{
    memclr(exprnindex, EXPRNINDEXSIZE * sizeof(Exprn **));
    cse_tab = (Exprn **) CSEAlloc(HASHSIZE * sizeof(Exprn **));
    memclr(cse_tab, HASHSIZE * sizeof(Exprn **));
    memclr(locindex, LOCINDEXSIZE * sizeof(Location **));
    locations = (Location **) CSEAlloc(LOCHASHSIZE * sizeof(Location **));
    memclr(locations, LOCHASHSIZE * sizeof(Location **));
    cseidsegment = CSEIDSEGSIZE;
    heapptr = (Exprn *)CSEAlloc(sizeof(Exprn));
    exop_(heapptr) = J_HEAPPTR;
    heapptr->nodeid = mknodeid_(0, EX_ALIAS|E_UNARYK);
    exuses_(heapptr) = NULL;
    exwaslive_(heapptr) = YES;
    csealiasid = csealiaslimit = 1;
    csenonaliasid = 0; csenonaliaslimit = CSEIDSEGSIZE;
    locationid = 0;
    floatconlist = NULL;
    knownregs = NULL;
    loadrs = NULL;
#ifdef TARGET_ALLOWS_COMPARE_CSES
    compares = NULL;
#endif
}

#ifdef EXPERIMENT_PRUNEEXPRNS
/* Temporarily disabled, because it was pruning too much (see comment below) */

static void renumber_cb(int32 id, VoidStar arg)
{
    VRegSetP *newp = (VRegSetP *)arg;
    int32 newid = exnewid_(exprn_(id));
    if (newid != 0) {
        bool oldp;
        cseset_insert(newid, *newp, &oldp);
    }
}

static VRegSetP renumber(VRegSetP set)
{
    VRegSetP newset = NULL;
    vregset_map(set, renumber_cb, (VoidStar)&newset);
    vregset_discard(set);
    return newset;
}

static void pruneandrenumberexprns(BlockHead *top)
{
    int32 i, j, count = 0;
    Exprn *left = NULL;
    cseidsegment = CSEIDSEGSIZE;
    csealiasid = csealiaslimit = 0;
    csenonaliasid = 0; csenonaliaslimit = CSEIDSEGSIZE;
    if (debugging(DEBUG_CSE)) cc_msg("\nDiscarding exprns");

    for (i = 0; i < EXPRNINDEXSIZE; i++) {
        Exprn **p = exprnindex[i];
        if (p == NULL) break;
        for (j = 0; j < EXPRNSEGSIZE; j++) {
            Exprn *ex = p[j];
            if (ex != NULL) {
                ExprnUse *use = exuses_(ex);
                /* if this is activated, only-once subexpressions of more-than-once
                   expressions will not be considered for lifting out of the whole
                   function "loop", and so will get loaded twice if the larger
                   expression is lifted.
                 */
                if (use == NULL) syserr(syserr_prune);
                /* are we sure that use must be non-NULL here?  If not,
                      test for NULL below).
                 */
                if (cdr_(use) == NULL && !(flags_(use) & U_LOCALCSE) &&
                    blknest_(use->block) <= 1) {
                    if (debugging(DEBUG_CSE)) {
                        cc_msg(" %ld", exid_(ex));
                        if ((++count) % 20 == 0) cc_msg("\n");
                    }
                    exnewid_(ex) = 0;
                } else {
                    if (exalias_(ex)) {
                        if (++csealiasid >= csealiaslimit) {
                            csealiasid = cseidsegment;
                            csealiaslimit = (cseidsegment += CSEIDSEGSIZE);
                        }
                        exnewid_(ex) = csealiasid;
                    } else {
                        if (++csenonaliasid >= csenonaliaslimit) {
                            csenonaliasid = cseidsegment;
                            csenonaliaslimit = (cseidsegment += CSEIDSEGSIZE);
                        }
                        exnewid_(ex) = csenonaliasid;
                    }
                    cdr_(ex) = left;
                    left = ex;
                }
            }
        }
    }
    if (debugging(DEBUG_CSE)) cc_msg(" [%ld]\n", count);
    {   BlockHead *bp;
        for (bp = top; bp != NULL; bp = blkdown_(bp)) {
            blk_available_(bp) = renumber(blk_available_(bp));
            blk_wanted_(bp) = renumber(blk_wanted_(bp));
        }
    }
    for (i = 0 ; i < LOCINDEXSIZE ; i++) {
        Location **index = locindex[i];
        if (index == 0) break;
        for (j = 0 ; j < LOCSEGSIZE ; j++) {
            Location *q = index[j];
            if (q == 0) break;
            q->users = renumber(q->users);
            q->aliasusers = renumber(q->aliasusers);
        }
    }
    for (i = 0; i < EXPRNINDEXSIZE; i++) {
        Exprn **p = exprnindex[i];
        if (p == NULL) break;
        for (j = 0; j < EXPRNSEGSIZE; j++) {
            p[j] = NULL;
        }
    }
    for (; left != NULL; left = cdr_(left)) {
        int32 newid = exnewid_(left);
        left->nodeid = mknodeid_(newid, left->nodeid & EX_ALIASandTYPE);
        exprn_(newid) = left;
    }
}
#endif

static void addkilledexprns(int32 locno, VoidStar arg)
{
    Location *loc = loc_(locno);
    VRegSetP *s = (VRegSetP *) arg;
    cseset_union(*s, loc->users);
    cseset_union(*s, loc->aliasusers);
}

void cse_scanblocks(BlockHead *top)
{   /* For each block, record the set of expressions evaluated within it.
     * Multiple occurrences of the same expression (with the same value) get
     * flattened at this stage.
     * Output:
     *   available   the set of expressions evaluated within and reaching the
     *               end of the block
     *   wanted      the set of expressions evaluated within the block and not
     *               killed within it (so could be evaluated earlier).
     *   killed      the set of LOCATIONS killed in the block
     */
    BlockHead *p;
    clock_t t0 = clock();
    csescan_setup();
    if (debugging(DEBUG_CSE | DEBUG_STORE)) {
        cc_msg("CSE available expression scan\n");
    }
    for (p = top; p != NULL; p = blkdown_(p)) {
        if (debugging(DEBUG_CSE) && CSEDebugLevel(1))
            cc_msg("L%li\n", (long)blklabname_(p));
        cse_scanblock(p);
        cseset_discard(liveexprns);
    }
    {   clock_t now = clock();
        if (debugging(DEBUG_CSE | DEBUG_STORE))
            cc_msg("%ld Exprns, %ld Locations - %d csecs\n",
                   (long)(cseidsegment-1), (long)(locationid), now-t0);
        t0 = now;
    }
#ifdef EXPERIMENT_PRUNEEXPRNS
    pruneandrenumberexprns(top);
#endif

    {   VRegSetP universe = NULL, callkills = NULL;
        int32 ul;
        {   int32 i;
            for (i = 1 ; i < cseidsegment ; i++)
                cseset_insert(i, universe, NULL);
            for (i = csealiasid+1 ; i < csealiaslimit ; i++)
                cseset_delete(i, universe, NULL);
            for (i = csenonaliasid+1 ; i < csenonaliaslimit ; i++)
                cseset_delete(i, universe, NULL);

            for (i = 0 ; i < LOCINDEXSIZE ; i++) {
                Location **index = locindex[i];
                int32 j;
                if (index == 0) break;
                for (j = 0 ; j < LOCSEGSIZE ; j++) {
                    Location *q = index[j];
                    if (q == 0) break;
                    if (ispublic(q))
                        cseset_union(callkills, q->users);
                }
            }
        }
        ul = cseset_size(universe);
        for (p = top; p != NULL; p = blkdown_(p)) {
        /* Turn the set of killed locations now recorded with each block into a
         * set of killed expressions.  We couldn't do this earlier, because the
         * full set of expressions which have a given location as leaf is available
         * only after the first pass.
         */
            VRegSetP s = NULL;
            bool calls;
            VRegSetP locs = cseset_delete(CALLLOC, blk_killed_(p), &calls);
            if (calls) s = cseset_copy(callkills);
#ifdef TARGET_ALLOWS_COMPARE_CSES
            { bool cc;
              locs = cseset_delete(CCLOC, blk_killed_(p), &cc);
              if (cc) cseset_union(s, compares);
            }
#endif
            cseset_map(locs, addkilledexprns, (VoidStar) &s);
            cseset_discard(locs);
            if (s != NULL) cseset_union(s, loadrs);
            {   int32 n = cseset_size(s);
                if (n >= ul/2) {
                    VRegSetP s1 = cseset_copy(universe);
                    cseset_difference(s1, s);
                    if (cseset_size(s1) >= n) {
                        cseset_discard(s1);
                    } else {
                        cseset_discard(s);
                        s = s1;
                        blk_killedinverted_(p) = YES;
                    }
                }
                blk_killed_(p) = s;
            }
        }
        cseset_discard(universe);
        cseset_discard(callkills);
        cseset_discard(loadrs);
#ifdef TARGET_ALLOWS_COMPARE_CSES
        cseset_discard(compares);
#endif
    }
    {   int32 i;
        for (i = 0 ; i < LOCINDEXSIZE ; i++) {
            Location **index = locindex[i];
            int32 j;
            if (index == 0) break;
            for (j = 0 ; j < LOCSEGSIZE ; j++) {
                Location *q = index[j];
                if (q == 0) break;
                cseset_discard(q->users);
                cseset_discard(q->aliasusers);
            }
        }
    }
    if (debugging(DEBUG_CSE | DEBUG_STORE)) {
        cc_msg("constructed killed(blocks) - %d csecs\n", clock()-t0);
    }
}

/* end of mip/csescan.c */
