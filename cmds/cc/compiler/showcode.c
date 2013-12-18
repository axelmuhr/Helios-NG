/* $Id: showcode.c,v 1.10 1994/09/23 08:44:45 nickc Exp $ */
#include "cchdr.h"
#include "util.h"
#include "xpuops.h"
#include "xrefs.h"
#include "cg.h"
#include "AEops.h"
#ifdef __STDC__
#define start_args(a,b)		va_start(a, b )
#include <stdarg.h>
#else
#define start_args(a,b)		va_start(a)
#include <varargs.h>
#endif

#define PUTFC(a,b)	putfc(a, b)

#ifdef COMPILING_ON_XPUTER
#undef va_alist
#define va_alist ...
#define va_dcl
#endif

#ifdef COMPILING_ON_DOS
#define va_alist ...
#define va_dcl
#undef PUTFC
#define PUTFC(a, b)	putfc(a, (long)(b))
#endif

void trace(char *str, ...);
void aprintf(char *str, ...);
void putfc( Xop op, long val );
void putfd( Xop op, Binder *b );
void putop( Xop op );
void putlab( LabelNumber *l );



#define c_ext	'_'
#define c_code	'.'

typedef struct Stub {
	struct Stub	*next;
	int		type;
	int		valid;
	Symstr		*sym;
} Stub;

#define type_ext	1
#define type_pv		2
#define type_fp		3

typedef struct DataCode {
	struct DataCode	*next;
	int		type;
	int		dest;
	union {
		LabelNumber	*l;
		Symstr		*s;
		int		i;
	} src;
	int size;
} DataCode;

/* DataCode types */
#define DC_MOVE		0
#define DC_FUNC		1
#define DC_STRING	2
#define DC_STATIC	3
#define DC_EXTERN	4
#define DC_EXTFUNC	5

typedef struct FileName {
	struct FileName	*next;
	LabelNumber	*lab;
	char		*name;	
} FileName;

FileName *filenamelist = NULL;

extern int maxssp;
extern int maxvsp;
extern int vsp;
extern int maxcall;
extern int ncalls;
extern Block *topblock;
extern Literal *litpoolhead;
extern DataInit *datainitp;
extern int dataloc;

static LabelNumber *infolab = NULL;
static LabelNumber *litlab = NULL;

extern Block *start_new_basic_block();

int nomodule;
int nodata;
int export_statics = FALSE;
int libfile = FALSE;

int fnstack;

Stub *stubhead, *stubend;
struct FPLIB fplib;
struct DEBUG debug;
struct BUILTIN builtin;

/* forward references */
void dumplits();
#ifdef __DOS386
void genstub( int type, Symstr *sym, long valid);
static dosym(Symstr *s, int ext);
#else
void genstub();
#endif
Binder *lib_binder();
LabelNumber *genpvstub();
LabelNumber *filenamelabel();

void asm_header()
{
	int i = strlen(pp_cisname);
	char *cisname;

	while( i>=0 && pp_cisname[i] != '/' ) i--;
	cisname = &pp_cisname[i+1];

#ifdef DBX
	if( dump_info ) db_init(cisname);
#endif

	if( !libfile && !nomodule )
	{		
	        align();
		aprintf("\tmodule\t-1\n");
		aprintf(".ModStart:\n");
		aprintf("\tword\t#60f160f1\n");
		aprintf("\tword\t.ModEnd-.ModStart\n");
		aprintf("\tblkb\t31,\"%s\" byte 0\n",cisname);
		aprintf("\tword\tmodnum\n");
		aprintf("\tword\t1\n");
		aprintf("\tword\t.MaxData\n");
		aprintf("\tinit\n");
	}

	stubhead = stubend = NULL;

	/* initialise fplib binders */
	fplib.real32op = lib_binder("real32op");
	fplib.real32rem = lib_binder("real32rem");
	fplib.real32gt = lib_binder("real32gt");
	fplib.real32eq = lib_binder("real32eq");

	fplib.real64op = lib_binder("real64op");
	fplib.real64rem = lib_binder("real64rem");
	fplib.real64gt = lib_binder("real64gt");
	fplib.real64eq = lib_binder("real64eq");

	fplib.real32toreal64 = lib_binder("real32toreal64");
	fplib.real64toreal32 = lib_binder("real64toreal32");
	fplib.int32toreal32 = lib_binder("int32toreal32");
	fplib.int32toreal64 = lib_binder("int32toreal64");
	fplib.real32toint32 = lib_binder("real32toint32");
	fplib.real64toint32 = lib_binder("real64toint32");

	/* debugging/stackcheck entries */
	debug.stackerror	= lib_binder("_stack_error");
	debug.notify_entry	= lib_binder("_notify_entry");
	debug.notify_return	= lib_binder("_notify_return");
	debug.notify_command	= lib_binder("_notify_command");
	
	/* builtins */
	builtin._operate	= lib_binder("_operate");
	builtin._direct		= lib_binder("_direct");
	builtin._setpri		= lib_binder("_setpri");	
}

Binder *lib_binder(name)
char *name;
{
	Symstr *s = sym_insert(name, s_identifier);
	return global_mk_binder(0,s,bitofstg_(s_extern),
		global_list6(t_fnap, te_int, 0, 1, 100, 0));
}

void asm_trailer()
{
#ifdef DBX
	if( dump_info ) db_tidy();
#endif
	genfilenames();
	makestubs();
	if( !nodata ) gendata();
	if( !libfile && !nomodule )
	{
		aprintf("\tdata\t.MaxData 0\n");
		align();
		aprintf(".ModEnd:\n");
	}
}

static int tracelevel = 0;

struct traceproc
{
	struct traceproc *last;
	char 		 *proc;
	int		 level;
} *tracestack = NULL;

void _push_trace(str)
char *str;
{
	struct traceproc *t = malloc(sizeof(struct traceproc));
	trace("{ %s()",str);
	tracelevel++;
	if( t == NULL ) return;
	t->last = tracestack;
	t->proc = str;
	t->level = tracelevel;
	tracestack = t;	
}

void _pop_trace() 
{
	struct traceproc *t = tracestack;
	tracelevel--;
	if( tracelevel <= 0 ) tracelevel = 0;
	if( t == NULL ) trace("} ??????");
	else
	{
		trace("} %s",t->proc);
		tracestack = t->last;
		free(t);
	}
}


void trace(str,va_alist)
char *str;
va_dcl
{
	int i = tracelevel;
	va_list a;
	start_args(a, str);
	aprintf("-- ");
	while(i--) putc(' ',asmstream);
	_vfprintf(asmstream,str,a);
        va_end(a);
	putc('\n',asmstream);
}

