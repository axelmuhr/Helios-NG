/*
 * $Header: /Chris/00/helios/msc/RCS/testinfo.c,v 2.0 91/08/21 18:06:51 chris
 * Exp Locker: chris $
 */

#include <helios.h>
#include <gsp.h>
#include <codes.h>
#include <asm.h>
#include <device.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <attrib.h>
#include <nonansi.h>
#include <fault.h>
#include "mscstruc.h"
#include "rdevinfo.h"

#ifdef	RANDOM
#define	Random(x)	sum_ ( 1, prod_ ((x), 1664525 ))
#else
#define	Random(x)	(x + 1)
#endif

extern uword    _ldtimer (int);
DCB            *DevOpen (char *, DiscDevInfo *);
void            DevClose (DCB *);
void            DevOperate (DCB *, DiscReq *);

Object         *WM;			/* Window Manager		 */
Semaphore       WMLock;			/* Lock for WM Object		 */
int             Seed;

typedef struct VolInfo {		/* All Data for a single Volume	 */
    Object         *Window;		/* Volume's window		 */
    FILE           *Input;		/* Input from window		 */
    FILE           *Output;		/* Output to window		 */

    int             MaxSize;		/* Request limits		 */
    int             MaxPos;

    int             RSeed;		/* Read randomisation seed	 */
    int             WSeed;		/* Write randomisation seed	 */
    List            Requests;		/* pending request list		 */
    int             ReqCount;		/* Number of chained Requests	 */
    Semaphore       ReqLock;		/* Semaphore for pending Req's	 */
}

                VolInfo;

typedef struct TestReq {		/* Extended Request structure	 */
    DiscReq         DReq;		/* Disc Request struct		 */
    word           *Buf;		/* Data buffer			 */
    int             BufSize;		/* Data buffer size		 */
    int             Seed;		/* Randomisation Seed		 */
    VolInfo        *Info;		/* Related Volume		 */
}

                TestReq;

typedef struct TestNode {
    Node            Node;		/* Chaining Node		 */
    TestReq        *TReq;		/* Request ptr			 */
}

                TestNode;

int
InitScreen (VolInfo * info, char *volname)
{
    char           *fname = "InitScreen";
    Attributes      Attr;		/* Window attributes		 */
    int             e;

    Wait (&WMLock);
    info->Window = Create (WM, volname, Type_File, 0, NULL);
    e = WM->Result2;
    Signal (&WMLock);
    if (info->Window == NULL) {
	IOdebug ("%s %s: Failed to create window (%x).", fname, volname, e);
	return e;
    }
    info->Output = fopen (info->Window->Name, "w");
    if (info->Output == NULL) {
	e = info->Window->Result2;
	IOdebug ("%s %s: Failed to open output stream (%x).", fname, volname, e);
	Delete (info->Window, NULL);
	return e;
    }
    info->Input = fopen (info->Window->Name, "r");
    if (info->Input == NULL) {
	e = info->Window->Result2;
	IOdebug ("%s %s: Failed to open input stream (%x).", fname, volname, e);
	fclose (info->Output);
	Delete (info->Window, NULL);
	return e;
    }
    GetAttributes (Heliosno (info->Input), &Attr);
    RemoveAttribute (&Attr, ConsoleEcho);
    RemoveAttribute (&Attr, ConsolePause);
    RemoveAttribute (&Attr, ConsoleIgnoreBreak);
    RemoveAttribute (&Attr, ConsoleBreakInterrupt);
    RemoveAttribute (&Attr, ConsoleRawOutput);
    AddAttribute (&Attr, ConsoleRawInput);
    SetAttributes (Heliosno (info->Input), &Attr);
    setvbuf (info->Input, NULL, _IONBF, 0);
}

