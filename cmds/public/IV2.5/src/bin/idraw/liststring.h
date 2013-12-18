// $Header: liststring.h,v 1.8 89/04/17 00:31:18 linton Exp $
// declares classes StringNode and StringList.

#ifndef liststring_h
#define liststring_h

#include "istring.h"
#include "list.h"

// A StringNode contains a pointer to a string of text.

class StringNode : public BaseNode {
public:

    StringNode(const char*);
    StringNode(const char*, int);
    ~StringNode();

    const char* GetString();

protected:

    char* string;		// points to a string of text

};

// StringNode stores a duplicate of the given string of text.

static StringNode::StringNode (const char* text) {
    string = strdup(text);
}

static StringNode::StringNode (const char* text, int length) {
    string = strndup(text, length);
}

// Free storage allocated for the string.

static StringNode::~StringNode () {
    delete string;
}

// Define inline access functions to get members' values.

inline const char* StringNode::GetString () {
    return string;
}

// A StringList manages a list of StringNodes.

class StringList : public BaseList {
public:

    StringNode* First();
    StringNode* Last();
    StringNode* Prev();
    StringNode* Next();
    StringNode* GetCur();
    StringNode* Index(int);

};

// Cast these functions to return StringNodes instead of BaseNodes.

inline StringNode* StringList::First () {
    return (StringNode*) BaseList::First();
}

inline StringNode* StringList::Last () {
    return (StringNode*) BaseList::Last();
}

inline StringNode* StringList::Prev () {
    return (StringNode*) BaseList::Prev();
}

inline StringNode* StringList::Next () {
    return (StringNode*) BaseList::Next();
}

inline StringNode* StringList::GetCur () {
    return (StringNode*) BaseList::GetCur();
}

inline StringNode* StringList::Index (int index) {
    return (StringNode*) BaseList::Index(index);
}

#endif
