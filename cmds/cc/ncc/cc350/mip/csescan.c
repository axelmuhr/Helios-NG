/*
 * csescan.c: CSE available expression analysis
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $ Codemist 115
 * Checkin $Date: 1993/11/23 16:41:52 $
 * Revising $Author: nickc $
 */

#ifndef NO_VERSION_STRINGS
extern char csescan_version[];
char csescan_version[] = "\ncsescan.c $Revision: 1.1 $ 115\n";
#endif

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
#include "errors.h"

typedef struct ExprnList {
    struct ExprnList *cdr;
    Exprn *exprn;
} ExprnList;

#define addtoreglist(r, l) l = (RegList *) syn_cons2(l, r)

static Location **locations; /* list of Locations used */
static Exprn **cse_tab;     /* hash table of Exprns */

#define CSEIDSEGSIZE 128     /* This must be a multiple of the VRegSet chunk size
                                (or its intended space-saving fails) */
static int32 cseidsegment;
static int32 csealiasid, csealiaslimit;
static int32 csenonaliasid, csenonaliaslimit;

static VRegSetP liveexprns,
                wantedexprns,
                killedlocations;

static VRegSetP loadrs;

#define islive(x) (vregset_member(exid_(x), liveexprns))
#define lockills(x, p) (vregset_member(x, (p)->users) || vregset_member(x, (p)->aliasusers))

static int32 locationid;

typedef struct FloatConList {
    struct FloatConList *cdr;
    FloatCon *f;
} FloatConList;

static FloatConList *floatconlist;

typedef struct RegValue {
    struct RegValue *cdr;
    VRegnum reg;
    Exprn *value;
} RegValue;

#define rv_reg_(r) ((r)->reg)
#define rv_val_(r) ((r)->value)

static RegValue *knownregs;

#ifdef ENABLE_CSE

/*
 * Debugging stuff.
 */

void cse_print_loc(Location *x)
{
    switch (loctype_(x)) {
    case LOC_VAR:
    case LOC_PVAR:
        cc_msg("'%s'", symname_(bindsym_(locbind_(x))));
        break;
    case LOC_MEM:
    case LOC_MEMB:
    case LOC_MEMW:
    case LOC_MEMD:
    case LOC_MEMF:
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
        }
        break;
    }
    cc_msg("\n");
}

void cse_printexits(int32 flags, LabelNumber *exit, LabelNumber *exit1)
{
    VRegInt gap, m;
    gap.r = GAP;
    if (flags & BLK2EXIT)
    {   m.l = exit1;
        print_jopcode(J_B + (flags & Q_MASK), gap, gap, m);
    }
    if (!(flags & BLK0EXIT))
    {   m.l = exit;
        print_jopcode(J_B, gap, gap, m);
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

#endif /* ENABLE_CSE */

#ifndef TARGET_HAS_SCALED_ADDRESSING
/* This seems an easy way of avoiding more optional compilation */
/* No -- surely the correct thing is to define something              */
/* else -- not do silly things like this.                                */
#  define J_SHIFTMASK 0L
#  define J_NEGINDEX  0L
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
    case LOC_MEMB: return 1;
    case LOC_MEMW: return 2;
    case LOC_MEMD: return 8;
    default:       return 4;
    }
}

static LocType loctype(J_OPCODE op)
{
    switch (j_memsize(op)) {
    case MEM_D: return LOC_MEMD;
    case MEM_F: return LOC_MEMF;
    case MEM_L: return LOC_MEM;
    case MEM_B: return LOC_MEMB;
    case MEM_W: return LOC_MEMW;
    default:    syserr(syserr_loctype);
                return LOC_MEM;
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
    J_OPCODE op = exop_(ex) & ~J_SHIFTMASK;
    if (ex == NULL) syserr(syserr_adconbase);
    if (op == J_ADCON || op == J_ADCONV)
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
            if ((exop_(ex) & J_SHIFTMASK) == 0) return adconbase(e2_(ex), YES);
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
       (type known to be LOC_MEMx) */

    LocType t = loctype_(loc);
    Exprn *b = locbase_(loc);
    if (publicvar_(loc)) {
        Exprn *b = adconbase(base, YES);
        return ( b == NULL ||
                 (exop_(b) == J_ADCONV && e1b_(b) == locbind_(loc)));
    }
    if (t < LOC_MEM) return NO;
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
            else if (e1b_(base1) == datasegment)
                return overlap(type, off, t, locoff_(loc)+bindaddr_(e1b_(base1)));
        }
        return NO;
#ifdef never
        if ((exop_(b) == J_ADCON || exop_(b) == J_ADCONV) &&
            (exop_(base) == J_ADCON || exop_(base) == J_ADCONV))
            return NO;
        else
            return YES;
#endif
    }
    return (t != type && overlap(type, off, t, locoff_(loc)));
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
            if (type >= LOC_MEM) {
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
    ULWRec *ulw = (ULWRec *)(int) arg;
    if (id != CALLLOC && !ulw->killed) {
        ulw->killed = lockills(ulw->expid, loc_(id));
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

static bool updateliveandwanted(int32 expid, int flags)
{
  /* Update the set of live expressions to include expid (the return value
   * indicates whether it was already in the set).
   * If the expression has not been killed, add it also to the set of wanted
   * expressions
   */
    bool old;
    cseset_insert(expid, liveexprns, &old);
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
                    if (ispublic(loc) && lockills(expid, loc))
                        return old;
                }
            }
        }
        if (!killedinblock(expid))
            cseset_insert(expid, wantedexprns, NULL);
    }
    return old;
}

static BlockHead *cse_currentblock;
static Icode *currenticode;

ExprnUse *mkExprnUse(ExprnUse *old, int flags, int valno)
{
    ExprnUse *q = Allocate(ExprnUse);
    cdr_(q) = old; q->block = cse_currentblock;
    icoden_(q) = currenticode - blkcode_(cse_currentblock);
    flags_(q) = flags;
    valno_(q) = valno;
    return q;
}

static bool samearglist(int32 n, Exprn *a[], Exprn *b[])
{
    int32 i;
    for (i = 0 ; i < n ; i++)
        if (a[i] == NULL || a[i] != b[i]) return NO;
    return YES;
}

