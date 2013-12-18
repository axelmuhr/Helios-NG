/*
 * Persistent structured graphics demonstration program.
 */

#include <InterViews/bitmap.h>
#include <InterViews/frame.h>
#include <InterViews/graphic.h>
#include <InterViews/panner.h>
#include <InterViews/raster.h>
#include <InterViews/rubrect.h>
#include <InterViews/sensor.h>
#include <InterViews/tray.h>
#include <InterViews/world.h>
#include <InterViews/Graphic/damage.h>
#include <InterViews/Graphic/ellipses.h>
#include <InterViews/Graphic/grblock.h>
#include <InterViews/Graphic/label.h>
#include <InterViews/Graphic/lines.h>
#include <InterViews/Graphic/picture.h>
#include <InterViews/Graphic/polygons.h>
#include <InterViews/Graphic/rasterrect.h>
#include <InterViews/Graphic/splines.h>
#include <InterViews/Graphic/stencil.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Graphics class defines and implements the outer-level interactor for
 * the application.  An instance of Graphic is inserted into the world.
 */
class Graphics : public MonoScene {
public:
    Graphics();
protected:
    Picture* InitPicture();
};

/*
 * View is a GraphicBlock with its Handle member function redefined to detect
 * hits on the Graphic objects it contains.
 */
class View : public GraphicBlock {
public:
    View(Graphic*);
    virtual ~View();

    void Move(Graphic*, Event&);
    void Scale(Graphic*, Event&);
    void Rotate(Graphic*, Event&);

    virtual void Handle(Event&);
    virtual void Update();
protected:
    virtual void Draw();
    virtual void Resize();
private:
    void Track(Event&, Rubberband&);
protected:
    Damage* damage;
};

/* Persistent object initialization routines *********************************/

/*
 * Persistent object creation function that creates an instance of any
 * persistent object used in the application, given a ClassId.  A pointer
 * to this function is passed to the object manager constructor.
 *
 * In this program, we don't derive any new classes from Persistent, so
 * GraphicsConstruct simply calls GraphicConstruct. (We could have eliminated
 * GraphicsConstruct altogether and simply passed a pointer to GraphicConstruct
 * to the object manager constructor.)
 */
Persistent* GraphicsConstruct (ClassId id) {
    switch (id) {
	// id's of newly derived persistent classes would be cased here, e.g.:
	// case DERIVED_PERSISTENT_ID_NUMBER:	return new DerivedPersistent;
	default: {
	    return GraphicConstruct(id);
	}
    }
}

Coord x[] = { 50, 100, 75, 100,  50,  0 };
Coord y[] = {  0,  50, 75, 100, 125, 25 };

