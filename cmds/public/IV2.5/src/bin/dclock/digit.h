/*
 * Digit class for digital clock
 */

#ifndef digit_h
#define digit_h

class Digit {
public :
    Digit(float, float);
    ~Digit();
    boolean Set(int);
    void Reconfig(Painter*);
    void Resize(Canvas*, int);
    void Redraw();
private:
    Segment* Segs[7];
    float Xorg;
    float Yorg;
};

#endif
