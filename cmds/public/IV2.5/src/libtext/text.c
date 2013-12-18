/*
 * Text - structured text objects
 */

#include <InterViews/Text/text.h>
#include <InterViews/Text/layout.h>
#include <bstring.h>
#include <string.h>

Text::Text (TextData cont) {
    if (cont == SELF) {
	context = this;
    } else {
	context = cont;
    }
    parent = nil;
    next = nil;
    size = 0;
}

Text::~Text () {
    if (parent != nil) {
	parent->Remove(this);
    }
}

boolean Text::Overflows (Layout* l) {
    return size > l->Remaining();
}

void Text::Locate (Coord& x1, Coord& y1, Coord& x2, Coord& y2) {
    if (parent != nil) {
	parent->Locate(x1, y1, x2, y2);
    }
}

void Text::Undraw () {
    if (parent != nil) {
	parent->Undraw();
    }
}

Whitespace::Whitespace (int s, TextData context) : (context) {
    size = s;
}

void Whitespace::Draw (Layout* l) {
    if (context != nil) {
	l->Enter(context);
    }
    if (size > 0) {
	if (l->X() > 0) {
	    l->Space(size);
	}
    } else if (size < 0) {
	l->Backspace(-size);
    }
    if (context != nil) {
	l->Leave(context);
    }
}

Text* Whitespace::Copy () {
    return new Whitespace(size, context==this ? SELF : context);
}

LineBreak::LineBreak (TextData context) : (BREAK, context) { }

Caret::Caret () { }

boolean Caret::Overflows (Layout*) {
    return false;
}

void Caret::Draw (Layout* l) {
    l->Caret();
}

Word::Word (const char* w, int s, TextData context) : (context) {
    word = w;
    size = s;
}

void Word::Draw (Layout* l) {
    if (size <= 0) {
	return;
    }
    if (context != nil) {
	l->Enter(context);
    }
    l->String(word, size);
    if (context != nil) {
	l->Leave(context);
    }
}

Text* Word::Copy () {
    return new Word(word, size, context==this ? SELF : context);
}

EditWord::EditWord (char* buff, int len) {
    word = nil;
    bufferlength = len;
    buffer = buff;
    size = 0;
    firstx = firsty = 0;
    pos = 0;
    drawn = false;
}

EditWord::~EditWord () {
    Done();
}

void EditWord::Draw (Layout* l) {
    l->Enter(context);
    firstx = l->X();
    firsty = l->Y();
    l->String(buffer, pos);
    l->Caret();
    l->String(buffer+pos, size-pos);
    drawn = true;
    l->Leave(context);
}

void EditWord::Undraw () {
    drawn = false;
    Text::Undraw();
}

void EditWord::Locate (Coord& x1, Coord& y1, Coord& x2, Coord& y2) {
    if (drawn && (y2 < 0 || x2 < 0)) {
	x1 = firstx;
	y1 = firsty;
	x2 = firstx + size;
	y2 = firsty;
    }
    Text::Locate(x1, y1, x2, y2);
}

void EditWord::Edit (Word* w) {
    word = w;
    size = word->size;
    if (size > bufferlength) {
	delete buffer;
	bufferlength = 2 * size;
	buffer = new char[ bufferlength + 1 ];
    }
    strncpy(buffer, w->word, size+1);
    pos = size;
    word->Parent()->Replace(word, this);
    firstx = firsty = 0;
    drawn = false;
}

void EditWord::Done () {
    if (word == nil) {
	return;
    }
    delete word->word;
    word->word = strncpy(new char[ size+1 ], buffer, size+1);
    word->size = size;
    this->Parent()->Replace(this, word);
    word = nil;
}

void EditWord::Insert (const char* s, int length) {
    if (word == nil) {
	return;
    }
    bcopy(buffer+pos, buffer+pos+length, max(0, bufferlength-(pos+length)));
    int l = min(bufferlength-pos, length);
    strncpy(buffer+pos, s, l);
    size += l;
    pos += l;
}