void
TidyScreen (VolInfo * info)
{
    fclose (info->Input);
    fclose (info->Output);
    Delete (info->Window, NULL);
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
fgetstring (char *line, int size, FILE * fi, FILE * fo)
{
    int             len = 0;
    char            ch;

    do {
	fflush (fo);
	ch = fgetc (fi);
	switch (ch) {
	case 0x0a:
	case 0x0d:
	    line[len] = '\0';
	    return line;
	case 0x08:
	    if (len > 0) {
		len--;
		fputs ("\b \b", fo);
	    }
	    break;
	case 0x7f:
	    while (len > 0) {
		len--;
		fputs ("\b \b", fo);
	    }
	    break;
	default:
	    if (ch >= ' ' && ch < 0x7f && len < size - 1) {
		line[len++] = ch;
		fputc (ch, fo);
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
fgetint (int def, int max, FILE * fi, FILE * fo)
{
    char            buf[20];
    int             val;

    unless (fgetstring (buf, 20, fi, fo))
      return def;
    val = ascint (buf, def);
    if (val > max)
	return def;
    return val;
}

int
SetBuffer (word * buf, int size, int seed)
{
    size >>= 2;
    while (size-- > 0) {
	*buf++ = seed;
	seed = Random (seed);
    }
    return seed;
}

int
CheckBuffer (word * buf, int size, int seed, FILE * fo, int *errsum)
{
    word            err = 0;
    word            pos;

    size >>= 2;
    for (pos = 0; pos < size; pos++, buf++, seed = Random (seed)) {
	if (*buf != seed) {
	    unless (err)
	      fputc ('\n', fo);
	    if (err++ < 10)
		fprintf (fo, "error @ 0x%06x : found 0x%08x, expected 0x%08x\n",
			 pos, *buf, seed);
	    seed = *buf;
	}
    }
    if (err) {
	fprintf (fo, "%d error%s found.\n", err, err != 1 ? "s" : "");
	*errsum += 1;
    }
    return seed;
}

void
VolSignal (TestReq * req)
{
    Signal (&req->Info->ReqLock);
}

DCB            *ddcb;			/* Driver's DCB			 */
word            Unit;			/* Driver's adressing unit	 */
word            start, end;		/* Time stamps			 */

void
VolExec (TestReq * treq)
{
    DevReq         *dreq = &treq->DReq.DevReq;

    dreq->Action = VolSignal;
    DevOperate (ddcb, &treq->DReq);
}

TestNode       *
NewTNode (int size)
{
    TestNode       *tnode;
    char           *tptr;

    tptr = Malloc (sizeof (TestNode) + sizeof (TestReq) + size);
    if (tptr == NULL)
	return NULL;
    tnode = (TestNode *) tptr;
    tptr += sizeof (TestNode);
    tnode->TReq = (TestReq *) tptr;
    tptr += sizeof (TestReq);
    tnode->TReq->Buf = (word *) tptr;
    tnode->TReq->BufSize = size;
    return tnode;
}

word
FreeTNode (Node * node, bool remove)
{
    if (remove)
	node = Remove (node);
    free (node);
    return 0;
}

int
TestKey (FILE * fp)
{
    int             buff[1];
    int             read;

    read = Read (Heliosno (fp), (byte *) buff, 1, OneSec / 1000);
    if (read > 0)
	return buff[0];
    return read;
}

void
TestTape (VolInfo * info, int volnum, char *volname, TestNode * tnode)
{
    char            msg[128];
    FILE           *fi = info->Input;
    FILE           *fo = info->Output;
    TestReq        *treq = tnode->TReq;
    DiscReq        *dreq = &treq->DReq;
    uword           tmax = 0;		/* maximum tape size		 */
    uword           rcmax = 0;		/* maximum read count		 */
    uword           wcmax = 0;		/* maximum write count		 */
    int             repos;
    uword           t1, t2, tt1, tt2, td, tsum;
    int             rseed, wseed;
    uword           rcount, wcount;
    int             count;
    int             avg;
    int             nloop = 1;		/* number of complete passes	 */
    int             errsum = 0;		/* data error count		 */
    uword           tsize;		/* tape size ( Units )		 */
    int             rsize = 0x10000 / Unit;	/* 64 KByte / Request	 */
    int             fsize = 4;		/* 4 MByte / File	 */
    int             wsize = 8;		/* 8 Files together	 */
    int             i, j, fmk;

/*
 * Test of a Tape: forever { Rewind while not EOT { Write 16 * ( 4 MByte + 1
 * Filemark ) Write 1 Filemark Space Reverse 17 Filemarks Space Reverse 1
 * MByte Read 16 MByte with Check, hereby skipping 15 Filemarks Space forward
 * 2 Filemarks } print tape capacity }
 */

/*
 * Seed = rseed = wseed = sum_ ( 1, prod_ ( Seed, 1664525 ));
 */
    fprintf (fo, "File Size ( %d MByte ) : ", fsize);
    fflush (fo);
    if ((fsize = fgetint (fsize, MaxInt, fi, fo)) <= 0)
	fsize = 4;
    fsize *= rsize * 16;

    fprintf (fo, "\nFiles per Loop ( %d ) : ", wsize);
    fflush (fo);
    if ((wsize = fgetint (wsize, MaxInt, fi, fo)) <= 0)
	wsize = 8;
    wsize *= fsize;

    fprintf (fo, "\n1: Space reverse, test whole tape  0: Rewind, test from start : ");
    fflush (fo);
    if ((repos = fgetint (0, 1, fi, fo)) < 0)
	repos = 0;

    fputc ('\n', fo);

    rseed = wseed = 0;
    treq->Info = info;
    dreq->Buf = treq->Buf;
    dreq->DevReq.SubDevice = volnum;
    dreq->DevReq.Action = VolSignal;
    forever
    {
	fprintf (fo, "Test Tape %s: %s with %d file%s of %d MByte, Loop %d:\n",
		 volname, repos ? "Complete" : "From start", wsize / fsize,
	    (wsize / fsize) != 1 ? "s" : "", fsize / (rsize * 16), nloop++);
	tsize = 0;
	rseed = wseed;
	rcount = wcount = 0;
	dreq->DevReq.Request = FG_Seek;
	dreq->DevReq.Timeout = -1;
	dreq->Pos = 0;
	dreq->Size = 0;
	fprintf (fo, "Rewind...\t");
	fflush (fo);
	t1 = _ldtimer (0);
	DevOperate (ddcb, dreq);
	Wait (&info->ReqLock);
	t2 = _ldtimer (0);
	if (dreq->DevReq.Result) {
	    Fault (dreq->DevReq.Result, msg, 128);
	    fprintf (fo, "\tResult: 0x%08x\x09bK\n+++\t%s\x09bK\n",
		     dreq->DevReq.Result, msg);
	    dreq->DevReq.Request = FG_GetInfo;
	    dreq->Size = 128;
	    DevOperate (ddcb, dreq);
	    Wait (&info->ReqLock);
	    fprintf (fo, "Sense : ");
	    for (i = 0; i < dreq->Actual; i++)
	    {
	    	if (0 == (i % 4))
	    	    fprintf (fo, "\n");
		fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
	    }
	    fputc ('\n', fo);
	    goto stat;
	}
	fprintf (fo, "\t%5d ms\x09bK\r", (t2 - t1 + 500) / 1000);
	if (TestKey (fi))
	    goto done;

write:
	fmk = 0;
	tsum = 0;
	count = 0;
	avg = 0;
	fputc ('\n', fo);
	tt1 = _ldtimer (0);
	for (j = 0; j < wsize / rsize;) {
	    dreq->DevReq.Request = FG_Write;
	    dreq->DevReq.Timeout = 5 * OneSec;
	    dreq->Pos = 0;
	    dreq->Size = rsize;
	    wseed = SetBuffer (dreq->Buf, Unit * rsize, wseed);
	    fprintf (fo, "Write %5d...\t", wcount++);
	    fflush (fo);
	    t1 = _ldtimer (0);
	    DevOperate (ddcb, dreq);
	    Wait (&info->ReqLock);
	    t2 = _ldtimer (0);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "\tResult: 0x%08x\x09bK\n+++\t%s\x09bK\n",
			 dreq->DevReq.Result, msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info->ReqLock);
		fprintf (fo, "Sense : ");
                for (i = 0; i < dreq->Actual; i++)
	        {
	    	    if (0 == (i % 4))
	    	        fprintf (fo, "\n");
		    fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
	        }
		fputc ('\n', fo);
		wcount--;
		goto seek;
	    }
	    td = (t2 - t1 + 500) / 1000;
	    avg = ((avg * count) + (65536000 / td)) / (count + 1);
	    count++;
	    fprintf (fo, "\t%5d ms, %4d KByte/sec, bus avg %4d KByte/sec\r",
		     td, 64000 / td, avg / 1000);
	    tsum += td;
	    tsize += rsize;
	    if (TestKey (fi))
		goto done;
	    j++;
	    if ((tsize % fsize) == 0) {
		tt2 = _ldtimer (0);
		count = 0;
		avg = 0;
		dreq->DevReq.Request = FG_WriteMark;
		dreq->DevReq.Timeout = 60 * OneSec;
		dreq->Pos = 0;
		dreq->Size = 1;
		fprintf (fo, "Write Filemark...");
		fflush (fo);
		t1 = _ldtimer (0);
		DevOperate (ddcb, dreq);
		Wait (&info->ReqLock);
		t2 = _ldtimer (0);
		if (dreq->DevReq.Result) {
		    Fault (dreq->DevReq.Result, msg, 128);
		    fprintf (fo, "\tResult: 0x%08x\x09bK\n+++\t%s\x09bK\n",
			     dreq->DevReq.Result, msg);
		    dreq->DevReq.Request = FG_GetInfo;
		    dreq->Size = 128;
		    DevOperate (ddcb, dreq);
		    Wait (&info->ReqLock);
		    fprintf (fo, "Sense : ");
                    for (i = 0; i < dreq->Actual; i++)
                    {
	    	        if (0 == (i % 4))
	    	            fprintf (fo, "\n");
		        fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
	            }
		    fputc ('\n', fo);
		    goto seek;;
		} else
		    fprintf (fo, "\t%5d ms, file avg %d KByte/sec\x09bK\n",
			     (t2 - t1 + 500) / 1000,
			 fsize / 1024 * Unit * 1000 / ((tt2 - tt1) / 1000));
		if (TestKey (fi))
		    goto done;
		fmk++;
		tt1 = _ldtimer (0);
	    }
	}
	dreq->DevReq.Request = FG_WriteMark;
	dreq->DevReq.Timeout = 60 * OneSec;
	dreq->Pos = 0;
	dreq->Size = 1;
	fprintf (fo, "Write Filemark...");
	fflush (fo);
	t1 = _ldtimer (0);
	DevOperate (ddcb, dreq);
	Wait (&info->ReqLock);
	t2 = _ldtimer (0);
	if (dreq->DevReq.Result) {
	    Fault (dreq->DevReq.Result, msg, 128);
	    fprintf (fo, "\tResult: 0x%08x\x09bK\n+++\t%s\x09bK\n",
		     dreq->DevReq.Result, msg);
	    dreq->DevReq.Request = FG_GetInfo;
	    dreq->Size = 128;
	    DevOperate (ddcb, dreq);
	    Wait (&info->ReqLock);
	    fprintf (fo, "Sense : ");
            for (i = 0; i < dreq->Actual; i++)
            {
    	        if (0 == (i % 4))
    	            fprintf (fo, "\n");
	        fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
            }
	    fputc ('\n', fo);
	} else {
	    fprintf (fo, "\t%5d ms\x09bK\r", (t2 - t1 + 500) / 1000);
	    fmk++;
	}
	if (TestKey (fi))
	    goto done;

seek:
	if (repos) {
	    dreq->DevReq.Request = FG_Seek;
	    dreq->DevReq.Timeout = -1;
	    dreq->Pos = 1;
	    dreq->Size = -fmk;
	    fprintf (fo, "\nSeek %d Filemarks...", dreq->Size);
	    fflush (fo);
	    t1 = _ldtimer (0);
	    DevOperate (ddcb, dreq);
	    Wait (&info->ReqLock);
	    t2 = _ldtimer (0);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "\tResult: 0x%08x\x09bK\n+++\t%s\x09bK\n",
			 dreq->DevReq.Result, msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info->ReqLock);
		fprintf (fo, "Sense : ");
                for (i = 0; i < dreq->Actual; i++)
                {
    	            if (0 == (i % 4))
    	                fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
		goto stat;
	    }
	    fprintf (fo, "\t%5d ms\x09bK\r", (t2 - t1 + 500) / 1000);
	    if (TestKey (fi))
		goto done;

	    dreq->DevReq.Request = FG_Seek;
	    dreq->DevReq.Timeout = -1;
	    dreq->Pos = 0;
	    if ((dreq->Size = j * rsize) > fsize)
		dreq->Size = fsize;
	    dreq->Size = -dreq->Size;
	    fprintf (fo, "Seek %d blocks...", dreq->Size);
	    fflush (fo);
	    t1 = _ldtimer (0);
	    DevOperate (ddcb, dreq);
	    Wait (&info->ReqLock);
	    t2 = _ldtimer (0);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "\tResult: 0x%08x\x09bK\n+++\t%s\x09bK\n",
			 dreq->DevReq.Result, msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info->ReqLock);
		fprintf (fo, "Sense : ");
                for (i = 0; i < dreq->Actual; i++)
                {
    	            if (0 == (i % 4))
    	                fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
		if ((dreq->DevReq.Result & EC_Mask) > EC_Warn)
		    goto stat;
	    } else
		fprintf (fo, "\t%5d ms\x09bK\r", (t2 - t1 + 500) / 1000);
	} else {
	    dreq->DevReq.Request = FG_Seek;
	    dreq->DevReq.Timeout = -1;
	    dreq->Pos = 0;
	    dreq->Size = 0;
	    fprintf (fo, "\nRewind...\t");
	    fflush (fo);
	    t1 = _ldtimer (0);
	    DevOperate (ddcb, dreq);
	    Wait (&info->ReqLock);
	    t2 = _ldtimer (0);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "\tResult: 0x%08x\x09bK\n+++\t%s\x09bK\n",
			 dreq->DevReq.Result, msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info->ReqLock);
		fprintf (fo, "Sense : ");
                for (i = 0; i < dreq->Actual; i++)
                {
    	            if (0 == (i % 4))
    	                fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
		goto stat;
	    }
	    fprintf (fo, "\t%5d ms\x09bK\r", (t2 - t1 + 500) / 1000);
	}
	if (TestKey (fi))
	    goto done;

	tsum = 0;
	count = 0;
	avg = 0;
	fputc ('\n', fo);
	tt1 = _ldtimer (0);
	for (i = 0; i < j;) {
    read:
	    dreq->DevReq.Request = FG_Read;
	    dreq->DevReq.Timeout = 60 * OneSec;
	    dreq->Pos = 0;
	    dreq->Size = rsize;
	    fprintf (fo, "Read %5d...\t", rcount++);
	    fflush (fo);
	    t1 = _ldtimer (0);
	    DevOperate (ddcb, dreq);
	    Wait (&info->ReqLock);
	    t2 = _ldtimer (0);
	    if (dreq->DevReq.Result) {
		rcount--;
		if ((dreq->DevReq.Result & (EG_Mask | EO_Mask)) == (EG_Congested + EO_File)) {

		    fprintf (fo, "\rFilemark found.\t\t%5d ms, file avg %d KByte/sec\x09bK\n",
			     (t2 - t1 + 500) / 1000,
			  fsize / 1024 * Unit * 1000 / ((t2 - tt1) / 1000));
		    tt1 = _ldtimer (0);
		    goto read;
		}
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "\tResult: 0x%08x\x09bK\n+++\t%s\x09bK\n",
			 dreq->DevReq.Result, msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info->ReqLock);
		fprintf (fo, "Sense : ");
                for (i = 0; i < dreq->Actual; i++)
                {
    	            if (0 == (i % 4))
    	                fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
		count = 0;
		avg = 0;
		break;
	    }
	    td = (t2 - t1 + 500) / 1000;
	    avg = ((avg * count) + (65536000 / td)) / (count + 1);
	    count++;
	    fprintf (fo, "\t%5d ms, %4d KByte/sec, bus avg %4d KByte/sec\r",
		     td, 64000 / td, avg / 1000);
	    tsum += td;
	    i++;
	    if (TestKey (fi))
		goto done;
	    rseed = CheckBuffer (dreq->Buf, 0x10000, rseed, fo, &errsum);
	}

	if (i < j || i < (wsize / rsize)) {
	    fprintf (fo, "\nWrite count : %d, Read Count %d, Tape Size %d units of %d bytes\n",
		     wcount, rcount, tsize, Unit);
	    fprintf (fo, "Number of Requests with data errors : %d\n", errsum);
	    if (tsize > tmax)
		tmax = tsize;
	    if (wcount > wcmax)
		wcmax = wcount;
	    if (rcount > rcmax)
		rcmax = rcount;
	    continue;
	}
	if (repos) {
	    dreq->DevReq.Request = FG_Seek;
	    dreq->DevReq.Timeout = -1;
	    dreq->Pos = 1;
	    dreq->Size = 2;
	    fprintf (fo, "\nSeek 2 Filemarks...");
	    fflush (fo);
	    t1 = _ldtimer (0);
	    DevOperate (ddcb, dreq);
	    Wait (&info->ReqLock);
	    t2 = _ldtimer (0);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "\tResult: 0x%08x\x09bK\n+++\t%s\x09bK\n",
			 dreq->DevReq.Result, msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info->ReqLock);
		fprintf (fo, "Sense : ");
                for (i = 0; i < dreq->Actual; i++)
                {
    	            if (0 == (i % 4))
    	                fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
		goto stat;
	    }
	    fprintf (fo, "\t%5d ms\x09bK\r", (t2 - t1 + 500) / 1000);
	    if (TestKey (fi))
		goto done;
	    goto write;
	}
    }
