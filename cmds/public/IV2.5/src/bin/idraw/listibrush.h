// $Header: listibrush.h,v 1.7 88/12/03 20:54:15 interran Exp $
// declares classes IBrushNode and IBrushList.

#ifndef listibrush_h
#define listibrush_h

#include "list.h"

// Declare imported types.

class IBrush;

// An IBrushNode contains an IBrush pointer.

class IBrushNode : public BaseNode {
public:

    IBrushNode(IBrush*);

    boolean SameValueAs(void*);

    IBrush* GetBrush();

protected:

    IBrush* brush;		// points to an IBrush

};

// IBrushNode stores the IBrush pointer.

inline IBrushNode::IBrushNode (IBrush* b) {
    brush = b;
}

// SameValueAs returns true if the given pointer equals the stored
// IBrush pointer.

static boolean IBrushNode::SameValueAs (void* p) {
    return brush == p;
}

// Define inline access functions to get members' values.

inline IBrush* IBrushNode::GetBrush () {
    return brush;
}

// An IBrushList manages a list of IBrushNodes.

class IBrushList : public BaseList {
public:

    IBrushNode* First();
    IBrushNode* Last();
    IBrushNode* Prev();
    IBrushNode* Next();
    IBrushNode* GetCur();
    IBrushNode* Index(int);

};

// Cast these functions to return IBrushNodes instead of BaseNodes.

inline IBrushNode* IBrushList::First () {
    return (IBrushNode*) BaseList::First();
}

inline IBrushNode* IBrushList::Last () {
    return (IBrushNode*) BaseList::Last();
}

inline IBrushNode* IBrushList::Prev () {
    return (IBrushNode*) BaseList::Prev();
}

inline IBrushNode* IBrushList::Next () {
    return (IBrushNode*) BaseList::Next();
}

inline IBrushNode* IBrushList::GetCur () {
    return (IBrushNode*) BaseList::GetCur();
}

inline IBrushNode* IBrushList::Index (int index) {
    return (IBrushNode*) BaseList::Index(index);
}

#endif
