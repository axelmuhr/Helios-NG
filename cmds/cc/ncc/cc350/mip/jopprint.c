/*
 * file jopprint.c - things maybe used while debugging compiler, version 20
 * Copyright (C) Codemist Ltd., April 1986.
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/11/23 16:41:52 $
 * Revising $Author: nickc $
 */

#ifndef NO_VERSION_STRINGS
extern char jopprint_version[];
char jopprint_version[] = "\njopprint.c $Revision: 1.1 $ 20\n";
#endif

#ifdef __STDC__
#  include <string.h>
#  include <stdarg.h>
#  include <stdlib.h>
#else
#  include <strings.h>
#  include <varargs.h>
#endif
#include "globals.h"
#include "cgdefs.h"
#define DEFINE_JOPTABLE 1
#include "jopcode.h"
#include "aeops.h"
#include "aetree.h"    /* for pr_stringsegs */
#include "flowgraf.h"   /* for is_exit_label */

#if defined ENABLE_CG || defined ENABLE_REGS || defined ENABLE_CSE

static char *condition_name(int32 w)
/* only for print_opname() */
{
    char *op;
    switch(w)
    {
case Q_EQ:  op = "(if EQ)"; break;
case Q_NE:  op = "(if NE)"; break;
case Q_HS:  op = "(if HS)"; break;
case Q_LO:  op = "(if LO)"; break;
/* case Q_MI:  op = "(if MI)"; break; */
/* case Q_PL:  op = "(if PL)"; break; */
/* case Q_VS:  op = "(if VS)"; break; */
/* case Q_VC:  op = "(if VC)"; break; */
case Q_HI:  op = "(if HI)"; break;
case Q_LS:  op = "(if LS)"; break;
case Q_GE:  op = "(if GE)"; break;
case Q_LT:  op = "(if LT)"; break;
case Q_GT:  op = "(if GT)"; break;
case Q_LE:  op = "(if LE)"; break;
case Q_AL|J_UNSIGNED:                /* bits moved, rationalise */
case Q_AL:  op = "       "; break;
case Q_NOT: op = "(never)"; break;
  case Q_UEQ:  op = "(ifUEQ)"; break;
  case Q_UNE:  op = "(ifUNE)"; break;
  case Q_XXX:  op = "(ifXXX)"; break;
default:    op = "??????????";
            sprintf(op, "Q_%.5lx", (long)w);
            break;
    }
    return op;
}

extern void jopprint_opname(J_OPCODE o)
{
    char v[20];
    J_OPCODE o1;
#ifdef TARGET_HAS_SCALED_ADDRESSING
    o1 = o & ~(Q_MASK|J_SIGNED|J_UNSIGNED|J_DEADBITS|J_SHIFTMASK|J_NEGINDEX);
#else
    o1 = o & ~(Q_MASK|J_SIGNED|J_UNSIGNED|J_DEADBITS);
#endif
    strcpy(v, joptable[o1 & 0xfff].name);
    if (o & J_SIGNED) strcat(v, "s");
    if (o & J_UNSIGNED) strcat(v, "u");
#ifdef TARGET_HAS_SCALED_ADDRESSING
    if (o & J_NEGINDEX) strcat(v, "m");
    if (o & J_SHIFTMASK)
    {   int32 m = (o & J_SHIFTMASK) >> J_SHIFTPOS;
        if ((m & SHIFT_RIGHT) == 0) strcat(v, "<<");
        else if (m & SHIFT_ARITH) strcat(v, ">>");
        else strcat(v, ">>L");
        sprintf(v+strlen(v), "%ld", (long)(m & SHIFT_MASK));
    }
    cc_msg("%-12s", v);
#else
    cc_msg("%-8s", v);
#endif
}

static void pr_bindlist(BindList *p)
{
    int ch = '{';
    while (p!=NULL)
    { /* To prevent ncc going bang when compiled with pcc */
#ifdef __STDC__
        cc_msg("%c$b", ch, p->bindlistcar);
#else
        cc_msg("%c", ch);
#endif
        ch = ',', p = p->bindlistcdr;
    }
    cc_msg("%s", (ch=='{') ? "{}" : "}");
}