gencode(name,external)
Symstr *name;
int external;
{
	
	optimise();		/* invoke optimiser */
	        
	infolab = litlab = NULL;
		
	genstub( type_ext, name , 0);

	if( !libfile && (external || export_statics) ) 
	{
		adddata( global_list4(0,name,LIT_FUNC,external) );
		dataloc += 4;
	}

	fnstack = 0;
	maxssp += maxcall;

	align();
	
	if( dump_info )
	{
		aprintf("\tword #60f760f7,.e.%s-.%s,%d,%d,modnum,_%s,0,0\n",
				_symname(name),_symname(name),
				maxssp,maxvsp,_symname(name));
	}
	
	if( !no_stack_checks || debug_notify > 0 || profile_option >= 1 ) 
	{
		infolab = nextlabel();
		infolab->refs=1;
		putlab(infolab);
	}

	if( proc_names )
	{
		aprintf("\tword #60f360f3,.%s byte \"%s\",0 align\n",
				_symname(name),_symname(name));
	}

	aprintf(".%s:\n",_symname(name));

	/* generate a stack check if required. This code assumes that the */
	/* scalar and vector stacks occupy the same block and grow	  */
	/* towards eachother.						  */
	if( !no_stack_checks && use_vecstack )
	{
		LabelNumber *l;
		PUTFC( f_ldl, 1 );
		PUTFC( f_ldnl, 1 );
		if( maxvsp != 0 ) PUTFC( f_ldnlp, maxvsp );
		PUTFC( f_ldlp, -(maxssp+STACK_SAFETY) );
		putop( op_gt );
		putfl( f_cj, l=nextlabel());
		l->refs++;
		aprintf("\tldc\t..%d-2\n",infolab->name);
		putop( op_ldpi );
		PUTFC( f_ldl, 1 );
		putsym( f_call, bindsym_(debug.stackerror), c_code );
		putlab(l);
		genstub( type_ext, bindsym_(debug.stackerror), 1 );
	}

	if( debug_notify >= 1 || profile_option >= 1 )
	{
		PUTFC( f_ldlp, 0 );
		aprintf("\tldc\t..%d-2\n",infolab->name);
		putop( op_ldpi );
		PUTFC( f_ldl, 1 );
		putsym( f_call, bindsym_(debug.notify_entry), c_code );
		genstub( type_ext, bindsym_(debug.notify_entry), 1 );
	}

	if( maxssp > 0 ) PUTFC( f_ajw, -maxssp );
	
	if( modtab != NULL ) 		/* load up module table if needed */
	{
		PUTFC( f_ldl, maxssp + 1 );
		PUTFC( f_ldnl, 0 );
		PUTFC( f_stl, maxssp - modtab->reallocal );
	}
	if( (modtab != NULL && ncalls>0) || maxvsp>0 )	
					/* if we use vstack		 */
	{				/* build new descriptor		 */
		PUTFC( f_ldl, maxssp + 1 );
		PUTFC( f_ldnl, 1 );
		if( maxvsp > 0 ) PUTFC( f_ldnlp, maxvsp );
		PUTFC( f_stl, maxssp - modtab->reallocal + 1);
	}
		
	if( owndata != NULL )		/* and own data pointer		*/
	{
		if( modtab != NULL ) 
		{
			PUTFC( f_ldl, maxssp - modtab->reallocal );
		}
		else {
			PUTFC( f_ldl, maxssp + 1 ); 
			PUTFC( f_ldnl, 0 );
		}
		aprintf("\tldnl\tmodnum\n");
                PUTFC( f_stl, maxssp - owndata->reallocal );
	}

	/* check here whether a literal pool local has been allocated 	*/
	/* and whether any literals have been put in it.		*/
	if( litpool != NULL && litpoolhead != NULL )
	{
		if( litlab == NULL ) litlab = nextlabel();
		aprintf("\tldc\t..%d-2\n",litlab->name);
		putop( op_ldpi );
		PUTFC( f_stl, maxssp - litpool->reallocal );
	}
	
	if( real32op != NULL )
	{
		if( modtab != NULL ) 
		{
			PUTFC( f_ldl, maxssp - modtab->reallocal );
		}
		else {
			PUTFC( f_ldl, maxssp + 1 ); 
			PUTFC( f_ldnl, 0 );
		}
		aprintf("\tldnl\t@_real32op\n");
		aprintf("\tldnl\t_real32op\n");
                PUTFC( f_stl, maxssp - real32op->reallocal );
	}

	if( real64op != NULL )
	{
		if( modtab != NULL ) 
		{
			PUTFC( f_ldl, maxssp - modtab->reallocal );
		}
		else {
			PUTFC( f_ldl, maxssp + 1 ); 
			PUTFC( f_ldnl, 0 );
		}
		aprintf("\tldnl\t@_real64op\n");
		aprintf("\tldnl\t_real64op\n");
                PUTFC( f_stl, maxssp - real64op->reallocal );
	}

	outputCode(topblock);

    if( dump_info )
    {
	aprintf(".e.%s:\n",_symname(name));
    }

    dumplits();

}

