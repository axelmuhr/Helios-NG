// $Header: highlighter.h,v 1.8 89/04/17 00:30:37 linton Exp $
// declares classes HighlighterParent and Highlighter.

#ifndef highlighter_h
#define highlighter_h

#include <InterViews/scene.h>

// A HighlighterParent creates a highlight painter for its interior
// Highlighters to share.

class HighlighterParent : public MonoScene {
public:

    HighlighterParent();
    ~HighlighterParent();

    boolean SameOutputAs(Painter*);
    Painter* GetHighlightPainter();

protected:

    Painter* highlight;		// stores painter to give interior Highlighters

};

// A Highlighter draws itself and highlights or unhighlights itself on
// command.

class Highlighter : public Interactor {
public:

    Highlighter();
    ~Highlighter();

    void SetHighlighterParent(HighlighterParent*);

    virtual void Highlight();
    virtual void Unhighlight();

protected:

    void Reconfig();

    HighlighterParent* hparent;	// gives us highlight painter if it's nonnil
    boolean highlighted;	// stores true if we should be highlighted
    Painter* highlight;		// draws us with reversed colors
    Painter* normal;		// draws us with normal colors

};

#endif
