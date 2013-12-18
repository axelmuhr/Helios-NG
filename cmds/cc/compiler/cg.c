/* 
 * Transputer code generator.
 *
 * Derived largely from the JOPCODE generator, this attempts to generate
 * code directly from the tree.
 *
 * 28 Aug 87: NHG : Started
 * October 87 James Cownie, T8 modifications, general tidying, changes to
 * the way assignments are generated (nothing that's used too often !).
 */
/* $Id: cg.c,v 1.5 1994/09/23 08:44:45 nickc Exp $ */

/* STILL TO DO 17/12/87
 *   Introduce a peepholer to remove some of the more gross code which we 
     generate at present and also fold up the fpload and operates.
     There are various other frigs which can be done at that point as well.
*/

#include "cchdr.h"
#include "AEops.h"
#include "util.h"
#include "xrefs.h"
#include "xpuops.h"
#include "cg.h"

#ifndef static
#define static
#endif

int max_icode, max_block;          /* statistics (lies, damn lies)   */
int ida,fda;
int current_stackdepth;
int greatest_stackdepth;                  /* needed for stack check code */
int max_argsize;
int procflags;
BindList *local_binders;
BindList *active_binders;
Binder *integer_binder;
bool has_main;

struct SwitchInfo switchinfo;

struct LoopInfo loopinfo;

LoopList *all_loops;          /* loopopt.c uses           */
bool in_loop;                 /* flowgraph.c inspects ... */
BlockList *this_loop;         /* ... and chains on this   */
bool loopseen;

int anylabels;         /* somewhat vestigial */


/* result_variable is an extra first arg. for use with structure returning  */
/* functions - private to cg_return and cg_topdecl                          */
static Binder *result_variable;

#ifdef cg_content_for_dot_
#undef cg_content_for_dot_
#endif

#define cg_content_for_dot_(x) \
   (mcrepofexpr(arg1_(x)) == 0x0000004 ? arg1_(x) : \
    mk_expr1(s_content, type_(x), take_address(x)))


/* forward references */

BindList *binderise();
VLocal *vlocal();
static VLocal *pushtemp();
static void poptemp();
void freetemp();
static int  inverseCompare();
static VLocal *allocatetemp();
extern bool open_compilable();
extern block_head *start_new_basic_block(); 
static void cg_cmd();
static void cg_exprvoid();
static void cg_expr();
static void cg_expr1();
static void cg_cast1();
static void cg_return();
static void cg_bindlist();
static void set_Arg();
static void set_VLocal();
static void cg_binary();
static void verify_integer();
static void cg_bindargs();
BindList *sort_bindlist();

/*************************************************************************/
/*                 Start of codegeneration proper.                       */
/*************************************************************************/

#ifdef unsigned_expression_
#undef unsigned_expression_
#endif

#define unsigned_expression_(x) ((mcrepofexpr(x)&0xff000000)==0x01000000)

static BindList *datasegbinders;

#define cg_loadzero(e) cg_loadconst(0,e)

int result2;

int ssp;				/* simulated stack pointer	*/
int maxssp;				/* max value thereof		*/
int vsp;				/* vector stack size		*/
int maxvsp;				/* max vector stack usage	*/
int margin;				/* low workspace offset margin	*/
int maxcall;				/* maximum call margin		*/

VLocal *local_chain;			/* chain of all VLocals in proc */

extern int nextrefs;			/* number of external refs	*/
extern int nstatrefs;			/* number of static references	*/
extern int vstackused;			/* true if vstack is used	*/
extern int real32used;
extern int real64used;
extern int needlitpool;

int addrexpr;				/* true if in address expression */

VLocal *modtab;				/* local containing module table */
VLocal *owndata;			/* local containing own data module */
VLocal *real32op;			/* local containing ptr to real32op */
VLocal *real64op;			/* local containing ptr to real64op */
VLocal *litpool;			/* local containing ptr to constants*/

#ifdef NEVER
int floatingHardware = FALSE;		/* default is to allow only those   */
int dupAllowed	= FALSE;		/* instructions which work on all   */
int popAllowed = FALSE;			/* transputers.			    */
#endif

VLocal *doubledest;			/* intermediate dest for fpsim	*/

static int invalof = 0;			/* counts nested valofs		*/

static void topdec_init()
{
    suspend_local_store();
    codebuf_reinit2();

    local_binders = NULL;

    active_binders = NULL;

    loopinfo.breaklab = loopinfo.contlab = NOTINLOOP;
    switchinfo.endcaselab = switchinfo.defaultlab = NOTINSWITCH;
    anylabels = loopseen = in_loop = FALSE;
    all_loops = NULL;
    this_loop = NULL;
    top_block = start_new_basic_block(nextlabel());
    procflags = 0;

    litpool = real32op = real64op = modtab = owndata = NULL;
    addrexpr = maxvsp = maxcall = vsp = ssp = maxssp = 0;
    local_chain = NULL;
    
    setInt( FullDepth );
    setFloat(FullDepth);
}

void cg_topdecl(x)
TopDecl *x;
{
    int savelinect = pp_linect;
    
    if (x==NULL) return;    /* Maybe result of previous error etc */
    switch (h0_(x))
    {

case s_fndef:
        {   Binder *b = x->v_f.fn.name;
            BindList *formals = x->v_f.fn.formals;
            Symstr *name = bindsym_(b);
            TypeExpr *t = prunetype(bindtype_(b)), *restype;
/* Object module generation may want to know if this module defines
   main() so that it can establish an entry point */
            if (name == mainsym &&
                (bindstg_(b) & bitofstg_(s_extern)) != 0) has_main = 1;
            if (h0_(t)!=t_fnap)
            {   syserr("type of name should be function in fndef node");
                return;
            }
            {   int w;
                restype = prunetype(typearg_(t));
/* N.B. use of special version of mcrepofexpr() here so that structure   */
/* results may (optionally) be treated more cautiously than other        */
/* structure values. Relevant in case of one-word integer-like structs.  */
                w = cautious_mcrepoftype(restype);
/* If a function returns a structure result I give it a hidden extra     */
/* first argument that will point to where the result must be dumped.    */
/* The same is done for any function which returns a double. This is 	 */
/* essential if we are simulating on a T4, but it is also done on a T8	 */
/* to preserve calling convention compatability. 			 */
                if( ((w & 0xff000000) == 0x03000000) || /* structure result */
		    		      (w==0x02000008) ) /* double result */
                {   result_variable = gentempbinder(ptrtotype_(restype));
                    formals = mkBindList(formals, result_variable);
                }
                else result_variable = NULL;
            }
            topdec_init();
	
/* Here I pre-parse the tree and extract things like the expression depths */
/* These are saved into a hash table keyed on the node address for later use*/

	    preparse(x);

/* Here is a nasty mess - 'formals' has SynAlloc store, which is clobbered */
/* on drop_local_store().  It needs copying for the one use below which    */
/* scans it to set PROC_ARGPUSH.                                           */
            formals = binderise(formals);
            if (dbgstream)
            {   /* for the basic block started by J_ENTER in cg_bindargs() */
                current_env = list2(0, formals);
            }
            cg_bindargs(formals, x->v_f.fn.ellipsis);
            if (profile_option) emitprofile();
#ifdef DBX
	    if( dump_info ) db_proc(b,formals,restype);
#endif
            cg_cmd(x->v_f.fn.body);

#ifdef never
            /* the following code is maybe what am would like */
            if (!deadcode && !equivtype(restype, te_void))
                cc_warn("implicit return in non-void %s()", _symname(name));
#endif
            cg_return(0, !equivtype(restype, te_void));
#ifdef never
/* Allow loop optimiser to collect adcons etc at top of function body.   */
            all_loops = mkLoopList(all_loops, this_loop, function_head);
#endif

#ifdef never
            phasename = "loopopt";
            {   BindList *invariant_binders = loop_invariants();
                /* Corrupt regvar_binders and local_binders to get */
                /* a spill_order list for allocate_registers()     */
                drop_local_store();   /* what loopopt used */
                allocate_registers(
                    (BindList *)nconc((List *)invariant_binders,
                        nconc((List *)local_binders, (List *)regvar_binders)));
                drop_local_store();   /* what regalloc used */
            }
#endif
            phasename = "machinecode";
#ifdef never
/* If (after register allocation etc) an argument is left active in      */
/* memory (rather than being slaved in a register) I will do the full    */
/* entry sequence. Force this by setting PROC_ARGPUSH in that case.      */
/* Note that PROC_ARGADDR will thereby imply PROC_ARGPUSH, but they are  */
/* different in that PROC_ARGADDR suppresses tail recursion (flowgraph.c)*/
/* Also if a big stack frame is needed I tell xxxgen.c to be cautious.   */
            if (greatest_stackdepth > 256) procflags |= PROC_BIGSTACK;
            while (formals != NULL)
                {   Binder *b = formals->bindlistcar;
                    if (bindxx_(b) == GAP) procflags |= PROC_ARGPUSH;
                    formals = formals->bindlistcdr;
                }

/* This is the place where I put tables into the output stream for use   */
/* by debugging tools - here are the names of functions.                 */
            if (feature & FEATURE_SAVENAME) codeseg_function_name(name);
            show_entry(name, bindstg_(b) & bitofstg_(s_static) ?
                             xr_code+xr_defloc : xr_code+xr_defext);
#endif
            linearize_code();

            gencode(name,bindstg_(b)&bitofstg_(s_static)?0:1);

            drop_local_store();	/* kill store used here */
            
            if (block_cur > max_block) max_block = block_cur;
            if (icode_cur > max_icode) max_icode = icode_cur;
        }
        break;

case s_decl:
        break;

default:syserr("unknown top level %d", h0_(x));
        break;
    }
    
   pp_linect = savelinect;	/* restore munged line count */
}

void cg_init()
{
    codebuf_init();
    datasegbinders = global_list2(0, datasegment);
    max_icode = 0; max_block = 0;
    has_main = FALSE;
}

void cg_tidy()
{   if (debugging(DEBUG_STORE))
        fprintf(stderr, "Max icode store %d, block heads %d bytes\n",
                max_icode, max_block);
}

/* One might worry here about functions that specify their formals with  */
/* a type such as char, short or float.  For language independence sem.c */
/* arranges that such things never occur by generating an explicit       */
/* narrowing assignment in this case.                                    */

