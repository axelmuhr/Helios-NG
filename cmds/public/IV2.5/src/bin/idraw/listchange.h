// $Header: listchange.h,v 1.8 89/04/17 00:31:09 linton Exp $
// declares subclasses of ChangeNode and class ChangeList.

#ifndef listchange_h
#define listchange_h

#include "list.h"

// Declare imported types.

class CenterList;
class Drawing;
class DrawingView;
class GroupList;
class IBrush;
class IBrushList;
class IColor;
class IColorList;
class IFont;
class IFontList;
class IPattern;
class IPatternList;
class Selection;
class SelectionList;
class State;

// ChangeNode stores a change to the Drawing in reversible form and
// can carry out or remove the change.

class ChangeNode : public BaseNode {
public:

    ChangeNode();
    ChangeNode(Drawing*, DrawingView*, boolean = false);
    ~ChangeNode();

    virtual void Do();
    virtual void Undo();

protected:

    Drawing* drawing;		// performs operations on drawing
    DrawingView* drawingview;	// displays drawing
    SelectionList* oldsl;	// lists the old Selections

};

// MoveChange translates the Selections.

class MoveChange : public ChangeNode {
public:

    MoveChange(Drawing*, DrawingView*, float, float);

    void Do();
    void Undo();

protected:

    float dx, dy;		// carries out translation
    float undodx, undody;	// removes translation

};

// ScaleChange scales the Selections.

class ScaleChange : public ChangeNode {
public:

    ScaleChange(Drawing*, DrawingView*, float, float);

    void Do();
    void Undo();

protected:

    float sx, sy;		// carries out scaling
    float undosx, undosy;	// removes scaling

};

// StretchChange stretches the Selections.

class StretchChange : public ChangeNode {
public:

    StretchChange(Drawing*, DrawingView*, float, Alignment);

    void Do();
    void Undo();

protected:

    Alignment OppositeSide(Alignment);

    float stretch;		// carries out stretching
    float undostretch;		// removes stretching
    Alignment side;		// indicates fixed side for do
    Alignment undoside;		// indicates fixed side for undo

};

// RotateChange rotates the Selections.

class RotateChange : public ChangeNode {
public:

    RotateChange(Drawing*, DrawingView*, float);

    void Do();
    void Undo();

protected:

    float angle;		// carries out rotation
    float undoangle;		// removes rotation

};

// ReplaceChange replaces a Selection with another Selection.

class ReplaceChange : public ChangeNode {
public:

    ReplaceChange(Drawing*, DrawingView*, Selection*, Selection*);
    ~ReplaceChange();

    void Do();
    void Undo();

protected:

    Selection* replacee;	// stores to-be-replaced Selection's address
    Selection* replacer;	// stores replacing Selection's address

};

// SetBrushChange sets the Selections' brush.

class SetBrushChange : public ChangeNode {
public:

    SetBrushChange(Drawing*, DrawingView*, IBrush*);
    ~SetBrushChange();

    void Do();
    void Undo();

protected:

    IBrush* brush;		// brush value to set
    IBrushList* undobrushlist;	// brush values to restore

};

// SetFgColorChange sets the Selections' foreground color.

class SetFgColorChange : public ChangeNode {
public:

    SetFgColorChange(Drawing*, DrawingView*, IColor*);
    ~SetFgColorChange();

    void Do();
    void Undo();

protected:

    IColor* fgcolor;		// color value to set
    IColorList* undofglist;	// color values to restore

};

// SetBgColorChange sets the Selections' background color.

class SetBgColorChange : public ChangeNode {
public:

    SetBgColorChange(Drawing*, DrawingView*, IColor*);
    ~SetBgColorChange();

    void Do();
    void Undo();

protected:

    IColor* bgcolor;		// color value to set
    IColorList* undobglist;	// color values to restore

};

// SetFontChange sets the Selections' font.

class SetFontChange : public ChangeNode {
public:

    SetFontChange(Drawing*, DrawingView*, IFont*);
    ~SetFontChange();

    void Do();
    void Undo();

protected:

    IFont* font;		// font value to set
    IFontList* undofontlist;	// font values to restore

};

// SetPatternChange sets the Selections' pattern.

class SetPatternChange : public ChangeNode {
public:

    SetPatternChange(Drawing*, DrawingView*, IPattern*);
    ~SetPatternChange();

    void Do();
    void Undo();

protected:

    IPattern* pattern;		   // pattern value to set
    IPatternList* undopatternlist; // pattern values to restore

};

