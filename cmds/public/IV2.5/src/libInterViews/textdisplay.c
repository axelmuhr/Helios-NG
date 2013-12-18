/*
 * TextDisplay - basic text displaying
 */

#include <InterViews/font.h>
#include <InterViews/painter.h>
#include <InterViews/rubline.h>
#include <InterViews/shape.h>
#include <InterViews/textdisplay.h>
#include <bstring.h>
#include <string.h>

class TextLine {
public:
    TextLine();
    ~TextLine();

    void Style(TextDisplay*, int line, int first, int last, int style);
    void AddStyle(TextDisplay*, int line, int first, int last, int style);
    void RemoveStyle(TextDisplay*, int line, int first, int last, int style);

    void Insert(TextDisplay*, int line, int index, const char*, int count);
    void Delete(TextDisplay*, int line, int index, int count);
    void Replace(TextDisplay*, int line, const char*, int count);

    void Redraw(TextDisplay*, int line, Coord left, Coord right);
    void Draw(TextDisplay*, int line, int first, int last);

    int Index(TextDisplay*, Coord x);
    Coord Offset(TextDisplay*, int index);
private:
    void Size(int);
    char* text;
    char* attr;
    int size;
    int lastchar;
    char prefix;
    char postfix;
};

TextDisplay::TextDisplay () {
    painter = nil;
    canvas = nil;
    xmin = 0;
    xmax = 0;
    ymax = 0;
    ymin = 0;
    x0 = 0;
    y0 = 0;
    lineheight = 0;
    tabwidth = 0;
    firstline = 0;
    lastline = 0;
    topline = 0;
    bottomline = -1;
    lines = nil;
    maxlines = 0;
    Size(firstline, lastline);
    caret = nil;
    CaretStyle(DefaultCaret);
    Caret(0, 0);
}

TextDisplay::~TextDisplay () {
    for (int i = firstline; i <= lastline; ++i) {
        delete Line(i, false);
    }
    delete lines;
    if (caret != nil) {
        delete caret;
    }
}

void TextDisplay::Scroll (int line, Coord, Coord y) {
    HideCaret();
    if (canvas != nil) {
        while (y > ymin) {
            line += 1;
            y -= lineheight;
        }
        while (y < ymin) {
            line -= 1;
            y += lineheight;
        }
        int shift = y - Base(line);
        y0 = y - ymin;
        bottomline = line;
        topline = line - (ymax - y + 1) / lineheight + 1;
        if (shift > 0) {
            painter->Copy(
                canvas, xmin, ymin, xmax, ymax-shift, canvas, xmin, ymin+shift
            );
            Coord top = Base(topline) + lineheight - 1;
            if (top < ymax) {
                Redraw(painter, canvas, xmin, top+1, xmax, ymax);
            }
            Redraw(painter, canvas, xmin, ymin, xmax, ymin+shift-1);
        } else if (shift < 0) {
            painter->Copy(
                canvas, xmin, ymin-shift, xmax, ymax, canvas, xmin, ymin
            );
            Coord bottom = Base(bottomline);
            if (bottom > ymin) {
                Redraw(painter, canvas, xmin, ymin, xmax, bottom-1);
            }
            Redraw(painter, canvas, xmin, ymax+shift+1, xmax, ymax);
        }
    }
    ShowCaret();
    Caret(caretline, caretindex);
}
    
void TextDisplay::Resize (
    Coord xn, Coord yn, Coord xx, Coord yx, int h, int t
) {
    xmin = xn;
    ymin = yn;
    xmax = xx;
    ymax = yx;
    lineheight = h;
    tabwidth = t;
    bottomline = topline + (ymax - ymin + 1 - y0) / lineheight - 1;
    Caret(caretline, caretindex);
}

void TextDisplay::Redraw (
    Painter* p, Canvas* c, Coord l, Coord b, Coord r, Coord t
) {
    painter = p;
    canvas = c;
    if (canvas != nil) {
        int first = LineNumber(t);
        int last = LineNumber(b);
        painter->ClearRect(canvas, l, b, r, t);
        for (int i = first; i <= last; ++i) {
            TextLine* line = Line(i, false);
            if (line != nil) {
                line->Redraw(this, i, l, r);
            }
        }
        if (caret == nil) {
            CaretStyle(caretstyle);
        } else {
            caret->Redraw();
        }
    }
}

