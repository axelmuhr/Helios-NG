/*
 * C++ interface to new improved random number generator
 */

#ifndef random_h
#define random_h

extern long random();
extern void srandom(int);

char* initstate(unsigned seed, char* state, int);
char* setstate(char* state);

#endif
