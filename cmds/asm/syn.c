/****************************************************************/
/* syn.c                                                        */
/*                                                              */
/* Assembler syntax analyser                                    */
/*                                                              */
/****************************************************************/
/* $Id: syn.c,v 1.6 1994/08/09 16:43:25 al Exp $ */

#include "asm.h"

#define trace if(traceflags&db_syn)_trace

PUBLIC WORD lineno;
PUBLIC WORD etype;

#ifdef __DOS386


PRIVATE WORD exp(void);
PRIVATE WORD exp1(void);
PRIVATE WORD term(void);
PRIVATE WORD tertiary(void);
PRIVATE WORD secondary(void);
PRIVATE WORD primary(void);
PRIVATE WORD combine(WORD op, WORD ltype, WORD rtype, WORD lexp, WORD rexp);

#else /* !__DOS386 */

PRIVATE WORD exp(ellipsis);
PRIVATE WORD exp1(ellipsis);
PRIVATE WORD term(ellipsis);
PRIVATE WORD tertiary(ellipsis);
PRIVATE WORD secondary(ellipsis);
PRIVATE WORD primary(ellipsis);
PRIVATE WORD combine(ellipsis);

#endif /* __DOS386 */

/********************************************************/
/* assemble                                             */
/*                                                      */
/* main assembler loop                                  */
/*                                                      */
/********************************************************/

PUBLIC void initasm()
{
        /* install special symbols */
        toksymref = insert("modnum",FALSE);
        toksym = VMAddr(Symbol,toksymref);
        toksym->type = s_modnum;
        toksym->def.w = 2;                /* allow up to 256 modules */
	toksym->global = 1;
        VMDirty(toksymref);
}

/********************************************************/
/* assemble                                             */
/*                                                      */
/* main assembler loop                                  */
/*                                                      */
/********************************************************/

