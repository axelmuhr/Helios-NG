// $Header: state.h,v 1.8 89/05/12 15:05:01 calder Exp $
// declares class State.

#ifndef state_h
#define state_h

#include <InterViews/defs.h>

// Declare imported types.

class Graphic;
class IBrush;
class IColor;
class IFont;
class IPattern;
class Interactor;
class InteractorList;
class MapIBrush;
class MapIColor;
class MapIFont;
class MapIPattern;

// A State stores state information about the user's drawing and paint
// attributes to be used when creating new Selections.

enum ModifStatus {
    ReadOnly,			// no modifications to drawing allowed
    Unmodified,			// no modifications have been made yet
    Modified			// at least one modification has been made
};

class State {
public:

    State(Interactor*);
    ~State();

    operator Graphic*();

    IBrush* GetBrush();
    IColor* GetFgColor();
    IColor* GetBgColor();
    const char* GetDrawingName();
    boolean GetFillBg();
    IFont* GetFont();
    boolean GetGridding();
    float GetMagnif();
    MapIBrush* GetMapIBrush();
    MapIColor* GetMapIFgColor();
    MapIColor* GetMapIBgColor();
    MapIFont* GetMapIFont();
    MapIPattern* GetMapIPattern();
    ModifStatus GetModifStatus();
    IPattern* GetPattern();

    void SetBrush(IBrush*);
    void SetFgColor(IColor*);
    void SetBgColor(IColor*);
    void SetDrawingName(const char*);
    void SetFillBg(boolean);
    void SetFont(IFont*);
    void SetGridding(boolean);
    void SetMagnif(float);
    void SetModifStatus(ModifStatus);
    void SetPattern(IPattern*);

    void Attach(Interactor*);
    void Detach(Interactor*);
    void UpdateViews();

protected:

    char* drawingname;		// stores name of file where drawing is saved
    Graphic* graphicstate;	// contains user-chosen paint attributes
    boolean gridding;		// stores true if grid will constrain points
    float magnif;		// stores drawing's magnification factor
    MapIBrush* mapibrush;	// stores brushes user can choose from
    MapIColor* mapifgcolor;	// stores fg colors user can choose from
    MapIColor* mapibgcolor;	// stores bg colors user can choose from
    MapIFont* mapifont;		// stores fonts user can choose from
    MapIPattern* mapipattern;	// stores patterns user can choose from
    ModifStatus modifstatus;	// determines if drawing is or can be modified
    InteractorList* viewlist;	// lists views interested in our state

};

// operator Graphic* returns the State's Graphic paint attributes for
// Editor's use in creating new Selections.

inline State::operator Graphic* () {
    return graphicstate;
}

#endif
