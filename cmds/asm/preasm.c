/************************************************************************/
/*                                                                      */
/* File: preasm.c                                                       */
/*                                                                      */
/* Changes:                                                             */
/*      NHG  20-Jun-87  : Created                                       */
/*                                                                      */
/* Description:                                                         */
/*      Generate pre-assembly output from the code vector               */
/*                                                                      */
/*                                                                      */
/* Copyright (c) 1987, Perihelion Software Ltd. All Rights Reserved.    */
/************************************************************************/
/* $Id: preasm.c,v 1.5 1994/08/09 16:43:25 al Exp $ */

#include "asm.h"

#define trace if(traceflags&db_preasm)_trace

extern WORD etype;
extern WORD DataSize;
extern UBYTE *codevec;
extern WORD codepos;
extern WORD symb;
extern Value tokval;
extern WORD lineno;

#ifdef __DOS386

static void wrch(char ch);
static void preval(WORD type, WORD earg);
static void predyadic(WORD op);
static void premonadic(WORD op);

#else /* !__DOS386 */

static void wrch(ellipsis);
static void preval(ellipsis);
static void predyadic(ellipsis);
static void premonadic(ellipsis);

#endif /* __DOS386 */

/****************************************************************/
/* parsepreasm                                                  */
/*                                                              */
/* convert preasm bytes into lexical tokens                     */
/*                                                              */
/****************************************************************/

PUBLIC void parsepreasm(ch)
WORD ch;
{
        trace("Parsepreasm %2x",ch);
        if( s_direct <= ch && ch <= (s_direct+15) )
        {
                symb = s_direct;
                tokval.w = (ch & 0x0f)<<4;
                return;
        }
        if( ch < s_firstdir ) symb = ch;
        else { symb = s_directive; tokval.w = ch; }
}

/****************************************************************/
/* Procedure: genpreasm                                         */
/* Description:                                                 */
/*      pre-assembler code generator                            */
/*                                                              */
/****************************************************************/

PUBLIC void genpreasm()
{
        WORD pc = 0;
        int i;
	Code *c;
	Symbol *s;
	asm_Module *m;
	VMRef curblock;

	curmod = module0;
	m = VMAddr(asm_Module,curmod);
	curblock = m->start;
	
	VMlock(curblock);

	code = VMAddr(Code,curblock);

        lineno = 1;
        codepos = 0;

	for(;;)
        {
                WORD tag = (c = code)->type;

trace("preasm c = %x %x %x %x",c,tag,c->vtype,c->value.v);
		if( tag == s_newseg ) 
		{
	    		VMunlock(curblock);
			curblock = c->value.v;
		    	VMlock(curblock);
	    		code = VMAddr(Code,curblock);
		    	continue;
		}

	        if( tag == s_end )
		{
			VMunlock(curblock);
			m = VMAddr(asm_Module,curmod);
	                curmod = m->next;
	                if( NullRef(curmod) ) break;

			m = VMAddr(asm_Module,curmod);
			curblock = m->start;
			VMlock(curblock);
			code = VMAddr(Code,curblock);

	                continue;
		}

#ifdef LINENO
                while( lineno < c->line ) /* preserve line numbering */
                {
                        wrch('\n');
                        lineno++;
                }
#endif
/*                if( pc != c->loc ) error("Phase error in Preasm");*/

                if( 0 <= tag && tag <= 15 )     /* a direct operation */
                {
                        trace("%x8: Direct %2x",pc,tag);
                        wrch((UBYTE)tag+s_direct);
                        preval((WORD)c->vtype,c->value.w);
                        goto next;
                }

                switch( (int)tag )
                {
                case s_module:
                        trace("%8x: MODULE",pc);
                        wrch((UBYTE)s_module);
                        m = VMAddr(asm_Module,curmod);
                        fprintf(outfd,"%ld",m->id);
			break;
			
                case s_labdef:
			trace("%8x: LABDEF %x",pc,c->value.w);
                        wrch('\n');
                        preval((WORD)s_codesymb,c->value.w);
                        wrch(':');
                        break;

                case s_bss:
                        trace("%8x: BSS %d",pc,c->size);
                        wrch((UBYTE)s_bss); wrch((UBYTE)c->size);
                        break;

                case s_literal:
                        trace("%8x: CODE %d",pc,c->size);
                        wrch((UBYTE)s_code); wrch((UBYTE)c->size);
                        for( i = 0; i < c->size ; i++ )
                                wrch(((UBYTE *)(&c->value.w))[i]);
                        break;

                case s_code:
                {
                	UBYTE *v = VMAddr(UBYTE,c->value.v);
                        trace("%8x: CODE %d",pc,c->size);
                        wrch((UBYTE)s_code); wrch((UBYTE)c->size);
                        for( i = 0; i < c->size ; i++ ) wrch(v[i]);
                        break;
		}
		
                case s_init:
                        trace("%8x: INIT",pc);
                        wrch((UBYTE)s_init);
                        break;

                case s_word:
                {
                        wrch((UBYTE)s_word);
                        trace("%8x: WORD",pc);
                        preval((WORD)c->vtype,c->value.w);
                        break;

                case s_align:
                        wrch((UBYTE)s_align);
                        trace("%8x: ALIGN",pc);
                        break;

                case s_global:
			trace("%8x: GLOBAL",pc);
                        wrch((UBYTE)s_global);
                        preval((WORD)c->vtype,c->value.w);
                        break;

                case s_data:
			trace("%8x: DATA",pc);
                        wrch((UBYTE)s_data);
                        s = VMAddr(Symbol,c->value.v);
                        preval((WORD)c->vtype,c->value.w);
                        fprintf(outfd," %ld",s->def.w);
                        break;

                case s_common:
			trace("%8x: COMMON",pc);
                        wrch((UBYTE)s_common);
                        s = VMAddr(Symbol,c->value.v);
                        preval((WORD)c->vtype,c->value.w);
                        fprintf(outfd," %ld",s->def.w);
                        break;
                }
		case s_size:
			trace("%8x: SIZE",pc);
			wrch((UBYTE)s_size);
			fprintf(outfd,"%ld",c->value.w);
			break;

                case s_ref:
			trace("%8x: REF",pc);
                        wrch((UBYTE)s_ref);
                        preval((WORD)c->vtype,c->value.w);
                        break;

                }

        next:
                pc += c->size;
                code++;
        }
}

