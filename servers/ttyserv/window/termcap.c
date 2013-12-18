/*************************************************************************
**									**
**		     T E R M C A P   R O U T I N E S			**
**		     -------------------------------			**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** termcap.c								**
**									**
**	- UNIX-like terminal capability routines			**
**	- based on the code from Norman Azadian 			**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	16/11/89 : C. Fleischer 				**
** Changed   :	18/01/90 : C. Fleischer	 rewritten, tputs discarded	**
*************************************************************************/

#include "window.h"

#define	BUFSIZE	1024
#define Timeout	5 * OneSec

extern char	**environ;		/* user's environment		*/

static char	*ebuf = NULL;		/* pointer to entry buffer	*/


/*************************************************************************
 * FIND SPECIFIED CAPABILITY ENTRY
 *
 * Parameter :	*ip	= capability name
 * Return    :	NULL	= tgetent() not called successfully or
 *			  capability not found
 *		other	= pointer to first character after the
 *			  capability id
 *
 ************************************************************************/

static char *
findCap ( char *id )
{
    char	*p;			/* pointer into entry buffer	*/

#ifdef	DEBUG
    Debug ( TCAP ) ( "findCap ( '%s' ), ebuf = %50s", id, ebuf );
#endif

    if ( ebuf == NULL )			/* tgetent () not called before	*/
	return NULL;

    for ( p = ebuf; *p; ++p )		/* check whole ebuf		*/
    {
	if ( ( p[0] == ':' ) && ( p[1] == id[0] ) && ( p[2] == id[1] ) )
	{				/* found ...			*/
#ifdef	DEBUG
	    Debug ( TCAP_ALL ) ( "findCap (%s): p=%50s...", id, p );
#endif
	    p += 3;
	    break;
	}
    }
    if ( !*p )				/* *p = 0 : not found.		*/
	p = NULL;

#ifdef	DEBUG
    Debug ( TCAP ) ( "findCap (): returning %40s...", p );
#endif
    return p;
}


/*************************************************************************
 * FILL THE INPUT BUFFER
 *
 * - Gets the named entry from the already-opened termcap file stream
 *	into the buffer.
 * - The size of the buffer is BUFSIZE, and it is considered to be
 *	an error if the size of the entry equals or exceeds this.
 * - A terminating NULL character is placed at the end of the entry.
 * - Terminal name and newlines are removed from the entry.
 *
 * Parameter :	*s	= termcap file stream
 *		*inbuf	= input buffer
 *		*ip	= pointer to first unprocessed char
 *		*cnt	= number of chars in buffer
 * Return    :	TRUE	= end of file reached
 *		FALSE	= more data in file
 *
 ************************************************************************/

static int
fillBuffer ( Stream *s, char *inbuf, char *ip, int *cnt )
{
    char	*ptmp;
    int		stat;

#ifdef	DEBUG
    Debug ( TCAP_ALL ) ( "fillBuffer ( %S, %x, %x, %d ) started.", s, inbuf, ip, *cnt );
#endif

    if ( ip > inbuf )			/* delete garbage from inbuf	*/
    {
#ifdef	DEBUG
	Debug ( TCAP_ALL ) ( "fillBuffer deleting garbage." );
#endif

    	for ( ptmp = inbuf ; *ip != '\0' ; ++ptmp, ++ip )
            *ptmp = *ip;
        *ptmp = *ip;			/* string stopper character	*/
        ip = inbuf;
        if ( strlen( ip ) != *cnt )
        {
#ifdef	DEBUG
            Debug ( ERROR ) ( "fillBuffer (): strlen ( ip ) != *cnt." );
#endif
            *cnt = 0;
            return TRUE;
        }
    }
    else
    	ptmp = inbuf;
					/* fill inbuf with more chars	*/
    stat = Read ( s, ptmp, BUFSIZE - *cnt - 1, Timeout );
    if ( 0 < stat )			/* end of file NOT reached	*/
    {
        *cnt += stat;
        inbuf[*cnt] = '\0';
        return FALSE;
    }
    return TRUE;
}

    
/*************************************************************************
 * READ TERMINAL ENTRY FROM OPEN TERMCAP FILE
 *
 * - Gets the named entry from the already-opened termcap file stream
 *	into the buffer.
 * - The size of the buffer is BUFSIZE, and it is considered to be
 *	an error if the size of the entry equals or exceeds this.
 * - A terminating NULL character is placed at the end of the entry.
 * - Terminal name and newlines are removed from the entry.
 *
 * Parameter :	*s	= termcap file stream
 *		*outbuf	= entry buffer
 *		*name	= terminal name
 * Return    :	FALSE	= entry not found
 *		TRUE	= entry found and read ok
 *
 ************************************************************************/