outputCode(b)
Block *b;
{
	LabelNumber *pendinglab = NULL;
	LabelNumber *pendinglab2 = NULL;
	Xop pendop = -1;

	while(b != NULL )
	{
		if(debugging(DEBUG_CG))
			trace("doing block %d(%d) %s jump %s pending op %s lab %d lab2 %d",
				b->lab->name,b->lab->refs,
				(b->flags&BlkFlg_alive)?"alive":"dead",
				opName(b->jump),opName(pendop),
				pendop==p_noop?-1:pendinglab->name,
				pendop!=f_cj?-1:pendinglab2->name);

		if( b->flags & BlkFlg_alive )
		{
			/* for the first real block in the procedure,	*/
			/* fake a jump to it. This is to get the generation */
			/* of its label correct.			*/
			if( pendop == -1 )
			{
				pendop = f_j;
				pendinglab = b->lab;
			}
			switch( pendop )
			{
			case f_cj:
				/* if we are about to do		*/
				/* cj x ; j y; x:			*/
				/* actually generate 			*/
				/* eqc 0; cj y; x:			*/
				if( pendinglab2 == b->lab )
				{
					PUTFC( f_eqc, 0 );
					putfl( f_cj, pendinglab );
					if( b->lab->refs > 1 ) putlab(b->lab);
					break;
				}
				/* if both branches go the same way, just jump */
				if( pendinglab == pendinglab2 )
				{
					putfl( f_j, pendinglab);
					putlab(b->lab);
					break;
				}
				/* else put out a normal cj	*/
				putfl( f_cj, pendinglab2 );
				/* & drop through to do jump */
			case p_j:
			case f_j:
				if( pendinglab == b->lab )
				{
					/* when we have a pending jump to the */
					/* block we are about to generate,    */
					/* only output the label if there is  */
					/* more than 1 ref to it.	      */
					if( pendinglab->refs > 1 ) putlab(b->lab);
					break;
				}
				else if( pendop == p_j )
				{
					PUTFC( f_ldc, 0 );
					putfl( f_cj, pendinglab );
				}
				else putfl( f_j, pendinglab );
				putlab(b->lab);
				break;
				
			case p_noop:
				putlab(b->lab);
				break;
			}
			
			pendop = p_noop;
			pendinglab = NULL;

			outputBlock(b);

			switch( b->jump )
			{
			case f_cj:
				pendop = f_cj;
				pendinglab = b->succ1;
				pendinglab2 = b->succ2;
				break;
			case f_j:
			case p_j:
				pendop = b->jump;
				pendinglab = b->succ1;
				break;
				
			case p_case:
				{
					int i, size = b->operand2.tabsize;
					LabelNumber **tab = b->operand1.table;
					for( i = 0; i<size; i++ )
						aprintf("\tsize 4 j ..%d\n",tab[i]->name);
					pendinglab = NULL;
					pendop = p_noop;
					break;
				}
			}
		}
		b = b->next;
	}

	switch( pendop )
	{
	case p_noop: break;
	
	case f_j:
		putfl( f_j, pendinglab );
		break;
		
	case p_j:
		PUTFC( f_ldc, 0 );
		putfl( f_cj, pendinglab );
		break;
		
	case f_cj:
		putfl( f_cj, pendinglab2 );
		putfl( f_j, pendinglab );
		break;
	}
}

static int fpoptab[4] = { op_fpldnlmuldb, 
			  op_fpldnlmulsn, 
			  op_fpldnladddb, 
			  op_fpldnladdsn };

outputBlock(b)
Block *b;
{
  Tcode *t;
  
  for (t = b->code; t != NULL; t = t->next)
  {
     Xop op = t -> op;
     
     switch (op)
     {  /* Jumps never get here */
         case f_opr: /* Operations */
         {
            int x = 0;
            switch( t->opd.value )
            {
            case op_fpldnlsn: x |= 1;
            case op_fpldnldb: 
            	if( t->next != NULL )
            	{
            		Tcode *tn = t->next;
            		    if( tn->op == f_opr &&
            		    (tn->opd.value == op_fpadd || tn->opd.value == op_fpmul ) )
            		{
            			if( tn->opd.value == op_fpadd ) x |= 2;
				t->opd.value = fpoptab[x];
				tn->op = p_noop;
				/* drop through to output op */
            		}
            	}
            }
            putop( (Xop)t -> opd.value );
            break;
	 }
	 
         case p_ldvl:
         case p_stvl:
         case p_ldvlp:
         {
		VLocal *v = t->opd.local;
		int offset = 0;
		switch( v->type )
		{
		case v_vec:
			if( op != p_ldvlp )
			    syserr("Gencode: attempt to access array directly");
			op = f_ldl;
			offset = fnstack + maxssp - v->reallocal;
			break;
			
		case v_var:
			op = (op == p_ldvl) ? f_ldl:
			     (op == p_stvl) ? f_stl: f_ldlp;
			offset = fnstack + maxssp - v->reallocal;
			break;

		case v_arg:
			op = (op == p_ldvl) ? f_ldl:
			     (op == p_stvl) ? f_stl: f_ldlp;
			offset = v->reallocal;
			break;
		}
		if( op != f_ldlp || t->next == NULL )
		{
			if( op == f_ldl && t->next != NULL &&
			    t->next->op == p_stvl &&
			    /*v->type == t->next->opd.local->type &&*/
			    v->reallocal == t->next->opd.local->reallocal )
			{
trace("peephole out ldl %d stl %d",v->reallocal, t->next->opd.local->reallocal );
				t = t->next;
			}
			else PUTFC( op, offset );
			break;
		}
		t->op = f_ldlp;
		t->opd.value = offset;

		/* drop through into ldlp case */
	}

		/* try a little peephole optimisation here		*/
	case f_ldlp:
		if( t->next != NULL )
		{
			Tcode *next = t->next;
			switch(next->op)
			{
				/* ldlp m ldnl n -> ldl m+n		*/
			case f_ldnl:
				next->op = f_ldl;
				next->opd.value += t->opd.value;
				break;	
			
				/* ldlp m stnl n -> stl m+n		*/
			case f_stnl:
				next->op = f_stl;
				next->opd.value += t->opd.value;
				break;	
			
				/* ldlp m ldnlp n -> ldlp m+n		*/
			case f_ldnlp:
				next->op = f_ldlp;
				next->opd.value += t->opd.value;
				break;	
			
			default:
				PUTFC( (Xop)t->op, t->opd.value );
				break;
			}
		}
		break;
        
        case p_infoline:
		if( t->opd.line->l < 0 || t->opd.line->l > 100000 )
		{
			trace("strange line no in showcode %d",t->opd.line->l);
			break;
		}
               	aprintf("-- Line %d (%s)\n", t -> opd.line -> l,
                                             t -> opd.line -> f);
		if( debug_notify >= 5 || profile_option >= 2 )
		{
			LabelNumber *l = filenamelabel(t->opd.line->f);
			aprintf("\tldc\t..%d-2\n",l->name); 
			putop( op_ldpi );

			PUTFC( f_ldc, t -> opd.line -> l );
			
			if( modtab ) /* &module table */
				PUTFC( f_ldlp, fnstack + maxssp - modtab->reallocal);
			else PUTFC( f_ldl, fnstack + maxssp + 1 );

			putsym( f_call, bindsym_(debug.notify_command), c_code );
			genstub( type_ext, bindsym_(debug.notify_command), 1 );
		}
               break;
	case p_setv1:
		PUTFC( f_ldl, maxssp - modtab->reallocal + 1);
		PUTFC( f_ldnlp, -(t->opd.value) );
		break;
	case p_setv2:	
		PUTFC( f_stl, maxssp - (t->opd.local)->reallocal );
		break;

	case p_ldx:
		loaddataptr(t->opd.binder);
		putsym( f_ldnl, bindsym_(t->opd.binder), c_ext );
		break;

	case p_stx:
		loaddataptr(t->opd.binder);
		putsym( f_stnl, bindsym_(t->opd.binder), c_ext );
		break;

	case p_ldxp:
		loaddataptr(t->opd.binder);
		putsym( f_ldnlp, bindsym_(t->opd.binder), c_ext );
		break;

	case p_lds:
		loaddataptr(t->opd.binder);
		putfd( f_ldnl, t->opd.binder);
		break;

	case p_sts:
		loaddataptr(t->opd.binder);
		putfd( f_stnl, t->opd.binder);
		break;

	case p_ldsp:
		loaddataptr(t->opd.binder);
		putfd( f_ldnlp, t->opd.binder);
		break;

	case p_ldpi:
		aprintf("\tldc\t..%d-2\n",t->opd.label->name);
		putop( op_ldpi );
		break;

	case p_ldpf:
		aprintf("\tldc\t.%s-2\n",_symname(bindsym_(t->opd.binder)));
		putop( op_ldpi );
		break;

	case p_fnstack:
		fnstack += t->opd.value;
		if( t->opd.value ) PUTFC( f_ajw, -t->opd.value );
		break;

	case p_call:
		if( modtab ) /* &module table */
			PUTFC( f_ldlp, fnstack + maxssp - modtab->reallocal);
		else PUTFC( f_ldl, fnstack + maxssp + 1 );

		putsym( f_call, t->opd.sym, c_code );
		genstub( type_ext, t->opd.sym, 1 );
		break;

	case p_callpv:
	{
		int local = ((VLocal *)bindxx_(t->opd.binder))->reallocal;
		if( modtab ) /* &module table */
			PUTFC( f_ldlp, fnstack + maxssp - modtab->reallocal);
		else PUTFC( f_ldl, fnstack + maxssp + 1 );
		putfl( f_call, genpvstub(fnstack+maxssp+4-local) );
		break;
	}
	
	case p_fpcall:
	{
		int local = t->opd.local->reallocal;
		putfl( f_call, genpvstub(maxssp+4-local) );
		break;
	}

	case p_ret:
		if( debug_notify >= 1 || profile_option >= 1 )
		{
			aprintf("\tldc\t..%d-2\n",infolab->name);
			putop( op_ldpi );

			if( modtab ) /* &module table */
				PUTFC( f_ldlp, fnstack + maxssp - modtab->reallocal);
			else PUTFC( f_ldl, fnstack + maxssp + 1 );
			
			putsym( f_call, bindsym_(debug.notify_return), c_code );
			genstub( type_ext, bindsym_(debug.notify_return), 1 );
		}

		if( maxssp > 0 ) PUTFC( f_ajw, maxssp);
		putop( op_ret );
		break;
	
	case p_j: 
		PUTFC(f_ldc,0);
	        putfl(f_cj,t -> opd.label);
	case p_noop:
	case p_noop2:	
	        break;
		
	/* Here we look for some simple peephole optimisations		*/
	case f_ldnlp:
		if( t->next != NULL )
		{
			Tcode *next = t->next;
			switch(next->op)
			{
				/* ldnlp m ldnl n -> ldnl m+n		*/
				/* ldnlp m stnl n -> stnl m+n		*/
				/* ldnlp m ldnlp n -> ldnlp m+n		*/
			case f_ldnl:
			case f_ldnlp:
			case f_stnl:
				next->opd.value += t->opd.value;
				/* go round main loop here so we pick up */
				/* ldnlp chains.			 */
				continue;
			}
		}
		         
         default:     /* Primaries which take constants */ 
		PUTFC( t->op, t->opd.value );
		break;
       }
   }
   /* Flag this block as output */
   b -> lab -> refs = 0;
   
}
		
