/************************************************************************/
/*                                                                      */
/* File: loadfile.c                                                     */
/*                                                                      */
/* Changes:                                                             */
/*      NHG  25-May-87  : Created                                       */
/*                                                                      */
/* Description:                                                         */
/*      File loader, puts all symbols and fixes in the appropriate list */
/* and the code is output via the pbyte function.                       */
/*                                                                      */
/*                                                                      */
/* Copyright (c) 1987, Perihelion Software Ltd. All Rights Reserved.    */
/************************************************************************/

#include <stdio.h>
#include <types.h>
#include <queue.h>
#include <loadfile.h>

#define New(_type) ((_type *)malloc(sizeof(_type)))

FILE *filefd;

struct List codes;
struct List datas;
struct List refs;
struct List fixes;

WORD refnum = 0;

/****************************************************************/
/* Procedure: initload                                          */
/*                                                              */
/****************************************************************/

initload()
{
        InitList(&codes);
        InitList(&datas);
        InitList(&refs);
        InitList(&fixes);
}

/****************************************************************/
/* Procedure: loadfile                                          */
/*                                                              */
/****************************************************************/

loadfile(name)
STRING name;
{
        WORD tag;

        filefd = fopen(name,"r");
        if( filefd <= 0 ) error("Cannot open '%s' for input",name);


        while( (tag = decodetag()) != t_eof )
        {

                if( 0 <= tag && tag <= 15 )
                {
                        dofix(tag);
                        continue;
                }

                switch( tag )
                {
                default:
                        error("Invalid tag %d in file '%s'",tag,name);

                case t_code:
                {
                        WORD size = decodetag();
                        int i;
                        for( i = 0; i < size; i++ ) pbyte(getbyte());
                        break;
                }

                case t_codesymb:
                {
                        struct symb *s = New(struct symb);
                        decodestr(s->name);
                        s->value = modbase + decodetag();
                        addsym(&codes,s);
                        break;
                }

                case t_datasymb:
                {
                        struct symb *s = New(struct symb);
                        decodestr(s->name);
                        s->value = decodetag();
                        addsym(&datas,s);
                        break;
                }

                case t_ref:
                {
                        struct symb *s = New(struct symb);
                        decodestr(s->name);
                        s->value = refnum++;
                        AddTail(&refs,s);
                        break;
                }

                case t_long:
                        dofix(tag);
                        break;
                }
        }
}

dofix(tag)
WORD tag;
{
        WORD offset = decodetag();
        WORD patchtag = decodetag();
        struct fix *f = New(struct fix);
        f->op = tag;
        f->type = patchtag;
        f->loc = modbase+offset;
        addfix(&fixes,f);
        switch( patchtag )
        {
        case t_codefix: f->value = decodetag(); break;
        case t_datafix: f->value = decodetag();
                                   decodetag(); break; /* for now */
        case t_staticfix:
        case t_modnum:
        case t_limit:
        case t_init:
        case t_maininit:                        break;
        }
/* dbg("Fix: %x %x %d %x",f->op,f->loc,f->type,f->value); */
}

WORD decodetag()
{
        WORD n;
        WORD value = getbyte();
        if( (value & 0x40) == 0 ) n = 0;
        else n = -1;

/* dbg("value = %2x",value); */
        while( (value & 0x80) != 0 )
        {
                n = (n<<7) | ( value & 0x7f);
                value = getbyte();
/* dbg("value = %2x",value); */
        }
        n = ( n << 7 ) | (value & 0x7f);
/* dbg("decodetag %d",n); */
        return n;
}

WORD decodestr(s)
STRING s;
{
        WORD size = decodetag();

        for( ;size > 0; size-- ) *s++ = getbyte();
        *s = '\0';
}

WORD getbyte()
{
        WORD c = getc(filefd);
        WORD d1;
        while( c == ' ' || c == '\n' ) c = getc(filefd);
        if( c == EOF ) return 0x7f;
        d1 = digit(c);
        return (d1<<4) | digit(getc(filefd));
}

WORD digit(c)
{
        c = tolower(c);
        if( '0' <= c && c <= '9' ) return c - '0';
        if( 'a' <= c && c <= 'f' ) return c - 'a' + 10;
        error("Format error in object file");
}

addfix(list,fix)
struct List *list;
struct fix *fix;
{
        struct Node *node = list->head;
        while( node->next != NULL && ((struct fix *)node)->loc <= fix->loc )
                node = node->next;
        PreInsert(node,fix);
}

addsym(list,sym)
struct List *list;
struct symb *sym;
{
        struct Node *node = list->head;
        while( node->next != NULL && ((struct symb *)node)->value <= sym->value )
                node = node->next;
        PreInsert(node,sym);
}

struct symb *nearsym(list,value)
struct List *list;
WORD value;
{
        struct Node *node = list->tail;
        while( node->prev != NULL )
        {
                if( ((struct symb *)node)->value <= value )
                        return (struct symb *)node;
                node = node->prev;
        }
        return NULL;
}

struct symb *findsym(list,value)
struct List *list;
WORD value;
{
        struct Node *node = list->tail;
        while( node->prev != NULL )
        {
                if( ((struct symb *)node)->value == value )
                        return (struct symb *)node;
                node = node->prev;
        }
        return NULL;
}

/*  -- End of disasm.c -- */
