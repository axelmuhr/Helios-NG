/*************************************************************************
**									**
**	       T E R M I N A L   W I N D O W   S E R V E R		**
**	       -------------------------------------------		**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**		    Copyright (c) 1991, Perihelion Software Ltd.	**
**			  All Rights Reserved.				**
**									**
**		    Copyright (c) 1993, Perihelion Software Ltd.	**
**			  All Rights Reserved.				**
**									**
**									**
** tty.c								**
**									**
**	- Terminal Window Server					**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	20/11/89 : C. Fleischer					**
** Changed   :	18/01/90 : C. Fleischer	 terminal reinitialisation	**
** Changed   :	12/09/90 : G. Jodlauk	 use of device interface	**
** Changed   :	17/09/90 : G. Jodlauk	 some general changes		**
** Changed   :	28/01/91 : N. Clifton	 fixed bugs shown up by xterm	**
** Changed   :	28/02/91 : N. Garnett	 added select			**
** Changed   :  23/03/93 : N. Clifton    fixed ctrl-C handling          **
*************************************************************************/

#define __in_tty	1		/* flag that we are in this module */
#define	DEBUG_MAIN

#include    "tty.h"

extern word	device_init_info_req( void );

/*---------------------------  Local data  -----------------------------*/

int		terminating;
Semaphore	Forever;
Semaphore	DispTerm;

RootStruct	*SysRoot;
Object		*root;
Object		*nte;			/* my Name table entry		*/

char		*def_name = "default";	/* default termcap entry name	*/
char		*def_dev = "tserial.d";	/* use serial driver as default	*/
char		*def_nte = "window";	/* default name table entry	*/

char		McName[64];		/* processor name		*/
DirNode		WindowRoot;		/* Window's root node		*/

NameInfo	WindowName =		/* Info structure to create	*/
{					/* the name table entry		*/
	NullPort,
	Flags_StripName,
	DefDirMatrix,
	(word *) NULL
};

DispatchInfo	WindowInfo =		/* Info for servlib.Dispatch ()	*/
{
    &WindowRoot,			/* root of name system		*/
    NullPort,				/* request port, inserted later	*/
    SS_Window,				/* Subsystem code: IO Process	*/
    McName,				/* name of parent directory	*/
    {	    Do_Private,     4000 },	/* Private request handler	*/
    {					/* GSP function handlers	*/
	{   Do_Open,	    2000 },	/* Open     : !	Special Handler	*/
	{   Do_Create,	    2000 },	/* Create   : !	Special Handler	*/
	{   DoLocate,	    2000 },	/* Locate   :	Default Handler	*/
	{   DoObjInfo,	    2000 },	/* ObjInfo  :	Default Handler	*/
	{   NullFn,	    2000 },	/* ServInfo :	not supported	*/
	{   Do_Delete,	    2000 },	/* Delete   : ! Special Handler	*/
	{   DoRename,	    2000 },	/* Rename   :	Default Handler	*/
	{   DoLink,	    2000 },	/* Link     :	Default Handler	*/
	{   DoProtect,	    2000 },	/* Protect  :	Default Handler	*/
	{   DoSetDate,	    2000 },	/* SetDate  :	Default Handler	*/
	{   DoRefine,	    2000 },	/* Refine   :	Default Handler	*/
	{   NullFn,	    2000 }	/* CloseObj :	not supported	*/
    }
};

/*************************************************************************
 * INSTALL A WINDOW AS THE CURRENT ONE
 *
 * - Update the Cur_Window, Cur_Screen and Cur_Keyboard pointers.
 * - Redraw the Terminal screen.
 *
 * ! This procedure must be entered with Window_Lock locked.
 *
 * Parameter  :	- nothing -
 * Return     :	- nothing -
 *
 ************************************************************************/

void
SetCurrentWindow (Window *w)
{
    Cur_Window = NULL;
    Cur_Screen = NULL;
    Cur_Keyboard = NULL;

    RedrawScreen (w);

    Cur_Window = w;
    Cur_Screen = &w->Screen;
    Cur_Keyboard = &w->Keyboard;
}

/*************************************************************************
 * INITIALISE AN ATTRIBUTE STRUCTURE
 *
 * - Set all Attribures to NULL.
 *
 * Parameter :	attr	= pointer to Attributes
 * Return    :	- nothing -
 *
 ************************************************************************/

void
InitAttributes (Attributes *attr)
{
    int i;
    word *wpointer = (word *) attr;
    for (i=0; i < 5; i++)
	*wpointer++ = 0L;
}


/*************************************************************************
 * INITIALISE A TERMINAL STRUCTURE
 *
 * Parameter  :	t	= pointer to terminal structure
 * Return     :	- nothing -
 *
 ************************************************************************/

static void
InitInput (Keyboard *k)
{
    InitSemaphore (&k->raw_lock, 1);
    k->in_head = 0;
    k->in_tail = 0;
    k->in_count = 0;
    k->in_flags = 0;
}


/*************************************************************************
 * CREATE AND INITIALISE A WINDOW
 *
 * - Allocate a window structure.
 * - Initialise the screen substructure.
 * - Initialise the terminal substructure.
 *
 * Parameter  :	name	= new window name
 * Return     :	pointer to new window structure
 *		NULL if an error occured
 *
 ************************************************************************/

static Window *
NewWindow (DirNode *dir, char *name)
{
#ifdef	DEBUG
    static char *fname = "NewWindow";
#endif
    Window	*w = (Window *) Malloc (sizeof (Window));

#ifdef	DEBUG
    Debug (CREATE) ("%s (%s, %s) started.", fname, dir->Name, name);
#endif

    if (w == NULL)			/* check the window pointer	*/
    {
#ifdef	DEBUG
	Debug (ERROR) ("%s failed to get Window memory !");
#endif
	return NULL;
    }
						/* initialise ObjNode	*/
    InitNode (&w->ObjNode, name, Type_File,
	Flags_Closeable | Flags_Interactive, DefFileMatrix);

    unless (AnsiInitScreen (&w->Screen))	/* initialise screen	*/
    {
#ifdef	DEBUG
	Debug (ERROR) ("%s : failed on Ansi_Init_Screen !", fname);
#endif
	Free (w);
	return NULL;
    }

    InitInput (&w->Keyboard);			/* initialise terminal	*/

    InitAttributes  (&w->Attribs);		/* initialise other variables	*/
    AddAttribute    (&w->Attribs, ConsoleEcho);
    RemoveAttribute (&w->Attribs, ConsoleIgnoreBreak);
    AddAttribute    (&w->Attribs, ConsoleBreakInterrupt);
    AddAttribute    (&w->Attribs, ConsolePause);
    RemoveAttribute (&w->Attribs, ConsoleRawInput);
    RemoveAttribute (&w->Attribs, ConsoleRawOutput);

    w->Attribs.Min  = w->Screen.Rows;
    w->Attribs.Time = w->Screen.Cols;
    w->raw_in       = FALSE;
    w->raw_out      = FALSE;
    w->Xoff         = FALSE;
    w->EventPort    = NullPort;
    w->EventCount   = 1;
    w->SelectMode   = 0;
    w->SelectPort   = NullPort;
/*
-- crf: 12/11/91 - Bug 811 (2)
-- set to TRUE by Do_Delete()
*/
    w->deleted      = FALSE;

    InitSemaphore (&w->ReadLock, 1);
    InitSemaphore (&w->WriteLock, 1);
    InitSemaphore (&w->SelectLock, 1);    

    Wait (&Window_Lock);		/* lock window globals		*/

    if (Window_Count == 0)		/* first window ?		*/
    {
	w->Next = w;			/* yes, let the chain pointers	*/
	w->Prev = w;			/* point to the new window.	*/

	Signal (&Input_Lock);		/* Start the Input Handler.	*/
    }
    else
    {
	w->Next = Cur_Window->Next;	/* no, put the new window into	*/
	w->Prev = Cur_Window;		/* the window chain behind the	*/
	Cur_Window->Next = w;		/* current window.		*/
	w->Next->Prev    = w;
    }

    Window_Count++;			/* count new window		*/

    SetCurrentWindow (w);		/* show new window		*/

    Signal (&Window_Lock);		/* release window globals	*/

#ifdef	DEBUG
    Debug (CREATE) ("%s (%s, %s) ready.", fname, dir->Name, name);
#endif

    return w;
#ifndef	DEBUG
    dir = dir;				/* keep the compiler happy...	*/
#endif
}

/*************************************************************************
 * DELETE A WINDOW STRUCTURE
 *
 * - Remove the window from its directory.
 * - Free the screen substructure.
 * - Free the window structure.
 *
 * Parameter  :	w	= window to be deleted
 * Return     :	- nothing -
 *
 ************************************************************************/

static void
FreeWindow (Window *w)
{
#ifdef	DEBUG
    static char *fname = "FreeWindow";

    Debug (DELETE) ("%s (%s) started.", fname, w->ObjNode.Name);
#endif

    Wait (&Window_Lock);		/* lock window globals		*/

    w->Next->Prev = w->Prev;		/* remove the window from	*/
    w->Prev->Next = w->Next;		/* the window chain.		*/

    Window_Count--;

    if (Window_Count > 0 && w == Cur_Window)	/* if current window is	*/
	SetCurrentWindow (w->Prev);	/* removed, switch to prev one.	*/

    Signal (&Window_Lock);		/* release window globals	*/

    Wait (&w->ReadLock);		/* wait for forked requests	*/
    Wait (&w->WriteLock);		/* to finish.			*/

    AnsiTidyScreen (&w->Screen);	/* now free all used memory.	*/
    Free (w);

#ifdef	DEBUG
    Debug (DELETE) ("%s (%s) ready.", fname, w->ObjNode.Name);
#endif
}


/*************************************************************************
 * TIDY UP FOR TERMINATION AND EXIT
 * 
 * This function never returns to the caller.
 *
 * Parameter  :	- nothing -
 * Return     : no return value
 *
 ************************************************************************/

