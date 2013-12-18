/*
 * Text buffer manager built on top of painter drawing primitives.
 */

#include <InterViews/Text/oldtextbuffer.h>
#include <InterViews/canvas.h>
#include <InterViews/paint.h>
#include <InterViews/painter.h>
#include <InterViews/shape.h>
#include <bstring.h>
#include <string.h>

static const int tabstops = 8;
static const char InverseBit = 0x1;
static const char UnderlineBit = 0x2;
static const char AltcharBit = 0x4;

inline boolean IsOn (char attr, char flag) {
    return (attr&flag) != 0;
}

inline boolean IsOff (char attr, char flag) {
    return (attr&flag) == 0;
}

inline void OldTextBuffer::SetCurPos () {
    curx = MapCharX(charx);
    cury = MapCharY(chary);
}

OldTextBuffer::OldTextBuffer () {
    rows = 24;
    cols = 80;
    Init();
}

OldTextBuffer::OldTextBuffer (int r, int c) {
    rows = r;
    cols = c;
    Init();
}

void OldTextBuffer::Reconfig () {
    shape->hunits = output->GetFont()->Width("n");
    shape->width = cols * shape->hunits;
    shape->vunits = output->GetFont()->Height();
    shape->height = rows * shape->vunits;
    delete highlight;
    highlight = new Painter(output);
    highlight->SetColors(output->GetBgColor(), output->GetFgColor());
}

void OldTextBuffer::Init () {
    highlight = nil;
    top = 0; bot = height;
    width = 0; height = 0;
    norCharSet = 'B';
    altCharSet = 'B';
    savCharSet = 'B';
    spaces = nil;
    data = nil;
    underline = false; savunderline = false;
    inverse = false; savinverse = false;
    curAttributes = 0; savAttributes = 0;
    charx = 0; chary = 0;
    savcharx = 0; savchary = 0;
    prescroll = true;
    showcursor = false;
    outline = false;
}

int OldTextBuffer::Column (Coord x) {
    register int c = x/charwidth;
    return (x>0) ? c+1 : c;
}

int OldTextBuffer::Row (Coord y) {
    register int y0 = ymax-y;
    register int r = y0/charheight;
    return (y0>0) ? r+1 : r;
}

void OldTextBuffer::Resize () {
    int oldwidth, oldheight, lastline, offj;
    LinePtr* olddata;
    register int i;
    register Line* line;

    canvas->SetBackground(output->GetBgColor());
    charwidth = shape->hunits;
    charheight = shape->vunits;
    oldwidth = width;
    width = (xmax + 1) / charwidth;
    oldheight = height;
    height = (ymax + 1) / charheight;
    if (bot == oldheight || bot > height) {
	bot = height;
    }
    delete spaces;
    spaces = new char[width+1];
    for (i = 0; i < width; i++) {
	spaces[i] = ' ';
    }
    spaces[width] = '\0';
    olddata = data;
    data = new LinePtr[height];
    for (i = 0; i < height; i++) {
	line = new Line;
	line->width = 0;
	line->data = new char[width+1];
	line->attributes = new char[width+1];
	strncpy(line->data, spaces, width+1);
	bzero(line->attributes, width+1);
	line->attributesSet = 0;
	data[i] = line;
    }
    /*
     * Try to preserve the bottom of the screen if shrinkage
     */
    if (oldheight > height) {
	/* shrunk */
	lastline = oldheight;
	do {
	    --lastline;
	} while (lastline >= 0 && olddata[lastline]->width == 0);
	if (height >= lastline) {
	    /* current screen fits */
	    offj = 0;
	} else {
	    /* not everything will fit */
	    offj = lastline - height + 1;
	}
    } else {
	/* grew */
	offj = 0;
    }
    for (i = 0; i < oldheight; i++) {
	if (i >= offj && i < offj + height) {
	    line = data[i-offj];
	    line->width = min(olddata[i]->width, width);
	    if (line->width != 0) {
		strncpy(line->data, olddata[i]->data, line->width);
		strncpy(line->attributes, olddata[i]->attributes, line->width);
		line->attributesSet = olddata[i]->attributesSet;
	    } else {
		line->attributesSet = 0;
	    }
	}
	delete olddata[i]->data;
	delete olddata[i]->attributes;
	delete olddata[i];
    }
    delete olddata;
    if (charx >= width) {
	charx = width - 1;
    }
    if (chary <= offj) {
	chary = 0;
    } else if (chary >= height) {
	chary = height - 1;
    }
    curx = MapCharX(charx);
    cury = MapCharY(chary);
    savcurx = curx;
    savcury = cury;
}

