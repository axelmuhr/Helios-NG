/*
 * TextViewer - basic text buffer
 */

#include <InterViews/scene.h>
#include <InterViews/Text/textviewer.h>
#include <InterViews/shape.h>
#include <InterViews/perspective.h>
#include <InterViews/sensor.h>
#include <bstring.h>
#include <string.h>

static const float pagefract = 0.8;	    // fraction of page to scroll

void TPainter::StyledText (Canvas* c, const char* p, int len, StyleSet) {
    // styles not implemented yet
    Text(c, p, len);
}

TextLine::TextLine () {
    size = 0;
    length = 0;
    text = nil;
    attr = nil;
    above = nil;
    below = nil;
    touched = true;
}

TextLine::~TextLine () {
    delete text;
    delete attr;
}

void TextLine::Size (int s) {
    if (s <= size && size != 0) {
	return;
    }
    int newsize = s<8 ? 8 : s<32 ? 32 : s<128 ? 128 : s<1024 ? 1024 : s;
    char* newtext = new char[newsize];
    bzero(newtext, newsize);
    bcopy(text, newtext, min(size, newsize));
    char* newattr = new char[newsize];
    bzero(newattr, newsize);
    bcopy(attr, newattr, min(size, newsize));
    delete text;
    text = newtext;
    delete attr;
    attr = newattr;
    size = newsize;
}

void TextLine::Replace (int index, const char* s, int len, char a) {
    Size(index+len);
    for (int i = 0; i<len; ++i) {
	text[i+index] = s[i];
	attr[i+index] = a;
    }
    length = max(length, index+len);
    touched = true;
}    

void TextLine::Insert (int index, const char* s, int len, char a) {
    Size(length+len);
    bcopy(text+index, text+index+len, length-index);
    for (int i = 0; i<len; ++i) {
	text[i+index] = s[i];
	attr[i+index] = a;
    }
    length += len;
    touched = true;
}

void TextLine::Blank (int index, int len) {
    Size(index+len);
    for (int i = index; i<length && i<index+len; ++i) {
	text[i] = ' ';
	attr[i] = '\0';
    }
    length = max(length, index+len);
    touched = true;
}

void TextLine::Delete (int index, int len) {
    int l = min(len, length-index);
    bcopy(text+index+l, text+index, length-index-l);
    bcopy(attr+index+l, attr+index, length-index-l);
    length -= l;
    text[length] = '\0';
    attr[length] = '\0';
    touched = true;
}

void TextLine::EndLine (int index) {
    Size(index);
    length = index;
    text[length] = '\0';
    attr[length] = '\0';
    touched = true;
}

TextLine* TextLine::Split (int index) {
    TextLine* newline = new TextLine;
    newline->Size(length-index);
    bcopy(text+index, newline->text, length-index);
    bcopy(attr+index, newline->attr, length-index);
    newline->length = length-index;
    length = index;
    text[length] = '\0';
    attr[length] = '\0';
    touched = true;
    return newline;
}

void TextLine::Merge (TextLine* line) {
    Size(length+line->length);
    bcopy(line->text, text+length, line->length);
    bcopy(line->attr, attr+length, line->length);
    length += line->length;
    touched = true;
}

TextViewer::TextViewer (Painter* out, int cols, int rows) : (allEvents, out) {
    overwrite = true;
    buffer = true;
    viewcaret = true;
    caretstyle = None;

    painter = new TPainter(output);
    highlight = new TPainter(output);
    highlight->Reverse();

    shape->hunits = painter->GetFont()->Width("n");
    shape->vunits = painter->GetFont()->Height();
    shape->width = shape->hunits * cols;
    shape->height = shape->vunits * rows;

    perspective = new Perspective;
    perspective->Init(1, -1, cols, 1);
    perspective->sy = 1;
    perspective->ly = int(perspective->curheight * pagefract);
    perspective->sx = 1;
    perspective->lx = int(perspective->curwidth * pagefract);

    top = bottom = new TextLine;
    top->Insert(0, "", 0, 0);

    prow = perspective->y0 + perspective->height - 1;
    prev = top;

    row = prow;
    col = perspective->x0;
    dot = FindLine(row);

    crow = 0;
    ccol = 0;
    caret = nil;

    margin = 0;
}