PUBLIC void assemble()
{
        jmp_buf oldreclev;
        Value expr;
	
	newfile();

        nextsym();

	memcpy(oldreclev,error_level,sizeof(jmp_buf));

        if( setjmp(error_level) != 0 )
        {
		int recovering = TRUE;
		while( recovering )
		{
			nextsym();
			switch( symb )
			{
			case s_direct:
			case s_oper1:
			case s_oper2:
			case s_oper3:
			case s_oper4:
			case s_directive:
			case s_code:
			case s_bss:
			case s_labdef:
 				recovering = FALSE;
#ifdef NEVER
			case s_datasymb:
			case s_commsymb:
			case s_codesymb:
			case s_unbound:
			case s_nl:
			case s_semic:
			case s_comma:
			case s_cr:
#endif
				;
			}
		}
        }

        while( symb != s_eof )
        {
                trace("Main loop symbol = %2lx %lx",symb,tokval.w);
                switch( (int)symb )
                {
                case s_direct:
                {
                        WORD op = tokval.w;
                        expr.w = exp1();
                        gendirect(op,etype,expr.w);
                        break;
                }

                case s_oper1:
                case s_oper2:
                case s_oper3:
                case s_oper4:
                        gencode(&tokval.w,symb&15L);
                        nextsym();
                        break;


                case s_datasymb:
                case s_commsymb:
			warn("Code symbol re-defined as data symbol '%s' - ignored",token);
			nextsym();		
                        break;

                case s_codesymb:
                case s_unbound:
                        if( tokval.w != NULL )
                        {
			    if( !inlib ) warn("Duplicate label definition '%s' - ignored",token);
                        }
                        else {
                            toksym->def.v = deflabel();
                            toksym->type = s_codesymb;
			    toksym->module = curmod;
			    if (!preasm && toksym->referenced) refsymbol_def(toksymref); 
			    VMDirty(toksymref);
                            if( preasm ) genlabdef(toksymref);
                        }
                        nextsym();
                        if( symb == s_colon ) nextsym();
                        else warn("Colon missing - assumed");
                        break;

                case s_labdef:
                        toksymref = insert(token,TRUE);
                        toksym = VMAddr(Symbol,toksymref);
                        toksym->type = s_codesymb;
                        toksym->def.v = deflabel();
                        toksym->module = curmod;
                        VMDirty(toksymref);
			if( preasm ) genlabdef(toksymref);
                        nextsym();
                        break;

                case s_nl:
                        lineno++;
                case s_semic:           /* for compatability */
                case s_comma:
		case s_cr:		/* ignore any CRs which get this far */
                        nextsym();
                        break;

                case s_code:            /* only appears in pre-asm input */
                {
                    WORD n = asm_rdch();
                    int i;
                    trace("P_Code %d",n);
                    for( i = 0 ; i < n ; i++ ) genbyte(asm_rdch());
                    nextsym();
                    break;
                }

                case s_bss:             /* ditto */
                {
                    WORD n = asm_rdch();
                    trace("P_Bss %d",n);
                    genblkb((WORD)s_number,n);
                    nextsym();
                    break;
                }

                case s_directive:

                    switch( (int)tokval.w )
                    {
    
                    case s_byte:
                            do {
                                    nextsym();
                                    if( symb == s_number )
                                    {
					    Value expr;
                                            expr.w = exp();
                                            if( etype != s_number )
                                                warn("Only numeric expressions or strings allowed in BYTE directive");
                                            else {
                                                if( expr.w > 255 || expr.w < -128 )
                                                   warn("Value #%lx out of range for BYTE directive",expr.w);
                                                genbyte(expr.w);
                                            }
                                    }
                                    else
                                    {
                                            if( symb == s_string ) gencode(token,toksize);
                                            else warn("Invalid item '%s' in BYTE directive - ignored",token);
                                            nextsym();
                                    }
                            } while ( symb == s_comma );
                            break;
    
                    case s_word:
                            do {
                                    expr.w = exp1();
                                    genword(etype,expr.w);
                            } while( symb == s_comma );
                            break;
    
                    case s_blkb:
                    {
			    WORD n;
			    Value e; e.w = exp1();
                            n = e.w;
                            while ( symb == s_comma && n > 0 )
                            {
                                    nextsym();
                                    if( symb == s_number )
                                    {
                                            expr.w = exp();
                                            if( etype != s_number )
                                                warn("Only numeric expressions or strings allowed in BYTE directive");
                                            else {
                                                if( expr.w > 255 || expr.w < -128 )
                                                   warn("Value #%lx out of range for BYTE directive",expr.w);
                                                genbyte(expr.w);
                                                n--;
                                            }
                                    }
                                    else
                                    {
                                            if( symb == s_string ) gencode(token,min(n,toksize));
                                            else warn("Invalid item '%s' in BYTE directive - ignored",token);
                                            n -= toksize;
                                            nextsym();
                                    }
                            }
                            if( n > 0 ) genblkb((WORD)s_number,n);
                            break;
                    }
    
                    case s_blkw:
                    {
			    WORD n;
			    Value e; e.w = exp1();
                            n = e.w;
                            while( symb == s_comma && n > 0)
                            {
                                    expr.w = exp1();
                                    genword(etype,expr.w);
                                    n--;
                            }
                            if( n > 0 ) genblkw((WORD)s_number,n);
                            break;
                    }
    
                    case s_init:
                            geninit();
                            nextsym();
                            break;
    
                    case s_align:
                            genalign();
                            nextsym();
                            break;

                    case s_global:
                            nextsym();
                            if( NullRef(toksymref) )
                            {
                                toksymref = insert(token,FALSE);
                                toksym = VMAddr(Symbol,toksymref);
                                toksym->type = s_unbound;
				VMDirty(toksymref);
                            }
                            else movesym(toksymref);
                            genglobal(toksymref);
                            nextsym();
                            break;

                    case s_data:
                    {
                            VMRef sym;
                            nextsym();
                            if( NullRef(toksymref) )
                            {
                                toksymref = insert(token,TRUE);
                                toksym = VMAddr(Symbol,toksymref);
                                toksym->type = s_datasymb;
                                toksym->module = curmod;
                                VMDirty(toksymref);
                            }
                            else /* it has already been referenced */
                                if ( toksym->type == s_unbound ) {
                                        toksym->type = s_datasymb;
					toksym->module = curmod;
					if (!preasm && toksym->referenced) refsymbol_def(toksymref);
					VMDirty(toksymref);
				}
                                else { 
					if( toksym->type == s_datasymb )
					{
					     if( !inlib ) 
					      recover("Duplicate definition of symbol '%s'",token);
					     else
					     {
					     	/* if already defined, re-define locally */
					     	/* to hold space in table	*/
					     	toksymref = insert(token,TRUE);
		                                toksym = VMAddr(Symbol,toksymref);
                	                	toksym->type = s_datasymb;
                        		        toksym->module = curmod;
		                                VMDirty(toksymref);
					     }
					}
					else recover("Illegal symbol in DATA directive: '%s'",token);
				}
                            sym = toksymref;
                            expr.w = exp1();
                            if( etype != s_number )
                                recover("only numeric expressions allowed in DATA directive");
                            gendata(sym,expr.w);
                            break;
                    }

                    case s_common:
                    {
                            VMRef sym;
                            nextsym();
                            if( NullRef(toksymref) )
                            {
                                toksymref = insert(token,TRUE);
                                toksym = VMAddr(Symbol,toksymref);
                                toksym->type = s_commsymb;
                                toksym->module = curmod;
                                VMDirty(toksymref);
                            }
                            else
                                if ( toksym->type == s_unbound ) {
                                        toksym->type = s_commsymb;
					toksym->module = curmod;
					if (!preasm && toksym->referenced) refsymbol_def(toksymref);
					VMDirty(toksymref);
				}
                                else { 
					if( toksym->type != s_commsymb )
						recover("illegal symbol in COMMON directive: '%s'",token);
				}
                            sym = toksymref;
                            expr.w = exp1();
                            if( etype != s_number )
				recover("only numeric expressions allowed in COMMON directive");
                            gencommon(sym,expr.w);
                            break;
                    }

                    case s_module:
                        expr.w = exp1();
                        if( etype != s_number )
                                recover("Numerical expressions only allowed in MODULE directive");
                        genmodule(expr.w);
                        break;

		    case s_size:
			expr.w = exp1();
                        if( etype != s_number )
	                       recover("Numerical expressions only allowed in SIZE directive");
                        gensize(expr.w);
                        break;
                        
                    case s_ref:
			nextsym();
                    	if( symb > s_token ) recover("Illegal operand to REF directive");
			if( symb == s_token )
			{
				toksymref = insert(token,FALSE);
				toksym = VMAddr(Symbol,toksymref);
				toksym->type = s_unbound;
				/*toksym->module = NullVMRef;*/
				VMDirty(toksymref);
			}
			else movesym(toksymref);

			if (!preasm)
			  refsymbol_nondef(toksymref);
			genref(toksymref);
			nextsym();			
			break;
                }
                        break;
                default:
                        warn("Unexpected symbol: '%s' (%02x) - ignored",token,symb);
                        nextsym();
                        break;

                }
        }
        memcpy(error_level,oldreclev,sizeof(jmp_buf));
}