static void cg_bindargs(x,ellipsis)
BindList *x; int ellipsis;
/* Ellipsis is true if this function was defined with a '...' at the end */
/* of the list of formal pars it should accept.                          */
/* This is not currently used */
{
    BindList *x1;
    int argn = 0;

    /* set ssp to offset of first arg */
    ssp = -link_area_size;
    for (x1=x; x1!=NULL; x1=x1->bindlistcdr)
    {   Binder *b = x1->bindlistcar;
        int len = mcrepofexpr((Expr *)b);
        int mode = (len >> 24) & 0xff;
        len &= 0x00ffffff;
/* Things in an argument list can only have s_auto storage class     */
        if (!(bindstg_(b) & bitofstg_(s_auto))) syserr("Non-auto arg!");
        set_Arg(b, mode, len, 1);
        argn += pad_to_word(len)/4;
    }
    max_argsize = argn*4;

    active_binders = NULL;
    current_stackdepth = greatest_stackdepth = 0;
    ssp = maxssp = 0;
#ifdef never
/* To help with procedure-invariant processing I establish an empty      */
/* block immediately after top_block.                                    */
    function_head = start_new_basic_block(nextlabel());
    this_loop = NULL;
    in_loop = 1;
    start_new_basic_block(nextlabel());
#endif

/* Here I decide whether we need to allocate slave locations for the 	*/
/* module table, vector stack and my own local data pointer.		*/
/* The values will be loaded up into the locals in the procedure entry	*/
/* sequence and will be used in the expansions of the code macros.	*/
	if( !use_vecstack ) vstackused = FALSE;
	if( nextrefs > 1 || vstackused ) 
		modtab = vlocal(8);
	if( nstatrefs > 1 ) owndata = vlocal(4);
	if( needlitpool ) litpool = vlocal(4);
	
	if( !floatingHardware && real64used ) real64op = vlocal(4);
	if( !floatingHardware && real32used ) real32op = vlocal(4);
}

/* code generation for commands ...                                      */

static void cg_cmd(x)
Cmd *x;
{
    AEop op;
    for (;;)
    {
        if (x==0) return;

        pp_linect = x->fileline.l;	/* ensure errors get correct line no */
        
       if( pp_linect < 0 || pp_linect >10000 ) 
       		trace("strange line number in cg %d",pp_linect);
       		
        emit(p_infoline, &x->fileline);

	push_trace("cg_cmd");
	
	if(debugging(DEBUG_CG)) trace("Cg_cmd: %x %d",x,h0_(x));

	setInt(FullDepth); setFloat(FullDepth);

        switch (op = h0_(x))
        {

default:    syserr("<odd cmd %d = %s>",op, symbol_name_(op));
            { pop_trace(); return; }

case s_endcase:    /* those 'break's meaning "exit switch" */
            if (switchinfo.endcaselab==NOTINSWITCH) syserr("cg_cmd(endcase)");
            else
                emit(f_j, switchinfo.endcaselab);
            { pop_trace(); return; }

case s_break:      /* those 'break's meaning "exit loop" */
            if (loopinfo.breaklab==NOTINLOOP) syserr("cg_cmd(break)");
            else
            {   if (loopinfo.breaklab == 0)
                    loopinfo.breaklab = nextlabel();
                emit(f_j, loopinfo.breaklab);
            }
            { pop_trace(); return; }

case s_continue:
            if (loopinfo.contlab==NOTINLOOP) syserr("cg_cmd(cont)");
            else
            {   if (loopinfo.contlab == 0)
                    loopinfo.contlab = nextlabel();
                emit(f_j, loopinfo.contlab);
            }
            { pop_trace(); return; }

case s_resultis:
case s_return:
            cg_count();
            cg_return(cmd1e_(x), 0);
            { pop_trace(); return; }

case s_do:
            cg_loop(0, 0, 0, cmd1c_(x), cmd2e_(x));
            { pop_trace(); return; }

case s_for: cg_loop(cmd1e_(x), cmd2e_(x), cmd3e_(x), cmd4c_(x), 0);
            { pop_trace(); return; }

case s_if:  {   LabelNumber *l1 = nextlabel();
                AvailList *adconsave;
                cg_test(cmd1e_(x), 0, l1);
                adconsave = adconlist;
                cg_cmd(cmd2c_(x));
                if ((x = cmd3c_(x))==0) start_new_basic_block(l1);
                else
                {   LabelNumber *l2 = nextlabel();
                    emit(f_j, l2);
                    start_new_basic_block(l1);
/*                    align_block();			/ * align else label */
                    adconlist = adconsave;
                    cg_cmd(x);
                    start_new_basic_block(l2);
                }
/*                align_block();				/ * align test end label */
                adconlist = adconsave;
            }
            { pop_trace(); return; }

case s_switch:
            switch (mcrepofexpr(cmd1e_(x)))
            {
        case 0x4:
        case 0x01000004:
                break;
        default:        /* I think that it has been cast to (int) already */
                syserr("Expression in switch statement must be integer type");
            }
            cg_expr(cmd1e_(x));
            cg_count();
            {   struct SwitchInfo oswitchinfo;
                int ncases = 0, i;
                CasePair *casevec;
                Cmd *c;
                int oanylabels = anylabels;
		VLocal *v = (VLocal *) NULL;

		oswitchinfo = switchinfo;
                switchinfo.endcaselab = nextlabel();
                switchinfo.binders = active_binders;
                switchinfo.defaultlab =
                    switch_default_(x) ? nextlabel() : switchinfo.endcaselab;
                anylabels = IN_SWITCH_BIT;
                for (c = switch_caselist_(x); c != 0; c = case_next_(c))
                    ncases++;                /* count cases */
/* n.b. SynAlloc is used in the next line -- the store will die very soon */
/* we should use a 'C' auto vector for small amounts here                 */
                casevec = SynAlloc(ncases*sizeof(casevec[0]));
                i = ncases;
                for (c = switch_caselist_(x); c != 0; c = case_next_(c))
                {   LabelNumber *ln = nextlabel();
                    if (h0_(c) != s_case) syserr("internal err caselist");
                    i--;    /* syn sorts them backwards */
                    casevec[i].caseval = evaluate(cmd1e_(c));
                    casevec[i].caselab = ln;
                    /* case_lab_(c) */ cmd4c_(c) = (Cmd *)ln;
                }
#ifdef never
		if( !floatingHardware ) v = pushtemp( INTREG );
#else
		v = pushtemp( INTREG );
#endif
                /* previous phases guarantee the cases are sorted by now */
                casebranch(v, casevec, ncases, switchinfo.defaultlab);
                if (v != (VLocal *)NULL)
                    freetemp(v);
                setInt(FullDepth); setFloat(FullDepth);
                cg_cmd(cmd2c_(x));
                start_new_basic_block(switchinfo.endcaselab);
                anylabels = oanylabels | (anylabels & SEEN_LAB_BIT);
                switchinfo = oswitchinfo;
            }
            { pop_trace(); return; }

case s_case:
            if (switchinfo.defaultlab==NOTINSWITCH) syserr("cg_cmd(case)");
            {   LabelNumber *l1 = case_lab_(x);
                if (l1 == NULL) syserr("Unset case_lab");
                cg_case_or_default(l1);
            }
            anylabels |= SEEN_CASE_BIT;
            x = cmd2c_(x);
            continue;

case s_default:
            if (switchinfo.defaultlab==NOTINSWITCH) syserr("cg_cmd(default)");
            cg_case_or_default(switchinfo.defaultlab);
            anylabels |= SEEN_CASE_BIT;
            x = cmd1c_(x);
            continue;

case s_colon:
            {   LabBind *labbinder = cmd1lab_(x);
                LabelNumber *ln = labbinder->labinternlab;
                if (ln == NULL) ln = labbinder->labinternlab = nextlabel();
                start_new_basic_block(ln);
            }
            anylabels |= SEEN_LAB_BIT;
            x = cmd2c_(x);
            cg_count();
            continue;

case s_goto:
            {   LabBind *labbinder = cmd1lab_(x);
                LabelNumber *l = labbinder->labinternlab;
                if (l == NULL) l = labbinder->labinternlab = nextlabel();
                if (labbinder->labuses & l_defined)  /* else bind.c err msg */
                { 
                  emit(f_j, l);
                }
            }
            { pop_trace(); return; }

case s_semicolon: /* This insignificant line is where all the 
                     real work on assignments etc gets performed ! */
            cg_exprvoid(cmd1e_(x));
            { pop_trace(); return; }

case s_block:
            {   BindList *sl = active_binders;
                int d = current_stackdepth;
		int savessp = ssp;
                CmdList *cl = cmdblk_cl_(x);
                BindList *bl = cmdblk_bl_(x);
#ifdef DBX
		if( dump_info ) db_blockstart(x);
#endif
                cg_bindlist(bl, 0);
                if (dbgstream)
                {   current_env = list2(current_env, binderise(bl));
                    /* Hmm, this code is in flux pro tem. but the idea is   */
                    /* that we have to put debug scope info into block      */
                    /* heads so that it cannot get deadcoded away (discuss) */
                    (void)start_new_basic_block(nextlabel());
                }
                while (cl!=NULL)
                {   cg_cmd(cmdcar_(cl));
                    cl = cdr_(cl);
                }
                if (dbgstream)
                {   current_env = current_env -> bllcdr;
                    (void)start_new_basic_block(nextlabel());
                }
#ifdef DBX
		if( dump_info ) db_blockend(x);
#endif
                active_binders = sl;
                current_stackdepth = d;
		ssp = savessp;
            }
            { pop_trace(); return; }
        }
    }
}

static int integer_constant(x)
Expr *x;
{
/* Test if x is an integer constant, and if it is leave its value in result2 */
    if (h0_(x)==s_integer)
    {   result2 = intval_(x);
        return TRUE;
    }
    return FALSE;
}

static void cg_exprvoid(x)
Expr *x;
{
    int saveida = ida;
    int savefda = fda;
    cg_expr1(x, FALSE);
    /* @@@ This will not work on the H1 */
    if( ida != saveida ) emit( f_opr, op_teststd );
    setInt(saveida); setFloat(savefda);
}

static void cg_expr(x)
Expr *x;
{
    cg_expr1(x, TRUE);
}

void cg_addrexpr(x)
Expr *x;
{
	int oldae = addrexpr;
	addrexpr = TRUE;
	cg_expr1(x, TRUE);
	addrexpr = oldae;
}

static void cg_diadvoid(x)
Expr *x;
{   
    int saveida = ida;
    int savefda = fda;
    cg_expr1(arg1_(x), FALSE);
    setInt(saveida); setFloat(savefda);
    cg_expr1(arg2_(x), FALSE);
    setInt(saveida); setFloat(savefda);
}

