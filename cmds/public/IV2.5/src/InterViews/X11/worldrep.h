/*
 * X11-dependent World representation.
 */

#ifndef worldrep_h
#define worldrep_h

#include <InterViews/X11/Xlib.h>

extern WorldRep* _world;

/*
 * X window --> interactor mapping
 */
extern InteractorTable* assocTable;

/*
 * The current WorldView instance is only relevant to
 * window manager applications.
 */
extern WorldView* _worldview;

enum TxFontsOption {
    TxFontsDefault, TxFontsOff, TxFontsOn, TxFontsCache
};

enum TxImagesOption {
    TxImagesDefault, TxImagesAuto, TxImagesDest, TxImagesSource
};

enum DashOption {
    DashDefault, DashNone, DashThin, DashAll
};

class WorldRep {
private:
    friend class World;

    static char _host[100];
    Display* _display;
    int _screen;
    Window _root;
    Visual* _visual;
    XColormap _cmap;
    int _xor;
    TxFontsOption _txfonts;
    TxImagesOption _tximages;
    DashOption _dash;

    char* gethostname();
public:
    char* hostname () { return _host[0] == '\0' ? gethostname() : _host; }
    Display* display () { return _display; }
    int screen () { return _screen; }
    Window root () { return _root; }
    Visual* visual () { return _visual; }
    XColormap cmap () { return _cmap; }
    int xor () { return _xor; }
    TxFontsOption txfonts () { return _txfonts; }
    TxImagesOption tximages () { return _tximages; }
    DashOption dash () { return _dash; }
};

#endif
