// $Header: listgroup.h,v 1.7 89/04/17 00:31:14 linton Exp $
// declares classes GroupNode and GroupList.

#ifndef listgroup_h
#define listgroup_h

#include "listselectn.h"
#include "selection.h"

// Declare imported types.

class PictSelection;

// A GroupNode contains a PictSelection pointer, a boolean value, and
// two SelectionList pointers.

class GroupNode : public BaseNode {
public:

    GroupNode(PictSelection*, boolean, SelectionList*);
    ~GroupNode();

    PictSelection* GetParent();
    boolean GetHasChildren();
    SelectionList* GetChildren();
    SelectionList* GetChildrenGS();

protected:

    PictSelection* parent;	// contains a Selection which may have children
    boolean haschildren;	// true if this parent has children
    SelectionList* children;	// lists the parent's children, if any
    SelectionList* childrengs;	// stores the children's orig. graphic states

};

// GroupNode stores the parent, boolean value, children, and the
// children's original graphic states.

static GroupNode::GroupNode (PictSelection* p, boolean h, SelectionList* sl) {
    parent = p;
    haschildren = h;
    children = new SelectionList;
    childrengs = new SelectionList;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* child = sl->GetCur()->GetSelection();
	Selection* childgs = new Selection(child);
	children->Append(new SelectionNode(child));
	childrengs->Append(new SelectionNode(childgs));
    }
}

// Free storage allocated to list the children and allocated
// for their graphic states.

static GroupNode::~GroupNode () {
    delete children;
    for (childrengs->First(); !childrengs->AtEnd(); childrengs->Next()) {
	delete childrengs->GetCur()->GetSelection();
    }
    delete childrengs;
}

// Define inline access functions to get members' values.

inline PictSelection* GroupNode::GetParent () {
    return parent;
}

inline boolean GroupNode::GetHasChildren () {
    return haschildren;
}

inline SelectionList* GroupNode::GetChildren () {
    return children;
}

inline SelectionList* GroupNode::GetChildrenGS () {
    return childrengs;
}

// A GroupList manages a list of GroupNodes.

class GroupList : public BaseList {
public:

    GroupNode* First();
    GroupNode* Last();
    GroupNode* Prev();
    GroupNode* Next();
    GroupNode* GetCur();
    GroupNode* Index(int);

};

// Cast these functions to return GroupNodes instead of BaseNodes.

inline GroupNode* GroupList::First () {
    return (GroupNode*) BaseList::First();
}

inline GroupNode* GroupList::Last () {
    return (GroupNode*) BaseList::Last();
}

inline GroupNode* GroupList::Prev () {
    return (GroupNode*) BaseList::Prev();
}

inline GroupNode* GroupList::Next () {
    return (GroupNode*) BaseList::Next();
}

inline GroupNode* GroupList::GetCur () {
    return (GroupNode*) BaseList::GetCur();
}

inline GroupNode* GroupList::Index (int index) {
    return (GroupNode*) BaseList::Index(index);
}

#endif