static int
getEntry ( Stream *s, char *outbuf, char *name )
{
    word	namlen;			/* # chars in name		*/
    char	inbuf[BUFSIZE];		/* file is read into here	*/
    int		eof	= FALSE;	/* end of file reached ?	*/
    int		cnt	= 0;		/* # unprocessed chars in inbuf	*/
    char	*ip;			/* pointer into input buffer	*/
    char	*op;			/* pointer into output buffer	*/
    char	*ptmp;			/* temporary pointer		*/
    int		stat;			/* status of read(), etc	*/

#ifdef	DEBUG
    Debug ( TCAP ) ( "getEntry ( %s ) %08x, '%s' )", s->Name, inbuf, name );
#endif

    op = outbuf;			/* set pointers			*/
    namlen = strlen ( name );

    eof = fillBuffer ( s, inbuf, inbuf, &cnt );

    if ( eof && cnt < 1 )		/* end of file, error !		*/
    {
#ifdef	DEBUG
	Debug ( ERROR ) ( "getEntry(): file is empty" );
#endif
	goto error;
    }

#ifdef	DEBUG
    Debug ( TCAP_ALL ) ( "getEntry scanning...." );
#endif

    for ( ip = inbuf; 0 < cnt; ++ip, --cnt )
    {
#ifdef	DEBUG
	Debug ( TCAP_ALL ) ( "cnt=%d, ip='%40s...'", cnt, ip );
#endif
	stat = strspn ( ip, "\r\n \t\b\f" );	/* count leading wspace	*/
	if ( 0 < stat )			/* and skip them		*/
	{
	    ip = &ip[--stat];
	    cnt -= stat;
	}
	elif ( *ip == '#' )		/* comment line ?		*/
	{				/* skip it			*/
	    ptmp = ip;
	    ip = strchr ( ip, ( char ) '\n' );	/* find end of line	*/
	    cnt  -=  ( ip == NULL ) ? cnt : ( int )( ip - ptmp );
	}
	elif ( strncmp( name, ip, namlen ) == 0 )	/* terminal name found	*/
	{
#ifdef	DEBUG
	    Debug ( TCAP ) ( "getEntry(): SUCCESS, ip = '%40s...', cnt=%d", ip,cnt );
#endif
	    ptmp = ip;
	    ip = strchr ( ip, ( char ) ':' );	/* skip over namelist	*/
	    cnt  -=  ( ip == NULL ) ? cnt : ( int )( ip - ptmp );
	    	    
	    eof = fillBuffer ( s, inbuf, ip, &cnt );	/* refill inbuf	*/
	    ip = inbuf;
	    
#ifdef	DEBUG
	    Debug ( TCAP_ALL ) ( "getEntry(): raw entry is: '%40s'", ip );
#endif

					/* now copy entry into output	*/
					/* buffer, eliminate non-space	*/
					/* wspace and continuation \	*/

	    for ( op = outbuf; ip != NULL && *ip != '\0'; ++ip )
	    {
					/* continuation \ with crlf	*/
		if ( ip[0] == '\\' && ip[1] == '\r' && ip[2] == '\n' )
		    ip = &ip[2];
					/* continuation \ with lf	*/
		elif ( ip[0] == '\\' && ip[1] == '\n' )
		    ++ip;
					/* wspace			*/
		elif ( strchr ( "\t\r\b\f", *ip ) != NULL )
		    continue;
					/* single lf: end of entry.	*/
		elif ( *ip == '\n' )
		    break;
					/* skip duplicate colon		*/
		elif ( *ip == ':' && op[-1] == ':' )
		    continue;
					/* else copy to buffer		*/
		else
		    *op++  =  *ip;
	    }

	    if ( *ip != '\n' )		/* no lf: end of inbuf reached	*/
	    {
#ifdef	DEBUG
		Debug ( ERROR ) ( "getEntry(): entry too long" );
#endif
		goto error;
	    }

	    *op = '\0';			/* maintain outbuf[] as string	*/
#ifdef	DEBUG
	    Debug ( TCAP ) ( "\ngetEntry(): outbuf='%40s'", outbuf );
	    Debug ( TCAP ) ( "getEntry(): returning 1  [SUCCESS]" );
#endif
	    return TRUE;
	}
	else				/* terminal name not found yet,	*/
	{				/* advance to next name in list	*/
	    ptmp = ip;
	    ip = strpbrk( ip, "|:" );	/* find name delimiter		*/
	    if ( ip == NULL )
	    {
#ifdef	DEBUG
		Debug ( ERROR ) ( "getEntry(): bad format" );
#endif
		goto error;
	    }
	    cnt -= ip - ptmp;		/* skip name			*/
	    if ( *ip != '|' )		/* at end of namelist for entry	*/
	    {				/* dispose of entire entry	*/

		eof = fillBuffer ( s, inbuf, ip, &cnt );
		ip = inbuf;

#ifdef	DEBUG
		Debug ( TCAP_ALL ) ( "end of namelist, cnt=%d", cnt );
#endif

		for ( ++ip, --cnt; 0 < cnt; ++ip, --cnt )
		{
		    if ( ip[0] == '\n' &&
		      ( ( ip[-1] == '\r' && ip[-2] != '\\' )
		      || ( ip[-1] != '\r' && ip[-1] != '\\' ) ) )
		    {			/* delete this entry from inbuf	*/
		    	eof = fillBuffer ( s, inbuf, ip, &cnt );
		    	ip = inbuf;
		    	break;
		    }
		}
		if ( cnt <= 0 )
		{
#ifdef	DEBUG
		    Debug ( ERROR ) ( "getEntry(): entry too long!" );
#endif
		    goto error;
		}
	    }
	}
    	if ( cnt < BUFSIZE / 2 && !eof )
    	{
    	    eof = fillBuffer ( s, inbuf, ip, &cnt );
    	    ip = inbuf;
    	}
    }
error:
    outbuf[0] = '\0';			/* not found			*/
#ifdef	DEBUG
    Debug ( TCAP ) ( "getEntry(): returning 0  [FAILURE]" );
#endif
    return FALSE;
}