void TextDisplay::Size (int first, int last) {
    if (last - first >= maxlines) {
        int newmaxlines = last - first + 10;
        TextLine** newlines = new TextLine* [newmaxlines];
        bzero(newlines, newmaxlines * sizeof(TextLine*));
        bcopy(lines, newlines, maxlines * sizeof(TextLine*));
        delete lines;
        lines = newlines;
        maxlines = newmaxlines;
    }
    if (first < firstline) {
        int count = firstline-first;
        bcopy(lines, lines+count, (lastline-firstline+1) * sizeof(TextLine*));
        bzero(lines, count * sizeof(TextLine*));
    }
    firstline = first;
    lastline = last;
}

TextLine* TextDisplay::Line (int line, boolean create) {
    if (create) {
        Size(min(firstline, line), max(lastline, line));
    }
    if (line < firstline || line > lastline) {
        return nil;
    } else {
        TextLine* l = lines[Index(line)];
        Coord base = Base(line);
        if (l == nil && create) {
            l = new TextLine();
            lines[Index(line)] = l;
        }
        return l;
    }
}

int TextDisplay::Index (int line) {
    return line - firstline;
}

void TextDisplay::InsertLinesAfter (int line, int count) {
    Size(min(firstline, line), max(lastline, line) + count);
    bcopy(
        lines + Index(line + 1), lines + Index(line + count + 1),
        (lastline - line - count) * sizeof(TextLine*)
    );
    bzero(lines + Index(line+1), count * sizeof(TextLine*));
    if (canvas != nil) {
        HideCaret();
        Coord y = Base(line) - 1;
        int shift = count * lineheight;
        painter->Copy(
            canvas, xmin, ymin + shift, xmax, y, canvas, xmin, ymin
        );
        Coord bottom = Base(bottomline);
        if (bottom > ymin) {
            Redraw(painter, canvas, xmin, ymin, xmax, bottom-1);
        }
        Redraw(painter, canvas, xmin, y-shift+1, xmax, y);
        ShowCaret();
    }
}

void TextDisplay::InsertLinesBefore (int line, int count) {
    HideCaret();
    Size(min(firstline, line) - count, max(lastline, line));
    bcopy(
        lines + Index(line), lines + Index(line+count),
        (lastline - line - count + 1) * sizeof(TextLine*)
    );
    bzero(lines + Index(line - count), count * sizeof(TextLine*));
    if (canvas != nil) {
        Coord y = Top(line) + 1;
        int shift = count * lineheight;
        painter->Copy(
            canvas, xmin, y, xmax, ymax-shift, canvas, xmin, y+shift
        );
        Coord top = Base(topline) + lineheight - 1;
        if (top < xmax) {
            Redraw(painter, canvas, xmin, top, xmax, ymax);
        }
        Redraw(painter, canvas, xmin, y, xmax, y+shift-1);
    }
    ShowCaret();
}

void TextDisplay::DeleteLinesAfter (int line, int count) {
    HideCaret();
    Size(min(firstline, line), max(lastline, line));
    count = min(count, lastline - line);
    for (int i = 0; i < count; ++i) {
        delete Line(line+i+1, false);
    }
    bcopy(
        lines + Index(line + count + 1), lines + Index(line + 1),
        (lastline - line - count) * sizeof(TextLine*)
    );
    bzero(lines + Index(lastline - count), count * sizeof(TextLine*));
    if (canvas != nil) {
        Coord y = Base(line) - 1;
        int shift = count * lineheight;
        painter->Copy(
            canvas, xmin, ymin, xmax, y-shift, canvas, xmin, ymin+shift
        );
        Redraw(painter, canvas, xmin, ymin, xmax, ymin+shift-1);
    }
    ShowCaret();
}

void TextDisplay::DeleteLinesBefore (int line, int count) {
    HideCaret();
    Size(min(firstline, line), max(lastline, line));
    count = min(count, line - firstline);
    for (int i = 0; i < count; ++i) {
        delete Line(line-i, false);
    }
    bcopy(
        lines + Index(line+count), lines + Index(line+1),
        (lastline - line - count + 1) * sizeof(TextLine*)
    );
    bzero(lines + Index(lastline - count), count * sizeof(TextLine*));
    if (canvas != nil) {
        Coord y = Top(line) + 1;
        int shift = count * lineheight;
        painter->Copy(
            canvas, xmin, y+shift, xmax, ymax, canvas, xmin, y
        );
        Redraw(painter, canvas, xmin, ymax-shift+1, xmax, ymax);
    }
    ShowCaret();
}