void print_jopcode_1(J_OPCODE op, VRegInt r1, VRegInt r2, VRegInt m)
{
        cc_msg("%8s", condition_name(op & Q_MASK));
        jopprint_opname(op);

        if (gap_r1(op)) cc_msg("-, ");
        else if (uses_r1(op) || pseudo_reads_r1(op))
        {   if (r1.r == GAP) cc_msg("-, ");
            else cc_msg("%ld, ", (long)regname_(r1.r));
        }
        else cc_msg("%ld, ", (long)r1.i);

        if (gap_r2(op)) cc_msg("-, ");
        else if (op==J_INFOLINE || op==J_COUNT)
            cc_msg("'%s', ", r2.str);
/* AM: I suspect the position of the 'P' for pure is wrong for some     */
        else if (op==J_CALLK || op==J_CALLR ||
                 op==J_TAILCALLK || op==J_TAILCALLR)
        {   cc_msg("%ld", r2.i & K_ARGWORDMASK);
            if (r2.i & K_STRUCTRET) cc_msg("[+]");  /* + implicit arg   */
            if (r2.i >> K_FPARG_SHIFT) cc_msg("f%ld", r2.i >> K_FPARG_SHIFT);
            if (r2.i & K_PURE) cc_msg("P");         /* pure             */
            if (r2.i & K_VACALL) cc_msg("...");     /* call vararg fn   */
            cc_msg(", ");
        }
        else if (pseudo_reads_r2(op) && r2.r == GAP) cc_msg("-, ");
        else cc_msg("%ld, ",
               (long)((uses_r2(op) || pseudo_reads_r2(op)) ?
                       regname_(r2.r) : r2.i));
        if (uses_stack(op) ||
            op==J_CALLK || op == J_TAILCALLK || op==J_CALL2KP ||
            op==J_ADCON || op == J_INIT || op == J_INITF || op == J_INITD)
        {   Binder *bb = m.b;
            if (h0_(bb) == s_identifier)
            /* To allow print_jopcode to be called from local cgs */
                cc_msg("$r", (Symstr *)bb);
            else
            {   cc_msg("$b", bb);
                if (bindstg_(bb) & bitofstg_(s_auto))
                {   VRegnum r3 = bindxx_(bb);
                    if (r3 != GAP) cc_msg(" [r%ld]", (long)regname_(r3));
                }
            }
        }
        else if (op==J_POP)             /* dying.                       */
        {   RegList *p = m.rl;
            int ch = '{';
            while (p!=NULL)
            {   cc_msg("%c%ld ", (int)ch, (long)regname_(p->rlcar));
                ch = ',';
                p = p->rlcdr;
            }
            cc_msg("%s", ch=='{' ? "{}" : "}");
        }
        else if (op==J_STRING)
            pr_stringsegs(m.s);
        else if (op==J_SETSPENV)
        {   pr_bindlist(m.bl);
            cc_msg(" from ");
            pr_bindlist(r2.bl);
        }
        else if (op==J_SETSPGOTO)
        {   cc_msg("L%ld from ", (long)lab_xname_(m.l));
            pr_bindlist(r2.bl);
        }
        else if (uses_r3(op))
        {   if (m.r == GAP) cc_msg("<**missing register**>");
            else cc_msg("%ld", (long)regname_(m.r));
        }
        else if ((op & ~Q_MASK)==J_B || op == J_BXX || op == J_LABEL)
            cc_msg("L%ld", (long)lab_xname_(m.l));
        else switch (op)
        {   case J_MOVDK: case J_CMPDK:
            case J_ADDDK: case J_SUBDK:
            case J_MULDK: case J_DIVDK:
            case J_MOVFK: case J_CMPFK:
            case J_ADDFK: case J_SUBFK:
            case J_MULFK: case J_DIVFK:
                cc_msg("%s", m.f->floatstr);
                break;
            default:
                cc_msg("%ld", (long)m.i);
                if (m.i > 1000 || m.i < -1000) cc_msg("  [%#lx]", (long)m.i);
                break;
        }
}

void print_jopcode(J_OPCODE op, VRegInt r1, VRegInt r2, VRegInt m)
{   cc_msg("        ");
    print_jopcode_1(op, r1, r2, m);
    cc_msg("\n");
    if (op == J_CASEBRANCH)
    {   LabelNumber **v = r2.lnn;
        int32 i, ncase = m.i;
        r1.r = r2.r = GAP;
        for (i=0; i<ncase; i++)
        {   m.l = v[i];
            print_jopcode(J_BXX, r1, r2, m);
        }
    }
}
#else
void print_jopcode(J_OPCODE op, VRegInt r1, VRegInt r2, VRegInt m)
{
    IGNORE(op); IGNORE(r1); IGNORE(r2); IGNORE(m);
}
#endif

#ifdef ENABLE_REGS
void print_xjopcode(J_OPCODE op, VRegInt r1, VRegInt r2, VRegInt m,
                    char *fmt, ...)
{   va_list ap;
    va_start(ap, fmt);
    print_jopcode_1(op, r1, r2, m);
    cc_msg(" ");
    _vfprintf(stderr, fmt, ap);
    cc_msg("\n");
    /* since this is only used by regalloc we do not need to print out   */
    /* branch tables of CASEBRANCH as it never calls it with such things */
}
#else
void print_xjopcode(J_OPCODE op, VRegInt r1, VRegInt r2, VRegInt m,
                    char *fmt, ...)
{
    IGNORE(op); IGNORE(r1); IGNORE(r2); IGNORE(m); IGNORE(fmt);
}
#endif

/* end of jopprint.c */