done:
    fprintf (fo, "\nTest stopped.\n");
    dreq->DevReq.Request = FG_Seek;
    dreq->DevReq.Timeout = -1;
    dreq->Pos = 0;
    dreq->Size = 0;
    fprintf (fo, "Rewind...\t");
    fflush (fo);
    t1 = _ldtimer (0);
    DevOperate (ddcb, dreq);
    Wait (&info->ReqLock);
    t2 = _ldtimer (0);
    if (dreq->DevReq.Result) {
	Fault (dreq->DevReq.Result, msg, 128);
	fprintf (fo, "\tResult: 0x%08x\x09bK\n+++\t%s\x09bK\n",
		 dreq->DevReq.Result, msg);
	dreq->DevReq.Request = FG_GetInfo;
	dreq->Size = 128;
	DevOperate (ddcb, dreq);
	Wait (&info->ReqLock);
	fprintf (fo, "Sense : ");
        for (i = 0; i < dreq->Actual; i++)
        {
            if (0 == (i % 4))
    	        fprintf (fo, "\n");
	    fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
        }
	fputc ('\n', fo);
    } else
	fprintf (fo, "\t%5d ms\x09bK\r", (t2 - t1 + 500) / 1000);
stat:
    if (tsize > tmax)
	tmax = tsize;
    if (wcount > wcmax)
	wcmax = wcount;
    if (rcount > rcmax)
	rcmax = rcount;
    fprintf (fo, "\nMaximum values after %d Loop%s:",
	     nloop - 1, nloop - 2 ? "s" : "");
    fprintf (fo, "\nWrite count : %d, Read Count %d, Tape Size %d units of %d bytes\n",
	     wcmax, rcmax, tmax, Unit);
    fprintf (fo, "Number of Requests with data errors : %d\n", errsum);
}

