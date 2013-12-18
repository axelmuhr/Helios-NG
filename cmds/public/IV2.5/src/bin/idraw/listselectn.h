// $Header: listselectn.h,v 1.7 88/12/03 20:54:28 interran Exp $
// declares classes SelectionNode and SelectionList.

#ifndef listselectn_h
#define listselectn_h

#include "list.h"

// Declare imported types.

class Selection;

// A SelectionNode stores a pointer to a Selection.

class SelectionNode : public BaseNode {
public:

    SelectionNode(Selection*);

    boolean SameValueAs(void*);

    Selection* GetSelection();

protected:

    Selection* selection;	// points to a Selection

};

// SelectionNode stores the Selection pointer.

inline SelectionNode::SelectionNode (Selection* s) {
    selection = s;
}

// SameValueAs returns true if the given pointer equals the stored
// Selection pointer.

static boolean SelectionNode::SameValueAs (void* p) {
    return selection == p;
}

// Define inline access functions to get members' values.

static Selection* SelectionNode::GetSelection () {
    return selection;
}

// A SelectionList manages a list of SelectionNodes.

class SelectionList : public BaseList {
public:

    SelectionNode* First();
    SelectionNode* Last();
    SelectionNode* Prev();
    SelectionNode* Next();
    SelectionNode* GetCur();
    SelectionNode* Index(int);

};

// Cast these functions to return SelectionNodes instead of BaseNodes.

inline SelectionNode* SelectionList::First () {
    return (SelectionNode*) BaseList::First();
}

inline SelectionNode* SelectionList::Last () {
    return (SelectionNode*) BaseList::Last();
}

inline SelectionNode* SelectionList::Prev () {
    return (SelectionNode*) BaseList::Prev();
}

inline SelectionNode* SelectionList::Next () {
    return (SelectionNode*) BaseList::Next();
}

inline SelectionNode* SelectionList::GetCur () {
    return (SelectionNode*) BaseList::GetCur();
}

inline SelectionNode* SelectionList::Index (int index) {
    return (SelectionNode*) BaseList::Index(index);
}

#endif