void OldTextBuffer::Redraw (Coord left, Coord bottom, Coord right, Coord top) {
    int leftchar, rightchar, firstline, lastline;
    register int i, j;
    int x, y;
    int w, ulp, inp, ntp;		/* underline, inverse and text */
    register Line* line;

    if (top > ymax) {
	firstline = 0;
    } else {
	firstline = (ymax - top) / charheight;
    }
    if (bottom < 0) {
	lastline = height - 1;
    } else {
	lastline = ((ymax + charheight) / charheight) - 1;
	if (lastline >= height) {
	    lastline = height - 1;
	}
    }
    if (left < 0) {
	leftchar = 0;
    } else {
	leftchar = left / charwidth;
    }
    if (right > xmax) {
	rightchar = width - 1;
    } else {
	rightchar = (xmax + charwidth) / charwidth;
    }
    output->ClearRect(canvas, left, bottom, right, top);
    x = MapCharX(leftchar);
    y = MapCharY(firstline);
    for (i = firstline; i <= lastline; i++) {
	line = data[i];
	if (line->width != 0) {
	    w = min(line->width-1, rightchar);
	    if (IsOn(line->attributesSet, InverseBit|UnderlineBit)) {
		j = leftchar;
		inp = -1;
		ntp = -1;
		while (j <= w) {
		    if (IsOn(line->attributes[j], InverseBit)) {
			if (ntp >= 0) { /* have normal text */
			    output->MoveTo(x + MapCharX(ntp), y);
			    output->Text(canvas, &line->data[ntp], j - ntp);
			    ntp = -1;	/* reset */
			}
			if (inp < 0) {
			    inp = j;	/* save initial position */
			}
		    } else {
			if (inp >= 0) {  /* have inverse text */
			    highlight->MoveTo(x + MapCharX(inp), y);
			    highlight->Text(canvas, &line->data[inp], j - inp);
			    inp = -1;	/* reset */
			}
			if (ntp < 0) {
			    ntp = j;	/* save initial position */
			}
		    }
		    j++;		/* next character position */
		}
		if (ntp >= 0) {		/* left over normal text */
		    output->MoveTo(x + MapCharX(ntp - leftchar), y);
		    output->Text(canvas, &line->data[ntp], j - ntp);
		} else if (inp >= 0) {	/* left over inverse text */
		    highlight->MoveTo(x + MapCharX(inp - leftchar), y);
		    highlight->Text(canvas, &line->data[inp], j - inp);
		}
		j = leftchar;
		ulp = -1;
		while (j <= w) {
		    if (IsOn(line->attributes[j], UnderlineBit)) {
			if (ulp < 0) {
			    ulp = j;	/* save initial position */
			}
		    } else {
			if (ulp >= 0) { /* have underlined text */
			    output->Line(
				canvas, x + MapCharX(ulp), y,
				x + MapCharX(j) - 1, y
			    );
			    ulp = -1;	/* reset */
			}
		    }
		    j++;		/* next character position */
		}
		if (ulp >= 0) {		/* left over underlines */
		    output->Line(
			canvas, x + MapCharX(ulp), y,
			x + MapCharX(j) - 1, y
		    );
		}
	    } else if (w >= leftchar) {
		output->MoveTo(x, y);
		output->Text(canvas, &line->data[leftchar], w - leftchar + 1);
	    }
	}
	y -= charheight;
    }
    if (showcursor && chary >= firstline && chary <= lastline) {
	CursorOn();
    }
}