void
HandleTape (int volnum, char *volname, bool * opened)
{
    char            msg[128];
    char           *fname = "HandleTape";
    VolInfo         info;
    FILE           *fi;
    FILE           *fo;
    TestNode       *tnode;
    TestReq        *treq;
    DiscReq        *dreq;
    int             errsum = 0;
    uword           t1, t2;
    int             ch, i;

    InitScreen (&info, volname);
    fi = info.Input;
    fo = info.Output;

    info.MaxSize = 0x10000 / Unit;
    InitList (&info.Requests);
    InitSemaphore (&info.ReqLock, 0);
    info.WSeed = info.RSeed = 0;
/* Get default req & 64k buffer	 */
    while ((tnode = NewTNode (0x10000)) == NULL) {
	fprintf (fo, "\n%s #%d (%s): Failed to allocate default tnode.",
		 fname, volnum, volname);
	Delay (OneSec);
    }
    treq = tnode->TReq;
    treq->Info = &info;
    dreq = &treq->DReq;
    dreq->Buf = treq->Buf;
    dreq->DevReq.SubDevice = volnum;
    forever
    {
	fprintf (fo, "\n%s #%d (%s) :\n", fname, volnum, volname);
	fputs ("(G)etSize  (R)ead  (W)rite  (S)eek  (E)rase  (F)ilemark  (T)est  (C)lose : ", fo);
	fflush (fo);
	ch = fgetc (fi);
	fprintf (fo, "%c\n", ch);
	switch (ch) {
	case 't':
	case 'T':
	    TestTape (&info, volnum, volname, tnode);
	    continue;

	case 'g':
	case 'G':
	    dreq->DevReq.Request = FG_GetSize;
	    dreq->DevReq.Timeout = -1;
	    fputs ("executing GetSize request ...\n", fo);
	    VolExec (treq);
	    Wait (&info.ReqLock);
	    fprintf (fo, "Device has %d blocks of %d bytes.\n",
		     dreq->DevReq.Result, dreq->Actual);
	    continue;
	case 'r':
	case 'R':
	    dreq->DevReq.Request = FG_Read;
	    dreq->DevReq.Timeout = 60 * OneSec;
	    fprintf (fo, "Read for Size ( 1 .. %d units ) : ", info.MaxSize);
	    if ((dreq->Size = fgetint (-1, info.MaxSize, fi, fo)) <= 0) {
		fputc ('\n', fo);
		continue;
	    }
	    fputs ("\nexecuting Read request ...\n", fo);
	    t1 = _ldtimer (0);
	    VolExec (treq);
	    Wait (&info.ReqLock);
	    t2 = _ldtimer (0);
	    fprintf (fo, "time: %d ms, Actual: %d, Result 0x%x\n",
		     (t2 - t1) / 1000, dreq->Actual, dreq->DevReq.Result);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "+++\t%s\x09bK\n", msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info.ReqLock);
		fprintf (fo, "Sense : ");
                for (i = 0; i < dreq->Actual; i++)
                {
                    if (0 == (i % 4))
    	                fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
		continue;
	    }
	    fprintf (fo, "Comparison seed (0x%x) : ", info.RSeed);
	    info.RSeed = fgetint (info.RSeed, MaxInt, fi, fo);
	    fputc ('\n', fo);
	    info.RSeed = CheckBuffer (treq->Buf, dreq->Actual * Unit, info.RSeed, fo, &errsum);
	    continue;
	case 'w':
	case 'W':
	    dreq->DevReq.Request = FG_Write;
	    dreq->DevReq.Timeout = 60 * OneSec;
	    fprintf (fo, "Write for Size ( 1 .. %d units ) : ", info.MaxSize);
	    if ((dreq->Size = fgetint (-1, info.MaxSize, fi, fo)) <= 0)
		continue;
	    fprintf (fo, "\nComparison seed (0x%x) : ", info.WSeed);
	    info.WSeed = fgetint (info.WSeed, MaxInt, fi, fo);
	    info.WSeed = SetBuffer (treq->Buf, dreq->Size * Unit, info.WSeed);
	    fputs ("\nexecuting Write request ...\n", fo);
	    t1 = _ldtimer (0);
	    VolExec (treq);
	    Wait (&info.ReqLock);
	    t2 = _ldtimer (0);
	    fprintf (fo, "time: %d ms, Actual: %d, Result 0x%x\n",
		     (t2 - t1) / 1000, dreq->Actual, dreq->DevReq.Result);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "+++\t%s\x09bK\n", msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info.ReqLock);
		fprintf (fo, "Sense : ");
                for (i = 0; i < dreq->Actual; i++)
                {
                    if (0 == (i % 4))
    	                fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
	    }
	    continue;
	case 's':
	case 'S':
	    dreq->DevReq.Request = FG_Seek;
	    dreq->DevReq.Timeout = -1;
	    fprintf (fo, "Seek for 0: Blocks  1: Filemarks  2: seq. Filemarks  3: End of recorded Area : ");
	    dreq->Pos = fgetint (0, 3, fi, fo);
	    fprintf (fo, "\nNumber of Blocks/Filemarks : ");
	    dreq->Size = fgetint (0, MaxInt, fi, fo);
	    fprintf (fo, "\nexecuting Seek request (%d,%d)...\n", dreq->Pos, dreq->Size);
	    t1 = _ldtimer (0);
	    VolExec (treq);
	    Wait (&info.ReqLock);
	    t2 = _ldtimer (0);
	    fprintf (fo, "time: %d ms, Actual: %d, Result 0x%x\n",
		     (t2 - t1) / 1000, dreq->Actual, dreq->DevReq.Result);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "+++\t%s\x09bK\n", msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info.ReqLock);
		fprintf (fo, "Sense : ");
                for (i = 0; i < dreq->Actual; i++)
                {
                    if (0 == (i % 4))
    	                fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
	    }
	    continue;
	case 'e':
	case 'E':
	    dreq->DevReq.Request = FG_Format;
	    dreq->DevReq.Timeout = -1;
	    fputs ("executing Erase request ...\n", fo);
	    t1 = _ldtimer (0);
	    VolExec (treq);
	    Wait (&info.ReqLock);
	    t2 = _ldtimer (0);
	    fprintf (fo, "time: %d ms, Actual: %d, Result 0x%x\n",
		     (t2 - t1) / 1000, dreq->Actual, dreq->DevReq.Result);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "+++\t%s\x09bK\n", msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info.ReqLock);
		fprintf (fo, "Sense : ");
                for (i = 0; i < dreq->Actual; i++)
                {
                    if (0 == (i % 4))
    	                fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
	    }
	    continue;
	case 'f':
	case 'F':
	    dreq->DevReq.Request = FG_WriteMark;
	    dreq->DevReq.Timeout = -1;
	    fprintf (fo, "Number of Filemarks to write : ");
	    dreq->Size = fgetint (0, MaxInt, fi, fo);
	    fputs ("\nexecuting WriteMark request ...\n", fo);
	    t1 = _ldtimer (0);
	    VolExec (treq);
	    Wait (&info.ReqLock);
	    t2 = _ldtimer (0);
	    fprintf (fo, "time: %d ms, Actual: %d, Result 0x%x\n",
		     (t2 - t1) / 1000, dreq->Actual, dreq->DevReq.Result);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "+++\t%s\x09bK\n", msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info.ReqLock);
		fprintf (fo, "Sense : ");
                for (i = 0; i < dreq->Actual; i++)
                {
                    if (0 == (i % 4))
    	                fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
	    }
	    continue;

	case 'c':
	case 'C':
	    dreq->DevReq.Request = FG_Close;
	    dreq->DevReq.Timeout = -1;
	    fputs ("executing Close request ...\n", fo);
	    t1 = _ldtimer (0);
	    VolExec (treq);
	    Wait (&info.ReqLock);
	    t2 = _ldtimer (0);
	    fprintf (fo, "time: %d ms, Actual: %d, Result 0x%x\n",
		     (t2 - t1) / 1000, dreq->Actual, dreq->DevReq.Result);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "+++\t%s\x09bK\n", msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info.ReqLock);
		fprintf (fo, "Sense : ");
                for (i = 0; i < dreq->Actual; i++)
                {
                    if (0 == (i % 4))
    	                fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
	    }
	    fputs ("press any key to close window...", fo);
	    fflush (fo);
	    ch = fgetc (fi);
	    Free (tnode);
	    TidyScreen (&info);
	    *opened = FALSE;
	    return;
	default:
	    continue;
	}
    }
}