/* note that 'valneeded' and 'RegSort rsort' really tell similar stories */
/* Maybe a VOIDREG version of RegSort subsumes both                      */
static void cg_expr1(x,valneeded)
Expr *x; int valneeded;
{
    AEop op;
    int mclength = mcrepofexpr(x);
    int mcmode = (mclength>>24) & 0xff;
    int cautious_mcmode = (cautious_mcrepofexpr(x)>>24) & 0xff;
    RegSort rsort = (mclength &= 0x00ffffff,
	(mcmode!=2) ? INTREG : (mclength==4 ? FLTREG : DBLREG ));
    bool reverse = FALSE, negate = FALSE;

    push_trace("cg_expr1");
        
    if(debugging(DEBUG_CG))
    {  
       trace("cg_expr1 : %s mode %2x length %d", symbol_name_(h0_(x)), 
             mcmode, mclength); 
    }

/* The next line will not catch cases where loading a one-word struct    */
/* occurs improperly if that gets treated as an integer. I have to let   */
/* this case slip through since in places I really want to treat such    */
/* structs as ints. Anyway all I lose is an internal check that should   */
/* never really fail anyway.                                             */
    if (mcmode==3 && valneeded)
        syserr("Value of structure requested improperly");
    if (x == 0) { syserr("missing expr"); pop_trace(); return; }

    else switch (op = h0_(x))
    {

case s_binder:
            if (!valneeded && !isvolatile_expr(x)) { pop_trace(); return; }
            cg_var((Binder *)x, s_content, mcmode, mclength, valneeded);
            pop_trace(); return;

case s_integer:
            if (!valneeded) { pop_trace(); return; }
            emit(f_ldc, intval_(x));
	    pushInt();
            pop_trace(); return;

case s_floatcon:
            if (!valneeded) { pop_trace(); return; }
            emitFpConst( x, rsort );
            pop_trace(); return;

case s_string:
            if (!valneeded) { pop_trace(); return; }
            emitstring(((String *)x)->strseg);
	    pushInt();
            pop_trace(); return;

case s_valof:
            {   
		int savessp = ssp;
		invalof++;
                cg_cmd((Cmd *)arg1_(x));
                invalof--;
		ssp = savessp;
            }
	    pop_trace(); return;
	    	    
case s_let:
            {   
                int d = current_stackdepth;
		int savessp = ssp;
                cg_bindlist((BindList *)arg1_(x), 0);
                cg_expr1(arg2_(x), valneeded);
                current_stackdepth = d;
		ssp = savessp;
            }
            pop_trace(); return;
case s_fnap:
            if( (cautious_mcmode==3) )
/* In a void context I am allowed to call a function that returns a      */
/* structure value. Here I have to make room on that stack for that      */
/* value to get dumped.                                                  */
/* NHG: also do this if the result is double in fpsim.			 */
            {   TypeExpr *t = type_(x);
                Binder *gen = gentempbinder(t);
                bindstg_(gen) |= b_addrof;
                x = mk_expr2(s_let,
                             t,
                             mkBindList(0, gen),
                             mk_expr2(s_comma,
                                      t,
                                      mk_expr2(s_fnap,
                                               te_void,
                                               arg1_(x),
                                               mkExprList(exprfnargs_(x),
                                                  take_address((Expr *)gen))),
/* This last item in the comma expression is only useful if the value of */
/* this (structure) function call is required. This would not normally   */
/* be a possibility, since structure-values can only occur in rather     */
/* special places and in general cg_expr() is not allowed to load one.   */
/* But one-word structure returning functions being called to provide    */
/* arguments for other functions can drop me into here on account of the */
/* punning between these structures and integer values.                  */
                                      (Expr *)gen));
                if (debugging(DEBUG_CG))
                {   eprintf("Structure fn call -> ");
                    pr_expr(x);
                }
		pp_expr(x,valneeded);
                cg_expr1(x, valneeded);
		pop_trace(); return;
            }
            else if(mcrepofexpr(x) == 0x02000008)
/* If the function returns a double, we must                               */
/* do the same as above, and supply a double argument for the result to go */
/* into. If there is already a doubledest set up we use that.		   */
/* This is done even on a T800 where in theory the result could be returned*/
/* in FAreg. This is not done in order to preserve call conventions. 	   */
            {   TypeExpr *t = type_(x);
                Binder *gen = gentempbinder(t);
                bindstg_(gen) |= b_addrof;
		if( doubledest == NULL )
		{
                     x = mk_expr2(s_let,
                             t,
                             mkBindList(0, gen),
                             mk_expr2(s_comma,
                                      t,
                                      mk_expr2(s_fnap,
                                               te_void,
                                               arg1_(x),
                                               mkExprList(exprfnargs_(x),
                                                  take_address((Expr *)gen))),
/* On a T800 this final item will cause the result to be loaded into FAreg */
/* which is where it will be most useful.				   */
                                      (Expr *)gen));
		}
		else {	/* evaluate into existing destination 		   */
			if( doubledest->len == 2 )
			{
			    gen->bindmcrep = 0x02000008;
			    bindxx_(gen) = (int) doubledest;
                            x = mk_expr2(s_comma,
                                      t,
                                      mk_expr2(s_fnap,
                                               te_void,
                                               arg1_(x),
                                               mkExprList(exprfnargs_(x),
                                                   take_address((Expr *)gen))),
                                      (Expr *)gen);
			}
			else {
			    TypeExpr *pt = ptrtotype_(t);
			    gen->bindmcrep = 4;
			    bindxx_(gen) = (int) doubledest;
			    bindtype_(gen) = pt;
                            x = mk_expr2(s_comma,
                                      pt,
                                      mk_expr2(s_fnap,
                                               te_void,
                                               arg1_(x),
                                               mkExprList(exprfnargs_(x),
                                                          (Expr *)gen)),
					       (Expr *)gen);
			}
		}
                if (debugging(DEBUG_CG))
                {   eprintf("Double fn call -> ");
                    pr_expr(x);
                }
		pp_expr(x,valneeded);
                cg_expr1(x, valneeded);
		pop_trace(); return;
            }
/* Some functions have special rules re compilation. open_compilable()   */
/* returns FALSE if it is handed anything other than one of these.       */
            if( ! open_compilable(x, mcmode, mclength) ) 
            		cg_fnap(x, rsort, valneeded);
	    pop_trace(); return;
case s_cond:
            {   LabelNumber *l3 = nextlabel();
                AvailList *adconsave = adconlist;
/* Previous phases of the compiler must have arranged that the two arms  */
/* of the condition each have the same mode. In particular they must     */
/* either be both integer or both floating point values. This has to     */
/* include the possibility of them being voided. Structure values are    */
/* only legal here if the conditional expression as a whole is voided.   */
                cg_cond(x,valneeded,l3,FALSE,TRUE);
                start_new_basic_block(l3);
		emitcjfix(); popInt();
                adconlist = adconsave;
                pop_trace(); return;
            }

case s_cast:cg_cast1(arg1_(x), mclength, cautious_mcmode);
            pop_trace(); return;
case s_addrof:
            cg_addr(arg1_(x), valneeded);
            pop_trace(); return;
case s_boolnot:
            if (!valneeded) 
            {
                cg_exprvoid(arg1_(x));
	    }
	    else if ( isrelational_( h0_( arg1_(x) ) ))
	    {
	       h0_(arg1_(x)) = inverseCompare( h0_( arg1_(x)));
	       cg_expr(arg1_(x));
	    }
	    else
	    {
	       cg_expr(arg1_(x));
	       emit( f_eqc, 0);
	    }
	    pop_trace(); return;

case s_notequal:
            negate = TRUE; 
            op = s_equalequal;     
case s_equalequal:
            if (!valneeded)
            {
               cg_diadvoid(x);
               pop_trace(); return;
            } 
            /* Check for an integer constant and use eqc if found */
            if (rsort == INTREG)
            {
              if( integer_constant(arg1_(x)) && smallint(result2))
	      { 
	          int r2=result2; 
	          cg_expr(arg2_(x)); 
	          emit( f_eqc, r2 ); 
	      }
	      else if( integer_constant(arg2_(x)) && smallint(result2))
	      { 
	         int r2=result2; 
	         cg_expr(arg1_(x)); 
	         emit( f_eqc, r2 ); 
	      }
	      else goto gencmp;

              if (negate)
                 emit( f_eqc, 0);
              pop_trace(); return;
            }         
case s_greater:
            goto gencmp;
            
case s_greaterequal:
            negate = TRUE;
case s_less:
            reverse = TRUE;
            op = s_greater;
            goto gencmp;
case s_lessequal:
            op = s_greater;
            negate = TRUE;
gencmp:
            if (!valneeded) 
            {
                cg_diadvoid(x);
	        pop_trace(); return;
	    }
	    else
	    {
	    	Expr *a1;
	    	Expr *a2;
	    	
	    	if( reverse ) 	a2 = arg1_(x), a1 = arg2_(x);
	    	else 		a1 = arg1_(x), a2 = arg2_(x);
		
	    	if( !floatingHardware && mcrepofexpr(a1) == 0x02000008 )
			cg_doublecmp(op, a1, a2);
		else 
			cg_binary(op, a1, a2, op == s_equalequal,rsort);

	       if (negate)
	          emit(f_eqc, 0);
	          
	    }
	    pop_trace(); return;   
	         

case s_andand:
case s_oror:
#ifdef JIMS
            { /* the ONE case where cj does the right thing ! */
               LabelNumber *l = nextlabel();
               
               cg_test1(x, op == s_oror, l, TRUE);
               start_new_basic_block(l);
               pop_trace(); return;
            }
#else
	/* The valuep stuff in cg_test1 does not work, back off to original
	   code.
	*/
            x = mk_expr3(s_cond, te_int,
                                 x,
                                 mkintconst(te_int, 1, 0),
                                 mkintconst(te_int, 0, 0));
            cg_expr(x); pop_trace(); return;

#endif
case s_comma:
            cg_exprvoid(arg1_(x));
            cg_expr1(arg2_(x), valneeded);
	    pop_trace(); return;
	    
case s_assign:
            if (cautious_mcmode==3)
            { /* Structure */  
                if (mcmode==0)
                { /* treated as an integer */
                    cg_scalarAssign(x,valneeded);
                    pop_trace(); return;
                }
/* The value of a structure assignment may be required in the case that  */
/* (a=b) occurs as an arg to a function and the structure a is one word. */
/* this should be handled above ..., so this should never occur !!!      */
                else if (valneeded)
                    syserr("value of structure assignment needed");
                else
                {   structure_assign(arg1_(x), arg2_(x), mclength);
                    pop_trace(); return;
                }
            }
case s_displace:
	    cg_scalarAssign(x,valneeded);
	    pop_trace(); return;

case s_content: /* These are now only the ones where we're NOT storing */
            cg_indirect( arg1_(x), s_content, mcmode, mclength, valneeded);
            pop_trace(); return;
	    
case s_dot:
/* This case is NASTY if arg1 might be computed by a function call. I    */
/* detect that case and deal with it in a somewhat messy way. I hope the */
/* case is so infrequent that performance is not critical.               */
/* At present     (f().field1).field2     generates especially silly     */
/* code - I may want to improve on it later.                             */
        if (structure_function_value(arg1_(x)))
        {   TypeExpr *t = typeofexpr(arg1_(x));
            Binder *gen = gentempbinder(t);
            Expr *xx = mk_exprwdot(s_dot,type_(x),(Expr *)gen,exprdotoff_(x));
            x = mk_expr2(s_let,
                         type_(x),
                         mkBindList(0, gen),
                         mk_expr2(s_comma,
                                  type_(x),
                                  mk_expr2(s_assign,
                                           t,
                                           (Expr *)gen,
                                           arg1_(x)),
                                  cg_content_for_dot_(xx)));
        }
        else x = cg_content_for_dot_(x);
	pp_expr(x, valneeded);
        cg_expr1(x, valneeded);
        pop_trace(); return;
        

case s_monplus:
/* Monadic plus does not have to generate any code                       */
            cg_expr1(arg1_(x), valneeded);
            pop_trace(); return;
case s_neg:
            if (!valneeded) 
            {
                cg_exprvoid(arg1_(x));
                pop_trace(); return;
            }
	    cg_expr(arg1_(x));
	    if( rsort == INTREG )
	    {	/* invert and add 1 */
		emit(f_opr,op_not);
		emit(f_adc, 1);
	    }
            else 
            {
		if( floatingHardware )
		{
			emit(f_opr,rsort==FLTREG ? op_fpldzerosn: op_fpldzerodb);
			emit(f_opr,op_fprev);
			emit(f_opr,op_fpsub);
		}
		else {
			emitFpConst( fc_zero, rsort );
			emit( f_opr, op_rev );
			codeFpCall( s_minus, mcrepofexpr(arg1_(x)) );
		}
            }
            pop_trace(); return;

case s_bitnot:
            if (!valneeded) 
            {
                cg_exprvoid(arg1_(x));
                pop_trace(); return;
            }
            verify_integer(x);
            cg_expr(arg1_(x));
            emit(f_opr,op_not);
            pop_trace(); return;

case s_times:
        if (!valneeded) 
        {
            cg_diadvoid(x);
            pop_trace(); return;
        }
        else if (rsort != INTREG)
        {   cg_binary(s_times, arg1_(x), arg2_(x), TRUE, rsort) ;
            pop_trace(); return;
        }
        if (iszero(arg1_(x))) 
        {
            cg_loadzero(arg2_(x));
            pop_trace(); return;
        }
        if (iszero(arg2_(x))) 
        {
            cg_loadzero(arg1_(x));
            pop_trace(); return;
        }
        else if (isone(arg1_(x))) 
        {
            cg_expr(arg2_(x));
            pop_trace(); return;
        }
        else if (isone(arg2_(x))) 
        {
            cg_expr(arg1_(x));
            pop_trace(); return;
        }
        else if (isminusone(arg1_(x)))
        {   arg1_(x) = arg2_(x);
            h0_(x) = s_neg;
            cg_expr(x);
            pop_trace(); return;
        }
        else if (isminusone(arg2_(x)))
        {   h0_(x) = s_neg;
            cg_expr(x);
            pop_trace(); return;
        }
        else
        {   
            Expr *a1 = arg1_(x);
            Expr *a2 = arg2_(x);
            /* for address expressions, make sure any constant is	*/
            /* second arg so prod will work.				*/
            if( addrexpr && integer_constant(a1) )
                 cg_binary(s_times, a2, a1, TRUE, INTREG);
            else cg_binary(s_times, a1, a2, TRUE, INTREG);
            pop_trace(); return;
        }

case s_plus:
        if (!valneeded) 
        {
            cg_diadvoid(x);
            pop_trace(); return;
        }
        else if (rsort != INTREG)
        {
	    cg_binary(s_plus, arg1_(x), arg2_(x), TRUE, rsort);
	    pop_trace(); return;
	}
	else if (iszero(arg1_(x))) 
        {
            cg_expr(arg2_(x));
            pop_trace(); return;
        }
        else if (iszero(arg2_(x))) 
        {
            cg_expr(arg1_(x));
            pop_trace(); return;
        }
	else if( integer_constant(arg1_(x)) && smallint(result2) )
	{ int r2=result2; cg_expr(arg2_(x)); emit( f_adc, r2 ); pop_trace(); return; }
	else if( integer_constant(arg2_(x)) && smallint(result2) )
	{ int r2=result2; cg_expr(arg1_(x)); emit( f_adc, r2 ); pop_trace(); return; }
        else 
        {
            cg_binary(s_plus, arg1_(x), arg2_(x), TRUE, INTREG);
            pop_trace(); return;
        }

case s_minus:
        if (!valneeded) 
        {  cg_diadvoid(x);
           pop_trace(); return;
        }
        else if (rsort != INTREG)
        {   cg_binary( s_minus, arg1_(x), arg2_(x), FALSE, rsort);
            pop_trace(); return;
        }
        if (iszero(arg1_(x)))
        {   arg1_(x) = arg2_(x);
            h0_(x) = s_neg;
            cg_expr(x);
            pop_trace(); return;
        }
        else if (iszero(arg2_(x))) 
        {   cg_expr(arg1_(x));
            pop_trace(); return;
        }
	else if( integer_constant(arg2_(x)) && smallint(result2))
	{ int r2=result2; cg_expr(arg1_(x)); emit( f_adc, -r2 ); pop_trace(); return; }
        else 
        {   cg_binary(s_minus, arg1_(x), arg2_(x), FALSE, INTREG);
            pop_trace(); return;
        }

case s_div:
/* Even if voiding this I calculate it in case there is a division error */
/* that ought to be reported.                                            */
/* But I arrange that the numerator is voided in this odd case, since    */
/* that can save me some effort.                                         */
        if (!valneeded)
        {   cg_exprvoid(arg1_(x));
            x = mk_expr2(s_div, type_(x), mkintconst(te_int, 1, 0), arg2_(x));
	    pp_expr(x,0);
        }
        if (iszero(arg1_(x))) 
        {   cg_loadzero(arg2_(x));
            pop_trace(); return;
        }
        else if (rsort != INTREG)
        {
            cg_binary(s_div, arg1_(x), arg2_(x), FALSE, rsort);
            pop_trace(); return;
        }
        else if (isone(arg2_(x))) 
        {   cg_expr(arg1_(x));
            pop_trace(); return;
        }
/* can't the unsignedness property get in rsort? */
        else if (mcmode==1)
	{
		VLocal *v1 = NULL;
		if( ida < 3 ) v1 = pushtemp( INTREG );

		if( idepth(depth(arg2_(x))) == 1 )
		{
			cg_expr(arg1_(x));
			emit( f_ldc, 0 ); pushInt();
			emit(f_opr, op_rev );
			cg_expr(arg2_(x));
			emit(f_opr, op_ldiv );
		}
		else {
			VLocal *v; 
			cg_expr( arg2_(x) );
			v = pushtemp( INTREG );
			cg_expr(arg1_(x));
			emit( f_ldc, 0 ); pushInt();
			emit(f_opr, op_rev );
			poptemp( v, INTREG );
			emit(f_opr, op_ldiv );
		}
		setInt( ida+2 );
		if( v1 != NULL )
		{
			poptemp( v1, INTREG );
			emit(f_opr, op_rev );
		}
		pop_trace(); return;
	}

        else
        {
            if (isminusone(arg2_(x)))  /* if !TARGET_HAS_NONFORTRAN_DIVIDE? */
            {   h0_(x) = s_neg;
                cg_expr(x);
                pop_trace(); return;
            }
            cg_binary(s_div, arg1_(x), arg2_(x), FALSE, INTREG);
            pop_trace(); return;
        }

case s_rem:
        if (rsort != INTREG) syserr("Float %%"); /* verify_integer? */
        else if (iszero(arg1_(x))) 
        {
           cg_loadzero(arg2_(x));
           pop_trace(); return;
        }
        else if (isone(arg2_(x))) 
        {
           cg_loadzero(arg1_(x));
           pop_trace(); return;
        }
/* can't the unsignedness property get in rsort? */
        else if (mcmode==1)
        {   int p;
            if ((p = ispoweroftwo(arg2_(x))) != 0)
            {    cg_binary(s_and, arg1_(x),
                                 mkintconst(te_int,(1<<p)-1,0), FALSE, INTREG);
                 pop_trace(); return;
             }
             {
		VLocal *v1 = NULL;
		if( ida < 3 ) v1 = pushtemp( INTREG );

		if( idepth(depth(arg2_(x))) == 1 )
		{
			cg_expr(arg1_(x));
			emit( f_ldc, 0 ); pushInt();
			emit(f_opr, op_rev );
			cg_expr(arg2_(x));
		}
		else {
			VLocal *v; 
			cg_expr( arg2_(x) );
			v = pushtemp( INTREG );
			cg_expr(arg1_(x));
			emit( f_ldc, 0 ); pushInt();
			emit(f_opr, op_rev );
			poptemp( v, INTREG );
		}
		emit(f_opr, op_ldiv );popInt();
		emit(f_opr, op_rev );
		popInt(); /* Cos we're not interested in the other bit */
		if( v1 != NULL ) 
		{
			poptemp( v1, INTREG );
			emit(f_opr, op_rev );
		}
		pop_trace(); return;
	}
        }
        else
        {
            if (isminusone(arg2_(x)))
            {   cg_loadzero(arg1_(x));   /* required by s_div defn */
                pop_trace(); return;
            }
            cg_binary(s_rem, arg1_(x), arg2_(x), FALSE , INTREG);
            pop_trace(); return;
        }

case s_leftshift:
        if (!valneeded) 
        {
            cg_diadvoid(x);
            pop_trace(); return;
        }
        verify_integer(x);
        if (iszero(arg2_(x))) 
        {   cg_expr(arg1_(x));
            pop_trace(); return;
        }
        else if (iszero(arg1_(x))) 
        {   cg_loadzero(arg2_(x));
            pop_trace(); return;
        }
        else 
        {   cg_binary( s_leftshift, arg1_(x), arg2_(x), FALSE, INTREG);
            pop_trace(); return;
        }

case s_rightshift:
        if (!valneeded) 
        {   cg_diadvoid(x);
            pop_trace(); return;
        }
        verify_integer(x);
        if (iszero(arg2_(x))) 
        {   cg_expr(arg1_(x));
            pop_trace(); return;
        }
        else if (iszero(arg1_(x))) 
        {   cg_loadzero(arg2_(x));
            pop_trace(); return;
        }
/* Note that for right shifts I need to generate different code for     */
/* signed and unsigned operations.                                      */
        else if( mcmode == 1 ) 
	/* unsigned shift						*/
	{   cg_binary(s_rightshift, arg1_(x), arg2_(x), FALSE, INTREG);
	    pop_trace(); return;
	}
	else {
	/* signed shift must be done as a double length operation (yuk)	*/
	/* NO ... FIX LATER ... Generate an unsigned shift, then use    */
	/* XWORD to propagate the sign bit back (At least for constant  */
	/* shift counts)                                                */
		VLocal *v1 = NULL;
		if( ida < 3 ) v1 = pushtemp( INTREG );

		if( idepth(depth(arg2_(x))) == 1 )
		{
			cg_expr(arg1_(x));
			emit(f_opr, op_xdble ); pushInt();
			cg_expr(arg2_(x));
			emit(f_opr, op_lshr ); popInt();popInt();
		}
		else {
			VLocal *v; 
			cg_expr( arg2_(x) );
			v = pushtemp( INTREG );
			cg_expr(arg1_(x));
			emit(f_opr, op_xdble ); pushInt();
			poptemp( v, INTREG );
			emit(f_opr, op_lshr ); popInt();popInt();
		}
		if( v1 != NULL )
		{
			poptemp( v1, INTREG );
			emit(f_opr,op_rev);
		}
		pop_trace(); return;
	}

case s_and:
        if (!valneeded) { cg_diadvoid(x); pop_trace(); return; }
        verify_integer(x);
        if (iszero(arg1_(x))) 
        {   cg_loadzero(arg2_(x));
            pop_trace(); return;
        }
        if (iszero(arg2_(x))) 
        {   cg_loadzero(arg1_(x));
            pop_trace(); return;
        }
        else if (isminusone(arg1_(x))) 
        {   cg_expr(arg2_(x));
            pop_trace(); return;
        }
        else if (isminusone(arg2_(x))) 
        {   cg_expr(arg1_(x));
            pop_trace(); return;
        }
        else 
        {   cg_binary(s_and, arg1_(x), arg2_(x), TRUE, INTREG);
            pop_trace(); return;
        }

case s_or:
        if (!valneeded) 
        {   cg_diadvoid(x);
            pop_trace(); return;
        }
        verify_integer(x);
        if (iszero(arg1_(x))) 
        {   cg_expr(arg2_(x));
            pop_trace(); return;
        }
        if (iszero(arg2_(x))) 
        {   cg_expr(arg1_(x));
            pop_trace(); return;
        }
        if (isminusone(arg1_(x))) 
        {   cg_loadconst(-1, arg2_(x));
            pop_trace(); return;
        }
        if (isminusone(arg2_(x))) 
        {   cg_loadconst(-1, arg1_(x));
            pop_trace(); return;
        }
        
        cg_binary(s_or, arg1_(x), arg2_(x), TRUE, INTREG);
        pop_trace(); return;

case s_xor:
        if (!valneeded)
        {   cg_diadvoid(x);
            pop_trace(); return;
        }
        verify_integer(x);
        if (iszero(arg1_(x)))
        {   cg_expr(arg2_(x));
            pop_trace(); return;
        }
        else if (iszero(arg2_(x)))
        {   cg_expr(arg1_(x));
            pop_trace(); return;
        }
        else if (isminusone(arg1_(x)))
        {   arg1_(x) = arg2_(x);
            h0_(x) = s_bitnot;
            cg_expr(x);
            pop_trace(); return;
        }
        else if (isminusone(arg2_(x)))
        {   h0_(x) = s_bitnot;
            cg_expr(x);
            pop_trace(); return;
        }
        else 
        {   cg_binary(s_xor, arg1_(x), arg2_(x), TRUE, INTREG);
            pop_trace(); return;
        }

default:
        syserr("cg_expr(%s)", symbol_name_(op));
        pop_trace(); return;
    }
}

