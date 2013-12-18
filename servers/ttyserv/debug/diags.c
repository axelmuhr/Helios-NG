/*************************************************************************
**									**
**		     S Y S T E M   D E B U G G I N G			**
**		     -------------------------------			**
**									**
**		    Copyright (C) 1990, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** diags.c								**
**									**
**	- Diag message exchange						**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	20/04/90 : C. Fleischer					**
*************************************************************************/

#ifndef __diags_h
#define __diags_h

#ifndef __helios_h
#include <helios.h>
#endif
#ifndef __debug_h
#include "../debug/debug.h"
#endif

word
XchgDiags ( MCB *t, MCB *r )
{
    word	e;			/* Error code			*/
    
    forever
    {
    	e = PutMsg ( t );		/* send Tx message		*/

    	if ( e < Err_Null )		/* Tx errors are serious.	*/
    	    goto done;
    	    
    	e = GetMsg ( r );		/* get Rx message		*/
    	
    					/* return on success or Error	*/
    	if ( ( e >= Err_Null ) || ( ( e & EC_Mask ) >= EC_Error ) )
    	    goto done;
    	    				/*Increment retry count		*/
    	t->MsgHdr.FnRc = ( t->MsgHdr.FnRc & ~FR_Mask ) |
    	    ( ( t->MsgHdr.FnRc + FR_Inc ) & FR_Mask );
					
    }					/* retry until success or Error	*/
done:
    return e;
}

word		
SetDiags ( char *name, word diags )
{
    word	control [IOCMsgMax];	/* Common control vector	*/
    byte	data [IOCDataMax];	/* Common data vector		*/
    Port	Reply	= NewPort ();	/* Our reply port		*/
    MCB		t;			/* Tx mcb			*/
    MCB		r;			/* Rx mcb			*/
    
   					/* Initialise Tx mcb		*/
    InitMCB ( &t, MsgHdr_Flags_preserve, MyTask->IOCPort, Reply, 
    	FC_GSP + FG_SetDiags );
    t.Timeout = OneSec;			/* Tx should be fast...		*/
    t.Control = control;
    t.Data = data;
    MarshalCommon ( &t, NULL, name );	/* Marshal server name		*/
    MarshalWord ( &t, diags );		/* and new diags value.		*/
    
   					/* Initialise Rx mcb		*/
    InitMCB ( &r, 0, Reply, NullPort, 0 );
    r.Timeout = 20 * OneSec;		/* Rx has IOCTimeout		*/
    r.Control = control;
    r.Data = data;
    
    return XchgDiags ( &t, &r );	/* Exchange messages.		*/
}

word		
GetDiags ( char *name, word *diags )
{
    word	control [IOCMsgMax];	/* Common control vector	*/
    byte	data [IOCDataMax];	/* Common data vector		*/
    Port	Reply	= NewPort ();	/* Our reply port		*/
    MCB		t;			/* Tx mcb			*/
    MCB		r;			/* Rx mcb			*/
    word	e;			/* Error code			*/
    
   					/* Initialise Tx mcb		*/
    InitMCB ( &t, MsgHdr_Flags_preserve, MyTask->IOCPort, Reply, 
    	FC_GSP + FG_GetDiags );
    t.Timeout = OneSec;			/* Tx should be fast...		*/
    t.Control = control;
    t.Data = data;
    MarshalCommon ( &t, NULL, name );	/* Marshal server name.		*/
    
   					/* Initialise Rx mcb		*/
    InitMCB ( &r, 0, Reply, NullPort, 0 );
    r.Timeout = 20 * OneSec;		/* Rx has IOCTimeout		*/
    r.Control = control;
    r.Data = data;
    
    e = XchgDiags ( &t, &r );		/* Exchange messages.		*/

    if ( e >= Err_Null && r.MsgHdr.FnRc == t.MsgHdr.FnRc )
    	*diags = r.Control[0];

    return e;
}

#endif

/*--- end of diags.c ---*/