void EditWord::Delete (int length) {
    if (word == nil) {
	return;
    }
    int l = min(pos, length);
    if (l > 0) {
	bcopy(buffer+pos, buffer+pos-length, bufferlength-pos);
	size -= l;
	pos -= l;
    }
}

void EditWord::GoTo (int p) {
    if (word == nil) {
	return;
    }
    pos = max(0, min(p, size));
}

void EditWord::Forward (int count) {
    if (word == nil) {
	return;
    }
    pos = min(pos+count, size);
}

void EditWord::Backward (int count) {
    if (word == nil) {
	return;
    }
    pos = max(pos-count, 0);
}

Composition::Composition (TextData context) : (context) {
    drawn = false;
    firstx = firsty = -1;
    nextx = nexty = -1;
    first = nil;
    last = nil;
    count = nil;
}

Composition::~Composition () {
    while (first != nil) {
	Text* doomed = first;
	first = first->Next();
	doomed->parent = nil;
	delete doomed;
    }
}

Text* Composition::Succ (Text* t) {
    Text* comp = first;
    while (comp != nil) {
	if (comp == t) {
	    break;
	} else {
	    comp = comp->Next();
	}
    }
    return comp->Next();
}

Text* Composition::Pred (Text* t) {
    Text* comp = first;
    while (comp != nil) {
	if (comp->Next() == t) {
	    break;
	} else {
	    comp = comp->Next();
	}
    }
    return comp;
}

boolean Composition::Follows (Text* t1, Text* t2) {
    boolean foundfirst = false;
    Text* comp = first;
    while (comp != nil) {
	if (comp == t1) {
	    foundfirst = true;
	} else if (comp == t2) {
	    if (foundfirst) {
		return true;
	    } else {
		return false;
	    }
	}
    }
    return false;
}

void Composition::DrawContents (Layout* l) {
    Text* comp = first;
    while (comp != nil) {
	comp->Draw(l);
	comp = comp->Next();
    }
}

Text* Composition::Copy () {
    Composition* result = new Composition(context == this ? SELF : context);
    result->Duplicate(this);
    return result;
}

void Composition::Duplicate (Composition* c) {
    Text* comp = c->first;
    while (comp != nil) {
	Composition::Build(comp->Copy());
	comp = comp->Next();
    }
    count = c->count;
}

void Composition::Append (Text* t) {
    if (t->parent != nil) {
	t->parent->Remove(t);
    }
    t->Reshape();
    t->parent = this;
    t->next = nil;
    if (first == nil) {
	first = t;
    }
    if (last != nil) {
	last->next = t;
    }
    last = t;
    ++count;
}

void Composition::Prepend (Text* t) {
    if (t->parent != nil) {
	t->parent->Remove(t);
    }
    t->Reshape();
    t->parent = this;
    t->next = first;
    if (last == nil) {
	last = t;
    }
    first = t;
    ++count;
}

void Composition::InsertAfter (Text* old, Text* t) {
    if (t->parent != nil) {
	t->parent->Remove(t);
    }
    t->Reshape();
    t->parent = this;
    if (last != nil && last == old) {
	t->next = nil;
	last->next = t;
	last = t;
    } else if (old == nil) {
	t->next = first;
	first = t;
	if (last == nil) {
	    last = t;
	}
    } else {
	Text* comp = first;
	while (comp != nil && comp != old) {
	    comp = comp->next;
	}
	if (comp != nil) {
	    t->next = comp->next;
	    comp->next = t;
	}
    }
    ++count;
    Reshape();
}

void Composition::InsertBefore (Text* old, Text* t) {
    if (t->parent != nil) {
	t->parent->Remove(t);
    }
    t->Reshape();
    t->parent = this;
    if (old == nil) {
	t->next = nil;
	if (first == nil) {
	    first = t;
	}
	if (last != nil) {
	    last->next = t;
	}
	last = t;
    } else if (old == first) {
	t->next = first;
	if (last == nil) {
	    last = t;
	}
	first = t;
    } else {
	Text* comp = first;
	while (comp != nil && comp->next != old) {
	    comp = comp->next;
	}
	if (comp != nil) {
	    t->next = comp->next;
	    comp->next = t;
	}
    }
    ++count;
    Reshape();
}