static const int iv_width = 53;
static const int iv_height = 53;
static unsigned char iv_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x0f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xfe, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x0f, 0xff, 0xff, 0xe0,
   0xff, 0x1f, 0xfe, 0x0f, 0xff, 0xff, 0xe0, 0xff, 0x1f, 0xfe, 0x0f, 0xff,
   0xff, 0xe0, 0xff, 0x1f, 0xfe, 0x0f, 0xff, 0xff, 0xe0, 0xff, 0x1f, 0xfe,
   0x0f, 0x0f, 0xf8, 0xe0, 0x03, 0x1e, 0xfe, 0x0f, 0x0f, 0xfc, 0xe0, 0x07,
   0x1e, 0xfe, 0x0f, 0x0f, 0x3e, 0x80, 0x0f, 0x1e, 0xfe, 0x0f, 0x0f, 0x7c,
   0xc0, 0x07, 0x1e, 0xfe, 0x0f, 0x0f, 0xf8, 0xe0, 0x03, 0x1e, 0xfe, 0x0f,
   0x4f, 0xf0, 0xf1, 0x41, 0x1e, 0xfe, 0x0f, 0xef, 0xe0, 0xfb, 0xe0, 0x1e,
   0xfe, 0x0f, 0xff, 0xc1, 0x7f, 0xf0, 0x1f, 0xfe, 0x0f, 0xff, 0x83, 0x3f,
   0xf8, 0x1f, 0xfe, 0x0f, 0xff, 0x07, 0x1f, 0xfc, 0x1f, 0xfe, 0x0f, 0xbf,
   0x0f, 0x0e, 0xbe, 0x1f, 0xfe, 0x0f, 0x3f, 0x1f, 0x04, 0x9f, 0x1f, 0xfe,
   0x0f, 0x00, 0x3e, 0x80, 0x0f, 0x00, 0xfe, 0x0f, 0x00, 0x7c, 0xc0, 0x07,
   0x00, 0xfe, 0x0f, 0x00, 0xf8, 0xe0, 0x03, 0x00, 0xfe, 0x0f, 0x00, 0x7c,
   0xc0, 0x07, 0x00, 0xfe, 0x0f, 0x00, 0x3e, 0x80, 0x0f, 0x00, 0xfe, 0x0f,
   0x3f, 0x1f, 0x04, 0x9f, 0x1f, 0xfe, 0x0f, 0xbf, 0x0f, 0x0e, 0xbe, 0x1f,
   0xfe, 0x0f, 0xff, 0x07, 0x1f, 0xfc, 0x1f, 0xfe, 0x0f, 0xff, 0x83, 0x3f,
   0xf8, 0x1f, 0xfe, 0x0f, 0xff, 0xc1, 0x7f, 0xf0, 0x1f, 0xfe, 0x0f, 0xef,
   0xe0, 0xfb, 0xe0, 0x1e, 0xfe, 0x0f, 0x4f, 0xf0, 0xf1, 0x41, 0x1e, 0xfe,
   0x0f, 0x0f, 0xf8, 0xe0, 0x03, 0x1e, 0xfe, 0x0f, 0x0f, 0x7c, 0xc0, 0x07,
   0x1e, 0xfe, 0x0f, 0x0f, 0x3e, 0x80, 0x0f, 0x1e, 0xfe, 0x0f, 0x0f, 0xfc,
   0xe0, 0x07, 0x1e, 0xfe, 0x0f, 0x0f, 0xf8, 0xe0, 0x03, 0x1e, 0xfe, 0x0f,
   0xff, 0xff, 0xe0, 0xff, 0x1f, 0xfe, 0x0f, 0xff, 0xff, 0xe0, 0xff, 0x1f,
   0xfe, 0x0f, 0xff, 0xff, 0xe0, 0xff, 0x1f, 0xfe, 0x0f, 0xff, 0xff, 0xe0,
   0xff, 0x1f, 0xfe, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x0f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xfe, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe,
   0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static Raster* CreateRaster () {
    const int w = 8;
    const int h = 8;
    const ColorIntensity limit = 65536;
    const ColorIntensity iinc = limit/w - 1;
    const ColorIntensity jinc = limit/h - 1;
    const ColorIntensity kinc = limit/(w*h) - 1;

    Color* color[w*h];
    int k = 0;

    for (int i = 1; i <= w; ++i) {
        for (int j = 1; j <= h; ++j) {
            color[k++] = new Color(i*iinc, j*jinc, limit - k*kinc);
        }
    }

    return new Raster(color, w, h);
}

/*
 * Primordial root object creation and initialization function.  A pointer
 * to this function is passed to the object manager constructor.
 */
void GraphicsInitialize (RefList* root) {
    Picture* pict = new Picture;
    FullGraphic dfault;
	    
    InitPPaint();
    root->Append(new RefList(pict));

    dfault.FillBg(true);
    dfault.SetColors(pblack, pwhite);
    dfault.SetPattern(psolid);
    dfault.SetBrush(psingle);
    dfault.SetFont(pstdfont);

    Line* line = new Line (0, 0, 75, 75, &dfault);
    MultiLine* multiline = new MultiLine (x, y, 6, &dfault);
    BSpline* spline = new BSpline (x, y, 6, &dfault);
    Rect* rect = new Rect (0, 0, 100, 100, &dfault);
    FillRect* frect = new FillRect (0, 0, 100, 100, &dfault);
    Circle* circle = new Circle (0, 0, 50, &dfault);
    FillCircle* fcircle = new FillCircle (0, 0, 50, &dfault);
    Polygon* poly = new Polygon (x, y, 6, &dfault);
    FillPolygon* fpoly = new FillPolygon (x, y, 6, &dfault);
    ClosedBSpline* cspline = new ClosedBSpline (x, y, 6, &dfault);
    FillBSpline* fspline = new FillBSpline (x, y, 6, &dfault);
    Label* label = new Label ("Type 'q' to quit this program.", &dfault);
    Stencil* stencil = new Stencil(new Bitmap(iv_bits, iv_width, iv_height));
    RasterRect* raster = new RasterRect(CreateRaster());

    line->Translate(0, 300);
    multiline->Translate(100, 300);
    spline->Translate(250, 300);
    rect->Translate(100, 150);
    frect->Translate(100, 0);
    circle->Scale(1.0, 0.6);
    circle->Translate(0, 150);
    fcircle->Scale(1.0, 0.6);
    poly->Translate(250, 150);
    fpoly->Translate(250, 0);
    cspline->Translate(400, 150);
    fspline->Translate(400, 0);
    label->Translate(350, 175);
    stencil->Translate(400, 300);
    raster->Scale(5, 5);
    raster->Translate(350, 350);

    pict->Append(line, multiline, spline, rect);
    pict->Append(frect, circle, fcircle, poly);
    pict->Append(fpoly, cspline, fspline, label);
    pict->Append(stencil, raster);
}

