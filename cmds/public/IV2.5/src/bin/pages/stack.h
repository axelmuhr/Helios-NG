/*
 * Stack
 */

#ifndef stack_h
#define stack_h

#include <InterViews/defs.h>

class StackEntry;

class Stack {
public:
    Stack();
    ~Stack();

    void Push(const void*);
    void Push(int);

    void Pop(const void*&);
    void Pop(int&);

    void Peek(const void*&);
    void Peek(int&);

    void Drop();
    void Dup();
    void Swap();

    boolean IsEmpty();
protected:
    StackEntry* top;
};

#endif
