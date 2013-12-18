/*
 * peepgen.c: Peephole table generator
 * Copyright (C) Advanced Risc Machines Ltd., 1991
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/07/14 14:14:31 $
 * Revising $Author: nickc $
 */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#ifdef __STDC__
#  include <stdarg.h>
#else
#  include <varargs.h>
#endif

#define NO 0
#define YES 1
typedef int bool;

#define StrEq(a, b) (strcmp(a, b) == 0)

static int peephole_index;

#define deadbit(f) (1<<(f))

typedef enum P_OpType {
   pot_none,
   pot_and,
   pot_or,
   pot_andnot,
   pot_prop,
   pot_peep,
   pot_peep_m,
   pot_op,
   pot_op_m,
   pot_opinset,
   pot_opinset_m
} P_OpType;

static char const * const pot_name[] = {
  "pot_none",
  "pot_and",
  "pot_or",
  "pot_andnot",
  "pot_prop",
  "pot_peep",
  "pot_peep_m",
  "pot_op",
  "pot_op_m",
  "pot_opinset",
  "pot_opinset_m"
};

typedef enum {
  p_loads_r1,
  p_uses_r1,
  p_uses_r2,
  p_uses_r3
} P_OpPred;

static char const * const opr_name[] = {
  "p_loads_r1",
  "p_uses_r1",
  "p_uses_r2",
  "p_uses_r3"
};

typedef enum {
   pf_none,
   pf_op,
   pf_peep,
   pf_r1,
   pf_r2,
   pf_r3,
   pf_r4,
   pf_dataflow,
   pf_cond
} P_Field;

static char const * const field_name[] = {
  "pf_none",
  "pf_op",
  "pf_peep",
  "pf_r1",
  "pf_r2",
  "pf_r3",
  "pf_r4",
  "pf_dataflow",
  "pf_cond"
};

static char const * const dead_name[] = {
/* nb: indexed by P_Field (thus the null entries) */
  0,
  0,
  0,
  "p_dead_r1",
  "p_dead_r2",
  "p_dead_r3"
};

typedef enum {
  pu_r1,
  pu_r2,
  pu_r3,
  pu_r4,
  pu_r3mask
} P_Use;

static char const * const use_name[] = {
  "pu_r1",
  "pu_r2",
  "pu_r3",
  "pu_r4",
  "pu_r3mask"
};

typedef enum {
   prt_none,
   prt_kill,
   prt_swapr2r3,
   prt_set,
   prt_asr,
   prt_proc
} P_ReplaceType;

static char const * const rt_name[] = {
   "prt_none",
   "prt_kill",
   "prt_swapr2r3",
   "prt_set",
   "prt_asr",
   "prt_proc"
};

typedef enum {
  pft_none,
  pft_int,
  pft_exprn,
  pft_field,
  pft_inst,
  pft_constraint   /* last, with no name */
} P_FieldType;

static char const * const ft_name[] = {
  "pft_none",
  "pft_val",
  "pft_exprn",
  "pft_field",
  "pft_inst",
  "pft_val"
};

typedef enum {
  peo_proc1,
  peo_proc2,
  peo_add,
  peo_sub,
  peo_or,
  peo_and,
  peo_eor,
  peo_shl,
  peo_shr,
  peo_div,
  peo_mul
} P_ExprnOp;

static char const * const peo_name[] = {
  "peo_proc1",
  "peo_proc2",
  "peo_add",
  "peo_sub",
  "peo_or",
  "peo_and",
  "peo_eor",
  "peo_shl",
  "peo_shr",
  "peo_div",
  "peo_mul"
};

typedef enum {
  pct_proc0,
  pct_proc1,
  pct_proc2,
  pct_ne,
  pct_eq,
  pct_lt,
  pct_le,
  pct_gt,
  pct_ge,
  pct_ltu,
  pct_leu,
  pct_gtu,
  pct_geu,
  pct_neg,
  pct_or
} P_ConstraintOp;

static char const * const ct_name[] = {
  "pct_proc0",
  "pct_proc1",
  "pct_proc2",
  "pct_ne",
  "pct_eq",
  "pct_lt",
  "pct_le",
  "pct_gt",
  "pct_ge",
  "pct_ltu",
  "pct_leu",
  "pct_gtu",
  "pct_geu",
  "pct_negate",
  "pct_or"
};

#define MaxOps 5
#define MaxConstraints 32
#define MaxReplace 32

typedef struct ProcDef {
  struct ProcDef *next;
  int n;
  char *arg;
  char *line[1];
} ProcDef;

typedef struct PeepExprn PeepExprn;

typedef struct {
  int inst;
  P_Field field;
} P_InstField;

typedef struct PeepConstraint PeepConstraint;

typedef struct PeepField {
  P_FieldType type;
  union {
    union {
      PeepConstraint *c;
      int i;
    } c;
    char *val;
    P_InstField f;
    union {
      PeepExprn *ex;
      int i;
    } ex;
  } f;
} PeepField;

struct PeepExprn {
  PeepExprn *next;
  P_ExprnOp op;
  char *fn;
  PeepField f1, f2;
};

typedef struct PeepReplace {
  P_ReplaceType type;
  int procno;
  PeepField f1, f2;
} PeepReplace;

typedef struct PeepOpDef {
  P_OpType type;
  int setcount;
  union {
    struct {
      union { struct PeepOpDef *p; int i; } op1;
      union { struct PeepOpDef *p; int i; } op2;
    } sub;
    struct {
      union { char *c; int i; } val;
      union { char *c; int i; } mask;
    } op;
    struct {
      int val;
      union { char *c; int i; } mask;
    } prop;
    struct {
      union { char **cp; int i; } ops;
      union { char *c; int i; } mask;
    } set;
  } p;
} PeepOpDef;

typedef struct PeepReplaceDef PeepReplaceDef;

typedef struct {
  int use;
  int kill;
} RegisterUsage;

typedef struct PeepOp {
  char *label;
  int index;
  PeepOpDef op;
  int debugix;
  int replacecount;
  union {
    PeepReplaceDef *r;
    int i;
  } replace;
  int dead;
  RegisterUsage maynot;
} PeepOp;

struct PeepReplaceDef {
  PeepReplaceDef *next;
  PeepOp *op;
  PeepReplace *replace[1];
};

struct PeepConstraint {
  P_ConstraintOp ctype;
  char *proc;
  PeepField f1, f2;
};

typedef struct PeepHole {
  struct PeepHole *next;
  int trace;
  int index;
  int opcount;
  int constraintcount;
  PeepOp **ops;
  PeepConstraint **constraints;
  char *gapconstraint;
} PeepHole;

typedef struct {
  const char *name;
  int val;
} KeyTable;

typedef struct {
  const char *name;
  int val;
  int args;
} KeyFnTable;

static KeyTable const op_predicates[] = {
  {"loads_r1", p_loads_r1},
  {"sets_r1", p_loads_r1},
  {"uses_r1", p_uses_r1},
  {"uses_r2", p_uses_r2},
  {"uses_r3", p_uses_r3},
  {0,0}
};

static KeyTable const op_dead[] = {
  {"dead_r1", deadbit(pf_r1)},
  {"dead_r2", deadbit(pf_r2)},
  {"dead_r3", deadbit(pf_r3)},
  {0,0}
};

static KeyTable const eop_table[] = {
  {"+", peo_add},
  {"-", peo_sub},
  {"|", peo_or},
  {"&", peo_and},
  {"^", peo_eor},
  {">>", peo_shr},
  {"<<", peo_shl},
  {"/", peo_div},
  {"*", peo_mul},
  {0,0}
};