void Composition::Remove (Text* old) {
    Text* comp = first;
    Text* prev = nil;
    while (comp != nil && comp != old) {
	prev = comp;
	comp = comp->next;
    }
    if (comp != nil) {
	if (comp == last) {
	    last = prev;
	}
	if (prev == nil) {
	    first = comp->next;
	} else {
	    prev->next = comp->next;
	}
	old->parent = nil;
	old->next = nil;
    }
    --count;
    Reshape();
}

void Composition::Replace (Text* old, Text* t) {
    InsertAfter(old, t);
    Remove(old);
}

void Composition::Size () {
    size = 0;
    Text* comp = first;
    while (comp != nil) {
	size += comp->size;
	comp = comp->next;
    }
}

void Composition::Reshape () {
    Size();
    if (parent != nil) {
	parent->Reshape();
    }
}

void Composition::Draw (Layout* l) {
    Coord x1 = l->X();
    Coord y1 = l->Y();
    boolean firstshifted = x1 != firstx || y1 != firsty;
    firstx = x1;
    firsty = y1;
    if (context != nil) {
	l->Enter(context);
    }
    if (firstshifted || ! drawn || ! l->SkipTo(nextx, nexty)) {
	DrawContents(l);
	if (l->Drawing()) {
	    drawn = true;
	} else if (firstshifted) {
	    l->ExtraDamage(x1, y1);
	}
    }
    if (context != nil) {
	l->Leave(context);
    }
    Coord x2 = l->X();
    Coord y2 = l->Y();
    boolean nextshifted = x2 != nextx || y2 != nexty;
    nextx = x2;
    nexty = y2;

    if (drawn) {
	if (firstshifted || nextshifted) {
	    l->ExtraDamage(firstx, firsty);
	} else {
	    l->EndDamage(firstx, firsty);
	}
    }
}

void Composition::Locate (Coord& x1, Coord& y1, Coord& x2, Coord& y2) {
    if (y2 < 0 || x2 < 0) {
	x1 = firstx;
	y1 = firsty;
	x2 = nextx;
	y2 = nexty;
    }
    Text::Locate(x1, y1, x2, y2);
}

void Composition::Undraw () {
    drawn = false;
    Text::Undraw();
}

Paragraph::Paragraph (Text* pre, int l, int r, TextData context) : (context) {
    prefix = pre;
    left = l;
    right = r;
}

void Paragraph::Locate (Coord& x1, Coord& y1, Coord& x2, Coord& y2) {
    x1 += left;
    x2 += left;
    Composition::Locate(x1, y1, x2, y2);
}

Text* Paragraph::Copy () {
    Paragraph* result = new Paragraph(
	prefix, left, right, 
	context==this ? SELF : context
    );
    result->Duplicate(this);
    return result;
}

void Paragraph::Reshape () {
    Size();
    if (prefix != nil) {
	size += prefix->GetSize();
    }
}

boolean Paragraph::Overflows (Layout* l) {
    return size + left + right > l->Remaining();
}

void Paragraph::DrawContents (Layout* l) {
    l->Margins(left, right);
    if (prefix != nil) {
	prefix->Draw(l);
    }
    if (l->Remaining() <= 0) {
	l->NewLine();
    }
    Text* comp = first;
    while (comp != nil) {
	comp->Draw(l);
	comp = comp->Next();
    }
    l->Margins(-left, -right);
}

Sentence::Sentence (Text* sep, TextData context) : (context) {
    separator = sep;
    if (separator != nil) {
	separator->SetContext(nil);
    }
}

Text* Sentence::Copy () {
    Sentence* result = 	new Sentence(separator, context==this?SELF:context);
    result->Duplicate(this);
    return result;
}

boolean Sentence::Overflows (Layout* l) {
    if (count == 0) {
	return false;
    } else {
	return first->Overflows(l);
    }
}

void Sentence::Reshape () {
    Size();
    if (separator != nil && count > 0) {
	size += (count-1) * separator->GetSize();
    }
    if (parent != nil) {
	parent->Reshape();
    }
}

