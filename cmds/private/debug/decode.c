/************************************************************************/
/*                                                                      */
/* File: decode.c                                                       */
/*                                                                      */
/* Changes:                                                             */
/*      NHG  10-May-87  : Created                                       */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/*      Decode bytes from an instruction stream. Also single instruction*/
/*      disassembly                                                     */
/*                                                                      */
/* Copyright (c) 1987, Perihelion Software Ltd. All Rights Reserved.    */
/************************************************************************/

#include <stdio.h>
#include <types.h>
#include <queue.h>
#include <decode.h>
#include <loadfile.h>

WORD function;
WORD operand;

static UBYTE ivec[8];           /* buffer for decoded instructions */

#include "optab.h"

/****************************************************************/
/* Procedure: decode                                            */
/*                                                              */
/* Decode bytes into function and operand                       */
/****************************************************************/


decode()
{
        WORD i = 0;
        WORD byte;
        operand = 0;
        for(;;)
        {
                byte = gbyte();
                ivec[i++] = byte;
                function = byte>>4;
                operand = (operand << 4) | (byte & 0xf);
                switch ( function )
                {
                case f_nfix: operand = ~operand;
                case f_pfix: break;
                default:
                             return;
                }
        }

}

/****************************************************************/
/* Procedure: disasm                                            */
/*                                                              */
/* Generate the text of the instruction plus its argument (if   */
/* any) to outfd.                                               */
/****************************************************************/

disasm( loc, op, opd )
WORD loc, op,opd;
{
        if( 0 <= op && op <= 0xe ) {
                fprintf(outfd,"%s ",directfns[op]);
                printopd(loc, op, opd);
        }
        else if( 0 <= opd && opd <= 0xac && oper[opd] != 0)
                fprintf(outfd,"%s",oper[opd]);
                else fprintf(outfd,"UNKNOWN %2x %8x",op,opd);

}

/****************************************************************/
/* dodisam                                                      */
/*                                                              */
/* disassemble a single instruction                             */
/****************************************************************/

dodisasm()
{
        WORD loc = curpos;
        WORD i;
        WORD ilen = 0;
        WORD nextfix = curfix->loc;
        WORD nextlab = curcode->value;

        function = 0;

        /* see if we have a label here */
        while( nextlab == loc )
        {
                fprintf(outfd,"                      ");
                fprintf(outfd,"%s:\n",curcode->name);
                curcode = (struct symb *)(curcode->node.next);
                nextlab = curcode->value;
        }

        /* see if we have a fixup at this loc */
        if( nextfix == loc )
        {
                if( curfix->op == t_long )
                        while( ilen < 4 ) ivec[ilen++] = gbyte();
                function = -2;
                nextfix = ((struct fix *)(curfix->node.next))->loc;
        }

        /* try to concatenate any NOPs */
        if( ilen == 0 )
        {
                while(  (ivec[ilen++] = gbyte()) == 0x20 )
                {
                        if( curpos == nextfix ) goto lab1;
                        if( ilen == 8 ) goto lab1;
                        if( function != -2 ) function = -1;
                }
                ungbyte(); ilen--;
        }
    lab1:

        /* and finally decode the instruction */
        if( ilen == 0 ) { decode(); ilen = curpos-loc; }

        fprintf(outfd,"%8x: ",loc);

        for( i = 0 ; i < min(ilen,4) ; i++ )
                fprintf(outfd,"%02x ",ivec[i]);

        for( ; i < 4 ; i++ ) fprintf(outfd,"   ");

        fprintf(outfd,"        ");

        switch( function )
        {
        case -1:
                fprintf(outfd,"nop");
                break;

        case -2:
                if( curfix->op == t_long ) {
                        fprintf(outfd,"word  ");
                        printfix(curfix);
                }
                else disasm(curpos, curfix->op, 0);
                curfix = (struct fix *)(curfix->node.next);
                break;

        default:
                disasm(curpos, function, operand);
        }

        putc('\n',outfd);

        if( i < ilen )
        {
                fprintf(outfd,"        : ");
                for( ; i < ilen ; i++ )
                        fprintf(outfd,"%02x ",ivec[i]);
                putc('\n',outfd);
        }

}

printopd(loc, op, opd)
WORD loc, op, opd;
{
        switch( op )
        {
        case f_call:
        case f_j:
        case f_ldc:
        case f_cj:
            if( function != -2 )
            {
                    WORD dest = loc+opd;
                    fprintf(outfd,"%8x",opd);
                    {
                            struct symb *s = nearsym(&codes,dest);
                            if( s != NULL )
                            {
                                    WORD diff = dest-s->value;
                                    if( diff == 0 )
                                         fprintf(outfd,"    %s",s->name);
                                    else fprintf(outfd,"    %s+%x",s->name,diff);
                            }
                            else fprintf(outfd,"    %8x",dest);
                    }
            }
            else printfix(curfix);
            return;

        case f_ldnl:
        case f_ldnlp:
        case f_stnl:
            if( function != -2 )
            {
                    struct symb *s = nearsym(&datas,opd);
                    fprintf(outfd,"%8x",opd);
                    if( s != NULL )
                    {
                            WORD diff = opd-s->value;
                            if( diff == 0 )
                                 fprintf(outfd,"    %s",s->name);
                            else fprintf(outfd,"    %s+%x",s->name,diff);
                    }
            }
            else printfix(curfix);
            return;

        default:
                fprintf(outfd,"%8x",opd);
                return;
        }
}

printfix(f)
struct fix *f;
{
        struct symb *s;
        switch( f->type )
        {
        case t_codefix:
                    s = findsym(&refs,curfix->value);
                    if( s != NULL ) fprintf(outfd," CODEFIX    %s",s->name);
                    break;

        case t_datafix:
                    s = findsym(&refs,curfix->value);
                    if( s != NULL ) fprintf(outfd," DATAFIX    %s",s->name);
                    break;

        case t_init:      fprintf(outfd,"    INIT"); break;
        case t_maininit:  fprintf(outfd," MAININIT"); break;
        case t_staticfix: fprintf(outfd," STATICFIX"); break;
        case t_modnum:    fprintf(outfd,"  MODNUM"); break;
        case t_limit:     fprintf(outfd,"   LIMIT"); break;
        }
}

/*  -- End of decode.c -- */
