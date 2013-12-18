/*
 * The subject is a simple list of squares.
 */

#ifndef squares_h
#define squares_h

#include <InterViews/subject.h>

class SquareData {
    friend class Squares;
    friend class SquaresView;

    float cx, cy, size;
    SquareData* next;
};

class Squares : public Subject {
public:
    Squares();
    ~Squares();

    void Add(float cx, float cy, float size);
    void Add();
    SquareData* Head () { return head; }
private:
    SquareData* head;
};

#endif