/*************************************************************************
 * READ A TERMINAL ENTRY FROM THE TERMCAP FILE
 *
 * - Looks in the environment for a TERMCAP variable.
 *	If found, and the value does not begin with a slash,
 *	the TERMCAP string is used instead of reading the termcap file.
 *	If it does begin with a slash, the string is used as a pathname
 *	rather than /helios/etc/termcap. This can speed up entry into
 *	programs that call tgetent(), as well as to help debug new
 *	terminal descriptions or to make one for your terminal
 *	if you can't write the file /etc/termcap.
 * - Extracts the entry for terminal name into the buffer at bp.
 *	bp should be a character array of size 1024 and must be
 *	retained through all subsequent calls to tgetnum(), tgetflag()
 *	and tgetstr().
 *
 * Parameter :	*bp	= user's buffer
 *		*name	= terminal type name
 * Return    :	-1	= failed to open termcap file
 *		0	= no entry for terminal name
 *		1	= everything fine.
 *
 ************************************************************************/

int
tgetent ( char *bp, char *name )
{
    char	**env = environ;
    char	*termcap;		/* pointer to $TERMCAP string	*/
    Object	*o;			/* termcap file object		*/
    Stream	*s;			/* stream to termcap file	*/
    int		retval;			/* return value			*/

#ifdef	DEBUG
    Debug ( TCAP ) ( "tgetent( %08x, '%s' )", bp, name );
#endif

    termcap = NULL;			/* scan user's environment	*/
    if ( env != NULL )
    {
    	for ( ; *env != NULL; env++ )
    	{
	    if ( strncmp ( "TERMCAP=", *env, 8 ) == 0 )
	    {
	    	termcap = &( *env )[8];	/* found : set termcap to value	*/
	    	break;
	    }
    	}
    }

#ifdef	DEBUG
    Debug ( TCAP ) ( "tgetent termcap = %x", termcap );
#endif
    
    if ( termcap != NULL && termcap[0] != '/' )
    {					/* use $TERMCAP as the entry	*/
#ifdef	DEBUG
	Debug ( TCAP ) ( "tgetent(): using contents of TERMCAP" );
#endif
	strncpy( bp, termcap, ( BUFSIZE-1 ) );
	bp[BUFSIZE] = '\0';
	termcap = "/helios/etc/termcap";
	retval = 1;			/* in case :tc capability found	*/
    }
    else
    {					/* look in termcap file		*/
	if ( termcap == NULL )
	    termcap = "/helios/etc/termcap";	/* use default file	*/

#ifdef	DEBUG
    	Debug ( TCAP ) ( "tgetent termcap = %s", termcap );
	Debug ( TCAP ) ( "tgetent(): opening file %s", termcap );
#endif
    
	o = Locate ( NULL, termcap );	/* try to find the termcap file	*/
	if ( o == NULL )
	{
#ifdef	DEBUG
	    Debug ( ERROR ) ( "tgetent( %s ): can't find termcap file '%s'",
		name, termcap );
#endif
	    retval = -1;
	}
	else
	{				/* try to open it		*/
	    s = Open ( o, NULL, O_ReadOnly );
	    if ( s == NULL )
	    {
#ifdef	DEBUG
		Debug ( ERROR ) ( "tgetent( %s ): can't open termcap file '%s'",
		    name, termcap );
#endif
		Close ( o );
		retval = -1;
	    }
	    else
	    {				/* get terminal entry		*/
		retval = getEntry ( s, bp, name );
		Close ( s );
		Close ( o );
	    }
	}
    }

    if ( retval == 1 )			/* preserve bp for future calls	*/
	ebuf = bp;

    bp = findCap( "tc" ); 		/* deal with :tc= capability	*/
    if ( bp != NULL )
    {
	char	newname[88];

#ifdef	DEBUG
	Debug ( TCAP ) ( "tgetent(): :tc found at %08x, is '%40s'", &bp[-3], &bp[-3] );
#endif
	strncpy( newname, &bp[1], sizeof newname );
	if ( strchr ( newname, ( char ) ':' ) != NULL )
	    *( strchr ( newname, ( char ) ':' ) )  = '\0';

	o = Locate ( NULL, termcap );	/* try to find the :tc file	*/
	if ( o == NULL )
	{
#ifdef	DEBUG
	    Debug ( ERROR ) ( "tgetent( %s ): can't find termcap file '%s'", name, termcap );
#endif
	    retval = -1;
	}
	else
	{				/* try to open it		*/
	    s = Open ( o, NULL, O_ReadOnly );
	    if ( s == NULL )
	    {
#ifdef	DEBUG
		Debug ( ERROR ) ( "tgetent( %s ): can't open termcap file '%s'", name, termcap );
#endif
		Close ( o );
		retval = -1;
	    }
	    else
	    {				/* get terminal entry		*/
		retval = getEntry ( s, &bp[-2], newname );
		Close ( s );
		Close ( o );
	    }
	}
    }

#ifdef	DEBUG
    Debug ( TCAP ) ( "tgetent(): returning %d", retval );
#endif
    return retval;
}