/********************************************************/
/* exp                                                  */
/*                                                      */
/* expression parser, builds a tree corresponding to    */
/* the input expression.                                */
/*                                                      */
/********************************************************/

#undef trace
#define trace if(traceflags&db_expr)_trace

PRIVATE WORD exp1()
{
        nextsym();
        return exp();
}

PRIVATE WORD exp()
{
        Value lexp; lexp.w = term();

        trace("exp symb = %2x '%s'",symb,token);
        while ( symb == s_shl ||
                symb == s_shr )
        {
                WORD ltype = etype;
                WORD op = symb;
                Value rexp; nextsym(); rexp.w = term();
                lexp.w = combine(op,ltype,etype,lexp.w,rexp.w);
        }
        return lexp.w;
}

PRIVATE WORD term()
{
        Value lexp; lexp.w = tertiary();

        trace("term symb = %2x '%s'",symb,token);
        while ( symb == s_and ||
                symb == s_or )
        {
                WORD ltype = etype;
                WORD op = symb;
                Value rexp; nextsym(); rexp.w = tertiary();
                lexp.w = combine(op,ltype,etype,lexp.w,rexp.w);
        }
        return lexp.w;
}

PRIVATE WORD tertiary()
{
        Value lexp; lexp.w = secondary();

        trace("tertiary symb = %2x '%s'",symb,token);
        while ( symb == s_plus ||
                symb == s_minus )
        {
                WORD ltype = etype;
                WORD op = symb;
                Value rexp; nextsym(); rexp.w = secondary();
                lexp.w = combine(op,ltype,etype,lexp.w,rexp.w);
        }
        return lexp.w;
}

