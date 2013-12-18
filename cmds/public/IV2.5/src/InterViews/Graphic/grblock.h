/*
 * GraphicBlock - an interactor that contains a picture.
 */

#ifndef grblock_h
#define grblock_h

#include <InterViews/interactor.h>

typedef enum Zooming { Continuous, Binary };

class Graphic;
class Perspective;

class GraphicBlock : public Interactor {
public:
    GraphicBlock(
	Sensor*, Graphic*, 
	Coord pad = 0, Alignment = Center, Zooming = Continuous
    );
    ~GraphicBlock();

    virtual void Update();
    virtual void Draw();
    virtual void Adjust(Perspective&);
    virtual void Reconfig();
    virtual void Invert();

    Graphic* GetGraphic();
    float GetMagnification();
    void SetMagnification(float);
protected:
    virtual void Resize();
    virtual void Redraw(Coord, Coord, Coord, Coord);

    void SwapPainters();	    /* background: a hack to avoid SetColors */
    Painter* GetPainter();	    /* in menu highlighting. */
    void InvertPainter();
    void Init();

    void Normalize(Perspective&);   /* normalize units */
    void Align();		    /* align graphic */
    void Fix();			    /* keep alignment fixed during resize */
    float NearestPow2(float);	    /* convert to nearest power of 2 */
    float ScaleFactor(Perspective&);
    void UpdatePerspective();       /* recalc based on graphic's bbox */

    virtual void GetGraphicBox(Coord&, Coord&, Coord&, Coord&);
    virtual void Zoom(Perspective&);
    virtual void Scroll(Perspective&);
    virtual float LimitMagnification(float);
protected:
    Graphic* graphic;
    Coord pad;
    Alignment align;
    Zooming zooming;
    Coord x0, y0;                   /* graphic offset */
    float mag;			    /* total magnification */
    Painter* alt;		    /* alternate painter for drawing */
};

inline Graphic* GraphicBlock::GetGraphic () { return graphic; }
inline float GraphicBlock::GetMagnification () { return mag; }

#endif