/*************************************************************************
 * GET THE VALUE OF A NUMERICAL CAPABILITY
 *
 * Parameter :	*id	= capability name
 * Return    :	-1	= not given for the terminal
 *		others	= numeric capability value
 *
 ************************************************************************/

int
tgetnum ( char *id )
{
    int		retval;
    char	*p;

#ifdef	DEBUG
    Debug ( TCAP ) ( "tgetnum( '%s' )", id );
#endif
    p = findCap ( id );
    if ( p == NULL || *p != '#' )
	retval = -1;			/* not found, or not numeric	*/
    else
    {
	retval = 0;			/* collect digits		*/
	for ( ++p; *p != ':'; ++p )
	    retval  =  ( retval * 10 ) + ( *p - '0' );
    }
#ifdef	DEBUG
    Debug ( TCAP ) ( "tgetnum( '%s' ) = %d", id, retval );
#endif
    return retval;
}


/*************************************************************************
 * GET THE VALUE OF A BOOLEAN CAPABILITY
 *
 * Parameter :	*id	= capability name
 * Return    :	FALSE	= not given for the terminal
 *		TRUE	= capability present
 *
 ************************************************************************/

int
tgetflag ( char *id )
{
    int		retval;
    char	*p;

#ifdef	DEBUG
    Debug ( TCAP ) ( "tgetflag( '%s' )", id );
#endif
    p = findCap ( id );
    retval = ( p != NULL && *p == ':' );
#ifdef	DEBUG
    Debug ( TCAP ) ( "tgetflag( '%s' ) = %d", id, retval );
#endif
    return retval;
}


