/*
 * TextEditor - basic interactive editor for mulit-line text
 */

#include <InterViews/font.h>
#include <InterViews/painter.h>
#include <InterViews/perspective.h>
#include <InterViews/shape.h>
#include <InterViews/textbuffer.h>
#include <InterViews/textdisplay.h>
#include <InterViews/texteditor.h>
#include <InterViews/transformer.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>

TextEditor::TextEditor (int r, int c, int t, int h) {
    Init(r, c, t, h);
}

TextEditor::TextEditor (const char* name, int r, int c, int t, int h) {
    SetInstance(name);
    Init(r, c, t, h);
}

void TextEditor::Init (int r, int c, int t, int h) {
    SetClassName("TextEditor");
    text = nil;
    dot = 0;
    mark = 0;
    shaperows = r;
    shapecolumns = c;
    lineheight = 1;
    tabsize = t;
    highlight = h;
    display = nil;
    perspective = new Perspective();
}

TextEditor::~TextEditor () {
    delete display;
    delete perspective;
}

void TextEditor::Reconfig () {
    Font* f = output->GetFont();
    shape->hunits = f->Width("n");
    shape->vunits = f->Height();
    shape->Rect(shape->hunits*shapecolumns, shape->vunits*shaperows);
    shape->Rigid(hfil, hfil, vfil, vfil);
    lineheight = shape->vunits;
}

void TextEditor::Resize () {
    display->Resize(0, 0, xmax, ymax, lineheight, tabsize * shape->hunits);
    int topmargin = (
        perspective->height - perspective->curheight - perspective->cury
    );
    perspective->sy = lineheight;
    perspective->ly = ymax + 1;
    perspective->height = text->Height() * lineheight;
    perspective->cury = perspective->height - topmargin - (ymax+1);
    perspective->curheight = (ymax+1);
    perspective->Update();
}

void TextEditor::Redraw (Coord l, Coord b, Coord r, Coord t) {
    Transformer tr(output->GetTransformer());
    Coord x1, y1, x2, y2, x3, y3, x4, y4;
    tr.InvTransform(l, b, x1, y1);
    tr.InvTransform(l, t, x2, y2);
    tr.InvTransform(r, t, x3, y3);
    tr.InvTransform(r, b, x4, y4);
    display->Redraw(
        output, canvas,
        min(x1, min(x2, min(x3, x4))), min(y1, min(y2, min(y3, y4))),
        max(x1, max(x2, max(x3, x4))), max(y1, max(y2, max(y3, y4)))
    );
}

void TextEditor::Adjust (Perspective& np) {
    float scale = float(np.height) / float(perspective->height);
    ScrollTo(0, perspective->y0 + int((np.cury - np.y0) * scale));
}

void TextEditor::Edit (TextBuffer* t) {
    delete display;
    display = new TextDisplay();
    text = t;
    int lines = text->Height();
    for (int i = 0; i < lines; ++i) {
        int bol = text->LineIndex(i);
        int eol = text->EndOfLine(bol);
        display->ReplaceText(i, text->Text(bol, eol), eol - bol);
    }
    if (canvas != nil) {
        Resize();
        output->ClearRect(canvas, 0, 0, xmax, ymax);
        display->Redraw(output, canvas, 0, 0, xmax, ymax);
    }
}

void TextEditor::InsertText (const char* s, int count) {
    count = text->Insert(dot, s, count);
    int sline = text->LineNumber(dot);
    int fline = text->LineNumber(dot + count);
    if (sline == fline) {
        int offset = text->LineOffset(dot);
        display->InsertText(sline, offset, text->Text(dot), count);
    } else {
        display->InsertLinesAfter(sline, fline-sline);
        for (int i = sline; i <= fline; ++i) {
            int bol = text->BeginningOfLine(text->LineIndex(i));
            int eol = text->EndOfLine(bol);
            display->ReplaceText(i, text->Text(bol, eol), eol-bol);
        }
        if (canvas != nil) {
            int dh = lineheight * (fline-sline);
            perspective->height += dh;
            perspective->cury += dh;
            perspective->Update();
        }
    }
    Select(dot + count);
}