OldTextBuffer::~OldTextBuffer () {
    register int i;

    delete highlight;
    for (i = 0; i < height; i++) {
	delete data[i]->data;
	delete data[i]->attributes;
	delete data[i];
    }
    delete data;
    delete spaces;
}

void OldTextBuffer::CursorOn () {
    char buf[1], attr;
    register int col;
    register Line* line;
    register Coord x, y;
    Painter* p;

    if (!showcursor) {
	showcursor = true;
	line = data[chary];
	if (charx >= line->width && charx != width) {
	    buf[0] = ' ';
	    attr = '\0';
	} else {
	    col = min(charx, width - 1);
	    buf[0] = line->data[col];
	    attr = line->attributes[col];
	}
	x = curx;
	y = cury;
	if (outline) {
	    output->Rect(canvas, x, y, x + charwidth - 1, y + charheight-1);
	} else {
	    p = IsOn(attr, InverseBit) ? output : highlight;
	    p->MoveTo(x, y);
	    p->Text(canvas, buf, 1);
	    if (IsOn(attr, UnderlineBit)) {
		p->Line(canvas, x, y, x + charwidth - 1, y);
	    }
	}
    }
}

void OldTextBuffer::CursorOff () {
    char buf[1], attr;
    register Line* line;
    register int col;
    Painter* p;

    if (showcursor) {
	showcursor = false;
	line = data[chary];
	if (charx >= line->width && charx != width) {
	    buf[0] = ' ';
	    attr = '\0';
	} else {
	    col = min(charx, width-1);
	    buf[0] = line->data[col];
	    attr = line->attributes[col];
	}
	p = IsOn(attr, InverseBit) ? highlight : output;
	p->MoveTo(curx, cury);
	p->Text(canvas, buf, 1);
	if (IsOn(attr, UnderlineBit)) {
	    p->Line(canvas, curx, cury, curx + charwidth - 1, cury);
	}
    }
}

/*
 * Clear a given number of lines starting at a given line number.
 */

void OldTextBuffer::ClearLines (int where, int count) {
    int right, limit, i;
    register Line* line;

    right = 0;
    limit = min(where + count, bot);
    for (i = where; i < limit; i++) {
	line = data[i];
	if (line->width > right) {
	    right = line->width;
	}
	line->width = 0;
	strncpy(line->data, spaces, width + 1);
	bzero(line->attributes, width + 1);
	line->attributesSet = 0;
    }
    if (right > 0) {
	output->ClearRect(
	    canvas, 0, MapCharY(limit-1),
	    MapCharX(right) - 1, MapCharY(where-1) - 1
	);
    }
}

/*
 * Clear the entire text buffer.
 */

void OldTextBuffer::ClearScreen () {
    EraseScreen(2);
    charx = 0; chary = 0;
    SetCurPos();
}

/*
 * Cursor movement.
 */

void OldTextBuffer::CursorDown (int count) {
    int last;

    if (chary < bot) {
	last = bot;
    } else {
	last = height;
    }
    chary += count;
    if (chary >= last) {
	chary = last - 1;
    }
    cury = MapCharY(chary);
}

void OldTextBuffer::CursorLeft (int count) {
    charx -= count;
    if (charx < 0) {
	charx = 0;
    }
    curx = MapCharX(charx);
}

void OldTextBuffer::CursorRight (int count) {
    charx += count;
    if (charx >= width) {
	charx = width - 1;
    }
    curx = MapCharX(charx);
}

void OldTextBuffer::CursorUp (int count) {
    int first;

    if (chary >= top) {
	first = top;
    } else {
	first = 0;
    }
    chary -= count;
    if (chary < first) {
	chary = first;
    }
    cury = MapCharY(chary);
}

/*
 * Delete characters.
 */

