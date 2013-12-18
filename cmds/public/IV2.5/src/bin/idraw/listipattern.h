// $Header: listipattern.h,v 1.7 88/12/03 20:54:26 interran Exp $
// declares classes IPatternNode and IPatternList.

#ifndef listipattern_h
#define listipattern_h

#include "list.h"

// Declare imported types.

class IPattern;

// An IPatternNode contains an IPattern pointer.

class IPatternNode : public BaseNode {
public:

    IPatternNode(IPattern*);

    boolean SameValueAs(void*);

    IPattern* GetPattern();

protected:

    IPattern* pattern;		// points to an IPattern

};

// IPatternNode stores the IPattern pointer.

inline IPatternNode::IPatternNode (IPattern* p) {
    pattern = p;
}

// SameValueAs returns true if the given pointer equals the stored
// IPattern pointer.

static boolean IPatternNode::SameValueAs (void* p) {
    return pattern == p;
}

// Define inline access functions to get members' values.

inline IPattern* IPatternNode::GetPattern () {
    return pattern;
}

// An IPatternList manages a list of IPatternNodes.

class IPatternList : public BaseList {
public:

    IPatternNode* First();
    IPatternNode* Last();
    IPatternNode* Prev();
    IPatternNode* Next();
    IPatternNode* GetCur();
    IPatternNode* Index(int);

};

// Cast these functions to return IPatternNodes instead of BaseNodes.

inline IPatternNode* IPatternList::First () {
    return (IPatternNode*) BaseList::First();
}

inline IPatternNode* IPatternList::Last () {
    return (IPatternNode*) BaseList::Last();
}

inline IPatternNode* IPatternList::Prev () {
    return (IPatternNode*) BaseList::Prev();
}

inline IPatternNode* IPatternList::Next () {
    return (IPatternNode*) BaseList::Next();
}

inline IPatternNode* IPatternList::GetCur () {
    return (IPatternNode*) BaseList::GetCur();
}

inline IPatternNode* IPatternList::Index (int index) {
    return (IPatternNode*) BaseList::Index(index);
}

#endif
