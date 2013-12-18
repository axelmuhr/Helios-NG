/*
 * Bitmap - Bit map for InterViews
 */

#include <InterViews/bitmap.h>
#include <InterViews/transformer.h>

Bitmap::Bitmap (const char* filename) {
    rep = new BitmapRep(filename);
}

Bitmap::Bitmap (void* d, int w, int h, int x, int y) {
    rep = new BitmapRep(d, w, h, x, y);
}

Bitmap::Bitmap (Font* f, int c) {
    rep = new BitmapRep(f, c);
}

Bitmap::Bitmap (Bitmap* b) {
    rep = new BitmapRep(b->rep, NoTx);
}

Bitmap::~Bitmap () {
    if (LastRef()) {
	delete rep;
    }
}

void* Bitmap::Map () {
    return rep->GetMap();
}

void Bitmap::Transform (Transformer* t) {
    BitmapRep* newrep = new BitmapRep(rep, t);
    delete rep;
    rep = newrep;
}

void Bitmap::Scale (float sx, float sy) {
    Transformer* t = new Transformer;
    t->Scale(sx,sy);
    Transform(t);
    delete t;
}

void Bitmap::Rotate (float angle) {
    Transformer* t = new Transformer;
    t->Rotate(angle);
    Transform(t);
    delete t;
}

void Bitmap::FlipHorizontal () {
    BitmapRep* newrep = new BitmapRep(rep, FlipH);
    delete rep;
    rep = newrep;
}

void Bitmap::FlipVertical () {
    BitmapRep* newrep = new BitmapRep(rep, FlipV);
    delete rep;
    rep = newrep;
}

void Bitmap::Invert () {
    BitmapRep* newrep = new BitmapRep(rep, Inv);
    delete rep;
    rep = newrep;
}

void Bitmap::Rotate90 () {
    BitmapRep* newrep = new BitmapRep(rep, Rot90);
    delete rep;
    rep = newrep;
}

void Bitmap::Rotate180 () {
    BitmapRep* newrep = new BitmapRep(rep, Rot180);
    delete rep;
    rep = newrep;
}

void Bitmap::Rotate270 () {
    BitmapRep* newrep = new BitmapRep(rep, Rot270);
    delete rep;
    rep = newrep;
}

boolean Bitmap::Contains (int x, int y) {
    return x >= Left() && x <= Right() && y >= Bottom() && y <= Top();
}

boolean Bitmap::Peek (int x, int y) {
    return Contains(x, y) ? rep->GetBit(x-Left(), y-Bottom()) : false;
}

void Bitmap::Poke (boolean bit, int x, int y) {
    if (Contains(x, y)) {
        rep->PutBit(x-Left(), y-Bottom(), bit);
        rep->Touch();
    }
}
