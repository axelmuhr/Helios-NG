/*
 * Layout - a chain of TextBlocks
 */

#include <InterViews/Text/layout.h>
#include <InterViews/Text/textblock.h>
#include <InterViews/Text/text.h>
#include <InterViews/Text/textpainter.h>
#include <InterViews/sensor.h>
#include <memory.h>

static const int PAINTMAPSIZE = 100;
static const int MAXCONTEXTDEPTH = 50;
static const int BIG = 1000000;

class Context {
public:
    Context() { context = nil; painter = nil; left = 0; right = 0; }
    TextData context;
    TextPainter* painter;
    Coord left;
    Coord right;
};

typedef Context* ContextP;

class PaintMap {
    struct PaintEntry {
	PaintEntry(TextData t, TextPainter* v) { tag = t; value = v; }
	TextData tag;
	TextPainter* value;
	PaintEntry* next;
    } ** entries;
    int count;

    boolean Match(TextData t1, TextData t2) { return t1 == t2; }
    int Hash(TextData tag) { return ((int)tag>>4) & count; }
public:
    PaintMap(int count);
    ~PaintMap();
    void Insert(TextData, TextPainter*);
    void Remove(TextData);
    TextPainter* Find(TextData tag);
};

PaintMap::PaintMap (int c) {
    count = 1;
    while (count < c) {
	count *= 2;
    }
    typedef PaintEntry* ep;
    entries = new ep[ count ];
    for (int i = 0; i<count; ++i) {
	entries[i] = nil;
    }
    --count;
}

PaintMap::~PaintMap () {
    ++count;
    for (int i = 0; i<count; ++i) {
	PaintEntry* entry = entries[i];
	while (entry != nil) {
	    PaintEntry* doomed = entry;
	    entry = entry->next;
	    delete doomed;
	}
    }
    delete entries;
}

void PaintMap::Insert (TextData tag, TextPainter* value) {
    int hash = Hash(tag);
    PaintEntry* newEntry = new PaintEntry(tag, value);
    newEntry->next = entries[hash];
    entries[hash] = newEntry;
}

void PaintMap::Remove (TextData tag) {
    int hash = Hash(tag);
    PaintEntry* entry = entries[hash];
    PaintEntry* prev = nil;
    while (entry != nil) {
	if (Match(entry->tag, tag) ) {
	    break;
	} else {
	    prev = entry;
	    entry = entry->next;
	}
    }
    if (entry != nil) {
	if (prev != nil) {
	    prev->next = entry->next;
	} else {
	    entries[hash] = entry->next;
	}
	delete entry;
    }
}

TextPainter* PaintMap::Find (TextData tag) {
    int hash = Hash(tag);
    PaintEntry* entry = entries[hash];
    while (entry != nil) {
	if (Match(entry->tag, tag) ) {
	    break;
	} else {
	    entry = entry->next;
	}
    }
    if (entry != nil) {
	return entry->value;
    } else {
	return nil;
    }
}


class Region {
public:
    Coord x1;
    Coord y1;
    Coord x2;
    Coord y2;

    Region() { x1 = BIG; y1 = BIG; x2 = -BIG; y2 = -BIG; }
    Region(Coord xx1, Coord yy1, Coord xx2, Coord yy2);

    boolean AllAfter(Coord x, Coord y) { return y<y1 || y==y1 && x<x1; }
    boolean AllBefore(Coord x, Coord y) { return y>y2 || y==y2 && x>x2; }
    boolean Intersects(Coord y) { return y>=y1 && y<=y2; }
    boolean Encloses(Coord x, Coord y) {
	return !AllAfter(x, y) && !AllBefore(x, y);
    }

    void Extend(Region*);
    void Limit(Coord x1, Coord y1, Coord x2, Coord y2);
    void Extra(Coord x, Coord y);
    void Truncate(Coord x, Coord y);
};

Region::Region (Coord xx1, Coord yy1, Coord xx2, Coord yy2) {
    x1 = xx1; y1 = yy1; x2 = xx2; y2 = yy2;
}

void Region::Extend (Region* r) {
    if (r == nil) {
	return;
    }
    x1 = min(x1, r->x1);
    y1 = min(y1, r->y1);
    x2 = max(x2, r->x2);
    y2 = max(y2, r->y2);
}

void Region::Limit (Coord xx1, Coord yy1, Coord xx2, Coord yy2) {
    x1 = max(x1, xx1);
    y1 = max(y1, yy1);
    x2 = min(x2, xx2);
    y2 = min(y2, yy2);
}

