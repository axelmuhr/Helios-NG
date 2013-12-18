// $Header: ipaint.h,v 1.8 89/05/12 15:01:04 calder Exp $
// declares classes IBrush, IFont, and IPattern.

#ifndef ipaint_h
#define ipaint_h

#include <InterViews/Graphic/ppaint.h>

// An IBrush knows how to test for its noneness and get its line
// pattern, width, arrows, and dash pattern.

class IBrush : public PBrush {
public:

    IBrush();
    IBrush(int, int, boolean, boolean);

    boolean None();
    int GetLinePattern();
    int Width();
    boolean LeftArrow();
    boolean RightArrow();
    const int* GetDashPattern();
    int GetDashPatternSize();
    int GetDashOffset();
    operator Brush*();

protected:

    void CalcDashPat(int);

    boolean leftarrow;		// stores true if line starts from an arrowhead
    boolean rightarrow;		// stores true if line ends in an arrowhead
    int dashpat[patternWidth];	// stores dash pattern
    int dashpatsize;		// stores number of defined elements in dashpat
    int dashoffset;		// stores dash pattern's offset

};

// Define inline access functions to get members' values.

inline boolean IBrush::None () {
    return (value == nil);
}

inline boolean IBrush::LeftArrow () {
    return leftarrow;
}

inline boolean IBrush::RightArrow () {
    return rightarrow;
}

inline int IBrush::GetLinePattern () {
    return p;
}

inline const int* IBrush::GetDashPattern () {
    return dashpat;
}

inline int IBrush::GetDashPatternSize () {
    return dashpatsize;
}

inline int IBrush::GetDashOffset () {
    return dashoffset;
}

inline IBrush::operator Brush* () {
    return value;
}

// An IColor knows how to get its name.

class IColor : public PColor {
public:

    IColor(const char*);
    IColor(int, int, int, const char*);
    IColor(Color*, const char*);
    ~IColor();

    const char* GetName();
    operator Color*();

protected:

    char* name;			// stores name passed into constructor

};

// Define inline access functions to get members' values.

inline const char* IColor::GetName () {
    return name;
}

inline IColor::operator Color* () {
    return value;
}

// An IFont knows how to get its name, print font, and print size.

class IFont : public PFont {
public:

    IFont(const char*, const char*, const char*);
    ~IFont();

    const char* GetName();
    const char* GetPrintFont();
    const char* GetPrintSize();
    const char* GetPrintFontAndSize();
    operator Font*();

protected:

    const char* FilterName(const char*);

    char* printfont;		// stores name of font used by printer
    char* printsize;		// stores scale of font used by printer
    char* printfontandsize;	// stores name and size separated by a blank

};

// Define inline access functions to get members' values.

inline const char* IFont::GetName () {
    return name ? name : "stdfont";
}

inline const char* IFont::GetPrintFont () {
    return printfont;
}

inline const char* IFont::GetPrintSize () {
    return printsize;
}

inline const char* IFont::GetPrintFontAndSize () {
    return printfontandsize;
}

inline IFont::operator Font* () {
    return value;
}

// An IPattern knows how to test for its noneness or fullness and get
// its dither, data, and gray level.

class IPattern : public PPattern {
public:

    IPattern();
    IPattern(int, float);
    IPattern(int pattern[patternHeight], int);

    boolean None();
    float GetGrayLevel();
    const int* GetData();
    int GetSize();
    operator Pattern*();

protected:

    float graylevel;		// stores gray level for grayscale patterns
    int size;			// stores pat's orig size (4x4, 8x8, or 16x16)

};

// Define inline access functions to get members' values.

inline boolean IPattern::None () {
    return (value == nil);
}

inline float IPattern::GetGrayLevel () {
    return graylevel;
}

inline const int* IPattern::GetData () {
    return data;
}

inline int IPattern::GetSize () {
    return size;
}

inline IPattern::operator Pattern* () {
    return value;
}

#endif
