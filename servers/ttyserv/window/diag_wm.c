/*************************************************************************
**									**
**	       C O N S O L E  &  W I N D O W   S E R V E R		**
**	       -------------------------------------------		**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** diag_wm.c								**
**									**
**	- Utility to change window debug modes				**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	20/04/90 : C. Fleischer					**
*************************************************************************/

#define __in_diag_wm	1		/* flag that we are in this module */

#include "window.h"
#include "../debug/diags.h"
#include <ctype.h>

static char	*areas[] = { "tcap", "input", "term", "ansi", 
			     "read", "write", "open", "create", 
			     "delete", "attr", "setterm", "cbreak", 
			     "main", "" };

static char	*state[] = { "off", "on ", "???", "all" };

/*************************************************************************
 * EXPLAIN THE USAGE OF THIS PROGRAM
 *
 * - Show all valid arguments
 *
 ************************************************************************/

void
Usage ( void )
{
    fputs ( "\
diag_wm : select window manager diagnostics\n\
Usage	: diag_wm [[-]all] [[+|-]area1] [[+|-]area2]...\n\
Diagnostic areas:\n\
tcap	  termcap functions\n\
input	  handler for asynchroneus input\n\
term	  physical terminal output\n\
ansi	  ansi emulator and virtual screen output\n\
read	  handle GSP read\n\
write	  handle GSP write\n\
open	  open a window\n\
create	  create a window\n\
delete	  delete a window\n\
attr	  change window attributes\n\
setterm   change terminal emulation\n\
cbreak	  handle Ctrl-C event\n\
main	  general server operation\n\
all	  activate/deactivate all areas\n\
show	  show currently activated areas\n\
The '+' modifier activates more output, a '-' modifier disables an area.\n\
The parameters are scanned in the same order as given in the command line.\n\
", stderr );
}


/*************************************************************************
 * SHOW CURRENT DIAG SETTINGS
 *
 * Parameter  :	diags	= current diag mode
 *
 ************************************************************************/

void
ShowAreas ( word diags )
{
    int		i	= 0;
    
    printf ( "\nCurrent diagnostic area settings : 0x%08x\n", diags );
    while ( *areas [i] )
    {
   	printf ( "%-8s  %-3s   ", areas [i], state [diags & 3] );
    	diags >>= 2;
    	i++;
    }
    putchar ( '\n' );
}

bool
strucmp ( char *s1, char *s2 )
{
    while ( *s1 && *s2 )
    	if ( toupper ( *s1++ ) != toupper ( *s2++ ) )
    	    return FALSE;
    return *s1 == *s2;
}

/*************************************************************************
 * MAIN BODY
 *
 ************************************************************************/

int
main ( int argc, char *argv[] )
{
    char	wmname[100];
    word	diags	= 0;
    char	*arg;
    word	argn;
    word	weight;
    word	mask;
    word	error, i;
					/* Check args for plausibility	*/
    if ( argc == 1 )
    	Usage ();
    else
    {
    	strncpy ( wmname, Heliosno ( stdin )->Name, 99 );
    	wmname[99] = '\0';

    	* ( strrchr ( wmname, c_dirchar ) ) = '\0';
    	
     	error = GetDiags ( wmname, &diags );
     	if ( error < Err_Null )
     	{
     	    fprintf ( stderr, "diag_wm : cannot get diags for %s - %x\n",
     	    	wmname, error );
     	    exit ( 1 );
     	}
     	for ( argn = 1; argn < argc; argn++ )
     	{
     	    arg = argv [argn];

   	    if ( strucmp ( arg, "show" ) )
   	    {
     	    	ShowAreas ( diags );
     	    	continue;
     	    }

     	    if ( *arg == '-' )
     	    {
     	    	arg++;
     	    	weight = 0;
     	    }
     	    elif ( *arg == '+' )
     	    {
     	    	arg++;
     	    	weight = 3;
     	    }
     	    else
     	    	weight = 1; 

     	    if ( strucmp ( arg, "all" ) )
     	    {
     	    	if ( weight == 3 )
     	    	    diags = 0xffffffff;
     	    	elif ( weight == 1 )
     	    	    diags = 0x55555555;
     	    	else	
     	    	    diags = 0;
     	    	continue;
     	    }
     	    mask = 3;
     	    for ( i = 0; *areas [i]; i++ )
     	    {
     	    	if ( strucmp ( areas [i], arg ) )
     	    	{
     	    	    diags = ( diags & ~mask) | weight;
     	    	    break;
     	    	}
     	    	else
     	    	{
     	    	    mask <<= 2;
     	    	    weight <<= 2;
     	    	}
     	    }
     	}
     	unless ( strucmp ( arg, "show" ) )
     	    ShowAreas ( diags );
     	

     	error = SetDiags ( wmname, diags );	
     	if ( error < Err_Null )
     	{
     	    fprintf ( stderr, "diag_wm : cannot set diags for %s - %x\n",
     	    	wmname, error );
     	    exit ( 1 );
     	}
    }
}

/*--- end of diag_wm.c ---*/