static KeyTable const constraint_ops[] = {
  {"!=", pct_ne},
  {"==", pct_eq},
  {"<",  pct_lt},
  {"<=", pct_le},
  {">",  pct_gt},
  {">=", pct_ge},
  {"<u", pct_ltu},
  {"<=u",pct_leu},
  {">u", pct_gtu},
  {">=u",pct_geu},
  {"!", pct_neg},
  {"||", pct_or},
  {0,0}
};

static KeyTable const replace_ops[] = {
  {"=", prt_set},
  {0,0}
};

static KeyFnTable const replace_keys[] = {
  {"kill", prt_kill, 1},
  {"swapr2r3", prt_swapr2r3, 1},
  {"adjuststackrefs", prt_asr, 2},
  {0,0}
};

static KeyTable const field_table[] = {
  {"op", pf_op},
  {"r1", pf_r1},
  {"r2", pf_r2},
  {"r3", pf_r3},
  {"m",  pf_r3},
  {"r4", pf_r4},
  {"peep", pf_peep},
  {"dataflow", pf_dataflow},
  {"cond", pf_cond},
  {0,0}
};

#define Id_MaxLen 255
typedef struct {
  int len;
  char b[Id_MaxLen+1];
} Id;

typedef struct ProcDefList {
  ProcDef *list;
  ProcDef **end;
  int count;
} ProcDefList;

typedef struct FnArgList {
  struct FnArgList *next;
  const char *fn;
  int args;
} FnArgList;

static ProcDefList rproc;

static FnArgList *constraintfns,
                 *exprnfns;

const char *pcp_regset_unused,
           *pcp_regset_unkilled,
           *pcp_nointervening,
           *pep_bit;

static FILE *debug;
static char *debugbuf;
static int debugix;
#define DebugSize 2048

static void InitProcDefList(ProcDefList *p) {
  p->list = 0;
  p->end = &p->list;
  p->count = 0;
}

static KeyTable const *LookUp(const Id *id, const KeyTable *t) {
  for (; t->name != 0; t++)
    if (StrEq(id->b, t->name)) return t;
  return NULL;
}

static KeyFnTable const *LookUpFn(const Id *id, const KeyFnTable *t) {
  for (; t->name != 0; t++)
    if (StrEq(id->b, t->name))
      return t;
  return 0;
}

static char *outputfile;
static FILE *input, *output;
static int line_number;
static int bracket_nesting;

static void die(void) {
  fclose(output);
  if (outputfile != 0) remove(outputfile);
  exit(1);
}

static void Message(const char *format, const char *s1, va_list ap) {
  fprintf(stderr, "%s at line %d: ", s1, line_number);
  vfprintf(stderr, format, ap);
  fprintf(stderr, "\n\n");
}

static void FatalError(const char *s, ...) {
  va_list ap;
  va_start(ap, s);
  Message(s, "fatal error", ap);
  va_end(ap);
  die();
}

static void SyntaxError(const char *s, ...) {
  va_list ap;
  va_start(ap, s);
  Message(s, "syntax error", ap);
  va_end(ap);
  die();
}

static void Warn(const char *s, ...) {
  va_list ap;
  va_start(ap, s);
  Message(s, "warning", ap);
  va_end(ap);
}

static int NextCh() {
  int ch = fgetc(input);
  if (debug)
    debugbuf[debugix++] = ch;
  if (ch == '\n')
    line_number++;
  else if (ch == '(')
    bracket_nesting++;
  else if (ch == ')')
    bracket_nesting--;
  return ch;
}

static int SigCh(int ch) {
  for (;;) {
    while (isspace(ch)) ch = NextCh();
    if (ch != ';') return ch;
    do ch = NextCh(); while (ch != '\n');
  }
}

static int WantCh(char want, int ch, const char *whence) {
  ch = SigCh(ch);
  if (ch != want) SyntaxError("%s: need '%c'", whence, want);
  return SigCh(NextCh());
}

typedef struct StringList {
  struct StringList *next;
  char string[1];
} StringList;

static StringList *syms;

static char *NewHeapString(const char *s, int len) {
  StringList *sym = (StringList *) malloc(sizeof(*sym)+len);
  if (sym == 0) FatalError("out of store");
  memcpy(sym->string, s, len+1);
  sym->next = syms; syms = sym;
  return sym->string;
}

static char *HeapString(const Id *i) {
  StringList *sym;
  for (sym = syms; sym != NULL; sym = sym->next)
    if (StrEq(i->b, sym->string))
      return sym->string;
  return NewHeapString(i->b, i->len);
}

static void AddFn(FnArgList **pp, const char *fn, int args) {
  FnArgList *p;
  for (; (p = *pp) != NULL; pp = &(p->next))
    if (StrEq(fn, p->fn)) {
      if (p->args != args) SyntaxError("Inconsistent argument count for %s", fn);
      return;
    }
  p = (FnArgList *) malloc(sizeof(*p));
  p->next = NULL; p->fn = fn; p->args = args;
  *pp = p;
}

static void StuffId(Id *id, int ch) {
  if (id->len >= Id_MaxLen)
    FatalError("Overlong id %s", id->b);
  id->b[id->len++] = ch;
  id->b[id->len] = 0;
}

static int ReadId(Id *id, int ch) {
  id->len = 0;
  while (!isspace(ch) && ch != '.' && ch != '(' && ch != ')') {
    StuffId(id, ch);
    ch = NextCh();
  }
  return SigCh(ch);
}

static int ReadOpSpec(int ch, PeepOpDef *p) {
  Id id;
  char *mask = 0;
  ch = SigCh(ch);
  if (ch == '&') {
    id.len = 0;
    ch = SigCh(NextCh());
    while (ch != '=' && ch != '<' && ch != '\n') {
      if (!isspace(ch)) StuffId(&id, ch);
      ch = NextCh();
    }
    if (id.len == 0 || ch == '\n')
      SyntaxError("bad %s mask", p->type == pot_op ? "op" : "peep");
    mask = HeapString(&id);
  }
  if (ch == '=' && (ch = NextCh()) == '=') {
    ch = ReadId(&id, SigCh(NextCh()));
    p->setcount = 0;
    p->p.op.val.c = HeapString(&id);
    p->p.op.mask.c = mask;
  } else if (p->type == pot_op && ch == '<' && (ch = NextCh()) == '-') {
    int n = 0;
    typedef struct strlist { struct strlist *next; char *p; } strlist;
    strlist *strl = 0, **strp = &strl;
    ch = WantCh('(', NextCh(), "opset");
    while (ch != ')') {
      strlist *s;
      ch = ReadId(&id, ch);
      if (id.len == 0) SyntaxError("unexpected %c in op set", ch);
      n++;
      s = (strlist *) malloc(sizeof(*s));
      *strp = s; strp = &s->next;
      s->next = 0; s->p = HeapString(&id);
      if (ch == ',') ch = SigCh(NextCh());
    }
    ch = WantCh(')', ch, "opset");
    { char **v = (char **) malloc(n * sizeof(char *));
      int i;
      for (i = 0; i < n; i++, strl = strl->next)
        v[i] = strl->p;
      p->type = pot_opinset;
      p->setcount = n;
      p->p.set.ops.cp = v;
      p->p.set.mask.c = mask;
    }
  } else
    SyntaxError("unexpected '%c' in %s specification", ch,
                 p->type == pot_op ? "op" : "peep");
  return ch;
}

