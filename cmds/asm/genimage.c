/************************************************************************/
/*                                                                      */
/* File: genimage.c                                                     */
/*                                                                      */
/* Changes:                                                             */
/*      NHG  07-July-87  : Created                                      */
/*                                                                      */
/* Description:                                                         */
/*      Generate an image file from the code vector                     */
/*                                                                      */
/*                                                                      */
/* Copyright (c) 1987, Perihelion Software Ltd. All Rights Reserved.    */
/************************************************************************/
/* $Id: genimage.c,v 1.7 1994/08/09 16:43:25 al Exp $ */

#include "asm.h"

#define trace if(traceflags&db_genimage)_trace

PRIVATE WORD nchars = 0;

#ifdef __DOS386

PRIVATE void outbyte(UBYTE b);
PRIVATE void outbyte1(WORD b);
PRIVATE void imageheader(WORD imagesize);
PRIVATE void outword(WORD val);

#else /* !__DOS386 */

PRIVATE void outbyte(ellipsis);
PRIVATE void outbyte1(ellipsis);
PRIVATE void imageheader(ellipsis);
PRIVATE void outword(ellipsis);

#endif /* __DOS386 */

/****************************************************************/
/* Procedure: genimage                                          */
/* Description:                                                 */
/*      image file generator                                    */
/*                                                              */
/****************************************************************/
PUBLIC void genimage()
{
        WORD pc = 0;
        int i;
        register Code *c;
        register WORD tag;
        asm_Module *m;
	VMRef curblock;

	curmod = module0;
	m = VMAddr(asm_Module,curmod);
	curblock = m->start;
	
	VMlock(curblock);

	code = VMAddr(Code,curblock);

	VMDirty(curblock);
	
        imageheader(simPc+4);		/* +4 for trailing 0 */

        codepos = 0;

        for(;;)
        {
        	tag = (c = code)->type;
        	
        	oldloc = c->loc;
        	    
		if( tag == s_newseg ) 
		{
	    		VMunlock(curblock);
			curblock = c->value.v;
		    	VMlock(curblock);
	    		code = VMAddr(Code,curblock);
	    		VMDirty(curblock);
		    	continue;
		}

		if ( tag == s_end )
		{
			VMunlock(curblock);
			m = VMAddr(asm_Module,curmod);
	                curmod = m->next;
	                if( NullRef(curmod) ) break;

	                m = VMAddr(asm_Module,curmod);

			curblock = m->start;
			VMlock(curblock);
			code = VMAddr(Code,curblock);
			VMDirty(curblock);
        	        continue;
		}

                if( pc != code->loc ) error("Phase error in Genimage");

                if( 0 <= tag && tag <= 15 )     /* a direct operation */
                {
			WORD pf;
                        WORD val = eval((WORD)code->vtype,code->value.w,(WORD)pc+code->size);
                        trace("%8x: Fn %2x %2x %8x",pc,tag,etype,val);
                        /* El Grando Kludgerama - under certain pathalogical
                           curcumstances we have to generate a value in
                           more bytes than strictly necessary to get the code
                           offsets to work. The following line sorts this out
                           for us.
                        */
                        pf=pfsize(val);

                        
                        if( pf < c->size ) 
			{
if(traceflags&db_kludge)
	_trace("%8x: Kludge fix %d -> %d type %d value %d vtype %d",pc,
		pfsize(val),(WORD)code->size,tag,val,(WORD)code->vtype);
				waste += c->size-pf;
				while( pf++ < code->size ) outbyte1(f_pfix);
			}
			else if( pf > code->size ) 
			{
_trace("pf %d c->size %d; code %x val %x",pf,c->size,tag,val);
				error("Code size error");
			}
			
                        encode( (WORD)(tag<<4), val, outbyte1);
                        goto next;
                }

                switch( (int)tag )
                {
		case s_newfile:
			strcpy(infile,(char *)code->value.w);
			break;

                case s_bss:
                        trace("%8x: BSS %d",pc,(WORD)code->size);
                        for( i = 0; i < code->size ; i++ ) outbyte(0);
                        break;

                case s_literal:
                        trace("%8x: CODE %d",pc,(WORD)code->size);
                        for( i = 0; i < code->size ; i++ )
                                outbyte(((UBYTE *)(&code->value.w))[i]);
                        break;

                case s_code:
                {
                	UBYTE *v = VMAddr(UBYTE,code->value.v);
                        trace("%8x: CODE %d",pc,(WORD)code->size);
                        for( i = 0; i < code->size ; i++ )
                                outbyte(v[i]);
                        break;
		}
		
                case s_init:
                        trace("%8x: INIT",pc);
                        if( NullRef(code->value.v) ) outword(0L);
                        else {
                                WORD next = VMAddr(Code,code->value.v)->loc;
                                outword((WORD)(next-pc));
                        }
                        break;

                case s_word:
                {
                        WORD val = eval((WORD)code->vtype,code->value.w,(WORD)pc);
                        trace("%8x: WORD %2x %8x",pc,etype,val);
                        if( asize(pc) != 0 ) warn("WORD not on word boundary");
                        /* default to val */
                        outword(val);
                        break;
                }

                case s_align:
                        trace("%8x: ALIGN %d",pc,(WORD)code->size);
                        waste += code->size;
                        for( i = 0 ; i < code->size ; i++ ) outbyte1(f_pfix);
                        break;

		case s_module:
                case s_global:
                case s_data:
                case s_common:
		case s_size:
		case s_ref:
                        break;

                }

        next:
                pc += code->size;
                code++;
        }
        
       outword(0L);	/* image trailer, replaces end.p */
}

/****************************************************************/
/* imageheader                                                  */
/*                                                              */
/* Generate image file header                                   */
/*                                                              */
/****************************************************************/

PRIVATE void imageheader(imagesize)
WORD imagesize;
{
        outword((WORD)i_program);
        outword(0L);
        outword(imagesize);
}

/****************************************************************/
/* outword                                                      */
/*                                                              */
/* output a word to image file                                  */
/*                                                              */
/****************************************************************/

PRIVATE void outword(val)
WORD val;
{
        int i;
        for( i = 0 ; i < 32 ; i+=8 ) outbyte((UBYTE)(val>>i));
}

/****************************************************************/
/* Procedure: outbyte                                           */
/*                                                              */
/*      output a byte to image file                             */
/*                                                              */
/****************************************************************/

PRIVATE void outbyte1(b)
WORD b;
{
	outbyte((UBYTE)b);
}

PRIVATE void outbyte(b)
UBYTE b;
{
        if( hexopt )
        {
                if( (nchars % 16) == 0 ) putc('\n',outfd);
                fprintf(outfd,"%02x ",(UBYTE)b);
        }
        else {
                putc(b,outfd);
        }
        nchars++;
}

/*  -- End of genimage.c -- */