PRIVATE void wrch(ch)
char ch;
{
#ifdef MWC
        binputc(ch,outfd);
#else
        putc(ch,outfd);
#endif
}

PRIVATE void preval(type,earg)
WORD type,earg;
{
    Value expr;
    Symbol *s;
        
    expr.w = earg;

    if( type == s_unbound )
    {
	s = VMAddr(Symbol,expr.v);
	etype = s->type;
    }
    else etype = type;
    
    trace("PrEval %2x %8x",etype,earg);

    switch( (int)etype )
    {
    case s_number:      /* just a number */
        fprintf(outfd,"%ld",expr.w);
        return;

    case s_at:
        wrch('@');

    case s_modnum:
    case s_unbound:
    case s_label:
    case s_datasymb:
    case s_commsymb:
    case s_coderef:
    case s_codesymb:    /* label  */
	s = VMAddr(Symbol,expr.v);
        fprintf(outfd,"%s",s->name);
        return;

    case s_expr:        /* top of an expression tree     */
    {
    	Bnode *b = VMAddr(Bnode,expr.v);
    	WORD ntype = b->ntype;
    	WORD rtype = b->rtype;
    	WORD rexp = b->rexp.w;
trace("expr l %x %x r %x %x",b->ltype,b->lexp.w,rtype,rexp);
        wrch('(');
        preval((WORD)b->ltype,b->lexp.w);
        predyadic(ntype);
        preval(rtype,rexp);
        wrch(')');
        return;
    }

    case s_monadic:
    {
    	Unode *u = VMAddr(Unode,expr.v);
    	WORD uetype = u->etype;
    	WORD uexpr = u->expr.w;
        wrch('(');
        premonadic((WORD)u->ntype);
        preval(uetype,uexpr);
        wrch(')');
        return;
    }
    
    default: recover("Unexpected symbol during tokenisation %x",etype);
    
    }

}

PRIVATE void predyadic(op)
WORD op;
{
    switch( (int)op )
    {
        case s_plus : wrch('+'); return;
        case s_minus: wrch('-'); return;
        case s_mul  : wrch('*'); return;
        case s_div  : wrch('/'); return;
        case s_rem  : wrch('%'); return;
        case s_shl  : wrch('<'); return;
        case s_shr  : wrch('>'); return;
        case s_and  : wrch('&'); return;
        case s_or   : wrch('|'); return;
        case s_xor  : wrch('^'); return;
    }
}

PRIVATE void premonadic(op)
WORD op;
{
    switch ( (int)op )
    {
        case s_minus: wrch('-'); return;
        case s_not  : wrch('~'); return;
    }
}


/*  -- End of preasm.c -- */
