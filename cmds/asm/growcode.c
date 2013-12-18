/************************************************************************/
/*                                                                      */
/* File: growcode.c                                                     */
/*                                                                      */
/* Changes:                                                             */
/*      V0.0   NHG  16-May-87 : Created                                 */
/*                                                                      */
/* Description:                                                         */
/*      Massages the code which has been built in the code vector to    */
/*      minimise the length of all prefix encodings. This also performs */
/*      some of the semantic checking of the code.                      */
/*                                                                      */
/*                                                                      */
/* Copyright (c) 1987, Perihelion Software Ltd. All Rights Reserved.    */
/************************************************************************/
/* $Id: growcode.c,v 1.7 1994/08/09 16:43:25 al Exp $ */

#include "asm.h"

#define trace if (traceflags & db_growcode) _trace

#define MaxIterations 1000

PUBLIC WORD 	simPc;
PUBLIC Code *	code;
PUBLIC WORD 	iteration;
PRIVATE 	dataoffset;
PUBLIC WORD 	oldloc;

#ifdef __DOS386

WORD eval(WORD type, WORD earg, WORD loc);
WORD dodyadic(WORD op, WORD lexp, WORD rexp);
WORD domonadic(WORD op, WORD value);

#else /* !__DOS386 */

WORD eval(ellipsis);
WORD dodyadic(ellipsis);
WORD domonadic(ellipsis);

#endif /* __DOS386 */

/****************************************************************/
/* Procedure: growcode                                          */
/* Description:                                                 */
/*      Main code massaging loop                                */
/*                                                              */
/****************************************************************/

PUBLIC void
growcode()
{
  WORD 		oldchanges = 0;
  bool 		growonly = FALSE;
  jmp_buf 	oldreclev;
  WORD 		changes;
  asm_Module *	m;
  

  dataoffset = 0;
  iteration  = 1;
  
  memcpy( oldreclev, error_level, sizeof (jmp_buf) );
  
  do
    {
      register Code *	c;
      register WORD 	tag;
      VMRef 		curblock;

      
      curmod   = module0;
      
      m        = VMAddr( asm_Module, curmod );
      curblock = m->start;
      
      VMlock( curblock );
      
      VMDirty( curblock );
      
      code = VMAddr( Code, curblock );
      
      changes = 0;
      simPc   = 0;
      
      trace( "Iteration = %ld", iteration );
      
      if ( iteration == MaxIterations )
        {
	  warn( "PROBABLE BUG IN GROWCODE, %ld iterations", iteration );
	  
	  break;
        }
      
      /* when an error is detected, longjmp here */
      
      if ( setjmp( error_level ) != 0 )
	code++, errors++;
      
      for (;;)
        {
	  tag = (c = code)->type;
	  
	  oldloc = c->loc;
	  c->loc = simPc;
	  
	  if ( simPc != oldloc )
	    changes++;
	  
	  trace( "code: %5ld : %2lx %3ld %2x %8lx", c->loc, tag, (WORD)c->size, c->vtype, c->value.w );
	  
	  if ( tag == s_newseg ) 
	    {
	      VMunlock( curblock );
	      
	      curblock = c->value.v;
	      
	      VMlock( curblock );
	      
	      code = VMAddr( Code, curblock );
	      
	      VMDirty( curblock );
	      
	      continue;
	    }
	  
	  if ( tag == s_end )
	    {
	      VMunlock( curblock );
	      
	      m = VMAddr( asm_Module, curmod );
	      
	      curmod = m->next;
	      
	      if ( NullRef( curmod ) )
		break;
	      
	      m = VMAddr( asm_Module, curmod );
	      
	      if ( m->id < 0 )
		error("Invalid module 1: %x %x %x %d", curmod, m, m->next, m->id );
	      
	      curblock = m->start;
	      
	      VMlock( curblock );
	      
	      code = VMAddr( Code, curblock );
	      
	      dataoffset = 0;
	      
	      VMDirty( curblock );
	      
	      trace( "Module %d %d", m->id, iteration );
	      
	      continue;
            }
#ifdef LINENO
	  lineno = c->line;
#endif
	  if ( 0 <= tag && tag <= 15 ) /* its a direct instruction */
            {
	      register WORD 	expr;
	      register WORD 	isize;


	      /* if the value is a code symbol, switch it to a direct	*/
	      /* ref to the code, this eliminates an indirect through 	*/
	      /* the symbol table.					*/
	      
	      if (  c->vtype == s_codesymb ) 
		{
		  Symbol *	s = VMAddr( Symbol, c->value.v );
		  
		  c->vtype   = s_coderef;
		  c->value.v = s->def.v;
		}
	      else if ( c->vtype == s_unbound )
		{
		  Symbol *	s = VMAddr( Symbol, c->value.v );
		  
		  if ( s->type == s_codesymb )
		    {
		      c->vtype   = s_coderef;
		      c->value.v = s->def.v;
		    }
		}
	      else if( c->vtype == s_at ) /* also switch @symbol to a number */
		{
		  Symbol *	s = VMAddr( Symbol, c->value.v );
		  
		  if ( NullRef( s->module ) ) 
		    {
		      warn( "DATA label %s undefined - module set to zero", s->name );
		      
		      c->vtype   = s_number;
		      c->value.w = 0;	
		    }
		  else
		    {
		      m = VMAddr( asm_Module, s->module );
		      
		      if ( m->id < 0 )
			{
			  error( "Bad module number for %s m %x (%s) id %x", s->name, s->module, m->file, m->id );
			}
		      
		      c->vtype   = s_number;
		      c->value.w = m->id;
		    }
		}
	      
	      /* if the size of the instruction has been fixed, do not 	*/
	      /* re-calculate.					 	*/
	      
	      if ( c->flags & codeflags_fixed ) 
		{
		  simPc += c->size;
		  
		  goto next;
		}
	      
	      expr = eval( (WORD)c->vtype, c->value.w, (WORD)simPc + c->size );
	      
	      if ( etype == s_datadone )
		{
		  c->vtype   = s_number;
		  c->value.w = expr;
		}
	      
	      isize = pfsize( expr );
	      
	      trace( "Etype = %lx expr = %lx isize = %lx", etype, expr, isize );
	      
	      /* For code references. If the offset is -ve then		*/
	      /* repeat the evaluation with the new code size until	*/
	      /* it converges. If the offset is +ve and has increased 	*/
	      /* in size, allow for this in new offset.			*/
	      
	      if ( etype == s_coderef )
		{
		  if ( expr < 0 )
		    {
		      int 	osize = c->size;
		      int 	i;

		      
		      for (i = 0; i < 10 && isize != osize; i++ )
			{
			  expr  = eval( (WORD)c->vtype, c->value.w, (WORD)simPc + isize );
			  osize = isize;
			  isize = pfsize( expr );
			}
		    }
		}

	      /* We only allow the code to grow, never shrink. Occasionally
		 this means generating values in more bytes than necessary,
		 this is kludged up in genimage.
		 */
	      
	      if ( growonly )
		{
		  if ( isize > c->size )
		    c->size = isize;
		  
		  simPc += c->size;
		}
	      else
		simPc += c->size = isize;
	      
	      goto next;
            }
	  
	  switch ( (int)tag )
            {
	      /* straight bytes */
            case s_bss:
            case s_code:
            case s_literal:
	      simPc += c->size;
	      break;
	      
	      /* single words */
            case s_init:
            case s_word:
	      simPc += TargetBytesPerWord;
	      break;
	      
	      /* padding to word boundary */
	      /* aligns are the only things which are allowed to shrink */
	      
            case s_align:
	      simPc += c->size = asize( simPc );
	      break;
	      
	    case s_newfile:
	      strcpy( infile, (char *)c->value.w );
	      trace( "new file %s", infile );
	      break;
	      
	    case s_data:
	    case s_common:
	    case s_size:
	    case s_module:
	      break;
	      
            } /* end of switch */
	  
	next:
	  code++;
	  
        } /* end of while */
      
      if ( verbose )
	{
	  fprintf( verfd, "Iteration %4ld Changes %ld     \r", iteration, changes );
	  
	  fflush( verfd );
	}
      
      iteration++;
      
      if ( iteration > 2  || oldchanges == changes )
	growonly = TRUE;
      
      oldchanges = changes;
      
    }
  while( (changes > 0 || iteration <= 2 ) && errors == 0  );
  
  memcpy( error_level, oldreclev, sizeof (jmp_buf) );
  
  /* report( "\n%ld Iterations Required", iteration ); */

  return;
}