void TextEditor::DeleteText (int count) {
    int d = dot;
    int c = count;
    while (c > 0) {
        d = text->NextCharacter(d);
        --c;
    }
    while (c < 0) {
        dot = text->PreviousCharacter(dot);
        ++c;
    }
    count = d - dot;
    int sline = text->LineNumber(dot);
    int fline = text->LineNumber(d);
    text->Delete(dot, count);
    if (sline == fline) {
        int offset = text->LineOffset(dot);
        display->DeleteText(sline, offset, count);
    } else {
        int bol = text->BeginningOfLine(dot);
        int eol = text->EndOfLine(dot);
        display->DeleteLinesAfter(sline, fline-sline);
        display->ReplaceText(sline, text->Text(bol, eol), eol-bol);
        if (canvas != nil) {
            int dh = lineheight * (fline-sline);
            perspective->height -= dh;
            perspective->cury -= dh;
            perspective->Update();
        }
    }
    Select(dot);
}

void TextEditor::DeleteSelection () {
    if (mark != dot) {
        DeleteText(mark - dot);
    }
}

void TextEditor::BeginningOfSelection () {
    Select(min(mark, dot));
}

void TextEditor::EndOfSelection () {
    Select(max(mark, dot));
}

void TextEditor::BeginningOfWord () {
    if (dot != mark) {
        Select(min(mark, dot));
    } else {
        Select(text->BeginningOfWord(dot));
    }
}

void TextEditor::EndOfWord () {
    if (dot != mark) {
        Select(max(mark, dot));
    } else {
        Select(text->EndOfWord(dot));
    }
}

void TextEditor::BeginningOfLine () {
    if (dot != mark) {
        Select(min(mark, dot));
    } else {
        Select(text->BeginningOfLine(dot));
    }
}

void TextEditor::EndOfLine () {
    if (dot != mark) {
        Select(max(mark, dot));
    } else {
        Select(text->EndOfLine(dot));
    }
}

void TextEditor::BeginningOfText() {
    Select(text->BeginningOfText());
}

void TextEditor::EndOfText() {
    Select(text->EndOfText());
}

void TextEditor::ForwardCharacter (int count) {
    if (mark != dot) {
        Select(max(mark, dot));
    } else {
        int d = dot;
        while (count > 0) {
            d = text->NextCharacter(d);
            --count;
        }
        Select(d);
    }
}

void TextEditor::BackwardCharacter (int count) {
    if (dot != mark) {
        Select(min(mark, dot));
    } else {
        int d = dot;
        while (count > 0) {
            d = text->PreviousCharacter(d);
            --count;
        }
        Select(d);
    }
}

void TextEditor::ForwardLine (int count) {
    if (dot != mark) {
        Select(max(mark, dot));
    } else {
        int d = dot;
        while (count > 0) {
            d = text->BeginningOfNextLine(d);
            --count;
        }
        Select(d);
    }
}

void TextEditor::BackwardLine (int count) {
    if (dot != mark) {
        Select(min(mark, dot));
    } else {
        int d = dot;
        while (count > 0) {
            d = text->BeginningOfLine(text->EndOfPreviousLine(d));
            --count;
        }
        Select(d);
    }
}

void TextEditor::ForwardWord (int count) {
    if (dot != mark) {
        Select(max(mark, dot));
    } else {
        int d = dot;
        while (count > 0) {
            d = text->BeginningOfNextWord(d);
            --count;
        }
        Select(d);
    }
}

void TextEditor::BackwardWord (int count) {
    if (dot != mark) {
        Select(min(mark, dot));
    } else {
        int d = dot;
        while (count > 0) {
            d = text->BeginningOfWord(text->EndOfPreviousWord(d));
            --count;
        }
        Select(d);
    }
}

void TextEditor::ForwardPage (int count) {
    int pagesize = perspective->curheight / perspective->sy;
    ForwardLine(pagesize * count);
}

