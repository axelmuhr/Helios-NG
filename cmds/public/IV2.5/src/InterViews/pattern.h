/*
 * A pattern is a bit array describing where to fill.
 */

#ifndef pattern_h
#define pattern_h

#include <InterViews/resource.h>

static const int patternHeight = 16;
static const int patternWidth = 16;

class Pattern : public Resource {
public:
    Pattern(int p[patternHeight]);
    Pattern(int dither);
    Pattern(class Bitmap*);
    ~Pattern();
private:
    friend class Painter;

    void* info;
};

extern Pattern* solid;
extern Pattern* clear;
extern Pattern* lightgray;
extern Pattern* gray;
extern Pattern* darkgray;

#endif
