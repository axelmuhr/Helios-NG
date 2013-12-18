/************************************************************************/
/*                                                                      */
/* File: disasm.c                                                       */
/*                                                                      */
/* Changes:                                                             */
/*      NHG  22-May-87  : Created                                       */
/*                                                                      */
/* Description:                                                         */
/*      Object file disassembler                                        */
/*      Uses loadfile.c and decode.c which are common with the debugger */
/*      and queue.c which is common with everything                     */
/*                                                                      */
/*                                                                      */
/* Copyright (c) 1987, Perihelion Software Ltd. All Rights Reserved.    */
/************************************************************************/

#include <stdio.h>
#include <types.h>
#include <queue.h>
#include <loadfile.h>
#include <decode.h>

#define New(_type) ((_type *)malloc(sizeof(_type)))

FILE *infd, *outfd;

extern FILE *filefd;

UBYTE *codevec;
WORD modbase = 0;
WORD curpos;
WORD codelim;

struct fix *curfix;
struct symb *curcode;

extern WORD function;
extern WORD operand;

/****************************************************************/
/* Procedure: main                                              */
/*                                                              */
/****************************************************************/

main(argc,argv)
WORD argc;
STRING *argv;
{
        infd = stdin;
        outfd = stdout;

        if( argc > 2 )
        {
                outfd = fopen(argv[2],"w");
                if( outfd <= 0 ) error("Cannot open '%s' for input",argv[2]);
        }

        /* sledgehammer tactics */
        codevec = malloc(30000);

        initload();

        curpos = 0;

        if(codevec == 0) error("Cannot get code vector");

        loadfile(argv[1]);

        codelim = curpos;
        curpos = modbase;

        putsymbs("codesymb",&codes,1);
        putsymbs("datasymb",&datas,1);
        putsymbs("ref",&refs,0);

        putc('\n',outfd);

        /* these avoid having to check for end of list in dodisasm */
        curfix = New(struct fix);
        curfix->loc = codelim;
        AddTail(&fixes,curfix);

        curcode = New(struct symb);
        curcode->value = codelim+1;
        AddTail(&codes,curcode);

        curcode = (struct symb *)(codes.head);
        curfix = (struct fix *)(fixes.head);

        while( curpos < codelim ) dodisasm();

        tidyup(0);

}

void pbyte(x)
UBYTE x;
{
        codevec[curpos++] = x;
}

UBYTE gbyte()
{
        return codevec[curpos++];
}

void ungbyte()
{
        curpos--;
}

putsymbs(symtype,list,val)
STRING symtype;
struct List *list;
WORD val;
{
        struct Node *node = list->head;

        while( node->next != NULL )
        {
                STRING fmt = val ? "%s %s %x\n" : "%s %s\n";
                struct symb *s = (struct symb *)node;
                fprintf(outfd,"                              ");
                fprintf(outfd,fmt,symtype,s->name,s->value);
                node = node->next;
        }
}

error(str,a,b,c,d,e,f)
{
        printf(str,a,b,c,d,e,f);
        putchar('\n');
/* dbg(str,a,b,c,d,e,f); */
        tidyup(20);
}

dbg(str,a,b,c,d,e,f)
{
        fprintf(outfd,str,a,b,c,d,e,f);
        putc('\n',outfd);
}

tidyup(rc)
{
        if( infd != stdin && infd != 0) fclose(infd);
        if( outfd != stdout && outfd != 0) fclose(outfd);
        if( filefd != 0 && filefd != 0) fclose(filefd);

        if( codevec != 0 ) free(codevec);
        exit(rc);
}


/*  -- End of disasm.c -- */
