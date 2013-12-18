/*
 * Implementation of squares metaview.
 */

#include "metaview.h"
#include <InterViews/border.h>
#include <InterViews/box.h>
#include <InterViews/button.h>
#include <InterViews/deck.h>
#include <InterViews/event.h>
#include <InterViews/frame.h>
#include <InterViews/glue.h>
#include <InterViews/message.h>
#include <InterViews/paint.h>
#include <InterViews/painter.h>
#include <InterViews/tray.h>
#include <InterViews/world.h>

class Builder {
public:
    Builder(SquaresMetaView*);
private:
    friend class SquaresMetaView;

    SquaresMetaView* meta;
    Button* accept, * cancel;
    Button* scrollers, * panner;
    Button* small, * medium, * large;
    Button* below, * above;
    Button* right, * left;
    Button* bottomright;
    Button* bottomleft;
    Button* topright;
    Button* topleft;
    Button* horizontal;
    Button* vertical;
    
    Interactor* Body();
    Interactor* Title();
    Interactor* TypeLabel();
    Interactor* Types();
    Interactor* SizeLabel();
    Interactor* Sizes();
    
    Interactor* Controls();
    Interactor* TypeSizeControls();
    Interactor* ScrollerPositionControls();
    Interactor* PannerPositionControls();

    Interactor* ScrollerLabel();
    Interactor* PannerLabel();
    Interactor* HScrollerPositions();
    Interactor* VScrollerPositions();
};

SquaresMetaView::SquaresMetaView () {
    type = AdjustByScrollers;
    size = Medium;
    right = true;
    below = true;
    align = BottomRight;
    hscroll = true;
    vscroll = true;
    Init();
}

SquaresMetaView::SquaresMetaView (SquaresMetaView* m) {
    type = m->type;
    size = m->size;
    right = m->right;
    below = m->below;
    align = m->align;
    hscroll = m->hscroll;
    vscroll = m->vscroll;
    Init();
}

void SquaresMetaView::Init () {
    deck = new Deck;
    typeButton = new ButtonState;
    sizeButton = new ButtonState;
    rightButton = new ButtonState;
    belowButton = new ButtonState;
    alignButton = new ButtonState;
    hscrollButton = new ButtonState;
    vscrollButton = new ButtonState;
    accept = new ButtonState;
    Make();
}

SquaresMetaView::~SquaresMetaView () {
    delete typeButton;
    delete sizeButton;
    delete rightButton;
    delete belowButton;
    delete alignButton;
    delete hscrollButton;
    delete vscrollButton;
    delete accept;
}

boolean SquaresMetaView::Popup (Event& e) {
    World* w;
    Coord wx, wy;
    int status;

    typeButton->SetValue(type);
    sizeButton->SetValue(size);
    rightButton->SetValue(right);
    belowButton->SetValue(below);
    alignButton->SetValue(align);
    hscrollButton->SetValue(hscroll);
    vscrollButton->SetValue(vscroll);
    accept->SetValue(0);

    e.GetAbsolute(w, wx, wy);
    w->InsertToplevel(this, e.target);
    do {
        Read(e);
        if (e.eventType == KeyEvent || e.eventType == DownEvent) {
            Root()->Raise(this);
        }
        e.target->Handle(e);
        typeButton->GetValue(status);
        deck->FlipTo((status == AdjustByScrollers) ? 1 : 2);
        accept->GetValue(status);
    } while (status == 0 && e.target != nil);
    w->Remove(this);

    if (status == 1) {
	int v;

        typeButton->GetValue(v); type = v;
        sizeButton->GetValue(v); size = v;
        rightButton->GetValue(v); right = v;
        belowButton->GetValue(v); below = v;
        alignButton->GetValue(v); align = v;
        vscrollButton->GetValue(v); vscroll = v;
        hscrollButton->GetValue(v); hscroll = v;
	return true;
    }
    return false;
}

void SquaresMetaView::Make () {
    register Builder* b = new Builder(this);

    b->accept = new PushButton("Accept", accept, 1);
    b->cancel = new PushButton("Cancel", accept, -1);

    b->scrollers = new RadioButton("scrollers", typeButton, AdjustByScrollers);
    b->panner = new RadioButton("panner", typeButton, AdjustByPanner);

    b->small = new RadioButton("small", sizeButton, Small);
    b->medium = new RadioButton("medium", sizeButton, Medium);
    b->large = new RadioButton("large", sizeButton, Large);

    b->below = new RadioButton("below", belowButton, true);
    b->above = new RadioButton("above", belowButton, false); 
    b->right = new RadioButton("right", rightButton, true);
    b->left = new RadioButton("left", rightButton, false);

    b->bottomright = new RadioButton("bottom right", alignButton, BottomRight);
    b->bottomleft = new RadioButton("bottom left", alignButton, BottomLeft);
    b->topright = new RadioButton("top right", alignButton, TopRight);
    b->topleft = new RadioButton("top left", alignButton, TopLeft);

    b->horizontal = new CheckBox("Horizontal", hscrollButton, true, false);
    b->vertical = new CheckBox("Vertical", vscrollButton, true, false);

    b->horizontal->Attach(b->below);
    b->horizontal->Attach(b->above);
    b->vertical->Attach(b->left);
    b->vertical->Attach(b->right);

    Insert(new Frame("dialog", b->Body()));
    delete b;
}