void OldTextBuffer::DeleteCharacters (int count) {
    int rightx;
    register int i;
    register Line* line;

    line = data[chary];
    if (charx > line->width) {
	return;
    }
    rightx = width - count;
    for (i = charx; i < rightx; i++) {
	line->data[i] = line->data[i+count];
	line->attributes[i] = line->attributes[i+count];
    }
    if (charx + count - 1 <= line->width) {
	line->width -= count;
    } else if (charx == 0) {
	line->width = 0;
    } else {
	line->width = charx - 1;
    }
    if (charx + count < width) {
	output->Copy(
	    canvas, curx + count*charwidth, cury,
	    width*charwidth-1, cury + charheight-1,
	    canvas, curx, cury
	);
    }
    output->ClearRect(
	canvas, max(0, (width-count)*charwidth), cury,
	width*charwidth - 1, cury + charheight - 1
    );
}

/*
 * Erase to beginning of line.
 */

void OldTextBuffer::EraseBOL () {
    register int i;
    register Line* line;

    output->ClearRect(
	canvas, 0, cury, curx + charwidth - 1, cury + charheight - 1
    );
    line = data[chary];
    for (i = 0; i < charx; i++) {
	line->data[i] = ' ';
	line->attributes[i] = 0;
    }
}

/*
 * Erase to end of line
 */

void OldTextBuffer::EraseEOL () {
    register int i;
    register Line* line;

    output->ClearRect(canvas, curx, cury, xmax, cury + charheight - 1);
    line = data[chary];
    for (i = charx; i < width; i++) {
	line->data[i] = ' ';
	line->attributes[i] = 0;
    }
    line->width = charx;
    if (charx == 0) line->attributesSet = 0;
}

/*
 * Erase the current line.
 */

void OldTextBuffer::EraseLine () {
    ClearLines(chary, 1);
}

/*
 * Clear from the current line to the bottom of the terminal.
 */

void OldTextBuffer::EraseEOS () {
    EraseEOL();
    ClearLines(chary, height - chary);
}

/*
 * Handle screen erasure.
 *
 *	0 Erase from current to end of screen (inclusive)
 *      1 Erase from current to beginning of screen (inclusive)
 *      2 Erase entire screen
 */

void OldTextBuffer::EraseScreen (int mode) {
    register Line* line;
    int i, lb, le;
    Coord yb, ye;

    switch (mode) {
	case 0:
	    EraseEOL();
	    yb = 0;
	    lb = chary + 1;
	    ye = cury - 1;
	    le = height - 1;
	    break;
	case 1:
	    EraseBOL();
	    yb = cury + charheight;
	    lb = 0;
	    ye = ymax;
	    le = chary - 1;
	    break;
	case 2:
	    yb = 0;
	    lb = 0;
	    ye = ymax;
	    le = height - 1;
	    break;
	default:
	    return;
    }
    if (yb < ye) {
	output->ClearRect(canvas, 0, yb, xmax, ye);
    }
    for (i = lb; i <= le; i++) {
	line = data[i];
	line->width = 0;
	strncpy(line->data, spaces, width + 1);
	bzero(line->attributes, width + 1);
	line->attributesSet = 0;
    }
}

/*
 * Goto a character position on the window.
 */

void OldTextBuffer::Goto (int row, int col) {
    if (col > 0 && col <= width && row > 0 && row <= height) {
	charx = col - 1;
	chary = row - 1;
	SetCurPos();
    }
}

/*
 * Insert characters
 */

void OldTextBuffer::InsertCharacters (int count) {
    register int i, rightx;
    register Line* line;

    line = data[chary];
    if (charx > line->width) {
	return;
    }
    rightx = charx + count;
    if (rightx < width) {
	for (i = width-1; i >= rightx; i--) {
	    line->data[i] = line->data[i-count];
	    line->attributes[i] = line->attributes[i-count];
	}
	output->Copy(
	    canvas, curx, cury,
	    (width-count)*charwidth - 1, cury + charheight - 1,
	    canvas, curx + count*charwidth, cury
	);
    }
    line->width += count;
    output->ClearRect(
	canvas, curx, cury,
	min(xmax-1, curx + count*charwidth - 1), cury + charheight - 1
    );
}

/*
 * Perform downwards scrolling.
 */