TextViewer::~TextViewer () {
    delete painter;
    delete highlight;
    while (top != nil) {
	TextLine* doomed = top;
	top = top->below;
	delete doomed;
    }
}

Coord TextViewer::YPix (Coord y) {
    return
	ymax+1 - (perspective->cury+perspective->curheight-y) * shape->vunits;
}

Coord TextViewer::YChar (Coord y) {
    return
	perspective->cury + perspective->curheight-1 - (ymax-y)/shape->vunits;
}

Coord TextViewer::XPix (TextLine* line, Coord x) {
    Coord xx = x - perspective->x0;
    Font* f = painter->GetFont();
    Coord ww = (line->text != nil) ? f->Width(line->text, xx) : 0;
    return ww - (perspective->curx-perspective->x0) * shape->hunits;
}

Coord TextViewer::XChar (TextLine* line, Coord x) {
    Coord xx = x + (perspective->curx-perspective->x0) * shape->hunits;
    Font* f = painter->GetFont();
    Coord index = (line->text != nil) ?
	f->Index(line->text, line->length, xx, false) : 0;
    return index + perspective->x0 - 1;
}

void TextViewer::ToPix (Coord& x, Coord& y) {
    x = XPix(FindLine(y), x);
    y = YPix(y);
}

void TextViewer::ToChar (Coord& x, Coord& y) {
    y = YChar(y);
    x = XChar(FindLine(y), x);
}

TextLine* TextViewer::FindLine (Coord y) {	    // y in char coords
    if (y > prow) {
	while (y > prow && prev->above != nil) {
	    ++prow;
	    prev = prev->above;
	}
    } else if (y < prow) {
	while (y < prow && prev->below != nil) {
	    --prow;
	    prev = prev->below;
	}
    }
    return prev;
}

void TextViewer::Position (TextLine*& line, Coord& r, Coord& c) {
    r = max(r, perspective->y0);
    r = min(r, perspective->y0 + perspective->height - 1);
    while ((line = FindLine(r)) != top && line->size == 0) {
	++r;
    }
    c = max(c, perspective->x0);
    if (c > line->length+1) {
	c = line->length+1;
    }
}

void TextViewer::AddLine (TextLine* before, TextLine* newLine) {
    if (before == nil) {
	newLine->above = bottom;
	newLine->below = nil;
	newLine->above->below = newLine;
	bottom = newLine;
    } else if (before == top) {
	newLine->above = nil;
	newLine->below = before;
	newLine->below->above = newLine;
	top = newLine;
    } else {
	newLine->above = before->above;
	newLine->below = before;
	newLine->above->below = newLine;
	newLine->below->above = newLine;
    }
}

void TextViewer::AddLines (TextLine* before, int count) {
    if (count == 0) {
	return;
    }
    while (count > 0) {
	TextLine* newLine = new TextLine;
	newLine->EndLine(0);
	AddLine(before, newLine);
	if (
	    before != nil
	    && perspective->curheight == perspective->height
	    && bottom->size==0
	) {
	    RemoveLine(nil);
	} else {
	    ++perspective->height;
	    --perspective->y0;
	}
	--count;
    }
    prow = perspective->y0 + perspective->height - 1;
    prev = top;
    dot = FindLine(row);
}

void TextViewer::RemoveLine (TextLine* before) {
    TextLine* doomed = (before==nil) ? bottom : before->above;
    if (doomed == top) {
	top = doomed->below;
	doomed->below->above = nil;
    } else if (doomed == bottom) {
	doomed->above->below = nil;
	bottom = doomed->above;
    } else if (doomed != nil) {
	doomed->above->below = doomed->below;
	doomed->below->above = doomed->above;
    }
    delete doomed;
}

void TextViewer::RemoveLines (TextLine* before, int count) {
    if (count == 0) {
	return;
    }
    boolean draw = false;
    while (count > 0) {
	RemoveLine(before);
	--perspective->height;
	++perspective->y0;
	--count;
    }
    while (perspective->curheight > perspective->height) {
	AddLine(nil, new TextLine);
	++perspective->height;
	--perspective->y0;
	draw = true;
    }
    if (perspective->cury < perspective->y0) {
	perspective->cury = perspective->y0;
	draw = true;
    }
    prow = perspective->y0 + perspective->height - 1;
    prev = top;
    dot = FindLine(row);
    if (draw) {
	Draw();
    }
}

