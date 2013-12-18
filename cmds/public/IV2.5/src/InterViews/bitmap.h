/*
 * Bitmap - a two-dimensional boolean mask
 */

#ifndef bitmap_h
#define bitmap_h

#include <InterViews/defs.h>
#include <InterViews/resource.h>

class Font;
class Transformer;

class Bitmap : public Resource {
public:
    Bitmap(const char* filename);
    Bitmap(void*, int width, int height, int x0 = 0, int y0 = 0);
    Bitmap(Font*, int);
    Bitmap(Bitmap*);
    ~Bitmap();

    void* Map();

    int Left();
    int Right();
    int Top();
    int Bottom();
    int Width();
    int Height();

    void Transform(Transformer*);
    void Scale(float sx, float sy);
    void Rotate(float angle);

    void FlipHorizontal();
    void FlipVertical();
    void Rotate90();
    void Rotate180();
    void Rotate270();
    void Invert();

    boolean Contains(int x, int y);
    boolean Peek(int x, int y);
    void Poke(boolean bit, int x, int y);
private:
    friend class Painter;

    class BitmapRep* rep;
};

enum BitTx {NoTx, FlipH, FlipV, Rot90, Rot180, Rot270, Inv};

class BitmapRep {
private:
    friend class Bitmap;
    friend class Painter;

    BitmapRep(const char* filename);
    BitmapRep(void* data, int w, int h, int x, int y);
    BitmapRep(Font*, int);
    BitmapRep(BitmapRep*, BitTx);
    BitmapRep(BitmapRep*, Transformer*);
    ~BitmapRep();

    void Touch();
    void PutBit(int x, int y, boolean bit);
    boolean GetBit(int x, int y);
    void* GetMap();
    void* GetData();

    int width;
    int height;
    int x0;
    int y0;
    void* map;
    void* data;
};

inline int Bitmap::Left () { return -rep->x0; }
inline int Bitmap::Right () { return rep->width - rep->x0 - 1; }
inline int Bitmap::Top () { return rep->height - rep->y0 - 1; }
inline int Bitmap::Bottom () { return -rep->y0; }
inline int Bitmap::Width () { return rep->width; }
inline int Bitmap::Height () { return rep->height; }

#endif