#ifndef	RANDOM
#undef	Random
#define	Random(x)	sum_ ( 1, prod_ ((x), 1664525 ))
#endif

void
TestDisc (VolInfo * info, int volnum, char *volname)
{
    char            msg[128];
    FILE           *fi = info->Input;
    FILE           *fo = info->Output;
    TestNode       *tnode;
    TestReq        *treq;
    DiscReq        *dreq;
    uword           t1, t2, tsum;
    int             dummy;
    int             ch;
    int             nreq = 16;
    int             sreq = info->MaxSize;
    int             i;

    fprintf (fo, "Test Disc %s :\nNumber of requests ( %d ): ", volname, nreq);
    if ((nreq = fgetint (nreq, 32, fi, fo)) <= 0)
	nreq = 16;
    fprintf (fo, "\nRequest size ( 1 .. %d units ) : ", sreq);
    if ((sreq = fgetint (sreq, info->MaxSize, fi, fo)) < 0)
	sreq = info->MaxSize;

    Seed = dummy = Random (Seed);
    for (i = 0; i < nreq; i++) {
	if ((tnode = NewTNode (sreq * Unit)) == NULL)
	    break;

	treq = tnode->TReq;
	dreq = &treq->DReq;
	treq->Seed = dummy;
	treq->Info = info;
	dummy = SetBuffer (treq->Buf, sreq * Unit, dummy);
	dreq->Size = sreq;
	dreq->Buf = treq->Buf;
	dreq->Actual = 0;
	dreq->DevReq.SubDevice = volnum;
	dreq->DevReq.Action = VolSignal;
	dreq->DevReq.Timeout = -1;
	AddTail (&info->Requests, (Node *) tnode);
    }
    nreq = i;
    fprintf (fo, "\nAllocted %d Requests of %d units.\n", nreq, sreq);
    forever
    {
	fprintf (fo, "Test: (S)ame (C)ontiguous op(T)imised (R)andom (O)ptimised (E)nd : ");
	fflush (fo);
	ch = fgetc (fi);
	fprintf (fo, "%c\nPress any key to stop :\n", ch);
	fflush (fo);
loop:
	switch (ch) {
	default:
	    continue;

	case 'e':
	case 'E':
	    fputs ("End tests.\n", fo);
	    WalkList (&info->Requests, FreeTNode, TRUE);
	    return;

	case 's':
	case 'S':
	    fprintf (fo, "Same:\n");
	    i = ((Seed = Random (Seed)) & 0x7FFFFFFF)
	      % (info->MaxPos - sreq);
	    for (tnode = Head_ (TestNode, info->Requests);
		 !EndOfList_ (tnode); tnode = Next_ (TestNode, tnode))
		tnode->TReq->DReq.Pos = i;
	    break;

	case 'c':
	case 'C':
	    fprintf (fo, "Contiguous :\n");
	    i = ((Seed = Random (Seed)) & 0x7FFFFFFF)
	      % (info->MaxPos - sreq * nreq);
	    for (tnode = Head_ (TestNode, info->Requests);
		 !EndOfList_ (tnode); tnode = Next_ (TestNode, tnode)) {
		tnode->TReq->DReq.Pos = i;
		i += sreq;
	    }
	    break;

	case 't':
	case 'T':
	    fprintf (fo, "Contiguous Optimised :\n");
	    i = ((Seed = Random (Seed)) & 0x7FFFFFFF)
	      % (info->MaxPos - sreq * nreq);
	    for (tnode = Head_ (TestNode, info->Requests);
		 !EndOfList_ (tnode); tnode = Next_ (TestNode, tnode)) {
		tnode->TReq->DReq.Pos = i;
		i += sreq;
	    }
	    break;

	case 'r':
	case 'R':
	    fprintf (fo, "Random :\n");
	    for (tnode = Head_ (TestNode, info->Requests);
		 !EndOfList_ (tnode); tnode = Next_ (TestNode, tnode))
		tnode->TReq->DReq.Pos =
		  ((Seed = Random (Seed)) & 0x7FFFFFFF)
		  % (info->MaxPos - sreq);
	    break;

	case 'o':
	case 'O':
	    fprintf (fo, "Random Optimised :\n");
	    for (tnode = Head_ (TestNode, info->Requests);
		 !EndOfList_ (tnode); tnode = Next_ (TestNode, tnode))
		tnode->TReq->DReq.Pos =
		  ((Seed = Random (Seed)) & 0x7FFFFFFF)
		  % (info->MaxPos - sreq);
	    break;
	}

	for (tnode = Head_ (TestNode, info->Requests);
	     !EndOfList_ (tnode); tnode = Next_ (TestNode, tnode)) {

#ifdef	CHECKDATA
	    dummy = SetBuffer (treq->Buf, sreq * Unit, tnode->TReq->Seed);
#endif

	    tnode->TReq->DReq.DevReq.Request = FG_Write;
	    tnode->TReq->DReq.DevReq.Timeout = 5 * OneSec;
	}


	switch (ch) {
	default:
	    tsum = 0;
	    for (tnode = Head_ (TestNode, info->Requests);
		 !EndOfList_ (tnode); tnode = Next_ (TestNode, tnode)) {
		t1 = _ldtimer (0);
		DevOperate (ddcb, &tnode->TReq->DReq);
		Wait (&info->ReqLock);
		t2 = _ldtimer (0);
		tsum += (t2 - t1);
		dreq = &tnode->TReq->DReq;
		if (dreq->DevReq.Result != Err_Null
		    || dreq->Actual != dreq->Size) {
		    fprintf (fo, "\nSize: %d Actual: %d Result: 0x%x\n",
			     dreq->Size, dreq->Actual, dreq->DevReq.Result);
		    if (dreq->DevReq.Result) {
			Fault (dreq->DevReq.Result, msg, 128);
			fprintf (fo, "+++\t%s\n", msg);
			dreq->DevReq.Request = FG_GetInfo;
			dreq->Size = 128;
			DevOperate (ddcb, dreq);
			Wait (&info->ReqLock);
			fprintf (fo, "Sense : ");
                        for (i = 0; i < dreq->Actual; i++)
                        {
                            if (0 == (i % 4))
    	                        fprintf (fo, "\n");
	                    fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                        }
			fputc ('\n', fo);
		    }
		} else
		    fputc ('w', fo);
		fflush (fo);
	    }
	    break;
	case 'o':
	case 'O':
	case 't':
	case 'T':
	    tnode = Head_ (TestNode, info->Requests);
	    i = 0;
	    t1 = _ldtimer (0);
	    for (; !EndOfList_ (tnode); tnode = Next_ (TestNode, tnode), i++)
		DevOperate (ddcb, &tnode->TReq->DReq);
	    while (i-- > 0)
		Wait (&info->ReqLock);
	    t2 = _ldtimer (0);
	    tsum = t2 - t1;
	    for (tnode = Head_ (TestNode, info->Requests);
		 !EndOfList_ (tnode); tnode = Next_ (TestNode, tnode)) {
		dreq = &tnode->TReq->DReq;
		if (dreq->DevReq.Result != Err_Null
		    || dreq->Actual != dreq->Size) {
		    fprintf (fo, "\nSize: %d Actual: %d Result: 0x%x\n",
			     dreq->Size, dreq->Actual, dreq->DevReq.Result);
		    if (dreq->DevReq.Result) {
			Fault (dreq->DevReq.Result, msg, 128);
			fprintf (fo, "+++\t%s\n", msg);
			dreq->DevReq.Request = FG_GetInfo;
			dreq->Size = 128;
			DevOperate (ddcb, dreq);
			Wait (&info->ReqLock);
			fprintf (fo, "Sense : ");
                        for (i = 0; i < dreq->Actual; i++)
                        {
                            if (0 == (i % 4))
    	                        fprintf (fo, "\n");
	                    fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                        }
			fputc ('\n', fo);
		    }
		} else
		    fputc ('w', fo);
	    }
	    break;
	}

	fprintf (fo, "\n%d requests of %d KByte each written in %d ms ( %d KByte/sec )\n",
		 nreq, sreq * Unit / 1024, tsum / 1000, nreq * sreq * Unit / 1024 * OneSec / tsum);

	for (tnode = Head_ (TestNode, info->Requests);
	     !EndOfList_ (tnode); tnode = Next_ (TestNode, tnode)) {
	    tnode->TReq->DReq.DevReq.Request = FG_Read;
	    tnode->TReq->DReq.DevReq.Timeout = 5 * OneSec;
	}
	switch (ch) {
	default:
	    tsum = 0;
	    for (tnode = Head_ (TestNode, info->Requests);
		 !EndOfList_ (tnode); tnode = Next_ (TestNode, tnode)) {
		t1 = _ldtimer (0);
		DevOperate (ddcb, &tnode->TReq->DReq);
		Wait (&info->ReqLock);
		t2 = _ldtimer (0);
		tsum += (t2 - t1);
		dreq = &tnode->TReq->DReq;
		if (dreq->DevReq.Result != Err_Null
		    || dreq->Actual != dreq->Size) {
		    fprintf (fo, "\nSize: %d Actual: %d Result: 0x%x\n",
			     dreq->Size, dreq->Actual, dreq->DevReq.Result);
		    if (dreq->DevReq.Result) {
			Fault (dreq->DevReq.Result, msg, 128);
			fprintf (fo, "+++\t%s\n", msg);
			dreq->DevReq.Request = FG_GetInfo;
			dreq->Size = 128;
			DevOperate (ddcb, dreq);
			Wait (&info->ReqLock);
			fprintf (fo, "Sense : ");
                        for (i = 0; i < dreq->Actual; i++)
                        {
                            if (0 == (i % 4))
    	                        fprintf (fo, "\n");
	                    fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                        }
			fputc ('\n', fo);
		    }
		} else {
		    fputc ('r', fo);

#ifdef	CHECKDATA
		    dummy = CheckBuffer (treq->Buf, sreq * Unit, treq->Seed,
					 fo, &dummy);
#endif
		}
		fflush (fo);
	    }
	    break;
	case 'o':
	case 'O':
	case 't':
	case 'T':
	    tnode = Head_ (TestNode, info->Requests);
	    i = 0;
	    t1 = _ldtimer (0);
	    for (; !EndOfList_ (tnode); tnode = Next_ (TestNode, tnode), i++)
		DevOperate (ddcb, &tnode->TReq->DReq);
	    while (i-- > 0)
		Wait (&info->ReqLock);
	    t2 = _ldtimer (0);
	    tsum = t2 - t1;
	    for (tnode = Head_ (TestNode, info->Requests);
		 !EndOfList_ (tnode); tnode = Next_ (TestNode, tnode)) {
		dreq = &tnode->TReq->DReq;
		if (dreq->DevReq.Result != Err_Null
		    || dreq->Actual != dreq->Size) {
		    fprintf (fo, "\nSize: %d Actual: %d Result: 0x%x\n",
			     dreq->Size, dreq->Actual, dreq->DevReq.Result);
		    if (dreq->DevReq.Result) {
			Fault (dreq->DevReq.Result, msg, 128);
			fprintf (fo, "+++\t%s\n", msg);
			dreq->DevReq.Request = FG_GetInfo;
			dreq->Size = 128;
			DevOperate (ddcb, dreq);
			Wait (&info->ReqLock);
			fprintf (fo, "Sense : ");
                        for (i = 0; i < dreq->Actual; i++)
                        {
                            if (0 == (i % 4))
    	                        fprintf (fo, "\n");
	                    fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                        }
			fputc ('\n', fo);
		    }
		} else {
		    fputc ('r', fo);

#ifdef	CHECKDATA
		    dummy = CheckBuffer (treq->Buf, sreq * Unit, treq->Seed,
					 fo, &dummy);
#endif
		}
	    }
	    break;
	}

	fprintf (fo, "\n%d requests of %d KByte each read in %d ms ( %d KByte/sec )\n",
		 nreq, sreq * Unit / 1024, tsum / 1000, nreq * sreq * Unit / 1024 * OneSec / tsum);

	unless (TestKey (fi))
	  goto loop;
    }
}

