/*************************************************************************
**									**
**	       C O N S O L E  &  W I N D O W   S E R V E R		**
**	       -------------------------------------------		**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** input.c								**
**									**
**	- termcap-related Terminal input routines			**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	20/11/89 : C. Fleischer					**
** Changed   :	18/01/90 : C. Fleischer	 terminal reinitialisation	**
*************************************************************************/

#define __in_input	1		/* flag that we are in this module */

#include "window.h"

/*************************************************************************
 * READ A SINGLE CHARACTER FROM THE INPUT LINE
 *
 * - Try to read a character with the specified timeout.
 *   If I got a character, all is fine, else if the timeout is -1,
 *   repeat the try until I get a character.
 *
 * Parameter  :	s	= opened Stream from input device
 *		timeout = timeout in microseconds or
 *			  -1 for infinite timeout
 * Return     :	character read
 *
 ************************************************************************/

static int
GetKey (Stream *s, word timeout)
{
#ifdef	DEBUG
    static char	*fname = "GetKey";
#endif
    byte	data[1];
    word	timelimit;
    word	error;

#ifdef	DEBUG
    Debug (INPUT) ("%s started.", fname);
#endif
    timelimit = (timeout < 0 || timeout > 10 * OneSec) ? 10 * OneSec : timeout;
    do
    {
	error = Read (s, data, 1, timelimit);
#ifdef	DEBUG
	Debug (INPUT_ALL) ("%s : read returned %08x", fname, error);
#endif
	if (error == 1)
	    return (int) data[0];
    }
    until (timeout > 0 || error < Err_Null);
    return -1;
}

/*************************************************************************
 * ADD A SINGLE CHARACTER TO THE RAW BUFFER
 *
 * - Lock the raw buffer of the specified terminal.
 * - Add the new character and increment the head pointer.
 *   If the buffer is full, forget about the new character by
 *   decrementing the head pointer.
 * - Release the raw buffer.
 *
 * Parameter  :	ch	= character to be stored
 *		term	= input data area of the current window
 * Return     :	- nothing -
 *
 ************************************************************************/

static void
PutCtrlC (Keyboard *t)
{
    Semaphore	*lock	= &t->raw_lock;

    Wait (lock);			/* lock the raw buffer		*/

    t->in_flags |= Cooked_CtrlC;	/* set the CtrlC flag		*/

    Signal (lock);			/* release the raw buffer	*/
}


/*************************************************************************
 * ADD A SINGLE CHARACTER TO THE RAW BUFFER
 *
 * - Lock the raw buffer of the specified terminal.
 * - Add the new character and increment the head pointer.
 *   If the buffer is full, forget about the new character by
 *   decrementing the head pointer.
 * - Release the raw buffer.
 *
 * Parameter  :	ch	= character to be stored
 *		term	= input data area of the current window
 * Return     :	- nothing -
 *
 ************************************************************************/

static void
PutRaw (char ch, Keyboard *t)
{
#ifdef	DEBUG
    static char	*fname	= "Put_Raw";
#endif
    Semaphore	*lock	= &t->raw_lock;	/* local pointers are faster	*/
    int		*tail	= &t->in_tail;	/* than a reference		*/
    int 	*head	= &t->in_head;	/* via the t parameter.		*/

    Wait (lock);			/* lock the raw buffer		*/

#ifdef	DEBUG
    Debug (INPUT_ALL) ("%s before : head = %02x, tail = %02x ", fname, *head, *tail);
#endif

    t->in_raw[*head] = ch;		/* add the character and	*/
    *head = (*head + 1) & InBufMask;	/* increment head pointer	*/

    if (*head == *tail)			/* buffer overflow ?		*/
	*head = (*head - 1) & InBufMask;	/* decrement head ptr	*/

#ifdef	DEBUG
    Debug (INPUT_ALL) ("%s after : head = %02x, tail = %02x", fname, *head, *tail);
#endif

    Signal (lock);			/* release the raw buffer	*/
}


/*************************************************************************
 * ADD A STRING TO THE RAW BUFFER
 *
 * - Lock the raw buffer of the specified terminal.
 * - Save the current head pointer.
 * - Add the new characters and increment the head pointer.
 *   If the buffer overflows, forget about the string by
 *   restoring the saved head pointer.
 * - Release the raw buffer.
 *
 * Parameter  :	str	= character to be stored
 *		term	= input data area of the current window
 * Return     :	- nothing -
 *
 ************************************************************************/

