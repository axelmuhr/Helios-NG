// $Header: listifont.h,v 1.7 88/12/03 20:54:20 interran Exp $
// declares classes IFontNode and IFontList.

#ifndef listifont_h
#define listifont_h

#include "list.h"

// Declare imported types.

class IFont;

// An IFontNode contains an IFont pointer.

class IFontNode : public BaseNode {
public:

    IFontNode(IFont*);

    boolean SameValueAs(void*);

    IFont* GetFont();

protected:

    IFont* font;		// points to an IFont

};

// IFontNode stores the IFont pointer.

inline IFontNode::IFontNode (IFont* f) {
    font = f;
}

// SameValueAs returns true if the given pointer equals the stored
// IFont pointer.

static boolean IFontNode::SameValueAs (void* p) {
    return font == p;
}

// Define inline access functions to get members' values.

inline IFont* IFontNode::GetFont () {
    return font;
}

// An IFontList manages a list of IFontNodes.

class IFontList : public BaseList {
public:

    IFontNode* First();
    IFontNode* Last();
    IFontNode* Prev();
    IFontNode* Next();
    IFontNode* GetCur();
    IFontNode* Index(int);

};

// Cast these functions to return IFontNodes instead of BaseNodes.

inline IFontNode* IFontList::First () {
    return (IFontNode*) BaseList::First();
}

inline IFontNode* IFontList::Last () {
    return (IFontNode*) BaseList::Last();
}

inline IFontNode* IFontList::Prev () {
    return (IFontNode*) BaseList::Prev();
}

inline IFontNode* IFontList::Next () {
    return (IFontNode*) BaseList::Next();
}

inline IFontNode* IFontList::GetCur () {
    return (IFontNode*) BaseList::GetCur();
}

inline IFontNode* IFontList::Index (int index) {
    return (IFontNode*) BaseList::Index(index);
}

#endif
