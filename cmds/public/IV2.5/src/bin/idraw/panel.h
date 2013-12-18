// $Header: panel.h,v 1.7 89/04/17 00:31:24 linton Exp $
// declares classes Panel and PanelItem.

#ifndef panel_h
#define panel_h

#include "highlighter.h"

// Declare imported and used-before-defined types.

class PanelItem;

// A Panel displays several items but highlights only one item.

class Panel : public HighlighterParent {
public:

    Panel();

    void Enter(PanelItem*, char);
    PanelItem* LookUp(char);

    void Highlight(PanelItem*);
    PanelItem* GetCur();

    void PerformCurrentFunction(Event&);
    void PerformTemporaryFunction(Event&, char);

protected:

    PanelItem* cur;		// stores currently selected item
    PanelItem* items[128];	// stores items by their associated character

};

// A PanelItem displays two text labels and performs a function.

class PanelItem : public Highlighter {
public:

    PanelItem(Panel*, const char*, const char*, char);
    ~PanelItem();

    void Handle(Event&);

    virtual void Perform(Event&);

protected:

    void Reconfig();
    void Redraw(Coord, Coord, Coord, Coord);
    void Resize();

    Panel* panel;		// stores panel which this item belongs to
    char* name;			// stores item's text label
    char* key;			// stores label of key which selects this item

    Coord name_x, name_y;	// stores position at which to display name
    Coord key_x, key_y;		// stores position at which to display key
    Coord side;			// size of largest square fitting in canvas
    Coord offx;			// horizontal offset needed to center square
    Coord offy;			// vertical offset needed to center square

};

#endif
