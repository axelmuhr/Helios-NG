// $Header: istring.h,v 1.7 89/03/19 12:17:40 interran Exp $
// extends <string.h> to define strdup and strndup too.

#ifndef istring_h
#define istring_h

#include <string.h>

// strdup allocates and returns a duplicate of the given string.

inline char* strdup (const char* s) {
    char* dup = new char[strlen(s) + 1];
    strcpy(dup, s);
    return dup;
}

// strndup allocates and returns a duplicate of the first len
// characters of the given string.

inline char* strndup (const char* s, int len) {
    char* dup = new char[len + 1];
    strncpy(dup, s, len);
    dup[len] = '\0';
    return dup;
}

#endif
