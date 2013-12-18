// $Header: idraw.h,v 1.7 89/04/17 00:30:47 linton Exp $
// declares class Idraw.

#ifndef idraw_h
#define idraw_h

#include <InterViews/scene.h>

// Declare imported types.

class Drawing;
class DrawingView;
class Editor;
class ErrHandler;
class MapKey;
class State;

// An Idraw displays a drawing editor.

class Idraw : public MonoScene {
public:

    Idraw(int, char**);
    ~Idraw();

    void Run();

    void Handle(Event&);
    void Update();

protected:

    void ParseArgs(int, char**);
    void Init();

    const char* initialfile;	// stores name of initial file to open if any

    Drawing* drawing;		// performs operations on drawing
    DrawingView* drawingview;	// displays drawing
    Editor* editor;		// handles drawing and editing operations
    ErrHandler* errhandler;	// handles an X request error
    MapKey* mapkey;		// maps characters to Interactors
    State* state;		// stores current state info about drawing
    Interactor* tools;		// displays drawing tools

};

#endif