void OldTextBuffer::ScrollDown (int where, int count) {
    const int MAXLINES = 200;
    register Line* line;
    int i, right, limit, last;
    Line* wdp[MAXLINES];
    int savewidth;

    right = 0;
    limit = max(bot - count, where);
    for (i = limit; i < bot; i++) {
	line = data[i];
	if (line->width > right) {
	    right = line->width;
	}
	wdp[i-limit] = line;
    }
    last = where + count;
    if (last < bot) {
	for (i = bot - 1; i >= last; i--) {
	    line = data[i-count];
	    if (line->width > right) {
		right = line->width;
	    }
	    data[i] = line;
	}
	output->Copy(
	    canvas, 0, MapCharY(limit-1),
	    MapCharX(right) - 1, MapCharY(where-1) - 1,
	    canvas, 0, MapCharY(bot-1)
	);
    }
    last = min(last, bot);
    for (i = where; i < last; i++) {
	savewidth = data[i]->width;
	data[i] = wdp[i-where];
	data[i]->width = savewidth;
    }
    ClearLines(where, count);
}

/*
 * Perform upwards scrolling.
 */

void OldTextBuffer::ScrollUp (int where, int count) {
    const int MAXLINES = 200;
    register Line* line;
    register int i, right, limit;
    Line* wdp[MAXLINES];
    int savewidth;

    right = 0;
    limit = min(where + count, bot);
    for (i = where; i < limit; i++) {
	line = data[i];
	if (line->width > right) {
	    right = line->width;
	}
	wdp[i-where] = line;
    }
    if (limit < bot) {
	for (i = limit; i < bot; i++) {
	    line = data[i];
	    if (line->width > right) {
		right = line->width;
	    }
	    data[i-count] = line;
	}
	output->Copy(
	    canvas, 0, MapCharY(bot-1),
	    MapCharX(right) - 1, MapCharY(where+count-1) - 1,
	    canvas, 0, MapCharY(bot-count-1)
	);
    }
    limit = max(bot - count, where);
    for (i = limit; i < bot; i++) {
	savewidth = data[i]->width;
	data[i] = wdp[i-limit];
	data[i]->width = savewidth;
    }
    ClearLines(limit, count);
}

void OldTextBuffer::ForwardScroll () {
    if (chary == bot-1) {
	ScrollUp(top, 1);
    } else if (chary < height-1) {
	CursorDown(1);
    }
}

void OldTextBuffer::ReverseScroll () {
    if (chary == top) {
	ScrollDown(chary, 1);
    } else if (chary > 0) {
	CursorUp(1);
    }
}

void OldTextBuffer::InsertLines (int count) {
    if (chary >= top && chary < bot) {
	ScrollDown(chary, count);
	charx = 0;
	curx = 0;
    }
}

void OldTextBuffer::DeleteLines (int count) {
    if (chary >= top && chary < bot) {
	ScrollUp(chary, count);
	charx = 0;
	curx = 0;
    }
}

/*
 * Determine the number of lines we can scroll now so
 * we won't have to do them one-by-one later.
 */

void OldTextBuffer::PreScroll (const char* bufstart, const char* bufend) {
    register char* cur;
    register int cc;
    int lines, tablength;

    lines = 0;
    cc = charx;
    for (cur = (char*) bufstart; cur < bufend; cur++) {
	if (*cur == '\n') {
	    ++lines;
	    cc = 0;
	} else if (*cur == '\r') {
	    cc = 0;
	} else {
	    if (cc == width) {
		++lines;
		cc = 0;
	    }
	    if (*cur == '\t') {
		tablength = tabstops - (cc % tabstops);
		if (cc + tablength < width) {
		    cc += tablength;
		}
	    } else if (*cur >= ' ' && *cur < '\177') {
		++cc;
	    } else {
		/* play it safe for unknown control characters */
		break;
	    }
	}
    }
    lines = (chary + lines + 1) - bot;
    if (lines > chary) {
	lines = chary;
    }
    if (lines > 1) {
	ScrollUp(top, lines);
	chary -= lines;
	cury = MapCharY(chary);
    }
}

