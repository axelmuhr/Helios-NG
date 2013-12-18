/*
 * $Header: /Chris/00/helios/msc/RCS/testdrv.c,v 2.0 91/08/21 18:06:41 chris
 * Exp Locker: chris $
 */

#include "msc.h"
#include <stdio.h>
#include <ctype.h>
#include <attrib.h>
#include <nonansi.h>
#include "rdevinfo.h"

SDB            *
MakeSDB (word OwnId, Channel * Event, int nInOut)
{
    int             size;
    SDB            *sdb;
    char           *sptr;
    int             i;

/* calculate size per user	 */
    size = sizeof (Channel) + sizeof (Channel *)
      + sizeof (RCB *) + sizeof (uword);

/* calc required amount		 */
    size = sizeof (SDB) + nInOut * (size);

/* alloc for SDB and arrays	 */
    if ((sdb = (SDB *) Malloc (size)) == NULL)
	return NULL;

    sdb->OwnId = OwnId;			/* setpu variables		 */
    sdb->XI_reg0 = 0;
    sdb->XI_reg1 = 0;
    sdb->ScsiStatus = 0;
    sdb->AuxStatus = 0;
    sdb->nInOut = nInOut;
    sdb->Event = Event;
    ChanReset (sdb->Event);

/* set up arrays		 */
    sptr = (char *) sdb + sizeof (SDB);

    sdb->DevState = (uword *) sptr;
    sptr += nInOut * sizeof (uword);

    sdb->Request = (RCB **) sptr;
    sptr += nInOut * sizeof (RCB *);

    sdb->InOut = (Channel **) sptr;
    sptr += nInOut * sizeof (Channel *);
/* initialise arrays and chans	 */
    for (i = 0; i < nInOut; i++) {
	sdb->InOut[i] = (Channel *) sptr;
	ChanReset ((Channel *) sptr);
	sdb->Request[i] = NULL;
	sdb->DevState[i] = 0x20;
	sptr += sizeof (Channel);
    }
    return sdb;
}

#define min(a,b)	(((a)<(b))?(a):(b))
#define puthex(c)	fputc (((c) < 10 ? (c) + '0' : (c) + '7'), stdout )
#define putchr(c)	fputc (((c) < 32 || c > 126 ? '.' : (c)), stdout )

char           *header = "\f\n\n\n%s\n\n\
xxxxxx | x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA cB xC xD xE xF | ascii\n\
-------+-------------------------------------------------+-----------------\n";

#define C_Nop		0
#define C_Done		-1
#define C_Tab		-2
#define C_XPgUp		-3
#define C_XPgDn		-4
#define C_XUp		-5
#define C_XDown		-6
#define C_XLeft		-7
#define C_XRight	-8
#define C_PgUp		-9
#define C_PgDn		-10
#define C_Up		-11
#define C_Down		-12
#define C_Left		-13
#define C_Right		-14

