/************************************************************************/
/*                                                                      */
/* File: dbdecode.c                                                     */
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
#include <helios.h>
#include <queue.h>

#include "dbdecode.h"
#include "debug.h"

#define New(_type) (_type *)malloc(sizeof(_type))

#define swap(x) (x)
#define min(x,y) (x<y?x:y)

WORD function;
WORD operand;

static UBYTE ivec[8];           /* buffer for decoded instructions */

struct List modlist;

struct module *newmodule();
struct proc   *newproc();
struct symb   *newsym();
struct symb   *nearsym();

extern WORD isize;
extern WORD *ibuf;

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
/* dodisam                                                      */
/*                                                              */
/* disassemble a single instruction                             */
/****************************************************************/

dodisasm()
{
        WORD loc = curpos;
        WORD i;
        WORD ilen = 0;
        struct module *m = nearsym(&modlist,loc);
        struct proc *p = m==NULL?NULL:nearsym(&m->procsyms,loc);
        struct symb *s = p==NULL?NULL:nearsym(&p->codesyms,loc);
        WORD nextlab;

        function = 0;

        /* see if we have a label here */
        if ( s != NULL ) nextlab = s->value;

        while( nextlab == loc )
        {
                fprintf(outfd,"                      ");
                fprintf(outfd,"%s:\n",s->name);
                s = (struct symb *)(s->node.Next);
                if( s==NULL ) {
                        p = (struct symb *)(p->node.Next);
                        if( p == NULL ) {
                                m = (struct module *)(m->node.Next);
                                if( m == NULL ) break;
                                p = (struct proc *)(m->procsyms.Head);
                        }
                        s = (struct symb *)(p->codesyms.Head);
                }
                nextlab = s->value;
        }

        /* try to concatenate any NOPs */
        if( ilen == 0 )
        {
                while(  (ivec[ilen++] = gbyte()) == 0x20 )
                {
                        if( ilen == 8 ) goto lab1;
                        function = -1;
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
                fprintf(outfd,"%s %8x    ",directfns[op],opd);
                switch( op )
                {
                case f_call:
                case f_j:
                case f_cj:
                        codeopd( opd, loc );
                        break;
                case f_ldnl:
                case f_stnl:
                case f_ldnlp:
                        dataopd( opd, loc );
                        break;
                case f_ldl:
                case f_stl:
                case f_ldlp:
                        stackopd( opd, loc );
                        break;
                case f_ldc:
                        codeopd( opd, loc );
                        putc(' ',outfd);
                        dataopd( opd, loc );
                        putc(' ',outfd);
                        stackopd( opd, loc );
                        break;
                }

        }
        else if( 0 <= opd && opd <= 0xac && oper[opd] != 0)
                fprintf(outfd,"%s",oper[opd]);
                else fprintf(outfd,"UNKNOWN %2x %8x",op,opd);

}

codeopd( opd, loc )
WORD opd, loc;
{
        WORD dest = loc+opd;
        struct module *m = nearsym(&modlist,dest);
        struct proc *p = m==NULL?NULL:nearsym(&m->procsyms,dest);
        struct symb *s = p==NULL?NULL:nearsym(&p->codesyms,dest);
        if( s != NULL )
        {
                WORD diff = dest-s->value;
                if( diff == 0 )
                     fprintf(outfd,"%s",s->name);
                else fprintf(outfd,"%s+%x",s->name,diff);
        }
        else fprintf(outfd,"%8x",dest);
}

dataopd( opd, loc )
WORD opd, loc;
{
        struct module *m = nearsym(&modlist,loc);
        struct symb *s = m==NULL?NULL:nearsym(&m->staticsyms,opd);
        if( s != NULL )
        {
                WORD diff = opd-s->value;
                if( diff == 0 )
                     fprintf(outfd,"%s",s->name);
                else fprintf(outfd,"%s+%x",s->name,diff);
        }
}

stackopd( opd, loc )
WORD opd, loc;
{
        struct module *m = nearsym(&modlist,loc);
        struct proc *p = m==NULL?NULL:nearsym(&m->procsyms,loc);
        struct symb *s = p==NULL?NULL:nearsym(&p->stacksyms,opd);
        if( s != NULL )
        {
                WORD diff = opd-s->value;
                if( diff == 0 )
                     fprintf(outfd,"%s",s->name);
                else fprintf(outfd,"%s+%x",s->name,diff);
        }
}

/****************************************************************/
/* symbol table stuff                                           */
/*                                                              */
/****************************************************************/

initsym()
{
        struct module *m;
        InitList(&modlist);
        m = newmodule("System",MinInt,MemStart-MinInt);
        newproc(&m->procsyms,"System",MinInt);
}

defsyms()
{
        struct module *m = (struct module *)(modlist.Head);
        struct proc *p = NULL;
        int i;
        for( i = 0 ; i<(isize>>2); i++ )
        {
                switch( swap(ibuf[i]) )
                {
                default: break;

                case t_module:
                        m->size = i-m->base;
                        m = newmodule(&ibuf[i+2],LoadBase+(i<<2),swap(ibuf[i+1]));
                        break;

                case t_proc:
                        p = newproc(&m->procsyms,&ibuf[i+2],LoadBase+4+(i<<2)+swap(ibuf[i+1]));
                        newsym(&p->codesyms,&ibuf[i+2],LoadBase+4+(i<<2)+swap(ibuf[i+1]));
                        break;

                case t_code:
                        newsym(&p->codesyms,&ibuf[i+2],LoadBase+4+(i<<2)+swap(ibuf[i+1]));
                        break;

                case t_stack:
                        newsym(&p->stacksyms,&ibuf[i+2],swap(ibuf[i+1]));
                        break;

                case t_static:
                        newsym(&m->staticsyms,&ibuf[i+2],swap(ibuf[i+1]));
                        break;
                }
        }
}

struct module *newmodule(name,base,size)
char *name;
WORD base, size;
{
        struct module *m = New(struct module);

        if( m == NULL ) error("Insufficient memory");

        strcpy(&m->name,name);
        m->base = base;
        m->size = size;

        InitList(&m->procsyms);
        InitList(&m->staticsyms);

        AddTail(&modlist,m);
        return m;
}

struct proc *newproc(list,name,value)
struct List *list;
char *name;
WORD value;
{
        struct proc *p = New(struct proc);

        if( p == NULL ) error("Insufficient memory");

        strcpy(&p->name,name);
        p->value = value;

        InitList(&p->codesyms);
        InitList(&p->stacksyms);

        addsym(list,p);

        return p;
}

struct symb *newsym(list,name,value)
struct List *list;
char *name;
WORD value;
{
        struct symb *s = New(struct symb);

        if( s == NULL ) error("Insufficient memory");

        strcpy(&s->name,name);
        s->value = value;

        addsym(list,s);

        return s;
}

addsym(list,sym)
struct List *list;
struct symb *sym;
{
        struct Node *node = list->Head;
        while( node->Next != NULL && ((struct symb *)node)->value <= sym->value )
                node = node->Next;
        PreInsert(node,sym);
}

struct symb *nearsym(list,value)
struct List *list;
WORD value;
{
        struct Node *node = list->Tail;
        while( node->Prev != NULL )
        {
                if( ((struct symb *)node)->value <= value )
                        return (struct symb *)node;
                node = node->Prev;
        }
        return NULL;
}

struct symb *findsym(list,value)
struct List *list;
WORD value;
{
        struct Node *node = list->Tail;
        while( node->Prev != NULL )
        {
                if( ((struct symb *)node)->value == value )
                        return (struct symb *)node;
                node = node->Prev;
        }
        return NULL;
}


/*  -- End of dbdecode.c -- */