void Sentence::DrawContents (Layout* l) {
    Text* comp = first;
    while (comp != nil) {
	comp->Draw(l);
	comp = comp->Next();
	if (comp != nil) {
	    if (separator != nil) {
		separator->Draw(l);
	    }
	    if (comp->Overflows(l)) {
		l->NewLine();
	    }
	}
    }
}

Display::Display (int ind, TextData context) : (context) {
    indent = ind;
    broken = false;
}

Text* Display::Copy () {
    Display* result = new Display(indent, context==this ? SELF : context);
    result->Duplicate(this);
    return result;
}

void Display::Reshape () {
    Size();
    if (parent != nil) {
	parent->Reshape();
    }
}

boolean Display::Overflows (Layout* l) {
    if (count >= 2) {
	return first->GetSize() > l->Remaining();
    } else {
	return false;
    }
}

void Display::DrawContents (Layout* l) {
    switch (count) {
    case 0:
	break;
    case 1:
	Drawer(l, first, nil);
	break;
    case 2:
	first->Draw(l);
	Drawer(l, last, nil);
	break;
    default:
	first->Draw(l);
	Drawer(l, first->Next(), last);
	last->Draw(l);
	break;
    }
}

void Display::Drawer (Layout* l, Text* start, Text* stop) {
    broken = start->Overflows(l);
    Text* t = start;
    if (broken) {
	l->Margins(indent, 0);
	l->Enter(nil);
	l->NewLine();
	TextData lastcontext = nil;
	while (t != stop) {
	    t->Draw(l);
	    lastcontext = t->GetContext();
	    t = t->Next();
	}
	l->Margins(-indent, 0);
	if (stop != nil) {
	    l->NewLine();
	}
	l->Leave(nil);
    } else {
	while (t != stop) {
	    t->Draw(l);
	    t = t->Next();
	}
    }
}

void Display::Locate (Coord& x1, Coord& y1, Coord& x2, Coord& y2) {
    if (broken) {
	x1 += indent;
	x2 += indent;
    }
    Composition::Locate(x1, y1, x2, y2);
}

TextList::TextList (
    Text* sep, Text* keep, 
    Text* pre, Text* post, 
    TextData context
) : (context) {
    separator = sep;
    if (separator != nil) {
	separator->SetContext(nil);
	separator->Reshape();
    }
    keeper = keep;
    if (keeper != nil) {
	keeper->SetContext(nil);
	keeper->Reshape();
    }
    prefix = pre;
    if (prefix != nil) {
	prefix->SetContext(nil);
	prefix->Reshape();
    }
    postfix = post;
    if (postfix != nil) {
	postfix->SetContext(nil);
	postfix->Reshape();
    }
    if (keeper != nil) {
	size = keeper->GetSize();
    } else {
	size = 0;
    }
}

void TextList::DrawContents (Layout* l) {
    if (count == 0) {
	if (keeper != nil) {
	    keeper->Draw(l);
	}
    } else {
	boolean broken = Overflows(l);
	if (prefix != nil) {
	    prefix->Draw(l);
	}
	Text* comp = first;
	while (comp != nil) {
	    comp->Draw(l);
	    comp = comp->Next();
	    if (comp != nil) {
		if (separator != nil) {
		    separator->Draw(l);
		}
		if (broken) {
		    l->NewLine();
		}
	    }
	}
	if (postfix != nil) {
	    postfix->Draw(l);
	}
    }
}

void TextList::Reshape () {
    Size();
    if (count == 0) {
	if (keeper != nil) {
	    size = keeper->GetSize();
	} else {
	    size = 0;
	}
    } else if (count > 0) {
	if (separator != nil) {
	    size += (count-1) * separator->GetSize();
	}
	if (prefix != nil) {
	    size += prefix->GetSize();
	}
	if (postfix != nil) {
	    size += postfix->GetSize();
	}
    }
    if (parent != nil) {
	parent->Reshape();
    }
}

Text* TextList::Copy () {
    TextList* result = new TextList(
	separator, keeper, 
	prefix, postfix, 
	context==this?SELF:context
    );
    result->Duplicate(this);
    return result;
}
