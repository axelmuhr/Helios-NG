/*
 * StringEditor - interactive editor for character strings
 */

#include <InterViews/button.h>
#include <InterViews/font.h>
#include <InterViews/painter.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <InterViews/stringeditor.h>
#include <InterViews/textbuffer.h>
#include <InterViews/textdisplay.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static const int BUFFERSIZE = 1000;

StringEditor::StringEditor (ButtonState* s, int a, int c, const char* samp) {
    Init(s, a, c, samp);
}

StringEditor::StringEditor (
    const char* name, ButtonState* s, int a, int c, const char* samp
) {
    SetInstance(name);
    Init(s, a, c, samp);
}

void StringEditor::Init (ButtonState* s, int a, int c, const char* samp) {
    SetClassName("StringEditor");
    sample = samp;
    size = BUFFERSIZE;
    buffer = new char[size];
    text = new TextBuffer(buffer, 0, size);
    left = 0;
    right = 0;
    subject = s;
    if (subject != nil) {
        subject->Attach(this);
    }
    accept = a;
    cancel = c;
    display = new TextDisplay();
    display->CaretStyle(NoCaret);
    input = new Sensor();
    input->Catch(KeyEvent);
    input->Catch(DownEvent);
    Message(sample);
}

StringEditor::~StringEditor () {
    if (subject != nil) {
        subject->Detach(this);
    }
    delete text;
    delete buffer;
    delete display;
}

void StringEditor::Reconfig () {
    Font* f = output->GetFont();
    shape->Rect(f->Width(sample), f->Height());
    shape->Rigid(hfil, hfil, 0, 0);
}

void StringEditor::Resize () {
    display->Resize(0, 0, xmax, ymax, output->GetFont()->Height(), 0);
}

void StringEditor::Redraw (Coord l, Coord b, Coord r, Coord t) {
    display->Redraw(output, canvas, l, b, r, t);
}

void StringEditor::Message (const char* t) {
    text->Delete(text->BeginningOfText(), text->Length());
    text->Insert(0, t, strlen(t));
    int bol = text->BeginningOfLine(0);
    int eol = text->EndOfLine(0);
    display->ReplaceText(0, text->Text(bol, eol), eol - bol);
    Select(eol);
}

void StringEditor::Select (int l) {
    Select(l, l);
}

void StringEditor::Select (int l, int r) {
    if (r < l) {
        int tmp = r;
        r = l;
        l = tmp;
    }
    l = max(l, text->BeginningOfLine(left));
    r = min(r, text->EndOfLine(right));
    if (r < left || l > right) {
        if (right > left) {
            display->Style(0, left, 0, right-1, Plain);
        }
        if (r > l) {
            display->Style(0, l, 0, r-1, Reversed);
        }
    } else {
        if (l < left) {
            display->Style(0, l, 0, left-1, Reversed);
        } else if (l > left) {
            display->Style(0, left, 0, l-1, Plain);
        }
        if (r > right) {
            display->Style(0, right, 0, r-1, Reversed);
        } else if (r < right) {
            display->Style(0, r, 0, right-1, Plain);
        }
    }
    left = l;
    right = r;
    if (left == right) {
        display->Caret(0, left);
    } else {
        display->Caret(-1, 0);
    }
}

void StringEditor::Edit () {
    Event e;
    Handle(e);
}

void StringEditor::Edit (const char* t, int l, int r) {
    Message(t);
    Select(l, r);
    Edit();
}

const char* StringEditor::Text () {
    return text->Text();
}

boolean StringEditor::HandleChar (char c) {
    boolean done = false;
    switch (c) {
      case SEBeginningOfLine:
        Select(text->BeginningOfLine(left));
        break;
      case SEEndOfLine:
        Select(text->EndOfLine(right));
        break;
      case SESelectAll:
        Select(text->BeginningOfText(), text->EndOfText());
        break;
      case SESelectWord:
        Select(text->BeginningOfWord(text->PreviousCharacter(left)), right);
        break;
      case SEPreviousCharacter:
        Select(text->PreviousCharacter(left));
        break;
      case SENectCharacter:
        Select(text->NextCharacter(right));
        break;
      case SEDeleteNextCharacter:
        if (left == right) {
            right = text->NextCharacter(right);
        }
        text->Delete(left, right-left);
        display->DeleteText(0, left, right-left);
        Select(left);
        break;
      case SEDeletePreviousCharacter:
      case SEDeletePreviousCharacterAlt:
        if (left == right) {
            left = text->PreviousCharacter(left);
        }
        text->Delete(left, right-left);
        display->DeleteText(0, left, right-left);
        Select(left);
        break;
      case SECancelEdit:
        done = true;
        if (subject != nil) {
            subject->SetValue(cancel);
        }
        break;
      case SEAcceptEdit:
        if (subject != nil) {
            subject->SetValue(accept);
        }
        done = true;
        break;
      default:
        if (!iscntrl(c)) {
            InsertText(&c, 1);
        }
        break;
    }
    return done;
}

void StringEditor::InsertText (const char* t, int len) {
    if (left != right) {
        text->Delete(left, right-left);
        display->DeleteText(0, left, right-left);
    }
    text->Insert(left, t, len);
    display->InsertText(0, left, t, len);
    int l = left;
    while (len > 0) {
        l = text->NextCharacter(l);
        --len;
    }
    Select(l);
}

void StringEditor::Handle (Event& e) {
    boolean done = false;
    display->CaretStyle(BarCaret);
    do {
        switch (e.eventType) {
        case KeyEvent:
            if (e.len != 0) {
                done = HandleChar(e.keystring[0]);
            }
            break;
        case DownEvent:
            if (e.target == this) {
                if (e.button == LEFTMOUSE) {
                    int start = display->LineIndex(0, e.x);
                    do {
                        Select(start, display->LineIndex(0, e.x));
                        Poll(e);
                    } while (e.leftmouse);
                }
            } else {
                UnRead(e);
                done = true;
            }
            break;
        }
        if (!done) {
            Read(e);
        }
    } while (!done);
    display->CaretStyle(NoCaret);
}