void TextDisplay::InsertText (int l, int i, const char* t, int c) {
    HideCaret();
    Line(l, true)->Insert(this, l, i, t, c);
    ShowCaret();
}

void TextDisplay::DeleteText (int l, int i, int c) {
    HideCaret();
    Line(l, true)->Delete(this, l, i, c);
    ShowCaret();
}

void TextDisplay::ReplaceText (int l, const char* t, int c) {
    HideCaret();
    Line(l, true)->Replace(this, l, t, c);
    ShowCaret();
}

void TextDisplay::Style (int l1, int i1, int l2, int i2, int style) {
    HideCaret();
    for (int l = l1; l <= l2; ++l) {
        int first = (l == l1) ? i1 : -10000;
        int last = (l == l2) ? i2 : 10000;
        Line(l, true)->Style(this, l, first, last, style);
    }
    ShowCaret();
}

void TextDisplay::AddStyle (int l1, int i1, int l2, int i2, int style) {
    HideCaret();
    for (int l = l1; l <= l2; ++l) {
        int first = (l == l1) ? i1 : -10000;
        int last = (l == l2) ? i2 : 10000;
        Line(l, true)->AddStyle(this, l, first, last, style);
    }
    ShowCaret();
}

void TextDisplay::RemoveStyle (int l1, int i1, int l2, int i2, int style) {
    HideCaret();
    for (int l = l1; l <= l2; ++l) {
        int first = (l == l1) ? i1 : -10000;
        int last = (l == l2) ? i2 : 10000;
        Line(l, true)->RemoveStyle(this, l, first, last, style);
    }
    ShowCaret();
}

void TextDisplay::CaretStyle (int style) {
    HideCaret();
    caretstyle = style;
    if (canvas != nil) {
        Coord base = Base(caretline);
        Coord top = Top(caretline);
        Coord offset = Left(caretline, caretindex);
        if (caret != nil) {
            delete caret;
        }
        switch (caretstyle) {
        case NoCaret:
            caret = new Rubberband(painter, canvas, 0, 0);
            break;
        case DefaultCaret:
        case BarCaret:
            caret = new SlidingLine(
                painter, canvas,
                offset, base, offset, top, offset, base
            );
            break;
        }
    }
    ShowCaret();
}
        
void TextDisplay::Caret (int line, int index) {
    caretline = line;
    caretindex = index;
    if (caret != nil) {
        Coord left = Left(caretline, caretindex);
        Coord bottom = Base(caretline);
        Coord top = Top(caretline);
        if (left < xmin || left > xmax || bottom < ymin || top > ymax) {
            caret->Track(-1000, 0);
        } else {
            caret->Track(left, bottom);
        }
    }
}

void TextDisplay::HideCaret () {
    if (caret != nil) {
        caret->Erase();
    }
}

void TextDisplay::ShowCaret () {
    if (caret != nil) {
        caret->Draw();
    }
}

int TextDisplay::LineNumber (Coord y) {
    return bottomline - (y - ymin - y0)/lineheight;
}

int TextDisplay::LineIndex (int line, Coord x) {
    TextLine* l = Line(line, false);
    if (l == nil) {
        return 0;
    } else {
        return l->Index(this, x - xmin);
    }
}

Coord TextDisplay::Base (int line) {
    return ymin + y0 + (bottomline-line) * lineheight;
}

Coord TextDisplay::Top (int line) {
    return Base(line) + lineheight - 1;
}

Coord TextDisplay::Left (int line, int index) {
    TextLine* l = Line(line, false);
    if (l == nil) {
        return xmin;
    } else {
        return xmin + l->Offset(this, index);
    }
}

Coord TextDisplay::Right (int line, int index) {
    TextLine* l = Line(line, false);
    if (l == nil) {
        return xmin;
    } else {
        return xmin + l->Offset(this, index+1) - 1;
    }
}

TextLine::TextLine () {
    lastchar = 0;
    size = 0;
    text = nil;
    attr = nil;
    prefix = 0;
    postfix = 0;
    Replace(nil, 0, "", 0);
}

TextLine::~TextLine () {
    delete text;
    delete attr;
}

