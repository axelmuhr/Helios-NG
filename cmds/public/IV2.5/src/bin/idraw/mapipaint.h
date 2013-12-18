// $Header: mapipaint.h,v 1.6 88/09/24 15:07:12 interran Exp $
// declares MapIPaint subclasses.

#ifndef mapipaint_h
#define mapipaint_h

#include <InterViews/defs.h>

// Declare imported types.

class BaseList;
class BaseNode;
class IBrush;
class IBrushList;
class IColor;
class IColorList;
class IFont;
class IFontList;
class IPattern;
class IPatternList;
class Interactor;

// A MapIPaint creates a list of predefined and user-defined entries.

class MapIPaint {
protected:

    void Init(BaseList*, Interactor*, const char*);
    void DefineEntries(BaseList*, Interactor*, const char*);
    void DefineInitial(Interactor*, const char*);
    virtual BaseNode* CreateEntry(const char*);

    int initial;		// denotes which entry is used on startup

};

// A MapIBrush manages a list of predefined and user-defined brushes.

class MapIBrush : public MapIPaint {
public:

    MapIBrush(Interactor*, const char*);
    ~MapIBrush();

    int Size();
    boolean AtEnd();
    IBrush* First();
    IBrush* Last();
    IBrush* Prev();
    IBrush* Next();
    IBrush* GetCur();
    IBrush* Index(int);
    boolean Find(IBrush*);
    IBrush* GetInitial();

    IBrush* FindOrAppend(boolean, int, int, int, int);

protected:

    BaseNode* CreateEntry(const char*);

    IBrushList* ibrushlist;	    // stores idraw's IBrushes

};

// A MapIColor manages a list of predefined and user-defined colors.

class MapIColor : public MapIPaint {
public:

    MapIColor(Interactor*, const char*);
    ~MapIColor();

    int Size();
    boolean AtEnd();
    IColor* First();
    IColor* Last();
    IColor* Prev();
    IColor* Next();
    IColor* GetCur();
    IColor* Index(int);
    boolean Find(IColor*);
    IColor* GetInitial();

    IColor* FindOrAppend(const char*, int, int, int);

protected:

    BaseNode* CreateEntry(const char*);

    IColorList* icolorlist;	    // stores idraw's IColors

};

// A MapIFont manages a list of predefined and user-defined fonts.

class MapIFont : public MapIPaint {
public:

    MapIFont(Interactor*, const char*);
    ~MapIFont();

    int Size();
    boolean AtEnd();
    IFont* First();
    IFont* Last();
    IFont* Prev();
    IFont* Next();
    IFont* GetCur();
    IFont* Index(int);
    boolean Find(IFont*);
    IFont* GetInitial();

    IFont* FindOrAppend(const char*, const char*, const char*);

protected:

    BaseNode* CreateEntry(const char*);

    IFontList* ifontlist;	    // stores idraw's IFonts

};

// A MapIPattern manages a list of predefined and user-defined
// patterns.

class MapIPattern : public MapIPaint {
public:

    MapIPattern(Interactor*, const char*);
    ~MapIPattern();

    int Size();
    boolean AtEnd();
    IPattern* First();
    IPattern* Last();
    IPattern* Prev();
    IPattern* Next();
    IPattern* GetCur();
    IPattern* Index(int);
    boolean Find(IPattern*);
    IPattern* GetInitial();

    IPattern* FindOrAppend(boolean, float, int*, int);

protected:

    BaseNode* CreateEntry(const char*);
    int CalcBitmap(float);

    void ExpandToFullSize(int*, int);

    IPatternList* ipatternlist;	    // stores idraw's IPatterns

};

#endif