/*************************************************************************
 * GET THE VALUE OF A STRING CAPABILITY
 *
 * - Returns the string value of the capability id, places it
 *	in the buffer at area, and advances the area pointer
 *	[past the terminating '\0' char].
 * - It decodes the abbreviations for this field
 *	except for cursor addressing and padding information.
 *
 * Parameter :	*id	= capability name
 *		**area	= pointer to the output pointer
 * Return    :	NULL	= not given for the terminal
 *		other	= decoded string
 *
 ************************************************************************/

char *
tgetstr ( char *id, char **area )
{
    char		*retval;	/* return value			*/
    char		*p;		/* pointer to capability string	*/
    unsigned		sum;		/* for chars given in octal	*/

#ifdef	DEBUG
    Debug ( TCAP ) ( "tgetstr( '%s', %08x ): *area=%08x", id, area, *area );
#endif

    p = findCap( id );
    if ( p == NULL || *p != '=' )
	retval = NULL;			/* not found.			*/
    else
    {
	retval = *area;			/* found, needs to be converted	*/
	for ( ++p; *p != ':'; ++p )
	{
	    if ( *p == '\\' )		/* special character		*/
		switch ( *++p )
		{			/* octal value			*/
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
#ifdef	DEBUG
		    Debug ( TCAP ) ( "p0 = %d, p1 = %d, p2 = %d, ",
		        ( p[0] - '0' ) << 6, ( p[1] - '0' ) << 3, p[2] - '0' );
#endif
		    sum = ( ( p[0] - '0' ) << 6 ) +
			( ( p[1] - '0' ) << 3 ) + ( p[2] - '0' );
#ifdef	DEBUG
		    Debug ( TCAP ) ( "sum is %d.", sum );
#endif
		    ++p;
		    ++p;
		    *( *area )++	=  ( char )( sum & 0377 );
					/** will \200 really end up	*/
					/* as \000 like it should ?	*/
		    break;
		case '^':	*( *area )++  =  '^';	break;
		case '\\':	*( *area )++  =  '\\';	break;
		case 'E':	*( *area )++  =  '\x1b';	break;
		case 'b':	*( *area )++  =  '\b';	break;
		case 'f':	*( *area )++  =  '\f';	break;
		case 'n':	*( *area )++  =  '\n';	break;
		case 'r':	*( *area )++  =  '\r';	break;
		case 't':	*( *area )++  =  '\t';	break;
		default:	*( *area )++  =  *p;	break;
		}
	    elif ( *p == '^' )		/* control character		*/
		*( *area )++  =  *++p - '@';
	    else			/* normal character		*/
		*( *area )++  =  *p;
	}
	*( *area )++ = '\0';		/* NULL-terminate the string	*/
    }

#ifdef	DEBUG
	Debug ( TCAP ) ( "tgetstr('%s') = '%L'", id, retval );
#endif

    return retval;
}


