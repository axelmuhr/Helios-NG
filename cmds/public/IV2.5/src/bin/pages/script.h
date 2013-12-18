/*
 * Script
 */

#ifndef script_h
#define script_h

#include <InterViews/defs.h>
#include <stdio.h>

static const char NUMBER = '0';
static const char QUOTE = '\'';
static const char NONE = '\0';

class Script {
public:
    Script() { }

    virtual boolean Next() { return false; }

    virtual char Op() { return NONE; }
    virtual int Number() { return 0; }
    virtual int Size() { return 0; }
    virtual const char* Text() { return nil; }
};

class StringScript : public Script {
public:
    StringScript(const char* script, int length);
    StringScript(const char* script);

    virtual boolean Next();

    virtual char Op();
    virtual int Number();
    virtual int Size();
    virtual const char* Text();
protected:
    const char* script;
    int remaining;
    const char* text;
    char op;
};

class FileScript : public Script {
public:
    FileScript(const char* filename, int length);
    FileScript(const char* filename);
    FileScript(FILE* f);

    virtual boolean Next();

    virtual char Op();
    virtual int Number();
    virtual int Size();
    virtual const char* Text();
protected:
    FILE* file;
    const char* text;
    char op;
};

#endif