void Region::Extra (Coord x, Coord y) {
    x1 = min(x1, x);
    y1 = min(y1, y);
    x2 = max(x2, BIG);
    y2 = max(y2, BIG);
}

void Region::Truncate (Coord x, Coord y) {
    x2 = min(x2, x);
    y2 = min(y2, y);
}

Layout::Layout (Text* t, Sensor* s, TextPainter* p) {
    contents = t;
    contents->Reshape();

    top = nil;
    tail = nil;
    block = nil;

    painters = new PaintMap(PAINTMAPSIZE);
    contexts = new ContextP[MAXCONTEXTDEPTH];
    for (int i = 0; i < MAXCONTEXTDEPTH; ++i) {
	contexts[i] = new Context;
    }
    input = s;
    if (input != nil) {
	input->Reference();
    }
    output = p;
    if (output != nil) {
	output->Reference();
    }
    (*contexts)->painter = output;

    hit = nil;
    lastcontext = nil;
    intarget = false;

    left = 0;
    right = 0;
    (*contexts)->left = 0;
    (*contexts)->right = 0;

    damage = nil;
    target = nil;
    extradamage = nil;
}

Layout::~Layout () {
    delete contents;
    delete painters;
    delete output;
    delete input;
    while (top != nil) {
	top->layout = nil;
	top->Draw();
	top = top->next;
    }
}

void Layout::Chain (TextBlock* newblock) {
    if (top == nil) {
	top = newblock;
	block = top;
    }
    if (tail != nil) {
	tail->next = newblock;
    }
    tail = newblock;
    newblock->Initialize(this, output, input);
}

void Layout::Unchain (TextBlock* oldblock) {
    TextBlock* b = top;
    TextBlock* prev = nil;
    while (b != nil) {
	if (b == oldblock) {
	    if (prev != nil) {
		prev->next = b->next;
	    } else {
		top = b->next;
	    }
	    if (b == tail) {
		tail = prev;
	    }
	    b->layout = nil;
	    break;
	} else {
	    prev = b;
	    b = b->next;
	}
    }
    Rechain();
}

void Layout::Rechain () {
    TextBlock* b = top;
    int yy = 0;
    while (b != nil) {
	b->start = yy;
	yy += b->rows;
	b = b->next;
    }
    Damage(0, 0, BIG, BIG);
}

void Layout::Flush () {
    if (block != nil) {
	block->Flush((*contexts)->painter);
    }
}

void Layout::GoTo (Coord xx, Coord yy) {
    Flush();
    x = xx;
    y = yy;
    intarget = target != nil && target->Intersects(y);
    block = top;
    while (block != nil) {
	if (block->Contains(yy)) {
	    width = block->Cols();
	    break;
	} else {
	    block = block->next;
	}
    }
    if (intarget && block != nil) {
	block->GoTo(xx + left, yy);
    }
}

void Layout::Caret () {
    if (intarget && block != nil) {
	Flush();
	block->Caret();
    }
}

void Layout::Overfull () {
    if (intarget && block != nil) {
	Flush();
	block->Overfull();
    }
}

void Layout::String (const char* s, int length) {
    if (block != nil) {
	int l = min(length, width-x-left);
	x += l;
	if (intarget) {
	    block->String(s, l);
	}
    } else {
	x += length;
    }
}

void Layout::Space (int count) {
    if (count > 0 && block != nil) {
	int l = min(count, width-x-left);
	x += l;
	if (intarget) {
	    block->Space(l);
	}
    } else {
	x += count;
    }
}

void Layout::Backspace (int count) {
    GoTo(x - count, y);
}

void Layout::NewLine () {
    if (intarget && block != nil) {
	Space(width-x-(*contexts)->left-(*contexts)->right);
	Flush();
	block->EndLine();
    }
    x = 0;
    y += 1;
    if (block != nil && block->PastEnd(y)) {
	block->LastLine(y-1);
	block = block->next;
    }
    intarget = target != nil && target->Intersects(y);
    if (intarget && block != nil) {
	int i = left - (*contexts)->left;
	x -= i;
	block->GoTo((*contexts)->left, y);
	block->StartLine();
	Space(i);
    }
}

void Layout::LastLine () {
    if (intarget && block != nil) {
	Space(width-x-(*contexts)->left-(*contexts)->right);
	Flush();
	block->EndLine();
    }
    TextBlock* b = block;
    while (b != nil) {
	b->GoTo(0, y);
	b->EndBlock();
	b = b->next;
    }
    if (tail != nil) {
	tail->LastLine(y);
    }
}

