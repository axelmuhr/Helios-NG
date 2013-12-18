/*
 * Interface to shell windows.
 */

#ifndef shell_h
#define shell_h

#include <InterViews/scene.h>

class Banner;
class OldTextBuffer;
class Emulator;

class Shell : public MonoScene {
public:
    int pty;

    Shell();
    Shell(int rows, int cols);
    Shell(char* name);
    Shell(char* name, int rows, int cols);

    void Run();
    void Write(const char*, int);
protected:
    Banner* banner;
    OldTextBuffer* text;
    Emulator* term;

    void Init();
    void Resize();
};

#endif