void
HandleDisc (int volnum, char *volname, bool * opened)
{
    char            msg[128];
    char           *fname = "HandleDisc";
    VolInfo         info;
    FILE           *fi;
    FILE           *fo;
    TestNode       *tnode;
    TestReq        *treq;
    DiscReq        *dreq;
    int             errsum = 0;
    uword           t1, t2;
    int             ch, i;

    InitScreen (&info, volname);
    fi = info.Input;
    fo = info.Output;

    info.MaxPos = -1;
    info.MaxSize = 0x10000 / Unit;
    InitList (&info.Requests);
    InitSemaphore (&info.ReqLock, 0);
    info.WSeed = info.RSeed = 0;
/* Get default req & 64k buffer	 */
    while ((tnode = NewTNode (0x10000)) == NULL) {
	fprintf (fo, "\n%s #%d (%s): Failed to allocate default tnode.",
		 fname, volnum, volname);
	Delay (OneSec);
    }
    treq = tnode->TReq;
    treq->Info = &info;
    dreq = &treq->DReq;
    dreq->Buf = treq->Buf;
    dreq->DevReq.SubDevice = volnum;
    dreq->DevReq.Request = FG_GetSize;
    dreq->DevReq.Timeout = -1;
    fputs ("executing GetSize request ...\n", fo);
    VolExec (treq);
    Wait (&info.ReqLock);
    fprintf (fo, "Device has %d blocks of %d bytes.\n",
	     dreq->DevReq.Result, dreq->Actual);
    if (dreq->DevReq.Result >= 0)
	info.MaxPos = dreq->DevReq.Result;
    forever
    {
	fprintf (fo, "\n%s #%d (%s) :\n", fname, volnum, volname);
	fputs ("(R)ead  (W)rite  (F)ormat  (G)etSize  (T)est  (C)lose : ", fo);
	fflush (fo);
	ch = fgetc (fi);
	fprintf (fo, "%c\n", ch);
	switch (ch) {
	case 'g':
	case 'G':
	    dreq->DevReq.Request = FG_GetSize;
	    dreq->DevReq.Timeout = 5 * OneSec;
	    fputs ("executing GetSize request ...\n", fo);
	    VolExec (treq);
	    Wait (&info.ReqLock);
	    fprintf (fo, "Device has %d blocks of %d bytes.\n",
		     dreq->DevReq.Result, dreq->Actual);
	    if (dreq->DevReq.Result >= 0)
		info.MaxPos = dreq->DevReq.Result;
	    continue;
	case 'r':
	case 'R':
	    dreq->DevReq.Request = FG_Read;
	    dreq->DevReq.Timeout = 5 * OneSec;
	    fprintf (fo, "Read for Size ( 1 .. %d units ) : ", info.MaxSize);
	    if ((dreq->Size = fgetint (-1, info.MaxSize, fi, fo)) <= 0) {
		fputc ('\n', fo);
		continue;
	    }
	    fprintf (fo, "\nstarting at Unit ( 0 .. %d ) : ", info.MaxPos - dreq->Size);
	    if ((dreq->Pos = fgetint (-1, info.MaxPos - dreq->Size, fi, fo)) < 0) {
		fputc ('\n', fo);
		continue;
	    }
	    fputs ("\nexecuting Read request ...\n", fo);
	    t1 = _ldtimer (0);
	    VolExec (treq);
	    Wait (&info.ReqLock);
	    t2 = _ldtimer (0);
	    fprintf (fo, "time: %d ms, Actual: %d, Result 0x%x\n",
		     (t2 - t1) / 1000, dreq->Actual, dreq->DevReq.Result);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "+++\t%s\n", msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info.ReqLock);
		fprintf (fo, "Sense : ");
                for (i = 0; i < dreq->Actual; i++)
                {
                    if (0 == (i % 4))
                        fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
		continue;
	    }
	    fprintf (fo, "Comparison seed (0x%x) : ", info.RSeed);
	    info.RSeed = fgetint (info.RSeed, MaxInt, fi, fo);
	    fputc ('\n', fo);
	    info.RSeed = CheckBuffer (treq->Buf, dreq->Actual * Unit, info.RSeed, fo, &errsum);
	    continue;
	case 'w':
	case 'W':
	    dreq->DevReq.Request = FG_Write;
	    dreq->DevReq.Timeout = 5 * OneSec;
	    fprintf (fo, "Write for Size ( 1 .. %d units ) : ", info.MaxSize);
	    if ((dreq->Size = fgetint (-1, info.MaxSize, fi, fo)) <= 0) {
		fputc ('\n', fo);
		continue;
	    }
	    fprintf (fo, "\nstarting at Unit ( 0 .. %d ) : ", info.MaxPos - dreq->Size);
	    if ((dreq->Pos = fgetint (-1, info.MaxPos - dreq->Size, fi, fo)) < 0) {
		fputc ('\n', fo);
		continue;
	    }
	    fprintf (fo, "\nComparison seed (0x%x) : ", info.WSeed);
	    info.WSeed = fgetint (info.WSeed, MaxInt, fi, fo);
	    info.WSeed = SetBuffer (treq->Buf, dreq->Size * Unit, info.WSeed);
	    fputs ("\nexecuting Write request ...\n", fo);
	    t1 = _ldtimer (0);
	    VolExec (treq);
	    Wait (&info.ReqLock);
	    t2 = _ldtimer (0);
	    fprintf (fo, "time: %d ms, Actual: %d, Result 0x%x\n",
		     (t2 - t1) / 1000, dreq->Actual, dreq->DevReq.Result);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "+++\t%s\n", msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info.ReqLock);
		fprintf (fo, "Sense : ");
		for (i = 0; i < dreq->Actual; i++)
                {
                    if (0 == (i % 4))
                        fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
	    }
	    continue;

	case 'f':
	case 'F':
	    dreq->DevReq.Request = FG_Format;
	    dreq->DevReq.Timeout = -1;
	    fputs ("executing Format request ...\n", fo);
	    t1 = _ldtimer (0);
	    VolExec (treq);
	    Wait (&info.ReqLock);
	    t2 = _ldtimer (0);
	    fprintf (fo, "time: %d ms, Actual: %d, Result 0x%x\n",
		     (t2 - t1) / 1000, dreq->Actual, dreq->DevReq.Result);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "+++\t%s\n", msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info.ReqLock);
		fprintf (fo, "Sense : ");
		for (i = 0; i < dreq->Actual; i++)
                {
                    if (0 == (i % 4))
                        fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
	    }
	    continue;

	case 't':
	case 'T':
	    Free (tnode);		/* release allocated req	 */
	    TestDisc (&info, volnum, volname);
	/* Get default req & 64k buffer	 */
	    while ((tnode = NewTNode (0x10000)) == NULL) {
		fprintf (fo, "\n%s #%d (%s): Failed to allocate default tnode.",
			 fname, volnum, volname);
		Delay (OneSec);
	    }
	    treq = tnode->TReq;
	    treq->Info = &info;
	    dreq = &treq->DReq;
	    dreq->Buf = treq->Buf;
	    dreq->DevReq.SubDevice = volnum;
	    continue;

	case 'c':
	case 'C':
	    dreq->DevReq.Request = FG_Close;
	    dreq->DevReq.Timeout = 5 * OneSec;
	    fputs ("executing Close request ...\n", fo);
	    t1 = _ldtimer (0);
	    VolExec (treq);
	    Wait (&info.ReqLock);
	    t2 = _ldtimer (0);
	    fprintf (fo, "time: %d ms, Actual: %d, Result 0x%x\n",
		     (t2 - t1) / 1000, dreq->Actual, dreq->DevReq.Result);
	    if (dreq->DevReq.Result) {
		Fault (dreq->DevReq.Result, msg, 128);
		fprintf (fo, "+++\t%s\n", msg);
		dreq->DevReq.Request = FG_GetInfo;
		dreq->Size = 128;
		DevOperate (ddcb, dreq);
		Wait (&info.ReqLock);
		fprintf (fo, "Sense : ");
		for (i = 0; i < dreq->Actual; i++)
                {
                    if (0 == (i % 4))
                        fprintf (fo, "\n");
	            fprintf (fo, "%02x ", ((byte *) dreq->Buf)[i]);
                }
		fputc ('\n', fo);
	    }
	    fputs ("press any key to close window...", fo);
	    fflush (fo);
	    ch = fgetc (fi);
	    Free (tnode);
	    TidyScreen (&info);
	    *opened = FALSE;
	    return;

	default:
	    continue;
	}
    }
}