static void
PutRawString (char *str, Keyboard *term)
{
    int	    saved_head; 		/* save old head pointer	*/
    int     overflow = FALSE;
    int     *head = &term->in_head;	/* a local pointer acts faster	*/

    Wait (&term->raw_lock);		/* lock the raw buffer		*/
    saved_head = *head;
    do
    {
	term->in_raw[*head] = *str++;	/* add the character and	*/
	*head = (*head + 1) & InBufMask;	/* increment head ptr	*/

	overflow = *head == term->in_tail;	/* buffer overflow ?	*/
    }
    until (overflow || *str == '\0');

    if (overflow)			/* if overflow, restore head	*/
	*head = saved_head;

    Signal (&term->raw_lock);		/* release the raw buffer	*/
}


/*************************************************************************
 * SEND A CBREAK EVENT
 *
 * - Build an IOevent message and send it to the specified port.
 *
 * Parameter  :	port	= Event port
 * Return     :	- nothing -
 *
 ************************************************************************/

static void
SendEvent (Port port)
{
#ifdef	DEBUG
    static char	*fname	= "SendEvent";
#endif
    MCB 	mcb;
    IOEvent	event;
    word	error;

#ifdef	DEBUG
    Debug (CBREAK) ("%s to %x.", fname, port);
#endif
    event.Type = Event_Break;
    event.Counter = 0;
    event.Stamp = _cputime ();
    InitMCB (&mcb, MsgHdr_Flags_preserve, port, NullPort, EventRc_IgnoreLost);
    mcb.Timeout = 0;
    mcb.Control = NULL;
    mcb.Data = (byte *) &event;
    error = PutMsg (&mcb);
#ifdef	DEBUG
    Debug (CBREAK) ("%s = %x.", fname, error);
#endif
}


/*************************************************************************
 * SWITCH TO ANOTHER WINDOW
 *
 * - Find out which window is the next (using the DirNode /window)
 * - Set the Cur_Window pointer to point to the next window.
 *
 * Parameter  :	- nothing -
 * Return     :	- nothing -
 *
 ************************************************************************/

static void
SwitchWindow (int dir)
{
    Wait (&Window_Lock);		/* lock window globals		*/

    if (dir == ANSI_next)
	SetCurrentWindow (Cur_Window->Next);
    elif (dir == ANSI_last)
	SetCurrentWindow (Cur_Window->Prev);
    else
	SetCurrentWindow (Cur_Window);

    Signal (&Window_Lock);		/* release window globals	*/
}


/*************************************************************************
 * HANDLE INPUT FROM THE TERMINAL LINE
 *
 * - Read characters from the terminal.
 * - Check for window change sequences and matching strings.
 * - Handle ^C, ^S and ^Q, send event if necessary.
 * - Add matched data to the current window's raw buffer.
 *
 * Parameter  :	line	= open terminal stream
 * Return     :	- nothing -
 *
 ************************************************************************/

