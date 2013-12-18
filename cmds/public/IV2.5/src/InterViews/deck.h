/*
 * Deck - a Scene for stacking Interactors
 */

#ifndef deck_h
#define deck_h

#include <InterViews/scene.h>

class Deck : public Scene {
public:
    Deck();
    Deck(const char*);
    Deck(Painter*);
    ~Deck();

    void Flip(int = 1);
    void FlipTo(int);
    void Top () { FlipTo(1); }
    void Bottom () { FlipTo(-1); }

    virtual void Adjust(Perspective&);
    virtual void Draw();
    virtual void GetComponents(Interactor**, int, Interactor**&, int&);
    virtual void Resize();
protected:
    class Card* cards;
    Interactor* top;

    virtual void DoInsert(Interactor*, boolean, Coord&, Coord&);
    virtual void DoRemove(Interactor*);
    virtual void Reconfig();
private:
    void FixPerspective();
    void Init();
    void NewTop();
};

#endif