static PeepOp *NextOp(int *chp) {
  Id id;
  int ch = SigCh(*chp);
  PeepOp *res;
  PeepOpDef op;
  PeepOpDef *opp = &op;
  KeyTable const *i;
  P_OpType andor = pot_op;
  memset(opp, 0, sizeof(*opp));
  if (ch == ')') {
    *chp = ch;
    return 0;
  }
  res = (PeepOp *) malloc(sizeof(*res));
  res->replacecount = 0;
  res->replace.r = NULL;
  res->dead = 0;
  res->maynot.use = res->maynot.kill = 0;

  ch = ReadId(&id, ch);
  res->label = HeapString(&id);
  ch = WantCh('(', ch, "NextOp");
  for (;;) {
    ch = SigCh(ch);
    if (ch == ')') {
      op.type = pot_none;
      break;
    }
    ch = ReadId(&id, ch);
    if ((i = LookUp(&id, op_dead)) != 0) {
      if (andor != pot_op && andor != pot_and)
        SyntaxError("Bad boolean operator for dead_xx");
      res->dead |= i->val;
    } else {
      if (andor != pot_op) {
        PeepOpDef *p1, *p2;
        p1 = (PeepOpDef *) malloc(sizeof(*p1));
        p2 = (PeepOpDef *) calloc(sizeof(*p2), 1);
        *p1 = *opp;
        opp->type = andor;
        opp->setcount = 0;
        opp->p.sub.op1.p = p1;
        opp->p.sub.op2.p = p2;
        opp = p2;
      }
      if (StrEq("op", id.b)) {
        opp->type = pot_op;
        ch = ReadOpSpec(ch, opp);
      } else if (StrEq("peep", id.b)) {
        opp->type = pot_peep;
        ch = ReadOpSpec(ch, opp);
      } else if ((i = LookUp(&id, op_predicates)) != 0) {
        opp->type = pot_prop;
        opp->setcount = 0;
        opp->p.prop.val = i->val;
        opp->p.op.mask.c = 0;
      } else
        SyntaxError("Unknown op predicate '%s'", id.b);
    }

    if (ch == '&') {
      if ((ch = NextCh()) == '&')
        andor = pot_and;
      else if (ch == '!')
        andor = pot_andnot;
      else break;
    } else if (ch == '|' && (ch = NextCh()) == '|')
      andor = pot_or;
    else break;

    ch = SigCh(NextCh());
  }
  if (debug) { res->debugix = debugix; }
  *chp = WantCh(')', ch, "NextOp");
  res->op = op;
  return res;
}

static int opMax;

static PeepOp **ReadOps(int *chp, int *np) {
  int ch = WantCh('(', *chp, "ReadOps");
  int count = 0;
  PeepOp *ops[MaxOps];
  ops[count] = NextOp(&ch);
  for (; ops[count] != NULL ;) {
    ops[++count] = NextOp(&ch);
  }
  if (count < 1) SyntaxError("Too few ops");
  if (opMax < count) opMax = count;
  *chp = WantCh(')', ch, "ReadOps");
  { PeepOp **r = (PeepOp **) malloc(count*sizeof(*r));
    int i;
    for (i = 0; i < count; i++) {
      r[i] = ops[count-i-1];
      r[i]->index = i;
    }
    *np = count;
    return r;
  }
}

static int FindOp(char *label, PeepOp **p, int n) {
  for (; --n >= 0; )
    if (StrEq(label, p[n]->label)) break;
  return n;
}

#define rf_rid 1
#define rf_lab 2

static int ReadField(Id *id, PeepHole const *ph, PeepField *f,
                     int ch, int flags) {
  int i;
  if (flags & rf_rid) {
    ch = SigCh(ch);
    if (ch == '(') {
      PeepExprn *pex = (PeepExprn *) malloc(sizeof(*pex));
      int operands = 2;
      char *fn = NULL;
      KeyTable const *kp;
      ch = ReadId(id, SigCh(NextCh()));
      kp = LookUp(id, eop_table);
      if (kp != NULL)
        pex->op = (P_ExprnOp)kp->val;
      else
        fn = HeapString(id);
      ch = ReadField(id, ph, &pex->f1, ch, rf_rid);
      if (fn != 0) {
        if (ch == ')')
          operands = 1, pex->op = peo_proc1;
        else
          pex->op = peo_proc2;
        AddFn(&exprnfns, fn, operands);
      }
      pex->fn = fn;
      if (operands == 2)
        ch = ReadField(id, ph, &pex->f2, ch, rf_rid);
      else
        pex->f2.type = pft_none;
      f->type = pft_exprn;
      f->f.ex.ex = pex;
      return WantCh(')', ch, "ReadExp");
    }
    ch = ReadId(id, SigCh(ch));
  }
  if (ch == '.') {
    KeyTable const *fn;
    i = FindOp(id->b, ph->ops, ph->opcount);
    if (i < 0) SyntaxError("Bad op label %s", id->b);
    ch = ReadId(id, NextCh());
    fn = LookUp(id, field_table);
    if (fn == 0) SyntaxError("Bad field %s", id->b);
    f->type = pft_field;
    f->f.f.inst = i;
    f->f.f.field = (P_Field)fn->val;

  } else if ((flags & rf_lab) &&
             (i = FindOp(id->b, ph->ops, ph->opcount)) >= 0) {
    f->type = pft_inst;
    f->f.f.inst = i;
    f->f.f.field = pf_none;

  } else {
    f->type = pft_int;
    f->f.val = HeapString(id);
  }
  return SigCh(ch);
}

static PeepConstraint *ReadConstraint(PeepHole const *ph, int *chp) {
  int ch = WantCh('(', *chp, "ReadConstraint");
  Id id;
  PeepConstraint *res = (PeepConstraint *)malloc(sizeof(*res));
  KeyTable const *op;

  res->f1.type = res->f2.type = pft_none;
  ch = ReadId(&id, ch);
  op = LookUp(&id, constraint_ops);
  if (op == 0) {
    int args = pct_proc0;
    res->proc = HeapString(&id);
    if (ch != ')') { args++; ch = ReadField(&id, ph, &res->f1, ch, rf_rid+rf_lab); }
    if (ch != ')') { args++; ch = ReadField(&id, ph, &res->f2, ch, rf_rid+rf_lab); }
    res->ctype = (P_ConstraintOp)args;
    if (res->proc != pcp_nointervening)
        AddFn(&constraintfns, res->proc, args);
    *chp = WantCh(')', ch, "ReadConstraint");
    return res;
  }
  { if (op->val == pct_neg || op->val == pct_or) {
      *chp = ch;
      res->f1.type = pft_constraint;
      res->f1.f.c.c = ReadConstraint(ph, chp);
      if (op->val == pct_or) {
        res->f2.type = pft_constraint;
        res->f2.f.c.c = ReadConstraint(ph, chp);
      }
      ch = *chp;
    } else {
      ch = ReadField(&id, ph, &res->f1, ch, rf_rid);
      ch = ReadField(&id, ph, &res->f2, ch, rf_rid);
    }
    res->proc = NULL;
    res->ctype = (P_ConstraintOp)op->val;
  }
  *chp = WantCh(')', ch, "ReadConstraint");
  return res;
}

#define MayNotKill(p, i, f) ((p)->ops[i]->maynot.kill |= (1 << ((f) - pf_r1)))
#define MayNotUse(p, i, f) ((p)->ops[i]->maynot.use |= (1 << ((f) - pf_r1)))