Coord TextLine::Offset (TextDisplay* display, int index) {
    if (display != nil && display->painter != nil) {
        Font* f = display->painter->GetFont();
        index = max(0, min(index, lastchar + 1));
        int w = 0;
        int i = 0;
        int cw;
        while (i < index) {
            if (text[i] == '\t') {
                if (display->tabwidth > 0) {
                    cw = display->tabwidth - w % display->tabwidth;
                } else {
                    cw = 0;
                }
            } else {
                cw = f->Width(text+i, 1);
            }
            w += cw;
            ++i;
        }
        return w;
    } else {
        return 0;
    }
}

int TextLine::Index (TextDisplay* display, Coord x) {
    if (display != nil && display->painter != nil) {
        Font* f = display->painter->GetFont();
        x = max(0, x);
        int i = 0;
        int w = 0;
        int cw = 0;
        while (i <= lastchar) {
            if (text[i] == '\t') {
                if (display->tabwidth > 0) {
                    cw = display->tabwidth - w % display->tabwidth;
                } else {
                    cw = 0;
                }
            } else {
                cw = f->Width(text+i, 1);
            }
            w += cw;
            if (w > x) {
                if (w - x < cw/2) {
                    ++i;
                }
                break;
            }
            ++i;
        }
        return i;
    } else {
        return 0;
    }
}

void TextLine::Size (int last) {
    if (last >= size) {
        int newsize = last<28 ? 28 : last<124 ? 124 : last<1020 ? 1020 : last;
        char* newtext = new char[newsize];
        bzero(newtext, newsize);
        bcopy(text, newtext, size);
        delete text;
        text = newtext;
        char* newattr = new char[newsize];
        bzero(newattr, newsize);
        bcopy(attr, newattr, size);
        delete attr;
        attr = newattr;
        size = newsize;
    }
}

void TextLine::Style (
    TextDisplay* display, int line, int first, int last, int style
) {
    if (first < 0) {
        prefix = style;
    }
    if (last > lastchar) {
        postfix = style;
    }
    int f = max(first, 0);
    int l = min(last, lastchar);
    for (int i = f; i <= l; ++i) {
        attr[i] = style;
    }
    Draw(display, line, first, last);
}

void TextLine::AddStyle (
    TextDisplay* display, int line, int first, int last, int style
) {
    if (first < 0) {
        prefix = prefix | style;
    }
    if (last > lastchar) {
        postfix = postfix | style;
    }
    int f = max(first, 0);
    int l = min(last, lastchar);
    for (int i = f; i <= l; ++i) {
        attr[i] = attr[i] | style;
    }
    Draw(display, line, first, last);
}

void TextLine::RemoveStyle (
    TextDisplay* display, int line, int first, int last, int st
) {
    if (first < 0) {
        prefix = prefix & ~st;
    }
    if (last > lastchar) {
        postfix = postfix & ~st;
    }
    int f = max(first, 0);
    int l = min(last, lastchar);
    for (int i = f; i <= l; ++i) {
        attr[i] = attr[i] & ~st;
    }
    Draw(display, line, first, last);
}

void TextLine::Insert (
    TextDisplay* display, int line, int index, const char* s, int count
) {
    Coord left, right;
    int shift;
    index = max(0, index);
    Size(max(index, lastchar) + count);
    int src = index;
    int dst = index + count;
    int len = max(0, lastchar - index + 1);
    lastchar += count;
    if (display != nil && display->canvas != nil) {
        left = display->Left(line, index);
        right = display->Right(line, lastchar+1);
    }
    bcopy(text + src, text + dst, len);
    bcopy(attr + src, attr + dst, len);
    bcopy(s, text + src, count);
    bzero(attr + src, count);
    if (display != nil && display->canvas != nil) {
        Font* f = display->painter->GetFont();
        if (strchr(text+index, '\t') != nil) {
            Draw(display, line, index, lastchar+1);
        } else {
            shift = display->Left(line, index+count) - left;
            right = min(right, display->xmax - shift);
            if (left <= right) {
                Coord bottom = display->Base(line);
                Coord top = bottom + f->Height() - 1;
                display->painter->Copy(
                    display->canvas, left, bottom, right, top,
                    display->canvas, left+shift, bottom
                );
            }
            Draw(display, line, index, index+count-1);
        }
    }
}