/*************************************************************************
 * PUT AN INTEGER VALUE INTO A STRING
 *
 * - Recursively puts the number digits into the string,
 *	at least width digits ( left padded with zeroes ).
 *
 * Parameter :	*dest	= destination string
 *		value	= integer value
 *		width	= minimum field width
 * Return    :		  destination pointer to next location
 *			  behind number string
 *
 ************************************************************************/

static char *
tputint ( char *dest, int value, int width )
{
    if ( value < 0 )
    {
	*dest++ = '-';
	value = -value;
    }
    if ( width > 0 || value > 0 )
    {
	dest = tputint ( dest, value / 10, width - 1 );
	*dest++ = value % 10 + '0';
    }
    return dest;
}


/*************************************************************************
 * BUILD A CURSOR ADRESSING STRING
 *
 * - Returns a cursor addressing string decoded from cm
 *	to go to column destcol in line destline.
 * - Programs which call tgoto() should be sure to turn off
 *	the XTABS bit( s ), since tgoto() may not output a tab.
 * - Note that programs using termcap should in general turn off
 *	XTABS anyway since some terminals use control I for other
 *	functions, such as non-destructive space.
 * - If a % sequence is given which is not understood, then tgoto()
 *	returns "OOPS".
 *
 * Parameter :	*cm	= cursor movement capability string
 *		destcol	= coloumn to go to	( 2nd evaluated parameter )
 *		destlin	= line to go to		( 1st evaluated parameter )
 * Return    :		  adressing string
 *
 ************************************************************************/

char *
tgoto ( char *cm, int destcol, int destlin )
{
    static char	answer[88];		/* result stashed here		*/
    bool	reversed;		/* YES when should send col 1st */
    int		value;			/* next value to output 	*/
    char	*outp;			/* pointer into answer[]	*/

#ifdef	DEBUG
    Debug (TCAP) ( "tgoto('%L', %d, %d)", cm, destcol, destlin );
#endif
    reversed = FALSE;
    value = destlin;
    outp = answer;
    for ( ;*cm; ++cm )
    {
	if ( *cm == '%' )
	{
	    switch ( *++cm )
	    {
	    case '%':
		*outp++ = '%';
		break;
	    case 'd':
		outp = tputint ( outp, value, 1 );
		value = ( reversed ) ? destlin : destcol;
		break;
	    case '2':
		outp = tputint ( outp, value, 2 );
		value = ( reversed ) ? destlin : destcol;
		break;
	    case '3':
		outp = tputint ( outp, value, 3 );
		value = ( reversed ) ? destlin : destcol;
		break;
	    case '.':
		*outp++ = value;
		value = ( reversed ) ? destlin : destcol;
		break;
	    case '+':
		*outp++ = value + *++cm;
		value = ( reversed ) ? destlin : destcol;
		break;
	    case '>':
		if ( value > *++cm )
		    value += *++cm;
		else
		    ++cm;
		break;
	    case 'r':
		value = ( reversed ) ? destlin : destcol;
		reversed = !reversed;
		break;
	    case 'i':
		++value;
		break;
	    case 'n':
		destcol  ^= 0140;
		destlin ^= 0140;
		break;
	    case 'B':
		value = ( 16 * ( value / 10 ) )  +	( value % 10 );
		break;
	    case 'D':
		value = ( value - ( 2 * ( value % 16 ) ) );
		break;
	    default:
		strcpy ( outp, "OOPS" );
		outp += 4;
		break;
	    }
#ifdef	DEBUG
	    Debug (TCAP) ( "tgoto(): reversed=%d, value=%d", reversed, value );
#endif
	}
	else
	    *outp++ = *cm;
    }
    *outp = '\0';
#ifdef	DEBUG
    Debug (TCAP) ( "tgoto(): returning '%L'", answer );
#endif
    return answer;
}

/*--- end of termcap.c ---*/