/****************************************************************/
/* Procedure: eval                                              */
/* Description:                                                 */
/*      Evaluate an expression and return its value, and its    */
/*      type in etype.                                          */
/*                                                              */
/****************************************************************/

#undef trace
#define trace if(traceflags&db_eval)_trace

PUBLIC WORD eval(type,earg,loc)
WORD type,earg,loc;
{
    Value expr;
    Symbol *s = NULL;
    asm_Module *m = NULL;
    
    expr.w = earg;	/* some compilers can't handle union args!! */

    if( type == s_unbound )
    {
    	s = VMAddr(Symbol,expr.v);
    	etype = s->type;
    }
    else etype = type;

    trace("Eval %2lx %8lx %8lx",etype,earg,loc);

    switch( (int)etype )
    {
    case s_number:      /* just a number - return it */
        return expr.w;

    case s_codesymb:    /* code label - return offset from current loc */
    {
	Code *c;
	WORD labloc;
    	/* Problem here: if this pass has pushed the new location	*/
    	/* of the current instruction past that of the instruction	*/
    	/* referenced, the offset here will be negative. This can lead	*/
    	/* to our generating larger instructions than necessary.	*/
        /* If the offset appears to be negative, but the old offset was	*/
        /* positive, keep to the old estimate for now.			*/
        /* This will sort itself out in a future iteration when the 	*/
        /* code gets closer to its eventual size.			*/

	s = VMAddr(Symbol,expr.v);
	c = VMAddr(Code,s->def.v);

        labloc = c->loc;
/*if(labloc <0 || labloc >1000000 ) _trace("bad labloc c = %x s = %s",c,s->name);*/

        if( (labloc < loc) && (labloc > (oldloc+code->size)) ) 
        {
        	return labloc-oldloc-code->size;
        }
        else return labloc - loc;
    }
    
    case s_coderef:
  	{
  		Code *c = VMAddr(Code,expr.v);
  		WORD labloc = c->loc;

		if( (labloc < loc) && (labloc > (oldloc+code->size)) ) 
		{
			return labloc-oldloc-code->size;
		}
		else return labloc - loc;
  	}

    case s_datasymb:
    case s_commsymb:
/*       	s = VMAddr(Symbol,expr.v); */
       	return 1;
        	
    case s_datadone:
    case s_commdone:
    	s = VMAddr(Symbol,expr.v);
	return s->def.w;

    case s_modnum:
    	m = VMAddr(asm_Module,curmod);
	etype = s_number;
	if( m->id < 0 ) error("Invalid module 2: %x %x %x %d",curmod,m,m->next,m->id);
        return m->id;
    
    case s_at:
        etype = s_number;
        s = VMAddr(Symbol,expr.v);
        m = VMAddr(asm_Module,s->module);
       	if( m->id < 0 ) error("Invalid module 3: %x %x %x %d",s->module,m,m->next,m->id);
        return m->id;

    case s_unbound:     /* undefined - an error!!        */
        s = VMAddr(Symbol,expr.v);
        warn("Symbol '%s' undefined - set to zero",s->name);
        s->def.w = 0;
        s->type = s_datadone;
        return 0;

    case s_expr:        /* top of an expression tree     */
    {
    	WORD ntype;
    	WORD ltype, rtype;
    	Value lexp, rexp;
    	WORD lvalue, rvalue;
    	
	Bnode *b = VMAddr(Bnode,expr.v);
	
	ntype = b->ntype;
	ltype = b->ltype; rtype = b->rtype;
	lexp = b->lexp; rexp = b->rexp;


	switch( ltype )
	{
		Symbol *s;

	case s_unbound:
		s = VMAddr(Symbol,lexp.v);
		if( s->type != s_codesymb) break;

	case s_codesymb:
		VMlock(expr.v);
		s = VMAddr(Symbol,lexp.v);
		ltype = b->ltype = s_coderef;
		lexp = b->lexp = s->def;
		VMDirty(expr.v);
		VMunlock(expr.v);
		break;
	}

	switch( rtype )
	{
		Symbol *s;

	case s_unbound:
		s = VMAddr(Symbol,rexp.v);
		if( s->type != s_codesymb) break;

	case s_codesymb:
		VMlock(expr.v);
		s = VMAddr(Symbol,rexp.v);
		rtype = b->rtype = s_coderef;
		rexp = b->rexp = s->def;
		VMDirty(expr.v);
		VMunlock(expr.v);
		break;
	}

        lvalue = eval(ltype,lexp.w,loc); ltype = etype;
        
        rvalue = eval(rtype,rexp.w,loc); rtype = etype;


	trace("Expr: L %2lx %8lx Op %2lx R %2lx %8lx",ltype,lvalue,
		ntype,rtype,rvalue);
		
        if( islabel(ltype) && islabel(rtype) && ltype != rtype )
            recover("Cannot mix Code and Data symbols in same expression");

        /* this ensures that the overall type of an expression reflects any
           label references it contains. */
        etype = ltype<rtype?ltype:rtype;

        return dodyadic(ntype,lvalue,rvalue);
    }

    case s_monadic:
    {
	Unode *u = VMAddr(Unode,expr.v);
	WORD ntype = u->ntype;
	WORD uetype = u->etype;
	Value uexpr;

	uexpr.w = u->expr.w;
	
	switch( uetype )
	{
		Symbol *s;

	case s_unbound:
		s = VMAddr(Symbol,uexpr.v);
		if( s->type != s_codesymb) break;

	case s_codesymb:
		VMlock(expr.v);
		s = VMAddr(Symbol,uexpr.v);
		uetype = u->etype = s_coderef;
		uexpr = u->expr = s->def;
		VMDirty(expr.v);
		VMunlock(expr.v);
		break;
	}

        return domonadic(ntype,eval(uetype,uexpr.w,loc));
    }

    }

    return 0;
}

PUBLIC WORD dodyadic(op,lexp,rexp)
WORD op,lexp,rexp;
{

    switch( (int)op )
    {
        case s_plus : return lexp  +  rexp;
        case s_minus: return lexp  -  rexp;
        case s_mul  : return lexp  *  rexp;
        case s_div  : return lexp  /  rexp;
        case s_rem  : return lexp  %  rexp;
        case s_shl  : return lexp  << rexp;
        case s_shr  : return lexp  >> rexp;
        case s_and  : return lexp  &  rexp;
        case s_or   : return lexp  |  rexp;
        case s_xor  : return lexp  ^  rexp;
    }

    return 0;
}

PUBLIC WORD domonadic(op,value)
WORD op,value;
{
    switch ( (int)op )
    {
        case s_minus: return -value;
        case s_not  : return ~value;
    }

    return 0;
}

/*  -- End of growcode.c -- */
