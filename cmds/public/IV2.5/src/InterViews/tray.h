/*
 * Tray - composes interactors into (possibly) overlapping layouts.
 */

#ifndef tray_h
#define tray_h

#include <InterViews/scene.h>
#include <InterViews/shape.h>

class TrayElement;
class TSolver;

class TGlue {
public:
    TGlue(int w = 0, int h = 0, int hstretch = hfil, int vstretch = vfil);
    TGlue(int w, int h, int hshr, int hstr, int vshr, int vstr);
    ~TGlue();

    Shape* GetShape();
private:
    Shape* shape;
};

class Tray : public Scene {
public:
    Tray(Interactor* background = nil);
    Tray(const char*, Interactor* background = nil);
    Tray(Sensor*, Painter*, Interactor* background = nil);
    ~Tray();
    
    virtual void Draw();
    virtual void Reshape(Shape&);
    virtual void GetComponents(Interactor**, int, Interactor**&, int&);

    void Align(Alignment, Interactor*, TGlue* = nil);
    void Align(Alignment, Interactor*, Alignment, Interactor*, TGlue* = nil);
    void Align(
	Alignment, Interactor*, Interactor*,
	Interactor* = nil, Interactor* = nil, Interactor* = nil,
	Interactor* = nil, Interactor* = nil
    );
    void HBox(
	Interactor*, Interactor*,
	Interactor* = nil, Interactor* = nil, Interactor* = nil,
	Interactor* = nil, Interactor* = nil
    );
    void VBox(
	Interactor*, Interactor*,
	Interactor* = nil, Interactor* = nil, Interactor* = nil,
	Interactor* = nil, Interactor* = nil
    );
protected:
    virtual void DoInsert(Interactor*, boolean, Coord& x, Coord& y);
    virtual void DoChange(Interactor*);
    virtual void DoRemove(Interactor*);
    virtual void Reconfig();
    virtual void Resize();
private:
    int nelements;
    TrayElement* head;
    TrayElement* tail;
    Interactor* bg;
    TSolver* tsolver;

    void Init(Interactor*);
    void ComponentBounds(int&, int&);
    boolean AlreadyInserted(Interactor*);
    void CalcShape();
    void PlaceElement(TrayElement*);
    boolean TrayOrBg(Interactor*);
};

inline Shape* TGlue::GetShape () { return shape; }

#endif