/*
 * Flush the current line of the text buffer.
 */

void OldTextBuffer::FlushLine () {
    Painter* p;
    register Line* line;
    int num;

    if (lastx == curx && lasty == cury) {
	return;
    }
    if (lasty != cury && charx < lastc) {
	lastc = 0;
    }
    num = charx - lastc;
    line = data[chary];
    if (num > 0 && line->width > 0) {
	p = inverse ? highlight : output;
	p->MoveTo(lastx, cury);
	p->Text(canvas, &line->data[lastc], num);
	if (underline) {
	    p->Line(
		canvas, lastx, cury, min(lastx+MapCharX(num) - 1, xmax), cury
	    );
	}
    }
    SavePos();
}

void OldTextBuffer::SavePos () {
    lastx = curx;
    lasty = cury;
    lastc = charx;
}

void OldTextBuffer::CarriageReturn () {
    FlushLine();
    curx = 0;
    charx = 0;
    FlushLine();
}

void OldTextBuffer::BackSpace () {
    if (curx - charwidth >= 0) {
	FlushLine();
	if (charx != width) {
	    curx -= charwidth;
	}
	--charx;
	FlushLine();
    }
}

void OldTextBuffer::Tab () {
    int stop;

    stop = tabstops - charx % tabstops;
    if (charx + stop < width) {
	FlushLine();
	charx += stop;
	curx = MapCharX(charx);
	lastx = curx;
	lastc = charx;
    }
}

void OldTextBuffer::Underline (boolean b) {
    underline = b;
    if (b) {
	curAttributes |= UnderlineBit;
    } else {
	curAttributes &= ~UnderlineBit;
    }
}

void OldTextBuffer::Inverse (boolean b) {
    inverse = b;
    if (b) {
	curAttributes |= InverseBit;
    } else {
	curAttributes &= ~InverseBit;
    }
}

void OldTextBuffer::Bold (boolean) {
    /* unimplemented */
}

void OldTextBuffer::Blink (boolean) {
    /* unimplemented */
}

void OldTextBuffer::UseAlt (boolean b) {
    if (b) {
	curAttributes |= AltcharBit;
    } else {
	curAttributes &= ~AltcharBit;
    }
}

void OldTextBuffer::AddChar (char c) {
    register Line* line;

    if (charx >= width) {
	FlushLine();
	charx = 0;
	curx = MapCharX(charx);
	if (chary >= bot - 1) {
	    if (chary >= top && chary < bot) {
		ScrollUp(top, 1);
	    }
	} else {
	    cury -= charheight;
	    ++chary;
	}
	FlushLine();
    }
    switch (IsOn(curAttributes, AltcharBit) ? altCharSet : norCharSet) {
	case 'A':			/* United Kingdom */
	    if (c == '#') {
		c = '\036';
	    }
	    break;
	case 'B':			/* ASCII */
	    break;
	case '0':			/* Special Graphics */
	    if (c >= 0x5f && c <= 0x7e) {
		c -= 0x5f;
	    }
	    break;
    }
    curx += charwidth;
    line = data[chary];
    line->data[charx] = c&0177;
    line->attributes[charx] = curAttributes;
    line->attributesSet |= curAttributes;
    ++charx;
    line->width = max(charx, line->width);
    if (charx == width) {		/* funny VT100 margins */
	FlushLine();
	curx -= charwidth;
	FlushLine();
    }
}

void OldTextBuffer::SaveCursor () {
    savcurx = curx;
    savcury = cury;
    savcharx = charx;
    savchary = chary;
    savunderline = underline;
    savinverse = inverse;
    savAttributes = curAttributes;
    savCharSet = norCharSet;
}

void OldTextBuffer::RestoreCursor () {
    curx = savcurx;
    cury = savcury;
    charx = savcharx;
    chary = savchary;
    underline = savunderline;
    inverse = savinverse;
    curAttributes = savAttributes;
    norCharSet = savCharSet;
}