static PeepConstraint **ReadConstraints(PeepHole *ph, int *chp, int *np) {
  PeepConstraint *pcs[MaxConstraints];
  int i = 0;
  for (; *chp == '('; ) {
    PeepConstraint *pc = ReadConstraint(ph, chp);
    if (pc->ctype == pct_proc1 && pc->proc != 0 &&
        (pc->proc == pcp_regset_unused || pc->proc == pcp_regset_unkilled) &&
        pc->f1.type == pft_exprn &&
        pc->f1.f.ex.ex->op == peo_proc1 &&
        pc->f1.f.ex.ex->fn == pep_bit &&
        pc->f1.f.ex.ex->f1.type == pft_field) {
      int inst = pc->f1.f.ex.ex->f1.f.f.inst;
      P_Field f = pc->f1.f.ex.ex->f1.f.f.field;
      if (pc->proc == pcp_regset_unused)
        MayNotUse(ph, inst, f);
      MayNotKill(ph, inst, f);
    } else if (pc->ctype == pct_proc1 && pc->proc == pcp_nointervening) {
      if (pc->f1.type != pft_int) SyntaxError("Bad gap constraint");
      ph->gapconstraint = pc->f1.f.val;
    } else {
      pcs[i++] = pc;
    }
  }
  *chp = WantCh(')', *chp, "ReadConstraints");
  { PeepConstraint **res = (PeepConstraint **) malloc(i * sizeof(*res));
    memcpy(res, pcs, i * sizeof(res));
    *np = i;
    return res;
  }
}

#define BSize 80

static PeepReplace *ReadReplacement(PeepHole const *ph, int *chp) {
  int ch = WantCh('(', *chp, "ReadReplacement");
  Id id;
  PeepReplace *res = (PeepReplace *) malloc(sizeof(*res));
  if (ch == '{') {
    int count = 0;
    int depth = 0;
    char *p[32];
    int i = 0;
    char b[BSize];
    for (;; ch = NextCh()) {
      if (ch == EOF) FatalError("EOF in replacement proc");
      if (i == BSize-1) {
        char *q = (char *)malloc(BSize);
        b[BSize-1] = 0;
        memcpy(q, b, BSize);
        p[count++] = q;
        i = 0;
      }
      b[i++] = ch;
      if (ch == '}') {
        if (--depth == 0) break;
      } else if (ch == '{') {
        depth++;
      }
    }
    if (i > 0) {
      char *q = (char *)malloc(i+1);
      b[i] = 0;
      memcpy(q, b, i+1);
      p[count++] = q;
    }
    { ProcDef *d = (ProcDef *) malloc(sizeof(ProcDef)+count * sizeof(char *));
      d->n = count;
      memcpy(d->line, p, count * sizeof(char *));
      d->next = 0;
      *(rproc.end) = d;
      rproc.end = &d->next;
      res->procno = rproc.count++;
      ch = ReadId(&id, SigCh(NextCh()));
      i = FindOp(id.b, ph->ops, ph->opcount);
      if (i < 0) SyntaxError("Bad op label %s", id.b);
      d->arg = HeapString(&id);
    }
    res->type = prt_proc;
    res->f1.type = pft_inst;
    res->f1.f.f.inst = i;
    res->f2.type = pft_none;
  } else {
    KeyFnTable const *fn;
    int args = 2;
    int rflag = rf_rid;
    res->procno = -1;
    ch = ReadId(&id, ch);
    fn = LookUpFn(&id, replace_keys);
    if (fn != 0) {
      res->type = (P_ReplaceType)fn->val;
      rflag = rf_rid+rf_lab;
      args = fn->args;
    } else {
      KeyTable const *k;
      k = LookUp(&id, replace_ops);
      if (k == 0) SyntaxError("Bad replace operator %s", id.b);
      res->type = (P_ReplaceType)k->val;
    }
    ch = ReadField(&id, ph, &res->f1, ch, rflag);
    if (args > 1)
      ch = ReadField(&id, ph, &res->f2, ch, rf_rid);
    else
      res->f2.type = pft_none;
  }
  *chp = WantCh(')', ch, "ReadReplacement");
  return res;
}

static int TargetInst(PeepReplace *pr) {
  return (pr->f1.type != pft_field && pr->f1.type != pft_inst) ?
                   0 : pr->f1.f.f.inst;
}

static void ReadReplacements(PeepHole *ph, int *chp) {
  PeepReplace *pr[MaxReplace];
  int n = 0, i;
  int ch = SigCh(*chp);

  for (; ch == '('; n++) {
    pr[n] = ReadReplacement(ph, &ch);
  }
  for (i = 0; i < n; i++)
    ph->ops[TargetInst(pr[i])]->replacecount++;

  for (i = 0; i < ph->opcount; i++) {
    PeepOp *op = ph->ops[i];
    PeepReplaceDef *prd = (PeepReplaceDef *)
        malloc(sizeof(*prd) + (op->replacecount-1) * sizeof(PeepReplace *));
    op->replace.r = prd;
    prd->next = 0;
    prd->op = op;
    op->replacecount = 0;
  }
  for (i = 0; i < n; i++) {
    PeepOp *op = ph->ops[TargetInst(pr[i])];
    op->replace.r->replace[op->replacecount++] = pr[i];
  }
  for (i = 0; i < ph->opcount; i++) {
    PeepOp *op = ph->ops[i];
    if (op->replacecount > 1) {
      int j;
      bool hadnonkill = NO, hadkill = NO;
      PeepReplace **r = op->replace.r->replace;
      for (j = 0; j < op->replacecount; j++)
        if (r[j]->type == prt_kill)
          hadkill = YES;
        else if (r[j]->type != prt_asr)
          hadnonkill = YES;

      if (hadkill && hadnonkill)
        Warn("kill together with field write probably mistaken");
    }
  }
  *chp = WantCh(')', ch, "ReadReplacements");
}

#define DEFINE_JOPTABLE  1
#define DEFINE_A_JOPTABLE 1
#define ENABLE_CG 1 /* bodge - get JOPCODE tables to include names */
#define JOPCODEDEF_ONLY

typedef long int32;
typedef unsigned long unsigned32;

#define _defs_LOADED 1

#include "jopcode.h"
#include "mcdpriv.h"

static int32 u_bit[] = {
  _J_SET_R1,
  _J_READ_R1,
  _J_READ_R2,
  _J_READ_R3
};

static int32 a_u_bit[] = {
  _a_set_r1,
  _a_read_r1,
  _a_read_r2,
  _a_read_r3
};

#define JOPNAME(op) ((op)<=J_LAST_JOPCODE ? joptable[op].name :\
                                            a_joptable[(op)-(J_LAST_JOPCODE+1)].name)

static bool MatchOp(char const *opplus, const char *op) {
  char justop[16];
  if (opplus[0] == 'J' && opplus[1] == '_') {
    int len = strcspn(opplus, " +");
    memcpy(justop, &opplus[2], len-2);
    justop[len-2] = 0;
    opplus = &justop[0];
  }
  return StrEq(opplus, op);
}

static bool OpSatisfies(J_OPCODE op, PeepOpDef const *p) {
  switch(p->type) {
    case pot_and:     return OpSatisfies(op, p->p.sub.op1.p) &&
                             OpSatisfies(op, p->p.sub.op2.p);
    case pot_or:      return OpSatisfies(op, p->p.sub.op1.p) ||
                             OpSatisfies(op, p->p.sub.op2.p);
    case pot_andnot:  return OpSatisfies(op, p->p.sub.op1.p) &&
                            !OpSatisfies(op, p->p.sub.op2.p);
    case pot_prop:    if (op <= J_LAST_JOPCODE)
                          return (_joptable(op) & u_bit[p->p.prop.val]) != 0;
                      else
                          return (a_attributes(op) & a_u_bit[p->p.prop.val]) != 0;
    case pot_op_m:
    case pot_op:      return MatchOp(p->p.op.val.c, JOPNAME(op));
    case pot_opinset_m:
    case pot_opinset: { char const *opname = JOPNAME(op);
                        int i = 0;
                        for (i = 0; i < p->setcount; i++)
                          if (MatchOp(p->p.set.ops.cp[i], opname))
                            return YES;
                        return NO;
                      }
    default:          return YES;
  }
}

