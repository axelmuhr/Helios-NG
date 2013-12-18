/*
 * Interface to Label, an object derived from Graphic.
 */

#ifndef label_h
#define label_h

#include <InterViews/Graphic/base.h>

class Label : public Graphic {
public:
    Label();
    Label(const char*, Graphic* = nil);
    Label(const char*, int, Graphic* = nil);
    ~Label();

    const char* GetOriginal();
    const char* GetOriginal(int&);
    void GetOriginal(char*&);
    void GetOriginal(char*&, int&);

    virtual void SetFont(PFont*);
    virtual PFont* GetFont();

    virtual Graphic* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual void getExtent(float&, float&, float&, float&, float&, Graphic*);
    virtual boolean contains(PointObj&, Graphic*);
    virtual boolean intersects(BoxObj&, Graphic*);
    virtual void draw(Canvas*, Graphic*);

    virtual boolean read(PFile*);
    virtual boolean write(PFile*);
protected:
    char* string;
    int count;
    Ref font;
};

#endif