void EOFRead(Window *w)
{
    Keyboard	*k	= &w->Keyboard;	/* terminal structure		*/
    Semaphore	*lock	= &k->raw_lock;	/* raw buffer locking semaphore	*/
    int		*head	= &k->in_head;	/* circular head pointer	*/
    int		*tail	= &k->in_tail;	/* circular tail pointer	*/
	
    Wait(lock);
    
    k->in_raw[*head] = 0x04;	/* ^D */
    *head = (*head + 1) & InBufMask;
    if( *head == *tail )
    	*head = (*head - 1) & InBufMask;
    	
    Signal(lock);
}

void
Tidyup( int arg )
{
#ifdef	DEBUG
    static char *fname	= "Tidyup";
#endif
    char 	*name = NULL;
    static int	done = FALSE;
    int 	e;

#ifdef	DEBUG
    Debug (MAIN) ("%s started (%d).", fname, done);
#endif

    signal(SIGTERM, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    if (done) {
/*	IOdebug("%s: Tidyup already", name); */
    	Delay(OneSec*100);
    }

    /* start by EOF ing all pending reads */
    WalkList(&WindowRoot.Entries,(WordFnPtr)EOFRead);
    Delay(OneSec/10);
    
    done = TRUE;
    terminating = TRUE;
    name = (char *)Malloc((word)strlen(nte->Name)+1);
    if (name)
        strcpy(name, nte->Name);
    
/*    IOdebug("%s: Deleting NTE", name); */
    while ((e = (int)Delete (nte, NULL)) < 0)
        IOdebug("%s: Cannot delete NTE %x", name, e);
    Wait (&Input_Lock);			/* Stop the Input Handler,	*/
    Wait (&Window_Lock);		/* lock the Globals.		*/
/*    IOdebug("%s: Freeing my request port", name); */
    FreePort (WindowInfo.ReqPort);
/*    IOdebug("%s: Waiting for dispatcher termination", name); */
    Wait(&DispTerm);
/*    IOdebug ("%s: Dispatcher terminated", name); */
    
    if (termdcb)
        CloseDevice(termdcb);

    Delay(OneSec/2);

#ifdef	DEBUG
    Debug (MAIN) ("%s ready (%d).", fname, done);
#endif

    exit (0);

    arg = arg;
}

/*************************************************************************
 * HANDLE PRIVATE MESSAGES
 *
 * Parameter  :	srvinfo	= Dispatcher parameter block
 * Return     :	- nothing -
 *
 ************************************************************************/


void
Do_Private (ServInfo *srvinfo)
{
#ifdef	DEBUG
    static char *fname	= "Do_Private";
#endif
    MCB 	*m	= srvinfo->m;
    word	fnrc	= srvinfo->FnCode;
   
    if (terminating == TRUE)
	Wait(&Forever);
    
#ifdef	DEBUG
    if ( fnrc == FG_SetDiags || fnrc == FG_GetDiags )
    	Do_Diags ( srvinfo );
    else
#endif
    	
    if (fnrc == FG_Terminate)		/* Terminate the server :	*/
    {					/* Delete the name table entry,	*/
#ifdef	DEBUG
	Debug (MAIN) ("%s terminating.", fname);
#endif
	if (m->MsgHdr.Reply != NullPort)	/* Someone is waiting	*/
	{				/* for a termination reply :	*/
		InitMCB (m, 0, m->MsgHdr.Reply, NullPort,
	        FC_GSP + SS_Window + FG_Terminate);
	        PutMsg (m);		/* send  a reply message.	*/
	}
	if (terminating == FALSE) 
		raise(SIGTERM);
	else
		Wait(&Forever);
    }
    elif (fnrc == FG_Reinit)		/* Use another termcap entry:	*/
    {
    	struct ReinitReq
    	{
    	    IOCCommon	Common;
    	    Offset	Name;
        }	*req	= (struct ReinitReq *) m->Control;
    	char	*tname	= (char *) &m->Data[req->Name];
	int	noname 	= req->Name == -1; 
	int result = 0; 

#ifdef	DEBUG
	Debug (SETTERM) ("%s : new terminal \"%s\".", my_nte, tname);
#endif
    	Wait (&Input_Lock);		/* Stop the Input Handler,	*/

    	Wait (&Window_Lock);		/* lock the Globals.		*/

    	TermFlush ();			/* Flush the terminal buffer.	*/
					/* new terminal	specified :	*/
/*	if (*tname && strcmp (tname, term_name)) */
/* 	Imagine, you've just changed your termcap entry ...		*/
/*	If name is "", then MarshalString did MarshalWord -1.		*/ 
	unless (noname)
	{
#ifdef	DEBUG
	    Debug (SETTERM) ("%s reinitialises to \"%s\".", fname, tname);
#endif

	    if (AnsiReinit (tname))	/* try to reinitialise tables.	*/
	    {				/* succeded: save		*/

#ifdef	DEBUG
	    	Debug (SETTERM) ("%s \"%s\" reinitialised ok.", fname, tname);
#endif
	    	strncpy (term_name, tname, 31);	/* new terminal name.	*/
	    	term_name[31] = '\0';
	    	Input_Reset = TRUE;
            }
            else			/* failed: use old terminal.	*/
            {
#ifdef	DEBUG
	    	Debug (SETTERM) ("%s \"%s\" failed, resets to \"%s\".", 
	    	    fname, tname, term_name);
#endif
		result = 1;
        	AnsiReinit (term_name);
            }
            SetCurrentWindow (Cur_Window);	/* Reinitialise window.	*/
	}

	if (m->MsgHdr.Reply != NullPort)	/* Someone waiting for	*/
	{				/* a reinitialisation reply :	*/
	    InitMCB (m, 0, m->MsgHdr.Reply, NullPort,	/* send the	*/
	    	FC_GSP + SS_Window + FG_Reinit + (word)result);/* reply.	*/
	    MarshalString (m, term_name);
	    PutMsg (m);
	}

	Signal (&Window_Lock);		/* Release the Globals		*/

	Signal (&Input_Lock);		/* and start the Input Handler.	*/
    }
    else				/* Other private calls are not	*/
	ErrorMsg (m, EC_Error + EG_FnCode + EO_Server);	/* supported.	*/
}

void
DoReinit( int arg )
{
	/* IOdebug ("%s: SIGHUP -> reinitialisation '%s'.",my_nte, term_name); */
	
    	Wait (&Input_Lock);		/* Stop the Input Handler,	*/
    	Wait (&Window_Lock);		/* lock the Globals.		*/
       	TermFlush ();			/* Flush the terminal buffer.	*/
     	AnsiReinit (term_name);		/* Reinitialise termcaps	*/
        SetCurrentWindow (Cur_Window);	/* Reinitialise window.		*/
	Signal (&Window_Lock);		/* Release the Globals		*/
	Signal (&Input_Lock);		/* and start the Input Handler.	*/

	return;

	arg = arg;
}


/*************************************************************************
 * HANDLE THE OPEN REQUEST AND ALL SUBSEQUENT STREAM REQUESTS
 *
 * Parameter  :	srvinfo	= Dispatcher parameter block
 * Return     :	- nothing -
 *
 ************************************************************************/

void
Do_Open (ServInfo *srvinfo)
{
#ifdef	DEBUG
    static char *fname	= "Do_Open";
#endif
    word	sflags	= 0;
    int		EventID	= 0;
    MCB 	*m	= srvinfo->m;
    MsgBuf	*r;
    DirNode	*d;
    ObjNode	*o;
    Window	*w	= NULL;
    IOCMsg2	*req	= (IOCMsg2 *) (m->Control);
    Port	reqport;
    byte	*data	= m->Data;
    char	*pathname = srvinfo->Pathname;
    word	e;

#ifdef	DEBUG
    Debug (OPEN) ("%s (%s) started.", fname, pathname);
#endif

    if (terminating == TRUE)
	Wait( &Forever );

    /* As I do not know yet whether I am called on the /window directory	*/
    /* or on one of the windows, I have to call GetTargetDir and		*/
    /* GetTargetObj separately.							*/

    d = (DirNode *) GetTargetDir (srvinfo);

    if (d == NULL)
    {
#ifdef	DEBUG
	Debug (ERROR) ("%s : failed on GetTargetDir !", fname);
#endif
	ErrorMsg (m, Err_Null);
	return;
    }

    reqport = NewPort ();		/* Get a new port for requests.	*/
    
    if (reqport == NullPort)
    {
#ifdef	DEBUG
	Debug (ERROR) ("%s : failed to get reqport.", fname);
#endif
	ErrorMsg (m, EC_Warn + EG_NoMemory + EO_Server);
	
	return;
    }

    r = New( MsgBuf );			/* Get a Buffer for the reply.	*/
    
    if (r == NULL)
    {
#ifdef	DEBUG
	Debug (ERROR) ("%s : failed to get reply buffer.", fname);
#endif
	ErrorMsg (m, EC_Warn + EG_NoMemory + EO_Server);
	FreePort (reqport);
	return;
    }

    o = GetTargetObj( srvinfo );

    if (o == NULL && (req->Arg.Mode & O_Create))	/* window does	*/
    {							/* not exist, so create it	*/
							/* Check the access rights...	*/
	unless (CheckMask (req->Common.Access.Access, AccMask_W))
	{
	    ErrorMsg (m, EC_Error + EG_Protected + EO_Directory);
	    FreePort (reqport);
	    Free (r);
	    return;
	}
					/* could we create the window ?	*/
	if ((w = NewWindow( d, objname( pathname ) )) == NULL)
	{
	    ErrorMsg (m, EC_Warn + EG_NoMemory + EO_Server);
	    FreePort (reqport);
	    Free (r);
	    return;
	}

	o = &(w->ObjNode);		/* XXX - added by NC */
	
	Insert (d, &w->ObjNode, TRUE);	/* insert in directory	*/
    }
    elif (o == NULL)			/* window does not exist.	*/
    {
#ifdef	DEBUG
	Debug (ERROR) ("%s : window does not exist.", fname);
#endif
	ErrorMsg( m, EC_Error + EG_Unknown + EO_Object );
	
	FreePort( reqport );
	
	Free( r );
	
	return;
    }

    /* Check the access rights...	*/
    
    unless (CheckMask (req->Common.Access.Access, (int)req->Arg.Mode & Flags_Mode))
    {
#ifdef	DEBUG
	Debug (ERROR) ("%s : invalid access rights.", fname);
#endif
	ErrorMsg (m, EC_Error + EG_Protected + EO_File);
	
	if (w)
	{
	    Unlink (&w->ObjNode, TRUE);
	    FreeWindow (w);
	}
	
	FreePort (reqport);
	Free (r);
	return;
    }

    unless (w)
      w = (Window *) o;			/* until here, w was a new	*/
					/* created window.		*/
					/* Now it is THE window.	*/

#ifdef OLDCODE
    FormOpenReply( r, m, (ObjNode *) w,	Flags_Server | Flags_Closeable | Flags_Selectable, pathname );
#else
/*
-- crf: 12/11/91 - added Flags_Interactive
*/
    FormOpenReply( r, m, (ObjNode *) w,	Flags_Server | Flags_Closeable | 
                   Flags_Selectable | Flags_Interactive, pathname );
#endif

    r->mcb.MsgHdr.Reply = reqport;

    PutMsg( &r->mcb );			/* send an open reply to	*/
    
    Free( r );				/* the client.			*/

#ifdef	DEBUG
    Debug (OPEN) ("%s : %s opened.", fname, pathname);
#endif

    if (o->Type == Type_Directory)	/* The object is /window.	*/
    {					/* Let this be done by the	*/
#ifdef	DEBUG
	Debug (OPEN) ("%s : calling DirServer.", fname);
#endif

	DirServer( srvinfo, m, reqport );	/* directory handler.	*/

#ifdef	DEBUG
	Debug (OPEN) ("%s : DirServer ready.", fname);
#endif
	FreePort( reqport );

	return;
    }

    o->Account++;			/* Increment the usage counter. */

    UnLockTarget( srvinfo );

    forever				/* Loop for the Stream GSP	*/
    {
	m->MsgHdr.Dest = reqport;
	m->Timeout     = StreamTimeout;
	m->Data        = data;

	e = GetMsg( m );		/* Get the next request.	*/
	
	if (e == EK_Timeout) break;	/* If Timeout, then terminate,	*/
	if (e < Err_Null) continue;	/* else ignore the message.	*/

	Wait( &w->ObjNode.Lock );	/* Lock the object and call the	*/
					/* known handling functions.	*/
	switch (m->MsgHdr.FnRc & FG_Mask)
	{
	case FG_Read:
	    WindowRead( m, w, &sflags );
	    break;

	case FG_Write:
	    WindowWrite( m, w );
	    break;

	case FG_Select:
	     WindowSelect( m, w );
	     break;

	case FG_SetInfo:
	    WindowSetInfo( m, w );
	    break;

	case FG_GetInfo:
	    WindowGetInfo( m, w );
	    break;

	case FG_EnableEvents:
	    WindowEnableEvents( m, w, &EventID );
	    break;

	case FG_Close:
#ifdef	DEBUG
	    Debug (OPEN) ("Close on /%s/%s.",my_nte, w->ObjNode.Name);
#endif
	    if (m->MsgHdr.Reply != NullPort)
	      ErrorMsg( m, Err_Null );
	    
	    FreePort( reqport );
	    
	    if (EventID == w->EventCount)	/* Event installed ?	*/
		w->EventPort = NullPort;	/* remove it		*/
	    
	    w->ObjNode.Account--;	/* Remove the usage mark	*/

/*
-- crf: 12/11/91 - Bug 811 (2)
-- If w->deleted has been set (by Do_Delete()) and the usage count is zero, get
-- rid of the window
*/	    
	    if ((w->deleted) && (!w->ObjNode.Account))
	    {
		Unlink (&w->ObjNode, TRUE);
		FreeWindow (w);
		return;
	    }

	    Signal( &w->ObjNode.Lock );	/* and release the object.	*/
	    
	    return;

	default:
	    ErrorMsg( m, EC_Error + EG_WrongFn + EO_File );
	    break;
	}

	Signal( &w->ObjNode.Lock );	/* Release the object for the	*/
					/* next request.		*/
    }

#ifdef	DEBUG
    Debug (ERROR) ("/%s/%s: Stream timed out !!!", my_nte, w->ObjNode.Name);
#endif

					/* The stream has timed out :	*/
    Wait (&w->ObjNode.Lock);		/* Lock the window object,	*/
    FreePort (reqport);			/* free the Stream Port,	*/
    if (EventID == w->EventCount)	/* did we install an Event ?	*/
	w->EventPort = NullPort;	/* remove it			*/
    w->ObjNode.Account--;		/* remove the usage mark	*/
    Signal (&w->ObjNode.Lock);		/* and release the object.	*/
}


/*************************************************************************
 * HANDLE THE CREATE REQUEST
 *
 * Parameter  :	srvinfo	= Dispatcher parameter block
 * Return     :	- nothing -
 *
 ************************************************************************/

void
Do_Create (ServInfo *srvinfo)
{
#ifdef	DEBUG
    static char	*fname	= "Do_Create";
#endif
    MCB 	*m	= srvinfo->m;
    MsgBuf	*r;
    DirNode	*d;
    ObjNode	*o;
    Window	*w	= NULL;
    IOCCreate	*req	= (IOCCreate *) (m->Control);
    char	*pathname = srvinfo->Pathname;

#ifdef	DEBUG
    Debug (CREATE) ("%s (%s) started.", fname, pathname);
#endif

    if (terminating == TRUE)
	Wait(&Forever);

    if ((d = GetTargetDir (srvinfo)) == NULL)	/* get context dir	*/
    {
	ErrorMsg (m, EO_Directory);
	return;
    }

    if ((o = GetTargetObj (srvinfo)) != NULL)
    {						/* name already exists,	*/
#ifdef	DEBUG
	Debug (CREATE) ("%s (%s) name exists.", fname, pathname);
#endif
	strcat (pathname, ".");			/* append WindowID	*/
	addint (pathname, WindowID++);

	if ((o = Lookup (d, objname (pathname), TRUE)) != NULL)
	{					/* name still exists !	*/
#ifdef	DEBUG
	    Debug (CREATE) ("%s (%s) name also exists.", fname, pathname);
#endif
	    ErrorMsg (m, EC_Error + EG_Create + EO_File);
	    return;
	}
    }
					/* have we write access to dir?	*/
    unless (CheckMask (req->Common.Access.Access, AccMask_W))
    {
	ErrorMsg (m, EC_Error + EG_Protected + EO_Directory);
	return;
    }

    if ((r = New (MsgBuf)) == NULL)	/* get a buffer for the reply	*/
    {
	ErrorMsg (m, EC_Warn + EG_NoMemory + EO_Server);
	return;
    }
					/* could we create the window ?	*/
    if ((w = NewWindow (d, objname (pathname))) == NULL)
    {
	ErrorMsg (m, EC_Warn + EG_NoMemory + EO_Server);
	Free (r);
	return;
    }
    Insert (d, &w->ObjNode, TRUE);	/* insert window into directory	*/


    FormOpenReply (r, m, (ObjNode *) w, 0, pathname);	/* send the	*/
    PutMsg (&r->mcb);			/* open reply to the client.	*/
    Free (r);
#ifdef	DEBUG
    Debug (CREATE) ("%s (%s) ready.", fname, pathname);
#endif
}


/*************************************************************************
 * HANDLE THE DELETE REQUEST
 *
 * Parameter  :	srvinfo	= Dispatcher parameter block
 * Return     :	- nothing -
 *
 ************************************************************************/

void
Do_Delete (ServInfo *srvinfo)
{
#ifdef	DEBUG
    static char	*fname	= "Do_Delete";
#endif
    MCB 	*m	= srvinfo->m;
    Window	*w;
    IOCCommon	*req	= (IOCCommon *) (m->Control);

#ifdef	DEBUG
    Debug (DELETE) ("%s (%s)", fname, srvinfo->Pathname);
#endif

    if (terminating == TRUE)
	Wait(&Forever);

    if ((w = (Window *) GetTarget (srvinfo)) == NULL)	/* get window	*/
    {
#ifdef	DEBUG
	Debug (DELETE) ("%s cannot find target.", fname);
#endif	
	ErrorMsg (m, EO_File);
	return;
    }
    unless (CheckMask (req->Access.Access,AccMask_D))	/* check access	*/
    {
	ErrorMsg (m, EC_Error + EG_Protected + EO_File);
	return;
    }

    if (w->ObjNode.Type == Type_Directory)	/* object is /window,	*/
    {						/* cannot be deleted !	*/
#ifdef	DEBUG
	Debug (DELETE) ("%s : cannot delete /window !", fname);
#endif	
	ErrorMsg (m, EC_Error + EG_Delete + EO_Directory);
	return;
    }
    elif (w->ObjNode.Type == Type_File)		/* object is a window	*/
    {
	if (w->ObjNode.Account > 0)		/* still open streams	*/
	{
#ifdef	DEBUG
	    Debug (DELETE) ("%s (%s) : %d open streams.", 
	    	fname, srvinfo->Pathname, w->ObjNode.Account);
#endif	
/*
-- crf: 12/11/91 - Bug 811 (2)
-- Need to record the fact that a Delete request has been received so I can
-- clobber the window later ...
*/
            w->deleted = TRUE ;

	    ErrorMsg (m, EC_Error + EG_InUse + EO_File);
	    return;
	}
	Unlink (&w->ObjNode, TRUE);
	FreeWindow (w);			/* delete it		*/
	ErrorMsg (m, Err_Null);
	return;
    }
    Unlink (&w->ObjNode, TRUE);		/* delete other objects	*/
    Free (w);
    ErrorMsg (m, Err_Null);
}


/*************************************************************************
 * READ A SINGLE CHARACTER FROM A WINDOW'S RAW BUFFER
 *
 * - Characters arrive in the buffer asynchroneously,
 *   they are put into by Input.HandleInput.
 *
 * Parameter  :	w	= window
 * Return     :	-1	if buffer is empty
 *		-2	if Ctrl-C Break has occured
 *		ch	the extracted character
 *
 ************************************************************************/

static int
ReadChar (Window *w)
{
#ifdef	DEBUG
    char	*fname	= "ReadChar";
#endif
    Keyboard	*k	= &w->Keyboard;	/* terminal structure		*/
    Semaphore	*lock	= &k->raw_lock;	/* raw buffer locking semaphore	*/
    int		*head	= &k->in_head;	/* circular head pointer	*/
    int		*tail	= &k->in_tail;	/* circular tail pointer	*/
    int		ch;			/* extracted char		*/

    Wait (lock);			/* lock the raw buffer		*/

    
    if (k->in_flags & Cooked_CtrlC)	/* check for CBreak event	*/
    {					/* event has happened :		*/
    	k->in_flags &= ~Cooked_CtrlC;	/* reset CBreak flag		*/
    	Signal (lock);			/* release the buffer,		*/
    	return -2;			/* return -2 (Ctrl-C Event)	*/
    }
#ifdef	DEBUG
    Debug (READ_ALL) ("%s before: head = %02x, tail = %02x ", fname, *head, *tail);
#endif

    if (*tail == *head)			/* check for buffer empty	*/
    {					/* buffer is empty :		*/
	Signal (lock);			/* release the buffer,		*/
	return -1;			/* return -1 (not available)	*/
    }
    ch = (int) k->in_raw[*tail];	/* get the character		*/

    *tail = (*tail + 1) & InBufMask;	/* increment tail ptr		*/

#ifdef	DEBUG
    Debug (READ_ALL) ("%s after: head = %02x, tail = %02x ", fname, *head, *tail);
#endif

    Signal (lock);			/* release the buffer		*/


#ifdef	DEBUG
    Debug (READ) ((ch >= ' ' && ch < 127) ? "%s '%c'" : "%s %02x", fname, ch);
#endif

    return ch;				/* that's all.			*/
}


/*************************************************************************
 * READ CHARACTERS INTO A WINDOW'S COOKED BUFFER
 *
 * - Read characters from the raw buffer, handle things like
 *   Delete, Backspace, Ctrl-D and Return.
 *
 * Parameter  :	w	= window
 *		timeout	= timelimit
 * Return     :	TRUE	if data from the buffer can be extracted or
 *			Ctrl-D was read
 *		FALSE	to not read cooked data from the buffer
 *
 ************************************************************************/

static bool
Put_Cooked (Window *w, int timeout, word *sflags)
{
    Keyboard	*k	= &w->Keyboard;
    Screen	*s	= &w->Screen;
    word	tstep	= OneSec / 200;
    word	echo	= w->echo;
    char	*cook	= k->in_cook;
    int		*cooked	= &k->in_count;
    int		ch;

    while (timeout > 0)
    {
	if ((ch = ReadChar (w)) < 0)	/* read raw character		*/
	{				/* no character available :	*/
#if 0
	  /*
	   * XXX - NC - 23/3/93 (ref bug no )
	   *
	   * There should be no need to translate ctrl-C into ctrl-D
	   * and if this bit of code is removed then telnetd and rlogind
	   * appear to work.  Hence ...
	   */
	  
	    if (ch == -2)		/* check for Ctrl-C		*/
	    	return TRUE;		/* return like Ctrl-D		*/
#endif
	    Delay (tstep);		/* sleep a little bit,		*/
	    if ((timeout -= (int)tstep) < 0)	/* then check for timeout :	*/
		break;			/* reached, so break		*/
	    continue;			/* else try reading once more	*/
	}
	switch (ch)			/* analyse the character :	*/
	{
	case 0x0a :			/* carriage return and linefeed	*/
	case 0x0d :			/* are treated equally		*/
	    if (*cooked >= CookSize)	/* buffer full :		*/
	    {				/* delete last character	*/
		if (echo)
		    AnsiWriteData (s, "\b \b", 3);	/* on screen	*/
		(*cooked)--;
	    }
	    cook[(*cooked)++] = '\n';	/* store carriage return,	*/
	    if (echo)
		AnsiWriteData (s, "\r\n", 2);		/* echo it	*/
	    k->in_flags |= Cooked_Done;	/* and set Done flag.		*/
	    return TRUE;

	case 0x04 :			/* Ctrl-D : end of file		*/
	    *sflags |= Cooked_EOF;	/* set EOF flag.		*/
	    return TRUE;

	case 0x08 :			/* backspace			*/
	    if (*cooked)		/* if possible,			*/
	    {
		(*cooked)--;		/* delete last character,	*/
		if (echo)
		    AnsiWriteData (s, "\b \b", 3);	/* on screen.	*/
	    }
	    break;
	case 0x7F :			/* delete : erase whole line	*/
	    if (echo)
		AnsiWriteData (s, "\r\x09bK", 3);
	    *cooked = 0;
	    break;
	default :			/* any other key :		*/
	    if (ch >= ' ')		/* put printable keys into the	*/
	    {				/* cook buffer, overwrite last	*/
		if (*cooked >= CookSize)	/* key if overflow.	*/
		{
		    if (echo)
			AnsiWriteData (s, "\b \b", 3);
		    (*cooked)--;
		}
		cook[(*cooked)++] = (char) ch;
		if (echo)
		    AnsiWrite (s, (char) ch);
	    }
	}
	if (echo)
	    AnsiFlush (s);
    }
    k->in_flags &= ~Cooked_Done;	/* timeout reached:		*/
    return FALSE;			/* buffer not valid.		*/
}


/*************************************************************************
 * READ CHARACTERS FROM A WINDOW'S COOKED BUFFER
 *
 * - Characters are put into the cooked buffer by Put_Cooked (),
 *   they are extracted here.
 *
 * Parameter  :	w	= window
 *		buffer	= where to put the characters
 *		count	= amount of characters desired
 * Return     :	number of characters read
 *
 ************************************************************************/

static int
Get_Cooked (Window *w, char *buffer, int count)
{
    Keyboard	*k	= &w->Keyboard;
    char	*cook	= k->in_cook;
    int		*cooked	= &k->in_count;
    int		result;

    if (*cooked == 0 || count == 0)	/* cooker empty or silly count?	*/
    {
	k->in_flags &= ~Cooked_Done;	/* reset Done flag,		*/
	return 0;			/* nothing else to do.		*/
    }

    if (*cooked <= count)		/* less data than requested	*/
    {
	memcpy (buffer, cook, *cooked);	/* copy the data to the buffer	*/
	result = *cooked;
	k->in_flags &= ~Cooked_Done;	/* reset Done flag,		*/
	*cooked = 0;			/* reset the cooked counter	*/
    }
    else				/* more data than requested	*/
    {
	memcpy (buffer, cook, count);	/* copy the desired amount	*/
	result = count;
					/* adjust the cooked buffer	*/
	memmove (&cook[0], &cook[count], *cooked - count);
	*cooked -= count;		/* ans the cooked counter	*/
    }
    return result;
}


/*************************************************************************
 * READ CHARACTERS IN RAW MODE
 *
 * - First, empty the cooked buffer.
 * - Then try to get characters from the raw buffer within the timeout.
 *
 * Parameter  :	w	= window
 *		buffer	= where to put the characters
 *		count	= amount of characters desired
 *		timeout	= timelimit
 * Return     :	number of characters read
 *
 ************************************************************************/

static int
ReadRaw (Window *w, char *buffer, int count, int timeout, word *rc)
{
#ifdef	DEBUG
    static char	*fname	= "ReadRaw";
#endif
    Screen	*s	= &w->Screen;
    word	tstep	= OneSec / 200;
    word	echo	= w->echo;
    int		result;
    int		ch;

    
#ifdef	DEBUG
    Debug (READ) ("%s started.", fname);
#endif

    result = Get_Cooked (w, buffer, count);	/* empty the cooker	*/

#ifdef	DEBUG
    Debug (READ_ALL) ("%s : got %d chars from cooker.", fname, result);
#endif

    while (result < count)
    {
	if ((ch = ReadChar (w)) < 0)	/* read raw character		*/
	{				/* no character available :	*/
#if 0
	  /*
	   * XXX - NC - 23/3/93 (ref bug no )
	   *
	   * There should be no need to translate ctrl-C into ctrl-D
	   * and if this bit of code is removed then telnetd and rlogind
	   * appear to work.  Hence ...
	   */
	  
	    if (ch == -2)		/* check for Ctrl-C		*/
	    {				/* Ctrl-C event occured:	*/
	    	*rc = ReadRc_EOD;	/* return with EOD code.	*/
	    	return result;
	    }
#endif
	    Delay (tstep);		/* sleep a little bit,		*/
	    if ((timeout -= (int)tstep) < 0)	/* then check for timeout :	*/
		break;			/* reached, so break		*/
	    continue;			/* else try reading once more	*/
	}
	buffer[result++] = (char) ch;	/* put the char into the buffer	*/
	if (echo)			/* write out if echo		*/
	{
	    AnsiWrite (s, (char) ch);
	    AnsiFlush (s);
	}
    }

    if (result)
	*rc = ReadRc_EOD;
    else
	*rc = SS_Window + EC_Recover + EG_Timeout + EO_Stream;
    return result;
}


/*************************************************************************
 * READ CHARACTERS IN COOKED MODE
 *
 * - First, empty the cooked buffer.
 * - Then try to get characters from the raw buffer within the timeout.
 *
 * Parameter  :	w	= window
 *		buffer	= where to put the characters
 *		count	= amount of characters desired
 *		timeout	= timelimit
 *		rc	= return code pointer
 *		sflags	= stream flags pointer
 * Return     :	number of characters read
 *
 ************************************************************************/

static int
ReadCooked (Window *w, char *buffer, int count, int timeout, word *rc, word *sflags)
{
    Keyboard	*k	= &w->Keyboard;
    int		result;

     
    if ((k->in_flags & Cooked_Done) &&	/* cooker valid: get data	*/
      (result = Get_Cooked (w, buffer, count)) > 0)
    {					/* get the rest from the cooker	*/
	if ((*sflags & Cooked_EOF) && (!k->in_count))
	    *rc = ReadRc_EOF;		/* Ctrl-D and buffer empty: EOF	*/
	else
	    *rc = ReadRc_EOD;		/* else only EOD		*/
        AnsiFlush (&w->Screen);		/* now flush the screen.	*/
	return result;
    }

    if (*sflags & Cooked_EOF)		/* trying to read past Ctrl-D ?	*/
    {
	*rc = ReadRc_EOF;		/* yes, set EOF code		*/
	return 0;
    }
					/* try to fill cooker		*/
    unless (Put_Cooked (w, timeout, sflags) && (k->in_flags & Cooked_Done))
    {
	*rc = SS_Window + EC_Recover + EG_Timeout + EO_Stream;
	return 0;
    }

    result = Get_Cooked (w, buffer, count);	/* get data from cooker	*/

    if ((*sflags & Cooked_EOF) && (!k->in_count))
	*rc = ReadRc_EOF;		/* Ctrl-D and buffer empty: EOF	*/
    else
	*rc = ReadRc_EOD;		/* else only EOD		*/
    AnsiFlush (&w->Screen);		/* now flush the screen.	*/
    return result;
}


/*************************************************************************
 * PERFORM THE READ REQUEST
 *
 * - This function is forked by WindowRead () to perform the reading.
 *   Until termination, the ReadLock semaphore inhibits other read
 *   requests.
 *
 * Parameter  :	w	= window to read from
 *		size	= number of bytes to read
 *		timeout	= read timeout
 *		reply	= port where to send the data
 *		sflags	= stream flags pointer
 * Return     :	- nothing -
 *
 ************************************************************************/

void
HandleRead (Window *w, word size, word timeout, Port reply, word *sflags)
{
    MCB		m;
    word	control[1];
    char	data[IOCDataMax + 1];
    word	rc;

#ifdef	DEBUG
    Debug (READ) ("/%s/%s : HandleRead request %d bytes, port %x, timeout %d.",
	my_nte, w->ObjNode.Name, size, reply, timeout);
#endif

    if (w->raw_in)			/* call appropiate read fn.	*/
	size = ReadRaw (w, data, (int)size, (int)timeout, &rc);
    else
	size = ReadCooked (w, data, (int)size, (int)timeout, &rc, sflags);

    InitMCB (&m, 0, reply, NullPort, rc);	/* build reply message.	*/
    m.MsgHdr.DataSize = (int)size;
    m.Control = control;
    m.Data = (byte *) data;
    PutMsg (&m);			/* Send the result back.	*/

    Signal (&w->ReadLock);		/* unlock read semaphore.	*/

#ifdef	DEBUG
    Debug (READ) ("/%s/%s : HandleRead ready, size = %x, rc = %08x.",
	my_nte, w->ObjNode.Name, size, rc);
    data [size] = '\0';
    Debug (READ_ALL) ("HandleRead got %L", data );
#endif
    
}

/*************************************************************************
 * HANDLE THE READ REQUEST
 *
 * - Check the read size : negative sizes are not allowed, and
 *   the size must not exceed IOCDataMax, so that the reply will
 *   consist of a single message.
 * - Check the ReadLock : if another Read is in progress, the reply
 *   will be rejected.
 * - Fork the HandleRead () to perform the reading.
 *
 * Parameter  :	m	= mcb of request message
 *		w	= window
 *		sflags	= stream flags pointer
 * Return     :	- nothing -
 *
 ************************************************************************/

void
WindowRead (MCB *m, Window *w, word *sflags)
{
    ReadWrite	*rw	= (ReadWrite *) m->Control;
    word	timeout	= rw->Timeout;
    word	size	= rw->Size;

#ifdef	DEBUG
    Debug (READ) ("/%s/%s : Read request %d bytes, port %x, timeout %d.",
	my_nte, w->ObjNode.Name, size, m->MsgHdr.Reply, timeout);
#endif

    if (size < 0)			/* Check the read size :	*/
    {					/* no negative sizes allowed !	*/
#ifdef	DEBUG
	Debug (ERROR) ("/%s/%s : Read size %d illegal !",
	    my_nte, w->ObjNode.Name, size);
#endif
	ErrorMsg (m, EC_Error | EG_Parameter | 2);
	return;
    }

    if (size > IOCDataMax)		/* limit the read size : max.	*/
	size = IOCDataMax;		/* size is IOCDataMax.		*/

    timeout = (timeout < 0 || timeout > IdleTimeout) ? IdleTimeout : timeout;

/*    if (TestSemaphore (&w->ReadLock) < 1)	/ * check active Read	* /
    {
#ifdef	DEBUG
	Debug (READ) ("HandleRead currently active.");
#endif
	ErrorMsg (m, SS_Window + EC_Recover + EG_InUse + EO_Port);
	return;
    }
*/


    Wait(&w->SelectLock);
    if( w->SelectPort && w->SelectMode & O_ReadOnly )
    {
    	FreePort(w->SelectPort);
    	w->SelectPort = NullPort;
    } 
    Signal(&w->SelectLock);    
    

/* this Wait will NOT wait, because no other process can get access to	*/
    Wait (&w->ReadLock);	/* the ReadLock (ObjNode locked !)	*/

    unless (Fork (2000, HandleRead, 20, w, size, timeout, m->MsgHdr.Reply, sflags))
    {
#ifdef	DEBUG
	Debug (ERROR) ("WindowRead failed to fork HandleRead !");
#endif
	ErrorMsg (m, SS_Window + EC_Warn + EG_NoMemory + EO_Server);
	Signal (&w->ReadLock);
	return;
    }

#ifdef	DEBUG
    Debug (READ) ("/%s/%s : Read started.", my_nte, w->ObjNode.Name);
#endif
}


/*************************************************************************
 * WRITE SOME DATA TO A WINDOW
 *
 * - Check the raw_out flag: If set, write the data without any checks,
 *   else check for printable characters and newlines.
 *
 * Parameter  :	w	= window where the data goes
 *		data	= data to be written
 *		count	= amount of data
 * Return     :	- nothing -
 *
 ************************************************************************/

static void
WriteData (Window *w, char *data, int count)
{
    Screen	*s	= &w->Screen;
    char	ch;

    if (w->raw_out)
	AnsiWriteData (s, data, count);
    else
    {
	while ((count)--)
	{
	    ch = *data++;
	    if (ch == '\n')
		AnsiWrite (s, '\r');
#if 0 /* on closer reading of the ANSI spec... */
	    if ((32 <= ch && ch <= 255) || (0x07 <= ch && ch <= 0x0d))
#endif
	    AnsiWrite (s, ch);
	}
    }
}

void
HandleWrite (MCB *m, Window *w, Semaphore *PortLock)
{
#ifdef	DEBUG
    static char	*fname	= "HandleWrite";
#endif
    char	*buffer;
    char	*bp, *lp;
    ReadWrite	*rw	= (ReadWrite *) m->Control;
    Port	dest	= m->MsgHdr.Dest;
    Port	reply	= m->MsgHdr.Reply;
    word	dsize	= m->MsgHdr.DataSize;
    word	size	= rw->Size;
    word	got	= 0;		/* number of bytes received	*/
    word	error	= Err_Null;

#ifdef	DEBUG
    Debug (WRITE_ALL) ("/%s/%s : %s started.", my_nte, w->ObjNode.Name, fname);
#endif

    if ((buffer = (char *)Malloc (size)) == NULL)	/* get local buffer	*/
    {
#ifdef	DEBUG
	Debug (ERROR) ("%s failed to allocate %d bytes buffer.", size);
#endif
	error = SS_Window + EC_Warn + EG_NoMemory + EO_Server;
	goto done;
    }
    bp = buffer;

    if (dsize > 0)			/* some data came with the	*/
    {					/* request: put it into		*/
	memcpy (buffer, m->Data, (int)dsize);	/* the local buffer.	*/
	got += dsize;
	bp += dsize;
    }

    if (got >= size)			/* that was all the work...	*/
	goto done;

    InitMCB (m, MsgHdr_Flags_preserve, reply, NullPort, WriteRc_Sizes);
    MarshalWord (m, IOCDataMax);	/* More data expected, tell the	*/
    MarshalWord (m, IOCDataMax);	/* Client about our buffer.	*/

    if ((error = PutMsg (m)) < Err_Null)	/* send Buffer sizes	*/
    {
#ifdef	DEBUG
	Debug (ERROR) ("HandleWrite error %E on sending sizes.", error);
#endif
	Free (buffer);
	Signal (PortLock);		/* release Server port.		*/
	return;
    }

    m->MsgHdr.Dest = dest;		/* now the real work starts...	*/
    m->Timeout = WriteTimeout;

    while (got < size)
    {
	if ((error = GetMsg (m)) < Err_Null)	/* get next data block	*/
	{
#ifdef	DEBUG
	    Debug (ERROR) ("/%s/%s : %s received error %E.",
		my_nte, w->ObjNode.Name, fname, error);
#endif
	    Free (buffer);
	    Signal (PortLock);		/* release Server port.		*/
	    return;			/* on error, abort transfer.	*/
	}
	dsize = m->MsgHdr.DataSize;
	if (got + dsize > size)		/* check data size. (must not	*/
		dsize = size - got;	/* be more than size - got)	*/

	memcpy (bp, m->Data, (int)dsize);
	got += dsize;
	bp += dsize;
    }

done:

/* When I get here, an error occured or all data is in the buffer	*/
/* and the Client waits for a Reply and I can release the Server port.	*/

    InitMCB (m, 0, reply, NullPort, error < 0 ? error : WriteRc_Done);
    MarshalWord (m, got);
    PutMsg (m);

    Signal (PortLock);

#ifdef	DEBUG
    Debug (WRITE_ALL) ("%s : %s released ServerPort.",
	fname, w->ObjNode.Name);
#endif

    bp = buffer;			/* Now I can write the received	*/
    dsize = 0;				/* data to the screen.		*/
    while (dsize < got)			/* By doing this line by line,	*/
    {					/* the user has a chance to	*/
	lp = bp;			/* stop output by pressing ^S.	*/
	size = 0;
	do
	{				/* search for end of line	*/
	    lp++;
	    size++;
	}
	while (*lp != '\0' && *lp != '\n' && dsize + size < got);
	while (w->Xoff)			/* check for Xoff		*/
	{
	    Delay (OneSec / 10);
	}
	WriteData (w, bp, (int)size);	/* write the line.		*/
	bp = lp;
	dsize += size;
    }
    AnsiFlush (&w->Screen);		/* now flush the screen.	*/
    Free (buffer);

    Signal (&w->WriteLock);		/* unlock Write semaphore.	*/
#ifdef	DEBUG
    Debug (WRITE_ALL) ("/%s/%s : %s ready.", my_nte, w->ObjNode.Name, fname);
#endif
}

/*************************************************************************
 * HANDLE THE WRITE REQUEST
 *
 * - The ConsolePause is handled on the Message level:
 *   If the console is enabled, then proceed by forking HandleWrite ().
 * - To check for low memory, the buffer allocation is done within
 *   HandleWrite.
 *
 * Parameter  :	m	= mcb of request message
 *		w	= window
 * Return     :	- nothing -
 *
 ************************************************************************/

void
WindowWrite (MCB *m, Window *w)
{
    Semaphore	PortLock;
    ReadWrite	*rw	= (ReadWrite *) m->Control;
    Port	reply	= m->MsgHdr.Reply;
    word	timeout	= rw->Timeout;
    word	size	= rw->Size;
    word	tlimit;
    word	tstep	= OneSec / 10;

#ifdef	DEBUG
    Debug (WRITE) ("/%s/%s : Write request %d bytes, port %x, timeout %d.",
	my_nte, w->ObjNode.Name, size, reply, timeout);
#endif

    if (size <= 0)			/* nothing to be written :	*/
    {					/* send Done message.		*/
#ifdef	DEBUG
	Debug (WRITE) ("Write size <= 0.");
#endif
	InitMCB (m, 0, reply, NullPort, WriteRc_Done);
	MarshalWord (m, 0);
	PutMsg (m);
	return;
    }
					/* calculate private timeout	*/
    tlimit = (timeout < 0 || timeout > 20 * OneSec) ? 20 * OneSec : timeout;

    while (w->Xoff)			/* wait until output is enabled	*/
    {
	Delay (tstep);			/* sleep 1/10 sec		*/

	if ((tlimit -= tstep) <= 0)	/* timelimit reached ?		*/
	{				/* yes, abort transfer		*/
#ifdef	DEBUG
	    Debug (WRITE) ("Write timed out on Xoff.");
#endif
	    ErrorMsg (m, SS_Window + EC_Recover + EG_Timeout + EO_Stream);
	    return;
	}
    }

/*    if (TestSemaphore (&w->WriteLock) < 1)	/ * check active Write	* /
    {
#ifdef	DEBUG
	Debug (WRITE) ("HandleWrite currently active at %x.", &w->WriteLock);
#endif
	ErrorMsg (m, SS_Window + EC_Recover + EG_InUse + EO_Port);
	return;
    }
*/

    Wait(&w->SelectLock);
    if( w->SelectPort && w->SelectMode & O_WriteOnly )
    {
    	FreePort(w->SelectPort);
    	w->SelectPort = NullPort;
    } 
    Signal(&w->SelectLock);    

/* this Wait will NOT wait, because no other process can get access to	*/
    Wait (&w->WriteLock);	/* the WriteLock (ObjNode locked !)	*/

    InitSemaphore (&PortLock, 0);	/* initialise Server Port Lock	*/

    if (Fork (2000, HandleWrite, 12, m, w, &PortLock))
    {
#ifdef	DEBUG
	Debug (WRITE_ALL) ("/%s/%s : HandleWrite forked.", my_nte, w->ObjNode.Name);
#endif
					/* wait for HandleWrite to	*/
	Wait (&PortLock);		/* complete transfer.		*/
#ifdef	DEBUG
	Debug (WRITE) ("/%s/%s : Write ready.", my_nte, w->ObjNode.Name);
#endif
    }
    else
    {
#ifdef	DEBUG
	Debug (WRITE) ("WindowWrite failed to fork HandleWrite.");
#endif
	ErrorMsg (m, SS_Window + EC_Warn + EG_NoMemory + EO_Server);
    }
}


/*************************************************************************
 * HANDLE THE SELECT REQUEST
 *
 * - Return a message to the given port when either data is available
 *   or a write can proceed. If the required condition is not true, create
 *   a process to wait for it to happen.
 *
 * Parameter  :	w	= window
 *            : m       = request message
 * Return     :	- nothing -
 *
 ************************************************************************/

#define TestWriteReady(w) (!(w)->Xoff)

#define TestReadReady(w) ((w)->Keyboard.in_head != (w)->Keyboard.in_tail)

void HandleSelect(Window *w)
{
	word result = 0;
	
	for(;;)
	{
		Delay(OneSec/50);
		
		Wait(&w->SelectLock);
	
		if( w->SelectPort != NullPort )
		{
			if( w->SelectMode & O_ReadOnly && TestReadReady(w) ) result |= O_ReadOnly;
			if( w->SelectMode & O_WriteOnly && TestWriteReady(w) ) result |= O_WriteOnly;
	
			if( result )
			{
				MCB m;
				InitMCB(&m,0,w->SelectPort,NullPort,result);
				PutMsg(&m);
				w->SelectPort = NullPort;
				goto done;
			}
		}
		else goto done;
		
		Signal(&w->SelectLock);	
	}
	
done:
	Signal(&w->SelectLock);	
}

void
WindowSelect(MCB *m, Window *w)
{
	word mode = m->MsgHdr.FnRc & FF_Mask;
	word result = 0;
	
	Wait(&w->SelectLock);

	if( mode & O_ReadOnly && TestReadReady(w) ) result |= O_ReadOnly;
	if( mode & O_WriteOnly && TestWriteReady(w) ) result |= O_WriteOnly;
	
	if( result )
	{
#if 0
		if( w->SelectPort != NullPort )
		{
			InitMCB(m,0,w->SelectPort,NullPort,result);
			PutMsg(m);
			w->SelectPort = NullPort;
		}
#endif
		InitMCB(m,0,m->MsgHdr.Reply,NullPort,result);
		PutMsg(m);
	}
	else
	{
		int handler_running = w->SelectPort != NullPort;
		w->SelectMode = mode;
		FreePort(w->SelectPort);
		w->SelectPort = m->MsgHdr.Reply;
		if( !handler_running ) Fork(1000, HandleSelect, sizeof(w), w);
	}
	
	Signal(&w->SelectLock);
}

/*************************************************************************
 * HANDLE THE GETINFO REQUEST
 *
 * - Marshal the window's Attributes into the data vector.
 * - Send the reply message.
 *
 * Parameter  :	w	= window
 * Return     :	- nothing -
 *
 ************************************************************************/

void
WindowGetInfo (MCB *m, Window *w)
{
    Port	reply	= m->MsgHdr.Reply;

    
    Wait (&Attr_Lock);

#ifdef	DEBUG
    Debug (ATTR) ("%s/%s: GetInfo returns: R %d, C %d", nte->Name, w->ObjNode.Name, w->Attribs.Min, w->Attribs.Time );
/*    Debug (ATTR) ("%s/%s: GetInfo returns: R %d, C %d\n\r\t\t%s%s%s%s%s",
    	nte->Name, w->ObjNode.Name,
    	w->Attribs.Min, w->Attribs.Time,
        IsAnAttribute (&w->Attribs, ConsoleEcho) ? " echo" : "",
        IsAnAttribute (&w->Attribs, ConsoleIgnoreBreak) ? " ignbrk" : "",
        IsAnAttribute (&w->Attribs, ConsoleBreakInterrupt) ? " brkintr" : "",
        IsAnAttribute (&w->Attribs, ConsoleRawInput) ? " rawin" : "",
        IsAnAttribute (&w->Attribs, ConsoleRawOutput) ? " rawout" : "");
*/
#endif

    InitMCB (m, 0, reply, NullPort, Err_Null);

    MarshalData (m, sizeof (Attributes), (byte *) &w->Attribs);

    Signal (&Attr_Lock);

    PutMsg (m);
}


/*************************************************************************
 * HANDLE THE SETINFO REQUEST
 *
 * - Copy the new attributes from the data vector, preserving the
 *   window size (which cannot be changed !)
 * - Send the reply message.
 *
 * Parameter  :	m	= MCB of request message
 *		w	= window
 * Return     :	- nothing -
 *
 ************************************************************************/

void
WindowSetInfo (MCB *m, Window *w)
{
    Port	reply	= m->MsgHdr.Reply;

    Wait (&Attr_Lock);
    
#ifdef	DEBUG
    Debug (ATTR) ("%s/%s: SetInfo before: R %d, C %d", nte->Name, w->ObjNode.Name, w->Attribs.Min, w->Attribs.Time );
/*    Debug (ATTR) ("%s/%s: SetInfo before: R %d, C %d\n\r\t\t%s%s%s%s%s",
    	nte->Name, w->ObjNode.Name,
    	w->Attribs.Min, w->Attribs.Time,
        IsAnAttribute (&w->Attribs, ConsoleEcho) ? " echo" : "",
        IsAnAttribute (&w->Attribs, ConsoleIgnoreBreak) ? " ignbrk" : "",
        IsAnAttribute (&w->Attribs, ConsoleBreakInterrupt) ? " brkintr" : "",
        IsAnAttribute (&w->Attribs, ConsoleRawInput) ? " rawin" : "",
        IsAnAttribute (&w->Attribs, ConsoleRawOutput) ? " rawout" : "");
*/
#endif

/*    Wait (&w->ReadLock);	*/	/* wait for forked requests	*/
/*    Wait (&w->WriteLock);	*/	/* to finish.			*/

    memcpy( &w->Attribs, m->Data, sizeof( Attributes ) );

    w->Attribs.Min  = w->Screen.Rows;
    w->Attribs.Time = w->Screen.Cols;

    w->echo    = IsAnAttribute( &w->Attribs, ConsoleEcho );
    w->raw_in  = IsAnAttribute( &w->Attribs, ConsoleRawInput  );
    w->raw_out = IsAnAttribute( &w->Attribs, ConsoleRawOutput );

    InitMCB (m, 0, reply, NullPort, Err_Null);

    PutMsg (m);

/*    Signal (&w->ReadLock);	*/	/* reenable forking of requests	*/
/*    Signal (&w->WriteLock);	*/

#ifdef	DEBUG
    Debug (ATTR) ("%s/%s: SetInfo after: R %d, C %d", nte->Name, w->ObjNode.Name, w->Attribs.Min, w->Attribs.Time );
/*    Debug (ATTR) ("%s/%s: SetInfo after: R %d, C %d\n\r\t\t%s%s%s%s%s",
    	nte->Name, w->ObjNode.Name,
    	w->Attribs.Min, w->Attribs.Time,
        IsAnAttribute (&w->Attribs, ConsoleEcho) ? " echo" : "",
        IsAnAttribute (&w->Attribs, ConsoleIgnoreBreak) ? " ignbrk" : "",
        IsAnAttribute (&w->Attribs, ConsoleBreakInterrupt) ? " brkintr" : "",
        IsAnAttribute (&w->Attribs, ConsoleRawInput) ? " rawin" : "",
        IsAnAttribute (&w->Attribs, ConsoleRawOutput) ? " rawout" : "");
*/
#endif

    Signal (&Attr_Lock);
}

/*************************************************************************
 * HANDLE THE DEVICE GETINFO REQUEST called by device action procedure		
 *
 * - Copy the attributes of the current window 
 *
 * Parameter  :	a	= Attributes
 * Return     :	- nothing -
 *
 ************************************************************************/

void
DeviceWindowGetInfo (Attributes *a)
{
    Window *w = Cur_Window;

    unless (w)
    	return;

    Wait (&Attr_Lock);

#ifdef	DEBUG
    Debug (ATTR) ("%s/%s: Device GetInfo returns: R %d, C %d", nte->Name, w->ObjNode.Name, w->Attribs.Min, w->Attribs.Time );
/*    Debug (ATTR) ("%s/%s: Device GetInfo returns: R %d, C %d\n\r\t\t%s%s%s%s%s",
    	nte->Name, w->ObjNode.Name,
    	w->Attribs.Min, w->Attribs.Time,
        IsAnAttribute (&w->Attribs, ConsoleEcho) ? " echo" : "",
        IsAnAttribute (&w->Attribs, ConsoleIgnoreBreak) ? " ignbrk" : "",
        IsAnAttribute (&w->Attribs, ConsoleBreakInterrupt) ? " brkintr" : "",
        IsAnAttribute (&w->Attribs, ConsoleRawInput) ? " rawin" : "",
        IsAnAttribute (&w->Attribs, ConsoleRawOutput) ? " rawout" : "");
*/
#endif

    memcpy (a, &w->Attribs, sizeof (Attributes));

    Signal (&Attr_Lock);
}


/*************************************************************************
 * HANDLE THE DEVICE SETINFO REQUEST called by device action procedure		
 *
 * - Copy the new attributes, preserving the window size (which
 *   cannot be changed !) to the current window.
 *
 * Parameter  :	a	= Attributes
 * Return     :	- nothing -
 *
 ************************************************************************/

void
DeviceWindowSetInfo (Attributes *a)
{
    Window *w = Cur_Window;
    
    unless (w)
    	return;

    Wait (&Attr_Lock);

#ifdef	DEBUG
    Debug (ATTR) ("%s/%s: Device SetInfo before: R %d, C %d", nte->Name, w->ObjNode.Name, w->Attribs.Min, w->Attribs.Time );
/*    Debug (ATTR) ("%s/%s: Device SetInfo before: R %d, C %d\n\r\t\t%s%s%s%s%s",
    	nte->Name, w->ObjNode.Name,
    	w->Attribs.Min, w->Attribs.Time,
        IsAnAttribute (&w->Attribs, ConsoleEcho) ? " echo" : "",
        IsAnAttribute (&w->Attribs, ConsoleIgnoreBreak) ? " ignbrk" : "",
        IsAnAttribute (&w->Attribs, ConsoleBreakInterrupt) ? " brkintr" : "",
        IsAnAttribute (&w->Attribs, ConsoleRawInput) ? " rawin" : "",
        IsAnAttribute (&w->Attribs, ConsoleRawOutput) ? " rawout" : "");
*/
#endif

/*    Wait (&w->ReadLock);	*/	/* wait for forked requests	*/
/*    Wait (&w->WriteLock);	*/	/* to finish.			*/

    memcpy (&w->Attribs, a, sizeof (Attributes));

    if (w->Attribs.Min == 0)
	w->Attribs.Min = w->Screen.Rows;

    if (w->Attribs.Time == 0)
	w->Attribs.Time = w->Screen.Cols;

    if (w->Attribs.Min  != w->Screen.Rows ||
	w->Attribs.Time != w->Screen.Cols  )
      {
	Window *	wp;

	
	term_rows = w->Attribs.Min;
	term_cols = w->Attribs.Time;

	term_maxrow = term_rows - 1;
	term_maxcol = term_cols - 1;

	for (wp = w->Next; wp != w; wp = wp->Next)
	  {
	    AnsiReinitScreen( &wp->Screen );

	    wp->Attribs.Min  = wp->Screen.Rows;
	    wp->Attribs.Time = wp->Screen.Cols;
	  }

	AnsiReinitScreen( &wp->Screen );
      }
    
    w->echo    = IsAnAttribute( &w->Attribs, ConsoleEcho );
    w->raw_in  = IsAnAttribute( &w->Attribs, ConsoleRawInput  );
    w->raw_out = IsAnAttribute( &w->Attribs, ConsoleRawOutput );

/*    Signal (&w->ReadLock);	*/	/* reenable forking of requests	*/
/*    Signal (&w->WriteLock);	*/


#ifdef	DEBUG
    Debug (ATTR) ("%s/%s: Device SetInfo after: R %d, C %d", nte->Name, w->ObjNode.Name, w->Attribs.Min, w->Attribs.Time );
/*    Debug (ATTR) ("%s/%s: Device SetInfo after: R %d, C %d\n\r\t\t%s%s%s%s%s",
    	nte->Name, w->ObjNode.Name,
    	w->Attribs.Min, w->Attribs.Time,
        IsAnAttribute (&w->Attribs, ConsoleEcho) ? " echo" : "",
        IsAnAttribute (&w->Attribs, ConsoleIgnoreBreak) ? " ignbrk" : "",
        IsAnAttribute (&w->Attribs, ConsoleBreakInterrupt) ? " brkintr" : "",
        IsAnAttribute (&w->Attribs, ConsoleRawInput) ? " rawin" : "",
        IsAnAttribute (&w->Attribs, ConsoleRawOutput) ? " rawout" : "");
*/
#endif

    Signal (&Attr_Lock);
}


/*************************************************************************
 * HANDLE THE ENABLEEVENTS REQUEST
 *
 * - Check the Event mask :
 *   If a CBreak event shall be enabled, store the Reply port
 *   in the window structure and save the window event port counter.
 *   If the Event shall be disabled, check the window event port counter.
 *   If the counter equals my saved value, remove the event port.
 * - Send the reply message.
 *
 * Parameter  :	m	= MCB of request message
 *		w	= window
 * Return     :	- nothing -
 *
 ************************************************************************/

void
WindowEnableEvents (MCB *m, Window *w, int *id)
{
    Port	reply	= m->MsgHdr.Reply;
    word	mask	= m->Control[0];

#ifdef	DEBUG
    Debug (CBREAK) ("/%s/%s : EnableEvents (%x) started.",
	my_nte, w->ObjNode.Name, mask);
#endif

    if (mask & Event_Break)		/* enable CBreak event		*/
    {
#ifdef OLDCODE
/* Do Nothing */
#else
/*
-- crf: 01/11/91 - Bug 811 (1)
-- Problem: tty server eating up ports
-- Fix: Free the event port
-- Not really happy with things here ... this fix can at best be regarded
-- as temporary. The general handling of break events needs to be examined 
-- at some length.
*/
	FreePort (w->EventPort) ;
#endif
	w->EventPort = reply;		/* install Event port		*/
	*id = ++(w->EventCount);	/* save EventCount		*/
    }
    else				/* disable CBreak event		*/
    {
	if (w->EventCount == *id)	/* are we the Event port owner?	*/
	{
	    w->EventPort = NullPort;	/* clear Event port		*/
	    *id = 0;			/* reset saved EventCount	*/
	}
    }

#ifdef	DEBUG
    Debug (CBREAK) ("Event for /%s/%s : Port = %x, mask = %x, id = %d.", 
	my_nte, w->ObjNode.Name, reply, mask & Event_Break, *id);
#endif
    
    InitMCB (m, MsgHdr_Flags_preserve, reply, NullPort, Err_Null);	/* send reply		*/
    MarshalWord (m, mask & Event_Break);
    PutMsg (m);

#ifdef	DEBUG
    Debug (CBREAK) ("/%s/%s : EnableEvents ready, id = %d.",
	my_nte, w->ObjNode.Name, *id);
#endif
}

/*************************************************************************
 * MAIN BODY
 *
 * - The command line is scanned for an input name, an output name 
 *   and an terminal emulation name.
 * - The termcap data base is scanned for the specified terminal entry.
 *   If the specified terminal cannot be found, the ANSI escape sequences
 *   are ignored.
 * - The dispatcher is started.
 *
 * Parameter  :	- nothing -
 * Return     :	- nothing - The server should never return.
 *
 ************************************************************************/


int
main (int argc, char **argv)
{
    char	*fname = "TTY";
    char	device_name[32];
    int		ch, i;
    word	e;
    int		fixname  = FALSE;	/* will choose my own nte	*/
    int		sendname = FALSE;	/* wont send my nte path	*/
    Object	*ti	 = NULL;
    Object	*to	 = NULL;		/* terminal inout & output	*/
    Object	*o;			/* machine root object		*/
    Object	*t;
    LinkNode	*Parent;		/* Node for parent link		*/
    TermDeviceInfo TDI;
    Attributes	Attr;

/*  PatchMalloc(); */
    
    InitSemaphore (&Window_Lock, 1);	/* init global Lock		*/
    InitSemaphore (&Attr_Lock, 1);
    InitSemaphore (&Input_Lock, 0);	/* init input alarm		*/
    InitSemaphore (&Forever, 0);	/* termination lock		*/
    InitSemaphore (&DispTerm, 0);	/* dispatcher termination	*/

    terminating = FALSE;
    
#ifdef	DEBUG
    DebugInit ();
    Debug (MAIN) ("%s started.", fname);
#endif

    strcpy (term_name, def_name);
    strcpy(device_name,def_dev);	/* use default device driver	*/
    
#ifdef	DEBUG
    Debug (MAIN) ("%s got %d args.", fname, argc);
#endif
    while ((ch = getopt (argc, argv, "i:o:l:n:t:d:upsw")) != -1)
    {
#ifdef	DEBUG
	Debug (MAIN) ("%s found %c (%s).", fname, ch, argv [optind]);
#endif
	switch (ch)
	{
	    case 'i' :
	    	if (ti != NULL)
	    	{
/*	    	    IOdebug ("%s: duplicate input !", fname); */
	    	    goto opterr;
	    	}

	    	if ((ti = Locate (NULL, optarg)) == NULL)
	    	{
/*		    IOdebug ("%s: cannot locate %s !", fname, optarg); */
		    goto opterr;
	    	}
	    	break;
	    
	    case 'o' :
	    	if (to != NULL)
	    	{
/*	    	    IOdebug ("%s: duplicate output !", fname); */
	    	    goto opterr;
	    	}

	    	if ((to = Locate (NULL, optarg)) == NULL)
	    	{
/*		    IOdebug ("%s: cannot locate %s !", fname, optarg);  */
		    goto opterr;
	    	}
	    	break;

	    case 'l' :
	    	if (to != NULL || ti != NULL)
	    	{
/*	    	    IOdebug ("%s: duplicate in/output !", fname); */
	    	    goto opterr;
	    	}

	    	if ((to = ti = Locate (NULL, optarg)) == NULL)
	    	{
/*		    IOdebug ("%s: cannot locate %s !", fname, optarg); */
		    goto opterr;
	    	}
	    	break;

	    case 't' :
	    	strncpy (term_name, optarg, 31);
	    	term_name[31] = '\0';
	    	break;

	    case 'n' :
		strncpy(my_nte, optarg, 31);
	    	my_nte[31] = '\0';
		fixname = TRUE;
	    	break;

	    case 's' :
		strncpy(device_name,def_dev,31);
		device_name[31] = '\0';
	    	break;

	    case 'p' :
		strncpy(device_name,"tpseudo.d",31);
		device_name[31] = '\0';
	    	sendname = TRUE;
	    	break;

	    case 'd':
		strncpy(device_name,optarg,31);
		device_name[31] = '\0';
	    	break;	    	
	
	    case 'w':
	    	sendname = TRUE;
		break;

	    case 'u':
	    default :
	    opterr:
	    	fprintf (stdout, "\n%s : Terminal window server (C) 1990, Parsytec GmbH\n", *argv);
	    	fprintf (stdout, "\n   :                    and (c) 1991, Perihelion Software Ltd.\n" );
	        fprintf (stderr, "Usage : %s [options]\n", *argv);
	    	fprintf (stdout, "Valid options :\n");
	    	fprintf (stdout, "-i path  Read input from <path> (default: stdin).\n");
	    	fprintf (stdout, "-o path  Write output to <path> (default: stdout).\n");
	    	fprintf (stdout, "-l path  Input and output to <path>.\n");
	    	fprintf (stdout, "-n name  Use <name> as name table entry.\n");
	    	fprintf (stdout, "-t term  Initialise for terminal <term>.\n");
	    	fprintf (stdout, "-s       Use serial pty driver (default).\n");
	    	fprintf (stdout, "-p       Use tpseudo.d pty driver.\n");
	    	fprintf (stdout, "-d dev   Use driver <dev>.\n");
	    	fprintf (stdout, "-w       Write server pathname onto output stream.\n");
	    	fprintf (stdout, "-u       Print this message.\n");
	    	fprintf (stdout, "Not more than one input and one output must be specified.\n");
	    	fprintf (stdout, "Not more than one driver must be specified.\n");
		exit (0x100);
	}
    }

    if (ti == NULL)
	line_in = Heliosno(stdin);

    if (to == NULL)
	line_out = Heliosno(stdout);
    
   
    if ( line_in == NULL && (line_in = Open (ti, NULL, O_ReadOnly) ) == NULL)
    {
/*	IOdebug ("%s: failed to open %s as input (%E) !", fname, ti->Name, Result2 (ti)); */

	exit (0x400);
    }

    if ( line_out == NULL && (line_out = Open (to, NULL, O_WriteOnly) ) == NULL)
    {
/*	IOdebug ("%s: failed to open %s as output (%E) !", fname, to->Name, Result2 (to)); */

	exit (0x400);
    }

    if ( strcmp(device_name,def_dev) == 0 ) 
        /* I assume to be connected to a serial line! */
	if ( GetAttributes(line_in, &Attr) == sizeof(Attr) )
	{
	    RemoveAttribute(&Attr, RS232_IXON);
	    (void)SetAttributes(line_in, &Attr);
	}
    
    if (ti) Close (ti);
    if (to) Close (to);
    
#ifdef	DEBUG
    Debug (MAIN_ALL) ("%s: terminal name is \"%s\"", fname, term_name);
#endif
    if (!AnsiInit (term_name) && strcmp (term_name, def_name))
    {
    	strcpy (term_name, def_name);
        AnsiInit (term_name);
    }

    (void) signal(SIGHUP, DoReinit);

    MachineName (McName);		/* Get machine name for the	*/
					/* name tree.			*/
    o = Locate (NULL, McName);

    unless (o)
    	IOdebug("PANIC Locate %s", McName);
    	
    /* find free nte for tty */

    if (fixname == FALSE)
    {
	strcpy(my_nte, "tty.0");
      
	for (i = 0; i < 100;)
	  {
	    t = Locate(o, my_nte);

	    /*
	     * XXX - NC - 23/3/93
	     *
	     * The test for EG_Broken is to catch broken ttyservers
	     * that have gone away without removing their name from
	     * the name table.
	     */
	       
	    if (t == NULL && ((Result2( o ) & EG_Mask) != EG_Broken))
		break;
	    Close(t);
/*	    IOdebug("'%s' already exists", my_nte); */
	    strcpy(my_nte, "tty.");
	    addint(my_nte, ++i);
	  }
	if (t)
	  {
	    IOdebug ("%s: no free NTE found.", fname);
	    exit (0x400);
	  }
    }

    InitNode ((ObjNode *) &WindowRoot, my_nte,
	Type_Directory, 0, DefDirMatrix);
    InitList (&WindowRoot.Entries);	/* Initialise my root node.	*/
    WindowRoot.Nentries = 0;
					/* Install my request port.	*/
    WindowInfo.ReqPort = WindowName.Port = NewPort(); /*  MyTask->Port; */
    if (WindowInfo.ReqPort == NullPort)
	exit(0x400);    

    /* Create the parent link "..".	*/
    Parent = (LinkNode *) Malloc (sizeof (LinkNode) + (word)strlen (McName));
    if (Parent == NULL)
    {
/*	IOdebug ("%s: not enough memory to build parent link.", fname); */
	exit (0x400);
    }
    InitNode (&Parent->ObjNode, "..", Type_Link, 0, DefDirMatrix);
    Parent->Cap = o->Access;
    strcpy (Parent->Link, McName);
    WindowRoot.Parent = (DirNode *) Parent;

#ifdef	DEBUG
    Debug (MAIN_ALL) ("%s creating NTE %s.", fname, my_nte);
#endif

    oserr = 0;
    
    nte = Create (o, my_nte, Type_Name, sizeof(NameInfo), (byte *) &WindowName);
    
    if (nte == 0)
      {
	IOdebug ("%s: cannot create NTE %s, oserr = %x", fname, my_nte, oserr );
	exit (0x500);
      }	    

    (void) signal(SIGTERM, Tidyup);
	
    (void) signal(SIGINT,  Tidyup);	/* XXX - NC - 23/3/93 - catch ctrl-C as well */

    Close (o);

#ifdef	DEBUG
    Debug (MAIN_ALL) ("%s NTE %s created.", fname, my_nte);
#endif

    if (sendname == TRUE)
    {
	e = Write(line_out, nte->Name, strlen(nte->Name), IdleTimeout);
	if (e != strlen(nte->Name))
	  {
/*	    IOdebug ("%s: failed to send nte->Name: %s! %x", fname, nte->Name, e); */
	    exit (0x500);
	}
    }

    TDI.NTE_Name = my_nte;
    TDI.NTE_Name[0] = 'p';
    TDI.read = line_in;
    TDI.write = line_out;

    if ( ( termdcb = OpenDevice(device_name, &TDI) ) == NULL )
    {
/*	IOdebug ("%s: failed to init Device !", fname); */
    	Delete (nte, NULL);
	exit (0x600);
    }

    if (device_init_info_req() < 0)
    {
	IOdebug ("%s: failed to init Device !", fname);
    	Delete (nte, NULL);
        CloseDevice(termdcb);
	exit (0x600);
    }
    
    unless (Fork (2000, HandleInput, 0))	/* create input handler	*/
    {
	IOdebug ("ttyserv: failed to fork Handle_Input !");
    	Delete (nte, NULL);
	exit (0x600);
    }

#ifdef	DEBUG
    Debug (MAIN) ("%s %s starting dispatcher.", fname, nte->Name);
#endif

    Dispatch (&WindowInfo);		/* This was the intialisation,	*/
					/* now comes the work.		*/

#ifdef	DEBUG    
    Debug (MAIN) ("%s %s dispatcher terminated.", fname, nte->Name);
#endif

    Signal(&DispTerm);

    if (terminating == FALSE)
	raise(SIGTERM);

    Wait(&Forever);

#ifdef	DEBUG
    Debug (MAIN) ("%s terminating.", fname);
#endif
    exit (0);

}

/*--- end of tty.c ---*/