boolean Layout::SkipTo (Coord xx, Coord yy) {
    if (
	target == nil
	|| target->AllBefore(x+left, y)
	|| target->AllAfter(xx+left, yy)
) {
	GoTo(xx, yy);
	return true;
    } else {
	return false;
    }
}

void Layout::Damage (Coord x1, Coord y1, Coord x2, Coord y2) {
    if (damage == nil) {
	damage = new Region;
    }
    Region r(x1, y1, x2, y2);
    damage->Extend(&r);
}

void Layout::ExtraDamage (Coord x, Coord y) {
    if (extradamage == nil) {
	extradamage = new Region;
    }
    extradamage->Extra(x+left, y);
}

void Layout::EndDamage (Coord x, Coord y) {
    if (extradamage == nil) {
	return;
    }
    extradamage->Truncate(x+left, y);
}

void Layout::Repair () {
    if (contents == nil || damage == nil) {
	return;
    }
    target = nil;
    extradamage = nil;
    GoTo(0, 0);
    contents->Draw(this);

    do {
	target = new Region;
	target->Extend(damage);
	delete damage; damage = nil;
	target->Extend(extradamage);
	delete extradamage; extradamage = nil;
	GoTo(0, 0);
	contents->Draw(this);
	if (extradamage != nil) {
	    extradamage->x1 = max(target->x2, extradamage->x1);
	    extradamage->y1 = max(target->y2, extradamage->y1);
	}
	delete target;
    } while (extradamage != nil);
    LastLine();
}

void Layout::Margins (Coord dl, Coord dr) {
    left += dl;
    right += dr;
    x -= dl;
}

void Layout::Enter (TextData context) {
    if (hit == nil) {
	if (target != nil && ! target->AllAfter(x+left-1, y) ) {
	    if ((*contexts)->context == nil) {
		hit = context;
	    } else {
		hit = (*contexts)->context;
	    }
	} else {
	    lastcontext = nil;
	}
    }
    TextPainter* oldpaint = (*contexts)->painter;
    Coord oldleft = (*contexts)->left;
    Coord oldright = (*contexts)->right;
    TextPainter* paint = painters->Find(context);
    if (paint == nil) {
	paint = oldpaint;
    }
    if (paint != oldpaint) {
	Flush();
    }
    ++contexts;
    (*contexts)->context = context;
    (*contexts)->painter = paint;
    if (paint != oldpaint) {
	(*contexts)->left = left;
	(*contexts)->right = right;
    } else {
	(*contexts)->left = oldleft;
	(*contexts)->right = oldright;
    }
}

void Layout::Leave (TextData context) {
    if (hit == nil) {
	if (target != nil && ! target->AllAfter(x+left-1, y) ) {
	    if (context == nil) {
		hit = lastcontext;
	    } else {
		hit = (*contexts)->context;
	    }
	} else {
	    lastcontext = context;
	}
    }
    Painter* oldpaint = (*contexts)->painter;
    Painter* paint = (*(contexts-1))->painter;
    if (paint != oldpaint) {
	Flush();
    }
    --contexts;
}

void Layout::Listen (Sensor* s) {
    TextBlock* b = top;
    while (b != nil) {
	b->Listen(s);
	b = b->next;
    }
}

void Layout::Paint (TextData context, TextPainter* paint) {
    if (paint == nil) {
	painters->Remove(context);
    } else {
	painters->Insert(context, paint);
    }
}

void Layout::Unpaint (TextData context) {
    painters->Remove(context);
}

void Layout::Show (Text* t) {
    Touch(t);
    Repair();
}

void Layout::Touch (Text* t) {
    if (t == nil) {
	return;
    }
    Region touched;
    t->Locate(touched.x1, touched.y1, touched.x2, touched.y2);
    touched.x1 += left; touched.x2 += left;
    t->Undraw();
    if (damage == nil) {
	damage = new Region;
    }
    damage->Extend(&touched);
}

boolean Layout::Hit (Event& e, TextData& context, TextData& after) {
    TextBlock* b = top;
    while (b != nil) {
	if (b == e.target) {
	    hit = nil;
	    Coord xx = b->XChar(e.x);
	    Coord yy = b->YChar(e.y) + b->start;
	    Repair();
	    damage = new Region(xx, yy, xx, yy);
	    Repair();
	    context = hit;
	    after = lastcontext;
	    return true;
	} else {
	    b = b->next;
	}
    }
    return false;
}
