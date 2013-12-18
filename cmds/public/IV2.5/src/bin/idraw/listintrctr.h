// $Header: listintrctr.h,v 1.7 88/12/03 20:54:23 interran Exp $
// declares classes InteractorNode and InteractorList.

#ifndef listintrctr_h
#define listintrctr_h

#include "list.h"

// Declare imported types.

class Interactor;

// An InteractorNode contains an Interactor pointer.

class InteractorNode : public BaseNode {
public:

    InteractorNode(Interactor*);

    boolean SameValueAs(void*);

    Interactor* GetInteractor();

protected:

    Interactor* interactor;	// points to a Interactor

};

// InteractorNode stores the Interactor pointer.

inline InteractorNode::InteractorNode (Interactor* i) {
    interactor = i;
}

// SameValueAs returns true if the given pointer equals the stored
// Interactor pointer.

static boolean InteractorNode::SameValueAs (void* p) {
    return interactor == p;
}

// Define inline access functions to get members' values.

inline Interactor* InteractorNode::GetInteractor () {
    return interactor;
}

// An InteractorList manages a list of InteractorNodes.

class InteractorList : public BaseList {
public:

    InteractorNode* First();
    InteractorNode* Last();
    InteractorNode* Prev();
    InteractorNode* Next();
    InteractorNode* GetCur();
    InteractorNode* Index(int);

};

// Cast these functions to return InteractorNodes instead of
// BaseNodes.

inline InteractorNode* InteractorList::First () {
    return (InteractorNode*) BaseList::First();
}

inline InteractorNode* InteractorList::Last () {
    return (InteractorNode*) BaseList::Last();
}

inline InteractorNode* InteractorList::Prev () {
    return (InteractorNode*) BaseList::Prev();
}

inline InteractorNode* InteractorList::Next () {
    return (InteractorNode*) BaseList::Next();
}

inline InteractorNode* InteractorList::GetCur () {
    return (InteractorNode*) BaseList::GetCur();
}

inline InteractorNode* InteractorList::Index (int index) {
    return (InteractorNode*) BaseList::Index(index);
}

#endif