static unsigned32 a_opuse(char const *op, P_OpPred u)
{
  int n = 0;
  char justop[16];
  if (op[0] == 'J' && op[1] == '_') {
    int len = strcspn(op, " +");
    memcpy(justop, &op[2], len-2);
    justop[len-2] = 0;
    op = &justop[0];
  }
  for (n = 0; joptable[n].name != 0; n++)
    if (StrEq(op, joptable[n].name))
      return _joptable(n) & u_bit[u];
  for (n = 0; a_joptable[n].name != 0; n++)
    if (StrEq(op, a_joptable[n].name))
      return a_joptable[n].bits & a_u_bit[u];
  return 0;
}

static bool peepop_use(PeepOpDef const *p, P_OpPred u) {
  if (u == p_uses_r3+1) /* uses_r4 - a function of peep as well as op */
    return YES;

  switch (p->type) {
  case pot_op_m:
  case pot_op:        return a_opuse(p->p.op.val.c, u) != 0;
  case pot_prop:      return p->p.prop.val == u;
  case pot_opinset:
  case pot_opinset_m: { int i = 0;
                        for (i = 0; i < p->setcount; i++)
                          if (a_opuse(p->p.set.ops.cp[i], u))
                            return YES;
                        return NO;
                      }
  case pot_and:
  case pot_or:
  case pot_andnot:    return peepop_use(p->p.sub.op1.p, u) ||
                             peepop_use(p->p.sub.op2.p, u);
  }
  return NO;
}

static bool FindConstraintEqFields(P_InstField *resf, PeepHole *p, P_InstField *f) {
  int cix = 0;
  for (; cix < p->constraintcount; cix++) {
    PeepConstraint *pc = p->constraints[cix];
    if (pc->f1.type == pft_field && pc->f2.type == pft_field) {
      if (pc->f1.f.f.inst == f->inst && pc->f1.f.f.field == f->field) {
        *resf = pc->f2.f.f;
        return YES;
      } else if (pc->f2.f.f.inst == f->inst && pc->f2.f.f.field == f->field) {
        *resf = pc->f1.f.f;
        return YES;
      }
    }
  }
  return NO;
}

static void DeduceDeadBits(PeepHole *p) {
  int i;
  for (i = 0; i < p->opcount; i++) {
    int count = p->ops[i]->replacecount;
    PeepReplace **prv = p->ops[i]->replace.r->replace;
    for (; --count >= 0; ) {
      PeepReplace *pr = prv[count];
      P_ReplaceType w = pr->type;
      P_InstField otherf;
      if (w == prt_set && pr->f2.type == pft_field) {
        if (pr->f1.f.f.field == pf_r1 && peepop_use(&p->ops[i]->op, p_loads_r1) &&
            FindConstraintEqFields(&otherf, p, &pr->f1.f.f))
          p->ops[otherf.inst]->dead |= deadbit(otherf.field);

      } else if (w == prt_kill && peepop_use(&p->ops[i]->op, p_loads_r1)) {
        P_InstField f;
        f.inst = i; f.field = pf_r1;
        if (FindConstraintEqFields(&otherf, p, &f)) {
          int countj = p->ops[otherf.inst]->replacecount;
          P_Field field = otherf.field;
          int j;
          PeepReplace **pjv = p->ops[otherf.inst]->replace.r->replace;
          for (j = 0; j < countj ; j++) {
            PeepReplace *pj = pjv[j];
            if (pj->type == prt_swapr2r3 &&
                (field == pf_r2 || field == pf_r3))
              field = field == pf_r2 ? pf_r3 : pf_r2;
            else if (pj->type == prt_set &&
                     pj->f2.type == pft_field &&
                     pj->f1.f.f.field == field) {
              p->ops[otherf.inst]->dead |= deadbit(otherf.field);
              break;
            }
          }
        }
      }
    }
  }
}

static bool n_use(char const *newop, PeepOpDef const *p, P_OpPred u) {
  bool b;
  if (newop != NULL)
    b = (a_opuse(newop, u) != 0);
  else
    b = peepop_use(p, u);
  return b;
}

static bool FirstRealOp(PeepOp **ops, int i) {
  for (; --i >= 0; )
    if (ops[i]->op.type != pot_none)
      return NO;
  return YES;
}

#define OldUsesField(p, i, f) \
    ((f)>=pf_r1 && (f) <= pf_r4 && \
     peepop_use(&(p)->ops[i]->op, (P_OpPred)((f)+p_uses_r1-pf_r1)))
#define OldWritesR1(p, i) peepop_use(&(p)->ops[i]->op, p_loads_r1)

#define NewUsesField(new, p, i, f) \
    ((f)>=pf_r1 && (f) <= pf_r4 && \
     n_use(new, &(p)->ops[i]->op, (P_OpPred)((f)+p_uses_r1-pf_r1)))
#define NewWritesR1(new, p, i) n_use(new, &(p)->ops[i]->op, p_loads_r1)

static void DeduceUsage(PeepHole *p) {
  int i;

  if (p->gapconstraint != NULL && StrEq(p->gapconstraint, "G_ANY"))
    return;

  for (i = 0; i < p->constraintcount; i++) {
    PeepConstraint *c = p->constraints[i];
    if (c->ctype == pct_eq && c->f1.type == pft_field && c->f2.type == pft_field) {
      P_InstField *f1 = &c->f1.f.f,
                  *f2 = &c->f2.f.f;
      if (f1->inst < f2->inst)
        f1 = &c->f2.f.f, f2 = &c->f1.f.f;
      else if (f1->inst == f2->inst)
        continue;
      /* f1 is in the earlier instruction */
      if ( OldUsesField(p, f2->inst, f2->field) &&
           ( OldUsesField(p, f1->inst, f1->field) ||
             ( f1->field == pf_r1 && OldWritesR1(p, f1->inst)) ))
        MayNotKill(p, f1->inst, f1->field);
    }
  }
  for (i = 0; i < p->opcount; i++) {
    int count = p->ops[i]->replacecount;
    PeepReplace **prv = p->ops[i]->replace.r->replace;
    bool swap = NO;
    int c;
    char const *newop = NULL;
    for (c = 0; c < count; c++) {
      PeepReplace *pr = prv[c];
      if (pr->type == prt_set &&
          pr->f1.type == pft_field && pr->f1.f.f.field == pf_op &&
          pr->f2.type == pft_int) {
        newop = pr->f2.f.val;
        break;
      }
    }
    for (c = 0; c < count; c++) {
      PeepReplace *pr = prv[c];
      switch (pr->type) {
      case prt_swapr2r3:
        swap = YES;
        break;

      case prt_kill:
        if (i != 0 && OldWritesR1(p, i))
          MayNotUse(p, i, pf_r1);
        break;

      case prt_set:
        { P_Field f1 = pr->f1.f.f.field;
          if (OldUsesField(p, i, f1)) {
            P_Field f = f1;
            if (swap && (f == pf_r2 || f == pf_r3))
              f = f == pf_r2 ? pf_r3 : pf_r2;
            MayNotKill(p, i, f);
          }

          if (pr->f2.type == pft_field) {
            int i2 = pr->f2.f.f.inst;
            if (p->ops[i]->op.type == pot_none &&
                FirstRealOp(p->ops, i2))
              break;

            if (OldUsesField(p, i2, pr->f2.f.f.field) &&
                NewUsesField(newop, p, i, f1))
              MayNotKill(p, i2, pr->f2.f.f.field);

            else if (f1 == pf_r1 && NewWritesR1(newop, p, i))
              MayNotUse(p, i, pf_r1);
          }
        }
      }
    }
  }
}

#undef MayNotKill
#undef MayNotUse

#undef OldUsesField
#undef OldWritesR1

#undef NewUsesField
#undef NewWritesR1

