/*
 * Emulation for xterm-compatible termcap entry.
 */

#include <InterViews/defs.h>
#include <InterViews/Text/emulator.h>
#include <InterViews/Text/oldtextbuffer.h>
#include <osfcn.h>

static const int ESC = 033;	/* ascii escape */
static const int BELL = 07;	/* audible bell */
static const int TAB = 011;	/* vertical tab */

static const int MAXARGS = 15;

/*
 * Set screen attributes
 */

void Emulator::SetAttributes (int args[], int n) {
    register int i;

    for (i = 0; i < n; i++) {
	switch (args[i]) {
	    case 0:	/* reset state */
		text->Underline(false);
		text->Inverse(false);
		text->Bold(false);
		break;
	    case 1:	/* bold */
		text->Bold(true);
		break;
	    case 4:	/* underline */
		text->Underline(true);
		break;
	    case 5:	/* blink */
		text->Blink(true);
		break;
	    case 7:	/* inverse */
		text->Inverse(true);
		break;
	    case 8:	/* display */
		/* what does this mean? */
		break;
	    default:
		/* nothing to do */;
	}
    }
}

/*
 * Parse ANSI terminal arguments of the form N;M
 * where N and M are integers and possibly ommited.
 */

void Emulator::Args (const char* st, int blen, int& bc, int& n, int args[]) {
    register int i;

    for (i = 0; ; i++) {
	if (i < MAXARGS) {
	    args[i] = 0;
	}
	while (bc < blen && st[bc] >= '0' && st[bc] <= '9') {
	    if (i < MAXARGS) {
		args[i] = 10*args[i] + (st[bc]-'0');
	    }
	    ++bc;
	}
	if (bc >= blen || st[bc] != ';') {
	    break;
	}
	++bc;
    }
    n = min(i+1, MAXARGS);
}

/*
 * Validate that the given arguments are legal for the
 * given function
 */

boolean Emulator::Valid (int c, int numargs, int args[]) {
    boolean r;
    register int i, a;

    r = true;
    switch (c) {
	case 'c': /* terminal identification */
	    if (numargs > 1) {
		r = false;
	    } else if (args[0] != 0) {
		r = false;
	    }
	    break;
	case 'r': /* screen region  */
	    if (numargs > 2) {
		r = false;
	    } else if (numargs == 2) {
		if (args[0] > text->GetHeight() ||
		    args[1] > text->GetHeight() ||
		    args[0] > args[1]) {
		    r = false;
		}
	    } else if (numargs == 1) {
		if (args[0] > text->GetHeight()) {
		    r = false;
		}
	    }
	    break;
	case 'J': /* screen erase */
	case 'K': /* line erase */
	    if (numargs != 0) {
		a = args[0];
		if (a != 0 && a != 1 && a != 2) {
		    r = false;
		}
	    }
	    break;
	case 'm':
	    for (i = 0; i < numargs; i++) {
		a = args[i];
		if (a != 0 && a != 4 && a != 5 && a != 7 && a != 8) {
		    r = false;
		    break;
		}
	    }
	    break;
	default:
	    /* assume ok */;
    }
    return r;
}

/*
 * General purpose routine to handle default arguments.
 */

void Emulator::DefaultArgs (int c, int& numargs, int args[]) {
    switch (c) {
	case 'A': /* cursor up */
	case 'B': /* cursor down */
	case 'C': /* cursor right */
	case 'D': /* cursor left */
	    if (numargs == 0 || args[0] == 0) {
		args[0] = 1;
	    }
	    numargs = 1;
	    break;
	case 'H': /* cursor addressing */
	    if (numargs == 0) {
		args[0] = 1;
		args[1] = 1;
	    } else if (numargs == 1) {
		if (args[0] == 0) {
		    args[0] = 1;
		}
		args[1] = 1;
	    } else if (numargs > 1) {
		if (args[0] == 0) {
		    args[0] = 1;
		}
		if (args[1] == 0) {
		    args[1] = 1;
		}
	    }
	    numargs = 2;
	    break;
	case 'J': /* erase screen */
	case 'K': /* erase line */
	case 'm': /* set attributes */
	    if (numargs == 0) {
		args[0] = 0;
	    }
	    numargs = 1;
	    break;
	case 'L': /* insert line */
	case 'M': /* delete line */
	case 'P': /* insert character */
	case '@': /* delete character */
	    if (numargs == 0 || args[0] == 0) {
		args[0] = 1;
	    }
	    numargs = 1;
	    break;
	case 'c': /* Terminal Identification */
	    if (numargs == 0) {
		args[0] = 0;
	    }
	    numargs = 1;
	    break;
	case 'r': /* screen region */
	    if (numargs == 0) {
		args[0] = 1;
		args[1] = text->GetHeight();
	    } else if (numargs == 1) {
		args[1] = text->GetHeight();
	    }
	    numargs = 2;
	    break;
	default:
	    numargs = 0;
    }
}

/*
 * This is called if an escape sequence isn't complete.
 */

void Emulator::SaveEscape (const char* buf, int blen, int bp) {
    register int i, j;

    for (i = 0, j = bp; i < sizeof(escbuffer) && j < blen; i++, j++) {
	escbuffer[i] = buf[j];
    }
    within = i;
}

/*
 * This routine is called to complete an escape sequence that
 * was terminated in the middle.
 */

void Emulator::CompleteEscape (const char* buf, int blen, int& bp) {
    int c;
    register int i, j;
    int nargs;
    int args[MAXARGS];
    int bc;

    j = min(sizeof(escbuffer), within + blen);
    for (i = within; i < j; i++) {
	escbuffer[i] = buf[i - within];
    }
    if (ParseEscape(escbuffer, j, bp, bc, c, args, nargs)) {
	DoEscape(c, args, nargs);
	bp += (bc - within);
	within = 0;
    } else {
	bp = blen;
    }
}