static void cg_cast1(x1,mclength,mcmode)
Expr *x1; const int mclength; const int mcmode;
{
    int arglength = mcrepofexpr(x1);
    int argmode = (arglength>>24) & 0xff;
    RegSort rsort = (mcmode!=2) ? INTREG : mclength==4 ? FLTREG : DBLREG;
    arglength &= 0x00ffffff;

    while (h0_(x1)==s_cast)
/* The inner casts are irrelevant in the following cases:                */
/*     (float) (double) x                                                */
/*     (char) (short) x                                                  */
/*     (char) (int) x                                                    */
/*     (short) (int) x                                                   */
/* plus the corresponding unsigned cases.                                */
    {   if (mcmode==2 && argmode==2 && mclength==4 ||
            mcmode<2 && argmode<2 && mclength<arglength ||
            mcmode==argmode && mclength==arglength)
        {   arglength = mcrepofexpr(x1 = arg1_(x1));
            argmode = (arglength>>24) & 0xff;
            arglength &= 0x00ffffff;
            continue;
        }
        else break;
    }

    push_trace("cg_cast");
    
    if(debugging(DEBUG_CG)) 
	trace("Cg_cast: %x op %s mc mode %d l %d  arg mode %d l %d",
		x1,symbol_name_(h0_(x1)),mcmode,mclength,argmode,arglength);

    if (mclength==0) 
    {
       cg_exprvoid(x1);  /* cast to void */
       pop_trace(); return;
    }
    if (mcmode==3 || argmode==3)
    {   if (mcmode==argmode && mclength==arglength) { pop_trace(); return; }
        else syserr("Illegal cast involving a structure or union");
    }

    if (mcmode==argmode) switch(mcmode)
    {
case 0:     /* change width of integer */
        cg_expr(x1);
/*trace("mclen %d arglen %d ida %d",mclength,arglength,ida);*/
#ifdef NEWCAST
	/* values are kept in the evaluation stack as full-width values */
	/* hence widening casts can be ignored.				*/
	if( mclength >= arglength ) { pop_trace(); return; }
	
	/* a narrowing cast requires us to mask and sign extend from	*/
	/* the new width.						*/

	if( ida < 2 )
	{
		/* to sign extend we need 2 stack regs, make space and do it */
		VLocal *v;
		
		emit(f_opr, op_rev );
		v = pushtemp( INTREG );
		emit( f_ldc, (1<<(8*mclength))-1 );pushInt();
		emit(f_opr, op_and );popInt();
		emit( f_ldc, 1<<(8*mclength-1) );pushInt();
		emit(f_opr, op_xword );popInt();
		poptemp( v, INTREG );
		emit(f_opr, op_rev );
	}
	else {
		emit( f_ldc, (1<<(8*mclength))-1 );pushInt();
		emit(f_opr, op_and );popInt();
		emit( f_ldc, 1<<(8*mclength-1) );pushInt();
		emit(f_opr, op_xword );popInt();
	}
#else /* NEWCAST */
#if 0
        /* no need to do anything if widths are same, or source is a 	*/
        /* variable, since these are stored in 32-bit form anyhow.	*/
        if (mclength == arglength || h0_(x1) == s_binder ) return ;
#else
        if (mclength == arglength ) { pop_trace(); return; }
#endif
	/* because some cretin got the xword instruction wrong, we have	*/
	/* to both mask and shift to extend the sign correctly.		*/
        if( mclength > arglength )	/* widening to mclength, sign extend */
        {
#if 0
        	if( arglength == 2 ) { pop_trace(); return;}	/* hopefully already widened */
#endif
		if( ida < 2 )
		{
		/* to sign extend we need 2 stack regs, make space and do it */
			VLocal *v;
		
			emit(f_opr, op_rev );
			v = pushtemp( INTREG );
			emit( f_ldc, (1<<(8*arglength))-1 ); pushInt();
			emit(f_opr, op_and ); popInt();
			emit( f_ldc, 1<<(8*arglength-1) );pushInt();
			emit(f_opr, op_xword );	popInt();
			poptemp( v, INTREG );
			emit(f_opr, op_rev );
		}
		else {
			emit( f_ldc, (1<<(8*arglength))-1 ); pushInt();
			emit(f_opr, op_and ); popInt();
			emit( f_ldc, 1<<(8*arglength-1) );pushInt();
			emit(f_opr, op_xword );popInt();
		}	
        }
        /* else narrow it to the machine length, extend sign from top bit */
	else if( ida < 2 )
	{
	/* to sign extend we need 2 stack regs, make space and do it */
		VLocal *v;
		
		emit(f_opr, op_rev );
		v = pushtemp( INTREG );
		emit( f_ldc, (1<<(8*mclength))-1 );pushInt();
		emit(f_opr, op_and );popInt();
		emit( f_ldc, 1<<(8*mclength-1) );pushInt();
		emit(f_opr, op_xword );popInt();
		poptemp( v, INTREG );
		emit(f_opr, op_rev );
	}
	else {
		emit( f_ldc, (1<<(8*mclength))-1 );pushInt();
		emit(f_opr, op_and );popInt();
		emit( f_ldc, 1<<(8*mclength-1) );pushInt();
		emit(f_opr, op_xword );popInt();
	}
#endif /* NEWCAST */
        pop_trace(); return;
case 1:     /* change width of (unsigned) */
        cg_expr(x1);
#ifdef NEWCAST

	/* values are kept in the evaluation stack as full-width values */
	/* hence widening casts can be ignored.				*/
	if( mclength >= arglength ) { pop_trace(); return; }
	
	/* a narrowing cast requires us to mask down to the new width.	*/

	if( ida < 2 )		/* narrow to mclength by masking	*/
	{
		/* to mask we need 2 stack regs, make space and do it	*/
		VLocal *v;

		emit(f_opr, op_rev );
		v = pushtemp( INTREG );
		emit( f_ldc, (1<<(8*mclength))-1 ); pushInt();
		emit(f_opr, op_and ); popInt();
		poptemp( v, INTREG );
		emit(f_opr, op_rev );
	}
	else {
		emit( f_ldc, (1<<(8*mclength))-1 );pushInt();
		emit(f_opr, op_and );popInt();
	}
	
#else /* NEWCAST */
#if 0        
        /* no need to do anything if widths are same, or source is a 	*/
        /* variable, since these are stored in 32-bit form anyhow.	*/
        if (mclength == arglength || h0_(x1) == s_binder ) return ;
#else
        if (mclength == arglength ) { pop_trace(); return; }
#endif   
        if( mclength > arglength )	/* widen to mclength, mask top bits */
        {
#if 0
        	if( arglength == 2 ) { pop_trace(); return; }	/* hopefully already widened */        	
#endif
		if( ida < 2 )
		{
		/* to mask we need 2 stack regs, make space and do it */
			VLocal *v;

			emit(f_opr, op_rev );
			v = pushtemp( INTREG );
			emit( f_ldc, (1<<(8*arglength))-1 ); pushInt();
			emit(f_opr, op_and ); popInt();
			poptemp( v, INTREG );
			emit(f_opr, op_rev );
		}
		else {
			emit( f_ldc, (1<<(8*arglength))-1 );pushInt();
			emit(f_opr, op_and );popInt();
		}
        }
	else if( ida < 2 )	/* narrow to mclength by masking	*/
	{
		/* to mask we need 2 stack regs, make space and do it */
		VLocal *v;

		emit(f_opr, op_rev );
		v = pushtemp( INTREG );
		emit( f_ldc, (1<<(8*mclength))-1 ); pushInt();
		emit(f_opr, op_and ); popInt();
		poptemp( v, INTREG );
		emit(f_opr, op_rev );
	}
	else {
		emit( f_ldc, (1<<(8*mclength))-1 );pushInt();
		emit(f_opr, op_and );popInt();
	}
#endif /* NEWCAST */
        pop_trace(); return;

case 2:     /* change width of float */
	{

		if( mclength==arglength )
		{
		        cg_expr(x1);
			{ pop_trace(); return; }
		}
		if( floatingHardware )
		{
		        cg_expr(x1);
			if( mclength < arglength ) emit(f_opr, op_fpur64tor32 );
			else emit(f_opr, op_fpur32tor64 );
			pop_trace(); return;
		}
		else
		{
			if( mclength < arglength )
			{
				VLocal *proc = allocatetemp(INTREG);
				VLocal *olddd = doubledest;
				
				doubledest = allocatetemp(DBLREG);
				
				emit( p_ldx, fplib.real64toreal32 );
				emit( p_stvl, proc );
					
			        cg_expr(x1);
				emit( f_ldc, ROUND_NEAREST );
				emit( p_fpcall, proc );
	
				freetemp( doubledest );
				freetemp( proc );
				doubledest = olddd;
			}
			else {
				VLocal *proc = allocatetemp(INTREG);
				
				if( doubledest == NULL )
					syserr("real32 -> real64 with no dest");

				emit( p_ldx, fplib.real32toreal64 );
				emit( p_stvl, proc );
			
			        cg_expr(x1);
				doubleaddr(doubledest);
				emit( p_fpcall, proc );

				freetemp( proc );
				doubleaddr(doubledest);

			}
			setInt(FullDepth-1);
			{ pop_trace(); return; }
		}
	}
default:
        if (mclength!=arglength) 
            syserr("bad mode %d in cast expression", mcmode);
        cg_expr(x1); 
        { pop_trace(); return; }
    }
    else if (mcmode==2)
    {   /* floating something */

/* Earlier parts of the compiler ensure that it is only necessary to     */
/* cope with full 32-bit integral types here. Such things as (float) on  */
/* a character are dealt with as (float)(int)<char> with the inner cast  */
/* explicit in the parse tree.                                           */
/****** FLOATING HARDWARE CHECK ******/
        if (arglength!=4)
            syserr("cg_cast(float %d)", arglength);
        if (argmode != 0)    /* unsigned -> float - simulate with signed */
        {
            /* This is done by an ACN trick ...                          */
            /* xor source, minint                                        */
            /* -> double                                                 */
            /* add in the 2**31 again                                    */
            /* and if necessary round to single                          */
            /* How unpleasant !                                          */
	    if( floatingHardware )
	    {
		VLocal *v;
            	cg_expr(x1);             /* Load the argument */
#if 0
            	emit(f_opr, op_mint );pushInt();
            	emit(f_opr, op_xor  );popInt();
            	v = pushtemp( INTREG ); /* Ready to load and float */
            	emit( p_ldvlp, v); pushInt();
            	emit(f_opr, op_fpi32tor64 ); popInt(); pushFloat(); /* Loaded as a double */
            	freetemp(v);
            	emitFpConst( fc_two_31, DBLREG);
            	emit(f_opr, op_fpadd ); popFloat();
#else
            	v = pushtemp( INTREG );
            	emit( p_ldvlp, v); pushInt();
		emit( f_opr, op_fpb32tor64 ); popInt(); pushFloat();
            	freetemp(v);
#endif
            	if (mclength == 4) 
               		emit(f_opr, op_fpur64tor32 );
	    }
	    else {
		VLocal *v = NULL;
		VLocal *proc = allocatetemp(INTREG);

		if( doubledest == NULL )
		{
			if( mclength == 8 ) syserr("unsigned -> double without dest");
			v = doubledest = allocatetemp(DBLREG);	
		}
	
		emit( p_ldx, fplib.int32toreal64 );
		emit( p_stvl, proc );

            	cg_expr(x1);			/* Load the argument	*/
            	emit(f_opr, op_mint );		/* subtract 2**31	*/
            	emit(f_opr, op_xor  );
		doubleaddr(doubledest);
		emit( p_fpcall, proc );		/* convert to double	*/
            	emitFpConst( fc_two_31, DBLREG);
		doubleaddr(doubledest);
		codeFpCall( s_plus, 0x02000008 ); /* add in 2**31	*/

            	if (mclength == 4) 		/* convert to single if necc */
		{
			emit( p_ldx, fplib.real64toreal32 );
			emit( p_stvl, proc );
				
			emit( f_ldc, ROUND_NEAREST );
			emit( p_fpcall, proc );
		}
		/* in double case result is in doubledest */
		
		if( v ) freetemp(v), doubledest = NULL;

		freetemp(proc);

		setInt(FullDepth-1);
	    }
        }
	else
        {   /* Check to see whether the value we're converting */
            /* is already in store, in which case load its address */
            /* Else store it to a temp first */
	    if( floatingHardware )
	    {
            	if (instore( x1 )) 
               		cg_addr(x1, TRUE);
            	else
            	{
               		VLocal *temp;
               		cg_expr(x1);
               		temp = pushtemp(INTREG);
               		emit( p_ldvlp, temp); pushInt();
               		freetemp(temp); /* Actually after the next instruction ...*/
            	}
            	emit(f_opr,rsort==FLTREG ? op_fpi32tor32:op_fpi32tor64);pushFloat(); popInt();
	    }
	    else {
		if( rsort == FLTREG )
		{	/* signed int to single */
			VLocal *proc = allocatetemp(INTREG);
			emit( p_ldx, fplib.int32toreal32 );
			emit( p_stvl, proc );
			cg_expr(x1);
			emit( f_ldc, ROUND_NEAREST );
			emit( p_fpcall, proc );
			freetemp(proc);
		}
		else {	/* signed int to double */
			VLocal *dest = doubledest;
			VLocal *proc = allocatetemp(INTREG);

			if( dest == NULL )
				syserr("int32 -> double with no dest");

			emit( p_ldx, fplib.int32toreal64 );
			emit( p_stvl, proc );
			cg_expr(x1);
			doubleaddr(dest);
			emit( p_fpcall, proc );
			freetemp(proc);
			doubleaddr(dest);
		}
		setInt(FullDepth-1);
	    }
	}
        { pop_trace(); return; }
    }
    else if (argmode==2)
    {   /* fixing something THIS CODE IS NOT VERY CLEVER at generating
           int i = fix( xxx ) ; because it doesn't know that the target
           is really a store location, hence it will introduce a temp,
           and then load and store it... FIX LATER ...
        */
        VLocal *v = allocatetemp(INTREG);
        
/* N.B. the mclength==4 test in the next line is to produce shorter code */
/* for (unsigned short)(double)x.  It implies that this is calculated as */
/* (unsigned short)(int)(double)x.                                       */
        if (mcmode != 0 && mclength == 4)
        {
/* Fixing to an unsigned result is HORRIBLE, and is done using lots of   */
/* instructions here. The idea is to subtract 2**31 and fix, and then    */
/* to add back 2**31 to the resulting integer. We can cheat slightly here*/
/* by using round negative so that what we generate is                   */
/*     fixu(x) rounded to zero = fixs(x-2^31) round neg + 2^31           */     
/* The trick only works if I use double precision, (cos 2^31 is not      */
/* precisely representable in SP) so for float I start off by widening the*/
/* input.                                                                */
	    if( floatingHardware )
            {   
                cg_expr(x1);
                if (arglength==4)
                   emit(f_opr,op_fpur32tor64);
                emitFpConst(fc_two_31, DBLREG);
                emit(f_opr,op_fpadd); popFloat();
                emit( p_ldvlp, v); pushInt();
                emit(f_opr,op_fpurm);
                emit(f_opr,op_fprtoi32); 
		emit(f_opr, op_fpstnli32);
		popInt(); popFloat();
                emit(p_ldvl, v); pushInt();
                emit(f_opr, op_mint ); pushInt();
                emit(f_opr, op_xor ) ; popInt();
             }
	     else {
	     	VLocal *dbl = NULL;
	     	extern FloatCon *fc_unsfix;
	     	
	     	if( doubledest == NULL ) 
	     		dbl = doubledest = allocatetemp(DBLREG);
	     	
		if( arglength == 4 )
		{
			emit( p_ldx, fplib.real32toreal64 );
			emit( p_stvl, v );
			
		        cg_expr(x1);
			doubleaddr(doubledest);
			emit( p_fpcall, v );
			doubleaddr(doubledest);
		}
		else cg_expr(x1);
		/* doubedest now contains the value to convert */

                emitFpConst(fc_unsfix, DBLREG);
                codeFpCall(s_minus,0x02000008);
                /* doubledest = doubledest - (2**31+0.5)	*/
                
                emit( p_ldx, fplib.real64toint32 );
                emit( p_stvl, v );
                doubleaddr(doubledest);
                emit(f_ldc,ROUND_NEAREST);
                emit(p_fpcall, v);
		/* we now have (int)doubledest on the stack add back 2**31 */

		emit(f_opr, op_mint);
		emit(f_opr, op_xor );
		
		if( dbl != NULL ) freetemp(dbl), doubledest = NULL;
		
		setInt(FullDepth-1);             
	     }
        }
        else 
        { /* Fix to a signed result is rather easier. */
          /* This is the case where we would like to know the target */
	     if( floatingHardware )
	     {
	             cg_expr(x1);
		     emit(p_ldvlp, v);pushInt();
		     emit( f_opr, op_fpurz );	/* round to zero */
	             emit(f_opr, op_fprtoi32 ); 
		     emit(f_opr, op_fpstnli32);
		     popFloat(); popInt();
        	     emit( p_ldvl, v); pushInt();
	     }
	     else {
		if( arglength == 4 )
		{
			emit( p_ldx, fplib.real32toint32 );
			emit( p_stvl, v );
			cg_expr(x1);
			emit( f_ldc, ROUND_ZERO );
			emit( p_fpcall, v );
		}
		else {
			VLocal *olddd = doubledest;
			doubledest = allocatetemp(DBLREG);
			emit( p_ldx, fplib.real64toint32 );
			emit( p_stvl, v );
			cg_expr(x1);
			emit( f_ldc, ROUND_ZERO );
			emit( p_fpcall, v );
			freetemp( doubledest );
			doubledest = olddd;
		}
		setInt(FullDepth-1);
	     }
        }
        
        freetemp(v);
        
/* If I do something like (short)<some floating expression> I need to    */
/* squash the result down to 16 bits.                                    */
        if (mclength < 4)
        {
            if (mcmode == 0)
            { /* mask and sign extend the correct bit field */
		emit( f_ldc, (1<<(8*mclength))-1 ); pushInt();
		emit(f_opr, op_and );popInt();            
                emit( f_ldc, 1 << (8*mclength ));  pushInt();
                emit(f_opr, op_xword ); popInt();
            }
            else
            { /* Unsigned case, mask out high significance bits */
                emit(f_ldc, (1<<(8*mclength))-1 ); pushInt();
                emit(f_opr, op_and );popInt();
            }
        }
    } /* End of the FIX cases */
    else if (arglength==4 && mclength==4)
    {   
       cg_expr(x1);
       { pop_trace(); return; }
    }
    else if (mcmode==1)		/* signed -> unsigned		*/
    {   
    	int masklength = mclength;
        cg_expr(x1);
#ifdef NEWCAST

	/* For widening casts we mask the sign bits out down to the */
	/* arglength. When narrowing we mask down to the mclength.  */
	if( mclength >= arglength )
	{
		if( arglength == 4 ) { pop_trace(); return; }
		masklength = arglength;
	}
	
#else /* NEWCAST */
        if (mclength==4) { pop_trace(); return; }
#endif /* NEWCAST */
	if( ida < 2 )
	{
		/* to mask we need 2 stack regs, make space and do it */
		VLocal *v;

		emit(f_opr, op_rev );
		v = pushtemp( INTREG );
		emit( f_ldc, (1<<(8*masklength))-1 ); pushInt();
		emit(f_opr, op_and ); popInt();
		poptemp( v, INTREG );
		emit(f_opr, op_rev );
	}
	else {
		emit( f_ldc, (1<<(8*masklength))-1 );pushInt();
		emit(f_opr, op_and );popInt();
	}
        { pop_trace(); return; }
    }
    else if (mcmode==0)		/* unsigned -> signed		*/
    {
        cg_expr(x1);
#ifdef NEWCAST
	if( mclength >= arglength ) { pop_trace(); return; }
#else
        if (mclength==4) { pop_trace(); return; }
#endif
	if( ida < 2 )
	{
		/* to sign extend we need 2 stack regs, make space and do it */
		VLocal *v;
		emit(f_opr, op_rev );
		v = pushtemp( INTREG );
		emit( f_ldc, (1<<(8*mclength))-1 );pushInt();
		emit(f_opr, op_and );popInt();		
		emit( f_ldc, 1<<(8*mclength-1) );pushInt();
		emit(f_opr, op_xword );popInt();
		poptemp( v, INTREG );
		emit(f_opr, op_rev );
	}
	else {
		emit( f_ldc, (1<<(8*mclength))-1 );pushInt();
		emit(f_opr, op_and );popInt();		
		emit( f_ldc, 1<<(8*mclength-1) );pushInt();
		emit(f_opr, op_xword );popInt();
	}
        { pop_trace(); return; }
    }
    else
    {   syserr("cast %d %d %d %d", mcmode, mclength, argmode, arglength);
        { pop_trace(); return; }
    }
}