static void WriteDeadBits(int d, FILE *f) {
  int i = pf_r1;
  for (; i <= pf_r4; i++)
    if (d & deadbit(i)) {
      d ^= deadbit(i);
      fputs(dead_name[i], f);
      if (d == 0) break;
      fputc('+', f);
    }
}

static void WriteUsage(int u, char const *after, FILE *f) {
  int n;
  if (u == 0)
    fputc('0', f);
  else for (n = pu_r1; n < pu_r3mask; n++)
    if (u & (1 << n)) {
      u ^= 1 << n;
      fputs(use_name[n], f);
      if (u == 0) break;
      fputc('+', f);
    }
  fputs(after, f);
}

static PeepHole *ReadPeepHole(int *chp) {
  bool traceit = NO;
  int ch = SigCh(*chp);
  if (ch == 'T') { traceit = YES; ch = SigCh(NextCh()); }
  if (ch == EOF) return 0;
  { PeepHole *p = (PeepHole *) malloc(sizeof(*p));
    p->next = NULL;
    p->index = ++peephole_index;
    p->trace = traceit;
    p->gapconstraint = NULL;

    ch = WantCh('(', ch, "ReadPeepHole 1");
    p->ops = ReadOps(&ch, &p->opcount);

    ch = WantCh('(', ch, "ReadPeepHole 2");
    p->constraints = ReadConstraints(p, &ch, &p->constraintcount);

    ch = WantCh('(', ch, "ReadPeepHole 3");
    ReadReplacements(p, &ch);
    DeduceDeadBits(p);
    DeduceUsage(p);

    if (debug) {
      int i = p->opcount;
      int ix = 0;
      for (; --i >= 0; ) {
        PeepOp *op = p->ops[i];
        int end = op->debugix;
        int posn = 0;
        if (debugbuf[end] == '\n')
          end--;
        else if (debugbuf[end+1] == ')' && debugbuf[end+2] == '\n')
          end++; /* cope with single op given as ( xx(..) ) */

        for (; ix <= end; ix++) {
          char c = debugbuf[ix];
          if (c == '\n')
            posn = 0;
          else
            posn++;
          fputc(c, debug);
        }
        for (; posn < 50;  posn++) fputc(' ', debug);
        fputs("; ", debug);
        if (op->dead != 0) WriteDeadBits(op->dead, debug);
        if (op->maynot.use != 0) {
          fputs(" maynotuse ", debug); WriteUsage(op->maynot.use, "", debug);
        }
        if (op->maynot.kill != 0) {
          fputs(" maynotkill ", debug); WriteUsage(op->maynot.kill, "", debug);
        }
        if (debugbuf[ix] != '\n') fputc('\n', debug);
      }
      for (; ix < debugix; ix++) fputc(debugbuf[ix], debug);
      debugix = 0;
    }
    *chp = WantCh(')', ch, "ReadPeepHole");
    return p;
  }
}

static void WriteField(PeepField const *f, char const *after) {
  switch (f->type) {
  case pft_none:  fputc('0', output); break;
  case pft_int:   fputs(f->f.val, output); break;
  case pft_field: fprintf(output, "fb_(%d, %s)", f->f.f.inst, field_name[f->f.f.field]);
                  break;
  case pft_inst:  fprintf(output, "fb_(%d, 0)", f->f.f.inst); break;
  case pft_constraint:
  case pft_exprn: fprintf(output, "%d", f->f.ex.i); break;
  }
  fputs(after, output);
}

typedef struct OpDefList {
  struct OpDefList *next;
  PeepOpDef *op;
} OpDefList;

typedef struct IOpList {
  struct IOpList *next;
  char *val;
} IOpList;

static int setindex,
           opindex;

static IOpList *iops;
static IOpList **iopp;
static OpDefList *opdefs;
static OpDefList **opdefp;

static int NewOpList(char *op) {
  IOpList *p = (IOpList *) malloc(sizeof(*p));
  p->next = 0; p->val = op;
  *iopp = p; iopp = &p->next;
  return setindex++;
}

static int FindOpList(char *op) {
  IOpList *p = iops;
  int i = 0;
  for (; p != NULL; p = p->next, i++)
    if (op == p->val) return i;
  return NewOpList(op);
}

static int FindSubOp(PeepOpDef *op) {
  OpDefList *p = opdefs;
  int i = 0;
  for (; p != NULL; p = p->next, i++)
    if (memcmp(p->op, op, sizeof(*op)) == 0) return i;
  p = (OpDefList *) malloc(sizeof(*p));
  p->next = 0;
  p->op = op;
  *opdefp = p; opdefp = &p->next;
  return opindex++;
}

static void WriteTableForOp(PeepOpDef *p) {
  switch (p->type) {
  case pot_peep:
  case pot_op:
    if (p->p.op.mask.c != 0) {
      p->p.op.val.i = FindOpList(p->p.op.val.c);
      p->p.op.mask.i = FindOpList(p->p.op.mask.c);
      p->type = (P_OpType)(p->type + (pot_peep_m - pot_peep));
    }
    break;

  case pot_opinset:
    if (p->p.set.mask.c != 0) {
      p->p.set.mask.i = FindOpList(p->p.set.mask.c);
      p->type = pot_opinset_m;
    } else
      p->p.set.mask.i = 0;
    { IOpList *q = iops;
      int i = 0;
      for (; q != NULL; q = q->next, i++)
        if (q->val == p->p.set.ops.cp[0]) {
          int ix = 1; IOpList *qq = q->next;
          for (; ix < p->setcount && qq != NULL; qq = qq->next, ix++)
            if (qq->val != p->p.set.ops.cp[ix]) break;
          if (ix==p->setcount) break;
        }
      if (q == NULL) {
        int ix;
        for (ix = 0; ix < p->setcount; ix++)
          NewOpList(p->p.set.ops.cp[ix]);
      }
      p->p.set.ops.i = i;
    }
    break;

  case pot_and:
  case pot_or:
  case pot_andnot:
    { PeepOpDef *op1 = p->p.sub.op1.p,
                *op2 = p->p.sub.op2.p;
      WriteTableForOp(op1);
      WriteTableForOp(op2);
      p->p.sub.op1.i = FindSubOp(op1);
      p->p.sub.op2.i = FindSubOp(op2);
      break;
    }

  case pot_none:
  case pot_prop:
    break;
  }
}

static void WriteOpDef(PeepOpDef *p, char const *after, RegisterUsage const *u) {

  fprintf(output, "{%s, ", pot_name[p->type]);
  WriteUsage(u->use, ", ", output);
  WriteUsage(u->kill, ", ", output);
  switch (p->type) {
    case pot_none:    fputs("0, 0", output);
                      break;

    case pot_prop:    fprintf(output, "0, %s", opr_name[p->p.prop.val]);
                      break;

    case pot_peep:
    case pot_op:      fprintf(output, "0, %s", p->p.op.val.c);
                      break;

    case pot_op_m:
    case pot_peep_m:  fprintf(output, "0, fh_(%d, %d)",
                              p->p.op.mask.i, p->p.op.val.i);
                      break;

    case pot_and:
    case pot_or:
    case pot_andnot:  fprintf(output, "0, fh_(%d, %d)",
                              p->p.sub.op1.i, p->p.sub.op2.i);
                      break;

    case pot_opinset_m:
    case pot_opinset: fprintf(output, "%d, fh_(%d, %d)",
                              p->setcount,
                              p->p.set.mask.i, p->p.set.ops.i);
  }
  fprintf(output, "}%s", after);
}

static RegisterUsage const nullusage = {0, 0};

