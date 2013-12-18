/*
 * BSD byte-level operations.
 */

#ifndef bstring_h
#define bstring_h

extern void bcopy(const void* src, void* dst, int length);
extern int bcmp(const void* src, const void* dst, int length);
extern void bzero(void* dst, int length);
extern int fft(int);

#endif
