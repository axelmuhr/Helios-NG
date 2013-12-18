/*
 * A menu is a list of items to select from using the mouse.
 */

#ifndef menu_h
#define menu_h

#include <InterViews/scene.h>

class Menu : public MonoScene {
public:
    Menu(boolean persistent = true);
    Menu(const char*, boolean persistent = true);
    Menu(Sensor*, Painter*, boolean persistent = true);

    virtual void Compose();
    virtual void Popup(Event&, Interactor*&);
    virtual void Handle(Event&);
    void GetSelection (Interactor*& i) { i = cur; }
protected:
    Scene* layout;
    Interactor* cur;
    Coord xoff, yoff;
    boolean persistent;

    virtual void DoInsert(Interactor*, boolean, Coord&, Coord&);
    virtual void Reconfig();
private:
    void Init();
};

class HMenu : public Menu {
public:
    HMenu(boolean persistent = true);
    HMenu(const char*, boolean persistent = true);
    HMenu(Sensor*, Painter*, boolean persistent = true);
private:
    void Init();
};

class VMenu : public Menu {
public:
    VMenu(boolean persistent = true);
    VMenu(const char*, boolean persistent = true);
    VMenu(Sensor*, Painter*, boolean persistent = true);
private:
    void Init();
};

/*
 * A menu item is an interactor that highlights itself when
 * it gets focus, and unhighlights itself when it loses focus.
 */

class MenuItem : public Interactor {
public:
    int tag;

    MenuItem(int = 0);
    MenuItem(const char*, int = 0);
    MenuItem(Painter*, int = 0);
    ~MenuItem();

    virtual void Handle(Event&);
    virtual void Highlight();
    virtual void UnHighlight();
protected:
    Painter* highlight;
    Painter* normal;

    virtual void Reconfig();
private:
    void Init(int);
};

class TextItem : public MenuItem {
public:
    TextItem(const char*, int = 0);
    TextItem(const char*, const char*, int = 0);
    TextItem(Painter*, const char*, int = 0);
protected:
    const char* text;

    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
private:
    void Init(const char*);
};

#endif
