/*
 * Implementation of squares subject.
 */

#include "squares.h"

#if defined(hpux)
#   include <stdlib.h>
#else
#   include <random.h>
#endif

Squares::Squares () {
    head = nil;
}

Squares::~Squares () {
    register SquareData* p, * next;

    for (p = head; p != nil; p = next) {
	next = p->next;
	delete p;
    }
}

void Squares::Add (float cx, float cy, float size) {
    register SquareData* s = new SquareData;
    s->cx = cx;
    s->cy = cy;
    s->size = size;
    s->next = head;
    head = s;
    Notify();
}

static float Random () {
#ifdef hpux
    const int bigint = 1<<14;
    int r = rand();
#else
    const int bigint = 1<<30;
    int r = random();
#endif
    if (r < 0) {
	r = -r;
    }
    return float(r % bigint) / float(bigint);
}

void Squares::Add () {
    Add(Random()/2 + .25, Random()/2 + .25, Random()/2);
}