// AddChange adds the Selections to the Drawing.

class AddChange : public ChangeNode {
public:

    AddChange(Drawing*, DrawingView*);
    ~AddChange();

    void Do();
    void Undo();

protected:

    boolean done;		// remembers if change was done

};

// DeleteChange deletes the Selections from the Drawing.

class DeleteChange : public ChangeNode {
public:

    DeleteChange(Drawing*, DrawingView*);
    ~DeleteChange();

    void Do();
    void Undo();

protected:

    SelectionList* prevlist;	// lists the selections' predecessors
    boolean done;		// remembers if change was done

};

// CutChange removes the Selections from the Drawing and copies them
// to the Clipboard, deleting the Clipboard's previous contents.

class CutChange : public DeleteChange {
public:

    CutChange(Drawing*, DrawingView*);

    void Do();

};

// CopyChange copies the Selections to the Clipboard, deleting the
// Clipboard's previous contents.

class CopyChange : public ChangeNode {
public:

    CopyChange(Drawing*, DrawingView*);

    void Do();

};

// PasteChange copies the Selections in the Clipboard and appends the
// new Selections to the Drawing.

class PasteChange : public AddChange {
public:

    PasteChange(Drawing*, DrawingView*, State*);

};

// DuplicateChange copies the Selections and appends the new
// Selections to the Drawing.

class DuplicateChange : public AddChange {
public:

    DuplicateChange(Drawing*, DrawingView*);

};

// GroupChange groups the Selections into a single PictSelection.

class GroupChange : public ChangeNode {
public:

    GroupChange(Drawing*, DrawingView*);
    ~GroupChange();

    void Do();
    void Undo();

protected:

    SelectionList* prevlist;	// lists the selections' predecessors
    GroupList* grouplist;	// lists the selections and their new parent
    boolean done;		// remembers whether change was done

};

// UngroupChange ungroups each PictSelection into its children.

class UngroupChange : public ChangeNode {
public:

    UngroupChange(Drawing*, DrawingView*);
    ~UngroupChange();

    void Do();
    void Undo();

protected:

    GroupList* undogrouplist;	// lists the selections and their children
    boolean done;		// remembers whether change was done

};

// BringToFrontChange brings the Selections to the front of the
// Drawing.

class BringToFrontChange : public ChangeNode {
public:

    BringToFrontChange(Drawing*, DrawingView*);
    ~BringToFrontChange();

    void Do();
    void Undo();

protected:

    SelectionList* prevlist;	// lists the selections' predecessors

};

// SendToBackChange sends the Selections to the back of the Drawing.

class SendToBackChange : public ChangeNode {
public:

    SendToBackChange(Drawing*, DrawingView*);
    ~SendToBackChange();

    void Do();
    void Undo();

protected:

    SelectionList* prevlist;	// lists the selections' predecessors

};

// AlignChange aligns the Selections.

class AlignChange : public ChangeNode {
public:

    AlignChange(Drawing*, DrawingView*, Alignment, Alignment);
    ~AlignChange();

    void Do();
    void Undo();

protected:

    Alignment falign;		// part of fixed Selection to align up with
    Alignment malign;		// part of moving Selection to align up with
    CenterList* centerlist;	// stores Selections' original positions

};

// AlignToGridChange aligns the Selections to the grid.

class AlignToGridChange : public ChangeNode {
public:

    AlignToGridChange(Drawing*, DrawingView*);
    ~AlignToGridChange();

    void Do();
    void Undo();

protected:

    CenterList* centerlist;	// stores Selections' original positions

};

// A ChangeList manages a list of ChangeNodes.

class ChangeList : public BaseList {
public:

    ChangeNode* First();
    ChangeNode* Last();
    ChangeNode* Prev();
    ChangeNode* Next();
    ChangeNode* GetCur();
    ChangeNode* Index(int);

};

inline ChangeNode* ChangeList::First () {
    return (ChangeNode*) BaseList::First();
}

inline ChangeNode* ChangeList::Last () {
    return (ChangeNode*) BaseList::Last();
}

inline ChangeNode* ChangeList::Prev () {
    return (ChangeNode*) BaseList::Prev();
}

inline ChangeNode* ChangeList::Next () {
    return (ChangeNode*) BaseList::Next();
}

inline ChangeNode* ChangeList::GetCur () {
    return (ChangeNode*) BaseList::GetCur();
}

inline ChangeNode* ChangeList::Index (int index) {
    return (ChangeNode*) BaseList::Index(index);
}

#endif
