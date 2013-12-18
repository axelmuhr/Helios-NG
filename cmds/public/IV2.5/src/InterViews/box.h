/*
 * Boxes are used to compose side-by-side.
 */

#ifndef box_h
#define box_h

#include <InterViews/scene.h>

class BoxElement;
class BoxCanonical;

class Box : public Scene {
public:
    void Align(Alignment);
    void Draw();
    void GetComponents(Interactor**, int, Interactor**&, int&);
    void Resize();
protected:
    Alignment align;

    Box();
    ~Box();

    virtual void ComputeShape(Shape*);
    virtual void GetActual(int& major, int& minor);
    virtual void GetCanonical(Shape*, BoxCanonical&);
    BoxElement* Head();
    virtual void PlaceElement(
	Interactor*, Coord major, int len, int thick, int minor
    );
    virtual void Reconfig();
private:
    int nelements;
    BoxElement* head;
    BoxElement* tail;

    void DoInsert(Interactor*, boolean, Coord& x, Coord& y);
    void DoChange(Interactor*);
    void DoRemove(Interactor*);
};

class HBox : public Box {
public:
    HBox();
    HBox(Interactor*);
    HBox(Interactor*, Interactor*);
    HBox(Interactor*, Interactor*, Interactor*);
    HBox(Interactor*, Interactor*, Interactor*, Interactor*);
    HBox(Interactor*, Interactor*, Interactor*, Interactor*, Interactor*);
    HBox(
	Interactor*, Interactor*, Interactor*, Interactor*, Interactor*,
	Interactor*
    );
    HBox(
	Interactor*, Interactor*, Interactor*, Interactor*, Interactor*,
	Interactor*, Interactor*
    );
protected:
    void ComputeShape(Shape*);
    void GetActual(int& major, int& minor);
    void GetCanonical(Shape*, BoxCanonical&);
    void PlaceElement(Interactor*, Coord, int, int, int);
private:
    void Init();
};

class VBox : public Box {
public:
    VBox();
    VBox(Interactor*);
    VBox(Interactor*, Interactor*);
    VBox(Interactor*, Interactor*, Interactor*);
    VBox(Interactor*, Interactor*, Interactor*, Interactor*);
    VBox(Interactor*, Interactor*, Interactor*, Interactor*, Interactor*);
    VBox(
	Interactor*, Interactor*, Interactor*, Interactor*, Interactor*,
	Interactor*
    );
    VBox(
	Interactor*, Interactor*, Interactor*, Interactor*, Interactor*,
	Interactor*, Interactor*
    );
protected:
    void ComputeShape(Shape*);
    void GetActual(int& major, int& minor);
    void GetCanonical(Shape*, BoxCanonical&);
    void PlaceElement(Interactor*, Coord, int, int, int);
private:
    void Init();
};

#endif