void TextViewer::DrawLine(TextLine* line, 
    Coord baseline, Coord first, Coord last
) {
    if (canvas == nil) {
	return;
    }
    Coord f = max(first, perspective->x0);
    Coord l = min(last, line->length-1+perspective->x0);
    if (last >= line->length-1+perspective->x0) {
	if (
	    line->size > 0 && line->length > 0 &&
	    StyleSet(line->attr[line->length-1]).Includes(Reversed)
	) {
	    highlight->ClearRect(canvas, 
		XPix(line, l+1), baseline, xmax, baseline+shape->vunits-1
	    );
	} else {
	    painter->ClearRect(canvas, 
		XPix(line, l+1), baseline, xmax, baseline+shape->vunits-1
	    );
	}
    }
    if (line->size == 0 || l<f) {
	return;
    }
    char* cp = line->text+f-perspective->x0;
    char* ap = line->attr+f-perspective->x0;
    char* start = cp;
    char* finish = line->text+l-perspective->x0;
    char a = *ap;
    for (;;) {
	if (*ap != a || cp > finish) {
	    TPainter* p;
	    if (StyleSet(a).Includes (Reversed)) {
		p = highlight;
	    } else {
		p = painter;
	    }
	    p->MoveTo(XPix(line, start-line->text+perspective->x0), baseline);
	    p->StyledText(canvas, start, cp-start, StyleSet(a));
	    if (cp>finish) {
		break;
	    } else {
		start = cp;
		a = *ap;
	    }
	} else {
	    ++cp; ++ap;
	}
    }
}

void TextViewer::FlushLine (TextLine* line, Coord baseline) {
    if (line->touched) {
	DrawLine(line, baseline, XChar(line, 0), XChar(line, xmax));
	line->touched = false;
    }
}

void TextViewer::HideCaret () {
    if (caret == nil) {
	return;
    }
    DrawLine(caret, YPix(crow), ccol, ccol);
}

void TextViewer::ShowCaret () {
    if (caret == nil || canvas == nil) {
	return;
    }
    Coord caretx = XPix(caret, ccol);
    Coord carety = YPix(crow);

    char c;
    StyleSet s;
    if (caret->text != nil && ccol <= caret->length-1+perspective->x0) {
	c = caret->text[ccol-perspective->x0];
	s = StyleSet(caret->attr[ccol-perspective->x0]);
    } else {
	c = ' ';
    }
    int width = painter->GetFont()->Width(&c, 1);
    int height = shape->vunits;

    Painter* p, *hl;
    if (s.Includes(Reversed)) {
	p = highlight; hl = painter;
    } else {
	p = painter; hl = highlight;
    }
    switch (caretstyle) {
    case None:
	break;
    case Bar:
	p->Line(canvas, caretx, carety, caretx, carety+height-1);
	break;
    case Block:
	hl->MoveTo(caretx, carety);
	hl->Text(canvas, &c, 1	);
	break;
    case Outline:
	p->Rect(canvas, caretx, carety, caretx+width-1, carety+height-1);
	break;
    case Underline:
	p->Rect(canvas, caretx, carety, caretx+width-1, carety+1);
	break;
    default:
	break;
    }
}

void TextViewer::BringToView (Coord l, Coord c) {
    boolean draw = false;
    if (l < perspective->cury) {
	perspective->cury = max(l-perspective->curheight/2, perspective->y0);
	draw = true;
    }
    if (l > perspective->cury + perspective->curheight - 1) {
	perspective->cury = min(
	    l-perspective->curheight/2, 
	    perspective->y0+perspective->height-perspective->curheight
	);
	draw = true;
    }
    Coord xx = XPix(FindLine(l), c);
    if (xx < shape->hunits && perspective->curx > perspective->x0) {
	perspective->curx = max(
	    perspective->curx + (xx-xmax/2)/shape->hunits, 
	    perspective->x0
	);
	draw = true;
    }
    if (xx > xmax) {
	perspective->curx = min(
	    perspective->curx + (xx-xmax/2)/shape->hunits, 
	    perspective->x0+perspective->width-perspective->curwidth
	);
	draw = true;
    }
    if (draw) {
	Draw();
    }
}

