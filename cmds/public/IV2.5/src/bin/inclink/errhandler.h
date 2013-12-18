/*
 * Interface to error handling operations.
 */

#ifndef errhandler_h
#define errhandler_h

#include "defs.h"

class ErrorHandler {
protected:
    int errlevel;
    int count;
    char* msgbuff;
    char* msgend;
    int msglen;

    void Flush();
    void Copy(const char*);

public:
    ErrorHandler(int, int);
    void Reset () { count = 0; }
    virtual boolean StartMessage();
    void Add(const char*);
    void Add(int);
    virtual void EndMessage();
    void operator()(const char*);
    void operator()(const char*, const char*);
    void operator()(const char*, int);
    void operator()(const char*, const char*, int);
    void operator()(const char*, int, int);
    int NumErrors () { return count; }
};

extern const char* errprefix;

extern ErrorHandler Error;
extern ErrorHandler Warning;

class FatalErrorHandler : public ErrorHandler {
public:
    FatalErrorHandler(int, int);
    void EndMessage();
};

extern FatalErrorHandler Fatal;
extern FatalErrorHandler Panic;

class DebugMessage : public ErrorHandler {
public:
    boolean messages;

    DebugMessage();
    boolean StartMessage();
};

extern DebugMessage Debug;

#endif