loaddataptr(b)
Binder *b;
{
	if( (bindstg_(b) & bitofstg_(s_static)) && owndata != NULL )
	{
		PUTFC( f_ldl, fnstack + maxssp - owndata->reallocal );
	}
	else if ( modtab != NULL )
	{
		PUTFC( f_ldl, fnstack + maxssp - modtab->reallocal );
		if( bindstg_(b) & bitofstg_(s_static) )
			aprintf("\tldnl\tmodnum\n");
		else putat( f_ldnl, bindsym_(b));
	}
	else {
		PUTFC( f_ldl, fnstack + maxssp + 1 );
		PUTFC( f_ldnl, 0 );
		if( bindstg_(b) & bitofstg_(s_static) )
			aprintf("\tldnl\tmodnum\n");
		else putat( f_ldnl, bindsym_(b));
	}
}


#ifdef OLDLITS
void dumplits()
{
	Literal *l = litpoolhead;
	if( l!=NULL) trace("Literals");
	for(;l != NULL; l = l->next ) 
	{
		switch( l->type )
		{
		case lit_string:
		{
			putlab( l->lab );
			putstring( (StringSegList *)l -> v.s );
			break;
		}
		case lit_floatSn:
		{ /* BYTE SEX MAY HURT HERE ... CHECK AND BEWARE */
		        if (debugging(DEBUG_CG))
		           trace("Single precision float");
			align();
			putlab( l->lab );
			putword(l -> v.fb.val);
			break;
		}	
		case lit_floatDb:
		{ /* BYTE SEX MAY HURT HERE ... CHECK AND BEWARE */
		        if (debugging(DEBUG_CG))
		           trace("Double precision float");
			align();
			putlab( l->lab );
			putword(l -> v.db.lsd);
			putword(l -> v.db.msd);
			break;
		}	
		default:
			syserr("Unexpected literal type (%d)",l->type);
			break;
		}
	}
}
#else
void dumplits()
{
	Literal *l = litpoolhead;
	if( l != NULL )
	{
		trace("Literals");
		if( litlab == NULL ) litlab = nextlabel();
		litlab->refs++;
		align();
		putlab( litlab );
	}
	for(;l != NULL; l = l->next ) 
	{
		switch( l->type )
		{
		case lit_integer:
			putword( l->v.i );
			break;

		case lit_string:
			putstring( (StringSegList *)l -> v.s );
			align();
			break;

		case lit_floatSn:
			/* BYTE SEX MAY HURT HERE ... CHECK AND BEWARE */
		        if (debugging(DEBUG_CG))trace("Single precision float");
			putword(l -> v.fb.val);
			break;

		case lit_floatDb:
			/* BYTE SEX MAY HURT HERE ... CHECK AND BEWARE */
		        if (debugging(DEBUG_CG))trace("Double precision float");
			putword(l -> v.db.lsd);
			putword(l -> v.db.msd);
			break;

		default:
			syserr("Unexpected literal type (%d)",l->type);
			break;
		}
	}
}