static void useoldexprn(Exprn *p, int flags)
{
    /* if the expression is already live in this block, then local cse
     * will kill this occurrence, so I don't want to remember its position.
     */
    if (!(flags & U_NOTDEF) &&
        ( !updateliveandwanted(exid_(p), flags) ||
          (flags & U_STORE) ) ) {
        /* If it has become alive, having previously been alive, there may
         * still be some locations whose value I think it is.  (Up to now,
         * I would have known they weren't because it wasn't alive).  Their
         * values must be killed here.
         */
        {   LocList *q = exlocs_(p);
            for (; q != NULL; q = (LocList *) discard2((VoidStar) q)) {
                Location *loc = q->loc;
                if (locvalue_(loc) == p) locvalue_(loc) = NULL;
            }
            exlocs_(p) = NULL;
            {   RegValue *r, **prevp = &knownregs;
                for (r = knownregs ; r != NULL ; r = *prevp) {
                    if (rv_val_(r) == p)
                        *prevp = (RegValue *)discard3((VoidStar)r);
                    else
                        prevp = &cdr_(r);
                }
            }

        }
        exuses_(p) = mkExprnUse(exuses_(p), flags & ~U_STORE, 0);
        exwaslive_(p) = NO;
    }
}

static Exprn *find_exprn(int32 op, Exprn *a, Exprn *b, Exprn *arg[], int flags)
{
    Exprn *p, *prev;
    Exprn **list = &cse_tab[hash(op, a, b)];
    int32 type = cse_optype(op);
    switch (type) {
    case E_BINARY:
        if (b == NULL) return NULL;
    case E_BINARYK:
        if (a == NULL) return NULL;
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
        if (a == NULL) return NULL;
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
        {   int32 n = regname_((int32)b);
            int32 i;
            for (i = 0 ; i < n ; i++)
                if (arg[i] == NULL) return NULL;
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
            e2_(p)   = b;
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
        exuses_(p) = (flags & U_NOTDEF) ? NULL : mkExprnUse(NULL, flags, 0);
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
        p->nodeid = (id << 5) | alias | type;  /* rewrite with the correct id */
        updateusers(id, p);
        if (!(flags & U_NOTDEF)) updateliveandwanted(exid_(p), flags);

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

/*
 * Stuff about reading/writing/corrupting Locations.
 */

static Location *find_loc(LocType type, Exprn *base, int32 k, J_OPCODE load, bool realbase)
{
    int32 id1 = k;
    Location *p, *prev;
    Location **list = &locations[lochash(type, id1, (int32)base)];
    for (prev = NULL, p = *list; p != NULL; prev = p, p = cdr_(p)) {
        if (loctype_(p) != type  &&
             !( loctype_(p) <= LOC_PVAR && type <= LOC_PVAR))
            continue;
        if (locoff_(p) != id1) continue;
        if (type < LOC_MEM || locbase_(p) == base) {
            if (prev != NULL) {
                cdr_(prev) = cdr_(p); cdr_(p) = *list; *list = p;
            }
            return p;
        }
    }
    {   int32 size = type == LOC_VAR ? SizeOfLocVar : sizeof(Location);
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
    p->idandtype = ((locationid++)<<4) | type | (realbase ? LOC_REALBASE : 0);
    if (type >= LOC_MEM) {
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

    p->load = find_exprn(load, (Exprn *) p, 0, NULL, U_NOTDEF+U_NOTREF);
    return (*list = p);
}

static int32 _J_LDRK_w[] =
{
    J_LDRBK, J_LDRWK, J_LDRK, J_LDRFK, J_LDRDK
};

#define j_to_ldrk(a) _J_LDRK_w[j_memsize(a)]

static int32 _J_STRK_w[] =
{
    J_STRBK, J_STRWK, J_STRK, J_STRFK, J_STRDK
};

#define j_to_strk(a) _J_STRK_w[j_memsize(a)]

static bool is_narrow(Location *loc)
{
    int32 type = loctype_(loc);
    return (type == LOC_MEMB || type == LOC_MEMW);
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

static bool already_narrowed(Exprn *e, J_OPCODE op)
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

static Exprn *loc_read(Location *p, int exprnflags, J_OPCODE op)
{
    /* op is to tell us about sign/zero-extension if the location is
       a narrow one. */
    Exprn *e = locvalue_(p);
    if (e == NULL || !islive(e) || !islive(p->load)) {
        locvalue_(p) = NULL;
        /* for the latter two cases.  Otherwise, if e subsequently becomes
           live we are in trouble
         */
        if (is_flavoured(op, p)) {
            e = flavoured_load(op, p, exprnflags);
        } else {
            e = p->load;
            if ( !islive(e) &&
                 is_narrow(p) &&
                 ( (op & (J_UNSIGNED | J_SIGNED)) == 0)) {
                /* If we have a plain load of a narrow location, the value
                   from a previous flavoured load will do.  (This works only
                   locally, of course - a way to do it non-locally would
                   supersede this code
                 */
                Exprn *e1 = flavoured_load(op | J_SIGNED, p,
                                           U_NOTREF+U_NOTDEF+U_PEEK);
                if (e1 != NULL && islive(e1))
                    e = e1;
                else {
                    e1 = flavoured_load(op | J_UNSIGNED, p,
                                        U_NOTREF+U_NOTDEF+U_PEEK);
                    if (e1 != NULL && islive(e1))
                        e = e1;
                }
            }
            useoldexprn(e, exprnflags);
            /* I don't think this is particularly useful */
            /*locvalue_(p) = e;*/
        }
    } else if (is_flavoured(op, p)) {
        /* if e is already a correctly extended value for the flavour of
           load wanted, then we should use it.
         */
        if (!already_narrowed(e, op))
            e = flavoured_load(op, p, exprnflags);
    }
    return e;
}

static void setlocvalue(Location *p, Exprn *val)
{
    locvalue_(p) = val;
    if (val != NULL) exlocs_(val) = mkLocList(exlocs_(val), p);
    cseset_difference(liveexprns, p->users);
    cseset_difference(liveexprns, p->aliasusers);
    cseset_insert(locid_(p), killedlocations, NULL);
    /* Now for any locations whose known values depended on p, we don't know
     * their values anymore.  It's horribly expensive to go through them here,
     * though - instead, we check for liveness when we look at the value.  In
     * addition, we need to ensure that the value doesn't become live again in
     * between - so every time an expression becomes live, we destroy the value
     * of all locations with that value.
     */
}

static RegValue *newregvalue(RegValue *next, VRegnum reg)
{
    return (RegValue *) CSEList3((int32)next, reg, NULL);
}

static void setreg(VRegnum reg, Exprn *val, bool newvalue)
{
    RegValue **prevp = &knownregs, *r;
    Exprn *ex = find_exprn(CSE_LOADR, (Exprn *) reg, 0, NULL, U_NOTDEF+U_NOTREF+U_PEEK);
    if (ex != NULL) cseset_delete(exid_(ex), liveexprns, NULL);
    for (r = knownregs ; r != NULL ; prevp = &cdr_(r), r = cdr_(r)) {
        if (rv_reg_(r) == reg) {
            if (newvalue) {
                RegValue *p = r;
                for (; p != NULL && rv_reg_(p) == reg; p = cdr_(r)) {
                    if (p != r) {
                        cdr_(r) = (RegValue *) discard3((VoidStar) p);
                    }
                }
            } else if (val != NULL) {
                cdr_(r) = newregvalue(cdr_(r), reg);
                r = cdr_(r);
            }
            if (val == NULL) {
                while (r != NULL && rv_reg_(r) == reg) {
                    r = (RegValue *) discard3((VoidStar) r);
                    *prevp = r;
                }
                return;
            }
            break;
        }
    }
    if (r == NULL ) {
        if (val == NULL) return;
        knownregs = newregvalue(knownregs, reg);
        r = knownregs;
    }
    rv_val_(r) = val;
    if (val != NULL) {
        exwaslive_(val) = YES;
        if (exop_(val) == J_RESULT2) exwaslive_(e1_(val)) = YES;
    }
}

static Exprn *valueinreg(VRegnum reg)
{  /* Returns the value held in the argument register if it is known,
    * otherwise NULL.
    */
    RegValue **prevp = &knownregs, *r;
    for (r = knownregs ; r != NULL ; prevp = &cdr_(r), r = cdr_(r)) {
        if (rv_reg_(r) == reg) {
            do {
                if (!islive(rv_val_(r))) {
                    r = (RegValue *) discard3((VoidStar) r);
                    *prevp = r;
               } else
                   return rv_val_(r);
            } while (r != NULL && rv_reg_(r) == reg);
            return NULL;
        }
    }
    return NULL;
}

static Exprn *readreg(VRegnum reg)
{  /* Returns the value held in the argument register if it is known,
    * otherwise an Exprn describing the register (used only for base/
    * index registers of store accesses).
    */
    Exprn *res = valueinreg(reg);
    if (res != NULL) return res;
    res = find_exprn(CSE_LOADR, (Exprn *) reg, 0, NULL, U_NOTDEF+U_NOTREF);
    cseset_insert(exid_(res), loadrs, NULL);
    cseset_insert(exid_(res), liveexprns, NULL);
    setreg(reg, res, YES);
    return res;
}

static void setmem(Location *p, Exprn *val)
{
    setlocvalue(p, val);
}

static void setvar(Location *p, Exprn *val)
{
    int32 i;
    setlocvalue(p, val);
    /* Now if p may be an alias for anything, we must discard
       the known value of that thing. */
    if (locpublic_(p)) {
        for (i = 0 ; i < LOCINDEXSIZE ; i++) {
            Location **index = locindex[i];
            int32 j;
            if (index == 0) break;
            for (j = 0 ; j < LOCSEGSIZE ; j++) {
                Location *q = index[j];
                if (q == 0) break;
                if (loctype_(q) >= LOC_MEM &&
                    possiblealias(p, loctype_(q), locbase_(q), locoff_(q)) )
                    setlocvalue(q, NULL);
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
            if (ispublic(q)) setlocvalue(q, NULL); /* value of p may be changed */
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
    for (i = 0 ; i < LOCINDEXSIZE ; i++) {
        Location **index = locindex[i];
        int32 j;
        if (index == 0) break;
        for (j = 0 ; j < LOCSEGSIZE ; j++) {
            Location *q = index[j];
            if (q == 0) break;
            locvalue_(q) = NULL;
        }
    }
    for (i = 0 ; i != HASHSIZE ; i++) {
        Exprn *p = cse_tab[i];
        for ( ; p != NULL ; p = cdr_(p)) {
            exwaslive_(p) = NO;
            while (exlocs_(p) != NULL)
                exlocs_(p) = (LocList *) discard2((VoidStar) exlocs_(p));
        }
    }
    liveexprns = NULL;
    wantedexprns = NULL;
    killedlocations = NULL;
}

static void cse_corrupt_register(VRegnum r)
{
    setreg(r, NULL, YES);
}

static int32 shiftedval(J_OPCODE op, int32 b)
{
#ifdef TARGET_HAS_SCALED_ADDRESSING
    if (op & J_SHIFTMASK) {
        int32 msh = (op & J_SHIFTMASK) >> J_SHIFTPOS;
        if ((msh & SHIFT_RIGHT) == 0)
            b = b << (msh & SHIFT_MASK);
        else if (msh & SHIFT_ARITH)
            b = b >> (msh & SHIFT_MASK);
        else
            b = (int32) (((unsigned32) b) >> (msh & SHIFT_MASK));
    }
#else
    IGNORE(op);
#endif
    return b;
}

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

static void removecomparison(int32 a, int32 b, int32 op, BlockHead *block)
{
    bool taken;
    switch (op & Q_MASK) {
    case Q_EQ | Q_UBIT:
    case Q_EQ:              taken = (a == b); break;
    case Q_NE | Q_UBIT:
    case Q_NE:              taken = (a != b); break;
    case Q_HS | Q_UBIT:     taken = ((unsigned32)a >= (unsigned32)b); break;
    case Q_LO | Q_UBIT:     taken = ((unsigned32)a < (unsigned32)b); break;
    case Q_HI | Q_UBIT:     taken = ((unsigned32)a > (unsigned32)b); break;
    case Q_LS | Q_UBIT:     taken = ((unsigned32)a <= (unsigned32)b); break;
    case Q_GE:              taken = (a >= b); break;
    case Q_LT:              taken = (a < b); break;
    case Q_GT:              taken = (a > b); break;
    case Q_LE:              taken = (a <= b); break;
    case Q_AL:              taken = YES; break;
    case Q_NOT:             taken = NO; break;
    default:    syserr(syserr_removecomparison, (long)op);
    case Q_XXX:             return;
    }
    if (taken) blknext_(block) = blknext1_(block);
    blkflags_(block) &= ~(Q_MASK | BLK2EXIT);
    currenticode->op = J_NOOP;
    if (debugging(DEBUG_CSE))
        cc_msg("Comparison removed: %sbranch always taken\n",
               taken ? "" : "other ");
}

static Exprn *newexprnpart(int32 part, Exprn *e)
{
    Exprn *e1 = find_exprn(part, e, NULL, NULL, U_NOTREF+U_NOTDEF);
    if (islive(e)) cseset_insert(exid_(e1), liveexprns, NULL);
    return e1;
}

static bool evalconst(J_OPCODE op, int32 *resp, Exprn *ax, int32 b)
{
    int32 res, a;
    if (exop_(ax) != J_MOVK) return NO;
    a = e1k_(ax);
    switch (op & ~(J_SIGNED | J_UNSIGNED | J_SHIFTMASK)) {
    case J_ADDK: res = a + b; break;
    case J_MULK: res = a * b; break;
    case J_ANDK: res = a & b; break;
    case J_ORRK: res = a | b; break;
    case J_EORK: res = a ^ b; break;
    case J_SUBK: res = a - b; break;
    case J_DIVK: if (b == 0) { cc_warn(sem_warn_divrem_0, s_div); return NO; }
                 res = (op & J_UNSIGNED) ? (int32) (((unsigned32) a) / b) :
                                           a / b;
                 break;
    case J_RSBK: res = b - a; break;
    case J_REMK: if (b == 0) { cc_warn(sem_warn_divrem_0, s_rem); return NO; }
                 res = (op & J_UNSIGNED) ? (int32) (((unsigned32) a) % b) :
                                           a % b;
                 break;
    case J_SHLK: res = a << b; break; /* @@@ not if TARGET_LACKS_RIGHTSHIFT? */
    case J_SHRK: res = (op & J_UNSIGNED) ? (int32) (((unsigned32) a) >> b) :
                                           TARGET_RIGHTSHIFT(a, b);
                 break;
#ifdef TARGET_IS_HELIOS
      /*
       * XXX - NC - 19/6/92
       *
       * The following case has been added to catch the following code.
       * It is not Helios specific, but I could not think of a better
       * ifdef to use ...
       *
       * struct s { short shrt; } * ptr;
       * void func( void ) { int a; a = ptr->shrt = 0; }
       */

    case J_EXTEND:      
      return NO;
#endif
    default: syserr(syserr_evalconst, (long)op);
             return NO;
    }
    *resp = res;
    if (debugging(DEBUG_CSE) && var_cse_enabled > 16)
        cc_msg("Compile-time evaluable = %ld\n", res);
    return YES;
}

static J_OPCODE evaluablefn(Expr *fn, VRegnum res)
{
#ifndef TARGET_HAS_DIVIDE
    if (fn == arg1_(sim.divfn))
        return res == R_P1 ? J_DIVK | J_SIGNED : J_REMK | J_SIGNED;
    if (fn == arg1_(sim.udivfn))
        return res == R_P1 ? J_DIVK | J_UNSIGNED : J_REMK | J_UNSIGNED;
#endif
#if defined(TARGET_LACKS_REMAINDER) || !defined(TARGET_HAS_DIVIDE)
    if (fn == arg1_(sim.remfn)) return J_REMK | J_SIGNED;
    if (fn == arg1_(sim.uremfn)) return J_REMK | J_UNSIGNED;
#endif
#ifdef TARGET_HAS_MULTIPLY
    if (fn == arg1_(sim.mulfn)) return J_MULK;
#endif
    return 0;
}


static bool evaluablecall(Expr *fn, VRegnum res, int32 nargs, Exprn *arg[], int32 *resp)
{
    J_OPCODE op = evaluablefn(fn, res);
    if (op == 0) return NO;
    /* All compile-time evaluable fns are binary currently.
       Software FP?
     */
    if (nargs == 2 && arg[0] != NULL && arg[1] != NULL) {
        /* Beware arguments for divide & remainder functions reversed
           from the obvious order.
         */
        if (exop_(arg[1]) == J_MOVK && exop_(arg[0]) == J_MOVK)
            return evalconst(op, resp, arg[1], e1k_(arg[0]));
    }
    return NO;
}

typedef struct StoreAccessList {
    struct StoreAccessList *cdr;
    Location *loc;
    Icode *ic;
} StoreAccessList;

static StoreAccessList *storeaccesses;

static void add_store_access(Location *x) {
    StoreAccessList *p = Allocate(StoreAccessList);
    cdr_(p) = storeaccesses;
    p->loc = x; p->ic = currenticode;
    storeaccesses = p;
}

static void killoldunusedstore(Location *loc)
{ /* If the last reference to loc was in a store instruction, discard it.
     We can ignore stores to something which may be an alias of loc,
     but not loads from a possible alias.
   */
    StoreAccessList *a = storeaccesses;
    LocType type = loctype_(loc);
    for (; a != NULL; a = cdr_(a)) {
        if (loc == a->loc) {
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
        } else if (!stores_r1(a->ic->op) && type != LOC_VAR) {
            LocType typea = loctype_(a->loc);
           /* asymmetry of possiblealias is annoying here */
            if (typea == LOC_PVAR) {
                if ( type != LOC_PVAR &&
                     possiblealias(a->loc, type, locbase_(loc), locoff_(loc)))
                    return;
            } else if (typea != LOC_VAR)
                if (possiblealias(loc, typea, locbase_(a->loc), locoff_(a->loc)))
                    return;
        }
    }
}
static void storein(VRegnum r1, Location *loc, Exprn *val)
{
    Exprn *locload = loc->load;
    bool waslive = cseset_member(exid_(locload), liveexprns);
    Exprn *node = loc_read(loc, U_NOTREF, exop_(locload));
    if (node == val) {
        if (debugging(DEBUG_CSE)) {
            cc_msg("-- redundant "); jopprint_opname(currenticode->op);
            cc_msg("\n");
        }
        currenticode->op = J_NOOP;
    } else {
        if (waslive) {
        /* There is a load or store of this location within this block which
         * is still valid.  If a store, we may be able to eradicate it (if
         * there is no intervening load from a possible alias).
         */
            killoldunusedstore(loc);
        }
        if (loctype_(loc) <= LOC_PVAR)
            setvar(loc, val);
        else
            setmem(loc, val);
        add_store_access(loc);
        useoldexprn(locload, U_NOTREF+U_STORE);
        exwaslive_(locload) = YES;
        setreg(r1, locload, NO);
    }
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
     if (loc == NULL) syserr("csescan(USE/VSTORE)");
     set = cseset_delete(exid_(loc->load), set, NULL);
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
#if defined TARGET_LDRFK_MAX || defined TARGET_LDRK_QUANTUM
    int32 type = j_memsize(op);
#endif
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

#define MOVK_flags(r1) (isint_realreg_(r1) ? U_NOTREF : 0)

static void cse_scanblock(BlockHead *block)
{
#define find_adconv(m) \
    find_exprn(J_ADCONV, (Exprn *) m, 0, NULL, U_NOTDEF+U_NOTREF)
/* Note here that if scaled addressing is not supported the SUBK option */
/* here will never get activated. The apparent test on the NEGINDEX bit */
/* is redundant but harmless.                                           */
#define find_addrr(op, e1, e2) \
    find_exprn(((op & J_NEGINDEX) ? J_SUBR : J_ADDR) + (op&J_SHIFTMASK), \
               e1, e2, NULL, U_NOTREF+U_NOTDEF)
#define estack_push(e) \
    estack = (ExprnList *) syn_cons2(estack, e)

    Icode *c, *limit;
    ExprnList *estack = NULL;
    Location *loc = NULL;
    int32 flags = blkflags_(block);        /* remember these now, because they may */
    LabelNumber *exit = blknext_(block);   /* get altered (by value propagation to */
#ifdef ENABLE_CSE
    LabelNumber *exit1 = blknext1_(block); /* a compare) before we reach the end.  */
#endif
    storeaccesses = NULL;
    cse_currentblock = block;
    blocksetup();
    for (c = blkcode_(block), limit = c + blklength_(block); c < limit; ++c) {
        Exprn *e1, *e2, *node, *val;
        int32 op = c->op;
        VRegInt r1 = c->r1, r2 = c->r2, m = c->m;
        bool isload = NO;
        currenticode = c;

	
        if (debugging(DEBUG_CSE) && var_cse_enabled > 16)
            print_jopcode(op, r1, r2, m);
        e1 = NULL; e2 = NULL; node = NULL;
        /*
         * Convert all instructions to nodes of the DAG.
         * Do copy propagation on the fly.
         */
        if (reads_r1(op)) e1 = valueinreg(r1.r);
        if (reads_r2(op)) e1 = valueinreg(r2.r);
        if (reads_r3(op))
            e2 = valueinreg(m.r);
        else
            e2 = (Exprn *)m.b;

        switch (op & ~(Q_MASK | J_SIGNED | J_NEGINDEX | J_SHIFTMASK)) {
        case J_CALLK:
#ifdef TARGET_HAS_DIVREM_FUNCTION
            if ((r2.i & K_PURE) &&              /* check not needed?    */
                  ((Expr *)m.sym == arg1_(sim.divfn) ||
                   (Expr *)m.sym == arg1_(sim.udivfn)))
                c->op = op = J_CALL2KP, c->r2.i = r2.i &= K_ARGWORDMASK;
        case J_CALL2KP:
#endif
        case J_CALLR:
        {   int32 i, argwords = r2.i & K_ARGWORDMASK;
            if ((op == J_CALLK && (r2.i & K_PURE) || op == J_CALL2KP) &&
                (argwords <= NARGREGS))
            {   Exprn *arg[NARGREGS==0 ? 1 : NARGREGS];
                int32 val;
                for (i = 0 ; i < argwords; i++)
                    arg[i] = valueinreg(virtreg(R_P1+i, INTREG));
                if (evaluablecall((Expr *)m.sym, r1.r, argwords, arg, &val))
                {
		  c = trytokillargs(c, blkcode_(block), (c+1) < limit);
                    currenticode = c;
                    op = c->op = J_MOVK;
                    r1 = c->r1;
                    c->r2.r = GAP;
                    c->m.i  = val;
                    node = find_exprn(J_MOVK, (Exprn *)val, 0, NULL,
                                      MOVK_flags(r1.r));
                    break;
                }
                if (op == J_CALL2KP && regname_(r1.r) != R_A1) {
                    Exprn *div = find_exprn(op, (Exprn *) m.sym,
/* The next line looks as if it is WRONG wrt any type abstraction on VRegnums */
                                (Exprn *)(regtype_(r1.r) + argwords), arg, 0);
                    if (div != NULL) {
                    /* care needs to be exercised here - the call of find_exprn
                       above will have made div live, so its use in the RESULT2
                       exprn below will always set exwaslive_.
                     */
                        bool live = exwaslive_(div);
                        node = find_exprn(J_RESULT2, div, NULL, NULL,
                                          U_NOTREF+U_NOTDEF);
                        if (!live) {
                            exwaslive_(div) = NO;
                            exwaslive_(node) = NO;
                        }
                        if (&useicode_(exuses_(div)) == c)
                            valno_(exuses_(div)) = 1;
                    }
                } else
                    node = find_exprn(op, (Exprn *) m.sym,
/* The next line looks as if it is WRONG wrt any type abstraction on VRegnums */
                                (Exprn *)(regtype_(r1.r) + argwords), arg, 0);
            } else
                corruptmem();
/* @@@ This set of things amounts to  'corrupt all non-callee-save registers'.
   Is there a better way to express this now? */
/* @@@ REVIEW/bugfix in the light of ACN code to preserve such regs in */
/* regalloc if regmaskvec for callee allows?                              */
            for (i=0; i<NARGREGS; i++)
                cse_corrupt_register(virtreg(R_A1+i, INTREG));
#ifdef TARGET_STRUCT_RESULT_REGISTER
            if (r2.i & K_STRUCTRET)
                cse_corrupt_register(
                    virtreg(TARGET_STRUCT_RESULT_REGISTER, INTREG));
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
            break;
        case J_ADCON:
        case J_ADCOND: case J_ADCONF:
        case J_FNCON:
        case J_ADCONV:
            node = find_exprn(op, (Exprn *) m.b, 0, NULL, 0);
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
        case J_POP:
            {   RegList *p = (RegList *)dreverse((List *)m.rl);
                RegList *pcopy = p;
                for (; p != NULL; p = p->rlcdr) {
                    Exprn *e;
                    if (estack == NULL)
                        e = NULL;
                    else {
                        e = estack->exprn;
                        estack = (ExprnList *) discard2((VoidStar) estack);
                        if (e != NULL && !islive(e)) e = NULL;
                    }
                    setreg(p->rlcar, e, YES);
                }
/*
 * Note from ACN: the use of dreverse() here to restore the side-effects
 * that the first call of it made feels delicate - I would find things easier
 * to read (maybe?) if dreverse was always used for its value and the list
 * that was input to it was deemed to have been totally destroyed.
 * Thus I would vote for
 *    c->m = (int32)dreverse((Lisp *)pcopy);
 * here even though it is not really necessary.   ?? perhaps ??
 */
                (void)dreverse((List *)pcopy);
            }
            break;
        case J_LDRVK: case J_LDRFVK: case J_LDRDVK:
        case J_LDRBVK: case J_LDRWVK:
            {   Exprn *base = find_adconv(m.b);
                loc = find_loc(loctype(op), base, r2.r, j_to_ldrk(op), NO);
                node = loc_read(loc, 0, op);
            }
            isload = YES;
            break;
        case J_STRVK: case J_STRFVK: case J_STRDVK:
        case J_STRBVK:
#ifndef TARGET_LACKS_HALFWORD_STORE
        case J_STRWVK:
#endif
            {   Exprn *base = find_adconv(m.b);
                val = valueinreg(r1.r);
                loc = find_loc(loctype(op), base, r2.r, j_to_ldrk(op), NO);
                storein(r1.r, loc, val);
            }
            break;
        case J_LDRR: case J_LDRFR: case J_LDRDR:
        case J_LDRBR: case J_LDRWR:
            {   Exprn *base;
                e1 = readreg(r2.r);
                e2 = readreg(m.r);
                if (exop_(e2) == J_MOVK) {
                    int32 a2 = shiftedval(op, e1k_(e2));
#ifdef TARGET_LDRK_MAX
                    if (validdisplacement(op, a2))
#endif
		      {
                        c->op = op = j_to_ldrk(op) | (op & (J_SIGNED | J_UNSIGNED));
                        c->m.i = m.i = a2;
                        goto transformedtoldrk;
                    }
                }
                base = find_addrr(op, e1, e2);
                loc = find_loc(loctype(op), base, 0, j_to_ldrk(op), NO);
                node = loc_read(loc, 0, op);
            }
            isload = YES;
            break;
        case J_LDRK: case J_LDRFK: case J_LDRDK:
        case J_LDRBK: case J_LDRWK:
            e1 = readreg(r2.r);
transformedtoldrk:
            loc = find_loc(loctype(op), e1, m.i, j_to_ldrk(op), YES);
            node = loc_read(loc, 0, op);
            isload = YES;
            break;
        case J_STRR: case J_STRFR: case J_STRDR:
        case J_STRBR:
#ifndef TARGET_LACKS_HALFWORD_STORE
        case J_STRWR:
#endif
            {   Exprn *base;
                e1 = readreg(r2.r);
                e2 = readreg(m.r);
                val = valueinreg(r1.r);
                if (exop_(e2) == J_MOVK) {
                    int32 a2 = shiftedval(op, e1k_(e2));
#ifdef TARGET_LDRK_MAX
                    if (validdisplacement(op, a2))
#endif
		      {
			
                        c->op = op = j_to_strk(op);
                        c->m.i = m.i = a2;
                        goto transformedtostrk;
                    }
                }
                base = find_addrr(op, e1, e2);
                loc = find_loc(loctype(op), base, 0, j_to_ldrk(op), NO);
                storein(r1.r, loc, val);
            }
            break;
        case J_STRK: case J_STRFK: case J_STRDK:
        case J_STRBK:
#ifndef TARGET_LACKS_HALFWORD_STORE
        case J_STRWK:
#endif
            val = valueinreg(r1.r);
            e1 = readreg(r2.r);
transformedtostrk:
            loc = find_loc(loctype(op), e1, m.i, j_to_ldrk(op), YES);
            storein(r1.r, loc, val);
            break;
        case J_LDRV1: case J_LDRFV1: case J_LDRDV1:
            op = op + (J_LDRV - J_LDRV1);
        case J_LDRV: case J_LDRFV: case J_LDRDV:
            loc = find_loc(LOC_VAR, NULL, m.i, j_to_ldrk(op), NO);
            node = loc_read(loc, 0, op);
            isload = YES;
            break;
        case J_STRV: case J_STRFV: case J_STRDV:
            loc = find_loc(LOC_VAR, NULL, m.i, j_to_ldrk(op), NO);
            storein(r1.r, loc, e1);
            break;
        case J_MOVR: case J_MOVFR: case J_MOVDR:
            if (e2 != NULL) {
                node = e2;
                if (!cseset_member(exid_(e2), liveexprns))
                    exuses_(e2) = mkExprnUse(exuses_(e2), 0, 0);
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
            if (e1 == NULL || exop_(e1) != J_ADCON || exop_(e1) != J_ADCONV)
                corruptmem();
            else {
                int32 i = 0, span = 4; J_OPCODE ldop = J_LDRK;
#if alignof_struct < 4
                span = 1, ldop = J_LDRBK;
#endif
                for (; i < m.i; i += span) {
                    Location *loc = find_loc(LOC_MEM, e1, i, ldop, YES);
                    setlocvalue(loc, NULL);
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
        case J_CMPFK: case J_CMPDK:
        case J_INFOLINE: case J_INFOSCOPE: case J_INFOBODY:
        case J_COUNT:
        case J_WORD:
        case J_CASEBRANCH:
            node = NULL;
            break;

        case J_CMPFR: case J_CMPDR:
            if (e1 != NULL && e2 != NULL && e1 == e2)
                removecomparison(0, 0, op, block);
            break;

#ifdef RANGECHECK_SUPPORTED
        case J_CHKLR: case J_CHKUR:
            if (e2 == NULL || exop_(e2) != J_MOVK) {
                node = (e2 == NULL) ? NULL : find_exprn(op, e1, e2, NULL, 0);
                break;
            }
            m.i = shiftedval(op, e1k_(e2));
        case J_CHKLK: case J_CHKUK:
            if (e1 != NULL && exop_(e1) == J_MOVK) {
                int32 index = e1k_(e1);
                int32 realop = op & ~(Q_MASK | J_SIGNED | J_NEGINDEX | J_SHIFTMASK);
                if ((realop == J_CHKLK || realop == J_CHKLR) ? (index >= m.i) :
                                                               (index <= m.i)) {
                    c->op = J_NOOP;
                    node = NULL;
                    break;
                }
            }
            node = (e1 == NULL || e2 == NULL) ? NULL : find_exprn(op, e1, e2, NULL, 0);
            break;
        case J_CHKNEK:
            if (e1 != NULL && exop_(e1) == J_MOVK) {
                if (e1k_(e1) != m.i) {
                    c->op = J_NOOP;
                    node = NULL;
                    break;
                }
            }
            node = (e1 == NULL) ? NULL : find_exprn(op, e1, e2, NULL, 0);
            break;
        case J_CHKNEFR:
        case J_CHKNEDR:
            break;
#endif
        case J_CMPR:
            if (e1 != NULL && e2 != NULL) {
                if (exop_(e2) == J_MOVK) {
                    int32 a2 = shiftedval(op, e1k_(e2));
                    if (exop_(e1) == J_MOVK && exop_(e2) == J_MOVK)
                        removecomparison(e1k_(e1), shiftedval(op, e1k_(e2)),
                                         op, block);
                    else {
                        c->op = J_RTOK(op) & ~J_SHIFTMASK;
                        c->m.i = a2;
                    }
                } else if (e1 == e2)
                    removecomparison(0, 0, op, block);

#ifdef ONE_FINE_DAY
/*@@@ something along the following lines may well be a good idea, but I have
      to be sure that the constant was actually written, not inferred from the
      known value of some variable - otherwise, the possibility of overflow
      prohibits this.
 */
                else if ((op & J_SHIFTMASK) == 0) {
                    int32 n1 = 0, n2 = 0;
                    if (exprisexprplusk(e1, e2, &n1, &n2))
                        removecomparison(n1, n2, op, block);
                }
#endif
            }
            break;
        case J_CMPK:
            if (e1 != NULL && exop_(e1) == J_MOVK)
                removecomparison(e1k_(e1), m.i, op, block);
            else if (!immed_cmp(m.i)) {
                op = J_MOVK;
                node = find_exprn(op, e2, 0, NULL, 0);
            }
            break;

        case J_USE: case J_USEF: case J_USED:
        case J_VSTORE:
            /* I think I have to assume that this directly follows a load
             * or store.  I can't get from the register to the store location
             * in the case of stores, if the value being stored was known.
             */
            /* Note that this leads to a curiosity on the ARM for:
             *   volatile short a; f() { a=1,a=2; }
             */
            wantedexprns = deleteloads(loc, wantedexprns);
            liveexprns = deleteloads(loc, liveexprns);
            node = NULL;
            break;

        case J_MOVFR1: case J_MOVDR1:
            cse_corrupt_register(r1.r);
            node = NULL;
            break;

        case J_MOVK:
            node = find_exprn(op, e2, 0, NULL, MOVK_flags(r1.r));
            break;

        case J_MOVFK: case J_MOVDK:
            e2 = (Exprn *) canonicalfpconst(m.f);
        case J_STRING:
        case J_NEGR: case J_NEGFR: case J_NEGDR:
        case J_NOTR:
        case J_FLTFR: case J_FLTDR:
        case J_FIXFR: case J_FIXDR:
        case J_MOVDFR: case J_MOVFDR:
            if (e2 != NULL)
                node = find_exprn(op, e2, 0, NULL, 0);
            break;

        case J_ADDFK: case J_ADDDK:
        case J_MULFK: case J_MULDK:
        case J_SUBFK: case J_SUBDK:
        case J_DIVFK: case J_DIVDK:
            e2 = (Exprn *) canonicalfpconst(m.f);

        case J_ADDFR: case J_ADDDR:
        case J_MULFR: case J_MULDR:
        case J_SUBFR: case J_SUBDR:
        case J_DIVFR: case J_DIVDR:
        case J_INLINE1: case J_INLINE1F: case J_INLINE1D:
            if (e1 != NULL && e2 != NULL)
                node = find_exprn(op, e1, e2, NULL, 0);
            break;

        case J_ADDR: case J_MULR: case J_ANDR: case J_ORRR: case J_EORR:
        case J_SUBR: case J_DIVR: case J_RSBR: case J_REMR:
        case J_SHLR: case J_SHRR:
            if (e1 != NULL && e2 != NULL) {
                int flags = 0;
                if (exop_(e2) == J_MOVK
#ifdef TARGET_LACKS_MULTIPLY_LITERALS
		    && (op & ~(Q_MASK | J_SIGNED | J_NEGINDEX | J_SHIFTMASK)) != J_MULR
#endif
#ifdef TARGET_LACKS_DIVIDE_LITERALS
		    && (op & ~(Q_MASK | J_SIGNED | J_NEGINDEX | J_SHIFTMASK)) != J_DIVR
		    && (op & ~(Q_MASK | J_SIGNED | J_NEGINDEX | J_SHIFTMASK)) != J_REMR
#endif
		    ) {
                    int32 a2 = shiftedval(op, e1k_(e2));
                    if (evalconst(J_RTOK(op), &c->m.i, e1, a2)) {
                        op = c->op = J_MOVK;
                        c->r2.r = GAP;
                        e1 = (Exprn *)c->m.i; e2 = 0;
                        flags = MOVK_flags(c->r1.r);
                    } else {
                        op = c->op = J_RTOK(op) & ~J_SHIFTMASK;
                        e2 = (Exprn *)(c->m.i = a2);
                    }
                }
                node = find_exprn(op, e1, e2, NULL, flags);
            }
            break;
        case J_ADDK: case J_MULK: case J_ANDK: case J_ORRK: case J_EORK:
        case J_SUBK: case J_DIVK: case J_RSBK: case J_REMK:
        case J_SHLK: case J_SHRK: case J_EXTEND:
            if (e1 != NULL) {
                if ( exop_(e1) == J_MOVK &&
                     evalconst(op, &c->m.i, e1, m.i) ) {
                    c->op = J_MOVK;
                    c->r2.r = GAP;
                    node = find_exprn(J_MOVK, (Exprn *)c->m.i, 0, NULL,
                                      MOVK_flags(r1.r));
                } else
                    node = find_exprn(op, e1, e2, NULL, 0);
            }
            break;

        default:
#ifdef ENABLE_CSE
            jopprint_opname(op);        /* helps me find the error! */
#endif
            syserr(syserr_scanblock, (long)op);
            node = NULL;
            break;

        } /* switch */
#ifdef RANGECHECK_SUPPORTED
        if (j_is_check(op) && node != NULL) {
        /* Apparently, a test for exwaslive_(node) here is wrong.  Why? */
            if (&useicode_(exuses_(node)) != c) {
                c->op = J_NOOP;
                if (debugging(DEBUG_CSE) && var_cse_enabled > 16)
                    cc_msg("-- local CSE reference [%ld] -> NOOP\n", exid_(node));
            }
        } else
#endif
        if (loads_r1(op)) {
            if ( node != NULL &&
                 !register_movep(op) &&
                 ( ( isload &&
                     ( extype_(node) != E_LOAD || exloc_(node) != loc) ) ||
                   /* That is, node is a propagated value */
                   ( ( exwaslive_(node) ||
                       ( exop_(node) == J_RESULT2 &&
                         exwaslive_(e1_(node)) )) &&

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
                     ( !isint_realreg_(r1.r) ||
                       ( op != J_MOVK && op != J_ADCON && op != J_ADCONV)) )))
            {
                if (exop_(node) != CSE_WORD1 && exop_(node) != CSE_WORD2)
                {   if (exop_(node) == J_RESULT2)
                        isload &= !addlocalcse(e1_(node), 1, block);
                    else
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
                add_store_access(loc);
            setreg(r1.r, node, YES);
        }
    }
    while (estack != NULL) estack = (ExprnList *)discard2((VoidStar)estack);
    if (!(flags & BLK2EXIT) && is_exit_label(exit)) {
      /* We may kill all unused stores to local objects.  regalloc will do
         better with non-address-taken binders, but not with address-taken
         ones or local structs.
       */
        StoreAccessList *a;
        for (a = storeaccesses; a != NULL; a = cdr_(a)) {
            Icode *ic = a->ic;
            if (stores_r1(ic->op)) {
                Location *loc = a->loc;
                if (islive(loc->load) &&
                    ( ( (loctype_(loc) == LOC_VAR ||
                         loctype_(loc) == LOC_PVAR) &&
                        (bindstg_(locbind_(loc)) &
                         (bitofstg_(s_auto) | b_globalregvar))
                            == bitofstg_(s_auto)) ||
                      (exop_(locbase_(loc)) == J_ADCONV))) {
                    killoldunusedstore(loc);
                }
            }
        }
    }
    while (storeaccesses != NULL)
        storeaccesses = (StoreAccessList *)discard3((VoidStar)storeaccesses);
    while (knownregs != NULL)
        knownregs = (RegValue *)discard3((VoidStar)knownregs);
#ifdef ENABLE_CSE
    if (debugging(DEBUG_CSE) && var_cse_enabled > 16)
        cse_printexits(flags, exit, exit1);
#endif
    blk_available_(block) = liveexprns;
    blk_wanted_(block) = wantedexprns;
    blk_killed_(block) = killedlocations;
#undef find_adconv
#undef find_addrr
#undef estack_push
}

static void csescan_setup(void)
{
    cse_tab = (Exprn **) CSEAlloc(HASHSIZE * sizeof(Exprn **));
    memclr(cse_tab, HASHSIZE * sizeof(Exprn **));
    locations = (Location **) CSEAlloc(LOCHASHSIZE * sizeof(Location **));
    memclr(locations, LOCHASHSIZE * sizeof(Location **));
    cseidsegment = CSEIDSEGSIZE;
    csealiasid = csealiaslimit = 0;
    csenonaliasid = 0; csenonaliaslimit = CSEIDSEGSIZE;
    locationid = 0;
    floatconlist = NULL;
    knownregs = NULL;
    loadrs = NULL;
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
        left->nodeid = (newid << 5) | (left->nodeid & 0x1f);
        exprn_(newid) = left;
    }
}
#endif

static void addkilledexprns(int32 locno, VoidStar arg)
{
    Location *loc = loc_(locno);
    VRegSetP *s = (VRegSetP *)(int) arg;
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
        if (debugging(DEBUG_CSE) && var_cse_enabled > 16)
            cc_msg("L%li\n", (long)blklabname_(p));
        cse_scanblock(p);
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
                universe = cseset_insert(i, universe, NULL);
            for (i = csealiasid+1 ; i < csealiaslimit ; i++)
                universe = cseset_delete(i, universe, NULL);
            for (i = csenonaliasid+1 ; i < csenonaliaslimit ; i++)
                universe = cseset_delete(i, universe, NULL);

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
            cseset_map(locs, addkilledexprns, (VoidStar) &s);
            cseset_discard(locs);
            if (s != NULL) cseset_union(s, loadrs);
            {   int32 n = cseset_size(s);
                if (n >= ul/2) {
                    VRegSetP s1 = cseset_copy(universe);
                    s1 = cseset_difference(s1, s);
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