void
HandleInput (Stream *line)
{
#ifdef	DEBUG
    static char	*fname = "HandleInput";
#endif
    char	in_buf[16];		/* unmatched input buffer	*/
    bool	found;			/* equal match strings found	*/
    bool	next;			/* next character necessary	*/
    int 	in_count = 0;		/* input counter		*/
    int 	matching = -1;		/* number of sequence in match	*/
    int 	chw;
    char	*c, *cc, ch;

    forever
    {
#ifdef	DEBUG
	Debug (INPUT_ALL) ("%s : try to read char.", fname);
#endif

	chw = GetKey (line, NoTimeout);	/* get next character	*/

#ifdef	DEBUG
	Debug (INPUT) ("%s : read char %x", fname, chw);
#endif

	ch = (char) chw;
	in_buf[in_count++] = ch;	/* store it in the buffer	*/

	Wait (&Input_Lock);		/* lock input statics		*/

	if (Input_Reset)		/* check for Reinitialisation	*/
	    matching = -1;		/* and reset match index	*/

	if (ch == '\x1b' && in_count == 1)	/* escape is special :	*/
	{				/* try to get the next char	*/
	    if ((chw = GetKey (line, ShortTimeout)) >= 0)
	    {				/* if any, might be a sequence	*/
		in_buf[in_count++] = (char) chw;
#ifdef	DEBUG
		Debug (INPUT) ("%s : read char %x", fname, chw);
#endif
	    }
	    else			/* else was it a single escape	*/
	    {
		Wait (&Window_Lock);	/* lock Globals,		*/
		PutRaw ('\x1b', Cur_Keyboard);	/* buffer the Escape,	*/
		Signal (&Window_Lock);	/* release Globals and		*/
		in_count = 0;		/* reset the buffer counter.	*/
		Signal (&Input_Lock);
		continue;
	    }
	}
	in_buf[in_count] = '\0';
	next = FALSE;
	    
	while (in_count && !next)	/* check whole input buffer	*/
	{
#ifdef	DEBUG
	    Debug (INPUT_ALL) ("%s : in_count = %d, in_buf = \"%s\"",
		fname, in_count, in_buf);
#endif

	    ch = in_buf[0];

	    if ((ch < 32 || ch > 126) &&	/* non-printable char	*/
		key_tablen > 0 &&	/* some match sequences given	*/
		matching < 0 &&		/* and not currently in a match	*/
		ch >= key_table[0].local_sequence[0] &&
		ch <= key_table[key_tablen - 1].local_sequence[0])
	    {
		matching = 0;
	    }

	    if (matching >= 0)		/* try to match current in_buf	*/
	    {
		found = FALSE;
		for ( ; matching < key_tablen; matching++)
		{
		    c = in_buf;
		    cc = key_table[matching].local_sequence;

		    if (*c < *cc)	/* because sequences are sorted	*/
		    {			/* the matching has failed.	*/
			matching = -1;
			break;
		    }
					/* compare strings		*/
		    for ( ;*c && *cc && *c == *cc; c++, cc++) ;

		    if (*cc == '\0')	/* strings are equal		*/
		    {
			found = TRUE;
			break;
		    }
		    if (*c == '\0')	/* buffer matches		*/
		    {
			next = TRUE;	/* more characters needed	*/
			break;
		    }
		}

		if (next)		/* get the next character	*/
		{
		    Signal (&Input_Lock);
		    continue;
		}
		if (found)		/* strings matched :		*/
		{			/* check for special sequence	*/
		    cc = key_table[matching].helios_sequence;
		    matching = -1;
		    if (*cc == 'n')
			SwitchWindow (ANSI_next);
		    elif (*cc == 'p')
			SwitchWindow (ANSI_last);
		    else
		    {			/* No special sequence,		*/
		    	Wait (&Window_Lock);
			PutRawString (cc, Cur_Keyboard);
			Signal (&Window_Lock);
		    }
		    in_count = 0;
		    if (*c != '\0')	/* sequence was only part of	*/
		    {			/* in_buf, so clean in_buf	*/
			for (cc = in_buf; *c; c++, cc++, in_count++)
			    *cc = *c;
			*cc = '\0';
		    }
		    Signal (&Input_Lock);
		    continue;
		}
		if (matching >= key_tablen)	/* end of table reached	*/
		    matching = -1;	/* so reset matching to -1	*/
	    }

	    Signal (&Input_Lock);	/* release input statics	*/

	    Wait (&Window_Lock);	/* lock globals			*/
	    
	    if (ch == '\x03')		/* Ctrl-C			*/
	    {
#ifdef	DEBUG
		Debug (CBREAK) ("%s got ^C", fname);
#endif
		if (IsAnAttribute (&Cur_Window->Attribs, ConsoleIgnoreBreak))
		{
#ifdef	DEBUG
		    Debug (CBREAK) ("^C ignored.");
#endif
		    Signal (&Window_Lock);
		    continue;		/* if breaks ignored, ignore ^C	*/
		}
					/* if BreakInt and handler	*/
		elif (IsAnAttribute (&Cur_Window->Attribs, ConsoleBreakInterrupt)
		    && (Cur_Window->EventPort != NullPort))
		    {
#ifdef	DEBUG
			Debug (CBREAK) ("Sending Event for ^C.");
#endif
					/* then try to send Break event	*/
			SendEvent (Cur_Window->EventPort);
			PutCtrlC (Cur_Keyboard);
		    }

		else			/* else put into buffer		*/
		{
#ifdef	DEBUG
		    Debug (CBREAK) ("Put ^C to Input buffer.");
#endif
		    PutRaw (ch, Cur_Keyboard);
		}
	    }
	    elif (ch == '\x11')		/* Ctrl-Q			*/
	    {
		if (Cur_Window->Xoff)	/* re-enable output		*/
		    Cur_Window->Xoff = FALSE;

		else			/* else put into buffer		*/
		    PutRaw (ch, Cur_Keyboard);
	    }
	    elif (ch == '\x13')
	    {
		if (IsAnAttribute (&Cur_Window->Attribs, ConsolePause))
		    Cur_Window->Xoff = TRUE;	/* stop output		*/

		else			/* else put into buffer		*/
		    PutRaw (ch, Cur_Keyboard);
	    }

	    else			/* other keys : add to raw buf	*/
		PutRaw (ch, Cur_Keyboard);

	    Signal (&Window_Lock);
	    
#ifdef	DEBUG
	    Debug (INPUT_ALL) ("%s : before in_count = %d, in_buf = \"%s\"",
		fname, in_count, in_buf);
#endif

	    c = cc = in_buf;		/* delete ch from in_buf	*/
	    c++;
	    while ((*cc++ = *c++) != '\0')
		;
	    in_count--;

#ifdef	DEBUG
	    Debug (INPUT_ALL) ("%s : after in_count = %d, in_buf = \"%s\"",
		fname, in_count, in_buf);
#endif
	}
    }
}