static void WriteOpTables(PeepHole *pl) {
  int i;
  setindex = 0;
  opindex = 0;
  opdefs = 0; iops = 0;
  opdefp = &opdefs; iopp = &iops;
  for (; pl != NULL; pl = pl->next) {
    PeepOp **ops = pl->ops;
    for (i = 0; i < pl->opcount; i++)
      WriteTableForOp(&ops[i]->op);
  }
  fputs("static const int32 peepsets[] = {\n", output);
  for (i = 0; iops != NULL; i++, iops = iops->next)
    fprintf(output, "  /*%3d*/%s,\n", i, iops->val);
  fputs("  0\n};\nstatic const PeepOpDef peepsubs[] = {", output);
  for (i = 0; opdefs != NULL; i++, opdefs = opdefs->next) {
    if (i != 0) fputc(',', output);
    fprintf(output, "\n  /*%3d*/", i);
    WriteOpDef(opdefs->op, "", &nullusage);
  }
  if (i == 0) fputs("\n  {0, 0, 0}", output);
  fputs("\n};\n", output);
}

static PeepReplaceDef *replacelist[MaxReplace];
static int replacementcount;

static bool SameField(PeepField const *f1, PeepField const *f2) {
  if (f1->type != f2->type) return NO;
  switch (f1->type) {
  case pft_int:      return f1->f.val == f2->f.val;
  case pft_field:    return f1->f.f.inst == f2->f.f.inst &&
                            f1->f.f.field == f2->f.f.field;
  case pft_inst:     return f1->f.f.inst == f2->f.f.inst;
  case pft_exprn:    return f1->f.ex.i == f2->f.ex.i;
  default:           return YES;
  }
}

static PeepExprn *exlist;
static PeepExprn **exp;

static void NoteExprnsInField(PeepField *f) {
  if (f->type == pft_exprn) {
    PeepExprn *ex = f->f.ex.ex;
    NoteExprnsInField(&ex->f1);
    NoteExprnsInField(&ex->f2);
    { PeepExprn *p = exlist;
      int n = 0;
      for (; p != 0; p = p->next, n++) {
        if (p->op == ex->op &&
            p->fn == ex->fn &&
            SameField(&p->f1, &ex->f1) &&
            SameField(&p->f2, &ex->f2)) {
          f->f.ex.i = n;
          return;
        }
      }
      ex->next = NULL;
      *exp = ex;
      exp = &ex->next;
      f->f.ex.i = n;
    }
  }
}

static void WriteExprns(void) {
  int i = 0;
  fputs("static PeepExprn const peepexprns[] = {\n", output);
  for (; exlist != NULL; exlist = exlist->next, i++) {
    if (i != 0) fputs(",\n", output);
    fprintf(output, " /*%3d*/{%s, ", i, peo_name[exlist->op]);
    if (exlist->fn == NULL)
      fputs("0, ", output);
    else
      fprintf(output, "pep_%s, ", exlist->fn);
    fprintf(output, "%s, %s, ",
            ft_name[exlist->f1.type], ft_name[exlist->f2.type]);
    WriteField(&exlist->f1, ",");
    WriteField(&exlist->f2, "}");
  }
  if (i == 0) fputs("  {}", output);
  fputs("\n};\n\n", output);
}

static bool SameReplace(PeepReplace const *r1, PeepReplace const *r2) {
  return (r1->type == r2->type &&
          r1->f1.type == r2->f1.type &&
          r1->procno == r2->procno &&
          (r1->f1.type != pft_field || r1->f1.f.f.field == r2->f1.f.f.field) &&
          SameField(&r1->f2, &r2->f2));
}

static void WriteReplacements() {
  int count = MaxReplace;
  int index = 0;
  PeepReplace **replacements = (PeepReplace **)
      malloc(replacementcount * sizeof(PeepReplace *));
  fputs("static PeepReplace const replacements[] = {\n  /*  0*/", output);
  { int i, c;
    for (c = count; --c >= 0;) {
      PeepReplaceDef *prd = replacelist[c];
      for (; prd != NULL; prd = prd->next)
        for (i = 0; i < c; i++)
          NoteExprnsInField(&(prd->replace[i]->f2));
    }
  }
  for (; --count >= 0;) {
    PeepReplaceDef *prd = replacelist[count];
    for (; prd != NULL; prd = prd->next) {
      int r;
      if (count == 0) {
        prd->op->replace.i = 0;
        continue;
      }
      for (r = 0; r <= index - count; r++) {
        if (SameReplace(replacements[r], prd->replace[0])) {
          int i;
          for (i = 1; i < count; i++)
            if (!SameReplace(replacements[r+i], prd->replace[i]))
              break;
          if (i == count) {
            prd->op->replace.i = r;
            goto alreadypresent;
          }
        }
      }
      prd->op->replace.i = index;
      for (r = 0; r < prd->op->replacecount; r++, index++) {
        PeepReplace *pr = prd->replace[r];
        replacements[index] = pr;
        if (index != 0 || r != 0) fprintf(output, ",\n  /*%3d*/", index);
        if (pr->procno >= 0) {
          fprintf(output, "{%s, 0, 0, %d}", rt_name[pr->type], pr->procno);
        } else if (pr->f1.type != pft_field && pr->f2.type == pft_none) {
          fprintf(output, "{%s}", rt_name[pr->type]);
        } else {
          fprintf(output, "{%s, %s, %s, ",
                          rt_name[pr->type], field_name[pr->f1.f.f.field], ft_name[pr->f2.type]);
          WriteField(&pr->f2, "}");
        }
      }
alreadypresent:;
    }
  }
  fputs("\n};\n", output);
}

static void CollectReplacements(int n, PeepOp **ops) {
  for (; --n >= 0; ) {
    int count = ops[n]->replacecount;
    PeepReplaceDef *pr = ops[n]->replace.r;
    pr->next = replacelist[count];
    replacelist[count] = pr;
    replacementcount += count;
  }
}

static void WriteOps(int ix, int n, PeepOp * const *ops) {
  int i;
  fprintf(output, "static PeepOp const p%d_ops[] = {\n  {", ix);
  for (i = 0; i < n; i++) {
    PeepOp *op = ops[i];
    if (i != 0) fputs(",\n  {", output);
    WriteOpDef(&op->op, ", ", &op->maynot);
    if (op->dead == 0)
      fputc('0', output);
    else
      WriteDeadBits(op->dead, output);

    fprintf(output, ", %d, %d}", op->replacecount, op->replace.i);
  }
  fputs("};\n", output);
}

static void CheckArgCt(FnArgList *p, char const *type) {
  for (; p != NULL; p = p->next)
    fprintf(output, "\
#if %s_argct_%s != %d\n # error wrong argument count for %s_%s\n#endif\n",
                    type, p->fn, p->args, type, p->fn);
}

static void NoteExprnsInConstraint(PeepConstraint *pc) {
  if (pc->f1.type == pft_constraint)
    NoteExprnsInConstraint(pc->f1.f.c.c);
  else
    NoteExprnsInField(&pc->f1);
  if (pc->f2.type == pft_constraint)
    NoteExprnsInConstraint(pc->f2.f.c.c);
  else
    NoteExprnsInField(&pc->f2);
}

static int CollectSubConstraints(PeepHole *p, PeepConstraint *pc, PeepConstraint **subc, int ix) {
  if (pc->f1.type == pft_constraint) {
    ix = CollectSubConstraints(p, pc->f1.f.c.c, subc, ix);
    subc[ix] = pc->f1.f.c.c;
    pc->f1.f.c.i = p->constraintcount+ix;
    ix++;
  }
  if (pc->f2.type == pft_constraint) {
    ix = CollectSubConstraints(p, pc->f2.f.c.c, subc, ix);
    subc[ix] = pc->f2.f.c.c;
    pc->f2.f.c.i = p->constraintcount+ix;
    ix++;
  }
  return ix;
}

