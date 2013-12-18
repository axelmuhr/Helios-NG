// $Header: listboolean.h,v 1.6 88/09/24 15:06:21 interran Exp $
// declares classes booleanNode and booleanList.

#ifndef listboolean_h
#define listboolean_h

#include "list.h"

// A booleanNode contains a boolean value.

class booleanNode : public BaseNode {
public:

    booleanNode(boolean);

    boolean GetBoolean();

protected:

    boolean value;		// contains a boolean value

};

// booleanNode stores the boolean value.

inline booleanNode::booleanNode (boolean v) {
    value = v;
}

// Define inline access functions to get members' values.

inline boolean booleanNode::GetBoolean () {
    return value;
}

// A booleanList manages a list of booleanNodes.

class booleanList : public BaseList {
public:

    booleanNode* First();
    booleanNode* Last();
    booleanNode* Prev();
    booleanNode* Next();
    booleanNode* GetCur();
    booleanNode* Index(int);

};

// Cast these functions to return booleanNodes instead of BaseNodes.

inline booleanNode* booleanList::First () {
    return (booleanNode*) BaseList::First();
}

inline booleanNode* booleanList::Last () {
    return (booleanNode*) BaseList::Last();
}

inline booleanNode* booleanList::Prev () {
    return (booleanNode*) BaseList::Prev();
}

inline booleanNode* booleanList::Next () {
    return (booleanNode*) BaseList::Next();
}

inline booleanNode* booleanList::GetCur () {
    return (booleanNode*) BaseList::GetCur();
}

inline booleanNode* booleanList::Index (int index) {
    return (booleanNode*) BaseList::Index(index);
}

#endif
