/************************************************************************/
/*                                                                      */
/* File: growdata.c                                                     */
/*                                                                      */
/* Changes:                                                             */
/*      V0.0   NHG  06-Jan-89 : Created                                 */
/*                                                                      */
/* Description:                                                         */
/*	Allocates data area offsets and performs some error checking	*/
/*                                                                      */
/*                                                                      */
/* Copyright (c) 1987, Perihelion Software Ltd. All Rights Reserved.    */
/************************************************************************/
/* $Id: growdata.c,v 1.4 1991/06/07 07:30:14 nickc Exp $ */

#include "asm.h"

#define trace if(traceflags&db_growcode)_trace

PRIVATE dataoffset;

/****************************************************************/
/* Procedure: growdata                                          */
/* Description:                                                 */
/*      Main code massaging loop                                */
/*                                                              */
/****************************************************************/

PUBLIC void growdata()
{
    Symbol *s;
    asm_Module *m;

    register Code *c;
    register WORD tag;
	VMRef curblock;

    dataoffset = 0;

	curmod = module0;
	m = VMAddr(asm_Module,curmod);
	curblock = m->start;
	
	VMlock(curblock);

	VMDirty(curblock);
	
	code = VMAddr(Code,curblock);

        simPc = 0;

        for(;;)
        {
	    tag = (c = code)->type;
#if 0
            trace("data: %5ld : %2lx %3ld %2x %8lx",c->loc,tag,(WORD)c->size,c->vtype,c->value.w);
#endif
            c->loc = simPc;
	    

	    if( tag == s_newseg ) 
	    {
	    	VMunlock(curblock);
		curblock = c->value.v;
	    	VMlock(curblock);
	    	code = VMAddr(Code,curblock);
            	VMDirty(curblock);
	    	continue;
	    }

            if( tag == s_end )
	    {
		VMunlock(curblock);
		m = VMAddr(asm_Module,curmod);
                curmod = m->next;
                if( NullRef(curmod) ) break;
                
                m = VMAddr(asm_Module,curmod);

		if( m->id < 0 ) error("Invalid module 1: %x %x %x %d",curmod,m,m->next,m->id);
		curblock = m->start;
		VMlock(curblock);
		code = VMAddr(Code,curblock);
		dataoffset = 0;
            	VMDirty(curblock);
		trace("Module %d %d",m->id,iteration);
                continue;
            }

            if( 0 <= tag && tag <= 15 ) 
            {
            	/* expand DATA refs here ?? */
            	simPc += c->size;
            	goto next;
            }

            switch( (int)tag )
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
                simPc += c->size = asize(simPc);
                break;

	    case s_data:
	    {
		WORD dsize;
		s = VMAddr(Symbol,c->value.v);
		dsize = s->def.w;
		if( s->type == s_datadone ) 
			recover("Error in data mapping");
		s->type = s_datadone;
		s->def.w = dataoffset;
		trace("DATA  : %s at %d size %d",s->name,dataoffset,dsize);
		dataoffset += dsize;
		VMDirty(c->value.v);
		break;
	    }

    	    case s_common:
		s = VMAddr(Symbol,c->value.v);
		if( s->type == s_commsymb )
		{
			WORD dsize = s->def.w;
			s->def.w = dataoffset;
			s->type = s_datadone;
			trace("COMMON: %s at %d size %d",s->name,dataoffset,dsize);
			dataoffset += dsize;
			VMDirty(c->value.v);
		}
		break;

	    case s_newfile:
		strcpy(infile,(char *)c->value.w);
		trace("new file %s",infile);
		break;

	    case s_size:
	    case s_module:
		break;

            } /* end of switch */

next:
            code++;

        } /* end of while */

}


/*  -- End of growcode.c -- */