static void WriteConstraint(PeepConstraint *pc, bool nl) {
  if (nl) fprintf(output, ",\n");
  fprintf(output, "  {%s, ", ct_name[pc->ctype]);
  if (pc->proc != NULL)
    fprintf(output, "pcp_%s, ", pc->proc);
  else
    fputs("0, ", output);
  fprintf(output, "%s, %s, ", ft_name[pc->f1.type], ft_name[pc->f2.type]);
  WriteField(&pc->f1, ", ");
  WriteField(&pc->f2, "}");
}

typedef struct IntList { struct IntList *next; int i; } IntList;
static struct { int count; IntList *p; } perop[J_LAST_A_JOPCODE+1];

static void WritePeepHoles(PeepHole *pl) {
  fputs("typedef int32 propproc(J_OPCODE);\n", output);
  fputs("static propproc * const peepprops[] = {\n", output);
  fputs(" a_loads_r1,\n a_uses_r1,\n a_uses_r2,\n a_uses_r3\n};\n", output);

  fputs("#define p_loads_r1 0\n#define p_uses_r1 1\n", output);
  fputs("#define p_uses_r2 2\n#define p_uses_r3 3\n", output);

  fputs("#define fb_(a, b) ((((int32)(a))<<8) | (b))\n", output);
  fputs("#define fh_(a, b) ((((int32)(a))<<16) | (b))\n", output);

  { PeepHole *p;
    PeepConstraint *subc[MaxConstraints];
    int i;
    exlist = 0; exp = &exlist;
    WriteOpTables(pl);
    replacementcount = 0;
    for (p = pl; p != NULL; p = p->next)
      for (i = 0; i < p->constraintcount; i++)
        NoteExprnsInConstraint(p->constraints[i]);

    for (p = pl; p != NULL; p = p->next)
      CollectReplacements(p->opcount, p->ops);

    WriteReplacements();
    WriteExprns();
    for (p = pl; p != NULL; p = p->next) {
      WriteOps(p->index, p->opcount, p->ops);
      if (p->constraintcount != 0) {
        int subc_ix = 0;
        for (i = 0; i < p->constraintcount; i++)
          subc_ix = CollectSubConstraints(p, p->constraints[i], subc, subc_ix);

        fprintf(output, "static PeepConstraint const c%d[] = {\n", p->index);
        for (i = 0; i < p->constraintcount; i++)
          WriteConstraint(p->constraints[i], i != 0);

        for (i = 0; i < subc_ix; i++)
          WriteConstraint(subc[i], YES);

        fputs("};\n", output);
      }
    }
  }
  fputs("static PeepHole const patterns[] = {\n", output);
  for (; pl != NULL; pl = pl->next) {
    fprintf(output, "  {&p%d_ops[0], ", pl->index);
    if (pl->constraintcount != 0)
      fprintf(output, "&c%d[0]", pl->index);
    else
      fputc('0', output);
    fprintf(output, ", %d, %d, %d, %s}", pl->opcount, pl->constraintcount, pl->trace,
                    pl->gapconstraint == NULL ? "0": pl->gapconstraint);
    if (pl->next != NULL) fputc(',', output);
    fputc('\n', output);
  }
  fprintf(output, "};\n\n#define PeepholeMax %d\n#define MaxInst %d\n", peephole_index, opMax);
  CheckArgCt(constraintfns, "pcp");
  CheckArgCt(exprnfns, "pep");
  { int i;
    char *b;
    for (i = 0; i <= J_LAST_A_JOPCODE; i++) {
      IntList *ip = perop[i].p;
      if (ip != NULL) {
        b = "";
        fprintf(output, "static int const po_%d[] = { /* %s */\n", i, JOPNAME(i));
        for (; ip != NULL; ip = ip->next) {
          fprintf(output, "%s\t%d", b, ip->i-1);
          b = ",\n";
        }
        fputs("\n};\n", output);
      }
    }
    fputs("\nstatic struct { int i; int const *peepv; } const peepholeperop[] = {\n", output);
    b = "";
    for (i = 0; i <= J_LAST_A_JOPCODE; i++) {
      fprintf(output, "%s\t{%d, ", b, perop[i].count);
      if (perop[i].count == 0)
        fputs("0", output);
      else
        fprintf(output, "&po_%d[0]", i);
      b = "},\n";
    }
    fputs("}\n};\n", output);
  }
}

static void ComputePeepholesPerOp(PeepHole *pl) {
  int i = 0;
  for (i = 0; i <= J_LAST_A_JOPCODE; i++) {
    IntList **ip = &perop[i].p;
    PeepHole *p;
    perop[i].count = 0;
    perop[i].p = 0;
    for (p = pl; p != NULL; p = p->next) {
      if (OpSatisfies(i, &(p->ops[0]->op))) {
        IntList *t = (IntList *) malloc(sizeof(*t));
        perop[i].count++;
        t->next = NULL; t->i = p->index;
        *ip = t;
        ip = &t->next;
      }
    }
  }
}

static void ArgErr(char const *mess, char const *arg) {
  fprintf(stderr, "%s%s\n", mess, arg);
  exit(1);
}

static char *AddHeapString(char const *s) {
  return NewHeapString(s, strlen(s));
}

int main(int argc, char **argv) {
  int ch = 0;
  PeepHole *pl = 0, **pp = &pl;
  int argno = 1;

  syms = NULL;
  debug = NULL;
  constraintfns = exprnfns = NULL;

  if (StrEq(argv[1], "-d") || StrEq(argv[1], "-D")) {
    debugbuf = (char *)malloc(DebugSize);
    debugix = 0;
    if (StrEq(argv[2], "-")) {
      debug = stdout;
    } else {
      debug = fopen(argv[2], "w");
      if (debug == NULL) ArgErr("can't write ", argv[2]);
    }
    argno = 3;
  }

  if (argc != (2 + argno)) ArgErr("Usage: peepgen <source> <destn>", "");

  if (StrEq(argv[argno], "-"))
    input = stdin;
  else {
    input = fopen(argv[argno], "r");
    if (input == NULL) ArgErr("can't read ", argv[argno]);
  }
  argno++;
  if (StrEq(argv[argno], "-")) {
    outputfile = 0;
    output = stdout;
  } else {
    outputfile = argv[argno];
    output = fopen(outputfile, "w");
    if (output == NULL) ArgErr("can't write ", argv[argno]);
  }

  pcp_regset_unused = AddHeapString("regset_unused");
  pcp_regset_unkilled = AddHeapString("regset_unkilled");
  pcp_nointervening = AddHeapString("nointervening");
  pep_bit = AddHeapString("bit");

  opMax = 0;
  InitProcDefList(&rproc);
  line_number = 1;
  bracket_nesting = 0;
  ch = NextCh();

  for (; ch != EOF ;) {
    PeepHole *ph = ReadPeepHole(&ch);
    if (ph == NULL) break;
    *pp = ph;
    pp = &ph->next;
  }

  { ProcDef *p;
    int i = 0;
    for (p  = rproc.list; p != 0; p = p->next) {
      int l;
      fprintf(output, "static bool rproc__%d(PendingOp *%s, PendingOp **ops, RegisterUsage *u)\n", i++, p->arg);
      for (l = 0; l < p->n; l++) fprintf(output, "%s", p->line[l]);
      fputc('\n', output);
    }
    fputs("static RProc * const rprocs[] = {\n", output);
    for (i = 0; i < rproc.count; i++) fprintf(output, "  rproc__%d,\n", i);
    fputs("  0\n};\n\n", output);
  }
  ComputePeepholesPerOp(pl);
  WritePeepHoles(pl);
  return 0;
}

