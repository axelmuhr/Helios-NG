/*************************************************************************
**									**
**	       C O N S O L E  &  W I N D O W   S E R V E R		**
**	       -------------------------------------------		**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** setterm.c								**
**									**
**	- Utility to set terminal emulation				**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	17/01/90 : C. Fleischer					**
*************************************************************************/

#define __in_globals	1		/* flag that we are in this module */

#include "window.h"

/*************************************************************************
 * MAIN BODY
 *
 * - Scan the command line for a terminal emulation name.
 * - Send a private message to the window server specified
 *   in the standard environment (stdin Server).
 *
 * Parameter  :	termcap entry name
 *		Example : setterm vt100
 * Return     :	Error code
 *
 ************************************************************************/

int 
main ( int argc, char *argv[] )
{
    char	*tname;
    char	wmname[100];
    MCB		m;
    word	e;
    word	Control_V[IOCMsgMax];
    byte	Data_V[IOCDataMax];
    Port	reply;
  
    					/* Check args for plausibility	*/
    if ( argc == 1 )
    	tname = "";	
    else
    {
    	tname = argv[1];
    	if ( argc > 2 )
 	fprintf (stderr, "%s : Further arguments are ignored !\n",
 		 argv[0] );
    }
    
    strncpy ( wmname, Heliosno ( stdin )->Name, 99 );
    wmname[99] = '\0';

    * ( strrchr ( wmname, c_dirchar ) ) = '\0';
/*    printf ( "%s: window server is \"%s\".\n", argv[0], wmname );	*/
        
 /*-----------------  Prepare MCB for marshalling  ---------------------*/
 

    reply = NewPort ();			/* Basic initialisation of the	*/
					/* MesssageControlBlock		*/
    InitMCB ( &m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply,
 	   FC_GSP + FG_Reinit );
 	   				/* Preparing control and data	*/
    m.Control = Control_V;		/* vector			*/
    m.Data    = Data_V; 	   
    MarshalCommon ( &m, Null ( Object ), wmname );          
    MarshalString ( &m, tname );

/*    printf ( "%s sending request.\n", argv[0] );			*/
/*    fflush ( stdout );						*/
    
    e = PutMsg ( &m );			/* Send message to the server	*/
    if ( e != Err_Null )
    {
 	fprintf (stderr, "%s : Can't send message to server %s :%x\n",
 		 argv[0], wmname, e);
 	return 1;
    }
 					/* Wait for reply		*/
 					/* from the window server...	*/
    InitMCB ( &m, MsgHdr_Flags_preserve, reply, NullPort, 0 );
    m.Timeout = MaxInt;

/*    printf ( "%s waiting for reply.\n", argv[0] );			*/
/*    fflush ( stdout );						*/
    
    e = GetMsg ( &m );
    FreePort ( reply );
 
    if ( m.MsgHdr.FnRc == FC_GSP + SS_Window + FG_Reinit )
    {
    	printf ("%s: terminal is \"%s\".\n", argv[0], &m.Data[m.Control[0]]);
	return 0;
    }
    else
    {
 	fprintf (stderr,"%s: Failed to set terminal type : got %08x, expected %08x\n",
 		 argv[0], m.MsgHdr.FnRc, FC_GSP+ SS_Window + FG_Reinit ); 
	return 1;
    }
}

/*--- end of setterm.c ---*/
