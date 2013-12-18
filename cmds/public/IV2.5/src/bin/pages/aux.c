/*
 * auxilary functions for pages
 */

#include <stdio.h>
#include "stack.h"
#include "script.h"
#include <InterViews/Text/layout.h>
#include <InterViews/Text/textpainter.h>
#include <InterViews/paint.h>

extern class Painter* stdpaint;

static const void* BEGINMARKER = (void*) -1;
static const void* ENDMARKER = (void*) -2;

static const char CONTEXT = '!';

static const char NL = '<';
static const char SPACE = '^';
static const char WHITESPACE = '+';
static const char EMPTY = '_';
static const char COMPOSITION = '*';
static const char PAR = '~';
static const char SENTENCE = '.';
static const char BLOCK = '#';
static const char TEXTLIST = ';';
static const char BEGIN = '{';
static const char END = '}';
static const char STYLE = '\\';
static const char BOLD = 'b';
static const char UNDERLINED = 'u';
static const char INVERTED = 'i';
static const char GREYED = 'g';

TextPainter* bold;
TextPainter* underlined;
TextPainter* shaded;
TextPainter* inverted;
TextPainter* greyed;

void* tmp;

Composition* Construct(Stack* state, Composition* c) {
    Text* t;
    state->Pop(tmp);
    t = (Text*)tmp;
    if (t == ENDMARKER) {
	state->Pop(tmp);
        t = (Text*)tmp;
	while (t != BEGINMARKER) {
	    c->Prepend(t);
	    state->Pop(tmp);
            t = (Text*)tmp;
	}
    } else {
	c->Prepend(t);
    }
    return c;
}

Text* Build(FILE* file, Layout* layout) {
    bold = new TextPainter(stdpaint);
    bold->Bold();
    underlined = new TextPainter(stdpaint);
    underlined->Underlined();
    shaded = new TextPainter(stdpaint);
    shaded->SetColors(black, new Color("Turquoise"));
    inverted = new TextPainter(stdpaint);
    inverted->Inverted();
    greyed = new TextPainter(stdpaint);
    greyed->SetColors(new Color("LightGrey"), white);

    Stack* state = new Stack;
    FileScript script(file);
    int left;
    int right;
    int indent;
    int size;
    void* top;
    Text* context;
    Text* prefix;
    Text* postfix;
    Text* keeper;
    Text* separator;
    while (script.Next()) {
	switch(script.Op()) {
	case NUMBER :
	    int i = script.Number();
	    state->Push(i);
	    break;
	case NL :
	    state->Push(new LineBreak(nil));
	    break;
	case SPACE :
	    state->Push(new Whitespace(1, nil));
	    break;
	case WHITESPACE :
	    state->Pop(size);
	    state->Push(new Whitespace(size, nil));
	    break;
	case EMPTY :
	    state->Push(new Whitespace(0, nil));
	    break;
	case COMPOSITION :
	    state->Push(Construct(state, new Composition(nil)));
	    break;
	case PAR :
	    state->Pop(right);
	    state->Pop(left);
	    state->Pop(tmp);
            prefix = (Text*)tmp;
	    state->Push(Construct(state,new Paragraph(prefix,left,right,nil)));
	    break;
	case SENTENCE :
	    state->Pop(tmp);
            separator = (Text*)tmp;
	    state->Push(Construct(state, new Sentence(separator, nil)));
	    break;
	case BLOCK :
	    state->Pop(indent);
	    state->Push(Construct(state, new Display(indent, nil)));
	    break;
	case TEXTLIST :
	    state->Pop(tmp);
            postfix = (Text*)tmp;
	    state->Pop(tmp);
            prefix = (Text*)tmp;
	    state->Pop(tmp);
            keeper = (Text*)tmp;
	    state->Pop(tmp);
            separator = (Text*)tmp;
	    state->Push(Construct(
		state,
		new TextList(separator, keeper, prefix, postfix, nil)
	   ));
	    break;
	case CONTEXT :
	    state->Peek(top);
	    context = (Text*)top;
	    context->SetContext(context);
	    break;
	case STYLE :
	    state->Peek(top);
	    context = (Text*)top;
	    switch(*(script.Text()+1)) {
	    case BOLD : layout->Paint(context, bold); break;
	    case UNDERLINED : layout->Paint(context, underlined); break;
	    case INVERTED : layout->Paint(context, inverted); break;
	    case GREYED : layout->Paint(context, greyed); break;
	    }
	    break;
	case BEGIN :
	    state->Push(BEGINMARKER);
	    break;
	case END :
	    state->Push(ENDMARKER);
	    break;
	case QUOTE :
	    state->Push(new Word(script.Text(), script.Size(), nil));
	    break;
	default :
	    state->Push(new Word(script.Text(), script.Size()));
	    break;
	}
    }

    state->Pop(tmp);
    return (Text*)tmp;
}
