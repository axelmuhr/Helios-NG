/*
 * Text drawing fonts
 */

#ifndef font_h
#define font_h

#include <InterViews/resource.h>

class Font : public Resource {
public:
    Font(const char*);
    Font(const char*, int);
    ~Font();

    int Baseline();
    boolean FixedWidth();
    int Height();
    int Index(const char*, int offset, boolean between);
    int Index(const char*, int, int offset, boolean between);
    boolean Valid();
    int Width(const char*);
    int Width(const char*, int);
    void* Info();
private:
    friend class Painter;

    class FontRep* rep;

    void GetFontByName(const char*);
    void Init();
    boolean Lookup(const char*, int);
};

class FontRep : public Resource {
    friend class Font;
    friend class Painter;

    void* id;
    void* info;
    int height;

    ~FontRep();
};

extern Font* stdfont;

#endif