static void cg_return(x,implicitinvaluefn)
Expr *x; 
bool implicitinvaluefn;
{
    if (x!=0)
    {
	int rep = cautious_mcrepofexpr(x);
/* Structure results are massaged here to give an assignment via a       */
/* special variable.                                                     */
        if( ((rep & 0xff000000)==0x03000000) )
/* N.B. special version of mcrepofexpr() to cope with the rules about    */
/* passing back results from (external) functions.                       */
        {   TypeExpr *t = typeofexpr(x);
	    Expr *xx;
            if (result_variable==NULL) syserr("unexpected structure result");
            else 
	    {
/* make up      *result_variable = <result expression>;                  */
                xx=mk_expr2(s_assign,
                            t,
                            mk_expr1(s_content,
                                     t,
                                     (Expr *)result_variable),
                            x);
		pp_expr(xx,0);
		cg_exprvoid(xx);

	    }
        }
        else 
        {   int mcrep = mcrepofexpr(x);
            int mcmode = (mcrep>>24) & 0xff;
            /* The next line takes care of compiling 'correct' code for */
            /* void f() { return g();}                                  */
            if ((mcrep & 0xffffff) == 0) cg_exprvoid(x);
            else 
		if( rep==0x02000008 )
		{
			/* double result, assign it to the result variable */
			TypeExpr *t = typeofexpr(x);
			Expr *xx;
	                if (result_variable==NULL) syserr("unexpected double result");
                	xx=mk_expr2(s_assign,
                        	    t,
	                            mk_expr1(s_content,
	                                     t,
	                                     (Expr *)result_variable),
                            	    x);
			pp_expr(xx,0);
			cg_exprvoid(xx);
		}
		else cg_expr(x);
        }
    }
    if( !invalof )
    {
	emit( p_ret , 0 );
	setInt(FullDepth); setFloat(FullDepth);
    }
}