void TextViewer::Draw () {
    perspective->Update();
    Redraw(0, 0, xmax, ymax);
}

void TextViewer::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    HideCaret();
    if (x1==x2 || y1==y2) {
	return;
    }
    if (y1 < (ymax+1) % shape->vunits) {
	y1 = (ymax+1) % shape->vunits;
    }
    TextLine* begin = FindLine(YChar(y1));
    TextLine* end = FindLine(YChar(y2))->above;
    Coord baseline = YPix(YChar(y1));
    for (TextLine* t = begin; t != end; t = t->above) {
	DrawLine(t, baseline, XChar(t, x1), XChar(t, x2)+1);
	baseline += shape->vunits;
    }
    ShowCaret();
}

void TextViewer::Handle (Event&) {
}

void TextViewer::Resize () {
    Coord rows = (ymax+1)/shape->vunits;
    perspective->curheight = rows;
    if (perspective->cury > -rows) {
	perspective->cury = -rows;
    }
    if (perspective->height < rows) {
	while (rows > perspective->height) {
	    AddLine(nil, new TextLine);
	    ++perspective->height;
	}
	perspective->y0 = -rows;
	perspective->cury = -rows;
    } else if (perspective->cury == perspective->y0) {
	while (perspective->height > rows && bottom->size==0) {
	    RemoveLine(nil);
	    --perspective->height;
	    ++perspective->y0;
	    ++perspective->cury;
	}
    }
    Coord cols = (xmax+1)/shape->hunits;
    Coord width = 0;
    for (TextLine* l = top; l != nil; l = l->below) {
	if (l->text != nil) {
	    width = max(
		width, 
		painter->GetFont()->Width(l->text, l->length)
	    );
	}
    }
    perspective->curwidth = cols;
    perspective->width = width/shape->hunits;

    if (perspective->curx+cols > perspective->width+1) {
	perspective->curx = perspective->width + 1 - cols;
    }
    if (perspective->width < cols) {
	perspective->width = cols;
	perspective->x0 = 1;
	perspective->curx = 1;
    }
    perspective->ly = int(perspective->curheight * pagefract);
    perspective->lx = int(perspective->curwidth * pagefract);
    dot = FindLine(row);
}

void TextViewer::Adjust (Perspective& p) {
    int mincury = perspective->y0;
    int maxcury = perspective->y0+perspective->height-perspective->curheight;
    perspective->cury = max(mincury, min(p.cury, maxcury));
    perspective->curx = p.curx;
    Draw();
}

void TextViewer::Reshape (Shape& s) {
    *shape = s;
    if (Parent() != nil) {
	Parent()->Change(this);
    }
}

void TextViewer::NoCaret () {
    HideCaret();
    caret = nil;
    crow = 0;
    ccol = 0;
}

void TextViewer::Caret () {
    HideCaret();
    crow = row;
    ccol = col;
    caret = dot;
    if (viewcaret) {
	BringToView(crow, ccol);
    }
    ShowCaret();
}

void TextViewer::Caret (Coord r, Coord c) {
    HideCaret();
    crow = -r;
    ccol = c;
    Position(caret, crow, ccol);
    if (viewcaret) {
	BringToView(crow, ccol);
    }
    ShowCaret();
}

void TextViewer::View () {
    BringToView(-row, col);
}

void TextViewer::View (Coord r, Coord c) {
    BringToView(-r, c);
}

void TextViewer::GoTo (Coord r, Coord c) {
    row = -r;
    col = c;
    Position(dot, row, col);
}

void TextViewer::GetPos (Coord& r, Coord& c) {
    r = -row;
    c = col;
}

void TextViewer::Margin (int m) {
    margin = m;
}

void TextViewer::Indent (int i) {
    margin += i;
    if (margin < 0) {
	margin = 0;
    }
}

