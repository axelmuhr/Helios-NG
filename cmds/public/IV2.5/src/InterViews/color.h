/*
 * A color is defined by rgb intensities and usually accessed by name.
 */

#ifndef color_h
#define color_h

#include <InterViews/resource.h>

/*
 * Color intensity should be subrange 0..65535, but for compatibility
 * reasons it is an integer.
 */
typedef int ColorIntensity;

class Color : public Resource {
public:
    Color(ColorIntensity r, ColorIntensity g, ColorIntensity b);
    Color(const char*);
    Color(const char*, int length);
    Color(int pixel);
    ~Color();

    void Intensities(ColorIntensity& r, ColorIntensity& g, ColorIntensity& b);
    void DisplayIntensities(
        ColorIntensity& r, ColorIntensity& g, ColorIntensity& b
    );
    int PixelValue();
    boolean Valid();
protected:
    class ColorRep* rep;
    ColorIntensity red, green, blue;
};

class ColorRep {
private:
    friend class Color;

    ColorRep(int pixel, ColorIntensity&, ColorIntensity&, ColorIntensity&);
    ColorRep(const char*, ColorIntensity&, ColorIntensity&, ColorIntensity&);
    ColorRep(ColorIntensity, ColorIntensity, ColorIntensity);
    ~ColorRep();

    int GetPixel();
    void GetIntensities(ColorIntensity&, ColorIntensity&, ColorIntensity&);

    void* info;
};

extern Color* black;
extern Color* white;

inline void Color::Intensities (
    ColorIntensity& r, ColorIntensity& g, ColorIntensity& b
) {
    r = red; g = green; b = blue;
}

inline void Color::DisplayIntensities (
    ColorIntensity& r, ColorIntensity& g, ColorIntensity& b
) {
    rep->GetIntensities(r, g, b);
}

inline int Color::PixelValue () { return rep->GetPixel(); }

#endif