/************************************************************************
 * COUNT THE NUMBER OF INFO NODES IN A CHAIN
 *
 * Parameter :	info	= ptr to a Device Info structure
 * Return    :	number of Info entries
 *
 ***********************************************************************/

static          word
CountInfos (word * info)
{
    word            count = 1;

    forever
    {
	if (*info == -1)
	    return count;
	info = (word *) RTOA (*info);
	count++;
    }
}

void
Do_Signal (DiscReq * req)
{
    Signal (&req->WaitLock);
}

Attributes      OldAttr;
Attributes      NewAttr;

int
main (int argc, char **argv)
{
    char            msg[128];
    char           *fname = argv [0];
    char            wmname[128];
    void           *devinfo;		/* devinfo pointer		 */
    InfoNode       *info;		/* devinfo node pointer		 */
    DiscDevInfo    *ddinfo = NULL;	/* disc device info		 */
    FileSysInfo    *fsinfo = NULL;
    VolumeInfo     *vvinfo = NULL;	/* volume info			 */
    VolumeInfo     *vvptr;
    char           *driver;		/* name of device driver	 */
    word            volnum;		/* opened Volume number		 */
    char           *volname;
    bool           *opened;
    DiscReq         dreq;
    LoadResult      lr;
    word            omin, omax;
    word            nvol;
    word            ovol;
    int             i, ch;
    char           *FileSysName = argv [1]; 
    char           *DiscDevName;

    Seed = _ldtimer (0);

/* OI 23 Oct 91                                                */
/* Now using command line arguments instead of fixed "msctest" */

    if (argc != (1 + 1))
    {
    	fprintf (stderr, "Usage: remote <MSCboard> %s <FileServer>\n", fname);
    	return 1;
    }

/* get devinfo and driver name	 */

    if ((devinfo = load_devinfo ()) == NULL) {
	fprintf (stderr, "%s: cannot find DevInfo !", fname);
	return 1;			/* I cannot proceed without..	 */
    }

    if ((info = find_info (devinfo, Info_FileSys, FileSysName)) == NULL) {
	fprintf (stderr, "%s: cannot find %s file system entry in DevInfo !\n\n", fname, FileSysName);
	return 1;			/* I cannot proceed without..	 */
    }

    fsinfo = (FileSysInfo *) RTOA (info->Info);
    vvinfo = (VolumeInfo *) RTOA (fsinfo->Volumes);
    DiscDevName = RTOA (fsinfo->DeviceName);

    if ((info = find_info (devinfo, Info_DiscDev, DiscDevName)) == NULL) {
	fprintf (stderr, "%s: cannot find %s in DevInfo.\n\n", fname, DiscDevName);
	return 1;			/* I cannot proceed without..	 */
    }
    ddinfo = (DiscDevInfo *) RTOA (info->Info);
    driver = RTOA (ddinfo->Name);

    ddcb = DevOpen (driver, ddinfo);	/* Load and initialise the	 */
/* Device driver.		 */
    if (ddcb == NULL) {			/* I cannot proceed without...	 */
	fprintf (stderr, "%s: failed to open device driver %s !\n\n",
		 fname, driver);
	return 1;
    }
    Unit = ddinfo->Addressing;
    nvol = CountInfos ((word *) vvinfo);
    while ((opened = Malloc (nvol * sizeof (bool))) == NULL) {
	printf ("main: failed to allocate Opened array.\n");
	Delay (OneSec);
    }
    for (i = 0; i < nvol; i++)
	opened[i] = FALSE;

    GetAttributes (Heliosno (stdin), &OldAttr);
    NewAttr = OldAttr;
    RemoveAttribute (&NewAttr, ConsoleEcho);
    RemoveAttribute (&NewAttr, ConsolePause);
    RemoveAttribute (&NewAttr, ConsoleIgnoreBreak);
    RemoveAttribute (&NewAttr, ConsoleBreakInterrupt);
    RemoveAttribute (&NewAttr, ConsoleRawOutput);
    AddAttribute (&NewAttr, ConsoleRawInput);
    SetAttributes (Heliosno (stdin), &NewAttr);
    setvbuf (stdin, NULL, _IONBF, 0);

    strncpy (wmname, Heliosno (stdin)->Name, 127);
    wmname[127] = '\0';
    *(strrchr (wmname, c_dirchar)) = '\0';
    if ((WM = Locate (NULL, wmname)) == NULL) {
	printf ("main: failed to locate Window Manager.\n");
	exit (1);
    }
    InitSemaphore (&WMLock, 1);
    forever
    {
	vvptr = vvinfo;
	printf ("\nDefined Volumes : %d\n", nvol);
	for (i = 0, ovol = 0, vvptr = vvinfo; i < nvol;
	     i++, vvptr = (VolumeInfo *) RTOA (vvptr->Next)) {
	    printf ("\t%d : %c\t%s\n",
		    i, opened[i] ? '*' : ' ', (char *) RTOA (vvptr->Name));
	    if (opened[i])
		ovol++;
	}
	printf ("\n(O)pen Volume %s%s: ",
		ovol < nvol ? " open (A)ll " : "", ovol ? "" : " (Q)uit ");
	fflush (stdout);
	ch = fgetc (stdin);
	printf ("%c\n", ch);
	switch (ch) {
	case 'o':
	case 'O':
	    printf ("Which Volume : ");
	    volnum = fgetint (-1, nvol - 1, stdin, stdout);
	    fputc ('\n', stdout);
	    if (volnum < 0)
		continue;
	    if (opened[volnum]) {
		printf ("\nVolume %d already opened !\n", volnum);
		continue;
	    }
	    omin = omax = volnum;
	    break;
	case 'a':
	case 'A':
	    unless (ovol < nvol)
	      continue;
	    omin = 0;
	    omax = nvol - 1;
	    break;
	case 'q':
	case 'Q':
	    if (ovol)
		continue;
	    goto done;
	default:
	    continue;
	}
	for (volnum = omin; volnum <= omax; volnum++) {
	    if (opened[volnum])
		continue;
	    vvptr = vvinfo;
	    for (i = 0; i < volnum; i++)
		vvptr = (VolumeInfo *) RTOA (vvptr->Next);
	    volname = (char *) RTOA (vvptr->Name);
	    printf ("\nsending Open request for Volume #%d ( %s ) ...",
		    volnum, volname);
	    fflush (stdout);
	    dreq.DevReq.Request = FG_Open;
	    dreq.DevReq.Action = Do_Signal;
	    dreq.DevReq.SubDevice = volnum;
	    dreq.DevReq.Timeout = -1;
	    dreq.Pos = 0;
	    dreq.Size = 0;
	    dreq.Buf = &lr;
	    InitSemaphore (&dreq.WaitLock, 0);
	    start = _ldtimer (0);
	    DevOperate (ddcb, &dreq);
	    Wait (&dreq.WaitLock);
	    end = _ldtimer (0);
	    printf ("time = %d ms\n", (end - start) / 1000);
	    printf ("Actual %d, Result 0x%x\n", dreq.Actual, dreq.DevReq.Result);
	    if (dreq.DevReq.Result) {
		Fault (dreq.DevReq.Result, msg, 128);
		printf ("+++\t%s\n", msg);
		dreq.DevReq.Request = FG_GetInfo;
		dreq.Buf = msg;
		dreq.Size = 128;
		DevOperate (ddcb, &dreq);
		Wait (&dreq.WaitLock);
		printf ("Sense : ");
		for (i = 0; i < dreq.Actual; i++)
                {
                    if (0 == (i % 4))
                        fprintf (stdout, "\n");
	            fprintf (stdout, "%02x ", ((byte *) dreq.Buf)[i]);
                }
		fputc ('\n', stdout);
	    }
	    printf ("\nLoad Result :\tError 0x%x\n", lr.Error);
	    if (lr.Error) {
		Fault (lr.Error, msg, 128);
		printf ("+++\t%s\n", msg);
	    }
	    printf ("\tRaw %d, Removable %d, Loaded %d, Protected %d, Formatted %d, NotLocked %d\n",
		    lr.Raw, lr.Removable, lr.Loaded,
		    lr.Protected, lr.Formatted, lr.NotLocked);
	    fflush (stdout);
	    unless (lr.Loaded)
	      printf ("Volume #%d not loaded.\n", volnum);
	    else
	    {
		if (lr.Raw) {
		    unless (Fork (2000, HandleTape, 12, volnum, volname, &opened[volnum])) {
			printf ("main: failed to fork HandleTape for Volume %d\n",
				volnum);
		    }
		    else
		    opened[volnum] = TRUE;
		} else {
		    unless (Fork (2000, HandleDisc, 12, volnum, volname, &opened[volnum])) {
			printf ("main: failed to fork HandleDisc for Volume %d\n",
				volnum);
		    }
		    else
		    opened[volnum] = TRUE;
		}
		Delay (OneSec);
	    }
	}
    }
done:
    DevClose (ddcb);

    fputs ("\x09b25B\r\x9bJ", stdout);
    SetAttributes (Heliosno (stdin), &OldAttr);
}