int
NextKey (void)
{
    bool            esc = FALSE;
    int             ch1, ch2, ch3, ch4;

    forever
    {
	if ((ch1 = fgetc (stdin)) == 0x1b) {
	    esc = TRUE;
	    ch1 = fgetc (stdin);
	}
/****************************************************************************/
/* OI 1 April 1992 :	look for vi/emacs codes                             */

#define Cntrl(C)    ((C) - '@')

#if 0
#define LeftVI  'h'
#define RightVI 'l'
#define UpVI    'k'
#define DownVI  'j'
#define PgUpVI      Cntrl ('B')     /* == LeftEmacs */
#define PgDnVI      Cntrl ('F')     /* == RightEmacs */
#endif

#define LeftEmacs   Cntrl ('B')  	/* == PgDnVI */
#define RightEmacs  Cntrl ('F')  	/* == PgUpVI */
#define UpEmacs     Cntrl ('P')
#define DownEmacs   Cntrl ('N')

#if 0
#define PgUpEmacs   Cntrl ('Z')		/* Suspend   */
#define PgDnEmacs   Cntrl ('V')
#endif

#define Check(Editor,Direction) case Direction##Editor:         \
                                {                               \
                                    return (C_##Direction);     \
                                }                               \
                                break
                                    

        switch (ch1)
        {
#if 0
	    Check (VI, Left);
	    Check (VI, Right);
	    Check (VI, Up);
	    Check (VI, Down);
	    Check (VI, PgUp);
	    Check (VI, PgDn);
#endif

	    Check (Emacs, Left);
	    Check (Emacs, Right);
	    Check (Emacs, Up);
	    Check (Emacs, Down);

#if 0
	    Check (Emacs, PgUp);
	    Check (Emacs, PgDn);
#endif

	    default:
	    {
                /* will be processed later */
	    }
	    break;
        }

/***************************************************************************/
	    
	if (isprint (ch1))
	    return ch1;

/* OI 31 March 1992       vvvvvvvvvvvvvvv */
	if ((ch1 == '\r') || (ch1 == '\n'))
	    return C_Done;
	    
	if (ch1 == '\t')
	    return C_Tab;
	if (ch1 != 0x9b)
	    continue;
	ch2 = fgetc (stdin);
	switch (ch2) {
	case 'A':
	    return (esc ? C_XUp : C_Up);
	case 'B':
	    return (esc ? C_XDown : C_Down);
	case 'C':
	    return (esc ? C_XRight : C_Right);
	case 'D':
	    return (esc ? C_XLeft : C_Left);
	case 'H':
	    return C_XPgUp;
	case '2':
	case '3':
	case '4':
	case ' ':
	case 'T':
	case 'S':
	    ch3 = fgetc (stdin);
	    if (ch3 == 'z') {
		switch (ch2) {
		case '2':
		    return C_XPgDn;
		case '3':
		    return (esc ? C_XPgUp : C_PgUp);
		case '4':
		    return (esc ? C_XPgDn : C_PgDn);
		}
	    }
	    elif (ch3 == '~') {
		switch (ch2) {
		case 'S':
		    return C_XDown;
		case 'T':
		    return C_XUp;
		}
	    }
	    elif (ch2 == ' ' && (ch3 == '@' || ch3 == 'A')) {
		ch4 = fgetc (stdin);
		if (ch4 == '~') {
		    switch (ch3) {
		    case '@':
			return C_XRight;
		    case 'A':
			return C_XLeft;
		    }
		}
	    }
	}
    }
}

void
ShowPage (word start, word size, byte * data, char *hline)
{
    word            end = start + size;
    word            line = start & ~0xf;
    word            pos;

    printf (header, hline);
    for (line = start & ~0xf; line < end; line += 16) {
	pos = 0;
	fprintf (stdout, "%06x | ", line);
	for (pos = line; pos < line + 16; pos++) {
	    if (pos < start || pos >= end)
		fputs ("   ", stdout);
	    else {
		puthex (data[pos] >> 4);
		puthex (data[pos] & 0x0f);
		putchr (' ');
	    }
	}
	fputs ("| ", stdout);
	for (pos = line; pos < line + 16; pos++) {
	    if (pos < start || pos >= end)
		putchr (' ');
	    else
		putchr (data[pos]);
	}
	fputc ('\n', stdout);
    }
}

void
ShowData (int size, byte * Data, char *hline)
{
    int             pstart = 0;
    int             pend = (size - 1) & ~0xff;
    int             ch;

    if (size <= 0)
	return;
    ShowPage (pstart, min (256, size - pstart), Data, hline);
    forever
    {
	ch = NextKey ();
	switch (ch) {
	case C_Done:
	    return;
	case C_XPgUp:
	    if (pstart == 0)
		continue;
	    pstart = 0;
	    break;
	case C_XPgDn:
	    if (pstart >= pend)
		continue;
	    pstart = pend;
	    break;
	case C_PgUp:
	    if (pstart < 256)
		continue;
	    pstart -= 256;
	    break;
	case C_PgDn:
	    if (pstart >= pend)
		continue;
	    pstart += 256;
	    break;
	default:
	    continue;
	}
	ShowPage (pstart, min (256, size - pstart), Data, hline);
    }
}

#define gotoxy(x,y)	fprintf ( stdout, "\x9b%d;%dH", (y), (x))
#define	row(p)		(8 + ((p) >> 4))
#define ccol(p)		(60 + ((p) & 0xf))
#define xcol(p,n)	(10 + n + ((p) & 0xf) * 3)
#define xgoto(p,n)	gotoxy(xcol((p),(n)),row(p))
#define cgoto(p)	gotoxy(ccol(p),row(p))

void
do_refresh (int pos, byte data)
{
    cgoto (pos);
    putchr (data);
    xgoto (pos, 0);
    puthex (data >> 4);
    puthex (data & 15);
}

#define refresh()	do_refresh (pos,Data[start+pos])

int
EditPage (int start, int size, byte * Data, int *spos, bool * hex, bool show, char *hline)
{
    bool            low = FALSE;
    int             val;
    int             pos = *spos & 0xff;
    int             ch;

    if (show)
	ShowPage (start, size, Data, hline);
    forever
    {
	*hex ? xgoto (pos, low) : cgoto (pos);
	fflush (stdout);
	switch (ch = NextKey ()) {
	case C_Done:
	case C_XPgUp:
	case C_XPgDn:
	    goto done;
	case C_PgUp:
	    pos -= 256;
	    break;
	case C_PgDn:
	    pos += 256;
	    break;
	case C_XUp:
	    pos &= 0x0f;
	    break;
	case C_XDown:
	    pos = ((size - 1) & 0xf0) | (pos & 0x0f);
	    if (pos >= size)
		pos -= 16;
	    break;
	case C_Up:
	    pos -= 16;
	    break;
	case C_Down:
	    pos += 16;
	    break;
	case C_XLeft:
	    pos &= 0xf0;
	    break;
	case C_XRight:
	    if ((pos |= 0x0f) >= size)
		pos = size - 1;
	    break;
	case C_Left:
	    if (*hex && low)
		low = FALSE;
	    else {
		low = TRUE;
		pos--;
	    }
	    break;
	case C_Right:
	    if (*hex && !low)
		low = TRUE;
	    else {
		low = FALSE;
		pos++;
	    }
	    break;
	case C_Tab:
	    *hex = !*hex;
	    low = FALSE;
	    continue;
	default:
	    if (*hex) {
		if ('0' <= ch && ch <= '9')
		    val = ch - '0';
		elif ('A' <= ch && ch <= 'F') val = ch - '7';
		elif ('a' <= ch && ch <= 'f') val = ch - 'W';
		elif (ch == 'p' || ch == 'P') {
		    int             pp;

		    for (pp = 0; pp < size; pp++)
			Data[start + pp] = (start >> 8) + pp;
		    ShowPage (start, size, Data, hline);
		    continue;
		}
		elif (ch == 'r' || ch == 'R') {
		    int             pp;

		    for (pp = 0; pp < size; pp++)
			Data[start + pp] = 0;
		    ShowPage (start, size, Data, hline);
		    continue;
		}
		elif (ch == 's' || ch == 'S') {
		    int             pp;

		    for (pp = 0; pp < size; pp++)
			Data[start + pp] = 0xff;
		    ShowPage (start, size, Data, hline);
		    continue;
		}
		else
		continue;
		if (low) {
		    Data[start + pos] = Data[start + pos] & 0xF0 | val;
		    low = FALSE;
		    refresh ();
		    pos++;
		} else {
		    Data[start + pos] = Data[start + pos] & 0x0F | (val << 4);
		    low = TRUE;
		    refresh ();
		}
	    }
	    elif (ch > 0) {
		Data[start + pos] = ch;
		refresh ();
		pos++;
	    }
	    ch = C_Right;
	}
	if (pos < 0 || pos >= size)
	    break;
    }
done:
    *spos = (*spos & ~0xff) + pos;
    return ch;
}

#define hi(p)	((p) & ~0xff)
#define lo(p)	((p) & 0xff)

void
EditData (int size, byte * Data, char *hline)
{
    bool            hex = TRUE;
    bool            show = TRUE;
    int             pend = hi (size - 1);
    int             pos = 0;
    int             ch = 0;

    if (size <= 0)
	return;
    forever
    {
	if (pos < 0)
	    pos = 0;
	elif (pos >= size) pos = size - 1;
	ch = EditPage (hi (pos), min (256, size - hi (pos)), Data,
		       &pos, &hex, show, hline);
	show = TRUE;
	switch (ch) {
	case C_Done:
	    return;
	case C_XPgUp:
	    if (hi (pos) == 0)
		show = FALSE;
	    pos = 0;
	    continue;
	case C_XPgDn:
	    if (hi (pos) >= pend)
		show = FALSE;
	    pos = size - 1;
	    continue;
	case C_PgUp:
	case C_Up:
	case C_Left:
	    if (pos < 0)
		show = FALSE;
	    continue;
	case C_PgDn:
	    if (hi (pos - 256) == pend)
		show = FALSE;
	    continue;
	case C_Down:
	    if (hi (pos - 16) == pend)
		show = FALSE;
	    continue;
	case C_Right:
	    if (hi (pos - 1) == pend)
		show = FALSE;
	    continue;
	}
    }
}


Attributes      oattr;
Attributes      nattr;
Stream         *input;

void
InitScreen (void)
{
    input = Heliosno (stdin);
    GetAttributes (input, &oattr);
    nattr = oattr;
    RemoveAttribute (&nattr, ConsoleEcho);
    RemoveAttribute (&nattr, ConsolePause);
    RemoveAttribute (&nattr, ConsoleIgnoreBreak);
    RemoveAttribute (&nattr, ConsoleBreakInterrupt);
    RemoveAttribute (&nattr, ConsoleRawOutput);
    AddAttribute (&nattr, ConsoleRawInput);
    SetAttributes (input, &nattr);
    setvbuf (stdin, NULL, _IONBF, 0);
}

void
TidyScreen (void)
{
    fputs ("\x09b25B\r\x9bJ", stdout);
    SetAttributes (input, &oattr);
}

/*************************************************************************
 * GET A LINE FROM THE TERMINAL
 *
 * - Characters are read and stored in a buffer.
 * - Reading terminates with CR or LF,
 *   BS and DEL are available for editing.
 *
 * Parameter  :	line	= buffer for the line
 *		size	= buffer size in bytes
 * Return     :		null-terminated string
 *			(without the terminating CR/LF)
 *
 ************************************************************************/

char           *
getstring (char *line, int size)
{
    int             len = 0;
    char            ch;

    do {
	fflush (stdout);
	ch = fgetc (stdin);
	switch (ch) {
	case 0x0a:
	case 0x0d:
	    line[len] = '\0';
	    return line;
	case 0x08:
	    if (len > 0) {
		len--;
		fputs ("\b \b", stdout);
	    }
	    break;
	case 0x7f:
	    while (len > 0) {
		len--;
		fputs ("\b \b", stdout);
	    }
	    break;
	default:
	    if (ch >= ' ' && ch < 0x7f && len < size - 1) {
		line[len++] = ch;
		fputc (ch, stdout);
	    }
	    break;
	}
    }
    while (len < size - 1);
    line[len] = '\0';
    return line;
}

/*************************************************************************
 * CALCULATE A NUMERIC DIGITS VALUE
 *
 * - If the character is not a valid digit
 *   or the value exceeds the radix, -1 is returned.
 *
 * Return	:	digit value
 *
 ************************************************************************/

static int
ch_val (char ch, int radix)
{
    int             val = -1;

    if (ch >= '0' && ch <= '9')
	val = ch - '0';
    elif (ch >= 'a' && ch <= 'f') val = ch - 'a' + 10;
    elif (ch >= 'A' && ch <= 'F') val = ch - 'A' + 10;

    return (val < radix ? val : -1);
}

/*************************************************************************
 * CONVERT A STRING TO AN INTEGRAL VALUE
 *
 * - ascint expects the line to consist of the following:
 *   1. Leading white space (optional).
 *   2. A plus or minus sign (optional).
 *   3. An octal '0' or hexadecimal '0x' or '0X' prefix (optional).
 *   4. A sequence of digits within the range of the appropiate base.
 *   5. One or more unrecognised characters
 *	(including the terminating null char).
 *   If the result would overflow the conversion stops.
 *
 * Parameter  :	def	= default value (also returned in case of error)
 * Return     :		converted value
 *
 ************************************************************************/

int
ascint (char *nptr, int def)
{
    bool            ok = FALSE;
    int             nflag = 0;
    int             base = 0;
    int             dlow = 0;
    int             dhigh = 0;
    int             dvalue;
    int             digit;
    char            c;

/* skip white space		 */
    while ((c = *nptr++) != 0 && (c == ' ' || c == '\t'));

    if (c == '-') {			/* check for sign		 */
	nflag++;			/* negative: set flag, skip	 */
	c = *nptr++;
    }
    elif (c == '+')
      c = *nptr++;			/* positive: skip		 */

    if (c == '0') {			/* get base value		 */
	c = *nptr++;			/* leading '0': octal or hex	 */
	if (c == 'x' || c == 'X') {
	    base = 16;			/* is hex...			 */
	    c = *nptr++;
	} else {
	    base = 8;			/* is octal...			 */
	    nptr -= 2;			/* reuse digit			 */
	    c = '0';
	}
    } else
	base = 10;			/* default is decimal.		 */

    while ((digit = ch_val (c, base)) >= 0) {	/* now collect value.	 */
	dlow = base * dlow + digit;	/* low half		 */
	dhigh = base * dhigh + (dlow >> 16);	/* high half		 */
	dlow &= 0xffff;
	if (dhigh >= 0x8000) {		/* check for overflow	 */
	    ok = FALSE;
	    break;
	}
	ok = TRUE;
	dvalue = (dhigh << 16) | dlow;	/* get final value	 */
	c = *nptr++;
    }
    if (ok)
	return (nflag ? -dvalue : dvalue);	/* return signed value.	 */
    else
	return def;
}

int
readint (int def, int max)
{
    char            buf[20];
    int             val;

    unless (getstring (buf, 20))
      return def;
    val = ascint (buf, def);
    if (val < 0 || val > max)
	return def;
    return val;
}

void
ShowRequest (RCB * rcb)
{
/* OI 06 March 1992:   Status code extraction added */

#define StatusMask          (0x1E)
#define StatusShiftRight    (1)

    word    StatusCode  = (rcb->Status & StatusMask) >> StatusShiftRight;
    
    printf ("\f\n\nCurrent Request parameters: \n\n");
    printf ("1)\tTarget ID   : %d\n", GetID (rcb->DriveID));
    printf ("2)\tTarget LUN  : %d\n", GetLUN (rcb->DriveID));
    printf ("3)\tSector size : %d\n", rcb->BlkSize);
    printf ("4)\tRead        : %d\n", rcb->Read);
    printf ("5)\tBlockmove   : %d\n", rcb->Block);
    printf ("6)\tTimeout     : %d sec\n", rcb->Req->DevReq.Timeout / OneSec);
    printf ("7)\tCDB size    : %d\n", rcb->Regs[0]);
    printf ("8)\tCDB         : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
	    rcb->Regs[3], rcb->Regs[4], rcb->Regs[5], rcb->Regs[6],
	    rcb->Regs[7], rcb->Regs[8], rcb->Regs[9], rcb->Regs[10],
	    rcb->Regs[11], rcb->Regs[12], rcb->Regs[13], rcb->Regs[14]);
    printf ("9)\tSize        : %d\n", rcb->Size);
    printf ("  \tRest        : %d\n", rcb->sdp.Rest);
    printf ("  \tStatus      : 0x%x", rcb->Status);
    if ((rcb->Status & StatusMask) == rcb->Status)      /* It's a SCSI status */
    {
        printf ("  (Code: 0x%x)", StatusCode);
    }
    printf ("\nA)\tData        : %02x %02x %02x %02x ...\n\n",
	    rcb->Data[0], rcb->Data[1], rcb->Data[2], rcb->Data[3]);
}

int
EditRequest (RCB * req)
{
    int             ch;

    forever
    {
	ShowRequest (req);
	printf ("1..A: edit field  R: send request Q: quit ");
	fflush (stdout);
	ch = fgetc (stdin);
	switch (ch) {
	case 'q':
	case 'Q':
	    return C_Done;
	case 'r':
	case 'R':
	    return C_Nop;
	case '1':
	    printf ("\n\nNew Target ID : ");
	    fflush (stdout);
	    SetID (req->DriveID, readint (GetID (req->DriveID), 6));
	    break;
	case '2':
	    printf ("\n\nNew target LUN : ");
	    fflush (stdout);
	    SetLUN (req->DriveID, readint (GetLUN (req->DriveID), 6));
	    break;
	case '3':
	    printf ("\n\nNew sector size : ");
	    fflush (stdout);
	    req->BlkSize = readint (req->BlkSize, 4096);
	    break;
	case '4':
	    printf ("\n\nRead (1) or Write (0) : ");
	    fflush (stdout);
	    req->Read = readint (req->Read, 1);
	    break;
	case '5':
	    printf ("\n\nBlockmove : ");
	    fflush (stdout);
	    req->Block = readint (req->Block, 1);
	    break;
	case '6':
	    printf ("\n\nTimeout : ");
	    fflush (stdout);
	    req->Req->DevReq.Timeout = readint (-1, 2140) * OneSec;
	    break;
	case '7':
	    printf ("\n\nNew CDB size : ");
	    fflush (stdout);
	    req->Regs[0] = readint (req->Regs[0], 12);
	    break;
	case '8':
	    EditData (12, (byte *) & req->Regs[3], "New CDB");
	    break;
	case '9':
	    printf ("\n\nNew Transfer size : ");
	    fflush (stdout);
	    req->Size = readint (req->Size, 0x100000);
	    break;
	case 'a':
	case 'A':
	    EditData (0x100000, req->Data, "New Data");
	    break;
	}
    }
}

DCB            *DevOpen (char *, DiscDevInfo *);
void            DevClose (DCB *);
void            DevOperate (DCB *, DiscReq *);


#define	INPUTS		1

DiscDCB        *ddcb;
DiscReq         dreq;
RCB             request;
byte            Data[0x100000];


int
main (int argc, char **argv)
{
    char           *fname = argv [0];
    DiscDevInfo    *ddinfo = NULL;	/* disc device info		 */
    void           *devinfo;		/* devinfo pointer		 */
    InfoNode       *info;		/* devinfo node pointer		 */
    char           *driver;		/* name of device driver	 */
    word            to;
    word            ok;

    if (argc != 2) {
	printf ("Usage: remote <MSCboard> %s <DiscDevice>\n", fname);
	return 1;
    }
/* get devinfo and driver name	 */
    if ((devinfo = load_devinfo ()) == NULL) {
	printf ("%s: cannot find DevInfo !\n", fname);
	return 1;			/* I cannot proceed without..	 */
    }
    if ((info = find_info (devinfo, Info_DiscDev, argv[1])) == NULL) {
	printf ("%s: cannot find %s entry in DevInfo !\n", fname, argv[1]);
	return 1;			/* I cannot proceed without..	 */
    }
    ddinfo = (DiscDevInfo *) RTOA (info->Info);
    driver = RTOA (ddinfo->Name);
IOdebug ("Open driver \"%s\"...", driver);    
    ddcb = (DiscDCB *) DevOpen (driver, ddinfo);
/* Load and initialise the	 */
/* Device driver.		 */
    if (ddcb == NULL) {			/* I cannot proceed without...	 */
	printf ("%s: failed to open device driver %s !\n", fname, driver);
	return 1;
    }
    InitScreen ();

    memset (&request, 0, sizeof (RCB));

    forever
    {
	request.Data = Data;
	request.Req = &dreq;
	if (EditRequest (&request) == C_Done)
	    break;

	printf ("\n\nsending request...");
	fflush (stdout);
	to = request.Req->DevReq.Timeout;
	if (to < 0)
	    request.EndTime = -1;
	else
	    request.EndTime = sum_ (_ldtimer (0), to);
	WriteWord (ddcb->Channels[0], (word) & request);
	ReadWord (ddcb->Channels[0], ok);
    }

    DevClose ((DCB *) ddcb);
    TidyScreen ();
}
