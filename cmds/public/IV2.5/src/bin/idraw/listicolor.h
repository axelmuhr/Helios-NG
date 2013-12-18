// $Header: listicolor.h,v 1.7 88/12/03 20:54:18 interran Exp $
// declares classes IColorNode and IColorList.

#ifndef listicolor_h
#define listicolor_h

#include "list.h"

// Declare imported types.

class IColor;

// An IColorNode contains an IColor pointer.

class IColorNode : public BaseNode {
public:

    IColorNode(IColor*);

    boolean SameValueAs(void*);

    IColor* GetColor();

protected:

    IColor* color;		// points to an IColor

};

// IColorNode stores the IColor pointer.

inline IColorNode::IColorNode (IColor* c) {
    color = c;
}

// SameValueAs returns true if the given pointer equals the stored
// IColor pointer.

static boolean IColorNode::SameValueAs (void* p) {
    return color == p;
}

// Define inline access functions to get members' values.

inline IColor* IColorNode::GetColor () {
    return color;
}

// An IColorList manages a list of IColorNodes.

class IColorList : public BaseList {
public:

    IColorNode* First();
    IColorNode* Last();
    IColorNode* Prev();
    IColorNode* Next();
    IColorNode* GetCur();
    IColorNode* Index(int);

};

// Cast these functions to return IColorNodes instead of BaseNodes.

inline IColorNode* IColorList::First () {
    return (IColorNode*) BaseList::First();
}

inline IColorNode* IColorList::Last () {
    return (IColorNode*) BaseList::Last();
}

inline IColorNode* IColorList::Prev () {
    return (IColorNode*) BaseList::Prev();
}

inline IColorNode* IColorList::Next () {
    return (IColorNode*) BaseList::Next();
}

inline IColorNode* IColorList::GetCur () {
    return (IColorNode*) BaseList::GetCur();
}

inline IColorNode* IColorList::Index (int index) {
    return (IColorNode*) BaseList::Index(index);
}

#endif