PRIVATE WORD secondary()
{
        Value lexp; lexp.w = primary();

        trace("secondary symb = %2x '%s'",symb,token);
        while ( symb == s_mul ||
                symb == s_div ||
                symb == s_rem )
        {
                WORD ltype = etype;
                WORD op = symb;
                Value rexp; nextsym(); rexp.w = primary();
                lexp.w = combine(op,ltype,etype,lexp.w,rexp.w);
        }
        return lexp.w;
}

PRIVATE WORD primary()
{
        WORD op = symb;
        Value res;

        trace("primary symb = %2x '%s'",symb,token);
        switch( (int)op )
        {
        case s_lbra:
                res.w = exp1();
                if ( symb != s_rbra ) recover("Malformed expression");
                nextsym();
                break;

        case s_number:
                etype = s_number;
                res.w = tokval.w;
                nextsym();
                break;

        case s_token:
                toksymref = insert(token,TRUE);
		toksym = VMAddr(Symbol,toksymref);
                op = toksym->type = s_unbound;
                VMDirty(toksymref);
        case s_unbound:
                etype = op;
                res.v = toksymref;
		if (!preasm)
		  refsymbol_nondef(toksymref);                
                nextsym();
                break;

        case s_modnum:
                etype = op;
                res.v = toksymref;
                nextsym();
                break;

        case s_commsymb:
        case s_datasymb:
		etype = s_unbound;
		res.v = toksymref;
		if (!preasm)
		  refsymbol_nondef(toksymref);
		nextsym();
		break;
		
        case s_codesymb:
        	if( preasm )
		{
			etype = op;
	                res.v = toksymref;
	        }
	        else {
			etype = s_coderef;
			res = toksym->def;
		}
		if (!preasm)
		  refsymbol_nondef(toksymref);
		nextsym();
		break;

        case s_at:
                nextsym();
                if( symb > s_token ) recover("'@' operator may only be applied to labels");
		if( symb == s_token )
		{
			toksymref = insert(token,FALSE);
			toksym = VMAddr(Symbol,toksymref);
			toksym->type = s_unbound;
			/*toksym->module = NullVMRef;*/
			VMDirty(toksymref);
		}
		else movesym(toksymref);

		if (!preasm)
		  refsymbol_nondef(toksymref);
                etype = op;
		res.v = toksymref;
		nextsym();
                break;

        case s_minus:
        case s_not:
                res.w = exp1();
                if( etype == s_number ) res.w = domonadic(op,res.w);
                else {
                        res.v = unary( op, etype, res.w );
                        etype = s_monadic;
                }
                break;

        default:
                recover("Unexpected symbol '%s' in expression",token);

        }
        return res.w;
}

PRIVATE WORD combine( op, ltype, rtype, lexp, rexp )
WORD op, ltype, rtype;
WORD lexp, rexp;
{
        Value res;

        if( ltype == s_number && rtype == s_number )
        {
                res.w = dodyadic(op,lexp,rexp);
                etype = s_number;
        }
        else {
                res.v = binary(op,ltype,rtype,lexp,rexp);
                etype = s_expr;
        }
        return res.w;
}

/* End of syn.c */