void TextEditor::BackwardPage (int count) {
    int pagesize = perspective->curheight / perspective->sy;
    BackwardLine(pagesize * count);
}

void TextEditor::ScrollToSelection () {
    int y = perspective->height - (text->LineNumber(dot)+1) * lineheight;
    if (
        y > perspective->cury + perspective->curheight - lineheight
        || y < perspective->cury
    ) {
        ScrollTo(0, y - perspective->curheight/2);
    }
}

void TextEditor::ScrollToView (Coord x, Coord y) {
    Transformer tr(output->GetTransformer());
    tr.InvTransform(x, y);
    if (y > ymax) {
        ScrollTo(0, perspective->y0 + perspective->cury - (ymax-y));
    } else if (y < 0) {
        ScrollTo(0, perspective->y0 + perspective->cury - (-y));
    }
}

void TextEditor::ScrollBy (int, int dy) {
    ScrollTo(0, perspective->cury + dy);
}

void TextEditor::ScrollTo (int x, int y) {
    int maxy = perspective->height - perspective->curheight;
    int miny = min(maxy, -perspective->curheight/2);
    perspective->cury = max(miny, min(y, maxy));
    perspective->Update();
    int topmargin = perspective->height - perspective->cury;
    int lines = topmargin / lineheight;
    display->Scroll(lines-1, x, topmargin - lines*lineheight);
}

void TextEditor::Select (int d) {
    Select(d, d);
}

void TextEditor::SelectMore (int m) {
    Select(dot, m);
}

void TextEditor::SelectAll () {
    Select(text->EndOfText(), text->BeginningOfText());
}

void TextEditor::Select (int d, int m) {
    int oldl = min(dot, mark);
    int oldr = max(dot, mark);
    int newl = min(d, m);
    int newr = max(d, m);
    if (oldl == oldr && newl != newr) {
        display->CaretStyle(NoCaret);
    }
    if (newr < oldl || newl > oldr) {
        if (oldr > oldl) {
            display->RemoveStyle(
                text->LineNumber(oldl), text->LineOffset(oldl),
                text->LineNumber(oldr-1), text->LineOffset(oldr-1),
                highlight
            );
        }
        if (newr > newl) {
            display->AddStyle(
                text->LineNumber(newl), text->LineOffset(newl),
                text->LineNumber(newr-1), text->LineOffset(newr-1),
                highlight
            );
        }
    } else {
        if (newl < oldl) {
            display->AddStyle(
                text->LineNumber(newl), text->LineOffset(newl),
                text->LineNumber(oldl-1), text->LineOffset(oldl-1),
                highlight
            );
        } else if (newl > oldl) {
            display->RemoveStyle(
                text->LineNumber(oldl), text->LineOffset(oldl),
                text->LineNumber(newl-1), text->LineOffset(newl-1),
                highlight
            );
        }
        if (newr > oldr) {
            display->AddStyle(
                text->LineNumber(oldr), text->LineOffset(oldr),
                text->LineNumber(newr-1), text->LineOffset(newr-1),
                highlight
            );
        } else if (newr < oldr) {
            display->RemoveStyle(
                text->LineNumber(newr), text->LineOffset(newr),
                text->LineNumber(oldr-1), text->LineOffset(oldr-1),
                highlight
            );
        }
    }
    if (oldl != oldr && newl == newr) {
        display->CaretStyle(BarCaret);
    }
    dot = d;
    mark = m;
    if (dot == mark) {
        display->Caret(text->LineNumber(dot), text->LineOffset(dot));
    }
}

int TextEditor::Locate (Coord x, Coord y) {
    Transformer tr(output->GetTransformer());
    tr.InvTransform(x, y);
    int line = display->LineNumber(y);
    int index = display->LineIndex(line, x);
    int l = text->LineIndex(line);
    int i = 0;
    while (i < index) {
        l = text->NextCharacter(l);
        i += 1;
    }
    return l;
}