/* The next routine is a nasty hack -- see its uses.   */
/* It copies a SynAlloc BindList into a BindAlloc one. */
static BindList *binderise(l)
BindList *l;
{   BindList *f1 = NULL;
    for (; l != NULL; l = l->bindlistcdr)
        f1 = mkBindList(f1, l->bindlistcar);
    return (BindList *)dreverse((List *)f1);
}

static void cg_bindlist(x,initflag)
BindList *x; int initflag;
{
    BindList *old_binders = active_binders;
    if(debugging(DEBUG_CG)) trace("Cg_bindlist");
    x = sort_bindlist(x);
    for (; x!=NULL; x = x->bindlistcdr)
    {   Binder *b = x->bindlistcar;
        if (bindstg_(b) & bitofstg_(s_auto))        /* ignore statics    */
        /* N.B. register vars must also have auto bit set                */
        {   int len = mcrepofexpr((Expr *)b);
            int mode = (len >> 24) & 0xff;
            len &= 0x00ffffff;
            set_VLocal(b, mode, len, initflag);
            active_binders = mkBindList(active_binders, b);
            bindaddr_(b) = (int) active_binders;
/* the next 3 line calculation should be done after regalloc, not here */
            current_stackdepth += pad_to_word(len);
            if (current_stackdepth > greatest_stackdepth)
                greatest_stackdepth = current_stackdepth;
        }
    }
}

