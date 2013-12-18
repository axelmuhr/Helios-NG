/*
 * C++ interface to C library regular expression functions
 */

#ifndef regexpr_h
#define regexpr_h

/*
 * BSD library routines
 */

extern const char* re_comp(const char* pattern);
extern int re_exec(const char* string);

#endif
