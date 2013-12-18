/* $Id: cg.h,v 1.3 1991/01/09 10:47:54 nickc Exp $ */
/* Jim's defs for bool, true, false */
#define TRUE  1
#define FALSE 0

#define static

#define link_area_size	2		/* words in proc linkage	*/

#define max_preargs	4		/* max # of pre-allocated arg slots */

typedef int		Xop;		/* transputer operand		*/

typedef int		RealLocal;	/* may become more complex 	*/

/* processor configuration options */
#ifdef NEVER
extern int	floatingHardware;	/* H/w Fp unit (T800,T810)	*/
extern int	dupAllowed;		/* dup instr.  (T800,T810,T425)	*/
extern int	popAllowed;		/* pop instr.  (T425,T810)	*/
#endif

/* stack local */
typedef struct VLocal {
	struct VLocal	*next;		/* next in local_chain		*/
	Binder		*b;		/* defining symbol, if any	*/
	int		l;		/* line defined on		*/
	RealLocal	reallocal;	/* offset on ssp		*/
	int		type;		/* see below			*/
	int		refs;		/* number of refs		*/
	int		len;		/* size in words		*/
} VLocal;

#define v_var		1	/* variable/parameter on workspace stack*/
#define v_vec		2	/* vector on vector stack		*/
#define v_arg		3	/* function call argument		*/

extern VLocal *local_chain;

/* During fp simulation on the T4 all double operations need a destination */
/* this is either a double length space on the stack, or a pointer on the  */
/* stack to such a place. This macro decides which and loads the address   */
/* of the first word.							   */

#define doubleaddr(v) (((v)->len==2)?emit(p_ldvlp,v):emit(p_ldvl,v))

typedef struct Block {
	struct Block 	*next;		/* next block in chain		*/
	struct Block	*prev;		/* prev block in chain		*/
	Xop		jump;		/* terminating jump		*/
	LabelNumber	*lab;		/* label for this block		*/
	int		flags;		/* block flags			*/
	struct Tcode	*code;		/* start of code list		*/
	struct Tcode    *current;       /* end of code list             */
	union {
		LabelNumber	*next1;	/* label of successor block	*/
		LabelNumber	**table;/* or case branch table		*/
	} operand1;
	union {
		LabelNumber	*next2;	/* alternate successor		*/
		int		tabsize;/* or size of case table	*/
	} operand2;
	/* more to come */
} Block;

#define succ1 operand1.next1
#define succ2 operand2.next2

/* block flags */
#define BlkFlg_align	1	/* align block to word boundary		*/
#define BlkFlg_alive	2	/* block is reachable			*/
#define BlkFlg_visit	4	/* block is being visited		*/

typedef union  Operand 
{
	VLocal	    *local;    /* Local variable access ops */
	LabelNumber *label;    /* ldpis                     */
	Binder      *binder;   /* Calls                     */
	Symstr	    *sym;      /* Calls			    */
	FileLine    *line;     /* p_infoline                */
	long         value;    /* Constants (long ?)        */
		
} Operand;

typedef struct Tcode {
	struct Tcode    *next;          
	Xop		 op;
	Operand          opd;
} Tcode;

typedef struct Literal {
	struct Literal	*next;
	int 		type;
	union {
		StringSegList	*s;		/* string 		*/
		DbleBin        db;		/* space for floatcon	*/
		FloatBin       fb;
		int		i;
		unsigned	u;
	} v;
#ifdef OLDLITS
	LabelNumber *lab;
#else
	int	offset;
#endif
} Literal;

#define lit_string	1
#define lit_floatSn	2 
#define lit_floatDb	3
#define lit_integer	4
#define lit_unsigned	5

/* The following two macros test the size of an integer constant.	*/
/* poolint decides whether a constant should be placed in the literal	*/
/* pool, small integers, values near MinInt and ~MinInt can be compiled	*/
/* inline.								*/
/* smallint tests whether a value can be represented using less than 	*/
/* maxpfix prefixes (+1 for actual instruction).			*/

#define maxpfix 3

#define maxsmallint (1<<((maxpfix+1)*4))

#define poolint(v) ((!smallint(v)) && (v>=((int)MinInt+maxsmallint)) && (v!=(~MinInt)))

#define smallint(v) ((-maxsmallint <= v) && (v < maxsmallint))

extern struct FPLIB {
	Binder *real32op, *real64op;			/* + - * / */
	Binder *real32rem, *real32eq, *real32gt;	/* single rem > == */
	Binder *real64rem, *real64eq, *real64gt;	/* double rem > == */
	Binder *real32toreal64, *real64toreal32;	/* change size of real */
	Binder *real32toint32, *int32toreal32;		/* single <-> int */
	Binder *real64toint32, *int32toreal64;		/* double <-> int */
} fplib;

extern struct DEBUG {
	Binder *stackerror;		/* called on stack overflow	*/
	Binder *notify_entry;		/* called on procedure entry	*/
	Binder *notify_return;		/* called on procedure return	*/
	Binder *notify_command;		/* called before each command	*/
} debug;

extern struct BUILTIN {
	Binder *_operate;		/* execute Transputer instruction */
	Binder *_direct;		/* execute direct instruction	  */
	Binder *_setpri;		/* set process priority		  */
} builtin;

#define LIT_FUNC	0x0d000000

#define NOTINLOOP   ((LabelNumber *)0x80000000)
#define NOTINSWITCH ((LabelNumber *)0x80000000)

extern int max_icode, max_block;          /* statistics (lies, damn lies) */