/* Implementation of Graphics class ******************************************/

Graphics::Graphics () {
    Picture* pict = InitPicture();
    View* view = new View(pict);
    Tray* interior = new Tray(view);
    Frame* pannerFrame = new Frame(new Panner(view));

    interior->Align(BottomRight, pannerFrame);
    interior->Propagate(false);  // keep pannerFrame from changing entire view
    Insert(interior);
}

Picture* Graphics::InitPicture () {
    TheManager = new ObjectMan(
	"graphics", &GraphicsInitialize, &GraphicsConstruct
    );
    Picture* pict = (Picture*) (*(*TheManager->GetRoot())[1])();
    pict->SetTransformer(nil);
    return pict;
}

/* Implementation of View class **********************************************/

View::View (Graphic* g) : (nil, g) {
    input = new Sensor(updownEvents);
    input->Catch(KeyEvent);
    damage = nil;
}

View::~View () { delete damage; }

void View::Handle (Event& e) {
    Coord slop = round(cm/15);
    Picture* pict = (Picture*) graphic;
    Graphic* g;

    if (e.eventType == DownEvent) {
	BoxObj b = BoxObj(e.x-slop, e.y-slop, e.x+slop, e.y+slop);
	g = pict->LastGraphicIntersecting(b);
	if (g != nil) {
            switch (e.button) {
            case LEFTMOUSE:
                Move(g, e); break;
            case MIDDLEMOUSE: 
                Scale(g, e); break;
            case RIGHTMOUSE:
                Rotate(g, e); break;
            }
	}
    } else if (e.eventType == KeyEvent && *e.keystring == 'q') {
        e.target = nil;
    }    
}

void View::Move (Graphic* g, Event& e) {
    Coord l, b, r, t, newl, newb;
    float dx, dy, mag = GetMagnification();
    
    damage->Incur(g);
    g->GetBox(l, b, r, t);
    SlidingRect sr(output, canvas, l, b, r, t, e.x, e.y);

    Track(e, sr);
    sr.GetCurrent(newl, newb, r, t);
    dx = (newl - l) / mag;  // get distance moved, corrected to reflect
    dy = (newb - b) / mag;  // current magnification
    g->Translate(dx, dy);
    g->Touch();		    // so that new position will be written out
    damage->Incur(g);
    Update();
}

void View::Scale (Graphic* g, Event& e) {
    Coord l, b, r, t;
    float cx, cy, scale;
    
    damage->Incur(g);
    g->GetBox(l, b, r, t);
    g->GetCenter(cx, cy);
    ScalingRect sr(output, canvas, l, b, r, t, round(cx), round(cy));

    Track(e, sr);
    scale = sr.CurrentScaling();
    g->Scale(scale, scale, cx, cy);
    g->Touch();
    damage->Incur(g);
    Update();
}

void View::Rotate (Graphic* g, Event& e) {
    Coord l, b, r, t;
    float cx, cy;
    
    damage->Incur(g);
    g->GetBox(l, b, r, t);
    g->GetCenter(cx, cy);
    RotatingRect rr(output, canvas, l, b, r, t, round(cx), round(cy), e.x,e.y);

    Track(e, rr);
    g->Rotate(rr.CurrentAngle(), cx, cy);
    g->Touch();
    damage->Incur(g);
    Update();
}

void View::Update () {
    UpdatePerspective();
    damage->Repair();
}

void View::Draw () {
    damage->Reset();
    GraphicBlock::Draw();
}

void View::Resize () {
    GraphicBlock::Resize();
    if (damage == nil) {
	damage = new Damage(canvas, output, GetGraphic());
    }
}

void View::Track (Event& e, Rubberband& r) {
    r.Draw();

    Listen(allEvents);
    do {
	if (e.eventType == MotionEvent) {
	    r.Track(e.x, e.y);
	}
	Read(e);
    } while (e.eventType != UpEvent);
    Listen(input);

    r.Erase();
}

/* Main program **************************************************************/

int main (int argc, char* argv[]) {
    register int i;
    boolean save = false;

    World* world = new World("graphics", argc, argv);

    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-s") == 0) {
	    save = true;
	} else {
	    fprintf(stderr, "bad arg %s\n", argv[i]);
	    exit(1);
	}
    }

    Graphics* graphics = new Graphics;
    world->InsertApplication(graphics);
    graphics->Run();
    delete graphics;
    if (save) {
	delete TheManager;
    }
    return 0;
}
