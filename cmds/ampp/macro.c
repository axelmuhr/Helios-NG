/****************************************************************/
/* File: macro.c                                                */
/*                                                              */
/* machine independant entry to macro processor. Infd and outfd */
/* are already set to the input and output streams.             */
/*                                                              */
/* Author: NHG 19-Feb-87                                        */
/****************************************************************/
#ifdef __TRAN
static char *RcsId = "$Id: macro.c,v 1.4 1994/03/08 13:05:57 nickc Exp $ Copyright (C) Perihelion Software Ltd.";
#endif
  
#include "ampp.h"
#include <string.h>
  
#define trace if(traceflags&db_parse)_trace

PUBLIC jmp_buf reclev;

PRIVATE int parse_depth;

PRIVATE void eval( void );
PRIVATE void evalvar( void );
PRIVATE void collectarg( struct Arg * arg );

/********************************************************/
/* macro                                                */
/*                                                      */
/* processor entry point                                */
/*                                                      */
/********************************************************/

PUBLIC void macro()
{
	WORD nesting = 0;
        jmp_buf catchall;

        /* the default recovery level simply quits */
        if( setjmp(catchall) ) return;
        memcpy(reclev, catchall, sizeof(jmp_buf));

	parse_depth = 0;
	
	symb = 0;
	
        while( symb != s_eof ) nesting = parse((int)nesting);

	if( nesting != 0 ) warn("Mismatched brackets at EOF %d",nesting);
}


/********************************************************/
/* parse                                                */
/*                                                      */
/* Recursively called routine which copies a single item*/
/* from the input to the output doing any macro         */
/* replacements etc. as it goes. An item consists of a  */
/* single token, or a sequence of tokens enclosed in    */
/* brackets. If the item is preceeded by a quote it is  */
/* not examined for macros but is transferred directly	*/
/* to the output.					*/
/*                                                      */
/********************************************************/

PUBLIC WORD parse(INT old_nesting)
{
        int nesting = (int) old_nesting;
        int quoted;
	
        nextsym();

        if( symb==s_eof ) return nesting;

	parse_depth++;

	if( ( quoted = (symb==s_quote) ) != 0 ) nextsym();

/*	trace("Parse %squoted depth %d",quoted?"":"un",parse_depth); */
		
        for(;;)
        {
                switch ( (int)symb )
                {
                case s_builtin: if(!quoted) { builtin(); break; }

                case s_macro:   if(!quoted) { eval(); break; }

		case s_hexvar:
                case s_var:     if(!quoted) { evalvar(); break; }

                default:
                case s_token:  
				if( toksize == 1 ) wrch(token[0]);
				else wrstr(token);
				break;

                case s_lbra:
                        if( nesting++ > 0) wrch(c_lbra);
                        break;

                case s_rbra:
                        if( --nesting > 0 ) wrch(c_rbra);
                        break;

                case s_eof: return nesting;

                case s_quote:
                        if( quoted ) wrch(c_quote);
                        pbchar(c_quote);
                        nesting = (int) parse((WORD)nesting);
                        break;
                       
                case s_concat:
                        if( quoted ) wrch(c_concat);
                        break;
                }

                if ( nesting <= old_nesting ) break;

                nextsym();
        }

	parse_depth--;

	return nesting;
}


/********************************************************/
/* eval                                                 */
/*                                                      */
/* The current symbol has been identified as a macro,   */
/* push its definition back onto the input and continue.*/
/*                                                      */
/********************************************************/

PRIVATE void eval()
{
        struct Macro *macro = tokdef->Value.macro;
        INT i;
        struct Arg *arg = (struct Arg *)(macro->arglist.Head);

	trace("Eval %12s depth %d",token,parse_depth);
	
        for( i = 0 ; i < macro->nargs ; i++ )
        {
                collectarg(arg);
                arg = (struct Arg *)(arg->node.Next);
        }

        /* We now push the text of the macro backwards onto the putback
           stack. We surround it with brackets so one call to parse will
           read it all, we can then unwind the argument defs.
        */

        pbchar(c_rbra);				/* closing bracket */

        pbdef(&macro->def); 			/* put def back */

        pbchar(c_lbra);				/* opening bracket */

        parse(0L);				/* evaluate the macro */

        /* now clear the argument definitions */
        arg = (struct Arg *)(macro->arglist.Head);
        for( i = 0 ; i < macro->nargs ; i++ )
        {
                unwind(arg->sym);
                arg = (struct Arg *)(arg->node.Next);
        }

}

/********************************************************/
/* collectarg                                           */
/*                                                      */
/* Collect the definition of an argument and add a def  */
/* for it.                                              */
/* If the formal parameter was prefixed by a number of  */
/* quotes, these are placed in front of the actual	*/
/* parameter before it is read in.			*/
/*                                                      */
/********************************************************/

PRIVATE void collectarg(struct Arg *arg)
{
        struct Macro *macro = New(struct Macro);
	INT quoted = arg->quoted;
	
        pbskipspaces();

        while( quoted-- ) pbchar(c_quote);

        getdef(&macro->def);

        macro->nargs = 0;

        adddef((INT)s_macro,arg->sym,(INT)macro);
}

/********************************************************/
/* evalvar                                              */
/*                                                      */
/* evaluate a variable and replace it in the input with */
/* a representative digit string.                       */
/*                                                      */
/********************************************************/

PRIVATE void evalvar()
{
	if( symb == s_var ) pbnum(tokval);
	else pbhex(tokval);
}