extern int current_stackdepth;
extern int greatest_stackdepth;           /* needed for stack check code */
extern int max_argsize;
extern int procflags;
extern BindList *local_binders;
extern BindList *active_binders;
extern Binder *integer_binder;
extern bool has_main;

extern struct SwitchInfo {
    BindList *binders;            /* 'endcase' may be non-local goto */
    LabelNumber *defaultlab, *endcaselab;
} switchinfo;

extern struct LoopInfo {
    BindList *binders;  /* 'break', 'continue' may be non-local goto */
    LabelNumber *breaklab, *contlab;
} loopinfo;

extern LoopList *all_loops;          /* loopopt.c uses           */
extern bool in_loop;                 /* flowgraph.c inspects ... */
extern BlockList *this_loop;         /* ... and chains on this   */
extern bool loopseen;

extern int anylabels;         /* somewhat vestigial */
#define SEEN_CASE_BIT   1     /* bits therein       */
#define SEEN_LAB_BIT    2
#define IN_SWITCH_BIT   4

#define cg_content_for_dot_(x) \
   (mcrepofexpr(arg1_(x)) == 0x0000004 ? arg1_(x) : \
    mk_expr1(s_content, type_(x), take_address(x)))

typedef struct CasePair { int caseval; LabelNumber *caselab;} CasePair;

#define unsigned_expression_(x) ((mcrepofexpr(x)&0xff000000)==0x01000000)

extern int result2;

#define def_callmargin 4	/* default number of slots for calls */
#define big_callmargin 1999	/* should be big enough */

/* dump_info controls whether we put out a procedure info block	*/
/* this also makes maxcallmargin large so there are no internal	*/
/* ajw's in the procedure.					*/
#define dump_info (pp_pragmavec['i'-'a'] > 0)
#define var_dump_info (pp_pragmavec['i'-'a'])

/* debug notify indicates the level of debugging notify routines added	*/
/* to the code. -1 and 0 switch it off. >1 includes procedure entry &	*/
/* exit routines, >5 includes per-line routines. Other values reserved  */
/* for any future expansion.						*/
#define debug_notify (pp_pragmavec['n'-'a'])

/* use_vecstack controls whether we put vectors and structures on the 	*/
/* vector stack. This is usually only switched off for language 	*/
/* independant system code.						*/
#define use_vecstack (pp_pragmavec['f'-'a'] != 0)
#define var_use_vecstack pp_pragmavec['f'-'a']

/* branch_opt controls whether we do branch_chain optimisation		*/
/* switch off with pragma -j0						*/
#define branch_opt (pp_pragmavec['j'-'a'] != 0)

/* sort_locals controls whether we sort the local variables of a block	*/
/* into most-used order.						*/
#define sort_locals (pp_pragmavec['k'-'a'] != 0)
#define var_sort_locals (pp_pragmavec['k'-'a'])

/* proc_names controls whether we put out procedure names into the code */
#define proc_names (pp_pragmavec['g'-'a'] != 0)

/* proc_type indicates the processor type we are compiling for		*/
/* currently -1,4 = T414, 5 = T425, 8 = T800, 9 = T810.			*/
#define proc_type		(pp_pragmavec['t'-'a'])
#define floatingHardware	(proc_type == 8 || proc_type == 9)
#define dupAllowed		(proc_type == 8 || proc_type == 9 || proc_type == 5)
#define popAllowed		(proc_type == 5 || proc_type == 9)

/*   pragmas	default		description
	a	ON		warn about implicit function args
	b	ON		warn about implicit casts
	c	OFF		memory access checks (not used)
	d	ON		warn about deprecated features
	e			#error behaviour
	f	ON		use vector stack
	g	ON		put proc names in code
	h
	i	OFF		generate procedure info blocks
	j	ON		eliminate jump-to-jumps
	k	ON		sort block local variables
	l
	m
	n	OFF		generate notifys
	o
	p	OFF		profiling level
	q
	r	
	s	ON		stack checking 
	t	-1		processor type
	u
	v			special treatment for printf etc.
	w
	x
	y
	z
*/

/* Definitions for depths packed together into a single word */
#define IntUse          1
#define FpUse           (1<<FpShift)
#define FpShift         8
#define NeedsDD		0x10000		/* set if T4 expr need doubledest */

#define usesAll         FullDepth | (FullDepth << FpShift) /* e.g. Fnap */
#define idepth(x)       ((x)&0x00ff)
#define fdepth(x)       ((x)&0xff00)

#define FullDepth	3

extern VLocal *modtab;
extern VLocal *owndata;
extern VLocal *real32op;
extern VLocal *real64op;
extern VLocal *doubledest;
extern VLocal *litpool;

extern int addrexpr;			/* true if evaluating address	  */

extern int ida,fda;                     /* Depths available in the stacks */
/* Functions to manipulate the stacks ...
   these will become macros when checking is no longer needed */
extern void pushInt();    /* ida-- */
extern void popInt();     /* ida++ */
extern void setInt();     /* ida = */
extern void pushFloat();  /* fda-- */
extern void popFloat();   /* fda++ */
extern void setFloat();   /* fda = */

/* Other external functions */
extern void emitp();
extern void emitv();
extern void codeOperation();
extern char * opName();
extern int pp_expr();

#define trace mytrace

#define push_trace if(debugging(DEBUG_CG))_push_trace
#define pop_trace if(debugging(DEBUG_CG))_pop_trace

/* floating point emulation rounding modes */

#define ROUND_ZERO	0
#define ROUND_NEAREST	1
#define ROUND_POSITIVE	2
#define ROUND_MINUS	3

#define STACK_SAFETY	64	/* safety margin for stack error code */
