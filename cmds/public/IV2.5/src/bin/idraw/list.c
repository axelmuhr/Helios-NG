// $Header: list.c,v 1.8 89/04/17 00:30:58 linton Exp $
// implements classes BaseNode and BaseList.

#include "list.h"

// BaseNode zeroes its pointers.

BaseNode::BaseNode () {
    prev = nil;
    next = nil;
}

BaseNode::~BaseNode () {
    // no storage allocated by base class
}

// SameValueAs returns true if this class contains a data member of
// any pointer type that has the same value as the given pointer.

boolean BaseNode::SameValueAs (void*) {
    // base class contains no data members
    return false;
}

// BaseList starts with only a header node.

BaseList::BaseList () {
    cur = head = new BaseNode;
    head->prev = head->next = head;
    size = 0;
}

// ~BaseList destroys the list.

BaseList::~BaseList () {
    DeleteAll();
    delete head;
}

// Index returns the node that would be the indexed element if the
// list was a C array (0 = first, 1 = next, ..., size-1 = last) or nil
// if the index is out of bounds.  Index sets the current node to the
// indexed node if it's in the list; otherwise, the current node
// remains the same.

BaseNode* BaseList::Index (int index) {
    BaseNode* elem = nil;
    if (index >= 0 && index < size) {
	elem = head->next;
	for (int i = 0; i < index; i++) {
	    elem = elem->next;
	}
	cur = elem;
    }
    return elem;
}

// Find returns true if the list contains a node with a data member
// whose value is the same as the given pointer or false if there is
// no such node.  Find sets the current node to that node only if it
// finds such a node; otherwise, the current node remains the same.

boolean BaseList::Find (void* value) {
    for (BaseNode* elem = head->next; elem != head; elem = elem->next) {
	if (elem->SameValueAs(value)) {
	    cur = elem;
	    return true;
	}
    }
    return false;
}

// Append inserts a node after the last node of the list.  The current
// node remains the same.

void BaseList::Append (BaseNode* appendee) {
    BaseNode* last = head->prev;
    appendee->prev = last;
    appendee->next = head;
    head->prev = appendee;
    last->next = appendee;
    ++size;
}

// Prepend inserts a node before the first node of the list.  The
// current node remains the same.

void BaseList::Prepend (BaseNode* prependee) {
    BaseNode* first = head->next;
    prependee->prev = head;
    prependee->next = first;
    first->prev = prependee;
    head->next  = prependee;
    ++size;
}

// InsertAfterCur inserts a node after the current node of the list.
// The current node remains the same.

void BaseList::InsertAfterCur (BaseNode* prependee) {
    BaseNode* first = cur->next;
    prependee->prev = cur;
    prependee->next = first;
    first->prev = prependee;
    cur->next   = prependee;
    ++size;
}

// InsertBeforeCur inserts a node before the current node of the list.
// The current node remains the same.

void BaseList::InsertBeforeCur (BaseNode* appendee) {
    BaseNode* last = cur->prev;
    appendee->prev = last;
    appendee->next = cur;
    cur->prev  = appendee;
    last->next = appendee;
    ++size;
}

// RemoveCur removes the current node of the list (if it's a node and
// not the head) and sets the current node to the following node.

void BaseList::RemoveCur () {
    if (cur != head) {
	BaseNode* before = cur->prev;
	BaseNode* after  = cur->next;
	after->prev  = before;
	before->next = after;
	cur = after;
	--size;
    }
}

// DeleteCur deletes the current node of the list (if it's a node and
// not the head) and sets the current node to the following node.

void BaseList::DeleteCur () {
    if (cur != head) {
	BaseNode* before = cur->prev;
	BaseNode* after  = cur->next;
	after->prev  = before;
	before->next = after;
	delete cur;
	cur = after;
	--size;
    }
}

// DeleteAll deletes all the nodes of the list, making the list empty
// again, and sets the current node to the list's head.

void BaseList::DeleteAll () {
    BaseNode* after = nil;
    for (BaseNode* doomed = head->next; doomed != head; doomed = after) {
	after = doomed->next;
	delete doomed;
    }
    cur = head;
    head->prev = head->next = head;
    size = 0;
}