/*
 * This routine is called when an escape character is found. It interprets
 * and invokes the necessary routines to perform the command sequence.
 */

boolean Emulator::ParseEscape (
    const char* buf, int blen, int obp, int& bc, int& c,
    int args[], int& numargs
) {
    int bp;
    boolean r;
    register char *cur;

    r = true;
    bp = obp + 1;
    cur = (char*) &buf[bp];
    if (bp >= blen) {
	r = false;
	SaveEscape(buf, blen, obp);
	--bp;
    } else if (*cur == '[') {
	++bp;
	++cur;
	if (*cur == '?' || *cur == '=') {
	    ++bp;
	    ++cur;
	}
	if (*cur == ';' || (*cur >= '0' && *cur <= '9')) {
	    Args(buf, blen, bp, numargs, args);
	} else {
	    numargs = 0;
	}
	if (bp >= blen) {
	    r = false;
	    SaveEscape(buf, blen, obp);
	    --bp;
	} else {
	    c = buf[bp];
	    if (Valid(c, numargs, args)) {
		DefaultArgs(c, numargs, args);
	    } else {
		r = false;
	    }
	}
    } else {
	c = *cur;
	switch (*cur) {
	case '7':			/* Save Cursor */
	case '8':			/* Restore Cursor */
	    break;
	case 'D':			/* forward scroll */
	case 'M':			/* reverse scroll */
	    numargs = -1;
	    break;
	case '(':			/* set normal character set */
	case ')':			/* set alternate character set */
	    if (bp >= blen) {
		r = false;
		SaveEscape(buf, blen, obp);
		--bp;
	    } else {
		++bp;
		++cur;
		switch (*cur) {
		case 'A':		/* supported font types */
		case 'B':
		case '0':
		    args[0] = *cur;
		    numargs = 1;
		    break;
		default:
		    r = false;
		    break;
		}
	    }
	}
    }
    bc = bp - obp + 1;
    return r;
}

/*
 * Once the escape sequence is parsed, get here to execute it.
 */

void Emulator::DoEscape (int func, int args[], int nargs) {
    switch (func) {
	case 'A':
	    text->CursorUp(args[0]);
	    break;
	case 'B':
	    text->CursorDown(args[0]);
	    break;
	case 'C':
	    text->CursorRight(args[0]);
	    break;
	case 'D':
	    if (nargs >= 0) {
		text->CursorLeft(args[0]);
	    } else {
		text->ForwardScroll();
	    }
	    break;
	case 'H':
	    text->Goto(args[0], args[1]);
	    break;
	case 'J':
	    text->EraseScreen(args[0]);
	    break;
	case 'K':
	    if (args[0] == 0) {
		text->EraseEOL();
	    } else if (args[0] == 1) {
		text->EraseBOL();
	    } else {
		text->EraseLine();
	    }
	    break;
	case 'L':
	    text->InsertLines(args[0]);
	    break;
	case 'M':
	    if (nargs >= 0) {
		text->DeleteLines(args[0]);
	    } else {
		text->ReverseScroll();
	    }
	    break;
	case 'P':
	    text->DeleteCharacters(args[0]);
	    break;
	case '@':
	    text->InsertCharacters(args[0]);
	    break;
	case 'c': /* Terminal Identification */
	    write(device, "\033[?6c", 5);
	    break;
	case 'm': /* set attributes */
	    SetAttributes(args, nargs);
	    break;
	case 'r': /* set screen region */
	    text->SetRegion(args[0], args[1]);
	    break;
	case '7': /* Save cursor position */
	    text->SaveCursor();
	    break;
	case '8': /* Restore cursor position */
	    text->RestoreCursor();
	    break;
	case '(': /* Set normal character set */
	    text->SetNorCharSet(args[0]);
	    break;
	case ')': /* Set alternate character set */
	    text->SetAltCharSet(args[0]);
	    break;
	default:
	    /* ignore */;
    }
}

/*
 * Write out characters, interpreting escape sequences.
 */

void Emulator::Write (const char* buf, int blen) {
    register int bp;
    register int c;
    int tmpbp, bc;
    int func;
    int nargs;
    int args[MAXARGS];

    bp = 0;
    if (within != 0) {
	tmpbp = bp;
	CompleteEscape(buf, blen, tmpbp);
	bp = tmpbp;
    } else {
	text->CursorOff();
    }
    text->CheckScroll(buf, bp, blen);
    text->SavePos();
    for (; bp < blen; bp++) {
	c = buf[bp];
	switch (c) {
	    case ESC:
		tmpbp = bp;
		if (ParseEscape(buf, blen, tmpbp, bc, func, args, nargs)) {
		    text->FlushLine();
		    DoEscape(func, args, nargs);
		    text->SavePos();
		}
		bp = tmpbp + bc - 1;
		break;
	    case '\016':		/* SO */
		text->UseAlt(true);
		break;
	    case '\017':		/* SI */
		text->UseAlt(false);
		break;
	    case '\r':
		text->CarriageReturn();
		break;
	    case '\n':	
		text->FlushLine();
		text->ForwardScroll();
		text->FlushLine();
		break;
	    case '\b':
		text->BackSpace();
		break;
	    case TAB:
		text->Tab();
		break;
	    case BELL:
//		world->RingBell(1);
		break;
	    case '\0':
	    case '\177':		/* padding */
		break;
	    default:
		text->AddChar(c);
	}
    }
    text->FlushLine();
    if (within == 0) {
	text->CursorOn();
    }
}
