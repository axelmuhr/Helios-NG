/*
 * Simple vt100-style terminal emulator,
 * using an OldTextBuffer to manage output.
 */

#ifndef emulator_h
#define emulator_h

#include <InterViews/defs.h>

class OldTextBuffer;

class Emulator {
public:
    Emulator (OldTextBuffer* t) { device = -1; text = t; within = 0; }
    void SetDevice (int fd) { device = fd; }
    void Write(const char*, int);
protected:
    int device;			/* needed for reply on terminal id */
    OldTextBuffer* text;		/* output destination */
    int within;
    char escbuffer[50];

    void SetAttributes(int args[], int n);
    void Args(const char*, int len, int& bc, int& n, int args[]);
    boolean Valid(int, int numargs, int args[]);
    void DefaultArgs(int, int& numargs, int args[]);
    void SaveEscape(const char*, int blen, int bp);
    void CompleteEscape(const char*, int blen, int& bp);
    boolean ParseEscape(
	const char*, int blen, int obp, int& bc, int& c,
	int args[], int& numargs
    );
    void DoEscape(int, int args[], int nargs);
};

#endif