void TextLine::Delete (
    TextDisplay* display, int line, int index, int count
) {
    Coord left, right;
    int shift;
    Size(max(lastchar, index));
    count = max(0, min(count, lastchar-index+1));
    int src = index + count;
    int dst = index;
    int len = lastchar - (index + count) + 1;
    if (display != nil && display->canvas != nil) {
        left = display->Left(line, index + count);
        right = min(display->Right(line, lastchar + 1), display->xmax);
    }
    bcopy(text + src, text + dst, len);
    bcopy(attr + src, attr + dst, len);
    bzero(text + lastchar + 1 - count, count);
    bzero(attr + lastchar + 1 - count, count);
    lastchar -= count;
    if (display != nil && display->canvas != nil) {
        Font* f = display->painter->GetFont();
        if (strchr(text+index, '\t') != nil) {
            Draw(display, line, index, lastchar);
        } else {
            shift = left - display->Left(line, index);
            Coord bottom = display->Base(line);
            Coord top = bottom + f->Height() -1 ;
            if (left <= right) {
                display->painter->Copy(
                    display->canvas, left, bottom, right, top,
                    display->canvas, left-shift, bottom
                );
            }
            if (shift > 0) {
                Redraw(display, line, right-shift+1, right);
            }
        }
    }
}

void TextLine::Replace (
    TextDisplay* display, int line, const char* t, int c
) {
    Coord left, right;
    if (display != nil && display->canvas != nil) {
        left = display->Left(line, 0);
        right = min(display->Right(line, lastchar + 1), display->xmax);
        Font* f = display->painter->GetFont();
        Coord bottom = display->Base(line);
        Coord top = bottom + f->Height() - 1;
        if (left < right) {
            display->painter->ClearRect(
                display->canvas, left, bottom, right, top
            );
        }
    }
    delete text;
    text = nil;
    delete attr;
    attr = nil;
    size = 0;
    Size(c);
    lastchar = c - 1;
    bcopy(t, text, c);
    bzero(attr, c);
    Draw(display, line, -1, lastchar+1);
}

void TextLine::Redraw (
    TextDisplay* display, int line, Coord l, Coord r
) {
    if (display != nil && display->canvas != nil) {
        Draw(
            display, line,
            display->LineIndex(line, l), display->LineIndex(line, r)
        );
    }
}

void TextLine::Draw (
    TextDisplay* display, int line, int first, int last
) {
    if (display != nil && display->canvas != nil) {
        Font* f = display->painter->GetFont();
        Coord bottom = display->Base(line);
        Coord top = bottom + f->Height() - 1;
        if (line < display->topline || line > display->bottomline) {
            if (top > display->ymin && bottom < display->ymax) {
                display->painter->ClearRect(
                    display->canvas,
                    display->xmin, max(display->ymin, bottom),
                    display->xmax, min(display->ymax, top)
                );
            }
        } else {
            Coord left = display->Left(line, first);
            if (first < 0) {
                if (prefix & Reversed) {
                    display->painter->FillRect(
                        display->canvas, display->xmin, bottom, left, top
                    );
                } else {
                    display->painter->ClearRect(
                        display->canvas, display->xmin, bottom, left, top
                    );
                }
            }
            int start = max(0, first);
            int finish = min(lastchar, last);
            int done = start;
            display->painter->MoveTo(left, bottom);
            for (int i = start; i <= finish+1; ++i) {
                if (i==finish+1 || attr[i]!=attr[done] || text[i]=='\t') {
                    if (done != i && text[done] == '\t') {
                        Coord l, r, y;
                        display->painter->GetPosition(l, y);
                        r = display->Right(line, done);
                        if (attr[done] & Reversed) {
                            display->painter->FillRect(
                                display->canvas, l, bottom, r, top
                            );
                        } else {
                            display->painter->ClearRect(
                                display->canvas, l, bottom, r, top
                            );
                        }
                        display->painter->MoveTo(r+1, bottom);
                        ++done;
                    }
                    if (done != i) {
                        display->painter->SetStyle(attr[done]);
                        display->painter->Text(
                            display->canvas, text + done, i - done
                        );
                        done = i;
                    }
                }
            }
            display->painter->SetStyle(Plain);
            if (last > lastchar) {
                Coord r, y;
                display->painter->GetPosition(r, y);
                if (postfix & Reversed) {
                    display->painter->FillRect(
                        display->canvas, r, bottom, display->xmax, top
                    );
                } else {
                    display->painter->ClearRect(
                        display->canvas, r, bottom, display->xmax, top
                    );
                }
            }
        }
    }
}
