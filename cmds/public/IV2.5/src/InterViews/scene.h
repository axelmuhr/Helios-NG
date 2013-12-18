/*
 * Basic composite object for interaction.
 */

#ifndef scene_h
#define scene_h

#include <InterViews/interactor.h>

class Scene : public Interactor {
public:
    void Insert(Interactor*);
    void Insert(Interactor*, Coord x, Coord y, Alignment = BottomLeft);
    void Change(Interactor* = nil);
    void Remove(Interactor*);

    void Move(Interactor*, Coord x, Coord y, Alignment = BottomLeft);
    void Raise(Interactor*);
    void Lower(Interactor*);

    void Propagate(boolean);
protected:
    boolean propagate;

    Scene();
    Scene(Sensor*, Painter*);

    void PrepareToInsert(Interactor*);
    virtual Interactor* Wrap(Interactor*);
    virtual void DoInsert(Interactor*, boolean, Coord& x, Coord& y);
    virtual void DoChange(Interactor*);
    virtual void DoMove(Interactor*, Coord& x, Coord& y);
    virtual void DoRemove(Interactor*);
    virtual void DoRaise(Interactor*);
    virtual void DoLower(Interactor*);

    void Place(Interactor*, Coord, Coord, Coord, Coord, boolean map = true);
    void UserPlace(Interactor*, int w, int h);

    void Map(Interactor*, boolean raised = true);
    void Unmap(Interactor*);
private:
    void Assign(Interactor*, Coord x, Coord y, int w, int h);
    void DoAlign(Interactor*, Alignment, Coord& x, Coord& y);
    void DoMap(Interactor*, int w, int h);
    void InitCanvas(Interactor*);
    void MakeWindow(Interactor*, Coord x, Coord y, int w, int h);
    void SetWindowProperties(
	Interactor*, Coord x, Coord y, int w, int h, boolean placed
    );
    void Orphan(Interactor*);
};

/* Scene with a single component */
class MonoScene : public Scene {
public:
    MonoScene();
    MonoScene(Sensor*, Painter*);
    ~MonoScene();

    virtual void Draw();
    virtual void GetComponents(Interactor**, int, Interactor**&, int&);
    virtual void Resize();
protected:
    Interactor* component;

    virtual void DoInsert(Interactor*, boolean, Coord&, Coord&);
    virtual void DoChange(Interactor*);
    virtual void DoRemove(Interactor*);

    virtual void Reconfig();
};

#endif