void TextViewer::Insert (int rows, int cols) {
    boolean save = overwrite;
    overwrite = false;
    Coord newcol = col + cols;
    Coord newrow = row - rows;
    if (newcol < perspective->x0) {
	newcol = perspective->x0;
    }
    Coord toprow = perspective->y0 + perspective->height - 1;
    if (newrow > toprow) {
	newrow = toprow;
	newcol = perspective->x0;
    }
    if (newrow < row) {
	TextLine* splitline = dot->Split(col-perspective->x0);
	int shift = (row-newrow) * shape->vunits;
	if (canvas != nil) {
	    Coord yy = YPix(row);
	    output->Copy(canvas, 0, 0, xmax, yy - 1, canvas, 0, -shift);
	    output->ClearRect(canvas, 0, yy - shift, xmax, yy - 1);
	}
	AddLines(dot->below, row-newrow);
	GoTo(-newrow, perspective->x0);
	dot->Merge(splitline);
	delete splitline;
    } else {
	TextLine* splitline = dot->Split(col-perspective->x0);
	int shift = (newrow-row) * shape->vunits;
	if (canvas != nil) {
	    Coord yy = YPix(row);
	    output->Copy(canvas, 0, -shift, xmax, yy - 1, canvas, 0, 0);
	    Redraw(0, 0, xmax, shift - 1);
	}
	RemoveLines(dot->below, newrow-row);
	GoTo(-newrow, perspective->x0);
	GoTo(-newrow, dot->length - perspective->x0 + 2);
	dot->Merge(splitline);
	delete splitline;
    }
    if (newcol > col) {
	Spaces(newcol - col);
    } else if (newcol < col) {
	dot->Delete(newcol-perspective->x0, col-newcol);
    }
    if (!buffer) {
	Flush();
    }
    overwrite = save;
}

void TextViewer::String (const char* s, int len) {
    if (overwrite) {
	dot->Replace(col-1, s, len, style);
    } else {
	dot->Insert(col-1, s, len, style);
    }
    Coord w = painter->GetFont()->Width(dot->text, dot->length);
    if (w > perspective->width* shape->hunits) {
	perspective->width = w / shape->hunits + 1;
    }
    if (!buffer) {
	perspective->Update();
	FlushLine(dot, YPix(row));
    }
    GoTo(-row, col+len);
}

void TextViewer::String (const char* s) {
    String(s, strlen(s));
}

void TextViewer::NewLine () {
    if (!overwrite) {
	Insert(1, - (col-perspective->x0));
    } else {
	EndLine();
	if (dot == bottom || dot->below->size == 0) {
	    Insert(1, - (col-perspective->x0));
	} else {
	    GoTo(-row+1, perspective->x0);
	}
    }
    StyleSet oldstyle = style;
    style = StyleSet();
    Spaces(margin);
    style = oldstyle;
}

void TextViewer::Rubout (int count) {
    if (overwrite) {
	dot->Blank(col-1, count);
    } else {
	dot->Delete(col-1, count);
    }
    if (!buffer) {
	FlushLine(dot, YPix(row));
    }
}

void TextViewer::Tab (int spacing) {
    int newcol = ((col-perspective->x0)/spacing + 1) * spacing + 1;
    Spaces(newcol - col);
}

void TextViewer::Spaces (int count) {
    static const char* spaces = "                              ";
    int l = strlen(spaces);
    while (count/l > 0) {
	String(spaces, l);
	count -= l;
    }
    if (count > 0) {
	String(spaces, count);
    }
}

void TextViewer::EndLine () {
    dot->EndLine(col - perspective->x0);
}

void TextViewer::EndText () {
    EndLine();
    TextLine* l = dot;
    int count = 0;
    while (l->below != nil && l->below->size > 0) {
	++count;
	l = l->below;
    }
    RemoveLines(l->below, count);
}

void TextViewer::Flush () {
    perspective->Update();
    HideCaret();
    TextLine* begin = FindLine(perspective->cury);
    TextLine* end = 
	FindLine(perspective->cury+perspective->curheight-1)->above;
    Coord baseline = YPix(perspective->cury);
    for (TextLine* t = begin; t != end; t = t->above) {
	FlushLine(t, baseline);
	baseline += shape->vunits;
    }
    ShowCaret();
}