/* sorts a bindlist into most-used order depending on data stored	*/
/* during the preparse phase.						*/
BindList *sort_bindlist(x)
BindList *x;
{
	BindList *new = NULL;
	BindList *p, *prev;

	if( !sort_locals ) return x;
		
	for(; x != NULL; x = x->bindlistcdr )
	{
		Binder *b = x->bindlistcar;
		int uses = getinfo(b);

		for( prev=NULL,p = new; p!=NULL; prev=p,p = p->bindlistcdr )
		{
			if( getinfo(p->bindlistcar) > uses ) break;
		}
		if( prev == NULL ) new = mkBindList( p, b );
		else prev->bindlistcdr = mkBindList( p, b );
	}
	return new;
}

/* allocates a virtual local, on the machine stack or vector stack
   depending on actual size.
 */
#define maxScalarSize 8
VLocal *vlocal(len)
int len;
{
	VLocal *v = BindAlloc(sizeof(VLocal));
	v->next = local_chain;
	local_chain = v;
	v->b = NULL;
	v->l = pp_linect;
	v->refs = 0;
	v->len = pad_to_word(len)/4;
	if( len > maxScalarSize && use_vecstack )
	{
		/* put it on vector stack */
		ssp += 1;
		vsp += v->len;
		v->reallocal = ssp;
		if( ssp > maxssp ) maxssp = ssp;
		if( vsp > maxvsp ) maxvsp = vsp;
		emit( p_setv1, vsp );
		emit( p_setv2, v   );
		v->type = v_vec;
	}
	else {
		/* put it on workspace stack */
		ssp += v->len;
		v->reallocal = ssp;
		if( ssp > maxssp ) maxssp = ssp;
		v->type = v_var;
	}
/* trace("vlocal: %x %x %x",v,v->reallocal,v->len); */
	return v;
}

/* Generation and use of temporaries these behave in a stack like manner */
static VLocal *pushtemp( rsort )
RegSort rsort;
{
	VLocal *v = vlocal(rsort==DBLREG?8:4);
	if( floatingHardware && rsort != INTREG )
	{
		emit( p_ldvlp, v ); pushInt();
		emit(f_opr, rsort==FLTREG ? op_fpstnlsn: op_fpstnldb );
		popFloat(); popInt();
	}
	else {
		emit( p_stvl, v );
		popInt();
	}
	return v;
}

static VLocal *allocatetemp(rsort)
RegSort rsort;
{
   return vlocal(rsort==DBLREG?8:4);
}
   
static void poptemp( v ,rsort)
VLocal *v;
RegSort rsort;
{
	if( floatingHardware && rsort != INTREG ) 
        { 
                emit( p_ldvlp, v ); pushInt();
                emit(f_opr, rsort==FLTREG ? op_fpldnlsn: op_fpldnldb ); 
		pushFloat();popInt();
        } 
        else { 
                emit( p_ldvl, v );
		pushInt();
        }
        freetemp(v);
}

void freetemp(v)
VLocal *v;
{
   int size = v -> len;
   
   if (size > maxScalarSize)
   { /* Currently only scalar temps should be being allocated */
      syserr("Vector temporary being freed");
   }
   else
   {
      if (ssp != v->reallocal)
         syserr("Temporaries not used in stack like manner");
      else
         ssp = ssp - size;
         /* SHOULD FREE THE SynAlloced store as well */
	 /* NO! the VLocal structure is still needed!! */
   }
}
   

/* Allocate a VLocal for an argument. Unlike normal locals these will
 * always be on the stack, regardless of size. The VLocal struct is
 * set up with a negative offset, so it may be used like any other.
 */
static void set_Arg(b,mode,len,initflag)
Binder *b;
int mode;
int len;
int initflag;
{
	VLocal *v = BindAlloc(sizeof(VLocal));
	v->next = local_chain;
	local_chain = v;
	v->b = b;
	v->l = pp_linect;
	v->reallocal = ssp;
	v->refs = 0;
	v->type = v_var;
	v->len = pad_to_word(len)/4;
	ssp -= v->len;
/* trace("set_Arg: %x %x %x",v,v->reallocal,v->len); */
	bindxx_(b) = (int) v;
	mode = initflag;
}

static void set_VLocal(b,mode,len,initflag)
Binder *b;
int mode;
int len;
int initflag;
{
	VLocal *v;
	v = vlocal(len);
	bindxx_(b) = (int) v;
	v->b = b;
	if(debugging(DEBUG_CG))
		trace("set_VLocal %s %d",_symname(bindsym_(b)),v->reallocal);
	mode = initflag;
}

Binder *genfnargbinder(t,argpos,mcmode,mcsize)
TypeExpr *t;
int argpos,mcsize,mcmode;
{
	Binder *b = gentempbinder(t);
	VLocal *v = BindAlloc(sizeof(VLocal));

	v->next = local_chain;
	local_chain = v;
	v->b = b;
	v->l = pp_linect;
	
	b->bindmcrep = (mcmode<<24)+mcsize;
	v->reallocal = argpos;
	v->type = v_arg;
	v->len = pad_to_word(mcsize)/4;
	v->refs = 0;
	bindxx_(b) = (int) v;
	return b;
}

#define cg_expr2(xx,mode,sim) \
if( sim && mode == 0x02000008 && doubledest == NULL ) 		\
{								\
	doubledest = allocatetemp(DBLREG);			\
}								\
cg_expr1(xx,TRUE);