Builder::Builder (SquaresMetaView* m) {
    meta = m;
}

inline int spc (int n = 1) { return n * round(.25*cm); }
inline HGlue* hspc (int n = 1) { return new HGlue(spc(n), spc(n), spc(2*n)); }
inline VGlue* vspc (int n = 1) { return new VGlue(spc(n), spc(n), spc(2*n)); }

inline int gap (int n = 1) { return n * round(1*cm); }
inline HGlue* hgap (int n = 1) { return new HGlue(gap(n), gap(n), gap(2*n)); }
inline VGlue* vgap (int n = 1) { return new VGlue(gap(n), gap(n), gap(2*n)); }

Interactor* Builder::Body () {
    return new HBox(
	new HGlue(gap(), gap(), hfil),
	new VBox(
	    new VGlue(gap(), gap(), vfil),
	    Title(),
	    new VGlue(spc(2), spc(2), 0),
	    Controls(),
	    new VGlue(gap(), gap(), vfil)
	),
	new HGlue(gap(), gap(), hfil)
    );
}

Interactor* Builder::Controls () {
    meta->deck->Insert(ScrollerPositionControls());
    meta->deck->Insert(PannerPositionControls());

    return new VBox(
	TypeSizeControls(),
	new VGlue(spc(), spc(), 0),
	meta->deck
    );
}

Interactor* Builder::TypeSizeControls () {
    Tray* t = new Tray;
    Interactor* typeLabel = TypeLabel();
    Interactor* types = Types();
    Interactor* sizeLabel = SizeLabel();
    Interactor* sizes = Sizes();
    
    t->HBox(t, typeLabel, hgap(), types, new HGlue, t);
    t->HBox(t, sizeLabel, hgap(), sizes, new HGlue, t);
    t->Align(Left, types, sizes);

    t->VBox(t, typeLabel, vspc(), sizeLabel, t);
    t->Align(VertCenter, typeLabel, types);
    t->Align(VertCenter, sizeLabel, sizes);

    return t;
}

Interactor* Builder::ScrollerPositionControls () {
    Tray* t = new Tray;
    Interactor* scrollerLabel = ScrollerLabel();
    Interactor* hscrollerPositions = HScrollerPositions();
    Interactor* vscrollerPositions = VScrollerPositions();
    
    t->HBox(t, scrollerLabel);
    t->HBox(t, hspc(2), horizontal, hspc(2), hscrollerPositions, new HGlue, t);
    t->HBox(t, hspc(2), vertical, hspc(2), vscrollerPositions, new HGlue, t);
    t->Align(Left, horizontal, vertical);
    t->Align(Left, hscrollerPositions, vscrollerPositions);

    t->VBox(t, scrollerLabel, vspc(), horizontal, vspc(), vertical, t);
    t->Align(VertCenter, horizontal, hscrollerPositions);
    t->Align(VertCenter, vertical, vscrollerPositions);
    
    return t;
}

Interactor* Builder::PannerPositionControls () {
    Tray* t = new Tray;
    Interactor* pannerLabel = PannerLabel();

    t->HBox(t, pannerLabel);
    t->HBox(t, hspc(2), topleft, hspc(), topright, new HGlue, t);
    t->HBox(t, hspc(2), bottomleft, hspc(), bottomright, new HGlue, t);
    t->Align(Left, topleft, bottomleft);
    t->Align(Left, topright, bottomright);

    t->VBox(t, pannerLabel, vspc(), topleft, vspc(), bottomleft, t);
    t->Align(VertCenter, topleft, topright);
    t->Align(VertCenter, bottomleft, bottomright);
    
    return t;
}

Interactor* Builder::Title () {
    return new HBox(
	 new VBox(
	    new VGlue,
	    new HBox(
		new Message("MetaViewTitle", "Squares Frame Setup"),
		new HGlue
	    ),
	    new VGlue,
	    new HBorder,
	    new VGlue(2, 0),
	    new HBorder
	),
	new HGlue(spc(), 0),
	new VBox(
	    accept,
	    new VGlue(spc(), spc(), 0),
	    cancel
	)
    );
}

Interactor* Builder::TypeLabel () {
    return new Message("Adjuster type:");
}

Interactor* Builder::Types () {
    return new HBox(
	scrollers,
	new HGlue(spc(), spc(), 0),
	panner
    );
}

Interactor* Builder::SizeLabel () {
    return new Message("Adjuster size:");
}

Interactor* Builder::Sizes () {
    return new HBox(
	small,
	new HGlue(spc(), spc(), 0),
	medium,
	new HGlue(spc(), spc(), 0),
	large
    );
}

Interactor* Builder::ScrollerLabel () {
    return new Message("Scroller positions:");
}

Interactor* Builder::PannerLabel () {
    return new Message("Panner position:");
}

Interactor* Builder::HScrollerPositions () {
    return new HBox(
	above,
	new HGlue(spc(), spc(), 0),
	below
    );
}

Interactor* Builder::VScrollerPositions () {
    return new HBox(
	left,
	new HGlue(spc(), spc(), 0),
	right
    );
}
