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
/* $Id: gencsc.c,v 1.4 1991/06/07 07:30:14 nickc Exp $ */

#include "asm.h"

#define trace if(traceflags&db_gencsc)_trace

#define t_rec           0l
#define t_number        1l
#define t_fn            2l
#define t_pfix          3l
#define tagshift        6
#define bytedata        0x3fl

#define f_startlist     0l
#define f_endlist       1l

PRIVATE  WORD nchars = 0;

PRIVATE  void outbyte();
PRIVATE  void outbyte1();
PRIVATE  void flushrec();
PRIVATE  void wbyte();

/****************************************************************/
/* Procedure: gencsc                                            */
/* Description:                                                 */
/*      image file generator                                    */
/*                                                              */
/****************************************************************/

PUBLIC gencsc()
{
        WORD pc = 0;
        int i;
        code = codebase;

        codepos = 0;

        putitem(t_fn, f_startlist);

        cscheader(simPc);

        while ( code->type != s_end )
        {
                WORD tag = code->type;

		if( tag == s_newseg ) 
		{
			code = (struct code *)code->value;
			continue;
		}

                if( pc != code->loc ) error("Phase error in Genimage");

                if( 0 <= tag && tag <= 15 )     /* a direct operation */
                {
                        WORD val = eval((WORD)code->vtype,(WORD)code->value,(WORD)pc+code->size);
                        trace("%8x: Fn %2x %2x %8x",pc,tag,etype,val);
                        /* El Grando Kludgerama - under certain pathalogical
                           curcumstances we have to generate a value in one
                           more byte than strictly necessary to get the code
                           offsets to work. The following line sorts this out
                           for us.
                        */
                        if( pfsize(val) < code->size ) outbyte(f_pfix);

                        encode( (WORD)(tag<<4), val, outbyte1);
                        goto next;
                }

                switch( (int)tag )
                {
                case s_module:
                        curmod = (struct asm_Module *)(code->value);
                        trace("MODULE: %x %d",curmod,curmod->id);
                        break;

		case s_newfile:
			strcpy(infile,code->value);
			break;

                case s_bss:
                        trace("%8x: BSS %d",pc,code->size);
                        for( i = 0; i < code->size ; i++ ) outbyte(0);
                        break;

                case s_literal:
                        trace("%8x: CODE %d",pc,code->size);
                        for( i = 0; i < code->size ; i++ )
                                outbyte(((UBYTE *)(&code->value))[i]);
                        break;

                case s_code:
                        trace("%8x: CODE %d",pc,code->size);
                        for( i = 0; i < code->size ; i++ )
                                outbyte(((UBYTE *)(code->value))[i]);
                        break;

                case s_init:
                        trace("%8x: INIT",pc);
                        if( code->value == NULL ) outword(0);
                        else {
                                WORD next = ((struct code *)(code->value))->loc;
                                outword(next-pc);
                        }
                        break;

                case s_word:
                {
                        WORD val = eval((WORD)code->vtype,(WORD)code->value,(WORD)pc);
                        trace("%8x: WORD %2x %8x",pc,etype,val);
                        if( asize(pc) != 0 ) warn("WORD not on word boundary");
                        /* default to val */
                        outword(val);
                        break;
                }

                case s_align:
                        trace("%8x: ALIGN %d",pc,code->size);
                        for( i = 0 ; i < code->size ; i++ ) outbyte(f_pfix);
                        break;

                case s_global:
                case s_data:
                case s_common:
		case s_size:
                        break;

                }

        next:
                pc += code->size;
                code++;
        }

        flushrec();
        putitem(t_fn, f_endlist);
}

/****************************************************************/
/* cscheader                                                    */
/*                                                              */
/* Generate image file header                                   */
/*                                                              */
/****************************************************************/

PRIVATE cscheader(imagesize)
WORD imagesize;
{
        int i;
        static char *id = "Perihelion Assembler V1.00";

        outword(1l);                     /* interface desc       */
        outbyte(2);

        outword((WORD)strlen(id));            /* id string            */
        for( i = 0 ; i <strlen(id) ; i++ ) outbyte(id[i]);

        outword( 4l );                   /* target               */

        outword( 1l );                   /* version              */

        outword( 100l );                 /* workspace            */

        outword( 0l );                   /* entry point          */

        outword( imagesize );           /* size of code         */

        flushrec();
}

/****************************************************************/
/* putitem                                                      */
/*                                                              */
/* encode an item into ops format                               */
/*                                                              */
/****************************************************************/

PRIVATE putitem( op, opd )
WORD op, opd;
{
        trace("Putitem %d %d",op,opd);
        if( opd > bytedata )
                putstep( opd >> tagshift );
        wbyte((UBYTE)((op<<tagshift) | (opd & bytedata )) );
}

PRIVATE  putstep( opd )
WORD opd;
{
        if( opd > bytedata )
                putstep( opd >> tagshift );
        wbyte((UBYTE)( (t_pfix<<tagshift) | (opd & bytedata )) );
}

/****************************************************************/
/* outword                                                      */
/*                                                              */
/* output a word to image file                                  */
/*                                                              */
/****************************************************************/

PRIVATE  outword(val)
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

PRIVATE  void outbyte(b)
UBYTE b;
{
        if( codepos == 254 ) flushrec();
        codevec[codepos] = b;
	codepos++;
}

PRIVATE  void flushrec()
{
        int i;
        if( codepos == 0 ) return;
        trace("Flushrec %d",codepos);
        putitem(t_rec, codepos );
        for( i = 0 ; i < codepos ; i++ ) wbyte(codevec[i]);
        codepos = 0;
}

PRIVATE  void wbyte(b)
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

/*  -- End of gencsc.c -- */