#endif

LabelNumber *genpvstub( local )
int local;
{
	Stub *s = stubhead;
	LabelNumber *l;
	
	/* only generate one stub for each offset value */

	while( s != NULL )
	{
		if( (s->type == type_pv) && ((int)s->sym == local) )
		{
			return (LabelNumber *)s->valid;
		}
		s = s->next;
	}

	l = nextlabel();
	genstub( type_pv, (Symstr *)local, (long)l );
	return l;
}

void genstub( type, sym , valid)
int type;
Symstr *sym;
long valid;
{
	Stub *s = stubhead;
	if( type!=type_pv ) while( s != NULL )
	{
		/* slightly dodgy comparison	*/
		if( (s->type!=type_pv) && s->sym == sym ) {
			if( s->valid ) s->valid = valid;
			return;
		}
		s = s->next;
	}
	/* there is no existing stub for this one, add it */
	s = GlobAlloc(sizeof(Stub));
	s->next = NULL;
	s->type = type;
	s->valid = valid;
	s->sym = sym;
	if( stubend!=NULL ) stubend->next = s;
	if( stubhead==NULL ) stubhead = s;
	stubend = s;
}

makestubs()
{
	Stub *s = stubhead;
	if( s!=NULL ) trace("Stubs");
	while( s != NULL )
	{
		switch( s->type )
		{
		case type_pv:
		{
			int local = (int)(s->sym);
			align();
			aprintf("..%d:\n",((LabelNumber *)(s->valid))->name);
			PUTFC( f_ldl, local);
			putop( op_gcall );
			break;
		}
		case type_ext:
			if( s->valid && nomodule==0 && !libfile )
			{
				align();
				aprintf(".%s:\n",_symname(s->sym));
				PUTFC( f_ldl, 1 );
				PUTFC( f_ldnl, 0 );
				putat( f_ldnl, s->sym );
				putsym( f_ldnl, s->sym, c_ext );
				putop( op_gcall );
			}
			break;
		
		default:
			syserr("Unknown stub type: %d",s->type);	
		}
		s = s->next;
	}
}

LabelNumber *filenamelabel(name)
char *name;
{
	FileName *f = filenamelist;
	char *s;
	while( f != NULL )
	{
		if( strcmp(name,f->name) == 0 ) return f->lab;
		f = f->next;
	}
	f = (FileName *)GlobAlloc(sizeof(FileName));
	s = (char *)GlobAlloc(pad_to_word(strlen(name)+1));
	f->next = filenamelist;
	f->lab = nextlabel();
	f->name = s;
	strcpy(s,name);
	filenamelist = f;
	return f->lab;
}

genfilenames()
{
	FileName *f = filenamelist;
	if( f!=NULL ) trace("File Names");
	while( f != NULL )
	{
		f->lab->refs=1;
		align();
		putlab(f->lab);
		aprintf("\tword #60f960f9, modnum ");
		aprintf("byte \"%s\",0\n",f->name);
		f = f->next;
	}	
}

putsym(op,s,pch)
Xop op;
Symstr *s;
char pch;
{
	aprintf("\t%s\t%c%s\n",opName(op),pch,_symname(s));
}

putat(op,s)
Xop op;
Symstr *s;
{
        aprintf("\t%s\t@_%s\n",opName(op),_symname(s));
}

void
putfc(op,val)
Xop op;
long val;
{    
    if( op == f_ajw && val == 0 ) return;
    aprintf("\t%s\t%d\n",opName(op),val);
}

void
putfd(op,b)
Xop op;
Binder *b;
{
	if( nodata )
		aprintf("\t%s\t_%s\n",opName(op),_symname(bindsym_(b)));
	else aprintf("\t%s\t..dataseg+%d\n",opName(op),bindaddr_(b)>>2);
}

void
putop( op )
Xop op;
{
   aprintf("\t%s\n", opName(op));
}

void
putlab(l)
LabelNumber *l;
{
        if( l->refs == 0 ) return;
	/* @@@@ note that aligning all labels makes dhrystone go a little */
	/* faster, but the code is larger.				  */
        if( (l->block != DUFF_ADDR) && ((l->block->flags & BlkFlg_align) != 0)) 
        	align();
	aprintf("..%d: -- %d refs\n",l->name, l->refs);
}

putfl(op,l)
Xop op;
LabelNumber *l;
{
	aprintf("\t%s\t..%d\n",opName(op),l->name);
}

align()
{
	aprintf("\talign\n");
}

putstring(s)
StringSegList *s;
{
	while( s != NULL )
	{
		char * str = s->strsegbase;
		int size = s->strseglen;

		while( size != 0 )
		{
			int i;
			int bsize = size > 128 ? 128 : size;
	
			aprintf("\tbyte\t\"");
			for( i = 0; i < bsize; i++ ) sputc(*str++);
			aprintf("\"\n");		

			size -= bsize;
		}
		s = cdr_((List *)s);
	}
	aprintf("\tbyte\t0\n");
}

putword(word)
int word;
{
#if 0
	aprintf("\tword\t#%08x\n",word);
#else
	aprintf("\tbyte\t#%02x,#%02x,#%02x,#%02x\n",
		(word>>0)&0xff,
		(word>>8)&0xff,
		(word>>16)&0xff,
		(word>>24)&0xff);
#endif
}

void aprintf(str, va_alist)
char *str;
va_dcl
{
	va_list a;
	start_args(a, str);
	_vfprintf(asmstream,str,a);
        va_end(a);
}

sputc(c)
char c;
{
	switch( c )
	{
	case 0x07: aputc('\\'); aputc('a'); return;		
	case '\b': aputc('\\'); aputc('b'); return;
	case '\f': aputc('\\'); aputc('f'); return;	
	case '\n': aputc('\\'); aputc('n'); return;
	case '\r': aputc('\\'); aputc('r'); return;
	case '\t': aputc('\\'); aputc('t'); return;
	case 0x0b: aputc('\\'); aputc('v'); return;	
	case '\\': aputc('\\'); aputc('\\'); return;
	case '\?': aputc('\\'); aputc('\?'); return;	
	case '\"': aputc('\\'); aputc('\"'); return;
	case '\'': aputc('\\'); aputc('\''); return;	
	default: 
		if( isprint(c) ) aputc(c);
		else aprintf("\\x0%02x",c&0xff);
		return;
	}
}

