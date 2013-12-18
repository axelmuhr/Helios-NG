// $Header: listcenter.h,v 1.6 88/09/24 15:06:24 interran Exp $
// declares classes CenterNode and CenterList.

#ifndef listcenter_h
#define listcenter_h

#include "list.h"

// A CenterNode contains two float values.

class CenterNode : public BaseNode {
public:

    CenterNode(float, float);

    float GetCx();
    float GetCy();

protected:

    float cx, cy;		// stores a position

};

// CenterNode stores the float values.

inline CenterNode::CenterNode (float x, float y) {
    cx = x;
    cy = y;
}

// Define inline access functions to get members' values.

inline float CenterNode::GetCx () {
    return cx;
}

inline float CenterNode::GetCy () {
    return cy;
}

// A CenterList manages a list of CenterNodes.

class CenterList : public BaseList {
public:

    CenterNode* First();
    CenterNode* Last();
    CenterNode* Prev();
    CenterNode* Next();
    CenterNode* GetCur();
    CenterNode* Index(int);

};

// Cast these functions to return CenterNodes instead of BaseNodes.

inline CenterNode* CenterList::First () {
    return (CenterNode*) BaseList::First();
}

inline CenterNode* CenterList::Last () {
    return (CenterNode*) BaseList::Last();
}

inline CenterNode* CenterList::Prev () {
    return (CenterNode*) BaseList::Prev();
}

inline CenterNode* CenterList::Next () {
    return (CenterNode*) BaseList::Next();
}

inline CenterNode* CenterList::GetCur () {
    return (CenterNode*) BaseList::GetCur();
}

inline CenterNode* CenterList::Index (int index) {
    return (CenterNode*) BaseList::Index(index);
}

#endif