/*************************************************************************
 * COMPARE KEY DEFINITIONS
 *
 * - This function is used as an argument to qsort().
 * Parameter  :	k1	= first key definition
 *		k2	= second key definition
 * Return     :	-1	= k1 < k2
 *		0	= k1 = k2
 *		+1	= k1 > k2
 *
 ************************************************************************/

static int
key_comp (const key_def *k1, const key_def *k2)
{
    return strcmp (k1->local_sequence, k2->local_sequence);
}

/*************************************************************************
 * INITIALISE THE INPUT SCAN STRINGS
 *
 * - Get the termcap entries for the helios special keys.
 *   Set unspecified entries to point to the null string.
 *   Delete the padding information.
 * - Get the termcap entries for some other terminal parameters.
 *
 * Parameter  :	- nothing -
 * Return     :	- nothing -
 *
 ************************************************************************/

void
InputInit (void)
{
#ifdef	DEBUG
    static char	*fname = "InputInit";
#endif
    List	key_list;
    tcap_def	*def;
    key_node	*node;
    int 	keys = 0;
    int		i;
    char	*temp;

#ifdef	DEBUG
    Debug (SETTERM) ("%s started.", fname);
#endif

    InitList (&key_list);

    if (key_table != NULL)
    {
    	Free (key_table);
    	key_table = NULL;
    	key_tablen = 0;
    }
    
    for (def = tcap_table; *def->tcap_name != '\0'; def++)
    {
	temp = tgetstr (def->tcap_name, &tcap_index);
	if (temp != NULL)
	{
	    if ((node = Malloc (sizeof (key_node))) == NULL)
	    {
#ifdef	DEBUG
		Debug (ERROR) ("%s : out of memory whilst scanning termcap.",
		    fname);
#endif
		break;
	    }
	    node->local_sequence = temp;
	    node->helios_sequence = def->helios_sequence;
	    AddTail (&key_list, (Node *) node);
	    keys++;
	}
	elif (def->helios_sequence[0] == 'n')
	{
#ifdef	DEBUG
	    Debug (ERROR) ("%s : no sequence defined for <next window>.",
	    	fname);
#endif
	}
	elif (def->helios_sequence[0] == 'p')
	{
#ifdef	DEBUG
	    Debug (ERROR) ("%s : no sequence defined for <previous window>.",
	    	fname);
#endif
	}
    }
    if ((key_table = (key_def *) Malloc (keys * sizeof (key_def))) == NULL)
    {
#ifdef	DEBUG
	Debug (ERROR) ("%s : out of memory while building key_table.",
	    fname);
#endif
	key_table = NULL;
	key_tablen = 0;
	return;
    }
    key_tablen = keys;
    for (i = 0; i < keys; i++)
    {
	node = (key_node *) RemHead (&key_list);
	key_table[i].local_sequence = node->local_sequence;
	key_table[i].helios_sequence = node->helios_sequence;
	Free (node);
    }
    qsort ((void *) key_table, keys, sizeof (key_def), (CompFnPtr) key_comp);

#ifdef	DEBUG
    Debug (SETTERM) ("%s ready.", fname);
#endif
}


/*************************************************************************
 * CLEAN UP THE INPUT SCAN TABLE
 *
 * - Free the key_table if it was allocated before.
 *
 * Parameter  :	- nothing -
 * Return     :	- nothing -
 *
 ************************************************************************/

void
InputTidy (void)
{
    if (key_table != NULL)
    {
    	Free (key_table);
    	key_table = NULL;
    	key_tablen = 0;
    }
}
/*--- end of input.c ---*/