aputc(c)
char c;
{
	putc(c,asmstream);
}

/* data initialisation */


static unsigned char dvec[256];
static int vpos;
static LabelNumber *datalab;
static int labset;
static int datapos, datastart;
static int zeroes, datasize;
static Symstr *pendsym;
static int sympos;
static int symext;
DataCode *codelist0;
DataCode *codelist1;

/* layout of init routine stack */
#define init_dataseg	0
#define init_module	1
#define init_ret	2
#define init_display	3
#define init_pri	4

gendata()
{
	LabelNumber *database;
	int i;
	DataInit *d = datainitp;
	DataCode *c;

	if( d == NULL ) return;
	else trace("Data Initialization");
	
	codelist0 = NULL;
	codelist1 = NULL;
	datalab = NULL;
	pendsym = NULL;
	sympos = zeroes = datasize = datastart = datapos = 0;

	(datalab=nextlabel())->refs++;
	labset = 0;

	while( d != NULL )
	{
		switch( d->sort ) 
		{
		case LIT_LABEL:
		        dosym((Symstr *)d->rpt,d->len);
			break;

		case LIT_FUNC:
			codelist0 = global_list4( codelist0, DC_FUNC, datapos, d->rpt );
			if( !libfile )
			{
				dosym((Symstr *)d->rpt,d->len);
				dataword(LIT_NUMBER,0);
			}
			break;
			
		case LIT_HH:
		case LIT_BBH:
		case LIT_HBB:
		case LIT_STRING:
		case LIT_NUMBER:
			i = d->rpt;
			while( i-- ) dataword(d->sort,d->val);
			break;

		case LIT_FPNUM:
		{
			FloatCon *f = (FloatCon *)d->val;
			i = d->rpt;
			while (i--)
				if (d->len == 4)
					dataword(LIT_NUMBER,f->floatbin.fb.val);
				else {
					dataword(LIT_NUMBER,f->floatbin.db.lsd);
					dataword(LIT_NUMBER,f->floatbin.db.msd);
				}
			break;
		}

		case LIT_ADCON:
			if( (Symstr *)(d->len)==bindsym_(codesegment))
			{
				codelist0 = global_list4( codelist0, DC_STRING, 
					datapos, d->val );
				dataword(LIT_NUMBER,0);
				break;
			}
			else if( (Symstr *)(d->len)==bindsym_(datasegment))
			{
				codelist0 = global_list4( codelist0, DC_STATIC,
					datapos, d->val );
				dataword(LIT_NUMBER,0);
				break;
			}
			else {
				/* this is either an external data ref or one to
				   a function. Detect the latter by searching
				   Stubs list. In either case we must generate
				   into codelist1 to wait until external module
				   has initialised.
				*/
				Stub *s = stubhead;
				while( s != NULL )
				{
				    if( (s->type==type_ext) && 
						(s->sym == (Symstr *)d->len) )
				    {
					/* external function pointer */
					if( s->valid )
					    codelist1 = global_list4( codelist1,
					      DC_EXTFUNC, datapos, d->len );
					/* local function pointer */
					else
					    codelist0 = global_list4( codelist0,
					     DC_FUNC, datapos, d->len );
					break;
				    }
				    else s = s->next;
				}
				if( s == NULL )
				{
					/* external data pointer */
					codelist1 = global_list5( codelist1, DC_EXTERN,
						datapos, d->len, d->val );
				}
				dataword(LIT_NUMBER,0);
			}

		default:
			break;
		}
		d = cdr_((List *)d);
	}

	dosym(NULL,1);

	if( datasize > 0 ) genmove();
	
	/* Now generate the init routine */

	{
		LabelNumber *code0;
		LabelNumber *done;

		/* output init routine header */
		align();
		aprintf("\tinit\n");
		PUTFC( f_ajw, -(init_ret) );
		PUTFC( f_ldl, init_display );
		PUTFC( f_ldnl, 0 );
		aprintf("\tldnl\tmodnum\n");
		PUTFC( f_stl, init_module );
		PUTFC( f_ldl, init_module );
		aprintf("\tldnlp\t..dataseg\n");
		PUTFC( f_stl, init_dataseg );

#ifdef V1_0
		/* V1.0 init routines called only once */
		putcodelist(codelist0);
		putcodelist(codelist1);
#else
		/* V1.1 init routines called twice	*/
		code0=nextlabel();
		code0->refs++;

		if( codelist0 == NULL ) done = code0;
		else done = nextlabel(), done->refs++;
		
		PUTFC( f_ldl, init_pri );
		putfl( f_cj, code0 );

		putcodelist(codelist1);
		putfl( f_j, done );
	
		putlab( code0 );
		if( done != code0 )
		{
			putcodelist(codelist0);
			putlab( done );
		}
#endif
		
		PUTFC( f_ajw, init_ret );
		putop( op_ret );
	}
}

putcodelist(codelist)
DataCode *codelist;
{	
	DataCode *c = codelist;
	while( c != NULL )
	{
		switch( c->type )
		{
		case DC_MOVE:		/* constant data initialisation */
			aprintf("\tldc\t..%d-2\n",c->src.l->name);
			putop( op_ldpi );
			PUTFC( f_ldl, init_dataseg );
			if (c->dest != 0) PUTFC( f_adc, c->dest );
			PUTFC( f_ldc, c->size );
			putop( op_move );
			break;

		case DC_FUNC:		/* function pointer */
			aprintf("\tldc\t.%s-2\n",_symname(c->src.s));
			putop( op_ldpi ); 
			if( !libfile ) 
			{	/* normal files, store by offset into dataseg */
				PUTFC( f_ldl, init_dataseg );
				PUTFC( f_stnl, c->dest>>2 );
			}
			else {	/* library files, store by name	into module */
				PUTFC( f_ldl, init_module  );
				putsym(f_stnl, c->src.s, c_ext);
			}
			break;

		case DC_STRING:		/* own code reference (typically a string) */
			aprintf("\tldc\t..%d-2\n",c->src.i);
			putop( op_ldpi );
			PUTFC( f_ldl, init_dataseg );
			PUTFC( f_stnl, c->dest>>2 );
			break;

		case DC_STATIC:		/* own data reference */
			PUTFC( f_ldl, init_dataseg );
			PUTFC( f_ldnlp, c->src.i>>2 );
			if( c->src.i & 0x3 ) PUTFC( f_adc, c->src.i & 0x3 );
			PUTFC( f_ldl, init_dataseg );
			PUTFC( f_stnl, c->dest>>2 );
			break;

		case DC_EXTERN:		/* external data reference */
			PUTFC( f_ldl, init_display );
			PUTFC( f_ldnl, 0 );
			putat( f_ldnl, c->src.s );
			putsym( f_ldnlp, c->src.s, c_ext );
			if( c->size != 0 )
			  {
			    
				if( c->size % 4 == 0 ) 
					PUTFC( f_ldnlp, c->size>>2);
				else PUTFC( f_adc, c->size );
			      }
			PUTFC( f_ldl, init_dataseg );
			PUTFC( f_stnl, c->dest>>2 );
			break;

		case DC_EXTFUNC:	/* external function reference */
			PUTFC( f_ldl, init_display );
			PUTFC( f_ldnl, 0 );			
			putat( f_ldnl, c->src.s );
			putsym( f_ldnl, c->src.s, c_ext );
			PUTFC( f_ldl, init_dataseg );
			PUTFC( f_stnl, c->dest>>2 );
			break;
		}
		c = c->next;
	}
}