#define rle2 0x01
#define lle2 0x02
#define rgtl 0x04
#define leqr 0x08
/* NOW TAKES an AEop !!! */
static void cg_binary(op,a1,a2,commutesp,fpp)
AEop op; Expr *a1; Expr *a2;
int commutesp; RegSort fpp;
{
    int d1 = depth(a1);
    int d2 = depth(a2);
    int mode1 = mcrepofexpr(a1);
    int mode2 = mcrepofexpr(a2);
    int floating = floatingHardware && 
    (fpp!=INTREG||(((mode1>>24)&0xff) == 2)||(((mode2>>24)&0xff) == 2));
    VLocal *v;
    int da;
    int floatsim = !floatingHardware &&
    (fpp!=INTREG||(((mode1>>24)&0xff) == 2)||(((mode2>>24)&0xff) == 2));
    VLocal *olddd = doubledest;

    push_trace("cg_binary");
        
    if(debugging(DEBUG_CG)) 
	trace("cg_binary: %s d1=%d d2=%d ida=%d fda=%d m1=%x m2 = %x floating=%d fpp=%x",
		symbol_name_(op),d1,d2,ida,fda,mode1,mode2,floating,fpp);


	if( floating ) da = fda;
	else da = ida, fpp = (fpp==DBLREG)?DBLREG:INTREG;

    if( da < 2 ) syserr("cg_binary called with insufficient stack depth %d",da);

    if (floatingHardware && is_same(a1, a2))
    {   
        cg_expr2(a1,mode1,floatsim);
        duplicate(fpp);
    }
    else {
	int id1 = idepth(d1);
	int id2 = idepth(d2);

	if( floating && !floatsim) {
		d1 = fdepth(d1) >> FpShift; 
		d2 = fdepth(d2) >> FpShift; 
	}
	else {
		d1 = id1;
		d2 = id2;
	}

	if( !floating && ((max(d1,d2)+1) <= da ) )
	{
		cg_expr2(a1,mode1,floatsim);
		cg_expr2(a2,mode2,floatsim);
	}
	else {
		switch( ((d2 <= (da-1)) ? rle2:0) |
			((d1 <= (da-1)) ? lle2:0) |
			((d2 >d1) ? rgtl:0) |
			((d1==d2) ? leqr:0) )
		{
		case 0:			/* r>2, l>2, l>r	*/
		case rgtl:		/* r>2, l>2, l<r        */
		case leqr:		/* r>2, l>2, l=r        */
		/* generate: right save left restore op		*/
			if( floatsim && fpp == DBLREG )
			{
				VLocal *prevdest = doubledest;
				if( doubledest == NULL )
					syserr("double op with no dest");

				if( needsdd( a2 ) )
				{
					/* allocate temp dd for a2 */
					v = allocatetemp(DBLREG);
					cg_expr( a2 ); setInt(FullDepth);
					doubledest = v;
					cg_expr( a1 );
					doubleaddr(prevdest); pushInt();
					doubledest = prevdest;
				}
				else
				{
					cg_expr( a2 );
					v = pushtemp( INTREG );
					setInt(FullDepth);
					cg_expr( a1 );
					doubleaddr( v ); pushInt();
				}

				codeFpCall(op, mode1);
				doubleaddr(doubledest); setInt(FullDepth-1);
				freetemp( v );
				{ pop_trace(); return; }
			}
			else 
			{
				cg_expr2( a2, mode2 ,floatsim);
				v = pushtemp(fpp);
				cg_expr2( a1, mode1 ,floatsim);
				poptemp( v , fpp );
			}
			break;
		
		case rle2:		/* r<=2, l>2, l>r	*/
		case rle2|lle2:		/* r<=2, l<=2, l>r      */
		case rle2|lle2|leqr:	/* r<=2, l<=2, l=r	*/
		/* generate: left right op			*/
			cg_expr2( a1, mode1 ,floatsim );
			cg_expr2( a2, mode2 ,floatsim );
			break;

		case lle2|rgtl:		/* r > 2, l <= 2, r > l	*/
		case rle2|lle2|rgtl:	/* r<= 2, l <= 2, r > l */
		/* generate: right left [rev] op		*/
			cg_expr2( a2, mode2 ,floatsim );
			cg_expr2( a1, mode1 ,floatsim );
			if( !commutesp ) emit(f_opr, floating?op_fprev:op_rev );
			break;

		default:
			syserr("cg_binary: Impossible tree depth relationship");
		}
	}
    }

	/* T4 floating point emulation is handled here */
	if( floatsim ) 
	{
		codeFpCall(op, mode1);
		if( olddd == NULL && doubledest != NULL ) 
		{
			freetemp(doubledest);
			doubledest = NULL;
		}
		{ pop_trace(); return; }
	}

    if( op != s_nothing )
    {
        codeOperation( op, floating, mode1);
	if( !floating ) 
	   popInt(); 
	else 
	{
	   if (op == s_equalequal || op == s_greater)
	   {
	      popFloat(); popFloat();
	      pushInt();
	   }
	   else
	      popFloat();
	}
    }
    pop_trace();
}

static void verify_integer(x)
Expr *x;
{
    switch(mcrepofexpr(x))
    {
case 0x00000001:    /* signed integers are OK */
case 0x00000002:
case 0x00000004:
case 0x01000001:    /* unsigned integers are OK */
case 0x01000002:
case 0x01000004:
case 0x03000001:    /* structures/unions up to 4 bytes long are OK */
case 0x03000002:
case 0x03000003:
case 0x03000004:
        return;
default:
        syserr("integer expression expected");
    }
}

int depth(x)
Expr *x;
{
	int mclength = mcrepofexpr(x);
        int mcmode = (mclength>>24) & 0xff;
        RegSort rsort = (mclength &= 0x00ffffff,
                (mcmode!=2) ? INTREG : (mclength==4 ? FLTREG : DBLREG ));

	switch( h0_(x) )
	{
	case s_binder:
		if( rsort != INTREG ) return IntUse+FpUse;  
	case s_integer:
	case s_string:
		return IntUse;
	case s_floatcon:
		return IntUse+FpUse;
	case s_fnap:
		/* if the result is double, we need a doubledest */
		if( !floatingHardware && mcmode == 2 &&	mclength == 8 ) 
			return usesAll | NeedsDD;
	        else 	return usesAll;
		
	case s_addrof:
		if( h0_(arg1_(x)) == s_binder ) return IntUse;
	default:
	    {
		int info = getinfo(x);
		return (info == -1) ? FpUse+IntUse : info ;
	    }
	}
}

bool needsdd(x)
Expr *x;
{
	int d = depth(x);
	if( (d & NeedsDD) != 0 ) return TRUE;
	return FALSE;
}

static bool is_same(a,b)
Expr *a; Expr *b;
/* Expressions are considered the same if they compute the same value    */
/* and do not have any side-effects.                                     */
{
    AEop op;
    for (;;)
    {   if ((op=h0_(a))!=h0_(b)) return FALSE;
        if (isvolatile_expr(a) || isvolatile_expr(b)) return FALSE;
        switch (op)
        {
    case s_binder:
            return (a==b) ;  /* volatile attribute already checked */
    case s_integer:
            return (intval_(a)==intval_(b));
    case s_string:
    case s_floatcon:
            return (a==b) ;    /* improve? */
    case s_dot:
            if (exprdotoff_(a) != exprdotoff_(b)) return FALSE;
            a = arg1_(a), b = arg1_(b);
            continue;
    case s_cast:
            if (!equivtype(type_(a), type_(b))) return FALSE;
    case s_addrof:
    case s_bitnot:
    case s_boolnot:
    case s_neg:
    case s_content:
    case s_monplus:
            a = arg1_(a);
            b = arg1_(b);
            continue;
    case s_andand:
    case s_oror:
    case s_equalequal:
    case s_notequal:
    case s_greater:
    case s_greaterequal:
    case s_less:
    case s_lessequal:
    case s_comma:
    case s_and:
    case s_times:
    case s_plus:
    case s_minus:
    case s_div:
    case s_leftshift:
    case s_or:
    case s_rem:
    case s_rightshift:
    case s_xor:
            if (!is_same(arg1_(a), arg1_(b))) return FALSE;
            a = arg2_(a);
            b = arg2_(b);
            continue;
    default:
            return FALSE;
        }
    }
}

static int iszero(x)
Expr *x;
{
    return (integer_constant(x) && result2==0);
}

static int isone(x)
Expr *x;
{
    return (integer_constant(x) && result2==1);
}

static int isminusone(x)
Expr *x;
{
    return (integer_constant(x) && result2==-1);
}

/* Doesn't treat one as a power of two for some reason ! */
static int ispoweroftwo(x)
Expr *x;
{
    unsigned int n, r;
    if (!integer_constant(x)) return 0;
    n = result2;
    r = n & (-n);
    if (n == 0 || r != n) return 0;
    r = 0;
    while (n != 1) r++, n >>= 1;
    return r;
}

static bool instore( x )
Expr *x;
{
  switch (h0_(x))
  {
    case s_content:
    case s_string: 
    case s_binder: return TRUE;
    
    default:       return FALSE;
  }
}

/* Functions for manipulating the stack depth */
void pushInt() 
{
   if( debugging(DEBUG_CG) ) trace("ida(%d)--",ida);
   if (ida < 1)
      syserr("Pushing an int with no space left");
   ida--;
}

void popInt() 
{
   if( debugging(DEBUG_CG) ) trace("ida(%d)++",ida);
   if (ida > 2)
      syserr("Popping an int with nothing to pop");
   ida++;
}

void setInt( depth )
int depth;
{
   if( debugging(DEBUG_CG) ) trace("ida=%d",depth);
  if ((0 <= depth) && (depth <= 3))
     ida = depth;
  else
     syserr("Setting integer depth to %d", depth);
}

void pushFloat() 
{
   if( debugging(DEBUG_CG) ) trace("fda(%d)--",fda);
   if (fda < 1)
      syserr("Pushing a float with no space left");
   fda--;
}

void popFloat() 
{
   if( debugging(DEBUG_CG) ) trace("fda(%d)++",fda);
   if (fda > 2)
      syserr("Popping a float with nothing to pop");
   fda++;
}

void setFloat( depth )
int depth;
{
   if( debugging(DEBUG_CG) ) trace("fda=%d",depth);
  if ((0 <= depth) && (depth <= 3))
     fda = depth;
  else
     syserr("Setting float depth to %d", depth);
}

int inverseCompare( op ) 
AEop op;
{
  switch (op)
  {
      case s_equalequal:  return s_notequal;
      case s_notequal:    return s_equalequal;
      case s_greater:     return s_lessequal;
      case s_lessequal:   return s_greater;
      case s_greaterequal:return s_less;
      case s_less:        return s_greaterequal;
      default:            syserr("Invalid op %d in inversecompare",op);
  }
}

/* End of section cg.c */