int codeloc()
{
	LabelNumber *l = nextlabel();
	l->refs++;
	putlab( l );
	return l->name;
}

/* this is only called between procedures */
void codeseg_stringsegs(s,i)
StringSegList *s;
int i;
{	
	putstring( s );
}

/* the following code is meant to be host byte sex independant */
dataword(sort,w)
int sort,w;
{
	char *c = (char *)&w;
	short *s = (short *)&w;
	
	switch ( sort )
	{
	case LIT_BBBB:
		databyte(c[0]);
		databyte(c[1]);
		databyte(c[2]);
		databyte(c[3]);
		break;

	case LIT_BBH:
		databyte(c[0]);
		databyte(c[1]);
		databyte(s[1]&0xff);
		databyte((s[1]>>8)&0xff);
		break;

	case LIT_HBB:
		databyte(s[0]&0xff);
		databyte((s[0]>>8)&0xff);
		databyte(c[2]);
		databyte(c[3]);
		break;

	case LIT_HH:
		databyte(s[0]&0xff);
		databyte((s[0]>>8)&0xff);
		databyte(s[1]&0xff);
		databyte((s[1]>>8)&0xff);
		break;

	case LIT_NUMBER:
		databyte((w>>0)&0xff);
		databyte((w>>8)&0xff);
		databyte((w>>16)&0xff);
		databyte((w>>24)&0xff);
		break;

	default:
		syserr("Unexpected data sort: %x",sort);
	}
}


databyte(b)
int b;
{
	if( b == 0 ) { zeroes++; datapos++; return; }


	/* the following constant may need tuning */
	if( zeroes <= 16 ) 
	{
		while( zeroes ) databyte1(0), zeroes-- ;
	}
	else
	{	/* generate a move */
		genmove();
	}

	if( datasize == 0 )		/* first non zero byte ? */
		datastart = datapos;


	databyte1(b);
	datapos++;
}

databyte1(b)
int b;
{
	if( vpos == 256 ) flushdata();
	dvec[vpos++] = b;
	datasize++;
}

flushdata()
{
	int i;
	if( vpos )
	{
		if( !labset ) putlab( datalab ), labset++;

		aprintf("\n\tbyte\t");
		for ( i = 0 ; i < vpos-1 ; i++ )
		{
			aprintf("%d,",dvec[i]);
		}
		aprintf("%d\n",dvec[vpos-1]);
	}
	vpos = 0;
}

genmove()
{
	if( datasize == 0 ) return;
	flushdata();
	codelist0 = global_list5( codelist0, DC_MOVE, datastart, datalab, datasize );
	(datalab=nextlabel())->refs++;
	labset = datasize = zeroes = 0;
}

static dosym(s,ext)
Symstr *s;
int ext;
{
	if( ((pendsym != NULL) && (ext||export_statics)) || 
		(s == bindsym_(datasegment)) )
	{
		if( pendsym == bindsym_(datasegment) )
		{
			  aprintf("\tdata\t..dataseg %d\n",(datapos-sympos)>>2);
		}
		else if( pendsym != NULL )
		{
			if( symext )
				aprintf("\tglobal\t_%s\n",_symname(pendsym));

			aprintf("\tdata\t_%s %d\n",
				_symname(pendsym), (datapos-sympos)>>2);

		}
		pendsym = s;
		sympos = datapos;
		symext = ext;
	}
}

char *opName( op ) 
int op;
{
   switch (op)
   {
    case f_j: return("j");
    case f_ldlp: return("ldlp");
    case f_ldnl: return("ldnl");
    case f_ldc: return("ldc");
    case f_ldnlp: return("ldnlp");
    case f_ldl: return("ldl");
    case f_adc: return("adc");
    case f_call: return("call");
    case f_cj: return("cj");
    case f_ajw: return("ajw");
    case f_eqc: return("eqc");
    case f_stl: return("stl");
    case f_stnl: return("stnl");
    case f_opr: return("opr");
    case op_rev: return("rev");
    case op_lb: return("lb");
    case op_bsub: return("bsub");
    case op_endp: return("endp");
    case op_diff: return("diff");
    case op_add: return("add");
    case op_gcall: return("gcall");
    case op_in: return("in");
    case op_prod: return("prod");
    case op_gt: return("gt");
    case op_wsub: return("wsub");
    case op_out: return("out");
    case op_sub: return("sub");
    case op_startp: return("startp");
    case op_outbyte: return("outbyte");
    case op_outword: return("outword");
    case op_seterr: return("seterr");
    case op_resetch: return("resetch");
    case op_csub0: return("csub0");
    case op_stopp: return("stopp");
    case op_ladd: return("ladd");
    case op_stlb: return("stlb");
    case op_sthf: return("sthf");
    case op_norm: return("norm");
    case op_ldiv: return("ldiv");
    case op_ldpi: return("ldpi");
    case op_stlf: return("stlf");
    case op_xdble: return("xdble");
    case op_ldpri: return("ldpri");
    case op_rem: return("rem");
    case op_ret: return("ret");
    case op_lend: return("lend");
    case op_ldtimer: return("ldtimer");
    case op_testerr: return("testerr");
    case op_testpranal: return("testpranal");
    case op_tin: return("tin");
    case op_div: return("div");
    case op_dist: return("dist");
    case op_disc: return("disc");
    case op_diss: return("diss");
    case op_lmul: return("lmul");
    case op_not: return("not");
    case op_xor: return("xor");
    case op_bcnt: return("bcnt");
    case op_lshr: return("lshr");
    case op_lshl: return("lshl");
    case op_lsum: return("lsum");
    case op_lsub: return("lsub");
    case op_runp: return("runp");
    case op_xword: return("xword");
    case op_sb: return("sb");
    case op_gajw: return("gajw");
    case op_savel: return("savel");
    case op_saveh: return("saveh");
    case op_wcnt: return("wcnt");
    case op_shr: return("shr");
    case op_shl: return("shl");
    case op_mint: return("mint");
    case op_alt: return("alt");
    case op_altwt: return("altwt");
    case op_altend: return("altend");
    case op_and: return("and");
    case op_enbt: return("enbt");
    case op_enbc: return("enbc");
    case op_enbs: return("enbs");
    case op_move: return("move");
    case op_or: return("or");
    case op_csngl: return("csngl");
    case op_ccnt1: return("ccnt1");
    case op_talt: return("talt");
    case op_ldiff: return("ldiff");
    case op_sthb: return("sthb");
    case op_taltwt: return("taltwt");
    case op_sum: return("sum");
    case op_mul: return("mul");
    case op_sttimer: return("sttimer");
    case op_stoperr: return("stoperr");
    case op_cword: return("cword");
    case op_clrhalterr: return("clrhalterr");
    case op_sethalterr: return("sethalterr");
    case op_testhalterr: return("testhalterr");
    case op_dup: return("dup");
    case op_move2dinit: return("move2dinit");
    case op_move2dall: return("move2dall");
    case op_move2dnonzero: return("move2dnonzero");
    case op_move2dzero: return("move2dzero");
    case op_unpacksn: return("unpacksn");
    case op_postnormsn: return("postnormsn");
    case op_roundsn: return("roundsn");
    case op_ldinf: return("ldinf");
    case op_fmul: return("fmul");
    case op_cflerr: return("cflerr");
    case op_crcword: return("crcword");
    case op_crcbyte: return("crcbyte");
    case op_bitcnt: return("bitcnt");
    case op_bitrevword: return("bitrevword");
    case op_bitrevnbits: return("bitrevnbits");
    case op_wsubdb: return("wsubdb");
    case op_fpldndbi: return("fpldndbi");
    case op_fpchkerr: return("fpchkerr");
    case op_fpstnldb: return("fpstnldb");
    case op_fpldnlsni: return("fpldnlsni");
    case op_fpadd: return("fpadd");
    case op_fpstnlsn: return("fpstnlsn");
    case op_fpsub: return("fpsub");
    case op_fpldnldb: return("fpldnldb");
    case op_fpmul: return("fpmul");
    case op_fpdiv: return("fpdiv");
    case op_fpldnlsn: return("fpldnlsn");
    case op_fpremfirst: return("fpremfirst");
    case op_fpremstep: return("fpremstep");
    case op_fpnan: return("fpnan");
    case op_fpordered: return("fpordered");
    case op_fpnotfinite: return("fpnotfinite");
    case op_fpgt: return("fpgt");
    case op_fpeq: return("fpeq");
    case op_fpi32tor32: return("fpi32tor32");
    case op_fpi32tor64: return("fpi32tor64");
    case op_fpb32tor64: return("fpb32tor64");
    case op_fptesterr: return("fptesterr");
    case op_fprtoi32: return("fprtoi32");
    case op_fpstnli32: return("fpstnli32");
    case op_fpldzerosn: return("fpldzerosn");
    case op_fpldzerodb: return("fpldzerodb");
    case op_fpint: return("fpint");
    case op_fpdup: return("fpdup");
    case op_fprev: return("fprev");
    case op_fpldnladddb: return("fpldnladddb");
    case op_fpldnlmuldb: return("fpldnlmuldb");
    case op_fpldnladdsn: return("fpldnladdsn");
    case op_fpentry: return("fpentry");
    case op_fpldnlmulsn: return("fpldnlmulsn");
    case op_fpusqrtfirst: return("fpusqrtfirst");
    case op_fpusqrtstep: return("fpusqrtstep");
    case op_fpusqrtlast: return("fpusqrtlast");
    case op_fpurp: return("fpurp");
    case op_fpurm: return("fpurm");
    case op_fpurz: return("fpurz");
    case op_fpur32tor64: return("fpur32tor64");
    case op_fpur64tor32: return("fpur64tor32");
    case op_fpuexpdec32: return("fpuexpdec32");
    case op_fpuexpinc32: return("fpuexpinc32");
    case op_fpuabs: return("fpuabs");
    case op_fpunoround: return("fpunoround");
    case op_fpuchki32: return("fpuchki32");
    case op_fpuchki64: return("fpuchki64");
    case op_fpudivby2: return("fpudivby2");
    case op_fpumulby2: return("fpumulby2");
    case op_fpurn: return("fpurn");
    case op_fpuseterr: return("fpuseterr");
    case op_fpuclrerr: return("fpuclrerr");
    /* T425 instructions */
    case op_start: return "start";
    case op_testhardchan: return "testhardchan";
    case op_testldd: return "testldd";
    case op_testlde: return "testlde";
    case op_testlds: return "testlds";
    case op_teststd: return "teststd";
    case op_testste: return "testste";
    case op_teststs: return "teststs";
    case op_break: return "break";
    case op_clrj0break: return "clrj0break";
    case op_setj0break: return "setj0break";
    case op_testj0break: return "testj0break";
    case op_timerdisableh: return "timerdisableh";
    case op_timerdisablel: return "timerdisablel";
    case op_timerenableh: return "timerenableh";
    case op_timerenablel: return "timerenablel";
    case op_ldmemstartval: return "ldmemstartval";
    case op_pop: return "pop";
    case op_lddevid: return "lddevid";
    /* internal codes, used for debug output */
    case p_infoline	: return("infoline");
    case p_call		: return("call");
    case p_callpv	: return("callpv");
    case p_fnstack	: return("fnstack");
    case p_ldx		: return("ldx");
    case p_stx		: return("stx");
    case p_ldxp		: return("ldxp");
    case p_ret		: return("ret");
    case p_ldpi		: return("ldpi");
    case p_ldpf		: return("ldpf");
    case p_lds		: return("lds");
    case p_sts		: return("sts");
    case p_ldsp		: return("ldsp");
    case p_j		: return("ldc 0 cj");
    case p_noop		: return("noop");
    case p_ldvl		: return("ldvl");
    case p_stvl		: return("stvl");
    case p_ldvlp	: return("ldvlp");
    default: return ("INVALID OP");
   }
}
